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

#ਅਨ੍ਤਰ੍ਭਾਵ "llvm/ADT/APInt.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "llvm/ADT/ArrayRef.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "llvm/ADT/FoldingSet.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "llvm/ADT/Hashing.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "llvm/ADT/Sequence.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "llvm/ADT/SmallString.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "llvm/ADT/StringRef.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "llvm/ADT/bit.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "llvm/Support/Alignment.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "llvm/Support/Debug.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "llvm/Support/ErrorHandling.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "llvm/Support/MathExtras.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "llvm/Support/SwapByteOrder.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "llvm/Support/raw_ostream.h"
#ਅਨ੍ਤਰ੍ਭਾਵ <cmath>
#ਅਨ੍ਤਰ੍ਭਾਵ <optional>

ਵਰਤੋਂ ਨਾਮਾਕਾਸ਼ llvm;

#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ DEBUG_TYPE "apint"

/// A utility function for allocating memory, checking for allocation failures,
/// and ensuring the contents are zeroed.
ਸਤਰਅੰਦਰ ਸਥਿਰ uint64_t* get_Cleared_ਸਮ੍ਰਿਤੀ(ਬਿਨਾਨਿਸ਼ਾਨੀ ਅੰਕ_ਸ਼ਬਦ) {
  ਵਾਪਸ ਨਵਾਂ uint64_t[ਅੰਕ_ਸ਼ਬਦ]();
}

/// A utility function for allocating memory and checking for allocation
/// failure.  The content is not zeroed.
ਸਤਰਅੰਦਰ ਸਥਿਰ uint64_t* get_ਸਮ੍ਰਿਤੀ(ਬਿਨਾਨਿਸ਼ਾਨੀ ਅੰਕ_ਸ਼ਬਦ) {
  ਵਾਪਸ ਨਵਾਂ uint64_t[ਅੰਕ_ਸ਼ਬਦ];
}

/// A utility function that converts a character to a digit.
ਸਤਰਅੰਦਰ ਸਥਿਰ ਬਿਨਾਨਿਸ਼ਾਨੀ getDigit(ਅੱਖਰ cdigit, uint8_t radix) {
  ਬਿਨਾਨਿਸ਼ਾਨੀ r;

  ਜੇ (radix == 16 || radix == 36) {
    r = cdigit - '0';
    ਜੇ (r <= 9)
      ਵਾਪਸ r;

    r = cdigit - 'A';
    ਜੇ (r <= radix - 11U)
      ਵਾਪਸ r + 10;

    r = cdigit - 'a';
    ਜੇ (r <= radix - 11U)
      ਵਾਪਸ r + 10;

    radix = 10;
  }

  r = cdigit - '0';
  ਜੇ (r < radix)
    ਵਾਪਸ r;

  ਵਾਪਸ UINT_MAX;
}


ਖਾਲੀ APInt::initSlowCase(uint64_t val, ਤਾਰਕਿਕ isSigned) {
  ਜੇ (isSigned && int64_t(val) < 0) {
    U.pVal = get_ਸਮ੍ਰਿਤੀ(get_ਅੰਕ_ਸ਼ਬਦ());
    U.pVal[0] = val;
    memset(&U.pVal[1], 0xFF, APINT_ਸ਼ਬਦ_ਮਾਪ * (get_ਅੰਕ_ਸ਼ਬਦ() - 1));
    clear_ਅਣਵਰਤਿਆ_ਬਿੱਟ();
  } ਨਹੀਂਤਾਂ {
    U.pVal = get_Cleared_ਸਮ੍ਰਿਤੀ(get_ਅੰਕ_ਸ਼ਬਦ());
    U.pVal[0] = val;
  }
}

ਖਾਲੀ APInt::initSlowCase(ਅਚਲ APInt& that) {
  U.pVal = get_ਸਮ੍ਰਿਤੀ(get_ਅੰਕ_ਸ਼ਬਦ());
  ਸਮ੍ਰਿਤੀ_ਨਕਲ(U.pVal, that.U.pVal, get_ਅੰਕ_ਸ਼ਬਦ() * APINT_ਸ਼ਬਦ_ਮਾਪ);
}

ਖਾਲੀ APInt::initFromArray(ArrayRef<uint64_t> bigVal) {
  ਪ੍ਰਤਿਜ੍ਞਾ(bigVal.ਅੰਕੜੇ() && "Null pointer detected!");
  ਜੇ (LLVM_LIKELY(is_Single_ਸ਼ਬਦ()))
    U.VAL = bigVal[0];
  ਨਹੀਂਤਾਂ {
    // Get memory, cleared to 0
    U.pVal = get_Cleared_ਸਮ੍ਰਿਤੀ(get_ਅੰਕ_ਸ਼ਬਦ());
    // Calculate the number of words to copy
    ਬਿਨਾਨਿਸ਼ਾਨੀ ਸ਼ਬਦ = std::min<ਬਿਨਾਨਿਸ਼ਾਨੀ>(bigVal.ਮਾਪ_2(), get_ਅੰਕ_ਸ਼ਬਦ());
    // Copy the words from bigVal to pVal
    ਸਮ੍ਰਿਤੀ_ਨਕਲ(U.pVal, bigVal.ਅੰਕੜੇ(), ਸ਼ਬਦ * APINT_ਸ਼ਬਦ_ਮਾਪ);
  }
  // Make sure unused high bits are cleared
  clear_ਅਣਵਰਤਿਆ_ਬਿੱਟ();
}

APInt::APInt(ਬਿਨਾਨਿਸ਼ਾਨੀ ਅੰਕ_ਬਿੱਟ, ArrayRef<uint64_t> bigVal) : ਦੋਆਧਾਰੀ_ਅੰਕ_ਚੌੜਾਈ_2(ਅੰਕ_ਬਿੱਟ) {
  initFromArray(bigVal);
}

APInt::APInt(ਬਿਨਾਨਿਸ਼ਾਨੀ numbits, StringRef Str, uint8_t radix)
    : ਦੋਆਧਾਰੀ_ਅੰਕ_ਚੌੜਾਈ_2(numbits) {
  fromString(numbits, Str, radix);
}

ਖਾਲੀ APInt::reallocate(ਬਿਨਾਨਿਸ਼ਾਨੀ New_ਦੋਆਧਾਰੀ_ਅੰਕ_ਚੌੜਾਈ) {
  // If the number of words is the same we can just change the width and stop.
  ਜੇ (get_ਅੰਕ_ਸ਼ਬਦ() == get_ਅੰਕ_ਸ਼ਬਦ(New_ਦੋਆਧਾਰੀ_ਅੰਕ_ਚੌੜਾਈ)) {
    ਦੋਆਧਾਰੀ_ਅੰਕ_ਚੌੜਾਈ_2 = New_ਦੋਆਧਾਰੀ_ਅੰਕ_ਚੌੜਾਈ;
    ਵਾਪਸ;
  }

  // If we have an allocation, delete it.
  ਜੇ (LLVM_UNLIKELY(!is_Single_ਸ਼ਬਦ()))
    ਮਿਟਾਓ [] U.pVal;

  // Update BitWidth.
  ਦੋਆਧਾਰੀ_ਅੰਕ_ਚੌੜਾਈ_2 = New_ਦੋਆਧਾਰੀ_ਅੰਕ_ਚੌੜਾਈ;

  // If we are supposed to have an allocation, create it.
  ਜੇ (LLVM_UNLIKELY(!is_Single_ਸ਼ਬਦ()))
    U.pVal = get_ਸਮ੍ਰਿਤੀ(get_ਅੰਕ_ਸ਼ਬਦ());
}

ਖਾਲੀ APInt::ਸੌਂਪੋ_Slow_Case(ਅਚਲ APInt &RHS) {
  // Don't do anything for X = X
  ਜੇ (ਇਹ == &RHS)
    ਵਾਪਸ;

  // Adjust the bit width and handle allocations as necessary.
  reallocate(RHS.get_ਦੋਆਧਾਰੀ_ਅੰਕ_ਚੌੜਾਈ());

  // Copy the data.
  ਜੇ (LLVM_LIKELY(is_Single_ਸ਼ਬਦ()))
    U.VAL = RHS.U.VAL;
  ਨਹੀਂਤਾਂ
    ਸਮ੍ਰਿਤੀ_ਨਕਲ(U.pVal, RHS.U.pVal, get_ਅੰਕ_ਸ਼ਬਦ() * APINT_ਸ਼ਬਦ_ਮਾਪ);
}

/// This method 'profiles' an APInt for use with FoldingSet.
ਖਾਲੀ APInt::Profile(Folding_ਸੈੱਟ_ਗੰਢ_ID& ID) ਅਚਲ {
  ID.ਜੋੜੋ_Integer(ਦੋਆਧਾਰੀ_ਅੰਕ_ਚੌੜਾਈ_2);

  ਜੇ (LLVM_LIKELY(is_Single_ਸ਼ਬਦ())) {
    ID.ਜੋੜੋ_Integer(U.VAL);
    ਵਾਪਸ;
  }

  ਬਿਨਾਨਿਸ਼ਾਨੀ ਅੰਕ_ਸ਼ਬਦ_2 = get_ਅੰਕ_ਸ਼ਬਦ();
  ਲਈ (ਬਿਨਾਨਿਸ਼ਾਨੀ i = 0; i < ਅੰਕ_ਸ਼ਬਦ_2; ++i)
    ID.ਜੋੜੋ_Integer(U.pVal[i]);
}

ਤਾਰਕਿਕ APInt::isAligned(Align A) ਅਚਲ {
  ਜੇ (isZero())
    ਵਾਪਸ ਸੱਚ;
  ਅਚਲ ਬਿਨਾਨਿਸ਼ਾਨੀ TrailingZeroes = countr_zero();
  ਅਚਲ ਬਿਨਾਨਿਸ਼ਾਨੀ MinimumTrailingZeroes = Log2(A);
  ਵਾਪਸ TrailingZeroes >= MinimumTrailingZeroes;
}

/// Prefix increment operator. Increments the APInt by one.
APInt& APInt::ਸੰਚਾਲਕ++() {
  ਜੇ (LLVM_LIKELY(is_Single_ਸ਼ਬਦ()))
    ++U.VAL;
  ਨਹੀਂਤਾਂ
    tcIncrement(U.pVal, get_ਅੰਕ_ਸ਼ਬਦ());
  ਵਾਪਸ clear_ਅਣਵਰਤਿਆ_ਬਿੱਟ();
}

/// Prefix decrement operator. Decrements the APInt by one.
APInt& APInt::ਸੰਚਾਲਕ--() {
  ਜੇ (LLVM_LIKELY(is_Single_ਸ਼ਬਦ()))
    --U.VAL;
  ਨਹੀਂਤਾਂ
    tcDecrement(U.pVal, get_ਅੰਕ_ਸ਼ਬਦ());
  ਵਾਪਸ clear_ਅਣਵਰਤਿਆ_ਬਿੱਟ();
}

/// Adds the RHS APInt to this APInt.
/// @returns this, after addition of RHS.
/// Addition assignment operator.
APInt& APInt::ਸੰਚਾਲਕ+=(ਅਚਲ APInt& RHS) {
  ਪ੍ਰਤਿਜ੍ਞਾ(ਦੋਆਧਾਰੀ_ਅੰਕ_ਚੌੜਾਈ_2 == RHS.ਦੋਆਧਾਰੀ_ਅੰਕ_ਚੌੜਾਈ_2 && "Bit widths must be the same");
  ਜੇ (LLVM_LIKELY(is_Single_ਸ਼ਬਦ()))
    U.VAL += RHS.U.VAL;
  ਨਹੀਂਤਾਂ
    tc_ਜੋੜੋ(U.pVal, RHS.U.pVal, 0, get_ਅੰਕ_ਸ਼ਬਦ());
  ਵਾਪਸ clear_ਅਣਵਰਤਿਆ_ਬਿੱਟ();
}

APInt& APInt::ਸੰਚਾਲਕ+=(uint64_t RHS) {
  ਜੇ (LLVM_LIKELY(is_Single_ਸ਼ਬਦ()))
    U.VAL += RHS;
  ਨਹੀਂਤਾਂ
    tc_ਜੋੜੋ_Part(U.pVal, RHS, get_ਅੰਕ_ਸ਼ਬਦ());
  ਵਾਪਸ clear_ਅਣਵਰਤਿਆ_ਬਿੱਟ();
}

/// Subtracts the RHS APInt from this APInt
/// @returns this, after subtraction
/// Subtraction assignment operator.
APInt& APInt::ਸੰਚਾਲਕ-=(ਅਚਲ APInt& RHS) {
  ਪ੍ਰਤਿਜ੍ਞਾ(ਦੋਆਧਾਰੀ_ਅੰਕ_ਚੌੜਾਈ_2 == RHS.ਦੋਆਧਾਰੀ_ਅੰਕ_ਚੌੜਾਈ_2 && "Bit widths must be the same");
  ਜੇ (LLVM_LIKELY(is_Single_ਸ਼ਬਦ()))
    U.VAL -= RHS.U.VAL;
  ਨਹੀਂਤਾਂ
    tc_ਘਟਾਓ(U.pVal, RHS.U.pVal, 0, get_ਅੰਕ_ਸ਼ਬਦ());
  ਵਾਪਸ clear_ਅਣਵਰਤਿਆ_ਬਿੱਟ();
}

APInt& APInt::ਸੰਚਾਲਕ-=(uint64_t RHS) {
  ਜੇ (LLVM_LIKELY(is_Single_ਸ਼ਬਦ()))
    U.VAL -= RHS;
  ਨਹੀਂਤਾਂ
    tc_ਘਟਾਓ_Part(U.pVal, RHS, get_ਅੰਕ_ਸ਼ਬਦ());
  ਵਾਪਸ clear_ਅਣਵਰਤਿਆ_ਬਿੱਟ();
}

APInt APInt::ਸੰਚਾਲਕ*(ਅਚਲ APInt& RHS) ਅਚਲ {
  ਪ੍ਰਤਿਜ੍ਞਾ(ਦੋਆਧਾਰੀ_ਅੰਕ_ਚੌੜਾਈ_2 == RHS.ਦੋਆਧਾਰੀ_ਅੰਕ_ਚੌੜਾਈ_2 && "Bit widths must be the same");
  ਜੇ (LLVM_LIKELY(is_Single_ਸ਼ਬਦ()))
    ਵਾਪਸ APInt(ਦੋਆਧਾਰੀ_ਅੰਕ_ਚੌੜਾਈ_2, U.VAL * RHS.U.VAL, /*isSigned=*/ਝੂਠ,
                 /*implicitTrunc=*/ਸੱਚ);

  APInt ਨਤੀਜਾ(get_ਸਮ੍ਰਿਤੀ(get_ਅੰਕ_ਸ਼ਬਦ()), get_ਦੋਆਧਾਰੀ_ਅੰਕ_ਚੌੜਾਈ());
  tcMultiply(ਨਤੀਜਾ.U.pVal, U.pVal, RHS.U.pVal, get_ਅੰਕ_ਸ਼ਬਦ());
  ਨਤੀਜਾ.clear_ਅਣਵਰਤਿਆ_ਬਿੱਟ();
  ਵਾਪਸ ਨਤੀਜਾ;
}

ਖਾਲੀ APInt::and_ਸੌਂਪੋ_Slow_Case(ਅਚਲ APInt &RHS) {
  ਸ਼ਬਦ_Type *dst = U.pVal, *rhs = RHS.U.pVal;
  ਲਈ (size_t i = 0, e = get_ਅੰਕ_ਸ਼ਬਦ(); i != e; ++i)
    dst[i] &= rhs[i];
}

ਖਾਲੀ APInt::or_ਸੌਂਪੋ_Slow_Case(ਅਚਲ APInt &RHS) {
  ਸ਼ਬਦ_Type *dst = U.pVal, *rhs = RHS.U.pVal;
  ਲਈ (size_t i = 0, e = get_ਅੰਕ_ਸ਼ਬਦ(); i != e; ++i)
    dst[i] |= rhs[i];
}

ਖਾਲੀ APInt::xor_ਸੌਂਪੋ_Slow_Case(ਅਚਲ APInt &RHS) {
  ਸ਼ਬਦ_Type *dst = U.pVal, *rhs = RHS.U.pVal;
  ਲਈ (size_t i = 0, e = get_ਅੰਕ_ਸ਼ਬਦ(); i != e; ++i)
    dst[i] ^= rhs[i];
}

APInt &APInt::ਸੰਚਾਲਕ*=(ਅਚਲ APInt &RHS) {
  *ਇਹ = *ਇਹ * RHS;
  ਵਾਪਸ *ਇਹ;
}

APInt& APInt::ਸੰਚਾਲਕ*=(uint64_t RHS) {
  ਜੇ (LLVM_LIKELY(is_Single_ਸ਼ਬਦ())) {
    U.VAL *= RHS;
  } ਨਹੀਂਤਾਂ {
    ਬਿਨਾਨਿਸ਼ਾਨੀ ਅੰਕ_ਸ਼ਬਦ_2 = get_ਅੰਕ_ਸ਼ਬਦ();
    tcMultiplyPart(U.pVal, U.pVal, RHS, 0, ਅੰਕ_ਸ਼ਬਦ_2, ਅੰਕ_ਸ਼ਬਦ_2, ਝੂਠ);
  }
  ਵਾਪਸ clear_ਅਣਵਰਤਿਆ_ਬਿੱਟ();
}

ਤਾਰਕਿਕ APInt::equalSlowCase(ਅਚਲ APInt &RHS) ਅਚਲ {
  ਵਾਪਸ std::equal(U.pVal, U.pVal + get_ਅੰਕ_ਸ਼ਬਦ(), RHS.U.pVal);
}

ਪੂਰਨ APInt::compare(ਅਚਲ APInt& RHS) ਅਚਲ {
  ਪ੍ਰਤਿਜ੍ਞਾ(ਦੋਆਧਾਰੀ_ਅੰਕ_ਚੌੜਾਈ_2 == RHS.ਦੋਆਧਾਰੀ_ਅੰਕ_ਚੌੜਾਈ_2 && "Bit widths must be same for comparison");
  ਜੇ (LLVM_LIKELY(is_Single_ਸ਼ਬਦ()))
    ਵਾਪਸ U.VAL < RHS.U.VAL ? -1 : U.VAL > RHS.U.VAL;

  ਵਾਪਸ tcCompare(U.pVal, RHS.U.pVal, get_ਅੰਕ_ਸ਼ਬਦ());
}

ਪੂਰਨ APInt::compareSigned(ਅਚਲ APInt& RHS) ਅਚਲ {
  ਪ੍ਰਤਿਜ੍ਞਾ(ਦੋਆਧਾਰੀ_ਅੰਕ_ਚੌੜਾਈ_2 == RHS.ਦੋਆਧਾਰੀ_ਅੰਕ_ਚੌੜਾਈ_2 && "Bit widths must be same for comparison");
  ਜੇ (LLVM_LIKELY(is_Single_ਸ਼ਬਦ())) {
    int64_t lhsSext = SignExtend64(U.VAL, ਦੋਆਧਾਰੀ_ਅੰਕ_ਚੌੜਾਈ_2);
    int64_t rhsSext = SignExtend64(RHS.U.VAL, ਦੋਆਧਾਰੀ_ਅੰਕ_ਚੌੜਾਈ_2);
    ਵਾਪਸ lhsSext < rhsSext ? -1 : lhsSext > rhsSext;
  }

  ਤਾਰਕਿਕ lhsNeg = isNegative();
  ਤਾਰਕਿਕ rhsNeg = RHS.isNegative();

  // If the sign bits don't match, then (LHS < RHS) if LHS is negative
  ਜੇ (lhsNeg != rhsNeg)
    ਵਾਪਸ lhsNeg ? -1 : 1;

  // Otherwise we can just use an unsigned comparison, because even negative
  // numbers compare correctly this way if both have the same signed-ness.
  ਵਾਪਸ tcCompare(U.pVal, RHS.U.pVal, get_ਅੰਕ_ਸ਼ਬਦ());
}

ਖਾਲੀ APInt::ਸੈੱਟ_ਬਿੱਟ_Slow_Case(ਬਿਨਾਨਿਸ਼ਾਨੀ lo_ਦੋਆਧਾਰੀ_ਅੰਕ, ਬਿਨਾਨਿਸ਼ਾਨੀ hi_ਦੋਆਧਾਰੀ_ਅੰਕ) {
  ਬਿਨਾਨਿਸ਼ਾਨੀ lo_ਸ਼ਬਦ = which_ਸ਼ਬਦ(lo_ਦੋਆਧਾਰੀ_ਅੰਕ);
  ਬਿਨਾਨਿਸ਼ਾਨੀ hi_ਸ਼ਬਦ = which_ਸ਼ਬਦ(hi_ਦੋਆਧਾਰੀ_ਅੰਕ);

  // Create an initial mask for the low word with zeros below loBit.
  uint64_t lo_ਨਕਾਬ = WORDTYPE_MAX << which_ਦੋਆਧਾਰੀ_ਅੰਕ(lo_ਦੋਆਧਾਰੀ_ਅੰਕ);

  // If hiBit is not aligned, we need a high mask.
  ਬਿਨਾਨਿਸ਼ਾਨੀ hi_ਖਿਸਕਾਓ_Amt = which_ਦੋਆਧਾਰੀ_ਅੰਕ(hi_ਦੋਆਧਾਰੀ_ਅੰਕ);
  ਜੇ (hi_ਖਿਸਕਾਓ_Amt != 0) {
    // Create a high mask with zeros above hiBit.
    uint64_t hi_ਨਕਾਬ = WORDTYPE_MAX >> (APINT_ਬਿੱਟ_ਪ੍ਰਤੀ_ਸ਼ਬਦ - hi_ਖਿਸਕਾਓ_Amt);
    // If loWord and hiWord are equal, then we combine the masks. Otherwise,
    // set the bits in hiWord.
    ਜੇ (hi_ਸ਼ਬਦ == lo_ਸ਼ਬਦ)
      lo_ਨਕਾਬ &= hi_ਨਕਾਬ;
    ਨਹੀਂਤਾਂ
      U.pVal[hi_ਸ਼ਬਦ] |= hi_ਨਕਾਬ;
  }
  // Apply the mask to the low word.
  U.pVal[lo_ਸ਼ਬਦ] |= lo_ਨਕਾਬ;

  // Fill any words between loWord and hiWord with all ones.
  ਲਈ (ਬਿਨਾਨਿਸ਼ਾਨੀ ਸ਼ਬਦ_2 = lo_ਸ਼ਬਦ + 1; ਸ਼ਬਦ_2 < hi_ਸ਼ਬਦ; ++ਸ਼ਬਦ_2)
    U.pVal[ਸ਼ਬਦ_2] = WORDTYPE_MAX;
}

ਖਾਲੀ APInt::clear_ਬਿੱਟ_Slow_Case(ਬਿਨਾਨਿਸ਼ਾਨੀ Lo_ਦੋਆਧਾਰੀ_ਅੰਕ, ਬਿਨਾਨਿਸ਼ਾਨੀ Hi_ਦੋਆਧਾਰੀ_ਅੰਕ) {
  ਬਿਨਾਨਿਸ਼ਾਨੀ Lo_ਸ਼ਬਦ = which_ਸ਼ਬਦ(Lo_ਦੋਆਧਾਰੀ_ਅੰਕ);
  ਬਿਨਾਨਿਸ਼ਾਨੀ Hi_ਸ਼ਬਦ = which_ਸ਼ਬਦ(Hi_ਦੋਆਧਾਰੀ_ਅੰਕ);

  // Create an initial mask for the low word with ones below loBit.
  uint64_t Lo_ਨਕਾਬ = ~(WORDTYPE_MAX << which_ਦੋਆਧਾਰੀ_ਅੰਕ(Lo_ਦੋਆਧਾਰੀ_ਅੰਕ));

  // If HiBit is not aligned, we need a high mask.
  ਬਿਨਾਨਿਸ਼ਾਨੀ Hi_ਖਿਸਕਾਓ_Amt = which_ਦੋਆਧਾਰੀ_ਅੰਕ(Hi_ਦੋਆਧਾਰੀ_ਅੰਕ);
  ਜੇ (Hi_ਖਿਸਕਾਓ_Amt != 0) {
    // Create a high mask with ones above HiBit.
    uint64_t Hi_ਨਕਾਬ = ~(WORDTYPE_MAX >> (APINT_ਬਿੱਟ_ਪ੍ਰਤੀ_ਸ਼ਬਦ - Hi_ਖਿਸਕਾਓ_Amt));
    // If LoWord and HiWord are equal, then we combine the masks. Otherwise,
    // clear the bits in HiWord.
    ਜੇ (Hi_ਸ਼ਬਦ == Lo_ਸ਼ਬਦ)
      Lo_ਨਕਾਬ |= Hi_ਨਕਾਬ;
    ਨਹੀਂਤਾਂ
      U.pVal[Hi_ਸ਼ਬਦ] &= Hi_ਨਕਾਬ;
  }
  // Apply the mask to the low word.
  U.pVal[Lo_ਸ਼ਬਦ] &= Lo_ਨਕਾਬ;

  // Fill any words between LoWord and HiWord with all zeros.
  ਲਈ (ਬਿਨਾਨਿਸ਼ਾਨੀ ਸ਼ਬਦ_3 = Lo_ਸ਼ਬਦ + 1; ਸ਼ਬਦ_3 < Hi_ਸ਼ਬਦ; ++ਸ਼ਬਦ_3)
    U.pVal[ਸ਼ਬਦ_3] = 0;
}

// Complement a bignum in-place.
ਸਥਿਰ ਖਾਲੀ tcComplement(APInt::ਸ਼ਬਦ_Type *dst, ਬਿਨਾਨਿਸ਼ਾਨੀ parts) {
  ਲਈ (ਬਿਨਾਨਿਸ਼ਾਨੀ i = 0; i < parts; i++)
    dst[i] = ~dst[i];
}

/// Toggle every bit to its opposite value.
ਖਾਲੀ APInt::flip_All_ਬਿੱਟ_Slow_Case() {
  tcComplement(U.pVal, get_ਅੰਕ_ਸ਼ਬਦ());
  clear_ਅਣਵਰਤਿਆ_ਬਿੱਟ();
}

/// Concatenate the bits from "NewLSB" onto the bottom of *this.  This is
/// equivalent to:
///   (this->zext(NewWidth) << NewLSB.getBitWidth()) | NewLSB.zext(NewWidth)
/// In the slow case, we know the result is large.
APInt APInt::concatSlowCase(ਅਚਲ APInt &NewLSB) ਅਚਲ {
  ਬਿਨਾਨਿਸ਼ਾਨੀ New_ਚੌੜਾਈ = get_ਦੋਆਧਾਰੀ_ਅੰਕ_ਚੌੜਾਈ() + NewLSB.get_ਦੋਆਧਾਰੀ_ਅੰਕ_ਚੌੜਾਈ();
  APInt ਨਤੀਜਾ = NewLSB.zext(New_ਚੌੜਾਈ);
  ਨਤੀਜਾ.ਪਾਓ_ਬਿੱਟ(*ਇਹ, NewLSB.get_ਦੋਆਧਾਰੀ_ਅੰਕ_ਚੌੜਾਈ());
  ਵਾਪਸ ਨਤੀਜਾ;
}

/// Toggle a given bit to its opposite value whose position is given
/// as "bitPosition".
/// Toggles a given bit to its opposite value.
ਖਾਲੀ APInt::flip_ਦੋਆਧਾਰੀ_ਅੰਕ(ਬਿਨਾਨਿਸ਼ਾਨੀ ਦੋਆਧਾਰੀ_ਅੰਕ_Position) {
  ਪ੍ਰਤਿਜ੍ਞਾ(ਦੋਆਧਾਰੀ_ਅੰਕ_Position < ਦੋਆਧਾਰੀ_ਅੰਕ_ਚੌੜਾਈ_2 && "Out of the bit-width range!");
  ਸੈੱਟ_ਦੋਆਧਾਰੀ_ਅੰਕ_Val(ਦੋਆਧਾਰੀ_ਅੰਕ_Position, !(*ਇਹ)[ਦੋਆਧਾਰੀ_ਅੰਕ_Position]);
}

