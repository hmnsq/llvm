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

#অন্তর্ভাব "llvm/ADT/APFloat.h"
#অন্তর্ভাব "llvm/ADT/APSInt.h"
#অন্তর্ভাব "llvm/ADT/ArrayRef.h"
#অন্তর্ভাব "llvm/ADT/FloatingPointMode.h"
#অন্তর্ভাব "llvm/ADT/FoldingSet.h"
#অন্তর্ভাব "llvm/ADT/Hashing.h"
#অন্তর্ভাব "llvm/ADT/STLExtras.h"
#অন্তর্ভাব "llvm/ADT/SmallVector.h"
#অন্তর্ভাব "llvm/ADT/StringExtras.h"
#অন্তর্ভাব "llvm/ADT/StringRef.h"
#অন্তর্ভাব "llvm/ADT/StringSwitch.h"
#অন্তর্ভাব "llvm/Config/llvm-config.h"
#অন্তর্ভাব "llvm/Support/Debug.h"
#অন্তর্ভাব "llvm/Support/Error.h"
#অন্তর্ভাব "llvm/Support/MathExtras.h"
#অন্তর্ভাব "llvm/Support/raw_ostream.h"
#অন্তর্ভাব <cstring>
#অন্তর্ভাব <iterator>
#অন্তর্ভাব <limits.h>

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

#অন্তর্ভাব "shared/math.h"
#অন্তর্ভাব "shared/math_check_exceptions.h"

#define APFLOAT_DISPATCH_ON_SEMANTICS(METHOD_CALL)                             \
  কুরু {                                                                         \
    যদি (usesLayout<IEEEFloat>(getSemantics()))                                 \
      প্রত্যাগম U.IEEE.METHOD_CALL;                                               \
    যদি (usesLayout<DoubleAPFloat>(getSemantics()))                             \
      প্রত্যাগম U.Double.METHOD_CALL;                                             \
    llvm_unreachable("Unexpected semantics");                                  \
  } যাবত্ (অসত্য)

using নামাকাশ llvm;

/// A macro used to combine two fcCategory enums into one key which can be used
/// in a switch statement to classify how the interaction of two APFloat's
/// categories affects an operation.
///
/// TODO: If clang source code is ever allowed to use constexpr in its own
/// codebase, change this into a static inline function.
#define Pack_Categories_Into_চাবি(_lhs, _rhs) ((_lhs) * 4 + (_rhs))

/* Assumed in hexadecimal significand parsing, and conversion to
   hexadecimal strings.  */
static_assert(APFloatBase::integerPartWidth % 4 == 0, "Part width must be divisible by 4!");

নামাকাশ llvm {

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
    সত্য,
    সত্য,
    সত্য,
    সত্য,
    সত্য};
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
    অসত্য,
    অসত্য,
    অসত্য,
    অসত্য};

constexpr fltSemantics APFloatBase::semFloat8E5M3FNU = {
    16,
    -14,
    4,
    8,
    fltNonfiniteBehavior::NanOnly,
    fltNanEncoding::AllOnes,
    সত্য,
    অসত্য,
    অসত্য};

constexpr fltSemantics APFloatBase::semFloat6E3M2FN = {
    4, -2, 3, 6, fltNonfiniteBehavior::FiniteOnly};
constexpr fltSemantics APFloatBase::semFloat6E2M3FN = {
    2, 0, 4, 6, fltNonfiniteBehavior::FiniteOnly};
constexpr fltSemantics APFloatBase::semFloat4E2M1FN = {
    2, 0, 2, 4, fltNonfiniteBehavior::FiniteOnly};
constexpr fltSemantics APFloatBase::semBogus = {0, 0, 0, 0};

ধ্রুব llvm::fltSemantics &APFloatBase::EnumToSemantics(Semantics S) {
  বিকল্প (S) {
  পক্ষ S_IEEEhalf:
    প্রত্যাগম IEEEhalf();
  পক্ষ S_BFloat:
    প্রত্যাগম BFloat();
  পক্ষ S_IEEEsingle:
    প্রত্যাগম IEEEsingle();
  পক্ষ S_IEEEdouble:
    প্রত্যাগম IEEEdouble();
  পক্ষ S_IEEEquad:
    প্রত্যাগম IEEEquad();
  পক্ষ S_x87DoubleExtended:
    প্রত্যাগম x87DoubleExtended();
  পক্ষ S_PPCDoubleDouble:
    প্রত্যাগম PPCDoubleDouble();
  পক্ষ S_PPCDoubleDoubleLegacy:
    প্রত্যাগম PPCDoubleDoubleLegacy();
  পক্ষ S_Float8E5M2:
    প্রত্যাগম Float8E5M2();
  পক্ষ S_Float8E5M2FNUZ:
    প্রত্যাগম Float8E5M2FNUZ();
  পক্ষ S_Float8E4M3:
    প্রত্যাগম Float8E4M3();
  পক্ষ S_Float8E4M3FN:
    প্রত্যাগম Float8E4M3FN();
  পক্ষ S_Float8E4M3FNUZ:
    প্রত্যাগম Float8E4M3FNUZ();
  পক্ষ S_Float8E4M3B11FNUZ:
    প্রত্যাগম Float8E4M3B11FNUZ();
  পক্ষ S_Float8E3M4:
    প্রত্যাগম Float8E3M4();
  পক্ষ S_FloatTF32:
    প্রত্যাগম FloatTF32();
  পক্ষ S_Float8E8M0FNU:
    প্রত্যাগম Float8E8M0FNU();
  পক্ষ S_Float8E5M3FNU:
    প্রত্যাগম Float8E5M3FNU();
  পক্ষ S_Float6E3M2FN:
    প্রত্যাগম Float6E3M2FN();
  পক্ষ S_Float6E2M3FN:
    প্রত্যাগম Float6E2M3FN();
  পক্ষ S_Float4E2M1FN:
    প্রত্যাগম Float4E2M1FN();
  }
  llvm_unreachable("Unrecognised floating semantics");
}

APFloatBase::Semantics
APFloatBase::SemanticsToEnum(ধ্রুব llvm::fltSemantics &Sem) {
  যদি (&Sem == &llvm::APFloat::IEEEhalf())
    প্রত্যাগম S_IEEEhalf;
  অন্যথা যদি (&Sem == &llvm::APFloat::BFloat())
    প্রত্যাগম S_BFloat;
  অন্যথা যদি (&Sem == &llvm::APFloat::IEEEsingle())
    প্রত্যাগম S_IEEEsingle;
  অন্যথা যদি (&Sem == &llvm::APFloat::IEEEdouble())
    প্রত্যাগম S_IEEEdouble;
  অন্যথা যদি (&Sem == &llvm::APFloat::IEEEquad())
    প্রত্যাগম S_IEEEquad;
  অন্যথা যদি (&Sem == &llvm::APFloat::x87DoubleExtended())
    প্রত্যাগম S_x87DoubleExtended;
  অন্যথা যদি (&Sem == &llvm::APFloat::PPCDoubleDouble())
    প্রত্যাগম S_PPCDoubleDouble;
  অন্যথা যদি (&Sem == &llvm::APFloat::PPCDoubleDoubleLegacy())
    প্রত্যাগম S_PPCDoubleDoubleLegacy;
  অন্যথা যদি (&Sem == &llvm::APFloat::Float8E5M2())
    প্রত্যাগম S_Float8E5M2;
  অন্যথা যদি (&Sem == &llvm::APFloat::Float8E5M2FNUZ())
    প্রত্যাগম S_Float8E5M2FNUZ;
  অন্যথা যদি (&Sem == &llvm::APFloat::Float8E4M3())
    প্রত্যাগম S_Float8E4M3;
  অন্যথা যদি (&Sem == &llvm::APFloat::Float8E4M3FN())
    প্রত্যাগম S_Float8E4M3FN;
  অন্যথা যদি (&Sem == &llvm::APFloat::Float8E4M3FNUZ())
    প্রত্যাগম S_Float8E4M3FNUZ;
  অন্যথা যদি (&Sem == &llvm::APFloat::Float8E4M3B11FNUZ())
    প্রত্যাগম S_Float8E4M3B11FNUZ;
  অন্যথা যদি (&Sem == &llvm::APFloat::Float8E3M4())
    প্রত্যাগম S_Float8E3M4;
  অন্যথা যদি (&Sem == &llvm::APFloat::FloatTF32())
    প্রত্যাগম S_FloatTF32;
  অন্যথা যদি (&Sem == &llvm::APFloat::Float8E8M0FNU())
    প্রত্যাগম S_Float8E8M0FNU;
  অন্যথা যদি (&Sem == &llvm::APFloat::Float8E5M3FNU())
    প্রত্যাগম S_Float8E5M3FNU;
  অন্যথা যদি (&Sem == &llvm::APFloat::Float6E3M2FN())
    প্রত্যাগম S_Float6E3M2FN;
  অন্যথা যদি (&Sem == &llvm::APFloat::Float6E2M3FN())
    প্রত্যাগম S_Float6E2M3FN;
  অন্যথা যদি (&Sem == &llvm::APFloat::Float4E2M1FN())
    প্রত্যাগম S_Float4E2M1FN;
  অন্যথা
    llvm_unreachable("Unknown floating semantics");
}

তার্কিক APFloatBase::isRepresentableBy(ধ্রুব fltSemantics &A,
                                    ধ্রুব fltSemantics &B) {
  প্রত্যাগম A.maxExponent <= B.maxExponent && A.minExponent >= B.minExponent &&
         A.precision <= B.precision;
}

