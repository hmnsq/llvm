//===-- APInt.cpp - Implement APInt class ---------------------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
// This file implements a class to represent arbitrary precision integer
// constant values and provide a variety of arithmetic operations on them.
//
//===----------------------------------------------------------------------===//

#ಅನ್ತರ್ಭಾವ "llvm/ADT/APInt.h"
#ಅನ್ತರ್ಭಾವ "llvm/ADT/ArrayRef.h"
#ಅನ್ತರ್ಭಾವ "llvm/ADT/FoldingSet.h"
#ಅನ್ತರ್ಭಾವ "llvm/ADT/Hashing.h"
#ಅನ್ತರ್ಭಾವ "llvm/ADT/Sequence.h"
#ಅನ್ತರ್ಭಾವ "llvm/ADT/SmallString.h"
#ಅನ್ತರ್ಭಾವ "llvm/ADT/StringRef.h"
#ಅನ್ತರ್ಭಾವ "llvm/ADT/bit.h"
#ಅನ್ತರ್ಭಾವ "llvm/Support/Alignment.h"
#ಅನ್ತರ್ಭಾವ "llvm/Support/Debug.h"
#ಅನ್ತರ್ಭಾವ "llvm/Support/ErrorHandling.h"
#ಅನ್ತರ್ಭಾವ "llvm/Support/MathExtras.h"
#ಅನ್ತರ್ಭಾವ "llvm/Support/SwapByteOrder.h"
#ಅನ್ತರ್ಭಾವ "llvm/Support/raw_ostream.h"
#ಅನ್ತರ್ಭಾವ <cmath>
#ಅನ್ತರ್ಭಾವ <optional>

ಬಳಸಿ ನಾಮಾಕಾಶ llvm;

#ವ್ಯಾಖ್ಯಾನಿಸು DEBUG_TYPE "apint"

/// A utility function for allocating memory, checking for allocation failures,
/// and ensuring the contents are zeroed.
ಸಾಲೊಳಗೆ ಸ್ಥಿರ uint64_t* get_Cleared_ಸ್ಮರಣೆ(ಚಿಹ್ನೆರಹಿತ ಅಂಕೆ_ಪದಗಳು) {
  ಹಿಂತಿರುಗಿ ಹೊಸ uint64_t[ಅಂಕೆ_ಪದಗಳು]();
}

/// A utility function for allocating memory and checking for allocation
/// failure.  The content is not zeroed.
ಸಾಲೊಳಗೆ ಸ್ಥಿರ uint64_t* get_ಸ್ಮರಣೆ(ಚಿಹ್ನೆರಹಿತ ಅಂಕೆ_ಪದಗಳು) {
  ಹಿಂತಿರುಗಿ ಹೊಸ uint64_t[ಅಂಕೆ_ಪದಗಳು];
}

/// A utility function that converts a character to a digit.
ಸಾಲೊಳಗೆ ಸ್ಥಿರ ಚಿಹ್ನೆರಹಿತ getDigit(ಅಕ್ಷರ cdigit, uint8_t radix) {
  ಚಿಹ್ನೆರಹಿತ r;

  ಆದರೆ (radix == 16 || radix == 36) {
    r = cdigit - '0';
    ಆದರೆ (r <= 9)
      ಹಿಂತಿರುಗಿ r;

    r = cdigit - 'A';
    ಆದರೆ (r <= radix - 11U)
      ಹಿಂತಿರುಗಿ r + 10;

    r = cdigit - 'a';
    ಆದರೆ (r <= radix - 11U)
      ಹಿಂತಿರುಗಿ r + 10;

    radix = 10;
  }

  r = cdigit - '0';
  ಆದರೆ (r < radix)
    ಹಿಂತಿರುಗಿ r;

  ಹಿಂತಿರುಗಿ UINT_MAX;
}


ಶೂನ್ಯ APInt::initSlowCase(uint64_t val, ತಾರ್ಕಿಕ isSigned) {
  ಆದರೆ (isSigned && int64_t(val) < 0) {
    U.pVal = get_ಸ್ಮರಣೆ(get_ಅಂಕೆ_ಪದಗಳು());
    U.pVal[0] = val;
    memset(&U.pVal[1], 0xFF, APINT_ಶಬ್ದ_ಗಾತ್ರ * (get_ಅಂಕೆ_ಪದಗಳು() - 1));
    clear_ಬಳಸದ_ಬಿಟ್ಟುಗಳು();
  } ಇಲ್ಲದಿದ್ದರೆ {
    U.pVal = get_Cleared_ಸ್ಮರಣೆ(get_ಅಂಕೆ_ಪದಗಳು());
    U.pVal[0] = val;
  }
}

ಶೂನ್ಯ APInt::initSlowCase(ಸ್ಥಿರಾಂಕ APInt& that) {
  U.pVal = get_ಸ್ಮರಣೆ(get_ಅಂಕೆ_ಪದಗಳು());
  ಸ್ಮರಣೆ_ನಕಲು(U.pVal, that.U.pVal, get_ಅಂಕೆ_ಪದಗಳು() * APINT_ಶಬ್ದ_ಗಾತ್ರ);
}

ಶೂನ್ಯ APInt::initFromArray(ArrayRef<uint64_t> bigVal) {
  ಪ್ರತಿಜ್ಞಾ(bigVal.ದತ್ತಾಂಶ() && "Null pointer detected!");
  ಆದರೆ (LLVM_LIKELY(is_Single_ಶಬ್ದ()))
    U.VAL = bigVal[0];
  ಇಲ್ಲದಿದ್ದರೆ {
    // Get memory, cleared to 0
    U.pVal = get_Cleared_ಸ್ಮರಣೆ(get_ಅಂಕೆ_ಪದಗಳು());
    // Calculate the number of words to copy
    ಚಿಹ್ನೆರಹಿತ ಪದಗಳು = std::min<ಚಿಹ್ನೆರಹಿತ>(bigVal.ಗಾತ್ರ_3(), get_ಅಂಕೆ_ಪದಗಳು());
    // Copy the words from bigVal to pVal
    ಸ್ಮರಣೆ_ನಕಲು(U.pVal, bigVal.ದತ್ತಾಂಶ(), ಪದಗಳು * APINT_ಶಬ್ದ_ಗಾತ್ರ);
  }
  // Make sure unused high bits are cleared
  clear_ಬಳಸದ_ಬಿಟ್ಟುಗಳು();
}

APInt::APInt(ಚಿಹ್ನೆರಹಿತ ಅಂಕೆ_ಬಿಟ್ಟುಗಳು, ArrayRef<uint64_t> bigVal) : ಬಿಟ್_ಅಗಲ_2(ಅಂಕೆ_ಬಿಟ್ಟುಗಳು) {
  initFromArray(bigVal);
}

APInt::APInt(ಚಿಹ್ನೆರಹಿತ numbits, StringRef Str, uint8_t radix)
    : ಬಿಟ್_ಅಗಲ_2(numbits) {
  fromString(numbits, Str, radix);
}

ಶೂನ್ಯ APInt::reallocate(ಚಿಹ್ನೆರಹಿತ New_ಬಿಟ್_ಅಗಲ) {
  // If the number of words is the same we can just change the width and stop.
  ಆದರೆ (get_ಅಂಕೆ_ಪದಗಳು() == get_ಅಂಕೆ_ಪದಗಳು(New_ಬಿಟ್_ಅಗಲ)) {
    ಬಿಟ್_ಅಗಲ_2 = New_ಬಿಟ್_ಅಗಲ;
    ಹಿಂತಿರುಗಿ;
  }

  // If we have an allocation, delete it.
  ಆದರೆ (LLVM_UNLIKELY(!is_Single_ಶಬ್ದ()))
    ಅಳಿಸು [] U.pVal;

  // Update BitWidth.
  ಬಿಟ್_ಅಗಲ_2 = New_ಬಿಟ್_ಅಗಲ;

  // If we are supposed to have an allocation, create it.
  ಆದರೆ (LLVM_UNLIKELY(!is_Single_ಶಬ್ದ()))
    U.pVal = get_ಸ್ಮರಣೆ(get_ಅಂಕೆ_ಪದಗಳು());
}

ಶೂನ್ಯ APInt::ನಿಯೋಜಿಸು_Slow_Case(ಸ್ಥಿರಾಂಕ APInt &RHS) {
  // Don't do anything for X = X
  ಆದರೆ (ಇದು == &RHS)
    ಹಿಂತಿರುಗಿ;

  // Adjust the bit width and handle allocations as necessary.
  reallocate(RHS.get_ಬಿಟ್_ಅಗಲ());

  // Copy the data.
  ಆದರೆ (LLVM_LIKELY(is_Single_ಶಬ್ದ()))
    U.VAL = RHS.U.VAL;
  ಇಲ್ಲದಿದ್ದರೆ
    ಸ್ಮರಣೆ_ನಕಲು(U.pVal, RHS.U.pVal, get_ಅಂಕೆ_ಪದಗಳು() * APINT_ಶಬ್ದ_ಗಾತ್ರ);
}

/// This method 'profiles' an APInt for use with FoldingSet.
ಶೂನ್ಯ APInt::Profile(Folding_ಗಣ_ಗಂಟು_ID& ID) ಸ್ಥಿರಾಂಕ {
  ID.ಕೂಡಿಸು_Integer(ಬಿಟ್_ಅಗಲ_2);

  ಆದರೆ (LLVM_LIKELY(is_Single_ಶಬ್ದ())) {
    ID.ಕೂಡಿಸು_Integer(U.VAL);
    ಹಿಂತಿರುಗಿ;
  }

  ಚಿಹ್ನೆರಹಿತ ಅಂಕೆ_ಪದಗಳು_2 = get_ಅಂಕೆ_ಪದಗಳು();
  ಗಾಗಿ (ಚಿಹ್ನೆರಹಿತ i = 0; i < ಅಂಕೆ_ಪದಗಳು_2; ++i)
    ID.ಕೂಡಿಸು_Integer(U.pVal[i]);
}

ತಾರ್ಕಿಕ APInt::isAligned(Align A) ಸ್ಥಿರಾಂಕ {
  ಆದರೆ (isZero())
    ಹಿಂತಿರುಗಿ ನಿಜ;
  ಸ್ಥಿರಾಂಕ ಚಿಹ್ನೆರಹಿತ TrailingZeroes = countr_zero();
  ಸ್ಥಿರಾಂಕ ಚಿಹ್ನೆರಹಿತ MinimumTrailingZeroes = Log2(A);
  ಹಿಂತಿರುಗಿ TrailingZeroes >= MinimumTrailingZeroes;
}

/// Prefix increment operator. Increments the APInt by one.
APInt& APInt::ಕ್ರಿಯಾಕಾರಕ++() {
  ಆದರೆ (LLVM_LIKELY(is_Single_ಶಬ್ದ()))
    ++U.VAL;
  ಇಲ್ಲದಿದ್ದರೆ
    tcIncrement(U.pVal, get_ಅಂಕೆ_ಪದಗಳು());
  ಹಿಂತಿರುಗಿ clear_ಬಳಸದ_ಬಿಟ್ಟುಗಳು();
}

/// Prefix decrement operator. Decrements the APInt by one.
APInt& APInt::ಕ್ರಿಯಾಕಾರಕ--() {
  ಆದರೆ (LLVM_LIKELY(is_Single_ಶಬ್ದ()))
    --U.VAL;
  ಇಲ್ಲದಿದ್ದರೆ
    tcDecrement(U.pVal, get_ಅಂಕೆ_ಪದಗಳು());
  ಹಿಂತಿರುಗಿ clear_ಬಳಸದ_ಬಿಟ್ಟುಗಳು();
}

/// Adds the RHS APInt to this APInt.
/// @returns this, after addition of RHS.
/// Addition assignment operator.
APInt& APInt::ಕ್ರಿಯಾಕಾರಕ+=(ಸ್ಥಿರಾಂಕ APInt& RHS) {
  ಪ್ರತಿಜ್ಞಾ(ಬಿಟ್_ಅಗಲ_2 == RHS.ಬಿಟ್_ಅಗಲ_2 && "Bit widths must be the same");
  ಆದರೆ (LLVM_LIKELY(is_Single_ಶಬ್ದ()))
    U.VAL += RHS.U.VAL;
  ಇಲ್ಲದಿದ್ದರೆ
    tc_ಕೂಡಿಸು(U.pVal, RHS.U.pVal, 0, get_ಅಂಕೆ_ಪದಗಳು());
  ಹಿಂತಿರುಗಿ clear_ಬಳಸದ_ಬಿಟ್ಟುಗಳು();
}

APInt& APInt::ಕ್ರಿಯಾಕಾರಕ+=(uint64_t RHS) {
  ಆದರೆ (LLVM_LIKELY(is_Single_ಶಬ್ದ()))
    U.VAL += RHS;
  ಇಲ್ಲದಿದ್ದರೆ
    tc_ಕೂಡಿಸು_Part(U.pVal, RHS, get_ಅಂಕೆ_ಪದಗಳು());
  ಹಿಂತಿರುಗಿ clear_ಬಳಸದ_ಬಿಟ್ಟುಗಳು();
}

/// Subtracts the RHS APInt from this APInt
/// @returns this, after subtraction
/// Subtraction assignment operator.
APInt& APInt::ಕ್ರಿಯಾಕಾರಕ-=(ಸ್ಥಿರಾಂಕ APInt& RHS) {
  ಪ್ರತಿಜ್ಞಾ(ಬಿಟ್_ಅಗಲ_2 == RHS.ಬಿಟ್_ಅಗಲ_2 && "Bit widths must be the same");
  ಆದರೆ (LLVM_LIKELY(is_Single_ಶಬ್ದ()))
    U.VAL -= RHS.U.VAL;
  ಇಲ್ಲದಿದ್ದರೆ
    tc_ಕಳೆ(U.pVal, RHS.U.pVal, 0, get_ಅಂಕೆ_ಪದಗಳು());
  ಹಿಂತಿರುಗಿ clear_ಬಳಸದ_ಬಿಟ್ಟುಗಳು();
}

APInt& APInt::ಕ್ರಿಯಾಕಾರಕ-=(uint64_t RHS) {
  ಆದರೆ (LLVM_LIKELY(is_Single_ಶಬ್ದ()))
    U.VAL -= RHS;
  ಇಲ್ಲದಿದ್ದರೆ
    tc_ಕಳೆ_Part(U.pVal, RHS, get_ಅಂಕೆ_ಪದಗಳು());
  ಹಿಂತಿರುಗಿ clear_ಬಳಸದ_ಬಿಟ್ಟುಗಳು();
}

APInt APInt::ಕ್ರಿಯಾಕಾರಕ*(ಸ್ಥಿರಾಂಕ APInt& RHS) ಸ್ಥಿರಾಂಕ {
  ಪ್ರತಿಜ್ಞಾ(ಬಿಟ್_ಅಗಲ_2 == RHS.ಬಿಟ್_ಅಗಲ_2 && "Bit widths must be the same");
  ಆದರೆ (LLVM_LIKELY(is_Single_ಶಬ್ದ()))
    ಹಿಂತಿರುಗಿ APInt(ಬಿಟ್_ಅಗಲ_2, U.VAL * RHS.U.VAL, /*isSigned=*/ಸುಳ್ಳು,
                 /*implicitTrunc=*/ನಿಜ);

  APInt ಫಲಿತಾಂಶ(get_ಸ್ಮರಣೆ(get_ಅಂಕೆ_ಪದಗಳು()), get_ಬಿಟ್_ಅಗಲ());
  tcMultiply(ಫಲಿತಾಂಶ.U.pVal, U.pVal, RHS.U.pVal, get_ಅಂಕೆ_ಪದಗಳು());
  ಫಲಿತಾಂಶ.clear_ಬಳಸದ_ಬಿಟ್ಟುಗಳು();
  ಹಿಂತಿರುಗಿ ಫಲಿತಾಂಶ;
}

ಶೂನ್ಯ APInt::and_ನಿಯೋಜಿಸು_Slow_Case(ಸ್ಥಿರಾಂಕ APInt &RHS) {
  ಶಬ್ದ_Type *dst = U.pVal, *rhs = RHS.U.pVal;
  ಗಾಗಿ (size_t i = 0, e = get_ಅಂಕೆ_ಪದಗಳು(); i != e; ++i)
    dst[i] &= rhs[i];
}

ಶೂನ್ಯ APInt::or_ನಿಯೋಜಿಸು_Slow_Case(ಸ್ಥಿರಾಂಕ APInt &RHS) {
  ಶಬ್ದ_Type *dst = U.pVal, *rhs = RHS.U.pVal;
  ಗಾಗಿ (size_t i = 0, e = get_ಅಂಕೆ_ಪದಗಳು(); i != e; ++i)
    dst[i] |= rhs[i];
}

ಶೂನ್ಯ APInt::xor_ನಿಯೋಜಿಸು_Slow_Case(ಸ್ಥಿರಾಂಕ APInt &RHS) {
  ಶಬ್ದ_Type *dst = U.pVal, *rhs = RHS.U.pVal;
  ಗಾಗಿ (size_t i = 0, e = get_ಅಂಕೆ_ಪದಗಳು(); i != e; ++i)
    dst[i] ^= rhs[i];
}

APInt &APInt::ಕ್ರಿಯಾಕಾರಕ*=(ಸ್ಥಿರಾಂಕ APInt &RHS) {
  *ಇದು = *ಇದು * RHS;
  ಹಿಂತಿರುಗಿ *ಇದು;
}

APInt& APInt::ಕ್ರಿಯಾಕಾರಕ*=(uint64_t RHS) {
  ಆದರೆ (LLVM_LIKELY(is_Single_ಶಬ್ದ())) {
    U.VAL *= RHS;
  } ಇಲ್ಲದಿದ್ದರೆ {
    ಚಿಹ್ನೆರಹಿತ ಅಂಕೆ_ಪದಗಳು_2 = get_ಅಂಕೆ_ಪದಗಳು();
    tcMultiplyPart(U.pVal, U.pVal, RHS, 0, ಅಂಕೆ_ಪದಗಳು_2, ಅಂಕೆ_ಪದಗಳು_2, ಸುಳ್ಳು);
  }
  ಹಿಂತಿರುಗಿ clear_ಬಳಸದ_ಬಿಟ್ಟುಗಳು();
}

ತಾರ್ಕಿಕ APInt::equalSlowCase(ಸ್ಥಿರಾಂಕ APInt &RHS) ಸ್ಥಿರಾಂಕ {
  ಹಿಂತಿರುಗಿ std::equal(U.pVal, U.pVal + get_ಅಂಕೆ_ಪದಗಳು(), RHS.U.pVal);
}

ಪೂರ್ಣಾಂಕ APInt::compare(ಸ್ಥಿರಾಂಕ APInt& RHS) ಸ್ಥಿರಾಂಕ {
  ಪ್ರತಿಜ್ಞಾ(ಬಿಟ್_ಅಗಲ_2 == RHS.ಬಿಟ್_ಅಗಲ_2 && "Bit widths must be same for comparison");
  ಆದರೆ (LLVM_LIKELY(is_Single_ಶಬ್ದ()))
    ಹಿಂತಿರುಗಿ U.VAL < RHS.U.VAL ? -1 : U.VAL > RHS.U.VAL;

  ಹಿಂತಿರುಗಿ tcCompare(U.pVal, RHS.U.pVal, get_ಅಂಕೆ_ಪದಗಳು());
}

ಪೂರ್ಣಾಂಕ APInt::compareSigned(ಸ್ಥಿರಾಂಕ APInt& RHS) ಸ್ಥಿರಾಂಕ {
  ಪ್ರತಿಜ್ಞಾ(ಬಿಟ್_ಅಗಲ_2 == RHS.ಬಿಟ್_ಅಗಲ_2 && "Bit widths must be same for comparison");
  ಆದರೆ (LLVM_LIKELY(is_Single_ಶಬ್ದ())) {
    int64_t lhsSext = SignExtend64(U.VAL, ಬಿಟ್_ಅಗಲ_2);
    int64_t rhsSext = SignExtend64(RHS.U.VAL, ಬಿಟ್_ಅಗಲ_2);
    ಹಿಂತಿರುಗಿ lhsSext < rhsSext ? -1 : lhsSext > rhsSext;
  }

  ತಾರ್ಕಿಕ lhsNeg = isNegative();
  ತಾರ್ಕಿಕ rhsNeg = RHS.isNegative();

  // If the sign bits don't match, then (LHS < RHS) if LHS is negative
  ಆದರೆ (lhsNeg != rhsNeg)
    ಹಿಂತಿರುಗಿ lhsNeg ? -1 : 1;

  // Otherwise we can just use an unsigned comparison, because even negative
  // numbers compare correctly this way if both have the same signed-ness.
  ಹಿಂತಿರುಗಿ tcCompare(U.pVal, RHS.U.pVal, get_ಅಂಕೆ_ಪದಗಳು());
}

ಶೂನ್ಯ APInt::ಗಣ_ಬಿಟ್ಟುಗಳು_Slow_Case(ಚಿಹ್ನೆರಹಿತ lo_ಬಿಟ್, ಚಿಹ್ನೆರಹಿತ hi_ಬಿಟ್) {
  ಚಿಹ್ನೆರಹಿತ lo_ಶಬ್ದ = which_ಶಬ್ದ(lo_ಬಿಟ್);
  ಚಿಹ್ನೆರಹಿತ hi_ಶಬ್ದ = which_ಶಬ್ದ(hi_ಬಿಟ್);

  // Create an initial mask for the low word with zeros below loBit.
  uint64_t lo_ಮುಖವಾಡ = WORDTYPE_MAX << which_ಬಿಟ್(lo_ಬಿಟ್);

  // If hiBit is not aligned, we need a high mask.
  ಚಿಹ್ನೆರಹಿತ hi_ಸರಿಸು_Amt = which_ಬಿಟ್(hi_ಬಿಟ್);
  ಆದರೆ (hi_ಸರಿಸು_Amt != 0) {
    // Create a high mask with zeros above hiBit.
    uint64_t hi_ಮುಖವಾಡ = WORDTYPE_MAX >> (APINT_ಬಿಟ್ಟುಗಳು_ಪ್ರತಿ_ಶಬ್ದ - hi_ಸರಿಸು_Amt);
    // If loWord and hiWord are equal, then we combine the masks. Otherwise,
    // set the bits in hiWord.
    ಆದರೆ (hi_ಶಬ್ದ == lo_ಶಬ್ದ)
      lo_ಮುಖವಾಡ &= hi_ಮುಖವಾಡ;
    ಇಲ್ಲದಿದ್ದರೆ
      U.pVal[hi_ಶಬ್ದ] |= hi_ಮುಖವಾಡ;
  }
  // Apply the mask to the low word.
  U.pVal[lo_ಶಬ್ದ] |= lo_ಮುಖವಾಡ;

  // Fill any words between loWord and hiWord with all ones.
  ಗಾಗಿ (ಚಿಹ್ನೆರಹಿತ ಶಬ್ದ = lo_ಶಬ್ದ + 1; ಶಬ್ದ < hi_ಶಬ್ದ; ++ಶಬ್ದ)
    U.pVal[ಶಬ್ದ] = WORDTYPE_MAX;
}

ಶೂನ್ಯ APInt::clear_ಬಿಟ್ಟುಗಳು_Slow_Case(ಚಿಹ್ನೆರಹಿತ Lo_ಬಿಟ್, ಚಿಹ್ನೆರಹಿತ Hi_ಬಿಟ್) {
  ಚಿಹ್ನೆರಹಿತ Lo_ಶಬ್ದ = which_ಶಬ್ದ(Lo_ಬಿಟ್);
  ಚಿಹ್ನೆರಹಿತ Hi_ಶಬ್ದ = which_ಶಬ್ದ(Hi_ಬಿಟ್);

  // Create an initial mask for the low word with ones below loBit.
  uint64_t Lo_ಮುಖವಾಡ = ~(WORDTYPE_MAX << which_ಬಿಟ್(Lo_ಬಿಟ್));

  // If HiBit is not aligned, we need a high mask.
  ಚಿಹ್ನೆರಹಿತ Hi_ಸರಿಸು_Amt = which_ಬಿಟ್(Hi_ಬಿಟ್);
  ಆದರೆ (Hi_ಸರಿಸು_Amt != 0) {
    // Create a high mask with ones above HiBit.
    uint64_t Hi_ಮುಖವಾಡ = ~(WORDTYPE_MAX >> (APINT_ಬಿಟ್ಟುಗಳು_ಪ್ರತಿ_ಶಬ್ದ - Hi_ಸರಿಸು_Amt));
    // If LoWord and HiWord are equal, then we combine the masks. Otherwise,
    // clear the bits in HiWord.
    ಆದರೆ (Hi_ಶಬ್ದ == Lo_ಶಬ್ದ)
      Lo_ಮುಖವಾಡ |= Hi_ಮುಖವಾಡ;
    ಇಲ್ಲದಿದ್ದರೆ
      U.pVal[Hi_ಶಬ್ದ] &= Hi_ಮುಖವಾಡ;
  }
  // Apply the mask to the low word.
  U.pVal[Lo_ಶಬ್ದ] &= Lo_ಮುಖವಾಡ;

  // Fill any words between LoWord and HiWord with all zeros.
  ಗಾಗಿ (ಚಿಹ್ನೆರಹಿತ ಶಬ್ದ_2 = Lo_ಶಬ್ದ + 1; ಶಬ್ದ_2 < Hi_ಶಬ್ದ; ++ಶಬ್ದ_2)
    U.pVal[ಶಬ್ದ_2] = 0;
}

// Complement a bignum in-place.
ಸ್ಥಿರ ಶೂನ್ಯ tcComplement(APInt::ಶಬ್ದ_Type *dst, ಚಿಹ್ನೆರಹಿತ parts) {
  ಗಾಗಿ (ಚಿಹ್ನೆರಹಿತ i = 0; i < parts; i++)
    dst[i] = ~dst[i];
}

/// Toggle every bit to its opposite value.
ಶೂನ್ಯ APInt::flip_All_ಬಿಟ್ಟುಗಳು_Slow_Case() {
  tcComplement(U.pVal, get_ಅಂಕೆ_ಪದಗಳು());
  clear_ಬಳಸದ_ಬಿಟ್ಟುಗಳು();
}

/// Concatenate the bits from "NewLSB" onto the bottom of *this.  This is
/// equivalent to:
///   (this->zext(NewWidth) << NewLSB.getBitWidth()) | NewLSB.zext(NewWidth)
/// In the slow case, we know the result is large.
APInt APInt::concatSlowCase(ಸ್ಥಿರಾಂಕ APInt &NewLSB) ಸ್ಥಿರಾಂಕ {
  ಚಿಹ್ನೆರಹಿತ New_ಅಗಲ = get_ಬಿಟ್_ಅಗಲ() + NewLSB.get_ಬಿಟ್_ಅಗಲ();
  APInt ಫಲಿತಾಂಶ = NewLSB.zext(New_ಅಗಲ);
  ಫಲಿತಾಂಶ.ಸೇರಿಸು_ಬಿಟ್ಟುಗಳು(*ಇದು, NewLSB.get_ಬಿಟ್_ಅಗಲ());
  ಹಿಂತಿರುಗಿ ಫಲಿತಾಂಶ;
}

/// Toggle a given bit to its opposite value whose position is given
/// as "bitPosition".
/// Toggles a given bit to its opposite value.
ಶೂನ್ಯ APInt::flip_ಬಿಟ್(ಚಿಹ್ನೆರಹಿತ ಬಿಟ್_Position) {
  ಪ್ರತಿಜ್ಞಾ(ಬಿಟ್_Position < ಬಿಟ್_ಅಗಲ_2 && "Out of the bit-width range!");
  ಗಣ_ಬಿಟ್_Val(ಬಿಟ್_Position, !(*ಇದು)[ಬಿಟ್_Position]);
}