ਖਾਲੀ APInt::ਪਾਓ_ਬਿੱਟ(ਅਚਲ APInt &sub_ਬਿੱਟ, ਬਿਨਾਨਿਸ਼ਾਨੀ ਦੋਆਧਾਰੀ_ਅੰਕ_Position) {
  ਬਿਨਾਨਿਸ਼ਾਨੀ sub_ਦੋਆਧਾਰੀ_ਅੰਕ_ਚੌੜਾਈ = sub_ਬਿੱਟ.get_ਦੋਆਧਾਰੀ_ਅੰਕ_ਚੌੜਾਈ();
  ਪ੍ਰਤਿਜ੍ਞਾ((sub_ਦੋਆਧਾਰੀ_ਅੰਕ_ਚੌੜਾਈ + ਦੋਆਧਾਰੀ_ਅੰਕ_Position) <= ਦੋਆਧਾਰੀ_ਅੰਕ_ਚੌੜਾਈ_2 && "Illegal bit insertion");

  // inserting no bits is a noop.
  ਜੇ (sub_ਦੋਆਧਾਰੀ_ਅੰਕ_ਚੌੜਾਈ == 0)
    ਵਾਪਸ;

  // Insertion is a direct copy.
  ਜੇ (sub_ਦੋਆਧਾਰੀ_ਅੰਕ_ਚੌੜਾਈ == ਦੋਆਧਾਰੀ_ਅੰਕ_ਚੌੜਾਈ_2) {
    *ਇਹ = sub_ਬਿੱਟ;
    ਵਾਪਸ;
  }

  // Single word result can be done as a direct bitmask.
  ਜੇ (LLVM_LIKELY(is_Single_ਸ਼ਬਦ())) {
    uint64_t ਨਕਾਬ_2 = WORDTYPE_MAX >> (APINT_ਬਿੱਟ_ਪ੍ਰਤੀ_ਸ਼ਬਦ - sub_ਦੋਆਧਾਰੀ_ਅੰਕ_ਚੌੜਾਈ);
    U.VAL &= ~(ਨਕਾਬ_2 << ਦੋਆਧਾਰੀ_ਅੰਕ_Position);
    U.VAL |= (sub_ਬਿੱਟ.U.VAL << ਦੋਆਧਾਰੀ_ਅੰਕ_Position);
    ਵਾਪਸ;
  }

  ਬਿਨਾਨਿਸ਼ਾਨੀ lo_ਦੋਆਧਾਰੀ_ਅੰਕ = which_ਦੋਆਧਾਰੀ_ਅੰਕ(ਦੋਆਧਾਰੀ_ਅੰਕ_Position);
  ਬਿਨਾਨਿਸ਼ਾਨੀ lo_ਸ਼ਬਦ = which_ਸ਼ਬਦ(ਦੋਆਧਾਰੀ_ਅੰਕ_Position);
  ਬਿਨਾਨਿਸ਼ਾਨੀ hi1_ਸ਼ਬਦ = which_ਸ਼ਬਦ(ਦੋਆਧਾਰੀ_ਅੰਕ_Position + sub_ਦੋਆਧਾਰੀ_ਅੰਕ_ਚੌੜਾਈ - 1);

  // Insertion within a single word can be done as a direct bitmask.
  ਜੇ (lo_ਸ਼ਬਦ == hi1_ਸ਼ਬਦ) {
    uint64_t ਨਕਾਬ_2 = WORDTYPE_MAX >> (APINT_ਬਿੱਟ_ਪ੍ਰਤੀ_ਸ਼ਬਦ - sub_ਦੋਆਧਾਰੀ_ਅੰਕ_ਚੌੜਾਈ);
    U.pVal[lo_ਸ਼ਬਦ] &= ~(ਨਕਾਬ_2 << lo_ਦੋਆਧਾਰੀ_ਅੰਕ);
    U.pVal[lo_ਸ਼ਬਦ] |= (sub_ਬਿੱਟ.U.VAL << lo_ਦੋਆਧਾਰੀ_ਅੰਕ);
    ਵਾਪਸ;
  }

  // Insert on word boundaries.
  ਜੇ (lo_ਦੋਆਧਾਰੀ_ਅੰਕ == 0) {
    // Direct copy whole words.
    ਬਿਨਾਨਿਸ਼ਾਨੀ ਅੰਕ_ਸਾਰਾ_Sub_ਸ਼ਬਦ = sub_ਦੋਆਧਾਰੀ_ਅੰਕ_ਚੌੜਾਈ / APINT_ਬਿੱਟ_ਪ੍ਰਤੀ_ਸ਼ਬਦ;
    ਸਮ੍ਰਿਤੀ_ਨਕਲ(U.pVal + lo_ਸ਼ਬਦ, sub_ਬਿੱਟ.get_Raw_ਅੰਕੜੇ(),
           ਅੰਕ_ਸਾਰਾ_Sub_ਸ਼ਬਦ * APINT_ਸ਼ਬਦ_ਮਾਪ);

    // Mask+insert remaining bits.
    ਬਿਨਾਨਿਸ਼ਾਨੀ ਬਚਿਆ_ਬਿੱਟ = sub_ਦੋਆਧਾਰੀ_ਅੰਕ_ਚੌੜਾਈ % APINT_ਬਿੱਟ_ਪ੍ਰਤੀ_ਸ਼ਬਦ;
    ਜੇ (ਬਚਿਆ_ਬਿੱਟ != 0) {
      uint64_t ਨਕਾਬ_2 = WORDTYPE_MAX >> (APINT_ਬਿੱਟ_ਪ੍ਰਤੀ_ਸ਼ਬਦ - ਬਚਿਆ_ਬਿੱਟ);
      U.pVal[hi1_ਸ਼ਬਦ] &= ~ਨਕਾਬ_2;
      U.pVal[hi1_ਸ਼ਬਦ] |= sub_ਬਿੱਟ.get_ਸ਼ਬਦ(sub_ਦੋਆਧਾਰੀ_ਅੰਕ_ਚੌੜਾਈ - 1);
    }
    ਵਾਪਸ;
  }

  // General case - set/clear individual bits in dst based on src.
  // TODO - there is scope for optimization here, but at the moment this code
  // path is barely used so prefer readability over performance.
  ਲਈ (ਬਿਨਾਨਿਸ਼ਾਨੀ i = 0; i != sub_ਦੋਆਧਾਰੀ_ਅੰਕ_ਚੌੜਾਈ; ++i)
    ਸੈੱਟ_ਦੋਆਧਾਰੀ_ਅੰਕ_Val(ਦੋਆਧਾਰੀ_ਅੰਕ_Position + i, sub_ਬਿੱਟ[i]);
}

ਖਾਲੀ APInt::ਪਾਓ_ਬਿੱਟ(uint64_t sub_ਬਿੱਟ, ਬਿਨਾਨਿਸ਼ਾਨੀ ਦੋਆਧਾਰੀ_ਅੰਕ_Position, ਬਿਨਾਨਿਸ਼ਾਨੀ ਅੰਕ_ਬਿੱਟ) {
  uint64_t ਨਕਾਬ_ਬਿੱਟ = ਨਕਾਬ_Trailing_Ones<uint64_t>(ਅੰਕ_ਬਿੱਟ);
  sub_ਬਿੱਟ &= ਨਕਾਬ_ਬਿੱਟ;
  ਜੇ (LLVM_LIKELY(is_Single_ਸ਼ਬਦ())) {
    U.VAL &= ~(ਨਕਾਬ_ਬਿੱਟ << ਦੋਆਧਾਰੀ_ਅੰਕ_Position);
    U.VAL |= sub_ਬਿੱਟ << ਦੋਆਧਾਰੀ_ਅੰਕ_Position;
    ਵਾਪਸ;
  }

  ਬਿਨਾਨਿਸ਼ਾਨੀ lo_ਦੋਆਧਾਰੀ_ਅੰਕ = which_ਦੋਆਧਾਰੀ_ਅੰਕ(ਦੋਆਧਾਰੀ_ਅੰਕ_Position);
  ਬਿਨਾਨਿਸ਼ਾਨੀ lo_ਸ਼ਬਦ = which_ਸ਼ਬਦ(ਦੋਆਧਾਰੀ_ਅੰਕ_Position);
  ਬਿਨਾਨਿਸ਼ਾਨੀ hi_ਸ਼ਬਦ = which_ਸ਼ਬਦ(ਦੋਆਧਾਰੀ_ਅੰਕ_Position + ਅੰਕ_ਬਿੱਟ - 1);
  ਜੇ (lo_ਸ਼ਬਦ == hi_ਸ਼ਬਦ) {
    U.pVal[lo_ਸ਼ਬਦ] &= ~(ਨਕਾਬ_ਬਿੱਟ << lo_ਦੋਆਧਾਰੀ_ਅੰਕ);
    U.pVal[lo_ਸ਼ਬਦ] |= sub_ਬਿੱਟ << lo_ਦੋਆਧਾਰੀ_ਅੰਕ;
    ਵਾਪਸ;
  }

  static_assert(8 * ਆਕਾਰ(ਸ਼ਬਦ_Type) <= 64, "This code assumes only two words affected");
  ਬਿਨਾਨਿਸ਼ਾਨੀ ਸ਼ਬਦ_ਬਿੱਟ = 8 * ਆਕਾਰ(ਸ਼ਬਦ_Type);
  U.pVal[lo_ਸ਼ਬਦ] &= ~(ਨਕਾਬ_ਬਿੱਟ << lo_ਦੋਆਧਾਰੀ_ਅੰਕ);
  U.pVal[lo_ਸ਼ਬਦ] |= sub_ਬਿੱਟ << lo_ਦੋਆਧਾਰੀ_ਅੰਕ;

  U.pVal[hi_ਸ਼ਬਦ] &= ~(ਨਕਾਬ_ਬਿੱਟ >> (ਸ਼ਬਦ_ਬਿੱਟ - lo_ਦੋਆਧਾਰੀ_ਅੰਕ));
  U.pVal[hi_ਸ਼ਬਦ] |= sub_ਬਿੱਟ >> (ਸ਼ਬਦ_ਬਿੱਟ - lo_ਦੋਆਧਾਰੀ_ਅੰਕ);
}

APInt APInt::ਕੱਢੋ_ਬਿੱਟ(ਬਿਨਾਨਿਸ਼ਾਨੀ ਅੰਕ_ਬਿੱਟ, ਬਿਨਾਨਿਸ਼ਾਨੀ ਦੋਆਧਾਰੀ_ਅੰਕ_Position) ਅਚਲ {
  ਪ੍ਰਤਿਜ੍ਞਾ(ਦੋਆਧਾਰੀ_ਅੰਕ_Position < ਦੋਆਧਾਰੀ_ਅੰਕ_ਚੌੜਾਈ_2 && (ਅੰਕ_ਬਿੱਟ + ਦੋਆਧਾਰੀ_ਅੰਕ_Position) <= ਦੋਆਧਾਰੀ_ਅੰਕ_ਚੌੜਾਈ_2 &&
         "Illegal bit extraction");

  ਜੇ (LLVM_LIKELY(is_Single_ਸ਼ਬਦ()))
    ਵਾਪਸ APInt(ਅੰਕ_ਬਿੱਟ, U.VAL >> ਦੋਆਧਾਰੀ_ਅੰਕ_Position, /*isSigned=*/ਝੂਠ,
                 /*implicitTrunc=*/ਸੱਚ);

  ਬਿਨਾਨਿਸ਼ਾਨੀ lo_ਦੋਆਧਾਰੀ_ਅੰਕ = which_ਦੋਆਧਾਰੀ_ਅੰਕ(ਦੋਆਧਾਰੀ_ਅੰਕ_Position);
  ਬਿਨਾਨਿਸ਼ਾਨੀ lo_ਸ਼ਬਦ = which_ਸ਼ਬਦ(ਦੋਆਧਾਰੀ_ਅੰਕ_Position);
  ਬਿਨਾਨਿਸ਼ਾਨੀ hi_ਸ਼ਬਦ = which_ਸ਼ਬਦ(ਦੋਆਧਾਰੀ_ਅੰਕ_Position + ਅੰਕ_ਬਿੱਟ - 1);

  // Single word result extracting bits from a single word source.
  ਜੇ (lo_ਸ਼ਬਦ == hi_ਸ਼ਬਦ)
    ਵਾਪਸ APInt(ਅੰਕ_ਬਿੱਟ, U.pVal[lo_ਸ਼ਬਦ] >> lo_ਦੋਆਧਾਰੀ_ਅੰਕ, /*isSigned=*/ਝੂਠ,
                 /*implicitTrunc=*/ਸੱਚ);

  // Extracting bits that start on a source word boundary can be done
  // as a fast memory copy.
  ਜੇ (lo_ਦੋਆਧਾਰੀ_ਅੰਕ == 0)
    ਵਾਪਸ APInt(ਅੰਕ_ਬਿੱਟ, ArrayRef(U.pVal + lo_ਸ਼ਬਦ, 1 + hi_ਸ਼ਬਦ - lo_ਸ਼ਬਦ));

  // General case - shift + copy source words directly into place.
  APInt ਨਤੀਜਾ(ਅੰਕ_ਬਿੱਟ, 0);
  ਬਿਨਾਨਿਸ਼ਾਨੀ ਅੰਕ_Src_ਸ਼ਬਦ = get_ਅੰਕ_ਸ਼ਬਦ();
  ਬਿਨਾਨਿਸ਼ਾਨੀ ਅੰਕ_Dst_ਸ਼ਬਦ = ਨਤੀਜਾ.get_ਅੰਕ_ਸ਼ਬਦ();

  uint64_t *DestPtr =
      LLVM_LIKELY(ਨਤੀਜਾ.is_Single_ਸ਼ਬਦ()) ? &ਨਤੀਜਾ.U.VAL : ਨਤੀਜਾ.U.pVal;
  ਲਈ (ਬਿਨਾਨਿਸ਼ਾਨੀ ਸ਼ਬਦ_2 = 0; ਸ਼ਬਦ_2 < ਅੰਕ_Dst_ਸ਼ਬਦ; ++ਸ਼ਬਦ_2) {
    uint64_t w0 = U.pVal[lo_ਸ਼ਬਦ + ਸ਼ਬਦ_2];
    uint64_t w1 =
        (lo_ਸ਼ਬਦ + ਸ਼ਬਦ_2 + 1) < ਅੰਕ_Src_ਸ਼ਬਦ ? U.pVal[lo_ਸ਼ਬਦ + ਸ਼ਬਦ_2 + 1] : 0;
    DestPtr[ਸ਼ਬਦ_2] = (w0 >> lo_ਦੋਆਧਾਰੀ_ਅੰਕ) | (w1 << (APINT_ਬਿੱਟ_ਪ੍ਰਤੀ_ਸ਼ਬਦ - lo_ਦੋਆਧਾਰੀ_ਅੰਕ));
  }

  ਵਾਪਸ ਨਤੀਜਾ.clear_ਅਣਵਰਤਿਆ_ਬਿੱਟ();
}

uint64_t APInt::ਕੱਢੋ_ਬਿੱਟ_As_ZExt_ਮੁੱਲ(ਬਿਨਾਨਿਸ਼ਾਨੀ ਅੰਕ_ਬਿੱਟ,
                                       ਬਿਨਾਨਿਸ਼ਾਨੀ ਦੋਆਧਾਰੀ_ਅੰਕ_Position) ਅਚਲ {
  ਪ੍ਰਤਿਜ੍ਞਾ(ਦੋਆਧਾਰੀ_ਅੰਕ_Position < ਦੋਆਧਾਰੀ_ਅੰਕ_ਚੌੜਾਈ_2 && (ਅੰਕ_ਬਿੱਟ + ਦੋਆਧਾਰੀ_ਅੰਕ_Position) <= ਦੋਆਧਾਰੀ_ਅੰਕ_ਚੌੜਾਈ_2 &&
         "Illegal bit extraction");
  ਪ੍ਰਤਿਜ੍ਞਾ(ਅੰਕ_ਬਿੱਟ <= 64 && "Illegal bit extraction");

  uint64_t ਨਕਾਬ_ਬਿੱਟ = ਨਕਾਬ_Trailing_Ones<uint64_t>(ਅੰਕ_ਬਿੱਟ);
  ਜੇ (LLVM_LIKELY(is_Single_ਸ਼ਬਦ()))
    ਵਾਪਸ (U.VAL >> ਦੋਆਧਾਰੀ_ਅੰਕ_Position) & ਨਕਾਬ_ਬਿੱਟ;

  static_assert(APINT_ਬਿੱਟ_ਪ੍ਰਤੀ_ਸ਼ਬਦ >= 64,
                "This code assumes only two words affected");
  ਬਿਨਾਨਿਸ਼ਾਨੀ lo_ਦੋਆਧਾਰੀ_ਅੰਕ = which_ਦੋਆਧਾਰੀ_ਅੰਕ(ਦੋਆਧਾਰੀ_ਅੰਕ_Position);
  ਬਿਨਾਨਿਸ਼ਾਨੀ lo_ਸ਼ਬਦ = which_ਸ਼ਬਦ(ਦੋਆਧਾਰੀ_ਅੰਕ_Position);
  ਬਿਨਾਨਿਸ਼ਾਨੀ hi_ਸ਼ਬਦ = which_ਸ਼ਬਦ(ਦੋਆਧਾਰੀ_ਅੰਕ_Position + ਅੰਕ_ਬਿੱਟ - 1);
  ਜੇ (lo_ਸ਼ਬਦ == hi_ਸ਼ਬਦ)
    ਵਾਪਸ (U.pVal[lo_ਸ਼ਬਦ] >> lo_ਦੋਆਧਾਰੀ_ਅੰਕ) & ਨਕਾਬ_ਬਿੱਟ;

  uint64_t ret_ਬਿੱਟ = U.pVal[lo_ਸ਼ਬਦ] >> lo_ਦੋਆਧਾਰੀ_ਅੰਕ;
  ret_ਬਿੱਟ |= U.pVal[hi_ਸ਼ਬਦ] << (APINT_ਬਿੱਟ_ਪ੍ਰਤੀ_ਸ਼ਬਦ - lo_ਦੋਆਧਾਰੀ_ਅੰਕ);
  ret_ਬਿੱਟ &= ਨਕਾਬ_ਬਿੱਟ;
  ਵਾਪਸ ret_ਬਿੱਟ;
}

ਬਿਨਾਨਿਸ਼ਾਨੀ APInt::get_Sufficient_ਬਿੱਟ_Needed(StringRef Str, uint8_t Radix) {
  ਪ੍ਰਤਿਜ੍ਞਾ(!Str.ਖਾਲੀ_ਥਾਵਾਂ() && "Invalid string length");
  size_t StrLen = Str.ਮਾਪ_2();

  // Each computation below needs to know if it's negative.
  ਬਿਨਾਨਿਸ਼ਾਨੀ IsNegative = ਝੂਠ;
  ਜੇ (Str[0] == '-' || Str[0] == '+') {
    IsNegative = Str[0] == '-';
    StrLen--;
    ਪ੍ਰਤਿਜ੍ਞਾ(StrLen && "String is only a sign, needs a value.");
  }

  // For radixes of power-of-two values, the bits required is accurately and
  // easily computed.
  ਜੇ (Radix == 2)
    ਵਾਪਸ StrLen + IsNegative;
  ਜੇ (Radix == 8)
    ਵਾਪਸ StrLen * 3 + IsNegative;
  ਜੇ (Radix == 16)
    ਵਾਪਸ StrLen * 4 + IsNegative;

  // Compute a sufficient number of bits that is always large enough but might
  // be too large. This avoids the assertion in the constructor. This
  // calculation doesn't work appropriately for the numbers 0-9, so just use 4
  // bits in that case.
  ਜੇ (Radix == 10)
    ਵਾਪਸ (StrLen == 1 ? 4 : StrLen * 64 / 18) + IsNegative;

  ਪ੍ਰਤਿਜ੍ਞਾ(Radix == 36);
  ਵਾਪਸ (StrLen == 1 ? 7 : StrLen * 16 / 3) + IsNegative;
}

ਬਿਨਾਨਿਸ਼ਾਨੀ APInt::get_ਬਿੱਟ_Needed(StringRef str, uint8_t radix) {
  // Compute a sufficient number of bits that is always large enough but might
  // be too large.
  ਬਿਨਾਨਿਸ਼ਾਨੀ sufficient = get_Sufficient_ਬਿੱਟ_Needed(str, radix);

  // For bases 2, 8, and 16, the sufficient number of bits is exact and we can
  // return the value directly. For bases 10 and 36, we need to do extra work.
  ਜੇ (radix == 2 || radix == 8 || radix == 16)
    ਵਾਪਸ sufficient;

  // This is grossly inefficient but accurate. We could probably do something
  // with a computation of roughly slen*64/20 and then adjust by the value of
  // the first few digits. But, I'm not sure how accurate that could be.
  size_t slen = str.ਮਾਪ_2();

  // Each computation below needs to know if it's negative.
  StringRef::iterator p = str.begin();
  ਬਿਨਾਨਿਸ਼ਾਨੀ isNegative = *p == '-';
  ਜੇ (*p == '-' || *p == '+') {
    p++;
    slen--;
    ਪ੍ਰਤਿਜ੍ਞਾ(slen && "String is only a sign, needs a value.");
  }


  // Convert to the actual binary value.
  APInt tmp(sufficient, StringRef(p, slen), radix);

  // Compute how many bits are required. If the log is infinite, assume we need
  // just bit. If the log is exact and value is negative, then the value is
  // MinSignedValue with (log + 1) bits.
  ਬਿਨਾਨਿਸ਼ਾਨੀ ਲਘੂਗਣਕ = tmp.logBase2();
  ਜੇ (ਲਘੂਗਣਕ == (ਬਿਨਾਨਿਸ਼ਾਨੀ)-1) {
    ਵਾਪਸ isNegative + 1;
  } ਨਹੀਂਤਾਂ ਜੇ (isNegative && tmp.isPowerOf2()) {
    ਵਾਪਸ isNegative + ਲਘੂਗਣਕ;
  } ਨਹੀਂਤਾਂ {
    ਵਾਪਸ isNegative + ਲਘੂਗਣਕ + 1;
  }
}

hash_ਕੋਡ llvm::hash_ਮੁੱਲ(ਅਚਲ APInt &Arg) {
  ਜੇ (LLVM_LIKELY(Arg.is_Single_ਸ਼ਬਦ()))
    ਵਾਪਸ hash_combine(Arg.ਦੋਆਧਾਰੀ_ਅੰਕ_ਚੌੜਾਈ_2, Arg.U.VAL);

  ਵਾਪਸ hash_combine(
      Arg.ਦੋਆਧਾਰੀ_ਅੰਕ_ਚੌੜਾਈ_2,
      hash_combine_range(Arg.U.pVal, Arg.U.pVal + Arg.get_ਅੰਕ_ਸ਼ਬਦ()));
}

ਬਿਨਾਨਿਸ਼ਾਨੀ DenseMapInfo<APInt, ਖਾਲੀ>::get_Hash_ਮੁੱਲ(ਅਚਲ APInt &ਕੁੰਜੀ) {
  ਵਾਪਸ static_cast<ਬਿਨਾਨਿਸ਼ਾਨੀ>(hash_ਮੁੱਲ(ਕੁੰਜੀ));
}

ਤਾਰਕਿਕ APInt::isSplat(ਬਿਨਾਨਿਸ਼ਾਨੀ Splat_ਮਾਪ_ਵਿੱਚ_ਬਿੱਟ) ਅਚਲ {
  ਪ੍ਰਤਿਜ੍ਞਾ(get_ਦੋਆਧਾਰੀ_ਅੰਕ_ਚੌੜਾਈ() % Splat_ਮਾਪ_ਵਿੱਚ_ਬਿੱਟ == 0 &&
         "SplatSizeInBits must divide width!");
  // We can check that all parts of an integer are equal by making use of a
  // little trick: rotate and check if it's still the same value.
  ਵਾਪਸ *ਇਹ == rotl(Splat_ਮਾਪ_ਵਿੱਚ_ਬਿੱਟ);
}

/// This function returns the high "numBits" bits of this APInt.
APInt APInt::get_Hi_ਬਿੱਟ(ਬਿਨਾਨਿਸ਼ਾਨੀ ਅੰਕ_ਬਿੱਟ) ਅਚਲ {
  ਵਾਪਸ ਇਹ->lshr(ਦੋਆਧਾਰੀ_ਅੰਕ_ਚੌੜਾਈ_2 - ਅੰਕ_ਬਿੱਟ);
}

/// This function returns the low "numBits" bits of this APInt.
APInt APInt::get_Lo_ਬਿੱਟ(ਬਿਨਾਨਿਸ਼ਾਨੀ ਅੰਕ_ਬਿੱਟ) ਅਚਲ {
  APInt ਨਤੀਜਾ(get_ਨੀਵਾਂ_ਬਿੱਟ_ਸੈੱਟ(ਦੋਆਧਾਰੀ_ਅੰਕ_ਚੌੜਾਈ_2, ਅੰਕ_ਬਿੱਟ));
  ਨਤੀਜਾ &= *ਇਹ;
  ਵਾਪਸ ਨਤੀਜਾ;
}

/// Return a value containing V broadcasted over NewLen bits.
APInt APInt::getSplat(ਬਿਨਾਨਿਸ਼ਾਨੀ NewLen, ਅਚਲ APInt &V) {
  ਪ੍ਰਤਿਜ੍ਞਾ(NewLen >= V.get_ਦੋਆਧਾਰੀ_ਅੰਕ_ਚੌੜਾਈ() && "Can't splat to smaller bit width!");

  APInt Val = V.zext(NewLen);
  ਲਈ (ਬਿਨਾਨਿਸ਼ਾਨੀ I = V.get_ਦੋਆਧਾਰੀ_ਅੰਕ_ਚੌੜਾਈ(); I < NewLen; I <<= 1)
    Val |= Val << I;

  ਵਾਪਸ Val;
}

ਬਿਨਾਨਿਸ਼ਾਨੀ APInt::ਗਿਣਤੀ_Leading_Zeros_Slow_Case() ਅਚਲ {
  ਬਿਨਾਨਿਸ਼ਾਨੀ ਗਿਣਤੀ_3 = 0;
  ਲਈ (ਪੂਰਨ i = get_ਅੰਕ_ਸ਼ਬਦ() - 1; i >= 0; --i) {
    uint64_t V = U.pVal[i];
    ਜੇ (V == 0)
      ਗਿਣਤੀ_3 += APINT_ਬਿੱਟ_ਪ੍ਰਤੀ_ਸ਼ਬਦ;
    ਨਹੀਂਤਾਂ {
      ਗਿਣਤੀ_3 += llvm::countl_zero(V);
      ਰੋਕੋ;
    }
  }
  // Adjust for unused bits in the most significant word (they are zero).
  ਬਿਨਾਨਿਸ਼ਾਨੀ Mod = ਦੋਆਧਾਰੀ_ਅੰਕ_ਚੌੜਾਈ_2 % APINT_ਬਿੱਟ_ਪ੍ਰਤੀ_ਸ਼ਬਦ;
  ਗਿਣਤੀ_3 -= Mod > 0 ? APINT_ਬਿੱਟ_ਪ੍ਰਤੀ_ਸ਼ਬਦ - Mod : 0;
  ਵਾਪਸ ਗਿਣਤੀ_3;
}

ਬਿਨਾਨਿਸ਼ਾਨੀ APInt::ਗਿਣਤੀ_Leading_Ones_Slow_Case() ਅਚਲ {
  ਬਿਨਾਨਿਸ਼ਾਨੀ ਉੱਚਾ_ਸ਼ਬਦ_ਬਿੱਟ = ਦੋਆਧਾਰੀ_ਅੰਕ_ਚੌੜਾਈ_2 % APINT_ਬਿੱਟ_ਪ੍ਰਤੀ_ਸ਼ਬਦ;
  ਬਿਨਾਨਿਸ਼ਾਨੀ ਖਿਸਕਾਓ;
  ਜੇ (!ਉੱਚਾ_ਸ਼ਬਦ_ਬਿੱਟ) {
    ਉੱਚਾ_ਸ਼ਬਦ_ਬਿੱਟ = APINT_ਬਿੱਟ_ਪ੍ਰਤੀ_ਸ਼ਬਦ;
    ਖਿਸਕਾਓ = 0;
  } ਨਹੀਂਤਾਂ {
    ਖਿਸਕਾਓ = APINT_ਬਿੱਟ_ਪ੍ਰਤੀ_ਸ਼ਬਦ - ਉੱਚਾ_ਸ਼ਬਦ_ਬਿੱਟ;
  }
  ਪੂਰਨ i = get_ਅੰਕ_ਸ਼ਬਦ() - 1;
  ਬਿਨਾਨਿਸ਼ਾਨੀ ਗਿਣਤੀ_3 = llvm::countl_one(U.pVal[i] << ਖਿਸਕਾਓ);
  ਜੇ (ਗਿਣਤੀ_3 == ਉੱਚਾ_ਸ਼ਬਦ_ਬਿੱਟ) {
    ਲਈ (i--; i >= 0; --i) {
      ਜੇ (U.pVal[i] == WORDTYPE_MAX)
        ਗਿਣਤੀ_3 += APINT_ਬਿੱਟ_ਪ੍ਰਤੀ_ਸ਼ਬਦ;
      ਨਹੀਂਤਾਂ {
        ਗਿਣਤੀ_3 += llvm::countl_one(U.pVal[i]);
        ਰੋਕੋ;
      }
    }
  }
  ਵਾਪਸ ਗਿਣਤੀ_3;
}

ਬਿਨਾਨਿਸ਼ਾਨੀ APInt::ਗਿਣਤੀ_Trailing_Zeros_Slow_Case() ਅਚਲ {
  ਬਿਨਾਨਿਸ਼ਾਨੀ ਗਿਣਤੀ_3 = 0;
  ਬਿਨਾਨਿਸ਼ਾਨੀ i = 0;
  ਲਈ (; i < get_ਅੰਕ_ਸ਼ਬਦ() && U.pVal[i] == 0; ++i)
    ਗਿਣਤੀ_3 += APINT_ਬਿੱਟ_ਪ੍ਰਤੀ_ਸ਼ਬਦ;
  ਜੇ (i < get_ਅੰਕ_ਸ਼ਬਦ())
    ਗਿਣਤੀ_3 += llvm::countr_zero(U.pVal[i]);
  ਵਾਪਸ std::min(ਗਿਣਤੀ_3, ਦੋਆਧਾਰੀ_ਅੰਕ_ਚੌੜਾਈ_2);
}

