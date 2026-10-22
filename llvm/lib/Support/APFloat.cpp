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

#અન્તર્ભાવ "llvm/ADT/APFloat.h"
#અન્તર્ભાવ "llvm/ADT/APSInt.h"
#અન્તર્ભાવ "llvm/ADT/ArrayRef.h"
#અન્તર્ભાવ "llvm/ADT/FloatingPointMode.h"
#અન્તર્ભાવ "llvm/ADT/FoldingSet.h"
#અન્તર્ભાવ "llvm/ADT/Hashing.h"
#અન્તર્ભાવ "llvm/ADT/STLExtras.h"
#અન્તર્ભાવ "llvm/ADT/SmallVector.h"
#અન્તર્ભાવ "llvm/ADT/StringExtras.h"
#અન્તર્ભાવ "llvm/ADT/StringRef.h"
#અન્તર્ભાવ "llvm/ADT/StringSwitch.h"
#અન્તર્ભાવ "llvm/Config/llvm-config.h"
#અન્તર્ભાવ "llvm/Support/Debug.h"
#અન્તર્ભાવ "llvm/Support/Error.h"
#અન્તર્ભાવ "llvm/Support/MathExtras.h"
#અન્તર્ભાવ "llvm/Support/raw_ostream.h"
#અન્તર્ભાવ <cstring>
#અન્તર્ભાવ <iterator>
#અન્તર્ભાવ <limits.h>

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

#અન્તર્ભાવ "shared/math.h"
#અન્તર્ભાવ "shared/math_check_exceptions.h"

#define APFLOAT_DISPATCH_ON_SEMANTICS(METHOD_CALL)                             \
  કરો {                                                                         \
    જો (usesLayout<IEEEFloat>(getSemantics()))                                 \
      પાછા U.IEEE.METHOD_CALL;                                               \
    જો (usesLayout<DoubleAPFloat>(getSemantics()))                             \
      પાછા U.Double.METHOD_CALL;                                             \
    llvm_unreachable("Unexpected semantics");                                  \
  } જ્યારે (ખોટું)

using નામાકાશ llvm;

/// A macro used to combine two fcCategory enums into one key which can be used
/// in a switch statement to classify how the interaction of two APFloat's
/// categories affects an operation.
///
/// TODO: If clang source code is ever allowed to use constexpr in its own
/// codebase, change this into a static inline function.
#define Pack_Categories_Into_ચાવી(_lhs, _rhs) ((_lhs) * 4 + (_rhs))

/* Assumed in hexadecimal significand parsing, and conversion to
   hexadecimal strings.  */
static_assert(APFloatBase::integerPartWidth % 4 == 0, "Part width must be divisible by 4!");

નામાકાશ llvm {

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
    સાચું,
    સાચું,
    સાચું,
    સાચું,
    સાચું};
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
    ખોટું,
    ખોટું,
    ખોટું,
    ખોટું};

constexpr fltSemantics APFloatBase::semFloat8E5M3FNU = {
    16,
    -14,
    4,
    8,
    fltNonfiniteBehavior::NanOnly,
    fltNanEncoding::AllOnes,
    સાચું,
    ખોટું,
    ખોટું};

constexpr fltSemantics APFloatBase::semFloat6E3M2FN = {
    4, -2, 3, 6, fltNonfiniteBehavior::FiniteOnly};
constexpr fltSemantics APFloatBase::semFloat6E2M3FN = {
    2, 0, 4, 6, fltNonfiniteBehavior::FiniteOnly};
constexpr fltSemantics APFloatBase::semFloat4E2M1FN = {
    2, 0, 2, 4, fltNonfiniteBehavior::FiniteOnly};
constexpr fltSemantics APFloatBase::semBogus = {0, 0, 0, 0};

અચલ llvm::fltSemantics &APFloatBase::EnumToSemantics(Semantics S) {
  પસંદગી (S) {
  સ્થિતિ S_IEEEhalf:
    પાછા IEEEhalf();
  સ્થિતિ S_BFloat:
    પાછા BFloat();
  સ્થિતિ S_IEEEsingle:
    પાછા IEEEsingle();
  સ્થિતિ S_IEEEdouble:
    પાછા IEEEdouble();
  સ્થિતિ S_IEEEquad:
    પાછા IEEEquad();
  સ્થિતિ S_x87DoubleExtended:
    પાછા x87DoubleExtended();
  સ્થિતિ S_PPCDoubleDouble:
    પાછા PPCDoubleDouble();
  સ્થિતિ S_PPCDoubleDoubleLegacy:
    પાછા PPCDoubleDoubleLegacy();
  સ્થિતિ S_Float8E5M2:
    પાછા Float8E5M2();
  સ્થિતિ S_Float8E5M2FNUZ:
    પાછા Float8E5M2FNUZ();
  સ્થિતિ S_Float8E4M3:
    પાછા Float8E4M3();
  સ્થિતિ S_Float8E4M3FN:
    પાછા Float8E4M3FN();
  સ્થિતિ S_Float8E4M3FNUZ:
    પાછા Float8E4M3FNUZ();
  સ્થિતિ S_Float8E4M3B11FNUZ:
    પાછા Float8E4M3B11FNUZ();
  સ્થિતિ S_Float8E3M4:
    પાછા Float8E3M4();
  સ્થિતિ S_FloatTF32:
    પાછા FloatTF32();
  સ્થિતિ S_Float8E8M0FNU:
    પાછા Float8E8M0FNU();
  સ્થિતિ S_Float8E5M3FNU:
    પાછા Float8E5M3FNU();
  સ્થિતિ S_Float6E3M2FN:
    પાછા Float6E3M2FN();
  સ્થિતિ S_Float6E2M3FN:
    પાછા Float6E2M3FN();
  સ્થિતિ S_Float4E2M1FN:
    પાછા Float4E2M1FN();
  }
  llvm_unreachable("Unrecognised floating semantics");
}

APFloatBase::Semantics
APFloatBase::SemanticsToEnum(અચલ llvm::fltSemantics &Sem) {
  જો (&Sem == &llvm::APFloat::IEEEhalf())
    પાછા S_IEEEhalf;
  નહીંતર જો (&Sem == &llvm::APFloat::BFloat())
    પાછા S_BFloat;
  નહીંતર જો (&Sem == &llvm::APFloat::IEEEsingle())
    પાછા S_IEEEsingle;
  નહીંતર જો (&Sem == &llvm::APFloat::IEEEdouble())
    પાછા S_IEEEdouble;
  નહીંતર જો (&Sem == &llvm::APFloat::IEEEquad())
    પાછા S_IEEEquad;
  નહીંતર જો (&Sem == &llvm::APFloat::x87DoubleExtended())
    પાછા S_x87DoubleExtended;
  નહીંતર જો (&Sem == &llvm::APFloat::PPCDoubleDouble())
    પાછા S_PPCDoubleDouble;
  નહીંતર જો (&Sem == &llvm::APFloat::PPCDoubleDoubleLegacy())
    પાછા S_PPCDoubleDoubleLegacy;
  નહીંતર જો (&Sem == &llvm::APFloat::Float8E5M2())
    પાછા S_Float8E5M2;
  નહીંતર જો (&Sem == &llvm::APFloat::Float8E5M2FNUZ())
    પાછા S_Float8E5M2FNUZ;
  નહીંતર જો (&Sem == &llvm::APFloat::Float8E4M3())
    પાછા S_Float8E4M3;
  નહીંતર જો (&Sem == &llvm::APFloat::Float8E4M3FN())
    પાછા S_Float8E4M3FN;
  નહીંતર જો (&Sem == &llvm::APFloat::Float8E4M3FNUZ())
    પાછા S_Float8E4M3FNUZ;
  નહીંતર જો (&Sem == &llvm::APFloat::Float8E4M3B11FNUZ())
    પાછા S_Float8E4M3B11FNUZ;
  નહીંતર જો (&Sem == &llvm::APFloat::Float8E3M4())
    પાછા S_Float8E3M4;
  નહીંતર જો (&Sem == &llvm::APFloat::FloatTF32())
    પાછા S_FloatTF32;
  નહીંતર જો (&Sem == &llvm::APFloat::Float8E8M0FNU())
    પાછા S_Float8E8M0FNU;
  નહીંતર જો (&Sem == &llvm::APFloat::Float8E5M3FNU())
    પાછા S_Float8E5M3FNU;
  નહીંતર જો (&Sem == &llvm::APFloat::Float6E3M2FN())
    પાછા S_Float6E3M2FN;
  નહીંતર જો (&Sem == &llvm::APFloat::Float6E2M3FN())
    પાછા S_Float6E2M3FN;
  નહીંતર જો (&Sem == &llvm::APFloat::Float4E2M1FN())
    પાછા S_Float4E2M1FN;
  નહીંતર
    llvm_unreachable("Unknown floating semantics");
}

તાર્કિક APFloatBase::isRepresentableBy(અચલ fltSemantics &A,
                                    અચલ fltSemantics &B) {
  પાછા A.maxExponent <= B.maxExponent && A.minExponent >= B.minExponent &&
         A.precision <= B.precision;
}