ಶೂನ್ಯ APInt::ಸೇರಿಸು_ಬಿಟ್ಟುಗಳು(ಸ್ಥಿರಾಂಕ APInt &sub_ಬಿಟ್ಟುಗಳು, ಚಿಹ್ನೆರಹಿತ ಬಿಟ್_Position) {
  ಚಿಹ್ನೆರಹಿತ sub_ಬಿಟ್_ಅಗಲ = sub_ಬಿಟ್ಟುಗಳು.get_ಬಿಟ್_ಅಗಲ();
  ಪ್ರತಿಜ್ಞಾ((sub_ಬಿಟ್_ಅಗಲ + ಬಿಟ್_Position) <= ಬಿಟ್_ಅಗಲ_2 && "Illegal bit insertion");

  // inserting no bits is a noop.
  ಆದರೆ (sub_ಬಿಟ್_ಅಗಲ == 0)
    ಹಿಂತಿರುಗಿ;

  // Insertion is a direct copy.
  ಆದರೆ (sub_ಬಿಟ್_ಅಗಲ == ಬಿಟ್_ಅಗಲ_2) {
    *ಇದು = sub_ಬಿಟ್ಟುಗಳು;
    ಹಿಂತಿರುಗಿ;
  }

  // Single word result can be done as a direct bitmask.
  ಆದರೆ (LLVM_LIKELY(is_Single_ಶಬ್ದ())) {
    uint64_t ಮುಖವಾಡ_2 = WORDTYPE_MAX >> (APINT_ಬಿಟ್ಟುಗಳು_ಪ್ರತಿ_ಶಬ್ದ - sub_ಬಿಟ್_ಅಗಲ);
    U.VAL &= ~(ಮುಖವಾಡ_2 << ಬಿಟ್_Position);
    U.VAL |= (sub_ಬಿಟ್ಟುಗಳು.U.VAL << ಬಿಟ್_Position);
    ಹಿಂತಿರುಗಿ;
  }

  ಚಿಹ್ನೆರಹಿತ lo_ಬಿಟ್ = which_ಬಿಟ್(ಬಿಟ್_Position);
  ಚಿಹ್ನೆರಹಿತ lo_ಶಬ್ದ = which_ಶಬ್ದ(ಬಿಟ್_Position);
  ಚಿಹ್ನೆರಹಿತ hi1_ಶಬ್ದ = which_ಶಬ್ದ(ಬಿಟ್_Position + sub_ಬಿಟ್_ಅಗಲ - 1);

  // Insertion within a single word can be done as a direct bitmask.
  ಆದರೆ (lo_ಶಬ್ದ == hi1_ಶಬ್ದ) {
    uint64_t ಮುಖವಾಡ_2 = WORDTYPE_MAX >> (APINT_ಬಿಟ್ಟುಗಳು_ಪ್ರತಿ_ಶಬ್ದ - sub_ಬಿಟ್_ಅಗಲ);
    U.pVal[lo_ಶಬ್ದ] &= ~(ಮುಖವಾಡ_2 << lo_ಬಿಟ್);
    U.pVal[lo_ಶಬ್ದ] |= (sub_ಬಿಟ್ಟುಗಳು.U.VAL << lo_ಬಿಟ್);
    ಹಿಂತಿರುಗಿ;
  }

  // Insert on word boundaries.
  ಆದರೆ (lo_ಬಿಟ್ == 0) {
    // Direct copy whole words.
    ಚಿಹ್ನೆರಹಿತ ಅಂಕೆ_ಸಂಪೂರ್ಣ_Sub_ಪದಗಳು = sub_ಬಿಟ್_ಅಗಲ / APINT_ಬಿಟ್ಟುಗಳು_ಪ್ರತಿ_ಶಬ್ದ;
    ಸ್ಮರಣೆ_ನಕಲು(U.pVal + lo_ಶಬ್ದ, sub_ಬಿಟ್ಟುಗಳು.get_Raw_ದತ್ತಾಂಶ(),
           ಅಂಕೆ_ಸಂಪೂರ್ಣ_Sub_ಪದಗಳು * APINT_ಶಬ್ದ_ಗಾತ್ರ);

    // Mask+insert remaining bits.
    ಚಿಹ್ನೆರಹಿತ ಉಳಿದ_ಬಿಟ್ಟುಗಳು = sub_ಬಿಟ್_ಅಗಲ % APINT_ಬಿಟ್ಟುಗಳು_ಪ್ರತಿ_ಶಬ್ದ;
    ಆದರೆ (ಉಳಿದ_ಬಿಟ್ಟುಗಳು != 0) {
      uint64_t ಮುಖವಾಡ_2 = WORDTYPE_MAX >> (APINT_ಬಿಟ್ಟುಗಳು_ಪ್ರತಿ_ಶಬ್ದ - ಉಳಿದ_ಬಿಟ್ಟುಗಳು);
      U.pVal[hi1_ಶಬ್ದ] &= ~ಮುಖವಾಡ_2;
      U.pVal[hi1_ಶಬ್ದ] |= sub_ಬಿಟ್ಟುಗಳು.get_ಶಬ್ದ(sub_ಬಿಟ್_ಅಗಲ - 1);
    }
    ಹಿಂತಿರುಗಿ;
  }

  // General case - set/clear individual bits in dst based on src.
  // TODO - there is scope for optimization here, but at the moment this code
  // path is barely used so prefer readability over performance.
  ಗಾಗಿ (ಚಿಹ್ನೆರಹಿತ i = 0; i != sub_ಬಿಟ್_ಅಗಲ; ++i)
    ಗಣ_ಬಿಟ್_Val(ಬಿಟ್_Position + i, sub_ಬಿಟ್ಟುಗಳು[i]);
}

ಶೂನ್ಯ APInt::ಸೇರಿಸು_ಬಿಟ್ಟುಗಳು(uint64_t sub_ಬಿಟ್ಟುಗಳು, ಚಿಹ್ನೆರಹಿತ ಬಿಟ್_Position, ಚಿಹ್ನೆರಹಿತ ಅಂಕೆ_ಬಿಟ್ಟುಗಳು) {
  uint64_t ಮುಖವಾಡ_ಬಿಟ್ಟುಗಳು = ಮುಖವಾಡ_Trailing_Ones<uint64_t>(ಅಂಕೆ_ಬಿಟ್ಟುಗಳು);
  sub_ಬಿಟ್ಟುಗಳು &= ಮುಖವಾಡ_ಬಿಟ್ಟುಗಳು;
  ಆದರೆ (LLVM_LIKELY(is_Single_ಶಬ್ದ())) {
    U.VAL &= ~(ಮುಖವಾಡ_ಬಿಟ್ಟುಗಳು << ಬಿಟ್_Position);
    U.VAL |= sub_ಬಿಟ್ಟುಗಳು << ಬಿಟ್_Position;
    ಹಿಂತಿರುಗಿ;
  }

  ಚಿಹ್ನೆರಹಿತ lo_ಬಿಟ್ = which_ಬಿಟ್(ಬಿಟ್_Position);
  ಚಿಹ್ನೆರಹಿತ lo_ಶಬ್ದ = which_ಶಬ್ದ(ಬಿಟ್_Position);
  ಚಿಹ್ನೆರಹಿತ hi_ಶಬ್ದ = which_ಶಬ್ದ(ಬಿಟ್_Position + ಅಂಕೆ_ಬಿಟ್ಟುಗಳು - 1);
  ಆದರೆ (lo_ಶಬ್ದ == hi_ಶಬ್ದ) {
    U.pVal[lo_ಶಬ್ದ] &= ~(ಮುಖವಾಡ_ಬಿಟ್ಟುಗಳು << lo_ಬಿಟ್);
    U.pVal[lo_ಶಬ್ದ] |= sub_ಬಿಟ್ಟುಗಳು << lo_ಬಿಟ್;
    ಹಿಂತಿರುಗಿ;
  }

  static_assert(8 * ಗಾತ್ರ(ಶಬ್ದ_Type) <= 64, "This code assumes only two words affected");
  ಚಿಹ್ನೆರಹಿತ ಶಬ್ದ_ಬಿಟ್ಟುಗಳು = 8 * ಗಾತ್ರ(ಶಬ್ದ_Type);
  U.pVal[lo_ಶಬ್ದ] &= ~(ಮುಖವಾಡ_ಬಿಟ್ಟುಗಳು << lo_ಬಿಟ್);
  U.pVal[lo_ಶಬ್ದ] |= sub_ಬಿಟ್ಟುಗಳು << lo_ಬಿಟ್;

  U.pVal[hi_ಶಬ್ದ] &= ~(ಮುಖವಾಡ_ಬಿಟ್ಟುಗಳು >> (ಶಬ್ದ_ಬಿಟ್ಟುಗಳು - lo_ಬಿಟ್));
  U.pVal[hi_ಶಬ್ದ] |= sub_ಬಿಟ್ಟುಗಳು >> (ಶಬ್ದ_ಬಿಟ್ಟುಗಳು - lo_ಬಿಟ್);
}

APInt APInt::ಹೊರತೆಗೆ_ಬಿಟ್ಟುಗಳು(ಚಿಹ್ನೆರಹಿತ ಅಂಕೆ_ಬಿಟ್ಟುಗಳು, ಚಿಹ್ನೆರಹಿತ ಬಿಟ್_Position) ಸ್ಥಿರಾಂಕ {
  ಪ್ರತಿಜ್ಞಾ(ಬಿಟ್_Position < ಬಿಟ್_ಅಗಲ_2 && (ಅಂಕೆ_ಬಿಟ್ಟುಗಳು + ಬಿಟ್_Position) <= ಬಿಟ್_ಅಗಲ_2 &&
         "Illegal bit extraction");

  ಆದರೆ (LLVM_LIKELY(is_Single_ಶಬ್ದ()))
    ಹಿಂತಿರುಗಿ APInt(ಅಂಕೆ_ಬಿಟ್ಟುಗಳು, U.VAL >> ಬಿಟ್_Position, /*isSigned=*/ಸುಳ್ಳು,
                 /*implicitTrunc=*/ನಿಜ);

  ಚಿಹ್ನೆರಹಿತ lo_ಬಿಟ್ = which_ಬಿಟ್(ಬಿಟ್_Position);
  ಚಿಹ್ನೆರಹಿತ lo_ಶಬ್ದ = which_ಶಬ್ದ(ಬಿಟ್_Position);
  ಚಿಹ್ನೆರಹಿತ hi_ಶಬ್ದ = which_ಶಬ್ದ(ಬಿಟ್_Position + ಅಂಕೆ_ಬಿಟ್ಟುಗಳು - 1);

  // Single word result extracting bits from a single word source.
  ಆದರೆ (lo_ಶಬ್ದ == hi_ಶಬ್ದ)
    ಹಿಂತಿರುಗಿ APInt(ಅಂಕೆ_ಬಿಟ್ಟುಗಳು, U.pVal[lo_ಶಬ್ದ] >> lo_ಬಿಟ್, /*isSigned=*/ಸುಳ್ಳು,
                 /*implicitTrunc=*/ನಿಜ);

  // Extracting bits that start on a source word boundary can be done
  // as a fast memory copy.
  ಆದರೆ (lo_ಬಿಟ್ == 0)
    ಹಿಂತಿರುಗಿ APInt(ಅಂಕೆ_ಬಿಟ್ಟುಗಳು, ArrayRef(U.pVal + lo_ಶಬ್ದ, 1 + hi_ಶಬ್ದ - lo_ಶಬ್ದ));

  // General case - shift + copy source words directly into place.
  APInt ಫಲಿತಾಂಶ(ಅಂಕೆ_ಬಿಟ್ಟುಗಳು, 0);
  ಚಿಹ್ನೆರಹಿತ ಅಂಕೆ_Src_ಪದಗಳು = get_ಅಂಕೆ_ಪದಗಳು();
  ಚಿಹ್ನೆರಹಿತ ಅಂಕೆ_Dst_ಪದಗಳು = ಫಲಿತಾಂಶ.get_ಅಂಕೆ_ಪದಗಳು();

  uint64_t *DestPtr =
      LLVM_LIKELY(ಫಲಿತಾಂಶ.is_Single_ಶಬ್ದ()) ? &ಫಲಿತಾಂಶ.U.VAL : ಫಲಿತಾಂಶ.U.pVal;
  ಗಾಗಿ (ಚಿಹ್ನೆರಹಿತ ಶಬ್ದ = 0; ಶಬ್ದ < ಅಂಕೆ_Dst_ಪದಗಳು; ++ಶಬ್ದ) {
    uint64_t w0 = U.pVal[lo_ಶಬ್ದ + ಶಬ್ದ];
    uint64_t w1 =
        (lo_ಶಬ್ದ + ಶಬ್ದ + 1) < ಅಂಕೆ_Src_ಪದಗಳು ? U.pVal[lo_ಶಬ್ದ + ಶಬ್ದ + 1] : 0;
    DestPtr[ಶಬ್ದ] = (w0 >> lo_ಬಿಟ್) | (w1 << (APINT_ಬಿಟ್ಟುಗಳು_ಪ್ರತಿ_ಶಬ್ದ - lo_ಬಿಟ್));
  }

  ಹಿಂತಿರುಗಿ ಫಲಿತಾಂಶ.clear_ಬಳಸದ_ಬಿಟ್ಟುಗಳು();
}

uint64_t APInt::ಹೊರತೆಗೆ_ಬಿಟ್ಟುಗಳು_As_ZExt_ಮೌಲ್ಯ(ಚಿಹ್ನೆರಹಿತ ಅಂಕೆ_ಬಿಟ್ಟುಗಳು,
                                       ಚಿಹ್ನೆರಹಿತ ಬಿಟ್_Position) ಸ್ಥಿರಾಂಕ {
  ಪ್ರತಿಜ್ಞಾ(ಬಿಟ್_Position < ಬಿಟ್_ಅಗಲ_2 && (ಅಂಕೆ_ಬಿಟ್ಟುಗಳು + ಬಿಟ್_Position) <= ಬಿಟ್_ಅಗಲ_2 &&
         "Illegal bit extraction");
  ಪ್ರತಿಜ್ಞಾ(ಅಂಕೆ_ಬಿಟ್ಟುಗಳು <= 64 && "Illegal bit extraction");

  uint64_t ಮುಖವಾಡ_ಬಿಟ್ಟುಗಳು = ಮುಖವಾಡ_Trailing_Ones<uint64_t>(ಅಂಕೆ_ಬಿಟ್ಟುಗಳು);
  ಆದರೆ (LLVM_LIKELY(is_Single_ಶಬ್ದ()))
    ಹಿಂತಿರುಗಿ (U.VAL >> ಬಿಟ್_Position) & ಮುಖವಾಡ_ಬಿಟ್ಟುಗಳು;

  static_assert(APINT_ಬಿಟ್ಟುಗಳು_ಪ್ರತಿ_ಶಬ್ದ >= 64,
                "This code assumes only two words affected");
  ಚಿಹ್ನೆರಹಿತ lo_ಬಿಟ್ = which_ಬಿಟ್(ಬಿಟ್_Position);
  ಚಿಹ್ನೆರಹಿತ lo_ಶಬ್ದ = which_ಶಬ್ದ(ಬಿಟ್_Position);
  ಚಿಹ್ನೆರಹಿತ hi_ಶಬ್ದ = which_ಶಬ್ದ(ಬಿಟ್_Position + ಅಂಕೆ_ಬಿಟ್ಟುಗಳು - 1);
  ಆದರೆ (lo_ಶಬ್ದ == hi_ಶಬ್ದ)
    ಹಿಂತಿರುಗಿ (U.pVal[lo_ಶಬ್ದ] >> lo_ಬಿಟ್) & ಮುಖವಾಡ_ಬಿಟ್ಟುಗಳು;

  uint64_t ret_ಬಿಟ್ಟುಗಳು = U.pVal[lo_ಶಬ್ದ] >> lo_ಬಿಟ್;
  ret_ಬಿಟ್ಟುಗಳು |= U.pVal[hi_ಶಬ್ದ] << (APINT_ಬಿಟ್ಟುಗಳು_ಪ್ರತಿ_ಶಬ್ದ - lo_ಬಿಟ್);
  ret_ಬಿಟ್ಟುಗಳು &= ಮುಖವಾಡ_ಬಿಟ್ಟುಗಳು;
  ಹಿಂತಿರುಗಿ ret_ಬಿಟ್ಟುಗಳು;
}

ಚಿಹ್ನೆರಹಿತ APInt::get_Sufficient_ಬಿಟ್ಟುಗಳು_Needed(StringRef Str, uint8_t Radix) {
  ಪ್ರತಿಜ್ಞಾ(!Str.ಖಾಲಿಗಳು() && "Invalid string length");
  size_t StrLen = Str.ಗಾತ್ರ_3();

  // Each computation below needs to know if it's negative.
  ಚಿಹ್ನೆರಹಿತ IsNegative = ಸುಳ್ಳು;
  ಆದರೆ (Str[0] == '-' || Str[0] == '+') {
    IsNegative = Str[0] == '-';
    StrLen--;
    ಪ್ರತಿಜ್ಞಾ(StrLen && "String is only a sign, needs a value.");
  }

  // For radixes of power-of-two values, the bits required is accurately and
  // easily computed.
  ಆದರೆ (Radix == 2)
    ಹಿಂತಿರುಗಿ StrLen + IsNegative;
  ಆದರೆ (Radix == 8)
    ಹಿಂತಿರುಗಿ StrLen * 3 + IsNegative;
  ಆದರೆ (Radix == 16)
    ಹಿಂತಿರುಗಿ StrLen * 4 + IsNegative;

  // Compute a sufficient number of bits that is always large enough but might
  // be too large. This avoids the assertion in the constructor. This
  // calculation doesn't work appropriately for the numbers 0-9, so just use 4
  // bits in that case.
  ಆದರೆ (Radix == 10)
    ಹಿಂತಿರುಗಿ (StrLen == 1 ? 4 : StrLen * 64 / 18) + IsNegative;

  ಪ್ರತಿಜ್ಞಾ(Radix == 36);
  ಹಿಂತಿರುಗಿ (StrLen == 1 ? 7 : StrLen * 16 / 3) + IsNegative;
}

ಚಿಹ್ನೆರಹಿತ APInt::get_ಬಿಟ್ಟುಗಳು_Needed(StringRef str, uint8_t radix) {
  // Compute a sufficient number of bits that is always large enough but might
  // be too large.
  ಚಿಹ್ನೆರಹಿತ sufficient = get_Sufficient_ಬಿಟ್ಟುಗಳು_Needed(str, radix);

  // For bases 2, 8, and 16, the sufficient number of bits is exact and we can
  // return the value directly. For bases 10 and 36, we need to do extra work.
  ಆದರೆ (radix == 2 || radix == 8 || radix == 16)
    ಹಿಂತಿರುಗಿ sufficient;

  // This is grossly inefficient but accurate. We could probably do something
  // with a computation of roughly slen*64/20 and then adjust by the value of
  // the first few digits. But, I'm not sure how accurate that could be.
  size_t slen = str.ಗಾತ್ರ_3();

  // Each computation below needs to know if it's negative.
  StringRef::iterator p = str.begin();
  ಚಿಹ್ನೆರಹಿತ isNegative = *p == '-';
  ಆದರೆ (*p == '-' || *p == '+') {
    p++;
    slen--;
    ಪ್ರತಿಜ್ಞಾ(slen && "String is only a sign, needs a value.");
  }


  // Convert to the actual binary value.
  APInt tmp(sufficient, StringRef(p, slen), radix);

  // Compute how many bits are required. If the log is infinite, assume we need
  // just bit. If the log is exact and value is negative, then the value is
  // MinSignedValue with (log + 1) bits.
  ಚಿಹ್ನೆರಹಿತ ಲಘುಗಣಕ = tmp.logBase2();
  ಆದರೆ (ಲಘುಗಣಕ == (ಚಿಹ್ನೆರಹಿತ)-1) {
    ಹಿಂತಿರುಗಿ isNegative + 1;
  } ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (isNegative && tmp.isPowerOf2()) {
    ಹಿಂತಿರುಗಿ isNegative + ಲಘುಗಣಕ;
  } ಇಲ್ಲದಿದ್ದರೆ {
    ಹಿಂತಿರುಗಿ isNegative + ಲಘುಗಣಕ + 1;
  }
}

hash_ಸಂಕೇತ llvm::hash_ಮೌಲ್ಯ(ಸ್ಥಿರಾಂಕ APInt &Arg) {
  ಆದರೆ (LLVM_LIKELY(Arg.is_Single_ಶಬ್ದ()))
    ಹಿಂತಿರುಗಿ hash_combine(Arg.ಬಿಟ್_ಅಗಲ_2, Arg.U.VAL);

  ಹಿಂತಿರುಗಿ hash_combine(
      Arg.ಬಿಟ್_ಅಗಲ_2,
      hash_combine_range(Arg.U.pVal, Arg.U.pVal + Arg.get_ಅಂಕೆ_ಪದಗಳು()));
}

ಚಿಹ್ನೆರಹಿತ DenseMapInfo<APInt, ಶೂನ್ಯ>::get_Hash_ಮೌಲ್ಯ(ಸ್ಥಿರಾಂಕ APInt &ಕೀಲಿ) {
  ಹಿಂತಿರುಗಿ static_cast<ಚಿಹ್ನೆರಹಿತ>(hash_ಮೌಲ್ಯ(ಕೀಲಿ));
}

ತಾರ್ಕಿಕ APInt::isSplat(ಚಿಹ್ನೆರಹಿತ Splat_ಗಾತ್ರ_ಒಳ_ಬಿಟ್ಟುಗಳು) ಸ್ಥಿರಾಂಕ {
  ಪ್ರತಿಜ್ಞಾ(get_ಬಿಟ್_ಅಗಲ() % Splat_ಗಾತ್ರ_ಒಳ_ಬಿಟ್ಟುಗಳು == 0 &&
         "SplatSizeInBits must divide width!");
  // We can check that all parts of an integer are equal by making use of a
  // little trick: rotate and check if it's still the same value.
  ಹಿಂತಿರುಗಿ *ಇದು == rotl(Splat_ಗಾತ್ರ_ಒಳ_ಬಿಟ್ಟುಗಳು);
}

/// This function returns the high "numBits" bits of this APInt.
APInt APInt::get_Hi_ಬಿಟ್ಟುಗಳು(ಚಿಹ್ನೆರಹಿತ ಅಂಕೆ_ಬಿಟ್ಟುಗಳು) ಸ್ಥಿರಾಂಕ {
  ಹಿಂತಿರುಗಿ ಇದು->lshr(ಬಿಟ್_ಅಗಲ_2 - ಅಂಕೆ_ಬಿಟ್ಟುಗಳು);
}

/// This function returns the low "numBits" bits of this APInt.
APInt APInt::get_Lo_ಬಿಟ್ಟುಗಳು(ಚಿಹ್ನೆರಹಿತ ಅಂಕೆ_ಬಿಟ್ಟುಗಳು) ಸ್ಥಿರಾಂಕ {
  APInt ಫಲಿತಾಂಶ(get_ಕೆಳಗಿನ_ಬಿಟ್ಟುಗಳು_ಗಣ(ಬಿಟ್_ಅಗಲ_2, ಅಂಕೆ_ಬಿಟ್ಟುಗಳು));
  ಫಲಿತಾಂಶ &= *ಇದು;
  ಹಿಂತಿರುಗಿ ಫಲಿತಾಂಶ;
}

/// Return a value containing V broadcasted over NewLen bits.
APInt APInt::getSplat(ಚಿಹ್ನೆರಹಿತ NewLen, ಸ್ಥಿರಾಂಕ APInt &V) {
  ಪ್ರತಿಜ್ಞಾ(NewLen >= V.get_ಬಿಟ್_ಅಗಲ() && "Can't splat to smaller bit width!");

  APInt Val = V.zext(NewLen);
  ಗಾಗಿ (ಚಿಹ್ನೆರಹಿತ I = V.get_ಬಿಟ್_ಅಗಲ(); I < NewLen; I <<= 1)
    Val |= Val << I;

  ಹಿಂತಿರುಗಿ Val;
}

ಚಿಹ್ನೆರಹಿತ APInt::ಎಣಿಕೆ_Leading_Zeros_Slow_Case() ಸ್ಥಿರಾಂಕ {
  ಚಿಹ್ನೆರಹಿತ ಎಣಿಕೆ_3 = 0;
  ಗಾಗಿ (ಪೂರ್ಣಾಂಕ i = get_ಅಂಕೆ_ಪದಗಳು() - 1; i >= 0; --i) {
    uint64_t V = U.pVal[i];
    ಆದರೆ (V == 0)
      ಎಣಿಕೆ_3 += APINT_ಬಿಟ್ಟುಗಳು_ಪ್ರತಿ_ಶಬ್ದ;
    ಇಲ್ಲದಿದ್ದರೆ {
      ಎಣಿಕೆ_3 += llvm::countl_zero(V);
      ನಿಲ್ಲಿಸು;
    }
  }
  // Adjust for unused bits in the most significant word (they are zero).
  ಚಿಹ್ನೆರಹಿತ Mod = ಬಿಟ್_ಅಗಲ_2 % APINT_ಬಿಟ್ಟುಗಳು_ಪ್ರತಿ_ಶಬ್ದ;
  ಎಣಿಕೆ_3 -= Mod > 0 ? APINT_ಬಿಟ್ಟುಗಳು_ಪ್ರತಿ_ಶಬ್ದ - Mod : 0;
  ಹಿಂತಿರುಗಿ ಎಣಿಕೆ_3;
}

ಚಿಹ್ನೆರಹಿತ APInt::ಎಣಿಕೆ_Leading_Ones_Slow_Case() ಸ್ಥಿರಾಂಕ {
  ಚಿಹ್ನೆರಹಿತ ಮೇಲಿನ_ಶಬ್ದ_ಬಿಟ್ಟುಗಳು = ಬಿಟ್_ಅಗಲ_2 % APINT_ಬಿಟ್ಟುಗಳು_ಪ್ರತಿ_ಶಬ್ದ;
  ಚಿಹ್ನೆರಹಿತ ಸರಿಸು;
  ಆದರೆ (!ಮೇಲಿನ_ಶಬ್ದ_ಬಿಟ್ಟುಗಳು) {
    ಮೇಲಿನ_ಶಬ್ದ_ಬಿಟ್ಟುಗಳು = APINT_ಬಿಟ್ಟುಗಳು_ಪ್ರತಿ_ಶಬ್ದ;
    ಸರಿಸು = 0;
  } ಇಲ್ಲದಿದ್ದರೆ {
    ಸರಿಸು = APINT_ಬಿಟ್ಟುಗಳು_ಪ್ರತಿ_ಶಬ್ದ - ಮೇಲಿನ_ಶಬ್ದ_ಬಿಟ್ಟುಗಳು;
  }
  ಪೂರ್ಣಾಂಕ i = get_ಅಂಕೆ_ಪದಗಳು() - 1;
  ಚಿಹ್ನೆರಹಿತ ಎಣಿಕೆ_3 = llvm::countl_one(U.pVal[i] << ಸರಿಸು);
  ಆದರೆ (ಎಣಿಕೆ_3 == ಮೇಲಿನ_ಶಬ್ದ_ಬಿಟ್ಟುಗಳು) {
    ಗಾಗಿ (i--; i >= 0; --i) {
      ಆದರೆ (U.pVal[i] == WORDTYPE_MAX)
        ಎಣಿಕೆ_3 += APINT_ಬಿಟ್ಟುಗಳು_ಪ್ರತಿ_ಶಬ್ದ;
      ಇಲ್ಲದಿದ್ದರೆ {
        ಎಣಿಕೆ_3 += llvm::countl_one(U.pVal[i]);
        ನಿಲ್ಲಿಸು;
      }
    }
  }
  ಹಿಂತಿರುಗಿ ಎಣಿಕೆ_3;
}

ಚಿಹ್ನೆರಹಿತ APInt::ಎಣಿಕೆ_Trailing_Zeros_Slow_Case() ಸ್ಥಿರಾಂಕ {
  ಚಿಹ್ನೆರಹಿತ ಎಣಿಕೆ_3 = 0;
  ಚಿಹ್ನೆರಹಿತ i = 0;
  ಗಾಗಿ (; i < get_ಅಂಕೆ_ಪದಗಳು() && U.pVal[i] == 0; ++i)
    ಎಣಿಕೆ_3 += APINT_ಬಿಟ್ಟುಗಳು_ಪ್ರತಿ_ಶಬ್ದ;
  ಆದರೆ (i < get_ಅಂಕೆ_ಪದಗಳು())
    ಎಣಿಕೆ_3 += llvm::countr_zero(U.pVal[i]);
  ಹಿಂತಿರುಗಿ std::min(ಎಣಿಕೆ_3, ಬಿಟ್_ಅಗಲ_2);
}

ಚಿಹ್ನೆರಹಿತ APInt::ಎಣಿಕೆ_Trailing_Ones_Slow_Case() ಸ್ಥಿರಾಂಕ {
  ಚಿಹ್ನೆರಹಿತ ಎಣಿಕೆ_3 = 0;
  ಚಿಹ್ನೆರಹಿತ i = 0;
  ಗಾಗಿ (; i < get_ಅಂಕೆ_ಪದಗಳು() && U.pVal[i] == WORDTYPE_MAX; ++i)
    ಎಣಿಕೆ_3 += APINT_ಬಿಟ್ಟುಗಳು_ಪ್ರತಿ_ಶಬ್ದ;
  ಆದರೆ (i < get_ಅಂಕೆ_ಪದಗಳು())
    ಎಣಿಕೆ_3 += llvm::countr_one(U.pVal[i]);
  ಪ್ರತಿಜ್ಞಾ(ಎಣಿಕೆ_3 <= ಬಿಟ್_ಅಗಲ_2);
  ಹಿಂತಿರುಗಿ ಎಣಿಕೆ_3;
}

ಚಿಹ್ನೆರಹಿತ APInt::ಎಣಿಕೆ_Population_Slow_Case() ಸ್ಥಿರಾಂಕ {
  ಚಿಹ್ನೆರಹಿತ ಎಣಿಕೆ_3 = 0;
  ಗಾಗಿ (ಚಿಹ್ನೆರಹಿತ i = 0; i < get_ಅಂಕೆ_ಪದಗಳು(); ++i)
    ಎಣಿಕೆ_3 += llvm::popcount(U.pVal[i]);
  ಹಿಂತಿರುಗಿ ಎಣಿಕೆ_3;
}