ਬਿਨਾਨਿਸ਼ਾਨੀ APInt::ਗਿਣਤੀ_Trailing_Ones_Slow_Case() ਅਚਲ {
  ਬਿਨਾਨਿਸ਼ਾਨੀ ਗਿਣਤੀ_3 = 0;
  ਬਿਨਾਨਿਸ਼ਾਨੀ i = 0;
  ਲਈ (; i < get_ਅੰਕ_ਸ਼ਬਦ() && U.pVal[i] == WORDTYPE_MAX; ++i)
    ਗਿਣਤੀ_3 += APINT_ਬਿੱਟ_ਪ੍ਰਤੀ_ਸ਼ਬਦ;
  ਜੇ (i < get_ਅੰਕ_ਸ਼ਬਦ())
    ਗਿਣਤੀ_3 += llvm::countr_one(U.pVal[i]);
  ਪ੍ਰਤਿਜ੍ਞਾ(ਗਿਣਤੀ_3 <= ਦੋਆਧਾਰੀ_ਅੰਕ_ਚੌੜਾਈ_2);
  ਵਾਪਸ ਗਿਣਤੀ_3;
}

ਬਿਨਾਨਿਸ਼ਾਨੀ APInt::ਗਿਣਤੀ_Population_Slow_Case() ਅਚਲ {
  ਬਿਨਾਨਿਸ਼ਾਨੀ ਗਿਣਤੀ_3 = 0;
  ਲਈ (ਬਿਨਾਨਿਸ਼ਾਨੀ i = 0; i < get_ਅੰਕ_ਸ਼ਬਦ(); ++i)
    ਗਿਣਤੀ_3 += llvm::popcount(U.pVal[i]);
  ਵਾਪਸ ਗਿਣਤੀ_3;
}

ਤਾਰਕਿਕ APInt::isPowerOf2SlowCase() ਅਚਲ {
  ਬਿਨਾਨਿਸ਼ਾਨੀ ਗਿਣਤੀ_3 = 0;
  ਲਈ (ਬਿਨਾਨਿਸ਼ਾਨੀ i = 0; i < get_ਅੰਕ_ਸ਼ਬਦ(); ++i) {
    ਗਿਣਤੀ_3 += llvm::popcount(U.pVal[i]);
    ਜੇ (ਗਿਣਤੀ_3 > 1)
      ਵਾਪਸ ਝੂਠ;
  }
  ਵਾਪਸ ਗਿਣਤੀ_3 == 1;
}

ਤਾਰਕਿਕ APInt::intersectsSlowCase(ਅਚਲ APInt &RHS) ਅਚਲ {
  ਲਈ (ਬਿਨਾਨਿਸ਼ਾਨੀ i = 0, e = get_ਅੰਕ_ਸ਼ਬਦ(); i != e; ++i)
    ਜੇ ((U.pVal[i] & RHS.U.pVal[i]) != 0)
      ਵਾਪਸ ਸੱਚ;

  ਵਾਪਸ ਝੂਠ;
}

ਤਾਰਕਿਕ APInt::is_Subset_ਦਾ_Slow_Case(ਅਚਲ APInt &RHS) ਅਚਲ {
  ਲਈ (ਬਿਨਾਨਿਸ਼ਾਨੀ i = 0, e = get_ਅੰਕ_ਸ਼ਬਦ(); i != e; ++i)
    ਜੇ ((U.pVal[i] & ~RHS.U.pVal[i]) != 0)
      ਵਾਪਸ ਝੂਠ;

  ਵਾਪਸ ਸੱਚ;
}

ਤਾਰਕਿਕ APInt::is_Inverse_ਦਾ_Slow_Case(ਅਚਲ APInt &RHS) ਅਚਲ {
  ਅਚਲ ਬਿਨਾਨਿਸ਼ਾਨੀ ਆਖਰੀ = get_ਅੰਕ_ਸ਼ਬਦ() - 1;
  ਲਈ (ਬਿਨਾਨਿਸ਼ਾਨੀ I = 0; I != ਆਖਰੀ; ++I)
    ਜੇ ((U.pVal[I] ^ RHS.U.pVal[I]) != WORDTYPE_MAX)
      ਵਾਪਸ ਝੂਠ;

  ਬਿਨਾਨਿਸ਼ਾਨੀ ਪੂਛ_ਬਿੱਟ = ਦੋਆਧਾਰੀ_ਅੰਕ_ਚੌੜਾਈ_2 - ਆਖਰੀ * APINT_ਬਿੱਟ_ਪ੍ਰਤੀ_ਸ਼ਬਦ;
  ਸ਼ਬਦ_Type ਪੂਛ_ਨਕਾਬ = llvm::ਨਕਾਬ_Trailing_Ones<ਸ਼ਬਦ_Type>(ਪੂਛ_ਬਿੱਟ);
  ਵਾਪਸ (U.pVal[ਆਖਰੀ] ^ RHS.U.pVal[ਆਖਰੀ]) == ਪੂਛ_ਨਕਾਬ;
}

APInt APInt::byte_ਅਦਲਾ_ਬਦਲੀ() ਅਚਲ {
  ਪ੍ਰਤਿਜ੍ਞਾ(ਦੋਆਧਾਰੀ_ਅੰਕ_ਚੌੜਾਈ_2 >= 16 && ਦੋਆਧਾਰੀ_ਅੰਕ_ਚੌੜਾਈ_2 % 8 == 0 && "Cannot byteswap!");
  ਜੇ (ਦੋਆਧਾਰੀ_ਅੰਕ_ਚੌੜਾਈ_2 == 16)
    ਵਾਪਸ APInt(ਦੋਆਧਾਰੀ_ਅੰਕ_ਚੌੜਾਈ_2, llvm::byteswap<uint16_t>(U.VAL));
  ਜੇ (ਦੋਆਧਾਰੀ_ਅੰਕ_ਚੌੜਾਈ_2 == 32)
    ਵਾਪਸ APInt(ਦੋਆਧਾਰੀ_ਅੰਕ_ਚੌੜਾਈ_2, llvm::byteswap<uint32_t>(U.VAL));
  ਜੇ (ਦੋਆਧਾਰੀ_ਅੰਕ_ਚੌੜਾਈ_2 <= 64) {
    uint64_t Tmp1 = llvm::byteswap<uint64_t>(U.VAL);
    Tmp1 >>= (64 - ਦੋਆਧਾਰੀ_ਅੰਕ_ਚੌੜਾਈ_2);
    ਵਾਪਸ APInt(ਦੋਆਧਾਰੀ_ਅੰਕ_ਚੌੜਾਈ_2, Tmp1);
  }

  APInt ਨਤੀਜਾ(get_ਅੰਕ_ਸ਼ਬਦ() * APINT_ਬਿੱਟ_ਪ੍ਰਤੀ_ਸ਼ਬਦ, 0);
  ਲਈ (ਬਿਨਾਨਿਸ਼ਾਨੀ I = 0, N = get_ਅੰਕ_ਸ਼ਬਦ(); I != N; ++I)
    ਨਤੀਜਾ.U.pVal[I] = llvm::byteswap<uint64_t>(U.pVal[N - I - 1]);
  ਜੇ (ਨਤੀਜਾ.ਦੋਆਧਾਰੀ_ਅੰਕ_ਚੌੜਾਈ_2 != ਦੋਆਧਾਰੀ_ਅੰਕ_ਚੌੜਾਈ_2) {
    ਨਤੀਜਾ.lshr_ਵਿੱਚ_ਰੱਖੋ(ਨਤੀਜਾ.ਦੋਆਧਾਰੀ_ਅੰਕ_ਚੌੜਾਈ_2 - ਦੋਆਧਾਰੀ_ਅੰਕ_ਚੌੜਾਈ_2);
    ਨਤੀਜਾ.ਦੋਆਧਾਰੀ_ਅੰਕ_ਚੌੜਾਈ_2 = ਦੋਆਧਾਰੀ_ਅੰਕ_ਚੌੜਾਈ_2;
  }
  ਵਾਪਸ ਨਤੀਜਾ;
}

APInt APInt::ਉਲਟ_ਬਿੱਟ() ਅਚਲ {
  ਜੇ (LLVM_LIKELY(is_Single_ਸ਼ਬਦ())) {
    ਚੋਣ (ਦੋਆਧਾਰੀ_ਅੰਕ_ਚੌੜਾਈ_2) {
    ਹਾਲਤ 64:
      ਵਾਪਸ APInt(ਦੋਆਧਾਰੀ_ਅੰਕ_ਚੌੜਾਈ_2, llvm::ਉਲਟ_ਬਿੱਟ<uint64_t>(U.VAL));
    ਹਾਲਤ 32:
      ਵਾਪਸ APInt(ਦੋਆਧਾਰੀ_ਅੰਕ_ਚੌੜਾਈ_2, llvm::ਉਲਟ_ਬਿੱਟ<uint32_t>(U.VAL));
    ਹਾਲਤ 16:
      ਵਾਪਸ APInt(ਦੋਆਧਾਰੀ_ਅੰਕ_ਚੌੜਾਈ_2, llvm::ਉਲਟ_ਬਿੱਟ<uint16_t>(U.VAL));
    ਹਾਲਤ 8:
      ਵਾਪਸ APInt(ਦੋਆਧਾਰੀ_ਅੰਕ_ਚੌੜਾਈ_2, llvm::ਉਲਟ_ਬਿੱਟ<uint8_t>(U.VAL));
    ਹਾਲਤ 1: // fallthrough
    ਹਾਲਤ 0:
      ਵਾਪਸ *ਇਹ;
    ਮੂਲ:
      ਵਾਪਸ APInt(ਦੋਆਧਾਰੀ_ਅੰਕ_ਚੌੜਾਈ_2,
                   llvm::ਉਲਟ_ਬਿੱਟ<uint64_t>(U.VAL) >> (64 - ਦੋਆਧਾਰੀ_ਅੰਕ_ਚੌੜਾਈ_2));
    }
  }

  APInt ਨਤੀਜਾ(ਦੋਆਧਾਰੀ_ਅੰਕ_ਚੌੜਾਈ_2, 0);
  ਬਿਨਾਨਿਸ਼ਾਨੀ ਅੰਕ_ਸ਼ਬਦ_2 = get_ਅੰਕ_ਸ਼ਬਦ();
  ਬਿਨਾਨਿਸ਼ਾਨੀ Excess_ਬਿੱਟ = ਅੰਕ_ਸ਼ਬਦ_2 * APINT_ਬਿੱਟ_ਪ੍ਰਤੀ_ਸ਼ਬਦ - ਦੋਆਧਾਰੀ_ਅੰਕ_ਚੌੜਾਈ_2;
  ਜੇ (Excess_ਬਿੱਟ == 0) {
    // Fast path. No cross-word shift needed.
    ਲਈ (ਬਿਨਾਨਿਸ਼ਾਨੀ I = 0; I < ਅੰਕ_ਸ਼ਬਦ_2; ++I)
      ਨਤੀਜਾ.U.pVal[I] = llvm::ਉਲਟ_ਬਿੱਟ<uint64_t>(U.pVal[ਅੰਕ_ਸ਼ਬਦ_2 - 1 - I]);
    ਵਾਪਸ ਨਤੀਜਾ;
  }
  // Holds reversed bits of the previous (more significant) word.
  uint64_t PrevRev = llvm::ਉਲਟ_ਬਿੱਟ<uint64_t>(U.pVal[ਅੰਕ_ਸ਼ਬਦ_2 - 1]);
  ਲਈ (ਬਿਨਾਨਿਸ਼ਾਨੀ I = 0; I < ਅੰਕ_ਸ਼ਬਦ_2 - 1; ++I) {
    uint64_t CurrRev = llvm::ਉਲਟ_ਬਿੱਟ<uint64_t>(U.pVal[ਅੰਕ_ਸ਼ਬਦ_2 - 2 - I]);
    ਨਤੀਜਾ.U.pVal[I] = (PrevRev >> Excess_ਬਿੱਟ) | (CurrRev << (64 - Excess_ਬਿੱਟ));
    PrevRev = CurrRev;
  }
  ਨਤੀਜਾ.U.pVal[ਅੰਕ_ਸ਼ਬਦ_2 - 1] = PrevRev >> Excess_ਬਿੱਟ;
  ਵਾਪਸ ਨਤੀਜਾ;
}

APInt llvm::APIntOps::Greatest_Common_ਭਾਜਕ(APInt A, APInt B, ਤਾਰਕਿਕ IsSigned) {
  // Take absolute value if IsSigned.
  ਜੇ (IsSigned) {
    A = A.ਨਿਰਪੇਖ();
    B = B.ਨਿਰਪੇਖ();
  }

  // Fast-path a common case.
  ਜੇ (A == B) ਵਾਪਸ A;

  // Corner cases: if either operand is zero, the other is the gcd.
  ਜੇ (!A) ਵਾਪਸ B;
  ਜੇ (!B) ਵਾਪਸ A;

  // Count common powers of 2 and remove all other powers of 2.
  ਬਿਨਾਨਿਸ਼ਾਨੀ Pow2;
  {
    ਬਿਨਾਨਿਸ਼ਾਨੀ Pow2_A = A.countr_zero();
    ਬਿਨਾਨਿਸ਼ਾਨੀ Pow2_B = B.countr_zero();
    ਜੇ (Pow2_A > Pow2_B) {
      A.lshr_ਵਿੱਚ_ਰੱਖੋ(Pow2_A - Pow2_B);
      Pow2 = Pow2_B;
    } ਨਹੀਂਤਾਂ ਜੇ (Pow2_B > Pow2_A) {
      B.lshr_ਵਿੱਚ_ਰੱਖੋ(Pow2_B - Pow2_A);
      Pow2 = Pow2_A;
    } ਨਹੀਂਤਾਂ {
      Pow2 = Pow2_A;
    }
  }

  // Both operands are odd multiples of 2^Pow_2:
  //
  //   gcd(a, b) = gcd(|a - b| / 2^i, min(a, b))
  //
  // This is a modified version of Stein's algorithm, taking advantage of
  // efficient countTrailingZeros().
  ਜਦੋਂ (A != B) {
    ਜੇ (A.ugt(B)) {
      A -= B;
      A.lshr_ਵਿੱਚ_ਰੱਖੋ(A.countr_zero() - Pow2);
    } ਨਹੀਂਤਾਂ {
      B -= A;
      B.lshr_ਵਿੱਚ_ਰੱਖੋ(B.countr_zero() - Pow2);
    }
  }

  ਵਾਪਸ A;
}

APInt llvm::APIntOps::ਗੇੜਾ_Double_ਮੰਜ਼ਿਲ_APInt(ਦੁੱਗਣਾ Double, ਬਿਨਾਨਿਸ਼ਾਨੀ ਚੌੜਾਈ_2) {
  uint64_t I = ਦੋਆਧਾਰੀ_ਅੰਕ_cast<uint64_t>(Double);

  // Get the sign bit from the highest order bit
  ਤਾਰਕਿਕ isNeg = I >> 63;

  // Get the 11-bit exponent and adjust for the 1023 bit bias
  int64_t ਘਾਤਾਂਕ = ((I >> 52) & 0x7ff) - 1023;

  // If the exponent is negative, the value is < 0 so just return 0.
  ਜੇ (ਘਾਤਾਂਕ < 0)
    ਵਾਪਸ APInt(ਚੌੜਾਈ_2, 0u);

  // Extract the mantissa by clearing the top 12 bits (sign + exponent).
  uint64_t mantissa = (I & (~0ULL >> 12)) | 1ULL << 52;

  // If the exponent doesn't shift all bits out of the mantissa
  ਜੇ (ਘਾਤਾਂਕ < 52)
    ਵਾਪਸ isNeg ? -APInt(ਚੌੜਾਈ_2, mantissa >> (52 - ਘਾਤਾਂਕ)) :
                    APInt(ਚੌੜਾਈ_2, mantissa >> (52 - ਘਾਤਾਂਕ));

  // If the client didn't provide enough bits for us to shift the mantissa into
  // then the result is undefined, just return 0
  ਜੇ (ਚੌੜਾਈ_2 <= ਘਾਤਾਂਕ - 52)
    ਵਾਪਸ APInt(ਚੌੜਾਈ_2, 0);

  // Otherwise, we have to shift the mantissa bits up to the right location
  APInt Tmp(ਚੌੜਾਈ_2, mantissa);
  Tmp <<= (ਬਿਨਾਨਿਸ਼ਾਨੀ)ਘਾਤਾਂਕ - 52;
  ਵਾਪਸ isNeg ? -Tmp : Tmp;
}

/// This function converts this APInt to a double.
/// The layout for double is as following (IEEE Standard 754):
///  --------------------------------------
/// |  Sign    Exponent    Fraction    Bias |
/// |-------------------------------------- |
/// |  1[63]   11[62-52]   52[51-00]   1023 |
///  --------------------------------------
ਦੁੱਗਣਾ APInt::ਗੇੜਾ_ਮੰਜ਼ਿਲ_Double(ਤਾਰਕਿਕ isSigned) ਅਚਲ {
  // Handle the simple case where the value is contained in one uint64_t.
  // It is wrong to optimize getWord(0) to VAL; there might be more than one word.
  ਜੇ (LLVM_LIKELY(is_Single_ਸ਼ਬਦ() || get_ਸਰਗਰਮ_ਬਿੱਟ() <= APINT_ਬਿੱਟ_ਪ੍ਰਤੀ_ਸ਼ਬਦ)) {
    ਜੇ (isSigned) {
      int64_t sext = SignExtend64(get_ਸ਼ਬਦ(0), ਦੋਆਧਾਰੀ_ਅੰਕ_ਚੌੜਾਈ_2);
      ਵਾਪਸ ਦੁੱਗਣਾ(sext);
    }
    ਵਾਪਸ ਦੁੱਗਣਾ(get_ਸ਼ਬਦ(0));
  }

  // Determine if the value is negative.
  ਤਾਰਕਿਕ isNeg = isSigned ? (*ਇਹ)[ਦੋਆਧਾਰੀ_ਅੰਕ_ਚੌੜਾਈ_2-1] : ਝੂਠ;

  // Construct the absolute value if we're negative.
  APInt Tmp(isNeg ? -(*ਇਹ) : (*ਇਹ));

  // Figure out how many bits we're using.
  ਬਿਨਾਨਿਸ਼ਾਨੀ n = Tmp.get_ਸਰਗਰਮ_ਬਿੱਟ();

  // The exponent (without bias normalization) is just the number of bits
  // we are using. Note that the sign bit is gone since we constructed the
  // absolute value.
  uint64_t ਘਾਤਾਂਕ = n;

  // Return infinity for exponent overflow
  ਜੇ (ਘਾਤਾਂਕ > 1023) {
    ਜੇ (!isSigned || !isNeg)
      ਵਾਪਸ std::numeric_limits<ਦੁੱਗਣਾ>::infinity();
    ਨਹੀਂਤਾਂ
      ਵਾਪਸ -std::numeric_limits<ਦੁੱਗਣਾ>::infinity();
  }
  ਘਾਤਾਂਕ += 1023; // Increment for 1023 bias

  // Number of bits in mantissa is 52. To obtain the mantissa value, we must
  // extract the high 52 bits from the correct words in pVal.
  uint64_t mantissa;
  ਬਿਨਾਨਿਸ਼ਾਨੀ hi_ਸ਼ਬਦ = which_ਸ਼ਬਦ(n-1);
  ਜੇ (hi_ਸ਼ਬਦ == 0) {
    mantissa = Tmp.U.pVal[0];
    ਜੇ (n > 52)
      mantissa >>= n - 52; // shift down, we want the top 52 bits.
  } ਨਹੀਂਤਾਂ {
    ਪ੍ਰਤਿਜ੍ਞਾ(hi_ਸ਼ਬਦ > 0 && "huh?");
    uint64_t hibits = Tmp.U.pVal[hi_ਸ਼ਬਦ] << (52 - n % APINT_ਬਿੱਟ_ਪ੍ਰਤੀ_ਸ਼ਬਦ);
    uint64_t lobits = Tmp.U.pVal[hi_ਸ਼ਬਦ-1] >> (11 + n % APINT_ਬਿੱਟ_ਪ੍ਰਤੀ_ਸ਼ਬਦ);
    mantissa = hibits | lobits;
  }

  // The leading bit of mantissa is implicit, so get rid of it.
  uint64_t sign = isNeg ? (1ULL << (APINT_ਬਿੱਟ_ਪ੍ਰਤੀ_ਸ਼ਬਦ - 1)) : 0;
  uint64_t I = sign | (ਘਾਤਾਂਕ << 52) | mantissa;
  ਵਾਪਸ ਦੋਆਧਾਰੀ_ਅੰਕ_cast<ਦੁੱਗਣਾ>(I);
}

// Truncate to new width.
APInt APInt::trunc(ਬਿਨਾਨਿਸ਼ਾਨੀ ਚੌੜਾਈ_2) ਅਚਲ {
  ਪ੍ਰਤਿਜ੍ਞਾ(ਚੌੜਾਈ_2 <= ਦੋਆਧਾਰੀ_ਅੰਕ_ਚੌੜਾਈ_2 && "Invalid APInt Truncate request");

  ਜੇ (ਚੌੜਾਈ_2 <= APINT_ਬਿੱਟ_ਪ੍ਰਤੀ_ਸ਼ਬਦ)
    ਵਾਪਸ APInt(ਚੌੜਾਈ_2, get_Raw_ਅੰਕੜੇ()[0], /*isSigned=*/ਝੂਠ,
                 /*implicitTrunc=*/ਸੱਚ);

  ਜੇ (ਚੌੜਾਈ_2 == ਦੋਆਧਾਰੀ_ਅੰਕ_ਚੌੜਾਈ_2)
    ਵਾਪਸ *ਇਹ;

  APInt ਨਤੀਜਾ(get_ਸਮ੍ਰਿਤੀ(get_ਅੰਕ_ਸ਼ਬਦ(ਚੌੜਾਈ_2)), ਚੌੜਾਈ_2);

  // Copy full words.
  ਬਿਨਾਨਿਸ਼ਾਨੀ i;
  ਲਈ (i = 0; i != ਚੌੜਾਈ_2 / APINT_ਬਿੱਟ_ਪ੍ਰਤੀ_ਸ਼ਬਦ; i++)
    ਨਤੀਜਾ.U.pVal[i] = U.pVal[i];

  // Truncate and copy any partial word.
  ਬਿਨਾਨਿਸ਼ਾਨੀ ਬਿੱਟ = (0 - ਚੌੜਾਈ_2) % APINT_ਬਿੱਟ_ਪ੍ਰਤੀ_ਸ਼ਬਦ;
  ਜੇ (ਬਿੱਟ != 0)
    ਨਤੀਜਾ.U.pVal[i] = U.pVal[i] << ਬਿੱਟ >> ਬਿੱਟ;

  ਵਾਪਸ ਨਤੀਜਾ;
}

// Truncate to new width with unsigned saturation.
APInt APInt::truncUSat(ਬਿਨਾਨਿਸ਼ਾਨੀ ਚੌੜਾਈ_2) ਅਚਲ {
  ਪ੍ਰਤਿਜ੍ਞਾ(ਚੌੜਾਈ_2 <= ਦੋਆਧਾਰੀ_ਅੰਕ_ਚੌੜਾਈ_2 && "Invalid APInt Truncate request");

  // Can we just losslessly truncate it?
  ਜੇ (isIntN(ਚੌੜਾਈ_2))
    ਵਾਪਸ trunc(ਚੌੜਾਈ_2);
  // If not, then just return the new limit.
  ਵਾਪਸ APInt::get_Max_ਮੁੱਲ(ਚੌੜਾਈ_2);
}

// Truncate to new width with signed saturation to signed result.
APInt APInt::truncSSat(ਬਿਨਾਨਿਸ਼ਾਨੀ ਚੌੜਾਈ_2) ਅਚਲ {
  ਪ੍ਰਤਿਜ੍ਞਾ(ਚੌੜਾਈ_2 <= ਦੋਆਧਾਰੀ_ਅੰਕ_ਚੌੜਾਈ_2 && "Invalid APInt Truncate request");

  // Can we just losslessly truncate it?
  ਜੇ (isSignedIntN(ਚੌੜਾਈ_2))
    ਵਾਪਸ trunc(ਚੌੜਾਈ_2);
  // If not, then just return the new limits.
  ਵਾਪਸ isNegative() ? APInt::get_Signed_Min_ਮੁੱਲ(ਚੌੜਾਈ_2)
                      : APInt::get_Signed_Max_ਮੁੱਲ(ਚੌੜਾਈ_2);
}

// Truncate to new width with signed saturation to unsigned result.
APInt APInt::truncSSatU(ਬਿਨਾਨਿਸ਼ਾਨੀ ਚੌੜਾਈ_2) ਅਚਲ {
  ਪ੍ਰਤਿਜ੍ਞਾ(ਚੌੜਾਈ_2 <= ਦੋਆਧਾਰੀ_ਅੰਕ_ਚੌੜਾਈ_2 && "Invalid APInt Truncate request");

  // Can we just losslessly truncate it?
  ਜੇ (isIntN(ਚੌੜਾਈ_2))
    ਵਾਪਸ trunc(ਚੌੜਾਈ_2);
  // If not, then just return the new limits.
  ਵਾਪਸ isNegative() ? APInt::getZero(ਚੌੜਾਈ_2) : APInt::get_Max_ਮੁੱਲ(ਚੌੜਾਈ_2);
}

// Sign extend to a new width.
APInt APInt::sext(ਬਿਨਾਨਿਸ਼ਾਨੀ ਚੌੜਾਈ) ਅਚਲ {
  ਪ੍ਰਤਿਜ੍ਞਾ(ਚੌੜਾਈ >= ਦੋਆਧਾਰੀ_ਅੰਕ_ਚੌੜਾਈ_2 && "Invalid APInt SignExtend request");

  ਜੇ (ਚੌੜਾਈ <= APINT_ਬਿੱਟ_ਪ੍ਰਤੀ_ਸ਼ਬਦ)
    ਵਾਪਸ APInt(ਚੌੜਾਈ, SignExtend64(U.VAL, ਦੋਆਧਾਰੀ_ਅੰਕ_ਚੌੜਾਈ_2), /*isSigned=*/ਸੱਚ);

  ਜੇ (ਚੌੜਾਈ == ਦੋਆਧਾਰੀ_ਅੰਕ_ਚੌੜਾਈ_2)
    ਵਾਪਸ *ਇਹ;

  APInt ਨਤੀਜਾ(get_ਸਮ੍ਰਿਤੀ(get_ਅੰਕ_ਸ਼ਬਦ(ਚੌੜਾਈ)), ਚੌੜਾਈ);

  // Copy words.
  std::ਸਮ੍ਰਿਤੀ_ਨਕਲ(ਨਤੀਜਾ.U.pVal, get_Raw_ਅੰਕੜੇ(), get_ਅੰਕ_ਸ਼ਬਦ() * APINT_ਸ਼ਬਦ_ਮਾਪ);

  // Sign extend the last word since there may be unused bits in the input.
  ਨਤੀਜਾ.U.pVal[get_ਅੰਕ_ਸ਼ਬਦ() - 1] =
      SignExtend64(ਨਤੀਜਾ.U.pVal[get_ਅੰਕ_ਸ਼ਬਦ() - 1],
                   ((ਦੋਆਧਾਰੀ_ਅੰਕ_ਚੌੜਾਈ_2 - 1) % APINT_ਬਿੱਟ_ਪ੍ਰਤੀ_ਸ਼ਬਦ) + 1);

  // Fill with sign bits.
  std::memset(ਨਤੀਜਾ.U.pVal + get_ਅੰਕ_ਸ਼ਬਦ(), isNegative() ? -1 : 0,
              (ਨਤੀਜਾ.get_ਅੰਕ_ਸ਼ਬਦ() - get_ਅੰਕ_ਸ਼ਬਦ()) * APINT_ਸ਼ਬਦ_ਮਾਪ);
  ਨਤੀਜਾ.clear_ਅਣਵਰਤਿਆ_ਬਿੱਟ();
  ਵਾਪਸ ਨਤੀਜਾ;
}

