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

#ଅନ୍ତର୍ଭାଵ "llvm/ADT/APFloat.h"
#ଅନ୍ତର୍ଭାଵ "llvm/ADT/APSInt.h"
#ଅନ୍ତର୍ଭାଵ "llvm/ADT/ArrayRef.h"
#ଅନ୍ତର୍ଭାଵ "llvm/ADT/FloatingPointMode.h"
#ଅନ୍ତର୍ଭାଵ "llvm/ADT/FoldingSet.h"
#ଅନ୍ତର୍ଭାଵ "llvm/ADT/Hashing.h"
#ଅନ୍ତର୍ଭାଵ "llvm/ADT/STLExtras.h"
#ଅନ୍ତର୍ଭାଵ "llvm/ADT/SmallVector.h"
#ଅନ୍ତର୍ଭାଵ "llvm/ADT/StringExtras.h"
#ଅନ୍ତର୍ଭାଵ "llvm/ADT/StringRef.h"
#ଅନ୍ତର୍ଭାଵ "llvm/ADT/StringSwitch.h"
#ଅନ୍ତର୍ଭାଵ "llvm/Config/llvm-config.h"
#ଅନ୍ତର୍ଭାଵ "llvm/Support/Debug.h"
#ଅନ୍ତର୍ଭାଵ "llvm/Support/Error.h"
#ଅନ୍ତର୍ଭାଵ "llvm/Support/MathExtras.h"
#ଅନ୍ତର୍ଭାଵ "llvm/Support/raw_ostream.h"
#ଅନ୍ତର୍ଭାଵ <cstring>
#ଅନ୍ତର୍ଭାଵ <iterator>
#ଅନ୍ତର୍ଭାଵ <limits.h>

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

#ଅନ୍ତର୍ଭାଵ "shared/math.h"
#ଅନ୍ତର୍ଭାଵ "shared/math_check_exceptions.h"

#define APFLOAT_DISPATCH_ON_SEMANTICS(METHOD_CALL)                             \
  କର {                                                                         \
    ଯଦି (usesLayout<IEEEFloat>(getSemantics()))                                 \
      ଫେରନ୍ତୁ U.IEEE.METHOD_CALL;                                               \
    ଯଦି (usesLayout<DoubleAPFloat>(getSemantics()))                             \
      ଫେରନ୍ତୁ U.Double.METHOD_CALL;                                             \
    llvm_unreachable("Unexpected semantics");                                  \
  } ଯେତେବେଳେ (ମିଥ୍ୟା)

using ନାମାକାଶ llvm;

/// A macro used to combine two fcCategory enums into one key which can be used
/// in a switch statement to classify how the interaction of two APFloat's
/// categories affects an operation.
///
/// TODO: If clang source code is ever allowed to use constexpr in its own
/// codebase, change this into a static inline function.
#define Pack_Categories_Into_ଚାବି(_lhs, _rhs) ((_lhs) * 4 + (_rhs))

/* Assumed in hexadecimal significand parsing, and conversion to
   hexadecimal strings.  */
static_assert(APFloatBase::integerPartWidth % 4 == 0, "Part width must be divisible by 4!");

ନାମାକାଶ llvm {

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
    ସତ୍ୟ,
    ସତ୍ୟ,
    ସତ୍ୟ,
    ସତ୍ୟ,
    ସତ୍ୟ};
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
    ମିଥ୍ୟା,
    ମିଥ୍ୟା,
    ମିଥ୍ୟା,
    ମିଥ୍ୟା};

constexpr fltSemantics APFloatBase::semFloat8E5M3FNU = {
    16,
    -14,
    4,
    8,
    fltNonfiniteBehavior::NanOnly,
    fltNanEncoding::AllOnes,
    ସତ୍ୟ,
    ମିଥ୍ୟା,
    ମିଥ୍ୟା};

constexpr fltSemantics APFloatBase::semFloat6E3M2FN = {
    4, -2, 3, 6, fltNonfiniteBehavior::FiniteOnly};
constexpr fltSemantics APFloatBase::semFloat6E2M3FN = {
    2, 0, 4, 6, fltNonfiniteBehavior::FiniteOnly};
constexpr fltSemantics APFloatBase::semFloat4E2M1FN = {
    2, 0, 2, 4, fltNonfiniteBehavior::FiniteOnly};
constexpr fltSemantics APFloatBase::semBogus = {0, 0, 0, 0};

ଅଚଳ llvm::fltSemantics &APFloatBase::EnumToSemantics(Semantics S) {
  ବାଛ (S) {
  ସ୍ଥିତି S_IEEEhalf:
    ଫେରନ୍ତୁ IEEEhalf();
  ସ୍ଥିତି S_BFloat:
    ଫେରନ୍ତୁ BFloat();
  ସ୍ଥିତି S_IEEEsingle:
    ଫେରନ୍ତୁ IEEEsingle();
  ସ୍ଥିତି S_IEEEdouble:
    ଫେରନ୍ତୁ IEEEdouble();
  ସ୍ଥିତି S_IEEEquad:
    ଫେରନ୍ତୁ IEEEquad();
  ସ୍ଥିତି S_x87DoubleExtended:
    ଫେରନ୍ତୁ x87DoubleExtended();
  ସ୍ଥିତି S_PPCDoubleDouble:
    ଫେରନ୍ତୁ PPCDoubleDouble();
  ସ୍ଥିତି S_PPCDoubleDoubleLegacy:
    ଫେରନ୍ତୁ PPCDoubleDoubleLegacy();
  ସ୍ଥିତି S_Float8E5M2:
    ଫେରନ୍ତୁ Float8E5M2();
  ସ୍ଥିତି S_Float8E5M2FNUZ:
    ଫେରନ୍ତୁ Float8E5M2FNUZ();
  ସ୍ଥିତି S_Float8E4M3:
    ଫେରନ୍ତୁ Float8E4M3();
  ସ୍ଥିତି S_Float8E4M3FN:
    ଫେରନ୍ତୁ Float8E4M3FN();
  ସ୍ଥିତି S_Float8E4M3FNUZ:
    ଫେରନ୍ତୁ Float8E4M3FNUZ();
  ସ୍ଥିତି S_Float8E4M3B11FNUZ:
    ଫେରନ୍ତୁ Float8E4M3B11FNUZ();
  ସ୍ଥିତି S_Float8E3M4:
    ଫେରନ୍ତୁ Float8E3M4();
  ସ୍ଥିତି S_FloatTF32:
    ଫେରନ୍ତୁ FloatTF32();
  ସ୍ଥିତି S_Float8E8M0FNU:
    ଫେରନ୍ତୁ Float8E8M0FNU();
  ସ୍ଥିତି S_Float8E5M3FNU:
    ଫେରନ୍ତୁ Float8E5M3FNU();
  ସ୍ଥିତି S_Float6E3M2FN:
    ଫେରନ୍ତୁ Float6E3M2FN();
  ସ୍ଥିତି S_Float6E2M3FN:
    ଫେରନ୍ତୁ Float6E2M3FN();
  ସ୍ଥିତି S_Float4E2M1FN:
    ଫେରନ୍ତୁ Float4E2M1FN();
  }
  llvm_unreachable("Unrecognised floating semantics");
}

APFloatBase::Semantics
APFloatBase::SemanticsToEnum(ଅଚଳ llvm::fltSemantics &Sem) {
  ଯଦି (&Sem == &llvm::APFloat::IEEEhalf())
    ଫେରନ୍ତୁ S_IEEEhalf;
  ଅନ୍ୟଥା ଯଦି (&Sem == &llvm::APFloat::BFloat())
    ଫେରନ୍ତୁ S_BFloat;
  ଅନ୍ୟଥା ଯଦି (&Sem == &llvm::APFloat::IEEEsingle())
    ଫେରନ୍ତୁ S_IEEEsingle;
  ଅନ୍ୟଥା ଯଦି (&Sem == &llvm::APFloat::IEEEdouble())
    ଫେରନ୍ତୁ S_IEEEdouble;
  ଅନ୍ୟଥା ଯଦି (&Sem == &llvm::APFloat::IEEEquad())
    ଫେରନ୍ତୁ S_IEEEquad;
  ଅନ୍ୟଥା ଯଦି (&Sem == &llvm::APFloat::x87DoubleExtended())
    ଫେରନ୍ତୁ S_x87DoubleExtended;
  ଅନ୍ୟଥା ଯଦି (&Sem == &llvm::APFloat::PPCDoubleDouble())
    ଫେରନ୍ତୁ S_PPCDoubleDouble;
  ଅନ୍ୟଥା ଯଦି (&Sem == &llvm::APFloat::PPCDoubleDoubleLegacy())
    ଫେରନ୍ତୁ S_PPCDoubleDoubleLegacy;
  ଅନ୍ୟଥା ଯଦି (&Sem == &llvm::APFloat::Float8E5M2())
    ଫେରନ୍ତୁ S_Float8E5M2;
  ଅନ୍ୟଥା ଯଦି (&Sem == &llvm::APFloat::Float8E5M2FNUZ())
    ଫେରନ୍ତୁ S_Float8E5M2FNUZ;
  ଅନ୍ୟଥା ଯଦି (&Sem == &llvm::APFloat::Float8E4M3())
    ଫେରନ୍ତୁ S_Float8E4M3;
  ଅନ୍ୟଥା ଯଦି (&Sem == &llvm::APFloat::Float8E4M3FN())
    ଫେରନ୍ତୁ S_Float8E4M3FN;
  ଅନ୍ୟଥା ଯଦି (&Sem == &llvm::APFloat::Float8E4M3FNUZ())
    ଫେରନ୍ତୁ S_Float8E4M3FNUZ;
  ଅନ୍ୟଥା ଯଦି (&Sem == &llvm::APFloat::Float8E4M3B11FNUZ())
    ଫେରନ୍ତୁ S_Float8E4M3B11FNUZ;
  ଅନ୍ୟଥା ଯଦି (&Sem == &llvm::APFloat::Float8E3M4())
    ଫେରନ୍ତୁ S_Float8E3M4;
  ଅନ୍ୟଥା ଯଦି (&Sem == &llvm::APFloat::FloatTF32())
    ଫେରନ୍ତୁ S_FloatTF32;
  ଅନ୍ୟଥା ଯଦି (&Sem == &llvm::APFloat::Float8E8M0FNU())
    ଫେରନ୍ତୁ S_Float8E8M0FNU;
  ଅନ୍ୟଥା ଯଦି (&Sem == &llvm::APFloat::Float8E5M3FNU())
    ଫେରନ୍ତୁ S_Float8E5M3FNU;
  ଅନ୍ୟଥା ଯଦି (&Sem == &llvm::APFloat::Float6E3M2FN())
    ଫେରନ୍ତୁ S_Float6E3M2FN;
  ଅନ୍ୟଥା ଯଦି (&Sem == &llvm::APFloat::Float6E2M3FN())
    ଫେରନ୍ତୁ S_Float6E2M3FN;
  ଅନ୍ୟଥା ଯଦି (&Sem == &llvm::APFloat::Float4E2M1FN())
    ଫେରନ୍ତୁ S_Float4E2M1FN;
  ଅନ୍ୟଥା
    llvm_unreachable("Unknown floating semantics");
}

ତାର୍କିକ APFloatBase::isRepresentableBy(ଅଚଳ fltSemantics &A,
                                    ଅଚଳ fltSemantics &B) {
  ଫେରନ୍ତୁ A.maxExponent <= B.maxExponent && A.minExponent >= B.minExponent &&
         A.precision <= B.precision;
}