ತಾರ್ಕಿಕ APInt::isPowerOf2SlowCase() ಸ್ಥಿರಾಂಕ {
  ಚಿಹ್ನೆರಹಿತ ಎಣಿಕೆ_3 = 0;
  ಗಾಗಿ (ಚಿಹ್ನೆರಹಿತ i = 0; i < get_ಅಂಕೆ_ಪದಗಳು(); ++i) {
    ಎಣಿಕೆ_3 += llvm::popcount(U.pVal[i]);
    ಆದರೆ (ಎಣಿಕೆ_3 > 1)
      ಹಿಂತಿರುಗಿ ಸುಳ್ಳು;
  }
  ಹಿಂತಿರುಗಿ ಎಣಿಕೆ_3 == 1;
}

ತಾರ್ಕಿಕ APInt::intersectsSlowCase(ಸ್ಥಿರಾಂಕ APInt &RHS) ಸ್ಥಿರಾಂಕ {
  ಗಾಗಿ (ಚಿಹ್ನೆರಹಿತ i = 0, e = get_ಅಂಕೆ_ಪದಗಳು(); i != e; ++i)
    ಆದರೆ ((U.pVal[i] & RHS.U.pVal[i]) != 0)
      ಹಿಂತಿರುಗಿ ನಿಜ;

  ಹಿಂತಿರುಗಿ ಸುಳ್ಳು;
}

ತಾರ್ಕಿಕ APInt::is_Subset_ದ_Slow_Case(ಸ್ಥಿರಾಂಕ APInt &RHS) ಸ್ಥಿರಾಂಕ {
  ಗಾಗಿ (ಚಿಹ್ನೆರಹಿತ i = 0, e = get_ಅಂಕೆ_ಪದಗಳು(); i != e; ++i)
    ಆದರೆ ((U.pVal[i] & ~RHS.U.pVal[i]) != 0)
      ಹಿಂತಿರುಗಿ ಸುಳ್ಳು;

  ಹಿಂತಿರುಗಿ ನಿಜ;
}

ತಾರ್ಕಿಕ APInt::is_Inverse_ದ_Slow_Case(ಸ್ಥಿರಾಂಕ APInt &RHS) ಸ್ಥಿರಾಂಕ {
  ಸ್ಥಿರಾಂಕ ಚಿಹ್ನೆರಹಿತ ಕೊನೆಯ = get_ಅಂಕೆ_ಪದಗಳು() - 1;
  ಗಾಗಿ (ಚಿಹ್ನೆರಹಿತ I = 0; I != ಕೊನೆಯ; ++I)
    ಆದರೆ ((U.pVal[I] ^ RHS.U.pVal[I]) != WORDTYPE_MAX)
      ಹಿಂತಿರುಗಿ ಸುಳ್ಳು;

  ಚಿಹ್ನೆರಹಿತ ಬಾಲ_ಬಿಟ್ಟುಗಳು = ಬಿಟ್_ಅಗಲ_2 - ಕೊನೆಯ * APINT_ಬಿಟ್ಟುಗಳು_ಪ್ರತಿ_ಶಬ್ದ;
  ಶಬ್ದ_Type ಬಾಲ_ಮುಖವಾಡ = llvm::ಮುಖವಾಡ_Trailing_Ones<ಶಬ್ದ_Type>(ಬಾಲ_ಬಿಟ್ಟುಗಳು);
  ಹಿಂತಿರುಗಿ (U.pVal[ಕೊನೆಯ] ^ RHS.U.pVal[ಕೊನೆಯ]) == ಬಾಲ_ಮುಖವಾಡ;
}

APInt APInt::byte_ಅದಲುಬದಲು() ಸ್ಥಿರಾಂಕ {
  ಪ್ರತಿಜ್ಞಾ(ಬಿಟ್_ಅಗಲ_2 >= 16 && ಬಿಟ್_ಅಗಲ_2 % 8 == 0 && "Cannot byteswap!");
  ಆದರೆ (ಬಿಟ್_ಅಗಲ_2 == 16)
    ಹಿಂತಿರುಗಿ APInt(ಬಿಟ್_ಅಗಲ_2, llvm::byteswap<uint16_t>(U.VAL));
  ಆದರೆ (ಬಿಟ್_ಅಗಲ_2 == 32)
    ಹಿಂತಿರುಗಿ APInt(ಬಿಟ್_ಅಗಲ_2, llvm::byteswap<uint32_t>(U.VAL));
  ಆದರೆ (ಬಿಟ್_ಅಗಲ_2 <= 64) {
    uint64_t Tmp1 = llvm::byteswap<uint64_t>(U.VAL);
    Tmp1 >>= (64 - ಬಿಟ್_ಅಗಲ_2);
    ಹಿಂತಿರುಗಿ APInt(ಬಿಟ್_ಅಗಲ_2, Tmp1);
  }

  APInt ಫಲಿತಾಂಶ(get_ಅಂಕೆ_ಪದಗಳು() * APINT_ಬಿಟ್ಟುಗಳು_ಪ್ರತಿ_ಶಬ್ದ, 0);
  ಗಾಗಿ (ಚಿಹ್ನೆರಹಿತ I = 0, N = get_ಅಂಕೆ_ಪದಗಳು(); I != N; ++I)
    ಫಲಿತಾಂಶ.U.pVal[I] = llvm::byteswap<uint64_t>(U.pVal[N - I - 1]);
  ಆದರೆ (ಫಲಿತಾಂಶ.ಬಿಟ್_ಅಗಲ_2 != ಬಿಟ್_ಅಗಲ_2) {
    ಫಲಿತಾಂಶ.lshr_ಒಳ_ಇರಿಸು(ಫಲಿತಾಂಶ.ಬಿಟ್_ಅಗಲ_2 - ಬಿಟ್_ಅಗಲ_2);
    ಫಲಿತಾಂಶ.ಬಿಟ್_ಅಗಲ_2 = ಬಿಟ್_ಅಗಲ_2;
  }
  ಹಿಂತಿರುಗಿ ಫಲಿತಾಂಶ;
}

APInt APInt::ಹಿಮ್ಮುಖ_ಬಿಟ್ಟುಗಳು() ಸ್ಥಿರಾಂಕ {
  ಆದರೆ (LLVM_LIKELY(is_Single_ಶಬ್ದ())) {
    ಆಯ್ಕೆ (ಬಿಟ್_ಅಗಲ_2) {
    ಸ್ಥಿತಿ 64:
      ಹಿಂತಿರುಗಿ APInt(ಬಿಟ್_ಅಗಲ_2, llvm::ಹಿಮ್ಮುಖ_ಬಿಟ್ಟುಗಳು<uint64_t>(U.VAL));
    ಸ್ಥಿತಿ 32:
      ಹಿಂತಿರುಗಿ APInt(ಬಿಟ್_ಅಗಲ_2, llvm::ಹಿಮ್ಮುಖ_ಬಿಟ್ಟುಗಳು<uint32_t>(U.VAL));
    ಸ್ಥಿತಿ 16:
      ಹಿಂತಿರುಗಿ APInt(ಬಿಟ್_ಅಗಲ_2, llvm::ಹಿಮ್ಮುಖ_ಬಿಟ್ಟುಗಳು<uint16_t>(U.VAL));
    ಸ್ಥಿತಿ 8:
      ಹಿಂತಿರುಗಿ APInt(ಬಿಟ್_ಅಗಲ_2, llvm::ಹಿಮ್ಮುಖ_ಬಿಟ್ಟುಗಳು<uint8_t>(U.VAL));
    ಸ್ಥಿತಿ 1: // fallthrough
    ಸ್ಥಿತಿ 0:
      ಹಿಂತಿರುಗಿ *ಇದು;
    ಪೂರ್ವನಿಯೋಜಿತ:
      ಹಿಂತಿರುಗಿ APInt(ಬಿಟ್_ಅಗಲ_2,
                   llvm::ಹಿಮ್ಮುಖ_ಬಿಟ್ಟುಗಳು<uint64_t>(U.VAL) >> (64 - ಬಿಟ್_ಅಗಲ_2));
    }
  }

  APInt ಫಲಿತಾಂಶ(ಬಿಟ್_ಅಗಲ_2, 0);
  ಚಿಹ್ನೆರಹಿತ ಅಂಕೆ_ಪದಗಳು_2 = get_ಅಂಕೆ_ಪದಗಳು();
  ಚಿಹ್ನೆರಹಿತ Excess_ಬಿಟ್ಟುಗಳು = ಅಂಕೆ_ಪದಗಳು_2 * APINT_ಬಿಟ್ಟುಗಳು_ಪ್ರತಿ_ಶಬ್ದ - ಬಿಟ್_ಅಗಲ_2;
  ಆದರೆ (Excess_ಬಿಟ್ಟುಗಳು == 0) {
    // Fast path. No cross-word shift needed.
    ಗಾಗಿ (ಚಿಹ್ನೆರಹಿತ I = 0; I < ಅಂಕೆ_ಪದಗಳು_2; ++I)
      ಫಲಿತಾಂಶ.U.pVal[I] = llvm::ಹಿಮ್ಮುಖ_ಬಿಟ್ಟುಗಳು<uint64_t>(U.pVal[ಅಂಕೆ_ಪದಗಳು_2 - 1 - I]);
    ಹಿಂತಿರುಗಿ ಫಲಿತಾಂಶ;
  }
  // Holds reversed bits of the previous (more significant) word.
  uint64_t PrevRev = llvm::ಹಿಮ್ಮುಖ_ಬಿಟ್ಟುಗಳು<uint64_t>(U.pVal[ಅಂಕೆ_ಪದಗಳು_2 - 1]);
  ಗಾಗಿ (ಚಿಹ್ನೆರಹಿತ I = 0; I < ಅಂಕೆ_ಪದಗಳು_2 - 1; ++I) {
    uint64_t CurrRev = llvm::ಹಿಮ್ಮುಖ_ಬಿಟ್ಟುಗಳು<uint64_t>(U.pVal[ಅಂಕೆ_ಪದಗಳು_2 - 2 - I]);
    ಫಲಿತಾಂಶ.U.pVal[I] = (PrevRev >> Excess_ಬಿಟ್ಟುಗಳು) | (CurrRev << (64 - Excess_ಬಿಟ್ಟುಗಳು));
    PrevRev = CurrRev;
  }
  ಫಲಿತಾಂಶ.U.pVal[ಅಂಕೆ_ಪದಗಳು_2 - 1] = PrevRev >> Excess_ಬಿಟ್ಟುಗಳು;
  ಹಿಂತಿರುಗಿ ಫಲಿತಾಂಶ;
}

APInt llvm::APIntOps::Greatest_Common_ಭಾಜಕ(APInt A, APInt B, ತಾರ್ಕಿಕ IsSigned) {
  // Take absolute value if IsSigned.
  ಆದರೆ (IsSigned) {
    A = A.ನಿರಪೇಕ್ಷ();
    B = B.ನಿರಪೇಕ್ಷ();
  }

  // Fast-path a common case.
  ಆದರೆ (A == B) ಹಿಂತಿರುಗಿ A;

  // Corner cases: if either operand is zero, the other is the gcd.
  ಆದರೆ (!A) ಹಿಂತಿರುಗಿ B;
  ಆದರೆ (!B) ಹಿಂತಿರುಗಿ A;

  // Count common powers of 2 and remove all other powers of 2.
  ಚಿಹ್ನೆರಹಿತ Pow2;
  {
    ಚಿಹ್ನೆರಹಿತ Pow2_A = A.countr_zero();
    ಚಿಹ್ನೆರಹಿತ Pow2_B = B.countr_zero();
    ಆದರೆ (Pow2_A > Pow2_B) {
      A.lshr_ಒಳ_ಇರಿಸು(Pow2_A - Pow2_B);
      Pow2 = Pow2_B;
    } ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (Pow2_B > Pow2_A) {
      B.lshr_ಒಳ_ಇರಿಸು(Pow2_B - Pow2_A);
      Pow2 = Pow2_A;
    } ಇಲ್ಲದಿದ್ದರೆ {
      Pow2 = Pow2_A;
    }
  }

  // Both operands are odd multiples of 2^Pow_2:
  //
  //   gcd(a, b) = gcd(|a - b| / 2^i, min(a, b))
  //
  // This is a modified version of Stein's algorithm, taking advantage of
  // efficient countTrailingZeros().
  ಆಗ (A != B) {
    ಆದರೆ (A.ugt(B)) {
      A -= B;
      A.lshr_ಒಳ_ಇರಿಸು(A.countr_zero() - Pow2);
    } ಇಲ್ಲದಿದ್ದರೆ {
      B -= A;
      B.lshr_ಒಳ_ಇರಿಸು(B.countr_zero() - Pow2);
    }
  }

  ಹಿಂತಿರುಗಿ A;
}

APInt llvm::APIntOps::ಸುತ್ತು_Double_ಗಮ್ಯ_APInt(ದ್ವಿಗುಣ Double, ಚಿಹ್ನೆರಹಿತ ಅಗಲ_2) {
  uint64_t I = ಬಿಟ್_cast<uint64_t>(Double);

  // Get the sign bit from the highest order bit
  ತಾರ್ಕಿಕ isNeg = I >> 63;

  // Get the 11-bit exponent and adjust for the 1023 bit bias
  int64_t ಘಾತಾಂಕ = ((I >> 52) & 0x7ff) - 1023;

  // If the exponent is negative, the value is < 0 so just return 0.
  ಆದರೆ (ಘಾತಾಂಕ < 0)
    ಹಿಂತಿರುಗಿ APInt(ಅಗಲ_2, 0u);

  // Extract the mantissa by clearing the top 12 bits (sign + exponent).
  uint64_t mantissa = (I & (~0ULL >> 12)) | 1ULL << 52;

  // If the exponent doesn't shift all bits out of the mantissa
  ಆದರೆ (ಘಾತಾಂಕ < 52)
    ಹಿಂತಿರುಗಿ isNeg ? -APInt(ಅಗಲ_2, mantissa >> (52 - ಘಾತಾಂಕ)) :
                    APInt(ಅಗಲ_2, mantissa >> (52 - ಘಾತಾಂಕ));

  // If the client didn't provide enough bits for us to shift the mantissa into
  // then the result is undefined, just return 0
  ಆದರೆ (ಅಗಲ_2 <= ಘಾತಾಂಕ - 52)
    ಹಿಂತಿರುಗಿ APInt(ಅಗಲ_2, 0);

  // Otherwise, we have to shift the mantissa bits up to the right location
  APInt Tmp(ಅಗಲ_2, mantissa);
  Tmp <<= (ಚಿಹ್ನೆರಹಿತ)ಘಾತಾಂಕ - 52;
  ಹಿಂತಿರುಗಿ isNeg ? -Tmp : Tmp;
}

/// This function converts this APInt to a double.
/// The layout for double is as following (IEEE Standard 754):
///  --------------------------------------
/// |  Sign    Exponent    Fraction    Bias |
/// |-------------------------------------- |
/// |  1[63]   11[62-52]   52[51-00]   1023 |
///  --------------------------------------
ದ್ವಿಗುಣ APInt::ಸುತ್ತು_ಗಮ್ಯ_Double(ತಾರ್ಕಿಕ isSigned) ಸ್ಥಿರಾಂಕ {
  // Handle the simple case where the value is contained in one uint64_t.
  // It is wrong to optimize getWord(0) to VAL; there might be more than one word.
  ಆದರೆ (LLVM_LIKELY(is_Single_ಶಬ್ದ() || get_ಸಕ್ರಿಯ_ಬಿಟ್ಟುಗಳು() <= APINT_ಬಿಟ್ಟುಗಳು_ಪ್ರತಿ_ಶಬ್ದ)) {
    ಆದರೆ (isSigned) {
      int64_t sext = SignExtend64(get_ಶಬ್ದ(0), ಬಿಟ್_ಅಗಲ_2);
      ಹಿಂತಿರುಗಿ ದ್ವಿಗುಣ(sext);
    }
    ಹಿಂತಿರುಗಿ ದ್ವಿಗುಣ(get_ಶಬ್ದ(0));
  }

  // Determine if the value is negative.
  ತಾರ್ಕಿಕ isNeg = isSigned ? (*ಇದು)[ಬಿಟ್_ಅಗಲ_2-1] : ಸುಳ್ಳು;

  // Construct the absolute value if we're negative.
  APInt Tmp(isNeg ? -(*ಇದು) : (*ಇದು));

  // Figure out how many bits we're using.
  ಚಿಹ್ನೆರಹಿತ n = Tmp.get_ಸಕ್ರಿಯ_ಬಿಟ್ಟುಗಳು();

  // The exponent (without bias normalization) is just the number of bits
  // we are using. Note that the sign bit is gone since we constructed the
  // absolute value.
  uint64_t ಘಾತಾಂಕ = n;

  // Return infinity for exponent overflow
  ಆದರೆ (ಘಾತಾಂಕ > 1023) {
    ಆದರೆ (!isSigned || !isNeg)
      ಹಿಂತಿರುಗಿ std::numeric_limits<ದ್ವಿಗುಣ>::infinity();
    ಇಲ್ಲದಿದ್ದರೆ
      ಹಿಂತಿರುಗಿ -std::numeric_limits<ದ್ವಿಗುಣ>::infinity();
  }
  ಘಾತಾಂಕ += 1023; // Increment for 1023 bias

  // Number of bits in mantissa is 52. To obtain the mantissa value, we must
  // extract the high 52 bits from the correct words in pVal.
  uint64_t mantissa;
  ಚಿಹ್ನೆರಹಿತ hi_ಶಬ್ದ = which_ಶಬ್ದ(n-1);
  ಆದರೆ (hi_ಶಬ್ದ == 0) {
    mantissa = Tmp.U.pVal[0];
    ಆದರೆ (n > 52)
      mantissa >>= n - 52; // shift down, we want the top 52 bits.
  } ಇಲ್ಲದಿದ್ದರೆ {
    ಪ್ರತಿಜ್ಞಾ(hi_ಶಬ್ದ > 0 && "huh?");
    uint64_t hibits = Tmp.U.pVal[hi_ಶಬ್ದ] << (52 - n % APINT_ಬಿಟ್ಟುಗಳು_ಪ್ರತಿ_ಶಬ್ದ);
    uint64_t lobits = Tmp.U.pVal[hi_ಶಬ್ದ-1] >> (11 + n % APINT_ಬಿಟ್ಟುಗಳು_ಪ್ರತಿ_ಶಬ್ದ);
    mantissa = hibits | lobits;
  }

  // The leading bit of mantissa is implicit, so get rid of it.
  uint64_t sign = isNeg ? (1ULL << (APINT_ಬಿಟ್ಟುಗಳು_ಪ್ರತಿ_ಶಬ್ದ - 1)) : 0;
  uint64_t I = sign | (ಘಾತಾಂಕ << 52) | mantissa;
  ಹಿಂತಿರುಗಿ ಬಿಟ್_cast<ದ್ವಿಗುಣ>(I);
}

// Truncate to new width.
APInt APInt::trunc(ಚಿಹ್ನೆರಹಿತ ಅಗಲ_2) ಸ್ಥಿರಾಂಕ {
  ಪ್ರತಿಜ್ಞಾ(ಅಗಲ_2 <= ಬಿಟ್_ಅಗಲ_2 && "Invalid APInt Truncate request");

  ಆದರೆ (ಅಗಲ_2 <= APINT_ಬಿಟ್ಟುಗಳು_ಪ್ರತಿ_ಶಬ್ದ)
    ಹಿಂತಿರುಗಿ APInt(ಅಗಲ_2, get_Raw_ದತ್ತಾಂಶ()[0], /*isSigned=*/ಸುಳ್ಳು,
                 /*implicitTrunc=*/ನಿಜ);

  ಆದರೆ (ಅಗಲ_2 == ಬಿಟ್_ಅಗಲ_2)
    ಹಿಂತಿರುಗಿ *ಇದು;

  APInt ಫಲಿತಾಂಶ(get_ಸ್ಮರಣೆ(get_ಅಂಕೆ_ಪದಗಳು(ಅಗಲ_2)), ಅಗಲ_2);

  // Copy full words.
  ಚಿಹ್ನೆರಹಿತ i;
  ಗಾಗಿ (i = 0; i != ಅಗಲ_2 / APINT_ಬಿಟ್ಟುಗಳು_ಪ್ರತಿ_ಶಬ್ದ; i++)
    ಫಲಿತಾಂಶ.U.pVal[i] = U.pVal[i];

  // Truncate and copy any partial word.
  ಚಿಹ್ನೆರಹಿತ ಬಿಟ್ಟುಗಳು = (0 - ಅಗಲ_2) % APINT_ಬಿಟ್ಟುಗಳು_ಪ್ರತಿ_ಶಬ್ದ;
  ಆದರೆ (ಬಿಟ್ಟುಗಳು != 0)
    ಫಲಿತಾಂಶ.U.pVal[i] = U.pVal[i] << ಬಿಟ್ಟುಗಳು >> ಬಿಟ್ಟುಗಳು;

  ಹಿಂತಿರುಗಿ ಫಲಿತಾಂಶ;
}

// Truncate to new width with unsigned saturation.
APInt APInt::truncUSat(ಚಿಹ್ನೆರಹಿತ ಅಗಲ_2) ಸ್ಥಿರಾಂಕ {
  ಪ್ರತಿಜ್ಞಾ(ಅಗಲ_2 <= ಬಿಟ್_ಅಗಲ_2 && "Invalid APInt Truncate request");

  // Can we just losslessly truncate it?
  ಆದರೆ (isIntN(ಅಗಲ_2))
    ಹಿಂತಿರುಗಿ trunc(ಅಗಲ_2);
  // If not, then just return the new limit.
  ಹಿಂತಿರುಗಿ APInt::get_Max_ಮೌಲ್ಯ(ಅಗಲ_2);
}

// Truncate to new width with signed saturation to signed result.
APInt APInt::truncSSat(ಚಿಹ್ನೆರಹಿತ ಅಗಲ_2) ಸ್ಥಿರಾಂಕ {
  ಪ್ರತಿಜ್ಞಾ(ಅಗಲ_2 <= ಬಿಟ್_ಅಗಲ_2 && "Invalid APInt Truncate request");

  // Can we just losslessly truncate it?
  ಆದರೆ (isSignedIntN(ಅಗಲ_2))
    ಹಿಂತಿರುಗಿ trunc(ಅಗಲ_2);
  // If not, then just return the new limits.
  ಹಿಂತಿರುಗಿ isNegative() ? APInt::get_Signed_Min_ಮೌಲ್ಯ(ಅಗಲ_2)
                      : APInt::get_Signed_Max_ಮೌಲ್ಯ(ಅಗಲ_2);
}

// Truncate to new width with signed saturation to unsigned result.
APInt APInt::truncSSatU(ಚಿಹ್ನೆರಹಿತ ಅಗಲ_2) ಸ್ಥಿರಾಂಕ {
  ಪ್ರತಿಜ್ಞಾ(ಅಗಲ_2 <= ಬಿಟ್_ಅಗಲ_2 && "Invalid APInt Truncate request");

  // Can we just losslessly truncate it?
  ಆದರೆ (isIntN(ಅಗಲ_2))
    ಹಿಂತಿರುಗಿ trunc(ಅಗಲ_2);
  // If not, then just return the new limits.
  ಹಿಂತಿರುಗಿ isNegative() ? APInt::getZero(ಅಗಲ_2) : APInt::get_Max_ಮೌಲ್ಯ(ಅಗಲ_2);
}

// Sign extend to a new width.
APInt APInt::sext(ಚಿಹ್ನೆರಹಿತ ಅಗಲ) ಸ್ಥಿರಾಂಕ {
  ಪ್ರತಿಜ್ಞಾ(ಅಗಲ >= ಬಿಟ್_ಅಗಲ_2 && "Invalid APInt SignExtend request");

  ಆದರೆ (ಅಗಲ <= APINT_ಬಿಟ್ಟುಗಳು_ಪ್ರತಿ_ಶಬ್ದ)
    ಹಿಂತಿರುಗಿ APInt(ಅಗಲ, SignExtend64(U.VAL, ಬಿಟ್_ಅಗಲ_2), /*isSigned=*/ನಿಜ);

  ಆದರೆ (ಅಗಲ == ಬಿಟ್_ಅಗಲ_2)
    ಹಿಂತಿರುಗಿ *ಇದು;

  APInt ಫಲಿತಾಂಶ(get_ಸ್ಮರಣೆ(get_ಅಂಕೆ_ಪದಗಳು(ಅಗಲ)), ಅಗಲ);

  // Copy words.
  std::ಸ್ಮರಣೆ_ನಕಲು(ಫಲಿತಾಂಶ.U.pVal, get_Raw_ದತ್ತಾಂಶ(), get_ಅಂಕೆ_ಪದಗಳು() * APINT_ಶಬ್ದ_ಗಾತ್ರ);

  // Sign extend the last word since there may be unused bits in the input.
  ಫಲಿತಾಂಶ.U.pVal[get_ಅಂಕೆ_ಪದಗಳು() - 1] =
      SignExtend64(ಫಲಿತಾಂಶ.U.pVal[get_ಅಂಕೆ_ಪದಗಳು() - 1],
                   ((ಬಿಟ್_ಅಗಲ_2 - 1) % APINT_ಬಿಟ್ಟುಗಳು_ಪ್ರತಿ_ಶಬ್ದ) + 1);

  // Fill with sign bits.
  std::memset(ಫಲಿತಾಂಶ.U.pVal + get_ಅಂಕೆ_ಪದಗಳು(), isNegative() ? -1 : 0,
              (ಫಲಿತಾಂಶ.get_ಅಂಕೆ_ಪದಗಳು() - get_ಅಂಕೆ_ಪದಗಳು()) * APINT_ಶಬ್ದ_ಗಾತ್ರ);
  ಫಲಿತಾಂಶ.clear_ಬಳಸದ_ಬಿಟ್ಟುಗಳು();
  ಹಿಂತಿರುಗಿ ಫಲಿತಾಂಶ;
}

//  Zero extend to a new width.
APInt APInt::zext(ಚಿಹ್ನೆರಹಿತ ಅಗಲ_2) ಸ್ಥಿರಾಂಕ {
  ಪ್ರತಿಜ್ಞಾ(ಅಗಲ_2 >= ಬಿಟ್_ಅಗಲ_2 && "Invalid APInt ZeroExtend request");

  ಆದರೆ (ಅಗಲ_2 <= APINT_ಬಿಟ್ಟುಗಳು_ಪ್ರತಿ_ಶಬ್ದ)
    ಹಿಂತಿರುಗಿ APInt(ಅಗಲ_2, U.VAL);

  ಆದರೆ (ಅಗಲ_2 == ಬಿಟ್_ಅಗಲ_2)
    ಹಿಂತಿರುಗಿ *ಇದು;

  APInt ಫಲಿತಾಂಶ(get_ಸ್ಮರಣೆ(get_ಅಂಕೆ_ಪದಗಳು(ಅಗಲ_2)), ಅಗಲ_2);

  // Copy words.
  std::ಸ್ಮರಣೆ_ನಕಲು(ಫಲಿತಾಂಶ.U.pVal, get_Raw_ದತ್ತಾಂಶ(), get_ಅಂಕೆ_ಪದಗಳು() * APINT_ಶಬ್ದ_ಗಾತ್ರ);

  // Zero remaining words.
  std::memset(ಫಲಿತಾಂಶ.U.pVal + get_ಅಂಕೆ_ಪದಗಳು(), 0,
              (ಫಲಿತಾಂಶ.get_ಅಂಕೆ_ಪದಗಳು() - get_ಅಂಕೆ_ಪದಗಳು()) * APINT_ಶಬ್ದ_ಗಾತ್ರ);

  ಹಿಂತಿರುಗಿ ಫಲಿತಾಂಶ;
}

APInt APInt::zextOrTrunc(ಚಿಹ್ನೆರಹಿತ ಅಗಲ_2) ಸ್ಥಿರಾಂಕ {
  ಆದರೆ (ಬಿಟ್_ಅಗಲ_2 < ಅಗಲ_2)
    ಹಿಂತಿರುಗಿ zext(ಅಗಲ_2);
  ಆದರೆ (ಬಿಟ್_ಅಗಲ_2 > ಅಗಲ_2)
    ಹಿಂತಿರುಗಿ trunc(ಅಗಲ_2);
  ಹಿಂತಿರುಗಿ *ಇದು;
}

APInt APInt::sextOrTrunc(ಚಿಹ್ನೆರಹಿತ ಅಗಲ_2) ಸ್ಥಿರಾಂಕ {
  ಆದರೆ (ಬಿಟ್_ಅಗಲ_2 < ಅಗಲ_2)
    ಹಿಂತಿರುಗಿ sext(ಅಗಲ_2);
  ಆದರೆ (ಬಿಟ್_ಅಗಲ_2 > ಅಗಲ_2)
    ಹಿಂತಿರುಗಿ trunc(ಅಗಲ_2);
  ಹಿಂತಿರುಗಿ *ಇದು;
}