//  Zero extend to a new width.
APInt APInt::zext(ਬਿਨਾਨਿਸ਼ਾਨੀ ਚੌੜਾਈ_2) ਅਚਲ {
  ਪ੍ਰਤਿਜ੍ਞਾ(ਚੌੜਾਈ_2 >= ਦੋਆਧਾਰੀ_ਅੰਕ_ਚੌੜਾਈ_2 && "Invalid APInt ZeroExtend request");

  ਜੇ (ਚੌੜਾਈ_2 <= APINT_ਬਿੱਟ_ਪ੍ਰਤੀ_ਸ਼ਬਦ)
    ਵਾਪਸ APInt(ਚੌੜਾਈ_2, U.VAL);

  ਜੇ (ਚੌੜਾਈ_2 == ਦੋਆਧਾਰੀ_ਅੰਕ_ਚੌੜਾਈ_2)
    ਵਾਪਸ *ਇਹ;

  APInt ਨਤੀਜਾ(get_ਸਮ੍ਰਿਤੀ(get_ਅੰਕ_ਸ਼ਬਦ(ਚੌੜਾਈ_2)), ਚੌੜਾਈ_2);

  // Copy words.
  std::ਸਮ੍ਰਿਤੀ_ਨਕਲ(ਨਤੀਜਾ.U.pVal, get_Raw_ਅੰਕੜੇ(), get_ਅੰਕ_ਸ਼ਬਦ() * APINT_ਸ਼ਬਦ_ਮਾਪ);

  // Zero remaining words.
  std::memset(ਨਤੀਜਾ.U.pVal + get_ਅੰਕ_ਸ਼ਬਦ(), 0,
              (ਨਤੀਜਾ.get_ਅੰਕ_ਸ਼ਬਦ() - get_ਅੰਕ_ਸ਼ਬਦ()) * APINT_ਸ਼ਬਦ_ਮਾਪ);

  ਵਾਪਸ ਨਤੀਜਾ;
}

APInt APInt::zextOrTrunc(ਬਿਨਾਨਿਸ਼ਾਨੀ ਚੌੜਾਈ_2) ਅਚਲ {
  ਜੇ (ਦੋਆਧਾਰੀ_ਅੰਕ_ਚੌੜਾਈ_2 < ਚੌੜਾਈ_2)
    ਵਾਪਸ zext(ਚੌੜਾਈ_2);
  ਜੇ (ਦੋਆਧਾਰੀ_ਅੰਕ_ਚੌੜਾਈ_2 > ਚੌੜਾਈ_2)
    ਵਾਪਸ trunc(ਚੌੜਾਈ_2);
  ਵਾਪਸ *ਇਹ;
}

APInt APInt::sextOrTrunc(ਬਿਨਾਨਿਸ਼ਾਨੀ ਚੌੜਾਈ_2) ਅਚਲ {
  ਜੇ (ਦੋਆਧਾਰੀ_ਅੰਕ_ਚੌੜਾਈ_2 < ਚੌੜਾਈ_2)
    ਵਾਪਸ sext(ਚੌੜਾਈ_2);
  ਜੇ (ਦੋਆਧਾਰੀ_ਅੰਕ_ਚੌੜਾਈ_2 > ਚੌੜਾਈ_2)
    ਵਾਪਸ trunc(ਚੌੜਾਈ_2);
  ਵਾਪਸ *ਇਹ;
}

/// Arithmetic right-shift this APInt by shiftAmt.
/// Arithmetic right-shift function.
ਖਾਲੀ APInt::ashr_ਵਿੱਚ_ਰੱਖੋ(ਅਚਲ APInt &ਖਿਸਕਾਓ_Amt) {
  ashr_ਵਿੱਚ_ਰੱਖੋ((ਬਿਨਾਨਿਸ਼ਾਨੀ)ਖਿਸਕਾਓ_Amt.get_Limited_ਮੁੱਲ(ਦੋਆਧਾਰੀ_ਅੰਕ_ਚੌੜਾਈ_2));
}

/// Arithmetic right-shift this APInt by shiftAmt.
/// Arithmetic right-shift function.
ਖਾਲੀ APInt::ashrSlowCase(ਬਿਨਾਨਿਸ਼ਾਨੀ ਖਿਸਕਾਓ_Amt_2) {
  // Don't bother performing a no-op shift.
  ਜੇ (!ਖਿਸਕਾਓ_Amt_2)
    ਵਾਪਸ;

  // Save the original sign bit for later.
  ਤਾਰਕਿਕ Negative = isNegative();

  // WordShift is the inter-part shift; BitShift is intra-part shift.
  ਬਿਨਾਨਿਸ਼ਾਨੀ ਸ਼ਬਦ_ਖਿਸਕਾਓ = ਖਿਸਕਾਓ_Amt_2 / APINT_ਬਿੱਟ_ਪ੍ਰਤੀ_ਸ਼ਬਦ;
  ਬਿਨਾਨਿਸ਼ਾਨੀ ਦੋਆਧਾਰੀ_ਅੰਕ_ਖਿਸਕਾਓ = ਖਿਸਕਾਓ_Amt_2 % APINT_ਬਿੱਟ_ਪ੍ਰਤੀ_ਸ਼ਬਦ;

  ਬਿਨਾਨਿਸ਼ਾਨੀ ਸ਼ਬਦ_ਮੰਜ਼ਿਲ_Move = get_ਅੰਕ_ਸ਼ਬਦ() - ਸ਼ਬਦ_ਖਿਸਕਾਓ;
  ਜੇ (ਸ਼ਬਦ_ਮੰਜ਼ਿਲ_Move != 0) {
    // Sign extend the last word to fill in the unused bits.
    U.pVal[get_ਅੰਕ_ਸ਼ਬਦ() - 1] = SignExtend64(
        U.pVal[get_ਅੰਕ_ਸ਼ਬਦ() - 1], ((ਦੋਆਧਾਰੀ_ਅੰਕ_ਚੌੜਾਈ_2 - 1) % APINT_ਬਿੱਟ_ਪ੍ਰਤੀ_ਸ਼ਬਦ) + 1);

    // Fastpath for moving by whole words.
    ਜੇ (ਦੋਆਧਾਰੀ_ਅੰਕ_ਖਿਸਕਾਓ == 0) {
      std::memmove(U.pVal, U.pVal + ਸ਼ਬਦ_ਖਿਸਕਾਓ, ਸ਼ਬਦ_ਮੰਜ਼ਿਲ_Move * APINT_ਸ਼ਬਦ_ਮਾਪ);
    } ਨਹੀਂਤਾਂ {
      // Move the words containing significant bits.
      ਲਈ (ਬਿਨਾਨਿਸ਼ਾਨੀ i = 0; i != ਸ਼ਬਦ_ਮੰਜ਼ਿਲ_Move - 1; ++i)
        U.pVal[i] = (U.pVal[i + ਸ਼ਬਦ_ਖਿਸਕਾਓ] >> ਦੋਆਧਾਰੀ_ਅੰਕ_ਖਿਸਕਾਓ) |
                    (U.pVal[i + ਸ਼ਬਦ_ਖਿਸਕਾਓ + 1] << (APINT_ਬਿੱਟ_ਪ੍ਰਤੀ_ਸ਼ਬਦ - ਦੋਆਧਾਰੀ_ਅੰਕ_ਖਿਸਕਾਓ));

      // Handle the last word which has no high bits to copy. Use an arithmetic
      // shift to preserve the sign bit.
      U.pVal[ਸ਼ਬਦ_ਮੰਜ਼ਿਲ_Move - 1] =
          (int64_t)U.pVal[ਸ਼ਬਦ_ਖਿਸਕਾਓ + ਸ਼ਬਦ_ਮੰਜ਼ਿਲ_Move - 1] >> ਦੋਆਧਾਰੀ_ਅੰਕ_ਖਿਸਕਾਓ;
    }
  }

  // Fill in the remainder based on the original sign.
  std::memset(U.pVal + ਸ਼ਬਦ_ਮੰਜ਼ਿਲ_Move, Negative ? -1 : 0,
              ਸ਼ਬਦ_ਖਿਸਕਾਓ * APINT_ਸ਼ਬਦ_ਮਾਪ);
  clear_ਅਣਵਰਤਿਆ_ਬਿੱਟ();
}

/// Logical right-shift this APInt by shiftAmt.
/// Logical right-shift function.
ਖਾਲੀ APInt::lshr_ਵਿੱਚ_ਰੱਖੋ(ਅਚਲ APInt &ਖਿਸਕਾਓ_Amt) {
  lshr_ਵਿੱਚ_ਰੱਖੋ((ਬਿਨਾਨਿਸ਼ਾਨੀ)ਖਿਸਕਾਓ_Amt.get_Limited_ਮੁੱਲ(ਦੋਆਧਾਰੀ_ਅੰਕ_ਚੌੜਾਈ_2));
}

/// Logical right-shift this APInt by shiftAmt.
/// Logical right-shift function.
ਖਾਲੀ APInt::lshrSlowCase(ਬਿਨਾਨਿਸ਼ਾਨੀ ਖਿਸਕਾਓ_Amt_2) {
  tc_ਖਿਸਕਾਓ_ਸੱਜਾ(U.pVal, get_ਅੰਕ_ਸ਼ਬਦ(), ਖਿਸਕਾਓ_Amt_2);
}

/// Left-shift this APInt by shiftAmt.
/// Left-shift function.
APInt &APInt::ਸੰਚਾਲਕ<<=(ਅਚਲ APInt &ਖਿਸਕਾਓ_Amt) {
  // It's undefined behavior in C to shift by BitWidth or greater.
  *ਇਹ <<= (ਬਿਨਾਨਿਸ਼ਾਨੀ)ਖਿਸਕਾਓ_Amt.get_Limited_ਮੁੱਲ(ਦੋਆਧਾਰੀ_ਅੰਕ_ਚੌੜਾਈ_2);
  ਵਾਪਸ *ਇਹ;
}

ਖਾਲੀ APInt::shlSlowCase(ਬਿਨਾਨਿਸ਼ਾਨੀ ਖਿਸਕਾਓ_Amt_2) {
  tc_ਖਿਸਕਾਓ_ਖੱਬਾ(U.pVal, get_ਅੰਕ_ਸ਼ਬਦ(), ਖਿਸਕਾਓ_Amt_2);
  clear_ਅਣਵਰਤਿਆ_ਬਿੱਟ();
}

// Calculate the rotate amount modulo the bit width.
ਸਥਿਰ ਬਿਨਾਨਿਸ਼ਾਨੀ rotateModulo(ਬਿਨਾਨਿਸ਼ਾਨੀ ਦੋਆਧਾਰੀ_ਅੰਕ_ਚੌੜਾਈ_2, ਅਚਲ APInt &rotateAmt) {
  ਜੇ (LLVM_UNLIKELY(ਦੋਆਧਾਰੀ_ਅੰਕ_ਚੌੜਾਈ_2 == 0))
    ਵਾਪਸ 0;
  ਬਿਨਾਨਿਸ਼ਾਨੀ rot_ਦੋਆਧਾਰੀ_ਅੰਕ_ਚੌੜਾਈ = rotateAmt.get_ਦੋਆਧਾਰੀ_ਅੰਕ_ਚੌੜਾਈ();
  APInt rot = rotateAmt;
  ਜੇ (rot_ਦੋਆਧਾਰੀ_ਅੰਕ_ਚੌੜਾਈ < ਦੋਆਧਾਰੀ_ਅੰਕ_ਚੌੜਾਈ_2) {
    // Extend the rotate APInt, so that the urem doesn't divide by 0.
    // e.g. APInt(1, 32) would give APInt(1, 0).
    rot = rotateAmt.zext(ਦੋਆਧਾਰੀ_ਅੰਕ_ਚੌੜਾਈ_2);
  }
  rot = rot.urem(APInt(rot.get_ਦੋਆਧਾਰੀ_ਅੰਕ_ਚੌੜਾਈ(), ਦੋਆਧਾਰੀ_ਅੰਕ_ਚੌੜਾਈ_2));
  ਵਾਪਸ rot.get_Limited_ਮੁੱਲ(ਦੋਆਧਾਰੀ_ਅੰਕ_ਚੌੜਾਈ_2);
}

APInt APInt::rotl(ਅਚਲ APInt &rotateAmt) ਅਚਲ {
  ਵਾਪਸ rotl(rotateModulo(ਦੋਆਧਾਰੀ_ਅੰਕ_ਚੌੜਾਈ_2, rotateAmt));
}

APInt APInt::rotl(ਬਿਨਾਨਿਸ਼ਾਨੀ rotateAmt) ਅਚਲ {
  ਜੇ (LLVM_UNLIKELY(ਦੋਆਧਾਰੀ_ਅੰਕ_ਚੌੜਾਈ_2 == 0))
    ਵਾਪਸ *ਇਹ;
  rotateAmt %= ਦੋਆਧਾਰੀ_ਅੰਕ_ਚੌੜਾਈ_2;
  ਜੇ (rotateAmt == 0)
    ਵਾਪਸ *ਇਹ;
  ਵਾਪਸ shl(rotateAmt) | lshr(ਦੋਆਧਾਰੀ_ਅੰਕ_ਚੌੜਾਈ_2 - rotateAmt);
}

APInt APInt::rotr(ਅਚਲ APInt &rotateAmt) ਅਚਲ {
  ਵਾਪਸ rotr(rotateModulo(ਦੋਆਧਾਰੀ_ਅੰਕ_ਚੌੜਾਈ_2, rotateAmt));
}