ତାର୍କିକ APFloatBase::isLosslesslyConvertibleTo(ଅଚଳ fltSemantics &From,
                                            ଅଚଳ fltSemantics &InitialTo,
                                            ତାର୍କିକ IgnoreNaNs) {
  ଯଦି (&From == &InitialTo)
    ଫେରନ୍ତୁ ସତ୍ୟ;

  // PPC double-double cannot be described by a conventional exponent range
  // and precision. In particular, converting it to another semantics drops
  // its low double, so conservatively reject conversions from it.
  ଯଦି (&From == &semPPCDoubleDouble)
    ଫେରନ୍ତୁ ମିଥ୍ୟା;

  // Converting to PPC double-double is lossless exactly when the value fits
  // in its high double. In other words, the conversion is lossless if the
  // conversion to IEEEdouble is lossless.
  ଅଚଳ fltSemantics &To =
      &InitialTo == &semPPCDoubleDouble ? semIEEEdouble : InitialTo;

  ଯଦି (!isRepresentableBy(From, To))
    ଫେରନ୍ତୁ ମିଥ୍ୟା;

  ଯଦି ((From.hasZero && !To.hasZero) ||
      (From.hasSignedRepr && !To.hasSignedRepr))
    ଫେରନ୍ତୁ ମିଥ୍ୟା;

  // NegativeZero NaN encoding repurposes the negative-zero bit pattern, so a
  // conversion to such a format cannot preserve a source negative zero.
  ତାର୍କିକ FromHasSignedZero = From.hasZero && From.hasSignedRepr &&
                           From.nanEncoding != fltNanEncoding::NegativeZero;
  ତାର୍କିକ ToHasSignedZero = To.hasZero && To.hasSignedRepr &&
                         To.nanEncoding != fltNanEncoding::NegativeZero;
  ଯଦି (FromHasSignedZero && !ToHasSignedZero)
    ଫେରନ୍ତୁ ମିଥ୍ୟା;

  // isRepresentableBy compares normalized exponent ranges. Also ensure that
  // the smallest source value, which may be denormal, is represented exactly
  // by the destination semantics.
  APFloat SmallestFrom = APFloat::getSmallest(From);
  ତାର୍କିକ LosesInfo = ମିଥ୍ୟା;
  (ଶୂନ୍ୟ)SmallestFrom.convert(To, APFloat::rmNearestTiesToEven, &LosesInfo);
  ଯଦି (LosesInfo)
    ଫେରନ୍ତୁ ମିଥ୍ୟା;

  ଯଦି (From.nonFiniteBehavior == fltNonfiniteBehavior::FiniteOnly)
    ଫେରନ୍ତୁ ସତ୍ୟ;

  // Even when NaN representations can be ignored, NaNs must remain NaNs and
  // infinities must remain infinities. Otherwise the conversion can change
  // whether an operation with nnan has poison-producing operands.
  ଯଦି (IgnoreNaNs) {
    ଯଦି (From.nonFiniteBehavior == fltNonfiniteBehavior::IEEE754)
      ଫେରନ୍ତୁ To.nonFiniteBehavior == fltNonfiniteBehavior::IEEE754;
    ଫେରନ୍ତୁ To.nonFiniteBehavior != fltNonfiniteBehavior::FiniteOnly;
  }

  ଯଦି (From.nonFiniteBehavior == fltNonfiniteBehavior::IEEE754) {
    // Converting an IEEE signaling NaN to another semantics quiets it, so the
    // original value cannot be recovered by converting it back.
    ଫେରନ୍ତୁ ମିଥ୍ୟା;
  }

  // NanOnly formats have no signaling NaNs. IEEE semantics can represent
  // their quiet NaNs; conversions between NanOnly formats are conservatively
  // accepted only when they use the same NaN encoding.
  ଯଦି (To.nonFiniteBehavior == fltNonfiniteBehavior::IEEE754)
    ଫେରନ୍ତୁ ସତ୍ୟ;
  ଫେରନ୍ତୁ To.nonFiniteBehavior == fltNonfiniteBehavior::NanOnly &&
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
ସ୍ଥିର constexpr unsigned ଅଙ୍କ_Pow5_Parts_On_ଗଦା = 16;

ସ୍ଥିର unsigned ପୂର୍ଣ୍ଣସଂଖ୍ୟା powerOfFivePartsForExponent(unsigned ପୂର୍ଣ୍ଣସଂଖ୍ୟା power) {
  ପ୍ରତିଜ୍ଞା(power <= maxExponent);

  // 815 / 351 is an upper bound on log2(5).  Reserve an additional part for
  // tcFullMultiply, which writes the full product width even when its most
  // significant part is zero.
  ଫେରନ୍ତୁ 2 + ((power * 815) / (351 * APFloatBase::integerPartWidth));
}

unsigned ପୂର୍ଣ୍ଣସଂଖ୍ୟା APFloatBase::semanticsPrecision(ଅଚଳ fltSemantics &semantics) {
  ଫେରନ୍ତୁ semantics.precision;
}
APFloatBase::ExponentType
APFloatBase::semanticsMaxExponent(ଅଚଳ fltSemantics &semantics) {
  ଫେରନ୍ତୁ semantics.maxExponent;
}
APFloatBase::ExponentType
APFloatBase::semanticsMinExponent(ଅଚଳ fltSemantics &semantics) {
  ଫେରନ୍ତୁ semantics.minExponent;
}
unsigned ପୂର୍ଣ୍ଣସଂଖ୍ୟା APFloatBase::semanticsSizeInBits(ଅଚଳ fltSemantics &semantics) {
  ଫେରନ୍ତୁ semantics.sizeInBits;
}
unsigned ପୂର୍ଣ୍ଣସଂଖ୍ୟା APFloatBase::semanticsIntSizeInBits(ଅଚଳ fltSemantics &semantics,
                                                 ତାର୍କିକ isSigned) {
  // The max FP value is pow(2, MaxExponent) * (1 + MaxFraction), so we need
  // at least one more bit than the MaxExponent to hold the max FP value.
  unsigned ପୂର୍ଣ୍ଣସଂଖ୍ୟା MinBitWidth = semanticsMaxExponent(semantics) + 1;
  // Extra sign bit needed.
  ଯଦି (isSigned)
    ++MinBitWidth;
  ଫେରନ୍ତୁ MinBitWidth;
}

ତାର୍କିକ APFloatBase::semanticsHasZero(ଅଚଳ fltSemantics &semantics) {
  ଫେରନ୍ତୁ semantics.hasZero;
}

ତାର୍କିକ APFloatBase::semanticsHasSignedRepr(ଅଚଳ fltSemantics &semantics) {
  ଫେରନ୍ତୁ semantics.hasSignedRepr;
}

ତାର୍କିକ APFloatBase::semanticsHasInf(ଅଚଳ fltSemantics &semantics) {
  ଫେରନ୍ତୁ semantics.nonFiniteBehavior == fltNonfiniteBehavior::IEEE754;
}

ତାର୍କିକ APFloatBase::semanticsHasNaN(ଅଚଳ fltSemantics &semantics) {
  ଫେରନ୍ତୁ semantics.nonFiniteBehavior != fltNonfiniteBehavior::FiniteOnly;
}

ତାର୍କିକ APFloatBase::isIEEELikeFP(ଅଚଳ fltSemantics &semantics) {
  // Keep in sync with Type::isIEEELikeFPTy
  ଫେରନ୍ତୁ SemanticsToEnum(semantics) <= S_IEEEquad;
}

ତାର୍କିକ APFloatBase::hasSignBitInMSB(ଅଚଳ fltSemantics &semantics) {
  ଫେରନ୍ତୁ semantics.hasSignBitInMSB;
}

ତାର୍କିକ APFloatBase::isRepresentableAsNormalIn(ଅଚଳ fltSemantics &Src,
                                            ଅଚଳ fltSemantics &Dst) {
  // Exponent range must be larger.
  ଯଦି (Src.maxExponent >= Dst.maxExponent || Src.minExponent <= Dst.minExponent)
    ଫେରନ୍ତୁ ମିଥ୍ୟା;

  // If the mantissa is long enough, the result value could still be denormal
  // with a larger exponent range.
  //
  // FIXME: This condition is probably not accurate but also shouldn't be a
  // practical concern with existing types.
  ଫେରନ୍ତୁ Dst.precision >= Src.precision;
}

unsigned APFloatBase::getSizeInBits(ଅଚଳ fltSemantics &Sem) {
  ଫେରନ୍ତୁ Sem.sizeInBits;
}

ସ୍ଥିର constexpr APFloatBase::ExponentType
exponentZero(ଅଚଳ fltSemantics &semantics) {
  ଫେରନ୍ତୁ semantics.minExponent - 1;
}

ସ୍ଥିର constexpr APFloatBase::ExponentType
exponentInf(ଅଚଳ fltSemantics &semantics) {
  ଫେରନ୍ତୁ semantics.maxExponent + 1;
}

ସ୍ଥିର constexpr APFloatBase::ExponentType
exponentNaN(ଅଚଳ fltSemantics &semantics) {
  ଯଦି (semantics.nonFiniteBehavior == fltNonfiniteBehavior::NanOnly) {
    ଯଦି (semantics.nanEncoding == fltNanEncoding::NegativeZero)
      ଫେରନ୍ତୁ exponentZero(semantics);
    ଯଦି (semantics.hasSignedRepr || semantics.precision > 1)
      ଫେରନ୍ତୁ semantics.maxExponent;
  }
  ଫେରନ୍ତୁ semantics.maxExponent + 1;
}

/* A bunch of private, handy routines.  */

ସ୍ଥିର ଧାଡ଼ିଭିତରେ Error createError(ଅଚଳ Twine &Err) {
  ଫେରନ୍ତୁ make_error<StringError>(Err, inconvertibleErrorCode());
}

ସ୍ଥିର constexpr ଧାଡ଼ିଭିତରେ unsigned ପୂର୍ଣ୍ଣସଂଖ୍ୟା part_ଗଣନା_For_Bits(unsigned ପୂର୍ଣ୍ଣସଂଖ୍ୟା bits) {
  ଫେରନ୍ତୁ std::max(1u, (bits + APFloatBase::integerPartWidth - 1) /
                          APFloatBase::integerPartWidth);
}

/* Returns 0U-9U.  Return values >= 10U are not digits.  */
ସ୍ଥିର ଧାଡ଼ିଭିତରେ unsigned ପୂର୍ଣ୍ଣସଂଖ୍ୟା
dec_Digit_ମୂଲ୍ୟ(unsigned ପୂର୍ଣ୍ଣସଂଖ୍ୟା c)
{
  ଫେରନ୍ତୁ c - '0';
}

/* Return the value of a decimal exponent of the form
   [+-]ddddddd.

   If the exponent overflows, returns a large exponent with the
   appropriate sign.  */
ସ୍ଥିର Expected<ପୂର୍ଣ୍ଣସଂଖ୍ୟା> readExponent(StringRef::iterator begin,
                                  StringRef::iterator end) {
  ଅଚଳ unsigned ପୂର୍ଣ୍ଣସଂଖ୍ୟା overlargeExponent = 24000;  /* FIXME.  */
  StringRef::iterator p = begin;

  // Treat no exponent as 0 to match binutils
  ଯଦି (p == end || ((*p == '-' || *p == '+') && (p + 1) == end))
    ଫେରନ୍ତୁ 0;

  ତାର୍କିକ isNegative = *p == '-';
  ଯଦି (*p == '-' || *p == '+') {
    p++;
    ଯଦି (p == end)
      ଫେରନ୍ତୁ createError("Exponent has no digits");
  }

  unsigned absExponent = dec_Digit_ମୂଲ୍ୟ(*p++);
  ଯଦି (absExponent >= 10U)
    ଫେରନ୍ତୁ createError("Invalid character in exponent");

  ପାଇଁ (; p != end; ++p) {
    unsigned ମୂଲ୍ୟ_2 = dec_Digit_ମୂଲ୍ୟ(*p);
    ଯଦି (ମୂଲ୍ୟ_2 >= 10U)
      ଫେରନ୍ତୁ createError("Invalid character in exponent");

    absExponent = absExponent * 10U + ମୂଲ୍ୟ_2;
    ଯଦି (absExponent >= overlargeExponent) {
      absExponent = overlargeExponent;
      ଭାଙ୍ଗ;
    }
  }

  ଯଦି (isNegative)
    ଫେରନ୍ତୁ -(ପୂର୍ଣ୍ଣସଂଖ୍ୟା) absExponent;
  ଅନ୍ୟଥା
    ଫେରନ୍ତୁ (ପୂର୍ଣ୍ଣସଂଖ୍ୟା) absExponent;
}

/* This is ugly and needs cleaning up, but I don't immediately see
   how whilst remaining safe.  */
ସ୍ଥିର Expected<ପୂର୍ଣ୍ଣସଂଖ୍ୟା> totalExponent(StringRef::iterator p,
                                   StringRef::iterator end,
                                   ପୂର୍ଣ୍ଣସଂଖ୍ୟା exponentAdjustment) {
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା exponent = 0;

  ଯଦି (p == end)
    ଫେରନ୍ତୁ createError("Exponent has no digits");

  ତାର୍କିକ negative = *p == '-';
  ଯଦି (*p == '-' || *p == '+') {
    p++;
    ଯଦି (p == end)
      ଫେରନ୍ତୁ createError("Exponent has no digits");
  }

  ପୂର୍ଣ୍ଣସଂଖ୍ୟା unsignedExponent = 0;
  ତାର୍କିକ overflow = ମିଥ୍ୟା;
  ପାଇଁ (; p != end; ++p) {
    unsigned ପୂର୍ଣ୍ଣସଂଖ୍ୟା ମୂଲ୍ୟ_2;

    ମୂଲ୍ୟ_2 = dec_Digit_ମୂଲ୍ୟ(*p);
    ଯଦି (ମୂଲ୍ୟ_2 >= 10U)
      ଫେରନ୍ତୁ createError("Invalid character in exponent");

    unsignedExponent = unsignedExponent * 10 + ମୂଲ୍ୟ_2;
    ଯଦି (unsignedExponent > 32767) {
      overflow = ସତ୍ୟ;
      ଭାଙ୍ଗ;
    }
  }

  ଯଦି (exponentAdjustment > 32767 || exponentAdjustment < -32768)
    overflow = ସତ୍ୟ;

  ଯଦି (!overflow) {
    exponent = unsignedExponent;
    ଯଦି (negative)
      exponent = -exponent;
    exponent += exponentAdjustment;
    ଯଦି (exponent > 32767 || exponent < -32768)
      overflow = ସତ୍ୟ;
  }

  ଯଦି (overflow)
    exponent = negative ? -32768: 32767;

  ଫେରନ୍ତୁ exponent;
}

ସ୍ଥିର Expected<StringRef::iterator>
skipLeadingZeroesAndAnyDot(StringRef::iterator begin, StringRef::iterator end,
                           StringRef::iterator *dot) {
  StringRef::iterator p = begin;
  *dot = end;
  ଯେତେବେଳେ (p != end && *p == '0')
    p++;

  ଯଦି (p != end && *p == '.') {
    *dot = p++;

    ଯଦି (end - begin == 1)
      ଫେରନ୍ତୁ createError("Significand has no digits");

    ଯେତେବେଳେ (p != end && *p == '0')
      p++;
  }

  ଫେରନ୍ତୁ p;
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
ସଂରଚନା decimalInfo {
  ଅଚଳ ଅକ୍ଷର *firstSigDigit;
  ଅଚଳ ଅକ୍ଷର *lastSigDigit;
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା exponent;
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା normalizedExponent;
};

ସ୍ଥିର Error interpretDecimal(StringRef::iterator begin,
                              StringRef::iterator end, decimalInfo *D) {
  StringRef::iterator dot = end;

  auto PtrOrErr = skipLeadingZeroesAndAnyDot(begin, end, &dot);
  ଯଦି (!PtrOrErr)
    ଫେରନ୍ତୁ PtrOrErr.takeError();
  StringRef::iterator p = *PtrOrErr;

  D->firstSigDigit = p;
  D->exponent = 0;
  D->normalizedExponent = 0;

  ପାଇଁ (; p != end; ++p) {
    ଯଦି (*p == '.') {
      ଯଦି (dot != end)
        ଫେରନ୍ତୁ createError("String contains multiple dots");
      dot = p++;
      ଯଦି (p == end)
        ଭାଙ୍ଗ;
    }
    ଯଦି (dec_Digit_ମୂଲ୍ୟ(*p) >= 10U)
      ଭାଙ୍ଗ;
  }

  ଯଦି (p != end) {
    ଯଦି (*p != 'e' && *p != 'E')
      ଫେରନ୍ତୁ createError("Invalid character in significand");
    ଯଦି (p == begin)
      ଫେରନ୍ତୁ createError("Significand has no digits");
    ଯଦି (dot != end && p - begin == 1)
      ଫେରନ୍ତୁ createError("Significand has no digits");

    /* p points to the first non-digit in the string */
    auto ExpOrErr = readExponent(p + 1, end);
    ଯଦି (!ExpOrErr)
      ଫେରନ୍ତୁ ExpOrErr.takeError();
    D->exponent = *ExpOrErr;

    /* Implied decimal point?  */
    ଯଦି (dot == end)
      dot = p;
  }

  /* If number is all zeroes accept any exponent.  */
  ଯଦି (p != D->firstSigDigit) {
    /* Drop insignificant trailing zeroes.  */
    ଯଦି (p != begin) {
      କର
        କର
          p--;
        ଯେତେବେଳେ (p != begin && *p == '0');
      ଯେତେବେଳେ (p != begin && *p == '.');
    }

    /* Adjust the exponents for any decimal point.  */
    D->exponent += static_cast<APFloat::ExponentType>((dot - p) - (dot > p));
    D->normalizedExponent = (D->exponent +
              static_cast<APFloat::ExponentType>((p - D->firstSigDigit)
                                      - (dot > D->firstSigDigit && dot < p)));
  }

  D->lastSigDigit = p;
  ଫେରନ୍ତୁ Error::success();
}

/* Return the trailing fraction of a hexadecimal number.
   DIGITVALUE is the first hex digit of the fraction, P points to
   the next digit.  */
ସ୍ଥିର Expected<lostFraction>
trailingHexadecimalFraction(StringRef::iterator p, StringRef::iterator end,
                            unsigned ପୂର୍ଣ୍ଣସଂଖ୍ୟା digit_ମୂଲ୍ୟ) {
  /* If the first trailing digit isn't 0 or 8 we can work out the
     fraction immediately.  */
  ଯଦି (digit_ମୂଲ୍ୟ > 8)
    ଫେରନ୍ତୁ lfMoreThanHalf;
  ଅନ୍ୟଥା ଯଦି (digit_ମୂଲ୍ୟ < 8 && digit_ମୂଲ୍ୟ > 0)
    ଫେରନ୍ତୁ lfLessThanHalf;

  // Otherwise we need to find the first non-zero digit.
  ଯେତେବେଳେ (p != end && (*p == '0' || *p == '.'))
    p++;

  ଯଦି (p == end)
    ଫେରନ୍ତୁ createError("Invalid trailing hexadecimal fraction!");

  unsigned hexDigit = hex_Digit_ମୂଲ୍ୟ(*p);

  /* If we ran off the end it is exactly zero or one-half, otherwise
     a little more.  */
  ଯଦି (hexDigit == UINT_MAX)
    ଫେରନ୍ତୁ digit_ମୂଲ୍ୟ == 0 ? lfExactlyZero: lfExactlyHalf;
  ଅନ୍ୟଥା
    ଫେରନ୍ତୁ digit_ମୂଲ୍ୟ == 0 ? lfLessThanHalf: lfMoreThanHalf;
}

/* Return the fraction lost were a bignum truncated losing the least
   significant BITS bits.  */
ସ୍ଥିର lostFraction
lostFractionThroughTruncation(ଅଚଳ APFloatBase::integerPart *parts,
                              unsigned ପୂର୍ଣ୍ଣସଂଖ୍ୟା part_ଗଣନା,
                              unsigned ପୂର୍ଣ୍ଣସଂଖ୍ୟା bits)
{
  unsigned lsb = APInt::tcLSB(parts, part_ଗଣନା);

  /* Note this is guaranteed true if bits == 0, or LSB == UINT_MAX.  */
  ଯଦି (bits <= lsb)
    ଫେରନ୍ତୁ lfExactlyZero;
  ଯଦି (bits == lsb + 1)
    ଫେରନ୍ତୁ lfExactlyHalf;
  ଯଦି (bits <= part_ଗଣନା * APFloatBase::integerPartWidth &&
      APInt::tcExtractBit(parts, bits - 1))
    ଫେରନ୍ତୁ lfMoreThanHalf;

  ଫେରନ୍ତୁ lfLessThanHalf;
}

/* Shift DST right BITS bits noting lost fraction.  */
ସ୍ଥିର lostFraction
shift_ଡାହାଣ(APFloatBase::integerPart *dst, unsigned ପୂର୍ଣ୍ଣସଂଖ୍ୟା parts, unsigned ପୂର୍ଣ୍ଣସଂଖ୍ୟା bits)
{
  lostFraction lost_fraction = lostFractionThroughTruncation(dst, parts, bits);

  APInt::tc_Shift_ଡାହାଣ(dst, parts, bits);

  ଫେରନ୍ତୁ lost_fraction;
}

/* Combine the effect of two lost fractions.  */
ସ୍ଥିର lostFraction
combineLostFractions(lostFraction moreSignificant,
                     lostFraction lessSignificant)
{
  ଯଦି (lessSignificant != lfExactlyZero) {
    ଯଦି (moreSignificant == lfExactlyZero)
      moreSignificant = lfLessThanHalf;
    ଅନ୍ୟଥା ଯଦି (moreSignificant == lfExactlyHalf)
      moreSignificant = lfMoreThanHalf;
  }

  ଫେରନ୍ତୁ moreSignificant;
}

/* The error from the true value, in half-ulps, on multiplying two
   floating point numbers, which differ from the value they
   approximate by at most HUE1 and HUE2 half-ulps, is strictly less
   than the returned value.

   See "How to Read Floating Point Numbers Accurately" by William D
   Clinger.  */
ସ୍ଥିର unsigned ପୂର୍ଣ୍ଣସଂଖ୍ୟା
HUerrBound(ତାର୍କିକ inexactMultiply, unsigned ପୂର୍ଣ୍ଣସଂଖ୍ୟା HUerr1, unsigned ପୂର୍ଣ୍ଣସଂଖ୍ୟା HUerr2)
{
  ପ୍ରତିଜ୍ଞା(HUerr1 < 2 || HUerr2 < 2 || (HUerr1 + HUerr2 < 8));

  ଯଦି (HUerr1 + HUerr2 == 0)
    ଫେରନ୍ତୁ inexactMultiply * 2;  /* <= inexactMultiply half-ulps.  */
  ଅନ୍ୟଥା
    ଫେରନ୍ତୁ inexactMultiply + 2 * (HUerr1 + HUerr2);
}

/* The number of ulps from the boundary (zero, or half if ISNEAREST)
   when the least significant BITS are truncated.  BITS cannot be
   zero.  */
ସ୍ଥିର APFloatBase::integerPart
ulpsFromBoundary(ଅଚଳ APFloatBase::integerPart *parts, unsigned ପୂର୍ଣ୍ଣସଂଖ୍ୟା bits,
                 ତାର୍କିକ isNearest) {
  ପ୍ରତିଜ୍ଞା(bits != 0);

  bits--;
  unsigned ଗଣନା_2 = bits / APFloatBase::integerPartWidth;
  unsigned partBits = bits % APFloatBase::integerPartWidth + 1;

  APFloatBase::integerPart part =
      parts[ଗଣନା_2] & (~(APFloatBase::integerPart)0 >>
                      (APFloatBase::integerPartWidth - partBits));

  APFloatBase::integerPart boundary;
  ଯଦି (isNearest)
    boundary = (APFloatBase::integerPart) 1 << (partBits - 1);
  ଅନ୍ୟଥା
    boundary = 0;

  ଯଦି (ଗଣନା_2 == 0) {
    ଯଦି (part - boundary <= boundary - part)
      ଫେରନ୍ତୁ part - boundary;
    ଅନ୍ୟଥା
      ଫେରନ୍ତୁ boundary - part;
  }

  ଯଦି (part == boundary) {
    ଯେତେବେଳେ (--ଗଣନା_2)
      ଯଦି (parts[ଗଣନା_2])
        ଫେରନ୍ତୁ ~(APFloatBase::integerPart) 0; /* A lot.  */

    ଫେରନ୍ତୁ parts[0];
  } ଅନ୍ୟଥା ଯଦି (part == boundary - 1) {
    ଯେତେବେଳେ (--ଗଣନା_2)
      ଯଦି (~parts[ଗଣନା_2])
        ଫେରନ୍ତୁ ~(APFloatBase::integerPart) 0; /* A lot.  */

    ଫେରନ୍ତୁ -parts[0];
  }

  ଫେରନ୍ତୁ ~(APFloatBase::integerPart) 0; /* A lot.  */
}

ସ୍ଥିର constexpr unsigned PowerOfFivePartCounts[] = {1,  1,  2,  3,   5,  10,
                                                     19, 38, 75, 149, 298};

ସ୍ଥିର constexpr APFloatBase::integerPart PowerOfFiveParts[] = {
#ଅନ୍ତର୍ଭାଵ "APFloatPowerOfFiveTable.inc"
};

static_assert([] {
  unsigned Sum = 0;
  ପାଇଁ (unsigned ଗଣନା_3 : PowerOfFivePartCounts)
    Sum += ଗଣନା_3;
  ଫେରନ୍ତୁ Sum;
}() == std::size(PowerOfFiveParts));

/* Place pow(5, power) in DST, and return the number of parts used.
   DST must be at least one part larger than size of the answer.  */
ସ୍ଥିର unsigned ପୂର୍ଣ୍ଣସଂଖ୍ୟା
powerOf5(APFloatBase::integerPart *dst, unsigned ପୂର୍ଣ୍ଣସଂଖ୍ୟା power) {
  ସ୍ଥିର ଅଚଳ APFloatBase::integerPart firstEightPowers[] = {
      1, 5, 25, 125, 625, 3125, 15625, 78125};

  SmallVector<APFloatBase::integerPart, ଅଙ୍କ_Pow5_Parts_On_ଗଦା> scratch;
  scratch.resize_for_overwrite(powerOfFivePartsForExponent(power));

  APFloatBase::integerPart *p1, *p2;
  ଅଚଳ APFloatBase::integerPart *pow5;

  p1 = dst;
  p2 = scratch.data();

  *p1 = firstEightPowers[power & 7];
  power >>= 3;

  unsigned result = 1;
  pow5 = PowerOfFiveParts;

  ପାଇଁ (unsigned ପୂର୍ଣ୍ଣସଂଖ୍ୟା n = 0; power; power >>= 1, n++) {
    unsigned parts_ଗଣନା = PowerOfFivePartCounts[n];

    ଯଦି (power & 1) {
      APFloatBase::integerPart *tmp;

      APInt::tcFullMultiply(p2, p1, pow5, result, parts_ଗଣନା);
      result += parts_ଗଣନା;
      ଯଦି (p2[result - 1] == 0)
        result--;

      /* Now result is in p1 with partsCount parts and p2 is scratch
         space.  */
      tmp = p1;
      p1 = p2;
      p2 = tmp;
    }

    pow5 += parts_ଗଣନା;
  }

  ଯଦି (p1 != dst)
    APInt::tcAssign(dst, p1, result);

  ଫେରନ୍ତୁ result;
}

/* Zero at the end to avoid modular arithmetic when adding one; used
   when rounding up during hexadecimal output.  */
ସ୍ଥିର ଅଚଳ ଅକ୍ଷର hexDigitsLower[] = "0123456789abcdef0";
ସ୍ଥିର ଅଚଳ ଅକ୍ଷର hexDigitsUpper[] = "0123456789ABCDEF0";
ସ୍ଥିର ଅଚଳ ଅକ୍ଷର infinityL[] = "infinity";
ସ୍ଥିର ଅଚଳ ଅକ୍ଷର infinityU[] = "INFINITY";
ସ୍ଥିର ଅଚଳ ଅକ୍ଷର NaNL[] = "nan";
ସ୍ଥିର ଅଚଳ ଅକ୍ଷର NaNU[] = "NAN";

/* Write out an integerPart in hexadecimal, starting with the most
   significant nibble.  Write out exactly COUNT hexdigits, return
   COUNT.  */
ସ୍ଥିର unsigned ପୂର୍ଣ୍ଣସଂଖ୍ୟା
partAsHex (ଅକ୍ଷର *dst, APFloatBase::integerPart part, unsigned ପୂର୍ଣ୍ଣସଂଖ୍ୟା ଗଣନା_2,
           ଅଚଳ ଅକ୍ଷର *hexDigitChars)
{
  unsigned ପୂର୍ଣ୍ଣସଂଖ୍ୟା result = ଗଣନା_2;

  ପ୍ରତିଜ୍ଞା(ଗଣନା_2 != 0 && ଗଣନା_2 <= APFloatBase::integerPartWidth / 4);

  part >>= (APFloatBase::integerPartWidth - 4 * ଗଣନା_2);
  ଯେତେବେଳେ (ଗଣନା_2--) {
    dst[ଗଣନା_2] = hexDigitChars[part & 0xf];
    part >>= 4;
  }

  ଫେରନ୍ତୁ result;
}

/* Write out an unsigned decimal integer.  */
ସ୍ଥିର ଅକ୍ଷର *writeUnsignedDecimal(ଅକ୍ଷର *dst, unsigned ପୂର୍ଣ୍ଣସଂଖ୍ୟା n) {
  ଅକ୍ଷର buff[40], *p;

  p = buff;
  କର
    *p++ = '0' + n % 10;
  ଯେତେବେଳେ (n /= 10);

  କର
    *dst++ = *--p;
  ଯେତେବେଳେ (p != buff);

  ଫେରନ୍ତୁ dst;
}

/* Write out a signed decimal integer.  */
ସ୍ଥିର ଅକ୍ଷର *writeSignedDecimal(ଅକ୍ଷର *dst, ପୂର୍ଣ୍ଣସଂଖ୍ୟା ମୂଲ୍ୟ_2) {
  ଯଦି (ମୂଲ୍ୟ_2 < 0) {
    *dst++ = '-';
    dst = writeUnsignedDecimal(dst, -(unsigned) ମୂଲ୍ୟ_2);
  } ଅନ୍ୟଥା {
    dst = writeUnsignedDecimal(dst, ମୂଲ୍ୟ_2);
  }

  ଫେରନ୍ତୁ dst;
}

// Compute the ULP of the input using a definition from:
// Jean-Michel Muller. On the definition of ulp(x). [Research Report] RR-5504,
// LIP RR-2005-09, INRIA, LIP. 2005, pp.16. inria-00070503
ସ୍ଥିର APFloat harrisonUlp(ଅଚଳ APFloat &X) {
  ଅଚଳ fltSemantics &Sem = X.getSemantics();
  ବାଛ (X.getCategory()) {
  ସ୍ଥିତି APFloat::fcNaN:
    ଫେରନ୍ତୁ APFloat::getQNaN(Sem);
  ସ୍ଥିତି APFloat::fcInfinity:
    ଫେରନ୍ତୁ APFloat::getInf(Sem);
  ସ୍ଥିତି APFloat::fcZero:
    ଫେରନ୍ତୁ APFloat::getSmallest(Sem);
  ସ୍ଥିତି APFloat::fcNormal:
    ଭାଙ୍ଗ;
  }
  ଯଦି (X.isDenormal() || X.isSmallestNormalized())
    ଫେରନ୍ତୁ APFloat::getSmallest(Sem);
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା Exp = ilogb(X);
  ଯଦି (X.getExactLog2() != INT_MIN)
    Exp -= 1;
  ଫେରନ୍ତୁ scalbn(APFloat::getOne(Sem), Exp - (Sem.precision - 1),
                APFloat::rmNearestTiesToEven);
}

ନାମାକାଶ detail {
/* Constructors.  */
ଶୂନ୍ୟ IEEEFloat::initialize(ଅଚଳ fltSemantics *ourSemantics) {
  semantics = ourSemantics;
  unsigned ଗଣନା_2 = part_ଗଣନା();
  ଯଦି (ଗଣନା_2 > 1)
    significand.parts = ନୂତନ integerPart[ଗଣନା_2];
}

ଶୂନ୍ୟ IEEEFloat::freeSignificand() {
  ଯଦି (needsCleanup())
    delete [] significand.parts;
}

ଶୂନ୍ୟ IEEEFloat::assign(ଅଚଳ IEEEFloat &rhs) {
  ପ୍ରତିଜ୍ଞା(semantics == rhs.semantics);

  sign = rhs.sign;
  category = rhs.category;
  exponent = rhs.exponent;
  ଯଦି (isFiniteNonZero() || category == fcNaN)
    copySignificand(rhs);
}

ଶୂନ୍ୟ IEEEFloat::copySignificand(ଅଚଳ IEEEFloat &rhs) {
  ପ୍ରତିଜ୍ଞା(isFiniteNonZero() || category == fcNaN);
  ପ୍ରତିଜ୍ଞା(rhs.part_ଗଣନା() >= part_ଗଣନା());

  APInt::tcAssign(significandParts(), rhs.significandParts(),
                  part_ଗଣନା());
}

/* Make this number a NaN, with an arbitrary but deterministic value
   for the significand.  If double or longer, this is a signalling NaN,
   which may not be ideal.  If float, this is QNaN(0).  */
ଶୂନ୍ୟ IEEEFloat::makeNaN(ତାର୍କିକ SNaN, ତାର୍କିକ Negative, ଅଚଳ APInt *fill) {
  ଯଦି (semantics->nonFiniteBehavior == fltNonfiniteBehavior::FiniteOnly)
    llvm_unreachable("This floating point format does not support NaN");

  ଯଦି (Negative && !semantics->hasSignedRepr)
    llvm_unreachable(
        "This floating point format does not support signed values");

  category = fcNaN;
  sign = Negative;
  exponent = exponentNaN();

  integerPart *significand = significandParts();
  unsigned ଅଙ୍କ_Parts = part_ଗଣନା();

  APInt fill_storage;
  ଯଦି (semantics->nonFiniteBehavior == fltNonfiniteBehavior::NanOnly) {
    // Finite-only types do not distinguish signalling and quiet NaN, so
    // make them all signalling.
    SNaN = ମିଥ୍ୟା;
    ଯଦି (semantics->nanEncoding == fltNanEncoding::NegativeZero) {
      sign = ସତ୍ୟ;
      fill_storage = APInt::getZero(semantics->precision - 1);
    } ଅନ୍ୟଥା {
      fill_storage = APInt::getAllOnes(semantics->precision - 1);
    }
    fill = &fill_storage;
  }

  // Set the significand bits to the fill.
  ଯଦି (!fill || fill->get_ଅଙ୍କ_ଶବ୍ଦଗୁଡ଼ିକ() < ଅଙ୍କ_Parts)
    APInt::tcSet(significand, 0, ଅଙ୍କ_Parts);
  ଯଦି (fill) {
    APInt::tcAssign(significand, fill->getRawData(),
                    std::min(fill->get_ଅଙ୍କ_ଶବ୍ଦଗୁଡ଼ିକ(), ଅଙ୍କ_Parts));

    // Zero out the excess bits of the significand.
    unsigned bitsToPreserve = semantics->precision - 1;
    unsigned part = bitsToPreserve / 64;
    bitsToPreserve %= 64;
    significand[part] &= ((1ULL << bitsToPreserve) - 1);
    ପାଇଁ (part++; part != ଅଙ୍କ_Parts; ++part)
      significand[part] = 0;
  }

  unsigned QNaNBit =
      (semantics->precision >= 2) ? (semantics->precision - 2) : 0;

  ଯଦି (SNaN) {
    // We always have to clear the QNaN bit to make it an SNaN.
    APInt::tcClearBit(significand, QNaNBit);

    // If there are no bits set in the payload, we have to set
    // *something* to make it a NaN instead of an infinity;
    // conventionally, this is the next bit down from the QNaN bit.
    ଯଦି (APInt::tcIsZero(significand, ଅଙ୍କ_Parts))
      APInt::tcSetBit(significand, QNaNBit - 1);
  } ଅନ୍ୟଥା ଯଦି (semantics->nanEncoding == fltNanEncoding::NegativeZero) {
    // The only NaN is a quiet NaN, and it has no bits sets in the significand.
    // Do nothing.
  } ଅନ୍ୟଥା {
    // We always have to set the QNaN bit to make it a QNaN.
    APInt::tcSetBit(significand, QNaNBit);
  }

  // For x87 extended precision, we want to make a NaN, not a
  // pseudo-NaN.  Maybe we should expose the ability to make
  // pseudo-NaNs?
  ଯଦି (semantics == &APFloatBase::semX87DoubleExtended)
    APInt::tcSetBit(significand, QNaNBit + 1);
}

IEEEFloat &IEEEFloat::operator=(ଅଚଳ IEEEFloat &rhs) {
  ଯଦି (ଏହି != &rhs) {
    ଯଦି (semantics != rhs.semantics) {
      freeSignificand();
      initialize(rhs.semantics);
    }
    assign(rhs);
  }

  ଫେରନ୍ତୁ *ଏହି;
}

IEEEFloat &IEEEFloat::operator=(IEEEFloat &&rhs) {
  freeSignificand();

  semantics = rhs.semantics;
  significand = rhs.significand;
  exponent = rhs.exponent;
  category = rhs.category;
  sign = rhs.sign;

  rhs.semantics = &APFloatBase::semBogus;
  ଫେରନ୍ତୁ *ଏହି;
}

ତାର୍କିକ IEEEFloat::isDenormal() ଅଚଳ {
  ଫେରନ୍ତୁ getSemantics().hasDenormals && isFiniteNonZero() &&
         (exponent == semantics->minExponent) &&
         (APInt::tcExtractBit(significandParts(), semantics->precision - 1) ==
          0);
}

ତାର୍କିକ IEEEFloat::isSmallest() ଅଚଳ {
  // The smallest number by magnitude in our format will be the smallest
  // denormal, i.e. the floating point number with exponent being minimum
  // exponent and significand bitwise equal to 1 (i.e. with MSB equal to 0).
  ଫେରନ୍ତୁ isFiniteNonZero() && exponent == semantics->minExponent &&
    significandMSB() == 0;
}

ତାର୍କିକ IEEEFloat::isSmallestNormalized() ଅଚଳ {
  ଫେରନ୍ତୁ getCategory() == fcNormal && exponent == semantics->minExponent &&
         isSignificandAllZerosExceptMSB();
}

unsigned ପୂର୍ଣ୍ଣସଂଖ୍ୟା IEEEFloat::get_ଅଙ୍କ_ଉପର_Bits() ଅଚଳ {
  ଅଚଳ unsigned ପୂର୍ଣ୍ଣସଂଖ୍ୟା Part_ଗଣନା = part_ଗଣନା_For_Bits(semantics->precision);
  ଅଚଳ unsigned ପୂର୍ଣ୍ଣସଂଖ୍ୟା Bits = Part_ଗଣନା * integerPartWidth;

  // Compute how many bits are used in the final word.
  // When precision is just 1, it represents the 'Pth'
  // Precision bit and not the actual significand bit.
  ଅଚଳ unsigned ପୂର୍ଣ୍ଣସଂଖ୍ୟା ଅଙ୍କ_ଉପର_Bits = (semantics->precision > 1)
                                       ? (Bits - semantics->precision + 1)
                                       : (Bits - semantics->precision);
  ଫେରନ୍ତୁ ଅଙ୍କ_ଉପର_Bits;
}

ତାର୍କିକ IEEEFloat::isSignificandAllOnes() ଅଚଳ {
  // Test if the significand excluding the integral bit is all ones. This allows
  // us to test for binade boundaries.
  ଅଚଳ integerPart *Parts = significandParts();
  ଅଚଳ unsigned Part_ଗଣନା = part_ଗଣନା_For_Bits(semantics->precision);
  ପାଇଁ (unsigned i = 0; i < Part_ଗଣନା - 1; i++)
    ଯଦି (~Parts[i])
      ଫେରନ୍ତୁ ମିଥ୍ୟା;

  // Set the unused high bits to all ones when we compare.
  ଅଚଳ unsigned ଅଙ୍କ_ଉପର_Bits = get_ଅଙ୍କ_ଉପର_Bits();
  ପ୍ରତିଜ୍ଞା(ଅଙ୍କ_ଉପର_Bits <= integerPartWidth && ଅଙ୍କ_ଉପର_Bits > 0 &&
         "Can not have more high bits to fill than integerPartWidth");
  ଅଚଳ integerPart ଉପର_Bit_Fill =
    ~integerPart(0) << (integerPartWidth - ଅଙ୍କ_ଉପର_Bits);
  ଯଦି ((semantics->precision <= 1) || (~(Parts[Part_ଗଣନା - 1] | ଉପର_Bit_Fill)))
    ଫେରନ୍ତୁ ମିଥ୍ୟା;

  ଫେରନ୍ତୁ ସତ୍ୟ;
}

ତାର୍କିକ IEEEFloat::isSignificandAllOnesExceptLSB() ଅଚଳ {
  // Test if the significand excluding the integral bit is all ones except for
  // the least significant bit.
  ଅଚଳ integerPart *Parts = significandParts();

  ଯଦି (Parts[0] & 1)
    ଫେରନ୍ତୁ ମିଥ୍ୟା;

  ଅଚଳ unsigned Part_ଗଣନା = part_ଗଣନା_For_Bits(semantics->precision);
  ପାଇଁ (unsigned i = 0; i < Part_ଗଣନା - 1; i++) {
    ଯଦି (~Parts[i] & ~unsigned{!i})
      ଫେରନ୍ତୁ ମିଥ୍ୟା;
  }

  // Set the unused high bits to all ones when we compare.
  ଅଚଳ unsigned ଅଙ୍କ_ଉପର_Bits = get_ଅଙ୍କ_ଉପର_Bits();
  ପ୍ରତିଜ୍ଞା(ଅଙ୍କ_ଉପର_Bits <= integerPartWidth && ଅଙ୍କ_ଉପର_Bits > 0 &&
         "Can not have more high bits to fill than integerPartWidth");
  ଅଚଳ integerPart ଉପର_Bit_Fill = ~integerPart(0)
                                  << (integerPartWidth - ଅଙ୍କ_ଉପର_Bits);
  ଯଦି (~(Parts[Part_ଗଣନା - 1] | ଉପର_Bit_Fill | 0x1))
    ଫେରନ୍ତୁ ମିଥ୍ୟା;

  ଫେରନ୍ତୁ ସତ୍ୟ;
}

ତାର୍କିକ IEEEFloat::isSignificandAllZeros() ଅଚଳ {
  // Test if the significand excluding the integral bit is all zeros. This
  // allows us to test for binade boundaries.
  ଅଚଳ integerPart *Parts = significandParts();
  ଅଚଳ unsigned Part_ଗଣନା = part_ଗଣନା_For_Bits(semantics->precision);

  ପାଇଁ (unsigned i = 0; i < Part_ଗଣନା - 1; i++)
    ଯଦି (Parts[i])
      ଫେରନ୍ତୁ ମିଥ୍ୟା;

  // Compute how many bits are used in the final word.
  ଅଚଳ unsigned ଅଙ୍କ_ଉପର_Bits = get_ଅଙ୍କ_ଉପର_Bits();
  ପ୍ରତିଜ୍ଞା(ଅଙ୍କ_ଉପର_Bits < integerPartWidth && "Can not have more high bits to "
         "clear than integerPartWidth");
  ଅଚଳ integerPart ଉପର_Bit_Mask = ~integerPart(0) >> ଅଙ୍କ_ଉପର_Bits;

  ଯଦି ((semantics->precision > 1) && (Parts[Part_ଗଣନା - 1] & ଉପର_Bit_Mask))
    ଫେରନ୍ତୁ ମିଥ୍ୟା;

  ଫେରନ୍ତୁ ସତ୍ୟ;
}

ତାର୍କିକ IEEEFloat::isSignificandAllZerosExceptMSB() ଅଚଳ {
  ଅଚଳ integerPart *Parts = significandParts();
  ଅଚଳ unsigned Part_ଗଣନା = part_ଗଣନା_For_Bits(semantics->precision);

  ପାଇଁ (unsigned i = 0; i < Part_ଗଣନା - 1; i++) {
    ଯଦି (Parts[i])
      ଫେରନ୍ତୁ ମିଥ୍ୟା;
  }

  ଅଚଳ unsigned ଅଙ୍କ_ଉପର_Bits = get_ଅଙ୍କ_ଉପର_Bits();
  ଅଚଳ integerPart MSBMask = integerPart(1)
                              << (integerPartWidth - ଅଙ୍କ_ଉପର_Bits);
  ଫେରନ୍ତୁ ((semantics->precision <= 1) || (Parts[Part_ଗଣନା - 1] == MSBMask));
}

ତାର୍କିକ IEEEFloat::isLargest() ଅଚଳ {
  ତାର୍କିକ IsMaxExp = isFiniteNonZero() && exponent == semantics->maxExponent;
  ଯଦି (semantics->nonFiniteBehavior == fltNonfiniteBehavior::NanOnly &&
      semantics->nanEncoding == fltNanEncoding::AllOnes) {
    // The largest number by magnitude in our format will be the floating point
    // number with maximum exponent and with significand that is all ones except
    // the LSB.
    ଫେରନ୍ତୁ (IsMaxExp && APFloat::hasSignificand(*semantics))
               ? isSignificandAllOnesExceptLSB()
               : IsMaxExp;
  } ଅନ୍ୟଥା {
    // The largest number by magnitude in our format will be the floating point
    // number with maximum exponent and with significand that is all ones.
    ଫେରନ୍ତୁ IsMaxExp && isSignificandAllOnes();
  }
}

ତାର୍କିକ IEEEFloat::isInteger() ଅଚଳ {
  // This could be made more efficient; I'm going for obviously correct.
  ଯଦି (!isFinite()) ଫେରନ୍ତୁ ମିଥ୍ୟା;
  IEEEFloat truncated = *ଏହି;
  truncated.roundToIntegral(rmTowardZero);
  ଫେରନ୍ତୁ compare(truncated) == cmpEqual;
}

ତାର୍କିକ IEEEFloat::bitwiseIsEqual(ଅଚଳ IEEEFloat &rhs) ଅଚଳ {
  ଯଦି (ଏହି == &rhs)
    ଫେରନ୍ତୁ ସତ୍ୟ;
  ଯଦି (semantics != rhs.semantics ||
      category != rhs.category ||
      sign != rhs.sign)
    ଫେରନ୍ତୁ ମିଥ୍ୟା;
  ଯଦି (category==fcZero || category==fcInfinity)
    ଫେରନ୍ତୁ ସତ୍ୟ;

  ଯଦି (isFiniteNonZero() && exponent != rhs.exponent)
    ଫେରନ୍ତୁ ମିଥ୍ୟା;

  ଫେରନ୍ତୁ std::equal(significandParts(), significandParts() + part_ଗଣନା(),
                    rhs.significandParts());
}

IEEEFloat::IEEEFloat(ଅଚଳ fltSemantics &ourSemantics, integerPart ମୂଲ୍ୟ_2) {
  initialize(&ourSemantics);
  sign = 0;
  category = fcNormal;
  zeroSignificand();
  exponent = ourSemantics.precision - 1;
  significandParts()[0] = ମୂଲ୍ୟ_2;
  normalize(rmNearestTiesToEven, lfExactlyZero);
}

IEEEFloat::IEEEFloat(ଅଚଳ fltSemantics &ourSemantics) {
  initialize(&ourSemantics);
  // The Float8E8MOFNU format does not have a representation
  // for zero. So, use the closest representation instead.
  // Moreover, the all-zero encoding represents a valid
  // normal value (which is the smallestNormalized here).
  // Hence, we call makeSmallestNormalized (where category is
  // 'fcNormal') instead of makeZero (where category is 'fcZero').
  ourSemantics.hasZero ? makeZero(ମିଥ୍ୟା) : makeSmallestNormalized(ମିଥ୍ୟା);
}

// Delegate to the previous constructor, because later copy constructor may
// actually inspects category, which can't be garbage.
IEEEFloat::IEEEFloat(ଅଚଳ fltSemantics &ourSemantics, uninitializedTag tag)
    : IEEEFloat(ourSemantics) {}

IEEEFloat::IEEEFloat(ଅଚଳ IEEEFloat &rhs) {
  initialize(rhs.semantics);
  assign(rhs);
}

IEEEFloat::IEEEFloat(IEEEFloat &&rhs) : semantics(&APFloatBase::semBogus) {
  *ଏହି = std::move(rhs);
}

IEEEFloat::~IEEEFloat() { freeSignificand(); }

unsigned ପୂର୍ଣ୍ଣସଂଖ୍ୟା IEEEFloat::part_ଗଣନା() ଅଚଳ {
  ଫେରନ୍ତୁ part_ଗଣନା_For_Bits(semantics->precision + 1);
}

ଅଚଳ APFloat::integerPart *IEEEFloat::significandParts() ଅଚଳ {
  ଫେରନ୍ତୁ const_cast<IEEEFloat *>(ଏହି)->significandParts();
}

APFloat::integerPart *IEEEFloat::significandParts() {
  ଯଦି (part_ଗଣନା() > 1)
    ଫେରନ୍ତୁ significand.parts;
  ଅନ୍ୟଥା
    ଫେରନ୍ତୁ &significand.part;
}

ଶୂନ୍ୟ IEEEFloat::zeroSignificand() {
  APInt::tcSet(significandParts(), 0, part_ଗଣନା());
}

/* Increment an fcNormal floating point number's significand.  */
ଶୂନ୍ୟ IEEEFloat::incrementSignificand() {
  [[maybe_unused]] integerPart carry =
      APInt::tcIncrement(significandParts(), part_ଗଣନା());

  /* Our callers should never cause us to overflow.  */
  ପ୍ରତିଜ୍ଞା(carry == 0);
}

/* Add the significand of the RHS.  Returns the carry flag.  */
APFloat::integerPart IEEEFloat::addSignificand(ଅଚଳ IEEEFloat &rhs) {
  integerPart *parts = significandParts();

  ପ୍ରତିଜ୍ଞା(semantics == rhs.semantics);
  ପ୍ରତିଜ୍ଞା(exponent == rhs.exponent);

  ଫେରନ୍ତୁ APInt::tcAdd(parts, rhs.significandParts(), 0, part_ଗଣନା());
}

/* Subtract the significand of the RHS with a borrow flag.  Returns
   the borrow flag.  */
APFloat::integerPart IEEEFloat::subtractSignificand(ଅଚଳ IEEEFloat &rhs,
                                                    integerPart borrow) {
  integerPart *parts = significandParts();

  ପ୍ରତିଜ୍ଞା(semantics == rhs.semantics);
  ପ୍ରତିଜ୍ଞା(exponent == rhs.exponent);

  ଫେରନ୍ତୁ APInt::tcSubtract(parts, rhs.significandParts(), borrow,
                           part_ଗଣନା());
}

/* Multiply the significand of the RHS.  If ADDEND is non-NULL, add it
   on to the full-precision result of the multiplication.  Returns the
   lost fraction.  */
lostFraction IEEEFloat::multiplySignificand(ଅଚଳ IEEEFloat &rhs,
                                            IEEEFloat addend,
                                            ତାର୍କିକ ignoreAddend) {
  integerPart scratch[4];
  ତାର୍କିକ ignored;

  ପ୍ରତିଜ୍ଞା(semantics == rhs.semantics);

  unsigned precision = semantics->precision;

  // Allocate space for twice as many bits as the original significand, plus one
  // extra bit for the addition to overflow into.
  unsigned new_Parts_ଗଣନା = part_ଗଣନା_For_Bits(precision * 2 + 1);

  // FIXME: Replace with SmallVector<4>.
  integerPart *fullSignificand =
      new_Parts_ଗଣନା > 4 ? ନୂତନ integerPart[new_Parts_ଗଣନା] : scratch;

  integerPart *lhsSignificand = significandParts();
  unsigned parts_ଗଣନା = part_ଗଣନା();

  APInt::tcFullMultiply(fullSignificand, lhsSignificand,
                        rhs.significandParts(), parts_ଗଣନା, parts_ଗଣନା);

  lostFraction lost_fraction = lfExactlyZero;
  // One, not zero, based MSB.
  unsigned omsb = APInt::tcMSB(fullSignificand, new_Parts_ଗଣନା) + 1;
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

  ଯଦି (!ignoreAddend && addend.isNonZero()) {
    // The intermediate result of the multiplication has "2 * precision"
    // signicant bit; adjust the addend to be consistent with mul result.
    //
    Significand savedSignificand = significand;
    ଅଚଳ fltSemantics *savedSemantics = semantics;

    // Normalize our MSB to one below the top bit to allow for overflow.
    unsigned extendedPrecision = 2 * precision + 1;
    ଯଦି (omsb != extendedPrecision - 1) {
      ପ୍ରତିଜ୍ଞା(extendedPrecision > omsb);
      APInt::tc_Shift_ବାମ(fullSignificand, new_Parts_ଗଣନା,
                         (extendedPrecision - 1) - omsb);
      exponent -= (extendedPrecision - 1) - omsb;
    }

    /* Create new semantics.  */
    fltSemantics extendedSemantics = *semantics;
    extendedSemantics.precision = extendedPrecision;

    ଯଦି (new_Parts_ଗଣନା == 1)
      significand.part = fullSignificand[0];
    ଅନ୍ୟଥା
      significand.parts = fullSignificand;
    semantics = &extendedSemantics;

    // Make a copy so we can convert it to the extended semantics.
    // Note that we cannot convert the addend directly, as the extendedSemantics
    // is a local variable (which we take a reference to).
    IEEEFloat extendedAddend(addend);
    [[maybe_unused]] opStatus status = extendedAddend.convert(
        extendedSemantics, APFloat::rmTowardZero, &ignored);
    ପ୍ରତିଜ୍ଞା(status == APFloat::opOK);

    // Shift the significand of the addend right by one bit. This guarantees
    // that the high bit of the significand is zero (same as fullSignificand),
    // so the addition will overflow (if it does overflow at all) into the top bit.
    lost_fraction = extendedAddend.shift_Significand_ଡାହାଣ(1);
    ପ୍ରତିଜ୍ଞା(lost_fraction == lfExactlyZero &&
           "Lost precision while shifting addend for fused-multiply-add.");

    lost_fraction = addOrSubtractSignificand(extendedAddend, ମିଥ୍ୟା);

    /* Restore our state.  */
    ଯଦି (new_Parts_ଗଣନା == 1)
      fullSignificand[0] = significand.part;
    significand = savedSignificand;
    semantics = savedSemantics;

    omsb = APInt::tcMSB(fullSignificand, new_Parts_ଗଣନା) + 1;
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
  ଯଦି (omsb > precision) {
    unsigned ପୂର୍ଣ୍ଣସଂଖ୍ୟା bits, significantParts;
    lostFraction lf;

    bits = omsb - precision;
    significantParts = part_ଗଣନା_For_Bits(omsb);
    lf = shift_ଡାହାଣ(fullSignificand, significantParts, bits);
    lost_fraction = combineLostFractions(lf, lost_fraction);
    exponent += bits;
  }

  APInt::tcAssign(lhsSignificand, fullSignificand, parts_ଗଣନା);

  ଯଦି (new_Parts_ଗଣନା > 4)
    delete [] fullSignificand;

  ଫେରନ୍ତୁ lost_fraction;
}

lostFraction IEEEFloat::multiplySignificand(ଅଚଳ IEEEFloat &rhs) {
  // When the given semantics has zero, the addend here is a zero.
  // i.e . it belongs to the 'fcZero' category.
  // But when the semantics does not support zero, we need to
  // explicitly convey that this addend should be ignored
  // for multiplication.
  ଫେରନ୍ତୁ multiplySignificand(rhs, IEEEFloat(*semantics), !semantics->hasZero);
}

/* Multiply the significands of LHS and RHS to DST.  */
lostFraction IEEEFloat::divideSignificand(ଅଚଳ IEEEFloat &rhs) {
  integerPart scratch[4];

  ପ୍ରତିଜ୍ଞା(semantics == rhs.semantics);

  integerPart *lhsSignificand = significandParts();
  ଅଚଳ integerPart *rhsSignificand = rhs.significandParts();
  unsigned parts_ଗଣନା = part_ଗଣନା();

  integerPart *dividend =
      parts_ଗଣନା > 2 ? ନୂତନ integerPart[parts_ଗଣନା * 2] : scratch;
  integerPart *ଭାଜକ = dividend + parts_ଗଣନା;

  /* Copy the dividend and divisor as they will be modified in-place.  */
  ପାଇଁ (unsigned i = 0; i < parts_ଗଣନା; i++) {
    dividend[i] = lhsSignificand[i];
    ଭାଜକ[i] = rhsSignificand[i];
    lhsSignificand[i] = 0;
  }

  exponent -= rhs.exponent;

  unsigned ପୂର୍ଣ୍ଣସଂଖ୍ୟା precision = semantics->precision;

  /* Normalize the divisor.  */
  unsigned bit = precision - APInt::tcMSB(ଭାଜକ, parts_ଗଣନା) - 1;
  ଯଦି (bit) {
    exponent += bit;
    APInt::tc_Shift_ବାମ(ଭାଜକ, parts_ଗଣନା, bit);
  }

  /* Normalize the dividend.  */
  bit = precision - APInt::tcMSB(dividend, parts_ଗଣନା) - 1;
  ଯଦି (bit) {
    exponent -= bit;
    APInt::tc_Shift_ବାମ(dividend, parts_ଗଣନା, bit);
  }

  /* Ensure the dividend >= divisor initially for the loop below.
     Incidentally, this means that the division loop below is
     guaranteed to set the integer bit to one.  */
  ଯଦି (APInt::tcCompare(dividend, ଭାଜକ, parts_ଗଣନା) < 0) {
    exponent--;
    APInt::tc_Shift_ବାମ(dividend, parts_ଗଣନା, 1);
    ପ୍ରତିଜ୍ଞା(APInt::tcCompare(dividend, ଭାଜକ, parts_ଗଣନା) >= 0);
  }

  /* Long division.  */
  ପାଇଁ (bit = precision; bit; bit -= 1) {
    ଯଦି (APInt::tcCompare(dividend, ଭାଜକ, parts_ଗଣନା) >= 0) {
      APInt::tcSubtract(dividend, ଭାଜକ, 0, parts_ଗଣନା);
      APInt::tcSetBit(lhsSignificand, bit - 1);
    }

    APInt::tc_Shift_ବାମ(dividend, parts_ଗଣନା, 1);
  }

  /* Figure out the lost fraction.  */
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା cmp = APInt::tcCompare(dividend, ଭାଜକ, parts_ଗଣନା);

  lostFraction lost_fraction;
  ଯଦି (cmp > 0)
    lost_fraction = lfMoreThanHalf;
  ଅନ୍ୟଥା ଯଦି (cmp == 0)
    lost_fraction = lfExactlyHalf;
  ଅନ୍ୟଥା ଯଦି (APInt::tcIsZero(dividend, parts_ଗଣନା))
    lost_fraction = lfExactlyZero;
  ଅନ୍ୟଥା
    lost_fraction = lfLessThanHalf;

  ଯଦି (parts_ଗଣନା > 2)
    delete [] dividend;

  ଫେରନ୍ତୁ lost_fraction;
}

unsigned ପୂର୍ଣ୍ଣସଂଖ୍ୟା IEEEFloat::significandMSB() ଅଚଳ {
  ଫେରନ୍ତୁ APInt::tcMSB(significandParts(), part_ଗଣନା());
}

unsigned ପୂର୍ଣ୍ଣସଂଖ୍ୟା IEEEFloat::significandLSB() ଅଚଳ {
  ଫେରନ୍ତୁ APInt::tcLSB(significandParts(), part_ଗଣନା());
}

/* Note that a zero result is NOT normalized to fcZero.  */
lostFraction IEEEFloat::shift_Significand_ଡାହାଣ(unsigned ପୂର୍ଣ୍ଣସଂଖ୍ୟା bits) {
  /* Our exponent should not overflow.  */
  ପ୍ରତିଜ୍ଞା((ExponentType) (exponent + bits) >= exponent);

  exponent += bits;

  ଫେରନ୍ତୁ shift_ଡାହାଣ(significandParts(), part_ଗଣନା(), bits);
}

/* Shift the significand left BITS bits, subtract BITS from its exponent.  */
ଶୂନ୍ୟ IEEEFloat::shift_Significand_ବାମ(unsigned ପୂର୍ଣ୍ଣସଂଖ୍ୟା bits) {
  ପ୍ରତିଜ୍ଞା(bits < semantics->precision ||
         (semantics->precision == 1 && bits <= 1));

  ଯଦି (bits) {
    unsigned ପୂର୍ଣ୍ଣସଂଖ୍ୟା parts_ଗଣନା = part_ଗଣନା();

    APInt::tc_Shift_ବାମ(significandParts(), parts_ଗଣନା, bits);
    exponent -= bits;

    ପ୍ରତିଜ୍ଞା(!APInt::tcIsZero(significandParts(), parts_ଗଣନା));
  }
}

APFloat::cmpResult IEEEFloat::compare_Absolute_ମୂଲ୍ୟ(ଅଚଳ IEEEFloat &rhs) ଅଚଳ {
  ପ୍ରତିଜ୍ଞା(semantics == rhs.semantics);
  ପ୍ରତିଜ୍ଞା(isFiniteNonZero());
  ପ୍ରତିଜ୍ଞା(rhs.isFiniteNonZero());

  ପୂର୍ଣ୍ଣସଂଖ୍ୟା compare = exponent - rhs.exponent;

  /* If exponents are equal, do an unsigned bignum comparison of the
     significands.  */
  ଯଦି (compare == 0)
    compare = APInt::tcCompare(significandParts(), rhs.significandParts(),
                               part_ଗଣନା());

  ଯଦି (compare > 0)
    ଫେରନ୍ତୁ cmpGreaterThan;
  ଅନ୍ୟଥା ଯଦି (compare < 0)
    ଫେରନ୍ତୁ cmpLessThan;
  ଅନ୍ୟଥା
    ଫେରନ୍ତୁ cmpEqual;
}

/* Set the least significant BITS bits of a bignum, clear the
   rest.  */
ସ୍ଥିର ଶୂନ୍ୟ tcSetLeastSignificantBits(APInt::WordType *dst, unsigned parts,
                                      unsigned bits) {
  unsigned i = 0;
  ଯେତେବେଳେ (bits > APInt::APINT_BITS_PER_WORD) {
    dst[i++] = ~(APInt::WordType)0;
    bits -= APInt::APINT_BITS_PER_WORD;
  }

  ଯଦି (bits)
    dst[i++] = ~(APInt::WordType)0 >> (APInt::APINT_BITS_PER_WORD - bits);

  ଯେତେବେଳେ (i < parts)
    dst[i++] = 0;
}

/* Handle overflow.  Sign is preserved.  We either become infinity or
   the largest finite number.  */
APFloat::opStatus IEEEFloat::handleOverflow(roundingMode rounding_mode) {
  ଯଦି (semantics->nonFiniteBehavior != fltNonfiniteBehavior::FiniteOnly) {
    /* Infinity?  */
    ଯଦି (rounding_mode == rmNearestTiesToEven ||
        rounding_mode == rmNearestTiesToAway ||
        (rounding_mode == rmTowardPositive && !sign) ||
        (rounding_mode == rmTowardNegative && sign)) {
      ଯଦି (semantics->nonFiniteBehavior == fltNonfiniteBehavior::NanOnly)
        makeNaN(ମିଥ୍ୟା, sign);
      ଅନ୍ୟଥା
        category = fcInfinity;
      ଫେରନ୍ତୁ static_cast<opStatus>(opOverflow | opInexact);
    }
  }

  /* Otherwise we become the largest finite number.  */
  category = fcNormal;
  exponent = semantics->maxExponent;
  tcSetLeastSignificantBits(significandParts(), part_ଗଣନା(),
                            semantics->precision);
  ଯଦି (semantics->nonFiniteBehavior == fltNonfiniteBehavior::NanOnly &&
      semantics->nanEncoding == fltNanEncoding::AllOnes)
    APInt::tcClearBit(significandParts(), 0);

  ଫେରନ୍ତୁ opInexact;
}

/* Returns TRUE if, when truncating the current number, with BIT the
   new LSB, with the given lost fraction and rounding mode, the result
   would need to be rounded away from zero (i.e., by increasing the
   signficand).  This routine must work for fcZero of both signs, and
   fcNormal numbers.  */
ତାର୍କିକ IEEEFloat::roundAwayFromZero(roundingMode rounding_mode,
                                  lostFraction lost_fraction,
                                  unsigned ପୂର୍ଣ୍ଣସଂଖ୍ୟା bit) ଅଚଳ {
  /* NaNs and infinities should not have lost fractions.  */
  ପ୍ରତିଜ୍ଞା(isFiniteNonZero() || category == fcZero);

  /* Current callers never pass this so we don't handle it.  */
  ପ୍ରତିଜ୍ଞା(lost_fraction != lfExactlyZero);

  ବାଛ (rounding_mode) {
  ସ୍ଥିତି rmNearestTiesToAway:
    ଫେରନ୍ତୁ lost_fraction == lfExactlyHalf || lost_fraction == lfMoreThanHalf;

  ସ୍ଥିତି rmNearestTiesToEven:
    ଯଦି (lost_fraction == lfMoreThanHalf)
      ଫେରନ୍ତୁ ସତ୍ୟ;

    /* Our zeroes don't have a significand to test.  */
    ଯଦି (lost_fraction == lfExactlyHalf && category != fcZero)
      ଫେରନ୍ତୁ APInt::tcExtractBit(significandParts(), bit);

    ଫେରନ୍ତୁ ମିଥ୍ୟା;

  ସ୍ଥିତି rmTowardZero:
    ଫେରନ୍ତୁ ମିଥ୍ୟା;

  ସ୍ଥିତି rmTowardPositive:
    ଫେରନ୍ତୁ !sign;

  ସ୍ଥିତି rmTowardNegative:
    ଫେରନ୍ତୁ sign;

  ପୂର୍ବନିର୍ଦ୍ଧାରିତ:
    ଭାଙ୍ଗ;
  }
  llvm_unreachable("Invalid rounding mode found");
}

APFloat::opStatus IEEEFloat::normalize(roundingMode rounding_mode,
                                       lostFraction lost_fraction) {
  ଯଦି (!isFiniteNonZero())
    ଫେରନ୍ତୁ opOK;

  /* Before rounding normalize the exponent of fcNormal numbers.  */
  /* One, not zero, based MSB.  */
  unsigned omsb = significandMSB() + 1;

  // Only skip this `if` if the value is exactly zero.
  ଯଦି (omsb || lost_fraction != lfExactlyZero) {
    /* OMSB is numbered from 1.  We want to place it in the integer
       bit numbered PRECISION if possible, with a compensating change in
       the exponent.  */
    ପୂର୍ଣ୍ଣସଂଖ୍ୟା exponentChange = omsb - semantics->precision;

    /* If the resulting exponent is too high, overflow according to
       the rounding mode.  */
    ଯଦି (exponent + exponentChange > semantics->maxExponent)
      ଫେରନ୍ତୁ handleOverflow(rounding_mode);

    /* Subnormal numbers have exponent minExponent, and their MSB
       is forced based on that.  */
    ଯଦି (exponent + exponentChange < semantics->minExponent)
      exponentChange = semantics->minExponent - exponent;

    /* Shifting left is easy as we don't lose precision.  */
    ଯଦି (exponentChange < 0) {
      ପ୍ରତିଜ୍ଞା(lost_fraction == lfExactlyZero);

      shift_Significand_ବାମ(-exponentChange);

      ଫେରନ୍ତୁ opOK;
    }

    ଯଦି (exponentChange > 0) {
      lostFraction lf;

      /* Shift right and capture any new lost fraction.  */
      lf = shift_Significand_ଡାହାଣ(exponentChange);

      lost_fraction = combineLostFractions(lf, lost_fraction);

      /* Keep OMSB up-to-date.  */
      ଯଦି (omsb > (unsigned) exponentChange)
        omsb -= exponentChange;
      ଅନ୍ୟଥା
        omsb = 0;
    }
  }

  // The all-ones values is an overflow if NaN is all ones. If NaN is
  // represented by negative zero, then it is a valid finite value.
  ଯଦି (semantics->nonFiniteBehavior == fltNonfiniteBehavior::NanOnly &&
      semantics->nanEncoding == fltNanEncoding::AllOnes &&
      exponent == semantics->maxExponent && isSignificandAllOnes())
    ଫେରନ୍ତୁ handleOverflow(rounding_mode);

  /* Now round the number according to rounding_mode given the lost
     fraction.  */

  /* As specified in IEEE 754, since we do not trap we do not report
     underflow for exact results.  */
  ଯଦି (lost_fraction == lfExactlyZero) {
    /* Canonicalize zeroes.  */
    ଯଦି (omsb == 0) {
      category = fcZero;
      ଯଦି (semantics->nanEncoding == fltNanEncoding::NegativeZero)
        sign = ମିଥ୍ୟା;
      ଯଦି (!semantics->hasZero)
        makeSmallestNormalized(ମିଥ୍ୟା);
    }

    ଫେରନ୍ତୁ opOK;
  }

  /* Increment the significand if we're rounding away from zero.  */
  ଯଦି (roundAwayFromZero(rounding_mode, lost_fraction, 0)) {
    ଯଦି (omsb == 0)
      exponent = semantics->minExponent;

    incrementSignificand();
    omsb = significandMSB() + 1;

    /* Did the significand increment overflow?  */
    ଯଦି (omsb == (unsigned) semantics->precision + 1) {
      /* Renormalize by incrementing the exponent and shifting our
         significand right one.  However if we already have the
         maximum exponent we overflow to infinity.  */
      ଯଦି (exponent == semantics->maxExponent)
        // Invoke overflow handling with a rounding mode that will guarantee
        // that the result gets turned into the correct infinity representation.
        // This is needed instead of just setting the category to infinity to
        // account for 8-bit floating point types that have no inf, only NaN.
        ଫେରନ୍ତୁ handleOverflow(sign ? rmTowardNegative : rmTowardPositive);

      shift_Significand_ଡାହାଣ(1);

      ଫେରନ୍ତୁ opInexact;
    }

    // The all-ones values is an overflow if NaN is all ones. If NaN is
    // represented by negative zero, then it is a valid finite value.
    ଯଦି (semantics->nonFiniteBehavior == fltNonfiniteBehavior::NanOnly &&
        semantics->nanEncoding == fltNanEncoding::AllOnes &&
        exponent == semantics->maxExponent && isSignificandAllOnes())
      ଫେରନ୍ତୁ handleOverflow(rounding_mode);
  }

  /* The normal case - we were and are not denormal, and any
     significand increment above didn't overflow.  */
  ଯଦି (omsb == semantics->precision)
    ଫେରନ୍ତୁ opInexact;

  /* We have a non-zero denormal.  */
  ପ୍ରତିଜ୍ଞା(omsb < semantics->precision);

  /* Canonicalize zeroes.  */
  ଯଦି (omsb == 0) {
    category = fcZero;
    ଯଦି (semantics->nanEncoding == fltNanEncoding::NegativeZero)
      sign = ମିଥ୍ୟା;
    // This condition handles the case where the semantics
    // does not have zero but uses the all-zero encoding
    // to represent the smallest normal value.
    ଯଦି (!semantics->hasZero)
      makeSmallestNormalized(ମିଥ୍ୟା);
  }

  /* The fcZero case is a denormal that underflowed to zero.  */
  ଫେରନ୍ତୁ (opStatus) (opUnderflow | opInexact);
}

APFloat::opStatus IEEEFloat::addOrSubtractSpecials(ଅଚଳ IEEEFloat &rhs,
                                                   ତାର୍କିକ subtract) {
  ବାଛ (Pack_Categories_Into_ଚାବି(category, rhs.category)) {
  ପୂର୍ବନିର୍ଦ୍ଧାରିତ:
    llvm_unreachable(nullptr);

  ସ୍ଥିତି Pack_Categories_Into_ଚାବି(fcZero, fcNaN):
  ସ୍ଥିତି Pack_Categories_Into_ଚାବି(fcNormal, fcNaN):
  ସ୍ଥିତି Pack_Categories_Into_ଚାବି(fcInfinity, fcNaN):
    assign(rhs);
    [[fallthrough]];
  ସ୍ଥିତି Pack_Categories_Into_ଚାବି(fcNaN, fcZero):
  ସ୍ଥିତି Pack_Categories_Into_ଚାବି(fcNaN, fcNormal):
  ସ୍ଥିତି Pack_Categories_Into_ଚାବି(fcNaN, fcInfinity):
  ସ୍ଥିତି Pack_Categories_Into_ଚାବି(fcNaN, fcNaN):
    ଯଦି (isSignaling()) {
      makeQuiet();
      ଫେରନ୍ତୁ opInvalidOp;
    }
    ଫେରନ୍ତୁ rhs.isSignaling() ? opInvalidOp : opOK;

  ସ୍ଥିତି Pack_Categories_Into_ଚାବି(fcNormal, fcZero):
  ସ୍ଥିତି Pack_Categories_Into_ଚାବି(fcInfinity, fcNormal):
  ସ୍ଥିତି Pack_Categories_Into_ଚାବି(fcInfinity, fcZero):
    ଫେରନ୍ତୁ opOK;

  ସ୍ଥିତି Pack_Categories_Into_ଚାବି(fcNormal, fcInfinity):
  ସ୍ଥିତି Pack_Categories_Into_ଚାବି(fcZero, fcInfinity):
    category = fcInfinity;
    sign = rhs.sign ^ subtract;
    ଫେରନ୍ତୁ opOK;

  ସ୍ଥିତି Pack_Categories_Into_ଚାବି(fcZero, fcNormal):
    assign(rhs);
    sign = rhs.sign ^ subtract;
    ଫେରନ୍ତୁ opOK;

  ସ୍ଥିତି Pack_Categories_Into_ଚାବି(fcZero, fcZero):
    /* Sign depends on rounding mode; handled by caller.  */
    ଫେରନ୍ତୁ opOK;

  ସ୍ଥିତି Pack_Categories_Into_ଚାବି(fcInfinity, fcInfinity):
    /* Differently signed infinities can only be validly
       subtracted.  */
    ଯଦି (((sign ^ rhs.sign)!=0) != subtract) {
      makeNaN();
      ଫେରନ୍ତୁ opInvalidOp;
    }

    ଫେରନ୍ତୁ opOK;

  ସ୍ଥିତି Pack_Categories_Into_ଚାବି(fcNormal, fcNormal):
    ଫେରନ୍ତୁ opDivByZero;
  }
}

/* Add or subtract two normal numbers.  */
lostFraction IEEEFloat::addOrSubtractSignificand(ଅଚଳ IEEEFloat &rhs,
                                                 ତାର୍କିକ subtract) {
  [[maybe_unused]] integerPart carry = 0;
  lostFraction lost_fraction;

  /* Determine if the operation on the absolute values is effectively
     an addition or subtraction.  */
  subtract ^= static_cast<ତାର୍କିକ>(sign ^ rhs.sign);

  /* Are we bigger exponent-wise than the RHS?  */
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା bits = exponent - rhs.exponent;

  /* Subtraction is more subtle than one might naively expect.  */
  ଯଦି (subtract) {
    ଯଦି ((bits < 0) && !semantics->hasSignedRepr)
      llvm_unreachable(
          "This floating point format does not support signed values");

    IEEEFloat temp_rhs(rhs);
    ତାର୍କିକ lost_fraction_is_from_rhs = ମିଥ୍ୟା;

    ଯଦି (bits == 0)
      lost_fraction = lfExactlyZero;
    ଅନ୍ୟଥା ଯଦି (bits > 0) {
      lost_fraction = temp_rhs.shift_Significand_ଡାହାଣ(bits - 1);
      lost_fraction_is_from_rhs = ସତ୍ୟ;
      shift_Significand_ବାମ(1);
    } ଅନ୍ୟଥା {
      lost_fraction = shift_Significand_ଡାହାଣ(-bits - 1);
      temp_rhs.shift_Significand_ବାମ(1);
    }

    // Should we reverse the subtraction.
    cmpResult cmp_result = compare_Absolute_ମୂଲ୍ୟ(temp_rhs);
    ଯଦି (cmp_result == cmpLessThan) {
      ତାର୍କିକ borrow =
          lost_fraction != lfExactlyZero && !lost_fraction_is_from_rhs;
      ଯଦି (borrow) {
        // The lost fraction is being subtracted, borrow from the significand
        // and invert `lost_fraction`.
        ଯଦି (lost_fraction == lfLessThanHalf)
          lost_fraction = lfMoreThanHalf;
        ଅନ୍ୟଥା ଯଦି (lost_fraction == lfMoreThanHalf)
          lost_fraction = lfLessThanHalf;
      }
      carry = temp_rhs.subtractSignificand(*ଏହି, borrow);
      copySignificand(temp_rhs);
      sign = !sign;
    } ଅନ୍ୟଥା ଯଦି (cmp_result == cmpGreaterThan) {
      ତାର୍କିକ borrow = lost_fraction != lfExactlyZero && lost_fraction_is_from_rhs;
      ଯଦି (borrow) {
        // The lost fraction is being subtracted, borrow from the significand
        // and invert `lost_fraction`.
        ଯଦି (lost_fraction == lfLessThanHalf)
          lost_fraction = lfMoreThanHalf;
        ଅନ୍ୟଥା ଯଦି (lost_fraction == lfMoreThanHalf)
          lost_fraction = lfLessThanHalf;
      }
      carry = subtractSignificand(temp_rhs, borrow);
    } ଅନ୍ୟଥା { // cmpEqual
      zeroSignificand();
      ଯଦି (lost_fraction != lfExactlyZero && lost_fraction_is_from_rhs) {
        // rhs is slightly larger due to the lost fraction, flip the sign.
        sign = !sign;
      }
    }

    /* The code above is intended to ensure that no borrow is
       necessary.  */
    ପ୍ରତିଜ୍ଞା(!carry);
  } ଅନ୍ୟଥା {
    ଯଦି (bits > 0) {
      IEEEFloat temp_rhs(rhs);

      lost_fraction = temp_rhs.shift_Significand_ଡାହାଣ(bits);
      carry = addSignificand(temp_rhs);
    } ଅନ୍ୟଥା {
      lost_fraction = shift_Significand_ଡାହାଣ(-bits);
      carry = addSignificand(rhs);
    }

    /* We have a guard bit; generating a carry cannot happen.  */
    ପ୍ରତିଜ୍ଞା(!carry);
  }

  ଫେରନ୍ତୁ lost_fraction;
}

APFloat::opStatus IEEEFloat::multiplySpecials(ଅଚଳ IEEEFloat &rhs) {
  ବାଛ (Pack_Categories_Into_ଚାବି(category, rhs.category)) {
  ପୂର୍ବନିର୍ଦ୍ଧାରିତ:
    llvm_unreachable(nullptr);

  ସ୍ଥିତି Pack_Categories_Into_ଚାବି(fcZero, fcNaN):
  ସ୍ଥିତି Pack_Categories_Into_ଚାବି(fcNormal, fcNaN):
  ସ୍ଥିତି Pack_Categories_Into_ଚାବି(fcInfinity, fcNaN):
    assign(rhs);
    sign = ମିଥ୍ୟା;
    [[fallthrough]];
  ସ୍ଥିତି Pack_Categories_Into_ଚାବି(fcNaN, fcZero):
  ସ୍ଥିତି Pack_Categories_Into_ଚାବି(fcNaN, fcNormal):
  ସ୍ଥିତି Pack_Categories_Into_ଚାବି(fcNaN, fcInfinity):
  ସ୍ଥିତି Pack_Categories_Into_ଚାବି(fcNaN, fcNaN):
    sign ^= rhs.sign; // restore the original sign
    ଯଦି (isSignaling()) {
      makeQuiet();
      ଫେରନ୍ତୁ opInvalidOp;
    }
    ଫେରନ୍ତୁ rhs.isSignaling() ? opInvalidOp : opOK;

  ସ୍ଥିତି Pack_Categories_Into_ଚାବି(fcNormal, fcInfinity):
  ସ୍ଥିତି Pack_Categories_Into_ଚାବି(fcInfinity, fcNormal):
  ସ୍ଥିତି Pack_Categories_Into_ଚାବି(fcInfinity, fcInfinity):
    category = fcInfinity;
    ଫେରନ୍ତୁ opOK;

  ସ୍ଥିତି Pack_Categories_Into_ଚାବି(fcZero, fcNormal):
  ସ୍ଥିତି Pack_Categories_Into_ଚାବି(fcNormal, fcZero):
  ସ୍ଥିତି Pack_Categories_Into_ଚାବି(fcZero, fcZero):
    category = fcZero;
    ଫେରନ୍ତୁ opOK;

  ସ୍ଥିତି Pack_Categories_Into_ଚାବି(fcZero, fcInfinity):
  ସ୍ଥିତି Pack_Categories_Into_ଚାବି(fcInfinity, fcZero):
    makeNaN();
    ଫେରନ୍ତୁ opInvalidOp;

  ସ୍ଥିତି Pack_Categories_Into_ଚାବି(fcNormal, fcNormal):
    ଫେରନ୍ତୁ opOK;
  }
}

APFloat::opStatus IEEEFloat::divideSpecials(ଅଚଳ IEEEFloat &rhs) {
  ବାଛ (Pack_Categories_Into_ଚାବି(category, rhs.category)) {
  ପୂର୍ବନିର୍ଦ୍ଧାରିତ:
    llvm_unreachable(nullptr);

  ସ୍ଥିତି Pack_Categories_Into_ଚାବି(fcZero, fcNaN):
  ସ୍ଥିତି Pack_Categories_Into_ଚାବି(fcNormal, fcNaN):
  ସ୍ଥିତି Pack_Categories_Into_ଚାବି(fcInfinity, fcNaN):
    assign(rhs);
    sign = ମିଥ୍ୟା;
    [[fallthrough]];
  ସ୍ଥିତି Pack_Categories_Into_ଚାବି(fcNaN, fcZero):
  ସ୍ଥିତି Pack_Categories_Into_ଚାବି(fcNaN, fcNormal):
  ସ୍ଥିତି Pack_Categories_Into_ଚାବି(fcNaN, fcInfinity):
  ସ୍ଥିତି Pack_Categories_Into_ଚାବି(fcNaN, fcNaN):
    sign ^= rhs.sign; // restore the original sign
    ଯଦି (isSignaling()) {
      makeQuiet();
      ଫେରନ୍ତୁ opInvalidOp;
    }
    ଫେରନ୍ତୁ rhs.isSignaling() ? opInvalidOp : opOK;

  ସ୍ଥିତି Pack_Categories_Into_ଚାବି(fcInfinity, fcZero):
  ସ୍ଥିତି Pack_Categories_Into_ଚାବି(fcInfinity, fcNormal):
  ସ୍ଥିତି Pack_Categories_Into_ଚାବି(fcZero, fcInfinity):
  ସ୍ଥିତି Pack_Categories_Into_ଚାବି(fcZero, fcNormal):
    ଫେରନ୍ତୁ opOK;

  ସ୍ଥିତି Pack_Categories_Into_ଚାବି(fcNormal, fcInfinity):
    category = fcZero;
    ଫେରନ୍ତୁ opOK;

  ସ୍ଥିତି Pack_Categories_Into_ଚାବି(fcNormal, fcZero):
    ଯଦି (semantics->nonFiniteBehavior == fltNonfiniteBehavior::NanOnly)
      makeNaN(ମିଥ୍ୟା, sign);
    ଅନ୍ୟଥା
      category = fcInfinity;
    ଫେରନ୍ତୁ opDivByZero;

  ସ୍ଥିତି Pack_Categories_Into_ଚାବି(fcInfinity, fcInfinity):
  ସ୍ଥିତି Pack_Categories_Into_ଚାବି(fcZero, fcZero):
    makeNaN();
    ଫେରନ୍ତୁ opInvalidOp;

  ସ୍ଥିତି Pack_Categories_Into_ଚାବି(fcNormal, fcNormal):
    ଫେରନ୍ତୁ opOK;
  }
}

APFloat::opStatus IEEEFloat::modSpecials(ଅଚଳ IEEEFloat &rhs) {
  ବାଛ (Pack_Categories_Into_ଚାବି(category, rhs.category)) {
  ପୂର୍ବନିର୍ଦ୍ଧାରିତ:
    llvm_unreachable(nullptr);

  ସ୍ଥିତି Pack_Categories_Into_ଚାବି(fcZero, fcNaN):
  ସ୍ଥିତି Pack_Categories_Into_ଚାବି(fcNormal, fcNaN):
  ସ୍ଥିତି Pack_Categories_Into_ଚାବି(fcInfinity, fcNaN):
    assign(rhs);
    [[fallthrough]];
  ସ୍ଥିତି Pack_Categories_Into_ଚାବି(fcNaN, fcZero):
  ସ୍ଥିତି Pack_Categories_Into_ଚାବି(fcNaN, fcNormal):
  ସ୍ଥିତି Pack_Categories_Into_ଚାବି(fcNaN, fcInfinity):
  ସ୍ଥିତି Pack_Categories_Into_ଚାବି(fcNaN, fcNaN):
    ଯଦି (isSignaling()) {
      makeQuiet();
      ଫେରନ୍ତୁ opInvalidOp;
    }
    ଫେରନ୍ତୁ rhs.isSignaling() ? opInvalidOp : opOK;

  ସ୍ଥିତି Pack_Categories_Into_ଚାବି(fcZero, fcInfinity):
  ସ୍ଥିତି Pack_Categories_Into_ଚାବି(fcZero, fcNormal):
  ସ୍ଥିତି Pack_Categories_Into_ଚାବି(fcNormal, fcInfinity):
    ଫେରନ୍ତୁ opOK;

  ସ୍ଥିତି Pack_Categories_Into_ଚାବି(fcNormal, fcZero):
  ସ୍ଥିତି Pack_Categories_Into_ଚାବି(fcInfinity, fcZero):
  ସ୍ଥିତି Pack_Categories_Into_ଚାବି(fcInfinity, fcNormal):
  ସ୍ଥିତି Pack_Categories_Into_ଚାବି(fcInfinity, fcInfinity):
  ସ୍ଥିତି Pack_Categories_Into_ଚାବି(fcZero, fcZero):
    makeNaN();
    ଫେରନ୍ତୁ opInvalidOp;

  ସ୍ଥିତି Pack_Categories_Into_ଚାବି(fcNormal, fcNormal):
    ଫେରନ୍ତୁ opOK;
  }
}

APFloat::opStatus IEEEFloat::remainderSpecials(ଅଚଳ IEEEFloat &rhs) {
  ବାଛ (Pack_Categories_Into_ଚାବି(category, rhs.category)) {
  ପୂର୍ବନିର୍ଦ୍ଧାରିତ:
    llvm_unreachable(nullptr);

  ସ୍ଥିତି Pack_Categories_Into_ଚାବି(fcZero, fcNaN):
  ସ୍ଥିତି Pack_Categories_Into_ଚାବି(fcNormal, fcNaN):
  ସ୍ଥିତି Pack_Categories_Into_ଚାବି(fcInfinity, fcNaN):
    assign(rhs);
    [[fallthrough]];
  ସ୍ଥିତି Pack_Categories_Into_ଚାବି(fcNaN, fcZero):
  ସ୍ଥିତି Pack_Categories_Into_ଚାବି(fcNaN, fcNormal):
  ସ୍ଥିତି Pack_Categories_Into_ଚାବି(fcNaN, fcInfinity):
  ସ୍ଥିତି Pack_Categories_Into_ଚାବି(fcNaN, fcNaN):
    ଯଦି (isSignaling()) {
      makeQuiet();
      ଫେରନ୍ତୁ opInvalidOp;
    }
    ଫେରନ୍ତୁ rhs.isSignaling() ? opInvalidOp : opOK;

  ସ୍ଥିତି Pack_Categories_Into_ଚାବି(fcZero, fcInfinity):
  ସ୍ଥିତି Pack_Categories_Into_ଚାବି(fcZero, fcNormal):
  ସ୍ଥିତି Pack_Categories_Into_ଚାବି(fcNormal, fcInfinity):
    ଫେରନ୍ତୁ opOK;

  ସ୍ଥିତି Pack_Categories_Into_ଚାବି(fcNormal, fcZero):
  ସ୍ଥିତି Pack_Categories_Into_ଚାବି(fcInfinity, fcZero):
  ସ୍ଥିତି Pack_Categories_Into_ଚାବି(fcInfinity, fcNormal):
  ସ୍ଥିତି Pack_Categories_Into_ଚାବି(fcInfinity, fcInfinity):
  ସ୍ଥିତି Pack_Categories_Into_ଚାବି(fcZero, fcZero):
    makeNaN();
    ଫେରନ୍ତୁ opInvalidOp;

  ସ୍ଥିତି Pack_Categories_Into_ଚାବି(fcNormal, fcNormal):
    ଫେରନ୍ତୁ opDivByZero; // fake status, indicating this is not a special case
  }
}

/* Change sign.  */
ଶୂନ୍ୟ IEEEFloat::changeSign() {
  // With NaN-as-negative-zero, neither NaN or negative zero can change
  // their signs.
  ଯଦି (semantics->nanEncoding == fltNanEncoding::NegativeZero &&
      (isZero() || isNaN()))
    ଫେରନ୍ତୁ;
  /* Look mummy, this one's easy.  */
  sign = !sign;
}

/* Normalized addition or subtraction.  */
APFloat::opStatus IEEEFloat::addOrSubtract(ଅଚଳ IEEEFloat &rhs,
                                           roundingMode rounding_mode,
                                           ତାର୍କିକ subtract) {
  opStatus fs = addOrSubtractSpecials(rhs, subtract);

  /* This return code means it was not a simple case.  */
  ଯଦି (fs == opDivByZero) {
    lostFraction lost_fraction;

    lost_fraction = addOrSubtractSignificand(rhs, subtract);
    fs = normalize(rounding_mode, lost_fraction);

    /* Can only be zero if we lost no fraction.  */
    ପ୍ରତିଜ୍ଞା(category != fcZero || lost_fraction == lfExactlyZero);
  }

  /* If two numbers add (exactly) to zero, IEEE 754 decrees it is a
     positive zero unless rounding to minus infinity, except that
     adding two like-signed zeroes gives that zero.  */
  ଯଦି (category == fcZero) {
    ଯଦି (rhs.category != fcZero || (sign == rhs.sign) == subtract)
      sign = (rounding_mode == rmTowardNegative);
    // NaN-in-negative-zero means zeros need to be normalized to +0.
    ଯଦି (semantics->nanEncoding == fltNanEncoding::NegativeZero)
      sign = ମିଥ୍ୟା;
  }

  ଫେରନ୍ତୁ fs;
}

/* Normalized addition.  */
APFloat::opStatus IEEEFloat::add(ଅଚଳ IEEEFloat &rhs,
                                 roundingMode rounding_mode) {
  ଫେରନ୍ତୁ addOrSubtract(rhs, rounding_mode, ମିଥ୍ୟା);
}

/* Normalized subtraction.  */
APFloat::opStatus IEEEFloat::subtract(ଅଚଳ IEEEFloat &rhs,
                                      roundingMode rounding_mode) {
  ଫେରନ୍ତୁ addOrSubtract(rhs, rounding_mode, ସତ୍ୟ);
}

/* Normalized multiply.  */
APFloat::opStatus IEEEFloat::multiply(ଅଚଳ IEEEFloat &rhs,
                                      roundingMode rounding_mode) {
  sign ^= rhs.sign;
  opStatus fs = multiplySpecials(rhs);

  ଯଦି (isZero() && semantics->nanEncoding == fltNanEncoding::NegativeZero)
    sign = ମିଥ୍ୟା;
  ଯଦି (isFiniteNonZero()) {
    lostFraction lost_fraction = multiplySignificand(rhs);
    fs = normalize(rounding_mode, lost_fraction);
    ଯଦି (lost_fraction != lfExactlyZero)
      fs = (opStatus) (fs | opInexact);
  }

  ଫେରନ୍ତୁ fs;
}

/* Normalized divide.  */
APFloat::opStatus IEEEFloat::divide(ଅଚଳ IEEEFloat &rhs,
                                    roundingMode rounding_mode) {
  sign ^= rhs.sign;
  opStatus fs = divideSpecials(rhs);

  ଯଦି (isZero() && semantics->nanEncoding == fltNanEncoding::NegativeZero)
    sign = ମିଥ୍ୟା;
  ଯଦି (isFiniteNonZero()) {
    lostFraction lost_fraction = divideSignificand(rhs);
    fs = normalize(rounding_mode, lost_fraction);
    ଯଦି (lost_fraction != lfExactlyZero)
      fs = (opStatus) (fs | opInexact);
  }

  ଫେରନ୍ତୁ fs;
}

/* Normalized remainder.  */
APFloat::opStatus IEEEFloat::remainder(ଅଚଳ IEEEFloat &rhs) {
  unsigned ପୂର୍ଣ୍ଣସଂଖ୍ୟା origSign = sign;

  // First handle the special cases.
  opStatus fs = remainderSpecials(rhs);
  ଯଦି (fs != opDivByZero)
    ଫେରନ୍ତୁ fs;

  fs = opOK;

  // Make sure the current value is less than twice the denom. If the addition
  // did not succeed (an overflow has happened), which means that the finite
  // value we currently posses must be less than twice the denom (as we are
  // using the same semantics).
  IEEEFloat P2 = rhs;
  ଯଦି (P2.add(rhs, rmNearestTiesToEven) == opOK) {
    fs = mod(P2);
    ପ୍ରତିଜ୍ଞା(fs == opOK);
  }

  // Lets work with absolute numbers.
  IEEEFloat P = rhs;
  P.sign = ମିଥ୍ୟା;
  sign = ମିଥ୍ୟା;

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
  ତାର୍କିକ losesInfo;
  fltSemantics extendedSemantics = *semantics;
  extendedSemantics.maxExponent++;
  extendedSemantics.minExponent--;
  extendedSemantics.precision += 2;

  IEEEFloat VEx = *ଏହି;
  fs = VEx.convert(extendedSemantics, rmNearestTiesToEven, &losesInfo);
  ପ୍ରତିଜ୍ଞା(fs == opOK && !losesInfo);
  IEEEFloat PEx = P;
  fs = PEx.convert(extendedSemantics, rmNearestTiesToEven, &losesInfo);
  ପ୍ରତିଜ୍ଞା(fs == opOK && !losesInfo);

  // It is simpler to work with 2x instead of 0.5p, and we do not need to lose
  // any fraction.
  fs = VEx.add(VEx, rmNearestTiesToEven);
  ପ୍ରତିଜ୍ଞା(fs == opOK);

  ଯଦି (VEx.compare(PEx) == cmpGreaterThan) {
    fs = subtract(P, rmNearestTiesToEven);
    ପ୍ରତିଜ୍ଞା(fs == opOK);

    // Make VEx = this.add(this), but because we have different semantics, we do
    // not want to `convert` again, so we just subtract PEx twice (which equals
    // to the desired value).
    fs = VEx.subtract(PEx, rmNearestTiesToEven);
    ପ୍ରତିଜ୍ଞା(fs == opOK);
    fs = VEx.subtract(PEx, rmNearestTiesToEven);
    ପ୍ରତିଜ୍ଞା(fs == opOK);

    cmpResult result = VEx.compare(PEx);
    ଯଦି (result == cmpGreaterThan || result == cmpEqual) {
      fs = subtract(P, rmNearestTiesToEven);
      ପ୍ରତିଜ୍ଞା(fs == opOK);
    }
  }

  ଯଦି (isZero()) {
    sign = origSign;    // IEEE754 requires this
    ଯଦି (semantics->nanEncoding == fltNanEncoding::NegativeZero)
      // But some 8-bit floats only have positive 0.
      sign = ମିଥ୍ୟା;
  } ଅନ୍ୟଥା {
    sign ^= origSign;
  }
  ଫେରନ୍ତୁ fs;
}

/* Normalized llvm frem (C fmod). */
APFloat::opStatus IEEEFloat::mod(ଅଚଳ IEEEFloat &rhs) {
  opStatus fs = modSpecials(rhs);
  unsigned ପୂର୍ଣ୍ଣସଂଖ୍ୟା origSign = sign;

  ଯେତେବେଳେ (isFiniteNonZero() && rhs.isFiniteNonZero() &&
         compare_Absolute_ମୂଲ୍ୟ(rhs) != cmpLessThan) {
    ପୂର୍ଣ୍ଣସଂଖ୍ୟା Exp = ilogb(*ଏହି) - ilogb(rhs);
    IEEEFloat V = scalbn(rhs, Exp, rmNearestTiesToEven);
    // V can overflow to NaN with fltNonfiniteBehavior::NanOnly, so explicitly
    // check for it.
    ଯଦି (V.isNaN() || compare_Absolute_ମୂଲ୍ୟ(V) == cmpLessThan)
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
    ଯଦି (!semantics->hasZero && ଏହି->isSmallest())
      ଭାଙ୍ଗ;

    ପ୍ରତିଜ୍ଞା(fs==opOK);
  }
  ଯଦି (isZero()) {
    sign = origSign; // fmod requires this
    ଯଦି (semantics->nanEncoding == fltNanEncoding::NegativeZero)
      sign = ମିଥ୍ୟା;
  }
  ଫେରନ୍ତୁ fs;
}

/* Normalized fused-multiply-add.  */
APFloat::opStatus IEEEFloat::fusedMultiplyAdd(ଅଚଳ IEEEFloat &multiplicand,
                                              ଅଚଳ IEEEFloat &addend,
                                              roundingMode rounding_mode) {
  opStatus fs;

  /* Post-multiplication sign, before addition.  */
  sign ^= multiplicand.sign;

  /* If and only if all arguments are normal do we need to do an
     extended-precision calculation.  */
  ଯଦି (isFiniteNonZero() &&
      multiplicand.isFiniteNonZero() &&
      addend.isFinite()) {
    lostFraction lost_fraction;

    lost_fraction = multiplySignificand(multiplicand, addend);
    fs = normalize(rounding_mode, lost_fraction);
    ଯଦି (lost_fraction != lfExactlyZero)
      fs = (opStatus) (fs | opInexact);

    /* If two numbers add (exactly) to zero, IEEE 754 decrees it is a
       positive zero unless rounding to minus infinity, except that
       adding two like-signed zeroes gives that zero.  */
    ଯଦି (category == fcZero && !(fs & opUnderflow) && sign != addend.sign) {
      sign = (rounding_mode == rmTowardNegative);
      ଯଦି (semantics->nanEncoding == fltNanEncoding::NegativeZero)
        sign = ମିଥ୍ୟା;
    }
  } ଅନ୍ୟଥା {
    fs = multiplySpecials(multiplicand);

    /* FS can only be opOK or opInvalidOp.  There is no more work
       to do in the latter case.  The IEEE-754R standard says it is
       implementation-defined in this case whether, if ADDEND is a
       quiet NaN, we raise invalid op; this implementation does so.

       If we need to do the addition we can do so with normal
       precision.  */
    ଯଦି (fs == opOK)
      fs = addOrSubtract(addend, rounding_mode, ମିଥ୍ୟା);
  }

  ଫେରନ୍ତୁ fs;
}

/* Rounding-mode correct round to integral value.  */
APFloat::opStatus IEEEFloat::roundToIntegral(roundingMode rounding_mode) {
  ଯଦି (isInfinity())
    // [IEEE Std 754-2008 6.1]:
    // The behavior of infinity in floating-point arithmetic is derived from the
    // limiting cases of real arithmetic with operands of arbitrarily
    // large magnitude, when such a limit exists.
    // ...
    // Operations on infinite operands are usually exact and therefore signal no
    // exceptions ...
    ଫେରନ୍ତୁ opOK;

  ଯଦି (isNaN()) {
    ଯଦି (isSignaling()) {
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
      ଫେରନ୍ତୁ opInvalidOp;
    } ଅନ୍ୟଥା {
      // [IEEE Std 754-2008 6.2]:
      // For an operation with quiet NaN inputs, other than maximum and minimum
      // operations, if a floating-point result is to be delivered the result
      // shall be a quiet NaN which should be one of the input NaNs.
      // ...
      // Every general-computational and quiet-computational operation involving
      // one or more input NaNs, none of them signaling, shall signal no
      // exception, except fusedMultiplyAdd might signal the invalid operation
      // exception(see 7.2).
      ଫେରନ୍ତୁ opOK;
    }
  }

  ଯଦି (isZero()) {
    // [IEEE Std 754-2008 6.3]:
    // ... the sign of the result of conversions, the quantize operation, the
    // roundToIntegral operations, and the roundToIntegralExact(see 5.3.1) is
    // the sign of the first or only operand.
    ଫେରନ୍ତୁ opOK;
  }

  // If the exponent is large enough, we know that this value is already
  // integral, and the arithmetic below would potentially cause it to saturate
  // to +/-Inf.  Bail out early instead.
  ଯଦି (exponent + 1 >= (ପୂର୍ଣ୍ଣସଂଖ୍ୟା)APFloat::semanticsPrecision(*semantics))
    ଫେରନ୍ତୁ opOK;

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
  opStatus fs = MagicConstant.convertFromAPInt(IntegerConstant, ମିଥ୍ୟା,
                                               rmNearestTiesToEven);
  ପ୍ରତିଜ୍ଞା(fs == opOK);
  MagicConstant.sign = sign;

  // Preserve the input sign so that we can handle the case of zero result
  // correctly.
  ତାର୍କିକ ନିବେଶ_Sign = isNegative();

  fs = add(MagicConstant, rounding_mode);

  // Current value and 'MagicConstant' are both integers, so the result of the
  // subtraction is always exact according to Sterbenz' lemma.
  subtract(MagicConstant, rounding_mode);

  // Restore the input sign.
  ଯଦି (ନିବେଶ_Sign != isNegative())
    changeSign();

  ଫେରନ୍ତୁ fs;
}

/* Comparison requires normalized numbers.  */
APFloat::cmpResult IEEEFloat::compare(ଅଚଳ IEEEFloat &rhs) ଅଚଳ {
  ପ୍ରତିଜ୍ଞା(semantics == rhs.semantics);

  ବାଛ (Pack_Categories_Into_ଚାବି(category, rhs.category)) {
  ପୂର୍ବନିର୍ଦ୍ଧାରିତ:
    llvm_unreachable(nullptr);

  ସ୍ଥିତି Pack_Categories_Into_ଚାବି(fcNaN, fcZero):
  ସ୍ଥିତି Pack_Categories_Into_ଚାବି(fcNaN, fcNormal):
  ସ୍ଥିତି Pack_Categories_Into_ଚାବି(fcNaN, fcInfinity):
  ସ୍ଥିତି Pack_Categories_Into_ଚାବି(fcNaN, fcNaN):
  ସ୍ଥିତି Pack_Categories_Into_ଚାବି(fcZero, fcNaN):
  ସ୍ଥିତି Pack_Categories_Into_ଚାବି(fcNormal, fcNaN):
  ସ୍ଥିତି Pack_Categories_Into_ଚାବି(fcInfinity, fcNaN):
    ଫେରନ୍ତୁ cmpUnordered;

  ସ୍ଥିତି Pack_Categories_Into_ଚାବି(fcInfinity, fcNormal):
  ସ୍ଥିତି Pack_Categories_Into_ଚାବି(fcInfinity, fcZero):
  ସ୍ଥିତି Pack_Categories_Into_ଚାବି(fcNormal, fcZero):
    ଯଦି (sign)
      ଫେରନ୍ତୁ cmpLessThan;
    ଅନ୍ୟଥା
      ଫେରନ୍ତୁ cmpGreaterThan;

  ସ୍ଥିତି Pack_Categories_Into_ଚାବି(fcNormal, fcInfinity):
  ସ୍ଥିତି Pack_Categories_Into_ଚାବି(fcZero, fcInfinity):
  ସ୍ଥିତି Pack_Categories_Into_ଚାବି(fcZero, fcNormal):
    ଯଦି (rhs.sign)
      ଫେରନ୍ତୁ cmpGreaterThan;
    ଅନ୍ୟଥା
      ଫେରନ୍ତୁ cmpLessThan;

  ସ୍ଥିତି Pack_Categories_Into_ଚାବି(fcInfinity, fcInfinity):
    ଯଦି (sign == rhs.sign)
      ଫେରନ୍ତୁ cmpEqual;
    ଅନ୍ୟଥା ଯଦି (sign)
      ଫେରନ୍ତୁ cmpLessThan;
    ଅନ୍ୟଥା
      ଫେରନ୍ତୁ cmpGreaterThan;

  ସ୍ଥିତି Pack_Categories_Into_ଚାବି(fcZero, fcZero):
    ଫେରନ୍ତୁ cmpEqual;

  ସ୍ଥିତି Pack_Categories_Into_ଚାବି(fcNormal, fcNormal):
    ଭାଙ୍ଗ;
  }

  cmpResult result;
  /* Two normal numbers.  Do they have the same sign?  */
  ଯଦି (sign != rhs.sign) {
    ଯଦି (sign)
      result = cmpLessThan;
    ଅନ୍ୟଥା
      result = cmpGreaterThan;
  } ଅନ୍ୟଥା {
    /* Compare absolute values; invert result if negative.  */
    result = compare_Absolute_ମୂଲ୍ୟ(rhs);

    ଯଦି (sign) {
      ଯଦି (result == cmpLessThan)
        result = cmpGreaterThan;
      ଅନ୍ୟଥା ଯଦି (result == cmpGreaterThan)
        result = cmpLessThan;
    }
  }

  ଫେରନ୍ତୁ result;
}

/// IEEEFloat::convert - convert a value of one floating point type to another.
/// The return value corresponds to the IEEE754 exceptions.  *losesInfo
/// records whether the transformation lost information, i.e. whether
/// converting the result back to the original type will produce the
/// original value (this is almost the same as return value==fsOK, but there
/// are edge cases where this is not so).

APFloat::opStatus IEEEFloat::convert(ଅଚଳ fltSemantics &toSemantics,
                                     roundingMode rounding_mode,
                                     ତାର୍କିକ *losesInfo) {
  opStatus fs;
  ଅଚଳ fltSemantics &fromSemantics = *semantics;
  ତାର୍କିକ is_signaling = isSignaling();

  lostFraction lostFraction = lfExactlyZero;
  unsigned new_Part_ଗଣନା = part_ଗଣନା_For_Bits(toSemantics.precision + 1);
  unsigned old_Part_ଗଣନା = part_ଗଣନା();
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା shift = toSemantics.precision - fromSemantics.precision;

  ତାର୍କିକ X86SpecialNan = ମିଥ୍ୟା;
  ଯଦି (&fromSemantics == &APFloatBase::semX87DoubleExtended &&
      &toSemantics != &APFloatBase::semX87DoubleExtended && category == fcNaN &&
      (!(*significandParts() & 0x8000000000000000ULL) ||
       !(*significandParts() & 0x4000000000000000ULL))) {
    // x86 has some unusual NaNs which cannot be represented in any other
    // format; note them here.
    X86SpecialNan = ସତ୍ୟ;
  }

  // If this is a truncation of a denormal number, and the target semantics
  // has larger exponent range than the source semantics (this can happen
  // when truncating from PowerPC double-double to double format), the
  // right shift could lose result mantissa bits.  Adjust exponent instead
  // of performing excessive shift.
  // Also do a similar trick in case shifting denormal would produce zero
  // significand as this case isn't handled correctly by normalize.
  ଯଦି (shift < 0 && isFiniteNonZero()) {
    ପୂର୍ଣ୍ଣସଂଖ୍ୟା omsb = significandMSB() + 1;
    ପୂର୍ଣ୍ଣସଂଖ୍ୟା exponentChange = omsb - fromSemantics.precision;
    ଯଦି (exponent + exponentChange < toSemantics.minExponent)
      exponentChange = toSemantics.minExponent - exponent;
    exponentChange = std::max(exponentChange, shift);
    ଯଦି (exponentChange < 0) {
      shift -= exponentChange;
      exponent += exponentChange;
    } ଅନ୍ୟଥା ଯଦି (omsb <= -shift) {
      exponentChange = omsb + shift - 1; // leave at least one bit set
      shift -= exponentChange;
      exponent += exponentChange;
    }
  }

  // If this is a truncation, perform the shift before we narrow the storage.
  ଯଦି (shift < 0 && (isFiniteNonZero() ||
                    (category == fcNaN && semantics->nonFiniteBehavior !=
                                              fltNonfiniteBehavior::NanOnly)))
    lostFraction = shift_ଡାହାଣ(significandParts(), old_Part_ଗଣନା, -shift);

  // Fix the storage so it can hold to new value.
  ଯଦି (new_Part_ଗଣନା > old_Part_ଗଣନା) {
    // The new type requires more storage; make it available.
    integerPart *newParts;
    newParts = ନୂତନ integerPart[new_Part_ଗଣନା];
    APInt::tcSet(newParts, 0, new_Part_ଗଣନା);
    ଯଦି (isFiniteNonZero() || category==fcNaN)
      APInt::tcAssign(newParts, significandParts(), old_Part_ଗଣନା);
    freeSignificand();
    significand.parts = newParts;
  } ଅନ୍ୟଥା ଯଦି (new_Part_ଗଣନା == 1 && old_Part_ଗଣନା != 1) {
    // Switch to built-in storage for a single part.
    integerPart newPart = 0;
    ଯଦି (isFiniteNonZero() || category==fcNaN)
      newPart = significandParts()[0];
    freeSignificand();
    significand.part = newPart;
  }

  // Now that we have the right storage, switch the semantics.
  semantics = &toSemantics;

  // If this is an extension, perform the shift now that the storage is
  // available.
  ଯଦି (shift > 0 && (isFiniteNonZero() || category==fcNaN))
    APInt::tc_Shift_ବାମ(significandParts(), new_Part_ଗଣନା, shift);

  ଯଦି (isFiniteNonZero()) {
    fs = normalize(rounding_mode, lostFraction);
    *losesInfo = (fs != opOK);
  } ଅନ୍ୟଥା ଯଦି (category == fcNaN) {
    ଯଦି (semantics->nonFiniteBehavior == fltNonfiniteBehavior::NanOnly) {
      *losesInfo =
          fromSemantics.nonFiniteBehavior != fltNonfiniteBehavior::NanOnly;
      makeNaN(ମିଥ୍ୟା, sign);
      fs = is_signaling ? opInvalidOp : opOK;
    } ଅନ୍ୟଥା {
      // If NaN is negative zero, we need to create a new NaN to avoid
      // converting NaN to -Inf.
      ଯଦି (fromSemantics.nanEncoding == fltNanEncoding::NegativeZero &&
          semantics->nanEncoding != fltNanEncoding::NegativeZero)
        makeNaN(ମିଥ୍ୟା, ମିଥ୍ୟା);

      // If the source has no significand, there are no payload bits to carry
      // over, and an all-zero significand would encode an Inf. Create a new
      // NaN.
      ଯଦି (!APFloat::hasSignificand(fromSemantics))
        makeNaN(ମିଥ୍ୟା, sign);

      *losesInfo = lostFraction != lfExactlyZero || X86SpecialNan;

      // For x87 extended precision, we want to make a NaN, not a special NaN
      // if the input wasn't special either.
      ଯଦି (!X86SpecialNan && semantics == &APFloatBase::semX87DoubleExtended)
        APInt::tcSetBit(significandParts(), semantics->precision - 1);

      // Convert of sNaN creates qNaN and raises an exception (invalid op).
      // This also guarantees that a sNaN does not become Inf on a truncation
      // that loses all payload bits.
      ଯଦି (is_signaling) {
        makeQuiet();
        fs = opInvalidOp;
      } ଅନ୍ୟଥା {
        fs = opOK;
      }
    }
  } ଅନ୍ୟଥା ଯଦି (category == fcInfinity &&
             semantics->nonFiniteBehavior == fltNonfiniteBehavior::NanOnly) {
    makeNaN(ମିଥ୍ୟା, sign);
    *losesInfo = ସତ୍ୟ;
    fs = opInexact;
  } ଅନ୍ୟଥା ଯଦି (category == fcZero &&
             semantics->nanEncoding == fltNanEncoding::NegativeZero) {
    // Negative zero loses info, but positive zero doesn't.
    *losesInfo =
        fromSemantics.nanEncoding != fltNanEncoding::NegativeZero && sign;
    fs = *losesInfo ? opInexact : opOK;
    // NaN is negative zero means -0 -> +0, which can lose information
    sign = ମିଥ୍ୟା;
  } ଅନ୍ୟଥା {
    *losesInfo = ମିଥ୍ୟା;
    fs = opOK;
  }

  // The target may have no encoding for a negative value, or none for zero.
  // The paths above only report what rounding lost, so report these here too:
  // a caller that checks losesInfo would otherwise accept a result the target
  // cannot represent, and printing that result asserts.
  ଯଦି ((sign && !semantics->hasSignedRepr) ||
      (category == fcZero && !semantics->hasZero)) {
    *losesInfo = ସତ୍ୟ;
    ଯଦି (fs == opOK)
      fs = opInexact;
  }

  ଯଦି (category == fcZero && !semantics->hasZero)
    makeSmallestNormalized(ମିଥ୍ୟା);
  ଫେରନ୍ତୁ fs;
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
    MutableArrayRef<integerPart> parts, unsigned ପୂର୍ଣ୍ଣସଂଖ୍ୟା width, ତାର୍କିକ isSigned,
    roundingMode rounding_mode, ତାର୍କିକ *isExact) ଅଚଳ {
  *isExact = ମିଥ୍ୟା;

  /* Handle the three special cases first.  */
  ଯଦି (category == fcInfinity || category == fcNaN)
    ଫେରନ୍ତୁ opInvalidOp;

  unsigned dst_Parts_ଗଣନା = part_ଗଣନା_For_Bits(width);
  ପ୍ରତିଜ୍ଞା(dst_Parts_ଗଣନା <= parts.size() && "Integer too big");

  ଯଦି (category == fcZero) {
    APInt::tcSet(parts.data(), 0, dst_Parts_ଗଣନା);
    // Negative zero can't be represented as an int.
    *isExact = !sign;
    ଫେରନ୍ତୁ opOK;
  }

  ଅଚଳ integerPart *src = significandParts();

  unsigned truncatedBits;
  /* Step 1: place our absolute value, with any fraction truncated, in
     the destination.  */
  ଯଦି (exponent < 0) {
    /* Our absolute value is less than one; truncate everything.  */
    APInt::tcSet(parts.data(), 0, dst_Parts_ଗଣନା);
    /* For exponent -1 the integer bit represents .5, look at that.
       For smaller exponents leftmost truncated bit is 0. */
    truncatedBits = semantics->precision -1U - exponent;
  } ଅନ୍ୟଥା {
    /* We want the most significant (exponent + 1) bits; the rest are
       truncated.  */
    unsigned ପୂର୍ଣ୍ଣସଂଖ୍ୟା bits = exponent + 1U;

    /* Hopelessly large in magnitude?  */
    ଯଦି (bits > width)
      ଫେରନ୍ତୁ opInvalidOp;

    ଯଦି (bits < semantics->precision) {
      /* We truncate (semantics->precision - bits) bits.  */
      truncatedBits = semantics->precision - bits;
      APInt::tcExtract(parts.data(), dst_Parts_ଗଣନା, src, bits, truncatedBits);
    } ଅନ୍ୟଥା {
      /* We want at least as many bits as are available.  */
      APInt::tcExtract(parts.data(), dst_Parts_ଗଣନା, src, semantics->precision,
                       0);
      APInt::tc_Shift_ବାମ(parts.data(), dst_Parts_ଗଣନା,
                         bits - semantics->precision);
      truncatedBits = 0;
    }
  }

  /* Step 2: work out any lost fraction, and increment the absolute
     value if we would round away from zero.  */
  lostFraction lost_fraction;
  ଯଦି (truncatedBits) {
    lost_fraction = lostFractionThroughTruncation(src, part_ଗଣନା(),
                                                  truncatedBits);
    ଯଦି (lost_fraction != lfExactlyZero &&
        roundAwayFromZero(rounding_mode, lost_fraction, truncatedBits)) {
      ଯଦି (APInt::tcIncrement(parts.data(), dst_Parts_ଗଣନା))
        ଫେରନ୍ତୁ opInvalidOp;     /* Overflow.  */
    }
  } ଅନ୍ୟଥା {
    lost_fraction = lfExactlyZero;
  }

  /* Step 3: check if we fit in the destination.  */
  unsigned ପୂର୍ଣ୍ଣସଂଖ୍ୟା omsb = APInt::tcMSB(parts.data(), dst_Parts_ଗଣନା) + 1;

  ଯଦି (sign) {
    ଯଦି (!isSigned) {
      /* Negative numbers cannot be represented as unsigned.  */
      ଯଦି (omsb != 0)
        ଫେରନ୍ତୁ opInvalidOp;
    } ଅନ୍ୟଥା {
      /* It takes omsb bits to represent the unsigned integer value.
         We lose a bit for the sign, but care is needed as the
         maximally negative integer is a special case.  */
      ଯଦି (omsb == width &&
          APInt::tcLSB(parts.data(), dst_Parts_ଗଣନା) + 1 != omsb)
        ଫେରନ୍ତୁ opInvalidOp;

      /* This case can happen because of rounding.  */
      ଯଦି (omsb > width)
        ଫେରନ୍ତୁ opInvalidOp;
    }

    APInt::tcNegate (parts.data(), dst_Parts_ଗଣନା);
  } ଅନ୍ୟଥା {
    ଯଦି (omsb >= width + !isSigned)
      ଫେରନ୍ତୁ opInvalidOp;
  }

  ଯଦି (lost_fraction == lfExactlyZero) {
    *isExact = ସତ୍ୟ;
    ଫେରନ୍ତୁ opOK;
  }
  ଫେରନ୍ତୁ opInexact;
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
                            unsigned ପୂର୍ଣ୍ଣସଂଖ୍ୟା width, ତାର୍କିକ isSigned,
                            roundingMode rounding_mode, ତାର୍କିକ *isExact) ଅଚଳ {
  opStatus fs = convertToSignExtendedInteger(parts, width, isSigned,
                                             rounding_mode, isExact);

  ଯଦି (fs == opInvalidOp) {
    unsigned ପୂର୍ଣ୍ଣସଂଖ୍ୟା bits, dst_Parts_ଗଣନା;

    dst_Parts_ଗଣନା = part_ଗଣନା_For_Bits(width);
    ପ୍ରତିଜ୍ଞା(dst_Parts_ଗଣନା <= parts.size() && "Integer too big");

    ଯଦି (category == fcNaN)
      bits = 0;
    ଅନ୍ୟଥା ଯଦି (sign)
      bits = isSigned;
    ଅନ୍ୟଥା
      bits = width - isSigned;

    tcSetLeastSignificantBits(parts.data(), dst_Parts_ଗଣନା, bits);
    ଯଦି (sign && isSigned)
      APInt::tc_Shift_ବାମ(parts.data(), dst_Parts_ଗଣନା, width - 1);
  }

  ଫେରନ୍ତୁ fs;
}

/* Convert an unsigned integer SRC to a floating point number,
   rounding according to ROUNDING_MODE.  The sign of the floating
   point number is not modified.  */
APFloat::opStatus IEEEFloat::convertFromUnsignedParts(
    ଅଚଳ integerPart *src, unsigned ପୂର୍ଣ୍ଣସଂଖ୍ୟା src_ଗଣନା, roundingMode rounding_mode) {
  category = fcNormal;
  unsigned omsb = APInt::tcMSB(src, src_ଗଣନା) + 1;
  integerPart *dst = significandParts();
  unsigned dst_ଗଣନା = part_ଗଣନା();
  unsigned precision = semantics->precision;

  /* We want the most significant PRECISION bits of SRC.  There may not
     be that many; extract what we can.  */
  lostFraction lost_fraction;
  ଯଦି (precision <= omsb) {
    exponent = omsb - 1;
    lost_fraction = lostFractionThroughTruncation(src, src_ଗଣନା,
                                                  omsb - precision);
    APInt::tcExtract(dst, dst_ଗଣନା, src, precision, omsb - precision);
  } ଅନ୍ୟଥା {
    exponent = precision - 1;
    lost_fraction = lfExactlyZero;
    APInt::tcExtract(dst, dst_ଗଣନା, src, omsb, 0);
  }

  ଫେରନ୍ତୁ normalize(rounding_mode, lost_fraction);
}

APFloat::opStatus IEEEFloat::convertFromAPInt(ଅଚଳ APInt &Val, ତାର୍କିକ isSigned,
                                              roundingMode rounding_mode) {
  unsigned ପୂର୍ଣ୍ଣସଂଖ୍ୟା part_ଗଣନା = Val.get_ଅଙ୍କ_ଶବ୍ଦଗୁଡ଼ିକ();
  APInt api = Val;

  sign = ମିଥ୍ୟା;
  ଯଦି (isSigned && api.isNegative()) {
    sign = ସତ୍ୟ;
    api = -api;
  }

  ଫେରନ୍ତୁ convertFromUnsignedParts(api.getRawData(), part_ଗଣନା, rounding_mode);
}

Expected<APFloat::opStatus>
IEEEFloat::convertFromHexadecimalString(StringRef s,
                                        roundingMode rounding_mode) {
  lostFraction lost_fraction = lfExactlyZero;

  category = fcNormal;
  zeroSignificand();
  exponent = 0;

  integerPart *significand = significandParts();
  unsigned parts_ଗଣନା = part_ଗଣନା();
  unsigned bitPos = parts_ଗଣନା * integerPartWidth;
  ତାର୍କିକ computedTrailingFraction = ମିଥ୍ୟା;

  // Skip leading zeroes and any (hexa)decimal point.
  StringRef::iterator begin = s.begin();
  StringRef::iterator end = s.end();
  StringRef::iterator dot;
  auto PtrOrErr = skipLeadingZeroesAndAnyDot(begin, end, &dot);
  ଯଦି (!PtrOrErr)
    ଫେରନ୍ତୁ PtrOrErr.takeError();
  StringRef::iterator p = *PtrOrErr;
  StringRef::iterator firstSignificantDigit = p;

  ଯେତେବେଳେ (p != end) {
    integerPart hex_ମୂଲ୍ୟ;

    ଯଦି (*p == '.') {
      ଯଦି (dot != end)
        ଫେରନ୍ତୁ createError("String contains multiple dots");
      dot = p++;
      ଜାରି;
    }

    hex_ମୂଲ୍ୟ = hex_Digit_ମୂଲ୍ୟ(*p);
    ଯଦି (hex_ମୂଲ୍ୟ == UINT_MAX)
      ଭାଙ୍ଗ;

    p++;

    // Store the number while we have space.
    ଯଦି (bitPos) {
      bitPos -= 4;
      hex_ମୂଲ୍ୟ <<= bitPos % integerPartWidth;
      significand[bitPos / integerPartWidth] |= hex_ମୂଲ୍ୟ;
    } ଅନ୍ୟଥା ଯଦି (!computedTrailingFraction) {
      auto FractOrErr = trailingHexadecimalFraction(p, end, hex_ମୂଲ୍ୟ);
      ଯଦି (!FractOrErr)
        ଫେରନ୍ତୁ FractOrErr.takeError();
      lost_fraction = *FractOrErr;
      computedTrailingFraction = ସତ୍ୟ;
    }
  }

  /* Hex floats require an exponent but not a hexadecimal point.  */
  ଯଦି (p == end)
    ଫେରନ୍ତୁ createError("Hex strings require an exponent");
  ଯଦି (*p != 'p' && *p != 'P')
    ଫେରନ୍ତୁ createError("Invalid character in significand");
  ଯଦି (p == begin)
    ଫେରନ୍ତୁ createError("Significand has no digits");
  ଯଦି (dot != end && p - begin == 1)
    ଫେରନ୍ତୁ createError("Significand has no digits");

  /* Ignore the exponent if we are zero.  */
  ଯଦି (p != firstSignificantDigit) {
    ପୂର୍ଣ୍ଣସଂଖ୍ୟା expAdjustment;

    /* Implicit hexadecimal point?  */
    ଯଦି (dot == end)
      dot = p;

    /* Calculate the exponent adjustment implicit in the number of
       significant digits.  */
    expAdjustment = static_cast<ପୂର୍ଣ୍ଣସଂଖ୍ୟା>(dot - firstSignificantDigit);
    ଯଦି (expAdjustment < 0)
      expAdjustment++;
    expAdjustment = expAdjustment * 4 - 1;

    /* Adjust for writing the significand starting at the most
       significant nibble.  */
    expAdjustment += semantics->precision;
    expAdjustment -= parts_ଗଣନା * integerPartWidth;

    /* Adjust for the given exponent.  */
    auto ExpOrErr = totalExponent(p + 1, end, expAdjustment);
    ଯଦି (!ExpOrErr)
      ଫେରନ୍ତୁ ExpOrErr.takeError();
    exponent = *ExpOrErr;
  }

  ଫେରନ୍ତୁ normalize(rounding_mode, lost_fraction);
}

APFloat::opStatus
IEEEFloat::roundSignificandWithExponent(ଅଚଳ integerPart *decSigParts,
                                        unsigned sig_Part_ଗଣନା, ପୂର୍ଣ୍ଣସଂଖ୍ୟା exp,
                                        roundingMode rounding_mode) {
  fltSemantics calcSemantics = { 32767, -32767, 0, 0 };
  unsigned ପୂର୍ଣ୍ଣସଂଖ୍ୟା power = exp >= 0 ? exp : -exp;
  unsigned ପୂର୍ଣ୍ଣସଂଖ୍ୟା pow5PartCapacity = powerOfFivePartsForExponent(power);
  SmallVector<integerPart, ଅଙ୍କ_Pow5_Parts_On_ଗଦା> pow5Parts;
  pow5Parts.resize_for_overwrite(pow5PartCapacity);

  ତାର୍କିକ isNearest = rounding_mode == rmNearestTiesToEven ||
                   rounding_mode == rmNearestTiesToAway;

  unsigned parts = part_ଗଣନା_For_Bits(semantics->precision + 11);

  /* Calculate pow(5, abs(exp)).  */
  unsigned pow5_Part_ଗଣନା = powerOf5(pow5Parts.data(), power);

  ପାଇଁ (;; parts *= 2) {
    unsigned ପୂର୍ଣ୍ଣସଂଖ୍ୟା excessPrecision, truncatedBits;

    calcSemantics.precision = parts * integerPartWidth - 1;
    excessPrecision = calcSemantics.precision - semantics->precision;
    truncatedBits = excessPrecision;

    IEEEFloat decSig(calcSemantics, uninitialized);
    decSig.makeZero(sign);
    IEEEFloat pow5(calcSemantics);

    opStatus sigStatus = decSig.convertFromUnsignedParts(
        decSigParts, sig_Part_ଗଣନା, rmNearestTiesToEven);
    opStatus powStatus = pow5.convertFromUnsignedParts(
        pow5Parts.data(), pow5_Part_ଗଣନା, rmNearestTiesToEven);
    /* Add exp, as 10^n = 5^n * 2^n.  */
    decSig.exponent += exp;

    lostFraction calcLostFraction;
    integerPart HUerr, HUdistance;
    unsigned ପୂର୍ଣ୍ଣସଂଖ୍ୟା powHUerr;

    ଯଦି (exp >= 0) {
      /* multiplySignificand leaves the precision-th bit set to 1.  */
      calcLostFraction = decSig.multiplySignificand(pow5);
      powHUerr = powStatus != opOK;
    } ଅନ୍ୟଥା {
      calcLostFraction = decSig.divideSignificand(pow5);
      /* Denormal numbers have less precision.  */
      ଯଦି (decSig.exponent < semantics->minExponent) {
        excessPrecision += (semantics->minExponent - decSig.exponent);
        truncatedBits = excessPrecision;
        excessPrecision = std::min(excessPrecision, calcSemantics.precision);
      }
      /* Extra half-ulp lost in reciprocal of exponent.  */
      powHUerr = (powStatus == opOK && calcLostFraction == lfExactlyZero) ? 0:2;
    }

    /* Both multiplySignificand and divideSignificand return the
       result with the integer bit set.  */
    ପ୍ରତିଜ୍ଞା(APInt::tcExtractBit
           (decSig.significandParts(), calcSemantics.precision - 1) == 1);

    HUerr = HUerrBound(calcLostFraction != lfExactlyZero, sigStatus != opOK,
                       powHUerr);
    HUdistance = 2 * ulpsFromBoundary(decSig.significandParts(),
                                      excessPrecision, isNearest);

    /* Are we guaranteed to round correctly if we truncate?  */
    ଯଦି (HUdistance >= HUerr) {
      APInt::tcExtract(significandParts(), part_ଗଣନା(), decSig.significandParts(),
                       calcSemantics.precision - excessPrecision,
                       excessPrecision);
      /* Take the exponent of decSig.  If we tcExtract-ed less bits
         above we must adjust our exponent to compensate for the
         implicit right shift.  */
      exponent = (decSig.exponent + semantics->precision
                  - (calcSemantics.precision - excessPrecision));
      calcLostFraction = lostFractionThroughTruncation(decSig.significandParts(),
                                                       decSig.part_ଗଣନା(),
                                                       truncatedBits);
      ଫେରନ୍ତୁ static_cast<opStatus>(normalize(rounding_mode, calcLostFraction) |
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
  ଯଦି (Error Err = interpretDecimal(p, str.end(), &D))
    ଫେରନ୍ତୁ std::move(Err);

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
  ଯଦି (D.firstSigDigit == str.end() || dec_Digit_ମୂଲ୍ୟ(*D.firstSigDigit) >= 10U) {
    category = fcZero;
    fs = opOK;
    ଯଦି (semantics->nanEncoding == fltNanEncoding::NegativeZero)
      sign = ମିଥ୍ୟା;
    ଯଦି (!semantics->hasZero)
      makeSmallestNormalized(ମିଥ୍ୟା);

    /* Check whether the normalized exponent is high enough to overflow
       max during the log-rebasing in the max-exponent check below. */
  } ଅନ୍ୟଥା ଯଦି (D.normalizedExponent - 1 > INT_MAX / 42039) {
    fs = handleOverflow(rounding_mode);

  /* If it wasn't, then it also wasn't high enough to overflow max
     during the log-rebasing in the min-exponent check.  Check that it
     won't overflow min in either check, then perform the min-exponent
     check. */
  } ଅନ୍ୟଥା ଯଦି (D.normalizedExponent - 1 < INT_MIN / 42039 ||
             (D.normalizedExponent + 1) * 28738 <=
               8651 * (semantics->minExponent - (ପୂର୍ଣ୍ଣସଂଖ୍ୟା) semantics->precision)) {
    /* Underflow to zero and round.  */
    category = fcNormal;
    zeroSignificand();
    fs = normalize(rounding_mode, lfLessThanHalf);

  /* We can finally safely perform the max-exponent check. */
  } ଅନ୍ୟଥା ଯଦି ((D.normalizedExponent - 1) * 42039
             >= 12655 * semantics->maxExponent) {
    /* Overflow and round.  */
    fs = handleOverflow(rounding_mode);
  } ଅନ୍ୟଥା {
    integerPart *decSignificand;
    unsigned ପୂର୍ଣ୍ଣସଂଖ୍ୟା part_ଗଣନା;

    /* A tight upper bound on number of bits required to hold an
       N-digit decimal integer is N * 196 / 59.  Allocate enough space
       to hold the full significand, and an extra part required by
       tcMultiplyPart.  */
    part_ଗଣନା = static_cast<unsigned ପୂର୍ଣ୍ଣସଂଖ୍ୟା>(D.lastSigDigit - D.firstSigDigit) + 1;
    part_ଗଣନା = part_ଗଣନା_For_Bits(1 + 196 * part_ଗଣନା / 59);
    decSignificand = ନୂତନ integerPart[part_ଗଣନା + 1];
    part_ଗଣନା = 0;

    /* Convert to binary efficiently - we do almost all multiplication
       in an integerPart.  When this would overflow do we do a single
       bignum multiplication, and then revert again to multiplication
       in an integerPart.  */
    କର {
      integerPart dec_ମୂଲ୍ୟ, val, multiplier;

      val = 0;
      multiplier = 1;

      କର {
        ଯଦି (*p == '.') {
          p++;
          ଯଦି (p == str.end()) {
            ଭାଙ୍ଗ;
          }
        }
        dec_ମୂଲ୍ୟ = dec_Digit_ମୂଲ୍ୟ(*p++);
        ଯଦି (dec_ମୂଲ୍ୟ >= 10U) {
          delete[] decSignificand;
          ଫେରନ୍ତୁ createError("Invalid character in significand");
        }
        multiplier *= 10;
        val = val * 10 + dec_ମୂଲ୍ୟ;
        /* The maximum number that can be multiplied by ten with any
           digit added without overflowing an integerPart.  */
      } ଯେତେବେଳେ (p <= D.lastSigDigit && multiplier <= (~ (integerPart) 0 - 9) / 10);

      /* Multiply out the current part.  */
      APInt::tcMultiplyPart(decSignificand, decSignificand, multiplier, val,
                            part_ଗଣନା, part_ଗଣନା + 1, ମିଥ୍ୟା);

      /* If we used another part (likely but not guaranteed), increase
         the count.  */
      ଯଦି (decSignificand[part_ଗଣନା])
        part_ଗଣନା++;
    } ଯେତେବେଳେ (p <= D.lastSigDigit);

    category = fcNormal;
    fs = roundSignificandWithExponent(decSignificand, part_ଗଣନା,
                                      D.exponent, rounding_mode);

    delete [] decSignificand;
  }

  ଫେରନ୍ତୁ fs;
}

ତାର୍କିକ IEEEFloat::convertFromStringSpecials(StringRef str) {
  ଅଚଳ size_t MIN_NAME_SIZE = 3;

  ଯଦି (str.size() < MIN_NAME_SIZE)
    ଫେରନ୍ତୁ ମିଥ୍ୟା;

  ଯଦି (str == "inf" || str == "INFINITY" || str == "+Inf" || str == "+inf") {
    makeInf(ମିଥ୍ୟା);
    ଫେରନ୍ତୁ ସତ୍ୟ;
  }

  ତାର୍କିକ IsNegative = str.consume_front("-");
  ଯଦି (IsNegative) {
    ଯଦି (str.size() < MIN_NAME_SIZE)
      ଫେରନ୍ତୁ ମିଥ୍ୟା;

    ଯଦି (str == "inf" || str == "INFINITY" || str == "Inf") {
      makeInf(ସତ୍ୟ);
      ଫେରନ୍ତୁ ସତ୍ୟ;
    }
  }

  // If we have a 's' (or 'S') prefix, then this is a Signaling NaN.
  ତାର୍କିକ IsSignaling = str.consume_front_insensitive("s");
  ଯଦି (IsSignaling) {
    ଯଦି (str.size() < MIN_NAME_SIZE)
      ଫେରନ୍ତୁ ମିଥ୍ୟା;
  }

  ଯଦି (str.consume_front("nan") || str.consume_front("NaN")) {
    // A NaN without payload.
    ଯଦି (str.empty()) {
      makeNaN(IsSignaling, IsNegative);
      ଫେରନ୍ତୁ ସତ୍ୟ;
    }

    // Allow the payload to be inside parentheses.
    ଯଦି (str.front() == '(') {
      // Parentheses should be balanced (and not empty).
      ଯଦି (str.size() <= 2 || str.back() != ')')
        ଫେରନ୍ତୁ ମିଥ୍ୟା;

      str = str.slice(1, str.size() - 1);
    }

    // Determine the payload number's radix.
    unsigned Radix = 10;
    ଯଦି (str[0] == '0') {
      ଯଦି (str.size() > 1 && tolower(str[1]) == 'x') {
        str = str.drop_front(2);
        Radix = 16;
      } ଅନ୍ୟଥା {
        Radix = 8;
      }
    }

    // Parse the payload and make the NaN.
    APInt Payload;
    ଯଦି (!str.getAsInteger(Radix, Payload)) {
      makeNaN(IsSignaling, IsNegative, &Payload);
      ଫେରନ୍ତୁ ସତ୍ୟ;
    }
  }

  ଫେରନ୍ତୁ ମିଥ୍ୟା;
}

Expected<APFloat::opStatus>
IEEEFloat::convertFromString(StringRef str, roundingMode rounding_mode) {
  ଯଦି (str.empty())
    ଫେରନ୍ତୁ createError("Invalid string length");

  // Handle special cases.
  ଯଦି (convertFromStringSpecials(str))
    ଫେରନ୍ତୁ opOK;

  /* Handle a leading minus sign.  */
  StringRef::iterator p = str.begin();
  size_t slen = str.size();
  sign = *p == '-' ? 1 : 0;
  ଯଦି (sign && !semantics->hasSignedRepr)
    llvm_unreachable(
        "This floating point format does not support signed values");

  ଯଦି (*p == '-' || *p == '+') {
    p++;
    slen--;
    ଯଦି (!slen)
      ଫେରନ୍ତୁ createError("String has no digits");
  }

  ଯଦି (slen >= 2 && p[0] == '0' && (p[1] == 'x' || p[1] == 'X')) {
    ଯଦି (slen == 2)
      ଫେରନ୍ତୁ createError("Invalid string");
    ଫେରନ୍ତୁ convertFromHexadecimalString(StringRef(p + 2, slen - 2),
                                        rounding_mode);
  }

  ଫେରନ୍ତୁ convertFromDecimalString(StringRef(p, slen), rounding_mode);
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
unsigned ପୂର୍ଣ୍ଣସଂଖ୍ୟା IEEEFloat::convertToHexString(ଅକ୍ଷର *dst, unsigned ପୂର୍ଣ୍ଣସଂଖ୍ୟା hexDigits,
                                           ତାର୍କିକ upperCase,
                                           roundingMode rounding_mode) ଅଚଳ {
  ଅକ୍ଷର *p = dst;
  ଯଦି (sign)
    *dst++ = '-';

  ବାଛ (category) {
  ସ୍ଥିତି fcInfinity:
    memcpy (dst, upperCase ? infinityU: infinityL, ଆକାର infinityU - 1);
    dst += ଆକାର infinityL - 1;
    ଭାଙ୍ଗ;

  ସ୍ଥିତି fcNaN:
    memcpy (dst, upperCase ? NaNU: NaNL, ଆକାର NaNU - 1);
    dst += ଆକାର NaNU - 1;
    ଭାଙ୍ଗ;

  ସ୍ଥିତି fcZero:
    *dst++ = '0';
    *dst++ = upperCase ? 'X': 'x';
    *dst++ = '0';
    ଯଦି (hexDigits > 1) {
      *dst++ = '.';
      memset (dst, '0', hexDigits - 1);
      dst += hexDigits - 1;
    }
    *dst++ = upperCase ? 'P': 'p';
    *dst++ = '0';
    ଭାଙ୍ଗ;

  ସ୍ଥିତି fcNormal:
    dst = convertNormalToHexString (dst, hexDigits, upperCase, rounding_mode);
    ଭାଙ୍ଗ;
  }

  *dst = 0;

  ଫେରନ୍ତୁ static_cast<unsigned ପୂର୍ଣ୍ଣସଂଖ୍ୟା>(dst - p);
}

/* Does the hard work of outputting the correctly rounded hexadecimal
   form of a normal floating point number with the specified number of
   hexadecimal digits.  If HEXDIGITS is zero the minimum number of
   digits necessary to print the value precisely is output.  */
ଅକ୍ଷର *IEEEFloat::convertNormalToHexString(ଅକ୍ଷର *dst, unsigned ପୂର୍ଣ୍ଣସଂଖ୍ୟା hexDigits,
                                          ତାର୍କିକ upperCase,
                                          roundingMode rounding_mode) ଅଚଳ {
  *dst++ = '0';
  *dst++ = upperCase ? 'X': 'x';

  ତାର୍କିକ roundUp = ମିଥ୍ୟା;
  ଅଚଳ ଅକ୍ଷର *hexDigitChars = upperCase ? hexDigitsUpper : hexDigitsLower;

  ଅଚଳ integerPart *significand = significandParts();
  unsigned parts_ଗଣନା = part_ଗଣନା();

  /* +3 because the first digit only uses the single integer bit, so
     we have 3 virtual zero most-significant-bits.  */
  unsigned ମୂଲ୍ୟ_Bits = semantics->precision + 3;
  unsigned shift = integerPartWidth - ମୂଲ୍ୟ_Bits % integerPartWidth;

  /* The natural number of digits required ignoring trailing
     insignificant zeroes.  */
  unsigned outputDigits = (ମୂଲ୍ୟ_Bits - significandLSB() + 3) / 4;

  /* hexDigits of zero means use the required number for the
     precision.  Otherwise, see if we are truncating.  If we are,
     find out if we need to round away from zero.  */
  ଯଦି (hexDigits) {
    ଯଦି (hexDigits < outputDigits) {
      /* We are dropping non-zero bits, so need to check how to round.
         "bits" is the number of dropped bits.  */
      unsigned ପୂର୍ଣ୍ଣସଂଖ୍ୟା bits;
      lostFraction fraction;

      bits = ମୂଲ୍ୟ_Bits - hexDigits * 4;
      fraction = lostFractionThroughTruncation (significand, parts_ଗଣନା, bits);
      roundUp = roundAwayFromZero(rounding_mode, fraction, bits);
    }
    outputDigits = hexDigits;
  }

  /* Write the digits consecutively, and start writing in the location
     of the hexadecimal point.  We move the most significant digit
     left and add the hexadecimal point later.  */
  ଅକ୍ଷର *p = ++dst;

  unsigned ଗଣନା_2 = (ମୂଲ୍ୟ_Bits + integerPartWidth - 1) / integerPartWidth;

  ଯେତେବେଳେ (outputDigits && ଗଣନା_2) {
    integerPart part;

    /* Put the most significant integerPartWidth bits in "part".  */
    ଯଦି (--ଗଣନା_2 == parts_ଗଣନା)
      part = 0;  /* An imaginary higher zero part.  */
    ଅନ୍ୟଥା
      part = significand[ଗଣନା_2] << shift;

    ଯଦି (ଗଣନା_2 && shift)
      part |= significand[ଗଣନା_2 - 1] >> (integerPartWidth - shift);

    /* Convert as much of "part" to hexdigits as we can.  */
    unsigned ପୂର୍ଣ୍ଣସଂଖ୍ୟା curDigits = integerPartWidth / 4;

    curDigits = std::min(curDigits, outputDigits);
    dst += partAsHex (dst, part, curDigits, hexDigitChars);
    outputDigits -= curDigits;
  }

  ଯଦି (roundUp) {
    ଅକ୍ଷର *q = dst;

    /* Note that hexDigitChars has a trailing '0'.  */
    କର {
      q--;
      *q = hexDigitChars[hex_Digit_ମୂଲ୍ୟ (*q) + 1];
    } ଯେତେବେଳେ (*q == '0');
    ପ୍ରତିଜ୍ଞା(q >= p);
  } ଅନ୍ୟଥା {
    /* Add trailing zeroes.  */
    memset (dst, '0', outputDigits);
    dst += outputDigits;
  }

  /* Move the most significant digit to before the point, and if there
     is something after the decimal point add it.  This must come
     after rounding above.  */
  p[-1] = p[0];
  ଯଦି (dst -1 == p)
    dst--;
  ଅନ୍ୟଥା
    p[0] = '.';

  /* Finally output the exponent.  */
  *dst++ = upperCase ? 'P': 'p';

  ଫେରନ୍ତୁ writeSignedDecimal (dst, exponent);
}

hash_code hash_ମୂଲ୍ୟ(ଅଚଳ IEEEFloat &Arg) {
  ଯଦି (!Arg.isFiniteNonZero())
    ଫେରନ୍ତୁ hash_combine((uint8_t)Arg.category,
                        // NaN has no sign, fix it at zero.
                        Arg.isNaN() ? (uint8_t)0 : (uint8_t)Arg.sign,
                        Arg.semantics->precision);

  // Normal floats need their exponent and significand hashed.
  ଫେରନ୍ତୁ hash_combine((uint8_t)Arg.category, (uint8_t)Arg.sign,
                      Arg.semantics->precision, Arg.exponent,
                      hash_combine_range(
                        Arg.significandParts(),
                        Arg.significandParts() + Arg.part_ଗଣନା()));
}

// Conversion from APFloat to/from host float/double.  It may eventually be
// possible to eliminate these and have everybody deal with APFloats, but that
// will take a while.  This approach will not easily extend to long double.
// Current implementation requires integerPartWidth==64, which is correct at
// the moment but could be made more general.

// Denormals have exponent minExponent in APFloat, but minExponent-1 in
// the actual IEEE respresentations.  We compensate for that here.

APInt IEEEFloat::convertF80LongDoubleAPFloatToAPInt() ଅଚଳ {
  ପ୍ରତିଜ୍ଞା(part_ଗଣନା() == 2);
  ଫେରନ୍ତୁ convertIEEEFloatToAPInt<APFloatBase::semX87DoubleExtended>();
}

APInt IEEEFloat::convertPPCDoubleDoubleLegacyAPFloatToAPInt() ଅଚଳ {
  ପ୍ରତିଜ୍ଞା(semantics ==
         (ଅଚଳ llvm::fltSemantics *)&APFloatBase::semPPCDoubleDoubleLegacy);
  ପ୍ରତିଜ୍ଞା(part_ଗଣନା()==2);

  uint64_t ଶବ୍ଦଗୁଡ଼ିକ[2];
  ତାର୍କିକ losesInfo;

  // Convert number to double.  To avoid spurious underflows, we re-
  // normalize against the "double" minExponent first, and only *then*
  // truncate the mantissa.  The result of that second conversion
  // may be inexact, but should never underflow.
  // Declare fltSemantics before APFloat that uses it (and
  // saves pointer to it) to ensure correct destruction order.
  fltSemantics extendedSemantics = *semantics;
  extendedSemantics.minExponent = APFloatBase::semIEEEdouble.minExponent;
  IEEEFloat extended(*ଏହି);
  [[maybe_unused]] opStatus fs =
      extended.convert(extendedSemantics, rmNearestTiesToEven, &losesInfo);
  ପ୍ରତିଜ୍ଞା(fs == opOK && !losesInfo);

  IEEEFloat u(extended);
  fs = u.convert(APFloatBase::semIEEEdouble, rmNearestTiesToEven, &losesInfo);
  ପ୍ରତିଜ୍ଞା(fs == opOK || fs == opInexact);
  ଶବ୍ଦଗୁଡ଼ିକ[0] = *u.convertDoubleAPFloatToAPInt().getRawData();

  // If conversion was exact or resulted in a special case, we're done;
  // just set the second double to zero.  Otherwise, re-convert back to
  // the extended format and compute the difference.  This now should
  // convert exactly to double.
  ଯଦି (u.isFiniteNonZero() && losesInfo) {
    fs = u.convert(extendedSemantics, rmNearestTiesToEven, &losesInfo);
    ପ୍ରତିଜ୍ଞା(fs == opOK && !losesInfo);

    IEEEFloat v(extended);
    v.subtract(u, rmNearestTiesToEven);
    fs = v.convert(APFloatBase::semIEEEdouble, rmNearestTiesToEven, &losesInfo);
    ପ୍ରତିଜ୍ଞା(fs == opOK && !losesInfo);
    ଶବ୍ଦଗୁଡ଼ିକ[1] = *v.convertDoubleAPFloatToAPInt().getRawData();
  } ଅନ୍ୟଥା {
    ଶବ୍ଦଗୁଡ଼ିକ[1] = 0;
  }

  ଫେରନ୍ତୁ APInt(128, ଶବ୍ଦଗୁଡ଼ିକ);
}

template <ଅଚଳ fltSemantics &S>
APInt IEEEFloat::convertIEEEFloatToAPInt() ଅଚଳ {
  ପ୍ରତିଜ୍ଞା(semantics == &S);
  constexpr unsigned ପୂର୍ଣ୍ଣସଂଖ୍ୟା trailing_significand_bits =
      S.precision - 1 + S.hasExplicitIntegerBit;
  constexpr ପୂର୍ଣ୍ଣସଂଖ୍ୟା integer_bit_part = (S.precision - 1) / integerPartWidth;
  constexpr integerPart integer_bit = integerPart{1}
                                      << ((S.precision - 1) % integerPartWidth);
  constexpr uint64_t significand_mask = integer_bit - 1;
  constexpr unsigned ପୂର୍ଣ୍ଣସଂଖ୍ୟା exponent_bits =
      S.sizeInBits - (S.hasSignedRepr ? 1 : 0) - trailing_significand_bits;
  static_assert(exponent_bits < 64);
  constexpr uint64_t exponent_mask = (uint64_t{1} << exponent_bits) - 1;
  constexpr ତାର୍କିକ is_zero_exp_reserved = S.hasDenormals || S.hasZero;
  constexpr ପୂର୍ଣ୍ଣସଂଖ୍ୟା bias = -(S.minExponent - (is_zero_exp_reserved ? 1 : 0));

  uint64_t myexponent;
  std::array<integerPart, part_ଗଣନା_For_Bits(trailing_significand_bits)>
      mysignificand;

  ଯଦି (isFiniteNonZero()) {
    myexponent = exponent + bias;
    std::copy_n(significandParts(), mysignificand.size(),
                mysignificand.begin());
    ଯଦି (myexponent == 1 &&
        !(significandParts()[integer_bit_part] & integer_bit))
      myexponent = 0; // denormal
  } ଅନ୍ୟଥା ଯଦି (category == fcZero) {
    ଯଦି (!S.hasZero)
      llvm_unreachable("semantics does not support zero!");
    myexponent = ::exponentZero(S) + bias;
    mysignificand.fill(0);
  } ଅନ୍ୟଥା ଯଦି (category == fcInfinity) {
    ଯଦି (S.nonFiniteBehavior == fltNonfiniteBehavior::NanOnly ||
        S.nonFiniteBehavior == fltNonfiniteBehavior::FiniteOnly)
      llvm_unreachable("semantics don't support inf!");
    myexponent = ::exponentInf(S) + bias;
    mysignificand.fill(0);
    ଯଦି constexpr (S.hasExplicitIntegerBit) {
      mysignificand[0] = integerPart{1} << (trailing_significand_bits - 1);
    }
  } ଅନ୍ୟଥା {
    ପ୍ରତିଜ୍ଞା(category == fcNaN && "Unknown category!");
    ଯଦି (S.nonFiniteBehavior == fltNonfiniteBehavior::FiniteOnly)
      llvm_unreachable("semantics don't support NaN!");
    myexponent = ::exponentNaN(S) + bias;
    std::copy_n(significandParts(), mysignificand.size(),
                mysignificand.begin());
  }
  std::array<uint64_t, (S.sizeInBits + 63) / 64> ଶବ୍ଦଗୁଡ଼ିକ;
  auto ଶବ୍ଦଗୁଡ଼ିକ_iter =
      std::copy_n(mysignificand.begin(), mysignificand.size(), ଶବ୍ଦଗୁଡ଼ିକ.begin());
  ଯଦି constexpr (!S.hasExplicitIntegerBit) {
    ଯଦି constexpr (significand_mask != 0 || trailing_significand_bits == 0) {
      // Clear the integer bit.
      ଶବ୍ଦଗୁଡ଼ିକ[mysignificand.size() - 1] &= significand_mask;
    }
  }
  std::fill(ଶବ୍ଦଗୁଡ଼ିକ_iter, ଶବ୍ଦଗୁଡ଼ିକ.end(), uint64_t{0});
  constexpr size_t last_word = ଶବ୍ଦଗୁଡ଼ିକ.size() - 1;
  ଯଦି constexpr (S.hasSignedRepr) {
    uint64_t shifted_sign = static_cast<uint64_t>(sign & 1)
                            << ((S.sizeInBits - 1) % 64);
    ଶବ୍ଦଗୁଡ଼ିକ[last_word] |= shifted_sign;
  }
  uint64_t shifted_exponent = (myexponent & exponent_mask)
                              << (trailing_significand_bits % 64);
  ଶବ୍ଦଗୁଡ଼ିକ[last_word] |= shifted_exponent;
  ଯଦି constexpr (last_word == 0) {
    ଫେରନ୍ତୁ APInt(S.sizeInBits, ଶବ୍ଦଗୁଡ଼ିକ[0]);
  }
  ଫେରନ୍ତୁ APInt(S.sizeInBits, ଶବ୍ଦଗୁଡ଼ିକ);
}

APInt IEEEFloat::convertQuadrupleAPFloatToAPInt() ଅଚଳ {
  ପ୍ରତିଜ୍ଞା(part_ଗଣନା() == 2);
  ଫେରନ୍ତୁ convertIEEEFloatToAPInt<APFloatBase::semIEEEquad>();
}

APInt IEEEFloat::convertDoubleAPFloatToAPInt() ଅଚଳ {
  ପ୍ରତିଜ୍ଞା(part_ଗଣନା()==1);
  ଫେରନ୍ତୁ convertIEEEFloatToAPInt<APFloatBase::semIEEEdouble>();
}

APInt IEEEFloat::convertFloatAPFloatToAPInt() ଅଚଳ {
  ପ୍ରତିଜ୍ଞା(part_ଗଣନା()==1);
  ଫେରନ୍ତୁ convertIEEEFloatToAPInt<APFloatBase::semIEEEsingle>();
}

APInt IEEEFloat::convertBFloatAPFloatToAPInt() ଅଚଳ {
  ପ୍ରତିଜ୍ଞା(part_ଗଣନା() == 1);
  ଫେରନ୍ତୁ convertIEEEFloatToAPInt<APFloatBase::semBFloat>();
}

APInt IEEEFloat::convertHalfAPFloatToAPInt() ଅଚଳ {
  ପ୍ରତିଜ୍ଞା(part_ଗଣନା()==1);
  ଫେରନ୍ତୁ convertIEEEFloatToAPInt<APFloatBase::APFloatBase::semIEEEhalf>();
}

APInt IEEEFloat::convertFloat8E5M2APFloatToAPInt() ଅଚଳ {
  ପ୍ରତିଜ୍ଞା(part_ଗଣନା() == 1);
  ଫେରନ୍ତୁ convertIEEEFloatToAPInt<APFloatBase::semFloat8E5M2>();
}

APInt IEEEFloat::convertFloat8E5M2FNUZAPFloatToAPInt() ଅଚଳ {
  ପ୍ରତିଜ୍ଞା(part_ଗଣନା() == 1);
  ଫେରନ୍ତୁ convertIEEEFloatToAPInt<APFloatBase::semFloat8E5M2FNUZ>();
}

APInt IEEEFloat::convertFloat8E4M3APFloatToAPInt() ଅଚଳ {
  ପ୍ରତିଜ୍ଞା(part_ଗଣନା() == 1);
  ଫେରନ୍ତୁ convertIEEEFloatToAPInt<APFloatBase::semFloat8E4M3>();
}

APInt IEEEFloat::convertFloat8E4M3FNAPFloatToAPInt() ଅଚଳ {
  ପ୍ରତିଜ୍ଞା(part_ଗଣନା() == 1);
  ଫେରନ୍ତୁ convertIEEEFloatToAPInt<APFloatBase::semFloat8E4M3FN>();
}

APInt IEEEFloat::convertFloat8E4M3FNUZAPFloatToAPInt() ଅଚଳ {
  ପ୍ରତିଜ୍ଞା(part_ଗଣନା() == 1);
  ଫେରନ୍ତୁ convertIEEEFloatToAPInt<APFloatBase::semFloat8E4M3FNUZ>();
}

APInt IEEEFloat::convertFloat8E4M3B11FNUZAPFloatToAPInt() ଅଚଳ {
  ପ୍ରତିଜ୍ଞା(part_ଗଣନା() == 1);
  ଫେରନ୍ତୁ convertIEEEFloatToAPInt<APFloatBase::semFloat8E4M3B11FNUZ>();
}

APInt IEEEFloat::convertFloat8E3M4APFloatToAPInt() ଅଚଳ {
  ପ୍ରତିଜ୍ଞା(part_ଗଣନା() == 1);
  ଫେରନ୍ତୁ convertIEEEFloatToAPInt<APFloatBase::semFloat8E3M4>();
}

APInt IEEEFloat::convertFloatTF32APFloatToAPInt() ଅଚଳ {
  ପ୍ରତିଜ୍ଞା(part_ଗଣନା() == 1);
  ଫେରନ୍ତୁ convertIEEEFloatToAPInt<APFloatBase::semFloatTF32>();
}

APInt IEEEFloat::convertFloat8E8M0FNUAPFloatToAPInt() ଅଚଳ {
  ପ୍ରତିଜ୍ଞା(part_ଗଣନା() == 1);
  ଫେରନ୍ତୁ convertIEEEFloatToAPInt<APFloatBase::semFloat8E8M0FNU>();
}

APInt IEEEFloat::convertFloat8E5M3FNUAPFloatToAPInt() ଅଚଳ {
  ପ୍ରତିଜ୍ଞା(part_ଗଣନା() == 1);
  ଫେରନ୍ତୁ convertIEEEFloatToAPInt<APFloatBase::semFloat8E5M3FNU>();
}

APInt IEEEFloat::convertFloat6E3M2FNAPFloatToAPInt() ଅଚଳ {
  ପ୍ରତିଜ୍ଞା(part_ଗଣନା() == 1);
  ଫେରନ୍ତୁ convertIEEEFloatToAPInt<APFloatBase::semFloat6E3M2FN>();
}

APInt IEEEFloat::convertFloat6E2M3FNAPFloatToAPInt() ଅଚଳ {
  ପ୍ରତିଜ୍ଞା(part_ଗଣନା() == 1);
  ଫେରନ୍ତୁ convertIEEEFloatToAPInt<APFloatBase::semFloat6E2M3FN>();
}

APInt IEEEFloat::convertFloat4E2M1FNAPFloatToAPInt() ଅଚଳ {
  ପ୍ରତିଜ୍ଞା(part_ଗଣନା() == 1);
  ଫେରନ୍ତୁ convertIEEEFloatToAPInt<APFloatBase::semFloat4E2M1FN>();
}

// This function creates an APInt that is just a bit map of the floating
// point constant as it would appear in memory.  It is not a conversion,
// and treating the result as a normal integer is unlikely to be useful.

APInt IEEEFloat::bitcastToAPInt() ଅଚଳ {
  ଯଦି (semantics == (ଅଚଳ llvm::fltSemantics *)&APFloatBase::semIEEEhalf)
    ଫେରନ୍ତୁ convertHalfAPFloatToAPInt();

  ଯଦି (semantics == (ଅଚଳ llvm::fltSemantics *)&APFloatBase::semBFloat)
    ଫେରନ୍ତୁ convertBFloatAPFloatToAPInt();

  ଯଦି (semantics == (ଅଚଳ llvm::fltSemantics *)&APFloatBase::semIEEEsingle)
    ଫେରନ୍ତୁ convertFloatAPFloatToAPInt();

  ଯଦି (semantics == (ଅଚଳ llvm::fltSemantics *)&APFloatBase::semIEEEdouble)
    ଫେରନ୍ତୁ convertDoubleAPFloatToAPInt();

  ଯଦି (semantics == (ଅଚଳ llvm::fltSemantics *)&APFloatBase::semIEEEquad)
    ଫେରନ୍ତୁ convertQuadrupleAPFloatToAPInt();

  ଯଦି (semantics ==
      (ଅଚଳ llvm::fltSemantics *)&APFloatBase::semX87DoubleExtended)
    ଫେରନ୍ତୁ convertF80LongDoubleAPFloatToAPInt();

  ଯଦି (semantics ==
      (ଅଚଳ llvm::fltSemantics *)&APFloatBase::semPPCDoubleDoubleLegacy)
    ଫେରନ୍ତୁ convertPPCDoubleDoubleLegacyAPFloatToAPInt();

  ଯଦି (semantics == (ଅଚଳ llvm::fltSemantics *)&APFloatBase::semFloat8E5M2)
    ଫେରନ୍ତୁ convertFloat8E5M2APFloatToAPInt();

  ଯଦି (semantics == (ଅଚଳ llvm::fltSemantics *)&APFloatBase::semFloat8E5M2FNUZ)
    ଫେରନ୍ତୁ convertFloat8E5M2FNUZAPFloatToAPInt();

  ଯଦି (semantics == (ଅଚଳ llvm::fltSemantics *)&APFloatBase::semFloat8E4M3)
    ଫେରନ୍ତୁ convertFloat8E4M3APFloatToAPInt();

  ଯଦି (semantics == (ଅଚଳ llvm::fltSemantics *)&APFloatBase::semFloat8E4M3FN)
    ଫେରନ୍ତୁ convertFloat8E4M3FNAPFloatToAPInt();

  ଯଦି (semantics == (ଅଚଳ llvm::fltSemantics *)&APFloatBase::semFloat8E4M3FNUZ)
    ଫେରନ୍ତୁ convertFloat8E4M3FNUZAPFloatToAPInt();

  ଯଦି (semantics ==
      (ଅଚଳ llvm::fltSemantics *)&APFloatBase::semFloat8E4M3B11FNUZ)
    ଫେରନ୍ତୁ convertFloat8E4M3B11FNUZAPFloatToAPInt();

  ଯଦି (semantics == (ଅଚଳ llvm::fltSemantics *)&APFloatBase::semFloat8E3M4)
    ଫେରନ୍ତୁ convertFloat8E3M4APFloatToAPInt();

  ଯଦି (semantics == (ଅଚଳ llvm::fltSemantics *)&APFloatBase::semFloatTF32)
    ଫେରନ୍ତୁ convertFloatTF32APFloatToAPInt();

  ଯଦି (semantics == (ଅଚଳ llvm::fltSemantics *)&APFloatBase::semFloat8E8M0FNU)
    ଫେରନ୍ତୁ convertFloat8E8M0FNUAPFloatToAPInt();

  ଯଦି (semantics == (ଅଚଳ llvm::fltSemantics *)&APFloatBase::semFloat8E5M3FNU)
    ଫେରନ୍ତୁ convertFloat8E5M3FNUAPFloatToAPInt();

  ଯଦି (semantics == (ଅଚଳ llvm::fltSemantics *)&APFloatBase::semFloat6E3M2FN)
    ଫେରନ୍ତୁ convertFloat6E3M2FNAPFloatToAPInt();

  ଯଦି (semantics == (ଅଚଳ llvm::fltSemantics *)&APFloatBase::semFloat6E2M3FN)
    ଫେରନ୍ତୁ convertFloat6E2M3FNAPFloatToAPInt();

  ଯଦି (semantics == (ଅଚଳ llvm::fltSemantics *)&APFloatBase::semFloat4E2M1FN)
    ଫେରନ୍ତୁ convertFloat4E2M1FNAPFloatToAPInt();

  llvm_unreachable("unknown format!");
}

ଦଶମିକ IEEEFloat::convertToFloat() ଅଚଳ {
  ପ୍ରତିଜ୍ଞା(semantics == (ଅଚଳ llvm::fltSemantics *)&APFloatBase::semIEEEsingle &&
         "Float semantics are not IEEEsingle");
  APInt api = bitcastToAPInt();
  ଫେରନ୍ତୁ api.bitsToFloat();
}

ଦ୍ୱିଗୁଣ IEEEFloat::convertToDouble() ଅଚଳ {
  ପ୍ରତିଜ୍ଞା(semantics == (ଅଚଳ llvm::fltSemantics *)&APFloatBase::semIEEEdouble &&
         "Float semantics are not IEEEdouble");
  APInt api = bitcastToAPInt();
  ଫେରନ୍ତୁ api.bitsToDouble();
}

#ifdef HAS_IEE754_FLOAT128
float128 IEEEFloat::convertToQuad() ଅଚଳ {
  ପ୍ରତିଜ୍ଞା(semantics == (ଅଚଳ llvm::fltSemantics *)&APFloatBase::semIEEEquad &&
         "Float semantics are not IEEEquads");
  APInt api = bitcastToAPInt();
  ଫେରନ୍ତୁ api.bitsToQuad();
}
#endif

ଶୂନ୍ୟ IEEEFloat::initFromF80LongDoubleAPInt(ଅଚଳ APInt &api) {
  ଫେରନ୍ତୁ initFromIEEEAPInt<APFloatBase::semX87DoubleExtended>(api);
}

ଶୂନ୍ୟ IEEEFloat::initFromPPCDoubleDoubleLegacyAPInt(ଅଚଳ APInt &api) {
  uint64_t i1 = api.getRawData()[0];
  uint64_t i2 = api.getRawData()[1];
  ତାର୍କିକ losesInfo;

  // Get the first double and convert to our format.
  initFromDoubleAPInt(APInt(64, i1));
  [[maybe_unused]] opStatus fs = convert(APFloatBase::semPPCDoubleDoubleLegacy,
                                         rmNearestTiesToEven, &losesInfo);
  // (convert may return opInvalidOp if i1 is an sNaN).
  ପ୍ରତିଜ୍ଞା((fs == opOK || fs == opInvalidOp) && !losesInfo);

  // Unless we have a special case, add in second double.
  ଯଦି (isFiniteNonZero()) {
    IEEEFloat v(APFloatBase::semIEEEdouble, APInt(64, i2));
    // The first double determines the category, so ignore a non-canonical
    // second double.
    ଯଦି (v.isFinite()) {
      fs = v.convert(APFloatBase::semPPCDoubleDoubleLegacy, rmNearestTiesToEven,
                     &losesInfo);
      ପ୍ରତିଜ୍ଞା(fs == opOK && !losesInfo);

      add(v, rmNearestTiesToEven);
    }
  }
}

// The E8M0 format has the following characteristics:
// It is an 8-bit unsigned format with only exponents (no actual significand).
// No encodings for {zero, infinities or denorms}.
// NaN is represented by all 1's.
// Bias is 127.
ଶୂନ୍ୟ IEEEFloat::initFromFloat8E8M0FNUAPInt(ଅଚଳ APInt &api) {
  initFromIEEEAPInt<APFloatBase::semFloat8E8M0FNU>(api);
}

ଶୂନ୍ୟ IEEEFloat::initFromFloat8E5M3FNUAPInt(ଅଚଳ APInt &api) {
  initFromIEEEAPInt<APFloatBase::semFloat8E5M3FNU>(api);
}

template <ଅଚଳ fltSemantics &S>
ଶୂନ୍ୟ IEEEFloat::initFromIEEEAPInt(ଅଚଳ APInt &api) {
  ପ୍ରତିଜ୍ଞା(api.getBitWidth() == S.sizeInBits);

  constexpr unsigned ପୂର୍ଣ୍ଣସଂଖ୍ୟା trailing_significand_bits =
      S.precision - 1 + S.hasExplicitIntegerBit;
  constexpr integerPart integer_bit =
      integerPart{1} << (trailing_significand_bits % integerPartWidth);
  constexpr uint64_t significand_mask = integer_bit - 1;
  constexpr unsigned ପୂର୍ଣ୍ଣସଂଖ୍ୟା exponent_bits =
      S.sizeInBits - (S.hasSignedRepr ? 1 : 0) - trailing_significand_bits;
  static_assert(exponent_bits < 64);
  constexpr unsigned ପୂର୍ଣ୍ଣସଂଖ୍ୟା stored_significand_parts =
      part_ଗଣନା_For_Bits(trailing_significand_bits + 1);
  constexpr uint64_t exponent_mask = (uint64_t{1} << exponent_bits) - 1;
  constexpr ତାର୍କିକ is_zero_exp_reserved = S.hasDenormals || S.hasZero;
  constexpr ପୂର୍ଣ୍ଣସଂଖ୍ୟା bias = -(S.minExponent - (is_zero_exp_reserved ? 1 : 0));
  constexpr ତାର୍କିକ has_significand = trailing_significand_bits > 0;

  // Copy the bits of the significand. We need to clear out the exponent and
  // sign bit in the last word.
  std::array<integerPart, stored_significand_parts> mysignificand;
  ଯଦି constexpr (has_significand) {
    std::copy_n(api.getRawData(), mysignificand.size(), mysignificand.begin());
    ଯଦି constexpr (significand_mask != 0 || S.precision >= integerPartWidth) {
      mysignificand[mysignificand.size() - 1] &= significand_mask;
    }
  } ଅନ୍ୟଥା {
    std::fill_n(mysignificand.begin(), mysignificand.size(), 0);
    // Always set integer bit to 1 for consistency in APFloat's internal
    // representation.
    mysignificand[0] = 1;
  }

  // We assume the last word holds the sign bit, the exponent, and potentially
  // some of the trailing significand field.
  uint64_t last_word = api.getRawData()[api.get_ଅଙ୍କ_ଶବ୍ଦଗୁଡ଼ିକ() - 1];
  uint64_t myexponent =
      (last_word >> (trailing_significand_bits % 64)) & exponent_mask;

  initialize(&S);
  ପ୍ରତିଜ୍ଞା(part_ଗଣନା() == mysignificand.size());

  sign = S.hasSignedRepr
             ? static_cast<unsigned ପୂର୍ଣ୍ଣସଂଖ୍ୟା>(last_word >> ((S.sizeInBits - 1) % 64))
             : 0;

  ତାର୍କିକ all_zero_significand =
      has_significand && llvm::all_of(mysignificand, equal_to(0));

  ତାର୍କିକ is_zero = myexponent == 0 && all_zero_significand && S.hasZero;

  ଯଦି constexpr (S.nonFiniteBehavior == fltNonfiniteBehavior::IEEE754) {
    ତାର୍କିକ is_inf = ମିଥ୍ୟା;

    ଯଦି constexpr (S.hasExplicitIntegerBit) {
      // This is only used and tested for x87DoubleExtended
      static_assert(S.precision == 64);
      constexpr integerPart significand_mask_no_int_bit =
          (uint64_t{1} << (trailing_significand_bits - 1)) - 1;
      ଅଚଳ integerPart myintegerbit =
          mysignificand[0] >> (trailing_significand_bits - 1);

      is_inf = myexponent - bias == ::exponentInf(S) && myintegerbit == 1 &&
               (mysignificand[0] & significand_mask_no_int_bit) == 0;
    } ଅନ୍ୟଥା {
      is_inf = myexponent - bias == ::exponentInf(S) && all_zero_significand;
    }

    ଯଦି (is_inf) {
      makeInf(sign);
      ଫେରନ୍ତୁ;
    }
  }

  ତାର୍କିକ is_nan = ମିଥ୍ୟା;

  ଯଦି constexpr (S.nanEncoding == fltNanEncoding::IEEE) {
    ଯଦି constexpr (S.hasExplicitIntegerBit) {
      // This is only used and tested for x87DoubleExtended
      static_assert(S.precision == 64);
      ଅଚଳ integerPart myintegerbit =
          mysignificand[0] >> (trailing_significand_bits - 1);
      constexpr integerPart significand_mask_no_int_bit =
          (uint64_t{1} << (trailing_significand_bits - 1)) - 1;

      ଯଦି (myexponent - bias == ::exponentNaN(S) &&
          (mysignificand[0] & significand_mask_no_int_bit) != 0) {
        // regular NaN and pseudoNaN
        is_nan = ସତ୍ୟ;
      } ଅନ୍ୟଥା ଯଦି (myexponent - bias == ::exponentNaN(S) &&
                 (mysignificand[0] & significand_mask_no_int_bit) == 0) {
        // pseudoinfinity
        is_nan = ସତ୍ୟ;
      } ଅନ୍ୟଥା ଯଦି (myexponent - bias != ::exponentNaN(S) && myexponent != 0 &&
                 myintegerbit == 0) {
        // unnormal
        is_nan = ସତ୍ୟ;
      }
    } ଅନ୍ୟଥା {
      is_nan = myexponent - bias == ::exponentNaN(S) && !all_zero_significand;
    }
  } ଅନ୍ୟଥା ଯଦି constexpr (S.nanEncoding == fltNanEncoding::AllOnes) {
    ତାର୍କିକ all_ones_significand =
        std::all_of(mysignificand.begin(), mysignificand.end() - 1,
                    [](integerPart bits) { ଫେରନ୍ତୁ bits == ~integerPart{0}; }) &&
        (!significand_mask ||
         mysignificand[mysignificand.size() - 1] == significand_mask);
    is_nan = myexponent - bias == ::exponentNaN(S) && all_ones_significand;
  } ଅନ୍ୟଥା ଯଦି constexpr (S.nanEncoding == fltNanEncoding::NegativeZero) {
    is_nan = is_zero && sign;
  }

  ଯଦି (is_nan) {
    category = fcNaN;
    exponent = ::exponentNaN(S);
    std::copy_n(mysignificand.begin(), mysignificand.size(),
                significandParts());
    ଫେରନ୍ତୁ;
  }

  ଯଦି (is_zero) {
    makeZero(sign);
    ଫେରନ୍ତୁ;
  }

  category = fcNormal;
  exponent = myexponent - bias;
  std::copy_n(mysignificand.begin(), mysignificand.size(), significandParts());
  ଯଦି (myexponent == 0 && S.hasDenormals) // denormal
    exponent = S.minExponent;
  ଅନ୍ୟଥା {
    ଯଦି constexpr (!S.hasExplicitIntegerBit) {
      significandParts()[mysignificand.size() - 1] |= integer_bit;
    }
  }
}

ଶୂନ୍ୟ IEEEFloat::initFromQuadrupleAPInt(ଅଚଳ APInt &api) {
  initFromIEEEAPInt<APFloatBase::semIEEEquad>(api);
}

ଶୂନ୍ୟ IEEEFloat::initFromDoubleAPInt(ଅଚଳ APInt &api) {
  initFromIEEEAPInt<APFloatBase::semIEEEdouble>(api);
}

ଶୂନ୍ୟ IEEEFloat::initFromFloatAPInt(ଅଚଳ APInt &api) {
  initFromIEEEAPInt<APFloatBase::semIEEEsingle>(api);
}

ଶୂନ୍ୟ IEEEFloat::initFromBFloatAPInt(ଅଚଳ APInt &api) {
  initFromIEEEAPInt<APFloatBase::semBFloat>(api);
}

ଶୂନ୍ୟ IEEEFloat::initFromHalfAPInt(ଅଚଳ APInt &api) {
  initFromIEEEAPInt<APFloatBase::semIEEEhalf>(api);
}

ଶୂନ୍ୟ IEEEFloat::initFromFloat8E5M2APInt(ଅଚଳ APInt &api) {
  initFromIEEEAPInt<APFloatBase::semFloat8E5M2>(api);
}

ଶୂନ୍ୟ IEEEFloat::initFromFloat8E5M2FNUZAPInt(ଅଚଳ APInt &api) {
  initFromIEEEAPInt<APFloatBase::semFloat8E5M2FNUZ>(api);
}

ଶୂନ୍ୟ IEEEFloat::initFromFloat8E4M3APInt(ଅଚଳ APInt &api) {
  initFromIEEEAPInt<APFloatBase::semFloat8E4M3>(api);
}

ଶୂନ୍ୟ IEEEFloat::initFromFloat8E4M3FNAPInt(ଅଚଳ APInt &api) {
  initFromIEEEAPInt<APFloatBase::semFloat8E4M3FN>(api);
}

ଶୂନ୍ୟ IEEEFloat::initFromFloat8E4M3FNUZAPInt(ଅଚଳ APInt &api) {
  initFromIEEEAPInt<APFloatBase::semFloat8E4M3FNUZ>(api);
}

ଶୂନ୍ୟ IEEEFloat::initFromFloat8E4M3B11FNUZAPInt(ଅଚଳ APInt &api) {
  initFromIEEEAPInt<APFloatBase::semFloat8E4M3B11FNUZ>(api);
}

ଶୂନ୍ୟ IEEEFloat::initFromFloat8E3M4APInt(ଅଚଳ APInt &api) {
  initFromIEEEAPInt<APFloatBase::semFloat8E3M4>(api);
}

ଶୂନ୍ୟ IEEEFloat::initFromFloatTF32APInt(ଅଚଳ APInt &api) {
  initFromIEEEAPInt<APFloatBase::semFloatTF32>(api);
}

ଶୂନ୍ୟ IEEEFloat::initFromFloat6E3M2FNAPInt(ଅଚଳ APInt &api) {
  initFromIEEEAPInt<APFloatBase::semFloat6E3M2FN>(api);
}

ଶୂନ୍ୟ IEEEFloat::initFromFloat6E2M3FNAPInt(ଅଚଳ APInt &api) {
  initFromIEEEAPInt<APFloatBase::semFloat6E2M3FN>(api);
}

ଶୂନ୍ୟ IEEEFloat::initFromFloat4E2M1FNAPInt(ଅଚଳ APInt &api) {
  initFromIEEEAPInt<APFloatBase::semFloat4E2M1FN>(api);
}

/// Treat api as containing the bits of a floating point number.
ଶୂନ୍ୟ IEEEFloat::initFromAPInt(ଅଚଳ fltSemantics *Sem, ଅଚଳ APInt &api) {
  ପ୍ରତିଜ୍ଞା(api.getBitWidth() == Sem->sizeInBits);
  ଯଦି (Sem == &APFloatBase::semIEEEhalf)
    ଫେରନ୍ତୁ initFromHalfAPInt(api);
  ଯଦି (Sem == &APFloatBase::semBFloat)
    ଫେରନ୍ତୁ initFromBFloatAPInt(api);
  ଯଦି (Sem == &APFloatBase::semIEEEsingle)
    ଫେରନ୍ତୁ initFromFloatAPInt(api);
  ଯଦି (Sem == &APFloatBase::semIEEEdouble)
    ଫେରନ୍ତୁ initFromDoubleAPInt(api);
  ଯଦି (Sem == &APFloatBase::semIEEEquad)
    ଫେରନ୍ତୁ initFromQuadrupleAPInt(api);
  ଯଦି (Sem == &APFloatBase::semX87DoubleExtended)
    ଫେରନ୍ତୁ initFromF80LongDoubleAPInt(api);
  ଯଦି (Sem == &APFloatBase::semPPCDoubleDoubleLegacy)
    ଫେରନ୍ତୁ initFromPPCDoubleDoubleLegacyAPInt(api);
  ଯଦି (Sem == &APFloatBase::semFloat8E5M2)
    ଫେରନ୍ତୁ initFromFloat8E5M2APInt(api);
  ଯଦି (Sem == &APFloatBase::semFloat8E5M2FNUZ)
    ଫେରନ୍ତୁ initFromFloat8E5M2FNUZAPInt(api);
  ଯଦି (Sem == &APFloatBase::semFloat8E4M3)
    ଫେରନ୍ତୁ initFromFloat8E4M3APInt(api);
  ଯଦି (Sem == &APFloatBase::semFloat8E4M3FN)
    ଫେରନ୍ତୁ initFromFloat8E4M3FNAPInt(api);
  ଯଦି (Sem == &APFloatBase::semFloat8E4M3FNUZ)
    ଫେରନ୍ତୁ initFromFloat8E4M3FNUZAPInt(api);
  ଯଦି (Sem == &APFloatBase::semFloat8E4M3B11FNUZ)
    ଫେରନ୍ତୁ initFromFloat8E4M3B11FNUZAPInt(api);
  ଯଦି (Sem == &APFloatBase::semFloat8E3M4)
    ଫେରନ୍ତୁ initFromFloat8E3M4APInt(api);
  ଯଦି (Sem == &APFloatBase::semFloatTF32)
    ଫେରନ୍ତୁ initFromFloatTF32APInt(api);
  ଯଦି (Sem == &APFloatBase::semFloat8E8M0FNU)
    ଫେରନ୍ତୁ initFromFloat8E8M0FNUAPInt(api);
  ଯଦି (Sem == &APFloatBase::semFloat8E5M3FNU)
    ଫେରନ୍ତୁ initFromFloat8E5M3FNUAPInt(api);
  ଯଦି (Sem == &APFloatBase::semFloat6E3M2FN)
    ଫେରନ୍ତୁ initFromFloat6E3M2FNAPInt(api);
  ଯଦି (Sem == &APFloatBase::semFloat6E2M3FN)
    ଫେରନ୍ତୁ initFromFloat6E2M3FNAPInt(api);
  ଯଦି (Sem == &APFloatBase::semFloat4E2M1FN)
    ଫେରନ୍ତୁ initFromFloat4E2M1FNAPInt(api);

  llvm_unreachable("unsupported semantics");
}

/// Make this number the largest magnitude normal number in the given
/// semantics.
ଶୂନ୍ୟ IEEEFloat::makeLargest(ତାର୍କିକ Negative) {
  ଯଦି (Negative && !semantics->hasSignedRepr)
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
  unsigned Part_ଗଣନା = part_ଗଣନା();
  memset(significand, 0xFF, ଆକାର(integerPart)*(Part_ଗଣନା - 1));

  // Set the high integerPart especially setting all unused top bits for
  // internal consistency.
  ଅଚଳ unsigned ଅଙ୍କ_Unused_ଉପର_Bits =
    Part_ଗଣନା*integerPartWidth - semantics->precision;
  significand[Part_ଗଣନା - 1] = (ଅଙ୍କ_Unused_ଉପର_Bits < integerPartWidth)
                                   ? (~integerPart(0) >> ଅଙ୍କ_Unused_ଉପର_Bits)
                                   : 0;
  ଯଦି (semantics->nonFiniteBehavior == fltNonfiniteBehavior::NanOnly &&
      semantics->nanEncoding == fltNanEncoding::AllOnes &&
      (semantics->precision > 1))
    significand[0] &= ~integerPart(1);
}

/// Make this number the smallest magnitude denormal number in the given
/// semantics.
ଶୂନ୍ୟ IEEEFloat::makeSmallest(ତାର୍କିକ Negative) {
  ଯଦି (Negative && !semantics->hasSignedRepr)
    llvm_unreachable(
        "This floating point format does not support signed values");
  // We want (in interchange format):
  //   sign = {Negative}
  //   exponent = 0..0
  //   significand = 0..01
  category = fcNormal;
  sign = Negative;
  exponent = semantics->minExponent;
  APInt::tcSet(significandParts(), 1, part_ଗଣନା());
}

ଶୂନ୍ୟ IEEEFloat::makeSmallestNormalized(ତାର୍କିକ Negative) {
  ଯଦି (Negative && !semantics->hasSignedRepr)
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

IEEEFloat::IEEEFloat(ଅଚଳ fltSemantics &Sem, ଅଚଳ APInt &API) {
  initFromAPInt(&Sem, API);
}

IEEEFloat::IEEEFloat(ଦଶମିକ f) {
  initFromAPInt(&APFloatBase::semIEEEsingle, APInt::floatToBits(f));
}

IEEEFloat::IEEEFloat(ଦ୍ୱିଗୁଣ d) {
  initFromAPInt(&APFloatBase::semIEEEdouble, APInt::doubleToBits(d));
}

ନାମାକାଶ {
  ଶୂନ୍ୟ append(SmallVectorImpl<ଅକ୍ଷର> &Buffer, StringRef Str) {
    Buffer.append(Str.begin(), Str.end());
  }

  /// Removes data from the given significand until it is no more
  /// precise than is required for the desired precision.
  ଶୂନ୍ୟ AdjustToPrecision(APInt &significand,
                         ପୂର୍ଣ୍ଣସଂଖ୍ୟା &exp, unsigned FormatPrecision) {
    unsigned bits = significand.getActiveBits();

    // 196/59 is a very slight overestimate of lg_2(10).
    unsigned bitsRequired = (FormatPrecision * 196 + 58) / 59;

    ଯଦି (bits <= bitsRequired) ଫେରନ୍ତୁ;

    unsigned tensRemovable = (bits - bitsRequired) * 59 / 196;
    ଯଦି (!tensRemovable) ଫେରନ୍ତୁ;

    exp += tensRemovable;

    APInt ଭାଜକ(significand.getBitWidth(), 1);
    APInt powten(significand.getBitWidth(), 10);
    ଯେତେବେଳେ (ସତ୍ୟ) {
      ଯଦି (tensRemovable & 1)
        ଭାଜକ *= powten;
      tensRemovable >>= 1;
      ଯଦି (!tensRemovable) ଭାଙ୍ଗ;
      powten *= powten;
    }

    significand = significand.udiv(ଭାଜକ);

    // Truncate the significand down to its active bit count.
    significand = significand.trunc(significand.getActiveBits());
  }


  ଶୂନ୍ୟ AdjustToPrecision(SmallVectorImpl<ଅକ୍ଷର> &buffer,
                         ପୂର୍ଣ୍ଣସଂଖ୍ୟା &exp, unsigned FormatPrecision) {
    unsigned N = buffer.size();
    ଯଦି (N <= FormatPrecision) ଫେରନ୍ତୁ;

    // The most significant figures are the last ones in the buffer.
    unsigned FirstSignificant = N - FormatPrecision;

    // Round.
    // FIXME: this probably shouldn't use 'round half up'.

    // Rounding down is just a truncation, except we also want to drop
    // trailing zeros from the new result.
    ଯଦି (buffer[FirstSignificant - 1] < '5') {
      ଯେତେବେଳେ (FirstSignificant < N && buffer[FirstSignificant] == '0')
        FirstSignificant++;

      exp += FirstSignificant;
      buffer.erase(&buffer[0], &buffer[FirstSignificant]);
      ଫେରନ୍ତୁ;
    }

    // Rounding up requires a decimal add-with-carry.  If we continue
    // the carry, the newly-introduced zeros will just be truncated.
    ପାଇଁ (unsigned I = FirstSignificant; I != N; ++I) {
      ଯଦି (buffer[I] == '9') {
        FirstSignificant++;
      } ଅନ୍ୟଥା {
        buffer[I]++;
        ଭାଙ୍ଗ;
      }
    }

    // If we carried through, we have exactly one digit of precision.
    ଯଦି (FirstSignificant == N) {
      exp += FirstSignificant;
      buffer.clear();
      buffer.ଠେଲ_back('1');
      ଫେରନ୍ତୁ;
    }

    exp += FirstSignificant;
    buffer.erase(&buffer[0], &buffer[FirstSignificant]);
  }

  ଶୂନ୍ୟ toStringImpl(SmallVectorImpl<ଅକ୍ଷର> &Str, ଅଚଳ ତାର୍କିକ isNeg, ପୂର୍ଣ୍ଣସଂଖ୍ୟା exp,
                    APInt significand, unsigned FormatPrecision,
                    unsigned FormatMaxPadding, ତାର୍କିକ TruncateZero) {
    ଅଚଳ ପୂର୍ଣ୍ଣସଂଖ୍ୟା semanticsPrecision = significand.getBitWidth();

    ଯଦି (isNeg)
      Str.ଠେଲ_back('-');

    // Set FormatPrecision if zero.  We want to do this before we
    // truncate trailing zeros, as those are part of the precision.
    ଯଦି (!FormatPrecision) {
      // We use enough digits so the number can be round-tripped back to an
      // APFloat. The formula comes from "How to Print Floating-Point Numbers
      // Accurately" by Steele and White.
      // FIXME: Using a formula based purely on the precision is conservative;
      // we can print fewer digits depending on the actual value being printed.

      // FormatPrecision = 2 + floor(significandBits / lg_2(10))
      FormatPrecision = 2 + semanticsPrecision * 59 / 196;
    }

    // Ignore trailing binary zeros.
    ପୂର୍ଣ୍ଣସଂଖ୍ୟା trailingZeros = significand.countr_zero();
    exp += trailingZeros;
    significand.lshrInPlace(trailingZeros);

    // Change the exponent from 2^e to 10^e.
    ଯଦି (exp == 0) {
      // Nothing to do.
    } ଅନ୍ୟଥା ଯଦି (exp > 0) {
      // Just shift left.
      significand = significand.zext(semanticsPrecision + exp);
      significand <<= exp;
      exp = 0;
    } ଅନ୍ୟଥା { /* exp < 0 */
      ପୂର୍ଣ୍ଣସଂଖ୍ୟା texp = -exp;

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
      ଯେତେବେଳେ (ସତ୍ୟ) {
        ଯଦି (texp & 1)
          significand *= five_to_the_i;

        texp >>= 1;
        ଯଦି (!texp)
          ଭାଙ୍ଗ;
        five_to_the_i *= five_to_the_i;
      }
    }

    AdjustToPrecision(significand, exp, FormatPrecision);

    SmallVector<ଅକ୍ଷର, 256> buffer;

    // Fill the buffer.
    unsigned precision = significand.getBitWidth();
    ଯଦି (precision < 4) {
      // We need enough precision to store the value 10.
      precision = 4;
      significand = significand.zext(precision);
    }
    APInt ten(precision, 10);
    APInt digit(precision, 0);

    ତାର୍କିକ inTrail = ସତ୍ୟ;
    ଯେତେବେଳେ (significand != 0) {
      // digit <- significand % 10
      // significand <- significand / 10
      APInt::udivrem(significand, ten, significand, digit);

      unsigned d = digit.get_ZExt_ମୂଲ୍ୟ();

      // Drop trailing zeros.
      ଯଦି (inTrail && !d)
        exp++;
      ଅନ୍ୟଥା {
        buffer.ଠେଲ_back((ଅକ୍ଷର) ('0' + d));
        inTrail = ମିଥ୍ୟା;
      }
    }

    ପ୍ରତିଜ୍ଞା(!buffer.empty() && "no characters in buffer!");

    // Drop down to FormatPrecision.
    // TODO: don't do more precise calculations above than are required.
    AdjustToPrecision(buffer, exp, FormatPrecision);

    unsigned NDigits = buffer.size();

    // Check whether we should use scientific notation.
    ତାର୍କିକ FormatScientific;
    ଯଦି (!FormatMaxPadding) {
      FormatScientific = ସତ୍ୟ;
    } ଅନ୍ୟଥା {
      ଯଦି (exp >= 0) {
        // 765e3 --> 765000
        //              ^^^
        // But we shouldn't make the number look more precise than it is.
        FormatScientific = ((unsigned) exp > FormatMaxPadding ||
                            NDigits + (unsigned) exp > FormatPrecision);
      } ଅନ୍ୟଥା {
        // Power of the most significant digit.
        ପୂର୍ଣ୍ଣସଂଖ୍ୟା MSD = exp + (ପୂର୍ଣ୍ଣସଂଖ୍ୟା) (NDigits - 1);
        ଯଦି (MSD >= 0) {
          // 765e-2 == 7.65
          FormatScientific = ମିଥ୍ୟା;
        } ଅନ୍ୟଥା {
          // 765e-5 == 0.00765
          //           ^ ^^
          FormatScientific = ((unsigned) -MSD) > FormatMaxPadding;
        }
      }
    }

    // Scientific formatting is pretty straightforward.
    ଯଦି (FormatScientific) {
      exp += (NDigits - 1);

      Str.ଠେଲ_back(buffer[NDigits-1]);
      Str.ଠେଲ_back('.');
      ଯଦି (NDigits == 1 && TruncateZero)
        Str.ଠେଲ_back('0');
      ଅନ୍ୟଥା
        ପାଇଁ (unsigned I = 1; I != NDigits; ++I)
          Str.ଠେଲ_back(buffer[NDigits-1-I]);
      // Fill with zeros up to FormatPrecision.
      ଯଦି (!TruncateZero && FormatPrecision > NDigits - 1)
        Str.append(FormatPrecision - NDigits + 1, '0');
      // For !TruncateZero we use lower 'e'.
      Str.ଠେଲ_back(TruncateZero ? 'E' : 'e');

      Str.ଠେଲ_back(exp >= 0 ? '+' : '-');
      ଯଦି (exp < 0)
        exp = -exp;
      SmallVector<ଅକ୍ଷର, 6> expbuf;
      କର {
        expbuf.ଠେଲ_back((ଅକ୍ଷର) ('0' + (exp % 10)));
        exp /= 10;
      } ଯେତେବେଳେ (exp);
      // Exponent always at least two digits if we do not truncate zeros.
      ଯଦି (!TruncateZero && expbuf.size() < 2)
        expbuf.ଠେଲ_back('0');
      ପାଇଁ (unsigned I = 0, E = expbuf.size(); I != E; ++I)
        Str.ଠେଲ_back(expbuf[E-1-I]);
      ଫେରନ୍ତୁ;
    }

    // Non-scientific, positive exponents.
    ଯଦି (exp >= 0) {
      ପାଇଁ (unsigned I = 0; I != NDigits; ++I)
        Str.ଠେଲ_back(buffer[NDigits-1-I]);
      ପାଇଁ (unsigned I = 0; I != (unsigned) exp; ++I)
        Str.ଠେଲ_back('0');
      ଫେରନ୍ତୁ;
    }

    // Non-scientific, negative exponents.

    // The number of digits to the left of the decimal point.
    ପୂର୍ଣ୍ଣସଂଖ୍ୟା NWholeDigits = exp + (ପୂର୍ଣ୍ଣସଂଖ୍ୟା) NDigits;

    unsigned I = 0;
    ଯଦି (NWholeDigits > 0) {
      ପାଇଁ (; I != (unsigned) NWholeDigits; ++I)
        Str.ଠେଲ_back(buffer[NDigits-I-1]);
      Str.ଠେଲ_back('.');
    } ଅନ୍ୟଥା {
      unsigned NZeros = 1 + (unsigned) -NWholeDigits;

      Str.ଠେଲ_back('0');
      Str.ଠେଲ_back('.');
      ପାଇଁ (unsigned Z = 1; Z != NZeros; ++Z)
        Str.ଠେଲ_back('0');
    }

    ପାଇଁ (; I != NDigits; ++I)
      Str.ଠେଲ_back(buffer[NDigits-I-1]);

  }
} // namespace

ଶୂନ୍ୟ IEEEFloat::toString(SmallVectorImpl<ଅକ୍ଷର> &Str, unsigned FormatPrecision,
                         unsigned FormatMaxPadding, ତାର୍କିକ TruncateZero) ଅଚଳ {
  ବାଛ (category) {
  ସ୍ଥିତି fcInfinity:
    ଯଦି (isNegative())
      ଫେରନ୍ତୁ append(Str, "-Inf");
    ଅନ୍ୟଥା
      ଫେରନ୍ତୁ append(Str, "+Inf");

  ସ୍ଥିତି fcNaN: ଫେରନ୍ତୁ append(Str, "NaN");

  ସ୍ଥିତି fcZero:
    ଯଦି (isNegative())
      Str.ଠେଲ_back('-');

    ଯଦି (!FormatMaxPadding) {
      ଯଦି (TruncateZero)
        append(Str, "0.0E+0");
      ଅନ୍ୟଥା {
        append(Str, "0.0");
        ଯଦି (FormatPrecision > 1)
          Str.append(FormatPrecision - 1, '0');
        append(Str, "e+00");
      }
    } ଅନ୍ୟଥା {
      Str.ଠେଲ_back('0');
    }
    ଫେରନ୍ତୁ;

  ସ୍ଥିତି fcNormal:
    ଭାଙ୍ଗ;
  }

  // Decompose the number into an APInt and an exponent.
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା exp = exponent - ((ପୂର୍ଣ୍ଣସଂଖ୍ୟା) semantics->precision - 1);
  APInt significand(
      semantics->precision,
      ArrayRef(significandParts(), part_ଗଣନା_For_Bits(semantics->precision)));

  toStringImpl(Str, isNegative(), exp, significand, FormatPrecision,
               FormatMaxPadding, TruncateZero);

}

ପୂର୍ଣ୍ଣସଂଖ୍ୟା IEEEFloat::getExactLog2Abs() ଅଚଳ {
  ଯଦି (!isFinite() || isZero())
    ଫେରନ୍ତୁ INT_MIN;

  ଅଚଳ integerPart *Parts = significandParts();
  ଅଚଳ ପୂର୍ଣ୍ଣସଂଖ୍ୟା Part_ଗଣନା = part_ଗଣନା_For_Bits(semantics->precision);

  ପୂର୍ଣ୍ଣସଂଖ୍ୟା କାଢ଼_ଗଣନା = 0;
  ପାଇଁ (ପୂର୍ଣ୍ଣସଂଖ୍ୟା i = 0; i < Part_ଗଣନା; ++i) {
    କାଢ଼_ଗଣନା += llvm::popcount(Parts[i]);
    ଯଦି (କାଢ଼_ଗଣନା > 1)
      ଫେରନ୍ତୁ INT_MIN;
  }

  ଯଦି (exponent != semantics->minExponent)
    ଫେରନ୍ତୁ exponent;

  ପୂର୍ଣ୍ଣସଂଖ୍ୟା CountrParts = 0;
  ପାଇଁ (ପୂର୍ଣ୍ଣସଂଖ୍ୟା i = 0; i < Part_ଗଣନା;
       ++i, CountrParts += APInt::APINT_BITS_PER_WORD) {
    ଯଦି (Parts[i] != 0) {
      ଫେରନ୍ତୁ exponent - semantics->precision + CountrParts +
             llvm::countr_zero(Parts[i]) + 1;
    }
  }

  llvm_unreachable("didn't find the set bit");
}

ତାର୍କିକ IEEEFloat::isSignaling() ଅଚଳ {
  ଯଦି (!isNaN())
    ଫେରନ୍ତୁ ମିଥ୍ୟା;
  ଯଦି (semantics->nonFiniteBehavior == fltNonfiniteBehavior::NanOnly ||
      semantics->nonFiniteBehavior == fltNonfiniteBehavior::FiniteOnly)
    ଫେରନ୍ତୁ ମିଥ୍ୟା;

  // IEEE-754R 2008 6.2.1: A signaling NaN bit string should be encoded with the
  // first bit of the trailing significand being 0.
  ଫେରନ୍ତୁ !APInt::tcExtractBit(significandParts(), semantics->precision - 2);
}

/// IEEE-754R 2008 5.3.1: nextUp/nextDown.
///
/// *NOTE* since nextDown(x) = -nextUp(-x), we only implement nextUp with
/// appropriate sign switching before/after the computation.
APFloat::opStatus IEEEFloat::next(ତାର୍କିକ nextDown) {
  // If we are performing nextDown, swap sign so we have -x.
  ଯଦି (nextDown)
    changeSign();

  // Compute nextUp(x)
  opStatus result = opOK;

  // Handle each float category separately.
  ବାଛ (category) {
  ସ୍ଥିତି fcInfinity:
    // nextUp(+inf) = +inf
    ଯଦି (!isNegative())
      ଭାଙ୍ଗ;
    // nextUp(-inf) = -getLargest()
    makeLargest(ସତ୍ୟ);
    ଭାଙ୍ଗ;
  ସ୍ଥିତି fcNaN:
    // IEEE-754R 2008 6.2 Par 2: nextUp(sNaN) = qNaN. Set Invalid flag.
    // IEEE-754R 2008 6.2: nextUp(qNaN) = qNaN. Must be identity so we do not
    //                     change the payload.
    ଯଦି (isSignaling()) {
      result = opInvalidOp;
      // For consistency, propagate the sign of the sNaN to the qNaN.
      makeNaN(ମିଥ୍ୟା, isNegative(), nullptr);
    }
    ଭାଙ୍ଗ;
  ସ୍ଥିତି fcZero:
    // nextUp(pm 0) = +getSmallest()
    makeSmallest(ମିଥ୍ୟା);
    ଭାଙ୍ଗ;
  ସ୍ଥିତି fcNormal:
    // nextUp(-getSmallest()) = -0
    ଯଦି (isSmallest() && isNegative()) {
      APInt::tcSet(significandParts(), 0, part_ଗଣନା());
      category = fcZero;
      exponent = 0;
      ଯଦି (semantics->nanEncoding == fltNanEncoding::NegativeZero)
        sign = ମିଥ୍ୟା;
      ଯଦି (!semantics->hasZero)
        makeSmallestNormalized(ମିଥ୍ୟା);
      ଭାଙ୍ଗ;
    }

    ଯଦି (isLargest() && !isNegative()) {
      ଯଦି (semantics->nonFiniteBehavior == fltNonfiniteBehavior::NanOnly) {
        // nextUp(getLargest()) == NAN
        makeNaN();
        ଭାଙ୍ଗ;
      } ଅନ୍ୟଥା ଯଦି (semantics->nonFiniteBehavior ==
                 fltNonfiniteBehavior::FiniteOnly) {
        // nextUp(getLargest()) == getLargest()
        ଭାଙ୍ଗ;
      } ଅନ୍ୟଥା {
        // nextUp(getLargest()) == INFINITY
        APInt::tcSet(significandParts(), 0, part_ଗଣନା());
        category = fcInfinity;
        exponent = semantics->maxExponent + 1;
        ଭାଙ୍ଗ;
      }
    }

    // nextUp(normal) == normal + inc.
    ଯଦି (isNegative()) {
      // If we are negative, we need to decrement the significand.

      // We only cross a binade boundary that requires adjusting the exponent
      // if:
      //   1. exponent != semantics->minExponent. This implies we are not in the
      //   smallest binade or are dealing with denormals.
      //   2. Our significand excluding the integral bit is all zeros.
      ତାର୍କିକ WillCrossBinadeBoundary =
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
      APInt::tcDecrement(Parts, part_ଗଣନା());

      ଯଦି (WillCrossBinadeBoundary) {
        // Our result is a normal number. Do the following:
        // 1. Set the integral bit to 1.
        // 2. Decrement the exponent.
        APInt::tcSetBit(Parts, semantics->precision - 1);
        exponent--;
      }
    } ଅନ୍ୟଥା {
      // If we are positive, we need to increment the significand.

      // We only cross a binade boundary that requires adjusting the exponent if
      // the input is not a denormal and all of said input's significand bits
      // are set. If all of said conditions are true: clear the significand, set
      // the integral bit to 1, and increment the exponent. If we have a
      // denormal always increment since moving denormals and the numbers in the
      // smallest normal binade have the same exponent in our representation.
      // If there are only exponents, any increment always crosses the
      // BinadeBoundary.
      ତାର୍କିକ WillCrossBinadeBoundary = !APFloat::hasSignificand(*semantics) ||
                                     (!isDenormal() && isSignificandAllOnes());

      ଯଦି (WillCrossBinadeBoundary) {
        integerPart *Parts = significandParts();
        APInt::tcSet(Parts, 0, part_ଗଣନା());
        APInt::tcSetBit(Parts, semantics->precision - 1);
        ପ୍ରତିଜ୍ଞା(exponent != semantics->maxExponent &&
               "We can not increment an exponent beyond the maxExponent allowed"
               " by the given floating point semantics.");
        exponent++;
      } ଅନ୍ୟଥା {
        incrementSignificand();
      }
    }
    ଭାଙ୍ଗ;
  }

  // If we are performing nextDown, swap sign so we have -nextUp(-x)
  ଯଦି (nextDown)
    changeSign();

  ଫେରନ୍ତୁ result;
}

APInt IEEEFloat::getNaNPayload() ଅଚଳ {
  ପ୍ରତିଜ୍ଞା(isNaN() && "Can only be called on NaN values");
  // Number of bits in the payload, excluding the (maybe implied) integer bit.
  unsigned Bits = semantics->precision - 1;
  ଫେରନ୍ତୁ APInt(Bits, ArrayRef(significandParts(), part_ଗଣନା_For_Bits(Bits)));
}

APFloatBase::ExponentType IEEEFloat::exponentNaN() ଅଚଳ {
  ଫେରନ୍ତୁ ::exponentNaN(*semantics);
}

APFloatBase::ExponentType IEEEFloat::exponentInf() ଅଚଳ {
  ଫେରନ୍ତୁ ::exponentInf(*semantics);
}

APFloatBase::ExponentType IEEEFloat::exponentZero() ଅଚଳ {
  ଫେରନ୍ତୁ ::exponentZero(*semantics);
}

ଶୂନ୍ୟ IEEEFloat::makeInf(ତାର୍କିକ Negative) {
  ଯଦି (semantics->nonFiniteBehavior == fltNonfiniteBehavior::FiniteOnly)
    llvm_unreachable("This floating point format does not support Inf");

  ଯଦି (semantics->nonFiniteBehavior == fltNonfiniteBehavior::NanOnly) {
    // There is no Inf, so make NaN instead.
    makeNaN(ମିଥ୍ୟା, Negative);
    ଫେରନ୍ତୁ;
  }
  category = fcInfinity;
  sign = Negative;
  exponent = exponentInf();
  APInt::tcSet(significandParts(), 0, part_ଗଣନା());
}

ଶୂନ୍ୟ IEEEFloat::makeZero(ତାର୍କିକ Negative) {
  ଯଦି (!semantics->hasZero)
    llvm_unreachable("This floating point format does not support Zero");

  category = fcZero;
  sign = Negative;
  ଯଦି (semantics->nanEncoding == fltNanEncoding::NegativeZero) {
    // Merge negative zero to positive because 0b10000...000 is used for NaN
    sign = ମିଥ୍ୟା;
  }
  exponent = exponentZero();
  APInt::tcSet(significandParts(), 0, part_ଗଣନା());
}

ଶୂନ୍ୟ IEEEFloat::makeQuiet() {
  ପ୍ରତିଜ୍ଞା(isNaN());
  ଯଦି (semantics->nonFiniteBehavior != fltNonfiniteBehavior::NanOnly)
    APInt::tcSetBit(significandParts(), semantics->precision - 2);
}

ପୂର୍ଣ୍ଣସଂଖ୍ୟା ilogb(ଅଚଳ IEEEFloat &Arg) {
  ଯଦି (Arg.isNaN())
    ଫେରନ୍ତୁ APFloat::IEK_NaN;
  ଯଦି (Arg.isZero())
    ଫେରନ୍ତୁ APFloat::IEK_Zero;
  ଯଦି (Arg.isInfinity())
    ଫେରନ୍ତୁ APFloat::IEK_Inf;
  ଯଦି (!Arg.isDenormal())
    ଫେରନ୍ତୁ Arg.exponent;

  IEEEFloat Normalized(Arg);
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା SignificandBits = Arg.getSemantics().precision - 1;

  Normalized.exponent += SignificandBits;
  Normalized.normalize(APFloat::rmNearestTiesToEven, lfExactlyZero);
  ଫେରନ୍ତୁ Normalized.exponent - SignificandBits;
}

IEEEFloat scalbn(IEEEFloat X, ପୂର୍ଣ୍ଣସଂଖ୍ୟା Exp, roundingMode RoundingMode) {
  auto MaxExp = X.getSemantics().maxExponent;
  auto MinExp = X.getSemantics().minExponent;

  // If Exp is wildly out-of-scale, simply adding it to X.exponent will
  // overflow; clamp it to a safe range before adding, but ensure that the range
  // is large enough that the clamp does not change the result. The range we
  // need to support is the difference between the largest possible exponent and
  // the normalized exponent of half the smallest denormal.

  ପୂର୍ଣ୍ଣସଂଖ୍ୟା SignificandBits = X.getSemantics().precision - 1;
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା MaxIncrement = MaxExp - (MinExp - SignificandBits) + 1;

  // Clamp to one past the range ends to let normalize handle overlflow.
  X.exponent += std::clamp(Exp, -MaxIncrement - 1, MaxIncrement);
  X.normalize(RoundingMode, lfExactlyZero);
  ଯଦି (X.isNaN())
    X.makeQuiet();
  ଫେରନ୍ତୁ X;
}

IEEEFloat frexp(ଅଚଳ IEEEFloat &Val, ପୂର୍ଣ୍ଣସଂଖ୍ୟା &Exp, roundingMode RM) {
  Exp = ilogb(Val);

  // Quiet signalling nans.
  ଯଦି (Exp == APFloat::IEK_NaN) {
    IEEEFloat Quiet(Val);
    Quiet.makeQuiet();
    ଫେରନ୍ତୁ Quiet;
  }

  ଯଦି (Exp == APFloat::IEK_Inf)
    ଫେରନ୍ତୁ Val;

  // 1 is added because frexp is defined to return a normalized fraction in
  // +/-[0.5, 1.0), rather than the usual +/-[1.0, 2.0).
  Exp = Exp == APFloat::IEK_Zero ? 0 : Exp + 1;
  ଫେରନ୍ତୁ scalbn(Val, -Exp, RM);
}

DoubleAPFloat::DoubleAPFloat(ଅଚଳ fltSemantics &S)
    : Semantics(&S),
      Floats(ନୂତନ APFloat[2]{APFloat(APFloatBase::semIEEEdouble),
                            APFloat(APFloatBase::semIEEEdouble)}) {
  ପ୍ରତିଜ୍ଞା(Semantics == &APFloatBase::semPPCDoubleDouble);
}

DoubleAPFloat::DoubleAPFloat(ଅଚଳ fltSemantics &S, uninitializedTag)
    : Semantics(&S), Floats(ନୂତନ APFloat[2]{
                         APFloat(APFloatBase::semIEEEdouble, uninitialized),
                         APFloat(APFloatBase::semIEEEdouble, uninitialized)}) {
  ପ୍ରତିଜ୍ଞା(Semantics == &APFloatBase::semPPCDoubleDouble);
}

DoubleAPFloat::DoubleAPFloat(ଅଚଳ fltSemantics &S, integerPart I)
    : Semantics(&S),
      Floats(ନୂତନ APFloat[2]{APFloat(APFloatBase::semIEEEdouble, I),
                            APFloat(APFloatBase::semIEEEdouble)}) {
  ପ୍ରତିଜ୍ଞା(Semantics == &APFloatBase::semPPCDoubleDouble);
}

DoubleAPFloat::DoubleAPFloat(ଅଚଳ fltSemantics &S, ଅଚଳ APInt &I)
    : Semantics(&S),
      Floats(ନୂତନ APFloat[2]{
          APFloat(APFloatBase::semIEEEdouble, APInt(64, I.getRawData()[0])),
          APFloat(APFloatBase::semIEEEdouble, APInt(64, I.getRawData()[1]))}) {
  ପ୍ରତିଜ୍ଞା(Semantics == &APFloatBase::semPPCDoubleDouble);
}

DoubleAPFloat::DoubleAPFloat(ଅଚଳ fltSemantics &S, APFloat &&First,
                             APFloat &&Second)
    : Semantics(&S),
      Floats(ନୂତନ APFloat[2]{std::move(First), std::move(Second)}) {
  ପ୍ରତିଜ୍ଞା(Semantics == &APFloatBase::semPPCDoubleDouble);
  ପ୍ରତିଜ୍ଞା(&Floats[0].getSemantics() == &APFloatBase::semIEEEdouble);
  ପ୍ରତିଜ୍ଞା(&Floats[1].getSemantics() == &APFloatBase::semIEEEdouble);
}

DoubleAPFloat::DoubleAPFloat(ଅଚଳ DoubleAPFloat &RHS)
    : Semantics(RHS.Semantics),
      Floats(RHS.Floats ? ନୂତନ APFloat[2]{APFloat(RHS.Floats[0]),
                                         APFloat(RHS.Floats[1])}
                        : nullptr) {
  ପ୍ରତିଜ୍ଞା(Semantics == &APFloatBase::semPPCDoubleDouble);
}

DoubleAPFloat::DoubleAPFloat(DoubleAPFloat &&RHS)
    : Semantics(RHS.Semantics), Floats(RHS.Floats) {
  RHS.Semantics = &APFloatBase::semBogus;
  RHS.Floats = nullptr;
  ପ୍ରତିଜ୍ଞା(Semantics == &APFloatBase::semPPCDoubleDouble);
}

DoubleAPFloat &DoubleAPFloat::operator=(ଅଚଳ DoubleAPFloat &RHS) {
  ଯଦି (Semantics == RHS.Semantics && RHS.Floats) {
    Floats[0] = RHS.Floats[0];
    Floats[1] = RHS.Floats[1];
  } ଅନ୍ୟଥା ଯଦି (ଏହି != &RHS) {
    ଏହି->~DoubleAPFloat();
    ନୂତନ (ଏହି) DoubleAPFloat(RHS);
  }
  ଫେରନ୍ତୁ *ଏହି;
}

// Returns a result such that:
// 1. abs(Lo) <= ulp(Hi)/2
// 2. Hi == RTNE(Hi + Lo)
// 3. Hi + Lo == X + Y
//
// Requires that log2(X) >= log2(Y).
ସ୍ଥିର std::pair<APFloat, APFloat> fastTwoSum(APFloat X, APFloat Y) {
  ଯଦି (!X.isFinite())
    ଫେରନ୍ତୁ {X, APFloat::getZero(X.getSemantics(), /*Negative=*/ମିଥ୍ୟା)};
  APFloat Hi = X + Y;
  APFloat Delta = Hi - X;
  APFloat Lo = Y - Delta;
  ଫେରନ୍ତୁ {Hi, Lo};
}

// Implement addition, subtraction, multiplication and division based on:
// "Software for Doubled-Precision Floating-Point Computations",
// by Seppo Linnainmaa, ACM TOMS vol 7 no 3, September 1981, pages 272-283.
APFloat::opStatus DoubleAPFloat::addImpl(ଅଚଳ APFloat &a, ଅଚଳ APFloat &aa,
                                         ଅଚଳ APFloat &c, ଅଚଳ APFloat &cc,
                                         roundingMode RM) {
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା Status = opOK;
  APFloat z = a;
  Status |= z.add(c, RM);
  ଯଦି (!z.isFinite()) {
    ଯଦି (!z.isInfinity()) {
      Floats[0] = std::move(z);
      Floats[1].makeZero(/* Neg = */ ମିଥ୍ୟା);
      ଫେରନ୍ତୁ (opStatus)Status;
    }
    Status = opOK;
    auto AComparedToC = a.compare_Absolute_ମୂଲ୍ୟ(c);
    z = cc;
    Status |= z.add(aa, RM);
    ଯଦି (AComparedToC == APFloat::cmpGreaterThan) {
      // z = cc + aa + c + a;
      Status |= z.add(c, RM);
      Status |= z.add(a, RM);
    } ଅନ୍ୟଥା {
      // z = cc + aa + a + c;
      Status |= z.add(a, RM);
      Status |= z.add(c, RM);
    }
    ଯଦି (!z.isFinite()) {
      Floats[0] = std::move(z);
      Floats[1].makeZero(/* Neg = */ ମିଥ୍ୟା);
      ଫେରନ୍ତୁ (opStatus)Status;
    }
    Floats[0] = z;
    APFloat zz = aa;
    Status |= zz.add(cc, RM);
    ଯଦି (AComparedToC == APFloat::cmpGreaterThan) {
      // Floats[1] = a - z + c + zz;
      Floats[1] = a;
      Status |= Floats[1].subtract(z, RM);
      Status |= Floats[1].add(c, RM);
      Status |= Floats[1].add(zz, RM);
    } ଅନ୍ୟଥା {
      // Floats[1] = c - z + a + zz;
      Floats[1] = c;
      Status |= Floats[1].subtract(z, RM);
      Status |= Floats[1].add(a, RM);
      Status |= Floats[1].add(zz, RM);
    }
  } ଅନ୍ୟଥା {
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
    ଯଦି (zz.isZero() && !zz.isNegative()) {
      Floats[0] = std::move(z);
      Floats[1].makeZero(/* Neg = */ ମିଥ୍ୟା);
      ଫେରନ୍ତୁ opOK;
    }
    Floats[0] = z;
    Status |= Floats[0].add(zz, RM);
    ଯଦି (!Floats[0].isFinite()) {
      Floats[1].makeZero(/* Neg = */ ମିଥ୍ୟା);
      ଫେରନ୍ତୁ (opStatus)Status;
    }
    Floats[1] = std::move(z);
    Status |= Floats[1].subtract(Floats[0], RM);
    Status |= Floats[1].add(zz, RM);
  }
  ଫେରନ୍ତୁ (opStatus)Status;
}

APFloat::opStatus DoubleAPFloat::addWithSpecial(ଅଚଳ DoubleAPFloat &LHS,
                                                ଅଚଳ DoubleAPFloat &RHS,
                                                DoubleAPFloat &Out,
                                                roundingMode RM) {
  ଯଦି (LHS.getCategory() == fcNaN) {
    Out = LHS;
    ଫେରନ୍ତୁ opOK;
  }
  ଯଦି (RHS.getCategory() == fcNaN) {
    Out = RHS;
    ଫେରନ୍ତୁ opOK;
  }
  ଯଦି (LHS.getCategory() == fcZero) {
    Out = RHS;
    ଫେରନ୍ତୁ opOK;
  }
  ଯଦି (RHS.getCategory() == fcZero) {
    Out = LHS;
    ଫେରନ୍ତୁ opOK;
  }
  ଯଦି (LHS.getCategory() == fcInfinity && RHS.getCategory() == fcInfinity &&
      LHS.isNegative() != RHS.isNegative()) {
    Out.makeNaN(ମିଥ୍ୟା, Out.isNegative(), nullptr);
    ଫେରନ୍ତୁ opInvalidOp;
  }
  ଯଦି (LHS.getCategory() == fcInfinity) {
    Out = LHS;
    ଫେରନ୍ତୁ opOK;
  }
  ଯଦି (RHS.getCategory() == fcInfinity) {
    Out = RHS;
    ଫେରନ୍ତୁ opOK;
  }
  ପ୍ରତିଜ୍ଞା(LHS.getCategory() == fcNormal && RHS.getCategory() == fcNormal);

  APFloat A(LHS.Floats[0]), AA(LHS.Floats[1]), C(RHS.Floats[0]),
      CC(RHS.Floats[1]);
  ପ୍ରତିଜ୍ଞା(&A.getSemantics() == &APFloatBase::semIEEEdouble);
  ପ୍ରତିଜ୍ଞା(&AA.getSemantics() == &APFloatBase::semIEEEdouble);
  ପ୍ରତିଜ୍ଞା(&C.getSemantics() == &APFloatBase::semIEEEdouble);
  ପ୍ରତିଜ୍ଞା(&CC.getSemantics() == &APFloatBase::semIEEEdouble);
  ପ୍ରତିଜ୍ଞା(&Out.Floats[0].getSemantics() == &APFloatBase::semIEEEdouble);
  ପ୍ରତିଜ୍ଞା(&Out.Floats[1].getSemantics() == &APFloatBase::semIEEEdouble);
  ଫେରନ୍ତୁ Out.addImpl(A, AA, C, CC, RM);
}

APFloat::opStatus DoubleAPFloat::add(ଅଚଳ DoubleAPFloat &RHS,
                                     roundingMode RM) {
  ଫେରନ୍ତୁ addWithSpecial(*ଏହି, RHS, *ଏହି, RM);
}

APFloat::opStatus DoubleAPFloat::subtract(ଅଚଳ DoubleAPFloat &RHS,
                                          roundingMode RM) {
  changeSign();
  auto Ret = add(RHS, RM);
  changeSign();
  ଫେରନ୍ତୁ Ret;
}

APFloat::opStatus DoubleAPFloat::multiply(ଅଚଳ DoubleAPFloat &RHS,
                                          APFloat::roundingMode RM) {
  ଅଚଳ auto &LHS = *ଏହି;
  auto &Out = *ଏହି;
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
  ଯଦି (LHS.getCategory() == fcNaN) {
    Out = LHS;
    ଫେରନ୍ତୁ opOK;
  }
  ଯଦି (RHS.getCategory() == fcNaN) {
    Out = RHS;
    ଫେରନ୍ତୁ opOK;
  }
  ଯଦି ((LHS.getCategory() == fcZero && RHS.getCategory() == fcInfinity) ||
      (LHS.getCategory() == fcInfinity && RHS.getCategory() == fcZero)) {
    Out.makeNaN(ମିଥ୍ୟା, ମିଥ୍ୟା, nullptr);
    ଫେରନ୍ତୁ opOK;
  }
  ଯଦି (LHS.getCategory() == fcZero || LHS.getCategory() == fcInfinity) {
    Out = LHS;
    ଫେରନ୍ତୁ opOK;
  }
  ଯଦି (RHS.getCategory() == fcZero || RHS.getCategory() == fcInfinity) {
    Out = RHS;
    ଫେରନ୍ତୁ opOK;
  }
  ପ୍ରତିଜ୍ଞା(LHS.getCategory() == fcNormal && RHS.getCategory() == fcNormal &&
         "Special cases not handled exhaustively");

  ପୂର୍ଣ୍ଣସଂଖ୍ୟା Status = opOK;
  APFloat A = Floats[0], B = Floats[1], C = RHS.Floats[0], D = RHS.Floats[1];
  // t = a * c
  APFloat T = A;
  Status |= T.multiply(C, RM);
  ଯଦି (!T.isFiniteNonZero()) {
    Floats[0] = std::move(T);
    Floats[1].makeZero(/* Neg = */ ମିଥ୍ୟା);
    ଫେରନ୍ତୁ (opStatus)Status;
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
  ଯଦି (!U.isFinite()) {
    Floats[1].makeZero(/* Neg = */ ମିଥ୍ୟା);
  } ଅନ୍ୟଥା {
    // Floats[1] = (t - u) + tau
    Status |= T.subtract(U, RM);
    Status |= T.add(Tau, RM);
    Floats[1] = std::move(T);
  }
  ଫେରନ୍ତୁ (opStatus)Status;
}

APFloat::opStatus DoubleAPFloat::divide(ଅଚଳ DoubleAPFloat &RHS,
                                        APFloat::roundingMode RM) {
  ପ୍ରତିଜ୍ଞା(Semantics == &APFloatBase::semPPCDoubleDouble &&
         "Unexpected Semantics");
  APFloat Tmp(APFloatBase::semPPCDoubleDoubleLegacy, bitcastToAPInt());
  auto Ret = Tmp.divide(
      APFloat(APFloatBase::semPPCDoubleDoubleLegacy, RHS.bitcastToAPInt()), RM);
  *ଏହି = DoubleAPFloat(APFloatBase::semPPCDoubleDouble, Tmp.bitcastToAPInt());
  ଫେରନ୍ତୁ Ret;
}

APFloat::opStatus DoubleAPFloat::remainder(ଅଚଳ DoubleAPFloat &RHS) {
  ପ୍ରତିଜ୍ଞା(Semantics == &APFloatBase::semPPCDoubleDouble &&
         "Unexpected Semantics");
  APFloat Tmp(APFloatBase::semPPCDoubleDoubleLegacy, bitcastToAPInt());
  auto Ret = Tmp.remainder(
      APFloat(APFloatBase::semPPCDoubleDoubleLegacy, RHS.bitcastToAPInt()));
  *ଏହି = DoubleAPFloat(APFloatBase::semPPCDoubleDouble, Tmp.bitcastToAPInt());
  ଫେରନ୍ତୁ Ret;
}

APFloat::opStatus DoubleAPFloat::mod(ଅଚଳ DoubleAPFloat &RHS) {
  ପ୍ରତିଜ୍ଞା(Semantics == &APFloatBase::semPPCDoubleDouble &&
         "Unexpected Semantics");
  APFloat Tmp(APFloatBase::semPPCDoubleDoubleLegacy, bitcastToAPInt());
  auto Ret = Tmp.mod(
      APFloat(APFloatBase::semPPCDoubleDoubleLegacy, RHS.bitcastToAPInt()));
  *ଏହି = DoubleAPFloat(APFloatBase::semPPCDoubleDouble, Tmp.bitcastToAPInt());
  ଫେରନ୍ତୁ Ret;
}

APFloat::opStatus
DoubleAPFloat::fusedMultiplyAdd(ଅଚଳ DoubleAPFloat &Multiplicand,
                                ଅଚଳ DoubleAPFloat &Addend,
                                APFloat::roundingMode RM) {
  ପ୍ରତିଜ୍ଞା(Semantics == &APFloatBase::semPPCDoubleDouble &&
         "Unexpected Semantics");
  APFloat Tmp(APFloatBase::semPPCDoubleDoubleLegacy, bitcastToAPInt());
  auto Ret = Tmp.fusedMultiplyAdd(
      APFloat(APFloatBase::semPPCDoubleDoubleLegacy,
              Multiplicand.bitcastToAPInt()),
      APFloat(APFloatBase::semPPCDoubleDoubleLegacy, Addend.bitcastToAPInt()),
      RM);
  *ଏହି = DoubleAPFloat(APFloatBase::semPPCDoubleDouble, Tmp.bitcastToAPInt());
  ଫେରନ୍ତୁ Ret;
}

APFloat::opStatus DoubleAPFloat::roundToIntegral(APFloat::roundingMode RM) {
  ପ୍ରତିଜ୍ଞା(Semantics == &APFloatBase::semPPCDoubleDouble &&
         "Unexpected Semantics");
  ଅଚଳ APFloat &Hi = getFirst();
  ଅଚଳ APFloat &Lo = getSecond();

  APFloat RoundedHi = Hi;
  ଅଚଳ opStatus HiStatus = RoundedHi.roundToIntegral(RM);

  // We can reduce the problem to just the high part if the input:
  // 1. Represents a non-finite value.
  // 2. Has a component which is zero.
  ଯଦି (!Hi.isFiniteNonZero() || Lo.isZero()) {
    Floats[0] = std::move(RoundedHi);
    Floats[1].makeZero(/*Neg=*/ମିଥ୍ୟା);
    ଫେରନ୍ତୁ HiStatus;
  }

  // Adjust `Rounded` in the direction of `TieBreaker` if `ToRound` was at a
  // halfway point.
  auto RoundToNearestHelper = [](APFloat ToRound, APFloat Rounded,
                                 APFloat TieBreaker) {
    // RoundingError tells us which direction we rounded:
    //   - RoundingError > 0: we rounded up.
    //   - RoundingError < 0: we rounded down.
    // Sterbenz' lemma ensures that RoundingError is exact.
    ଅଚଳ APFloat RoundingError = Rounded - ToRound;
    ଯଦି (TieBreaker.isNonZero() &&
        TieBreaker.isNegative() != RoundingError.isNegative() &&
        abs(RoundingError).is_Exactly_ମୂଲ୍ୟ(0.5))
      Rounded.add(
          APFloat::getOne(Rounded.getSemantics(), TieBreaker.isNegative()),
          rmNearestTiesToEven);
    ଫେରନ୍ତୁ Rounded;
  };

  // Case 1: Hi is not an integer.
  // Special cases are for rounding modes that are sensitive to ties.
  ଯଦି (RoundedHi != Hi) {
    // We need to consider the case where Hi was between two integers and the
    // rounding mode broke the tie when, in fact, Lo may have had a different
    // sign than Hi.
    ଯଦି (RM == rmNearestTiesToAway || RM == rmNearestTiesToEven)
      RoundedHi = RoundToNearestHelper(Hi, RoundedHi, Lo);

    Floats[0] = std::move(RoundedHi);
    Floats[1].makeZero(/*Neg=*/ମିଥ୍ୟା);
    ଫେରନ୍ତୁ HiStatus;
  }

  // Case 2: Hi is an integer.
  // Special cases are for rounding modes which are rounding towards or away from zero.
  RoundingMode LoRoundingMode;
  ଯଦି (RM == rmTowardZero)
    // When our input is positive, we want the Lo component rounded toward
    // negative infinity to get the smallest result magnitude. Likewise,
    // negative inputs want the Lo component rounded toward positive infinity.
    LoRoundingMode = isNegative() ? rmTowardPositive : rmTowardNegative;
  ଅନ୍ୟଥା
    LoRoundingMode = RM;

  APFloat RoundedLo = Lo;
  ଅଚଳ opStatus LoStatus = RoundedLo.roundToIntegral(LoRoundingMode);
  ଯଦି (LoRoundingMode == rmNearestTiesToAway)
    // We need to consider the case where Lo was between two integers and the
    // rounding mode broke the tie when, in fact, Hi may have had a different
    // sign than Lo.
    RoundedLo = RoundToNearestHelper(Lo, RoundedLo, Hi);

  // We must ensure that the final result has no overlap between the two APFloat values.
  std::tie(RoundedHi, RoundedLo) = fastTwoSum(RoundedHi, RoundedLo);

  Floats[0] = std::move(RoundedHi);
  Floats[1] = std::move(RoundedLo);
  ଫେରନ୍ତୁ LoStatus;
}

ଶୂନ୍ୟ DoubleAPFloat::changeSign() {
  Floats[0].changeSign();
  Floats[1].changeSign();
}

APFloat::cmpResult
DoubleAPFloat::compare_Absolute_ମୂଲ୍ୟ(ଅଚଳ DoubleAPFloat &RHS) ଅଚଳ {
  // Compare absolute values of the high parts.
  ଅଚଳ cmpResult HiPartCmp = Floats[0].compare_Absolute_ମୂଲ୍ୟ(RHS.Floats[0]);
  ଯଦି (HiPartCmp != cmpEqual)
    ଫେରନ୍ତୁ HiPartCmp;

  // Zero, regardless of sign, is equal.
  ଯଦି (Floats[1].isZero() && RHS.Floats[1].isZero())
    ଫେରନ୍ତୁ cmpEqual;

  // At this point, |this->Hi| == |RHS.Hi|.
  // The magnitude is |Hi+Lo| which is Hi+|Lo| if signs of Hi and Lo are the
  // same, and Hi-|Lo| if signs are different.
  ଅଚଳ ତାର୍କିକ ThisIsSubtractive =
      Floats[0].isNegative() != Floats[1].isNegative();
  ଅଚଳ ତାର୍କିକ RHSIsSubtractive =
      RHS.Floats[0].isNegative() != RHS.Floats[1].isNegative();

  // Case 1: The low part of 'this' is zero.
  ଯଦି (Floats[1].isZero())
    // We are comparing |Hi| vs. |Hi| ± |RHS.Lo|.
    // If RHS is subtractive, its magnitude is smaller.
    // If RHS is additive, its magnitude is larger.
    ଫେରନ୍ତୁ RHSIsSubtractive ? cmpGreaterThan : cmpLessThan;

  // Case 2: The low part of 'RHS' is zero (and we know 'this' is not).
  ଯଦି (RHS.Floats[1].isZero())
    // We are comparing |Hi| ± |This.Lo| vs. |Hi|.
    // If 'this' is subtractive, its magnitude is smaller.
    // If 'this' is additive, its magnitude is larger.
    ଫେରନ୍ତୁ ThisIsSubtractive ? cmpLessThan : cmpGreaterThan;

  // If their natures differ, the additive one is larger.
  ଯଦି (ThisIsSubtractive != RHSIsSubtractive)
    ଫେରନ୍ତୁ ThisIsSubtractive ? cmpLessThan : cmpGreaterThan;

  // Case 3: Both are additive (Hi+|Lo|) or both are subtractive (Hi-|Lo|).
  // The comparison now depends on the magnitude of the low parts.
  ଅଚଳ cmpResult LoPartCmp = Floats[1].compare_Absolute_ମୂଲ୍ୟ(RHS.Floats[1]);

  ଯଦି (ThisIsSubtractive) {
    // Both are subtractive (Hi-|Lo|), so the comparison of |Lo| is inverted.
    ଯଦି (LoPartCmp == cmpLessThan)
      ଫେରନ୍ତୁ cmpGreaterThan;
    ଯଦି (LoPartCmp == cmpGreaterThan)
      ଫେରନ୍ତୁ cmpLessThan;
  }

  // If additive, the comparison of |Lo| is direct.
  // If equal, they are equal.
  ଫେରନ୍ତୁ LoPartCmp;
}

APFloat::fltCategory DoubleAPFloat::getCategory() ଅଚଳ {
  ଫେରନ୍ତୁ Floats[0].getCategory();
}

ତାର୍କିକ DoubleAPFloat::isNegative() ଅଚଳ { ଫେରନ୍ତୁ Floats[0].isNegative(); }

ଶୂନ୍ୟ DoubleAPFloat::makeInf(ତାର୍କିକ Neg) {
  Floats[0].makeInf(Neg);
  Floats[1].makeZero(/* Neg = */ ମିଥ୍ୟା);
}

ଶୂନ୍ୟ DoubleAPFloat::makeZero(ତାର୍କିକ Neg) {
  Floats[0].makeZero(Neg);
  Floats[1].makeZero(/* Neg = */ ମିଥ୍ୟା);
}

ଶୂନ୍ୟ DoubleAPFloat::makeLargest(ତାର୍କିକ Neg) {
  ପ୍ରତିଜ୍ଞା(Semantics == &APFloatBase::semPPCDoubleDouble &&
         "Unexpected Semantics");
  Floats[0] =
      APFloat(APFloatBase::semIEEEdouble, APInt(64, 0x7fefffffffffffffull));
  Floats[1] =
      APFloat(APFloatBase::semIEEEdouble, APInt(64, 0x7c8ffffffffffffeull));
  ଯଦି (Neg)
    changeSign();
}

ଶୂନ୍ୟ DoubleAPFloat::makeSmallest(ତାର୍କିକ Neg) {
  ପ୍ରତିଜ୍ଞା(Semantics == &APFloatBase::semPPCDoubleDouble &&
         "Unexpected Semantics");
  Floats[0].makeSmallest(Neg);
  Floats[1].makeZero(/* Neg = */ ମିଥ୍ୟା);
}

ଶୂନ୍ୟ DoubleAPFloat::makeSmallestNormalized(ତାର୍କିକ Neg) {
  ପ୍ରତିଜ୍ଞା(Semantics == &APFloatBase::semPPCDoubleDouble &&
         "Unexpected Semantics");
  Floats[0] =
      APFloat(APFloatBase::semIEEEdouble, APInt(64, 0x0360000000000000ull));
  ଯଦି (Neg)
    Floats[0].changeSign();
  Floats[1].makeZero(/* Neg = */ ମିଥ୍ୟା);
}

ଶୂନ୍ୟ DoubleAPFloat::makeNaN(ତାର୍କିକ SNaN, ତାର୍କିକ Neg, ଅଚଳ APInt *fill) {
  Floats[0].makeNaN(SNaN, Neg, fill);
  Floats[1].makeZero(/* Neg = */ ମିଥ୍ୟା);
}

APFloat::cmpResult DoubleAPFloat::compare(ଅଚଳ DoubleAPFloat &RHS) ଅଚଳ {
  auto Result = Floats[0].compare(RHS.Floats[0]);
  // |Float[0]| > |Float[1]|
  ଯଦି (Result == APFloat::cmpEqual)
    ଫେରନ୍ତୁ Floats[1].compare(RHS.Floats[1]);
  ଫେରନ୍ତୁ Result;
}

ତାର୍କିକ DoubleAPFloat::bitwiseIsEqual(ଅଚଳ DoubleAPFloat &RHS) ଅଚଳ {
  ଫେରନ୍ତୁ Floats[0].bitwiseIsEqual(RHS.Floats[0]) &&
         Floats[1].bitwiseIsEqual(RHS.Floats[1]);
}

hash_code hash_ମୂଲ୍ୟ(ଅଚଳ DoubleAPFloat &Arg) {
  ଯଦି (Arg.Floats)
    ଫେରନ୍ତୁ hash_combine(hash_ମୂଲ୍ୟ(Arg.Floats[0]), hash_ମୂଲ୍ୟ(Arg.Floats[1]));
  ଫେରନ୍ତୁ hash_combine(Arg.Semantics);
}

APInt DoubleAPFloat::bitcastToAPInt() ଅଚଳ {
  ପ୍ରତିଜ୍ଞା(Semantics == &APFloatBase::semPPCDoubleDouble &&
         "Unexpected Semantics");
  uint64_t Data[] = {
      Floats[0].bitcastToAPInt().getRawData()[0],
      Floats[1].bitcastToAPInt().getRawData()[0],
  };
  ଫେରନ୍ତୁ APInt(128, Data);
}

Expected<APFloat::opStatus> DoubleAPFloat::convertFromString(StringRef S,
                                                             roundingMode RM) {
  ପ୍ରତିଜ୍ଞା(Semantics == &APFloatBase::semPPCDoubleDouble &&
         "Unexpected Semantics");
  APFloat Tmp(APFloatBase::semPPCDoubleDoubleLegacy);
  auto Ret = Tmp.convertFromString(S, RM);
  *ଏହି = DoubleAPFloat(APFloatBase::semPPCDoubleDouble, Tmp.bitcastToAPInt());
  ଫେରନ୍ତୁ Ret;
}

// The double-double lattice of values corresponds to numbers which obey:
// - abs(lo) <= 1/2 * ulp(hi)
// - roundTiesToEven(hi + lo) == hi
//
// nextUp must choose the smallest output > input that follows these rules.
// nexDown must choose the largest output < input that follows these rules.
APFloat::opStatus DoubleAPFloat::next(ତାର୍କିକ nextDown) {
  ପ୍ରତିଜ୍ଞା(Semantics == &APFloatBase::semPPCDoubleDouble &&
         "Unexpected Semantics");
  // nextDown(x) = -nextUp(-x)
  ଯଦି (nextDown) {
    changeSign();
    APFloat::opStatus Result = next(/*nextDown=*/ମିଥ୍ୟା);
    changeSign();
    ଫେରନ୍ତୁ Result;
  }
  ବାଛ (getCategory()) {
  ସ୍ଥିତି fcInfinity:
    // nextUp(+inf) = +inf
    // nextUp(-inf) = -getLargest()
    ଯଦି (isNegative())
      makeLargest(ସତ୍ୟ);
    ଫେରନ୍ତୁ opOK;

  ସ୍ଥିତି fcNaN:
    // IEEE-754R 2008 6.2 Par 2: nextUp(sNaN) = qNaN. Set Invalid flag.
    // IEEE-754R 2008 6.2: nextUp(qNaN) = qNaN. Must be identity so we do not
    //                     change the payload.
    ଯଦି (getFirst().isSignaling()) {
      // For consistency, propagate the sign of the sNaN to the qNaN.
      makeNaN(ମିଥ୍ୟା, isNegative(), nullptr);
      ଫେରନ୍ତୁ opInvalidOp;
    }
    ଫେରନ୍ତୁ opOK;

  ସ୍ଥିତି fcZero:
    // nextUp(pm 0) = +getSmallest()
    makeSmallest(ମିଥ୍ୟା);
    ଫେରନ୍ତୁ opOK;

  ସ୍ଥିତି fcNormal:
    ଭାଙ୍ଗ;
  }

  ଅଚଳ APFloat &HiOld = getFirst();
  ଅଚଳ APFloat &LoOld = getSecond();

  APFloat NextLo = LoOld;
  NextLo.next(/*nextDown=*/ମିଥ୍ୟା);

  // We want to admit values where:
  // 1. abs(Lo) <= ulp(Hi)/2
  // 2. Hi == RTNE(Hi + lo)
  auto InLattice = [](ଅଚଳ APFloat &Hi, ଅଚଳ APFloat &Lo) {
    ଫେରନ୍ତୁ Hi + Lo == Hi;
  };

  // Check if (HiOld, nextUp(LoOld) is in the lattice.
  ଯଦି (InLattice(HiOld, NextLo)) {
    // Yes, the result is (HiOld, nextUp(LoOld)).
    Floats[1] = std::move(NextLo);

    // TODO: Because we currently rely on semPPCDoubleDoubleLegacy, our maximum
    // value is defined to have exactly 106 bits of precision. This limitation
    // results in semPPCDoubleDouble being unable to reach its maximum canonical
    // value.
    DoubleAPFloat Largest{*Semantics, uninitialized};
    Largest.makeLargest(/*Neg=*/ମିଥ୍ୟା);
    ଯଦି (compare(Largest) == cmpGreaterThan)
      makeInf(/*Neg=*/ମିଥ୍ୟା);

    ଫେରନ୍ତୁ opOK;
  }

  // Now we need to handle the cases where (HiOld, nextUp(LoOld)) is not the
  // correct result. We know the new hi component will be nextUp(HiOld) but our
  // lattice rules make it a little ambiguous what the correct NextLo must be.
  APFloat NextHi = HiOld;
  NextHi.next(/*nextDown=*/ମିଥ୍ୟା);

  // nextUp(getLargest()) == INFINITY
  ଯଦି (NextHi.isInfinity()) {
    makeInf(/*Neg=*/ମିଥ୍ୟା);
    ଫେରନ୍ତୁ opOK;
  }

  // IEEE 754-2019 5.3.1:
  // "If x is the negative number of least magnitude in x's format, nextUp(x) is
  // -0."
  ଯଦି (NextHi.isZero()) {
    makeZero(/*Neg=*/ସତ୍ୟ);
    ଫେରନ୍ତୁ opOK;
  }

  // abs(NextLo) must be <= ulp(NextHi)/2. We want NextLo to be as close to
  // negative infinity as possible.
  NextLo = neg(scalbn(harrisonUlp(NextHi), -1, rmTowardZero));
  ଯଦି (!InLattice(NextHi, NextLo))
    // RTNE may mean that Lo must be < ulp(NextHi) / 2 so we bump NextLo.
    NextLo.next(/*nextDown=*/ମିଥ୍ୟା);

  Floats[0] = std::move(NextHi);
  Floats[1] = std::move(NextLo);

  ଫେରନ୍ତୁ opOK;
}

APFloat::opStatus DoubleAPFloat::convertToSignExtendedInteger(
    MutableArrayRef<integerPart> ନିବେଶ, unsigned ପୂର୍ଣ୍ଣସଂଖ୍ୟା Width, ତାର୍କିକ IsSigned,
    roundingMode RM, ତାର୍କିକ *IsExact) ଅଚଳ {
  ପ୍ରତିଜ୍ଞା(Semantics == &APFloatBase::semPPCDoubleDouble &&
         "Unexpected Semantics");

  // If Hi is not finite, or Lo is zero, the value is entirely represented
  // by Hi. Delegate to the simpler single-APFloat conversion.
  ଯଦି (!getFirst().isFiniteNonZero() || getSecond().isZero())
    ଫେରନ୍ତୁ getFirst().convertToInteger(ନିବେଶ, Width, IsSigned, RM, IsExact);

  // First, round the full double-double value to an integral value. This
  // simplifies the rest of the function, as we no longer need to consider
  // fractional parts.
  *IsExact = ମିଥ୍ୟା;
  DoubleAPFloat Integral = *ଏହି;
  ଅଚଳ opStatus RoundStatus = Integral.roundToIntegral(RM);
  ଯଦି (RoundStatus == opInvalidOp)
    ଫେରନ୍ତୁ opInvalidOp;
  ଅଚଳ APFloat &IntegralHi = Integral.getFirst();
  ଅଚଳ APFloat &IntegralLo = Integral.getSecond();

  // If rounding results in either component being zero, the sum is trivial.
  // Delegate to the simpler single-APFloat conversion.
  ତାର୍କିକ HiIsExact;
  ଯଦି (IntegralHi.isZero() || IntegralLo.isZero()) {
    ଅଚଳ opStatus HiStatus =
        IntegralHi.convertToInteger(ନିବେଶ, Width, IsSigned, RM, &HiIsExact);
    // The conversion from an integer-valued float to an APInt may fail if the
    // result would be out of range.  Regardless, taking this path is only
    // possible if rounding occurred during the initial `roundToIntegral`.
    ଫେରନ୍ତୁ HiStatus == opOK ? opInexact : HiStatus;
  }

  // A negative number cannot be represented by an unsigned integer.
  // Since a double-double is canonical, if Hi is negative, the sum is negative.
  ଯଦି (!IsSigned && IntegralHi.isNegative())
    ଫେରନ୍ତୁ opInvalidOp;

  // Handle the special boundary case where |Hi| is exactly the power of two
  // that marks the edge of the integer's range (e.g., 2^63 for int64_t). In
  // this situation, Hi itself won't fit, but the sum Hi + Lo might.
  // `PositiveOverflowWidth` is the bit number for this boundary (N-1 for
  // signed, N for unsigned).
  ତାର୍କିକ LoIsExact;
  ଅଚଳ ପୂର୍ଣ୍ଣସଂଖ୍ୟା HiExactLog2 = IntegralHi.getExactLog2Abs();
  ଅଚଳ unsigned PositiveOverflowWidth = IsSigned ? Width - 1 : Width;
  ଯଦି (HiExactLog2 >= 0 &&
      static_cast<unsigned>(HiExactLog2) == PositiveOverflowWidth) {
    // If Hi and Lo have the same sign, |Hi + Lo| > |Hi|, so the sum is
    // guaranteed to overflow. E.g., for uint128_t, (2^128, 1) overflows.
    ଯଦି (IntegralHi.isNegative() == IntegralLo.isNegative())
      ଫେରନ୍ତୁ opInvalidOp;

    // If the signs differ, the sum will fit. We can compute the result using
    // properties of two's complement arithmetic without a wide intermediate
    // integer. E.g., for uint128_t, (2^128, -1) should be 2^128 - 1.
    ଅଚଳ opStatus LoStatus = IntegralLo.convertToInteger(
        ନିବେଶ, Width, /*IsSigned=*/ସତ୍ୟ, RM, &LoIsExact);
    ଯଦି (LoStatus == opInvalidOp)
      ଫେରନ୍ତୁ opInvalidOp;

    // Adjust the bit pattern of Lo to account for Hi's value:
    //  - For unsigned (Hi=2^Width): `2^Width + Lo` in `Width`-bit
    //    arithmetic is equivalent to just `Lo`. The conversion of `Lo` above
    //    already produced the correct final bit pattern.
    //  - For signed (Hi=2^(Width-1)): The sum `2^(Width-1) + Lo` (where Lo<0)
    //    can be computed by taking the two's complement pattern for `Lo` and
    //    clearing the sign bit.
    ଯଦି (IsSigned && !IntegralHi.isNegative())
      APInt::tcClearBit(ନିବେଶ.data(), PositiveOverflowWidth);
    *IsExact = RoundStatus == opOK;
    ଫେରନ୍ତୁ RoundStatus;
  }

  // Convert Hi into an integer.  This may not fit but that is OK: we know that
  // Hi + Lo would not fit either in this situation.
  ଅଚଳ opStatus HiStatus = IntegralHi.convertToInteger(
      ନିବେଶ, Width, IsSigned, rmTowardZero, &HiIsExact);
  ଯଦି (HiStatus == opInvalidOp)
    ଫେରନ୍ତୁ HiStatus;

  // Convert Lo into a temporary integer of the same width.
  APSInt LoResult{Width, /*isUnsigned=*/!IsSigned};
  ଅଚଳ opStatus LoStatus =
      IntegralLo.convertToInteger(LoResult, rmTowardZero, &LoIsExact);
  ଯଦି (LoStatus == opInvalidOp)
    ଫେରନ୍ତୁ LoStatus;

  // Add Lo to Hi. This addition is guaranteed not to overflow because of the
  // double-double canonicalization rule (`|Lo| <= ulp(Hi)/2`). The only case
  // where the sum could cross the integer type's boundary is when Hi is a
  // power of two, which is handled by the special case block above.
  APInt::tcAdd(ନିବେଶ.data(), LoResult.getRawData(), /*carry=*/0, ନିବେଶ.size());

  *IsExact = RoundStatus == opOK;
  ଫେରନ୍ତୁ RoundStatus;
}

APFloat::opStatus
DoubleAPFloat::convertToInteger(MutableArrayRef<integerPart> ନିବେଶ,
                                unsigned ପୂର୍ଣ୍ଣସଂଖ୍ୟା Width, ତାର୍କିକ IsSigned,
                                roundingMode RM, ତାର୍କିକ *IsExact) ଅଚଳ {
  opStatus FS =
      convertToSignExtendedInteger(ନିବେଶ, Width, IsSigned, RM, IsExact);

  ଯଦି (FS == opInvalidOp) {
    ଅଚଳ unsigned Dst_Parts_ଗଣନା = part_ଗଣନା_For_Bits(Width);
    ପ୍ରତିଜ୍ଞା(Dst_Parts_ଗଣନା <= ନିବେଶ.size() && "Integer too big");

    unsigned Bits;
    ଯଦି (getCategory() == fcNaN)
      Bits = 0;
    ଅନ୍ୟଥା ଯଦି (isNegative())
      Bits = IsSigned;
    ଅନ୍ୟଥା
      Bits = Width - IsSigned;

    tcSetLeastSignificantBits(ନିବେଶ.data(), Dst_Parts_ଗଣନା, Bits);
    ଯଦି (isNegative() && IsSigned)
      APInt::tc_Shift_ବାମ(ନିବେଶ.data(), Dst_Parts_ଗଣନା, Width - 1);
  }

  ଫେରନ୍ତୁ FS;
}

APFloat::opStatus DoubleAPFloat::handleOverflow(roundingMode RM) {
  ବାଛ (RM) {
  ସ୍ଥିତି APFloat::rmTowardZero:
    makeLargest(/*Neg=*/isNegative());
    ଭାଙ୍ଗ;
  ସ୍ଥିତି APFloat::rmTowardNegative:
    ଯଦି (isNegative())
      makeInf(/*Neg=*/ସତ୍ୟ);
    ଅନ୍ୟଥା
      makeLargest(/*Neg=*/ମିଥ୍ୟା);
    ଭାଙ୍ଗ;
  ସ୍ଥିତି APFloat::rmTowardPositive:
    ଯଦି (isNegative())
      makeLargest(/*Neg=*/ସତ୍ୟ);
    ଅନ୍ୟଥା
      makeInf(/*Neg=*/ମିଥ୍ୟା);
    ଭାଙ୍ଗ;
  ସ୍ଥିତି APFloat::rmNearestTiesToAway:
  ସ୍ଥିତି APFloat::rmNearestTiesToEven:
    makeInf(/*Neg=*/isNegative());
    ଭାଙ୍ଗ;
  ପୂର୍ବନିର୍ଦ୍ଧାରିତ:
    llvm_unreachable("Invalid rounding mode found");
  }
  opStatus S = opInexact;
  ଯଦି (!getFirst().isFinite())
    S = static_cast<opStatus>(S | opOverflow);
  ଫେରନ୍ତୁ S;
}

APFloat::opStatus DoubleAPFloat::convertFromUnsignedParts(
    ଅଚଳ integerPart *Src, unsigned ପୂର୍ଣ୍ଣସଂଖ୍ୟା Src_ଗଣନା, roundingMode RM) {
  // Find the most significant bit of the source integer. APInt::tcMSB returns
  // UINT_MAX for a zero value.
  ଅଚଳ unsigned SrcMSB = APInt::tcMSB(Src, Src_ଗଣନା);
  ଯଦି (SrcMSB == UINT_MAX) {
    // The source integer is 0.
    makeZero(/*Neg=*/ମିଥ୍ୟା);
    ଫେରନ୍ତୁ opOK;
  }

  // Create a minimally-sized APInt to represent the source value.
  ଅଚଳ unsigned SrcBitWidth = SrcMSB + 1;
  APSInt SrcInt{APInt{/*numBits=*/SrcBitWidth, ArrayRef(Src, Src_ଗଣନା)},
                /*isUnsigned=*/ସତ୍ୟ};

  // Stage 1: Initial Approximation.
  // Convert the source integer SrcInt to the Hi part of the DoubleAPFloat.
  // We use round-to-nearest because it minimizes the initial error, which is
  // crucial for the subsequent steps.
  APFloat Hi{getFirst().getSemantics()};
  Hi.convertFromAPInt(SrcInt, /*IsSigned=*/ମିଥ୍ୟା, rmNearestTiesToEven);

  // If the first approximation already overflows, the number is too large.
  // NOTE: The underlying semantics are *more* conservative when choosing to
  // overflow because their notion of ULP is much larger. As such, it is always
  // safe to overflow at the DoubleAPFloat level if the APFloat overflows.
  ଯଦି (!Hi.isFinite())
    ଫେରନ୍ତୁ handleOverflow(RM);

  // Stage 2: Exact Error Calculation.
  // Calculate the exact error of the first approximation: Error = SrcInt - Hi.
  // This is done by converting Hi back to an integer and subtracting it from
  // the original source.
  ତାର୍କିକ HiAsIntIsExact;
  // Create an integer representation of Hi. Its width is determined by the
  // exponent of Hi, ensuring it's just large enough. This width can exceed
  // SrcBitWidth if the conversion to Hi rounded up to a power of two.
  // accurately when converted back to an integer.
  APSInt HiAsInt{static_cast<uint32_t>(ilogb(Hi) + 1), /*isUnsigned=*/ସତ୍ୟ};
  Hi.convertToInteger(HiAsInt, rmNearestTiesToEven, &HiAsIntIsExact);
  ଅଚଳ APInt Error = SrcInt.zext(HiAsInt.getBitWidth()) - HiAsInt;

  // Stage 3: Error Approximation and Rounding.
  // Convert the integer error into the Lo part of the DoubleAPFloat. This step
  // captures the remainder of the original number. The rounding mode for this
  // conversion (LoRM) may need to be adjusted from the user-requested RM to
  // ensure the final sum (Hi + Lo) rounds correctly.
  roundingMode LoRM = RM;
  // Adjustments are only necessary when the initial approximation Hi was an
  // overestimate, making the Error negative.
  ଯଦି (Error.isNegative()) {
    ଯଦି (RM == rmNearestTiesToAway) {
      // For rmNearestTiesToAway, a tie should round away from zero. Since
      // SrcInt is positive, this means rounding toward +infinity.
      // A standard conversion of a negative Error would round ties toward
      // -infinity, causing the final sum Hi + Lo to be smaller. To
      // counteract this, we detect the tie case and override the rounding
      // mode for Lo to rmTowardPositive.
      ଅଚଳ unsigned ErrorActiveBits = Error.getSignificantBits() - 1;
      ଅଚଳ unsigned LoPrecision = getSecond().getSemantics().precision;
      ଯଦି (ErrorActiveBits > LoPrecision) {
        ଅଚଳ unsigned RoundingBoundary = ErrorActiveBits - LoPrecision;
        // A tie occurs when the bits to be truncated are of the form 100...0.
        // This is detected by checking if the number of trailing zeros is
        // exactly one less than the number of bits being truncated.
        ଯଦି (Error.ଗଣନା_Trailing_Zeros() == RoundingBoundary - 1)
          LoRM = rmTowardPositive;
      }
    } ଅନ୍ୟଥା ଯଦି (RM == rmTowardZero) {
      // For rmTowardZero, the final positive result must be truncated (rounded
      // down). When Hi is an overestimate, Error is negative. A standard
      // rmTowardZero conversion of Error would make it *less* negative,
      // effectively rounding the final sum Hi + Lo *up*. To ensure the sum
      // rounds down correctly, we force Lo to round toward -infinity.
      LoRM = rmTowardNegative;
    }
  }

  APFloat Lo{getSecond().getSemantics()};
  opStatus Status = Lo.convertFromAPInt(Error, /*IsSigned=*/ସତ୍ୟ, LoRM);

  // Renormalize the pair (Hi, Lo) into a canonical DoubleAPFloat form where the
  // components do not overlap. fastTwoSum performs this operation.
  std::tie(Hi, Lo) = fastTwoSum(Hi, Lo);
  Floats[0] = std::move(Hi);
  Floats[1] = std::move(Lo);

  // A final check for overflow is needed because fastTwoSum can cause a
  // carry-out from Lo that pushes Hi to infinity.
  ଯଦି (!getFirst().isFinite())
    ଫେରନ୍ତୁ handleOverflow(RM);

  // The largest DoubleAPFloat must be canonical. Values which are larger are
  // not canonical and are equivalent to overflow.
  ଯଦି (getFirst().isFiniteNonZero() && Floats[0].isLargest()) {
    DoubleAPFloat Largest{*Semantics};
    Largest.makeLargest(/*Neg=*/ମିଥ୍ୟା);
    ଯଦି (compare(Largest) == APFloat::cmpGreaterThan)
      ଫେରନ୍ତୁ handleOverflow(RM);
  }

  // The final status of the operation is determined by the conversion of the
  // error term. If Lo could represent Error exactly, the entire conversion
  // is exact. Otherwise, it's inexact.
  ଫେରନ୍ତୁ Status;
}

APFloat::opStatus DoubleAPFloat::convertFromAPInt(ଅଚଳ APInt &ନିବେଶ,
                                                  ତାର୍କିକ IsSigned,
                                                  roundingMode RM) {
  ଅଚଳ ତାର୍କିକ Negate_ନିବେଶ = IsSigned && ନିବେଶ.isNegative();
  APInt API = ନିବେଶ;
  ଯଦି (Negate_ନିବେଶ)
    API.negate();

  ଅଚଳ APFloat::opStatus Status =
      convertFromUnsignedParts(API.getRawData(), API.get_ଅଙ୍କ_ଶବ୍ଦଗୁଡ଼ିକ(), RM);
  ଯଦି (Negate_ନିବେଶ)
    changeSign();
  ଫେରନ୍ତୁ Status;
}

unsigned ପୂର୍ଣ୍ଣସଂଖ୍ୟା DoubleAPFloat::convertToHexString(ଅକ୍ଷର *DST,
                                               unsigned ପୂର୍ଣ୍ଣସଂଖ୍ୟା HexDigits,
                                               ତାର୍କିକ UpperCase,
                                               roundingMode RM) ଅଚଳ {
  ପ୍ରତିଜ୍ଞା(Semantics == &APFloatBase::semPPCDoubleDouble &&
         "Unexpected Semantics");
  ଫେରନ୍ତୁ APFloat(APFloatBase::semPPCDoubleDoubleLegacy, bitcastToAPInt())
      .convertToHexString(DST, HexDigits, UpperCase, RM);
}

ତାର୍କିକ DoubleAPFloat::isDenormal() ଅଚଳ {
  ଫେରନ୍ତୁ getCategory() == fcNormal &&
         (Floats[0].isDenormal() || Floats[1].isDenormal() ||
          // (double)(Hi + Lo) == Hi defines a normal number.
          Floats[0] != Floats[0] + Floats[1]);
}

ତାର୍କିକ DoubleAPFloat::isSmallest() ଅଚଳ {
  ଯଦି (getCategory() != fcNormal)
    ଫେରନ୍ତୁ ମିଥ୍ୟା;
  DoubleAPFloat Tmp(*ଏହି);
  Tmp.makeSmallest(ଏହି->isNegative());
  ଫେରନ୍ତୁ Tmp.compare(*ଏହି) == cmpEqual;
}

ତାର୍କିକ DoubleAPFloat::isSmallestNormalized() ଅଚଳ {
  ଯଦି (getCategory() != fcNormal)
    ଫେରନ୍ତୁ ମିଥ୍ୟା;

  DoubleAPFloat Tmp(*ଏହି);
  Tmp.makeSmallestNormalized(ଏହି->isNegative());
  ଫେରନ୍ତୁ Tmp.compare(*ଏହି) == cmpEqual;
}

ତାର୍କିକ DoubleAPFloat::isLargest() ଅଚଳ {
  ଯଦି (getCategory() != fcNormal)
    ଫେରନ୍ତୁ ମିଥ୍ୟା;
  DoubleAPFloat Tmp(*ଏହି);
  Tmp.makeLargest(ଏହି->isNegative());
  ଫେରନ୍ତୁ Tmp.compare(*ଏହି) == cmpEqual;
}

ତାର୍କିକ DoubleAPFloat::isInteger() ଅଚଳ {
  ପ୍ରତିଜ୍ଞା(Semantics == &APFloatBase::semPPCDoubleDouble &&
         "Unexpected Semantics");
  ଫେରନ୍ତୁ Floats[0].isInteger() && Floats[1].isInteger();
}

ଶୂନ୍ୟ DoubleAPFloat::toString(SmallVectorImpl<ଅକ୍ଷର> &Str,
                             unsigned FormatPrecision,
                             unsigned FormatMaxPadding,
                             ତାର୍କିକ TruncateZero) ଅଚଳ {
  ପ୍ରତିଜ୍ଞା(Semantics == &APFloatBase::semPPCDoubleDouble &&
         "Unexpected Semantics");
  APFloat(APFloatBase::semPPCDoubleDoubleLegacy, bitcastToAPInt())
      .toString(Str, FormatPrecision, FormatMaxPadding, TruncateZero);
}

ପୂର୍ଣ୍ଣସଂଖ୍ୟା DoubleAPFloat::getExactLog2Abs() ଅଚଳ {
  // In order for Hi + Lo to be a power of two, the following must be true:
  // 1. Hi must be a power of two.
  // 2. Lo must be zero.
  ଯଦି (getSecond().isNonZero())
    ଫେରନ୍ତୁ INT_MIN;
  ଫେରନ୍ତୁ getFirst().getExactLog2Abs();
}

ପୂର୍ଣ୍ଣସଂଖ୍ୟା ilogb(ଅଚଳ DoubleAPFloat &Arg) {
  ଅଚଳ APFloat &Hi = Arg.getFirst();
  ଅଚଳ APFloat &Lo = Arg.getSecond();
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା IlogbResult = ilogb(Hi);
  // Zero and non-finite values can delegate to ilogb(Hi).
  ଯଦି (Arg.getCategory() != fcNormal)
    ଫେରନ୍ତୁ IlogbResult;
  // If Lo can't change the binade, we can delegate to ilogb(Hi).
  ଯଦି (Lo.isZero() || Hi.isNegative() == Lo.isNegative())
    ଫେରନ୍ତୁ IlogbResult;
  ଯଦି (Hi.getExactLog2Abs() == INT_MIN)
    ଫେରନ୍ତୁ IlogbResult;
  // Numbers of the form 2^a - 2^b or -2^a + 2^b are almost powers of two but
  // get nudged out of the binade by the low component.
  ଫେରନ୍ତୁ IlogbResult - 1;
}

DoubleAPFloat scalbn(ଅଚଳ DoubleAPFloat &Arg, ପୂର୍ଣ୍ଣସଂଖ୍ୟା Exp,
                     APFloat::roundingMode RM) {
  ପ୍ରତିଜ୍ଞା(Arg.Semantics == &APFloatBase::PPCDoubleDouble() &&
         "Unexpected Semantics");
  ଫେରନ୍ତୁ DoubleAPFloat(APFloatBase::PPCDoubleDouble(),
                       scalbn(Arg.Floats[0], Exp, RM),
                       scalbn(Arg.Floats[1], Exp, RM));
}

DoubleAPFloat frexp(ଅଚଳ DoubleAPFloat &Arg, ପୂର୍ଣ୍ଣସଂଖ୍ୟା &Exp,
                    APFloat::roundingMode RM) {
  ପ୍ରତିଜ୍ଞା(Arg.Semantics == &APFloatBase::PPCDoubleDouble() &&
         "Unexpected Semantics");

  // Get the unbiased exponent e of the number, where |Arg| = m * 2^e for m in
  // [1.0, 2.0).
  Exp = ilogb(Arg);

  // For NaNs, quiet any signaling NaN and return the result, as per standard
  // practice.
  ଯଦି (Exp == APFloat::IEK_NaN) {
    DoubleAPFloat Quiet{Arg};
    Quiet.getFirst() = Quiet.getFirst().makeQuiet();
    ଫେରନ୍ତୁ Quiet;
  }

  // For infinity, return it unchanged. The exponent remains IEK_Inf.
  ଯଦି (Exp == APFloat::IEK_Inf)
    ଫେରନ୍ତୁ Arg;

  // For zero, the fraction is zero and the standard requires the exponent be 0.
  ଯଦି (Exp == APFloat::IEK_Zero) {
    Exp = 0;
    ଫେରନ୍ତୁ Arg;
  }

  ଅଚଳ APFloat &Hi = Arg.getFirst();
  ଅଚଳ APFloat &Lo = Arg.getSecond();

  // frexp requires the fraction's absolute value to be in [0.5, 1.0).
  // ilogb provides an exponent for an absolute value in [1.0, 2.0).
  // Increment the exponent to ensure the fraction is in the correct range.
  ++Exp;

  ଅଚଳ ତାର୍କିକ SignsDisagree = Hi.isNegative() != Lo.isNegative();
  APFloat Second = Lo;
  ଯଦି (Arg.getCategory() == APFloat::fcNormal && Lo.isFiniteNonZero()) {
    roundingMode LoRoundingMode;
    // The interpretation of rmTowardZero depends on the sign of the combined
    // Arg rather than the sign of the component.
    ଯଦି (RM == rmTowardZero)
      LoRoundingMode = Arg.isNegative() ? rmTowardPositive : rmTowardNegative;
    // For rmNearestTiesToAway, we face a similar problem. If signs disagree,
    // Lo is a correction *toward* zero relative to Hi. Rounding Lo
    // "away from zero" based on its own sign would move the value in the
    // wrong direction. As a safe proxy, we use rmNearestTiesToEven, which is
    // direction-agnostic. We only need to bother with this if Lo is scaled
    // down.
    ଅନ୍ୟଥା ଯଦି (RM == rmNearestTiesToAway && SignsDisagree && Exp > 0)
      LoRoundingMode = rmNearestTiesToEven;
    ଅନ୍ୟଥା
      LoRoundingMode = RM;
    Second = scalbn(Lo, -Exp, LoRoundingMode);
    // The rmNearestTiesToEven proxy is correct most of the time, but it
    // differs from rmNearestTiesToAway when the scaled value of Lo is an
    // exact midpoint.
    // NOTE: This is morally equivalent to roundTiesTowardZero.
    ଯଦି (RM == rmNearestTiesToAway && LoRoundingMode == rmNearestTiesToEven) {
      // Re-scale the result back to check if rounding occurred.
      ଅଚଳ APFloat RecomposedLo = scalbn(Second, Exp, rmNearestTiesToEven);
      ଯଦି (RecomposedLo != Lo) {
        // RoundingError tells us which direction we rounded:
        //   - RoundingError > 0: we rounded up.
        //   - RoundingError < 0: we down up.
        ଅଚଳ APFloat RoundingError = RecomposedLo - Lo;
        // Determine if scalbn(Lo, -Exp) landed exactly on a midpoint.
        // We do this by checking if the absolute rounding error is exactly
        // half a ULP of the result.
        ଅଚଳ APFloat UlpOfSecond = harrisonUlp(Second);
        ଅଚଳ APFloat ScaledUlpOfSecond =
            scalbn(UlpOfSecond, Exp - 1, rmNearestTiesToEven);
        ଅଚଳ ତାର୍କିକ IsMidpoint = abs(RoundingError) == ScaledUlpOfSecond;
        ଅଚଳ ତାର୍କିକ RoundedLoAway =
            Second.isNegative() == RoundingError.isNegative();
        // The sign of Hi and Lo disagree and we rounded Lo away: we must
        // decrease the magnitude of Second to increase the magnitude
        // First+Second.
        ଯଦି (IsMidpoint && RoundedLoAway)
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
    ଯଦି (Second.isZero() && SignsDisagree && Hi.getExactLog2Abs() != INT_MIN)
      ++Exp;
  }

  APFloat First = scalbn(Hi, -Exp, RM);
  ଫେରନ୍ତୁ DoubleAPFloat(APFloatBase::PPCDoubleDouble(), std::move(First),
                       std::move(Second));
}

APInt DoubleAPFloat::getNaNPayload() ଅଚଳ { ଫେରନ୍ତୁ Floats[0].getNaNPayload(); }
} // namespace detail

APFloat::Storage::Storage(IEEEFloat F, ଅଚଳ fltSemantics &Semantics) {
  ଯଦି (usesLayout<IEEEFloat>(Semantics)) {
    ନୂତନ (&IEEE) IEEEFloat(std::move(F));
    ଫେରନ୍ତୁ;
  }
  ଯଦି (usesLayout<DoubleAPFloat>(Semantics)) {
    ଅଚଳ fltSemantics& S = F.getSemantics();
    ନୂତନ (&Double) DoubleAPFloat(Semantics, APFloat(std::move(F), S),
                                APFloat(APFloatBase::IEEEdouble()));
    ଫେରନ୍ତୁ;
  }
  llvm_unreachable("Unexpected semantics");
}

Expected<APFloat::opStatus> APFloat::convertFromString(StringRef Str,
                                                       roundingMode RM) {
  APFLOAT_DISPATCH_ON_SEMANTICS(convertFromString(Str, RM));
}

hash_code hash_ମୂଲ୍ୟ(ଅଚଳ APFloat &Arg) {
  ଯଦି (APFloat::usesLayout<detail::IEEEFloat>(Arg.getSemantics()))
    ଫେରନ୍ତୁ hash_ମୂଲ୍ୟ(Arg.U.IEEE);
  ଯଦି (APFloat::usesLayout<detail::DoubleAPFloat>(Arg.getSemantics()))
    ଫେରନ୍ତୁ hash_ମୂଲ୍ୟ(Arg.U.Double);
  llvm_unreachable("Unexpected semantics");
}

APFloat::APFloat(ଅଚଳ fltSemantics &Semantics, StringRef S)
    : APFloat(Semantics) {
  auto StatusOrErr = convertFromString(S, rmNearestTiesToEven);
  ପ୍ରତିଜ୍ଞା(StatusOrErr && "Invalid floating point representation");
  consumeError(StatusOrErr.takeError());
}

FPClassTest APFloat::classify() ଅଚଳ {
  ଯଦି (isZero())
    ଫେରନ୍ତୁ isNegative() ? fcNegZero : fcPosZero;
  ଯଦି (isNormal())
    ଫେରନ୍ତୁ isNegative() ? fcNegNormal : fcPosNormal;
  ଯଦି (isDenormal())
    ଫେରନ୍ତୁ isNegative() ? fcNegSubnormal : fcPosSubnormal;
  ଯଦି (isInfinity())
    ଫେରନ୍ତୁ isNegative() ? fcNegInf : fcPosInf;
  ପ୍ରତିଜ୍ଞା(isNaN() && "Other class of FP constant");
  ଫେରନ୍ତୁ isSignaling() ? fcSNan : fcQNan;
}

ତାର୍କିକ APFloat::getExactInverse(APFloat *Inv) ଅଚଳ {
  // Only finite, non-zero numbers can have a useful, representable inverse.
  // This check filters out +/- zero, +/- infinity, and NaN.
  ଯଦି (!isFiniteNonZero())
    ଫେରନ୍ତୁ ମିଥ୍ୟା;

  // Historically, this function rejects subnormal inputs.  One reason why this
  // might be important is that subnormals may behave differently under FTZ/DAZ
  // runtime behavior.
  ଯଦି (isDenormal())
    ଫେରନ୍ତୁ ମିଥ୍ୟା;

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
  ଅଚଳ ପୂର୍ଣ୍ଣସଂଖ୍ୟା Exp = getExactLog2Abs();
  ଯଦି (Exp == INT_MIN)
    ଫେରନ୍ତୁ ମିଥ୍ୟା;

  // The inverse of +/- 2^Exp is +/- 2^(-Exp). We can compute this by
  // scaling 1.0 by the negated exponent.
  APFloat Reciprocal =
      scalbn(APFloat::getOne(getSemantics(), /*Negative=*/isNegative()), -Exp,
             rmTowardZero);

  // scalbn might round if the resulting exponent -Exp is outside the
  // representable range, causing overflow (to infinity) or underflow. We
  // must verify that the result is still the exact power of two we expect.
  ଯଦି (Reciprocal.getExactLog2Abs() != -Exp)
    ଫେରନ୍ତୁ ମିଥ୍ୟା;

  // Avoid multiplication with a subnormal, it is not safe on all platforms and
  // may be slower than a normal division.
  ଯଦି (Reciprocal.isDenormal())
    ଫେରନ୍ତୁ ମିଥ୍ୟା;

  ପ୍ରତିଜ୍ଞା(Reciprocal.isFiniteNonZero());

  ଯଦି (Inv)
    *Inv = std::move(Reciprocal);

  ଫେରନ୍ତୁ ସତ୍ୟ;
}

APFloat::opStatus APFloat::convert(ଅଚଳ fltSemantics &ToSemantics,
                                   roundingMode RM, ତାର୍କିକ *losesInfo) {
  ଯଦି (&getSemantics() == &ToSemantics) {
    *losesInfo = ମିଥ୍ୟା;
    ଫେରନ୍ତୁ opOK;
  }
  ଯଦି (usesLayout<IEEEFloat>(getSemantics()) &&
      usesLayout<IEEEFloat>(ToSemantics))
    ଫେରନ୍ତୁ U.IEEE.convert(ToSemantics, RM, losesInfo);
  ଯଦି (usesLayout<IEEEFloat>(getSemantics()) &&
      usesLayout<DoubleAPFloat>(ToSemantics)) {
    ପ୍ରତିଜ୍ଞା(&ToSemantics == &APFloatBase::semPPCDoubleDouble);
    auto Ret =
        U.IEEE.convert(APFloatBase::semPPCDoubleDoubleLegacy, RM, losesInfo);
    *ଏହି = APFloat(ToSemantics, U.IEEE.bitcastToAPInt());
    ଫେରନ୍ତୁ Ret;
  }
  ଯଦି (usesLayout<DoubleAPFloat>(getSemantics()) &&
      usesLayout<IEEEFloat>(ToSemantics)) {
    auto Ret = getIEEE().convert(ToSemantics, RM, losesInfo);
    *ଏହି = APFloat(std::move(getIEEE()), ToSemantics);
    ଫେରନ୍ତୁ Ret;
  }
  llvm_unreachable("Unexpected semantics");
}

APFloat APFloat::get_All_Ones_ମୂଲ୍ୟ(ଅଚଳ fltSemantics &Semantics) {
  ଫେରନ୍ତୁ APFloat(Semantics, APInt::getAllOnes(Semantics.sizeInBits));
}

ଶୂନ୍ୟ APFloat::print(raw_ostream &OS) ଅଚଳ {
  SmallVector<ଅକ୍ଷର, 16> Buffer;
  toString(Buffer);
  OS << Buffer;
}

#ଯଦି !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)
LLVM_DUMP_METHOD ଶୂନ୍ୟ APFloat::dump() ଅଚଳ {
  print(dbgs());
  dbgs() << '\n';
}
#endif

ଶୂନ୍ୟ APFloat::Profile(Folding_Set_ଗଣ୍ଠି_ID &NID) ଅଚଳ {
  NID.Add(bitcastToAPInt());
}

APFloat::opStatus APFloat::convertToInteger(APSInt &result,
                                            roundingMode rounding_mode,
                                            ତାର୍କିକ *isExact) ଅଚଳ {
  unsigned bitWidth = result.getBitWidth();
  SmallVector<uint64_t, 4> parts(result.get_ଅଙ୍କ_ଶବ୍ଦଗୁଡ଼ିକ());
  opStatus status = convertToInteger(parts, bitWidth, result.isSigned(),
                                     rounding_mode, isExact);
  // Keeps the original signed-ness.
  result = APInt(bitWidth, parts);
  ଫେରନ୍ତୁ status;
}

ଦ୍ୱିଗୁଣ APFloat::convertToDouble() ଅଚଳ {
  ଯଦି (&getSemantics() == &APFloatBase::semIEEEdouble)
    ଫେରନ୍ତୁ getIEEE().convertToDouble();
  ପ୍ରତିଜ୍ଞା(isRepresentableBy(getSemantics(), semIEEEdouble) &&
         "Float semantics is not representable by IEEEdouble");
  APFloat Temp = *ଏହି;
  ତାର୍କିକ LosesInfo;
  [[maybe_unused]] opStatus St =
      Temp.convert(APFloatBase::semIEEEdouble, rmNearestTiesToEven, &LosesInfo);
  ପ୍ରତିଜ୍ଞା(!(St & opInexact) && !LosesInfo && "Unexpected imprecision");
  ଫେରନ୍ତୁ Temp.getIEEE().convertToDouble();
}

#ifdef HAS_IEE754_FLOAT128
float128 APFloat::convertToQuad() ଅଚଳ {
  ଯଦି (&getSemantics() == &APFloatBase::semIEEEquad)
    ଫେରନ୍ତୁ getIEEE().convertToQuad();
  ପ୍ରତିଜ୍ଞା(isRepresentableBy(getSemantics(), semIEEEquad) &&
         "Float semantics is not representable by IEEEquad");
  APFloat Temp = *ଏହି;
  ତାର୍କିକ LosesInfo;
  [[maybe_unused]] opStatus St =
      Temp.convert(APFloatBase::semIEEEquad, rmNearestTiesToEven, &LosesInfo);
  ପ୍ରତିଜ୍ଞା(!(St & opInexact) && !LosesInfo && "Unexpected imprecision");
  ଫେରନ୍ତୁ Temp.getIEEE().convertToQuad();
}
#endif

ଦଶମିକ APFloat::convertToFloat() ଅଚଳ {
  ଯଦି (&getSemantics() == &APFloatBase::semIEEEsingle)
    ଫେରନ୍ତୁ getIEEE().convertToFloat();
  ପ୍ରତିଜ୍ଞା(isRepresentableBy(getSemantics(), semIEEEsingle) &&
         "Float semantics is not representable by IEEEsingle");
  APFloat Temp = *ଏହି;
  ତାର୍କିକ LosesInfo;
  [[maybe_unused]] opStatus St =
      Temp.convert(APFloatBase::semIEEEsingle, rmNearestTiesToEven, &LosesInfo);
  ପ୍ରତିଜ୍ଞା(!(St & opInexact) && !LosesInfo && "Unexpected imprecision");
  ଫେରନ୍ତୁ Temp.getIEEE().convertToFloat();
}

unsigned APFloatBase::getArbitraryFPFormatSizeInBits(StringRef Format) {
  ଫେରନ୍ତୁ StringSwitch<unsigned>(Format)
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

ତାର୍କିକ APFloatBase::isValidArbitraryFPFormat(StringRef Format) {
  ଫେରନ୍ତୁ getArbitraryFPFormatSizeInBits(Format) != 0;
}

ଅଚଳ fltSemantics *APFloatBase::getArbitraryFPSemantics(StringRef Format) {
  // TODO: extend to remaining arbitrary FP types: Float8E4M3, Float8E3M4,
  // Float8E5M2FNUZ, Float8E4M3FNUZ, Float8E4M3B11FNUZ, Float8E8M0FNU.
  ଫେରନ୍ତୁ StringSwitch<ଅଚଳ fltSemantics *>(Format)
      .Case("Float8E5M2", &semFloat8E5M2)
      .Case("Float8E4M3FN", &semFloat8E4M3FN)
      .Case("Float8E5M3FNU", &semFloat8E5M3FNU)
      .Case("Float4E2M1FN", &semFloat4E2M1FN)
      .Case("Float6E3M2FN", &semFloat6E3M2FN)
      .Case("Float6E2M3FN", &semFloat6E2M3FN)
      .Default(nullptr);
}

APFloat::Storage::~Storage() {
  ଯଦି (usesLayout<IEEEFloat>(*semantics)) {
    IEEE.~IEEEFloat();
    ଫେରନ୍ତୁ;
  }
  ଯଦି (usesLayout<DoubleAPFloat>(*semantics)) {
    Double.~DoubleAPFloat();
    ଫେରନ୍ତୁ;
  }
  llvm_unreachable("Unexpected semantics");
}

APFloat::Storage::Storage(ଅଚଳ APFloat::Storage &RHS) {
  ଯଦି (usesLayout<IEEEFloat>(*RHS.semantics)) {
    ନୂତନ (ଏହି) IEEEFloat(RHS.IEEE);
    ଫେରନ୍ତୁ;
  }
  ଯଦି (usesLayout<DoubleAPFloat>(*RHS.semantics)) {
    ନୂତନ (ଏହି) DoubleAPFloat(RHS.Double);
    ଫେରନ୍ତୁ;
  }
  llvm_unreachable("Unexpected semantics");
}

APFloat::Storage::Storage(APFloat::Storage &&RHS) {
  ଯଦି (usesLayout<IEEEFloat>(*RHS.semantics)) {
    ନୂତନ (ଏହି) IEEEFloat(std::move(RHS.IEEE));
    ଫେରନ୍ତୁ;
  }
  ଯଦି (usesLayout<DoubleAPFloat>(*RHS.semantics)) {
    ନୂତନ (ଏହି) DoubleAPFloat(std::move(RHS.Double));
    ଫେରନ୍ତୁ;
  }
  llvm_unreachable("Unexpected semantics");
}

APFloat::Storage &APFloat::Storage::operator=(ଅଚଳ APFloat::Storage &RHS) {
  ଯଦି (usesLayout<IEEEFloat>(*semantics) &&
      usesLayout<IEEEFloat>(*RHS.semantics)) {
    IEEE = RHS.IEEE;
  } ଅନ୍ୟଥା ଯଦି (usesLayout<DoubleAPFloat>(*semantics) &&
             usesLayout<DoubleAPFloat>(*RHS.semantics)) {
    Double = RHS.Double;
  } ଅନ୍ୟଥା ଯଦି (ଏହି != &RHS) {
    ଏହି->~Storage();
    ନୂତନ (ଏହି) Storage(RHS);
  }
  ଫେରନ୍ତୁ *ଏହି;
}

APFloat::Storage &APFloat::Storage::operator=(APFloat::Storage &&RHS) {
  ଯଦି (usesLayout<IEEEFloat>(*semantics) &&
      usesLayout<IEEEFloat>(*RHS.semantics)) {
    IEEE = std::move(RHS.IEEE);
  } ଅନ୍ୟଥା ଯଦି (usesLayout<DoubleAPFloat>(*semantics) &&
             usesLayout<DoubleAPFloat>(*RHS.semantics)) {
    Double = std::move(RHS.Double);
  } ଅନ୍ୟଥା ଯଦି (ଏହି != &RHS) {
    ଏହି->~Storage();
    ନୂତନ (ଏହି) Storage(std::move(RHS));
  }
  ଫେରନ୍ତୁ *ଏହି;
}

ନାମାକାଶ {

APFloat::opStatus getOpStatusFromLibc(ପୂର୍ଣ୍ଣସଂଖ୍ୟା libc_exceptions) {
  APFloat::opStatus status = APFloat::opOK;
  ଯଦି (libc_exceptions & FE_INVALID)
    status = static_cast<APFloat::opStatus>(status | APFloat::opInvalidOp);
  ଯଦି (libc_exceptions & FE_DIVBYZERO)
    status = static_cast<APFloat::opStatus>(status | APFloat::opDivByZero);
  ଯଦି (libc_exceptions & FE_OVERFLOW)
    status = static_cast<APFloat::opStatus>(status | APFloat::opOverflow);
  ଯଦି (libc_exceptions & FE_UNDERFLOW)
    status = static_cast<APFloat::opStatus>(status | APFloat::opUnderflow);
  ଯଦି (libc_exceptions & FE_INEXACT)
    status = static_cast<APFloat::opStatus>(status | APFloat::opInexact);
  ଫେରନ୍ତୁ status;
}

} // namespace

// TODO: Support other rounding modes when LLVM libc math implement static
// roundings.
std::optional<APFloat> exp(ଅଚଳ APFloat &x, RoundingMode rounding_mode,
                           APFloat::opStatus *status) {

  ଯଦି (rounding_mode == APFloatBase::rmNearestTiesToEven) {
    ଯଦି (APFloat::SemanticsToEnum(x.getSemantics()) ==
        APFloatBase::S_IEEEsingle) {
      ଦଶମିକ x_val = x.convertToFloat();
      ପୂର୍ଣ୍ଣସଂଖ୍ୟା exc =
          LIBC_NAMESPACE::shared::check::exp_exceptions(x_val, FE_TONEAREST);
      ଯଦି (status) {
        *status = getOpStatusFromLibc(exc);
        ଯଦି (x.isSignaling()) {
          // 32-bit x86 will silence sNaN when loading floats, so we explicitly
          // add the INVALID exception here.
          *status =
              static_cast<APFloat::opStatus>(*status | APFloat::opInvalidOp);
        }
      }
      ଦଶମିକ result = LIBC_NAMESPACE::shared::expf(x_val);
      ଫେରନ୍ତୁ APFloat(result);
    }
    ଯଦି (APFloat::SemanticsToEnum(x.getSemantics()) ==
        APFloatBase::S_IEEEdouble) {
      ଦ୍ୱିଗୁଣ x_val = x.convertToDouble();
      ପୂର୍ଣ୍ଣସଂଖ୍ୟା exc =
          LIBC_NAMESPACE::shared::check::exp_exceptions(x_val, FE_TONEAREST);
      ଯଦି (status) {
        *status = getOpStatusFromLibc(exc);
        ଯଦି (x.isSignaling()) {
          // 32-bit x86 will silence sNaN when loading floats, so we explicitly
          // add the INVALID exception here.
          *status =
              static_cast<APFloat::opStatus>(*status | APFloat::opInvalidOp);
        }
      }
      ଦ୍ୱିଗୁଣ result = LIBC_NAMESPACE::shared::exp(x_val);
      ଫେରନ୍ତୁ APFloat(result);
    }
  }
  ଫେରନ୍ତୁ std::nullopt;
}

} // namespace llvm

#undef APFLOAT_DISPATCH_ON_SEMANTICS