/// Arithmetic right-shift this APInt by shiftAmt.
/// Arithmetic right-shift function.
ಶೂನ್ಯ APInt::ashr_ಒಳ_ಇರಿಸು(ಸ್ಥಿರಾಂಕ APInt &ಸರಿಸು_Amt) {
  ashr_ಒಳ_ಇರಿಸು((ಚಿಹ್ನೆರಹಿತ)ಸರಿಸು_Amt.get_Limited_ಮೌಲ್ಯ(ಬಿಟ್_ಅಗಲ_2));
}

/// Arithmetic right-shift this APInt by shiftAmt.
/// Arithmetic right-shift function.
ಶೂನ್ಯ APInt::ashrSlowCase(ಚಿಹ್ನೆರಹಿತ ಸರಿಸು_Amt_2) {
  // Don't bother performing a no-op shift.
  ಆದರೆ (!ಸರಿಸು_Amt_2)
    ಹಿಂತಿರುಗಿ;

  // Save the original sign bit for later.
  ತಾರ್ಕಿಕ Negative = isNegative();

  // WordShift is the inter-part shift; BitShift is intra-part shift.
  ಚಿಹ್ನೆರಹಿತ ಶಬ್ದ_ಸರಿಸು = ಸರಿಸು_Amt_2 / APINT_ಬಿಟ್ಟುಗಳು_ಪ್ರತಿ_ಶಬ್ದ;
  ಚಿಹ್ನೆರಹಿತ ಬಿಟ್_ಸರಿಸು = ಸರಿಸು_Amt_2 % APINT_ಬಿಟ್ಟುಗಳು_ಪ್ರತಿ_ಶಬ್ದ;

  ಚಿಹ್ನೆರಹಿತ ಪದಗಳು_ಗಮ್ಯ_Move = get_ಅಂಕೆ_ಪದಗಳು() - ಶಬ್ದ_ಸರಿಸು;
  ಆದರೆ (ಪದಗಳು_ಗಮ್ಯ_Move != 0) {
    // Sign extend the last word to fill in the unused bits.
    U.pVal[get_ಅಂಕೆ_ಪದಗಳು() - 1] = SignExtend64(
        U.pVal[get_ಅಂಕೆ_ಪದಗಳು() - 1], ((ಬಿಟ್_ಅಗಲ_2 - 1) % APINT_ಬಿಟ್ಟುಗಳು_ಪ್ರತಿ_ಶಬ್ದ) + 1);

    // Fastpath for moving by whole words.
    ಆದರೆ (ಬಿಟ್_ಸರಿಸು == 0) {
      std::memmove(U.pVal, U.pVal + ಶಬ್ದ_ಸರಿಸು, ಪದಗಳು_ಗಮ್ಯ_Move * APINT_ಶಬ್ದ_ಗಾತ್ರ);
    } ಇಲ್ಲದಿದ್ದರೆ {
      // Move the words containing significant bits.
      ಗಾಗಿ (ಚಿಹ್ನೆರಹಿತ i = 0; i != ಪದಗಳು_ಗಮ್ಯ_Move - 1; ++i)
        U.pVal[i] = (U.pVal[i + ಶಬ್ದ_ಸರಿಸು] >> ಬಿಟ್_ಸರಿಸು) |
                    (U.pVal[i + ಶಬ್ದ_ಸರಿಸು + 1] << (APINT_ಬಿಟ್ಟುಗಳು_ಪ್ರತಿ_ಶಬ್ದ - ಬಿಟ್_ಸರಿಸು));

      // Handle the last word which has no high bits to copy. Use an arithmetic
      // shift to preserve the sign bit.
      U.pVal[ಪದಗಳು_ಗಮ್ಯ_Move - 1] =
          (int64_t)U.pVal[ಶಬ್ದ_ಸರಿಸು + ಪದಗಳು_ಗಮ್ಯ_Move - 1] >> ಬಿಟ್_ಸರಿಸು;
    }
  }

  // Fill in the remainder based on the original sign.
  std::memset(U.pVal + ಪದಗಳು_ಗಮ್ಯ_Move, Negative ? -1 : 0,
              ಶಬ್ದ_ಸರಿಸು * APINT_ಶಬ್ದ_ಗಾತ್ರ);
  clear_ಬಳಸದ_ಬಿಟ್ಟುಗಳು();
}

/// Logical right-shift this APInt by shiftAmt.
/// Logical right-shift function.
ಶೂನ್ಯ APInt::lshr_ಒಳ_ಇರಿಸು(ಸ್ಥಿರಾಂಕ APInt &ಸರಿಸು_Amt) {
  lshr_ಒಳ_ಇರಿಸು((ಚಿಹ್ನೆರಹಿತ)ಸರಿಸು_Amt.get_Limited_ಮೌಲ್ಯ(ಬಿಟ್_ಅಗಲ_2));
}

/// Logical right-shift this APInt by shiftAmt.
/// Logical right-shift function.
ಶೂನ್ಯ APInt::lshrSlowCase(ಚಿಹ್ನೆರಹಿತ ಸರಿಸು_Amt_2) {
  tc_ಸರಿಸು_ಬಲ(U.pVal, get_ಅಂಕೆ_ಪದಗಳು(), ಸರಿಸು_Amt_2);
}

/// Left-shift this APInt by shiftAmt.
/// Left-shift function.
APInt &APInt::ಕ್ರಿಯಾಕಾರಕ<<=(ಸ್ಥಿರಾಂಕ APInt &ಸರಿಸು_Amt) {
  // It's undefined behavior in C to shift by BitWidth or greater.
  *ಇದು <<= (ಚಿಹ್ನೆರಹಿತ)ಸರಿಸು_Amt.get_Limited_ಮೌಲ್ಯ(ಬಿಟ್_ಅಗಲ_2);
  ಹಿಂತಿರುಗಿ *ಇದು;
}

ಶೂನ್ಯ APInt::shlSlowCase(ಚಿಹ್ನೆರಹಿತ ಸರಿಸು_Amt_2) {
  tc_ಸರಿಸು_ಎಡ(U.pVal, get_ಅಂಕೆ_ಪದಗಳು(), ಸರಿಸು_Amt_2);
  clear_ಬಳಸದ_ಬಿಟ್ಟುಗಳು();
}

// Calculate the rotate amount modulo the bit width.
ಸ್ಥಿರ ಚಿಹ್ನೆರಹಿತ rotateModulo(ಚಿಹ್ನೆರಹಿತ ಬಿಟ್_ಅಗಲ_2, ಸ್ಥಿರಾಂಕ APInt &rotateAmt) {
  ಆದರೆ (LLVM_UNLIKELY(ಬಿಟ್_ಅಗಲ_2 == 0))
    ಹಿಂತಿರುಗಿ 0;
  ಚಿಹ್ನೆರಹಿತ rot_ಬಿಟ್_ಅಗಲ = rotateAmt.get_ಬಿಟ್_ಅಗಲ();
  APInt rot = rotateAmt;
  ಆದರೆ (rot_ಬಿಟ್_ಅಗಲ < ಬಿಟ್_ಅಗಲ_2) {
    // Extend the rotate APInt, so that the urem doesn't divide by 0.
    // e.g. APInt(1, 32) would give APInt(1, 0).
    rot = rotateAmt.zext(ಬಿಟ್_ಅಗಲ_2);
  }
  rot = rot.urem(APInt(rot.get_ಬಿಟ್_ಅಗಲ(), ಬಿಟ್_ಅಗಲ_2));
  ಹಿಂತಿರುಗಿ rot.get_Limited_ಮೌಲ್ಯ(ಬಿಟ್_ಅಗಲ_2);
}

APInt APInt::rotl(ಸ್ಥಿರಾಂಕ APInt &rotateAmt) ಸ್ಥಿರಾಂಕ {
  ಹಿಂತಿರುಗಿ rotl(rotateModulo(ಬಿಟ್_ಅಗಲ_2, rotateAmt));
}

APInt APInt::rotl(ಚಿಹ್ನೆರಹಿತ rotateAmt) ಸ್ಥಿರಾಂಕ {
  ಆದರೆ (LLVM_UNLIKELY(ಬಿಟ್_ಅಗಲ_2 == 0))
    ಹಿಂತಿರುಗಿ *ಇದು;
  rotateAmt %= ಬಿಟ್_ಅಗಲ_2;
  ಆದರೆ (rotateAmt == 0)
    ಹಿಂತಿರುಗಿ *ಇದು;
  ಹಿಂತಿರುಗಿ shl(rotateAmt) | lshr(ಬಿಟ್_ಅಗಲ_2 - rotateAmt);
}

APInt APInt::rotr(ಸ್ಥಿರಾಂಕ APInt &rotateAmt) ಸ್ಥಿರಾಂಕ {
  ಹಿಂತಿರುಗಿ rotr(rotateModulo(ಬಿಟ್_ಅಗಲ_2, rotateAmt));
}

APInt APInt::rotr(ಚಿಹ್ನೆರಹಿತ rotateAmt) ಸ್ಥಿರಾಂಕ {
  ಆದರೆ (ಬಿಟ್_ಅಗಲ_2 == 0)
    ಹಿಂತಿರುಗಿ *ಇದು;
  rotateAmt %= ಬಿಟ್_ಅಗಲ_2;
  ಆದರೆ (rotateAmt == 0)
    ಹಿಂತಿರುಗಿ *ಇದು;
  ಹಿಂತಿರುಗಿ lshr(rotateAmt) | shl(ಬಿಟ್_ಅಗಲ_2 - rotateAmt);
}

/// \returns the nearest log base 2 of this APInt. Ties round up.
///
/// NOTE: When we have a BitWidth of 1, we define:
///
///   log2(0) = UINT32_MAX
///   log2(1) = 0
///
/// to get around any mathematical concerns resulting from
/// referencing 2 in a space where 2 does no exist.
ಚಿಹ್ನೆರಹಿತ APInt::ಹತ್ತಿರದ_Log_Base2() ಸ್ಥಿರಾಂಕ {
  // Special case when we have a bitwidth of 1. If VAL is 1, then we
  // get 0. If VAL is 0, we get WORDTYPE_MAX which gets truncated to
  // UINT32_MAX.
  ಆದರೆ (ಬಿಟ್_ಅಗಲ_2 == 1)
    ಹಿಂತಿರುಗಿ U.VAL - 1;

  // Handle the zero case.
  ಆದರೆ (isZero())
    ಹಿಂತಿರುಗಿ UINT32_MAX;

  // The non-zero case is handled by computing:
  //
  //   nearestLogBase2(x) = logBase2(x) + x[logBase2(x)-1].
  //
  // where x[i] is referring to the value of the ith bit of x.
  ಚಿಹ್ನೆರಹಿತ lg = logBase2();
  ಹಿಂತಿರುಗಿ lg + ಚಿಹ್ನೆರಹಿತ((*ಇದು)[lg - 1]);
}

// Square Root - this method computes and returns the square root of "this".
// Three mechanisms are used for computation. For small values (<= 5 bits),
// a table lookup is done. This gets some performance for common cases. For
// values using less than 52 bits, the value is converted to double and then
// the libc sqrt function is called. The result is rounded and then converted
// back to a uint64_t which is then used to construct the result. Finally,
// the Babylonian method for computing square roots is used.
APInt APInt::sqrtFloor() ಸ್ಥಿರಾಂಕ {

  // Determine the magnitude of the value.
  ಚಿಹ್ನೆರಹಿತ magnitude = get_ಸಕ್ರಿಯ_ಬಿಟ್ಟುಗಳು();

  // Use a fast table for some small values. This also gets rid of some
  // rounding errors in libc sqrt for small values.
  ಆದರೆ (magnitude <= 5) {
    ಸ್ಥಿರ ಸ್ಥಿರಾಂಕ uint8_t results[32] = {
        /*     0 */ 0,
        /*  1- 3 */ 1, 1, 1,
        /*  4- 8 */ 2, 2, 2, 2, 2,
        /*  9-15 */ 3, 3, 3, 3, 3, 3, 3,
        /* 16-24 */ 4, 4, 4, 4, 4, 4, 4, 4, 4,
        /* 25-31 */ 5, 5, 5, 5, 5, 5, 5,
    };
    ಹಿಂತಿರುಗಿ APInt(ಬಿಟ್_ಅಗಲ_2,
                 results[(LLVM_LIKELY(is_Single_ಶಬ್ದ()) ? U.VAL : U.pVal[0])]);
  }

  // If the magnitude of the value fits in less than 52 bits (the precision of
  // an IEEE double precision floating point value), then we can use the
  // libc sqrt function which will probably use a hardware sqrt computation.
  // This should be faster than the algorithm below.
  ಆದರೆ (magnitude < 52) {
    ಹಿಂತಿರುಗಿ APInt(ಬಿಟ್_ಅಗಲ_2,
                 uint64_t(::floor(::ವರ್ಗಮೂಲ(ದ್ವಿಗುಣ(
                     LLVM_LIKELY(is_Single_ಶಬ್ದ()) ? U.VAL : U.pVal[0])))));
  }

  // Okay, all the short cuts are exhausted. We must compute it. The following
  // is a classical Babylonian method for computing the square root. This code
  // was adapted to APInt from a wikipedia article on such computations.
  // See http://www.wikipedia.org/ and go to the page named
  // Calculate_an_integer_square_root.
  ಚಿಹ್ನೆರಹಿತ nbits = ಬಿಟ್_ಅಗಲ_2, i = 4;
  APInt testy(ಬಿಟ್_ಅಗಲ_2, 16);
  APInt x_old(ಬಿಟ್_ಅಗಲ_2, 1);
  APInt x_new(ಬಿಟ್_ಅಗಲ_2, 0);
  APInt two(ಬಿಟ್_ಅಗಲ_2, 2);

  // Select a good starting value using binary logarithms.
  ಗಾಗಿ (;; i += 2, testy = testy.shl(2))
    ಆದರೆ (i >= nbits || ಇದು->ule(testy)) {
      x_old = x_old.shl(i / 2);
      ನಿಲ್ಲಿಸು;
    }

  // Use the Babylonian method to arrive at the integer square root:
  ಗಾಗಿ (;;) {
    x_new = (ಇದು->udiv(x_old) + x_old).udiv(two);
    ಆದರೆ (x_old.ule(x_new))
      ನಿಲ್ಲಿಸು;
    x_old = x_new;
  }
  ಹಿಂತಿರುಗಿ x_old;
}

/// \returns the multiplicative inverse of an odd APInt modulo 2^BitWidth.
APInt APInt::multiplicativeInverse() ಸ್ಥಿರಾಂಕ {
  ಪ್ರತಿಜ್ಞಾ((*ಇದು)[0] &&
         "multiplicative inverse is only defined for odd numbers!");

  // Use Newton's method.
  APInt ಗುಣಕ = *ಇದು;
  APInt T;
  ಆಗ (!(T = *ಇದು * ಗುಣಕ).isOne())
    ಗುಣಕ *= 2 - std::move(T);
  ಹಿಂತಿರುಗಿ ಗುಣಕ;
}

/// Implementation of Knuth's Algorithm D (Division of nonnegative integers)
/// from "Art of Computer Programming, Volume 2", section 4.3.1, p. 272. The
/// variables here have the same names as in the algorithm. Comments explain
/// the algorithm and any deviation from it.
ಸ್ಥಿರ ಶೂನ್ಯ KnuthDiv(uint32_t *u, uint32_t *v, uint32_t *q, uint32_t* r,
                     ಚಿಹ್ನೆರಹಿತ m, ಚಿಹ್ನೆರಹಿತ n) {
  ಪ್ರತಿಜ್ಞಾ(u && "Must provide dividend");
  ಪ್ರತಿಜ್ಞಾ(v && "Must provide divisor");
  ಪ್ರತಿಜ್ಞಾ(q && "Must provide quotient");
  ಪ್ರತಿಜ್ಞಾ(u != v && u != q && v != q && "Must use different memory");
  ಪ್ರತಿಜ್ಞಾ(n>1 && "n must be > 1");

  // b denotes the base of the number system. In our case b is 2^32.
  ಸ್ಥಿರಾಂಕ uint64_t b = uint64_t(1) << 32;

// The DEBUG macros here tend to be spam in the debug output if you're not
// debugging this code. Disable them unless KNUTH_DEBUG is defined.
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ KNUTH_DEBUG
#ವ್ಯಾಖ್ಯಾನಿಸು DEBUG_KNUTH(X) LLVM_DEBUG(X)
#ಇಲ್ಲದಿದ್ದರೆ
#ವ್ಯಾಖ್ಯಾನಿಸು DEBUG_KNUTH(X) ಮಾಡು {} ಆಗ(ಸುಳ್ಳು)
#ಆದರೆ_ಅಂತ್ಯ

  DEBUG_KNUTH(dbgs() << "KnuthDiv: m=" << m << " n=" << n << '\n');
  DEBUG_KNUTH(dbgs() << "KnuthDiv: original:");
  DEBUG_KNUTH(ಗಾಗಿ (ಪೂರ್ಣಾಂಕ i = m + n; i >= 0; i--) dbgs() << " " << u[i]);
  DEBUG_KNUTH(dbgs() << " by");
  DEBUG_KNUTH(ಗಾಗಿ (ಪೂರ್ಣಾಂಕ i = n; i > 0; i--) dbgs() << " " << v[i - 1]);
  DEBUG_KNUTH(dbgs() << '\n');
  // D1. [Normalize.] Set d = b / (v[n-1] + 1) and multiply all the digits of
  // u and v by d. Note that we have taken Knuth's advice here to use a power
  // of 2 value for d such that d * v[n-1] >= b/2 (b is the base). A power of
  // 2 allows us to shift instead of multiply and it is easy to determine the
  // shift amount from the leading zeros.  We are basically normalizing the u
  // and v so that its high bits are shifted to the top of v's range without
  // overflow. Note that this can require an extra word in u so that u must
  // be of length m+n+1.
  ಚಿಹ್ನೆರಹಿತ ಸರಿಸು = llvm::countl_zero(v[n - 1]);
  uint32_t v_ವಾಹಕ = 0;
  uint32_t u_ವಾಹಕ = 0;
  ಆದರೆ (ಸರಿಸು) {
    ಗಾಗಿ (ಚಿಹ್ನೆರಹಿತ i = 0; i < m+n; ++i) {
      uint32_t u_tmp = u[i] >> (32 - ಸರಿಸು);
      u[i] = (u[i] << ಸರಿಸು) | u_ವಾಹಕ;
      u_ವಾಹಕ = u_tmp;
    }
    ಗಾಗಿ (ಚಿಹ್ನೆರಹಿತ i = 0; i < n; ++i) {
      uint32_t v_tmp = v[i] >> (32 - ಸರಿಸು);
      v[i] = (v[i] << ಸರಿಸು) | v_ವಾಹಕ;
      v_ವಾಹಕ = v_tmp;
    }
  }
  u[m+n] = u_ವಾಹಕ;

  DEBUG_KNUTH(dbgs() << "KnuthDiv:   normal:");
  DEBUG_KNUTH(ಗಾಗಿ (ಪೂರ್ಣಾಂಕ i = m + n; i >= 0; i--) dbgs() << " " << u[i]);
  DEBUG_KNUTH(dbgs() << " by");
  DEBUG_KNUTH(ಗಾಗಿ (ಪೂರ್ಣಾಂಕ i = n; i > 0; i--) dbgs() << " " << v[i - 1]);
  DEBUG_KNUTH(dbgs() << '\n');

  // D2. [Initialize j.]  Set j to m. This is the loop counter over the places.
  ಪೂರ್ಣಾಂಕ j = m;
  ಮಾಡು {
    DEBUG_KNUTH(dbgs() << "KnuthDiv: quotient digit #" << j << '\n');
    // D3. [Calculate q'.].
    //     Set qp = (u[j+n]*b + u[j+n-1]) / v[n-1]. (qp=qprime=q')
    //     Set rp = (u[j+n]*b + u[j+n-1]) % v[n-1]. (rp=rprime=r')
    // Now test if qp == b or qp*v[n-2] > b*rp + u[j+n-2]; if so, decrease
    // qp by 1, increase rp by v[n-1], and repeat this test if rp < b. The test
    // on v[n-2] determines at high speed most of the cases in which the trial
    // value qp is one too large, and it eliminates all cases where qp is two
    // too large.
    uint64_t dividend = Make_64(u[j+n], u[j+n-1]);
    DEBUG_KNUTH(dbgs() << "KnuthDiv: dividend == " << dividend << '\n');
    uint64_t qp = dividend / v[n-1];
    uint64_t rp = dividend % v[n-1];
    ಆದರೆ (qp == b || qp*v[n-2] > b*rp + u[j+n-2]) {
      qp--;
      rp += v[n-1];
      ಆದರೆ (rp < b && (qp == b || qp*v[n-2] > b*rp + u[j+n-2]))
        qp--;
    }
    DEBUG_KNUTH(dbgs() << "KnuthDiv: qp == " << qp << ", rp == " << rp << '\n');

    // D4. [Multiply and subtract.] Replace (u[j+n]u[j+n-1]...u[j]) with
    // (u[j+n]u[j+n-1]..u[j]) - qp * (v[n-1]...v[1]v[0]). This computation
    // consists of a simple multiplication by a one-place number, combined with
    // a subtraction.
    // The digits (u[j+n]...u[j]) should be kept positive; if the result of
    // this step is actually negative, (u[j+n]...u[j]) should be left as the
    // true value plus b**(n+1), namely as the b's complement of
    // the true value, and a "borrow" to the left should be remembered.
    int64_t borrow = 0;
    ಗಾಗಿ (ಚಿಹ್ನೆರಹಿತ i = 0; i < n; ++i) {
      uint64_t p = qp * uint64_t(v[i]);
      int64_t subres = int64_t(u[j+i]) - borrow - Lo_32(p);
      u[j+i] = Lo_32(subres);
      borrow = Hi_32(p) - Hi_32(subres);
      DEBUG_KNUTH(dbgs() << "KnuthDiv: u[j+i] = " << u[j + i]
                        << ", borrow = " << borrow << '\n');
    }
    ತಾರ್ಕಿಕ isNeg = u[j+n] < borrow;
    u[j+n] -= Lo_32(borrow);

    DEBUG_KNUTH(dbgs() << "KnuthDiv: after subtraction:");
    DEBUG_KNUTH(ಗಾಗಿ (ಪೂರ್ಣಾಂಕ i = m + n; i >= 0; i--) dbgs() << " " << u[i]);
    DEBUG_KNUTH(dbgs() << '\n');

    // D5. [Test remainder.] Set q[j] = qp. If the result of step D4 was
    // negative, go to step D6; otherwise go on to step D7.
    q[j] = Lo_32(qp);
    ಆದರೆ (isNeg) {
      // D6. [Add back]. The probability that this step is necessary is very
      // small, on the order of only 2/b. Make sure that test data accounts for
      // this possibility. Decrease q[j] by 1
      q[j]--;
      // and add (0v[n-1]...v[1]v[0]) to (u[j+n]u[j+n-1]...u[j+1]u[j]).
      // A carry will occur to the left of u[j+n], and it should be ignored
      // since it cancels with the borrow that occurred in D4.
      ತಾರ್ಕಿಕ ವಾಹಕ = ಸುಳ್ಳು;
      ಗಾಗಿ (ಚಿಹ್ನೆರಹಿತ i = 0; i < n; i++) {
        uint32_t ಎಲ್ಲೆ = std::min(u[j+i],v[i]);
        u[j+i] += v[i] + ವಾಹಕ;
        ವಾಹಕ = u[j+i] < ಎಲ್ಲೆ || (ವಾಹಕ && u[j+i] == ಎಲ್ಲೆ);
      }
      u[j+n] += ವಾಹಕ;
    }
    DEBUG_KNUTH(dbgs() << "KnuthDiv: after correction:");
    DEBUG_KNUTH(ಗಾಗಿ (ಪೂರ್ಣಾಂಕ i = m + n; i >= 0; i--) dbgs() << " " << u[i]);
    DEBUG_KNUTH(dbgs() << "\nKnuthDiv: digit result = " << q[j] << '\n');

    // D7. [Loop on j.]  Decrease j by one. Now if j >= 0, go back to D3.
  } ಆಗ (--j >= 0);

  DEBUG_KNUTH(dbgs() << "KnuthDiv: quotient:");
  DEBUG_KNUTH(ಗಾಗಿ (ಪೂರ್ಣಾಂಕ i = m; i >= 0; i--) dbgs() << " " << q[i]);
  DEBUG_KNUTH(dbgs() << '\n');

  // D8. [Unnormalize]. Now q[...] is the desired quotient, and the desired
  // remainder may be obtained by dividing u[...] by d. If r is non-null we
  // compute the remainder (urem uses this).
  ಆದರೆ (r) {
    // The value d is expressed by the "shift" value above since we avoided
    // multiplication by d by using a shift left. So, all we have to do is
    // shift right here.
    ಆದರೆ (ಸರಿಸು) {
      uint32_t ವಾಹಕ = 0;
      DEBUG_KNUTH(dbgs() << "KnuthDiv: remainder:");
      ಗಾಗಿ (ಪೂರ್ಣಾಂಕ i = n-1; i >= 0; i--) {
        r[i] = (u[i] >> ಸರಿಸು) | ವಾಹಕ;
        ವಾಹಕ = u[i] << (32 - ಸರಿಸು);
        DEBUG_KNUTH(dbgs() << " " << r[i]);
      }
    } ಇಲ್ಲದಿದ್ದರೆ {
      ಗಾಗಿ (ಪೂರ್ಣಾಂಕ i = n-1; i >= 0; i--) {
        r[i] = u[i];
        DEBUG_KNUTH(dbgs() << " " << r[i]);
      }
    }
    DEBUG_KNUTH(dbgs() << '\n');
  }
  DEBUG_KNUTH(dbgs() << '\n');
}

ಶೂನ್ಯ APInt::divide(ಸ್ಥಿರಾಂಕ ಶಬ್ದ_Type *LHS, ಚಿಹ್ನೆರಹಿತ lhs_ಪದಗಳು, ಸ್ಥಿರಾಂಕ ಶಬ್ದ_Type *RHS,
                   ಚಿಹ್ನೆರಹಿತ rhs_ಪದಗಳು, ಶಬ್ದ_Type *Quotient, ಶಬ್ದ_Type *Remainder) {
  ಪ್ರತಿಜ್ಞಾ(lhs_ಪದಗಳು >= rhs_ಪದಗಳು && "Fractional result");

  // First, compose the values into an array of 32-bit words instead of
  // 64-bit words. This is a necessity of both the "short division" algorithm
  // and the Knuth "classical algorithm" which requires there to be native
  // operations for +, -, and * on an m bit value with an m*2 bit result. We
  // can't use 64-bit operands here because we don't have native results of
  // 128-bits. Furthermore, casting the 64-bit values to 32-bit values won't
  // work on large-endian machines.
  ಚಿಹ್ನೆರಹಿತ n = rhs_ಪದಗಳು * 2;
  ಚಿಹ್ನೆರಹಿತ m = (lhs_ಪದಗಳು * 2) - n;

  // Allocate space for the temporary values we need either on the stack, if
  // it will fit, or on the heap if it won't.
  uint32_t SPACE[128];
  uint32_t *U = ಶೂನ್ಯಸೂಚಕ;
  uint32_t *V = ಶೂನ್ಯಸೂಚಕ;
  uint32_t *Q = ಶೂನ್ಯಸೂಚಕ;
  uint32_t *R = ಶೂನ್ಯಸೂಚಕ;
  ಆದರೆ ((Remainder?4:3)*n+2*m+1 <= 128) {
    U = &SPACE[0];
    V = &SPACE[m+n+1];
    Q = &SPACE[(m+n+1) + n];
    ಆದರೆ (Remainder)
      R = &SPACE[(m+n+1) + n + (m+n)];
  } ಇಲ್ಲದಿದ್ದರೆ {
    U = ಹೊಸ uint32_t[m + n + 1];
    V = ಹೊಸ uint32_t[n];
    Q = ಹೊಸ uint32_t[m+n];
    ಆದರೆ (Remainder)
      R = ಹೊಸ uint32_t[n];
  }

  // Initialize the dividend
  memset(U, 0, (m+n+1)*ಗಾತ್ರ(uint32_t));
  ಗಾಗಿ (ಚಿಹ್ನೆರಹಿತ i = 0; i < lhs_ಪದಗಳು; ++i) {
    uint64_t tmp = LHS[i];
    U[i * 2] = Lo_32(tmp);
    U[i * 2 + 1] = Hi_32(tmp);
  }
  U[m+n] = 0; // this extra word is for "spill" in the Knuth algorithm.

  // Initialize the divisor
  memset(V, 0, (n)*ಗಾತ್ರ(uint32_t));
  ಗಾಗಿ (ಚಿಹ್ನೆರಹಿತ i = 0; i < rhs_ಪದಗಳು; ++i) {
    uint64_t tmp = RHS[i];
    V[i * 2] = Lo_32(tmp);
    V[i * 2 + 1] = Hi_32(tmp);
  }

  // initialize the quotient and remainder
  memset(Q, 0, (m+n) * ಗಾತ್ರ(uint32_t));
  ಆದರೆ (Remainder)
    memset(R, 0, n * ಗಾತ್ರ(uint32_t));

  // Now, adjust m and n for the Knuth division. n is the number of words in
  // the divisor. m is the number of words by which the dividend exceeds the
  // divisor (i.e. m+n is the length of the dividend). These sizes must not
  // contain any zero words or the Knuth algorithm fails.
  ಗಾಗಿ (ಚಿಹ್ನೆರಹಿತ i = n; i > 0 && V[i-1] == 0; i--) {
    n--;
    m++;
  }
  ಗಾಗಿ (ಚಿಹ್ನೆರಹಿತ i = m+n; i > 0 && U[i-1] == 0; i--)
    m--;

  // If we're left with only a single word for the divisor, Knuth doesn't work
  // so we implement the short division algorithm here. This is much simpler
  // and faster because we are certain that we can divide a 64-bit quantity
  // by a 32-bit quantity at hardware speed and short division is simply a
  // series of such operations. This is just like doing short division but we
  // are using base 2^32 instead of base 10.
  ಪ್ರತಿಜ್ಞಾ(n != 0 && "Divide by zero?");
  ಆದರೆ (n == 1) {
    uint32_t ಭಾಜಕ = V[0];
    uint32_t remainder = 0;
    ಗಾಗಿ (ಪೂರ್ಣಾಂಕ i = m; i >= 0; i--) {
      uint64_t partial_dividend = Make_64(remainder, U[i]);
      ಆದರೆ (partial_dividend == 0) {
        Q[i] = 0;
        remainder = 0;
      } ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (partial_dividend < ಭಾಜಕ) {
        Q[i] = 0;
        remainder = Lo_32(partial_dividend);
      } ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (partial_dividend == ಭಾಜಕ) {
        Q[i] = 1;
        remainder = 0;
      } ಇಲ್ಲದಿದ್ದರೆ {
        Q[i] = Lo_32(partial_dividend / ಭಾಜಕ);
        remainder = Lo_32(partial_dividend - (Q[i] * ಭಾಜಕ));
      }
    }
    ಆದರೆ (R)
      R[0] = remainder;
  } ಇಲ್ಲದಿದ್ದರೆ {
    // Now we're ready to invoke the Knuth classical divide algorithm. In this
    // case n > 1.
    KnuthDiv(U, V, Q, R, m, n);
  }

  // If the caller wants the quotient
  ಆದರೆ (Quotient) {
    ಗಾಗಿ (ಚಿಹ್ನೆರಹಿತ i = 0; i < lhs_ಪದಗಳು; ++i)
      Quotient[i] = Make_64(Q[i*2+1], Q[i*2]);
  }

  // If the caller wants the remainder
  ಆದರೆ (Remainder) {
    ಗಾಗಿ (ಚಿಹ್ನೆರಹಿತ i = 0; i < rhs_ಪದಗಳು; ++i)
      Remainder[i] = Make_64(R[i*2+1], R[i*2]);
  }

  // Clean up the memory we allocated.
  ಆದರೆ (U != &SPACE[0]) {
    ಅಳಿಸು [] U;
    ಅಳಿಸು [] V;
    ಅಳಿಸು [] Q;
    ಅಳಿಸು [] R;
  }
}

