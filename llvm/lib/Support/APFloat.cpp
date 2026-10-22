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

#అన్తర్భావ "llvm/ADT/APFloat.h"
#అన్తర్భావ "llvm/ADT/APSInt.h"
#అన్తర్భావ "llvm/ADT/ArrayRef.h"
#అన్తర్భావ "llvm/ADT/FloatingPointMode.h"
#అన్తర్భావ "llvm/ADT/FoldingSet.h"
#అన్తర్భావ "llvm/ADT/Hashing.h"
#అన్తర్భావ "llvm/ADT/STLExtras.h"
#అన్తర్భావ "llvm/ADT/SmallVector.h"
#అన్తర్భావ "llvm/ADT/StringExtras.h"
#అన్తర్భావ "llvm/ADT/StringRef.h"
#అన్తర్భావ "llvm/ADT/StringSwitch.h"
#అన్తర్భావ "llvm/Config/llvm-config.h"
#అన్తర్భావ "llvm/Support/Debug.h"
#అన్తర్భావ "llvm/Support/Error.h"
#అన్తర్భావ "llvm/Support/MathExtras.h"
#అన్తర్భావ "llvm/Support/raw_ostream.h"
#అన్తర్భావ <cstring>
#అన్తర్భావ <iterator>
#అన్తర్భావ <limits.h>

/// Shared headers from LLVM libc
/// Make sure to add ${LLVM_SOURCE_DIR}/../libc to include directories.
///
/// Notes: So far it looks like APFloat does not check errnos or floating-point
/// exceptions after calling the math functions, so we will configure LLVM libc
/// math functions to skip setting errnos and floating-point exceptions
/// explicitly.  We also put them in a separate namespace so that the symbols
/// do not clash with other libc math builds just in case.
#నిర్వచించు LIBC_NAMESPACE __llvm_libc_apfloat
#నిర్వచించు LIBC_MATH (LIBC_MATH_NO_ERRNO | LIBC_MATH_NO_EXCEPT)

#అన్తర్భావ "shared/math.h"
#అన్తర్భావ "shared/math_check_exceptions.h"

#నిర్వచించు APFLOAT_DISPATCH_ON_SEMANTICS(పద్ధతి_CALL)                             \
  చేయి {                                                                         \
    ఒకవేళ (ఉపయోగిస్తుంది_Layout<IEEEFloat>(getSemantics()))                                 \
      తిరిగి U.IEEE.పద్ధతి_CALL;                                               \
    ఒకవేళ (ఉపయోగిస్తుంది_Layout<DoubleAPFloat>(getSemantics()))                             \
      తిరిగి U.Double.పద్ధతి_CALL;                                             \
    llvm_unreachable("Unexpected semantics");                                  \
  } అయితే (అబద్ధం)

ఉపయోగించి నామాకాశ llvm;

/// A macro used to combine two fcCategory enums into one key which can be used
/// in a switch statement to classify how the interaction of two APFloat's
/// categories affects an operation.
///
/// TODO: If clang source code is ever allowed to use constexpr in its own
/// codebase, change this into a static inline function.
#నిర్వచించు Pack_Categories_Into_కీలకం(_lhs, _rhs) ((_lhs) * 4 + (_rhs))

/* Assumed in hexadecimal significand parsing, and conversion to
   hexadecimal strings.  */
static_assert(APFloat_పునాది::integer_Part_వెడల్పు % 4 == 0, "Part width must be divisible by 4!");