APInt APInt::rotr(ਬਿਨਾਨਿਸ਼ਾਨੀ rotateAmt) ਅਚਲ {
  ਜੇ (ਦੋਆਧਾਰੀ_ਅੰਕ_ਚੌੜਾਈ_2 == 0)
    ਵਾਪਸ *ਇਹ;
  rotateAmt %= ਦੋਆਧਾਰੀ_ਅੰਕ_ਚੌੜਾਈ_2;
  ਜੇ (rotateAmt == 0)
    ਵਾਪਸ *ਇਹ;
  ਵਾਪਸ lshr(rotateAmt) | shl(ਦੋਆਧਾਰੀ_ਅੰਕ_ਚੌੜਾਈ_2 - rotateAmt);
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
ਬਿਨਾਨਿਸ਼ਾਨੀ APInt::ਸਭ_ਤੋਂ_ਨੇੜੇ_Log_Base2() ਅਚਲ {
  // Special case when we have a bitwidth of 1. If VAL is 1, then we
  // get 0. If VAL is 0, we get WORDTYPE_MAX which gets truncated to
  // UINT32_MAX.
  ਜੇ (ਦੋਆਧਾਰੀ_ਅੰਕ_ਚੌੜਾਈ_2 == 1)
    ਵਾਪਸ U.VAL - 1;

  // Handle the zero case.
  ਜੇ (isZero())
    ਵਾਪਸ UINT32_MAX;

  // The non-zero case is handled by computing:
  //
  //   nearestLogBase2(x) = logBase2(x) + x[logBase2(x)-1].
  //
  // where x[i] is referring to the value of the ith bit of x.
  ਬਿਨਾਨਿਸ਼ਾਨੀ lg = logBase2();
  ਵਾਪਸ lg + ਬਿਨਾਨਿਸ਼ਾਨੀ((*ਇਹ)[lg - 1]);
}

// Square Root - this method computes and returns the square root of "this".
// Three mechanisms are used for computation. For small values (<= 5 bits),
// a table lookup is done. This gets some performance for common cases. For
// values using less than 52 bits, the value is converted to double and then
// the libc sqrt function is called. The result is rounded and then converted
// back to a uint64_t which is then used to construct the result. Finally,
// the Babylonian method for computing square roots is used.
APInt APInt::sqrtFloor() ਅਚਲ {

  // Determine the magnitude of the value.
  ਬਿਨਾਨਿਸ਼ਾਨੀ magnitude = get_ਸਰਗਰਮ_ਬਿੱਟ();

  // Use a fast table for some small values. This also gets rid of some
  // rounding errors in libc sqrt for small values.
  ਜੇ (magnitude <= 5) {
    ਸਥਿਰ ਅਚਲ uint8_t results[32] = {
        /*     0 */ 0,
        /*  1- 3 */ 1, 1, 1,
        /*  4- 8 */ 2, 2, 2, 2, 2,
        /*  9-15 */ 3, 3, 3, 3, 3, 3, 3,
        /* 16-24 */ 4, 4, 4, 4, 4, 4, 4, 4, 4,
        /* 25-31 */ 5, 5, 5, 5, 5, 5, 5,
    };
    ਵਾਪਸ APInt(ਦੋਆਧਾਰੀ_ਅੰਕ_ਚੌੜਾਈ_2,
                 results[(LLVM_LIKELY(is_Single_ਸ਼ਬਦ()) ? U.VAL : U.pVal[0])]);
  }

  // If the magnitude of the value fits in less than 52 bits (the precision of
  // an IEEE double precision floating point value), then we can use the
  // libc sqrt function which will probably use a hardware sqrt computation.
  // This should be faster than the algorithm below.
  ਜੇ (magnitude < 52) {
    ਵਾਪਸ APInt(ਦੋਆਧਾਰੀ_ਅੰਕ_ਚੌੜਾਈ_2,
                 uint64_t(::floor(::ਵਰਗਮੂਲ(ਦੁੱਗਣਾ(
                     LLVM_LIKELY(is_Single_ਸ਼ਬਦ()) ? U.VAL : U.pVal[0])))));
  }

  // Okay, all the short cuts are exhausted. We must compute it. The following
  // is a classical Babylonian method for computing the square root. This code
  // was adapted to APInt from a wikipedia article on such computations.
  // See http://www.wikipedia.org/ and go to the page named
  // Calculate_an_integer_square_root.
  ਬਿਨਾਨਿਸ਼ਾਨੀ nbits = ਦੋਆਧਾਰੀ_ਅੰਕ_ਚੌੜਾਈ_2, i = 4;
  APInt testy(ਦੋਆਧਾਰੀ_ਅੰਕ_ਚੌੜਾਈ_2, 16);
  APInt x_old(ਦੋਆਧਾਰੀ_ਅੰਕ_ਚੌੜਾਈ_2, 1);
  APInt x_new(ਦੋਆਧਾਰੀ_ਅੰਕ_ਚੌੜਾਈ_2, 0);
  APInt two(ਦੋਆਧਾਰੀ_ਅੰਕ_ਚੌੜਾਈ_2, 2);

  // Select a good starting value using binary logarithms.
  ਲਈ (;; i += 2, testy = testy.shl(2))
    ਜੇ (i >= nbits || ਇਹ->ule(testy)) {
      x_old = x_old.shl(i / 2);
      ਰੋਕੋ;
    }

  // Use the Babylonian method to arrive at the integer square root:
  ਲਈ (;;) {
    x_new = (ਇਹ->udiv(x_old) + x_old).udiv(two);
    ਜੇ (x_old.ule(x_new))
      ਰੋਕੋ;
    x_old = x_new;
  }
  ਵਾਪਸ x_old;
}

/// \returns the multiplicative inverse of an odd APInt modulo 2^BitWidth.
APInt APInt::multiplicativeInverse() ਅਚਲ {
  ਪ੍ਰਤਿਜ੍ਞਾ((*ਇਹ)[0] &&
         "multiplicative inverse is only defined for odd numbers!");

  // Use Newton's method.
  APInt ਗੁਣਕ = *ਇਹ;
  APInt T;
  ਜਦੋਂ (!(T = *ਇਹ * ਗੁਣਕ).isOne())
    ਗੁਣਕ *= 2 - std::move(T);
  ਵਾਪਸ ਗੁਣਕ;
}

/// Implementation of Knuth's Algorithm D (Division of nonnegative integers)
/// from "Art of Computer Programming, Volume 2", section 4.3.1, p. 272. The
/// variables here have the same names as in the algorithm. Comments explain
/// the algorithm and any deviation from it.
ਸਥਿਰ ਖਾਲੀ KnuthDiv(uint32_t *u, uint32_t *v, uint32_t *q, uint32_t* r,
                     ਬਿਨਾਨਿਸ਼ਾਨੀ m, ਬਿਨਾਨਿਸ਼ਾਨੀ n) {
  ਪ੍ਰਤਿਜ੍ਞਾ(u && "Must provide dividend");
  ਪ੍ਰਤਿਜ੍ਞਾ(v && "Must provide divisor");
  ਪ੍ਰਤਿਜ੍ਞਾ(q && "Must provide quotient");
  ਪ੍ਰਤਿਜ੍ਞਾ(u != v && u != q && v != q && "Must use different memory");
  ਪ੍ਰਤਿਜ੍ਞਾ(n>1 && "n must be > 1");

  // b denotes the base of the number system. In our case b is 2^32.
  ਅਚਲ uint64_t b = uint64_t(1) << 32;

// The DEBUG macros here tend to be spam in the debug output if you're not
// debugging this code. Disable them unless KNUTH_DEBUG is defined.
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ KNUTH_DEBUG
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ DEBUG_KNUTH(X) LLVM_DEBUG(X)
#ਨਹੀਂਤਾਂ
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ DEBUG_KNUTH(X) ਕਰੋ {} ਜਦੋਂ(ਝੂਠ)
#ਜੇ_ਅੰਤ

  DEBUG_KNUTH(dbgs() << "KnuthDiv: m=" << m << " n=" << n << '\n');
  DEBUG_KNUTH(dbgs() << "KnuthDiv: original:");
  DEBUG_KNUTH(ਲਈ (ਪੂਰਨ i = m + n; i >= 0; i--) dbgs() << " " << u[i]);
  DEBUG_KNUTH(dbgs() << " by");
  DEBUG_KNUTH(ਲਈ (ਪੂਰਨ i = n; i > 0; i--) dbgs() << " " << v[i - 1]);
  DEBUG_KNUTH(dbgs() << '\n');
  // D1. [Normalize.] Set d = b / (v[n-1] + 1) and multiply all the digits of
  // u and v by d. Note that we have taken Knuth's advice here to use a power
  // of 2 value for d such that d * v[n-1] >= b/2 (b is the base). A power of
  // 2 allows us to shift instead of multiply and it is easy to determine the
  // shift amount from the leading zeros.  We are basically normalizing the u
  // and v so that its high bits are shifted to the top of v's range without
  // overflow. Note that this can require an extra word in u so that u must
  // be of length m+n+1.
  ਬਿਨਾਨਿਸ਼ਾਨੀ ਖਿਸਕਾਓ = llvm::countl_zero(v[n - 1]);
  uint32_t v_ਹਾਸਲ = 0;
  uint32_t u_ਹਾਸਲ = 0;
  ਜੇ (ਖਿਸਕਾਓ) {
    ਲਈ (ਬਿਨਾਨਿਸ਼ਾਨੀ i = 0; i < m+n; ++i) {
      uint32_t u_tmp = u[i] >> (32 - ਖਿਸਕਾਓ);
      u[i] = (u[i] << ਖਿਸਕਾਓ) | u_ਹਾਸਲ;
      u_ਹਾਸਲ = u_tmp;
    }
    ਲਈ (ਬਿਨਾਨਿਸ਼ਾਨੀ i = 0; i < n; ++i) {
      uint32_t v_tmp = v[i] >> (32 - ਖਿਸਕਾਓ);
      v[i] = (v[i] << ਖਿਸਕਾਓ) | v_ਹਾਸਲ;
      v_ਹਾਸਲ = v_tmp;
    }
  }
  u[m+n] = u_ਹਾਸਲ;

  DEBUG_KNUTH(dbgs() << "KnuthDiv:   normal:");
  DEBUG_KNUTH(ਲਈ (ਪੂਰਨ i = m + n; i >= 0; i--) dbgs() << " " << u[i]);
  DEBUG_KNUTH(dbgs() << " by");
  DEBUG_KNUTH(ਲਈ (ਪੂਰਨ i = n; i > 0; i--) dbgs() << " " << v[i - 1]);
  DEBUG_KNUTH(dbgs() << '\n');

  // D2. [Initialize j.]  Set j to m. This is the loop counter over the places.
  ਪੂਰਨ j = m;
  ਕਰੋ {
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
    ਜੇ (qp == b || qp*v[n-2] > b*rp + u[j+n-2]) {
      qp--;
      rp += v[n-1];
      ਜੇ (rp < b && (qp == b || qp*v[n-2] > b*rp + u[j+n-2]))
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
    ਲਈ (ਬਿਨਾਨਿਸ਼ਾਨੀ i = 0; i < n; ++i) {
      uint64_t p = qp * uint64_t(v[i]);
      int64_t subres = int64_t(u[j+i]) - borrow - Lo_32(p);
      u[j+i] = Lo_32(subres);
      borrow = Hi_32(p) - Hi_32(subres);
      DEBUG_KNUTH(dbgs() << "KnuthDiv: u[j+i] = " << u[j + i]
                        << ", borrow = " << borrow << '\n');
    }
    ਤਾਰਕਿਕ isNeg = u[j+n] < borrow;
    u[j+n] -= Lo_32(borrow);

    DEBUG_KNUTH(dbgs() << "KnuthDiv: after subtraction:");
    DEBUG_KNUTH(ਲਈ (ਪੂਰਨ i = m + n; i >= 0; i--) dbgs() << " " << u[i]);
    DEBUG_KNUTH(dbgs() << '\n');

    // D5. [Test remainder.] Set q[j] = qp. If the result of step D4 was
    // negative, go to step D6; otherwise go on to step D7.
    q[j] = Lo_32(qp);
    ਜੇ (isNeg) {
      // D6. [Add back]. The probability that this step is necessary is very
      // small, on the order of only 2/b. Make sure that test data accounts for
      // this possibility. Decrease q[j] by 1
      q[j]--;
      // and add (0v[n-1]...v[1]v[0]) to (u[j+n]u[j+n-1]...u[j+1]u[j]).
      // A carry will occur to the left of u[j+n], and it should be ignored
      // since it cancels with the borrow that occurred in D4.
      ਤਾਰਕਿਕ ਹਾਸਲ = ਝੂਠ;
      ਲਈ (ਬਿਨਾਨਿਸ਼ਾਨੀ i = 0; i < n; i++) {
        uint32_t ਹੱਦਬੰਦੀ = std::min(u[j+i],v[i]);
        u[j+i] += v[i] + ਹਾਸਲ;
        ਹਾਸਲ = u[j+i] < ਹੱਦਬੰਦੀ || (ਹਾਸਲ && u[j+i] == ਹੱਦਬੰਦੀ);
      }
      u[j+n] += ਹਾਸਲ;
    }
    DEBUG_KNUTH(dbgs() << "KnuthDiv: after correction:");
    DEBUG_KNUTH(ਲਈ (ਪੂਰਨ i = m + n; i >= 0; i--) dbgs() << " " << u[i]);
    DEBUG_KNUTH(dbgs() << "\nKnuthDiv: digit result = " << q[j] << '\n');

    // D7. [Loop on j.]  Decrease j by one. Now if j >= 0, go back to D3.
  } ਜਦੋਂ (--j >= 0);

  DEBUG_KNUTH(dbgs() << "KnuthDiv: quotient:");
  DEBUG_KNUTH(ਲਈ (ਪੂਰਨ i = m; i >= 0; i--) dbgs() << " " << q[i]);
  DEBUG_KNUTH(dbgs() << '\n');

  // D8. [Unnormalize]. Now q[...] is the desired quotient, and the desired
  // remainder may be obtained by dividing u[...] by d. If r is non-null we
  // compute the remainder (urem uses this).
  ਜੇ (r) {
    // The value d is expressed by the "shift" value above since we avoided
    // multiplication by d by using a shift left. So, all we have to do is
    // shift right here.
    ਜੇ (ਖਿਸਕਾਓ) {
      uint32_t ਹਾਸਲ = 0;
      DEBUG_KNUTH(dbgs() << "KnuthDiv: remainder:");
      ਲਈ (ਪੂਰਨ i = n-1; i >= 0; i--) {
        r[i] = (u[i] >> ਖਿਸਕਾਓ) | ਹਾਸਲ;
        ਹਾਸਲ = u[i] << (32 - ਖਿਸਕਾਓ);
        DEBUG_KNUTH(dbgs() << " " << r[i]);
      }
    } ਨਹੀਂਤਾਂ {
      ਲਈ (ਪੂਰਨ i = n-1; i >= 0; i--) {
        r[i] = u[i];
        DEBUG_KNUTH(dbgs() << " " << r[i]);
      }
    }
    DEBUG_KNUTH(dbgs() << '\n');
  }
  DEBUG_KNUTH(dbgs() << '\n');
}

ਖਾਲੀ APInt::divide(ਅਚਲ ਸ਼ਬਦ_Type *LHS, ਬਿਨਾਨਿਸ਼ਾਨੀ lhs_ਸ਼ਬਦ, ਅਚਲ ਸ਼ਬਦ_Type *RHS,
                   ਬਿਨਾਨਿਸ਼ਾਨੀ rhs_ਸ਼ਬਦ, ਸ਼ਬਦ_Type *Quotient, ਸ਼ਬਦ_Type *Remainder) {
  ਪ੍ਰਤਿਜ੍ਞਾ(lhs_ਸ਼ਬਦ >= rhs_ਸ਼ਬਦ && "Fractional result");

  // First, compose the values into an array of 32-bit words instead of
  // 64-bit words. This is a necessity of both the "short division" algorithm
  // and the Knuth "classical algorithm" which requires there to be native
  // operations for +, -, and * on an m bit value with an m*2 bit result. We
  // can't use 64-bit operands here because we don't have native results of
  // 128-bits. Furthermore, casting the 64-bit values to 32-bit values won't
  // work on large-endian machines.
  ਬਿਨਾਨਿਸ਼ਾਨੀ n = rhs_ਸ਼ਬਦ * 2;
  ਬਿਨਾਨਿਸ਼ਾਨੀ m = (lhs_ਸ਼ਬਦ * 2) - n;

  // Allocate space for the temporary values we need either on the stack, if
  // it will fit, or on the heap if it won't.
  uint32_t SPACE[128];
  uint32_t *U = ਖਾਲੀ_ਸੂਚਕ;
  uint32_t *V = ਖਾਲੀ_ਸੂਚਕ;
  uint32_t *Q = ਖਾਲੀ_ਸੂਚਕ;
  uint32_t *R = ਖਾਲੀ_ਸੂਚਕ;
  ਜੇ ((Remainder?4:3)*n+2*m+1 <= 128) {
    U = &SPACE[0];
    V = &SPACE[m+n+1];
    Q = &SPACE[(m+n+1) + n];
    ਜੇ (Remainder)
      R = &SPACE[(m+n+1) + n + (m+n)];
  } ਨਹੀਂਤਾਂ {
    U = ਨਵਾਂ uint32_t[m + n + 1];
    V = ਨਵਾਂ uint32_t[n];
    Q = ਨਵਾਂ uint32_t[m+n];
    ਜੇ (Remainder)
      R = ਨਵਾਂ uint32_t[n];
  }

  // Initialize the dividend
  memset(U, 0, (m+n+1)*ਆਕਾਰ(uint32_t));
  ਲਈ (ਬਿਨਾਨਿਸ਼ਾਨੀ i = 0; i < lhs_ਸ਼ਬਦ; ++i) {
    uint64_t tmp = LHS[i];
    U[i * 2] = Lo_32(tmp);
    U[i * 2 + 1] = Hi_32(tmp);
  }
  U[m+n] = 0; // this extra word is for "spill" in the Knuth algorithm.

  // Initialize the divisor
  memset(V, 0, (n)*ਆਕਾਰ(uint32_t));
  ਲਈ (ਬਿਨਾਨਿਸ਼ਾਨੀ i = 0; i < rhs_ਸ਼ਬਦ; ++i) {
    uint64_t tmp = RHS[i];
    V[i * 2] = Lo_32(tmp);
    V[i * 2 + 1] = Hi_32(tmp);
  }

  // initialize the quotient and remainder
  memset(Q, 0, (m+n) * ਆਕਾਰ(uint32_t));
  ਜੇ (Remainder)
    memset(R, 0, n * ਆਕਾਰ(uint32_t));

  // Now, adjust m and n for the Knuth division. n is the number of words in
  // the divisor. m is the number of words by which the dividend exceeds the
  // divisor (i.e. m+n is the length of the dividend). These sizes must not
  // contain any zero words or the Knuth algorithm fails.
  ਲਈ (ਬਿਨਾਨਿਸ਼ਾਨੀ i = n; i > 0 && V[i-1] == 0; i--) {
    n--;
    m++;
  }
  ਲਈ (ਬਿਨਾਨਿਸ਼ਾਨੀ i = m+n; i > 0 && U[i-1] == 0; i--)
    m--;

  // If we're left with only a single word for the divisor, Knuth doesn't work
  // so we implement the short division algorithm here. This is much simpler
  // and faster because we are certain that we can divide a 64-bit quantity
  // by a 32-bit quantity at hardware speed and short division is simply a
  // series of such operations. This is just like doing short division but we
  // are using base 2^32 instead of base 10.
  ਪ੍ਰਤਿਜ੍ਞਾ(n != 0 && "Divide by zero?");
  ਜੇ (n == 1) {
    uint32_t ਭਾਜਕ = V[0];
    uint32_t remainder = 0;
    ਲਈ (ਪੂਰਨ i = m; i >= 0; i--) {
      uint64_t partial_dividend = Make_64(remainder, U[i]);
      ਜੇ (partial_dividend == 0) {
        Q[i] = 0;
        remainder = 0;
      } ਨਹੀਂਤਾਂ ਜੇ (partial_dividend < ਭਾਜਕ) {
        Q[i] = 0;
        remainder = Lo_32(partial_dividend);
      } ਨਹੀਂਤਾਂ ਜੇ (partial_dividend == ਭਾਜਕ) {
        Q[i] = 1;
        remainder = 0;
      } ਨਹੀਂਤਾਂ {
        Q[i] = Lo_32(partial_dividend / ਭਾਜਕ);
        remainder = Lo_32(partial_dividend - (Q[i] * ਭਾਜਕ));
      }
    }
    ਜੇ (R)
      R[0] = remainder;
  } ਨਹੀਂਤਾਂ {
    // Now we're ready to invoke the Knuth classical divide algorithm. In this
    // case n > 1.
    KnuthDiv(U, V, Q, R, m, n);
  }

  // If the caller wants the quotient
  ਜੇ (Quotient) {
    ਲਈ (ਬਿਨਾਨਿਸ਼ਾਨੀ i = 0; i < lhs_ਸ਼ਬਦ; ++i)
      Quotient[i] = Make_64(Q[i*2+1], Q[i*2]);
  }

  // If the caller wants the remainder
  ਜੇ (Remainder) {
    ਲਈ (ਬਿਨਾਨਿਸ਼ਾਨੀ i = 0; i < rhs_ਸ਼ਬਦ; ++i)
      Remainder[i] = Make_64(R[i*2+1], R[i*2]);
  }

  // Clean up the memory we allocated.
  ਜੇ (U != &SPACE[0]) {
    ਮਿਟਾਓ [] U;
    ਮਿਟਾਓ [] V;
    ਮਿਟਾਓ [] Q;
    ਮਿਟਾਓ [] R;
  }
}

APInt APInt::udiv(ਅਚਲ APInt &RHS) ਅਚਲ {
  ਪ੍ਰਤਿਜ੍ਞਾ(ਦੋਆਧਾਰੀ_ਅੰਕ_ਚੌੜਾਈ_2 == RHS.ਦੋਆਧਾਰੀ_ਅੰਕ_ਚੌੜਾਈ_2 && "Bit widths must be the same");

  // First, deal with the easy case
  ਜੇ (LLVM_LIKELY(is_Single_ਸ਼ਬਦ())) {
    ਪ੍ਰਤਿਜ੍ਞਾ(RHS.U.VAL != 0 && "Divide by zero?");
    ਵਾਪਸ APInt(ਦੋਆਧਾਰੀ_ਅੰਕ_ਚੌੜਾਈ_2, U.VAL / RHS.U.VAL);
  }

  // Get some facts about the LHS and RHS number of bits and words
  ਬਿਨਾਨਿਸ਼ਾਨੀ lhs_ਸ਼ਬਦ = get_ਅੰਕ_ਸ਼ਬਦ(get_ਸਰਗਰਮ_ਬਿੱਟ());
  ਬਿਨਾਨਿਸ਼ਾਨੀ rhs_ਬਿੱਟ  = RHS.get_ਸਰਗਰਮ_ਬਿੱਟ();
  ਬਿਨਾਨਿਸ਼ਾਨੀ rhs_ਸ਼ਬਦ = get_ਅੰਕ_ਸ਼ਬਦ(rhs_ਬਿੱਟ);
  ਪ੍ਰਤਿਜ੍ਞਾ(rhs_ਸ਼ਬਦ && "Divided by zero???");

  // Deal with some degenerate cases
  ਜੇ (!lhs_ਸ਼ਬਦ)
    // 0 / X ===> 0
    ਵਾਪਸ APInt(ਦੋਆਧਾਰੀ_ਅੰਕ_ਚੌੜਾਈ_2, 0);
  ਜੇ (rhs_ਬਿੱਟ == 1)
    // X / 1 ===> X
    ਵਾਪਸ *ਇਹ;
  ਜੇ (lhs_ਸ਼ਬਦ < rhs_ਸ਼ਬਦ || ਇਹ->ult(RHS))
    // X / Y ===> 0, iff X < Y
    ਵਾਪਸ APInt(ਦੋਆਧਾਰੀ_ਅੰਕ_ਚੌੜਾਈ_2, 0);
  ਜੇ (*ਇਹ == RHS)
    // X / X ===> 1
    ਵਾਪਸ APInt(ਦੋਆਧਾਰੀ_ਅੰਕ_ਚੌੜਾਈ_2, 1);
  ਜੇ (lhs_ਸ਼ਬਦ == 1) // rhsWords is 1 if lhsWords is 1.
    // All high words are zero, just use native divide
    ਵਾਪਸ APInt(ਦੋਆਧਾਰੀ_ਅੰਕ_ਚੌੜਾਈ_2, ਇਹ->U.pVal[0] / RHS.U.pVal[0]);

  // We have to compute it the hard way. Invoke the Knuth divide algorithm.
  APInt Quotient(ਦੋਆਧਾਰੀ_ਅੰਕ_ਚੌੜਾਈ_2, 0); // to hold result.
  divide(U.pVal, lhs_ਸ਼ਬਦ, RHS.U.pVal, rhs_ਸ਼ਬਦ, Quotient.U.pVal, ਖਾਲੀ_ਸੂਚਕ);
  ਵਾਪਸ Quotient;
}

APInt APInt::udiv(uint64_t RHS) ਅਚਲ {
  ਪ੍ਰਤਿਜ੍ਞਾ(RHS != 0 && "Divide by zero?");

  // First, deal with the easy case
  ਜੇ (LLVM_LIKELY(is_Single_ਸ਼ਬਦ()))
    ਵਾਪਸ APInt(ਦੋਆਧਾਰੀ_ਅੰਕ_ਚੌੜਾਈ_2, U.VAL / RHS);

  // Get some facts about the LHS words.
  ਬਿਨਾਨਿਸ਼ਾਨੀ lhs_ਸ਼ਬਦ = get_ਅੰਕ_ਸ਼ਬਦ(get_ਸਰਗਰਮ_ਬਿੱਟ());

  // Deal with some degenerate cases
  ਜੇ (!lhs_ਸ਼ਬਦ)
    // 0 / X ===> 0
    ਵਾਪਸ APInt(ਦੋਆਧਾਰੀ_ਅੰਕ_ਚੌੜਾਈ_2, 0);
  ਜੇ (RHS == 1)
    // X / 1 ===> X
    ਵਾਪਸ *ਇਹ;
  ਜੇ (ਇਹ->ult(RHS))
    // X / Y ===> 0, iff X < Y
    ਵਾਪਸ APInt(ਦੋਆਧਾਰੀ_ਅੰਕ_ਚੌੜਾਈ_2, 0);
  ਜੇ (*ਇਹ == RHS)
    // X / X ===> 1
    ਵਾਪਸ APInt(ਦੋਆਧਾਰੀ_ਅੰਕ_ਚੌੜਾਈ_2, 1);
  ਜੇ (lhs_ਸ਼ਬਦ == 1) // rhsWords is 1 if lhsWords is 1.
    // All high words are zero, just use native divide
    ਵਾਪਸ APInt(ਦੋਆਧਾਰੀ_ਅੰਕ_ਚੌੜਾਈ_2, ਇਹ->U.pVal[0] / RHS);

  // We have to compute it the hard way. Invoke the Knuth divide algorithm.
  APInt Quotient(ਦੋਆਧਾਰੀ_ਅੰਕ_ਚੌੜਾਈ_2, 0); // to hold result.
  divide(U.pVal, lhs_ਸ਼ਬਦ, &RHS, 1, Quotient.U.pVal, ਖਾਲੀ_ਸੂਚਕ);
  ਵਾਪਸ Quotient;
}

APInt APInt::sdiv(ਅਚਲ APInt &RHS) ਅਚਲ {
  ਜੇ (isNegative()) {
    ਜੇ (RHS.isNegative())
      ਵਾਪਸ (-(*ਇਹ)).udiv(-RHS);
    ਵਾਪਸ -((-(*ਇਹ)).udiv(RHS));
  }
  ਜੇ (RHS.isNegative())
    ਵਾਪਸ -(ਇਹ->udiv(-RHS));
  ਵਾਪਸ ਇਹ->udiv(RHS);
}

APInt APInt::sdiv(int64_t RHS) ਅਚਲ {
  ਜੇ (isNegative()) {
    ਜੇ (RHS < 0)
      ਵਾਪਸ (-(*ਇਹ)).udiv(-RHS);
    ਵਾਪਸ -((-(*ਇਹ)).udiv(RHS));
  }
  ਜੇ (RHS < 0)
    ਵਾਪਸ -(ਇਹ->udiv(-RHS));
  ਵਾਪਸ ਇਹ->udiv(RHS);
}

APInt APInt::urem(ਅਚਲ APInt &RHS) ਅਚਲ {
  ਪ੍ਰਤਿਜ੍ਞਾ(ਦੋਆਧਾਰੀ_ਅੰਕ_ਚੌੜਾਈ_2 == RHS.ਦੋਆਧਾਰੀ_ਅੰਕ_ਚੌੜਾਈ_2 && "Bit widths must be the same");
  ਜੇ (LLVM_LIKELY(is_Single_ਸ਼ਬਦ())) {
    ਪ੍ਰਤਿਜ੍ਞਾ(RHS.U.VAL != 0 && "Remainder by zero?");
    ਵਾਪਸ APInt(ਦੋਆਧਾਰੀ_ਅੰਕ_ਚੌੜਾਈ_2, U.VAL % RHS.U.VAL);
  }

  // Get some facts about the LHS
  ਬਿਨਾਨਿਸ਼ਾਨੀ lhs_ਸ਼ਬਦ = get_ਅੰਕ_ਸ਼ਬਦ(get_ਸਰਗਰਮ_ਬਿੱਟ());

  // Get some facts about the RHS
  ਬਿਨਾਨਿਸ਼ਾਨੀ rhs_ਬਿੱਟ = RHS.get_ਸਰਗਰਮ_ਬਿੱਟ();
  ਬਿਨਾਨਿਸ਼ਾਨੀ rhs_ਸ਼ਬਦ = get_ਅੰਕ_ਸ਼ਬਦ(rhs_ਬਿੱਟ);
  ਪ੍ਰਤਿਜ੍ਞਾ(rhs_ਸ਼ਬਦ && "Performing remainder operation by zero ???");

  // Check the degenerate cases
  ਜੇ (lhs_ਸ਼ਬਦ == 0)
    // 0 % Y ===> 0
    ਵਾਪਸ APInt(ਦੋਆਧਾਰੀ_ਅੰਕ_ਚੌੜਾਈ_2, 0);
  ਜੇ (rhs_ਬਿੱਟ == 1)
    // X % 1 ===> 0
    ਵਾਪਸ APInt(ਦੋਆਧਾਰੀ_ਅੰਕ_ਚੌੜਾਈ_2, 0);
  ਜੇ (lhs_ਸ਼ਬਦ < rhs_ਸ਼ਬਦ || ਇਹ->ult(RHS))
    // X % Y ===> X, iff X < Y
    ਵਾਪਸ *ਇਹ;
  ਜੇ (*ਇਹ == RHS)
    // X % X == 0;
    ਵਾਪਸ APInt(ਦੋਆਧਾਰੀ_ਅੰਕ_ਚੌੜਾਈ_2, 0);
  ਜੇ (lhs_ਸ਼ਬਦ == 1)
    // All high words are zero, just use native remainder
    ਵਾਪਸ APInt(ਦੋਆਧਾਰੀ_ਅੰਕ_ਚੌੜਾਈ_2, U.pVal[0] % RHS.U.pVal[0]);
  ਜੇ (RHS.isPowerOf2()) {
    // X % 2^w ===> X & (2^w - 1)
    APInt ਨਤੀਜਾ(*ਇਹ);
    ਨਤੀਜਾ.clear_ਬਿੱਟ(RHS.logBase2(), ਦੋਆਧਾਰੀ_ਅੰਕ_ਚੌੜਾਈ_2);
    ਵਾਪਸ ਨਤੀਜਾ;
  }

  // We have to compute it the hard way. Invoke the Knuth divide algorithm.
  APInt Remainder(ਦੋਆਧਾਰੀ_ਅੰਕ_ਚੌੜਾਈ_2, 0);
  divide(U.pVal, lhs_ਸ਼ਬਦ, RHS.U.pVal, rhs_ਸ਼ਬਦ, ਖਾਲੀ_ਸੂਚਕ, Remainder.U.pVal);
  ਵਾਪਸ Remainder;
}

uint64_t APInt::urem(uint64_t RHS) ਅਚਲ {
  ਪ੍ਰਤਿਜ੍ਞਾ(RHS != 0 && "Remainder by zero?");

  ਜੇ (LLVM_LIKELY(is_Single_ਸ਼ਬਦ()))
    ਵਾਪਸ U.VAL % RHS;

  // Get some facts about the LHS
  ਬਿਨਾਨਿਸ਼ਾਨੀ lhs_ਸ਼ਬਦ = get_ਅੰਕ_ਸ਼ਬਦ(get_ਸਰਗਰਮ_ਬਿੱਟ());

  // Check the degenerate cases
  ਜੇ (lhs_ਸ਼ਬਦ == 0)
    // 0 % Y ===> 0
    ਵਾਪਸ 0;
  ਜੇ (RHS == 1)
    // X % 1 ===> 0
    ਵਾਪਸ 0;
  ਜੇ (ਇਹ->ult(RHS))
    // X % Y ===> X, iff X < Y
    ਵਾਪਸ get_ZExt_ਮੁੱਲ();
  ਜੇ (*ਇਹ == RHS)
    // X % X == 0;
    ਵਾਪਸ 0;
  ਜੇ (lhs_ਸ਼ਬਦ == 1)
    // All high words are zero, just use native remainder
    ਵਾਪਸ U.pVal[0] % RHS;
  ਜੇ (llvm::isPowerOf2_64(RHS))
    // X % 2^w ===> X & (2^w - 1)
    ਵਾਪਸ U.pVal[0] & (RHS - 1);

  // We have to compute it the hard way. Invoke the Knuth divide algorithm.
  uint64_t Remainder;
  divide(U.pVal, lhs_ਸ਼ਬਦ, &RHS, 1, ਖਾਲੀ_ਸੂਚਕ, &Remainder);
  ਵਾਪਸ Remainder;
}

APInt APInt::srem(ਅਚਲ APInt &RHS) ਅਚਲ {
  ਜੇ (isNegative()) {
    ਜੇ (RHS.isNegative())
      ਵਾਪਸ -((-(*ਇਹ)).urem(-RHS));
    ਵਾਪਸ -((-(*ਇਹ)).urem(RHS));
  }
  ਜੇ (RHS.isNegative())
    ਵਾਪਸ ਇਹ->urem(-RHS);
  ਵਾਪਸ ਇਹ->urem(RHS);
}

int64_t APInt::srem(int64_t RHS) ਅਚਲ {
  ਜੇ (isNegative()) {
    ਜੇ (RHS < 0)
      ਵਾਪਸ -((-(*ਇਹ)).urem(-RHS));
    ਵਾਪਸ -((-(*ਇਹ)).urem(RHS));
  }
  ਜੇ (RHS < 0)
    ਵਾਪਸ ਇਹ->urem(-RHS);
  ਵਾਪਸ ਇਹ->urem(RHS);
}

ਖਾਲੀ APInt::udivrem(ਅਚਲ APInt &LHS, ਅਚਲ APInt &RHS,
                    APInt &Quotient, APInt &Remainder) {
  ਪ੍ਰਤਿਜ੍ਞਾ(LHS.ਦੋਆਧਾਰੀ_ਅੰਕ_ਚੌੜਾਈ_2 == RHS.ਦੋਆਧਾਰੀ_ਅੰਕ_ਚੌੜਾਈ_2 && "Bit widths must be the same");
  ਬਿਨਾਨਿਸ਼ਾਨੀ ਦੋਆਧਾਰੀ_ਅੰਕ_ਚੌੜਾਈ_2 = LHS.ਦੋਆਧਾਰੀ_ਅੰਕ_ਚੌੜਾਈ_2;

  // First, deal with the easy case
  ਜੇ (LLVM_LIKELY(LHS.is_Single_ਸ਼ਬਦ())) {
    ਪ੍ਰਤਿਜ੍ਞਾ(RHS.U.VAL != 0 && "Divide by zero?");
    uint64_t QuotVal = LHS.U.VAL / RHS.U.VAL;
    uint64_t RemVal = LHS.U.VAL % RHS.U.VAL;
    Quotient = APInt(ਦੋਆਧਾਰੀ_ਅੰਕ_ਚੌੜਾਈ_2, QuotVal);
    Remainder = APInt(ਦੋਆਧਾਰੀ_ਅੰਕ_ਚੌੜਾਈ_2, RemVal);
    ਵਾਪਸ;
  }

  // Get some size facts about the dividend and divisor
  ਬਿਨਾਨਿਸ਼ਾਨੀ lhs_ਸ਼ਬਦ = get_ਅੰਕ_ਸ਼ਬਦ(LHS.get_ਸਰਗਰਮ_ਬਿੱਟ());
  ਬਿਨਾਨਿਸ਼ਾਨੀ rhs_ਬਿੱਟ  = RHS.get_ਸਰਗਰਮ_ਬਿੱਟ();
  ਬਿਨਾਨਿਸ਼ਾਨੀ rhs_ਸ਼ਬਦ = get_ਅੰਕ_ਸ਼ਬਦ(rhs_ਬਿੱਟ);
  ਪ੍ਰਤਿਜ੍ਞਾ(rhs_ਸ਼ਬਦ && "Performing divrem operation by zero ???");

  // Check the degenerate cases
  ਜੇ (lhs_ਸ਼ਬਦ == 0) {
    Quotient = APInt(ਦੋਆਧਾਰੀ_ਅੰਕ_ਚੌੜਾਈ_2, 0);    // 0 / Y ===> 0
    Remainder = APInt(ਦੋਆਧਾਰੀ_ਅੰਕ_ਚੌੜਾਈ_2, 0);   // 0 % Y ===> 0
    ਵਾਪਸ;
  }

  ਜੇ (rhs_ਬਿੱਟ == 1) {
    Quotient = LHS;                   // X / 1 ===> X
    Remainder = APInt(ਦੋਆਧਾਰੀ_ਅੰਕ_ਚੌੜਾਈ_2, 0);   // X % 1 ===> 0
  }

  ਜੇ (lhs_ਸ਼ਬਦ < rhs_ਸ਼ਬਦ || LHS.ult(RHS)) {
    Remainder = LHS;                  // X % Y ===> X, iff X < Y
    Quotient = APInt(ਦੋਆਧਾਰੀ_ਅੰਕ_ਚੌੜਾਈ_2, 0);    // X / Y ===> 0, iff X < Y
    ਵਾਪਸ;
  }

  ਜੇ (LHS == RHS) {
    Quotient  = APInt(ਦੋਆਧਾਰੀ_ਅੰਕ_ਚੌੜਾਈ_2, 1);   // X / X ===> 1
    Remainder = APInt(ਦੋਆਧਾਰੀ_ਅੰਕ_ਚੌੜਾਈ_2, 0);   // X % X ===> 0;
    ਵਾਪਸ;
  }

  // Make sure there is enough space to hold the results.
  // NOTE: This assumes that reallocate won't affect any bits if it doesn't
  // change the size. This is necessary if Quotient or Remainder is aliased
  // with LHS or RHS.
  Quotient.reallocate(ਦੋਆਧਾਰੀ_ਅੰਕ_ਚੌੜਾਈ_2);
  Remainder.reallocate(ਦੋਆਧਾਰੀ_ਅੰਕ_ਚੌੜਾਈ_2);

  ਜੇ (lhs_ਸ਼ਬਦ == 1) { // rhsWords is 1 if lhsWords is 1.
    // There is only one word to consider so use the native versions.
    uint64_t lhs_ਮੁੱਲ = LHS.U.pVal[0];
    uint64_t rhs_ਮੁੱਲ = RHS.U.pVal[0];
    Quotient = lhs_ਮੁੱਲ / rhs_ਮੁੱਲ;
    Remainder = lhs_ਮੁੱਲ % rhs_ਮੁੱਲ;
    ਵਾਪਸ;
  }

  // Okay, lets do it the long way
  divide(LHS.U.pVal, lhs_ਸ਼ਬਦ, RHS.U.pVal, rhs_ਸ਼ਬਦ, Quotient.U.pVal,
         Remainder.U.pVal);
  // Clear the rest of the Quotient and Remainder.
  std::memset(Quotient.U.pVal + lhs_ਸ਼ਬਦ, 0,
              (get_ਅੰਕ_ਸ਼ਬਦ(ਦੋਆਧਾਰੀ_ਅੰਕ_ਚੌੜਾਈ_2) - lhs_ਸ਼ਬਦ) * APINT_ਸ਼ਬਦ_ਮਾਪ);
  std::memset(Remainder.U.pVal + rhs_ਸ਼ਬਦ, 0,
              (get_ਅੰਕ_ਸ਼ਬਦ(ਦੋਆਧਾਰੀ_ਅੰਕ_ਚੌੜਾਈ_2) - rhs_ਸ਼ਬਦ) * APINT_ਸ਼ਬਦ_ਮਾਪ);
}

ਖਾਲੀ APInt::udivrem(ਅਚਲ APInt &LHS, uint64_t RHS, APInt &Quotient,
                    uint64_t &Remainder) {
  ਪ੍ਰਤਿਜ੍ਞਾ(RHS != 0 && "Divide by zero?");
  ਬਿਨਾਨਿਸ਼ਾਨੀ ਦੋਆਧਾਰੀ_ਅੰਕ_ਚੌੜਾਈ_2 = LHS.ਦੋਆਧਾਰੀ_ਅੰਕ_ਚੌੜਾਈ_2;

  // First, deal with the easy case
  ਜੇ (LLVM_LIKELY(LHS.is_Single_ਸ਼ਬਦ())) {
    uint64_t QuotVal = LHS.U.VAL / RHS;
    Remainder = LHS.U.VAL % RHS;
    Quotient = APInt(ਦੋਆਧਾਰੀ_ਅੰਕ_ਚੌੜਾਈ_2, QuotVal);
    ਵਾਪਸ;
  }

  // Get some size facts about the dividend and divisor
  ਬਿਨਾਨਿਸ਼ਾਨੀ lhs_ਸ਼ਬਦ = get_ਅੰਕ_ਸ਼ਬਦ(LHS.get_ਸਰਗਰਮ_ਬਿੱਟ());

  // Check the degenerate cases
  ਜੇ (lhs_ਸ਼ਬਦ == 0) {
    Quotient = APInt(ਦੋਆਧਾਰੀ_ਅੰਕ_ਚੌੜਾਈ_2, 0);    // 0 / Y ===> 0
    Remainder = 0;                    // 0 % Y ===> 0
    ਵਾਪਸ;
  }

  ਜੇ (RHS == 1) {
    Quotient = LHS;                   // X / 1 ===> X
    Remainder = 0;                    // X % 1 ===> 0
    ਵਾਪਸ;
  }

  ਜੇ (LHS.ult(RHS)) {
    Remainder = LHS.get_ZExt_ਮੁੱਲ();   // X % Y ===> X, iff X < Y
    Quotient = APInt(ਦੋਆਧਾਰੀ_ਅੰਕ_ਚੌੜਾਈ_2, 0);    // X / Y ===> 0, iff X < Y
    ਵਾਪਸ;
  }

  ਜੇ (LHS == RHS) {
    Quotient  = APInt(ਦੋਆਧਾਰੀ_ਅੰਕ_ਚੌੜਾਈ_2, 1);   // X / X ===> 1
    Remainder = 0;                    // X % X ===> 0;
    ਵਾਪਸ;
  }

  // Make sure there is enough space to hold the results.
  // NOTE: This assumes that reallocate won't affect any bits if it doesn't
  // change the size. This is necessary if Quotient is aliased with LHS.
  Quotient.reallocate(ਦੋਆਧਾਰੀ_ਅੰਕ_ਚੌੜਾਈ_2);

  ਜੇ (lhs_ਸ਼ਬਦ == 1) { // rhsWords is 1 if lhsWords is 1.
    // There is only one word to consider so use the native versions.
    uint64_t lhs_ਮੁੱਲ = LHS.U.pVal[0];
    Quotient = lhs_ਮੁੱਲ / RHS;
    Remainder = lhs_ਮੁੱਲ % RHS;
    ਵਾਪਸ;
  }

  // Okay, lets do it the long way
  divide(LHS.U.pVal, lhs_ਸ਼ਬਦ, &RHS, 1, Quotient.U.pVal, &Remainder);
  // Clear the rest of the Quotient.
  std::memset(Quotient.U.pVal + lhs_ਸ਼ਬਦ, 0,
              (get_ਅੰਕ_ਸ਼ਬਦ(ਦੋਆਧਾਰੀ_ਅੰਕ_ਚੌੜਾਈ_2) - lhs_ਸ਼ਬਦ) * APINT_ਸ਼ਬਦ_ਮਾਪ);
}

ਖਾਲੀ APInt::sdivrem(ਅਚਲ APInt &LHS, ਅਚਲ APInt &RHS,
                    APInt &Quotient, APInt &Remainder) {
  ਜੇ (LHS.isNegative()) {
    ਜੇ (RHS.isNegative())
      APInt::udivrem(-LHS, -RHS, Quotient, Remainder);
    ਨਹੀਂਤਾਂ {
      APInt::udivrem(-LHS, RHS, Quotient, Remainder);
      Quotient.negate();
    }
    Remainder.negate();
  } ਨਹੀਂਤਾਂ ਜੇ (RHS.isNegative()) {
    APInt::udivrem(LHS, -RHS, Quotient, Remainder);
    Quotient.negate();
  } ਨਹੀਂਤਾਂ {
    APInt::udivrem(LHS, RHS, Quotient, Remainder);
  }
}

ਖਾਲੀ APInt::sdivrem(ਅਚਲ APInt &LHS, int64_t RHS,
                    APInt &Quotient, int64_t &Remainder) {
  uint64_t R = Remainder;
  ਜੇ (LHS.isNegative()) {
    ਜੇ (RHS < 0)
      APInt::udivrem(-LHS, -RHS, Quotient, R);
    ਨਹੀਂਤਾਂ {
      APInt::udivrem(-LHS, RHS, Quotient, R);
      Quotient.negate();
    }
    R = -R;
  } ਨਹੀਂਤਾਂ ਜੇ (RHS < 0) {
    APInt::udivrem(LHS, -RHS, Quotient, R);
    Quotient.negate();
  } ਨਹੀਂਤਾਂ {
    APInt::udivrem(LHS, RHS, Quotient, R);
  }
  Remainder = R;
}

APInt APInt::sadd_ov(ਅਚਲ APInt &RHS, ਤਾਰਕਿਕ &ਉਛਾਲ) ਅਚਲ {
  APInt Res = *ਇਹ+RHS;
  ਉਛਾਲ = isNonNegative() == RHS.isNonNegative() &&
             Res.isNonNegative() != isNonNegative();
  ਵਾਪਸ Res;
}

APInt APInt::uadd_ov(ਅਚਲ APInt &RHS, ਤਾਰਕਿਕ &ਉਛਾਲ) ਅਚਲ {
  APInt Res = *ਇਹ+RHS;
  ਉਛਾਲ = Res.ult(RHS);
  ਵਾਪਸ Res;
}

APInt APInt::ssub_ov(ਅਚਲ APInt &RHS, ਤਾਰਕਿਕ &ਉਛਾਲ) ਅਚਲ {
  APInt Res = *ਇਹ - RHS;
  ਉਛਾਲ = isNonNegative() != RHS.isNonNegative() &&
             Res.isNonNegative() != isNonNegative();
  ਵਾਪਸ Res;
}

APInt APInt::usub_ov(ਅਚਲ APInt &RHS, ਤਾਰਕਿਕ &ਉਛਾਲ) ਅਚਲ {
  APInt Res = *ਇਹ-RHS;
  ਉਛਾਲ = Res.ugt(*ਇਹ);
  ਵਾਪਸ Res;
}

APInt APInt::sdiv_ov(ਅਚਲ APInt &RHS, ਤਾਰਕਿਕ &ਉਛਾਲ) ਅਚਲ {
  // MININT/-1  -->  overflow.
  ਉਛਾਲ = is_Min_Signed_ਮੁੱਲ() && RHS.isAllOnes();
  ਵਾਪਸ sdiv(RHS);
}

APInt APInt::smul_ov(ਅਚਲ APInt &RHS, ਤਾਰਕਿਕ &ਉਛਾਲ) ਅਚਲ {
  APInt Res = *ਇਹ * RHS;

  ਜੇ (RHS != 0)
    ਉਛਾਲ = Res.sdiv(RHS) != *ਇਹ ||
               (is_Min_Signed_ਮੁੱਲ() && RHS.isAllOnes());
  ਨਹੀਂਤਾਂ
    ਉਛਾਲ = ਝੂਠ;
  ਵਾਪਸ Res;
}

APInt APInt::umul_ov(ਅਚਲ APInt &RHS, ਤਾਰਕਿਕ &ਉਛਾਲ) ਅਚਲ {
  ਜੇ (countl_zero() + RHS.countl_zero() + 2 <= ਦੋਆਧਾਰੀ_ਅੰਕ_ਚੌੜਾਈ_2) {
    ਉਛਾਲ = ਸੱਚ;
    ਵਾਪਸ *ਇਹ * RHS;
  }

  APInt Res = lshr(1) * RHS;
  ਉਛਾਲ = Res.isNegative();
  Res <<= 1;
  ਜੇ ((*ਇਹ)[0]) {
    Res += RHS;
    ਜੇ (Res.ult(RHS))
      ਉਛਾਲ = ਸੱਚ;
  }
  ਵਾਪਸ Res;
}

APInt APInt::sshl_ov(ਅਚਲ APInt &ShAmt, ਤਾਰਕਿਕ &ਉਛਾਲ) ਅਚਲ {
  ਵਾਪਸ sshl_ov(ShAmt.get_Limited_ਮੁੱਲ(get_ਦੋਆਧਾਰੀ_ਅੰਕ_ਚੌੜਾਈ()), ਉਛਾਲ);
}

APInt APInt::sshl_ov(ਬਿਨਾਨਿਸ਼ਾਨੀ ShAmt, ਤਾਰਕਿਕ &ਉਛਾਲ) ਅਚਲ {
  ਉਛਾਲ = ShAmt >= get_ਦੋਆਧਾਰੀ_ਅੰਕ_ਚੌੜਾਈ();
  ਜੇ (ਉਛਾਲ)
    ਵਾਪਸ APInt(ਦੋਆਧਾਰੀ_ਅੰਕ_ਚੌੜਾਈ_2, 0);

  ਜੇ (isNonNegative()) // Don't allow sign change.
    ਉਛਾਲ = ShAmt >= countl_zero();
  ਨਹੀਂਤਾਂ
    ਉਛਾਲ = ShAmt >= countl_one();

  ਵਾਪਸ *ਇਹ << ShAmt;
}

APInt APInt::ushl_ov(ਅਚਲ APInt &ShAmt, ਤਾਰਕਿਕ &ਉਛਾਲ) ਅਚਲ {
  ਵਾਪਸ ushl_ov(ShAmt.get_Limited_ਮੁੱਲ(get_ਦੋਆਧਾਰੀ_ਅੰਕ_ਚੌੜਾਈ()), ਉਛਾਲ);
}

APInt APInt::ushl_ov(ਬਿਨਾਨਿਸ਼ਾਨੀ ShAmt, ਤਾਰਕਿਕ &ਉਛਾਲ) ਅਚਲ {
  ਉਛਾਲ = ShAmt >= get_ਦੋਆਧਾਰੀ_ਅੰਕ_ਚੌੜਾਈ();
  ਜੇ (ਉਛਾਲ)
    ਵਾਪਸ APInt(ਦੋਆਧਾਰੀ_ਅੰਕ_ਚੌੜਾਈ_2, 0);

  ਉਛਾਲ = ShAmt > countl_zero();

  ਵਾਪਸ *ਇਹ << ShAmt;
}

APInt APInt::sfloordiv_ov(ਅਚਲ APInt &RHS, ਤਾਰਕਿਕ &ਉਛਾਲ) ਅਚਲ {
  APInt quotient = sdiv_ov(RHS, ਉਛਾਲ);
  ਜੇ ((quotient * RHS != *ਇਹ) && (isNegative() != RHS.isNegative()))
    ਵਾਪਸ quotient - 1;
  ਵਾਪਸ quotient;
}

APInt APInt::sadd_sat(ਅਚਲ APInt &RHS) ਅਚਲ {
  ਤਾਰਕਿਕ ਉਛਾਲ;
  APInt Res = sadd_ov(RHS, ਉਛਾਲ);
  ਜੇ (!ਉਛਾਲ)
    ਵਾਪਸ Res;

  ਵਾਪਸ isNegative() ? APInt::get_Signed_Min_ਮੁੱਲ(ਦੋਆਧਾਰੀ_ਅੰਕ_ਚੌੜਾਈ_2)
                      : APInt::get_Signed_Max_ਮੁੱਲ(ਦੋਆਧਾਰੀ_ਅੰਕ_ਚੌੜਾਈ_2);
}

APInt APInt::uadd_sat(ਅਚਲ APInt &RHS) ਅਚਲ {
  ਤਾਰਕਿਕ ਉਛਾਲ;
  APInt Res = uadd_ov(RHS, ਉਛਾਲ);
  ਜੇ (!ਉਛਾਲ)
    ਵਾਪਸ Res;

  ਵਾਪਸ APInt::get_Max_ਮੁੱਲ(ਦੋਆਧਾਰੀ_ਅੰਕ_ਚੌੜਾਈ_2);
}

APInt APInt::ssub_sat(ਅਚਲ APInt &RHS) ਅਚਲ {
  ਤਾਰਕਿਕ ਉਛਾਲ;
  APInt Res = ssub_ov(RHS, ਉਛਾਲ);
  ਜੇ (!ਉਛਾਲ)
    ਵਾਪਸ Res;

  ਵਾਪਸ isNegative() ? APInt::get_Signed_Min_ਮੁੱਲ(ਦੋਆਧਾਰੀ_ਅੰਕ_ਚੌੜਾਈ_2)
                      : APInt::get_Signed_Max_ਮੁੱਲ(ਦੋਆਧਾਰੀ_ਅੰਕ_ਚੌੜਾਈ_2);
}

APInt APInt::usub_sat(ਅਚਲ APInt &RHS) ਅਚਲ {
  ਤਾਰਕਿਕ ਉਛਾਲ;
  APInt Res = usub_ov(RHS, ਉਛਾਲ);
  ਜੇ (!ਉਛਾਲ)
    ਵਾਪਸ Res;

  ਵਾਪਸ APInt(ਦੋਆਧਾਰੀ_ਅੰਕ_ਚੌੜਾਈ_2, 0);
}

APInt APInt::smul_sat(ਅਚਲ APInt &RHS) ਅਚਲ {
  ਤਾਰਕਿਕ ਉਛਾਲ;
  APInt Res = smul_ov(RHS, ਉਛਾਲ);
  ਜੇ (!ਉਛਾਲ)
    ਵਾਪਸ Res;

  // The result is negative if one and only one of inputs is negative.
  ਤਾਰਕਿਕ ResIsNegative = isNegative() ^ RHS.isNegative();

  ਵਾਪਸ ResIsNegative ? APInt::get_Signed_Min_ਮੁੱਲ(ਦੋਆਧਾਰੀ_ਅੰਕ_ਚੌੜਾਈ_2)
                       : APInt::get_Signed_Max_ਮੁੱਲ(ਦੋਆਧਾਰੀ_ਅੰਕ_ਚੌੜਾਈ_2);
}

APInt APInt::umul_sat(ਅਚਲ APInt &RHS) ਅਚਲ {
  ਤਾਰਕਿਕ ਉਛਾਲ;
  APInt Res = umul_ov(RHS, ਉਛਾਲ);
  ਜੇ (!ਉਛਾਲ)
    ਵਾਪਸ Res;

  ਵਾਪਸ APInt::get_Max_ਮੁੱਲ(ਦੋਆਧਾਰੀ_ਅੰਕ_ਚੌੜਾਈ_2);
}

APInt APInt::sshl_sat(ਅਚਲ APInt &RHS) ਅਚਲ {
  ਵਾਪਸ sshl_sat(RHS.get_Limited_ਮੁੱਲ(get_ਦੋਆਧਾਰੀ_ਅੰਕ_ਚੌੜਾਈ()));
}

APInt APInt::sshl_sat(ਬਿਨਾਨਿਸ਼ਾਨੀ RHS) ਅਚਲ {
  ਤਾਰਕਿਕ ਉਛਾਲ;
  APInt Res = sshl_ov(RHS, ਉਛਾਲ);
  ਜੇ (!ਉਛਾਲ)
    ਵਾਪਸ Res;

  ਵਾਪਸ isNegative() ? APInt::get_Signed_Min_ਮੁੱਲ(ਦੋਆਧਾਰੀ_ਅੰਕ_ਚੌੜਾਈ_2)
                      : APInt::get_Signed_Max_ਮੁੱਲ(ਦੋਆਧਾਰੀ_ਅੰਕ_ਚੌੜਾਈ_2);
}

APInt APInt::ushl_sat(ਅਚਲ APInt &RHS) ਅਚਲ {
  ਵਾਪਸ ushl_sat(RHS.get_Limited_ਮੁੱਲ(get_ਦੋਆਧਾਰੀ_ਅੰਕ_ਚੌੜਾਈ()));
}

APInt APInt::ushl_sat(ਬਿਨਾਨਿਸ਼ਾਨੀ RHS) ਅਚਲ {
  ਤਾਰਕਿਕ ਉਛਾਲ;
  APInt Res = ushl_ov(RHS, ਉਛਾਲ);
  ਜੇ (!ਉਛਾਲ)
    ਵਾਪਸ Res;

  ਵਾਪਸ APInt::get_Max_ਮੁੱਲ(ਦੋਆਧਾਰੀ_ਅੰਕ_ਚੌੜਾਈ_2);
}

ਖਾਲੀ APInt::fromString(ਬਿਨਾਨਿਸ਼ਾਨੀ numbits, StringRef str, uint8_t radix) {
  // Check our assumptions here
  ਪ੍ਰਤਿਜ੍ਞਾ(!str.ਖਾਲੀ_ਥਾਵਾਂ() && "Invalid string length");
  ਪ੍ਰਤਿਜ੍ਞਾ((radix == 10 || radix == 8 || radix == 16 || radix == 2 ||
          radix == 36) &&
         "Radix should be 2, 8, 10, 16, or 36!");

  StringRef::iterator p = str.begin();
  size_t slen = str.ਮਾਪ_2();
  ਤਾਰਕਿਕ isNeg = *p == '-';
  ਜੇ (*p == '-' || *p == '+') {
    p++;
    slen--;
    ਪ੍ਰਤਿਜ੍ਞਾ(slen && "String is only a sign, needs a value.");
  }
  ਪ੍ਰਤਿਜ੍ਞਾ((slen <= numbits || radix != 2) && "Insufficient bit width");
  ਪ੍ਰਤਿਜ੍ਞਾ(((slen-1)*3 <= numbits || radix != 8) && "Insufficient bit width");
  ਪ੍ਰਤਿਜ੍ਞਾ(((slen-1)*4 <= numbits || radix != 16) && "Insufficient bit width");
  ਪ੍ਰਤਿਜ੍ਞਾ((((slen-1)*64)/22 <= numbits || radix != 10) &&
         "Insufficient bit width");

  // Allocate memory if needed
  ਜੇ (LLVM_LIKELY(is_Single_ਸ਼ਬਦ()))
    U.VAL = 0;
  ਨਹੀਂਤਾਂ
    U.pVal = get_Cleared_ਸਮ੍ਰਿਤੀ(get_ਅੰਕ_ਸ਼ਬਦ());

  // Figure out if we can shift instead of multiply
  ਬਿਨਾਨਿਸ਼ਾਨੀ ਖਿਸਕਾਓ = (radix == 16 ? 4 : radix == 8 ? 3 : radix == 2 ? 1 : 0);

  // Enter digit traversal loop
  ਲਈ (StringRef::iterator e = str.ਸਿਰਾ(); p != e; ++p) {
    ਬਿਨਾਨਿਸ਼ਾਨੀ digit = getDigit(*p, radix);
    ਪ੍ਰਤਿਜ੍ਞਾ(digit < radix && "Invalid character in digit string");

    // Shift or multiply the value by the radix
    ਜੇ (slen > 1) {
      ਜੇ (ਖਿਸਕਾਓ)
        *ਇਹ <<= ਖਿਸਕਾਓ;
      ਨਹੀਂਤਾਂ
        *ਇਹ *= radix;
    }

    // Add in the digit we just interpreted
    *ਇਹ += digit;
  }
  // If its negative, put it in two's complement form
  ਜੇ (isNeg)
    ਇਹ->negate();
}

ਖਾਲੀ APInt::ਮੰਜ਼ਿਲ_String(Small_ਸਦਿਸ਼_Impl<ਅੱਖਰ> &Str, ਬਿਨਾਨਿਸ਼ਾਨੀ Radix, ਤਾਰਕਿਕ Signed,
                     ਤਾਰਕਿਕ formatAsCLiteral, ਤਾਰਕਿਕ ਉੱਤਲਾ_Case_2,
                     ਤਾਰਕਿਕ ਪਾਓ_Separators) ਅਚਲ {
  ਪ੍ਰਤਿਜ੍ਞਾ((Radix == 10 || Radix == 8 || Radix == 16 || Radix == 2 ||
          Radix == 36) &&
         "Radix should be 2, 8, 10, 16, or 36!");

  ਅਚਲ ਅੱਖਰ *ਅਗੇਤਰ = "";
  ਜੇ (formatAsCLiteral) {
    ਚੋਣ (Radix) {
      ਹਾਲਤ 2:
        // Binary literals are a non-standard extension added in gcc 4.3:
        // http://gcc.gnu.org/onlinedocs/gcc-4.3.0/gcc/Binary-constants.html
        ਅਗੇਤਰ = "0b";
        ਰੋਕੋ;
      ਹਾਲਤ 8:
        ਅਗੇਤਰ = "0";
        ਰੋਕੋ;
      ਹਾਲਤ 10:
        ਰੋਕੋ; // No prefix
      ਹਾਲਤ 16:
        ਅਗੇਤਰ = "0x";
        ਰੋਕੋ;
      ਮੂਲ:
        llvm_unreachable("Invalid radix!");
    }
  }

  // Number of digits in a group between separators.
  ਬਿਨਾਨਿਸ਼ਾਨੀ Grouping = (Radix == 8 || Radix == 10) ? 3 : 4;

  // First, check for a zero value and just short circuit the logic below.
  ਜੇ (isZero()) {
    ਜਦੋਂ (*ਅਗੇਤਰ) {
      Str.ਧੱਕੋ_ਪਿੱਠ(*ਅਗੇਤਰ);
      ++ਅਗੇਤਰ;
    };
    Str.ਧੱਕੋ_ਪਿੱਠ('0');
    ਵਾਪਸ;
  }

  ਸਥਿਰ ਅਚਲ ਅੱਖਰ BothDigits[] = "0123456789abcdefghijklmnopqrstuvwxyz"
                                   "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ";
  ਅਚਲ ਅੱਖਰ *Digits = BothDigits + (ਉੱਤਲਾ_Case_2 ? 36 : 0);

  ਜੇ (LLVM_LIKELY(is_Single_ਸ਼ਬਦ())) {
    ਅੱਖਰ ਬਫਰ[65];
    ਅੱਖਰ *BufPtr = std::ਸਿਰਾ(ਬਫਰ);

    uint64_t N;
    ਜੇ (!Signed) {
      N = get_ZExt_ਮੁੱਲ();
    } ਨਹੀਂਤਾਂ {
      int64_t I = get_SExt_ਮੁੱਲ();
      ਜੇ (I >= 0) {
        N = I;
      } ਨਹੀਂਤਾਂ {
        Str.ਧੱਕੋ_ਪਿੱਠ('-');
        N = -(uint64_t)I;
      }
    }

    ਜਦੋਂ (*ਅਗੇਤਰ) {
      Str.ਧੱਕੋ_ਪਿੱਠ(*ਅਗੇਤਰ);
      ++ਅਗੇਤਰ;
    };

    ਪੂਰਨ Pos = 0;
    ਜਦੋਂ (N) {
      ਜੇ (ਪਾਓ_Separators && Pos % Grouping == 0 && Pos > 0)
        *--BufPtr = '\'';
      *--BufPtr = Digits[N % Radix];
      N /= Radix;
      Pos++;
    }
    Str.append(BufPtr, std::ਸਿਰਾ(ਬਫਰ));
    ਵਾਪਸ;
  }

  APInt Tmp(*ਇਹ);

  ਜੇ (Signed && isNegative()) {
    // They want to print the signed version and it is a negative value
    // Flip the bits and add one to turn it into the equivalent positive
    // value and put a '-' in the result.
    Tmp.negate();
    Str.ਧੱਕੋ_ਪਿੱਠ('-');
  }

  ਜਦੋਂ (*ਅਗੇਤਰ) {
    Str.ਧੱਕੋ_ਪਿੱਠ(*ਅਗੇਤਰ);
    ++ਅਗੇਤਰ;
  }

  // We insert the digits backward, then reverse them to get the right order.
  ਬਿਨਾਨਿਸ਼ਾਨੀ ਸ਼ੁਰੂ_Dig = Str.ਮਾਪ_2();

  // For the 2, 8 and 16 bit cases, we can just shift instead of divide
  // because the number of bits per digit (1, 3 and 4 respectively) divides
  // equally.  We just shift until the value is zero.
  ਜੇ (Radix == 2 || Radix == 8 || Radix == 16) {
    // Just shift tmp right for each digit width until it becomes zero
    ਬਿਨਾਨਿਸ਼ਾਨੀ ਖਿਸਕਾਓ_Amt_2 = (Radix == 16 ? 4 : (Radix == 8 ? 3 : 1));
    ਬਿਨਾਨਿਸ਼ਾਨੀ ਨਕਾਬ_Amt = Radix - 1;

    ਪੂਰਨ Pos = 0;
    ਜਦੋਂ (Tmp.get_Bool_ਮੁੱਲ()) {
      ਬਿਨਾਨਿਸ਼ਾਨੀ Digit = ਬਿਨਾਨਿਸ਼ਾਨੀ(Tmp.get_Raw_ਅੰਕੜੇ()[0]) & ਨਕਾਬ_Amt;
      ਜੇ (ਪਾਓ_Separators && Pos % Grouping == 0 && Pos > 0)
        Str.ਧੱਕੋ_ਪਿੱਠ('\'');

      Str.ਧੱਕੋ_ਪਿੱਠ(Digits[Digit]);
      Tmp.lshr_ਵਿੱਚ_ਰੱਖੋ(ਖਿਸਕਾਓ_Amt_2);
      Pos++;
    }
  } ਨਹੀਂਤਾਂ {
    ਪੂਰਨ Pos = 0;
    ਜਦੋਂ (Tmp.get_Bool_ਮੁੱਲ()) {
      uint64_t Digit;
      udivrem(Tmp, Radix, Tmp, Digit);
      ਪ੍ਰਤਿਜ੍ਞਾ(Digit < Radix && "divide failed");
      ਜੇ (ਪਾਓ_Separators && Pos % Grouping == 0 && Pos > 0)
        Str.ਧੱਕੋ_ਪਿੱਠ('\'');

      Str.ਧੱਕੋ_ਪਿੱਠ(Digits[Digit]);
      Pos++;
    }
  }

  // Reverse the digits before returning.
  std::ਉਲਟ(Str.begin()+ਸ਼ੁਰੂ_Dig, Str.ਸਿਰਾ());
}

#ਜੇ !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)
LLVM_DUMP_ਵਿਧੀ ਖਾਲੀ APInt::dump() ਅਚਲ {
  SmallString<40> S, U;
  ਇਹ->ਮੰਜ਼ਿਲ_String_Unsigned(U);
  ਇਹ->ਮੰਜ਼ਿਲ_String_Signed(S);
  dbgs() << "APInt(" << ਦੋਆਧਾਰੀ_ਅੰਕ_ਚੌੜਾਈ_2 << "b, "
         << U << "u " << S << "s)\n";
}
#ਜੇ_ਅੰਤ

ਖਾਲੀ APInt::print(raw_ostream &OS, ਤਾਰਕਿਕ isSigned) ਅਚਲ {
  SmallString<40> S;
  ਇਹ->ਮੰਜ਼ਿਲ_String(S, 10, isSigned, /* formatAsCLiteral = */ਝੂਠ);
  OS << S;
}

// This implements a variety of operations on a representation of
// arbitrary precision, two's-complement, bignum integer values.

// Assumed by lowHalf, highHalf, partMSB and partLSB.  A fairly safe
// and unrestricting assumption.
static_assert(APInt::APINT_ਬਿੱਟ_ਪ੍ਰਤੀ_ਸ਼ਬਦ % 2 == 0,
              "Part width must be divisible by 2!");

// Returns the integer part with the least significant BITS set.
// BITS cannot be zero.
ਸਥਿਰ ਸਤਰਅੰਦਰ APInt::ਸ਼ਬਦ_Type ਨੀਵਾਂ_ਦੋਆਧਾਰੀ_ਅੰਕ_ਨਕਾਬ(ਬਿਨਾਨਿਸ਼ਾਨੀ ਬਿੱਟ) {
  ਪ੍ਰਤਿਜ੍ਞਾ(ਬਿੱਟ != 0 && ਬਿੱਟ <= APInt::APINT_ਬਿੱਟ_ਪ੍ਰਤੀ_ਸ਼ਬਦ);
  ਵਾਪਸ ~(APInt::ਸ਼ਬਦ_Type) 0 >> (APInt::APINT_ਬਿੱਟ_ਪ੍ਰਤੀ_ਸ਼ਬਦ - ਬਿੱਟ);
}

/// Returns the value of the lower half of PART.
ਸਥਿਰ ਸਤਰਅੰਦਰ APInt::ਸ਼ਬਦ_Type ਨੀਵਾਂ_Half(APInt::ਸ਼ਬਦ_Type part) {
  ਵਾਪਸ part & ਨੀਵਾਂ_ਦੋਆਧਾਰੀ_ਅੰਕ_ਨਕਾਬ(APInt::APINT_ਬਿੱਟ_ਪ੍ਰਤੀ_ਸ਼ਬਦ / 2);
}

/// Returns the value of the upper half of PART.
ਸਥਿਰ ਸਤਰਅੰਦਰ APInt::ਸ਼ਬਦ_Type ਉੱਚਾ_Half(APInt::ਸ਼ਬਦ_Type part) {
  ਵਾਪਸ part >> (APInt::APINT_ਬਿੱਟ_ਪ੍ਰਤੀ_ਸ਼ਬਦ / 2);
}

/// Sets the least significant part of a bignum to the input value, and zeroes
/// out higher parts.
ਖਾਲੀ APInt::tc_ਸੈੱਟ(ਸ਼ਬਦ_Type *dst, ਸ਼ਬਦ_Type part, ਬਿਨਾਨਿਸ਼ਾਨੀ parts) {
  ਪ੍ਰਤਿਜ੍ਞਾ(parts > 0);
  dst[0] = part;
  ਲਈ (ਬਿਨਾਨਿਸ਼ਾਨੀ i = 1; i < parts; i++)
    dst[i] = 0;
}

/// Assign one bignum to another.
ਖਾਲੀ APInt::tc_ਸੌਂਪੋ(ਸ਼ਬਦ_Type *dst, ਅਚਲ ਸ਼ਬਦ_Type *src, ਬਿਨਾਨਿਸ਼ਾਨੀ parts) {
  ਲਈ (ਬਿਨਾਨਿਸ਼ਾਨੀ i = 0; i < parts; i++)
    dst[i] = src[i];
}

/// Returns true if a bignum is zero, false otherwise.
ਤਾਰਕਿਕ APInt::tcIsZero(ਅਚਲ ਸ਼ਬਦ_Type *src, ਬਿਨਾਨਿਸ਼ਾਨੀ parts) {
  ਲਈ (ਬਿਨਾਨਿਸ਼ਾਨੀ i = 0; i < parts; i++)
    ਜੇ (src[i])
      ਵਾਪਸ ਝੂਠ;

  ਵਾਪਸ ਸੱਚ;
}

/// Extract the given bit of a bignum; returns 0 or 1.
ਪੂਰਨ APInt::tc_ਕੱਢੋ_ਦੋਆਧਾਰੀ_ਅੰਕ(ਅਚਲ ਸ਼ਬਦ_Type *parts, ਬਿਨਾਨਿਸ਼ਾਨੀ ਦੋਆਧਾਰੀ_ਅੰਕ) {
  ਵਾਪਸ (parts[which_ਸ਼ਬਦ(ਦੋਆਧਾਰੀ_ਅੰਕ)] & ਨਕਾਬ_ਦੋਆਧਾਰੀ_ਅੰਕ(ਦੋਆਧਾਰੀ_ਅੰਕ)) != 0;
}

/// Set the given bit of a bignum.
ਖਾਲੀ APInt::tc_ਸੈੱਟ_ਦੋਆਧਾਰੀ_ਅੰਕ(ਸ਼ਬਦ_Type *parts, ਬਿਨਾਨਿਸ਼ਾਨੀ ਦੋਆਧਾਰੀ_ਅੰਕ) {
  parts[which_ਸ਼ਬਦ(ਦੋਆਧਾਰੀ_ਅੰਕ)] |= ਨਕਾਬ_ਦੋਆਧਾਰੀ_ਅੰਕ(ਦੋਆਧਾਰੀ_ਅੰਕ);
}

/// Clears the given bit of a bignum.
ਖਾਲੀ APInt::tc_Clear_ਦੋਆਧਾਰੀ_ਅੰਕ(ਸ਼ਬਦ_Type *parts, ਬਿਨਾਨਿਸ਼ਾਨੀ ਦੋਆਧਾਰੀ_ਅੰਕ) {
  parts[which_ਸ਼ਬਦ(ਦੋਆਧਾਰੀ_ਅੰਕ)] &= ~ਨਕਾਬ_ਦੋਆਧਾਰੀ_ਅੰਕ(ਦੋਆਧਾਰੀ_ਅੰਕ);
}

/// Returns the bit number of the least significant set bit of a number.  If the
/// input number has no bits set UINT_MAX is returned.
ਬਿਨਾਨਿਸ਼ਾਨੀ APInt::tcLSB(ਅਚਲ ਸ਼ਬਦ_Type *parts, ਬਿਨਾਨਿਸ਼ਾਨੀ n) {
  ਲਈ (ਬਿਨਾਨਿਸ਼ਾਨੀ i = 0; i < n; i++) {
    ਜੇ (parts[i] != 0) {
      ਬਿਨਾਨਿਸ਼ਾਨੀ lsb = llvm::countr_zero(parts[i]);
      ਵਾਪਸ lsb + i * APINT_ਬਿੱਟ_ਪ੍ਰਤੀ_ਸ਼ਬਦ;
    }
  }

  ਵਾਪਸ UINT_MAX;
}

/// Returns the bit number of the most significant set bit of a number.
/// If the input number has no bits set UINT_MAX is returned.
ਬਿਨਾਨਿਸ਼ਾਨੀ APInt::tcMSB(ਅਚਲ ਸ਼ਬਦ_Type *parts, ਬਿਨਾਨਿਸ਼ਾਨੀ n) {
  ਕਰੋ {
    --n;

    ਜੇ (parts[n] != 0) {
      static_assert(ਆਕਾਰ(parts[n]) <= ਆਕਾਰ(uint64_t));
      ਬਿਨਾਨਿਸ਼ਾਨੀ msb = llvm::Log2_64(parts[n]);

      ਵਾਪਸ msb + n * APINT_ਬਿੱਟ_ਪ੍ਰਤੀ_ਸ਼ਬਦ;
    }
  } ਜਦੋਂ (n);

  ਵਾਪਸ UINT_MAX;
}

/// Copy the bit vector of width srcBITS from SRC, starting at bit srcLSB, to
/// DST, of dstCOUNT parts, such that the bit srcLSB becomes the least
/// significant bit of DST.  All high bits above srcBITS in DST are zero-filled.
/// */
ਖਾਲੀ
APInt::tc_ਕੱਢੋ(ਸ਼ਬਦ_Type *dst, ਬਿਨਾਨਿਸ਼ਾਨੀ dst_ਗਿਣਤੀ, ਅਚਲ ਸ਼ਬਦ_Type *src,
                 ਬਿਨਾਨਿਸ਼ਾਨੀ src_ਬਿੱਟ, ਬਿਨਾਨਿਸ਼ਾਨੀ srcLSB) {
  ਬਿਨਾਨਿਸ਼ਾਨੀ dstParts = (src_ਬਿੱਟ + APINT_ਬਿੱਟ_ਪ੍ਰਤੀ_ਸ਼ਬਦ - 1) / APINT_ਬਿੱਟ_ਪ੍ਰਤੀ_ਸ਼ਬਦ;
  ਪ੍ਰਤਿਜ੍ਞਾ(dstParts <= dst_ਗਿਣਤੀ);

  ਬਿਨਾਨਿਸ਼ਾਨੀ ਪਹਿਲਾ_Src_Part = srcLSB / APINT_ਬਿੱਟ_ਪ੍ਰਤੀ_ਸ਼ਬਦ;
  tc_ਸੌਂਪੋ(dst, src + ਪਹਿਲਾ_Src_Part, dstParts);

  ਬਿਨਾਨਿਸ਼ਾਨੀ ਖਿਸਕਾਓ = srcLSB % APINT_ਬਿੱਟ_ਪ੍ਰਤੀ_ਸ਼ਬਦ;
  tc_ਖਿਸਕਾਓ_ਸੱਜਾ(dst, dstParts, ਖਿਸਕਾਓ);

  // We now have (dstParts * APINT_BITS_PER_WORD - shift) bits from SRC
  // in DST.  If this is less that srcBits, append the rest, else
  // clear the high bits.
  ਬਿਨਾਨਿਸ਼ਾਨੀ n = dstParts * APINT_ਬਿੱਟ_ਪ੍ਰਤੀ_ਸ਼ਬਦ - ਖਿਸਕਾਓ;
  ਜੇ (n < src_ਬਿੱਟ) {
    ਸ਼ਬਦ_Type ਨਕਾਬ_2 = ਨੀਵਾਂ_ਦੋਆਧਾਰੀ_ਅੰਕ_ਨਕਾਬ (src_ਬਿੱਟ - n);
    dst[dstParts - 1] |= ((src[ਪਹਿਲਾ_Src_Part + dstParts] & ਨਕਾਬ_2)
                          << n % APINT_ਬਿੱਟ_ਪ੍ਰਤੀ_ਸ਼ਬਦ);
  } ਨਹੀਂਤਾਂ ਜੇ (n > src_ਬਿੱਟ) {
    ਜੇ (src_ਬਿੱਟ % APINT_ਬਿੱਟ_ਪ੍ਰਤੀ_ਸ਼ਬਦ)
      dst[dstParts - 1] &= ਨੀਵਾਂ_ਦੋਆਧਾਰੀ_ਅੰਕ_ਨਕਾਬ (src_ਬਿੱਟ % APINT_ਬਿੱਟ_ਪ੍ਰਤੀ_ਸ਼ਬਦ);
  }

  // Clear high parts.
  ਜਦੋਂ (dstParts < dst_ਗਿਣਤੀ)
    dst[dstParts++] = 0;
}

//// DST += RHS + C where C is zero or one.  Returns the carry flag.
APInt::ਸ਼ਬਦ_Type APInt::tc_ਜੋੜੋ(ਸ਼ਬਦ_Type *dst, ਅਚਲ ਸ਼ਬਦ_Type *rhs,
                             ਸ਼ਬਦ_Type c, ਬਿਨਾਨਿਸ਼ਾਨੀ parts) {
  ਪ੍ਰਤਿਜ੍ਞਾ(c <= 1);

  ਲਈ (ਬਿਨਾਨਿਸ਼ਾਨੀ i = 0; i < parts; i++) {
    ਸ਼ਬਦ_Type l = dst[i];
    ਜੇ (c) {
      dst[i] += rhs[i] + 1;
      c = (dst[i] <= l);
    } ਨਹੀਂਤਾਂ {
      dst[i] += rhs[i];
      c = (dst[i] < l);
    }
  }

  ਵਾਪਸ c;
}

/// This function adds a single "word" integer, src, to the multiple
/// "word" integer array, dst[]. dst[] is modified to reflect the addition and
/// 1 is returned if there is a carry out, otherwise 0 is returned.
/// @returns the carry of the addition.
APInt::ਸ਼ਬਦ_Type APInt::tc_ਜੋੜੋ_Part(ਸ਼ਬਦ_Type *dst, ਸ਼ਬਦ_Type src,
                                 ਬਿਨਾਨਿਸ਼ਾਨੀ parts) {
  ਲਈ (ਬਿਨਾਨਿਸ਼ਾਨੀ i = 0; i < parts; ++i) {
    dst[i] += src;
    ਜੇ (dst[i] >= src)
      ਵਾਪਸ 0; // No need to carry so exit early.
    src = 1; // Carry one to next digit.
  }

  ਵਾਪਸ 1;
}

/// DST -= RHS + C where C is zero or one.  Returns the carry flag.
APInt::ਸ਼ਬਦ_Type APInt::tc_ਘਟਾਓ(ਸ਼ਬਦ_Type *dst, ਅਚਲ ਸ਼ਬਦ_Type *rhs,
                                  ਸ਼ਬਦ_Type c, ਬਿਨਾਨਿਸ਼ਾਨੀ parts) {
  ਪ੍ਰਤਿਜ੍ਞਾ(c <= 1);

  ਲਈ (ਬਿਨਾਨਿਸ਼ਾਨੀ i = 0; i < parts; i++) {
    ਸ਼ਬਦ_Type l = dst[i];
    ਜੇ (c) {
      dst[i] -= rhs[i] + 1;
      c = (dst[i] >= l);
    } ਨਹੀਂਤਾਂ {
      dst[i] -= rhs[i];
      c = (dst[i] > l);
    }
  }

  ਵਾਪਸ c;
}

/// This function subtracts a single "word" (64-bit word), src, from
/// the multi-word integer array, dst[], propagating the borrowed 1 value until
/// no further borrowing is needed or it runs out of "words" in dst.  The result
/// is 1 if "borrowing" exhausted the digits in dst, or 0 if dst was not
/// exhausted. In other words, if src > dst then this function returns 1,
/// otherwise 0.
/// @returns the borrow out of the subtraction
APInt::ਸ਼ਬਦ_Type APInt::tc_ਘਟਾਓ_Part(ਸ਼ਬਦ_Type *dst, ਸ਼ਬਦ_Type src,
                                      ਬਿਨਾਨਿਸ਼ਾਨੀ parts) {
  ਲਈ (ਬਿਨਾਨਿਸ਼ਾਨੀ i = 0; i < parts; ++i) {
    ਸ਼ਬਦ_Type Dst = dst[i];
    dst[i] -= src;
    ਜੇ (src <= Dst)
      ਵਾਪਸ 0; // No need to borrow so exit early.
    src = 1; // We have to "borrow 1" from next "word"
  }

  ਵਾਪਸ 1;
}

/// Negate a bignum in-place.
ਖਾਲੀ APInt::tcNegate(ਸ਼ਬਦ_Type *dst, ਬਿਨਾਨਿਸ਼ਾਨੀ parts) {
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
ਪੂਰਨ APInt::tcMultiplyPart(ਸ਼ਬਦ_Type *dst, ਅਚਲ ਸ਼ਬਦ_Type *src,
                          ਸ਼ਬਦ_Type multiplier, ਸ਼ਬਦ_Type ਹਾਸਲ,
                          ਬਿਨਾਨਿਸ਼ਾਨੀ srcParts, ਬਿਨਾਨਿਸ਼ਾਨੀ dstParts,
                          ਤਾਰਕਿਕ ਜੋੜੋ) {
  // Otherwise our writes of DST kill our later reads of SRC.
  ਪ੍ਰਤਿਜ੍ਞਾ(dst <= src || dst >= src + srcParts);
  ਪ੍ਰਤਿਜ੍ਞਾ(dstParts <= srcParts + 1);

  // N loops; minimum of dstParts and srcParts.
  ਬਿਨਾਨਿਸ਼ਾਨੀ n = std::min(dstParts, srcParts);

  ਲਈ (ਬਿਨਾਨਿਸ਼ਾਨੀ i = 0; i < n; i++) {
    // [LOW, HIGH] = MULTIPLIER * SRC[i] + DST[i] + CARRY.
    // This cannot overflow, because:
    //   (n - 1) * (n - 1) + 2 (n - 1) = (n - 1) * (n + 1)
    // which is less than n^2.
    ਸ਼ਬਦ_Type srcPart = src[i];
    ਸ਼ਬਦ_Type ਨੀਵਾਂ, mid, ਉੱਚਾ;
    ਜੇ (multiplier == 0 || srcPart == 0) {
      ਨੀਵਾਂ = ਹਾਸਲ;
      ਉੱਚਾ = 0;
    } ਨਹੀਂਤਾਂ {
      ਨੀਵਾਂ = ਨੀਵਾਂ_Half(srcPart) * ਨੀਵਾਂ_Half(multiplier);
      ਉੱਚਾ = ਉੱਚਾ_Half(srcPart) * ਉੱਚਾ_Half(multiplier);

      mid = ਨੀਵਾਂ_Half(srcPart) * ਉੱਚਾ_Half(multiplier);
      ਉੱਚਾ += ਉੱਚਾ_Half(mid);
      mid <<= APINT_ਬਿੱਟ_ਪ੍ਰਤੀ_ਸ਼ਬਦ / 2;
      ਜੇ (ਨੀਵਾਂ + mid < ਨੀਵਾਂ)
        ਉੱਚਾ++;
      ਨੀਵਾਂ += mid;

      mid = ਉੱਚਾ_Half(srcPart) * ਨੀਵਾਂ_Half(multiplier);
      ਉੱਚਾ += ਉੱਚਾ_Half(mid);
      mid <<= APINT_ਬਿੱਟ_ਪ੍ਰਤੀ_ਸ਼ਬਦ / 2;
      ਜੇ (ਨੀਵਾਂ + mid < ਨੀਵਾਂ)
        ਉੱਚਾ++;
      ਨੀਵਾਂ += mid;

      // Now add carry.
      ਜੇ (ਨੀਵਾਂ + ਹਾਸਲ < ਨੀਵਾਂ)
        ਉੱਚਾ++;
      ਨੀਵਾਂ += ਹਾਸਲ;
    }

    ਜੇ (ਜੋੜੋ) {
      // And now DST[i], and store the new low part there.
      ਜੇ (ਨੀਵਾਂ + dst[i] < ਨੀਵਾਂ)
        ਉੱਚਾ++;
      dst[i] += ਨੀਵਾਂ;
    } ਨਹੀਂਤਾਂ {
      dst[i] = ਨੀਵਾਂ;
    }

    ਹਾਸਲ = ਉੱਚਾ;
  }

  ਜੇ (srcParts < dstParts) {
    // Full multiplication, there is no overflow.
    ਪ੍ਰਤਿਜ੍ਞਾ(srcParts + 1 == dstParts);
    dst[srcParts] = ਹਾਸਲ;
    ਵਾਪਸ 0;
  }

  // We overflowed if there is carry.
  ਜੇ (ਹਾਸਲ)
    ਵਾਪਸ 1;

  // We would overflow if any significant unwritten parts would be
  // non-zero.  This is true if any remaining src parts are non-zero
  // and the multiplier is non-zero.
  ਜੇ (multiplier)
    ਲਈ (ਬਿਨਾਨਿਸ਼ਾਨੀ i = dstParts; i < srcParts; i++)
      ਜੇ (src[i])
        ਵਾਪਸ 1;

  // We fitted in the narrow destination.
  ਵਾਪਸ 0;
}

/// DST = LHS * RHS, where DST has the same width as the operands and
/// is filled with the least significant parts of the result.  Returns
/// one if overflow occurred, otherwise zero.  DST must be disjoint
/// from both operands.
ਪੂਰਨ APInt::tcMultiply(ਸ਼ਬਦ_Type *dst, ਅਚਲ ਸ਼ਬਦ_Type *lhs,
                      ਅਚਲ ਸ਼ਬਦ_Type *rhs, ਬਿਨਾਨਿਸ਼ਾਨੀ parts) {
  ਪ੍ਰਤਿਜ੍ਞਾ(dst != lhs && dst != rhs);

  ਪੂਰਨ ਉਛਾਲ_2 = 0;

  ਲਈ (ਬਿਨਾਨਿਸ਼ਾਨੀ i = 0; i < parts; i++) {
    // Don't accumulate on the first iteration so we don't need to initalize
    // dst to 0.
    ਉਛਾਲ_2 |=
        tcMultiplyPart(&dst[i], lhs, rhs[i], 0, parts, parts - i, i != 0);
  }

  ਵਾਪਸ ਉਛਾਲ_2;
}

/// DST = LHS * RHS, where DST has width the sum of the widths of the
/// operands. No overflow occurs. DST must be disjoint from both operands.
ਖਾਲੀ APInt::tc_ਭਰਿਆ_Multiply(ਸ਼ਬਦ_Type *dst, ਅਚਲ ਸ਼ਬਦ_Type *lhs,
                           ਅਚਲ ਸ਼ਬਦ_Type *rhs, ਬਿਨਾਨਿਸ਼ਾਨੀ lhsParts,
                           ਬਿਨਾਨਿਸ਼ਾਨੀ rhsParts) {
  // Put the narrower number on the LHS for less loops below.
  ਜੇ (lhsParts > rhsParts)
    ਵਾਪਸ tc_ਭਰਿਆ_Multiply (dst, rhs, lhs, rhsParts, lhsParts);

  ਪ੍ਰਤਿਜ੍ਞਾ(dst != lhs && dst != rhs);

  ਲਈ (ਬਿਨਾਨਿਸ਼ਾਨੀ i = 0; i < lhsParts; i++) {
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
ਪੂਰਨ APInt::tcDivide(ਸ਼ਬਦ_Type *lhs, ਅਚਲ ਸ਼ਬਦ_Type *rhs,
                    ਸ਼ਬਦ_Type *remainder, ਸ਼ਬਦ_Type *srhs,
                    ਬਿਨਾਨਿਸ਼ਾਨੀ parts) {
  ਪ੍ਰਤਿਜ੍ਞਾ(lhs != remainder && lhs != srhs && remainder != srhs);

  ਬਿਨਾਨਿਸ਼ਾਨੀ ਖਿਸਕਾਓ_ਗਿਣਤੀ = tcMSB(rhs, parts) + 1;
  ਜੇ (ਖਿਸਕਾਓ_ਗਿਣਤੀ == 0)
    ਵਾਪਸ ਸੱਚ;

  ਖਿਸਕਾਓ_ਗਿਣਤੀ = parts * APINT_ਬਿੱਟ_ਪ੍ਰਤੀ_ਸ਼ਬਦ - ਖਿਸਕਾਓ_ਗਿਣਤੀ;
  ਬਿਨਾਨਿਸ਼ਾਨੀ n = ਖਿਸਕਾਓ_ਗਿਣਤੀ / APINT_ਬਿੱਟ_ਪ੍ਰਤੀ_ਸ਼ਬਦ;
  ਸ਼ਬਦ_Type ਨਕਾਬ_2 = (ਸ਼ਬਦ_Type) 1 << (ਖਿਸਕਾਓ_ਗਿਣਤੀ % APINT_ਬਿੱਟ_ਪ੍ਰਤੀ_ਸ਼ਬਦ);

  tc_ਸੌਂਪੋ(srhs, rhs, parts);
  tc_ਖਿਸਕਾਓ_ਖੱਬਾ(srhs, parts, ਖਿਸਕਾਓ_ਗਿਣਤੀ);
  tc_ਸੌਂਪੋ(remainder, lhs, parts);
  tc_ਸੈੱਟ(lhs, 0, parts);

  // Loop, subtracting SRHS if REMAINDER is greater and adding that to the
  // total.
  ਲਈ (;;) {
    ਪੂਰਨ compare = tcCompare(remainder, srhs, parts);
    ਜੇ (compare >= 0) {
      tc_ਘਟਾਓ(remainder, srhs, 0, parts);
      lhs[n] |= ਨਕਾਬ_2;
    }

    ਜੇ (ਖਿਸਕਾਓ_ਗਿਣਤੀ == 0)
      ਰੋਕੋ;
    ਖਿਸਕਾਓ_ਗਿਣਤੀ--;
    tc_ਖਿਸਕਾਓ_ਸੱਜਾ(srhs, parts, 1);
    ਜੇ ((ਨਕਾਬ_2 >>= 1) == 0) {
      ਨਕਾਬ_2 = (ਸ਼ਬਦ_Type) 1 << (APINT_ਬਿੱਟ_ਪ੍ਰਤੀ_ਸ਼ਬਦ - 1);
      n--;
    }
  }

  ਵਾਪਸ ਝੂਠ;
}

/// Shift a bignum left Count bits in-place. Shifted in bits are zero. There are
/// no restrictions on Count.
ਖਾਲੀ APInt::tc_ਖਿਸਕਾਓ_ਖੱਬਾ(ਸ਼ਬਦ_Type *Dst, ਬਿਨਾਨਿਸ਼ਾਨੀ ਸ਼ਬਦ_4, ਬਿਨਾਨਿਸ਼ਾਨੀ ਗਿਣਤੀ_3) {
  // Don't bother performing a no-op shift.
  ਜੇ (!ਗਿਣਤੀ_3)
    ਵਾਪਸ;

  // WordShift is the inter-part shift; BitShift is the intra-part shift.
  ਬਿਨਾਨਿਸ਼ਾਨੀ ਸ਼ਬਦ_ਖਿਸਕਾਓ = std::min(ਗਿਣਤੀ_3 / APINT_ਬਿੱਟ_ਪ੍ਰਤੀ_ਸ਼ਬਦ, ਸ਼ਬਦ_4);
  ਬਿਨਾਨਿਸ਼ਾਨੀ ਦੋਆਧਾਰੀ_ਅੰਕ_ਖਿਸਕਾਓ = ਗਿਣਤੀ_3 % APINT_ਬਿੱਟ_ਪ੍ਰਤੀ_ਸ਼ਬਦ;

  // Fastpath for moving by whole words.
  ਜੇ (ਦੋਆਧਾਰੀ_ਅੰਕ_ਖਿਸਕਾਓ == 0) {
    std::memmove(Dst + ਸ਼ਬਦ_ਖਿਸਕਾਓ, Dst, (ਸ਼ਬਦ_4 - ਸ਼ਬਦ_ਖਿਸਕਾਓ) * APINT_ਸ਼ਬਦ_ਮਾਪ);
  } ਨਹੀਂਤਾਂ {
    ਜਦੋਂ (ਸ਼ਬਦ_4-- > ਸ਼ਬਦ_ਖਿਸਕਾਓ) {
      Dst[ਸ਼ਬਦ_4] = Dst[ਸ਼ਬਦ_4 - ਸ਼ਬਦ_ਖਿਸਕਾਓ] << ਦੋਆਧਾਰੀ_ਅੰਕ_ਖਿਸਕਾਓ;
      ਜੇ (ਸ਼ਬਦ_4 > ਸ਼ਬਦ_ਖਿਸਕਾਓ)
        Dst[ਸ਼ਬਦ_4] |=
          Dst[ਸ਼ਬਦ_4 - ਸ਼ਬਦ_ਖਿਸਕਾਓ - 1] >> (APINT_ਬਿੱਟ_ਪ੍ਰਤੀ_ਸ਼ਬਦ - ਦੋਆਧਾਰੀ_ਅੰਕ_ਖਿਸਕਾਓ);
    }
  }

  // Fill in the remainder with 0s.
  std::memset(Dst, 0, ਸ਼ਬਦ_ਖਿਸਕਾਓ * APINT_ਸ਼ਬਦ_ਮਾਪ);
}

/// Shift a bignum right Count bits in-place. Shifted in bits are zero. There
/// are no restrictions on Count.
ਖਾਲੀ APInt::tc_ਖਿਸਕਾਓ_ਸੱਜਾ(ਸ਼ਬਦ_Type *Dst, ਬਿਨਾਨਿਸ਼ਾਨੀ ਸ਼ਬਦ_4, ਬਿਨਾਨਿਸ਼ਾਨੀ ਗਿਣਤੀ_3) {
  // Don't bother performing a no-op shift.
  ਜੇ (!ਗਿਣਤੀ_3)
    ਵਾਪਸ;

  // WordShift is the inter-part shift; BitShift is the intra-part shift.
  ਬਿਨਾਨਿਸ਼ਾਨੀ ਸ਼ਬਦ_ਖਿਸਕਾਓ = std::min(ਗਿਣਤੀ_3 / APINT_ਬਿੱਟ_ਪ੍ਰਤੀ_ਸ਼ਬਦ, ਸ਼ਬਦ_4);
  ਬਿਨਾਨਿਸ਼ਾਨੀ ਦੋਆਧਾਰੀ_ਅੰਕ_ਖਿਸਕਾਓ = ਗਿਣਤੀ_3 % APINT_ਬਿੱਟ_ਪ੍ਰਤੀ_ਸ਼ਬਦ;

  ਬਿਨਾਨਿਸ਼ਾਨੀ ਸ਼ਬਦ_ਮੰਜ਼ਿਲ_Move = ਸ਼ਬਦ_4 - ਸ਼ਬਦ_ਖਿਸਕਾਓ;
  // Fastpath for moving by whole words.
  ਜੇ (ਦੋਆਧਾਰੀ_ਅੰਕ_ਖਿਸਕਾਓ == 0) {
    std::memmove(Dst, Dst + ਸ਼ਬਦ_ਖਿਸਕਾਓ, ਸ਼ਬਦ_ਮੰਜ਼ਿਲ_Move * APINT_ਸ਼ਬਦ_ਮਾਪ);
  } ਨਹੀਂਤਾਂ {
    ਲਈ (ਬਿਨਾਨਿਸ਼ਾਨੀ i = 0; i != ਸ਼ਬਦ_ਮੰਜ਼ਿਲ_Move; ++i) {
      Dst[i] = Dst[i + ਸ਼ਬਦ_ਖਿਸਕਾਓ] >> ਦੋਆਧਾਰੀ_ਅੰਕ_ਖਿਸਕਾਓ;
      ਜੇ (i + 1 != ਸ਼ਬਦ_ਮੰਜ਼ਿਲ_Move)
        Dst[i] |= Dst[i + ਸ਼ਬਦ_ਖਿਸਕਾਓ + 1] << (APINT_ਬਿੱਟ_ਪ੍ਰਤੀ_ਸ਼ਬਦ - ਦੋਆਧਾਰੀ_ਅੰਕ_ਖਿਸਕਾਓ);
    }
  }

  // Fill in the remainder with 0s.
  std::memset(Dst + ਸ਼ਬਦ_ਮੰਜ਼ਿਲ_Move, 0, ਸ਼ਬਦ_ਖਿਸਕਾਓ * APINT_ਸ਼ਬਦ_ਮਾਪ);
}

// Comparison (unsigned) of two bignums.
ਪੂਰਨ APInt::tcCompare(ਅਚਲ ਸ਼ਬਦ_Type *lhs, ਅਚਲ ਸ਼ਬਦ_Type *rhs,
                     ਬਿਨਾਨਿਸ਼ਾਨੀ parts) {
  ਜਦੋਂ (parts) {
    parts--;
    ਜੇ (lhs[parts] != rhs[parts])
      ਵਾਪਸ (lhs[parts] > rhs[parts]) ? 1 : -1;
  }

  ਵਾਪਸ 0;
}

APInt llvm::APIntOps::RoundingUDiv(ਅਚਲ APInt &A, ਅਚਲ APInt &B,
                                   APInt::Rounding RM) {
  // Currently udivrem always rounds down.
  ਚੋਣ (RM) {
  ਹਾਲਤ APInt::Rounding::ਥੱਲੇ:
  ਹਾਲਤ APInt::Rounding::TOWARD_ZERO:
    ਵਾਪਸ A.udiv(B);
  ਹਾਲਤ APInt::Rounding::ਉੱਪਰ: {
    APInt Quo, Rem;
    APInt::udivrem(A, B, Quo, Rem);
    ਜੇ (Rem.isZero())
      ਵਾਪਸ Quo;
    ਵਾਪਸ Quo + 1;
  }
  }
  llvm_unreachable("Unknown APInt::Rounding enum");
}

APInt llvm::APIntOps::RoundingSDiv(ਅਚਲ APInt &A, ਅਚਲ APInt &B,
                                   APInt::Rounding RM) {
  ਚੋਣ (RM) {
  ਹਾਲਤ APInt::Rounding::ਥੱਲੇ:
  ਹਾਲਤ APInt::Rounding::ਉੱਪਰ: {
    APInt Quo, Rem;
    APInt::sdivrem(A, B, Quo, Rem);
    ਜੇ (Rem.isZero())
      ਵਾਪਸ Quo;
    // This algorithm deals with arbitrary rounding mode used by sdivrem.
    // We want to check whether the non-integer part of the mathematical value
    // is negative or not. If the non-integer part is negative, we need to round
    // down from Quo; otherwise, if it's positive or 0, we return Quo, as it's
    // already rounded down.
    ਜੇ (RM == APInt::Rounding::ਥੱਲੇ) {
      ਜੇ (Rem.isNegative() != B.isNegative())
        ਵਾਪਸ Quo - 1;
      ਵਾਪਸ Quo;
    }
    ਜੇ (Rem.isNegative() != B.isNegative())
      ਵਾਪਸ Quo;
    ਵਾਪਸ Quo + 1;
  }
  // Currently sdiv rounds towards zero.
  ਹਾਲਤ APInt::Rounding::TOWARD_ZERO:
    ਵਾਪਸ A.sdiv(B);
  }
  llvm_unreachable("Unknown APInt::Rounding enum");
}

std::optional<APInt>
llvm::APIntOps::SolveQuadraticEquationWrap(APInt A, APInt B, APInt C,
                                           ਬਿਨਾਨਿਸ਼ਾਨੀ Range_ਚੌੜਾਈ) {
  ਬਿਨਾਨਿਸ਼ਾਨੀ Coeff_ਚੌੜਾਈ = A.get_ਦੋਆਧਾਰੀ_ਅੰਕ_ਚੌੜਾਈ();
  ਪ੍ਰਤਿਜ੍ਞਾ(Coeff_ਚੌੜਾਈ == B.get_ਦੋਆਧਾਰੀ_ਅੰਕ_ਚੌੜਾਈ() && Coeff_ਚੌੜਾਈ == C.get_ਦੋਆਧਾਰੀ_ਅੰਕ_ਚੌੜਾਈ());
  ਪ੍ਰਤਿਜ੍ਞਾ(Range_ਚੌੜਾਈ <= Coeff_ਚੌੜਾਈ &&
         "Value range width should be less than coefficient width");
  ਪ੍ਰਤਿਜ੍ਞਾ(Range_ਚੌੜਾਈ > 1 && "Value range bit width should be > 1");

  LLVM_DEBUG(dbgs() << __func__ << ": solving " << A << "x^2 + " << B
                    << "x + " << C << ", rw:" << Range_ਚੌੜਾਈ << '\n');

  // Identify 0 as a (non)solution immediately.
  ਜੇ (C.sextOrTrunc(Range_ਚੌੜਾਈ).isZero()) {
    LLVM_DEBUG(dbgs() << __func__ << ": zero solution\n");
    ਵਾਪਸ APInt(Coeff_ਚੌੜਾਈ, 0);
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
  Coeff_ਚੌੜਾਈ *= 3;
  A = A.sext(Coeff_ਚੌੜਾਈ);
  B = B.sext(Coeff_ਚੌੜਾਈ);
  C = C.sext(Coeff_ਚੌੜਾਈ);

  // Make A > 0 for simplicity. Negate cannot overflow at this point because
  // the bit width has increased.
  ਜੇ (A.isNegative()) {
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
  APInt R = APInt::get_One_ਦੋਆਧਾਰੀ_ਅੰਕ_ਸੈੱਟ(Coeff_ਚੌੜਾਈ, Range_ਚੌੜਾਈ);
  APInt TwoA = 2 * A;
  APInt SqrB = B * B;
  ਤਾਰਕਿਕ Pick_ਨੀਵਾਂ;

  ਸਵੈਚਾਲਿਤ ਗੇੜਾ_ਉੱਪਰ_2 = [] (ਅਚਲ APInt &V, ਅਚਲ APInt &A) -> APInt {
    ਪ੍ਰਤਿਜ੍ਞਾ(A.is_Strictly_ਸਕਾਰਾਤਮਕ());
    APInt T = V.ਨਿਰਪੇਖ().urem(A);
    ਜੇ (T.isZero())
      ਵਾਪਸ V;
    ਵਾਪਸ V.isNegative() ? V+T : V+(A-T);
  };

  // The vertex of the parabola is at -B/2A, but since A > 0, it's negative
  // iff B is positive.
  ਜੇ (B.isNonNegative()) {
    // If B >= 0, the vertex it at a negative location (or at 0), so in
    // order to have a non-negative solution we need to pick k that makes
    // C-kR negative. To satisfy all the requirements for the solution
    // that we are looking for, it needs to be closest to 0 of all k.
    C = C.srem(R);
    ਜੇ (C.is_Strictly_ਸਕਾਰਾਤਮਕ())
      C -= R;
    // Pick the greater solution.
    Pick_ਨੀਵਾਂ = ਝੂਠ;
  } ਨਹੀਂਤਾਂ {
    // If B < 0, the vertex is at a positive location. For any solution
    // to exist, the discriminant must be non-negative. This means that
    // C-kR <= B^2/4A is a necessary condition for k, i.e. there is a
    // lower bound on values of k: kR >= C - B^2/4A.
    APInt LowkR = C - SqrB.udiv(2*TwoA); // udiv because all values > 0.
    // Round LowkR up (towards +inf) to the nearest kR.
    LowkR = ਗੇੜਾ_ਉੱਪਰ_2(LowkR, R);

    // If there exists k meeting the condition above, and such that
    // C-kR > 0, there will be two positive real number solutions of
    // q(x) = kR. Out of all such values of k, pick the one that makes
    // C-kR closest to 0, (i.e. pick maximum k such that C-kR > 0).
    // In other words, find maximum k such that LowkR <= kR < C.
    ਜੇ (C.sgt(LowkR)) {
      // If LowkR < C, then such a k is guaranteed to exist because
      // LowkR itself is a multiple of R.
      C -= -ਗੇੜਾ_ਉੱਪਰ_2(-C, R);      // C = C - RoundDown(C, R)
      // Pick the smaller solution.
      Pick_ਨੀਵਾਂ = ਸੱਚ;
    } ਨਹੀਂਤਾਂ {
      // If C-kR < 0 for all potential k's, it means that one solution
      // will be negative, while the other will be positive. The positive
      // solution will shift towards 0 if the parabola is moved up.
      // Pick the kR closest to the lower bound (i.e. make C-kR closest
      // to 0, or in other words, out of all parabolas that have solutions,
      // pick the one that is the farthest "up").
      // Since LowkR is itself a multiple of R, simply take C-LowkR.
      C -= LowkR;
      // Pick the greater solution.
      Pick_ਨੀਵਾਂ = ਝੂਠ;
    }
  }

  LLVM_DEBUG(dbgs() << __func__ << ": updated coefficients " << A << "x^2 + "
                    << B << "x + " << C << ", rw:" << Range_ਚੌੜਾਈ << '\n');

  APInt D = SqrB - 4*A*C;
  ਪ੍ਰਤਿਜ੍ਞਾ(D.isNonNegative() && "Negative discriminant");
  APInt SQ = D.sqrtFloor();

  APInt Q = SQ * SQ;
  ਤਾਰਕਿਕ InexactSQ = Q != D;

  APInt X;
  APInt Rem;

  // SQ is rounded down (i.e SQ * SQ <= D), so the roots may be inexact.
  // When using the quadratic formula directly, the calculated low root
  // may be greater than the exact one, since we would be subtracting SQ.
  // To make sure that the calculated root is not greater than the exact
  // one, subtract SQ+1 when calculating the low root (for inexact value
  // of SQ).
  ਜੇ (Pick_ਨੀਵਾਂ)
    APInt::sdivrem(-B - (SQ+InexactSQ), TwoA, X, Rem);
  ਨਹੀਂਤਾਂ
    APInt::sdivrem(-B + SQ, TwoA, X, Rem);

  // The updated coefficients should be such that the (exact) solution is
  // positive. Since APInt division rounds towards 0, the calculated one
  // can be 0, but cannot be negative.
  ਪ੍ਰਤਿਜ੍ਞਾ(X.isNonNegative() && "Solution should be non-negative");

  ਜੇ (!InexactSQ && Rem.isZero()) {
    LLVM_DEBUG(dbgs() << __func__ << ": solution (root): " << X << '\n');
    ਵਾਪਸ X;
  }

  ਪ੍ਰਤਿਜ੍ਞਾ((SQ*SQ).sle(D) && "SQ = |_sqrt(D)_|, so SQ*SQ <= D");
  // The exact value of the square root of D should be between SQ and SQ+1.
  // This implies that the solution should be between that corresponding to
  // SQ (i.e. X) and that corresponding to SQ+1.
  //
  // The calculated X cannot be greater than the exact (real) solution.
  // Actually it must be strictly less than the exact solution, while
  // X+1 will be greater than or equal to it.

  APInt VX = (A*X + B)*X + C;
  APInt VY = VX + TwoA*X + A + B;
  ਤਾਰਕਿਕ SignChange =
      VX.isNegative() != VY.isNegative() || VX.isZero() != VY.isZero();
  // If the sign did not change between X and X+1, X is not a valid solution.
  // This could happen when the actual (exact) roots don't have an integer
  // between them, so they would both be contained between X and X+1.
  ਜੇ (!SignChange) {
    LLVM_DEBUG(dbgs() << __func__ << ": no valid solution\n");
    ਵਾਪਸ std::nullopt;
  }

  X += 1;
  LLVM_DEBUG(dbgs() << __func__ << ": solution (wrap): " << X << '\n');
  ਵਾਪਸ X;
}

std::optional<ਬਿਨਾਨਿਸ਼ਾਨੀ>
llvm::APIntOps::Get_Most_Significant_Different_ਦੋਆਧਾਰੀ_ਅੰਕ(ਅਚਲ APInt &A, ਅਚਲ APInt &B) {
  ਪ੍ਰਤਿਜ੍ਞਾ(A.get_ਦੋਆਧਾਰੀ_ਅੰਕ_ਚੌੜਾਈ() == B.get_ਦੋਆਧਾਰੀ_ਅੰਕ_ਚੌੜਾਈ() && "Must have the same bitwidth");
  ਜੇ (A == B)
    ਵਾਪਸ std::nullopt;
  ਵਾਪਸ A.get_ਦੋਆਧਾਰੀ_ਅੰਕ_ਚੌੜਾਈ() - ((A ^ B).countl_zero() + 1);
}

APInt llvm::APIntOps::Scale_ਦੋਆਧਾਰੀ_ਅੰਕ_ਨਕਾਬ(ਅਚਲ APInt &A, ਬਿਨਾਨਿਸ਼ਾਨੀ New_ਦੋਆਧਾਰੀ_ਅੰਕ_ਚੌੜਾਈ,
                                   ਤਾਰਕਿਕ ਮੇਲ_All_ਬਿੱਟ) {
  ਬਿਨਾਨਿਸ਼ਾਨੀ Old_ਦੋਆਧਾਰੀ_ਅੰਕ_ਚੌੜਾਈ = A.get_ਦੋਆਧਾਰੀ_ਅੰਕ_ਚੌੜਾਈ();
  ਪ੍ਰਤਿਜ੍ਞਾ((((Old_ਦੋਆਧਾਰੀ_ਅੰਕ_ਚੌੜਾਈ % New_ਦੋਆਧਾਰੀ_ਅੰਕ_ਚੌੜਾਈ) == 0) ||
          ((New_ਦੋਆਧਾਰੀ_ਅੰਕ_ਚੌੜਾਈ % Old_ਦੋਆਧਾਰੀ_ਅੰਕ_ਚੌੜਾਈ) == 0)) &&
         "One size should be a multiple of the other one. "
         "Can't do fractional scaling.");

  // Check for matching bitwidths.
  ਜੇ (Old_ਦੋਆਧਾਰੀ_ਅੰਕ_ਚੌੜਾਈ == New_ਦੋਆਧਾਰੀ_ਅੰਕ_ਚੌੜਾਈ)
    ਵਾਪਸ A;

  APInt NewA = APInt::getZero(New_ਦੋਆਧਾਰੀ_ਅੰਕ_ਚੌੜਾਈ);

  // Check for null input.
  ਜੇ (A.isZero())
    ਵਾਪਸ NewA;

  ਜੇ (New_ਦੋਆਧਾਰੀ_ਅੰਕ_ਚੌੜਾਈ > Old_ਦੋਆਧਾਰੀ_ਅੰਕ_ਚੌੜਾਈ) {
    // Repeat bits.
    ਬਿਨਾਨਿਸ਼ਾਨੀ Scale = New_ਦੋਆਧਾਰੀ_ਅੰਕ_ਚੌੜਾਈ / Old_ਦੋਆਧਾਰੀ_ਅੰਕ_ਚੌੜਾਈ;
    ਲਈ (ਬਿਨਾਨਿਸ਼ਾਨੀ i = 0; i != Old_ਦੋਆਧਾਰੀ_ਅੰਕ_ਚੌੜਾਈ; ++i)
      ਜੇ (A[i])
        NewA.ਸੈੱਟ_ਬਿੱਟ(i * Scale, (i + 1) * Scale);
  } ਨਹੀਂਤਾਂ {
    ਬਿਨਾਨਿਸ਼ਾਨੀ Scale = Old_ਦੋਆਧਾਰੀ_ਅੰਕ_ਚੌੜਾਈ / New_ਦੋਆਧਾਰੀ_ਅੰਕ_ਚੌੜਾਈ;
    ਲਈ (ਬਿਨਾਨਿਸ਼ਾਨੀ i = 0; i != New_ਦੋਆਧਾਰੀ_ਅੰਕ_ਚੌੜਾਈ; ++i) {
      ਜੇ (ਮੇਲ_All_ਬਿੱਟ) {
        ਜੇ (A.ਕੱਢੋ_ਬਿੱਟ(Scale, i * Scale).isAllOnes())
          NewA.ਸੈੱਟ_ਦੋਆਧਾਰੀ_ਅੰਕ(i);
      } ਨਹੀਂਤਾਂ {
        ਜੇ (!A.ਕੱਢੋ_ਬਿੱਟ(Scale, i * Scale).isZero())
          NewA.ਸੈੱਟ_ਦੋਆਧਾਰੀ_ਅੰਕ(i);
      }
    }
  }

  ਵਾਪਸ NewA;
}

/// StoreIntToMemory - Fills the StoreBytes bytes of memory starting from Dst
/// with the integer held in IntVal.
ਖਾਲੀ llvm::Store_Int_ਮੰਜ਼ਿਲ_ਸਮ੍ਰਿਤੀ(ਅਚਲ APInt &IntVal, uint8_t *Dst,
                            ਬਿਨਾਨਿਸ਼ਾਨੀ Store_ਬਾਈਟਾਂ) {
  ਪ੍ਰਤਿਜ੍ਞਾ((IntVal.get_ਦੋਆਧਾਰੀ_ਅੰਕ_ਚੌੜਾਈ()+7)/8 >= Store_ਬਾਈਟਾਂ && "Integer too small!");
  ਅਚਲ uint8_t *Src = (ਅਚਲ uint8_t *)IntVal.get_Raw_ਅੰਕੜੇ();

  ਜੇ (sys::IsLittleEndianHost) {
    // Little-endian host - the source is ordered from LSB to MSB.  Order the
    // destination from LSB to MSB: Do a straight copy.
    ਸਮ੍ਰਿਤੀ_ਨਕਲ(Dst, Src, Store_ਬਾਈਟਾਂ);
  } ਨਹੀਂਤਾਂ {
    // Big-endian host - the source is an array of 64 bit words ordered from
    // LSW to MSW.  Each word is ordered from MSB to LSB.  Order the destination
    // from MSB to LSB: Reverse the word order, but not the bytes in a word.
    ਜਦੋਂ (Store_ਬਾਈਟਾਂ > ਆਕਾਰ(uint64_t)) {
      Store_ਬਾਈਟਾਂ -= ਆਕਾਰ(uint64_t);
      // May not be aligned so use memcpy.
      ਸਮ੍ਰਿਤੀ_ਨਕਲ(Dst + Store_ਬਾਈਟਾਂ, Src, ਆਕਾਰ(uint64_t));
      Src += ਆਕਾਰ(uint64_t);
    }

    ਸਮ੍ਰਿਤੀ_ਨਕਲ(Dst, Src + ਆਕਾਰ(uint64_t) - Store_ਬਾਈਟਾਂ, Store_ਬਾਈਟਾਂ);
  }
}

/// LoadIntFromMemory - Loads the integer stored in the LoadBytes bytes starting
/// from Src into IntVal, which is assumed to be wide enough and to hold zero.
ਖਾਲੀ llvm::ਲੱਦੋ_Int_From_ਸਮ੍ਰਿਤੀ(APInt &IntVal, ਅਚਲ uint8_t *Src,
                             ਬਿਨਾਨਿਸ਼ਾਨੀ ਲੱਦੋ_ਬਾਈਟਾਂ) {
  ਪ੍ਰਤਿਜ੍ਞਾ((IntVal.get_ਦੋਆਧਾਰੀ_ਅੰਕ_ਚੌੜਾਈ()+7)/8 >= ਲੱਦੋ_ਬਾਈਟਾਂ && "Integer too small!");
  uint8_t *Dst = reinterpret_cast<uint8_t *>(
                   const_cast<uint64_t *>(IntVal.get_Raw_ਅੰਕੜੇ()));

  ਜੇ (sys::IsLittleEndianHost)
    // Little-endian host - the destination must be ordered from LSB to MSB.
    // The source is ordered from LSB to MSB: Do a straight copy.
    ਸਮ੍ਰਿਤੀ_ਨਕਲ(Dst, Src, ਲੱਦੋ_ਬਾਈਟਾਂ);
  ਨਹੀਂਤਾਂ {
    // Big-endian - the destination is an array of 64 bit words ordered from
    // LSW to MSW.  Each word must be ordered from MSB to LSB.  The source is
    // ordered from MSB to LSB: Reverse the word order, but not the bytes in
    // a word.
    ਜਦੋਂ (ਲੱਦੋ_ਬਾਈਟਾਂ > ਆਕਾਰ(uint64_t)) {
      ਲੱਦੋ_ਬਾਈਟਾਂ -= ਆਕਾਰ(uint64_t);
      // May not be aligned so use memcpy.
      ਸਮ੍ਰਿਤੀ_ਨਕਲ(Dst, Src + ਲੱਦੋ_ਬਾਈਟਾਂ, ਆਕਾਰ(uint64_t));
      Dst += ਆਕਾਰ(uint64_t);
    }

    ਸਮ੍ਰਿਤੀ_ਨਕਲ(Dst + ਆਕਾਰ(uint64_t) - ਲੱਦੋ_ਬਾਈਟਾਂ, Src, ਲੱਦੋ_ਬਾਈਟਾਂ);
  }
}

APInt APIntOps::avgFloorS(ਅਚਲ APInt &C1, ਅਚਲ APInt &C2) {
  // Return floor((C1 + C2) / 2)
  ਵਾਪਸ (C1 & C2) + (C1 ^ C2).ashr(1);
}

APInt APIntOps::avgFloorU(ਅਚਲ APInt &C1, ਅਚਲ APInt &C2) {
  // Return floor((C1 + C2) / 2)
  ਵਾਪਸ (C1 & C2) + (C1 ^ C2).lshr(1);
}

APInt APIntOps::avgCeilS(ਅਚਲ APInt &C1, ਅਚਲ APInt &C2) {
  // Return ceil((C1 + C2) / 2)
  ਵਾਪਸ (C1 | C2) - (C1 ^ C2).ashr(1);
}

APInt APIntOps::avgCeilU(ਅਚਲ APInt &C1, ਅਚਲ APInt &C2) {
  // Return ceil((C1 + C2) / 2)
  ਵਾਪਸ (C1 | C2) - (C1 ^ C2).lshr(1);
}

APInt APIntOps::avgSubFloorS(ਅਚਲ APInt &C1, ਅਚਲ APInt &C2) {
  // Return floor((C1 - C2) / 2), using C1 - C2 == (C1 ^ C2) - 2 * (~C1 & C2).
  ਵਾਪਸ (C1 ^ C2).ashr(1) - (~C1 & C2);
}

APInt APIntOps::avgSubFloorU(ਅਚਲ APInt &C1, ਅਚਲ APInt &C2) {
  // Return floor((C1 - C2) / 2), using C1 - C2 == (C1 ^ C2) - 2 * (~C1 & C2).
  ਵਾਪਸ (C1 ^ C2).lshr(1) - (~C1 & C2);
}

APInt APIntOps::mulhs(ਅਚਲ APInt &C1, ਅਚਲ APInt &C2) {
  ਪ੍ਰਤਿਜ੍ਞਾ(C1.get_ਦੋਆਧਾਰੀ_ਅੰਕ_ਚੌੜਾਈ() == C2.get_ਦੋਆਧਾਰੀ_ਅੰਕ_ਚੌੜਾਈ() && "Unequal bitwidths");
  ਬਿਨਾਨਿਸ਼ਾਨੀ ਭਰਿਆ_ਚੌੜਾਈ = C1.get_ਦੋਆਧਾਰੀ_ਅੰਕ_ਚੌੜਾਈ() * 2;
  APInt C1Ext = C1.sext(ਭਰਿਆ_ਚੌੜਾਈ);
  APInt C2Ext = C2.sext(ਭਰਿਆ_ਚੌੜਾਈ);
  ਵਾਪਸ (C1Ext * C2Ext).ਕੱਢੋ_ਬਿੱਟ(C1.get_ਦੋਆਧਾਰੀ_ਅੰਕ_ਚੌੜਾਈ(), C1.get_ਦੋਆਧਾਰੀ_ਅੰਕ_ਚੌੜਾਈ());
}

APInt APIntOps::mulhu(ਅਚਲ APInt &C1, ਅਚਲ APInt &C2) {
  ਪ੍ਰਤਿਜ੍ਞਾ(C1.get_ਦੋਆਧਾਰੀ_ਅੰਕ_ਚੌੜਾਈ() == C2.get_ਦੋਆਧਾਰੀ_ਅੰਕ_ਚੌੜਾਈ() && "Unequal bitwidths");
  ਬਿਨਾਨਿਸ਼ਾਨੀ ਭਰਿਆ_ਚੌੜਾਈ = C1.get_ਦੋਆਧਾਰੀ_ਅੰਕ_ਚੌੜਾਈ() * 2;
  APInt C1Ext = C1.zext(ਭਰਿਆ_ਚੌੜਾਈ);
  APInt C2Ext = C2.zext(ਭਰਿਆ_ਚੌੜਾਈ);
  ਵਾਪਸ (C1Ext * C2Ext).ਕੱਢੋ_ਬਿੱਟ(C1.get_ਦੋਆਧਾਰੀ_ਅੰਕ_ਚੌੜਾਈ(), C1.get_ਦੋਆਧਾਰੀ_ਅੰਕ_ਚੌੜਾਈ());
}

APInt APIntOps::mulsExtended(ਅਚਲ APInt &C1, ਅਚਲ APInt &C2) {
  ਪ੍ਰਤਿਜ੍ਞਾ(C1.get_ਦੋਆਧਾਰੀ_ਅੰਕ_ਚੌੜਾਈ() == C2.get_ਦੋਆਧਾਰੀ_ਅੰਕ_ਚੌੜਾਈ() && "Unequal bitwidths");
  ਬਿਨਾਨਿਸ਼ਾਨੀ ਭਰਿਆ_ਚੌੜਾਈ = C1.get_ਦੋਆਧਾਰੀ_ਅੰਕ_ਚੌੜਾਈ() * 2;
  APInt C1Ext = C1.sext(ਭਰਿਆ_ਚੌੜਾਈ);
  APInt C2Ext = C2.sext(ਭਰਿਆ_ਚੌੜਾਈ);
  ਵਾਪਸ C1Ext * C2Ext;
}

APInt APIntOps::muluExtended(ਅਚਲ APInt &C1, ਅਚਲ APInt &C2) {
  ਪ੍ਰਤਿਜ੍ਞਾ(C1.get_ਦੋਆਧਾਰੀ_ਅੰਕ_ਚੌੜਾਈ() == C2.get_ਦੋਆਧਾਰੀ_ਅੰਕ_ਚੌੜਾਈ() && "Unequal bitwidths");
  ਬਿਨਾਨਿਸ਼ਾਨੀ ਭਰਿਆ_ਚੌੜਾਈ = C1.get_ਦੋਆਧਾਰੀ_ਅੰਕ_ਚੌੜਾਈ() * 2;
  APInt C1Ext = C1.zext(ਭਰਿਆ_ਚੌੜਾਈ);
  APInt C2Ext = C2.zext(ਭਰਿਆ_ਚੌੜਾਈ);
  ਵਾਪਸ C1Ext * C2Ext;
}

APInt APIntOps::ਘਾਤ(ਅਚਲ APInt &X, int64_t N) {
  ਪ੍ਰਤਿਜ੍ਞਾ(N >= 0 && "negative exponents not supported.");
  APInt Acc = APInt(X.get_ਦੋਆਧਾਰੀ_ਅੰਕ_ਚੌੜਾਈ(), 1);
  ਜੇ (N == 0)
    ਵਾਪਸ Acc;
  APInt ਅਧਾਰ = X;
  int64_t ਬਚਿਆ_Exponent = N;
  ਜਦੋਂ (ਬਚਿਆ_Exponent > 0) {
    ਜਦੋਂ (ਬਚਿਆ_Exponent % 2 == 0) {
      ਅਧਾਰ *= ਅਧਾਰ;
      ਬਚਿਆ_Exponent /= 2;
    }
    --ਬਚਿਆ_Exponent;
    Acc *= ਅਧਾਰ;
  }
  ਵਾਪਸ Acc;
}

APInt llvm::APIntOps::fshl(ਅਚਲ APInt &Hi, ਅਚਲ APInt &Lo,
                           ਅਚਲ APInt &ਖਿਸਕਾਓ_2) {
  ਪ੍ਰਤਿਜ੍ਞਾ(Hi.get_ਦੋਆਧਾਰੀ_ਅੰਕ_ਚੌੜਾਈ() == Lo.get_ਦੋਆਧਾਰੀ_ਅੰਕ_ਚੌੜਾਈ());
  ਬਿਨਾਨਿਸ਼ਾਨੀ ਖਿਸਕਾਓ_Amt_2 = rotateModulo(Hi.get_ਦੋਆਧਾਰੀ_ਅੰਕ_ਚੌੜਾਈ(), ਖਿਸਕਾਓ_2);
  ਜੇ (ਖਿਸਕਾਓ_Amt_2 == 0)
    ਵਾਪਸ Hi;
  ਵਾਪਸ Hi.shl(ਖਿਸਕਾਓ_Amt_2) | Lo.lshr(Hi.get_ਦੋਆਧਾਰੀ_ਅੰਕ_ਚੌੜਾਈ() - ਖਿਸਕਾਓ_Amt_2);
}

APInt llvm::APIntOps::fshr(ਅਚਲ APInt &Hi, ਅਚਲ APInt &Lo,
                           ਅਚਲ APInt &ਖਿਸਕਾਓ_2) {
  ਪ੍ਰਤਿਜ੍ਞਾ(Hi.get_ਦੋਆਧਾਰੀ_ਅੰਕ_ਚੌੜਾਈ() == Lo.get_ਦੋਆਧਾਰੀ_ਅੰਕ_ਚੌੜਾਈ());
  ਬਿਨਾਨਿਸ਼ਾਨੀ ਖਿਸਕਾਓ_Amt_2 = rotateModulo(Hi.get_ਦੋਆਧਾਰੀ_ਅੰਕ_ਚੌੜਾਈ(), ਖਿਸਕਾਓ_2);
  ਜੇ (ਖਿਸਕਾਓ_Amt_2 == 0)
    ਵਾਪਸ Lo;
  ਵਾਪਸ Hi.shl(Hi.get_ਦੋਆਧਾਰੀ_ਅੰਕ_ਚੌੜਾਈ() - ਖਿਸਕਾਓ_Amt_2) | Lo.lshr(ਖਿਸਕਾਓ_Amt_2);
}

APInt llvm::APIntOps::clmul(ਅਚਲ APInt &LHS, ਅਚਲ APInt &RHS) {
  ਬਿਨਾਨਿਸ਼ਾਨੀ BW = LHS.get_ਦੋਆਧਾਰੀ_ਅੰਕ_ਚੌੜਾਈ();
  ਪ੍ਰਤਿਜ੍ਞਾ(BW == RHS.get_ਦੋਆਧਾਰੀ_ਅੰਕ_ਚੌੜਾਈ() && "Operand mismatch");
  APInt ਨਤੀਜਾ(BW, 0);
  ਲਈ (ਬਿਨਾਨਿਸ਼ਾਨੀ I : seq(std::min(RHS.get_ਸਰਗਰਮ_ਬਿੱਟ(), BW - LHS.countr_zero())))
    ਜੇ (RHS[I])
      ਨਤੀਜਾ ^= LHS << I;
  ਵਾਪਸ ਨਤੀਜਾ;
}

APInt llvm::APIntOps::clmulr(ਅਚਲ APInt &LHS, ਅਚਲ APInt &RHS) {
  ਪ੍ਰਤਿਜ੍ਞਾ(LHS.get_ਦੋਆਧਾਰੀ_ਅੰਕ_ਚੌੜਾਈ() == RHS.get_ਦੋਆਧਾਰੀ_ਅੰਕ_ਚੌੜਾਈ());
  ਵਾਪਸ clmul(LHS.ਉਲਟ_ਬਿੱਟ(), RHS.ਉਲਟ_ਬਿੱਟ()).ਉਲਟ_ਬਿੱਟ();
}

APInt llvm::APIntOps::clmulh(ਅਚਲ APInt &LHS, ਅਚਲ APInt &RHS) {
  ਪ੍ਰਤਿਜ੍ਞਾ(LHS.get_ਦੋਆਧਾਰੀ_ਅੰਕ_ਚੌੜਾਈ() == RHS.get_ਦੋਆਧਾਰੀ_ਅੰਕ_ਚੌੜਾਈ());
  ਵਾਪਸ clmulr(LHS, RHS).lshr(1);
}

APInt llvm::APIntOps::pext(ਅਚਲ APInt &Val, ਅਚਲ APInt &ਨਕਾਬ) {
  ਬਿਨਾਨਿਸ਼ਾਨੀ BW = Val.get_ਦੋਆਧਾਰੀ_ਅੰਕ_ਚੌੜਾਈ();
  ਪ੍ਰਤਿਜ੍ਞਾ(BW == ਨਕਾਬ.get_ਦੋਆਧਾਰੀ_ਅੰਕ_ਚੌੜਾਈ() && "Operand mismatch");
  APInt ਨਤੀਜਾ = APInt::getZero(BW);
  ਲਈ (ਬਿਨਾਨਿਸ਼ਾਨੀ I = 0, P = 0; I != BW; ++I)
    ਜੇ (ਨਕਾਬ[I])
      ਨਤੀਜਾ.ਸੈੱਟ_ਦੋਆਧਾਰੀ_ਅੰਕ_Val(P++, Val[I]);
  ਵਾਪਸ ਨਤੀਜਾ;
}

APInt llvm::APIntOps::pdep(ਅਚਲ APInt &Val, ਅਚਲ APInt &ਨਕਾਬ) {
  ਬਿਨਾਨਿਸ਼ਾਨੀ BW = Val.get_ਦੋਆਧਾਰੀ_ਅੰਕ_ਚੌੜਾਈ();
  ਪ੍ਰਤਿਜ੍ਞਾ(BW == ਨਕਾਬ.get_ਦੋਆਧਾਰੀ_ਅੰਕ_ਚੌੜਾਈ() && "Operand mismatch");
  APInt ਨਤੀਜਾ = APInt::getZero(BW);
  ਲਈ (ਬਿਨਾਨਿਸ਼ਾਨੀ I = 0, P = 0; I != BW; ++I)
    ਜੇ (ਨਕਾਬ[I])
      ਨਤੀਜਾ.ਸੈੱਟ_ਦੋਆਧਾਰੀ_ਅੰਕ_Val(I, Val[P++]);
  ਵਾਪਸ ਨਤੀਜਾ;
}