APInt APInt::udiv(ಸ್ಥಿರಾಂಕ APInt &RHS) ಸ್ಥಿರಾಂಕ {
  ಪ್ರತಿಜ್ಞಾ(ಬಿಟ್_ಅಗಲ_2 == RHS.ಬಿಟ್_ಅಗಲ_2 && "Bit widths must be the same");

  // First, deal with the easy case
  ಆದರೆ (LLVM_LIKELY(is_Single_ಶಬ್ದ())) {
    ಪ್ರತಿಜ್ಞಾ(RHS.U.VAL != 0 && "Divide by zero?");
    ಹಿಂತಿರುಗಿ APInt(ಬಿಟ್_ಅಗಲ_2, U.VAL / RHS.U.VAL);
  }

  // Get some facts about the LHS and RHS number of bits and words
  ಚಿಹ್ನೆರಹಿತ lhs_ಪದಗಳು = get_ಅಂಕೆ_ಪದಗಳು(get_ಸಕ್ರಿಯ_ಬಿಟ್ಟುಗಳು());
  ಚಿಹ್ನೆರಹಿತ rhs_ಬಿಟ್ಟುಗಳು  = RHS.get_ಸಕ್ರಿಯ_ಬಿಟ್ಟುಗಳು();
  ಚಿಹ್ನೆರಹಿತ rhs_ಪದಗಳು = get_ಅಂಕೆ_ಪದಗಳು(rhs_ಬಿಟ್ಟುಗಳು);
  ಪ್ರತಿಜ್ಞಾ(rhs_ಪದಗಳು && "Divided by zero???");

  // Deal with some degenerate cases
  ಆದರೆ (!lhs_ಪದಗಳು)
    // 0 / X ===> 0
    ಹಿಂತಿರುಗಿ APInt(ಬಿಟ್_ಅಗಲ_2, 0);
  ಆದರೆ (rhs_ಬಿಟ್ಟುಗಳು == 1)
    // X / 1 ===> X
    ಹಿಂತಿರುಗಿ *ಇದು;
  ಆದರೆ (lhs_ಪದಗಳು < rhs_ಪದಗಳು || ಇದು->ult(RHS))
    // X / Y ===> 0, iff X < Y
    ಹಿಂತಿರುಗಿ APInt(ಬಿಟ್_ಅಗಲ_2, 0);
  ಆದರೆ (*ಇದು == RHS)
    // X / X ===> 1
    ಹಿಂತಿರುಗಿ APInt(ಬಿಟ್_ಅಗಲ_2, 1);
  ಆದರೆ (lhs_ಪದಗಳು == 1) // rhsWords is 1 if lhsWords is 1.
    // All high words are zero, just use native divide
    ಹಿಂತಿರುಗಿ APInt(ಬಿಟ್_ಅಗಲ_2, ಇದು->U.pVal[0] / RHS.U.pVal[0]);

  // We have to compute it the hard way. Invoke the Knuth divide algorithm.
  APInt Quotient(ಬಿಟ್_ಅಗಲ_2, 0); // to hold result.
  divide(U.pVal, lhs_ಪದಗಳು, RHS.U.pVal, rhs_ಪದಗಳು, Quotient.U.pVal, ಶೂನ್ಯಸೂಚಕ);
  ಹಿಂತಿರುಗಿ Quotient;
}

APInt APInt::udiv(uint64_t RHS) ಸ್ಥಿರಾಂಕ {
  ಪ್ರತಿಜ್ಞಾ(RHS != 0 && "Divide by zero?");

  // First, deal with the easy case
  ಆದರೆ (LLVM_LIKELY(is_Single_ಶಬ್ದ()))
    ಹಿಂತಿರುಗಿ APInt(ಬಿಟ್_ಅಗಲ_2, U.VAL / RHS);

  // Get some facts about the LHS words.
  ಚಿಹ್ನೆರಹಿತ lhs_ಪದಗಳು = get_ಅಂಕೆ_ಪದಗಳು(get_ಸಕ್ರಿಯ_ಬಿಟ್ಟುಗಳು());

  // Deal with some degenerate cases
  ಆದರೆ (!lhs_ಪದಗಳು)
    // 0 / X ===> 0
    ಹಿಂತಿರುಗಿ APInt(ಬಿಟ್_ಅಗಲ_2, 0);
  ಆದರೆ (RHS == 1)
    // X / 1 ===> X
    ಹಿಂತಿರುಗಿ *ಇದು;
  ಆದರೆ (ಇದು->ult(RHS))
    // X / Y ===> 0, iff X < Y
    ಹಿಂತಿರುಗಿ APInt(ಬಿಟ್_ಅಗಲ_2, 0);
  ಆದರೆ (*ಇದು == RHS)
    // X / X ===> 1
    ಹಿಂತಿರುಗಿ APInt(ಬಿಟ್_ಅಗಲ_2, 1);
  ಆದರೆ (lhs_ಪದಗಳು == 1) // rhsWords is 1 if lhsWords is 1.
    // All high words are zero, just use native divide
    ಹಿಂತಿರುಗಿ APInt(ಬಿಟ್_ಅಗಲ_2, ಇದು->U.pVal[0] / RHS);

  // We have to compute it the hard way. Invoke the Knuth divide algorithm.
  APInt Quotient(ಬಿಟ್_ಅಗಲ_2, 0); // to hold result.
  divide(U.pVal, lhs_ಪದಗಳು, &RHS, 1, Quotient.U.pVal, ಶೂನ್ಯಸೂಚಕ);
  ಹಿಂತಿರುಗಿ Quotient;
}

APInt APInt::sdiv(ಸ್ಥಿರಾಂಕ APInt &RHS) ಸ್ಥಿರಾಂಕ {
  ಆದರೆ (isNegative()) {
    ಆದರೆ (RHS.isNegative())
      ಹಿಂತಿರುಗಿ (-(*ಇದು)).udiv(-RHS);
    ಹಿಂತಿರುಗಿ -((-(*ಇದು)).udiv(RHS));
  }
  ಆದರೆ (RHS.isNegative())
    ಹಿಂತಿರುಗಿ -(ಇದು->udiv(-RHS));
  ಹಿಂತಿರುಗಿ ಇದು->udiv(RHS);
}

APInt APInt::sdiv(int64_t RHS) ಸ್ಥಿರಾಂಕ {
  ಆದರೆ (isNegative()) {
    ಆದರೆ (RHS < 0)
      ಹಿಂತಿರುಗಿ (-(*ಇದು)).udiv(-RHS);
    ಹಿಂತಿರುಗಿ -((-(*ಇದು)).udiv(RHS));
  }
  ಆದರೆ (RHS < 0)
    ಹಿಂತಿರುಗಿ -(ಇದು->udiv(-RHS));
  ಹಿಂತಿರುಗಿ ಇದು->udiv(RHS);
}

APInt APInt::urem(ಸ್ಥಿರಾಂಕ APInt &RHS) ಸ್ಥಿರಾಂಕ {
  ಪ್ರತಿಜ್ಞಾ(ಬಿಟ್_ಅಗಲ_2 == RHS.ಬಿಟ್_ಅಗಲ_2 && "Bit widths must be the same");
  ಆದರೆ (LLVM_LIKELY(is_Single_ಶಬ್ದ())) {
    ಪ್ರತಿಜ್ಞಾ(RHS.U.VAL != 0 && "Remainder by zero?");
    ಹಿಂತಿರುಗಿ APInt(ಬಿಟ್_ಅಗಲ_2, U.VAL % RHS.U.VAL);
  }

  // Get some facts about the LHS
  ಚಿಹ್ನೆರಹಿತ lhs_ಪದಗಳು = get_ಅಂಕೆ_ಪದಗಳು(get_ಸಕ್ರಿಯ_ಬಿಟ್ಟುಗಳು());

  // Get some facts about the RHS
  ಚಿಹ್ನೆರಹಿತ rhs_ಬಿಟ್ಟುಗಳು = RHS.get_ಸಕ್ರಿಯ_ಬಿಟ್ಟುಗಳು();
  ಚಿಹ್ನೆರಹಿತ rhs_ಪದಗಳು = get_ಅಂಕೆ_ಪದಗಳು(rhs_ಬಿಟ್ಟುಗಳು);
  ಪ್ರತಿಜ್ಞಾ(rhs_ಪದಗಳು && "Performing remainder operation by zero ???");

  // Check the degenerate cases
  ಆದರೆ (lhs_ಪದಗಳು == 0)
    // 0 % Y ===> 0
    ಹಿಂತಿರುಗಿ APInt(ಬಿಟ್_ಅಗಲ_2, 0);
  ಆದರೆ (rhs_ಬಿಟ್ಟುಗಳು == 1)
    // X % 1 ===> 0
    ಹಿಂತಿರುಗಿ APInt(ಬಿಟ್_ಅಗಲ_2, 0);
  ಆದರೆ (lhs_ಪದಗಳು < rhs_ಪದಗಳು || ಇದು->ult(RHS))
    // X % Y ===> X, iff X < Y
    ಹಿಂತಿರುಗಿ *ಇದು;
  ಆದರೆ (*ಇದು == RHS)
    // X % X == 0;
    ಹಿಂತಿರುಗಿ APInt(ಬಿಟ್_ಅಗಲ_2, 0);
  ಆದರೆ (lhs_ಪದಗಳು == 1)
    // All high words are zero, just use native remainder
    ಹಿಂತಿರುಗಿ APInt(ಬಿಟ್_ಅಗಲ_2, U.pVal[0] % RHS.U.pVal[0]);
  ಆದರೆ (RHS.isPowerOf2()) {
    // X % 2^w ===> X & (2^w - 1)
    APInt ಫಲಿತಾಂಶ(*ಇದು);
    ಫಲಿತಾಂಶ.clear_ಬಿಟ್ಟುಗಳು(RHS.logBase2(), ಬಿಟ್_ಅಗಲ_2);
    ಹಿಂತಿರುಗಿ ಫಲಿತಾಂಶ;
  }

  // We have to compute it the hard way. Invoke the Knuth divide algorithm.
  APInt Remainder(ಬಿಟ್_ಅಗಲ_2, 0);
  divide(U.pVal, lhs_ಪದಗಳು, RHS.U.pVal, rhs_ಪದಗಳು, ಶೂನ್ಯಸೂಚಕ, Remainder.U.pVal);
  ಹಿಂತಿರುಗಿ Remainder;
}

uint64_t APInt::urem(uint64_t RHS) ಸ್ಥಿರಾಂಕ {
  ಪ್ರತಿಜ್ಞಾ(RHS != 0 && "Remainder by zero?");

  ಆದರೆ (LLVM_LIKELY(is_Single_ಶಬ್ದ()))
    ಹಿಂತಿರುಗಿ U.VAL % RHS;

  // Get some facts about the LHS
  ಚಿಹ್ನೆರಹಿತ lhs_ಪದಗಳು = get_ಅಂಕೆ_ಪದಗಳು(get_ಸಕ್ರಿಯ_ಬಿಟ್ಟುಗಳು());

  // Check the degenerate cases
  ಆದರೆ (lhs_ಪದಗಳು == 0)
    // 0 % Y ===> 0
    ಹಿಂತಿರುಗಿ 0;
  ಆದರೆ (RHS == 1)
    // X % 1 ===> 0
    ಹಿಂತಿರುಗಿ 0;
  ಆದರೆ (ಇದು->ult(RHS))
    // X % Y ===> X, iff X < Y
    ಹಿಂತಿರುಗಿ get_ZExt_ಮೌಲ್ಯ();
  ಆದರೆ (*ಇದು == RHS)
    // X % X == 0;
    ಹಿಂತಿರುಗಿ 0;
  ಆದರೆ (lhs_ಪದಗಳು == 1)
    // All high words are zero, just use native remainder
    ಹಿಂತಿರುಗಿ U.pVal[0] % RHS;
  ಆದರೆ (llvm::isPowerOf2_64(RHS))
    // X % 2^w ===> X & (2^w - 1)
    ಹಿಂತಿರುಗಿ U.pVal[0] & (RHS - 1);

  // We have to compute it the hard way. Invoke the Knuth divide algorithm.
  uint64_t Remainder;
  divide(U.pVal, lhs_ಪದಗಳು, &RHS, 1, ಶೂನ್ಯಸೂಚಕ, &Remainder);
  ಹಿಂತಿರುಗಿ Remainder;
}

APInt APInt::srem(ಸ್ಥಿರಾಂಕ APInt &RHS) ಸ್ಥಿರಾಂಕ {
  ಆದರೆ (isNegative()) {
    ಆದರೆ (RHS.isNegative())
      ಹಿಂತಿರುಗಿ -((-(*ಇದು)).urem(-RHS));
    ಹಿಂತಿರುಗಿ -((-(*ಇದು)).urem(RHS));
  }
  ಆದರೆ (RHS.isNegative())
    ಹಿಂತಿರುಗಿ ಇದು->urem(-RHS);
  ಹಿಂತಿರುಗಿ ಇದು->urem(RHS);
}

int64_t APInt::srem(int64_t RHS) ಸ್ಥಿರಾಂಕ {
  ಆದರೆ (isNegative()) {
    ಆದರೆ (RHS < 0)
      ಹಿಂತಿರುಗಿ -((-(*ಇದು)).urem(-RHS));
    ಹಿಂತಿರುಗಿ -((-(*ಇದು)).urem(RHS));
  }
  ಆದರೆ (RHS < 0)
    ಹಿಂತಿರುಗಿ ಇದು->urem(-RHS);
  ಹಿಂತಿರುಗಿ ಇದು->urem(RHS);
}

ಶೂನ್ಯ APInt::udivrem(ಸ್ಥಿರಾಂಕ APInt &LHS, ಸ್ಥಿರಾಂಕ APInt &RHS,
                    APInt &Quotient, APInt &Remainder) {
  ಪ್ರತಿಜ್ಞಾ(LHS.ಬಿಟ್_ಅಗಲ_2 == RHS.ಬಿಟ್_ಅಗಲ_2 && "Bit widths must be the same");
  ಚಿಹ್ನೆರಹಿತ ಬಿಟ್_ಅಗಲ_2 = LHS.ಬಿಟ್_ಅಗಲ_2;

  // First, deal with the easy case
  ಆದರೆ (LLVM_LIKELY(LHS.is_Single_ಶಬ್ದ())) {
    ಪ್ರತಿಜ್ಞಾ(RHS.U.VAL != 0 && "Divide by zero?");
    uint64_t QuotVal = LHS.U.VAL / RHS.U.VAL;
    uint64_t RemVal = LHS.U.VAL % RHS.U.VAL;
    Quotient = APInt(ಬಿಟ್_ಅಗಲ_2, QuotVal);
    Remainder = APInt(ಬಿಟ್_ಅಗಲ_2, RemVal);
    ಹಿಂತಿರುಗಿ;
  }

  // Get some size facts about the dividend and divisor
  ಚಿಹ್ನೆರಹಿತ lhs_ಪದಗಳು = get_ಅಂಕೆ_ಪದಗಳು(LHS.get_ಸಕ್ರಿಯ_ಬಿಟ್ಟುಗಳು());
  ಚಿಹ್ನೆರಹಿತ rhs_ಬಿಟ್ಟುಗಳು  = RHS.get_ಸಕ್ರಿಯ_ಬಿಟ್ಟುಗಳು();
  ಚಿಹ್ನೆರಹಿತ rhs_ಪದಗಳು = get_ಅಂಕೆ_ಪದಗಳು(rhs_ಬಿಟ್ಟುಗಳು);
  ಪ್ರತಿಜ್ಞಾ(rhs_ಪದಗಳು && "Performing divrem operation by zero ???");

  // Check the degenerate cases
  ಆದರೆ (lhs_ಪದಗಳು == 0) {
    Quotient = APInt(ಬಿಟ್_ಅಗಲ_2, 0);    // 0 / Y ===> 0
    Remainder = APInt(ಬಿಟ್_ಅಗಲ_2, 0);   // 0 % Y ===> 0
    ಹಿಂತಿರುಗಿ;
  }

  ಆದರೆ (rhs_ಬಿಟ್ಟುಗಳು == 1) {
    Quotient = LHS;                   // X / 1 ===> X
    Remainder = APInt(ಬಿಟ್_ಅಗಲ_2, 0);   // X % 1 ===> 0
  }

  ಆದರೆ (lhs_ಪದಗಳು < rhs_ಪದಗಳು || LHS.ult(RHS)) {
    Remainder = LHS;                  // X % Y ===> X, iff X < Y
    Quotient = APInt(ಬಿಟ್_ಅಗಲ_2, 0);    // X / Y ===> 0, iff X < Y
    ಹಿಂತಿರುಗಿ;
  }

  ಆದರೆ (LHS == RHS) {
    Quotient  = APInt(ಬಿಟ್_ಅಗಲ_2, 1);   // X / X ===> 1
    Remainder = APInt(ಬಿಟ್_ಅಗಲ_2, 0);   // X % X ===> 0;
    ಹಿಂತಿರುಗಿ;
  }

  // Make sure there is enough space to hold the results.
  // NOTE: This assumes that reallocate won't affect any bits if it doesn't
  // change the size. This is necessary if Quotient or Remainder is aliased
  // with LHS or RHS.
  Quotient.reallocate(ಬಿಟ್_ಅಗಲ_2);
  Remainder.reallocate(ಬಿಟ್_ಅಗಲ_2);

  ಆದರೆ (lhs_ಪದಗಳು == 1) { // rhsWords is 1 if lhsWords is 1.
    // There is only one word to consider so use the native versions.
    uint64_t lhs_ಮೌಲ್ಯ = LHS.U.pVal[0];
    uint64_t rhs_ಮೌಲ್ಯ = RHS.U.pVal[0];
    Quotient = lhs_ಮೌಲ್ಯ / rhs_ಮೌಲ್ಯ;
    Remainder = lhs_ಮೌಲ್ಯ % rhs_ಮೌಲ್ಯ;
    ಹಿಂತಿರುಗಿ;
  }

  // Okay, lets do it the long way
  divide(LHS.U.pVal, lhs_ಪದಗಳು, RHS.U.pVal, rhs_ಪದಗಳು, Quotient.U.pVal,
         Remainder.U.pVal);
  // Clear the rest of the Quotient and Remainder.
  std::memset(Quotient.U.pVal + lhs_ಪದಗಳು, 0,
              (get_ಅಂಕೆ_ಪದಗಳು(ಬಿಟ್_ಅಗಲ_2) - lhs_ಪದಗಳು) * APINT_ಶಬ್ದ_ಗಾತ್ರ);
  std::memset(Remainder.U.pVal + rhs_ಪದಗಳು, 0,
              (get_ಅಂಕೆ_ಪದಗಳು(ಬಿಟ್_ಅಗಲ_2) - rhs_ಪದಗಳು) * APINT_ಶಬ್ದ_ಗಾತ್ರ);
}

ಶೂನ್ಯ APInt::udivrem(ಸ್ಥಿರಾಂಕ APInt &LHS, uint64_t RHS, APInt &Quotient,
                    uint64_t &Remainder) {
  ಪ್ರತಿಜ್ಞಾ(RHS != 0 && "Divide by zero?");
  ಚಿಹ್ನೆರಹಿತ ಬಿಟ್_ಅಗಲ_2 = LHS.ಬಿಟ್_ಅಗಲ_2;

  // First, deal with the easy case
  ಆದರೆ (LLVM_LIKELY(LHS.is_Single_ಶಬ್ದ())) {
    uint64_t QuotVal = LHS.U.VAL / RHS;
    Remainder = LHS.U.VAL % RHS;
    Quotient = APInt(ಬಿಟ್_ಅಗಲ_2, QuotVal);
    ಹಿಂತಿರುಗಿ;
  }

  // Get some size facts about the dividend and divisor
  ಚಿಹ್ನೆರಹಿತ lhs_ಪದಗಳು = get_ಅಂಕೆ_ಪದಗಳು(LHS.get_ಸಕ್ರಿಯ_ಬಿಟ್ಟುಗಳು());

  // Check the degenerate cases
  ಆದರೆ (lhs_ಪದಗಳು == 0) {
    Quotient = APInt(ಬಿಟ್_ಅಗಲ_2, 0);    // 0 / Y ===> 0
    Remainder = 0;                    // 0 % Y ===> 0
    ಹಿಂತಿರುಗಿ;
  }

  ಆದರೆ (RHS == 1) {
    Quotient = LHS;                   // X / 1 ===> X
    Remainder = 0;                    // X % 1 ===> 0
    ಹಿಂತಿರುಗಿ;
  }

  ಆದರೆ (LHS.ult(RHS)) {
    Remainder = LHS.get_ZExt_ಮೌಲ್ಯ();   // X % Y ===> X, iff X < Y
    Quotient = APInt(ಬಿಟ್_ಅಗಲ_2, 0);    // X / Y ===> 0, iff X < Y
    ಹಿಂತಿರುಗಿ;
  }

  ಆದರೆ (LHS == RHS) {
    Quotient  = APInt(ಬಿಟ್_ಅಗಲ_2, 1);   // X / X ===> 1
    Remainder = 0;                    // X % X ===> 0;
    ಹಿಂತಿರುಗಿ;
  }

  // Make sure there is enough space to hold the results.
  // NOTE: This assumes that reallocate won't affect any bits if it doesn't
  // change the size. This is necessary if Quotient is aliased with LHS.
  Quotient.reallocate(ಬಿಟ್_ಅಗಲ_2);

  ಆದರೆ (lhs_ಪದಗಳು == 1) { // rhsWords is 1 if lhsWords is 1.
    // There is only one word to consider so use the native versions.
    uint64_t lhs_ಮೌಲ್ಯ = LHS.U.pVal[0];
    Quotient = lhs_ಮೌಲ್ಯ / RHS;
    Remainder = lhs_ಮೌಲ್ಯ % RHS;
    ಹಿಂತಿರುಗಿ;
  }

  // Okay, lets do it the long way
  divide(LHS.U.pVal, lhs_ಪದಗಳು, &RHS, 1, Quotient.U.pVal, &Remainder);
  // Clear the rest of the Quotient.
  std::memset(Quotient.U.pVal + lhs_ಪದಗಳು, 0,
              (get_ಅಂಕೆ_ಪದಗಳು(ಬಿಟ್_ಅಗಲ_2) - lhs_ಪದಗಳು) * APINT_ಶಬ್ದ_ಗಾತ್ರ);
}

ಶೂನ್ಯ APInt::sdivrem(ಸ್ಥಿರಾಂಕ APInt &LHS, ಸ್ಥಿರಾಂಕ APInt &RHS,
                    APInt &Quotient, APInt &Remainder) {
  ಆದರೆ (LHS.isNegative()) {
    ಆದರೆ (RHS.isNegative())
      APInt::udivrem(-LHS, -RHS, Quotient, Remainder);
    ಇಲ್ಲದಿದ್ದರೆ {
      APInt::udivrem(-LHS, RHS, Quotient, Remainder);
      Quotient.negate();
    }
    Remainder.negate();
  } ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (RHS.isNegative()) {
    APInt::udivrem(LHS, -RHS, Quotient, Remainder);
    Quotient.negate();
  } ಇಲ್ಲದಿದ್ದರೆ {
    APInt::udivrem(LHS, RHS, Quotient, Remainder);
  }
}

ಶೂನ್ಯ APInt::sdivrem(ಸ್ಥಿರಾಂಕ APInt &LHS, int64_t RHS,
                    APInt &Quotient, int64_t &Remainder) {
  uint64_t R = Remainder;
  ಆದರೆ (LHS.isNegative()) {
    ಆದರೆ (RHS < 0)
      APInt::udivrem(-LHS, -RHS, Quotient, R);
    ಇಲ್ಲದಿದ್ದರೆ {
      APInt::udivrem(-LHS, RHS, Quotient, R);
      Quotient.negate();
    }
    R = -R;
  } ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (RHS < 0) {
    APInt::udivrem(LHS, -RHS, Quotient, R);
    Quotient.negate();
  } ಇಲ್ಲದಿದ್ದರೆ {
    APInt::udivrem(LHS, RHS, Quotient, R);
  }
  Remainder = R;
}

APInt APInt::sadd_ov(ಸ್ಥಿರಾಂಕ APInt &RHS, ತಾರ್ಕಿಕ &ಅತಿಪ್ರವಾಹ) ಸ್ಥಿರಾಂಕ {
  APInt Res = *ಇದು+RHS;
  ಅತಿಪ್ರವಾಹ = isNonNegative() == RHS.isNonNegative() &&
             Res.isNonNegative() != isNonNegative();
  ಹಿಂತಿರುಗಿ Res;
}

APInt APInt::uadd_ov(ಸ್ಥಿರಾಂಕ APInt &RHS, ತಾರ್ಕಿಕ &ಅತಿಪ್ರವಾಹ) ಸ್ಥಿರಾಂಕ {
  APInt Res = *ಇದು+RHS;
  ಅತಿಪ್ರವಾಹ = Res.ult(RHS);
  ಹಿಂತಿರುಗಿ Res;
}

APInt APInt::ssub_ov(ಸ್ಥಿರಾಂಕ APInt &RHS, ತಾರ್ಕಿಕ &ಅತಿಪ್ರವಾಹ) ಸ್ಥಿರಾಂಕ {
  APInt Res = *ಇದು - RHS;
  ಅತಿಪ್ರವಾಹ = isNonNegative() != RHS.isNonNegative() &&
             Res.isNonNegative() != isNonNegative();
  ಹಿಂತಿರುಗಿ Res;
}

APInt APInt::usub_ov(ಸ್ಥಿರಾಂಕ APInt &RHS, ತಾರ್ಕಿಕ &ಅತಿಪ್ರವಾಹ) ಸ್ಥಿರಾಂಕ {
  APInt Res = *ಇದು-RHS;
  ಅತಿಪ್ರವಾಹ = Res.ugt(*ಇದು);
  ಹಿಂತಿರುಗಿ Res;
}

APInt APInt::sdiv_ov(ಸ್ಥಿರಾಂಕ APInt &RHS, ತಾರ್ಕಿಕ &ಅತಿಪ್ರವಾಹ) ಸ್ಥಿರಾಂಕ {
  // MININT/-1  -->  overflow.
  ಅತಿಪ್ರವಾಹ = is_Min_Signed_ಮೌಲ್ಯ() && RHS.isAllOnes();
  ಹಿಂತಿರುಗಿ sdiv(RHS);
}

