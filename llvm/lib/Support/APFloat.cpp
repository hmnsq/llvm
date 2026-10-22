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

#ਅਨ੍ਤਰ੍ਭਾਵ "llvm/ADT/APFloat.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "llvm/ADT/APSInt.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "llvm/ADT/ArrayRef.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "llvm/ADT/FloatingPointMode.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "llvm/ADT/FoldingSet.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "llvm/ADT/Hashing.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "llvm/ADT/STLExtras.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "llvm/ADT/SmallVector.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "llvm/ADT/StringExtras.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "llvm/ADT/StringRef.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "llvm/ADT/StringSwitch.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "llvm/Config/llvm-config.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "llvm/Support/Debug.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "llvm/Support/Error.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "llvm/Support/MathExtras.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "llvm/Support/raw_ostream.h"
#ਅਨ੍ਤਰ੍ਭਾਵ <cstring>
#ਅਨ੍ਤਰ੍ਭਾਵ <iterator>
#ਅਨ੍ਤਰ੍ਭਾਵ <limits.h>

/// Shared headers from LLVM libc
/// Make sure to add ${LLVM_SOURCE_DIR}/../libc to include directories.
///
/// Notes: So far it looks like APFloat does not check errnos or floating-point
/// exceptions after calling the math functions, so we will configure LLVM libc
/// math functions to skip setting errnos and floating-point exceptions
/// explicitly.  We also put them in a separate namespace so that the symbols
/// do not clash with other libc math builds just in case.
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ LIBC_NAMESPACE __llvm_libc_apfloat
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ LIBC_MATH (LIBC_MATH_NO_ERRNO | LIBC_MATH_NO_EXCEPT)

#ਅਨ੍ਤਰ੍ਭਾਵ "shared/math.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "shared/math_check_exceptions.h"

#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ APFLOAT_DISPATCH_ON_SEMANTICS(ਵਿਧੀ_CALL)                             \
  ਕਰੋ {                                                                         \
    ਜੇ (ਵਰਤਦਾ_Layout<IEEEFloat>(getSemantics()))                                 \
      ਵਾਪਸ U.IEEE.ਵਿਧੀ_CALL;                                               \
    ਜੇ (ਵਰਤਦਾ_Layout<DoubleAPFloat>(getSemantics()))                             \
      ਵਾਪਸ U.Double.ਵਿਧੀ_CALL;                                             \
    llvm_unreachable("Unexpected semantics");                                  \
  } ਜਦੋਂ (ਝੂਠ)

ਵਰਤੋਂ ਨਾਮਾਕਾਸ਼ llvm;

/// A macro used to combine two fcCategory enums into one key which can be used
/// in a switch statement to classify how the interaction of two APFloat's
/// categories affects an operation.
///
/// TODO: If clang source code is ever allowed to use constexpr in its own
/// codebase, change this into a static inline function.
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ Pack_Categories_Into_ਕੁੰਜੀ(_lhs, _rhs) ((_lhs) * 4 + (_rhs))

/* Assumed in hexadecimal significand parsing, and conversion to
   hexadecimal strings.  */
static_assert(APFloat_ਅਧਾਰ::integer_Part_ਚੌੜਾਈ % 4 == 0, "Part width must be divisible by 4!");

ਨਾਮਾਕਾਸ਼ llvm {

constexpr fltSemantics APFloat_ਅਧਾਰ::semIEEEhalf = {15, -14, 11, 16};
constexpr fltSemantics APFloat_ਅਧਾਰ::semBFloat = {127, -126, 8, 16};
constexpr fltSemantics APFloat_ਅਧਾਰ::semIEEEsingle = {127, -126, 24, 32};
constexpr fltSemantics APFloat_ਅਧਾਰ::semIEEEdouble = {1023, -1022, 53, 64};
constexpr fltSemantics APFloat_ਅਧਾਰ::semIEEEquad = {16383, -16382, 113, 128};
constexpr fltSemantics APFloat_ਅਧਾਰ::semX87DoubleExtended = {
    16383,
    -16382,
    64,
    80,
    fltNonfiniteBehavior::IEEE754,
    fltNanEncoding::IEEE,
    ਸੱਚ,
    ਸੱਚ,
    ਸੱਚ,
    ਸੱਚ,
    ਸੱਚ};
constexpr fltSemantics APFloat_ਅਧਾਰ::semPPCDoubleDouble = {-1, 0, 0, 128};
constexpr fltSemantics APFloat_ਅਧਾਰ::semPPCDoubleDoubleLegacy = {
    1023, -1022 + 53, 53 + 53, 128};
constexpr fltSemantics APFloat_ਅਧਾਰ::semFloat8E5M2 = {15, -14, 3, 8};
constexpr fltSemantics APFloat_ਅਧਾਰ::semFloat8E5M2FNUZ = {
    15, -15, 3, 8, fltNonfiniteBehavior::NanOnly, fltNanEncoding::NegativeZero};
constexpr fltSemantics APFloat_ਅਧਾਰ::semFloat8E4M3 = {7, -6, 4, 8};
constexpr fltSemantics APFloat_ਅਧਾਰ::semFloat8E4M3FN = {
    8, -6, 4, 8, fltNonfiniteBehavior::NanOnly, fltNanEncoding::AllOnes};
constexpr fltSemantics APFloat_ਅਧਾਰ::semFloat8E4M3FNUZ = {
    7, -7, 4, 8, fltNonfiniteBehavior::NanOnly, fltNanEncoding::NegativeZero};
constexpr fltSemantics APFloat_ਅਧਾਰ::semFloat8E4M3B11FNUZ = {
    4, -10, 4, 8, fltNonfiniteBehavior::NanOnly, fltNanEncoding::NegativeZero};
constexpr fltSemantics APFloat_ਅਧਾਰ::semFloat8E3M4 = {3, -2, 5, 8};
constexpr fltSemantics APFloat_ਅਧਾਰ::semFloatTF32 = {127, -126, 11, 19};
constexpr fltSemantics APFloat_ਅਧਾਰ::semFloat8E8M0FNU = {
    127,
    -127,
    1,
    8,
    fltNonfiniteBehavior::NanOnly,
    fltNanEncoding::AllOnes,
    ਝੂਠ,
    ਝੂਠ,
    ਝੂਠ,
    ਝੂਠ};

constexpr fltSemantics APFloat_ਅਧਾਰ::semFloat8E5M3FNU = {
    16,
    -14,
    4,
    8,
    fltNonfiniteBehavior::NanOnly,
    fltNanEncoding::AllOnes,
    ਸੱਚ,
    ਝੂਠ,
    ਝੂਠ};

constexpr fltSemantics APFloat_ਅਧਾਰ::semFloat6E3M2FN = {
    4, -2, 3, 6, fltNonfiniteBehavior::FiniteOnly};
constexpr fltSemantics APFloat_ਅਧਾਰ::semFloat6E2M3FN = {
    2, 0, 4, 6, fltNonfiniteBehavior::FiniteOnly};
constexpr fltSemantics APFloat_ਅਧਾਰ::semFloat4E2M1FN = {
    2, 0, 2, 4, fltNonfiniteBehavior::FiniteOnly};
constexpr fltSemantics APFloat_ਅਧਾਰ::semBogus = {0, 0, 0, 0};

ਅਚਲ llvm::fltSemantics &APFloat_ਅਧਾਰ::Enum_ਮੰਜ਼ਿਲ_Semantics(Semantics S) {
  ਚੋਣ (S) {
  ਹਾਲਤ S_IEEEhalf:
    ਵਾਪਸ IEEEhalf();
  ਹਾਲਤ S_BFloat:
    ਵਾਪਸ BFloat();
  ਹਾਲਤ S_IEEEsingle:
    ਵਾਪਸ IEEEsingle();
  ਹਾਲਤ S_IEEEdouble:
    ਵਾਪਸ IEEEdouble();
  ਹਾਲਤ S_IEEEquad:
    ਵਾਪਸ IEEEquad();
  ਹਾਲਤ S_x87DoubleExtended:
    ਵਾਪਸ x87DoubleExtended();
  ਹਾਲਤ S_PPCDoubleDouble:
    ਵਾਪਸ PPCDoubleDouble();
  ਹਾਲਤ S_PPCDoubleDoubleLegacy:
    ਵਾਪਸ PPCDoubleDoubleLegacy();
  ਹਾਲਤ S_Float8E5M2:
    ਵਾਪਸ Float8E5M2();
  ਹਾਲਤ S_Float8E5M2FNUZ:
    ਵਾਪਸ Float8E5M2FNUZ();
  ਹਾਲਤ S_Float8E4M3:
    ਵਾਪਸ Float8E4M3();
  ਹਾਲਤ S_Float8E4M3FN:
    ਵਾਪਸ Float8E4M3FN();
  ਹਾਲਤ S_Float8E4M3FNUZ:
    ਵਾਪਸ Float8E4M3FNUZ();
  ਹਾਲਤ S_Float8E4M3B11FNUZ:
    ਵਾਪਸ Float8E4M3B11FNUZ();
  ਹਾਲਤ S_Float8E3M4:
    ਵਾਪਸ Float8E3M4();
  ਹਾਲਤ S_FloatTF32:
    ਵਾਪਸ FloatTF32();
  ਹਾਲਤ S_Float8E8M0FNU:
    ਵਾਪਸ Float8E8M0FNU();
  ਹਾਲਤ S_Float8E5M3FNU:
    ਵਾਪਸ Float8E5M3FNU();
  ਹਾਲਤ S_Float6E3M2FN:
    ਵਾਪਸ Float6E3M2FN();
  ਹਾਲਤ S_Float6E2M3FN:
    ਵਾਪਸ Float6E2M3FN();
  ਹਾਲਤ S_Float4E2M1FN:
    ਵਾਪਸ Float4E2M1FN();
  }
  llvm_unreachable("Unrecognised floating semantics");
}

APFloat_ਅਧਾਰ::Semantics
APFloat_ਅਧਾਰ::Semantics_ਮੰਜ਼ਿਲ_Enum(ਅਚਲ llvm::fltSemantics &Sem) {
  ਜੇ (&Sem == &llvm::APFloat::IEEEhalf())
    ਵਾਪਸ S_IEEEhalf;
  ਨਹੀਂਤਾਂ ਜੇ (&Sem == &llvm::APFloat::BFloat())
    ਵਾਪਸ S_BFloat;
  ਨਹੀਂਤਾਂ ਜੇ (&Sem == &llvm::APFloat::IEEEsingle())
    ਵਾਪਸ S_IEEEsingle;
  ਨਹੀਂਤਾਂ ਜੇ (&Sem == &llvm::APFloat::IEEEdouble())
    ਵਾਪਸ S_IEEEdouble;
  ਨਹੀਂਤਾਂ ਜੇ (&Sem == &llvm::APFloat::IEEEquad())
    ਵਾਪਸ S_IEEEquad;
  ਨਹੀਂਤਾਂ ਜੇ (&Sem == &llvm::APFloat::x87DoubleExtended())
    ਵਾਪਸ S_x87DoubleExtended;
  ਨਹੀਂਤਾਂ ਜੇ (&Sem == &llvm::APFloat::PPCDoubleDouble())
    ਵਾਪਸ S_PPCDoubleDouble;
  ਨਹੀਂਤਾਂ ਜੇ (&Sem == &llvm::APFloat::PPCDoubleDoubleLegacy())
    ਵਾਪਸ S_PPCDoubleDoubleLegacy;
  ਨਹੀਂਤਾਂ ਜੇ (&Sem == &llvm::APFloat::Float8E5M2())
    ਵਾਪਸ S_Float8E5M2;
  ਨਹੀਂਤਾਂ ਜੇ (&Sem == &llvm::APFloat::Float8E5M2FNUZ())
    ਵਾਪਸ S_Float8E5M2FNUZ;
  ਨਹੀਂਤਾਂ ਜੇ (&Sem == &llvm::APFloat::Float8E4M3())
    ਵਾਪਸ S_Float8E4M3;
  ਨਹੀਂਤਾਂ ਜੇ (&Sem == &llvm::APFloat::Float8E4M3FN())
    ਵਾਪਸ S_Float8E4M3FN;
  ਨਹੀਂਤਾਂ ਜੇ (&Sem == &llvm::APFloat::Float8E4M3FNUZ())
    ਵਾਪਸ S_Float8E4M3FNUZ;
  ਨਹੀਂਤਾਂ ਜੇ (&Sem == &llvm::APFloat::Float8E4M3B11FNUZ())
    ਵਾਪਸ S_Float8E4M3B11FNUZ;
  ਨਹੀਂਤਾਂ ਜੇ (&Sem == &llvm::APFloat::Float8E3M4())
    ਵਾਪਸ S_Float8E3M4;
  ਨਹੀਂਤਾਂ ਜੇ (&Sem == &llvm::APFloat::FloatTF32())
    ਵਾਪਸ S_FloatTF32;
  ਨਹੀਂਤਾਂ ਜੇ (&Sem == &llvm::APFloat::Float8E8M0FNU())
    ਵਾਪਸ S_Float8E8M0FNU;
  ਨਹੀਂਤਾਂ ਜੇ (&Sem == &llvm::APFloat::Float8E5M3FNU())
    ਵਾਪਸ S_Float8E5M3FNU;
  ਨਹੀਂਤਾਂ ਜੇ (&Sem == &llvm::APFloat::Float6E3M2FN())
    ਵਾਪਸ S_Float6E3M2FN;
  ਨਹੀਂਤਾਂ ਜੇ (&Sem == &llvm::APFloat::Float6E2M3FN())
    ਵਾਪਸ S_Float6E2M3FN;
  ਨਹੀਂਤਾਂ ਜੇ (&Sem == &llvm::APFloat::Float4E2M1FN())
    ਵਾਪਸ S_Float4E2M1FN;
  ਨਹੀਂਤਾਂ
    llvm_unreachable("Unknown floating semantics");
}

ਤਾਰਕਿਕ APFloat_ਅਧਾਰ::isRepresentableBy(ਅਚਲ fltSemantics &A,
                                    ਅਚਲ fltSemantics &B) {
  ਵਾਪਸ A.maxExponent <= B.maxExponent && A.minExponent >= B.minExponent &&
         A.precision <= B.precision;
}

ਤਾਰਕਿਕ APFloat_ਅਧਾਰ::is_Losslessly_Convertible_ਮੰਜ਼ਿਲ(ਅਚਲ fltSemantics &From,
                                            ਅਚਲ fltSemantics &Initial_ਮੰਜ਼ਿਲ,
                                            ਤਾਰਕਿਕ IgnoreNaNs) {
  ਜੇ (&From == &Initial_ਮੰਜ਼ਿਲ)
    ਵਾਪਸ ਸੱਚ;

  // PPC double-double cannot be described by a conventional exponent range
  // and precision. In particular, converting it to another semantics drops
  // its low double, so conservatively reject conversions from it.
  ਜੇ (&From == &semPPCDoubleDouble)
    ਵਾਪਸ ਝੂਠ;

  // Converting to PPC double-double is lossless exactly when the value fits
  // in its high double. In other words, the conversion is lossless if the
  // conversion to IEEEdouble is lossless.
  ਅਚਲ fltSemantics &ਮੰਜ਼ਿਲ =
      &Initial_ਮੰਜ਼ਿਲ == &semPPCDoubleDouble ? semIEEEdouble : Initial_ਮੰਜ਼ਿਲ;

  ਜੇ (!isRepresentableBy(From, ਮੰਜ਼ਿਲ))
    ਵਾਪਸ ਝੂਠ;

  ਜੇ ((From.hasZero && !ਮੰਜ਼ਿਲ.hasZero) ||
      (From.hasSignedRepr && !ਮੰਜ਼ਿਲ.hasSignedRepr))
    ਵਾਪਸ ਝੂਠ;

  // NegativeZero NaN encoding repurposes the negative-zero bit pattern, so a
  // conversion to such a format cannot preserve a source negative zero.
  ਤਾਰਕਿਕ FromHasSignedZero = From.hasZero && From.hasSignedRepr &&
                           From.nanEncoding != fltNanEncoding::NegativeZero;
  ਤਾਰਕਿਕ ਮੰਜ਼ਿਲ_Has_Signed_Zero = ਮੰਜ਼ਿਲ.hasZero && ਮੰਜ਼ਿਲ.hasSignedRepr &&
                         ਮੰਜ਼ਿਲ.nanEncoding != fltNanEncoding::NegativeZero;
  ਜੇ (FromHasSignedZero && !ਮੰਜ਼ਿਲ_Has_Signed_Zero)
    ਵਾਪਸ ਝੂਠ;

  // isRepresentableBy compares normalized exponent ranges. Also ensure that
  // the smallest source value, which may be denormal, is represented exactly
  // by the destination semantics.
  APFloat ਸਭ_ਤੋਂ_ਛੋਟਾ_From = APFloat::get_ਸਭ_ਤੋਂ_ਛੋਟਾ(From);
  ਤਾਰਕਿਕ LosesInfo = ਝੂਠ;
  (ਖਾਲੀ)ਸਭ_ਤੋਂ_ਛੋਟਾ_From.convert(ਮੰਜ਼ਿਲ, APFloat::rm_ਸਭ_ਤੋਂ_ਨੇੜੇ_Ties_ਮੰਜ਼ਿਲ_Even, &LosesInfo);
  ਜੇ (LosesInfo)
    ਵਾਪਸ ਝੂਠ;

  ਜੇ (From.nonFiniteBehavior == fltNonfiniteBehavior::FiniteOnly)
    ਵਾਪਸ ਸੱਚ;

  // Even when NaN representations can be ignored, NaNs must remain NaNs and
  // infinities must remain infinities. Otherwise the conversion can change
  // whether an operation with nnan has poison-producing operands.
  ਜੇ (IgnoreNaNs) {
    ਜੇ (From.nonFiniteBehavior == fltNonfiniteBehavior::IEEE754)
      ਵਾਪਸ ਮੰਜ਼ਿਲ.nonFiniteBehavior == fltNonfiniteBehavior::IEEE754;
    ਵਾਪਸ ਮੰਜ਼ਿਲ.nonFiniteBehavior != fltNonfiniteBehavior::FiniteOnly;
  }

  ਜੇ (From.nonFiniteBehavior == fltNonfiniteBehavior::IEEE754) {
    // Converting an IEEE signaling NaN to another semantics quiets it, so the
    // original value cannot be recovered by converting it back.
    ਵਾਪਸ ਝੂਠ;
  }

  // NanOnly formats have no signaling NaNs. IEEE semantics can represent
  // their quiet NaNs; conversions between NanOnly formats are conservatively
  // accepted only when they use the same NaN encoding.
  ਜੇ (ਮੰਜ਼ਿਲ.nonFiniteBehavior == fltNonfiniteBehavior::IEEE754)
    ਵਾਪਸ ਸੱਚ;
  ਵਾਪਸ ਮੰਜ਼ਿਲ.nonFiniteBehavior == fltNonfiniteBehavior::NanOnly &&
         From.nanEncoding == ਮੰਜ਼ਿਲ.nanEncoding;
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
[[maybe_ਅਣਵਰਤਿਆ]] constexpr ਬਿਨਾਨਿਸ਼ਾਨੀ maxExponent = 16383;

// Decimal exponents in the f16 and f32 ranges need at most 2 and 6 parts.
// Keep 16 parts (128 bytes) inline: enough for 5^413, but not a large stack
// frame. Longer fractional spellings use heap storage.
ਸਥਿਰ constexpr ਬਿਨਾਨਿਸ਼ਾਨੀ ਅੰਕ_Pow5_Parts_On_ਢੇਰ = 16;

ਸਥਿਰ ਬਿਨਾਨਿਸ਼ਾਨੀ ਪੂਰਨ power_ਦਾ_Five_Parts_For_Exponent(ਬਿਨਾਨਿਸ਼ਾਨੀ ਪੂਰਨ power) {
  ਪ੍ਰਤਿਜ੍ਞਾ(power <= maxExponent);

  // 815 / 351 is an upper bound on log2(5).  Reserve an additional part for
  // tcFullMultiply, which writes the full product width even when its most
  // significant part is zero.
  ਵਾਪਸ 2 + ((power * 815) / (351 * APFloat_ਅਧਾਰ::integer_Part_ਚੌੜਾਈ));
}

ਬਿਨਾਨਿਸ਼ਾਨੀ ਪੂਰਨ APFloat_ਅਧਾਰ::semanticsPrecision(ਅਚਲ fltSemantics &semantics) {
  ਵਾਪਸ semantics.precision;
}
APFloat_ਅਧਾਰ::ExponentType
APFloat_ਅਧਾਰ::semanticsMaxExponent(ਅਚਲ fltSemantics &semantics) {
  ਵਾਪਸ semantics.maxExponent;
}
APFloat_ਅਧਾਰ::ExponentType
APFloat_ਅਧਾਰ::semanticsMinExponent(ਅਚਲ fltSemantics &semantics) {
  ਵਾਪਸ semantics.minExponent;
}
ਬਿਨਾਨਿਸ਼ਾਨੀ ਪੂਰਨ APFloat_ਅਧਾਰ::semantics_ਮਾਪ_ਵਿੱਚ_ਬਿੱਟ(ਅਚਲ fltSemantics &semantics) {
  ਵਾਪਸ semantics.ਮਾਪ_ਵਿੱਚ_ਬਿੱਟ;
}
ਬਿਨਾਨਿਸ਼ਾਨੀ ਪੂਰਨ APFloat_ਅਧਾਰ::semantics_Int_ਮਾਪ_ਵਿੱਚ_ਬਿੱਟ(ਅਚਲ fltSemantics &semantics,
                                                 ਤਾਰਕਿਕ isSigned) {
  // The max FP value is pow(2, MaxExponent) * (1 + MaxFraction), so we need
  // at least one more bit than the MaxExponent to hold the max FP value.
  ਬਿਨਾਨਿਸ਼ਾਨੀ ਪੂਰਨ Min_ਦੋਆਧਾਰੀ_ਅੰਕ_ਚੌੜਾਈ = semanticsMaxExponent(semantics) + 1;
  // Extra sign bit needed.
  ਜੇ (isSigned)
    ++Min_ਦੋਆਧਾਰੀ_ਅੰਕ_ਚੌੜਾਈ;
  ਵਾਪਸ Min_ਦੋਆਧਾਰੀ_ਅੰਕ_ਚੌੜਾਈ;
}

ਤਾਰਕਿਕ APFloat_ਅਧਾਰ::semanticsHasZero(ਅਚਲ fltSemantics &semantics) {
  ਵਾਪਸ semantics.hasZero;
}

ਤਾਰਕਿਕ APFloat_ਅਧਾਰ::semanticsHasSignedRepr(ਅਚਲ fltSemantics &semantics) {
  ਵਾਪਸ semantics.hasSignedRepr;
}

ਤਾਰਕਿਕ APFloat_ਅਧਾਰ::semanticsHasInf(ਅਚਲ fltSemantics &semantics) {
  ਵਾਪਸ semantics.nonFiniteBehavior == fltNonfiniteBehavior::IEEE754;
}

ਤਾਰਕਿਕ APFloat_ਅਧਾਰ::semanticsHasNaN(ਅਚਲ fltSemantics &semantics) {
  ਵਾਪਸ semantics.nonFiniteBehavior != fltNonfiniteBehavior::FiniteOnly;
}

ਤਾਰਕਿਕ APFloat_ਅਧਾਰ::isIEEELikeFP(ਅਚਲ fltSemantics &semantics) {
  // Keep in sync with Type::isIEEELikeFPTy
  ਵਾਪਸ Semantics_ਮੰਜ਼ਿਲ_Enum(semantics) <= S_IEEEquad;
}

ਤਾਰਕਿਕ APFloat_ਅਧਾਰ::has_Sign_ਦੋਆਧਾਰੀ_ਅੰਕ_ਵਿੱਚ_MSB(ਅਚਲ fltSemantics &semantics) {
  ਵਾਪਸ semantics.has_Sign_ਦੋਆਧਾਰੀ_ਅੰਕ_ਵਿੱਚ_MSB;
}

ਤਾਰਕਿਕ APFloat_ਅਧਾਰ::is_Representable_As_ਸਾਧਾਰਨ_ਵਿੱਚ(ਅਚਲ fltSemantics &Src,
                                            ਅਚਲ fltSemantics &Dst) {
  // Exponent range must be larger.
  ਜੇ (Src.maxExponent >= Dst.maxExponent || Src.minExponent <= Dst.minExponent)
    ਵਾਪਸ ਝੂਠ;

  // If the mantissa is long enough, the result value could still be denormal
  // with a larger exponent range.
  //
  // FIXME: This condition is probably not accurate but also shouldn't be a
  // practical concern with existing types.
  ਵਾਪਸ Dst.precision >= Src.precision;
}

ਬਿਨਾਨਿਸ਼ਾਨੀ APFloat_ਅਧਾਰ::get_ਮਾਪ_ਵਿੱਚ_ਬਿੱਟ(ਅਚਲ fltSemantics &Sem) {
  ਵਾਪਸ Sem.ਮਾਪ_ਵਿੱਚ_ਬਿੱਟ;
}

ਸਥਿਰ constexpr APFloat_ਅਧਾਰ::ExponentType
exponentZero(ਅਚਲ fltSemantics &semantics) {
  ਵਾਪਸ semantics.minExponent - 1;
}

ਸਥਿਰ constexpr APFloat_ਅਧਾਰ::ExponentType
exponentInf(ਅਚਲ fltSemantics &semantics) {
  ਵਾਪਸ semantics.maxExponent + 1;
}

ਸਥਿਰ constexpr APFloat_ਅਧਾਰ::ExponentType
exponentNaN(ਅਚਲ fltSemantics &semantics) {
  ਜੇ (semantics.nonFiniteBehavior == fltNonfiniteBehavior::NanOnly) {
    ਜੇ (semantics.nanEncoding == fltNanEncoding::NegativeZero)
      ਵਾਪਸ exponentZero(semantics);
    ਜੇ (semantics.hasSignedRepr || semantics.precision > 1)
      ਵਾਪਸ semantics.maxExponent;
  }
  ਵਾਪਸ semantics.maxExponent + 1;
}

/* A bunch of private, handy routines.  */

ਸਥਿਰ ਸਤਰਅੰਦਰ Error createError(ਅਚਲ Twine &Err) {
  ਵਾਪਸ make_error<StringError>(Err, inconvertible_Error_ਕੋਡ());
}

ਸਥਿਰ constexpr ਸਤਰਅੰਦਰ ਬਿਨਾਨਿਸ਼ਾਨੀ ਪੂਰਨ part_ਗਿਣਤੀ_For_ਬਿੱਟ(ਬਿਨਾਨਿਸ਼ਾਨੀ ਪੂਰਨ ਬਿੱਟ) {
  ਵਾਪਸ std::max(1u, (ਬਿੱਟ + APFloat_ਅਧਾਰ::integer_Part_ਚੌੜਾਈ - 1) /
                          APFloat_ਅਧਾਰ::integer_Part_ਚੌੜਾਈ);
}

/* Returns 0U-9U.  Return values >= 10U are not digits.  */
ਸਥਿਰ ਸਤਰਅੰਦਰ ਬਿਨਾਨਿਸ਼ਾਨੀ ਪੂਰਨ
dec_Digit_ਮੁੱਲ(ਬਿਨਾਨਿਸ਼ਾਨੀ ਪੂਰਨ c)
{
  ਵਾਪਸ c - '0';
}

/* Return the value of a decimal exponent of the form
   [+-]ddddddd.

   If the exponent overflows, returns a large exponent with the
   appropriate sign.  */
ਸਥਿਰ Expected<ਪੂਰਨ> readExponent(StringRef::iterator begin,
                                  StringRef::iterator ਸਿਰਾ) {
  ਅਚਲ ਬਿਨਾਨਿਸ਼ਾਨੀ ਪੂਰਨ overlargeExponent = 24000;  /* FIXME.  */
  StringRef::iterator p = begin;

  // Treat no exponent as 0 to match binutils
  ਜੇ (p == ਸਿਰਾ || ((*p == '-' || *p == '+') && (p + 1) == ਸਿਰਾ))
    ਵਾਪਸ 0;

  ਤਾਰਕਿਕ isNegative = *p == '-';
  ਜੇ (*p == '-' || *p == '+') {
    p++;
    ਜੇ (p == ਸਿਰਾ)
      ਵਾਪਸ createError("Exponent has no digits");
  }

  ਬਿਨਾਨਿਸ਼ਾਨੀ absExponent = dec_Digit_ਮੁੱਲ(*p++);
  ਜੇ (absExponent >= 10U)
    ਵਾਪਸ createError("Invalid character in exponent");

  ਲਈ (; p != ਸਿਰਾ; ++p) {
    ਬਿਨਾਨਿਸ਼ਾਨੀ ਮੁੱਲ_2 = dec_Digit_ਮੁੱਲ(*p);
    ਜੇ (ਮੁੱਲ_2 >= 10U)
      ਵਾਪਸ createError("Invalid character in exponent");

    absExponent = absExponent * 10U + ਮੁੱਲ_2;
    ਜੇ (absExponent >= overlargeExponent) {
      absExponent = overlargeExponent;
      ਰੋਕੋ;
    }
  }

  ਜੇ (isNegative)
    ਵਾਪਸ -(ਪੂਰਨ) absExponent;
  ਨਹੀਂਤਾਂ
    ਵਾਪਸ (ਪੂਰਨ) absExponent;
}

/* This is ugly and needs cleaning up, but I don't immediately see
   how whilst remaining safe.  */
ਸਥਿਰ Expected<ਪੂਰਨ> ਕੁੱਲ_Exponent(StringRef::iterator p,
                                   StringRef::iterator ਸਿਰਾ,
                                   ਪੂਰਨ exponentAdjustment) {
  ਪੂਰਨ exponent = 0;

  ਜੇ (p == ਸਿਰਾ)
    ਵਾਪਸ createError("Exponent has no digits");

  ਤਾਰਕਿਕ negative = *p == '-';
  ਜੇ (*p == '-' || *p == '+') {
    p++;
    ਜੇ (p == ਸਿਰਾ)
      ਵਾਪਸ createError("Exponent has no digits");
  }

  ਪੂਰਨ unsignedExponent = 0;
  ਤਾਰਕਿਕ ਉਛਾਲ_2 = ਝੂਠ;
  ਲਈ (; p != ਸਿਰਾ; ++p) {
    ਬਿਨਾਨਿਸ਼ਾਨੀ ਪੂਰਨ ਮੁੱਲ_2;

    ਮੁੱਲ_2 = dec_Digit_ਮੁੱਲ(*p);
    ਜੇ (ਮੁੱਲ_2 >= 10U)
      ਵਾਪਸ createError("Invalid character in exponent");

    unsignedExponent = unsignedExponent * 10 + ਮੁੱਲ_2;
    ਜੇ (unsignedExponent > 32767) {
      ਉਛਾਲ_2 = ਸੱਚ;
      ਰੋਕੋ;
    }
  }

  ਜੇ (exponentAdjustment > 32767 || exponentAdjustment < -32768)
    ਉਛਾਲ_2 = ਸੱਚ;

  ਜੇ (!ਉਛਾਲ_2) {
    exponent = unsignedExponent;
    ਜੇ (negative)
      exponent = -exponent;
    exponent += exponentAdjustment;
    ਜੇ (exponent > 32767 || exponent < -32768)
      ਉਛਾਲ_2 = ਸੱਚ;
  }

  ਜੇ (ਉਛਾਲ_2)
    exponent = negative ? -32768: 32767;

  ਵਾਪਸ exponent;
}

ਸਥਿਰ Expected<StringRef::iterator>
skipLeadingZeroesAndAnyDot(StringRef::iterator begin, StringRef::iterator ਸਿਰਾ,
                           StringRef::iterator *dot) {
  StringRef::iterator p = begin;
  *dot = ਸਿਰਾ;
  ਜਦੋਂ (p != ਸਿਰਾ && *p == '0')
    p++;

  ਜੇ (p != ਸਿਰਾ && *p == '.') {
    *dot = p++;

    ਜੇ (ਸਿਰਾ - begin == 1)
      ਵਾਪਸ createError("Significand has no digits");

    ਜਦੋਂ (p != ਸਿਰਾ && *p == '0')
      p++;
  }

  ਵਾਪਸ p;
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
ਬਣਤਰ decimalInfo {
  ਅਚਲ ਅੱਖਰ *ਪਹਿਲਾ_Sig_Digit;
  ਅਚਲ ਅੱਖਰ *ਆਖਰੀ_Sig_Digit;
  ਪੂਰਨ exponent;
  ਪੂਰਨ normalizedExponent;
};

ਸਥਿਰ Error interpretDecimal(StringRef::iterator begin,
                              StringRef::iterator ਸਿਰਾ, decimalInfo *D) {
  StringRef::iterator dot = ਸਿਰਾ;

  ਸਵੈਚਾਲਿਤ PtrOrErr = skipLeadingZeroesAndAnyDot(begin, ਸਿਰਾ, &dot);
  ਜੇ (!PtrOrErr)
    ਵਾਪਸ PtrOrErr.takeError();
  StringRef::iterator p = *PtrOrErr;

  D->ਪਹਿਲਾ_Sig_Digit = p;
  D->exponent = 0;
  D->normalizedExponent = 0;

  ਲਈ (; p != ਸਿਰਾ; ++p) {
    ਜੇ (*p == '.') {
      ਜੇ (dot != ਸਿਰਾ)
        ਵਾਪਸ createError("String contains multiple dots");
      dot = p++;
      ਜੇ (p == ਸਿਰਾ)
        ਰੋਕੋ;
    }
    ਜੇ (dec_Digit_ਮੁੱਲ(*p) >= 10U)
      ਰੋਕੋ;
  }

  ਜੇ (p != ਸਿਰਾ) {
    ਜੇ (*p != 'e' && *p != 'E')
      ਵਾਪਸ createError("Invalid character in significand");
    ਜੇ (p == begin)
      ਵਾਪਸ createError("Significand has no digits");
    ਜੇ (dot != ਸਿਰਾ && p - begin == 1)
      ਵਾਪਸ createError("Significand has no digits");

    /* p points to the first non-digit in the string */
    ਸਵੈਚਾਲਿਤ ExpOrErr = readExponent(p + 1, ਸਿਰਾ);
    ਜੇ (!ExpOrErr)
      ਵਾਪਸ ExpOrErr.takeError();
    D->exponent = *ExpOrErr;

    /* Implied decimal point?  */
    ਜੇ (dot == ਸਿਰਾ)
      dot = p;
  }

  /* If number is all zeroes accept any exponent.  */
  ਜੇ (p != D->ਪਹਿਲਾ_Sig_Digit) {
    /* Drop insignificant trailing zeroes.  */
    ਜੇ (p != begin) {
      ਕਰੋ
        ਕਰੋ
          p--;
        ਜਦੋਂ (p != begin && *p == '0');
      ਜਦੋਂ (p != begin && *p == '.');
    }

    /* Adjust the exponents for any decimal point.  */
    D->exponent += static_cast<APFloat::ExponentType>((dot - p) - (dot > p));
    D->normalizedExponent = (D->exponent +
              static_cast<APFloat::ExponentType>((p - D->ਪਹਿਲਾ_Sig_Digit)
                                      - (dot > D->ਪਹਿਲਾ_Sig_Digit && dot < p)));
  }

  D->ਆਖਰੀ_Sig_Digit = p;
  ਵਾਪਸ Error::success();
}

/* Return the trailing fraction of a hexadecimal number.
   DIGITVALUE is the first hex digit of the fraction, P points to
   the next digit.  */
ਸਥਿਰ Expected<ਗੁਆਚਾ_Fraction>
trailingHexadecimalFraction(StringRef::iterator p, StringRef::iterator ਸਿਰਾ,
                            ਬਿਨਾਨਿਸ਼ਾਨੀ ਪੂਰਨ digit_ਮੁੱਲ) {
  /* If the first trailing digit isn't 0 or 8 we can work out the
     fraction immediately.  */
  ਜੇ (digit_ਮੁੱਲ > 8)
    ਵਾਪਸ lf_ਹੋਰ_Than_Half;
  ਨਹੀਂਤਾਂ ਜੇ (digit_ਮੁੱਲ < 8 && digit_ਮੁੱਲ > 0)
    ਵਾਪਸ lfLessThanHalf;

  // Otherwise we need to find the first non-zero digit.
  ਜਦੋਂ (p != ਸਿਰਾ && (*p == '0' || *p == '.'))
    p++;

  ਜੇ (p == ਸਿਰਾ)
    ਵਾਪਸ createError("Invalid trailing hexadecimal fraction!");

  ਬਿਨਾਨਿਸ਼ਾਨੀ hexDigit = hex_Digit_ਮੁੱਲ(*p);

  /* If we ran off the end it is exactly zero or one-half, otherwise
     a little more.  */
  ਜੇ (hexDigit == UINT_MAX)
    ਵਾਪਸ digit_ਮੁੱਲ == 0 ? lfExactlyZero: lfExactlyHalf;
  ਨਹੀਂਤਾਂ
    ਵਾਪਸ digit_ਮੁੱਲ == 0 ? lfLessThanHalf: lf_ਹੋਰ_Than_Half;
}

/* Return the fraction lost were a bignum truncated losing the least
   significant BITS bits.  */
ਸਥਿਰ ਗੁਆਚਾ_Fraction
ਗੁਆਚਾ_Fraction_Through_Truncation(ਅਚਲ APFloat_ਅਧਾਰ::integerPart *parts,
                              ਬਿਨਾਨਿਸ਼ਾਨੀ ਪੂਰਨ part_ਗਿਣਤੀ,
                              ਬਿਨਾਨਿਸ਼ਾਨੀ ਪੂਰਨ ਬਿੱਟ)
{
  ਬਿਨਾਨਿਸ਼ਾਨੀ lsb = APInt::tcLSB(parts, part_ਗਿਣਤੀ);

  /* Note this is guaranteed true if bits == 0, or LSB == UINT_MAX.  */
  ਜੇ (ਬਿੱਟ <= lsb)
    ਵਾਪਸ lfExactlyZero;
  ਜੇ (ਬਿੱਟ == lsb + 1)
    ਵਾਪਸ lfExactlyHalf;
  ਜੇ (ਬਿੱਟ <= part_ਗਿਣਤੀ * APFloat_ਅਧਾਰ::integer_Part_ਚੌੜਾਈ &&
      APInt::tc_ਕੱਢੋ_ਦੋਆਧਾਰੀ_ਅੰਕ(parts, ਬਿੱਟ - 1))
    ਵਾਪਸ lf_ਹੋਰ_Than_Half;

  ਵਾਪਸ lfLessThanHalf;
}

/* Shift DST right BITS bits noting lost fraction.  */
ਸਥਿਰ ਗੁਆਚਾ_Fraction
ਖਿਸਕਾਓ_ਸੱਜਾ(APFloat_ਅਧਾਰ::integerPart *dst, ਬਿਨਾਨਿਸ਼ਾਨੀ ਪੂਰਨ parts, ਬਿਨਾਨਿਸ਼ਾਨੀ ਪੂਰਨ ਬਿੱਟ)
{
  ਗੁਆਚਾ_Fraction ਗੁਆਚਾ_fraction = ਗੁਆਚਾ_Fraction_Through_Truncation(dst, parts, ਬਿੱਟ);

  APInt::tc_ਖਿਸਕਾਓ_ਸੱਜਾ(dst, parts, ਬਿੱਟ);

  ਵਾਪਸ ਗੁਆਚਾ_fraction;
}

/* Combine the effect of two lost fractions.  */
ਸਥਿਰ ਗੁਆਚਾ_Fraction
combine_ਗੁਆਚਾ_Fractions(ਗੁਆਚਾ_Fraction ਹੋਰ_Significant,
                     ਗੁਆਚਾ_Fraction lessSignificant)
{
  ਜੇ (lessSignificant != lfExactlyZero) {
    ਜੇ (ਹੋਰ_Significant == lfExactlyZero)
      ਹੋਰ_Significant = lfLessThanHalf;
    ਨਹੀਂਤਾਂ ਜੇ (ਹੋਰ_Significant == lfExactlyHalf)
      ਹੋਰ_Significant = lf_ਹੋਰ_Than_Half;
  }

  ਵਾਪਸ ਹੋਰ_Significant;
}

/* The error from the true value, in half-ulps, on multiplying two
   floating point numbers, which differ from the value they
   approximate by at most HUE1 and HUE2 half-ulps, is strictly less
   than the returned value.

   See "How to Read Floating Point Numbers Accurately" by William D
   Clinger.  */
ਸਥਿਰ ਬਿਨਾਨਿਸ਼ਾਨੀ ਪੂਰਨ
HUerrBound(ਤਾਰਕਿਕ inexactMultiply, ਬਿਨਾਨਿਸ਼ਾਨੀ ਪੂਰਨ HUerr1, ਬਿਨਾਨਿਸ਼ਾਨੀ ਪੂਰਨ HUerr2)
{
  ਪ੍ਰਤਿਜ੍ਞਾ(HUerr1 < 2 || HUerr2 < 2 || (HUerr1 + HUerr2 < 8));

  ਜੇ (HUerr1 + HUerr2 == 0)
    ਵਾਪਸ inexactMultiply * 2;  /* <= inexactMultiply half-ulps.  */
  ਨਹੀਂਤਾਂ
    ਵਾਪਸ inexactMultiply + 2 * (HUerr1 + HUerr2);
}

/* The number of ulps from the boundary (zero, or half if ISNEAREST)
   when the least significant BITS are truncated.  BITS cannot be
   zero.  */
ਸਥਿਰ APFloat_ਅਧਾਰ::integerPart
ulpsFromBoundary(ਅਚਲ APFloat_ਅਧਾਰ::integerPart *parts, ਬਿਨਾਨਿਸ਼ਾਨੀ ਪੂਰਨ ਬਿੱਟ,
                 ਤਾਰਕਿਕ is_ਸਭ_ਤੋਂ_ਨੇੜੇ) {
  ਪ੍ਰਤਿਜ੍ਞਾ(ਬਿੱਟ != 0);

  ਬਿੱਟ--;
  ਬਿਨਾਨਿਸ਼ਾਨੀ ਗਿਣਤੀ_2 = ਬਿੱਟ / APFloat_ਅਧਾਰ::integer_Part_ਚੌੜਾਈ;
  ਬਿਨਾਨਿਸ਼ਾਨੀ part_ਬਿੱਟ = ਬਿੱਟ % APFloat_ਅਧਾਰ::integer_Part_ਚੌੜਾਈ + 1;

  APFloat_ਅਧਾਰ::integerPart part =
      parts[ਗਿਣਤੀ_2] & (~(APFloat_ਅਧਾਰ::integerPart)0 >>
                      (APFloat_ਅਧਾਰ::integer_Part_ਚੌੜਾਈ - part_ਬਿੱਟ));

  APFloat_ਅਧਾਰ::integerPart boundary;
  ਜੇ (is_ਸਭ_ਤੋਂ_ਨੇੜੇ)
    boundary = (APFloat_ਅਧਾਰ::integerPart) 1 << (part_ਬਿੱਟ - 1);
  ਨਹੀਂਤਾਂ
    boundary = 0;

  ਜੇ (ਗਿਣਤੀ_2 == 0) {
    ਜੇ (part - boundary <= boundary - part)
      ਵਾਪਸ part - boundary;
    ਨਹੀਂਤਾਂ
      ਵਾਪਸ boundary - part;
  }

  ਜੇ (part == boundary) {
    ਜਦੋਂ (--ਗਿਣਤੀ_2)
      ਜੇ (parts[ਗਿਣਤੀ_2])
        ਵਾਪਸ ~(APFloat_ਅਧਾਰ::integerPart) 0; /* A lot.  */

    ਵਾਪਸ parts[0];
  } ਨਹੀਂਤਾਂ ਜੇ (part == boundary - 1) {
    ਜਦੋਂ (--ਗਿਣਤੀ_2)
      ਜੇ (~parts[ਗਿਣਤੀ_2])
        ਵਾਪਸ ~(APFloat_ਅਧਾਰ::integerPart) 0; /* A lot.  */

    ਵਾਪਸ -parts[0];
  }

  ਵਾਪਸ ~(APFloat_ਅਧਾਰ::integerPart) 0; /* A lot.  */
}

ਸਥਿਰ constexpr ਬਿਨਾਨਿਸ਼ਾਨੀ Power_ਦਾ_Five_Part_Counts[] = {1,  1,  2,  3,   5,  10,
                                                     19, 38, 75, 149, 298};

ਸਥਿਰ constexpr APFloat_ਅਧਾਰ::integerPart Power_ਦਾ_Five_Parts[] = {
#ਅਨ੍ਤਰ੍ਭਾਵ "APFloatPowerOfFiveTable.inc"
};

static_assert([] {
  ਬਿਨਾਨਿਸ਼ਾਨੀ ਜੋੜ = 0;
  ਲਈ (ਬਿਨਾਨਿਸ਼ਾਨੀ ਗਿਣਤੀ_3 : Power_ਦਾ_Five_Part_Counts)
    ਜੋੜ += ਗਿਣਤੀ_3;
  ਵਾਪਸ ਜੋੜ;
}() == std::ਮਾਪ_2(Power_ਦਾ_Five_Parts));

/* Place pow(5, power) in DST, and return the number of parts used.
   DST must be at least one part larger than size of the answer.  */
ਸਥਿਰ ਬਿਨਾਨਿਸ਼ਾਨੀ ਪੂਰਨ
powerOf5(APFloat_ਅਧਾਰ::integerPart *dst, ਬਿਨਾਨਿਸ਼ਾਨੀ ਪੂਰਨ power) {
  ਸਥਿਰ ਅਚਲ APFloat_ਅਧਾਰ::integerPart ਪਹਿਲਾ_Eight_Powers[] = {
      1, 5, 25, 125, 625, 3125, 15625, 78125};

  Small_ਸਦਿਸ਼<APFloat_ਅਧਾਰ::integerPart, ਅੰਕ_Pow5_Parts_On_ਢੇਰ> scratch;
  scratch.resize_for_overwrite(power_ਦਾ_Five_Parts_For_Exponent(power));

  APFloat_ਅਧਾਰ::integerPart *p1, *p2;
  ਅਚਲ APFloat_ਅਧਾਰ::integerPart *pow5;

  p1 = dst;
  p2 = scratch.ਅੰਕੜੇ();

  *p1 = ਪਹਿਲਾ_Eight_Powers[power & 7];
  power >>= 3;

  ਬਿਨਾਨਿਸ਼ਾਨੀ ਨਤੀਜਾ_2 = 1;
  pow5 = Power_ਦਾ_Five_Parts;

  ਲਈ (ਬਿਨਾਨਿਸ਼ਾਨੀ ਪੂਰਨ n = 0; power; power >>= 1, n++) {
    ਬਿਨਾਨਿਸ਼ਾਨੀ parts_ਗਿਣਤੀ = Power_ਦਾ_Five_Part_Counts[n];

    ਜੇ (power & 1) {
      APFloat_ਅਧਾਰ::integerPart *tmp;

      APInt::tc_ਭਰਿਆ_Multiply(p2, p1, pow5, ਨਤੀਜਾ_2, parts_ਗਿਣਤੀ);
      ਨਤੀਜਾ_2 += parts_ਗਿਣਤੀ;
      ਜੇ (p2[ਨਤੀਜਾ_2 - 1] == 0)
        ਨਤੀਜਾ_2--;

      /* Now result is in p1 with partsCount parts and p2 is scratch
         space.  */
      tmp = p1;
      p1 = p2;
      p2 = tmp;
    }

    pow5 += parts_ਗਿਣਤੀ;
  }

  ਜੇ (p1 != dst)
    APInt::tc_ਸੌਂਪੋ(dst, p1, ਨਤੀਜਾ_2);

  ਵਾਪਸ ਨਤੀਜਾ_2;
}

/* Zero at the end to avoid modular arithmetic when adding one; used
   when rounding up during hexadecimal output.  */
ਸਥਿਰ ਅਚਲ ਅੱਖਰ hex_Digits_ਹੇਠਲਾ[] = "0123456789abcdef0";
ਸਥਿਰ ਅਚਲ ਅੱਖਰ hex_Digits_ਉੱਤਲਾ[] = "0123456789ABCDEF0";
ਸਥਿਰ ਅਚਲ ਅੱਖਰ infinityL[] = "infinity";
ਸਥਿਰ ਅਚਲ ਅੱਖਰ infinityU[] = "INFINITY";
ਸਥਿਰ ਅਚਲ ਅੱਖਰ NaNL[] = "nan";
ਸਥਿਰ ਅਚਲ ਅੱਖਰ NaNU[] = "NAN";

/* Write out an integerPart in hexadecimal, starting with the most
   significant nibble.  Write out exactly COUNT hexdigits, return
   COUNT.  */
ਸਥਿਰ ਬਿਨਾਨਿਸ਼ਾਨੀ ਪੂਰਨ
partAsHex (ਅੱਖਰ *dst, APFloat_ਅਧਾਰ::integerPart part, ਬਿਨਾਨਿਸ਼ਾਨੀ ਪੂਰਨ ਗਿਣਤੀ_2,
           ਅਚਲ ਅੱਖਰ *hexDigitChars)
{
  ਬਿਨਾਨਿਸ਼ਾਨੀ ਪੂਰਨ ਨਤੀਜਾ_2 = ਗਿਣਤੀ_2;

  ਪ੍ਰਤਿਜ੍ਞਾ(ਗਿਣਤੀ_2 != 0 && ਗਿਣਤੀ_2 <= APFloat_ਅਧਾਰ::integer_Part_ਚੌੜਾਈ / 4);

  part >>= (APFloat_ਅਧਾਰ::integer_Part_ਚੌੜਾਈ - 4 * ਗਿਣਤੀ_2);
  ਜਦੋਂ (ਗਿਣਤੀ_2--) {
    dst[ਗਿਣਤੀ_2] = hexDigitChars[part & 0xf];
    part >>= 4;
  }

  ਵਾਪਸ ਨਤੀਜਾ_2;
}

/* Write out an unsigned decimal integer.  */
ਸਥਿਰ ਅੱਖਰ *writeUnsignedDecimal(ਅੱਖਰ *dst, ਬਿਨਾਨਿਸ਼ਾਨੀ ਪੂਰਨ n) {
  ਅੱਖਰ buff[40], *p;

  p = buff;
  ਕਰੋ
    *p++ = '0' + n % 10;
  ਜਦੋਂ (n /= 10);

  ਕਰੋ
    *dst++ = *--p;
  ਜਦੋਂ (p != buff);

  ਵਾਪਸ dst;
}

/* Write out a signed decimal integer.  */
ਸਥਿਰ ਅੱਖਰ *writeSignedDecimal(ਅੱਖਰ *dst, ਪੂਰਨ ਮੁੱਲ_2) {
  ਜੇ (ਮੁੱਲ_2 < 0) {
    *dst++ = '-';
    dst = writeUnsignedDecimal(dst, -(ਬਿਨਾਨਿਸ਼ਾਨੀ) ਮੁੱਲ_2);
  } ਨਹੀਂਤਾਂ {
    dst = writeUnsignedDecimal(dst, ਮੁੱਲ_2);
  }

  ਵਾਪਸ dst;
}

// Compute the ULP of the input using a definition from:
// Jean-Michel Muller. On the definition of ulp(x). [Research Report] RR-5504,
// LIP RR-2005-09, INRIA, LIP. 2005, pp.16. inria-00070503
ਸਥਿਰ APFloat harrisonUlp(ਅਚਲ APFloat &X) {
  ਅਚਲ fltSemantics &Sem = X.getSemantics();
  ਚੋਣ (X.getCategory()) {
  ਹਾਲਤ APFloat::fcNaN:
    ਵਾਪਸ APFloat::getQNaN(Sem);
  ਹਾਲਤ APFloat::fcInfinity:
    ਵਾਪਸ APFloat::getInf(Sem);
  ਹਾਲਤ APFloat::fcZero:
    ਵਾਪਸ APFloat::get_ਸਭ_ਤੋਂ_ਛੋਟਾ(Sem);
  ਹਾਲਤ APFloat::fc_ਸਾਧਾਰਨ:
    ਰੋਕੋ;
  }
  ਜੇ (X.isDenormal() || X.is_ਸਭ_ਤੋਂ_ਛੋਟਾ_Normalized())
    ਵਾਪਸ APFloat::get_ਸਭ_ਤੋਂ_ਛੋਟਾ(Sem);
  ਪੂਰਨ Exp = ilogb(X);
  ਜੇ (X.getExactLog2() != INT_MIN)
    Exp -= 1;
  ਵਾਪਸ scalbn(APFloat::getOne(Sem), Exp - (Sem.precision - 1),
                APFloat::rm_ਸਭ_ਤੋਂ_ਨੇੜੇ_Ties_ਮੰਜ਼ਿਲ_Even);
}

ਨਾਮਾਕਾਸ਼ detail {
/* Constructors.  */
ਖਾਲੀ IEEEFloat::initialize(ਅਚਲ fltSemantics *ourSemantics) {
  semantics = ourSemantics;
  ਬਿਨਾਨਿਸ਼ਾਨੀ ਗਿਣਤੀ_2 = part_ਗਿਣਤੀ();
  ਜੇ (ਗਿਣਤੀ_2 > 1)
    significand.parts = ਨਵਾਂ integerPart[ਗਿਣਤੀ_2];
}

ਖਾਲੀ IEEEFloat::freeSignificand() {
  ਜੇ (needsCleanup())
    ਮਿਟਾਓ [] significand.parts;
}

ਖਾਲੀ IEEEFloat::ਸੌਂਪੋ(ਅਚਲ IEEEFloat &rhs) {
  ਪ੍ਰਤਿਜ੍ਞਾ(semantics == rhs.semantics);

  sign = rhs.sign;
  category = rhs.category;
  exponent = rhs.exponent;
  ਜੇ (isFiniteNonZero() || category == fcNaN)
    copySignificand(rhs);
}

ਖਾਲੀ IEEEFloat::copySignificand(ਅਚਲ IEEEFloat &rhs) {
  ਪ੍ਰਤਿਜ੍ਞਾ(isFiniteNonZero() || category == fcNaN);
  ਪ੍ਰਤਿਜ੍ਞਾ(rhs.part_ਗਿਣਤੀ() >= part_ਗਿਣਤੀ());

  APInt::tc_ਸੌਂਪੋ(significandParts(), rhs.significandParts(),
                  part_ਗਿਣਤੀ());
}

/* Make this number a NaN, with an arbitrary but deterministic value
   for the significand.  If double or longer, this is a signalling NaN,
   which may not be ideal.  If float, this is QNaN(0).  */
ਖਾਲੀ IEEEFloat::makeNaN(ਤਾਰਕਿਕ SNaN, ਤਾਰਕਿਕ Negative, ਅਚਲ APInt *fill) {
  ਜੇ (semantics->nonFiniteBehavior == fltNonfiniteBehavior::FiniteOnly)
    llvm_unreachable("This floating point format does not support NaN");

  ਜੇ (Negative && !semantics->hasSignedRepr)
    llvm_unreachable(
        "This floating point format does not support signed values");

  category = fcNaN;
  sign = Negative;
  exponent = exponentNaN();

  integerPart *significand = significandParts();
  ਬਿਨਾਨਿਸ਼ਾਨੀ ਅੰਕ_Parts = part_ਗਿਣਤੀ();

  APInt fill_storage;
  ਜੇ (semantics->nonFiniteBehavior == fltNonfiniteBehavior::NanOnly) {
    // Finite-only types do not distinguish signalling and quiet NaN, so
    // make them all signalling.
    SNaN = ਝੂਠ;
    ਜੇ (semantics->nanEncoding == fltNanEncoding::NegativeZero) {
      sign = ਸੱਚ;
      fill_storage = APInt::getZero(semantics->precision - 1);
    } ਨਹੀਂਤਾਂ {
      fill_storage = APInt::getAllOnes(semantics->precision - 1);
    }
    fill = &fill_storage;
  }

  // Set the significand bits to the fill.
  ਜੇ (!fill || fill->get_ਅੰਕ_ਸ਼ਬਦ() < ਅੰਕ_Parts)
    APInt::tc_ਸੈੱਟ(significand, 0, ਅੰਕ_Parts);
  ਜੇ (fill) {
    APInt::tc_ਸੌਂਪੋ(significand, fill->get_Raw_ਅੰਕੜੇ(),
                    std::min(fill->get_ਅੰਕ_ਸ਼ਬਦ(), ਅੰਕ_Parts));

    // Zero out the excess bits of the significand.
    ਬਿਨਾਨਿਸ਼ਾਨੀ ਬਿੱਟ_ਮੰਜ਼ਿਲ_Preserve = semantics->precision - 1;
    ਬਿਨਾਨਿਸ਼ਾਨੀ part = ਬਿੱਟ_ਮੰਜ਼ਿਲ_Preserve / 64;
    ਬਿੱਟ_ਮੰਜ਼ਿਲ_Preserve %= 64;
    significand[part] &= ((1ULL << ਬਿੱਟ_ਮੰਜ਼ਿਲ_Preserve) - 1);
    ਲਈ (part++; part != ਅੰਕ_Parts; ++part)
      significand[part] = 0;
  }

  ਬਿਨਾਨਿਸ਼ਾਨੀ QNaNBit =
      (semantics->precision >= 2) ? (semantics->precision - 2) : 0;

  ਜੇ (SNaN) {
    // We always have to clear the QNaN bit to make it an SNaN.
    APInt::tc_Clear_ਦੋਆਧਾਰੀ_ਅੰਕ(significand, QNaNBit);

    // If there are no bits set in the payload, we have to set
    // *something* to make it a NaN instead of an infinity;
    // conventionally, this is the next bit down from the QNaN bit.
    ਜੇ (APInt::tcIsZero(significand, ਅੰਕ_Parts))
      APInt::tc_ਸੈੱਟ_ਦੋਆਧਾਰੀ_ਅੰਕ(significand, QNaNBit - 1);
  } ਨਹੀਂਤਾਂ ਜੇ (semantics->nanEncoding == fltNanEncoding::NegativeZero) {
    // The only NaN is a quiet NaN, and it has no bits sets in the significand.
    // Do nothing.
  } ਨਹੀਂਤਾਂ {
    // We always have to set the QNaN bit to make it a QNaN.
    APInt::tc_ਸੈੱਟ_ਦੋਆਧਾਰੀ_ਅੰਕ(significand, QNaNBit);
  }

  // For x87 extended precision, we want to make a NaN, not a
  // pseudo-NaN.  Maybe we should expose the ability to make
  // pseudo-NaNs?
  ਜੇ (semantics == &APFloat_ਅਧਾਰ::semX87DoubleExtended)
    APInt::tc_ਸੈੱਟ_ਦੋਆਧਾਰੀ_ਅੰਕ(significand, QNaNBit + 1);
}

IEEEFloat &IEEEFloat::ਸੰਚਾਲਕ=(ਅਚਲ IEEEFloat &rhs) {
  ਜੇ (ਇਹ != &rhs) {
    ਜੇ (semantics != rhs.semantics) {
      freeSignificand();
      initialize(rhs.semantics);
    }
    ਸੌਂਪੋ(rhs);
  }

  ਵਾਪਸ *ਇਹ;
}

IEEEFloat &IEEEFloat::ਸੰਚਾਲਕ=(IEEEFloat &&rhs) {
  freeSignificand();

  semantics = rhs.semantics;
  significand = rhs.significand;
  exponent = rhs.exponent;
  category = rhs.category;
  sign = rhs.sign;

  rhs.semantics = &APFloat_ਅਧਾਰ::semBogus;
  ਵਾਪਸ *ਇਹ;
}

ਤਾਰਕਿਕ IEEEFloat::isDenormal() ਅਚਲ {
  ਵਾਪਸ getSemantics().hasDenormals && isFiniteNonZero() &&
         (exponent == semantics->minExponent) &&
         (APInt::tc_ਕੱਢੋ_ਦੋਆਧਾਰੀ_ਅੰਕ(significandParts(), semantics->precision - 1) ==
          0);
}

ਤਾਰਕਿਕ IEEEFloat::is_ਸਭ_ਤੋਂ_ਛੋਟਾ() ਅਚਲ {
  // The smallest number by magnitude in our format will be the smallest
  // denormal, i.e. the floating point number with exponent being minimum
  // exponent and significand bitwise equal to 1 (i.e. with MSB equal to 0).
  ਵਾਪਸ isFiniteNonZero() && exponent == semantics->minExponent &&
    significandMSB() == 0;
}

ਤਾਰਕਿਕ IEEEFloat::is_ਸਭ_ਤੋਂ_ਛੋਟਾ_Normalized() ਅਚਲ {
  ਵਾਪਸ getCategory() == fc_ਸਾਧਾਰਨ && exponent == semantics->minExponent &&
         isSignificandAllZerosExceptMSB();
}

ਬਿਨਾਨਿਸ਼ਾਨੀ ਪੂਰਨ IEEEFloat::get_ਅੰਕ_ਉੱਚਾ_ਬਿੱਟ() ਅਚਲ {
  ਅਚਲ ਬਿਨਾਨਿਸ਼ਾਨੀ ਪੂਰਨ Part_ਗਿਣਤੀ = part_ਗਿਣਤੀ_For_ਬਿੱਟ(semantics->precision);
  ਅਚਲ ਬਿਨਾਨਿਸ਼ਾਨੀ ਪੂਰਨ ਬਿੱਟ_2 = Part_ਗਿਣਤੀ * integer_Part_ਚੌੜਾਈ;

  // Compute how many bits are used in the final word.
  // When precision is just 1, it represents the 'Pth'
  // Precision bit and not the actual significand bit.
  ਅਚਲ ਬਿਨਾਨਿਸ਼ਾਨੀ ਪੂਰਨ ਅੰਕ_ਉੱਚਾ_ਬਿੱਟ = (semantics->precision > 1)
                                       ? (ਬਿੱਟ_2 - semantics->precision + 1)
                                       : (ਬਿੱਟ_2 - semantics->precision);
  ਵਾਪਸ ਅੰਕ_ਉੱਚਾ_ਬਿੱਟ;
}

ਤਾਰਕਿਕ IEEEFloat::isSignificandAllOnes() ਅਚਲ {
  // Test if the significand excluding the integral bit is all ones. This allows
  // us to test for binade boundaries.
  ਅਚਲ integerPart *Parts = significandParts();
  ਅਚਲ ਬਿਨਾਨਿਸ਼ਾਨੀ Part_ਗਿਣਤੀ = part_ਗਿਣਤੀ_For_ਬਿੱਟ(semantics->precision);
  ਲਈ (ਬਿਨਾਨਿਸ਼ਾਨੀ i = 0; i < Part_ਗਿਣਤੀ - 1; i++)
    ਜੇ (~Parts[i])
      ਵਾਪਸ ਝੂਠ;

  // Set the unused high bits to all ones when we compare.
  ਅਚਲ ਬਿਨਾਨਿਸ਼ਾਨੀ ਅੰਕ_ਉੱਚਾ_ਬਿੱਟ = get_ਅੰਕ_ਉੱਚਾ_ਬਿੱਟ();
  ਪ੍ਰਤਿਜ੍ਞਾ(ਅੰਕ_ਉੱਚਾ_ਬਿੱਟ <= integer_Part_ਚੌੜਾਈ && ਅੰਕ_ਉੱਚਾ_ਬਿੱਟ > 0 &&
         "Can not have more high bits to fill than integerPartWidth");
  ਅਚਲ integerPart ਉੱਚਾ_ਦੋਆਧਾਰੀ_ਅੰਕ_Fill =
    ~integerPart(0) << (integer_Part_ਚੌੜਾਈ - ਅੰਕ_ਉੱਚਾ_ਬਿੱਟ);
  ਜੇ ((semantics->precision <= 1) || (~(Parts[Part_ਗਿਣਤੀ - 1] | ਉੱਚਾ_ਦੋਆਧਾਰੀ_ਅੰਕ_Fill)))
    ਵਾਪਸ ਝੂਠ;

  ਵਾਪਸ ਸੱਚ;
}

ਤਾਰਕਿਕ IEEEFloat::isSignificandAllOnesExceptLSB() ਅਚਲ {
  // Test if the significand excluding the integral bit is all ones except for
  // the least significant bit.
  ਅਚਲ integerPart *Parts = significandParts();

  ਜੇ (Parts[0] & 1)
    ਵਾਪਸ ਝੂਠ;

  ਅਚਲ ਬਿਨਾਨਿਸ਼ਾਨੀ Part_ਗਿਣਤੀ = part_ਗਿਣਤੀ_For_ਬਿੱਟ(semantics->precision);
  ਲਈ (ਬਿਨਾਨਿਸ਼ਾਨੀ i = 0; i < Part_ਗਿਣਤੀ - 1; i++) {
    ਜੇ (~Parts[i] & ~ਬਿਨਾਨਿਸ਼ਾਨੀ{!i})
      ਵਾਪਸ ਝੂਠ;
  }

  // Set the unused high bits to all ones when we compare.
  ਅਚਲ ਬਿਨਾਨਿਸ਼ਾਨੀ ਅੰਕ_ਉੱਚਾ_ਬਿੱਟ = get_ਅੰਕ_ਉੱਚਾ_ਬਿੱਟ();
  ਪ੍ਰਤਿਜ੍ਞਾ(ਅੰਕ_ਉੱਚਾ_ਬਿੱਟ <= integer_Part_ਚੌੜਾਈ && ਅੰਕ_ਉੱਚਾ_ਬਿੱਟ > 0 &&
         "Can not have more high bits to fill than integerPartWidth");
  ਅਚਲ integerPart ਉੱਚਾ_ਦੋਆਧਾਰੀ_ਅੰਕ_Fill = ~integerPart(0)
                                  << (integer_Part_ਚੌੜਾਈ - ਅੰਕ_ਉੱਚਾ_ਬਿੱਟ);
  ਜੇ (~(Parts[Part_ਗਿਣਤੀ - 1] | ਉੱਚਾ_ਦੋਆਧਾਰੀ_ਅੰਕ_Fill | 0x1))
    ਵਾਪਸ ਝੂਠ;

  ਵਾਪਸ ਸੱਚ;
}

ਤਾਰਕਿਕ IEEEFloat::isSignificandAllZeros() ਅਚਲ {
  // Test if the significand excluding the integral bit is all zeros. This
  // allows us to test for binade boundaries.
  ਅਚਲ integerPart *Parts = significandParts();
  ਅਚਲ ਬਿਨਾਨਿਸ਼ਾਨੀ Part_ਗਿਣਤੀ = part_ਗਿਣਤੀ_For_ਬਿੱਟ(semantics->precision);

  ਲਈ (ਬਿਨਾਨਿਸ਼ਾਨੀ i = 0; i < Part_ਗਿਣਤੀ - 1; i++)
    ਜੇ (Parts[i])
      ਵਾਪਸ ਝੂਠ;

  // Compute how many bits are used in the final word.
  ਅਚਲ ਬਿਨਾਨਿਸ਼ਾਨੀ ਅੰਕ_ਉੱਚਾ_ਬਿੱਟ = get_ਅੰਕ_ਉੱਚਾ_ਬਿੱਟ();
  ਪ੍ਰਤਿਜ੍ਞਾ(ਅੰਕ_ਉੱਚਾ_ਬਿੱਟ < integer_Part_ਚੌੜਾਈ && "Can not have more high bits to "
         "clear than integerPartWidth");
  ਅਚਲ integerPart ਉੱਚਾ_ਦੋਆਧਾਰੀ_ਅੰਕ_ਨਕਾਬ = ~integerPart(0) >> ਅੰਕ_ਉੱਚਾ_ਬਿੱਟ;

  ਜੇ ((semantics->precision > 1) && (Parts[Part_ਗਿਣਤੀ - 1] & ਉੱਚਾ_ਦੋਆਧਾਰੀ_ਅੰਕ_ਨਕਾਬ))
    ਵਾਪਸ ਝੂਠ;

  ਵਾਪਸ ਸੱਚ;
}

ਤਾਰਕਿਕ IEEEFloat::isSignificandAllZerosExceptMSB() ਅਚਲ {
  ਅਚਲ integerPart *Parts = significandParts();
  ਅਚਲ ਬਿਨਾਨਿਸ਼ਾਨੀ Part_ਗਿਣਤੀ = part_ਗਿਣਤੀ_For_ਬਿੱਟ(semantics->precision);

  ਲਈ (ਬਿਨਾਨਿਸ਼ਾਨੀ i = 0; i < Part_ਗਿਣਤੀ - 1; i++) {
    ਜੇ (Parts[i])
      ਵਾਪਸ ਝੂਠ;
  }

  ਅਚਲ ਬਿਨਾਨਿਸ਼ਾਨੀ ਅੰਕ_ਉੱਚਾ_ਬਿੱਟ = get_ਅੰਕ_ਉੱਚਾ_ਬਿੱਟ();
  ਅਚਲ integerPart MSBMask = integerPart(1)
                              << (integer_Part_ਚੌੜਾਈ - ਅੰਕ_ਉੱਚਾ_ਬਿੱਟ);
  ਵਾਪਸ ((semantics->precision <= 1) || (Parts[Part_ਗਿਣਤੀ - 1] == MSBMask));
}

ਤਾਰਕਿਕ IEEEFloat::is_ਸਭ_ਤੋਂ_ਵੱਡਾ() ਅਚਲ {
  ਤਾਰਕਿਕ IsMaxExp = isFiniteNonZero() && exponent == semantics->maxExponent;
  ਜੇ (semantics->nonFiniteBehavior == fltNonfiniteBehavior::NanOnly &&
      semantics->nanEncoding == fltNanEncoding::AllOnes) {
    // The largest number by magnitude in our format will be the floating point
    // number with maximum exponent and with significand that is all ones except
    // the LSB.
    ਵਾਪਸ (IsMaxExp && APFloat::hasSignificand(*semantics))
               ? isSignificandAllOnesExceptLSB()
               : IsMaxExp;
  } ਨਹੀਂਤਾਂ {
    // The largest number by magnitude in our format will be the floating point
    // number with maximum exponent and with significand that is all ones.
    ਵਾਪਸ IsMaxExp && isSignificandAllOnes();
  }
}

ਤਾਰਕਿਕ IEEEFloat::isInteger() ਅਚਲ {
  // This could be made more efficient; I'm going for obviously correct.
  ਜੇ (!isFinite()) ਵਾਪਸ ਝੂਠ;
  IEEEFloat truncated = *ਇਹ;
  truncated.ਗੇੜਾ_ਮੰਜ਼ਿਲ_Integral(rmTowardZero);
  ਵਾਪਸ compare(truncated) == cmpEqual;
}

ਤਾਰਕਿਕ IEEEFloat::bitwiseIsEqual(ਅਚਲ IEEEFloat &rhs) ਅਚਲ {
  ਜੇ (ਇਹ == &rhs)
    ਵਾਪਸ ਸੱਚ;
  ਜੇ (semantics != rhs.semantics ||
      category != rhs.category ||
      sign != rhs.sign)
    ਵਾਪਸ ਝੂਠ;
  ਜੇ (category==fcZero || category==fcInfinity)
    ਵਾਪਸ ਸੱਚ;

  ਜੇ (isFiniteNonZero() && exponent != rhs.exponent)
    ਵਾਪਸ ਝੂਠ;

  ਵਾਪਸ std::equal(significandParts(), significandParts() + part_ਗਿਣਤੀ(),
                    rhs.significandParts());
}

IEEEFloat::IEEEFloat(ਅਚਲ fltSemantics &ourSemantics, integerPart ਮੁੱਲ_2) {
  initialize(&ourSemantics);
  sign = 0;
  category = fc_ਸਾਧਾਰਨ;
  zeroSignificand();
  exponent = ourSemantics.precision - 1;
  significandParts()[0] = ਮੁੱਲ_2;
  normalize(rm_ਸਭ_ਤੋਂ_ਨੇੜੇ_Ties_ਮੰਜ਼ਿਲ_Even, lfExactlyZero);
}

IEEEFloat::IEEEFloat(ਅਚਲ fltSemantics &ourSemantics) {
  initialize(&ourSemantics);
  // The Float8E8MOFNU format does not have a representation
  // for zero. So, use the closest representation instead.
  // Moreover, the all-zero encoding represents a valid
  // normal value (which is the smallestNormalized here).
  // Hence, we call makeSmallestNormalized (where category is
  // 'fcNormal') instead of makeZero (where category is 'fcZero').
  ourSemantics.hasZero ? makeZero(ਝੂਠ) : make_ਸਭ_ਤੋਂ_ਛੋਟਾ_Normalized(ਝੂਠ);
}

// Delegate to the previous constructor, because later copy constructor may
// actually inspects category, which can't be garbage.
IEEEFloat::IEEEFloat(ਅਚਲ fltSemantics &ourSemantics, uninitialized_ਟੈਗ ਟੈਗ_2)
    : IEEEFloat(ourSemantics) {}

IEEEFloat::IEEEFloat(ਅਚਲ IEEEFloat &rhs) {
  initialize(rhs.semantics);
  ਸੌਂਪੋ(rhs);
}

IEEEFloat::IEEEFloat(IEEEFloat &&rhs) : semantics(&APFloat_ਅਧਾਰ::semBogus) {
  *ਇਹ = std::move(rhs);
}

IEEEFloat::~IEEEFloat() { freeSignificand(); }

ਬਿਨਾਨਿਸ਼ਾਨੀ ਪੂਰਨ IEEEFloat::part_ਗਿਣਤੀ() ਅਚਲ {
  ਵਾਪਸ part_ਗਿਣਤੀ_For_ਬਿੱਟ(semantics->precision + 1);
}

ਅਚਲ APFloat::integerPart *IEEEFloat::significandParts() ਅਚਲ {
  ਵਾਪਸ const_cast<IEEEFloat *>(ਇਹ)->significandParts();
}

APFloat::integerPart *IEEEFloat::significandParts() {
  ਜੇ (part_ਗਿਣਤੀ() > 1)
    ਵਾਪਸ significand.parts;
  ਨਹੀਂਤਾਂ
    ਵਾਪਸ &significand.part;
}

ਖਾਲੀ IEEEFloat::zeroSignificand() {
  APInt::tc_ਸੈੱਟ(significandParts(), 0, part_ਗਿਣਤੀ());
}

/* Increment an fcNormal floating point number's significand.  */
ਖਾਲੀ IEEEFloat::incrementSignificand() {
  [[maybe_ਅਣਵਰਤਿਆ]] integerPart ਹਾਸਲ =
      APInt::tcIncrement(significandParts(), part_ਗਿਣਤੀ());

  /* Our callers should never cause us to overflow.  */
  ਪ੍ਰਤਿਜ੍ਞਾ(ਹਾਸਲ == 0);
}

/* Add the significand of the RHS.  Returns the carry flag.  */
APFloat::integerPart IEEEFloat::ਜੋੜੋ_Significand(ਅਚਲ IEEEFloat &rhs) {
  integerPart *parts = significandParts();

  ਪ੍ਰਤਿਜ੍ਞਾ(semantics == rhs.semantics);
  ਪ੍ਰਤਿਜ੍ਞਾ(exponent == rhs.exponent);

  ਵਾਪਸ APInt::tc_ਜੋੜੋ(parts, rhs.significandParts(), 0, part_ਗਿਣਤੀ());
}

/* Subtract the significand of the RHS with a borrow flag.  Returns
   the borrow flag.  */
APFloat::integerPart IEEEFloat::ਘਟਾਓ_Significand(ਅਚਲ IEEEFloat &rhs,
                                                    integerPart borrow) {
  integerPart *parts = significandParts();

  ਪ੍ਰਤਿਜ੍ਞਾ(semantics == rhs.semantics);
  ਪ੍ਰਤਿਜ੍ਞਾ(exponent == rhs.exponent);

  ਵਾਪਸ APInt::tc_ਘਟਾਓ(parts, rhs.significandParts(), borrow,
                           part_ਗਿਣਤੀ());
}

/* Multiply the significand of the RHS.  If ADDEND is non-NULL, add it
   on to the full-precision result of the multiplication.  Returns the
   lost fraction.  */
ਗੁਆਚਾ_Fraction IEEEFloat::multiplySignificand(ਅਚਲ IEEEFloat &rhs,
                                            IEEEFloat addend,
                                            ਤਾਰਕਿਕ ignoreAddend) {
  integerPart scratch[4];
  ਤਾਰਕਿਕ ignored;

  ਪ੍ਰਤਿਜ੍ਞਾ(semantics == rhs.semantics);

  ਬਿਨਾਨਿਸ਼ਾਨੀ precision = semantics->precision;

  // Allocate space for twice as many bits as the original significand, plus one
  // extra bit for the addition to overflow into.
  ਬਿਨਾਨਿਸ਼ਾਨੀ new_Parts_ਗਿਣਤੀ = part_ਗਿਣਤੀ_For_ਬਿੱਟ(precision * 2 + 1);

  // FIXME: Replace with SmallVector<4>.
  integerPart *ਭਰਿਆ_Significand =
      new_Parts_ਗਿਣਤੀ > 4 ? ਨਵਾਂ integerPart[new_Parts_ਗਿਣਤੀ] : scratch;

  integerPart *lhsSignificand = significandParts();
  ਬਿਨਾਨਿਸ਼ਾਨੀ parts_ਗਿਣਤੀ = part_ਗਿਣਤੀ();

  APInt::tc_ਭਰਿਆ_Multiply(ਭਰਿਆ_Significand, lhsSignificand,
                        rhs.significandParts(), parts_ਗਿਣਤੀ, parts_ਗਿਣਤੀ);

  ਗੁਆਚਾ_Fraction ਗੁਆਚਾ_fraction = lfExactlyZero;
  // One, not zero, based MSB.
  ਬਿਨਾਨਿਸ਼ਾਨੀ omsb = APInt::tcMSB(ਭਰਿਆ_Significand, new_Parts_ਗਿਣਤੀ) + 1;
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

  ਜੇ (!ignoreAddend && addend.isNonZero()) {
    // The intermediate result of the multiplication has "2 * precision"
    // signicant bit; adjust the addend to be consistent with mul result.
    //
    Significand savedSignificand = significand;
    ਅਚਲ fltSemantics *savedSemantics = semantics;

    // Normalize our MSB to one below the top bit to allow for overflow.
    ਬਿਨਾਨਿਸ਼ਾਨੀ extendedPrecision = 2 * precision + 1;
    ਜੇ (omsb != extendedPrecision - 1) {
      ਪ੍ਰਤਿਜ੍ਞਾ(extendedPrecision > omsb);
      APInt::tc_ਖਿਸਕਾਓ_ਖੱਬਾ(ਭਰਿਆ_Significand, new_Parts_ਗਿਣਤੀ,
                         (extendedPrecision - 1) - omsb);
      exponent -= (extendedPrecision - 1) - omsb;
    }

    /* Create new semantics.  */
    fltSemantics extendedSemantics = *semantics;
    extendedSemantics.precision = extendedPrecision;

    ਜੇ (new_Parts_ਗਿਣਤੀ == 1)
      significand.part = ਭਰਿਆ_Significand[0];
    ਨਹੀਂਤਾਂ
      significand.parts = ਭਰਿਆ_Significand;
    semantics = &extendedSemantics;

    // Make a copy so we can convert it to the extended semantics.
    // Note that we cannot convert the addend directly, as the extendedSemantics
    // is a local variable (which we take a reference to).
    IEEEFloat extendedAddend(addend);
    [[maybe_ਅਣਵਰਤਿਆ]] op_ਸਥਿਤੀ ਸਥਿਤੀ_2 = extendedAddend.convert(
        extendedSemantics, APFloat::rmTowardZero, &ignored);
    ਪ੍ਰਤਿਜ੍ਞਾ(ਸਥਿਤੀ_2 == APFloat::opOK);

    // Shift the significand of the addend right by one bit. This guarantees
    // that the high bit of the significand is zero (same as fullSignificand),
    // so the addition will overflow (if it does overflow at all) into the top bit.
    ਗੁਆਚਾ_fraction = extendedAddend.ਖਿਸਕਾਓ_Significand_ਸੱਜਾ(1);
    ਪ੍ਰਤਿਜ੍ਞਾ(ਗੁਆਚਾ_fraction == lfExactlyZero &&
           "Lost precision while shifting addend for fused-multiply-add.");

    ਗੁਆਚਾ_fraction = ਜੋੜੋ_Or_ਘਟਾਓ_Significand(extendedAddend, ਝੂਠ);

    /* Restore our state.  */
    ਜੇ (new_Parts_ਗਿਣਤੀ == 1)
      ਭਰਿਆ_Significand[0] = significand.part;
    significand = savedSignificand;
    semantics = savedSemantics;

    omsb = APInt::tcMSB(ਭਰਿਆ_Significand, new_Parts_ਗਿਣਤੀ) + 1;
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
  ਜੇ (omsb > precision) {
    ਬਿਨਾਨਿਸ਼ਾਨੀ ਪੂਰਨ ਬਿੱਟ, significantParts;
    ਗੁਆਚਾ_Fraction lf;

    ਬਿੱਟ = omsb - precision;
    significantParts = part_ਗਿਣਤੀ_For_ਬਿੱਟ(omsb);
    lf = ਖਿਸਕਾਓ_ਸੱਜਾ(ਭਰਿਆ_Significand, significantParts, ਬਿੱਟ);
    ਗੁਆਚਾ_fraction = combine_ਗੁਆਚਾ_Fractions(lf, ਗੁਆਚਾ_fraction);
    exponent += ਬਿੱਟ;
  }

  APInt::tc_ਸੌਂਪੋ(lhsSignificand, ਭਰਿਆ_Significand, parts_ਗਿਣਤੀ);

  ਜੇ (new_Parts_ਗਿਣਤੀ > 4)
    ਮਿਟਾਓ [] ਭਰਿਆ_Significand;

  ਵਾਪਸ ਗੁਆਚਾ_fraction;
}

ਗੁਆਚਾ_Fraction IEEEFloat::multiplySignificand(ਅਚਲ IEEEFloat &rhs) {
  // When the given semantics has zero, the addend here is a zero.
  // i.e . it belongs to the 'fcZero' category.
  // But when the semantics does not support zero, we need to
  // explicitly convey that this addend should be ignored
  // for multiplication.
  ਵਾਪਸ multiplySignificand(rhs, IEEEFloat(*semantics), !semantics->hasZero);
}

/* Multiply the significands of LHS and RHS to DST.  */
ਗੁਆਚਾ_Fraction IEEEFloat::divideSignificand(ਅਚਲ IEEEFloat &rhs) {
  integerPart scratch[4];

  ਪ੍ਰਤਿਜ੍ਞਾ(semantics == rhs.semantics);

  integerPart *lhsSignificand = significandParts();
  ਅਚਲ integerPart *rhsSignificand = rhs.significandParts();
  ਬਿਨਾਨਿਸ਼ਾਨੀ parts_ਗਿਣਤੀ = part_ਗਿਣਤੀ();

  integerPart *dividend =
      parts_ਗਿਣਤੀ > 2 ? ਨਵਾਂ integerPart[parts_ਗਿਣਤੀ * 2] : scratch;
  integerPart *ਭਾਜਕ = dividend + parts_ਗਿਣਤੀ;

  /* Copy the dividend and divisor as they will be modified in-place.  */
  ਲਈ (ਬਿਨਾਨਿਸ਼ਾਨੀ i = 0; i < parts_ਗਿਣਤੀ; i++) {
    dividend[i] = lhsSignificand[i];
    ਭਾਜਕ[i] = rhsSignificand[i];
    lhsSignificand[i] = 0;
  }

  exponent -= rhs.exponent;

  ਬਿਨਾਨਿਸ਼ਾਨੀ ਪੂਰਨ precision = semantics->precision;

  /* Normalize the divisor.  */
  ਬਿਨਾਨਿਸ਼ਾਨੀ ਦੋਆਧਾਰੀ_ਅੰਕ = precision - APInt::tcMSB(ਭਾਜਕ, parts_ਗਿਣਤੀ) - 1;
  ਜੇ (ਦੋਆਧਾਰੀ_ਅੰਕ) {
    exponent += ਦੋਆਧਾਰੀ_ਅੰਕ;
    APInt::tc_ਖਿਸਕਾਓ_ਖੱਬਾ(ਭਾਜਕ, parts_ਗਿਣਤੀ, ਦੋਆਧਾਰੀ_ਅੰਕ);
  }

  /* Normalize the dividend.  */
  ਦੋਆਧਾਰੀ_ਅੰਕ = precision - APInt::tcMSB(dividend, parts_ਗਿਣਤੀ) - 1;
  ਜੇ (ਦੋਆਧਾਰੀ_ਅੰਕ) {
    exponent -= ਦੋਆਧਾਰੀ_ਅੰਕ;
    APInt::tc_ਖਿਸਕਾਓ_ਖੱਬਾ(dividend, parts_ਗਿਣਤੀ, ਦੋਆਧਾਰੀ_ਅੰਕ);
  }

  /* Ensure the dividend >= divisor initially for the loop below.
     Incidentally, this means that the division loop below is
     guaranteed to set the integer bit to one.  */
  ਜੇ (APInt::tcCompare(dividend, ਭਾਜਕ, parts_ਗਿਣਤੀ) < 0) {
    exponent--;
    APInt::tc_ਖਿਸਕਾਓ_ਖੱਬਾ(dividend, parts_ਗਿਣਤੀ, 1);
    ਪ੍ਰਤਿਜ੍ਞਾ(APInt::tcCompare(dividend, ਭਾਜਕ, parts_ਗਿਣਤੀ) >= 0);
  }

  /* Long division.  */
  ਲਈ (ਦੋਆਧਾਰੀ_ਅੰਕ = precision; ਦੋਆਧਾਰੀ_ਅੰਕ; ਦੋਆਧਾਰੀ_ਅੰਕ -= 1) {
    ਜੇ (APInt::tcCompare(dividend, ਭਾਜਕ, parts_ਗਿਣਤੀ) >= 0) {
      APInt::tc_ਘਟਾਓ(dividend, ਭਾਜਕ, 0, parts_ਗਿਣਤੀ);
      APInt::tc_ਸੈੱਟ_ਦੋਆਧਾਰੀ_ਅੰਕ(lhsSignificand, ਦੋਆਧਾਰੀ_ਅੰਕ - 1);
    }

    APInt::tc_ਖਿਸਕਾਓ_ਖੱਬਾ(dividend, parts_ਗਿਣਤੀ, 1);
  }

  /* Figure out the lost fraction.  */
  ਪੂਰਨ cmp = APInt::tcCompare(dividend, ਭਾਜਕ, parts_ਗਿਣਤੀ);

  ਗੁਆਚਾ_Fraction ਗੁਆਚਾ_fraction;
  ਜੇ (cmp > 0)
    ਗੁਆਚਾ_fraction = lf_ਹੋਰ_Than_Half;
  ਨਹੀਂਤਾਂ ਜੇ (cmp == 0)
    ਗੁਆਚਾ_fraction = lfExactlyHalf;
  ਨਹੀਂਤਾਂ ਜੇ (APInt::tcIsZero(dividend, parts_ਗਿਣਤੀ))
    ਗੁਆਚਾ_fraction = lfExactlyZero;
  ਨਹੀਂਤਾਂ
    ਗੁਆਚਾ_fraction = lfLessThanHalf;

  ਜੇ (parts_ਗਿਣਤੀ > 2)
    ਮਿਟਾਓ [] dividend;

  ਵਾਪਸ ਗੁਆਚਾ_fraction;
}

ਬਿਨਾਨਿਸ਼ਾਨੀ ਪੂਰਨ IEEEFloat::significandMSB() ਅਚਲ {
  ਵਾਪਸ APInt::tcMSB(significandParts(), part_ਗਿਣਤੀ());
}

ਬਿਨਾਨਿਸ਼ਾਨੀ ਪੂਰਨ IEEEFloat::significandLSB() ਅਚਲ {
  ਵਾਪਸ APInt::tcLSB(significandParts(), part_ਗਿਣਤੀ());
}

/* Note that a zero result is NOT normalized to fcZero.  */
ਗੁਆਚਾ_Fraction IEEEFloat::ਖਿਸਕਾਓ_Significand_ਸੱਜਾ(ਬਿਨਾਨਿਸ਼ਾਨੀ ਪੂਰਨ ਬਿੱਟ) {
  /* Our exponent should not overflow.  */
  ਪ੍ਰਤਿਜ੍ਞਾ((ExponentType) (exponent + ਬਿੱਟ) >= exponent);

  exponent += ਬਿੱਟ;

  ਵਾਪਸ ਖਿਸਕਾਓ_ਸੱਜਾ(significandParts(), part_ਗਿਣਤੀ(), ਬਿੱਟ);
}

/* Shift the significand left BITS bits, subtract BITS from its exponent.  */
ਖਾਲੀ IEEEFloat::ਖਿਸਕਾਓ_Significand_ਖੱਬਾ(ਬਿਨਾਨਿਸ਼ਾਨੀ ਪੂਰਨ ਬਿੱਟ) {
  ਪ੍ਰਤਿਜ੍ਞਾ(ਬਿੱਟ < semantics->precision ||
         (semantics->precision == 1 && ਬਿੱਟ <= 1));

  ਜੇ (ਬਿੱਟ) {
    ਬਿਨਾਨਿਸ਼ਾਨੀ ਪੂਰਨ parts_ਗਿਣਤੀ = part_ਗਿਣਤੀ();

    APInt::tc_ਖਿਸਕਾਓ_ਖੱਬਾ(significandParts(), parts_ਗਿਣਤੀ, ਬਿੱਟ);
    exponent -= ਬਿੱਟ;

    ਪ੍ਰਤਿਜ੍ਞਾ(!APInt::tcIsZero(significandParts(), parts_ਗਿਣਤੀ));
  }
}

APFloat::cmp_ਨਤੀਜਾ IEEEFloat::compare_Absolute_ਮੁੱਲ(ਅਚਲ IEEEFloat &rhs) ਅਚਲ {
  ਪ੍ਰਤਿਜ੍ਞਾ(semantics == rhs.semantics);
  ਪ੍ਰਤਿਜ੍ਞਾ(isFiniteNonZero());
  ਪ੍ਰਤਿਜ੍ਞਾ(rhs.isFiniteNonZero());

  ਪੂਰਨ compare = exponent - rhs.exponent;

  /* If exponents are equal, do an unsigned bignum comparison of the
     significands.  */
  ਜੇ (compare == 0)
    compare = APInt::tcCompare(significandParts(), rhs.significandParts(),
                               part_ਗਿਣਤੀ());

  ਜੇ (compare > 0)
    ਵਾਪਸ cmpGreaterThan;
  ਨਹੀਂਤਾਂ ਜੇ (compare < 0)
    ਵਾਪਸ cmpLessThan;
  ਨਹੀਂਤਾਂ
    ਵਾਪਸ cmpEqual;
}

/* Set the least significant BITS bits of a bignum, clear the
   rest.  */
ਸਥਿਰ ਖਾਲੀ tc_ਸੈੱਟ_ਘੱਟ_ਤੋਂ_ਘੱਟ_Significant_ਬਿੱਟ(APInt::ਸ਼ਬਦ_Type *dst, ਬਿਨਾਨਿਸ਼ਾਨੀ parts,
                                      ਬਿਨਾਨਿਸ਼ਾਨੀ ਬਿੱਟ) {
  ਬਿਨਾਨਿਸ਼ਾਨੀ i = 0;
  ਜਦੋਂ (ਬਿੱਟ > APInt::APINT_ਬਿੱਟ_ਪ੍ਰਤੀ_ਸ਼ਬਦ) {
    dst[i++] = ~(APInt::ਸ਼ਬਦ_Type)0;
    ਬਿੱਟ -= APInt::APINT_ਬਿੱਟ_ਪ੍ਰਤੀ_ਸ਼ਬਦ;
  }

  ਜੇ (ਬਿੱਟ)
    dst[i++] = ~(APInt::ਸ਼ਬਦ_Type)0 >> (APInt::APINT_ਬਿੱਟ_ਪ੍ਰਤੀ_ਸ਼ਬਦ - ਬਿੱਟ);

  ਜਦੋਂ (i < parts)
    dst[i++] = 0;
}

/* Handle overflow.  Sign is preserved.  We either become infinity or
   the largest finite number.  */
APFloat::op_ਸਥਿਤੀ IEEEFloat::handle_ਉਛਾਲ(rounding_ਬਹੁਲਕ rounding_ਬਹੁਲਕ_2) {
  ਜੇ (semantics->nonFiniteBehavior != fltNonfiniteBehavior::FiniteOnly) {
    /* Infinity?  */
    ਜੇ (rounding_ਬਹੁਲਕ_2 == rm_ਸਭ_ਤੋਂ_ਨੇੜੇ_Ties_ਮੰਜ਼ਿਲ_Even ||
        rounding_ਬਹੁਲਕ_2 == rm_ਸਭ_ਤੋਂ_ਨੇੜੇ_Ties_ਮੰਜ਼ਿਲ_Away ||
        (rounding_ਬਹੁਲਕ_2 == rm_Toward_ਸਕਾਰਾਤਮਕ && !sign) ||
        (rounding_ਬਹੁਲਕ_2 == rmTowardNegative && sign)) {
      ਜੇ (semantics->nonFiniteBehavior == fltNonfiniteBehavior::NanOnly)
        makeNaN(ਝੂਠ, sign);
      ਨਹੀਂਤਾਂ
        category = fcInfinity;
      ਵਾਪਸ static_cast<op_ਸਥਿਤੀ>(op_ਉਛਾਲ | opInexact);
    }
  }

  /* Otherwise we become the largest finite number.  */
  category = fc_ਸਾਧਾਰਨ;
  exponent = semantics->maxExponent;
  tc_ਸੈੱਟ_ਘੱਟ_ਤੋਂ_ਘੱਟ_Significant_ਬਿੱਟ(significandParts(), part_ਗਿਣਤੀ(),
                            semantics->precision);
  ਜੇ (semantics->nonFiniteBehavior == fltNonfiniteBehavior::NanOnly &&
      semantics->nanEncoding == fltNanEncoding::AllOnes)
    APInt::tc_Clear_ਦੋਆਧਾਰੀ_ਅੰਕ(significandParts(), 0);

  ਵਾਪਸ opInexact;
}

/* Returns TRUE if, when truncating the current number, with BIT the
   new LSB, with the given lost fraction and rounding mode, the result
   would need to be rounded away from zero (i.e., by increasing the
   signficand).  This routine must work for fcZero of both signs, and
   fcNormal numbers.  */
ਤਾਰਕਿਕ IEEEFloat::ਗੇੜਾ_Away_From_Zero(rounding_ਬਹੁਲਕ rounding_ਬਹੁਲਕ_2,
                                  ਗੁਆਚਾ_Fraction ਗੁਆਚਾ_fraction,
                                  ਬਿਨਾਨਿਸ਼ਾਨੀ ਪੂਰਨ ਦੋਆਧਾਰੀ_ਅੰਕ) ਅਚਲ {
  /* NaNs and infinities should not have lost fractions.  */
  ਪ੍ਰਤਿਜ੍ਞਾ(isFiniteNonZero() || category == fcZero);

  /* Current callers never pass this so we don't handle it.  */
  ਪ੍ਰਤਿਜ੍ਞਾ(ਗੁਆਚਾ_fraction != lfExactlyZero);

  ਚੋਣ (rounding_ਬਹੁਲਕ_2) {
  ਹਾਲਤ rm_ਸਭ_ਤੋਂ_ਨੇੜੇ_Ties_ਮੰਜ਼ਿਲ_Away:
    ਵਾਪਸ ਗੁਆਚਾ_fraction == lfExactlyHalf || ਗੁਆਚਾ_fraction == lf_ਹੋਰ_Than_Half;

  ਹਾਲਤ rm_ਸਭ_ਤੋਂ_ਨੇੜੇ_Ties_ਮੰਜ਼ਿਲ_Even:
    ਜੇ (ਗੁਆਚਾ_fraction == lf_ਹੋਰ_Than_Half)
      ਵਾਪਸ ਸੱਚ;

    /* Our zeroes don't have a significand to test.  */
    ਜੇ (ਗੁਆਚਾ_fraction == lfExactlyHalf && category != fcZero)
      ਵਾਪਸ APInt::tc_ਕੱਢੋ_ਦੋਆਧਾਰੀ_ਅੰਕ(significandParts(), ਦੋਆਧਾਰੀ_ਅੰਕ);

    ਵਾਪਸ ਝੂਠ;

  ਹਾਲਤ rmTowardZero:
    ਵਾਪਸ ਝੂਠ;

  ਹਾਲਤ rm_Toward_ਸਕਾਰਾਤਮਕ:
    ਵਾਪਸ !sign;

  ਹਾਲਤ rmTowardNegative:
    ਵਾਪਸ sign;

  ਮੂਲ:
    ਰੋਕੋ;
  }
  llvm_unreachable("Invalid rounding mode found");
}

APFloat::op_ਸਥਿਤੀ IEEEFloat::normalize(rounding_ਬਹੁਲਕ rounding_ਬਹੁਲਕ_2,
                                       ਗੁਆਚਾ_Fraction ਗੁਆਚਾ_fraction) {
  ਜੇ (!isFiniteNonZero())
    ਵਾਪਸ opOK;

  /* Before rounding normalize the exponent of fcNormal numbers.  */
  /* One, not zero, based MSB.  */
  ਬਿਨਾਨਿਸ਼ਾਨੀ omsb = significandMSB() + 1;

  // Only skip this `if` if the value is exactly zero.
  ਜੇ (omsb || ਗੁਆਚਾ_fraction != lfExactlyZero) {
    /* OMSB is numbered from 1.  We want to place it in the integer
       bit numbered PRECISION if possible, with a compensating change in
       the exponent.  */
    ਪੂਰਨ exponentChange = omsb - semantics->precision;

    /* If the resulting exponent is too high, overflow according to
       the rounding mode.  */
    ਜੇ (exponent + exponentChange > semantics->maxExponent)
      ਵਾਪਸ handle_ਉਛਾਲ(rounding_ਬਹੁਲਕ_2);

    /* Subnormal numbers have exponent minExponent, and their MSB
       is forced based on that.  */
    ਜੇ (exponent + exponentChange < semantics->minExponent)
      exponentChange = semantics->minExponent - exponent;

    /* Shifting left is easy as we don't lose precision.  */
    ਜੇ (exponentChange < 0) {
      ਪ੍ਰਤਿਜ੍ਞਾ(ਗੁਆਚਾ_fraction == lfExactlyZero);

      ਖਿਸਕਾਓ_Significand_ਖੱਬਾ(-exponentChange);

      ਵਾਪਸ opOK;
    }

    ਜੇ (exponentChange > 0) {
      ਗੁਆਚਾ_Fraction lf;

      /* Shift right and capture any new lost fraction.  */
      lf = ਖਿਸਕਾਓ_Significand_ਸੱਜਾ(exponentChange);

      ਗੁਆਚਾ_fraction = combine_ਗੁਆਚਾ_Fractions(lf, ਗੁਆਚਾ_fraction);

      /* Keep OMSB up-to-date.  */
      ਜੇ (omsb > (ਬਿਨਾਨਿਸ਼ਾਨੀ) exponentChange)
        omsb -= exponentChange;
      ਨਹੀਂਤਾਂ
        omsb = 0;
    }
  }

  // The all-ones values is an overflow if NaN is all ones. If NaN is
  // represented by negative zero, then it is a valid finite value.
  ਜੇ (semantics->nonFiniteBehavior == fltNonfiniteBehavior::NanOnly &&
      semantics->nanEncoding == fltNanEncoding::AllOnes &&
      exponent == semantics->maxExponent && isSignificandAllOnes())
    ਵਾਪਸ handle_ਉਛਾਲ(rounding_ਬਹੁਲਕ_2);

  /* Now round the number according to rounding_mode given the lost
     fraction.  */

  /* As specified in IEEE 754, since we do not trap we do not report
     underflow for exact results.  */
  ਜੇ (ਗੁਆਚਾ_fraction == lfExactlyZero) {
    /* Canonicalize zeroes.  */
    ਜੇ (omsb == 0) {
      category = fcZero;
      ਜੇ (semantics->nanEncoding == fltNanEncoding::NegativeZero)
        sign = ਝੂਠ;
      ਜੇ (!semantics->hasZero)
        make_ਸਭ_ਤੋਂ_ਛੋਟਾ_Normalized(ਝੂਠ);
    }

    ਵਾਪਸ opOK;
  }

  /* Increment the significand if we're rounding away from zero.  */
  ਜੇ (ਗੇੜਾ_Away_From_Zero(rounding_ਬਹੁਲਕ_2, ਗੁਆਚਾ_fraction, 0)) {
    ਜੇ (omsb == 0)
      exponent = semantics->minExponent;

    incrementSignificand();
    omsb = significandMSB() + 1;

    /* Did the significand increment overflow?  */
    ਜੇ (omsb == (ਬਿਨਾਨਿਸ਼ਾਨੀ) semantics->precision + 1) {
      /* Renormalize by incrementing the exponent and shifting our
         significand right one.  However if we already have the
         maximum exponent we overflow to infinity.  */
      ਜੇ (exponent == semantics->maxExponent)
        // Invoke overflow handling with a rounding mode that will guarantee
        // that the result gets turned into the correct infinity representation.
        // This is needed instead of just setting the category to infinity to
        // account for 8-bit floating point types that have no inf, only NaN.
        ਵਾਪਸ handle_ਉਛਾਲ(sign ? rmTowardNegative : rm_Toward_ਸਕਾਰਾਤਮਕ);

      ਖਿਸਕਾਓ_Significand_ਸੱਜਾ(1);

      ਵਾਪਸ opInexact;
    }

    // The all-ones values is an overflow if NaN is all ones. If NaN is
    // represented by negative zero, then it is a valid finite value.
    ਜੇ (semantics->nonFiniteBehavior == fltNonfiniteBehavior::NanOnly &&
        semantics->nanEncoding == fltNanEncoding::AllOnes &&
        exponent == semantics->maxExponent && isSignificandAllOnes())
      ਵਾਪਸ handle_ਉਛਾਲ(rounding_ਬਹੁਲਕ_2);
  }

  /* The normal case - we were and are not denormal, and any
     significand increment above didn't overflow.  */
  ਜੇ (omsb == semantics->precision)
    ਵਾਪਸ opInexact;

  /* We have a non-zero denormal.  */
  ਪ੍ਰਤਿਜ੍ਞਾ(omsb < semantics->precision);

  /* Canonicalize zeroes.  */
  ਜੇ (omsb == 0) {
    category = fcZero;
    ਜੇ (semantics->nanEncoding == fltNanEncoding::NegativeZero)
      sign = ਝੂਠ;
    // This condition handles the case where the semantics
    // does not have zero but uses the all-zero encoding
    // to represent the smallest normal value.
    ਜੇ (!semantics->hasZero)
      make_ਸਭ_ਤੋਂ_ਛੋਟਾ_Normalized(ਝੂਠ);
  }

  /* The fcZero case is a denormal that underflowed to zero.  */
  ਵਾਪਸ (op_ਸਥਿਤੀ) (opUnderflow | opInexact);
}

APFloat::op_ਸਥਿਤੀ IEEEFloat::ਜੋੜੋ_Or_ਘਟਾਓ_Specials(ਅਚਲ IEEEFloat &rhs,
                                                   ਤਾਰਕਿਕ ਘਟਾਓ) {
  ਚੋਣ (Pack_Categories_Into_ਕੁੰਜੀ(category, rhs.category)) {
  ਮੂਲ:
    llvm_unreachable(ਖਾਲੀ_ਸੂਚਕ);

  ਹਾਲਤ Pack_Categories_Into_ਕੁੰਜੀ(fcZero, fcNaN):
  ਹਾਲਤ Pack_Categories_Into_ਕੁੰਜੀ(fc_ਸਾਧਾਰਨ, fcNaN):
  ਹਾਲਤ Pack_Categories_Into_ਕੁੰਜੀ(fcInfinity, fcNaN):
    ਸੌਂਪੋ(rhs);
    [[fallthrough]];
  ਹਾਲਤ Pack_Categories_Into_ਕੁੰਜੀ(fcNaN, fcZero):
  ਹਾਲਤ Pack_Categories_Into_ਕੁੰਜੀ(fcNaN, fc_ਸਾਧਾਰਨ):
  ਹਾਲਤ Pack_Categories_Into_ਕੁੰਜੀ(fcNaN, fcInfinity):
  ਹਾਲਤ Pack_Categories_Into_ਕੁੰਜੀ(fcNaN, fcNaN):
    ਜੇ (isSignaling()) {
      makeQuiet();
      ਵਾਪਸ opInvalidOp;
    }
    ਵਾਪਸ rhs.isSignaling() ? opInvalidOp : opOK;

  ਹਾਲਤ Pack_Categories_Into_ਕੁੰਜੀ(fc_ਸਾਧਾਰਨ, fcZero):
  ਹਾਲਤ Pack_Categories_Into_ਕੁੰਜੀ(fcInfinity, fc_ਸਾਧਾਰਨ):
  ਹਾਲਤ Pack_Categories_Into_ਕੁੰਜੀ(fcInfinity, fcZero):
    ਵਾਪਸ opOK;

  ਹਾਲਤ Pack_Categories_Into_ਕੁੰਜੀ(fc_ਸਾਧਾਰਨ, fcInfinity):
  ਹਾਲਤ Pack_Categories_Into_ਕੁੰਜੀ(fcZero, fcInfinity):
    category = fcInfinity;
    sign = rhs.sign ^ ਘਟਾਓ;
    ਵਾਪਸ opOK;

  ਹਾਲਤ Pack_Categories_Into_ਕੁੰਜੀ(fcZero, fc_ਸਾਧਾਰਨ):
    ਸੌਂਪੋ(rhs);
    sign = rhs.sign ^ ਘਟਾਓ;
    ਵਾਪਸ opOK;

  ਹਾਲਤ Pack_Categories_Into_ਕੁੰਜੀ(fcZero, fcZero):
    /* Sign depends on rounding mode; handled by caller.  */
    ਵਾਪਸ opOK;

  ਹਾਲਤ Pack_Categories_Into_ਕੁੰਜੀ(fcInfinity, fcInfinity):
    /* Differently signed infinities can only be validly
       subtracted.  */
    ਜੇ (((sign ^ rhs.sign)!=0) != ਘਟਾਓ) {
      makeNaN();
      ਵਾਪਸ opInvalidOp;
    }

    ਵਾਪਸ opOK;

  ਹਾਲਤ Pack_Categories_Into_ਕੁੰਜੀ(fc_ਸਾਧਾਰਨ, fc_ਸਾਧਾਰਨ):
    ਵਾਪਸ opDivByZero;
  }
}

/* Add or subtract two normal numbers.  */
ਗੁਆਚਾ_Fraction IEEEFloat::ਜੋੜੋ_Or_ਘਟਾਓ_Significand(ਅਚਲ IEEEFloat &rhs,
                                                 ਤਾਰਕਿਕ ਘਟਾਓ) {
  [[maybe_ਅਣਵਰਤਿਆ]] integerPart ਹਾਸਲ = 0;
  ਗੁਆਚਾ_Fraction ਗੁਆਚਾ_fraction;

  /* Determine if the operation on the absolute values is effectively
     an addition or subtraction.  */
  ਘਟਾਓ ^= static_cast<ਤਾਰਕਿਕ>(sign ^ rhs.sign);

  /* Are we bigger exponent-wise than the RHS?  */
  ਪੂਰਨ ਬਿੱਟ = exponent - rhs.exponent;

  /* Subtraction is more subtle than one might naively expect.  */
  ਜੇ (ਘਟਾਓ) {
    ਜੇ ((ਬਿੱਟ < 0) && !semantics->hasSignedRepr)
      llvm_unreachable(
          "This floating point format does not support signed values");

    IEEEFloat ਆਰਜ਼ੀ_rhs(rhs);
    ਤਾਰਕਿਕ ਗੁਆਚਾ_fraction_is_from_rhs = ਝੂਠ;

    ਜੇ (ਬਿੱਟ == 0)
      ਗੁਆਚਾ_fraction = lfExactlyZero;
    ਨਹੀਂਤਾਂ ਜੇ (ਬਿੱਟ > 0) {
      ਗੁਆਚਾ_fraction = ਆਰਜ਼ੀ_rhs.ਖਿਸਕਾਓ_Significand_ਸੱਜਾ(ਬਿੱਟ - 1);
      ਗੁਆਚਾ_fraction_is_from_rhs = ਸੱਚ;
      ਖਿਸਕਾਓ_Significand_ਖੱਬਾ(1);
    } ਨਹੀਂਤਾਂ {
      ਗੁਆਚਾ_fraction = ਖਿਸਕਾਓ_Significand_ਸੱਜਾ(-ਬਿੱਟ - 1);
      ਆਰਜ਼ੀ_rhs.ਖਿਸਕਾਓ_Significand_ਖੱਬਾ(1);
    }

    // Should we reverse the subtraction.
    cmp_ਨਤੀਜਾ cmp_ਨਤੀਜਾ_2 = compare_Absolute_ਮੁੱਲ(ਆਰਜ਼ੀ_rhs);
    ਜੇ (cmp_ਨਤੀਜਾ_2 == cmpLessThan) {
      ਤਾਰਕਿਕ borrow =
          ਗੁਆਚਾ_fraction != lfExactlyZero && !ਗੁਆਚਾ_fraction_is_from_rhs;
      ਜੇ (borrow) {
        // The lost fraction is being subtracted, borrow from the significand
        // and invert `lost_fraction`.
        ਜੇ (ਗੁਆਚਾ_fraction == lfLessThanHalf)
          ਗੁਆਚਾ_fraction = lf_ਹੋਰ_Than_Half;
        ਨਹੀਂਤਾਂ ਜੇ (ਗੁਆਚਾ_fraction == lf_ਹੋਰ_Than_Half)
          ਗੁਆਚਾ_fraction = lfLessThanHalf;
      }
      ਹਾਸਲ = ਆਰਜ਼ੀ_rhs.ਘਟਾਓ_Significand(*ਇਹ, borrow);
      copySignificand(ਆਰਜ਼ੀ_rhs);
      sign = !sign;
    } ਨਹੀਂਤਾਂ ਜੇ (cmp_ਨਤੀਜਾ_2 == cmpGreaterThan) {
      ਤਾਰਕਿਕ borrow = ਗੁਆਚਾ_fraction != lfExactlyZero && ਗੁਆਚਾ_fraction_is_from_rhs;
      ਜੇ (borrow) {
        // The lost fraction is being subtracted, borrow from the significand
        // and invert `lost_fraction`.
        ਜੇ (ਗੁਆਚਾ_fraction == lfLessThanHalf)
          ਗੁਆਚਾ_fraction = lf_ਹੋਰ_Than_Half;
        ਨਹੀਂਤਾਂ ਜੇ (ਗੁਆਚਾ_fraction == lf_ਹੋਰ_Than_Half)
          ਗੁਆਚਾ_fraction = lfLessThanHalf;
      }
      ਹਾਸਲ = ਘਟਾਓ_Significand(ਆਰਜ਼ੀ_rhs, borrow);
    } ਨਹੀਂਤਾਂ { // cmpEqual
      zeroSignificand();
      ਜੇ (ਗੁਆਚਾ_fraction != lfExactlyZero && ਗੁਆਚਾ_fraction_is_from_rhs) {
        // rhs is slightly larger due to the lost fraction, flip the sign.
        sign = !sign;
      }
    }

    /* The code above is intended to ensure that no borrow is
       necessary.  */
    ਪ੍ਰਤਿਜ੍ਞਾ(!ਹਾਸਲ);
  } ਨਹੀਂਤਾਂ {
    ਜੇ (ਬਿੱਟ > 0) {
      IEEEFloat ਆਰਜ਼ੀ_rhs(rhs);

      ਗੁਆਚਾ_fraction = ਆਰਜ਼ੀ_rhs.ਖਿਸਕਾਓ_Significand_ਸੱਜਾ(ਬਿੱਟ);
      ਹਾਸਲ = ਜੋੜੋ_Significand(ਆਰਜ਼ੀ_rhs);
    } ਨਹੀਂਤਾਂ {
      ਗੁਆਚਾ_fraction = ਖਿਸਕਾਓ_Significand_ਸੱਜਾ(-ਬਿੱਟ);
      ਹਾਸਲ = ਜੋੜੋ_Significand(rhs);
    }

    /* We have a guard bit; generating a carry cannot happen.  */
    ਪ੍ਰਤਿਜ੍ਞਾ(!ਹਾਸਲ);
  }

  ਵਾਪਸ ਗੁਆਚਾ_fraction;
}

APFloat::op_ਸਥਿਤੀ IEEEFloat::multiplySpecials(ਅਚਲ IEEEFloat &rhs) {
  ਚੋਣ (Pack_Categories_Into_ਕੁੰਜੀ(category, rhs.category)) {
  ਮੂਲ:
    llvm_unreachable(ਖਾਲੀ_ਸੂਚਕ);

  ਹਾਲਤ Pack_Categories_Into_ਕੁੰਜੀ(fcZero, fcNaN):
  ਹਾਲਤ Pack_Categories_Into_ਕੁੰਜੀ(fc_ਸਾਧਾਰਨ, fcNaN):
  ਹਾਲਤ Pack_Categories_Into_ਕੁੰਜੀ(fcInfinity, fcNaN):
    ਸੌਂਪੋ(rhs);
    sign = ਝੂਠ;
    [[fallthrough]];
  ਹਾਲਤ Pack_Categories_Into_ਕੁੰਜੀ(fcNaN, fcZero):
  ਹਾਲਤ Pack_Categories_Into_ਕੁੰਜੀ(fcNaN, fc_ਸਾਧਾਰਨ):
  ਹਾਲਤ Pack_Categories_Into_ਕੁੰਜੀ(fcNaN, fcInfinity):
  ਹਾਲਤ Pack_Categories_Into_ਕੁੰਜੀ(fcNaN, fcNaN):
    sign ^= rhs.sign; // restore the original sign
    ਜੇ (isSignaling()) {
      makeQuiet();
      ਵਾਪਸ opInvalidOp;
    }
    ਵਾਪਸ rhs.isSignaling() ? opInvalidOp : opOK;

  ਹਾਲਤ Pack_Categories_Into_ਕੁੰਜੀ(fc_ਸਾਧਾਰਨ, fcInfinity):
  ਹਾਲਤ Pack_Categories_Into_ਕੁੰਜੀ(fcInfinity, fc_ਸਾਧਾਰਨ):
  ਹਾਲਤ Pack_Categories_Into_ਕੁੰਜੀ(fcInfinity, fcInfinity):
    category = fcInfinity;
    ਵਾਪਸ opOK;

  ਹਾਲਤ Pack_Categories_Into_ਕੁੰਜੀ(fcZero, fc_ਸਾਧਾਰਨ):
  ਹਾਲਤ Pack_Categories_Into_ਕੁੰਜੀ(fc_ਸਾਧਾਰਨ, fcZero):
  ਹਾਲਤ Pack_Categories_Into_ਕੁੰਜੀ(fcZero, fcZero):
    category = fcZero;
    ਵਾਪਸ opOK;

  ਹਾਲਤ Pack_Categories_Into_ਕੁੰਜੀ(fcZero, fcInfinity):
  ਹਾਲਤ Pack_Categories_Into_ਕੁੰਜੀ(fcInfinity, fcZero):
    makeNaN();
    ਵਾਪਸ opInvalidOp;

  ਹਾਲਤ Pack_Categories_Into_ਕੁੰਜੀ(fc_ਸਾਧਾਰਨ, fc_ਸਾਧਾਰਨ):
    ਵਾਪਸ opOK;
  }
}

APFloat::op_ਸਥਿਤੀ IEEEFloat::divideSpecials(ਅਚਲ IEEEFloat &rhs) {
  ਚੋਣ (Pack_Categories_Into_ਕੁੰਜੀ(category, rhs.category)) {
  ਮੂਲ:
    llvm_unreachable(ਖਾਲੀ_ਸੂਚਕ);

  ਹਾਲਤ Pack_Categories_Into_ਕੁੰਜੀ(fcZero, fcNaN):
  ਹਾਲਤ Pack_Categories_Into_ਕੁੰਜੀ(fc_ਸਾਧਾਰਨ, fcNaN):
  ਹਾਲਤ Pack_Categories_Into_ਕੁੰਜੀ(fcInfinity, fcNaN):
    ਸੌਂਪੋ(rhs);
    sign = ਝੂਠ;
    [[fallthrough]];
  ਹਾਲਤ Pack_Categories_Into_ਕੁੰਜੀ(fcNaN, fcZero):
  ਹਾਲਤ Pack_Categories_Into_ਕੁੰਜੀ(fcNaN, fc_ਸਾਧਾਰਨ):
  ਹਾਲਤ Pack_Categories_Into_ਕੁੰਜੀ(fcNaN, fcInfinity):
  ਹਾਲਤ Pack_Categories_Into_ਕੁੰਜੀ(fcNaN, fcNaN):
    sign ^= rhs.sign; // restore the original sign
    ਜੇ (isSignaling()) {
      makeQuiet();
      ਵਾਪਸ opInvalidOp;
    }
    ਵਾਪਸ rhs.isSignaling() ? opInvalidOp : opOK;

  ਹਾਲਤ Pack_Categories_Into_ਕੁੰਜੀ(fcInfinity, fcZero):
  ਹਾਲਤ Pack_Categories_Into_ਕੁੰਜੀ(fcInfinity, fc_ਸਾਧਾਰਨ):
  ਹਾਲਤ Pack_Categories_Into_ਕੁੰਜੀ(fcZero, fcInfinity):
  ਹਾਲਤ Pack_Categories_Into_ਕੁੰਜੀ(fcZero, fc_ਸਾਧਾਰਨ):
    ਵਾਪਸ opOK;

  ਹਾਲਤ Pack_Categories_Into_ਕੁੰਜੀ(fc_ਸਾਧਾਰਨ, fcInfinity):
    category = fcZero;
    ਵਾਪਸ opOK;

  ਹਾਲਤ Pack_Categories_Into_ਕੁੰਜੀ(fc_ਸਾਧਾਰਨ, fcZero):
    ਜੇ (semantics->nonFiniteBehavior == fltNonfiniteBehavior::NanOnly)
      makeNaN(ਝੂਠ, sign);
    ਨਹੀਂਤਾਂ
      category = fcInfinity;
    ਵਾਪਸ opDivByZero;

  ਹਾਲਤ Pack_Categories_Into_ਕੁੰਜੀ(fcInfinity, fcInfinity):
  ਹਾਲਤ Pack_Categories_Into_ਕੁੰਜੀ(fcZero, fcZero):
    makeNaN();
    ਵਾਪਸ opInvalidOp;

  ਹਾਲਤ Pack_Categories_Into_ਕੁੰਜੀ(fc_ਸਾਧਾਰਨ, fc_ਸਾਧਾਰਨ):
    ਵਾਪਸ opOK;
  }
}

APFloat::op_ਸਥਿਤੀ IEEEFloat::modSpecials(ਅਚਲ IEEEFloat &rhs) {
  ਚੋਣ (Pack_Categories_Into_ਕੁੰਜੀ(category, rhs.category)) {
  ਮੂਲ:
    llvm_unreachable(ਖਾਲੀ_ਸੂਚਕ);

  ਹਾਲਤ Pack_Categories_Into_ਕੁੰਜੀ(fcZero, fcNaN):
  ਹਾਲਤ Pack_Categories_Into_ਕੁੰਜੀ(fc_ਸਾਧਾਰਨ, fcNaN):
  ਹਾਲਤ Pack_Categories_Into_ਕੁੰਜੀ(fcInfinity, fcNaN):
    ਸੌਂਪੋ(rhs);
    [[fallthrough]];
  ਹਾਲਤ Pack_Categories_Into_ਕੁੰਜੀ(fcNaN, fcZero):
  ਹਾਲਤ Pack_Categories_Into_ਕੁੰਜੀ(fcNaN, fc_ਸਾਧਾਰਨ):
  ਹਾਲਤ Pack_Categories_Into_ਕੁੰਜੀ(fcNaN, fcInfinity):
  ਹਾਲਤ Pack_Categories_Into_ਕੁੰਜੀ(fcNaN, fcNaN):
    ਜੇ (isSignaling()) {
      makeQuiet();
      ਵਾਪਸ opInvalidOp;
    }
    ਵਾਪਸ rhs.isSignaling() ? opInvalidOp : opOK;

  ਹਾਲਤ Pack_Categories_Into_ਕੁੰਜੀ(fcZero, fcInfinity):
  ਹਾਲਤ Pack_Categories_Into_ਕੁੰਜੀ(fcZero, fc_ਸਾਧਾਰਨ):
  ਹਾਲਤ Pack_Categories_Into_ਕੁੰਜੀ(fc_ਸਾਧਾਰਨ, fcInfinity):
    ਵਾਪਸ opOK;

  ਹਾਲਤ Pack_Categories_Into_ਕੁੰਜੀ(fc_ਸਾਧਾਰਨ, fcZero):
  ਹਾਲਤ Pack_Categories_Into_ਕੁੰਜੀ(fcInfinity, fcZero):
  ਹਾਲਤ Pack_Categories_Into_ਕੁੰਜੀ(fcInfinity, fc_ਸਾਧਾਰਨ):
  ਹਾਲਤ Pack_Categories_Into_ਕੁੰਜੀ(fcInfinity, fcInfinity):
  ਹਾਲਤ Pack_Categories_Into_ਕੁੰਜੀ(fcZero, fcZero):
    makeNaN();
    ਵਾਪਸ opInvalidOp;

  ਹਾਲਤ Pack_Categories_Into_ਕੁੰਜੀ(fc_ਸਾਧਾਰਨ, fc_ਸਾਧਾਰਨ):
    ਵਾਪਸ opOK;
  }
}

APFloat::op_ਸਥਿਤੀ IEEEFloat::remainderSpecials(ਅਚਲ IEEEFloat &rhs) {
  ਚੋਣ (Pack_Categories_Into_ਕੁੰਜੀ(category, rhs.category)) {
  ਮੂਲ:
    llvm_unreachable(ਖਾਲੀ_ਸੂਚਕ);

  ਹਾਲਤ Pack_Categories_Into_ਕੁੰਜੀ(fcZero, fcNaN):
  ਹਾਲਤ Pack_Categories_Into_ਕੁੰਜੀ(fc_ਸਾਧਾਰਨ, fcNaN):
  ਹਾਲਤ Pack_Categories_Into_ਕੁੰਜੀ(fcInfinity, fcNaN):
    ਸੌਂਪੋ(rhs);
    [[fallthrough]];
  ਹਾਲਤ Pack_Categories_Into_ਕੁੰਜੀ(fcNaN, fcZero):
  ਹਾਲਤ Pack_Categories_Into_ਕੁੰਜੀ(fcNaN, fc_ਸਾਧਾਰਨ):
  ਹਾਲਤ Pack_Categories_Into_ਕੁੰਜੀ(fcNaN, fcInfinity):
  ਹਾਲਤ Pack_Categories_Into_ਕੁੰਜੀ(fcNaN, fcNaN):
    ਜੇ (isSignaling()) {
      makeQuiet();
      ਵਾਪਸ opInvalidOp;
    }
    ਵਾਪਸ rhs.isSignaling() ? opInvalidOp : opOK;

  ਹਾਲਤ Pack_Categories_Into_ਕੁੰਜੀ(fcZero, fcInfinity):
  ਹਾਲਤ Pack_Categories_Into_ਕੁੰਜੀ(fcZero, fc_ਸਾਧਾਰਨ):
  ਹਾਲਤ Pack_Categories_Into_ਕੁੰਜੀ(fc_ਸਾਧਾਰਨ, fcInfinity):
    ਵਾਪਸ opOK;

  ਹਾਲਤ Pack_Categories_Into_ਕੁੰਜੀ(fc_ਸਾਧਾਰਨ, fcZero):
  ਹਾਲਤ Pack_Categories_Into_ਕੁੰਜੀ(fcInfinity, fcZero):
  ਹਾਲਤ Pack_Categories_Into_ਕੁੰਜੀ(fcInfinity, fc_ਸਾਧਾਰਨ):
  ਹਾਲਤ Pack_Categories_Into_ਕੁੰਜੀ(fcInfinity, fcInfinity):
  ਹਾਲਤ Pack_Categories_Into_ਕੁੰਜੀ(fcZero, fcZero):
    makeNaN();
    ਵਾਪਸ opInvalidOp;

  ਹਾਲਤ Pack_Categories_Into_ਕੁੰਜੀ(fc_ਸਾਧਾਰਨ, fc_ਸਾਧਾਰਨ):
    ਵਾਪਸ opDivByZero; // fake status, indicating this is not a special case
  }
}

/* Change sign.  */
ਖਾਲੀ IEEEFloat::changeSign() {
  // With NaN-as-negative-zero, neither NaN or negative zero can change
  // their signs.
  ਜੇ (semantics->nanEncoding == fltNanEncoding::NegativeZero &&
      (isZero() || isNaN()))
    ਵਾਪਸ;
  /* Look mummy, this one's easy.  */
  sign = !sign;
}

/* Normalized addition or subtraction.  */
APFloat::op_ਸਥਿਤੀ IEEEFloat::ਜੋੜੋ_Or_ਘਟਾਓ(ਅਚਲ IEEEFloat &rhs,
                                           rounding_ਬਹੁਲਕ rounding_ਬਹੁਲਕ_2,
                                           ਤਾਰਕਿਕ ਘਟਾਓ) {
  op_ਸਥਿਤੀ fs = ਜੋੜੋ_Or_ਘਟਾਓ_Specials(rhs, ਘਟਾਓ);

  /* This return code means it was not a simple case.  */
  ਜੇ (fs == opDivByZero) {
    ਗੁਆਚਾ_Fraction ਗੁਆਚਾ_fraction;

    ਗੁਆਚਾ_fraction = ਜੋੜੋ_Or_ਘਟਾਓ_Significand(rhs, ਘਟਾਓ);
    fs = normalize(rounding_ਬਹੁਲਕ_2, ਗੁਆਚਾ_fraction);

    /* Can only be zero if we lost no fraction.  */
    ਪ੍ਰਤਿਜ੍ਞਾ(category != fcZero || ਗੁਆਚਾ_fraction == lfExactlyZero);
  }

  /* If two numbers add (exactly) to zero, IEEE 754 decrees it is a
     positive zero unless rounding to minus infinity, except that
     adding two like-signed zeroes gives that zero.  */
  ਜੇ (category == fcZero) {
    ਜੇ (rhs.category != fcZero || (sign == rhs.sign) == ਘਟਾਓ)
      sign = (rounding_ਬਹੁਲਕ_2 == rmTowardNegative);
    // NaN-in-negative-zero means zeros need to be normalized to +0.
    ਜੇ (semantics->nanEncoding == fltNanEncoding::NegativeZero)
      sign = ਝੂਠ;
  }

  ਵਾਪਸ fs;
}

/* Normalized addition.  */
APFloat::op_ਸਥਿਤੀ IEEEFloat::ਜੋੜੋ(ਅਚਲ IEEEFloat &rhs,
                                 rounding_ਬਹੁਲਕ rounding_ਬਹੁਲਕ_2) {
  ਵਾਪਸ ਜੋੜੋ_Or_ਘਟਾਓ(rhs, rounding_ਬਹੁਲਕ_2, ਝੂਠ);
}

/* Normalized subtraction.  */
APFloat::op_ਸਥਿਤੀ IEEEFloat::ਘਟਾਓ(ਅਚਲ IEEEFloat &rhs,
                                      rounding_ਬਹੁਲਕ rounding_ਬਹੁਲਕ_2) {
  ਵਾਪਸ ਜੋੜੋ_Or_ਘਟਾਓ(rhs, rounding_ਬਹੁਲਕ_2, ਸੱਚ);
}

/* Normalized multiply.  */
APFloat::op_ਸਥਿਤੀ IEEEFloat::multiply(ਅਚਲ IEEEFloat &rhs,
                                      rounding_ਬਹੁਲਕ rounding_ਬਹੁਲਕ_2) {
  sign ^= rhs.sign;
  op_ਸਥਿਤੀ fs = multiplySpecials(rhs);

  ਜੇ (isZero() && semantics->nanEncoding == fltNanEncoding::NegativeZero)
    sign = ਝੂਠ;
  ਜੇ (isFiniteNonZero()) {
    ਗੁਆਚਾ_Fraction ਗੁਆਚਾ_fraction = multiplySignificand(rhs);
    fs = normalize(rounding_ਬਹੁਲਕ_2, ਗੁਆਚਾ_fraction);
    ਜੇ (ਗੁਆਚਾ_fraction != lfExactlyZero)
      fs = (op_ਸਥਿਤੀ) (fs | opInexact);
  }

  ਵਾਪਸ fs;
}

/* Normalized divide.  */
APFloat::op_ਸਥਿਤੀ IEEEFloat::divide(ਅਚਲ IEEEFloat &rhs,
                                    rounding_ਬਹੁਲਕ rounding_ਬਹੁਲਕ_2) {
  sign ^= rhs.sign;
  op_ਸਥਿਤੀ fs = divideSpecials(rhs);

  ਜੇ (isZero() && semantics->nanEncoding == fltNanEncoding::NegativeZero)
    sign = ਝੂਠ;
  ਜੇ (isFiniteNonZero()) {
    ਗੁਆਚਾ_Fraction ਗੁਆਚਾ_fraction = divideSignificand(rhs);
    fs = normalize(rounding_ਬਹੁਲਕ_2, ਗੁਆਚਾ_fraction);
    ਜੇ (ਗੁਆਚਾ_fraction != lfExactlyZero)
      fs = (op_ਸਥਿਤੀ) (fs | opInexact);
  }

  ਵਾਪਸ fs;
}

/* Normalized remainder.  */
APFloat::op_ਸਥਿਤੀ IEEEFloat::remainder(ਅਚਲ IEEEFloat &rhs) {
  ਬਿਨਾਨਿਸ਼ਾਨੀ ਪੂਰਨ origSign = sign;

  // First handle the special cases.
  op_ਸਥਿਤੀ fs = remainderSpecials(rhs);
  ਜੇ (fs != opDivByZero)
    ਵਾਪਸ fs;

  fs = opOK;

  // Make sure the current value is less than twice the denom. If the addition
  // did not succeed (an overflow has happened), which means that the finite
  // value we currently posses must be less than twice the denom (as we are
  // using the same semantics).
  IEEEFloat P2 = rhs;
  ਜੇ (P2.ਜੋੜੋ(rhs, rm_ਸਭ_ਤੋਂ_ਨੇੜੇ_Ties_ਮੰਜ਼ਿਲ_Even) == opOK) {
    fs = mod(P2);
    ਪ੍ਰਤਿਜ੍ਞਾ(fs == opOK);
  }

  // Lets work with absolute numbers.
  IEEEFloat P = rhs;
  P.sign = ਝੂਠ;
  sign = ਝੂਠ;

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
  ਤਾਰਕਿਕ losesInfo;
  fltSemantics extendedSemantics = *semantics;
  extendedSemantics.maxExponent++;
  extendedSemantics.minExponent--;
  extendedSemantics.precision += 2;

  IEEEFloat VEx = *ਇਹ;
  fs = VEx.convert(extendedSemantics, rm_ਸਭ_ਤੋਂ_ਨੇੜੇ_Ties_ਮੰਜ਼ਿਲ_Even, &losesInfo);
  ਪ੍ਰਤਿਜ੍ਞਾ(fs == opOK && !losesInfo);
  IEEEFloat PEx = P;
  fs = PEx.convert(extendedSemantics, rm_ਸਭ_ਤੋਂ_ਨੇੜੇ_Ties_ਮੰਜ਼ਿਲ_Even, &losesInfo);
  ਪ੍ਰਤਿਜ੍ਞਾ(fs == opOK && !losesInfo);

  // It is simpler to work with 2x instead of 0.5p, and we do not need to lose
  // any fraction.
  fs = VEx.ਜੋੜੋ(VEx, rm_ਸਭ_ਤੋਂ_ਨੇੜੇ_Ties_ਮੰਜ਼ਿਲ_Even);
  ਪ੍ਰਤਿਜ੍ਞਾ(fs == opOK);

  ਜੇ (VEx.compare(PEx) == cmpGreaterThan) {
    fs = ਘਟਾਓ(P, rm_ਸਭ_ਤੋਂ_ਨੇੜੇ_Ties_ਮੰਜ਼ਿਲ_Even);
    ਪ੍ਰਤਿਜ੍ਞਾ(fs == opOK);

    // Make VEx = this.add(this), but because we have different semantics, we do
    // not want to `convert` again, so we just subtract PEx twice (which equals
    // to the desired value).
    fs = VEx.ਘਟਾਓ(PEx, rm_ਸਭ_ਤੋਂ_ਨੇੜੇ_Ties_ਮੰਜ਼ਿਲ_Even);
    ਪ੍ਰਤਿਜ੍ਞਾ(fs == opOK);
    fs = VEx.ਘਟਾਓ(PEx, rm_ਸਭ_ਤੋਂ_ਨੇੜੇ_Ties_ਮੰਜ਼ਿਲ_Even);
    ਪ੍ਰਤਿਜ੍ਞਾ(fs == opOK);

    cmp_ਨਤੀਜਾ ਨਤੀਜਾ_2 = VEx.compare(PEx);
    ਜੇ (ਨਤੀਜਾ_2 == cmpGreaterThan || ਨਤੀਜਾ_2 == cmpEqual) {
      fs = ਘਟਾਓ(P, rm_ਸਭ_ਤੋਂ_ਨੇੜੇ_Ties_ਮੰਜ਼ਿਲ_Even);
      ਪ੍ਰਤਿਜ੍ਞਾ(fs == opOK);
    }
  }

  ਜੇ (isZero()) {
    sign = origSign;    // IEEE754 requires this
    ਜੇ (semantics->nanEncoding == fltNanEncoding::NegativeZero)
      // But some 8-bit floats only have positive 0.
      sign = ਝੂਠ;
  } ਨਹੀਂਤਾਂ {
    sign ^= origSign;
  }
  ਵਾਪਸ fs;
}

/* Normalized llvm frem (C fmod). */
APFloat::op_ਸਥਿਤੀ IEEEFloat::mod(ਅਚਲ IEEEFloat &rhs) {
  op_ਸਥਿਤੀ fs = modSpecials(rhs);
  ਬਿਨਾਨਿਸ਼ਾਨੀ ਪੂਰਨ origSign = sign;

  ਜਦੋਂ (isFiniteNonZero() && rhs.isFiniteNonZero() &&
         compare_Absolute_ਮੁੱਲ(rhs) != cmpLessThan) {
    ਪੂਰਨ Exp = ilogb(*ਇਹ) - ilogb(rhs);
    IEEEFloat V = scalbn(rhs, Exp, rm_ਸਭ_ਤੋਂ_ਨੇੜੇ_Ties_ਮੰਜ਼ਿਲ_Even);
    // V can overflow to NaN with fltNonfiniteBehavior::NanOnly, so explicitly
    // check for it.
    ਜੇ (V.isNaN() || compare_Absolute_ਮੁੱਲ(V) == cmpLessThan)
      V = scalbn(rhs, Exp - 1, rm_ਸਭ_ਤੋਂ_ਨੇੜੇ_Ties_ਮੰਜ਼ਿਲ_Even);
    V.sign = sign;

    fs = ਘਟਾਓ(V, rm_ਸਭ_ਤੋਂ_ਨੇੜੇ_Ties_ਮੰਜ਼ਿਲ_Even);

    // When the semantics supports zero, this loop's
    // exit-condition is handled by the 'isFiniteNonZero'
    // category check above. However, when the semantics
    // does not have 'fcZero' and we have reached the
    // minimum possible value, (and any further subtract
    // will underflow to the same value) explicitly
    // provide an exit-path here.
    ਜੇ (!semantics->hasZero && ਇਹ->is_ਸਭ_ਤੋਂ_ਛੋਟਾ())
      ਰੋਕੋ;

    ਪ੍ਰਤਿਜ੍ਞਾ(fs==opOK);
  }
  ਜੇ (isZero()) {
    sign = origSign; // fmod requires this
    ਜੇ (semantics->nanEncoding == fltNanEncoding::NegativeZero)
      sign = ਝੂਠ;
  }
  ਵਾਪਸ fs;
}

/* Normalized fused-multiply-add.  */
APFloat::op_ਸਥਿਤੀ IEEEFloat::fused_Multiply_ਜੋੜੋ(ਅਚਲ IEEEFloat &multiplicand,
                                              ਅਚਲ IEEEFloat &addend,
                                              rounding_ਬਹੁਲਕ rounding_ਬਹੁਲਕ_2) {
  op_ਸਥਿਤੀ fs;

  /* Post-multiplication sign, before addition.  */
  sign ^= multiplicand.sign;

  /* If and only if all arguments are normal do we need to do an
     extended-precision calculation.  */
  ਜੇ (isFiniteNonZero() &&
      multiplicand.isFiniteNonZero() &&
      addend.isFinite()) {
    ਗੁਆਚਾ_Fraction ਗੁਆਚਾ_fraction;

    ਗੁਆਚਾ_fraction = multiplySignificand(multiplicand, addend);
    fs = normalize(rounding_ਬਹੁਲਕ_2, ਗੁਆਚਾ_fraction);
    ਜੇ (ਗੁਆਚਾ_fraction != lfExactlyZero)
      fs = (op_ਸਥਿਤੀ) (fs | opInexact);

    /* If two numbers add (exactly) to zero, IEEE 754 decrees it is a
       positive zero unless rounding to minus infinity, except that
       adding two like-signed zeroes gives that zero.  */
    ਜੇ (category == fcZero && !(fs & opUnderflow) && sign != addend.sign) {
      sign = (rounding_ਬਹੁਲਕ_2 == rmTowardNegative);
      ਜੇ (semantics->nanEncoding == fltNanEncoding::NegativeZero)
        sign = ਝੂਠ;
    }
  } ਨਹੀਂਤਾਂ {
    fs = multiplySpecials(multiplicand);

    /* FS can only be opOK or opInvalidOp.  There is no more work
       to do in the latter case.  The IEEE-754R standard says it is
       implementation-defined in this case whether, if ADDEND is a
       quiet NaN, we raise invalid op; this implementation does so.

       If we need to do the addition we can do so with normal
       precision.  */
    ਜੇ (fs == opOK)
      fs = ਜੋੜੋ_Or_ਘਟਾਓ(addend, rounding_ਬਹੁਲਕ_2, ਝੂਠ);
  }

  ਵਾਪਸ fs;
}

/* Rounding-mode correct round to integral value.  */
APFloat::op_ਸਥਿਤੀ IEEEFloat::ਗੇੜਾ_ਮੰਜ਼ਿਲ_Integral(rounding_ਬਹੁਲਕ rounding_ਬਹੁਲਕ_2) {
  ਜੇ (isInfinity())
    // [IEEE Std 754-2008 6.1]:
    // The behavior of infinity in floating-point arithmetic is derived from the
    // limiting cases of real arithmetic with operands of arbitrarily
    // large magnitude, when such a limit exists.
    // ...
    // Operations on infinite operands are usually exact and therefore signal no
    // exceptions ...
    ਵਾਪਸ opOK;

  ਜੇ (isNaN()) {
    ਜੇ (isSignaling()) {
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
      ਵਾਪਸ opInvalidOp;
    } ਨਹੀਂਤਾਂ {
      // [IEEE Std 754-2008 6.2]:
      // For an operation with quiet NaN inputs, other than maximum and minimum
      // operations, if a floating-point result is to be delivered the result
      // shall be a quiet NaN which should be one of the input NaNs.
      // ...
      // Every general-computational and quiet-computational operation involving
      // one or more input NaNs, none of them signaling, shall signal no
      // exception, except fusedMultiplyAdd might signal the invalid operation
      // exception(see 7.2).
      ਵਾਪਸ opOK;
    }
  }

  ਜੇ (isZero()) {
    // [IEEE Std 754-2008 6.3]:
    // ... the sign of the result of conversions, the quantize operation, the
    // roundToIntegral operations, and the roundToIntegralExact(see 5.3.1) is
    // the sign of the first or only operand.
    ਵਾਪਸ opOK;
  }

  // If the exponent is large enough, we know that this value is already
  // integral, and the arithmetic below would potentially cause it to saturate
  // to +/-Inf.  Bail out early instead.
  ਜੇ (exponent + 1 >= (ਪੂਰਨ)APFloat::semanticsPrecision(*semantics))
    ਵਾਪਸ opOK;

  // The algorithm here is quite simple: we add 2^(p-1), where p is the
  // precision of our format, and then subtract it back off again.  The choice
  // of rounding modes for the addition/subtraction determines the rounding mode
  // for our integral rounding as well.
  // NOTE: When the input value is negative, we do subtraction followed by
  // addition instead.
  APInt IntegerConstant(ਅਗਲਾ_Power_Of2(APFloat::semanticsPrecision(*semantics)),
                        1);
  IntegerConstant <<= APFloat::semanticsPrecision(*semantics) - 1;
  IEEEFloat MagicConstant(*semantics);
  op_ਸਥਿਤੀ fs = MagicConstant.convertFromAPInt(IntegerConstant, ਝੂਠ,
                                               rm_ਸਭ_ਤੋਂ_ਨੇੜੇ_Ties_ਮੰਜ਼ਿਲ_Even);
  ਪ੍ਰਤਿਜ੍ਞਾ(fs == opOK);
  MagicConstant.sign = sign;

  // Preserve the input sign so that we can handle the case of zero result
  // correctly.
  ਤਾਰਕਿਕ ਨਿਵੇਸ਼_Sign = isNegative();

  fs = ਜੋੜੋ(MagicConstant, rounding_ਬਹੁਲਕ_2);

  // Current value and 'MagicConstant' are both integers, so the result of the
  // subtraction is always exact according to Sterbenz' lemma.
  ਘਟਾਓ(MagicConstant, rounding_ਬਹੁਲਕ_2);

  // Restore the input sign.
  ਜੇ (ਨਿਵੇਸ਼_Sign != isNegative())
    changeSign();

  ਵਾਪਸ fs;
}

/* Comparison requires normalized numbers.  */
APFloat::cmp_ਨਤੀਜਾ IEEEFloat::compare(ਅਚਲ IEEEFloat &rhs) ਅਚਲ {
  ਪ੍ਰਤਿਜ੍ਞਾ(semantics == rhs.semantics);

  ਚੋਣ (Pack_Categories_Into_ਕੁੰਜੀ(category, rhs.category)) {
  ਮੂਲ:
    llvm_unreachable(ਖਾਲੀ_ਸੂਚਕ);

  ਹਾਲਤ Pack_Categories_Into_ਕੁੰਜੀ(fcNaN, fcZero):
  ਹਾਲਤ Pack_Categories_Into_ਕੁੰਜੀ(fcNaN, fc_ਸਾਧਾਰਨ):
  ਹਾਲਤ Pack_Categories_Into_ਕੁੰਜੀ(fcNaN, fcInfinity):
  ਹਾਲਤ Pack_Categories_Into_ਕੁੰਜੀ(fcNaN, fcNaN):
  ਹਾਲਤ Pack_Categories_Into_ਕੁੰਜੀ(fcZero, fcNaN):
  ਹਾਲਤ Pack_Categories_Into_ਕੁੰਜੀ(fc_ਸਾਧਾਰਨ, fcNaN):
  ਹਾਲਤ Pack_Categories_Into_ਕੁੰਜੀ(fcInfinity, fcNaN):
    ਵਾਪਸ cmpUnordered;

  ਹਾਲਤ Pack_Categories_Into_ਕੁੰਜੀ(fcInfinity, fc_ਸਾਧਾਰਨ):
  ਹਾਲਤ Pack_Categories_Into_ਕੁੰਜੀ(fcInfinity, fcZero):
  ਹਾਲਤ Pack_Categories_Into_ਕੁੰਜੀ(fc_ਸਾਧਾਰਨ, fcZero):
    ਜੇ (sign)
      ਵਾਪਸ cmpLessThan;
    ਨਹੀਂਤਾਂ
      ਵਾਪਸ cmpGreaterThan;

  ਹਾਲਤ Pack_Categories_Into_ਕੁੰਜੀ(fc_ਸਾਧਾਰਨ, fcInfinity):
  ਹਾਲਤ Pack_Categories_Into_ਕੁੰਜੀ(fcZero, fcInfinity):
  ਹਾਲਤ Pack_Categories_Into_ਕੁੰਜੀ(fcZero, fc_ਸਾਧਾਰਨ):
    ਜੇ (rhs.sign)
      ਵਾਪਸ cmpGreaterThan;
    ਨਹੀਂਤਾਂ
      ਵਾਪਸ cmpLessThan;

  ਹਾਲਤ Pack_Categories_Into_ਕੁੰਜੀ(fcInfinity, fcInfinity):
    ਜੇ (sign == rhs.sign)
      ਵਾਪਸ cmpEqual;
    ਨਹੀਂਤਾਂ ਜੇ (sign)
      ਵਾਪਸ cmpLessThan;
    ਨਹੀਂਤਾਂ
      ਵਾਪਸ cmpGreaterThan;

  ਹਾਲਤ Pack_Categories_Into_ਕੁੰਜੀ(fcZero, fcZero):
    ਵਾਪਸ cmpEqual;

  ਹਾਲਤ Pack_Categories_Into_ਕੁੰਜੀ(fc_ਸਾਧਾਰਨ, fc_ਸਾਧਾਰਨ):
    ਰੋਕੋ;
  }

  cmp_ਨਤੀਜਾ ਨਤੀਜਾ_2;
  /* Two normal numbers.  Do they have the same sign?  */
  ਜੇ (sign != rhs.sign) {
    ਜੇ (sign)
      ਨਤੀਜਾ_2 = cmpLessThan;
    ਨਹੀਂਤਾਂ
      ਨਤੀਜਾ_2 = cmpGreaterThan;
  } ਨਹੀਂਤਾਂ {
    /* Compare absolute values; invert result if negative.  */
    ਨਤੀਜਾ_2 = compare_Absolute_ਮੁੱਲ(rhs);

    ਜੇ (sign) {
      ਜੇ (ਨਤੀਜਾ_2 == cmpLessThan)
        ਨਤੀਜਾ_2 = cmpGreaterThan;
      ਨਹੀਂਤਾਂ ਜੇ (ਨਤੀਜਾ_2 == cmpGreaterThan)
        ਨਤੀਜਾ_2 = cmpLessThan;
    }
  }

  ਵਾਪਸ ਨਤੀਜਾ_2;
}

/// IEEEFloat::convert - convert a value of one floating point type to another.
/// The return value corresponds to the IEEE754 exceptions.  *losesInfo
/// records whether the transformation lost information, i.e. whether
/// converting the result back to the original type will produce the
/// original value (this is almost the same as return value==fsOK, but there
/// are edge cases where this is not so).

APFloat::op_ਸਥਿਤੀ IEEEFloat::convert(ਅਚਲ fltSemantics &ਮੰਜ਼ਿਲ_Semantics,
                                     rounding_ਬਹੁਲਕ rounding_ਬਹੁਲਕ_2,
                                     ਤਾਰਕਿਕ *losesInfo) {
  op_ਸਥਿਤੀ fs;
  ਅਚਲ fltSemantics &fromSemantics = *semantics;
  ਤਾਰਕਿਕ is_signaling = isSignaling();

  ਗੁਆਚਾ_Fraction ਗੁਆਚਾ_Fraction = lfExactlyZero;
  ਬਿਨਾਨਿਸ਼ਾਨੀ new_Part_ਗਿਣਤੀ = part_ਗਿਣਤੀ_For_ਬਿੱਟ(ਮੰਜ਼ਿਲ_Semantics.precision + 1);
  ਬਿਨਾਨਿਸ਼ਾਨੀ old_Part_ਗਿਣਤੀ = part_ਗਿਣਤੀ();
  ਪੂਰਨ ਖਿਸਕਾਓ = ਮੰਜ਼ਿਲ_Semantics.precision - fromSemantics.precision;

  ਤਾਰਕਿਕ X86SpecialNan = ਝੂਠ;
  ਜੇ (&fromSemantics == &APFloat_ਅਧਾਰ::semX87DoubleExtended &&
      &ਮੰਜ਼ਿਲ_Semantics != &APFloat_ਅਧਾਰ::semX87DoubleExtended && category == fcNaN &&
      (!(*significandParts() & 0x8000000000000000ULL) ||
       !(*significandParts() & 0x4000000000000000ULL))) {
    // x86 has some unusual NaNs which cannot be represented in any other
    // format; note them here.
    X86SpecialNan = ਸੱਚ;
  }

  // If this is a truncation of a denormal number, and the target semantics
  // has larger exponent range than the source semantics (this can happen
  // when truncating from PowerPC double-double to double format), the
  // right shift could lose result mantissa bits.  Adjust exponent instead
  // of performing excessive shift.
  // Also do a similar trick in case shifting denormal would produce zero
  // significand as this case isn't handled correctly by normalize.
  ਜੇ (ਖਿਸਕਾਓ < 0 && isFiniteNonZero()) {
    ਪੂਰਨ omsb = significandMSB() + 1;
    ਪੂਰਨ exponentChange = omsb - fromSemantics.precision;
    ਜੇ (exponent + exponentChange < ਮੰਜ਼ਿਲ_Semantics.minExponent)
      exponentChange = ਮੰਜ਼ਿਲ_Semantics.minExponent - exponent;
    exponentChange = std::max(exponentChange, ਖਿਸਕਾਓ);
    ਜੇ (exponentChange < 0) {
      ਖਿਸਕਾਓ -= exponentChange;
      exponent += exponentChange;
    } ਨਹੀਂਤਾਂ ਜੇ (omsb <= -ਖਿਸਕਾਓ) {
      exponentChange = omsb + ਖਿਸਕਾਓ - 1; // leave at least one bit set
      ਖਿਸਕਾਓ -= exponentChange;
      exponent += exponentChange;
    }
  }

  // If this is a truncation, perform the shift before we narrow the storage.
  ਜੇ (ਖਿਸਕਾਓ < 0 && (isFiniteNonZero() ||
                    (category == fcNaN && semantics->nonFiniteBehavior !=
                                              fltNonfiniteBehavior::NanOnly)))
    ਗੁਆਚਾ_Fraction = ਖਿਸਕਾਓ_ਸੱਜਾ(significandParts(), old_Part_ਗਿਣਤੀ, -ਖਿਸਕਾਓ);

  // Fix the storage so it can hold to new value.
  ਜੇ (new_Part_ਗਿਣਤੀ > old_Part_ਗਿਣਤੀ) {
    // The new type requires more storage; make it available.
    integerPart *newParts;
    newParts = ਨਵਾਂ integerPart[new_Part_ਗਿਣਤੀ];
    APInt::tc_ਸੈੱਟ(newParts, 0, new_Part_ਗਿਣਤੀ);
    ਜੇ (isFiniteNonZero() || category==fcNaN)
      APInt::tc_ਸੌਂਪੋ(newParts, significandParts(), old_Part_ਗਿਣਤੀ);
    freeSignificand();
    significand.parts = newParts;
  } ਨਹੀਂਤਾਂ ਜੇ (new_Part_ਗਿਣਤੀ == 1 && old_Part_ਗਿਣਤੀ != 1) {
    // Switch to built-in storage for a single part.
    integerPart newPart = 0;
    ਜੇ (isFiniteNonZero() || category==fcNaN)
      newPart = significandParts()[0];
    freeSignificand();
    significand.part = newPart;
  }

  // Now that we have the right storage, switch the semantics.
  semantics = &ਮੰਜ਼ਿਲ_Semantics;

  // If this is an extension, perform the shift now that the storage is
  // available.
  ਜੇ (ਖਿਸਕਾਓ > 0 && (isFiniteNonZero() || category==fcNaN))
    APInt::tc_ਖਿਸਕਾਓ_ਖੱਬਾ(significandParts(), new_Part_ਗਿਣਤੀ, ਖਿਸਕਾਓ);

  ਜੇ (isFiniteNonZero()) {
    fs = normalize(rounding_ਬਹੁਲਕ_2, ਗੁਆਚਾ_Fraction);
    *losesInfo = (fs != opOK);
  } ਨਹੀਂਤਾਂ ਜੇ (category == fcNaN) {
    ਜੇ (semantics->nonFiniteBehavior == fltNonfiniteBehavior::NanOnly) {
      *losesInfo =
          fromSemantics.nonFiniteBehavior != fltNonfiniteBehavior::NanOnly;
      makeNaN(ਝੂਠ, sign);
      fs = is_signaling ? opInvalidOp : opOK;
    } ਨਹੀਂਤਾਂ {
      // If NaN is negative zero, we need to create a new NaN to avoid
      // converting NaN to -Inf.
      ਜੇ (fromSemantics.nanEncoding == fltNanEncoding::NegativeZero &&
          semantics->nanEncoding != fltNanEncoding::NegativeZero)
        makeNaN(ਝੂਠ, ਝੂਠ);

      // If the source has no significand, there are no payload bits to carry
      // over, and an all-zero significand would encode an Inf. Create a new
      // NaN.
      ਜੇ (!APFloat::hasSignificand(fromSemantics))
        makeNaN(ਝੂਠ, sign);

      *losesInfo = ਗੁਆਚਾ_Fraction != lfExactlyZero || X86SpecialNan;

      // For x87 extended precision, we want to make a NaN, not a special NaN
      // if the input wasn't special either.
      ਜੇ (!X86SpecialNan && semantics == &APFloat_ਅਧਾਰ::semX87DoubleExtended)
        APInt::tc_ਸੈੱਟ_ਦੋਆਧਾਰੀ_ਅੰਕ(significandParts(), semantics->precision - 1);

      // Convert of sNaN creates qNaN and raises an exception (invalid op).
      // This also guarantees that a sNaN does not become Inf on a truncation
      // that loses all payload bits.
      ਜੇ (is_signaling) {
        makeQuiet();
        fs = opInvalidOp;
      } ਨਹੀਂਤਾਂ {
        fs = opOK;
      }
    }
  } ਨਹੀਂਤਾਂ ਜੇ (category == fcInfinity &&
             semantics->nonFiniteBehavior == fltNonfiniteBehavior::NanOnly) {
    makeNaN(ਝੂਠ, sign);
    *losesInfo = ਸੱਚ;
    fs = opInexact;
  } ਨਹੀਂਤਾਂ ਜੇ (category == fcZero &&
             semantics->nanEncoding == fltNanEncoding::NegativeZero) {
    // Negative zero loses info, but positive zero doesn't.
    *losesInfo =
        fromSemantics.nanEncoding != fltNanEncoding::NegativeZero && sign;
    fs = *losesInfo ? opInexact : opOK;
    // NaN is negative zero means -0 -> +0, which can lose information
    sign = ਝੂਠ;
  } ਨਹੀਂਤਾਂ {
    *losesInfo = ਝੂਠ;
    fs = opOK;
  }

  // The target may have no encoding for a negative value, or none for zero.
  // The paths above only report what rounding lost, so report these here too:
  // a caller that checks losesInfo would otherwise accept a result the target
  // cannot represent, and printing that result asserts.
  ਜੇ ((sign && !semantics->hasSignedRepr) ||
      (category == fcZero && !semantics->hasZero)) {
    *losesInfo = ਸੱਚ;
    ਜੇ (fs == opOK)
      fs = opInexact;
  }

  ਜੇ (category == fcZero && !semantics->hasZero)
    make_ਸਭ_ਤੋਂ_ਛੋਟਾ_Normalized(ਝੂਠ);
  ਵਾਪਸ fs;
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
APFloat::op_ਸਥਿਤੀ IEEEFloat::convert_ਮੰਜ਼ਿਲ_Sign_Extended_Integer(
    MutableArrayRef<integerPart> parts, ਬਿਨਾਨਿਸ਼ਾਨੀ ਪੂਰਨ ਚੌੜਾਈ_2, ਤਾਰਕਿਕ isSigned,
    rounding_ਬਹੁਲਕ rounding_ਬਹੁਲਕ_2, ਤਾਰਕਿਕ *isExact) ਅਚਲ {
  *isExact = ਝੂਠ;

  /* Handle the three special cases first.  */
  ਜੇ (category == fcInfinity || category == fcNaN)
    ਵਾਪਸ opInvalidOp;

  ਬਿਨਾਨਿਸ਼ਾਨੀ dst_Parts_ਗਿਣਤੀ = part_ਗਿਣਤੀ_For_ਬਿੱਟ(ਚੌੜਾਈ_2);
  ਪ੍ਰਤਿਜ੍ਞਾ(dst_Parts_ਗਿਣਤੀ <= parts.ਮਾਪ_2() && "Integer too big");

  ਜੇ (category == fcZero) {
    APInt::tc_ਸੈੱਟ(parts.ਅੰਕੜੇ(), 0, dst_Parts_ਗਿਣਤੀ);
    // Negative zero can't be represented as an int.
    *isExact = !sign;
    ਵਾਪਸ opOK;
  }

  ਅਚਲ integerPart *src = significandParts();

  ਬਿਨਾਨਿਸ਼ਾਨੀ truncated_ਬਿੱਟ;
  /* Step 1: place our absolute value, with any fraction truncated, in
     the destination.  */
  ਜੇ (exponent < 0) {
    /* Our absolute value is less than one; truncate everything.  */
    APInt::tc_ਸੈੱਟ(parts.ਅੰਕੜੇ(), 0, dst_Parts_ਗਿਣਤੀ);
    /* For exponent -1 the integer bit represents .5, look at that.
       For smaller exponents leftmost truncated bit is 0. */
    truncated_ਬਿੱਟ = semantics->precision -1U - exponent;
  } ਨਹੀਂਤਾਂ {
    /* We want the most significant (exponent + 1) bits; the rest are
       truncated.  */
    ਬਿਨਾਨਿਸ਼ਾਨੀ ਪੂਰਨ ਬਿੱਟ = exponent + 1U;

    /* Hopelessly large in magnitude?  */
    ਜੇ (ਬਿੱਟ > ਚੌੜਾਈ_2)
      ਵਾਪਸ opInvalidOp;

    ਜੇ (ਬਿੱਟ < semantics->precision) {
      /* We truncate (semantics->precision - bits) bits.  */
      truncated_ਬਿੱਟ = semantics->precision - ਬਿੱਟ;
      APInt::tc_ਕੱਢੋ(parts.ਅੰਕੜੇ(), dst_Parts_ਗਿਣਤੀ, src, ਬਿੱਟ, truncated_ਬਿੱਟ);
    } ਨਹੀਂਤਾਂ {
      /* We want at least as many bits as are available.  */
      APInt::tc_ਕੱਢੋ(parts.ਅੰਕੜੇ(), dst_Parts_ਗਿਣਤੀ, src, semantics->precision,
                       0);
      APInt::tc_ਖਿਸਕਾਓ_ਖੱਬਾ(parts.ਅੰਕੜੇ(), dst_Parts_ਗਿਣਤੀ,
                         ਬਿੱਟ - semantics->precision);
      truncated_ਬਿੱਟ = 0;
    }
  }

  /* Step 2: work out any lost fraction, and increment the absolute
     value if we would round away from zero.  */
  ਗੁਆਚਾ_Fraction ਗੁਆਚਾ_fraction;
  ਜੇ (truncated_ਬਿੱਟ) {
    ਗੁਆਚਾ_fraction = ਗੁਆਚਾ_Fraction_Through_Truncation(src, part_ਗਿਣਤੀ(),
                                                  truncated_ਬਿੱਟ);
    ਜੇ (ਗੁਆਚਾ_fraction != lfExactlyZero &&
        ਗੇੜਾ_Away_From_Zero(rounding_ਬਹੁਲਕ_2, ਗੁਆਚਾ_fraction, truncated_ਬਿੱਟ)) {
      ਜੇ (APInt::tcIncrement(parts.ਅੰਕੜੇ(), dst_Parts_ਗਿਣਤੀ))
        ਵਾਪਸ opInvalidOp;     /* Overflow.  */
    }
  } ਨਹੀਂਤਾਂ {
    ਗੁਆਚਾ_fraction = lfExactlyZero;
  }

  /* Step 3: check if we fit in the destination.  */
  ਬਿਨਾਨਿਸ਼ਾਨੀ ਪੂਰਨ omsb = APInt::tcMSB(parts.ਅੰਕੜੇ(), dst_Parts_ਗਿਣਤੀ) + 1;

  ਜੇ (sign) {
    ਜੇ (!isSigned) {
      /* Negative numbers cannot be represented as unsigned.  */
      ਜੇ (omsb != 0)
        ਵਾਪਸ opInvalidOp;
    } ਨਹੀਂਤਾਂ {
      /* It takes omsb bits to represent the unsigned integer value.
         We lose a bit for the sign, but care is needed as the
         maximally negative integer is a special case.  */
      ਜੇ (omsb == ਚੌੜਾਈ_2 &&
          APInt::tcLSB(parts.ਅੰਕੜੇ(), dst_Parts_ਗਿਣਤੀ) + 1 != omsb)
        ਵਾਪਸ opInvalidOp;

      /* This case can happen because of rounding.  */
      ਜੇ (omsb > ਚੌੜਾਈ_2)
        ਵਾਪਸ opInvalidOp;
    }

    APInt::tcNegate (parts.ਅੰਕੜੇ(), dst_Parts_ਗਿਣਤੀ);
  } ਨਹੀਂਤਾਂ {
    ਜੇ (omsb >= ਚੌੜਾਈ_2 + !isSigned)
      ਵਾਪਸ opInvalidOp;
  }

  ਜੇ (ਗੁਆਚਾ_fraction == lfExactlyZero) {
    *isExact = ਸੱਚ;
    ਵਾਪਸ opOK;
  }
  ਵਾਪਸ opInexact;
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
APFloat::op_ਸਥਿਤੀ
IEEEFloat::convert_ਮੰਜ਼ਿਲ_Integer(MutableArrayRef<integerPart> parts,
                            ਬਿਨਾਨਿਸ਼ਾਨੀ ਪੂਰਨ ਚੌੜਾਈ_2, ਤਾਰਕਿਕ isSigned,
                            rounding_ਬਹੁਲਕ rounding_ਬਹੁਲਕ_2, ਤਾਰਕਿਕ *isExact) ਅਚਲ {
  op_ਸਥਿਤੀ fs = convert_ਮੰਜ਼ਿਲ_Sign_Extended_Integer(parts, ਚੌੜਾਈ_2, isSigned,
                                             rounding_ਬਹੁਲਕ_2, isExact);

  ਜੇ (fs == opInvalidOp) {
    ਬਿਨਾਨਿਸ਼ਾਨੀ ਪੂਰਨ ਬਿੱਟ, dst_Parts_ਗਿਣਤੀ;

    dst_Parts_ਗਿਣਤੀ = part_ਗਿਣਤੀ_For_ਬਿੱਟ(ਚੌੜਾਈ_2);
    ਪ੍ਰਤਿਜ੍ਞਾ(dst_Parts_ਗਿਣਤੀ <= parts.ਮਾਪ_2() && "Integer too big");

    ਜੇ (category == fcNaN)
      ਬਿੱਟ = 0;
    ਨਹੀਂਤਾਂ ਜੇ (sign)
      ਬਿੱਟ = isSigned;
    ਨਹੀਂਤਾਂ
      ਬਿੱਟ = ਚੌੜਾਈ_2 - isSigned;

    tc_ਸੈੱਟ_ਘੱਟ_ਤੋਂ_ਘੱਟ_Significant_ਬਿੱਟ(parts.ਅੰਕੜੇ(), dst_Parts_ਗਿਣਤੀ, ਬਿੱਟ);
    ਜੇ (sign && isSigned)
      APInt::tc_ਖਿਸਕਾਓ_ਖੱਬਾ(parts.ਅੰਕੜੇ(), dst_Parts_ਗਿਣਤੀ, ਚੌੜਾਈ_2 - 1);
  }

  ਵਾਪਸ fs;
}

/* Convert an unsigned integer SRC to a floating point number,
   rounding according to ROUNDING_MODE.  The sign of the floating
   point number is not modified.  */
APFloat::op_ਸਥਿਤੀ IEEEFloat::convertFromUnsignedParts(
    ਅਚਲ integerPart *src, ਬਿਨਾਨਿਸ਼ਾਨੀ ਪੂਰਨ src_ਗਿਣਤੀ, rounding_ਬਹੁਲਕ rounding_ਬਹੁਲਕ_2) {
  category = fc_ਸਾਧਾਰਨ;
  ਬਿਨਾਨਿਸ਼ਾਨੀ omsb = APInt::tcMSB(src, src_ਗਿਣਤੀ) + 1;
  integerPart *dst = significandParts();
  ਬਿਨਾਨਿਸ਼ਾਨੀ dst_ਗਿਣਤੀ = part_ਗਿਣਤੀ();
  ਬਿਨਾਨਿਸ਼ਾਨੀ precision = semantics->precision;

  /* We want the most significant PRECISION bits of SRC.  There may not
     be that many; extract what we can.  */
  ਗੁਆਚਾ_Fraction ਗੁਆਚਾ_fraction;
  ਜੇ (precision <= omsb) {
    exponent = omsb - 1;
    ਗੁਆਚਾ_fraction = ਗੁਆਚਾ_Fraction_Through_Truncation(src, src_ਗਿਣਤੀ,
                                                  omsb - precision);
    APInt::tc_ਕੱਢੋ(dst, dst_ਗਿਣਤੀ, src, precision, omsb - precision);
  } ਨਹੀਂਤਾਂ {
    exponent = precision - 1;
    ਗੁਆਚਾ_fraction = lfExactlyZero;
    APInt::tc_ਕੱਢੋ(dst, dst_ਗਿਣਤੀ, src, omsb, 0);
  }

  ਵਾਪਸ normalize(rounding_ਬਹੁਲਕ_2, ਗੁਆਚਾ_fraction);
}

APFloat::op_ਸਥਿਤੀ IEEEFloat::convertFromAPInt(ਅਚਲ APInt &Val, ਤਾਰਕਿਕ isSigned,
                                              rounding_ਬਹੁਲਕ rounding_ਬਹੁਲਕ_2) {
  ਬਿਨਾਨਿਸ਼ਾਨੀ ਪੂਰਨ part_ਗਿਣਤੀ = Val.get_ਅੰਕ_ਸ਼ਬਦ();
  APInt api = Val;

  sign = ਝੂਠ;
  ਜੇ (isSigned && api.isNegative()) {
    sign = ਸੱਚ;
    api = -api;
  }

  ਵਾਪਸ convertFromUnsignedParts(api.get_Raw_ਅੰਕੜੇ(), part_ਗਿਣਤੀ, rounding_ਬਹੁਲਕ_2);
}

Expected<APFloat::op_ਸਥਿਤੀ>
IEEEFloat::convertFromHexadecimalString(StringRef s,
                                        rounding_ਬਹੁਲਕ rounding_ਬਹੁਲਕ_2) {
  ਗੁਆਚਾ_Fraction ਗੁਆਚਾ_fraction = lfExactlyZero;

  category = fc_ਸਾਧਾਰਨ;
  zeroSignificand();
  exponent = 0;

  integerPart *significand = significandParts();
  ਬਿਨਾਨਿਸ਼ਾਨੀ parts_ਗਿਣਤੀ = part_ਗਿਣਤੀ();
  ਬਿਨਾਨਿਸ਼ਾਨੀ ਦੋਆਧਾਰੀ_ਅੰਕ_Pos = parts_ਗਿਣਤੀ * integer_Part_ਚੌੜਾਈ;
  ਤਾਰਕਿਕ computedTrailingFraction = ਝੂਠ;

  // Skip leading zeroes and any (hexa)decimal point.
  StringRef::iterator begin = s.begin();
  StringRef::iterator ਸਿਰਾ = s.ਸਿਰਾ();
  StringRef::iterator dot;
  ਸਵੈਚਾਲਿਤ PtrOrErr = skipLeadingZeroesAndAnyDot(begin, ਸਿਰਾ, &dot);
  ਜੇ (!PtrOrErr)
    ਵਾਪਸ PtrOrErr.takeError();
  StringRef::iterator p = *PtrOrErr;
  StringRef::iterator ਪਹਿਲਾ_Significant_Digit = p;

  ਜਦੋਂ (p != ਸਿਰਾ) {
    integerPart hex_ਮੁੱਲ;

    ਜੇ (*p == '.') {
      ਜੇ (dot != ਸਿਰਾ)
        ਵਾਪਸ createError("String contains multiple dots");
      dot = p++;
      ਜਾਰੀ;
    }

    hex_ਮੁੱਲ = hex_Digit_ਮੁੱਲ(*p);
    ਜੇ (hex_ਮੁੱਲ == UINT_MAX)
      ਰੋਕੋ;

    p++;

    // Store the number while we have space.
    ਜੇ (ਦੋਆਧਾਰੀ_ਅੰਕ_Pos) {
      ਦੋਆਧਾਰੀ_ਅੰਕ_Pos -= 4;
      hex_ਮੁੱਲ <<= ਦੋਆਧਾਰੀ_ਅੰਕ_Pos % integer_Part_ਚੌੜਾਈ;
      significand[ਦੋਆਧਾਰੀ_ਅੰਕ_Pos / integer_Part_ਚੌੜਾਈ] |= hex_ਮੁੱਲ;
    } ਨਹੀਂਤਾਂ ਜੇ (!computedTrailingFraction) {
      ਸਵੈਚਾਲਿਤ FractOrErr = trailingHexadecimalFraction(p, ਸਿਰਾ, hex_ਮੁੱਲ);
      ਜੇ (!FractOrErr)
        ਵਾਪਸ FractOrErr.takeError();
      ਗੁਆਚਾ_fraction = *FractOrErr;
      computedTrailingFraction = ਸੱਚ;
    }
  }

  /* Hex floats require an exponent but not a hexadecimal point.  */
  ਜੇ (p == ਸਿਰਾ)
    ਵਾਪਸ createError("Hex strings require an exponent");
  ਜੇ (*p != 'p' && *p != 'P')
    ਵਾਪਸ createError("Invalid character in significand");
  ਜੇ (p == begin)
    ਵਾਪਸ createError("Significand has no digits");
  ਜੇ (dot != ਸਿਰਾ && p - begin == 1)
    ਵਾਪਸ createError("Significand has no digits");

  /* Ignore the exponent if we are zero.  */
  ਜੇ (p != ਪਹਿਲਾ_Significant_Digit) {
    ਪੂਰਨ expAdjustment;

    /* Implicit hexadecimal point?  */
    ਜੇ (dot == ਸਿਰਾ)
      dot = p;

    /* Calculate the exponent adjustment implicit in the number of
       significant digits.  */
    expAdjustment = static_cast<ਪੂਰਨ>(dot - ਪਹਿਲਾ_Significant_Digit);
    ਜੇ (expAdjustment < 0)
      expAdjustment++;
    expAdjustment = expAdjustment * 4 - 1;

    /* Adjust for writing the significand starting at the most
       significant nibble.  */
    expAdjustment += semantics->precision;
    expAdjustment -= parts_ਗਿਣਤੀ * integer_Part_ਚੌੜਾਈ;

    /* Adjust for the given exponent.  */
    ਸਵੈਚਾਲਿਤ ExpOrErr = ਕੁੱਲ_Exponent(p + 1, ਸਿਰਾ, expAdjustment);
    ਜੇ (!ExpOrErr)
      ਵਾਪਸ ExpOrErr.takeError();
    exponent = *ExpOrErr;
  }

  ਵਾਪਸ normalize(rounding_ਬਹੁਲਕ_2, ਗੁਆਚਾ_fraction);
}

APFloat::op_ਸਥਿਤੀ
IEEEFloat::ਗੇੜਾ_Significand_With_Exponent(ਅਚਲ integerPart *decSigParts,
                                        ਬਿਨਾਨਿਸ਼ਾਨੀ sig_Part_ਗਿਣਤੀ, ਪੂਰਨ ਘਾਤਾਂਕ,
                                        rounding_ਬਹੁਲਕ rounding_ਬਹੁਲਕ_2) {
  fltSemantics calcSemantics = { 32767, -32767, 0, 0 };
  ਬਿਨਾਨਿਸ਼ਾਨੀ ਪੂਰਨ power = ਘਾਤਾਂਕ >= 0 ? ਘਾਤਾਂਕ : -ਘਾਤਾਂਕ;
  ਬਿਨਾਨਿਸ਼ਾਨੀ ਪੂਰਨ pow5_Part_ਸਮਰੱਥਾ = power_ਦਾ_Five_Parts_For_Exponent(power);
  Small_ਸਦਿਸ਼<integerPart, ਅੰਕ_Pow5_Parts_On_ਢੇਰ> pow5Parts;
  pow5Parts.resize_for_overwrite(pow5_Part_ਸਮਰੱਥਾ);

  ਤਾਰਕਿਕ is_ਸਭ_ਤੋਂ_ਨੇੜੇ = rounding_ਬਹੁਲਕ_2 == rm_ਸਭ_ਤੋਂ_ਨੇੜੇ_Ties_ਮੰਜ਼ਿਲ_Even ||
                   rounding_ਬਹੁਲਕ_2 == rm_ਸਭ_ਤੋਂ_ਨੇੜੇ_Ties_ਮੰਜ਼ਿਲ_Away;

  ਬਿਨਾਨਿਸ਼ਾਨੀ parts = part_ਗਿਣਤੀ_For_ਬਿੱਟ(semantics->precision + 11);

  /* Calculate pow(5, abs(exp)).  */
  ਬਿਨਾਨਿਸ਼ਾਨੀ pow5_Part_ਗਿਣਤੀ = powerOf5(pow5Parts.ਅੰਕੜੇ(), power);

  ਲਈ (;; parts *= 2) {
    ਬਿਨਾਨਿਸ਼ਾਨੀ ਪੂਰਨ excessPrecision, truncated_ਬਿੱਟ;

    calcSemantics.precision = parts * integer_Part_ਚੌੜਾਈ - 1;
    excessPrecision = calcSemantics.precision - semantics->precision;
    truncated_ਬਿੱਟ = excessPrecision;

    IEEEFloat decSig(calcSemantics, uninitialized);
    decSig.makeZero(sign);
    IEEEFloat pow5(calcSemantics);

    op_ਸਥਿਤੀ sig_ਸਥਿਤੀ = decSig.convertFromUnsignedParts(
        decSigParts, sig_Part_ਗਿਣਤੀ, rm_ਸਭ_ਤੋਂ_ਨੇੜੇ_Ties_ਮੰਜ਼ਿਲ_Even);
    op_ਸਥਿਤੀ pow_ਸਥਿਤੀ = pow5.convertFromUnsignedParts(
        pow5Parts.ਅੰਕੜੇ(), pow5_Part_ਗਿਣਤੀ, rm_ਸਭ_ਤੋਂ_ਨੇੜੇ_Ties_ਮੰਜ਼ਿਲ_Even);
    /* Add exp, as 10^n = 5^n * 2^n.  */
    decSig.exponent += ਘਾਤਾਂਕ;

    ਗੁਆਚਾ_Fraction calc_ਗੁਆਚਾ_Fraction;
    integerPart HUerr, HUdistance;
    ਬਿਨਾਨਿਸ਼ਾਨੀ ਪੂਰਨ powHUerr;

    ਜੇ (ਘਾਤਾਂਕ >= 0) {
      /* multiplySignificand leaves the precision-th bit set to 1.  */
      calc_ਗੁਆਚਾ_Fraction = decSig.multiplySignificand(pow5);
      powHUerr = pow_ਸਥਿਤੀ != opOK;
    } ਨਹੀਂਤਾਂ {
      calc_ਗੁਆਚਾ_Fraction = decSig.divideSignificand(pow5);
      /* Denormal numbers have less precision.  */
      ਜੇ (decSig.exponent < semantics->minExponent) {
        excessPrecision += (semantics->minExponent - decSig.exponent);
        truncated_ਬਿੱਟ = excessPrecision;
        excessPrecision = std::min(excessPrecision, calcSemantics.precision);
      }
      /* Extra half-ulp lost in reciprocal of exponent.  */
      powHUerr = (pow_ਸਥਿਤੀ == opOK && calc_ਗੁਆਚਾ_Fraction == lfExactlyZero) ? 0:2;
    }

    /* Both multiplySignificand and divideSignificand return the
       result with the integer bit set.  */
    ਪ੍ਰਤਿਜ੍ਞਾ(APInt::tc_ਕੱਢੋ_ਦੋਆਧਾਰੀ_ਅੰਕ
           (decSig.significandParts(), calcSemantics.precision - 1) == 1);

    HUerr = HUerrBound(calc_ਗੁਆਚਾ_Fraction != lfExactlyZero, sig_ਸਥਿਤੀ != opOK,
                       powHUerr);
    HUdistance = 2 * ulpsFromBoundary(decSig.significandParts(),
                                      excessPrecision, is_ਸਭ_ਤੋਂ_ਨੇੜੇ);

    /* Are we guaranteed to round correctly if we truncate?  */
    ਜੇ (HUdistance >= HUerr) {
      APInt::tc_ਕੱਢੋ(significandParts(), part_ਗਿਣਤੀ(), decSig.significandParts(),
                       calcSemantics.precision - excessPrecision,
                       excessPrecision);
      /* Take the exponent of decSig.  If we tcExtract-ed less bits
         above we must adjust our exponent to compensate for the
         implicit right shift.  */
      exponent = (decSig.exponent + semantics->precision
                  - (calcSemantics.precision - excessPrecision));
      calc_ਗੁਆਚਾ_Fraction = ਗੁਆਚਾ_Fraction_Through_Truncation(decSig.significandParts(),
                                                       decSig.part_ਗਿਣਤੀ(),
                                                       truncated_ਬਿੱਟ);
      ਵਾਪਸ static_cast<op_ਸਥਿਤੀ>(normalize(rounding_ਬਹੁਲਕ_2, calc_ਗੁਆਚਾ_Fraction) |
                                   ((sig_ਸਥਿਤੀ | pow_ਸਥਿਤੀ) & opInexact));
    }
  }
}

Expected<APFloat::op_ਸਥਿਤੀ>
IEEEFloat::convertFromDecimalString(StringRef str, rounding_ਬਹੁਲਕ rounding_ਬਹੁਲਕ_2) {
  decimalInfo D;
  op_ਸਥਿਤੀ fs;

  /* Scan the text.  */
  StringRef::iterator p = str.begin();
  ਜੇ (Error Err = interpretDecimal(p, str.ਸਿਰਾ(), &D))
    ਵਾਪਸ std::move(Err);

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
  ਜੇ (D.ਪਹਿਲਾ_Sig_Digit == str.ਸਿਰਾ() || dec_Digit_ਮੁੱਲ(*D.ਪਹਿਲਾ_Sig_Digit) >= 10U) {
    category = fcZero;
    fs = opOK;
    ਜੇ (semantics->nanEncoding == fltNanEncoding::NegativeZero)
      sign = ਝੂਠ;
    ਜੇ (!semantics->hasZero)
      make_ਸਭ_ਤੋਂ_ਛੋਟਾ_Normalized(ਝੂਠ);

    /* Check whether the normalized exponent is high enough to overflow
       max during the log-rebasing in the max-exponent check below. */
  } ਨਹੀਂਤਾਂ ਜੇ (D.normalizedExponent - 1 > ਵੱਧ_ਤੋਂ_ਵੱਧ_ਅੰਕ / 42039) {
    fs = handle_ਉਛਾਲ(rounding_ਬਹੁਲਕ_2);

  /* If it wasn't, then it also wasn't high enough to overflow max
     during the log-rebasing in the min-exponent check.  Check that it
     won't overflow min in either check, then perform the min-exponent
     check. */
  } ਨਹੀਂਤਾਂ ਜੇ (D.normalizedExponent - 1 < INT_MIN / 42039 ||
             (D.normalizedExponent + 1) * 28738 <=
               8651 * (semantics->minExponent - (ਪੂਰਨ) semantics->precision)) {
    /* Underflow to zero and round.  */
    category = fc_ਸਾਧਾਰਨ;
    zeroSignificand();
    fs = normalize(rounding_ਬਹੁਲਕ_2, lfLessThanHalf);

  /* We can finally safely perform the max-exponent check. */
  } ਨਹੀਂਤਾਂ ਜੇ ((D.normalizedExponent - 1) * 42039
             >= 12655 * semantics->maxExponent) {
    /* Overflow and round.  */
    fs = handle_ਉਛਾਲ(rounding_ਬਹੁਲਕ_2);
  } ਨਹੀਂਤਾਂ {
    integerPart *decSignificand;
    ਬਿਨਾਨਿਸ਼ਾਨੀ ਪੂਰਨ part_ਗਿਣਤੀ;

    /* A tight upper bound on number of bits required to hold an
       N-digit decimal integer is N * 196 / 59.  Allocate enough space
       to hold the full significand, and an extra part required by
       tcMultiplyPart.  */
    part_ਗਿਣਤੀ = static_cast<ਬਿਨਾਨਿਸ਼ਾਨੀ ਪੂਰਨ>(D.ਆਖਰੀ_Sig_Digit - D.ਪਹਿਲਾ_Sig_Digit) + 1;
    part_ਗਿਣਤੀ = part_ਗਿਣਤੀ_For_ਬਿੱਟ(1 + 196 * part_ਗਿਣਤੀ / 59);
    decSignificand = ਨਵਾਂ integerPart[part_ਗਿਣਤੀ + 1];
    part_ਗਿਣਤੀ = 0;

    /* Convert to binary efficiently - we do almost all multiplication
       in an integerPart.  When this would overflow do we do a single
       bignum multiplication, and then revert again to multiplication
       in an integerPart.  */
    ਕਰੋ {
      integerPart dec_ਮੁੱਲ, val, multiplier;

      val = 0;
      multiplier = 1;

      ਕਰੋ {
        ਜੇ (*p == '.') {
          p++;
          ਜੇ (p == str.ਸਿਰਾ()) {
            ਰੋਕੋ;
          }
        }
        dec_ਮੁੱਲ = dec_Digit_ਮੁੱਲ(*p++);
        ਜੇ (dec_ਮੁੱਲ >= 10U) {
          ਮਿਟਾਓ[] decSignificand;
          ਵਾਪਸ createError("Invalid character in significand");
        }
        multiplier *= 10;
        val = val * 10 + dec_ਮੁੱਲ;
        /* The maximum number that can be multiplied by ten with any
           digit added without overflowing an integerPart.  */
      } ਜਦੋਂ (p <= D.ਆਖਰੀ_Sig_Digit && multiplier <= (~ (integerPart) 0 - 9) / 10);

      /* Multiply out the current part.  */
      APInt::tcMultiplyPart(decSignificand, decSignificand, multiplier, val,
                            part_ਗਿਣਤੀ, part_ਗਿਣਤੀ + 1, ਝੂਠ);

      /* If we used another part (likely but not guaranteed), increase
         the count.  */
      ਜੇ (decSignificand[part_ਗਿਣਤੀ])
        part_ਗਿਣਤੀ++;
    } ਜਦੋਂ (p <= D.ਆਖਰੀ_Sig_Digit);

    category = fc_ਸਾਧਾਰਨ;
    fs = ਗੇੜਾ_Significand_With_Exponent(decSignificand, part_ਗਿਣਤੀ,
                                      D.exponent, rounding_ਬਹੁਲਕ_2);

    ਮਿਟਾਓ [] decSignificand;
  }

  ਵਾਪਸ fs;
}

ਤਾਰਕਿਕ IEEEFloat::convertFromStringSpecials(StringRef str) {
  ਅਚਲ size_t MIN_NAME_ਮਾਪ = 3;

  ਜੇ (str.ਮਾਪ_2() < MIN_NAME_ਮਾਪ)
    ਵਾਪਸ ਝੂਠ;

  ਜੇ (str == "inf" || str == "INFINITY" || str == "+Inf" || str == "+inf") {
    makeInf(ਝੂਠ);
    ਵਾਪਸ ਸੱਚ;
  }

  ਤਾਰਕਿਕ IsNegative = str.consume_front("-");
  ਜੇ (IsNegative) {
    ਜੇ (str.ਮਾਪ_2() < MIN_NAME_ਮਾਪ)
      ਵਾਪਸ ਝੂਠ;

    ਜੇ (str == "inf" || str == "INFINITY" || str == "Inf") {
      makeInf(ਸੱਚ);
      ਵਾਪਸ ਸੱਚ;
    }
  }

  // If we have a 's' (or 'S') prefix, then this is a Signaling NaN.
  ਤਾਰਕਿਕ IsSignaling = str.consume_front_insensitive("s");
  ਜੇ (IsSignaling) {
    ਜੇ (str.ਮਾਪ_2() < MIN_NAME_ਮਾਪ)
      ਵਾਪਸ ਝੂਠ;
  }

  ਜੇ (str.consume_front("nan") || str.consume_front("NaN")) {
    // A NaN without payload.
    ਜੇ (str.ਖਾਲੀ_ਥਾਵਾਂ()) {
      makeNaN(IsSignaling, IsNegative);
      ਵਾਪਸ ਸੱਚ;
    }

    // Allow the payload to be inside parentheses.
    ਜੇ (str.front() == '(') {
      // Parentheses should be balanced (and not empty).
      ਜੇ (str.ਮਾਪ_2() <= 2 || str.ਪਿੱਠ() != ')')
        ਵਾਪਸ ਝੂਠ;

      str = str.ਟੁਕੜਾ(1, str.ਮਾਪ_2() - 1);
    }

    // Determine the payload number's radix.
    ਬਿਨਾਨਿਸ਼ਾਨੀ Radix = 10;
    ਜੇ (str[0] == '0') {
      ਜੇ (str.ਮਾਪ_2() > 1 && tolower(str[1]) == 'x') {
        str = str.drop_front(2);
        Radix = 16;
      } ਨਹੀਂਤਾਂ {
        Radix = 8;
      }
    }

    // Parse the payload and make the NaN.
    APInt Payload;
    ਜੇ (!str.getAsInteger(Radix, Payload)) {
      makeNaN(IsSignaling, IsNegative, &Payload);
      ਵਾਪਸ ਸੱਚ;
    }
  }

  ਵਾਪਸ ਝੂਠ;
}

Expected<APFloat::op_ਸਥਿਤੀ>
IEEEFloat::convertFromString(StringRef str, rounding_ਬਹੁਲਕ rounding_ਬਹੁਲਕ_2) {
  ਜੇ (str.ਖਾਲੀ_ਥਾਵਾਂ())
    ਵਾਪਸ createError("Invalid string length");

  // Handle special cases.
  ਜੇ (convertFromStringSpecials(str))
    ਵਾਪਸ opOK;

  /* Handle a leading minus sign.  */
  StringRef::iterator p = str.begin();
  size_t slen = str.ਮਾਪ_2();
  sign = *p == '-' ? 1 : 0;
  ਜੇ (sign && !semantics->hasSignedRepr)
    llvm_unreachable(
        "This floating point format does not support signed values");

  ਜੇ (*p == '-' || *p == '+') {
    p++;
    slen--;
    ਜੇ (!slen)
      ਵਾਪਸ createError("String has no digits");
  }

  ਜੇ (slen >= 2 && p[0] == '0' && (p[1] == 'x' || p[1] == 'X')) {
    ਜੇ (slen == 2)
      ਵਾਪਸ createError("Invalid string");
    ਵਾਪਸ convertFromHexadecimalString(StringRef(p + 2, slen - 2),
                                        rounding_ਬਹੁਲਕ_2);
  }

  ਵਾਪਸ convertFromDecimalString(StringRef(p, slen), rounding_ਬਹੁਲਕ_2);
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
ਬਿਨਾਨਿਸ਼ਾਨੀ ਪੂਰਨ IEEEFloat::convert_ਮੰਜ਼ਿਲ_Hex_String(ਅੱਖਰ *dst, ਬਿਨਾਨਿਸ਼ਾਨੀ ਪੂਰਨ hexDigits,
                                           ਤਾਰਕਿਕ ਉੱਤਲਾ_Case,
                                           rounding_ਬਹੁਲਕ rounding_ਬਹੁਲਕ_2) ਅਚਲ {
  ਅੱਖਰ *p = dst;
  ਜੇ (sign)
    *dst++ = '-';

  ਚੋਣ (category) {
  ਹਾਲਤ fcInfinity:
    ਸਮ੍ਰਿਤੀ_ਨਕਲ (dst, ਉੱਤਲਾ_Case ? infinityU: infinityL, ਆਕਾਰ infinityU - 1);
    dst += ਆਕਾਰ infinityL - 1;
    ਰੋਕੋ;

  ਹਾਲਤ fcNaN:
    ਸਮ੍ਰਿਤੀ_ਨਕਲ (dst, ਉੱਤਲਾ_Case ? NaNU: NaNL, ਆਕਾਰ NaNU - 1);
    dst += ਆਕਾਰ NaNU - 1;
    ਰੋਕੋ;

  ਹਾਲਤ fcZero:
    *dst++ = '0';
    *dst++ = ਉੱਤਲਾ_Case ? 'X': 'x';
    *dst++ = '0';
    ਜੇ (hexDigits > 1) {
      *dst++ = '.';
      memset (dst, '0', hexDigits - 1);
      dst += hexDigits - 1;
    }
    *dst++ = ਉੱਤਲਾ_Case ? 'P': 'p';
    *dst++ = '0';
    ਰੋਕੋ;

  ਹਾਲਤ fc_ਸਾਧਾਰਨ:
    dst = convert_ਸਾਧਾਰਨ_ਮੰਜ਼ਿਲ_Hex_String (dst, hexDigits, ਉੱਤਲਾ_Case, rounding_ਬਹੁਲਕ_2);
    ਰੋਕੋ;
  }

  *dst = 0;

  ਵਾਪਸ static_cast<ਬਿਨਾਨਿਸ਼ਾਨੀ ਪੂਰਨ>(dst - p);
}

/* Does the hard work of outputting the correctly rounded hexadecimal
   form of a normal floating point number with the specified number of
   hexadecimal digits.  If HEXDIGITS is zero the minimum number of
   digits necessary to print the value precisely is output.  */
ਅੱਖਰ *IEEEFloat::convert_ਸਾਧਾਰਨ_ਮੰਜ਼ਿਲ_Hex_String(ਅੱਖਰ *dst, ਬਿਨਾਨਿਸ਼ਾਨੀ ਪੂਰਨ hexDigits,
                                          ਤਾਰਕਿਕ ਉੱਤਲਾ_Case,
                                          rounding_ਬਹੁਲਕ rounding_ਬਹੁਲਕ_2) ਅਚਲ {
  *dst++ = '0';
  *dst++ = ਉੱਤਲਾ_Case ? 'X': 'x';

  ਤਾਰਕਿਕ ਗੇੜਾ_ਉੱਪਰ = ਝੂਠ;
  ਅਚਲ ਅੱਖਰ *hexDigitChars = ਉੱਤਲਾ_Case ? hex_Digits_ਉੱਤਲਾ : hex_Digits_ਹੇਠਲਾ;

  ਅਚਲ integerPart *significand = significandParts();
  ਬਿਨਾਨਿਸ਼ਾਨੀ parts_ਗਿਣਤੀ = part_ਗਿਣਤੀ();

  /* +3 because the first digit only uses the single integer bit, so
     we have 3 virtual zero most-significant-bits.  */
  ਬਿਨਾਨਿਸ਼ਾਨੀ ਮੁੱਲ_ਬਿੱਟ = semantics->precision + 3;
  ਬਿਨਾਨਿਸ਼ਾਨੀ ਖਿਸਕਾਓ = integer_Part_ਚੌੜਾਈ - ਮੁੱਲ_ਬਿੱਟ % integer_Part_ਚੌੜਾਈ;

  /* The natural number of digits required ignoring trailing
     insignificant zeroes.  */
  ਬਿਨਾਨਿਸ਼ਾਨੀ outputDigits = (ਮੁੱਲ_ਬਿੱਟ - significandLSB() + 3) / 4;

  /* hexDigits of zero means use the required number for the
     precision.  Otherwise, see if we are truncating.  If we are,
     find out if we need to round away from zero.  */
  ਜੇ (hexDigits) {
    ਜੇ (hexDigits < outputDigits) {
      /* We are dropping non-zero bits, so need to check how to round.
         "bits" is the number of dropped bits.  */
      ਬਿਨਾਨਿਸ਼ਾਨੀ ਪੂਰਨ ਬਿੱਟ;
      ਗੁਆਚਾ_Fraction fraction;

      ਬਿੱਟ = ਮੁੱਲ_ਬਿੱਟ - hexDigits * 4;
      fraction = ਗੁਆਚਾ_Fraction_Through_Truncation (significand, parts_ਗਿਣਤੀ, ਬਿੱਟ);
      ਗੇੜਾ_ਉੱਪਰ = ਗੇੜਾ_Away_From_Zero(rounding_ਬਹੁਲਕ_2, fraction, ਬਿੱਟ);
    }
    outputDigits = hexDigits;
  }

  /* Write the digits consecutively, and start writing in the location
     of the hexadecimal point.  We move the most significant digit
     left and add the hexadecimal point later.  */
  ਅੱਖਰ *p = ++dst;

  ਬਿਨਾਨਿਸ਼ਾਨੀ ਗਿਣਤੀ_2 = (ਮੁੱਲ_ਬਿੱਟ + integer_Part_ਚੌੜਾਈ - 1) / integer_Part_ਚੌੜਾਈ;

  ਜਦੋਂ (outputDigits && ਗਿਣਤੀ_2) {
    integerPart part;

    /* Put the most significant integerPartWidth bits in "part".  */
    ਜੇ (--ਗਿਣਤੀ_2 == parts_ਗਿਣਤੀ)
      part = 0;  /* An imaginary higher zero part.  */
    ਨਹੀਂਤਾਂ
      part = significand[ਗਿਣਤੀ_2] << ਖਿਸਕਾਓ;

    ਜੇ (ਗਿਣਤੀ_2 && ਖਿਸਕਾਓ)
      part |= significand[ਗਿਣਤੀ_2 - 1] >> (integer_Part_ਚੌੜਾਈ - ਖਿਸਕਾਓ);

    /* Convert as much of "part" to hexdigits as we can.  */
    ਬਿਨਾਨਿਸ਼ਾਨੀ ਪੂਰਨ curDigits = integer_Part_ਚੌੜਾਈ / 4;

    curDigits = std::min(curDigits, outputDigits);
    dst += partAsHex (dst, part, curDigits, hexDigitChars);
    outputDigits -= curDigits;
  }

  ਜੇ (ਗੇੜਾ_ਉੱਪਰ) {
    ਅੱਖਰ *q = dst;

    /* Note that hexDigitChars has a trailing '0'.  */
    ਕਰੋ {
      q--;
      *q = hexDigitChars[hex_Digit_ਮੁੱਲ (*q) + 1];
    } ਜਦੋਂ (*q == '0');
    ਪ੍ਰਤਿਜ੍ਞਾ(q >= p);
  } ਨਹੀਂਤਾਂ {
    /* Add trailing zeroes.  */
    memset (dst, '0', outputDigits);
    dst += outputDigits;
  }

  /* Move the most significant digit to before the point, and if there
     is something after the decimal point add it.  This must come
     after rounding above.  */
  p[-1] = p[0];
  ਜੇ (dst -1 == p)
    dst--;
  ਨਹੀਂਤਾਂ
    p[0] = '.';

  /* Finally output the exponent.  */
  *dst++ = ਉੱਤਲਾ_Case ? 'P': 'p';

  ਵਾਪਸ writeSignedDecimal (dst, exponent);
}

hash_ਕੋਡ hash_ਮੁੱਲ(ਅਚਲ IEEEFloat &Arg) {
  ਜੇ (!Arg.isFiniteNonZero())
    ਵਾਪਸ hash_combine((uint8_t)Arg.category,
                        // NaN has no sign, fix it at zero.
                        Arg.isNaN() ? (uint8_t)0 : (uint8_t)Arg.sign,
                        Arg.semantics->precision);

  // Normal floats need their exponent and significand hashed.
  ਵਾਪਸ hash_combine((uint8_t)Arg.category, (uint8_t)Arg.sign,
                      Arg.semantics->precision, Arg.exponent,
                      hash_combine_range(
                        Arg.significandParts(),
                        Arg.significandParts() + Arg.part_ਗਿਣਤੀ()));
}

// Conversion from APFloat to/from host float/double.  It may eventually be
// possible to eliminate these and have everybody deal with APFloats, but that
// will take a while.  This approach will not easily extend to long double.
// Current implementation requires integerPartWidth==64, which is correct at
// the moment but could be made more general.

// Denormals have exponent minExponent in APFloat, but minExponent-1 in
// the actual IEEE respresentations.  We compensate for that here.

APInt IEEEFloat::convert_F80_Long_Double_APFloat_ਮੰਜ਼ਿਲ_APInt() ਅਚਲ {
  ਪ੍ਰਤਿਜ੍ਞਾ(part_ਗਿਣਤੀ() == 2);
  ਵਾਪਸ convert_IEEEFloat_ਮੰਜ਼ਿਲ_APInt<APFloat_ਅਧਾਰ::semX87DoubleExtended>();
}

APInt IEEEFloat::convert_PPCDouble_Double_Legacy_APFloat_ਮੰਜ਼ਿਲ_APInt() ਅਚਲ {
  ਪ੍ਰਤਿਜ੍ਞਾ(semantics ==
         (ਅਚਲ llvm::fltSemantics *)&APFloat_ਅਧਾਰ::semPPCDoubleDoubleLegacy);
  ਪ੍ਰਤਿਜ੍ਞਾ(part_ਗਿਣਤੀ()==2);

  uint64_t ਸ਼ਬਦ[2];
  ਤਾਰਕਿਕ losesInfo;

  // Convert number to double.  To avoid spurious underflows, we re-
  // normalize against the "double" minExponent first, and only *then*
  // truncate the mantissa.  The result of that second conversion
  // may be inexact, but should never underflow.
  // Declare fltSemantics before APFloat that uses it (and
  // saves pointer to it) to ensure correct destruction order.
  fltSemantics extendedSemantics = *semantics;
  extendedSemantics.minExponent = APFloat_ਅਧਾਰ::semIEEEdouble.minExponent;
  IEEEFloat extended(*ਇਹ);
  [[maybe_ਅਣਵਰਤਿਆ]] op_ਸਥਿਤੀ fs =
      extended.convert(extendedSemantics, rm_ਸਭ_ਤੋਂ_ਨੇੜੇ_Ties_ਮੰਜ਼ਿਲ_Even, &losesInfo);
  ਪ੍ਰਤਿਜ੍ਞਾ(fs == opOK && !losesInfo);

  IEEEFloat u(extended);
  fs = u.convert(APFloat_ਅਧਾਰ::semIEEEdouble, rm_ਸਭ_ਤੋਂ_ਨੇੜੇ_Ties_ਮੰਜ਼ਿਲ_Even, &losesInfo);
  ਪ੍ਰਤਿਜ੍ਞਾ(fs == opOK || fs == opInexact);
  ਸ਼ਬਦ[0] = *u.convert_Double_APFloat_ਮੰਜ਼ਿਲ_APInt().get_Raw_ਅੰਕੜੇ();

  // If conversion was exact or resulted in a special case, we're done;
  // just set the second double to zero.  Otherwise, re-convert back to
  // the extended format and compute the difference.  This now should
  // convert exactly to double.
  ਜੇ (u.isFiniteNonZero() && losesInfo) {
    fs = u.convert(extendedSemantics, rm_ਸਭ_ਤੋਂ_ਨੇੜੇ_Ties_ਮੰਜ਼ਿਲ_Even, &losesInfo);
    ਪ੍ਰਤਿਜ੍ਞਾ(fs == opOK && !losesInfo);

    IEEEFloat v(extended);
    v.ਘਟਾਓ(u, rm_ਸਭ_ਤੋਂ_ਨੇੜੇ_Ties_ਮੰਜ਼ਿਲ_Even);
    fs = v.convert(APFloat_ਅਧਾਰ::semIEEEdouble, rm_ਸਭ_ਤੋਂ_ਨੇੜੇ_Ties_ਮੰਜ਼ਿਲ_Even, &losesInfo);
    ਪ੍ਰਤਿਜ੍ਞਾ(fs == opOK && !losesInfo);
    ਸ਼ਬਦ[1] = *v.convert_Double_APFloat_ਮੰਜ਼ਿਲ_APInt().get_Raw_ਅੰਕੜੇ();
  } ਨਹੀਂਤਾਂ {
    ਸ਼ਬਦ[1] = 0;
  }

  ਵਾਪਸ APInt(128, ਸ਼ਬਦ);
}

ਸਾਂਚਾ <ਅਚਲ fltSemantics &S>
APInt IEEEFloat::convert_IEEEFloat_ਮੰਜ਼ਿਲ_APInt() ਅਚਲ {
  ਪ੍ਰਤਿਜ੍ਞਾ(semantics == &S);
  constexpr ਬਿਨਾਨਿਸ਼ਾਨੀ ਪੂਰਨ trailing_significand_ਬਿੱਟ =
      S.precision - 1 + S.has_Explicit_Integer_ਦੋਆਧਾਰੀ_ਅੰਕ;
  constexpr ਪੂਰਨ integer_ਦੋਆਧਾਰੀ_ਅੰਕ_part = (S.precision - 1) / integer_Part_ਚੌੜਾਈ;
  constexpr integerPart integer_ਦੋਆਧਾਰੀ_ਅੰਕ = integerPart{1}
                                      << ((S.precision - 1) % integer_Part_ਚੌੜਾਈ);
  constexpr uint64_t significand_ਨਕਾਬ = integer_ਦੋਆਧਾਰੀ_ਅੰਕ - 1;
  constexpr ਬਿਨਾਨਿਸ਼ਾਨੀ ਪੂਰਨ exponent_ਬਿੱਟ =
      S.ਮਾਪ_ਵਿੱਚ_ਬਿੱਟ - (S.hasSignedRepr ? 1 : 0) - trailing_significand_ਬਿੱਟ;
  static_assert(exponent_ਬਿੱਟ < 64);
  constexpr uint64_t exponent_ਨਕਾਬ = (uint64_t{1} << exponent_ਬਿੱਟ) - 1;
  constexpr ਤਾਰਕਿਕ is_zero_exp_reserved = S.hasDenormals || S.hasZero;
  constexpr ਪੂਰਨ bias = -(S.minExponent - (is_zero_exp_reserved ? 1 : 0));

  uint64_t myexponent;
  std::array<integerPart, part_ਗਿਣਤੀ_For_ਬਿੱਟ(trailing_significand_ਬਿੱਟ)>
      mysignificand;

  ਜੇ (isFiniteNonZero()) {
    myexponent = exponent + bias;
    std::copy_n(significandParts(), mysignificand.ਮਾਪ_2(),
                mysignificand.begin());
    ਜੇ (myexponent == 1 &&
        !(significandParts()[integer_ਦੋਆਧਾਰੀ_ਅੰਕ_part] & integer_ਦੋਆਧਾਰੀ_ਅੰਕ))
      myexponent = 0; // denormal
  } ਨਹੀਂਤਾਂ ਜੇ (category == fcZero) {
    ਜੇ (!S.hasZero)
      llvm_unreachable("semantics does not support zero!");
    myexponent = ::exponentZero(S) + bias;
    mysignificand.fill(0);
  } ਨਹੀਂਤਾਂ ਜੇ (category == fcInfinity) {
    ਜੇ (S.nonFiniteBehavior == fltNonfiniteBehavior::NanOnly ||
        S.nonFiniteBehavior == fltNonfiniteBehavior::FiniteOnly)
      llvm_unreachable("semantics don't support inf!");
    myexponent = ::exponentInf(S) + bias;
    mysignificand.fill(0);
    ਜੇ constexpr (S.has_Explicit_Integer_ਦੋਆਧਾਰੀ_ਅੰਕ) {
      mysignificand[0] = integerPart{1} << (trailing_significand_ਬਿੱਟ - 1);
    }
  } ਨਹੀਂਤਾਂ {
    ਪ੍ਰਤਿਜ੍ਞਾ(category == fcNaN && "Unknown category!");
    ਜੇ (S.nonFiniteBehavior == fltNonfiniteBehavior::FiniteOnly)
      llvm_unreachable("semantics don't support NaN!");
    myexponent = ::exponentNaN(S) + bias;
    std::copy_n(significandParts(), mysignificand.ਮਾਪ_2(),
                mysignificand.begin());
  }
  std::array<uint64_t, (S.ਮਾਪ_ਵਿੱਚ_ਬਿੱਟ + 63) / 64> ਸ਼ਬਦ;
  ਸਵੈਚਾਲਿਤ ਸ਼ਬਦ_iter =
      std::copy_n(mysignificand.begin(), mysignificand.ਮਾਪ_2(), ਸ਼ਬਦ.begin());
  ਜੇ constexpr (!S.has_Explicit_Integer_ਦੋਆਧਾਰੀ_ਅੰਕ) {
    ਜੇ constexpr (significand_ਨਕਾਬ != 0 || trailing_significand_ਬਿੱਟ == 0) {
      // Clear the integer bit.
      ਸ਼ਬਦ[mysignificand.ਮਾਪ_2() - 1] &= significand_ਨਕਾਬ;
    }
  }
  std::fill(ਸ਼ਬਦ_iter, ਸ਼ਬਦ.ਸਿਰਾ(), uint64_t{0});
  constexpr size_t ਆਖਰੀ_ਸ਼ਬਦ = ਸ਼ਬਦ.ਮਾਪ_2() - 1;
  ਜੇ constexpr (S.hasSignedRepr) {
    uint64_t shifted_sign = static_cast<uint64_t>(sign & 1)
                            << ((S.ਮਾਪ_ਵਿੱਚ_ਬਿੱਟ - 1) % 64);
    ਸ਼ਬਦ[ਆਖਰੀ_ਸ਼ਬਦ] |= shifted_sign;
  }
  uint64_t shifted_exponent = (myexponent & exponent_ਨਕਾਬ)
                              << (trailing_significand_ਬਿੱਟ % 64);
  ਸ਼ਬਦ[ਆਖਰੀ_ਸ਼ਬਦ] |= shifted_exponent;
  ਜੇ constexpr (ਆਖਰੀ_ਸ਼ਬਦ == 0) {
    ਵਾਪਸ APInt(S.ਮਾਪ_ਵਿੱਚ_ਬਿੱਟ, ਸ਼ਬਦ[0]);
  }
  ਵਾਪਸ APInt(S.ਮਾਪ_ਵਿੱਚ_ਬਿੱਟ, ਸ਼ਬਦ);
}

APInt IEEEFloat::convert_Quadruple_APFloat_ਮੰਜ਼ਿਲ_APInt() ਅਚਲ {
  ਪ੍ਰਤਿਜ੍ਞਾ(part_ਗਿਣਤੀ() == 2);
  ਵਾਪਸ convert_IEEEFloat_ਮੰਜ਼ਿਲ_APInt<APFloat_ਅਧਾਰ::semIEEEquad>();
}

APInt IEEEFloat::convert_Double_APFloat_ਮੰਜ਼ਿਲ_APInt() ਅਚਲ {
  ਪ੍ਰਤਿਜ੍ਞਾ(part_ਗਿਣਤੀ()==1);
  ਵਾਪਸ convert_IEEEFloat_ਮੰਜ਼ਿਲ_APInt<APFloat_ਅਧਾਰ::semIEEEdouble>();
}

APInt IEEEFloat::convert_Float_APFloat_ਮੰਜ਼ਿਲ_APInt() ਅਚਲ {
  ਪ੍ਰਤਿਜ੍ਞਾ(part_ਗਿਣਤੀ()==1);
  ਵਾਪਸ convert_IEEEFloat_ਮੰਜ਼ਿਲ_APInt<APFloat_ਅਧਾਰ::semIEEEsingle>();
}

APInt IEEEFloat::convert_BFloat_APFloat_ਮੰਜ਼ਿਲ_APInt() ਅਚਲ {
  ਪ੍ਰਤਿਜ੍ਞਾ(part_ਗਿਣਤੀ() == 1);
  ਵਾਪਸ convert_IEEEFloat_ਮੰਜ਼ਿਲ_APInt<APFloat_ਅਧਾਰ::semBFloat>();
}

APInt IEEEFloat::convert_Half_APFloat_ਮੰਜ਼ਿਲ_APInt() ਅਚਲ {
  ਪ੍ਰਤਿਜ੍ਞਾ(part_ਗਿਣਤੀ()==1);
  ਵਾਪਸ convert_IEEEFloat_ਮੰਜ਼ਿਲ_APInt<APFloat_ਅਧਾਰ::APFloat_ਅਧਾਰ::semIEEEhalf>();
}

APInt IEEEFloat::convert_Float8_E5_M2_APFloat_ਮੰਜ਼ਿਲ_APInt() ਅਚਲ {
  ਪ੍ਰਤਿਜ੍ਞਾ(part_ਗਿਣਤੀ() == 1);
  ਵਾਪਸ convert_IEEEFloat_ਮੰਜ਼ਿਲ_APInt<APFloat_ਅਧਾਰ::semFloat8E5M2>();
}

APInt IEEEFloat::convert_Float8_E5_M2_FNUZAPFloat_ਮੰਜ਼ਿਲ_APInt() ਅਚਲ {
  ਪ੍ਰਤਿਜ੍ਞਾ(part_ਗਿਣਤੀ() == 1);
  ਵਾਪਸ convert_IEEEFloat_ਮੰਜ਼ਿਲ_APInt<APFloat_ਅਧਾਰ::semFloat8E5M2FNUZ>();
}

APInt IEEEFloat::convert_Float8_E4_M3_APFloat_ਮੰਜ਼ਿਲ_APInt() ਅਚਲ {
  ਪ੍ਰਤਿਜ੍ਞਾ(part_ਗਿਣਤੀ() == 1);
  ਵਾਪਸ convert_IEEEFloat_ਮੰਜ਼ਿਲ_APInt<APFloat_ਅਧਾਰ::semFloat8E4M3>();
}

APInt IEEEFloat::convert_Float8_E4_M3_FNAPFloat_ਮੰਜ਼ਿਲ_APInt() ਅਚਲ {
  ਪ੍ਰਤਿਜ੍ਞਾ(part_ਗਿਣਤੀ() == 1);
  ਵਾਪਸ convert_IEEEFloat_ਮੰਜ਼ਿਲ_APInt<APFloat_ਅਧਾਰ::semFloat8E4M3FN>();
}

APInt IEEEFloat::convert_Float8_E4_M3_FNUZAPFloat_ਮੰਜ਼ਿਲ_APInt() ਅਚਲ {
  ਪ੍ਰਤਿਜ੍ਞਾ(part_ਗਿਣਤੀ() == 1);
  ਵਾਪਸ convert_IEEEFloat_ਮੰਜ਼ਿਲ_APInt<APFloat_ਅਧਾਰ::semFloat8E4M3FNUZ>();
}

APInt IEEEFloat::convert_Float8_E4_M3_B11_FNUZAPFloat_ਮੰਜ਼ਿਲ_APInt() ਅਚਲ {
  ਪ੍ਰਤਿਜ੍ਞਾ(part_ਗਿਣਤੀ() == 1);
  ਵਾਪਸ convert_IEEEFloat_ਮੰਜ਼ਿਲ_APInt<APFloat_ਅਧਾਰ::semFloat8E4M3B11FNUZ>();
}

APInt IEEEFloat::convert_Float8_E3_M4_APFloat_ਮੰਜ਼ਿਲ_APInt() ਅਚਲ {
  ਪ੍ਰਤਿਜ੍ਞਾ(part_ਗਿਣਤੀ() == 1);
  ਵਾਪਸ convert_IEEEFloat_ਮੰਜ਼ਿਲ_APInt<APFloat_ਅਧਾਰ::semFloat8E3M4>();
}

APInt IEEEFloat::convert_Float_TF32_APFloat_ਮੰਜ਼ਿਲ_APInt() ਅਚਲ {
  ਪ੍ਰਤਿਜ੍ਞਾ(part_ਗਿਣਤੀ() == 1);
  ਵਾਪਸ convert_IEEEFloat_ਮੰਜ਼ਿਲ_APInt<APFloat_ਅਧਾਰ::semFloatTF32>();
}

APInt IEEEFloat::convert_Float8_E8_M0_FNUAPFloat_ਮੰਜ਼ਿਲ_APInt() ਅਚਲ {
  ਪ੍ਰਤਿਜ੍ਞਾ(part_ਗਿਣਤੀ() == 1);
  ਵਾਪਸ convert_IEEEFloat_ਮੰਜ਼ਿਲ_APInt<APFloat_ਅਧਾਰ::semFloat8E8M0FNU>();
}

APInt IEEEFloat::convert_Float8_E5_M3_FNUAPFloat_ਮੰਜ਼ਿਲ_APInt() ਅਚਲ {
  ਪ੍ਰਤਿਜ੍ਞਾ(part_ਗਿਣਤੀ() == 1);
  ਵਾਪਸ convert_IEEEFloat_ਮੰਜ਼ਿਲ_APInt<APFloat_ਅਧਾਰ::semFloat8E5M3FNU>();
}

APInt IEEEFloat::convert_Float6_E3_M2_FNAPFloat_ਮੰਜ਼ਿਲ_APInt() ਅਚਲ {
  ਪ੍ਰਤਿਜ੍ਞਾ(part_ਗਿਣਤੀ() == 1);
  ਵਾਪਸ convert_IEEEFloat_ਮੰਜ਼ਿਲ_APInt<APFloat_ਅਧਾਰ::semFloat6E3M2FN>();
}

APInt IEEEFloat::convert_Float6_E2_M3_FNAPFloat_ਮੰਜ਼ਿਲ_APInt() ਅਚਲ {
  ਪ੍ਰਤਿਜ੍ਞਾ(part_ਗਿਣਤੀ() == 1);
  ਵਾਪਸ convert_IEEEFloat_ਮੰਜ਼ਿਲ_APInt<APFloat_ਅਧਾਰ::semFloat6E2M3FN>();
}

APInt IEEEFloat::convert_Float4_E2_M1_FNAPFloat_ਮੰਜ਼ਿਲ_APInt() ਅਚਲ {
  ਪ੍ਰਤਿਜ੍ਞਾ(part_ਗਿਣਤੀ() == 1);
  ਵਾਪਸ convert_IEEEFloat_ਮੰਜ਼ਿਲ_APInt<APFloat_ਅਧਾਰ::semFloat4E2M1FN>();
}

// This function creates an APInt that is just a bit map of the floating
// point constant as it would appear in memory.  It is not a conversion,
// and treating the result as a normal integer is unlikely to be useful.

APInt IEEEFloat::bitcast_ਮੰਜ਼ਿਲ_APInt() ਅਚਲ {
  ਜੇ (semantics == (ਅਚਲ llvm::fltSemantics *)&APFloat_ਅਧਾਰ::semIEEEhalf)
    ਵਾਪਸ convert_Half_APFloat_ਮੰਜ਼ਿਲ_APInt();

  ਜੇ (semantics == (ਅਚਲ llvm::fltSemantics *)&APFloat_ਅਧਾਰ::semBFloat)
    ਵਾਪਸ convert_BFloat_APFloat_ਮੰਜ਼ਿਲ_APInt();

  ਜੇ (semantics == (ਅਚਲ llvm::fltSemantics *)&APFloat_ਅਧਾਰ::semIEEEsingle)
    ਵਾਪਸ convert_Float_APFloat_ਮੰਜ਼ਿਲ_APInt();

  ਜੇ (semantics == (ਅਚਲ llvm::fltSemantics *)&APFloat_ਅਧਾਰ::semIEEEdouble)
    ਵਾਪਸ convert_Double_APFloat_ਮੰਜ਼ਿਲ_APInt();

  ਜੇ (semantics == (ਅਚਲ llvm::fltSemantics *)&APFloat_ਅਧਾਰ::semIEEEquad)
    ਵਾਪਸ convert_Quadruple_APFloat_ਮੰਜ਼ਿਲ_APInt();

  ਜੇ (semantics ==
      (ਅਚਲ llvm::fltSemantics *)&APFloat_ਅਧਾਰ::semX87DoubleExtended)
    ਵਾਪਸ convert_F80_Long_Double_APFloat_ਮੰਜ਼ਿਲ_APInt();

  ਜੇ (semantics ==
      (ਅਚਲ llvm::fltSemantics *)&APFloat_ਅਧਾਰ::semPPCDoubleDoubleLegacy)
    ਵਾਪਸ convert_PPCDouble_Double_Legacy_APFloat_ਮੰਜ਼ਿਲ_APInt();

  ਜੇ (semantics == (ਅਚਲ llvm::fltSemantics *)&APFloat_ਅਧਾਰ::semFloat8E5M2)
    ਵਾਪਸ convert_Float8_E5_M2_APFloat_ਮੰਜ਼ਿਲ_APInt();

  ਜੇ (semantics == (ਅਚਲ llvm::fltSemantics *)&APFloat_ਅਧਾਰ::semFloat8E5M2FNUZ)
    ਵਾਪਸ convert_Float8_E5_M2_FNUZAPFloat_ਮੰਜ਼ਿਲ_APInt();

  ਜੇ (semantics == (ਅਚਲ llvm::fltSemantics *)&APFloat_ਅਧਾਰ::semFloat8E4M3)
    ਵਾਪਸ convert_Float8_E4_M3_APFloat_ਮੰਜ਼ਿਲ_APInt();

  ਜੇ (semantics == (ਅਚਲ llvm::fltSemantics *)&APFloat_ਅਧਾਰ::semFloat8E4M3FN)
    ਵਾਪਸ convert_Float8_E4_M3_FNAPFloat_ਮੰਜ਼ਿਲ_APInt();

  ਜੇ (semantics == (ਅਚਲ llvm::fltSemantics *)&APFloat_ਅਧਾਰ::semFloat8E4M3FNUZ)
    ਵਾਪਸ convert_Float8_E4_M3_FNUZAPFloat_ਮੰਜ਼ਿਲ_APInt();

  ਜੇ (semantics ==
      (ਅਚਲ llvm::fltSemantics *)&APFloat_ਅਧਾਰ::semFloat8E4M3B11FNUZ)
    ਵਾਪਸ convert_Float8_E4_M3_B11_FNUZAPFloat_ਮੰਜ਼ਿਲ_APInt();

  ਜੇ (semantics == (ਅਚਲ llvm::fltSemantics *)&APFloat_ਅਧਾਰ::semFloat8E3M4)
    ਵਾਪਸ convert_Float8_E3_M4_APFloat_ਮੰਜ਼ਿਲ_APInt();

  ਜੇ (semantics == (ਅਚਲ llvm::fltSemantics *)&APFloat_ਅਧਾਰ::semFloatTF32)
    ਵਾਪਸ convert_Float_TF32_APFloat_ਮੰਜ਼ਿਲ_APInt();

  ਜੇ (semantics == (ਅਚਲ llvm::fltSemantics *)&APFloat_ਅਧਾਰ::semFloat8E8M0FNU)
    ਵਾਪਸ convert_Float8_E8_M0_FNUAPFloat_ਮੰਜ਼ਿਲ_APInt();

  ਜੇ (semantics == (ਅਚਲ llvm::fltSemantics *)&APFloat_ਅਧਾਰ::semFloat8E5M3FNU)
    ਵਾਪਸ convert_Float8_E5_M3_FNUAPFloat_ਮੰਜ਼ਿਲ_APInt();

  ਜੇ (semantics == (ਅਚਲ llvm::fltSemantics *)&APFloat_ਅਧਾਰ::semFloat6E3M2FN)
    ਵਾਪਸ convert_Float6_E3_M2_FNAPFloat_ਮੰਜ਼ਿਲ_APInt();

  ਜੇ (semantics == (ਅਚਲ llvm::fltSemantics *)&APFloat_ਅਧਾਰ::semFloat6E2M3FN)
    ਵਾਪਸ convert_Float6_E2_M3_FNAPFloat_ਮੰਜ਼ਿਲ_APInt();

  ਜੇ (semantics == (ਅਚਲ llvm::fltSemantics *)&APFloat_ਅਧਾਰ::semFloat4E2M1FN)
    ਵਾਪਸ convert_Float4_E2_M1_FNAPFloat_ਮੰਜ਼ਿਲ_APInt();

  llvm_unreachable("unknown format!");
}

ਦਸ਼ਮਲਵ IEEEFloat::convert_ਮੰਜ਼ਿਲ_Float() ਅਚਲ {
  ਪ੍ਰਤਿਜ੍ਞਾ(semantics == (ਅਚਲ llvm::fltSemantics *)&APFloat_ਅਧਾਰ::semIEEEsingle &&
         "Float semantics are not IEEEsingle");
  APInt api = bitcast_ਮੰਜ਼ਿਲ_APInt();
  ਵਾਪਸ api.ਬਿੱਟ_ਮੰਜ਼ਿਲ_Float();
}

ਦੁੱਗਣਾ IEEEFloat::convert_ਮੰਜ਼ਿਲ_Double() ਅਚਲ {
  ਪ੍ਰਤਿਜ੍ਞਾ(semantics == (ਅਚਲ llvm::fltSemantics *)&APFloat_ਅਧਾਰ::semIEEEdouble &&
         "Float semantics are not IEEEdouble");
  APInt api = bitcast_ਮੰਜ਼ਿਲ_APInt();
  ਵਾਪਸ api.ਬਿੱਟ_ਮੰਜ਼ਿਲ_Double();
}

#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ HAS_IEE754_FLOAT128
float128 IEEEFloat::convert_ਮੰਜ਼ਿਲ_Quad() ਅਚਲ {
  ਪ੍ਰਤਿਜ੍ਞਾ(semantics == (ਅਚਲ llvm::fltSemantics *)&APFloat_ਅਧਾਰ::semIEEEquad &&
         "Float semantics are not IEEEquads");
  APInt api = bitcast_ਮੰਜ਼ਿਲ_APInt();
  ਵਾਪਸ api.ਬਿੱਟ_ਮੰਜ਼ਿਲ_Quad();
}
#ਜੇ_ਅੰਤ

ਖਾਲੀ IEEEFloat::initFromF80LongDoubleAPInt(ਅਚਲ APInt &api) {
  ਵਾਪਸ initFromIEEEAPInt<APFloat_ਅਧਾਰ::semX87DoubleExtended>(api);
}

ਖਾਲੀ IEEEFloat::initFromPPCDoubleDoubleLegacyAPInt(ਅਚਲ APInt &api) {
  uint64_t i1 = api.get_Raw_ਅੰਕੜੇ()[0];
  uint64_t i2 = api.get_Raw_ਅੰਕੜੇ()[1];
  ਤਾਰਕਿਕ losesInfo;

  // Get the first double and convert to our format.
  initFromDoubleAPInt(APInt(64, i1));
  [[maybe_ਅਣਵਰਤਿਆ]] op_ਸਥਿਤੀ fs = convert(APFloat_ਅਧਾਰ::semPPCDoubleDoubleLegacy,
                                         rm_ਸਭ_ਤੋਂ_ਨੇੜੇ_Ties_ਮੰਜ਼ਿਲ_Even, &losesInfo);
  // (convert may return opInvalidOp if i1 is an sNaN).
  ਪ੍ਰਤਿਜ੍ਞਾ((fs == opOK || fs == opInvalidOp) && !losesInfo);

  // Unless we have a special case, add in second double.
  ਜੇ (isFiniteNonZero()) {
    IEEEFloat v(APFloat_ਅਧਾਰ::semIEEEdouble, APInt(64, i2));
    // The first double determines the category, so ignore a non-canonical
    // second double.
    ਜੇ (v.isFinite()) {
      fs = v.convert(APFloat_ਅਧਾਰ::semPPCDoubleDoubleLegacy, rm_ਸਭ_ਤੋਂ_ਨੇੜੇ_Ties_ਮੰਜ਼ਿਲ_Even,
                     &losesInfo);
      ਪ੍ਰਤਿਜ੍ਞਾ(fs == opOK && !losesInfo);

      ਜੋੜੋ(v, rm_ਸਭ_ਤੋਂ_ਨੇੜੇ_Ties_ਮੰਜ਼ਿਲ_Even);
    }
  }
}

// The E8M0 format has the following characteristics:
// It is an 8-bit unsigned format with only exponents (no actual significand).
// No encodings for {zero, infinities or denorms}.
// NaN is represented by all 1's.
// Bias is 127.
ਖਾਲੀ IEEEFloat::initFromFloat8E8M0FNUAPInt(ਅਚਲ APInt &api) {
  initFromIEEEAPInt<APFloat_ਅਧਾਰ::semFloat8E8M0FNU>(api);
}

ਖਾਲੀ IEEEFloat::initFromFloat8E5M3FNUAPInt(ਅਚਲ APInt &api) {
  initFromIEEEAPInt<APFloat_ਅਧਾਰ::semFloat8E5M3FNU>(api);
}

ਸਾਂਚਾ <ਅਚਲ fltSemantics &S>
ਖਾਲੀ IEEEFloat::initFromIEEEAPInt(ਅਚਲ APInt &api) {
  ਪ੍ਰਤਿਜ੍ਞਾ(api.get_ਦੋਆਧਾਰੀ_ਅੰਕ_ਚੌੜਾਈ() == S.ਮਾਪ_ਵਿੱਚ_ਬਿੱਟ);

  constexpr ਬਿਨਾਨਿਸ਼ਾਨੀ ਪੂਰਨ trailing_significand_ਬਿੱਟ =
      S.precision - 1 + S.has_Explicit_Integer_ਦੋਆਧਾਰੀ_ਅੰਕ;
  constexpr integerPart integer_ਦੋਆਧਾਰੀ_ਅੰਕ =
      integerPart{1} << (trailing_significand_ਬਿੱਟ % integer_Part_ਚੌੜਾਈ);
  constexpr uint64_t significand_ਨਕਾਬ = integer_ਦੋਆਧਾਰੀ_ਅੰਕ - 1;
  constexpr ਬਿਨਾਨਿਸ਼ਾਨੀ ਪੂਰਨ exponent_ਬਿੱਟ =
      S.ਮਾਪ_ਵਿੱਚ_ਬਿੱਟ - (S.hasSignedRepr ? 1 : 0) - trailing_significand_ਬਿੱਟ;
  static_assert(exponent_ਬਿੱਟ < 64);
  constexpr ਬਿਨਾਨਿਸ਼ਾਨੀ ਪੂਰਨ stored_significand_parts =
      part_ਗਿਣਤੀ_For_ਬਿੱਟ(trailing_significand_ਬਿੱਟ + 1);
  constexpr uint64_t exponent_ਨਕਾਬ = (uint64_t{1} << exponent_ਬਿੱਟ) - 1;
  constexpr ਤਾਰਕਿਕ is_zero_exp_reserved = S.hasDenormals || S.hasZero;
  constexpr ਪੂਰਨ bias = -(S.minExponent - (is_zero_exp_reserved ? 1 : 0));
  constexpr ਤਾਰਕਿਕ has_significand = trailing_significand_ਬਿੱਟ > 0;

  // Copy the bits of the significand. We need to clear out the exponent and
  // sign bit in the last word.
  std::array<integerPart, stored_significand_parts> mysignificand;
  ਜੇ constexpr (has_significand) {
    std::copy_n(api.get_Raw_ਅੰਕੜੇ(), mysignificand.ਮਾਪ_2(), mysignificand.begin());
    ਜੇ constexpr (significand_ਨਕਾਬ != 0 || S.precision >= integer_Part_ਚੌੜਾਈ) {
      mysignificand[mysignificand.ਮਾਪ_2() - 1] &= significand_ਨਕਾਬ;
    }
  } ਨਹੀਂਤਾਂ {
    std::fill_n(mysignificand.begin(), mysignificand.ਮਾਪ_2(), 0);
    // Always set integer bit to 1 for consistency in APFloat's internal
    // representation.
    mysignificand[0] = 1;
  }

  // We assume the last word holds the sign bit, the exponent, and potentially
  // some of the trailing significand field.
  uint64_t ਆਖਰੀ_ਸ਼ਬਦ = api.get_Raw_ਅੰਕੜੇ()[api.get_ਅੰਕ_ਸ਼ਬਦ() - 1];
  uint64_t myexponent =
      (ਆਖਰੀ_ਸ਼ਬਦ >> (trailing_significand_ਬਿੱਟ % 64)) & exponent_ਨਕਾਬ;

  initialize(&S);
  ਪ੍ਰਤਿਜ੍ਞਾ(part_ਗਿਣਤੀ() == mysignificand.ਮਾਪ_2());

  sign = S.hasSignedRepr
             ? static_cast<ਬਿਨਾਨਿਸ਼ਾਨੀ ਪੂਰਨ>(ਆਖਰੀ_ਸ਼ਬਦ >> ((S.ਮਾਪ_ਵਿੱਚ_ਬਿੱਟ - 1) % 64))
             : 0;

  ਤਾਰਕਿਕ all_zero_significand =
      has_significand && llvm::all_ਦਾ(mysignificand, equal_ਮੰਜ਼ਿਲ(0));

  ਤਾਰਕਿਕ is_zero = myexponent == 0 && all_zero_significand && S.hasZero;

  ਜੇ constexpr (S.nonFiniteBehavior == fltNonfiniteBehavior::IEEE754) {
    ਤਾਰਕਿਕ is_inf = ਝੂਠ;

    ਜੇ constexpr (S.has_Explicit_Integer_ਦੋਆਧਾਰੀ_ਅੰਕ) {
      // This is only used and tested for x87DoubleExtended
      static_assert(S.precision == 64);
      constexpr integerPart significand_ਨਕਾਬ_no_int_ਦੋਆਧਾਰੀ_ਅੰਕ =
          (uint64_t{1} << (trailing_significand_ਬਿੱਟ - 1)) - 1;
      ਅਚਲ integerPart myintegerbit =
          mysignificand[0] >> (trailing_significand_ਬਿੱਟ - 1);

      is_inf = myexponent - bias == ::exponentInf(S) && myintegerbit == 1 &&
               (mysignificand[0] & significand_ਨਕਾਬ_no_int_ਦੋਆਧਾਰੀ_ਅੰਕ) == 0;
    } ਨਹੀਂਤਾਂ {
      is_inf = myexponent - bias == ::exponentInf(S) && all_zero_significand;
    }

    ਜੇ (is_inf) {
      makeInf(sign);
      ਵਾਪਸ;
    }
  }

  ਤਾਰਕਿਕ is_nan = ਝੂਠ;

  ਜੇ constexpr (S.nanEncoding == fltNanEncoding::IEEE) {
    ਜੇ constexpr (S.has_Explicit_Integer_ਦੋਆਧਾਰੀ_ਅੰਕ) {
      // This is only used and tested for x87DoubleExtended
      static_assert(S.precision == 64);
      ਅਚਲ integerPart myintegerbit =
          mysignificand[0] >> (trailing_significand_ਬਿੱਟ - 1);
      constexpr integerPart significand_ਨਕਾਬ_no_int_ਦੋਆਧਾਰੀ_ਅੰਕ =
          (uint64_t{1} << (trailing_significand_ਬਿੱਟ - 1)) - 1;

      ਜੇ (myexponent - bias == ::exponentNaN(S) &&
          (mysignificand[0] & significand_ਨਕਾਬ_no_int_ਦੋਆਧਾਰੀ_ਅੰਕ) != 0) {
        // regular NaN and pseudoNaN
        is_nan = ਸੱਚ;
      } ਨਹੀਂਤਾਂ ਜੇ (myexponent - bias == ::exponentNaN(S) &&
                 (mysignificand[0] & significand_ਨਕਾਬ_no_int_ਦੋਆਧਾਰੀ_ਅੰਕ) == 0) {
        // pseudoinfinity
        is_nan = ਸੱਚ;
      } ਨਹੀਂਤਾਂ ਜੇ (myexponent - bias != ::exponentNaN(S) && myexponent != 0 &&
                 myintegerbit == 0) {
        // unnormal
        is_nan = ਸੱਚ;
      }
    } ਨਹੀਂਤਾਂ {
      is_nan = myexponent - bias == ::exponentNaN(S) && !all_zero_significand;
    }
  } ਨਹੀਂਤਾਂ ਜੇ constexpr (S.nanEncoding == fltNanEncoding::AllOnes) {
    ਤਾਰਕਿਕ all_ones_significand =
        std::all_ਦਾ(mysignificand.begin(), mysignificand.ਸਿਰਾ() - 1,
                    [](integerPart ਬਿੱਟ) { ਵਾਪਸ ਬਿੱਟ == ~integerPart{0}; }) &&
        (!significand_ਨਕਾਬ ||
         mysignificand[mysignificand.ਮਾਪ_2() - 1] == significand_ਨਕਾਬ);
    is_nan = myexponent - bias == ::exponentNaN(S) && all_ones_significand;
  } ਨਹੀਂਤਾਂ ਜੇ constexpr (S.nanEncoding == fltNanEncoding::NegativeZero) {
    is_nan = is_zero && sign;
  }

  ਜੇ (is_nan) {
    category = fcNaN;
    exponent = ::exponentNaN(S);
    std::copy_n(mysignificand.begin(), mysignificand.ਮਾਪ_2(),
                significandParts());
    ਵਾਪਸ;
  }

  ਜੇ (is_zero) {
    makeZero(sign);
    ਵਾਪਸ;
  }

  category = fc_ਸਾਧਾਰਨ;
  exponent = myexponent - bias;
  std::copy_n(mysignificand.begin(), mysignificand.ਮਾਪ_2(), significandParts());
  ਜੇ (myexponent == 0 && S.hasDenormals) // denormal
    exponent = S.minExponent;
  ਨਹੀਂਤਾਂ {
    ਜੇ constexpr (!S.has_Explicit_Integer_ਦੋਆਧਾਰੀ_ਅੰਕ) {
      significandParts()[mysignificand.ਮਾਪ_2() - 1] |= integer_ਦੋਆਧਾਰੀ_ਅੰਕ;
    }
  }
}

ਖਾਲੀ IEEEFloat::initFromQuadrupleAPInt(ਅਚਲ APInt &api) {
  initFromIEEEAPInt<APFloat_ਅਧਾਰ::semIEEEquad>(api);
}

ਖਾਲੀ IEEEFloat::initFromDoubleAPInt(ਅਚਲ APInt &api) {
  initFromIEEEAPInt<APFloat_ਅਧਾਰ::semIEEEdouble>(api);
}

ਖਾਲੀ IEEEFloat::initFromFloatAPInt(ਅਚਲ APInt &api) {
  initFromIEEEAPInt<APFloat_ਅਧਾਰ::semIEEEsingle>(api);
}

ਖਾਲੀ IEEEFloat::initFromBFloatAPInt(ਅਚਲ APInt &api) {
  initFromIEEEAPInt<APFloat_ਅਧਾਰ::semBFloat>(api);
}

ਖਾਲੀ IEEEFloat::initFromHalfAPInt(ਅਚਲ APInt &api) {
  initFromIEEEAPInt<APFloat_ਅਧਾਰ::semIEEEhalf>(api);
}

ਖਾਲੀ IEEEFloat::initFromFloat8E5M2APInt(ਅਚਲ APInt &api) {
  initFromIEEEAPInt<APFloat_ਅਧਾਰ::semFloat8E5M2>(api);
}

ਖਾਲੀ IEEEFloat::initFromFloat8E5M2FNUZAPInt(ਅਚਲ APInt &api) {
  initFromIEEEAPInt<APFloat_ਅਧਾਰ::semFloat8E5M2FNUZ>(api);
}

ਖਾਲੀ IEEEFloat::initFromFloat8E4M3APInt(ਅਚਲ APInt &api) {
  initFromIEEEAPInt<APFloat_ਅਧਾਰ::semFloat8E4M3>(api);
}

ਖਾਲੀ IEEEFloat::initFromFloat8E4M3FNAPInt(ਅਚਲ APInt &api) {
  initFromIEEEAPInt<APFloat_ਅਧਾਰ::semFloat8E4M3FN>(api);
}

ਖਾਲੀ IEEEFloat::initFromFloat8E4M3FNUZAPInt(ਅਚਲ APInt &api) {
  initFromIEEEAPInt<APFloat_ਅਧਾਰ::semFloat8E4M3FNUZ>(api);
}

ਖਾਲੀ IEEEFloat::initFromFloat8E4M3B11FNUZAPInt(ਅਚਲ APInt &api) {
  initFromIEEEAPInt<APFloat_ਅਧਾਰ::semFloat8E4M3B11FNUZ>(api);
}

ਖਾਲੀ IEEEFloat::initFromFloat8E3M4APInt(ਅਚਲ APInt &api) {
  initFromIEEEAPInt<APFloat_ਅਧਾਰ::semFloat8E3M4>(api);
}

ਖਾਲੀ IEEEFloat::initFromFloatTF32APInt(ਅਚਲ APInt &api) {
  initFromIEEEAPInt<APFloat_ਅਧਾਰ::semFloatTF32>(api);
}

ਖਾਲੀ IEEEFloat::initFromFloat6E3M2FNAPInt(ਅਚਲ APInt &api) {
  initFromIEEEAPInt<APFloat_ਅਧਾਰ::semFloat6E3M2FN>(api);
}

ਖਾਲੀ IEEEFloat::initFromFloat6E2M3FNAPInt(ਅਚਲ APInt &api) {
  initFromIEEEAPInt<APFloat_ਅਧਾਰ::semFloat6E2M3FN>(api);
}

ਖਾਲੀ IEEEFloat::initFromFloat4E2M1FNAPInt(ਅਚਲ APInt &api) {
  initFromIEEEAPInt<APFloat_ਅਧਾਰ::semFloat4E2M1FN>(api);
}

/// Treat api as containing the bits of a floating point number.
ਖਾਲੀ IEEEFloat::initFromAPInt(ਅਚਲ fltSemantics *Sem, ਅਚਲ APInt &api) {
  ਪ੍ਰਤਿਜ੍ਞਾ(api.get_ਦੋਆਧਾਰੀ_ਅੰਕ_ਚੌੜਾਈ() == Sem->ਮਾਪ_ਵਿੱਚ_ਬਿੱਟ);
  ਜੇ (Sem == &APFloat_ਅਧਾਰ::semIEEEhalf)
    ਵਾਪਸ initFromHalfAPInt(api);
  ਜੇ (Sem == &APFloat_ਅਧਾਰ::semBFloat)
    ਵਾਪਸ initFromBFloatAPInt(api);
  ਜੇ (Sem == &APFloat_ਅਧਾਰ::semIEEEsingle)
    ਵਾਪਸ initFromFloatAPInt(api);
  ਜੇ (Sem == &APFloat_ਅਧਾਰ::semIEEEdouble)
    ਵਾਪਸ initFromDoubleAPInt(api);
  ਜੇ (Sem == &APFloat_ਅਧਾਰ::semIEEEquad)
    ਵਾਪਸ initFromQuadrupleAPInt(api);
  ਜੇ (Sem == &APFloat_ਅਧਾਰ::semX87DoubleExtended)
    ਵਾਪਸ initFromF80LongDoubleAPInt(api);
  ਜੇ (Sem == &APFloat_ਅਧਾਰ::semPPCDoubleDoubleLegacy)
    ਵਾਪਸ initFromPPCDoubleDoubleLegacyAPInt(api);
  ਜੇ (Sem == &APFloat_ਅਧਾਰ::semFloat8E5M2)
    ਵਾਪਸ initFromFloat8E5M2APInt(api);
  ਜੇ (Sem == &APFloat_ਅਧਾਰ::semFloat8E5M2FNUZ)
    ਵਾਪਸ initFromFloat8E5M2FNUZAPInt(api);
  ਜੇ (Sem == &APFloat_ਅਧਾਰ::semFloat8E4M3)
    ਵਾਪਸ initFromFloat8E4M3APInt(api);
  ਜੇ (Sem == &APFloat_ਅਧਾਰ::semFloat8E4M3FN)
    ਵਾਪਸ initFromFloat8E4M3FNAPInt(api);
  ਜੇ (Sem == &APFloat_ਅਧਾਰ::semFloat8E4M3FNUZ)
    ਵਾਪਸ initFromFloat8E4M3FNUZAPInt(api);
  ਜੇ (Sem == &APFloat_ਅਧਾਰ::semFloat8E4M3B11FNUZ)
    ਵਾਪਸ initFromFloat8E4M3B11FNUZAPInt(api);
  ਜੇ (Sem == &APFloat_ਅਧਾਰ::semFloat8E3M4)
    ਵਾਪਸ initFromFloat8E3M4APInt(api);
  ਜੇ (Sem == &APFloat_ਅਧਾਰ::semFloatTF32)
    ਵਾਪਸ initFromFloatTF32APInt(api);
  ਜੇ (Sem == &APFloat_ਅਧਾਰ::semFloat8E8M0FNU)
    ਵਾਪਸ initFromFloat8E8M0FNUAPInt(api);
  ਜੇ (Sem == &APFloat_ਅਧਾਰ::semFloat8E5M3FNU)
    ਵਾਪਸ initFromFloat8E5M3FNUAPInt(api);
  ਜੇ (Sem == &APFloat_ਅਧਾਰ::semFloat6E3M2FN)
    ਵਾਪਸ initFromFloat6E3M2FNAPInt(api);
  ਜੇ (Sem == &APFloat_ਅਧਾਰ::semFloat6E2M3FN)
    ਵਾਪਸ initFromFloat6E2M3FNAPInt(api);
  ਜੇ (Sem == &APFloat_ਅਧਾਰ::semFloat4E2M1FN)
    ਵਾਪਸ initFromFloat4E2M1FNAPInt(api);

  llvm_unreachable("unsupported semantics");
}

/// Make this number the largest magnitude normal number in the given
/// semantics.
ਖਾਲੀ IEEEFloat::make_ਸਭ_ਤੋਂ_ਵੱਡਾ(ਤਾਰਕਿਕ Negative) {
  ਜੇ (Negative && !semantics->hasSignedRepr)
    llvm_unreachable(
        "This floating point format does not support signed values");
  // We want (in interchange format):
  //   sign = {Negative}
  //   exponent = 1..10
  //   significand = 1..1
  category = fc_ਸਾਧਾਰਨ;
  sign = Negative;
  exponent = semantics->maxExponent;

  // Use memset to set all but the highest integerPart to all ones.
  integerPart *significand = significandParts();
  ਬਿਨਾਨਿਸ਼ਾਨੀ Part_ਗਿਣਤੀ = part_ਗਿਣਤੀ();
  memset(significand, 0xFF, ਆਕਾਰ(integerPart)*(Part_ਗਿਣਤੀ - 1));

  // Set the high integerPart especially setting all unused top bits for
  // internal consistency.
  ਅਚਲ ਬਿਨਾਨਿਸ਼ਾਨੀ ਅੰਕ_ਅਣਵਰਤਿਆ_ਉੱਚਾ_ਬਿੱਟ =
    Part_ਗਿਣਤੀ*integer_Part_ਚੌੜਾਈ - semantics->precision;
  significand[Part_ਗਿਣਤੀ - 1] = (ਅੰਕ_ਅਣਵਰਤਿਆ_ਉੱਚਾ_ਬਿੱਟ < integer_Part_ਚੌੜਾਈ)
                                   ? (~integerPart(0) >> ਅੰਕ_ਅਣਵਰਤਿਆ_ਉੱਚਾ_ਬਿੱਟ)
                                   : 0;
  ਜੇ (semantics->nonFiniteBehavior == fltNonfiniteBehavior::NanOnly &&
      semantics->nanEncoding == fltNanEncoding::AllOnes &&
      (semantics->precision > 1))
    significand[0] &= ~integerPart(1);
}

/// Make this number the smallest magnitude denormal number in the given
/// semantics.
ਖਾਲੀ IEEEFloat::make_ਸਭ_ਤੋਂ_ਛੋਟਾ(ਤਾਰਕਿਕ Negative) {
  ਜੇ (Negative && !semantics->hasSignedRepr)
    llvm_unreachable(
        "This floating point format does not support signed values");
  // We want (in interchange format):
  //   sign = {Negative}
  //   exponent = 0..0
  //   significand = 0..01
  category = fc_ਸਾਧਾਰਨ;
  sign = Negative;
  exponent = semantics->minExponent;
  APInt::tc_ਸੈੱਟ(significandParts(), 1, part_ਗਿਣਤੀ());
}

ਖਾਲੀ IEEEFloat::make_ਸਭ_ਤੋਂ_ਛੋਟਾ_Normalized(ਤਾਰਕਿਕ Negative) {
  ਜੇ (Negative && !semantics->hasSignedRepr)
    llvm_unreachable(
        "This floating point format does not support signed values");
  // We want (in interchange format):
  //   sign = {Negative}
  //   exponent = 0..0
  //   significand = 10..0

  category = fc_ਸਾਧਾਰਨ;
  zeroSignificand();
  sign = Negative;
  exponent = semantics->minExponent;
  APInt::tc_ਸੈੱਟ_ਦੋਆਧਾਰੀ_ਅੰਕ(significandParts(), semantics->precision - 1);
}

IEEEFloat::IEEEFloat(ਅਚਲ fltSemantics &Sem, ਅਚਲ APInt &API) {
  initFromAPInt(&Sem, API);
}

IEEEFloat::IEEEFloat(ਦਸ਼ਮਲਵ f) {
  initFromAPInt(&APFloat_ਅਧਾਰ::semIEEEsingle, APInt::float_ਮੰਜ਼ਿਲ_ਬਿੱਟ(f));
}

IEEEFloat::IEEEFloat(ਦੁੱਗਣਾ d) {
  initFromAPInt(&APFloat_ਅਧਾਰ::semIEEEdouble, APInt::double_ਮੰਜ਼ਿਲ_ਬਿੱਟ(d));
}

ਨਾਮਾਕਾਸ਼ {
  ਖਾਲੀ append(Small_ਸਦਿਸ਼_Impl<ਅੱਖਰ> &ਬਫਰ, StringRef Str) {
    ਬਫਰ.append(Str.begin(), Str.ਸਿਰਾ());
  }

  /// Removes data from the given significand until it is no more
  /// precise than is required for the desired precision.
  ਖਾਲੀ Adjust_ਮੰਜ਼ਿਲ_Precision(APInt &significand,
                         ਪੂਰਨ &ਘਾਤਾਂਕ, ਬਿਨਾਨਿਸ਼ਾਨੀ FormatPrecision) {
    ਬਿਨਾਨਿਸ਼ਾਨੀ ਬਿੱਟ = significand.get_ਸਰਗਰਮ_ਬਿੱਟ();

    // 196/59 is a very slight overestimate of lg_2(10).
    ਬਿਨਾਨਿਸ਼ਾਨੀ ਬਿੱਟ_Required = (FormatPrecision * 196 + 58) / 59;

    ਜੇ (ਬਿੱਟ <= ਬਿੱਟ_Required) ਵਾਪਸ;

    ਬਿਨਾਨਿਸ਼ਾਨੀ tensRemovable = (ਬਿੱਟ - ਬਿੱਟ_Required) * 59 / 196;
    ਜੇ (!tensRemovable) ਵਾਪਸ;

    ਘਾਤਾਂਕ += tensRemovable;

    APInt ਭਾਜਕ(significand.get_ਦੋਆਧਾਰੀ_ਅੰਕ_ਚੌੜਾਈ(), 1);
    APInt powten(significand.get_ਦੋਆਧਾਰੀ_ਅੰਕ_ਚੌੜਾਈ(), 10);
    ਜਦੋਂ (ਸੱਚ) {
      ਜੇ (tensRemovable & 1)
        ਭਾਜਕ *= powten;
      tensRemovable >>= 1;
      ਜੇ (!tensRemovable) ਰੋਕੋ;
      powten *= powten;
    }

    significand = significand.udiv(ਭਾਜਕ);

    // Truncate the significand down to its active bit count.
    significand = significand.trunc(significand.get_ਸਰਗਰਮ_ਬਿੱਟ());
  }


  ਖਾਲੀ Adjust_ਮੰਜ਼ਿਲ_Precision(Small_ਸਦਿਸ਼_Impl<ਅੱਖਰ> &ਬਫਰ_2,
                         ਪੂਰਨ &ਘਾਤਾਂਕ, ਬਿਨਾਨਿਸ਼ਾਨੀ FormatPrecision) {
    ਬਿਨਾਨਿਸ਼ਾਨੀ N = ਬਫਰ_2.ਮਾਪ_2();
    ਜੇ (N <= FormatPrecision) ਵਾਪਸ;

    // The most significant figures are the last ones in the buffer.
    ਬਿਨਾਨਿਸ਼ਾਨੀ ਪਹਿਲਾ_Significant = N - FormatPrecision;

    // Round.
    // FIXME: this probably shouldn't use 'round half up'.

    // Rounding down is just a truncation, except we also want to drop
    // trailing zeros from the new result.
    ਜੇ (ਬਫਰ_2[ਪਹਿਲਾ_Significant - 1] < '5') {
      ਜਦੋਂ (ਪਹਿਲਾ_Significant < N && ਬਫਰ_2[ਪਹਿਲਾ_Significant] == '0')
        ਪਹਿਲਾ_Significant++;

      ਘਾਤਾਂਕ += ਪਹਿਲਾ_Significant;
      ਬਫਰ_2.erase(&ਬਫਰ_2[0], &ਬਫਰ_2[ਪਹਿਲਾ_Significant]);
      ਵਾਪਸ;
    }

    // Rounding up requires a decimal add-with-carry.  If we continue
    // the carry, the newly-introduced zeros will just be truncated.
    ਲਈ (ਬਿਨਾਨਿਸ਼ਾਨੀ I = ਪਹਿਲਾ_Significant; I != N; ++I) {
      ਜੇ (ਬਫਰ_2[I] == '9') {
        ਪਹਿਲਾ_Significant++;
      } ਨਹੀਂਤਾਂ {
        ਬਫਰ_2[I]++;
        ਰੋਕੋ;
      }
    }

    // If we carried through, we have exactly one digit of precision.
    ਜੇ (ਪਹਿਲਾ_Significant == N) {
      ਘਾਤਾਂਕ += ਪਹਿਲਾ_Significant;
      ਬਫਰ_2.clear();
      ਬਫਰ_2.ਧੱਕੋ_ਪਿੱਠ('1');
      ਵਾਪਸ;
    }

    ਘਾਤਾਂਕ += ਪਹਿਲਾ_Significant;
    ਬਫਰ_2.erase(&ਬਫਰ_2[0], &ਬਫਰ_2[ਪਹਿਲਾ_Significant]);
  }

  ਖਾਲੀ ਮੰਜ਼ਿਲ_String_Impl(Small_ਸਦਿਸ਼_Impl<ਅੱਖਰ> &Str, ਅਚਲ ਤਾਰਕਿਕ isNeg, ਪੂਰਨ ਘਾਤਾਂਕ,
                    APInt significand, ਬਿਨਾਨਿਸ਼ਾਨੀ FormatPrecision,
                    ਬਿਨਾਨਿਸ਼ਾਨੀ FormatMaxPadding, ਤਾਰਕਿਕ TruncateZero) {
    ਅਚਲ ਪੂਰਨ semanticsPrecision = significand.get_ਦੋਆਧਾਰੀ_ਅੰਕ_ਚੌੜਾਈ();

    ਜੇ (isNeg)
      Str.ਧੱਕੋ_ਪਿੱਠ('-');

    // Set FormatPrecision if zero.  We want to do this before we
    // truncate trailing zeros, as those are part of the precision.
    ਜੇ (!FormatPrecision) {
      // We use enough digits so the number can be round-tripped back to an
      // APFloat. The formula comes from "How to Print Floating-Point Numbers
      // Accurately" by Steele and White.
      // FIXME: Using a formula based purely on the precision is conservative;
      // we can print fewer digits depending on the actual value being printed.

      // FormatPrecision = 2 + floor(significandBits / lg_2(10))
      FormatPrecision = 2 + semanticsPrecision * 59 / 196;
    }

    // Ignore trailing binary zeros.
    ਪੂਰਨ trailingZeros = significand.countr_zero();
    ਘਾਤਾਂਕ += trailingZeros;
    significand.lshr_ਵਿੱਚ_ਰੱਖੋ(trailingZeros);

    // Change the exponent from 2^e to 10^e.
    ਜੇ (ਘਾਤਾਂਕ == 0) {
      // Nothing to do.
    } ਨਹੀਂਤਾਂ ਜੇ (ਘਾਤਾਂਕ > 0) {
      // Just shift left.
      significand = significand.zext(semanticsPrecision + ਘਾਤਾਂਕ);
      significand <<= ਘਾਤਾਂਕ;
      ਘਾਤਾਂਕ = 0;
    } ਨਹੀਂਤਾਂ { /* exp < 0 */
      ਪੂਰਨ texp = -ਘਾਤਾਂਕ;

      // We transform this using the identity:
      //   (N)(2^-e) == (N)(5^e)(10^-e)
      // This means we have to multiply N (the significand) by 5^e.
      // To avoid overflow, we have to operate on numbers large
      // enough to store N * 5^e:
      //   log2(N * 5^e) == log2(N) + e * log2(5)
      //                 <= semantics->precision + e * 137 / 59
      //   (log_2(5) ~ 2.321928 < 2.322034 ~ 137/59)

      ਬਿਨਾਨਿਸ਼ਾਨੀ precision = semanticsPrecision + (137 * texp + 136) / 59;

      // Multiply significand by 5^e.
      //   N * 5^0101 == N * 5^(1*1) * 5^(0*2) * 5^(1*4) * 5^(0*8)
      significand = significand.zext(precision);
      APInt five_ਮੰਜ਼ਿਲ_the_i(precision, 5);
      ਜਦੋਂ (ਸੱਚ) {
        ਜੇ (texp & 1)
          significand *= five_ਮੰਜ਼ਿਲ_the_i;

        texp >>= 1;
        ਜੇ (!texp)
          ਰੋਕੋ;
        five_ਮੰਜ਼ਿਲ_the_i *= five_ਮੰਜ਼ਿਲ_the_i;
      }
    }

    Adjust_ਮੰਜ਼ਿਲ_Precision(significand, ਘਾਤਾਂਕ, FormatPrecision);

    Small_ਸਦਿਸ਼<ਅੱਖਰ, 256> ਬਫਰ_2;

    // Fill the buffer.
    ਬਿਨਾਨਿਸ਼ਾਨੀ precision = significand.get_ਦੋਆਧਾਰੀ_ਅੰਕ_ਚੌੜਾਈ();
    ਜੇ (precision < 4) {
      // We need enough precision to store the value 10.
      precision = 4;
      significand = significand.zext(precision);
    }
    APInt ten(precision, 10);
    APInt digit(precision, 0);

    ਤਾਰਕਿਕ ਵਿੱਚ_Trail = ਸੱਚ;
    ਜਦੋਂ (significand != 0) {
      // digit <- significand % 10
      // significand <- significand / 10
      APInt::udivrem(significand, ten, significand, digit);

      ਬਿਨਾਨਿਸ਼ਾਨੀ d = digit.get_ZExt_ਮੁੱਲ();

      // Drop trailing zeros.
      ਜੇ (ਵਿੱਚ_Trail && !d)
        ਘਾਤਾਂਕ++;
      ਨਹੀਂਤਾਂ {
        ਬਫਰ_2.ਧੱਕੋ_ਪਿੱਠ((ਅੱਖਰ) ('0' + d));
        ਵਿੱਚ_Trail = ਝੂਠ;
      }
    }

    ਪ੍ਰਤਿਜ੍ਞਾ(!ਬਫਰ_2.ਖਾਲੀ_ਥਾਵਾਂ() && "no characters in buffer!");

    // Drop down to FormatPrecision.
    // TODO: don't do more precise calculations above than are required.
    Adjust_ਮੰਜ਼ਿਲ_Precision(ਬਫਰ_2, ਘਾਤਾਂਕ, FormatPrecision);

    ਬਿਨਾਨਿਸ਼ਾਨੀ NDigits = ਬਫਰ_2.ਮਾਪ_2();

    // Check whether we should use scientific notation.
    ਤਾਰਕਿਕ FormatScientific;
    ਜੇ (!FormatMaxPadding) {
      FormatScientific = ਸੱਚ;
    } ਨਹੀਂਤਾਂ {
      ਜੇ (ਘਾਤਾਂਕ >= 0) {
        // 765e3 --> 765000
        //              ^^^
        // But we shouldn't make the number look more precise than it is.
        FormatScientific = ((ਬਿਨਾਨਿਸ਼ਾਨੀ) ਘਾਤਾਂਕ > FormatMaxPadding ||
                            NDigits + (ਬਿਨਾਨਿਸ਼ਾਨੀ) ਘਾਤਾਂਕ > FormatPrecision);
      } ਨਹੀਂਤਾਂ {
        // Power of the most significant digit.
        ਪੂਰਨ MSD = ਘਾਤਾਂਕ + (ਪੂਰਨ) (NDigits - 1);
        ਜੇ (MSD >= 0) {
          // 765e-2 == 7.65
          FormatScientific = ਝੂਠ;
        } ਨਹੀਂਤਾਂ {
          // 765e-5 == 0.00765
          //           ^ ^^
          FormatScientific = ((ਬਿਨਾਨਿਸ਼ਾਨੀ) -MSD) > FormatMaxPadding;
        }
      }
    }

    // Scientific formatting is pretty straightforward.
    ਜੇ (FormatScientific) {
      ਘਾਤਾਂਕ += (NDigits - 1);

      Str.ਧੱਕੋ_ਪਿੱਠ(ਬਫਰ_2[NDigits-1]);
      Str.ਧੱਕੋ_ਪਿੱਠ('.');
      ਜੇ (NDigits == 1 && TruncateZero)
        Str.ਧੱਕੋ_ਪਿੱਠ('0');
      ਨਹੀਂਤਾਂ
        ਲਈ (ਬਿਨਾਨਿਸ਼ਾਨੀ I = 1; I != NDigits; ++I)
          Str.ਧੱਕੋ_ਪਿੱਠ(ਬਫਰ_2[NDigits-1-I]);
      // Fill with zeros up to FormatPrecision.
      ਜੇ (!TruncateZero && FormatPrecision > NDigits - 1)
        Str.append(FormatPrecision - NDigits + 1, '0');
      // For !TruncateZero we use lower 'e'.
      Str.ਧੱਕੋ_ਪਿੱਠ(TruncateZero ? 'E' : 'e');

      Str.ਧੱਕੋ_ਪਿੱਠ(ਘਾਤਾਂਕ >= 0 ? '+' : '-');
      ਜੇ (ਘਾਤਾਂਕ < 0)
        ਘਾਤਾਂਕ = -ਘਾਤਾਂਕ;
      Small_ਸਦਿਸ਼<ਅੱਖਰ, 6> expbuf;
      ਕਰੋ {
        expbuf.ਧੱਕੋ_ਪਿੱਠ((ਅੱਖਰ) ('0' + (ਘਾਤਾਂਕ % 10)));
        ਘਾਤਾਂਕ /= 10;
      } ਜਦੋਂ (ਘਾਤਾਂਕ);
      // Exponent always at least two digits if we do not truncate zeros.
      ਜੇ (!TruncateZero && expbuf.ਮਾਪ_2() < 2)
        expbuf.ਧੱਕੋ_ਪਿੱਠ('0');
      ਲਈ (ਬਿਨਾਨਿਸ਼ਾਨੀ I = 0, E = expbuf.ਮਾਪ_2(); I != E; ++I)
        Str.ਧੱਕੋ_ਪਿੱਠ(expbuf[E-1-I]);
      ਵਾਪਸ;
    }

    // Non-scientific, positive exponents.
    ਜੇ (ਘਾਤਾਂਕ >= 0) {
      ਲਈ (ਬਿਨਾਨਿਸ਼ਾਨੀ I = 0; I != NDigits; ++I)
        Str.ਧੱਕੋ_ਪਿੱਠ(ਬਫਰ_2[NDigits-1-I]);
      ਲਈ (ਬਿਨਾਨਿਸ਼ਾਨੀ I = 0; I != (ਬਿਨਾਨਿਸ਼ਾਨੀ) ਘਾਤਾਂਕ; ++I)
        Str.ਧੱਕੋ_ਪਿੱਠ('0');
      ਵਾਪਸ;
    }

    // Non-scientific, negative exponents.

    // The number of digits to the left of the decimal point.
    ਪੂਰਨ NWholeDigits = ਘਾਤਾਂਕ + (ਪੂਰਨ) NDigits;

    ਬਿਨਾਨਿਸ਼ਾਨੀ I = 0;
    ਜੇ (NWholeDigits > 0) {
      ਲਈ (; I != (ਬਿਨਾਨਿਸ਼ਾਨੀ) NWholeDigits; ++I)
        Str.ਧੱਕੋ_ਪਿੱਠ(ਬਫਰ_2[NDigits-I-1]);
      Str.ਧੱਕੋ_ਪਿੱਠ('.');
    } ਨਹੀਂਤਾਂ {
      ਬਿਨਾਨਿਸ਼ਾਨੀ NZeros = 1 + (ਬਿਨਾਨਿਸ਼ਾਨੀ) -NWholeDigits;

      Str.ਧੱਕੋ_ਪਿੱਠ('0');
      Str.ਧੱਕੋ_ਪਿੱਠ('.');
      ਲਈ (ਬਿਨਾਨਿਸ਼ਾਨੀ Z = 1; Z != NZeros; ++Z)
        Str.ਧੱਕੋ_ਪਿੱਠ('0');
    }

    ਲਈ (; I != NDigits; ++I)
      Str.ਧੱਕੋ_ਪਿੱਠ(ਬਫਰ_2[NDigits-I-1]);

  }
} // namespace

ਖਾਲੀ IEEEFloat::ਮੰਜ਼ਿਲ_String(Small_ਸਦਿਸ਼_Impl<ਅੱਖਰ> &Str, ਬਿਨਾਨਿਸ਼ਾਨੀ FormatPrecision,
                         ਬਿਨਾਨਿਸ਼ਾਨੀ FormatMaxPadding, ਤਾਰਕਿਕ TruncateZero) ਅਚਲ {
  ਚੋਣ (category) {
  ਹਾਲਤ fcInfinity:
    ਜੇ (isNegative())
      ਵਾਪਸ append(Str, "-Inf");
    ਨਹੀਂਤਾਂ
      ਵਾਪਸ append(Str, "+Inf");

  ਹਾਲਤ fcNaN: ਵਾਪਸ append(Str, "NaN");

  ਹਾਲਤ fcZero:
    ਜੇ (isNegative())
      Str.ਧੱਕੋ_ਪਿੱਠ('-');

    ਜੇ (!FormatMaxPadding) {
      ਜੇ (TruncateZero)
        append(Str, "0.0E+0");
      ਨਹੀਂਤਾਂ {
        append(Str, "0.0");
        ਜੇ (FormatPrecision > 1)
          Str.append(FormatPrecision - 1, '0');
        append(Str, "e+00");
      }
    } ਨਹੀਂਤਾਂ {
      Str.ਧੱਕੋ_ਪਿੱਠ('0');
    }
    ਵਾਪਸ;

  ਹਾਲਤ fc_ਸਾਧਾਰਨ:
    ਰੋਕੋ;
  }

  // Decompose the number into an APInt and an exponent.
  ਪੂਰਨ ਘਾਤਾਂਕ = exponent - ((ਪੂਰਨ) semantics->precision - 1);
  APInt significand(
      semantics->precision,
      ArrayRef(significandParts(), part_ਗਿਣਤੀ_For_ਬਿੱਟ(semantics->precision)));

  ਮੰਜ਼ਿਲ_String_Impl(Str, isNegative(), ਘਾਤਾਂਕ, significand, FormatPrecision,
               FormatMaxPadding, TruncateZero);

}

ਪੂਰਨ IEEEFloat::getExactLog2Abs() ਅਚਲ {
  ਜੇ (!isFinite() || isZero())
    ਵਾਪਸ INT_MIN;

  ਅਚਲ integerPart *Parts = significandParts();
  ਅਚਲ ਪੂਰਨ Part_ਗਿਣਤੀ = part_ਗਿਣਤੀ_For_ਬਿੱਟ(semantics->precision);

  ਪੂਰਨ ਕੱਢੋ_ਗਿਣਤੀ = 0;
  ਲਈ (ਪੂਰਨ i = 0; i < Part_ਗਿਣਤੀ; ++i) {
    ਕੱਢੋ_ਗਿਣਤੀ += llvm::popcount(Parts[i]);
    ਜੇ (ਕੱਢੋ_ਗਿਣਤੀ > 1)
      ਵਾਪਸ INT_MIN;
  }

  ਜੇ (exponent != semantics->minExponent)
    ਵਾਪਸ exponent;

  ਪੂਰਨ CountrParts = 0;
  ਲਈ (ਪੂਰਨ i = 0; i < Part_ਗਿਣਤੀ;
       ++i, CountrParts += APInt::APINT_ਬਿੱਟ_ਪ੍ਰਤੀ_ਸ਼ਬਦ) {
    ਜੇ (Parts[i] != 0) {
      ਵਾਪਸ exponent - semantics->precision + CountrParts +
             llvm::countr_zero(Parts[i]) + 1;
    }
  }

  llvm_unreachable("didn't find the set bit");
}

ਤਾਰਕਿਕ IEEEFloat::isSignaling() ਅਚਲ {
  ਜੇ (!isNaN())
    ਵਾਪਸ ਝੂਠ;
  ਜੇ (semantics->nonFiniteBehavior == fltNonfiniteBehavior::NanOnly ||
      semantics->nonFiniteBehavior == fltNonfiniteBehavior::FiniteOnly)
    ਵਾਪਸ ਝੂਠ;

  // IEEE-754R 2008 6.2.1: A signaling NaN bit string should be encoded with the
  // first bit of the trailing significand being 0.
  ਵਾਪਸ !APInt::tc_ਕੱਢੋ_ਦੋਆਧਾਰੀ_ਅੰਕ(significandParts(), semantics->precision - 2);
}

/// IEEE-754R 2008 5.3.1: nextUp/nextDown.
///
/// *NOTE* since nextDown(x) = -nextUp(-x), we only implement nextUp with
/// appropriate sign switching before/after the computation.
APFloat::op_ਸਥਿਤੀ IEEEFloat::ਅਗਲਾ(ਤਾਰਕਿਕ ਅਗਲਾ_ਥੱਲੇ) {
  // If we are performing nextDown, swap sign so we have -x.
  ਜੇ (ਅਗਲਾ_ਥੱਲੇ)
    changeSign();

  // Compute nextUp(x)
  op_ਸਥਿਤੀ ਨਤੀਜਾ_2 = opOK;

  // Handle each float category separately.
  ਚੋਣ (category) {
  ਹਾਲਤ fcInfinity:
    // nextUp(+inf) = +inf
    ਜੇ (!isNegative())
      ਰੋਕੋ;
    // nextUp(-inf) = -getLargest()
    make_ਸਭ_ਤੋਂ_ਵੱਡਾ(ਸੱਚ);
    ਰੋਕੋ;
  ਹਾਲਤ fcNaN:
    // IEEE-754R 2008 6.2 Par 2: nextUp(sNaN) = qNaN. Set Invalid flag.
    // IEEE-754R 2008 6.2: nextUp(qNaN) = qNaN. Must be identity so we do not
    //                     change the payload.
    ਜੇ (isSignaling()) {
      ਨਤੀਜਾ_2 = opInvalidOp;
      // For consistency, propagate the sign of the sNaN to the qNaN.
      makeNaN(ਝੂਠ, isNegative(), ਖਾਲੀ_ਸੂਚਕ);
    }
    ਰੋਕੋ;
  ਹਾਲਤ fcZero:
    // nextUp(pm 0) = +getSmallest()
    make_ਸਭ_ਤੋਂ_ਛੋਟਾ(ਝੂਠ);
    ਰੋਕੋ;
  ਹਾਲਤ fc_ਸਾਧਾਰਨ:
    // nextUp(-getSmallest()) = -0
    ਜੇ (is_ਸਭ_ਤੋਂ_ਛੋਟਾ() && isNegative()) {
      APInt::tc_ਸੈੱਟ(significandParts(), 0, part_ਗਿਣਤੀ());
      category = fcZero;
      exponent = 0;
      ਜੇ (semantics->nanEncoding == fltNanEncoding::NegativeZero)
        sign = ਝੂਠ;
      ਜੇ (!semantics->hasZero)
        make_ਸਭ_ਤੋਂ_ਛੋਟਾ_Normalized(ਝੂਠ);
      ਰੋਕੋ;
    }

    ਜੇ (is_ਸਭ_ਤੋਂ_ਵੱਡਾ() && !isNegative()) {
      ਜੇ (semantics->nonFiniteBehavior == fltNonfiniteBehavior::NanOnly) {
        // nextUp(getLargest()) == NAN
        makeNaN();
        ਰੋਕੋ;
      } ਨਹੀਂਤਾਂ ਜੇ (semantics->nonFiniteBehavior ==
                 fltNonfiniteBehavior::FiniteOnly) {
        // nextUp(getLargest()) == getLargest()
        ਰੋਕੋ;
      } ਨਹੀਂਤਾਂ {
        // nextUp(getLargest()) == INFINITY
        APInt::tc_ਸੈੱਟ(significandParts(), 0, part_ਗਿਣਤੀ());
        category = fcInfinity;
        exponent = semantics->maxExponent + 1;
        ਰੋਕੋ;
      }
    }

    // nextUp(normal) == normal + inc.
    ਜੇ (isNegative()) {
      // If we are negative, we need to decrement the significand.

      // We only cross a binade boundary that requires adjusting the exponent
      // if:
      //   1. exponent != semantics->minExponent. This implies we are not in the
      //   smallest binade or are dealing with denormals.
      //   2. Our significand excluding the integral bit is all zeros.
      ਤਾਰਕਿਕ WillCrossBinadeBoundary =
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
      APInt::tcDecrement(Parts, part_ਗਿਣਤੀ());

      ਜੇ (WillCrossBinadeBoundary) {
        // Our result is a normal number. Do the following:
        // 1. Set the integral bit to 1.
        // 2. Decrement the exponent.
        APInt::tc_ਸੈੱਟ_ਦੋਆਧਾਰੀ_ਅੰਕ(Parts, semantics->precision - 1);
        exponent--;
      }
    } ਨਹੀਂਤਾਂ {
      // If we are positive, we need to increment the significand.

      // We only cross a binade boundary that requires adjusting the exponent if
      // the input is not a denormal and all of said input's significand bits
      // are set. If all of said conditions are true: clear the significand, set
      // the integral bit to 1, and increment the exponent. If we have a
      // denormal always increment since moving denormals and the numbers in the
      // smallest normal binade have the same exponent in our representation.
      // If there are only exponents, any increment always crosses the
      // BinadeBoundary.
      ਤਾਰਕਿਕ WillCrossBinadeBoundary = !APFloat::hasSignificand(*semantics) ||
                                     (!isDenormal() && isSignificandAllOnes());

      ਜੇ (WillCrossBinadeBoundary) {
        integerPart *Parts = significandParts();
        APInt::tc_ਸੈੱਟ(Parts, 0, part_ਗਿਣਤੀ());
        APInt::tc_ਸੈੱਟ_ਦੋਆਧਾਰੀ_ਅੰਕ(Parts, semantics->precision - 1);
        ਪ੍ਰਤਿਜ੍ਞਾ(exponent != semantics->maxExponent &&
               "We can not increment an exponent beyond the maxExponent allowed"
               " by the given floating point semantics.");
        exponent++;
      } ਨਹੀਂਤਾਂ {
        incrementSignificand();
      }
    }
    ਰੋਕੋ;
  }

  // If we are performing nextDown, swap sign so we have -nextUp(-x)
  ਜੇ (ਅਗਲਾ_ਥੱਲੇ)
    changeSign();

  ਵਾਪਸ ਨਤੀਜਾ_2;
}

APInt IEEEFloat::getNaNPayload() ਅਚਲ {
  ਪ੍ਰਤਿਜ੍ਞਾ(isNaN() && "Can only be called on NaN values");
  // Number of bits in the payload, excluding the (maybe implied) integer bit.
  ਬਿਨਾਨਿਸ਼ਾਨੀ ਬਿੱਟ_2 = semantics->precision - 1;
  ਵਾਪਸ APInt(ਬਿੱਟ_2, ArrayRef(significandParts(), part_ਗਿਣਤੀ_For_ਬਿੱਟ(ਬਿੱਟ_2)));
}

APFloat_ਅਧਾਰ::ExponentType IEEEFloat::exponentNaN() ਅਚਲ {
  ਵਾਪਸ ::exponentNaN(*semantics);
}

APFloat_ਅਧਾਰ::ExponentType IEEEFloat::exponentInf() ਅਚਲ {
  ਵਾਪਸ ::exponentInf(*semantics);
}

APFloat_ਅਧਾਰ::ExponentType IEEEFloat::exponentZero() ਅਚਲ {
  ਵਾਪਸ ::exponentZero(*semantics);
}

ਖਾਲੀ IEEEFloat::makeInf(ਤਾਰਕਿਕ Negative) {
  ਜੇ (semantics->nonFiniteBehavior == fltNonfiniteBehavior::FiniteOnly)
    llvm_unreachable("This floating point format does not support Inf");

  ਜੇ (semantics->nonFiniteBehavior == fltNonfiniteBehavior::NanOnly) {
    // There is no Inf, so make NaN instead.
    makeNaN(ਝੂਠ, Negative);
    ਵਾਪਸ;
  }
  category = fcInfinity;
  sign = Negative;
  exponent = exponentInf();
  APInt::tc_ਸੈੱਟ(significandParts(), 0, part_ਗਿਣਤੀ());
}

ਖਾਲੀ IEEEFloat::makeZero(ਤਾਰਕਿਕ Negative) {
  ਜੇ (!semantics->hasZero)
    llvm_unreachable("This floating point format does not support Zero");

  category = fcZero;
  sign = Negative;
  ਜੇ (semantics->nanEncoding == fltNanEncoding::NegativeZero) {
    // Merge negative zero to positive because 0b10000...000 is used for NaN
    sign = ਝੂਠ;
  }
  exponent = exponentZero();
  APInt::tc_ਸੈੱਟ(significandParts(), 0, part_ਗਿਣਤੀ());
}

ਖਾਲੀ IEEEFloat::makeQuiet() {
  ਪ੍ਰਤਿਜ੍ਞਾ(isNaN());
  ਜੇ (semantics->nonFiniteBehavior != fltNonfiniteBehavior::NanOnly)
    APInt::tc_ਸੈੱਟ_ਦੋਆਧਾਰੀ_ਅੰਕ(significandParts(), semantics->precision - 2);
}

ਪੂਰਨ ilogb(ਅਚਲ IEEEFloat &Arg) {
  ਜੇ (Arg.isNaN())
    ਵਾਪਸ APFloat::IEK_NaN;
  ਜੇ (Arg.isZero())
    ਵਾਪਸ APFloat::IEK_Zero;
  ਜੇ (Arg.isInfinity())
    ਵਾਪਸ APFloat::IEK_Inf;
  ਜੇ (!Arg.isDenormal())
    ਵਾਪਸ Arg.exponent;

  IEEEFloat Normalized(Arg);
  ਪੂਰਨ Significand_ਬਿੱਟ = Arg.getSemantics().precision - 1;

  Normalized.exponent += Significand_ਬਿੱਟ;
  Normalized.normalize(APFloat::rm_ਸਭ_ਤੋਂ_ਨੇੜੇ_Ties_ਮੰਜ਼ਿਲ_Even, lfExactlyZero);
  ਵਾਪਸ Normalized.exponent - Significand_ਬਿੱਟ;
}

IEEEFloat scalbn(IEEEFloat X, ਪੂਰਨ Exp, rounding_ਬਹੁਲਕ Rounding_ਬਹੁਲਕ) {
  ਸਵੈਚਾਲਿਤ MaxExp = X.getSemantics().maxExponent;
  ਸਵੈਚਾਲਿਤ MinExp = X.getSemantics().minExponent;

  // If Exp is wildly out-of-scale, simply adding it to X.exponent will
  // overflow; clamp it to a safe range before adding, but ensure that the range
  // is large enough that the clamp does not change the result. The range we
  // need to support is the difference between the largest possible exponent and
  // the normalized exponent of half the smallest denormal.

  ਪੂਰਨ Significand_ਬਿੱਟ = X.getSemantics().precision - 1;
  ਪੂਰਨ MaxIncrement = MaxExp - (MinExp - Significand_ਬਿੱਟ) + 1;

  // Clamp to one past the range ends to let normalize handle overlflow.
  X.exponent += std::clamp(Exp, -MaxIncrement - 1, MaxIncrement);
  X.normalize(Rounding_ਬਹੁਲਕ, lfExactlyZero);
  ਜੇ (X.isNaN())
    X.makeQuiet();
  ਵਾਪਸ X;
}

IEEEFloat frexp(ਅਚਲ IEEEFloat &Val, ਪੂਰਨ &Exp, rounding_ਬਹੁਲਕ RM) {
  Exp = ilogb(Val);

  // Quiet signalling nans.
  ਜੇ (Exp == APFloat::IEK_NaN) {
    IEEEFloat Quiet(Val);
    Quiet.makeQuiet();
    ਵਾਪਸ Quiet;
  }

  ਜੇ (Exp == APFloat::IEK_Inf)
    ਵਾਪਸ Val;

  // 1 is added because frexp is defined to return a normalized fraction in
  // +/-[0.5, 1.0), rather than the usual +/-[1.0, 2.0).
  Exp = Exp == APFloat::IEK_Zero ? 0 : Exp + 1;
  ਵਾਪਸ scalbn(Val, -Exp, RM);
}

DoubleAPFloat::DoubleAPFloat(ਅਚਲ fltSemantics &S)
    : Semantics(&S),
      Floats(ਨਵਾਂ APFloat[2]{APFloat(APFloat_ਅਧਾਰ::semIEEEdouble),
                            APFloat(APFloat_ਅਧਾਰ::semIEEEdouble)}) {
  ਪ੍ਰਤਿਜ੍ਞਾ(Semantics == &APFloat_ਅਧਾਰ::semPPCDoubleDouble);
}

DoubleAPFloat::DoubleAPFloat(ਅਚਲ fltSemantics &S, uninitialized_ਟੈਗ)
    : Semantics(&S), Floats(ਨਵਾਂ APFloat[2]{
                         APFloat(APFloat_ਅਧਾਰ::semIEEEdouble, uninitialized),
                         APFloat(APFloat_ਅਧਾਰ::semIEEEdouble, uninitialized)}) {
  ਪ੍ਰਤਿਜ੍ਞਾ(Semantics == &APFloat_ਅਧਾਰ::semPPCDoubleDouble);
}

DoubleAPFloat::DoubleAPFloat(ਅਚਲ fltSemantics &S, integerPart I)
    : Semantics(&S),
      Floats(ਨਵਾਂ APFloat[2]{APFloat(APFloat_ਅਧਾਰ::semIEEEdouble, I),
                            APFloat(APFloat_ਅਧਾਰ::semIEEEdouble)}) {
  ਪ੍ਰਤਿਜ੍ਞਾ(Semantics == &APFloat_ਅਧਾਰ::semPPCDoubleDouble);
}

DoubleAPFloat::DoubleAPFloat(ਅਚਲ fltSemantics &S, ਅਚਲ APInt &I)
    : Semantics(&S),
      Floats(ਨਵਾਂ APFloat[2]{
          APFloat(APFloat_ਅਧਾਰ::semIEEEdouble, APInt(64, I.get_Raw_ਅੰਕੜੇ()[0])),
          APFloat(APFloat_ਅਧਾਰ::semIEEEdouble, APInt(64, I.get_Raw_ਅੰਕੜੇ()[1]))}) {
  ਪ੍ਰਤਿਜ੍ਞਾ(Semantics == &APFloat_ਅਧਾਰ::semPPCDoubleDouble);
}

DoubleAPFloat::DoubleAPFloat(ਅਚਲ fltSemantics &S, APFloat &&ਪਹਿਲਾ,
                             APFloat &&ਦੂਜਾ)
    : Semantics(&S),
      Floats(ਨਵਾਂ APFloat[2]{std::move(ਪਹਿਲਾ), std::move(ਦੂਜਾ)}) {
  ਪ੍ਰਤਿਜ੍ਞਾ(Semantics == &APFloat_ਅਧਾਰ::semPPCDoubleDouble);
  ਪ੍ਰਤਿਜ੍ਞਾ(&Floats[0].getSemantics() == &APFloat_ਅਧਾਰ::semIEEEdouble);
  ਪ੍ਰਤਿਜ੍ਞਾ(&Floats[1].getSemantics() == &APFloat_ਅਧਾਰ::semIEEEdouble);
}

DoubleAPFloat::DoubleAPFloat(ਅਚਲ DoubleAPFloat &RHS)
    : Semantics(RHS.Semantics),
      Floats(RHS.Floats ? ਨਵਾਂ APFloat[2]{APFloat(RHS.Floats[0]),
                                         APFloat(RHS.Floats[1])}
                        : ਖਾਲੀ_ਸੂਚਕ) {
  ਪ੍ਰਤਿਜ੍ਞਾ(Semantics == &APFloat_ਅਧਾਰ::semPPCDoubleDouble);
}

DoubleAPFloat::DoubleAPFloat(DoubleAPFloat &&RHS)
    : Semantics(RHS.Semantics), Floats(RHS.Floats) {
  RHS.Semantics = &APFloat_ਅਧਾਰ::semBogus;
  RHS.Floats = ਖਾਲੀ_ਸੂਚਕ;
  ਪ੍ਰਤਿਜ੍ਞਾ(Semantics == &APFloat_ਅਧਾਰ::semPPCDoubleDouble);
}

DoubleAPFloat &DoubleAPFloat::ਸੰਚਾਲਕ=(ਅਚਲ DoubleAPFloat &RHS) {
  ਜੇ (Semantics == RHS.Semantics && RHS.Floats) {
    Floats[0] = RHS.Floats[0];
    Floats[1] = RHS.Floats[1];
  } ਨਹੀਂਤਾਂ ਜੇ (ਇਹ != &RHS) {
    ਇਹ->~DoubleAPFloat();
    ਨਵਾਂ (ਇਹ) DoubleAPFloat(RHS);
  }
  ਵਾਪਸ *ਇਹ;
}

// Returns a result such that:
// 1. abs(Lo) <= ulp(Hi)/2
// 2. Hi == RTNE(Hi + Lo)
// 3. Hi + Lo == X + Y
//
// Requires that log2(X) >= log2(Y).
ਸਥਿਰ std::pair<APFloat, APFloat> fast_Two_ਜੋੜ(APFloat X, APFloat Y) {
  ਜੇ (!X.isFinite())
    ਵਾਪਸ {X, APFloat::getZero(X.getSemantics(), /*Negative=*/ਝੂਠ)};
  APFloat Hi = X + Y;
  APFloat Delta = Hi - X;
  APFloat Lo = Y - Delta;
  ਵਾਪਸ {Hi, Lo};
}

// Implement addition, subtraction, multiplication and division based on:
// "Software for Doubled-Precision Floating-Point Computations",
// by Seppo Linnainmaa, ACM TOMS vol 7 no 3, September 1981, pages 272-283.
APFloat::op_ਸਥਿਤੀ DoubleAPFloat::ਜੋੜੋ_Impl(ਅਚਲ APFloat &a, ਅਚਲ APFloat &aa,
                                         ਅਚਲ APFloat &c, ਅਚਲ APFloat &cc,
                                         rounding_ਬਹੁਲਕ RM) {
  ਪੂਰਨ ਸਥਿਤੀ = opOK;
  APFloat z = a;
  ਸਥਿਤੀ |= z.ਜੋੜੋ(c, RM);
  ਜੇ (!z.isFinite()) {
    ਜੇ (!z.isInfinity()) {
      Floats[0] = std::move(z);
      Floats[1].makeZero(/* Neg = */ ਝੂਠ);
      ਵਾਪਸ (op_ਸਥਿਤੀ)ਸਥਿਤੀ;
    }
    ਸਥਿਤੀ = opOK;
    ਸਵੈਚਾਲਿਤ ACompared_ਮੰਜ਼ਿਲ_C = a.compare_Absolute_ਮੁੱਲ(c);
    z = cc;
    ਸਥਿਤੀ |= z.ਜੋੜੋ(aa, RM);
    ਜੇ (ACompared_ਮੰਜ਼ਿਲ_C == APFloat::cmpGreaterThan) {
      // z = cc + aa + c + a;
      ਸਥਿਤੀ |= z.ਜੋੜੋ(c, RM);
      ਸਥਿਤੀ |= z.ਜੋੜੋ(a, RM);
    } ਨਹੀਂਤਾਂ {
      // z = cc + aa + a + c;
      ਸਥਿਤੀ |= z.ਜੋੜੋ(a, RM);
      ਸਥਿਤੀ |= z.ਜੋੜੋ(c, RM);
    }
    ਜੇ (!z.isFinite()) {
      Floats[0] = std::move(z);
      Floats[1].makeZero(/* Neg = */ ਝੂਠ);
      ਵਾਪਸ (op_ਸਥਿਤੀ)ਸਥਿਤੀ;
    }
    Floats[0] = z;
    APFloat zz = aa;
    ਸਥਿਤੀ |= zz.ਜੋੜੋ(cc, RM);
    ਜੇ (ACompared_ਮੰਜ਼ਿਲ_C == APFloat::cmpGreaterThan) {
      // Floats[1] = a - z + c + zz;
      Floats[1] = a;
      ਸਥਿਤੀ |= Floats[1].ਘਟਾਓ(z, RM);
      ਸਥਿਤੀ |= Floats[1].ਜੋੜੋ(c, RM);
      ਸਥਿਤੀ |= Floats[1].ਜੋੜੋ(zz, RM);
    } ਨਹੀਂਤਾਂ {
      // Floats[1] = c - z + a + zz;
      Floats[1] = c;
      ਸਥਿਤੀ |= Floats[1].ਘਟਾਓ(z, RM);
      ਸਥਿਤੀ |= Floats[1].ਜੋੜੋ(a, RM);
      ਸਥਿਤੀ |= Floats[1].ਜੋੜੋ(zz, RM);
    }
  } ਨਹੀਂਤਾਂ {
    // q = a - z;
    APFloat q = a;
    ਸਥਿਤੀ |= q.ਘਟਾਓ(z, RM);

    // zz = q + c + (a - (q + z)) + aa + cc;
    // Compute a - (q + z) as -((q + z) - a) to avoid temporary copies.
    ਸਵੈਚਾਲਿਤ zz = q;
    ਸਥਿਤੀ |= zz.ਜੋੜੋ(c, RM);
    ਸਥਿਤੀ |= q.ਜੋੜੋ(z, RM);
    ਸਥਿਤੀ |= q.ਘਟਾਓ(a, RM);
    q.changeSign();
    ਸਥਿਤੀ |= zz.ਜੋੜੋ(q, RM);
    ਸਥਿਤੀ |= zz.ਜੋੜੋ(aa, RM);
    ਸਥਿਤੀ |= zz.ਜੋੜੋ(cc, RM);
    ਜੇ (zz.isZero() && !zz.isNegative()) {
      Floats[0] = std::move(z);
      Floats[1].makeZero(/* Neg = */ ਝੂਠ);
      ਵਾਪਸ opOK;
    }
    Floats[0] = z;
    ਸਥਿਤੀ |= Floats[0].ਜੋੜੋ(zz, RM);
    ਜੇ (!Floats[0].isFinite()) {
      Floats[1].makeZero(/* Neg = */ ਝੂਠ);
      ਵਾਪਸ (op_ਸਥਿਤੀ)ਸਥਿਤੀ;
    }
    Floats[1] = std::move(z);
    ਸਥਿਤੀ |= Floats[1].ਘਟਾਓ(Floats[0], RM);
    ਸਥਿਤੀ |= Floats[1].ਜੋੜੋ(zz, RM);
  }
  ਵਾਪਸ (op_ਸਥਿਤੀ)ਸਥਿਤੀ;
}

APFloat::op_ਸਥਿਤੀ DoubleAPFloat::ਜੋੜੋ_With_Special(ਅਚਲ DoubleAPFloat &LHS,
                                                ਅਚਲ DoubleAPFloat &RHS,
                                                DoubleAPFloat &ਬਾਹਰਲਾ,
                                                rounding_ਬਹੁਲਕ RM) {
  ਜੇ (LHS.getCategory() == fcNaN) {
    ਬਾਹਰਲਾ = LHS;
    ਵਾਪਸ opOK;
  }
  ਜੇ (RHS.getCategory() == fcNaN) {
    ਬਾਹਰਲਾ = RHS;
    ਵਾਪਸ opOK;
  }
  ਜੇ (LHS.getCategory() == fcZero) {
    ਬਾਹਰਲਾ = RHS;
    ਵਾਪਸ opOK;
  }
  ਜੇ (RHS.getCategory() == fcZero) {
    ਬਾਹਰਲਾ = LHS;
    ਵਾਪਸ opOK;
  }
  ਜੇ (LHS.getCategory() == fcInfinity && RHS.getCategory() == fcInfinity &&
      LHS.isNegative() != RHS.isNegative()) {
    ਬਾਹਰਲਾ.makeNaN(ਝੂਠ, ਬਾਹਰਲਾ.isNegative(), ਖਾਲੀ_ਸੂਚਕ);
    ਵਾਪਸ opInvalidOp;
  }
  ਜੇ (LHS.getCategory() == fcInfinity) {
    ਬਾਹਰਲਾ = LHS;
    ਵਾਪਸ opOK;
  }
  ਜੇ (RHS.getCategory() == fcInfinity) {
    ਬਾਹਰਲਾ = RHS;
    ਵਾਪਸ opOK;
  }
  ਪ੍ਰਤਿਜ੍ਞਾ(LHS.getCategory() == fc_ਸਾਧਾਰਨ && RHS.getCategory() == fc_ਸਾਧਾਰਨ);

  APFloat A(LHS.Floats[0]), AA(LHS.Floats[1]), C(RHS.Floats[0]),
      CC(RHS.Floats[1]);
  ਪ੍ਰਤਿਜ੍ਞਾ(&A.getSemantics() == &APFloat_ਅਧਾਰ::semIEEEdouble);
  ਪ੍ਰਤਿਜ੍ਞਾ(&AA.getSemantics() == &APFloat_ਅਧਾਰ::semIEEEdouble);
  ਪ੍ਰਤਿਜ੍ਞਾ(&C.getSemantics() == &APFloat_ਅਧਾਰ::semIEEEdouble);
  ਪ੍ਰਤਿਜ੍ਞਾ(&CC.getSemantics() == &APFloat_ਅਧਾਰ::semIEEEdouble);
  ਪ੍ਰਤਿਜ੍ਞਾ(&ਬਾਹਰਲਾ.Floats[0].getSemantics() == &APFloat_ਅਧਾਰ::semIEEEdouble);
  ਪ੍ਰਤਿਜ੍ਞਾ(&ਬਾਹਰਲਾ.Floats[1].getSemantics() == &APFloat_ਅਧਾਰ::semIEEEdouble);
  ਵਾਪਸ ਬਾਹਰਲਾ.ਜੋੜੋ_Impl(A, AA, C, CC, RM);
}

APFloat::op_ਸਥਿਤੀ DoubleAPFloat::ਜੋੜੋ(ਅਚਲ DoubleAPFloat &RHS,
                                     rounding_ਬਹੁਲਕ RM) {
  ਵਾਪਸ ਜੋੜੋ_With_Special(*ਇਹ, RHS, *ਇਹ, RM);
}

APFloat::op_ਸਥਿਤੀ DoubleAPFloat::ਘਟਾਓ(ਅਚਲ DoubleAPFloat &RHS,
                                          rounding_ਬਹੁਲਕ RM) {
  changeSign();
  ਸਵੈਚਾਲਿਤ Ret = ਜੋੜੋ(RHS, RM);
  changeSign();
  ਵਾਪਸ Ret;
}

APFloat::op_ਸਥਿਤੀ DoubleAPFloat::multiply(ਅਚਲ DoubleAPFloat &RHS,
                                          APFloat::rounding_ਬਹੁਲਕ RM) {
  ਅਚਲ ਸਵੈਚਾਲਿਤ &LHS = *ਇਹ;
  ਸਵੈਚਾਲਿਤ &ਬਾਹਰਲਾ = *ਇਹ;
  /* Interesting observation: For special categories, finding the lowest
     common ancestor of the following layered graph gives the correct
     return category:

        NaN
       /   \
     Zero  Inf
       \   /
ਸਾਧਾਰਨ

     e.g. NaN * NaN = NaN
          Zero * Inf = NaN
          Normal * Zero = Zero
          Normal * Inf = Inf
  */
  ਜੇ (LHS.getCategory() == fcNaN) {
    ਬਾਹਰਲਾ = LHS;
    ਵਾਪਸ opOK;
  }
  ਜੇ (RHS.getCategory() == fcNaN) {
    ਬਾਹਰਲਾ = RHS;
    ਵਾਪਸ opOK;
  }
  ਜੇ ((LHS.getCategory() == fcZero && RHS.getCategory() == fcInfinity) ||
      (LHS.getCategory() == fcInfinity && RHS.getCategory() == fcZero)) {
    ਬਾਹਰਲਾ.makeNaN(ਝੂਠ, ਝੂਠ, ਖਾਲੀ_ਸੂਚਕ);
    ਵਾਪਸ opOK;
  }
  ਜੇ (LHS.getCategory() == fcZero || LHS.getCategory() == fcInfinity) {
    ਬਾਹਰਲਾ = LHS;
    ਵਾਪਸ opOK;
  }
  ਜੇ (RHS.getCategory() == fcZero || RHS.getCategory() == fcInfinity) {
    ਬਾਹਰਲਾ = RHS;
    ਵਾਪਸ opOK;
  }
  ਪ੍ਰਤਿਜ੍ਞਾ(LHS.getCategory() == fc_ਸਾਧਾਰਨ && RHS.getCategory() == fc_ਸਾਧਾਰਨ &&
         "Special cases not handled exhaustively");

  ਪੂਰਨ ਸਥਿਤੀ = opOK;
  APFloat A = Floats[0], B = Floats[1], C = RHS.Floats[0], D = RHS.Floats[1];
  // t = a * c
  APFloat T = A;
  ਸਥਿਤੀ |= T.multiply(C, RM);
  ਜੇ (!T.isFiniteNonZero()) {
    Floats[0] = std::move(T);
    Floats[1].makeZero(/* Neg = */ ਝੂਠ);
    ਵਾਪਸ (op_ਸਥਿਤੀ)ਸਥਿਤੀ;
  }

  // tau = fmsub(a, c, t), that is -fmadd(-a, c, t).
  APFloat Tau = A;
  T.changeSign();
  ਸਥਿਤੀ |= Tau.fused_Multiply_ਜੋੜੋ(C, T, RM);
  T.changeSign();
  {
    // v = a * d
    APFloat V = A;
    ਸਥਿਤੀ |= V.multiply(D, RM);
    // w = b * c
    APFloat W = B;
    ਸਥਿਤੀ |= W.multiply(C, RM);
    ਸਥਿਤੀ |= V.ਜੋੜੋ(W, RM);
    // tau += v + w
    ਸਥਿਤੀ |= Tau.ਜੋੜੋ(V, RM);
  }
  // u = t + tau
  APFloat U = T;
  ਸਥਿਤੀ |= U.ਜੋੜੋ(Tau, RM);

  Floats[0] = U;
  ਜੇ (!U.isFinite()) {
    Floats[1].makeZero(/* Neg = */ ਝੂਠ);
  } ਨਹੀਂਤਾਂ {
    // Floats[1] = (t - u) + tau
    ਸਥਿਤੀ |= T.ਘਟਾਓ(U, RM);
    ਸਥਿਤੀ |= T.ਜੋੜੋ(Tau, RM);
    Floats[1] = std::move(T);
  }
  ਵਾਪਸ (op_ਸਥਿਤੀ)ਸਥਿਤੀ;
}

APFloat::op_ਸਥਿਤੀ DoubleAPFloat::divide(ਅਚਲ DoubleAPFloat &RHS,
                                        APFloat::rounding_ਬਹੁਲਕ RM) {
  ਪ੍ਰਤਿਜ੍ਞਾ(Semantics == &APFloat_ਅਧਾਰ::semPPCDoubleDouble &&
         "Unexpected Semantics");
  APFloat Tmp(APFloat_ਅਧਾਰ::semPPCDoubleDoubleLegacy, bitcast_ਮੰਜ਼ਿਲ_APInt());
  ਸਵੈਚਾਲਿਤ Ret = Tmp.divide(
      APFloat(APFloat_ਅਧਾਰ::semPPCDoubleDoubleLegacy, RHS.bitcast_ਮੰਜ਼ਿਲ_APInt()), RM);
  *ਇਹ = DoubleAPFloat(APFloat_ਅਧਾਰ::semPPCDoubleDouble, Tmp.bitcast_ਮੰਜ਼ਿਲ_APInt());
  ਵਾਪਸ Ret;
}

APFloat::op_ਸਥਿਤੀ DoubleAPFloat::remainder(ਅਚਲ DoubleAPFloat &RHS) {
  ਪ੍ਰਤਿਜ੍ਞਾ(Semantics == &APFloat_ਅਧਾਰ::semPPCDoubleDouble &&
         "Unexpected Semantics");
  APFloat Tmp(APFloat_ਅਧਾਰ::semPPCDoubleDoubleLegacy, bitcast_ਮੰਜ਼ਿਲ_APInt());
  ਸਵੈਚਾਲਿਤ Ret = Tmp.remainder(
      APFloat(APFloat_ਅਧਾਰ::semPPCDoubleDoubleLegacy, RHS.bitcast_ਮੰਜ਼ਿਲ_APInt()));
  *ਇਹ = DoubleAPFloat(APFloat_ਅਧਾਰ::semPPCDoubleDouble, Tmp.bitcast_ਮੰਜ਼ਿਲ_APInt());
  ਵਾਪਸ Ret;
}

APFloat::op_ਸਥਿਤੀ DoubleAPFloat::mod(ਅਚਲ DoubleAPFloat &RHS) {
  ਪ੍ਰਤਿਜ੍ਞਾ(Semantics == &APFloat_ਅਧਾਰ::semPPCDoubleDouble &&
         "Unexpected Semantics");
  APFloat Tmp(APFloat_ਅਧਾਰ::semPPCDoubleDoubleLegacy, bitcast_ਮੰਜ਼ਿਲ_APInt());
  ਸਵੈਚਾਲਿਤ Ret = Tmp.mod(
      APFloat(APFloat_ਅਧਾਰ::semPPCDoubleDoubleLegacy, RHS.bitcast_ਮੰਜ਼ਿਲ_APInt()));
  *ਇਹ = DoubleAPFloat(APFloat_ਅਧਾਰ::semPPCDoubleDouble, Tmp.bitcast_ਮੰਜ਼ਿਲ_APInt());
  ਵਾਪਸ Ret;
}

APFloat::op_ਸਥਿਤੀ
DoubleAPFloat::fused_Multiply_ਜੋੜੋ(ਅਚਲ DoubleAPFloat &Multiplicand,
                                ਅਚਲ DoubleAPFloat &Addend,
                                APFloat::rounding_ਬਹੁਲਕ RM) {
  ਪ੍ਰਤਿਜ੍ਞਾ(Semantics == &APFloat_ਅਧਾਰ::semPPCDoubleDouble &&
         "Unexpected Semantics");
  APFloat Tmp(APFloat_ਅਧਾਰ::semPPCDoubleDoubleLegacy, bitcast_ਮੰਜ਼ਿਲ_APInt());
  ਸਵੈਚਾਲਿਤ Ret = Tmp.fused_Multiply_ਜੋੜੋ(
      APFloat(APFloat_ਅਧਾਰ::semPPCDoubleDoubleLegacy,
              Multiplicand.bitcast_ਮੰਜ਼ਿਲ_APInt()),
      APFloat(APFloat_ਅਧਾਰ::semPPCDoubleDoubleLegacy, Addend.bitcast_ਮੰਜ਼ਿਲ_APInt()),
      RM);
  *ਇਹ = DoubleAPFloat(APFloat_ਅਧਾਰ::semPPCDoubleDouble, Tmp.bitcast_ਮੰਜ਼ਿਲ_APInt());
  ਵਾਪਸ Ret;
}

APFloat::op_ਸਥਿਤੀ DoubleAPFloat::ਗੇੜਾ_ਮੰਜ਼ਿਲ_Integral(APFloat::rounding_ਬਹੁਲਕ RM) {
  ਪ੍ਰਤਿਜ੍ਞਾ(Semantics == &APFloat_ਅਧਾਰ::semPPCDoubleDouble &&
         "Unexpected Semantics");
  ਅਚਲ APFloat &Hi = get_ਪਹਿਲਾ();
  ਅਚਲ APFloat &Lo = get_ਦੂਜਾ();

  APFloat RoundedHi = Hi;
  ਅਚਲ op_ਸਥਿਤੀ Hi_ਸਥਿਤੀ = RoundedHi.ਗੇੜਾ_ਮੰਜ਼ਿਲ_Integral(RM);

  // We can reduce the problem to just the high part if the input:
  // 1. Represents a non-finite value.
  // 2. Has a component which is zero.
  ਜੇ (!Hi.isFiniteNonZero() || Lo.isZero()) {
    Floats[0] = std::move(RoundedHi);
    Floats[1].makeZero(/*Neg=*/ਝੂਠ);
    ਵਾਪਸ Hi_ਸਥਿਤੀ;
  }

  // Adjust `Rounded` in the direction of `TieBreaker` if `ToRound` was at a
  // halfway point.
  ਸਵੈਚਾਲਿਤ ਗੇੜਾ_ਮੰਜ਼ਿਲ_ਸਭ_ਤੋਂ_ਨੇੜੇ_Helper = [](APFloat ਮੰਜ਼ਿਲ_ਗੇੜਾ, APFloat Rounded,
                                 APFloat TieBreaker) {
    // RoundingError tells us which direction we rounded:
    //   - RoundingError > 0: we rounded up.
    //   - RoundingError < 0: we rounded down.
    // Sterbenz' lemma ensures that RoundingError is exact.
    ਅਚਲ APFloat RoundingError = Rounded - ਮੰਜ਼ਿਲ_ਗੇੜਾ;
    ਜੇ (TieBreaker.isNonZero() &&
        TieBreaker.isNegative() != RoundingError.isNegative() &&
        ਨਿਰਪੇਖ(RoundingError).is_Exactly_ਮੁੱਲ(0.5))
      Rounded.ਜੋੜੋ(
          APFloat::getOne(Rounded.getSemantics(), TieBreaker.isNegative()),
          rm_ਸਭ_ਤੋਂ_ਨੇੜੇ_Ties_ਮੰਜ਼ਿਲ_Even);
    ਵਾਪਸ Rounded;
  };

  // Case 1: Hi is not an integer.
  // Special cases are for rounding modes that are sensitive to ties.
  ਜੇ (RoundedHi != Hi) {
    // We need to consider the case where Hi was between two integers and the
    // rounding mode broke the tie when, in fact, Lo may have had a different
    // sign than Hi.
    ਜੇ (RM == rm_ਸਭ_ਤੋਂ_ਨੇੜੇ_Ties_ਮੰਜ਼ਿਲ_Away || RM == rm_ਸਭ_ਤੋਂ_ਨੇੜੇ_Ties_ਮੰਜ਼ਿਲ_Even)
      RoundedHi = ਗੇੜਾ_ਮੰਜ਼ਿਲ_ਸਭ_ਤੋਂ_ਨੇੜੇ_Helper(Hi, RoundedHi, Lo);

    Floats[0] = std::move(RoundedHi);
    Floats[1].makeZero(/*Neg=*/ਝੂਠ);
    ਵਾਪਸ Hi_ਸਥਿਤੀ;
  }

  // Case 2: Hi is an integer.
  // Special cases are for rounding modes which are rounding towards or away from zero.
  Rounding_ਬਹੁਲਕ Lo_Rounding_ਬਹੁਲਕ;
  ਜੇ (RM == rmTowardZero)
    // When our input is positive, we want the Lo component rounded toward
    // negative infinity to get the smallest result magnitude. Likewise,
    // negative inputs want the Lo component rounded toward positive infinity.
    Lo_Rounding_ਬਹੁਲਕ = isNegative() ? rm_Toward_ਸਕਾਰਾਤਮਕ : rmTowardNegative;
  ਨਹੀਂਤਾਂ
    Lo_Rounding_ਬਹੁਲਕ = RM;

  APFloat RoundedLo = Lo;
  ਅਚਲ op_ਸਥਿਤੀ Lo_ਸਥਿਤੀ = RoundedLo.ਗੇੜਾ_ਮੰਜ਼ਿਲ_Integral(Lo_Rounding_ਬਹੁਲਕ);
  ਜੇ (Lo_Rounding_ਬਹੁਲਕ == rm_ਸਭ_ਤੋਂ_ਨੇੜੇ_Ties_ਮੰਜ਼ਿਲ_Away)
    // We need to consider the case where Lo was between two integers and the
    // rounding mode broke the tie when, in fact, Hi may have had a different
    // sign than Lo.
    RoundedLo = ਗੇੜਾ_ਮੰਜ਼ਿਲ_ਸਭ_ਤੋਂ_ਨੇੜੇ_Helper(Lo, RoundedLo, Hi);

  // We must ensure that the final result has no overlap between the two APFloat values.
  std::tie(RoundedHi, RoundedLo) = fast_Two_ਜੋੜ(RoundedHi, RoundedLo);

  Floats[0] = std::move(RoundedHi);
  Floats[1] = std::move(RoundedLo);
  ਵਾਪਸ Lo_ਸਥਿਤੀ;
}

ਖਾਲੀ DoubleAPFloat::changeSign() {
  Floats[0].changeSign();
  Floats[1].changeSign();
}

APFloat::cmp_ਨਤੀਜਾ
DoubleAPFloat::compare_Absolute_ਮੁੱਲ(ਅਚਲ DoubleAPFloat &RHS) ਅਚਲ {
  // Compare absolute values of the high parts.
  ਅਚਲ cmp_ਨਤੀਜਾ HiPartCmp = Floats[0].compare_Absolute_ਮੁੱਲ(RHS.Floats[0]);
  ਜੇ (HiPartCmp != cmpEqual)
    ਵਾਪਸ HiPartCmp;

  // Zero, regardless of sign, is equal.
  ਜੇ (Floats[1].isZero() && RHS.Floats[1].isZero())
    ਵਾਪਸ cmpEqual;

  // At this point, |this->Hi| == |RHS.Hi|.
  // The magnitude is |Hi+Lo| which is Hi+|Lo| if signs of Hi and Lo are the
  // same, and Hi-|Lo| if signs are different.
  ਅਚਲ ਤਾਰਕਿਕ ThisIsSubtractive =
      Floats[0].isNegative() != Floats[1].isNegative();
  ਅਚਲ ਤਾਰਕਿਕ RHSIsSubtractive =
      RHS.Floats[0].isNegative() != RHS.Floats[1].isNegative();

  // Case 1: The low part of 'this' is zero.
  ਜੇ (Floats[1].isZero())
    // We are comparing |Hi| vs. |Hi| ± |RHS.Lo|.
    // If RHS is subtractive, its magnitude is smaller.
    // If RHS is additive, its magnitude is larger.
    ਵਾਪਸ RHSIsSubtractive ? cmpGreaterThan : cmpLessThan;

  // Case 2: The low part of 'RHS' is zero (and we know 'this' is not).
  ਜੇ (RHS.Floats[1].isZero())
    // We are comparing |Hi| ± |This.Lo| vs. |Hi|.
    // If 'this' is subtractive, its magnitude is smaller.
    // If 'this' is additive, its magnitude is larger.
    ਵਾਪਸ ThisIsSubtractive ? cmpLessThan : cmpGreaterThan;

  // If their natures differ, the additive one is larger.
  ਜੇ (ThisIsSubtractive != RHSIsSubtractive)
    ਵਾਪਸ ThisIsSubtractive ? cmpLessThan : cmpGreaterThan;

  // Case 3: Both are additive (Hi+|Lo|) or both are subtractive (Hi-|Lo|).
  // The comparison now depends on the magnitude of the low parts.
  ਅਚਲ cmp_ਨਤੀਜਾ LoPartCmp = Floats[1].compare_Absolute_ਮੁੱਲ(RHS.Floats[1]);

  ਜੇ (ThisIsSubtractive) {
    // Both are subtractive (Hi-|Lo|), so the comparison of |Lo| is inverted.
    ਜੇ (LoPartCmp == cmpLessThan)
      ਵਾਪਸ cmpGreaterThan;
    ਜੇ (LoPartCmp == cmpGreaterThan)
      ਵਾਪਸ cmpLessThan;
  }

  // If additive, the comparison of |Lo| is direct.
  // If equal, they are equal.
  ਵਾਪਸ LoPartCmp;
}

APFloat::fltCategory DoubleAPFloat::getCategory() ਅਚਲ {
  ਵਾਪਸ Floats[0].getCategory();
}

ਤਾਰਕਿਕ DoubleAPFloat::isNegative() ਅਚਲ { ਵਾਪਸ Floats[0].isNegative(); }

ਖਾਲੀ DoubleAPFloat::makeInf(ਤਾਰਕਿਕ Neg) {
  Floats[0].makeInf(Neg);
  Floats[1].makeZero(/* Neg = */ ਝੂਠ);
}

ਖਾਲੀ DoubleAPFloat::makeZero(ਤਾਰਕਿਕ Neg) {
  Floats[0].makeZero(Neg);
  Floats[1].makeZero(/* Neg = */ ਝੂਠ);
}

ਖਾਲੀ DoubleAPFloat::make_ਸਭ_ਤੋਂ_ਵੱਡਾ(ਤਾਰਕਿਕ Neg) {
  ਪ੍ਰਤਿਜ੍ਞਾ(Semantics == &APFloat_ਅਧਾਰ::semPPCDoubleDouble &&
         "Unexpected Semantics");
  Floats[0] =
      APFloat(APFloat_ਅਧਾਰ::semIEEEdouble, APInt(64, 0x7fefffffffffffffull));
  Floats[1] =
      APFloat(APFloat_ਅਧਾਰ::semIEEEdouble, APInt(64, 0x7c8ffffffffffffeull));
  ਜੇ (Neg)
    changeSign();
}

ਖਾਲੀ DoubleAPFloat::make_ਸਭ_ਤੋਂ_ਛੋਟਾ(ਤਾਰਕਿਕ Neg) {
  ਪ੍ਰਤਿਜ੍ਞਾ(Semantics == &APFloat_ਅਧਾਰ::semPPCDoubleDouble &&
         "Unexpected Semantics");
  Floats[0].make_ਸਭ_ਤੋਂ_ਛੋਟਾ(Neg);
  Floats[1].makeZero(/* Neg = */ ਝੂਠ);
}

ਖਾਲੀ DoubleAPFloat::make_ਸਭ_ਤੋਂ_ਛੋਟਾ_Normalized(ਤਾਰਕਿਕ Neg) {
  ਪ੍ਰਤਿਜ੍ਞਾ(Semantics == &APFloat_ਅਧਾਰ::semPPCDoubleDouble &&
         "Unexpected Semantics");
  Floats[0] =
      APFloat(APFloat_ਅਧਾਰ::semIEEEdouble, APInt(64, 0x0360000000000000ull));
  ਜੇ (Neg)
    Floats[0].changeSign();
  Floats[1].makeZero(/* Neg = */ ਝੂਠ);
}

ਖਾਲੀ DoubleAPFloat::makeNaN(ਤਾਰਕਿਕ SNaN, ਤਾਰਕਿਕ Neg, ਅਚਲ APInt *fill) {
  Floats[0].makeNaN(SNaN, Neg, fill);
  Floats[1].makeZero(/* Neg = */ ਝੂਠ);
}

APFloat::cmp_ਨਤੀਜਾ DoubleAPFloat::compare(ਅਚਲ DoubleAPFloat &RHS) ਅਚਲ {
  ਸਵੈਚਾਲਿਤ ਨਤੀਜਾ = Floats[0].compare(RHS.Floats[0]);
  // |Float[0]| > |Float[1]|
  ਜੇ (ਨਤੀਜਾ == APFloat::cmpEqual)
    ਵਾਪਸ Floats[1].compare(RHS.Floats[1]);
  ਵਾਪਸ ਨਤੀਜਾ;
}

ਤਾਰਕਿਕ DoubleAPFloat::bitwiseIsEqual(ਅਚਲ DoubleAPFloat &RHS) ਅਚਲ {
  ਵਾਪਸ Floats[0].bitwiseIsEqual(RHS.Floats[0]) &&
         Floats[1].bitwiseIsEqual(RHS.Floats[1]);
}

hash_ਕੋਡ hash_ਮੁੱਲ(ਅਚਲ DoubleAPFloat &Arg) {
  ਜੇ (Arg.Floats)
    ਵਾਪਸ hash_combine(hash_ਮੁੱਲ(Arg.Floats[0]), hash_ਮੁੱਲ(Arg.Floats[1]));
  ਵਾਪਸ hash_combine(Arg.Semantics);
}

APInt DoubleAPFloat::bitcast_ਮੰਜ਼ਿਲ_APInt() ਅਚਲ {
  ਪ੍ਰਤਿਜ੍ਞਾ(Semantics == &APFloat_ਅਧਾਰ::semPPCDoubleDouble &&
         "Unexpected Semantics");
  uint64_t ਅੰਕੜੇ_2[] = {
      Floats[0].bitcast_ਮੰਜ਼ਿਲ_APInt().get_Raw_ਅੰਕੜੇ()[0],
      Floats[1].bitcast_ਮੰਜ਼ਿਲ_APInt().get_Raw_ਅੰਕੜੇ()[0],
  };
  ਵਾਪਸ APInt(128, ਅੰਕੜੇ_2);
}

Expected<APFloat::op_ਸਥਿਤੀ> DoubleAPFloat::convertFromString(StringRef S,
                                                             rounding_ਬਹੁਲਕ RM) {
  ਪ੍ਰਤਿਜ੍ਞਾ(Semantics == &APFloat_ਅਧਾਰ::semPPCDoubleDouble &&
         "Unexpected Semantics");
  APFloat Tmp(APFloat_ਅਧਾਰ::semPPCDoubleDoubleLegacy);
  ਸਵੈਚਾਲਿਤ Ret = Tmp.convertFromString(S, RM);
  *ਇਹ = DoubleAPFloat(APFloat_ਅਧਾਰ::semPPCDoubleDouble, Tmp.bitcast_ਮੰਜ਼ਿਲ_APInt());
  ਵਾਪਸ Ret;
}

// The double-double lattice of values corresponds to numbers which obey:
// - abs(lo) <= 1/2 * ulp(hi)
// - roundTiesToEven(hi + lo) == hi
//
// nextUp must choose the smallest output > input that follows these rules.
// nexDown must choose the largest output < input that follows these rules.
APFloat::op_ਸਥਿਤੀ DoubleAPFloat::ਅਗਲਾ(ਤਾਰਕਿਕ ਅਗਲਾ_ਥੱਲੇ) {
  ਪ੍ਰਤਿਜ੍ਞਾ(Semantics == &APFloat_ਅਧਾਰ::semPPCDoubleDouble &&
         "Unexpected Semantics");
  // nextDown(x) = -nextUp(-x)
  ਜੇ (ਅਗਲਾ_ਥੱਲੇ) {
    changeSign();
    APFloat::op_ਸਥਿਤੀ ਨਤੀਜਾ = ਅਗਲਾ(/*nextDown=*/ਝੂਠ);
    changeSign();
    ਵਾਪਸ ਨਤੀਜਾ;
  }
  ਚੋਣ (getCategory()) {
  ਹਾਲਤ fcInfinity:
    // nextUp(+inf) = +inf
    // nextUp(-inf) = -getLargest()
    ਜੇ (isNegative())
      make_ਸਭ_ਤੋਂ_ਵੱਡਾ(ਸੱਚ);
    ਵਾਪਸ opOK;

  ਹਾਲਤ fcNaN:
    // IEEE-754R 2008 6.2 Par 2: nextUp(sNaN) = qNaN. Set Invalid flag.
    // IEEE-754R 2008 6.2: nextUp(qNaN) = qNaN. Must be identity so we do not
    //                     change the payload.
    ਜੇ (get_ਪਹਿਲਾ().isSignaling()) {
      // For consistency, propagate the sign of the sNaN to the qNaN.
      makeNaN(ਝੂਠ, isNegative(), ਖਾਲੀ_ਸੂਚਕ);
      ਵਾਪਸ opInvalidOp;
    }
    ਵਾਪਸ opOK;

  ਹਾਲਤ fcZero:
    // nextUp(pm 0) = +getSmallest()
    make_ਸਭ_ਤੋਂ_ਛੋਟਾ(ਝੂਠ);
    ਵਾਪਸ opOK;

  ਹਾਲਤ fc_ਸਾਧਾਰਨ:
    ਰੋਕੋ;
  }

  ਅਚਲ APFloat &HiOld = get_ਪਹਿਲਾ();
  ਅਚਲ APFloat &LoOld = get_ਦੂਜਾ();

  APFloat ਅਗਲਾ_Lo = LoOld;
  ਅਗਲਾ_Lo.ਅਗਲਾ(/*nextDown=*/ਝੂਠ);

  // We want to admit values where:
  // 1. abs(Lo) <= ulp(Hi)/2
  // 2. Hi == RTNE(Hi + lo)
  ਸਵੈਚਾਲਿਤ ਵਿੱਚ_Lattice = [](ਅਚਲ APFloat &Hi, ਅਚਲ APFloat &Lo) {
    ਵਾਪਸ Hi + Lo == Hi;
  };

  // Check if (HiOld, nextUp(LoOld) is in the lattice.
  ਜੇ (ਵਿੱਚ_Lattice(HiOld, ਅਗਲਾ_Lo)) {
    // Yes, the result is (HiOld, nextUp(LoOld)).
    Floats[1] = std::move(ਅਗਲਾ_Lo);

    // TODO: Because we currently rely on semPPCDoubleDoubleLegacy, our maximum
    // value is defined to have exactly 106 bits of precision. This limitation
    // results in semPPCDoubleDouble being unable to reach its maximum canonical
    // value.
    DoubleAPFloat ਸਭ_ਤੋਂ_ਵੱਡਾ{*Semantics, uninitialized};
    ਸਭ_ਤੋਂ_ਵੱਡਾ.make_ਸਭ_ਤੋਂ_ਵੱਡਾ(/*Neg=*/ਝੂਠ);
    ਜੇ (compare(ਸਭ_ਤੋਂ_ਵੱਡਾ) == cmpGreaterThan)
      makeInf(/*Neg=*/ਝੂਠ);

    ਵਾਪਸ opOK;
  }

  // Now we need to handle the cases where (HiOld, nextUp(LoOld)) is not the
  // correct result. We know the new hi component will be nextUp(HiOld) but our
  // lattice rules make it a little ambiguous what the correct NextLo must be.
  APFloat ਅਗਲਾ_Hi = HiOld;
  ਅਗਲਾ_Hi.ਅਗਲਾ(/*nextDown=*/ਝੂਠ);

  // nextUp(getLargest()) == INFINITY
  ਜੇ (ਅਗਲਾ_Hi.isInfinity()) {
    makeInf(/*Neg=*/ਝੂਠ);
    ਵਾਪਸ opOK;
  }

  // IEEE 754-2019 5.3.1:
  // "If x is the negative number of least magnitude in x's format, nextUp(x) is
  // -0."
  ਜੇ (ਅਗਲਾ_Hi.isZero()) {
    makeZero(/*Neg=*/ਸੱਚ);
    ਵਾਪਸ opOK;
  }

  // abs(NextLo) must be <= ulp(NextHi)/2. We want NextLo to be as close to
  // negative infinity as possible.
  ਅਗਲਾ_Lo = neg(scalbn(harrisonUlp(ਅਗਲਾ_Hi), -1, rmTowardZero));
  ਜੇ (!ਵਿੱਚ_Lattice(ਅਗਲਾ_Hi, ਅਗਲਾ_Lo))
    // RTNE may mean that Lo must be < ulp(NextHi) / 2 so we bump NextLo.
    ਅਗਲਾ_Lo.ਅਗਲਾ(/*nextDown=*/ਝੂਠ);

  Floats[0] = std::move(ਅਗਲਾ_Hi);
  Floats[1] = std::move(ਅਗਲਾ_Lo);

  ਵਾਪਸ opOK;
}

APFloat::op_ਸਥਿਤੀ DoubleAPFloat::convert_ਮੰਜ਼ਿਲ_Sign_Extended_Integer(
    MutableArrayRef<integerPart> ਨਿਵੇਸ਼, ਬਿਨਾਨਿਸ਼ਾਨੀ ਪੂਰਨ ਚੌੜਾਈ, ਤਾਰਕਿਕ IsSigned,
    rounding_ਬਹੁਲਕ RM, ਤਾਰਕਿਕ *IsExact) ਅਚਲ {
  ਪ੍ਰਤਿਜ੍ਞਾ(Semantics == &APFloat_ਅਧਾਰ::semPPCDoubleDouble &&
         "Unexpected Semantics");

  // If Hi is not finite, or Lo is zero, the value is entirely represented
  // by Hi. Delegate to the simpler single-APFloat conversion.
  ਜੇ (!get_ਪਹਿਲਾ().isFiniteNonZero() || get_ਦੂਜਾ().isZero())
    ਵਾਪਸ get_ਪਹਿਲਾ().convert_ਮੰਜ਼ਿਲ_Integer(ਨਿਵੇਸ਼, ਚੌੜਾਈ, IsSigned, RM, IsExact);

  // First, round the full double-double value to an integral value. This
  // simplifies the rest of the function, as we no longer need to consider
  // fractional parts.
  *IsExact = ਝੂਠ;
  DoubleAPFloat Integral = *ਇਹ;
  ਅਚਲ op_ਸਥਿਤੀ ਗੇੜਾ_ਸਥਿਤੀ = Integral.ਗੇੜਾ_ਮੰਜ਼ਿਲ_Integral(RM);
  ਜੇ (ਗੇੜਾ_ਸਥਿਤੀ == opInvalidOp)
    ਵਾਪਸ opInvalidOp;
  ਅਚਲ APFloat &IntegralHi = Integral.get_ਪਹਿਲਾ();
  ਅਚਲ APFloat &IntegralLo = Integral.get_ਦੂਜਾ();

  // If rounding results in either component being zero, the sum is trivial.
  // Delegate to the simpler single-APFloat conversion.
  ਤਾਰਕਿਕ HiIsExact;
  ਜੇ (IntegralHi.isZero() || IntegralLo.isZero()) {
    ਅਚਲ op_ਸਥਿਤੀ Hi_ਸਥਿਤੀ =
        IntegralHi.convert_ਮੰਜ਼ਿਲ_Integer(ਨਿਵੇਸ਼, ਚੌੜਾਈ, IsSigned, RM, &HiIsExact);
    // The conversion from an integer-valued float to an APInt may fail if the
    // result would be out of range.  Regardless, taking this path is only
    // possible if rounding occurred during the initial `roundToIntegral`.
    ਵਾਪਸ Hi_ਸਥਿਤੀ == opOK ? opInexact : Hi_ਸਥਿਤੀ;
  }

  // A negative number cannot be represented by an unsigned integer.
  // Since a double-double is canonical, if Hi is negative, the sum is negative.
  ਜੇ (!IsSigned && IntegralHi.isNegative())
    ਵਾਪਸ opInvalidOp;

  // Handle the special boundary case where |Hi| is exactly the power of two
  // that marks the edge of the integer's range (e.g., 2^63 for int64_t). In
  // this situation, Hi itself won't fit, but the sum Hi + Lo might.
  // `PositiveOverflowWidth` is the bit number for this boundary (N-1 for
  // signed, N for unsigned).
  ਤਾਰਕਿਕ LoIsExact;
  ਅਚਲ ਪੂਰਨ HiExactLog2 = IntegralHi.getExactLog2Abs();
  ਅਚਲ ਬਿਨਾਨਿਸ਼ਾਨੀ ਸਕਾਰਾਤਮਕ_ਉਛਾਲ_ਚੌੜਾਈ = IsSigned ? ਚੌੜਾਈ - 1 : ਚੌੜਾਈ;
  ਜੇ (HiExactLog2 >= 0 &&
      static_cast<ਬਿਨਾਨਿਸ਼ਾਨੀ>(HiExactLog2) == ਸਕਾਰਾਤਮਕ_ਉਛਾਲ_ਚੌੜਾਈ) {
    // If Hi and Lo have the same sign, |Hi + Lo| > |Hi|, so the sum is
    // guaranteed to overflow. E.g., for uint128_t, (2^128, 1) overflows.
    ਜੇ (IntegralHi.isNegative() == IntegralLo.isNegative())
      ਵਾਪਸ opInvalidOp;

    // If the signs differ, the sum will fit. We can compute the result using
    // properties of two's complement arithmetic without a wide intermediate
    // integer. E.g., for uint128_t, (2^128, -1) should be 2^128 - 1.
    ਅਚਲ op_ਸਥਿਤੀ Lo_ਸਥਿਤੀ = IntegralLo.convert_ਮੰਜ਼ਿਲ_Integer(
        ਨਿਵੇਸ਼, ਚੌੜਾਈ, /*IsSigned=*/ਸੱਚ, RM, &LoIsExact);
    ਜੇ (Lo_ਸਥਿਤੀ == opInvalidOp)
      ਵਾਪਸ opInvalidOp;

    // Adjust the bit pattern of Lo to account for Hi's value:
    //  - For unsigned (Hi=2^Width): `2^Width + Lo` in `Width`-bit
    //    arithmetic is equivalent to just `Lo`. The conversion of `Lo` above
    //    already produced the correct final bit pattern.
    //  - For signed (Hi=2^(Width-1)): The sum `2^(Width-1) + Lo` (where Lo<0)
    //    can be computed by taking the two's complement pattern for `Lo` and
    //    clearing the sign bit.
    ਜੇ (IsSigned && !IntegralHi.isNegative())
      APInt::tc_Clear_ਦੋਆਧਾਰੀ_ਅੰਕ(ਨਿਵੇਸ਼.ਅੰਕੜੇ(), ਸਕਾਰਾਤਮਕ_ਉਛਾਲ_ਚੌੜਾਈ);
    *IsExact = ਗੇੜਾ_ਸਥਿਤੀ == opOK;
    ਵਾਪਸ ਗੇੜਾ_ਸਥਿਤੀ;
  }

  // Convert Hi into an integer.  This may not fit but that is OK: we know that
  // Hi + Lo would not fit either in this situation.
  ਅਚਲ op_ਸਥਿਤੀ Hi_ਸਥਿਤੀ = IntegralHi.convert_ਮੰਜ਼ਿਲ_Integer(
      ਨਿਵੇਸ਼, ਚੌੜਾਈ, IsSigned, rmTowardZero, &HiIsExact);
  ਜੇ (Hi_ਸਥਿਤੀ == opInvalidOp)
    ਵਾਪਸ Hi_ਸਥਿਤੀ;

  // Convert Lo into a temporary integer of the same width.
  APSInt Lo_ਨਤੀਜਾ{ਚੌੜਾਈ, /*isUnsigned=*/!IsSigned};
  ਅਚਲ op_ਸਥਿਤੀ Lo_ਸਥਿਤੀ =
      IntegralLo.convert_ਮੰਜ਼ਿਲ_Integer(Lo_ਨਤੀਜਾ, rmTowardZero, &LoIsExact);
  ਜੇ (Lo_ਸਥਿਤੀ == opInvalidOp)
    ਵਾਪਸ Lo_ਸਥਿਤੀ;

  // Add Lo to Hi. This addition is guaranteed not to overflow because of the
  // double-double canonicalization rule (`|Lo| <= ulp(Hi)/2`). The only case
  // where the sum could cross the integer type's boundary is when Hi is a
  // power of two, which is handled by the special case block above.
  APInt::tc_ਜੋੜੋ(ਨਿਵੇਸ਼.ਅੰਕੜੇ(), Lo_ਨਤੀਜਾ.get_Raw_ਅੰਕੜੇ(), /*carry=*/0, ਨਿਵੇਸ਼.ਮਾਪ_2());

  *IsExact = ਗੇੜਾ_ਸਥਿਤੀ == opOK;
  ਵਾਪਸ ਗੇੜਾ_ਸਥਿਤੀ;
}

APFloat::op_ਸਥਿਤੀ
DoubleAPFloat::convert_ਮੰਜ਼ਿਲ_Integer(MutableArrayRef<integerPart> ਨਿਵੇਸ਼,
                                ਬਿਨਾਨਿਸ਼ਾਨੀ ਪੂਰਨ ਚੌੜਾਈ, ਤਾਰਕਿਕ IsSigned,
                                rounding_ਬਹੁਲਕ RM, ਤਾਰਕਿਕ *IsExact) ਅਚਲ {
  op_ਸਥਿਤੀ FS =
      convert_ਮੰਜ਼ਿਲ_Sign_Extended_Integer(ਨਿਵੇਸ਼, ਚੌੜਾਈ, IsSigned, RM, IsExact);

  ਜੇ (FS == opInvalidOp) {
    ਅਚਲ ਬਿਨਾਨਿਸ਼ਾਨੀ Dst_Parts_ਗਿਣਤੀ = part_ਗਿਣਤੀ_For_ਬਿੱਟ(ਚੌੜਾਈ);
    ਪ੍ਰਤਿਜ੍ਞਾ(Dst_Parts_ਗਿਣਤੀ <= ਨਿਵੇਸ਼.ਮਾਪ_2() && "Integer too big");

    ਬਿਨਾਨਿਸ਼ਾਨੀ ਬਿੱਟ_2;
    ਜੇ (getCategory() == fcNaN)
      ਬਿੱਟ_2 = 0;
    ਨਹੀਂਤਾਂ ਜੇ (isNegative())
      ਬਿੱਟ_2 = IsSigned;
    ਨਹੀਂਤਾਂ
      ਬਿੱਟ_2 = ਚੌੜਾਈ - IsSigned;

    tc_ਸੈੱਟ_ਘੱਟ_ਤੋਂ_ਘੱਟ_Significant_ਬਿੱਟ(ਨਿਵੇਸ਼.ਅੰਕੜੇ(), Dst_Parts_ਗਿਣਤੀ, ਬਿੱਟ_2);
    ਜੇ (isNegative() && IsSigned)
      APInt::tc_ਖਿਸਕਾਓ_ਖੱਬਾ(ਨਿਵੇਸ਼.ਅੰਕੜੇ(), Dst_Parts_ਗਿਣਤੀ, ਚੌੜਾਈ - 1);
  }

  ਵਾਪਸ FS;
}

APFloat::op_ਸਥਿਤੀ DoubleAPFloat::handle_ਉਛਾਲ(rounding_ਬਹੁਲਕ RM) {
  ਚੋਣ (RM) {
  ਹਾਲਤ APFloat::rmTowardZero:
    make_ਸਭ_ਤੋਂ_ਵੱਡਾ(/*Neg=*/isNegative());
    ਰੋਕੋ;
  ਹਾਲਤ APFloat::rmTowardNegative:
    ਜੇ (isNegative())
      makeInf(/*Neg=*/ਸੱਚ);
    ਨਹੀਂਤਾਂ
      make_ਸਭ_ਤੋਂ_ਵੱਡਾ(/*Neg=*/ਝੂਠ);
    ਰੋਕੋ;
  ਹਾਲਤ APFloat::rm_Toward_ਸਕਾਰਾਤਮਕ:
    ਜੇ (isNegative())
      make_ਸਭ_ਤੋਂ_ਵੱਡਾ(/*Neg=*/ਸੱਚ);
    ਨਹੀਂਤਾਂ
      makeInf(/*Neg=*/ਝੂਠ);
    ਰੋਕੋ;
  ਹਾਲਤ APFloat::rm_ਸਭ_ਤੋਂ_ਨੇੜੇ_Ties_ਮੰਜ਼ਿਲ_Away:
  ਹਾਲਤ APFloat::rm_ਸਭ_ਤੋਂ_ਨੇੜੇ_Ties_ਮੰਜ਼ਿਲ_Even:
    makeInf(/*Neg=*/isNegative());
    ਰੋਕੋ;
  ਮੂਲ:
    llvm_unreachable("Invalid rounding mode found");
  }
  op_ਸਥਿਤੀ S = opInexact;
  ਜੇ (!get_ਪਹਿਲਾ().isFinite())
    S = static_cast<op_ਸਥਿਤੀ>(S | op_ਉਛਾਲ);
  ਵਾਪਸ S;
}

APFloat::op_ਸਥਿਤੀ DoubleAPFloat::convertFromUnsignedParts(
    ਅਚਲ integerPart *Src, ਬਿਨਾਨਿਸ਼ਾਨੀ ਪੂਰਨ Src_ਗਿਣਤੀ, rounding_ਬਹੁਲਕ RM) {
  // Find the most significant bit of the source integer. APInt::tcMSB returns
  // UINT_MAX for a zero value.
  ਅਚਲ ਬਿਨਾਨਿਸ਼ਾਨੀ SrcMSB = APInt::tcMSB(Src, Src_ਗਿਣਤੀ);
  ਜੇ (SrcMSB == UINT_MAX) {
    // The source integer is 0.
    makeZero(/*Neg=*/ਝੂਠ);
    ਵਾਪਸ opOK;
  }

  // Create a minimally-sized APInt to represent the source value.
  ਅਚਲ ਬਿਨਾਨਿਸ਼ਾਨੀ Src_ਦੋਆਧਾਰੀ_ਅੰਕ_ਚੌੜਾਈ = SrcMSB + 1;
  APSInt SrcInt{APInt{/*numBits=*/Src_ਦੋਆਧਾਰੀ_ਅੰਕ_ਚੌੜਾਈ, ArrayRef(Src, Src_ਗਿਣਤੀ)},
                /*isUnsigned=*/ਸੱਚ};

  // Stage 1: Initial Approximation.
  // Convert the source integer SrcInt to the Hi part of the DoubleAPFloat.
  // We use round-to-nearest because it minimizes the initial error, which is
  // crucial for the subsequent steps.
  APFloat Hi{get_ਪਹਿਲਾ().getSemantics()};
  Hi.convertFromAPInt(SrcInt, /*IsSigned=*/ਝੂਠ, rm_ਸਭ_ਤੋਂ_ਨੇੜੇ_Ties_ਮੰਜ਼ਿਲ_Even);

  // If the first approximation already overflows, the number is too large.
  // NOTE: The underlying semantics are *more* conservative when choosing to
  // overflow because their notion of ULP is much larger. As such, it is always
  // safe to overflow at the DoubleAPFloat level if the APFloat overflows.
  ਜੇ (!Hi.isFinite())
    ਵਾਪਸ handle_ਉਛਾਲ(RM);

  // Stage 2: Exact Error Calculation.
  // Calculate the exact error of the first approximation: Error = SrcInt - Hi.
  // This is done by converting Hi back to an integer and subtracting it from
  // the original source.
  ਤਾਰਕਿਕ HiAsIntIsExact;
  // Create an integer representation of Hi. Its width is determined by the
  // exponent of Hi, ensuring it's just large enough. This width can exceed
  // SrcBitWidth if the conversion to Hi rounded up to a power of two.
  // accurately when converted back to an integer.
  APSInt HiAsInt{static_cast<uint32_t>(ilogb(Hi) + 1), /*isUnsigned=*/ਸੱਚ};
  Hi.convert_ਮੰਜ਼ਿਲ_Integer(HiAsInt, rm_ਸਭ_ਤੋਂ_ਨੇੜੇ_Ties_ਮੰਜ਼ਿਲ_Even, &HiAsIntIsExact);
  ਅਚਲ APInt Error = SrcInt.zext(HiAsInt.get_ਦੋਆਧਾਰੀ_ਅੰਕ_ਚੌੜਾਈ()) - HiAsInt;

  // Stage 3: Error Approximation and Rounding.
  // Convert the integer error into the Lo part of the DoubleAPFloat. This step
  // captures the remainder of the original number. The rounding mode for this
  // conversion (LoRM) may need to be adjusted from the user-requested RM to
  // ensure the final sum (Hi + Lo) rounds correctly.
  rounding_ਬਹੁਲਕ LoRM = RM;
  // Adjustments are only necessary when the initial approximation Hi was an
  // overestimate, making the Error negative.
  ਜੇ (Error.isNegative()) {
    ਜੇ (RM == rm_ਸਭ_ਤੋਂ_ਨੇੜੇ_Ties_ਮੰਜ਼ਿਲ_Away) {
      // For rmNearestTiesToAway, a tie should round away from zero. Since
      // SrcInt is positive, this means rounding toward +infinity.
      // A standard conversion of a negative Error would round ties toward
      // -infinity, causing the final sum Hi + Lo to be smaller. To
      // counteract this, we detect the tie case and override the rounding
      // mode for Lo to rmTowardPositive.
      ਅਚਲ ਬਿਨਾਨਿਸ਼ਾਨੀ Error_ਸਰਗਰਮ_ਬਿੱਟ = Error.get_Significant_ਬਿੱਟ() - 1;
      ਅਚਲ ਬਿਨਾਨਿਸ਼ਾਨੀ LoPrecision = get_ਦੂਜਾ().getSemantics().precision;
      ਜੇ (Error_ਸਰਗਰਮ_ਬਿੱਟ > LoPrecision) {
        ਅਚਲ ਬਿਨਾਨਿਸ਼ਾਨੀ RoundingBoundary = Error_ਸਰਗਰਮ_ਬਿੱਟ - LoPrecision;
        // A tie occurs when the bits to be truncated are of the form 100...0.
        // This is detected by checking if the number of trailing zeros is
        // exactly one less than the number of bits being truncated.
        ਜੇ (Error.ਗਿਣਤੀ_Trailing_Zeros() == RoundingBoundary - 1)
          LoRM = rm_Toward_ਸਕਾਰਾਤਮਕ;
      }
    } ਨਹੀਂਤਾਂ ਜੇ (RM == rmTowardZero) {
      // For rmTowardZero, the final positive result must be truncated (rounded
      // down). When Hi is an overestimate, Error is negative. A standard
      // rmTowardZero conversion of Error would make it *less* negative,
      // effectively rounding the final sum Hi + Lo *up*. To ensure the sum
      // rounds down correctly, we force Lo to round toward -infinity.
      LoRM = rmTowardNegative;
    }
  }

  APFloat Lo{get_ਦੂਜਾ().getSemantics()};
  op_ਸਥਿਤੀ ਸਥਿਤੀ = Lo.convertFromAPInt(Error, /*IsSigned=*/ਸੱਚ, LoRM);

  // Renormalize the pair (Hi, Lo) into a canonical DoubleAPFloat form where the
  // components do not overlap. fastTwoSum performs this operation.
  std::tie(Hi, Lo) = fast_Two_ਜੋੜ(Hi, Lo);
  Floats[0] = std::move(Hi);
  Floats[1] = std::move(Lo);

  // A final check for overflow is needed because fastTwoSum can cause a
  // carry-out from Lo that pushes Hi to infinity.
  ਜੇ (!get_ਪਹਿਲਾ().isFinite())
    ਵਾਪਸ handle_ਉਛਾਲ(RM);

  // The largest DoubleAPFloat must be canonical. Values which are larger are
  // not canonical and are equivalent to overflow.
  ਜੇ (get_ਪਹਿਲਾ().isFiniteNonZero() && Floats[0].is_ਸਭ_ਤੋਂ_ਵੱਡਾ()) {
    DoubleAPFloat ਸਭ_ਤੋਂ_ਵੱਡਾ{*Semantics};
    ਸਭ_ਤੋਂ_ਵੱਡਾ.make_ਸਭ_ਤੋਂ_ਵੱਡਾ(/*Neg=*/ਝੂਠ);
    ਜੇ (compare(ਸਭ_ਤੋਂ_ਵੱਡਾ) == APFloat::cmpGreaterThan)
      ਵਾਪਸ handle_ਉਛਾਲ(RM);
  }

  // The final status of the operation is determined by the conversion of the
  // error term. If Lo could represent Error exactly, the entire conversion
  // is exact. Otherwise, it's inexact.
  ਵਾਪਸ ਸਥਿਤੀ;
}

APFloat::op_ਸਥਿਤੀ DoubleAPFloat::convertFromAPInt(ਅਚਲ APInt &ਨਿਵੇਸ਼,
                                                  ਤਾਰਕਿਕ IsSigned,
                                                  rounding_ਬਹੁਲਕ RM) {
  ਅਚਲ ਤਾਰਕਿਕ Negate_ਨਿਵੇਸ਼ = IsSigned && ਨਿਵੇਸ਼.isNegative();
  APInt API = ਨਿਵੇਸ਼;
  ਜੇ (Negate_ਨਿਵੇਸ਼)
    API.negate();

  ਅਚਲ APFloat::op_ਸਥਿਤੀ ਸਥਿਤੀ =
      convertFromUnsignedParts(API.get_Raw_ਅੰਕੜੇ(), API.get_ਅੰਕ_ਸ਼ਬਦ(), RM);
  ਜੇ (Negate_ਨਿਵੇਸ਼)
    changeSign();
  ਵਾਪਸ ਸਥਿਤੀ;
}

ਬਿਨਾਨਿਸ਼ਾਨੀ ਪੂਰਨ DoubleAPFloat::convert_ਮੰਜ਼ਿਲ_Hex_String(ਅੱਖਰ *DST,
                                               ਬਿਨਾਨਿਸ਼ਾਨੀ ਪੂਰਨ HexDigits,
                                               ਤਾਰਕਿਕ ਉੱਤਲਾ_Case_2,
                                               rounding_ਬਹੁਲਕ RM) ਅਚਲ {
  ਪ੍ਰਤਿਜ੍ਞਾ(Semantics == &APFloat_ਅਧਾਰ::semPPCDoubleDouble &&
         "Unexpected Semantics");
  ਵਾਪਸ APFloat(APFloat_ਅਧਾਰ::semPPCDoubleDoubleLegacy, bitcast_ਮੰਜ਼ਿਲ_APInt())
      .convert_ਮੰਜ਼ਿਲ_Hex_String(DST, HexDigits, ਉੱਤਲਾ_Case_2, RM);
}

ਤਾਰਕਿਕ DoubleAPFloat::isDenormal() ਅਚਲ {
  ਵਾਪਸ getCategory() == fc_ਸਾਧਾਰਨ &&
         (Floats[0].isDenormal() || Floats[1].isDenormal() ||
          // (double)(Hi + Lo) == Hi defines a normal number.
          Floats[0] != Floats[0] + Floats[1]);
}

ਤਾਰਕਿਕ DoubleAPFloat::is_ਸਭ_ਤੋਂ_ਛੋਟਾ() ਅਚਲ {
  ਜੇ (getCategory() != fc_ਸਾਧਾਰਨ)
    ਵਾਪਸ ਝੂਠ;
  DoubleAPFloat Tmp(*ਇਹ);
  Tmp.make_ਸਭ_ਤੋਂ_ਛੋਟਾ(ਇਹ->isNegative());
  ਵਾਪਸ Tmp.compare(*ਇਹ) == cmpEqual;
}

ਤਾਰਕਿਕ DoubleAPFloat::is_ਸਭ_ਤੋਂ_ਛੋਟਾ_Normalized() ਅਚਲ {
  ਜੇ (getCategory() != fc_ਸਾਧਾਰਨ)
    ਵਾਪਸ ਝੂਠ;

  DoubleAPFloat Tmp(*ਇਹ);
  Tmp.make_ਸਭ_ਤੋਂ_ਛੋਟਾ_Normalized(ਇਹ->isNegative());
  ਵਾਪਸ Tmp.compare(*ਇਹ) == cmpEqual;
}

ਤਾਰਕਿਕ DoubleAPFloat::is_ਸਭ_ਤੋਂ_ਵੱਡਾ() ਅਚਲ {
  ਜੇ (getCategory() != fc_ਸਾਧਾਰਨ)
    ਵਾਪਸ ਝੂਠ;
  DoubleAPFloat Tmp(*ਇਹ);
  Tmp.make_ਸਭ_ਤੋਂ_ਵੱਡਾ(ਇਹ->isNegative());
  ਵਾਪਸ Tmp.compare(*ਇਹ) == cmpEqual;
}

ਤਾਰਕਿਕ DoubleAPFloat::isInteger() ਅਚਲ {
  ਪ੍ਰਤਿਜ੍ਞਾ(Semantics == &APFloat_ਅਧਾਰ::semPPCDoubleDouble &&
         "Unexpected Semantics");
  ਵਾਪਸ Floats[0].isInteger() && Floats[1].isInteger();
}

ਖਾਲੀ DoubleAPFloat::ਮੰਜ਼ਿਲ_String(Small_ਸਦਿਸ਼_Impl<ਅੱਖਰ> &Str,
                             ਬਿਨਾਨਿਸ਼ਾਨੀ FormatPrecision,
                             ਬਿਨਾਨਿਸ਼ਾਨੀ FormatMaxPadding,
                             ਤਾਰਕਿਕ TruncateZero) ਅਚਲ {
  ਪ੍ਰਤਿਜ੍ਞਾ(Semantics == &APFloat_ਅਧਾਰ::semPPCDoubleDouble &&
         "Unexpected Semantics");
  APFloat(APFloat_ਅਧਾਰ::semPPCDoubleDoubleLegacy, bitcast_ਮੰਜ਼ਿਲ_APInt())
      .ਮੰਜ਼ਿਲ_String(Str, FormatPrecision, FormatMaxPadding, TruncateZero);
}

ਪੂਰਨ DoubleAPFloat::getExactLog2Abs() ਅਚਲ {
  // In order for Hi + Lo to be a power of two, the following must be true:
  // 1. Hi must be a power of two.
  // 2. Lo must be zero.
  ਜੇ (get_ਦੂਜਾ().isNonZero())
    ਵਾਪਸ INT_MIN;
  ਵਾਪਸ get_ਪਹਿਲਾ().getExactLog2Abs();
}

ਪੂਰਨ ilogb(ਅਚਲ DoubleAPFloat &Arg) {
  ਅਚਲ APFloat &Hi = Arg.get_ਪਹਿਲਾ();
  ਅਚਲ APFloat &Lo = Arg.get_ਦੂਜਾ();
  ਪੂਰਨ Ilogb_ਨਤੀਜਾ = ilogb(Hi);
  // Zero and non-finite values can delegate to ilogb(Hi).
  ਜੇ (Arg.getCategory() != fc_ਸਾਧਾਰਨ)
    ਵਾਪਸ Ilogb_ਨਤੀਜਾ;
  // If Lo can't change the binade, we can delegate to ilogb(Hi).
  ਜੇ (Lo.isZero() || Hi.isNegative() == Lo.isNegative())
    ਵਾਪਸ Ilogb_ਨਤੀਜਾ;
  ਜੇ (Hi.getExactLog2Abs() == INT_MIN)
    ਵਾਪਸ Ilogb_ਨਤੀਜਾ;
  // Numbers of the form 2^a - 2^b or -2^a + 2^b are almost powers of two but
  // get nudged out of the binade by the low component.
  ਵਾਪਸ Ilogb_ਨਤੀਜਾ - 1;
}

DoubleAPFloat scalbn(ਅਚਲ DoubleAPFloat &Arg, ਪੂਰਨ Exp,
                     APFloat::rounding_ਬਹੁਲਕ RM) {
  ਪ੍ਰਤਿਜ੍ਞਾ(Arg.Semantics == &APFloat_ਅਧਾਰ::PPCDoubleDouble() &&
         "Unexpected Semantics");
  ਵਾਪਸ DoubleAPFloat(APFloat_ਅਧਾਰ::PPCDoubleDouble(),
                       scalbn(Arg.Floats[0], Exp, RM),
                       scalbn(Arg.Floats[1], Exp, RM));
}

DoubleAPFloat frexp(ਅਚਲ DoubleAPFloat &Arg, ਪੂਰਨ &Exp,
                    APFloat::rounding_ਬਹੁਲਕ RM) {
  ਪ੍ਰਤਿਜ੍ਞਾ(Arg.Semantics == &APFloat_ਅਧਾਰ::PPCDoubleDouble() &&
         "Unexpected Semantics");

  // Get the unbiased exponent e of the number, where |Arg| = m * 2^e for m in
  // [1.0, 2.0).
  Exp = ilogb(Arg);

  // For NaNs, quiet any signaling NaN and return the result, as per standard
  // practice.
  ਜੇ (Exp == APFloat::IEK_NaN) {
    DoubleAPFloat Quiet{Arg};
    Quiet.get_ਪਹਿਲਾ() = Quiet.get_ਪਹਿਲਾ().makeQuiet();
    ਵਾਪਸ Quiet;
  }

  // For infinity, return it unchanged. The exponent remains IEK_Inf.
  ਜੇ (Exp == APFloat::IEK_Inf)
    ਵਾਪਸ Arg;

  // For zero, the fraction is zero and the standard requires the exponent be 0.
  ਜੇ (Exp == APFloat::IEK_Zero) {
    Exp = 0;
    ਵਾਪਸ Arg;
  }

  ਅਚਲ APFloat &Hi = Arg.get_ਪਹਿਲਾ();
  ਅਚਲ APFloat &Lo = Arg.get_ਦੂਜਾ();

  // frexp requires the fraction's absolute value to be in [0.5, 1.0).
  // ilogb provides an exponent for an absolute value in [1.0, 2.0).
  // Increment the exponent to ensure the fraction is in the correct range.
  ++Exp;

  ਅਚਲ ਤਾਰਕਿਕ SignsDisagree = Hi.isNegative() != Lo.isNegative();
  APFloat ਦੂਜਾ = Lo;
  ਜੇ (Arg.getCategory() == APFloat::fc_ਸਾਧਾਰਨ && Lo.isFiniteNonZero()) {
    rounding_ਬਹੁਲਕ Lo_Rounding_ਬਹੁਲਕ;
    // The interpretation of rmTowardZero depends on the sign of the combined
    // Arg rather than the sign of the component.
    ਜੇ (RM == rmTowardZero)
      Lo_Rounding_ਬਹੁਲਕ = Arg.isNegative() ? rm_Toward_ਸਕਾਰਾਤਮਕ : rmTowardNegative;
    // For rmNearestTiesToAway, we face a similar problem. If signs disagree,
    // Lo is a correction *toward* zero relative to Hi. Rounding Lo
    // "away from zero" based on its own sign would move the value in the
    // wrong direction. As a safe proxy, we use rmNearestTiesToEven, which is
    // direction-agnostic. We only need to bother with this if Lo is scaled
    // down.
    ਨਹੀਂਤਾਂ ਜੇ (RM == rm_ਸਭ_ਤੋਂ_ਨੇੜੇ_Ties_ਮੰਜ਼ਿਲ_Away && SignsDisagree && Exp > 0)
      Lo_Rounding_ਬਹੁਲਕ = rm_ਸਭ_ਤੋਂ_ਨੇੜੇ_Ties_ਮੰਜ਼ਿਲ_Even;
    ਨਹੀਂਤਾਂ
      Lo_Rounding_ਬਹੁਲਕ = RM;
    ਦੂਜਾ = scalbn(Lo, -Exp, Lo_Rounding_ਬਹੁਲਕ);
    // The rmNearestTiesToEven proxy is correct most of the time, but it
    // differs from rmNearestTiesToAway when the scaled value of Lo is an
    // exact midpoint.
    // NOTE: This is morally equivalent to roundTiesTowardZero.
    ਜੇ (RM == rm_ਸਭ_ਤੋਂ_ਨੇੜੇ_Ties_ਮੰਜ਼ਿਲ_Away && Lo_Rounding_ਬਹੁਲਕ == rm_ਸਭ_ਤੋਂ_ਨੇੜੇ_Ties_ਮੰਜ਼ਿਲ_Even) {
      // Re-scale the result back to check if rounding occurred.
      ਅਚਲ APFloat RecomposedLo = scalbn(ਦੂਜਾ, Exp, rm_ਸਭ_ਤੋਂ_ਨੇੜੇ_Ties_ਮੰਜ਼ਿਲ_Even);
      ਜੇ (RecomposedLo != Lo) {
        // RoundingError tells us which direction we rounded:
        //   - RoundingError > 0: we rounded up.
        //   - RoundingError < 0: we down up.
        ਅਚਲ APFloat RoundingError = RecomposedLo - Lo;
        // Determine if scalbn(Lo, -Exp) landed exactly on a midpoint.
        // We do this by checking if the absolute rounding error is exactly
        // half a ULP of the result.
        ਅਚਲ APFloat Ulp_ਦਾ_ਦੂਜਾ = harrisonUlp(ਦੂਜਾ);
        ਅਚਲ APFloat Scaled_Ulp_ਦਾ_ਦੂਜਾ =
            scalbn(Ulp_ਦਾ_ਦੂਜਾ, Exp - 1, rm_ਸਭ_ਤੋਂ_ਨੇੜੇ_Ties_ਮੰਜ਼ਿਲ_Even);
        ਅਚਲ ਤਾਰਕਿਕ IsMidpoint = ਨਿਰਪੇਖ(RoundingError) == Scaled_Ulp_ਦਾ_ਦੂਜਾ;
        ਅਚਲ ਤਾਰਕਿਕ RoundedLoAway =
            ਦੂਜਾ.isNegative() == RoundingError.isNegative();
        // The sign of Hi and Lo disagree and we rounded Lo away: we must
        // decrease the magnitude of Second to increase the magnitude
        // First+Second.
        ਜੇ (IsMidpoint && RoundedLoAway)
          ਦੂਜਾ.ਅਗਲਾ(/*nextDown=*/!ਦੂਜਾ.isNegative());
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
    ਜੇ (ਦੂਜਾ.isZero() && SignsDisagree && Hi.getExactLog2Abs() != INT_MIN)
      ++Exp;
  }

  APFloat ਪਹਿਲਾ = scalbn(Hi, -Exp, RM);
  ਵਾਪਸ DoubleAPFloat(APFloat_ਅਧਾਰ::PPCDoubleDouble(), std::move(ਪਹਿਲਾ),
                       std::move(ਦੂਜਾ));
}

APInt DoubleAPFloat::getNaNPayload() ਅਚਲ { ਵਾਪਸ Floats[0].getNaNPayload(); }
} // namespace detail

APFloat::Storage::Storage(IEEEFloat F, ਅਚਲ fltSemantics &Semantics) {
  ਜੇ (ਵਰਤਦਾ_Layout<IEEEFloat>(Semantics)) {
    ਨਵਾਂ (&IEEE) IEEEFloat(std::move(F));
    ਵਾਪਸ;
  }
  ਜੇ (ਵਰਤਦਾ_Layout<DoubleAPFloat>(Semantics)) {
    ਅਚਲ fltSemantics& S = F.getSemantics();
    ਨਵਾਂ (&Double) DoubleAPFloat(Semantics, APFloat(std::move(F), S),
                                APFloat(APFloat_ਅਧਾਰ::IEEEdouble()));
    ਵਾਪਸ;
  }
  llvm_unreachable("Unexpected semantics");
}

Expected<APFloat::op_ਸਥਿਤੀ> APFloat::convertFromString(StringRef Str,
                                                       rounding_ਬਹੁਲਕ RM) {
  APFLOAT_DISPATCH_ON_SEMANTICS(convertFromString(Str, RM));
}

hash_ਕੋਡ hash_ਮੁੱਲ(ਅਚਲ APFloat &Arg) {
  ਜੇ (APFloat::ਵਰਤਦਾ_Layout<detail::IEEEFloat>(Arg.getSemantics()))
    ਵਾਪਸ hash_ਮੁੱਲ(Arg.U.IEEE);
  ਜੇ (APFloat::ਵਰਤਦਾ_Layout<detail::DoubleAPFloat>(Arg.getSemantics()))
    ਵਾਪਸ hash_ਮੁੱਲ(Arg.U.Double);
  llvm_unreachable("Unexpected semantics");
}

APFloat::APFloat(ਅਚਲ fltSemantics &Semantics, StringRef S)
    : APFloat(Semantics) {
  ਸਵੈਚਾਲਿਤ ਸਥਿਤੀ_Or_Err = convertFromString(S, rm_ਸਭ_ਤੋਂ_ਨੇੜੇ_Ties_ਮੰਜ਼ਿਲ_Even);
  ਪ੍ਰਤਿਜ੍ਞਾ(ਸਥਿਤੀ_Or_Err && "Invalid floating point representation");
  consumeError(ਸਥਿਤੀ_Or_Err.takeError());
}

FPClassTest APFloat::classify() ਅਚਲ {
  ਜੇ (isZero())
    ਵਾਪਸ isNegative() ? fcNegZero : fcPosZero;
  ਜੇ (is_ਸਾਧਾਰਨ())
    ਵਾਪਸ isNegative() ? fc_Neg_ਸਾਧਾਰਨ : fc_Pos_ਸਾਧਾਰਨ;
  ਜੇ (isDenormal())
    ਵਾਪਸ isNegative() ? fcNegSubnormal : fcPosSubnormal;
  ਜੇ (isInfinity())
    ਵਾਪਸ isNegative() ? fcNegInf : fcPosInf;
  ਪ੍ਰਤਿਜ੍ਞਾ(isNaN() && "Other class of FP constant");
  ਵਾਪਸ isSignaling() ? fcSNan : fcQNan;
}

ਤਾਰਕਿਕ APFloat::getExactInverse(APFloat *Inv) ਅਚਲ {
  // Only finite, non-zero numbers can have a useful, representable inverse.
  // This check filters out +/- zero, +/- infinity, and NaN.
  ਜੇ (!isFiniteNonZero())
    ਵਾਪਸ ਝੂਠ;

  // Historically, this function rejects subnormal inputs.  One reason why this
  // might be important is that subnormals may behave differently under FTZ/DAZ
  // runtime behavior.
  ਜੇ (isDenormal())
    ਵਾਪਸ ਝੂਠ;

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
  ਅਚਲ ਪੂਰਨ Exp = getExactLog2Abs();
  ਜੇ (Exp == INT_MIN)
    ਵਾਪਸ ਝੂਠ;

  // The inverse of +/- 2^Exp is +/- 2^(-Exp). We can compute this by
  // scaling 1.0 by the negated exponent.
  APFloat Reciprocal =
      scalbn(APFloat::getOne(getSemantics(), /*Negative=*/isNegative()), -Exp,
             rmTowardZero);

  // scalbn might round if the resulting exponent -Exp is outside the
  // representable range, causing overflow (to infinity) or underflow. We
  // must verify that the result is still the exact power of two we expect.
  ਜੇ (Reciprocal.getExactLog2Abs() != -Exp)
    ਵਾਪਸ ਝੂਠ;

  // Avoid multiplication with a subnormal, it is not safe on all platforms and
  // may be slower than a normal division.
  ਜੇ (Reciprocal.isDenormal())
    ਵਾਪਸ ਝੂਠ;

  ਪ੍ਰਤਿਜ੍ਞਾ(Reciprocal.isFiniteNonZero());

  ਜੇ (Inv)
    *Inv = std::move(Reciprocal);

  ਵਾਪਸ ਸੱਚ;
}

APFloat::op_ਸਥਿਤੀ APFloat::convert(ਅਚਲ fltSemantics &ਮੰਜ਼ਿਲ_Semantics_2,
                                   rounding_ਬਹੁਲਕ RM, ਤਾਰਕਿਕ *losesInfo) {
  ਜੇ (&getSemantics() == &ਮੰਜ਼ਿਲ_Semantics_2) {
    *losesInfo = ਝੂਠ;
    ਵਾਪਸ opOK;
  }
  ਜੇ (ਵਰਤਦਾ_Layout<IEEEFloat>(getSemantics()) &&
      ਵਰਤਦਾ_Layout<IEEEFloat>(ਮੰਜ਼ਿਲ_Semantics_2))
    ਵਾਪਸ U.IEEE.convert(ਮੰਜ਼ਿਲ_Semantics_2, RM, losesInfo);
  ਜੇ (ਵਰਤਦਾ_Layout<IEEEFloat>(getSemantics()) &&
      ਵਰਤਦਾ_Layout<DoubleAPFloat>(ਮੰਜ਼ਿਲ_Semantics_2)) {
    ਪ੍ਰਤਿਜ੍ਞਾ(&ਮੰਜ਼ਿਲ_Semantics_2 == &APFloat_ਅਧਾਰ::semPPCDoubleDouble);
    ਸਵੈਚਾਲਿਤ Ret =
        U.IEEE.convert(APFloat_ਅਧਾਰ::semPPCDoubleDoubleLegacy, RM, losesInfo);
    *ਇਹ = APFloat(ਮੰਜ਼ਿਲ_Semantics_2, U.IEEE.bitcast_ਮੰਜ਼ਿਲ_APInt());
    ਵਾਪਸ Ret;
  }
  ਜੇ (ਵਰਤਦਾ_Layout<DoubleAPFloat>(getSemantics()) &&
      ਵਰਤਦਾ_Layout<IEEEFloat>(ਮੰਜ਼ਿਲ_Semantics_2)) {
    ਸਵੈਚਾਲਿਤ Ret = getIEEE().convert(ਮੰਜ਼ਿਲ_Semantics_2, RM, losesInfo);
    *ਇਹ = APFloat(std::move(getIEEE()), ਮੰਜ਼ਿਲ_Semantics_2);
    ਵਾਪਸ Ret;
  }
  llvm_unreachable("Unexpected semantics");
}

APFloat APFloat::get_All_Ones_ਮੁੱਲ(ਅਚਲ fltSemantics &Semantics) {
  ਵਾਪਸ APFloat(Semantics, APInt::getAllOnes(Semantics.ਮਾਪ_ਵਿੱਚ_ਬਿੱਟ));
}

ਖਾਲੀ APFloat::print(raw_ostream &OS) ਅਚਲ {
  Small_ਸਦਿਸ਼<ਅੱਖਰ, 16> ਬਫਰ;
  ਮੰਜ਼ਿਲ_String(ਬਫਰ);
  OS << ਬਫਰ;
}

#ਜੇ !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)
LLVM_DUMP_ਵਿਧੀ ਖਾਲੀ APFloat::dump() ਅਚਲ {
  print(dbgs());
  dbgs() << '\n';
}
#ਜੇ_ਅੰਤ

ਖਾਲੀ APFloat::Profile(Folding_ਸੈੱਟ_ਗੰਢ_ID &NID) ਅਚਲ {
  NID.ਜੋੜੋ_2(bitcast_ਮੰਜ਼ਿਲ_APInt());
}

APFloat::op_ਸਥਿਤੀ APFloat::convert_ਮੰਜ਼ਿਲ_Integer(APSInt &ਨਤੀਜਾ_2,
                                            rounding_ਬਹੁਲਕ rounding_ਬਹੁਲਕ_2,
                                            ਤਾਰਕਿਕ *isExact) ਅਚਲ {
  ਬਿਨਾਨਿਸ਼ਾਨੀ ਦੋਆਧਾਰੀ_ਅੰਕ_ਚੌੜਾਈ = ਨਤੀਜਾ_2.get_ਦੋਆਧਾਰੀ_ਅੰਕ_ਚੌੜਾਈ();
  Small_ਸਦਿਸ਼<uint64_t, 4> parts(ਨਤੀਜਾ_2.get_ਅੰਕ_ਸ਼ਬਦ());
  op_ਸਥਿਤੀ ਸਥਿਤੀ_2 = convert_ਮੰਜ਼ਿਲ_Integer(parts, ਦੋਆਧਾਰੀ_ਅੰਕ_ਚੌੜਾਈ, ਨਤੀਜਾ_2.isSigned(),
                                     rounding_ਬਹੁਲਕ_2, isExact);
  // Keeps the original signed-ness.
  ਨਤੀਜਾ_2 = APInt(ਦੋਆਧਾਰੀ_ਅੰਕ_ਚੌੜਾਈ, parts);
  ਵਾਪਸ ਸਥਿਤੀ_2;
}

ਦੁੱਗਣਾ APFloat::convert_ਮੰਜ਼ਿਲ_Double() ਅਚਲ {
  ਜੇ (&getSemantics() == &APFloat_ਅਧਾਰ::semIEEEdouble)
    ਵਾਪਸ getIEEE().convert_ਮੰਜ਼ਿਲ_Double();
  ਪ੍ਰਤਿਜ੍ਞਾ(isRepresentableBy(getSemantics(), semIEEEdouble) &&
         "Float semantics is not representable by IEEEdouble");
  APFloat ਆਰਜ਼ੀ = *ਇਹ;
  ਤਾਰਕਿਕ LosesInfo;
  [[maybe_ਅਣਵਰਤਿਆ]] op_ਸਥਿਤੀ St =
      ਆਰਜ਼ੀ.convert(APFloat_ਅਧਾਰ::semIEEEdouble, rm_ਸਭ_ਤੋਂ_ਨੇੜੇ_Ties_ਮੰਜ਼ਿਲ_Even, &LosesInfo);
  ਪ੍ਰਤਿਜ੍ਞਾ(!(St & opInexact) && !LosesInfo && "Unexpected imprecision");
  ਵਾਪਸ ਆਰਜ਼ੀ.getIEEE().convert_ਮੰਜ਼ਿਲ_Double();
}

#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ HAS_IEE754_FLOAT128
float128 APFloat::convert_ਮੰਜ਼ਿਲ_Quad() ਅਚਲ {
  ਜੇ (&getSemantics() == &APFloat_ਅਧਾਰ::semIEEEquad)
    ਵਾਪਸ getIEEE().convert_ਮੰਜ਼ਿਲ_Quad();
  ਪ੍ਰਤਿਜ੍ਞਾ(isRepresentableBy(getSemantics(), semIEEEquad) &&
         "Float semantics is not representable by IEEEquad");
  APFloat ਆਰਜ਼ੀ = *ਇਹ;
  ਤਾਰਕਿਕ LosesInfo;
  [[maybe_ਅਣਵਰਤਿਆ]] op_ਸਥਿਤੀ St =
      ਆਰਜ਼ੀ.convert(APFloat_ਅਧਾਰ::semIEEEquad, rm_ਸਭ_ਤੋਂ_ਨੇੜੇ_Ties_ਮੰਜ਼ਿਲ_Even, &LosesInfo);
  ਪ੍ਰਤਿਜ੍ਞਾ(!(St & opInexact) && !LosesInfo && "Unexpected imprecision");
  ਵਾਪਸ ਆਰਜ਼ੀ.getIEEE().convert_ਮੰਜ਼ਿਲ_Quad();
}
#ਜੇ_ਅੰਤ

ਦਸ਼ਮਲਵ APFloat::convert_ਮੰਜ਼ਿਲ_Float() ਅਚਲ {
  ਜੇ (&getSemantics() == &APFloat_ਅਧਾਰ::semIEEEsingle)
    ਵਾਪਸ getIEEE().convert_ਮੰਜ਼ਿਲ_Float();
  ਪ੍ਰਤਿਜ੍ਞਾ(isRepresentableBy(getSemantics(), semIEEEsingle) &&
         "Float semantics is not representable by IEEEsingle");
  APFloat ਆਰਜ਼ੀ = *ਇਹ;
  ਤਾਰਕਿਕ LosesInfo;
  [[maybe_ਅਣਵਰਤਿਆ]] op_ਸਥਿਤੀ St =
      ਆਰਜ਼ੀ.convert(APFloat_ਅਧਾਰ::semIEEEsingle, rm_ਸਭ_ਤੋਂ_ਨੇੜੇ_Ties_ਮੰਜ਼ਿਲ_Even, &LosesInfo);
  ਪ੍ਰਤਿਜ੍ਞਾ(!(St & opInexact) && !LosesInfo && "Unexpected imprecision");
  ਵਾਪਸ ਆਰਜ਼ੀ.getIEEE().convert_ਮੰਜ਼ਿਲ_Float();
}

ਬਿਨਾਨਿਸ਼ਾਨੀ APFloat_ਅਧਾਰ::get_Arbitrary_FPFormat_ਮਾਪ_ਵਿੱਚ_ਬਿੱਟ(StringRef Format) {
  ਵਾਪਸ StringSwitch<ਬਿਨਾਨਿਸ਼ਾਨੀ>(Format)
      .Case("Float8E5M2", get_ਮਾਪ_ਵਿੱਚ_ਬਿੱਟ(semFloat8E5M2))
      .Case("Float8E5M2FNUZ", get_ਮਾਪ_ਵਿੱਚ_ਬਿੱਟ(semFloat8E5M2FNUZ))
      .Case("Float8E4M3", get_ਮਾਪ_ਵਿੱਚ_ਬਿੱਟ(semFloat8E4M3))
      .Case("Float8E4M3FN", get_ਮਾਪ_ਵਿੱਚ_ਬਿੱਟ(semFloat8E4M3FN))
      .Case("Float8E4M3FNUZ", get_ਮਾਪ_ਵਿੱਚ_ਬਿੱਟ(semFloat8E4M3FNUZ))
      .Case("Float8E4M3B11FNUZ", get_ਮਾਪ_ਵਿੱਚ_ਬਿੱਟ(semFloat8E4M3B11FNUZ))
      .Case("Float8E3M4", get_ਮਾਪ_ਵਿੱਚ_ਬਿੱਟ(semFloat8E3M4))
      .Case("Float8E8M0FNU", get_ਮਾਪ_ਵਿੱਚ_ਬਿੱਟ(semFloat8E8M0FNU))
      .Case("Float6E3M2FN", get_ਮਾਪ_ਵਿੱਚ_ਬਿੱਟ(semFloat6E3M2FN))
      .Case("Float6E2M3FN", get_ਮਾਪ_ਵਿੱਚ_ਬਿੱਟ(semFloat6E2M3FN))
      .Case("Float4E2M1FN", get_ਮਾਪ_ਵਿੱਚ_ਬਿੱਟ(semFloat4E2M1FN))
      .Case("Float8E5M3FNU", get_ਮਾਪ_ਵਿੱਚ_ਬਿੱਟ(semFloat8E5M3FNU))
      .Default(0);
}

ਤਾਰਕਿਕ APFloat_ਅਧਾਰ::isValidArbitraryFPFormat(StringRef Format) {
  ਵਾਪਸ get_Arbitrary_FPFormat_ਮਾਪ_ਵਿੱਚ_ਬਿੱਟ(Format) != 0;
}

ਅਚਲ fltSemantics *APFloat_ਅਧਾਰ::getArbitraryFPSemantics(StringRef Format) {
  // TODO: extend to remaining arbitrary FP types: Float8E4M3, Float8E3M4,
  // Float8E5M2FNUZ, Float8E4M3FNUZ, Float8E4M3B11FNUZ, Float8E8M0FNU.
  ਵਾਪਸ StringSwitch<ਅਚਲ fltSemantics *>(Format)
      .Case("Float8E5M2", &semFloat8E5M2)
      .Case("Float8E4M3FN", &semFloat8E4M3FN)
      .Case("Float8E5M3FNU", &semFloat8E5M3FNU)
      .Case("Float4E2M1FN", &semFloat4E2M1FN)
      .Case("Float6E3M2FN", &semFloat6E3M2FN)
      .Case("Float6E2M3FN", &semFloat6E2M3FN)
      .Default(ਖਾਲੀ_ਸੂਚਕ);
}

APFloat::Storage::~Storage() {
  ਜੇ (ਵਰਤਦਾ_Layout<IEEEFloat>(*semantics)) {
    IEEE.~IEEEFloat();
    ਵਾਪਸ;
  }
  ਜੇ (ਵਰਤਦਾ_Layout<DoubleAPFloat>(*semantics)) {
    Double.~DoubleAPFloat();
    ਵਾਪਸ;
  }
  llvm_unreachable("Unexpected semantics");
}

APFloat::Storage::Storage(ਅਚਲ APFloat::Storage &RHS) {
  ਜੇ (ਵਰਤਦਾ_Layout<IEEEFloat>(*RHS.semantics)) {
    ਨਵਾਂ (ਇਹ) IEEEFloat(RHS.IEEE);
    ਵਾਪਸ;
  }
  ਜੇ (ਵਰਤਦਾ_Layout<DoubleAPFloat>(*RHS.semantics)) {
    ਨਵਾਂ (ਇਹ) DoubleAPFloat(RHS.Double);
    ਵਾਪਸ;
  }
  llvm_unreachable("Unexpected semantics");
}

APFloat::Storage::Storage(APFloat::Storage &&RHS) {
  ਜੇ (ਵਰਤਦਾ_Layout<IEEEFloat>(*RHS.semantics)) {
    ਨਵਾਂ (ਇਹ) IEEEFloat(std::move(RHS.IEEE));
    ਵਾਪਸ;
  }
  ਜੇ (ਵਰਤਦਾ_Layout<DoubleAPFloat>(*RHS.semantics)) {
    ਨਵਾਂ (ਇਹ) DoubleAPFloat(std::move(RHS.Double));
    ਵਾਪਸ;
  }
  llvm_unreachable("Unexpected semantics");
}

APFloat::Storage &APFloat::Storage::ਸੰਚਾਲਕ=(ਅਚਲ APFloat::Storage &RHS) {
  ਜੇ (ਵਰਤਦਾ_Layout<IEEEFloat>(*semantics) &&
      ਵਰਤਦਾ_Layout<IEEEFloat>(*RHS.semantics)) {
    IEEE = RHS.IEEE;
  } ਨਹੀਂਤਾਂ ਜੇ (ਵਰਤਦਾ_Layout<DoubleAPFloat>(*semantics) &&
             ਵਰਤਦਾ_Layout<DoubleAPFloat>(*RHS.semantics)) {
    Double = RHS.Double;
  } ਨਹੀਂਤਾਂ ਜੇ (ਇਹ != &RHS) {
    ਇਹ->~Storage();
    ਨਵਾਂ (ਇਹ) Storage(RHS);
  }
  ਵਾਪਸ *ਇਹ;
}

APFloat::Storage &APFloat::Storage::ਸੰਚਾਲਕ=(APFloat::Storage &&RHS) {
  ਜੇ (ਵਰਤਦਾ_Layout<IEEEFloat>(*semantics) &&
      ਵਰਤਦਾ_Layout<IEEEFloat>(*RHS.semantics)) {
    IEEE = std::move(RHS.IEEE);
  } ਨਹੀਂਤਾਂ ਜੇ (ਵਰਤਦਾ_Layout<DoubleAPFloat>(*semantics) &&
             ਵਰਤਦਾ_Layout<DoubleAPFloat>(*RHS.semantics)) {
    Double = std::move(RHS.Double);
  } ਨਹੀਂਤਾਂ ਜੇ (ਇਹ != &RHS) {
    ਇਹ->~Storage();
    ਨਵਾਂ (ਇਹ) Storage(std::move(RHS));
  }
  ਵਾਪਸ *ਇਹ;
}

ਨਾਮਾਕਾਸ਼ {

APFloat::op_ਸਥਿਤੀ get_Op_ਸਥਿਤੀ_From_Libc(ਪੂਰਨ libc_exceptions) {
  APFloat::op_ਸਥਿਤੀ ਸਥਿਤੀ_2 = APFloat::opOK;
  ਜੇ (libc_exceptions & FE_INVALID)
    ਸਥਿਤੀ_2 = static_cast<APFloat::op_ਸਥਿਤੀ>(ਸਥਿਤੀ_2 | APFloat::opInvalidOp);
  ਜੇ (libc_exceptions & FE_DIVBYZERO)
    ਸਥਿਤੀ_2 = static_cast<APFloat::op_ਸਥਿਤੀ>(ਸਥਿਤੀ_2 | APFloat::opDivByZero);
  ਜੇ (libc_exceptions & FE_ਉਛਾਲ)
    ਸਥਿਤੀ_2 = static_cast<APFloat::op_ਸਥਿਤੀ>(ਸਥਿਤੀ_2 | APFloat::op_ਉਛਾਲ);
  ਜੇ (libc_exceptions & FE_UNDERFLOW)
    ਸਥਿਤੀ_2 = static_cast<APFloat::op_ਸਥਿਤੀ>(ਸਥਿਤੀ_2 | APFloat::opUnderflow);
  ਜੇ (libc_exceptions & FE_INEXACT)
    ਸਥਿਤੀ_2 = static_cast<APFloat::op_ਸਥਿਤੀ>(ਸਥਿਤੀ_2 | APFloat::opInexact);
  ਵਾਪਸ ਸਥਿਤੀ_2;
}

} // namespace

// TODO: Support other rounding modes when LLVM libc math implement static
// roundings.
std::optional<APFloat> ਘਾਤਾਂਕ(ਅਚਲ APFloat &x, Rounding_ਬਹੁਲਕ rounding_ਬਹੁਲਕ_2,
                           APFloat::op_ਸਥਿਤੀ *ਸਥਿਤੀ_2) {

  ਜੇ (rounding_ਬਹੁਲਕ_2 == APFloat_ਅਧਾਰ::rm_ਸਭ_ਤੋਂ_ਨੇੜੇ_Ties_ਮੰਜ਼ਿਲ_Even) {
    ਜੇ (APFloat::Semantics_ਮੰਜ਼ਿਲ_Enum(x.getSemantics()) ==
        APFloat_ਅਧਾਰ::S_IEEEsingle) {
      ਦਸ਼ਮਲਵ x_val = x.convert_ਮੰਜ਼ਿਲ_Float();
      ਪੂਰਨ exc =
          LIBC_NAMESPACE::shared::check::exp_exceptions(x_val, FE_TONEAREST);
      ਜੇ (ਸਥਿਤੀ_2) {
        *ਸਥਿਤੀ_2 = get_Op_ਸਥਿਤੀ_From_Libc(exc);
        ਜੇ (x.isSignaling()) {
          // 32-bit x86 will silence sNaN when loading floats, so we explicitly
          // add the INVALID exception here.
          *ਸਥਿਤੀ_2 =
              static_cast<APFloat::op_ਸਥਿਤੀ>(*ਸਥਿਤੀ_2 | APFloat::opInvalidOp);
        }
      }
      ਦਸ਼ਮਲਵ ਨਤੀਜਾ_2 = LIBC_NAMESPACE::shared::expf(x_val);
      ਵਾਪਸ APFloat(ਨਤੀਜਾ_2);
    }
    ਜੇ (APFloat::Semantics_ਮੰਜ਼ਿਲ_Enum(x.getSemantics()) ==
        APFloat_ਅਧਾਰ::S_IEEEdouble) {
      ਦੁੱਗਣਾ x_val = x.convert_ਮੰਜ਼ਿਲ_Double();
      ਪੂਰਨ exc =
          LIBC_NAMESPACE::shared::check::exp_exceptions(x_val, FE_TONEAREST);
      ਜੇ (ਸਥਿਤੀ_2) {
        *ਸਥਿਤੀ_2 = get_Op_ਸਥਿਤੀ_From_Libc(exc);
        ਜੇ (x.isSignaling()) {
          // 32-bit x86 will silence sNaN when loading floats, so we explicitly
          // add the INVALID exception here.
          *ਸਥਿਤੀ_2 =
              static_cast<APFloat::op_ਸਥਿਤੀ>(*ਸਥਿਤੀ_2 | APFloat::opInvalidOp);
        }
      }
      ਦੁੱਗਣਾ ਨਤੀਜਾ_2 = LIBC_NAMESPACE::shared::ਘਾਤਾਂਕ(x_val);
      ਵਾਪਸ APFloat(ਨਤੀਜਾ_2);
    }
  }
  ਵਾਪਸ std::nullopt;
}

} // namespace llvm

#ਪਰਿਭਾਸ਼ਾ_ਰੱਦ APFLOAT_DISPATCH_ON_SEMANTICS