তার্কিক APFloatBase::isLosslesslyConvertibleTo(ধ্রুব fltSemantics &From,
                                            ধ্রুব fltSemantics &InitialTo,
                                            তার্কিক IgnoreNaNs) {
  যদি (&From == &InitialTo)
    প্রত্যাগম সত্য;

  // PPC double-double cannot be described by a conventional exponent range
  // and precision. In particular, converting it to another semantics drops
  // its low double, so conservatively reject conversions from it.
  যদি (&From == &semPPCDoubleDouble)
    প্রত্যাগম অসত্য;

  // Converting to PPC double-double is lossless exactly when the value fits
  // in its high double. In other words, the conversion is lossless if the
  // conversion to IEEEdouble is lossless.
  ধ্রুব fltSemantics &To =
      &InitialTo == &semPPCDoubleDouble ? semIEEEdouble : InitialTo;

  যদি (!isRepresentableBy(From, To))
    প্রত্যাগম অসত্য;

  যদি ((From.hasZero && !To.hasZero) ||
      (From.hasSignedRepr && !To.hasSignedRepr))
    প্রত্যাগম অসত্য;

  // NegativeZero NaN encoding repurposes the negative-zero bit pattern, so a
  // conversion to such a format cannot preserve a source negative zero.
  তার্কিক FromHasSignedZero = From.hasZero && From.hasSignedRepr &&
                           From.nanEncoding != fltNanEncoding::NegativeZero;
  তার্কিক ToHasSignedZero = To.hasZero && To.hasSignedRepr &&
                         To.nanEncoding != fltNanEncoding::NegativeZero;
  যদি (FromHasSignedZero && !ToHasSignedZero)
    প্রত্যাগম অসত্য;

  // isRepresentableBy compares normalized exponent ranges. Also ensure that
  // the smallest source value, which may be denormal, is represented exactly
  // by the destination semantics.
  APFloat SmallestFrom = APFloat::getSmallest(From);
  তার্কিক LosesInfo = অসত্য;
  (শূন্য)SmallestFrom.convert(To, APFloat::rmNearestTiesToEven, &LosesInfo);
  যদি (LosesInfo)
    প্রত্যাগম অসত্য;

  যদি (From.nonFiniteBehavior == fltNonfiniteBehavior::FiniteOnly)
    প্রত্যাগম সত্য;

  // Even when NaN representations can be ignored, NaNs must remain NaNs and
  // infinities must remain infinities. Otherwise the conversion can change
  // whether an operation with nnan has poison-producing operands.
  যদি (IgnoreNaNs) {
    যদি (From.nonFiniteBehavior == fltNonfiniteBehavior::IEEE754)
      প্রত্যাগম To.nonFiniteBehavior == fltNonfiniteBehavior::IEEE754;
    প্রত্যাগম To.nonFiniteBehavior != fltNonfiniteBehavior::FiniteOnly;
  }

  যদি (From.nonFiniteBehavior == fltNonfiniteBehavior::IEEE754) {
    // Converting an IEEE signaling NaN to another semantics quiets it, so the
    // original value cannot be recovered by converting it back.
    প্রত্যাগম অসত্য;
  }

  // NanOnly formats have no signaling NaNs. IEEE semantics can represent
  // their quiet NaNs; conversions between NanOnly formats are conservatively
  // accepted only when they use the same NaN encoding.
  যদি (To.nonFiniteBehavior == fltNonfiniteBehavior::IEEE754)
    প্রত্যাগম সত্য;
  প্রত্যাগম To.nonFiniteBehavior == fltNonfiniteBehavior::NanOnly &&
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
স্থির constexpr unsigned অঙ্ক_Pow5_Parts_On_স্তূপ = 16;

স্থির unsigned পূর্ণাঙ্ক powerOfFivePartsForExponent(unsigned পূর্ণাঙ্ক power) {
  প্রতিজ্ঞা(power <= maxExponent);

  // 815 / 351 is an upper bound on log2(5).  Reserve an additional part for
  // tcFullMultiply, which writes the full product width even when its most
  // significant part is zero.
  প্রত্যাগম 2 + ((power * 815) / (351 * APFloatBase::integerPartWidth));
}

unsigned পূর্ণাঙ্ক APFloatBase::semanticsPrecision(ধ্রুব fltSemantics &semantics) {
  প্রত্যাগম semantics.precision;
}
APFloatBase::ExponentType
APFloatBase::semanticsMaxExponent(ধ্রুব fltSemantics &semantics) {
  প্রত্যাগম semantics.maxExponent;
}
APFloatBase::ExponentType
APFloatBase::semanticsMinExponent(ধ্রুব fltSemantics &semantics) {
  প্রত্যাগম semantics.minExponent;
}
unsigned পূর্ণাঙ্ক APFloatBase::semanticsSizeInBits(ধ্রুব fltSemantics &semantics) {
  প্রত্যাগম semantics.sizeInBits;
}
unsigned পূর্ণাঙ্ক APFloatBase::semanticsIntSizeInBits(ধ্রুব fltSemantics &semantics,
                                                 তার্কিক isSigned) {
  // The max FP value is pow(2, MaxExponent) * (1 + MaxFraction), so we need
  // at least one more bit than the MaxExponent to hold the max FP value.
  unsigned পূর্ণাঙ্ক MinBitWidth = semanticsMaxExponent(semantics) + 1;
  // Extra sign bit needed.
  যদি (isSigned)
    ++MinBitWidth;
  প্রত্যাগম MinBitWidth;
}

তার্কিক APFloatBase::semanticsHasZero(ধ্রুব fltSemantics &semantics) {
  প্রত্যাগম semantics.hasZero;
}

তার্কিক APFloatBase::semanticsHasSignedRepr(ধ্রুব fltSemantics &semantics) {
  প্রত্যাগম semantics.hasSignedRepr;
}

তার্কিক APFloatBase::semanticsHasInf(ধ্রুব fltSemantics &semantics) {
  প্রত্যাগম semantics.nonFiniteBehavior == fltNonfiniteBehavior::IEEE754;
}

তার্কিক APFloatBase::semanticsHasNaN(ধ্রুব fltSemantics &semantics) {
  প্রত্যাগম semantics.nonFiniteBehavior != fltNonfiniteBehavior::FiniteOnly;
}

তার্কিক APFloatBase::isIEEELikeFP(ধ্রুব fltSemantics &semantics) {
  // Keep in sync with Type::isIEEELikeFPTy
  প্রত্যাগম SemanticsToEnum(semantics) <= S_IEEEquad;
}

তার্কিক APFloatBase::hasSignBitInMSB(ধ্রুব fltSemantics &semantics) {
  প্রত্যাগম semantics.hasSignBitInMSB;
}

তার্কিক APFloatBase::isRepresentableAsNormalIn(ধ্রুব fltSemantics &Src,
                                            ধ্রুব fltSemantics &Dst) {
  // Exponent range must be larger.
  যদি (Src.maxExponent >= Dst.maxExponent || Src.minExponent <= Dst.minExponent)
    প্রত্যাগম অসত্য;

  // If the mantissa is long enough, the result value could still be denormal
  // with a larger exponent range.
  //
  // FIXME: This condition is probably not accurate but also shouldn't be a
  // practical concern with existing types.
  প্রত্যাগম Dst.precision >= Src.precision;
}

unsigned APFloatBase::getSizeInBits(ধ্রুব fltSemantics &Sem) {
  প্রত্যাগম Sem.sizeInBits;
}

স্থির constexpr APFloatBase::ExponentType
exponentZero(ধ্রুব fltSemantics &semantics) {
  প্রত্যাগম semantics.minExponent - 1;
}

স্থির constexpr APFloatBase::ExponentType
exponentInf(ধ্রুব fltSemantics &semantics) {
  প্রত্যাগম semantics.maxExponent + 1;
}

স্থির constexpr APFloatBase::ExponentType
exponentNaN(ধ্রুব fltSemantics &semantics) {
  যদি (semantics.nonFiniteBehavior == fltNonfiniteBehavior::NanOnly) {
    যদি (semantics.nanEncoding == fltNanEncoding::NegativeZero)
      প্রত্যাগম exponentZero(semantics);
    যদি (semantics.hasSignedRepr || semantics.precision > 1)
      প্রত্যাগম semantics.maxExponent;
  }
  প্রত্যাগম semantics.maxExponent + 1;
}

/* A bunch of private, handy routines.  */

স্থির অন্তঃস্থ Error createError(ধ্রুব Twine &Err) {
  প্রত্যাগম make_error<StringError>(Err, inconvertibleErrorCode());
}

স্থির constexpr অন্তঃস্থ unsigned পূর্ণাঙ্ক part_গুনতি_For_Bits(unsigned পূর্ণাঙ্ক bits) {
  প্রত্যাগম std::max(1u, (bits + APFloatBase::integerPartWidth - 1) /
                          APFloatBase::integerPartWidth);
}

/* Returns 0U-9U.  Return values >= 10U are not digits.  */
স্থির অন্তঃস্থ unsigned পূর্ণাঙ্ক
dec_Digit_মান(unsigned পূর্ণাঙ্ক c)
{
  প্রত্যাগম c - '0';
}

/* Return the value of a decimal exponent of the form
   [+-]ddddddd.

   If the exponent overflows, returns a large exponent with the
   appropriate sign.  */
স্থির Expected<পূর্ণাঙ্ক> readExponent(StringRef::iterator begin,
                                  StringRef::iterator end) {
  ধ্রুব unsigned পূর্ণাঙ্ক overlargeExponent = 24000;  /* FIXME.  */
  StringRef::iterator p = begin;

  // Treat no exponent as 0 to match binutils
  যদি (p == end || ((*p == '-' || *p == '+') && (p + 1) == end))
    প্রত্যাগম 0;

  তার্কিক isNegative = *p == '-';
  যদি (*p == '-' || *p == '+') {
    p++;
    যদি (p == end)
      প্রত্যাগম createError("Exponent has no digits");
  }

  unsigned absExponent = dec_Digit_মান(*p++);
  যদি (absExponent >= 10U)
    প্রত্যাগম createError("Invalid character in exponent");

  প্রতি (; p != end; ++p) {
    unsigned মান_2 = dec_Digit_মান(*p);
    যদি (মান_2 >= 10U)
      প্রত্যাগম createError("Invalid character in exponent");

    absExponent = absExponent * 10U + মান_2;
    যদি (absExponent >= overlargeExponent) {
      absExponent = overlargeExponent;
      ভঙ্গ;
    }
  }

  যদি (isNegative)
    প্রত্যাগম -(পূর্ণাঙ্ক) absExponent;
  অন্যথা
    প্রত্যাগম (পূর্ণাঙ্ক) absExponent;
}

/* This is ugly and needs cleaning up, but I don't immediately see
   how whilst remaining safe.  */
স্থির Expected<পূর্ণাঙ্ক> totalExponent(StringRef::iterator p,
                                   StringRef::iterator end,
                                   পূর্ণাঙ্ক exponentAdjustment) {
  পূর্ণাঙ্ক exponent = 0;

  যদি (p == end)
    প্রত্যাগম createError("Exponent has no digits");

  তার্কিক negative = *p == '-';
  যদি (*p == '-' || *p == '+') {
    p++;
    যদি (p == end)
      প্রত্যাগম createError("Exponent has no digits");
  }

  পূর্ণাঙ্ক unsignedExponent = 0;
  তার্কিক overflow = অসত্য;
  প্রতি (; p != end; ++p) {
    unsigned পূর্ণাঙ্ক মান_2;

    মান_2 = dec_Digit_মান(*p);
    যদি (মান_2 >= 10U)
      প্রত্যাগম createError("Invalid character in exponent");

    unsignedExponent = unsignedExponent * 10 + মান_2;
    যদি (unsignedExponent > 32767) {
      overflow = সত্য;
      ভঙ্গ;
    }
  }

  যদি (exponentAdjustment > 32767 || exponentAdjustment < -32768)
    overflow = সত্য;

  যদি (!overflow) {
    exponent = unsignedExponent;
    যদি (negative)
      exponent = -exponent;
    exponent += exponentAdjustment;
    যদি (exponent > 32767 || exponent < -32768)
      overflow = সত্য;
  }

  যদি (overflow)
    exponent = negative ? -32768: 32767;

  প্রত্যাগম exponent;
}

স্থির Expected<StringRef::iterator>
skipLeadingZeroesAndAnyDot(StringRef::iterator begin, StringRef::iterator end,
                           StringRef::iterator *dot) {
  StringRef::iterator p = begin;
  *dot = end;
  যাবত্ (p != end && *p == '0')
    p++;

  যদি (p != end && *p == '.') {
    *dot = p++;

    যদি (end - begin == 1)
      প্রত্যাগম createError("Significand has no digits");

    যাবত্ (p != end && *p == '0')
      p++;
  }

  প্রত্যাগম p;
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
সংরচনা decimalInfo {
  ধ্রুব বর্ণ *firstSigDigit;
  ধ্রুব বর্ণ *lastSigDigit;
  পূর্ণাঙ্ক exponent;
  পূর্ণাঙ্ক normalizedExponent;
};

স্থির Error interpretDecimal(StringRef::iterator begin,
                              StringRef::iterator end, decimalInfo *D) {
  StringRef::iterator dot = end;

  auto PtrOrErr = skipLeadingZeroesAndAnyDot(begin, end, &dot);
  যদি (!PtrOrErr)
    প্রত্যাগম PtrOrErr.takeError();
  StringRef::iterator p = *PtrOrErr;

  D->firstSigDigit = p;
  D->exponent = 0;
  D->normalizedExponent = 0;

  প্রতি (; p != end; ++p) {
    যদি (*p == '.') {
      যদি (dot != end)
        প্রত্যাগম createError("String contains multiple dots");
      dot = p++;
      যদি (p == end)
        ভঙ্গ;
    }
    যদি (dec_Digit_মান(*p) >= 10U)
      ভঙ্গ;
  }

  যদি (p != end) {
    যদি (*p != 'e' && *p != 'E')
      প্রত্যাগম createError("Invalid character in significand");
    যদি (p == begin)
      প্রত্যাগম createError("Significand has no digits");
    যদি (dot != end && p - begin == 1)
      প্রত্যাগম createError("Significand has no digits");

    /* p points to the first non-digit in the string */
    auto ExpOrErr = readExponent(p + 1, end);
    যদি (!ExpOrErr)
      প্রত্যাগম ExpOrErr.takeError();
    D->exponent = *ExpOrErr;

    /* Implied decimal point?  */
    যদি (dot == end)
      dot = p;
  }

  /* If number is all zeroes accept any exponent.  */
  যদি (p != D->firstSigDigit) {
    /* Drop insignificant trailing zeroes.  */
    যদি (p != begin) {
      কুরু
        কুরু
          p--;
        যাবত্ (p != begin && *p == '0');
      যাবত্ (p != begin && *p == '.');
    }

    /* Adjust the exponents for any decimal point.  */
    D->exponent += static_cast<APFloat::ExponentType>((dot - p) - (dot > p));
    D->normalizedExponent = (D->exponent +
              static_cast<APFloat::ExponentType>((p - D->firstSigDigit)
                                      - (dot > D->firstSigDigit && dot < p)));
  }

  D->lastSigDigit = p;
  প্রত্যাগম Error::success();
}

/* Return the trailing fraction of a hexadecimal number.
   DIGITVALUE is the first hex digit of the fraction, P points to
   the next digit.  */
স্থির Expected<lostFraction>
trailingHexadecimalFraction(StringRef::iterator p, StringRef::iterator end,
                            unsigned পূর্ণাঙ্ক digit_মান) {
  /* If the first trailing digit isn't 0 or 8 we can work out the
     fraction immediately.  */
  যদি (digit_মান > 8)
    প্রত্যাগম lfMoreThanHalf;
  অন্যথা যদি (digit_মান < 8 && digit_মান > 0)
    প্রত্যাগম lfLessThanHalf;

  // Otherwise we need to find the first non-zero digit.
  যাবত্ (p != end && (*p == '0' || *p == '.'))
    p++;

  যদি (p == end)
    প্রত্যাগম createError("Invalid trailing hexadecimal fraction!");

  unsigned hexDigit = hex_Digit_মান(*p);

  /* If we ran off the end it is exactly zero or one-half, otherwise
     a little more.  */
  যদি (hexDigit == UINT_MAX)
    প্রত্যাগম digit_মান == 0 ? lfExactlyZero: lfExactlyHalf;
  অন্যথা
    প্রত্যাগম digit_মান == 0 ? lfLessThanHalf: lfMoreThanHalf;
}

/* Return the fraction lost were a bignum truncated losing the least
   significant BITS bits.  */
স্থির lostFraction
lostFractionThroughTruncation(ধ্রুব APFloatBase::integerPart *parts,
                              unsigned পূর্ণাঙ্ক part_গুনতি,
                              unsigned পূর্ণাঙ্ক bits)
{
  unsigned lsb = APInt::tcLSB(parts, part_গুনতি);

  /* Note this is guaranteed true if bits == 0, or LSB == UINT_MAX.  */
  যদি (bits <= lsb)
    প্রত্যাগম lfExactlyZero;
  যদি (bits == lsb + 1)
    প্রত্যাগম lfExactlyHalf;
  যদি (bits <= part_গুনতি * APFloatBase::integerPartWidth &&
      APInt::tcExtractBit(parts, bits - 1))
    প্রত্যাগম lfMoreThanHalf;

  প্রত্যাগম lfLessThanHalf;
}

/* Shift DST right BITS bits noting lost fraction.  */
স্থির lostFraction
shift_ডান(APFloatBase::integerPart *dst, unsigned পূর্ণাঙ্ক parts, unsigned পূর্ণাঙ্ক bits)
{
  lostFraction lost_fraction = lostFractionThroughTruncation(dst, parts, bits);

  APInt::tc_Shift_ডান(dst, parts, bits);

  প্রত্যাগম lost_fraction;
}

/* Combine the effect of two lost fractions.  */
স্থির lostFraction
combineLostFractions(lostFraction moreSignificant,
                     lostFraction lessSignificant)
{
  যদি (lessSignificant != lfExactlyZero) {
    যদি (moreSignificant == lfExactlyZero)
      moreSignificant = lfLessThanHalf;
    অন্যথা যদি (moreSignificant == lfExactlyHalf)
      moreSignificant = lfMoreThanHalf;
  }

  প্রত্যাগম moreSignificant;
}

/* The error from the true value, in half-ulps, on multiplying two
   floating point numbers, which differ from the value they
   approximate by at most HUE1 and HUE2 half-ulps, is strictly less
   than the returned value.

   See "How to Read Floating Point Numbers Accurately" by William D
   Clinger.  */
স্থির unsigned পূর্ণাঙ্ক
HUerrBound(তার্কিক inexactMultiply, unsigned পূর্ণাঙ্ক HUerr1, unsigned পূর্ণাঙ্ক HUerr2)
{
  প্রতিজ্ঞা(HUerr1 < 2 || HUerr2 < 2 || (HUerr1 + HUerr2 < 8));

  যদি (HUerr1 + HUerr2 == 0)
    প্রত্যাগম inexactMultiply * 2;  /* <= inexactMultiply half-ulps.  */
  অন্যথা
    প্রত্যাগম inexactMultiply + 2 * (HUerr1 + HUerr2);
}

/* The number of ulps from the boundary (zero, or half if ISNEAREST)
   when the least significant BITS are truncated.  BITS cannot be
   zero.  */
স্থির APFloatBase::integerPart
ulpsFromBoundary(ধ্রুব APFloatBase::integerPart *parts, unsigned পূর্ণাঙ্ক bits,
                 তার্কিক isNearest) {
  প্রতিজ্ঞা(bits != 0);

  bits--;
  unsigned গুনতি = bits / APFloatBase::integerPartWidth;
  unsigned partBits = bits % APFloatBase::integerPartWidth + 1;

  APFloatBase::integerPart part =
      parts[গুনতি] & (~(APFloatBase::integerPart)0 >>
                      (APFloatBase::integerPartWidth - partBits));

  APFloatBase::integerPart boundary;
  যদি (isNearest)
    boundary = (APFloatBase::integerPart) 1 << (partBits - 1);
  অন্যথা
    boundary = 0;

  যদি (গুনতি == 0) {
    যদি (part - boundary <= boundary - part)
      প্রত্যাগম part - boundary;
    অন্যথা
      প্রত্যাগম boundary - part;
  }

  যদি (part == boundary) {
    যাবত্ (--গুনতি)
      যদি (parts[গুনতি])
        প্রত্যাগম ~(APFloatBase::integerPart) 0; /* A lot.  */

    প্রত্যাগম parts[0];
  } অন্যথা যদি (part == boundary - 1) {
    যাবত্ (--গুনতি)
      যদি (~parts[গুনতি])
        প্রত্যাগম ~(APFloatBase::integerPart) 0; /* A lot.  */

    প্রত্যাগম -parts[0];
  }

  প্রত্যাগম ~(APFloatBase::integerPart) 0; /* A lot.  */
}

স্থির constexpr unsigned PowerOfFivePartCounts[] = {1,  1,  2,  3,   5,  10,
                                                     19, 38, 75, 149, 298};

স্থির constexpr APFloatBase::integerPart PowerOfFiveParts[] = {
#অন্তর্ভাব "APFloatPowerOfFiveTable.inc"
};

static_assert([] {
  unsigned Sum = 0;
  প্রতি (unsigned গুনতি_2 : PowerOfFivePartCounts)
    Sum += গুনতি_2;
  প্রত্যাগম Sum;
}() == std::size(PowerOfFiveParts));

/* Place pow(5, power) in DST, and return the number of parts used.
   DST must be at least one part larger than size of the answer.  */
স্থির unsigned পূর্ণাঙ্ক
powerOf5(APFloatBase::integerPart *dst, unsigned পূর্ণাঙ্ক power) {
  স্থির ধ্রুব APFloatBase::integerPart firstEightPowers[] = {
      1, 5, 25, 125, 625, 3125, 15625, 78125};

  SmallVector<APFloatBase::integerPart, অঙ্ক_Pow5_Parts_On_স্তূপ> scratch;
  scratch.resize_for_overwrite(powerOfFivePartsForExponent(power));

  APFloatBase::integerPart *p1, *p2;
  ধ্রুব APFloatBase::integerPart *pow5;

  p1 = dst;
  p2 = scratch.data();

  *p1 = firstEightPowers[power & 7];
  power >>= 3;

  unsigned result = 1;
  pow5 = PowerOfFiveParts;

  প্রতি (unsigned পূর্ণাঙ্ক n = 0; power; power >>= 1, n++) {
    unsigned parts_গুনতি = PowerOfFivePartCounts[n];

    যদি (power & 1) {
      APFloatBase::integerPart *tmp;

      APInt::tcFullMultiply(p2, p1, pow5, result, parts_গুনতি);
      result += parts_গুনতি;
      যদি (p2[result - 1] == 0)
        result--;

      /* Now result is in p1 with partsCount parts and p2 is scratch
         space.  */
      tmp = p1;
      p1 = p2;
      p2 = tmp;
    }

    pow5 += parts_গুনতি;
  }

  যদি (p1 != dst)
    APInt::tcAssign(dst, p1, result);

  প্রত্যাগম result;
}

/* Zero at the end to avoid modular arithmetic when adding one; used
   when rounding up during hexadecimal output.  */
স্থির ধ্রুব বর্ণ hexDigitsLower[] = "0123456789abcdef0";
স্থির ধ্রুব বর্ণ hexDigitsUpper[] = "0123456789ABCDEF0";
স্থির ধ্রুব বর্ণ infinityL[] = "infinity";
স্থির ধ্রুব বর্ণ infinityU[] = "INFINITY";
স্থির ধ্রুব বর্ণ NaNL[] = "nan";
স্থির ধ্রুব বর্ণ NaNU[] = "NAN";

/* Write out an integerPart in hexadecimal, starting with the most
   significant nibble.  Write out exactly COUNT hexdigits, return
   COUNT.  */
স্থির unsigned পূর্ণাঙ্ক
partAsHex (বর্ণ *dst, APFloatBase::integerPart part, unsigned পূর্ণাঙ্ক গুনতি,
           ধ্রুব বর্ণ *hexDigitChars)
{
  unsigned পূর্ণাঙ্ক result = গুনতি;

  প্রতিজ্ঞা(গুনতি != 0 && গুনতি <= APFloatBase::integerPartWidth / 4);

  part >>= (APFloatBase::integerPartWidth - 4 * গুনতি);
  যাবত্ (গুনতি--) {
    dst[গুনতি] = hexDigitChars[part & 0xf];
    part >>= 4;
  }

  প্রত্যাগম result;
}

/* Write out an unsigned decimal integer.  */
স্থির বর্ণ *writeUnsignedDecimal(বর্ণ *dst, unsigned পূর্ণাঙ্ক n) {
  বর্ণ buff[40], *p;

  p = buff;
  কুরু
    *p++ = '0' + n % 10;
  যাবত্ (n /= 10);

  কুরু
    *dst++ = *--p;
  যাবত্ (p != buff);

  প্রত্যাগম dst;
}

/* Write out a signed decimal integer.  */
স্থির বর্ণ *writeSignedDecimal(বর্ণ *dst, পূর্ণাঙ্ক মান_2) {
  যদি (মান_2 < 0) {
    *dst++ = '-';
    dst = writeUnsignedDecimal(dst, -(unsigned) মান_2);
  } অন্যথা {
    dst = writeUnsignedDecimal(dst, মান_2);
  }

  প্রত্যাগম dst;
}

// Compute the ULP of the input using a definition from:
// Jean-Michel Muller. On the definition of ulp(x). [Research Report] RR-5504,
// LIP RR-2005-09, INRIA, LIP. 2005, pp.16. inria-00070503
স্থির APFloat harrisonUlp(ধ্রুব APFloat &X) {
  ধ্রুব fltSemantics &Sem = X.getSemantics();
  বিকল্প (X.getCategory()) {
  পক্ষ APFloat::fcNaN:
    প্রত্যাগম APFloat::getQNaN(Sem);
  পক্ষ APFloat::fcInfinity:
    প্রত্যাগম APFloat::getInf(Sem);
  পক্ষ APFloat::fcZero:
    প্রত্যাগম APFloat::getSmallest(Sem);
  পক্ষ APFloat::fcNormal:
    ভঙ্গ;
  }
  যদি (X.isDenormal() || X.isSmallestNormalized())
    প্রত্যাগম APFloat::getSmallest(Sem);
  পূর্ণাঙ্ক Exp = ilogb(X);
  যদি (X.getExactLog2() != INT_MIN)
    Exp -= 1;
  প্রত্যাগম scalbn(APFloat::getOne(Sem), Exp - (Sem.precision - 1),
                APFloat::rmNearestTiesToEven);
}

নামাকাশ detail {
/* Constructors.  */
শূন্য IEEEFloat::initialize(ধ্রুব fltSemantics *ourSemantics) {
  semantics = ourSemantics;
  unsigned গুনতি = part_গুনতি();
  যদি (গুনতি > 1)
    significand.parts = নূতন integerPart[গুনতি];
}

শূন্য IEEEFloat::freeSignificand() {
  যদি (needsCleanup())
    delete [] significand.parts;
}

শূন্য IEEEFloat::assign(ধ্রুব IEEEFloat &rhs) {
  প্রতিজ্ঞা(semantics == rhs.semantics);

  sign = rhs.sign;
  category = rhs.category;
  exponent = rhs.exponent;
  যদি (isFiniteNonZero() || category == fcNaN)
    copySignificand(rhs);
}

শূন্য IEEEFloat::copySignificand(ধ্রুব IEEEFloat &rhs) {
  প্রতিজ্ঞা(isFiniteNonZero() || category == fcNaN);
  প্রতিজ্ঞা(rhs.part_গুনতি() >= part_গুনতি());

  APInt::tcAssign(significandParts(), rhs.significandParts(),
                  part_গুনতি());
}

/* Make this number a NaN, with an arbitrary but deterministic value
   for the significand.  If double or longer, this is a signalling NaN,
   which may not be ideal.  If float, this is QNaN(0).  */
শূন্য IEEEFloat::makeNaN(তার্কিক SNaN, তার্কিক Negative, ধ্রুব APInt *fill) {
  যদি (semantics->nonFiniteBehavior == fltNonfiniteBehavior::FiniteOnly)
    llvm_unreachable("This floating point format does not support NaN");

  যদি (Negative && !semantics->hasSignedRepr)
    llvm_unreachable(
        "This floating point format does not support signed values");

  category = fcNaN;
  sign = Negative;
  exponent = exponentNaN();

  integerPart *significand = significandParts();
  unsigned অঙ্ক_Parts = part_গুনতি();

  APInt fill_storage;
  যদি (semantics->nonFiniteBehavior == fltNonfiniteBehavior::NanOnly) {
    // Finite-only types do not distinguish signalling and quiet NaN, so
    // make them all signalling.
    SNaN = অসত্য;
    যদি (semantics->nanEncoding == fltNanEncoding::NegativeZero) {
      sign = সত্য;
      fill_storage = APInt::getZero(semantics->precision - 1);
    } অন্যথা {
      fill_storage = APInt::getAllOnes(semantics->precision - 1);
    }
    fill = &fill_storage;
  }

  // Set the significand bits to the fill.
  যদি (!fill || fill->get_অঙ্ক_শব্দগুলি() < অঙ্ক_Parts)
    APInt::tcSet(significand, 0, অঙ্ক_Parts);
  যদি (fill) {
    APInt::tcAssign(significand, fill->getRawData(),
                    std::min(fill->get_অঙ্ক_শব্দগুলি(), অঙ্ক_Parts));

    // Zero out the excess bits of the significand.
    unsigned bitsToPreserve = semantics->precision - 1;
    unsigned part = bitsToPreserve / 64;
    bitsToPreserve %= 64;
    significand[part] &= ((1ULL << bitsToPreserve) - 1);
    প্রতি (part++; part != অঙ্ক_Parts; ++part)
      significand[part] = 0;
  }

  unsigned QNaNBit =
      (semantics->precision >= 2) ? (semantics->precision - 2) : 0;

  যদি (SNaN) {
    // We always have to clear the QNaN bit to make it an SNaN.
    APInt::tcClearBit(significand, QNaNBit);

    // If there are no bits set in the payload, we have to set
    // *something* to make it a NaN instead of an infinity;
    // conventionally, this is the next bit down from the QNaN bit.
    যদি (APInt::tcIsZero(significand, অঙ্ক_Parts))
      APInt::tcSetBit(significand, QNaNBit - 1);
  } অন্যথা যদি (semantics->nanEncoding == fltNanEncoding::NegativeZero) {
    // The only NaN is a quiet NaN, and it has no bits sets in the significand.
    // Do nothing.
  } অন্যথা {
    // We always have to set the QNaN bit to make it a QNaN.
    APInt::tcSetBit(significand, QNaNBit);
  }

  // For x87 extended precision, we want to make a NaN, not a
  // pseudo-NaN.  Maybe we should expose the ability to make
  // pseudo-NaNs?
  যদি (semantics == &APFloatBase::semX87DoubleExtended)
    APInt::tcSetBit(significand, QNaNBit + 1);
}

IEEEFloat &IEEEFloat::operator=(ধ্রুব IEEEFloat &rhs) {
  যদি (এতত্ != &rhs) {
    যদি (semantics != rhs.semantics) {
      freeSignificand();
      initialize(rhs.semantics);
    }
    assign(rhs);
  }

  প্রত্যাগম *এতত্;
}

IEEEFloat &IEEEFloat::operator=(IEEEFloat &&rhs) {
  freeSignificand();

  semantics = rhs.semantics;
  significand = rhs.significand;
  exponent = rhs.exponent;
  category = rhs.category;
  sign = rhs.sign;

  rhs.semantics = &APFloatBase::semBogus;
  প্রত্যাগম *এতত্;
}

তার্কিক IEEEFloat::isDenormal() ধ্রুব {
  প্রত্যাগম getSemantics().hasDenormals && isFiniteNonZero() &&
         (exponent == semantics->minExponent) &&
         (APInt::tcExtractBit(significandParts(), semantics->precision - 1) ==
          0);
}

তার্কিক IEEEFloat::isSmallest() ধ্রুব {
  // The smallest number by magnitude in our format will be the smallest
  // denormal, i.e. the floating point number with exponent being minimum
  // exponent and significand bitwise equal to 1 (i.e. with MSB equal to 0).
  প্রত্যাগম isFiniteNonZero() && exponent == semantics->minExponent &&
    significandMSB() == 0;
}

তার্কিক IEEEFloat::isSmallestNormalized() ধ্রুব {
  প্রত্যাগম getCategory() == fcNormal && exponent == semantics->minExponent &&
         isSignificandAllZerosExceptMSB();
}

unsigned পূর্ণাঙ্ক IEEEFloat::get_অঙ্ক_উপরের_Bits() ধ্রুব {
  ধ্রুব unsigned পূর্ণাঙ্ক Part_গুনতি = part_গুনতি_For_Bits(semantics->precision);
  ধ্রুব unsigned পূর্ণাঙ্ক Bits = Part_গুনতি * integerPartWidth;

  // Compute how many bits are used in the final word.
  // When precision is just 1, it represents the 'Pth'
  // Precision bit and not the actual significand bit.
  ধ্রুব unsigned পূর্ণাঙ্ক অঙ্ক_উপরের_Bits = (semantics->precision > 1)
                                       ? (Bits - semantics->precision + 1)
                                       : (Bits - semantics->precision);
  প্রত্যাগম অঙ্ক_উপরের_Bits;
}

তার্কিক IEEEFloat::isSignificandAllOnes() ধ্রুব {
  // Test if the significand excluding the integral bit is all ones. This allows
  // us to test for binade boundaries.
  ধ্রুব integerPart *Parts = significandParts();
  ধ্রুব unsigned Part_গুনতি = part_গুনতি_For_Bits(semantics->precision);
  প্রতি (unsigned i = 0; i < Part_গুনতি - 1; i++)
    যদি (~Parts[i])
      প্রত্যাগম অসত্য;

  // Set the unused high bits to all ones when we compare.
  ধ্রুব unsigned অঙ্ক_উপরের_Bits = get_অঙ্ক_উপরের_Bits();
  প্রতিজ্ঞা(অঙ্ক_উপরের_Bits <= integerPartWidth && অঙ্ক_উপরের_Bits > 0 &&
         "Can not have more high bits to fill than integerPartWidth");
  ধ্রুব integerPart উপরের_Bit_Fill =
    ~integerPart(0) << (integerPartWidth - অঙ্ক_উপরের_Bits);
  যদি ((semantics->precision <= 1) || (~(Parts[Part_গুনতি - 1] | উপরের_Bit_Fill)))
    প্রত্যাগম অসত্য;

  প্রত্যাগম সত্য;
}

তার্কিক IEEEFloat::isSignificandAllOnesExceptLSB() ধ্রুব {
  // Test if the significand excluding the integral bit is all ones except for
  // the least significant bit.
  ধ্রুব integerPart *Parts = significandParts();

  যদি (Parts[0] & 1)
    প্রত্যাগম অসত্য;

  ধ্রুব unsigned Part_গুনতি = part_গুনতি_For_Bits(semantics->precision);
  প্রতি (unsigned i = 0; i < Part_গুনতি - 1; i++) {
    যদি (~Parts[i] & ~unsigned{!i})
      প্রত্যাগম অসত্য;
  }

  // Set the unused high bits to all ones when we compare.
  ধ্রুব unsigned অঙ্ক_উপরের_Bits = get_অঙ্ক_উপরের_Bits();
  প্রতিজ্ঞা(অঙ্ক_উপরের_Bits <= integerPartWidth && অঙ্ক_উপরের_Bits > 0 &&
         "Can not have more high bits to fill than integerPartWidth");
  ধ্রুব integerPart উপরের_Bit_Fill = ~integerPart(0)
                                  << (integerPartWidth - অঙ্ক_উপরের_Bits);
  যদি (~(Parts[Part_গুনতি - 1] | উপরের_Bit_Fill | 0x1))
    প্রত্যাগম অসত্য;

  প্রত্যাগম সত্য;
}

তার্কিক IEEEFloat::isSignificandAllZeros() ধ্রুব {
  // Test if the significand excluding the integral bit is all zeros. This
  // allows us to test for binade boundaries.
  ধ্রুব integerPart *Parts = significandParts();
  ধ্রুব unsigned Part_গুনতি = part_গুনতি_For_Bits(semantics->precision);

  প্রতি (unsigned i = 0; i < Part_গুনতি - 1; i++)
    যদি (Parts[i])
      প্রত্যাগম অসত্য;

  // Compute how many bits are used in the final word.
  ধ্রুব unsigned অঙ্ক_উপরের_Bits = get_অঙ্ক_উপরের_Bits();
  প্রতিজ্ঞা(অঙ্ক_উপরের_Bits < integerPartWidth && "Can not have more high bits to "
         "clear than integerPartWidth");
  ধ্রুব integerPart উপরের_Bit_Mask = ~integerPart(0) >> অঙ্ক_উপরের_Bits;

  যদি ((semantics->precision > 1) && (Parts[Part_গুনতি - 1] & উপরের_Bit_Mask))
    প্রত্যাগম অসত্য;

  প্রত্যাগম সত্য;
}

তার্কিক IEEEFloat::isSignificandAllZerosExceptMSB() ধ্রুব {
  ধ্রুব integerPart *Parts = significandParts();
  ধ্রুব unsigned Part_গুনতি = part_গুনতি_For_Bits(semantics->precision);

  প্রতি (unsigned i = 0; i < Part_গুনতি - 1; i++) {
    যদি (Parts[i])
      প্রত্যাগম অসত্য;
  }

  ধ্রুব unsigned অঙ্ক_উপরের_Bits = get_অঙ্ক_উপরের_Bits();
  ধ্রুব integerPart MSBMask = integerPart(1)
                              << (integerPartWidth - অঙ্ক_উপরের_Bits);
  প্রত্যাগম ((semantics->precision <= 1) || (Parts[Part_গুনতি - 1] == MSBMask));
}

তার্কিক IEEEFloat::isLargest() ধ্রুব {
  তার্কিক IsMaxExp = isFiniteNonZero() && exponent == semantics->maxExponent;
  যদি (semantics->nonFiniteBehavior == fltNonfiniteBehavior::NanOnly &&
      semantics->nanEncoding == fltNanEncoding::AllOnes) {
    // The largest number by magnitude in our format will be the floating point
    // number with maximum exponent and with significand that is all ones except
    // the LSB.
    প্রত্যাগম (IsMaxExp && APFloat::hasSignificand(*semantics))
               ? isSignificandAllOnesExceptLSB()
               : IsMaxExp;
  } অন্যথা {
    // The largest number by magnitude in our format will be the floating point
    // number with maximum exponent and with significand that is all ones.
    প্রত্যাগম IsMaxExp && isSignificandAllOnes();
  }
}

তার্কিক IEEEFloat::isInteger() ধ্রুব {
  // This could be made more efficient; I'm going for obviously correct.
  যদি (!isFinite()) প্রত্যাগম অসত্য;
  IEEEFloat truncated = *এতত্;
  truncated.roundToIntegral(rmTowardZero);
  প্রত্যাগম compare(truncated) == cmpEqual;
}

তার্কিক IEEEFloat::bitwiseIsEqual(ধ্রুব IEEEFloat &rhs) ধ্রুব {
  যদি (এতত্ == &rhs)
    প্রত্যাগম সত্য;
  যদি (semantics != rhs.semantics ||
      category != rhs.category ||
      sign != rhs.sign)
    প্রত্যাগম অসত্য;
  যদি (category==fcZero || category==fcInfinity)
    প্রত্যাগম সত্য;

  যদি (isFiniteNonZero() && exponent != rhs.exponent)
    প্রত্যাগম অসত্য;

  প্রত্যাগম std::equal(significandParts(), significandParts() + part_গুনতি(),
                    rhs.significandParts());
}

IEEEFloat::IEEEFloat(ধ্রুব fltSemantics &ourSemantics, integerPart মান_2) {
  initialize(&ourSemantics);
  sign = 0;
  category = fcNormal;
  zeroSignificand();
  exponent = ourSemantics.precision - 1;
  significandParts()[0] = মান_2;
  normalize(rmNearestTiesToEven, lfExactlyZero);
}

IEEEFloat::IEEEFloat(ধ্রুব fltSemantics &ourSemantics) {
  initialize(&ourSemantics);
  // The Float8E8MOFNU format does not have a representation
  // for zero. So, use the closest representation instead.
  // Moreover, the all-zero encoding represents a valid
  // normal value (which is the smallestNormalized here).
  // Hence, we call makeSmallestNormalized (where category is
  // 'fcNormal') instead of makeZero (where category is 'fcZero').
  ourSemantics.hasZero ? makeZero(অসত্য) : makeSmallestNormalized(অসত্য);
}

// Delegate to the previous constructor, because later copy constructor may
// actually inspects category, which can't be garbage.
IEEEFloat::IEEEFloat(ধ্রুব fltSemantics &ourSemantics, uninitializedTag tag)
    : IEEEFloat(ourSemantics) {}

IEEEFloat::IEEEFloat(ধ্রুব IEEEFloat &rhs) {
  initialize(rhs.semantics);
  assign(rhs);
}

IEEEFloat::IEEEFloat(IEEEFloat &&rhs) : semantics(&APFloatBase::semBogus) {
  *এতত্ = std::move(rhs);
}

IEEEFloat::~IEEEFloat() { freeSignificand(); }

unsigned পূর্ণাঙ্ক IEEEFloat::part_গুনতি() ধ্রুব {
  প্রত্যাগম part_গুনতি_For_Bits(semantics->precision + 1);
}

ধ্রুব APFloat::integerPart *IEEEFloat::significandParts() ধ্রুব {
  প্রত্যাগম const_cast<IEEEFloat *>(এতত্)->significandParts();
}

APFloat::integerPart *IEEEFloat::significandParts() {
  যদি (part_গুনতি() > 1)
    প্রত্যাগম significand.parts;
  অন্যথা
    প্রত্যাগম &significand.part;
}

শূন্য IEEEFloat::zeroSignificand() {
  APInt::tcSet(significandParts(), 0, part_গুনতি());
}

/* Increment an fcNormal floating point number's significand.  */
শূন্য IEEEFloat::incrementSignificand() {
  [[maybe_unused]] integerPart carry =
      APInt::tcIncrement(significandParts(), part_গুনতি());

  /* Our callers should never cause us to overflow.  */
  প্রতিজ্ঞা(carry == 0);
}

/* Add the significand of the RHS.  Returns the carry flag.  */
APFloat::integerPart IEEEFloat::addSignificand(ধ্রুব IEEEFloat &rhs) {
  integerPart *parts = significandParts();

  প্রতিজ্ঞা(semantics == rhs.semantics);
  প্রতিজ্ঞা(exponent == rhs.exponent);

  প্রত্যাগম APInt::tcAdd(parts, rhs.significandParts(), 0, part_গুনতি());
}

/* Subtract the significand of the RHS with a borrow flag.  Returns
   the borrow flag.  */
APFloat::integerPart IEEEFloat::subtractSignificand(ধ্রুব IEEEFloat &rhs,
                                                    integerPart borrow) {
  integerPart *parts = significandParts();

  প্রতিজ্ঞা(semantics == rhs.semantics);
  প্রতিজ্ঞা(exponent == rhs.exponent);

  প্রত্যাগম APInt::tcSubtract(parts, rhs.significandParts(), borrow,
                           part_গুনতি());
}

/* Multiply the significand of the RHS.  If ADDEND is non-NULL, add it
   on to the full-precision result of the multiplication.  Returns the
   lost fraction.  */
lostFraction IEEEFloat::multiplySignificand(ধ্রুব IEEEFloat &rhs,
                                            IEEEFloat addend,
                                            তার্কিক ignoreAddend) {
  integerPart scratch[4];
  তার্কিক ignored;

  প্রতিজ্ঞা(semantics == rhs.semantics);

  unsigned precision = semantics->precision;

  // Allocate space for twice as many bits as the original significand, plus one
  // extra bit for the addition to overflow into.
  unsigned new_Parts_গুনতি = part_গুনতি_For_Bits(precision * 2 + 1);

  // FIXME: Replace with SmallVector<4>.
  integerPart *fullSignificand =
      new_Parts_গুনতি > 4 ? নূতন integerPart[new_Parts_গুনতি] : scratch;

  integerPart *lhsSignificand = significandParts();
  unsigned parts_গুনতি = part_গুনতি();

  APInt::tcFullMultiply(fullSignificand, lhsSignificand,
                        rhs.significandParts(), parts_গুনতি, parts_গুনতি);

  lostFraction lost_fraction = lfExactlyZero;
  // One, not zero, based MSB.
  unsigned omsb = APInt::tcMSB(fullSignificand, new_Parts_গুনতি) + 1;
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

  যদি (!ignoreAddend && addend.isNonZero()) {
    // The intermediate result of the multiplication has "2 * precision"
    // signicant bit; adjust the addend to be consistent with mul result.
    //
    Significand savedSignificand = significand;
    ধ্রুব fltSemantics *savedSemantics = semantics;

    // Normalize our MSB to one below the top bit to allow for overflow.
    unsigned extendedPrecision = 2 * precision + 1;
    যদি (omsb != extendedPrecision - 1) {
      প্রতিজ্ঞা(extendedPrecision > omsb);
      APInt::tc_Shift_বাম(fullSignificand, new_Parts_গুনতি,
                         (extendedPrecision - 1) - omsb);
      exponent -= (extendedPrecision - 1) - omsb;
    }

    /* Create new semantics.  */
    fltSemantics extendedSemantics = *semantics;
    extendedSemantics.precision = extendedPrecision;

    যদি (new_Parts_গুনতি == 1)
      significand.part = fullSignificand[0];
    অন্যথা
      significand.parts = fullSignificand;
    semantics = &extendedSemantics;

    // Make a copy so we can convert it to the extended semantics.
    // Note that we cannot convert the addend directly, as the extendedSemantics
    // is a local variable (which we take a reference to).
    IEEEFloat extendedAddend(addend);
    [[maybe_unused]] opStatus status = extendedAddend.convert(
        extendedSemantics, APFloat::rmTowardZero, &ignored);
    প্রতিজ্ঞা(status == APFloat::opOK);

    // Shift the significand of the addend right by one bit. This guarantees
    // that the high bit of the significand is zero (same as fullSignificand),
    // so the addition will overflow (if it does overflow at all) into the top bit.
    lost_fraction = extendedAddend.shift_Significand_ডান(1);
    প্রতিজ্ঞা(lost_fraction == lfExactlyZero &&
           "Lost precision while shifting addend for fused-multiply-add.");

    lost_fraction = addOrSubtractSignificand(extendedAddend, অসত্য);

    /* Restore our state.  */
    যদি (new_Parts_গুনতি == 1)
      fullSignificand[0] = significand.part;
    significand = savedSignificand;
    semantics = savedSemantics;

    omsb = APInt::tcMSB(fullSignificand, new_Parts_গুনতি) + 1;
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
  যদি (omsb > precision) {
    unsigned পূর্ণাঙ্ক bits, significantParts;
    lostFraction lf;

    bits = omsb - precision;
    significantParts = part_গুনতি_For_Bits(omsb);
    lf = shift_ডান(fullSignificand, significantParts, bits);
    lost_fraction = combineLostFractions(lf, lost_fraction);
    exponent += bits;
  }

  APInt::tcAssign(lhsSignificand, fullSignificand, parts_গুনতি);

  যদি (new_Parts_গুনতি > 4)
    delete [] fullSignificand;

  প্রত্যাগম lost_fraction;
}

lostFraction IEEEFloat::multiplySignificand(ধ্রুব IEEEFloat &rhs) {
  // When the given semantics has zero, the addend here is a zero.
  // i.e . it belongs to the 'fcZero' category.
  // But when the semantics does not support zero, we need to
  // explicitly convey that this addend should be ignored
  // for multiplication.
  প্রত্যাগম multiplySignificand(rhs, IEEEFloat(*semantics), !semantics->hasZero);
}

/* Multiply the significands of LHS and RHS to DST.  */
lostFraction IEEEFloat::divideSignificand(ধ্রুব IEEEFloat &rhs) {
  integerPart scratch[4];

  প্রতিজ্ঞা(semantics == rhs.semantics);

  integerPart *lhsSignificand = significandParts();
  ধ্রুব integerPart *rhsSignificand = rhs.significandParts();
  unsigned parts_গুনতি = part_গুনতি();

  integerPart *dividend =
      parts_গুনতি > 2 ? নূতন integerPart[parts_গুনতি * 2] : scratch;
  integerPart *ভাজক = dividend + parts_গুনতি;

  /* Copy the dividend and divisor as they will be modified in-place.  */
  প্রতি (unsigned i = 0; i < parts_গুনতি; i++) {
    dividend[i] = lhsSignificand[i];
    ভাজক[i] = rhsSignificand[i];
    lhsSignificand[i] = 0;
  }

  exponent -= rhs.exponent;

  unsigned পূর্ণাঙ্ক precision = semantics->precision;

  /* Normalize the divisor.  */
  unsigned bit = precision - APInt::tcMSB(ভাজক, parts_গুনতি) - 1;
  যদি (bit) {
    exponent += bit;
    APInt::tc_Shift_বাম(ভাজক, parts_গুনতি, bit);
  }

  /* Normalize the dividend.  */
  bit = precision - APInt::tcMSB(dividend, parts_গুনতি) - 1;
  যদি (bit) {
    exponent -= bit;
    APInt::tc_Shift_বাম(dividend, parts_গুনতি, bit);
  }

  /* Ensure the dividend >= divisor initially for the loop below.
     Incidentally, this means that the division loop below is
     guaranteed to set the integer bit to one.  */
  যদি (APInt::tcCompare(dividend, ভাজক, parts_গুনতি) < 0) {
    exponent--;
    APInt::tc_Shift_বাম(dividend, parts_গুনতি, 1);
    প্রতিজ্ঞা(APInt::tcCompare(dividend, ভাজক, parts_গুনতি) >= 0);
  }

  /* Long division.  */
  প্রতি (bit = precision; bit; bit -= 1) {
    যদি (APInt::tcCompare(dividend, ভাজক, parts_গুনতি) >= 0) {
      APInt::tcSubtract(dividend, ভাজক, 0, parts_গুনতি);
      APInt::tcSetBit(lhsSignificand, bit - 1);
    }

    APInt::tc_Shift_বাম(dividend, parts_গুনতি, 1);
  }

  /* Figure out the lost fraction.  */
  পূর্ণাঙ্ক cmp = APInt::tcCompare(dividend, ভাজক, parts_গুনতি);

  lostFraction lost_fraction;
  যদি (cmp > 0)
    lost_fraction = lfMoreThanHalf;
  অন্যথা যদি (cmp == 0)
    lost_fraction = lfExactlyHalf;
  অন্যথা যদি (APInt::tcIsZero(dividend, parts_গুনতি))
    lost_fraction = lfExactlyZero;
  অন্যথা
    lost_fraction = lfLessThanHalf;

  যদি (parts_গুনতি > 2)
    delete [] dividend;

  প্রত্যাগম lost_fraction;
}

unsigned পূর্ণাঙ্ক IEEEFloat::significandMSB() ধ্রুব {
  প্রত্যাগম APInt::tcMSB(significandParts(), part_গুনতি());
}

unsigned পূর্ণাঙ্ক IEEEFloat::significandLSB() ধ্রুব {
  প্রত্যাগম APInt::tcLSB(significandParts(), part_গুনতি());
}

/* Note that a zero result is NOT normalized to fcZero.  */
lostFraction IEEEFloat::shift_Significand_ডান(unsigned পূর্ণাঙ্ক bits) {
  /* Our exponent should not overflow.  */
  প্রতিজ্ঞা((ExponentType) (exponent + bits) >= exponent);

  exponent += bits;

  প্রত্যাগম shift_ডান(significandParts(), part_গুনতি(), bits);
}

/* Shift the significand left BITS bits, subtract BITS from its exponent.  */
শূন্য IEEEFloat::shift_Significand_বাম(unsigned পূর্ণাঙ্ক bits) {
  প্রতিজ্ঞা(bits < semantics->precision ||
         (semantics->precision == 1 && bits <= 1));

  যদি (bits) {
    unsigned পূর্ণাঙ্ক parts_গুনতি = part_গুনতি();

    APInt::tc_Shift_বাম(significandParts(), parts_গুনতি, bits);
    exponent -= bits;

    প্রতিজ্ঞা(!APInt::tcIsZero(significandParts(), parts_গুনতি));
  }
}

APFloat::cmpResult IEEEFloat::compare_Absolute_মান(ধ্রুব IEEEFloat &rhs) ধ্রুব {
  প্রতিজ্ঞা(semantics == rhs.semantics);
  প্রতিজ্ঞা(isFiniteNonZero());
  প্রতিজ্ঞা(rhs.isFiniteNonZero());

  পূর্ণাঙ্ক compare = exponent - rhs.exponent;

  /* If exponents are equal, do an unsigned bignum comparison of the
     significands.  */
  যদি (compare == 0)
    compare = APInt::tcCompare(significandParts(), rhs.significandParts(),
                               part_গুনতি());

  যদি (compare > 0)
    প্রত্যাগম cmpGreaterThan;
  অন্যথা যদি (compare < 0)
    প্রত্যাগম cmpLessThan;
  অন্যথা
    প্রত্যাগম cmpEqual;
}

/* Set the least significant BITS bits of a bignum, clear the
   rest.  */
স্থির শূন্য tcSetLeastSignificantBits(APInt::WordType *dst, unsigned parts,
                                      unsigned bits) {
  unsigned i = 0;
  যাবত্ (bits > APInt::APINT_BITS_PER_WORD) {
    dst[i++] = ~(APInt::WordType)0;
    bits -= APInt::APINT_BITS_PER_WORD;
  }

  যদি (bits)
    dst[i++] = ~(APInt::WordType)0 >> (APInt::APINT_BITS_PER_WORD - bits);

  যাবত্ (i < parts)
    dst[i++] = 0;
}

/* Handle overflow.  Sign is preserved.  We either become infinity or
   the largest finite number.  */
APFloat::opStatus IEEEFloat::handleOverflow(roundingMode rounding_mode) {
  যদি (semantics->nonFiniteBehavior != fltNonfiniteBehavior::FiniteOnly) {
    /* Infinity?  */
    যদি (rounding_mode == rmNearestTiesToEven ||
        rounding_mode == rmNearestTiesToAway ||
        (rounding_mode == rmTowardPositive && !sign) ||
        (rounding_mode == rmTowardNegative && sign)) {
      যদি (semantics->nonFiniteBehavior == fltNonfiniteBehavior::NanOnly)
        makeNaN(অসত্য, sign);
      অন্যথা
        category = fcInfinity;
      প্রত্যাগম static_cast<opStatus>(opOverflow | opInexact);
    }
  }

  /* Otherwise we become the largest finite number.  */
  category = fcNormal;
  exponent = semantics->maxExponent;
  tcSetLeastSignificantBits(significandParts(), part_গুনতি(),
                            semantics->precision);
  যদি (semantics->nonFiniteBehavior == fltNonfiniteBehavior::NanOnly &&
      semantics->nanEncoding == fltNanEncoding::AllOnes)
    APInt::tcClearBit(significandParts(), 0);

  প্রত্যাগম opInexact;
}

/* Returns TRUE if, when truncating the current number, with BIT the
   new LSB, with the given lost fraction and rounding mode, the result
   would need to be rounded away from zero (i.e., by increasing the
   signficand).  This routine must work for fcZero of both signs, and
   fcNormal numbers.  */
তার্কিক IEEEFloat::roundAwayFromZero(roundingMode rounding_mode,
                                  lostFraction lost_fraction,
                                  unsigned পূর্ণাঙ্ক bit) ধ্রুব {
  /* NaNs and infinities should not have lost fractions.  */
  প্রতিজ্ঞা(isFiniteNonZero() || category == fcZero);

  /* Current callers never pass this so we don't handle it.  */
  প্রতিজ্ঞা(lost_fraction != lfExactlyZero);

  বিকল্প (rounding_mode) {
  পক্ষ rmNearestTiesToAway:
    প্রত্যাগম lost_fraction == lfExactlyHalf || lost_fraction == lfMoreThanHalf;

  পক্ষ rmNearestTiesToEven:
    যদি (lost_fraction == lfMoreThanHalf)
      প্রত্যাগম সত্য;

    /* Our zeroes don't have a significand to test.  */
    যদি (lost_fraction == lfExactlyHalf && category != fcZero)
      প্রত্যাগম APInt::tcExtractBit(significandParts(), bit);

    প্রত্যাগম অসত্য;

  পক্ষ rmTowardZero:
    প্রত্যাগম অসত্য;

  পক্ষ rmTowardPositive:
    প্রত্যাগম !sign;

  পক্ষ rmTowardNegative:
    প্রত্যাগম sign;

  স্বভাব:
    ভঙ্গ;
  }
  llvm_unreachable("Invalid rounding mode found");
}

APFloat::opStatus IEEEFloat::normalize(roundingMode rounding_mode,
                                       lostFraction lost_fraction) {
  যদি (!isFiniteNonZero())
    প্রত্যাগম opOK;

  /* Before rounding normalize the exponent of fcNormal numbers.  */
  /* One, not zero, based MSB.  */
  unsigned omsb = significandMSB() + 1;

  // Only skip this `if` if the value is exactly zero.
  যদি (omsb || lost_fraction != lfExactlyZero) {
    /* OMSB is numbered from 1.  We want to place it in the integer
       bit numbered PRECISION if possible, with a compensating change in
       the exponent.  */
    পূর্ণাঙ্ক exponentChange = omsb - semantics->precision;

    /* If the resulting exponent is too high, overflow according to
       the rounding mode.  */
    যদি (exponent + exponentChange > semantics->maxExponent)
      প্রত্যাগম handleOverflow(rounding_mode);

    /* Subnormal numbers have exponent minExponent, and their MSB
       is forced based on that.  */
    যদি (exponent + exponentChange < semantics->minExponent)
      exponentChange = semantics->minExponent - exponent;

    /* Shifting left is easy as we don't lose precision.  */
    যদি (exponentChange < 0) {
      প্রতিজ্ঞা(lost_fraction == lfExactlyZero);

      shift_Significand_বাম(-exponentChange);

      প্রত্যাগম opOK;
    }

    যদি (exponentChange > 0) {
      lostFraction lf;

      /* Shift right and capture any new lost fraction.  */
      lf = shift_Significand_ডান(exponentChange);

      lost_fraction = combineLostFractions(lf, lost_fraction);

      /* Keep OMSB up-to-date.  */
      যদি (omsb > (unsigned) exponentChange)
        omsb -= exponentChange;
      অন্যথা
        omsb = 0;
    }
  }

  // The all-ones values is an overflow if NaN is all ones. If NaN is
  // represented by negative zero, then it is a valid finite value.
  যদি (semantics->nonFiniteBehavior == fltNonfiniteBehavior::NanOnly &&
      semantics->nanEncoding == fltNanEncoding::AllOnes &&
      exponent == semantics->maxExponent && isSignificandAllOnes())
    প্রত্যাগম handleOverflow(rounding_mode);

  /* Now round the number according to rounding_mode given the lost
     fraction.  */

  /* As specified in IEEE 754, since we do not trap we do not report
     underflow for exact results.  */
  যদি (lost_fraction == lfExactlyZero) {
    /* Canonicalize zeroes.  */
    যদি (omsb == 0) {
      category = fcZero;
      যদি (semantics->nanEncoding == fltNanEncoding::NegativeZero)
        sign = অসত্য;
      যদি (!semantics->hasZero)
        makeSmallestNormalized(অসত্য);
    }

    প্রত্যাগম opOK;
  }

  /* Increment the significand if we're rounding away from zero.  */
  যদি (roundAwayFromZero(rounding_mode, lost_fraction, 0)) {
    যদি (omsb == 0)
      exponent = semantics->minExponent;

    incrementSignificand();
    omsb = significandMSB() + 1;

    /* Did the significand increment overflow?  */
    যদি (omsb == (unsigned) semantics->precision + 1) {
      /* Renormalize by incrementing the exponent and shifting our
         significand right one.  However if we already have the
         maximum exponent we overflow to infinity.  */
      যদি (exponent == semantics->maxExponent)
        // Invoke overflow handling with a rounding mode that will guarantee
        // that the result gets turned into the correct infinity representation.
        // This is needed instead of just setting the category to infinity to
        // account for 8-bit floating point types that have no inf, only NaN.
        প্রত্যাগম handleOverflow(sign ? rmTowardNegative : rmTowardPositive);

      shift_Significand_ডান(1);

      প্রত্যাগম opInexact;
    }

    // The all-ones values is an overflow if NaN is all ones. If NaN is
    // represented by negative zero, then it is a valid finite value.
    যদি (semantics->nonFiniteBehavior == fltNonfiniteBehavior::NanOnly &&
        semantics->nanEncoding == fltNanEncoding::AllOnes &&
        exponent == semantics->maxExponent && isSignificandAllOnes())
      প্রত্যাগম handleOverflow(rounding_mode);
  }

  /* The normal case - we were and are not denormal, and any
     significand increment above didn't overflow.  */
  যদি (omsb == semantics->precision)
    প্রত্যাগম opInexact;

  /* We have a non-zero denormal.  */
  প্রতিজ্ঞা(omsb < semantics->precision);

  /* Canonicalize zeroes.  */
  যদি (omsb == 0) {
    category = fcZero;
    যদি (semantics->nanEncoding == fltNanEncoding::NegativeZero)
      sign = অসত্য;
    // This condition handles the case where the semantics
    // does not have zero but uses the all-zero encoding
    // to represent the smallest normal value.
    যদি (!semantics->hasZero)
      makeSmallestNormalized(অসত্য);
  }

  /* The fcZero case is a denormal that underflowed to zero.  */
  প্রত্যাগম (opStatus) (opUnderflow | opInexact);
}

APFloat::opStatus IEEEFloat::addOrSubtractSpecials(ধ্রুব IEEEFloat &rhs,
                                                   তার্কিক subtract) {
  বিকল্প (Pack_Categories_Into_চাবি(category, rhs.category)) {
  স্বভাব:
    llvm_unreachable(nullptr);

  পক্ষ Pack_Categories_Into_চাবি(fcZero, fcNaN):
  পক্ষ Pack_Categories_Into_চাবি(fcNormal, fcNaN):
  পক্ষ Pack_Categories_Into_চাবি(fcInfinity, fcNaN):
    assign(rhs);
    [[fallthrough]];
  পক্ষ Pack_Categories_Into_চাবি(fcNaN, fcZero):
  পক্ষ Pack_Categories_Into_চাবি(fcNaN, fcNormal):
  পক্ষ Pack_Categories_Into_চাবি(fcNaN, fcInfinity):
  পক্ষ Pack_Categories_Into_চাবি(fcNaN, fcNaN):
    যদি (isSignaling()) {
      makeQuiet();
      প্রত্যাগম opInvalidOp;
    }
    প্রত্যাগম rhs.isSignaling() ? opInvalidOp : opOK;

  পক্ষ Pack_Categories_Into_চাবি(fcNormal, fcZero):
  পক্ষ Pack_Categories_Into_চাবি(fcInfinity, fcNormal):
  পক্ষ Pack_Categories_Into_চাবি(fcInfinity, fcZero):
    প্রত্যাগম opOK;

  পক্ষ Pack_Categories_Into_চাবি(fcNormal, fcInfinity):
  পক্ষ Pack_Categories_Into_চাবি(fcZero, fcInfinity):
    category = fcInfinity;
    sign = rhs.sign ^ subtract;
    প্রত্যাগম opOK;

  পক্ষ Pack_Categories_Into_চাবি(fcZero, fcNormal):
    assign(rhs);
    sign = rhs.sign ^ subtract;
    প্রত্যাগম opOK;

  পক্ষ Pack_Categories_Into_চাবি(fcZero, fcZero):
    /* Sign depends on rounding mode; handled by caller.  */
    প্রত্যাগম opOK;

  পক্ষ Pack_Categories_Into_চাবি(fcInfinity, fcInfinity):
    /* Differently signed infinities can only be validly
       subtracted.  */
    যদি (((sign ^ rhs.sign)!=0) != subtract) {
      makeNaN();
      প্রত্যাগম opInvalidOp;
    }

    প্রত্যাগম opOK;

  পক্ষ Pack_Categories_Into_চাবি(fcNormal, fcNormal):
    প্রত্যাগম opDivByZero;
  }
}

/* Add or subtract two normal numbers.  */
lostFraction IEEEFloat::addOrSubtractSignificand(ধ্রুব IEEEFloat &rhs,
                                                 তার্কিক subtract) {
  [[maybe_unused]] integerPart carry = 0;
  lostFraction lost_fraction;

  /* Determine if the operation on the absolute values is effectively
     an addition or subtraction.  */
  subtract ^= static_cast<তার্কিক>(sign ^ rhs.sign);

  /* Are we bigger exponent-wise than the RHS?  */
  পূর্ণাঙ্ক bits = exponent - rhs.exponent;

  /* Subtraction is more subtle than one might naively expect.  */
  যদি (subtract) {
    যদি ((bits < 0) && !semantics->hasSignedRepr)
      llvm_unreachable(
          "This floating point format does not support signed values");

    IEEEFloat temp_rhs(rhs);
    তার্কিক lost_fraction_is_from_rhs = অসত্য;

    যদি (bits == 0)
      lost_fraction = lfExactlyZero;
    অন্যথা যদি (bits > 0) {
      lost_fraction = temp_rhs.shift_Significand_ডান(bits - 1);
      lost_fraction_is_from_rhs = সত্য;
      shift_Significand_বাম(1);
    } অন্যথা {
      lost_fraction = shift_Significand_ডান(-bits - 1);
      temp_rhs.shift_Significand_বাম(1);
    }

    // Should we reverse the subtraction.
    cmpResult cmp_result = compare_Absolute_মান(temp_rhs);
    যদি (cmp_result == cmpLessThan) {
      তার্কিক borrow =
          lost_fraction != lfExactlyZero && !lost_fraction_is_from_rhs;
      যদি (borrow) {
        // The lost fraction is being subtracted, borrow from the significand
        // and invert `lost_fraction`.
        যদি (lost_fraction == lfLessThanHalf)
          lost_fraction = lfMoreThanHalf;
        অন্যথা যদি (lost_fraction == lfMoreThanHalf)
          lost_fraction = lfLessThanHalf;
      }
      carry = temp_rhs.subtractSignificand(*এতত্, borrow);
      copySignificand(temp_rhs);
      sign = !sign;
    } অন্যথা যদি (cmp_result == cmpGreaterThan) {
      তার্কিক borrow = lost_fraction != lfExactlyZero && lost_fraction_is_from_rhs;
      যদি (borrow) {
        // The lost fraction is being subtracted, borrow from the significand
        // and invert `lost_fraction`.
        যদি (lost_fraction == lfLessThanHalf)
          lost_fraction = lfMoreThanHalf;
        অন্যথা যদি (lost_fraction == lfMoreThanHalf)
          lost_fraction = lfLessThanHalf;
      }
      carry = subtractSignificand(temp_rhs, borrow);
    } অন্যথা { // cmpEqual
      zeroSignificand();
      যদি (lost_fraction != lfExactlyZero && lost_fraction_is_from_rhs) {
        // rhs is slightly larger due to the lost fraction, flip the sign.
        sign = !sign;
      }
    }

    /* The code above is intended to ensure that no borrow is
       necessary.  */
    প্রতিজ্ঞা(!carry);
  } অন্যথা {
    যদি (bits > 0) {
      IEEEFloat temp_rhs(rhs);

      lost_fraction = temp_rhs.shift_Significand_ডান(bits);
      carry = addSignificand(temp_rhs);
    } অন্যথা {
      lost_fraction = shift_Significand_ডান(-bits);
      carry = addSignificand(rhs);
    }

    /* We have a guard bit; generating a carry cannot happen.  */
    প্রতিজ্ঞা(!carry);
  }

  প্রত্যাগম lost_fraction;
}

APFloat::opStatus IEEEFloat::multiplySpecials(ধ্রুব IEEEFloat &rhs) {
  বিকল্প (Pack_Categories_Into_চাবি(category, rhs.category)) {
  স্বভাব:
    llvm_unreachable(nullptr);

  পক্ষ Pack_Categories_Into_চাবি(fcZero, fcNaN):
  পক্ষ Pack_Categories_Into_চাবি(fcNormal, fcNaN):
  পক্ষ Pack_Categories_Into_চাবি(fcInfinity, fcNaN):
    assign(rhs);
    sign = অসত্য;
    [[fallthrough]];
  পক্ষ Pack_Categories_Into_চাবি(fcNaN, fcZero):
  পক্ষ Pack_Categories_Into_চাবি(fcNaN, fcNormal):
  পক্ষ Pack_Categories_Into_চাবি(fcNaN, fcInfinity):
  পক্ষ Pack_Categories_Into_চাবি(fcNaN, fcNaN):
    sign ^= rhs.sign; // restore the original sign
    যদি (isSignaling()) {
      makeQuiet();
      প্রত্যাগম opInvalidOp;
    }
    প্রত্যাগম rhs.isSignaling() ? opInvalidOp : opOK;

  পক্ষ Pack_Categories_Into_চাবি(fcNormal, fcInfinity):
  পক্ষ Pack_Categories_Into_চাবি(fcInfinity, fcNormal):
  পক্ষ Pack_Categories_Into_চাবি(fcInfinity, fcInfinity):
    category = fcInfinity;
    প্রত্যাগম opOK;

  পক্ষ Pack_Categories_Into_চাবি(fcZero, fcNormal):
  পক্ষ Pack_Categories_Into_চাবি(fcNormal, fcZero):
  পক্ষ Pack_Categories_Into_চাবি(fcZero, fcZero):
    category = fcZero;
    প্রত্যাগম opOK;

  পক্ষ Pack_Categories_Into_চাবি(fcZero, fcInfinity):
  পক্ষ Pack_Categories_Into_চাবি(fcInfinity, fcZero):
    makeNaN();
    প্রত্যাগম opInvalidOp;

  পক্ষ Pack_Categories_Into_চাবি(fcNormal, fcNormal):
    প্রত্যাগম opOK;
  }
}

APFloat::opStatus IEEEFloat::divideSpecials(ধ্রুব IEEEFloat &rhs) {
  বিকল্প (Pack_Categories_Into_চাবি(category, rhs.category)) {
  স্বভাব:
    llvm_unreachable(nullptr);

  পক্ষ Pack_Categories_Into_চাবি(fcZero, fcNaN):
  পক্ষ Pack_Categories_Into_চাবি(fcNormal, fcNaN):
  পক্ষ Pack_Categories_Into_চাবি(fcInfinity, fcNaN):
    assign(rhs);
    sign = অসত্য;
    [[fallthrough]];
  পক্ষ Pack_Categories_Into_চাবি(fcNaN, fcZero):
  পক্ষ Pack_Categories_Into_চাবি(fcNaN, fcNormal):
  পক্ষ Pack_Categories_Into_চাবি(fcNaN, fcInfinity):
  পক্ষ Pack_Categories_Into_চাবি(fcNaN, fcNaN):
    sign ^= rhs.sign; // restore the original sign
    যদি (isSignaling()) {
      makeQuiet();
      প্রত্যাগম opInvalidOp;
    }
    প্রত্যাগম rhs.isSignaling() ? opInvalidOp : opOK;

  পক্ষ Pack_Categories_Into_চাবি(fcInfinity, fcZero):
  পক্ষ Pack_Categories_Into_চাবি(fcInfinity, fcNormal):
  পক্ষ Pack_Categories_Into_চাবি(fcZero, fcInfinity):
  পক্ষ Pack_Categories_Into_চাবি(fcZero, fcNormal):
    প্রত্যাগম opOK;

  পক্ষ Pack_Categories_Into_চাবি(fcNormal, fcInfinity):
    category = fcZero;
    প্রত্যাগম opOK;

  পক্ষ Pack_Categories_Into_চাবি(fcNormal, fcZero):
    যদি (semantics->nonFiniteBehavior == fltNonfiniteBehavior::NanOnly)
      makeNaN(অসত্য, sign);
    অন্যথা
      category = fcInfinity;
    প্রত্যাগম opDivByZero;

  পক্ষ Pack_Categories_Into_চাবি(fcInfinity, fcInfinity):
  পক্ষ Pack_Categories_Into_চাবি(fcZero, fcZero):
    makeNaN();
    প্রত্যাগম opInvalidOp;

  পক্ষ Pack_Categories_Into_চাবি(fcNormal, fcNormal):
    প্রত্যাগম opOK;
  }
}

APFloat::opStatus IEEEFloat::modSpecials(ধ্রুব IEEEFloat &rhs) {
  বিকল্প (Pack_Categories_Into_চাবি(category, rhs.category)) {
  স্বভাব:
    llvm_unreachable(nullptr);

  পক্ষ Pack_Categories_Into_চাবি(fcZero, fcNaN):
  পক্ষ Pack_Categories_Into_চাবি(fcNormal, fcNaN):
  পক্ষ Pack_Categories_Into_চাবি(fcInfinity, fcNaN):
    assign(rhs);
    [[fallthrough]];
  পক্ষ Pack_Categories_Into_চাবি(fcNaN, fcZero):
  পক্ষ Pack_Categories_Into_চাবি(fcNaN, fcNormal):
  পক্ষ Pack_Categories_Into_চাবি(fcNaN, fcInfinity):
  পক্ষ Pack_Categories_Into_চাবি(fcNaN, fcNaN):
    যদি (isSignaling()) {
      makeQuiet();
      প্রত্যাগম opInvalidOp;
    }
    প্রত্যাগম rhs.isSignaling() ? opInvalidOp : opOK;

  পক্ষ Pack_Categories_Into_চাবি(fcZero, fcInfinity):
  পক্ষ Pack_Categories_Into_চাবি(fcZero, fcNormal):
  পক্ষ Pack_Categories_Into_চাবি(fcNormal, fcInfinity):
    প্রত্যাগম opOK;

  পক্ষ Pack_Categories_Into_চাবি(fcNormal, fcZero):
  পক্ষ Pack_Categories_Into_চাবি(fcInfinity, fcZero):
  পক্ষ Pack_Categories_Into_চাবি(fcInfinity, fcNormal):
  পক্ষ Pack_Categories_Into_চাবি(fcInfinity, fcInfinity):
  পক্ষ Pack_Categories_Into_চাবি(fcZero, fcZero):
    makeNaN();
    প্রত্যাগম opInvalidOp;

  পক্ষ Pack_Categories_Into_চাবি(fcNormal, fcNormal):
    প্রত্যাগম opOK;
  }
}

APFloat::opStatus IEEEFloat::remainderSpecials(ধ্রুব IEEEFloat &rhs) {
  বিকল্প (Pack_Categories_Into_চাবি(category, rhs.category)) {
  স্বভাব:
    llvm_unreachable(nullptr);

  পক্ষ Pack_Categories_Into_চাবি(fcZero, fcNaN):
  পক্ষ Pack_Categories_Into_চাবি(fcNormal, fcNaN):
  পক্ষ Pack_Categories_Into_চাবি(fcInfinity, fcNaN):
    assign(rhs);
    [[fallthrough]];
  পক্ষ Pack_Categories_Into_চাবি(fcNaN, fcZero):
  পক্ষ Pack_Categories_Into_চাবি(fcNaN, fcNormal):
  পক্ষ Pack_Categories_Into_চাবি(fcNaN, fcInfinity):
  পক্ষ Pack_Categories_Into_চাবি(fcNaN, fcNaN):
    যদি (isSignaling()) {
      makeQuiet();
      প্রত্যাগম opInvalidOp;
    }
    প্রত্যাগম rhs.isSignaling() ? opInvalidOp : opOK;

  পক্ষ Pack_Categories_Into_চাবি(fcZero, fcInfinity):
  পক্ষ Pack_Categories_Into_চাবি(fcZero, fcNormal):
  পক্ষ Pack_Categories_Into_চাবি(fcNormal, fcInfinity):
    প্রত্যাগম opOK;

  পক্ষ Pack_Categories_Into_চাবি(fcNormal, fcZero):
  পক্ষ Pack_Categories_Into_চাবি(fcInfinity, fcZero):
  পক্ষ Pack_Categories_Into_চাবি(fcInfinity, fcNormal):
  পক্ষ Pack_Categories_Into_চাবি(fcInfinity, fcInfinity):
  পক্ষ Pack_Categories_Into_চাবি(fcZero, fcZero):
    makeNaN();
    প্রত্যাগম opInvalidOp;

  পক্ষ Pack_Categories_Into_চাবি(fcNormal, fcNormal):
    প্রত্যাগম opDivByZero; // fake status, indicating this is not a special case
  }
}

/* Change sign.  */
শূন্য IEEEFloat::changeSign() {
  // With NaN-as-negative-zero, neither NaN or negative zero can change
  // their signs.
  যদি (semantics->nanEncoding == fltNanEncoding::NegativeZero &&
      (isZero() || isNaN()))
    প্রত্যাগম;
  /* Look mummy, this one's easy.  */
  sign = !sign;
}

/* Normalized addition or subtraction.  */
APFloat::opStatus IEEEFloat::addOrSubtract(ধ্রুব IEEEFloat &rhs,
                                           roundingMode rounding_mode,
                                           তার্কিক subtract) {
  opStatus fs = addOrSubtractSpecials(rhs, subtract);

  /* This return code means it was not a simple case.  */
  যদি (fs == opDivByZero) {
    lostFraction lost_fraction;

    lost_fraction = addOrSubtractSignificand(rhs, subtract);
    fs = normalize(rounding_mode, lost_fraction);

    /* Can only be zero if we lost no fraction.  */
    প্রতিজ্ঞা(category != fcZero || lost_fraction == lfExactlyZero);
  }

  /* If two numbers add (exactly) to zero, IEEE 754 decrees it is a
     positive zero unless rounding to minus infinity, except that
     adding two like-signed zeroes gives that zero.  */
  যদি (category == fcZero) {
    যদি (rhs.category != fcZero || (sign == rhs.sign) == subtract)
      sign = (rounding_mode == rmTowardNegative);
    // NaN-in-negative-zero means zeros need to be normalized to +0.
    যদি (semantics->nanEncoding == fltNanEncoding::NegativeZero)
      sign = অসত্য;
  }

  প্রত্যাগম fs;
}

/* Normalized addition.  */
APFloat::opStatus IEEEFloat::add(ধ্রুব IEEEFloat &rhs,
                                 roundingMode rounding_mode) {
  প্রত্যাগম addOrSubtract(rhs, rounding_mode, অসত্য);
}

/* Normalized subtraction.  */
APFloat::opStatus IEEEFloat::subtract(ধ্রুব IEEEFloat &rhs,
                                      roundingMode rounding_mode) {
  প্রত্যাগম addOrSubtract(rhs, rounding_mode, সত্য);
}

/* Normalized multiply.  */
APFloat::opStatus IEEEFloat::multiply(ধ্রুব IEEEFloat &rhs,
                                      roundingMode rounding_mode) {
  sign ^= rhs.sign;
  opStatus fs = multiplySpecials(rhs);

  যদি (isZero() && semantics->nanEncoding == fltNanEncoding::NegativeZero)
    sign = অসত্য;
  যদি (isFiniteNonZero()) {
    lostFraction lost_fraction = multiplySignificand(rhs);
    fs = normalize(rounding_mode, lost_fraction);
    যদি (lost_fraction != lfExactlyZero)
      fs = (opStatus) (fs | opInexact);
  }

  প্রত্যাগম fs;
}

/* Normalized divide.  */
APFloat::opStatus IEEEFloat::divide(ধ্রুব IEEEFloat &rhs,
                                    roundingMode rounding_mode) {
  sign ^= rhs.sign;
  opStatus fs = divideSpecials(rhs);

  যদি (isZero() && semantics->nanEncoding == fltNanEncoding::NegativeZero)
    sign = অসত্য;
  যদি (isFiniteNonZero()) {
    lostFraction lost_fraction = divideSignificand(rhs);
    fs = normalize(rounding_mode, lost_fraction);
    যদি (lost_fraction != lfExactlyZero)
      fs = (opStatus) (fs | opInexact);
  }

  প্রত্যাগম fs;
}

/* Normalized remainder.  */
APFloat::opStatus IEEEFloat::remainder(ধ্রুব IEEEFloat &rhs) {
  unsigned পূর্ণাঙ্ক origSign = sign;

  // First handle the special cases.
  opStatus fs = remainderSpecials(rhs);
  যদি (fs != opDivByZero)
    প্রত্যাগম fs;

  fs = opOK;

  // Make sure the current value is less than twice the denom. If the addition
  // did not succeed (an overflow has happened), which means that the finite
  // value we currently posses must be less than twice the denom (as we are
  // using the same semantics).
  IEEEFloat P2 = rhs;
  যদি (P2.add(rhs, rmNearestTiesToEven) == opOK) {
    fs = mod(P2);
    প্রতিজ্ঞা(fs == opOK);
  }

  // Lets work with absolute numbers.
  IEEEFloat P = rhs;
  P.sign = অসত্য;
  sign = অসত্য;

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
  তার্কিক losesInfo;
  fltSemantics extendedSemantics = *semantics;
  extendedSemantics.maxExponent++;
  extendedSemantics.minExponent--;
  extendedSemantics.precision += 2;

  IEEEFloat VEx = *এতত্;
  fs = VEx.convert(extendedSemantics, rmNearestTiesToEven, &losesInfo);
  প্রতিজ্ঞা(fs == opOK && !losesInfo);
  IEEEFloat PEx = P;
  fs = PEx.convert(extendedSemantics, rmNearestTiesToEven, &losesInfo);
  প্রতিজ্ঞা(fs == opOK && !losesInfo);

  // It is simpler to work with 2x instead of 0.5p, and we do not need to lose
  // any fraction.
  fs = VEx.add(VEx, rmNearestTiesToEven);
  প্রতিজ্ঞা(fs == opOK);

  যদি (VEx.compare(PEx) == cmpGreaterThan) {
    fs = subtract(P, rmNearestTiesToEven);
    প্রতিজ্ঞা(fs == opOK);

    // Make VEx = this.add(this), but because we have different semantics, we do
    // not want to `convert` again, so we just subtract PEx twice (which equals
    // to the desired value).
    fs = VEx.subtract(PEx, rmNearestTiesToEven);
    প্রতিজ্ঞা(fs == opOK);
    fs = VEx.subtract(PEx, rmNearestTiesToEven);
    প্রতিজ্ঞা(fs == opOK);

    cmpResult result = VEx.compare(PEx);
    যদি (result == cmpGreaterThan || result == cmpEqual) {
      fs = subtract(P, rmNearestTiesToEven);
      প্রতিজ্ঞা(fs == opOK);
    }
  }

  যদি (isZero()) {
    sign = origSign;    // IEEE754 requires this
    যদি (semantics->nanEncoding == fltNanEncoding::NegativeZero)
      // But some 8-bit floats only have positive 0.
      sign = অসত্য;
  } অন্যথা {
    sign ^= origSign;
  }
  প্রত্যাগম fs;
}

/* Normalized llvm frem (C fmod). */
APFloat::opStatus IEEEFloat::mod(ধ্রুব IEEEFloat &rhs) {
  opStatus fs = modSpecials(rhs);
  unsigned পূর্ণাঙ্ক origSign = sign;

  যাবত্ (isFiniteNonZero() && rhs.isFiniteNonZero() &&
         compare_Absolute_মান(rhs) != cmpLessThan) {
    পূর্ণাঙ্ক Exp = ilogb(*এতত্) - ilogb(rhs);
    IEEEFloat V = scalbn(rhs, Exp, rmNearestTiesToEven);
    // V can overflow to NaN with fltNonfiniteBehavior::NanOnly, so explicitly
    // check for it.
    যদি (V.isNaN() || compare_Absolute_মান(V) == cmpLessThan)
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
    যদি (!semantics->hasZero && এতত্->isSmallest())
      ভঙ্গ;

    প্রতিজ্ঞা(fs==opOK);
  }
  যদি (isZero()) {
    sign = origSign; // fmod requires this
    যদি (semantics->nanEncoding == fltNanEncoding::NegativeZero)
      sign = অসত্য;
  }
  প্রত্যাগম fs;
}

/* Normalized fused-multiply-add.  */
APFloat::opStatus IEEEFloat::fusedMultiplyAdd(ধ্রুব IEEEFloat &multiplicand,
                                              ধ্রুব IEEEFloat &addend,
                                              roundingMode rounding_mode) {
  opStatus fs;

  /* Post-multiplication sign, before addition.  */
  sign ^= multiplicand.sign;

  /* If and only if all arguments are normal do we need to do an
     extended-precision calculation.  */
  যদি (isFiniteNonZero() &&
      multiplicand.isFiniteNonZero() &&
      addend.isFinite()) {
    lostFraction lost_fraction;

    lost_fraction = multiplySignificand(multiplicand, addend);
    fs = normalize(rounding_mode, lost_fraction);
    যদি (lost_fraction != lfExactlyZero)
      fs = (opStatus) (fs | opInexact);

    /* If two numbers add (exactly) to zero, IEEE 754 decrees it is a
       positive zero unless rounding to minus infinity, except that
       adding two like-signed zeroes gives that zero.  */
    যদি (category == fcZero && !(fs & opUnderflow) && sign != addend.sign) {
      sign = (rounding_mode == rmTowardNegative);
      যদি (semantics->nanEncoding == fltNanEncoding::NegativeZero)
        sign = অসত্য;
    }
  } অন্যথা {
    fs = multiplySpecials(multiplicand);

    /* FS can only be opOK or opInvalidOp.  There is no more work
       to do in the latter case.  The IEEE-754R standard says it is
       implementation-defined in this case whether, if ADDEND is a
       quiet NaN, we raise invalid op; this implementation does so.

       If we need to do the addition we can do so with normal
       precision.  */
    যদি (fs == opOK)
      fs = addOrSubtract(addend, rounding_mode, অসত্য);
  }

  প্রত্যাগম fs;
}

/* Rounding-mode correct round to integral value.  */
APFloat::opStatus IEEEFloat::roundToIntegral(roundingMode rounding_mode) {
  যদি (isInfinity())
    // [IEEE Std 754-2008 6.1]:
    // The behavior of infinity in floating-point arithmetic is derived from the
    // limiting cases of real arithmetic with operands of arbitrarily
    // large magnitude, when such a limit exists.
    // ...
    // Operations on infinite operands are usually exact and therefore signal no
    // exceptions ...
    প্রত্যাগম opOK;

  যদি (isNaN()) {
    যদি (isSignaling()) {
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
      প্রত্যাগম opInvalidOp;
    } অন্যথা {
      // [IEEE Std 754-2008 6.2]:
      // For an operation with quiet NaN inputs, other than maximum and minimum
      // operations, if a floating-point result is to be delivered the result
      // shall be a quiet NaN which should be one of the input NaNs.
      // ...
      // Every general-computational and quiet-computational operation involving
      // one or more input NaNs, none of them signaling, shall signal no
      // exception, except fusedMultiplyAdd might signal the invalid operation
      // exception(see 7.2).
      প্রত্যাগম opOK;
    }
  }

  যদি (isZero()) {
    // [IEEE Std 754-2008 6.3]:
    // ... the sign of the result of conversions, the quantize operation, the
    // roundToIntegral operations, and the roundToIntegralExact(see 5.3.1) is
    // the sign of the first or only operand.
    প্রত্যাগম opOK;
  }

  // If the exponent is large enough, we know that this value is already
  // integral, and the arithmetic below would potentially cause it to saturate
  // to +/-Inf.  Bail out early instead.
  যদি (exponent + 1 >= (পূর্ণাঙ্ক)APFloat::semanticsPrecision(*semantics))
    প্রত্যাগম opOK;

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
  opStatus fs = MagicConstant.convertFromAPInt(IntegerConstant, অসত্য,
                                               rmNearestTiesToEven);
  প্রতিজ্ঞা(fs == opOK);
  MagicConstant.sign = sign;

  // Preserve the input sign so that we can handle the case of zero result
  // correctly.
  তার্কিক প্রবেশ_Sign = isNegative();

  fs = add(MagicConstant, rounding_mode);

  // Current value and 'MagicConstant' are both integers, so the result of the
  // subtraction is always exact according to Sterbenz' lemma.
  subtract(MagicConstant, rounding_mode);

  // Restore the input sign.
  যদি (প্রবেশ_Sign != isNegative())
    changeSign();

  প্রত্যাগম fs;
}

/* Comparison requires normalized numbers.  */
APFloat::cmpResult IEEEFloat::compare(ধ্রুব IEEEFloat &rhs) ধ্রুব {
  প্রতিজ্ঞা(semantics == rhs.semantics);

  বিকল্প (Pack_Categories_Into_চাবি(category, rhs.category)) {
  স্বভাব:
    llvm_unreachable(nullptr);

  পক্ষ Pack_Categories_Into_চাবি(fcNaN, fcZero):
  পক্ষ Pack_Categories_Into_চাবি(fcNaN, fcNormal):
  পক্ষ Pack_Categories_Into_চাবি(fcNaN, fcInfinity):
  পক্ষ Pack_Categories_Into_চাবি(fcNaN, fcNaN):
  পক্ষ Pack_Categories_Into_চাবি(fcZero, fcNaN):
  পক্ষ Pack_Categories_Into_চাবি(fcNormal, fcNaN):
  পক্ষ Pack_Categories_Into_চাবি(fcInfinity, fcNaN):
    প্রত্যাগম cmpUnordered;

  পক্ষ Pack_Categories_Into_চাবি(fcInfinity, fcNormal):
  পক্ষ Pack_Categories_Into_চাবি(fcInfinity, fcZero):
  পক্ষ Pack_Categories_Into_চাবি(fcNormal, fcZero):
    যদি (sign)
      প্রত্যাগম cmpLessThan;
    অন্যথা
      প্রত্যাগম cmpGreaterThan;

  পক্ষ Pack_Categories_Into_চাবি(fcNormal, fcInfinity):
  পক্ষ Pack_Categories_Into_চাবি(fcZero, fcInfinity):
  পক্ষ Pack_Categories_Into_চাবি(fcZero, fcNormal):
    যদি (rhs.sign)
      প্রত্যাগম cmpGreaterThan;
    অন্যথা
      প্রত্যাগম cmpLessThan;

  পক্ষ Pack_Categories_Into_চাবি(fcInfinity, fcInfinity):
    যদি (sign == rhs.sign)
      প্রত্যাগম cmpEqual;
    অন্যথা যদি (sign)
      প্রত্যাগম cmpLessThan;
    অন্যথা
      প্রত্যাগম cmpGreaterThan;

  পক্ষ Pack_Categories_Into_চাবি(fcZero, fcZero):
    প্রত্যাগম cmpEqual;

  পক্ষ Pack_Categories_Into_চাবি(fcNormal, fcNormal):
    ভঙ্গ;
  }

  cmpResult result;
  /* Two normal numbers.  Do they have the same sign?  */
  যদি (sign != rhs.sign) {
    যদি (sign)
      result = cmpLessThan;
    অন্যথা
      result = cmpGreaterThan;
  } অন্যথা {
    /* Compare absolute values; invert result if negative.  */
    result = compare_Absolute_মান(rhs);

    যদি (sign) {
      যদি (result == cmpLessThan)
        result = cmpGreaterThan;
      অন্যথা যদি (result == cmpGreaterThan)
        result = cmpLessThan;
    }
  }

  প্রত্যাগম result;
}

/// IEEEFloat::convert - convert a value of one floating point type to another.
/// The return value corresponds to the IEEE754 exceptions.  *losesInfo
/// records whether the transformation lost information, i.e. whether
/// converting the result back to the original type will produce the
/// original value (this is almost the same as return value==fsOK, but there
/// are edge cases where this is not so).

APFloat::opStatus IEEEFloat::convert(ধ্রুব fltSemantics &toSemantics,
                                     roundingMode rounding_mode,
                                     তার্কিক *losesInfo) {
  opStatus fs;
  ধ্রুব fltSemantics &fromSemantics = *semantics;
  তার্কিক is_signaling = isSignaling();

  lostFraction lostFraction = lfExactlyZero;
  unsigned new_Part_গুনতি = part_গুনতি_For_Bits(toSemantics.precision + 1);
  unsigned old_Part_গুনতি = part_গুনতি();
  পূর্ণাঙ্ক shift = toSemantics.precision - fromSemantics.precision;

  তার্কিক X86SpecialNan = অসত্য;
  যদি (&fromSemantics == &APFloatBase::semX87DoubleExtended &&
      &toSemantics != &APFloatBase::semX87DoubleExtended && category == fcNaN &&
      (!(*significandParts() & 0x8000000000000000ULL) ||
       !(*significandParts() & 0x4000000000000000ULL))) {
    // x86 has some unusual NaNs which cannot be represented in any other
    // format; note them here.
    X86SpecialNan = সত্য;
  }

  // If this is a truncation of a denormal number, and the target semantics
  // has larger exponent range than the source semantics (this can happen
  // when truncating from PowerPC double-double to double format), the
  // right shift could lose result mantissa bits.  Adjust exponent instead
  // of performing excessive shift.
  // Also do a similar trick in case shifting denormal would produce zero
  // significand as this case isn't handled correctly by normalize.
  যদি (shift < 0 && isFiniteNonZero()) {
    পূর্ণাঙ্ক omsb = significandMSB() + 1;
    পূর্ণাঙ্ক exponentChange = omsb - fromSemantics.precision;
    যদি (exponent + exponentChange < toSemantics.minExponent)
      exponentChange = toSemantics.minExponent - exponent;
    exponentChange = std::max(exponentChange, shift);
    যদি (exponentChange < 0) {
      shift -= exponentChange;
      exponent += exponentChange;
    } অন্যথা যদি (omsb <= -shift) {
      exponentChange = omsb + shift - 1; // leave at least one bit set
      shift -= exponentChange;
      exponent += exponentChange;
    }
  }

  // If this is a truncation, perform the shift before we narrow the storage.
  যদি (shift < 0 && (isFiniteNonZero() ||
                    (category == fcNaN && semantics->nonFiniteBehavior !=
                                              fltNonfiniteBehavior::NanOnly)))
    lostFraction = shift_ডান(significandParts(), old_Part_গুনতি, -shift);

  // Fix the storage so it can hold to new value.
  যদি (new_Part_গুনতি > old_Part_গুনতি) {
    // The new type requires more storage; make it available.
    integerPart *newParts;
    newParts = নূতন integerPart[new_Part_গুনতি];
    APInt::tcSet(newParts, 0, new_Part_গুনতি);
    যদি (isFiniteNonZero() || category==fcNaN)
      APInt::tcAssign(newParts, significandParts(), old_Part_গুনতি);
    freeSignificand();
    significand.parts = newParts;
  } অন্যথা যদি (new_Part_গুনতি == 1 && old_Part_গুনতি != 1) {
    // Switch to built-in storage for a single part.
    integerPart newPart = 0;
    যদি (isFiniteNonZero() || category==fcNaN)
      newPart = significandParts()[0];
    freeSignificand();
    significand.part = newPart;
  }

  // Now that we have the right storage, switch the semantics.
  semantics = &toSemantics;

  // If this is an extension, perform the shift now that the storage is
  // available.
  যদি (shift > 0 && (isFiniteNonZero() || category==fcNaN))
    APInt::tc_Shift_বাম(significandParts(), new_Part_গুনতি, shift);

  যদি (isFiniteNonZero()) {
    fs = normalize(rounding_mode, lostFraction);
    *losesInfo = (fs != opOK);
  } অন্যথা যদি (category == fcNaN) {
    যদি (semantics->nonFiniteBehavior == fltNonfiniteBehavior::NanOnly) {
      *losesInfo =
          fromSemantics.nonFiniteBehavior != fltNonfiniteBehavior::NanOnly;
      makeNaN(অসত্য, sign);
      fs = is_signaling ? opInvalidOp : opOK;
    } অন্যথা {
      // If NaN is negative zero, we need to create a new NaN to avoid
      // converting NaN to -Inf.
      যদি (fromSemantics.nanEncoding == fltNanEncoding::NegativeZero &&
          semantics->nanEncoding != fltNanEncoding::NegativeZero)
        makeNaN(অসত্য, অসত্য);

      // If the source has no significand, there are no payload bits to carry
      // over, and an all-zero significand would encode an Inf. Create a new
      // NaN.
      যদি (!APFloat::hasSignificand(fromSemantics))
        makeNaN(অসত্য, sign);

      *losesInfo = lostFraction != lfExactlyZero || X86SpecialNan;

      // For x87 extended precision, we want to make a NaN, not a special NaN
      // if the input wasn't special either.
      যদি (!X86SpecialNan && semantics == &APFloatBase::semX87DoubleExtended)
        APInt::tcSetBit(significandParts(), semantics->precision - 1);

      // Convert of sNaN creates qNaN and raises an exception (invalid op).
      // This also guarantees that a sNaN does not become Inf on a truncation
      // that loses all payload bits.
      যদি (is_signaling) {
        makeQuiet();
        fs = opInvalidOp;
      } অন্যথা {
        fs = opOK;
      }
    }
  } অন্যথা যদি (category == fcInfinity &&
             semantics->nonFiniteBehavior == fltNonfiniteBehavior::NanOnly) {
    makeNaN(অসত্য, sign);
    *losesInfo = সত্য;
    fs = opInexact;
  } অন্যথা যদি (category == fcZero &&
             semantics->nanEncoding == fltNanEncoding::NegativeZero) {
    // Negative zero loses info, but positive zero doesn't.
    *losesInfo =
        fromSemantics.nanEncoding != fltNanEncoding::NegativeZero && sign;
    fs = *losesInfo ? opInexact : opOK;
    // NaN is negative zero means -0 -> +0, which can lose information
    sign = অসত্য;
  } অন্যথা {
    *losesInfo = অসত্য;
    fs = opOK;
  }

  // The target may have no encoding for a negative value, or none for zero.
  // The paths above only report what rounding lost, so report these here too:
  // a caller that checks losesInfo would otherwise accept a result the target
  // cannot represent, and printing that result asserts.
  যদি ((sign && !semantics->hasSignedRepr) ||
      (category == fcZero && !semantics->hasZero)) {
    *losesInfo = সত্য;
    যদি (fs == opOK)
      fs = opInexact;
  }

  যদি (category == fcZero && !semantics->hasZero)
    makeSmallestNormalized(অসত্য);
  প্রত্যাগম fs;
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
    MutableArrayRef<integerPart> parts, unsigned পূর্ণাঙ্ক width, তার্কিক isSigned,
    roundingMode rounding_mode, তার্কিক *isExact) ধ্রুব {
  *isExact = অসত্য;

  /* Handle the three special cases first.  */
  যদি (category == fcInfinity || category == fcNaN)
    প্রত্যাগম opInvalidOp;

  unsigned dst_Parts_গুনতি = part_গুনতি_For_Bits(width);
  প্রতিজ্ঞা(dst_Parts_গুনতি <= parts.size() && "Integer too big");

  যদি (category == fcZero) {
    APInt::tcSet(parts.data(), 0, dst_Parts_গুনতি);
    // Negative zero can't be represented as an int.
    *isExact = !sign;
    প্রত্যাগম opOK;
  }

  ধ্রুব integerPart *src = significandParts();

  unsigned truncatedBits;
  /* Step 1: place our absolute value, with any fraction truncated, in
     the destination.  */
  যদি (exponent < 0) {
    /* Our absolute value is less than one; truncate everything.  */
    APInt::tcSet(parts.data(), 0, dst_Parts_গুনতি);
    /* For exponent -1 the integer bit represents .5, look at that.
       For smaller exponents leftmost truncated bit is 0. */
    truncatedBits = semantics->precision -1U - exponent;
  } অন্যথা {
    /* We want the most significant (exponent + 1) bits; the rest are
       truncated.  */
    unsigned পূর্ণাঙ্ক bits = exponent + 1U;

    /* Hopelessly large in magnitude?  */
    যদি (bits > width)
      প্রত্যাগম opInvalidOp;

    যদি (bits < semantics->precision) {
      /* We truncate (semantics->precision - bits) bits.  */
      truncatedBits = semantics->precision - bits;
      APInt::tcExtract(parts.data(), dst_Parts_গুনতি, src, bits, truncatedBits);
    } অন্যথা {
      /* We want at least as many bits as are available.  */
      APInt::tcExtract(parts.data(), dst_Parts_গুনতি, src, semantics->precision,
                       0);
      APInt::tc_Shift_বাম(parts.data(), dst_Parts_গুনতি,
                         bits - semantics->precision);
      truncatedBits = 0;
    }
  }

  /* Step 2: work out any lost fraction, and increment the absolute
     value if we would round away from zero.  */
  lostFraction lost_fraction;
  যদি (truncatedBits) {
    lost_fraction = lostFractionThroughTruncation(src, part_গুনতি(),
                                                  truncatedBits);
    যদি (lost_fraction != lfExactlyZero &&
        roundAwayFromZero(rounding_mode, lost_fraction, truncatedBits)) {
      যদি (APInt::tcIncrement(parts.data(), dst_Parts_গুনতি))
        প্রত্যাগম opInvalidOp;     /* Overflow.  */
    }
  } অন্যথা {
    lost_fraction = lfExactlyZero;
  }

  /* Step 3: check if we fit in the destination.  */
  unsigned পূর্ণাঙ্ক omsb = APInt::tcMSB(parts.data(), dst_Parts_গুনতি) + 1;

  যদি (sign) {
    যদি (!isSigned) {
      /* Negative numbers cannot be represented as unsigned.  */
      যদি (omsb != 0)
        প্রত্যাগম opInvalidOp;
    } অন্যথা {
      /* It takes omsb bits to represent the unsigned integer value.
         We lose a bit for the sign, but care is needed as the
         maximally negative integer is a special case.  */
      যদি (omsb == width &&
          APInt::tcLSB(parts.data(), dst_Parts_গুনতি) + 1 != omsb)
        প্রত্যাগম opInvalidOp;

      /* This case can happen because of rounding.  */
      যদি (omsb > width)
        প্রত্যাগম opInvalidOp;
    }

    APInt::tcNegate (parts.data(), dst_Parts_গুনতি);
  } অন্যথা {
    যদি (omsb >= width + !isSigned)
      প্রত্যাগম opInvalidOp;
  }

  যদি (lost_fraction == lfExactlyZero) {
    *isExact = সত্য;
    প্রত্যাগম opOK;
  }
  প্রত্যাগম opInexact;
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
                            unsigned পূর্ণাঙ্ক width, তার্কিক isSigned,
                            roundingMode rounding_mode, তার্কিক *isExact) ধ্রুব {
  opStatus fs = convertToSignExtendedInteger(parts, width, isSigned,
                                             rounding_mode, isExact);

  যদি (fs == opInvalidOp) {
    unsigned পূর্ণাঙ্ক bits, dst_Parts_গুনতি;

    dst_Parts_গুনতি = part_গুনতি_For_Bits(width);
    প্রতিজ্ঞা(dst_Parts_গুনতি <= parts.size() && "Integer too big");

    যদি (category == fcNaN)
      bits = 0;
    অন্যথা যদি (sign)
      bits = isSigned;
    অন্যথা
      bits = width - isSigned;

    tcSetLeastSignificantBits(parts.data(), dst_Parts_গুনতি, bits);
    যদি (sign && isSigned)
      APInt::tc_Shift_বাম(parts.data(), dst_Parts_গুনতি, width - 1);
  }

  প্রত্যাগম fs;
}

/* Convert an unsigned integer SRC to a floating point number,
   rounding according to ROUNDING_MODE.  The sign of the floating
   point number is not modified.  */
APFloat::opStatus IEEEFloat::convertFromUnsignedParts(
    ধ্রুব integerPart *src, unsigned পূর্ণাঙ্ক src_গুনতি, roundingMode rounding_mode) {
  category = fcNormal;
  unsigned omsb = APInt::tcMSB(src, src_গুনতি) + 1;
  integerPart *dst = significandParts();
  unsigned dst_গুনতি = part_গুনতি();
  unsigned precision = semantics->precision;

  /* We want the most significant PRECISION bits of SRC.  There may not
     be that many; extract what we can.  */
  lostFraction lost_fraction;
  যদি (precision <= omsb) {
    exponent = omsb - 1;
    lost_fraction = lostFractionThroughTruncation(src, src_গুনতি,
                                                  omsb - precision);
    APInt::tcExtract(dst, dst_গুনতি, src, precision, omsb - precision);
  } অন্যথা {
    exponent = precision - 1;
    lost_fraction = lfExactlyZero;
    APInt::tcExtract(dst, dst_গুনতি, src, omsb, 0);
  }

  প্রত্যাগম normalize(rounding_mode, lost_fraction);
}

APFloat::opStatus IEEEFloat::convertFromAPInt(ধ্রুব APInt &Val, তার্কিক isSigned,
                                              roundingMode rounding_mode) {
  unsigned পূর্ণাঙ্ক part_গুনতি = Val.get_অঙ্ক_শব্দগুলি();
  APInt api = Val;

  sign = অসত্য;
  যদি (isSigned && api.isNegative()) {
    sign = সত্য;
    api = -api;
  }

  প্রত্যাগম convertFromUnsignedParts(api.getRawData(), part_গুনতি, rounding_mode);
}

Expected<APFloat::opStatus>
IEEEFloat::convertFromHexadecimalString(StringRef s,
                                        roundingMode rounding_mode) {
  lostFraction lost_fraction = lfExactlyZero;

  category = fcNormal;
  zeroSignificand();
  exponent = 0;

  integerPart *significand = significandParts();
  unsigned parts_গুনতি = part_গুনতি();
  unsigned bitPos = parts_গুনতি * integerPartWidth;
  তার্কিক computedTrailingFraction = অসত্য;

  // Skip leading zeroes and any (hexa)decimal point.
  StringRef::iterator begin = s.begin();
  StringRef::iterator end = s.end();
  StringRef::iterator dot;
  auto PtrOrErr = skipLeadingZeroesAndAnyDot(begin, end, &dot);
  যদি (!PtrOrErr)
    প্রত্যাগম PtrOrErr.takeError();
  StringRef::iterator p = *PtrOrErr;
  StringRef::iterator firstSignificantDigit = p;

  যাবত্ (p != end) {
    integerPart hex_মান;

    যদি (*p == '.') {
      যদি (dot != end)
        প্রত্যাগম createError("String contains multiple dots");
      dot = p++;
      অনুবর্ত;
    }

    hex_মান = hex_Digit_মান(*p);
    যদি (hex_মান == UINT_MAX)
      ভঙ্গ;

    p++;

    // Store the number while we have space.
    যদি (bitPos) {
      bitPos -= 4;
      hex_মান <<= bitPos % integerPartWidth;
      significand[bitPos / integerPartWidth] |= hex_মান;
    } অন্যথা যদি (!computedTrailingFraction) {
      auto FractOrErr = trailingHexadecimalFraction(p, end, hex_মান);
      যদি (!FractOrErr)
        প্রত্যাগম FractOrErr.takeError();
      lost_fraction = *FractOrErr;
      computedTrailingFraction = সত্য;
    }
  }

  /* Hex floats require an exponent but not a hexadecimal point.  */
  যদি (p == end)
    প্রত্যাগম createError("Hex strings require an exponent");
  যদি (*p != 'p' && *p != 'P')
    প্রত্যাগম createError("Invalid character in significand");
  যদি (p == begin)
    প্রত্যাগম createError("Significand has no digits");
  যদি (dot != end && p - begin == 1)
    প্রত্যাগম createError("Significand has no digits");

  /* Ignore the exponent if we are zero.  */
  যদি (p != firstSignificantDigit) {
    পূর্ণাঙ্ক expAdjustment;

    /* Implicit hexadecimal point?  */
    যদি (dot == end)
      dot = p;

    /* Calculate the exponent adjustment implicit in the number of
       significant digits.  */
    expAdjustment = static_cast<পূর্ণাঙ্ক>(dot - firstSignificantDigit);
    যদি (expAdjustment < 0)
      expAdjustment++;
    expAdjustment = expAdjustment * 4 - 1;

    /* Adjust for writing the significand starting at the most
       significant nibble.  */
    expAdjustment += semantics->precision;
    expAdjustment -= parts_গুনতি * integerPartWidth;

    /* Adjust for the given exponent.  */
    auto ExpOrErr = totalExponent(p + 1, end, expAdjustment);
    যদি (!ExpOrErr)
      প্রত্যাগম ExpOrErr.takeError();
    exponent = *ExpOrErr;
  }

  প্রত্যাগম normalize(rounding_mode, lost_fraction);
}

APFloat::opStatus
IEEEFloat::roundSignificandWithExponent(ধ্রুব integerPart *decSigParts,
                                        unsigned sig_Part_গুনতি, পূর্ণাঙ্ক exp,
                                        roundingMode rounding_mode) {
  fltSemantics calcSemantics = { 32767, -32767, 0, 0 };
  unsigned পূর্ণাঙ্ক power = exp >= 0 ? exp : -exp;
  unsigned পূর্ণাঙ্ক pow5PartCapacity = powerOfFivePartsForExponent(power);
  SmallVector<integerPart, অঙ্ক_Pow5_Parts_On_স্তূপ> pow5Parts;
  pow5Parts.resize_for_overwrite(pow5PartCapacity);

  তার্কিক isNearest = rounding_mode == rmNearestTiesToEven ||
                   rounding_mode == rmNearestTiesToAway;

  unsigned parts = part_গুনতি_For_Bits(semantics->precision + 11);

  /* Calculate pow(5, abs(exp)).  */
  unsigned pow5_Part_গুনতি = powerOf5(pow5Parts.data(), power);

  প্রতি (;; parts *= 2) {
    unsigned পূর্ণাঙ্ক excessPrecision, truncatedBits;

    calcSemantics.precision = parts * integerPartWidth - 1;
    excessPrecision = calcSemantics.precision - semantics->precision;
    truncatedBits = excessPrecision;

    IEEEFloat decSig(calcSemantics, uninitialized);
    decSig.makeZero(sign);
    IEEEFloat pow5(calcSemantics);

    opStatus sigStatus = decSig.convertFromUnsignedParts(
        decSigParts, sig_Part_গুনতি, rmNearestTiesToEven);
    opStatus powStatus = pow5.convertFromUnsignedParts(
        pow5Parts.data(), pow5_Part_গুনতি, rmNearestTiesToEven);
    /* Add exp, as 10^n = 5^n * 2^n.  */
    decSig.exponent += exp;

    lostFraction calcLostFraction;
    integerPart HUerr, HUdistance;
    unsigned পূর্ণাঙ্ক powHUerr;

    যদি (exp >= 0) {
      /* multiplySignificand leaves the precision-th bit set to 1.  */
      calcLostFraction = decSig.multiplySignificand(pow5);
      powHUerr = powStatus != opOK;
    } অন্যথা {
      calcLostFraction = decSig.divideSignificand(pow5);
      /* Denormal numbers have less precision.  */
      যদি (decSig.exponent < semantics->minExponent) {
        excessPrecision += (semantics->minExponent - decSig.exponent);
        truncatedBits = excessPrecision;
        excessPrecision = std::min(excessPrecision, calcSemantics.precision);
      }
      /* Extra half-ulp lost in reciprocal of exponent.  */
      powHUerr = (powStatus == opOK && calcLostFraction == lfExactlyZero) ? 0:2;
    }

    /* Both multiplySignificand and divideSignificand return the
       result with the integer bit set.  */
    প্রতিজ্ঞা(APInt::tcExtractBit
           (decSig.significandParts(), calcSemantics.precision - 1) == 1);

    HUerr = HUerrBound(calcLostFraction != lfExactlyZero, sigStatus != opOK,
                       powHUerr);
    HUdistance = 2 * ulpsFromBoundary(decSig.significandParts(),
                                      excessPrecision, isNearest);

    /* Are we guaranteed to round correctly if we truncate?  */
    যদি (HUdistance >= HUerr) {
      APInt::tcExtract(significandParts(), part_গুনতি(), decSig.significandParts(),
                       calcSemantics.precision - excessPrecision,
                       excessPrecision);
      /* Take the exponent of decSig.  If we tcExtract-ed less bits
         above we must adjust our exponent to compensate for the
         implicit right shift.  */
      exponent = (decSig.exponent + semantics->precision
                  - (calcSemantics.precision - excessPrecision));
      calcLostFraction = lostFractionThroughTruncation(decSig.significandParts(),
                                                       decSig.part_গুনতি(),
                                                       truncatedBits);
      প্রত্যাগম static_cast<opStatus>(normalize(rounding_mode, calcLostFraction) |
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
  যদি (Error Err = interpretDecimal(p, str.end(), &D))
    প্রত্যাগম std::move(Err);

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
  যদি (D.firstSigDigit == str.end() || dec_Digit_মান(*D.firstSigDigit) >= 10U) {
    category = fcZero;
    fs = opOK;
    যদি (semantics->nanEncoding == fltNanEncoding::NegativeZero)
      sign = অসত্য;
    যদি (!semantics->hasZero)
      makeSmallestNormalized(অসত্য);

    /* Check whether the normalized exponent is high enough to overflow
       max during the log-rebasing in the max-exponent check below. */
  } অন্যথা যদি (D.normalizedExponent - 1 > INT_MAX / 42039) {
    fs = handleOverflow(rounding_mode);

  /* If it wasn't, then it also wasn't high enough to overflow max
     during the log-rebasing in the min-exponent check.  Check that it
     won't overflow min in either check, then perform the min-exponent
     check. */
  } অন্যথা যদি (D.normalizedExponent - 1 < INT_MIN / 42039 ||
             (D.normalizedExponent + 1) * 28738 <=
               8651 * (semantics->minExponent - (পূর্ণাঙ্ক) semantics->precision)) {
    /* Underflow to zero and round.  */
    category = fcNormal;
    zeroSignificand();
    fs = normalize(rounding_mode, lfLessThanHalf);

  /* We can finally safely perform the max-exponent check. */
  } অন্যথা যদি ((D.normalizedExponent - 1) * 42039
             >= 12655 * semantics->maxExponent) {
    /* Overflow and round.  */
    fs = handleOverflow(rounding_mode);
  } অন্যথা {
    integerPart *decSignificand;
    unsigned পূর্ণাঙ্ক part_গুনতি;

    /* A tight upper bound on number of bits required to hold an
       N-digit decimal integer is N * 196 / 59.  Allocate enough space
       to hold the full significand, and an extra part required by
       tcMultiplyPart.  */
    part_গুনতি = static_cast<unsigned পূর্ণাঙ্ক>(D.lastSigDigit - D.firstSigDigit) + 1;
    part_গুনতি = part_গুনতি_For_Bits(1 + 196 * part_গুনতি / 59);
    decSignificand = নূতন integerPart[part_গুনতি + 1];
    part_গুনতি = 0;

    /* Convert to binary efficiently - we do almost all multiplication
       in an integerPart.  When this would overflow do we do a single
       bignum multiplication, and then revert again to multiplication
       in an integerPart.  */
    কুরু {
      integerPart dec_মান, val, multiplier;

      val = 0;
      multiplier = 1;

      কুরু {
        যদি (*p == '.') {
          p++;
          যদি (p == str.end()) {
            ভঙ্গ;
          }
        }
        dec_মান = dec_Digit_মান(*p++);
        যদি (dec_মান >= 10U) {
          delete[] decSignificand;
          প্রত্যাগম createError("Invalid character in significand");
        }
        multiplier *= 10;
        val = val * 10 + dec_মান;
        /* The maximum number that can be multiplied by ten with any
           digit added without overflowing an integerPart.  */
      } যাবত্ (p <= D.lastSigDigit && multiplier <= (~ (integerPart) 0 - 9) / 10);

      /* Multiply out the current part.  */
      APInt::tcMultiplyPart(decSignificand, decSignificand, multiplier, val,
                            part_গুনতি, part_গুনতি + 1, অসত্য);

      /* If we used another part (likely but not guaranteed), increase
         the count.  */
      যদি (decSignificand[part_গুনতি])
        part_গুনতি++;
    } যাবত্ (p <= D.lastSigDigit);

    category = fcNormal;
    fs = roundSignificandWithExponent(decSignificand, part_গুনতি,
                                      D.exponent, rounding_mode);

    delete [] decSignificand;
  }

  প্রত্যাগম fs;
}

তার্কিক IEEEFloat::convertFromStringSpecials(StringRef str) {
  ধ্রুব size_t MIN_NAME_SIZE = 3;

  যদি (str.size() < MIN_NAME_SIZE)
    প্রত্যাগম অসত্য;

  যদি (str == "inf" || str == "INFINITY" || str == "+Inf" || str == "+inf") {
    makeInf(অসত্য);
    প্রত্যাগম সত্য;
  }

  তার্কিক IsNegative = str.consume_front("-");
  যদি (IsNegative) {
    যদি (str.size() < MIN_NAME_SIZE)
      প্রত্যাগম অসত্য;

    যদি (str == "inf" || str == "INFINITY" || str == "Inf") {
      makeInf(সত্য);
      প্রত্যাগম সত্য;
    }
  }

  // If we have a 's' (or 'S') prefix, then this is a Signaling NaN.
  তার্কিক IsSignaling = str.consume_front_insensitive("s");
  যদি (IsSignaling) {
    যদি (str.size() < MIN_NAME_SIZE)
      প্রত্যাগম অসত্য;
  }

  যদি (str.consume_front("nan") || str.consume_front("NaN")) {
    // A NaN without payload.
    যদি (str.empty()) {
      makeNaN(IsSignaling, IsNegative);
      প্রত্যাগম সত্য;
    }

    // Allow the payload to be inside parentheses.
    যদি (str.front() == '(') {
      // Parentheses should be balanced (and not empty).
      যদি (str.size() <= 2 || str.back() != ')')
        প্রত্যাগম অসত্য;

      str = str.slice(1, str.size() - 1);
    }

    // Determine the payload number's radix.
    unsigned Radix = 10;
    যদি (str[0] == '0') {
      যদি (str.size() > 1 && tolower(str[1]) == 'x') {
        str = str.drop_front(2);
        Radix = 16;
      } অন্যথা {
        Radix = 8;
      }
    }

    // Parse the payload and make the NaN.
    APInt Payload;
    যদি (!str.getAsInteger(Radix, Payload)) {
      makeNaN(IsSignaling, IsNegative, &Payload);
      প্রত্যাগম সত্য;
    }
  }

  প্রত্যাগম অসত্য;
}

Expected<APFloat::opStatus>
IEEEFloat::convertFromString(StringRef str, roundingMode rounding_mode) {
  যদি (str.empty())
    প্রত্যাগম createError("Invalid string length");

  // Handle special cases.
  যদি (convertFromStringSpecials(str))
    প্রত্যাগম opOK;

  /* Handle a leading minus sign.  */
  StringRef::iterator p = str.begin();
  size_t slen = str.size();
  sign = *p == '-' ? 1 : 0;
  যদি (sign && !semantics->hasSignedRepr)
    llvm_unreachable(
        "This floating point format does not support signed values");

  যদি (*p == '-' || *p == '+') {
    p++;
    slen--;
    যদি (!slen)
      প্রত্যাগম createError("String has no digits");
  }

  যদি (slen >= 2 && p[0] == '0' && (p[1] == 'x' || p[1] == 'X')) {
    যদি (slen == 2)
      প্রত্যাগম createError("Invalid string");
    প্রত্যাগম convertFromHexadecimalString(StringRef(p + 2, slen - 2),
                                        rounding_mode);
  }

  প্রত্যাগম convertFromDecimalString(StringRef(p, slen), rounding_mode);
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
unsigned পূর্ণাঙ্ক IEEEFloat::convertToHexString(বর্ণ *dst, unsigned পূর্ণাঙ্ক hexDigits,
                                           তার্কিক upperCase,
                                           roundingMode rounding_mode) ধ্রুব {
  বর্ণ *p = dst;
  যদি (sign)
    *dst++ = '-';

  বিকল্প (category) {
  পক্ষ fcInfinity:
    memcpy (dst, upperCase ? infinityU: infinityL, পরিমাণ infinityU - 1);
    dst += পরিমাণ infinityL - 1;
    ভঙ্গ;

  পক্ষ fcNaN:
    memcpy (dst, upperCase ? NaNU: NaNL, পরিমাণ NaNU - 1);
    dst += পরিমাণ NaNU - 1;
    ভঙ্গ;

  পক্ষ fcZero:
    *dst++ = '0';
    *dst++ = upperCase ? 'X': 'x';
    *dst++ = '0';
    যদি (hexDigits > 1) {
      *dst++ = '.';
      memset (dst, '0', hexDigits - 1);
      dst += hexDigits - 1;
    }
    *dst++ = upperCase ? 'P': 'p';
    *dst++ = '0';
    ভঙ্গ;

  পক্ষ fcNormal:
    dst = convertNormalToHexString (dst, hexDigits, upperCase, rounding_mode);
    ভঙ্গ;
  }

  *dst = 0;

  প্রত্যাগম static_cast<unsigned পূর্ণাঙ্ক>(dst - p);
}

/* Does the hard work of outputting the correctly rounded hexadecimal
   form of a normal floating point number with the specified number of
   hexadecimal digits.  If HEXDIGITS is zero the minimum number of
   digits necessary to print the value precisely is output.  */
বর্ণ *IEEEFloat::convertNormalToHexString(বর্ণ *dst, unsigned পূর্ণাঙ্ক hexDigits,
                                          তার্কিক upperCase,
                                          roundingMode rounding_mode) ধ্রুব {
  *dst++ = '0';
  *dst++ = upperCase ? 'X': 'x';

  তার্কিক roundUp = অসত্য;
  ধ্রুব বর্ণ *hexDigitChars = upperCase ? hexDigitsUpper : hexDigitsLower;

  ধ্রুব integerPart *significand = significandParts();
  unsigned parts_গুনতি = part_গুনতি();

  /* +3 because the first digit only uses the single integer bit, so
     we have 3 virtual zero most-significant-bits.  */
  unsigned মান_Bits = semantics->precision + 3;
  unsigned shift = integerPartWidth - মান_Bits % integerPartWidth;

  /* The natural number of digits required ignoring trailing
     insignificant zeroes.  */
  unsigned outputDigits = (মান_Bits - significandLSB() + 3) / 4;

  /* hexDigits of zero means use the required number for the
     precision.  Otherwise, see if we are truncating.  If we are,
     find out if we need to round away from zero.  */
  যদি (hexDigits) {
    যদি (hexDigits < outputDigits) {
      /* We are dropping non-zero bits, so need to check how to round.
         "bits" is the number of dropped bits.  */
      unsigned পূর্ণাঙ্ক bits;
      lostFraction fraction;

      bits = মান_Bits - hexDigits * 4;
      fraction = lostFractionThroughTruncation (significand, parts_গুনতি, bits);
      roundUp = roundAwayFromZero(rounding_mode, fraction, bits);
    }
    outputDigits = hexDigits;
  }

  /* Write the digits consecutively, and start writing in the location
     of the hexadecimal point.  We move the most significant digit
     left and add the hexadecimal point later.  */
  বর্ণ *p = ++dst;

  unsigned গুনতি = (মান_Bits + integerPartWidth - 1) / integerPartWidth;

  যাবত্ (outputDigits && গুনতি) {
    integerPart part;

    /* Put the most significant integerPartWidth bits in "part".  */
    যদি (--গুনতি == parts_গুনতি)
      part = 0;  /* An imaginary higher zero part.  */
    অন্যথা
      part = significand[গুনতি] << shift;

    যদি (গুনতি && shift)
      part |= significand[গুনতি - 1] >> (integerPartWidth - shift);

    /* Convert as much of "part" to hexdigits as we can.  */
    unsigned পূর্ণাঙ্ক curDigits = integerPartWidth / 4;

    curDigits = std::min(curDigits, outputDigits);
    dst += partAsHex (dst, part, curDigits, hexDigitChars);
    outputDigits -= curDigits;
  }

  যদি (roundUp) {
    বর্ণ *q = dst;

    /* Note that hexDigitChars has a trailing '0'.  */
    কুরু {
      q--;
      *q = hexDigitChars[hex_Digit_মান (*q) + 1];
    } যাবত্ (*q == '0');
    প্রতিজ্ঞা(q >= p);
  } অন্যথা {
    /* Add trailing zeroes.  */
    memset (dst, '0', outputDigits);
    dst += outputDigits;
  }

  /* Move the most significant digit to before the point, and if there
     is something after the decimal point add it.  This must come
     after rounding above.  */
  p[-1] = p[0];
  যদি (dst -1 == p)
    dst--;
  অন্যথা
    p[0] = '.';

  /* Finally output the exponent.  */
  *dst++ = upperCase ? 'P': 'p';

  প্রত্যাগম writeSignedDecimal (dst, exponent);
}

hash_code hash_মান(ধ্রুব IEEEFloat &Arg) {
  যদি (!Arg.isFiniteNonZero())
    প্রত্যাগম hash_combine((uint8_t)Arg.category,
                        // NaN has no sign, fix it at zero.
                        Arg.isNaN() ? (uint8_t)0 : (uint8_t)Arg.sign,
                        Arg.semantics->precision);

  // Normal floats need their exponent and significand hashed.
  প্রত্যাগম hash_combine((uint8_t)Arg.category, (uint8_t)Arg.sign,
                      Arg.semantics->precision, Arg.exponent,
                      hash_combine_range(
                        Arg.significandParts(),
                        Arg.significandParts() + Arg.part_গুনতি()));
}

// Conversion from APFloat to/from host float/double.  It may eventually be
// possible to eliminate these and have everybody deal with APFloats, but that
// will take a while.  This approach will not easily extend to long double.
// Current implementation requires integerPartWidth==64, which is correct at
// the moment but could be made more general.

// Denormals have exponent minExponent in APFloat, but minExponent-1 in
// the actual IEEE respresentations.  We compensate for that here.

APInt IEEEFloat::convertF80LongDoubleAPFloatToAPInt() ধ্রুব {
  প্রতিজ্ঞা(part_গুনতি() == 2);
  প্রত্যাগম convertIEEEFloatToAPInt<APFloatBase::semX87DoubleExtended>();
}

APInt IEEEFloat::convertPPCDoubleDoubleLegacyAPFloatToAPInt() ধ্রুব {
  প্রতিজ্ঞা(semantics ==
         (ধ্রুব llvm::fltSemantics *)&APFloatBase::semPPCDoubleDoubleLegacy);
  প্রতিজ্ঞা(part_গুনতি()==2);

  uint64_t শব্দগুলি[2];
  তার্কিক losesInfo;

  // Convert number to double.  To avoid spurious underflows, we re-
  // normalize against the "double" minExponent first, and only *then*
  // truncate the mantissa.  The result of that second conversion
  // may be inexact, but should never underflow.
  // Declare fltSemantics before APFloat that uses it (and
  // saves pointer to it) to ensure correct destruction order.
  fltSemantics extendedSemantics = *semantics;
  extendedSemantics.minExponent = APFloatBase::semIEEEdouble.minExponent;
  IEEEFloat extended(*এতত্);
  [[maybe_unused]] opStatus fs =
      extended.convert(extendedSemantics, rmNearestTiesToEven, &losesInfo);
  প্রতিজ্ঞা(fs == opOK && !losesInfo);

  IEEEFloat u(extended);
  fs = u.convert(APFloatBase::semIEEEdouble, rmNearestTiesToEven, &losesInfo);
  প্রতিজ্ঞা(fs == opOK || fs == opInexact);
  শব্দগুলি[0] = *u.convertDoubleAPFloatToAPInt().getRawData();

  // If conversion was exact or resulted in a special case, we're done;
  // just set the second double to zero.  Otherwise, re-convert back to
  // the extended format and compute the difference.  This now should
  // convert exactly to double.
  যদি (u.isFiniteNonZero() && losesInfo) {
    fs = u.convert(extendedSemantics, rmNearestTiesToEven, &losesInfo);
    প্রতিজ্ঞা(fs == opOK && !losesInfo);

    IEEEFloat v(extended);
    v.subtract(u, rmNearestTiesToEven);
    fs = v.convert(APFloatBase::semIEEEdouble, rmNearestTiesToEven, &losesInfo);
    প্রতিজ্ঞা(fs == opOK && !losesInfo);
    শব্দগুলি[1] = *v.convertDoubleAPFloatToAPInt().getRawData();
  } অন্যথা {
    শব্দগুলি[1] = 0;
  }

  প্রত্যাগম APInt(128, শব্দগুলি);
}

template <ধ্রুব fltSemantics &S>
APInt IEEEFloat::convertIEEEFloatToAPInt() ধ্রুব {
  প্রতিজ্ঞা(semantics == &S);
  constexpr unsigned পূর্ণাঙ্ক trailing_significand_bits =
      S.precision - 1 + S.hasExplicitIntegerBit;
  constexpr পূর্ণাঙ্ক integer_bit_part = (S.precision - 1) / integerPartWidth;
  constexpr integerPart integer_bit = integerPart{1}
                                      << ((S.precision - 1) % integerPartWidth);
  constexpr uint64_t significand_mask = integer_bit - 1;
  constexpr unsigned পূর্ণাঙ্ক exponent_bits =
      S.sizeInBits - (S.hasSignedRepr ? 1 : 0) - trailing_significand_bits;
  static_assert(exponent_bits < 64);
  constexpr uint64_t exponent_mask = (uint64_t{1} << exponent_bits) - 1;
  constexpr তার্কিক is_zero_exp_reserved = S.hasDenormals || S.hasZero;
  constexpr পূর্ণাঙ্ক bias = -(S.minExponent - (is_zero_exp_reserved ? 1 : 0));

  uint64_t myexponent;
  std::array<integerPart, part_গুনতি_For_Bits(trailing_significand_bits)>
      mysignificand;

  যদি (isFiniteNonZero()) {
    myexponent = exponent + bias;
    std::copy_n(significandParts(), mysignificand.size(),
                mysignificand.begin());
    যদি (myexponent == 1 &&
        !(significandParts()[integer_bit_part] & integer_bit))
      myexponent = 0; // denormal
  } অন্যথা যদি (category == fcZero) {
    যদি (!S.hasZero)
      llvm_unreachable("semantics does not support zero!");
    myexponent = ::exponentZero(S) + bias;
    mysignificand.fill(0);
  } অন্যথা যদি (category == fcInfinity) {
    যদি (S.nonFiniteBehavior == fltNonfiniteBehavior::NanOnly ||
        S.nonFiniteBehavior == fltNonfiniteBehavior::FiniteOnly)
      llvm_unreachable("semantics don't support inf!");
    myexponent = ::exponentInf(S) + bias;
    mysignificand.fill(0);
    যদি constexpr (S.hasExplicitIntegerBit) {
      mysignificand[0] = integerPart{1} << (trailing_significand_bits - 1);
    }
  } অন্যথা {
    প্রতিজ্ঞা(category == fcNaN && "Unknown category!");
    যদি (S.nonFiniteBehavior == fltNonfiniteBehavior::FiniteOnly)
      llvm_unreachable("semantics don't support NaN!");
    myexponent = ::exponentNaN(S) + bias;
    std::copy_n(significandParts(), mysignificand.size(),
                mysignificand.begin());
  }
  std::array<uint64_t, (S.sizeInBits + 63) / 64> শব্দগুলি;
  auto শব্দগুলি_iter =
      std::copy_n(mysignificand.begin(), mysignificand.size(), শব্দগুলি.begin());
  যদি constexpr (!S.hasExplicitIntegerBit) {
    যদি constexpr (significand_mask != 0 || trailing_significand_bits == 0) {
      // Clear the integer bit.
      শব্দগুলি[mysignificand.size() - 1] &= significand_mask;
    }
  }
  std::fill(শব্দগুলি_iter, শব্দগুলি.end(), uint64_t{0});
  constexpr size_t last_word = শব্দগুলি.size() - 1;
  যদি constexpr (S.hasSignedRepr) {
    uint64_t shifted_sign = static_cast<uint64_t>(sign & 1)
                            << ((S.sizeInBits - 1) % 64);
    শব্দগুলি[last_word] |= shifted_sign;
  }
  uint64_t shifted_exponent = (myexponent & exponent_mask)
                              << (trailing_significand_bits % 64);
  শব্দগুলি[last_word] |= shifted_exponent;
  যদি constexpr (last_word == 0) {
    প্রত্যাগম APInt(S.sizeInBits, শব্দগুলি[0]);
  }
  প্রত্যাগম APInt(S.sizeInBits, শব্দগুলি);
}

APInt IEEEFloat::convertQuadrupleAPFloatToAPInt() ধ্রুব {
  প্রতিজ্ঞা(part_গুনতি() == 2);
  প্রত্যাগম convertIEEEFloatToAPInt<APFloatBase::semIEEEquad>();
}

APInt IEEEFloat::convertDoubleAPFloatToAPInt() ধ্রুব {
  প্রতিজ্ঞা(part_গুনতি()==1);
  প্রত্যাগম convertIEEEFloatToAPInt<APFloatBase::semIEEEdouble>();
}

APInt IEEEFloat::convertFloatAPFloatToAPInt() ধ্রুব {
  প্রতিজ্ঞা(part_গুনতি()==1);
  প্রত্যাগম convertIEEEFloatToAPInt<APFloatBase::semIEEEsingle>();
}

APInt IEEEFloat::convertBFloatAPFloatToAPInt() ধ্রুব {
  প্রতিজ্ঞা(part_গুনতি() == 1);
  প্রত্যাগম convertIEEEFloatToAPInt<APFloatBase::semBFloat>();
}

APInt IEEEFloat::convertHalfAPFloatToAPInt() ধ্রুব {
  প্রতিজ্ঞা(part_গুনতি()==1);
  প্রত্যাগম convertIEEEFloatToAPInt<APFloatBase::APFloatBase::semIEEEhalf>();
}

APInt IEEEFloat::convertFloat8E5M2APFloatToAPInt() ধ্রুব {
  প্রতিজ্ঞা(part_গুনতি() == 1);
  প্রত্যাগম convertIEEEFloatToAPInt<APFloatBase::semFloat8E5M2>();
}

APInt IEEEFloat::convertFloat8E5M2FNUZAPFloatToAPInt() ধ্রুব {
  প্রতিজ্ঞা(part_গুনতি() == 1);
  প্রত্যাগম convertIEEEFloatToAPInt<APFloatBase::semFloat8E5M2FNUZ>();
}

APInt IEEEFloat::convertFloat8E4M3APFloatToAPInt() ধ্রুব {
  প্রতিজ্ঞা(part_গুনতি() == 1);
  প্রত্যাগম convertIEEEFloatToAPInt<APFloatBase::semFloat8E4M3>();
}

APInt IEEEFloat::convertFloat8E4M3FNAPFloatToAPInt() ধ্রুব {
  প্রতিজ্ঞা(part_গুনতি() == 1);
  প্রত্যাগম convertIEEEFloatToAPInt<APFloatBase::semFloat8E4M3FN>();
}

APInt IEEEFloat::convertFloat8E4M3FNUZAPFloatToAPInt() ধ্রুব {
  প্রতিজ্ঞা(part_গুনতি() == 1);
  প্রত্যাগম convertIEEEFloatToAPInt<APFloatBase::semFloat8E4M3FNUZ>();
}

APInt IEEEFloat::convertFloat8E4M3B11FNUZAPFloatToAPInt() ধ্রুব {
  প্রতিজ্ঞা(part_গুনতি() == 1);
  প্রত্যাগম convertIEEEFloatToAPInt<APFloatBase::semFloat8E4M3B11FNUZ>();
}

APInt IEEEFloat::convertFloat8E3M4APFloatToAPInt() ধ্রুব {
  প্রতিজ্ঞা(part_গুনতি() == 1);
  প্রত্যাগম convertIEEEFloatToAPInt<APFloatBase::semFloat8E3M4>();
}

APInt IEEEFloat::convertFloatTF32APFloatToAPInt() ধ্রুব {
  প্রতিজ্ঞা(part_গুনতি() == 1);
  প্রত্যাগম convertIEEEFloatToAPInt<APFloatBase::semFloatTF32>();
}

APInt IEEEFloat::convertFloat8E8M0FNUAPFloatToAPInt() ধ্রুব {
  প্রতিজ্ঞা(part_গুনতি() == 1);
  প্রত্যাগম convertIEEEFloatToAPInt<APFloatBase::semFloat8E8M0FNU>();
}

APInt IEEEFloat::convertFloat8E5M3FNUAPFloatToAPInt() ধ্রুব {
  প্রতিজ্ঞা(part_গুনতি() == 1);
  প্রত্যাগম convertIEEEFloatToAPInt<APFloatBase::semFloat8E5M3FNU>();
}

APInt IEEEFloat::convertFloat6E3M2FNAPFloatToAPInt() ধ্রুব {
  প্রতিজ্ঞা(part_গুনতি() == 1);
  প্রত্যাগম convertIEEEFloatToAPInt<APFloatBase::semFloat6E3M2FN>();
}

APInt IEEEFloat::convertFloat6E2M3FNAPFloatToAPInt() ধ্রুব {
  প্রতিজ্ঞা(part_গুনতি() == 1);
  প্রত্যাগম convertIEEEFloatToAPInt<APFloatBase::semFloat6E2M3FN>();
}

APInt IEEEFloat::convertFloat4E2M1FNAPFloatToAPInt() ধ্রুব {
  প্রতিজ্ঞা(part_গুনতি() == 1);
  প্রত্যাগম convertIEEEFloatToAPInt<APFloatBase::semFloat4E2M1FN>();
}

// This function creates an APInt that is just a bit map of the floating
// point constant as it would appear in memory.  It is not a conversion,
// and treating the result as a normal integer is unlikely to be useful.

APInt IEEEFloat::bitcastToAPInt() ধ্রুব {
  যদি (semantics == (ধ্রুব llvm::fltSemantics *)&APFloatBase::semIEEEhalf)
    প্রত্যাগম convertHalfAPFloatToAPInt();

  যদি (semantics == (ধ্রুব llvm::fltSemantics *)&APFloatBase::semBFloat)
    প্রত্যাগম convertBFloatAPFloatToAPInt();

  যদি (semantics == (ধ্রুব llvm::fltSemantics *)&APFloatBase::semIEEEsingle)
    প্রত্যাগম convertFloatAPFloatToAPInt();

  যদি (semantics == (ধ্রুব llvm::fltSemantics *)&APFloatBase::semIEEEdouble)
    প্রত্যাগম convertDoubleAPFloatToAPInt();

  যদি (semantics == (ধ্রুব llvm::fltSemantics *)&APFloatBase::semIEEEquad)
    প্রত্যাগম convertQuadrupleAPFloatToAPInt();

  যদি (semantics ==
      (ধ্রুব llvm::fltSemantics *)&APFloatBase::semX87DoubleExtended)
    প্রত্যাগম convertF80LongDoubleAPFloatToAPInt();

  যদি (semantics ==
      (ধ্রুব llvm::fltSemantics *)&APFloatBase::semPPCDoubleDoubleLegacy)
    প্রত্যাগম convertPPCDoubleDoubleLegacyAPFloatToAPInt();

  যদি (semantics == (ধ্রুব llvm::fltSemantics *)&APFloatBase::semFloat8E5M2)
    প্রত্যাগম convertFloat8E5M2APFloatToAPInt();

  যদি (semantics == (ধ্রুব llvm::fltSemantics *)&APFloatBase::semFloat8E5M2FNUZ)
    প্রত্যাগম convertFloat8E5M2FNUZAPFloatToAPInt();

  যদি (semantics == (ধ্রুব llvm::fltSemantics *)&APFloatBase::semFloat8E4M3)
    প্রত্যাগম convertFloat8E4M3APFloatToAPInt();

  যদি (semantics == (ধ্রুব llvm::fltSemantics *)&APFloatBase::semFloat8E4M3FN)
    প্রত্যাগম convertFloat8E4M3FNAPFloatToAPInt();

  যদি (semantics == (ধ্রুব llvm::fltSemantics *)&APFloatBase::semFloat8E4M3FNUZ)
    প্রত্যাগম convertFloat8E4M3FNUZAPFloatToAPInt();

  যদি (semantics ==
      (ধ্রুব llvm::fltSemantics *)&APFloatBase::semFloat8E4M3B11FNUZ)
    প্রত্যাগম convertFloat8E4M3B11FNUZAPFloatToAPInt();

  যদি (semantics == (ধ্রুব llvm::fltSemantics *)&APFloatBase::semFloat8E3M4)
    প্রত্যাগম convertFloat8E3M4APFloatToAPInt();

  যদি (semantics == (ধ্রুব llvm::fltSemantics *)&APFloatBase::semFloatTF32)
    প্রত্যাগম convertFloatTF32APFloatToAPInt();

  যদি (semantics == (ধ্রুব llvm::fltSemantics *)&APFloatBase::semFloat8E8M0FNU)
    প্রত্যাগম convertFloat8E8M0FNUAPFloatToAPInt();

  যদি (semantics == (ধ্রুব llvm::fltSemantics *)&APFloatBase::semFloat8E5M3FNU)
    প্রত্যাগম convertFloat8E5M3FNUAPFloatToAPInt();

  যদি (semantics == (ধ্রুব llvm::fltSemantics *)&APFloatBase::semFloat6E3M2FN)
    প্রত্যাগম convertFloat6E3M2FNAPFloatToAPInt();

  যদি (semantics == (ধ্রুব llvm::fltSemantics *)&APFloatBase::semFloat6E2M3FN)
    প্রত্যাগম convertFloat6E2M3FNAPFloatToAPInt();

  যদি (semantics == (ধ্রুব llvm::fltSemantics *)&APFloatBase::semFloat4E2M1FN)
    প্রত্যাগম convertFloat4E2M1FNAPFloatToAPInt();

  llvm_unreachable("unknown format!");
}

ভিন্ন IEEEFloat::convertToFloat() ধ্রুব {
  প্রতিজ্ঞা(semantics == (ধ্রুব llvm::fltSemantics *)&APFloatBase::semIEEEsingle &&
         "Float semantics are not IEEEsingle");
  APInt api = bitcastToAPInt();
  প্রত্যাগম api.bitsToFloat();
}

দ্বিগুণ IEEEFloat::convertToDouble() ধ্রুব {
  প্রতিজ্ঞা(semantics == (ধ্রুব llvm::fltSemantics *)&APFloatBase::semIEEEdouble &&
         "Float semantics are not IEEEdouble");
  APInt api = bitcastToAPInt();
  প্রত্যাগম api.bitsToDouble();
}

#ifdef HAS_IEE754_FLOAT128
float128 IEEEFloat::convertToQuad() ধ্রুব {
  প্রতিজ্ঞা(semantics == (ধ্রুব llvm::fltSemantics *)&APFloatBase::semIEEEquad &&
         "Float semantics are not IEEEquads");
  APInt api = bitcastToAPInt();
  প্রত্যাগম api.bitsToQuad();
}
#endif

শূন্য IEEEFloat::initFromF80LongDoubleAPInt(ধ্রুব APInt &api) {
  প্রত্যাগম initFromIEEEAPInt<APFloatBase::semX87DoubleExtended>(api);
}

শূন্য IEEEFloat::initFromPPCDoubleDoubleLegacyAPInt(ধ্রুব APInt &api) {
  uint64_t i1 = api.getRawData()[0];
  uint64_t i2 = api.getRawData()[1];
  তার্কিক losesInfo;

  // Get the first double and convert to our format.
  initFromDoubleAPInt(APInt(64, i1));
  [[maybe_unused]] opStatus fs = convert(APFloatBase::semPPCDoubleDoubleLegacy,
                                         rmNearestTiesToEven, &losesInfo);
  // (convert may return opInvalidOp if i1 is an sNaN).
  প্রতিজ্ঞা((fs == opOK || fs == opInvalidOp) && !losesInfo);

  // Unless we have a special case, add in second double.
  যদি (isFiniteNonZero()) {
    IEEEFloat v(APFloatBase::semIEEEdouble, APInt(64, i2));
    // The first double determines the category, so ignore a non-canonical
    // second double.
    যদি (v.isFinite()) {
      fs = v.convert(APFloatBase::semPPCDoubleDoubleLegacy, rmNearestTiesToEven,
                     &losesInfo);
      প্রতিজ্ঞা(fs == opOK && !losesInfo);

      add(v, rmNearestTiesToEven);
    }
  }
}

// The E8M0 format has the following characteristics:
// It is an 8-bit unsigned format with only exponents (no actual significand).
// No encodings for {zero, infinities or denorms}.
// NaN is represented by all 1's.
// Bias is 127.
শূন্য IEEEFloat::initFromFloat8E8M0FNUAPInt(ধ্রুব APInt &api) {
  initFromIEEEAPInt<APFloatBase::semFloat8E8M0FNU>(api);
}

শূন্য IEEEFloat::initFromFloat8E5M3FNUAPInt(ধ্রুব APInt &api) {
  initFromIEEEAPInt<APFloatBase::semFloat8E5M3FNU>(api);
}

template <ধ্রুব fltSemantics &S>
শূন্য IEEEFloat::initFromIEEEAPInt(ধ্রুব APInt &api) {
  প্রতিজ্ঞা(api.getBitWidth() == S.sizeInBits);

  constexpr unsigned পূর্ণাঙ্ক trailing_significand_bits =
      S.precision - 1 + S.hasExplicitIntegerBit;
  constexpr integerPart integer_bit =
      integerPart{1} << (trailing_significand_bits % integerPartWidth);
  constexpr uint64_t significand_mask = integer_bit - 1;
  constexpr unsigned পূর্ণাঙ্ক exponent_bits =
      S.sizeInBits - (S.hasSignedRepr ? 1 : 0) - trailing_significand_bits;
  static_assert(exponent_bits < 64);
  constexpr unsigned পূর্ণাঙ্ক stored_significand_parts =
      part_গুনতি_For_Bits(trailing_significand_bits + 1);
  constexpr uint64_t exponent_mask = (uint64_t{1} << exponent_bits) - 1;
  constexpr তার্কিক is_zero_exp_reserved = S.hasDenormals || S.hasZero;
  constexpr পূর্ণাঙ্ক bias = -(S.minExponent - (is_zero_exp_reserved ? 1 : 0));
  constexpr তার্কিক has_significand = trailing_significand_bits > 0;

  // Copy the bits of the significand. We need to clear out the exponent and
  // sign bit in the last word.
  std::array<integerPart, stored_significand_parts> mysignificand;
  যদি constexpr (has_significand) {
    std::copy_n(api.getRawData(), mysignificand.size(), mysignificand.begin());
    যদি constexpr (significand_mask != 0 || S.precision >= integerPartWidth) {
      mysignificand[mysignificand.size() - 1] &= significand_mask;
    }
  } অন্যথা {
    std::fill_n(mysignificand.begin(), mysignificand.size(), 0);
    // Always set integer bit to 1 for consistency in APFloat's internal
    // representation.
    mysignificand[0] = 1;
  }

  // We assume the last word holds the sign bit, the exponent, and potentially
  // some of the trailing significand field.
  uint64_t last_word = api.getRawData()[api.get_অঙ্ক_শব্দগুলি() - 1];
  uint64_t myexponent =
      (last_word >> (trailing_significand_bits % 64)) & exponent_mask;

  initialize(&S);
  প্রতিজ্ঞা(part_গুনতি() == mysignificand.size());

  sign = S.hasSignedRepr
             ? static_cast<unsigned পূর্ণাঙ্ক>(last_word >> ((S.sizeInBits - 1) % 64))
             : 0;

  তার্কিক all_zero_significand =
      has_significand && llvm::all_of(mysignificand, equal_to(0));

  তার্কিক is_zero = myexponent == 0 && all_zero_significand && S.hasZero;

  যদি constexpr (S.nonFiniteBehavior == fltNonfiniteBehavior::IEEE754) {
    তার্কিক is_inf = অসত্য;

    যদি constexpr (S.hasExplicitIntegerBit) {
      // This is only used and tested for x87DoubleExtended
      static_assert(S.precision == 64);
      constexpr integerPart significand_mask_no_int_bit =
          (uint64_t{1} << (trailing_significand_bits - 1)) - 1;
      ধ্রুব integerPart myintegerbit =
          mysignificand[0] >> (trailing_significand_bits - 1);

      is_inf = myexponent - bias == ::exponentInf(S) && myintegerbit == 1 &&
               (mysignificand[0] & significand_mask_no_int_bit) == 0;
    } অন্যথা {
      is_inf = myexponent - bias == ::exponentInf(S) && all_zero_significand;
    }

    যদি (is_inf) {
      makeInf(sign);
      প্রত্যাগম;
    }
  }

  তার্কিক is_nan = অসত্য;

  যদি constexpr (S.nanEncoding == fltNanEncoding::IEEE) {
    যদি constexpr (S.hasExplicitIntegerBit) {
      // This is only used and tested for x87DoubleExtended
      static_assert(S.precision == 64);
      ধ্রুব integerPart myintegerbit =
          mysignificand[0] >> (trailing_significand_bits - 1);
      constexpr integerPart significand_mask_no_int_bit =
          (uint64_t{1} << (trailing_significand_bits - 1)) - 1;

      যদি (myexponent - bias == ::exponentNaN(S) &&
          (mysignificand[0] & significand_mask_no_int_bit) != 0) {
        // regular NaN and pseudoNaN
        is_nan = সত্য;
      } অন্যথা যদি (myexponent - bias == ::exponentNaN(S) &&
                 (mysignificand[0] & significand_mask_no_int_bit) == 0) {
        // pseudoinfinity
        is_nan = সত্য;
      } অন্যথা যদি (myexponent - bias != ::exponentNaN(S) && myexponent != 0 &&
                 myintegerbit == 0) {
        // unnormal
        is_nan = সত্য;
      }
    } অন্যথা {
      is_nan = myexponent - bias == ::exponentNaN(S) && !all_zero_significand;
    }
  } অন্যথা যদি constexpr (S.nanEncoding == fltNanEncoding::AllOnes) {
    তার্কিক all_ones_significand =
        std::all_of(mysignificand.begin(), mysignificand.end() - 1,
                    [](integerPart bits) { প্রত্যাগম bits == ~integerPart{0}; }) &&
        (!significand_mask ||
         mysignificand[mysignificand.size() - 1] == significand_mask);
    is_nan = myexponent - bias == ::exponentNaN(S) && all_ones_significand;
  } অন্যথা যদি constexpr (S.nanEncoding == fltNanEncoding::NegativeZero) {
    is_nan = is_zero && sign;
  }

  যদি (is_nan) {
    category = fcNaN;
    exponent = ::exponentNaN(S);
    std::copy_n(mysignificand.begin(), mysignificand.size(),
                significandParts());
    প্রত্যাগম;
  }

  যদি (is_zero) {
    makeZero(sign);
    প্রত্যাগম;
  }

  category = fcNormal;
  exponent = myexponent - bias;
  std::copy_n(mysignificand.begin(), mysignificand.size(), significandParts());
  যদি (myexponent == 0 && S.hasDenormals) // denormal
    exponent = S.minExponent;
  অন্যথা {
    যদি constexpr (!S.hasExplicitIntegerBit) {
      significandParts()[mysignificand.size() - 1] |= integer_bit;
    }
  }
}

শূন্য IEEEFloat::initFromQuadrupleAPInt(ধ্রুব APInt &api) {
  initFromIEEEAPInt<APFloatBase::semIEEEquad>(api);
}

শূন্য IEEEFloat::initFromDoubleAPInt(ধ্রুব APInt &api) {
  initFromIEEEAPInt<APFloatBase::semIEEEdouble>(api);
}

শূন্য IEEEFloat::initFromFloatAPInt(ধ্রুব APInt &api) {
  initFromIEEEAPInt<APFloatBase::semIEEEsingle>(api);
}

শূন্য IEEEFloat::initFromBFloatAPInt(ধ্রুব APInt &api) {
  initFromIEEEAPInt<APFloatBase::semBFloat>(api);
}

শূন্য IEEEFloat::initFromHalfAPInt(ধ্রুব APInt &api) {
  initFromIEEEAPInt<APFloatBase::semIEEEhalf>(api);
}

শূন্য IEEEFloat::initFromFloat8E5M2APInt(ধ্রুব APInt &api) {
  initFromIEEEAPInt<APFloatBase::semFloat8E5M2>(api);
}

শূন্য IEEEFloat::initFromFloat8E5M2FNUZAPInt(ধ্রুব APInt &api) {
  initFromIEEEAPInt<APFloatBase::semFloat8E5M2FNUZ>(api);
}

শূন্য IEEEFloat::initFromFloat8E4M3APInt(ধ্রুব APInt &api) {
  initFromIEEEAPInt<APFloatBase::semFloat8E4M3>(api);
}

শূন্য IEEEFloat::initFromFloat8E4M3FNAPInt(ধ্রুব APInt &api) {
  initFromIEEEAPInt<APFloatBase::semFloat8E4M3FN>(api);
}

শূন্য IEEEFloat::initFromFloat8E4M3FNUZAPInt(ধ্রুব APInt &api) {
  initFromIEEEAPInt<APFloatBase::semFloat8E4M3FNUZ>(api);
}

শূন্য IEEEFloat::initFromFloat8E4M3B11FNUZAPInt(ধ্রুব APInt &api) {
  initFromIEEEAPInt<APFloatBase::semFloat8E4M3B11FNUZ>(api);
}

শূন্য IEEEFloat::initFromFloat8E3M4APInt(ধ্রুব APInt &api) {
  initFromIEEEAPInt<APFloatBase::semFloat8E3M4>(api);
}

শূন্য IEEEFloat::initFromFloatTF32APInt(ধ্রুব APInt &api) {
  initFromIEEEAPInt<APFloatBase::semFloatTF32>(api);
}

শূন্য IEEEFloat::initFromFloat6E3M2FNAPInt(ধ্রুব APInt &api) {
  initFromIEEEAPInt<APFloatBase::semFloat6E3M2FN>(api);
}

শূন্য IEEEFloat::initFromFloat6E2M3FNAPInt(ধ্রুব APInt &api) {
  initFromIEEEAPInt<APFloatBase::semFloat6E2M3FN>(api);
}

শূন্য IEEEFloat::initFromFloat4E2M1FNAPInt(ধ্রুব APInt &api) {
  initFromIEEEAPInt<APFloatBase::semFloat4E2M1FN>(api);
}

/// Treat api as containing the bits of a floating point number.
শূন্য IEEEFloat::initFromAPInt(ধ্রুব fltSemantics *Sem, ধ্রুব APInt &api) {
  প্রতিজ্ঞা(api.getBitWidth() == Sem->sizeInBits);
  যদি (Sem == &APFloatBase::semIEEEhalf)
    প্রত্যাগম initFromHalfAPInt(api);
  যদি (Sem == &APFloatBase::semBFloat)
    প্রত্যাগম initFromBFloatAPInt(api);
  যদি (Sem == &APFloatBase::semIEEEsingle)
    প্রত্যাগম initFromFloatAPInt(api);
  যদি (Sem == &APFloatBase::semIEEEdouble)
    প্রত্যাগম initFromDoubleAPInt(api);
  যদি (Sem == &APFloatBase::semIEEEquad)
    প্রত্যাগম initFromQuadrupleAPInt(api);
  যদি (Sem == &APFloatBase::semX87DoubleExtended)
    প্রত্যাগম initFromF80LongDoubleAPInt(api);
  যদি (Sem == &APFloatBase::semPPCDoubleDoubleLegacy)
    প্রত্যাগম initFromPPCDoubleDoubleLegacyAPInt(api);
  যদি (Sem == &APFloatBase::semFloat8E5M2)
    প্রত্যাগম initFromFloat8E5M2APInt(api);
  যদি (Sem == &APFloatBase::semFloat8E5M2FNUZ)
    প্রত্যাগম initFromFloat8E5M2FNUZAPInt(api);
  যদি (Sem == &APFloatBase::semFloat8E4M3)
    প্রত্যাগম initFromFloat8E4M3APInt(api);
  যদি (Sem == &APFloatBase::semFloat8E4M3FN)
    প্রত্যাগম initFromFloat8E4M3FNAPInt(api);
  যদি (Sem == &APFloatBase::semFloat8E4M3FNUZ)
    প্রত্যাগম initFromFloat8E4M3FNUZAPInt(api);
  যদি (Sem == &APFloatBase::semFloat8E4M3B11FNUZ)
    প্রত্যাগম initFromFloat8E4M3B11FNUZAPInt(api);
  যদি (Sem == &APFloatBase::semFloat8E3M4)
    প্রত্যাগম initFromFloat8E3M4APInt(api);
  যদি (Sem == &APFloatBase::semFloatTF32)
    প্রত্যাগম initFromFloatTF32APInt(api);
  যদি (Sem == &APFloatBase::semFloat8E8M0FNU)
    প্রত্যাগম initFromFloat8E8M0FNUAPInt(api);
  যদি (Sem == &APFloatBase::semFloat8E5M3FNU)
    প্রত্যাগম initFromFloat8E5M3FNUAPInt(api);
  যদি (Sem == &APFloatBase::semFloat6E3M2FN)
    প্রত্যাগম initFromFloat6E3M2FNAPInt(api);
  যদি (Sem == &APFloatBase::semFloat6E2M3FN)
    প্রত্যাগম initFromFloat6E2M3FNAPInt(api);
  যদি (Sem == &APFloatBase::semFloat4E2M1FN)
    প্রত্যাগম initFromFloat4E2M1FNAPInt(api);

  llvm_unreachable("unsupported semantics");
}

/// Make this number the largest magnitude normal number in the given
/// semantics.
শূন্য IEEEFloat::makeLargest(তার্কিক Negative) {
  যদি (Negative && !semantics->hasSignedRepr)
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
  unsigned Part_গুনতি = part_গুনতি();
  memset(significand, 0xFF, পরিমাণ(integerPart)*(Part_গুনতি - 1));

  // Set the high integerPart especially setting all unused top bits for
  // internal consistency.
  ধ্রুব unsigned অঙ্ক_Unused_উপরের_Bits =
    Part_গুনতি*integerPartWidth - semantics->precision;
  significand[Part_গুনতি - 1] = (অঙ্ক_Unused_উপরের_Bits < integerPartWidth)
                                   ? (~integerPart(0) >> অঙ্ক_Unused_উপরের_Bits)
                                   : 0;
  যদি (semantics->nonFiniteBehavior == fltNonfiniteBehavior::NanOnly &&
      semantics->nanEncoding == fltNanEncoding::AllOnes &&
      (semantics->precision > 1))
    significand[0] &= ~integerPart(1);
}

/// Make this number the smallest magnitude denormal number in the given
/// semantics.
শূন্য IEEEFloat::makeSmallest(তার্কিক Negative) {
  যদি (Negative && !semantics->hasSignedRepr)
    llvm_unreachable(
        "This floating point format does not support signed values");
  // We want (in interchange format):
  //   sign = {Negative}
  //   exponent = 0..0
  //   significand = 0..01
  category = fcNormal;
  sign = Negative;
  exponent = semantics->minExponent;
  APInt::tcSet(significandParts(), 1, part_গুনতি());
}

শূন্য IEEEFloat::makeSmallestNormalized(তার্কিক Negative) {
  যদি (Negative && !semantics->hasSignedRepr)
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

IEEEFloat::IEEEFloat(ধ্রুব fltSemantics &Sem, ধ্রুব APInt &API) {
  initFromAPInt(&Sem, API);
}

IEEEFloat::IEEEFloat(ভিন্ন f) {
  initFromAPInt(&APFloatBase::semIEEEsingle, APInt::floatToBits(f));
}

IEEEFloat::IEEEFloat(দ্বিগুণ d) {
  initFromAPInt(&APFloatBase::semIEEEdouble, APInt::doubleToBits(d));
}

নামাকাশ {
  শূন্য append(SmallVectorImpl<বর্ণ> &Buffer, StringRef Str) {
    Buffer.append(Str.begin(), Str.end());
  }

  /// Removes data from the given significand until it is no more
  /// precise than is required for the desired precision.
  শূন্য AdjustToPrecision(APInt &significand,
                         পূর্ণাঙ্ক &exp, unsigned FormatPrecision) {
    unsigned bits = significand.getActiveBits();

    // 196/59 is a very slight overestimate of lg_2(10).
    unsigned bitsRequired = (FormatPrecision * 196 + 58) / 59;

    যদি (bits <= bitsRequired) প্রত্যাগম;

    unsigned tensRemovable = (bits - bitsRequired) * 59 / 196;
    যদি (!tensRemovable) প্রত্যাগম;

    exp += tensRemovable;

    APInt ভাজক(significand.getBitWidth(), 1);
    APInt powten(significand.getBitWidth(), 10);
    যাবত্ (সত্য) {
      যদি (tensRemovable & 1)
        ভাজক *= powten;
      tensRemovable >>= 1;
      যদি (!tensRemovable) ভঙ্গ;
      powten *= powten;
    }

    significand = significand.udiv(ভাজক);

    // Truncate the significand down to its active bit count.
    significand = significand.trunc(significand.getActiveBits());
  }


  শূন্য AdjustToPrecision(SmallVectorImpl<বর্ণ> &buffer,
                         পূর্ণাঙ্ক &exp, unsigned FormatPrecision) {
    unsigned N = buffer.size();
    যদি (N <= FormatPrecision) প্রত্যাগম;

    // The most significant figures are the last ones in the buffer.
    unsigned FirstSignificant = N - FormatPrecision;

    // Round.
    // FIXME: this probably shouldn't use 'round half up'.

    // Rounding down is just a truncation, except we also want to drop
    // trailing zeros from the new result.
    যদি (buffer[FirstSignificant - 1] < '5') {
      যাবত্ (FirstSignificant < N && buffer[FirstSignificant] == '0')
        FirstSignificant++;

      exp += FirstSignificant;
      buffer.erase(&buffer[0], &buffer[FirstSignificant]);
      প্রত্যাগম;
    }

    // Rounding up requires a decimal add-with-carry.  If we continue
    // the carry, the newly-introduced zeros will just be truncated.
    প্রতি (unsigned I = FirstSignificant; I != N; ++I) {
      যদি (buffer[I] == '9') {
        FirstSignificant++;
      } অন্যথা {
        buffer[I]++;
        ভঙ্গ;
      }
    }

    // If we carried through, we have exactly one digit of precision.
    যদি (FirstSignificant == N) {
      exp += FirstSignificant;
      buffer.clear();
      buffer.ঠেলো_back('1');
      প্রত্যাগম;
    }

    exp += FirstSignificant;
    buffer.erase(&buffer[0], &buffer[FirstSignificant]);
  }

  শূন্য toStringImpl(SmallVectorImpl<বর্ণ> &Str, ধ্রুব তার্কিক isNeg, পূর্ণাঙ্ক exp,
                    APInt significand, unsigned FormatPrecision,
                    unsigned FormatMaxPadding, তার্কিক TruncateZero) {
    ধ্রুব পূর্ণাঙ্ক semanticsPrecision = significand.getBitWidth();

    যদি (isNeg)
      Str.ঠেলো_back('-');

    // Set FormatPrecision if zero.  We want to do this before we
    // truncate trailing zeros, as those are part of the precision.
    যদি (!FormatPrecision) {
      // We use enough digits so the number can be round-tripped back to an
      // APFloat. The formula comes from "How to Print Floating-Point Numbers
      // Accurately" by Steele and White.
      // FIXME: Using a formula based purely on the precision is conservative;
      // we can print fewer digits depending on the actual value being printed.

      // FormatPrecision = 2 + floor(significandBits / lg_2(10))
      FormatPrecision = 2 + semanticsPrecision * 59 / 196;
    }

    // Ignore trailing binary zeros.
    পূর্ণাঙ্ক trailingZeros = significand.countr_zero();
    exp += trailingZeros;
    significand.lshrInPlace(trailingZeros);

    // Change the exponent from 2^e to 10^e.
    যদি (exp == 0) {
      // Nothing to do.
    } অন্যথা যদি (exp > 0) {
      // Just shift left.
      significand = significand.zext(semanticsPrecision + exp);
      significand <<= exp;
      exp = 0;
    } অন্যথা { /* exp < 0 */
      পূর্ণাঙ্ক texp = -exp;

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
      যাবত্ (সত্য) {
        যদি (texp & 1)
          significand *= five_to_the_i;

        texp >>= 1;
        যদি (!texp)
          ভঙ্গ;
        five_to_the_i *= five_to_the_i;
      }
    }

    AdjustToPrecision(significand, exp, FormatPrecision);

    SmallVector<বর্ণ, 256> buffer;

    // Fill the buffer.
    unsigned precision = significand.getBitWidth();
    যদি (precision < 4) {
      // We need enough precision to store the value 10.
      precision = 4;
      significand = significand.zext(precision);
    }
    APInt ten(precision, 10);
    APInt digit(precision, 0);

    তার্কিক inTrail = সত্য;
    যাবত্ (significand != 0) {
      // digit <- significand % 10
      // significand <- significand / 10
      APInt::udivrem(significand, ten, significand, digit);

      unsigned d = digit.get_ZExt_মান();

      // Drop trailing zeros.
      যদি (inTrail && !d)
        exp++;
      অন্যথা {
        buffer.ঠেলো_back((বর্ণ) ('0' + d));
        inTrail = অসত্য;
      }
    }

    প্রতিজ্ঞা(!buffer.empty() && "no characters in buffer!");

    // Drop down to FormatPrecision.
    // TODO: don't do more precise calculations above than are required.
    AdjustToPrecision(buffer, exp, FormatPrecision);

    unsigned NDigits = buffer.size();

    // Check whether we should use scientific notation.
    তার্কিক FormatScientific;
    যদি (!FormatMaxPadding) {
      FormatScientific = সত্য;
    } অন্যথা {
      যদি (exp >= 0) {
        // 765e3 --> 765000
        //              ^^^
        // But we shouldn't make the number look more precise than it is.
        FormatScientific = ((unsigned) exp > FormatMaxPadding ||
                            NDigits + (unsigned) exp > FormatPrecision);
      } অন্যথা {
        // Power of the most significant digit.
        পূর্ণাঙ্ক MSD = exp + (পূর্ণাঙ্ক) (NDigits - 1);
        যদি (MSD >= 0) {
          // 765e-2 == 7.65
          FormatScientific = অসত্য;
        } অন্যথা {
          // 765e-5 == 0.00765
          //           ^ ^^
          FormatScientific = ((unsigned) -MSD) > FormatMaxPadding;
        }
      }
    }

    // Scientific formatting is pretty straightforward.
    যদি (FormatScientific) {
      exp += (NDigits - 1);

      Str.ঠেলো_back(buffer[NDigits-1]);
      Str.ঠেলো_back('.');
      যদি (NDigits == 1 && TruncateZero)
        Str.ঠেলো_back('0');
      অন্যথা
        প্রতি (unsigned I = 1; I != NDigits; ++I)
          Str.ঠেলো_back(buffer[NDigits-1-I]);
      // Fill with zeros up to FormatPrecision.
      যদি (!TruncateZero && FormatPrecision > NDigits - 1)
        Str.append(FormatPrecision - NDigits + 1, '0');
      // For !TruncateZero we use lower 'e'.
      Str.ঠেলো_back(TruncateZero ? 'E' : 'e');

      Str.ঠেলো_back(exp >= 0 ? '+' : '-');
      যদি (exp < 0)
        exp = -exp;
      SmallVector<বর্ণ, 6> expbuf;
      কুরু {
        expbuf.ঠেলো_back((বর্ণ) ('0' + (exp % 10)));
        exp /= 10;
      } যাবত্ (exp);
      // Exponent always at least two digits if we do not truncate zeros.
      যদি (!TruncateZero && expbuf.size() < 2)
        expbuf.ঠেলো_back('0');
      প্রতি (unsigned I = 0, E = expbuf.size(); I != E; ++I)
        Str.ঠেলো_back(expbuf[E-1-I]);
      প্রত্যাগম;
    }

    // Non-scientific, positive exponents.
    যদি (exp >= 0) {
      প্রতি (unsigned I = 0; I != NDigits; ++I)
        Str.ঠেলো_back(buffer[NDigits-1-I]);
      প্রতি (unsigned I = 0; I != (unsigned) exp; ++I)
        Str.ঠেলো_back('0');
      প্রত্যাগম;
    }

    // Non-scientific, negative exponents.

    // The number of digits to the left of the decimal point.
    পূর্ণাঙ্ক NWholeDigits = exp + (পূর্ণাঙ্ক) NDigits;

    unsigned I = 0;
    যদি (NWholeDigits > 0) {
      প্রতি (; I != (unsigned) NWholeDigits; ++I)
        Str.ঠেলো_back(buffer[NDigits-I-1]);
      Str.ঠেলো_back('.');
    } অন্যথা {
      unsigned NZeros = 1 + (unsigned) -NWholeDigits;

      Str.ঠেলো_back('0');
      Str.ঠেলো_back('.');
      প্রতি (unsigned Z = 1; Z != NZeros; ++Z)
        Str.ঠেলো_back('0');
    }

    প্রতি (; I != NDigits; ++I)
      Str.ঠেলো_back(buffer[NDigits-I-1]);

  }
} // namespace

শূন্য IEEEFloat::toString(SmallVectorImpl<বর্ণ> &Str, unsigned FormatPrecision,
                         unsigned FormatMaxPadding, তার্কিক TruncateZero) ধ্রুব {
  বিকল্প (category) {
  পক্ষ fcInfinity:
    যদি (isNegative())
      প্রত্যাগম append(Str, "-Inf");
    অন্যথা
      প্রত্যাগম append(Str, "+Inf");

  পক্ষ fcNaN: প্রত্যাগম append(Str, "NaN");

  পক্ষ fcZero:
    যদি (isNegative())
      Str.ঠেলো_back('-');

    যদি (!FormatMaxPadding) {
      যদি (TruncateZero)
        append(Str, "0.0E+0");
      অন্যথা {
        append(Str, "0.0");
        যদি (FormatPrecision > 1)
          Str.append(FormatPrecision - 1, '0');
        append(Str, "e+00");
      }
    } অন্যথা {
      Str.ঠেলো_back('0');
    }
    প্রত্যাগম;

  পক্ষ fcNormal:
    ভঙ্গ;
  }

  // Decompose the number into an APInt and an exponent.
  পূর্ণাঙ্ক exp = exponent - ((পূর্ণাঙ্ক) semantics->precision - 1);
  APInt significand(
      semantics->precision,
      ArrayRef(significandParts(), part_গুনতি_For_Bits(semantics->precision)));

  toStringImpl(Str, isNegative(), exp, significand, FormatPrecision,
               FormatMaxPadding, TruncateZero);

}

পূর্ণাঙ্ক IEEEFloat::getExactLog2Abs() ধ্রুব {
  যদি (!isFinite() || isZero())
    প্রত্যাগম INT_MIN;

  ধ্রুব integerPart *Parts = significandParts();
  ধ্রুব পূর্ণাঙ্ক Part_গুনতি = part_গুনতি_For_Bits(semantics->precision);

  পূর্ণাঙ্ক বের_করো_গুনতি = 0;
  প্রতি (পূর্ণাঙ্ক i = 0; i < Part_গুনতি; ++i) {
    বের_করো_গুনতি += llvm::popcount(Parts[i]);
    যদি (বের_করো_গুনতি > 1)
      প্রত্যাগম INT_MIN;
  }

  যদি (exponent != semantics->minExponent)
    প্রত্যাগম exponent;

  পূর্ণাঙ্ক CountrParts = 0;
  প্রতি (পূর্ণাঙ্ক i = 0; i < Part_গুনতি;
       ++i, CountrParts += APInt::APINT_BITS_PER_WORD) {
    যদি (Parts[i] != 0) {
      প্রত্যাগম exponent - semantics->precision + CountrParts +
             llvm::countr_zero(Parts[i]) + 1;
    }
  }

  llvm_unreachable("didn't find the set bit");
}

তার্কিক IEEEFloat::isSignaling() ধ্রুব {
  যদি (!isNaN())
    প্রত্যাগম অসত্য;
  যদি (semantics->nonFiniteBehavior == fltNonfiniteBehavior::NanOnly ||
      semantics->nonFiniteBehavior == fltNonfiniteBehavior::FiniteOnly)
    প্রত্যাগম অসত্য;

  // IEEE-754R 2008 6.2.1: A signaling NaN bit string should be encoded with the
  // first bit of the trailing significand being 0.
  প্রত্যাগম !APInt::tcExtractBit(significandParts(), semantics->precision - 2);
}

/// IEEE-754R 2008 5.3.1: nextUp/nextDown.
///
/// *NOTE* since nextDown(x) = -nextUp(-x), we only implement nextUp with
/// appropriate sign switching before/after the computation.
APFloat::opStatus IEEEFloat::next(তার্কিক nextDown) {
  // If we are performing nextDown, swap sign so we have -x.
  যদি (nextDown)
    changeSign();

  // Compute nextUp(x)
  opStatus result = opOK;

  // Handle each float category separately.
  বিকল্প (category) {
  পক্ষ fcInfinity:
    // nextUp(+inf) = +inf
    যদি (!isNegative())
      ভঙ্গ;
    // nextUp(-inf) = -getLargest()
    makeLargest(সত্য);
    ভঙ্গ;
  পক্ষ fcNaN:
    // IEEE-754R 2008 6.2 Par 2: nextUp(sNaN) = qNaN. Set Invalid flag.
    // IEEE-754R 2008 6.2: nextUp(qNaN) = qNaN. Must be identity so we do not
    //                     change the payload.
    যদি (isSignaling()) {
      result = opInvalidOp;
      // For consistency, propagate the sign of the sNaN to the qNaN.
      makeNaN(অসত্য, isNegative(), nullptr);
    }
    ভঙ্গ;
  পক্ষ fcZero:
    // nextUp(pm 0) = +getSmallest()
    makeSmallest(অসত্য);
    ভঙ্গ;
  পক্ষ fcNormal:
    // nextUp(-getSmallest()) = -0
    যদি (isSmallest() && isNegative()) {
      APInt::tcSet(significandParts(), 0, part_গুনতি());
      category = fcZero;
      exponent = 0;
      যদি (semantics->nanEncoding == fltNanEncoding::NegativeZero)
        sign = অসত্য;
      যদি (!semantics->hasZero)
        makeSmallestNormalized(অসত্য);
      ভঙ্গ;
    }

    যদি (isLargest() && !isNegative()) {
      যদি (semantics->nonFiniteBehavior == fltNonfiniteBehavior::NanOnly) {
        // nextUp(getLargest()) == NAN
        makeNaN();
        ভঙ্গ;
      } অন্যথা যদি (semantics->nonFiniteBehavior ==
                 fltNonfiniteBehavior::FiniteOnly) {
        // nextUp(getLargest()) == getLargest()
        ভঙ্গ;
      } অন্যথা {
        // nextUp(getLargest()) == INFINITY
        APInt::tcSet(significandParts(), 0, part_গুনতি());
        category = fcInfinity;
        exponent = semantics->maxExponent + 1;
        ভঙ্গ;
      }
    }

    // nextUp(normal) == normal + inc.
    যদি (isNegative()) {
      // If we are negative, we need to decrement the significand.

      // We only cross a binade boundary that requires adjusting the exponent
      // if:
      //   1. exponent != semantics->minExponent. This implies we are not in the
      //   smallest binade or are dealing with denormals.
      //   2. Our significand excluding the integral bit is all zeros.
      তার্কিক WillCrossBinadeBoundary =
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
      APInt::tcDecrement(Parts, part_গুনতি());

      যদি (WillCrossBinadeBoundary) {
        // Our result is a normal number. Do the following:
        // 1. Set the integral bit to 1.
        // 2. Decrement the exponent.
        APInt::tcSetBit(Parts, semantics->precision - 1);
        exponent--;
      }
    } অন্যথা {
      // If we are positive, we need to increment the significand.

      // We only cross a binade boundary that requires adjusting the exponent if
      // the input is not a denormal and all of said input's significand bits
      // are set. If all of said conditions are true: clear the significand, set
      // the integral bit to 1, and increment the exponent. If we have a
      // denormal always increment since moving denormals and the numbers in the
      // smallest normal binade have the same exponent in our representation.
      // If there are only exponents, any increment always crosses the
      // BinadeBoundary.
      তার্কিক WillCrossBinadeBoundary = !APFloat::hasSignificand(*semantics) ||
                                     (!isDenormal() && isSignificandAllOnes());

      যদি (WillCrossBinadeBoundary) {
        integerPart *Parts = significandParts();
        APInt::tcSet(Parts, 0, part_গুনতি());
        APInt::tcSetBit(Parts, semantics->precision - 1);
        প্রতিজ্ঞা(exponent != semantics->maxExponent &&
               "We can not increment an exponent beyond the maxExponent allowed"
               " by the given floating point semantics.");
        exponent++;
      } অন্যথা {
        incrementSignificand();
      }
    }
    ভঙ্গ;
  }

  // If we are performing nextDown, swap sign so we have -nextUp(-x)
  যদি (nextDown)
    changeSign();

  প্রত্যাগম result;
}

APInt IEEEFloat::getNaNPayload() ধ্রুব {
  প্রতিজ্ঞা(isNaN() && "Can only be called on NaN values");
  // Number of bits in the payload, excluding the (maybe implied) integer bit.
  unsigned Bits = semantics->precision - 1;
  প্রত্যাগম APInt(Bits, ArrayRef(significandParts(), part_গুনতি_For_Bits(Bits)));
}

APFloatBase::ExponentType IEEEFloat::exponentNaN() ধ্রুব {
  প্রত্যাগম ::exponentNaN(*semantics);
}

APFloatBase::ExponentType IEEEFloat::exponentInf() ধ্রুব {
  প্রত্যাগম ::exponentInf(*semantics);
}

APFloatBase::ExponentType IEEEFloat::exponentZero() ধ্রুব {
  প্রত্যাগম ::exponentZero(*semantics);
}

শূন্য IEEEFloat::makeInf(তার্কিক Negative) {
  যদি (semantics->nonFiniteBehavior == fltNonfiniteBehavior::FiniteOnly)
    llvm_unreachable("This floating point format does not support Inf");

  যদি (semantics->nonFiniteBehavior == fltNonfiniteBehavior::NanOnly) {
    // There is no Inf, so make NaN instead.
    makeNaN(অসত্য, Negative);
    প্রত্যাগম;
  }
  category = fcInfinity;
  sign = Negative;
  exponent = exponentInf();
  APInt::tcSet(significandParts(), 0, part_গুনতি());
}

শূন্য IEEEFloat::makeZero(তার্কিক Negative) {
  যদি (!semantics->hasZero)
    llvm_unreachable("This floating point format does not support Zero");

  category = fcZero;
  sign = Negative;
  যদি (semantics->nanEncoding == fltNanEncoding::NegativeZero) {
    // Merge negative zero to positive because 0b10000...000 is used for NaN
    sign = অসত্য;
  }
  exponent = exponentZero();
  APInt::tcSet(significandParts(), 0, part_গুনতি());
}

শূন্য IEEEFloat::makeQuiet() {
  প্রতিজ্ঞা(isNaN());
  যদি (semantics->nonFiniteBehavior != fltNonfiniteBehavior::NanOnly)
    APInt::tcSetBit(significandParts(), semantics->precision - 2);
}

পূর্ণাঙ্ক ilogb(ধ্রুব IEEEFloat &Arg) {
  যদি (Arg.isNaN())
    প্রত্যাগম APFloat::IEK_NaN;
  যদি (Arg.isZero())
    প্রত্যাগম APFloat::IEK_Zero;
  যদি (Arg.isInfinity())
    প্রত্যাগম APFloat::IEK_Inf;
  যদি (!Arg.isDenormal())
    প্রত্যাগম Arg.exponent;

  IEEEFloat Normalized(Arg);
  পূর্ণাঙ্ক SignificandBits = Arg.getSemantics().precision - 1;

  Normalized.exponent += SignificandBits;
  Normalized.normalize(APFloat::rmNearestTiesToEven, lfExactlyZero);
  প্রত্যাগম Normalized.exponent - SignificandBits;
}

IEEEFloat scalbn(IEEEFloat X, পূর্ণাঙ্ক Exp, roundingMode RoundingMode) {
  auto MaxExp = X.getSemantics().maxExponent;
  auto MinExp = X.getSemantics().minExponent;

  // If Exp is wildly out-of-scale, simply adding it to X.exponent will
  // overflow; clamp it to a safe range before adding, but ensure that the range
  // is large enough that the clamp does not change the result. The range we
  // need to support is the difference between the largest possible exponent and
  // the normalized exponent of half the smallest denormal.

  পূর্ণাঙ্ক SignificandBits = X.getSemantics().precision - 1;
  পূর্ণাঙ্ক MaxIncrement = MaxExp - (MinExp - SignificandBits) + 1;

  // Clamp to one past the range ends to let normalize handle overlflow.
  X.exponent += std::clamp(Exp, -MaxIncrement - 1, MaxIncrement);
  X.normalize(RoundingMode, lfExactlyZero);
  যদি (X.isNaN())
    X.makeQuiet();
  প্রত্যাগম X;
}

IEEEFloat frexp(ধ্রুব IEEEFloat &Val, পূর্ণাঙ্ক &Exp, roundingMode RM) {
  Exp = ilogb(Val);

  // Quiet signalling nans.
  যদি (Exp == APFloat::IEK_NaN) {
    IEEEFloat Quiet(Val);
    Quiet.makeQuiet();
    প্রত্যাগম Quiet;
  }

  যদি (Exp == APFloat::IEK_Inf)
    প্রত্যাগম Val;

  // 1 is added because frexp is defined to return a normalized fraction in
  // +/-[0.5, 1.0), rather than the usual +/-[1.0, 2.0).
  Exp = Exp == APFloat::IEK_Zero ? 0 : Exp + 1;
  প্রত্যাগম scalbn(Val, -Exp, RM);
}

DoubleAPFloat::DoubleAPFloat(ধ্রুব fltSemantics &S)
    : Semantics(&S),
      Floats(নূতন APFloat[2]{APFloat(APFloatBase::semIEEEdouble),
                            APFloat(APFloatBase::semIEEEdouble)}) {
  প্রতিজ্ঞা(Semantics == &APFloatBase::semPPCDoubleDouble);
}

DoubleAPFloat::DoubleAPFloat(ধ্রুব fltSemantics &S, uninitializedTag)
    : Semantics(&S), Floats(নূতন APFloat[2]{
                         APFloat(APFloatBase::semIEEEdouble, uninitialized),
                         APFloat(APFloatBase::semIEEEdouble, uninitialized)}) {
  প্রতিজ্ঞা(Semantics == &APFloatBase::semPPCDoubleDouble);
}

DoubleAPFloat::DoubleAPFloat(ধ্রুব fltSemantics &S, integerPart I)
    : Semantics(&S),
      Floats(নূতন APFloat[2]{APFloat(APFloatBase::semIEEEdouble, I),
                            APFloat(APFloatBase::semIEEEdouble)}) {
  প্রতিজ্ঞা(Semantics == &APFloatBase::semPPCDoubleDouble);
}

DoubleAPFloat::DoubleAPFloat(ধ্রুব fltSemantics &S, ধ্রুব APInt &I)
    : Semantics(&S),
      Floats(নূতন APFloat[2]{
          APFloat(APFloatBase::semIEEEdouble, APInt(64, I.getRawData()[0])),
          APFloat(APFloatBase::semIEEEdouble, APInt(64, I.getRawData()[1]))}) {
  প্রতিজ্ঞা(Semantics == &APFloatBase::semPPCDoubleDouble);
}

DoubleAPFloat::DoubleAPFloat(ধ্রুব fltSemantics &S, APFloat &&First,
                             APFloat &&Second)
    : Semantics(&S),
      Floats(নূতন APFloat[2]{std::move(First), std::move(Second)}) {
  প্রতিজ্ঞা(Semantics == &APFloatBase::semPPCDoubleDouble);
  প্রতিজ্ঞা(&Floats[0].getSemantics() == &APFloatBase::semIEEEdouble);
  প্রতিজ্ঞা(&Floats[1].getSemantics() == &APFloatBase::semIEEEdouble);
}

DoubleAPFloat::DoubleAPFloat(ধ্রুব DoubleAPFloat &RHS)
    : Semantics(RHS.Semantics),
      Floats(RHS.Floats ? নূতন APFloat[2]{APFloat(RHS.Floats[0]),
                                         APFloat(RHS.Floats[1])}
                        : nullptr) {
  প্রতিজ্ঞা(Semantics == &APFloatBase::semPPCDoubleDouble);
}

DoubleAPFloat::DoubleAPFloat(DoubleAPFloat &&RHS)
    : Semantics(RHS.Semantics), Floats(RHS.Floats) {
  RHS.Semantics = &APFloatBase::semBogus;
  RHS.Floats = nullptr;
  প্রতিজ্ঞা(Semantics == &APFloatBase::semPPCDoubleDouble);
}

DoubleAPFloat &DoubleAPFloat::operator=(ধ্রুব DoubleAPFloat &RHS) {
  যদি (Semantics == RHS.Semantics && RHS.Floats) {
    Floats[0] = RHS.Floats[0];
    Floats[1] = RHS.Floats[1];
  } অন্যথা যদি (এতত্ != &RHS) {
    এতত্->~DoubleAPFloat();
    নূতন (এতত্) DoubleAPFloat(RHS);
  }
  প্রত্যাগম *এতত্;
}

// Returns a result such that:
// 1. abs(Lo) <= ulp(Hi)/2
// 2. Hi == RTNE(Hi + Lo)
// 3. Hi + Lo == X + Y
//
// Requires that log2(X) >= log2(Y).
স্থির std::pair<APFloat, APFloat> fastTwoSum(APFloat X, APFloat Y) {
  যদি (!X.isFinite())
    প্রত্যাগম {X, APFloat::getZero(X.getSemantics(), /*Negative=*/অসত্য)};
  APFloat Hi = X + Y;
  APFloat Delta = Hi - X;
  APFloat Lo = Y - Delta;
  প্রত্যাগম {Hi, Lo};
}

// Implement addition, subtraction, multiplication and division based on:
// "Software for Doubled-Precision Floating-Point Computations",
// by Seppo Linnainmaa, ACM TOMS vol 7 no 3, September 1981, pages 272-283.
APFloat::opStatus DoubleAPFloat::addImpl(ধ্রুব APFloat &a, ধ্রুব APFloat &aa,
                                         ধ্রুব APFloat &c, ধ্রুব APFloat &cc,
                                         roundingMode RM) {
  পূর্ণাঙ্ক Status = opOK;
  APFloat z = a;
  Status |= z.add(c, RM);
  যদি (!z.isFinite()) {
    যদি (!z.isInfinity()) {
      Floats[0] = std::move(z);
      Floats[1].makeZero(/* Neg = */ অসত্য);
      প্রত্যাগম (opStatus)Status;
    }
    Status = opOK;
    auto AComparedToC = a.compare_Absolute_মান(c);
    z = cc;
    Status |= z.add(aa, RM);
    যদি (AComparedToC == APFloat::cmpGreaterThan) {
      // z = cc + aa + c + a;
      Status |= z.add(c, RM);
      Status |= z.add(a, RM);
    } অন্যথা {
      // z = cc + aa + a + c;
      Status |= z.add(a, RM);
      Status |= z.add(c, RM);
    }
    যদি (!z.isFinite()) {
      Floats[0] = std::move(z);
      Floats[1].makeZero(/* Neg = */ অসত্য);
      প্রত্যাগম (opStatus)Status;
    }
    Floats[0] = z;
    APFloat zz = aa;
    Status |= zz.add(cc, RM);
    যদি (AComparedToC == APFloat::cmpGreaterThan) {
      // Floats[1] = a - z + c + zz;
      Floats[1] = a;
      Status |= Floats[1].subtract(z, RM);
      Status |= Floats[1].add(c, RM);
      Status |= Floats[1].add(zz, RM);
    } অন্যথা {
      // Floats[1] = c - z + a + zz;
      Floats[1] = c;
      Status |= Floats[1].subtract(z, RM);
      Status |= Floats[1].add(a, RM);
      Status |= Floats[1].add(zz, RM);
    }
  } অন্যথা {
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
    যদি (zz.isZero() && !zz.isNegative()) {
      Floats[0] = std::move(z);
      Floats[1].makeZero(/* Neg = */ অসত্য);
      প্রত্যাগম opOK;
    }
    Floats[0] = z;
    Status |= Floats[0].add(zz, RM);
    যদি (!Floats[0].isFinite()) {
      Floats[1].makeZero(/* Neg = */ অসত্য);
      প্রত্যাগম (opStatus)Status;
    }
    Floats[1] = std::move(z);
    Status |= Floats[1].subtract(Floats[0], RM);
    Status |= Floats[1].add(zz, RM);
  }
  প্রত্যাগম (opStatus)Status;
}

APFloat::opStatus DoubleAPFloat::addWithSpecial(ধ্রুব DoubleAPFloat &LHS,
                                                ধ্রুব DoubleAPFloat &RHS,
                                                DoubleAPFloat &Out,
                                                roundingMode RM) {
  যদি (LHS.getCategory() == fcNaN) {
    Out = LHS;
    প্রত্যাগম opOK;
  }
  যদি (RHS.getCategory() == fcNaN) {
    Out = RHS;
    প্রত্যাগম opOK;
  }
  যদি (LHS.getCategory() == fcZero) {
    Out = RHS;
    প্রত্যাগম opOK;
  }
  যদি (RHS.getCategory() == fcZero) {
    Out = LHS;
    প্রত্যাগম opOK;
  }
  যদি (LHS.getCategory() == fcInfinity && RHS.getCategory() == fcInfinity &&
      LHS.isNegative() != RHS.isNegative()) {
    Out.makeNaN(অসত্য, Out.isNegative(), nullptr);
    প্রত্যাগম opInvalidOp;
  }
  যদি (LHS.getCategory() == fcInfinity) {
    Out = LHS;
    প্রত্যাগম opOK;
  }
  যদি (RHS.getCategory() == fcInfinity) {
    Out = RHS;
    প্রত্যাগম opOK;
  }
  প্রতিজ্ঞা(LHS.getCategory() == fcNormal && RHS.getCategory() == fcNormal);

  APFloat A(LHS.Floats[0]), AA(LHS.Floats[1]), C(RHS.Floats[0]),
      CC(RHS.Floats[1]);
  প্রতিজ্ঞা(&A.getSemantics() == &APFloatBase::semIEEEdouble);
  প্রতিজ্ঞা(&AA.getSemantics() == &APFloatBase::semIEEEdouble);
  প্রতিজ্ঞা(&C.getSemantics() == &APFloatBase::semIEEEdouble);
  প্রতিজ্ঞা(&CC.getSemantics() == &APFloatBase::semIEEEdouble);
  প্রতিজ্ঞা(&Out.Floats[0].getSemantics() == &APFloatBase::semIEEEdouble);
  প্রতিজ্ঞা(&Out.Floats[1].getSemantics() == &APFloatBase::semIEEEdouble);
  প্রত্যাগম Out.addImpl(A, AA, C, CC, RM);
}

APFloat::opStatus DoubleAPFloat::add(ধ্রুব DoubleAPFloat &RHS,
                                     roundingMode RM) {
  প্রত্যাগম addWithSpecial(*এতত্, RHS, *এতত্, RM);
}

APFloat::opStatus DoubleAPFloat::subtract(ধ্রুব DoubleAPFloat &RHS,
                                          roundingMode RM) {
  changeSign();
  auto Ret = add(RHS, RM);
  changeSign();
  প্রত্যাগম Ret;
}

APFloat::opStatus DoubleAPFloat::multiply(ধ্রুব DoubleAPFloat &RHS,
                                          APFloat::roundingMode RM) {
  ধ্রুব auto &LHS = *এতত্;
  auto &Out = *এতত্;
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
  যদি (LHS.getCategory() == fcNaN) {
    Out = LHS;
    প্রত্যাগম opOK;
  }
  যদি (RHS.getCategory() == fcNaN) {
    Out = RHS;
    প্রত্যাগম opOK;
  }
  যদি ((LHS.getCategory() == fcZero && RHS.getCategory() == fcInfinity) ||
      (LHS.getCategory() == fcInfinity && RHS.getCategory() == fcZero)) {
    Out.makeNaN(অসত্য, অসত্য, nullptr);
    প্রত্যাগম opOK;
  }
  যদি (LHS.getCategory() == fcZero || LHS.getCategory() == fcInfinity) {
    Out = LHS;
    প্রত্যাগম opOK;
  }
  যদি (RHS.getCategory() == fcZero || RHS.getCategory() == fcInfinity) {
    Out = RHS;
    প্রত্যাগম opOK;
  }
  প্রতিজ্ঞা(LHS.getCategory() == fcNormal && RHS.getCategory() == fcNormal &&
         "Special cases not handled exhaustively");

  পূর্ণাঙ্ক Status = opOK;
  APFloat A = Floats[0], B = Floats[1], C = RHS.Floats[0], D = RHS.Floats[1];
  // t = a * c
  APFloat T = A;
  Status |= T.multiply(C, RM);
  যদি (!T.isFiniteNonZero()) {
    Floats[0] = std::move(T);
    Floats[1].makeZero(/* Neg = */ অসত্য);
    প্রত্যাগম (opStatus)Status;
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
  যদি (!U.isFinite()) {
    Floats[1].makeZero(/* Neg = */ অসত্য);
  } অন্যথা {
    // Floats[1] = (t - u) + tau
    Status |= T.subtract(U, RM);
    Status |= T.add(Tau, RM);
    Floats[1] = std::move(T);
  }
  প্রত্যাগম (opStatus)Status;
}

APFloat::opStatus DoubleAPFloat::divide(ধ্রুব DoubleAPFloat &RHS,
                                        APFloat::roundingMode RM) {
  প্রতিজ্ঞা(Semantics == &APFloatBase::semPPCDoubleDouble &&
         "Unexpected Semantics");
  APFloat Tmp(APFloatBase::semPPCDoubleDoubleLegacy, bitcastToAPInt());
  auto Ret = Tmp.divide(
      APFloat(APFloatBase::semPPCDoubleDoubleLegacy, RHS.bitcastToAPInt()), RM);
  *এতত্ = DoubleAPFloat(APFloatBase::semPPCDoubleDouble, Tmp.bitcastToAPInt());
  প্রত্যাগম Ret;
}

APFloat::opStatus DoubleAPFloat::remainder(ধ্রুব DoubleAPFloat &RHS) {
  প্রতিজ্ঞা(Semantics == &APFloatBase::semPPCDoubleDouble &&
         "Unexpected Semantics");
  APFloat Tmp(APFloatBase::semPPCDoubleDoubleLegacy, bitcastToAPInt());
  auto Ret = Tmp.remainder(
      APFloat(APFloatBase::semPPCDoubleDoubleLegacy, RHS.bitcastToAPInt()));
  *এতত্ = DoubleAPFloat(APFloatBase::semPPCDoubleDouble, Tmp.bitcastToAPInt());
  প্রত্যাগম Ret;
}

APFloat::opStatus DoubleAPFloat::mod(ধ্রুব DoubleAPFloat &RHS) {
  প্রতিজ্ঞা(Semantics == &APFloatBase::semPPCDoubleDouble &&
         "Unexpected Semantics");
  APFloat Tmp(APFloatBase::semPPCDoubleDoubleLegacy, bitcastToAPInt());
  auto Ret = Tmp.mod(
      APFloat(APFloatBase::semPPCDoubleDoubleLegacy, RHS.bitcastToAPInt()));
  *এতত্ = DoubleAPFloat(APFloatBase::semPPCDoubleDouble, Tmp.bitcastToAPInt());
  প্রত্যাগম Ret;
}

APFloat::opStatus
DoubleAPFloat::fusedMultiplyAdd(ধ্রুব DoubleAPFloat &Multiplicand,
                                ধ্রুব DoubleAPFloat &Addend,
                                APFloat::roundingMode RM) {
  প্রতিজ্ঞা(Semantics == &APFloatBase::semPPCDoubleDouble &&
         "Unexpected Semantics");
  APFloat Tmp(APFloatBase::semPPCDoubleDoubleLegacy, bitcastToAPInt());
  auto Ret = Tmp.fusedMultiplyAdd(
      APFloat(APFloatBase::semPPCDoubleDoubleLegacy,
              Multiplicand.bitcastToAPInt()),
      APFloat(APFloatBase::semPPCDoubleDoubleLegacy, Addend.bitcastToAPInt()),
      RM);
  *এতত্ = DoubleAPFloat(APFloatBase::semPPCDoubleDouble, Tmp.bitcastToAPInt());
  প্রত্যাগম Ret;
}

APFloat::opStatus DoubleAPFloat::roundToIntegral(APFloat::roundingMode RM) {
  প্রতিজ্ঞা(Semantics == &APFloatBase::semPPCDoubleDouble &&
         "Unexpected Semantics");
  ধ্রুব APFloat &Hi = getFirst();
  ধ্রুব APFloat &Lo = getSecond();

  APFloat RoundedHi = Hi;
  ধ্রুব opStatus HiStatus = RoundedHi.roundToIntegral(RM);

  // We can reduce the problem to just the high part if the input:
  // 1. Represents a non-finite value.
  // 2. Has a component which is zero.
  যদি (!Hi.isFiniteNonZero() || Lo.isZero()) {
    Floats[0] = std::move(RoundedHi);
    Floats[1].makeZero(/*Neg=*/অসত্য);
    প্রত্যাগম HiStatus;
  }

  // Adjust `Rounded` in the direction of `TieBreaker` if `ToRound` was at a
  // halfway point.
  auto RoundToNearestHelper = [](APFloat ToRound, APFloat Rounded,
                                 APFloat TieBreaker) {
    // RoundingError tells us which direction we rounded:
    //   - RoundingError > 0: we rounded up.
    //   - RoundingError < 0: we rounded down.
    // Sterbenz' lemma ensures that RoundingError is exact.
    ধ্রুব APFloat RoundingError = Rounded - ToRound;
    যদি (TieBreaker.isNonZero() &&
        TieBreaker.isNegative() != RoundingError.isNegative() &&
        abs(RoundingError).is_Exactly_মান(0.5))
      Rounded.add(
          APFloat::getOne(Rounded.getSemantics(), TieBreaker.isNegative()),
          rmNearestTiesToEven);
    প্রত্যাগম Rounded;
  };

  // Case 1: Hi is not an integer.
  // Special cases are for rounding modes that are sensitive to ties.
  যদি (RoundedHi != Hi) {
    // We need to consider the case where Hi was between two integers and the
    // rounding mode broke the tie when, in fact, Lo may have had a different
    // sign than Hi.
    যদি (RM == rmNearestTiesToAway || RM == rmNearestTiesToEven)
      RoundedHi = RoundToNearestHelper(Hi, RoundedHi, Lo);

    Floats[0] = std::move(RoundedHi);
    Floats[1].makeZero(/*Neg=*/অসত্য);
    প্রত্যাগম HiStatus;
  }

  // Case 2: Hi is an integer.
  // Special cases are for rounding modes which are rounding towards or away from zero.
  RoundingMode LoRoundingMode;
  যদি (RM == rmTowardZero)
    // When our input is positive, we want the Lo component rounded toward
    // negative infinity to get the smallest result magnitude. Likewise,
    // negative inputs want the Lo component rounded toward positive infinity.
    LoRoundingMode = isNegative() ? rmTowardPositive : rmTowardNegative;
  অন্যথা
    LoRoundingMode = RM;

  APFloat RoundedLo = Lo;
  ধ্রুব opStatus LoStatus = RoundedLo.roundToIntegral(LoRoundingMode);
  যদি (LoRoundingMode == rmNearestTiesToAway)
    // We need to consider the case where Lo was between two integers and the
    // rounding mode broke the tie when, in fact, Hi may have had a different
    // sign than Lo.
    RoundedLo = RoundToNearestHelper(Lo, RoundedLo, Hi);

  // We must ensure that the final result has no overlap between the two APFloat values.
  std::tie(RoundedHi, RoundedLo) = fastTwoSum(RoundedHi, RoundedLo);

  Floats[0] = std::move(RoundedHi);
  Floats[1] = std::move(RoundedLo);
  প্রত্যাগম LoStatus;
}

শূন্য DoubleAPFloat::changeSign() {
  Floats[0].changeSign();
  Floats[1].changeSign();
}

APFloat::cmpResult
DoubleAPFloat::compare_Absolute_মান(ধ্রুব DoubleAPFloat &RHS) ধ্রুব {
  // Compare absolute values of the high parts.
  ধ্রুব cmpResult HiPartCmp = Floats[0].compare_Absolute_মান(RHS.Floats[0]);
  যদি (HiPartCmp != cmpEqual)
    প্রত্যাগম HiPartCmp;

  // Zero, regardless of sign, is equal.
  যদি (Floats[1].isZero() && RHS.Floats[1].isZero())
    প্রত্যাগম cmpEqual;

  // At this point, |this->Hi| == |RHS.Hi|.
  // The magnitude is |Hi+Lo| which is Hi+|Lo| if signs of Hi and Lo are the
  // same, and Hi-|Lo| if signs are different.
  ধ্রুব তার্কিক ThisIsSubtractive =
      Floats[0].isNegative() != Floats[1].isNegative();
  ধ্রুব তার্কিক RHSIsSubtractive =
      RHS.Floats[0].isNegative() != RHS.Floats[1].isNegative();

  // Case 1: The low part of 'this' is zero.
  যদি (Floats[1].isZero())
    // We are comparing |Hi| vs. |Hi| ± |RHS.Lo|.
    // If RHS is subtractive, its magnitude is smaller.
    // If RHS is additive, its magnitude is larger.
    প্রত্যাগম RHSIsSubtractive ? cmpGreaterThan : cmpLessThan;

  // Case 2: The low part of 'RHS' is zero (and we know 'this' is not).
  যদি (RHS.Floats[1].isZero())
    // We are comparing |Hi| ± |This.Lo| vs. |Hi|.
    // If 'this' is subtractive, its magnitude is smaller.
    // If 'this' is additive, its magnitude is larger.
    প্রত্যাগম ThisIsSubtractive ? cmpLessThan : cmpGreaterThan;

  // If their natures differ, the additive one is larger.
  যদি (ThisIsSubtractive != RHSIsSubtractive)
    প্রত্যাগম ThisIsSubtractive ? cmpLessThan : cmpGreaterThan;

  // Case 3: Both are additive (Hi+|Lo|) or both are subtractive (Hi-|Lo|).
  // The comparison now depends on the magnitude of the low parts.
  ধ্রুব cmpResult LoPartCmp = Floats[1].compare_Absolute_মান(RHS.Floats[1]);

  যদি (ThisIsSubtractive) {
    // Both are subtractive (Hi-|Lo|), so the comparison of |Lo| is inverted.
    যদি (LoPartCmp == cmpLessThan)
      প্রত্যাগম cmpGreaterThan;
    যদি (LoPartCmp == cmpGreaterThan)
      প্রত্যাগম cmpLessThan;
  }

  // If additive, the comparison of |Lo| is direct.
  // If equal, they are equal.
  প্রত্যাগম LoPartCmp;
}

APFloat::fltCategory DoubleAPFloat::getCategory() ধ্রুব {
  প্রত্যাগম Floats[0].getCategory();
}

তার্কিক DoubleAPFloat::isNegative() ধ্রুব { প্রত্যাগম Floats[0].isNegative(); }

শূন্য DoubleAPFloat::makeInf(তার্কিক Neg) {
  Floats[0].makeInf(Neg);
  Floats[1].makeZero(/* Neg = */ অসত্য);
}

শূন্য DoubleAPFloat::makeZero(তার্কিক Neg) {
  Floats[0].makeZero(Neg);
  Floats[1].makeZero(/* Neg = */ অসত্য);
}

শূন্য DoubleAPFloat::makeLargest(তার্কিক Neg) {
  প্রতিজ্ঞা(Semantics == &APFloatBase::semPPCDoubleDouble &&
         "Unexpected Semantics");
  Floats[0] =
      APFloat(APFloatBase::semIEEEdouble, APInt(64, 0x7fefffffffffffffull));
  Floats[1] =
      APFloat(APFloatBase::semIEEEdouble, APInt(64, 0x7c8ffffffffffffeull));
  যদি (Neg)
    changeSign();
}

শূন্য DoubleAPFloat::makeSmallest(তার্কিক Neg) {
  প্রতিজ্ঞা(Semantics == &APFloatBase::semPPCDoubleDouble &&
         "Unexpected Semantics");
  Floats[0].makeSmallest(Neg);
  Floats[1].makeZero(/* Neg = */ অসত্য);
}

শূন্য DoubleAPFloat::makeSmallestNormalized(তার্কিক Neg) {
  প্রতিজ্ঞা(Semantics == &APFloatBase::semPPCDoubleDouble &&
         "Unexpected Semantics");
  Floats[0] =
      APFloat(APFloatBase::semIEEEdouble, APInt(64, 0x0360000000000000ull));
  যদি (Neg)
    Floats[0].changeSign();
  Floats[1].makeZero(/* Neg = */ অসত্য);
}

শূন্য DoubleAPFloat::makeNaN(তার্কিক SNaN, তার্কিক Neg, ধ্রুব APInt *fill) {
  Floats[0].makeNaN(SNaN, Neg, fill);
  Floats[1].makeZero(/* Neg = */ অসত্য);
}

APFloat::cmpResult DoubleAPFloat::compare(ধ্রুব DoubleAPFloat &RHS) ধ্রুব {
  auto Result = Floats[0].compare(RHS.Floats[0]);
  // |Float[0]| > |Float[1]|
  যদি (Result == APFloat::cmpEqual)
    প্রত্যাগম Floats[1].compare(RHS.Floats[1]);
  প্রত্যাগম Result;
}

তার্কিক DoubleAPFloat::bitwiseIsEqual(ধ্রুব DoubleAPFloat &RHS) ধ্রুব {
  প্রত্যাগম Floats[0].bitwiseIsEqual(RHS.Floats[0]) &&
         Floats[1].bitwiseIsEqual(RHS.Floats[1]);
}

hash_code hash_মান(ধ্রুব DoubleAPFloat &Arg) {
  যদি (Arg.Floats)
    প্রত্যাগম hash_combine(hash_মান(Arg.Floats[0]), hash_মান(Arg.Floats[1]));
  প্রত্যাগম hash_combine(Arg.Semantics);
}

APInt DoubleAPFloat::bitcastToAPInt() ধ্রুব {
  প্রতিজ্ঞা(Semantics == &APFloatBase::semPPCDoubleDouble &&
         "Unexpected Semantics");
  uint64_t Data[] = {
      Floats[0].bitcastToAPInt().getRawData()[0],
      Floats[1].bitcastToAPInt().getRawData()[0],
  };
  প্রত্যাগম APInt(128, Data);
}

Expected<APFloat::opStatus> DoubleAPFloat::convertFromString(StringRef S,
                                                             roundingMode RM) {
  প্রতিজ্ঞা(Semantics == &APFloatBase::semPPCDoubleDouble &&
         "Unexpected Semantics");
  APFloat Tmp(APFloatBase::semPPCDoubleDoubleLegacy);
  auto Ret = Tmp.convertFromString(S, RM);
  *এতত্ = DoubleAPFloat(APFloatBase::semPPCDoubleDouble, Tmp.bitcastToAPInt());
  প্রত্যাগম Ret;
}

// The double-double lattice of values corresponds to numbers which obey:
// - abs(lo) <= 1/2 * ulp(hi)
// - roundTiesToEven(hi + lo) == hi
//
// nextUp must choose the smallest output > input that follows these rules.
// nexDown must choose the largest output < input that follows these rules.
APFloat::opStatus DoubleAPFloat::next(তার্কিক nextDown) {
  প্রতিজ্ঞা(Semantics == &APFloatBase::semPPCDoubleDouble &&
         "Unexpected Semantics");
  // nextDown(x) = -nextUp(-x)
  যদি (nextDown) {
    changeSign();
    APFloat::opStatus Result = next(/*nextDown=*/অসত্য);
    changeSign();
    প্রত্যাগম Result;
  }
  বিকল্প (getCategory()) {
  পক্ষ fcInfinity:
    // nextUp(+inf) = +inf
    // nextUp(-inf) = -getLargest()
    যদি (isNegative())
      makeLargest(সত্য);
    প্রত্যাগম opOK;

  পক্ষ fcNaN:
    // IEEE-754R 2008 6.2 Par 2: nextUp(sNaN) = qNaN. Set Invalid flag.
    // IEEE-754R 2008 6.2: nextUp(qNaN) = qNaN. Must be identity so we do not
    //                     change the payload.
    যদি (getFirst().isSignaling()) {
      // For consistency, propagate the sign of the sNaN to the qNaN.
      makeNaN(অসত্য, isNegative(), nullptr);
      প্রত্যাগম opInvalidOp;
    }
    প্রত্যাগম opOK;

  পক্ষ fcZero:
    // nextUp(pm 0) = +getSmallest()
    makeSmallest(অসত্য);
    প্রত্যাগম opOK;

  পক্ষ fcNormal:
    ভঙ্গ;
  }

  ধ্রুব APFloat &HiOld = getFirst();
  ধ্রুব APFloat &LoOld = getSecond();

  APFloat NextLo = LoOld;
  NextLo.next(/*nextDown=*/অসত্য);

  // We want to admit values where:
  // 1. abs(Lo) <= ulp(Hi)/2
  // 2. Hi == RTNE(Hi + lo)
  auto InLattice = [](ধ্রুব APFloat &Hi, ধ্রুব APFloat &Lo) {
    প্রত্যাগম Hi + Lo == Hi;
  };

  // Check if (HiOld, nextUp(LoOld) is in the lattice.
  যদি (InLattice(HiOld, NextLo)) {
    // Yes, the result is (HiOld, nextUp(LoOld)).
    Floats[1] = std::move(NextLo);

    // TODO: Because we currently rely on semPPCDoubleDoubleLegacy, our maximum
    // value is defined to have exactly 106 bits of precision. This limitation
    // results in semPPCDoubleDouble being unable to reach its maximum canonical
    // value.
    DoubleAPFloat Largest{*Semantics, uninitialized};
    Largest.makeLargest(/*Neg=*/অসত্য);
    যদি (compare(Largest) == cmpGreaterThan)
      makeInf(/*Neg=*/অসত্য);

    প্রত্যাগম opOK;
  }

  // Now we need to handle the cases where (HiOld, nextUp(LoOld)) is not the
  // correct result. We know the new hi component will be nextUp(HiOld) but our
  // lattice rules make it a little ambiguous what the correct NextLo must be.
  APFloat NextHi = HiOld;
  NextHi.next(/*nextDown=*/অসত্য);

  // nextUp(getLargest()) == INFINITY
  যদি (NextHi.isInfinity()) {
    makeInf(/*Neg=*/অসত্য);
    প্রত্যাগম opOK;
  }

  // IEEE 754-2019 5.3.1:
  // "If x is the negative number of least magnitude in x's format, nextUp(x) is
  // -0."
  যদি (NextHi.isZero()) {
    makeZero(/*Neg=*/সত্য);
    প্রত্যাগম opOK;
  }

  // abs(NextLo) must be <= ulp(NextHi)/2. We want NextLo to be as close to
  // negative infinity as possible.
  NextLo = neg(scalbn(harrisonUlp(NextHi), -1, rmTowardZero));
  যদি (!InLattice(NextHi, NextLo))
    // RTNE may mean that Lo must be < ulp(NextHi) / 2 so we bump NextLo.
    NextLo.next(/*nextDown=*/অসত্য);

  Floats[0] = std::move(NextHi);
  Floats[1] = std::move(NextLo);

  প্রত্যাগম opOK;
}

APFloat::opStatus DoubleAPFloat::convertToSignExtendedInteger(
    MutableArrayRef<integerPart> প্রবেশ, unsigned পূর্ণাঙ্ক Width, তার্কিক IsSigned,
    roundingMode RM, তার্কিক *IsExact) ধ্রুব {
  প্রতিজ্ঞা(Semantics == &APFloatBase::semPPCDoubleDouble &&
         "Unexpected Semantics");

  // If Hi is not finite, or Lo is zero, the value is entirely represented
  // by Hi. Delegate to the simpler single-APFloat conversion.
  যদি (!getFirst().isFiniteNonZero() || getSecond().isZero())
    প্রত্যাগম getFirst().convertToInteger(প্রবেশ, Width, IsSigned, RM, IsExact);

  // First, round the full double-double value to an integral value. This
  // simplifies the rest of the function, as we no longer need to consider
  // fractional parts.
  *IsExact = অসত্য;
  DoubleAPFloat Integral = *এতত্;
  ধ্রুব opStatus RoundStatus = Integral.roundToIntegral(RM);
  যদি (RoundStatus == opInvalidOp)
    প্রত্যাগম opInvalidOp;
  ধ্রুব APFloat &IntegralHi = Integral.getFirst();
  ধ্রুব APFloat &IntegralLo = Integral.getSecond();

  // If rounding results in either component being zero, the sum is trivial.
  // Delegate to the simpler single-APFloat conversion.
  তার্কিক HiIsExact;
  যদি (IntegralHi.isZero() || IntegralLo.isZero()) {
    ধ্রুব opStatus HiStatus =
        IntegralHi.convertToInteger(প্রবেশ, Width, IsSigned, RM, &HiIsExact);
    // The conversion from an integer-valued float to an APInt may fail if the
    // result would be out of range.  Regardless, taking this path is only
    // possible if rounding occurred during the initial `roundToIntegral`.
    প্রত্যাগম HiStatus == opOK ? opInexact : HiStatus;
  }

  // A negative number cannot be represented by an unsigned integer.
  // Since a double-double is canonical, if Hi is negative, the sum is negative.
  যদি (!IsSigned && IntegralHi.isNegative())
    প্রত্যাগম opInvalidOp;

  // Handle the special boundary case where |Hi| is exactly the power of two
  // that marks the edge of the integer's range (e.g., 2^63 for int64_t). In
  // this situation, Hi itself won't fit, but the sum Hi + Lo might.
  // `PositiveOverflowWidth` is the bit number for this boundary (N-1 for
  // signed, N for unsigned).
  তার্কিক LoIsExact;
  ধ্রুব পূর্ণাঙ্ক HiExactLog2 = IntegralHi.getExactLog2Abs();
  ধ্রুব unsigned PositiveOverflowWidth = IsSigned ? Width - 1 : Width;
  যদি (HiExactLog2 >= 0 &&
      static_cast<unsigned>(HiExactLog2) == PositiveOverflowWidth) {
    // If Hi and Lo have the same sign, |Hi + Lo| > |Hi|, so the sum is
    // guaranteed to overflow. E.g., for uint128_t, (2^128, 1) overflows.
    যদি (IntegralHi.isNegative() == IntegralLo.isNegative())
      প্রত্যাগম opInvalidOp;

    // If the signs differ, the sum will fit. We can compute the result using
    // properties of two's complement arithmetic without a wide intermediate
    // integer. E.g., for uint128_t, (2^128, -1) should be 2^128 - 1.
    ধ্রুব opStatus LoStatus = IntegralLo.convertToInteger(
        প্রবেশ, Width, /*IsSigned=*/সত্য, RM, &LoIsExact);
    যদি (LoStatus == opInvalidOp)
      প্রত্যাগম opInvalidOp;

    // Adjust the bit pattern of Lo to account for Hi's value:
    //  - For unsigned (Hi=2^Width): `2^Width + Lo` in `Width`-bit
    //    arithmetic is equivalent to just `Lo`. The conversion of `Lo` above
    //    already produced the correct final bit pattern.
    //  - For signed (Hi=2^(Width-1)): The sum `2^(Width-1) + Lo` (where Lo<0)
    //    can be computed by taking the two's complement pattern for `Lo` and
    //    clearing the sign bit.
    যদি (IsSigned && !IntegralHi.isNegative())
      APInt::tcClearBit(প্রবেশ.data(), PositiveOverflowWidth);
    *IsExact = RoundStatus == opOK;
    প্রত্যাগম RoundStatus;
  }

  // Convert Hi into an integer.  This may not fit but that is OK: we know that
  // Hi + Lo would not fit either in this situation.
  ধ্রুব opStatus HiStatus = IntegralHi.convertToInteger(
      প্রবেশ, Width, IsSigned, rmTowardZero, &HiIsExact);
  যদি (HiStatus == opInvalidOp)
    প্রত্যাগম HiStatus;

  // Convert Lo into a temporary integer of the same width.
  APSInt LoResult{Width, /*isUnsigned=*/!IsSigned};
  ধ্রুব opStatus LoStatus =
      IntegralLo.convertToInteger(LoResult, rmTowardZero, &LoIsExact);
  যদি (LoStatus == opInvalidOp)
    প্রত্যাগম LoStatus;

  // Add Lo to Hi. This addition is guaranteed not to overflow because of the
  // double-double canonicalization rule (`|Lo| <= ulp(Hi)/2`). The only case
  // where the sum could cross the integer type's boundary is when Hi is a
  // power of two, which is handled by the special case block above.
  APInt::tcAdd(প্রবেশ.data(), LoResult.getRawData(), /*carry=*/0, প্রবেশ.size());

  *IsExact = RoundStatus == opOK;
  প্রত্যাগম RoundStatus;
}

APFloat::opStatus
DoubleAPFloat::convertToInteger(MutableArrayRef<integerPart> প্রবেশ,
                                unsigned পূর্ণাঙ্ক Width, তার্কিক IsSigned,
                                roundingMode RM, তার্কিক *IsExact) ধ্রুব {
  opStatus FS =
      convertToSignExtendedInteger(প্রবেশ, Width, IsSigned, RM, IsExact);

  যদি (FS == opInvalidOp) {
    ধ্রুব unsigned Dst_Parts_গুনতি = part_গুনতি_For_Bits(Width);
    প্রতিজ্ঞা(Dst_Parts_গুনতি <= প্রবেশ.size() && "Integer too big");

    unsigned Bits;
    যদি (getCategory() == fcNaN)
      Bits = 0;
    অন্যথা যদি (isNegative())
      Bits = IsSigned;
    অন্যথা
      Bits = Width - IsSigned;

    tcSetLeastSignificantBits(প্রবেশ.data(), Dst_Parts_গুনতি, Bits);
    যদি (isNegative() && IsSigned)
      APInt::tc_Shift_বাম(প্রবেশ.data(), Dst_Parts_গুনতি, Width - 1);
  }

  প্রত্যাগম FS;
}

APFloat::opStatus DoubleAPFloat::handleOverflow(roundingMode RM) {
  বিকল্প (RM) {
  পক্ষ APFloat::rmTowardZero:
    makeLargest(/*Neg=*/isNegative());
    ভঙ্গ;
  পক্ষ APFloat::rmTowardNegative:
    যদি (isNegative())
      makeInf(/*Neg=*/সত্য);
    অন্যথা
      makeLargest(/*Neg=*/অসত্য);
    ভঙ্গ;
  পক্ষ APFloat::rmTowardPositive:
    যদি (isNegative())
      makeLargest(/*Neg=*/সত্য);
    অন্যথা
      makeInf(/*Neg=*/অসত্য);
    ভঙ্গ;
  পক্ষ APFloat::rmNearestTiesToAway:
  পক্ষ APFloat::rmNearestTiesToEven:
    makeInf(/*Neg=*/isNegative());
    ভঙ্গ;
  স্বভাব:
    llvm_unreachable("Invalid rounding mode found");
  }
  opStatus S = opInexact;
  যদি (!getFirst().isFinite())
    S = static_cast<opStatus>(S | opOverflow);
  প্রত্যাগম S;
}

APFloat::opStatus DoubleAPFloat::convertFromUnsignedParts(
    ধ্রুব integerPart *Src, unsigned পূর্ণাঙ্ক Src_গুনতি, roundingMode RM) {
  // Find the most significant bit of the source integer. APInt::tcMSB returns
  // UINT_MAX for a zero value.
  ধ্রুব unsigned SrcMSB = APInt::tcMSB(Src, Src_গুনতি);
  যদি (SrcMSB == UINT_MAX) {
    // The source integer is 0.
    makeZero(/*Neg=*/অসত্য);
    প্রত্যাগম opOK;
  }

  // Create a minimally-sized APInt to represent the source value.
  ধ্রুব unsigned SrcBitWidth = SrcMSB + 1;
  APSInt SrcInt{APInt{/*numBits=*/SrcBitWidth, ArrayRef(Src, Src_গুনতি)},
                /*isUnsigned=*/সত্য};

  // Stage 1: Initial Approximation.
  // Convert the source integer SrcInt to the Hi part of the DoubleAPFloat.
  // We use round-to-nearest because it minimizes the initial error, which is
  // crucial for the subsequent steps.
  APFloat Hi{getFirst().getSemantics()};
  Hi.convertFromAPInt(SrcInt, /*IsSigned=*/অসত্য, rmNearestTiesToEven);

  // If the first approximation already overflows, the number is too large.
  // NOTE: The underlying semantics are *more* conservative when choosing to
  // overflow because their notion of ULP is much larger. As such, it is always
  // safe to overflow at the DoubleAPFloat level if the APFloat overflows.
  যদি (!Hi.isFinite())
    প্রত্যাগম handleOverflow(RM);

  // Stage 2: Exact Error Calculation.
  // Calculate the exact error of the first approximation: Error = SrcInt - Hi.
  // This is done by converting Hi back to an integer and subtracting it from
  // the original source.
  তার্কিক HiAsIntIsExact;
  // Create an integer representation of Hi. Its width is determined by the
  // exponent of Hi, ensuring it's just large enough. This width can exceed
  // SrcBitWidth if the conversion to Hi rounded up to a power of two.
  // accurately when converted back to an integer.
  APSInt HiAsInt{static_cast<uint32_t>(ilogb(Hi) + 1), /*isUnsigned=*/সত্য};
  Hi.convertToInteger(HiAsInt, rmNearestTiesToEven, &HiAsIntIsExact);
  ধ্রুব APInt Error = SrcInt.zext(HiAsInt.getBitWidth()) - HiAsInt;

  // Stage 3: Error Approximation and Rounding.
  // Convert the integer error into the Lo part of the DoubleAPFloat. This step
  // captures the remainder of the original number. The rounding mode for this
  // conversion (LoRM) may need to be adjusted from the user-requested RM to
  // ensure the final sum (Hi + Lo) rounds correctly.
  roundingMode LoRM = RM;
  // Adjustments are only necessary when the initial approximation Hi was an
  // overestimate, making the Error negative.
  যদি (Error.isNegative()) {
    যদি (RM == rmNearestTiesToAway) {
      // For rmNearestTiesToAway, a tie should round away from zero. Since
      // SrcInt is positive, this means rounding toward +infinity.
      // A standard conversion of a negative Error would round ties toward
      // -infinity, causing the final sum Hi + Lo to be smaller. To
      // counteract this, we detect the tie case and override the rounding
      // mode for Lo to rmTowardPositive.
      ধ্রুব unsigned ErrorActiveBits = Error.getSignificantBits() - 1;
      ধ্রুব unsigned LoPrecision = getSecond().getSemantics().precision;
      যদি (ErrorActiveBits > LoPrecision) {
        ধ্রুব unsigned RoundingBoundary = ErrorActiveBits - LoPrecision;
        // A tie occurs when the bits to be truncated are of the form 100...0.
        // This is detected by checking if the number of trailing zeros is
        // exactly one less than the number of bits being truncated.
        যদি (Error.গুনতি_Trailing_Zeros() == RoundingBoundary - 1)
          LoRM = rmTowardPositive;
      }
    } অন্যথা যদি (RM == rmTowardZero) {
      // For rmTowardZero, the final positive result must be truncated (rounded
      // down). When Hi is an overestimate, Error is negative. A standard
      // rmTowardZero conversion of Error would make it *less* negative,
      // effectively rounding the final sum Hi + Lo *up*. To ensure the sum
      // rounds down correctly, we force Lo to round toward -infinity.
      LoRM = rmTowardNegative;
    }
  }

  APFloat Lo{getSecond().getSemantics()};
  opStatus Status = Lo.convertFromAPInt(Error, /*IsSigned=*/সত্য, LoRM);

  // Renormalize the pair (Hi, Lo) into a canonical DoubleAPFloat form where the
  // components do not overlap. fastTwoSum performs this operation.
  std::tie(Hi, Lo) = fastTwoSum(Hi, Lo);
  Floats[0] = std::move(Hi);
  Floats[1] = std::move(Lo);

  // A final check for overflow is needed because fastTwoSum can cause a
  // carry-out from Lo that pushes Hi to infinity.
  যদি (!getFirst().isFinite())
    প্রত্যাগম handleOverflow(RM);

  // The largest DoubleAPFloat must be canonical. Values which are larger are
  // not canonical and are equivalent to overflow.
  যদি (getFirst().isFiniteNonZero() && Floats[0].isLargest()) {
    DoubleAPFloat Largest{*Semantics};
    Largest.makeLargest(/*Neg=*/অসত্য);
    যদি (compare(Largest) == APFloat::cmpGreaterThan)
      প্রত্যাগম handleOverflow(RM);
  }

  // The final status of the operation is determined by the conversion of the
  // error term. If Lo could represent Error exactly, the entire conversion
  // is exact. Otherwise, it's inexact.
  প্রত্যাগম Status;
}

APFloat::opStatus DoubleAPFloat::convertFromAPInt(ধ্রুব APInt &প্রবেশ,
                                                  তার্কিক IsSigned,
                                                  roundingMode RM) {
  ধ্রুব তার্কিক Negate_প্রবেশ = IsSigned && প্রবেশ.isNegative();
  APInt API = প্রবেশ;
  যদি (Negate_প্রবেশ)
    API.negate();

  ধ্রুব APFloat::opStatus Status =
      convertFromUnsignedParts(API.getRawData(), API.get_অঙ্ক_শব্দগুলি(), RM);
  যদি (Negate_প্রবেশ)
    changeSign();
  প্রত্যাগম Status;
}

unsigned পূর্ণাঙ্ক DoubleAPFloat::convertToHexString(বর্ণ *DST,
                                               unsigned পূর্ণাঙ্ক HexDigits,
                                               তার্কিক UpperCase,
                                               roundingMode RM) ধ্রুব {
  প্রতিজ্ঞা(Semantics == &APFloatBase::semPPCDoubleDouble &&
         "Unexpected Semantics");
  প্রত্যাগম APFloat(APFloatBase::semPPCDoubleDoubleLegacy, bitcastToAPInt())
      .convertToHexString(DST, HexDigits, UpperCase, RM);
}

তার্কিক DoubleAPFloat::isDenormal() ধ্রুব {
  প্রত্যাগম getCategory() == fcNormal &&
         (Floats[0].isDenormal() || Floats[1].isDenormal() ||
          // (double)(Hi + Lo) == Hi defines a normal number.
          Floats[0] != Floats[0] + Floats[1]);
}

তার্কিক DoubleAPFloat::isSmallest() ধ্রুব {
  যদি (getCategory() != fcNormal)
    প্রত্যাগম অসত্য;
  DoubleAPFloat Tmp(*এতত্);
  Tmp.makeSmallest(এতত্->isNegative());
  প্রত্যাগম Tmp.compare(*এতত্) == cmpEqual;
}

তার্কিক DoubleAPFloat::isSmallestNormalized() ধ্রুব {
  যদি (getCategory() != fcNormal)
    প্রত্যাগম অসত্য;

  DoubleAPFloat Tmp(*এতত্);
  Tmp.makeSmallestNormalized(এতত্->isNegative());
  প্রত্যাগম Tmp.compare(*এতত্) == cmpEqual;
}

তার্কিক DoubleAPFloat::isLargest() ধ্রুব {
  যদি (getCategory() != fcNormal)
    প্রত্যাগম অসত্য;
  DoubleAPFloat Tmp(*এতত্);
  Tmp.makeLargest(এতত্->isNegative());
  প্রত্যাগম Tmp.compare(*এতত্) == cmpEqual;
}

তার্কিক DoubleAPFloat::isInteger() ধ্রুব {
  প্রতিজ্ঞা(Semantics == &APFloatBase::semPPCDoubleDouble &&
         "Unexpected Semantics");
  প্রত্যাগম Floats[0].isInteger() && Floats[1].isInteger();
}

শূন্য DoubleAPFloat::toString(SmallVectorImpl<বর্ণ> &Str,
                             unsigned FormatPrecision,
                             unsigned FormatMaxPadding,
                             তার্কিক TruncateZero) ধ্রুব {
  প্রতিজ্ঞা(Semantics == &APFloatBase::semPPCDoubleDouble &&
         "Unexpected Semantics");
  APFloat(APFloatBase::semPPCDoubleDoubleLegacy, bitcastToAPInt())
      .toString(Str, FormatPrecision, FormatMaxPadding, TruncateZero);
}

পূর্ণাঙ্ক DoubleAPFloat::getExactLog2Abs() ধ্রুব {
  // In order for Hi + Lo to be a power of two, the following must be true:
  // 1. Hi must be a power of two.
  // 2. Lo must be zero.
  যদি (getSecond().isNonZero())
    প্রত্যাগম INT_MIN;
  প্রত্যাগম getFirst().getExactLog2Abs();
}

পূর্ণাঙ্ক ilogb(ধ্রুব DoubleAPFloat &Arg) {
  ধ্রুব APFloat &Hi = Arg.getFirst();
  ধ্রুব APFloat &Lo = Arg.getSecond();
  পূর্ণাঙ্ক IlogbResult = ilogb(Hi);
  // Zero and non-finite values can delegate to ilogb(Hi).
  যদি (Arg.getCategory() != fcNormal)
    প্রত্যাগম IlogbResult;
  // If Lo can't change the binade, we can delegate to ilogb(Hi).
  যদি (Lo.isZero() || Hi.isNegative() == Lo.isNegative())
    প্রত্যাগম IlogbResult;
  যদি (Hi.getExactLog2Abs() == INT_MIN)
    প্রত্যাগম IlogbResult;
  // Numbers of the form 2^a - 2^b or -2^a + 2^b are almost powers of two but
  // get nudged out of the binade by the low component.
  প্রত্যাগম IlogbResult - 1;
}

DoubleAPFloat scalbn(ধ্রুব DoubleAPFloat &Arg, পূর্ণাঙ্ক Exp,
                     APFloat::roundingMode RM) {
  প্রতিজ্ঞা(Arg.Semantics == &APFloatBase::PPCDoubleDouble() &&
         "Unexpected Semantics");
  প্রত্যাগম DoubleAPFloat(APFloatBase::PPCDoubleDouble(),
                       scalbn(Arg.Floats[0], Exp, RM),
                       scalbn(Arg.Floats[1], Exp, RM));
}

DoubleAPFloat frexp(ধ্রুব DoubleAPFloat &Arg, পূর্ণাঙ্ক &Exp,
                    APFloat::roundingMode RM) {
  প্রতিজ্ঞা(Arg.Semantics == &APFloatBase::PPCDoubleDouble() &&
         "Unexpected Semantics");

  // Get the unbiased exponent e of the number, where |Arg| = m * 2^e for m in
  // [1.0, 2.0).
  Exp = ilogb(Arg);

  // For NaNs, quiet any signaling NaN and return the result, as per standard
  // practice.
  যদি (Exp == APFloat::IEK_NaN) {
    DoubleAPFloat Quiet{Arg};
    Quiet.getFirst() = Quiet.getFirst().makeQuiet();
    প্রত্যাগম Quiet;
  }

  // For infinity, return it unchanged. The exponent remains IEK_Inf.
  যদি (Exp == APFloat::IEK_Inf)
    প্রত্যাগম Arg;

  // For zero, the fraction is zero and the standard requires the exponent be 0.
  যদি (Exp == APFloat::IEK_Zero) {
    Exp = 0;
    প্রত্যাগম Arg;
  }

  ধ্রুব APFloat &Hi = Arg.getFirst();
  ধ্রুব APFloat &Lo = Arg.getSecond();

  // frexp requires the fraction's absolute value to be in [0.5, 1.0).
  // ilogb provides an exponent for an absolute value in [1.0, 2.0).
  // Increment the exponent to ensure the fraction is in the correct range.
  ++Exp;

  ধ্রুব তার্কিক SignsDisagree = Hi.isNegative() != Lo.isNegative();
  APFloat Second = Lo;
  যদি (Arg.getCategory() == APFloat::fcNormal && Lo.isFiniteNonZero()) {
    roundingMode LoRoundingMode;
    // The interpretation of rmTowardZero depends on the sign of the combined
    // Arg rather than the sign of the component.
    যদি (RM == rmTowardZero)
      LoRoundingMode = Arg.isNegative() ? rmTowardPositive : rmTowardNegative;
    // For rmNearestTiesToAway, we face a similar problem. If signs disagree,
    // Lo is a correction *toward* zero relative to Hi. Rounding Lo
    // "away from zero" based on its own sign would move the value in the
    // wrong direction. As a safe proxy, we use rmNearestTiesToEven, which is
    // direction-agnostic. We only need to bother with this if Lo is scaled
    // down.
    অন্যথা যদি (RM == rmNearestTiesToAway && SignsDisagree && Exp > 0)
      LoRoundingMode = rmNearestTiesToEven;
    অন্যথা
      LoRoundingMode = RM;
    Second = scalbn(Lo, -Exp, LoRoundingMode);
    // The rmNearestTiesToEven proxy is correct most of the time, but it
    // differs from rmNearestTiesToAway when the scaled value of Lo is an
    // exact midpoint.
    // NOTE: This is morally equivalent to roundTiesTowardZero.
    যদি (RM == rmNearestTiesToAway && LoRoundingMode == rmNearestTiesToEven) {
      // Re-scale the result back to check if rounding occurred.
      ধ্রুব APFloat RecomposedLo = scalbn(Second, Exp, rmNearestTiesToEven);
      যদি (RecomposedLo != Lo) {
        // RoundingError tells us which direction we rounded:
        //   - RoundingError > 0: we rounded up.
        //   - RoundingError < 0: we down up.
        ধ্রুব APFloat RoundingError = RecomposedLo - Lo;
        // Determine if scalbn(Lo, -Exp) landed exactly on a midpoint.
        // We do this by checking if the absolute rounding error is exactly
        // half a ULP of the result.
        ধ্রুব APFloat UlpOfSecond = harrisonUlp(Second);
        ধ্রুব APFloat ScaledUlpOfSecond =
            scalbn(UlpOfSecond, Exp - 1, rmNearestTiesToEven);
        ধ্রুব তার্কিক IsMidpoint = abs(RoundingError) == ScaledUlpOfSecond;
        ধ্রুব তার্কিক RoundedLoAway =
            Second.isNegative() == RoundingError.isNegative();
        // The sign of Hi and Lo disagree and we rounded Lo away: we must
        // decrease the magnitude of Second to increase the magnitude
        // First+Second.
        যদি (IsMidpoint && RoundedLoAway)
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
    যদি (Second.isZero() && SignsDisagree && Hi.getExactLog2Abs() != INT_MIN)
      ++Exp;
  }

  APFloat First = scalbn(Hi, -Exp, RM);
  প্রত্যাগম DoubleAPFloat(APFloatBase::PPCDoubleDouble(), std::move(First),
                       std::move(Second));
}

APInt DoubleAPFloat::getNaNPayload() ধ্রুব { প্রত্যাগম Floats[0].getNaNPayload(); }
} // namespace detail

APFloat::Storage::Storage(IEEEFloat F, ধ্রুব fltSemantics &Semantics) {
  যদি (usesLayout<IEEEFloat>(Semantics)) {
    নূতন (&IEEE) IEEEFloat(std::move(F));
    প্রত্যাগম;
  }
  যদি (usesLayout<DoubleAPFloat>(Semantics)) {
    ধ্রুব fltSemantics& S = F.getSemantics();
    নূতন (&Double) DoubleAPFloat(Semantics, APFloat(std::move(F), S),
                                APFloat(APFloatBase::IEEEdouble()));
    প্রত্যাগম;
  }
  llvm_unreachable("Unexpected semantics");
}

Expected<APFloat::opStatus> APFloat::convertFromString(StringRef Str,
                                                       roundingMode RM) {
  APFLOAT_DISPATCH_ON_SEMANTICS(convertFromString(Str, RM));
}

hash_code hash_মান(ধ্রুব APFloat &Arg) {
  যদি (APFloat::usesLayout<detail::IEEEFloat>(Arg.getSemantics()))
    প্রত্যাগম hash_মান(Arg.U.IEEE);
  যদি (APFloat::usesLayout<detail::DoubleAPFloat>(Arg.getSemantics()))
    প্রত্যাগম hash_মান(Arg.U.Double);
  llvm_unreachable("Unexpected semantics");
}

APFloat::APFloat(ধ্রুব fltSemantics &Semantics, StringRef S)
    : APFloat(Semantics) {
  auto StatusOrErr = convertFromString(S, rmNearestTiesToEven);
  প্রতিজ্ঞা(StatusOrErr && "Invalid floating point representation");
  consumeError(StatusOrErr.takeError());
}

FPClassTest APFloat::classify() ধ্রুব {
  যদি (isZero())
    প্রত্যাগম isNegative() ? fcNegZero : fcPosZero;
  যদি (isNormal())
    প্রত্যাগম isNegative() ? fcNegNormal : fcPosNormal;
  যদি (isDenormal())
    প্রত্যাগম isNegative() ? fcNegSubnormal : fcPosSubnormal;
  যদি (isInfinity())
    প্রত্যাগম isNegative() ? fcNegInf : fcPosInf;
  প্রতিজ্ঞা(isNaN() && "Other class of FP constant");
  প্রত্যাগম isSignaling() ? fcSNan : fcQNan;
}

তার্কিক APFloat::getExactInverse(APFloat *Inv) ধ্রুব {
  // Only finite, non-zero numbers can have a useful, representable inverse.
  // This check filters out +/- zero, +/- infinity, and NaN.
  যদি (!isFiniteNonZero())
    প্রত্যাগম অসত্য;

  // Historically, this function rejects subnormal inputs.  One reason why this
  // might be important is that subnormals may behave differently under FTZ/DAZ
  // runtime behavior.
  যদি (isDenormal())
    প্রত্যাগম অসত্য;

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
  ধ্রুব পূর্ণাঙ্ক Exp = getExactLog2Abs();
  যদি (Exp == INT_MIN)
    প্রত্যাগম অসত্য;

  // The inverse of +/- 2^Exp is +/- 2^(-Exp). We can compute this by
  // scaling 1.0 by the negated exponent.
  APFloat Reciprocal =
      scalbn(APFloat::getOne(getSemantics(), /*Negative=*/isNegative()), -Exp,
             rmTowardZero);

  // scalbn might round if the resulting exponent -Exp is outside the
  // representable range, causing overflow (to infinity) or underflow. We
  // must verify that the result is still the exact power of two we expect.
  যদি (Reciprocal.getExactLog2Abs() != -Exp)
    প্রত্যাগম অসত্য;

  // Avoid multiplication with a subnormal, it is not safe on all platforms and
  // may be slower than a normal division.
  যদি (Reciprocal.isDenormal())
    প্রত্যাগম অসত্য;

  প্রতিজ্ঞা(Reciprocal.isFiniteNonZero());

  যদি (Inv)
    *Inv = std::move(Reciprocal);

  প্রত্যাগম সত্য;
}

APFloat::opStatus APFloat::convert(ধ্রুব fltSemantics &ToSemantics,
                                   roundingMode RM, তার্কিক *losesInfo) {
  যদি (&getSemantics() == &ToSemantics) {
    *losesInfo = অসত্য;
    প্রত্যাগম opOK;
  }
  যদি (usesLayout<IEEEFloat>(getSemantics()) &&
      usesLayout<IEEEFloat>(ToSemantics))
    প্রত্যাগম U.IEEE.convert(ToSemantics, RM, losesInfo);
  যদি (usesLayout<IEEEFloat>(getSemantics()) &&
      usesLayout<DoubleAPFloat>(ToSemantics)) {
    প্রতিজ্ঞা(&ToSemantics == &APFloatBase::semPPCDoubleDouble);
    auto Ret =
        U.IEEE.convert(APFloatBase::semPPCDoubleDoubleLegacy, RM, losesInfo);
    *এতত্ = APFloat(ToSemantics, U.IEEE.bitcastToAPInt());
    প্রত্যাগম Ret;
  }
  যদি (usesLayout<DoubleAPFloat>(getSemantics()) &&
      usesLayout<IEEEFloat>(ToSemantics)) {
    auto Ret = getIEEE().convert(ToSemantics, RM, losesInfo);
    *এতত্ = APFloat(std::move(getIEEE()), ToSemantics);
    প্রত্যাগম Ret;
  }
  llvm_unreachable("Unexpected semantics");
}

APFloat APFloat::get_All_Ones_মান(ধ্রুব fltSemantics &Semantics) {
  প্রত্যাগম APFloat(Semantics, APInt::getAllOnes(Semantics.sizeInBits));
}

শূন্য APFloat::print(raw_ostream &OS) ধ্রুব {
  SmallVector<বর্ণ, 16> Buffer;
  toString(Buffer);
  OS << Buffer;
}

#যদি !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)
LLVM_DUMP_METHOD শূন্য APFloat::dump() ধ্রুব {
  print(dbgs());
  dbgs() << '\n';
}
#endif

শূন্য APFloat::Profile(Folding_Set_গ্রন্থি_ID &NID) ধ্রুব {
  NID.Add(bitcastToAPInt());
}

APFloat::opStatus APFloat::convertToInteger(APSInt &result,
                                            roundingMode rounding_mode,
                                            তার্কিক *isExact) ধ্রুব {
  unsigned bitWidth = result.getBitWidth();
  SmallVector<uint64_t, 4> parts(result.get_অঙ্ক_শব্দগুলি());
  opStatus status = convertToInteger(parts, bitWidth, result.isSigned(),
                                     rounding_mode, isExact);
  // Keeps the original signed-ness.
  result = APInt(bitWidth, parts);
  প্রত্যাগম status;
}

দ্বিগুণ APFloat::convertToDouble() ধ্রুব {
  যদি (&getSemantics() == &APFloatBase::semIEEEdouble)
    প্রত্যাগম getIEEE().convertToDouble();
  প্রতিজ্ঞা(isRepresentableBy(getSemantics(), semIEEEdouble) &&
         "Float semantics is not representable by IEEEdouble");
  APFloat Temp = *এতত্;
  তার্কিক LosesInfo;
  [[maybe_unused]] opStatus St =
      Temp.convert(APFloatBase::semIEEEdouble, rmNearestTiesToEven, &LosesInfo);
  প্রতিজ্ঞা(!(St & opInexact) && !LosesInfo && "Unexpected imprecision");
  প্রত্যাগম Temp.getIEEE().convertToDouble();
}

#ifdef HAS_IEE754_FLOAT128
float128 APFloat::convertToQuad() ধ্রুব {
  যদি (&getSemantics() == &APFloatBase::semIEEEquad)
    প্রত্যাগম getIEEE().convertToQuad();
  প্রতিজ্ঞা(isRepresentableBy(getSemantics(), semIEEEquad) &&
         "Float semantics is not representable by IEEEquad");
  APFloat Temp = *এতত্;
  তার্কিক LosesInfo;
  [[maybe_unused]] opStatus St =
      Temp.convert(APFloatBase::semIEEEquad, rmNearestTiesToEven, &LosesInfo);
  প্রতিজ্ঞা(!(St & opInexact) && !LosesInfo && "Unexpected imprecision");
  প্রত্যাগম Temp.getIEEE().convertToQuad();
}
#endif

ভিন্ন APFloat::convertToFloat() ধ্রুব {
  যদি (&getSemantics() == &APFloatBase::semIEEEsingle)
    প্রত্যাগম getIEEE().convertToFloat();
  প্রতিজ্ঞা(isRepresentableBy(getSemantics(), semIEEEsingle) &&
         "Float semantics is not representable by IEEEsingle");
  APFloat Temp = *এতত্;
  তার্কিক LosesInfo;
  [[maybe_unused]] opStatus St =
      Temp.convert(APFloatBase::semIEEEsingle, rmNearestTiesToEven, &LosesInfo);
  প্রতিজ্ঞা(!(St & opInexact) && !LosesInfo && "Unexpected imprecision");
  প্রত্যাগম Temp.getIEEE().convertToFloat();
}

unsigned APFloatBase::getArbitraryFPFormatSizeInBits(StringRef Format) {
  প্রত্যাগম StringSwitch<unsigned>(Format)
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

তার্কিক APFloatBase::isValidArbitraryFPFormat(StringRef Format) {
  প্রত্যাগম getArbitraryFPFormatSizeInBits(Format) != 0;
}

ধ্রুব fltSemantics *APFloatBase::getArbitraryFPSemantics(StringRef Format) {
  // TODO: extend to remaining arbitrary FP types: Float8E4M3, Float8E3M4,
  // Float8E5M2FNUZ, Float8E4M3FNUZ, Float8E4M3B11FNUZ, Float8E8M0FNU.
  প্রত্যাগম StringSwitch<ধ্রুব fltSemantics *>(Format)
      .Case("Float8E5M2", &semFloat8E5M2)
      .Case("Float8E4M3FN", &semFloat8E4M3FN)
      .Case("Float8E5M3FNU", &semFloat8E5M3FNU)
      .Case("Float4E2M1FN", &semFloat4E2M1FN)
      .Case("Float6E3M2FN", &semFloat6E3M2FN)
      .Case("Float6E2M3FN", &semFloat6E2M3FN)
      .Default(nullptr);
}

APFloat::Storage::~Storage() {
  যদি (usesLayout<IEEEFloat>(*semantics)) {
    IEEE.~IEEEFloat();
    প্রত্যাগম;
  }
  যদি (usesLayout<DoubleAPFloat>(*semantics)) {
    Double.~DoubleAPFloat();
    প্রত্যাগম;
  }
  llvm_unreachable("Unexpected semantics");
}

APFloat::Storage::Storage(ধ্রুব APFloat::Storage &RHS) {
  যদি (usesLayout<IEEEFloat>(*RHS.semantics)) {
    নূতন (এতত্) IEEEFloat(RHS.IEEE);
    প্রত্যাগম;
  }
  যদি (usesLayout<DoubleAPFloat>(*RHS.semantics)) {
    নূতন (এতত্) DoubleAPFloat(RHS.Double);
    প্রত্যাগম;
  }
  llvm_unreachable("Unexpected semantics");
}

APFloat::Storage::Storage(APFloat::Storage &&RHS) {
  যদি (usesLayout<IEEEFloat>(*RHS.semantics)) {
    নূতন (এতত্) IEEEFloat(std::move(RHS.IEEE));
    প্রত্যাগম;
  }
  যদি (usesLayout<DoubleAPFloat>(*RHS.semantics)) {
    নূতন (এতত্) DoubleAPFloat(std::move(RHS.Double));
    প্রত্যাগম;
  }
  llvm_unreachable("Unexpected semantics");
}

APFloat::Storage &APFloat::Storage::operator=(ধ্রুব APFloat::Storage &RHS) {
  যদি (usesLayout<IEEEFloat>(*semantics) &&
      usesLayout<IEEEFloat>(*RHS.semantics)) {
    IEEE = RHS.IEEE;
  } অন্যথা যদি (usesLayout<DoubleAPFloat>(*semantics) &&
             usesLayout<DoubleAPFloat>(*RHS.semantics)) {
    Double = RHS.Double;
  } অন্যথা যদি (এতত্ != &RHS) {
    এতত্->~Storage();
    নূতন (এতত্) Storage(RHS);
  }
  প্রত্যাগম *এতত্;
}

APFloat::Storage &APFloat::Storage::operator=(APFloat::Storage &&RHS) {
  যদি (usesLayout<IEEEFloat>(*semantics) &&
      usesLayout<IEEEFloat>(*RHS.semantics)) {
    IEEE = std::move(RHS.IEEE);
  } অন্যথা যদি (usesLayout<DoubleAPFloat>(*semantics) &&
             usesLayout<DoubleAPFloat>(*RHS.semantics)) {
    Double = std::move(RHS.Double);
  } অন্যথা যদি (এতত্ != &RHS) {
    এতত্->~Storage();
    নূতন (এতত্) Storage(std::move(RHS));
  }
  প্রত্যাগম *এতত্;
}

নামাকাশ {

APFloat::opStatus getOpStatusFromLibc(পূর্ণাঙ্ক libc_exceptions) {
  APFloat::opStatus status = APFloat::opOK;
  যদি (libc_exceptions & FE_INVALID)
    status = static_cast<APFloat::opStatus>(status | APFloat::opInvalidOp);
  যদি (libc_exceptions & FE_DIVBYZERO)
    status = static_cast<APFloat::opStatus>(status | APFloat::opDivByZero);
  যদি (libc_exceptions & FE_OVERFLOW)
    status = static_cast<APFloat::opStatus>(status | APFloat::opOverflow);
  যদি (libc_exceptions & FE_UNDERFLOW)
    status = static_cast<APFloat::opStatus>(status | APFloat::opUnderflow);
  যদি (libc_exceptions & FE_INEXACT)
    status = static_cast<APFloat::opStatus>(status | APFloat::opInexact);
  প্রত্যাগম status;
}

} // namespace

// TODO: Support other rounding modes when LLVM libc math implement static
// roundings.
std::optional<APFloat> exp(ধ্রুব APFloat &x, RoundingMode rounding_mode,
                           APFloat::opStatus *status) {

  যদি (rounding_mode == APFloatBase::rmNearestTiesToEven) {
    যদি (APFloat::SemanticsToEnum(x.getSemantics()) ==
        APFloatBase::S_IEEEsingle) {
      ভিন্ন x_val = x.convertToFloat();
      পূর্ণাঙ্ক exc =
          LIBC_NAMESPACE::shared::check::exp_exceptions(x_val, FE_TONEAREST);
      যদি (status) {
        *status = getOpStatusFromLibc(exc);
        যদি (x.isSignaling()) {
          // 32-bit x86 will silence sNaN when loading floats, so we explicitly
          // add the INVALID exception here.
          *status =
              static_cast<APFloat::opStatus>(*status | APFloat::opInvalidOp);
        }
      }
      ভিন্ন result = LIBC_NAMESPACE::shared::expf(x_val);
      প্রত্যাগম APFloat(result);
    }
    যদি (APFloat::SemanticsToEnum(x.getSemantics()) ==
        APFloatBase::S_IEEEdouble) {
      দ্বিগুণ x_val = x.convertToDouble();
      পূর্ণাঙ্ক exc =
          LIBC_NAMESPACE::shared::check::exp_exceptions(x_val, FE_TONEAREST);
      যদি (status) {
        *status = getOpStatusFromLibc(exc);
        যদি (x.isSignaling()) {
          // 32-bit x86 will silence sNaN when loading floats, so we explicitly
          // add the INVALID exception here.
          *status =
              static_cast<APFloat::opStatus>(*status | APFloat::opInvalidOp);
        }
      }
      দ্বিগুণ result = LIBC_NAMESPACE::shared::exp(x_val);
      প্রত্যাগম APFloat(result);
    }
  }
  প্রত্যাগম std::nullopt;
}

} // namespace llvm

#undef APFLOAT_DISPATCH_ON_SEMANTICS