APInt APInt::smul_ov(ಸ್ಥಿರಾಂಕ APInt &RHS, ತಾರ್ಕಿಕ &ಅತಿಪ್ರವಾಹ) ಸ್ಥಿರಾಂಕ {
  APInt Res = *ಇದು * RHS;

  ಆದರೆ (RHS != 0)
    ಅತಿಪ್ರವಾಹ = Res.sdiv(RHS) != *ಇದು ||
               (is_Min_Signed_ಮೌಲ್ಯ() && RHS.isAllOnes());
  ಇಲ್ಲದಿದ್ದರೆ
    ಅತಿಪ್ರವಾಹ = ಸುಳ್ಳು;
  ಹಿಂತಿರುಗಿ Res;
}

APInt APInt::umul_ov(ಸ್ಥಿರಾಂಕ APInt &RHS, ತಾರ್ಕಿಕ &ಅತಿಪ್ರವಾಹ) ಸ್ಥಿರಾಂಕ {
  ಆದರೆ (countl_zero() + RHS.countl_zero() + 2 <= ಬಿಟ್_ಅಗಲ_2) {
    ಅತಿಪ್ರವಾಹ = ನಿಜ;
    ಹಿಂತಿರುಗಿ *ಇದು * RHS;
  }

  APInt Res = lshr(1) * RHS;
  ಅತಿಪ್ರವಾಹ = Res.isNegative();
  Res <<= 1;
  ಆದರೆ ((*ಇದು)[0]) {
    Res += RHS;
    ಆದರೆ (Res.ult(RHS))
      ಅತಿಪ್ರವಾಹ = ನಿಜ;
  }
  ಹಿಂತಿರುಗಿ Res;
}

APInt APInt::sshl_ov(ಸ್ಥಿರಾಂಕ APInt &ShAmt, ತಾರ್ಕಿಕ &ಅತಿಪ್ರವಾಹ) ಸ್ಥಿರಾಂಕ {
  ಹಿಂತಿರುಗಿ sshl_ov(ShAmt.get_Limited_ಮೌಲ್ಯ(get_ಬಿಟ್_ಅಗಲ()), ಅತಿಪ್ರವಾಹ);
}

APInt APInt::sshl_ov(ಚಿಹ್ನೆರಹಿತ ShAmt, ತಾರ್ಕಿಕ &ಅತಿಪ್ರವಾಹ) ಸ್ಥಿರಾಂಕ {
  ಅತಿಪ್ರವಾಹ = ShAmt >= get_ಬಿಟ್_ಅಗಲ();
  ಆದರೆ (ಅತಿಪ್ರವಾಹ)
    ಹಿಂತಿರುಗಿ APInt(ಬಿಟ್_ಅಗಲ_2, 0);

  ಆದರೆ (isNonNegative()) // Don't allow sign change.
    ಅತಿಪ್ರವಾಹ = ShAmt >= countl_zero();
  ಇಲ್ಲದಿದ್ದರೆ
    ಅತಿಪ್ರವಾಹ = ShAmt >= countl_one();

  ಹಿಂತಿರುಗಿ *ಇದು << ShAmt;
}

APInt APInt::ushl_ov(ಸ್ಥಿರಾಂಕ APInt &ShAmt, ತಾರ್ಕಿಕ &ಅತಿಪ್ರವಾಹ) ಸ್ಥಿರಾಂಕ {
  ಹಿಂತಿರುಗಿ ushl_ov(ShAmt.get_Limited_ಮೌಲ್ಯ(get_ಬಿಟ್_ಅಗಲ()), ಅತಿಪ್ರವಾಹ);
}

APInt APInt::ushl_ov(ಚಿಹ್ನೆರಹಿತ ShAmt, ತಾರ್ಕಿಕ &ಅತಿಪ್ರವಾಹ) ಸ್ಥಿರಾಂಕ {
  ಅತಿಪ್ರವಾಹ = ShAmt >= get_ಬಿಟ್_ಅಗಲ();
  ಆದರೆ (ಅತಿಪ್ರವಾಹ)
    ಹಿಂತಿರುಗಿ APInt(ಬಿಟ್_ಅಗಲ_2, 0);

  ಅತಿಪ್ರವಾಹ = ShAmt > countl_zero();

  ಹಿಂತಿರುಗಿ *ಇದು << ShAmt;
}

APInt APInt::sfloordiv_ov(ಸ್ಥಿರಾಂಕ APInt &RHS, ತಾರ್ಕಿಕ &ಅತಿಪ್ರವಾಹ) ಸ್ಥಿರಾಂಕ {
  APInt quotient = sdiv_ov(RHS, ಅತಿಪ್ರವಾಹ);
  ಆದರೆ ((quotient * RHS != *ಇದು) && (isNegative() != RHS.isNegative()))
    ಹಿಂತಿರುಗಿ quotient - 1;
  ಹಿಂತಿರುಗಿ quotient;
}

APInt APInt::sadd_sat(ಸ್ಥಿರಾಂಕ APInt &RHS) ಸ್ಥಿರಾಂಕ {
  ತಾರ್ಕಿಕ ಅತಿಪ್ರವಾಹ;
  APInt Res = sadd_ov(RHS, ಅತಿಪ್ರವಾಹ);
  ಆದರೆ (!ಅತಿಪ್ರವಾಹ)
    ಹಿಂತಿರುಗಿ Res;

  ಹಿಂತಿರುಗಿ isNegative() ? APInt::get_Signed_Min_ಮೌಲ್ಯ(ಬಿಟ್_ಅಗಲ_2)
                      : APInt::get_Signed_Max_ಮೌಲ್ಯ(ಬಿಟ್_ಅಗಲ_2);
}

APInt APInt::uadd_sat(ಸ್ಥಿರಾಂಕ APInt &RHS) ಸ್ಥಿರಾಂಕ {
  ತಾರ್ಕಿಕ ಅತಿಪ್ರವಾಹ;
  APInt Res = uadd_ov(RHS, ಅತಿಪ್ರವಾಹ);
  ಆದರೆ (!ಅತಿಪ್ರವಾಹ)
    ಹಿಂತಿರುಗಿ Res;

  ಹಿಂತಿರುಗಿ APInt::get_Max_ಮೌಲ್ಯ(ಬಿಟ್_ಅಗಲ_2);
}

APInt APInt::ssub_sat(ಸ್ಥಿರಾಂಕ APInt &RHS) ಸ್ಥಿರಾಂಕ {
  ತಾರ್ಕಿಕ ಅತಿಪ್ರವಾಹ;
  APInt Res = ssub_ov(RHS, ಅತಿಪ್ರವಾಹ);
  ಆದರೆ (!ಅತಿಪ್ರವಾಹ)
    ಹಿಂತಿರುಗಿ Res;

  ಹಿಂತಿರುಗಿ isNegative() ? APInt::get_Signed_Min_ಮೌಲ್ಯ(ಬಿಟ್_ಅಗಲ_2)
                      : APInt::get_Signed_Max_ಮೌಲ್ಯ(ಬಿಟ್_ಅಗಲ_2);
}

APInt APInt::usub_sat(ಸ್ಥಿರಾಂಕ APInt &RHS) ಸ್ಥಿರಾಂಕ {
  ತಾರ್ಕಿಕ ಅತಿಪ್ರವಾಹ;
  APInt Res = usub_ov(RHS, ಅತಿಪ್ರವಾಹ);
  ಆದರೆ (!ಅತಿಪ್ರವಾಹ)
    ಹಿಂತಿರುಗಿ Res;

  ಹಿಂತಿರುಗಿ APInt(ಬಿಟ್_ಅಗಲ_2, 0);
}

APInt APInt::smul_sat(ಸ್ಥಿರಾಂಕ APInt &RHS) ಸ್ಥಿರಾಂಕ {
  ತಾರ್ಕಿಕ ಅತಿಪ್ರವಾಹ;
  APInt Res = smul_ov(RHS, ಅತಿಪ್ರವಾಹ);
  ಆದರೆ (!ಅತಿಪ್ರವಾಹ)
    ಹಿಂತಿರುಗಿ Res;

  // The result is negative if one and only one of inputs is negative.
  ತಾರ್ಕಿಕ ResIsNegative = isNegative() ^ RHS.isNegative();

  ಹಿಂತಿರುಗಿ ResIsNegative ? APInt::get_Signed_Min_ಮೌಲ್ಯ(ಬಿಟ್_ಅಗಲ_2)
                       : APInt::get_Signed_Max_ಮೌಲ್ಯ(ಬಿಟ್_ಅಗಲ_2);
}

APInt APInt::umul_sat(ಸ್ಥಿರಾಂಕ APInt &RHS) ಸ್ಥಿರಾಂಕ {
  ತಾರ್ಕಿಕ ಅತಿಪ್ರವಾಹ;
  APInt Res = umul_ov(RHS, ಅತಿಪ್ರವಾಹ);
  ಆದರೆ (!ಅತಿಪ್ರವಾಹ)
    ಹಿಂತಿರುಗಿ Res;

  ಹಿಂತಿರುಗಿ APInt::get_Max_ಮೌಲ್ಯ(ಬಿಟ್_ಅಗಲ_2);
}

APInt APInt::sshl_sat(ಸ್ಥಿರಾಂಕ APInt &RHS) ಸ್ಥಿರಾಂಕ {
  ಹಿಂತಿರುಗಿ sshl_sat(RHS.get_Limited_ಮೌಲ್ಯ(get_ಬಿಟ್_ಅಗಲ()));
}

APInt APInt::sshl_sat(ಚಿಹ್ನೆರಹಿತ RHS) ಸ್ಥಿರಾಂಕ {
  ತಾರ್ಕಿಕ ಅತಿಪ್ರವಾಹ;
  APInt Res = sshl_ov(RHS, ಅತಿಪ್ರವಾಹ);
  ಆದರೆ (!ಅತಿಪ್ರವಾಹ)
    ಹಿಂತಿರುಗಿ Res;

  ಹಿಂತಿರುಗಿ isNegative() ? APInt::get_Signed_Min_ಮೌಲ್ಯ(ಬಿಟ್_ಅಗಲ_2)
                      : APInt::get_Signed_Max_ಮೌಲ್ಯ(ಬಿಟ್_ಅಗಲ_2);
}

APInt APInt::ushl_sat(ಸ್ಥಿರಾಂಕ APInt &RHS) ಸ್ಥಿರಾಂಕ {
  ಹಿಂತಿರುಗಿ ushl_sat(RHS.get_Limited_ಮೌಲ್ಯ(get_ಬಿಟ್_ಅಗಲ()));
}

APInt APInt::ushl_sat(ಚಿಹ್ನೆರಹಿತ RHS) ಸ್ಥಿರಾಂಕ {
  ತಾರ್ಕಿಕ ಅತಿಪ್ರವಾಹ;
  APInt Res = ushl_ov(RHS, ಅತಿಪ್ರವಾಹ);
  ಆದರೆ (!ಅತಿಪ್ರವಾಹ)
    ಹಿಂತಿರುಗಿ Res;

  ಹಿಂತಿರುಗಿ APInt::get_Max_ಮೌಲ್ಯ(ಬಿಟ್_ಅಗಲ_2);
}

ಶೂನ್ಯ APInt::fromString(ಚಿಹ್ನೆರಹಿತ numbits, StringRef str, uint8_t radix) {
  // Check our assumptions here
  ಪ್ರತಿಜ್ಞಾ(!str.ಖಾಲಿಗಳು() && "Invalid string length");
  ಪ್ರತಿಜ್ಞಾ((radix == 10 || radix == 8 || radix == 16 || radix == 2 ||
          radix == 36) &&
         "Radix should be 2, 8, 10, 16, or 36!");

  StringRef::iterator p = str.begin();
  size_t slen = str.ಗಾತ್ರ_3();
  ತಾರ್ಕಿಕ isNeg = *p == '-';
  ಆದರೆ (*p == '-' || *p == '+') {
    p++;
    slen--;
    ಪ್ರತಿಜ್ಞಾ(slen && "String is only a sign, needs a value.");
  }
  ಪ್ರತಿಜ್ಞಾ((slen <= numbits || radix != 2) && "Insufficient bit width");
  ಪ್ರತಿಜ್ಞಾ(((slen-1)*3 <= numbits || radix != 8) && "Insufficient bit width");
  ಪ್ರತಿಜ್ಞಾ(((slen-1)*4 <= numbits || radix != 16) && "Insufficient bit width");
  ಪ್ರತಿಜ್ಞಾ((((slen-1)*64)/22 <= numbits || radix != 10) &&
         "Insufficient bit width");

  // Allocate memory if needed
  ಆದರೆ (LLVM_LIKELY(is_Single_ಶಬ್ದ()))
    U.VAL = 0;
  ಇಲ್ಲದಿದ್ದರೆ
    U.pVal = get_Cleared_ಸ್ಮರಣೆ(get_ಅಂಕೆ_ಪದಗಳು());

  // Figure out if we can shift instead of multiply
  ಚಿಹ್ನೆರಹಿತ ಸರಿಸು = (radix == 16 ? 4 : radix == 8 ? 3 : radix == 2 ? 1 : 0);

  // Enter digit traversal loop
  ಗಾಗಿ (StringRef::iterator e = str.ಕೊನೆ(); p != e; ++p) {
    ಚಿಹ್ನೆರಹಿತ digit = getDigit(*p, radix);
    ಪ್ರತಿಜ್ಞಾ(digit < radix && "Invalid character in digit string");

    // Shift or multiply the value by the radix
    ಆದರೆ (slen > 1) {
      ಆದರೆ (ಸರಿಸು)
        *ಇದು <<= ಸರಿಸು;
      ಇಲ್ಲದಿದ್ದರೆ
        *ಇದು *= radix;
    }

    // Add in the digit we just interpreted
    *ಇದು += digit;
  }
  // If its negative, put it in two's complement form
  ಆದರೆ (isNeg)
    ಇದು->negate();
}

ಶೂನ್ಯ APInt::ಗಮ್ಯ_String(Small_ಸದಿಶ_Impl<ಅಕ್ಷರ> &Str, ಚಿಹ್ನೆರಹಿತ Radix, ತಾರ್ಕಿಕ Signed,
                     ತಾರ್ಕಿಕ formatAsCLiteral, ತಾರ್ಕಿಕ ಊರ್ಧ್ವ_Case_2,
                     ತಾರ್ಕಿಕ ಸೇರಿಸು_Separators) ಸ್ಥಿರಾಂಕ {
  ಪ್ರತಿಜ್ಞಾ((Radix == 10 || Radix == 8 || Radix == 16 || Radix == 2 ||
          Radix == 36) &&
         "Radix should be 2, 8, 10, 16, or 36!");

  ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *ಪೂರ್ವಪ್ರತ್ಯಯ = "";
  ಆದರೆ (formatAsCLiteral) {
    ಆಯ್ಕೆ (Radix) {
      ಸ್ಥಿತಿ 2:
        // Binary literals are a non-standard extension added in gcc 4.3:
        // http://gcc.gnu.org/onlinedocs/gcc-4.3.0/gcc/Binary-constants.html
        ಪೂರ್ವಪ್ರತ್ಯಯ = "0b";
        ನಿಲ್ಲಿಸು;
      ಸ್ಥಿತಿ 8:
        ಪೂರ್ವಪ್ರತ್ಯಯ = "0";
        ನಿಲ್ಲಿಸು;
      ಸ್ಥಿತಿ 10:
        ನಿಲ್ಲಿಸು; // No prefix
      ಸ್ಥಿತಿ 16:
        ಪೂರ್ವಪ್ರತ್ಯಯ = "0x";
        ನಿಲ್ಲಿಸು;
      ಪೂರ್ವನಿಯೋಜಿತ:
        llvm_unreachable("Invalid radix!");
    }
  }

  // Number of digits in a group between separators.
  ಚಿಹ್ನೆರಹಿತ Grouping = (Radix == 8 || Radix == 10) ? 3 : 4;

  // First, check for a zero value and just short circuit the logic below.
  ಆದರೆ (isZero()) {
    ಆಗ (*ಪೂರ್ವಪ್ರತ್ಯಯ) {
      Str.ತಳ್ಳು_ಹಿಂಭಾಗ(*ಪೂರ್ವಪ್ರತ್ಯಯ);
      ++ಪೂರ್ವಪ್ರತ್ಯಯ;
    };
    Str.ತಳ್ಳು_ಹಿಂಭಾಗ('0');
    ಹಿಂತಿರುಗಿ;
  }

  ಸ್ಥಿರ ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ BothDigits[] = "0123456789abcdefghijklmnopqrstuvwxyz"
                                   "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ";
  ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *Digits = BothDigits + (ಊರ್ಧ್ವ_Case_2 ? 36 : 0);

  ಆದರೆ (LLVM_LIKELY(is_Single_ಶಬ್ದ())) {
    ಅಕ್ಷರ ಬಫರ್[65];
    ಅಕ್ಷರ *BufPtr = std::ಕೊನೆ(ಬಫರ್);

    uint64_t N;
    ಆದರೆ (!Signed) {
      N = get_ZExt_ಮೌಲ್ಯ();
    } ಇಲ್ಲದಿದ್ದರೆ {
      int64_t I = get_SExt_ಮೌಲ್ಯ();
      ಆದರೆ (I >= 0) {
        N = I;
      } ಇಲ್ಲದಿದ್ದರೆ {
        Str.ತಳ್ಳು_ಹಿಂಭಾಗ('-');
        N = -(uint64_t)I;
      }
    }

    ಆಗ (*ಪೂರ್ವಪ್ರತ್ಯಯ) {
      Str.ತಳ್ಳು_ಹಿಂಭಾಗ(*ಪೂರ್ವಪ್ರತ್ಯಯ);
      ++ಪೂರ್ವಪ್ರತ್ಯಯ;
    };

    ಪೂರ್ಣಾಂಕ Pos = 0;
    ಆಗ (N) {
      ಆದರೆ (ಸೇರಿಸು_Separators && Pos % Grouping == 0 && Pos > 0)
        *--BufPtr = '\'';
      *--BufPtr = Digits[N % Radix];
      N /= Radix;
      Pos++;
    }
    Str.append(BufPtr, std::ಕೊನೆ(ಬಫರ್));
    ಹಿಂತಿರುಗಿ;
  }

  APInt Tmp(*ಇದು);

  ಆದರೆ (Signed && isNegative()) {
    // They want to print the signed version and it is a negative value
    // Flip the bits and add one to turn it into the equivalent positive
    // value and put a '-' in the result.
    Tmp.negate();
    Str.ತಳ್ಳು_ಹಿಂಭಾಗ('-');
  }

  ಆಗ (*ಪೂರ್ವಪ್ರತ್ಯಯ) {
    Str.ತಳ್ಳು_ಹಿಂಭಾಗ(*ಪೂರ್ವಪ್ರತ್ಯಯ);
    ++ಪೂರ್ವಪ್ರತ್ಯಯ;
  }

  // We insert the digits backward, then reverse them to get the right order.
  ಚಿಹ್ನೆರಹಿತ ಆರಂಭ_Dig = Str.ಗಾತ್ರ_3();

  // For the 2, 8 and 16 bit cases, we can just shift instead of divide
  // because the number of bits per digit (1, 3 and 4 respectively) divides
  // equally.  We just shift until the value is zero.
  ಆದರೆ (Radix == 2 || Radix == 8 || Radix == 16) {
    // Just shift tmp right for each digit width until it becomes zero
    ಚಿಹ್ನೆರಹಿತ ಸರಿಸು_Amt_2 = (Radix == 16 ? 4 : (Radix == 8 ? 3 : 1));
    ಚಿಹ್ನೆರಹಿತ ಮುಖವಾಡ_Amt = Radix - 1;

    ಪೂರ್ಣಾಂಕ Pos = 0;
    ಆಗ (Tmp.get_Bool_ಮೌಲ್ಯ()) {
      ಚಿಹ್ನೆರಹಿತ Digit = ಚಿಹ್ನೆರಹಿತ(Tmp.get_Raw_ದತ್ತಾಂಶ()[0]) & ಮುಖವಾಡ_Amt;
      ಆದರೆ (ಸೇರಿಸು_Separators && Pos % Grouping == 0 && Pos > 0)
        Str.ತಳ್ಳು_ಹಿಂಭಾಗ('\'');

      Str.ತಳ್ಳು_ಹಿಂಭಾಗ(Digits[Digit]);
      Tmp.lshr_ಒಳ_ಇರಿಸು(ಸರಿಸು_Amt_2);
      Pos++;
    }
  } ಇಲ್ಲದಿದ್ದರೆ {
    ಪೂರ್ಣಾಂಕ Pos = 0;
    ಆಗ (Tmp.get_Bool_ಮೌಲ್ಯ()) {
      uint64_t Digit;
      udivrem(Tmp, Radix, Tmp, Digit);
      ಪ್ರತಿಜ್ಞಾ(Digit < Radix && "divide failed");
      ಆದರೆ (ಸೇರಿಸು_Separators && Pos % Grouping == 0 && Pos > 0)
        Str.ತಳ್ಳು_ಹಿಂಭಾಗ('\'');

      Str.ತಳ್ಳು_ಹಿಂಭಾಗ(Digits[Digit]);
      Pos++;
    }
  }

  // Reverse the digits before returning.
  std::ಹಿಮ್ಮುಖ(Str.begin()+ಆರಂಭ_Dig, Str.ಕೊನೆ());
}

#ಆದರೆ !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)
LLVM_DUMP_ವಿಧಾನ ಶೂನ್ಯ APInt::dump() ಸ್ಥಿರಾಂಕ {
  SmallString<40> S, U;
  ಇದು->ಗಮ್ಯ_String_Unsigned(U);
  ಇದು->ಗಮ್ಯ_String_Signed(S);
  dbgs() << "APInt(" << ಬಿಟ್_ಅಗಲ_2 << "b, "
         << U << "u " << S << "s)\n";
}
#ಆದರೆ_ಅಂತ್ಯ

ಶೂನ್ಯ APInt::print(raw_ostream &OS, ತಾರ್ಕಿಕ isSigned) ಸ್ಥಿರಾಂಕ {
  SmallString<40> S;
  ಇದು->ಗಮ್ಯ_String(S, 10, isSigned, /* formatAsCLiteral = */ಸುಳ್ಳು);
  OS << S;
}

// This implements a variety of operations on a representation of
// arbitrary precision, two's-complement, bignum integer values.

// Assumed by lowHalf, highHalf, partMSB and partLSB.  A fairly safe
// and unrestricting assumption.
static_assert(APInt::APINT_ಬಿಟ್ಟುಗಳು_ಪ್ರತಿ_ಶಬ್ದ % 2 == 0,
              "Part width must be divisible by 2!");

// Returns the integer part with the least significant BITS set.
// BITS cannot be zero.
ಸ್ಥಿರ ಸಾಲೊಳಗೆ APInt::ಶಬ್ದ_Type ಕೆಳಗಿನ_ಬಿಟ್_ಮುಖವಾಡ(ಚಿಹ್ನೆರಹಿತ ಬಿಟ್ಟುಗಳು) {
  ಪ್ರತಿಜ್ಞಾ(ಬಿಟ್ಟುಗಳು != 0 && ಬಿಟ್ಟುಗಳು <= APInt::APINT_ಬಿಟ್ಟುಗಳು_ಪ್ರತಿ_ಶಬ್ದ);
  ಹಿಂತಿರುಗಿ ~(APInt::ಶಬ್ದ_Type) 0 >> (APInt::APINT_ಬಿಟ್ಟುಗಳು_ಪ್ರತಿ_ಶಬ್ದ - ಬಿಟ್ಟುಗಳು);
}

/// Returns the value of the lower half of PART.
ಸ್ಥಿರ ಸಾಲೊಳಗೆ APInt::ಶಬ್ದ_Type ಕೆಳಗಿನ_Half(APInt::ಶಬ್ದ_Type part) {
  ಹಿಂತಿರುಗಿ part & ಕೆಳಗಿನ_ಬಿಟ್_ಮುಖವಾಡ(APInt::APINT_ಬಿಟ್ಟುಗಳು_ಪ್ರತಿ_ಶಬ್ದ / 2);
}

/// Returns the value of the upper half of PART.
ಸ್ಥಿರ ಸಾಲೊಳಗೆ APInt::ಶಬ್ದ_Type ಮೇಲಿನ_Half(APInt::ಶಬ್ದ_Type part) {
  ಹಿಂತಿರುಗಿ part >> (APInt::APINT_ಬಿಟ್ಟುಗಳು_ಪ್ರತಿ_ಶಬ್ದ / 2);
}

/// Sets the least significant part of a bignum to the input value, and zeroes
/// out higher parts.
ಶೂನ್ಯ APInt::tc_ಗಣ(ಶಬ್ದ_Type *dst, ಶಬ್ದ_Type part, ಚಿಹ್ನೆರಹಿತ parts) {
  ಪ್ರತಿಜ್ಞಾ(parts > 0);
  dst[0] = part;
  ಗಾಗಿ (ಚಿಹ್ನೆರಹಿತ i = 1; i < parts; i++)
    dst[i] = 0;
}

/// Assign one bignum to another.
ಶೂನ್ಯ APInt::tc_ನಿಯೋಜಿಸು(ಶಬ್ದ_Type *dst, ಸ್ಥಿರಾಂಕ ಶಬ್ದ_Type *src, ಚಿಹ್ನೆರಹಿತ parts) {
  ಗಾಗಿ (ಚಿಹ್ನೆರಹಿತ i = 0; i < parts; i++)
    dst[i] = src[i];
}

/// Returns true if a bignum is zero, false otherwise.
ತಾರ್ಕಿಕ APInt::tcIsZero(ಸ್ಥಿರಾಂಕ ಶಬ್ದ_Type *src, ಚಿಹ್ನೆರಹಿತ parts) {
  ಗಾಗಿ (ಚಿಹ್ನೆರಹಿತ i = 0; i < parts; i++)
    ಆದರೆ (src[i])
      ಹಿಂತಿರುಗಿ ಸುಳ್ಳು;

  ಹಿಂತಿರುಗಿ ನಿಜ;
}

/// Extract the given bit of a bignum; returns 0 or 1.
ಪೂರ್ಣಾಂಕ APInt::tc_ಹೊರತೆಗೆ_ಬಿಟ್(ಸ್ಥಿರಾಂಕ ಶಬ್ದ_Type *parts, ಚಿಹ್ನೆರಹಿತ ಬಿಟ್) {
  ಹಿಂತಿರುಗಿ (parts[which_ಶಬ್ದ(ಬಿಟ್)] & ಮುಖವಾಡ_ಬಿಟ್(ಬಿಟ್)) != 0;
}

/// Set the given bit of a bignum.
ಶೂನ್ಯ APInt::tc_ಗಣ_ಬಿಟ್(ಶಬ್ದ_Type *parts, ಚಿಹ್ನೆರಹಿತ ಬಿಟ್) {
  parts[which_ಶಬ್ದ(ಬಿಟ್)] |= ಮುಖವಾಡ_ಬಿಟ್(ಬಿಟ್);
}

/// Clears the given bit of a bignum.
ಶೂನ್ಯ APInt::tc_Clear_ಬಿಟ್(ಶಬ್ದ_Type *parts, ಚಿಹ್ನೆರಹಿತ ಬಿಟ್) {
  parts[which_ಶಬ್ದ(ಬಿಟ್)] &= ~ಮುಖವಾಡ_ಬಿಟ್(ಬಿಟ್);
}

/// Returns the bit number of the least significant set bit of a number.  If the
/// input number has no bits set UINT_MAX is returned.
ಚಿಹ್ನೆರಹಿತ APInt::tcLSB(ಸ್ಥಿರಾಂಕ ಶಬ್ದ_Type *parts, ಚಿಹ್ನೆರಹಿತ n) {
  ಗಾಗಿ (ಚಿಹ್ನೆರಹಿತ i = 0; i < n; i++) {
    ಆದರೆ (parts[i] != 0) {
      ಚಿಹ್ನೆರಹಿತ lsb = llvm::countr_zero(parts[i]);
      ಹಿಂತಿರುಗಿ lsb + i * APINT_ಬಿಟ್ಟುಗಳು_ಪ್ರತಿ_ಶಬ್ದ;
    }
  }

  ಹಿಂತಿರುಗಿ UINT_MAX;
}

/// Returns the bit number of the most significant set bit of a number.
/// If the input number has no bits set UINT_MAX is returned.
ಚಿಹ್ನೆರಹಿತ APInt::tcMSB(ಸ್ಥಿರಾಂಕ ಶಬ್ದ_Type *parts, ಚಿಹ್ನೆರಹಿತ n) {
  ಮಾಡು {
    --n;

    ಆದರೆ (parts[n] != 0) {
      static_assert(ಗಾತ್ರ(parts[n]) <= ಗಾತ್ರ(uint64_t));
      ಚಿಹ್ನೆರಹಿತ msb = llvm::Log2_64(parts[n]);

      ಹಿಂತಿರುಗಿ msb + n * APINT_ಬಿಟ್ಟುಗಳು_ಪ್ರತಿ_ಶಬ್ದ;
    }
  } ಆಗ (n);

  ಹಿಂತಿರುಗಿ UINT_MAX;
}

