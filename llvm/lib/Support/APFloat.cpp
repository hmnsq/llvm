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

#ಅನ್ತರ್ಭಾವ "llvm/ADT/APFloat.h"
#ಅನ್ತರ್ಭಾವ "llvm/ADT/APSInt.h"
#ಅನ್ತರ್ಭಾವ "llvm/ADT/ArrayRef.h"
#ಅನ್ತರ್ಭಾವ "llvm/ADT/FloatingPointMode.h"
#ಅನ್ತರ್ಭಾವ "llvm/ADT/FoldingSet.h"
#ಅನ್ತರ್ಭಾವ "llvm/ADT/Hashing.h"
#ಅನ್ತರ್ಭಾವ "llvm/ADT/STLExtras.h"
#ಅನ್ತರ್ಭಾವ "llvm/ADT/SmallVector.h"
#ಅನ್ತರ್ಭಾವ "llvm/ADT/StringExtras.h"
#ಅನ್ತರ್ಭಾವ "llvm/ADT/StringRef.h"
#ಅನ್ತರ್ಭಾವ "llvm/ADT/StringSwitch.h"
#ಅನ್ತರ್ಭಾವ "llvm/Config/llvm-config.h"
#ಅನ್ತರ್ಭಾವ "llvm/Support/Debug.h"
#ಅನ್ತರ್ಭಾವ "llvm/Support/Error.h"
#ಅನ್ತರ್ಭಾವ "llvm/Support/MathExtras.h"
#ಅನ್ತರ್ಭಾವ "llvm/Support/raw_ostream.h"
#ಅನ್ತರ್ಭಾವ <cstring>
#ಅನ್ತರ್ಭಾವ <iterator>
#ಅನ್ತರ್ಭಾವ <limits.h>

/// Shared headers from LLVM libc
/// Make sure to add ${LLVM_SOURCE_DIR}/../libc to include directories.
///
/// Notes: So far it looks like APFloat does not check errnos or floating-point
/// exceptions after calling the math functions, so we will configure LLVM libc
/// math functions to skip setting errnos and floating-point exceptions
/// explicitly.  We also put them in a separate namespace so that the symbols
/// do not clash with other libc math builds just in case.
#ವ್ಯಾಖ್ಯಾನಿಸು LIBC_NAMESPACE __llvm_libc_apfloat
#ವ್ಯಾಖ್ಯಾನಿಸು LIBC_MATH (LIBC_MATH_NO_ERRNO | LIBC_MATH_NO_EXCEPT)

#ಅನ್ತರ್ಭಾವ "shared/math.h"
#ಅನ್ತರ್ಭಾವ "shared/math_check_exceptions.h"

#ವ್ಯಾಖ್ಯಾನಿಸು APFLOAT_DISPATCH_ON_SEMANTICS(ವಿಧಾನ_CALL)                             \
  ಮಾಡು {                                                                         \
    ಆದರೆ (ಬಳಸುತ್ತದೆ_Layout<IEEEFloat>(getSemantics()))                                 \
      ಹಿಂತಿರುಗಿ U.IEEE.ವಿಧಾನ_CALL;                                               \
    ಆದರೆ (ಬಳಸುತ್ತದೆ_Layout<DoubleAPFloat>(getSemantics()))                             \
      ಹಿಂತಿರುಗಿ U.Double.ವಿಧಾನ_CALL;                                             \
    llvm_unreachable("Unexpected semantics");                                  \
  } ಆಗ (ಸುಳ್ಳು)

ಬಳಸಿ ನಾಮಾಕಾಶ llvm;

/// A macro used to combine two fcCategory enums into one key which can be used
/// in a switch statement to classify how the interaction of two APFloat's
/// categories affects an operation.
///
/// TODO: If clang source code is ever allowed to use constexpr in its own
/// codebase, change this into a static inline function.
#ವ್ಯಾಖ್ಯಾನಿಸು Pack_Categories_Into_ಕೀಲಿ(_lhs, _rhs) ((_lhs) * 4 + (_rhs))

/* Assumed in hexadecimal significand parsing, and conversion to
   hexadecimal strings.  */
static_assert(APFloat_ತಳ::integer_Part_ಅಗಲ % 4 == 0, "Part width must be divisible by 4!");

ನಾಮಾಕಾಶ llvm {

constexpr fltSemantics APFloat_ತಳ::semIEEEhalf = {15, -14, 11, 16};
constexpr fltSemantics APFloat_ತಳ::semBFloat = {127, -126, 8, 16};
constexpr fltSemantics APFloat_ತಳ::semIEEEsingle = {127, -126, 24, 32};
constexpr fltSemantics APFloat_ತಳ::semIEEEdouble = {1023, -1022, 53, 64};
constexpr fltSemantics APFloat_ತಳ::semIEEEquad = {16383, -16382, 113, 128};
constexpr fltSemantics APFloat_ತಳ::semX87DoubleExtended = {
    16383,
    -16382,
    64,
    80,
    fltNonfiniteBehavior::IEEE754,
    fltNanEncoding::IEEE,
    ನಿಜ,
    ನಿಜ,
    ನಿಜ,
    ನಿಜ,
    ನಿಜ};
constexpr fltSemantics APFloat_ತಳ::semPPCDoubleDouble = {-1, 0, 0, 128};
constexpr fltSemantics APFloat_ತಳ::semPPCDoubleDoubleLegacy = {
    1023, -1022 + 53, 53 + 53, 128};
constexpr fltSemantics APFloat_ತಳ::semFloat8E5M2 = {15, -14, 3, 8};
constexpr fltSemantics APFloat_ತಳ::semFloat8E5M2FNUZ = {
    15, -15, 3, 8, fltNonfiniteBehavior::NanOnly, fltNanEncoding::NegativeZero};
constexpr fltSemantics APFloat_ತಳ::semFloat8E4M3 = {7, -6, 4, 8};
constexpr fltSemantics APFloat_ತಳ::semFloat8E4M3FN = {
    8, -6, 4, 8, fltNonfiniteBehavior::NanOnly, fltNanEncoding::AllOnes};
constexpr fltSemantics APFloat_ತಳ::semFloat8E4M3FNUZ = {
    7, -7, 4, 8, fltNonfiniteBehavior::NanOnly, fltNanEncoding::NegativeZero};
constexpr fltSemantics APFloat_ತಳ::semFloat8E4M3B11FNUZ = {
    4, -10, 4, 8, fltNonfiniteBehavior::NanOnly, fltNanEncoding::NegativeZero};
constexpr fltSemantics APFloat_ತಳ::semFloat8E3M4 = {3, -2, 5, 8};
constexpr fltSemantics APFloat_ತಳ::semFloatTF32 = {127, -126, 11, 19};
constexpr fltSemantics APFloat_ತಳ::semFloat8E8M0FNU = {
    127,
    -127,
    1,
    8,
    fltNonfiniteBehavior::NanOnly,
    fltNanEncoding::AllOnes,
    ಸುಳ್ಳು,
    ಸುಳ್ಳು,
    ಸುಳ್ಳು,
    ಸುಳ್ಳು};

constexpr fltSemantics APFloat_ತಳ::semFloat8E5M3FNU = {
    16,
    -14,
    4,
    8,
    fltNonfiniteBehavior::NanOnly,
    fltNanEncoding::AllOnes,
    ನಿಜ,
    ಸುಳ್ಳು,
    ಸುಳ್ಳು};

constexpr fltSemantics APFloat_ತಳ::semFloat6E3M2FN = {
    4, -2, 3, 6, fltNonfiniteBehavior::FiniteOnly};
constexpr fltSemantics APFloat_ತಳ::semFloat6E2M3FN = {
    2, 0, 4, 6, fltNonfiniteBehavior::FiniteOnly};
constexpr fltSemantics APFloat_ತಳ::semFloat4E2M1FN = {
    2, 0, 2, 4, fltNonfiniteBehavior::FiniteOnly};
constexpr fltSemantics APFloat_ತಳ::semBogus = {0, 0, 0, 0};

ಸ್ಥಿರಾಂಕ llvm::fltSemantics &APFloat_ತಳ::Enum_ಗಮ್ಯ_Semantics(Semantics S) {
  ಆಯ್ಕೆ (S) {
  ಸ್ಥಿತಿ S_IEEEhalf:
    ಹಿಂತಿರುಗಿ IEEEhalf();
  ಸ್ಥಿತಿ S_BFloat:
    ಹಿಂತಿರುಗಿ BFloat();
  ಸ್ಥಿತಿ S_IEEEsingle:
    ಹಿಂತಿರುಗಿ IEEEsingle();
  ಸ್ಥಿತಿ S_IEEEdouble:
    ಹಿಂತಿರುಗಿ IEEEdouble();
  ಸ್ಥಿತಿ S_IEEEquad:
    ಹಿಂತಿರುಗಿ IEEEquad();
  ಸ್ಥಿತಿ S_x87DoubleExtended:
    ಹಿಂತಿರುಗಿ x87DoubleExtended();
  ಸ್ಥಿತಿ S_PPCDoubleDouble:
    ಹಿಂತಿರುಗಿ PPCDoubleDouble();
  ಸ್ಥಿತಿ S_PPCDoubleDoubleLegacy:
    ಹಿಂತಿರುಗಿ PPCDoubleDoubleLegacy();
  ಸ್ಥಿತಿ S_Float8E5M2:
    ಹಿಂತಿರುಗಿ Float8E5M2();
  ಸ್ಥಿತಿ S_Float8E5M2FNUZ:
    ಹಿಂತಿರುಗಿ Float8E5M2FNUZ();
  ಸ್ಥಿತಿ S_Float8E4M3:
    ಹಿಂತಿರುಗಿ Float8E4M3();
  ಸ್ಥಿತಿ S_Float8E4M3FN:
    ಹಿಂತಿರುಗಿ Float8E4M3FN();
  ಸ್ಥಿತಿ S_Float8E4M3FNUZ:
    ಹಿಂತಿರುಗಿ Float8E4M3FNUZ();
  ಸ್ಥಿತಿ S_Float8E4M3B11FNUZ:
    ಹಿಂತಿರುಗಿ Float8E4M3B11FNUZ();
  ಸ್ಥಿತಿ S_Float8E3M4:
    ಹಿಂತಿರುಗಿ Float8E3M4();
  ಸ್ಥಿತಿ S_FloatTF32:
    ಹಿಂತಿರುಗಿ FloatTF32();
  ಸ್ಥಿತಿ S_Float8E8M0FNU:
    ಹಿಂತಿರುಗಿ Float8E8M0FNU();
  ಸ್ಥಿತಿ S_Float8E5M3FNU:
    ಹಿಂತಿರುಗಿ Float8E5M3FNU();
  ಸ್ಥಿತಿ S_Float6E3M2FN:
    ಹಿಂತಿರುಗಿ Float6E3M2FN();
  ಸ್ಥಿತಿ S_Float6E2M3FN:
    ಹಿಂತಿರುಗಿ Float6E2M3FN();
  ಸ್ಥಿತಿ S_Float4E2M1FN:
    ಹಿಂತಿರುಗಿ Float4E2M1FN();
  }
  llvm_unreachable("Unrecognised floating semantics");
}

APFloat_ತಳ::Semantics
APFloat_ತಳ::Semantics_ಗಮ್ಯ_Enum(ಸ್ಥಿರಾಂಕ llvm::fltSemantics &Sem) {
  ಆದರೆ (&Sem == &llvm::APFloat::IEEEhalf())
    ಹಿಂತಿರುಗಿ S_IEEEhalf;
  ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (&Sem == &llvm::APFloat::BFloat())
    ಹಿಂತಿರುಗಿ S_BFloat;
  ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (&Sem == &llvm::APFloat::IEEEsingle())
    ಹಿಂತಿರುಗಿ S_IEEEsingle;
  ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (&Sem == &llvm::APFloat::IEEEdouble())
    ಹಿಂತಿರುಗಿ S_IEEEdouble;
  ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (&Sem == &llvm::APFloat::IEEEquad())
    ಹಿಂತಿರುಗಿ S_IEEEquad;
  ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (&Sem == &llvm::APFloat::x87DoubleExtended())
    ಹಿಂತಿರುಗಿ S_x87DoubleExtended;
  ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (&Sem == &llvm::APFloat::PPCDoubleDouble())
    ಹಿಂತಿರುಗಿ S_PPCDoubleDouble;
  ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (&Sem == &llvm::APFloat::PPCDoubleDoubleLegacy())
    ಹಿಂತಿರುಗಿ S_PPCDoubleDoubleLegacy;
  ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (&Sem == &llvm::APFloat::Float8E5M2())
    ಹಿಂತಿರುಗಿ S_Float8E5M2;
  ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (&Sem == &llvm::APFloat::Float8E5M2FNUZ())
    ಹಿಂತಿರುಗಿ S_Float8E5M2FNUZ;
  ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (&Sem == &llvm::APFloat::Float8E4M3())
    ಹಿಂತಿರುಗಿ S_Float8E4M3;
  ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (&Sem == &llvm::APFloat::Float8E4M3FN())
    ಹಿಂತಿರುಗಿ S_Float8E4M3FN;
  ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (&Sem == &llvm::APFloat::Float8E4M3FNUZ())
    ಹಿಂತಿರುಗಿ S_Float8E4M3FNUZ;
  ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (&Sem == &llvm::APFloat::Float8E4M3B11FNUZ())
    ಹಿಂತಿರುಗಿ S_Float8E4M3B11FNUZ;
  ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (&Sem == &llvm::APFloat::Float8E3M4())
    ಹಿಂತಿರುಗಿ S_Float8E3M4;
  ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (&Sem == &llvm::APFloat::FloatTF32())
    ಹಿಂತಿರುಗಿ S_FloatTF32;
  ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (&Sem == &llvm::APFloat::Float8E8M0FNU())
    ಹಿಂತಿರುಗಿ S_Float8E8M0FNU;
  ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (&Sem == &llvm::APFloat::Float8E5M3FNU())
    ಹಿಂತಿರುಗಿ S_Float8E5M3FNU;
  ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (&Sem == &llvm::APFloat::Float6E3M2FN())
    ಹಿಂತಿರುಗಿ S_Float6E3M2FN;
  ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (&Sem == &llvm::APFloat::Float6E2M3FN())
    ಹಿಂತಿರುಗಿ S_Float6E2M3FN;
  ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (&Sem == &llvm::APFloat::Float4E2M1FN())
    ಹಿಂತಿರುಗಿ S_Float4E2M1FN;
  ಇಲ್ಲದಿದ್ದರೆ
    llvm_unreachable("Unknown floating semantics");
}

ತಾರ್ಕಿಕ APFloat_ತಳ::isRepresentableBy(ಸ್ಥಿರಾಂಕ fltSemantics &A,
                                    ಸ್ಥಿರಾಂಕ fltSemantics &B) {
  ಹಿಂತಿರುಗಿ A.maxExponent <= B.maxExponent && A.minExponent >= B.minExponent &&
         A.precision <= B.precision;
}

ತಾರ್ಕಿಕ APFloat_ತಳ::is_Losslessly_Convertible_ಗಮ್ಯ(ಸ್ಥಿರಾಂಕ fltSemantics &From,
                                            ಸ್ಥಿರಾಂಕ fltSemantics &Initial_ಗಮ್ಯ,
                                            ತಾರ್ಕಿಕ IgnoreNaNs) {
  ಆದರೆ (&From == &Initial_ಗಮ್ಯ)
    ಹಿಂತಿರುಗಿ ನಿಜ;

  // PPC double-double cannot be described by a conventional exponent range
  // and precision. In particular, converting it to another semantics drops
  // its low double, so conservatively reject conversions from it.
  ಆದರೆ (&From == &semPPCDoubleDouble)
    ಹಿಂತಿರುಗಿ ಸುಳ್ಳು;

  // Converting to PPC double-double is lossless exactly when the value fits
  // in its high double. In other words, the conversion is lossless if the
  // conversion to IEEEdouble is lossless.
  ಸ್ಥಿರಾಂಕ fltSemantics &ಗಮ್ಯ =
      &Initial_ಗಮ್ಯ == &semPPCDoubleDouble ? semIEEEdouble : Initial_ಗಮ್ಯ;

  ಆದರೆ (!isRepresentableBy(From, ಗಮ್ಯ))
    ಹಿಂತಿರುಗಿ ಸುಳ್ಳು;

  ಆದರೆ ((From.hasZero && !ಗಮ್ಯ.hasZero) ||
      (From.hasSignedRepr && !ಗಮ್ಯ.hasSignedRepr))
    ಹಿಂತಿರುಗಿ ಸುಳ್ಳು;

  // NegativeZero NaN encoding repurposes the negative-zero bit pattern, so a
  // conversion to such a format cannot preserve a source negative zero.
  ತಾರ್ಕಿಕ FromHasSignedZero = From.hasZero && From.hasSignedRepr &&
                           From.nanEncoding != fltNanEncoding::NegativeZero;
  ತಾರ್ಕಿಕ ಗಮ್ಯ_Has_Signed_Zero = ಗಮ್ಯ.hasZero && ಗಮ್ಯ.hasSignedRepr &&
                         ಗಮ್ಯ.nanEncoding != fltNanEncoding::NegativeZero;
  ಆದರೆ (FromHasSignedZero && !ಗಮ್ಯ_Has_Signed_Zero)
    ಹಿಂತಿರುಗಿ ಸುಳ್ಳು;

  // isRepresentableBy compares normalized exponent ranges. Also ensure that
  // the smallest source value, which may be denormal, is represented exactly
  // by the destination semantics.
  APFloat ಅತಿಚಿಕ್ಕ_From = APFloat::get_ಅತಿಚಿಕ್ಕ(From);
  ತಾರ್ಕಿಕ LosesInfo = ಸುಳ್ಳು;
  (ಶೂನ್ಯ)ಅತಿಚಿಕ್ಕ_From.convert(ಗಮ್ಯ, APFloat::rm_ಹತ್ತಿರದ_Ties_ಗಮ್ಯ_Even, &LosesInfo);
  ಆದರೆ (LosesInfo)
    ಹಿಂತಿರುಗಿ ಸುಳ್ಳು;

  ಆದರೆ (From.nonFiniteBehavior == fltNonfiniteBehavior::FiniteOnly)
    ಹಿಂತಿರುಗಿ ನಿಜ;

  // Even when NaN representations can be ignored, NaNs must remain NaNs and
  // infinities must remain infinities. Otherwise the conversion can change
  // whether an operation with nnan has poison-producing operands.
  ಆದರೆ (IgnoreNaNs) {
    ಆದರೆ (From.nonFiniteBehavior == fltNonfiniteBehavior::IEEE754)
      ಹಿಂತಿರುಗಿ ಗಮ್ಯ.nonFiniteBehavior == fltNonfiniteBehavior::IEEE754;
    ಹಿಂತಿರುಗಿ ಗಮ್ಯ.nonFiniteBehavior != fltNonfiniteBehavior::FiniteOnly;
  }

  ಆದರೆ (From.nonFiniteBehavior == fltNonfiniteBehavior::IEEE754) {
    // Converting an IEEE signaling NaN to another semantics quiets it, so the
    // original value cannot be recovered by converting it back.
    ಹಿಂತಿರುಗಿ ಸುಳ್ಳು;
  }

  // NanOnly formats have no signaling NaNs. IEEE semantics can represent
  // their quiet NaNs; conversions between NanOnly formats are conservatively
  // accepted only when they use the same NaN encoding.
  ಆದರೆ (ಗಮ್ಯ.nonFiniteBehavior == fltNonfiniteBehavior::IEEE754)
    ಹಿಂತಿರುಗಿ ನಿಜ;
  ಹಿಂತಿರುಗಿ ಗಮ್ಯ.nonFiniteBehavior == fltNonfiniteBehavior::NanOnly &&
         From.nanEncoding == ಗಮ್ಯ.nanEncoding;
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
[[maybe_ಬಳಸದ]] constexpr ಚಿಹ್ನೆರಹಿತ maxExponent = 16383;

// Decimal exponents in the f16 and f32 ranges need at most 2 and 6 parts.
// Keep 16 parts (128 bytes) inline: enough for 5^413, but not a large stack
// frame. Longer fractional spellings use heap storage.
ಸ್ಥಿರ constexpr ಚಿಹ್ನೆರಹಿತ ಅಂಕೆ_Pow5_Parts_On_ರಾಶಿ = 16;

ಸ್ಥಿರ ಚಿಹ್ನೆರಹಿತ ಪೂರ್ಣಾಂಕ power_ದ_Five_Parts_For_Exponent(ಚಿಹ್ನೆರಹಿತ ಪೂರ್ಣಾಂಕ power) {
  ಪ್ರತಿಜ್ಞಾ(power <= maxExponent);

  // 815 / 351 is an upper bound on log2(5).  Reserve an additional part for
  // tcFullMultiply, which writes the full product width even when its most
  // significant part is zero.
  ಹಿಂತಿರುಗಿ 2 + ((power * 815) / (351 * APFloat_ತಳ::integer_Part_ಅಗಲ));
}

ಚಿಹ್ನೆರಹಿತ ಪೂರ್ಣಾಂಕ APFloat_ತಳ::semanticsPrecision(ಸ್ಥಿರಾಂಕ fltSemantics &semantics) {
  ಹಿಂತಿರುಗಿ semantics.precision;
}
APFloat_ತಳ::ExponentType
APFloat_ತಳ::semanticsMaxExponent(ಸ್ಥಿರಾಂಕ fltSemantics &semantics) {
  ಹಿಂತಿರುಗಿ semantics.maxExponent;
}
APFloat_ತಳ::ExponentType
APFloat_ತಳ::semanticsMinExponent(ಸ್ಥಿರಾಂಕ fltSemantics &semantics) {
  ಹಿಂತಿರುಗಿ semantics.minExponent;
}
ಚಿಹ್ನೆರಹಿತ ಪೂರ್ಣಾಂಕ APFloat_ತಳ::semantics_ಗಾತ್ರ_ಒಳ_ಬಿಟ್ಟುಗಳು(ಸ್ಥಿರಾಂಕ fltSemantics &semantics) {
  ಹಿಂತಿರುಗಿ semantics.ಗಾತ್ರ_ಒಳ_ಬಿಟ್ಟುಗಳು;
}
ಚಿಹ್ನೆರಹಿತ ಪೂರ್ಣಾಂಕ APFloat_ತಳ::semantics_Int_ಗಾತ್ರ_ಒಳ_ಬಿಟ್ಟುಗಳು(ಸ್ಥಿರಾಂಕ fltSemantics &semantics,
                                                 ತಾರ್ಕಿಕ isSigned) {
  // The max FP value is pow(2, MaxExponent) * (1 + MaxFraction), so we need
  // at least one more bit than the MaxExponent to hold the max FP value.
  ಚಿಹ್ನೆರಹಿತ ಪೂರ್ಣಾಂಕ Min_ಬಿಟ್_ಅಗಲ = semanticsMaxExponent(semantics) + 1;
  // Extra sign bit needed.
  ಆದರೆ (isSigned)
    ++Min_ಬಿಟ್_ಅಗಲ;
  ಹಿಂತಿರುಗಿ Min_ಬಿಟ್_ಅಗಲ;
}

ತಾರ್ಕಿಕ APFloat_ತಳ::semanticsHasZero(ಸ್ಥಿರಾಂಕ fltSemantics &semantics) {
  ಹಿಂತಿರುಗಿ semantics.hasZero;
}

ತಾರ್ಕಿಕ APFloat_ತಳ::semanticsHasSignedRepr(ಸ್ಥಿರಾಂಕ fltSemantics &semantics) {
  ಹಿಂತಿರುಗಿ semantics.hasSignedRepr;
}

ತಾರ್ಕಿಕ APFloat_ತಳ::semanticsHasInf(ಸ್ಥಿರಾಂಕ fltSemantics &semantics) {
  ಹಿಂತಿರುಗಿ semantics.nonFiniteBehavior == fltNonfiniteBehavior::IEEE754;
}

ತಾರ್ಕಿಕ APFloat_ತಳ::semanticsHasNaN(ಸ್ಥಿರಾಂಕ fltSemantics &semantics) {
  ಹಿಂತಿರುಗಿ semantics.nonFiniteBehavior != fltNonfiniteBehavior::FiniteOnly;
}

ತಾರ್ಕಿಕ APFloat_ತಳ::isIEEELikeFP(ಸ್ಥಿರಾಂಕ fltSemantics &semantics) {
  // Keep in sync with Type::isIEEELikeFPTy
  ಹಿಂತಿರುಗಿ Semantics_ಗಮ್ಯ_Enum(semantics) <= S_IEEEquad;
}

ತಾರ್ಕಿಕ APFloat_ತಳ::has_Sign_ಬಿಟ್_ಒಳ_MSB(ಸ್ಥಿರಾಂಕ fltSemantics &semantics) {
  ಹಿಂತಿರುಗಿ semantics.has_Sign_ಬಿಟ್_ಒಳ_MSB;
}

ತಾರ್ಕಿಕ APFloat_ತಳ::is_Representable_As_ಸಾಮಾನ್ಯ_ಒಳ(ಸ್ಥಿರಾಂಕ fltSemantics &Src,
                                            ಸ್ಥಿರಾಂಕ fltSemantics &Dst) {
  // Exponent range must be larger.
  ಆದರೆ (Src.maxExponent >= Dst.maxExponent || Src.minExponent <= Dst.minExponent)
    ಹಿಂತಿರುಗಿ ಸುಳ್ಳು;

  // If the mantissa is long enough, the result value could still be denormal
  // with a larger exponent range.
  //
  // FIXME: This condition is probably not accurate but also shouldn't be a
  // practical concern with existing types.
  ಹಿಂತಿರುಗಿ Dst.precision >= Src.precision;
}

ಚಿಹ್ನೆರಹಿತ APFloat_ತಳ::get_ಗಾತ್ರ_ಒಳ_ಬಿಟ್ಟುಗಳು(ಸ್ಥಿರಾಂಕ fltSemantics &Sem) {
  ಹಿಂತಿರುಗಿ Sem.ಗಾತ್ರ_ಒಳ_ಬಿಟ್ಟುಗಳು;
}

ಸ್ಥಿರ constexpr APFloat_ತಳ::ExponentType
exponentZero(ಸ್ಥಿರಾಂಕ fltSemantics &semantics) {
  ಹಿಂತಿರುಗಿ semantics.minExponent - 1;
}

ಸ್ಥಿರ constexpr APFloat_ತಳ::ExponentType
exponentInf(ಸ್ಥಿರಾಂಕ fltSemantics &semantics) {
  ಹಿಂತಿರುಗಿ semantics.maxExponent + 1;
}

ಸ್ಥಿರ constexpr APFloat_ತಳ::ExponentType
exponentNaN(ಸ್ಥಿರಾಂಕ fltSemantics &semantics) {
  ಆದರೆ (semantics.nonFiniteBehavior == fltNonfiniteBehavior::NanOnly) {
    ಆದರೆ (semantics.nanEncoding == fltNanEncoding::NegativeZero)
      ಹಿಂತಿರುಗಿ exponentZero(semantics);
    ಆದರೆ (semantics.hasSignedRepr || semantics.precision > 1)
      ಹಿಂತಿರುಗಿ semantics.maxExponent;
  }
  ಹಿಂತಿರುಗಿ semantics.maxExponent + 1;
}

/* A bunch of private, handy routines.  */

ಸ್ಥಿರ ಸಾಲೊಳಗೆ Error createError(ಸ್ಥಿರಾಂಕ Twine &Err) {
  ಹಿಂತಿರುಗಿ make_error<StringError>(Err, inconvertible_Error_ಸಂಕೇತ());
}

ಸ್ಥಿರ constexpr ಸಾಲೊಳಗೆ ಚಿಹ್ನೆರಹಿತ ಪೂರ್ಣಾಂಕ part_ಎಣಿಕೆ_For_ಬಿಟ್ಟುಗಳು(ಚಿಹ್ನೆರಹಿತ ಪೂರ್ಣಾಂಕ ಬಿಟ್ಟುಗಳು) {
  ಹಿಂತಿರುಗಿ std::max(1u, (ಬಿಟ್ಟುಗಳು + APFloat_ತಳ::integer_Part_ಅಗಲ - 1) /
                          APFloat_ತಳ::integer_Part_ಅಗಲ);
}

/* Returns 0U-9U.  Return values >= 10U are not digits.  */
ಸ್ಥಿರ ಸಾಲೊಳಗೆ ಚಿಹ್ನೆರಹಿತ ಪೂರ್ಣಾಂಕ
dec_Digit_ಮೌಲ್ಯ(ಚಿಹ್ನೆರಹಿತ ಪೂರ್ಣಾಂಕ c)
{
  ಹಿಂತಿರುಗಿ c - '0';
}

/* Return the value of a decimal exponent of the form
   [+-]ddddddd.

   If the exponent overflows, returns a large exponent with the
   appropriate sign.  */
ಸ್ಥಿರ Expected<ಪೂರ್ಣಾಂಕ> readExponent(StringRef::iterator begin,
                                  StringRef::iterator ಕೊನೆ) {
  ಸ್ಥಿರಾಂಕ ಚಿಹ್ನೆರಹಿತ ಪೂರ್ಣಾಂಕ overlargeExponent = 24000;  /* FIXME.  */
  StringRef::iterator p = begin;

  // Treat no exponent as 0 to match binutils
  ಆದರೆ (p == ಕೊನೆ || ((*p == '-' || *p == '+') && (p + 1) == ಕೊನೆ))
    ಹಿಂತಿರುಗಿ 0;

  ತಾರ್ಕಿಕ isNegative = *p == '-';
  ಆದರೆ (*p == '-' || *p == '+') {
    p++;
    ಆದರೆ (p == ಕೊನೆ)
      ಹಿಂತಿರುಗಿ createError("Exponent has no digits");
  }

  ಚಿಹ್ನೆರಹಿತ absExponent = dec_Digit_ಮೌಲ್ಯ(*p++);
  ಆದರೆ (absExponent >= 10U)
    ಹಿಂತಿರುಗಿ createError("Invalid character in exponent");

  ಗಾಗಿ (; p != ಕೊನೆ; ++p) {
    ಚಿಹ್ನೆರಹಿತ ಮೌಲ್ಯ_2 = dec_Digit_ಮೌಲ್ಯ(*p);
    ಆದರೆ (ಮೌಲ್ಯ_2 >= 10U)
      ಹಿಂತಿರುಗಿ createError("Invalid character in exponent");

    absExponent = absExponent * 10U + ಮೌಲ್ಯ_2;
    ಆದರೆ (absExponent >= overlargeExponent) {
      absExponent = overlargeExponent;
      ನಿಲ್ಲಿಸು;
    }
  }

  ಆದರೆ (isNegative)
    ಹಿಂತಿರುಗಿ -(ಪೂರ್ಣಾಂಕ) absExponent;
  ಇಲ್ಲದಿದ್ದರೆ
    ಹಿಂತಿರುಗಿ (ಪೂರ್ಣಾಂಕ) absExponent;
}

/* This is ugly and needs cleaning up, but I don't immediately see
   how whilst remaining safe.  */
ಸ್ಥಿರ Expected<ಪೂರ್ಣಾಂಕ> ಒಟ್ಟು_Exponent(StringRef::iterator p,
                                   StringRef::iterator ಕೊನೆ,
                                   ಪೂರ್ಣಾಂಕ exponentAdjustment) {
  ಪೂರ್ಣಾಂಕ exponent = 0;

  ಆದರೆ (p == ಕೊನೆ)
    ಹಿಂತಿರುಗಿ createError("Exponent has no digits");

  ತಾರ್ಕಿಕ negative = *p == '-';
  ಆದರೆ (*p == '-' || *p == '+') {
    p++;
    ಆದರೆ (p == ಕೊನೆ)
      ಹಿಂತಿರುಗಿ createError("Exponent has no digits");
  }

  ಪೂರ್ಣಾಂಕ unsignedExponent = 0;
  ತಾರ್ಕಿಕ ಅತಿಪ್ರವಾಹ_2 = ಸುಳ್ಳು;
  ಗಾಗಿ (; p != ಕೊನೆ; ++p) {
    ಚಿಹ್ನೆರಹಿತ ಪೂರ್ಣಾಂಕ ಮೌಲ್ಯ_2;

    ಮೌಲ್ಯ_2 = dec_Digit_ಮೌಲ್ಯ(*p);
    ಆದರೆ (ಮೌಲ್ಯ_2 >= 10U)
      ಹಿಂತಿರುಗಿ createError("Invalid character in exponent");

    unsignedExponent = unsignedExponent * 10 + ಮೌಲ್ಯ_2;
    ಆದರೆ (unsignedExponent > 32767) {
      ಅತಿಪ್ರವಾಹ_2 = ನಿಜ;
      ನಿಲ್ಲಿಸು;
    }
  }

  ಆದರೆ (exponentAdjustment > 32767 || exponentAdjustment < -32768)
    ಅತಿಪ್ರವಾಹ_2 = ನಿಜ;

  ಆದರೆ (!ಅತಿಪ್ರವಾಹ_2) {
    exponent = unsignedExponent;
    ಆದರೆ (negative)
      exponent = -exponent;
    exponent += exponentAdjustment;
    ಆದರೆ (exponent > 32767 || exponent < -32768)
      ಅತಿಪ್ರವಾಹ_2 = ನಿಜ;
  }

  ಆದರೆ (ಅತಿಪ್ರವಾಹ_2)
    exponent = negative ? -32768: 32767;

  ಹಿಂತಿರುಗಿ exponent;
}

ಸ್ಥಿರ Expected<StringRef::iterator>
skipLeadingZeroesAndAnyDot(StringRef::iterator begin, StringRef::iterator ಕೊನೆ,
                           StringRef::iterator *dot) {
  StringRef::iterator p = begin;
  *dot = ಕೊನೆ;
  ಆಗ (p != ಕೊನೆ && *p == '0')
    p++;

  ಆದರೆ (p != ಕೊನೆ && *p == '.') {
    *dot = p++;

    ಆದರೆ (ಕೊನೆ - begin == 1)
      ಹಿಂತಿರುಗಿ createError("Significand has no digits");

    ಆಗ (p != ಕೊನೆ && *p == '0')
      p++;
  }

  ಹಿಂತಿರುಗಿ p;
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
ರಚನೆ decimalInfo {
  ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *ಮೊದಲ_Sig_Digit;
  ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *ಕೊನೆಯ_Sig_Digit;
  ಪೂರ್ಣಾಂಕ exponent;
  ಪೂರ್ಣಾಂಕ normalizedExponent;
};

ಸ್ಥಿರ Error interpretDecimal(StringRef::iterator begin,
                              StringRef::iterator ಕೊನೆ, decimalInfo *D) {
  StringRef::iterator dot = ಕೊನೆ;

  ಸ್ವಯಂಚಾಲಿತ PtrOrErr = skipLeadingZeroesAndAnyDot(begin, ಕೊನೆ, &dot);
  ಆದರೆ (!PtrOrErr)
    ಹಿಂತಿರುಗಿ PtrOrErr.takeError();
  StringRef::iterator p = *PtrOrErr;

  D->ಮೊದಲ_Sig_Digit = p;
  D->exponent = 0;
  D->normalizedExponent = 0;

  ಗಾಗಿ (; p != ಕೊನೆ; ++p) {
    ಆದರೆ (*p == '.') {
      ಆದರೆ (dot != ಕೊನೆ)
        ಹಿಂತಿರುಗಿ createError("String contains multiple dots");
      dot = p++;
      ಆದರೆ (p == ಕೊನೆ)
        ನಿಲ್ಲಿಸು;
    }
    ಆದರೆ (dec_Digit_ಮೌಲ್ಯ(*p) >= 10U)
      ನಿಲ್ಲಿಸು;
  }

  ಆದರೆ (p != ಕೊನೆ) {
    ಆದರೆ (*p != 'e' && *p != 'E')
      ಹಿಂತಿರುಗಿ createError("Invalid character in significand");
    ಆದರೆ (p == begin)
      ಹಿಂತಿರುಗಿ createError("Significand has no digits");
    ಆದರೆ (dot != ಕೊನೆ && p - begin == 1)
      ಹಿಂತಿರುಗಿ createError("Significand has no digits");

    /* p points to the first non-digit in the string */
    ಸ್ವಯಂಚಾಲಿತ ExpOrErr = readExponent(p + 1, ಕೊನೆ);
    ಆದರೆ (!ExpOrErr)
      ಹಿಂತಿರುಗಿ ExpOrErr.takeError();
    D->exponent = *ExpOrErr;

    /* Implied decimal point?  */
    ಆದರೆ (dot == ಕೊನೆ)
      dot = p;
  }

  /* If number is all zeroes accept any exponent.  */
  ಆದರೆ (p != D->ಮೊದಲ_Sig_Digit) {
    /* Drop insignificant trailing zeroes.  */
    ಆದರೆ (p != begin) {
      ಮಾಡು
        ಮಾಡು
          p--;
        ಆಗ (p != begin && *p == '0');
      ಆಗ (p != begin && *p == '.');
    }

    /* Adjust the exponents for any decimal point.  */
    D->exponent += static_cast<APFloat::ExponentType>((dot - p) - (dot > p));
    D->normalizedExponent = (D->exponent +
              static_cast<APFloat::ExponentType>((p - D->ಮೊದಲ_Sig_Digit)
                                      - (dot > D->ಮೊದಲ_Sig_Digit && dot < p)));
  }

  D->ಕೊನೆಯ_Sig_Digit = p;
  ಹಿಂತಿರುಗಿ Error::success();
}

/* Return the trailing fraction of a hexadecimal number.
   DIGITVALUE is the first hex digit of the fraction, P points to
   the next digit.  */
ಸ್ಥಿರ Expected<ಕಳೆದುಹೋದ_Fraction>
trailingHexadecimalFraction(StringRef::iterator p, StringRef::iterator ಕೊನೆ,
                            ಚಿಹ್ನೆರಹಿತ ಪೂರ್ಣಾಂಕ digit_ಮೌಲ್ಯ) {
  /* If the first trailing digit isn't 0 or 8 we can work out the
     fraction immediately.  */
  ಆದರೆ (digit_ಮೌಲ್ಯ > 8)
    ಹಿಂತಿರುಗಿ lf_ಇನ್ನಷ್ಟು_Than_Half;
  ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (digit_ಮೌಲ್ಯ < 8 && digit_ಮೌಲ್ಯ > 0)
    ಹಿಂತಿರುಗಿ lfLessThanHalf;

  // Otherwise we need to find the first non-zero digit.
  ಆಗ (p != ಕೊನೆ && (*p == '0' || *p == '.'))
    p++;

  ಆದರೆ (p == ಕೊನೆ)
    ಹಿಂತಿರುಗಿ createError("Invalid trailing hexadecimal fraction!");

  ಚಿಹ್ನೆರಹಿತ hexDigit = hex_Digit_ಮೌಲ್ಯ(*p);

  /* If we ran off the end it is exactly zero or one-half, otherwise
     a little more.  */
  ಆದರೆ (hexDigit == UINT_MAX)
    ಹಿಂತಿರುಗಿ digit_ಮೌಲ್ಯ == 0 ? lfExactlyZero: lfExactlyHalf;
  ಇಲ್ಲದಿದ್ದರೆ
    ಹಿಂತಿರುಗಿ digit_ಮೌಲ್ಯ == 0 ? lfLessThanHalf: lf_ಇನ್ನಷ್ಟು_Than_Half;
}

/* Return the fraction lost were a bignum truncated losing the least
   significant BITS bits.  */
ಸ್ಥಿರ ಕಳೆದುಹೋದ_Fraction
ಕಳೆದುಹೋದ_Fraction_Through_Truncation(ಸ್ಥಿರಾಂಕ APFloat_ತಳ::integerPart *parts,
                              ಚಿಹ್ನೆರಹಿತ ಪೂರ್ಣಾಂಕ part_ಎಣಿಕೆ,
                              ಚಿಹ್ನೆರಹಿತ ಪೂರ್ಣಾಂಕ ಬಿಟ್ಟುಗಳು)
{
  ಚಿಹ್ನೆರಹಿತ lsb = APInt::tcLSB(parts, part_ಎಣಿಕೆ);

  /* Note this is guaranteed true if bits == 0, or LSB == UINT_MAX.  */
  ಆದರೆ (ಬಿಟ್ಟುಗಳು <= lsb)
    ಹಿಂತಿರುಗಿ lfExactlyZero;
  ಆದರೆ (ಬಿಟ್ಟುಗಳು == lsb + 1)
    ಹಿಂತಿರುಗಿ lfExactlyHalf;
  ಆದರೆ (ಬಿಟ್ಟುಗಳು <= part_ಎಣಿಕೆ * APFloat_ತಳ::integer_Part_ಅಗಲ &&
      APInt::tc_ಹೊರತೆಗೆ_ಬಿಟ್(parts, ಬಿಟ್ಟುಗಳು - 1))
    ಹಿಂತಿರುಗಿ lf_ಇನ್ನಷ್ಟು_Than_Half;

  ಹಿಂತಿರುಗಿ lfLessThanHalf;
}

/* Shift DST right BITS bits noting lost fraction.  */
ಸ್ಥಿರ ಕಳೆದುಹೋದ_Fraction
ಸರಿಸು_ಬಲ(APFloat_ತಳ::integerPart *dst, ಚಿಹ್ನೆರಹಿತ ಪೂರ್ಣಾಂಕ parts, ಚಿಹ್ನೆರಹಿತ ಪೂರ್ಣಾಂಕ ಬಿಟ್ಟುಗಳು)
{
  ಕಳೆದುಹೋದ_Fraction ಕಳೆದುಹೋದ_fraction = ಕಳೆದುಹೋದ_Fraction_Through_Truncation(dst, parts, ಬಿಟ್ಟುಗಳು);

  APInt::tc_ಸರಿಸು_ಬಲ(dst, parts, ಬಿಟ್ಟುಗಳು);

  ಹಿಂತಿರುಗಿ ಕಳೆದುಹೋದ_fraction;
}

/* Combine the effect of two lost fractions.  */
ಸ್ಥಿರ ಕಳೆದುಹೋದ_Fraction
combine_ಕಳೆದುಹೋದ_Fractions(ಕಳೆದುಹೋದ_Fraction ಇನ್ನಷ್ಟು_Significant,
                     ಕಳೆದುಹೋದ_Fraction lessSignificant)
{
  ಆದರೆ (lessSignificant != lfExactlyZero) {
    ಆದರೆ (ಇನ್ನಷ್ಟು_Significant == lfExactlyZero)
      ಇನ್ನಷ್ಟು_Significant = lfLessThanHalf;
    ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (ಇನ್ನಷ್ಟು_Significant == lfExactlyHalf)
      ಇನ್ನಷ್ಟು_Significant = lf_ಇನ್ನಷ್ಟು_Than_Half;
  }

  ಹಿಂತಿರುಗಿ ಇನ್ನಷ್ಟು_Significant;
}

/* The error from the true value, in half-ulps, on multiplying two
   floating point numbers, which differ from the value they
   approximate by at most HUE1 and HUE2 half-ulps, is strictly less
   than the returned value.

   See "How to Read Floating Point Numbers Accurately" by William D
   Clinger.  */
ಸ್ಥಿರ ಚಿಹ್ನೆರಹಿತ ಪೂರ್ಣಾಂಕ
HUerrBound(ತಾರ್ಕಿಕ inexactMultiply, ಚಿಹ್ನೆರಹಿತ ಪೂರ್ಣಾಂಕ HUerr1, ಚಿಹ್ನೆರಹಿತ ಪೂರ್ಣಾಂಕ HUerr2)
{
  ಪ್ರತಿಜ್ಞಾ(HUerr1 < 2 || HUerr2 < 2 || (HUerr1 + HUerr2 < 8));

  ಆದರೆ (HUerr1 + HUerr2 == 0)
    ಹಿಂತಿರುಗಿ inexactMultiply * 2;  /* <= inexactMultiply half-ulps.  */
  ಇಲ್ಲದಿದ್ದರೆ
    ಹಿಂತಿರುಗಿ inexactMultiply + 2 * (HUerr1 + HUerr2);
}

/* The number of ulps from the boundary (zero, or half if ISNEAREST)
   when the least significant BITS are truncated.  BITS cannot be
   zero.  */
ಸ್ಥಿರ APFloat_ತಳ::integerPart
ulpsFromBoundary(ಸ್ಥಿರಾಂಕ APFloat_ತಳ::integerPart *parts, ಚಿಹ್ನೆರಹಿತ ಪೂರ್ಣಾಂಕ ಬಿಟ್ಟುಗಳು,
                 ತಾರ್ಕಿಕ is_ಹತ್ತಿರದ) {
  ಪ್ರತಿಜ್ಞಾ(ಬಿಟ್ಟುಗಳು != 0);

  ಬಿಟ್ಟುಗಳು--;
  ಚಿಹ್ನೆರಹಿತ ಎಣಿಕೆ_2 = ಬಿಟ್ಟುಗಳು / APFloat_ತಳ::integer_Part_ಅಗಲ;
  ಚಿಹ್ನೆರಹಿತ part_ಬಿಟ್ಟುಗಳು = ಬಿಟ್ಟುಗಳು % APFloat_ತಳ::integer_Part_ಅಗಲ + 1;

  APFloat_ತಳ::integerPart part =
      parts[ಎಣಿಕೆ_2] & (~(APFloat_ತಳ::integerPart)0 >>
                      (APFloat_ತಳ::integer_Part_ಅಗಲ - part_ಬಿಟ್ಟುಗಳು));

  APFloat_ತಳ::integerPart boundary;
  ಆದರೆ (is_ಹತ್ತಿರದ)
    boundary = (APFloat_ತಳ::integerPart) 1 << (part_ಬಿಟ್ಟುಗಳು - 1);
  ಇಲ್ಲದಿದ್ದರೆ
    boundary = 0;

  ಆದರೆ (ಎಣಿಕೆ_2 == 0) {
    ಆದರೆ (part - boundary <= boundary - part)
      ಹಿಂತಿರುಗಿ part - boundary;
    ಇಲ್ಲದಿದ್ದರೆ
      ಹಿಂತಿರುಗಿ boundary - part;
  }

  ಆದರೆ (part == boundary) {
    ಆಗ (--ಎಣಿಕೆ_2)
      ಆದರೆ (parts[ಎಣಿಕೆ_2])
        ಹಿಂತಿರುಗಿ ~(APFloat_ತಳ::integerPart) 0; /* A lot.  */

    ಹಿಂತಿರುಗಿ parts[0];
  } ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (part == boundary - 1) {
    ಆಗ (--ಎಣಿಕೆ_2)
      ಆದರೆ (~parts[ಎಣಿಕೆ_2])
        ಹಿಂತಿರುಗಿ ~(APFloat_ತಳ::integerPart) 0; /* A lot.  */

    ಹಿಂತಿರುಗಿ -parts[0];
  }

  ಹಿಂತಿರುಗಿ ~(APFloat_ತಳ::integerPart) 0; /* A lot.  */
}

ಸ್ಥಿರ constexpr ಚಿಹ್ನೆರಹಿತ Power_ದ_Five_Part_Counts[] = {1,  1,  2,  3,   5,  10,
                                                     19, 38, 75, 149, 298};

ಸ್ಥಿರ constexpr APFloat_ತಳ::integerPart Power_ದ_Five_Parts[] = {
#ಅನ್ತರ್ಭಾವ "APFloatPowerOfFiveTable.inc"
};

static_assert([] {
  ಚಿಹ್ನೆರಹಿತ ಮೊತ್ತ = 0;
  ಗಾಗಿ (ಚಿಹ್ನೆರಹಿತ ಎಣಿಕೆ_3 : Power_ದ_Five_Part_Counts)
    ಮೊತ್ತ += ಎಣಿಕೆ_3;
  ಹಿಂತಿರುಗಿ ಮೊತ್ತ;
}() == std::ಗಾತ್ರ_3(Power_ದ_Five_Parts));

/* Place pow(5, power) in DST, and return the number of parts used.
   DST must be at least one part larger than size of the answer.  */
ಸ್ಥಿರ ಚಿಹ್ನೆರಹಿತ ಪೂರ್ಣಾಂಕ
powerOf5(APFloat_ತಳ::integerPart *dst, ಚಿಹ್ನೆರಹಿತ ಪೂರ್ಣಾಂಕ power) {
  ಸ್ಥಿರ ಸ್ಥಿರಾಂಕ APFloat_ತಳ::integerPart ಮೊದಲ_Eight_Powers[] = {
      1, 5, 25, 125, 625, 3125, 15625, 78125};

  Small_ಸದಿಶ<APFloat_ತಳ::integerPart, ಅಂಕೆ_Pow5_Parts_On_ರಾಶಿ> scratch;
  scratch.resize_for_overwrite(power_ದ_Five_Parts_For_Exponent(power));

  APFloat_ತಳ::integerPart *p1, *p2;
  ಸ್ಥಿರಾಂಕ APFloat_ತಳ::integerPart *pow5;

  p1 = dst;
  p2 = scratch.ದತ್ತಾಂಶ();

  *p1 = ಮೊದಲ_Eight_Powers[power & 7];
  power >>= 3;

  ಚಿಹ್ನೆರಹಿತ ಫಲಿತಾಂಶ_2 = 1;
  pow5 = Power_ದ_Five_Parts;

  ಗಾಗಿ (ಚಿಹ್ನೆರಹಿತ ಪೂರ್ಣಾಂಕ n = 0; power; power >>= 1, n++) {
    ಚಿಹ್ನೆರಹಿತ parts_ಎಣಿಕೆ = Power_ದ_Five_Part_Counts[n];

    ಆದರೆ (power & 1) {
      APFloat_ತಳ::integerPart *tmp;

      APInt::tc_ತುಂಬಿದ_Multiply(p2, p1, pow5, ಫಲಿತಾಂಶ_2, parts_ಎಣಿಕೆ);
      ಫಲಿತಾಂಶ_2 += parts_ಎಣಿಕೆ;
      ಆದರೆ (p2[ಫಲಿತಾಂಶ_2 - 1] == 0)
        ಫಲಿತಾಂಶ_2--;

      /* Now result is in p1 with partsCount parts and p2 is scratch
         space.  */
      tmp = p1;
      p1 = p2;
      p2 = tmp;
    }

    pow5 += parts_ಎಣಿಕೆ;
  }

  ಆದರೆ (p1 != dst)
    APInt::tc_ನಿಯೋಜಿಸು(dst, p1, ಫಲಿತಾಂಶ_2);

  ಹಿಂತಿರುಗಿ ಫಲಿತಾಂಶ_2;
}

/* Zero at the end to avoid modular arithmetic when adding one; used
   when rounding up during hexadecimal output.  */
ಸ್ಥಿರ ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ hex_Digits_ಅಧೋ[] = "0123456789abcdef0";
ಸ್ಥಿರ ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ hex_Digits_ಊರ್ಧ್ವ[] = "0123456789ABCDEF0";
ಸ್ಥಿರ ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ infinityL[] = "infinity";
ಸ್ಥಿರ ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ infinityU[] = "INFINITY";
ಸ್ಥಿರ ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ NaNL[] = "nan";
ಸ್ಥಿರ ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ NaNU[] = "NAN";

/* Write out an integerPart in hexadecimal, starting with the most
   significant nibble.  Write out exactly COUNT hexdigits, return
   COUNT.  */
ಸ್ಥಿರ ಚಿಹ್ನೆರಹಿತ ಪೂರ್ಣಾಂಕ
partAsHex (ಅಕ್ಷರ *dst, APFloat_ತಳ::integerPart part, ಚಿಹ್ನೆರಹಿತ ಪೂರ್ಣಾಂಕ ಎಣಿಕೆ_2,
           ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *hexDigitChars)
{
  ಚಿಹ್ನೆರಹಿತ ಪೂರ್ಣಾಂಕ ಫಲಿತಾಂಶ_2 = ಎಣಿಕೆ_2;

  ಪ್ರತಿಜ್ಞಾ(ಎಣಿಕೆ_2 != 0 && ಎಣಿಕೆ_2 <= APFloat_ತಳ::integer_Part_ಅಗಲ / 4);

  part >>= (APFloat_ತಳ::integer_Part_ಅಗಲ - 4 * ಎಣಿಕೆ_2);
  ಆಗ (ಎಣಿಕೆ_2--) {
    dst[ಎಣಿಕೆ_2] = hexDigitChars[part & 0xf];
    part >>= 4;
  }

  ಹಿಂತಿರುಗಿ ಫಲಿತಾಂಶ_2;
}

/* Write out an unsigned decimal integer.  */
ಸ್ಥಿರ ಅಕ್ಷರ *writeUnsignedDecimal(ಅಕ್ಷರ *dst, ಚಿಹ್ನೆರಹಿತ ಪೂರ್ಣಾಂಕ n) {
  ಅಕ್ಷರ buff[40], *p;

  p = buff;
  ಮಾಡು
    *p++ = '0' + n % 10;
  ಆಗ (n /= 10);

  ಮಾಡು
    *dst++ = *--p;
  ಆಗ (p != buff);

  ಹಿಂತಿರುಗಿ dst;
}

/* Write out a signed decimal integer.  */
ಸ್ಥಿರ ಅಕ್ಷರ *writeSignedDecimal(ಅಕ್ಷರ *dst, ಪೂರ್ಣಾಂಕ ಮೌಲ್ಯ_2) {
  ಆದರೆ (ಮೌಲ್ಯ_2 < 0) {
    *dst++ = '-';
    dst = writeUnsignedDecimal(dst, -(ಚಿಹ್ನೆರಹಿತ) ಮೌಲ್ಯ_2);
  } ಇಲ್ಲದಿದ್ದರೆ {
    dst = writeUnsignedDecimal(dst, ಮೌಲ್ಯ_2);
  }

  ಹಿಂತಿರುಗಿ dst;
}

// Compute the ULP of the input using a definition from:
// Jean-Michel Muller. On the definition of ulp(x). [Research Report] RR-5504,
// LIP RR-2005-09, INRIA, LIP. 2005, pp.16. inria-00070503
ಸ್ಥಿರ APFloat harrisonUlp(ಸ್ಥಿರಾಂಕ APFloat &X) {
  ಸ್ಥಿರಾಂಕ fltSemantics &Sem = X.getSemantics();
  ಆಯ್ಕೆ (X.getCategory()) {
  ಸ್ಥಿತಿ APFloat::fcNaN:
    ಹಿಂತಿರುಗಿ APFloat::getQNaN(Sem);
  ಸ್ಥಿತಿ APFloat::fcInfinity:
    ಹಿಂತಿರುಗಿ APFloat::getInf(Sem);
  ಸ್ಥಿತಿ APFloat::fcZero:
    ಹಿಂತಿರುಗಿ APFloat::get_ಅತಿಚಿಕ್ಕ(Sem);
  ಸ್ಥಿತಿ APFloat::fc_ಸಾಮಾನ್ಯ:
    ನಿಲ್ಲಿಸು;
  }
  ಆದರೆ (X.isDenormal() || X.is_ಅತಿಚಿಕ್ಕ_Normalized())
    ಹಿಂತಿರುಗಿ APFloat::get_ಅತಿಚಿಕ್ಕ(Sem);
  ಪೂರ್ಣಾಂಕ Exp = ilogb(X);
  ಆದರೆ (X.getExactLog2() != INT_MIN)
    Exp -= 1;
  ಹಿಂತಿರುಗಿ scalbn(APFloat::getOne(Sem), Exp - (Sem.precision - 1),
                APFloat::rm_ಹತ್ತಿರದ_Ties_ಗಮ್ಯ_Even);
}

ನಾಮಾಕಾಶ detail {
/* Constructors.  */
ಶೂನ್ಯ IEEEFloat::initialize(ಸ್ಥಿರಾಂಕ fltSemantics *ourSemantics) {
  semantics = ourSemantics;
  ಚಿಹ್ನೆರಹಿತ ಎಣಿಕೆ_2 = part_ಎಣಿಕೆ();
  ಆದರೆ (ಎಣಿಕೆ_2 > 1)
    significand.parts = ಹೊಸ integerPart[ಎಣಿಕೆ_2];
}

ಶೂನ್ಯ IEEEFloat::freeSignificand() {
  ಆದರೆ (needsCleanup())
    ಅಳಿಸು [] significand.parts;
}

ಶೂನ್ಯ IEEEFloat::ನಿಯೋಜಿಸು(ಸ್ಥಿರಾಂಕ IEEEFloat &rhs) {
  ಪ್ರತಿಜ್ಞಾ(semantics == rhs.semantics);

  sign = rhs.sign;
  category = rhs.category;
  exponent = rhs.exponent;
  ಆದರೆ (isFiniteNonZero() || category == fcNaN)
    copySignificand(rhs);
}

ಶೂನ್ಯ IEEEFloat::copySignificand(ಸ್ಥಿರಾಂಕ IEEEFloat &rhs) {
  ಪ್ರತಿಜ್ಞಾ(isFiniteNonZero() || category == fcNaN);
  ಪ್ರತಿಜ್ಞಾ(rhs.part_ಎಣಿಕೆ() >= part_ಎಣಿಕೆ());

  APInt::tc_ನಿಯೋಜಿಸು(significandParts(), rhs.significandParts(),
                  part_ಎಣಿಕೆ());
}

/* Make this number a NaN, with an arbitrary but deterministic value
   for the significand.  If double or longer, this is a signalling NaN,
   which may not be ideal.  If float, this is QNaN(0).  */
ಶೂನ್ಯ IEEEFloat::makeNaN(ತಾರ್ಕಿಕ SNaN, ತಾರ್ಕಿಕ Negative, ಸ್ಥಿರಾಂಕ APInt *fill) {
  ಆದರೆ (semantics->nonFiniteBehavior == fltNonfiniteBehavior::FiniteOnly)
    llvm_unreachable("This floating point format does not support NaN");

  ಆದರೆ (Negative && !semantics->hasSignedRepr)
    llvm_unreachable(
        "This floating point format does not support signed values");

  category = fcNaN;
  sign = Negative;
  exponent = exponentNaN();

  integerPart *significand = significandParts();
  ಚಿಹ್ನೆರಹಿತ ಅಂಕೆ_Parts = part_ಎಣಿಕೆ();

  APInt fill_storage;
  ಆದರೆ (semantics->nonFiniteBehavior == fltNonfiniteBehavior::NanOnly) {
    // Finite-only types do not distinguish signalling and quiet NaN, so
    // make them all signalling.
    SNaN = ಸುಳ್ಳು;
    ಆದರೆ (semantics->nanEncoding == fltNanEncoding::NegativeZero) {
      sign = ನಿಜ;
      fill_storage = APInt::getZero(semantics->precision - 1);
    } ಇಲ್ಲದಿದ್ದರೆ {
      fill_storage = APInt::getAllOnes(semantics->precision - 1);
    }
    fill = &fill_storage;
  }

  // Set the significand bits to the fill.
  ಆದರೆ (!fill || fill->get_ಅಂಕೆ_ಪದಗಳು() < ಅಂಕೆ_Parts)
    APInt::tc_ಗಣ(significand, 0, ಅಂಕೆ_Parts);
  ಆದರೆ (fill) {
    APInt::tc_ನಿಯೋಜಿಸು(significand, fill->get_Raw_ದತ್ತಾಂಶ(),
                    std::min(fill->get_ಅಂಕೆ_ಪದಗಳು(), ಅಂಕೆ_Parts));

    // Zero out the excess bits of the significand.
    ಚಿಹ್ನೆರಹಿತ ಬಿಟ್ಟುಗಳು_ಗಮ್ಯ_Preserve = semantics->precision - 1;
    ಚಿಹ್ನೆರಹಿತ part = ಬಿಟ್ಟುಗಳು_ಗಮ್ಯ_Preserve / 64;
    ಬಿಟ್ಟುಗಳು_ಗಮ್ಯ_Preserve %= 64;
    significand[part] &= ((1ULL << ಬಿಟ್ಟುಗಳು_ಗಮ್ಯ_Preserve) - 1);
    ಗಾಗಿ (part++; part != ಅಂಕೆ_Parts; ++part)
      significand[part] = 0;
  }

  ಚಿಹ್ನೆರಹಿತ QNaNBit =
      (semantics->precision >= 2) ? (semantics->precision - 2) : 0;

  ಆದರೆ (SNaN) {
    // We always have to clear the QNaN bit to make it an SNaN.
    APInt::tc_Clear_ಬಿಟ್(significand, QNaNBit);

    // If there are no bits set in the payload, we have to set
    // *something* to make it a NaN instead of an infinity;
    // conventionally, this is the next bit down from the QNaN bit.
    ಆದರೆ (APInt::tcIsZero(significand, ಅಂಕೆ_Parts))
      APInt::tc_ಗಣ_ಬಿಟ್(significand, QNaNBit - 1);
  } ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (semantics->nanEncoding == fltNanEncoding::NegativeZero) {
    // The only NaN is a quiet NaN, and it has no bits sets in the significand.
    // Do nothing.
  } ಇಲ್ಲದಿದ್ದರೆ {
    // We always have to set the QNaN bit to make it a QNaN.
    APInt::tc_ಗಣ_ಬಿಟ್(significand, QNaNBit);
  }

  // For x87 extended precision, we want to make a NaN, not a
  // pseudo-NaN.  Maybe we should expose the ability to make
  // pseudo-NaNs?
  ಆದರೆ (semantics == &APFloat_ತಳ::semX87DoubleExtended)
    APInt::tc_ಗಣ_ಬಿಟ್(significand, QNaNBit + 1);
}

IEEEFloat &IEEEFloat::ಕ್ರಿಯಾಕಾರಕ=(ಸ್ಥಿರಾಂಕ IEEEFloat &rhs) {
  ಆದರೆ (ಇದು != &rhs) {
    ಆದರೆ (semantics != rhs.semantics) {
      freeSignificand();
      initialize(rhs.semantics);
    }
    ನಿಯೋಜಿಸು(rhs);
  }

  ಹಿಂತಿರುಗಿ *ಇದು;
}

IEEEFloat &IEEEFloat::ಕ್ರಿಯಾಕಾರಕ=(IEEEFloat &&rhs) {
  freeSignificand();

  semantics = rhs.semantics;
  significand = rhs.significand;
  exponent = rhs.exponent;
  category = rhs.category;
  sign = rhs.sign;

  rhs.semantics = &APFloat_ತಳ::semBogus;
  ಹಿಂತಿರುಗಿ *ಇದು;
}

ತಾರ್ಕಿಕ IEEEFloat::isDenormal() ಸ್ಥಿರಾಂಕ {
  ಹಿಂತಿರುಗಿ getSemantics().hasDenormals && isFiniteNonZero() &&
         (exponent == semantics->minExponent) &&
         (APInt::tc_ಹೊರತೆಗೆ_ಬಿಟ್(significandParts(), semantics->precision - 1) ==
          0);
}

ತಾರ್ಕಿಕ IEEEFloat::is_ಅತಿಚಿಕ್ಕ() ಸ್ಥಿರಾಂಕ {
  // The smallest number by magnitude in our format will be the smallest
  // denormal, i.e. the floating point number with exponent being minimum
  // exponent and significand bitwise equal to 1 (i.e. with MSB equal to 0).
  ಹಿಂತಿರುಗಿ isFiniteNonZero() && exponent == semantics->minExponent &&
    significandMSB() == 0;
}

ತಾರ್ಕಿಕ IEEEFloat::is_ಅತಿಚಿಕ್ಕ_Normalized() ಸ್ಥಿರಾಂಕ {
  ಹಿಂತಿರುಗಿ getCategory() == fc_ಸಾಮಾನ್ಯ && exponent == semantics->minExponent &&
         isSignificandAllZerosExceptMSB();
}

ಚಿಹ್ನೆರಹಿತ ಪೂರ್ಣಾಂಕ IEEEFloat::get_ಅಂಕೆ_ಮೇಲಿನ_ಬಿಟ್ಟುಗಳು() ಸ್ಥಿರಾಂಕ {
  ಸ್ಥಿರಾಂಕ ಚಿಹ್ನೆರಹಿತ ಪೂರ್ಣಾಂಕ Part_ಎಣಿಕೆ = part_ಎಣಿಕೆ_For_ಬಿಟ್ಟುಗಳು(semantics->precision);
  ಸ್ಥಿರಾಂಕ ಚಿಹ್ನೆರಹಿತ ಪೂರ್ಣಾಂಕ ಬಿಟ್ಟುಗಳು_2 = Part_ಎಣಿಕೆ * integer_Part_ಅಗಲ;

  // Compute how many bits are used in the final word.
  // When precision is just 1, it represents the 'Pth'
  // Precision bit and not the actual significand bit.
  ಸ್ಥಿರಾಂಕ ಚಿಹ್ನೆರಹಿತ ಪೂರ್ಣಾಂಕ ಅಂಕೆ_ಮೇಲಿನ_ಬಿಟ್ಟುಗಳು = (semantics->precision > 1)
                                       ? (ಬಿಟ್ಟುಗಳು_2 - semantics->precision + 1)
                                       : (ಬಿಟ್ಟುಗಳು_2 - semantics->precision);
  ಹಿಂತಿರುಗಿ ಅಂಕೆ_ಮೇಲಿನ_ಬಿಟ್ಟುಗಳು;
}

ತಾರ್ಕಿಕ IEEEFloat::isSignificandAllOnes() ಸ್ಥಿರಾಂಕ {
  // Test if the significand excluding the integral bit is all ones. This allows
  // us to test for binade boundaries.
  ಸ್ಥಿರಾಂಕ integerPart *Parts = significandParts();
  ಸ್ಥಿರಾಂಕ ಚಿಹ್ನೆರಹಿತ Part_ಎಣಿಕೆ = part_ಎಣಿಕೆ_For_ಬಿಟ್ಟುಗಳು(semantics->precision);
  ಗಾಗಿ (ಚಿಹ್ನೆರಹಿತ i = 0; i < Part_ಎಣಿಕೆ - 1; i++)
    ಆದರೆ (~Parts[i])
      ಹಿಂತಿರುಗಿ ಸುಳ್ಳು;

  // Set the unused high bits to all ones when we compare.
  ಸ್ಥಿರಾಂಕ ಚಿಹ್ನೆರಹಿತ ಅಂಕೆ_ಮೇಲಿನ_ಬಿಟ್ಟುಗಳು = get_ಅಂಕೆ_ಮೇಲಿನ_ಬಿಟ್ಟುಗಳು();
  ಪ್ರತಿಜ್ಞಾ(ಅಂಕೆ_ಮೇಲಿನ_ಬಿಟ್ಟುಗಳು <= integer_Part_ಅಗಲ && ಅಂಕೆ_ಮೇಲಿನ_ಬಿಟ್ಟುಗಳು > 0 &&
         "Can not have more high bits to fill than integerPartWidth");
  ಸ್ಥಿರಾಂಕ integerPart ಮೇಲಿನ_ಬಿಟ್_Fill =
    ~integerPart(0) << (integer_Part_ಅಗಲ - ಅಂಕೆ_ಮೇಲಿನ_ಬಿಟ್ಟುಗಳು);
  ಆದರೆ ((semantics->precision <= 1) || (~(Parts[Part_ಎಣಿಕೆ - 1] | ಮೇಲಿನ_ಬಿಟ್_Fill)))
    ಹಿಂತಿರುಗಿ ಸುಳ್ಳು;

  ಹಿಂತಿರುಗಿ ನಿಜ;
}

ತಾರ್ಕಿಕ IEEEFloat::isSignificandAllOnesExceptLSB() ಸ್ಥಿರಾಂಕ {
  // Test if the significand excluding the integral bit is all ones except for
  // the least significant bit.
  ಸ್ಥಿರಾಂಕ integerPart *Parts = significandParts();

  ಆದರೆ (Parts[0] & 1)
    ಹಿಂತಿರುಗಿ ಸುಳ್ಳು;

  ಸ್ಥಿರಾಂಕ ಚಿಹ್ನೆರಹಿತ Part_ಎಣಿಕೆ = part_ಎಣಿಕೆ_For_ಬಿಟ್ಟುಗಳು(semantics->precision);
  ಗಾಗಿ (ಚಿಹ್ನೆರಹಿತ i = 0; i < Part_ಎಣಿಕೆ - 1; i++) {
    ಆದರೆ (~Parts[i] & ~ಚಿಹ್ನೆರಹಿತ{!i})
      ಹಿಂತಿರುಗಿ ಸುಳ್ಳು;
  }

  // Set the unused high bits to all ones when we compare.
  ಸ್ಥಿರಾಂಕ ಚಿಹ್ನೆರಹಿತ ಅಂಕೆ_ಮೇಲಿನ_ಬಿಟ್ಟುಗಳು = get_ಅಂಕೆ_ಮೇಲಿನ_ಬಿಟ್ಟುಗಳು();
  ಪ್ರತಿಜ್ಞಾ(ಅಂಕೆ_ಮೇಲಿನ_ಬಿಟ್ಟುಗಳು <= integer_Part_ಅಗಲ && ಅಂಕೆ_ಮೇಲಿನ_ಬಿಟ್ಟುಗಳು > 0 &&
         "Can not have more high bits to fill than integerPartWidth");
  ಸ್ಥಿರಾಂಕ integerPart ಮೇಲಿನ_ಬಿಟ್_Fill = ~integerPart(0)
                                  << (integer_Part_ಅಗಲ - ಅಂಕೆ_ಮೇಲಿನ_ಬಿಟ್ಟುಗಳು);
  ಆದರೆ (~(Parts[Part_ಎಣಿಕೆ - 1] | ಮೇಲಿನ_ಬಿಟ್_Fill | 0x1))
    ಹಿಂತಿರುಗಿ ಸುಳ್ಳು;

  ಹಿಂತಿರುಗಿ ನಿಜ;
}

ತಾರ್ಕಿಕ IEEEFloat::isSignificandAllZeros() ಸ್ಥಿರಾಂಕ {
  // Test if the significand excluding the integral bit is all zeros. This
  // allows us to test for binade boundaries.
  ಸ್ಥಿರಾಂಕ integerPart *Parts = significandParts();
  ಸ್ಥಿರಾಂಕ ಚಿಹ್ನೆರಹಿತ Part_ಎಣಿಕೆ = part_ಎಣಿಕೆ_For_ಬಿಟ್ಟುಗಳು(semantics->precision);

  ಗಾಗಿ (ಚಿಹ್ನೆರಹಿತ i = 0; i < Part_ಎಣಿಕೆ - 1; i++)
    ಆದರೆ (Parts[i])
      ಹಿಂತಿರುಗಿ ಸುಳ್ಳು;

  // Compute how many bits are used in the final word.
  ಸ್ಥಿರಾಂಕ ಚಿಹ್ನೆರಹಿತ ಅಂಕೆ_ಮೇಲಿನ_ಬಿಟ್ಟುಗಳು = get_ಅಂಕೆ_ಮೇಲಿನ_ಬಿಟ್ಟುಗಳು();
  ಪ್ರತಿಜ್ಞಾ(ಅಂಕೆ_ಮೇಲಿನ_ಬಿಟ್ಟುಗಳು < integer_Part_ಅಗಲ && "Can not have more high bits to "
         "clear than integerPartWidth");
  ಸ್ಥಿರಾಂಕ integerPart ಮೇಲಿನ_ಬಿಟ್_ಮುಖವಾಡ = ~integerPart(0) >> ಅಂಕೆ_ಮೇಲಿನ_ಬಿಟ್ಟುಗಳು;

  ಆದರೆ ((semantics->precision > 1) && (Parts[Part_ಎಣಿಕೆ - 1] & ಮೇಲಿನ_ಬಿಟ್_ಮುಖವಾಡ))
    ಹಿಂತಿರುಗಿ ಸುಳ್ಳು;

  ಹಿಂತಿರುಗಿ ನಿಜ;
}

ತಾರ್ಕಿಕ IEEEFloat::isSignificandAllZerosExceptMSB() ಸ್ಥಿರಾಂಕ {
  ಸ್ಥಿರಾಂಕ integerPart *Parts = significandParts();
  ಸ್ಥಿರಾಂಕ ಚಿಹ್ನೆರಹಿತ Part_ಎಣಿಕೆ = part_ಎಣಿಕೆ_For_ಬಿಟ್ಟುಗಳು(semantics->precision);

  ಗಾಗಿ (ಚಿಹ್ನೆರಹಿತ i = 0; i < Part_ಎಣಿಕೆ - 1; i++) {
    ಆದರೆ (Parts[i])
      ಹಿಂತಿರುಗಿ ಸುಳ್ಳು;
  }

  ಸ್ಥಿರಾಂಕ ಚಿಹ್ನೆರಹಿತ ಅಂಕೆ_ಮೇಲಿನ_ಬಿಟ್ಟುಗಳು = get_ಅಂಕೆ_ಮೇಲಿನ_ಬಿಟ್ಟುಗಳು();
  ಸ್ಥಿರಾಂಕ integerPart MSBMask = integerPart(1)
                              << (integer_Part_ಅಗಲ - ಅಂಕೆ_ಮೇಲಿನ_ಬಿಟ್ಟುಗಳು);
  ಹಿಂತಿರುಗಿ ((semantics->precision <= 1) || (Parts[Part_ಎಣಿಕೆ - 1] == MSBMask));
}

ತಾರ್ಕಿಕ IEEEFloat::is_ಅತಿದೊಡ್ಡ() ಸ್ಥಿರಾಂಕ {
  ತಾರ್ಕಿಕ IsMaxExp = isFiniteNonZero() && exponent == semantics->maxExponent;
  ಆದರೆ (semantics->nonFiniteBehavior == fltNonfiniteBehavior::NanOnly &&
      semantics->nanEncoding == fltNanEncoding::AllOnes) {
    // The largest number by magnitude in our format will be the floating point
    // number with maximum exponent and with significand that is all ones except
    // the LSB.
    ಹಿಂತಿರುಗಿ (IsMaxExp && APFloat::hasSignificand(*semantics))
               ? isSignificandAllOnesExceptLSB()
               : IsMaxExp;
  } ಇಲ್ಲದಿದ್ದರೆ {
    // The largest number by magnitude in our format will be the floating point
    // number with maximum exponent and with significand that is all ones.
    ಹಿಂತಿರುಗಿ IsMaxExp && isSignificandAllOnes();
  }
}

ತಾರ್ಕಿಕ IEEEFloat::isInteger() ಸ್ಥಿರಾಂಕ {
  // This could be made more efficient; I'm going for obviously correct.
  ಆದರೆ (!isFinite()) ಹಿಂತಿರುಗಿ ಸುಳ್ಳು;
  IEEEFloat truncated = *ಇದು;
  truncated.ಸುತ್ತು_ಗಮ್ಯ_Integral(rmTowardZero);
  ಹಿಂತಿರುಗಿ compare(truncated) == cmpEqual;
}

ತಾರ್ಕಿಕ IEEEFloat::bitwiseIsEqual(ಸ್ಥಿರಾಂಕ IEEEFloat &rhs) ಸ್ಥಿರಾಂಕ {
  ಆದರೆ (ಇದು == &rhs)
    ಹಿಂತಿರುಗಿ ನಿಜ;
  ಆದರೆ (semantics != rhs.semantics ||
      category != rhs.category ||
      sign != rhs.sign)
    ಹಿಂತಿರುಗಿ ಸುಳ್ಳು;
  ಆದರೆ (category==fcZero || category==fcInfinity)
    ಹಿಂತಿರುಗಿ ನಿಜ;

  ಆದರೆ (isFiniteNonZero() && exponent != rhs.exponent)
    ಹಿಂತಿರುಗಿ ಸುಳ್ಳು;

  ಹಿಂತಿರುಗಿ std::equal(significandParts(), significandParts() + part_ಎಣಿಕೆ(),
                    rhs.significandParts());
}

IEEEFloat::IEEEFloat(ಸ್ಥಿರಾಂಕ fltSemantics &ourSemantics, integerPart ಮೌಲ್ಯ_2) {
  initialize(&ourSemantics);
  sign = 0;
  category = fc_ಸಾಮಾನ್ಯ;
  zeroSignificand();
  exponent = ourSemantics.precision - 1;
  significandParts()[0] = ಮೌಲ್ಯ_2;
  normalize(rm_ಹತ್ತಿರದ_Ties_ಗಮ್ಯ_Even, lfExactlyZero);
}

IEEEFloat::IEEEFloat(ಸ್ಥಿರಾಂಕ fltSemantics &ourSemantics) {
  initialize(&ourSemantics);
  // The Float8E8MOFNU format does not have a representation
  // for zero. So, use the closest representation instead.
  // Moreover, the all-zero encoding represents a valid
  // normal value (which is the smallestNormalized here).
  // Hence, we call makeSmallestNormalized (where category is
  // 'fcNormal') instead of makeZero (where category is 'fcZero').
  ourSemantics.hasZero ? makeZero(ಸುಳ್ಳು) : make_ಅತಿಚಿಕ್ಕ_Normalized(ಸುಳ್ಳು);
}

// Delegate to the previous constructor, because later copy constructor may
// actually inspects category, which can't be garbage.
IEEEFloat::IEEEFloat(ಸ್ಥಿರಾಂಕ fltSemantics &ourSemantics, uninitialized_ಟ್ಯಾಗ್ ಟ್ಯಾಗ್_2)
    : IEEEFloat(ourSemantics) {}

IEEEFloat::IEEEFloat(ಸ್ಥಿರಾಂಕ IEEEFloat &rhs) {
  initialize(rhs.semantics);
  ನಿಯೋಜಿಸು(rhs);
}

IEEEFloat::IEEEFloat(IEEEFloat &&rhs) : semantics(&APFloat_ತಳ::semBogus) {
  *ಇದು = std::move(rhs);
}

IEEEFloat::~IEEEFloat() { freeSignificand(); }

ಚಿಹ್ನೆರಹಿತ ಪೂರ್ಣಾಂಕ IEEEFloat::part_ಎಣಿಕೆ() ಸ್ಥಿರಾಂಕ {
  ಹಿಂತಿರುಗಿ part_ಎಣಿಕೆ_For_ಬಿಟ್ಟುಗಳು(semantics->precision + 1);
}

ಸ್ಥಿರಾಂಕ APFloat::integerPart *IEEEFloat::significandParts() ಸ್ಥಿರಾಂಕ {
  ಹಿಂತಿರುಗಿ const_cast<IEEEFloat *>(ಇದು)->significandParts();
}

APFloat::integerPart *IEEEFloat::significandParts() {
  ಆದರೆ (part_ಎಣಿಕೆ() > 1)
    ಹಿಂತಿರುಗಿ significand.parts;
  ಇಲ್ಲದಿದ್ದರೆ
    ಹಿಂತಿರುಗಿ &significand.part;
}

ಶೂನ್ಯ IEEEFloat::zeroSignificand() {
  APInt::tc_ಗಣ(significandParts(), 0, part_ಎಣಿಕೆ());
}

/* Increment an fcNormal floating point number's significand.  */
ಶೂನ್ಯ IEEEFloat::incrementSignificand() {
  [[maybe_ಬಳಸದ]] integerPart ವಾಹಕ =
      APInt::tcIncrement(significandParts(), part_ಎಣಿಕೆ());

  /* Our callers should never cause us to overflow.  */
  ಪ್ರತಿಜ್ಞಾ(ವಾಹಕ == 0);
}

/* Add the significand of the RHS.  Returns the carry flag.  */
APFloat::integerPart IEEEFloat::ಕೂಡಿಸು_Significand(ಸ್ಥಿರಾಂಕ IEEEFloat &rhs) {
  integerPart *parts = significandParts();

  ಪ್ರತಿಜ್ಞಾ(semantics == rhs.semantics);
  ಪ್ರತಿಜ್ಞಾ(exponent == rhs.exponent);

  ಹಿಂತಿರುಗಿ APInt::tc_ಕೂಡಿಸು(parts, rhs.significandParts(), 0, part_ಎಣಿಕೆ());
}

/* Subtract the significand of the RHS with a borrow flag.  Returns
   the borrow flag.  */
APFloat::integerPart IEEEFloat::ಕಳೆ_Significand(ಸ್ಥಿರಾಂಕ IEEEFloat &rhs,
                                                    integerPart borrow) {
  integerPart *parts = significandParts();

  ಪ್ರತಿಜ್ಞಾ(semantics == rhs.semantics);
  ಪ್ರತಿಜ್ಞಾ(exponent == rhs.exponent);

  ಹಿಂತಿರುಗಿ APInt::tc_ಕಳೆ(parts, rhs.significandParts(), borrow,
                           part_ಎಣಿಕೆ());
}

/* Multiply the significand of the RHS.  If ADDEND is non-NULL, add it
   on to the full-precision result of the multiplication.  Returns the
   lost fraction.  */
ಕಳೆದುಹೋದ_Fraction IEEEFloat::multiplySignificand(ಸ್ಥಿರಾಂಕ IEEEFloat &rhs,
                                            IEEEFloat addend,
                                            ತಾರ್ಕಿಕ ignoreAddend) {
  integerPart scratch[4];
  ತಾರ್ಕಿಕ ignored;

  ಪ್ರತಿಜ್ಞಾ(semantics == rhs.semantics);

  ಚಿಹ್ನೆರಹಿತ precision = semantics->precision;

  // Allocate space for twice as many bits as the original significand, plus one
  // extra bit for the addition to overflow into.
  ಚಿಹ್ನೆರಹಿತ new_Parts_ಎಣಿಕೆ = part_ಎಣಿಕೆ_For_ಬಿಟ್ಟುಗಳು(precision * 2 + 1);

  // FIXME: Replace with SmallVector<4>.
  integerPart *ತುಂಬಿದ_Significand =
      new_Parts_ಎಣಿಕೆ > 4 ? ಹೊಸ integerPart[new_Parts_ಎಣಿಕೆ] : scratch;

  integerPart *lhsSignificand = significandParts();
  ಚಿಹ್ನೆರಹಿತ parts_ಎಣಿಕೆ = part_ಎಣಿಕೆ();

  APInt::tc_ತುಂಬಿದ_Multiply(ತುಂಬಿದ_Significand, lhsSignificand,
                        rhs.significandParts(), parts_ಎಣಿಕೆ, parts_ಎಣಿಕೆ);

  ಕಳೆದುಹೋದ_Fraction ಕಳೆದುಹೋದ_fraction = lfExactlyZero;
  // One, not zero, based MSB.
  ಚಿಹ್ನೆರಹಿತ omsb = APInt::tcMSB(ತುಂಬಿದ_Significand, new_Parts_ಎಣಿಕೆ) + 1;
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

  ಆದರೆ (!ignoreAddend && addend.isNonZero()) {
    // The intermediate result of the multiplication has "2 * precision"
    // signicant bit; adjust the addend to be consistent with mul result.
    //
    Significand savedSignificand = significand;
    ಸ್ಥಿರಾಂಕ fltSemantics *savedSemantics = semantics;

    // Normalize our MSB to one below the top bit to allow for overflow.
    ಚಿಹ್ನೆರಹಿತ extendedPrecision = 2 * precision + 1;
    ಆದರೆ (omsb != extendedPrecision - 1) {
      ಪ್ರತಿಜ್ಞಾ(extendedPrecision > omsb);
      APInt::tc_ಸರಿಸು_ಎಡ(ತುಂಬಿದ_Significand, new_Parts_ಎಣಿಕೆ,
                         (extendedPrecision - 1) - omsb);
      exponent -= (extendedPrecision - 1) - omsb;
    }

    /* Create new semantics.  */
    fltSemantics extendedSemantics = *semantics;
    extendedSemantics.precision = extendedPrecision;

    ಆದರೆ (new_Parts_ಎಣಿಕೆ == 1)
      significand.part = ತುಂಬಿದ_Significand[0];
    ಇಲ್ಲದಿದ್ದರೆ
      significand.parts = ತುಂಬಿದ_Significand;
    semantics = &extendedSemantics;

    // Make a copy so we can convert it to the extended semantics.
    // Note that we cannot convert the addend directly, as the extendedSemantics
    // is a local variable (which we take a reference to).
    IEEEFloat extendedAddend(addend);
    [[maybe_ಬಳಸದ]] op_ಸ್ಥಿತಿ ಸ್ಥಿತಿ_3 = extendedAddend.convert(
        extendedSemantics, APFloat::rmTowardZero, &ignored);
    ಪ್ರತಿಜ್ಞಾ(ಸ್ಥಿತಿ_3 == APFloat::opOK);

    // Shift the significand of the addend right by one bit. This guarantees
    // that the high bit of the significand is zero (same as fullSignificand),
    // so the addition will overflow (if it does overflow at all) into the top bit.
    ಕಳೆದುಹೋದ_fraction = extendedAddend.ಸರಿಸು_Significand_ಬಲ(1);
    ಪ್ರತಿಜ್ಞಾ(ಕಳೆದುಹೋದ_fraction == lfExactlyZero &&
           "Lost precision while shifting addend for fused-multiply-add.");

    ಕಳೆದುಹೋದ_fraction = ಕೂಡಿಸು_Or_ಕಳೆ_Significand(extendedAddend, ಸುಳ್ಳು);

    /* Restore our state.  */
    ಆದರೆ (new_Parts_ಎಣಿಕೆ == 1)
      ತುಂಬಿದ_Significand[0] = significand.part;
    significand = savedSignificand;
    semantics = savedSemantics;

    omsb = APInt::tcMSB(ತುಂಬಿದ_Significand, new_Parts_ಎಣಿಕೆ) + 1;
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
  ಆದರೆ (omsb > precision) {
    ಚಿಹ್ನೆರಹಿತ ಪೂರ್ಣಾಂಕ ಬಿಟ್ಟುಗಳು, significantParts;
    ಕಳೆದುಹೋದ_Fraction lf;

    ಬಿಟ್ಟುಗಳು = omsb - precision;
    significantParts = part_ಎಣಿಕೆ_For_ಬಿಟ್ಟುಗಳು(omsb);
    lf = ಸರಿಸು_ಬಲ(ತುಂಬಿದ_Significand, significantParts, ಬಿಟ್ಟುಗಳು);
    ಕಳೆದುಹೋದ_fraction = combine_ಕಳೆದುಹೋದ_Fractions(lf, ಕಳೆದುಹೋದ_fraction);
    exponent += ಬಿಟ್ಟುಗಳು;
  }

  APInt::tc_ನಿಯೋಜಿಸು(lhsSignificand, ತುಂಬಿದ_Significand, parts_ಎಣಿಕೆ);

  ಆದರೆ (new_Parts_ಎಣಿಕೆ > 4)
    ಅಳಿಸು [] ತುಂಬಿದ_Significand;

  ಹಿಂತಿರುಗಿ ಕಳೆದುಹೋದ_fraction;
}

ಕಳೆದುಹೋದ_Fraction IEEEFloat::multiplySignificand(ಸ್ಥಿರಾಂಕ IEEEFloat &rhs) {
  // When the given semantics has zero, the addend here is a zero.
  // i.e . it belongs to the 'fcZero' category.
  // But when the semantics does not support zero, we need to
  // explicitly convey that this addend should be ignored
  // for multiplication.
  ಹಿಂತಿರುಗಿ multiplySignificand(rhs, IEEEFloat(*semantics), !semantics->hasZero);
}

/* Multiply the significands of LHS and RHS to DST.  */
ಕಳೆದುಹೋದ_Fraction IEEEFloat::divideSignificand(ಸ್ಥಿರಾಂಕ IEEEFloat &rhs) {
  integerPart scratch[4];

  ಪ್ರತಿಜ್ಞಾ(semantics == rhs.semantics);

  integerPart *lhsSignificand = significandParts();
  ಸ್ಥಿರಾಂಕ integerPart *rhsSignificand = rhs.significandParts();
  ಚಿಹ್ನೆರಹಿತ parts_ಎಣಿಕೆ = part_ಎಣಿಕೆ();

  integerPart *dividend =
      parts_ಎಣಿಕೆ > 2 ? ಹೊಸ integerPart[parts_ಎಣಿಕೆ * 2] : scratch;
  integerPart *ಭಾಜಕ = dividend + parts_ಎಣಿಕೆ;

  /* Copy the dividend and divisor as they will be modified in-place.  */
  ಗಾಗಿ (ಚಿಹ್ನೆರಹಿತ i = 0; i < parts_ಎಣಿಕೆ; i++) {
    dividend[i] = lhsSignificand[i];
    ಭಾಜಕ[i] = rhsSignificand[i];
    lhsSignificand[i] = 0;
  }

  exponent -= rhs.exponent;

  ಚಿಹ್ನೆರಹಿತ ಪೂರ್ಣಾಂಕ precision = semantics->precision;

  /* Normalize the divisor.  */
  ಚಿಹ್ನೆರಹಿತ ಬಿಟ್ = precision - APInt::tcMSB(ಭಾಜಕ, parts_ಎಣಿಕೆ) - 1;
  ಆದರೆ (ಬಿಟ್) {
    exponent += ಬಿಟ್;
    APInt::tc_ಸರಿಸು_ಎಡ(ಭಾಜಕ, parts_ಎಣಿಕೆ, ಬಿಟ್);
  }

  /* Normalize the dividend.  */
  ಬಿಟ್ = precision - APInt::tcMSB(dividend, parts_ಎಣಿಕೆ) - 1;
  ಆದರೆ (ಬಿಟ್) {
    exponent -= ಬಿಟ್;
    APInt::tc_ಸರಿಸು_ಎಡ(dividend, parts_ಎಣಿಕೆ, ಬಿಟ್);
  }

  /* Ensure the dividend >= divisor initially for the loop below.
     Incidentally, this means that the division loop below is
     guaranteed to set the integer bit to one.  */
  ಆದರೆ (APInt::tcCompare(dividend, ಭಾಜಕ, parts_ಎಣಿಕೆ) < 0) {
    exponent--;
    APInt::tc_ಸರಿಸು_ಎಡ(dividend, parts_ಎಣಿಕೆ, 1);
    ಪ್ರತಿಜ್ಞಾ(APInt::tcCompare(dividend, ಭಾಜಕ, parts_ಎಣಿಕೆ) >= 0);
  }

  /* Long division.  */
  ಗಾಗಿ (ಬಿಟ್ = precision; ಬಿಟ್; ಬಿಟ್ -= 1) {
    ಆದರೆ (APInt::tcCompare(dividend, ಭಾಜಕ, parts_ಎಣಿಕೆ) >= 0) {
      APInt::tc_ಕಳೆ(dividend, ಭಾಜಕ, 0, parts_ಎಣಿಕೆ);
      APInt::tc_ಗಣ_ಬಿಟ್(lhsSignificand, ಬಿಟ್ - 1);
    }

    APInt::tc_ಸರಿಸು_ಎಡ(dividend, parts_ಎಣಿಕೆ, 1);
  }

  /* Figure out the lost fraction.  */
  ಪೂರ್ಣಾಂಕ cmp = APInt::tcCompare(dividend, ಭಾಜಕ, parts_ಎಣಿಕೆ);

  ಕಳೆದುಹೋದ_Fraction ಕಳೆದುಹೋದ_fraction;
  ಆದರೆ (cmp > 0)
    ಕಳೆದುಹೋದ_fraction = lf_ಇನ್ನಷ್ಟು_Than_Half;
  ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (cmp == 0)
    ಕಳೆದುಹೋದ_fraction = lfExactlyHalf;
  ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (APInt::tcIsZero(dividend, parts_ಎಣಿಕೆ))
    ಕಳೆದುಹೋದ_fraction = lfExactlyZero;
  ಇಲ್ಲದಿದ್ದರೆ
    ಕಳೆದುಹೋದ_fraction = lfLessThanHalf;

  ಆದರೆ (parts_ಎಣಿಕೆ > 2)
    ಅಳಿಸು [] dividend;

  ಹಿಂತಿರುಗಿ ಕಳೆದುಹೋದ_fraction;
}

ಚಿಹ್ನೆರಹಿತ ಪೂರ್ಣಾಂಕ IEEEFloat::significandMSB() ಸ್ಥಿರಾಂಕ {
  ಹಿಂತಿರುಗಿ APInt::tcMSB(significandParts(), part_ಎಣಿಕೆ());
}

ಚಿಹ್ನೆರಹಿತ ಪೂರ್ಣಾಂಕ IEEEFloat::significandLSB() ಸ್ಥಿರಾಂಕ {
  ಹಿಂತಿರುಗಿ APInt::tcLSB(significandParts(), part_ಎಣಿಕೆ());
}

/* Note that a zero result is NOT normalized to fcZero.  */
ಕಳೆದುಹೋದ_Fraction IEEEFloat::ಸರಿಸು_Significand_ಬಲ(ಚಿಹ್ನೆರಹಿತ ಪೂರ್ಣಾಂಕ ಬಿಟ್ಟುಗಳು) {
  /* Our exponent should not overflow.  */
  ಪ್ರತಿಜ್ಞಾ((ExponentType) (exponent + ಬಿಟ್ಟುಗಳು) >= exponent);

  exponent += ಬಿಟ್ಟುಗಳು;

  ಹಿಂತಿರುಗಿ ಸರಿಸು_ಬಲ(significandParts(), part_ಎಣಿಕೆ(), ಬಿಟ್ಟುಗಳು);
}

/* Shift the significand left BITS bits, subtract BITS from its exponent.  */
ಶೂನ್ಯ IEEEFloat::ಸರಿಸು_Significand_ಎಡ(ಚಿಹ್ನೆರಹಿತ ಪೂರ್ಣಾಂಕ ಬಿಟ್ಟುಗಳು) {
  ಪ್ರತಿಜ್ಞಾ(ಬಿಟ್ಟುಗಳು < semantics->precision ||
         (semantics->precision == 1 && ಬಿಟ್ಟುಗಳು <= 1));

  ಆದರೆ (ಬಿಟ್ಟುಗಳು) {
    ಚಿಹ್ನೆರಹಿತ ಪೂರ್ಣಾಂಕ parts_ಎಣಿಕೆ = part_ಎಣಿಕೆ();

    APInt::tc_ಸರಿಸು_ಎಡ(significandParts(), parts_ಎಣಿಕೆ, ಬಿಟ್ಟುಗಳು);
    exponent -= ಬಿಟ್ಟುಗಳು;

    ಪ್ರತಿಜ್ಞಾ(!APInt::tcIsZero(significandParts(), parts_ಎಣಿಕೆ));
  }
}

APFloat::cmp_ಫಲಿತಾಂಶ IEEEFloat::compare_Absolute_ಮೌಲ್ಯ(ಸ್ಥಿರಾಂಕ IEEEFloat &rhs) ಸ್ಥಿರಾಂಕ {
  ಪ್ರತಿಜ್ಞಾ(semantics == rhs.semantics);
  ಪ್ರತಿಜ್ಞಾ(isFiniteNonZero());
  ಪ್ರತಿಜ್ಞಾ(rhs.isFiniteNonZero());

  ಪೂರ್ಣಾಂಕ compare = exponent - rhs.exponent;

  /* If exponents are equal, do an unsigned bignum comparison of the
     significands.  */
  ಆದರೆ (compare == 0)
    compare = APInt::tcCompare(significandParts(), rhs.significandParts(),
                               part_ಎಣಿಕೆ());

  ಆದರೆ (compare > 0)
    ಹಿಂತಿರುಗಿ cmpGreaterThan;
  ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (compare < 0)
    ಹಿಂತಿರುಗಿ cmpLessThan;
  ಇಲ್ಲದಿದ್ದರೆ
    ಹಿಂತಿರುಗಿ cmpEqual;
}

/* Set the least significant BITS bits of a bignum, clear the
   rest.  */
ಸ್ಥಿರ ಶೂನ್ಯ tc_ಗಣ_ಕನಿಷ್ಠ_Significant_ಬಿಟ್ಟುಗಳು(APInt::ಶಬ್ದ_Type *dst, ಚಿಹ್ನೆರಹಿತ parts,
                                      ಚಿಹ್ನೆರಹಿತ ಬಿಟ್ಟುಗಳು) {
  ಚಿಹ್ನೆರಹಿತ i = 0;
  ಆಗ (ಬಿಟ್ಟುಗಳು > APInt::APINT_ಬಿಟ್ಟುಗಳು_ಪ್ರತಿ_ಶಬ್ದ) {
    dst[i++] = ~(APInt::ಶಬ್ದ_Type)0;
    ಬಿಟ್ಟುಗಳು -= APInt::APINT_ಬಿಟ್ಟುಗಳು_ಪ್ರತಿ_ಶಬ್ದ;
  }

  ಆದರೆ (ಬಿಟ್ಟುಗಳು)
    dst[i++] = ~(APInt::ಶಬ್ದ_Type)0 >> (APInt::APINT_ಬಿಟ್ಟುಗಳು_ಪ್ರತಿ_ಶಬ್ದ - ಬಿಟ್ಟುಗಳು);

  ಆಗ (i < parts)
    dst[i++] = 0;
}

/* Handle overflow.  Sign is preserved.  We either become infinity or
   the largest finite number.  */
APFloat::op_ಸ್ಥಿತಿ IEEEFloat::handle_ಅತಿಪ್ರವಾಹ(rounding_ಬಹುಲಕ rounding_ಬಹುಲಕ_2) {
  ಆದರೆ (semantics->nonFiniteBehavior != fltNonfiniteBehavior::FiniteOnly) {
    /* Infinity?  */
    ಆದರೆ (rounding_ಬಹುಲಕ_2 == rm_ಹತ್ತಿರದ_Ties_ಗಮ್ಯ_Even ||
        rounding_ಬಹುಲಕ_2 == rm_ಹತ್ತಿರದ_Ties_ಗಮ್ಯ_Away ||
        (rounding_ಬಹುಲಕ_2 == rm_Toward_ಧನಾತ್ಮಕ && !sign) ||
        (rounding_ಬಹುಲಕ_2 == rmTowardNegative && sign)) {
      ಆದರೆ (semantics->nonFiniteBehavior == fltNonfiniteBehavior::NanOnly)
        makeNaN(ಸುಳ್ಳು, sign);
      ಇಲ್ಲದಿದ್ದರೆ
        category = fcInfinity;
      ಹಿಂತಿರುಗಿ static_cast<op_ಸ್ಥಿತಿ>(op_ಅತಿಪ್ರವಾಹ | opInexact);
    }
  }

  /* Otherwise we become the largest finite number.  */
  category = fc_ಸಾಮಾನ್ಯ;
  exponent = semantics->maxExponent;
  tc_ಗಣ_ಕನಿಷ್ಠ_Significant_ಬಿಟ್ಟುಗಳು(significandParts(), part_ಎಣಿಕೆ(),
                            semantics->precision);
  ಆದರೆ (semantics->nonFiniteBehavior == fltNonfiniteBehavior::NanOnly &&
      semantics->nanEncoding == fltNanEncoding::AllOnes)
    APInt::tc_Clear_ಬಿಟ್(significandParts(), 0);

  ಹಿಂತಿರುಗಿ opInexact;
}

/* Returns TRUE if, when truncating the current number, with BIT the
   new LSB, with the given lost fraction and rounding mode, the result
   would need to be rounded away from zero (i.e., by increasing the
   signficand).  This routine must work for fcZero of both signs, and
   fcNormal numbers.  */
ತಾರ್ಕಿಕ IEEEFloat::ಸುತ್ತು_Away_From_Zero(rounding_ಬಹುಲಕ rounding_ಬಹುಲಕ_2,
                                  ಕಳೆದುಹೋದ_Fraction ಕಳೆದುಹೋದ_fraction,
                                  ಚಿಹ್ನೆರಹಿತ ಪೂರ್ಣಾಂಕ ಬಿಟ್) ಸ್ಥಿರಾಂಕ {
  /* NaNs and infinities should not have lost fractions.  */
  ಪ್ರತಿಜ್ಞಾ(isFiniteNonZero() || category == fcZero);

  /* Current callers never pass this so we don't handle it.  */
  ಪ್ರತಿಜ್ಞಾ(ಕಳೆದುಹೋದ_fraction != lfExactlyZero);

  ಆಯ್ಕೆ (rounding_ಬಹುಲಕ_2) {
  ಸ್ಥಿತಿ rm_ಹತ್ತಿರದ_Ties_ಗಮ್ಯ_Away:
    ಹಿಂತಿರುಗಿ ಕಳೆದುಹೋದ_fraction == lfExactlyHalf || ಕಳೆದುಹೋದ_fraction == lf_ಇನ್ನಷ್ಟು_Than_Half;

  ಸ್ಥಿತಿ rm_ಹತ್ತಿರದ_Ties_ಗಮ್ಯ_Even:
    ಆದರೆ (ಕಳೆದುಹೋದ_fraction == lf_ಇನ್ನಷ್ಟು_Than_Half)
      ಹಿಂತಿರುಗಿ ನಿಜ;

    /* Our zeroes don't have a significand to test.  */
    ಆದರೆ (ಕಳೆದುಹೋದ_fraction == lfExactlyHalf && category != fcZero)
      ಹಿಂತಿರುಗಿ APInt::tc_ಹೊರತೆಗೆ_ಬಿಟ್(significandParts(), ಬಿಟ್);

    ಹಿಂತಿರುಗಿ ಸುಳ್ಳು;

  ಸ್ಥಿತಿ rmTowardZero:
    ಹಿಂತಿರುಗಿ ಸುಳ್ಳು;

  ಸ್ಥಿತಿ rm_Toward_ಧನಾತ್ಮಕ:
    ಹಿಂತಿರುಗಿ !sign;

  ಸ್ಥಿತಿ rmTowardNegative:
    ಹಿಂತಿರುಗಿ sign;

  ಪೂರ್ವನಿಯೋಜಿತ:
    ನಿಲ್ಲಿಸು;
  }
  llvm_unreachable("Invalid rounding mode found");
}

APFloat::op_ಸ್ಥಿತಿ IEEEFloat::normalize(rounding_ಬಹುಲಕ rounding_ಬಹುಲಕ_2,
                                       ಕಳೆದುಹೋದ_Fraction ಕಳೆದುಹೋದ_fraction) {
  ಆದರೆ (!isFiniteNonZero())
    ಹಿಂತಿರುಗಿ opOK;

  /* Before rounding normalize the exponent of fcNormal numbers.  */
  /* One, not zero, based MSB.  */
  ಚಿಹ್ನೆರಹಿತ omsb = significandMSB() + 1;

  // Only skip this `if` if the value is exactly zero.
  ಆದರೆ (omsb || ಕಳೆದುಹೋದ_fraction != lfExactlyZero) {
    /* OMSB is numbered from 1.  We want to place it in the integer
       bit numbered PRECISION if possible, with a compensating change in
       the exponent.  */
    ಪೂರ್ಣಾಂಕ exponentChange = omsb - semantics->precision;

    /* If the resulting exponent is too high, overflow according to
       the rounding mode.  */
    ಆದರೆ (exponent + exponentChange > semantics->maxExponent)
      ಹಿಂತಿರುಗಿ handle_ಅತಿಪ್ರವಾಹ(rounding_ಬಹುಲಕ_2);

    /* Subnormal numbers have exponent minExponent, and their MSB
       is forced based on that.  */
    ಆದರೆ (exponent + exponentChange < semantics->minExponent)
      exponentChange = semantics->minExponent - exponent;

    /* Shifting left is easy as we don't lose precision.  */
    ಆದರೆ (exponentChange < 0) {
      ಪ್ರತಿಜ್ಞಾ(ಕಳೆದುಹೋದ_fraction == lfExactlyZero);

      ಸರಿಸು_Significand_ಎಡ(-exponentChange);

      ಹಿಂತಿರುಗಿ opOK;
    }

    ಆದರೆ (exponentChange > 0) {
      ಕಳೆದುಹೋದ_Fraction lf;

      /* Shift right and capture any new lost fraction.  */
      lf = ಸರಿಸು_Significand_ಬಲ(exponentChange);

      ಕಳೆದುಹೋದ_fraction = combine_ಕಳೆದುಹೋದ_Fractions(lf, ಕಳೆದುಹೋದ_fraction);

      /* Keep OMSB up-to-date.  */
      ಆದರೆ (omsb > (ಚಿಹ್ನೆರಹಿತ) exponentChange)
        omsb -= exponentChange;
      ಇಲ್ಲದಿದ್ದರೆ
        omsb = 0;
    }
  }

  // The all-ones values is an overflow if NaN is all ones. If NaN is
  // represented by negative zero, then it is a valid finite value.
  ಆದರೆ (semantics->nonFiniteBehavior == fltNonfiniteBehavior::NanOnly &&
      semantics->nanEncoding == fltNanEncoding::AllOnes &&
      exponent == semantics->maxExponent && isSignificandAllOnes())
    ಹಿಂತಿರುಗಿ handle_ಅತಿಪ್ರವಾಹ(rounding_ಬಹುಲಕ_2);

  /* Now round the number according to rounding_mode given the lost
     fraction.  */

  /* As specified in IEEE 754, since we do not trap we do not report
     underflow for exact results.  */
  ಆದರೆ (ಕಳೆದುಹೋದ_fraction == lfExactlyZero) {
    /* Canonicalize zeroes.  */
    ಆದರೆ (omsb == 0) {
      category = fcZero;
      ಆದರೆ (semantics->nanEncoding == fltNanEncoding::NegativeZero)
        sign = ಸುಳ್ಳು;
      ಆದರೆ (!semantics->hasZero)
        make_ಅತಿಚಿಕ್ಕ_Normalized(ಸುಳ್ಳು);
    }

    ಹಿಂತಿರುಗಿ opOK;
  }

  /* Increment the significand if we're rounding away from zero.  */
  ಆದರೆ (ಸುತ್ತು_Away_From_Zero(rounding_ಬಹುಲಕ_2, ಕಳೆದುಹೋದ_fraction, 0)) {
    ಆದರೆ (omsb == 0)
      exponent = semantics->minExponent;

    incrementSignificand();
    omsb = significandMSB() + 1;

    /* Did the significand increment overflow?  */
    ಆದರೆ (omsb == (ಚಿಹ್ನೆರಹಿತ) semantics->precision + 1) {
      /* Renormalize by incrementing the exponent and shifting our
         significand right one.  However if we already have the
         maximum exponent we overflow to infinity.  */
      ಆದರೆ (exponent == semantics->maxExponent)
        // Invoke overflow handling with a rounding mode that will guarantee
        // that the result gets turned into the correct infinity representation.
        // This is needed instead of just setting the category to infinity to
        // account for 8-bit floating point types that have no inf, only NaN.
        ಹಿಂತಿರುಗಿ handle_ಅತಿಪ್ರವಾಹ(sign ? rmTowardNegative : rm_Toward_ಧನಾತ್ಮಕ);

      ಸರಿಸು_Significand_ಬಲ(1);

      ಹಿಂತಿರುಗಿ opInexact;
    }

    // The all-ones values is an overflow if NaN is all ones. If NaN is
    // represented by negative zero, then it is a valid finite value.
    ಆದರೆ (semantics->nonFiniteBehavior == fltNonfiniteBehavior::NanOnly &&
        semantics->nanEncoding == fltNanEncoding::AllOnes &&
        exponent == semantics->maxExponent && isSignificandAllOnes())
      ಹಿಂತಿರುಗಿ handle_ಅತಿಪ್ರವಾಹ(rounding_ಬಹುಲಕ_2);
  }

  /* The normal case - we were and are not denormal, and any
     significand increment above didn't overflow.  */
  ಆದರೆ (omsb == semantics->precision)
    ಹಿಂತಿರುಗಿ opInexact;

  /* We have a non-zero denormal.  */
  ಪ್ರತಿಜ್ಞಾ(omsb < semantics->precision);

  /* Canonicalize zeroes.  */
  ಆದರೆ (omsb == 0) {
    category = fcZero;
    ಆದರೆ (semantics->nanEncoding == fltNanEncoding::NegativeZero)
      sign = ಸುಳ್ಳು;
    // This condition handles the case where the semantics
    // does not have zero but uses the all-zero encoding
    // to represent the smallest normal value.
    ಆದರೆ (!semantics->hasZero)
      make_ಅತಿಚಿಕ್ಕ_Normalized(ಸುಳ್ಳು);
  }

  /* The fcZero case is a denormal that underflowed to zero.  */
  ಹಿಂತಿರುಗಿ (op_ಸ್ಥಿತಿ) (opUnderflow | opInexact);
}

APFloat::op_ಸ್ಥಿತಿ IEEEFloat::ಕೂಡಿಸು_Or_ಕಳೆ_Specials(ಸ್ಥಿರಾಂಕ IEEEFloat &rhs,
                                                   ತಾರ್ಕಿಕ ಕಳೆ) {
  ಆಯ್ಕೆ (Pack_Categories_Into_ಕೀಲಿ(category, rhs.category)) {
  ಪೂರ್ವನಿಯೋಜಿತ:
    llvm_unreachable(ಶೂನ್ಯಸೂಚಕ);

  ಸ್ಥಿತಿ Pack_Categories_Into_ಕೀಲಿ(fcZero, fcNaN):
  ಸ್ಥಿತಿ Pack_Categories_Into_ಕೀಲಿ(fc_ಸಾಮಾನ್ಯ, fcNaN):
  ಸ್ಥಿತಿ Pack_Categories_Into_ಕೀಲಿ(fcInfinity, fcNaN):
    ನಿಯೋಜಿಸು(rhs);
    [[fallthrough]];
  ಸ್ಥಿತಿ Pack_Categories_Into_ಕೀಲಿ(fcNaN, fcZero):
  ಸ್ಥಿತಿ Pack_Categories_Into_ಕೀಲಿ(fcNaN, fc_ಸಾಮಾನ್ಯ):
  ಸ್ಥಿತಿ Pack_Categories_Into_ಕೀಲಿ(fcNaN, fcInfinity):
  ಸ್ಥಿತಿ Pack_Categories_Into_ಕೀಲಿ(fcNaN, fcNaN):
    ಆದರೆ (isSignaling()) {
      makeQuiet();
      ಹಿಂತಿರುಗಿ opInvalidOp;
    }
    ಹಿಂತಿರುಗಿ rhs.isSignaling() ? opInvalidOp : opOK;

  ಸ್ಥಿತಿ Pack_Categories_Into_ಕೀಲಿ(fc_ಸಾಮಾನ್ಯ, fcZero):
  ಸ್ಥಿತಿ Pack_Categories_Into_ಕೀಲಿ(fcInfinity, fc_ಸಾಮಾನ್ಯ):
  ಸ್ಥಿತಿ Pack_Categories_Into_ಕೀಲಿ(fcInfinity, fcZero):
    ಹಿಂತಿರುಗಿ opOK;

  ಸ್ಥಿತಿ Pack_Categories_Into_ಕೀಲಿ(fc_ಸಾಮಾನ್ಯ, fcInfinity):
  ಸ್ಥಿತಿ Pack_Categories_Into_ಕೀಲಿ(fcZero, fcInfinity):
    category = fcInfinity;
    sign = rhs.sign ^ ಕಳೆ;
    ಹಿಂತಿರುಗಿ opOK;

  ಸ್ಥಿತಿ Pack_Categories_Into_ಕೀಲಿ(fcZero, fc_ಸಾಮಾನ್ಯ):
    ನಿಯೋಜಿಸು(rhs);
    sign = rhs.sign ^ ಕಳೆ;
    ಹಿಂತಿರುಗಿ opOK;

  ಸ್ಥಿತಿ Pack_Categories_Into_ಕೀಲಿ(fcZero, fcZero):
    /* Sign depends on rounding mode; handled by caller.  */
    ಹಿಂತಿರುಗಿ opOK;

  ಸ್ಥಿತಿ Pack_Categories_Into_ಕೀಲಿ(fcInfinity, fcInfinity):
    /* Differently signed infinities can only be validly
       subtracted.  */
    ಆದರೆ (((sign ^ rhs.sign)!=0) != ಕಳೆ) {
      makeNaN();
      ಹಿಂತಿರುಗಿ opInvalidOp;
    }

    ಹಿಂತಿರುಗಿ opOK;

  ಸ್ಥಿತಿ Pack_Categories_Into_ಕೀಲಿ(fc_ಸಾಮಾನ್ಯ, fc_ಸಾಮಾನ್ಯ):
    ಹಿಂತಿರುಗಿ opDivByZero;
  }
}

/* Add or subtract two normal numbers.  */
ಕಳೆದುಹೋದ_Fraction IEEEFloat::ಕೂಡಿಸು_Or_ಕಳೆ_Significand(ಸ್ಥಿರಾಂಕ IEEEFloat &rhs,
                                                 ತಾರ್ಕಿಕ ಕಳೆ) {
  [[maybe_ಬಳಸದ]] integerPart ವಾಹಕ = 0;
  ಕಳೆದುಹೋದ_Fraction ಕಳೆದುಹೋದ_fraction;

  /* Determine if the operation on the absolute values is effectively
     an addition or subtraction.  */
  ಕಳೆ ^= static_cast<ತಾರ್ಕಿಕ>(sign ^ rhs.sign);

  /* Are we bigger exponent-wise than the RHS?  */
  ಪೂರ್ಣಾಂಕ ಬಿಟ್ಟುಗಳು = exponent - rhs.exponent;

  /* Subtraction is more subtle than one might naively expect.  */
  ಆದರೆ (ಕಳೆ) {
    ಆದರೆ ((ಬಿಟ್ಟುಗಳು < 0) && !semantics->hasSignedRepr)
      llvm_unreachable(
          "This floating point format does not support signed values");

    IEEEFloat ತಾತ್ಕಾಲಿಕ_rhs(rhs);
    ತಾರ್ಕಿಕ ಕಳೆದುಹೋದ_fraction_is_from_rhs = ಸುಳ್ಳು;

    ಆದರೆ (ಬಿಟ್ಟುಗಳು == 0)
      ಕಳೆದುಹೋದ_fraction = lfExactlyZero;
    ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (ಬಿಟ್ಟುಗಳು > 0) {
      ಕಳೆದುಹೋದ_fraction = ತಾತ್ಕಾಲಿಕ_rhs.ಸರಿಸು_Significand_ಬಲ(ಬಿಟ್ಟುಗಳು - 1);
      ಕಳೆದುಹೋದ_fraction_is_from_rhs = ನಿಜ;
      ಸರಿಸು_Significand_ಎಡ(1);
    } ಇಲ್ಲದಿದ್ದರೆ {
      ಕಳೆದುಹೋದ_fraction = ಸರಿಸು_Significand_ಬಲ(-ಬಿಟ್ಟುಗಳು - 1);
      ತಾತ್ಕಾಲಿಕ_rhs.ಸರಿಸು_Significand_ಎಡ(1);
    }

    // Should we reverse the subtraction.
    cmp_ಫಲಿತಾಂಶ cmp_ಫಲಿತಾಂಶ_2 = compare_Absolute_ಮೌಲ್ಯ(ತಾತ್ಕಾಲಿಕ_rhs);
    ಆದರೆ (cmp_ಫಲಿತಾಂಶ_2 == cmpLessThan) {
      ತಾರ್ಕಿಕ borrow =
          ಕಳೆದುಹೋದ_fraction != lfExactlyZero && !ಕಳೆದುಹೋದ_fraction_is_from_rhs;
      ಆದರೆ (borrow) {
        // The lost fraction is being subtracted, borrow from the significand
        // and invert `lost_fraction`.
        ಆದರೆ (ಕಳೆದುಹೋದ_fraction == lfLessThanHalf)
          ಕಳೆದುಹೋದ_fraction = lf_ಇನ್ನಷ್ಟು_Than_Half;
        ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (ಕಳೆದುಹೋದ_fraction == lf_ಇನ್ನಷ್ಟು_Than_Half)
          ಕಳೆದುಹೋದ_fraction = lfLessThanHalf;
      }
      ವಾಹಕ = ತಾತ್ಕಾಲಿಕ_rhs.ಕಳೆ_Significand(*ಇದು, borrow);
      copySignificand(ತಾತ್ಕಾಲಿಕ_rhs);
      sign = !sign;
    } ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (cmp_ಫಲಿತಾಂಶ_2 == cmpGreaterThan) {
      ತಾರ್ಕಿಕ borrow = ಕಳೆದುಹೋದ_fraction != lfExactlyZero && ಕಳೆದುಹೋದ_fraction_is_from_rhs;
      ಆದರೆ (borrow) {
        // The lost fraction is being subtracted, borrow from the significand
        // and invert `lost_fraction`.
        ಆದರೆ (ಕಳೆದುಹೋದ_fraction == lfLessThanHalf)
          ಕಳೆದುಹೋದ_fraction = lf_ಇನ್ನಷ್ಟು_Than_Half;
        ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (ಕಳೆದುಹೋದ_fraction == lf_ಇನ್ನಷ್ಟು_Than_Half)
          ಕಳೆದುಹೋದ_fraction = lfLessThanHalf;
      }
      ವಾಹಕ = ಕಳೆ_Significand(ತಾತ್ಕಾಲಿಕ_rhs, borrow);
    } ಇಲ್ಲದಿದ್ದರೆ { // cmpEqual
      zeroSignificand();
      ಆದರೆ (ಕಳೆದುಹೋದ_fraction != lfExactlyZero && ಕಳೆದುಹೋದ_fraction_is_from_rhs) {
        // rhs is slightly larger due to the lost fraction, flip the sign.
        sign = !sign;
      }
    }

    /* The code above is intended to ensure that no borrow is
       necessary.  */
    ಪ್ರತಿಜ್ಞಾ(!ವಾಹಕ);
  } ಇಲ್ಲದಿದ್ದರೆ {
    ಆದರೆ (ಬಿಟ್ಟುಗಳು > 0) {
      IEEEFloat ತಾತ್ಕಾಲಿಕ_rhs(rhs);

      ಕಳೆದುಹೋದ_fraction = ತಾತ್ಕಾಲಿಕ_rhs.ಸರಿಸು_Significand_ಬಲ(ಬಿಟ್ಟುಗಳು);
      ವಾಹಕ = ಕೂಡಿಸು_Significand(ತಾತ್ಕಾಲಿಕ_rhs);
    } ಇಲ್ಲದಿದ್ದರೆ {
      ಕಳೆದುಹೋದ_fraction = ಸರಿಸು_Significand_ಬಲ(-ಬಿಟ್ಟುಗಳು);
      ವಾಹಕ = ಕೂಡಿಸು_Significand(rhs);
    }

    /* We have a guard bit; generating a carry cannot happen.  */
    ಪ್ರತಿಜ್ಞಾ(!ವಾಹಕ);
  }

  ಹಿಂತಿರುಗಿ ಕಳೆದುಹೋದ_fraction;
}

APFloat::op_ಸ್ಥಿತಿ IEEEFloat::multiplySpecials(ಸ್ಥಿರಾಂಕ IEEEFloat &rhs) {
  ಆಯ್ಕೆ (Pack_Categories_Into_ಕೀಲಿ(category, rhs.category)) {
  ಪೂರ್ವನಿಯೋಜಿತ:
    llvm_unreachable(ಶೂನ್ಯಸೂಚಕ);

  ಸ್ಥಿತಿ Pack_Categories_Into_ಕೀಲಿ(fcZero, fcNaN):
  ಸ್ಥಿತಿ Pack_Categories_Into_ಕೀಲಿ(fc_ಸಾಮಾನ್ಯ, fcNaN):
  ಸ್ಥಿತಿ Pack_Categories_Into_ಕೀಲಿ(fcInfinity, fcNaN):
    ನಿಯೋಜಿಸು(rhs);
    sign = ಸುಳ್ಳು;
    [[fallthrough]];
  ಸ್ಥಿತಿ Pack_Categories_Into_ಕೀಲಿ(fcNaN, fcZero):
  ಸ್ಥಿತಿ Pack_Categories_Into_ಕೀಲಿ(fcNaN, fc_ಸಾಮಾನ್ಯ):
  ಸ್ಥಿತಿ Pack_Categories_Into_ಕೀಲಿ(fcNaN, fcInfinity):
  ಸ್ಥಿತಿ Pack_Categories_Into_ಕೀಲಿ(fcNaN, fcNaN):
    sign ^= rhs.sign; // restore the original sign
    ಆದರೆ (isSignaling()) {
      makeQuiet();
      ಹಿಂತಿರುಗಿ opInvalidOp;
    }
    ಹಿಂತಿರುಗಿ rhs.isSignaling() ? opInvalidOp : opOK;

  ಸ್ಥಿತಿ Pack_Categories_Into_ಕೀಲಿ(fc_ಸಾಮಾನ್ಯ, fcInfinity):
  ಸ್ಥಿತಿ Pack_Categories_Into_ಕೀಲಿ(fcInfinity, fc_ಸಾಮಾನ್ಯ):
  ಸ್ಥಿತಿ Pack_Categories_Into_ಕೀಲಿ(fcInfinity, fcInfinity):
    category = fcInfinity;
    ಹಿಂತಿರುಗಿ opOK;

  ಸ್ಥಿತಿ Pack_Categories_Into_ಕೀಲಿ(fcZero, fc_ಸಾಮಾನ್ಯ):
  ಸ್ಥಿತಿ Pack_Categories_Into_ಕೀಲಿ(fc_ಸಾಮಾನ್ಯ, fcZero):
  ಸ್ಥಿತಿ Pack_Categories_Into_ಕೀಲಿ(fcZero, fcZero):
    category = fcZero;
    ಹಿಂತಿರುಗಿ opOK;

  ಸ್ಥಿತಿ Pack_Categories_Into_ಕೀಲಿ(fcZero, fcInfinity):
  ಸ್ಥಿತಿ Pack_Categories_Into_ಕೀಲಿ(fcInfinity, fcZero):
    makeNaN();
    ಹಿಂತಿರುಗಿ opInvalidOp;

  ಸ್ಥಿತಿ Pack_Categories_Into_ಕೀಲಿ(fc_ಸಾಮಾನ್ಯ, fc_ಸಾಮಾನ್ಯ):
    ಹಿಂತಿರುಗಿ opOK;
  }
}

APFloat::op_ಸ್ಥಿತಿ IEEEFloat::divideSpecials(ಸ್ಥಿರಾಂಕ IEEEFloat &rhs) {
  ಆಯ್ಕೆ (Pack_Categories_Into_ಕೀಲಿ(category, rhs.category)) {
  ಪೂರ್ವನಿಯೋಜಿತ:
    llvm_unreachable(ಶೂನ್ಯಸೂಚಕ);

  ಸ್ಥಿತಿ Pack_Categories_Into_ಕೀಲಿ(fcZero, fcNaN):
  ಸ್ಥಿತಿ Pack_Categories_Into_ಕೀಲಿ(fc_ಸಾಮಾನ್ಯ, fcNaN):
  ಸ್ಥಿತಿ Pack_Categories_Into_ಕೀಲಿ(fcInfinity, fcNaN):
    ನಿಯೋಜಿಸು(rhs);
    sign = ಸುಳ್ಳು;
    [[fallthrough]];
  ಸ್ಥಿತಿ Pack_Categories_Into_ಕೀಲಿ(fcNaN, fcZero):
  ಸ್ಥಿತಿ Pack_Categories_Into_ಕೀಲಿ(fcNaN, fc_ಸಾಮಾನ್ಯ):
  ಸ್ಥಿತಿ Pack_Categories_Into_ಕೀಲಿ(fcNaN, fcInfinity):
  ಸ್ಥಿತಿ Pack_Categories_Into_ಕೀಲಿ(fcNaN, fcNaN):
    sign ^= rhs.sign; // restore the original sign
    ಆದರೆ (isSignaling()) {
      makeQuiet();
      ಹಿಂತಿರುಗಿ opInvalidOp;
    }
    ಹಿಂತಿರುಗಿ rhs.isSignaling() ? opInvalidOp : opOK;

  ಸ್ಥಿತಿ Pack_Categories_Into_ಕೀಲಿ(fcInfinity, fcZero):
  ಸ್ಥಿತಿ Pack_Categories_Into_ಕೀಲಿ(fcInfinity, fc_ಸಾಮಾನ್ಯ):
  ಸ್ಥಿತಿ Pack_Categories_Into_ಕೀಲಿ(fcZero, fcInfinity):
  ಸ್ಥಿತಿ Pack_Categories_Into_ಕೀಲಿ(fcZero, fc_ಸಾಮಾನ್ಯ):
    ಹಿಂತಿರುಗಿ opOK;

  ಸ್ಥಿತಿ Pack_Categories_Into_ಕೀಲಿ(fc_ಸಾಮಾನ್ಯ, fcInfinity):
    category = fcZero;
    ಹಿಂತಿರುಗಿ opOK;

  ಸ್ಥಿತಿ Pack_Categories_Into_ಕೀಲಿ(fc_ಸಾಮಾನ್ಯ, fcZero):
    ಆದರೆ (semantics->nonFiniteBehavior == fltNonfiniteBehavior::NanOnly)
      makeNaN(ಸುಳ್ಳು, sign);
    ಇಲ್ಲದಿದ್ದರೆ
      category = fcInfinity;
    ಹಿಂತಿರುಗಿ opDivByZero;

  ಸ್ಥಿತಿ Pack_Categories_Into_ಕೀಲಿ(fcInfinity, fcInfinity):
  ಸ್ಥಿತಿ Pack_Categories_Into_ಕೀಲಿ(fcZero, fcZero):
    makeNaN();
    ಹಿಂತಿರುಗಿ opInvalidOp;

  ಸ್ಥಿತಿ Pack_Categories_Into_ಕೀಲಿ(fc_ಸಾಮಾನ್ಯ, fc_ಸಾಮಾನ್ಯ):
    ಹಿಂತಿರುಗಿ opOK;
  }
}

APFloat::op_ಸ್ಥಿತಿ IEEEFloat::modSpecials(ಸ್ಥಿರಾಂಕ IEEEFloat &rhs) {
  ಆಯ್ಕೆ (Pack_Categories_Into_ಕೀಲಿ(category, rhs.category)) {
  ಪೂರ್ವನಿಯೋಜಿತ:
    llvm_unreachable(ಶೂನ್ಯಸೂಚಕ);

  ಸ್ಥಿತಿ Pack_Categories_Into_ಕೀಲಿ(fcZero, fcNaN):
  ಸ್ಥಿತಿ Pack_Categories_Into_ಕೀಲಿ(fc_ಸಾಮಾನ್ಯ, fcNaN):
  ಸ್ಥಿತಿ Pack_Categories_Into_ಕೀಲಿ(fcInfinity, fcNaN):
    ನಿಯೋಜಿಸು(rhs);
    [[fallthrough]];
  ಸ್ಥಿತಿ Pack_Categories_Into_ಕೀಲಿ(fcNaN, fcZero):
  ಸ್ಥಿತಿ Pack_Categories_Into_ಕೀಲಿ(fcNaN, fc_ಸಾಮಾನ್ಯ):
  ಸ್ಥಿತಿ Pack_Categories_Into_ಕೀಲಿ(fcNaN, fcInfinity):
  ಸ್ಥಿತಿ Pack_Categories_Into_ಕೀಲಿ(fcNaN, fcNaN):
    ಆದರೆ (isSignaling()) {
      makeQuiet();
      ಹಿಂತಿರುಗಿ opInvalidOp;
    }
    ಹಿಂತಿರುಗಿ rhs.isSignaling() ? opInvalidOp : opOK;

  ಸ್ಥಿತಿ Pack_Categories_Into_ಕೀಲಿ(fcZero, fcInfinity):
  ಸ್ಥಿತಿ Pack_Categories_Into_ಕೀಲಿ(fcZero, fc_ಸಾಮಾನ್ಯ):
  ಸ್ಥಿತಿ Pack_Categories_Into_ಕೀಲಿ(fc_ಸಾಮಾನ್ಯ, fcInfinity):
    ಹಿಂತಿರುಗಿ opOK;

  ಸ್ಥಿತಿ Pack_Categories_Into_ಕೀಲಿ(fc_ಸಾಮಾನ್ಯ, fcZero):
  ಸ್ಥಿತಿ Pack_Categories_Into_ಕೀಲಿ(fcInfinity, fcZero):
  ಸ್ಥಿತಿ Pack_Categories_Into_ಕೀಲಿ(fcInfinity, fc_ಸಾಮಾನ್ಯ):
  ಸ್ಥಿತಿ Pack_Categories_Into_ಕೀಲಿ(fcInfinity, fcInfinity):
  ಸ್ಥಿತಿ Pack_Categories_Into_ಕೀಲಿ(fcZero, fcZero):
    makeNaN();
    ಹಿಂತಿರುಗಿ opInvalidOp;

  ಸ್ಥಿತಿ Pack_Categories_Into_ಕೀಲಿ(fc_ಸಾಮಾನ್ಯ, fc_ಸಾಮಾನ್ಯ):
    ಹಿಂತಿರುಗಿ opOK;
  }
}

APFloat::op_ಸ್ಥಿತಿ IEEEFloat::remainderSpecials(ಸ್ಥಿರಾಂಕ IEEEFloat &rhs) {
  ಆಯ್ಕೆ (Pack_Categories_Into_ಕೀಲಿ(category, rhs.category)) {
  ಪೂರ್ವನಿಯೋಜಿತ:
    llvm_unreachable(ಶೂನ್ಯಸೂಚಕ);

  ಸ್ಥಿತಿ Pack_Categories_Into_ಕೀಲಿ(fcZero, fcNaN):
  ಸ್ಥಿತಿ Pack_Categories_Into_ಕೀಲಿ(fc_ಸಾಮಾನ್ಯ, fcNaN):
  ಸ್ಥಿತಿ Pack_Categories_Into_ಕೀಲಿ(fcInfinity, fcNaN):
    ನಿಯೋಜಿಸು(rhs);
    [[fallthrough]];
  ಸ್ಥಿತಿ Pack_Categories_Into_ಕೀಲಿ(fcNaN, fcZero):
  ಸ್ಥಿತಿ Pack_Categories_Into_ಕೀಲಿ(fcNaN, fc_ಸಾಮಾನ್ಯ):
  ಸ್ಥಿತಿ Pack_Categories_Into_ಕೀಲಿ(fcNaN, fcInfinity):
  ಸ್ಥಿತಿ Pack_Categories_Into_ಕೀಲಿ(fcNaN, fcNaN):
    ಆದರೆ (isSignaling()) {
      makeQuiet();
      ಹಿಂತಿರುಗಿ opInvalidOp;
    }
    ಹಿಂತಿರುಗಿ rhs.isSignaling() ? opInvalidOp : opOK;

  ಸ್ಥಿತಿ Pack_Categories_Into_ಕೀಲಿ(fcZero, fcInfinity):
  ಸ್ಥಿತಿ Pack_Categories_Into_ಕೀಲಿ(fcZero, fc_ಸಾಮಾನ್ಯ):
  ಸ್ಥಿತಿ Pack_Categories_Into_ಕೀಲಿ(fc_ಸಾಮಾನ್ಯ, fcInfinity):
    ಹಿಂತಿರುಗಿ opOK;

  ಸ್ಥಿತಿ Pack_Categories_Into_ಕೀಲಿ(fc_ಸಾಮಾನ್ಯ, fcZero):
  ಸ್ಥಿತಿ Pack_Categories_Into_ಕೀಲಿ(fcInfinity, fcZero):
  ಸ್ಥಿತಿ Pack_Categories_Into_ಕೀಲಿ(fcInfinity, fc_ಸಾಮಾನ್ಯ):
  ಸ್ಥಿತಿ Pack_Categories_Into_ಕೀಲಿ(fcInfinity, fcInfinity):
  ಸ್ಥಿತಿ Pack_Categories_Into_ಕೀಲಿ(fcZero, fcZero):
    makeNaN();
    ಹಿಂತಿರುಗಿ opInvalidOp;

  ಸ್ಥಿತಿ Pack_Categories_Into_ಕೀಲಿ(fc_ಸಾಮಾನ್ಯ, fc_ಸಾಮಾನ್ಯ):
    ಹಿಂತಿರುಗಿ opDivByZero; // fake status, indicating this is not a special case
  }
}

/* Change sign.  */
ಶೂನ್ಯ IEEEFloat::changeSign() {
  // With NaN-as-negative-zero, neither NaN or negative zero can change
  // their signs.
  ಆದರೆ (semantics->nanEncoding == fltNanEncoding::NegativeZero &&
      (isZero() || isNaN()))
    ಹಿಂತಿರುಗಿ;
  /* Look mummy, this one's easy.  */
  sign = !sign;
}

/* Normalized addition or subtraction.  */
APFloat::op_ಸ್ಥಿತಿ IEEEFloat::ಕೂಡಿಸು_Or_ಕಳೆ(ಸ್ಥಿರಾಂಕ IEEEFloat &rhs,
                                           rounding_ಬಹುಲಕ rounding_ಬಹುಲಕ_2,
                                           ತಾರ್ಕಿಕ ಕಳೆ) {
  op_ಸ್ಥಿತಿ fs = ಕೂಡಿಸು_Or_ಕಳೆ_Specials(rhs, ಕಳೆ);

  /* This return code means it was not a simple case.  */
  ಆದರೆ (fs == opDivByZero) {
    ಕಳೆದುಹೋದ_Fraction ಕಳೆದುಹೋದ_fraction;

    ಕಳೆದುಹೋದ_fraction = ಕೂಡಿಸು_Or_ಕಳೆ_Significand(rhs, ಕಳೆ);
    fs = normalize(rounding_ಬಹುಲಕ_2, ಕಳೆದುಹೋದ_fraction);

    /* Can only be zero if we lost no fraction.  */
    ಪ್ರತಿಜ್ಞಾ(category != fcZero || ಕಳೆದುಹೋದ_fraction == lfExactlyZero);
  }

  /* If two numbers add (exactly) to zero, IEEE 754 decrees it is a
     positive zero unless rounding to minus infinity, except that
     adding two like-signed zeroes gives that zero.  */
  ಆದರೆ (category == fcZero) {
    ಆದರೆ (rhs.category != fcZero || (sign == rhs.sign) == ಕಳೆ)
      sign = (rounding_ಬಹುಲಕ_2 == rmTowardNegative);
    // NaN-in-negative-zero means zeros need to be normalized to +0.
    ಆದರೆ (semantics->nanEncoding == fltNanEncoding::NegativeZero)
      sign = ಸುಳ್ಳು;
  }

  ಹಿಂತಿರುಗಿ fs;
}

/* Normalized addition.  */
APFloat::op_ಸ್ಥಿತಿ IEEEFloat::ಕೂಡಿಸು(ಸ್ಥಿರಾಂಕ IEEEFloat &rhs,
                                 rounding_ಬಹುಲಕ rounding_ಬಹುಲಕ_2) {
  ಹಿಂತಿರುಗಿ ಕೂಡಿಸು_Or_ಕಳೆ(rhs, rounding_ಬಹುಲಕ_2, ಸುಳ್ಳು);
}

/* Normalized subtraction.  */
APFloat::op_ಸ್ಥಿತಿ IEEEFloat::ಕಳೆ(ಸ್ಥಿರಾಂಕ IEEEFloat &rhs,
                                      rounding_ಬಹುಲಕ rounding_ಬಹುಲಕ_2) {
  ಹಿಂತಿರುಗಿ ಕೂಡಿಸು_Or_ಕಳೆ(rhs, rounding_ಬಹುಲಕ_2, ನಿಜ);
}

/* Normalized multiply.  */
APFloat::op_ಸ್ಥಿತಿ IEEEFloat::multiply(ಸ್ಥಿರಾಂಕ IEEEFloat &rhs,
                                      rounding_ಬಹುಲಕ rounding_ಬಹುಲಕ_2) {
  sign ^= rhs.sign;
  op_ಸ್ಥಿತಿ fs = multiplySpecials(rhs);

  ಆದರೆ (isZero() && semantics->nanEncoding == fltNanEncoding::NegativeZero)
    sign = ಸುಳ್ಳು;
  ಆದರೆ (isFiniteNonZero()) {
    ಕಳೆದುಹೋದ_Fraction ಕಳೆದುಹೋದ_fraction = multiplySignificand(rhs);
    fs = normalize(rounding_ಬಹುಲಕ_2, ಕಳೆದುಹೋದ_fraction);
    ಆದರೆ (ಕಳೆದುಹೋದ_fraction != lfExactlyZero)
      fs = (op_ಸ್ಥಿತಿ) (fs | opInexact);
  }

  ಹಿಂತಿರುಗಿ fs;
}

/* Normalized divide.  */
APFloat::op_ಸ್ಥಿತಿ IEEEFloat::divide(ಸ್ಥಿರಾಂಕ IEEEFloat &rhs,
                                    rounding_ಬಹುಲಕ rounding_ಬಹುಲಕ_2) {
  sign ^= rhs.sign;
  op_ಸ್ಥಿತಿ fs = divideSpecials(rhs);

  ಆದರೆ (isZero() && semantics->nanEncoding == fltNanEncoding::NegativeZero)
    sign = ಸುಳ್ಳು;
  ಆದರೆ (isFiniteNonZero()) {
    ಕಳೆದುಹೋದ_Fraction ಕಳೆದುಹೋದ_fraction = divideSignificand(rhs);
    fs = normalize(rounding_ಬಹುಲಕ_2, ಕಳೆದುಹೋದ_fraction);
    ಆದರೆ (ಕಳೆದುಹೋದ_fraction != lfExactlyZero)
      fs = (op_ಸ್ಥಿತಿ) (fs | opInexact);
  }

  ಹಿಂತಿರುಗಿ fs;
}

/* Normalized remainder.  */
APFloat::op_ಸ್ಥಿತಿ IEEEFloat::remainder(ಸ್ಥಿರಾಂಕ IEEEFloat &rhs) {
  ಚಿಹ್ನೆರಹಿತ ಪೂರ್ಣಾಂಕ origSign = sign;

  // First handle the special cases.
  op_ಸ್ಥಿತಿ fs = remainderSpecials(rhs);
  ಆದರೆ (fs != opDivByZero)
    ಹಿಂತಿರುಗಿ fs;

  fs = opOK;

  // Make sure the current value is less than twice the denom. If the addition
  // did not succeed (an overflow has happened), which means that the finite
  // value we currently posses must be less than twice the denom (as we are
  // using the same semantics).
  IEEEFloat P2 = rhs;
  ಆದರೆ (P2.ಕೂಡಿಸು(rhs, rm_ಹತ್ತಿರದ_Ties_ಗಮ್ಯ_Even) == opOK) {
    fs = mod(P2);
    ಪ್ರತಿಜ್ಞಾ(fs == opOK);
  }

  // Lets work with absolute numbers.
  IEEEFloat P = rhs;
  P.sign = ಸುಳ್ಳು;
  sign = ಸುಳ್ಳು;

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
  ತಾರ್ಕಿಕ losesInfo;
  fltSemantics extendedSemantics = *semantics;
  extendedSemantics.maxExponent++;
  extendedSemantics.minExponent--;
  extendedSemantics.precision += 2;

  IEEEFloat VEx = *ಇದು;
  fs = VEx.convert(extendedSemantics, rm_ಹತ್ತಿರದ_Ties_ಗಮ್ಯ_Even, &losesInfo);
  ಪ್ರತಿಜ್ಞಾ(fs == opOK && !losesInfo);
  IEEEFloat PEx = P;
  fs = PEx.convert(extendedSemantics, rm_ಹತ್ತಿರದ_Ties_ಗಮ್ಯ_Even, &losesInfo);
  ಪ್ರತಿಜ್ಞಾ(fs == opOK && !losesInfo);

  // It is simpler to work with 2x instead of 0.5p, and we do not need to lose
  // any fraction.
  fs = VEx.ಕೂಡಿಸು(VEx, rm_ಹತ್ತಿರದ_Ties_ಗಮ್ಯ_Even);
  ಪ್ರತಿಜ್ಞಾ(fs == opOK);

  ಆದರೆ (VEx.compare(PEx) == cmpGreaterThan) {
    fs = ಕಳೆ(P, rm_ಹತ್ತಿರದ_Ties_ಗಮ್ಯ_Even);
    ಪ್ರತಿಜ್ಞಾ(fs == opOK);

    // Make VEx = this.add(this), but because we have different semantics, we do
    // not want to `convert` again, so we just subtract PEx twice (which equals
    // to the desired value).
    fs = VEx.ಕಳೆ(PEx, rm_ಹತ್ತಿರದ_Ties_ಗಮ್ಯ_Even);
    ಪ್ರತಿಜ್ಞಾ(fs == opOK);
    fs = VEx.ಕಳೆ(PEx, rm_ಹತ್ತಿರದ_Ties_ಗಮ್ಯ_Even);
    ಪ್ರತಿಜ್ಞಾ(fs == opOK);

    cmp_ಫಲಿತಾಂಶ ಫಲಿತಾಂಶ_2 = VEx.compare(PEx);
    ಆದರೆ (ಫಲಿತಾಂಶ_2 == cmpGreaterThan || ಫಲಿತಾಂಶ_2 == cmpEqual) {
      fs = ಕಳೆ(P, rm_ಹತ್ತಿರದ_Ties_ಗಮ್ಯ_Even);
      ಪ್ರತಿಜ್ಞಾ(fs == opOK);
    }
  }

  ಆದರೆ (isZero()) {
    sign = origSign;    // IEEE754 requires this
    ಆದರೆ (semantics->nanEncoding == fltNanEncoding::NegativeZero)
      // But some 8-bit floats only have positive 0.
      sign = ಸುಳ್ಳು;
  } ಇಲ್ಲದಿದ್ದರೆ {
    sign ^= origSign;
  }
  ಹಿಂತಿರುಗಿ fs;
}

/* Normalized llvm frem (C fmod). */
APFloat::op_ಸ್ಥಿತಿ IEEEFloat::mod(ಸ್ಥಿರಾಂಕ IEEEFloat &rhs) {
  op_ಸ್ಥಿತಿ fs = modSpecials(rhs);
  ಚಿಹ್ನೆರಹಿತ ಪೂರ್ಣಾಂಕ origSign = sign;

  ಆಗ (isFiniteNonZero() && rhs.isFiniteNonZero() &&
         compare_Absolute_ಮೌಲ್ಯ(rhs) != cmpLessThan) {
    ಪೂರ್ಣಾಂಕ Exp = ilogb(*ಇದು) - ilogb(rhs);
    IEEEFloat V = scalbn(rhs, Exp, rm_ಹತ್ತಿರದ_Ties_ಗಮ್ಯ_Even);
    // V can overflow to NaN with fltNonfiniteBehavior::NanOnly, so explicitly
    // check for it.
    ಆದರೆ (V.isNaN() || compare_Absolute_ಮೌಲ್ಯ(V) == cmpLessThan)
      V = scalbn(rhs, Exp - 1, rm_ಹತ್ತಿರದ_Ties_ಗಮ್ಯ_Even);
    V.sign = sign;

    fs = ಕಳೆ(V, rm_ಹತ್ತಿರದ_Ties_ಗಮ್ಯ_Even);

    // When the semantics supports zero, this loop's
    // exit-condition is handled by the 'isFiniteNonZero'
    // category check above. However, when the semantics
    // does not have 'fcZero' and we have reached the
    // minimum possible value, (and any further subtract
    // will underflow to the same value) explicitly
    // provide an exit-path here.
    ಆದರೆ (!semantics->hasZero && ಇದು->is_ಅತಿಚಿಕ್ಕ())
      ನಿಲ್ಲಿಸು;

    ಪ್ರತಿಜ್ಞಾ(fs==opOK);
  }
  ಆದರೆ (isZero()) {
    sign = origSign; // fmod requires this
    ಆದರೆ (semantics->nanEncoding == fltNanEncoding::NegativeZero)
      sign = ಸುಳ್ಳು;
  }
  ಹಿಂತಿರುಗಿ fs;
}

/* Normalized fused-multiply-add.  */
APFloat::op_ಸ್ಥಿತಿ IEEEFloat::fused_Multiply_ಕೂಡಿಸು(ಸ್ಥಿರಾಂಕ IEEEFloat &multiplicand,
                                              ಸ್ಥಿರಾಂಕ IEEEFloat &addend,
                                              rounding_ಬಹುಲಕ rounding_ಬಹುಲಕ_2) {
  op_ಸ್ಥಿತಿ fs;

  /* Post-multiplication sign, before addition.  */
  sign ^= multiplicand.sign;

  /* If and only if all arguments are normal do we need to do an
     extended-precision calculation.  */
  ಆದರೆ (isFiniteNonZero() &&
      multiplicand.isFiniteNonZero() &&
      addend.isFinite()) {
    ಕಳೆದುಹೋದ_Fraction ಕಳೆದುಹೋದ_fraction;

    ಕಳೆದುಹೋದ_fraction = multiplySignificand(multiplicand, addend);
    fs = normalize(rounding_ಬಹುಲಕ_2, ಕಳೆದುಹೋದ_fraction);
    ಆದರೆ (ಕಳೆದುಹೋದ_fraction != lfExactlyZero)
      fs = (op_ಸ್ಥಿತಿ) (fs | opInexact);

    /* If two numbers add (exactly) to zero, IEEE 754 decrees it is a
       positive zero unless rounding to minus infinity, except that
       adding two like-signed zeroes gives that zero.  */
    ಆದರೆ (category == fcZero && !(fs & opUnderflow) && sign != addend.sign) {
      sign = (rounding_ಬಹುಲಕ_2 == rmTowardNegative);
      ಆದರೆ (semantics->nanEncoding == fltNanEncoding::NegativeZero)
        sign = ಸುಳ್ಳು;
    }
  } ಇಲ್ಲದಿದ್ದರೆ {
    fs = multiplySpecials(multiplicand);

    /* FS can only be opOK or opInvalidOp.  There is no more work
       to do in the latter case.  The IEEE-754R standard says it is
       implementation-defined in this case whether, if ADDEND is a
       quiet NaN, we raise invalid op; this implementation does so.

       If we need to do the addition we can do so with normal
       precision.  */
    ಆದರೆ (fs == opOK)
      fs = ಕೂಡಿಸು_Or_ಕಳೆ(addend, rounding_ಬಹುಲಕ_2, ಸುಳ್ಳು);
  }

  ಹಿಂತಿರುಗಿ fs;
}

/* Rounding-mode correct round to integral value.  */
APFloat::op_ಸ್ಥಿತಿ IEEEFloat::ಸುತ್ತು_ಗಮ್ಯ_Integral(rounding_ಬಹುಲಕ rounding_ಬಹುಲಕ_2) {
  ಆದರೆ (isInfinity())
    // [IEEE Std 754-2008 6.1]:
    // The behavior of infinity in floating-point arithmetic is derived from the
    // limiting cases of real arithmetic with operands of arbitrarily
    // large magnitude, when such a limit exists.
    // ...
    // Operations on infinite operands are usually exact and therefore signal no
    // exceptions ...
    ಹಿಂತಿರುಗಿ opOK;

  ಆದರೆ (isNaN()) {
    ಆದರೆ (isSignaling()) {
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
      ಹಿಂತಿರುಗಿ opInvalidOp;
    } ಇಲ್ಲದಿದ್ದರೆ {
      // [IEEE Std 754-2008 6.2]:
      // For an operation with quiet NaN inputs, other than maximum and minimum
      // operations, if a floating-point result is to be delivered the result
      // shall be a quiet NaN which should be one of the input NaNs.
      // ...
      // Every general-computational and quiet-computational operation involving
      // one or more input NaNs, none of them signaling, shall signal no
      // exception, except fusedMultiplyAdd might signal the invalid operation
      // exception(see 7.2).
      ಹಿಂತಿರುಗಿ opOK;
    }
  }

  ಆದರೆ (isZero()) {
    // [IEEE Std 754-2008 6.3]:
    // ... the sign of the result of conversions, the quantize operation, the
    // roundToIntegral operations, and the roundToIntegralExact(see 5.3.1) is
    // the sign of the first or only operand.
    ಹಿಂತಿರುಗಿ opOK;
  }

  // If the exponent is large enough, we know that this value is already
  // integral, and the arithmetic below would potentially cause it to saturate
  // to +/-Inf.  Bail out early instead.
  ಆದರೆ (exponent + 1 >= (ಪೂರ್ಣಾಂಕ)APFloat::semanticsPrecision(*semantics))
    ಹಿಂತಿರುಗಿ opOK;

  // The algorithm here is quite simple: we add 2^(p-1), where p is the
  // precision of our format, and then subtract it back off again.  The choice
  // of rounding modes for the addition/subtraction determines the rounding mode
  // for our integral rounding as well.
  // NOTE: When the input value is negative, we do subtraction followed by
  // addition instead.
  APInt IntegerConstant(ಮುಂದಿನ_Power_Of2(APFloat::semanticsPrecision(*semantics)),
                        1);
  IntegerConstant <<= APFloat::semanticsPrecision(*semantics) - 1;
  IEEEFloat MagicConstant(*semantics);
  op_ಸ್ಥಿತಿ fs = MagicConstant.convertFromAPInt(IntegerConstant, ಸುಳ್ಳು,
                                               rm_ಹತ್ತಿರದ_Ties_ಗಮ್ಯ_Even);
  ಪ್ರತಿಜ್ಞಾ(fs == opOK);
  MagicConstant.sign = sign;

  // Preserve the input sign so that we can handle the case of zero result
  // correctly.
  ತಾರ್ಕಿಕ ಒಳಹರಿವು_Sign = isNegative();

  fs = ಕೂಡಿಸು(MagicConstant, rounding_ಬಹುಲಕ_2);

  // Current value and 'MagicConstant' are both integers, so the result of the
  // subtraction is always exact according to Sterbenz' lemma.
  ಕಳೆ(MagicConstant, rounding_ಬಹುಲಕ_2);

  // Restore the input sign.
  ಆದರೆ (ಒಳಹರಿವು_Sign != isNegative())
    changeSign();

  ಹಿಂತಿರುಗಿ fs;
}

/* Comparison requires normalized numbers.  */
APFloat::cmp_ಫಲಿತಾಂಶ IEEEFloat::compare(ಸ್ಥಿರಾಂಕ IEEEFloat &rhs) ಸ್ಥಿರಾಂಕ {
  ಪ್ರತಿಜ್ಞಾ(semantics == rhs.semantics);

  ಆಯ್ಕೆ (Pack_Categories_Into_ಕೀಲಿ(category, rhs.category)) {
  ಪೂರ್ವನಿಯೋಜಿತ:
    llvm_unreachable(ಶೂನ್ಯಸೂಚಕ);

  ಸ್ಥಿತಿ Pack_Categories_Into_ಕೀಲಿ(fcNaN, fcZero):
  ಸ್ಥಿತಿ Pack_Categories_Into_ಕೀಲಿ(fcNaN, fc_ಸಾಮಾನ್ಯ):
  ಸ್ಥಿತಿ Pack_Categories_Into_ಕೀಲಿ(fcNaN, fcInfinity):
  ಸ್ಥಿತಿ Pack_Categories_Into_ಕೀಲಿ(fcNaN, fcNaN):
  ಸ್ಥಿತಿ Pack_Categories_Into_ಕೀಲಿ(fcZero, fcNaN):
  ಸ್ಥಿತಿ Pack_Categories_Into_ಕೀಲಿ(fc_ಸಾಮಾನ್ಯ, fcNaN):
  ಸ್ಥಿತಿ Pack_Categories_Into_ಕೀಲಿ(fcInfinity, fcNaN):
    ಹಿಂತಿರುಗಿ cmpUnordered;

  ಸ್ಥಿತಿ Pack_Categories_Into_ಕೀಲಿ(fcInfinity, fc_ಸಾಮಾನ್ಯ):
  ಸ್ಥಿತಿ Pack_Categories_Into_ಕೀಲಿ(fcInfinity, fcZero):
  ಸ್ಥಿತಿ Pack_Categories_Into_ಕೀಲಿ(fc_ಸಾಮಾನ್ಯ, fcZero):
    ಆದರೆ (sign)
      ಹಿಂತಿರುಗಿ cmpLessThan;
    ಇಲ್ಲದಿದ್ದರೆ
      ಹಿಂತಿರುಗಿ cmpGreaterThan;

  ಸ್ಥಿತಿ Pack_Categories_Into_ಕೀಲಿ(fc_ಸಾಮಾನ್ಯ, fcInfinity):
  ಸ್ಥಿತಿ Pack_Categories_Into_ಕೀಲಿ(fcZero, fcInfinity):
  ಸ್ಥಿತಿ Pack_Categories_Into_ಕೀಲಿ(fcZero, fc_ಸಾಮಾನ್ಯ):
    ಆದರೆ (rhs.sign)
      ಹಿಂತಿರುಗಿ cmpGreaterThan;
    ಇಲ್ಲದಿದ್ದರೆ
      ಹಿಂತಿರುಗಿ cmpLessThan;

  ಸ್ಥಿತಿ Pack_Categories_Into_ಕೀಲಿ(fcInfinity, fcInfinity):
    ಆದರೆ (sign == rhs.sign)
      ಹಿಂತಿರುಗಿ cmpEqual;
    ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (sign)
      ಹಿಂತಿರುಗಿ cmpLessThan;
    ಇಲ್ಲದಿದ್ದರೆ
      ಹಿಂತಿರುಗಿ cmpGreaterThan;

  ಸ್ಥಿತಿ Pack_Categories_Into_ಕೀಲಿ(fcZero, fcZero):
    ಹಿಂತಿರುಗಿ cmpEqual;

  ಸ್ಥಿತಿ Pack_Categories_Into_ಕೀಲಿ(fc_ಸಾಮಾನ್ಯ, fc_ಸಾಮಾನ್ಯ):
    ನಿಲ್ಲಿಸು;
  }

  cmp_ಫಲಿತಾಂಶ ಫಲಿತಾಂಶ_2;
  /* Two normal numbers.  Do they have the same sign?  */
  ಆದರೆ (sign != rhs.sign) {
    ಆದರೆ (sign)
      ಫಲಿತಾಂಶ_2 = cmpLessThan;
    ಇಲ್ಲದಿದ್ದರೆ
      ಫಲಿತಾಂಶ_2 = cmpGreaterThan;
  } ಇಲ್ಲದಿದ್ದರೆ {
    /* Compare absolute values; invert result if negative.  */
    ಫಲಿತಾಂಶ_2 = compare_Absolute_ಮೌಲ್ಯ(rhs);

    ಆದರೆ (sign) {
      ಆದರೆ (ಫಲಿತಾಂಶ_2 == cmpLessThan)
        ಫಲಿತಾಂಶ_2 = cmpGreaterThan;
      ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (ಫಲಿತಾಂಶ_2 == cmpGreaterThan)
        ಫಲಿತಾಂಶ_2 = cmpLessThan;
    }
  }

  ಹಿಂತಿರುಗಿ ಫಲಿತಾಂಶ_2;
}

/// IEEEFloat::convert - convert a value of one floating point type to another.
/// The return value corresponds to the IEEE754 exceptions.  *losesInfo
/// records whether the transformation lost information, i.e. whether
/// converting the result back to the original type will produce the
/// original value (this is almost the same as return value==fsOK, but there
/// are edge cases where this is not so).

APFloat::op_ಸ್ಥಿತಿ IEEEFloat::convert(ಸ್ಥಿರಾಂಕ fltSemantics &ಗಮ್ಯ_Semantics,
                                     rounding_ಬಹುಲಕ rounding_ಬಹುಲಕ_2,
                                     ತಾರ್ಕಿಕ *losesInfo) {
  op_ಸ್ಥಿತಿ fs;
  ಸ್ಥಿರಾಂಕ fltSemantics &fromSemantics = *semantics;
  ತಾರ್ಕಿಕ is_signaling = isSignaling();

  ಕಳೆದುಹೋದ_Fraction ಕಳೆದುಹೋದ_Fraction = lfExactlyZero;
  ಚಿಹ್ನೆರಹಿತ new_Part_ಎಣಿಕೆ = part_ಎಣಿಕೆ_For_ಬಿಟ್ಟುಗಳು(ಗಮ್ಯ_Semantics.precision + 1);
  ಚಿಹ್ನೆರಹಿತ old_Part_ಎಣಿಕೆ = part_ಎಣಿಕೆ();
  ಪೂರ್ಣಾಂಕ ಸರಿಸು = ಗಮ್ಯ_Semantics.precision - fromSemantics.precision;

  ತಾರ್ಕಿಕ X86SpecialNan = ಸುಳ್ಳು;
  ಆದರೆ (&fromSemantics == &APFloat_ತಳ::semX87DoubleExtended &&
      &ಗಮ್ಯ_Semantics != &APFloat_ತಳ::semX87DoubleExtended && category == fcNaN &&
      (!(*significandParts() & 0x8000000000000000ULL) ||
       !(*significandParts() & 0x4000000000000000ULL))) {
    // x86 has some unusual NaNs which cannot be represented in any other
    // format; note them here.
    X86SpecialNan = ನಿಜ;
  }

  // If this is a truncation of a denormal number, and the target semantics
  // has larger exponent range than the source semantics (this can happen
  // when truncating from PowerPC double-double to double format), the
  // right shift could lose result mantissa bits.  Adjust exponent instead
  // of performing excessive shift.
  // Also do a similar trick in case shifting denormal would produce zero
  // significand as this case isn't handled correctly by normalize.
  ಆದರೆ (ಸರಿಸು < 0 && isFiniteNonZero()) {
    ಪೂರ್ಣಾಂಕ omsb = significandMSB() + 1;
    ಪೂರ್ಣಾಂಕ exponentChange = omsb - fromSemantics.precision;
    ಆದರೆ (exponent + exponentChange < ಗಮ್ಯ_Semantics.minExponent)
      exponentChange = ಗಮ್ಯ_Semantics.minExponent - exponent;
    exponentChange = std::max(exponentChange, ಸರಿಸು);
    ಆದರೆ (exponentChange < 0) {
      ಸರಿಸು -= exponentChange;
      exponent += exponentChange;
    } ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (omsb <= -ಸರಿಸು) {
      exponentChange = omsb + ಸರಿಸು - 1; // leave at least one bit set
      ಸರಿಸು -= exponentChange;
      exponent += exponentChange;
    }
  }

  // If this is a truncation, perform the shift before we narrow the storage.
  ಆದರೆ (ಸರಿಸು < 0 && (isFiniteNonZero() ||
                    (category == fcNaN && semantics->nonFiniteBehavior !=
                                              fltNonfiniteBehavior::NanOnly)))
    ಕಳೆದುಹೋದ_Fraction = ಸರಿಸು_ಬಲ(significandParts(), old_Part_ಎಣಿಕೆ, -ಸರಿಸು);

  // Fix the storage so it can hold to new value.
  ಆದರೆ (new_Part_ಎಣಿಕೆ > old_Part_ಎಣಿಕೆ) {
    // The new type requires more storage; make it available.
    integerPart *newParts;
    newParts = ಹೊಸ integerPart[new_Part_ಎಣಿಕೆ];
    APInt::tc_ಗಣ(newParts, 0, new_Part_ಎಣಿಕೆ);
    ಆದರೆ (isFiniteNonZero() || category==fcNaN)
      APInt::tc_ನಿಯೋಜಿಸು(newParts, significandParts(), old_Part_ಎಣಿಕೆ);
    freeSignificand();
    significand.parts = newParts;
  } ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (new_Part_ಎಣಿಕೆ == 1 && old_Part_ಎಣಿಕೆ != 1) {
    // Switch to built-in storage for a single part.
    integerPart newPart = 0;
    ಆದರೆ (isFiniteNonZero() || category==fcNaN)
      newPart = significandParts()[0];
    freeSignificand();
    significand.part = newPart;
  }

  // Now that we have the right storage, switch the semantics.
  semantics = &ಗಮ್ಯ_Semantics;

  // If this is an extension, perform the shift now that the storage is
  // available.
  ಆದರೆ (ಸರಿಸು > 0 && (isFiniteNonZero() || category==fcNaN))
    APInt::tc_ಸರಿಸು_ಎಡ(significandParts(), new_Part_ಎಣಿಕೆ, ಸರಿಸು);

  ಆದರೆ (isFiniteNonZero()) {
    fs = normalize(rounding_ಬಹುಲಕ_2, ಕಳೆದುಹೋದ_Fraction);
    *losesInfo = (fs != opOK);
  } ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (category == fcNaN) {
    ಆದರೆ (semantics->nonFiniteBehavior == fltNonfiniteBehavior::NanOnly) {
      *losesInfo =
          fromSemantics.nonFiniteBehavior != fltNonfiniteBehavior::NanOnly;
      makeNaN(ಸುಳ್ಳು, sign);
      fs = is_signaling ? opInvalidOp : opOK;
    } ಇಲ್ಲದಿದ್ದರೆ {
      // If NaN is negative zero, we need to create a new NaN to avoid
      // converting NaN to -Inf.
      ಆದರೆ (fromSemantics.nanEncoding == fltNanEncoding::NegativeZero &&
          semantics->nanEncoding != fltNanEncoding::NegativeZero)
        makeNaN(ಸುಳ್ಳು, ಸುಳ್ಳು);

      // If the source has no significand, there are no payload bits to carry
      // over, and an all-zero significand would encode an Inf. Create a new
      // NaN.
      ಆದರೆ (!APFloat::hasSignificand(fromSemantics))
        makeNaN(ಸುಳ್ಳು, sign);

      *losesInfo = ಕಳೆದುಹೋದ_Fraction != lfExactlyZero || X86SpecialNan;

      // For x87 extended precision, we want to make a NaN, not a special NaN
      // if the input wasn't special either.
      ಆದರೆ (!X86SpecialNan && semantics == &APFloat_ತಳ::semX87DoubleExtended)
        APInt::tc_ಗಣ_ಬಿಟ್(significandParts(), semantics->precision - 1);

      // Convert of sNaN creates qNaN and raises an exception (invalid op).
      // This also guarantees that a sNaN does not become Inf on a truncation
      // that loses all payload bits.
      ಆದರೆ (is_signaling) {
        makeQuiet();
        fs = opInvalidOp;
      } ಇಲ್ಲದಿದ್ದರೆ {
        fs = opOK;
      }
    }
  } ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (category == fcInfinity &&
             semantics->nonFiniteBehavior == fltNonfiniteBehavior::NanOnly) {
    makeNaN(ಸುಳ್ಳು, sign);
    *losesInfo = ನಿಜ;
    fs = opInexact;
  } ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (category == fcZero &&
             semantics->nanEncoding == fltNanEncoding::NegativeZero) {
    // Negative zero loses info, but positive zero doesn't.
    *losesInfo =
        fromSemantics.nanEncoding != fltNanEncoding::NegativeZero && sign;
    fs = *losesInfo ? opInexact : opOK;
    // NaN is negative zero means -0 -> +0, which can lose information
    sign = ಸುಳ್ಳು;
  } ಇಲ್ಲದಿದ್ದರೆ {
    *losesInfo = ಸುಳ್ಳು;
    fs = opOK;
  }

  // The target may have no encoding for a negative value, or none for zero.
  // The paths above only report what rounding lost, so report these here too:
  // a caller that checks losesInfo would otherwise accept a result the target
  // cannot represent, and printing that result asserts.
  ಆದರೆ ((sign && !semantics->hasSignedRepr) ||
      (category == fcZero && !semantics->hasZero)) {
    *losesInfo = ನಿಜ;
    ಆದರೆ (fs == opOK)
      fs = opInexact;
  }

  ಆದರೆ (category == fcZero && !semantics->hasZero)
    make_ಅತಿಚಿಕ್ಕ_Normalized(ಸುಳ್ಳು);
  ಹಿಂತಿರುಗಿ fs;
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
APFloat::op_ಸ್ಥಿತಿ IEEEFloat::convert_ಗಮ್ಯ_Sign_Extended_Integer(
    MutableArrayRef<integerPart> parts, ಚಿಹ್ನೆರಹಿತ ಪೂರ್ಣಾಂಕ ಅಗಲ_2, ತಾರ್ಕಿಕ isSigned,
    rounding_ಬಹುಲಕ rounding_ಬಹುಲಕ_2, ತಾರ್ಕಿಕ *isExact) ಸ್ಥಿರಾಂಕ {
  *isExact = ಸುಳ್ಳು;

  /* Handle the three special cases first.  */
  ಆದರೆ (category == fcInfinity || category == fcNaN)
    ಹಿಂತಿರುಗಿ opInvalidOp;

  ಚಿಹ್ನೆರಹಿತ dst_Parts_ಎಣಿಕೆ = part_ಎಣಿಕೆ_For_ಬಿಟ್ಟುಗಳು(ಅಗಲ_2);
  ಪ್ರತಿಜ್ಞಾ(dst_Parts_ಎಣಿಕೆ <= parts.ಗಾತ್ರ_3() && "Integer too big");

  ಆದರೆ (category == fcZero) {
    APInt::tc_ಗಣ(parts.ದತ್ತಾಂಶ(), 0, dst_Parts_ಎಣಿಕೆ);
    // Negative zero can't be represented as an int.
    *isExact = !sign;
    ಹಿಂತಿರುಗಿ opOK;
  }

  ಸ್ಥಿರಾಂಕ integerPart *src = significandParts();

  ಚಿಹ್ನೆರಹಿತ truncated_ಬಿಟ್ಟುಗಳು;
  /* Step 1: place our absolute value, with any fraction truncated, in
     the destination.  */
  ಆದರೆ (exponent < 0) {
    /* Our absolute value is less than one; truncate everything.  */
    APInt::tc_ಗಣ(parts.ದತ್ತಾಂಶ(), 0, dst_Parts_ಎಣಿಕೆ);
    /* For exponent -1 the integer bit represents .5, look at that.
       For smaller exponents leftmost truncated bit is 0. */
    truncated_ಬಿಟ್ಟುಗಳು = semantics->precision -1U - exponent;
  } ಇಲ್ಲದಿದ್ದರೆ {
    /* We want the most significant (exponent + 1) bits; the rest are
       truncated.  */
    ಚಿಹ್ನೆರಹಿತ ಪೂರ್ಣಾಂಕ ಬಿಟ್ಟುಗಳು = exponent + 1U;

    /* Hopelessly large in magnitude?  */
    ಆದರೆ (ಬಿಟ್ಟುಗಳು > ಅಗಲ_2)
      ಹಿಂತಿರುಗಿ opInvalidOp;

    ಆದರೆ (ಬಿಟ್ಟುಗಳು < semantics->precision) {
      /* We truncate (semantics->precision - bits) bits.  */
      truncated_ಬಿಟ್ಟುಗಳು = semantics->precision - ಬಿಟ್ಟುಗಳು;
      APInt::tc_ಹೊರತೆಗೆ(parts.ದತ್ತಾಂಶ(), dst_Parts_ಎಣಿಕೆ, src, ಬಿಟ್ಟುಗಳು, truncated_ಬಿಟ್ಟುಗಳು);
    } ಇಲ್ಲದಿದ್ದರೆ {
      /* We want at least as many bits as are available.  */
      APInt::tc_ಹೊರತೆಗೆ(parts.ದತ್ತಾಂಶ(), dst_Parts_ಎಣಿಕೆ, src, semantics->precision,
                       0);
      APInt::tc_ಸರಿಸು_ಎಡ(parts.ದತ್ತಾಂಶ(), dst_Parts_ಎಣಿಕೆ,
                         ಬಿಟ್ಟುಗಳು - semantics->precision);
      truncated_ಬಿಟ್ಟುಗಳು = 0;
    }
  }

  /* Step 2: work out any lost fraction, and increment the absolute
     value if we would round away from zero.  */
  ಕಳೆದುಹೋದ_Fraction ಕಳೆದುಹೋದ_fraction;
  ಆದರೆ (truncated_ಬಿಟ್ಟುಗಳು) {
    ಕಳೆದುಹೋದ_fraction = ಕಳೆದುಹೋದ_Fraction_Through_Truncation(src, part_ಎಣಿಕೆ(),
                                                  truncated_ಬಿಟ್ಟುಗಳು);
    ಆದರೆ (ಕಳೆದುಹೋದ_fraction != lfExactlyZero &&
        ಸುತ್ತು_Away_From_Zero(rounding_ಬಹುಲಕ_2, ಕಳೆದುಹೋದ_fraction, truncated_ಬಿಟ್ಟುಗಳು)) {
      ಆದರೆ (APInt::tcIncrement(parts.ದತ್ತಾಂಶ(), dst_Parts_ಎಣಿಕೆ))
        ಹಿಂತಿರುಗಿ opInvalidOp;     /* Overflow.  */
    }
  } ಇಲ್ಲದಿದ್ದರೆ {
    ಕಳೆದುಹೋದ_fraction = lfExactlyZero;
  }

  /* Step 3: check if we fit in the destination.  */
  ಚಿಹ್ನೆರಹಿತ ಪೂರ್ಣಾಂಕ omsb = APInt::tcMSB(parts.ದತ್ತಾಂಶ(), dst_Parts_ಎಣಿಕೆ) + 1;

  ಆದರೆ (sign) {
    ಆದರೆ (!isSigned) {
      /* Negative numbers cannot be represented as unsigned.  */
      ಆದರೆ (omsb != 0)
        ಹಿಂತಿರುಗಿ opInvalidOp;
    } ಇಲ್ಲದಿದ್ದರೆ {
      /* It takes omsb bits to represent the unsigned integer value.
         We lose a bit for the sign, but care is needed as the
         maximally negative integer is a special case.  */
      ಆದರೆ (omsb == ಅಗಲ_2 &&
          APInt::tcLSB(parts.ದತ್ತಾಂಶ(), dst_Parts_ಎಣಿಕೆ) + 1 != omsb)
        ಹಿಂತಿರುಗಿ opInvalidOp;

      /* This case can happen because of rounding.  */
      ಆದರೆ (omsb > ಅಗಲ_2)
        ಹಿಂತಿರುಗಿ opInvalidOp;
    }

    APInt::tcNegate (parts.ದತ್ತಾಂಶ(), dst_Parts_ಎಣಿಕೆ);
  } ಇಲ್ಲದಿದ್ದರೆ {
    ಆದರೆ (omsb >= ಅಗಲ_2 + !isSigned)
      ಹಿಂತಿರುಗಿ opInvalidOp;
  }

  ಆದರೆ (ಕಳೆದುಹೋದ_fraction == lfExactlyZero) {
    *isExact = ನಿಜ;
    ಹಿಂತಿರುಗಿ opOK;
  }
  ಹಿಂತಿರುಗಿ opInexact;
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
APFloat::op_ಸ್ಥಿತಿ
IEEEFloat::convert_ಗಮ್ಯ_Integer(MutableArrayRef<integerPart> parts,
                            ಚಿಹ್ನೆರಹಿತ ಪೂರ್ಣಾಂಕ ಅಗಲ_2, ತಾರ್ಕಿಕ isSigned,
                            rounding_ಬಹುಲಕ rounding_ಬಹುಲಕ_2, ತಾರ್ಕಿಕ *isExact) ಸ್ಥಿರಾಂಕ {
  op_ಸ್ಥಿತಿ fs = convert_ಗಮ್ಯ_Sign_Extended_Integer(parts, ಅಗಲ_2, isSigned,
                                             rounding_ಬಹುಲಕ_2, isExact);

  ಆದರೆ (fs == opInvalidOp) {
    ಚಿಹ್ನೆರಹಿತ ಪೂರ್ಣಾಂಕ ಬಿಟ್ಟುಗಳು, dst_Parts_ಎಣಿಕೆ;

    dst_Parts_ಎಣಿಕೆ = part_ಎಣಿಕೆ_For_ಬಿಟ್ಟುಗಳು(ಅಗಲ_2);
    ಪ್ರತಿಜ್ಞಾ(dst_Parts_ಎಣಿಕೆ <= parts.ಗಾತ್ರ_3() && "Integer too big");

    ಆದರೆ (category == fcNaN)
      ಬಿಟ್ಟುಗಳು = 0;
    ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (sign)
      ಬಿಟ್ಟುಗಳು = isSigned;
    ಇಲ್ಲದಿದ್ದರೆ
      ಬಿಟ್ಟುಗಳು = ಅಗಲ_2 - isSigned;

    tc_ಗಣ_ಕನಿಷ್ಠ_Significant_ಬಿಟ್ಟುಗಳು(parts.ದತ್ತಾಂಶ(), dst_Parts_ಎಣಿಕೆ, ಬಿಟ್ಟುಗಳು);
    ಆದರೆ (sign && isSigned)
      APInt::tc_ಸರಿಸು_ಎಡ(parts.ದತ್ತಾಂಶ(), dst_Parts_ಎಣಿಕೆ, ಅಗಲ_2 - 1);
  }

  ಹಿಂತಿರುಗಿ fs;
}

/* Convert an unsigned integer SRC to a floating point number,
   rounding according to ROUNDING_MODE.  The sign of the floating
   point number is not modified.  */
APFloat::op_ಸ್ಥಿತಿ IEEEFloat::convertFromUnsignedParts(
    ಸ್ಥಿರಾಂಕ integerPart *src, ಚಿಹ್ನೆರಹಿತ ಪೂರ್ಣಾಂಕ src_ಎಣಿಕೆ, rounding_ಬಹುಲಕ rounding_ಬಹುಲಕ_2) {
  category = fc_ಸಾಮಾನ್ಯ;
  ಚಿಹ್ನೆರಹಿತ omsb = APInt::tcMSB(src, src_ಎಣಿಕೆ) + 1;
  integerPart *dst = significandParts();
  ಚಿಹ್ನೆರಹಿತ dst_ಎಣಿಕೆ = part_ಎಣಿಕೆ();
  ಚಿಹ್ನೆರಹಿತ precision = semantics->precision;

  /* We want the most significant PRECISION bits of SRC.  There may not
     be that many; extract what we can.  */
  ಕಳೆದುಹೋದ_Fraction ಕಳೆದುಹೋದ_fraction;
  ಆದರೆ (precision <= omsb) {
    exponent = omsb - 1;
    ಕಳೆದುಹೋದ_fraction = ಕಳೆದುಹೋದ_Fraction_Through_Truncation(src, src_ಎಣಿಕೆ,
                                                  omsb - precision);
    APInt::tc_ಹೊರತೆಗೆ(dst, dst_ಎಣಿಕೆ, src, precision, omsb - precision);
  } ಇಲ್ಲದಿದ್ದರೆ {
    exponent = precision - 1;
    ಕಳೆದುಹೋದ_fraction = lfExactlyZero;
    APInt::tc_ಹೊರತೆಗೆ(dst, dst_ಎಣಿಕೆ, src, omsb, 0);
  }

  ಹಿಂತಿರುಗಿ normalize(rounding_ಬಹುಲಕ_2, ಕಳೆದುಹೋದ_fraction);
}

APFloat::op_ಸ್ಥಿತಿ IEEEFloat::convertFromAPInt(ಸ್ಥಿರಾಂಕ APInt &Val, ತಾರ್ಕಿಕ isSigned,
                                              rounding_ಬಹುಲಕ rounding_ಬಹುಲಕ_2) {
  ಚಿಹ್ನೆರಹಿತ ಪೂರ್ಣಾಂಕ part_ಎಣಿಕೆ = Val.get_ಅಂಕೆ_ಪದಗಳು();
  APInt api = Val;

  sign = ಸುಳ್ಳು;
  ಆದರೆ (isSigned && api.isNegative()) {
    sign = ನಿಜ;
    api = -api;
  }

  ಹಿಂತಿರುಗಿ convertFromUnsignedParts(api.get_Raw_ದತ್ತಾಂಶ(), part_ಎಣಿಕೆ, rounding_ಬಹುಲಕ_2);
}

Expected<APFloat::op_ಸ್ಥಿತಿ>
IEEEFloat::convertFromHexadecimalString(StringRef s,
                                        rounding_ಬಹುಲಕ rounding_ಬಹುಲಕ_2) {
  ಕಳೆದುಹೋದ_Fraction ಕಳೆದುಹೋದ_fraction = lfExactlyZero;

  category = fc_ಸಾಮಾನ್ಯ;
  zeroSignificand();
  exponent = 0;

  integerPart *significand = significandParts();
  ಚಿಹ್ನೆರಹಿತ parts_ಎಣಿಕೆ = part_ಎಣಿಕೆ();
  ಚಿಹ್ನೆರಹಿತ ಬಿಟ್_Pos = parts_ಎಣಿಕೆ * integer_Part_ಅಗಲ;
  ತಾರ್ಕಿಕ computedTrailingFraction = ಸುಳ್ಳು;

  // Skip leading zeroes and any (hexa)decimal point.
  StringRef::iterator begin = s.begin();
  StringRef::iterator ಕೊನೆ = s.ಕೊನೆ();
  StringRef::iterator dot;
  ಸ್ವಯಂಚಾಲಿತ PtrOrErr = skipLeadingZeroesAndAnyDot(begin, ಕೊನೆ, &dot);
  ಆದರೆ (!PtrOrErr)
    ಹಿಂತಿರುಗಿ PtrOrErr.takeError();
  StringRef::iterator p = *PtrOrErr;
  StringRef::iterator ಮೊದಲ_Significant_Digit = p;

  ಆಗ (p != ಕೊನೆ) {
    integerPart hex_ಮೌಲ್ಯ;

    ಆದರೆ (*p == '.') {
      ಆದರೆ (dot != ಕೊನೆ)
        ಹಿಂತಿರುಗಿ createError("String contains multiple dots");
      dot = p++;
      ಮುಂದುವರಿ;
    }

    hex_ಮೌಲ್ಯ = hex_Digit_ಮೌಲ್ಯ(*p);
    ಆದರೆ (hex_ಮೌಲ್ಯ == UINT_MAX)
      ನಿಲ್ಲಿಸು;

    p++;

    // Store the number while we have space.
    ಆದರೆ (ಬಿಟ್_Pos) {
      ಬಿಟ್_Pos -= 4;
      hex_ಮೌಲ್ಯ <<= ಬಿಟ್_Pos % integer_Part_ಅಗಲ;
      significand[ಬಿಟ್_Pos / integer_Part_ಅಗಲ] |= hex_ಮೌಲ್ಯ;
    } ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (!computedTrailingFraction) {
      ಸ್ವಯಂಚಾಲಿತ FractOrErr = trailingHexadecimalFraction(p, ಕೊನೆ, hex_ಮೌಲ್ಯ);
      ಆದರೆ (!FractOrErr)
        ಹಿಂತಿರುಗಿ FractOrErr.takeError();
      ಕಳೆದುಹೋದ_fraction = *FractOrErr;
      computedTrailingFraction = ನಿಜ;
    }
  }

  /* Hex floats require an exponent but not a hexadecimal point.  */
  ಆದರೆ (p == ಕೊನೆ)
    ಹಿಂತಿರುಗಿ createError("Hex strings require an exponent");
  ಆದರೆ (*p != 'p' && *p != 'P')
    ಹಿಂತಿರುಗಿ createError("Invalid character in significand");
  ಆದರೆ (p == begin)
    ಹಿಂತಿರುಗಿ createError("Significand has no digits");
  ಆದರೆ (dot != ಕೊನೆ && p - begin == 1)
    ಹಿಂತಿರುಗಿ createError("Significand has no digits");

  /* Ignore the exponent if we are zero.  */
  ಆದರೆ (p != ಮೊದಲ_Significant_Digit) {
    ಪೂರ್ಣಾಂಕ expAdjustment;

    /* Implicit hexadecimal point?  */
    ಆದರೆ (dot == ಕೊನೆ)
      dot = p;

    /* Calculate the exponent adjustment implicit in the number of
       significant digits.  */
    expAdjustment = static_cast<ಪೂರ್ಣಾಂಕ>(dot - ಮೊದಲ_Significant_Digit);
    ಆದರೆ (expAdjustment < 0)
      expAdjustment++;
    expAdjustment = expAdjustment * 4 - 1;

    /* Adjust for writing the significand starting at the most
       significant nibble.  */
    expAdjustment += semantics->precision;
    expAdjustment -= parts_ಎಣಿಕೆ * integer_Part_ಅಗಲ;

    /* Adjust for the given exponent.  */
    ಸ್ವಯಂಚಾಲಿತ ExpOrErr = ಒಟ್ಟು_Exponent(p + 1, ಕೊನೆ, expAdjustment);
    ಆದರೆ (!ExpOrErr)
      ಹಿಂತಿರುಗಿ ExpOrErr.takeError();
    exponent = *ExpOrErr;
  }

  ಹಿಂತಿರುಗಿ normalize(rounding_ಬಹುಲಕ_2, ಕಳೆದುಹೋದ_fraction);
}

APFloat::op_ಸ್ಥಿತಿ
IEEEFloat::ಸುತ್ತು_Significand_With_Exponent(ಸ್ಥಿರಾಂಕ integerPart *decSigParts,
                                        ಚಿಹ್ನೆರಹಿತ sig_Part_ಎಣಿಕೆ, ಪೂರ್ಣಾಂಕ ಘಾತಾಂಕ,
                                        rounding_ಬಹುಲಕ rounding_ಬಹುಲಕ_2) {
  fltSemantics calcSemantics = { 32767, -32767, 0, 0 };
  ಚಿಹ್ನೆರಹಿತ ಪೂರ್ಣಾಂಕ power = ಘಾತಾಂಕ >= 0 ? ಘಾತಾಂಕ : -ಘಾತಾಂಕ;
  ಚಿಹ್ನೆರಹಿತ ಪೂರ್ಣಾಂಕ pow5_Part_ಸಾಮರ್ಥ್ಯ = power_ದ_Five_Parts_For_Exponent(power);
  Small_ಸದಿಶ<integerPart, ಅಂಕೆ_Pow5_Parts_On_ರಾಶಿ> pow5Parts;
  pow5Parts.resize_for_overwrite(pow5_Part_ಸಾಮರ್ಥ್ಯ);

  ತಾರ್ಕಿಕ is_ಹತ್ತಿರದ = rounding_ಬಹುಲಕ_2 == rm_ಹತ್ತಿರದ_Ties_ಗಮ್ಯ_Even ||
                   rounding_ಬಹುಲಕ_2 == rm_ಹತ್ತಿರದ_Ties_ಗಮ್ಯ_Away;

  ಚಿಹ್ನೆರಹಿತ parts = part_ಎಣಿಕೆ_For_ಬಿಟ್ಟುಗಳು(semantics->precision + 11);

  /* Calculate pow(5, abs(exp)).  */
  ಚಿಹ್ನೆರಹಿತ pow5_Part_ಎಣಿಕೆ = powerOf5(pow5Parts.ದತ್ತಾಂಶ(), power);

  ಗಾಗಿ (;; parts *= 2) {
    ಚಿಹ್ನೆರಹಿತ ಪೂರ್ಣಾಂಕ excessPrecision, truncated_ಬಿಟ್ಟುಗಳು;

    calcSemantics.precision = parts * integer_Part_ಅಗಲ - 1;
    excessPrecision = calcSemantics.precision - semantics->precision;
    truncated_ಬಿಟ್ಟುಗಳು = excessPrecision;

    IEEEFloat decSig(calcSemantics, uninitialized);
    decSig.makeZero(sign);
    IEEEFloat pow5(calcSemantics);

    op_ಸ್ಥಿತಿ sig_ಸ್ಥಿತಿ = decSig.convertFromUnsignedParts(
        decSigParts, sig_Part_ಎಣಿಕೆ, rm_ಹತ್ತಿರದ_Ties_ಗಮ್ಯ_Even);
    op_ಸ್ಥಿತಿ pow_ಸ್ಥಿತಿ = pow5.convertFromUnsignedParts(
        pow5Parts.ದತ್ತಾಂಶ(), pow5_Part_ಎಣಿಕೆ, rm_ಹತ್ತಿರದ_Ties_ಗಮ್ಯ_Even);
    /* Add exp, as 10^n = 5^n * 2^n.  */
    decSig.exponent += ಘಾತಾಂಕ;

    ಕಳೆದುಹೋದ_Fraction calc_ಕಳೆದುಹೋದ_Fraction;
    integerPart HUerr, HUdistance;
    ಚಿಹ್ನೆರಹಿತ ಪೂರ್ಣಾಂಕ powHUerr;

    ಆದರೆ (ಘಾತಾಂಕ >= 0) {
      /* multiplySignificand leaves the precision-th bit set to 1.  */
      calc_ಕಳೆದುಹೋದ_Fraction = decSig.multiplySignificand(pow5);
      powHUerr = pow_ಸ್ಥಿತಿ != opOK;
    } ಇಲ್ಲದಿದ್ದರೆ {
      calc_ಕಳೆದುಹೋದ_Fraction = decSig.divideSignificand(pow5);
      /* Denormal numbers have less precision.  */
      ಆದರೆ (decSig.exponent < semantics->minExponent) {
        excessPrecision += (semantics->minExponent - decSig.exponent);
        truncated_ಬಿಟ್ಟುಗಳು = excessPrecision;
        excessPrecision = std::min(excessPrecision, calcSemantics.precision);
      }
      /* Extra half-ulp lost in reciprocal of exponent.  */
      powHUerr = (pow_ಸ್ಥಿತಿ == opOK && calc_ಕಳೆದುಹೋದ_Fraction == lfExactlyZero) ? 0:2;
    }

    /* Both multiplySignificand and divideSignificand return the
       result with the integer bit set.  */
    ಪ್ರತಿಜ್ಞಾ(APInt::tc_ಹೊರತೆಗೆ_ಬಿಟ್
           (decSig.significandParts(), calcSemantics.precision - 1) == 1);

    HUerr = HUerrBound(calc_ಕಳೆದುಹೋದ_Fraction != lfExactlyZero, sig_ಸ್ಥಿತಿ != opOK,
                       powHUerr);
    HUdistance = 2 * ulpsFromBoundary(decSig.significandParts(),
                                      excessPrecision, is_ಹತ್ತಿರದ);

    /* Are we guaranteed to round correctly if we truncate?  */
    ಆದರೆ (HUdistance >= HUerr) {
      APInt::tc_ಹೊರತೆಗೆ(significandParts(), part_ಎಣಿಕೆ(), decSig.significandParts(),
                       calcSemantics.precision - excessPrecision,
                       excessPrecision);
      /* Take the exponent of decSig.  If we tcExtract-ed less bits
         above we must adjust our exponent to compensate for the
         implicit right shift.  */
      exponent = (decSig.exponent + semantics->precision
                  - (calcSemantics.precision - excessPrecision));
      calc_ಕಳೆದುಹೋದ_Fraction = ಕಳೆದುಹೋದ_Fraction_Through_Truncation(decSig.significandParts(),
                                                       decSig.part_ಎಣಿಕೆ(),
                                                       truncated_ಬಿಟ್ಟುಗಳು);
      ಹಿಂತಿರುಗಿ static_cast<op_ಸ್ಥಿತಿ>(normalize(rounding_ಬಹುಲಕ_2, calc_ಕಳೆದುಹೋದ_Fraction) |
                                   ((sig_ಸ್ಥಿತಿ | pow_ಸ್ಥಿತಿ) & opInexact));
    }
  }
}

Expected<APFloat::op_ಸ್ಥಿತಿ>
IEEEFloat::convertFromDecimalString(StringRef str, rounding_ಬಹುಲಕ rounding_ಬಹುಲಕ_2) {
  decimalInfo D;
  op_ಸ್ಥಿತಿ fs;

  /* Scan the text.  */
  StringRef::iterator p = str.begin();
  ಆದರೆ (Error Err = interpretDecimal(p, str.ಕೊನೆ(), &D))
    ಹಿಂತಿರುಗಿ std::move(Err);

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
  ಆದರೆ (D.ಮೊದಲ_Sig_Digit == str.ಕೊನೆ() || dec_Digit_ಮೌಲ್ಯ(*D.ಮೊದಲ_Sig_Digit) >= 10U) {
    category = fcZero;
    fs = opOK;
    ಆದರೆ (semantics->nanEncoding == fltNanEncoding::NegativeZero)
      sign = ಸುಳ್ಳು;
    ಆದರೆ (!semantics->hasZero)
      make_ಅತಿಚಿಕ್ಕ_Normalized(ಸುಳ್ಳು);

    /* Check whether the normalized exponent is high enough to overflow
       max during the log-rebasing in the max-exponent check below. */
  } ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (D.normalizedExponent - 1 > ಗರಿಷ್ಠ_ಪೂರ್ಣಾಂಕ / 42039) {
    fs = handle_ಅತಿಪ್ರವಾಹ(rounding_ಬಹುಲಕ_2);

  /* If it wasn't, then it also wasn't high enough to overflow max
     during the log-rebasing in the min-exponent check.  Check that it
     won't overflow min in either check, then perform the min-exponent
     check. */
  } ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (D.normalizedExponent - 1 < INT_MIN / 42039 ||
             (D.normalizedExponent + 1) * 28738 <=
               8651 * (semantics->minExponent - (ಪೂರ್ಣಾಂಕ) semantics->precision)) {
    /* Underflow to zero and round.  */
    category = fc_ಸಾಮಾನ್ಯ;
    zeroSignificand();
    fs = normalize(rounding_ಬಹುಲಕ_2, lfLessThanHalf);

  /* We can finally safely perform the max-exponent check. */
  } ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ ((D.normalizedExponent - 1) * 42039
             >= 12655 * semantics->maxExponent) {
    /* Overflow and round.  */
    fs = handle_ಅತಿಪ್ರವಾಹ(rounding_ಬಹುಲಕ_2);
  } ಇಲ್ಲದಿದ್ದರೆ {
    integerPart *decSignificand;
    ಚಿಹ್ನೆರಹಿತ ಪೂರ್ಣಾಂಕ part_ಎಣಿಕೆ;

    /* A tight upper bound on number of bits required to hold an
       N-digit decimal integer is N * 196 / 59.  Allocate enough space
       to hold the full significand, and an extra part required by
       tcMultiplyPart.  */
    part_ಎಣಿಕೆ = static_cast<ಚಿಹ್ನೆರಹಿತ ಪೂರ್ಣಾಂಕ>(D.ಕೊನೆಯ_Sig_Digit - D.ಮೊದಲ_Sig_Digit) + 1;
    part_ಎಣಿಕೆ = part_ಎಣಿಕೆ_For_ಬಿಟ್ಟುಗಳು(1 + 196 * part_ಎಣಿಕೆ / 59);
    decSignificand = ಹೊಸ integerPart[part_ಎಣಿಕೆ + 1];
    part_ಎಣಿಕೆ = 0;

    /* Convert to binary efficiently - we do almost all multiplication
       in an integerPart.  When this would overflow do we do a single
       bignum multiplication, and then revert again to multiplication
       in an integerPart.  */
    ಮಾಡು {
      integerPart dec_ಮೌಲ್ಯ, val, multiplier;

      val = 0;
      multiplier = 1;

      ಮಾಡು {
        ಆದರೆ (*p == '.') {
          p++;
          ಆದರೆ (p == str.ಕೊನೆ()) {
            ನಿಲ್ಲಿಸು;
          }
        }
        dec_ಮೌಲ್ಯ = dec_Digit_ಮೌಲ್ಯ(*p++);
        ಆದರೆ (dec_ಮೌಲ್ಯ >= 10U) {
          ಅಳಿಸು[] decSignificand;
          ಹಿಂತಿರುಗಿ createError("Invalid character in significand");
        }
        multiplier *= 10;
        val = val * 10 + dec_ಮೌಲ್ಯ;
        /* The maximum number that can be multiplied by ten with any
           digit added without overflowing an integerPart.  */
      } ಆಗ (p <= D.ಕೊನೆಯ_Sig_Digit && multiplier <= (~ (integerPart) 0 - 9) / 10);

      /* Multiply out the current part.  */
      APInt::tcMultiplyPart(decSignificand, decSignificand, multiplier, val,
                            part_ಎಣಿಕೆ, part_ಎಣಿಕೆ + 1, ಸುಳ್ಳು);

      /* If we used another part (likely but not guaranteed), increase
         the count.  */
      ಆದರೆ (decSignificand[part_ಎಣಿಕೆ])
        part_ಎಣಿಕೆ++;
    } ಆಗ (p <= D.ಕೊನೆಯ_Sig_Digit);

    category = fc_ಸಾಮಾನ್ಯ;
    fs = ಸುತ್ತು_Significand_With_Exponent(decSignificand, part_ಎಣಿಕೆ,
                                      D.exponent, rounding_ಬಹುಲಕ_2);

    ಅಳಿಸು [] decSignificand;
  }

  ಹಿಂತಿರುಗಿ fs;
}

ತಾರ್ಕಿಕ IEEEFloat::convertFromStringSpecials(StringRef str) {
  ಸ್ಥಿರಾಂಕ size_t MIN_NAME_ಗಾತ್ರ = 3;

  ಆದರೆ (str.ಗಾತ್ರ_3() < MIN_NAME_ಗಾತ್ರ)
    ಹಿಂತಿರುಗಿ ಸುಳ್ಳು;

  ಆದರೆ (str == "inf" || str == "INFINITY" || str == "+Inf" || str == "+inf") {
    makeInf(ಸುಳ್ಳು);
    ಹಿಂತಿರುಗಿ ನಿಜ;
  }

  ತಾರ್ಕಿಕ IsNegative = str.consume_front("-");
  ಆದರೆ (IsNegative) {
    ಆದರೆ (str.ಗಾತ್ರ_3() < MIN_NAME_ಗಾತ್ರ)
      ಹಿಂತಿರುಗಿ ಸುಳ್ಳು;

    ಆದರೆ (str == "inf" || str == "INFINITY" || str == "Inf") {
      makeInf(ನಿಜ);
      ಹಿಂತಿರುಗಿ ನಿಜ;
    }
  }

  // If we have a 's' (or 'S') prefix, then this is a Signaling NaN.
  ತಾರ್ಕಿಕ IsSignaling = str.consume_front_insensitive("s");
  ಆದರೆ (IsSignaling) {
    ಆದರೆ (str.ಗಾತ್ರ_3() < MIN_NAME_ಗಾತ್ರ)
      ಹಿಂತಿರುಗಿ ಸುಳ್ಳು;
  }

  ಆದರೆ (str.consume_front("nan") || str.consume_front("NaN")) {
    // A NaN without payload.
    ಆದರೆ (str.ಖಾಲಿಗಳು()) {
      makeNaN(IsSignaling, IsNegative);
      ಹಿಂತಿರುಗಿ ನಿಜ;
    }

    // Allow the payload to be inside parentheses.
    ಆದರೆ (str.front() == '(') {
      // Parentheses should be balanced (and not empty).
      ಆದರೆ (str.ಗಾತ್ರ_3() <= 2 || str.ಹಿಂಭಾಗ() != ')')
        ಹಿಂತಿರುಗಿ ಸುಳ್ಳು;

      str = str.ತುಂಡು(1, str.ಗಾತ್ರ_3() - 1);
    }

    // Determine the payload number's radix.
    ಚಿಹ್ನೆರಹಿತ Radix = 10;
    ಆದರೆ (str[0] == '0') {
      ಆದರೆ (str.ಗಾತ್ರ_3() > 1 && tolower(str[1]) == 'x') {
        str = str.drop_front(2);
        Radix = 16;
      } ಇಲ್ಲದಿದ್ದರೆ {
        Radix = 8;
      }
    }

    // Parse the payload and make the NaN.
    APInt Payload;
    ಆದರೆ (!str.getAsInteger(Radix, Payload)) {
      makeNaN(IsSignaling, IsNegative, &Payload);
      ಹಿಂತಿರುಗಿ ನಿಜ;
    }
  }

  ಹಿಂತಿರುಗಿ ಸುಳ್ಳು;
}

Expected<APFloat::op_ಸ್ಥಿತಿ>
IEEEFloat::convertFromString(StringRef str, rounding_ಬಹುಲಕ rounding_ಬಹುಲಕ_2) {
  ಆದರೆ (str.ಖಾಲಿಗಳು())
    ಹಿಂತಿರುಗಿ createError("Invalid string length");

  // Handle special cases.
  ಆದರೆ (convertFromStringSpecials(str))
    ಹಿಂತಿರುಗಿ opOK;

  /* Handle a leading minus sign.  */
  StringRef::iterator p = str.begin();
  size_t slen = str.ಗಾತ್ರ_3();
  sign = *p == '-' ? 1 : 0;
  ಆದರೆ (sign && !semantics->hasSignedRepr)
    llvm_unreachable(
        "This floating point format does not support signed values");

  ಆದರೆ (*p == '-' || *p == '+') {
    p++;
    slen--;
    ಆದರೆ (!slen)
      ಹಿಂತಿರುಗಿ createError("String has no digits");
  }

  ಆದರೆ (slen >= 2 && p[0] == '0' && (p[1] == 'x' || p[1] == 'X')) {
    ಆದರೆ (slen == 2)
      ಹಿಂತಿರುಗಿ createError("Invalid string");
    ಹಿಂತಿರುಗಿ convertFromHexadecimalString(StringRef(p + 2, slen - 2),
                                        rounding_ಬಹುಲಕ_2);
  }

  ಹಿಂತಿರುಗಿ convertFromDecimalString(StringRef(p, slen), rounding_ಬಹುಲಕ_2);
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
ಚಿಹ್ನೆರಹಿತ ಪೂರ್ಣಾಂಕ IEEEFloat::convert_ಗಮ್ಯ_Hex_String(ಅಕ್ಷರ *dst, ಚಿಹ್ನೆರಹಿತ ಪೂರ್ಣಾಂಕ hexDigits,
                                           ತಾರ್ಕಿಕ ಊರ್ಧ್ವ_Case,
                                           rounding_ಬಹುಲಕ rounding_ಬಹುಲಕ_2) ಸ್ಥಿರಾಂಕ {
  ಅಕ್ಷರ *p = dst;
  ಆದರೆ (sign)
    *dst++ = '-';

  ಆಯ್ಕೆ (category) {
  ಸ್ಥಿತಿ fcInfinity:
    ಸ್ಮರಣೆ_ನಕಲು (dst, ಊರ್ಧ್ವ_Case ? infinityU: infinityL, ಗಾತ್ರ infinityU - 1);
    dst += ಗಾತ್ರ infinityL - 1;
    ನಿಲ್ಲಿಸು;

  ಸ್ಥಿತಿ fcNaN:
    ಸ್ಮರಣೆ_ನಕಲು (dst, ಊರ್ಧ್ವ_Case ? NaNU: NaNL, ಗಾತ್ರ NaNU - 1);
    dst += ಗಾತ್ರ NaNU - 1;
    ನಿಲ್ಲಿಸು;

  ಸ್ಥಿತಿ fcZero:
    *dst++ = '0';
    *dst++ = ಊರ್ಧ್ವ_Case ? 'X': 'x';
    *dst++ = '0';
    ಆದರೆ (hexDigits > 1) {
      *dst++ = '.';
      memset (dst, '0', hexDigits - 1);
      dst += hexDigits - 1;
    }
    *dst++ = ಊರ್ಧ್ವ_Case ? 'P': 'p';
    *dst++ = '0';
    ನಿಲ್ಲಿಸು;

  ಸ್ಥಿತಿ fc_ಸಾಮಾನ್ಯ:
    dst = convert_ಸಾಮಾನ್ಯ_ಗಮ್ಯ_Hex_String (dst, hexDigits, ಊರ್ಧ್ವ_Case, rounding_ಬಹುಲಕ_2);
    ನಿಲ್ಲಿಸು;
  }

  *dst = 0;

  ಹಿಂತಿರುಗಿ static_cast<ಚಿಹ್ನೆರಹಿತ ಪೂರ್ಣಾಂಕ>(dst - p);
}

/* Does the hard work of outputting the correctly rounded hexadecimal
   form of a normal floating point number with the specified number of
   hexadecimal digits.  If HEXDIGITS is zero the minimum number of
   digits necessary to print the value precisely is output.  */
ಅಕ್ಷರ *IEEEFloat::convert_ಸಾಮಾನ್ಯ_ಗಮ್ಯ_Hex_String(ಅಕ್ಷರ *dst, ಚಿಹ್ನೆರಹಿತ ಪೂರ್ಣಾಂಕ hexDigits,
                                          ತಾರ್ಕಿಕ ಊರ್ಧ್ವ_Case,
                                          rounding_ಬಹುಲಕ rounding_ಬಹುಲಕ_2) ಸ್ಥಿರಾಂಕ {
  *dst++ = '0';
  *dst++ = ಊರ್ಧ್ವ_Case ? 'X': 'x';

  ತಾರ್ಕಿಕ ಸುತ್ತು_ಮೇಲೆ = ಸುಳ್ಳು;
  ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *hexDigitChars = ಊರ್ಧ್ವ_Case ? hex_Digits_ಊರ್ಧ್ವ : hex_Digits_ಅಧೋ;

  ಸ್ಥಿರಾಂಕ integerPart *significand = significandParts();
  ಚಿಹ್ನೆರಹಿತ parts_ಎಣಿಕೆ = part_ಎಣಿಕೆ();

  /* +3 because the first digit only uses the single integer bit, so
     we have 3 virtual zero most-significant-bits.  */
  ಚಿಹ್ನೆರಹಿತ ಮೌಲ್ಯ_ಬಿಟ್ಟುಗಳು = semantics->precision + 3;
  ಚಿಹ್ನೆರಹಿತ ಸರಿಸು = integer_Part_ಅಗಲ - ಮೌಲ್ಯ_ಬಿಟ್ಟುಗಳು % integer_Part_ಅಗಲ;

  /* The natural number of digits required ignoring trailing
     insignificant zeroes.  */
  ಚಿಹ್ನೆರಹಿತ outputDigits = (ಮೌಲ್ಯ_ಬಿಟ್ಟುಗಳು - significandLSB() + 3) / 4;

  /* hexDigits of zero means use the required number for the
     precision.  Otherwise, see if we are truncating.  If we are,
     find out if we need to round away from zero.  */
  ಆದರೆ (hexDigits) {
    ಆದರೆ (hexDigits < outputDigits) {
      /* We are dropping non-zero bits, so need to check how to round.
         "bits" is the number of dropped bits.  */
      ಚಿಹ್ನೆರಹಿತ ಪೂರ್ಣಾಂಕ ಬಿಟ್ಟುಗಳು;
      ಕಳೆದುಹೋದ_Fraction fraction;

      ಬಿಟ್ಟುಗಳು = ಮೌಲ್ಯ_ಬಿಟ್ಟುಗಳು - hexDigits * 4;
      fraction = ಕಳೆದುಹೋದ_Fraction_Through_Truncation (significand, parts_ಎಣಿಕೆ, ಬಿಟ್ಟುಗಳು);
      ಸುತ್ತು_ಮೇಲೆ = ಸುತ್ತು_Away_From_Zero(rounding_ಬಹುಲಕ_2, fraction, ಬಿಟ್ಟುಗಳು);
    }
    outputDigits = hexDigits;
  }

  /* Write the digits consecutively, and start writing in the location
     of the hexadecimal point.  We move the most significant digit
     left and add the hexadecimal point later.  */
  ಅಕ್ಷರ *p = ++dst;

  ಚಿಹ್ನೆರಹಿತ ಎಣಿಕೆ_2 = (ಮೌಲ್ಯ_ಬಿಟ್ಟುಗಳು + integer_Part_ಅಗಲ - 1) / integer_Part_ಅಗಲ;

  ಆಗ (outputDigits && ಎಣಿಕೆ_2) {
    integerPart part;

    /* Put the most significant integerPartWidth bits in "part".  */
    ಆದರೆ (--ಎಣಿಕೆ_2 == parts_ಎಣಿಕೆ)
      part = 0;  /* An imaginary higher zero part.  */
    ಇಲ್ಲದಿದ್ದರೆ
      part = significand[ಎಣಿಕೆ_2] << ಸರಿಸು;

    ಆದರೆ (ಎಣಿಕೆ_2 && ಸರಿಸು)
      part |= significand[ಎಣಿಕೆ_2 - 1] >> (integer_Part_ಅಗಲ - ಸರಿಸು);

    /* Convert as much of "part" to hexdigits as we can.  */
    ಚಿಹ್ನೆರಹಿತ ಪೂರ್ಣಾಂಕ curDigits = integer_Part_ಅಗಲ / 4;

    curDigits = std::min(curDigits, outputDigits);
    dst += partAsHex (dst, part, curDigits, hexDigitChars);
    outputDigits -= curDigits;
  }

  ಆದರೆ (ಸುತ್ತು_ಮೇಲೆ) {
    ಅಕ್ಷರ *q = dst;

    /* Note that hexDigitChars has a trailing '0'.  */
    ಮಾಡು {
      q--;
      *q = hexDigitChars[hex_Digit_ಮೌಲ್ಯ (*q) + 1];
    } ಆಗ (*q == '0');
    ಪ್ರತಿಜ್ಞಾ(q >= p);
  } ಇಲ್ಲದಿದ್ದರೆ {
    /* Add trailing zeroes.  */
    memset (dst, '0', outputDigits);
    dst += outputDigits;
  }

  /* Move the most significant digit to before the point, and if there
     is something after the decimal point add it.  This must come
     after rounding above.  */
  p[-1] = p[0];
  ಆದರೆ (dst -1 == p)
    dst--;
  ಇಲ್ಲದಿದ್ದರೆ
    p[0] = '.';

  /* Finally output the exponent.  */
  *dst++ = ಊರ್ಧ್ವ_Case ? 'P': 'p';

  ಹಿಂತಿರುಗಿ writeSignedDecimal (dst, exponent);
}

hash_ಸಂಕೇತ hash_ಮೌಲ್ಯ(ಸ್ಥಿರಾಂಕ IEEEFloat &Arg) {
  ಆದರೆ (!Arg.isFiniteNonZero())
    ಹಿಂತಿರುಗಿ hash_combine((uint8_t)Arg.category,
                        // NaN has no sign, fix it at zero.
                        Arg.isNaN() ? (uint8_t)0 : (uint8_t)Arg.sign,
                        Arg.semantics->precision);

  // Normal floats need their exponent and significand hashed.
  ಹಿಂತಿರುಗಿ hash_combine((uint8_t)Arg.category, (uint8_t)Arg.sign,
                      Arg.semantics->precision, Arg.exponent,
                      hash_combine_range(
                        Arg.significandParts(),
                        Arg.significandParts() + Arg.part_ಎಣಿಕೆ()));
}

// Conversion from APFloat to/from host float/double.  It may eventually be
// possible to eliminate these and have everybody deal with APFloats, but that
// will take a while.  This approach will not easily extend to long double.
// Current implementation requires integerPartWidth==64, which is correct at
// the moment but could be made more general.

// Denormals have exponent minExponent in APFloat, but minExponent-1 in
// the actual IEEE respresentations.  We compensate for that here.

APInt IEEEFloat::convert_F80_Long_Double_APFloat_ಗಮ್ಯ_APInt() ಸ್ಥಿರಾಂಕ {
  ಪ್ರತಿಜ್ಞಾ(part_ಎಣಿಕೆ() == 2);
  ಹಿಂತಿರುಗಿ convert_IEEEFloat_ಗಮ್ಯ_APInt<APFloat_ತಳ::semX87DoubleExtended>();
}

APInt IEEEFloat::convert_PPCDouble_Double_Legacy_APFloat_ಗಮ್ಯ_APInt() ಸ್ಥಿರಾಂಕ {
  ಪ್ರತಿಜ್ಞಾ(semantics ==
         (ಸ್ಥಿರಾಂಕ llvm::fltSemantics *)&APFloat_ತಳ::semPPCDoubleDoubleLegacy);
  ಪ್ರತಿಜ್ಞಾ(part_ಎಣಿಕೆ()==2);

  uint64_t ಪದಗಳು[2];
  ತಾರ್ಕಿಕ losesInfo;

  // Convert number to double.  To avoid spurious underflows, we re-
  // normalize against the "double" minExponent first, and only *then*
  // truncate the mantissa.  The result of that second conversion
  // may be inexact, but should never underflow.
  // Declare fltSemantics before APFloat that uses it (and
  // saves pointer to it) to ensure correct destruction order.
  fltSemantics extendedSemantics = *semantics;
  extendedSemantics.minExponent = APFloat_ತಳ::semIEEEdouble.minExponent;
  IEEEFloat extended(*ಇದು);
  [[maybe_ಬಳಸದ]] op_ಸ್ಥಿತಿ fs =
      extended.convert(extendedSemantics, rm_ಹತ್ತಿರದ_Ties_ಗಮ್ಯ_Even, &losesInfo);
  ಪ್ರತಿಜ್ಞಾ(fs == opOK && !losesInfo);

  IEEEFloat u(extended);
  fs = u.convert(APFloat_ತಳ::semIEEEdouble, rm_ಹತ್ತಿರದ_Ties_ಗಮ್ಯ_Even, &losesInfo);
  ಪ್ರತಿಜ್ಞಾ(fs == opOK || fs == opInexact);
  ಪದಗಳು[0] = *u.convert_Double_APFloat_ಗಮ್ಯ_APInt().get_Raw_ದತ್ತಾಂಶ();

  // If conversion was exact or resulted in a special case, we're done;
  // just set the second double to zero.  Otherwise, re-convert back to
  // the extended format and compute the difference.  This now should
  // convert exactly to double.
  ಆದರೆ (u.isFiniteNonZero() && losesInfo) {
    fs = u.convert(extendedSemantics, rm_ಹತ್ತಿರದ_Ties_ಗಮ್ಯ_Even, &losesInfo);
    ಪ್ರತಿಜ್ಞಾ(fs == opOK && !losesInfo);

    IEEEFloat v(extended);
    v.ಕಳೆ(u, rm_ಹತ್ತಿರದ_Ties_ಗಮ್ಯ_Even);
    fs = v.convert(APFloat_ತಳ::semIEEEdouble, rm_ಹತ್ತಿರದ_Ties_ಗಮ್ಯ_Even, &losesInfo);
    ಪ್ರತಿಜ್ಞಾ(fs == opOK && !losesInfo);
    ಪದಗಳು[1] = *v.convert_Double_APFloat_ಗಮ್ಯ_APInt().get_Raw_ದತ್ತಾಂಶ();
  } ಇಲ್ಲದಿದ್ದರೆ {
    ಪದಗಳು[1] = 0;
  }

  ಹಿಂತಿರುಗಿ APInt(128, ಪದಗಳು);
}

ಮಾದರಿ <ಸ್ಥಿರಾಂಕ fltSemantics &S>
APInt IEEEFloat::convert_IEEEFloat_ಗಮ್ಯ_APInt() ಸ್ಥಿರಾಂಕ {
  ಪ್ರತಿಜ್ಞಾ(semantics == &S);
  constexpr ಚಿಹ್ನೆರಹಿತ ಪೂರ್ಣಾಂಕ trailing_significand_ಬಿಟ್ಟುಗಳು =
      S.precision - 1 + S.has_Explicit_Integer_ಬಿಟ್;
  constexpr ಪೂರ್ಣಾಂಕ integer_ಬಿಟ್_part = (S.precision - 1) / integer_Part_ಅಗಲ;
  constexpr integerPart integer_ಬಿಟ್ = integerPart{1}
                                      << ((S.precision - 1) % integer_Part_ಅಗಲ);
  constexpr uint64_t significand_ಮುಖವಾಡ = integer_ಬಿಟ್ - 1;
  constexpr ಚಿಹ್ನೆರಹಿತ ಪೂರ್ಣಾಂಕ exponent_ಬಿಟ್ಟುಗಳು =
      S.ಗಾತ್ರ_ಒಳ_ಬಿಟ್ಟುಗಳು - (S.hasSignedRepr ? 1 : 0) - trailing_significand_ಬಿಟ್ಟುಗಳು;
  static_assert(exponent_ಬಿಟ್ಟುಗಳು < 64);
  constexpr uint64_t exponent_ಮುಖವಾಡ = (uint64_t{1} << exponent_ಬಿಟ್ಟುಗಳು) - 1;
  constexpr ತಾರ್ಕಿಕ is_zero_exp_reserved = S.hasDenormals || S.hasZero;
  constexpr ಪೂರ್ಣಾಂಕ bias = -(S.minExponent - (is_zero_exp_reserved ? 1 : 0));

  uint64_t myexponent;
  std::array<integerPart, part_ಎಣಿಕೆ_For_ಬಿಟ್ಟುಗಳು(trailing_significand_ಬಿಟ್ಟುಗಳು)>
      mysignificand;

  ಆದರೆ (isFiniteNonZero()) {
    myexponent = exponent + bias;
    std::copy_n(significandParts(), mysignificand.ಗಾತ್ರ_3(),
                mysignificand.begin());
    ಆದರೆ (myexponent == 1 &&
        !(significandParts()[integer_ಬಿಟ್_part] & integer_ಬಿಟ್))
      myexponent = 0; // denormal
  } ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (category == fcZero) {
    ಆದರೆ (!S.hasZero)
      llvm_unreachable("semantics does not support zero!");
    myexponent = ::exponentZero(S) + bias;
    mysignificand.fill(0);
  } ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (category == fcInfinity) {
    ಆದರೆ (S.nonFiniteBehavior == fltNonfiniteBehavior::NanOnly ||
        S.nonFiniteBehavior == fltNonfiniteBehavior::FiniteOnly)
      llvm_unreachable("semantics don't support inf!");
    myexponent = ::exponentInf(S) + bias;
    mysignificand.fill(0);
    ಆದರೆ constexpr (S.has_Explicit_Integer_ಬಿಟ್) {
      mysignificand[0] = integerPart{1} << (trailing_significand_ಬಿಟ್ಟುಗಳು - 1);
    }
  } ಇಲ್ಲದಿದ್ದರೆ {
    ಪ್ರತಿಜ್ಞಾ(category == fcNaN && "Unknown category!");
    ಆದರೆ (S.nonFiniteBehavior == fltNonfiniteBehavior::FiniteOnly)
      llvm_unreachable("semantics don't support NaN!");
    myexponent = ::exponentNaN(S) + bias;
    std::copy_n(significandParts(), mysignificand.ಗಾತ್ರ_3(),
                mysignificand.begin());
  }
  std::array<uint64_t, (S.ಗಾತ್ರ_ಒಳ_ಬಿಟ್ಟುಗಳು + 63) / 64> ಪದಗಳು;
  ಸ್ವಯಂಚಾಲಿತ ಪದಗಳು_iter =
      std::copy_n(mysignificand.begin(), mysignificand.ಗಾತ್ರ_3(), ಪದಗಳು.begin());
  ಆದರೆ constexpr (!S.has_Explicit_Integer_ಬಿಟ್) {
    ಆದರೆ constexpr (significand_ಮುಖವಾಡ != 0 || trailing_significand_ಬಿಟ್ಟುಗಳು == 0) {
      // Clear the integer bit.
      ಪದಗಳು[mysignificand.ಗಾತ್ರ_3() - 1] &= significand_ಮುಖವಾಡ;
    }
  }
  std::fill(ಪದಗಳು_iter, ಪದಗಳು.ಕೊನೆ(), uint64_t{0});
  constexpr size_t ಕೊನೆಯ_ಶಬ್ದ = ಪದಗಳು.ಗಾತ್ರ_3() - 1;
  ಆದರೆ constexpr (S.hasSignedRepr) {
    uint64_t shifted_sign = static_cast<uint64_t>(sign & 1)
                            << ((S.ಗಾತ್ರ_ಒಳ_ಬಿಟ್ಟುಗಳು - 1) % 64);
    ಪದಗಳು[ಕೊನೆಯ_ಶಬ್ದ] |= shifted_sign;
  }
  uint64_t shifted_exponent = (myexponent & exponent_ಮುಖವಾಡ)
                              << (trailing_significand_ಬಿಟ್ಟುಗಳು % 64);
  ಪದಗಳು[ಕೊನೆಯ_ಶಬ್ದ] |= shifted_exponent;
  ಆದರೆ constexpr (ಕೊನೆಯ_ಶಬ್ದ == 0) {
    ಹಿಂತಿರುಗಿ APInt(S.ಗಾತ್ರ_ಒಳ_ಬಿಟ್ಟುಗಳು, ಪದಗಳು[0]);
  }
  ಹಿಂತಿರುಗಿ APInt(S.ಗಾತ್ರ_ಒಳ_ಬಿಟ್ಟುಗಳು, ಪದಗಳು);
}

APInt IEEEFloat::convert_Quadruple_APFloat_ಗಮ್ಯ_APInt() ಸ್ಥಿರಾಂಕ {
  ಪ್ರತಿಜ್ಞಾ(part_ಎಣಿಕೆ() == 2);
  ಹಿಂತಿರುಗಿ convert_IEEEFloat_ಗಮ್ಯ_APInt<APFloat_ತಳ::semIEEEquad>();
}

APInt IEEEFloat::convert_Double_APFloat_ಗಮ್ಯ_APInt() ಸ್ಥಿರಾಂಕ {
  ಪ್ರತಿಜ್ಞಾ(part_ಎಣಿಕೆ()==1);
  ಹಿಂತಿರುಗಿ convert_IEEEFloat_ಗಮ್ಯ_APInt<APFloat_ತಳ::semIEEEdouble>();
}

APInt IEEEFloat::convert_Float_APFloat_ಗಮ್ಯ_APInt() ಸ್ಥಿರಾಂಕ {
  ಪ್ರತಿಜ್ಞಾ(part_ಎಣಿಕೆ()==1);
  ಹಿಂತಿರುಗಿ convert_IEEEFloat_ಗಮ್ಯ_APInt<APFloat_ತಳ::semIEEEsingle>();
}

APInt IEEEFloat::convert_BFloat_APFloat_ಗಮ್ಯ_APInt() ಸ್ಥಿರಾಂಕ {
  ಪ್ರತಿಜ್ಞಾ(part_ಎಣಿಕೆ() == 1);
  ಹಿಂತಿರುಗಿ convert_IEEEFloat_ಗಮ್ಯ_APInt<APFloat_ತಳ::semBFloat>();
}

APInt IEEEFloat::convert_Half_APFloat_ಗಮ್ಯ_APInt() ಸ್ಥಿರಾಂಕ {
  ಪ್ರತಿಜ್ಞಾ(part_ಎಣಿಕೆ()==1);
  ಹಿಂತಿರುಗಿ convert_IEEEFloat_ಗಮ್ಯ_APInt<APFloat_ತಳ::APFloat_ತಳ::semIEEEhalf>();
}

APInt IEEEFloat::convert_Float8_E5_M2_APFloat_ಗಮ್ಯ_APInt() ಸ್ಥಿರಾಂಕ {
  ಪ್ರತಿಜ್ಞಾ(part_ಎಣಿಕೆ() == 1);
  ಹಿಂತಿರುಗಿ convert_IEEEFloat_ಗಮ್ಯ_APInt<APFloat_ತಳ::semFloat8E5M2>();
}

APInt IEEEFloat::convert_Float8_E5_M2_FNUZAPFloat_ಗಮ್ಯ_APInt() ಸ್ಥಿರಾಂಕ {
  ಪ್ರತಿಜ್ಞಾ(part_ಎಣಿಕೆ() == 1);
  ಹಿಂತಿರುಗಿ convert_IEEEFloat_ಗಮ್ಯ_APInt<APFloat_ತಳ::semFloat8E5M2FNUZ>();
}

APInt IEEEFloat::convert_Float8_E4_M3_APFloat_ಗಮ್ಯ_APInt() ಸ್ಥಿರಾಂಕ {
  ಪ್ರತಿಜ್ಞಾ(part_ಎಣಿಕೆ() == 1);
  ಹಿಂತಿರುಗಿ convert_IEEEFloat_ಗಮ್ಯ_APInt<APFloat_ತಳ::semFloat8E4M3>();
}

APInt IEEEFloat::convert_Float8_E4_M3_FNAPFloat_ಗಮ್ಯ_APInt() ಸ್ಥಿರಾಂಕ {
  ಪ್ರತಿಜ್ಞಾ(part_ಎಣಿಕೆ() == 1);
  ಹಿಂತಿರುಗಿ convert_IEEEFloat_ಗಮ್ಯ_APInt<APFloat_ತಳ::semFloat8E4M3FN>();
}

APInt IEEEFloat::convert_Float8_E4_M3_FNUZAPFloat_ಗಮ್ಯ_APInt() ಸ್ಥಿರಾಂಕ {
  ಪ್ರತಿಜ್ಞಾ(part_ಎಣಿಕೆ() == 1);
  ಹಿಂತಿರುಗಿ convert_IEEEFloat_ಗಮ್ಯ_APInt<APFloat_ತಳ::semFloat8E4M3FNUZ>();
}

APInt IEEEFloat::convert_Float8_E4_M3_B11_FNUZAPFloat_ಗಮ್ಯ_APInt() ಸ್ಥಿರಾಂಕ {
  ಪ್ರತಿಜ್ಞಾ(part_ಎಣಿಕೆ() == 1);
  ಹಿಂತಿರುಗಿ convert_IEEEFloat_ಗಮ್ಯ_APInt<APFloat_ತಳ::semFloat8E4M3B11FNUZ>();
}

APInt IEEEFloat::convert_Float8_E3_M4_APFloat_ಗಮ್ಯ_APInt() ಸ್ಥಿರಾಂಕ {
  ಪ್ರತಿಜ್ಞಾ(part_ಎಣಿಕೆ() == 1);
  ಹಿಂತಿರುಗಿ convert_IEEEFloat_ಗಮ್ಯ_APInt<APFloat_ತಳ::semFloat8E3M4>();
}

APInt IEEEFloat::convert_Float_TF32_APFloat_ಗಮ್ಯ_APInt() ಸ್ಥಿರಾಂಕ {
  ಪ್ರತಿಜ್ಞಾ(part_ಎಣಿಕೆ() == 1);
  ಹಿಂತಿರುಗಿ convert_IEEEFloat_ಗಮ್ಯ_APInt<APFloat_ತಳ::semFloatTF32>();
}

APInt IEEEFloat::convert_Float8_E8_M0_FNUAPFloat_ಗಮ್ಯ_APInt() ಸ್ಥಿರಾಂಕ {
  ಪ್ರತಿಜ್ಞಾ(part_ಎಣಿಕೆ() == 1);
  ಹಿಂತಿರುಗಿ convert_IEEEFloat_ಗಮ್ಯ_APInt<APFloat_ತಳ::semFloat8E8M0FNU>();
}

APInt IEEEFloat::convert_Float8_E5_M3_FNUAPFloat_ಗಮ್ಯ_APInt() ಸ್ಥಿರಾಂಕ {
  ಪ್ರತಿಜ್ಞಾ(part_ಎಣಿಕೆ() == 1);
  ಹಿಂತಿರುಗಿ convert_IEEEFloat_ಗಮ್ಯ_APInt<APFloat_ತಳ::semFloat8E5M3FNU>();
}

APInt IEEEFloat::convert_Float6_E3_M2_FNAPFloat_ಗಮ್ಯ_APInt() ಸ್ಥಿರಾಂಕ {
  ಪ್ರತಿಜ್ಞಾ(part_ಎಣಿಕೆ() == 1);
  ಹಿಂತಿರುಗಿ convert_IEEEFloat_ಗಮ್ಯ_APInt<APFloat_ತಳ::semFloat6E3M2FN>();
}

APInt IEEEFloat::convert_Float6_E2_M3_FNAPFloat_ಗಮ್ಯ_APInt() ಸ್ಥಿರಾಂಕ {
  ಪ್ರತಿಜ್ಞಾ(part_ಎಣಿಕೆ() == 1);
  ಹಿಂತಿರುಗಿ convert_IEEEFloat_ಗಮ್ಯ_APInt<APFloat_ತಳ::semFloat6E2M3FN>();
}

APInt IEEEFloat::convert_Float4_E2_M1_FNAPFloat_ಗಮ್ಯ_APInt() ಸ್ಥಿರಾಂಕ {
  ಪ್ರತಿಜ್ಞಾ(part_ಎಣಿಕೆ() == 1);
  ಹಿಂತಿರುಗಿ convert_IEEEFloat_ಗಮ್ಯ_APInt<APFloat_ತಳ::semFloat4E2M1FN>();
}

// This function creates an APInt that is just a bit map of the floating
// point constant as it would appear in memory.  It is not a conversion,
// and treating the result as a normal integer is unlikely to be useful.

APInt IEEEFloat::bitcast_ಗಮ್ಯ_APInt() ಸ್ಥಿರಾಂಕ {
  ಆದರೆ (semantics == (ಸ್ಥಿರಾಂಕ llvm::fltSemantics *)&APFloat_ತಳ::semIEEEhalf)
    ಹಿಂತಿರುಗಿ convert_Half_APFloat_ಗಮ್ಯ_APInt();

  ಆದರೆ (semantics == (ಸ್ಥಿರಾಂಕ llvm::fltSemantics *)&APFloat_ತಳ::semBFloat)
    ಹಿಂತಿರುಗಿ convert_BFloat_APFloat_ಗಮ್ಯ_APInt();

  ಆದರೆ (semantics == (ಸ್ಥಿರಾಂಕ llvm::fltSemantics *)&APFloat_ತಳ::semIEEEsingle)
    ಹಿಂತಿರುಗಿ convert_Float_APFloat_ಗಮ್ಯ_APInt();

  ಆದರೆ (semantics == (ಸ್ಥಿರಾಂಕ llvm::fltSemantics *)&APFloat_ತಳ::semIEEEdouble)
    ಹಿಂತಿರುಗಿ convert_Double_APFloat_ಗಮ್ಯ_APInt();

  ಆದರೆ (semantics == (ಸ್ಥಿರಾಂಕ llvm::fltSemantics *)&APFloat_ತಳ::semIEEEquad)
    ಹಿಂತಿರುಗಿ convert_Quadruple_APFloat_ಗಮ್ಯ_APInt();

  ಆದರೆ (semantics ==
      (ಸ್ಥಿರಾಂಕ llvm::fltSemantics *)&APFloat_ತಳ::semX87DoubleExtended)
    ಹಿಂತಿರುಗಿ convert_F80_Long_Double_APFloat_ಗಮ್ಯ_APInt();

  ಆದರೆ (semantics ==
      (ಸ್ಥಿರಾಂಕ llvm::fltSemantics *)&APFloat_ತಳ::semPPCDoubleDoubleLegacy)
    ಹಿಂತಿರುಗಿ convert_PPCDouble_Double_Legacy_APFloat_ಗಮ್ಯ_APInt();

  ಆದರೆ (semantics == (ಸ್ಥಿರಾಂಕ llvm::fltSemantics *)&APFloat_ತಳ::semFloat8E5M2)
    ಹಿಂತಿರುಗಿ convert_Float8_E5_M2_APFloat_ಗಮ್ಯ_APInt();

  ಆದರೆ (semantics == (ಸ್ಥಿರಾಂಕ llvm::fltSemantics *)&APFloat_ತಳ::semFloat8E5M2FNUZ)
    ಹಿಂತಿರುಗಿ convert_Float8_E5_M2_FNUZAPFloat_ಗಮ್ಯ_APInt();

  ಆದರೆ (semantics == (ಸ್ಥಿರಾಂಕ llvm::fltSemantics *)&APFloat_ತಳ::semFloat8E4M3)
    ಹಿಂತಿರುಗಿ convert_Float8_E4_M3_APFloat_ಗಮ್ಯ_APInt();

  ಆದರೆ (semantics == (ಸ್ಥಿರಾಂಕ llvm::fltSemantics *)&APFloat_ತಳ::semFloat8E4M3FN)
    ಹಿಂತಿರುಗಿ convert_Float8_E4_M3_FNAPFloat_ಗಮ್ಯ_APInt();

  ಆದರೆ (semantics == (ಸ್ಥಿರಾಂಕ llvm::fltSemantics *)&APFloat_ತಳ::semFloat8E4M3FNUZ)
    ಹಿಂತಿರುಗಿ convert_Float8_E4_M3_FNUZAPFloat_ಗಮ್ಯ_APInt();

  ಆದರೆ (semantics ==
      (ಸ್ಥಿರಾಂಕ llvm::fltSemantics *)&APFloat_ತಳ::semFloat8E4M3B11FNUZ)
    ಹಿಂತಿರುಗಿ convert_Float8_E4_M3_B11_FNUZAPFloat_ಗಮ್ಯ_APInt();

  ಆದರೆ (semantics == (ಸ್ಥಿರಾಂಕ llvm::fltSemantics *)&APFloat_ತಳ::semFloat8E3M4)
    ಹಿಂತಿರುಗಿ convert_Float8_E3_M4_APFloat_ಗಮ್ಯ_APInt();

  ಆದರೆ (semantics == (ಸ್ಥಿರಾಂಕ llvm::fltSemantics *)&APFloat_ತಳ::semFloatTF32)
    ಹಿಂತಿರುಗಿ convert_Float_TF32_APFloat_ಗಮ್ಯ_APInt();

  ಆದರೆ (semantics == (ಸ್ಥಿರಾಂಕ llvm::fltSemantics *)&APFloat_ತಳ::semFloat8E8M0FNU)
    ಹಿಂತಿರುಗಿ convert_Float8_E8_M0_FNUAPFloat_ಗಮ್ಯ_APInt();

  ಆದರೆ (semantics == (ಸ್ಥಿರಾಂಕ llvm::fltSemantics *)&APFloat_ತಳ::semFloat8E5M3FNU)
    ಹಿಂತಿರುಗಿ convert_Float8_E5_M3_FNUAPFloat_ಗಮ್ಯ_APInt();

  ಆದರೆ (semantics == (ಸ್ಥಿರಾಂಕ llvm::fltSemantics *)&APFloat_ತಳ::semFloat6E3M2FN)
    ಹಿಂತಿರುಗಿ convert_Float6_E3_M2_FNAPFloat_ಗಮ್ಯ_APInt();

  ಆದರೆ (semantics == (ಸ್ಥಿರಾಂಕ llvm::fltSemantics *)&APFloat_ತಳ::semFloat6E2M3FN)
    ಹಿಂತಿರುಗಿ convert_Float6_E2_M3_FNAPFloat_ಗಮ್ಯ_APInt();

  ಆದರೆ (semantics == (ಸ್ಥಿರಾಂಕ llvm::fltSemantics *)&APFloat_ತಳ::semFloat4E2M1FN)
    ಹಿಂತಿರುಗಿ convert_Float4_E2_M1_FNAPFloat_ಗಮ್ಯ_APInt();

  llvm_unreachable("unknown format!");
}

ದಶಮಾಂಶ IEEEFloat::convert_ಗಮ್ಯ_Float() ಸ್ಥಿರಾಂಕ {
  ಪ್ರತಿಜ್ಞಾ(semantics == (ಸ್ಥಿರಾಂಕ llvm::fltSemantics *)&APFloat_ತಳ::semIEEEsingle &&
         "Float semantics are not IEEEsingle");
  APInt api = bitcast_ಗಮ್ಯ_APInt();
  ಹಿಂತಿರುಗಿ api.ಬಿಟ್ಟುಗಳು_ಗಮ್ಯ_Float();
}

ದ್ವಿಗುಣ IEEEFloat::convert_ಗಮ್ಯ_Double() ಸ್ಥಿರಾಂಕ {
  ಪ್ರತಿಜ್ಞಾ(semantics == (ಸ್ಥಿರಾಂಕ llvm::fltSemantics *)&APFloat_ತಳ::semIEEEdouble &&
         "Float semantics are not IEEEdouble");
  APInt api = bitcast_ಗಮ್ಯ_APInt();
  ಹಿಂತಿರುಗಿ api.ಬಿಟ್ಟುಗಳು_ಗಮ್ಯ_Double();
}

#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ HAS_IEE754_FLOAT128
float128 IEEEFloat::convert_ಗಮ್ಯ_Quad() ಸ್ಥಿರಾಂಕ {
  ಪ್ರತಿಜ್ಞಾ(semantics == (ಸ್ಥಿರಾಂಕ llvm::fltSemantics *)&APFloat_ತಳ::semIEEEquad &&
         "Float semantics are not IEEEquads");
  APInt api = bitcast_ಗಮ್ಯ_APInt();
  ಹಿಂತಿರುಗಿ api.ಬಿಟ್ಟುಗಳು_ಗಮ್ಯ_Quad();
}
#ಆದರೆ_ಅಂತ್ಯ

ಶೂನ್ಯ IEEEFloat::initFromF80LongDoubleAPInt(ಸ್ಥಿರಾಂಕ APInt &api) {
  ಹಿಂತಿರುಗಿ initFromIEEEAPInt<APFloat_ತಳ::semX87DoubleExtended>(api);
}

ಶೂನ್ಯ IEEEFloat::initFromPPCDoubleDoubleLegacyAPInt(ಸ್ಥಿರಾಂಕ APInt &api) {
  uint64_t i1 = api.get_Raw_ದತ್ತಾಂಶ()[0];
  uint64_t i2 = api.get_Raw_ದತ್ತಾಂಶ()[1];
  ತಾರ್ಕಿಕ losesInfo;

  // Get the first double and convert to our format.
  initFromDoubleAPInt(APInt(64, i1));
  [[maybe_ಬಳಸದ]] op_ಸ್ಥಿತಿ fs = convert(APFloat_ತಳ::semPPCDoubleDoubleLegacy,
                                         rm_ಹತ್ತಿರದ_Ties_ಗಮ್ಯ_Even, &losesInfo);
  // (convert may return opInvalidOp if i1 is an sNaN).
  ಪ್ರತಿಜ್ಞಾ((fs == opOK || fs == opInvalidOp) && !losesInfo);

  // Unless we have a special case, add in second double.
  ಆದರೆ (isFiniteNonZero()) {
    IEEEFloat v(APFloat_ತಳ::semIEEEdouble, APInt(64, i2));
    // The first double determines the category, so ignore a non-canonical
    // second double.
    ಆದರೆ (v.isFinite()) {
      fs = v.convert(APFloat_ತಳ::semPPCDoubleDoubleLegacy, rm_ಹತ್ತಿರದ_Ties_ಗಮ್ಯ_Even,
                     &losesInfo);
      ಪ್ರತಿಜ್ಞಾ(fs == opOK && !losesInfo);

      ಕೂಡಿಸು(v, rm_ಹತ್ತಿರದ_Ties_ಗಮ್ಯ_Even);
    }
  }
}

// The E8M0 format has the following characteristics:
// It is an 8-bit unsigned format with only exponents (no actual significand).
// No encodings for {zero, infinities or denorms}.
// NaN is represented by all 1's.
// Bias is 127.
ಶೂನ್ಯ IEEEFloat::initFromFloat8E8M0FNUAPInt(ಸ್ಥಿರಾಂಕ APInt &api) {
  initFromIEEEAPInt<APFloat_ತಳ::semFloat8E8M0FNU>(api);
}

ಶೂನ್ಯ IEEEFloat::initFromFloat8E5M3FNUAPInt(ಸ್ಥಿರಾಂಕ APInt &api) {
  initFromIEEEAPInt<APFloat_ತಳ::semFloat8E5M3FNU>(api);
}

ಮಾದರಿ <ಸ್ಥಿರಾಂಕ fltSemantics &S>
ಶೂನ್ಯ IEEEFloat::initFromIEEEAPInt(ಸ್ಥಿರಾಂಕ APInt &api) {
  ಪ್ರತಿಜ್ಞಾ(api.get_ಬಿಟ್_ಅಗಲ() == S.ಗಾತ್ರ_ಒಳ_ಬಿಟ್ಟುಗಳು);

  constexpr ಚಿಹ್ನೆರಹಿತ ಪೂರ್ಣಾಂಕ trailing_significand_ಬಿಟ್ಟುಗಳು =
      S.precision - 1 + S.has_Explicit_Integer_ಬಿಟ್;
  constexpr integerPart integer_ಬಿಟ್ =
      integerPart{1} << (trailing_significand_ಬಿಟ್ಟುಗಳು % integer_Part_ಅಗಲ);
  constexpr uint64_t significand_ಮುಖವಾಡ = integer_ಬಿಟ್ - 1;
  constexpr ಚಿಹ್ನೆರಹಿತ ಪೂರ್ಣಾಂಕ exponent_ಬಿಟ್ಟುಗಳು =
      S.ಗಾತ್ರ_ಒಳ_ಬಿಟ್ಟುಗಳು - (S.hasSignedRepr ? 1 : 0) - trailing_significand_ಬಿಟ್ಟುಗಳು;
  static_assert(exponent_ಬಿಟ್ಟುಗಳು < 64);
  constexpr ಚಿಹ್ನೆರಹಿತ ಪೂರ್ಣಾಂಕ stored_significand_parts =
      part_ಎಣಿಕೆ_For_ಬಿಟ್ಟುಗಳು(trailing_significand_ಬಿಟ್ಟುಗಳು + 1);
  constexpr uint64_t exponent_ಮುಖವಾಡ = (uint64_t{1} << exponent_ಬಿಟ್ಟುಗಳು) - 1;
  constexpr ತಾರ್ಕಿಕ is_zero_exp_reserved = S.hasDenormals || S.hasZero;
  constexpr ಪೂರ್ಣಾಂಕ bias = -(S.minExponent - (is_zero_exp_reserved ? 1 : 0));
  constexpr ತಾರ್ಕಿಕ has_significand = trailing_significand_ಬಿಟ್ಟುಗಳು > 0;

  // Copy the bits of the significand. We need to clear out the exponent and
  // sign bit in the last word.
  std::array<integerPart, stored_significand_parts> mysignificand;
  ಆದರೆ constexpr (has_significand) {
    std::copy_n(api.get_Raw_ದತ್ತಾಂಶ(), mysignificand.ಗಾತ್ರ_3(), mysignificand.begin());
    ಆದರೆ constexpr (significand_ಮುಖವಾಡ != 0 || S.precision >= integer_Part_ಅಗಲ) {
      mysignificand[mysignificand.ಗಾತ್ರ_3() - 1] &= significand_ಮುಖವಾಡ;
    }
  } ಇಲ್ಲದಿದ್ದರೆ {
    std::fill_n(mysignificand.begin(), mysignificand.ಗಾತ್ರ_3(), 0);
    // Always set integer bit to 1 for consistency in APFloat's internal
    // representation.
    mysignificand[0] = 1;
  }

  // We assume the last word holds the sign bit, the exponent, and potentially
  // some of the trailing significand field.
  uint64_t ಕೊನೆಯ_ಶಬ್ದ = api.get_Raw_ದತ್ತಾಂಶ()[api.get_ಅಂಕೆ_ಪದಗಳು() - 1];
  uint64_t myexponent =
      (ಕೊನೆಯ_ಶಬ್ದ >> (trailing_significand_ಬಿಟ್ಟುಗಳು % 64)) & exponent_ಮುಖವಾಡ;

  initialize(&S);
  ಪ್ರತಿಜ್ಞಾ(part_ಎಣಿಕೆ() == mysignificand.ಗಾತ್ರ_3());

  sign = S.hasSignedRepr
             ? static_cast<ಚಿಹ್ನೆರಹಿತ ಪೂರ್ಣಾಂಕ>(ಕೊನೆಯ_ಶಬ್ದ >> ((S.ಗಾತ್ರ_ಒಳ_ಬಿಟ್ಟುಗಳು - 1) % 64))
             : 0;

  ತಾರ್ಕಿಕ all_zero_significand =
      has_significand && llvm::all_ದ(mysignificand, equal_ಗಮ್ಯ(0));

  ತಾರ್ಕಿಕ is_zero = myexponent == 0 && all_zero_significand && S.hasZero;

  ಆದರೆ constexpr (S.nonFiniteBehavior == fltNonfiniteBehavior::IEEE754) {
    ತಾರ್ಕಿಕ is_inf = ಸುಳ್ಳು;

    ಆದರೆ constexpr (S.has_Explicit_Integer_ಬಿಟ್) {
      // This is only used and tested for x87DoubleExtended
      static_assert(S.precision == 64);
      constexpr integerPart significand_ಮುಖವಾಡ_no_int_ಬಿಟ್ =
          (uint64_t{1} << (trailing_significand_ಬಿಟ್ಟುಗಳು - 1)) - 1;
      ಸ್ಥಿರಾಂಕ integerPart myintegerbit =
          mysignificand[0] >> (trailing_significand_ಬಿಟ್ಟುಗಳು - 1);

      is_inf = myexponent - bias == ::exponentInf(S) && myintegerbit == 1 &&
               (mysignificand[0] & significand_ಮುಖವಾಡ_no_int_ಬಿಟ್) == 0;
    } ಇಲ್ಲದಿದ್ದರೆ {
      is_inf = myexponent - bias == ::exponentInf(S) && all_zero_significand;
    }

    ಆದರೆ (is_inf) {
      makeInf(sign);
      ಹಿಂತಿರುಗಿ;
    }
  }

  ತಾರ್ಕಿಕ is_nan = ಸುಳ್ಳು;

  ಆದರೆ constexpr (S.nanEncoding == fltNanEncoding::IEEE) {
    ಆದರೆ constexpr (S.has_Explicit_Integer_ಬಿಟ್) {
      // This is only used and tested for x87DoubleExtended
      static_assert(S.precision == 64);
      ಸ್ಥಿರಾಂಕ integerPart myintegerbit =
          mysignificand[0] >> (trailing_significand_ಬಿಟ್ಟುಗಳು - 1);
      constexpr integerPart significand_ಮುಖವಾಡ_no_int_ಬಿಟ್ =
          (uint64_t{1} << (trailing_significand_ಬಿಟ್ಟುಗಳು - 1)) - 1;

      ಆದರೆ (myexponent - bias == ::exponentNaN(S) &&
          (mysignificand[0] & significand_ಮುಖವಾಡ_no_int_ಬಿಟ್) != 0) {
        // regular NaN and pseudoNaN
        is_nan = ನಿಜ;
      } ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (myexponent - bias == ::exponentNaN(S) &&
                 (mysignificand[0] & significand_ಮುಖವಾಡ_no_int_ಬಿಟ್) == 0) {
        // pseudoinfinity
        is_nan = ನಿಜ;
      } ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (myexponent - bias != ::exponentNaN(S) && myexponent != 0 &&
                 myintegerbit == 0) {
        // unnormal
        is_nan = ನಿಜ;
      }
    } ಇಲ್ಲದಿದ್ದರೆ {
      is_nan = myexponent - bias == ::exponentNaN(S) && !all_zero_significand;
    }
  } ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ constexpr (S.nanEncoding == fltNanEncoding::AllOnes) {
    ತಾರ್ಕಿಕ all_ones_significand =
        std::all_ದ(mysignificand.begin(), mysignificand.ಕೊನೆ() - 1,
                    [](integerPart ಬಿಟ್ಟುಗಳು) { ಹಿಂತಿರುಗಿ ಬಿಟ್ಟುಗಳು == ~integerPart{0}; }) &&
        (!significand_ಮುಖವಾಡ ||
         mysignificand[mysignificand.ಗಾತ್ರ_3() - 1] == significand_ಮುಖವಾಡ);
    is_nan = myexponent - bias == ::exponentNaN(S) && all_ones_significand;
  } ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ constexpr (S.nanEncoding == fltNanEncoding::NegativeZero) {
    is_nan = is_zero && sign;
  }

  ಆದರೆ (is_nan) {
    category = fcNaN;
    exponent = ::exponentNaN(S);
    std::copy_n(mysignificand.begin(), mysignificand.ಗಾತ್ರ_3(),
                significandParts());
    ಹಿಂತಿರುಗಿ;
  }

  ಆದರೆ (is_zero) {
    makeZero(sign);
    ಹಿಂತಿರುಗಿ;
  }

  category = fc_ಸಾಮಾನ್ಯ;
  exponent = myexponent - bias;
  std::copy_n(mysignificand.begin(), mysignificand.ಗಾತ್ರ_3(), significandParts());
  ಆದರೆ (myexponent == 0 && S.hasDenormals) // denormal
    exponent = S.minExponent;
  ಇಲ್ಲದಿದ್ದರೆ {
    ಆದರೆ constexpr (!S.has_Explicit_Integer_ಬಿಟ್) {
      significandParts()[mysignificand.ಗಾತ್ರ_3() - 1] |= integer_ಬಿಟ್;
    }
  }
}

ಶೂನ್ಯ IEEEFloat::initFromQuadrupleAPInt(ಸ್ಥಿರಾಂಕ APInt &api) {
  initFromIEEEAPInt<APFloat_ತಳ::semIEEEquad>(api);
}

ಶೂನ್ಯ IEEEFloat::initFromDoubleAPInt(ಸ್ಥಿರಾಂಕ APInt &api) {
  initFromIEEEAPInt<APFloat_ತಳ::semIEEEdouble>(api);
}

ಶೂನ್ಯ IEEEFloat::initFromFloatAPInt(ಸ್ಥಿರಾಂಕ APInt &api) {
  initFromIEEEAPInt<APFloat_ತಳ::semIEEEsingle>(api);
}

ಶೂನ್ಯ IEEEFloat::initFromBFloatAPInt(ಸ್ಥಿರಾಂಕ APInt &api) {
  initFromIEEEAPInt<APFloat_ತಳ::semBFloat>(api);
}

ಶೂನ್ಯ IEEEFloat::initFromHalfAPInt(ಸ್ಥಿರಾಂಕ APInt &api) {
  initFromIEEEAPInt<APFloat_ತಳ::semIEEEhalf>(api);
}

ಶೂನ್ಯ IEEEFloat::initFromFloat8E5M2APInt(ಸ್ಥಿರಾಂಕ APInt &api) {
  initFromIEEEAPInt<APFloat_ತಳ::semFloat8E5M2>(api);
}

ಶೂನ್ಯ IEEEFloat::initFromFloat8E5M2FNUZAPInt(ಸ್ಥಿರಾಂಕ APInt &api) {
  initFromIEEEAPInt<APFloat_ತಳ::semFloat8E5M2FNUZ>(api);
}

ಶೂನ್ಯ IEEEFloat::initFromFloat8E4M3APInt(ಸ್ಥಿರಾಂಕ APInt &api) {
  initFromIEEEAPInt<APFloat_ತಳ::semFloat8E4M3>(api);
}

ಶೂನ್ಯ IEEEFloat::initFromFloat8E4M3FNAPInt(ಸ್ಥಿರಾಂಕ APInt &api) {
  initFromIEEEAPInt<APFloat_ತಳ::semFloat8E4M3FN>(api);
}

ಶೂನ್ಯ IEEEFloat::initFromFloat8E4M3FNUZAPInt(ಸ್ಥಿರಾಂಕ APInt &api) {
  initFromIEEEAPInt<APFloat_ತಳ::semFloat8E4M3FNUZ>(api);
}

ಶೂನ್ಯ IEEEFloat::initFromFloat8E4M3B11FNUZAPInt(ಸ್ಥಿರಾಂಕ APInt &api) {
  initFromIEEEAPInt<APFloat_ತಳ::semFloat8E4M3B11FNUZ>(api);
}

ಶೂನ್ಯ IEEEFloat::initFromFloat8E3M4APInt(ಸ್ಥಿರಾಂಕ APInt &api) {
  initFromIEEEAPInt<APFloat_ತಳ::semFloat8E3M4>(api);
}

ಶೂನ್ಯ IEEEFloat::initFromFloatTF32APInt(ಸ್ಥಿರಾಂಕ APInt &api) {
  initFromIEEEAPInt<APFloat_ತಳ::semFloatTF32>(api);
}

ಶೂನ್ಯ IEEEFloat::initFromFloat6E3M2FNAPInt(ಸ್ಥಿರಾಂಕ APInt &api) {
  initFromIEEEAPInt<APFloat_ತಳ::semFloat6E3M2FN>(api);
}

ಶೂನ್ಯ IEEEFloat::initFromFloat6E2M3FNAPInt(ಸ್ಥಿರಾಂಕ APInt &api) {
  initFromIEEEAPInt<APFloat_ತಳ::semFloat6E2M3FN>(api);
}

ಶೂನ್ಯ IEEEFloat::initFromFloat4E2M1FNAPInt(ಸ್ಥಿರಾಂಕ APInt &api) {
  initFromIEEEAPInt<APFloat_ತಳ::semFloat4E2M1FN>(api);
}

/// Treat api as containing the bits of a floating point number.
ಶೂನ್ಯ IEEEFloat::initFromAPInt(ಸ್ಥಿರಾಂಕ fltSemantics *Sem, ಸ್ಥಿರಾಂಕ APInt &api) {
  ಪ್ರತಿಜ್ಞಾ(api.get_ಬಿಟ್_ಅಗಲ() == Sem->ಗಾತ್ರ_ಒಳ_ಬಿಟ್ಟುಗಳು);
  ಆದರೆ (Sem == &APFloat_ತಳ::semIEEEhalf)
    ಹಿಂತಿರುಗಿ initFromHalfAPInt(api);
  ಆದರೆ (Sem == &APFloat_ತಳ::semBFloat)
    ಹಿಂತಿರುಗಿ initFromBFloatAPInt(api);
  ಆದರೆ (Sem == &APFloat_ತಳ::semIEEEsingle)
    ಹಿಂತಿರುಗಿ initFromFloatAPInt(api);
  ಆದರೆ (Sem == &APFloat_ತಳ::semIEEEdouble)
    ಹಿಂತಿರುಗಿ initFromDoubleAPInt(api);
  ಆದರೆ (Sem == &APFloat_ತಳ::semIEEEquad)
    ಹಿಂತಿರುಗಿ initFromQuadrupleAPInt(api);
  ಆದರೆ (Sem == &APFloat_ತಳ::semX87DoubleExtended)
    ಹಿಂತಿರುಗಿ initFromF80LongDoubleAPInt(api);
  ಆದರೆ (Sem == &APFloat_ತಳ::semPPCDoubleDoubleLegacy)
    ಹಿಂತಿರುಗಿ initFromPPCDoubleDoubleLegacyAPInt(api);
  ಆದರೆ (Sem == &APFloat_ತಳ::semFloat8E5M2)
    ಹಿಂತಿರುಗಿ initFromFloat8E5M2APInt(api);
  ಆದರೆ (Sem == &APFloat_ತಳ::semFloat8E5M2FNUZ)
    ಹಿಂತಿರುಗಿ initFromFloat8E5M2FNUZAPInt(api);
  ಆದರೆ (Sem == &APFloat_ತಳ::semFloat8E4M3)
    ಹಿಂತಿರುಗಿ initFromFloat8E4M3APInt(api);
  ಆದರೆ (Sem == &APFloat_ತಳ::semFloat8E4M3FN)
    ಹಿಂತಿರುಗಿ initFromFloat8E4M3FNAPInt(api);
  ಆದರೆ (Sem == &APFloat_ತಳ::semFloat8E4M3FNUZ)
    ಹಿಂತಿರುಗಿ initFromFloat8E4M3FNUZAPInt(api);
  ಆದರೆ (Sem == &APFloat_ತಳ::semFloat8E4M3B11FNUZ)
    ಹಿಂತಿರುಗಿ initFromFloat8E4M3B11FNUZAPInt(api);
  ಆದರೆ (Sem == &APFloat_ತಳ::semFloat8E3M4)
    ಹಿಂತಿರುಗಿ initFromFloat8E3M4APInt(api);
  ಆದರೆ (Sem == &APFloat_ತಳ::semFloatTF32)
    ಹಿಂತಿರುಗಿ initFromFloatTF32APInt(api);
  ಆದರೆ (Sem == &APFloat_ತಳ::semFloat8E8M0FNU)
    ಹಿಂತಿರುಗಿ initFromFloat8E8M0FNUAPInt(api);
  ಆದರೆ (Sem == &APFloat_ತಳ::semFloat8E5M3FNU)
    ಹಿಂತಿರುಗಿ initFromFloat8E5M3FNUAPInt(api);
  ಆದರೆ (Sem == &APFloat_ತಳ::semFloat6E3M2FN)
    ಹಿಂತಿರುಗಿ initFromFloat6E3M2FNAPInt(api);
  ಆದರೆ (Sem == &APFloat_ತಳ::semFloat6E2M3FN)
    ಹಿಂತಿರುಗಿ initFromFloat6E2M3FNAPInt(api);
  ಆದರೆ (Sem == &APFloat_ತಳ::semFloat4E2M1FN)
    ಹಿಂತಿರುಗಿ initFromFloat4E2M1FNAPInt(api);

  llvm_unreachable("unsupported semantics");
}

/// Make this number the largest magnitude normal number in the given
/// semantics.
ಶೂನ್ಯ IEEEFloat::make_ಅತಿದೊಡ್ಡ(ತಾರ್ಕಿಕ Negative) {
  ಆದರೆ (Negative && !semantics->hasSignedRepr)
    llvm_unreachable(
        "This floating point format does not support signed values");
  // We want (in interchange format):
  //   sign = {Negative}
  //   exponent = 1..10
  //   significand = 1..1
  category = fc_ಸಾಮಾನ್ಯ;
  sign = Negative;
  exponent = semantics->maxExponent;

  // Use memset to set all but the highest integerPart to all ones.
  integerPart *significand = significandParts();
  ಚಿಹ್ನೆರಹಿತ Part_ಎಣಿಕೆ = part_ಎಣಿಕೆ();
  memset(significand, 0xFF, ಗಾತ್ರ(integerPart)*(Part_ಎಣಿಕೆ - 1));

  // Set the high integerPart especially setting all unused top bits for
  // internal consistency.
  ಸ್ಥಿರಾಂಕ ಚಿಹ್ನೆರಹಿತ ಅಂಕೆ_ಬಳಸದ_ಮೇಲಿನ_ಬಿಟ್ಟುಗಳು =
    Part_ಎಣಿಕೆ*integer_Part_ಅಗಲ - semantics->precision;
  significand[Part_ಎಣಿಕೆ - 1] = (ಅಂಕೆ_ಬಳಸದ_ಮೇಲಿನ_ಬಿಟ್ಟುಗಳು < integer_Part_ಅಗಲ)
                                   ? (~integerPart(0) >> ಅಂಕೆ_ಬಳಸದ_ಮೇಲಿನ_ಬಿಟ್ಟುಗಳು)
                                   : 0;
  ಆದರೆ (semantics->nonFiniteBehavior == fltNonfiniteBehavior::NanOnly &&
      semantics->nanEncoding == fltNanEncoding::AllOnes &&
      (semantics->precision > 1))
    significand[0] &= ~integerPart(1);
}

/// Make this number the smallest magnitude denormal number in the given
/// semantics.
ಶೂನ್ಯ IEEEFloat::make_ಅತಿಚಿಕ್ಕ(ತಾರ್ಕಿಕ Negative) {
  ಆದರೆ (Negative && !semantics->hasSignedRepr)
    llvm_unreachable(
        "This floating point format does not support signed values");
  // We want (in interchange format):
  //   sign = {Negative}
  //   exponent = 0..0
  //   significand = 0..01
  category = fc_ಸಾಮಾನ್ಯ;
  sign = Negative;
  exponent = semantics->minExponent;
  APInt::tc_ಗಣ(significandParts(), 1, part_ಎಣಿಕೆ());
}

ಶೂನ್ಯ IEEEFloat::make_ಅತಿಚಿಕ್ಕ_Normalized(ತಾರ್ಕಿಕ Negative) {
  ಆದರೆ (Negative && !semantics->hasSignedRepr)
    llvm_unreachable(
        "This floating point format does not support signed values");
  // We want (in interchange format):
  //   sign = {Negative}
  //   exponent = 0..0
  //   significand = 10..0

  category = fc_ಸಾಮಾನ್ಯ;
  zeroSignificand();
  sign = Negative;
  exponent = semantics->minExponent;
  APInt::tc_ಗಣ_ಬಿಟ್(significandParts(), semantics->precision - 1);
}

IEEEFloat::IEEEFloat(ಸ್ಥಿರಾಂಕ fltSemantics &Sem, ಸ್ಥಿರಾಂಕ APInt &API) {
  initFromAPInt(&Sem, API);
}

IEEEFloat::IEEEFloat(ದಶಮಾಂಶ f) {
  initFromAPInt(&APFloat_ತಳ::semIEEEsingle, APInt::float_ಗಮ್ಯ_ಬಿಟ್ಟುಗಳು(f));
}

IEEEFloat::IEEEFloat(ದ್ವಿಗುಣ d) {
  initFromAPInt(&APFloat_ತಳ::semIEEEdouble, APInt::double_ಗಮ್ಯ_ಬಿಟ್ಟುಗಳು(d));
}

ನಾಮಾಕಾಶ {
  ಶೂನ್ಯ append(Small_ಸದಿಶ_Impl<ಅಕ್ಷರ> &ಬಫರ್, StringRef Str) {
    ಬಫರ್.append(Str.begin(), Str.ಕೊನೆ());
  }

  /// Removes data from the given significand until it is no more
  /// precise than is required for the desired precision.
  ಶೂನ್ಯ Adjust_ಗಮ್ಯ_Precision(APInt &significand,
                         ಪೂರ್ಣಾಂಕ &ಘಾತಾಂಕ, ಚಿಹ್ನೆರಹಿತ FormatPrecision) {
    ಚಿಹ್ನೆರಹಿತ ಬಿಟ್ಟುಗಳು = significand.get_ಸಕ್ರಿಯ_ಬಿಟ್ಟುಗಳು();

    // 196/59 is a very slight overestimate of lg_2(10).
    ಚಿಹ್ನೆರಹಿತ ಬಿಟ್ಟುಗಳು_Required = (FormatPrecision * 196 + 58) / 59;

    ಆದರೆ (ಬಿಟ್ಟುಗಳು <= ಬಿಟ್ಟುಗಳು_Required) ಹಿಂತಿರುಗಿ;

    ಚಿಹ್ನೆರಹಿತ tensRemovable = (ಬಿಟ್ಟುಗಳು - ಬಿಟ್ಟುಗಳು_Required) * 59 / 196;
    ಆದರೆ (!tensRemovable) ಹಿಂತಿರುಗಿ;

    ಘಾತಾಂಕ += tensRemovable;

    APInt ಭಾಜಕ(significand.get_ಬಿಟ್_ಅಗಲ(), 1);
    APInt powten(significand.get_ಬಿಟ್_ಅಗಲ(), 10);
    ಆಗ (ನಿಜ) {
      ಆದರೆ (tensRemovable & 1)
        ಭಾಜಕ *= powten;
      tensRemovable >>= 1;
      ಆದರೆ (!tensRemovable) ನಿಲ್ಲಿಸು;
      powten *= powten;
    }

    significand = significand.udiv(ಭಾಜಕ);

    // Truncate the significand down to its active bit count.
    significand = significand.trunc(significand.get_ಸಕ್ರಿಯ_ಬಿಟ್ಟುಗಳು());
  }


  ಶೂನ್ಯ Adjust_ಗಮ್ಯ_Precision(Small_ಸದಿಶ_Impl<ಅಕ್ಷರ> &ಬಫರ್_2,
                         ಪೂರ್ಣಾಂಕ &ಘಾತಾಂಕ, ಚಿಹ್ನೆರಹಿತ FormatPrecision) {
    ಚಿಹ್ನೆರಹಿತ N = ಬಫರ್_2.ಗಾತ್ರ_3();
    ಆದರೆ (N <= FormatPrecision) ಹಿಂತಿರುಗಿ;

    // The most significant figures are the last ones in the buffer.
    ಚಿಹ್ನೆರಹಿತ ಮೊದಲ_Significant = N - FormatPrecision;

    // Round.
    // FIXME: this probably shouldn't use 'round half up'.

    // Rounding down is just a truncation, except we also want to drop
    // trailing zeros from the new result.
    ಆದರೆ (ಬಫರ್_2[ಮೊದಲ_Significant - 1] < '5') {
      ಆಗ (ಮೊದಲ_Significant < N && ಬಫರ್_2[ಮೊದಲ_Significant] == '0')
        ಮೊದಲ_Significant++;

      ಘಾತಾಂಕ += ಮೊದಲ_Significant;
      ಬಫರ್_2.erase(&ಬಫರ್_2[0], &ಬಫರ್_2[ಮೊದಲ_Significant]);
      ಹಿಂತಿರುಗಿ;
    }

    // Rounding up requires a decimal add-with-carry.  If we continue
    // the carry, the newly-introduced zeros will just be truncated.
    ಗಾಗಿ (ಚಿಹ್ನೆರಹಿತ I = ಮೊದಲ_Significant; I != N; ++I) {
      ಆದರೆ (ಬಫರ್_2[I] == '9') {
        ಮೊದಲ_Significant++;
      } ಇಲ್ಲದಿದ್ದರೆ {
        ಬಫರ್_2[I]++;
        ನಿಲ್ಲಿಸು;
      }
    }

    // If we carried through, we have exactly one digit of precision.
    ಆದರೆ (ಮೊದಲ_Significant == N) {
      ಘಾತಾಂಕ += ಮೊದಲ_Significant;
      ಬಫರ್_2.clear();
      ಬಫರ್_2.ತಳ್ಳು_ಹಿಂಭಾಗ('1');
      ಹಿಂತಿರುಗಿ;
    }

    ಘಾತಾಂಕ += ಮೊದಲ_Significant;
    ಬಫರ್_2.erase(&ಬಫರ್_2[0], &ಬಫರ್_2[ಮೊದಲ_Significant]);
  }

  ಶೂನ್ಯ ಗಮ್ಯ_String_Impl(Small_ಸದಿಶ_Impl<ಅಕ್ಷರ> &Str, ಸ್ಥಿರಾಂಕ ತಾರ್ಕಿಕ isNeg, ಪೂರ್ಣಾಂಕ ಘಾತಾಂಕ,
                    APInt significand, ಚಿಹ್ನೆರಹಿತ FormatPrecision,
                    ಚಿಹ್ನೆರಹಿತ FormatMaxPadding, ತಾರ್ಕಿಕ TruncateZero) {
    ಸ್ಥಿರಾಂಕ ಪೂರ್ಣಾಂಕ semanticsPrecision = significand.get_ಬಿಟ್_ಅಗಲ();

    ಆದರೆ (isNeg)
      Str.ತಳ್ಳು_ಹಿಂಭಾಗ('-');

    // Set FormatPrecision if zero.  We want to do this before we
    // truncate trailing zeros, as those are part of the precision.
    ಆದರೆ (!FormatPrecision) {
      // We use enough digits so the number can be round-tripped back to an
      // APFloat. The formula comes from "How to Print Floating-Point Numbers
      // Accurately" by Steele and White.
      // FIXME: Using a formula based purely on the precision is conservative;
      // we can print fewer digits depending on the actual value being printed.

      // FormatPrecision = 2 + floor(significandBits / lg_2(10))
      FormatPrecision = 2 + semanticsPrecision * 59 / 196;
    }

    // Ignore trailing binary zeros.
    ಪೂರ್ಣಾಂಕ trailingZeros = significand.countr_zero();
    ಘಾತಾಂಕ += trailingZeros;
    significand.lshr_ಒಳ_ಇರಿಸು(trailingZeros);

    // Change the exponent from 2^e to 10^e.
    ಆದರೆ (ಘಾತಾಂಕ == 0) {
      // Nothing to do.
    } ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (ಘಾತಾಂಕ > 0) {
      // Just shift left.
      significand = significand.zext(semanticsPrecision + ಘಾತಾಂಕ);
      significand <<= ಘಾತಾಂಕ;
      ಘಾತಾಂಕ = 0;
    } ಇಲ್ಲದಿದ್ದರೆ { /* exp < 0 */
      ಪೂರ್ಣಾಂಕ texp = -ಘಾತಾಂಕ;

      // We transform this using the identity:
      //   (N)(2^-e) == (N)(5^e)(10^-e)
      // This means we have to multiply N (the significand) by 5^e.
      // To avoid overflow, we have to operate on numbers large
      // enough to store N * 5^e:
      //   log2(N * 5^e) == log2(N) + e * log2(5)
      //                 <= semantics->precision + e * 137 / 59
      //   (log_2(5) ~ 2.321928 < 2.322034 ~ 137/59)

      ಚಿಹ್ನೆರಹಿತ precision = semanticsPrecision + (137 * texp + 136) / 59;

      // Multiply significand by 5^e.
      //   N * 5^0101 == N * 5^(1*1) * 5^(0*2) * 5^(1*4) * 5^(0*8)
      significand = significand.zext(precision);
      APInt five_ಗಮ್ಯ_the_i(precision, 5);
      ಆಗ (ನಿಜ) {
        ಆದರೆ (texp & 1)
          significand *= five_ಗಮ್ಯ_the_i;

        texp >>= 1;
        ಆದರೆ (!texp)
          ನಿಲ್ಲಿಸು;
        five_ಗಮ್ಯ_the_i *= five_ಗಮ್ಯ_the_i;
      }
    }

    Adjust_ಗಮ್ಯ_Precision(significand, ಘಾತಾಂಕ, FormatPrecision);

    Small_ಸದಿಶ<ಅಕ್ಷರ, 256> ಬಫರ್_2;

    // Fill the buffer.
    ಚಿಹ್ನೆರಹಿತ precision = significand.get_ಬಿಟ್_ಅಗಲ();
    ಆದರೆ (precision < 4) {
      // We need enough precision to store the value 10.
      precision = 4;
      significand = significand.zext(precision);
    }
    APInt ten(precision, 10);
    APInt digit(precision, 0);

    ತಾರ್ಕಿಕ ಒಳ_Trail = ನಿಜ;
    ಆಗ (significand != 0) {
      // digit <- significand % 10
      // significand <- significand / 10
      APInt::udivrem(significand, ten, significand, digit);

      ಚಿಹ್ನೆರಹಿತ d = digit.get_ZExt_ಮೌಲ್ಯ();

      // Drop trailing zeros.
      ಆದರೆ (ಒಳ_Trail && !d)
        ಘಾತಾಂಕ++;
      ಇಲ್ಲದಿದ್ದರೆ {
        ಬಫರ್_2.ತಳ್ಳು_ಹಿಂಭಾಗ((ಅಕ್ಷರ) ('0' + d));
        ಒಳ_Trail = ಸುಳ್ಳು;
      }
    }

    ಪ್ರತಿಜ್ಞಾ(!ಬಫರ್_2.ಖಾಲಿಗಳು() && "no characters in buffer!");

    // Drop down to FormatPrecision.
    // TODO: don't do more precise calculations above than are required.
    Adjust_ಗಮ್ಯ_Precision(ಬಫರ್_2, ಘಾತಾಂಕ, FormatPrecision);

    ಚಿಹ್ನೆರಹಿತ NDigits = ಬಫರ್_2.ಗಾತ್ರ_3();

    // Check whether we should use scientific notation.
    ತಾರ್ಕಿಕ FormatScientific;
    ಆದರೆ (!FormatMaxPadding) {
      FormatScientific = ನಿಜ;
    } ಇಲ್ಲದಿದ್ದರೆ {
      ಆದರೆ (ಘಾತಾಂಕ >= 0) {
        // 765e3 --> 765000
        //              ^^^
        // But we shouldn't make the number look more precise than it is.
        FormatScientific = ((ಚಿಹ್ನೆರಹಿತ) ಘಾತಾಂಕ > FormatMaxPadding ||
                            NDigits + (ಚಿಹ್ನೆರಹಿತ) ಘಾತಾಂಕ > FormatPrecision);
      } ಇಲ್ಲದಿದ್ದರೆ {
        // Power of the most significant digit.
        ಪೂರ್ಣಾಂಕ MSD = ಘಾತಾಂಕ + (ಪೂರ್ಣಾಂಕ) (NDigits - 1);
        ಆದರೆ (MSD >= 0) {
          // 765e-2 == 7.65
          FormatScientific = ಸುಳ್ಳು;
        } ಇಲ್ಲದಿದ್ದರೆ {
          // 765e-5 == 0.00765
          //           ^ ^^
          FormatScientific = ((ಚಿಹ್ನೆರಹಿತ) -MSD) > FormatMaxPadding;
        }
      }
    }

    // Scientific formatting is pretty straightforward.
    ಆದರೆ (FormatScientific) {
      ಘಾತಾಂಕ += (NDigits - 1);

      Str.ತಳ್ಳು_ಹಿಂಭಾಗ(ಬಫರ್_2[NDigits-1]);
      Str.ತಳ್ಳು_ಹಿಂಭಾಗ('.');
      ಆದರೆ (NDigits == 1 && TruncateZero)
        Str.ತಳ್ಳು_ಹಿಂಭಾಗ('0');
      ಇಲ್ಲದಿದ್ದರೆ
        ಗಾಗಿ (ಚಿಹ್ನೆರಹಿತ I = 1; I != NDigits; ++I)
          Str.ತಳ್ಳು_ಹಿಂಭಾಗ(ಬಫರ್_2[NDigits-1-I]);
      // Fill with zeros up to FormatPrecision.
      ಆದರೆ (!TruncateZero && FormatPrecision > NDigits - 1)
        Str.append(FormatPrecision - NDigits + 1, '0');
      // For !TruncateZero we use lower 'e'.
      Str.ತಳ್ಳು_ಹಿಂಭಾಗ(TruncateZero ? 'E' : 'e');

      Str.ತಳ್ಳು_ಹಿಂಭಾಗ(ಘಾತಾಂಕ >= 0 ? '+' : '-');
      ಆದರೆ (ಘಾತಾಂಕ < 0)
        ಘಾತಾಂಕ = -ಘಾತಾಂಕ;
      Small_ಸದಿಶ<ಅಕ್ಷರ, 6> expbuf;
      ಮಾಡು {
        expbuf.ತಳ್ಳು_ಹಿಂಭಾಗ((ಅಕ್ಷರ) ('0' + (ಘಾತಾಂಕ % 10)));
        ಘಾತಾಂಕ /= 10;
      } ಆಗ (ಘಾತಾಂಕ);
      // Exponent always at least two digits if we do not truncate zeros.
      ಆದರೆ (!TruncateZero && expbuf.ಗಾತ್ರ_3() < 2)
        expbuf.ತಳ್ಳು_ಹಿಂಭಾಗ('0');
      ಗಾಗಿ (ಚಿಹ್ನೆರಹಿತ I = 0, E = expbuf.ಗಾತ್ರ_3(); I != E; ++I)
        Str.ತಳ್ಳು_ಹಿಂಭಾಗ(expbuf[E-1-I]);
      ಹಿಂತಿರುಗಿ;
    }

    // Non-scientific, positive exponents.
    ಆದರೆ (ಘಾತಾಂಕ >= 0) {
      ಗಾಗಿ (ಚಿಹ್ನೆರಹಿತ I = 0; I != NDigits; ++I)
        Str.ತಳ್ಳು_ಹಿಂಭಾಗ(ಬಫರ್_2[NDigits-1-I]);
      ಗಾಗಿ (ಚಿಹ್ನೆರಹಿತ I = 0; I != (ಚಿಹ್ನೆರಹಿತ) ಘಾತಾಂಕ; ++I)
        Str.ತಳ್ಳು_ಹಿಂಭಾಗ('0');
      ಹಿಂತಿರುಗಿ;
    }

    // Non-scientific, negative exponents.

    // The number of digits to the left of the decimal point.
    ಪೂರ್ಣಾಂಕ NWholeDigits = ಘಾತಾಂಕ + (ಪೂರ್ಣಾಂಕ) NDigits;

    ಚಿಹ್ನೆರಹಿತ I = 0;
    ಆದರೆ (NWholeDigits > 0) {
      ಗಾಗಿ (; I != (ಚಿಹ್ನೆರಹಿತ) NWholeDigits; ++I)
        Str.ತಳ್ಳು_ಹಿಂಭಾಗ(ಬಫರ್_2[NDigits-I-1]);
      Str.ತಳ್ಳು_ಹಿಂಭಾಗ('.');
    } ಇಲ್ಲದಿದ್ದರೆ {
      ಚಿಹ್ನೆರಹಿತ NZeros = 1 + (ಚಿಹ್ನೆರಹಿತ) -NWholeDigits;

      Str.ತಳ್ಳು_ಹಿಂಭಾಗ('0');
      Str.ತಳ್ಳು_ಹಿಂಭಾಗ('.');
      ಗಾಗಿ (ಚಿಹ್ನೆರಹಿತ Z = 1; Z != NZeros; ++Z)
        Str.ತಳ್ಳು_ಹಿಂಭಾಗ('0');
    }

    ಗಾಗಿ (; I != NDigits; ++I)
      Str.ತಳ್ಳು_ಹಿಂಭಾಗ(ಬಫರ್_2[NDigits-I-1]);

  }
} // namespace

ಶೂನ್ಯ IEEEFloat::ಗಮ್ಯ_String(Small_ಸದಿಶ_Impl<ಅಕ್ಷರ> &Str, ಚಿಹ್ನೆರಹಿತ FormatPrecision,
                         ಚಿಹ್ನೆರಹಿತ FormatMaxPadding, ತಾರ್ಕಿಕ TruncateZero) ಸ್ಥಿರಾಂಕ {
  ಆಯ್ಕೆ (category) {
  ಸ್ಥಿತಿ fcInfinity:
    ಆದರೆ (isNegative())
      ಹಿಂತಿರುಗಿ append(Str, "-Inf");
    ಇಲ್ಲದಿದ್ದರೆ
      ಹಿಂತಿರುಗಿ append(Str, "+Inf");

  ಸ್ಥಿತಿ fcNaN: ಹಿಂತಿರುಗಿ append(Str, "NaN");

  ಸ್ಥಿತಿ fcZero:
    ಆದರೆ (isNegative())
      Str.ತಳ್ಳು_ಹಿಂಭಾಗ('-');

    ಆದರೆ (!FormatMaxPadding) {
      ಆದರೆ (TruncateZero)
        append(Str, "0.0E+0");
      ಇಲ್ಲದಿದ್ದರೆ {
        append(Str, "0.0");
        ಆದರೆ (FormatPrecision > 1)
          Str.append(FormatPrecision - 1, '0');
        append(Str, "e+00");
      }
    } ಇಲ್ಲದಿದ್ದರೆ {
      Str.ತಳ್ಳು_ಹಿಂಭಾಗ('0');
    }
    ಹಿಂತಿರುಗಿ;

  ಸ್ಥಿತಿ fc_ಸಾಮಾನ್ಯ:
    ನಿಲ್ಲಿಸು;
  }

  // Decompose the number into an APInt and an exponent.
  ಪೂರ್ಣಾಂಕ ಘಾತಾಂಕ = exponent - ((ಪೂರ್ಣಾಂಕ) semantics->precision - 1);
  APInt significand(
      semantics->precision,
      ArrayRef(significandParts(), part_ಎಣಿಕೆ_For_ಬಿಟ್ಟುಗಳು(semantics->precision)));

  ಗಮ್ಯ_String_Impl(Str, isNegative(), ಘಾತಾಂಕ, significand, FormatPrecision,
               FormatMaxPadding, TruncateZero);

}

ಪೂರ್ಣಾಂಕ IEEEFloat::getExactLog2Abs() ಸ್ಥಿರಾಂಕ {
  ಆದರೆ (!isFinite() || isZero())
    ಹಿಂತಿರುಗಿ INT_MIN;

  ಸ್ಥಿರಾಂಕ integerPart *Parts = significandParts();
  ಸ್ಥಿರಾಂಕ ಪೂರ್ಣಾಂಕ Part_ಎಣಿಕೆ = part_ಎಣಿಕೆ_For_ಬಿಟ್ಟುಗಳು(semantics->precision);

  ಪೂರ್ಣಾಂಕ ತೆಗೆ_ಎಣಿಕೆ = 0;
  ಗಾಗಿ (ಪೂರ್ಣಾಂಕ i = 0; i < Part_ಎಣಿಕೆ; ++i) {
    ತೆಗೆ_ಎಣಿಕೆ += llvm::popcount(Parts[i]);
    ಆದರೆ (ತೆಗೆ_ಎಣಿಕೆ > 1)
      ಹಿಂತಿರುಗಿ INT_MIN;
  }

  ಆದರೆ (exponent != semantics->minExponent)
    ಹಿಂತಿರುಗಿ exponent;

  ಪೂರ್ಣಾಂಕ CountrParts = 0;
  ಗಾಗಿ (ಪೂರ್ಣಾಂಕ i = 0; i < Part_ಎಣಿಕೆ;
       ++i, CountrParts += APInt::APINT_ಬಿಟ್ಟುಗಳು_ಪ್ರತಿ_ಶಬ್ದ) {
    ಆದರೆ (Parts[i] != 0) {
      ಹಿಂತಿರುಗಿ exponent - semantics->precision + CountrParts +
             llvm::countr_zero(Parts[i]) + 1;
    }
  }

  llvm_unreachable("didn't find the set bit");
}

ತಾರ್ಕಿಕ IEEEFloat::isSignaling() ಸ್ಥಿರಾಂಕ {
  ಆದರೆ (!isNaN())
    ಹಿಂತಿರುಗಿ ಸುಳ್ಳು;
  ಆದರೆ (semantics->nonFiniteBehavior == fltNonfiniteBehavior::NanOnly ||
      semantics->nonFiniteBehavior == fltNonfiniteBehavior::FiniteOnly)
    ಹಿಂತಿರುಗಿ ಸುಳ್ಳು;

  // IEEE-754R 2008 6.2.1: A signaling NaN bit string should be encoded with the
  // first bit of the trailing significand being 0.
  ಹಿಂತಿರುಗಿ !APInt::tc_ಹೊರತೆಗೆ_ಬಿಟ್(significandParts(), semantics->precision - 2);
}

/// IEEE-754R 2008 5.3.1: nextUp/nextDown.
///
/// *NOTE* since nextDown(x) = -nextUp(-x), we only implement nextUp with
/// appropriate sign switching before/after the computation.
APFloat::op_ಸ್ಥಿತಿ IEEEFloat::ಮುಂದಿನ(ತಾರ್ಕಿಕ ಮುಂದಿನ_ಕೆಳಕ್ಕೆ) {
  // If we are performing nextDown, swap sign so we have -x.
  ಆದರೆ (ಮುಂದಿನ_ಕೆಳಕ್ಕೆ)
    changeSign();

  // Compute nextUp(x)
  op_ಸ್ಥಿತಿ ಫಲಿತಾಂಶ_2 = opOK;

  // Handle each float category separately.
  ಆಯ್ಕೆ (category) {
  ಸ್ಥಿತಿ fcInfinity:
    // nextUp(+inf) = +inf
    ಆದರೆ (!isNegative())
      ನಿಲ್ಲಿಸು;
    // nextUp(-inf) = -getLargest()
    make_ಅತಿದೊಡ್ಡ(ನಿಜ);
    ನಿಲ್ಲಿಸು;
  ಸ್ಥಿತಿ fcNaN:
    // IEEE-754R 2008 6.2 Par 2: nextUp(sNaN) = qNaN. Set Invalid flag.
    // IEEE-754R 2008 6.2: nextUp(qNaN) = qNaN. Must be identity so we do not
    //                     change the payload.
    ಆದರೆ (isSignaling()) {
      ಫಲಿತಾಂಶ_2 = opInvalidOp;
      // For consistency, propagate the sign of the sNaN to the qNaN.
      makeNaN(ಸುಳ್ಳು, isNegative(), ಶೂನ್ಯಸೂಚಕ);
    }
    ನಿಲ್ಲಿಸು;
  ಸ್ಥಿತಿ fcZero:
    // nextUp(pm 0) = +getSmallest()
    make_ಅತಿಚಿಕ್ಕ(ಸುಳ್ಳು);
    ನಿಲ್ಲಿಸು;
  ಸ್ಥಿತಿ fc_ಸಾಮಾನ್ಯ:
    // nextUp(-getSmallest()) = -0
    ಆದರೆ (is_ಅತಿಚಿಕ್ಕ() && isNegative()) {
      APInt::tc_ಗಣ(significandParts(), 0, part_ಎಣಿಕೆ());
      category = fcZero;
      exponent = 0;
      ಆದರೆ (semantics->nanEncoding == fltNanEncoding::NegativeZero)
        sign = ಸುಳ್ಳು;
      ಆದರೆ (!semantics->hasZero)
        make_ಅತಿಚಿಕ್ಕ_Normalized(ಸುಳ್ಳು);
      ನಿಲ್ಲಿಸು;
    }

    ಆದರೆ (is_ಅತಿದೊಡ್ಡ() && !isNegative()) {
      ಆದರೆ (semantics->nonFiniteBehavior == fltNonfiniteBehavior::NanOnly) {
        // nextUp(getLargest()) == NAN
        makeNaN();
        ನಿಲ್ಲಿಸು;
      } ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (semantics->nonFiniteBehavior ==
                 fltNonfiniteBehavior::FiniteOnly) {
        // nextUp(getLargest()) == getLargest()
        ನಿಲ್ಲಿಸು;
      } ಇಲ್ಲದಿದ್ದರೆ {
        // nextUp(getLargest()) == INFINITY
        APInt::tc_ಗಣ(significandParts(), 0, part_ಎಣಿಕೆ());
        category = fcInfinity;
        exponent = semantics->maxExponent + 1;
        ನಿಲ್ಲಿಸು;
      }
    }

    // nextUp(normal) == normal + inc.
    ಆದರೆ (isNegative()) {
      // If we are negative, we need to decrement the significand.

      // We only cross a binade boundary that requires adjusting the exponent
      // if:
      //   1. exponent != semantics->minExponent. This implies we are not in the
      //   smallest binade or are dealing with denormals.
      //   2. Our significand excluding the integral bit is all zeros.
      ತಾರ್ಕಿಕ WillCrossBinadeBoundary =
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
      APInt::tcDecrement(Parts, part_ಎಣಿಕೆ());

      ಆದರೆ (WillCrossBinadeBoundary) {
        // Our result is a normal number. Do the following:
        // 1. Set the integral bit to 1.
        // 2. Decrement the exponent.
        APInt::tc_ಗಣ_ಬಿಟ್(Parts, semantics->precision - 1);
        exponent--;
      }
    } ಇಲ್ಲದಿದ್ದರೆ {
      // If we are positive, we need to increment the significand.

      // We only cross a binade boundary that requires adjusting the exponent if
      // the input is not a denormal and all of said input's significand bits
      // are set. If all of said conditions are true: clear the significand, set
      // the integral bit to 1, and increment the exponent. If we have a
      // denormal always increment since moving denormals and the numbers in the
      // smallest normal binade have the same exponent in our representation.
      // If there are only exponents, any increment always crosses the
      // BinadeBoundary.
      ತಾರ್ಕಿಕ WillCrossBinadeBoundary = !APFloat::hasSignificand(*semantics) ||
                                     (!isDenormal() && isSignificandAllOnes());

      ಆದರೆ (WillCrossBinadeBoundary) {
        integerPart *Parts = significandParts();
        APInt::tc_ಗಣ(Parts, 0, part_ಎಣಿಕೆ());
        APInt::tc_ಗಣ_ಬಿಟ್(Parts, semantics->precision - 1);
        ಪ್ರತಿಜ್ಞಾ(exponent != semantics->maxExponent &&
               "We can not increment an exponent beyond the maxExponent allowed"
               " by the given floating point semantics.");
        exponent++;
      } ಇಲ್ಲದಿದ್ದರೆ {
        incrementSignificand();
      }
    }
    ನಿಲ್ಲಿಸು;
  }

  // If we are performing nextDown, swap sign so we have -nextUp(-x)
  ಆದರೆ (ಮುಂದಿನ_ಕೆಳಕ್ಕೆ)
    changeSign();

  ಹಿಂತಿರುಗಿ ಫಲಿತಾಂಶ_2;
}

APInt IEEEFloat::getNaNPayload() ಸ್ಥಿರಾಂಕ {
  ಪ್ರತಿಜ್ಞಾ(isNaN() && "Can only be called on NaN values");
  // Number of bits in the payload, excluding the (maybe implied) integer bit.
  ಚಿಹ್ನೆರಹಿತ ಬಿಟ್ಟುಗಳು_2 = semantics->precision - 1;
  ಹಿಂತಿರುಗಿ APInt(ಬಿಟ್ಟುಗಳು_2, ArrayRef(significandParts(), part_ಎಣಿಕೆ_For_ಬಿಟ್ಟುಗಳು(ಬಿಟ್ಟುಗಳು_2)));
}

APFloat_ತಳ::ExponentType IEEEFloat::exponentNaN() ಸ್ಥಿರಾಂಕ {
  ಹಿಂತಿರುಗಿ ::exponentNaN(*semantics);
}

APFloat_ತಳ::ExponentType IEEEFloat::exponentInf() ಸ್ಥಿರಾಂಕ {
  ಹಿಂತಿರುಗಿ ::exponentInf(*semantics);
}

APFloat_ತಳ::ExponentType IEEEFloat::exponentZero() ಸ್ಥಿರಾಂಕ {
  ಹಿಂತಿರುಗಿ ::exponentZero(*semantics);
}

ಶೂನ್ಯ IEEEFloat::makeInf(ತಾರ್ಕಿಕ Negative) {
  ಆದರೆ (semantics->nonFiniteBehavior == fltNonfiniteBehavior::FiniteOnly)
    llvm_unreachable("This floating point format does not support Inf");

  ಆದರೆ (semantics->nonFiniteBehavior == fltNonfiniteBehavior::NanOnly) {
    // There is no Inf, so make NaN instead.
    makeNaN(ಸುಳ್ಳು, Negative);
    ಹಿಂತಿರುಗಿ;
  }
  category = fcInfinity;
  sign = Negative;
  exponent = exponentInf();
  APInt::tc_ಗಣ(significandParts(), 0, part_ಎಣಿಕೆ());
}

ಶೂನ್ಯ IEEEFloat::makeZero(ತಾರ್ಕಿಕ Negative) {
  ಆದರೆ (!semantics->hasZero)
    llvm_unreachable("This floating point format does not support Zero");

  category = fcZero;
  sign = Negative;
  ಆದರೆ (semantics->nanEncoding == fltNanEncoding::NegativeZero) {
    // Merge negative zero to positive because 0b10000...000 is used for NaN
    sign = ಸುಳ್ಳು;
  }
  exponent = exponentZero();
  APInt::tc_ಗಣ(significandParts(), 0, part_ಎಣಿಕೆ());
}

ಶೂನ್ಯ IEEEFloat::makeQuiet() {
  ಪ್ರತಿಜ್ಞಾ(isNaN());
  ಆದರೆ (semantics->nonFiniteBehavior != fltNonfiniteBehavior::NanOnly)
    APInt::tc_ಗಣ_ಬಿಟ್(significandParts(), semantics->precision - 2);
}

ಪೂರ್ಣಾಂಕ ilogb(ಸ್ಥಿರಾಂಕ IEEEFloat &Arg) {
  ಆದರೆ (Arg.isNaN())
    ಹಿಂತಿರುಗಿ APFloat::IEK_NaN;
  ಆದರೆ (Arg.isZero())
    ಹಿಂತಿರುಗಿ APFloat::IEK_Zero;
  ಆದರೆ (Arg.isInfinity())
    ಹಿಂತಿರುಗಿ APFloat::IEK_Inf;
  ಆದರೆ (!Arg.isDenormal())
    ಹಿಂತಿರುಗಿ Arg.exponent;

  IEEEFloat Normalized(Arg);
  ಪೂರ್ಣಾಂಕ Significand_ಬಿಟ್ಟುಗಳು = Arg.getSemantics().precision - 1;

  Normalized.exponent += Significand_ಬಿಟ್ಟುಗಳು;
  Normalized.normalize(APFloat::rm_ಹತ್ತಿರದ_Ties_ಗಮ್ಯ_Even, lfExactlyZero);
  ಹಿಂತಿರುಗಿ Normalized.exponent - Significand_ಬಿಟ್ಟುಗಳು;
}

IEEEFloat scalbn(IEEEFloat X, ಪೂರ್ಣಾಂಕ Exp, rounding_ಬಹುಲಕ Rounding_ಬಹುಲಕ) {
  ಸ್ವಯಂಚಾಲಿತ MaxExp = X.getSemantics().maxExponent;
  ಸ್ವಯಂಚಾಲಿತ MinExp = X.getSemantics().minExponent;

  // If Exp is wildly out-of-scale, simply adding it to X.exponent will
  // overflow; clamp it to a safe range before adding, but ensure that the range
  // is large enough that the clamp does not change the result. The range we
  // need to support is the difference between the largest possible exponent and
  // the normalized exponent of half the smallest denormal.

  ಪೂರ್ಣಾಂಕ Significand_ಬಿಟ್ಟುಗಳು = X.getSemantics().precision - 1;
  ಪೂರ್ಣಾಂಕ MaxIncrement = MaxExp - (MinExp - Significand_ಬಿಟ್ಟುಗಳು) + 1;

  // Clamp to one past the range ends to let normalize handle overlflow.
  X.exponent += std::clamp(Exp, -MaxIncrement - 1, MaxIncrement);
  X.normalize(Rounding_ಬಹುಲಕ, lfExactlyZero);
  ಆದರೆ (X.isNaN())
    X.makeQuiet();
  ಹಿಂತಿರುಗಿ X;
}

IEEEFloat frexp(ಸ್ಥಿರಾಂಕ IEEEFloat &Val, ಪೂರ್ಣಾಂಕ &Exp, rounding_ಬಹುಲಕ RM) {
  Exp = ilogb(Val);

  // Quiet signalling nans.
  ಆದರೆ (Exp == APFloat::IEK_NaN) {
    IEEEFloat Quiet(Val);
    Quiet.makeQuiet();
    ಹಿಂತಿರುಗಿ Quiet;
  }

  ಆದರೆ (Exp == APFloat::IEK_Inf)
    ಹಿಂತಿರುಗಿ Val;

  // 1 is added because frexp is defined to return a normalized fraction in
  // +/-[0.5, 1.0), rather than the usual +/-[1.0, 2.0).
  Exp = Exp == APFloat::IEK_Zero ? 0 : Exp + 1;
  ಹಿಂತಿರುಗಿ scalbn(Val, -Exp, RM);
}

DoubleAPFloat::DoubleAPFloat(ಸ್ಥಿರಾಂಕ fltSemantics &S)
    : Semantics(&S),
      Floats(ಹೊಸ APFloat[2]{APFloat(APFloat_ತಳ::semIEEEdouble),
                            APFloat(APFloat_ತಳ::semIEEEdouble)}) {
  ಪ್ರತಿಜ್ಞಾ(Semantics == &APFloat_ತಳ::semPPCDoubleDouble);
}

DoubleAPFloat::DoubleAPFloat(ಸ್ಥಿರಾಂಕ fltSemantics &S, uninitialized_ಟ್ಯಾಗ್)
    : Semantics(&S), Floats(ಹೊಸ APFloat[2]{
                         APFloat(APFloat_ತಳ::semIEEEdouble, uninitialized),
                         APFloat(APFloat_ತಳ::semIEEEdouble, uninitialized)}) {
  ಪ್ರತಿಜ್ಞಾ(Semantics == &APFloat_ತಳ::semPPCDoubleDouble);
}

DoubleAPFloat::DoubleAPFloat(ಸ್ಥಿರಾಂಕ fltSemantics &S, integerPart I)
    : Semantics(&S),
      Floats(ಹೊಸ APFloat[2]{APFloat(APFloat_ತಳ::semIEEEdouble, I),
                            APFloat(APFloat_ತಳ::semIEEEdouble)}) {
  ಪ್ರತಿಜ್ಞಾ(Semantics == &APFloat_ತಳ::semPPCDoubleDouble);
}

DoubleAPFloat::DoubleAPFloat(ಸ್ಥಿರಾಂಕ fltSemantics &S, ಸ್ಥಿರಾಂಕ APInt &I)
    : Semantics(&S),
      Floats(ಹೊಸ APFloat[2]{
          APFloat(APFloat_ತಳ::semIEEEdouble, APInt(64, I.get_Raw_ದತ್ತಾಂಶ()[0])),
          APFloat(APFloat_ತಳ::semIEEEdouble, APInt(64, I.get_Raw_ದತ್ತಾಂಶ()[1]))}) {
  ಪ್ರತಿಜ್ಞಾ(Semantics == &APFloat_ತಳ::semPPCDoubleDouble);
}

DoubleAPFloat::DoubleAPFloat(ಸ್ಥಿರಾಂಕ fltSemantics &S, APFloat &&ಮೊದಲ,
                             APFloat &&ಎರಡನೇ)
    : Semantics(&S),
      Floats(ಹೊಸ APFloat[2]{std::move(ಮೊದಲ), std::move(ಎರಡನೇ)}) {
  ಪ್ರತಿಜ್ಞಾ(Semantics == &APFloat_ತಳ::semPPCDoubleDouble);
  ಪ್ರತಿಜ್ಞಾ(&Floats[0].getSemantics() == &APFloat_ತಳ::semIEEEdouble);
  ಪ್ರತಿಜ್ಞಾ(&Floats[1].getSemantics() == &APFloat_ತಳ::semIEEEdouble);
}

DoubleAPFloat::DoubleAPFloat(ಸ್ಥಿರಾಂಕ DoubleAPFloat &RHS)
    : Semantics(RHS.Semantics),
      Floats(RHS.Floats ? ಹೊಸ APFloat[2]{APFloat(RHS.Floats[0]),
                                         APFloat(RHS.Floats[1])}
                        : ಶೂನ್ಯಸೂಚಕ) {
  ಪ್ರತಿಜ್ಞಾ(Semantics == &APFloat_ತಳ::semPPCDoubleDouble);
}

DoubleAPFloat::DoubleAPFloat(DoubleAPFloat &&RHS)
    : Semantics(RHS.Semantics), Floats(RHS.Floats) {
  RHS.Semantics = &APFloat_ತಳ::semBogus;
  RHS.Floats = ಶೂನ್ಯಸೂಚಕ;
  ಪ್ರತಿಜ್ಞಾ(Semantics == &APFloat_ತಳ::semPPCDoubleDouble);
}

DoubleAPFloat &DoubleAPFloat::ಕ್ರಿಯಾಕಾರಕ=(ಸ್ಥಿರಾಂಕ DoubleAPFloat &RHS) {
  ಆದರೆ (Semantics == RHS.Semantics && RHS.Floats) {
    Floats[0] = RHS.Floats[0];
    Floats[1] = RHS.Floats[1];
  } ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (ಇದು != &RHS) {
    ಇದು->~DoubleAPFloat();
    ಹೊಸ (ಇದು) DoubleAPFloat(RHS);
  }
  ಹಿಂತಿರುಗಿ *ಇದು;
}

// Returns a result such that:
// 1. abs(Lo) <= ulp(Hi)/2
// 2. Hi == RTNE(Hi + Lo)
// 3. Hi + Lo == X + Y
//
// Requires that log2(X) >= log2(Y).
ಸ್ಥಿರ std::pair<APFloat, APFloat> fast_Two_ಮೊತ್ತ(APFloat X, APFloat Y) {
  ಆದರೆ (!X.isFinite())
    ಹಿಂತಿರುಗಿ {X, APFloat::getZero(X.getSemantics(), /*Negative=*/ಸುಳ್ಳು)};
  APFloat Hi = X + Y;
  APFloat Delta = Hi - X;
  APFloat Lo = Y - Delta;
  ಹಿಂತಿರುಗಿ {Hi, Lo};
}

// Implement addition, subtraction, multiplication and division based on:
// "Software for Doubled-Precision Floating-Point Computations",
// by Seppo Linnainmaa, ACM TOMS vol 7 no 3, September 1981, pages 272-283.
APFloat::op_ಸ್ಥಿತಿ DoubleAPFloat::ಕೂಡಿಸು_Impl(ಸ್ಥಿರಾಂಕ APFloat &a, ಸ್ಥಿರಾಂಕ APFloat &aa,
                                         ಸ್ಥಿರಾಂಕ APFloat &c, ಸ್ಥಿರಾಂಕ APFloat &cc,
                                         rounding_ಬಹುಲಕ RM) {
  ಪೂರ್ಣಾಂಕ ಸ್ಥಿತಿ_2 = opOK;
  APFloat z = a;
  ಸ್ಥಿತಿ_2 |= z.ಕೂಡಿಸು(c, RM);
  ಆದರೆ (!z.isFinite()) {
    ಆದರೆ (!z.isInfinity()) {
      Floats[0] = std::move(z);
      Floats[1].makeZero(/* Neg = */ ಸುಳ್ಳು);
      ಹಿಂತಿರುಗಿ (op_ಸ್ಥಿತಿ)ಸ್ಥಿತಿ_2;
    }
    ಸ್ಥಿತಿ_2 = opOK;
    ಸ್ವಯಂಚಾಲಿತ ACompared_ಗಮ್ಯ_C = a.compare_Absolute_ಮೌಲ್ಯ(c);
    z = cc;
    ಸ್ಥಿತಿ_2 |= z.ಕೂಡಿಸು(aa, RM);
    ಆದರೆ (ACompared_ಗಮ್ಯ_C == APFloat::cmpGreaterThan) {
      // z = cc + aa + c + a;
      ಸ್ಥಿತಿ_2 |= z.ಕೂಡಿಸು(c, RM);
      ಸ್ಥಿತಿ_2 |= z.ಕೂಡಿಸು(a, RM);
    } ಇಲ್ಲದಿದ್ದರೆ {
      // z = cc + aa + a + c;
      ಸ್ಥಿತಿ_2 |= z.ಕೂಡಿಸು(a, RM);
      ಸ್ಥಿತಿ_2 |= z.ಕೂಡಿಸು(c, RM);
    }
    ಆದರೆ (!z.isFinite()) {
      Floats[0] = std::move(z);
      Floats[1].makeZero(/* Neg = */ ಸುಳ್ಳು);
      ಹಿಂತಿರುಗಿ (op_ಸ್ಥಿತಿ)ಸ್ಥಿತಿ_2;
    }
    Floats[0] = z;
    APFloat zz = aa;
    ಸ್ಥಿತಿ_2 |= zz.ಕೂಡಿಸು(cc, RM);
    ಆದರೆ (ACompared_ಗಮ್ಯ_C == APFloat::cmpGreaterThan) {
      // Floats[1] = a - z + c + zz;
      Floats[1] = a;
      ಸ್ಥಿತಿ_2 |= Floats[1].ಕಳೆ(z, RM);
      ಸ್ಥಿತಿ_2 |= Floats[1].ಕೂಡಿಸು(c, RM);
      ಸ್ಥಿತಿ_2 |= Floats[1].ಕೂಡಿಸು(zz, RM);
    } ಇಲ್ಲದಿದ್ದರೆ {
      // Floats[1] = c - z + a + zz;
      Floats[1] = c;
      ಸ್ಥಿತಿ_2 |= Floats[1].ಕಳೆ(z, RM);
      ಸ್ಥಿತಿ_2 |= Floats[1].ಕೂಡಿಸು(a, RM);
      ಸ್ಥಿತಿ_2 |= Floats[1].ಕೂಡಿಸು(zz, RM);
    }
  } ಇಲ್ಲದಿದ್ದರೆ {
    // q = a - z;
    APFloat q = a;
    ಸ್ಥಿತಿ_2 |= q.ಕಳೆ(z, RM);

    // zz = q + c + (a - (q + z)) + aa + cc;
    // Compute a - (q + z) as -((q + z) - a) to avoid temporary copies.
    ಸ್ವಯಂಚಾಲಿತ zz = q;
    ಸ್ಥಿತಿ_2 |= zz.ಕೂಡಿಸು(c, RM);
    ಸ್ಥಿತಿ_2 |= q.ಕೂಡಿಸು(z, RM);
    ಸ್ಥಿತಿ_2 |= q.ಕಳೆ(a, RM);
    q.changeSign();
    ಸ್ಥಿತಿ_2 |= zz.ಕೂಡಿಸು(q, RM);
    ಸ್ಥಿತಿ_2 |= zz.ಕೂಡಿಸು(aa, RM);
    ಸ್ಥಿತಿ_2 |= zz.ಕೂಡಿಸು(cc, RM);
    ಆದರೆ (zz.isZero() && !zz.isNegative()) {
      Floats[0] = std::move(z);
      Floats[1].makeZero(/* Neg = */ ಸುಳ್ಳು);
      ಹಿಂತಿರುಗಿ opOK;
    }
    Floats[0] = z;
    ಸ್ಥಿತಿ_2 |= Floats[0].ಕೂಡಿಸು(zz, RM);
    ಆದರೆ (!Floats[0].isFinite()) {
      Floats[1].makeZero(/* Neg = */ ಸುಳ್ಳು);
      ಹಿಂತಿರುಗಿ (op_ಸ್ಥಿತಿ)ಸ್ಥಿತಿ_2;
    }
    Floats[1] = std::move(z);
    ಸ್ಥಿತಿ_2 |= Floats[1].ಕಳೆ(Floats[0], RM);
    ಸ್ಥಿತಿ_2 |= Floats[1].ಕೂಡಿಸು(zz, RM);
  }
  ಹಿಂತಿರುಗಿ (op_ಸ್ಥಿತಿ)ಸ್ಥಿತಿ_2;
}

APFloat::op_ಸ್ಥಿತಿ DoubleAPFloat::ಕೂಡಿಸು_With_Special(ಸ್ಥಿರಾಂಕ DoubleAPFloat &LHS,
                                                ಸ್ಥಿರಾಂಕ DoubleAPFloat &RHS,
                                                DoubleAPFloat &ಹೊರಹೋಗು,
                                                rounding_ಬಹುಲಕ RM) {
  ಆದರೆ (LHS.getCategory() == fcNaN) {
    ಹೊರಹೋಗು = LHS;
    ಹಿಂತಿರುಗಿ opOK;
  }
  ಆದರೆ (RHS.getCategory() == fcNaN) {
    ಹೊರಹೋಗು = RHS;
    ಹಿಂತಿರುಗಿ opOK;
  }
  ಆದರೆ (LHS.getCategory() == fcZero) {
    ಹೊರಹೋಗು = RHS;
    ಹಿಂತಿರುಗಿ opOK;
  }
  ಆದರೆ (RHS.getCategory() == fcZero) {
    ಹೊರಹೋಗು = LHS;
    ಹಿಂತಿರುಗಿ opOK;
  }
  ಆದರೆ (LHS.getCategory() == fcInfinity && RHS.getCategory() == fcInfinity &&
      LHS.isNegative() != RHS.isNegative()) {
    ಹೊರಹೋಗು.makeNaN(ಸುಳ್ಳು, ಹೊರಹೋಗು.isNegative(), ಶೂನ್ಯಸೂಚಕ);
    ಹಿಂತಿರುಗಿ opInvalidOp;
  }
  ಆದರೆ (LHS.getCategory() == fcInfinity) {
    ಹೊರಹೋಗು = LHS;
    ಹಿಂತಿರುಗಿ opOK;
  }
  ಆದರೆ (RHS.getCategory() == fcInfinity) {
    ಹೊರಹೋಗು = RHS;
    ಹಿಂತಿರುಗಿ opOK;
  }
  ಪ್ರತಿಜ್ಞಾ(LHS.getCategory() == fc_ಸಾಮಾನ್ಯ && RHS.getCategory() == fc_ಸಾಮಾನ್ಯ);

  APFloat A(LHS.Floats[0]), AA(LHS.Floats[1]), C(RHS.Floats[0]),
      CC(RHS.Floats[1]);
  ಪ್ರತಿಜ್ಞಾ(&A.getSemantics() == &APFloat_ತಳ::semIEEEdouble);
  ಪ್ರತಿಜ್ಞಾ(&AA.getSemantics() == &APFloat_ತಳ::semIEEEdouble);
  ಪ್ರತಿಜ್ಞಾ(&C.getSemantics() == &APFloat_ತಳ::semIEEEdouble);
  ಪ್ರತಿಜ್ಞಾ(&CC.getSemantics() == &APFloat_ತಳ::semIEEEdouble);
  ಪ್ರತಿಜ್ಞಾ(&ಹೊರಹೋಗು.Floats[0].getSemantics() == &APFloat_ತಳ::semIEEEdouble);
  ಪ್ರತಿಜ್ಞಾ(&ಹೊರಹೋಗು.Floats[1].getSemantics() == &APFloat_ತಳ::semIEEEdouble);
  ಹಿಂತಿರುಗಿ ಹೊರಹೋಗು.ಕೂಡಿಸು_Impl(A, AA, C, CC, RM);
}

APFloat::op_ಸ್ಥಿತಿ DoubleAPFloat::ಕೂಡಿಸು(ಸ್ಥಿರಾಂಕ DoubleAPFloat &RHS,
                                     rounding_ಬಹುಲಕ RM) {
  ಹಿಂತಿರುಗಿ ಕೂಡಿಸು_With_Special(*ಇದು, RHS, *ಇದು, RM);
}

APFloat::op_ಸ್ಥಿತಿ DoubleAPFloat::ಕಳೆ(ಸ್ಥಿರಾಂಕ DoubleAPFloat &RHS,
                                          rounding_ಬಹುಲಕ RM) {
  changeSign();
  ಸ್ವಯಂಚಾಲಿತ Ret = ಕೂಡಿಸು(RHS, RM);
  changeSign();
  ಹಿಂತಿರುಗಿ Ret;
}

APFloat::op_ಸ್ಥಿತಿ DoubleAPFloat::multiply(ಸ್ಥಿರಾಂಕ DoubleAPFloat &RHS,
                                          APFloat::rounding_ಬಹುಲಕ RM) {
  ಸ್ಥಿರಾಂಕ ಸ್ವಯಂಚಾಲಿತ &LHS = *ಇದು;
  ಸ್ವಯಂಚಾಲಿತ &ಹೊರಹೋಗು = *ಇದು;
  /* Interesting observation: For special categories, finding the lowest
     common ancestor of the following layered graph gives the correct
     return category:

        NaN
       /   \
     Zero  Inf
       \   /
ಸಾಮಾನ್ಯ

     e.g. NaN * NaN = NaN
          Zero * Inf = NaN
          Normal * Zero = Zero
          Normal * Inf = Inf
  */
  ಆದರೆ (LHS.getCategory() == fcNaN) {
    ಹೊರಹೋಗು = LHS;
    ಹಿಂತಿರುಗಿ opOK;
  }
  ಆದರೆ (RHS.getCategory() == fcNaN) {
    ಹೊರಹೋಗು = RHS;
    ಹಿಂತಿರುಗಿ opOK;
  }
  ಆದರೆ ((LHS.getCategory() == fcZero && RHS.getCategory() == fcInfinity) ||
      (LHS.getCategory() == fcInfinity && RHS.getCategory() == fcZero)) {
    ಹೊರಹೋಗು.makeNaN(ಸುಳ್ಳು, ಸುಳ್ಳು, ಶೂನ್ಯಸೂಚಕ);
    ಹಿಂತಿರುಗಿ opOK;
  }
  ಆದರೆ (LHS.getCategory() == fcZero || LHS.getCategory() == fcInfinity) {
    ಹೊರಹೋಗು = LHS;
    ಹಿಂತಿರುಗಿ opOK;
  }
  ಆದರೆ (RHS.getCategory() == fcZero || RHS.getCategory() == fcInfinity) {
    ಹೊರಹೋಗು = RHS;
    ಹಿಂತಿರುಗಿ opOK;
  }
  ಪ್ರತಿಜ್ಞಾ(LHS.getCategory() == fc_ಸಾಮಾನ್ಯ && RHS.getCategory() == fc_ಸಾಮಾನ್ಯ &&
         "Special cases not handled exhaustively");

  ಪೂರ್ಣಾಂಕ ಸ್ಥಿತಿ_2 = opOK;
  APFloat A = Floats[0], B = Floats[1], C = RHS.Floats[0], D = RHS.Floats[1];
  // t = a * c
  APFloat T = A;
  ಸ್ಥಿತಿ_2 |= T.multiply(C, RM);
  ಆದರೆ (!T.isFiniteNonZero()) {
    Floats[0] = std::move(T);
    Floats[1].makeZero(/* Neg = */ ಸುಳ್ಳು);
    ಹಿಂತಿರುಗಿ (op_ಸ್ಥಿತಿ)ಸ್ಥಿತಿ_2;
  }

  // tau = fmsub(a, c, t), that is -fmadd(-a, c, t).
  APFloat Tau = A;
  T.changeSign();
  ಸ್ಥಿತಿ_2 |= Tau.fused_Multiply_ಕೂಡಿಸು(C, T, RM);
  T.changeSign();
  {
    // v = a * d
    APFloat V = A;
    ಸ್ಥಿತಿ_2 |= V.multiply(D, RM);
    // w = b * c
    APFloat W = B;
    ಸ್ಥಿತಿ_2 |= W.multiply(C, RM);
    ಸ್ಥಿತಿ_2 |= V.ಕೂಡಿಸು(W, RM);
    // tau += v + w
    ಸ್ಥಿತಿ_2 |= Tau.ಕೂಡಿಸು(V, RM);
  }
  // u = t + tau
  APFloat U = T;
  ಸ್ಥಿತಿ_2 |= U.ಕೂಡಿಸು(Tau, RM);

  Floats[0] = U;
  ಆದರೆ (!U.isFinite()) {
    Floats[1].makeZero(/* Neg = */ ಸುಳ್ಳು);
  } ಇಲ್ಲದಿದ್ದರೆ {
    // Floats[1] = (t - u) + tau
    ಸ್ಥಿತಿ_2 |= T.ಕಳೆ(U, RM);
    ಸ್ಥಿತಿ_2 |= T.ಕೂಡಿಸು(Tau, RM);
    Floats[1] = std::move(T);
  }
  ಹಿಂತಿರುಗಿ (op_ಸ್ಥಿತಿ)ಸ್ಥಿತಿ_2;
}

APFloat::op_ಸ್ಥಿತಿ DoubleAPFloat::divide(ಸ್ಥಿರಾಂಕ DoubleAPFloat &RHS,
                                        APFloat::rounding_ಬಹುಲಕ RM) {
  ಪ್ರತಿಜ್ಞಾ(Semantics == &APFloat_ತಳ::semPPCDoubleDouble &&
         "Unexpected Semantics");
  APFloat Tmp(APFloat_ತಳ::semPPCDoubleDoubleLegacy, bitcast_ಗಮ್ಯ_APInt());
  ಸ್ವಯಂಚಾಲಿತ Ret = Tmp.divide(
      APFloat(APFloat_ತಳ::semPPCDoubleDoubleLegacy, RHS.bitcast_ಗಮ್ಯ_APInt()), RM);
  *ಇದು = DoubleAPFloat(APFloat_ತಳ::semPPCDoubleDouble, Tmp.bitcast_ಗಮ್ಯ_APInt());
  ಹಿಂತಿರುಗಿ Ret;
}

APFloat::op_ಸ್ಥಿತಿ DoubleAPFloat::remainder(ಸ್ಥಿರಾಂಕ DoubleAPFloat &RHS) {
  ಪ್ರತಿಜ್ಞಾ(Semantics == &APFloat_ತಳ::semPPCDoubleDouble &&
         "Unexpected Semantics");
  APFloat Tmp(APFloat_ತಳ::semPPCDoubleDoubleLegacy, bitcast_ಗಮ್ಯ_APInt());
  ಸ್ವಯಂಚಾಲಿತ Ret = Tmp.remainder(
      APFloat(APFloat_ತಳ::semPPCDoubleDoubleLegacy, RHS.bitcast_ಗಮ್ಯ_APInt()));
  *ಇದು = DoubleAPFloat(APFloat_ತಳ::semPPCDoubleDouble, Tmp.bitcast_ಗಮ್ಯ_APInt());
  ಹಿಂತಿರುಗಿ Ret;
}

APFloat::op_ಸ್ಥಿತಿ DoubleAPFloat::mod(ಸ್ಥಿರಾಂಕ DoubleAPFloat &RHS) {
  ಪ್ರತಿಜ್ಞಾ(Semantics == &APFloat_ತಳ::semPPCDoubleDouble &&
         "Unexpected Semantics");
  APFloat Tmp(APFloat_ತಳ::semPPCDoubleDoubleLegacy, bitcast_ಗಮ್ಯ_APInt());
  ಸ್ವಯಂಚಾಲಿತ Ret = Tmp.mod(
      APFloat(APFloat_ತಳ::semPPCDoubleDoubleLegacy, RHS.bitcast_ಗಮ್ಯ_APInt()));
  *ಇದು = DoubleAPFloat(APFloat_ತಳ::semPPCDoubleDouble, Tmp.bitcast_ಗಮ್ಯ_APInt());
  ಹಿಂತಿರುಗಿ Ret;
}

APFloat::op_ಸ್ಥಿತಿ
DoubleAPFloat::fused_Multiply_ಕೂಡಿಸು(ಸ್ಥಿರಾಂಕ DoubleAPFloat &Multiplicand,
                                ಸ್ಥಿರಾಂಕ DoubleAPFloat &Addend,
                                APFloat::rounding_ಬಹುಲಕ RM) {
  ಪ್ರತಿಜ್ಞಾ(Semantics == &APFloat_ತಳ::semPPCDoubleDouble &&
         "Unexpected Semantics");
  APFloat Tmp(APFloat_ತಳ::semPPCDoubleDoubleLegacy, bitcast_ಗಮ್ಯ_APInt());
  ಸ್ವಯಂಚಾಲಿತ Ret = Tmp.fused_Multiply_ಕೂಡಿಸು(
      APFloat(APFloat_ತಳ::semPPCDoubleDoubleLegacy,
              Multiplicand.bitcast_ಗಮ್ಯ_APInt()),
      APFloat(APFloat_ತಳ::semPPCDoubleDoubleLegacy, Addend.bitcast_ಗಮ್ಯ_APInt()),
      RM);
  *ಇದು = DoubleAPFloat(APFloat_ತಳ::semPPCDoubleDouble, Tmp.bitcast_ಗಮ್ಯ_APInt());
  ಹಿಂತಿರುಗಿ Ret;
}

APFloat::op_ಸ್ಥಿತಿ DoubleAPFloat::ಸುತ್ತು_ಗಮ್ಯ_Integral(APFloat::rounding_ಬಹುಲಕ RM) {
  ಪ್ರತಿಜ್ಞಾ(Semantics == &APFloat_ತಳ::semPPCDoubleDouble &&
         "Unexpected Semantics");
  ಸ್ಥಿರಾಂಕ APFloat &Hi = get_ಮೊದಲ();
  ಸ್ಥಿರಾಂಕ APFloat &Lo = get_ಎರಡನೇ();

  APFloat RoundedHi = Hi;
  ಸ್ಥಿರಾಂಕ op_ಸ್ಥಿತಿ Hi_ಸ್ಥಿತಿ = RoundedHi.ಸುತ್ತು_ಗಮ್ಯ_Integral(RM);

  // We can reduce the problem to just the high part if the input:
  // 1. Represents a non-finite value.
  // 2. Has a component which is zero.
  ಆದರೆ (!Hi.isFiniteNonZero() || Lo.isZero()) {
    Floats[0] = std::move(RoundedHi);
    Floats[1].makeZero(/*Neg=*/ಸುಳ್ಳು);
    ಹಿಂತಿರುಗಿ Hi_ಸ್ಥಿತಿ;
  }

  // Adjust `Rounded` in the direction of `TieBreaker` if `ToRound` was at a
  // halfway point.
  ಸ್ವಯಂಚಾಲಿತ ಸುತ್ತು_ಗಮ್ಯ_ಹತ್ತಿರದ_Helper = [](APFloat ಗಮ್ಯ_ಸುತ್ತು, APFloat Rounded,
                                 APFloat TieBreaker) {
    // RoundingError tells us which direction we rounded:
    //   - RoundingError > 0: we rounded up.
    //   - RoundingError < 0: we rounded down.
    // Sterbenz' lemma ensures that RoundingError is exact.
    ಸ್ಥಿರಾಂಕ APFloat RoundingError = Rounded - ಗಮ್ಯ_ಸುತ್ತು;
    ಆದರೆ (TieBreaker.isNonZero() &&
        TieBreaker.isNegative() != RoundingError.isNegative() &&
        ನಿರಪೇಕ್ಷ(RoundingError).is_Exactly_ಮೌಲ್ಯ(0.5))
      Rounded.ಕೂಡಿಸು(
          APFloat::getOne(Rounded.getSemantics(), TieBreaker.isNegative()),
          rm_ಹತ್ತಿರದ_Ties_ಗಮ್ಯ_Even);
    ಹಿಂತಿರುಗಿ Rounded;
  };

  // Case 1: Hi is not an integer.
  // Special cases are for rounding modes that are sensitive to ties.
  ಆದರೆ (RoundedHi != Hi) {
    // We need to consider the case where Hi was between two integers and the
    // rounding mode broke the tie when, in fact, Lo may have had a different
    // sign than Hi.
    ಆದರೆ (RM == rm_ಹತ್ತಿರದ_Ties_ಗಮ್ಯ_Away || RM == rm_ಹತ್ತಿರದ_Ties_ಗಮ್ಯ_Even)
      RoundedHi = ಸುತ್ತು_ಗಮ್ಯ_ಹತ್ತಿರದ_Helper(Hi, RoundedHi, Lo);

    Floats[0] = std::move(RoundedHi);
    Floats[1].makeZero(/*Neg=*/ಸುಳ್ಳು);
    ಹಿಂತಿರುಗಿ Hi_ಸ್ಥಿತಿ;
  }

  // Case 2: Hi is an integer.
  // Special cases are for rounding modes which are rounding towards or away from zero.
  Rounding_ಬಹುಲಕ Lo_Rounding_ಬಹುಲಕ;
  ಆದರೆ (RM == rmTowardZero)
    // When our input is positive, we want the Lo component rounded toward
    // negative infinity to get the smallest result magnitude. Likewise,
    // negative inputs want the Lo component rounded toward positive infinity.
    Lo_Rounding_ಬಹುಲಕ = isNegative() ? rm_Toward_ಧನಾತ್ಮಕ : rmTowardNegative;
  ಇಲ್ಲದಿದ್ದರೆ
    Lo_Rounding_ಬಹುಲಕ = RM;

  APFloat RoundedLo = Lo;
  ಸ್ಥಿರಾಂಕ op_ಸ್ಥಿತಿ Lo_ಸ್ಥಿತಿ = RoundedLo.ಸುತ್ತು_ಗಮ್ಯ_Integral(Lo_Rounding_ಬಹುಲಕ);
  ಆದರೆ (Lo_Rounding_ಬಹುಲಕ == rm_ಹತ್ತಿರದ_Ties_ಗಮ್ಯ_Away)
    // We need to consider the case where Lo was between two integers and the
    // rounding mode broke the tie when, in fact, Hi may have had a different
    // sign than Lo.
    RoundedLo = ಸುತ್ತು_ಗಮ್ಯ_ಹತ್ತಿರದ_Helper(Lo, RoundedLo, Hi);

  // We must ensure that the final result has no overlap between the two APFloat values.
  std::tie(RoundedHi, RoundedLo) = fast_Two_ಮೊತ್ತ(RoundedHi, RoundedLo);

  Floats[0] = std::move(RoundedHi);
  Floats[1] = std::move(RoundedLo);
  ಹಿಂತಿರುಗಿ Lo_ಸ್ಥಿತಿ;
}

ಶೂನ್ಯ DoubleAPFloat::changeSign() {
  Floats[0].changeSign();
  Floats[1].changeSign();
}

APFloat::cmp_ಫಲಿತಾಂಶ
DoubleAPFloat::compare_Absolute_ಮೌಲ್ಯ(ಸ್ಥಿರಾಂಕ DoubleAPFloat &RHS) ಸ್ಥಿರಾಂಕ {
  // Compare absolute values of the high parts.
  ಸ್ಥಿರಾಂಕ cmp_ಫಲಿತಾಂಶ HiPartCmp = Floats[0].compare_Absolute_ಮೌಲ್ಯ(RHS.Floats[0]);
  ಆದರೆ (HiPartCmp != cmpEqual)
    ಹಿಂತಿರುಗಿ HiPartCmp;

  // Zero, regardless of sign, is equal.
  ಆದರೆ (Floats[1].isZero() && RHS.Floats[1].isZero())
    ಹಿಂತಿರುಗಿ cmpEqual;

  // At this point, |this->Hi| == |RHS.Hi|.
  // The magnitude is |Hi+Lo| which is Hi+|Lo| if signs of Hi and Lo are the
  // same, and Hi-|Lo| if signs are different.
  ಸ್ಥಿರಾಂಕ ತಾರ್ಕಿಕ ThisIsSubtractive =
      Floats[0].isNegative() != Floats[1].isNegative();
  ಸ್ಥಿರಾಂಕ ತಾರ್ಕಿಕ RHSIsSubtractive =
      RHS.Floats[0].isNegative() != RHS.Floats[1].isNegative();

  // Case 1: The low part of 'this' is zero.
  ಆದರೆ (Floats[1].isZero())
    // We are comparing |Hi| vs. |Hi| ± |RHS.Lo|.
    // If RHS is subtractive, its magnitude is smaller.
    // If RHS is additive, its magnitude is larger.
    ಹಿಂತಿರುಗಿ RHSIsSubtractive ? cmpGreaterThan : cmpLessThan;

  // Case 2: The low part of 'RHS' is zero (and we know 'this' is not).
  ಆದರೆ (RHS.Floats[1].isZero())
    // We are comparing |Hi| ± |This.Lo| vs. |Hi|.
    // If 'this' is subtractive, its magnitude is smaller.
    // If 'this' is additive, its magnitude is larger.
    ಹಿಂತಿರುಗಿ ThisIsSubtractive ? cmpLessThan : cmpGreaterThan;

  // If their natures differ, the additive one is larger.
  ಆದರೆ (ThisIsSubtractive != RHSIsSubtractive)
    ಹಿಂತಿರುಗಿ ThisIsSubtractive ? cmpLessThan : cmpGreaterThan;

  // Case 3: Both are additive (Hi+|Lo|) or both are subtractive (Hi-|Lo|).
  // The comparison now depends on the magnitude of the low parts.
  ಸ್ಥಿರಾಂಕ cmp_ಫಲಿತಾಂಶ LoPartCmp = Floats[1].compare_Absolute_ಮೌಲ್ಯ(RHS.Floats[1]);

  ಆದರೆ (ThisIsSubtractive) {
    // Both are subtractive (Hi-|Lo|), so the comparison of |Lo| is inverted.
    ಆದರೆ (LoPartCmp == cmpLessThan)
      ಹಿಂತಿರುಗಿ cmpGreaterThan;
    ಆದರೆ (LoPartCmp == cmpGreaterThan)
      ಹಿಂತಿರುಗಿ cmpLessThan;
  }

  // If additive, the comparison of |Lo| is direct.
  // If equal, they are equal.
  ಹಿಂತಿರುಗಿ LoPartCmp;
}

APFloat::fltCategory DoubleAPFloat::getCategory() ಸ್ಥಿರಾಂಕ {
  ಹಿಂತಿರುಗಿ Floats[0].getCategory();
}

ತಾರ್ಕಿಕ DoubleAPFloat::isNegative() ಸ್ಥಿರಾಂಕ { ಹಿಂತಿರುಗಿ Floats[0].isNegative(); }

ಶೂನ್ಯ DoubleAPFloat::makeInf(ತಾರ್ಕಿಕ Neg) {
  Floats[0].makeInf(Neg);
  Floats[1].makeZero(/* Neg = */ ಸುಳ್ಳು);
}

ಶೂನ್ಯ DoubleAPFloat::makeZero(ತಾರ್ಕಿಕ Neg) {
  Floats[0].makeZero(Neg);
  Floats[1].makeZero(/* Neg = */ ಸುಳ್ಳು);
}

ಶೂನ್ಯ DoubleAPFloat::make_ಅತಿದೊಡ್ಡ(ತಾರ್ಕಿಕ Neg) {
  ಪ್ರತಿಜ್ಞಾ(Semantics == &APFloat_ತಳ::semPPCDoubleDouble &&
         "Unexpected Semantics");
  Floats[0] =
      APFloat(APFloat_ತಳ::semIEEEdouble, APInt(64, 0x7fefffffffffffffull));
  Floats[1] =
      APFloat(APFloat_ತಳ::semIEEEdouble, APInt(64, 0x7c8ffffffffffffeull));
  ಆದರೆ (Neg)
    changeSign();
}

ಶೂನ್ಯ DoubleAPFloat::make_ಅತಿಚಿಕ್ಕ(ತಾರ್ಕಿಕ Neg) {
  ಪ್ರತಿಜ್ಞಾ(Semantics == &APFloat_ತಳ::semPPCDoubleDouble &&
         "Unexpected Semantics");
  Floats[0].make_ಅತಿಚಿಕ್ಕ(Neg);
  Floats[1].makeZero(/* Neg = */ ಸುಳ್ಳು);
}

ಶೂನ್ಯ DoubleAPFloat::make_ಅತಿಚಿಕ್ಕ_Normalized(ತಾರ್ಕಿಕ Neg) {
  ಪ್ರತಿಜ್ಞಾ(Semantics == &APFloat_ತಳ::semPPCDoubleDouble &&
         "Unexpected Semantics");
  Floats[0] =
      APFloat(APFloat_ತಳ::semIEEEdouble, APInt(64, 0x0360000000000000ull));
  ಆದರೆ (Neg)
    Floats[0].changeSign();
  Floats[1].makeZero(/* Neg = */ ಸುಳ್ಳು);
}

ಶೂನ್ಯ DoubleAPFloat::makeNaN(ತಾರ್ಕಿಕ SNaN, ತಾರ್ಕಿಕ Neg, ಸ್ಥಿರಾಂಕ APInt *fill) {
  Floats[0].makeNaN(SNaN, Neg, fill);
  Floats[1].makeZero(/* Neg = */ ಸುಳ್ಳು);
}

APFloat::cmp_ಫಲಿತಾಂಶ DoubleAPFloat::compare(ಸ್ಥಿರಾಂಕ DoubleAPFloat &RHS) ಸ್ಥಿರಾಂಕ {
  ಸ್ವಯಂಚಾಲಿತ ಫಲಿತಾಂಶ = Floats[0].compare(RHS.Floats[0]);
  // |Float[0]| > |Float[1]|
  ಆದರೆ (ಫಲಿತಾಂಶ == APFloat::cmpEqual)
    ಹಿಂತಿರುಗಿ Floats[1].compare(RHS.Floats[1]);
  ಹಿಂತಿರುಗಿ ಫಲಿತಾಂಶ;
}

ತಾರ್ಕಿಕ DoubleAPFloat::bitwiseIsEqual(ಸ್ಥಿರಾಂಕ DoubleAPFloat &RHS) ಸ್ಥಿರಾಂಕ {
  ಹಿಂತಿರುಗಿ Floats[0].bitwiseIsEqual(RHS.Floats[0]) &&
         Floats[1].bitwiseIsEqual(RHS.Floats[1]);
}

hash_ಸಂಕೇತ hash_ಮೌಲ್ಯ(ಸ್ಥಿರಾಂಕ DoubleAPFloat &Arg) {
  ಆದರೆ (Arg.Floats)
    ಹಿಂತಿರುಗಿ hash_combine(hash_ಮೌಲ್ಯ(Arg.Floats[0]), hash_ಮೌಲ್ಯ(Arg.Floats[1]));
  ಹಿಂತಿರುಗಿ hash_combine(Arg.Semantics);
}

APInt DoubleAPFloat::bitcast_ಗಮ್ಯ_APInt() ಸ್ಥಿರಾಂಕ {
  ಪ್ರತಿಜ್ಞಾ(Semantics == &APFloat_ತಳ::semPPCDoubleDouble &&
         "Unexpected Semantics");
  uint64_t ದತ್ತಾಂಶ_2[] = {
      Floats[0].bitcast_ಗಮ್ಯ_APInt().get_Raw_ದತ್ತಾಂಶ()[0],
      Floats[1].bitcast_ಗಮ್ಯ_APInt().get_Raw_ದತ್ತಾಂಶ()[0],
  };
  ಹಿಂತಿರುಗಿ APInt(128, ದತ್ತಾಂಶ_2);
}

Expected<APFloat::op_ಸ್ಥಿತಿ> DoubleAPFloat::convertFromString(StringRef S,
                                                             rounding_ಬಹುಲಕ RM) {
  ಪ್ರತಿಜ್ಞಾ(Semantics == &APFloat_ತಳ::semPPCDoubleDouble &&
         "Unexpected Semantics");
  APFloat Tmp(APFloat_ತಳ::semPPCDoubleDoubleLegacy);
  ಸ್ವಯಂಚಾಲಿತ Ret = Tmp.convertFromString(S, RM);
  *ಇದು = DoubleAPFloat(APFloat_ತಳ::semPPCDoubleDouble, Tmp.bitcast_ಗಮ್ಯ_APInt());
  ಹಿಂತಿರುಗಿ Ret;
}

// The double-double lattice of values corresponds to numbers which obey:
// - abs(lo) <= 1/2 * ulp(hi)
// - roundTiesToEven(hi + lo) == hi
//
// nextUp must choose the smallest output > input that follows these rules.
// nexDown must choose the largest output < input that follows these rules.
APFloat::op_ಸ್ಥಿತಿ DoubleAPFloat::ಮುಂದಿನ(ತಾರ್ಕಿಕ ಮುಂದಿನ_ಕೆಳಕ್ಕೆ) {
  ಪ್ರತಿಜ್ಞಾ(Semantics == &APFloat_ತಳ::semPPCDoubleDouble &&
         "Unexpected Semantics");
  // nextDown(x) = -nextUp(-x)
  ಆದರೆ (ಮುಂದಿನ_ಕೆಳಕ್ಕೆ) {
    changeSign();
    APFloat::op_ಸ್ಥಿತಿ ಫಲಿತಾಂಶ = ಮುಂದಿನ(/*nextDown=*/ಸುಳ್ಳು);
    changeSign();
    ಹಿಂತಿರುಗಿ ಫಲಿತಾಂಶ;
  }
  ಆಯ್ಕೆ (getCategory()) {
  ಸ್ಥಿತಿ fcInfinity:
    // nextUp(+inf) = +inf
    // nextUp(-inf) = -getLargest()
    ಆದರೆ (isNegative())
      make_ಅತಿದೊಡ್ಡ(ನಿಜ);
    ಹಿಂತಿರುಗಿ opOK;

  ಸ್ಥಿತಿ fcNaN:
    // IEEE-754R 2008 6.2 Par 2: nextUp(sNaN) = qNaN. Set Invalid flag.
    // IEEE-754R 2008 6.2: nextUp(qNaN) = qNaN. Must be identity so we do not
    //                     change the payload.
    ಆದರೆ (get_ಮೊದಲ().isSignaling()) {
      // For consistency, propagate the sign of the sNaN to the qNaN.
      makeNaN(ಸುಳ್ಳು, isNegative(), ಶೂನ್ಯಸೂಚಕ);
      ಹಿಂತಿರುಗಿ opInvalidOp;
    }
    ಹಿಂತಿರುಗಿ opOK;

  ಸ್ಥಿತಿ fcZero:
    // nextUp(pm 0) = +getSmallest()
    make_ಅತಿಚಿಕ್ಕ(ಸುಳ್ಳು);
    ಹಿಂತಿರುಗಿ opOK;

  ಸ್ಥಿತಿ fc_ಸಾಮಾನ್ಯ:
    ನಿಲ್ಲಿಸು;
  }

  ಸ್ಥಿರಾಂಕ APFloat &HiOld = get_ಮೊದಲ();
  ಸ್ಥಿರಾಂಕ APFloat &LoOld = get_ಎರಡನೇ();

  APFloat ಮುಂದಿನ_Lo = LoOld;
  ಮುಂದಿನ_Lo.ಮುಂದಿನ(/*nextDown=*/ಸುಳ್ಳು);

  // We want to admit values where:
  // 1. abs(Lo) <= ulp(Hi)/2
  // 2. Hi == RTNE(Hi + lo)
  ಸ್ವಯಂಚಾಲಿತ ಒಳ_Lattice = [](ಸ್ಥಿರಾಂಕ APFloat &Hi, ಸ್ಥಿರಾಂಕ APFloat &Lo) {
    ಹಿಂತಿರುಗಿ Hi + Lo == Hi;
  };

  // Check if (HiOld, nextUp(LoOld) is in the lattice.
  ಆದರೆ (ಒಳ_Lattice(HiOld, ಮುಂದಿನ_Lo)) {
    // Yes, the result is (HiOld, nextUp(LoOld)).
    Floats[1] = std::move(ಮುಂದಿನ_Lo);

    // TODO: Because we currently rely on semPPCDoubleDoubleLegacy, our maximum
    // value is defined to have exactly 106 bits of precision. This limitation
    // results in semPPCDoubleDouble being unable to reach its maximum canonical
    // value.
    DoubleAPFloat ಅತಿದೊಡ್ಡ{*Semantics, uninitialized};
    ಅತಿದೊಡ್ಡ.make_ಅತಿದೊಡ್ಡ(/*Neg=*/ಸುಳ್ಳು);
    ಆದರೆ (compare(ಅತಿದೊಡ್ಡ) == cmpGreaterThan)
      makeInf(/*Neg=*/ಸುಳ್ಳು);

    ಹಿಂತಿರುಗಿ opOK;
  }

  // Now we need to handle the cases where (HiOld, nextUp(LoOld)) is not the
  // correct result. We know the new hi component will be nextUp(HiOld) but our
  // lattice rules make it a little ambiguous what the correct NextLo must be.
  APFloat ಮುಂದಿನ_Hi = HiOld;
  ಮುಂದಿನ_Hi.ಮುಂದಿನ(/*nextDown=*/ಸುಳ್ಳು);

  // nextUp(getLargest()) == INFINITY
  ಆದರೆ (ಮುಂದಿನ_Hi.isInfinity()) {
    makeInf(/*Neg=*/ಸುಳ್ಳು);
    ಹಿಂತಿರುಗಿ opOK;
  }

  // IEEE 754-2019 5.3.1:
  // "If x is the negative number of least magnitude in x's format, nextUp(x) is
  // -0."
  ಆದರೆ (ಮುಂದಿನ_Hi.isZero()) {
    makeZero(/*Neg=*/ನಿಜ);
    ಹಿಂತಿರುಗಿ opOK;
  }

  // abs(NextLo) must be <= ulp(NextHi)/2. We want NextLo to be as close to
  // negative infinity as possible.
  ಮುಂದಿನ_Lo = neg(scalbn(harrisonUlp(ಮುಂದಿನ_Hi), -1, rmTowardZero));
  ಆದರೆ (!ಒಳ_Lattice(ಮುಂದಿನ_Hi, ಮುಂದಿನ_Lo))
    // RTNE may mean that Lo must be < ulp(NextHi) / 2 so we bump NextLo.
    ಮುಂದಿನ_Lo.ಮುಂದಿನ(/*nextDown=*/ಸುಳ್ಳು);

  Floats[0] = std::move(ಮುಂದಿನ_Hi);
  Floats[1] = std::move(ಮುಂದಿನ_Lo);

  ಹಿಂತಿರುಗಿ opOK;
}

APFloat::op_ಸ್ಥಿತಿ DoubleAPFloat::convert_ಗಮ್ಯ_Sign_Extended_Integer(
    MutableArrayRef<integerPart> ಒಳಹರಿವು, ಚಿಹ್ನೆರಹಿತ ಪೂರ್ಣಾಂಕ ಅಗಲ, ತಾರ್ಕಿಕ IsSigned,
    rounding_ಬಹುಲಕ RM, ತಾರ್ಕಿಕ *IsExact) ಸ್ಥಿರಾಂಕ {
  ಪ್ರತಿಜ್ಞಾ(Semantics == &APFloat_ತಳ::semPPCDoubleDouble &&
         "Unexpected Semantics");

  // If Hi is not finite, or Lo is zero, the value is entirely represented
  // by Hi. Delegate to the simpler single-APFloat conversion.
  ಆದರೆ (!get_ಮೊದಲ().isFiniteNonZero() || get_ಎರಡನೇ().isZero())
    ಹಿಂತಿರುಗಿ get_ಮೊದಲ().convert_ಗಮ್ಯ_Integer(ಒಳಹರಿವು, ಅಗಲ, IsSigned, RM, IsExact);

  // First, round the full double-double value to an integral value. This
  // simplifies the rest of the function, as we no longer need to consider
  // fractional parts.
  *IsExact = ಸುಳ್ಳು;
  DoubleAPFloat Integral = *ಇದು;
  ಸ್ಥಿರಾಂಕ op_ಸ್ಥಿತಿ ಸುತ್ತು_ಸ್ಥಿತಿ = Integral.ಸುತ್ತು_ಗಮ್ಯ_Integral(RM);
  ಆದರೆ (ಸುತ್ತು_ಸ್ಥಿತಿ == opInvalidOp)
    ಹಿಂತಿರುಗಿ opInvalidOp;
  ಸ್ಥಿರಾಂಕ APFloat &IntegralHi = Integral.get_ಮೊದಲ();
  ಸ್ಥಿರಾಂಕ APFloat &IntegralLo = Integral.get_ಎರಡನೇ();

  // If rounding results in either component being zero, the sum is trivial.
  // Delegate to the simpler single-APFloat conversion.
  ತಾರ್ಕಿಕ HiIsExact;
  ಆದರೆ (IntegralHi.isZero() || IntegralLo.isZero()) {
    ಸ್ಥಿರಾಂಕ op_ಸ್ಥಿತಿ Hi_ಸ್ಥಿತಿ =
        IntegralHi.convert_ಗಮ್ಯ_Integer(ಒಳಹರಿವು, ಅಗಲ, IsSigned, RM, &HiIsExact);
    // The conversion from an integer-valued float to an APInt may fail if the
    // result would be out of range.  Regardless, taking this path is only
    // possible if rounding occurred during the initial `roundToIntegral`.
    ಹಿಂತಿರುಗಿ Hi_ಸ್ಥಿತಿ == opOK ? opInexact : Hi_ಸ್ಥಿತಿ;
  }

  // A negative number cannot be represented by an unsigned integer.
  // Since a double-double is canonical, if Hi is negative, the sum is negative.
  ಆದರೆ (!IsSigned && IntegralHi.isNegative())
    ಹಿಂತಿರುಗಿ opInvalidOp;

  // Handle the special boundary case where |Hi| is exactly the power of two
  // that marks the edge of the integer's range (e.g., 2^63 for int64_t). In
  // this situation, Hi itself won't fit, but the sum Hi + Lo might.
  // `PositiveOverflowWidth` is the bit number for this boundary (N-1 for
  // signed, N for unsigned).
  ತಾರ್ಕಿಕ LoIsExact;
  ಸ್ಥಿರಾಂಕ ಪೂರ್ಣಾಂಕ HiExactLog2 = IntegralHi.getExactLog2Abs();
  ಸ್ಥಿರಾಂಕ ಚಿಹ್ನೆರಹಿತ ಧನಾತ್ಮಕ_ಅತಿಪ್ರವಾಹ_ಅಗಲ = IsSigned ? ಅಗಲ - 1 : ಅಗಲ;
  ಆದರೆ (HiExactLog2 >= 0 &&
      static_cast<ಚಿಹ್ನೆರಹಿತ>(HiExactLog2) == ಧನಾತ್ಮಕ_ಅತಿಪ್ರವಾಹ_ಅಗಲ) {
    // If Hi and Lo have the same sign, |Hi + Lo| > |Hi|, so the sum is
    // guaranteed to overflow. E.g., for uint128_t, (2^128, 1) overflows.
    ಆದರೆ (IntegralHi.isNegative() == IntegralLo.isNegative())
      ಹಿಂತಿರುಗಿ opInvalidOp;

    // If the signs differ, the sum will fit. We can compute the result using
    // properties of two's complement arithmetic without a wide intermediate
    // integer. E.g., for uint128_t, (2^128, -1) should be 2^128 - 1.
    ಸ್ಥಿರಾಂಕ op_ಸ್ಥಿತಿ Lo_ಸ್ಥಿತಿ = IntegralLo.convert_ಗಮ್ಯ_Integer(
        ಒಳಹರಿವು, ಅಗಲ, /*IsSigned=*/ನಿಜ, RM, &LoIsExact);
    ಆದರೆ (Lo_ಸ್ಥಿತಿ == opInvalidOp)
      ಹಿಂತಿರುಗಿ opInvalidOp;

    // Adjust the bit pattern of Lo to account for Hi's value:
    //  - For unsigned (Hi=2^Width): `2^Width + Lo` in `Width`-bit
    //    arithmetic is equivalent to just `Lo`. The conversion of `Lo` above
    //    already produced the correct final bit pattern.
    //  - For signed (Hi=2^(Width-1)): The sum `2^(Width-1) + Lo` (where Lo<0)
    //    can be computed by taking the two's complement pattern for `Lo` and
    //    clearing the sign bit.
    ಆದರೆ (IsSigned && !IntegralHi.isNegative())
      APInt::tc_Clear_ಬಿಟ್(ಒಳಹರಿವು.ದತ್ತಾಂಶ(), ಧನಾತ್ಮಕ_ಅತಿಪ್ರವಾಹ_ಅಗಲ);
    *IsExact = ಸುತ್ತು_ಸ್ಥಿತಿ == opOK;
    ಹಿಂತಿರುಗಿ ಸುತ್ತು_ಸ್ಥಿತಿ;
  }

  // Convert Hi into an integer.  This may not fit but that is OK: we know that
  // Hi + Lo would not fit either in this situation.
  ಸ್ಥಿರಾಂಕ op_ಸ್ಥಿತಿ Hi_ಸ್ಥಿತಿ = IntegralHi.convert_ಗಮ್ಯ_Integer(
      ಒಳಹರಿವು, ಅಗಲ, IsSigned, rmTowardZero, &HiIsExact);
  ಆದರೆ (Hi_ಸ್ಥಿತಿ == opInvalidOp)
    ಹಿಂತಿರುಗಿ Hi_ಸ್ಥಿತಿ;

  // Convert Lo into a temporary integer of the same width.
  APSInt Lo_ಫಲಿತಾಂಶ{ಅಗಲ, /*isUnsigned=*/!IsSigned};
  ಸ್ಥಿರಾಂಕ op_ಸ್ಥಿತಿ Lo_ಸ್ಥಿತಿ =
      IntegralLo.convert_ಗಮ್ಯ_Integer(Lo_ಫಲಿತಾಂಶ, rmTowardZero, &LoIsExact);
  ಆದರೆ (Lo_ಸ್ಥಿತಿ == opInvalidOp)
    ಹಿಂತಿರುಗಿ Lo_ಸ್ಥಿತಿ;

  // Add Lo to Hi. This addition is guaranteed not to overflow because of the
  // double-double canonicalization rule (`|Lo| <= ulp(Hi)/2`). The only case
  // where the sum could cross the integer type's boundary is when Hi is a
  // power of two, which is handled by the special case block above.
  APInt::tc_ಕೂಡಿಸು(ಒಳಹರಿವು.ದತ್ತಾಂಶ(), Lo_ಫಲಿತಾಂಶ.get_Raw_ದತ್ತಾಂಶ(), /*carry=*/0, ಒಳಹರಿವು.ಗಾತ್ರ_3());

  *IsExact = ಸುತ್ತು_ಸ್ಥಿತಿ == opOK;
  ಹಿಂತಿರುಗಿ ಸುತ್ತು_ಸ್ಥಿತಿ;
}

APFloat::op_ಸ್ಥಿತಿ
DoubleAPFloat::convert_ಗಮ್ಯ_Integer(MutableArrayRef<integerPart> ಒಳಹರಿವು,
                                ಚಿಹ್ನೆರಹಿತ ಪೂರ್ಣಾಂಕ ಅಗಲ, ತಾರ್ಕಿಕ IsSigned,
                                rounding_ಬಹುಲಕ RM, ತಾರ್ಕಿಕ *IsExact) ಸ್ಥಿರಾಂಕ {
  op_ಸ್ಥಿತಿ FS =
      convert_ಗಮ್ಯ_Sign_Extended_Integer(ಒಳಹರಿವು, ಅಗಲ, IsSigned, RM, IsExact);

  ಆದರೆ (FS == opInvalidOp) {
    ಸ್ಥಿರಾಂಕ ಚಿಹ್ನೆರಹಿತ Dst_Parts_ಎಣಿಕೆ = part_ಎಣಿಕೆ_For_ಬಿಟ್ಟುಗಳು(ಅಗಲ);
    ಪ್ರತಿಜ್ಞಾ(Dst_Parts_ಎಣಿಕೆ <= ಒಳಹರಿವು.ಗಾತ್ರ_3() && "Integer too big");

    ಚಿಹ್ನೆರಹಿತ ಬಿಟ್ಟುಗಳು_2;
    ಆದರೆ (getCategory() == fcNaN)
      ಬಿಟ್ಟುಗಳು_2 = 0;
    ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (isNegative())
      ಬಿಟ್ಟುಗಳು_2 = IsSigned;
    ಇಲ್ಲದಿದ್ದರೆ
      ಬಿಟ್ಟುಗಳು_2 = ಅಗಲ - IsSigned;

    tc_ಗಣ_ಕನಿಷ್ಠ_Significant_ಬಿಟ್ಟುಗಳು(ಒಳಹರಿವು.ದತ್ತಾಂಶ(), Dst_Parts_ಎಣಿಕೆ, ಬಿಟ್ಟುಗಳು_2);
    ಆದರೆ (isNegative() && IsSigned)
      APInt::tc_ಸರಿಸು_ಎಡ(ಒಳಹರಿವು.ದತ್ತಾಂಶ(), Dst_Parts_ಎಣಿಕೆ, ಅಗಲ - 1);
  }

  ಹಿಂತಿರುಗಿ FS;
}

APFloat::op_ಸ್ಥಿತಿ DoubleAPFloat::handle_ಅತಿಪ್ರವಾಹ(rounding_ಬಹುಲಕ RM) {
  ಆಯ್ಕೆ (RM) {
  ಸ್ಥಿತಿ APFloat::rmTowardZero:
    make_ಅತಿದೊಡ್ಡ(/*Neg=*/isNegative());
    ನಿಲ್ಲಿಸು;
  ಸ್ಥಿತಿ APFloat::rmTowardNegative:
    ಆದರೆ (isNegative())
      makeInf(/*Neg=*/ನಿಜ);
    ಇಲ್ಲದಿದ್ದರೆ
      make_ಅತಿದೊಡ್ಡ(/*Neg=*/ಸುಳ್ಳು);
    ನಿಲ್ಲಿಸು;
  ಸ್ಥಿತಿ APFloat::rm_Toward_ಧನಾತ್ಮಕ:
    ಆದರೆ (isNegative())
      make_ಅತಿದೊಡ್ಡ(/*Neg=*/ನಿಜ);
    ಇಲ್ಲದಿದ್ದರೆ
      makeInf(/*Neg=*/ಸುಳ್ಳು);
    ನಿಲ್ಲಿಸು;
  ಸ್ಥಿತಿ APFloat::rm_ಹತ್ತಿರದ_Ties_ಗಮ್ಯ_Away:
  ಸ್ಥಿತಿ APFloat::rm_ಹತ್ತಿರದ_Ties_ಗಮ್ಯ_Even:
    makeInf(/*Neg=*/isNegative());
    ನಿಲ್ಲಿಸು;
  ಪೂರ್ವನಿಯೋಜಿತ:
    llvm_unreachable("Invalid rounding mode found");
  }
  op_ಸ್ಥಿತಿ S = opInexact;
  ಆದರೆ (!get_ಮೊದಲ().isFinite())
    S = static_cast<op_ಸ್ಥಿತಿ>(S | op_ಅತಿಪ್ರವಾಹ);
  ಹಿಂತಿರುಗಿ S;
}

APFloat::op_ಸ್ಥಿತಿ DoubleAPFloat::convertFromUnsignedParts(
    ಸ್ಥಿರಾಂಕ integerPart *Src, ಚಿಹ್ನೆರಹಿತ ಪೂರ್ಣಾಂಕ Src_ಎಣಿಕೆ, rounding_ಬಹುಲಕ RM) {
  // Find the most significant bit of the source integer. APInt::tcMSB returns
  // UINT_MAX for a zero value.
  ಸ್ಥಿರಾಂಕ ಚಿಹ್ನೆರಹಿತ SrcMSB = APInt::tcMSB(Src, Src_ಎಣಿಕೆ);
  ಆದರೆ (SrcMSB == UINT_MAX) {
    // The source integer is 0.
    makeZero(/*Neg=*/ಸುಳ್ಳು);
    ಹಿಂತಿರುಗಿ opOK;
  }

  // Create a minimally-sized APInt to represent the source value.
  ಸ್ಥಿರಾಂಕ ಚಿಹ್ನೆರಹಿತ Src_ಬಿಟ್_ಅಗಲ = SrcMSB + 1;
  APSInt SrcInt{APInt{/*numBits=*/Src_ಬಿಟ್_ಅಗಲ, ArrayRef(Src, Src_ಎಣಿಕೆ)},
                /*isUnsigned=*/ನಿಜ};

  // Stage 1: Initial Approximation.
  // Convert the source integer SrcInt to the Hi part of the DoubleAPFloat.
  // We use round-to-nearest because it minimizes the initial error, which is
  // crucial for the subsequent steps.
  APFloat Hi{get_ಮೊದಲ().getSemantics()};
  Hi.convertFromAPInt(SrcInt, /*IsSigned=*/ಸುಳ್ಳು, rm_ಹತ್ತಿರದ_Ties_ಗಮ್ಯ_Even);

  // If the first approximation already overflows, the number is too large.
  // NOTE: The underlying semantics are *more* conservative when choosing to
  // overflow because their notion of ULP is much larger. As such, it is always
  // safe to overflow at the DoubleAPFloat level if the APFloat overflows.
  ಆದರೆ (!Hi.isFinite())
    ಹಿಂತಿರುಗಿ handle_ಅತಿಪ್ರವಾಹ(RM);

  // Stage 2: Exact Error Calculation.
  // Calculate the exact error of the first approximation: Error = SrcInt - Hi.
  // This is done by converting Hi back to an integer and subtracting it from
  // the original source.
  ತಾರ್ಕಿಕ HiAsIntIsExact;
  // Create an integer representation of Hi. Its width is determined by the
  // exponent of Hi, ensuring it's just large enough. This width can exceed
  // SrcBitWidth if the conversion to Hi rounded up to a power of two.
  // accurately when converted back to an integer.
  APSInt HiAsInt{static_cast<uint32_t>(ilogb(Hi) + 1), /*isUnsigned=*/ನಿಜ};
  Hi.convert_ಗಮ್ಯ_Integer(HiAsInt, rm_ಹತ್ತಿರದ_Ties_ಗಮ್ಯ_Even, &HiAsIntIsExact);
  ಸ್ಥಿರಾಂಕ APInt Error = SrcInt.zext(HiAsInt.get_ಬಿಟ್_ಅಗಲ()) - HiAsInt;

  // Stage 3: Error Approximation and Rounding.
  // Convert the integer error into the Lo part of the DoubleAPFloat. This step
  // captures the remainder of the original number. The rounding mode for this
  // conversion (LoRM) may need to be adjusted from the user-requested RM to
  // ensure the final sum (Hi + Lo) rounds correctly.
  rounding_ಬಹುಲಕ LoRM = RM;
  // Adjustments are only necessary when the initial approximation Hi was an
  // overestimate, making the Error negative.
  ಆದರೆ (Error.isNegative()) {
    ಆದರೆ (RM == rm_ಹತ್ತಿರದ_Ties_ಗಮ್ಯ_Away) {
      // For rmNearestTiesToAway, a tie should round away from zero. Since
      // SrcInt is positive, this means rounding toward +infinity.
      // A standard conversion of a negative Error would round ties toward
      // -infinity, causing the final sum Hi + Lo to be smaller. To
      // counteract this, we detect the tie case and override the rounding
      // mode for Lo to rmTowardPositive.
      ಸ್ಥಿರಾಂಕ ಚಿಹ್ನೆರಹಿತ Error_ಸಕ್ರಿಯ_ಬಿಟ್ಟುಗಳು = Error.get_Significant_ಬಿಟ್ಟುಗಳು() - 1;
      ಸ್ಥಿರಾಂಕ ಚಿಹ್ನೆರಹಿತ LoPrecision = get_ಎರಡನೇ().getSemantics().precision;
      ಆದರೆ (Error_ಸಕ್ರಿಯ_ಬಿಟ್ಟುಗಳು > LoPrecision) {
        ಸ್ಥಿರಾಂಕ ಚಿಹ್ನೆರಹಿತ RoundingBoundary = Error_ಸಕ್ರಿಯ_ಬಿಟ್ಟುಗಳು - LoPrecision;
        // A tie occurs when the bits to be truncated are of the form 100...0.
        // This is detected by checking if the number of trailing zeros is
        // exactly one less than the number of bits being truncated.
        ಆದರೆ (Error.ಎಣಿಕೆ_Trailing_Zeros() == RoundingBoundary - 1)
          LoRM = rm_Toward_ಧನಾತ್ಮಕ;
      }
    } ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (RM == rmTowardZero) {
      // For rmTowardZero, the final positive result must be truncated (rounded
      // down). When Hi is an overestimate, Error is negative. A standard
      // rmTowardZero conversion of Error would make it *less* negative,
      // effectively rounding the final sum Hi + Lo *up*. To ensure the sum
      // rounds down correctly, we force Lo to round toward -infinity.
      LoRM = rmTowardNegative;
    }
  }

  APFloat Lo{get_ಎರಡನೇ().getSemantics()};
  op_ಸ್ಥಿತಿ ಸ್ಥಿತಿ_2 = Lo.convertFromAPInt(Error, /*IsSigned=*/ನಿಜ, LoRM);

  // Renormalize the pair (Hi, Lo) into a canonical DoubleAPFloat form where the
  // components do not overlap. fastTwoSum performs this operation.
  std::tie(Hi, Lo) = fast_Two_ಮೊತ್ತ(Hi, Lo);
  Floats[0] = std::move(Hi);
  Floats[1] = std::move(Lo);

  // A final check for overflow is needed because fastTwoSum can cause a
  // carry-out from Lo that pushes Hi to infinity.
  ಆದರೆ (!get_ಮೊದಲ().isFinite())
    ಹಿಂತಿರುಗಿ handle_ಅತಿಪ್ರವಾಹ(RM);

  // The largest DoubleAPFloat must be canonical. Values which are larger are
  // not canonical and are equivalent to overflow.
  ಆದರೆ (get_ಮೊದಲ().isFiniteNonZero() && Floats[0].is_ಅತಿದೊಡ್ಡ()) {
    DoubleAPFloat ಅತಿದೊಡ್ಡ{*Semantics};
    ಅತಿದೊಡ್ಡ.make_ಅತಿದೊಡ್ಡ(/*Neg=*/ಸುಳ್ಳು);
    ಆದರೆ (compare(ಅತಿದೊಡ್ಡ) == APFloat::cmpGreaterThan)
      ಹಿಂತಿರುಗಿ handle_ಅತಿಪ್ರವಾಹ(RM);
  }

  // The final status of the operation is determined by the conversion of the
  // error term. If Lo could represent Error exactly, the entire conversion
  // is exact. Otherwise, it's inexact.
  ಹಿಂತಿರುಗಿ ಸ್ಥಿತಿ_2;
}

APFloat::op_ಸ್ಥಿತಿ DoubleAPFloat::convertFromAPInt(ಸ್ಥಿರಾಂಕ APInt &ಒಳಹರಿವು,
                                                  ತಾರ್ಕಿಕ IsSigned,
                                                  rounding_ಬಹುಲಕ RM) {
  ಸ್ಥಿರಾಂಕ ತಾರ್ಕಿಕ Negate_ಒಳಹರಿವು = IsSigned && ಒಳಹರಿವು.isNegative();
  APInt API = ಒಳಹರಿವು;
  ಆದರೆ (Negate_ಒಳಹರಿವು)
    API.negate();

  ಸ್ಥಿರಾಂಕ APFloat::op_ಸ್ಥಿತಿ ಸ್ಥಿತಿ_2 =
      convertFromUnsignedParts(API.get_Raw_ದತ್ತಾಂಶ(), API.get_ಅಂಕೆ_ಪದಗಳು(), RM);
  ಆದರೆ (Negate_ಒಳಹರಿವು)
    changeSign();
  ಹಿಂತಿರುಗಿ ಸ್ಥಿತಿ_2;
}

ಚಿಹ್ನೆರಹಿತ ಪೂರ್ಣಾಂಕ DoubleAPFloat::convert_ಗಮ್ಯ_Hex_String(ಅಕ್ಷರ *DST,
                                               ಚಿಹ್ನೆರಹಿತ ಪೂರ್ಣಾಂಕ HexDigits,
                                               ತಾರ್ಕಿಕ ಊರ್ಧ್ವ_Case_2,
                                               rounding_ಬಹುಲಕ RM) ಸ್ಥಿರಾಂಕ {
  ಪ್ರತಿಜ್ಞಾ(Semantics == &APFloat_ತಳ::semPPCDoubleDouble &&
         "Unexpected Semantics");
  ಹಿಂತಿರುಗಿ APFloat(APFloat_ತಳ::semPPCDoubleDoubleLegacy, bitcast_ಗಮ್ಯ_APInt())
      .convert_ಗಮ್ಯ_Hex_String(DST, HexDigits, ಊರ್ಧ್ವ_Case_2, RM);
}

ತಾರ್ಕಿಕ DoubleAPFloat::isDenormal() ಸ್ಥಿರಾಂಕ {
  ಹಿಂತಿರುಗಿ getCategory() == fc_ಸಾಮಾನ್ಯ &&
         (Floats[0].isDenormal() || Floats[1].isDenormal() ||
          // (double)(Hi + Lo) == Hi defines a normal number.
          Floats[0] != Floats[0] + Floats[1]);
}

ತಾರ್ಕಿಕ DoubleAPFloat::is_ಅತಿಚಿಕ್ಕ() ಸ್ಥಿರಾಂಕ {
  ಆದರೆ (getCategory() != fc_ಸಾಮಾನ್ಯ)
    ಹಿಂತಿರುಗಿ ಸುಳ್ಳು;
  DoubleAPFloat Tmp(*ಇದು);
  Tmp.make_ಅತಿಚಿಕ್ಕ(ಇದು->isNegative());
  ಹಿಂತಿರುಗಿ Tmp.compare(*ಇದು) == cmpEqual;
}

ತಾರ್ಕಿಕ DoubleAPFloat::is_ಅತಿಚಿಕ್ಕ_Normalized() ಸ್ಥಿರಾಂಕ {
  ಆದರೆ (getCategory() != fc_ಸಾಮಾನ್ಯ)
    ಹಿಂತಿರುಗಿ ಸುಳ್ಳು;

  DoubleAPFloat Tmp(*ಇದು);
  Tmp.make_ಅತಿಚಿಕ್ಕ_Normalized(ಇದು->isNegative());
  ಹಿಂತಿರುಗಿ Tmp.compare(*ಇದು) == cmpEqual;
}

ತಾರ್ಕಿಕ DoubleAPFloat::is_ಅತಿದೊಡ್ಡ() ಸ್ಥಿರಾಂಕ {
  ಆದರೆ (getCategory() != fc_ಸಾಮಾನ್ಯ)
    ಹಿಂತಿರುಗಿ ಸುಳ್ಳು;
  DoubleAPFloat Tmp(*ಇದು);
  Tmp.make_ಅತಿದೊಡ್ಡ(ಇದು->isNegative());
  ಹಿಂತಿರುಗಿ Tmp.compare(*ಇದು) == cmpEqual;
}

ತಾರ್ಕಿಕ DoubleAPFloat::isInteger() ಸ್ಥಿರಾಂಕ {
  ಪ್ರತಿಜ್ಞಾ(Semantics == &APFloat_ತಳ::semPPCDoubleDouble &&
         "Unexpected Semantics");
  ಹಿಂತಿರುಗಿ Floats[0].isInteger() && Floats[1].isInteger();
}

ಶೂನ್ಯ DoubleAPFloat::ಗಮ್ಯ_String(Small_ಸದಿಶ_Impl<ಅಕ್ಷರ> &Str,
                             ಚಿಹ್ನೆರಹಿತ FormatPrecision,
                             ಚಿಹ್ನೆರಹಿತ FormatMaxPadding,
                             ತಾರ್ಕಿಕ TruncateZero) ಸ್ಥಿರಾಂಕ {
  ಪ್ರತಿಜ್ಞಾ(Semantics == &APFloat_ತಳ::semPPCDoubleDouble &&
         "Unexpected Semantics");
  APFloat(APFloat_ತಳ::semPPCDoubleDoubleLegacy, bitcast_ಗಮ್ಯ_APInt())
      .ಗಮ್ಯ_String(Str, FormatPrecision, FormatMaxPadding, TruncateZero);
}

ಪೂರ್ಣಾಂಕ DoubleAPFloat::getExactLog2Abs() ಸ್ಥಿರಾಂಕ {
  // In order for Hi + Lo to be a power of two, the following must be true:
  // 1. Hi must be a power of two.
  // 2. Lo must be zero.
  ಆದರೆ (get_ಎರಡನೇ().isNonZero())
    ಹಿಂತಿರುಗಿ INT_MIN;
  ಹಿಂತಿರುಗಿ get_ಮೊದಲ().getExactLog2Abs();
}

ಪೂರ್ಣಾಂಕ ilogb(ಸ್ಥಿರಾಂಕ DoubleAPFloat &Arg) {
  ಸ್ಥಿರಾಂಕ APFloat &Hi = Arg.get_ಮೊದಲ();
  ಸ್ಥಿರಾಂಕ APFloat &Lo = Arg.get_ಎರಡನೇ();
  ಪೂರ್ಣಾಂಕ Ilogb_ಫಲಿತಾಂಶ = ilogb(Hi);
  // Zero and non-finite values can delegate to ilogb(Hi).
  ಆದರೆ (Arg.getCategory() != fc_ಸಾಮಾನ್ಯ)
    ಹಿಂತಿರುಗಿ Ilogb_ಫಲಿತಾಂಶ;
  // If Lo can't change the binade, we can delegate to ilogb(Hi).
  ಆದರೆ (Lo.isZero() || Hi.isNegative() == Lo.isNegative())
    ಹಿಂತಿರುಗಿ Ilogb_ಫಲಿತಾಂಶ;
  ಆದರೆ (Hi.getExactLog2Abs() == INT_MIN)
    ಹಿಂತಿರುಗಿ Ilogb_ಫಲಿತಾಂಶ;
  // Numbers of the form 2^a - 2^b or -2^a + 2^b are almost powers of two but
  // get nudged out of the binade by the low component.
  ಹಿಂತಿರುಗಿ Ilogb_ಫಲಿತಾಂಶ - 1;
}

DoubleAPFloat scalbn(ಸ್ಥಿರಾಂಕ DoubleAPFloat &Arg, ಪೂರ್ಣಾಂಕ Exp,
                     APFloat::rounding_ಬಹುಲಕ RM) {
  ಪ್ರತಿಜ್ಞಾ(Arg.Semantics == &APFloat_ತಳ::PPCDoubleDouble() &&
         "Unexpected Semantics");
  ಹಿಂತಿರುಗಿ DoubleAPFloat(APFloat_ತಳ::PPCDoubleDouble(),
                       scalbn(Arg.Floats[0], Exp, RM),
                       scalbn(Arg.Floats[1], Exp, RM));
}

DoubleAPFloat frexp(ಸ್ಥಿರಾಂಕ DoubleAPFloat &Arg, ಪೂರ್ಣಾಂಕ &Exp,
                    APFloat::rounding_ಬಹುಲಕ RM) {
  ಪ್ರತಿಜ್ಞಾ(Arg.Semantics == &APFloat_ತಳ::PPCDoubleDouble() &&
         "Unexpected Semantics");

  // Get the unbiased exponent e of the number, where |Arg| = m * 2^e for m in
  // [1.0, 2.0).
  Exp = ilogb(Arg);

  // For NaNs, quiet any signaling NaN and return the result, as per standard
  // practice.
  ಆದರೆ (Exp == APFloat::IEK_NaN) {
    DoubleAPFloat Quiet{Arg};
    Quiet.get_ಮೊದಲ() = Quiet.get_ಮೊದಲ().makeQuiet();
    ಹಿಂತಿರುಗಿ Quiet;
  }

  // For infinity, return it unchanged. The exponent remains IEK_Inf.
  ಆದರೆ (Exp == APFloat::IEK_Inf)
    ಹಿಂತಿರುಗಿ Arg;

  // For zero, the fraction is zero and the standard requires the exponent be 0.
  ಆದರೆ (Exp == APFloat::IEK_Zero) {
    Exp = 0;
    ಹಿಂತಿರುಗಿ Arg;
  }

  ಸ್ಥಿರಾಂಕ APFloat &Hi = Arg.get_ಮೊದಲ();
  ಸ್ಥಿರಾಂಕ APFloat &Lo = Arg.get_ಎರಡನೇ();

  // frexp requires the fraction's absolute value to be in [0.5, 1.0).
  // ilogb provides an exponent for an absolute value in [1.0, 2.0).
  // Increment the exponent to ensure the fraction is in the correct range.
  ++Exp;

  ಸ್ಥಿರಾಂಕ ತಾರ್ಕಿಕ SignsDisagree = Hi.isNegative() != Lo.isNegative();
  APFloat ಎರಡನೇ = Lo;
  ಆದರೆ (Arg.getCategory() == APFloat::fc_ಸಾಮಾನ್ಯ && Lo.isFiniteNonZero()) {
    rounding_ಬಹುಲಕ Lo_Rounding_ಬಹುಲಕ;
    // The interpretation of rmTowardZero depends on the sign of the combined
    // Arg rather than the sign of the component.
    ಆದರೆ (RM == rmTowardZero)
      Lo_Rounding_ಬಹುಲಕ = Arg.isNegative() ? rm_Toward_ಧನಾತ್ಮಕ : rmTowardNegative;
    // For rmNearestTiesToAway, we face a similar problem. If signs disagree,
    // Lo is a correction *toward* zero relative to Hi. Rounding Lo
    // "away from zero" based on its own sign would move the value in the
    // wrong direction. As a safe proxy, we use rmNearestTiesToEven, which is
    // direction-agnostic. We only need to bother with this if Lo is scaled
    // down.
    ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (RM == rm_ಹತ್ತಿರದ_Ties_ಗಮ್ಯ_Away && SignsDisagree && Exp > 0)
      Lo_Rounding_ಬಹುಲಕ = rm_ಹತ್ತಿರದ_Ties_ಗಮ್ಯ_Even;
    ಇಲ್ಲದಿದ್ದರೆ
      Lo_Rounding_ಬಹುಲಕ = RM;
    ಎರಡನೇ = scalbn(Lo, -Exp, Lo_Rounding_ಬಹುಲಕ);
    // The rmNearestTiesToEven proxy is correct most of the time, but it
    // differs from rmNearestTiesToAway when the scaled value of Lo is an
    // exact midpoint.
    // NOTE: This is morally equivalent to roundTiesTowardZero.
    ಆದರೆ (RM == rm_ಹತ್ತಿರದ_Ties_ಗಮ್ಯ_Away && Lo_Rounding_ಬಹುಲಕ == rm_ಹತ್ತಿರದ_Ties_ಗಮ್ಯ_Even) {
      // Re-scale the result back to check if rounding occurred.
      ಸ್ಥಿರಾಂಕ APFloat RecomposedLo = scalbn(ಎರಡನೇ, Exp, rm_ಹತ್ತಿರದ_Ties_ಗಮ್ಯ_Even);
      ಆದರೆ (RecomposedLo != Lo) {
        // RoundingError tells us which direction we rounded:
        //   - RoundingError > 0: we rounded up.
        //   - RoundingError < 0: we down up.
        ಸ್ಥಿರಾಂಕ APFloat RoundingError = RecomposedLo - Lo;
        // Determine if scalbn(Lo, -Exp) landed exactly on a midpoint.
        // We do this by checking if the absolute rounding error is exactly
        // half a ULP of the result.
        ಸ್ಥಿರಾಂಕ APFloat Ulp_ದ_ಎರಡನೇ = harrisonUlp(ಎರಡನೇ);
        ಸ್ಥಿರಾಂಕ APFloat Scaled_Ulp_ದ_ಎರಡನೇ =
            scalbn(Ulp_ದ_ಎರಡನೇ, Exp - 1, rm_ಹತ್ತಿರದ_Ties_ಗಮ್ಯ_Even);
        ಸ್ಥಿರಾಂಕ ತಾರ್ಕಿಕ IsMidpoint = ನಿರಪೇಕ್ಷ(RoundingError) == Scaled_Ulp_ದ_ಎರಡನೇ;
        ಸ್ಥಿರಾಂಕ ತಾರ್ಕಿಕ RoundedLoAway =
            ಎರಡನೇ.isNegative() == RoundingError.isNegative();
        // The sign of Hi and Lo disagree and we rounded Lo away: we must
        // decrease the magnitude of Second to increase the magnitude
        // First+Second.
        ಆದರೆ (IsMidpoint && RoundedLoAway)
          ಎರಡನೇ.ಮುಂದಿನ(/*nextDown=*/!ಎರಡನೇ.isNegative());
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
    ಆದರೆ (ಎರಡನೇ.isZero() && SignsDisagree && Hi.getExactLog2Abs() != INT_MIN)
      ++Exp;
  }

  APFloat ಮೊದಲ = scalbn(Hi, -Exp, RM);
  ಹಿಂತಿರುಗಿ DoubleAPFloat(APFloat_ತಳ::PPCDoubleDouble(), std::move(ಮೊದಲ),
                       std::move(ಎರಡನೇ));
}

APInt DoubleAPFloat::getNaNPayload() ಸ್ಥಿರಾಂಕ { ಹಿಂತಿರುಗಿ Floats[0].getNaNPayload(); }
} // namespace detail

APFloat::Storage::Storage(IEEEFloat F, ಸ್ಥಿರಾಂಕ fltSemantics &Semantics) {
  ಆದರೆ (ಬಳಸುತ್ತದೆ_Layout<IEEEFloat>(Semantics)) {
    ಹೊಸ (&IEEE) IEEEFloat(std::move(F));
    ಹಿಂತಿರುಗಿ;
  }
  ಆದರೆ (ಬಳಸುತ್ತದೆ_Layout<DoubleAPFloat>(Semantics)) {
    ಸ್ಥಿರಾಂಕ fltSemantics& S = F.getSemantics();
    ಹೊಸ (&Double) DoubleAPFloat(Semantics, APFloat(std::move(F), S),
                                APFloat(APFloat_ತಳ::IEEEdouble()));
    ಹಿಂತಿರುಗಿ;
  }
  llvm_unreachable("Unexpected semantics");
}

Expected<APFloat::op_ಸ್ಥಿತಿ> APFloat::convertFromString(StringRef Str,
                                                       rounding_ಬಹುಲಕ RM) {
  APFLOAT_DISPATCH_ON_SEMANTICS(convertFromString(Str, RM));
}

hash_ಸಂಕೇತ hash_ಮೌಲ್ಯ(ಸ್ಥಿರಾಂಕ APFloat &Arg) {
  ಆದರೆ (APFloat::ಬಳಸುತ್ತದೆ_Layout<detail::IEEEFloat>(Arg.getSemantics()))
    ಹಿಂತಿರುಗಿ hash_ಮೌಲ್ಯ(Arg.U.IEEE);
  ಆದರೆ (APFloat::ಬಳಸುತ್ತದೆ_Layout<detail::DoubleAPFloat>(Arg.getSemantics()))
    ಹಿಂತಿರುಗಿ hash_ಮೌಲ್ಯ(Arg.U.Double);
  llvm_unreachable("Unexpected semantics");
}

APFloat::APFloat(ಸ್ಥಿರಾಂಕ fltSemantics &Semantics, StringRef S)
    : APFloat(Semantics) {
  ಸ್ವಯಂಚಾಲಿತ ಸ್ಥಿತಿ_Or_Err = convertFromString(S, rm_ಹತ್ತಿರದ_Ties_ಗಮ್ಯ_Even);
  ಪ್ರತಿಜ್ಞಾ(ಸ್ಥಿತಿ_Or_Err && "Invalid floating point representation");
  consumeError(ಸ್ಥಿತಿ_Or_Err.takeError());
}

FPClassTest APFloat::classify() ಸ್ಥಿರಾಂಕ {
  ಆದರೆ (isZero())
    ಹಿಂತಿರುಗಿ isNegative() ? fcNegZero : fcPosZero;
  ಆದರೆ (is_ಸಾಮಾನ್ಯ())
    ಹಿಂತಿರುಗಿ isNegative() ? fc_Neg_ಸಾಮಾನ್ಯ : fc_Pos_ಸಾಮಾನ್ಯ;
  ಆದರೆ (isDenormal())
    ಹಿಂತಿರುಗಿ isNegative() ? fcNegSubnormal : fcPosSubnormal;
  ಆದರೆ (isInfinity())
    ಹಿಂತಿರುಗಿ isNegative() ? fcNegInf : fcPosInf;
  ಪ್ರತಿಜ್ಞಾ(isNaN() && "Other class of FP constant");
  ಹಿಂತಿರುಗಿ isSignaling() ? fcSNan : fcQNan;
}

ತಾರ್ಕಿಕ APFloat::getExactInverse(APFloat *Inv) ಸ್ಥಿರಾಂಕ {
  // Only finite, non-zero numbers can have a useful, representable inverse.
  // This check filters out +/- zero, +/- infinity, and NaN.
  ಆದರೆ (!isFiniteNonZero())
    ಹಿಂತಿರುಗಿ ಸುಳ್ಳು;

  // Historically, this function rejects subnormal inputs.  One reason why this
  // might be important is that subnormals may behave differently under FTZ/DAZ
  // runtime behavior.
  ಆದರೆ (isDenormal())
    ಹಿಂತಿರುಗಿ ಸುಳ್ಳು;

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
  ಸ್ಥಿರಾಂಕ ಪೂರ್ಣಾಂಕ Exp = getExactLog2Abs();
  ಆದರೆ (Exp == INT_MIN)
    ಹಿಂತಿರುಗಿ ಸುಳ್ಳು;

  // The inverse of +/- 2^Exp is +/- 2^(-Exp). We can compute this by
  // scaling 1.0 by the negated exponent.
  APFloat Reciprocal =
      scalbn(APFloat::getOne(getSemantics(), /*Negative=*/isNegative()), -Exp,
             rmTowardZero);

  // scalbn might round if the resulting exponent -Exp is outside the
  // representable range, causing overflow (to infinity) or underflow. We
  // must verify that the result is still the exact power of two we expect.
  ಆದರೆ (Reciprocal.getExactLog2Abs() != -Exp)
    ಹಿಂತಿರುಗಿ ಸುಳ್ಳು;

  // Avoid multiplication with a subnormal, it is not safe on all platforms and
  // may be slower than a normal division.
  ಆದರೆ (Reciprocal.isDenormal())
    ಹಿಂತಿರುಗಿ ಸುಳ್ಳು;

  ಪ್ರತಿಜ್ಞಾ(Reciprocal.isFiniteNonZero());

  ಆದರೆ (Inv)
    *Inv = std::move(Reciprocal);

  ಹಿಂತಿರುಗಿ ನಿಜ;
}

APFloat::op_ಸ್ಥಿತಿ APFloat::convert(ಸ್ಥಿರಾಂಕ fltSemantics &ಗಮ್ಯ_Semantics_2,
                                   rounding_ಬಹುಲಕ RM, ತಾರ್ಕಿಕ *losesInfo) {
  ಆದರೆ (&getSemantics() == &ಗಮ್ಯ_Semantics_2) {
    *losesInfo = ಸುಳ್ಳು;
    ಹಿಂತಿರುಗಿ opOK;
  }
  ಆದರೆ (ಬಳಸುತ್ತದೆ_Layout<IEEEFloat>(getSemantics()) &&
      ಬಳಸುತ್ತದೆ_Layout<IEEEFloat>(ಗಮ್ಯ_Semantics_2))
    ಹಿಂತಿರುಗಿ U.IEEE.convert(ಗಮ್ಯ_Semantics_2, RM, losesInfo);
  ಆದರೆ (ಬಳಸುತ್ತದೆ_Layout<IEEEFloat>(getSemantics()) &&
      ಬಳಸುತ್ತದೆ_Layout<DoubleAPFloat>(ಗಮ್ಯ_Semantics_2)) {
    ಪ್ರತಿಜ್ಞಾ(&ಗಮ್ಯ_Semantics_2 == &APFloat_ತಳ::semPPCDoubleDouble);
    ಸ್ವಯಂಚಾಲಿತ Ret =
        U.IEEE.convert(APFloat_ತಳ::semPPCDoubleDoubleLegacy, RM, losesInfo);
    *ಇದು = APFloat(ಗಮ್ಯ_Semantics_2, U.IEEE.bitcast_ಗಮ್ಯ_APInt());
    ಹಿಂತಿರುಗಿ Ret;
  }
  ಆದರೆ (ಬಳಸುತ್ತದೆ_Layout<DoubleAPFloat>(getSemantics()) &&
      ಬಳಸುತ್ತದೆ_Layout<IEEEFloat>(ಗಮ್ಯ_Semantics_2)) {
    ಸ್ವಯಂಚಾಲಿತ Ret = getIEEE().convert(ಗಮ್ಯ_Semantics_2, RM, losesInfo);
    *ಇದು = APFloat(std::move(getIEEE()), ಗಮ್ಯ_Semantics_2);
    ಹಿಂತಿರುಗಿ Ret;
  }
  llvm_unreachable("Unexpected semantics");
}

APFloat APFloat::get_All_Ones_ಮೌಲ್ಯ(ಸ್ಥಿರಾಂಕ fltSemantics &Semantics) {
  ಹಿಂತಿರುಗಿ APFloat(Semantics, APInt::getAllOnes(Semantics.ಗಾತ್ರ_ಒಳ_ಬಿಟ್ಟುಗಳು));
}

ಶೂನ್ಯ APFloat::print(raw_ostream &OS) ಸ್ಥಿರಾಂಕ {
  Small_ಸದಿಶ<ಅಕ್ಷರ, 16> ಬಫರ್;
  ಗಮ್ಯ_String(ಬಫರ್);
  OS << ಬಫರ್;
}

#ಆದರೆ !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)
LLVM_DUMP_ವಿಧಾನ ಶೂನ್ಯ APFloat::dump() ಸ್ಥಿರಾಂಕ {
  print(dbgs());
  dbgs() << '\n';
}
#ಆದರೆ_ಅಂತ್ಯ

ಶೂನ್ಯ APFloat::Profile(Folding_ಗಣ_ಗಂಟು_ID &NID) ಸ್ಥಿರಾಂಕ {
  NID.ಕೂಡಿಸು_2(bitcast_ಗಮ್ಯ_APInt());
}

APFloat::op_ಸ್ಥಿತಿ APFloat::convert_ಗಮ್ಯ_Integer(APSInt &ಫಲಿತಾಂಶ_2,
                                            rounding_ಬಹುಲಕ rounding_ಬಹುಲಕ_2,
                                            ತಾರ್ಕಿಕ *isExact) ಸ್ಥಿರಾಂಕ {
  ಚಿಹ್ನೆರಹಿತ ಬಿಟ್_ಅಗಲ = ಫಲಿತಾಂಶ_2.get_ಬಿಟ್_ಅಗಲ();
  Small_ಸದಿಶ<uint64_t, 4> parts(ಫಲಿತಾಂಶ_2.get_ಅಂಕೆ_ಪದಗಳು());
  op_ಸ್ಥಿತಿ ಸ್ಥಿತಿ_3 = convert_ಗಮ್ಯ_Integer(parts, ಬಿಟ್_ಅಗಲ, ಫಲಿತಾಂಶ_2.isSigned(),
                                     rounding_ಬಹುಲಕ_2, isExact);
  // Keeps the original signed-ness.
  ಫಲಿತಾಂಶ_2 = APInt(ಬಿಟ್_ಅಗಲ, parts);
  ಹಿಂತಿರುಗಿ ಸ್ಥಿತಿ_3;
}

ದ್ವಿಗುಣ APFloat::convert_ಗಮ್ಯ_Double() ಸ್ಥಿರಾಂಕ {
  ಆದರೆ (&getSemantics() == &APFloat_ತಳ::semIEEEdouble)
    ಹಿಂತಿರುಗಿ getIEEE().convert_ಗಮ್ಯ_Double();
  ಪ್ರತಿಜ್ಞಾ(isRepresentableBy(getSemantics(), semIEEEdouble) &&
         "Float semantics is not representable by IEEEdouble");
  APFloat ತಾತ್ಕಾಲಿಕ = *ಇದು;
  ತಾರ್ಕಿಕ LosesInfo;
  [[maybe_ಬಳಸದ]] op_ಸ್ಥಿತಿ St =
      ತಾತ್ಕಾಲಿಕ.convert(APFloat_ತಳ::semIEEEdouble, rm_ಹತ್ತಿರದ_Ties_ಗಮ್ಯ_Even, &LosesInfo);
  ಪ್ರತಿಜ್ಞಾ(!(St & opInexact) && !LosesInfo && "Unexpected imprecision");
  ಹಿಂತಿರುಗಿ ತಾತ್ಕಾಲಿಕ.getIEEE().convert_ಗಮ್ಯ_Double();
}

#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ HAS_IEE754_FLOAT128
float128 APFloat::convert_ಗಮ್ಯ_Quad() ಸ್ಥಿರಾಂಕ {
  ಆದರೆ (&getSemantics() == &APFloat_ತಳ::semIEEEquad)
    ಹಿಂತಿರುಗಿ getIEEE().convert_ಗಮ್ಯ_Quad();
  ಪ್ರತಿಜ್ಞಾ(isRepresentableBy(getSemantics(), semIEEEquad) &&
         "Float semantics is not representable by IEEEquad");
  APFloat ತಾತ್ಕಾಲಿಕ = *ಇದು;
  ತಾರ್ಕಿಕ LosesInfo;
  [[maybe_ಬಳಸದ]] op_ಸ್ಥಿತಿ St =
      ತಾತ್ಕಾಲಿಕ.convert(APFloat_ತಳ::semIEEEquad, rm_ಹತ್ತಿರದ_Ties_ಗಮ್ಯ_Even, &LosesInfo);
  ಪ್ರತಿಜ್ಞಾ(!(St & opInexact) && !LosesInfo && "Unexpected imprecision");
  ಹಿಂತಿರುಗಿ ತಾತ್ಕಾಲಿಕ.getIEEE().convert_ಗಮ್ಯ_Quad();
}
#ಆದರೆ_ಅಂತ್ಯ

ದಶಮಾಂಶ APFloat::convert_ಗಮ್ಯ_Float() ಸ್ಥಿರಾಂಕ {
  ಆದರೆ (&getSemantics() == &APFloat_ತಳ::semIEEEsingle)
    ಹಿಂತಿರುಗಿ getIEEE().convert_ಗಮ್ಯ_Float();
  ಪ್ರತಿಜ್ಞಾ(isRepresentableBy(getSemantics(), semIEEEsingle) &&
         "Float semantics is not representable by IEEEsingle");
  APFloat ತಾತ್ಕಾಲಿಕ = *ಇದು;
  ತಾರ್ಕಿಕ LosesInfo;
  [[maybe_ಬಳಸದ]] op_ಸ್ಥಿತಿ St =
      ತಾತ್ಕಾಲಿಕ.convert(APFloat_ತಳ::semIEEEsingle, rm_ಹತ್ತಿರದ_Ties_ಗಮ್ಯ_Even, &LosesInfo);
  ಪ್ರತಿಜ್ಞಾ(!(St & opInexact) && !LosesInfo && "Unexpected imprecision");
  ಹಿಂತಿರುಗಿ ತಾತ್ಕಾಲಿಕ.getIEEE().convert_ಗಮ್ಯ_Float();
}

ಚಿಹ್ನೆರಹಿತ APFloat_ತಳ::get_Arbitrary_FPFormat_ಗಾತ್ರ_ಒಳ_ಬಿಟ್ಟುಗಳು(StringRef Format) {
  ಹಿಂತಿರುಗಿ StringSwitch<ಚಿಹ್ನೆರಹಿತ>(Format)
      .Case("Float8E5M2", get_ಗಾತ್ರ_ಒಳ_ಬಿಟ್ಟುಗಳು(semFloat8E5M2))
      .Case("Float8E5M2FNUZ", get_ಗಾತ್ರ_ಒಳ_ಬಿಟ್ಟುಗಳು(semFloat8E5M2FNUZ))
      .Case("Float8E4M3", get_ಗಾತ್ರ_ಒಳ_ಬಿಟ್ಟುಗಳು(semFloat8E4M3))
      .Case("Float8E4M3FN", get_ಗಾತ್ರ_ಒಳ_ಬಿಟ್ಟುಗಳು(semFloat8E4M3FN))
      .Case("Float8E4M3FNUZ", get_ಗಾತ್ರ_ಒಳ_ಬಿಟ್ಟುಗಳು(semFloat8E4M3FNUZ))
      .Case("Float8E4M3B11FNUZ", get_ಗಾತ್ರ_ಒಳ_ಬಿಟ್ಟುಗಳು(semFloat8E4M3B11FNUZ))
      .Case("Float8E3M4", get_ಗಾತ್ರ_ಒಳ_ಬಿಟ್ಟುಗಳು(semFloat8E3M4))
      .Case("Float8E8M0FNU", get_ಗಾತ್ರ_ಒಳ_ಬಿಟ್ಟುಗಳು(semFloat8E8M0FNU))
      .Case("Float6E3M2FN", get_ಗಾತ್ರ_ಒಳ_ಬಿಟ್ಟುಗಳು(semFloat6E3M2FN))
      .Case("Float6E2M3FN", get_ಗಾತ್ರ_ಒಳ_ಬಿಟ್ಟುಗಳು(semFloat6E2M3FN))
      .Case("Float4E2M1FN", get_ಗಾತ್ರ_ಒಳ_ಬಿಟ್ಟುಗಳು(semFloat4E2M1FN))
      .Case("Float8E5M3FNU", get_ಗಾತ್ರ_ಒಳ_ಬಿಟ್ಟುಗಳು(semFloat8E5M3FNU))
      .Default(0);
}

ತಾರ್ಕಿಕ APFloat_ತಳ::isValidArbitraryFPFormat(StringRef Format) {
  ಹಿಂತಿರುಗಿ get_Arbitrary_FPFormat_ಗಾತ್ರ_ಒಳ_ಬಿಟ್ಟುಗಳು(Format) != 0;
}

ಸ್ಥಿರಾಂಕ fltSemantics *APFloat_ತಳ::getArbitraryFPSemantics(StringRef Format) {
  // TODO: extend to remaining arbitrary FP types: Float8E4M3, Float8E3M4,
  // Float8E5M2FNUZ, Float8E4M3FNUZ, Float8E4M3B11FNUZ, Float8E8M0FNU.
  ಹಿಂತಿರುಗಿ StringSwitch<ಸ್ಥಿರಾಂಕ fltSemantics *>(Format)
      .Case("Float8E5M2", &semFloat8E5M2)
      .Case("Float8E4M3FN", &semFloat8E4M3FN)
      .Case("Float8E5M3FNU", &semFloat8E5M3FNU)
      .Case("Float4E2M1FN", &semFloat4E2M1FN)
      .Case("Float6E3M2FN", &semFloat6E3M2FN)
      .Case("Float6E2M3FN", &semFloat6E2M3FN)
      .Default(ಶೂನ್ಯಸೂಚಕ);
}

APFloat::Storage::~Storage() {
  ಆದರೆ (ಬಳಸುತ್ತದೆ_Layout<IEEEFloat>(*semantics)) {
    IEEE.~IEEEFloat();
    ಹಿಂತಿರುಗಿ;
  }
  ಆದರೆ (ಬಳಸುತ್ತದೆ_Layout<DoubleAPFloat>(*semantics)) {
    Double.~DoubleAPFloat();
    ಹಿಂತಿರುಗಿ;
  }
  llvm_unreachable("Unexpected semantics");
}

APFloat::Storage::Storage(ಸ್ಥಿರಾಂಕ APFloat::Storage &RHS) {
  ಆದರೆ (ಬಳಸುತ್ತದೆ_Layout<IEEEFloat>(*RHS.semantics)) {
    ಹೊಸ (ಇದು) IEEEFloat(RHS.IEEE);
    ಹಿಂತಿರುಗಿ;
  }
  ಆದರೆ (ಬಳಸುತ್ತದೆ_Layout<DoubleAPFloat>(*RHS.semantics)) {
    ಹೊಸ (ಇದು) DoubleAPFloat(RHS.Double);
    ಹಿಂತಿರುಗಿ;
  }
  llvm_unreachable("Unexpected semantics");
}

APFloat::Storage::Storage(APFloat::Storage &&RHS) {
  ಆದರೆ (ಬಳಸುತ್ತದೆ_Layout<IEEEFloat>(*RHS.semantics)) {
    ಹೊಸ (ಇದು) IEEEFloat(std::move(RHS.IEEE));
    ಹಿಂತಿರುಗಿ;
  }
  ಆದರೆ (ಬಳಸುತ್ತದೆ_Layout<DoubleAPFloat>(*RHS.semantics)) {
    ಹೊಸ (ಇದು) DoubleAPFloat(std::move(RHS.Double));
    ಹಿಂತಿರುಗಿ;
  }
  llvm_unreachable("Unexpected semantics");
}

APFloat::Storage &APFloat::Storage::ಕ್ರಿಯಾಕಾರಕ=(ಸ್ಥಿರಾಂಕ APFloat::Storage &RHS) {
  ಆದರೆ (ಬಳಸುತ್ತದೆ_Layout<IEEEFloat>(*semantics) &&
      ಬಳಸುತ್ತದೆ_Layout<IEEEFloat>(*RHS.semantics)) {
    IEEE = RHS.IEEE;
  } ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (ಬಳಸುತ್ತದೆ_Layout<DoubleAPFloat>(*semantics) &&
             ಬಳಸುತ್ತದೆ_Layout<DoubleAPFloat>(*RHS.semantics)) {
    Double = RHS.Double;
  } ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (ಇದು != &RHS) {
    ಇದು->~Storage();
    ಹೊಸ (ಇದು) Storage(RHS);
  }
  ಹಿಂತಿರುಗಿ *ಇದು;
}

APFloat::Storage &APFloat::Storage::ಕ್ರಿಯಾಕಾರಕ=(APFloat::Storage &&RHS) {
  ಆದರೆ (ಬಳಸುತ್ತದೆ_Layout<IEEEFloat>(*semantics) &&
      ಬಳಸುತ್ತದೆ_Layout<IEEEFloat>(*RHS.semantics)) {
    IEEE = std::move(RHS.IEEE);
  } ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (ಬಳಸುತ್ತದೆ_Layout<DoubleAPFloat>(*semantics) &&
             ಬಳಸುತ್ತದೆ_Layout<DoubleAPFloat>(*RHS.semantics)) {
    Double = std::move(RHS.Double);
  } ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (ಇದು != &RHS) {
    ಇದು->~Storage();
    ಹೊಸ (ಇದು) Storage(std::move(RHS));
  }
  ಹಿಂತಿರುಗಿ *ಇದು;
}

ನಾಮಾಕಾಶ {

APFloat::op_ಸ್ಥಿತಿ get_Op_ಸ್ಥಿತಿ_From_Libc(ಪೂರ್ಣಾಂಕ libc_exceptions) {
  APFloat::op_ಸ್ಥಿತಿ ಸ್ಥಿತಿ_3 = APFloat::opOK;
  ಆದರೆ (libc_exceptions & FE_INVALID)
    ಸ್ಥಿತಿ_3 = static_cast<APFloat::op_ಸ್ಥಿತಿ>(ಸ್ಥಿತಿ_3 | APFloat::opInvalidOp);
  ಆದರೆ (libc_exceptions & FE_DIVBYZERO)
    ಸ್ಥಿತಿ_3 = static_cast<APFloat::op_ಸ್ಥಿತಿ>(ಸ್ಥಿತಿ_3 | APFloat::opDivByZero);
  ಆದರೆ (libc_exceptions & FE_ಅತಿಪ್ರವಾಹ)
    ಸ್ಥಿತಿ_3 = static_cast<APFloat::op_ಸ್ಥಿತಿ>(ಸ್ಥಿತಿ_3 | APFloat::op_ಅತಿಪ್ರವಾಹ);
  ಆದರೆ (libc_exceptions & FE_UNDERFLOW)
    ಸ್ಥಿತಿ_3 = static_cast<APFloat::op_ಸ್ಥಿತಿ>(ಸ್ಥಿತಿ_3 | APFloat::opUnderflow);
  ಆದರೆ (libc_exceptions & FE_INEXACT)
    ಸ್ಥಿತಿ_3 = static_cast<APFloat::op_ಸ್ಥಿತಿ>(ಸ್ಥಿತಿ_3 | APFloat::opInexact);
  ಹಿಂತಿರುಗಿ ಸ್ಥಿತಿ_3;
}

} // namespace

// TODO: Support other rounding modes when LLVM libc math implement static
// roundings.
std::optional<APFloat> ಘಾತಾಂಕ(ಸ್ಥಿರಾಂಕ APFloat &x, Rounding_ಬಹುಲಕ rounding_ಬಹುಲಕ_2,
                           APFloat::op_ಸ್ಥಿತಿ *ಸ್ಥಿತಿ_3) {

  ಆದರೆ (rounding_ಬಹುಲಕ_2 == APFloat_ತಳ::rm_ಹತ್ತಿರದ_Ties_ಗಮ್ಯ_Even) {
    ಆದರೆ (APFloat::Semantics_ಗಮ್ಯ_Enum(x.getSemantics()) ==
        APFloat_ತಳ::S_IEEEsingle) {
      ದಶಮಾಂಶ x_val = x.convert_ಗಮ್ಯ_Float();
      ಪೂರ್ಣಾಂಕ exc =
          LIBC_NAMESPACE::shared::check::exp_exceptions(x_val, FE_TONEAREST);
      ಆದರೆ (ಸ್ಥಿತಿ_3) {
        *ಸ್ಥಿತಿ_3 = get_Op_ಸ್ಥಿತಿ_From_Libc(exc);
        ಆದರೆ (x.isSignaling()) {
          // 32-bit x86 will silence sNaN when loading floats, so we explicitly
          // add the INVALID exception here.
          *ಸ್ಥಿತಿ_3 =
              static_cast<APFloat::op_ಸ್ಥಿತಿ>(*ಸ್ಥಿತಿ_3 | APFloat::opInvalidOp);
        }
      }
      ದಶಮಾಂಶ ಫಲಿತಾಂಶ_2 = LIBC_NAMESPACE::shared::expf(x_val);
      ಹಿಂತಿರುಗಿ APFloat(ಫಲಿತಾಂಶ_2);
    }
    ಆದರೆ (APFloat::Semantics_ಗಮ್ಯ_Enum(x.getSemantics()) ==
        APFloat_ತಳ::S_IEEEdouble) {
      ದ್ವಿಗುಣ x_val = x.convert_ಗಮ್ಯ_Double();
      ಪೂರ್ಣಾಂಕ exc =
          LIBC_NAMESPACE::shared::check::exp_exceptions(x_val, FE_TONEAREST);
      ಆದರೆ (ಸ್ಥಿತಿ_3) {
        *ಸ್ಥಿತಿ_3 = get_Op_ಸ್ಥಿತಿ_From_Libc(exc);
        ಆದರೆ (x.isSignaling()) {
          // 32-bit x86 will silence sNaN when loading floats, so we explicitly
          // add the INVALID exception here.
          *ಸ್ಥಿತಿ_3 =
              static_cast<APFloat::op_ಸ್ಥಿತಿ>(*ಸ್ಥಿತಿ_3 | APFloat::opInvalidOp);
        }
      }
      ದ್ವಿಗುಣ ಫಲಿತಾಂಶ_2 = LIBC_NAMESPACE::shared::ಘಾತಾಂಕ(x_val);
      ಹಿಂತಿರುಗಿ APFloat(ಫಲಿತಾಂಶ_2);
    }
  }
  ಹಿಂತಿರುಗಿ std::nullopt;
}

} // namespace llvm

#ವ್ಯಾಖ್ಯಾನ_ರದ್ದು APFLOAT_DISPATCH_ON_SEMANTICS