તાર્કિક APFloatBase::isLosslesslyConvertibleTo(અચલ fltSemantics &From,
                                            અચલ fltSemantics &InitialTo,
                                            તાર્કિક IgnoreNaNs) {
  જો (&From == &InitialTo)
    પાછા સાચું;

  // PPC double-double cannot be described by a conventional exponent range
  // and precision. In particular, converting it to another semantics drops
  // its low double, so conservatively reject conversions from it.
  જો (&From == &semPPCDoubleDouble)
    પાછા ખોટું;

  // Converting to PPC double-double is lossless exactly when the value fits
  // in its high double. In other words, the conversion is lossless if the
  // conversion to IEEEdouble is lossless.
  અચલ fltSemantics &To =
      &InitialTo == &semPPCDoubleDouble ? semIEEEdouble : InitialTo;

  જો (!isRepresentableBy(From, To))
    પાછા ખોટું;

  જો ((From.hasZero && !To.hasZero) ||
      (From.hasSignedRepr && !To.hasSignedRepr))
    પાછા ખોટું;

  // NegativeZero NaN encoding repurposes the negative-zero bit pattern, so a
  // conversion to such a format cannot preserve a source negative zero.
  તાર્કિક FromHasSignedZero = From.hasZero && From.hasSignedRepr &&
                           From.nanEncoding != fltNanEncoding::NegativeZero;
  તાર્કિક ToHasSignedZero = To.hasZero && To.hasSignedRepr &&
                         To.nanEncoding != fltNanEncoding::NegativeZero;
  જો (FromHasSignedZero && !ToHasSignedZero)
    પાછા ખોટું;

  // isRepresentableBy compares normalized exponent ranges. Also ensure that
  // the smallest source value, which may be denormal, is represented exactly
  // by the destination semantics.
  APFloat SmallestFrom = APFloat::getSmallest(From);
  તાર્કિક LosesInfo = ખોટું;
  (શૂન્ય)SmallestFrom.convert(To, APFloat::rmNearestTiesToEven, &LosesInfo);
  જો (LosesInfo)
    પાછા ખોટું;

  જો (From.nonFiniteBehavior == fltNonfiniteBehavior::FiniteOnly)
    પાછા સાચું;

  // Even when NaN representations can be ignored, NaNs must remain NaNs and
  // infinities must remain infinities. Otherwise the conversion can change
  // whether an operation with nnan has poison-producing operands.
  જો (IgnoreNaNs) {
    જો (From.nonFiniteBehavior == fltNonfiniteBehavior::IEEE754)
      પાછા To.nonFiniteBehavior == fltNonfiniteBehavior::IEEE754;
    પાછા To.nonFiniteBehavior != fltNonfiniteBehavior::FiniteOnly;
  }

  જો (From.nonFiniteBehavior == fltNonfiniteBehavior::IEEE754) {
    // Converting an IEEE signaling NaN to another semantics quiets it, so the
    // original value cannot be recovered by converting it back.
    પાછા ખોટું;
  }

  // NanOnly formats have no signaling NaNs. IEEE semantics can represent
  // their quiet NaNs; conversions between NanOnly formats are conservatively
  // accepted only when they use the same NaN encoding.
  જો (To.nonFiniteBehavior == fltNonfiniteBehavior::IEEE754)
    પાછા સાચું;
  પાછા To.nonFiniteBehavior == fltNonfiniteBehavior::NanOnly &&
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
સ્થિર constexpr unsigned અંક_Pow5_Parts_On_ઢગલો = 16;

સ્થિર unsigned પૂર્ણાંક powerOfFivePartsForExponent(unsigned પૂર્ણાંક power) {
  પ્રતિજ્ઞા(power <= maxExponent);

  // 815 / 351 is an upper bound on log2(5).  Reserve an additional part for
  // tcFullMultiply, which writes the full product width even when its most
  // significant part is zero.
  પાછા 2 + ((power * 815) / (351 * APFloatBase::integerPartWidth));
}

unsigned પૂર્ણાંક APFloatBase::semanticsPrecision(અચલ fltSemantics &semantics) {
  પાછા semantics.precision;
}
APFloatBase::ExponentType
APFloatBase::semanticsMaxExponent(અચલ fltSemantics &semantics) {
  પાછા semantics.maxExponent;
}
APFloatBase::ExponentType
APFloatBase::semanticsMinExponent(અચલ fltSemantics &semantics) {
  પાછા semantics.minExponent;
}
unsigned પૂર્ણાંક APFloatBase::semanticsSizeInBits(અચલ fltSemantics &semantics) {
  પાછા semantics.sizeInBits;
}
unsigned પૂર્ણાંક APFloatBase::semanticsIntSizeInBits(અચલ fltSemantics &semantics,
                                                 તાર્કિક isSigned) {
  // The max FP value is pow(2, MaxExponent) * (1 + MaxFraction), so we need
  // at least one more bit than the MaxExponent to hold the max FP value.
  unsigned પૂર્ણાંક MinBitWidth = semanticsMaxExponent(semantics) + 1;
  // Extra sign bit needed.
  જો (isSigned)
    ++MinBitWidth;
  પાછા MinBitWidth;
}

તાર્કિક APFloatBase::semanticsHasZero(અચલ fltSemantics &semantics) {
  પાછા semantics.hasZero;
}

તાર્કિક APFloatBase::semanticsHasSignedRepr(અચલ fltSemantics &semantics) {
  પાછા semantics.hasSignedRepr;
}

તાર્કિક APFloatBase::semanticsHasInf(અચલ fltSemantics &semantics) {
  પાછા semantics.nonFiniteBehavior == fltNonfiniteBehavior::IEEE754;
}

તાર્કિક APFloatBase::semanticsHasNaN(અચલ fltSemantics &semantics) {
  પાછા semantics.nonFiniteBehavior != fltNonfiniteBehavior::FiniteOnly;
}

તાર્કિક APFloatBase::isIEEELikeFP(અચલ fltSemantics &semantics) {
  // Keep in sync with Type::isIEEELikeFPTy
  પાછા SemanticsToEnum(semantics) <= S_IEEEquad;
}

તાર્કિક APFloatBase::hasSignBitInMSB(અચલ fltSemantics &semantics) {
  પાછા semantics.hasSignBitInMSB;
}

તાર્કિક APFloatBase::isRepresentableAsNormalIn(અચલ fltSemantics &Src,
                                            અચલ fltSemantics &Dst) {
  // Exponent range must be larger.
  જો (Src.maxExponent >= Dst.maxExponent || Src.minExponent <= Dst.minExponent)
    પાછા ખોટું;

  // If the mantissa is long enough, the result value could still be denormal
  // with a larger exponent range.
  //
  // FIXME: This condition is probably not accurate but also shouldn't be a
  // practical concern with existing types.
  પાછા Dst.precision >= Src.precision;
}

unsigned APFloatBase::getSizeInBits(અચલ fltSemantics &Sem) {
  પાછા Sem.sizeInBits;
}

સ્થિર constexpr APFloatBase::ExponentType
exponentZero(અચલ fltSemantics &semantics) {
  પાછા semantics.minExponent - 1;
}

સ્થિર constexpr APFloatBase::ExponentType
exponentInf(અચલ fltSemantics &semantics) {
  પાછા semantics.maxExponent + 1;
}

સ્થિર constexpr APFloatBase::ExponentType
exponentNaN(અચલ fltSemantics &semantics) {
  જો (semantics.nonFiniteBehavior == fltNonfiniteBehavior::NanOnly) {
    જો (semantics.nanEncoding == fltNanEncoding::NegativeZero)
      પાછા exponentZero(semantics);
    જો (semantics.hasSignedRepr || semantics.precision > 1)
      પાછા semantics.maxExponent;
  }
  પાછા semantics.maxExponent + 1;
}

/* A bunch of private, handy routines.  */

સ્થિર અંતઃરેખ Error createError(અચલ Twine &Err) {
  પાછા make_error<StringError>(Err, inconvertibleErrorCode());
}

સ્થિર constexpr અંતઃરેખ unsigned પૂર્ણાંક part_ગણતરી_For_Bits(unsigned પૂર્ણાંક bits) {
  પાછા std::max(1u, (bits + APFloatBase::integerPartWidth - 1) /
                          APFloatBase::integerPartWidth);
}

/* Returns 0U-9U.  Return values >= 10U are not digits.  */
સ્થિર અંતઃરેખ unsigned પૂર્ણાંક
dec_Digit_કિંમત(unsigned પૂર્ણાંક c)
{
  પાછા c - '0';
}

/* Return the value of a decimal exponent of the form
   [+-]ddddddd.

   If the exponent overflows, returns a large exponent with the
   appropriate sign.  */
સ્થિર Expected<પૂર્ણાંક> readExponent(StringRef::iterator begin,
                                  StringRef::iterator end) {
  અચલ unsigned પૂર્ણાંક overlargeExponent = 24000;  /* FIXME.  */
  StringRef::iterator p = begin;

  // Treat no exponent as 0 to match binutils
  જો (p == end || ((*p == '-' || *p == '+') && (p + 1) == end))
    પાછા 0;

  તાર્કિક isNegative = *p == '-';
  જો (*p == '-' || *p == '+') {
    p++;
    જો (p == end)
      પાછા createError("Exponent has no digits");
  }

  unsigned absExponent = dec_Digit_કિંમત(*p++);
  જો (absExponent >= 10U)
    પાછા createError("Invalid character in exponent");

  માટે (; p != end; ++p) {
    unsigned કિંમત_2 = dec_Digit_કિંમત(*p);
    જો (કિંમત_2 >= 10U)
      પાછા createError("Invalid character in exponent");

    absExponent = absExponent * 10U + કિંમત_2;
    જો (absExponent >= overlargeExponent) {
      absExponent = overlargeExponent;
      તોડો;
    }
  }

  જો (isNegative)
    પાછા -(પૂર્ણાંક) absExponent;
  નહીંતર
    પાછા (પૂર્ણાંક) absExponent;
}

/* This is ugly and needs cleaning up, but I don't immediately see
   how whilst remaining safe.  */
સ્થિર Expected<પૂર્ણાંક> totalExponent(StringRef::iterator p,
                                   StringRef::iterator end,
                                   પૂર્ણાંક exponentAdjustment) {
  પૂર્ણાંક exponent = 0;

  જો (p == end)
    પાછા createError("Exponent has no digits");

  તાર્કિક negative = *p == '-';
  જો (*p == '-' || *p == '+') {
    p++;
    જો (p == end)
      પાછા createError("Exponent has no digits");
  }

  પૂર્ણાંક unsignedExponent = 0;
  તાર્કિક overflow = ખોટું;
  માટે (; p != end; ++p) {
    unsigned પૂર્ણાંક કિંમત_2;

    કિંમત_2 = dec_Digit_કિંમત(*p);
    જો (કિંમત_2 >= 10U)
      પાછા createError("Invalid character in exponent");

    unsignedExponent = unsignedExponent * 10 + કિંમત_2;
    જો (unsignedExponent > 32767) {
      overflow = સાચું;
      તોડો;
    }
  }

  જો (exponentAdjustment > 32767 || exponentAdjustment < -32768)
    overflow = સાચું;

  જો (!overflow) {
    exponent = unsignedExponent;
    જો (negative)
      exponent = -exponent;
    exponent += exponentAdjustment;
    જો (exponent > 32767 || exponent < -32768)
      overflow = સાચું;
  }

  જો (overflow)
    exponent = negative ? -32768: 32767;

  પાછા exponent;
}

સ્થિર Expected<StringRef::iterator>
skipLeadingZeroesAndAnyDot(StringRef::iterator begin, StringRef::iterator end,
                           StringRef::iterator *dot) {
  StringRef::iterator p = begin;
  *dot = end;
  જ્યારે (p != end && *p == '0')
    p++;

  જો (p != end && *p == '.') {
    *dot = p++;

    જો (end - begin == 1)
      પાછા createError("Significand has no digits");

    જ્યારે (p != end && *p == '0')
      p++;
  }

  પાછા p;
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
સંરચના decimalInfo {
  અચલ અક્ષર *firstSigDigit;
  અચલ અક્ષર *lastSigDigit;
  પૂર્ણાંક exponent;
  પૂર્ણાંક normalizedExponent;
};

સ્થિર Error interpretDecimal(StringRef::iterator begin,
                              StringRef::iterator end, decimalInfo *D) {
  StringRef::iterator dot = end;

  auto PtrOrErr = skipLeadingZeroesAndAnyDot(begin, end, &dot);
  જો (!PtrOrErr)
    પાછા PtrOrErr.takeError();
  StringRef::iterator p = *PtrOrErr;

  D->firstSigDigit = p;
  D->exponent = 0;
  D->normalizedExponent = 0;

  માટે (; p != end; ++p) {
    જો (*p == '.') {
      જો (dot != end)
        પાછા createError("String contains multiple dots");
      dot = p++;
      જો (p == end)
        તોડો;
    }
    જો (dec_Digit_કિંમત(*p) >= 10U)
      તોડો;
  }

  જો (p != end) {
    જો (*p != 'e' && *p != 'E')
      પાછા createError("Invalid character in significand");
    જો (p == begin)
      પાછા createError("Significand has no digits");
    જો (dot != end && p - begin == 1)
      પાછા createError("Significand has no digits");

    /* p points to the first non-digit in the string */
    auto ExpOrErr = readExponent(p + 1, end);
    જો (!ExpOrErr)
      પાછા ExpOrErr.takeError();
    D->exponent = *ExpOrErr;

    /* Implied decimal point?  */
    જો (dot == end)
      dot = p;
  }

  /* If number is all zeroes accept any exponent.  */
  જો (p != D->firstSigDigit) {
    /* Drop insignificant trailing zeroes.  */
    જો (p != begin) {
      કરો
        કરો
          p--;
        જ્યારે (p != begin && *p == '0');
      જ્યારે (p != begin && *p == '.');
    }

    /* Adjust the exponents for any decimal point.  */
    D->exponent += static_cast<APFloat::ExponentType>((dot - p) - (dot > p));
    D->normalizedExponent = (D->exponent +
              static_cast<APFloat::ExponentType>((p - D->firstSigDigit)
                                      - (dot > D->firstSigDigit && dot < p)));
  }

  D->lastSigDigit = p;
  પાછા Error::success();
}

/* Return the trailing fraction of a hexadecimal number.
   DIGITVALUE is the first hex digit of the fraction, P points to
   the next digit.  */
સ્થિર Expected<lostFraction>
trailingHexadecimalFraction(StringRef::iterator p, StringRef::iterator end,
                            unsigned પૂર્ણાંક digit_કિંમત) {
  /* If the first trailing digit isn't 0 or 8 we can work out the
     fraction immediately.  */
  જો (digit_કિંમત > 8)
    પાછા lfMoreThanHalf;
  નહીંતર જો (digit_કિંમત < 8 && digit_કિંમત > 0)
    પાછા lfLessThanHalf;

  // Otherwise we need to find the first non-zero digit.
  જ્યારે (p != end && (*p == '0' || *p == '.'))
    p++;

  જો (p == end)
    પાછા createError("Invalid trailing hexadecimal fraction!");

  unsigned hexDigit = hex_Digit_કિંમત(*p);

  /* If we ran off the end it is exactly zero or one-half, otherwise
     a little more.  */
  જો (hexDigit == UINT_MAX)
    પાછા digit_કિંમત == 0 ? lfExactlyZero: lfExactlyHalf;
  નહીંતર
    પાછા digit_કિંમત == 0 ? lfLessThanHalf: lfMoreThanHalf;
}

/* Return the fraction lost were a bignum truncated losing the least
   significant BITS bits.  */
સ્થિર lostFraction
lostFractionThroughTruncation(અચલ APFloatBase::integerPart *parts,
                              unsigned પૂર્ણાંક part_ગણતરી,
                              unsigned પૂર્ણાંક bits)
{
  unsigned lsb = APInt::tcLSB(parts, part_ગણતરી);

  /* Note this is guaranteed true if bits == 0, or LSB == UINT_MAX.  */
  જો (bits <= lsb)
    પાછા lfExactlyZero;
  જો (bits == lsb + 1)
    પાછા lfExactlyHalf;
  જો (bits <= part_ગણતરી * APFloatBase::integerPartWidth &&
      APInt::tcExtractBit(parts, bits - 1))
    પાછા lfMoreThanHalf;

  પાછા lfLessThanHalf;
}

/* Shift DST right BITS bits noting lost fraction.  */
સ્થિર lostFraction
shift_જમણું(APFloatBase::integerPart *dst, unsigned પૂર્ણાંક parts, unsigned પૂર્ણાંક bits)
{
  lostFraction lost_fraction = lostFractionThroughTruncation(dst, parts, bits);

  APInt::tc_Shift_જમણું(dst, parts, bits);

  પાછા lost_fraction;
}

/* Combine the effect of two lost fractions.  */
સ્થિર lostFraction
combineLostFractions(lostFraction moreSignificant,
                     lostFraction lessSignificant)
{
  જો (lessSignificant != lfExactlyZero) {
    જો (moreSignificant == lfExactlyZero)
      moreSignificant = lfLessThanHalf;
    નહીંતર જો (moreSignificant == lfExactlyHalf)
      moreSignificant = lfMoreThanHalf;
  }

  પાછા moreSignificant;
}

/* The error from the true value, in half-ulps, on multiplying two
   floating point numbers, which differ from the value they
   approximate by at most HUE1 and HUE2 half-ulps, is strictly less
   than the returned value.

   See "How to Read Floating Point Numbers Accurately" by William D
   Clinger.  */
સ્થિર unsigned પૂર્ણાંક
HUerrBound(તાર્કિક inexactMultiply, unsigned પૂર્ણાંક HUerr1, unsigned પૂર્ણાંક HUerr2)
{
  પ્રતિજ્ઞા(HUerr1 < 2 || HUerr2 < 2 || (HUerr1 + HUerr2 < 8));

  જો (HUerr1 + HUerr2 == 0)
    પાછા inexactMultiply * 2;  /* <= inexactMultiply half-ulps.  */
  નહીંતર
    પાછા inexactMultiply + 2 * (HUerr1 + HUerr2);
}

/* The number of ulps from the boundary (zero, or half if ISNEAREST)
   when the least significant BITS are truncated.  BITS cannot be
   zero.  */
સ્થિર APFloatBase::integerPart
ulpsFromBoundary(અચલ APFloatBase::integerPart *parts, unsigned પૂર્ણાંક bits,
                 તાર્કિક isNearest) {
  પ્રતિજ્ઞા(bits != 0);

  bits--;
  unsigned ગણતરી = bits / APFloatBase::integerPartWidth;
  unsigned partBits = bits % APFloatBase::integerPartWidth + 1;

  APFloatBase::integerPart part =
      parts[ગણતરી] & (~(APFloatBase::integerPart)0 >>
                      (APFloatBase::integerPartWidth - partBits));

  APFloatBase::integerPart boundary;
  જો (isNearest)
    boundary = (APFloatBase::integerPart) 1 << (partBits - 1);
  નહીંતર
    boundary = 0;

  જો (ગણતરી == 0) {
    જો (part - boundary <= boundary - part)
      પાછા part - boundary;
    નહીંતર
      પાછા boundary - part;
  }

  જો (part == boundary) {
    જ્યારે (--ગણતરી)
      જો (parts[ગણતરી])
        પાછા ~(APFloatBase::integerPart) 0; /* A lot.  */

    પાછા parts[0];
  } નહીંતર જો (part == boundary - 1) {
    જ્યારે (--ગણતરી)
      જો (~parts[ગણતરી])
        પાછા ~(APFloatBase::integerPart) 0; /* A lot.  */

    પાછા -parts[0];
  }

  પાછા ~(APFloatBase::integerPart) 0; /* A lot.  */
}

સ્થિર constexpr unsigned PowerOfFivePartCounts[] = {1,  1,  2,  3,   5,  10,
                                                     19, 38, 75, 149, 298};

સ્થિર constexpr APFloatBase::integerPart PowerOfFiveParts[] = {
#અન્તર્ભાવ "APFloatPowerOfFiveTable.inc"
};

static_assert([] {
  unsigned Sum = 0;
  માટે (unsigned ગણતરી_2 : PowerOfFivePartCounts)
    Sum += ગણતરી_2;
  પાછા Sum;
}() == std::size(PowerOfFiveParts));

/* Place pow(5, power) in DST, and return the number of parts used.
   DST must be at least one part larger than size of the answer.  */
સ્થિર unsigned પૂર્ણાંક
powerOf5(APFloatBase::integerPart *dst, unsigned પૂર્ણાંક power) {
  સ્થિર અચલ APFloatBase::integerPart firstEightPowers[] = {
      1, 5, 25, 125, 625, 3125, 15625, 78125};

  SmallVector<APFloatBase::integerPart, અંક_Pow5_Parts_On_ઢગલો> scratch;
  scratch.resize_for_overwrite(powerOfFivePartsForExponent(power));

  APFloatBase::integerPart *p1, *p2;
  અચલ APFloatBase::integerPart *pow5;

  p1 = dst;
  p2 = scratch.data();

  *p1 = firstEightPowers[power & 7];
  power >>= 3;

  unsigned result = 1;
  pow5 = PowerOfFiveParts;

  માટે (unsigned પૂર્ણાંક n = 0; power; power >>= 1, n++) {
    unsigned parts_ગણતરી = PowerOfFivePartCounts[n];

    જો (power & 1) {
      APFloatBase::integerPart *tmp;

      APInt::tcFullMultiply(p2, p1, pow5, result, parts_ગણતરી);
      result += parts_ગણતરી;
      જો (p2[result - 1] == 0)
        result--;

      /* Now result is in p1 with partsCount parts and p2 is scratch
         space.  */
      tmp = p1;
      p1 = p2;
      p2 = tmp;
    }

    pow5 += parts_ગણતરી;
  }

  જો (p1 != dst)
    APInt::tcAssign(dst, p1, result);

  પાછા result;
}

/* Zero at the end to avoid modular arithmetic when adding one; used
   when rounding up during hexadecimal output.  */
સ્થિર અચલ અક્ષર hexDigitsLower[] = "0123456789abcdef0";
સ્થિર અચલ અક્ષર hexDigitsUpper[] = "0123456789ABCDEF0";
સ્થિર અચલ અક્ષર infinityL[] = "infinity";
સ્થિર અચલ અક્ષર infinityU[] = "INFINITY";
સ્થિર અચલ અક્ષર NaNL[] = "nan";
સ્થિર અચલ અક્ષર NaNU[] = "NAN";

/* Write out an integerPart in hexadecimal, starting with the most
   significant nibble.  Write out exactly COUNT hexdigits, return
   COUNT.  */
સ્થિર unsigned પૂર્ણાંક
partAsHex (અક્ષર *dst, APFloatBase::integerPart part, unsigned પૂર્ણાંક ગણતરી,
           અચલ અક્ષર *hexDigitChars)
{
  unsigned પૂર્ણાંક result = ગણતરી;

  પ્રતિજ્ઞા(ગણતરી != 0 && ગણતરી <= APFloatBase::integerPartWidth / 4);

  part >>= (APFloatBase::integerPartWidth - 4 * ગણતરી);
  જ્યારે (ગણતરી--) {
    dst[ગણતરી] = hexDigitChars[part & 0xf];
    part >>= 4;
  }

  પાછા result;
}

/* Write out an unsigned decimal integer.  */
સ્થિર અક્ષર *writeUnsignedDecimal(અક્ષર *dst, unsigned પૂર્ણાંક n) {
  અક્ષર buff[40], *p;

  p = buff;
  કરો
    *p++ = '0' + n % 10;
  જ્યારે (n /= 10);

  કરો
    *dst++ = *--p;
  જ્યારે (p != buff);

  પાછા dst;
}

/* Write out a signed decimal integer.  */
સ્થિર અક્ષર *writeSignedDecimal(અક્ષર *dst, પૂર્ણાંક કિંમત_2) {
  જો (કિંમત_2 < 0) {
    *dst++ = '-';
    dst = writeUnsignedDecimal(dst, -(unsigned) કિંમત_2);
  } નહીંતર {
    dst = writeUnsignedDecimal(dst, કિંમત_2);
  }

  પાછા dst;
}

// Compute the ULP of the input using a definition from:
// Jean-Michel Muller. On the definition of ulp(x). [Research Report] RR-5504,
// LIP RR-2005-09, INRIA, LIP. 2005, pp.16. inria-00070503
સ્થિર APFloat harrisonUlp(અચલ APFloat &X) {
  અચલ fltSemantics &Sem = X.getSemantics();
  પસંદગી (X.getCategory()) {
  સ્થિતિ APFloat::fcNaN:
    પાછા APFloat::getQNaN(Sem);
  સ્થિતિ APFloat::fcInfinity:
    પાછા APFloat::getInf(Sem);
  સ્થિતિ APFloat::fcZero:
    પાછા APFloat::getSmallest(Sem);
  સ્થિતિ APFloat::fcNormal:
    તોડો;
  }
  જો (X.isDenormal() || X.isSmallestNormalized())
    પાછા APFloat::getSmallest(Sem);
  પૂર્ણાંક Exp = ilogb(X);
  જો (X.getExactLog2() != INT_MIN)
    Exp -= 1;
  પાછા scalbn(APFloat::getOne(Sem), Exp - (Sem.precision - 1),
                APFloat::rmNearestTiesToEven);
}

નામાકાશ detail {
/* Constructors.  */
શૂન્ય IEEEFloat::initialize(અચલ fltSemantics *ourSemantics) {
  semantics = ourSemantics;
  unsigned ગણતરી = part_ગણતરી();
  જો (ગણતરી > 1)
    significand.parts = નવું integerPart[ગણતરી];
}

શૂન્ય IEEEFloat::freeSignificand() {
  જો (needsCleanup())
    delete [] significand.parts;
}

શૂન્ય IEEEFloat::assign(અચલ IEEEFloat &rhs) {
  પ્રતિજ્ઞા(semantics == rhs.semantics);

  sign = rhs.sign;
  category = rhs.category;
  exponent = rhs.exponent;
  જો (isFiniteNonZero() || category == fcNaN)
    copySignificand(rhs);
}

શૂન્ય IEEEFloat::copySignificand(અચલ IEEEFloat &rhs) {
  પ્રતિજ્ઞા(isFiniteNonZero() || category == fcNaN);
  પ્રતિજ્ઞા(rhs.part_ગણતરી() >= part_ગણતરી());

  APInt::tcAssign(significandParts(), rhs.significandParts(),
                  part_ગણતરી());
}

/* Make this number a NaN, with an arbitrary but deterministic value
   for the significand.  If double or longer, this is a signalling NaN,
   which may not be ideal.  If float, this is QNaN(0).  */
શૂન્ય IEEEFloat::makeNaN(તાર્કિક SNaN, તાર્કિક Negative, અચલ APInt *fill) {
  જો (semantics->nonFiniteBehavior == fltNonfiniteBehavior::FiniteOnly)
    llvm_unreachable("This floating point format does not support NaN");

  જો (Negative && !semantics->hasSignedRepr)
    llvm_unreachable(
        "This floating point format does not support signed values");

  category = fcNaN;
  sign = Negative;
  exponent = exponentNaN();

  integerPart *significand = significandParts();
  unsigned અંક_Parts = part_ગણતરી();

  APInt fill_storage;
  જો (semantics->nonFiniteBehavior == fltNonfiniteBehavior::NanOnly) {
    // Finite-only types do not distinguish signalling and quiet NaN, so
    // make them all signalling.
    SNaN = ખોટું;
    જો (semantics->nanEncoding == fltNanEncoding::NegativeZero) {
      sign = સાચું;
      fill_storage = APInt::getZero(semantics->precision - 1);
    } નહીંતર {
      fill_storage = APInt::getAllOnes(semantics->precision - 1);
    }
    fill = &fill_storage;
  }

  // Set the significand bits to the fill.
  જો (!fill || fill->get_અંક_શબ્દો() < અંક_Parts)
    APInt::tcSet(significand, 0, અંક_Parts);
  જો (fill) {
    APInt::tcAssign(significand, fill->getRawData(),
                    std::min(fill->get_અંક_શબ્દો(), અંક_Parts));

    // Zero out the excess bits of the significand.
    unsigned bitsToPreserve = semantics->precision - 1;
    unsigned part = bitsToPreserve / 64;
    bitsToPreserve %= 64;
    significand[part] &= ((1ULL << bitsToPreserve) - 1);
    માટે (part++; part != અંક_Parts; ++part)
      significand[part] = 0;
  }

  unsigned QNaNBit =
      (semantics->precision >= 2) ? (semantics->precision - 2) : 0;

  જો (SNaN) {
    // We always have to clear the QNaN bit to make it an SNaN.
    APInt::tcClearBit(significand, QNaNBit);

    // If there are no bits set in the payload, we have to set
    // *something* to make it a NaN instead of an infinity;
    // conventionally, this is the next bit down from the QNaN bit.
    જો (APInt::tcIsZero(significand, અંક_Parts))
      APInt::tcSetBit(significand, QNaNBit - 1);
  } નહીંતર જો (semantics->nanEncoding == fltNanEncoding::NegativeZero) {
    // The only NaN is a quiet NaN, and it has no bits sets in the significand.
    // Do nothing.
  } નહીંતર {
    // We always have to set the QNaN bit to make it a QNaN.
    APInt::tcSetBit(significand, QNaNBit);
  }

  // For x87 extended precision, we want to make a NaN, not a
  // pseudo-NaN.  Maybe we should expose the ability to make
  // pseudo-NaNs?
  જો (semantics == &APFloatBase::semX87DoubleExtended)
    APInt::tcSetBit(significand, QNaNBit + 1);
}

IEEEFloat &IEEEFloat::operator=(અચલ IEEEFloat &rhs) {
  જો (આ != &rhs) {
    જો (semantics != rhs.semantics) {
      freeSignificand();
      initialize(rhs.semantics);
    }
    assign(rhs);
  }

  પાછા *આ;
}

IEEEFloat &IEEEFloat::operator=(IEEEFloat &&rhs) {
  freeSignificand();

  semantics = rhs.semantics;
  significand = rhs.significand;
  exponent = rhs.exponent;
  category = rhs.category;
  sign = rhs.sign;

  rhs.semantics = &APFloatBase::semBogus;
  પાછા *આ;
}

તાર્કિક IEEEFloat::isDenormal() અચલ {
  પાછા getSemantics().hasDenormals && isFiniteNonZero() &&
         (exponent == semantics->minExponent) &&
         (APInt::tcExtractBit(significandParts(), semantics->precision - 1) ==
          0);
}

તાર્કિક IEEEFloat::isSmallest() અચલ {
  // The smallest number by magnitude in our format will be the smallest
  // denormal, i.e. the floating point number with exponent being minimum
  // exponent and significand bitwise equal to 1 (i.e. with MSB equal to 0).
  પાછા isFiniteNonZero() && exponent == semantics->minExponent &&
    significandMSB() == 0;
}

તાર્કિક IEEEFloat::isSmallestNormalized() અચલ {
  પાછા getCategory() == fcNormal && exponent == semantics->minExponent &&
         isSignificandAllZerosExceptMSB();
}

unsigned પૂર્ણાંક IEEEFloat::get_અંક_ઉપલું_Bits() અચલ {
  અચલ unsigned પૂર્ણાંક Part_ગણતરી = part_ગણતરી_For_Bits(semantics->precision);
  અચલ unsigned પૂર્ણાંક Bits = Part_ગણતરી * integerPartWidth;

  // Compute how many bits are used in the final word.
  // When precision is just 1, it represents the 'Pth'
  // Precision bit and not the actual significand bit.
  અચલ unsigned પૂર્ણાંક અંક_ઉપલું_Bits = (semantics->precision > 1)
                                       ? (Bits - semantics->precision + 1)
                                       : (Bits - semantics->precision);
  પાછા અંક_ઉપલું_Bits;
}

તાર્કિક IEEEFloat::isSignificandAllOnes() અચલ {
  // Test if the significand excluding the integral bit is all ones. This allows
  // us to test for binade boundaries.
  અચલ integerPart *Parts = significandParts();
  અચલ unsigned Part_ગણતરી = part_ગણતરી_For_Bits(semantics->precision);
  માટે (unsigned i = 0; i < Part_ગણતરી - 1; i++)
    જો (~Parts[i])
      પાછા ખોટું;

  // Set the unused high bits to all ones when we compare.
  અચલ unsigned અંક_ઉપલું_Bits = get_અંક_ઉપલું_Bits();
  પ્રતિજ્ઞા(અંક_ઉપલું_Bits <= integerPartWidth && અંક_ઉપલું_Bits > 0 &&
         "Can not have more high bits to fill than integerPartWidth");
  અચલ integerPart ઉપલું_Bit_Fill =
    ~integerPart(0) << (integerPartWidth - અંક_ઉપલું_Bits);
  જો ((semantics->precision <= 1) || (~(Parts[Part_ગણતરી - 1] | ઉપલું_Bit_Fill)))
    પાછા ખોટું;

  પાછા સાચું;
}

તાર્કિક IEEEFloat::isSignificandAllOnesExceptLSB() અચલ {
  // Test if the significand excluding the integral bit is all ones except for
  // the least significant bit.
  અચલ integerPart *Parts = significandParts();

  જો (Parts[0] & 1)
    પાછા ખોટું;

  અચલ unsigned Part_ગણતરી = part_ગણતરી_For_Bits(semantics->precision);
  માટે (unsigned i = 0; i < Part_ગણતરી - 1; i++) {
    જો (~Parts[i] & ~unsigned{!i})
      પાછા ખોટું;
  }

  // Set the unused high bits to all ones when we compare.
  અચલ unsigned અંક_ઉપલું_Bits = get_અંક_ઉપલું_Bits();
  પ્રતિજ્ઞા(અંક_ઉપલું_Bits <= integerPartWidth && અંક_ઉપલું_Bits > 0 &&
         "Can not have more high bits to fill than integerPartWidth");
  અચલ integerPart ઉપલું_Bit_Fill = ~integerPart(0)
                                  << (integerPartWidth - અંક_ઉપલું_Bits);
  જો (~(Parts[Part_ગણતરી - 1] | ઉપલું_Bit_Fill | 0x1))
    પાછા ખોટું;

  પાછા સાચું;
}

તાર્કિક IEEEFloat::isSignificandAllZeros() અચલ {
  // Test if the significand excluding the integral bit is all zeros. This
  // allows us to test for binade boundaries.
  અચલ integerPart *Parts = significandParts();
  અચલ unsigned Part_ગણતરી = part_ગણતરી_For_Bits(semantics->precision);

  માટે (unsigned i = 0; i < Part_ગણતરી - 1; i++)
    જો (Parts[i])
      પાછા ખોટું;

  // Compute how many bits are used in the final word.
  અચલ unsigned અંક_ઉપલું_Bits = get_અંક_ઉપલું_Bits();
  પ્રતિજ્ઞા(અંક_ઉપલું_Bits < integerPartWidth && "Can not have more high bits to "
         "clear than integerPartWidth");
  અચલ integerPart ઉપલું_Bit_Mask = ~integerPart(0) >> અંક_ઉપલું_Bits;

  જો ((semantics->precision > 1) && (Parts[Part_ગણતરી - 1] & ઉપલું_Bit_Mask))
    પાછા ખોટું;

  પાછા સાચું;
}

તાર્કિક IEEEFloat::isSignificandAllZerosExceptMSB() અચલ {
  અચલ integerPart *Parts = significandParts();
  અચલ unsigned Part_ગણતરી = part_ગણતરી_For_Bits(semantics->precision);

  માટે (unsigned i = 0; i < Part_ગણતરી - 1; i++) {
    જો (Parts[i])
      પાછા ખોટું;
  }

  અચલ unsigned અંક_ઉપલું_Bits = get_અંક_ઉપલું_Bits();
  અચલ integerPart MSBMask = integerPart(1)
                              << (integerPartWidth - અંક_ઉપલું_Bits);
  પાછા ((semantics->precision <= 1) || (Parts[Part_ગણતરી - 1] == MSBMask));
}

તાર્કિક IEEEFloat::isLargest() અચલ {
  તાર્કિક IsMaxExp = isFiniteNonZero() && exponent == semantics->maxExponent;
  જો (semantics->nonFiniteBehavior == fltNonfiniteBehavior::NanOnly &&
      semantics->nanEncoding == fltNanEncoding::AllOnes) {
    // The largest number by magnitude in our format will be the floating point
    // number with maximum exponent and with significand that is all ones except
    // the LSB.
    પાછા (IsMaxExp && APFloat::hasSignificand(*semantics))
               ? isSignificandAllOnesExceptLSB()
               : IsMaxExp;
  } નહીંતર {
    // The largest number by magnitude in our format will be the floating point
    // number with maximum exponent and with significand that is all ones.
    પાછા IsMaxExp && isSignificandAllOnes();
  }
}

તાર્કિક IEEEFloat::isInteger() અચલ {
  // This could be made more efficient; I'm going for obviously correct.
  જો (!isFinite()) પાછા ખોટું;
  IEEEFloat truncated = *આ;
  truncated.roundToIntegral(rmTowardZero);
  પાછા compare(truncated) == cmpEqual;
}

તાર્કિક IEEEFloat::bitwiseIsEqual(અચલ IEEEFloat &rhs) અચલ {
  જો (આ == &rhs)
    પાછા સાચું;
  જો (semantics != rhs.semantics ||
      category != rhs.category ||
      sign != rhs.sign)
    પાછા ખોટું;
  જો (category==fcZero || category==fcInfinity)
    પાછા સાચું;

  જો (isFiniteNonZero() && exponent != rhs.exponent)
    પાછા ખોટું;

  પાછા std::equal(significandParts(), significandParts() + part_ગણતરી(),
                    rhs.significandParts());
}

IEEEFloat::IEEEFloat(અચલ fltSemantics &ourSemantics, integerPart કિંમત_2) {
  initialize(&ourSemantics);
  sign = 0;
  category = fcNormal;
  zeroSignificand();
  exponent = ourSemantics.precision - 1;
  significandParts()[0] = કિંમત_2;
  normalize(rmNearestTiesToEven, lfExactlyZero);
}

IEEEFloat::IEEEFloat(અચલ fltSemantics &ourSemantics) {
  initialize(&ourSemantics);
  // The Float8E8MOFNU format does not have a representation
  // for zero. So, use the closest representation instead.
  // Moreover, the all-zero encoding represents a valid
  // normal value (which is the smallestNormalized here).
  // Hence, we call makeSmallestNormalized (where category is
  // 'fcNormal') instead of makeZero (where category is 'fcZero').
  ourSemantics.hasZero ? makeZero(ખોટું) : makeSmallestNormalized(ખોટું);
}

// Delegate to the previous constructor, because later copy constructor may
// actually inspects category, which can't be garbage.
IEEEFloat::IEEEFloat(અચલ fltSemantics &ourSemantics, uninitializedTag tag)
    : IEEEFloat(ourSemantics) {}

IEEEFloat::IEEEFloat(અચલ IEEEFloat &rhs) {
  initialize(rhs.semantics);
  assign(rhs);
}

IEEEFloat::IEEEFloat(IEEEFloat &&rhs) : semantics(&APFloatBase::semBogus) {
  *આ = std::move(rhs);
}

IEEEFloat::~IEEEFloat() { freeSignificand(); }

unsigned પૂર્ણાંક IEEEFloat::part_ગણતરી() અચલ {
  પાછા part_ગણતરી_For_Bits(semantics->precision + 1);
}

અચલ APFloat::integerPart *IEEEFloat::significandParts() અચલ {
  પાછા const_cast<IEEEFloat *>(આ)->significandParts();
}

APFloat::integerPart *IEEEFloat::significandParts() {
  જો (part_ગણતરી() > 1)
    પાછા significand.parts;
  નહીંતર
    પાછા &significand.part;
}

શૂન્ય IEEEFloat::zeroSignificand() {
  APInt::tcSet(significandParts(), 0, part_ગણતરી());
}

/* Increment an fcNormal floating point number's significand.  */
શૂન્ય IEEEFloat::incrementSignificand() {
  [[maybe_unused]] integerPart carry =
      APInt::tcIncrement(significandParts(), part_ગણતરી());

  /* Our callers should never cause us to overflow.  */
  પ્રતિજ્ઞા(carry == 0);
}

/* Add the significand of the RHS.  Returns the carry flag.  */
APFloat::integerPart IEEEFloat::addSignificand(અચલ IEEEFloat &rhs) {
  integerPart *parts = significandParts();

  પ્રતિજ્ઞા(semantics == rhs.semantics);
  પ્રતિજ્ઞા(exponent == rhs.exponent);

  પાછા APInt::tcAdd(parts, rhs.significandParts(), 0, part_ગણતરી());
}

/* Subtract the significand of the RHS with a borrow flag.  Returns
   the borrow flag.  */
APFloat::integerPart IEEEFloat::subtractSignificand(અચલ IEEEFloat &rhs,
                                                    integerPart borrow) {
  integerPart *parts = significandParts();

  પ્રતિજ્ઞા(semantics == rhs.semantics);
  પ્રતિજ્ઞા(exponent == rhs.exponent);

  પાછા APInt::tcSubtract(parts, rhs.significandParts(), borrow,
                           part_ગણતરી());
}

/* Multiply the significand of the RHS.  If ADDEND is non-NULL, add it
   on to the full-precision result of the multiplication.  Returns the
   lost fraction.  */
lostFraction IEEEFloat::multiplySignificand(અચલ IEEEFloat &rhs,
                                            IEEEFloat addend,
                                            તાર્કિક ignoreAddend) {
  integerPart scratch[4];
  તાર્કિક ignored;

  પ્રતિજ્ઞા(semantics == rhs.semantics);

  unsigned precision = semantics->precision;

  // Allocate space for twice as many bits as the original significand, plus one
  // extra bit for the addition to overflow into.
  unsigned new_Parts_ગણતરી = part_ગણતરી_For_Bits(precision * 2 + 1);

  // FIXME: Replace with SmallVector<4>.
  integerPart *fullSignificand =
      new_Parts_ગણતરી > 4 ? નવું integerPart[new_Parts_ગણતરી] : scratch;

  integerPart *lhsSignificand = significandParts();
  unsigned parts_ગણતરી = part_ગણતરી();

  APInt::tcFullMultiply(fullSignificand, lhsSignificand,
                        rhs.significandParts(), parts_ગણતરી, parts_ગણતરી);

  lostFraction lost_fraction = lfExactlyZero;
  // One, not zero, based MSB.
  unsigned omsb = APInt::tcMSB(fullSignificand, new_Parts_ગણતરી) + 1;
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

  જો (!ignoreAddend && addend.isNonZero()) {
    // The intermediate result of the multiplication has "2 * precision"
    // signicant bit; adjust the addend to be consistent with mul result.
    //
    Significand savedSignificand = significand;
    અચલ fltSemantics *savedSemantics = semantics;

    // Normalize our MSB to one below the top bit to allow for overflow.
    unsigned extendedPrecision = 2 * precision + 1;
    જો (omsb != extendedPrecision - 1) {
      પ્રતિજ્ઞા(extendedPrecision > omsb);
      APInt::tc_Shift_ડાબું(fullSignificand, new_Parts_ગણતરી,
                         (extendedPrecision - 1) - omsb);
      exponent -= (extendedPrecision - 1) - omsb;
    }

    /* Create new semantics.  */
    fltSemantics extendedSemantics = *semantics;
    extendedSemantics.precision = extendedPrecision;

    જો (new_Parts_ગણતરી == 1)
      significand.part = fullSignificand[0];
    નહીંતર
      significand.parts = fullSignificand;
    semantics = &extendedSemantics;

    // Make a copy so we can convert it to the extended semantics.
    // Note that we cannot convert the addend directly, as the extendedSemantics
    // is a local variable (which we take a reference to).
    IEEEFloat extendedAddend(addend);
    [[maybe_unused]] opStatus status = extendedAddend.convert(
        extendedSemantics, APFloat::rmTowardZero, &ignored);
    પ્રતિજ્ઞા(status == APFloat::opOK);

    // Shift the significand of the addend right by one bit. This guarantees
    // that the high bit of the significand is zero (same as fullSignificand),
    // so the addition will overflow (if it does overflow at all) into the top bit.
    lost_fraction = extendedAddend.shift_Significand_જમણું(1);
    પ્રતિજ્ઞા(lost_fraction == lfExactlyZero &&
           "Lost precision while shifting addend for fused-multiply-add.");

    lost_fraction = addOrSubtractSignificand(extendedAddend, ખોટું);

    /* Restore our state.  */
    જો (new_Parts_ગણતરી == 1)
      fullSignificand[0] = significand.part;
    significand = savedSignificand;
    semantics = savedSemantics;

    omsb = APInt::tcMSB(fullSignificand, new_Parts_ગણતરી) + 1;
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
  જો (omsb > precision) {
    unsigned પૂર્ણાંક bits, significantParts;
    lostFraction lf;

    bits = omsb - precision;
    significantParts = part_ગણતરી_For_Bits(omsb);
    lf = shift_જમણું(fullSignificand, significantParts, bits);
    lost_fraction = combineLostFractions(lf, lost_fraction);
    exponent += bits;
  }

  APInt::tcAssign(lhsSignificand, fullSignificand, parts_ગણતરી);

  જો (new_Parts_ગણતરી > 4)
    delete [] fullSignificand;

  પાછા lost_fraction;
}

lostFraction IEEEFloat::multiplySignificand(અચલ IEEEFloat &rhs) {
  // When the given semantics has zero, the addend here is a zero.
  // i.e . it belongs to the 'fcZero' category.
  // But when the semantics does not support zero, we need to
  // explicitly convey that this addend should be ignored
  // for multiplication.
  પાછા multiplySignificand(rhs, IEEEFloat(*semantics), !semantics->hasZero);
}

/* Multiply the significands of LHS and RHS to DST.  */
lostFraction IEEEFloat::divideSignificand(અચલ IEEEFloat &rhs) {
  integerPart scratch[4];

  પ્રતિજ્ઞા(semantics == rhs.semantics);

  integerPart *lhsSignificand = significandParts();
  અચલ integerPart *rhsSignificand = rhs.significandParts();
  unsigned parts_ગણતરી = part_ગણતરી();

  integerPart *dividend =
      parts_ગણતરી > 2 ? નવું integerPart[parts_ગણતરી * 2] : scratch;
  integerPart *ભાજક = dividend + parts_ગણતરી;

  /* Copy the dividend and divisor as they will be modified in-place.  */
  માટે (unsigned i = 0; i < parts_ગણતરી; i++) {
    dividend[i] = lhsSignificand[i];
    ભાજક[i] = rhsSignificand[i];
    lhsSignificand[i] = 0;
  }

  exponent -= rhs.exponent;

  unsigned પૂર્ણાંક precision = semantics->precision;

  /* Normalize the divisor.  */
  unsigned bit = precision - APInt::tcMSB(ભાજક, parts_ગણતરી) - 1;
  જો (bit) {
    exponent += bit;
    APInt::tc_Shift_ડાબું(ભાજક, parts_ગણતરી, bit);
  }

  /* Normalize the dividend.  */
  bit = precision - APInt::tcMSB(dividend, parts_ગણતરી) - 1;
  જો (bit) {
    exponent -= bit;
    APInt::tc_Shift_ડાબું(dividend, parts_ગણતરી, bit);
  }

  /* Ensure the dividend >= divisor initially for the loop below.
     Incidentally, this means that the division loop below is
     guaranteed to set the integer bit to one.  */
  જો (APInt::tcCompare(dividend, ભાજક, parts_ગણતરી) < 0) {
    exponent--;
    APInt::tc_Shift_ડાબું(dividend, parts_ગણતરી, 1);
    પ્રતિજ્ઞા(APInt::tcCompare(dividend, ભાજક, parts_ગણતરી) >= 0);
  }

  /* Long division.  */
  માટે (bit = precision; bit; bit -= 1) {
    જો (APInt::tcCompare(dividend, ભાજક, parts_ગણતરી) >= 0) {
      APInt::tcSubtract(dividend, ભાજક, 0, parts_ગણતરી);
      APInt::tcSetBit(lhsSignificand, bit - 1);
    }

    APInt::tc_Shift_ડાબું(dividend, parts_ગણતરી, 1);
  }

  /* Figure out the lost fraction.  */
  પૂર્ણાંક cmp = APInt::tcCompare(dividend, ભાજક, parts_ગણતરી);

  lostFraction lost_fraction;
  જો (cmp > 0)
    lost_fraction = lfMoreThanHalf;
  નહીંતર જો (cmp == 0)
    lost_fraction = lfExactlyHalf;
  નહીંતર જો (APInt::tcIsZero(dividend, parts_ગણતરી))
    lost_fraction = lfExactlyZero;
  નહીંતર
    lost_fraction = lfLessThanHalf;

  જો (parts_ગણતરી > 2)
    delete [] dividend;

  પાછા lost_fraction;
}

unsigned પૂર્ણાંક IEEEFloat::significandMSB() અચલ {
  પાછા APInt::tcMSB(significandParts(), part_ગણતરી());
}

unsigned પૂર્ણાંક IEEEFloat::significandLSB() અચલ {
  પાછા APInt::tcLSB(significandParts(), part_ગણતરી());
}

/* Note that a zero result is NOT normalized to fcZero.  */
lostFraction IEEEFloat::shift_Significand_જમણું(unsigned પૂર્ણાંક bits) {
  /* Our exponent should not overflow.  */
  પ્રતિજ્ઞા((ExponentType) (exponent + bits) >= exponent);

  exponent += bits;

  પાછા shift_જમણું(significandParts(), part_ગણતરી(), bits);
}

/* Shift the significand left BITS bits, subtract BITS from its exponent.  */
શૂન્ય IEEEFloat::shift_Significand_ડાબું(unsigned પૂર્ણાંક bits) {
  પ્રતિજ્ઞા(bits < semantics->precision ||
         (semantics->precision == 1 && bits <= 1));

  જો (bits) {
    unsigned પૂર્ણાંક parts_ગણતરી = part_ગણતરી();

    APInt::tc_Shift_ડાબું(significandParts(), parts_ગણતરી, bits);
    exponent -= bits;

    પ્રતિજ્ઞા(!APInt::tcIsZero(significandParts(), parts_ગણતરી));
  }
}

APFloat::cmpResult IEEEFloat::compare_Absolute_કિંમત(અચલ IEEEFloat &rhs) અચલ {
  પ્રતિજ્ઞા(semantics == rhs.semantics);
  પ્રતિજ્ઞા(isFiniteNonZero());
  પ્રતિજ્ઞા(rhs.isFiniteNonZero());

  પૂર્ણાંક compare = exponent - rhs.exponent;

  /* If exponents are equal, do an unsigned bignum comparison of the
     significands.  */
  જો (compare == 0)
    compare = APInt::tcCompare(significandParts(), rhs.significandParts(),
                               part_ગણતરી());

  જો (compare > 0)
    પાછા cmpGreaterThan;
  નહીંતર જો (compare < 0)
    પાછા cmpLessThan;
  નહીંતર
    પાછા cmpEqual;
}

/* Set the least significant BITS bits of a bignum, clear the
   rest.  */
સ્થિર શૂન્ય tcSetLeastSignificantBits(APInt::WordType *dst, unsigned parts,
                                      unsigned bits) {
  unsigned i = 0;
  જ્યારે (bits > APInt::APINT_BITS_PER_WORD) {
    dst[i++] = ~(APInt::WordType)0;
    bits -= APInt::APINT_BITS_PER_WORD;
  }

  જો (bits)
    dst[i++] = ~(APInt::WordType)0 >> (APInt::APINT_BITS_PER_WORD - bits);

  જ્યારે (i < parts)
    dst[i++] = 0;
}

/* Handle overflow.  Sign is preserved.  We either become infinity or
   the largest finite number.  */
APFloat::opStatus IEEEFloat::handleOverflow(roundingMode rounding_mode) {
  જો (semantics->nonFiniteBehavior != fltNonfiniteBehavior::FiniteOnly) {
    /* Infinity?  */
    જો (rounding_mode == rmNearestTiesToEven ||
        rounding_mode == rmNearestTiesToAway ||
        (rounding_mode == rmTowardPositive && !sign) ||
        (rounding_mode == rmTowardNegative && sign)) {
      જો (semantics->nonFiniteBehavior == fltNonfiniteBehavior::NanOnly)
        makeNaN(ખોટું, sign);
      નહીંતર
        category = fcInfinity;
      પાછા static_cast<opStatus>(opOverflow | opInexact);
    }
  }

  /* Otherwise we become the largest finite number.  */
  category = fcNormal;
  exponent = semantics->maxExponent;
  tcSetLeastSignificantBits(significandParts(), part_ગણતરી(),
                            semantics->precision);
  જો (semantics->nonFiniteBehavior == fltNonfiniteBehavior::NanOnly &&
      semantics->nanEncoding == fltNanEncoding::AllOnes)
    APInt::tcClearBit(significandParts(), 0);

  પાછા opInexact;
}

/* Returns TRUE if, when truncating the current number, with BIT the
   new LSB, with the given lost fraction and rounding mode, the result
   would need to be rounded away from zero (i.e., by increasing the
   signficand).  This routine must work for fcZero of both signs, and
   fcNormal numbers.  */
તાર્કિક IEEEFloat::roundAwayFromZero(roundingMode rounding_mode,
                                  lostFraction lost_fraction,
                                  unsigned પૂર્ણાંક bit) અચલ {
  /* NaNs and infinities should not have lost fractions.  */
  પ્રતિજ્ઞા(isFiniteNonZero() || category == fcZero);

  /* Current callers never pass this so we don't handle it.  */
  પ્રતિજ્ઞા(lost_fraction != lfExactlyZero);

  પસંદગી (rounding_mode) {
  સ્થિતિ rmNearestTiesToAway:
    પાછા lost_fraction == lfExactlyHalf || lost_fraction == lfMoreThanHalf;

  સ્થિતિ rmNearestTiesToEven:
    જો (lost_fraction == lfMoreThanHalf)
      પાછા સાચું;

    /* Our zeroes don't have a significand to test.  */
    જો (lost_fraction == lfExactlyHalf && category != fcZero)
      પાછા APInt::tcExtractBit(significandParts(), bit);

    પાછા ખોટું;

  સ્થિતિ rmTowardZero:
    પાછા ખોટું;

  સ્થિતિ rmTowardPositive:
    પાછા !sign;

  સ્થિતિ rmTowardNegative:
    પાછા sign;

  મૂળભૂત:
    તોડો;
  }
  llvm_unreachable("Invalid rounding mode found");
}

APFloat::opStatus IEEEFloat::normalize(roundingMode rounding_mode,
                                       lostFraction lost_fraction) {
  જો (!isFiniteNonZero())
    પાછા opOK;

  /* Before rounding normalize the exponent of fcNormal numbers.  */
  /* One, not zero, based MSB.  */
  unsigned omsb = significandMSB() + 1;

  // Only skip this `if` if the value is exactly zero.
  જો (omsb || lost_fraction != lfExactlyZero) {
    /* OMSB is numbered from 1.  We want to place it in the integer
       bit numbered PRECISION if possible, with a compensating change in
       the exponent.  */
    પૂર્ણાંક exponentChange = omsb - semantics->precision;

    /* If the resulting exponent is too high, overflow according to
       the rounding mode.  */
    જો (exponent + exponentChange > semantics->maxExponent)
      પાછા handleOverflow(rounding_mode);

    /* Subnormal numbers have exponent minExponent, and their MSB
       is forced based on that.  */
    જો (exponent + exponentChange < semantics->minExponent)
      exponentChange = semantics->minExponent - exponent;

    /* Shifting left is easy as we don't lose precision.  */
    જો (exponentChange < 0) {
      પ્રતિજ્ઞા(lost_fraction == lfExactlyZero);

      shift_Significand_ડાબું(-exponentChange);

      પાછા opOK;
    }

    જો (exponentChange > 0) {
      lostFraction lf;

      /* Shift right and capture any new lost fraction.  */
      lf = shift_Significand_જમણું(exponentChange);

      lost_fraction = combineLostFractions(lf, lost_fraction);

      /* Keep OMSB up-to-date.  */
      જો (omsb > (unsigned) exponentChange)
        omsb -= exponentChange;
      નહીંતર
        omsb = 0;
    }
  }

  // The all-ones values is an overflow if NaN is all ones. If NaN is
  // represented by negative zero, then it is a valid finite value.
  જો (semantics->nonFiniteBehavior == fltNonfiniteBehavior::NanOnly &&
      semantics->nanEncoding == fltNanEncoding::AllOnes &&
      exponent == semantics->maxExponent && isSignificandAllOnes())
    પાછા handleOverflow(rounding_mode);

  /* Now round the number according to rounding_mode given the lost
     fraction.  */

  /* As specified in IEEE 754, since we do not trap we do not report
     underflow for exact results.  */
  જો (lost_fraction == lfExactlyZero) {
    /* Canonicalize zeroes.  */
    જો (omsb == 0) {
      category = fcZero;
      જો (semantics->nanEncoding == fltNanEncoding::NegativeZero)
        sign = ખોટું;
      જો (!semantics->hasZero)
        makeSmallestNormalized(ખોટું);
    }

    પાછા opOK;
  }

  /* Increment the significand if we're rounding away from zero.  */
  જો (roundAwayFromZero(rounding_mode, lost_fraction, 0)) {
    જો (omsb == 0)
      exponent = semantics->minExponent;

    incrementSignificand();
    omsb = significandMSB() + 1;

    /* Did the significand increment overflow?  */
    જો (omsb == (unsigned) semantics->precision + 1) {
      /* Renormalize by incrementing the exponent and shifting our
         significand right one.  However if we already have the
         maximum exponent we overflow to infinity.  */
      જો (exponent == semantics->maxExponent)
        // Invoke overflow handling with a rounding mode that will guarantee
        // that the result gets turned into the correct infinity representation.
        // This is needed instead of just setting the category to infinity to
        // account for 8-bit floating point types that have no inf, only NaN.
        પાછા handleOverflow(sign ? rmTowardNegative : rmTowardPositive);

      shift_Significand_જમણું(1);

      પાછા opInexact;
    }

    // The all-ones values is an overflow if NaN is all ones. If NaN is
    // represented by negative zero, then it is a valid finite value.
    જો (semantics->nonFiniteBehavior == fltNonfiniteBehavior::NanOnly &&
        semantics->nanEncoding == fltNanEncoding::AllOnes &&
        exponent == semantics->maxExponent && isSignificandAllOnes())
      પાછા handleOverflow(rounding_mode);
  }

  /* The normal case - we were and are not denormal, and any
     significand increment above didn't overflow.  */
  જો (omsb == semantics->precision)
    પાછા opInexact;

  /* We have a non-zero denormal.  */
  પ્રતિજ્ઞા(omsb < semantics->precision);

  /* Canonicalize zeroes.  */
  જો (omsb == 0) {
    category = fcZero;
    જો (semantics->nanEncoding == fltNanEncoding::NegativeZero)
      sign = ખોટું;
    // This condition handles the case where the semantics
    // does not have zero but uses the all-zero encoding
    // to represent the smallest normal value.
    જો (!semantics->hasZero)
      makeSmallestNormalized(ખોટું);
  }

  /* The fcZero case is a denormal that underflowed to zero.  */
  પાછા (opStatus) (opUnderflow | opInexact);
}

APFloat::opStatus IEEEFloat::addOrSubtractSpecials(અચલ IEEEFloat &rhs,
                                                   તાર્કિક subtract) {
  પસંદગી (Pack_Categories_Into_ચાવી(category, rhs.category)) {
  મૂળભૂત:
    llvm_unreachable(nullptr);

  સ્થિતિ Pack_Categories_Into_ચાવી(fcZero, fcNaN):
  સ્થિતિ Pack_Categories_Into_ચાવી(fcNormal, fcNaN):
  સ્થિતિ Pack_Categories_Into_ચાવી(fcInfinity, fcNaN):
    assign(rhs);
    [[fallthrough]];
  સ્થિતિ Pack_Categories_Into_ચાવી(fcNaN, fcZero):
  સ્થિતિ Pack_Categories_Into_ચાવી(fcNaN, fcNormal):
  સ્થિતિ Pack_Categories_Into_ચાવી(fcNaN, fcInfinity):
  સ્થિતિ Pack_Categories_Into_ચાવી(fcNaN, fcNaN):
    જો (isSignaling()) {
      makeQuiet();
      પાછા opInvalidOp;
    }
    પાછા rhs.isSignaling() ? opInvalidOp : opOK;

  સ્થિતિ Pack_Categories_Into_ચાવી(fcNormal, fcZero):
  સ્થિતિ Pack_Categories_Into_ચાવી(fcInfinity, fcNormal):
  સ્થિતિ Pack_Categories_Into_ચાવી(fcInfinity, fcZero):
    પાછા opOK;

  સ્થિતિ Pack_Categories_Into_ચાવી(fcNormal, fcInfinity):
  સ્થિતિ Pack_Categories_Into_ચાવી(fcZero, fcInfinity):
    category = fcInfinity;
    sign = rhs.sign ^ subtract;
    પાછા opOK;

  સ્થિતિ Pack_Categories_Into_ચાવી(fcZero, fcNormal):
    assign(rhs);
    sign = rhs.sign ^ subtract;
    પાછા opOK;

  સ્થિતિ Pack_Categories_Into_ચાવી(fcZero, fcZero):
    /* Sign depends on rounding mode; handled by caller.  */
    પાછા opOK;

  સ્થિતિ Pack_Categories_Into_ચાવી(fcInfinity, fcInfinity):
    /* Differently signed infinities can only be validly
       subtracted.  */
    જો (((sign ^ rhs.sign)!=0) != subtract) {
      makeNaN();
      પાછા opInvalidOp;
    }

    પાછા opOK;

  સ્થિતિ Pack_Categories_Into_ચાવી(fcNormal, fcNormal):
    પાછા opDivByZero;
  }
}

/* Add or subtract two normal numbers.  */
lostFraction IEEEFloat::addOrSubtractSignificand(અચલ IEEEFloat &rhs,
                                                 તાર્કિક subtract) {
  [[maybe_unused]] integerPart carry = 0;
  lostFraction lost_fraction;

  /* Determine if the operation on the absolute values is effectively
     an addition or subtraction.  */
  subtract ^= static_cast<તાર્કિક>(sign ^ rhs.sign);

  /* Are we bigger exponent-wise than the RHS?  */
  પૂર્ણાંક bits = exponent - rhs.exponent;

  /* Subtraction is more subtle than one might naively expect.  */
  જો (subtract) {
    જો ((bits < 0) && !semantics->hasSignedRepr)
      llvm_unreachable(
          "This floating point format does not support signed values");

    IEEEFloat temp_rhs(rhs);
    તાર્કિક lost_fraction_is_from_rhs = ખોટું;

    જો (bits == 0)
      lost_fraction = lfExactlyZero;
    નહીંતર જો (bits > 0) {
      lost_fraction = temp_rhs.shift_Significand_જમણું(bits - 1);
      lost_fraction_is_from_rhs = સાચું;
      shift_Significand_ડાબું(1);
    } નહીંતર {
      lost_fraction = shift_Significand_જમણું(-bits - 1);
      temp_rhs.shift_Significand_ડાબું(1);
    }

    // Should we reverse the subtraction.
    cmpResult cmp_result = compare_Absolute_કિંમત(temp_rhs);
    જો (cmp_result == cmpLessThan) {
      તાર્કિક borrow =
          lost_fraction != lfExactlyZero && !lost_fraction_is_from_rhs;
      જો (borrow) {
        // The lost fraction is being subtracted, borrow from the significand
        // and invert `lost_fraction`.
        જો (lost_fraction == lfLessThanHalf)
          lost_fraction = lfMoreThanHalf;
        નહીંતર જો (lost_fraction == lfMoreThanHalf)
          lost_fraction = lfLessThanHalf;
      }
      carry = temp_rhs.subtractSignificand(*આ, borrow);
      copySignificand(temp_rhs);
      sign = !sign;
    } નહીંતર જો (cmp_result == cmpGreaterThan) {
      તાર્કિક borrow = lost_fraction != lfExactlyZero && lost_fraction_is_from_rhs;
      જો (borrow) {
        // The lost fraction is being subtracted, borrow from the significand
        // and invert `lost_fraction`.
        જો (lost_fraction == lfLessThanHalf)
          lost_fraction = lfMoreThanHalf;
        નહીંતર જો (lost_fraction == lfMoreThanHalf)
          lost_fraction = lfLessThanHalf;
      }
      carry = subtractSignificand(temp_rhs, borrow);
    } નહીંતર { // cmpEqual
      zeroSignificand();
      જો (lost_fraction != lfExactlyZero && lost_fraction_is_from_rhs) {
        // rhs is slightly larger due to the lost fraction, flip the sign.
        sign = !sign;
      }
    }

    /* The code above is intended to ensure that no borrow is
       necessary.  */
    પ્રતિજ્ઞા(!carry);
  } નહીંતર {
    જો (bits > 0) {
      IEEEFloat temp_rhs(rhs);

      lost_fraction = temp_rhs.shift_Significand_જમણું(bits);
      carry = addSignificand(temp_rhs);
    } નહીંતર {
      lost_fraction = shift_Significand_જમણું(-bits);
      carry = addSignificand(rhs);
    }

    /* We have a guard bit; generating a carry cannot happen.  */
    પ્રતિજ્ઞા(!carry);
  }

  પાછા lost_fraction;
}

APFloat::opStatus IEEEFloat::multiplySpecials(અચલ IEEEFloat &rhs) {
  પસંદગી (Pack_Categories_Into_ચાવી(category, rhs.category)) {
  મૂળભૂત:
    llvm_unreachable(nullptr);

  સ્થિતિ Pack_Categories_Into_ચાવી(fcZero, fcNaN):
  સ્થિતિ Pack_Categories_Into_ચાવી(fcNormal, fcNaN):
  સ્થિતિ Pack_Categories_Into_ચાવી(fcInfinity, fcNaN):
    assign(rhs);
    sign = ખોટું;
    [[fallthrough]];
  સ્થિતિ Pack_Categories_Into_ચાવી(fcNaN, fcZero):
  સ્થિતિ Pack_Categories_Into_ચાવી(fcNaN, fcNormal):
  સ્થિતિ Pack_Categories_Into_ચાવી(fcNaN, fcInfinity):
  સ્થિતિ Pack_Categories_Into_ચાવી(fcNaN, fcNaN):
    sign ^= rhs.sign; // restore the original sign
    જો (isSignaling()) {
      makeQuiet();
      પાછા opInvalidOp;
    }
    પાછા rhs.isSignaling() ? opInvalidOp : opOK;

  સ્થિતિ Pack_Categories_Into_ચાવી(fcNormal, fcInfinity):
  સ્થિતિ Pack_Categories_Into_ચાવી(fcInfinity, fcNormal):
  સ્થિતિ Pack_Categories_Into_ચાવી(fcInfinity, fcInfinity):
    category = fcInfinity;
    પાછા opOK;

  સ્થિતિ Pack_Categories_Into_ચાવી(fcZero, fcNormal):
  સ્થિતિ Pack_Categories_Into_ચાવી(fcNormal, fcZero):
  સ્થિતિ Pack_Categories_Into_ચાવી(fcZero, fcZero):
    category = fcZero;
    પાછા opOK;

  સ્થિતિ Pack_Categories_Into_ચાવી(fcZero, fcInfinity):
  સ્થિતિ Pack_Categories_Into_ચાવી(fcInfinity, fcZero):
    makeNaN();
    પાછા opInvalidOp;

  સ્થિતિ Pack_Categories_Into_ચાવી(fcNormal, fcNormal):
    પાછા opOK;
  }
}

APFloat::opStatus IEEEFloat::divideSpecials(અચલ IEEEFloat &rhs) {
  પસંદગી (Pack_Categories_Into_ચાવી(category, rhs.category)) {
  મૂળભૂત:
    llvm_unreachable(nullptr);

  સ્થિતિ Pack_Categories_Into_ચાવી(fcZero, fcNaN):
  સ્થિતિ Pack_Categories_Into_ચાવી(fcNormal, fcNaN):
  સ્થિતિ Pack_Categories_Into_ચાવી(fcInfinity, fcNaN):
    assign(rhs);
    sign = ખોટું;
    [[fallthrough]];
  સ્થિતિ Pack_Categories_Into_ચાવી(fcNaN, fcZero):
  સ્થિતિ Pack_Categories_Into_ચાવી(fcNaN, fcNormal):
  સ્થિતિ Pack_Categories_Into_ચાવી(fcNaN, fcInfinity):
  સ્થિતિ Pack_Categories_Into_ચાવી(fcNaN, fcNaN):
    sign ^= rhs.sign; // restore the original sign
    જો (isSignaling()) {
      makeQuiet();
      પાછા opInvalidOp;
    }
    પાછા rhs.isSignaling() ? opInvalidOp : opOK;

  સ્થિતિ Pack_Categories_Into_ચાવી(fcInfinity, fcZero):
  સ્થિતિ Pack_Categories_Into_ચાવી(fcInfinity, fcNormal):
  સ્થિતિ Pack_Categories_Into_ચાવી(fcZero, fcInfinity):
  સ્થિતિ Pack_Categories_Into_ચાવી(fcZero, fcNormal):
    પાછા opOK;

  સ્થિતિ Pack_Categories_Into_ચાવી(fcNormal, fcInfinity):
    category = fcZero;
    પાછા opOK;

  સ્થિતિ Pack_Categories_Into_ચાવી(fcNormal, fcZero):
    જો (semantics->nonFiniteBehavior == fltNonfiniteBehavior::NanOnly)
      makeNaN(ખોટું, sign);
    નહીંતર
      category = fcInfinity;
    પાછા opDivByZero;

  સ્થિતિ Pack_Categories_Into_ચાવી(fcInfinity, fcInfinity):
  સ્થિતિ Pack_Categories_Into_ચાવી(fcZero, fcZero):
    makeNaN();
    પાછા opInvalidOp;

  સ્થિતિ Pack_Categories_Into_ચાવી(fcNormal, fcNormal):
    પાછા opOK;
  }
}

APFloat::opStatus IEEEFloat::modSpecials(અચલ IEEEFloat &rhs) {
  પસંદગી (Pack_Categories_Into_ચાવી(category, rhs.category)) {
  મૂળભૂત:
    llvm_unreachable(nullptr);

  સ્થિતિ Pack_Categories_Into_ચાવી(fcZero, fcNaN):
  સ્થિતિ Pack_Categories_Into_ચાવી(fcNormal, fcNaN):
  સ્થિતિ Pack_Categories_Into_ચાવી(fcInfinity, fcNaN):
    assign(rhs);
    [[fallthrough]];
  સ્થિતિ Pack_Categories_Into_ચાવી(fcNaN, fcZero):
  સ્થિતિ Pack_Categories_Into_ચાવી(fcNaN, fcNormal):
  સ્થિતિ Pack_Categories_Into_ચાવી(fcNaN, fcInfinity):
  સ્થિતિ Pack_Categories_Into_ચાવી(fcNaN, fcNaN):
    જો (isSignaling()) {
      makeQuiet();
      પાછા opInvalidOp;
    }
    પાછા rhs.isSignaling() ? opInvalidOp : opOK;

  સ્થિતિ Pack_Categories_Into_ચાવી(fcZero, fcInfinity):
  સ્થિતિ Pack_Categories_Into_ચાવી(fcZero, fcNormal):
  સ્થિતિ Pack_Categories_Into_ચાવી(fcNormal, fcInfinity):
    પાછા opOK;

  સ્થિતિ Pack_Categories_Into_ચાવી(fcNormal, fcZero):
  સ્થિતિ Pack_Categories_Into_ચાવી(fcInfinity, fcZero):
  સ્થિતિ Pack_Categories_Into_ચાવી(fcInfinity, fcNormal):
  સ્થિતિ Pack_Categories_Into_ચાવી(fcInfinity, fcInfinity):
  સ્થિતિ Pack_Categories_Into_ચાવી(fcZero, fcZero):
    makeNaN();
    પાછા opInvalidOp;

  સ્થિતિ Pack_Categories_Into_ચાવી(fcNormal, fcNormal):
    પાછા opOK;
  }
}

APFloat::opStatus IEEEFloat::remainderSpecials(અચલ IEEEFloat &rhs) {
  પસંદગી (Pack_Categories_Into_ચાવી(category, rhs.category)) {
  મૂળભૂત:
    llvm_unreachable(nullptr);

  સ્થિતિ Pack_Categories_Into_ચાવી(fcZero, fcNaN):
  સ્થિતિ Pack_Categories_Into_ચાવી(fcNormal, fcNaN):
  સ્થિતિ Pack_Categories_Into_ચાવી(fcInfinity, fcNaN):
    assign(rhs);
    [[fallthrough]];
  સ્થિતિ Pack_Categories_Into_ચાવી(fcNaN, fcZero):
  સ્થિતિ Pack_Categories_Into_ચાવી(fcNaN, fcNormal):
  સ્થિતિ Pack_Categories_Into_ચાવી(fcNaN, fcInfinity):
  સ્થિતિ Pack_Categories_Into_ચાવી(fcNaN, fcNaN):
    જો (isSignaling()) {
      makeQuiet();
      પાછા opInvalidOp;
    }
    પાછા rhs.isSignaling() ? opInvalidOp : opOK;

  સ્થિતિ Pack_Categories_Into_ચાવી(fcZero, fcInfinity):
  સ્થિતિ Pack_Categories_Into_ચાવી(fcZero, fcNormal):
  સ્થિતિ Pack_Categories_Into_ચાવી(fcNormal, fcInfinity):
    પાછા opOK;

  સ્થિતિ Pack_Categories_Into_ચાવી(fcNormal, fcZero):
  સ્થિતિ Pack_Categories_Into_ચાવી(fcInfinity, fcZero):
  સ્થિતિ Pack_Categories_Into_ચાવી(fcInfinity, fcNormal):
  સ્થિતિ Pack_Categories_Into_ચાવી(fcInfinity, fcInfinity):
  સ્થિતિ Pack_Categories_Into_ચાવી(fcZero, fcZero):
    makeNaN();
    પાછા opInvalidOp;

  સ્થિતિ Pack_Categories_Into_ચાવી(fcNormal, fcNormal):
    પાછા opDivByZero; // fake status, indicating this is not a special case
  }
}

/* Change sign.  */
શૂન્ય IEEEFloat::changeSign() {
  // With NaN-as-negative-zero, neither NaN or negative zero can change
  // their signs.
  જો (semantics->nanEncoding == fltNanEncoding::NegativeZero &&
      (isZero() || isNaN()))
    પાછા;
  /* Look mummy, this one's easy.  */
  sign = !sign;
}

/* Normalized addition or subtraction.  */
APFloat::opStatus IEEEFloat::addOrSubtract(અચલ IEEEFloat &rhs,
                                           roundingMode rounding_mode,
                                           તાર્કિક subtract) {
  opStatus fs = addOrSubtractSpecials(rhs, subtract);

  /* This return code means it was not a simple case.  */
  જો (fs == opDivByZero) {
    lostFraction lost_fraction;

    lost_fraction = addOrSubtractSignificand(rhs, subtract);
    fs = normalize(rounding_mode, lost_fraction);

    /* Can only be zero if we lost no fraction.  */
    પ્રતિજ્ઞા(category != fcZero || lost_fraction == lfExactlyZero);
  }

  /* If two numbers add (exactly) to zero, IEEE 754 decrees it is a
     positive zero unless rounding to minus infinity, except that
     adding two like-signed zeroes gives that zero.  */
  જો (category == fcZero) {
    જો (rhs.category != fcZero || (sign == rhs.sign) == subtract)
      sign = (rounding_mode == rmTowardNegative);
    // NaN-in-negative-zero means zeros need to be normalized to +0.
    જો (semantics->nanEncoding == fltNanEncoding::NegativeZero)
      sign = ખોટું;
  }

  પાછા fs;
}

/* Normalized addition.  */
APFloat::opStatus IEEEFloat::add(અચલ IEEEFloat &rhs,
                                 roundingMode rounding_mode) {
  પાછા addOrSubtract(rhs, rounding_mode, ખોટું);
}

/* Normalized subtraction.  */
APFloat::opStatus IEEEFloat::subtract(અચલ IEEEFloat &rhs,
                                      roundingMode rounding_mode) {
  પાછા addOrSubtract(rhs, rounding_mode, સાચું);
}

/* Normalized multiply.  */
APFloat::opStatus IEEEFloat::multiply(અચલ IEEEFloat &rhs,
                                      roundingMode rounding_mode) {
  sign ^= rhs.sign;
  opStatus fs = multiplySpecials(rhs);

  જો (isZero() && semantics->nanEncoding == fltNanEncoding::NegativeZero)
    sign = ખોટું;
  જો (isFiniteNonZero()) {
    lostFraction lost_fraction = multiplySignificand(rhs);
    fs = normalize(rounding_mode, lost_fraction);
    જો (lost_fraction != lfExactlyZero)
      fs = (opStatus) (fs | opInexact);
  }

  પાછા fs;
}

/* Normalized divide.  */
APFloat::opStatus IEEEFloat::divide(અચલ IEEEFloat &rhs,
                                    roundingMode rounding_mode) {
  sign ^= rhs.sign;
  opStatus fs = divideSpecials(rhs);

  જો (isZero() && semantics->nanEncoding == fltNanEncoding::NegativeZero)
    sign = ખોટું;
  જો (isFiniteNonZero()) {
    lostFraction lost_fraction = divideSignificand(rhs);
    fs = normalize(rounding_mode, lost_fraction);
    જો (lost_fraction != lfExactlyZero)
      fs = (opStatus) (fs | opInexact);
  }

  પાછા fs;
}

/* Normalized remainder.  */
APFloat::opStatus IEEEFloat::remainder(અચલ IEEEFloat &rhs) {
  unsigned પૂર્ણાંક origSign = sign;

  // First handle the special cases.
  opStatus fs = remainderSpecials(rhs);
  જો (fs != opDivByZero)
    પાછા fs;

  fs = opOK;

  // Make sure the current value is less than twice the denom. If the addition
  // did not succeed (an overflow has happened), which means that the finite
  // value we currently posses must be less than twice the denom (as we are
  // using the same semantics).
  IEEEFloat P2 = rhs;
  જો (P2.add(rhs, rmNearestTiesToEven) == opOK) {
    fs = mod(P2);
    પ્રતિજ્ઞા(fs == opOK);
  }

  // Lets work with absolute numbers.
  IEEEFloat P = rhs;
  P.sign = ખોટું;
  sign = ખોટું;

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
  તાર્કિક losesInfo;
  fltSemantics extendedSemantics = *semantics;
  extendedSemantics.maxExponent++;
  extendedSemantics.minExponent--;
  extendedSemantics.precision += 2;

  IEEEFloat VEx = *આ;
  fs = VEx.convert(extendedSemantics, rmNearestTiesToEven, &losesInfo);
  પ્રતિજ્ઞા(fs == opOK && !losesInfo);
  IEEEFloat PEx = P;
  fs = PEx.convert(extendedSemantics, rmNearestTiesToEven, &losesInfo);
  પ્રતિજ્ઞા(fs == opOK && !losesInfo);

  // It is simpler to work with 2x instead of 0.5p, and we do not need to lose
  // any fraction.
  fs = VEx.add(VEx, rmNearestTiesToEven);
  પ્રતિજ્ઞા(fs == opOK);

  જો (VEx.compare(PEx) == cmpGreaterThan) {
    fs = subtract(P, rmNearestTiesToEven);
    પ્રતિજ્ઞા(fs == opOK);

    // Make VEx = this.add(this), but because we have different semantics, we do
    // not want to `convert` again, so we just subtract PEx twice (which equals
    // to the desired value).
    fs = VEx.subtract(PEx, rmNearestTiesToEven);
    પ્રતિજ્ઞા(fs == opOK);
    fs = VEx.subtract(PEx, rmNearestTiesToEven);
    પ્રતિજ્ઞા(fs == opOK);

    cmpResult result = VEx.compare(PEx);
    જો (result == cmpGreaterThan || result == cmpEqual) {
      fs = subtract(P, rmNearestTiesToEven);
      પ્રતિજ્ઞા(fs == opOK);
    }
  }

  જો (isZero()) {
    sign = origSign;    // IEEE754 requires this
    જો (semantics->nanEncoding == fltNanEncoding::NegativeZero)
      // But some 8-bit floats only have positive 0.
      sign = ખોટું;
  } નહીંતર {
    sign ^= origSign;
  }
  પાછા fs;
}

/* Normalized llvm frem (C fmod). */
APFloat::opStatus IEEEFloat::mod(અચલ IEEEFloat &rhs) {
  opStatus fs = modSpecials(rhs);
  unsigned પૂર્ણાંક origSign = sign;

  જ્યારે (isFiniteNonZero() && rhs.isFiniteNonZero() &&
         compare_Absolute_કિંમત(rhs) != cmpLessThan) {
    પૂર્ણાંક Exp = ilogb(*આ) - ilogb(rhs);
    IEEEFloat V = scalbn(rhs, Exp, rmNearestTiesToEven);
    // V can overflow to NaN with fltNonfiniteBehavior::NanOnly, so explicitly
    // check for it.
    જો (V.isNaN() || compare_Absolute_કિંમત(V) == cmpLessThan)
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
    જો (!semantics->hasZero && આ->isSmallest())
      તોડો;

    પ્રતિજ્ઞા(fs==opOK);
  }
  જો (isZero()) {
    sign = origSign; // fmod requires this
    જો (semantics->nanEncoding == fltNanEncoding::NegativeZero)
      sign = ખોટું;
  }
  પાછા fs;
}

/* Normalized fused-multiply-add.  */
APFloat::opStatus IEEEFloat::fusedMultiplyAdd(અચલ IEEEFloat &multiplicand,
                                              અચલ IEEEFloat &addend,
                                              roundingMode rounding_mode) {
  opStatus fs;

  /* Post-multiplication sign, before addition.  */
  sign ^= multiplicand.sign;

  /* If and only if all arguments are normal do we need to do an
     extended-precision calculation.  */
  જો (isFiniteNonZero() &&
      multiplicand.isFiniteNonZero() &&
      addend.isFinite()) {
    lostFraction lost_fraction;

    lost_fraction = multiplySignificand(multiplicand, addend);
    fs = normalize(rounding_mode, lost_fraction);
    જો (lost_fraction != lfExactlyZero)
      fs = (opStatus) (fs | opInexact);

    /* If two numbers add (exactly) to zero, IEEE 754 decrees it is a
       positive zero unless rounding to minus infinity, except that
       adding two like-signed zeroes gives that zero.  */
    જો (category == fcZero && !(fs & opUnderflow) && sign != addend.sign) {
      sign = (rounding_mode == rmTowardNegative);
      જો (semantics->nanEncoding == fltNanEncoding::NegativeZero)
        sign = ખોટું;
    }
  } નહીંતર {
    fs = multiplySpecials(multiplicand);

    /* FS can only be opOK or opInvalidOp.  There is no more work
       to do in the latter case.  The IEEE-754R standard says it is
       implementation-defined in this case whether, if ADDEND is a
       quiet NaN, we raise invalid op; this implementation does so.

       If we need to do the addition we can do so with normal
       precision.  */
    જો (fs == opOK)
      fs = addOrSubtract(addend, rounding_mode, ખોટું);
  }

  પાછા fs;
}

/* Rounding-mode correct round to integral value.  */
APFloat::opStatus IEEEFloat::roundToIntegral(roundingMode rounding_mode) {
  જો (isInfinity())
    // [IEEE Std 754-2008 6.1]:
    // The behavior of infinity in floating-point arithmetic is derived from the
    // limiting cases of real arithmetic with operands of arbitrarily
    // large magnitude, when such a limit exists.
    // ...
    // Operations on infinite operands are usually exact and therefore signal no
    // exceptions ...
    પાછા opOK;

  જો (isNaN()) {
    જો (isSignaling()) {
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
      પાછા opInvalidOp;
    } નહીંતર {
      // [IEEE Std 754-2008 6.2]:
      // For an operation with quiet NaN inputs, other than maximum and minimum
      // operations, if a floating-point result is to be delivered the result
      // shall be a quiet NaN which should be one of the input NaNs.
      // ...
      // Every general-computational and quiet-computational operation involving
      // one or more input NaNs, none of them signaling, shall signal no
      // exception, except fusedMultiplyAdd might signal the invalid operation
      // exception(see 7.2).
      પાછા opOK;
    }
  }

  જો (isZero()) {
    // [IEEE Std 754-2008 6.3]:
    // ... the sign of the result of conversions, the quantize operation, the
    // roundToIntegral operations, and the roundToIntegralExact(see 5.3.1) is
    // the sign of the first or only operand.
    પાછા opOK;
  }

  // If the exponent is large enough, we know that this value is already
  // integral, and the arithmetic below would potentially cause it to saturate
  // to +/-Inf.  Bail out early instead.
  જો (exponent + 1 >= (પૂર્ણાંક)APFloat::semanticsPrecision(*semantics))
    પાછા opOK;

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
  opStatus fs = MagicConstant.convertFromAPInt(IntegerConstant, ખોટું,
                                               rmNearestTiesToEven);
  પ્રતિજ્ઞા(fs == opOK);
  MagicConstant.sign = sign;

  // Preserve the input sign so that we can handle the case of zero result
  // correctly.
  તાર્કિક આગત_Sign = isNegative();

  fs = add(MagicConstant, rounding_mode);

  // Current value and 'MagicConstant' are both integers, so the result of the
  // subtraction is always exact according to Sterbenz' lemma.
  subtract(MagicConstant, rounding_mode);

  // Restore the input sign.
  જો (આગત_Sign != isNegative())
    changeSign();

  પાછા fs;
}

/* Comparison requires normalized numbers.  */
APFloat::cmpResult IEEEFloat::compare(અચલ IEEEFloat &rhs) અચલ {
  પ્રતિજ્ઞા(semantics == rhs.semantics);

  પસંદગી (Pack_Categories_Into_ચાવી(category, rhs.category)) {
  મૂળભૂત:
    llvm_unreachable(nullptr);

  સ્થિતિ Pack_Categories_Into_ચાવી(fcNaN, fcZero):
  સ્થિતિ Pack_Categories_Into_ચાવી(fcNaN, fcNormal):
  સ્થિતિ Pack_Categories_Into_ચાવી(fcNaN, fcInfinity):
  સ્થિતિ Pack_Categories_Into_ચાવી(fcNaN, fcNaN):
  સ્થિતિ Pack_Categories_Into_ચાવી(fcZero, fcNaN):
  સ્થિતિ Pack_Categories_Into_ચાવી(fcNormal, fcNaN):
  સ્થિતિ Pack_Categories_Into_ચાવી(fcInfinity, fcNaN):
    પાછા cmpUnordered;

  સ્થિતિ Pack_Categories_Into_ચાવી(fcInfinity, fcNormal):
  સ્થિતિ Pack_Categories_Into_ચાવી(fcInfinity, fcZero):
  સ્થિતિ Pack_Categories_Into_ચાવી(fcNormal, fcZero):
    જો (sign)
      પાછા cmpLessThan;
    નહીંતર
      પાછા cmpGreaterThan;

  સ્થિતિ Pack_Categories_Into_ચાવી(fcNormal, fcInfinity):
  સ્થિતિ Pack_Categories_Into_ચાવી(fcZero, fcInfinity):
  સ્થિતિ Pack_Categories_Into_ચાવી(fcZero, fcNormal):
    જો (rhs.sign)
      પાછા cmpGreaterThan;
    નહીંતર
      પાછા cmpLessThan;

  સ્થિતિ Pack_Categories_Into_ચાવી(fcInfinity, fcInfinity):
    જો (sign == rhs.sign)
      પાછા cmpEqual;
    નહીંતર જો (sign)
      પાછા cmpLessThan;
    નહીંતર
      પાછા cmpGreaterThan;

  સ્થિતિ Pack_Categories_Into_ચાવી(fcZero, fcZero):
    પાછા cmpEqual;

  સ્થિતિ Pack_Categories_Into_ચાવી(fcNormal, fcNormal):
    તોડો;
  }

  cmpResult result;
  /* Two normal numbers.  Do they have the same sign?  */
  જો (sign != rhs.sign) {
    જો (sign)
      result = cmpLessThan;
    નહીંતર
      result = cmpGreaterThan;
  } નહીંતર {
    /* Compare absolute values; invert result if negative.  */
    result = compare_Absolute_કિંમત(rhs);

    જો (sign) {
      જો (result == cmpLessThan)
        result = cmpGreaterThan;
      નહીંતર જો (result == cmpGreaterThan)
        result = cmpLessThan;
    }
  }

  પાછા result;
}

/// IEEEFloat::convert - convert a value of one floating point type to another.
/// The return value corresponds to the IEEE754 exceptions.  *losesInfo
/// records whether the transformation lost information, i.e. whether
/// converting the result back to the original type will produce the
/// original value (this is almost the same as return value==fsOK, but there
/// are edge cases where this is not so).

APFloat::opStatus IEEEFloat::convert(અચલ fltSemantics &toSemantics,
                                     roundingMode rounding_mode,
                                     તાર્કિક *losesInfo) {
  opStatus fs;
  અચલ fltSemantics &fromSemantics = *semantics;
  તાર્કિક is_signaling = isSignaling();

  lostFraction lostFraction = lfExactlyZero;
  unsigned new_Part_ગણતરી = part_ગણતરી_For_Bits(toSemantics.precision + 1);
  unsigned old_Part_ગણતરી = part_ગણતરી();
  પૂર્ણાંક shift = toSemantics.precision - fromSemantics.precision;

  તાર્કિક X86SpecialNan = ખોટું;
  જો (&fromSemantics == &APFloatBase::semX87DoubleExtended &&
      &toSemantics != &APFloatBase::semX87DoubleExtended && category == fcNaN &&
      (!(*significandParts() & 0x8000000000000000ULL) ||
       !(*significandParts() & 0x4000000000000000ULL))) {
    // x86 has some unusual NaNs which cannot be represented in any other
    // format; note them here.
    X86SpecialNan = સાચું;
  }

  // If this is a truncation of a denormal number, and the target semantics
  // has larger exponent range than the source semantics (this can happen
  // when truncating from PowerPC double-double to double format), the
  // right shift could lose result mantissa bits.  Adjust exponent instead
  // of performing excessive shift.
  // Also do a similar trick in case shifting denormal would produce zero
  // significand as this case isn't handled correctly by normalize.
  જો (shift < 0 && isFiniteNonZero()) {
    પૂર્ણાંક omsb = significandMSB() + 1;
    પૂર્ણાંક exponentChange = omsb - fromSemantics.precision;
    જો (exponent + exponentChange < toSemantics.minExponent)
      exponentChange = toSemantics.minExponent - exponent;
    exponentChange = std::max(exponentChange, shift);
    જો (exponentChange < 0) {
      shift -= exponentChange;
      exponent += exponentChange;
    } નહીંતર જો (omsb <= -shift) {
      exponentChange = omsb + shift - 1; // leave at least one bit set
      shift -= exponentChange;
      exponent += exponentChange;
    }
  }

  // If this is a truncation, perform the shift before we narrow the storage.
  જો (shift < 0 && (isFiniteNonZero() ||
                    (category == fcNaN && semantics->nonFiniteBehavior !=
                                              fltNonfiniteBehavior::NanOnly)))
    lostFraction = shift_જમણું(significandParts(), old_Part_ગણતરી, -shift);

  // Fix the storage so it can hold to new value.
  જો (new_Part_ગણતરી > old_Part_ગણતરી) {
    // The new type requires more storage; make it available.
    integerPart *newParts;
    newParts = નવું integerPart[new_Part_ગણતરી];
    APInt::tcSet(newParts, 0, new_Part_ગણતરી);
    જો (isFiniteNonZero() || category==fcNaN)
      APInt::tcAssign(newParts, significandParts(), old_Part_ગણતરી);
    freeSignificand();
    significand.parts = newParts;
  } નહીંતર જો (new_Part_ગણતરી == 1 && old_Part_ગણતરી != 1) {
    // Switch to built-in storage for a single part.
    integerPart newPart = 0;
    જો (isFiniteNonZero() || category==fcNaN)
      newPart = significandParts()[0];
    freeSignificand();
    significand.part = newPart;
  }

  // Now that we have the right storage, switch the semantics.
  semantics = &toSemantics;

  // If this is an extension, perform the shift now that the storage is
  // available.
  જો (shift > 0 && (isFiniteNonZero() || category==fcNaN))
    APInt::tc_Shift_ડાબું(significandParts(), new_Part_ગણતરી, shift);

  જો (isFiniteNonZero()) {
    fs = normalize(rounding_mode, lostFraction);
    *losesInfo = (fs != opOK);
  } નહીંતર જો (category == fcNaN) {
    જો (semantics->nonFiniteBehavior == fltNonfiniteBehavior::NanOnly) {
      *losesInfo =
          fromSemantics.nonFiniteBehavior != fltNonfiniteBehavior::NanOnly;
      makeNaN(ખોટું, sign);
      fs = is_signaling ? opInvalidOp : opOK;
    } નહીંતર {
      // If NaN is negative zero, we need to create a new NaN to avoid
      // converting NaN to -Inf.
      જો (fromSemantics.nanEncoding == fltNanEncoding::NegativeZero &&
          semantics->nanEncoding != fltNanEncoding::NegativeZero)
        makeNaN(ખોટું, ખોટું);

      // If the source has no significand, there are no payload bits to carry
      // over, and an all-zero significand would encode an Inf. Create a new
      // NaN.
      જો (!APFloat::hasSignificand(fromSemantics))
        makeNaN(ખોટું, sign);

      *losesInfo = lostFraction != lfExactlyZero || X86SpecialNan;

      // For x87 extended precision, we want to make a NaN, not a special NaN
      // if the input wasn't special either.
      જો (!X86SpecialNan && semantics == &APFloatBase::semX87DoubleExtended)
        APInt::tcSetBit(significandParts(), semantics->precision - 1);

      // Convert of sNaN creates qNaN and raises an exception (invalid op).
      // This also guarantees that a sNaN does not become Inf on a truncation
      // that loses all payload bits.
      જો (is_signaling) {
        makeQuiet();
        fs = opInvalidOp;
      } નહીંતર {
        fs = opOK;
      }
    }
  } નહીંતર જો (category == fcInfinity &&
             semantics->nonFiniteBehavior == fltNonfiniteBehavior::NanOnly) {
    makeNaN(ખોટું, sign);
    *losesInfo = સાચું;
    fs = opInexact;
  } નહીંતર જો (category == fcZero &&
             semantics->nanEncoding == fltNanEncoding::NegativeZero) {
    // Negative zero loses info, but positive zero doesn't.
    *losesInfo =
        fromSemantics.nanEncoding != fltNanEncoding::NegativeZero && sign;
    fs = *losesInfo ? opInexact : opOK;
    // NaN is negative zero means -0 -> +0, which can lose information
    sign = ખોટું;
  } નહીંતર {
    *losesInfo = ખોટું;
    fs = opOK;
  }

  // The target may have no encoding for a negative value, or none for zero.
  // The paths above only report what rounding lost, so report these here too:
  // a caller that checks losesInfo would otherwise accept a result the target
  // cannot represent, and printing that result asserts.
  જો ((sign && !semantics->hasSignedRepr) ||
      (category == fcZero && !semantics->hasZero)) {
    *losesInfo = સાચું;
    જો (fs == opOK)
      fs = opInexact;
  }

  જો (category == fcZero && !semantics->hasZero)
    makeSmallestNormalized(ખોટું);
  પાછા fs;
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
    MutableArrayRef<integerPart> parts, unsigned પૂર્ણાંક width, તાર્કિક isSigned,
    roundingMode rounding_mode, તાર્કિક *isExact) અચલ {
  *isExact = ખોટું;

  /* Handle the three special cases first.  */
  જો (category == fcInfinity || category == fcNaN)
    પાછા opInvalidOp;

  unsigned dst_Parts_ગણતરી = part_ગણતરી_For_Bits(width);
  પ્રતિજ્ઞા(dst_Parts_ગણતરી <= parts.size() && "Integer too big");

  જો (category == fcZero) {
    APInt::tcSet(parts.data(), 0, dst_Parts_ગણતરી);
    // Negative zero can't be represented as an int.
    *isExact = !sign;
    પાછા opOK;
  }

  અચલ integerPart *src = significandParts();

  unsigned truncatedBits;
  /* Step 1: place our absolute value, with any fraction truncated, in
     the destination.  */
  જો (exponent < 0) {
    /* Our absolute value is less than one; truncate everything.  */
    APInt::tcSet(parts.data(), 0, dst_Parts_ગણતરી);
    /* For exponent -1 the integer bit represents .5, look at that.
       For smaller exponents leftmost truncated bit is 0. */
    truncatedBits = semantics->precision -1U - exponent;
  } નહીંતર {
    /* We want the most significant (exponent + 1) bits; the rest are
       truncated.  */
    unsigned પૂર્ણાંક bits = exponent + 1U;

    /* Hopelessly large in magnitude?  */
    જો (bits > width)
      પાછા opInvalidOp;

    જો (bits < semantics->precision) {
      /* We truncate (semantics->precision - bits) bits.  */
      truncatedBits = semantics->precision - bits;
      APInt::tcExtract(parts.data(), dst_Parts_ગણતરી, src, bits, truncatedBits);
    } નહીંતર {
      /* We want at least as many bits as are available.  */
      APInt::tcExtract(parts.data(), dst_Parts_ગણતરી, src, semantics->precision,
                       0);
      APInt::tc_Shift_ડાબું(parts.data(), dst_Parts_ગણતરી,
                         bits - semantics->precision);
      truncatedBits = 0;
    }
  }

  /* Step 2: work out any lost fraction, and increment the absolute
     value if we would round away from zero.  */
  lostFraction lost_fraction;
  જો (truncatedBits) {
    lost_fraction = lostFractionThroughTruncation(src, part_ગણતરી(),
                                                  truncatedBits);
    જો (lost_fraction != lfExactlyZero &&
        roundAwayFromZero(rounding_mode, lost_fraction, truncatedBits)) {
      જો (APInt::tcIncrement(parts.data(), dst_Parts_ગણતરી))
        પાછા opInvalidOp;     /* Overflow.  */
    }
  } નહીંતર {
    lost_fraction = lfExactlyZero;
  }

  /* Step 3: check if we fit in the destination.  */
  unsigned પૂર્ણાંક omsb = APInt::tcMSB(parts.data(), dst_Parts_ગણતરી) + 1;

  જો (sign) {
    જો (!isSigned) {
      /* Negative numbers cannot be represented as unsigned.  */
      જો (omsb != 0)
        પાછા opInvalidOp;
    } નહીંતર {
      /* It takes omsb bits to represent the unsigned integer value.
         We lose a bit for the sign, but care is needed as the
         maximally negative integer is a special case.  */
      જો (omsb == width &&
          APInt::tcLSB(parts.data(), dst_Parts_ગણતરી) + 1 != omsb)
        પાછા opInvalidOp;

      /* This case can happen because of rounding.  */
      જો (omsb > width)
        પાછા opInvalidOp;
    }

    APInt::tcNegate (parts.data(), dst_Parts_ગણતરી);
  } નહીંતર {
    જો (omsb >= width + !isSigned)
      પાછા opInvalidOp;
  }

  જો (lost_fraction == lfExactlyZero) {
    *isExact = સાચું;
    પાછા opOK;
  }
  પાછા opInexact;
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
                            unsigned પૂર્ણાંક width, તાર્કિક isSigned,
                            roundingMode rounding_mode, તાર્કિક *isExact) અચલ {
  opStatus fs = convertToSignExtendedInteger(parts, width, isSigned,
                                             rounding_mode, isExact);

  જો (fs == opInvalidOp) {
    unsigned પૂર્ણાંક bits, dst_Parts_ગણતરી;

    dst_Parts_ગણતરી = part_ગણતરી_For_Bits(width);
    પ્રતિજ્ઞા(dst_Parts_ગણતરી <= parts.size() && "Integer too big");

    જો (category == fcNaN)
      bits = 0;
    નહીંતર જો (sign)
      bits = isSigned;
    નહીંતર
      bits = width - isSigned;

    tcSetLeastSignificantBits(parts.data(), dst_Parts_ગણતરી, bits);
    જો (sign && isSigned)
      APInt::tc_Shift_ડાબું(parts.data(), dst_Parts_ગણતરી, width - 1);
  }

  પાછા fs;
}

/* Convert an unsigned integer SRC to a floating point number,
   rounding according to ROUNDING_MODE.  The sign of the floating
   point number is not modified.  */
APFloat::opStatus IEEEFloat::convertFromUnsignedParts(
    અચલ integerPart *src, unsigned પૂર્ણાંક src_ગણતરી, roundingMode rounding_mode) {
  category = fcNormal;
  unsigned omsb = APInt::tcMSB(src, src_ગણતરી) + 1;
  integerPart *dst = significandParts();
  unsigned dst_ગણતરી = part_ગણતરી();
  unsigned precision = semantics->precision;

  /* We want the most significant PRECISION bits of SRC.  There may not
     be that many; extract what we can.  */
  lostFraction lost_fraction;
  જો (precision <= omsb) {
    exponent = omsb - 1;
    lost_fraction = lostFractionThroughTruncation(src, src_ગણતરી,
                                                  omsb - precision);
    APInt::tcExtract(dst, dst_ગણતરી, src, precision, omsb - precision);
  } નહીંતર {
    exponent = precision - 1;
    lost_fraction = lfExactlyZero;
    APInt::tcExtract(dst, dst_ગણતરી, src, omsb, 0);
  }

  પાછા normalize(rounding_mode, lost_fraction);
}

APFloat::opStatus IEEEFloat::convertFromAPInt(અચલ APInt &Val, તાર્કિક isSigned,
                                              roundingMode rounding_mode) {
  unsigned પૂર્ણાંક part_ગણતરી = Val.get_અંક_શબ્દો();
  APInt api = Val;

  sign = ખોટું;
  જો (isSigned && api.isNegative()) {
    sign = સાચું;
    api = -api;
  }

  પાછા convertFromUnsignedParts(api.getRawData(), part_ગણતરી, rounding_mode);
}

Expected<APFloat::opStatus>
IEEEFloat::convertFromHexadecimalString(StringRef s,
                                        roundingMode rounding_mode) {
  lostFraction lost_fraction = lfExactlyZero;

  category = fcNormal;
  zeroSignificand();
  exponent = 0;

  integerPart *significand = significandParts();
  unsigned parts_ગણતરી = part_ગણતરી();
  unsigned bitPos = parts_ગણતરી * integerPartWidth;
  તાર્કિક computedTrailingFraction = ખોટું;

  // Skip leading zeroes and any (hexa)decimal point.
  StringRef::iterator begin = s.begin();
  StringRef::iterator end = s.end();
  StringRef::iterator dot;
  auto PtrOrErr = skipLeadingZeroesAndAnyDot(begin, end, &dot);
  જો (!PtrOrErr)
    પાછા PtrOrErr.takeError();
  StringRef::iterator p = *PtrOrErr;
  StringRef::iterator firstSignificantDigit = p;

  જ્યારે (p != end) {
    integerPart hex_કિંમત;

    જો (*p == '.') {
      જો (dot != end)
        પાછા createError("String contains multiple dots");
      dot = p++;
      ચાલુ;
    }

    hex_કિંમત = hex_Digit_કિંમત(*p);
    જો (hex_કિંમત == UINT_MAX)
      તોડો;

    p++;

    // Store the number while we have space.
    જો (bitPos) {
      bitPos -= 4;
      hex_કિંમત <<= bitPos % integerPartWidth;
      significand[bitPos / integerPartWidth] |= hex_કિંમત;
    } નહીંતર જો (!computedTrailingFraction) {
      auto FractOrErr = trailingHexadecimalFraction(p, end, hex_કિંમત);
      જો (!FractOrErr)
        પાછા FractOrErr.takeError();
      lost_fraction = *FractOrErr;
      computedTrailingFraction = સાચું;
    }
  }

  /* Hex floats require an exponent but not a hexadecimal point.  */
  જો (p == end)
    પાછા createError("Hex strings require an exponent");
  જો (*p != 'p' && *p != 'P')
    પાછા createError("Invalid character in significand");
  જો (p == begin)
    પાછા createError("Significand has no digits");
  જો (dot != end && p - begin == 1)
    પાછા createError("Significand has no digits");

  /* Ignore the exponent if we are zero.  */
  જો (p != firstSignificantDigit) {
    પૂર્ણાંક expAdjustment;

    /* Implicit hexadecimal point?  */
    જો (dot == end)
      dot = p;

    /* Calculate the exponent adjustment implicit in the number of
       significant digits.  */
    expAdjustment = static_cast<પૂર્ણાંક>(dot - firstSignificantDigit);
    જો (expAdjustment < 0)
      expAdjustment++;
    expAdjustment = expAdjustment * 4 - 1;

    /* Adjust for writing the significand starting at the most
       significant nibble.  */
    expAdjustment += semantics->precision;
    expAdjustment -= parts_ગણતરી * integerPartWidth;

    /* Adjust for the given exponent.  */
    auto ExpOrErr = totalExponent(p + 1, end, expAdjustment);
    જો (!ExpOrErr)
      પાછા ExpOrErr.takeError();
    exponent = *ExpOrErr;
  }

  પાછા normalize(rounding_mode, lost_fraction);
}

APFloat::opStatus
IEEEFloat::roundSignificandWithExponent(અચલ integerPart *decSigParts,
                                        unsigned sig_Part_ગણતરી, પૂર્ણાંક exp,
                                        roundingMode rounding_mode) {
  fltSemantics calcSemantics = { 32767, -32767, 0, 0 };
  unsigned પૂર્ણાંક power = exp >= 0 ? exp : -exp;
  unsigned પૂર્ણાંક pow5PartCapacity = powerOfFivePartsForExponent(power);
  SmallVector<integerPart, અંક_Pow5_Parts_On_ઢગલો> pow5Parts;
  pow5Parts.resize_for_overwrite(pow5PartCapacity);

  તાર્કિક isNearest = rounding_mode == rmNearestTiesToEven ||
                   rounding_mode == rmNearestTiesToAway;

  unsigned parts = part_ગણતરી_For_Bits(semantics->precision + 11);

  /* Calculate pow(5, abs(exp)).  */
  unsigned pow5_Part_ગણતરી = powerOf5(pow5Parts.data(), power);

  માટે (;; parts *= 2) {
    unsigned પૂર્ણાંક excessPrecision, truncatedBits;

    calcSemantics.precision = parts * integerPartWidth - 1;
    excessPrecision = calcSemantics.precision - semantics->precision;
    truncatedBits = excessPrecision;

    IEEEFloat decSig(calcSemantics, uninitialized);
    decSig.makeZero(sign);
    IEEEFloat pow5(calcSemantics);

    opStatus sigStatus = decSig.convertFromUnsignedParts(
        decSigParts, sig_Part_ગણતરી, rmNearestTiesToEven);
    opStatus powStatus = pow5.convertFromUnsignedParts(
        pow5Parts.data(), pow5_Part_ગણતરી, rmNearestTiesToEven);
    /* Add exp, as 10^n = 5^n * 2^n.  */
    decSig.exponent += exp;

    lostFraction calcLostFraction;
    integerPart HUerr, HUdistance;
    unsigned પૂર્ણાંક powHUerr;

    જો (exp >= 0) {
      /* multiplySignificand leaves the precision-th bit set to 1.  */
      calcLostFraction = decSig.multiplySignificand(pow5);
      powHUerr = powStatus != opOK;
    } નહીંતર {
      calcLostFraction = decSig.divideSignificand(pow5);
      /* Denormal numbers have less precision.  */
      જો (decSig.exponent < semantics->minExponent) {
        excessPrecision += (semantics->minExponent - decSig.exponent);
        truncatedBits = excessPrecision;
        excessPrecision = std::min(excessPrecision, calcSemantics.precision);
      }
      /* Extra half-ulp lost in reciprocal of exponent.  */
      powHUerr = (powStatus == opOK && calcLostFraction == lfExactlyZero) ? 0:2;
    }

    /* Both multiplySignificand and divideSignificand return the
       result with the integer bit set.  */
    પ્રતિજ્ઞા(APInt::tcExtractBit
           (decSig.significandParts(), calcSemantics.precision - 1) == 1);

    HUerr = HUerrBound(calcLostFraction != lfExactlyZero, sigStatus != opOK,
                       powHUerr);
    HUdistance = 2 * ulpsFromBoundary(decSig.significandParts(),
                                      excessPrecision, isNearest);

    /* Are we guaranteed to round correctly if we truncate?  */
    જો (HUdistance >= HUerr) {
      APInt::tcExtract(significandParts(), part_ગણતરી(), decSig.significandParts(),
                       calcSemantics.precision - excessPrecision,
                       excessPrecision);
      /* Take the exponent of decSig.  If we tcExtract-ed less bits
         above we must adjust our exponent to compensate for the
         implicit right shift.  */
      exponent = (decSig.exponent + semantics->precision
                  - (calcSemantics.precision - excessPrecision));
      calcLostFraction = lostFractionThroughTruncation(decSig.significandParts(),
                                                       decSig.part_ગણતરી(),
                                                       truncatedBits);
      પાછા static_cast<opStatus>(normalize(rounding_mode, calcLostFraction) |
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
  જો (Error Err = interpretDecimal(p, str.end(), &D))
    પાછા std::move(Err);

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
  જો (D.firstSigDigit == str.end() || dec_Digit_કિંમત(*D.firstSigDigit) >= 10U) {
    category = fcZero;
    fs = opOK;
    જો (semantics->nanEncoding == fltNanEncoding::NegativeZero)
      sign = ખોટું;
    જો (!semantics->hasZero)
      makeSmallestNormalized(ખોટું);

    /* Check whether the normalized exponent is high enough to overflow
       max during the log-rebasing in the max-exponent check below. */
  } નહીંતર જો (D.normalizedExponent - 1 > INT_MAX / 42039) {
    fs = handleOverflow(rounding_mode);

  /* If it wasn't, then it also wasn't high enough to overflow max
     during the log-rebasing in the min-exponent check.  Check that it
     won't overflow min in either check, then perform the min-exponent
     check. */
  } નહીંતર જો (D.normalizedExponent - 1 < INT_MIN / 42039 ||
             (D.normalizedExponent + 1) * 28738 <=
               8651 * (semantics->minExponent - (પૂર્ણાંક) semantics->precision)) {
    /* Underflow to zero and round.  */
    category = fcNormal;
    zeroSignificand();
    fs = normalize(rounding_mode, lfLessThanHalf);

  /* We can finally safely perform the max-exponent check. */
  } નહીંતર જો ((D.normalizedExponent - 1) * 42039
             >= 12655 * semantics->maxExponent) {
    /* Overflow and round.  */
    fs = handleOverflow(rounding_mode);
  } નહીંતર {
    integerPart *decSignificand;
    unsigned પૂર્ણાંક part_ગણતરી;

    /* A tight upper bound on number of bits required to hold an
       N-digit decimal integer is N * 196 / 59.  Allocate enough space
       to hold the full significand, and an extra part required by
       tcMultiplyPart.  */
    part_ગણતરી = static_cast<unsigned પૂર્ણાંક>(D.lastSigDigit - D.firstSigDigit) + 1;
    part_ગણતરી = part_ગણતરી_For_Bits(1 + 196 * part_ગણતરી / 59);
    decSignificand = નવું integerPart[part_ગણતરી + 1];
    part_ગણતરી = 0;

    /* Convert to binary efficiently - we do almost all multiplication
       in an integerPart.  When this would overflow do we do a single
       bignum multiplication, and then revert again to multiplication
       in an integerPart.  */
    કરો {
      integerPart dec_કિંમત, val, multiplier;

      val = 0;
      multiplier = 1;

      કરો {
        જો (*p == '.') {
          p++;
          જો (p == str.end()) {
            તોડો;
          }
        }
        dec_કિંમત = dec_Digit_કિંમત(*p++);
        જો (dec_કિંમત >= 10U) {
          delete[] decSignificand;
          પાછા createError("Invalid character in significand");
        }
        multiplier *= 10;
        val = val * 10 + dec_કિંમત;
        /* The maximum number that can be multiplied by ten with any
           digit added without overflowing an integerPart.  */
      } જ્યારે (p <= D.lastSigDigit && multiplier <= (~ (integerPart) 0 - 9) / 10);

      /* Multiply out the current part.  */
      APInt::tcMultiplyPart(decSignificand, decSignificand, multiplier, val,
                            part_ગણતરી, part_ગણતરી + 1, ખોટું);

      /* If we used another part (likely but not guaranteed), increase
         the count.  */
      જો (decSignificand[part_ગણતરી])
        part_ગણતરી++;
    } જ્યારે (p <= D.lastSigDigit);

    category = fcNormal;
    fs = roundSignificandWithExponent(decSignificand, part_ગણતરી,
                                      D.exponent, rounding_mode);

    delete [] decSignificand;
  }

  પાછા fs;
}

તાર્કિક IEEEFloat::convertFromStringSpecials(StringRef str) {
  અચલ size_t MIN_NAME_SIZE = 3;

  જો (str.size() < MIN_NAME_SIZE)
    પાછા ખોટું;

  જો (str == "inf" || str == "INFINITY" || str == "+Inf" || str == "+inf") {
    makeInf(ખોટું);
    પાછા સાચું;
  }

  તાર્કિક IsNegative = str.consume_front("-");
  જો (IsNegative) {
    જો (str.size() < MIN_NAME_SIZE)
      પાછા ખોટું;

    જો (str == "inf" || str == "INFINITY" || str == "Inf") {
      makeInf(સાચું);
      પાછા સાચું;
    }
  }

  // If we have a 's' (or 'S') prefix, then this is a Signaling NaN.
  તાર્કિક IsSignaling = str.consume_front_insensitive("s");
  જો (IsSignaling) {
    જો (str.size() < MIN_NAME_SIZE)
      પાછા ખોટું;
  }

  જો (str.consume_front("nan") || str.consume_front("NaN")) {
    // A NaN without payload.
    જો (str.empty()) {
      makeNaN(IsSignaling, IsNegative);
      પાછા સાચું;
    }

    // Allow the payload to be inside parentheses.
    જો (str.front() == '(') {
      // Parentheses should be balanced (and not empty).
      જો (str.size() <= 2 || str.back() != ')')
        પાછા ખોટું;

      str = str.slice(1, str.size() - 1);
    }

    // Determine the payload number's radix.
    unsigned Radix = 10;
    જો (str[0] == '0') {
      જો (str.size() > 1 && tolower(str[1]) == 'x') {
        str = str.drop_front(2);
        Radix = 16;
      } નહીંતર {
        Radix = 8;
      }
    }

    // Parse the payload and make the NaN.
    APInt Payload;
    જો (!str.getAsInteger(Radix, Payload)) {
      makeNaN(IsSignaling, IsNegative, &Payload);
      પાછા સાચું;
    }
  }

  પાછા ખોટું;
}

Expected<APFloat::opStatus>
IEEEFloat::convertFromString(StringRef str, roundingMode rounding_mode) {
  જો (str.empty())
    પાછા createError("Invalid string length");

  // Handle special cases.
  જો (convertFromStringSpecials(str))
    પાછા opOK;

  /* Handle a leading minus sign.  */
  StringRef::iterator p = str.begin();
  size_t slen = str.size();
  sign = *p == '-' ? 1 : 0;
  જો (sign && !semantics->hasSignedRepr)
    llvm_unreachable(
        "This floating point format does not support signed values");

  જો (*p == '-' || *p == '+') {
    p++;
    slen--;
    જો (!slen)
      પાછા createError("String has no digits");
  }

  જો (slen >= 2 && p[0] == '0' && (p[1] == 'x' || p[1] == 'X')) {
    જો (slen == 2)
      પાછા createError("Invalid string");
    પાછા convertFromHexadecimalString(StringRef(p + 2, slen - 2),
                                        rounding_mode);
  }

  પાછા convertFromDecimalString(StringRef(p, slen), rounding_mode);
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
unsigned પૂર્ણાંક IEEEFloat::convertToHexString(અક્ષર *dst, unsigned પૂર્ણાંક hexDigits,
                                           તાર્કિક upperCase,
                                           roundingMode rounding_mode) અચલ {
  અક્ષર *p = dst;
  જો (sign)
    *dst++ = '-';

  પસંદગી (category) {
  સ્થિતિ fcInfinity:
    memcpy (dst, upperCase ? infinityU: infinityL, કદ infinityU - 1);
    dst += કદ infinityL - 1;
    તોડો;

  સ્થિતિ fcNaN:
    memcpy (dst, upperCase ? NaNU: NaNL, કદ NaNU - 1);
    dst += કદ NaNU - 1;
    તોડો;

  સ્થિતિ fcZero:
    *dst++ = '0';
    *dst++ = upperCase ? 'X': 'x';
    *dst++ = '0';
    જો (hexDigits > 1) {
      *dst++ = '.';
      memset (dst, '0', hexDigits - 1);
      dst += hexDigits - 1;
    }
    *dst++ = upperCase ? 'P': 'p';
    *dst++ = '0';
    તોડો;

  સ્થિતિ fcNormal:
    dst = convertNormalToHexString (dst, hexDigits, upperCase, rounding_mode);
    તોડો;
  }

  *dst = 0;

  પાછા static_cast<unsigned પૂર્ણાંક>(dst - p);
}

/* Does the hard work of outputting the correctly rounded hexadecimal
   form of a normal floating point number with the specified number of
   hexadecimal digits.  If HEXDIGITS is zero the minimum number of
   digits necessary to print the value precisely is output.  */
અક્ષર *IEEEFloat::convertNormalToHexString(અક્ષર *dst, unsigned પૂર્ણાંક hexDigits,
                                          તાર્કિક upperCase,
                                          roundingMode rounding_mode) અચલ {
  *dst++ = '0';
  *dst++ = upperCase ? 'X': 'x';

  તાર્કિક roundUp = ખોટું;
  અચલ અક્ષર *hexDigitChars = upperCase ? hexDigitsUpper : hexDigitsLower;

  અચલ integerPart *significand = significandParts();
  unsigned parts_ગણતરી = part_ગણતરી();

  /* +3 because the first digit only uses the single integer bit, so
     we have 3 virtual zero most-significant-bits.  */
  unsigned કિંમત_Bits = semantics->precision + 3;
  unsigned shift = integerPartWidth - કિંમત_Bits % integerPartWidth;

  /* The natural number of digits required ignoring trailing
     insignificant zeroes.  */
  unsigned outputDigits = (કિંમત_Bits - significandLSB() + 3) / 4;

  /* hexDigits of zero means use the required number for the
     precision.  Otherwise, see if we are truncating.  If we are,
     find out if we need to round away from zero.  */
  જો (hexDigits) {
    જો (hexDigits < outputDigits) {
      /* We are dropping non-zero bits, so need to check how to round.
         "bits" is the number of dropped bits.  */
      unsigned પૂર્ણાંક bits;
      lostFraction fraction;

      bits = કિંમત_Bits - hexDigits * 4;
      fraction = lostFractionThroughTruncation (significand, parts_ગણતરી, bits);
      roundUp = roundAwayFromZero(rounding_mode, fraction, bits);
    }
    outputDigits = hexDigits;
  }

  /* Write the digits consecutively, and start writing in the location
     of the hexadecimal point.  We move the most significant digit
     left and add the hexadecimal point later.  */
  અક્ષર *p = ++dst;

  unsigned ગણતરી = (કિંમત_Bits + integerPartWidth - 1) / integerPartWidth;

  જ્યારે (outputDigits && ગણતરી) {
    integerPart part;

    /* Put the most significant integerPartWidth bits in "part".  */
    જો (--ગણતરી == parts_ગણતરી)
      part = 0;  /* An imaginary higher zero part.  */
    નહીંતર
      part = significand[ગણતરી] << shift;

    જો (ગણતરી && shift)
      part |= significand[ગણતરી - 1] >> (integerPartWidth - shift);

    /* Convert as much of "part" to hexdigits as we can.  */
    unsigned પૂર્ણાંક curDigits = integerPartWidth / 4;

    curDigits = std::min(curDigits, outputDigits);
    dst += partAsHex (dst, part, curDigits, hexDigitChars);
    outputDigits -= curDigits;
  }

  જો (roundUp) {
    અક્ષર *q = dst;

    /* Note that hexDigitChars has a trailing '0'.  */
    કરો {
      q--;
      *q = hexDigitChars[hex_Digit_કિંમત (*q) + 1];
    } જ્યારે (*q == '0');
    પ્રતિજ્ઞા(q >= p);
  } નહીંતર {
    /* Add trailing zeroes.  */
    memset (dst, '0', outputDigits);
    dst += outputDigits;
  }

  /* Move the most significant digit to before the point, and if there
     is something after the decimal point add it.  This must come
     after rounding above.  */
  p[-1] = p[0];
  જો (dst -1 == p)
    dst--;
  નહીંતર
    p[0] = '.';

  /* Finally output the exponent.  */
  *dst++ = upperCase ? 'P': 'p';

  પાછા writeSignedDecimal (dst, exponent);
}

hash_code hash_કિંમત(અચલ IEEEFloat &Arg) {
  જો (!Arg.isFiniteNonZero())
    પાછા hash_combine((uint8_t)Arg.category,
                        // NaN has no sign, fix it at zero.
                        Arg.isNaN() ? (uint8_t)0 : (uint8_t)Arg.sign,
                        Arg.semantics->precision);

  // Normal floats need their exponent and significand hashed.
  પાછા hash_combine((uint8_t)Arg.category, (uint8_t)Arg.sign,
                      Arg.semantics->precision, Arg.exponent,
                      hash_combine_range(
                        Arg.significandParts(),
                        Arg.significandParts() + Arg.part_ગણતરી()));
}

// Conversion from APFloat to/from host float/double.  It may eventually be
// possible to eliminate these and have everybody deal with APFloats, but that
// will take a while.  This approach will not easily extend to long double.
// Current implementation requires integerPartWidth==64, which is correct at
// the moment but could be made more general.

// Denormals have exponent minExponent in APFloat, but minExponent-1 in
// the actual IEEE respresentations.  We compensate for that here.

APInt IEEEFloat::convertF80LongDoubleAPFloatToAPInt() અચલ {
  પ્રતિજ્ઞા(part_ગણતરી() == 2);
  પાછા convertIEEEFloatToAPInt<APFloatBase::semX87DoubleExtended>();
}

APInt IEEEFloat::convertPPCDoubleDoubleLegacyAPFloatToAPInt() અચલ {
  પ્રતિજ્ઞા(semantics ==
         (અચલ llvm::fltSemantics *)&APFloatBase::semPPCDoubleDoubleLegacy);
  પ્રતિજ્ઞા(part_ગણતરી()==2);

  uint64_t શબ્દો[2];
  તાર્કિક losesInfo;

  // Convert number to double.  To avoid spurious underflows, we re-
  // normalize against the "double" minExponent first, and only *then*
  // truncate the mantissa.  The result of that second conversion
  // may be inexact, but should never underflow.
  // Declare fltSemantics before APFloat that uses it (and
  // saves pointer to it) to ensure correct destruction order.
  fltSemantics extendedSemantics = *semantics;
  extendedSemantics.minExponent = APFloatBase::semIEEEdouble.minExponent;
  IEEEFloat extended(*આ);
  [[maybe_unused]] opStatus fs =
      extended.convert(extendedSemantics, rmNearestTiesToEven, &losesInfo);
  પ્રતિજ્ઞા(fs == opOK && !losesInfo);

  IEEEFloat u(extended);
  fs = u.convert(APFloatBase::semIEEEdouble, rmNearestTiesToEven, &losesInfo);
  પ્રતિજ્ઞા(fs == opOK || fs == opInexact);
  શબ્દો[0] = *u.convertDoubleAPFloatToAPInt().getRawData();

  // If conversion was exact or resulted in a special case, we're done;
  // just set the second double to zero.  Otherwise, re-convert back to
  // the extended format and compute the difference.  This now should
  // convert exactly to double.
  જો (u.isFiniteNonZero() && losesInfo) {
    fs = u.convert(extendedSemantics, rmNearestTiesToEven, &losesInfo);
    પ્રતિજ્ઞા(fs == opOK && !losesInfo);

    IEEEFloat v(extended);
    v.subtract(u, rmNearestTiesToEven);
    fs = v.convert(APFloatBase::semIEEEdouble, rmNearestTiesToEven, &losesInfo);
    પ્રતિજ્ઞા(fs == opOK && !losesInfo);
    શબ્દો[1] = *v.convertDoubleAPFloatToAPInt().getRawData();
  } નહીંતર {
    શબ્દો[1] = 0;
  }

  પાછા APInt(128, શબ્દો);
}

template <અચલ fltSemantics &S>
APInt IEEEFloat::convertIEEEFloatToAPInt() અચલ {
  પ્રતિજ્ઞા(semantics == &S);
  constexpr unsigned પૂર્ણાંક trailing_significand_bits =
      S.precision - 1 + S.hasExplicitIntegerBit;
  constexpr પૂર્ણાંક integer_bit_part = (S.precision - 1) / integerPartWidth;
  constexpr integerPart integer_bit = integerPart{1}
                                      << ((S.precision - 1) % integerPartWidth);
  constexpr uint64_t significand_mask = integer_bit - 1;
  constexpr unsigned પૂર્ણાંક exponent_bits =
      S.sizeInBits - (S.hasSignedRepr ? 1 : 0) - trailing_significand_bits;
  static_assert(exponent_bits < 64);
  constexpr uint64_t exponent_mask = (uint64_t{1} << exponent_bits) - 1;
  constexpr તાર્કિક is_zero_exp_reserved = S.hasDenormals || S.hasZero;
  constexpr પૂર્ણાંક bias = -(S.minExponent - (is_zero_exp_reserved ? 1 : 0));

  uint64_t myexponent;
  std::array<integerPart, part_ગણતરી_For_Bits(trailing_significand_bits)>
      mysignificand;

  જો (isFiniteNonZero()) {
    myexponent = exponent + bias;
    std::copy_n(significandParts(), mysignificand.size(),
                mysignificand.begin());
    જો (myexponent == 1 &&
        !(significandParts()[integer_bit_part] & integer_bit))
      myexponent = 0; // denormal
  } નહીંતર જો (category == fcZero) {
    જો (!S.hasZero)
      llvm_unreachable("semantics does not support zero!");
    myexponent = ::exponentZero(S) + bias;
    mysignificand.fill(0);
  } નહીંતર જો (category == fcInfinity) {
    જો (S.nonFiniteBehavior == fltNonfiniteBehavior::NanOnly ||
        S.nonFiniteBehavior == fltNonfiniteBehavior::FiniteOnly)
      llvm_unreachable("semantics don't support inf!");
    myexponent = ::exponentInf(S) + bias;
    mysignificand.fill(0);
    જો constexpr (S.hasExplicitIntegerBit) {
      mysignificand[0] = integerPart{1} << (trailing_significand_bits - 1);
    }
  } નહીંતર {
    પ્રતિજ્ઞા(category == fcNaN && "Unknown category!");
    જો (S.nonFiniteBehavior == fltNonfiniteBehavior::FiniteOnly)
      llvm_unreachable("semantics don't support NaN!");
    myexponent = ::exponentNaN(S) + bias;
    std::copy_n(significandParts(), mysignificand.size(),
                mysignificand.begin());
  }
  std::array<uint64_t, (S.sizeInBits + 63) / 64> શબ્દો;
  auto શબ્દો_iter =
      std::copy_n(mysignificand.begin(), mysignificand.size(), શબ્દો.begin());
  જો constexpr (!S.hasExplicitIntegerBit) {
    જો constexpr (significand_mask != 0 || trailing_significand_bits == 0) {
      // Clear the integer bit.
      શબ્દો[mysignificand.size() - 1] &= significand_mask;
    }
  }
  std::fill(શબ્દો_iter, શબ્દો.end(), uint64_t{0});
  constexpr size_t last_word = શબ્દો.size() - 1;
  જો constexpr (S.hasSignedRepr) {
    uint64_t shifted_sign = static_cast<uint64_t>(sign & 1)
                            << ((S.sizeInBits - 1) % 64);
    શબ્દો[last_word] |= shifted_sign;
  }
  uint64_t shifted_exponent = (myexponent & exponent_mask)
                              << (trailing_significand_bits % 64);
  શબ્દો[last_word] |= shifted_exponent;
  જો constexpr (last_word == 0) {
    પાછા APInt(S.sizeInBits, શબ્દો[0]);
  }
  પાછા APInt(S.sizeInBits, શબ્દો);
}

APInt IEEEFloat::convertQuadrupleAPFloatToAPInt() અચલ {
  પ્રતિજ્ઞા(part_ગણતરી() == 2);
  પાછા convertIEEEFloatToAPInt<APFloatBase::semIEEEquad>();
}

APInt IEEEFloat::convertDoubleAPFloatToAPInt() અચલ {
  પ્રતિજ્ઞા(part_ગણતરી()==1);
  પાછા convertIEEEFloatToAPInt<APFloatBase::semIEEEdouble>();
}

APInt IEEEFloat::convertFloatAPFloatToAPInt() અચલ {
  પ્રતિજ્ઞા(part_ગણતરી()==1);
  પાછા convertIEEEFloatToAPInt<APFloatBase::semIEEEsingle>();
}

APInt IEEEFloat::convertBFloatAPFloatToAPInt() અચલ {
  પ્રતિજ્ઞા(part_ગણતરી() == 1);
  પાછા convertIEEEFloatToAPInt<APFloatBase::semBFloat>();
}

APInt IEEEFloat::convertHalfAPFloatToAPInt() અચલ {
  પ્રતિજ્ઞા(part_ગણતરી()==1);
  પાછા convertIEEEFloatToAPInt<APFloatBase::APFloatBase::semIEEEhalf>();
}

APInt IEEEFloat::convertFloat8E5M2APFloatToAPInt() અચલ {
  પ્રતિજ્ઞા(part_ગણતરી() == 1);
  પાછા convertIEEEFloatToAPInt<APFloatBase::semFloat8E5M2>();
}

APInt IEEEFloat::convertFloat8E5M2FNUZAPFloatToAPInt() અચલ {
  પ્રતિજ્ઞા(part_ગણતરી() == 1);
  પાછા convertIEEEFloatToAPInt<APFloatBase::semFloat8E5M2FNUZ>();
}

APInt IEEEFloat::convertFloat8E4M3APFloatToAPInt() અચલ {
  પ્રતિજ્ઞા(part_ગણતરી() == 1);
  પાછા convertIEEEFloatToAPInt<APFloatBase::semFloat8E4M3>();
}

APInt IEEEFloat::convertFloat8E4M3FNAPFloatToAPInt() અચલ {
  પ્રતિજ્ઞા(part_ગણતરી() == 1);
  પાછા convertIEEEFloatToAPInt<APFloatBase::semFloat8E4M3FN>();
}

APInt IEEEFloat::convertFloat8E4M3FNUZAPFloatToAPInt() અચલ {
  પ્રતિજ્ઞા(part_ગણતરી() == 1);
  પાછા convertIEEEFloatToAPInt<APFloatBase::semFloat8E4M3FNUZ>();
}

APInt IEEEFloat::convertFloat8E4M3B11FNUZAPFloatToAPInt() અચલ {
  પ્રતિજ્ઞા(part_ગણતરી() == 1);
  પાછા convertIEEEFloatToAPInt<APFloatBase::semFloat8E4M3B11FNUZ>();
}

APInt IEEEFloat::convertFloat8E3M4APFloatToAPInt() અચલ {
  પ્રતિજ્ઞા(part_ગણતરી() == 1);
  પાછા convertIEEEFloatToAPInt<APFloatBase::semFloat8E3M4>();
}

APInt IEEEFloat::convertFloatTF32APFloatToAPInt() અચલ {
  પ્રતિજ્ઞા(part_ગણતરી() == 1);
  પાછા convertIEEEFloatToAPInt<APFloatBase::semFloatTF32>();
}

APInt IEEEFloat::convertFloat8E8M0FNUAPFloatToAPInt() અચલ {
  પ્રતિજ્ઞા(part_ગણતરી() == 1);
  પાછા convertIEEEFloatToAPInt<APFloatBase::semFloat8E8M0FNU>();
}

APInt IEEEFloat::convertFloat8E5M3FNUAPFloatToAPInt() અચલ {
  પ્રતિજ્ઞા(part_ગણતરી() == 1);
  પાછા convertIEEEFloatToAPInt<APFloatBase::semFloat8E5M3FNU>();
}

APInt IEEEFloat::convertFloat6E3M2FNAPFloatToAPInt() અચલ {
  પ્રતિજ્ઞા(part_ગણતરી() == 1);
  પાછા convertIEEEFloatToAPInt<APFloatBase::semFloat6E3M2FN>();
}

APInt IEEEFloat::convertFloat6E2M3FNAPFloatToAPInt() અચલ {
  પ્રતિજ્ઞા(part_ગણતરી() == 1);
  પાછા convertIEEEFloatToAPInt<APFloatBase::semFloat6E2M3FN>();
}

APInt IEEEFloat::convertFloat4E2M1FNAPFloatToAPInt() અચલ {
  પ્રતિજ્ઞા(part_ગણતરી() == 1);
  પાછા convertIEEEFloatToAPInt<APFloatBase::semFloat4E2M1FN>();
}

// This function creates an APInt that is just a bit map of the floating
// point constant as it would appear in memory.  It is not a conversion,
// and treating the result as a normal integer is unlikely to be useful.

APInt IEEEFloat::bitcastToAPInt() અચલ {
  જો (semantics == (અચલ llvm::fltSemantics *)&APFloatBase::semIEEEhalf)
    પાછા convertHalfAPFloatToAPInt();

  જો (semantics == (અચલ llvm::fltSemantics *)&APFloatBase::semBFloat)
    પાછા convertBFloatAPFloatToAPInt();

  જો (semantics == (અચલ llvm::fltSemantics *)&APFloatBase::semIEEEsingle)
    પાછા convertFloatAPFloatToAPInt();

  જો (semantics == (અચલ llvm::fltSemantics *)&APFloatBase::semIEEEdouble)
    પાછા convertDoubleAPFloatToAPInt();

  જો (semantics == (અચલ llvm::fltSemantics *)&APFloatBase::semIEEEquad)
    પાછા convertQuadrupleAPFloatToAPInt();

  જો (semantics ==
      (અચલ llvm::fltSemantics *)&APFloatBase::semX87DoubleExtended)
    પાછા convertF80LongDoubleAPFloatToAPInt();

  જો (semantics ==
      (અચલ llvm::fltSemantics *)&APFloatBase::semPPCDoubleDoubleLegacy)
    પાછા convertPPCDoubleDoubleLegacyAPFloatToAPInt();

  જો (semantics == (અચલ llvm::fltSemantics *)&APFloatBase::semFloat8E5M2)
    પાછા convertFloat8E5M2APFloatToAPInt();

  જો (semantics == (અચલ llvm::fltSemantics *)&APFloatBase::semFloat8E5M2FNUZ)
    પાછા convertFloat8E5M2FNUZAPFloatToAPInt();

  જો (semantics == (અચલ llvm::fltSemantics *)&APFloatBase::semFloat8E4M3)
    પાછા convertFloat8E4M3APFloatToAPInt();

  જો (semantics == (અચલ llvm::fltSemantics *)&APFloatBase::semFloat8E4M3FN)
    પાછા convertFloat8E4M3FNAPFloatToAPInt();

  જો (semantics == (અચલ llvm::fltSemantics *)&APFloatBase::semFloat8E4M3FNUZ)
    પાછા convertFloat8E4M3FNUZAPFloatToAPInt();

  જો (semantics ==
      (અચલ llvm::fltSemantics *)&APFloatBase::semFloat8E4M3B11FNUZ)
    પાછા convertFloat8E4M3B11FNUZAPFloatToAPInt();

  જો (semantics == (અચલ llvm::fltSemantics *)&APFloatBase::semFloat8E3M4)
    પાછા convertFloat8E3M4APFloatToAPInt();

  જો (semantics == (અચલ llvm::fltSemantics *)&APFloatBase::semFloatTF32)
    પાછા convertFloatTF32APFloatToAPInt();

  જો (semantics == (અચલ llvm::fltSemantics *)&APFloatBase::semFloat8E8M0FNU)
    પાછા convertFloat8E8M0FNUAPFloatToAPInt();

  જો (semantics == (અચલ llvm::fltSemantics *)&APFloatBase::semFloat8E5M3FNU)
    પાછા convertFloat8E5M3FNUAPFloatToAPInt();

  જો (semantics == (અચલ llvm::fltSemantics *)&APFloatBase::semFloat6E3M2FN)
    પાછા convertFloat6E3M2FNAPFloatToAPInt();

  જો (semantics == (અચલ llvm::fltSemantics *)&APFloatBase::semFloat6E2M3FN)
    પાછા convertFloat6E2M3FNAPFloatToAPInt();

  જો (semantics == (અચલ llvm::fltSemantics *)&APFloatBase::semFloat4E2M1FN)
    પાછા convertFloat4E2M1FNAPFloatToAPInt();

  llvm_unreachable("unknown format!");
}

દશાંશ IEEEFloat::convertToFloat() અચલ {
  પ્રતિજ્ઞા(semantics == (અચલ llvm::fltSemantics *)&APFloatBase::semIEEEsingle &&
         "Float semantics are not IEEEsingle");
  APInt api = bitcastToAPInt();
  પાછા api.bitsToFloat();
}

દ્વિગુણ IEEEFloat::convertToDouble() અચલ {
  પ્રતિજ્ઞા(semantics == (અચલ llvm::fltSemantics *)&APFloatBase::semIEEEdouble &&
         "Float semantics are not IEEEdouble");
  APInt api = bitcastToAPInt();
  પાછા api.bitsToDouble();
}

#ifdef HAS_IEE754_FLOAT128
float128 IEEEFloat::convertToQuad() અચલ {
  પ્રતિજ્ઞા(semantics == (અચલ llvm::fltSemantics *)&APFloatBase::semIEEEquad &&
         "Float semantics are not IEEEquads");
  APInt api = bitcastToAPInt();
  પાછા api.bitsToQuad();
}
#endif

શૂન્ય IEEEFloat::initFromF80LongDoubleAPInt(અચલ APInt &api) {
  પાછા initFromIEEEAPInt<APFloatBase::semX87DoubleExtended>(api);
}

શૂન્ય IEEEFloat::initFromPPCDoubleDoubleLegacyAPInt(અચલ APInt &api) {
  uint64_t i1 = api.getRawData()[0];
  uint64_t i2 = api.getRawData()[1];
  તાર્કિક losesInfo;

  // Get the first double and convert to our format.
  initFromDoubleAPInt(APInt(64, i1));
  [[maybe_unused]] opStatus fs = convert(APFloatBase::semPPCDoubleDoubleLegacy,
                                         rmNearestTiesToEven, &losesInfo);
  // (convert may return opInvalidOp if i1 is an sNaN).
  પ્રતિજ્ઞા((fs == opOK || fs == opInvalidOp) && !losesInfo);

  // Unless we have a special case, add in second double.
  જો (isFiniteNonZero()) {
    IEEEFloat v(APFloatBase::semIEEEdouble, APInt(64, i2));
    // The first double determines the category, so ignore a non-canonical
    // second double.
    જો (v.isFinite()) {
      fs = v.convert(APFloatBase::semPPCDoubleDoubleLegacy, rmNearestTiesToEven,
                     &losesInfo);
      પ્રતિજ્ઞા(fs == opOK && !losesInfo);

      add(v, rmNearestTiesToEven);
    }
  }
}

// The E8M0 format has the following characteristics:
// It is an 8-bit unsigned format with only exponents (no actual significand).
// No encodings for {zero, infinities or denorms}.
// NaN is represented by all 1's.
// Bias is 127.
શૂન્ય IEEEFloat::initFromFloat8E8M0FNUAPInt(અચલ APInt &api) {
  initFromIEEEAPInt<APFloatBase::semFloat8E8M0FNU>(api);
}

શૂન્ય IEEEFloat::initFromFloat8E5M3FNUAPInt(અચલ APInt &api) {
  initFromIEEEAPInt<APFloatBase::semFloat8E5M3FNU>(api);
}

template <અચલ fltSemantics &S>
શૂન્ય IEEEFloat::initFromIEEEAPInt(અચલ APInt &api) {
  પ્રતિજ્ઞા(api.getBitWidth() == S.sizeInBits);

  constexpr unsigned પૂર્ણાંક trailing_significand_bits =
      S.precision - 1 + S.hasExplicitIntegerBit;
  constexpr integerPart integer_bit =
      integerPart{1} << (trailing_significand_bits % integerPartWidth);
  constexpr uint64_t significand_mask = integer_bit - 1;
  constexpr unsigned પૂર્ણાંક exponent_bits =
      S.sizeInBits - (S.hasSignedRepr ? 1 : 0) - trailing_significand_bits;
  static_assert(exponent_bits < 64);
  constexpr unsigned પૂર્ણાંક stored_significand_parts =
      part_ગણતરી_For_Bits(trailing_significand_bits + 1);
  constexpr uint64_t exponent_mask = (uint64_t{1} << exponent_bits) - 1;
  constexpr તાર્કિક is_zero_exp_reserved = S.hasDenormals || S.hasZero;
  constexpr પૂર્ણાંક bias = -(S.minExponent - (is_zero_exp_reserved ? 1 : 0));
  constexpr તાર્કિક has_significand = trailing_significand_bits > 0;

  // Copy the bits of the significand. We need to clear out the exponent and
  // sign bit in the last word.
  std::array<integerPart, stored_significand_parts> mysignificand;
  જો constexpr (has_significand) {
    std::copy_n(api.getRawData(), mysignificand.size(), mysignificand.begin());
    જો constexpr (significand_mask != 0 || S.precision >= integerPartWidth) {
      mysignificand[mysignificand.size() - 1] &= significand_mask;
    }
  } નહીંતર {
    std::fill_n(mysignificand.begin(), mysignificand.size(), 0);
    // Always set integer bit to 1 for consistency in APFloat's internal
    // representation.
    mysignificand[0] = 1;
  }

  // We assume the last word holds the sign bit, the exponent, and potentially
  // some of the trailing significand field.
  uint64_t last_word = api.getRawData()[api.get_અંક_શબ્દો() - 1];
  uint64_t myexponent =
      (last_word >> (trailing_significand_bits % 64)) & exponent_mask;

  initialize(&S);
  પ્રતિજ્ઞા(part_ગણતરી() == mysignificand.size());

  sign = S.hasSignedRepr
             ? static_cast<unsigned પૂર્ણાંક>(last_word >> ((S.sizeInBits - 1) % 64))
             : 0;

  તાર્કિક all_zero_significand =
      has_significand && llvm::all_of(mysignificand, equal_to(0));

  તાર્કિક is_zero = myexponent == 0 && all_zero_significand && S.hasZero;

  જો constexpr (S.nonFiniteBehavior == fltNonfiniteBehavior::IEEE754) {
    તાર્કિક is_inf = ખોટું;

    જો constexpr (S.hasExplicitIntegerBit) {
      // This is only used and tested for x87DoubleExtended
      static_assert(S.precision == 64);
      constexpr integerPart significand_mask_no_int_bit =
          (uint64_t{1} << (trailing_significand_bits - 1)) - 1;
      અચલ integerPart myintegerbit =
          mysignificand[0] >> (trailing_significand_bits - 1);

      is_inf = myexponent - bias == ::exponentInf(S) && myintegerbit == 1 &&
               (mysignificand[0] & significand_mask_no_int_bit) == 0;
    } નહીંતર {
      is_inf = myexponent - bias == ::exponentInf(S) && all_zero_significand;
    }

    જો (is_inf) {
      makeInf(sign);
      પાછા;
    }
  }

  તાર્કિક is_nan = ખોટું;

  જો constexpr (S.nanEncoding == fltNanEncoding::IEEE) {
    જો constexpr (S.hasExplicitIntegerBit) {
      // This is only used and tested for x87DoubleExtended
      static_assert(S.precision == 64);
      અચલ integerPart myintegerbit =
          mysignificand[0] >> (trailing_significand_bits - 1);
      constexpr integerPart significand_mask_no_int_bit =
          (uint64_t{1} << (trailing_significand_bits - 1)) - 1;

      જો (myexponent - bias == ::exponentNaN(S) &&
          (mysignificand[0] & significand_mask_no_int_bit) != 0) {
        // regular NaN and pseudoNaN
        is_nan = સાચું;
      } નહીંતર જો (myexponent - bias == ::exponentNaN(S) &&
                 (mysignificand[0] & significand_mask_no_int_bit) == 0) {
        // pseudoinfinity
        is_nan = સાચું;
      } નહીંતર જો (myexponent - bias != ::exponentNaN(S) && myexponent != 0 &&
                 myintegerbit == 0) {
        // unnormal
        is_nan = સાચું;
      }
    } નહીંતર {
      is_nan = myexponent - bias == ::exponentNaN(S) && !all_zero_significand;
    }
  } નહીંતર જો constexpr (S.nanEncoding == fltNanEncoding::AllOnes) {
    તાર્કિક all_ones_significand =
        std::all_of(mysignificand.begin(), mysignificand.end() - 1,
                    [](integerPart bits) { પાછા bits == ~integerPart{0}; }) &&
        (!significand_mask ||
         mysignificand[mysignificand.size() - 1] == significand_mask);
    is_nan = myexponent - bias == ::exponentNaN(S) && all_ones_significand;
  } નહીંતર જો constexpr (S.nanEncoding == fltNanEncoding::NegativeZero) {
    is_nan = is_zero && sign;
  }

  જો (is_nan) {
    category = fcNaN;
    exponent = ::exponentNaN(S);
    std::copy_n(mysignificand.begin(), mysignificand.size(),
                significandParts());
    પાછા;
  }

  જો (is_zero) {
    makeZero(sign);
    પાછા;
  }

  category = fcNormal;
  exponent = myexponent - bias;
  std::copy_n(mysignificand.begin(), mysignificand.size(), significandParts());
  જો (myexponent == 0 && S.hasDenormals) // denormal
    exponent = S.minExponent;
  નહીંતર {
    જો constexpr (!S.hasExplicitIntegerBit) {
      significandParts()[mysignificand.size() - 1] |= integer_bit;
    }
  }
}

શૂન્ય IEEEFloat::initFromQuadrupleAPInt(અચલ APInt &api) {
  initFromIEEEAPInt<APFloatBase::semIEEEquad>(api);
}

શૂન્ય IEEEFloat::initFromDoubleAPInt(અચલ APInt &api) {
  initFromIEEEAPInt<APFloatBase::semIEEEdouble>(api);
}

શૂન્ય IEEEFloat::initFromFloatAPInt(અચલ APInt &api) {
  initFromIEEEAPInt<APFloatBase::semIEEEsingle>(api);
}

શૂન્ય IEEEFloat::initFromBFloatAPInt(અચલ APInt &api) {
  initFromIEEEAPInt<APFloatBase::semBFloat>(api);
}

શૂન્ય IEEEFloat::initFromHalfAPInt(અચલ APInt &api) {
  initFromIEEEAPInt<APFloatBase::semIEEEhalf>(api);
}

શૂન્ય IEEEFloat::initFromFloat8E5M2APInt(અચલ APInt &api) {
  initFromIEEEAPInt<APFloatBase::semFloat8E5M2>(api);
}

શૂન્ય IEEEFloat::initFromFloat8E5M2FNUZAPInt(અચલ APInt &api) {
  initFromIEEEAPInt<APFloatBase::semFloat8E5M2FNUZ>(api);
}

શૂન્ય IEEEFloat::initFromFloat8E4M3APInt(અચલ APInt &api) {
  initFromIEEEAPInt<APFloatBase::semFloat8E4M3>(api);
}

શૂન્ય IEEEFloat::initFromFloat8E4M3FNAPInt(અચલ APInt &api) {
  initFromIEEEAPInt<APFloatBase::semFloat8E4M3FN>(api);
}

શૂન્ય IEEEFloat::initFromFloat8E4M3FNUZAPInt(અચલ APInt &api) {
  initFromIEEEAPInt<APFloatBase::semFloat8E4M3FNUZ>(api);
}

શૂન્ય IEEEFloat::initFromFloat8E4M3B11FNUZAPInt(અચલ APInt &api) {
  initFromIEEEAPInt<APFloatBase::semFloat8E4M3B11FNUZ>(api);
}

શૂન્ય IEEEFloat::initFromFloat8E3M4APInt(અચલ APInt &api) {
  initFromIEEEAPInt<APFloatBase::semFloat8E3M4>(api);
}

શૂન્ય IEEEFloat::initFromFloatTF32APInt(અચલ APInt &api) {
  initFromIEEEAPInt<APFloatBase::semFloatTF32>(api);
}

શૂન્ય IEEEFloat::initFromFloat6E3M2FNAPInt(અચલ APInt &api) {
  initFromIEEEAPInt<APFloatBase::semFloat6E3M2FN>(api);
}

શૂન્ય IEEEFloat::initFromFloat6E2M3FNAPInt(અચલ APInt &api) {
  initFromIEEEAPInt<APFloatBase::semFloat6E2M3FN>(api);
}

શૂન્ય IEEEFloat::initFromFloat4E2M1FNAPInt(અચલ APInt &api) {
  initFromIEEEAPInt<APFloatBase::semFloat4E2M1FN>(api);
}

/// Treat api as containing the bits of a floating point number.
શૂન્ય IEEEFloat::initFromAPInt(અચલ fltSemantics *Sem, અચલ APInt &api) {
  પ્રતિજ્ઞા(api.getBitWidth() == Sem->sizeInBits);
  જો (Sem == &APFloatBase::semIEEEhalf)
    પાછા initFromHalfAPInt(api);
  જો (Sem == &APFloatBase::semBFloat)
    પાછા initFromBFloatAPInt(api);
  જો (Sem == &APFloatBase::semIEEEsingle)
    પાછા initFromFloatAPInt(api);
  જો (Sem == &APFloatBase::semIEEEdouble)
    પાછા initFromDoubleAPInt(api);
  જો (Sem == &APFloatBase::semIEEEquad)
    પાછા initFromQuadrupleAPInt(api);
  જો (Sem == &APFloatBase::semX87DoubleExtended)
    પાછા initFromF80LongDoubleAPInt(api);
  જો (Sem == &APFloatBase::semPPCDoubleDoubleLegacy)
    પાછા initFromPPCDoubleDoubleLegacyAPInt(api);
  જો (Sem == &APFloatBase::semFloat8E5M2)
    પાછા initFromFloat8E5M2APInt(api);
  જો (Sem == &APFloatBase::semFloat8E5M2FNUZ)
    પાછા initFromFloat8E5M2FNUZAPInt(api);
  જો (Sem == &APFloatBase::semFloat8E4M3)
    પાછા initFromFloat8E4M3APInt(api);
  જો (Sem == &APFloatBase::semFloat8E4M3FN)
    પાછા initFromFloat8E4M3FNAPInt(api);
  જો (Sem == &APFloatBase::semFloat8E4M3FNUZ)
    પાછા initFromFloat8E4M3FNUZAPInt(api);
  જો (Sem == &APFloatBase::semFloat8E4M3B11FNUZ)
    પાછા initFromFloat8E4M3B11FNUZAPInt(api);
  જો (Sem == &APFloatBase::semFloat8E3M4)
    પાછા initFromFloat8E3M4APInt(api);
  જો (Sem == &APFloatBase::semFloatTF32)
    પાછા initFromFloatTF32APInt(api);
  જો (Sem == &APFloatBase::semFloat8E8M0FNU)
    પાછા initFromFloat8E8M0FNUAPInt(api);
  જો (Sem == &APFloatBase::semFloat8E5M3FNU)
    પાછા initFromFloat8E5M3FNUAPInt(api);
  જો (Sem == &APFloatBase::semFloat6E3M2FN)
    પાછા initFromFloat6E3M2FNAPInt(api);
  જો (Sem == &APFloatBase::semFloat6E2M3FN)
    પાછા initFromFloat6E2M3FNAPInt(api);
  જો (Sem == &APFloatBase::semFloat4E2M1FN)
    પાછા initFromFloat4E2M1FNAPInt(api);

  llvm_unreachable("unsupported semantics");
}

/// Make this number the largest magnitude normal number in the given
/// semantics.
શૂન્ય IEEEFloat::makeLargest(તાર્કિક Negative) {
  જો (Negative && !semantics->hasSignedRepr)
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
  unsigned Part_ગણતરી = part_ગણતરી();
  memset(significand, 0xFF, કદ(integerPart)*(Part_ગણતરી - 1));

  // Set the high integerPart especially setting all unused top bits for
  // internal consistency.
  અચલ unsigned અંક_Unused_ઉપલું_Bits =
    Part_ગણતરી*integerPartWidth - semantics->precision;
  significand[Part_ગણતરી - 1] = (અંક_Unused_ઉપલું_Bits < integerPartWidth)
                                   ? (~integerPart(0) >> અંક_Unused_ઉપલું_Bits)
                                   : 0;
  જો (semantics->nonFiniteBehavior == fltNonfiniteBehavior::NanOnly &&
      semantics->nanEncoding == fltNanEncoding::AllOnes &&
      (semantics->precision > 1))
    significand[0] &= ~integerPart(1);
}

/// Make this number the smallest magnitude denormal number in the given
/// semantics.
શૂન્ય IEEEFloat::makeSmallest(તાર્કિક Negative) {
  જો (Negative && !semantics->hasSignedRepr)
    llvm_unreachable(
        "This floating point format does not support signed values");
  // We want (in interchange format):
  //   sign = {Negative}
  //   exponent = 0..0
  //   significand = 0..01
  category = fcNormal;
  sign = Negative;
  exponent = semantics->minExponent;
  APInt::tcSet(significandParts(), 1, part_ગણતરી());
}

શૂન્ય IEEEFloat::makeSmallestNormalized(તાર્કિક Negative) {
  જો (Negative && !semantics->hasSignedRepr)
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

IEEEFloat::IEEEFloat(અચલ fltSemantics &Sem, અચલ APInt &API) {
  initFromAPInt(&Sem, API);
}

IEEEFloat::IEEEFloat(દશાંશ f) {
  initFromAPInt(&APFloatBase::semIEEEsingle, APInt::floatToBits(f));
}

IEEEFloat::IEEEFloat(દ્વિગુણ d) {
  initFromAPInt(&APFloatBase::semIEEEdouble, APInt::doubleToBits(d));
}

નામાકાશ {
  શૂન્ય append(SmallVectorImpl<અક્ષર> &Buffer, StringRef Str) {
    Buffer.append(Str.begin(), Str.end());
  }

  /// Removes data from the given significand until it is no more
  /// precise than is required for the desired precision.
  શૂન્ય AdjustToPrecision(APInt &significand,
                         પૂર્ણાંક &exp, unsigned FormatPrecision) {
    unsigned bits = significand.getActiveBits();

    // 196/59 is a very slight overestimate of lg_2(10).
    unsigned bitsRequired = (FormatPrecision * 196 + 58) / 59;

    જો (bits <= bitsRequired) પાછા;

    unsigned tensRemovable = (bits - bitsRequired) * 59 / 196;
    જો (!tensRemovable) પાછા;

    exp += tensRemovable;

    APInt ભાજક(significand.getBitWidth(), 1);
    APInt powten(significand.getBitWidth(), 10);
    જ્યારે (સાચું) {
      જો (tensRemovable & 1)
        ભાજક *= powten;
      tensRemovable >>= 1;
      જો (!tensRemovable) તોડો;
      powten *= powten;
    }

    significand = significand.udiv(ભાજક);

    // Truncate the significand down to its active bit count.
    significand = significand.trunc(significand.getActiveBits());
  }


  શૂન્ય AdjustToPrecision(SmallVectorImpl<અક્ષર> &buffer,
                         પૂર્ણાંક &exp, unsigned FormatPrecision) {
    unsigned N = buffer.size();
    જો (N <= FormatPrecision) પાછા;

    // The most significant figures are the last ones in the buffer.
    unsigned FirstSignificant = N - FormatPrecision;

    // Round.
    // FIXME: this probably shouldn't use 'round half up'.

    // Rounding down is just a truncation, except we also want to drop
    // trailing zeros from the new result.
    જો (buffer[FirstSignificant - 1] < '5') {
      જ્યારે (FirstSignificant < N && buffer[FirstSignificant] == '0')
        FirstSignificant++;

      exp += FirstSignificant;
      buffer.erase(&buffer[0], &buffer[FirstSignificant]);
      પાછા;
    }

    // Rounding up requires a decimal add-with-carry.  If we continue
    // the carry, the newly-introduced zeros will just be truncated.
    માટે (unsigned I = FirstSignificant; I != N; ++I) {
      જો (buffer[I] == '9') {
        FirstSignificant++;
      } નહીંતર {
        buffer[I]++;
        તોડો;
      }
    }

    // If we carried through, we have exactly one digit of precision.
    જો (FirstSignificant == N) {
      exp += FirstSignificant;
      buffer.clear();
      buffer.ધકેલો_back('1');
      પાછા;
    }

    exp += FirstSignificant;
    buffer.erase(&buffer[0], &buffer[FirstSignificant]);
  }

  શૂન્ય toStringImpl(SmallVectorImpl<અક્ષર> &Str, અચલ તાર્કિક isNeg, પૂર્ણાંક exp,
                    APInt significand, unsigned FormatPrecision,
                    unsigned FormatMaxPadding, તાર્કિક TruncateZero) {
    અચલ પૂર્ણાંક semanticsPrecision = significand.getBitWidth();

    જો (isNeg)
      Str.ધકેલો_back('-');

    // Set FormatPrecision if zero.  We want to do this before we
    // truncate trailing zeros, as those are part of the precision.
    જો (!FormatPrecision) {
      // We use enough digits so the number can be round-tripped back to an
      // APFloat. The formula comes from "How to Print Floating-Point Numbers
      // Accurately" by Steele and White.
      // FIXME: Using a formula based purely on the precision is conservative;
      // we can print fewer digits depending on the actual value being printed.

      // FormatPrecision = 2 + floor(significandBits / lg_2(10))
      FormatPrecision = 2 + semanticsPrecision * 59 / 196;
    }

    // Ignore trailing binary zeros.
    પૂર્ણાંક trailingZeros = significand.countr_zero();
    exp += trailingZeros;
    significand.lshrInPlace(trailingZeros);

    // Change the exponent from 2^e to 10^e.
    જો (exp == 0) {
      // Nothing to do.
    } નહીંતર જો (exp > 0) {
      // Just shift left.
      significand = significand.zext(semanticsPrecision + exp);
      significand <<= exp;
      exp = 0;
    } નહીંતર { /* exp < 0 */
      પૂર્ણાંક texp = -exp;

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
      જ્યારે (સાચું) {
        જો (texp & 1)
          significand *= five_to_the_i;

        texp >>= 1;
        જો (!texp)
          તોડો;
        five_to_the_i *= five_to_the_i;
      }
    }

    AdjustToPrecision(significand, exp, FormatPrecision);

    SmallVector<અક્ષર, 256> buffer;

    // Fill the buffer.
    unsigned precision = significand.getBitWidth();
    જો (precision < 4) {
      // We need enough precision to store the value 10.
      precision = 4;
      significand = significand.zext(precision);
    }
    APInt ten(precision, 10);
    APInt digit(precision, 0);

    તાર્કિક inTrail = સાચું;
    જ્યારે (significand != 0) {
      // digit <- significand % 10
      // significand <- significand / 10
      APInt::udivrem(significand, ten, significand, digit);

      unsigned d = digit.get_ZExt_કિંમત();

      // Drop trailing zeros.
      જો (inTrail && !d)
        exp++;
      નહીંતર {
        buffer.ધકેલો_back((અક્ષર) ('0' + d));
        inTrail = ખોટું;
      }
    }

    પ્રતિજ્ઞા(!buffer.empty() && "no characters in buffer!");

    // Drop down to FormatPrecision.
    // TODO: don't do more precise calculations above than are required.
    AdjustToPrecision(buffer, exp, FormatPrecision);

    unsigned NDigits = buffer.size();

    // Check whether we should use scientific notation.
    તાર્કિક FormatScientific;
    જો (!FormatMaxPadding) {
      FormatScientific = સાચું;
    } નહીંતર {
      જો (exp >= 0) {
        // 765e3 --> 765000
        //              ^^^
        // But we shouldn't make the number look more precise than it is.
        FormatScientific = ((unsigned) exp > FormatMaxPadding ||
                            NDigits + (unsigned) exp > FormatPrecision);
      } નહીંતર {
        // Power of the most significant digit.
        પૂર્ણાંક MSD = exp + (પૂર્ણાંક) (NDigits - 1);
        જો (MSD >= 0) {
          // 765e-2 == 7.65
          FormatScientific = ખોટું;
        } નહીંતર {
          // 765e-5 == 0.00765
          //           ^ ^^
          FormatScientific = ((unsigned) -MSD) > FormatMaxPadding;
        }
      }
    }

    // Scientific formatting is pretty straightforward.
    જો (FormatScientific) {
      exp += (NDigits - 1);

      Str.ધકેલો_back(buffer[NDigits-1]);
      Str.ધકેલો_back('.');
      જો (NDigits == 1 && TruncateZero)
        Str.ધકેલો_back('0');
      નહીંતર
        માટે (unsigned I = 1; I != NDigits; ++I)
          Str.ધકેલો_back(buffer[NDigits-1-I]);
      // Fill with zeros up to FormatPrecision.
      જો (!TruncateZero && FormatPrecision > NDigits - 1)
        Str.append(FormatPrecision - NDigits + 1, '0');
      // For !TruncateZero we use lower 'e'.
      Str.ધકેલો_back(TruncateZero ? 'E' : 'e');

      Str.ધકેલો_back(exp >= 0 ? '+' : '-');
      જો (exp < 0)
        exp = -exp;
      SmallVector<અક્ષર, 6> expbuf;
      કરો {
        expbuf.ધકેલો_back((અક્ષર) ('0' + (exp % 10)));
        exp /= 10;
      } જ્યારે (exp);
      // Exponent always at least two digits if we do not truncate zeros.
      જો (!TruncateZero && expbuf.size() < 2)
        expbuf.ધકેલો_back('0');
      માટે (unsigned I = 0, E = expbuf.size(); I != E; ++I)
        Str.ધકેલો_back(expbuf[E-1-I]);
      પાછા;
    }

    // Non-scientific, positive exponents.
    જો (exp >= 0) {
      માટે (unsigned I = 0; I != NDigits; ++I)
        Str.ધકેલો_back(buffer[NDigits-1-I]);
      માટે (unsigned I = 0; I != (unsigned) exp; ++I)
        Str.ધકેલો_back('0');
      પાછા;
    }

    // Non-scientific, negative exponents.

    // The number of digits to the left of the decimal point.
    પૂર્ણાંક NWholeDigits = exp + (પૂર્ણાંક) NDigits;

    unsigned I = 0;
    જો (NWholeDigits > 0) {
      માટે (; I != (unsigned) NWholeDigits; ++I)
        Str.ધકેલો_back(buffer[NDigits-I-1]);
      Str.ધકેલો_back('.');
    } નહીંતર {
      unsigned NZeros = 1 + (unsigned) -NWholeDigits;

      Str.ધકેલો_back('0');
      Str.ધકેલો_back('.');
      માટે (unsigned Z = 1; Z != NZeros; ++Z)
        Str.ધકેલો_back('0');
    }

    માટે (; I != NDigits; ++I)
      Str.ધકેલો_back(buffer[NDigits-I-1]);

  }
} // namespace

શૂન્ય IEEEFloat::toString(SmallVectorImpl<અક્ષર> &Str, unsigned FormatPrecision,
                         unsigned FormatMaxPadding, તાર્કિક TruncateZero) અચલ {
  પસંદગી (category) {
  સ્થિતિ fcInfinity:
    જો (isNegative())
      પાછા append(Str, "-Inf");
    નહીંતર
      પાછા append(Str, "+Inf");

  સ્થિતિ fcNaN: પાછા append(Str, "NaN");

  સ્થિતિ fcZero:
    જો (isNegative())
      Str.ધકેલો_back('-');

    જો (!FormatMaxPadding) {
      જો (TruncateZero)
        append(Str, "0.0E+0");
      નહીંતર {
        append(Str, "0.0");
        જો (FormatPrecision > 1)
          Str.append(FormatPrecision - 1, '0');
        append(Str, "e+00");
      }
    } નહીંતર {
      Str.ધકેલો_back('0');
    }
    પાછા;

  સ્થિતિ fcNormal:
    તોડો;
  }

  // Decompose the number into an APInt and an exponent.
  પૂર્ણાંક exp = exponent - ((પૂર્ણાંક) semantics->precision - 1);
  APInt significand(
      semantics->precision,
      ArrayRef(significandParts(), part_ગણતરી_For_Bits(semantics->precision)));

  toStringImpl(Str, isNegative(), exp, significand, FormatPrecision,
               FormatMaxPadding, TruncateZero);

}

પૂર્ણાંક IEEEFloat::getExactLog2Abs() અચલ {
  જો (!isFinite() || isZero())
    પાછા INT_MIN;

  અચલ integerPart *Parts = significandParts();
  અચલ પૂર્ણાંક Part_ગણતરી = part_ગણતરી_For_Bits(semantics->precision);

  પૂર્ણાંક કાઢો_ગણતરી = 0;
  માટે (પૂર્ણાંક i = 0; i < Part_ગણતરી; ++i) {
    કાઢો_ગણતરી += llvm::popcount(Parts[i]);
    જો (કાઢો_ગણતરી > 1)
      પાછા INT_MIN;
  }

  જો (exponent != semantics->minExponent)
    પાછા exponent;

  પૂર્ણાંક CountrParts = 0;
  માટે (પૂર્ણાંક i = 0; i < Part_ગણતરી;
       ++i, CountrParts += APInt::APINT_BITS_PER_WORD) {
    જો (Parts[i] != 0) {
      પાછા exponent - semantics->precision + CountrParts +
             llvm::countr_zero(Parts[i]) + 1;
    }
  }

  llvm_unreachable("didn't find the set bit");
}

તાર્કિક IEEEFloat::isSignaling() અચલ {
  જો (!isNaN())
    પાછા ખોટું;
  જો (semantics->nonFiniteBehavior == fltNonfiniteBehavior::NanOnly ||
      semantics->nonFiniteBehavior == fltNonfiniteBehavior::FiniteOnly)
    પાછા ખોટું;

  // IEEE-754R 2008 6.2.1: A signaling NaN bit string should be encoded with the
  // first bit of the trailing significand being 0.
  પાછા !APInt::tcExtractBit(significandParts(), semantics->precision - 2);
}

/// IEEE-754R 2008 5.3.1: nextUp/nextDown.
///
/// *NOTE* since nextDown(x) = -nextUp(-x), we only implement nextUp with
/// appropriate sign switching before/after the computation.
APFloat::opStatus IEEEFloat::next(તાર્કિક nextDown) {
  // If we are performing nextDown, swap sign so we have -x.
  જો (nextDown)
    changeSign();

  // Compute nextUp(x)
  opStatus result = opOK;

  // Handle each float category separately.
  પસંદગી (category) {
  સ્થિતિ fcInfinity:
    // nextUp(+inf) = +inf
    જો (!isNegative())
      તોડો;
    // nextUp(-inf) = -getLargest()
    makeLargest(સાચું);
    તોડો;
  સ્થિતિ fcNaN:
    // IEEE-754R 2008 6.2 Par 2: nextUp(sNaN) = qNaN. Set Invalid flag.
    // IEEE-754R 2008 6.2: nextUp(qNaN) = qNaN. Must be identity so we do not
    //                     change the payload.
    જો (isSignaling()) {
      result = opInvalidOp;
      // For consistency, propagate the sign of the sNaN to the qNaN.
      makeNaN(ખોટું, isNegative(), nullptr);
    }
    તોડો;
  સ્થિતિ fcZero:
    // nextUp(pm 0) = +getSmallest()
    makeSmallest(ખોટું);
    તોડો;
  સ્થિતિ fcNormal:
    // nextUp(-getSmallest()) = -0
    જો (isSmallest() && isNegative()) {
      APInt::tcSet(significandParts(), 0, part_ગણતરી());
      category = fcZero;
      exponent = 0;
      જો (semantics->nanEncoding == fltNanEncoding::NegativeZero)
        sign = ખોટું;
      જો (!semantics->hasZero)
        makeSmallestNormalized(ખોટું);
      તોડો;
    }

    જો (isLargest() && !isNegative()) {
      જો (semantics->nonFiniteBehavior == fltNonfiniteBehavior::NanOnly) {
        // nextUp(getLargest()) == NAN
        makeNaN();
        તોડો;
      } નહીંતર જો (semantics->nonFiniteBehavior ==
                 fltNonfiniteBehavior::FiniteOnly) {
        // nextUp(getLargest()) == getLargest()
        તોડો;
      } નહીંતર {
        // nextUp(getLargest()) == INFINITY
        APInt::tcSet(significandParts(), 0, part_ગણતરી());
        category = fcInfinity;
        exponent = semantics->maxExponent + 1;
        તોડો;
      }
    }

    // nextUp(normal) == normal + inc.
    જો (isNegative()) {
      // If we are negative, we need to decrement the significand.

      // We only cross a binade boundary that requires adjusting the exponent
      // if:
      //   1. exponent != semantics->minExponent. This implies we are not in the
      //   smallest binade or are dealing with denormals.
      //   2. Our significand excluding the integral bit is all zeros.
      તાર્કિક WillCrossBinadeBoundary =
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
      APInt::tcDecrement(Parts, part_ગણતરી());

      જો (WillCrossBinadeBoundary) {
        // Our result is a normal number. Do the following:
        // 1. Set the integral bit to 1.
        // 2. Decrement the exponent.
        APInt::tcSetBit(Parts, semantics->precision - 1);
        exponent--;
      }
    } નહીંતર {
      // If we are positive, we need to increment the significand.

      // We only cross a binade boundary that requires adjusting the exponent if
      // the input is not a denormal and all of said input's significand bits
      // are set. If all of said conditions are true: clear the significand, set
      // the integral bit to 1, and increment the exponent. If we have a
      // denormal always increment since moving denormals and the numbers in the
      // smallest normal binade have the same exponent in our representation.
      // If there are only exponents, any increment always crosses the
      // BinadeBoundary.
      તાર્કિક WillCrossBinadeBoundary = !APFloat::hasSignificand(*semantics) ||
                                     (!isDenormal() && isSignificandAllOnes());

      જો (WillCrossBinadeBoundary) {
        integerPart *Parts = significandParts();
        APInt::tcSet(Parts, 0, part_ગણતરી());
        APInt::tcSetBit(Parts, semantics->precision - 1);
        પ્રતિજ્ઞા(exponent != semantics->maxExponent &&
               "We can not increment an exponent beyond the maxExponent allowed"
               " by the given floating point semantics.");
        exponent++;
      } નહીંતર {
        incrementSignificand();
      }
    }
    તોડો;
  }

  // If we are performing nextDown, swap sign so we have -nextUp(-x)
  જો (nextDown)
    changeSign();

  પાછા result;
}

APInt IEEEFloat::getNaNPayload() અચલ {
  પ્રતિજ્ઞા(isNaN() && "Can only be called on NaN values");
  // Number of bits in the payload, excluding the (maybe implied) integer bit.
  unsigned Bits = semantics->precision - 1;
  પાછા APInt(Bits, ArrayRef(significandParts(), part_ગણતરી_For_Bits(Bits)));
}

APFloatBase::ExponentType IEEEFloat::exponentNaN() અચલ {
  પાછા ::exponentNaN(*semantics);
}

APFloatBase::ExponentType IEEEFloat::exponentInf() અચલ {
  પાછા ::exponentInf(*semantics);
}

APFloatBase::ExponentType IEEEFloat::exponentZero() અચલ {
  પાછા ::exponentZero(*semantics);
}

શૂન્ય IEEEFloat::makeInf(તાર્કિક Negative) {
  જો (semantics->nonFiniteBehavior == fltNonfiniteBehavior::FiniteOnly)
    llvm_unreachable("This floating point format does not support Inf");

  જો (semantics->nonFiniteBehavior == fltNonfiniteBehavior::NanOnly) {
    // There is no Inf, so make NaN instead.
    makeNaN(ખોટું, Negative);
    પાછા;
  }
  category = fcInfinity;
  sign = Negative;
  exponent = exponentInf();
  APInt::tcSet(significandParts(), 0, part_ગણતરી());
}

શૂન્ય IEEEFloat::makeZero(તાર્કિક Negative) {
  જો (!semantics->hasZero)
    llvm_unreachable("This floating point format does not support Zero");

  category = fcZero;
  sign = Negative;
  જો (semantics->nanEncoding == fltNanEncoding::NegativeZero) {
    // Merge negative zero to positive because 0b10000...000 is used for NaN
    sign = ખોટું;
  }
  exponent = exponentZero();
  APInt::tcSet(significandParts(), 0, part_ગણતરી());
}

શૂન્ય IEEEFloat::makeQuiet() {
  પ્રતિજ્ઞા(isNaN());
  જો (semantics->nonFiniteBehavior != fltNonfiniteBehavior::NanOnly)
    APInt::tcSetBit(significandParts(), semantics->precision - 2);
}

પૂર્ણાંક ilogb(અચલ IEEEFloat &Arg) {
  જો (Arg.isNaN())
    પાછા APFloat::IEK_NaN;
  જો (Arg.isZero())
    પાછા APFloat::IEK_Zero;
  જો (Arg.isInfinity())
    પાછા APFloat::IEK_Inf;
  જો (!Arg.isDenormal())
    પાછા Arg.exponent;

  IEEEFloat Normalized(Arg);
  પૂર્ણાંક SignificandBits = Arg.getSemantics().precision - 1;

  Normalized.exponent += SignificandBits;
  Normalized.normalize(APFloat::rmNearestTiesToEven, lfExactlyZero);
  પાછા Normalized.exponent - SignificandBits;
}

IEEEFloat scalbn(IEEEFloat X, પૂર્ણાંક Exp, roundingMode RoundingMode) {
  auto MaxExp = X.getSemantics().maxExponent;
  auto MinExp = X.getSemantics().minExponent;

  // If Exp is wildly out-of-scale, simply adding it to X.exponent will
  // overflow; clamp it to a safe range before adding, but ensure that the range
  // is large enough that the clamp does not change the result. The range we
  // need to support is the difference between the largest possible exponent and
  // the normalized exponent of half the smallest denormal.

  પૂર્ણાંક SignificandBits = X.getSemantics().precision - 1;
  પૂર્ણાંક MaxIncrement = MaxExp - (MinExp - SignificandBits) + 1;

  // Clamp to one past the range ends to let normalize handle overlflow.
  X.exponent += std::clamp(Exp, -MaxIncrement - 1, MaxIncrement);
  X.normalize(RoundingMode, lfExactlyZero);
  જો (X.isNaN())
    X.makeQuiet();
  પાછા X;
}

IEEEFloat frexp(અચલ IEEEFloat &Val, પૂર્ણાંક &Exp, roundingMode RM) {
  Exp = ilogb(Val);

  // Quiet signalling nans.
  જો (Exp == APFloat::IEK_NaN) {
    IEEEFloat Quiet(Val);
    Quiet.makeQuiet();
    પાછા Quiet;
  }

  જો (Exp == APFloat::IEK_Inf)
    પાછા Val;

  // 1 is added because frexp is defined to return a normalized fraction in
  // +/-[0.5, 1.0), rather than the usual +/-[1.0, 2.0).
  Exp = Exp == APFloat::IEK_Zero ? 0 : Exp + 1;
  પાછા scalbn(Val, -Exp, RM);
}

DoubleAPFloat::DoubleAPFloat(અચલ fltSemantics &S)
    : Semantics(&S),
      Floats(નવું APFloat[2]{APFloat(APFloatBase::semIEEEdouble),
                            APFloat(APFloatBase::semIEEEdouble)}) {
  પ્રતિજ્ઞા(Semantics == &APFloatBase::semPPCDoubleDouble);
}

DoubleAPFloat::DoubleAPFloat(અચલ fltSemantics &S, uninitializedTag)
    : Semantics(&S), Floats(નવું APFloat[2]{
                         APFloat(APFloatBase::semIEEEdouble, uninitialized),
                         APFloat(APFloatBase::semIEEEdouble, uninitialized)}) {
  પ્રતિજ્ઞા(Semantics == &APFloatBase::semPPCDoubleDouble);
}

DoubleAPFloat::DoubleAPFloat(અચલ fltSemantics &S, integerPart I)
    : Semantics(&S),
      Floats(નવું APFloat[2]{APFloat(APFloatBase::semIEEEdouble, I),
                            APFloat(APFloatBase::semIEEEdouble)}) {
  પ્રતિજ્ઞા(Semantics == &APFloatBase::semPPCDoubleDouble);
}

DoubleAPFloat::DoubleAPFloat(અચલ fltSemantics &S, અચલ APInt &I)
    : Semantics(&S),
      Floats(નવું APFloat[2]{
          APFloat(APFloatBase::semIEEEdouble, APInt(64, I.getRawData()[0])),
          APFloat(APFloatBase::semIEEEdouble, APInt(64, I.getRawData()[1]))}) {
  પ્રતિજ્ઞા(Semantics == &APFloatBase::semPPCDoubleDouble);
}

DoubleAPFloat::DoubleAPFloat(અચલ fltSemantics &S, APFloat &&First,
                             APFloat &&Second)
    : Semantics(&S),
      Floats(નવું APFloat[2]{std::move(First), std::move(Second)}) {
  પ્રતિજ્ઞા(Semantics == &APFloatBase::semPPCDoubleDouble);
  પ્રતિજ્ઞા(&Floats[0].getSemantics() == &APFloatBase::semIEEEdouble);
  પ્રતિજ્ઞા(&Floats[1].getSemantics() == &APFloatBase::semIEEEdouble);
}

DoubleAPFloat::DoubleAPFloat(અચલ DoubleAPFloat &RHS)
    : Semantics(RHS.Semantics),
      Floats(RHS.Floats ? નવું APFloat[2]{APFloat(RHS.Floats[0]),
                                         APFloat(RHS.Floats[1])}
                        : nullptr) {
  પ્રતિજ્ઞા(Semantics == &APFloatBase::semPPCDoubleDouble);
}

DoubleAPFloat::DoubleAPFloat(DoubleAPFloat &&RHS)
    : Semantics(RHS.Semantics), Floats(RHS.Floats) {
  RHS.Semantics = &APFloatBase::semBogus;
  RHS.Floats = nullptr;
  પ્રતિજ્ઞા(Semantics == &APFloatBase::semPPCDoubleDouble);
}

DoubleAPFloat &DoubleAPFloat::operator=(અચલ DoubleAPFloat &RHS) {
  જો (Semantics == RHS.Semantics && RHS.Floats) {
    Floats[0] = RHS.Floats[0];
    Floats[1] = RHS.Floats[1];
  } નહીંતર જો (આ != &RHS) {
    આ->~DoubleAPFloat();
    નવું (આ) DoubleAPFloat(RHS);
  }
  પાછા *આ;
}

// Returns a result such that:
// 1. abs(Lo) <= ulp(Hi)/2
// 2. Hi == RTNE(Hi + Lo)
// 3. Hi + Lo == X + Y
//
// Requires that log2(X) >= log2(Y).
સ્થિર std::pair<APFloat, APFloat> fastTwoSum(APFloat X, APFloat Y) {
  જો (!X.isFinite())
    પાછા {X, APFloat::getZero(X.getSemantics(), /*Negative=*/ખોટું)};
  APFloat Hi = X + Y;
  APFloat Delta = Hi - X;
  APFloat Lo = Y - Delta;
  પાછા {Hi, Lo};
}

// Implement addition, subtraction, multiplication and division based on:
// "Software for Doubled-Precision Floating-Point Computations",
// by Seppo Linnainmaa, ACM TOMS vol 7 no 3, September 1981, pages 272-283.
APFloat::opStatus DoubleAPFloat::addImpl(અચલ APFloat &a, અચલ APFloat &aa,
                                         અચલ APFloat &c, અચલ APFloat &cc,
                                         roundingMode RM) {
  પૂર્ણાંક Status = opOK;
  APFloat z = a;
  Status |= z.add(c, RM);
  જો (!z.isFinite()) {
    જો (!z.isInfinity()) {
      Floats[0] = std::move(z);
      Floats[1].makeZero(/* Neg = */ ખોટું);
      પાછા (opStatus)Status;
    }
    Status = opOK;
    auto AComparedToC = a.compare_Absolute_કિંમત(c);
    z = cc;
    Status |= z.add(aa, RM);
    જો (AComparedToC == APFloat::cmpGreaterThan) {
      // z = cc + aa + c + a;
      Status |= z.add(c, RM);
      Status |= z.add(a, RM);
    } નહીંતર {
      // z = cc + aa + a + c;
      Status |= z.add(a, RM);
      Status |= z.add(c, RM);
    }
    જો (!z.isFinite()) {
      Floats[0] = std::move(z);
      Floats[1].makeZero(/* Neg = */ ખોટું);
      પાછા (opStatus)Status;
    }
    Floats[0] = z;
    APFloat zz = aa;
    Status |= zz.add(cc, RM);
    જો (AComparedToC == APFloat::cmpGreaterThan) {
      // Floats[1] = a - z + c + zz;
      Floats[1] = a;
      Status |= Floats[1].subtract(z, RM);
      Status |= Floats[1].add(c, RM);
      Status |= Floats[1].add(zz, RM);
    } નહીંતર {
      // Floats[1] = c - z + a + zz;
      Floats[1] = c;
      Status |= Floats[1].subtract(z, RM);
      Status |= Floats[1].add(a, RM);
      Status |= Floats[1].add(zz, RM);
    }
  } નહીંતર {
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
    જો (zz.isZero() && !zz.isNegative()) {
      Floats[0] = std::move(z);
      Floats[1].makeZero(/* Neg = */ ખોટું);
      પાછા opOK;
    }
    Floats[0] = z;
    Status |= Floats[0].add(zz, RM);
    જો (!Floats[0].isFinite()) {
      Floats[1].makeZero(/* Neg = */ ખોટું);
      પાછા (opStatus)Status;
    }
    Floats[1] = std::move(z);
    Status |= Floats[1].subtract(Floats[0], RM);
    Status |= Floats[1].add(zz, RM);
  }
  પાછા (opStatus)Status;
}

APFloat::opStatus DoubleAPFloat::addWithSpecial(અચલ DoubleAPFloat &LHS,
                                                અચલ DoubleAPFloat &RHS,
                                                DoubleAPFloat &Out,
                                                roundingMode RM) {
  જો (LHS.getCategory() == fcNaN) {
    Out = LHS;
    પાછા opOK;
  }
  જો (RHS.getCategory() == fcNaN) {
    Out = RHS;
    પાછા opOK;
  }
  જો (LHS.getCategory() == fcZero) {
    Out = RHS;
    પાછા opOK;
  }
  જો (RHS.getCategory() == fcZero) {
    Out = LHS;
    પાછા opOK;
  }
  જો (LHS.getCategory() == fcInfinity && RHS.getCategory() == fcInfinity &&
      LHS.isNegative() != RHS.isNegative()) {
    Out.makeNaN(ખોટું, Out.isNegative(), nullptr);
    પાછા opInvalidOp;
  }
  જો (LHS.getCategory() == fcInfinity) {
    Out = LHS;
    પાછા opOK;
  }
  જો (RHS.getCategory() == fcInfinity) {
    Out = RHS;
    પાછા opOK;
  }
  પ્રતિજ્ઞા(LHS.getCategory() == fcNormal && RHS.getCategory() == fcNormal);

  APFloat A(LHS.Floats[0]), AA(LHS.Floats[1]), C(RHS.Floats[0]),
      CC(RHS.Floats[1]);
  પ્રતિજ્ઞા(&A.getSemantics() == &APFloatBase::semIEEEdouble);
  પ્રતિજ્ઞા(&AA.getSemantics() == &APFloatBase::semIEEEdouble);
  પ્રતિજ્ઞા(&C.getSemantics() == &APFloatBase::semIEEEdouble);
  પ્રતિજ્ઞા(&CC.getSemantics() == &APFloatBase::semIEEEdouble);
  પ્રતિજ્ઞા(&Out.Floats[0].getSemantics() == &APFloatBase::semIEEEdouble);
  પ્રતિજ્ઞા(&Out.Floats[1].getSemantics() == &APFloatBase::semIEEEdouble);
  પાછા Out.addImpl(A, AA, C, CC, RM);
}

APFloat::opStatus DoubleAPFloat::add(અચલ DoubleAPFloat &RHS,
                                     roundingMode RM) {
  પાછા addWithSpecial(*આ, RHS, *આ, RM);
}

APFloat::opStatus DoubleAPFloat::subtract(અચલ DoubleAPFloat &RHS,
                                          roundingMode RM) {
  changeSign();
  auto Ret = add(RHS, RM);
  changeSign();
  પાછા Ret;
}

APFloat::opStatus DoubleAPFloat::multiply(અચલ DoubleAPFloat &RHS,
                                          APFloat::roundingMode RM) {
  અચલ auto &LHS = *આ;
  auto &Out = *આ;
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
  જો (LHS.getCategory() == fcNaN) {
    Out = LHS;
    પાછા opOK;
  }
  જો (RHS.getCategory() == fcNaN) {
    Out = RHS;
    પાછા opOK;
  }
  જો ((LHS.getCategory() == fcZero && RHS.getCategory() == fcInfinity) ||
      (LHS.getCategory() == fcInfinity && RHS.getCategory() == fcZero)) {
    Out.makeNaN(ખોટું, ખોટું, nullptr);
    પાછા opOK;
  }
  જો (LHS.getCategory() == fcZero || LHS.getCategory() == fcInfinity) {
    Out = LHS;
    પાછા opOK;
  }
  જો (RHS.getCategory() == fcZero || RHS.getCategory() == fcInfinity) {
    Out = RHS;
    પાછા opOK;
  }
  પ્રતિજ્ઞા(LHS.getCategory() == fcNormal && RHS.getCategory() == fcNormal &&
         "Special cases not handled exhaustively");

  પૂર્ણાંક Status = opOK;
  APFloat A = Floats[0], B = Floats[1], C = RHS.Floats[0], D = RHS.Floats[1];
  // t = a * c
  APFloat T = A;
  Status |= T.multiply(C, RM);
  જો (!T.isFiniteNonZero()) {
    Floats[0] = std::move(T);
    Floats[1].makeZero(/* Neg = */ ખોટું);
    પાછા (opStatus)Status;
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
  જો (!U.isFinite()) {
    Floats[1].makeZero(/* Neg = */ ખોટું);
  } નહીંતર {
    // Floats[1] = (t - u) + tau
    Status |= T.subtract(U, RM);
    Status |= T.add(Tau, RM);
    Floats[1] = std::move(T);
  }
  પાછા (opStatus)Status;
}

APFloat::opStatus DoubleAPFloat::divide(અચલ DoubleAPFloat &RHS,
                                        APFloat::roundingMode RM) {
  પ્રતિજ્ઞા(Semantics == &APFloatBase::semPPCDoubleDouble &&
         "Unexpected Semantics");
  APFloat Tmp(APFloatBase::semPPCDoubleDoubleLegacy, bitcastToAPInt());
  auto Ret = Tmp.divide(
      APFloat(APFloatBase::semPPCDoubleDoubleLegacy, RHS.bitcastToAPInt()), RM);
  *આ = DoubleAPFloat(APFloatBase::semPPCDoubleDouble, Tmp.bitcastToAPInt());
  પાછા Ret;
}

APFloat::opStatus DoubleAPFloat::remainder(અચલ DoubleAPFloat &RHS) {
  પ્રતિજ્ઞા(Semantics == &APFloatBase::semPPCDoubleDouble &&
         "Unexpected Semantics");
  APFloat Tmp(APFloatBase::semPPCDoubleDoubleLegacy, bitcastToAPInt());
  auto Ret = Tmp.remainder(
      APFloat(APFloatBase::semPPCDoubleDoubleLegacy, RHS.bitcastToAPInt()));
  *આ = DoubleAPFloat(APFloatBase::semPPCDoubleDouble, Tmp.bitcastToAPInt());
  પાછા Ret;
}

APFloat::opStatus DoubleAPFloat::mod(અચલ DoubleAPFloat &RHS) {
  પ્રતિજ્ઞા(Semantics == &APFloatBase::semPPCDoubleDouble &&
         "Unexpected Semantics");
  APFloat Tmp(APFloatBase::semPPCDoubleDoubleLegacy, bitcastToAPInt());
  auto Ret = Tmp.mod(
      APFloat(APFloatBase::semPPCDoubleDoubleLegacy, RHS.bitcastToAPInt()));
  *આ = DoubleAPFloat(APFloatBase::semPPCDoubleDouble, Tmp.bitcastToAPInt());
  પાછા Ret;
}

APFloat::opStatus
DoubleAPFloat::fusedMultiplyAdd(અચલ DoubleAPFloat &Multiplicand,
                                અચલ DoubleAPFloat &Addend,
                                APFloat::roundingMode RM) {
  પ્રતિજ્ઞા(Semantics == &APFloatBase::semPPCDoubleDouble &&
         "Unexpected Semantics");
  APFloat Tmp(APFloatBase::semPPCDoubleDoubleLegacy, bitcastToAPInt());
  auto Ret = Tmp.fusedMultiplyAdd(
      APFloat(APFloatBase::semPPCDoubleDoubleLegacy,
              Multiplicand.bitcastToAPInt()),
      APFloat(APFloatBase::semPPCDoubleDoubleLegacy, Addend.bitcastToAPInt()),
      RM);
  *આ = DoubleAPFloat(APFloatBase::semPPCDoubleDouble, Tmp.bitcastToAPInt());
  પાછા Ret;
}

APFloat::opStatus DoubleAPFloat::roundToIntegral(APFloat::roundingMode RM) {
  પ્રતિજ્ઞા(Semantics == &APFloatBase::semPPCDoubleDouble &&
         "Unexpected Semantics");
  અચલ APFloat &Hi = getFirst();
  અચલ APFloat &Lo = getSecond();

  APFloat RoundedHi = Hi;
  અચલ opStatus HiStatus = RoundedHi.roundToIntegral(RM);

  // We can reduce the problem to just the high part if the input:
  // 1. Represents a non-finite value.
  // 2. Has a component which is zero.
  જો (!Hi.isFiniteNonZero() || Lo.isZero()) {
    Floats[0] = std::move(RoundedHi);
    Floats[1].makeZero(/*Neg=*/ખોટું);
    પાછા HiStatus;
  }

  // Adjust `Rounded` in the direction of `TieBreaker` if `ToRound` was at a
  // halfway point.
  auto RoundToNearestHelper = [](APFloat ToRound, APFloat Rounded,
                                 APFloat TieBreaker) {
    // RoundingError tells us which direction we rounded:
    //   - RoundingError > 0: we rounded up.
    //   - RoundingError < 0: we rounded down.
    // Sterbenz' lemma ensures that RoundingError is exact.
    અચલ APFloat RoundingError = Rounded - ToRound;
    જો (TieBreaker.isNonZero() &&
        TieBreaker.isNegative() != RoundingError.isNegative() &&
        abs(RoundingError).is_Exactly_કિંમત(0.5))
      Rounded.add(
          APFloat::getOne(Rounded.getSemantics(), TieBreaker.isNegative()),
          rmNearestTiesToEven);
    પાછા Rounded;
  };

  // Case 1: Hi is not an integer.
  // Special cases are for rounding modes that are sensitive to ties.
  જો (RoundedHi != Hi) {
    // We need to consider the case where Hi was between two integers and the
    // rounding mode broke the tie when, in fact, Lo may have had a different
    // sign than Hi.
    જો (RM == rmNearestTiesToAway || RM == rmNearestTiesToEven)
      RoundedHi = RoundToNearestHelper(Hi, RoundedHi, Lo);

    Floats[0] = std::move(RoundedHi);
    Floats[1].makeZero(/*Neg=*/ખોટું);
    પાછા HiStatus;
  }

  // Case 2: Hi is an integer.
  // Special cases are for rounding modes which are rounding towards or away from zero.
  RoundingMode LoRoundingMode;
  જો (RM == rmTowardZero)
    // When our input is positive, we want the Lo component rounded toward
    // negative infinity to get the smallest result magnitude. Likewise,
    // negative inputs want the Lo component rounded toward positive infinity.
    LoRoundingMode = isNegative() ? rmTowardPositive : rmTowardNegative;
  નહીંતર
    LoRoundingMode = RM;

  APFloat RoundedLo = Lo;
  અચલ opStatus LoStatus = RoundedLo.roundToIntegral(LoRoundingMode);
  જો (LoRoundingMode == rmNearestTiesToAway)
    // We need to consider the case where Lo was between two integers and the
    // rounding mode broke the tie when, in fact, Hi may have had a different
    // sign than Lo.
    RoundedLo = RoundToNearestHelper(Lo, RoundedLo, Hi);

  // We must ensure that the final result has no overlap between the two APFloat values.
  std::tie(RoundedHi, RoundedLo) = fastTwoSum(RoundedHi, RoundedLo);

  Floats[0] = std::move(RoundedHi);
  Floats[1] = std::move(RoundedLo);
  પાછા LoStatus;
}

શૂન્ય DoubleAPFloat::changeSign() {
  Floats[0].changeSign();
  Floats[1].changeSign();
}

APFloat::cmpResult
DoubleAPFloat::compare_Absolute_કિંમત(અચલ DoubleAPFloat &RHS) અચલ {
  // Compare absolute values of the high parts.
  અચલ cmpResult HiPartCmp = Floats[0].compare_Absolute_કિંમત(RHS.Floats[0]);
  જો (HiPartCmp != cmpEqual)
    પાછા HiPartCmp;

  // Zero, regardless of sign, is equal.
  જો (Floats[1].isZero() && RHS.Floats[1].isZero())
    પાછા cmpEqual;

  // At this point, |this->Hi| == |RHS.Hi|.
  // The magnitude is |Hi+Lo| which is Hi+|Lo| if signs of Hi and Lo are the
  // same, and Hi-|Lo| if signs are different.
  અચલ તાર્કિક ThisIsSubtractive =
      Floats[0].isNegative() != Floats[1].isNegative();
  અચલ તાર્કિક RHSIsSubtractive =
      RHS.Floats[0].isNegative() != RHS.Floats[1].isNegative();

  // Case 1: The low part of 'this' is zero.
  જો (Floats[1].isZero())
    // We are comparing |Hi| vs. |Hi| ± |RHS.Lo|.
    // If RHS is subtractive, its magnitude is smaller.
    // If RHS is additive, its magnitude is larger.
    પાછા RHSIsSubtractive ? cmpGreaterThan : cmpLessThan;

  // Case 2: The low part of 'RHS' is zero (and we know 'this' is not).
  જો (RHS.Floats[1].isZero())
    // We are comparing |Hi| ± |This.Lo| vs. |Hi|.
    // If 'this' is subtractive, its magnitude is smaller.
    // If 'this' is additive, its magnitude is larger.
    પાછા ThisIsSubtractive ? cmpLessThan : cmpGreaterThan;

  // If their natures differ, the additive one is larger.
  જો (ThisIsSubtractive != RHSIsSubtractive)
    પાછા ThisIsSubtractive ? cmpLessThan : cmpGreaterThan;

  // Case 3: Both are additive (Hi+|Lo|) or both are subtractive (Hi-|Lo|).
  // The comparison now depends on the magnitude of the low parts.
  અચલ cmpResult LoPartCmp = Floats[1].compare_Absolute_કિંમત(RHS.Floats[1]);

  જો (ThisIsSubtractive) {
    // Both are subtractive (Hi-|Lo|), so the comparison of |Lo| is inverted.
    જો (LoPartCmp == cmpLessThan)
      પાછા cmpGreaterThan;
    જો (LoPartCmp == cmpGreaterThan)
      પાછા cmpLessThan;
  }

  // If additive, the comparison of |Lo| is direct.
  // If equal, they are equal.
  પાછા LoPartCmp;
}

APFloat::fltCategory DoubleAPFloat::getCategory() અચલ {
  પાછા Floats[0].getCategory();
}

તાર્કિક DoubleAPFloat::isNegative() અચલ { પાછા Floats[0].isNegative(); }

શૂન્ય DoubleAPFloat::makeInf(તાર્કિક Neg) {
  Floats[0].makeInf(Neg);
  Floats[1].makeZero(/* Neg = */ ખોટું);
}

શૂન્ય DoubleAPFloat::makeZero(તાર્કિક Neg) {
  Floats[0].makeZero(Neg);
  Floats[1].makeZero(/* Neg = */ ખોટું);
}

શૂન્ય DoubleAPFloat::makeLargest(તાર્કિક Neg) {
  પ્રતિજ્ઞા(Semantics == &APFloatBase::semPPCDoubleDouble &&
         "Unexpected Semantics");
  Floats[0] =
      APFloat(APFloatBase::semIEEEdouble, APInt(64, 0x7fefffffffffffffull));
  Floats[1] =
      APFloat(APFloatBase::semIEEEdouble, APInt(64, 0x7c8ffffffffffffeull));
  જો (Neg)
    changeSign();
}

શૂન્ય DoubleAPFloat::makeSmallest(તાર્કિક Neg) {
  પ્રતિજ્ઞા(Semantics == &APFloatBase::semPPCDoubleDouble &&
         "Unexpected Semantics");
  Floats[0].makeSmallest(Neg);
  Floats[1].makeZero(/* Neg = */ ખોટું);
}

શૂન્ય DoubleAPFloat::makeSmallestNormalized(તાર્કિક Neg) {
  પ્રતિજ્ઞા(Semantics == &APFloatBase::semPPCDoubleDouble &&
         "Unexpected Semantics");
  Floats[0] =
      APFloat(APFloatBase::semIEEEdouble, APInt(64, 0x0360000000000000ull));
  જો (Neg)
    Floats[0].changeSign();
  Floats[1].makeZero(/* Neg = */ ખોટું);
}

શૂન્ય DoubleAPFloat::makeNaN(તાર્કિક SNaN, તાર્કિક Neg, અચલ APInt *fill) {
  Floats[0].makeNaN(SNaN, Neg, fill);
  Floats[1].makeZero(/* Neg = */ ખોટું);
}

APFloat::cmpResult DoubleAPFloat::compare(અચલ DoubleAPFloat &RHS) અચલ {
  auto Result = Floats[0].compare(RHS.Floats[0]);
  // |Float[0]| > |Float[1]|
  જો (Result == APFloat::cmpEqual)
    પાછા Floats[1].compare(RHS.Floats[1]);
  પાછા Result;
}

તાર્કિક DoubleAPFloat::bitwiseIsEqual(અચલ DoubleAPFloat &RHS) અચલ {
  પાછા Floats[0].bitwiseIsEqual(RHS.Floats[0]) &&
         Floats[1].bitwiseIsEqual(RHS.Floats[1]);
}

hash_code hash_કિંમત(અચલ DoubleAPFloat &Arg) {
  જો (Arg.Floats)
    પાછા hash_combine(hash_કિંમત(Arg.Floats[0]), hash_કિંમત(Arg.Floats[1]));
  પાછા hash_combine(Arg.Semantics);
}

APInt DoubleAPFloat::bitcastToAPInt() અચલ {
  પ્રતિજ્ઞા(Semantics == &APFloatBase::semPPCDoubleDouble &&
         "Unexpected Semantics");
  uint64_t Data[] = {
      Floats[0].bitcastToAPInt().getRawData()[0],
      Floats[1].bitcastToAPInt().getRawData()[0],
  };
  પાછા APInt(128, Data);
}

Expected<APFloat::opStatus> DoubleAPFloat::convertFromString(StringRef S,
                                                             roundingMode RM) {
  પ્રતિજ્ઞા(Semantics == &APFloatBase::semPPCDoubleDouble &&
         "Unexpected Semantics");
  APFloat Tmp(APFloatBase::semPPCDoubleDoubleLegacy);
  auto Ret = Tmp.convertFromString(S, RM);
  *આ = DoubleAPFloat(APFloatBase::semPPCDoubleDouble, Tmp.bitcastToAPInt());
  પાછા Ret;
}

// The double-double lattice of values corresponds to numbers which obey:
// - abs(lo) <= 1/2 * ulp(hi)
// - roundTiesToEven(hi + lo) == hi
//
// nextUp must choose the smallest output > input that follows these rules.
// nexDown must choose the largest output < input that follows these rules.
APFloat::opStatus DoubleAPFloat::next(તાર્કિક nextDown) {
  પ્રતિજ્ઞા(Semantics == &APFloatBase::semPPCDoubleDouble &&
         "Unexpected Semantics");
  // nextDown(x) = -nextUp(-x)
  જો (nextDown) {
    changeSign();
    APFloat::opStatus Result = next(/*nextDown=*/ખોટું);
    changeSign();
    પાછા Result;
  }
  પસંદગી (getCategory()) {
  સ્થિતિ fcInfinity:
    // nextUp(+inf) = +inf
    // nextUp(-inf) = -getLargest()
    જો (isNegative())
      makeLargest(સાચું);
    પાછા opOK;

  સ્થિતિ fcNaN:
    // IEEE-754R 2008 6.2 Par 2: nextUp(sNaN) = qNaN. Set Invalid flag.
    // IEEE-754R 2008 6.2: nextUp(qNaN) = qNaN. Must be identity so we do not
    //                     change the payload.
    જો (getFirst().isSignaling()) {
      // For consistency, propagate the sign of the sNaN to the qNaN.
      makeNaN(ખોટું, isNegative(), nullptr);
      પાછા opInvalidOp;
    }
    પાછા opOK;

  સ્થિતિ fcZero:
    // nextUp(pm 0) = +getSmallest()
    makeSmallest(ખોટું);
    પાછા opOK;

  સ્થિતિ fcNormal:
    તોડો;
  }

  અચલ APFloat &HiOld = getFirst();
  અચલ APFloat &LoOld = getSecond();

  APFloat NextLo = LoOld;
  NextLo.next(/*nextDown=*/ખોટું);

  // We want to admit values where:
  // 1. abs(Lo) <= ulp(Hi)/2
  // 2. Hi == RTNE(Hi + lo)
  auto InLattice = [](અચલ APFloat &Hi, અચલ APFloat &Lo) {
    પાછા Hi + Lo == Hi;
  };

  // Check if (HiOld, nextUp(LoOld) is in the lattice.
  જો (InLattice(HiOld, NextLo)) {
    // Yes, the result is (HiOld, nextUp(LoOld)).
    Floats[1] = std::move(NextLo);

    // TODO: Because we currently rely on semPPCDoubleDoubleLegacy, our maximum
    // value is defined to have exactly 106 bits of precision. This limitation
    // results in semPPCDoubleDouble being unable to reach its maximum canonical
    // value.
    DoubleAPFloat Largest{*Semantics, uninitialized};
    Largest.makeLargest(/*Neg=*/ખોટું);
    જો (compare(Largest) == cmpGreaterThan)
      makeInf(/*Neg=*/ખોટું);

    પાછા opOK;
  }

  // Now we need to handle the cases where (HiOld, nextUp(LoOld)) is not the
  // correct result. We know the new hi component will be nextUp(HiOld) but our
  // lattice rules make it a little ambiguous what the correct NextLo must be.
  APFloat NextHi = HiOld;
  NextHi.next(/*nextDown=*/ખોટું);

  // nextUp(getLargest()) == INFINITY
  જો (NextHi.isInfinity()) {
    makeInf(/*Neg=*/ખોટું);
    પાછા opOK;
  }

  // IEEE 754-2019 5.3.1:
  // "If x is the negative number of least magnitude in x's format, nextUp(x) is
  // -0."
  જો (NextHi.isZero()) {
    makeZero(/*Neg=*/સાચું);
    પાછા opOK;
  }

  // abs(NextLo) must be <= ulp(NextHi)/2. We want NextLo to be as close to
  // negative infinity as possible.
  NextLo = neg(scalbn(harrisonUlp(NextHi), -1, rmTowardZero));
  જો (!InLattice(NextHi, NextLo))
    // RTNE may mean that Lo must be < ulp(NextHi) / 2 so we bump NextLo.
    NextLo.next(/*nextDown=*/ખોટું);

  Floats[0] = std::move(NextHi);
  Floats[1] = std::move(NextLo);

  પાછા opOK;
}

APFloat::opStatus DoubleAPFloat::convertToSignExtendedInteger(
    MutableArrayRef<integerPart> આગત, unsigned પૂર્ણાંક Width, તાર્કિક IsSigned,
    roundingMode RM, તાર્કિક *IsExact) અચલ {
  પ્રતિજ્ઞા(Semantics == &APFloatBase::semPPCDoubleDouble &&
         "Unexpected Semantics");

  // If Hi is not finite, or Lo is zero, the value is entirely represented
  // by Hi. Delegate to the simpler single-APFloat conversion.
  જો (!getFirst().isFiniteNonZero() || getSecond().isZero())
    પાછા getFirst().convertToInteger(આગત, Width, IsSigned, RM, IsExact);

  // First, round the full double-double value to an integral value. This
  // simplifies the rest of the function, as we no longer need to consider
  // fractional parts.
  *IsExact = ખોટું;
  DoubleAPFloat Integral = *આ;
  અચલ opStatus RoundStatus = Integral.roundToIntegral(RM);
  જો (RoundStatus == opInvalidOp)
    પાછા opInvalidOp;
  અચલ APFloat &IntegralHi = Integral.getFirst();
  અચલ APFloat &IntegralLo = Integral.getSecond();

  // If rounding results in either component being zero, the sum is trivial.
  // Delegate to the simpler single-APFloat conversion.
  તાર્કિક HiIsExact;
  જો (IntegralHi.isZero() || IntegralLo.isZero()) {
    અચલ opStatus HiStatus =
        IntegralHi.convertToInteger(આગત, Width, IsSigned, RM, &HiIsExact);
    // The conversion from an integer-valued float to an APInt may fail if the
    // result would be out of range.  Regardless, taking this path is only
    // possible if rounding occurred during the initial `roundToIntegral`.
    પાછા HiStatus == opOK ? opInexact : HiStatus;
  }

  // A negative number cannot be represented by an unsigned integer.
  // Since a double-double is canonical, if Hi is negative, the sum is negative.
  જો (!IsSigned && IntegralHi.isNegative())
    પાછા opInvalidOp;

  // Handle the special boundary case where |Hi| is exactly the power of two
  // that marks the edge of the integer's range (e.g., 2^63 for int64_t). In
  // this situation, Hi itself won't fit, but the sum Hi + Lo might.
  // `PositiveOverflowWidth` is the bit number for this boundary (N-1 for
  // signed, N for unsigned).
  તાર્કિક LoIsExact;
  અચલ પૂર્ણાંક HiExactLog2 = IntegralHi.getExactLog2Abs();
  અચલ unsigned PositiveOverflowWidth = IsSigned ? Width - 1 : Width;
  જો (HiExactLog2 >= 0 &&
      static_cast<unsigned>(HiExactLog2) == PositiveOverflowWidth) {
    // If Hi and Lo have the same sign, |Hi + Lo| > |Hi|, so the sum is
    // guaranteed to overflow. E.g., for uint128_t, (2^128, 1) overflows.
    જો (IntegralHi.isNegative() == IntegralLo.isNegative())
      પાછા opInvalidOp;

    // If the signs differ, the sum will fit. We can compute the result using
    // properties of two's complement arithmetic without a wide intermediate
    // integer. E.g., for uint128_t, (2^128, -1) should be 2^128 - 1.
    અચલ opStatus LoStatus = IntegralLo.convertToInteger(
        આગત, Width, /*IsSigned=*/સાચું, RM, &LoIsExact);
    જો (LoStatus == opInvalidOp)
      પાછા opInvalidOp;

    // Adjust the bit pattern of Lo to account for Hi's value:
    //  - For unsigned (Hi=2^Width): `2^Width + Lo` in `Width`-bit
    //    arithmetic is equivalent to just `Lo`. The conversion of `Lo` above
    //    already produced the correct final bit pattern.
    //  - For signed (Hi=2^(Width-1)): The sum `2^(Width-1) + Lo` (where Lo<0)
    //    can be computed by taking the two's complement pattern for `Lo` and
    //    clearing the sign bit.
    જો (IsSigned && !IntegralHi.isNegative())
      APInt::tcClearBit(આગત.data(), PositiveOverflowWidth);
    *IsExact = RoundStatus == opOK;
    પાછા RoundStatus;
  }

  // Convert Hi into an integer.  This may not fit but that is OK: we know that
  // Hi + Lo would not fit either in this situation.
  અચલ opStatus HiStatus = IntegralHi.convertToInteger(
      આગત, Width, IsSigned, rmTowardZero, &HiIsExact);
  જો (HiStatus == opInvalidOp)
    પાછા HiStatus;

  // Convert Lo into a temporary integer of the same width.
  APSInt LoResult{Width, /*isUnsigned=*/!IsSigned};
  અચલ opStatus LoStatus =
      IntegralLo.convertToInteger(LoResult, rmTowardZero, &LoIsExact);
  જો (LoStatus == opInvalidOp)
    પાછા LoStatus;

  // Add Lo to Hi. This addition is guaranteed not to overflow because of the
  // double-double canonicalization rule (`|Lo| <= ulp(Hi)/2`). The only case
  // where the sum could cross the integer type's boundary is when Hi is a
  // power of two, which is handled by the special case block above.
  APInt::tcAdd(આગત.data(), LoResult.getRawData(), /*carry=*/0, આગત.size());

  *IsExact = RoundStatus == opOK;
  પાછા RoundStatus;
}

APFloat::opStatus
DoubleAPFloat::convertToInteger(MutableArrayRef<integerPart> આગત,
                                unsigned પૂર્ણાંક Width, તાર્કિક IsSigned,
                                roundingMode RM, તાર્કિક *IsExact) અચલ {
  opStatus FS =
      convertToSignExtendedInteger(આગત, Width, IsSigned, RM, IsExact);

  જો (FS == opInvalidOp) {
    અચલ unsigned Dst_Parts_ગણતરી = part_ગણતરી_For_Bits(Width);
    પ્રતિજ્ઞા(Dst_Parts_ગણતરી <= આગત.size() && "Integer too big");

    unsigned Bits;
    જો (getCategory() == fcNaN)
      Bits = 0;
    નહીંતર જો (isNegative())
      Bits = IsSigned;
    નહીંતર
      Bits = Width - IsSigned;

    tcSetLeastSignificantBits(આગત.data(), Dst_Parts_ગણતરી, Bits);
    જો (isNegative() && IsSigned)
      APInt::tc_Shift_ડાબું(આગત.data(), Dst_Parts_ગણતરી, Width - 1);
  }

  પાછા FS;
}

APFloat::opStatus DoubleAPFloat::handleOverflow(roundingMode RM) {
  પસંદગી (RM) {
  સ્થિતિ APFloat::rmTowardZero:
    makeLargest(/*Neg=*/isNegative());
    તોડો;
  સ્થિતિ APFloat::rmTowardNegative:
    જો (isNegative())
      makeInf(/*Neg=*/સાચું);
    નહીંતર
      makeLargest(/*Neg=*/ખોટું);
    તોડો;
  સ્થિતિ APFloat::rmTowardPositive:
    જો (isNegative())
      makeLargest(/*Neg=*/સાચું);
    નહીંતર
      makeInf(/*Neg=*/ખોટું);
    તોડો;
  સ્થિતિ APFloat::rmNearestTiesToAway:
  સ્થિતિ APFloat::rmNearestTiesToEven:
    makeInf(/*Neg=*/isNegative());
    તોડો;
  મૂળભૂત:
    llvm_unreachable("Invalid rounding mode found");
  }
  opStatus S = opInexact;
  જો (!getFirst().isFinite())
    S = static_cast<opStatus>(S | opOverflow);
  પાછા S;
}

APFloat::opStatus DoubleAPFloat::convertFromUnsignedParts(
    અચલ integerPart *Src, unsigned પૂર્ણાંક Src_ગણતરી, roundingMode RM) {
  // Find the most significant bit of the source integer. APInt::tcMSB returns
  // UINT_MAX for a zero value.
  અચલ unsigned SrcMSB = APInt::tcMSB(Src, Src_ગણતરી);
  જો (SrcMSB == UINT_MAX) {
    // The source integer is 0.
    makeZero(/*Neg=*/ખોટું);
    પાછા opOK;
  }

  // Create a minimally-sized APInt to represent the source value.
  અચલ unsigned SrcBitWidth = SrcMSB + 1;
  APSInt SrcInt{APInt{/*numBits=*/SrcBitWidth, ArrayRef(Src, Src_ગણતરી)},
                /*isUnsigned=*/સાચું};

  // Stage 1: Initial Approximation.
  // Convert the source integer SrcInt to the Hi part of the DoubleAPFloat.
  // We use round-to-nearest because it minimizes the initial error, which is
  // crucial for the subsequent steps.
  APFloat Hi{getFirst().getSemantics()};
  Hi.convertFromAPInt(SrcInt, /*IsSigned=*/ખોટું, rmNearestTiesToEven);

  // If the first approximation already overflows, the number is too large.
  // NOTE: The underlying semantics are *more* conservative when choosing to
  // overflow because their notion of ULP is much larger. As such, it is always
  // safe to overflow at the DoubleAPFloat level if the APFloat overflows.
  જો (!Hi.isFinite())
    પાછા handleOverflow(RM);

  // Stage 2: Exact Error Calculation.
  // Calculate the exact error of the first approximation: Error = SrcInt - Hi.
  // This is done by converting Hi back to an integer and subtracting it from
  // the original source.
  તાર્કિક HiAsIntIsExact;
  // Create an integer representation of Hi. Its width is determined by the
  // exponent of Hi, ensuring it's just large enough. This width can exceed
  // SrcBitWidth if the conversion to Hi rounded up to a power of two.
  // accurately when converted back to an integer.
  APSInt HiAsInt{static_cast<uint32_t>(ilogb(Hi) + 1), /*isUnsigned=*/સાચું};
  Hi.convertToInteger(HiAsInt, rmNearestTiesToEven, &HiAsIntIsExact);
  અચલ APInt Error = SrcInt.zext(HiAsInt.getBitWidth()) - HiAsInt;

  // Stage 3: Error Approximation and Rounding.
  // Convert the integer error into the Lo part of the DoubleAPFloat. This step
  // captures the remainder of the original number. The rounding mode for this
  // conversion (LoRM) may need to be adjusted from the user-requested RM to
  // ensure the final sum (Hi + Lo) rounds correctly.
  roundingMode LoRM = RM;
  // Adjustments are only necessary when the initial approximation Hi was an
  // overestimate, making the Error negative.
  જો (Error.isNegative()) {
    જો (RM == rmNearestTiesToAway) {
      // For rmNearestTiesToAway, a tie should round away from zero. Since
      // SrcInt is positive, this means rounding toward +infinity.
      // A standard conversion of a negative Error would round ties toward
      // -infinity, causing the final sum Hi + Lo to be smaller. To
      // counteract this, we detect the tie case and override the rounding
      // mode for Lo to rmTowardPositive.
      અચલ unsigned ErrorActiveBits = Error.getSignificantBits() - 1;
      અચલ unsigned LoPrecision = getSecond().getSemantics().precision;
      જો (ErrorActiveBits > LoPrecision) {
        અચલ unsigned RoundingBoundary = ErrorActiveBits - LoPrecision;
        // A tie occurs when the bits to be truncated are of the form 100...0.
        // This is detected by checking if the number of trailing zeros is
        // exactly one less than the number of bits being truncated.
        જો (Error.ગણતરી_Trailing_Zeros() == RoundingBoundary - 1)
          LoRM = rmTowardPositive;
      }
    } નહીંતર જો (RM == rmTowardZero) {
      // For rmTowardZero, the final positive result must be truncated (rounded
      // down). When Hi is an overestimate, Error is negative. A standard
      // rmTowardZero conversion of Error would make it *less* negative,
      // effectively rounding the final sum Hi + Lo *up*. To ensure the sum
      // rounds down correctly, we force Lo to round toward -infinity.
      LoRM = rmTowardNegative;
    }
  }

  APFloat Lo{getSecond().getSemantics()};
  opStatus Status = Lo.convertFromAPInt(Error, /*IsSigned=*/સાચું, LoRM);

  // Renormalize the pair (Hi, Lo) into a canonical DoubleAPFloat form where the
  // components do not overlap. fastTwoSum performs this operation.
  std::tie(Hi, Lo) = fastTwoSum(Hi, Lo);
  Floats[0] = std::move(Hi);
  Floats[1] = std::move(Lo);

  // A final check for overflow is needed because fastTwoSum can cause a
  // carry-out from Lo that pushes Hi to infinity.
  જો (!getFirst().isFinite())
    પાછા handleOverflow(RM);

  // The largest DoubleAPFloat must be canonical. Values which are larger are
  // not canonical and are equivalent to overflow.
  જો (getFirst().isFiniteNonZero() && Floats[0].isLargest()) {
    DoubleAPFloat Largest{*Semantics};
    Largest.makeLargest(/*Neg=*/ખોટું);
    જો (compare(Largest) == APFloat::cmpGreaterThan)
      પાછા handleOverflow(RM);
  }

  // The final status of the operation is determined by the conversion of the
  // error term. If Lo could represent Error exactly, the entire conversion
  // is exact. Otherwise, it's inexact.
  પાછા Status;
}

APFloat::opStatus DoubleAPFloat::convertFromAPInt(અચલ APInt &આગત,
                                                  તાર્કિક IsSigned,
                                                  roundingMode RM) {
  અચલ તાર્કિક Negate_આગત = IsSigned && આગત.isNegative();
  APInt API = આગત;
  જો (Negate_આગત)
    API.negate();

  અચલ APFloat::opStatus Status =
      convertFromUnsignedParts(API.getRawData(), API.get_અંક_શબ્દો(), RM);
  જો (Negate_આગત)
    changeSign();
  પાછા Status;
}

unsigned પૂર્ણાંક DoubleAPFloat::convertToHexString(અક્ષર *DST,
                                               unsigned પૂર્ણાંક HexDigits,
                                               તાર્કિક UpperCase,
                                               roundingMode RM) અચલ {
  પ્રતિજ્ઞા(Semantics == &APFloatBase::semPPCDoubleDouble &&
         "Unexpected Semantics");
  પાછા APFloat(APFloatBase::semPPCDoubleDoubleLegacy, bitcastToAPInt())
      .convertToHexString(DST, HexDigits, UpperCase, RM);
}

તાર્કિક DoubleAPFloat::isDenormal() અચલ {
  પાછા getCategory() == fcNormal &&
         (Floats[0].isDenormal() || Floats[1].isDenormal() ||
          // (double)(Hi + Lo) == Hi defines a normal number.
          Floats[0] != Floats[0] + Floats[1]);
}

તાર્કિક DoubleAPFloat::isSmallest() અચલ {
  જો (getCategory() != fcNormal)
    પાછા ખોટું;
  DoubleAPFloat Tmp(*આ);
  Tmp.makeSmallest(આ->isNegative());
  પાછા Tmp.compare(*આ) == cmpEqual;
}

તાર્કિક DoubleAPFloat::isSmallestNormalized() અચલ {
  જો (getCategory() != fcNormal)
    પાછા ખોટું;

  DoubleAPFloat Tmp(*આ);
  Tmp.makeSmallestNormalized(આ->isNegative());
  પાછા Tmp.compare(*આ) == cmpEqual;
}

તાર્કિક DoubleAPFloat::isLargest() અચલ {
  જો (getCategory() != fcNormal)
    પાછા ખોટું;
  DoubleAPFloat Tmp(*આ);
  Tmp.makeLargest(આ->isNegative());
  પાછા Tmp.compare(*આ) == cmpEqual;
}

તાર્કિક DoubleAPFloat::isInteger() અચલ {
  પ્રતિજ્ઞા(Semantics == &APFloatBase::semPPCDoubleDouble &&
         "Unexpected Semantics");
  પાછા Floats[0].isInteger() && Floats[1].isInteger();
}

શૂન્ય DoubleAPFloat::toString(SmallVectorImpl<અક્ષર> &Str,
                             unsigned FormatPrecision,
                             unsigned FormatMaxPadding,
                             તાર્કિક TruncateZero) અચલ {
  પ્રતિજ્ઞા(Semantics == &APFloatBase::semPPCDoubleDouble &&
         "Unexpected Semantics");
  APFloat(APFloatBase::semPPCDoubleDoubleLegacy, bitcastToAPInt())
      .toString(Str, FormatPrecision, FormatMaxPadding, TruncateZero);
}

પૂર્ણાંક DoubleAPFloat::getExactLog2Abs() અચલ {
  // In order for Hi + Lo to be a power of two, the following must be true:
  // 1. Hi must be a power of two.
  // 2. Lo must be zero.
  જો (getSecond().isNonZero())
    પાછા INT_MIN;
  પાછા getFirst().getExactLog2Abs();
}

પૂર્ણાંક ilogb(અચલ DoubleAPFloat &Arg) {
  અચલ APFloat &Hi = Arg.getFirst();
  અચલ APFloat &Lo = Arg.getSecond();
  પૂર્ણાંક IlogbResult = ilogb(Hi);
  // Zero and non-finite values can delegate to ilogb(Hi).
  જો (Arg.getCategory() != fcNormal)
    પાછા IlogbResult;
  // If Lo can't change the binade, we can delegate to ilogb(Hi).
  જો (Lo.isZero() || Hi.isNegative() == Lo.isNegative())
    પાછા IlogbResult;
  જો (Hi.getExactLog2Abs() == INT_MIN)
    પાછા IlogbResult;
  // Numbers of the form 2^a - 2^b or -2^a + 2^b are almost powers of two but
  // get nudged out of the binade by the low component.
  પાછા IlogbResult - 1;
}

DoubleAPFloat scalbn(અચલ DoubleAPFloat &Arg, પૂર્ણાંક Exp,
                     APFloat::roundingMode RM) {
  પ્રતિજ્ઞા(Arg.Semantics == &APFloatBase::PPCDoubleDouble() &&
         "Unexpected Semantics");
  પાછા DoubleAPFloat(APFloatBase::PPCDoubleDouble(),
                       scalbn(Arg.Floats[0], Exp, RM),
                       scalbn(Arg.Floats[1], Exp, RM));
}

DoubleAPFloat frexp(અચલ DoubleAPFloat &Arg, પૂર્ણાંક &Exp,
                    APFloat::roundingMode RM) {
  પ્રતિજ્ઞા(Arg.Semantics == &APFloatBase::PPCDoubleDouble() &&
         "Unexpected Semantics");

  // Get the unbiased exponent e of the number, where |Arg| = m * 2^e for m in
  // [1.0, 2.0).
  Exp = ilogb(Arg);

  // For NaNs, quiet any signaling NaN and return the result, as per standard
  // practice.
  જો (Exp == APFloat::IEK_NaN) {
    DoubleAPFloat Quiet{Arg};
    Quiet.getFirst() = Quiet.getFirst().makeQuiet();
    પાછા Quiet;
  }

  // For infinity, return it unchanged. The exponent remains IEK_Inf.
  જો (Exp == APFloat::IEK_Inf)
    પાછા Arg;

  // For zero, the fraction is zero and the standard requires the exponent be 0.
  જો (Exp == APFloat::IEK_Zero) {
    Exp = 0;
    પાછા Arg;
  }

  અચલ APFloat &Hi = Arg.getFirst();
  અચલ APFloat &Lo = Arg.getSecond();

  // frexp requires the fraction's absolute value to be in [0.5, 1.0).
  // ilogb provides an exponent for an absolute value in [1.0, 2.0).
  // Increment the exponent to ensure the fraction is in the correct range.
  ++Exp;

  અચલ તાર્કિક SignsDisagree = Hi.isNegative() != Lo.isNegative();
  APFloat Second = Lo;
  જો (Arg.getCategory() == APFloat::fcNormal && Lo.isFiniteNonZero()) {
    roundingMode LoRoundingMode;
    // The interpretation of rmTowardZero depends on the sign of the combined
    // Arg rather than the sign of the component.
    જો (RM == rmTowardZero)
      LoRoundingMode = Arg.isNegative() ? rmTowardPositive : rmTowardNegative;
    // For rmNearestTiesToAway, we face a similar problem. If signs disagree,
    // Lo is a correction *toward* zero relative to Hi. Rounding Lo
    // "away from zero" based on its own sign would move the value in the
    // wrong direction. As a safe proxy, we use rmNearestTiesToEven, which is
    // direction-agnostic. We only need to bother with this if Lo is scaled
    // down.
    નહીંતર જો (RM == rmNearestTiesToAway && SignsDisagree && Exp > 0)
      LoRoundingMode = rmNearestTiesToEven;
    નહીંતર
      LoRoundingMode = RM;
    Second = scalbn(Lo, -Exp, LoRoundingMode);
    // The rmNearestTiesToEven proxy is correct most of the time, but it
    // differs from rmNearestTiesToAway when the scaled value of Lo is an
    // exact midpoint.
    // NOTE: This is morally equivalent to roundTiesTowardZero.
    જો (RM == rmNearestTiesToAway && LoRoundingMode == rmNearestTiesToEven) {
      // Re-scale the result back to check if rounding occurred.
      અચલ APFloat RecomposedLo = scalbn(Second, Exp, rmNearestTiesToEven);
      જો (RecomposedLo != Lo) {
        // RoundingError tells us which direction we rounded:
        //   - RoundingError > 0: we rounded up.
        //   - RoundingError < 0: we down up.
        અચલ APFloat RoundingError = RecomposedLo - Lo;
        // Determine if scalbn(Lo, -Exp) landed exactly on a midpoint.
        // We do this by checking if the absolute rounding error is exactly
        // half a ULP of the result.
        અચલ APFloat UlpOfSecond = harrisonUlp(Second);
        અચલ APFloat ScaledUlpOfSecond =
            scalbn(UlpOfSecond, Exp - 1, rmNearestTiesToEven);
        અચલ તાર્કિક IsMidpoint = abs(RoundingError) == ScaledUlpOfSecond;
        અચલ તાર્કિક RoundedLoAway =
            Second.isNegative() == RoundingError.isNegative();
        // The sign of Hi and Lo disagree and we rounded Lo away: we must
        // decrease the magnitude of Second to increase the magnitude
        // First+Second.
        જો (IsMidpoint && RoundedLoAway)
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
    જો (Second.isZero() && SignsDisagree && Hi.getExactLog2Abs() != INT_MIN)
      ++Exp;
  }

  APFloat First = scalbn(Hi, -Exp, RM);
  પાછા DoubleAPFloat(APFloatBase::PPCDoubleDouble(), std::move(First),
                       std::move(Second));
}

APInt DoubleAPFloat::getNaNPayload() અચલ { પાછા Floats[0].getNaNPayload(); }
} // namespace detail

APFloat::Storage::Storage(IEEEFloat F, અચલ fltSemantics &Semantics) {
  જો (usesLayout<IEEEFloat>(Semantics)) {
    નવું (&IEEE) IEEEFloat(std::move(F));
    પાછા;
  }
  જો (usesLayout<DoubleAPFloat>(Semantics)) {
    અચલ fltSemantics& S = F.getSemantics();
    નવું (&Double) DoubleAPFloat(Semantics, APFloat(std::move(F), S),
                                APFloat(APFloatBase::IEEEdouble()));
    પાછા;
  }
  llvm_unreachable("Unexpected semantics");
}

Expected<APFloat::opStatus> APFloat::convertFromString(StringRef Str,
                                                       roundingMode RM) {
  APFLOAT_DISPATCH_ON_SEMANTICS(convertFromString(Str, RM));
}

hash_code hash_કિંમત(અચલ APFloat &Arg) {
  જો (APFloat::usesLayout<detail::IEEEFloat>(Arg.getSemantics()))
    પાછા hash_કિંમત(Arg.U.IEEE);
  જો (APFloat::usesLayout<detail::DoubleAPFloat>(Arg.getSemantics()))
    પાછા hash_કિંમત(Arg.U.Double);
  llvm_unreachable("Unexpected semantics");
}

APFloat::APFloat(અચલ fltSemantics &Semantics, StringRef S)
    : APFloat(Semantics) {
  auto StatusOrErr = convertFromString(S, rmNearestTiesToEven);
  પ્રતિજ્ઞા(StatusOrErr && "Invalid floating point representation");
  consumeError(StatusOrErr.takeError());
}

FPClassTest APFloat::classify() અચલ {
  જો (isZero())
    પાછા isNegative() ? fcNegZero : fcPosZero;
  જો (isNormal())
    પાછા isNegative() ? fcNegNormal : fcPosNormal;
  જો (isDenormal())
    પાછા isNegative() ? fcNegSubnormal : fcPosSubnormal;
  જો (isInfinity())
    પાછા isNegative() ? fcNegInf : fcPosInf;
  પ્રતિજ્ઞા(isNaN() && "Other class of FP constant");
  પાછા isSignaling() ? fcSNan : fcQNan;
}

તાર્કિક APFloat::getExactInverse(APFloat *Inv) અચલ {
  // Only finite, non-zero numbers can have a useful, representable inverse.
  // This check filters out +/- zero, +/- infinity, and NaN.
  જો (!isFiniteNonZero())
    પાછા ખોટું;

  // Historically, this function rejects subnormal inputs.  One reason why this
  // might be important is that subnormals may behave differently under FTZ/DAZ
  // runtime behavior.
  જો (isDenormal())
    પાછા ખોટું;

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
  અચલ પૂર્ણાંક Exp = getExactLog2Abs();
  જો (Exp == INT_MIN)
    પાછા ખોટું;

  // The inverse of +/- 2^Exp is +/- 2^(-Exp). We can compute this by
  // scaling 1.0 by the negated exponent.
  APFloat Reciprocal =
      scalbn(APFloat::getOne(getSemantics(), /*Negative=*/isNegative()), -Exp,
             rmTowardZero);

  // scalbn might round if the resulting exponent -Exp is outside the
  // representable range, causing overflow (to infinity) or underflow. We
  // must verify that the result is still the exact power of two we expect.
  જો (Reciprocal.getExactLog2Abs() != -Exp)
    પાછા ખોટું;

  // Avoid multiplication with a subnormal, it is not safe on all platforms and
  // may be slower than a normal division.
  જો (Reciprocal.isDenormal())
    પાછા ખોટું;

  પ્રતિજ્ઞા(Reciprocal.isFiniteNonZero());

  જો (Inv)
    *Inv = std::move(Reciprocal);

  પાછા સાચું;
}

APFloat::opStatus APFloat::convert(અચલ fltSemantics &ToSemantics,
                                   roundingMode RM, તાર્કિક *losesInfo) {
  જો (&getSemantics() == &ToSemantics) {
    *losesInfo = ખોટું;
    પાછા opOK;
  }
  જો (usesLayout<IEEEFloat>(getSemantics()) &&
      usesLayout<IEEEFloat>(ToSemantics))
    પાછા U.IEEE.convert(ToSemantics, RM, losesInfo);
  જો (usesLayout<IEEEFloat>(getSemantics()) &&
      usesLayout<DoubleAPFloat>(ToSemantics)) {
    પ્રતિજ્ઞા(&ToSemantics == &APFloatBase::semPPCDoubleDouble);
    auto Ret =
        U.IEEE.convert(APFloatBase::semPPCDoubleDoubleLegacy, RM, losesInfo);
    *આ = APFloat(ToSemantics, U.IEEE.bitcastToAPInt());
    પાછા Ret;
  }
  જો (usesLayout<DoubleAPFloat>(getSemantics()) &&
      usesLayout<IEEEFloat>(ToSemantics)) {
    auto Ret = getIEEE().convert(ToSemantics, RM, losesInfo);
    *આ = APFloat(std::move(getIEEE()), ToSemantics);
    પાછા Ret;
  }
  llvm_unreachable("Unexpected semantics");
}

APFloat APFloat::get_All_Ones_કિંમત(અચલ fltSemantics &Semantics) {
  પાછા APFloat(Semantics, APInt::getAllOnes(Semantics.sizeInBits));
}

શૂન્ય APFloat::print(raw_ostream &OS) અચલ {
  SmallVector<અક્ષર, 16> Buffer;
  toString(Buffer);
  OS << Buffer;
}

#જો !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)
LLVM_DUMP_METHOD શૂન્ય APFloat::dump() અચલ {
  print(dbgs());
  dbgs() << '\n';
}
#endif

શૂન્ય APFloat::Profile(Folding_Set_ગાંઠ_ID &NID) અચલ {
  NID.Add(bitcastToAPInt());
}

APFloat::opStatus APFloat::convertToInteger(APSInt &result,
                                            roundingMode rounding_mode,
                                            તાર્કિક *isExact) અચલ {
  unsigned bitWidth = result.getBitWidth();
  SmallVector<uint64_t, 4> parts(result.get_અંક_શબ્દો());
  opStatus status = convertToInteger(parts, bitWidth, result.isSigned(),
                                     rounding_mode, isExact);
  // Keeps the original signed-ness.
  result = APInt(bitWidth, parts);
  પાછા status;
}

દ્વિગુણ APFloat::convertToDouble() અચલ {
  જો (&getSemantics() == &APFloatBase::semIEEEdouble)
    પાછા getIEEE().convertToDouble();
  પ્રતિજ્ઞા(isRepresentableBy(getSemantics(), semIEEEdouble) &&
         "Float semantics is not representable by IEEEdouble");
  APFloat Temp = *આ;
  તાર્કિક LosesInfo;
  [[maybe_unused]] opStatus St =
      Temp.convert(APFloatBase::semIEEEdouble, rmNearestTiesToEven, &LosesInfo);
  પ્રતિજ્ઞા(!(St & opInexact) && !LosesInfo && "Unexpected imprecision");
  પાછા Temp.getIEEE().convertToDouble();
}

#ifdef HAS_IEE754_FLOAT128
float128 APFloat::convertToQuad() અચલ {
  જો (&getSemantics() == &APFloatBase::semIEEEquad)
    પાછા getIEEE().convertToQuad();
  પ્રતિજ્ઞા(isRepresentableBy(getSemantics(), semIEEEquad) &&
         "Float semantics is not representable by IEEEquad");
  APFloat Temp = *આ;
  તાર્કિક LosesInfo;
  [[maybe_unused]] opStatus St =
      Temp.convert(APFloatBase::semIEEEquad, rmNearestTiesToEven, &LosesInfo);
  પ્રતિજ્ઞા(!(St & opInexact) && !LosesInfo && "Unexpected imprecision");
  પાછા Temp.getIEEE().convertToQuad();
}
#endif

દશાંશ APFloat::convertToFloat() અચલ {
  જો (&getSemantics() == &APFloatBase::semIEEEsingle)
    પાછા getIEEE().convertToFloat();
  પ્રતિજ્ઞા(isRepresentableBy(getSemantics(), semIEEEsingle) &&
         "Float semantics is not representable by IEEEsingle");
  APFloat Temp = *આ;
  તાર્કિક LosesInfo;
  [[maybe_unused]] opStatus St =
      Temp.convert(APFloatBase::semIEEEsingle, rmNearestTiesToEven, &LosesInfo);
  પ્રતિજ્ઞા(!(St & opInexact) && !LosesInfo && "Unexpected imprecision");
  પાછા Temp.getIEEE().convertToFloat();
}

unsigned APFloatBase::getArbitraryFPFormatSizeInBits(StringRef Format) {
  પાછા StringSwitch<unsigned>(Format)
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

તાર્કિક APFloatBase::isValidArbitraryFPFormat(StringRef Format) {
  પાછા getArbitraryFPFormatSizeInBits(Format) != 0;
}

અચલ fltSemantics *APFloatBase::getArbitraryFPSemantics(StringRef Format) {
  // TODO: extend to remaining arbitrary FP types: Float8E4M3, Float8E3M4,
  // Float8E5M2FNUZ, Float8E4M3FNUZ, Float8E4M3B11FNUZ, Float8E8M0FNU.
  પાછા StringSwitch<અચલ fltSemantics *>(Format)
      .Case("Float8E5M2", &semFloat8E5M2)
      .Case("Float8E4M3FN", &semFloat8E4M3FN)
      .Case("Float8E5M3FNU", &semFloat8E5M3FNU)
      .Case("Float4E2M1FN", &semFloat4E2M1FN)
      .Case("Float6E3M2FN", &semFloat6E3M2FN)
      .Case("Float6E2M3FN", &semFloat6E2M3FN)
      .Default(nullptr);
}

APFloat::Storage::~Storage() {
  જો (usesLayout<IEEEFloat>(*semantics)) {
    IEEE.~IEEEFloat();
    પાછા;
  }
  જો (usesLayout<DoubleAPFloat>(*semantics)) {
    Double.~DoubleAPFloat();
    પાછા;
  }
  llvm_unreachable("Unexpected semantics");
}

APFloat::Storage::Storage(અચલ APFloat::Storage &RHS) {
  જો (usesLayout<IEEEFloat>(*RHS.semantics)) {
    નવું (આ) IEEEFloat(RHS.IEEE);
    પાછા;
  }
  જો (usesLayout<DoubleAPFloat>(*RHS.semantics)) {
    નવું (આ) DoubleAPFloat(RHS.Double);
    પાછા;
  }
  llvm_unreachable("Unexpected semantics");
}

APFloat::Storage::Storage(APFloat::Storage &&RHS) {
  જો (usesLayout<IEEEFloat>(*RHS.semantics)) {
    નવું (આ) IEEEFloat(std::move(RHS.IEEE));
    પાછા;
  }
  જો (usesLayout<DoubleAPFloat>(*RHS.semantics)) {
    નવું (આ) DoubleAPFloat(std::move(RHS.Double));
    પાછા;
  }
  llvm_unreachable("Unexpected semantics");
}

APFloat::Storage &APFloat::Storage::operator=(અચલ APFloat::Storage &RHS) {
  જો (usesLayout<IEEEFloat>(*semantics) &&
      usesLayout<IEEEFloat>(*RHS.semantics)) {
    IEEE = RHS.IEEE;
  } નહીંતર જો (usesLayout<DoubleAPFloat>(*semantics) &&
             usesLayout<DoubleAPFloat>(*RHS.semantics)) {
    Double = RHS.Double;
  } નહીંતર જો (આ != &RHS) {
    આ->~Storage();
    નવું (આ) Storage(RHS);
  }
  પાછા *આ;
}

APFloat::Storage &APFloat::Storage::operator=(APFloat::Storage &&RHS) {
  જો (usesLayout<IEEEFloat>(*semantics) &&
      usesLayout<IEEEFloat>(*RHS.semantics)) {
    IEEE = std::move(RHS.IEEE);
  } નહીંતર જો (usesLayout<DoubleAPFloat>(*semantics) &&
             usesLayout<DoubleAPFloat>(*RHS.semantics)) {
    Double = std::move(RHS.Double);
  } નહીંતર જો (આ != &RHS) {
    આ->~Storage();
    નવું (આ) Storage(std::move(RHS));
  }
  પાછા *આ;
}

નામાકાશ {

APFloat::opStatus getOpStatusFromLibc(પૂર્ણાંક libc_exceptions) {
  APFloat::opStatus status = APFloat::opOK;
  જો (libc_exceptions & FE_INVALID)
    status = static_cast<APFloat::opStatus>(status | APFloat::opInvalidOp);
  જો (libc_exceptions & FE_DIVBYZERO)
    status = static_cast<APFloat::opStatus>(status | APFloat::opDivByZero);
  જો (libc_exceptions & FE_OVERFLOW)
    status = static_cast<APFloat::opStatus>(status | APFloat::opOverflow);
  જો (libc_exceptions & FE_UNDERFLOW)
    status = static_cast<APFloat::opStatus>(status | APFloat::opUnderflow);
  જો (libc_exceptions & FE_INEXACT)
    status = static_cast<APFloat::opStatus>(status | APFloat::opInexact);
  પાછા status;
}

} // namespace

// TODO: Support other rounding modes when LLVM libc math implement static
// roundings.
std::optional<APFloat> exp(અચલ APFloat &x, RoundingMode rounding_mode,
                           APFloat::opStatus *status) {

  જો (rounding_mode == APFloatBase::rmNearestTiesToEven) {
    જો (APFloat::SemanticsToEnum(x.getSemantics()) ==
        APFloatBase::S_IEEEsingle) {
      દશાંશ x_val = x.convertToFloat();
      પૂર્ણાંક exc =
          LIBC_NAMESPACE::shared::check::exp_exceptions(x_val, FE_TONEAREST);
      જો (status) {
        *status = getOpStatusFromLibc(exc);
        જો (x.isSignaling()) {
          // 32-bit x86 will silence sNaN when loading floats, so we explicitly
          // add the INVALID exception here.
          *status =
              static_cast<APFloat::opStatus>(*status | APFloat::opInvalidOp);
        }
      }
      દશાંશ result = LIBC_NAMESPACE::shared::expf(x_val);
      પાછા APFloat(result);
    }
    જો (APFloat::SemanticsToEnum(x.getSemantics()) ==
        APFloatBase::S_IEEEdouble) {
      દ્વિગુણ x_val = x.convertToDouble();
      પૂર્ણાંક exc =
          LIBC_NAMESPACE::shared::check::exp_exceptions(x_val, FE_TONEAREST);
      જો (status) {
        *status = getOpStatusFromLibc(exc);
        જો (x.isSignaling()) {
          // 32-bit x86 will silence sNaN when loading floats, so we explicitly
          // add the INVALID exception here.
          *status =
              static_cast<APFloat::opStatus>(*status | APFloat::opInvalidOp);
        }
      }
      દ્વિગુણ result = LIBC_NAMESPACE::shared::exp(x_val);
      પાછા APFloat(result);
    }
  }
  પાછા std::nullopt;
}

} // namespace llvm

#undef APFLOAT_DISPATCH_ON_SEMANTICS