/// Copy the bit vector of width srcBITS from SRC, starting at bit srcLSB, to
/// DST, of dstCOUNT parts, such that the bit srcLSB becomes the least
/// significant bit of DST.  All high bits above srcBITS in DST are zero-filled.
/// */
ಶೂನ್ಯ
APInt::tc_ಹೊರತೆಗೆ(ಶಬ್ದ_Type *dst, ಚಿಹ್ನೆರಹಿತ dst_ಎಣಿಕೆ, ಸ್ಥಿರಾಂಕ ಶಬ್ದ_Type *src,
                 ಚಿಹ್ನೆರಹಿತ src_ಬಿಟ್ಟುಗಳು, ಚಿಹ್ನೆರಹಿತ srcLSB) {
  ಚಿಹ್ನೆರಹಿತ dstParts = (src_ಬಿಟ್ಟುಗಳು + APINT_ಬಿಟ್ಟುಗಳು_ಪ್ರತಿ_ಶಬ್ದ - 1) / APINT_ಬಿಟ್ಟುಗಳು_ಪ್ರತಿ_ಶಬ್ದ;
  ಪ್ರತಿಜ್ಞಾ(dstParts <= dst_ಎಣಿಕೆ);

  ಚಿಹ್ನೆರಹಿತ ಮೊದಲ_Src_Part = srcLSB / APINT_ಬಿಟ್ಟುಗಳು_ಪ್ರತಿ_ಶಬ್ದ;
  tc_ನಿಯೋಜಿಸು(dst, src + ಮೊದಲ_Src_Part, dstParts);

  ಚಿಹ್ನೆರಹಿತ ಸರಿಸು = srcLSB % APINT_ಬಿಟ್ಟುಗಳು_ಪ್ರತಿ_ಶಬ್ದ;
  tc_ಸರಿಸು_ಬಲ(dst, dstParts, ಸರಿಸು);

  // We now have (dstParts * APINT_BITS_PER_WORD - shift) bits from SRC
  // in DST.  If this is less that srcBits, append the rest, else
  // clear the high bits.
  ಚಿಹ್ನೆರಹಿತ n = dstParts * APINT_ಬಿಟ್ಟುಗಳು_ಪ್ರತಿ_ಶಬ್ದ - ಸರಿಸು;
  ಆದರೆ (n < src_ಬಿಟ್ಟುಗಳು) {
    ಶಬ್ದ_Type ಮುಖವಾಡ_2 = ಕೆಳಗಿನ_ಬಿಟ್_ಮುಖವಾಡ (src_ಬಿಟ್ಟುಗಳು - n);
    dst[dstParts - 1] |= ((src[ಮೊದಲ_Src_Part + dstParts] & ಮುಖವಾಡ_2)
                          << n % APINT_ಬಿಟ್ಟುಗಳು_ಪ್ರತಿ_ಶಬ್ದ);
  } ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (n > src_ಬಿಟ್ಟುಗಳು) {
    ಆದರೆ (src_ಬಿಟ್ಟುಗಳು % APINT_ಬಿಟ್ಟುಗಳು_ಪ್ರತಿ_ಶಬ್ದ)
      dst[dstParts - 1] &= ಕೆಳಗಿನ_ಬಿಟ್_ಮುಖವಾಡ (src_ಬಿಟ್ಟುಗಳು % APINT_ಬಿಟ್ಟುಗಳು_ಪ್ರತಿ_ಶಬ್ದ);
  }

  // Clear high parts.
  ಆಗ (dstParts < dst_ಎಣಿಕೆ)
    dst[dstParts++] = 0;
}

//// DST += RHS + C where C is zero or one.  Returns the carry flag.
APInt::ಶಬ್ದ_Type APInt::tc_ಕೂಡಿಸು(ಶಬ್ದ_Type *dst, ಸ್ಥಿರಾಂಕ ಶಬ್ದ_Type *rhs,
                             ಶಬ್ದ_Type c, ಚಿಹ್ನೆರಹಿತ parts) {
  ಪ್ರತಿಜ್ಞಾ(c <= 1);

  ಗಾಗಿ (ಚಿಹ್ನೆರಹಿತ i = 0; i < parts; i++) {
    ಶಬ್ದ_Type l = dst[i];
    ಆದರೆ (c) {
      dst[i] += rhs[i] + 1;
      c = (dst[i] <= l);
    } ಇಲ್ಲದಿದ್ದರೆ {
      dst[i] += rhs[i];
      c = (dst[i] < l);
    }
  }

  ಹಿಂತಿರುಗಿ c;
}

/// This function adds a single "word" integer, src, to the multiple
/// "word" integer array, dst[]. dst[] is modified to reflect the addition and
/// 1 is returned if there is a carry out, otherwise 0 is returned.
/// @returns the carry of the addition.
APInt::ಶಬ್ದ_Type APInt::tc_ಕೂಡಿಸು_Part(ಶಬ್ದ_Type *dst, ಶಬ್ದ_Type src,
                                 ಚಿಹ್ನೆರಹಿತ parts) {
  ಗಾಗಿ (ಚಿಹ್ನೆರಹಿತ i = 0; i < parts; ++i) {
    dst[i] += src;
    ಆದರೆ (dst[i] >= src)
      ಹಿಂತಿರುಗಿ 0; // No need to carry so exit early.
    src = 1; // Carry one to next digit.
  }

  ಹಿಂತಿರುಗಿ 1;
}

/// DST -= RHS + C where C is zero or one.  Returns the carry flag.
APInt::ಶಬ್ದ_Type APInt::tc_ಕಳೆ(ಶಬ್ದ_Type *dst, ಸ್ಥಿರಾಂಕ ಶಬ್ದ_Type *rhs,
                                  ಶಬ್ದ_Type c, ಚಿಹ್ನೆರಹಿತ parts) {
  ಪ್ರತಿಜ್ಞಾ(c <= 1);

  ಗಾಗಿ (ಚಿಹ್ನೆರಹಿತ i = 0; i < parts; i++) {
    ಶಬ್ದ_Type l = dst[i];
    ಆದರೆ (c) {
      dst[i] -= rhs[i] + 1;
      c = (dst[i] >= l);
    } ಇಲ್ಲದಿದ್ದರೆ {
      dst[i] -= rhs[i];
      c = (dst[i] > l);
    }
  }

  ಹಿಂತಿರುಗಿ c;
}

/// This function subtracts a single "word" (64-bit word), src, from
/// the multi-word integer array, dst[], propagating the borrowed 1 value until
/// no further borrowing is needed or it runs out of "words" in dst.  The result
/// is 1 if "borrowing" exhausted the digits in dst, or 0 if dst was not
/// exhausted. In other words, if src > dst then this function returns 1,
/// otherwise 0.
/// @returns the borrow out of the subtraction
APInt::ಶಬ್ದ_Type APInt::tc_ಕಳೆ_Part(ಶಬ್ದ_Type *dst, ಶಬ್ದ_Type src,
                                      ಚಿಹ್ನೆರಹಿತ parts) {
  ಗಾಗಿ (ಚಿಹ್ನೆರಹಿತ i = 0; i < parts; ++i) {
    ಶಬ್ದ_Type Dst = dst[i];
    dst[i] -= src;
    ಆದರೆ (src <= Dst)
      ಹಿಂತಿರುಗಿ 0; // No need to borrow so exit early.
    src = 1; // We have to "borrow 1" from next "word"
  }

  ಹಿಂತಿರುಗಿ 1;
}

/// Negate a bignum in-place.
ಶೂನ್ಯ APInt::tcNegate(ಶಬ್ದ_Type *dst, ಚಿಹ್ನೆರಹಿತ parts) {
  tcComplement(dst, parts);
  tcIncrement(dst, parts);
}

/// DST += SRC * MULTIPLIER + CARRY   if add is true
/// DST  = SRC * MULTIPLIER + CARRY   if add is false
/// Requires 0 <= DSTPARTS <= SRCPARTS + 1.  If DST overlaps SRC
/// they must start at the same point, i.e. DST == SRC.
/// If DSTPARTS == SRCPARTS + 1 no overflow occurs and zero is
/// returned.  Otherwise DST is filled with the least significant
/// DSTPARTS parts of the result, and if all of the omitted higher
/// parts were zero return zero, otherwise overflow occurred and
/// return one.
ಪೂರ್ಣಾಂಕ APInt::tcMultiplyPart(ಶಬ್ದ_Type *dst, ಸ್ಥಿರಾಂಕ ಶಬ್ದ_Type *src,
                          ಶಬ್ದ_Type multiplier, ಶಬ್ದ_Type ವಾಹಕ,
                          ಚಿಹ್ನೆರಹಿತ srcParts, ಚಿಹ್ನೆರಹಿತ dstParts,
                          ತಾರ್ಕಿಕ ಕೂಡಿಸು) {
  // Otherwise our writes of DST kill our later reads of SRC.
  ಪ್ರತಿಜ್ಞಾ(dst <= src || dst >= src + srcParts);
  ಪ್ರತಿಜ್ಞಾ(dstParts <= srcParts + 1);

  // N loops; minimum of dstParts and srcParts.
  ಚಿಹ್ನೆರಹಿತ n = std::min(dstParts, srcParts);

  ಗಾಗಿ (ಚಿಹ್ನೆರಹಿತ i = 0; i < n; i++) {
    // [LOW, HIGH] = MULTIPLIER * SRC[i] + DST[i] + CARRY.
    // This cannot overflow, because:
    //   (n - 1) * (n - 1) + 2 (n - 1) = (n - 1) * (n + 1)
    // which is less than n^2.
    ಶಬ್ದ_Type srcPart = src[i];
    ಶಬ್ದ_Type ಕೆಳಗಿನ, mid, ಮೇಲಿನ;
    ಆದರೆ (multiplier == 0 || srcPart == 0) {
      ಕೆಳಗಿನ = ವಾಹಕ;
      ಮೇಲಿನ = 0;
    } ಇಲ್ಲದಿದ್ದರೆ {
      ಕೆಳಗಿನ = ಕೆಳಗಿನ_Half(srcPart) * ಕೆಳಗಿನ_Half(multiplier);
      ಮೇಲಿನ = ಮೇಲಿನ_Half(srcPart) * ಮೇಲಿನ_Half(multiplier);

      mid = ಕೆಳಗಿನ_Half(srcPart) * ಮೇಲಿನ_Half(multiplier);
      ಮೇಲಿನ += ಮೇಲಿನ_Half(mid);
      mid <<= APINT_ಬಿಟ್ಟುಗಳು_ಪ್ರತಿ_ಶಬ್ದ / 2;
      ಆದರೆ (ಕೆಳಗಿನ + mid < ಕೆಳಗಿನ)
        ಮೇಲಿನ++;
      ಕೆಳಗಿನ += mid;

      mid = ಮೇಲಿನ_Half(srcPart) * ಕೆಳಗಿನ_Half(multiplier);
      ಮೇಲಿನ += ಮೇಲಿನ_Half(mid);
      mid <<= APINT_ಬಿಟ್ಟುಗಳು_ಪ್ರತಿ_ಶಬ್ದ / 2;
      ಆದರೆ (ಕೆಳಗಿನ + mid < ಕೆಳಗಿನ)
        ಮೇಲಿನ++;
      ಕೆಳಗಿನ += mid;

      // Now add carry.
      ಆದರೆ (ಕೆಳಗಿನ + ವಾಹಕ < ಕೆಳಗಿನ)
        ಮೇಲಿನ++;
      ಕೆಳಗಿನ += ವಾಹಕ;
    }

    ಆದರೆ (ಕೂಡಿಸು) {
      // And now DST[i], and store the new low part there.
      ಆದರೆ (ಕೆಳಗಿನ + dst[i] < ಕೆಳಗಿನ)
        ಮೇಲಿನ++;
      dst[i] += ಕೆಳಗಿನ;
    } ಇಲ್ಲದಿದ್ದರೆ {
      dst[i] = ಕೆಳಗಿನ;
    }

    ವಾಹಕ = ಮೇಲಿನ;
  }

  ಆದರೆ (srcParts < dstParts) {
    // Full multiplication, there is no overflow.
    ಪ್ರತಿಜ್ಞಾ(srcParts + 1 == dstParts);
    dst[srcParts] = ವಾಹಕ;
    ಹಿಂತಿರುಗಿ 0;
  }

  // We overflowed if there is carry.
  ಆದರೆ (ವಾಹಕ)
    ಹಿಂತಿರುಗಿ 1;

  // We would overflow if any significant unwritten parts would be
  // non-zero.  This is true if any remaining src parts are non-zero
  // and the multiplier is non-zero.
  ಆದರೆ (multiplier)
    ಗಾಗಿ (ಚಿಹ್ನೆರಹಿತ i = dstParts; i < srcParts; i++)
      ಆದರೆ (src[i])
        ಹಿಂತಿರುಗಿ 1;

  // We fitted in the narrow destination.
  ಹಿಂತಿರುಗಿ 0;
}

/// DST = LHS * RHS, where DST has the same width as the operands and
/// is filled with the least significant parts of the result.  Returns
/// one if overflow occurred, otherwise zero.  DST must be disjoint
/// from both operands.
ಪೂರ್ಣಾಂಕ APInt::tcMultiply(ಶಬ್ದ_Type *dst, ಸ್ಥಿರಾಂಕ ಶಬ್ದ_Type *lhs,
                      ಸ್ಥಿರಾಂಕ ಶಬ್ದ_Type *rhs, ಚಿಹ್ನೆರಹಿತ parts) {
  ಪ್ರತಿಜ್ಞಾ(dst != lhs && dst != rhs);

  ಪೂರ್ಣಾಂಕ ಅತಿಪ್ರವಾಹ_2 = 0;

  ಗಾಗಿ (ಚಿಹ್ನೆರಹಿತ i = 0; i < parts; i++) {
    // Don't accumulate on the first iteration so we don't need to initalize
    // dst to 0.
    ಅತಿಪ್ರವಾಹ_2 |=
        tcMultiplyPart(&dst[i], lhs, rhs[i], 0, parts, parts - i, i != 0);
  }

  ಹಿಂತಿರುಗಿ ಅತಿಪ್ರವಾಹ_2;
}

/// DST = LHS * RHS, where DST has width the sum of the widths of the
/// operands. No overflow occurs. DST must be disjoint from both operands.
ಶೂನ್ಯ APInt::tc_ತುಂಬಿದ_Multiply(ಶಬ್ದ_Type *dst, ಸ್ಥಿರಾಂಕ ಶಬ್ದ_Type *lhs,
                           ಸ್ಥಿರಾಂಕ ಶಬ್ದ_Type *rhs, ಚಿಹ್ನೆರಹಿತ lhsParts,
                           ಚಿಹ್ನೆರಹಿತ rhsParts) {
  // Put the narrower number on the LHS for less loops below.
  ಆದರೆ (lhsParts > rhsParts)
    ಹಿಂತಿರುಗಿ tc_ತುಂಬಿದ_Multiply (dst, rhs, lhs, rhsParts, lhsParts);

  ಪ್ರತಿಜ್ಞಾ(dst != lhs && dst != rhs);

  ಗಾಗಿ (ಚಿಹ್ನೆರಹಿತ i = 0; i < lhsParts; i++) {
    // Don't accumulate on the first iteration so we don't need to initalize
    // dst to 0.
    tcMultiplyPart(&dst[i], rhs, lhs[i], 0, rhsParts, rhsParts + 1, i != 0);
  }
}

// If RHS is zero LHS and REMAINDER are left unchanged, return one.
// Otherwise set LHS to LHS / RHS with the fractional part discarded,
// set REMAINDER to the remainder, return zero.  i.e.
//
//   OLD_LHS = RHS * LHS + REMAINDER
//
// SCRATCH is a bignum of the same size as the operands and result for
// use by the routine; its contents need not be initialized and are
// destroyed.  LHS, REMAINDER and SCRATCH must be distinct.
ಪೂರ್ಣಾಂಕ APInt::tcDivide(ಶಬ್ದ_Type *lhs, ಸ್ಥಿರಾಂಕ ಶಬ್ದ_Type *rhs,
                    ಶಬ್ದ_Type *remainder, ಶಬ್ದ_Type *srhs,
                    ಚಿಹ್ನೆರಹಿತ parts) {
  ಪ್ರತಿಜ್ಞಾ(lhs != remainder && lhs != srhs && remainder != srhs);

  ಚಿಹ್ನೆರಹಿತ ಸರಿಸು_ಎಣಿಕೆ = tcMSB(rhs, parts) + 1;
  ಆದರೆ (ಸರಿಸು_ಎಣಿಕೆ == 0)
    ಹಿಂತಿರುಗಿ ನಿಜ;

  ಸರಿಸು_ಎಣಿಕೆ = parts * APINT_ಬಿಟ್ಟುಗಳು_ಪ್ರತಿ_ಶಬ್ದ - ಸರಿಸು_ಎಣಿಕೆ;
  ಚಿಹ್ನೆರಹಿತ n = ಸರಿಸು_ಎಣಿಕೆ / APINT_ಬಿಟ್ಟುಗಳು_ಪ್ರತಿ_ಶಬ್ದ;
  ಶಬ್ದ_Type ಮುಖವಾಡ_2 = (ಶಬ್ದ_Type) 1 << (ಸರಿಸು_ಎಣಿಕೆ % APINT_ಬಿಟ್ಟುಗಳು_ಪ್ರತಿ_ಶಬ್ದ);

  tc_ನಿಯೋಜಿಸು(srhs, rhs, parts);
  tc_ಸರಿಸು_ಎಡ(srhs, parts, ಸರಿಸು_ಎಣಿಕೆ);
  tc_ನಿಯೋಜಿಸು(remainder, lhs, parts);
  tc_ಗಣ(lhs, 0, parts);

  // Loop, subtracting SRHS if REMAINDER is greater and adding that to the
  // total.
  ಗಾಗಿ (;;) {
    ಪೂರ್ಣಾಂಕ compare = tcCompare(remainder, srhs, parts);
    ಆದರೆ (compare >= 0) {
      tc_ಕಳೆ(remainder, srhs, 0, parts);
      lhs[n] |= ಮುಖವಾಡ_2;
    }

    ಆದರೆ (ಸರಿಸು_ಎಣಿಕೆ == 0)
      ನಿಲ್ಲಿಸು;
    ಸರಿಸು_ಎಣಿಕೆ--;
    tc_ಸರಿಸು_ಬಲ(srhs, parts, 1);
    ಆದರೆ ((ಮುಖವಾಡ_2 >>= 1) == 0) {
      ಮುಖವಾಡ_2 = (ಶಬ್ದ_Type) 1 << (APINT_ಬಿಟ್ಟುಗಳು_ಪ್ರತಿ_ಶಬ್ದ - 1);
      n--;
    }
  }

  ಹಿಂತಿರುಗಿ ಸುಳ್ಳು;
}

/// Shift a bignum left Count bits in-place. Shifted in bits are zero. There are
/// no restrictions on Count.
ಶೂನ್ಯ APInt::tc_ಸರಿಸು_ಎಡ(ಶಬ್ದ_Type *Dst, ಚಿಹ್ನೆರಹಿತ ಪದಗಳು_2, ಚಿಹ್ನೆರಹಿತ ಎಣಿಕೆ_3) {
  // Don't bother performing a no-op shift.
  ಆದರೆ (!ಎಣಿಕೆ_3)
    ಹಿಂತಿರುಗಿ;

  // WordShift is the inter-part shift; BitShift is the intra-part shift.
  ಚಿಹ್ನೆರಹಿತ ಶಬ್ದ_ಸರಿಸು = std::min(ಎಣಿಕೆ_3 / APINT_ಬಿಟ್ಟುಗಳು_ಪ್ರತಿ_ಶಬ್ದ, ಪದಗಳು_2);
  ಚಿಹ್ನೆರಹಿತ ಬಿಟ್_ಸರಿಸು = ಎಣಿಕೆ_3 % APINT_ಬಿಟ್ಟುಗಳು_ಪ್ರತಿ_ಶಬ್ದ;

  // Fastpath for moving by whole words.
  ಆದರೆ (ಬಿಟ್_ಸರಿಸು == 0) {
    std::memmove(Dst + ಶಬ್ದ_ಸರಿಸು, Dst, (ಪದಗಳು_2 - ಶಬ್ದ_ಸರಿಸು) * APINT_ಶಬ್ದ_ಗಾತ್ರ);
  } ಇಲ್ಲದಿದ್ದರೆ {
    ಆಗ (ಪದಗಳು_2-- > ಶಬ್ದ_ಸರಿಸು) {
      Dst[ಪದಗಳು_2] = Dst[ಪದಗಳು_2 - ಶಬ್ದ_ಸರಿಸು] << ಬಿಟ್_ಸರಿಸು;
      ಆದರೆ (ಪದಗಳು_2 > ಶಬ್ದ_ಸರಿಸು)
        Dst[ಪದಗಳು_2] |=
          Dst[ಪದಗಳು_2 - ಶಬ್ದ_ಸರಿಸು - 1] >> (APINT_ಬಿಟ್ಟುಗಳು_ಪ್ರತಿ_ಶಬ್ದ - ಬಿಟ್_ಸರಿಸು);
    }
  }

  // Fill in the remainder with 0s.
  std::memset(Dst, 0, ಶಬ್ದ_ಸರಿಸು * APINT_ಶಬ್ದ_ಗಾತ್ರ);
}

/// Shift a bignum right Count bits in-place. Shifted in bits are zero. There
/// are no restrictions on Count.
ಶೂನ್ಯ APInt::tc_ಸರಿಸು_ಬಲ(ಶಬ್ದ_Type *Dst, ಚಿಹ್ನೆರಹಿತ ಪದಗಳು_2, ಚಿಹ್ನೆರಹಿತ ಎಣಿಕೆ_3) {
  // Don't bother performing a no-op shift.
  ಆದರೆ (!ಎಣಿಕೆ_3)
    ಹಿಂತಿರುಗಿ;

  // WordShift is the inter-part shift; BitShift is the intra-part shift.
  ಚಿಹ್ನೆರಹಿತ ಶಬ್ದ_ಸರಿಸು = std::min(ಎಣಿಕೆ_3 / APINT_ಬಿಟ್ಟುಗಳು_ಪ್ರತಿ_ಶಬ್ದ, ಪದಗಳು_2);
  ಚಿಹ್ನೆರಹಿತ ಬಿಟ್_ಸರಿಸು = ಎಣಿಕೆ_3 % APINT_ಬಿಟ್ಟುಗಳು_ಪ್ರತಿ_ಶಬ್ದ;

  ಚಿಹ್ನೆರಹಿತ ಪದಗಳು_ಗಮ್ಯ_Move = ಪದಗಳು_2 - ಶಬ್ದ_ಸರಿಸು;
  // Fastpath for moving by whole words.
  ಆದರೆ (ಬಿಟ್_ಸರಿಸು == 0) {
    std::memmove(Dst, Dst + ಶಬ್ದ_ಸರಿಸು, ಪದಗಳು_ಗಮ್ಯ_Move * APINT_ಶಬ್ದ_ಗಾತ್ರ);
  } ಇಲ್ಲದಿದ್ದರೆ {
    ಗಾಗಿ (ಚಿಹ್ನೆರಹಿತ i = 0; i != ಪದಗಳು_ಗಮ್ಯ_Move; ++i) {
      Dst[i] = Dst[i + ಶಬ್ದ_ಸರಿಸು] >> ಬಿಟ್_ಸರಿಸು;
      ಆದರೆ (i + 1 != ಪದಗಳು_ಗಮ್ಯ_Move)
        Dst[i] |= Dst[i + ಶಬ್ದ_ಸರಿಸು + 1] << (APINT_ಬಿಟ್ಟುಗಳು_ಪ್ರತಿ_ಶಬ್ದ - ಬಿಟ್_ಸರಿಸು);
    }
  }

  // Fill in the remainder with 0s.
  std::memset(Dst + ಪದಗಳು_ಗಮ್ಯ_Move, 0, ಶಬ್ದ_ಸರಿಸು * APINT_ಶಬ್ದ_ಗಾತ್ರ);
}

// Comparison (unsigned) of two bignums.
ಪೂರ್ಣಾಂಕ APInt::tcCompare(ಸ್ಥಿರಾಂಕ ಶಬ್ದ_Type *lhs, ಸ್ಥಿರಾಂಕ ಶಬ್ದ_Type *rhs,
                     ಚಿಹ್ನೆರಹಿತ parts) {
  ಆಗ (parts) {
    parts--;
    ಆದರೆ (lhs[parts] != rhs[parts])
      ಹಿಂತಿರುಗಿ (lhs[parts] > rhs[parts]) ? 1 : -1;
  }

  ಹಿಂತಿರುಗಿ 0;
}

APInt llvm::APIntOps::RoundingUDiv(ಸ್ಥಿರಾಂಕ APInt &A, ಸ್ಥಿರಾಂಕ APInt &B,
                                   APInt::Rounding RM) {
  // Currently udivrem always rounds down.
  ಆಯ್ಕೆ (RM) {
  ಸ್ಥಿತಿ APInt::Rounding::ಕೆಳಕ್ಕೆ:
  ಸ್ಥಿತಿ APInt::Rounding::TOWARD_ZERO:
    ಹಿಂತಿರುಗಿ A.udiv(B);
  ಸ್ಥಿತಿ APInt::Rounding::ಮೇಲೆ: {
    APInt Quo, Rem;
    APInt::udivrem(A, B, Quo, Rem);
    ಆದರೆ (Rem.isZero())
      ಹಿಂತಿರುಗಿ Quo;
    ಹಿಂತಿರುಗಿ Quo + 1;
  }
  }
  llvm_unreachable("Unknown APInt::Rounding enum");
}

APInt llvm::APIntOps::RoundingSDiv(ಸ್ಥಿರಾಂಕ APInt &A, ಸ್ಥಿರಾಂಕ APInt &B,
                                   APInt::Rounding RM) {
  ಆಯ್ಕೆ (RM) {
  ಸ್ಥಿತಿ APInt::Rounding::ಕೆಳಕ್ಕೆ:
  ಸ್ಥಿತಿ APInt::Rounding::ಮೇಲೆ: {
    APInt Quo, Rem;
    APInt::sdivrem(A, B, Quo, Rem);
    ಆದರೆ (Rem.isZero())
      ಹಿಂತಿರುಗಿ Quo;
    // This algorithm deals with arbitrary rounding mode used by sdivrem.
    // We want to check whether the non-integer part of the mathematical value
    // is negative or not. If the non-integer part is negative, we need to round
    // down from Quo; otherwise, if it's positive or 0, we return Quo, as it's
    // already rounded down.
    ಆದರೆ (RM == APInt::Rounding::ಕೆಳಕ್ಕೆ) {
      ಆದರೆ (Rem.isNegative() != B.isNegative())
        ಹಿಂತಿರುಗಿ Quo - 1;
      ಹಿಂತಿರುಗಿ Quo;
    }
    ಆದರೆ (Rem.isNegative() != B.isNegative())
      ಹಿಂತಿರುಗಿ Quo;
    ಹಿಂತಿರುಗಿ Quo + 1;
  }
  // Currently sdiv rounds towards zero.
  ಸ್ಥಿತಿ APInt::Rounding::TOWARD_ZERO:
    ಹಿಂತಿರುಗಿ A.sdiv(B);
  }
  llvm_unreachable("Unknown APInt::Rounding enum");
}