నామాకాశ llvm {

constexpr fltSemantics APFloat_పునాది::semIEEEhalf = {15, -14, 11, 16};
constexpr fltSemantics APFloat_పునాది::semBFloat = {127, -126, 8, 16};
constexpr fltSemantics APFloat_పునాది::semIEEEsingle = {127, -126, 24, 32};
constexpr fltSemantics APFloat_పునాది::semIEEEdouble = {1023, -1022, 53, 64};
constexpr fltSemantics APFloat_పునాది::semIEEEquad = {16383, -16382, 113, 128};
constexpr fltSemantics APFloat_పునాది::semX87DoubleExtended = {
    16383,
    -16382,
    64,
    80,
    fltNonfiniteBehavior::IEEE754,
    fltNanEncoding::IEEE,
    నిజం,
    నిజం,
    నిజం,
    నిజం,
    నిజం};
constexpr fltSemantics APFloat_పునాది::semPPCDoubleDouble = {-1, 0, 0, 128};
constexpr fltSemantics APFloat_పునాది::semPPCDoubleDoubleLegacy = {
    1023, -1022 + 53, 53 + 53, 128};
constexpr fltSemantics APFloat_పునాది::semFloat8E5M2 = {15, -14, 3, 8};
constexpr fltSemantics APFloat_పునాది::semFloat8E5M2FNUZ = {
    15, -15, 3, 8, fltNonfiniteBehavior::NanOnly, fltNanEncoding::NegativeZero};
constexpr fltSemantics APFloat_పునాది::semFloat8E4M3 = {7, -6, 4, 8};
constexpr fltSemantics APFloat_పునాది::semFloat8E4M3FN = {
    8, -6, 4, 8, fltNonfiniteBehavior::NanOnly, fltNanEncoding::AllOnes};
constexpr fltSemantics APFloat_పునాది::semFloat8E4M3FNUZ = {
    7, -7, 4, 8, fltNonfiniteBehavior::NanOnly, fltNanEncoding::NegativeZero};
constexpr fltSemantics APFloat_పునాది::semFloat8E4M3B11FNUZ = {
    4, -10, 4, 8, fltNonfiniteBehavior::NanOnly, fltNanEncoding::NegativeZero};
constexpr fltSemantics APFloat_పునాది::semFloat8E3M4 = {3, -2, 5, 8};
constexpr fltSemantics APFloat_పునాది::semFloatTF32 = {127, -126, 11, 19};
constexpr fltSemantics APFloat_పునాది::semFloat8E8M0FNU = {
    127,
    -127,
    1,
    8,
    fltNonfiniteBehavior::NanOnly,
    fltNanEncoding::AllOnes,
    అబద్ధం,
    అబద్ధం,
    అబద్ధం,
    అబద్ధం};

constexpr fltSemantics APFloat_పునాది::semFloat8E5M3FNU = {
    16,
    -14,
    4,
    8,
    fltNonfiniteBehavior::NanOnly,
    fltNanEncoding::AllOnes,
    నిజం,
    అబద్ధం,
    అబద్ధం};

constexpr fltSemantics APFloat_పునాది::semFloat6E3M2FN = {
    4, -2, 3, 6, fltNonfiniteBehavior::FiniteOnly};
constexpr fltSemantics APFloat_పునాది::semFloat6E2M3FN = {
    2, 0, 4, 6, fltNonfiniteBehavior::FiniteOnly};
constexpr fltSemantics APFloat_పునాది::semFloat4E2M1FN = {
    2, 0, 2, 4, fltNonfiniteBehavior::FiniteOnly};
constexpr fltSemantics APFloat_పునాది::semBogus = {0, 0, 0, 0};

స్థిరరాశి llvm::fltSemantics &APFloat_పునాది::Enum_గమ్యం_Semantics(Semantics S) {
  ఎంపిక (S) {
  స్థితి S_IEEEhalf:
    తిరిగి IEEEhalf();
  స్థితి S_BFloat:
    తిరిగి BFloat();
  స్థితి S_IEEEsingle:
    తిరిగి IEEEsingle();
  స్థితి S_IEEEdouble:
    తిరిగి IEEEdouble();
  స్థితి S_IEEEquad:
    తిరిగి IEEEquad();
  స్థితి S_x87DoubleExtended:
    తిరిగి x87DoubleExtended();
  స్థితి S_PPCDoubleDouble:
    తిరిగి PPCDoubleDouble();
  స్థితి S_PPCDoubleDoubleLegacy:
    తిరిగి PPCDoubleDoubleLegacy();
  స్థితి S_Float8E5M2:
    తిరిగి Float8E5M2();
  స్థితి S_Float8E5M2FNUZ:
    తిరిగి Float8E5M2FNUZ();
  స్థితి S_Float8E4M3:
    తిరిగి Float8E4M3();
  స్థితి S_Float8E4M3FN:
    తిరిగి Float8E4M3FN();
  స్థితి S_Float8E4M3FNUZ:
    తిరిగి Float8E4M3FNUZ();
  స్థితి S_Float8E4M3B11FNUZ:
    తిరిగి Float8E4M3B11FNUZ();
  స్థితి S_Float8E3M4:
    తిరిగి Float8E3M4();
  స్థితి S_FloatTF32:
    తిరిగి FloatTF32();
  స్థితి S_Float8E8M0FNU:
    తిరిగి Float8E8M0FNU();
  స్థితి S_Float8E5M3FNU:
    తిరిగి Float8E5M3FNU();
  స్థితి S_Float6E3M2FN:
    తిరిగి Float6E3M2FN();
  స్థితి S_Float6E2M3FN:
    తిరిగి Float6E2M3FN();
  స్థితి S_Float4E2M1FN:
    తిరిగి Float4E2M1FN();
  }
  llvm_unreachable("Unrecognised floating semantics");
}

APFloat_పునాది::Semantics
APFloat_పునాది::Semantics_గమ్యం_Enum(స్థిరరాశి llvm::fltSemantics &Sem) {
  ఒకవేళ (&Sem == &llvm::APFloat::IEEEhalf())
    తిరిగి S_IEEEhalf;
  లేకపోతే ఒకవేళ (&Sem == &llvm::APFloat::BFloat())
    తిరిగి S_BFloat;
  లేకపోతే ఒకవేళ (&Sem == &llvm::APFloat::IEEEsingle())
    తిరిగి S_IEEEsingle;
  లేకపోతే ఒకవేళ (&Sem == &llvm::APFloat::IEEEdouble())
    తిరిగి S_IEEEdouble;
  లేకపోతే ఒకవేళ (&Sem == &llvm::APFloat::IEEEquad())
    తిరిగి S_IEEEquad;
  లేకపోతే ఒకవేళ (&Sem == &llvm::APFloat::x87DoubleExtended())
    తిరిగి S_x87DoubleExtended;
  లేకపోతే ఒకవేళ (&Sem == &llvm::APFloat::PPCDoubleDouble())
    తిరిగి S_PPCDoubleDouble;
  లేకపోతే ఒకవేళ (&Sem == &llvm::APFloat::PPCDoubleDoubleLegacy())
    తిరిగి S_PPCDoubleDoubleLegacy;
  లేకపోతే ఒకవేళ (&Sem == &llvm::APFloat::Float8E5M2())
    తిరిగి S_Float8E5M2;
  లేకపోతే ఒకవేళ (&Sem == &llvm::APFloat::Float8E5M2FNUZ())
    తిరిగి S_Float8E5M2FNUZ;
  లేకపోతే ఒకవేళ (&Sem == &llvm::APFloat::Float8E4M3())
    తిరిగి S_Float8E4M3;
  లేకపోతే ఒకవేళ (&Sem == &llvm::APFloat::Float8E4M3FN())
    తిరిగి S_Float8E4M3FN;
  లేకపోతే ఒకవేళ (&Sem == &llvm::APFloat::Float8E4M3FNUZ())
    తిరిగి S_Float8E4M3FNUZ;
  లేకపోతే ఒకవేళ (&Sem == &llvm::APFloat::Float8E4M3B11FNUZ())
    తిరిగి S_Float8E4M3B11FNUZ;
  లేకపోతే ఒకవేళ (&Sem == &llvm::APFloat::Float8E3M4())
    తిరిగి S_Float8E3M4;
  లేకపోతే ఒకవేళ (&Sem == &llvm::APFloat::FloatTF32())
    తిరిగి S_FloatTF32;
  లేకపోతే ఒకవేళ (&Sem == &llvm::APFloat::Float8E8M0FNU())
    తిరిగి S_Float8E8M0FNU;
  లేకపోతే ఒకవేళ (&Sem == &llvm::APFloat::Float8E5M3FNU())
    తిరిగి S_Float8E5M3FNU;
  లేకపోతే ఒకవేళ (&Sem == &llvm::APFloat::Float6E3M2FN())
    తిరిగి S_Float6E3M2FN;
  లేకపోతే ఒకవేళ (&Sem == &llvm::APFloat::Float6E2M3FN())
    తిరిగి S_Float6E2M3FN;
  లేకపోతే ఒకవేళ (&Sem == &llvm::APFloat::Float4E2M1FN())
    తిరిగి S_Float4E2M1FN;
  లేకపోతే
    llvm_unreachable("Unknown floating semantics");
}

తార్కిక APFloat_పునాది::isRepresentableBy(స్థిరరాశి fltSemantics &A,
                                    స్థిరరాశి fltSemantics &B) {
  తిరిగి A.maxExponent <= B.maxExponent && A.minExponent >= B.minExponent &&
         A.precision <= B.precision;
}

తార్కిక APFloat_పునాది::is_Losslessly_Convertible_గమ్యం(స్థిరరాశి fltSemantics &From,
                                            స్థిరరాశి fltSemantics &Initial_గమ్యం,
                                            తార్కిక IgnoreNaNs) {
  ఒకవేళ (&From == &Initial_గమ్యం)
    తిరిగి నిజం;

  // PPC double-double cannot be described by a conventional exponent range
  // and precision. In particular, converting it to another semantics drops
  // its low double, so conservatively reject conversions from it.
  ఒకవేళ (&From == &semPPCDoubleDouble)
    తిరిగి అబద్ధం;

  // Converting to PPC double-double is lossless exactly when the value fits
  // in its high double. In other words, the conversion is lossless if the
  // conversion to IEEEdouble is lossless.
  స్థిరరాశి fltSemantics &గమ్యం =
      &Initial_గమ్యం == &semPPCDoubleDouble ? semIEEEdouble : Initial_గమ్యం;

  ఒకవేళ (!isRepresentableBy(From, గమ్యం))
    తిరిగి అబద్ధం;

  ఒకవేళ ((From.hasZero && !గమ్యం.hasZero) ||
      (From.hasSignedRepr && !గమ్యం.hasSignedRepr))
    తిరిగి అబద్ధం;

  // NegativeZero NaN encoding repurposes the negative-zero bit pattern, so a
  // conversion to such a format cannot preserve a source negative zero.
  తార్కిక FromHasSignedZero = From.hasZero && From.hasSignedRepr &&
                           From.nanEncoding != fltNanEncoding::NegativeZero;
  తార్కిక గమ్యం_Has_Signed_Zero = గమ్యం.hasZero && గమ్యం.hasSignedRepr &&
                         గమ్యం.nanEncoding != fltNanEncoding::NegativeZero;
  ఒకవేళ (FromHasSignedZero && !గమ్యం_Has_Signed_Zero)
    తిరిగి అబద్ధం;

  // isRepresentableBy compares normalized exponent ranges. Also ensure that
  // the smallest source value, which may be denormal, is represented exactly
  // by the destination semantics.
  APFloat అతిచిన్న_From = APFloat::get_అతిచిన్న(From);
  తార్కిక LosesInfo = అబద్ధం;
  (శూన్య)అతిచిన్న_From.convert(గమ్యం, APFloat::rm_సమీప_Ties_గమ్యం_Even, &LosesInfo);
  ఒకవేళ (LosesInfo)
    తిరిగి అబద్ధం;

  ఒకవేళ (From.nonFiniteBehavior == fltNonfiniteBehavior::FiniteOnly)
    తిరిగి నిజం;

  // Even when NaN representations can be ignored, NaNs must remain NaNs and
  // infinities must remain infinities. Otherwise the conversion can change
  // whether an operation with nnan has poison-producing operands.
  ఒకవేళ (IgnoreNaNs) {
    ఒకవేళ (From.nonFiniteBehavior == fltNonfiniteBehavior::IEEE754)
      తిరిగి గమ్యం.nonFiniteBehavior == fltNonfiniteBehavior::IEEE754;
    తిరిగి గమ్యం.nonFiniteBehavior != fltNonfiniteBehavior::FiniteOnly;
  }

  ఒకవేళ (From.nonFiniteBehavior == fltNonfiniteBehavior::IEEE754) {
    // Converting an IEEE signaling NaN to another semantics quiets it, so the
    // original value cannot be recovered by converting it back.
    తిరిగి అబద్ధం;
  }

  // NanOnly formats have no signaling NaNs. IEEE semantics can represent
  // their quiet NaNs; conversions between NanOnly formats are conservatively
  // accepted only when they use the same NaN encoding.
  ఒకవేళ (గమ్యం.nonFiniteBehavior == fltNonfiniteBehavior::IEEE754)
    తిరిగి నిజం;
  తిరిగి గమ్యం.nonFiniteBehavior == fltNonfiniteBehavior::NanOnly &&
         From.nanEncoding == గమ్యం.nanEncoding;
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
[[maybe_వాడని]] constexpr గుర్తులేని maxExponent = 16383;

// Decimal exponents in the f16 and f32 ranges need at most 2 and 6 parts.
// Keep 16 parts (128 bytes) inline: enough for 5^413, but not a large stack
// frame. Longer fractional spellings use heap storage.
స్థిర constexpr గుర్తులేని అంకె_Pow5_Parts_On_దొంతర = 16;

స్థిర గుర్తులేని పూర్ణాంకం power_యొక్క_Five_Parts_For_Exponent(గుర్తులేని పూర్ణాంకం power) {
  ప్రతిజ్ఞా(power <= maxExponent);

  // 815 / 351 is an upper bound on log2(5).  Reserve an additional part for
  // tcFullMultiply, which writes the full product width even when its most
  // significant part is zero.
  తిరిగి 2 + ((power * 815) / (351 * APFloat_పునాది::integer_Part_వెడల్పు));
}

గుర్తులేని పూర్ణాంకం APFloat_పునాది::semanticsPrecision(స్థిరరాశి fltSemantics &semantics) {
  తిరిగి semantics.precision;
}
APFloat_పునాది::ExponentType
APFloat_పునాది::semanticsMaxExponent(స్థిరరాశి fltSemantics &semantics) {
  తిరిగి semantics.maxExponent;
}
APFloat_పునాది::ExponentType
APFloat_పునాది::semanticsMinExponent(స్థిరరాశి fltSemantics &semantics) {
  తిరిగి semantics.minExponent;
}
గుర్తులేని పూర్ణాంకం APFloat_పునాది::semantics_కొలత_లోపల_బిట్లు(స్థిరరాశి fltSemantics &semantics) {
  తిరిగి semantics.కొలత_లోపల_బిట్లు;
}
గుర్తులేని పూర్ణాంకం APFloat_పునాది::semantics_Int_కొలత_లోపల_బిట్లు(స్థిరరాశి fltSemantics &semantics,
                                                 తార్కిక isSigned) {
  // The max FP value is pow(2, MaxExponent) * (1 + MaxFraction), so we need
  // at least one more bit than the MaxExponent to hold the max FP value.
  గుర్తులేని పూర్ణాంకం Min_బిట్టు_వెడల్పు = semanticsMaxExponent(semantics) + 1;
  // Extra sign bit needed.
  ఒకవేళ (isSigned)
    ++Min_బిట్టు_వెడల్పు;
  తిరిగి Min_బిట్టు_వెడల్పు;
}

తార్కిక APFloat_పునాది::semanticsHasZero(స్థిరరాశి fltSemantics &semantics) {
  తిరిగి semantics.hasZero;
}

తార్కిక APFloat_పునాది::semanticsHasSignedRepr(స్థిరరాశి fltSemantics &semantics) {
  తిరిగి semantics.hasSignedRepr;
}

తార్కిక APFloat_పునాది::semanticsHasInf(స్థిరరాశి fltSemantics &semantics) {
  తిరిగి semantics.nonFiniteBehavior == fltNonfiniteBehavior::IEEE754;
}

తార్కిక APFloat_పునాది::semanticsHasNaN(స్థిరరాశి fltSemantics &semantics) {
  తిరిగి semantics.nonFiniteBehavior != fltNonfiniteBehavior::FiniteOnly;
}

తార్కిక APFloat_పునాది::isIEEELikeFP(స్థిరరాశి fltSemantics &semantics) {
  // Keep in sync with Type::isIEEELikeFPTy
  తిరిగి Semantics_గమ్యం_Enum(semantics) <= S_IEEEquad;
}

తార్కిక APFloat_పునాది::has_Sign_బిట్టు_లోపల_MSB(స్థిరరాశి fltSemantics &semantics) {
  తిరిగి semantics.has_Sign_బిట్టు_లోపల_MSB;
}

తార్కిక APFloat_పునాది::is_Representable_As_సాధారణ_లోపల(స్థిరరాశి fltSemantics &Src,
                                            స్థిరరాశి fltSemantics &Dst) {
  // Exponent range must be larger.
  ఒకవేళ (Src.maxExponent >= Dst.maxExponent || Src.minExponent <= Dst.minExponent)
    తిరిగి అబద్ధం;

  // If the mantissa is long enough, the result value could still be denormal
  // with a larger exponent range.
  //
  // FIXME: This condition is probably not accurate but also shouldn't be a
  // practical concern with existing types.
  తిరిగి Dst.precision >= Src.precision;
}

గుర్తులేని APFloat_పునాది::get_కొలత_లోపల_బిట్లు(స్థిరరాశి fltSemantics &Sem) {
  తిరిగి Sem.కొలత_లోపల_బిట్లు;
}

స్థిర constexpr APFloat_పునాది::ExponentType
exponentZero(స్థిరరాశి fltSemantics &semantics) {
  తిరిగి semantics.minExponent - 1;
}

స్థిర constexpr APFloat_పునాది::ExponentType
exponentInf(స్థిరరాశి fltSemantics &semantics) {
  తిరిగి semantics.maxExponent + 1;
}

స్థిర constexpr APFloat_పునాది::ExponentType
exponentNaN(స్థిరరాశి fltSemantics &semantics) {
  ఒకవేళ (semantics.nonFiniteBehavior == fltNonfiniteBehavior::NanOnly) {
    ఒకవేళ (semantics.nanEncoding == fltNanEncoding::NegativeZero)
      తిరిగి exponentZero(semantics);
    ఒకవేళ (semantics.hasSignedRepr || semantics.precision > 1)
      తిరిగి semantics.maxExponent;
  }
  తిరిగి semantics.maxExponent + 1;
}

/* A bunch of private, handy routines.  */

స్థిర అంతఃవరుస Error createError(స్థిరరాశి Twine &Err) {
  తిరిగి make_error<StringError>(Err, inconvertible_Error_సంకేతం());
}

స్థిర constexpr అంతఃవరుస గుర్తులేని పూర్ణాంకం part_లెక్క_For_బిట్లు(గుర్తులేని పూర్ణాంకం బిట్లు) {
  తిరిగి std::max(1u, (బిట్లు + APFloat_పునాది::integer_Part_వెడల్పు - 1) /
                          APFloat_పునాది::integer_Part_వెడల్పు);
}

/* Returns 0U-9U.  Return values >= 10U are not digits.  */
స్థిర అంతఃవరుస గుర్తులేని పూర్ణాంకం
dec_Digit_విలువ(గుర్తులేని పూర్ణాంకం c)
{
  తిరిగి c - '0';
}

/* Return the value of a decimal exponent of the form
   [+-]ddddddd.

   If the exponent overflows, returns a large exponent with the
   appropriate sign.  */
స్థిర Expected<పూర్ణాంకం> readExponent(StringRef::iterator begin,
                                  StringRef::iterator చివర) {
  స్థిరరాశి గుర్తులేని పూర్ణాంకం overlargeExponent = 24000;  /* FIXME.  */
  StringRef::iterator p = begin;

  // Treat no exponent as 0 to match binutils
  ఒకవేళ (p == చివర || ((*p == '-' || *p == '+') && (p + 1) == చివర))
    తిరిగి 0;

  తార్కిక isNegative = *p == '-';
  ఒకవేళ (*p == '-' || *p == '+') {
    p++;
    ఒకవేళ (p == చివర)
      తిరిగి createError("Exponent has no digits");
  }

  గుర్తులేని absExponent = dec_Digit_విలువ(*p++);
  ఒకవేళ (absExponent >= 10U)
    తిరిగి createError("Invalid character in exponent");

  కొరకు (; p != చివర; ++p) {
    గుర్తులేని విలువ_2 = dec_Digit_విలువ(*p);
    ఒకవేళ (విలువ_2 >= 10U)
      తిరిగి createError("Invalid character in exponent");

    absExponent = absExponent * 10U + విలువ_2;
    ఒకవేళ (absExponent >= overlargeExponent) {
      absExponent = overlargeExponent;
      ఆపు;
    }
  }

  ఒకవేళ (isNegative)
    తిరిగి -(పూర్ణాంకం) absExponent;
  లేకపోతే
    తిరిగి (పూర్ణాంకం) absExponent;
}

/* This is ugly and needs cleaning up, but I don't immediately see
   how whilst remaining safe.  */
స్థిర Expected<పూర్ణాంకం> మొత్తం_Exponent(StringRef::iterator p,
                                   StringRef::iterator చివర,
                                   పూర్ణాంకం exponentAdjustment) {
  పూర్ణాంకం exponent = 0;

  ఒకవేళ (p == చివర)
    తిరిగి createError("Exponent has no digits");

  తార్కిక negative = *p == '-';
  ఒకవేళ (*p == '-' || *p == '+') {
    p++;
    ఒకవేళ (p == చివర)
      తిరిగి createError("Exponent has no digits");
  }

  పూర్ణాంకం unsignedExponent = 0;
  తార్కిక అధికప్రవాహం_2 = అబద్ధం;
  కొరకు (; p != చివర; ++p) {
    గుర్తులేని పూర్ణాంకం విలువ_2;

    విలువ_2 = dec_Digit_విలువ(*p);
    ఒకవేళ (విలువ_2 >= 10U)
      తిరిగి createError("Invalid character in exponent");

    unsignedExponent = unsignedExponent * 10 + విలువ_2;
    ఒకవేళ (unsignedExponent > 32767) {
      అధికప్రవాహం_2 = నిజం;
      ఆపు;
    }
  }

  ఒకవేళ (exponentAdjustment > 32767 || exponentAdjustment < -32768)
    అధికప్రవాహం_2 = నిజం;

  ఒకవేళ (!అధికప్రవాహం_2) {
    exponent = unsignedExponent;
    ఒకవేళ (negative)
      exponent = -exponent;
    exponent += exponentAdjustment;
    ఒకవేళ (exponent > 32767 || exponent < -32768)
      అధికప్రవాహం_2 = నిజం;
  }

  ఒకవేళ (అధికప్రవాహం_2)
    exponent = negative ? -32768: 32767;

  తిరిగి exponent;
}

స్థిర Expected<StringRef::iterator>
skipLeadingZeroesAndAnyDot(StringRef::iterator begin, StringRef::iterator చివర,
                           StringRef::iterator *dot) {
  StringRef::iterator p = begin;
  *dot = చివర;
  అయితే (p != చివర && *p == '0')
    p++;

  ఒకవేళ (p != చివర && *p == '.') {
    *dot = p++;

    ఒకవేళ (చివర - begin == 1)
      తిరిగి createError("Significand has no digits");

    అయితే (p != చివర && *p == '0')
      p++;
  }

  తిరిగి p;
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
నిర్మాణం decimalInfo {
  స్థిరరాశి అక్షరం *మొదటి_Sig_Digit;
  స్థిరరాశి అక్షరం *చివరి_Sig_Digit;
  పూర్ణాంకం exponent;
  పూర్ణాంకం normalizedExponent;
};

స్థిర Error interpretDecimal(StringRef::iterator begin,
                              StringRef::iterator చివర, decimalInfo *D) {
  StringRef::iterator dot = చివర;

  స్వయంచాలక PtrOrErr = skipLeadingZeroesAndAnyDot(begin, చివర, &dot);
  ఒకవేళ (!PtrOrErr)
    తిరిగి PtrOrErr.takeError();
  StringRef::iterator p = *PtrOrErr;

  D->మొదటి_Sig_Digit = p;
  D->exponent = 0;
  D->normalizedExponent = 0;

  కొరకు (; p != చివర; ++p) {
    ఒకవేళ (*p == '.') {
      ఒకవేళ (dot != చివర)
        తిరిగి createError("String contains multiple dots");
      dot = p++;
      ఒకవేళ (p == చివర)
        ఆపు;
    }
    ఒకవేళ (dec_Digit_విలువ(*p) >= 10U)
      ఆపు;
  }

  ఒకవేళ (p != చివర) {
    ఒకవేళ (*p != 'e' && *p != 'E')
      తిరిగి createError("Invalid character in significand");
    ఒకవేళ (p == begin)
      తిరిగి createError("Significand has no digits");
    ఒకవేళ (dot != చివర && p - begin == 1)
      తిరిగి createError("Significand has no digits");

    /* p points to the first non-digit in the string */
    స్వయంచాలక ExpOrErr = readExponent(p + 1, చివర);
    ఒకవేళ (!ExpOrErr)
      తిరిగి ExpOrErr.takeError();
    D->exponent = *ExpOrErr;

    /* Implied decimal point?  */
    ఒకవేళ (dot == చివర)
      dot = p;
  }

  /* If number is all zeroes accept any exponent.  */
  ఒకవేళ (p != D->మొదటి_Sig_Digit) {
    /* Drop insignificant trailing zeroes.  */
    ఒకవేళ (p != begin) {
      చేయి
        చేయి
          p--;
        అయితే (p != begin && *p == '0');
      అయితే (p != begin && *p == '.');
    }

    /* Adjust the exponents for any decimal point.  */
    D->exponent += static_cast<APFloat::ExponentType>((dot - p) - (dot > p));
    D->normalizedExponent = (D->exponent +
              static_cast<APFloat::ExponentType>((p - D->మొదటి_Sig_Digit)
                                      - (dot > D->మొదటి_Sig_Digit && dot < p)));
  }

  D->చివరి_Sig_Digit = p;
  తిరిగి Error::success();
}

/* Return the trailing fraction of a hexadecimal number.
   DIGITVALUE is the first hex digit of the fraction, P points to
   the next digit.  */
స్థిర Expected<కోల్పోయిన_Fraction>
trailingHexadecimalFraction(StringRef::iterator p, StringRef::iterator చివర,
                            గుర్తులేని పూర్ణాంకం digit_విలువ) {
  /* If the first trailing digit isn't 0 or 8 we can work out the
     fraction immediately.  */
  ఒకవేళ (digit_విలువ > 8)
    తిరిగి lf_మరిన్ని_Than_Half;
  లేకపోతే ఒకవేళ (digit_విలువ < 8 && digit_విలువ > 0)
    తిరిగి lfLessThanHalf;

  // Otherwise we need to find the first non-zero digit.
  అయితే (p != చివర && (*p == '0' || *p == '.'))
    p++;

  ఒకవేళ (p == చివర)
    తిరిగి createError("Invalid trailing hexadecimal fraction!");

  గుర్తులేని hexDigit = hex_Digit_విలువ(*p);

  /* If we ran off the end it is exactly zero or one-half, otherwise
     a little more.  */
  ఒకవేళ (hexDigit == UINT_MAX)
    తిరిగి digit_విలువ == 0 ? lfExactlyZero: lfExactlyHalf;
  లేకపోతే
    తిరిగి digit_విలువ == 0 ? lfLessThanHalf: lf_మరిన్ని_Than_Half;
}

/* Return the fraction lost were a bignum truncated losing the least
   significant BITS bits.  */
స్థిర కోల్పోయిన_Fraction
కోల్పోయిన_Fraction_Through_Truncation(స్థిరరాశి APFloat_పునాది::integerPart *parts,
                              గుర్తులేని పూర్ణాంకం part_లెక్క,
                              గుర్తులేని పూర్ణాంకం బిట్లు)
{
  గుర్తులేని lsb = APInt::tcLSB(parts, part_లెక్క);

  /* Note this is guaranteed true if bits == 0, or LSB == UINT_MAX.  */
  ఒకవేళ (బిట్లు <= lsb)
    తిరిగి lfExactlyZero;
  ఒకవేళ (బిట్లు == lsb + 1)
    తిరిగి lfExactlyHalf;
  ఒకవేళ (బిట్లు <= part_లెక్క * APFloat_పునాది::integer_Part_వెడల్పు &&
      APInt::tc_వెలికితీయి_బిట్టు(parts, బిట్లు - 1))
    తిరిగి lf_మరిన్ని_Than_Half;

  తిరిగి lfLessThanHalf;
}

/* Shift DST right BITS bits noting lost fraction.  */
స్థిర కోల్పోయిన_Fraction
జరుపు_కుడి(APFloat_పునాది::integerPart *dst, గుర్తులేని పూర్ణాంకం parts, గుర్తులేని పూర్ణాంకం బిట్లు)
{
  కోల్పోయిన_Fraction కోల్పోయిన_fraction = కోల్పోయిన_Fraction_Through_Truncation(dst, parts, బిట్లు);

  APInt::tc_జరుపు_కుడి(dst, parts, బిట్లు);

  తిరిగి కోల్పోయిన_fraction;
}

/* Combine the effect of two lost fractions.  */
స్థిర కోల్పోయిన_Fraction
combine_కోల్పోయిన_Fractions(కోల్పోయిన_Fraction మరిన్ని_Significant,
                     కోల్పోయిన_Fraction lessSignificant)
{
  ఒకవేళ (lessSignificant != lfExactlyZero) {
    ఒకవేళ (మరిన్ని_Significant == lfExactlyZero)
      మరిన్ని_Significant = lfLessThanHalf;
    లేకపోతే ఒకవేళ (మరిన్ని_Significant == lfExactlyHalf)
      మరిన్ని_Significant = lf_మరిన్ని_Than_Half;
  }

  తిరిగి మరిన్ని_Significant;
}

/* The error from the true value, in half-ulps, on multiplying two
   floating point numbers, which differ from the value they
   approximate by at most HUE1 and HUE2 half-ulps, is strictly less
   than the returned value.

   See "How to Read Floating Point Numbers Accurately" by William D
   Clinger.  */
స్థిర గుర్తులేని పూర్ణాంకం
HUerrBound(తార్కిక inexactMultiply, గుర్తులేని పూర్ణాంకం HUerr1, గుర్తులేని పూర్ణాంకం HUerr2)
{
  ప్రతిజ్ఞా(HUerr1 < 2 || HUerr2 < 2 || (HUerr1 + HUerr2 < 8));

  ఒకవేళ (HUerr1 + HUerr2 == 0)
    తిరిగి inexactMultiply * 2;  /* <= inexactMultiply half-ulps.  */
  లేకపోతే
    తిరిగి inexactMultiply + 2 * (HUerr1 + HUerr2);
}

/* The number of ulps from the boundary (zero, or half if ISNEAREST)
   when the least significant BITS are truncated.  BITS cannot be
   zero.  */
స్థిర APFloat_పునాది::integerPart
ulpsFromBoundary(స్థిరరాశి APFloat_పునాది::integerPart *parts, గుర్తులేని పూర్ణాంకం బిట్లు,
                 తార్కిక is_సమీప) {
  ప్రతిజ్ఞా(బిట్లు != 0);

  బిట్లు--;
  గుర్తులేని లెక్క = బిట్లు / APFloat_పునాది::integer_Part_వెడల్పు;
  గుర్తులేని part_బిట్లు = బిట్లు % APFloat_పునాది::integer_Part_వెడల్పు + 1;

  APFloat_పునాది::integerPart part =
      parts[లెక్క] & (~(APFloat_పునాది::integerPart)0 >>
                      (APFloat_పునాది::integer_Part_వెడల్పు - part_బిట్లు));

  APFloat_పునాది::integerPart boundary;
  ఒకవేళ (is_సమీప)
    boundary = (APFloat_పునాది::integerPart) 1 << (part_బిట్లు - 1);
  లేకపోతే
    boundary = 0;

  ఒకవేళ (లెక్క == 0) {
    ఒకవేళ (part - boundary <= boundary - part)
      తిరిగి part - boundary;
    లేకపోతే
      తిరిగి boundary - part;
  }

  ఒకవేళ (part == boundary) {
    అయితే (--లెక్క)
      ఒకవేళ (parts[లెక్క])
        తిరిగి ~(APFloat_పునాది::integerPart) 0; /* A lot.  */

    తిరిగి parts[0];
  } లేకపోతే ఒకవేళ (part == boundary - 1) {
    అయితే (--లెక్క)
      ఒకవేళ (~parts[లెక్క])
        తిరిగి ~(APFloat_పునాది::integerPart) 0; /* A lot.  */

    తిరిగి -parts[0];
  }

  తిరిగి ~(APFloat_పునాది::integerPart) 0; /* A lot.  */
}

స్థిర constexpr గుర్తులేని Power_యొక్క_Five_Part_Counts[] = {1,  1,  2,  3,   5,  10,
                                                     19, 38, 75, 149, 298};

స్థిర constexpr APFloat_పునాది::integerPart Power_యొక్క_Five_Parts[] = {
#అన్తర్భావ "APFloatPowerOfFiveTable.inc"
};

static_assert([] {
  గుర్తులేని కూడిక = 0;
  కొరకు (గుర్తులేని లెక్క_2 : Power_యొక్క_Five_Part_Counts)
    కూడిక += లెక్క_2;
  తిరిగి కూడిక;
}() == std::కొలత_2(Power_యొక్క_Five_Parts));

/* Place pow(5, power) in DST, and return the number of parts used.
   DST must be at least one part larger than size of the answer.  */
స్థిర గుర్తులేని పూర్ణాంకం
powerOf5(APFloat_పునాది::integerPart *dst, గుర్తులేని పూర్ణాంకం power) {
  స్థిర స్థిరరాశి APFloat_పునాది::integerPart మొదటి_Eight_Powers[] = {
      1, 5, 25, 125, 625, 3125, 15625, 78125};

  Small_సదిశ<APFloat_పునాది::integerPart, అంకె_Pow5_Parts_On_దొంతర> scratch;
  scratch.resize_for_overwrite(power_యొక్క_Five_Parts_For_Exponent(power));

  APFloat_పునాది::integerPart *p1, *p2;
  స్థిరరాశి APFloat_పునాది::integerPart *pow5;

  p1 = dst;
  p2 = scratch.దత్తాంశం();

  *p1 = మొదటి_Eight_Powers[power & 7];
  power >>= 3;

  గుర్తులేని ఫలితం_2 = 1;
  pow5 = Power_యొక్క_Five_Parts;

  కొరకు (గుర్తులేని పూర్ణాంకం n = 0; power; power >>= 1, n++) {
    గుర్తులేని parts_లెక్క = Power_యొక్క_Five_Part_Counts[n];

    ఒకవేళ (power & 1) {
      APFloat_పునాది::integerPart *tmp;

      APInt::tc_నిండిన_Multiply(p2, p1, pow5, ఫలితం_2, parts_లెక్క);
      ఫలితం_2 += parts_లెక్క;
      ఒకవేళ (p2[ఫలితం_2 - 1] == 0)
        ఫలితం_2--;

      /* Now result is in p1 with partsCount parts and p2 is scratch
         space.  */
      tmp = p1;
      p1 = p2;
      p2 = tmp;
    }

    pow5 += parts_లెక్క;
  }

  ఒకవేళ (p1 != dst)
    APInt::tc_కేటాయించు(dst, p1, ఫలితం_2);

  తిరిగి ఫలితం_2;
}

/* Zero at the end to avoid modular arithmetic when adding one; used
   when rounding up during hexadecimal output.  */
స్థిర స్థిరరాశి అక్షరం hex_Digits_క్రింది[] = "0123456789abcdef0";
స్థిర స్థిరరాశి అక్షరం hex_Digits_పై[] = "0123456789ABCDEF0";
స్థిర స్థిరరాశి అక్షరం infinityL[] = "infinity";
స్థిర స్థిరరాశి అక్షరం infinityU[] = "INFINITY";
స్థిర స్థిరరాశి అక్షరం NaNL[] = "nan";
స్థిర స్థిరరాశి అక్షరం NaNU[] = "NAN";

/* Write out an integerPart in hexadecimal, starting with the most
   significant nibble.  Write out exactly COUNT hexdigits, return
   COUNT.  */
స్థిర గుర్తులేని పూర్ణాంకం
partAsHex (అక్షరం *dst, APFloat_పునాది::integerPart part, గుర్తులేని పూర్ణాంకం లెక్క,
           స్థిరరాశి అక్షరం *hexDigitChars)
{
  గుర్తులేని పూర్ణాంకం ఫలితం_2 = లెక్క;

  ప్రతిజ్ఞా(లెక్క != 0 && లెక్క <= APFloat_పునాది::integer_Part_వెడల్పు / 4);

  part >>= (APFloat_పునాది::integer_Part_వెడల్పు - 4 * లెక్క);
  అయితే (లెక్క--) {
    dst[లెక్క] = hexDigitChars[part & 0xf];
    part >>= 4;
  }

  తిరిగి ఫలితం_2;
}

/* Write out an unsigned decimal integer.  */
స్థిర అక్షరం *writeUnsignedDecimal(అక్షరం *dst, గుర్తులేని పూర్ణాంకం n) {
  అక్షరం buff[40], *p;

  p = buff;
  చేయి
    *p++ = '0' + n % 10;
  అయితే (n /= 10);

  చేయి
    *dst++ = *--p;
  అయితే (p != buff);

  తిరిగి dst;
}

/* Write out a signed decimal integer.  */
స్థిర అక్షరం *writeSignedDecimal(అక్షరం *dst, పూర్ణాంకం విలువ_2) {
  ఒకవేళ (విలువ_2 < 0) {
    *dst++ = '-';
    dst = writeUnsignedDecimal(dst, -(గుర్తులేని) విలువ_2);
  } లేకపోతే {
    dst = writeUnsignedDecimal(dst, విలువ_2);
  }

  తిరిగి dst;
}

// Compute the ULP of the input using a definition from:
// Jean-Michel Muller. On the definition of ulp(x). [Research Report] RR-5504,
// LIP RR-2005-09, INRIA, LIP. 2005, pp.16. inria-00070503
స్థిర APFloat harrisonUlp(స్థిరరాశి APFloat &X) {
  స్థిరరాశి fltSemantics &Sem = X.getSemantics();
  ఎంపిక (X.getCategory()) {
  స్థితి APFloat::fcNaN:
    తిరిగి APFloat::getQNaN(Sem);
  స్థితి APFloat::fcInfinity:
    తిరిగి APFloat::getInf(Sem);
  స్థితి APFloat::fcZero:
    తిరిగి APFloat::get_అతిచిన్న(Sem);
  స్థితి APFloat::fc_సాధారణ:
    ఆపు;
  }
  ఒకవేళ (X.isDenormal() || X.is_అతిచిన్న_Normalized())
    తిరిగి APFloat::get_అతిచిన్న(Sem);
  పూర్ణాంకం Exp = ilogb(X);
  ఒకవేళ (X.getExactLog2() != INT_MIN)
    Exp -= 1;
  తిరిగి scalbn(APFloat::getOne(Sem), Exp - (Sem.precision - 1),
                APFloat::rm_సమీప_Ties_గమ్యం_Even);
}

నామాకాశ detail {
/* Constructors.  */
శూన్య IEEEFloat::initialize(స్థిరరాశి fltSemantics *ourSemantics) {
  semantics = ourSemantics;
  గుర్తులేని లెక్క = part_లెక్క();
  ఒకవేళ (లెక్క > 1)
    significand.parts = కొత్త integerPart[లెక్క];
}

శూన్య IEEEFloat::freeSignificand() {
  ఒకవేళ (needsCleanup())
    తొలగించు [] significand.parts;
}

శూన్య IEEEFloat::కేటాయించు(స్థిరరాశి IEEEFloat &rhs) {
  ప్రతిజ్ఞా(semantics == rhs.semantics);

  sign = rhs.sign;
  category = rhs.category;
  exponent = rhs.exponent;
  ఒకవేళ (isFiniteNonZero() || category == fcNaN)
    copySignificand(rhs);
}

శూన్య IEEEFloat::copySignificand(స్థిరరాశి IEEEFloat &rhs) {
  ప్రతిజ్ఞా(isFiniteNonZero() || category == fcNaN);
  ప్రతిజ్ఞా(rhs.part_లెక్క() >= part_లెక్క());

  APInt::tc_కేటాయించు(significandParts(), rhs.significandParts(),
                  part_లెక్క());
}

/* Make this number a NaN, with an arbitrary but deterministic value
   for the significand.  If double or longer, this is a signalling NaN,
   which may not be ideal.  If float, this is QNaN(0).  */
శూన్య IEEEFloat::makeNaN(తార్కిక SNaN, తార్కిక Negative, స్థిరరాశి APInt *fill) {
  ఒకవేళ (semantics->nonFiniteBehavior == fltNonfiniteBehavior::FiniteOnly)
    llvm_unreachable("This floating point format does not support NaN");

  ఒకవేళ (Negative && !semantics->hasSignedRepr)
    llvm_unreachable(
        "This floating point format does not support signed values");

  category = fcNaN;
  sign = Negative;
  exponent = exponentNaN();

  integerPart *significand = significandParts();
  గుర్తులేని అంకె_Parts = part_లెక్క();

  APInt fill_storage;
  ఒకవేళ (semantics->nonFiniteBehavior == fltNonfiniteBehavior::NanOnly) {
    // Finite-only types do not distinguish signalling and quiet NaN, so
    // make them all signalling.
    SNaN = అబద్ధం;
    ఒకవేళ (semantics->nanEncoding == fltNanEncoding::NegativeZero) {
      sign = నిజం;
      fill_storage = APInt::getZero(semantics->precision - 1);
    } లేకపోతే {
      fill_storage = APInt::getAllOnes(semantics->precision - 1);
    }
    fill = &fill_storage;
  }

  // Set the significand bits to the fill.
  ఒకవేళ (!fill || fill->get_అంకె_పదాలు() < అంకె_Parts)
    APInt::tc_సమితి(significand, 0, అంకె_Parts);
  ఒకవేళ (fill) {
    APInt::tc_కేటాయించు(significand, fill->get_Raw_దత్తాంశం(),
                    std::min(fill->get_అంకె_పదాలు(), అంకె_Parts));

    // Zero out the excess bits of the significand.
    గుర్తులేని బిట్లు_గమ్యం_Preserve = semantics->precision - 1;
    గుర్తులేని part = బిట్లు_గమ్యం_Preserve / 64;
    బిట్లు_గమ్యం_Preserve %= 64;
    significand[part] &= ((1ULL << బిట్లు_గమ్యం_Preserve) - 1);
    కొరకు (part++; part != అంకె_Parts; ++part)
      significand[part] = 0;
  }

  గుర్తులేని QNaNBit =
      (semantics->precision >= 2) ? (semantics->precision - 2) : 0;

  ఒకవేళ (SNaN) {
    // We always have to clear the QNaN bit to make it an SNaN.
    APInt::tc_Clear_బిట్టు(significand, QNaNBit);

    // If there are no bits set in the payload, we have to set
    // *something* to make it a NaN instead of an infinity;
    // conventionally, this is the next bit down from the QNaN bit.
    ఒకవేళ (APInt::tcIsZero(significand, అంకె_Parts))
      APInt::tc_సమితి_బిట్టు(significand, QNaNBit - 1);
  } లేకపోతే ఒకవేళ (semantics->nanEncoding == fltNanEncoding::NegativeZero) {
    // The only NaN is a quiet NaN, and it has no bits sets in the significand.
    // Do nothing.
  } లేకపోతే {
    // We always have to set the QNaN bit to make it a QNaN.
    APInt::tc_సమితి_బిట్టు(significand, QNaNBit);
  }

  // For x87 extended precision, we want to make a NaN, not a
  // pseudo-NaN.  Maybe we should expose the ability to make
  // pseudo-NaNs?
  ఒకవేళ (semantics == &APFloat_పునాది::semX87DoubleExtended)
    APInt::tc_సమితి_బిట్టు(significand, QNaNBit + 1);
}

IEEEFloat &IEEEFloat::క్రియాకారకం=(స్థిరరాశి IEEEFloat &rhs) {
  ఒకవేళ (ఇది != &rhs) {
    ఒకవేళ (semantics != rhs.semantics) {
      freeSignificand();
      initialize(rhs.semantics);
    }
    కేటాయించు(rhs);
  }

  తిరిగి *ఇది;
}

IEEEFloat &IEEEFloat::క్రియాకారకం=(IEEEFloat &&rhs) {
  freeSignificand();

  semantics = rhs.semantics;
  significand = rhs.significand;
  exponent = rhs.exponent;
  category = rhs.category;
  sign = rhs.sign;

  rhs.semantics = &APFloat_పునాది::semBogus;
  తిరిగి *ఇది;
}

తార్కిక IEEEFloat::isDenormal() స్థిరరాశి {
  తిరిగి getSemantics().hasDenormals && isFiniteNonZero() &&
         (exponent == semantics->minExponent) &&
         (APInt::tc_వెలికితీయి_బిట్టు(significandParts(), semantics->precision - 1) ==
          0);
}

తార్కిక IEEEFloat::is_అతిచిన్న() స్థిరరాశి {
  // The smallest number by magnitude in our format will be the smallest
  // denormal, i.e. the floating point number with exponent being minimum
  // exponent and significand bitwise equal to 1 (i.e. with MSB equal to 0).
  తిరిగి isFiniteNonZero() && exponent == semantics->minExponent &&
    significandMSB() == 0;
}

తార్కిక IEEEFloat::is_అతిచిన్న_Normalized() స్థిరరాశి {
  తిరిగి getCategory() == fc_సాధారణ && exponent == semantics->minExponent &&
         isSignificandAllZerosExceptMSB();
}

గుర్తులేని పూర్ణాంకం IEEEFloat::get_అంకె_ఎగువ_బిట్లు() స్థిరరాశి {
  స్థిరరాశి గుర్తులేని పూర్ణాంకం Part_లెక్క = part_లెక్క_For_బిట్లు(semantics->precision);
  స్థిరరాశి గుర్తులేని పూర్ణాంకం బిట్లు_2 = Part_లెక్క * integer_Part_వెడల్పు;

  // Compute how many bits are used in the final word.
  // When precision is just 1, it represents the 'Pth'
  // Precision bit and not the actual significand bit.
  స్థిరరాశి గుర్తులేని పూర్ణాంకం అంకె_ఎగువ_బిట్లు = (semantics->precision > 1)
                                       ? (బిట్లు_2 - semantics->precision + 1)
                                       : (బిట్లు_2 - semantics->precision);
  తిరిగి అంకె_ఎగువ_బిట్లు;
}

తార్కిక IEEEFloat::isSignificandAllOnes() స్థిరరాశి {
  // Test if the significand excluding the integral bit is all ones. This allows
  // us to test for binade boundaries.
  స్థిరరాశి integerPart *Parts = significandParts();
  స్థిరరాశి గుర్తులేని Part_లెక్క = part_లెక్క_For_బిట్లు(semantics->precision);
  కొరకు (గుర్తులేని i = 0; i < Part_లెక్క - 1; i++)
    ఒకవేళ (~Parts[i])
      తిరిగి అబద్ధం;

  // Set the unused high bits to all ones when we compare.
  స్థిరరాశి గుర్తులేని అంకె_ఎగువ_బిట్లు = get_అంకె_ఎగువ_బిట్లు();
  ప్రతిజ్ఞా(అంకె_ఎగువ_బిట్లు <= integer_Part_వెడల్పు && అంకె_ఎగువ_బిట్లు > 0 &&
         "Can not have more high bits to fill than integerPartWidth");
  స్థిరరాశి integerPart ఎగువ_బిట్టు_Fill =
    ~integerPart(0) << (integer_Part_వెడల్పు - అంకె_ఎగువ_బిట్లు);
  ఒకవేళ ((semantics->precision <= 1) || (~(Parts[Part_లెక్క - 1] | ఎగువ_బిట్టు_Fill)))
    తిరిగి అబద్ధం;

  తిరిగి నిజం;
}

తార్కిక IEEEFloat::isSignificandAllOnesExceptLSB() స్థిరరాశి {
  // Test if the significand excluding the integral bit is all ones except for
  // the least significant bit.
  స్థిరరాశి integerPart *Parts = significandParts();

  ఒకవేళ (Parts[0] & 1)
    తిరిగి అబద్ధం;

  స్థిరరాశి గుర్తులేని Part_లెక్క = part_లెక్క_For_బిట్లు(semantics->precision);
  కొరకు (గుర్తులేని i = 0; i < Part_లెక్క - 1; i++) {
    ఒకవేళ (~Parts[i] & ~గుర్తులేని{!i})
      తిరిగి అబద్ధం;
  }

  // Set the unused high bits to all ones when we compare.
  స్థిరరాశి గుర్తులేని అంకె_ఎగువ_బిట్లు = get_అంకె_ఎగువ_బిట్లు();
  ప్రతిజ్ఞా(అంకె_ఎగువ_బిట్లు <= integer_Part_వెడల్పు && అంకె_ఎగువ_బిట్లు > 0 &&
         "Can not have more high bits to fill than integerPartWidth");
  స్థిరరాశి integerPart ఎగువ_బిట్టు_Fill = ~integerPart(0)
                                  << (integer_Part_వెడల్పు - అంకె_ఎగువ_బిట్లు);
  ఒకవేళ (~(Parts[Part_లెక్క - 1] | ఎగువ_బిట్టు_Fill | 0x1))
    తిరిగి అబద్ధం;

  తిరిగి నిజం;
}

తార్కిక IEEEFloat::isSignificandAllZeros() స్థిరరాశి {
  // Test if the significand excluding the integral bit is all zeros. This
  // allows us to test for binade boundaries.
  స్థిరరాశి integerPart *Parts = significandParts();
  స్థిరరాశి గుర్తులేని Part_లెక్క = part_లెక్క_For_బిట్లు(semantics->precision);

  కొరకు (గుర్తులేని i = 0; i < Part_లెక్క - 1; i++)
    ఒకవేళ (Parts[i])
      తిరిగి అబద్ధం;

  // Compute how many bits are used in the final word.
  స్థిరరాశి గుర్తులేని అంకె_ఎగువ_బిట్లు = get_అంకె_ఎగువ_బిట్లు();
  ప్రతిజ్ఞా(అంకె_ఎగువ_బిట్లు < integer_Part_వెడల్పు && "Can not have more high bits to "
         "clear than integerPartWidth");
  స్థిరరాశి integerPart ఎగువ_బిట్టు_ముసుగు = ~integerPart(0) >> అంకె_ఎగువ_బిట్లు;

  ఒకవేళ ((semantics->precision > 1) && (Parts[Part_లెక్క - 1] & ఎగువ_బిట్టు_ముసుగు))
    తిరిగి అబద్ధం;

  తిరిగి నిజం;
}

తార్కిక IEEEFloat::isSignificandAllZerosExceptMSB() స్థిరరాశి {
  స్థిరరాశి integerPart *Parts = significandParts();
  స్థిరరాశి గుర్తులేని Part_లెక్క = part_లెక్క_For_బిట్లు(semantics->precision);

  కొరకు (గుర్తులేని i = 0; i < Part_లెక్క - 1; i++) {
    ఒకవేళ (Parts[i])
      తిరిగి అబద్ధం;
  }

  స్థిరరాశి గుర్తులేని అంకె_ఎగువ_బిట్లు = get_అంకె_ఎగువ_బిట్లు();
  స్థిరరాశి integerPart MSBMask = integerPart(1)
                              << (integer_Part_వెడల్పు - అంకె_ఎగువ_బిట్లు);
  తిరిగి ((semantics->precision <= 1) || (Parts[Part_లెక్క - 1] == MSBMask));
}

తార్కిక IEEEFloat::is_అతిపెద్ద() స్థిరరాశి {
  తార్కిక IsMaxExp = isFiniteNonZero() && exponent == semantics->maxExponent;
  ఒకవేళ (semantics->nonFiniteBehavior == fltNonfiniteBehavior::NanOnly &&
      semantics->nanEncoding == fltNanEncoding::AllOnes) {
    // The largest number by magnitude in our format will be the floating point
    // number with maximum exponent and with significand that is all ones except
    // the LSB.
    తిరిగి (IsMaxExp && APFloat::hasSignificand(*semantics))
               ? isSignificandAllOnesExceptLSB()
               : IsMaxExp;
  } లేకపోతే {
    // The largest number by magnitude in our format will be the floating point
    // number with maximum exponent and with significand that is all ones.
    తిరిగి IsMaxExp && isSignificandAllOnes();
  }
}

తార్కిక IEEEFloat::isInteger() స్థిరరాశి {
  // This could be made more efficient; I'm going for obviously correct.
  ఒకవేళ (!isFinite()) తిరిగి అబద్ధం;
  IEEEFloat truncated = *ఇది;
  truncated.విడత_గమ్యం_Integral(rmTowardZero);
  తిరిగి compare(truncated) == cmpEqual;
}

తార్కిక IEEEFloat::bitwiseIsEqual(స్థిరరాశి IEEEFloat &rhs) స్థిరరాశి {
  ఒకవేళ (ఇది == &rhs)
    తిరిగి నిజం;
  ఒకవేళ (semantics != rhs.semantics ||
      category != rhs.category ||
      sign != rhs.sign)
    తిరిగి అబద్ధం;
  ఒకవేళ (category==fcZero || category==fcInfinity)
    తిరిగి నిజం;

  ఒకవేళ (isFiniteNonZero() && exponent != rhs.exponent)
    తిరిగి అబద్ధం;

  తిరిగి std::equal(significandParts(), significandParts() + part_లెక్క(),
                    rhs.significandParts());
}

IEEEFloat::IEEEFloat(స్థిరరాశి fltSemantics &ourSemantics, integerPart విలువ_2) {
  initialize(&ourSemantics);
  sign = 0;
  category = fc_సాధారణ;
  zeroSignificand();
  exponent = ourSemantics.precision - 1;
  significandParts()[0] = విలువ_2;
  normalize(rm_సమీప_Ties_గమ్యం_Even, lfExactlyZero);
}

IEEEFloat::IEEEFloat(స్థిరరాశి fltSemantics &ourSemantics) {
  initialize(&ourSemantics);
  // The Float8E8MOFNU format does not have a representation
  // for zero. So, use the closest representation instead.
  // Moreover, the all-zero encoding represents a valid
  // normal value (which is the smallestNormalized here).
  // Hence, we call makeSmallestNormalized (where category is
  // 'fcNormal') instead of makeZero (where category is 'fcZero').
  ourSemantics.hasZero ? makeZero(అబద్ధం) : make_అతిచిన్న_Normalized(అబద్ధం);
}

// Delegate to the previous constructor, because later copy constructor may
// actually inspects category, which can't be garbage.
IEEEFloat::IEEEFloat(స్థిరరాశి fltSemantics &ourSemantics, uninitialized_ట్యాగ్ ట్యాగ్_2)
    : IEEEFloat(ourSemantics) {}

IEEEFloat::IEEEFloat(స్థిరరాశి IEEEFloat &rhs) {
  initialize(rhs.semantics);
  కేటాయించు(rhs);
}

IEEEFloat::IEEEFloat(IEEEFloat &&rhs) : semantics(&APFloat_పునాది::semBogus) {
  *ఇది = std::move(rhs);
}

IEEEFloat::~IEEEFloat() { freeSignificand(); }

గుర్తులేని పూర్ణాంకం IEEEFloat::part_లెక్క() స్థిరరాశి {
  తిరిగి part_లెక్క_For_బిట్లు(semantics->precision + 1);
}

స్థిరరాశి APFloat::integerPart *IEEEFloat::significandParts() స్థిరరాశి {
  తిరిగి const_cast<IEEEFloat *>(ఇది)->significandParts();
}

APFloat::integerPart *IEEEFloat::significandParts() {
  ఒకవేళ (part_లెక్క() > 1)
    తిరిగి significand.parts;
  లేకపోతే
    తిరిగి &significand.part;
}

శూన్య IEEEFloat::zeroSignificand() {
  APInt::tc_సమితి(significandParts(), 0, part_లెక్క());
}

/* Increment an fcNormal floating point number's significand.  */
శూన్య IEEEFloat::incrementSignificand() {
  [[maybe_వాడని]] integerPart వాహకం =
      APInt::tcIncrement(significandParts(), part_లెక్క());

  /* Our callers should never cause us to overflow.  */
  ప్రతిజ్ఞా(వాహకం == 0);
}

/* Add the significand of the RHS.  Returns the carry flag.  */
APFloat::integerPart IEEEFloat::కూడు_Significand(స్థిరరాశి IEEEFloat &rhs) {
  integerPart *parts = significandParts();

  ప్రతిజ్ఞా(semantics == rhs.semantics);
  ప్రతిజ్ఞా(exponent == rhs.exponent);

  తిరిగి APInt::tc_కూడు(parts, rhs.significandParts(), 0, part_లెక్క());
}

/* Subtract the significand of the RHS with a borrow flag.  Returns
   the borrow flag.  */
APFloat::integerPart IEEEFloat::వ్యవకలనం_Significand(స్థిరరాశి IEEEFloat &rhs,
                                                    integerPart borrow) {
  integerPart *parts = significandParts();

  ప్రతిజ్ఞా(semantics == rhs.semantics);
  ప్రతిజ్ఞా(exponent == rhs.exponent);

  తిరిగి APInt::tc_వ్యవకలనం(parts, rhs.significandParts(), borrow,
                           part_లెక్క());
}

/* Multiply the significand of the RHS.  If ADDEND is non-NULL, add it
   on to the full-precision result of the multiplication.  Returns the
   lost fraction.  */
కోల్పోయిన_Fraction IEEEFloat::multiplySignificand(స్థిరరాశి IEEEFloat &rhs,
                                            IEEEFloat addend,
                                            తార్కిక ignoreAddend) {
  integerPart scratch[4];
  తార్కిక ignored;

  ప్రతిజ్ఞా(semantics == rhs.semantics);

  గుర్తులేని precision = semantics->precision;

  // Allocate space for twice as many bits as the original significand, plus one
  // extra bit for the addition to overflow into.
  గుర్తులేని new_Parts_లెక్క = part_లెక్క_For_బిట్లు(precision * 2 + 1);

  // FIXME: Replace with SmallVector<4>.
  integerPart *నిండిన_Significand =
      new_Parts_లెక్క > 4 ? కొత్త integerPart[new_Parts_లెక్క] : scratch;

  integerPart *lhsSignificand = significandParts();
  గుర్తులేని parts_లెక్క = part_లెక్క();

  APInt::tc_నిండిన_Multiply(నిండిన_Significand, lhsSignificand,
                        rhs.significandParts(), parts_లెక్క, parts_లెక్క);

  కోల్పోయిన_Fraction కోల్పోయిన_fraction = lfExactlyZero;
  // One, not zero, based MSB.
  గుర్తులేని omsb = APInt::tcMSB(నిండిన_Significand, new_Parts_లెక్క) + 1;
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

  ఒకవేళ (!ignoreAddend && addend.isNonZero()) {
    // The intermediate result of the multiplication has "2 * precision"
    // signicant bit; adjust the addend to be consistent with mul result.
    //
    Significand savedSignificand = significand;
    స్థిరరాశి fltSemantics *savedSemantics = semantics;

    // Normalize our MSB to one below the top bit to allow for overflow.
    గుర్తులేని extendedPrecision = 2 * precision + 1;
    ఒకవేళ (omsb != extendedPrecision - 1) {
      ప్రతిజ్ఞా(extendedPrecision > omsb);
      APInt::tc_జరుపు_ఎడమ(నిండిన_Significand, new_Parts_లెక్క,
                         (extendedPrecision - 1) - omsb);
      exponent -= (extendedPrecision - 1) - omsb;
    }

    /* Create new semantics.  */
    fltSemantics extendedSemantics = *semantics;
    extendedSemantics.precision = extendedPrecision;

    ఒకవేళ (new_Parts_లెక్క == 1)
      significand.part = నిండిన_Significand[0];
    లేకపోతే
      significand.parts = నిండిన_Significand;
    semantics = &extendedSemantics;

    // Make a copy so we can convert it to the extended semantics.
    // Note that we cannot convert the addend directly, as the extendedSemantics
    // is a local variable (which we take a reference to).
    IEEEFloat extendedAddend(addend);
    [[maybe_వాడని]] op_స్థితి స్థితి_3 = extendedAddend.convert(
        extendedSemantics, APFloat::rmTowardZero, &ignored);
    ప్రతిజ్ఞా(స్థితి_3 == APFloat::opOK);

    // Shift the significand of the addend right by one bit. This guarantees
    // that the high bit of the significand is zero (same as fullSignificand),
    // so the addition will overflow (if it does overflow at all) into the top bit.
    కోల్పోయిన_fraction = extendedAddend.జరుపు_Significand_కుడి(1);
    ప్రతిజ్ఞా(కోల్పోయిన_fraction == lfExactlyZero &&
           "Lost precision while shifting addend for fused-multiply-add.");

    కోల్పోయిన_fraction = కూడు_Or_వ్యవకలనం_Significand(extendedAddend, అబద్ధం);

    /* Restore our state.  */
    ఒకవేళ (new_Parts_లెక్క == 1)
      నిండిన_Significand[0] = significand.part;
    significand = savedSignificand;
    semantics = savedSemantics;

    omsb = APInt::tcMSB(నిండిన_Significand, new_Parts_లెక్క) + 1;
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
  ఒకవేళ (omsb > precision) {
    గుర్తులేని పూర్ణాంకం బిట్లు, significantParts;
    కోల్పోయిన_Fraction lf;

    బిట్లు = omsb - precision;
    significantParts = part_లెక్క_For_బిట్లు(omsb);
    lf = జరుపు_కుడి(నిండిన_Significand, significantParts, బిట్లు);
    కోల్పోయిన_fraction = combine_కోల్పోయిన_Fractions(lf, కోల్పోయిన_fraction);
    exponent += బిట్లు;
  }

  APInt::tc_కేటాయించు(lhsSignificand, నిండిన_Significand, parts_లెక్క);

  ఒకవేళ (new_Parts_లెక్క > 4)
    తొలగించు [] నిండిన_Significand;

  తిరిగి కోల్పోయిన_fraction;
}

కోల్పోయిన_Fraction IEEEFloat::multiplySignificand(స్థిరరాశి IEEEFloat &rhs) {
  // When the given semantics has zero, the addend here is a zero.
  // i.e . it belongs to the 'fcZero' category.
  // But when the semantics does not support zero, we need to
  // explicitly convey that this addend should be ignored
  // for multiplication.
  తిరిగి multiplySignificand(rhs, IEEEFloat(*semantics), !semantics->hasZero);
}

/* Multiply the significands of LHS and RHS to DST.  */
కోల్పోయిన_Fraction IEEEFloat::divideSignificand(స్థిరరాశి IEEEFloat &rhs) {
  integerPart scratch[4];

  ప్రతిజ్ఞా(semantics == rhs.semantics);

  integerPart *lhsSignificand = significandParts();
  స్థిరరాశి integerPart *rhsSignificand = rhs.significandParts();
  గుర్తులేని parts_లెక్క = part_లెక్క();

  integerPart *dividend =
      parts_లెక్క > 2 ? కొత్త integerPart[parts_లెక్క * 2] : scratch;
  integerPart *భాజకం = dividend + parts_లెక్క;

  /* Copy the dividend and divisor as they will be modified in-place.  */
  కొరకు (గుర్తులేని i = 0; i < parts_లెక్క; i++) {
    dividend[i] = lhsSignificand[i];
    భాజకం[i] = rhsSignificand[i];
    lhsSignificand[i] = 0;
  }

  exponent -= rhs.exponent;

  గుర్తులేని పూర్ణాంకం precision = semantics->precision;

  /* Normalize the divisor.  */
  గుర్తులేని బిట్టు = precision - APInt::tcMSB(భాజకం, parts_లెక్క) - 1;
  ఒకవేళ (బిట్టు) {
    exponent += బిట్టు;
    APInt::tc_జరుపు_ఎడమ(భాజకం, parts_లెక్క, బిట్టు);
  }

  /* Normalize the dividend.  */
  బిట్టు = precision - APInt::tcMSB(dividend, parts_లెక్క) - 1;
  ఒకవేళ (బిట్టు) {
    exponent -= బిట్టు;
    APInt::tc_జరుపు_ఎడమ(dividend, parts_లెక్క, బిట్టు);
  }

  /* Ensure the dividend >= divisor initially for the loop below.
     Incidentally, this means that the division loop below is
     guaranteed to set the integer bit to one.  */
  ఒకవేళ (APInt::tcCompare(dividend, భాజకం, parts_లెక్క) < 0) {
    exponent--;
    APInt::tc_జరుపు_ఎడమ(dividend, parts_లెక్క, 1);
    ప్రతిజ్ఞా(APInt::tcCompare(dividend, భాజకం, parts_లెక్క) >= 0);
  }

  /* Long division.  */
  కొరకు (బిట్టు = precision; బిట్టు; బిట్టు -= 1) {
    ఒకవేళ (APInt::tcCompare(dividend, భాజకం, parts_లెక్క) >= 0) {
      APInt::tc_వ్యవకలనం(dividend, భాజకం, 0, parts_లెక్క);
      APInt::tc_సమితి_బిట్టు(lhsSignificand, బిట్టు - 1);
    }

    APInt::tc_జరుపు_ఎడమ(dividend, parts_లెక్క, 1);
  }

  /* Figure out the lost fraction.  */
  పూర్ణాంకం cmp = APInt::tcCompare(dividend, భాజకం, parts_లెక్క);

  కోల్పోయిన_Fraction కోల్పోయిన_fraction;
  ఒకవేళ (cmp > 0)
    కోల్పోయిన_fraction = lf_మరిన్ని_Than_Half;
  లేకపోతే ఒకవేళ (cmp == 0)
    కోల్పోయిన_fraction = lfExactlyHalf;
  లేకపోతే ఒకవేళ (APInt::tcIsZero(dividend, parts_లెక్క))
    కోల్పోయిన_fraction = lfExactlyZero;
  లేకపోతే
    కోల్పోయిన_fraction = lfLessThanHalf;

  ఒకవేళ (parts_లెక్క > 2)
    తొలగించు [] dividend;

  తిరిగి కోల్పోయిన_fraction;
}

గుర్తులేని పూర్ణాంకం IEEEFloat::significandMSB() స్థిరరాశి {
  తిరిగి APInt::tcMSB(significandParts(), part_లెక్క());
}

గుర్తులేని పూర్ణాంకం IEEEFloat::significandLSB() స్థిరరాశి {
  తిరిగి APInt::tcLSB(significandParts(), part_లెక్క());
}

/* Note that a zero result is NOT normalized to fcZero.  */
కోల్పోయిన_Fraction IEEEFloat::జరుపు_Significand_కుడి(గుర్తులేని పూర్ణాంకం బిట్లు) {
  /* Our exponent should not overflow.  */
  ప్రతిజ్ఞా((ExponentType) (exponent + బిట్లు) >= exponent);

  exponent += బిట్లు;

  తిరిగి జరుపు_కుడి(significandParts(), part_లెక్క(), బిట్లు);
}

/* Shift the significand left BITS bits, subtract BITS from its exponent.  */
శూన్య IEEEFloat::జరుపు_Significand_ఎడమ(గుర్తులేని పూర్ణాంకం బిట్లు) {
  ప్రతిజ్ఞా(బిట్లు < semantics->precision ||
         (semantics->precision == 1 && బిట్లు <= 1));

  ఒకవేళ (బిట్లు) {
    గుర్తులేని పూర్ణాంకం parts_లెక్క = part_లెక్క();

    APInt::tc_జరుపు_ఎడమ(significandParts(), parts_లెక్క, బిట్లు);
    exponent -= బిట్లు;

    ప్రతిజ్ఞా(!APInt::tcIsZero(significandParts(), parts_లెక్క));
  }
}

APFloat::cmp_ఫలితం IEEEFloat::compare_Absolute_విలువ(స్థిరరాశి IEEEFloat &rhs) స్థిరరాశి {
  ప్రతిజ్ఞా(semantics == rhs.semantics);
  ప్రతిజ్ఞా(isFiniteNonZero());
  ప్రతిజ్ఞా(rhs.isFiniteNonZero());

  పూర్ణాంకం compare = exponent - rhs.exponent;

  /* If exponents are equal, do an unsigned bignum comparison of the
     significands.  */
  ఒకవేళ (compare == 0)
    compare = APInt::tcCompare(significandParts(), rhs.significandParts(),
                               part_లెక్క());

  ఒకవేళ (compare > 0)
    తిరిగి cmpGreaterThan;
  లేకపోతే ఒకవేళ (compare < 0)
    తిరిగి cmpLessThan;
  లేకపోతే
    తిరిగి cmpEqual;
}

/* Set the least significant BITS bits of a bignum, clear the
   rest.  */
స్థిర శూన్య tc_సమితి_కనిష్ఠ_Significant_బిట్లు(APInt::మాట_Type *dst, గుర్తులేని parts,
                                      గుర్తులేని బిట్లు) {
  గుర్తులేని i = 0;
  అయితే (బిట్లు > APInt::APINT_బిట్లు_చొప్పున_మాట) {
    dst[i++] = ~(APInt::మాట_Type)0;
    బిట్లు -= APInt::APINT_బిట్లు_చొప్పున_మాట;
  }

  ఒకవేళ (బిట్లు)
    dst[i++] = ~(APInt::మాట_Type)0 >> (APInt::APINT_బిట్లు_చొప్పున_మాట - బిట్లు);

  అయితే (i < parts)
    dst[i++] = 0;
}

/* Handle overflow.  Sign is preserved.  We either become infinity or
   the largest finite number.  */
APFloat::op_స్థితి IEEEFloat::handle_అధికప్రవాహం(rounding_బహుళకం rounding_బహుళకం_2) {
  ఒకవేళ (semantics->nonFiniteBehavior != fltNonfiniteBehavior::FiniteOnly) {
    /* Infinity?  */
    ఒకవేళ (rounding_బహుళకం_2 == rm_సమీప_Ties_గమ్యం_Even ||
        rounding_బహుళకం_2 == rm_సమీప_Ties_గమ్యం_Away ||
        (rounding_బహుళకం_2 == rm_Toward_ధనాత్మక && !sign) ||
        (rounding_బహుళకం_2 == rmTowardNegative && sign)) {
      ఒకవేళ (semantics->nonFiniteBehavior == fltNonfiniteBehavior::NanOnly)
        makeNaN(అబద్ధం, sign);
      లేకపోతే
        category = fcInfinity;
      తిరిగి static_cast<op_స్థితి>(op_అధికప్రవాహం | opInexact);
    }
  }

  /* Otherwise we become the largest finite number.  */
  category = fc_సాధారణ;
  exponent = semantics->maxExponent;
  tc_సమితి_కనిష్ఠ_Significant_బిట్లు(significandParts(), part_లెక్క(),
                            semantics->precision);
  ఒకవేళ (semantics->nonFiniteBehavior == fltNonfiniteBehavior::NanOnly &&
      semantics->nanEncoding == fltNanEncoding::AllOnes)
    APInt::tc_Clear_బిట్టు(significandParts(), 0);

  తిరిగి opInexact;
}

/* Returns TRUE if, when truncating the current number, with BIT the
   new LSB, with the given lost fraction and rounding mode, the result
   would need to be rounded away from zero (i.e., by increasing the
   signficand).  This routine must work for fcZero of both signs, and
   fcNormal numbers.  */
తార్కిక IEEEFloat::విడత_Away_From_Zero(rounding_బహుళకం rounding_బహుళకం_2,
                                  కోల్పోయిన_Fraction కోల్పోయిన_fraction,
                                  గుర్తులేని పూర్ణాంకం బిట్టు) స్థిరరాశి {
  /* NaNs and infinities should not have lost fractions.  */
  ప్రతిజ్ఞా(isFiniteNonZero() || category == fcZero);

  /* Current callers never pass this so we don't handle it.  */
  ప్రతిజ్ఞా(కోల్పోయిన_fraction != lfExactlyZero);

  ఎంపిక (rounding_బహుళకం_2) {
  స్థితి rm_సమీప_Ties_గమ్యం_Away:
    తిరిగి కోల్పోయిన_fraction == lfExactlyHalf || కోల్పోయిన_fraction == lf_మరిన్ని_Than_Half;

  స్థితి rm_సమీప_Ties_గమ్యం_Even:
    ఒకవేళ (కోల్పోయిన_fraction == lf_మరిన్ని_Than_Half)
      తిరిగి నిజం;

    /* Our zeroes don't have a significand to test.  */
    ఒకవేళ (కోల్పోయిన_fraction == lfExactlyHalf && category != fcZero)
      తిరిగి APInt::tc_వెలికితీయి_బిట్టు(significandParts(), బిట్టు);

    తిరిగి అబద్ధం;

  స్థితి rmTowardZero:
    తిరిగి అబద్ధం;

  స్థితి rm_Toward_ధనాత్మక:
    తిరిగి !sign;

  స్థితి rmTowardNegative:
    తిరిగి sign;

  అప్రమేయ:
    ఆపు;
  }
  llvm_unreachable("Invalid rounding mode found");
}

APFloat::op_స్థితి IEEEFloat::normalize(rounding_బహుళకం rounding_బహుళకం_2,
                                       కోల్పోయిన_Fraction కోల్పోయిన_fraction) {
  ఒకవేళ (!isFiniteNonZero())
    తిరిగి opOK;

  /* Before rounding normalize the exponent of fcNormal numbers.  */
  /* One, not zero, based MSB.  */
  గుర్తులేని omsb = significandMSB() + 1;

  // Only skip this `if` if the value is exactly zero.
  ఒకవేళ (omsb || కోల్పోయిన_fraction != lfExactlyZero) {
    /* OMSB is numbered from 1.  We want to place it in the integer
       bit numbered PRECISION if possible, with a compensating change in
       the exponent.  */
    పూర్ణాంకం exponentChange = omsb - semantics->precision;

    /* If the resulting exponent is too high, overflow according to
       the rounding mode.  */
    ఒకవేళ (exponent + exponentChange > semantics->maxExponent)
      తిరిగి handle_అధికప్రవాహం(rounding_బహుళకం_2);

    /* Subnormal numbers have exponent minExponent, and their MSB
       is forced based on that.  */
    ఒకవేళ (exponent + exponentChange < semantics->minExponent)
      exponentChange = semantics->minExponent - exponent;

    /* Shifting left is easy as we don't lose precision.  */
    ఒకవేళ (exponentChange < 0) {
      ప్రతిజ్ఞా(కోల్పోయిన_fraction == lfExactlyZero);

      జరుపు_Significand_ఎడమ(-exponentChange);

      తిరిగి opOK;
    }

    ఒకవేళ (exponentChange > 0) {
      కోల్పోయిన_Fraction lf;

      /* Shift right and capture any new lost fraction.  */
      lf = జరుపు_Significand_కుడి(exponentChange);

      కోల్పోయిన_fraction = combine_కోల్పోయిన_Fractions(lf, కోల్పోయిన_fraction);

      /* Keep OMSB up-to-date.  */
      ఒకవేళ (omsb > (గుర్తులేని) exponentChange)
        omsb -= exponentChange;
      లేకపోతే
        omsb = 0;
    }
  }

  // The all-ones values is an overflow if NaN is all ones. If NaN is
  // represented by negative zero, then it is a valid finite value.
  ఒకవేళ (semantics->nonFiniteBehavior == fltNonfiniteBehavior::NanOnly &&
      semantics->nanEncoding == fltNanEncoding::AllOnes &&
      exponent == semantics->maxExponent && isSignificandAllOnes())
    తిరిగి handle_అధికప్రవాహం(rounding_బహుళకం_2);

  /* Now round the number according to rounding_mode given the lost
     fraction.  */

  /* As specified in IEEE 754, since we do not trap we do not report
     underflow for exact results.  */
  ఒకవేళ (కోల్పోయిన_fraction == lfExactlyZero) {
    /* Canonicalize zeroes.  */
    ఒకవేళ (omsb == 0) {
      category = fcZero;
      ఒకవేళ (semantics->nanEncoding == fltNanEncoding::NegativeZero)
        sign = అబద్ధం;
      ఒకవేళ (!semantics->hasZero)
        make_అతిచిన్న_Normalized(అబద్ధం);
    }

    తిరిగి opOK;
  }

  /* Increment the significand if we're rounding away from zero.  */
  ఒకవేళ (విడత_Away_From_Zero(rounding_బహుళకం_2, కోల్పోయిన_fraction, 0)) {
    ఒకవేళ (omsb == 0)
      exponent = semantics->minExponent;

    incrementSignificand();
    omsb = significandMSB() + 1;

    /* Did the significand increment overflow?  */
    ఒకవేళ (omsb == (గుర్తులేని) semantics->precision + 1) {
      /* Renormalize by incrementing the exponent and shifting our
         significand right one.  However if we already have the
         maximum exponent we overflow to infinity.  */
      ఒకవేళ (exponent == semantics->maxExponent)
        // Invoke overflow handling with a rounding mode that will guarantee
        // that the result gets turned into the correct infinity representation.
        // This is needed instead of just setting the category to infinity to
        // account for 8-bit floating point types that have no inf, only NaN.
        తిరిగి handle_అధికప్రవాహం(sign ? rmTowardNegative : rm_Toward_ధనాత్మక);

      జరుపు_Significand_కుడి(1);

      తిరిగి opInexact;
    }

    // The all-ones values is an overflow if NaN is all ones. If NaN is
    // represented by negative zero, then it is a valid finite value.
    ఒకవేళ (semantics->nonFiniteBehavior == fltNonfiniteBehavior::NanOnly &&
        semantics->nanEncoding == fltNanEncoding::AllOnes &&
        exponent == semantics->maxExponent && isSignificandAllOnes())
      తిరిగి handle_అధికప్రవాహం(rounding_బహుళకం_2);
  }

  /* The normal case - we were and are not denormal, and any
     significand increment above didn't overflow.  */
  ఒకవేళ (omsb == semantics->precision)
    తిరిగి opInexact;

  /* We have a non-zero denormal.  */
  ప్రతిజ్ఞా(omsb < semantics->precision);

  /* Canonicalize zeroes.  */
  ఒకవేళ (omsb == 0) {
    category = fcZero;
    ఒకవేళ (semantics->nanEncoding == fltNanEncoding::NegativeZero)
      sign = అబద్ధం;
    // This condition handles the case where the semantics
    // does not have zero but uses the all-zero encoding
    // to represent the smallest normal value.
    ఒకవేళ (!semantics->hasZero)
      make_అతిచిన్న_Normalized(అబద్ధం);
  }

  /* The fcZero case is a denormal that underflowed to zero.  */
  తిరిగి (op_స్థితి) (opUnderflow | opInexact);
}

APFloat::op_స్థితి IEEEFloat::కూడు_Or_వ్యవకలనం_Specials(స్థిరరాశి IEEEFloat &rhs,
                                                   తార్కిక వ్యవకలనం) {
  ఎంపిక (Pack_Categories_Into_కీలకం(category, rhs.category)) {
  అప్రమేయ:
    llvm_unreachable(శూన్యసూచిక);

  స్థితి Pack_Categories_Into_కీలకం(fcZero, fcNaN):
  స్థితి Pack_Categories_Into_కీలకం(fc_సాధారణ, fcNaN):
  స్థితి Pack_Categories_Into_కీలకం(fcInfinity, fcNaN):
    కేటాయించు(rhs);
    [[fallthrough]];
  స్థితి Pack_Categories_Into_కీలకం(fcNaN, fcZero):
  స్థితి Pack_Categories_Into_కీలకం(fcNaN, fc_సాధారణ):
  స్థితి Pack_Categories_Into_కీలకం(fcNaN, fcInfinity):
  స్థితి Pack_Categories_Into_కీలకం(fcNaN, fcNaN):
    ఒకవేళ (isSignaling()) {
      makeQuiet();
      తిరిగి opInvalidOp;
    }
    తిరిగి rhs.isSignaling() ? opInvalidOp : opOK;

  స్థితి Pack_Categories_Into_కీలకం(fc_సాధారణ, fcZero):
  స్థితి Pack_Categories_Into_కీలకం(fcInfinity, fc_సాధారణ):
  స్థితి Pack_Categories_Into_కీలకం(fcInfinity, fcZero):
    తిరిగి opOK;

  స్థితి Pack_Categories_Into_కీలకం(fc_సాధారణ, fcInfinity):
  స్థితి Pack_Categories_Into_కీలకం(fcZero, fcInfinity):
    category = fcInfinity;
    sign = rhs.sign ^ వ్యవకలనం;
    తిరిగి opOK;

  స్థితి Pack_Categories_Into_కీలకం(fcZero, fc_సాధారణ):
    కేటాయించు(rhs);
    sign = rhs.sign ^ వ్యవకలనం;
    తిరిగి opOK;

  స్థితి Pack_Categories_Into_కీలకం(fcZero, fcZero):
    /* Sign depends on rounding mode; handled by caller.  */
    తిరిగి opOK;

  స్థితి Pack_Categories_Into_కీలకం(fcInfinity, fcInfinity):
    /* Differently signed infinities can only be validly
       subtracted.  */
    ఒకవేళ (((sign ^ rhs.sign)!=0) != వ్యవకలనం) {
      makeNaN();
      తిరిగి opInvalidOp;
    }

    తిరిగి opOK;

  స్థితి Pack_Categories_Into_కీలకం(fc_సాధారణ, fc_సాధారణ):
    తిరిగి opDivByZero;
  }
}

/* Add or subtract two normal numbers.  */
కోల్పోయిన_Fraction IEEEFloat::కూడు_Or_వ్యవకలనం_Significand(స్థిరరాశి IEEEFloat &rhs,
                                                 తార్కిక వ్యవకలనం) {
  [[maybe_వాడని]] integerPart వాహకం = 0;
  కోల్పోయిన_Fraction కోల్పోయిన_fraction;

  /* Determine if the operation on the absolute values is effectively
     an addition or subtraction.  */
  వ్యవకలనం ^= static_cast<తార్కిక>(sign ^ rhs.sign);

  /* Are we bigger exponent-wise than the RHS?  */
  పూర్ణాంకం బిట్లు = exponent - rhs.exponent;

  /* Subtraction is more subtle than one might naively expect.  */
  ఒకవేళ (వ్యవకలనం) {
    ఒకవేళ ((బిట్లు < 0) && !semantics->hasSignedRepr)
      llvm_unreachable(
          "This floating point format does not support signed values");

    IEEEFloat తాత్కాలిక_rhs(rhs);
    తార్కిక కోల్పోయిన_fraction_is_from_rhs = అబద్ధం;

    ఒకవేళ (బిట్లు == 0)
      కోల్పోయిన_fraction = lfExactlyZero;
    లేకపోతే ఒకవేళ (బిట్లు > 0) {
      కోల్పోయిన_fraction = తాత్కాలిక_rhs.జరుపు_Significand_కుడి(బిట్లు - 1);
      కోల్పోయిన_fraction_is_from_rhs = నిజం;
      జరుపు_Significand_ఎడమ(1);
    } లేకపోతే {
      కోల్పోయిన_fraction = జరుపు_Significand_కుడి(-బిట్లు - 1);
      తాత్కాలిక_rhs.జరుపు_Significand_ఎడమ(1);
    }

    // Should we reverse the subtraction.
    cmp_ఫలితం cmp_ఫలితం_2 = compare_Absolute_విలువ(తాత్కాలిక_rhs);
    ఒకవేళ (cmp_ఫలితం_2 == cmpLessThan) {
      తార్కిక borrow =
          కోల్పోయిన_fraction != lfExactlyZero && !కోల్పోయిన_fraction_is_from_rhs;
      ఒకవేళ (borrow) {
        // The lost fraction is being subtracted, borrow from the significand
        // and invert `lost_fraction`.
        ఒకవేళ (కోల్పోయిన_fraction == lfLessThanHalf)
          కోల్పోయిన_fraction = lf_మరిన్ని_Than_Half;
        లేకపోతే ఒకవేళ (కోల్పోయిన_fraction == lf_మరిన్ని_Than_Half)
          కోల్పోయిన_fraction = lfLessThanHalf;
      }
      వాహకం = తాత్కాలిక_rhs.వ్యవకలనం_Significand(*ఇది, borrow);
      copySignificand(తాత్కాలిక_rhs);
      sign = !sign;
    } లేకపోతే ఒకవేళ (cmp_ఫలితం_2 == cmpGreaterThan) {
      తార్కిక borrow = కోల్పోయిన_fraction != lfExactlyZero && కోల్పోయిన_fraction_is_from_rhs;
      ఒకవేళ (borrow) {
        // The lost fraction is being subtracted, borrow from the significand
        // and invert `lost_fraction`.
        ఒకవేళ (కోల్పోయిన_fraction == lfLessThanHalf)
          కోల్పోయిన_fraction = lf_మరిన్ని_Than_Half;
        లేకపోతే ఒకవేళ (కోల్పోయిన_fraction == lf_మరిన్ని_Than_Half)
          కోల్పోయిన_fraction = lfLessThanHalf;
      }
      వాహకం = వ్యవకలనం_Significand(తాత్కాలిక_rhs, borrow);
    } లేకపోతే { // cmpEqual
      zeroSignificand();
      ఒకవేళ (కోల్పోయిన_fraction != lfExactlyZero && కోల్పోయిన_fraction_is_from_rhs) {
        // rhs is slightly larger due to the lost fraction, flip the sign.
        sign = !sign;
      }
    }

    /* The code above is intended to ensure that no borrow is
       necessary.  */
    ప్రతిజ్ఞా(!వాహకం);
  } లేకపోతే {
    ఒకవేళ (బిట్లు > 0) {
      IEEEFloat తాత్కాలిక_rhs(rhs);

      కోల్పోయిన_fraction = తాత్కాలిక_rhs.జరుపు_Significand_కుడి(బిట్లు);
      వాహకం = కూడు_Significand(తాత్కాలిక_rhs);
    } లేకపోతే {
      కోల్పోయిన_fraction = జరుపు_Significand_కుడి(-బిట్లు);
      వాహకం = కూడు_Significand(rhs);
    }

    /* We have a guard bit; generating a carry cannot happen.  */
    ప్రతిజ్ఞా(!వాహకం);
  }

  తిరిగి కోల్పోయిన_fraction;
}

APFloat::op_స్థితి IEEEFloat::multiplySpecials(స్థిరరాశి IEEEFloat &rhs) {
  ఎంపిక (Pack_Categories_Into_కీలకం(category, rhs.category)) {
  అప్రమేయ:
    llvm_unreachable(శూన్యసూచిక);

  స్థితి Pack_Categories_Into_కీలకం(fcZero, fcNaN):
  స్థితి Pack_Categories_Into_కీలకం(fc_సాధారణ, fcNaN):
  స్థితి Pack_Categories_Into_కీలకం(fcInfinity, fcNaN):
    కేటాయించు(rhs);
    sign = అబద్ధం;
    [[fallthrough]];
  స్థితి Pack_Categories_Into_కీలకం(fcNaN, fcZero):
  స్థితి Pack_Categories_Into_కీలకం(fcNaN, fc_సాధారణ):
  స్థితి Pack_Categories_Into_కీలకం(fcNaN, fcInfinity):
  స్థితి Pack_Categories_Into_కీలకం(fcNaN, fcNaN):
    sign ^= rhs.sign; // restore the original sign
    ఒకవేళ (isSignaling()) {
      makeQuiet();
      తిరిగి opInvalidOp;
    }
    తిరిగి rhs.isSignaling() ? opInvalidOp : opOK;

  స్థితి Pack_Categories_Into_కీలకం(fc_సాధారణ, fcInfinity):
  స్థితి Pack_Categories_Into_కీలకం(fcInfinity, fc_సాధారణ):
  స్థితి Pack_Categories_Into_కీలకం(fcInfinity, fcInfinity):
    category = fcInfinity;
    తిరిగి opOK;

  స్థితి Pack_Categories_Into_కీలకం(fcZero, fc_సాధారణ):
  స్థితి Pack_Categories_Into_కీలకం(fc_సాధారణ, fcZero):
  స్థితి Pack_Categories_Into_కీలకం(fcZero, fcZero):
    category = fcZero;
    తిరిగి opOK;

  స్థితి Pack_Categories_Into_కీలకం(fcZero, fcInfinity):
  స్థితి Pack_Categories_Into_కీలకం(fcInfinity, fcZero):
    makeNaN();
    తిరిగి opInvalidOp;

  స్థితి Pack_Categories_Into_కీలకం(fc_సాధారణ, fc_సాధారణ):
    తిరిగి opOK;
  }
}

APFloat::op_స్థితి IEEEFloat::divideSpecials(స్థిరరాశి IEEEFloat &rhs) {
  ఎంపిక (Pack_Categories_Into_కీలకం(category, rhs.category)) {
  అప్రమేయ:
    llvm_unreachable(శూన్యసూచిక);

  స్థితి Pack_Categories_Into_కీలకం(fcZero, fcNaN):
  స్థితి Pack_Categories_Into_కీలకం(fc_సాధారణ, fcNaN):
  స్థితి Pack_Categories_Into_కీలకం(fcInfinity, fcNaN):
    కేటాయించు(rhs);
    sign = అబద్ధం;
    [[fallthrough]];
  స్థితి Pack_Categories_Into_కీలకం(fcNaN, fcZero):
  స్థితి Pack_Categories_Into_కీలకం(fcNaN, fc_సాధారణ):
  స్థితి Pack_Categories_Into_కీలకం(fcNaN, fcInfinity):
  స్థితి Pack_Categories_Into_కీలకం(fcNaN, fcNaN):
    sign ^= rhs.sign; // restore the original sign
    ఒకవేళ (isSignaling()) {
      makeQuiet();
      తిరిగి opInvalidOp;
    }
    తిరిగి rhs.isSignaling() ? opInvalidOp : opOK;

  స్థితి Pack_Categories_Into_కీలకం(fcInfinity, fcZero):
  స్థితి Pack_Categories_Into_కీలకం(fcInfinity, fc_సాధారణ):
  స్థితి Pack_Categories_Into_కీలకం(fcZero, fcInfinity):
  స్థితి Pack_Categories_Into_కీలకం(fcZero, fc_సాధారణ):
    తిరిగి opOK;

  స్థితి Pack_Categories_Into_కీలకం(fc_సాధారణ, fcInfinity):
    category = fcZero;
    తిరిగి opOK;

  స్థితి Pack_Categories_Into_కీలకం(fc_సాధారణ, fcZero):
    ఒకవేళ (semantics->nonFiniteBehavior == fltNonfiniteBehavior::NanOnly)
      makeNaN(అబద్ధం, sign);
    లేకపోతే
      category = fcInfinity;
    తిరిగి opDivByZero;

  స్థితి Pack_Categories_Into_కీలకం(fcInfinity, fcInfinity):
  స్థితి Pack_Categories_Into_కీలకం(fcZero, fcZero):
    makeNaN();
    తిరిగి opInvalidOp;

  స్థితి Pack_Categories_Into_కీలకం(fc_సాధారణ, fc_సాధారణ):
    తిరిగి opOK;
  }
}

APFloat::op_స్థితి IEEEFloat::modSpecials(స్థిరరాశి IEEEFloat &rhs) {
  ఎంపిక (Pack_Categories_Into_కీలకం(category, rhs.category)) {
  అప్రమేయ:
    llvm_unreachable(శూన్యసూచిక);

  స్థితి Pack_Categories_Into_కీలకం(fcZero, fcNaN):
  స్థితి Pack_Categories_Into_కీలకం(fc_సాధారణ, fcNaN):
  స్థితి Pack_Categories_Into_కీలకం(fcInfinity, fcNaN):
    కేటాయించు(rhs);
    [[fallthrough]];
  స్థితి Pack_Categories_Into_కీలకం(fcNaN, fcZero):
  స్థితి Pack_Categories_Into_కీలకం(fcNaN, fc_సాధారణ):
  స్థితి Pack_Categories_Into_కీలకం(fcNaN, fcInfinity):
  స్థితి Pack_Categories_Into_కీలకం(fcNaN, fcNaN):
    ఒకవేళ (isSignaling()) {
      makeQuiet();
      తిరిగి opInvalidOp;
    }
    తిరిగి rhs.isSignaling() ? opInvalidOp : opOK;

  స్థితి Pack_Categories_Into_కీలకం(fcZero, fcInfinity):
  స్థితి Pack_Categories_Into_కీలకం(fcZero, fc_సాధారణ):
  స్థితి Pack_Categories_Into_కీలకం(fc_సాధారణ, fcInfinity):
    తిరిగి opOK;

  స్థితి Pack_Categories_Into_కీలకం(fc_సాధారణ, fcZero):
  స్థితి Pack_Categories_Into_కీలకం(fcInfinity, fcZero):
  స్థితి Pack_Categories_Into_కీలకం(fcInfinity, fc_సాధారణ):
  స్థితి Pack_Categories_Into_కీలకం(fcInfinity, fcInfinity):
  స్థితి Pack_Categories_Into_కీలకం(fcZero, fcZero):
    makeNaN();
    తిరిగి opInvalidOp;

  స్థితి Pack_Categories_Into_కీలకం(fc_సాధారణ, fc_సాధారణ):
    తిరిగి opOK;
  }
}

APFloat::op_స్థితి IEEEFloat::remainderSpecials(స్థిరరాశి IEEEFloat &rhs) {
  ఎంపిక (Pack_Categories_Into_కీలకం(category, rhs.category)) {
  అప్రమేయ:
    llvm_unreachable(శూన్యసూచిక);

  స్థితి Pack_Categories_Into_కీలకం(fcZero, fcNaN):
  స్థితి Pack_Categories_Into_కీలకం(fc_సాధారణ, fcNaN):
  స్థితి Pack_Categories_Into_కీలకం(fcInfinity, fcNaN):
    కేటాయించు(rhs);
    [[fallthrough]];
  స్థితి Pack_Categories_Into_కీలకం(fcNaN, fcZero):
  స్థితి Pack_Categories_Into_కీలకం(fcNaN, fc_సాధారణ):
  స్థితి Pack_Categories_Into_కీలకం(fcNaN, fcInfinity):
  స్థితి Pack_Categories_Into_కీలకం(fcNaN, fcNaN):
    ఒకవేళ (isSignaling()) {
      makeQuiet();
      తిరిగి opInvalidOp;
    }
    తిరిగి rhs.isSignaling() ? opInvalidOp : opOK;

  స్థితి Pack_Categories_Into_కీలకం(fcZero, fcInfinity):
  స్థితి Pack_Categories_Into_కీలకం(fcZero, fc_సాధారణ):
  స్థితి Pack_Categories_Into_కీలకం(fc_సాధారణ, fcInfinity):
    తిరిగి opOK;

  స్థితి Pack_Categories_Into_కీలకం(fc_సాధారణ, fcZero):
  స్థితి Pack_Categories_Into_కీలకం(fcInfinity, fcZero):
  స్థితి Pack_Categories_Into_కీలకం(fcInfinity, fc_సాధారణ):
  స్థితి Pack_Categories_Into_కీలకం(fcInfinity, fcInfinity):
  స్థితి Pack_Categories_Into_కీలకం(fcZero, fcZero):
    makeNaN();
    తిరిగి opInvalidOp;

  స్థితి Pack_Categories_Into_కీలకం(fc_సాధారణ, fc_సాధారణ):
    తిరిగి opDivByZero; // fake status, indicating this is not a special case
  }
}

/* Change sign.  */
శూన్య IEEEFloat::changeSign() {
  // With NaN-as-negative-zero, neither NaN or negative zero can change
  // their signs.
  ఒకవేళ (semantics->nanEncoding == fltNanEncoding::NegativeZero &&
      (isZero() || isNaN()))
    తిరిగి;
  /* Look mummy, this one's easy.  */
  sign = !sign;
}

/* Normalized addition or subtraction.  */
APFloat::op_స్థితి IEEEFloat::కూడు_Or_వ్యవకలనం(స్థిరరాశి IEEEFloat &rhs,
                                           rounding_బహుళకం rounding_బహుళకం_2,
                                           తార్కిక వ్యవకలనం) {
  op_స్థితి fs = కూడు_Or_వ్యవకలనం_Specials(rhs, వ్యవకలనం);

  /* This return code means it was not a simple case.  */
  ఒకవేళ (fs == opDivByZero) {
    కోల్పోయిన_Fraction కోల్పోయిన_fraction;

    కోల్పోయిన_fraction = కూడు_Or_వ్యవకలనం_Significand(rhs, వ్యవకలనం);
    fs = normalize(rounding_బహుళకం_2, కోల్పోయిన_fraction);

    /* Can only be zero if we lost no fraction.  */
    ప్రతిజ్ఞా(category != fcZero || కోల్పోయిన_fraction == lfExactlyZero);
  }

  /* If two numbers add (exactly) to zero, IEEE 754 decrees it is a
     positive zero unless rounding to minus infinity, except that
     adding two like-signed zeroes gives that zero.  */
  ఒకవేళ (category == fcZero) {
    ఒకవేళ (rhs.category != fcZero || (sign == rhs.sign) == వ్యవకలనం)
      sign = (rounding_బహుళకం_2 == rmTowardNegative);
    // NaN-in-negative-zero means zeros need to be normalized to +0.
    ఒకవేళ (semantics->nanEncoding == fltNanEncoding::NegativeZero)
      sign = అబద్ధం;
  }

  తిరిగి fs;
}

/* Normalized addition.  */
APFloat::op_స్థితి IEEEFloat::కూడు(స్థిరరాశి IEEEFloat &rhs,
                                 rounding_బహుళకం rounding_బహుళకం_2) {
  తిరిగి కూడు_Or_వ్యవకలనం(rhs, rounding_బహుళకం_2, అబద్ధం);
}

/* Normalized subtraction.  */
APFloat::op_స్థితి IEEEFloat::వ్యవకలనం(స్థిరరాశి IEEEFloat &rhs,
                                      rounding_బహుళకం rounding_బహుళకం_2) {
  తిరిగి కూడు_Or_వ్యవకలనం(rhs, rounding_బహుళకం_2, నిజం);
}

/* Normalized multiply.  */
APFloat::op_స్థితి IEEEFloat::multiply(స్థిరరాశి IEEEFloat &rhs,
                                      rounding_బహుళకం rounding_బహుళకం_2) {
  sign ^= rhs.sign;
  op_స్థితి fs = multiplySpecials(rhs);

  ఒకవేళ (isZero() && semantics->nanEncoding == fltNanEncoding::NegativeZero)
    sign = అబద్ధం;
  ఒకవేళ (isFiniteNonZero()) {
    కోల్పోయిన_Fraction కోల్పోయిన_fraction = multiplySignificand(rhs);
    fs = normalize(rounding_బహుళకం_2, కోల్పోయిన_fraction);
    ఒకవేళ (కోల్పోయిన_fraction != lfExactlyZero)
      fs = (op_స్థితి) (fs | opInexact);
  }

  తిరిగి fs;
}

/* Normalized divide.  */
APFloat::op_స్థితి IEEEFloat::divide(స్థిరరాశి IEEEFloat &rhs,
                                    rounding_బహుళకం rounding_బహుళకం_2) {
  sign ^= rhs.sign;
  op_స్థితి fs = divideSpecials(rhs);

  ఒకవేళ (isZero() && semantics->nanEncoding == fltNanEncoding::NegativeZero)
    sign = అబద్ధం;
  ఒకవేళ (isFiniteNonZero()) {
    కోల్పోయిన_Fraction కోల్పోయిన_fraction = divideSignificand(rhs);
    fs = normalize(rounding_బహుళకం_2, కోల్పోయిన_fraction);
    ఒకవేళ (కోల్పోయిన_fraction != lfExactlyZero)
      fs = (op_స్థితి) (fs | opInexact);
  }

  తిరిగి fs;
}

/* Normalized remainder.  */
APFloat::op_స్థితి IEEEFloat::remainder(స్థిరరాశి IEEEFloat &rhs) {
  గుర్తులేని పూర్ణాంకం origSign = sign;

  // First handle the special cases.
  op_స్థితి fs = remainderSpecials(rhs);
  ఒకవేళ (fs != opDivByZero)
    తిరిగి fs;

  fs = opOK;

  // Make sure the current value is less than twice the denom. If the addition
  // did not succeed (an overflow has happened), which means that the finite
  // value we currently posses must be less than twice the denom (as we are
  // using the same semantics).
  IEEEFloat P2 = rhs;
  ఒకవేళ (P2.కూడు(rhs, rm_సమీప_Ties_గమ్యం_Even) == opOK) {
    fs = mod(P2);
    ప్రతిజ్ఞా(fs == opOK);
  }

  // Lets work with absolute numbers.
  IEEEFloat P = rhs;
  P.sign = అబద్ధం;
  sign = అబద్ధం;

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
  తార్కిక losesInfo;
  fltSemantics extendedSemantics = *semantics;
  extendedSemantics.maxExponent++;
  extendedSemantics.minExponent--;
  extendedSemantics.precision += 2;

  IEEEFloat VEx = *ఇది;
  fs = VEx.convert(extendedSemantics, rm_సమీప_Ties_గమ్యం_Even, &losesInfo);
  ప్రతిజ్ఞా(fs == opOK && !losesInfo);
  IEEEFloat PEx = P;
  fs = PEx.convert(extendedSemantics, rm_సమీప_Ties_గమ్యం_Even, &losesInfo);
  ప్రతిజ్ఞా(fs == opOK && !losesInfo);

  // It is simpler to work with 2x instead of 0.5p, and we do not need to lose
  // any fraction.
  fs = VEx.కూడు(VEx, rm_సమీప_Ties_గమ్యం_Even);
  ప్రతిజ్ఞా(fs == opOK);

  ఒకవేళ (VEx.compare(PEx) == cmpGreaterThan) {
    fs = వ్యవకలనం(P, rm_సమీప_Ties_గమ్యం_Even);
    ప్రతిజ్ఞా(fs == opOK);

    // Make VEx = this.add(this), but because we have different semantics, we do
    // not want to `convert` again, so we just subtract PEx twice (which equals
    // to the desired value).
    fs = VEx.వ్యవకలనం(PEx, rm_సమీప_Ties_గమ్యం_Even);
    ప్రతిజ్ఞా(fs == opOK);
    fs = VEx.వ్యవకలనం(PEx, rm_సమీప_Ties_గమ్యం_Even);
    ప్రతిజ్ఞా(fs == opOK);

    cmp_ఫలితం ఫలితం_2 = VEx.compare(PEx);
    ఒకవేళ (ఫలితం_2 == cmpGreaterThan || ఫలితం_2 == cmpEqual) {
      fs = వ్యవకలనం(P, rm_సమీప_Ties_గమ్యం_Even);
      ప్రతిజ్ఞా(fs == opOK);
    }
  }

  ఒకవేళ (isZero()) {
    sign = origSign;    // IEEE754 requires this
    ఒకవేళ (semantics->nanEncoding == fltNanEncoding::NegativeZero)
      // But some 8-bit floats only have positive 0.
      sign = అబద్ధం;
  } లేకపోతే {
    sign ^= origSign;
  }
  తిరిగి fs;
}

/* Normalized llvm frem (C fmod). */
APFloat::op_స్థితి IEEEFloat::mod(స్థిరరాశి IEEEFloat &rhs) {
  op_స్థితి fs = modSpecials(rhs);
  గుర్తులేని పూర్ణాంకం origSign = sign;

  అయితే (isFiniteNonZero() && rhs.isFiniteNonZero() &&
         compare_Absolute_విలువ(rhs) != cmpLessThan) {
    పూర్ణాంకం Exp = ilogb(*ఇది) - ilogb(rhs);
    IEEEFloat V = scalbn(rhs, Exp, rm_సమీప_Ties_గమ్యం_Even);
    // V can overflow to NaN with fltNonfiniteBehavior::NanOnly, so explicitly
    // check for it.
    ఒకవేళ (V.isNaN() || compare_Absolute_విలువ(V) == cmpLessThan)
      V = scalbn(rhs, Exp - 1, rm_సమీప_Ties_గమ్యం_Even);
    V.sign = sign;

    fs = వ్యవకలనం(V, rm_సమీప_Ties_గమ్యం_Even);

    // When the semantics supports zero, this loop's
    // exit-condition is handled by the 'isFiniteNonZero'
    // category check above. However, when the semantics
    // does not have 'fcZero' and we have reached the
    // minimum possible value, (and any further subtract
    // will underflow to the same value) explicitly
    // provide an exit-path here.
    ఒకవేళ (!semantics->hasZero && ఇది->is_అతిచిన్న())
      ఆపు;

    ప్రతిజ్ఞా(fs==opOK);
  }
  ఒకవేళ (isZero()) {
    sign = origSign; // fmod requires this
    ఒకవేళ (semantics->nanEncoding == fltNanEncoding::NegativeZero)
      sign = అబద్ధం;
  }
  తిరిగి fs;
}

/* Normalized fused-multiply-add.  */
APFloat::op_స్థితి IEEEFloat::fused_Multiply_కూడు(స్థిరరాశి IEEEFloat &multiplicand,
                                              స్థిరరాశి IEEEFloat &addend,
                                              rounding_బహుళకం rounding_బహుళకం_2) {
  op_స్థితి fs;

  /* Post-multiplication sign, before addition.  */
  sign ^= multiplicand.sign;

  /* If and only if all arguments are normal do we need to do an
     extended-precision calculation.  */
  ఒకవేళ (isFiniteNonZero() &&
      multiplicand.isFiniteNonZero() &&
      addend.isFinite()) {
    కోల్పోయిన_Fraction కోల్పోయిన_fraction;

    కోల్పోయిన_fraction = multiplySignificand(multiplicand, addend);
    fs = normalize(rounding_బహుళకం_2, కోల్పోయిన_fraction);
    ఒకవేళ (కోల్పోయిన_fraction != lfExactlyZero)
      fs = (op_స్థితి) (fs | opInexact);

    /* If two numbers add (exactly) to zero, IEEE 754 decrees it is a
       positive zero unless rounding to minus infinity, except that
       adding two like-signed zeroes gives that zero.  */
    ఒకవేళ (category == fcZero && !(fs & opUnderflow) && sign != addend.sign) {
      sign = (rounding_బహుళకం_2 == rmTowardNegative);
      ఒకవేళ (semantics->nanEncoding == fltNanEncoding::NegativeZero)
        sign = అబద్ధం;
    }
  } లేకపోతే {
    fs = multiplySpecials(multiplicand);

    /* FS can only be opOK or opInvalidOp.  There is no more work
       to do in the latter case.  The IEEE-754R standard says it is
       implementation-defined in this case whether, if ADDEND is a
       quiet NaN, we raise invalid op; this implementation does so.

       If we need to do the addition we can do so with normal
       precision.  */
    ఒకవేళ (fs == opOK)
      fs = కూడు_Or_వ్యవకలనం(addend, rounding_బహుళకం_2, అబద్ధం);
  }

  తిరిగి fs;
}

/* Rounding-mode correct round to integral value.  */
APFloat::op_స్థితి IEEEFloat::విడత_గమ్యం_Integral(rounding_బహుళకం rounding_బహుళకం_2) {
  ఒకవేళ (isInfinity())
    // [IEEE Std 754-2008 6.1]:
    // The behavior of infinity in floating-point arithmetic is derived from the
    // limiting cases of real arithmetic with operands of arbitrarily
    // large magnitude, when such a limit exists.
    // ...
    // Operations on infinite operands are usually exact and therefore signal no
    // exceptions ...
    తిరిగి opOK;

  ఒకవేళ (isNaN()) {
    ఒకవేళ (isSignaling()) {
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
      తిరిగి opInvalidOp;
    } లేకపోతే {
      // [IEEE Std 754-2008 6.2]:
      // For an operation with quiet NaN inputs, other than maximum and minimum
      // operations, if a floating-point result is to be delivered the result
      // shall be a quiet NaN which should be one of the input NaNs.
      // ...
      // Every general-computational and quiet-computational operation involving
      // one or more input NaNs, none of them signaling, shall signal no
      // exception, except fusedMultiplyAdd might signal the invalid operation
      // exception(see 7.2).
      తిరిగి opOK;
    }
  }

  ఒకవేళ (isZero()) {
    // [IEEE Std 754-2008 6.3]:
    // ... the sign of the result of conversions, the quantize operation, the
    // roundToIntegral operations, and the roundToIntegralExact(see 5.3.1) is
    // the sign of the first or only operand.
    తిరిగి opOK;
  }

  // If the exponent is large enough, we know that this value is already
  // integral, and the arithmetic below would potentially cause it to saturate
  // to +/-Inf.  Bail out early instead.
  ఒకవేళ (exponent + 1 >= (పూర్ణాంకం)APFloat::semanticsPrecision(*semantics))
    తిరిగి opOK;

  // The algorithm here is quite simple: we add 2^(p-1), where p is the
  // precision of our format, and then subtract it back off again.  The choice
  // of rounding modes for the addition/subtraction determines the rounding mode
  // for our integral rounding as well.
  // NOTE: When the input value is negative, we do subtraction followed by
  // addition instead.
  APInt IntegerConstant(తదుపరి_Power_Of2(APFloat::semanticsPrecision(*semantics)),
                        1);
  IntegerConstant <<= APFloat::semanticsPrecision(*semantics) - 1;
  IEEEFloat MagicConstant(*semantics);
  op_స్థితి fs = MagicConstant.convertFromAPInt(IntegerConstant, అబద్ధం,
                                               rm_సమీప_Ties_గమ్యం_Even);
  ప్రతిజ్ఞా(fs == opOK);
  MagicConstant.sign = sign;

  // Preserve the input sign so that we can handle the case of zero result
  // correctly.
  తార్కిక ఆదానం_Sign = isNegative();

  fs = కూడు(MagicConstant, rounding_బహుళకం_2);

  // Current value and 'MagicConstant' are both integers, so the result of the
  // subtraction is always exact according to Sterbenz' lemma.
  వ్యవకలనం(MagicConstant, rounding_బహుళకం_2);

  // Restore the input sign.
  ఒకవేళ (ఆదానం_Sign != isNegative())
    changeSign();

  తిరిగి fs;
}

/* Comparison requires normalized numbers.  */
APFloat::cmp_ఫలితం IEEEFloat::compare(స్థిరరాశి IEEEFloat &rhs) స్థిరరాశి {
  ప్రతిజ్ఞా(semantics == rhs.semantics);

  ఎంపిక (Pack_Categories_Into_కీలకం(category, rhs.category)) {
  అప్రమేయ:
    llvm_unreachable(శూన్యసూచిక);

  స్థితి Pack_Categories_Into_కీలకం(fcNaN, fcZero):
  స్థితి Pack_Categories_Into_కీలకం(fcNaN, fc_సాధారణ):
  స్థితి Pack_Categories_Into_కీలకం(fcNaN, fcInfinity):
  స్థితి Pack_Categories_Into_కీలకం(fcNaN, fcNaN):
  స్థితి Pack_Categories_Into_కీలకం(fcZero, fcNaN):
  స్థితి Pack_Categories_Into_కీలకం(fc_సాధారణ, fcNaN):
  స్థితి Pack_Categories_Into_కీలకం(fcInfinity, fcNaN):
    తిరిగి cmpUnordered;

  స్థితి Pack_Categories_Into_కీలకం(fcInfinity, fc_సాధారణ):
  స్థితి Pack_Categories_Into_కీలకం(fcInfinity, fcZero):
  స్థితి Pack_Categories_Into_కీలకం(fc_సాధారణ, fcZero):
    ఒకవేళ (sign)
      తిరిగి cmpLessThan;
    లేకపోతే
      తిరిగి cmpGreaterThan;

  స్థితి Pack_Categories_Into_కీలకం(fc_సాధారణ, fcInfinity):
  స్థితి Pack_Categories_Into_కీలకం(fcZero, fcInfinity):
  స్థితి Pack_Categories_Into_కీలకం(fcZero, fc_సాధారణ):
    ఒకవేళ (rhs.sign)
      తిరిగి cmpGreaterThan;
    లేకపోతే
      తిరిగి cmpLessThan;

  స్థితి Pack_Categories_Into_కీలకం(fcInfinity, fcInfinity):
    ఒకవేళ (sign == rhs.sign)
      తిరిగి cmpEqual;
    లేకపోతే ఒకవేళ (sign)
      తిరిగి cmpLessThan;
    లేకపోతే
      తిరిగి cmpGreaterThan;

  స్థితి Pack_Categories_Into_కీలకం(fcZero, fcZero):
    తిరిగి cmpEqual;

  స్థితి Pack_Categories_Into_కీలకం(fc_సాధారణ, fc_సాధారణ):
    ఆపు;
  }

  cmp_ఫలితం ఫలితం_2;
  /* Two normal numbers.  Do they have the same sign?  */
  ఒకవేళ (sign != rhs.sign) {
    ఒకవేళ (sign)
      ఫలితం_2 = cmpLessThan;
    లేకపోతే
      ఫలితం_2 = cmpGreaterThan;
  } లేకపోతే {
    /* Compare absolute values; invert result if negative.  */
    ఫలితం_2 = compare_Absolute_విలువ(rhs);

    ఒకవేళ (sign) {
      ఒకవేళ (ఫలితం_2 == cmpLessThan)
        ఫలితం_2 = cmpGreaterThan;
      లేకపోతే ఒకవేళ (ఫలితం_2 == cmpGreaterThan)
        ఫలితం_2 = cmpLessThan;
    }
  }

  తిరిగి ఫలితం_2;
}

/// IEEEFloat::convert - convert a value of one floating point type to another.
/// The return value corresponds to the IEEE754 exceptions.  *losesInfo
/// records whether the transformation lost information, i.e. whether
/// converting the result back to the original type will produce the
/// original value (this is almost the same as return value==fsOK, but there
/// are edge cases where this is not so).

APFloat::op_స్థితి IEEEFloat::convert(స్థిరరాశి fltSemantics &గమ్యం_Semantics,
                                     rounding_బహుళకం rounding_బహుళకం_2,
                                     తార్కిక *losesInfo) {
  op_స్థితి fs;
  స్థిరరాశి fltSemantics &fromSemantics = *semantics;
  తార్కిక is_signaling = isSignaling();

  కోల్పోయిన_Fraction కోల్పోయిన_Fraction = lfExactlyZero;
  గుర్తులేని new_Part_లెక్క = part_లెక్క_For_బిట్లు(గమ్యం_Semantics.precision + 1);
  గుర్తులేని old_Part_లెక్క = part_లెక్క();
  పూర్ణాంకం జరుపు = గమ్యం_Semantics.precision - fromSemantics.precision;

  తార్కిక X86SpecialNan = అబద్ధం;
  ఒకవేళ (&fromSemantics == &APFloat_పునాది::semX87DoubleExtended &&
      &గమ్యం_Semantics != &APFloat_పునాది::semX87DoubleExtended && category == fcNaN &&
      (!(*significandParts() & 0x8000000000000000ULL) ||
       !(*significandParts() & 0x4000000000000000ULL))) {
    // x86 has some unusual NaNs which cannot be represented in any other
    // format; note them here.
    X86SpecialNan = నిజం;
  }

  // If this is a truncation of a denormal number, and the target semantics
  // has larger exponent range than the source semantics (this can happen
  // when truncating from PowerPC double-double to double format), the
  // right shift could lose result mantissa bits.  Adjust exponent instead
  // of performing excessive shift.
  // Also do a similar trick in case shifting denormal would produce zero
  // significand as this case isn't handled correctly by normalize.
  ఒకవేళ (జరుపు < 0 && isFiniteNonZero()) {
    పూర్ణాంకం omsb = significandMSB() + 1;
    పూర్ణాంకం exponentChange = omsb - fromSemantics.precision;
    ఒకవేళ (exponent + exponentChange < గమ్యం_Semantics.minExponent)
      exponentChange = గమ్యం_Semantics.minExponent - exponent;
    exponentChange = std::max(exponentChange, జరుపు);
    ఒకవేళ (exponentChange < 0) {
      జరుపు -= exponentChange;
      exponent += exponentChange;
    } లేకపోతే ఒకవేళ (omsb <= -జరుపు) {
      exponentChange = omsb + జరుపు - 1; // leave at least one bit set
      జరుపు -= exponentChange;
      exponent += exponentChange;
    }
  }

  // If this is a truncation, perform the shift before we narrow the storage.
  ఒకవేళ (జరుపు < 0 && (isFiniteNonZero() ||
                    (category == fcNaN && semantics->nonFiniteBehavior !=
                                              fltNonfiniteBehavior::NanOnly)))
    కోల్పోయిన_Fraction = జరుపు_కుడి(significandParts(), old_Part_లెక్క, -జరుపు);

  // Fix the storage so it can hold to new value.
  ఒకవేళ (new_Part_లెక్క > old_Part_లెక్క) {
    // The new type requires more storage; make it available.
    integerPart *newParts;
    newParts = కొత్త integerPart[new_Part_లెక్క];
    APInt::tc_సమితి(newParts, 0, new_Part_లెక్క);
    ఒకవేళ (isFiniteNonZero() || category==fcNaN)
      APInt::tc_కేటాయించు(newParts, significandParts(), old_Part_లెక్క);
    freeSignificand();
    significand.parts = newParts;
  } లేకపోతే ఒకవేళ (new_Part_లెక్క == 1 && old_Part_లెక్క != 1) {
    // Switch to built-in storage for a single part.
    integerPart newPart = 0;
    ఒకవేళ (isFiniteNonZero() || category==fcNaN)
      newPart = significandParts()[0];
    freeSignificand();
    significand.part = newPart;
  }

  // Now that we have the right storage, switch the semantics.
  semantics = &గమ్యం_Semantics;

  // If this is an extension, perform the shift now that the storage is
  // available.
  ఒకవేళ (జరుపు > 0 && (isFiniteNonZero() || category==fcNaN))
    APInt::tc_జరుపు_ఎడమ(significandParts(), new_Part_లెక్క, జరుపు);

  ఒకవేళ (isFiniteNonZero()) {
    fs = normalize(rounding_బహుళకం_2, కోల్పోయిన_Fraction);
    *losesInfo = (fs != opOK);
  } లేకపోతే ఒకవేళ (category == fcNaN) {
    ఒకవేళ (semantics->nonFiniteBehavior == fltNonfiniteBehavior::NanOnly) {
      *losesInfo =
          fromSemantics.nonFiniteBehavior != fltNonfiniteBehavior::NanOnly;
      makeNaN(అబద్ధం, sign);
      fs = is_signaling ? opInvalidOp : opOK;
    } లేకపోతే {
      // If NaN is negative zero, we need to create a new NaN to avoid
      // converting NaN to -Inf.
      ఒకవేళ (fromSemantics.nanEncoding == fltNanEncoding::NegativeZero &&
          semantics->nanEncoding != fltNanEncoding::NegativeZero)
        makeNaN(అబద్ధం, అబద్ధం);

      // If the source has no significand, there are no payload bits to carry
      // over, and an all-zero significand would encode an Inf. Create a new
      // NaN.
      ఒకవేళ (!APFloat::hasSignificand(fromSemantics))
        makeNaN(అబద్ధం, sign);

      *losesInfo = కోల్పోయిన_Fraction != lfExactlyZero || X86SpecialNan;

      // For x87 extended precision, we want to make a NaN, not a special NaN
      // if the input wasn't special either.
      ఒకవేళ (!X86SpecialNan && semantics == &APFloat_పునాది::semX87DoubleExtended)
        APInt::tc_సమితి_బిట్టు(significandParts(), semantics->precision - 1);

      // Convert of sNaN creates qNaN and raises an exception (invalid op).
      // This also guarantees that a sNaN does not become Inf on a truncation
      // that loses all payload bits.
      ఒకవేళ (is_signaling) {
        makeQuiet();
        fs = opInvalidOp;
      } లేకపోతే {
        fs = opOK;
      }
    }
  } లేకపోతే ఒకవేళ (category == fcInfinity &&
             semantics->nonFiniteBehavior == fltNonfiniteBehavior::NanOnly) {
    makeNaN(అబద్ధం, sign);
    *losesInfo = నిజం;
    fs = opInexact;
  } లేకపోతే ఒకవేళ (category == fcZero &&
             semantics->nanEncoding == fltNanEncoding::NegativeZero) {
    // Negative zero loses info, but positive zero doesn't.
    *losesInfo =
        fromSemantics.nanEncoding != fltNanEncoding::NegativeZero && sign;
    fs = *losesInfo ? opInexact : opOK;
    // NaN is negative zero means -0 -> +0, which can lose information
    sign = అబద్ధం;
  } లేకపోతే {
    *losesInfo = అబద్ధం;
    fs = opOK;
  }

  // The target may have no encoding for a negative value, or none for zero.
  // The paths above only report what rounding lost, so report these here too:
  // a caller that checks losesInfo would otherwise accept a result the target
  // cannot represent, and printing that result asserts.
  ఒకవేళ ((sign && !semantics->hasSignedRepr) ||
      (category == fcZero && !semantics->hasZero)) {
    *losesInfo = నిజం;
    ఒకవేళ (fs == opOK)
      fs = opInexact;
  }

  ఒకవేళ (category == fcZero && !semantics->hasZero)
    make_అతిచిన్న_Normalized(అబద్ధం);
  తిరిగి fs;
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
APFloat::op_స్థితి IEEEFloat::convert_గమ్యం_Sign_Extended_Integer(
    MutableArrayRef<integerPart> parts, గుర్తులేని పూర్ణాంకం వెడల్పు_2, తార్కిక isSigned,
    rounding_బహుళకం rounding_బహుళకం_2, తార్కిక *isExact) స్థిరరాశి {
  *isExact = అబద్ధం;

  /* Handle the three special cases first.  */
  ఒకవేళ (category == fcInfinity || category == fcNaN)
    తిరిగి opInvalidOp;

  గుర్తులేని dst_Parts_లెక్క = part_లెక్క_For_బిట్లు(వెడల్పు_2);
  ప్రతిజ్ఞా(dst_Parts_లెక్క <= parts.కొలత_2() && "Integer too big");

  ఒకవేళ (category == fcZero) {
    APInt::tc_సమితి(parts.దత్తాంశం(), 0, dst_Parts_లెక్క);
    // Negative zero can't be represented as an int.
    *isExact = !sign;
    తిరిగి opOK;
  }

  స్థిరరాశి integerPart *src = significandParts();

  గుర్తులేని truncated_బిట్లు;
  /* Step 1: place our absolute value, with any fraction truncated, in
     the destination.  */
  ఒకవేళ (exponent < 0) {
    /* Our absolute value is less than one; truncate everything.  */
    APInt::tc_సమితి(parts.దత్తాంశం(), 0, dst_Parts_లెక్క);
    /* For exponent -1 the integer bit represents .5, look at that.
       For smaller exponents leftmost truncated bit is 0. */
    truncated_బిట్లు = semantics->precision -1U - exponent;
  } లేకపోతే {
    /* We want the most significant (exponent + 1) bits; the rest are
       truncated.  */
    గుర్తులేని పూర్ణాంకం బిట్లు = exponent + 1U;

    /* Hopelessly large in magnitude?  */
    ఒకవేళ (బిట్లు > వెడల్పు_2)
      తిరిగి opInvalidOp;

    ఒకవేళ (బిట్లు < semantics->precision) {
      /* We truncate (semantics->precision - bits) bits.  */
      truncated_బిట్లు = semantics->precision - బిట్లు;
      APInt::tc_వెలికితీయి(parts.దత్తాంశం(), dst_Parts_లెక్క, src, బిట్లు, truncated_బిట్లు);
    } లేకపోతే {
      /* We want at least as many bits as are available.  */
      APInt::tc_వెలికితీయి(parts.దత్తాంశం(), dst_Parts_లెక్క, src, semantics->precision,
                       0);
      APInt::tc_జరుపు_ఎడమ(parts.దత్తాంశం(), dst_Parts_లెక్క,
                         బిట్లు - semantics->precision);
      truncated_బిట్లు = 0;
    }
  }

  /* Step 2: work out any lost fraction, and increment the absolute
     value if we would round away from zero.  */
  కోల్పోయిన_Fraction కోల్పోయిన_fraction;
  ఒకవేళ (truncated_బిట్లు) {
    కోల్పోయిన_fraction = కోల్పోయిన_Fraction_Through_Truncation(src, part_లెక్క(),
                                                  truncated_బిట్లు);
    ఒకవేళ (కోల్పోయిన_fraction != lfExactlyZero &&
        విడత_Away_From_Zero(rounding_బహుళకం_2, కోల్పోయిన_fraction, truncated_బిట్లు)) {
      ఒకవేళ (APInt::tcIncrement(parts.దత్తాంశం(), dst_Parts_లెక్క))
        తిరిగి opInvalidOp;     /* Overflow.  */
    }
  } లేకపోతే {
    కోల్పోయిన_fraction = lfExactlyZero;
  }

  /* Step 3: check if we fit in the destination.  */
  గుర్తులేని పూర్ణాంకం omsb = APInt::tcMSB(parts.దత్తాంశం(), dst_Parts_లెక్క) + 1;

  ఒకవేళ (sign) {
    ఒకవేళ (!isSigned) {
      /* Negative numbers cannot be represented as unsigned.  */
      ఒకవేళ (omsb != 0)
        తిరిగి opInvalidOp;
    } లేకపోతే {
      /* It takes omsb bits to represent the unsigned integer value.
         We lose a bit for the sign, but care is needed as the
         maximally negative integer is a special case.  */
      ఒకవేళ (omsb == వెడల్పు_2 &&
          APInt::tcLSB(parts.దత్తాంశం(), dst_Parts_లెక్క) + 1 != omsb)
        తిరిగి opInvalidOp;

      /* This case can happen because of rounding.  */
      ఒకవేళ (omsb > వెడల్పు_2)
        తిరిగి opInvalidOp;
    }

    APInt::tcNegate (parts.దత్తాంశం(), dst_Parts_లెక్క);
  } లేకపోతే {
    ఒకవేళ (omsb >= వెడల్పు_2 + !isSigned)
      తిరిగి opInvalidOp;
  }

  ఒకవేళ (కోల్పోయిన_fraction == lfExactlyZero) {
    *isExact = నిజం;
    తిరిగి opOK;
  }
  తిరిగి opInexact;
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
APFloat::op_స్థితి
IEEEFloat::convert_గమ్యం_Integer(MutableArrayRef<integerPart> parts,
                            గుర్తులేని పూర్ణాంకం వెడల్పు_2, తార్కిక isSigned,
                            rounding_బహుళకం rounding_బహుళకం_2, తార్కిక *isExact) స్థిరరాశి {
  op_స్థితి fs = convert_గమ్యం_Sign_Extended_Integer(parts, వెడల్పు_2, isSigned,
                                             rounding_బహుళకం_2, isExact);

  ఒకవేళ (fs == opInvalidOp) {
    గుర్తులేని పూర్ణాంకం బిట్లు, dst_Parts_లెక్క;

    dst_Parts_లెక్క = part_లెక్క_For_బిట్లు(వెడల్పు_2);
    ప్రతిజ్ఞా(dst_Parts_లెక్క <= parts.కొలత_2() && "Integer too big");

    ఒకవేళ (category == fcNaN)
      బిట్లు = 0;
    లేకపోతే ఒకవేళ (sign)
      బిట్లు = isSigned;
    లేకపోతే
      బిట్లు = వెడల్పు_2 - isSigned;

    tc_సమితి_కనిష్ఠ_Significant_బిట్లు(parts.దత్తాంశం(), dst_Parts_లెక్క, బిట్లు);
    ఒకవేళ (sign && isSigned)
      APInt::tc_జరుపు_ఎడమ(parts.దత్తాంశం(), dst_Parts_లెక్క, వెడల్పు_2 - 1);
  }

  తిరిగి fs;
}

/* Convert an unsigned integer SRC to a floating point number,
   rounding according to ROUNDING_MODE.  The sign of the floating
   point number is not modified.  */
APFloat::op_స్థితి IEEEFloat::convertFromUnsignedParts(
    స్థిరరాశి integerPart *src, గుర్తులేని పూర్ణాంకం src_లెక్క, rounding_బహుళకం rounding_బహుళకం_2) {
  category = fc_సాధారణ;
  గుర్తులేని omsb = APInt::tcMSB(src, src_లెక్క) + 1;
  integerPart *dst = significandParts();
  గుర్తులేని dst_లెక్క = part_లెక్క();
  గుర్తులేని precision = semantics->precision;

  /* We want the most significant PRECISION bits of SRC.  There may not
     be that many; extract what we can.  */
  కోల్పోయిన_Fraction కోల్పోయిన_fraction;
  ఒకవేళ (precision <= omsb) {
    exponent = omsb - 1;
    కోల్పోయిన_fraction = కోల్పోయిన_Fraction_Through_Truncation(src, src_లెక్క,
                                                  omsb - precision);
    APInt::tc_వెలికితీయి(dst, dst_లెక్క, src, precision, omsb - precision);
  } లేకపోతే {
    exponent = precision - 1;
    కోల్పోయిన_fraction = lfExactlyZero;
    APInt::tc_వెలికితీయి(dst, dst_లెక్క, src, omsb, 0);
  }

  తిరిగి normalize(rounding_బహుళకం_2, కోల్పోయిన_fraction);
}

APFloat::op_స్థితి IEEEFloat::convertFromAPInt(స్థిరరాశి APInt &Val, తార్కిక isSigned,
                                              rounding_బహుళకం rounding_బహుళకం_2) {
  గుర్తులేని పూర్ణాంకం part_లెక్క = Val.get_అంకె_పదాలు();
  APInt api = Val;

  sign = అబద్ధం;
  ఒకవేళ (isSigned && api.isNegative()) {
    sign = నిజం;
    api = -api;
  }

  తిరిగి convertFromUnsignedParts(api.get_Raw_దత్తాంశం(), part_లెక్క, rounding_బహుళకం_2);
}

Expected<APFloat::op_స్థితి>
IEEEFloat::convertFromHexadecimalString(StringRef s,
                                        rounding_బహుళకం rounding_బహుళకం_2) {
  కోల్పోయిన_Fraction కోల్పోయిన_fraction = lfExactlyZero;

  category = fc_సాధారణ;
  zeroSignificand();
  exponent = 0;

  integerPart *significand = significandParts();
  గుర్తులేని parts_లెక్క = part_లెక్క();
  గుర్తులేని బిట్టు_Pos = parts_లెక్క * integer_Part_వెడల్పు;
  తార్కిక computedTrailingFraction = అబద్ధం;

  // Skip leading zeroes and any (hexa)decimal point.
  StringRef::iterator begin = s.begin();
  StringRef::iterator చివర = s.చివర();
  StringRef::iterator dot;
  స్వయంచాలక PtrOrErr = skipLeadingZeroesAndAnyDot(begin, చివర, &dot);
  ఒకవేళ (!PtrOrErr)
    తిరిగి PtrOrErr.takeError();
  StringRef::iterator p = *PtrOrErr;
  StringRef::iterator మొదటి_Significant_Digit = p;

  అయితే (p != చివర) {
    integerPart hex_విలువ;

    ఒకవేళ (*p == '.') {
      ఒకవేళ (dot != చివర)
        తిరిగి createError("String contains multiple dots");
      dot = p++;
      కొనసాగు;
    }

    hex_విలువ = hex_Digit_విలువ(*p);
    ఒకవేళ (hex_విలువ == UINT_MAX)
      ఆపు;

    p++;

    // Store the number while we have space.
    ఒకవేళ (బిట్టు_Pos) {
      బిట్టు_Pos -= 4;
      hex_విలువ <<= బిట్టు_Pos % integer_Part_వెడల్పు;
      significand[బిట్టు_Pos / integer_Part_వెడల్పు] |= hex_విలువ;
    } లేకపోతే ఒకవేళ (!computedTrailingFraction) {
      స్వయంచాలక FractOrErr = trailingHexadecimalFraction(p, చివర, hex_విలువ);
      ఒకవేళ (!FractOrErr)
        తిరిగి FractOrErr.takeError();
      కోల్పోయిన_fraction = *FractOrErr;
      computedTrailingFraction = నిజం;
    }
  }

  /* Hex floats require an exponent but not a hexadecimal point.  */
  ఒకవేళ (p == చివర)
    తిరిగి createError("Hex strings require an exponent");
  ఒకవేళ (*p != 'p' && *p != 'P')
    తిరిగి createError("Invalid character in significand");
  ఒకవేళ (p == begin)
    తిరిగి createError("Significand has no digits");
  ఒకవేళ (dot != చివర && p - begin == 1)
    తిరిగి createError("Significand has no digits");

  /* Ignore the exponent if we are zero.  */
  ఒకవేళ (p != మొదటి_Significant_Digit) {
    పూర్ణాంకం expAdjustment;

    /* Implicit hexadecimal point?  */
    ఒకవేళ (dot == చివర)
      dot = p;

    /* Calculate the exponent adjustment implicit in the number of
       significant digits.  */
    expAdjustment = static_cast<పూర్ణాంకం>(dot - మొదటి_Significant_Digit);
    ఒకవేళ (expAdjustment < 0)
      expAdjustment++;
    expAdjustment = expAdjustment * 4 - 1;

    /* Adjust for writing the significand starting at the most
       significant nibble.  */
    expAdjustment += semantics->precision;
    expAdjustment -= parts_లెక్క * integer_Part_వెడల్పు;

    /* Adjust for the given exponent.  */
    స్వయంచాలక ExpOrErr = మొత్తం_Exponent(p + 1, చివర, expAdjustment);
    ఒకవేళ (!ExpOrErr)
      తిరిగి ExpOrErr.takeError();
    exponent = *ExpOrErr;
  }

  తిరిగి normalize(rounding_బహుళకం_2, కోల్పోయిన_fraction);
}

APFloat::op_స్థితి
IEEEFloat::విడత_Significand_With_Exponent(స్థిరరాశి integerPart *decSigParts,
                                        గుర్తులేని sig_Part_లెక్క, పూర్ణాంకం ఘాతాంకం,
                                        rounding_బహుళకం rounding_బహుళకం_2) {
  fltSemantics calcSemantics = { 32767, -32767, 0, 0 };
  గుర్తులేని పూర్ణాంకం power = ఘాతాంకం >= 0 ? ఘాతాంకం : -ఘాతాంకం;
  గుర్తులేని పూర్ణాంకం pow5_Part_సామర్థ్యం = power_యొక్క_Five_Parts_For_Exponent(power);
  Small_సదిశ<integerPart, అంకె_Pow5_Parts_On_దొంతర> pow5Parts;
  pow5Parts.resize_for_overwrite(pow5_Part_సామర్థ్యం);

  తార్కిక is_సమీప = rounding_బహుళకం_2 == rm_సమీప_Ties_గమ్యం_Even ||
                   rounding_బహుళకం_2 == rm_సమీప_Ties_గమ్యం_Away;

  గుర్తులేని parts = part_లెక్క_For_బిట్లు(semantics->precision + 11);

  /* Calculate pow(5, abs(exp)).  */
  గుర్తులేని pow5_Part_లెక్క = powerOf5(pow5Parts.దత్తాంశం(), power);

  కొరకు (;; parts *= 2) {
    గుర్తులేని పూర్ణాంకం excessPrecision, truncated_బిట్లు;

    calcSemantics.precision = parts * integer_Part_వెడల్పు - 1;
    excessPrecision = calcSemantics.precision - semantics->precision;
    truncated_బిట్లు = excessPrecision;

    IEEEFloat decSig(calcSemantics, uninitialized);
    decSig.makeZero(sign);
    IEEEFloat pow5(calcSemantics);

    op_స్థితి sig_స్థితి = decSig.convertFromUnsignedParts(
        decSigParts, sig_Part_లెక్క, rm_సమీప_Ties_గమ్యం_Even);
    op_స్థితి pow_స్థితి = pow5.convertFromUnsignedParts(
        pow5Parts.దత్తాంశం(), pow5_Part_లెక్క, rm_సమీప_Ties_గమ్యం_Even);
    /* Add exp, as 10^n = 5^n * 2^n.  */
    decSig.exponent += ఘాతాంకం;

    కోల్పోయిన_Fraction calc_కోల్పోయిన_Fraction;
    integerPart HUerr, HUdistance;
    గుర్తులేని పూర్ణాంకం powHUerr;

    ఒకవేళ (ఘాతాంకం >= 0) {
      /* multiplySignificand leaves the precision-th bit set to 1.  */
      calc_కోల్పోయిన_Fraction = decSig.multiplySignificand(pow5);
      powHUerr = pow_స్థితి != opOK;
    } లేకపోతే {
      calc_కోల్పోయిన_Fraction = decSig.divideSignificand(pow5);
      /* Denormal numbers have less precision.  */
      ఒకవేళ (decSig.exponent < semantics->minExponent) {
        excessPrecision += (semantics->minExponent - decSig.exponent);
        truncated_బిట్లు = excessPrecision;
        excessPrecision = std::min(excessPrecision, calcSemantics.precision);
      }
      /* Extra half-ulp lost in reciprocal of exponent.  */
      powHUerr = (pow_స్థితి == opOK && calc_కోల్పోయిన_Fraction == lfExactlyZero) ? 0:2;
    }

    /* Both multiplySignificand and divideSignificand return the
       result with the integer bit set.  */
    ప్రతిజ్ఞా(APInt::tc_వెలికితీయి_బిట్టు
           (decSig.significandParts(), calcSemantics.precision - 1) == 1);

    HUerr = HUerrBound(calc_కోల్పోయిన_Fraction != lfExactlyZero, sig_స్థితి != opOK,
                       powHUerr);
    HUdistance = 2 * ulpsFromBoundary(decSig.significandParts(),
                                      excessPrecision, is_సమీప);

    /* Are we guaranteed to round correctly if we truncate?  */
    ఒకవేళ (HUdistance >= HUerr) {
      APInt::tc_వెలికితీయి(significandParts(), part_లెక్క(), decSig.significandParts(),
                       calcSemantics.precision - excessPrecision,
                       excessPrecision);
      /* Take the exponent of decSig.  If we tcExtract-ed less bits
         above we must adjust our exponent to compensate for the
         implicit right shift.  */
      exponent = (decSig.exponent + semantics->precision
                  - (calcSemantics.precision - excessPrecision));
      calc_కోల్పోయిన_Fraction = కోల్పోయిన_Fraction_Through_Truncation(decSig.significandParts(),
                                                       decSig.part_లెక్క(),
                                                       truncated_బిట్లు);
      తిరిగి static_cast<op_స్థితి>(normalize(rounding_బహుళకం_2, calc_కోల్పోయిన_Fraction) |
                                   ((sig_స్థితి | pow_స్థితి) & opInexact));
    }
  }
}

Expected<APFloat::op_స్థితి>
IEEEFloat::convertFromDecimalString(StringRef str, rounding_బహుళకం rounding_బహుళకం_2) {
  decimalInfo D;
  op_స్థితి fs;

  /* Scan the text.  */
  StringRef::iterator p = str.begin();
  ఒకవేళ (Error Err = interpretDecimal(p, str.చివర(), &D))
    తిరిగి std::move(Err);

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
  ఒకవేళ (D.మొదటి_Sig_Digit == str.చివర() || dec_Digit_విలువ(*D.మొదటి_Sig_Digit) >= 10U) {
    category = fcZero;
    fs = opOK;
    ఒకవేళ (semantics->nanEncoding == fltNanEncoding::NegativeZero)
      sign = అబద్ధం;
    ఒకవేళ (!semantics->hasZero)
      make_అతిచిన్న_Normalized(అబద్ధం);

    /* Check whether the normalized exponent is high enough to overflow
       max during the log-rebasing in the max-exponent check below. */
  } లేకపోతే ఒకవేళ (D.normalizedExponent - 1 > గరిష్ఠ_పూర్ణాంకం / 42039) {
    fs = handle_అధికప్రవాహం(rounding_బహుళకం_2);

  /* If it wasn't, then it also wasn't high enough to overflow max
     during the log-rebasing in the min-exponent check.  Check that it
     won't overflow min in either check, then perform the min-exponent
     check. */
  } లేకపోతే ఒకవేళ (D.normalizedExponent - 1 < INT_MIN / 42039 ||
             (D.normalizedExponent + 1) * 28738 <=
               8651 * (semantics->minExponent - (పూర్ణాంకం) semantics->precision)) {
    /* Underflow to zero and round.  */
    category = fc_సాధారణ;
    zeroSignificand();
    fs = normalize(rounding_బహుళకం_2, lfLessThanHalf);

  /* We can finally safely perform the max-exponent check. */
  } లేకపోతే ఒకవేళ ((D.normalizedExponent - 1) * 42039
             >= 12655 * semantics->maxExponent) {
    /* Overflow and round.  */
    fs = handle_అధికప్రవాహం(rounding_బహుళకం_2);
  } లేకపోతే {
    integerPart *decSignificand;
    గుర్తులేని పూర్ణాంకం part_లెక్క;

    /* A tight upper bound on number of bits required to hold an
       N-digit decimal integer is N * 196 / 59.  Allocate enough space
       to hold the full significand, and an extra part required by
       tcMultiplyPart.  */
    part_లెక్క = static_cast<గుర్తులేని పూర్ణాంకం>(D.చివరి_Sig_Digit - D.మొదటి_Sig_Digit) + 1;
    part_లెక్క = part_లెక్క_For_బిట్లు(1 + 196 * part_లెక్క / 59);
    decSignificand = కొత్త integerPart[part_లెక్క + 1];
    part_లెక్క = 0;

    /* Convert to binary efficiently - we do almost all multiplication
       in an integerPart.  When this would overflow do we do a single
       bignum multiplication, and then revert again to multiplication
       in an integerPart.  */
    చేయి {
      integerPart dec_విలువ, val, multiplier;

      val = 0;
      multiplier = 1;

      చేయి {
        ఒకవేళ (*p == '.') {
          p++;
          ఒకవేళ (p == str.చివర()) {
            ఆపు;
          }
        }
        dec_విలువ = dec_Digit_విలువ(*p++);
        ఒకవేళ (dec_విలువ >= 10U) {
          తొలగించు[] decSignificand;
          తిరిగి createError("Invalid character in significand");
        }
        multiplier *= 10;
        val = val * 10 + dec_విలువ;
        /* The maximum number that can be multiplied by ten with any
           digit added without overflowing an integerPart.  */
      } అయితే (p <= D.చివరి_Sig_Digit && multiplier <= (~ (integerPart) 0 - 9) / 10);

      /* Multiply out the current part.  */
      APInt::tcMultiplyPart(decSignificand, decSignificand, multiplier, val,
                            part_లెక్క, part_లెక్క + 1, అబద్ధం);

      /* If we used another part (likely but not guaranteed), increase
         the count.  */
      ఒకవేళ (decSignificand[part_లెక్క])
        part_లెక్క++;
    } అయితే (p <= D.చివరి_Sig_Digit);

    category = fc_సాధారణ;
    fs = విడత_Significand_With_Exponent(decSignificand, part_లెక్క,
                                      D.exponent, rounding_బహుళకం_2);

    తొలగించు [] decSignificand;
  }

  తిరిగి fs;
}

తార్కిక IEEEFloat::convertFromStringSpecials(StringRef str) {
  స్థిరరాశి size_t MIN_NAME_కొలత = 3;

  ఒకవేళ (str.కొలత_2() < MIN_NAME_కొలత)
    తిరిగి అబద్ధం;

  ఒకవేళ (str == "inf" || str == "INFINITY" || str == "+Inf" || str == "+inf") {
    makeInf(అబద్ధం);
    తిరిగి నిజం;
  }

  తార్కిక IsNegative = str.consume_front("-");
  ఒకవేళ (IsNegative) {
    ఒకవేళ (str.కొలత_2() < MIN_NAME_కొలత)
      తిరిగి అబద్ధం;

    ఒకవేళ (str == "inf" || str == "INFINITY" || str == "Inf") {
      makeInf(నిజం);
      తిరిగి నిజం;
    }
  }

  // If we have a 's' (or 'S') prefix, then this is a Signaling NaN.
  తార్కిక IsSignaling = str.consume_front_insensitive("s");
  ఒకవేళ (IsSignaling) {
    ఒకవేళ (str.కొలత_2() < MIN_NAME_కొలత)
      తిరిగి అబద్ధం;
  }

  ఒకవేళ (str.consume_front("nan") || str.consume_front("NaN")) {
    // A NaN without payload.
    ఒకవేళ (str.ఖాళీలు()) {
      makeNaN(IsSignaling, IsNegative);
      తిరిగి నిజం;
    }

    // Allow the payload to be inside parentheses.
    ఒకవేళ (str.front() == '(') {
      // Parentheses should be balanced (and not empty).
      ఒకవేళ (str.కొలత_2() <= 2 || str.వెనుక() != ')')
        తిరిగి అబద్ధం;

      str = str.ముక్క(1, str.కొలత_2() - 1);
    }

    // Determine the payload number's radix.
    గుర్తులేని Radix = 10;
    ఒకవేళ (str[0] == '0') {
      ఒకవేళ (str.కొలత_2() > 1 && tolower(str[1]) == 'x') {
        str = str.drop_front(2);
        Radix = 16;
      } లేకపోతే {
        Radix = 8;
      }
    }

    // Parse the payload and make the NaN.
    APInt Payload;
    ఒకవేళ (!str.getAsInteger(Radix, Payload)) {
      makeNaN(IsSignaling, IsNegative, &Payload);
      తిరిగి నిజం;
    }
  }

  తిరిగి అబద్ధం;
}

Expected<APFloat::op_స్థితి>
IEEEFloat::convertFromString(StringRef str, rounding_బహుళకం rounding_బహుళకం_2) {
  ఒకవేళ (str.ఖాళీలు())
    తిరిగి createError("Invalid string length");

  // Handle special cases.
  ఒకవేళ (convertFromStringSpecials(str))
    తిరిగి opOK;

  /* Handle a leading minus sign.  */
  StringRef::iterator p = str.begin();
  size_t slen = str.కొలత_2();
  sign = *p == '-' ? 1 : 0;
  ఒకవేళ (sign && !semantics->hasSignedRepr)
    llvm_unreachable(
        "This floating point format does not support signed values");

  ఒకవేళ (*p == '-' || *p == '+') {
    p++;
    slen--;
    ఒకవేళ (!slen)
      తిరిగి createError("String has no digits");
  }

  ఒకవేళ (slen >= 2 && p[0] == '0' && (p[1] == 'x' || p[1] == 'X')) {
    ఒకవేళ (slen == 2)
      తిరిగి createError("Invalid string");
    తిరిగి convertFromHexadecimalString(StringRef(p + 2, slen - 2),
                                        rounding_బహుళకం_2);
  }

  తిరిగి convertFromDecimalString(StringRef(p, slen), rounding_బహుళకం_2);
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
గుర్తులేని పూర్ణాంకం IEEEFloat::convert_గమ్యం_Hex_String(అక్షరం *dst, గుర్తులేని పూర్ణాంకం hexDigits,
                                           తార్కిక పై_Case,
                                           rounding_బహుళకం rounding_బహుళకం_2) స్థిరరాశి {
  అక్షరం *p = dst;
  ఒకవేళ (sign)
    *dst++ = '-';

  ఎంపిక (category) {
  స్థితి fcInfinity:
    జ్ఞాపకం_నకలు (dst, పై_Case ? infinityU: infinityL, పరిమాణం infinityU - 1);
    dst += పరిమాణం infinityL - 1;
    ఆపు;

  స్థితి fcNaN:
    జ్ఞాపకం_నకలు (dst, పై_Case ? NaNU: NaNL, పరిమాణం NaNU - 1);
    dst += పరిమాణం NaNU - 1;
    ఆపు;

  స్థితి fcZero:
    *dst++ = '0';
    *dst++ = పై_Case ? 'X': 'x';
    *dst++ = '0';
    ఒకవేళ (hexDigits > 1) {
      *dst++ = '.';
      memset (dst, '0', hexDigits - 1);
      dst += hexDigits - 1;
    }
    *dst++ = పై_Case ? 'P': 'p';
    *dst++ = '0';
    ఆపు;

  స్థితి fc_సాధారణ:
    dst = convert_సాధారణ_గమ్యం_Hex_String (dst, hexDigits, పై_Case, rounding_బహుళకం_2);
    ఆపు;
  }

  *dst = 0;

  తిరిగి static_cast<గుర్తులేని పూర్ణాంకం>(dst - p);
}

/* Does the hard work of outputting the correctly rounded hexadecimal
   form of a normal floating point number with the specified number of
   hexadecimal digits.  If HEXDIGITS is zero the minimum number of
   digits necessary to print the value precisely is output.  */
అక్షరం *IEEEFloat::convert_సాధారణ_గమ్యం_Hex_String(అక్షరం *dst, గుర్తులేని పూర్ణాంకం hexDigits,
                                          తార్కిక పై_Case,
                                          rounding_బహుళకం rounding_బహుళకం_2) స్థిరరాశి {
  *dst++ = '0';
  *dst++ = పై_Case ? 'X': 'x';

  తార్కిక విడత_పైకి = అబద్ధం;
  స్థిరరాశి అక్షరం *hexDigitChars = పై_Case ? hex_Digits_పై : hex_Digits_క్రింది;

  స్థిరరాశి integerPart *significand = significandParts();
  గుర్తులేని parts_లెక్క = part_లెక్క();

  /* +3 because the first digit only uses the single integer bit, so
     we have 3 virtual zero most-significant-bits.  */
  గుర్తులేని విలువ_బిట్లు = semantics->precision + 3;
  గుర్తులేని జరుపు = integer_Part_వెడల్పు - విలువ_బిట్లు % integer_Part_వెడల్పు;

  /* The natural number of digits required ignoring trailing
     insignificant zeroes.  */
  గుర్తులేని outputDigits = (విలువ_బిట్లు - significandLSB() + 3) / 4;

  /* hexDigits of zero means use the required number for the
     precision.  Otherwise, see if we are truncating.  If we are,
     find out if we need to round away from zero.  */
  ఒకవేళ (hexDigits) {
    ఒకవేళ (hexDigits < outputDigits) {
      /* We are dropping non-zero bits, so need to check how to round.
         "bits" is the number of dropped bits.  */
      గుర్తులేని పూర్ణాంకం బిట్లు;
      కోల్పోయిన_Fraction fraction;

      బిట్లు = విలువ_బిట్లు - hexDigits * 4;
      fraction = కోల్పోయిన_Fraction_Through_Truncation (significand, parts_లెక్క, బిట్లు);
      విడత_పైకి = విడత_Away_From_Zero(rounding_బహుళకం_2, fraction, బిట్లు);
    }
    outputDigits = hexDigits;
  }

  /* Write the digits consecutively, and start writing in the location
     of the hexadecimal point.  We move the most significant digit
     left and add the hexadecimal point later.  */
  అక్షరం *p = ++dst;

  గుర్తులేని లెక్క = (విలువ_బిట్లు + integer_Part_వెడల్పు - 1) / integer_Part_వెడల్పు;

  అయితే (outputDigits && లెక్క) {
    integerPart part;

    /* Put the most significant integerPartWidth bits in "part".  */
    ఒకవేళ (--లెక్క == parts_లెక్క)
      part = 0;  /* An imaginary higher zero part.  */
    లేకపోతే
      part = significand[లెక్క] << జరుపు;

    ఒకవేళ (లెక్క && జరుపు)
      part |= significand[లెక్క - 1] >> (integer_Part_వెడల్పు - జరుపు);

    /* Convert as much of "part" to hexdigits as we can.  */
    గుర్తులేని పూర్ణాంకం curDigits = integer_Part_వెడల్పు / 4;

    curDigits = std::min(curDigits, outputDigits);
    dst += partAsHex (dst, part, curDigits, hexDigitChars);
    outputDigits -= curDigits;
  }

  ఒకవేళ (విడత_పైకి) {
    అక్షరం *q = dst;

    /* Note that hexDigitChars has a trailing '0'.  */
    చేయి {
      q--;
      *q = hexDigitChars[hex_Digit_విలువ (*q) + 1];
    } అయితే (*q == '0');
    ప్రతిజ్ఞా(q >= p);
  } లేకపోతే {
    /* Add trailing zeroes.  */
    memset (dst, '0', outputDigits);
    dst += outputDigits;
  }

  /* Move the most significant digit to before the point, and if there
     is something after the decimal point add it.  This must come
     after rounding above.  */
  p[-1] = p[0];
  ఒకవేళ (dst -1 == p)
    dst--;
  లేకపోతే
    p[0] = '.';

  /* Finally output the exponent.  */
  *dst++ = పై_Case ? 'P': 'p';

  తిరిగి writeSignedDecimal (dst, exponent);
}

hash_సంకేతం hash_విలువ(స్థిరరాశి IEEEFloat &Arg) {
  ఒకవేళ (!Arg.isFiniteNonZero())
    తిరిగి hash_combine((uint8_t)Arg.category,
                        // NaN has no sign, fix it at zero.
                        Arg.isNaN() ? (uint8_t)0 : (uint8_t)Arg.sign,
                        Arg.semantics->precision);

  // Normal floats need their exponent and significand hashed.
  తిరిగి hash_combine((uint8_t)Arg.category, (uint8_t)Arg.sign,
                      Arg.semantics->precision, Arg.exponent,
                      hash_combine_range(
                        Arg.significandParts(),
                        Arg.significandParts() + Arg.part_లెక్క()));
}

// Conversion from APFloat to/from host float/double.  It may eventually be
// possible to eliminate these and have everybody deal with APFloats, but that
// will take a while.  This approach will not easily extend to long double.
// Current implementation requires integerPartWidth==64, which is correct at
// the moment but could be made more general.

// Denormals have exponent minExponent in APFloat, but minExponent-1 in
// the actual IEEE respresentations.  We compensate for that here.

APInt IEEEFloat::convert_F80_Long_Double_APFloat_గమ్యం_APInt() స్థిరరాశి {
  ప్రతిజ్ఞా(part_లెక్క() == 2);
  తిరిగి convert_IEEEFloat_గమ్యం_APInt<APFloat_పునాది::semX87DoubleExtended>();
}

APInt IEEEFloat::convert_PPCDouble_Double_Legacy_APFloat_గమ్యం_APInt() స్థిరరాశి {
  ప్రతిజ్ఞా(semantics ==
         (స్థిరరాశి llvm::fltSemantics *)&APFloat_పునాది::semPPCDoubleDoubleLegacy);
  ప్రతిజ్ఞా(part_లెక్క()==2);

  uint64_t పదాలు[2];
  తార్కిక losesInfo;

  // Convert number to double.  To avoid spurious underflows, we re-
  // normalize against the "double" minExponent first, and only *then*
  // truncate the mantissa.  The result of that second conversion
  // may be inexact, but should never underflow.
  // Declare fltSemantics before APFloat that uses it (and
  // saves pointer to it) to ensure correct destruction order.
  fltSemantics extendedSemantics = *semantics;
  extendedSemantics.minExponent = APFloat_పునాది::semIEEEdouble.minExponent;
  IEEEFloat extended(*ఇది);
  [[maybe_వాడని]] op_స్థితి fs =
      extended.convert(extendedSemantics, rm_సమీప_Ties_గమ్యం_Even, &losesInfo);
  ప్రతిజ్ఞా(fs == opOK && !losesInfo);

  IEEEFloat u(extended);
  fs = u.convert(APFloat_పునాది::semIEEEdouble, rm_సమీప_Ties_గమ్యం_Even, &losesInfo);
  ప్రతిజ్ఞా(fs == opOK || fs == opInexact);
  పదాలు[0] = *u.convert_Double_APFloat_గమ్యం_APInt().get_Raw_దత్తాంశం();

  // If conversion was exact or resulted in a special case, we're done;
  // just set the second double to zero.  Otherwise, re-convert back to
  // the extended format and compute the difference.  This now should
  // convert exactly to double.
  ఒకవేళ (u.isFiniteNonZero() && losesInfo) {
    fs = u.convert(extendedSemantics, rm_సమీప_Ties_గమ్యం_Even, &losesInfo);
    ప్రతిజ్ఞా(fs == opOK && !losesInfo);

    IEEEFloat v(extended);
    v.వ్యవకలనం(u, rm_సమీప_Ties_గమ్యం_Even);
    fs = v.convert(APFloat_పునాది::semIEEEdouble, rm_సమీప_Ties_గమ్యం_Even, &losesInfo);
    ప్రతిజ్ఞా(fs == opOK && !losesInfo);
    పదాలు[1] = *v.convert_Double_APFloat_గమ్యం_APInt().get_Raw_దత్తాంశం();
  } లేకపోతే {
    పదాలు[1] = 0;
  }

  తిరిగి APInt(128, పదాలు);
}

మూస <స్థిరరాశి fltSemantics &S>
APInt IEEEFloat::convert_IEEEFloat_గమ్యం_APInt() స్థిరరాశి {
  ప్రతిజ్ఞా(semantics == &S);
  constexpr గుర్తులేని పూర్ణాంకం trailing_significand_బిట్లు =
      S.precision - 1 + S.has_Explicit_Integer_బిట్టు;
  constexpr పూర్ణాంకం integer_బిట్టు_part = (S.precision - 1) / integer_Part_వెడల్పు;
  constexpr integerPart integer_బిట్టు = integerPart{1}
                                      << ((S.precision - 1) % integer_Part_వెడల్పు);
  constexpr uint64_t significand_ముసుగు = integer_బిట్టు - 1;
  constexpr గుర్తులేని పూర్ణాంకం exponent_బిట్లు =
      S.కొలత_లోపల_బిట్లు - (S.hasSignedRepr ? 1 : 0) - trailing_significand_బిట్లు;
  static_assert(exponent_బిట్లు < 64);
  constexpr uint64_t exponent_ముసుగు = (uint64_t{1} << exponent_బిట్లు) - 1;
  constexpr తార్కిక is_zero_exp_reserved = S.hasDenormals || S.hasZero;
  constexpr పూర్ణాంకం bias = -(S.minExponent - (is_zero_exp_reserved ? 1 : 0));

  uint64_t myexponent;
  std::array<integerPart, part_లెక్క_For_బిట్లు(trailing_significand_బిట్లు)>
      mysignificand;

  ఒకవేళ (isFiniteNonZero()) {
    myexponent = exponent + bias;
    std::copy_n(significandParts(), mysignificand.కొలత_2(),
                mysignificand.begin());
    ఒకవేళ (myexponent == 1 &&
        !(significandParts()[integer_బిట్టు_part] & integer_బిట్టు))
      myexponent = 0; // denormal
  } లేకపోతే ఒకవేళ (category == fcZero) {
    ఒకవేళ (!S.hasZero)
      llvm_unreachable("semantics does not support zero!");
    myexponent = ::exponentZero(S) + bias;
    mysignificand.fill(0);
  } లేకపోతే ఒకవేళ (category == fcInfinity) {
    ఒకవేళ (S.nonFiniteBehavior == fltNonfiniteBehavior::NanOnly ||
        S.nonFiniteBehavior == fltNonfiniteBehavior::FiniteOnly)
      llvm_unreachable("semantics don't support inf!");
    myexponent = ::exponentInf(S) + bias;
    mysignificand.fill(0);
    ఒకవేళ constexpr (S.has_Explicit_Integer_బిట్టు) {
      mysignificand[0] = integerPart{1} << (trailing_significand_బిట్లు - 1);
    }
  } లేకపోతే {
    ప్రతిజ్ఞా(category == fcNaN && "Unknown category!");
    ఒకవేళ (S.nonFiniteBehavior == fltNonfiniteBehavior::FiniteOnly)
      llvm_unreachable("semantics don't support NaN!");
    myexponent = ::exponentNaN(S) + bias;
    std::copy_n(significandParts(), mysignificand.కొలత_2(),
                mysignificand.begin());
  }
  std::array<uint64_t, (S.కొలత_లోపల_బిట్లు + 63) / 64> పదాలు;
  స్వయంచాలక పదాలు_iter =
      std::copy_n(mysignificand.begin(), mysignificand.కొలత_2(), పదాలు.begin());
  ఒకవేళ constexpr (!S.has_Explicit_Integer_బిట్టు) {
    ఒకవేళ constexpr (significand_ముసుగు != 0 || trailing_significand_బిట్లు == 0) {
      // Clear the integer bit.
      పదాలు[mysignificand.కొలత_2() - 1] &= significand_ముసుగు;
    }
  }
  std::fill(పదాలు_iter, పదాలు.చివర(), uint64_t{0});
  constexpr size_t చివరి_మాట = పదాలు.కొలత_2() - 1;
  ఒకవేళ constexpr (S.hasSignedRepr) {
    uint64_t shifted_sign = static_cast<uint64_t>(sign & 1)
                            << ((S.కొలత_లోపల_బిట్లు - 1) % 64);
    పదాలు[చివరి_మాట] |= shifted_sign;
  }
  uint64_t shifted_exponent = (myexponent & exponent_ముసుగు)
                              << (trailing_significand_బిట్లు % 64);
  పదాలు[చివరి_మాట] |= shifted_exponent;
  ఒకవేళ constexpr (చివరి_మాట == 0) {
    తిరిగి APInt(S.కొలత_లోపల_బిట్లు, పదాలు[0]);
  }
  తిరిగి APInt(S.కొలత_లోపల_బిట్లు, పదాలు);
}

APInt IEEEFloat::convert_Quadruple_APFloat_గమ్యం_APInt() స్థిరరాశి {
  ప్రతిజ్ఞా(part_లెక్క() == 2);
  తిరిగి convert_IEEEFloat_గమ్యం_APInt<APFloat_పునాది::semIEEEquad>();
}

APInt IEEEFloat::convert_Double_APFloat_గమ్యం_APInt() స్థిరరాశి {
  ప్రతిజ్ఞా(part_లెక్క()==1);
  తిరిగి convert_IEEEFloat_గమ్యం_APInt<APFloat_పునాది::semIEEEdouble>();
}

APInt IEEEFloat::convert_Float_APFloat_గమ్యం_APInt() స్థిరరాశి {
  ప్రతిజ్ఞా(part_లెక్క()==1);
  తిరిగి convert_IEEEFloat_గమ్యం_APInt<APFloat_పునాది::semIEEEsingle>();
}

APInt IEEEFloat::convert_BFloat_APFloat_గమ్యం_APInt() స్థిరరాశి {
  ప్రతిజ్ఞా(part_లెక్క() == 1);
  తిరిగి convert_IEEEFloat_గమ్యం_APInt<APFloat_పునాది::semBFloat>();
}

APInt IEEEFloat::convert_Half_APFloat_గమ్యం_APInt() స్థిరరాశి {
  ప్రతిజ్ఞా(part_లెక్క()==1);
  తిరిగి convert_IEEEFloat_గమ్యం_APInt<APFloat_పునాది::APFloat_పునాది::semIEEEhalf>();
}

APInt IEEEFloat::convert_Float8_E5_M2_APFloat_గమ్యం_APInt() స్థిరరాశి {
  ప్రతిజ్ఞా(part_లెక్క() == 1);
  తిరిగి convert_IEEEFloat_గమ్యం_APInt<APFloat_పునాది::semFloat8E5M2>();
}

APInt IEEEFloat::convert_Float8_E5_M2_FNUZAPFloat_గమ్యం_APInt() స్థిరరాశి {
  ప్రతిజ్ఞా(part_లెక్క() == 1);
  తిరిగి convert_IEEEFloat_గమ్యం_APInt<APFloat_పునాది::semFloat8E5M2FNUZ>();
}

APInt IEEEFloat::convert_Float8_E4_M3_APFloat_గమ్యం_APInt() స్థిరరాశి {
  ప్రతిజ్ఞా(part_లెక్క() == 1);
  తిరిగి convert_IEEEFloat_గమ్యం_APInt<APFloat_పునాది::semFloat8E4M3>();
}

APInt IEEEFloat::convert_Float8_E4_M3_FNAPFloat_గమ్యం_APInt() స్థిరరాశి {
  ప్రతిజ్ఞా(part_లెక్క() == 1);
  తిరిగి convert_IEEEFloat_గమ్యం_APInt<APFloat_పునాది::semFloat8E4M3FN>();
}

APInt IEEEFloat::convert_Float8_E4_M3_FNUZAPFloat_గమ్యం_APInt() స్థిరరాశి {
  ప్రతిజ్ఞా(part_లెక్క() == 1);
  తిరిగి convert_IEEEFloat_గమ్యం_APInt<APFloat_పునాది::semFloat8E4M3FNUZ>();
}

APInt IEEEFloat::convert_Float8_E4_M3_B11_FNUZAPFloat_గమ్యం_APInt() స్థిరరాశి {
  ప్రతిజ్ఞా(part_లెక్క() == 1);
  తిరిగి convert_IEEEFloat_గమ్యం_APInt<APFloat_పునాది::semFloat8E4M3B11FNUZ>();
}

APInt IEEEFloat::convert_Float8_E3_M4_APFloat_గమ్యం_APInt() స్థిరరాశి {
  ప్రతిజ్ఞా(part_లెక్క() == 1);
  తిరిగి convert_IEEEFloat_గమ్యం_APInt<APFloat_పునాది::semFloat8E3M4>();
}

APInt IEEEFloat::convert_Float_TF32_APFloat_గమ్యం_APInt() స్థిరరాశి {
  ప్రతిజ్ఞా(part_లెక్క() == 1);
  తిరిగి convert_IEEEFloat_గమ్యం_APInt<APFloat_పునాది::semFloatTF32>();
}

APInt IEEEFloat::convert_Float8_E8_M0_FNUAPFloat_గమ్యం_APInt() స్థిరరాశి {
  ప్రతిజ్ఞా(part_లెక్క() == 1);
  తిరిగి convert_IEEEFloat_గమ్యం_APInt<APFloat_పునాది::semFloat8E8M0FNU>();
}

APInt IEEEFloat::convert_Float8_E5_M3_FNUAPFloat_గమ్యం_APInt() స్థిరరాశి {
  ప్రతిజ్ఞా(part_లెక్క() == 1);
  తిరిగి convert_IEEEFloat_గమ్యం_APInt<APFloat_పునాది::semFloat8E5M3FNU>();
}

APInt IEEEFloat::convert_Float6_E3_M2_FNAPFloat_గమ్యం_APInt() స్థిరరాశి {
  ప్రతిజ్ఞా(part_లెక్క() == 1);
  తిరిగి convert_IEEEFloat_గమ్యం_APInt<APFloat_పునాది::semFloat6E3M2FN>();
}

APInt IEEEFloat::convert_Float6_E2_M3_FNAPFloat_గమ్యం_APInt() స్థిరరాశి {
  ప్రతిజ్ఞా(part_లెక్క() == 1);
  తిరిగి convert_IEEEFloat_గమ్యం_APInt<APFloat_పునాది::semFloat6E2M3FN>();
}

APInt IEEEFloat::convert_Float4_E2_M1_FNAPFloat_గమ్యం_APInt() స్థిరరాశి {
  ప్రతిజ్ఞా(part_లెక్క() == 1);
  తిరిగి convert_IEEEFloat_గమ్యం_APInt<APFloat_పునాది::semFloat4E2M1FN>();
}

// This function creates an APInt that is just a bit map of the floating
// point constant as it would appear in memory.  It is not a conversion,
// and treating the result as a normal integer is unlikely to be useful.

APInt IEEEFloat::bitcast_గమ్యం_APInt() స్థిరరాశి {
  ఒకవేళ (semantics == (స్థిరరాశి llvm::fltSemantics *)&APFloat_పునాది::semIEEEhalf)
    తిరిగి convert_Half_APFloat_గమ్యం_APInt();

  ఒకవేళ (semantics == (స్థిరరాశి llvm::fltSemantics *)&APFloat_పునాది::semBFloat)
    తిరిగి convert_BFloat_APFloat_గమ్యం_APInt();

  ఒకవేళ (semantics == (స్థిరరాశి llvm::fltSemantics *)&APFloat_పునాది::semIEEEsingle)
    తిరిగి convert_Float_APFloat_గమ్యం_APInt();

  ఒకవేళ (semantics == (స్థిరరాశి llvm::fltSemantics *)&APFloat_పునాది::semIEEEdouble)
    తిరిగి convert_Double_APFloat_గమ్యం_APInt();

  ఒకవేళ (semantics == (స్థిరరాశి llvm::fltSemantics *)&APFloat_పునాది::semIEEEquad)
    తిరిగి convert_Quadruple_APFloat_గమ్యం_APInt();

  ఒకవేళ (semantics ==
      (స్థిరరాశి llvm::fltSemantics *)&APFloat_పునాది::semX87DoubleExtended)
    తిరిగి convert_F80_Long_Double_APFloat_గమ్యం_APInt();

  ఒకవేళ (semantics ==
      (స్థిరరాశి llvm::fltSemantics *)&APFloat_పునాది::semPPCDoubleDoubleLegacy)
    తిరిగి convert_PPCDouble_Double_Legacy_APFloat_గమ్యం_APInt();

  ఒకవేళ (semantics == (స్థిరరాశి llvm::fltSemantics *)&APFloat_పునాది::semFloat8E5M2)
    తిరిగి convert_Float8_E5_M2_APFloat_గమ్యం_APInt();

  ఒకవేళ (semantics == (స్థిరరాశి llvm::fltSemantics *)&APFloat_పునాది::semFloat8E5M2FNUZ)
    తిరిగి convert_Float8_E5_M2_FNUZAPFloat_గమ్యం_APInt();

  ఒకవేళ (semantics == (స్థిరరాశి llvm::fltSemantics *)&APFloat_పునాది::semFloat8E4M3)
    తిరిగి convert_Float8_E4_M3_APFloat_గమ్యం_APInt();

  ఒకవేళ (semantics == (స్థిరరాశి llvm::fltSemantics *)&APFloat_పునాది::semFloat8E4M3FN)
    తిరిగి convert_Float8_E4_M3_FNAPFloat_గమ్యం_APInt();

  ఒకవేళ (semantics == (స్థిరరాశి llvm::fltSemantics *)&APFloat_పునాది::semFloat8E4M3FNUZ)
    తిరిగి convert_Float8_E4_M3_FNUZAPFloat_గమ్యం_APInt();

  ఒకవేళ (semantics ==
      (స్థిరరాశి llvm::fltSemantics *)&APFloat_పునాది::semFloat8E4M3B11FNUZ)
    తిరిగి convert_Float8_E4_M3_B11_FNUZAPFloat_గమ్యం_APInt();

  ఒకవేళ (semantics == (స్థిరరాశి llvm::fltSemantics *)&APFloat_పునాది::semFloat8E3M4)
    తిరిగి convert_Float8_E3_M4_APFloat_గమ్యం_APInt();

  ఒకవేళ (semantics == (స్థిరరాశి llvm::fltSemantics *)&APFloat_పునాది::semFloatTF32)
    తిరిగి convert_Float_TF32_APFloat_గమ్యం_APInt();

  ఒకవేళ (semantics == (స్థిరరాశి llvm::fltSemantics *)&APFloat_పునాది::semFloat8E8M0FNU)
    తిరిగి convert_Float8_E8_M0_FNUAPFloat_గమ్యం_APInt();

  ఒకవేళ (semantics == (స్థిరరాశి llvm::fltSemantics *)&APFloat_పునాది::semFloat8E5M3FNU)
    తిరిగి convert_Float8_E5_M3_FNUAPFloat_గమ్యం_APInt();

  ఒకవేళ (semantics == (స్థిరరాశి llvm::fltSemantics *)&APFloat_పునాది::semFloat6E3M2FN)
    తిరిగి convert_Float6_E3_M2_FNAPFloat_గమ్యం_APInt();

  ఒకవేళ (semantics == (స్థిరరాశి llvm::fltSemantics *)&APFloat_పునాది::semFloat6E2M3FN)
    తిరిగి convert_Float6_E2_M3_FNAPFloat_గమ్యం_APInt();

  ఒకవేళ (semantics == (స్థిరరాశి llvm::fltSemantics *)&APFloat_పునాది::semFloat4E2M1FN)
    తిరిగి convert_Float4_E2_M1_FNAPFloat_గమ్యం_APInt();

  llvm_unreachable("unknown format!");
}

దశాంశం IEEEFloat::convert_గమ్యం_Float() స్థిరరాశి {
  ప్రతిజ్ఞా(semantics == (స్థిరరాశి llvm::fltSemantics *)&APFloat_పునాది::semIEEEsingle &&
         "Float semantics are not IEEEsingle");
  APInt api = bitcast_గమ్యం_APInt();
  తిరిగి api.బిట్లు_గమ్యం_Float();
}

ద్విగుణ IEEEFloat::convert_గమ్యం_Double() స్థిరరాశి {
  ప్రతిజ్ఞా(semantics == (స్థిరరాశి llvm::fltSemantics *)&APFloat_పునాది::semIEEEdouble &&
         "Float semantics are not IEEEdouble");
  APInt api = bitcast_గమ్యం_APInt();
  తిరిగి api.బిట్లు_గమ్యం_Double();
}

#నిర్వచితమైతే HAS_IEE754_FLOAT128
float128 IEEEFloat::convert_గమ్యం_Quad() స్థిరరాశి {
  ప్రతిజ్ఞా(semantics == (స్థిరరాశి llvm::fltSemantics *)&APFloat_పునాది::semIEEEquad &&
         "Float semantics are not IEEEquads");
  APInt api = bitcast_గమ్యం_APInt();
  తిరిగి api.బిట్లు_గమ్యం_Quad();
}
#అయితే_ముగింపు

శూన్య IEEEFloat::initFromF80LongDoubleAPInt(స్థిరరాశి APInt &api) {
  తిరిగి initFromIEEEAPInt<APFloat_పునాది::semX87DoubleExtended>(api);
}

శూన్య IEEEFloat::initFromPPCDoubleDoubleLegacyAPInt(స్థిరరాశి APInt &api) {
  uint64_t i1 = api.get_Raw_దత్తాంశం()[0];
  uint64_t i2 = api.get_Raw_దత్తాంశం()[1];
  తార్కిక losesInfo;

  // Get the first double and convert to our format.
  initFromDoubleAPInt(APInt(64, i1));
  [[maybe_వాడని]] op_స్థితి fs = convert(APFloat_పునాది::semPPCDoubleDoubleLegacy,
                                         rm_సమీప_Ties_గమ్యం_Even, &losesInfo);
  // (convert may return opInvalidOp if i1 is an sNaN).
  ప్రతిజ్ఞా((fs == opOK || fs == opInvalidOp) && !losesInfo);

  // Unless we have a special case, add in second double.
  ఒకవేళ (isFiniteNonZero()) {
    IEEEFloat v(APFloat_పునాది::semIEEEdouble, APInt(64, i2));
    // The first double determines the category, so ignore a non-canonical
    // second double.
    ఒకవేళ (v.isFinite()) {
      fs = v.convert(APFloat_పునాది::semPPCDoubleDoubleLegacy, rm_సమీప_Ties_గమ్యం_Even,
                     &losesInfo);
      ప్రతిజ్ఞా(fs == opOK && !losesInfo);

      కూడు(v, rm_సమీప_Ties_గమ్యం_Even);
    }
  }
}

// The E8M0 format has the following characteristics:
// It is an 8-bit unsigned format with only exponents (no actual significand).
// No encodings for {zero, infinities or denorms}.
// NaN is represented by all 1's.
// Bias is 127.
శూన్య IEEEFloat::initFromFloat8E8M0FNUAPInt(స్థిరరాశి APInt &api) {
  initFromIEEEAPInt<APFloat_పునాది::semFloat8E8M0FNU>(api);
}

శూన్య IEEEFloat::initFromFloat8E5M3FNUAPInt(స్థిరరాశి APInt &api) {
  initFromIEEEAPInt<APFloat_పునాది::semFloat8E5M3FNU>(api);
}

మూస <స్థిరరాశి fltSemantics &S>
శూన్య IEEEFloat::initFromIEEEAPInt(స్థిరరాశి APInt &api) {
  ప్రతిజ్ఞా(api.get_బిట్టు_వెడల్పు() == S.కొలత_లోపల_బిట్లు);

  constexpr గుర్తులేని పూర్ణాంకం trailing_significand_బిట్లు =
      S.precision - 1 + S.has_Explicit_Integer_బిట్టు;
  constexpr integerPart integer_బిట్టు =
      integerPart{1} << (trailing_significand_బిట్లు % integer_Part_వెడల్పు);
  constexpr uint64_t significand_ముసుగు = integer_బిట్టు - 1;
  constexpr గుర్తులేని పూర్ణాంకం exponent_బిట్లు =
      S.కొలత_లోపల_బిట్లు - (S.hasSignedRepr ? 1 : 0) - trailing_significand_బిట్లు;
  static_assert(exponent_బిట్లు < 64);
  constexpr గుర్తులేని పూర్ణాంకం stored_significand_parts =
      part_లెక్క_For_బిట్లు(trailing_significand_బిట్లు + 1);
  constexpr uint64_t exponent_ముసుగు = (uint64_t{1} << exponent_బిట్లు) - 1;
  constexpr తార్కిక is_zero_exp_reserved = S.hasDenormals || S.hasZero;
  constexpr పూర్ణాంకం bias = -(S.minExponent - (is_zero_exp_reserved ? 1 : 0));
  constexpr తార్కిక has_significand = trailing_significand_బిట్లు > 0;

  // Copy the bits of the significand. We need to clear out the exponent and
  // sign bit in the last word.
  std::array<integerPart, stored_significand_parts> mysignificand;
  ఒకవేళ constexpr (has_significand) {
    std::copy_n(api.get_Raw_దత్తాంశం(), mysignificand.కొలత_2(), mysignificand.begin());
    ఒకవేళ constexpr (significand_ముసుగు != 0 || S.precision >= integer_Part_వెడల్పు) {
      mysignificand[mysignificand.కొలత_2() - 1] &= significand_ముసుగు;
    }
  } లేకపోతే {
    std::fill_n(mysignificand.begin(), mysignificand.కొలత_2(), 0);
    // Always set integer bit to 1 for consistency in APFloat's internal
    // representation.
    mysignificand[0] = 1;
  }

  // We assume the last word holds the sign bit, the exponent, and potentially
  // some of the trailing significand field.
  uint64_t చివరి_మాట = api.get_Raw_దత్తాంశం()[api.get_అంకె_పదాలు() - 1];
  uint64_t myexponent =
      (చివరి_మాట >> (trailing_significand_బిట్లు % 64)) & exponent_ముసుగు;

  initialize(&S);
  ప్రతిజ్ఞా(part_లెక్క() == mysignificand.కొలత_2());

  sign = S.hasSignedRepr
             ? static_cast<గుర్తులేని పూర్ణాంకం>(చివరి_మాట >> ((S.కొలత_లోపల_బిట్లు - 1) % 64))
             : 0;

  తార్కిక all_zero_significand =
      has_significand && llvm::all_యొక్క(mysignificand, equal_గమ్యం(0));

  తార్కిక is_zero = myexponent == 0 && all_zero_significand && S.hasZero;

  ఒకవేళ constexpr (S.nonFiniteBehavior == fltNonfiniteBehavior::IEEE754) {
    తార్కిక is_inf = అబద్ధం;

    ఒకవేళ constexpr (S.has_Explicit_Integer_బిట్టు) {
      // This is only used and tested for x87DoubleExtended
      static_assert(S.precision == 64);
      constexpr integerPart significand_ముసుగు_no_int_బిట్టు =
          (uint64_t{1} << (trailing_significand_బిట్లు - 1)) - 1;
      స్థిరరాశి integerPart myintegerbit =
          mysignificand[0] >> (trailing_significand_బిట్లు - 1);

      is_inf = myexponent - bias == ::exponentInf(S) && myintegerbit == 1 &&
               (mysignificand[0] & significand_ముసుగు_no_int_బిట్టు) == 0;
    } లేకపోతే {
      is_inf = myexponent - bias == ::exponentInf(S) && all_zero_significand;
    }

    ఒకవేళ (is_inf) {
      makeInf(sign);
      తిరిగి;
    }
  }

  తార్కిక is_nan = అబద్ధం;

  ఒకవేళ constexpr (S.nanEncoding == fltNanEncoding::IEEE) {
    ఒకవేళ constexpr (S.has_Explicit_Integer_బిట్టు) {
      // This is only used and tested for x87DoubleExtended
      static_assert(S.precision == 64);
      స్థిరరాశి integerPart myintegerbit =
          mysignificand[0] >> (trailing_significand_బిట్లు - 1);
      constexpr integerPart significand_ముసుగు_no_int_బిట్టు =
          (uint64_t{1} << (trailing_significand_బిట్లు - 1)) - 1;

      ఒకవేళ (myexponent - bias == ::exponentNaN(S) &&
          (mysignificand[0] & significand_ముసుగు_no_int_బిట్టు) != 0) {
        // regular NaN and pseudoNaN
        is_nan = నిజం;
      } లేకపోతే ఒకవేళ (myexponent - bias == ::exponentNaN(S) &&
                 (mysignificand[0] & significand_ముసుగు_no_int_బిట్టు) == 0) {
        // pseudoinfinity
        is_nan = నిజం;
      } లేకపోతే ఒకవేళ (myexponent - bias != ::exponentNaN(S) && myexponent != 0 &&
                 myintegerbit == 0) {
        // unnormal
        is_nan = నిజం;
      }
    } లేకపోతే {
      is_nan = myexponent - bias == ::exponentNaN(S) && !all_zero_significand;
    }
  } లేకపోతే ఒకవేళ constexpr (S.nanEncoding == fltNanEncoding::AllOnes) {
    తార్కిక all_ones_significand =
        std::all_యొక్క(mysignificand.begin(), mysignificand.చివర() - 1,
                    [](integerPart బిట్లు) { తిరిగి బిట్లు == ~integerPart{0}; }) &&
        (!significand_ముసుగు ||
         mysignificand[mysignificand.కొలత_2() - 1] == significand_ముసుగు);
    is_nan = myexponent - bias == ::exponentNaN(S) && all_ones_significand;
  } లేకపోతే ఒకవేళ constexpr (S.nanEncoding == fltNanEncoding::NegativeZero) {
    is_nan = is_zero && sign;
  }

  ఒకవేళ (is_nan) {
    category = fcNaN;
    exponent = ::exponentNaN(S);
    std::copy_n(mysignificand.begin(), mysignificand.కొలత_2(),
                significandParts());
    తిరిగి;
  }

  ఒకవేళ (is_zero) {
    makeZero(sign);
    తిరిగి;
  }

  category = fc_సాధారణ;
  exponent = myexponent - bias;
  std::copy_n(mysignificand.begin(), mysignificand.కొలత_2(), significandParts());
  ఒకవేళ (myexponent == 0 && S.hasDenormals) // denormal
    exponent = S.minExponent;
  లేకపోతే {
    ఒకవేళ constexpr (!S.has_Explicit_Integer_బిట్టు) {
      significandParts()[mysignificand.కొలత_2() - 1] |= integer_బిట్టు;
    }
  }
}

శూన్య IEEEFloat::initFromQuadrupleAPInt(స్థిరరాశి APInt &api) {
  initFromIEEEAPInt<APFloat_పునాది::semIEEEquad>(api);
}

శూన్య IEEEFloat::initFromDoubleAPInt(స్థిరరాశి APInt &api) {
  initFromIEEEAPInt<APFloat_పునాది::semIEEEdouble>(api);
}

శూన్య IEEEFloat::initFromFloatAPInt(స్థిరరాశి APInt &api) {
  initFromIEEEAPInt<APFloat_పునాది::semIEEEsingle>(api);
}

శూన్య IEEEFloat::initFromBFloatAPInt(స్థిరరాశి APInt &api) {
  initFromIEEEAPInt<APFloat_పునాది::semBFloat>(api);
}

శూన్య IEEEFloat::initFromHalfAPInt(స్థిరరాశి APInt &api) {
  initFromIEEEAPInt<APFloat_పునాది::semIEEEhalf>(api);
}

శూన్య IEEEFloat::initFromFloat8E5M2APInt(స్థిరరాశి APInt &api) {
  initFromIEEEAPInt<APFloat_పునాది::semFloat8E5M2>(api);
}

శూన్య IEEEFloat::initFromFloat8E5M2FNUZAPInt(స్థిరరాశి APInt &api) {
  initFromIEEEAPInt<APFloat_పునాది::semFloat8E5M2FNUZ>(api);
}

శూన్య IEEEFloat::initFromFloat8E4M3APInt(స్థిరరాశి APInt &api) {
  initFromIEEEAPInt<APFloat_పునాది::semFloat8E4M3>(api);
}

శూన్య IEEEFloat::initFromFloat8E4M3FNAPInt(స్థిరరాశి APInt &api) {
  initFromIEEEAPInt<APFloat_పునాది::semFloat8E4M3FN>(api);
}

శూన్య IEEEFloat::initFromFloat8E4M3FNUZAPInt(స్థిరరాశి APInt &api) {
  initFromIEEEAPInt<APFloat_పునాది::semFloat8E4M3FNUZ>(api);
}

శూన్య IEEEFloat::initFromFloat8E4M3B11FNUZAPInt(స్థిరరాశి APInt &api) {
  initFromIEEEAPInt<APFloat_పునాది::semFloat8E4M3B11FNUZ>(api);
}

శూన్య IEEEFloat::initFromFloat8E3M4APInt(స్థిరరాశి APInt &api) {
  initFromIEEEAPInt<APFloat_పునాది::semFloat8E3M4>(api);
}

శూన్య IEEEFloat::initFromFloatTF32APInt(స్థిరరాశి APInt &api) {
  initFromIEEEAPInt<APFloat_పునాది::semFloatTF32>(api);
}

శూన్య IEEEFloat::initFromFloat6E3M2FNAPInt(స్థిరరాశి APInt &api) {
  initFromIEEEAPInt<APFloat_పునాది::semFloat6E3M2FN>(api);
}

శూన్య IEEEFloat::initFromFloat6E2M3FNAPInt(స్థిరరాశి APInt &api) {
  initFromIEEEAPInt<APFloat_పునాది::semFloat6E2M3FN>(api);
}

శూన్య IEEEFloat::initFromFloat4E2M1FNAPInt(స్థిరరాశి APInt &api) {
  initFromIEEEAPInt<APFloat_పునాది::semFloat4E2M1FN>(api);
}

/// Treat api as containing the bits of a floating point number.
శూన్య IEEEFloat::initFromAPInt(స్థిరరాశి fltSemantics *Sem, స్థిరరాశి APInt &api) {
  ప్రతిజ్ఞా(api.get_బిట్టు_వెడల్పు() == Sem->కొలత_లోపల_బిట్లు);
  ఒకవేళ (Sem == &APFloat_పునాది::semIEEEhalf)
    తిరిగి initFromHalfAPInt(api);
  ఒకవేళ (Sem == &APFloat_పునాది::semBFloat)
    తిరిగి initFromBFloatAPInt(api);
  ఒకవేళ (Sem == &APFloat_పునాది::semIEEEsingle)
    తిరిగి initFromFloatAPInt(api);
  ఒకవేళ (Sem == &APFloat_పునాది::semIEEEdouble)
    తిరిగి initFromDoubleAPInt(api);
  ఒకవేళ (Sem == &APFloat_పునాది::semIEEEquad)
    తిరిగి initFromQuadrupleAPInt(api);
  ఒకవేళ (Sem == &APFloat_పునాది::semX87DoubleExtended)
    తిరిగి initFromF80LongDoubleAPInt(api);
  ఒకవేళ (Sem == &APFloat_పునాది::semPPCDoubleDoubleLegacy)
    తిరిగి initFromPPCDoubleDoubleLegacyAPInt(api);
  ఒకవేళ (Sem == &APFloat_పునాది::semFloat8E5M2)
    తిరిగి initFromFloat8E5M2APInt(api);
  ఒకవేళ (Sem == &APFloat_పునాది::semFloat8E5M2FNUZ)
    తిరిగి initFromFloat8E5M2FNUZAPInt(api);
  ఒకవేళ (Sem == &APFloat_పునాది::semFloat8E4M3)
    తిరిగి initFromFloat8E4M3APInt(api);
  ఒకవేళ (Sem == &APFloat_పునాది::semFloat8E4M3FN)
    తిరిగి initFromFloat8E4M3FNAPInt(api);
  ఒకవేళ (Sem == &APFloat_పునాది::semFloat8E4M3FNUZ)
    తిరిగి initFromFloat8E4M3FNUZAPInt(api);
  ఒకవేళ (Sem == &APFloat_పునాది::semFloat8E4M3B11FNUZ)
    తిరిగి initFromFloat8E4M3B11FNUZAPInt(api);
  ఒకవేళ (Sem == &APFloat_పునాది::semFloat8E3M4)
    తిరిగి initFromFloat8E3M4APInt(api);
  ఒకవేళ (Sem == &APFloat_పునాది::semFloatTF32)
    తిరిగి initFromFloatTF32APInt(api);
  ఒకవేళ (Sem == &APFloat_పునాది::semFloat8E8M0FNU)
    తిరిగి initFromFloat8E8M0FNUAPInt(api);
  ఒకవేళ (Sem == &APFloat_పునాది::semFloat8E5M3FNU)
    తిరిగి initFromFloat8E5M3FNUAPInt(api);
  ఒకవేళ (Sem == &APFloat_పునాది::semFloat6E3M2FN)
    తిరిగి initFromFloat6E3M2FNAPInt(api);
  ఒకవేళ (Sem == &APFloat_పునాది::semFloat6E2M3FN)
    తిరిగి initFromFloat6E2M3FNAPInt(api);
  ఒకవేళ (Sem == &APFloat_పునాది::semFloat4E2M1FN)
    తిరిగి initFromFloat4E2M1FNAPInt(api);

  llvm_unreachable("unsupported semantics");
}

/// Make this number the largest magnitude normal number in the given
/// semantics.
శూన్య IEEEFloat::make_అతిపెద్ద(తార్కిక Negative) {
  ఒకవేళ (Negative && !semantics->hasSignedRepr)
    llvm_unreachable(
        "This floating point format does not support signed values");
  // We want (in interchange format):
  //   sign = {Negative}
  //   exponent = 1..10
  //   significand = 1..1
  category = fc_సాధారణ;
  sign = Negative;
  exponent = semantics->maxExponent;

  // Use memset to set all but the highest integerPart to all ones.
  integerPart *significand = significandParts();
  గుర్తులేని Part_లెక్క = part_లెక్క();
  memset(significand, 0xFF, పరిమాణం(integerPart)*(Part_లెక్క - 1));

  // Set the high integerPart especially setting all unused top bits for
  // internal consistency.
  స్థిరరాశి గుర్తులేని అంకె_వాడని_ఎగువ_బిట్లు =
    Part_లెక్క*integer_Part_వెడల్పు - semantics->precision;
  significand[Part_లెక్క - 1] = (అంకె_వాడని_ఎగువ_బిట్లు < integer_Part_వెడల్పు)
                                   ? (~integerPart(0) >> అంకె_వాడని_ఎగువ_బిట్లు)
                                   : 0;
  ఒకవేళ (semantics->nonFiniteBehavior == fltNonfiniteBehavior::NanOnly &&
      semantics->nanEncoding == fltNanEncoding::AllOnes &&
      (semantics->precision > 1))
    significand[0] &= ~integerPart(1);
}

/// Make this number the smallest magnitude denormal number in the given
/// semantics.
శూన్య IEEEFloat::make_అతిచిన్న(తార్కిక Negative) {
  ఒకవేళ (Negative && !semantics->hasSignedRepr)
    llvm_unreachable(
        "This floating point format does not support signed values");
  // We want (in interchange format):
  //   sign = {Negative}
  //   exponent = 0..0
  //   significand = 0..01
  category = fc_సాధారణ;
  sign = Negative;
  exponent = semantics->minExponent;
  APInt::tc_సమితి(significandParts(), 1, part_లెక్క());
}

శూన్య IEEEFloat::make_అతిచిన్న_Normalized(తార్కిక Negative) {
  ఒకవేళ (Negative && !semantics->hasSignedRepr)
    llvm_unreachable(
        "This floating point format does not support signed values");
  // We want (in interchange format):
  //   sign = {Negative}
  //   exponent = 0..0
  //   significand = 10..0

  category = fc_సాధారణ;
  zeroSignificand();
  sign = Negative;
  exponent = semantics->minExponent;
  APInt::tc_సమితి_బిట్టు(significandParts(), semantics->precision - 1);
}

IEEEFloat::IEEEFloat(స్థిరరాశి fltSemantics &Sem, స్థిరరాశి APInt &API) {
  initFromAPInt(&Sem, API);
}

IEEEFloat::IEEEFloat(దశాంశం f) {
  initFromAPInt(&APFloat_పునాది::semIEEEsingle, APInt::float_గమ్యం_బిట్లు(f));
}

IEEEFloat::IEEEFloat(ద్విగుణ d) {
  initFromAPInt(&APFloat_పునాది::semIEEEdouble, APInt::double_గమ్యం_బిట్లు(d));
}

నామాకాశ {
  శూన్య append(Small_సదిశ_Impl<అక్షరం> &బఫరు, StringRef Str) {
    బఫరు.append(Str.begin(), Str.చివర());
  }

  /// Removes data from the given significand until it is no more
  /// precise than is required for the desired precision.
  శూన్య Adjust_గమ్యం_Precision(APInt &significand,
                         పూర్ణాంకం &ఘాతాంకం, గుర్తులేని FormatPrecision) {
    గుర్తులేని బిట్లు = significand.get_చురుకైన_బిట్లు();

    // 196/59 is a very slight overestimate of lg_2(10).
    గుర్తులేని బిట్లు_Required = (FormatPrecision * 196 + 58) / 59;

    ఒకవేళ (బిట్లు <= బిట్లు_Required) తిరిగి;

    గుర్తులేని tensRemovable = (బిట్లు - బిట్లు_Required) * 59 / 196;
    ఒకవేళ (!tensRemovable) తిరిగి;

    ఘాతాంకం += tensRemovable;

    APInt భాజకం(significand.get_బిట్టు_వెడల్పు(), 1);
    APInt powten(significand.get_బిట్టు_వెడల్పు(), 10);
    అయితే (నిజం) {
      ఒకవేళ (tensRemovable & 1)
        భాజకం *= powten;
      tensRemovable >>= 1;
      ఒకవేళ (!tensRemovable) ఆపు;
      powten *= powten;
    }

    significand = significand.udiv(భాజకం);

    // Truncate the significand down to its active bit count.
    significand = significand.trunc(significand.get_చురుకైన_బిట్లు());
  }


  శూన్య Adjust_గమ్యం_Precision(Small_సదిశ_Impl<అక్షరం> &బఫరు_2,
                         పూర్ణాంకం &ఘాతాంకం, గుర్తులేని FormatPrecision) {
    గుర్తులేని N = బఫరు_2.కొలత_2();
    ఒకవేళ (N <= FormatPrecision) తిరిగి;

    // The most significant figures are the last ones in the buffer.
    గుర్తులేని మొదటి_Significant = N - FormatPrecision;

    // Round.
    // FIXME: this probably shouldn't use 'round half up'.

    // Rounding down is just a truncation, except we also want to drop
    // trailing zeros from the new result.
    ఒకవేళ (బఫరు_2[మొదటి_Significant - 1] < '5') {
      అయితే (మొదటి_Significant < N && బఫరు_2[మొదటి_Significant] == '0')
        మొదటి_Significant++;

      ఘాతాంకం += మొదటి_Significant;
      బఫరు_2.erase(&బఫరు_2[0], &బఫరు_2[మొదటి_Significant]);
      తిరిగి;
    }

    // Rounding up requires a decimal add-with-carry.  If we continue
    // the carry, the newly-introduced zeros will just be truncated.
    కొరకు (గుర్తులేని I = మొదటి_Significant; I != N; ++I) {
      ఒకవేళ (బఫరు_2[I] == '9') {
        మొదటి_Significant++;
      } లేకపోతే {
        బఫరు_2[I]++;
        ఆపు;
      }
    }

    // If we carried through, we have exactly one digit of precision.
    ఒకవేళ (మొదటి_Significant == N) {
      ఘాతాంకం += మొదటి_Significant;
      బఫరు_2.clear();
      బఫరు_2.నెట్టు_వెనుక('1');
      తిరిగి;
    }

    ఘాతాంకం += మొదటి_Significant;
    బఫరు_2.erase(&బఫరు_2[0], &బఫరు_2[మొదటి_Significant]);
  }

  శూన్య గమ్యం_String_Impl(Small_సదిశ_Impl<అక్షరం> &Str, స్థిరరాశి తార్కిక isNeg, పూర్ణాంకం ఘాతాంకం,
                    APInt significand, గుర్తులేని FormatPrecision,
                    గుర్తులేని FormatMaxPadding, తార్కిక TruncateZero) {
    స్థిరరాశి పూర్ణాంకం semanticsPrecision = significand.get_బిట్టు_వెడల్పు();

    ఒకవేళ (isNeg)
      Str.నెట్టు_వెనుక('-');

    // Set FormatPrecision if zero.  We want to do this before we
    // truncate trailing zeros, as those are part of the precision.
    ఒకవేళ (!FormatPrecision) {
      // We use enough digits so the number can be round-tripped back to an
      // APFloat. The formula comes from "How to Print Floating-Point Numbers
      // Accurately" by Steele and White.
      // FIXME: Using a formula based purely on the precision is conservative;
      // we can print fewer digits depending on the actual value being printed.

      // FormatPrecision = 2 + floor(significandBits / lg_2(10))
      FormatPrecision = 2 + semanticsPrecision * 59 / 196;
    }

    // Ignore trailing binary zeros.
    పూర్ణాంకం trailingZeros = significand.countr_zero();
    ఘాతాంకం += trailingZeros;
    significand.lshr_లోపల_ఉంచు(trailingZeros);

    // Change the exponent from 2^e to 10^e.
    ఒకవేళ (ఘాతాంకం == 0) {
      // Nothing to do.
    } లేకపోతే ఒకవేళ (ఘాతాంకం > 0) {
      // Just shift left.
      significand = significand.zext(semanticsPrecision + ఘాతాంకం);
      significand <<= ఘాతాంకం;
      ఘాతాంకం = 0;
    } లేకపోతే { /* exp < 0 */
      పూర్ణాంకం texp = -ఘాతాంకం;

      // We transform this using the identity:
      //   (N)(2^-e) == (N)(5^e)(10^-e)
      // This means we have to multiply N (the significand) by 5^e.
      // To avoid overflow, we have to operate on numbers large
      // enough to store N * 5^e:
      //   log2(N * 5^e) == log2(N) + e * log2(5)
      //                 <= semantics->precision + e * 137 / 59
      //   (log_2(5) ~ 2.321928 < 2.322034 ~ 137/59)

      గుర్తులేని precision = semanticsPrecision + (137 * texp + 136) / 59;

      // Multiply significand by 5^e.
      //   N * 5^0101 == N * 5^(1*1) * 5^(0*2) * 5^(1*4) * 5^(0*8)
      significand = significand.zext(precision);
      APInt five_గమ్యం_the_i(precision, 5);
      అయితే (నిజం) {
        ఒకవేళ (texp & 1)
          significand *= five_గమ్యం_the_i;

        texp >>= 1;
        ఒకవేళ (!texp)
          ఆపు;
        five_గమ్యం_the_i *= five_గమ్యం_the_i;
      }
    }

    Adjust_గమ్యం_Precision(significand, ఘాతాంకం, FormatPrecision);

    Small_సదిశ<అక్షరం, 256> బఫరు_2;

    // Fill the buffer.
    గుర్తులేని precision = significand.get_బిట్టు_వెడల్పు();
    ఒకవేళ (precision < 4) {
      // We need enough precision to store the value 10.
      precision = 4;
      significand = significand.zext(precision);
    }
    APInt ten(precision, 10);
    APInt digit(precision, 0);

    తార్కిక లోపల_Trail = నిజం;
    అయితే (significand != 0) {
      // digit <- significand % 10
      // significand <- significand / 10
      APInt::udivrem(significand, ten, significand, digit);

      గుర్తులేని d = digit.get_ZExt_విలువ();

      // Drop trailing zeros.
      ఒకవేళ (లోపల_Trail && !d)
        ఘాతాంకం++;
      లేకపోతే {
        బఫరు_2.నెట్టు_వెనుక((అక్షరం) ('0' + d));
        లోపల_Trail = అబద్ధం;
      }
    }

    ప్రతిజ్ఞా(!బఫరు_2.ఖాళీలు() && "no characters in buffer!");

    // Drop down to FormatPrecision.
    // TODO: don't do more precise calculations above than are required.
    Adjust_గమ్యం_Precision(బఫరు_2, ఘాతాంకం, FormatPrecision);

    గుర్తులేని NDigits = బఫరు_2.కొలత_2();

    // Check whether we should use scientific notation.
    తార్కిక FormatScientific;
    ఒకవేళ (!FormatMaxPadding) {
      FormatScientific = నిజం;
    } లేకపోతే {
      ఒకవేళ (ఘాతాంకం >= 0) {
        // 765e3 --> 765000
        //              ^^^
        // But we shouldn't make the number look more precise than it is.
        FormatScientific = ((గుర్తులేని) ఘాతాంకం > FormatMaxPadding ||
                            NDigits + (గుర్తులేని) ఘాతాంకం > FormatPrecision);
      } లేకపోతే {
        // Power of the most significant digit.
        పూర్ణాంకం MSD = ఘాతాంకం + (పూర్ణాంకం) (NDigits - 1);
        ఒకవేళ (MSD >= 0) {
          // 765e-2 == 7.65
          FormatScientific = అబద్ధం;
        } లేకపోతే {
          // 765e-5 == 0.00765
          //           ^ ^^
          FormatScientific = ((గుర్తులేని) -MSD) > FormatMaxPadding;
        }
      }
    }

    // Scientific formatting is pretty straightforward.
    ఒకవేళ (FormatScientific) {
      ఘాతాంకం += (NDigits - 1);

      Str.నెట్టు_వెనుక(బఫరు_2[NDigits-1]);
      Str.నెట్టు_వెనుక('.');
      ఒకవేళ (NDigits == 1 && TruncateZero)
        Str.నెట్టు_వెనుక('0');
      లేకపోతే
        కొరకు (గుర్తులేని I = 1; I != NDigits; ++I)
          Str.నెట్టు_వెనుక(బఫరు_2[NDigits-1-I]);
      // Fill with zeros up to FormatPrecision.
      ఒకవేళ (!TruncateZero && FormatPrecision > NDigits - 1)
        Str.append(FormatPrecision - NDigits + 1, '0');
      // For !TruncateZero we use lower 'e'.
      Str.నెట్టు_వెనుక(TruncateZero ? 'E' : 'e');

      Str.నెట్టు_వెనుక(ఘాతాంకం >= 0 ? '+' : '-');
      ఒకవేళ (ఘాతాంకం < 0)
        ఘాతాంకం = -ఘాతాంకం;
      Small_సదిశ<అక్షరం, 6> expbuf;
      చేయి {
        expbuf.నెట్టు_వెనుక((అక్షరం) ('0' + (ఘాతాంకం % 10)));
        ఘాతాంకం /= 10;
      } అయితే (ఘాతాంకం);
      // Exponent always at least two digits if we do not truncate zeros.
      ఒకవేళ (!TruncateZero && expbuf.కొలత_2() < 2)
        expbuf.నెట్టు_వెనుక('0');
      కొరకు (గుర్తులేని I = 0, E = expbuf.కొలత_2(); I != E; ++I)
        Str.నెట్టు_వెనుక(expbuf[E-1-I]);
      తిరిగి;
    }

    // Non-scientific, positive exponents.
    ఒకవేళ (ఘాతాంకం >= 0) {
      కొరకు (గుర్తులేని I = 0; I != NDigits; ++I)
        Str.నెట్టు_వెనుక(బఫరు_2[NDigits-1-I]);
      కొరకు (గుర్తులేని I = 0; I != (గుర్తులేని) ఘాతాంకం; ++I)
        Str.నెట్టు_వెనుక('0');
      తిరిగి;
    }

    // Non-scientific, negative exponents.

    // The number of digits to the left of the decimal point.
    పూర్ణాంకం NWholeDigits = ఘాతాంకం + (పూర్ణాంకం) NDigits;

    గుర్తులేని I = 0;
    ఒకవేళ (NWholeDigits > 0) {
      కొరకు (; I != (గుర్తులేని) NWholeDigits; ++I)
        Str.నెట్టు_వెనుక(బఫరు_2[NDigits-I-1]);
      Str.నెట్టు_వెనుక('.');
    } లేకపోతే {
      గుర్తులేని NZeros = 1 + (గుర్తులేని) -NWholeDigits;

      Str.నెట్టు_వెనుక('0');
      Str.నెట్టు_వెనుక('.');
      కొరకు (గుర్తులేని Z = 1; Z != NZeros; ++Z)
        Str.నెట్టు_వెనుక('0');
    }

    కొరకు (; I != NDigits; ++I)
      Str.నెట్టు_వెనుక(బఫరు_2[NDigits-I-1]);

  }
} // namespace

శూన్య IEEEFloat::గమ్యం_String(Small_సదిశ_Impl<అక్షరం> &Str, గుర్తులేని FormatPrecision,
                         గుర్తులేని FormatMaxPadding, తార్కిక TruncateZero) స్థిరరాశి {
  ఎంపిక (category) {
  స్థితి fcInfinity:
    ఒకవేళ (isNegative())
      తిరిగి append(Str, "-Inf");
    లేకపోతే
      తిరిగి append(Str, "+Inf");

  స్థితి fcNaN: తిరిగి append(Str, "NaN");

  స్థితి fcZero:
    ఒకవేళ (isNegative())
      Str.నెట్టు_వెనుక('-');

    ఒకవేళ (!FormatMaxPadding) {
      ఒకవేళ (TruncateZero)
        append(Str, "0.0E+0");
      లేకపోతే {
        append(Str, "0.0");
        ఒకవేళ (FormatPrecision > 1)
          Str.append(FormatPrecision - 1, '0');
        append(Str, "e+00");
      }
    } లేకపోతే {
      Str.నెట్టు_వెనుక('0');
    }
    తిరిగి;

  స్థితి fc_సాధారణ:
    ఆపు;
  }

  // Decompose the number into an APInt and an exponent.
  పూర్ణాంకం ఘాతాంకం = exponent - ((పూర్ణాంకం) semantics->precision - 1);
  APInt significand(
      semantics->precision,
      ArrayRef(significandParts(), part_లెక్క_For_బిట్లు(semantics->precision)));

  గమ్యం_String_Impl(Str, isNegative(), ఘాతాంకం, significand, FormatPrecision,
               FormatMaxPadding, TruncateZero);

}

పూర్ణాంకం IEEEFloat::getExactLog2Abs() స్థిరరాశి {
  ఒకవేళ (!isFinite() || isZero())
    తిరిగి INT_MIN;

  స్థిరరాశి integerPart *Parts = significandParts();
  స్థిరరాశి పూర్ణాంకం Part_లెక్క = part_లెక్క_For_బిట్లు(semantics->precision);

  పూర్ణాంకం తీయి_లెక్క = 0;
  కొరకు (పూర్ణాంకం i = 0; i < Part_లెక్క; ++i) {
    తీయి_లెక్క += llvm::popcount(Parts[i]);
    ఒకవేళ (తీయి_లెక్క > 1)
      తిరిగి INT_MIN;
  }

  ఒకవేళ (exponent != semantics->minExponent)
    తిరిగి exponent;

  పూర్ణాంకం CountrParts = 0;
  కొరకు (పూర్ణాంకం i = 0; i < Part_లెక్క;
       ++i, CountrParts += APInt::APINT_బిట్లు_చొప్పున_మాట) {
    ఒకవేళ (Parts[i] != 0) {
      తిరిగి exponent - semantics->precision + CountrParts +
             llvm::countr_zero(Parts[i]) + 1;
    }
  }

  llvm_unreachable("didn't find the set bit");
}

తార్కిక IEEEFloat::isSignaling() స్థిరరాశి {
  ఒకవేళ (!isNaN())
    తిరిగి అబద్ధం;
  ఒకవేళ (semantics->nonFiniteBehavior == fltNonfiniteBehavior::NanOnly ||
      semantics->nonFiniteBehavior == fltNonfiniteBehavior::FiniteOnly)
    తిరిగి అబద్ధం;

  // IEEE-754R 2008 6.2.1: A signaling NaN bit string should be encoded with the
  // first bit of the trailing significand being 0.
  తిరిగి !APInt::tc_వెలికితీయి_బిట్టు(significandParts(), semantics->precision - 2);
}

/// IEEE-754R 2008 5.3.1: nextUp/nextDown.
///
/// *NOTE* since nextDown(x) = -nextUp(-x), we only implement nextUp with
/// appropriate sign switching before/after the computation.
APFloat::op_స్థితి IEEEFloat::తదుపరి(తార్కిక తదుపరి_కిందికి) {
  // If we are performing nextDown, swap sign so we have -x.
  ఒకవేళ (తదుపరి_కిందికి)
    changeSign();

  // Compute nextUp(x)
  op_స్థితి ఫలితం_2 = opOK;

  // Handle each float category separately.
  ఎంపిక (category) {
  స్థితి fcInfinity:
    // nextUp(+inf) = +inf
    ఒకవేళ (!isNegative())
      ఆపు;
    // nextUp(-inf) = -getLargest()
    make_అతిపెద్ద(నిజం);
    ఆపు;
  స్థితి fcNaN:
    // IEEE-754R 2008 6.2 Par 2: nextUp(sNaN) = qNaN. Set Invalid flag.
    // IEEE-754R 2008 6.2: nextUp(qNaN) = qNaN. Must be identity so we do not
    //                     change the payload.
    ఒకవేళ (isSignaling()) {
      ఫలితం_2 = opInvalidOp;
      // For consistency, propagate the sign of the sNaN to the qNaN.
      makeNaN(అబద్ధం, isNegative(), శూన్యసూచిక);
    }
    ఆపు;
  స్థితి fcZero:
    // nextUp(pm 0) = +getSmallest()
    make_అతిచిన్న(అబద్ధం);
    ఆపు;
  స్థితి fc_సాధారణ:
    // nextUp(-getSmallest()) = -0
    ఒకవేళ (is_అతిచిన్న() && isNegative()) {
      APInt::tc_సమితి(significandParts(), 0, part_లెక్క());
      category = fcZero;
      exponent = 0;
      ఒకవేళ (semantics->nanEncoding == fltNanEncoding::NegativeZero)
        sign = అబద్ధం;
      ఒకవేళ (!semantics->hasZero)
        make_అతిచిన్న_Normalized(అబద్ధం);
      ఆపు;
    }

    ఒకవేళ (is_అతిపెద్ద() && !isNegative()) {
      ఒకవేళ (semantics->nonFiniteBehavior == fltNonfiniteBehavior::NanOnly) {
        // nextUp(getLargest()) == NAN
        makeNaN();
        ఆపు;
      } లేకపోతే ఒకవేళ (semantics->nonFiniteBehavior ==
                 fltNonfiniteBehavior::FiniteOnly) {
        // nextUp(getLargest()) == getLargest()
        ఆపు;
      } లేకపోతే {
        // nextUp(getLargest()) == INFINITY
        APInt::tc_సమితి(significandParts(), 0, part_లెక్క());
        category = fcInfinity;
        exponent = semantics->maxExponent + 1;
        ఆపు;
      }
    }

    // nextUp(normal) == normal + inc.
    ఒకవేళ (isNegative()) {
      // If we are negative, we need to decrement the significand.

      // We only cross a binade boundary that requires adjusting the exponent
      // if:
      //   1. exponent != semantics->minExponent. This implies we are not in the
      //   smallest binade or are dealing with denormals.
      //   2. Our significand excluding the integral bit is all zeros.
      తార్కిక WillCrossBinadeBoundary =
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
      APInt::tcDecrement(Parts, part_లెక్క());

      ఒకవేళ (WillCrossBinadeBoundary) {
        // Our result is a normal number. Do the following:
        // 1. Set the integral bit to 1.
        // 2. Decrement the exponent.
        APInt::tc_సమితి_బిట్టు(Parts, semantics->precision - 1);
        exponent--;
      }
    } లేకపోతే {
      // If we are positive, we need to increment the significand.

      // We only cross a binade boundary that requires adjusting the exponent if
      // the input is not a denormal and all of said input's significand bits
      // are set. If all of said conditions are true: clear the significand, set
      // the integral bit to 1, and increment the exponent. If we have a
      // denormal always increment since moving denormals and the numbers in the
      // smallest normal binade have the same exponent in our representation.
      // If there are only exponents, any increment always crosses the
      // BinadeBoundary.
      తార్కిక WillCrossBinadeBoundary = !APFloat::hasSignificand(*semantics) ||
                                     (!isDenormal() && isSignificandAllOnes());

      ఒకవేళ (WillCrossBinadeBoundary) {
        integerPart *Parts = significandParts();
        APInt::tc_సమితి(Parts, 0, part_లెక్క());
        APInt::tc_సమితి_బిట్టు(Parts, semantics->precision - 1);
        ప్రతిజ్ఞా(exponent != semantics->maxExponent &&
               "We can not increment an exponent beyond the maxExponent allowed"
               " by the given floating point semantics.");
        exponent++;
      } లేకపోతే {
        incrementSignificand();
      }
    }
    ఆపు;
  }

  // If we are performing nextDown, swap sign so we have -nextUp(-x)
  ఒకవేళ (తదుపరి_కిందికి)
    changeSign();

  తిరిగి ఫలితం_2;
}

APInt IEEEFloat::getNaNPayload() స్థిరరాశి {
  ప్రతిజ్ఞా(isNaN() && "Can only be called on NaN values");
  // Number of bits in the payload, excluding the (maybe implied) integer bit.
  గుర్తులేని బిట్లు_2 = semantics->precision - 1;
  తిరిగి APInt(బిట్లు_2, ArrayRef(significandParts(), part_లెక్క_For_బిట్లు(బిట్లు_2)));
}

APFloat_పునాది::ExponentType IEEEFloat::exponentNaN() స్థిరరాశి {
  తిరిగి ::exponentNaN(*semantics);
}

APFloat_పునాది::ExponentType IEEEFloat::exponentInf() స్థిరరాశి {
  తిరిగి ::exponentInf(*semantics);
}

APFloat_పునాది::ExponentType IEEEFloat::exponentZero() స్థిరరాశి {
  తిరిగి ::exponentZero(*semantics);
}

శూన్య IEEEFloat::makeInf(తార్కిక Negative) {
  ఒకవేళ (semantics->nonFiniteBehavior == fltNonfiniteBehavior::FiniteOnly)
    llvm_unreachable("This floating point format does not support Inf");

  ఒకవేళ (semantics->nonFiniteBehavior == fltNonfiniteBehavior::NanOnly) {
    // There is no Inf, so make NaN instead.
    makeNaN(అబద్ధం, Negative);
    తిరిగి;
  }
  category = fcInfinity;
  sign = Negative;
  exponent = exponentInf();
  APInt::tc_సమితి(significandParts(), 0, part_లెక్క());
}

శూన్య IEEEFloat::makeZero(తార్కిక Negative) {
  ఒకవేళ (!semantics->hasZero)
    llvm_unreachable("This floating point format does not support Zero");

  category = fcZero;
  sign = Negative;
  ఒకవేళ (semantics->nanEncoding == fltNanEncoding::NegativeZero) {
    // Merge negative zero to positive because 0b10000...000 is used for NaN
    sign = అబద్ధం;
  }
  exponent = exponentZero();
  APInt::tc_సమితి(significandParts(), 0, part_లెక్క());
}

శూన్య IEEEFloat::makeQuiet() {
  ప్రతిజ్ఞా(isNaN());
  ఒకవేళ (semantics->nonFiniteBehavior != fltNonfiniteBehavior::NanOnly)
    APInt::tc_సమితి_బిట్టు(significandParts(), semantics->precision - 2);
}

పూర్ణాంకం ilogb(స్థిరరాశి IEEEFloat &Arg) {
  ఒకవేళ (Arg.isNaN())
    తిరిగి APFloat::IEK_NaN;
  ఒకవేళ (Arg.isZero())
    తిరిగి APFloat::IEK_Zero;
  ఒకవేళ (Arg.isInfinity())
    తిరిగి APFloat::IEK_Inf;
  ఒకవేళ (!Arg.isDenormal())
    తిరిగి Arg.exponent;

  IEEEFloat Normalized(Arg);
  పూర్ణాంకం Significand_బిట్లు = Arg.getSemantics().precision - 1;

  Normalized.exponent += Significand_బిట్లు;
  Normalized.normalize(APFloat::rm_సమీప_Ties_గమ్యం_Even, lfExactlyZero);
  తిరిగి Normalized.exponent - Significand_బిట్లు;
}

IEEEFloat scalbn(IEEEFloat X, పూర్ణాంకం Exp, rounding_బహుళకం Rounding_బహుళకం) {
  స్వయంచాలక MaxExp = X.getSemantics().maxExponent;
  స్వయంచాలక MinExp = X.getSemantics().minExponent;

  // If Exp is wildly out-of-scale, simply adding it to X.exponent will
  // overflow; clamp it to a safe range before adding, but ensure that the range
  // is large enough that the clamp does not change the result. The range we
  // need to support is the difference between the largest possible exponent and
  // the normalized exponent of half the smallest denormal.

  పూర్ణాంకం Significand_బిట్లు = X.getSemantics().precision - 1;
  పూర్ణాంకం MaxIncrement = MaxExp - (MinExp - Significand_బిట్లు) + 1;

  // Clamp to one past the range ends to let normalize handle overlflow.
  X.exponent += std::clamp(Exp, -MaxIncrement - 1, MaxIncrement);
  X.normalize(Rounding_బహుళకం, lfExactlyZero);
  ఒకవేళ (X.isNaN())
    X.makeQuiet();
  తిరిగి X;
}

IEEEFloat frexp(స్థిరరాశి IEEEFloat &Val, పూర్ణాంకం &Exp, rounding_బహుళకం RM) {
  Exp = ilogb(Val);

  // Quiet signalling nans.
  ఒకవేళ (Exp == APFloat::IEK_NaN) {
    IEEEFloat Quiet(Val);
    Quiet.makeQuiet();
    తిరిగి Quiet;
  }

  ఒకవేళ (Exp == APFloat::IEK_Inf)
    తిరిగి Val;

  // 1 is added because frexp is defined to return a normalized fraction in
  // +/-[0.5, 1.0), rather than the usual +/-[1.0, 2.0).
  Exp = Exp == APFloat::IEK_Zero ? 0 : Exp + 1;
  తిరిగి scalbn(Val, -Exp, RM);
}

DoubleAPFloat::DoubleAPFloat(స్థిరరాశి fltSemantics &S)
    : Semantics(&S),
      Floats(కొత్త APFloat[2]{APFloat(APFloat_పునాది::semIEEEdouble),
                            APFloat(APFloat_పునాది::semIEEEdouble)}) {
  ప్రతిజ్ఞా(Semantics == &APFloat_పునాది::semPPCDoubleDouble);
}

DoubleAPFloat::DoubleAPFloat(స్థిరరాశి fltSemantics &S, uninitialized_ట్యాగ్)
    : Semantics(&S), Floats(కొత్త APFloat[2]{
                         APFloat(APFloat_పునాది::semIEEEdouble, uninitialized),
                         APFloat(APFloat_పునాది::semIEEEdouble, uninitialized)}) {
  ప్రతిజ్ఞా(Semantics == &APFloat_పునాది::semPPCDoubleDouble);
}

DoubleAPFloat::DoubleAPFloat(స్థిరరాశి fltSemantics &S, integerPart I)
    : Semantics(&S),
      Floats(కొత్త APFloat[2]{APFloat(APFloat_పునాది::semIEEEdouble, I),
                            APFloat(APFloat_పునాది::semIEEEdouble)}) {
  ప్రతిజ్ఞా(Semantics == &APFloat_పునాది::semPPCDoubleDouble);
}

DoubleAPFloat::DoubleAPFloat(స్థిరరాశి fltSemantics &S, స్థిరరాశి APInt &I)
    : Semantics(&S),
      Floats(కొత్త APFloat[2]{
          APFloat(APFloat_పునాది::semIEEEdouble, APInt(64, I.get_Raw_దత్తాంశం()[0])),
          APFloat(APFloat_పునాది::semIEEEdouble, APInt(64, I.get_Raw_దత్తాంశం()[1]))}) {
  ప్రతిజ్ఞా(Semantics == &APFloat_పునాది::semPPCDoubleDouble);
}

DoubleAPFloat::DoubleAPFloat(స్థిరరాశి fltSemantics &S, APFloat &&మొదటి,
                             APFloat &&రెండవ)
    : Semantics(&S),
      Floats(కొత్త APFloat[2]{std::move(మొదటి), std::move(రెండవ)}) {
  ప్రతిజ్ఞా(Semantics == &APFloat_పునాది::semPPCDoubleDouble);
  ప్రతిజ్ఞా(&Floats[0].getSemantics() == &APFloat_పునాది::semIEEEdouble);
  ప్రతిజ్ఞా(&Floats[1].getSemantics() == &APFloat_పునాది::semIEEEdouble);
}

DoubleAPFloat::DoubleAPFloat(స్థిరరాశి DoubleAPFloat &RHS)
    : Semantics(RHS.Semantics),
      Floats(RHS.Floats ? కొత్త APFloat[2]{APFloat(RHS.Floats[0]),
                                         APFloat(RHS.Floats[1])}
                        : శూన్యసూచిక) {
  ప్రతిజ్ఞా(Semantics == &APFloat_పునాది::semPPCDoubleDouble);
}

DoubleAPFloat::DoubleAPFloat(DoubleAPFloat &&RHS)
    : Semantics(RHS.Semantics), Floats(RHS.Floats) {
  RHS.Semantics = &APFloat_పునాది::semBogus;
  RHS.Floats = శూన్యసూచిక;
  ప్రతిజ్ఞా(Semantics == &APFloat_పునాది::semPPCDoubleDouble);
}

DoubleAPFloat &DoubleAPFloat::క్రియాకారకం=(స్థిరరాశి DoubleAPFloat &RHS) {
  ఒకవేళ (Semantics == RHS.Semantics && RHS.Floats) {
    Floats[0] = RHS.Floats[0];
    Floats[1] = RHS.Floats[1];
  } లేకపోతే ఒకవేళ (ఇది != &RHS) {
    ఇది->~DoubleAPFloat();
    కొత్త (ఇది) DoubleAPFloat(RHS);
  }
  తిరిగి *ఇది;
}

// Returns a result such that:
// 1. abs(Lo) <= ulp(Hi)/2
// 2. Hi == RTNE(Hi + Lo)
// 3. Hi + Lo == X + Y
//
// Requires that log2(X) >= log2(Y).
స్థిర std::pair<APFloat, APFloat> fast_Two_కూడిక(APFloat X, APFloat Y) {
  ఒకవేళ (!X.isFinite())
    తిరిగి {X, APFloat::getZero(X.getSemantics(), /*Negative=*/అబద్ధం)};
  APFloat Hi = X + Y;
  APFloat Delta = Hi - X;
  APFloat Lo = Y - Delta;
  తిరిగి {Hi, Lo};
}

// Implement addition, subtraction, multiplication and division based on:
// "Software for Doubled-Precision Floating-Point Computations",
// by Seppo Linnainmaa, ACM TOMS vol 7 no 3, September 1981, pages 272-283.
APFloat::op_స్థితి DoubleAPFloat::కూడు_Impl(స్థిరరాశి APFloat &a, స్థిరరాశి APFloat &aa,
                                         స్థిరరాశి APFloat &c, స్థిరరాశి APFloat &cc,
                                         rounding_బహుళకం RM) {
  పూర్ణాంకం స్థితి_2 = opOK;
  APFloat z = a;
  స్థితి_2 |= z.కూడు(c, RM);
  ఒకవేళ (!z.isFinite()) {
    ఒకవేళ (!z.isInfinity()) {
      Floats[0] = std::move(z);
      Floats[1].makeZero(/* Neg = */ అబద్ధం);
      తిరిగి (op_స్థితి)స్థితి_2;
    }
    స్థితి_2 = opOK;
    స్వయంచాలక ACompared_గమ్యం_C = a.compare_Absolute_విలువ(c);
    z = cc;
    స్థితి_2 |= z.కూడు(aa, RM);
    ఒకవేళ (ACompared_గమ్యం_C == APFloat::cmpGreaterThan) {
      // z = cc + aa + c + a;
      స్థితి_2 |= z.కూడు(c, RM);
      స్థితి_2 |= z.కూడు(a, RM);
    } లేకపోతే {
      // z = cc + aa + a + c;
      స్థితి_2 |= z.కూడు(a, RM);
      స్థితి_2 |= z.కూడు(c, RM);
    }
    ఒకవేళ (!z.isFinite()) {
      Floats[0] = std::move(z);
      Floats[1].makeZero(/* Neg = */ అబద్ధం);
      తిరిగి (op_స్థితి)స్థితి_2;
    }
    Floats[0] = z;
    APFloat zz = aa;
    స్థితి_2 |= zz.కూడు(cc, RM);
    ఒకవేళ (ACompared_గమ్యం_C == APFloat::cmpGreaterThan) {
      // Floats[1] = a - z + c + zz;
      Floats[1] = a;
      స్థితి_2 |= Floats[1].వ్యవకలనం(z, RM);
      స్థితి_2 |= Floats[1].కూడు(c, RM);
      స్థితి_2 |= Floats[1].కూడు(zz, RM);
    } లేకపోతే {
      // Floats[1] = c - z + a + zz;
      Floats[1] = c;
      స్థితి_2 |= Floats[1].వ్యవకలనం(z, RM);
      స్థితి_2 |= Floats[1].కూడు(a, RM);
      స్థితి_2 |= Floats[1].కూడు(zz, RM);
    }
  } లేకపోతే {
    // q = a - z;
    APFloat q = a;
    స్థితి_2 |= q.వ్యవకలనం(z, RM);

    // zz = q + c + (a - (q + z)) + aa + cc;
    // Compute a - (q + z) as -((q + z) - a) to avoid temporary copies.
    స్వయంచాలక zz = q;
    స్థితి_2 |= zz.కూడు(c, RM);
    స్థితి_2 |= q.కూడు(z, RM);
    స్థితి_2 |= q.వ్యవకలనం(a, RM);
    q.changeSign();
    స్థితి_2 |= zz.కూడు(q, RM);
    స్థితి_2 |= zz.కూడు(aa, RM);
    స్థితి_2 |= zz.కూడు(cc, RM);
    ఒకవేళ (zz.isZero() && !zz.isNegative()) {
      Floats[0] = std::move(z);
      Floats[1].makeZero(/* Neg = */ అబద్ధం);
      తిరిగి opOK;
    }
    Floats[0] = z;
    స్థితి_2 |= Floats[0].కూడు(zz, RM);
    ఒకవేళ (!Floats[0].isFinite()) {
      Floats[1].makeZero(/* Neg = */ అబద్ధం);
      తిరిగి (op_స్థితి)స్థితి_2;
    }
    Floats[1] = std::move(z);
    స్థితి_2 |= Floats[1].వ్యవకలనం(Floats[0], RM);
    స్థితి_2 |= Floats[1].కూడు(zz, RM);
  }
  తిరిగి (op_స్థితి)స్థితి_2;
}

APFloat::op_స్థితి DoubleAPFloat::కూడు_With_Special(స్థిరరాశి DoubleAPFloat &LHS,
                                                స్థిరరాశి DoubleAPFloat &RHS,
                                                DoubleAPFloat &బహిర్గతం,
                                                rounding_బహుళకం RM) {
  ఒకవేళ (LHS.getCategory() == fcNaN) {
    బహిర్గతం = LHS;
    తిరిగి opOK;
  }
  ఒకవేళ (RHS.getCategory() == fcNaN) {
    బహిర్గతం = RHS;
    తిరిగి opOK;
  }
  ఒకవేళ (LHS.getCategory() == fcZero) {
    బహిర్గతం = RHS;
    తిరిగి opOK;
  }
  ఒకవేళ (RHS.getCategory() == fcZero) {
    బహిర్గతం = LHS;
    తిరిగి opOK;
  }
  ఒకవేళ (LHS.getCategory() == fcInfinity && RHS.getCategory() == fcInfinity &&
      LHS.isNegative() != RHS.isNegative()) {
    బహిర్గతం.makeNaN(అబద్ధం, బహిర్గతం.isNegative(), శూన్యసూచిక);
    తిరిగి opInvalidOp;
  }
  ఒకవేళ (LHS.getCategory() == fcInfinity) {
    బహిర్గతం = LHS;
    తిరిగి opOK;
  }
  ఒకవేళ (RHS.getCategory() == fcInfinity) {
    బహిర్గతం = RHS;
    తిరిగి opOK;
  }
  ప్రతిజ్ఞా(LHS.getCategory() == fc_సాధారణ && RHS.getCategory() == fc_సాధారణ);

  APFloat A(LHS.Floats[0]), AA(LHS.Floats[1]), C(RHS.Floats[0]),
      CC(RHS.Floats[1]);
  ప్రతిజ్ఞా(&A.getSemantics() == &APFloat_పునాది::semIEEEdouble);
  ప్రతిజ్ఞా(&AA.getSemantics() == &APFloat_పునాది::semIEEEdouble);
  ప్రతిజ్ఞా(&C.getSemantics() == &APFloat_పునాది::semIEEEdouble);
  ప్రతిజ్ఞా(&CC.getSemantics() == &APFloat_పునాది::semIEEEdouble);
  ప్రతిజ్ఞా(&బహిర్గతం.Floats[0].getSemantics() == &APFloat_పునాది::semIEEEdouble);
  ప్రతిజ్ఞా(&బహిర్గతం.Floats[1].getSemantics() == &APFloat_పునాది::semIEEEdouble);
  తిరిగి బహిర్గతం.కూడు_Impl(A, AA, C, CC, RM);
}

APFloat::op_స్థితి DoubleAPFloat::కూడు(స్థిరరాశి DoubleAPFloat &RHS,
                                     rounding_బహుళకం RM) {
  తిరిగి కూడు_With_Special(*ఇది, RHS, *ఇది, RM);
}

APFloat::op_స్థితి DoubleAPFloat::వ్యవకలనం(స్థిరరాశి DoubleAPFloat &RHS,
                                          rounding_బహుళకం RM) {
  changeSign();
  స్వయంచాలక Ret = కూడు(RHS, RM);
  changeSign();
  తిరిగి Ret;
}

APFloat::op_స్థితి DoubleAPFloat::multiply(స్థిరరాశి DoubleAPFloat &RHS,
                                          APFloat::rounding_బహుళకం RM) {
  స్థిరరాశి స్వయంచాలక &LHS = *ఇది;
  స్వయంచాలక &బహిర్గతం = *ఇది;
  /* Interesting observation: For special categories, finding the lowest
     common ancestor of the following layered graph gives the correct
     return category:

        NaN
       /   \
     Zero  Inf
       \   /
సాధారణ

     e.g. NaN * NaN = NaN
          Zero * Inf = NaN
          Normal * Zero = Zero
          Normal * Inf = Inf
  */
  ఒకవేళ (LHS.getCategory() == fcNaN) {
    బహిర్గతం = LHS;
    తిరిగి opOK;
  }
  ఒకవేళ (RHS.getCategory() == fcNaN) {
    బహిర్గతం = RHS;
    తిరిగి opOK;
  }
  ఒకవేళ ((LHS.getCategory() == fcZero && RHS.getCategory() == fcInfinity) ||
      (LHS.getCategory() == fcInfinity && RHS.getCategory() == fcZero)) {
    బహిర్గతం.makeNaN(అబద్ధం, అబద్ధం, శూన్యసూచిక);
    తిరిగి opOK;
  }
  ఒకవేళ (LHS.getCategory() == fcZero || LHS.getCategory() == fcInfinity) {
    బహిర్గతం = LHS;
    తిరిగి opOK;
  }
  ఒకవేళ (RHS.getCategory() == fcZero || RHS.getCategory() == fcInfinity) {
    బహిర్గతం = RHS;
    తిరిగి opOK;
  }
  ప్రతిజ్ఞా(LHS.getCategory() == fc_సాధారణ && RHS.getCategory() == fc_సాధారణ &&
         "Special cases not handled exhaustively");

  పూర్ణాంకం స్థితి_2 = opOK;
  APFloat A = Floats[0], B = Floats[1], C = RHS.Floats[0], D = RHS.Floats[1];
  // t = a * c
  APFloat T = A;
  స్థితి_2 |= T.multiply(C, RM);
  ఒకవేళ (!T.isFiniteNonZero()) {
    Floats[0] = std::move(T);
    Floats[1].makeZero(/* Neg = */ అబద్ధం);
    తిరిగి (op_స్థితి)స్థితి_2;
  }

  // tau = fmsub(a, c, t), that is -fmadd(-a, c, t).
  APFloat Tau = A;
  T.changeSign();
  స్థితి_2 |= Tau.fused_Multiply_కూడు(C, T, RM);
  T.changeSign();
  {
    // v = a * d
    APFloat V = A;
    స్థితి_2 |= V.multiply(D, RM);
    // w = b * c
    APFloat W = B;
    స్థితి_2 |= W.multiply(C, RM);
    స్థితి_2 |= V.కూడు(W, RM);
    // tau += v + w
    స్థితి_2 |= Tau.కూడు(V, RM);
  }
  // u = t + tau
  APFloat U = T;
  స్థితి_2 |= U.కూడు(Tau, RM);

  Floats[0] = U;
  ఒకవేళ (!U.isFinite()) {
    Floats[1].makeZero(/* Neg = */ అబద్ధం);
  } లేకపోతే {
    // Floats[1] = (t - u) + tau
    స్థితి_2 |= T.వ్యవకలనం(U, RM);
    స్థితి_2 |= T.కూడు(Tau, RM);
    Floats[1] = std::move(T);
  }
  తిరిగి (op_స్థితి)స్థితి_2;
}

APFloat::op_స్థితి DoubleAPFloat::divide(స్థిరరాశి DoubleAPFloat &RHS,
                                        APFloat::rounding_బహుళకం RM) {
  ప్రతిజ్ఞా(Semantics == &APFloat_పునాది::semPPCDoubleDouble &&
         "Unexpected Semantics");
  APFloat Tmp(APFloat_పునాది::semPPCDoubleDoubleLegacy, bitcast_గమ్యం_APInt());
  స్వయంచాలక Ret = Tmp.divide(
      APFloat(APFloat_పునాది::semPPCDoubleDoubleLegacy, RHS.bitcast_గమ్యం_APInt()), RM);
  *ఇది = DoubleAPFloat(APFloat_పునాది::semPPCDoubleDouble, Tmp.bitcast_గమ్యం_APInt());
  తిరిగి Ret;
}

APFloat::op_స్థితి DoubleAPFloat::remainder(స్థిరరాశి DoubleAPFloat &RHS) {
  ప్రతిజ్ఞా(Semantics == &APFloat_పునాది::semPPCDoubleDouble &&
         "Unexpected Semantics");
  APFloat Tmp(APFloat_పునాది::semPPCDoubleDoubleLegacy, bitcast_గమ్యం_APInt());
  స్వయంచాలక Ret = Tmp.remainder(
      APFloat(APFloat_పునాది::semPPCDoubleDoubleLegacy, RHS.bitcast_గమ్యం_APInt()));
  *ఇది = DoubleAPFloat(APFloat_పునాది::semPPCDoubleDouble, Tmp.bitcast_గమ్యం_APInt());
  తిరిగి Ret;
}

APFloat::op_స్థితి DoubleAPFloat::mod(స్థిరరాశి DoubleAPFloat &RHS) {
  ప్రతిజ్ఞా(Semantics == &APFloat_పునాది::semPPCDoubleDouble &&
         "Unexpected Semantics");
  APFloat Tmp(APFloat_పునాది::semPPCDoubleDoubleLegacy, bitcast_గమ్యం_APInt());
  స్వయంచాలక Ret = Tmp.mod(
      APFloat(APFloat_పునాది::semPPCDoubleDoubleLegacy, RHS.bitcast_గమ్యం_APInt()));
  *ఇది = DoubleAPFloat(APFloat_పునాది::semPPCDoubleDouble, Tmp.bitcast_గమ్యం_APInt());
  తిరిగి Ret;
}

APFloat::op_స్థితి
DoubleAPFloat::fused_Multiply_కూడు(స్థిరరాశి DoubleAPFloat &Multiplicand,
                                స్థిరరాశి DoubleAPFloat &Addend,
                                APFloat::rounding_బహుళకం RM) {
  ప్రతిజ్ఞా(Semantics == &APFloat_పునాది::semPPCDoubleDouble &&
         "Unexpected Semantics");
  APFloat Tmp(APFloat_పునాది::semPPCDoubleDoubleLegacy, bitcast_గమ్యం_APInt());
  స్వయంచాలక Ret = Tmp.fused_Multiply_కూడు(
      APFloat(APFloat_పునాది::semPPCDoubleDoubleLegacy,
              Multiplicand.bitcast_గమ్యం_APInt()),
      APFloat(APFloat_పునాది::semPPCDoubleDoubleLegacy, Addend.bitcast_గమ్యం_APInt()),
      RM);
  *ఇది = DoubleAPFloat(APFloat_పునాది::semPPCDoubleDouble, Tmp.bitcast_గమ్యం_APInt());
  తిరిగి Ret;
}

APFloat::op_స్థితి DoubleAPFloat::విడత_గమ్యం_Integral(APFloat::rounding_బహుళకం RM) {
  ప్రతిజ్ఞా(Semantics == &APFloat_పునాది::semPPCDoubleDouble &&
         "Unexpected Semantics");
  స్థిరరాశి APFloat &Hi = get_మొదటి();
  స్థిరరాశి APFloat &Lo = get_రెండవ();

  APFloat RoundedHi = Hi;
  స్థిరరాశి op_స్థితి Hi_స్థితి = RoundedHi.విడత_గమ్యం_Integral(RM);

  // We can reduce the problem to just the high part if the input:
  // 1. Represents a non-finite value.
  // 2. Has a component which is zero.
  ఒకవేళ (!Hi.isFiniteNonZero() || Lo.isZero()) {
    Floats[0] = std::move(RoundedHi);
    Floats[1].makeZero(/*Neg=*/అబద్ధం);
    తిరిగి Hi_స్థితి;
  }

  // Adjust `Rounded` in the direction of `TieBreaker` if `ToRound` was at a
  // halfway point.
  స్వయంచాలక విడత_గమ్యం_సమీప_Helper = [](APFloat గమ్యం_విడత, APFloat Rounded,
                                 APFloat TieBreaker) {
    // RoundingError tells us which direction we rounded:
    //   - RoundingError > 0: we rounded up.
    //   - RoundingError < 0: we rounded down.
    // Sterbenz' lemma ensures that RoundingError is exact.
    స్థిరరాశి APFloat RoundingError = Rounded - గమ్యం_విడత;
    ఒకవేళ (TieBreaker.isNonZero() &&
        TieBreaker.isNegative() != RoundingError.isNegative() &&
        నిరపేక్ష(RoundingError).is_Exactly_విలువ(0.5))
      Rounded.కూడు(
          APFloat::getOne(Rounded.getSemantics(), TieBreaker.isNegative()),
          rm_సమీప_Ties_గమ్యం_Even);
    తిరిగి Rounded;
  };

  // Case 1: Hi is not an integer.
  // Special cases are for rounding modes that are sensitive to ties.
  ఒకవేళ (RoundedHi != Hi) {
    // We need to consider the case where Hi was between two integers and the
    // rounding mode broke the tie when, in fact, Lo may have had a different
    // sign than Hi.
    ఒకవేళ (RM == rm_సమీప_Ties_గమ్యం_Away || RM == rm_సమీప_Ties_గమ్యం_Even)
      RoundedHi = విడత_గమ్యం_సమీప_Helper(Hi, RoundedHi, Lo);

    Floats[0] = std::move(RoundedHi);
    Floats[1].makeZero(/*Neg=*/అబద్ధం);
    తిరిగి Hi_స్థితి;
  }

  // Case 2: Hi is an integer.
  // Special cases are for rounding modes which are rounding towards or away from zero.
  Rounding_బహుళకం Lo_Rounding_బహుళకం;
  ఒకవేళ (RM == rmTowardZero)
    // When our input is positive, we want the Lo component rounded toward
    // negative infinity to get the smallest result magnitude. Likewise,
    // negative inputs want the Lo component rounded toward positive infinity.
    Lo_Rounding_బహుళకం = isNegative() ? rm_Toward_ధనాత్మక : rmTowardNegative;
  లేకపోతే
    Lo_Rounding_బహుళకం = RM;

  APFloat RoundedLo = Lo;
  స్థిరరాశి op_స్థితి Lo_స్థితి = RoundedLo.విడత_గమ్యం_Integral(Lo_Rounding_బహుళకం);
  ఒకవేళ (Lo_Rounding_బహుళకం == rm_సమీప_Ties_గమ్యం_Away)
    // We need to consider the case where Lo was between two integers and the
    // rounding mode broke the tie when, in fact, Hi may have had a different
    // sign than Lo.
    RoundedLo = విడత_గమ్యం_సమీప_Helper(Lo, RoundedLo, Hi);

  // We must ensure that the final result has no overlap between the two APFloat values.
  std::tie(RoundedHi, RoundedLo) = fast_Two_కూడిక(RoundedHi, RoundedLo);

  Floats[0] = std::move(RoundedHi);
  Floats[1] = std::move(RoundedLo);
  తిరిగి Lo_స్థితి;
}

శూన్య DoubleAPFloat::changeSign() {
  Floats[0].changeSign();
  Floats[1].changeSign();
}

APFloat::cmp_ఫలితం
DoubleAPFloat::compare_Absolute_విలువ(స్థిరరాశి DoubleAPFloat &RHS) స్థిరరాశి {
  // Compare absolute values of the high parts.
  స్థిరరాశి cmp_ఫలితం HiPartCmp = Floats[0].compare_Absolute_విలువ(RHS.Floats[0]);
  ఒకవేళ (HiPartCmp != cmpEqual)
    తిరిగి HiPartCmp;

  // Zero, regardless of sign, is equal.
  ఒకవేళ (Floats[1].isZero() && RHS.Floats[1].isZero())
    తిరిగి cmpEqual;

  // At this point, |this->Hi| == |RHS.Hi|.
  // The magnitude is |Hi+Lo| which is Hi+|Lo| if signs of Hi and Lo are the
  // same, and Hi-|Lo| if signs are different.
  స్థిరరాశి తార్కిక ThisIsSubtractive =
      Floats[0].isNegative() != Floats[1].isNegative();
  స్థిరరాశి తార్కిక RHSIsSubtractive =
      RHS.Floats[0].isNegative() != RHS.Floats[1].isNegative();

  // Case 1: The low part of 'this' is zero.
  ఒకవేళ (Floats[1].isZero())
    // We are comparing |Hi| vs. |Hi| ± |RHS.Lo|.
    // If RHS is subtractive, its magnitude is smaller.
    // If RHS is additive, its magnitude is larger.
    తిరిగి RHSIsSubtractive ? cmpGreaterThan : cmpLessThan;

  // Case 2: The low part of 'RHS' is zero (and we know 'this' is not).
  ఒకవేళ (RHS.Floats[1].isZero())
    // We are comparing |Hi| ± |This.Lo| vs. |Hi|.
    // If 'this' is subtractive, its magnitude is smaller.
    // If 'this' is additive, its magnitude is larger.
    తిరిగి ThisIsSubtractive ? cmpLessThan : cmpGreaterThan;

  // If their natures differ, the additive one is larger.
  ఒకవేళ (ThisIsSubtractive != RHSIsSubtractive)
    తిరిగి ThisIsSubtractive ? cmpLessThan : cmpGreaterThan;

  // Case 3: Both are additive (Hi+|Lo|) or both are subtractive (Hi-|Lo|).
  // The comparison now depends on the magnitude of the low parts.
  స్థిరరాశి cmp_ఫలితం LoPartCmp = Floats[1].compare_Absolute_విలువ(RHS.Floats[1]);

  ఒకవేళ (ThisIsSubtractive) {
    // Both are subtractive (Hi-|Lo|), so the comparison of |Lo| is inverted.
    ఒకవేళ (LoPartCmp == cmpLessThan)
      తిరిగి cmpGreaterThan;
    ఒకవేళ (LoPartCmp == cmpGreaterThan)
      తిరిగి cmpLessThan;
  }

  // If additive, the comparison of |Lo| is direct.
  // If equal, they are equal.
  తిరిగి LoPartCmp;
}

APFloat::fltCategory DoubleAPFloat::getCategory() స్థిరరాశి {
  తిరిగి Floats[0].getCategory();
}

తార్కిక DoubleAPFloat::isNegative() స్థిరరాశి { తిరిగి Floats[0].isNegative(); }

శూన్య DoubleAPFloat::makeInf(తార్కిక Neg) {
  Floats[0].makeInf(Neg);
  Floats[1].makeZero(/* Neg = */ అబద్ధం);
}

శూన్య DoubleAPFloat::makeZero(తార్కిక Neg) {
  Floats[0].makeZero(Neg);
  Floats[1].makeZero(/* Neg = */ అబద్ధం);
}

శూన్య DoubleAPFloat::make_అతిపెద్ద(తార్కిక Neg) {
  ప్రతిజ్ఞా(Semantics == &APFloat_పునాది::semPPCDoubleDouble &&
         "Unexpected Semantics");
  Floats[0] =
      APFloat(APFloat_పునాది::semIEEEdouble, APInt(64, 0x7fefffffffffffffull));
  Floats[1] =
      APFloat(APFloat_పునాది::semIEEEdouble, APInt(64, 0x7c8ffffffffffffeull));
  ఒకవేళ (Neg)
    changeSign();
}

శూన్య DoubleAPFloat::make_అతిచిన్న(తార్కిక Neg) {
  ప్రతిజ్ఞా(Semantics == &APFloat_పునాది::semPPCDoubleDouble &&
         "Unexpected Semantics");
  Floats[0].make_అతిచిన్న(Neg);
  Floats[1].makeZero(/* Neg = */ అబద్ధం);
}

శూన్య DoubleAPFloat::make_అతిచిన్న_Normalized(తార్కిక Neg) {
  ప్రతిజ్ఞా(Semantics == &APFloat_పునాది::semPPCDoubleDouble &&
         "Unexpected Semantics");
  Floats[0] =
      APFloat(APFloat_పునాది::semIEEEdouble, APInt(64, 0x0360000000000000ull));
  ఒకవేళ (Neg)
    Floats[0].changeSign();
  Floats[1].makeZero(/* Neg = */ అబద్ధం);
}

శూన్య DoubleAPFloat::makeNaN(తార్కిక SNaN, తార్కిక Neg, స్థిరరాశి APInt *fill) {
  Floats[0].makeNaN(SNaN, Neg, fill);
  Floats[1].makeZero(/* Neg = */ అబద్ధం);
}

APFloat::cmp_ఫలితం DoubleAPFloat::compare(స్థిరరాశి DoubleAPFloat &RHS) స్థిరరాశి {
  స్వయంచాలక ఫలితం = Floats[0].compare(RHS.Floats[0]);
  // |Float[0]| > |Float[1]|
  ఒకవేళ (ఫలితం == APFloat::cmpEqual)
    తిరిగి Floats[1].compare(RHS.Floats[1]);
  తిరిగి ఫలితం;
}

తార్కిక DoubleAPFloat::bitwiseIsEqual(స్థిరరాశి DoubleAPFloat &RHS) స్థిరరాశి {
  తిరిగి Floats[0].bitwiseIsEqual(RHS.Floats[0]) &&
         Floats[1].bitwiseIsEqual(RHS.Floats[1]);
}

hash_సంకేతం hash_విలువ(స్థిరరాశి DoubleAPFloat &Arg) {
  ఒకవేళ (Arg.Floats)
    తిరిగి hash_combine(hash_విలువ(Arg.Floats[0]), hash_విలువ(Arg.Floats[1]));
  తిరిగి hash_combine(Arg.Semantics);
}

APInt DoubleAPFloat::bitcast_గమ్యం_APInt() స్థిరరాశి {
  ప్రతిజ్ఞా(Semantics == &APFloat_పునాది::semPPCDoubleDouble &&
         "Unexpected Semantics");
  uint64_t దత్తాంశం_2[] = {
      Floats[0].bitcast_గమ్యం_APInt().get_Raw_దత్తాంశం()[0],
      Floats[1].bitcast_గమ్యం_APInt().get_Raw_దత్తాంశం()[0],
  };
  తిరిగి APInt(128, దత్తాంశం_2);
}

Expected<APFloat::op_స్థితి> DoubleAPFloat::convertFromString(StringRef S,
                                                             rounding_బహుళకం RM) {
  ప్రతిజ్ఞా(Semantics == &APFloat_పునాది::semPPCDoubleDouble &&
         "Unexpected Semantics");
  APFloat Tmp(APFloat_పునాది::semPPCDoubleDoubleLegacy);
  స్వయంచాలక Ret = Tmp.convertFromString(S, RM);
  *ఇది = DoubleAPFloat(APFloat_పునాది::semPPCDoubleDouble, Tmp.bitcast_గమ్యం_APInt());
  తిరిగి Ret;
}

// The double-double lattice of values corresponds to numbers which obey:
// - abs(lo) <= 1/2 * ulp(hi)
// - roundTiesToEven(hi + lo) == hi
//
// nextUp must choose the smallest output > input that follows these rules.
// nexDown must choose the largest output < input that follows these rules.
APFloat::op_స్థితి DoubleAPFloat::తదుపరి(తార్కిక తదుపరి_కిందికి) {
  ప్రతిజ్ఞా(Semantics == &APFloat_పునాది::semPPCDoubleDouble &&
         "Unexpected Semantics");
  // nextDown(x) = -nextUp(-x)
  ఒకవేళ (తదుపరి_కిందికి) {
    changeSign();
    APFloat::op_స్థితి ఫలితం = తదుపరి(/*nextDown=*/అబద్ధం);
    changeSign();
    తిరిగి ఫలితం;
  }
  ఎంపిక (getCategory()) {
  స్థితి fcInfinity:
    // nextUp(+inf) = +inf
    // nextUp(-inf) = -getLargest()
    ఒకవేళ (isNegative())
      make_అతిపెద్ద(నిజం);
    తిరిగి opOK;

  స్థితి fcNaN:
    // IEEE-754R 2008 6.2 Par 2: nextUp(sNaN) = qNaN. Set Invalid flag.
    // IEEE-754R 2008 6.2: nextUp(qNaN) = qNaN. Must be identity so we do not
    //                     change the payload.
    ఒకవేళ (get_మొదటి().isSignaling()) {
      // For consistency, propagate the sign of the sNaN to the qNaN.
      makeNaN(అబద్ధం, isNegative(), శూన్యసూచిక);
      తిరిగి opInvalidOp;
    }
    తిరిగి opOK;

  స్థితి fcZero:
    // nextUp(pm 0) = +getSmallest()
    make_అతిచిన్న(అబద్ధం);
    తిరిగి opOK;

  స్థితి fc_సాధారణ:
    ఆపు;
  }

  స్థిరరాశి APFloat &HiOld = get_మొదటి();
  స్థిరరాశి APFloat &LoOld = get_రెండవ();

  APFloat తదుపరి_Lo = LoOld;
  తదుపరి_Lo.తదుపరి(/*nextDown=*/అబద్ధం);

  // We want to admit values where:
  // 1. abs(Lo) <= ulp(Hi)/2
  // 2. Hi == RTNE(Hi + lo)
  స్వయంచాలక లోపల_Lattice = [](స్థిరరాశి APFloat &Hi, స్థిరరాశి APFloat &Lo) {
    తిరిగి Hi + Lo == Hi;
  };

  // Check if (HiOld, nextUp(LoOld) is in the lattice.
  ఒకవేళ (లోపల_Lattice(HiOld, తదుపరి_Lo)) {
    // Yes, the result is (HiOld, nextUp(LoOld)).
    Floats[1] = std::move(తదుపరి_Lo);

    // TODO: Because we currently rely on semPPCDoubleDoubleLegacy, our maximum
    // value is defined to have exactly 106 bits of precision. This limitation
    // results in semPPCDoubleDouble being unable to reach its maximum canonical
    // value.
    DoubleAPFloat అతిపెద్ద{*Semantics, uninitialized};
    అతిపెద్ద.make_అతిపెద్ద(/*Neg=*/అబద్ధం);
    ఒకవేళ (compare(అతిపెద్ద) == cmpGreaterThan)
      makeInf(/*Neg=*/అబద్ధం);

    తిరిగి opOK;
  }

  // Now we need to handle the cases where (HiOld, nextUp(LoOld)) is not the
  // correct result. We know the new hi component will be nextUp(HiOld) but our
  // lattice rules make it a little ambiguous what the correct NextLo must be.
  APFloat తదుపరి_Hi = HiOld;
  తదుపరి_Hi.తదుపరి(/*nextDown=*/అబద్ధం);

  // nextUp(getLargest()) == INFINITY
  ఒకవేళ (తదుపరి_Hi.isInfinity()) {
    makeInf(/*Neg=*/అబద్ధం);
    తిరిగి opOK;
  }

  // IEEE 754-2019 5.3.1:
  // "If x is the negative number of least magnitude in x's format, nextUp(x) is
  // -0."
  ఒకవేళ (తదుపరి_Hi.isZero()) {
    makeZero(/*Neg=*/నిజం);
    తిరిగి opOK;
  }

  // abs(NextLo) must be <= ulp(NextHi)/2. We want NextLo to be as close to
  // negative infinity as possible.
  తదుపరి_Lo = neg(scalbn(harrisonUlp(తదుపరి_Hi), -1, rmTowardZero));
  ఒకవేళ (!లోపల_Lattice(తదుపరి_Hi, తదుపరి_Lo))
    // RTNE may mean that Lo must be < ulp(NextHi) / 2 so we bump NextLo.
    తదుపరి_Lo.తదుపరి(/*nextDown=*/అబద్ధం);

  Floats[0] = std::move(తదుపరి_Hi);
  Floats[1] = std::move(తదుపరి_Lo);

  తిరిగి opOK;
}

APFloat::op_స్థితి DoubleAPFloat::convert_గమ్యం_Sign_Extended_Integer(
    MutableArrayRef<integerPart> ఆదానం, గుర్తులేని పూర్ణాంకం వెడల్పు, తార్కిక IsSigned,
    rounding_బహుళకం RM, తార్కిక *IsExact) స్థిరరాశి {
  ప్రతిజ్ఞా(Semantics == &APFloat_పునాది::semPPCDoubleDouble &&
         "Unexpected Semantics");

  // If Hi is not finite, or Lo is zero, the value is entirely represented
  // by Hi. Delegate to the simpler single-APFloat conversion.
  ఒకవేళ (!get_మొదటి().isFiniteNonZero() || get_రెండవ().isZero())
    తిరిగి get_మొదటి().convert_గమ్యం_Integer(ఆదానం, వెడల్పు, IsSigned, RM, IsExact);

  // First, round the full double-double value to an integral value. This
  // simplifies the rest of the function, as we no longer need to consider
  // fractional parts.
  *IsExact = అబద్ధం;
  DoubleAPFloat Integral = *ఇది;
  స్థిరరాశి op_స్థితి విడత_స్థితి = Integral.విడత_గమ్యం_Integral(RM);
  ఒకవేళ (విడత_స్థితి == opInvalidOp)
    తిరిగి opInvalidOp;
  స్థిరరాశి APFloat &IntegralHi = Integral.get_మొదటి();
  స్థిరరాశి APFloat &IntegralLo = Integral.get_రెండవ();

  // If rounding results in either component being zero, the sum is trivial.
  // Delegate to the simpler single-APFloat conversion.
  తార్కిక HiIsExact;
  ఒకవేళ (IntegralHi.isZero() || IntegralLo.isZero()) {
    స్థిరరాశి op_స్థితి Hi_స్థితి =
        IntegralHi.convert_గమ్యం_Integer(ఆదానం, వెడల్పు, IsSigned, RM, &HiIsExact);
    // The conversion from an integer-valued float to an APInt may fail if the
    // result would be out of range.  Regardless, taking this path is only
    // possible if rounding occurred during the initial `roundToIntegral`.
    తిరిగి Hi_స్థితి == opOK ? opInexact : Hi_స్థితి;
  }

  // A negative number cannot be represented by an unsigned integer.
  // Since a double-double is canonical, if Hi is negative, the sum is negative.
  ఒకవేళ (!IsSigned && IntegralHi.isNegative())
    తిరిగి opInvalidOp;

  // Handle the special boundary case where |Hi| is exactly the power of two
  // that marks the edge of the integer's range (e.g., 2^63 for int64_t). In
  // this situation, Hi itself won't fit, but the sum Hi + Lo might.
  // `PositiveOverflowWidth` is the bit number for this boundary (N-1 for
  // signed, N for unsigned).
  తార్కిక LoIsExact;
  స్థిరరాశి పూర్ణాంకం HiExactLog2 = IntegralHi.getExactLog2Abs();
  స్థిరరాశి గుర్తులేని ధనాత్మక_అధికప్రవాహం_వెడల్పు = IsSigned ? వెడల్పు - 1 : వెడల్పు;
  ఒకవేళ (HiExactLog2 >= 0 &&
      static_cast<గుర్తులేని>(HiExactLog2) == ధనాత్మక_అధికప్రవాహం_వెడల్పు) {
    // If Hi and Lo have the same sign, |Hi + Lo| > |Hi|, so the sum is
    // guaranteed to overflow. E.g., for uint128_t, (2^128, 1) overflows.
    ఒకవేళ (IntegralHi.isNegative() == IntegralLo.isNegative())
      తిరిగి opInvalidOp;

    // If the signs differ, the sum will fit. We can compute the result using
    // properties of two's complement arithmetic without a wide intermediate
    // integer. E.g., for uint128_t, (2^128, -1) should be 2^128 - 1.
    స్థిరరాశి op_స్థితి Lo_స్థితి = IntegralLo.convert_గమ్యం_Integer(
        ఆదానం, వెడల్పు, /*IsSigned=*/నిజం, RM, &LoIsExact);
    ఒకవేళ (Lo_స్థితి == opInvalidOp)
      తిరిగి opInvalidOp;

    // Adjust the bit pattern of Lo to account for Hi's value:
    //  - For unsigned (Hi=2^Width): `2^Width + Lo` in `Width`-bit
    //    arithmetic is equivalent to just `Lo`. The conversion of `Lo` above
    //    already produced the correct final bit pattern.
    //  - For signed (Hi=2^(Width-1)): The sum `2^(Width-1) + Lo` (where Lo<0)
    //    can be computed by taking the two's complement pattern for `Lo` and
    //    clearing the sign bit.
    ఒకవేళ (IsSigned && !IntegralHi.isNegative())
      APInt::tc_Clear_బిట్టు(ఆదానం.దత్తాంశం(), ధనాత్మక_అధికప్రవాహం_వెడల్పు);
    *IsExact = విడత_స్థితి == opOK;
    తిరిగి విడత_స్థితి;
  }

  // Convert Hi into an integer.  This may not fit but that is OK: we know that
  // Hi + Lo would not fit either in this situation.
  స్థిరరాశి op_స్థితి Hi_స్థితి = IntegralHi.convert_గమ్యం_Integer(
      ఆదానం, వెడల్పు, IsSigned, rmTowardZero, &HiIsExact);
  ఒకవేళ (Hi_స్థితి == opInvalidOp)
    తిరిగి Hi_స్థితి;

  // Convert Lo into a temporary integer of the same width.
  APSInt Lo_ఫలితం{వెడల్పు, /*isUnsigned=*/!IsSigned};
  స్థిరరాశి op_స్థితి Lo_స్థితి =
      IntegralLo.convert_గమ్యం_Integer(Lo_ఫలితం, rmTowardZero, &LoIsExact);
  ఒకవేళ (Lo_స్థితి == opInvalidOp)
    తిరిగి Lo_స్థితి;

  // Add Lo to Hi. This addition is guaranteed not to overflow because of the
  // double-double canonicalization rule (`|Lo| <= ulp(Hi)/2`). The only case
  // where the sum could cross the integer type's boundary is when Hi is a
  // power of two, which is handled by the special case block above.
  APInt::tc_కూడు(ఆదానం.దత్తాంశం(), Lo_ఫలితం.get_Raw_దత్తాంశం(), /*carry=*/0, ఆదానం.కొలత_2());

  *IsExact = విడత_స్థితి == opOK;
  తిరిగి విడత_స్థితి;
}

APFloat::op_స్థితి
DoubleAPFloat::convert_గమ్యం_Integer(MutableArrayRef<integerPart> ఆదానం,
                                గుర్తులేని పూర్ణాంకం వెడల్పు, తార్కిక IsSigned,
                                rounding_బహుళకం RM, తార్కిక *IsExact) స్థిరరాశి {
  op_స్థితి FS =
      convert_గమ్యం_Sign_Extended_Integer(ఆదానం, వెడల్పు, IsSigned, RM, IsExact);

  ఒకవేళ (FS == opInvalidOp) {
    స్థిరరాశి గుర్తులేని Dst_Parts_లెక్క = part_లెక్క_For_బిట్లు(వెడల్పు);
    ప్రతిజ్ఞా(Dst_Parts_లెక్క <= ఆదానం.కొలత_2() && "Integer too big");

    గుర్తులేని బిట్లు_2;
    ఒకవేళ (getCategory() == fcNaN)
      బిట్లు_2 = 0;
    లేకపోతే ఒకవేళ (isNegative())
      బిట్లు_2 = IsSigned;
    లేకపోతే
      బిట్లు_2 = వెడల్పు - IsSigned;

    tc_సమితి_కనిష్ఠ_Significant_బిట్లు(ఆదానం.దత్తాంశం(), Dst_Parts_లెక్క, బిట్లు_2);
    ఒకవేళ (isNegative() && IsSigned)
      APInt::tc_జరుపు_ఎడమ(ఆదానం.దత్తాంశం(), Dst_Parts_లెక్క, వెడల్పు - 1);
  }

  తిరిగి FS;
}

APFloat::op_స్థితి DoubleAPFloat::handle_అధికప్రవాహం(rounding_బహుళకం RM) {
  ఎంపిక (RM) {
  స్థితి APFloat::rmTowardZero:
    make_అతిపెద్ద(/*Neg=*/isNegative());
    ఆపు;
  స్థితి APFloat::rmTowardNegative:
    ఒకవేళ (isNegative())
      makeInf(/*Neg=*/నిజం);
    లేకపోతే
      make_అతిపెద్ద(/*Neg=*/అబద్ధం);
    ఆపు;
  స్థితి APFloat::rm_Toward_ధనాత్మక:
    ఒకవేళ (isNegative())
      make_అతిపెద్ద(/*Neg=*/నిజం);
    లేకపోతే
      makeInf(/*Neg=*/అబద్ధం);
    ఆపు;
  స్థితి APFloat::rm_సమీప_Ties_గమ్యం_Away:
  స్థితి APFloat::rm_సమీప_Ties_గమ్యం_Even:
    makeInf(/*Neg=*/isNegative());
    ఆపు;
  అప్రమేయ:
    llvm_unreachable("Invalid rounding mode found");
  }
  op_స్థితి S = opInexact;
  ఒకవేళ (!get_మొదటి().isFinite())
    S = static_cast<op_స్థితి>(S | op_అధికప్రవాహం);
  తిరిగి S;
}

APFloat::op_స్థితి DoubleAPFloat::convertFromUnsignedParts(
    స్థిరరాశి integerPart *Src, గుర్తులేని పూర్ణాంకం Src_లెక్క, rounding_బహుళకం RM) {
  // Find the most significant bit of the source integer. APInt::tcMSB returns
  // UINT_MAX for a zero value.
  స్థిరరాశి గుర్తులేని SrcMSB = APInt::tcMSB(Src, Src_లెక్క);
  ఒకవేళ (SrcMSB == UINT_MAX) {
    // The source integer is 0.
    makeZero(/*Neg=*/అబద్ధం);
    తిరిగి opOK;
  }

  // Create a minimally-sized APInt to represent the source value.
  స్థిరరాశి గుర్తులేని Src_బిట్టు_వెడల్పు = SrcMSB + 1;
  APSInt SrcInt{APInt{/*numBits=*/Src_బిట్టు_వెడల్పు, ArrayRef(Src, Src_లెక్క)},
                /*isUnsigned=*/నిజం};

  // Stage 1: Initial Approximation.
  // Convert the source integer SrcInt to the Hi part of the DoubleAPFloat.
  // We use round-to-nearest because it minimizes the initial error, which is
  // crucial for the subsequent steps.
  APFloat Hi{get_మొదటి().getSemantics()};
  Hi.convertFromAPInt(SrcInt, /*IsSigned=*/అబద్ధం, rm_సమీప_Ties_గమ్యం_Even);

  // If the first approximation already overflows, the number is too large.
  // NOTE: The underlying semantics are *more* conservative when choosing to
  // overflow because their notion of ULP is much larger. As such, it is always
  // safe to overflow at the DoubleAPFloat level if the APFloat overflows.
  ఒకవేళ (!Hi.isFinite())
    తిరిగి handle_అధికప్రవాహం(RM);

  // Stage 2: Exact Error Calculation.
  // Calculate the exact error of the first approximation: Error = SrcInt - Hi.
  // This is done by converting Hi back to an integer and subtracting it from
  // the original source.
  తార్కిక HiAsIntIsExact;
  // Create an integer representation of Hi. Its width is determined by the
  // exponent of Hi, ensuring it's just large enough. This width can exceed
  // SrcBitWidth if the conversion to Hi rounded up to a power of two.
  // accurately when converted back to an integer.
  APSInt HiAsInt{static_cast<uint32_t>(ilogb(Hi) + 1), /*isUnsigned=*/నిజం};
  Hi.convert_గమ్యం_Integer(HiAsInt, rm_సమీప_Ties_గమ్యం_Even, &HiAsIntIsExact);
  స్థిరరాశి APInt Error = SrcInt.zext(HiAsInt.get_బిట్టు_వెడల్పు()) - HiAsInt;

  // Stage 3: Error Approximation and Rounding.
  // Convert the integer error into the Lo part of the DoubleAPFloat. This step
  // captures the remainder of the original number. The rounding mode for this
  // conversion (LoRM) may need to be adjusted from the user-requested RM to
  // ensure the final sum (Hi + Lo) rounds correctly.
  rounding_బహుళకం LoRM = RM;
  // Adjustments are only necessary when the initial approximation Hi was an
  // overestimate, making the Error negative.
  ఒకవేళ (Error.isNegative()) {
    ఒకవేళ (RM == rm_సమీప_Ties_గమ్యం_Away) {
      // For rmNearestTiesToAway, a tie should round away from zero. Since
      // SrcInt is positive, this means rounding toward +infinity.
      // A standard conversion of a negative Error would round ties toward
      // -infinity, causing the final sum Hi + Lo to be smaller. To
      // counteract this, we detect the tie case and override the rounding
      // mode for Lo to rmTowardPositive.
      స్థిరరాశి గుర్తులేని Error_చురుకైన_బిట్లు = Error.get_Significant_బిట్లు() - 1;
      స్థిరరాశి గుర్తులేని LoPrecision = get_రెండవ().getSemantics().precision;
      ఒకవేళ (Error_చురుకైన_బిట్లు > LoPrecision) {
        స్థిరరాశి గుర్తులేని RoundingBoundary = Error_చురుకైన_బిట్లు - LoPrecision;
        // A tie occurs when the bits to be truncated are of the form 100...0.
        // This is detected by checking if the number of trailing zeros is
        // exactly one less than the number of bits being truncated.
        ఒకవేళ (Error.లెక్క_Trailing_Zeros() == RoundingBoundary - 1)
          LoRM = rm_Toward_ధనాత్మక;
      }
    } లేకపోతే ఒకవేళ (RM == rmTowardZero) {
      // For rmTowardZero, the final positive result must be truncated (rounded
      // down). When Hi is an overestimate, Error is negative. A standard
      // rmTowardZero conversion of Error would make it *less* negative,
      // effectively rounding the final sum Hi + Lo *up*. To ensure the sum
      // rounds down correctly, we force Lo to round toward -infinity.
      LoRM = rmTowardNegative;
    }
  }

  APFloat Lo{get_రెండవ().getSemantics()};
  op_స్థితి స్థితి_2 = Lo.convertFromAPInt(Error, /*IsSigned=*/నిజం, LoRM);

  // Renormalize the pair (Hi, Lo) into a canonical DoubleAPFloat form where the
  // components do not overlap. fastTwoSum performs this operation.
  std::tie(Hi, Lo) = fast_Two_కూడిక(Hi, Lo);
  Floats[0] = std::move(Hi);
  Floats[1] = std::move(Lo);

  // A final check for overflow is needed because fastTwoSum can cause a
  // carry-out from Lo that pushes Hi to infinity.
  ఒకవేళ (!get_మొదటి().isFinite())
    తిరిగి handle_అధికప్రవాహం(RM);

  // The largest DoubleAPFloat must be canonical. Values which are larger are
  // not canonical and are equivalent to overflow.
  ఒకవేళ (get_మొదటి().isFiniteNonZero() && Floats[0].is_అతిపెద్ద()) {
    DoubleAPFloat అతిపెద్ద{*Semantics};
    అతిపెద్ద.make_అతిపెద్ద(/*Neg=*/అబద్ధం);
    ఒకవేళ (compare(అతిపెద్ద) == APFloat::cmpGreaterThan)
      తిరిగి handle_అధికప్రవాహం(RM);
  }

  // The final status of the operation is determined by the conversion of the
  // error term. If Lo could represent Error exactly, the entire conversion
  // is exact. Otherwise, it's inexact.
  తిరిగి స్థితి_2;
}

APFloat::op_స్థితి DoubleAPFloat::convertFromAPInt(స్థిరరాశి APInt &ఆదానం,
                                                  తార్కిక IsSigned,
                                                  rounding_బహుళకం RM) {
  స్థిరరాశి తార్కిక Negate_ఆదానం = IsSigned && ఆదానం.isNegative();
  APInt API = ఆదానం;
  ఒకవేళ (Negate_ఆదానం)
    API.negate();

  స్థిరరాశి APFloat::op_స్థితి స్థితి_2 =
      convertFromUnsignedParts(API.get_Raw_దత్తాంశం(), API.get_అంకె_పదాలు(), RM);
  ఒకవేళ (Negate_ఆదానం)
    changeSign();
  తిరిగి స్థితి_2;
}

గుర్తులేని పూర్ణాంకం DoubleAPFloat::convert_గమ్యం_Hex_String(అక్షరం *DST,
                                               గుర్తులేని పూర్ణాంకం HexDigits,
                                               తార్కిక పై_Case_2,
                                               rounding_బహుళకం RM) స్థిరరాశి {
  ప్రతిజ్ఞా(Semantics == &APFloat_పునాది::semPPCDoubleDouble &&
         "Unexpected Semantics");
  తిరిగి APFloat(APFloat_పునాది::semPPCDoubleDoubleLegacy, bitcast_గమ్యం_APInt())
      .convert_గమ్యం_Hex_String(DST, HexDigits, పై_Case_2, RM);
}

తార్కిక DoubleAPFloat::isDenormal() స్థిరరాశి {
  తిరిగి getCategory() == fc_సాధారణ &&
         (Floats[0].isDenormal() || Floats[1].isDenormal() ||
          // (double)(Hi + Lo) == Hi defines a normal number.
          Floats[0] != Floats[0] + Floats[1]);
}

తార్కిక DoubleAPFloat::is_అతిచిన్న() స్థిరరాశి {
  ఒకవేళ (getCategory() != fc_సాధారణ)
    తిరిగి అబద్ధం;
  DoubleAPFloat Tmp(*ఇది);
  Tmp.make_అతిచిన్న(ఇది->isNegative());
  తిరిగి Tmp.compare(*ఇది) == cmpEqual;
}

తార్కిక DoubleAPFloat::is_అతిచిన్న_Normalized() స్థిరరాశి {
  ఒకవేళ (getCategory() != fc_సాధారణ)
    తిరిగి అబద్ధం;

  DoubleAPFloat Tmp(*ఇది);
  Tmp.make_అతిచిన్న_Normalized(ఇది->isNegative());
  తిరిగి Tmp.compare(*ఇది) == cmpEqual;
}

తార్కిక DoubleAPFloat::is_అతిపెద్ద() స్థిరరాశి {
  ఒకవేళ (getCategory() != fc_సాధారణ)
    తిరిగి అబద్ధం;
  DoubleAPFloat Tmp(*ఇది);
  Tmp.make_అతిపెద్ద(ఇది->isNegative());
  తిరిగి Tmp.compare(*ఇది) == cmpEqual;
}

తార్కిక DoubleAPFloat::isInteger() స్థిరరాశి {
  ప్రతిజ్ఞా(Semantics == &APFloat_పునాది::semPPCDoubleDouble &&
         "Unexpected Semantics");
  తిరిగి Floats[0].isInteger() && Floats[1].isInteger();
}

శూన్య DoubleAPFloat::గమ్యం_String(Small_సదిశ_Impl<అక్షరం> &Str,
                             గుర్తులేని FormatPrecision,
                             గుర్తులేని FormatMaxPadding,
                             తార్కిక TruncateZero) స్థిరరాశి {
  ప్రతిజ్ఞా(Semantics == &APFloat_పునాది::semPPCDoubleDouble &&
         "Unexpected Semantics");
  APFloat(APFloat_పునాది::semPPCDoubleDoubleLegacy, bitcast_గమ్యం_APInt())
      .గమ్యం_String(Str, FormatPrecision, FormatMaxPadding, TruncateZero);
}

పూర్ణాంకం DoubleAPFloat::getExactLog2Abs() స్థిరరాశి {
  // In order for Hi + Lo to be a power of two, the following must be true:
  // 1. Hi must be a power of two.
  // 2. Lo must be zero.
  ఒకవేళ (get_రెండవ().isNonZero())
    తిరిగి INT_MIN;
  తిరిగి get_మొదటి().getExactLog2Abs();
}

పూర్ణాంకం ilogb(స్థిరరాశి DoubleAPFloat &Arg) {
  స్థిరరాశి APFloat &Hi = Arg.get_మొదటి();
  స్థిరరాశి APFloat &Lo = Arg.get_రెండవ();
  పూర్ణాంకం Ilogb_ఫలితం = ilogb(Hi);
  // Zero and non-finite values can delegate to ilogb(Hi).
  ఒకవేళ (Arg.getCategory() != fc_సాధారణ)
    తిరిగి Ilogb_ఫలితం;
  // If Lo can't change the binade, we can delegate to ilogb(Hi).
  ఒకవేళ (Lo.isZero() || Hi.isNegative() == Lo.isNegative())
    తిరిగి Ilogb_ఫలితం;
  ఒకవేళ (Hi.getExactLog2Abs() == INT_MIN)
    తిరిగి Ilogb_ఫలితం;
  // Numbers of the form 2^a - 2^b or -2^a + 2^b are almost powers of two but
  // get nudged out of the binade by the low component.
  తిరిగి Ilogb_ఫలితం - 1;
}

DoubleAPFloat scalbn(స్థిరరాశి DoubleAPFloat &Arg, పూర్ణాంకం Exp,
                     APFloat::rounding_బహుళకం RM) {
  ప్రతిజ్ఞా(Arg.Semantics == &APFloat_పునాది::PPCDoubleDouble() &&
         "Unexpected Semantics");
  తిరిగి DoubleAPFloat(APFloat_పునాది::PPCDoubleDouble(),
                       scalbn(Arg.Floats[0], Exp, RM),
                       scalbn(Arg.Floats[1], Exp, RM));
}

DoubleAPFloat frexp(స్థిరరాశి DoubleAPFloat &Arg, పూర్ణాంకం &Exp,
                    APFloat::rounding_బహుళకం RM) {
  ప్రతిజ్ఞా(Arg.Semantics == &APFloat_పునాది::PPCDoubleDouble() &&
         "Unexpected Semantics");

  // Get the unbiased exponent e of the number, where |Arg| = m * 2^e for m in
  // [1.0, 2.0).
  Exp = ilogb(Arg);

  // For NaNs, quiet any signaling NaN and return the result, as per standard
  // practice.
  ఒకవేళ (Exp == APFloat::IEK_NaN) {
    DoubleAPFloat Quiet{Arg};
    Quiet.get_మొదటి() = Quiet.get_మొదటి().makeQuiet();
    తిరిగి Quiet;
  }

  // For infinity, return it unchanged. The exponent remains IEK_Inf.
  ఒకవేళ (Exp == APFloat::IEK_Inf)
    తిరిగి Arg;

  // For zero, the fraction is zero and the standard requires the exponent be 0.
  ఒకవేళ (Exp == APFloat::IEK_Zero) {
    Exp = 0;
    తిరిగి Arg;
  }

  స్థిరరాశి APFloat &Hi = Arg.get_మొదటి();
  స్థిరరాశి APFloat &Lo = Arg.get_రెండవ();

  // frexp requires the fraction's absolute value to be in [0.5, 1.0).
  // ilogb provides an exponent for an absolute value in [1.0, 2.0).
  // Increment the exponent to ensure the fraction is in the correct range.
  ++Exp;

  స్థిరరాశి తార్కిక SignsDisagree = Hi.isNegative() != Lo.isNegative();
  APFloat రెండవ = Lo;
  ఒకవేళ (Arg.getCategory() == APFloat::fc_సాధారణ && Lo.isFiniteNonZero()) {
    rounding_బహుళకం Lo_Rounding_బహుళకం;
    // The interpretation of rmTowardZero depends on the sign of the combined
    // Arg rather than the sign of the component.
    ఒకవేళ (RM == rmTowardZero)
      Lo_Rounding_బహుళకం = Arg.isNegative() ? rm_Toward_ధనాత్మక : rmTowardNegative;
    // For rmNearestTiesToAway, we face a similar problem. If signs disagree,
    // Lo is a correction *toward* zero relative to Hi. Rounding Lo
    // "away from zero" based on its own sign would move the value in the
    // wrong direction. As a safe proxy, we use rmNearestTiesToEven, which is
    // direction-agnostic. We only need to bother with this if Lo is scaled
    // down.
    లేకపోతే ఒకవేళ (RM == rm_సమీప_Ties_గమ్యం_Away && SignsDisagree && Exp > 0)
      Lo_Rounding_బహుళకం = rm_సమీప_Ties_గమ్యం_Even;
    లేకపోతే
      Lo_Rounding_బహుళకం = RM;
    రెండవ = scalbn(Lo, -Exp, Lo_Rounding_బహుళకం);
    // The rmNearestTiesToEven proxy is correct most of the time, but it
    // differs from rmNearestTiesToAway when the scaled value of Lo is an
    // exact midpoint.
    // NOTE: This is morally equivalent to roundTiesTowardZero.
    ఒకవేళ (RM == rm_సమీప_Ties_గమ్యం_Away && Lo_Rounding_బహుళకం == rm_సమీప_Ties_గమ్యం_Even) {
      // Re-scale the result back to check if rounding occurred.
      స్థిరరాశి APFloat RecomposedLo = scalbn(రెండవ, Exp, rm_సమీప_Ties_గమ్యం_Even);
      ఒకవేళ (RecomposedLo != Lo) {
        // RoundingError tells us which direction we rounded:
        //   - RoundingError > 0: we rounded up.
        //   - RoundingError < 0: we down up.
        స్థిరరాశి APFloat RoundingError = RecomposedLo - Lo;
        // Determine if scalbn(Lo, -Exp) landed exactly on a midpoint.
        // We do this by checking if the absolute rounding error is exactly
        // half a ULP of the result.
        స్థిరరాశి APFloat Ulp_యొక్క_రెండవ = harrisonUlp(రెండవ);
        స్థిరరాశి APFloat Scaled_Ulp_యొక్క_రెండవ =
            scalbn(Ulp_యొక్క_రెండవ, Exp - 1, rm_సమీప_Ties_గమ్యం_Even);
        స్థిరరాశి తార్కిక IsMidpoint = నిరపేక్ష(RoundingError) == Scaled_Ulp_యొక్క_రెండవ;
        స్థిరరాశి తార్కిక RoundedLoAway =
            రెండవ.isNegative() == RoundingError.isNegative();
        // The sign of Hi and Lo disagree and we rounded Lo away: we must
        // decrease the magnitude of Second to increase the magnitude
        // First+Second.
        ఒకవేళ (IsMidpoint && RoundedLoAway)
          రెండవ.తదుపరి(/*nextDown=*/!రెండవ.isNegative());
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
    ఒకవేళ (రెండవ.isZero() && SignsDisagree && Hi.getExactLog2Abs() != INT_MIN)
      ++Exp;
  }

  APFloat మొదటి = scalbn(Hi, -Exp, RM);
  తిరిగి DoubleAPFloat(APFloat_పునాది::PPCDoubleDouble(), std::move(మొదటి),
                       std::move(రెండవ));
}

APInt DoubleAPFloat::getNaNPayload() స్థిరరాశి { తిరిగి Floats[0].getNaNPayload(); }
} // namespace detail

APFloat::Storage::Storage(IEEEFloat F, స్థిరరాశి fltSemantics &Semantics) {
  ఒకవేళ (ఉపయోగిస్తుంది_Layout<IEEEFloat>(Semantics)) {
    కొత్త (&IEEE) IEEEFloat(std::move(F));
    తిరిగి;
  }
  ఒకవేళ (ఉపయోగిస్తుంది_Layout<DoubleAPFloat>(Semantics)) {
    స్థిరరాశి fltSemantics& S = F.getSemantics();
    కొత్త (&Double) DoubleAPFloat(Semantics, APFloat(std::move(F), S),
                                APFloat(APFloat_పునాది::IEEEdouble()));
    తిరిగి;
  }
  llvm_unreachable("Unexpected semantics");
}

Expected<APFloat::op_స్థితి> APFloat::convertFromString(StringRef Str,
                                                       rounding_బహుళకం RM) {
  APFLOAT_DISPATCH_ON_SEMANTICS(convertFromString(Str, RM));
}

hash_సంకేతం hash_విలువ(స్థిరరాశి APFloat &Arg) {
  ఒకవేళ (APFloat::ఉపయోగిస్తుంది_Layout<detail::IEEEFloat>(Arg.getSemantics()))
    తిరిగి hash_విలువ(Arg.U.IEEE);
  ఒకవేళ (APFloat::ఉపయోగిస్తుంది_Layout<detail::DoubleAPFloat>(Arg.getSemantics()))
    తిరిగి hash_విలువ(Arg.U.Double);
  llvm_unreachable("Unexpected semantics");
}

APFloat::APFloat(స్థిరరాశి fltSemantics &Semantics, StringRef S)
    : APFloat(Semantics) {
  స్వయంచాలక స్థితి_Or_Err = convertFromString(S, rm_సమీప_Ties_గమ్యం_Even);
  ప్రతిజ్ఞా(స్థితి_Or_Err && "Invalid floating point representation");
  consumeError(స్థితి_Or_Err.takeError());
}

FPClassTest APFloat::classify() స్థిరరాశి {
  ఒకవేళ (isZero())
    తిరిగి isNegative() ? fcNegZero : fcPosZero;
  ఒకవేళ (is_సాధారణ())
    తిరిగి isNegative() ? fc_Neg_సాధారణ : fc_Pos_సాధారణ;
  ఒకవేళ (isDenormal())
    తిరిగి isNegative() ? fcNegSubnormal : fcPosSubnormal;
  ఒకవేళ (isInfinity())
    తిరిగి isNegative() ? fcNegInf : fcPosInf;
  ప్రతిజ్ఞా(isNaN() && "Other class of FP constant");
  తిరిగి isSignaling() ? fcSNan : fcQNan;
}

తార్కిక APFloat::getExactInverse(APFloat *Inv) స్థిరరాశి {
  // Only finite, non-zero numbers can have a useful, representable inverse.
  // This check filters out +/- zero, +/- infinity, and NaN.
  ఒకవేళ (!isFiniteNonZero())
    తిరిగి అబద్ధం;

  // Historically, this function rejects subnormal inputs.  One reason why this
  // might be important is that subnormals may behave differently under FTZ/DAZ
  // runtime behavior.
  ఒకవేళ (isDenormal())
    తిరిగి అబద్ధం;

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
  స్థిరరాశి పూర్ణాంకం Exp = getExactLog2Abs();
  ఒకవేళ (Exp == INT_MIN)
    తిరిగి అబద్ధం;

  // The inverse of +/- 2^Exp is +/- 2^(-Exp). We can compute this by
  // scaling 1.0 by the negated exponent.
  APFloat Reciprocal =
      scalbn(APFloat::getOne(getSemantics(), /*Negative=*/isNegative()), -Exp,
             rmTowardZero);

  // scalbn might round if the resulting exponent -Exp is outside the
  // representable range, causing overflow (to infinity) or underflow. We
  // must verify that the result is still the exact power of two we expect.
  ఒకవేళ (Reciprocal.getExactLog2Abs() != -Exp)
    తిరిగి అబద్ధం;

  // Avoid multiplication with a subnormal, it is not safe on all platforms and
  // may be slower than a normal division.
  ఒకవేళ (Reciprocal.isDenormal())
    తిరిగి అబద్ధం;

  ప్రతిజ్ఞా(Reciprocal.isFiniteNonZero());

  ఒకవేళ (Inv)
    *Inv = std::move(Reciprocal);

  తిరిగి నిజం;
}

APFloat::op_స్థితి APFloat::convert(స్థిరరాశి fltSemantics &గమ్యం_Semantics_2,
                                   rounding_బహుళకం RM, తార్కిక *losesInfo) {
  ఒకవేళ (&getSemantics() == &గమ్యం_Semantics_2) {
    *losesInfo = అబద్ధం;
    తిరిగి opOK;
  }
  ఒకవేళ (ఉపయోగిస్తుంది_Layout<IEEEFloat>(getSemantics()) &&
      ఉపయోగిస్తుంది_Layout<IEEEFloat>(గమ్యం_Semantics_2))
    తిరిగి U.IEEE.convert(గమ్యం_Semantics_2, RM, losesInfo);
  ఒకవేళ (ఉపయోగిస్తుంది_Layout<IEEEFloat>(getSemantics()) &&
      ఉపయోగిస్తుంది_Layout<DoubleAPFloat>(గమ్యం_Semantics_2)) {
    ప్రతిజ్ఞా(&గమ్యం_Semantics_2 == &APFloat_పునాది::semPPCDoubleDouble);
    స్వయంచాలక Ret =
        U.IEEE.convert(APFloat_పునాది::semPPCDoubleDoubleLegacy, RM, losesInfo);
    *ఇది = APFloat(గమ్యం_Semantics_2, U.IEEE.bitcast_గమ్యం_APInt());
    తిరిగి Ret;
  }
  ఒకవేళ (ఉపయోగిస్తుంది_Layout<DoubleAPFloat>(getSemantics()) &&
      ఉపయోగిస్తుంది_Layout<IEEEFloat>(గమ్యం_Semantics_2)) {
    స్వయంచాలక Ret = getIEEE().convert(గమ్యం_Semantics_2, RM, losesInfo);
    *ఇది = APFloat(std::move(getIEEE()), గమ్యం_Semantics_2);
    తిరిగి Ret;
  }
  llvm_unreachable("Unexpected semantics");
}

APFloat APFloat::get_All_Ones_విలువ(స్థిరరాశి fltSemantics &Semantics) {
  తిరిగి APFloat(Semantics, APInt::getAllOnes(Semantics.కొలత_లోపల_బిట్లు));
}

శూన్య APFloat::print(raw_ostream &OS) స్థిరరాశి {
  Small_సదిశ<అక్షరం, 16> బఫరు;
  గమ్యం_String(బఫరు);
  OS << బఫరు;
}

#ఒకవేళ !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)
LLVM_DUMP_పద్ధతి శూన్య APFloat::dump() స్థిరరాశి {
  print(dbgs());
  dbgs() << '\n';
}
#అయితే_ముగింపు

శూన్య APFloat::Profile(Folding_సమితి_కణుపు_ID &NID) స్థిరరాశి {
  NID.కూడు_2(bitcast_గమ్యం_APInt());
}

APFloat::op_స్థితి APFloat::convert_గమ్యం_Integer(APSInt &ఫలితం_2,
                                            rounding_బహుళకం rounding_బహుళకం_2,
                                            తార్కిక *isExact) స్థిరరాశి {
  గుర్తులేని బిట్టు_వెడల్పు = ఫలితం_2.get_బిట్టు_వెడల్పు();
  Small_సదిశ<uint64_t, 4> parts(ఫలితం_2.get_అంకె_పదాలు());
  op_స్థితి స్థితి_3 = convert_గమ్యం_Integer(parts, బిట్టు_వెడల్పు, ఫలితం_2.isSigned(),
                                     rounding_బహుళకం_2, isExact);
  // Keeps the original signed-ness.
  ఫలితం_2 = APInt(బిట్టు_వెడల్పు, parts);
  తిరిగి స్థితి_3;
}

ద్విగుణ APFloat::convert_గమ్యం_Double() స్థిరరాశి {
  ఒకవేళ (&getSemantics() == &APFloat_పునాది::semIEEEdouble)
    తిరిగి getIEEE().convert_గమ్యం_Double();
  ప్రతిజ్ఞా(isRepresentableBy(getSemantics(), semIEEEdouble) &&
         "Float semantics is not representable by IEEEdouble");
  APFloat తాత్కాలిక = *ఇది;
  తార్కిక LosesInfo;
  [[maybe_వాడని]] op_స్థితి St =
      తాత్కాలిక.convert(APFloat_పునాది::semIEEEdouble, rm_సమీప_Ties_గమ్యం_Even, &LosesInfo);
  ప్రతిజ్ఞా(!(St & opInexact) && !LosesInfo && "Unexpected imprecision");
  తిరిగి తాత్కాలిక.getIEEE().convert_గమ్యం_Double();
}

#నిర్వచితమైతే HAS_IEE754_FLOAT128
float128 APFloat::convert_గమ్యం_Quad() స్థిరరాశి {
  ఒకవేళ (&getSemantics() == &APFloat_పునాది::semIEEEquad)
    తిరిగి getIEEE().convert_గమ్యం_Quad();
  ప్రతిజ్ఞా(isRepresentableBy(getSemantics(), semIEEEquad) &&
         "Float semantics is not representable by IEEEquad");
  APFloat తాత్కాలిక = *ఇది;
  తార్కిక LosesInfo;
  [[maybe_వాడని]] op_స్థితి St =
      తాత్కాలిక.convert(APFloat_పునాది::semIEEEquad, rm_సమీప_Ties_గమ్యం_Even, &LosesInfo);
  ప్రతిజ్ఞా(!(St & opInexact) && !LosesInfo && "Unexpected imprecision");
  తిరిగి తాత్కాలిక.getIEEE().convert_గమ్యం_Quad();
}
#అయితే_ముగింపు

దశాంశం APFloat::convert_గమ్యం_Float() స్థిరరాశి {
  ఒకవేళ (&getSemantics() == &APFloat_పునాది::semIEEEsingle)
    తిరిగి getIEEE().convert_గమ్యం_Float();
  ప్రతిజ్ఞా(isRepresentableBy(getSemantics(), semIEEEsingle) &&
         "Float semantics is not representable by IEEEsingle");
  APFloat తాత్కాలిక = *ఇది;
  తార్కిక LosesInfo;
  [[maybe_వాడని]] op_స్థితి St =
      తాత్కాలిక.convert(APFloat_పునాది::semIEEEsingle, rm_సమీప_Ties_గమ్యం_Even, &LosesInfo);
  ప్రతిజ్ఞా(!(St & opInexact) && !LosesInfo && "Unexpected imprecision");
  తిరిగి తాత్కాలిక.getIEEE().convert_గమ్యం_Float();
}

గుర్తులేని APFloat_పునాది::get_Arbitrary_FPFormat_కొలత_లోపల_బిట్లు(StringRef Format) {
  తిరిగి StringSwitch<గుర్తులేని>(Format)
      .Case("Float8E5M2", get_కొలత_లోపల_బిట్లు(semFloat8E5M2))
      .Case("Float8E5M2FNUZ", get_కొలత_లోపల_బిట్లు(semFloat8E5M2FNUZ))
      .Case("Float8E4M3", get_కొలత_లోపల_బిట్లు(semFloat8E4M3))
      .Case("Float8E4M3FN", get_కొలత_లోపల_బిట్లు(semFloat8E4M3FN))
      .Case("Float8E4M3FNUZ", get_కొలత_లోపల_బిట్లు(semFloat8E4M3FNUZ))
      .Case("Float8E4M3B11FNUZ", get_కొలత_లోపల_బిట్లు(semFloat8E4M3B11FNUZ))
      .Case("Float8E3M4", get_కొలత_లోపల_బిట్లు(semFloat8E3M4))
      .Case("Float8E8M0FNU", get_కొలత_లోపల_బిట్లు(semFloat8E8M0FNU))
      .Case("Float6E3M2FN", get_కొలత_లోపల_బిట్లు(semFloat6E3M2FN))
      .Case("Float6E2M3FN", get_కొలత_లోపల_బిట్లు(semFloat6E2M3FN))
      .Case("Float4E2M1FN", get_కొలత_లోపల_బిట్లు(semFloat4E2M1FN))
      .Case("Float8E5M3FNU", get_కొలత_లోపల_బిట్లు(semFloat8E5M3FNU))
      .Default(0);
}

తార్కిక APFloat_పునాది::isValidArbitraryFPFormat(StringRef Format) {
  తిరిగి get_Arbitrary_FPFormat_కొలత_లోపల_బిట్లు(Format) != 0;
}

స్థిరరాశి fltSemantics *APFloat_పునాది::getArbitraryFPSemantics(StringRef Format) {
  // TODO: extend to remaining arbitrary FP types: Float8E4M3, Float8E3M4,
  // Float8E5M2FNUZ, Float8E4M3FNUZ, Float8E4M3B11FNUZ, Float8E8M0FNU.
  తిరిగి StringSwitch<స్థిరరాశి fltSemantics *>(Format)
      .Case("Float8E5M2", &semFloat8E5M2)
      .Case("Float8E4M3FN", &semFloat8E4M3FN)
      .Case("Float8E5M3FNU", &semFloat8E5M3FNU)
      .Case("Float4E2M1FN", &semFloat4E2M1FN)
      .Case("Float6E3M2FN", &semFloat6E3M2FN)
      .Case("Float6E2M3FN", &semFloat6E2M3FN)
      .Default(శూన్యసూచిక);
}

APFloat::Storage::~Storage() {
  ఒకవేళ (ఉపయోగిస్తుంది_Layout<IEEEFloat>(*semantics)) {
    IEEE.~IEEEFloat();
    తిరిగి;
  }
  ఒకవేళ (ఉపయోగిస్తుంది_Layout<DoubleAPFloat>(*semantics)) {
    Double.~DoubleAPFloat();
    తిరిగి;
  }
  llvm_unreachable("Unexpected semantics");
}

APFloat::Storage::Storage(స్థిరరాశి APFloat::Storage &RHS) {
  ఒకవేళ (ఉపయోగిస్తుంది_Layout<IEEEFloat>(*RHS.semantics)) {
    కొత్త (ఇది) IEEEFloat(RHS.IEEE);
    తిరిగి;
  }
  ఒకవేళ (ఉపయోగిస్తుంది_Layout<DoubleAPFloat>(*RHS.semantics)) {
    కొత్త (ఇది) DoubleAPFloat(RHS.Double);
    తిరిగి;
  }
  llvm_unreachable("Unexpected semantics");
}

APFloat::Storage::Storage(APFloat::Storage &&RHS) {
  ఒకవేళ (ఉపయోగిస్తుంది_Layout<IEEEFloat>(*RHS.semantics)) {
    కొత్త (ఇది) IEEEFloat(std::move(RHS.IEEE));
    తిరిగి;
  }
  ఒకవేళ (ఉపయోగిస్తుంది_Layout<DoubleAPFloat>(*RHS.semantics)) {
    కొత్త (ఇది) DoubleAPFloat(std::move(RHS.Double));
    తిరిగి;
  }
  llvm_unreachable("Unexpected semantics");
}

APFloat::Storage &APFloat::Storage::క్రియాకారకం=(స్థిరరాశి APFloat::Storage &RHS) {
  ఒకవేళ (ఉపయోగిస్తుంది_Layout<IEEEFloat>(*semantics) &&
      ఉపయోగిస్తుంది_Layout<IEEEFloat>(*RHS.semantics)) {
    IEEE = RHS.IEEE;
  } లేకపోతే ఒకవేళ (ఉపయోగిస్తుంది_Layout<DoubleAPFloat>(*semantics) &&
             ఉపయోగిస్తుంది_Layout<DoubleAPFloat>(*RHS.semantics)) {
    Double = RHS.Double;
  } లేకపోతే ఒకవేళ (ఇది != &RHS) {
    ఇది->~Storage();
    కొత్త (ఇది) Storage(RHS);
  }
  తిరిగి *ఇది;
}

APFloat::Storage &APFloat::Storage::క్రియాకారకం=(APFloat::Storage &&RHS) {
  ఒకవేళ (ఉపయోగిస్తుంది_Layout<IEEEFloat>(*semantics) &&
      ఉపయోగిస్తుంది_Layout<IEEEFloat>(*RHS.semantics)) {
    IEEE = std::move(RHS.IEEE);
  } లేకపోతే ఒకవేళ (ఉపయోగిస్తుంది_Layout<DoubleAPFloat>(*semantics) &&
             ఉపయోగిస్తుంది_Layout<DoubleAPFloat>(*RHS.semantics)) {
    Double = std::move(RHS.Double);
  } లేకపోతే ఒకవేళ (ఇది != &RHS) {
    ఇది->~Storage();
    కొత్త (ఇది) Storage(std::move(RHS));
  }
  తిరిగి *ఇది;
}

నామాకాశ {

APFloat::op_స్థితి get_Op_స్థితి_From_Libc(పూర్ణాంకం libc_exceptions) {
  APFloat::op_స్థితి స్థితి_3 = APFloat::opOK;
  ఒకవేళ (libc_exceptions & FE_INVALID)
    స్థితి_3 = static_cast<APFloat::op_స్థితి>(స్థితి_3 | APFloat::opInvalidOp);
  ఒకవేళ (libc_exceptions & FE_DIVBYZERO)
    స్థితి_3 = static_cast<APFloat::op_స్థితి>(స్థితి_3 | APFloat::opDivByZero);
  ఒకవేళ (libc_exceptions & FE_అధికప్రవాహం)
    స్థితి_3 = static_cast<APFloat::op_స్థితి>(స్థితి_3 | APFloat::op_అధికప్రవాహం);
  ఒకవేళ (libc_exceptions & FE_UNDERFLOW)
    స్థితి_3 = static_cast<APFloat::op_స్థితి>(స్థితి_3 | APFloat::opUnderflow);
  ఒకవేళ (libc_exceptions & FE_INEXACT)
    స్థితి_3 = static_cast<APFloat::op_స్థితి>(స్థితి_3 | APFloat::opInexact);
  తిరిగి స్థితి_3;
}

} // namespace

// TODO: Support other rounding modes when LLVM libc math implement static
// roundings.
std::optional<APFloat> ఘాతాంకం(స్థిరరాశి APFloat &x, Rounding_బహుళకం rounding_బహుళకం_2,
                           APFloat::op_స్థితి *స్థితి_3) {

  ఒకవేళ (rounding_బహుళకం_2 == APFloat_పునాది::rm_సమీప_Ties_గమ్యం_Even) {
    ఒకవేళ (APFloat::Semantics_గమ్యం_Enum(x.getSemantics()) ==
        APFloat_పునాది::S_IEEEsingle) {
      దశాంశం x_val = x.convert_గమ్యం_Float();
      పూర్ణాంకం exc =
          LIBC_NAMESPACE::shared::check::exp_exceptions(x_val, FE_TONEAREST);
      ఒకవేళ (స్థితి_3) {
        *స్థితి_3 = get_Op_స్థితి_From_Libc(exc);
        ఒకవేళ (x.isSignaling()) {
          // 32-bit x86 will silence sNaN when loading floats, so we explicitly
          // add the INVALID exception here.
          *స్థితి_3 =
              static_cast<APFloat::op_స్థితి>(*స్థితి_3 | APFloat::opInvalidOp);
        }
      }
      దశాంశం ఫలితం_2 = LIBC_NAMESPACE::shared::expf(x_val);
      తిరిగి APFloat(ఫలితం_2);
    }
    ఒకవేళ (APFloat::Semantics_గమ్యం_Enum(x.getSemantics()) ==
        APFloat_పునాది::S_IEEEdouble) {
      ద్విగుణ x_val = x.convert_గమ్యం_Double();
      పూర్ణాంకం exc =
          LIBC_NAMESPACE::shared::check::exp_exceptions(x_val, FE_TONEAREST);
      ఒకవేళ (స్థితి_3) {
        *స్థితి_3 = get_Op_స్థితి_From_Libc(exc);
        ఒకవేళ (x.isSignaling()) {
          // 32-bit x86 will silence sNaN when loading floats, so we explicitly
          // add the INVALID exception here.
          *స్థితి_3 =
              static_cast<APFloat::op_స్థితి>(*స్థితి_3 | APFloat::opInvalidOp);
        }
      }
      ద్విగుణ ఫలితం_2 = LIBC_NAMESPACE::shared::ఘాతాంకం(x_val);
      తిరిగి APFloat(ఫలితం_2);
    }
  }
  తిరిగి std::nullopt;
}

} // namespace llvm

#నిర్వచనం_రద్దు APFLOAT_DISPATCH_ON_SEMANTICS