std::optional<APInt>
llvm::APIntOps::SolveQuadraticEquationWrap(APInt A, APInt B, APInt C,
                                           ಚಿಹ್ನೆರಹಿತ Range_ಅಗಲ) {
  ಚಿಹ್ನೆರಹಿತ Coeff_ಅಗಲ = A.get_ಬಿಟ್_ಅಗಲ();
  ಪ್ರತಿಜ್ಞಾ(Coeff_ಅಗಲ == B.get_ಬಿಟ್_ಅಗಲ() && Coeff_ಅಗಲ == C.get_ಬಿಟ್_ಅಗಲ());
  ಪ್ರತಿಜ್ಞಾ(Range_ಅಗಲ <= Coeff_ಅಗಲ &&
         "Value range width should be less than coefficient width");
  ಪ್ರತಿಜ್ಞಾ(Range_ಅಗಲ > 1 && "Value range bit width should be > 1");

  LLVM_DEBUG(dbgs() << __func__ << ": solving " << A << "x^2 + " << B
                    << "x + " << C << ", rw:" << Range_ಅಗಲ << '\n');

  // Identify 0 as a (non)solution immediately.
  ಆದರೆ (C.sextOrTrunc(Range_ಅಗಲ).isZero()) {
    LLVM_DEBUG(dbgs() << __func__ << ": zero solution\n");
    ಹಿಂತಿರುಗಿ APInt(Coeff_ಅಗಲ, 0);
  }

  // The result of APInt arithmetic has the same bit width as the operands,
  // so it can actually lose high bits. A product of two n-bit integers needs
  // 2n-1 bits to represent the full value.
  // The operation done below (on quadratic coefficients) that can produce
  // the largest value is the evaluation of the equation during bisection,
  // which needs 3 times the bitwidth of the coefficient, so the total number
  // of required bits is 3n.
  //
  // The purpose of this extension is to simulate the set Z of all integers,
  // where n+1 > n for all n in Z. In Z it makes sense to talk about positive
  // and negative numbers (not so much in a modulo arithmetic). The method
  // used to solve the equation is based on the standard formula for real
  // numbers, and uses the concepts of "positive" and "negative" with their
  // usual meanings.
  Coeff_ಅಗಲ *= 3;
  A = A.sext(Coeff_ಅಗಲ);
  B = B.sext(Coeff_ಅಗಲ);
  C = C.sext(Coeff_ಅಗಲ);

  // Make A > 0 for simplicity. Negate cannot overflow at this point because
  // the bit width has increased.
  ಆದರೆ (A.isNegative()) {
    A.negate();
    B.negate();
    C.negate();
  }

  // Solving an equation q(x) = 0 with coefficients in modular arithmetic
  // is really solving a set of equations q(x) = kR for k = 0, 1, 2, ...,
  // and R = 2^BitWidth.
  // Since we're trying not only to find exact solutions, but also values
  // that "wrap around", such a set will always have a solution, i.e. an x
  // that satisfies at least one of the equations, or such that |q(x)|
  // exceeds kR, while |q(x-1)| for the same k does not.
  //
  // We need to find a value k, such that Ax^2 + Bx + C = kR will have a
  // positive solution n (in the above sense), and also such that the n
  // will be the least among all solutions corresponding to k = 0, 1, ...
  // (more precisely, the least element in the set
  //   { n(k) | k is such that a solution n(k) exists }).
  //
  // Consider the parabola (over real numbers) that corresponds to the
  // quadratic equation. Since A > 0, the arms of the parabola will point
  // up. Picking different values of k will shift it up and down by R.
  //
  // We want to shift the parabola in such a way as to reduce the problem
  // of solving q(x) = kR to solving shifted_q(x) = 0.
  // (The interesting solutions are the ceilings of the real number
  // solutions.)
  APInt R = APInt::get_One_ಬಿಟ್_ಗಣ(Coeff_ಅಗಲ, Range_ಅಗಲ);
  APInt TwoA = 2 * A;
  APInt SqrB = B * B;
  ತಾರ್ಕಿಕ Pick_ಕೆಳಗಿನ;

  ಸ್ವಯಂಚಾಲಿತ ಸುತ್ತು_ಮೇಲೆ_2 = [] (ಸ್ಥಿರಾಂಕ APInt &V, ಸ್ಥಿರಾಂಕ APInt &A) -> APInt {
    ಪ್ರತಿಜ್ಞಾ(A.is_Strictly_ಧನಾತ್ಮಕ());
    APInt T = V.ನಿರಪೇಕ್ಷ().urem(A);
    ಆದರೆ (T.isZero())
      ಹಿಂತಿರುಗಿ V;
    ಹಿಂತಿರುಗಿ V.isNegative() ? V+T : V+(A-T);
  };

  // The vertex of the parabola is at -B/2A, but since A > 0, it's negative
  // iff B is positive.
  ಆದರೆ (B.isNonNegative()) {
    // If B >= 0, the vertex it at a negative location (or at 0), so in
    // order to have a non-negative solution we need to pick k that makes
    // C-kR negative. To satisfy all the requirements for the solution
    // that we are looking for, it needs to be closest to 0 of all k.
    C = C.srem(R);
    ಆದರೆ (C.is_Strictly_ಧನಾತ್ಮಕ())
      C -= R;
    // Pick the greater solution.
    Pick_ಕೆಳಗಿನ = ಸುಳ್ಳು;
  } ಇಲ್ಲದಿದ್ದರೆ {
    // If B < 0, the vertex is at a positive location. For any solution
    // to exist, the discriminant must be non-negative. This means that
    // C-kR <= B^2/4A is a necessary condition for k, i.e. there is a
    // lower bound on values of k: kR >= C - B^2/4A.
    APInt LowkR = C - SqrB.udiv(2*TwoA); // udiv because all values > 0.
    // Round LowkR up (towards +inf) to the nearest kR.
    LowkR = ಸುತ್ತು_ಮೇಲೆ_2(LowkR, R);

    // If there exists k meeting the condition above, and such that
    // C-kR > 0, there will be two positive real number solutions of
    // q(x) = kR. Out of all such values of k, pick the one that makes
    // C-kR closest to 0, (i.e. pick maximum k such that C-kR > 0).
    // In other words, find maximum k such that LowkR <= kR < C.
    ಆದರೆ (C.sgt(LowkR)) {
      // If LowkR < C, then such a k is guaranteed to exist because
      // LowkR itself is a multiple of R.
      C -= -ಸುತ್ತು_ಮೇಲೆ_2(-C, R);      // C = C - RoundDown(C, R)
      // Pick the smaller solution.
      Pick_ಕೆಳಗಿನ = ನಿಜ;
    } ಇಲ್ಲದಿದ್ದರೆ {
      // If C-kR < 0 for all potential k's, it means that one solution
      // will be negative, while the other will be positive. The positive
      // solution will shift towards 0 if the parabola is moved up.
      // Pick the kR closest to the lower bound (i.e. make C-kR closest
      // to 0, or in other words, out of all parabolas that have solutions,
      // pick the one that is the farthest "up").
      // Since LowkR is itself a multiple of R, simply take C-LowkR.
      C -= LowkR;
      // Pick the greater solution.
      Pick_ಕೆಳಗಿನ = ಸುಳ್ಳು;
    }
  }

  LLVM_DEBUG(dbgs() << __func__ << ": updated coefficients " << A << "x^2 + "
                    << B << "x + " << C << ", rw:" << Range_ಅಗಲ << '\n');

  APInt D = SqrB - 4*A*C;
  ಪ್ರತಿಜ್ಞಾ(D.isNonNegative() && "Negative discriminant");
  APInt SQ = D.sqrtFloor();

  APInt Q = SQ * SQ;
  ತಾರ್ಕಿಕ InexactSQ = Q != D;

  APInt X;
  APInt Rem;

  // SQ is rounded down (i.e SQ * SQ <= D), so the roots may be inexact.
  // When using the quadratic formula directly, the calculated low root
  // may be greater than the exact one, since we would be subtracting SQ.
  // To make sure that the calculated root is not greater than the exact
  // one, subtract SQ+1 when calculating the low root (for inexact value
  // of SQ).
  ಆದರೆ (Pick_ಕೆಳಗಿನ)
    APInt::sdivrem(-B - (SQ+InexactSQ), TwoA, X, Rem);
  ಇಲ್ಲದಿದ್ದರೆ
    APInt::sdivrem(-B + SQ, TwoA, X, Rem);

  // The updated coefficients should be such that the (exact) solution is
  // positive. Since APInt division rounds towards 0, the calculated one
  // can be 0, but cannot be negative.
  ಪ್ರತಿಜ್ಞಾ(X.isNonNegative() && "Solution should be non-negative");

  ಆದರೆ (!InexactSQ && Rem.isZero()) {
    LLVM_DEBUG(dbgs() << __func__ << ": solution (root): " << X << '\n');
    ಹಿಂತಿರುಗಿ X;
  }

  ಪ್ರತಿಜ್ಞಾ((SQ*SQ).sle(D) && "SQ = |_sqrt(D)_|, so SQ*SQ <= D");
  // The exact value of the square root of D should be between SQ and SQ+1.
  // This implies that the solution should be between that corresponding to
  // SQ (i.e. X) and that corresponding to SQ+1.
  //
  // The calculated X cannot be greater than the exact (real) solution.
  // Actually it must be strictly less than the exact solution, while
  // X+1 will be greater than or equal to it.

  APInt VX = (A*X + B)*X + C;
  APInt VY = VX + TwoA*X + A + B;
  ತಾರ್ಕಿಕ SignChange =
      VX.isNegative() != VY.isNegative() || VX.isZero() != VY.isZero();
  // If the sign did not change between X and X+1, X is not a valid solution.
  // This could happen when the actual (exact) roots don't have an integer
  // between them, so they would both be contained between X and X+1.
  ಆದರೆ (!SignChange) {
    LLVM_DEBUG(dbgs() << __func__ << ": no valid solution\n");
    ಹಿಂತಿರುಗಿ std::nullopt;
  }

  X += 1;
  LLVM_DEBUG(dbgs() << __func__ << ": solution (wrap): " << X << '\n');
  ಹಿಂತಿರುಗಿ X;
}

std::optional<ಚಿಹ್ನೆರಹಿತ>
llvm::APIntOps::Get_Most_Significant_Different_ಬಿಟ್(ಸ್ಥಿರಾಂಕ APInt &A, ಸ್ಥಿರಾಂಕ APInt &B) {
  ಪ್ರತಿಜ್ಞಾ(A.get_ಬಿಟ್_ಅಗಲ() == B.get_ಬಿಟ್_ಅಗಲ() && "Must have the same bitwidth");
  ಆದರೆ (A == B)
    ಹಿಂತಿರುಗಿ std::nullopt;
  ಹಿಂತಿರುಗಿ A.get_ಬಿಟ್_ಅಗಲ() - ((A ^ B).countl_zero() + 1);
}

APInt llvm::APIntOps::Scale_ಬಿಟ್_ಮುಖವಾಡ(ಸ್ಥಿರಾಂಕ APInt &A, ಚಿಹ್ನೆರಹಿತ New_ಬಿಟ್_ಅಗಲ,
                                   ತಾರ್ಕಿಕ ಹೊಂದಿಕೆ_All_ಬಿಟ್ಟುಗಳು) {
  ಚಿಹ್ನೆರಹಿತ Old_ಬಿಟ್_ಅಗಲ = A.get_ಬಿಟ್_ಅಗಲ();
  ಪ್ರತಿಜ್ಞಾ((((Old_ಬಿಟ್_ಅಗಲ % New_ಬಿಟ್_ಅಗಲ) == 0) ||
          ((New_ಬಿಟ್_ಅಗಲ % Old_ಬಿಟ್_ಅಗಲ) == 0)) &&
         "One size should be a multiple of the other one. "
         "Can't do fractional scaling.");

  // Check for matching bitwidths.
  ಆದರೆ (Old_ಬಿಟ್_ಅಗಲ == New_ಬಿಟ್_ಅಗಲ)
    ಹಿಂತಿರುಗಿ A;

  APInt NewA = APInt::getZero(New_ಬಿಟ್_ಅಗಲ);

  // Check for null input.
  ಆದರೆ (A.isZero())
    ಹಿಂತಿರುಗಿ NewA;

  ಆದರೆ (New_ಬಿಟ್_ಅಗಲ > Old_ಬಿಟ್_ಅಗಲ) {
    // Repeat bits.
    ಚಿಹ್ನೆರಹಿತ Scale = New_ಬಿಟ್_ಅಗಲ / Old_ಬಿಟ್_ಅಗಲ;
    ಗಾಗಿ (ಚಿಹ್ನೆರಹಿತ i = 0; i != Old_ಬಿಟ್_ಅಗಲ; ++i)
      ಆದರೆ (A[i])
        NewA.ಗಣ_ಬಿಟ್ಟುಗಳು(i * Scale, (i + 1) * Scale);
  } ಇಲ್ಲದಿದ್ದರೆ {
    ಚಿಹ್ನೆರಹಿತ Scale = Old_ಬಿಟ್_ಅಗಲ / New_ಬಿಟ್_ಅಗಲ;
    ಗಾಗಿ (ಚಿಹ್ನೆರಹಿತ i = 0; i != New_ಬಿಟ್_ಅಗಲ; ++i) {
      ಆದರೆ (ಹೊಂದಿಕೆ_All_ಬಿಟ್ಟುಗಳು) {
        ಆದರೆ (A.ಹೊರತೆಗೆ_ಬಿಟ್ಟುಗಳು(Scale, i * Scale).isAllOnes())
          NewA.ಗಣ_ಬಿಟ್(i);
      } ಇಲ್ಲದಿದ್ದರೆ {
        ಆದರೆ (!A.ಹೊರತೆಗೆ_ಬಿಟ್ಟುಗಳು(Scale, i * Scale).isZero())
          NewA.ಗಣ_ಬಿಟ್(i);
      }
    }
  }

  ಹಿಂತಿರುಗಿ NewA;
}

/// StoreIntToMemory - Fills the StoreBytes bytes of memory starting from Dst
/// with the integer held in IntVal.
ಶೂನ್ಯ llvm::Store_Int_ಗಮ್ಯ_ಸ್ಮರಣೆ(ಸ್ಥಿರಾಂಕ APInt &IntVal, uint8_t *Dst,
                            ಚಿಹ್ನೆರಹಿತ Store_ಬೈಟುಗಳು) {
  ಪ್ರತಿಜ್ಞಾ((IntVal.get_ಬಿಟ್_ಅಗಲ()+7)/8 >= Store_ಬೈಟುಗಳು && "Integer too small!");
  ಸ್ಥಿರಾಂಕ uint8_t *Src = (ಸ್ಥಿರಾಂಕ uint8_t *)IntVal.get_Raw_ದತ್ತಾಂಶ();

  ಆದರೆ (sys::IsLittleEndianHost) {
    // Little-endian host - the source is ordered from LSB to MSB.  Order the
    // destination from LSB to MSB: Do a straight copy.
    ಸ್ಮರಣೆ_ನಕಲು(Dst, Src, Store_ಬೈಟುಗಳು);
  } ಇಲ್ಲದಿದ್ದರೆ {
    // Big-endian host - the source is an array of 64 bit words ordered from
    // LSW to MSW.  Each word is ordered from MSB to LSB.  Order the destination
    // from MSB to LSB: Reverse the word order, but not the bytes in a word.
    ಆಗ (Store_ಬೈಟುಗಳು > ಗಾತ್ರ(uint64_t)) {
      Store_ಬೈಟುಗಳು -= ಗಾತ್ರ(uint64_t);
      // May not be aligned so use memcpy.
      ಸ್ಮರಣೆ_ನಕಲು(Dst + Store_ಬೈಟುಗಳು, Src, ಗಾತ್ರ(uint64_t));
      Src += ಗಾತ್ರ(uint64_t);
    }

    ಸ್ಮರಣೆ_ನಕಲು(Dst, Src + ಗಾತ್ರ(uint64_t) - Store_ಬೈಟುಗಳು, Store_ಬೈಟುಗಳು);
  }
}

/// LoadIntFromMemory - Loads the integer stored in the LoadBytes bytes starting
/// from Src into IntVal, which is assumed to be wide enough and to hold zero.
ಶೂನ್ಯ llvm::ತುಂಬು_Int_From_ಸ್ಮರಣೆ(APInt &IntVal, ಸ್ಥಿರಾಂಕ uint8_t *Src,
                             ಚಿಹ್ನೆರಹಿತ ತುಂಬು_ಬೈಟುಗಳು) {
  ಪ್ರತಿಜ್ಞಾ((IntVal.get_ಬಿಟ್_ಅಗಲ()+7)/8 >= ತುಂಬು_ಬೈಟುಗಳು && "Integer too small!");
  uint8_t *Dst = reinterpret_cast<uint8_t *>(
                   const_cast<uint64_t *>(IntVal.get_Raw_ದತ್ತಾಂಶ()));

  ಆದರೆ (sys::IsLittleEndianHost)
    // Little-endian host - the destination must be ordered from LSB to MSB.
    // The source is ordered from LSB to MSB: Do a straight copy.
    ಸ್ಮರಣೆ_ನಕಲು(Dst, Src, ತುಂಬು_ಬೈಟುಗಳು);
  ಇಲ್ಲದಿದ್ದರೆ {
    // Big-endian - the destination is an array of 64 bit words ordered from
    // LSW to MSW.  Each word must be ordered from MSB to LSB.  The source is
    // ordered from MSB to LSB: Reverse the word order, but not the bytes in
    // a word.
    ಆಗ (ತುಂಬು_ಬೈಟುಗಳು > ಗಾತ್ರ(uint64_t)) {
      ತುಂಬು_ಬೈಟುಗಳು -= ಗಾತ್ರ(uint64_t);
      // May not be aligned so use memcpy.
      ಸ್ಮರಣೆ_ನಕಲು(Dst, Src + ತುಂಬು_ಬೈಟುಗಳು, ಗಾತ್ರ(uint64_t));
      Dst += ಗಾತ್ರ(uint64_t);
    }

    ಸ್ಮರಣೆ_ನಕಲು(Dst + ಗಾತ್ರ(uint64_t) - ತುಂಬು_ಬೈಟುಗಳು, Src, ತುಂಬು_ಬೈಟುಗಳು);
  }
}

APInt APIntOps::avgFloorS(ಸ್ಥಿರಾಂಕ APInt &C1, ಸ್ಥಿರಾಂಕ APInt &C2) {
  // Return floor((C1 + C2) / 2)
  ಹಿಂತಿರುಗಿ (C1 & C2) + (C1 ^ C2).ashr(1);
}

APInt APIntOps::avgFloorU(ಸ್ಥಿರಾಂಕ APInt &C1, ಸ್ಥಿರಾಂಕ APInt &C2) {
  // Return floor((C1 + C2) / 2)
  ಹಿಂತಿರುಗಿ (C1 & C2) + (C1 ^ C2).lshr(1);
}

APInt APIntOps::avgCeilS(ಸ್ಥಿರಾಂಕ APInt &C1, ಸ್ಥಿರಾಂಕ APInt &C2) {
  // Return ceil((C1 + C2) / 2)
  ಹಿಂತಿರುಗಿ (C1 | C2) - (C1 ^ C2).ashr(1);
}

APInt APIntOps::avgCeilU(ಸ್ಥಿರಾಂಕ APInt &C1, ಸ್ಥಿರಾಂಕ APInt &C2) {
  // Return ceil((C1 + C2) / 2)
  ಹಿಂತಿರುಗಿ (C1 | C2) - (C1 ^ C2).lshr(1);
}

APInt APIntOps::avgSubFloorS(ಸ್ಥಿರಾಂಕ APInt &C1, ಸ್ಥಿರಾಂಕ APInt &C2) {
  // Return floor((C1 - C2) / 2), using C1 - C2 == (C1 ^ C2) - 2 * (~C1 & C2).
  ಹಿಂತಿರುಗಿ (C1 ^ C2).ashr(1) - (~C1 & C2);
}

APInt APIntOps::avgSubFloorU(ಸ್ಥಿರಾಂಕ APInt &C1, ಸ್ಥಿರಾಂಕ APInt &C2) {
  // Return floor((C1 - C2) / 2), using C1 - C2 == (C1 ^ C2) - 2 * (~C1 & C2).
  ಹಿಂತಿರುಗಿ (C1 ^ C2).lshr(1) - (~C1 & C2);
}

APInt APIntOps::mulhs(ಸ್ಥಿರಾಂಕ APInt &C1, ಸ್ಥಿರಾಂಕ APInt &C2) {
  ಪ್ರತಿಜ್ಞಾ(C1.get_ಬಿಟ್_ಅಗಲ() == C2.get_ಬಿಟ್_ಅಗಲ() && "Unequal bitwidths");
  ಚಿಹ್ನೆರಹಿತ ತುಂಬಿದ_ಅಗಲ = C1.get_ಬಿಟ್_ಅಗಲ() * 2;
  APInt C1Ext = C1.sext(ತುಂಬಿದ_ಅಗಲ);
  APInt C2Ext = C2.sext(ತುಂಬಿದ_ಅಗಲ);
  ಹಿಂತಿರುಗಿ (C1Ext * C2Ext).ಹೊರತೆಗೆ_ಬಿಟ್ಟುಗಳು(C1.get_ಬಿಟ್_ಅಗಲ(), C1.get_ಬಿಟ್_ಅಗಲ());
}

APInt APIntOps::mulhu(ಸ್ಥಿರಾಂಕ APInt &C1, ಸ್ಥಿರಾಂಕ APInt &C2) {
  ಪ್ರತಿಜ್ಞಾ(C1.get_ಬಿಟ್_ಅಗಲ() == C2.get_ಬಿಟ್_ಅಗಲ() && "Unequal bitwidths");
  ಚಿಹ್ನೆರಹಿತ ತುಂಬಿದ_ಅಗಲ = C1.get_ಬಿಟ್_ಅಗಲ() * 2;
  APInt C1Ext = C1.zext(ತುಂಬಿದ_ಅಗಲ);
  APInt C2Ext = C2.zext(ತುಂಬಿದ_ಅಗಲ);
  ಹಿಂತಿರುಗಿ (C1Ext * C2Ext).ಹೊರತೆಗೆ_ಬಿಟ್ಟುಗಳು(C1.get_ಬಿಟ್_ಅಗಲ(), C1.get_ಬಿಟ್_ಅಗಲ());
}

APInt APIntOps::mulsExtended(ಸ್ಥಿರಾಂಕ APInt &C1, ಸ್ಥಿರಾಂಕ APInt &C2) {
  ಪ್ರತಿಜ್ಞಾ(C1.get_ಬಿಟ್_ಅಗಲ() == C2.get_ಬಿಟ್_ಅಗಲ() && "Unequal bitwidths");
  ಚಿಹ್ನೆರಹಿತ ತುಂಬಿದ_ಅಗಲ = C1.get_ಬಿಟ್_ಅಗಲ() * 2;
  APInt C1Ext = C1.sext(ತುಂಬಿದ_ಅಗಲ);
  APInt C2Ext = C2.sext(ತುಂಬಿದ_ಅಗಲ);
  ಹಿಂತಿರುಗಿ C1Ext * C2Ext;
}

APInt APIntOps::muluExtended(ಸ್ಥಿರಾಂಕ APInt &C1, ಸ್ಥಿರಾಂಕ APInt &C2) {
  ಪ್ರತಿಜ್ಞಾ(C1.get_ಬಿಟ್_ಅಗಲ() == C2.get_ಬಿಟ್_ಅಗಲ() && "Unequal bitwidths");
  ಚಿಹ್ನೆರಹಿತ ತುಂಬಿದ_ಅಗಲ = C1.get_ಬಿಟ್_ಅಗಲ() * 2;
  APInt C1Ext = C1.zext(ತುಂಬಿದ_ಅಗಲ);
  APInt C2Ext = C2.zext(ತುಂಬಿದ_ಅಗಲ);
  ಹಿಂತಿರುಗಿ C1Ext * C2Ext;
}

APInt APIntOps::ಘಾತ(ಸ್ಥಿರಾಂಕ APInt &X, int64_t N) {
  ಪ್ರತಿಜ್ಞಾ(N >= 0 && "negative exponents not supported.");
  APInt Acc = APInt(X.get_ಬಿಟ್_ಅಗಲ(), 1);
  ಆದರೆ (N == 0)
    ಹಿಂತಿರುಗಿ Acc;
  APInt ತಳ = X;
  int64_t ಉಳಿದ_Exponent = N;
  ಆಗ (ಉಳಿದ_Exponent > 0) {
    ಆಗ (ಉಳಿದ_Exponent % 2 == 0) {
      ತಳ *= ತಳ;
      ಉಳಿದ_Exponent /= 2;
    }
    --ಉಳಿದ_Exponent;
    Acc *= ತಳ;
  }
  ಹಿಂತಿರುಗಿ Acc;
}

APInt llvm::APIntOps::fshl(ಸ್ಥಿರಾಂಕ APInt &Hi, ಸ್ಥಿರಾಂಕ APInt &Lo,
                           ಸ್ಥಿರಾಂಕ APInt &ಸರಿಸು_2) {
  ಪ್ರತಿಜ್ಞಾ(Hi.get_ಬಿಟ್_ಅಗಲ() == Lo.get_ಬಿಟ್_ಅಗಲ());
  ಚಿಹ್ನೆರಹಿತ ಸರಿಸು_Amt_2 = rotateModulo(Hi.get_ಬಿಟ್_ಅಗಲ(), ಸರಿಸು_2);
  ಆದರೆ (ಸರಿಸು_Amt_2 == 0)
    ಹಿಂತಿರುಗಿ Hi;
  ಹಿಂತಿರುಗಿ Hi.shl(ಸರಿಸು_Amt_2) | Lo.lshr(Hi.get_ಬಿಟ್_ಅಗಲ() - ಸರಿಸು_Amt_2);
}

APInt llvm::APIntOps::fshr(ಸ್ಥಿರಾಂಕ APInt &Hi, ಸ್ಥಿರಾಂಕ APInt &Lo,
                           ಸ್ಥಿರಾಂಕ APInt &ಸರಿಸು_2) {
  ಪ್ರತಿಜ್ಞಾ(Hi.get_ಬಿಟ್_ಅಗಲ() == Lo.get_ಬಿಟ್_ಅಗಲ());
  ಚಿಹ್ನೆರಹಿತ ಸರಿಸು_Amt_2 = rotateModulo(Hi.get_ಬಿಟ್_ಅಗಲ(), ಸರಿಸು_2);
  ಆದರೆ (ಸರಿಸು_Amt_2 == 0)
    ಹಿಂತಿರುಗಿ Lo;
  ಹಿಂತಿರುಗಿ Hi.shl(Hi.get_ಬಿಟ್_ಅಗಲ() - ಸರಿಸು_Amt_2) | Lo.lshr(ಸರಿಸು_Amt_2);
}

APInt llvm::APIntOps::clmul(ಸ್ಥಿರಾಂಕ APInt &LHS, ಸ್ಥಿರಾಂಕ APInt &RHS) {
  ಚಿಹ್ನೆರಹಿತ BW = LHS.get_ಬಿಟ್_ಅಗಲ();
  ಪ್ರತಿಜ್ಞಾ(BW == RHS.get_ಬಿಟ್_ಅಗಲ() && "Operand mismatch");
  APInt ಫಲಿತಾಂಶ(BW, 0);
  ಗಾಗಿ (ಚಿಹ್ನೆರಹಿತ I : seq(std::min(RHS.get_ಸಕ್ರಿಯ_ಬಿಟ್ಟುಗಳು(), BW - LHS.countr_zero())))
    ಆದರೆ (RHS[I])
      ಫಲಿತಾಂಶ ^= LHS << I;
  ಹಿಂತಿರುಗಿ ಫಲಿತಾಂಶ;
}

APInt llvm::APIntOps::clmulr(ಸ್ಥಿರಾಂಕ APInt &LHS, ಸ್ಥಿರಾಂಕ APInt &RHS) {
  ಪ್ರತಿಜ್ಞಾ(LHS.get_ಬಿಟ್_ಅಗಲ() == RHS.get_ಬಿಟ್_ಅಗಲ());
  ಹಿಂತಿರುಗಿ clmul(LHS.ಹಿಮ್ಮುಖ_ಬಿಟ್ಟುಗಳು(), RHS.ಹಿಮ್ಮುಖ_ಬಿಟ್ಟುಗಳು()).ಹಿಮ್ಮುಖ_ಬಿಟ್ಟುಗಳು();
}

APInt llvm::APIntOps::clmulh(ಸ್ಥಿರಾಂಕ APInt &LHS, ಸ್ಥಿರಾಂಕ APInt &RHS) {
  ಪ್ರತಿಜ್ಞಾ(LHS.get_ಬಿಟ್_ಅಗಲ() == RHS.get_ಬಿಟ್_ಅಗಲ());
  ಹಿಂತಿರುಗಿ clmulr(LHS, RHS).lshr(1);
}

APInt llvm::APIntOps::pext(ಸ್ಥಿರಾಂಕ APInt &Val, ಸ್ಥಿರಾಂಕ APInt &ಮುಖವಾಡ) {
  ಚಿಹ್ನೆರಹಿತ BW = Val.get_ಬಿಟ್_ಅಗಲ();
  ಪ್ರತಿಜ್ಞಾ(BW == ಮುಖವಾಡ.get_ಬಿಟ್_ಅಗಲ() && "Operand mismatch");
  APInt ಫಲಿತಾಂಶ = APInt::getZero(BW);
  ಗಾಗಿ (ಚಿಹ್ನೆರಹಿತ I = 0, P = 0; I != BW; ++I)
    ಆದರೆ (ಮುಖವಾಡ[I])
      ಫಲಿತಾಂಶ.ಗಣ_ಬಿಟ್_Val(P++, Val[I]);
  ಹಿಂತಿರುಗಿ ಫಲಿತಾಂಶ;
}

APInt llvm::APIntOps::pdep(ಸ್ಥಿರಾಂಕ APInt &Val, ಸ್ಥಿರಾಂಕ APInt &ಮುಖವಾಡ) {
  ಚಿಹ್ನೆರಹಿತ BW = Val.get_ಬಿಟ್_ಅಗಲ();
  ಪ್ರತಿಜ್ಞಾ(BW == ಮುಖವಾಡ.get_ಬಿಟ್_ಅಗಲ() && "Operand mismatch");
  APInt ಫಲಿತಾಂಶ = APInt::getZero(BW);
  ಗಾಗಿ (ಚಿಹ್ನೆರಹಿತ I = 0, P = 0; I != BW; ++I)
    ಆದರೆ (ಮುಖವಾಡ[I])
      ಫಲಿತಾಂಶ.ಗಣ_ಬಿಟ್_Val(I, Val[P++]);
  ಹಿಂತಿರುಗಿ ಫಲಿತಾಂಶ;
}
