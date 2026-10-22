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

#ضمن "llvm/ADT/APInt.h"
#ضمن "llvm/ADT/ArrayRef.h"
#ضمن "llvm/ADT/FoldingSet.h"
#ضمن "llvm/ADT/Hashing.h"
#ضمن "llvm/ADT/Sequence.h"
#ضمن "llvm/ADT/SmallString.h"
#ضمن "llvm/ADT/StringRef.h"
#ضمن "llvm/ADT/bit.h"
#ضمن "llvm/Support/Alignment.h"
#ضمن "llvm/Support/Debug.h"
#ضمن "llvm/Support/ErrorHandling.h"
#ضمن "llvm/Support/MathExtras.h"
#ضمن "llvm/Support/SwapByteOrder.h"
#ضمن "llvm/Support/raw_ostream.h"
#ضمن <cmath>
#ضمن <optional>

ورتوں نطاق llvm;

#تعریف_کرو DEBUG_TYPE "apint"

/// A utility function for allocating memory, checking for allocation failures,
/// and ensuring the contents are zeroed.
مضمن ساكن uint64_t* get_Cleared_حافظہ(بنانشانی عدد_الفاظ) {
  أرجع جديد uint64_t[عدد_الفاظ]();
}

/// A utility function for allocating memory and checking for allocation
/// failure.  The content is not zeroed.
مضمن ساكن uint64_t* get_حافظہ(بنانشانی عدد_الفاظ) {
  أرجع جديد uint64_t[عدد_الفاظ];
}

/// A utility function that converts a character to a digit.
مضمن ساكن بنانشانی getDigit(حرف cdigit, uint8_t radix) {
  بنانشانی r;

  إذا (radix == 16 || radix == 36) {
    r = cdigit - '0';
    إذا (r <= 9)
      أرجع r;

    r = cdigit - 'A';
    إذا (r <= radix - 11U)
      أرجع r + 10;

    r = cdigit - 'a';
    إذا (r <= radix - 11U)
      أرجع r + 10;

    radix = 10;
  }

  r = cdigit - '0';
  إذا (r < radix)
    أرجع r;

  أرجع UINT_MAX;
}


فراغ APInt::initSlowCase(uint64_t val, منطقي isSigned) {
  إذا (isSigned && int64_t(val) < 0) {
    U.pVal = get_حافظہ(get_عدد_الفاظ());
    U.pVal[0] = val;
    memset(&U.pVal[1], 0xFF, APINT_لفظ_ناپ * (get_عدد_الفاظ() - 1));
    clear_بن_ورتیا_بٹ();
  } وإلا {
    U.pVal = get_Cleared_حافظہ(get_عدد_الفاظ());
    U.pVal[0] = val;
  }
}

فراغ APInt::initSlowCase(ثابت APInt& that) {
  U.pVal = get_حافظہ(get_عدد_الفاظ());
  حافظہ_نقل(U.pVal, that.U.pVal, get_عدد_الفاظ() * APINT_لفظ_ناپ);
}

فراغ APInt::initFromArray(ArrayRef<uint64_t> bigVal) {
  أكد(bigVal.انکڑے() && "Null pointer detected!");
  إذا (LLVM_LIKELY(is_Single_لفظ()))
    U.VAL = bigVal[0];
  وإلا {
    // Get memory, cleared to 0
    U.pVal = get_Cleared_حافظہ(get_عدد_الفاظ());
    // Calculate the number of words to copy
    بنانشانی الفاظ = std::min<بنانشانی>(bigVal.ناپ_2(), get_عدد_الفاظ());
    // Copy the words from bigVal to pVal
    حافظہ_نقل(U.pVal, bigVal.انکڑے(), الفاظ * APINT_لفظ_ناپ);
  }
  // Make sure unused high bits are cleared
  clear_بن_ورتیا_بٹ();
}

APInt::APInt(بنانشانی عدد_بٹ, ArrayRef<uint64_t> bigVal) : ثنائی_ہندسہ_چوڑائی_2(عدد_بٹ) {
  initFromArray(bigVal);
}

APInt::APInt(بنانشانی numbits, StringRef Str, uint8_t radix)
    : ثنائی_ہندسہ_چوڑائی_2(numbits) {
  fromString(numbits, Str, radix);
}

فراغ APInt::reallocate(بنانشانی New_ثنائی_ہندسہ_چوڑائی) {
  // If the number of words is the same we can just change the width and stop.
  إذا (get_عدد_الفاظ() == get_عدد_الفاظ(New_ثنائی_ہندسہ_چوڑائی)) {
    ثنائی_ہندسہ_چوڑائی_2 = New_ثنائی_ہندسہ_چوڑائی;
    أرجع;
  }

  // If we have an allocation, delete it.
  إذا (LLVM_UNLIKELY(!is_Single_لفظ()))
    مٹاؤ [] U.pVal;

  // Update BitWidth.
  ثنائی_ہندسہ_چوڑائی_2 = New_ثنائی_ہندسہ_چوڑائی;

  // If we are supposed to have an allocation, create it.
  إذا (LLVM_UNLIKELY(!is_Single_لفظ()))
    U.pVal = get_حافظہ(get_عدد_الفاظ());
}

فراغ APInt::سونپو_Slow_Case(ثابت APInt &RHS) {
  // Don't do anything for X = X
  إذا (هذا == &RHS)
    أرجع;

  // Adjust the bit width and handle allocations as necessary.
  reallocate(RHS.get_ثنائی_ہندسہ_چوڑائی());

  // Copy the data.
  إذا (LLVM_LIKELY(is_Single_لفظ()))
    U.VAL = RHS.U.VAL;
  وإلا
    حافظہ_نقل(U.pVal, RHS.U.pVal, get_عدد_الفاظ() * APINT_لفظ_ناپ);
}

/// This method 'profiles' an APInt for use with FoldingSet.
فراغ APInt::Profile(Folding_سیٹ_گرہ_ID& ID) ثابت {
  ID.جوڑو_Integer(ثنائی_ہندسہ_چوڑائی_2);

  إذا (LLVM_LIKELY(is_Single_لفظ())) {
    ID.جوڑو_Integer(U.VAL);
    أرجع;
  }

  بنانشانی عدد_الفاظ_2 = get_عدد_الفاظ();
  لكل (بنانشانی i = 0; i < عدد_الفاظ_2; ++i)
    ID.جوڑو_Integer(U.pVal[i]);
}

منطقي APInt::isAligned(Align A) ثابت {
  إذا (isZero())
    أرجع سچ;
  ثابت بنانشانی TrailingZeroes = countr_zero();
  ثابت بنانشانی MinimumTrailingZeroes = Log2(A);
  أرجع TrailingZeroes >= MinimumTrailingZeroes;
}

/// Prefix increment operator. Increments the APInt by one.
APInt& APInt::عامل++() {
  إذا (LLVM_LIKELY(is_Single_لفظ()))
    ++U.VAL;
  وإلا
    tcIncrement(U.pVal, get_عدد_الفاظ());
  أرجع clear_بن_ورتیا_بٹ();
}

/// Prefix decrement operator. Decrements the APInt by one.
APInt& APInt::عامل--() {
  إذا (LLVM_LIKELY(is_Single_لفظ()))
    --U.VAL;
  وإلا
    tcDecrement(U.pVal, get_عدد_الفاظ());
  أرجع clear_بن_ورتیا_بٹ();
}

/// Adds the RHS APInt to this APInt.
/// @returns this, after addition of RHS.
/// Addition assignment operator.
APInt& APInt::عامل+=(ثابت APInt& RHS) {
  أكد(ثنائی_ہندسہ_چوڑائی_2 == RHS.ثنائی_ہندسہ_چوڑائی_2 && "Bit widths must be the same");
  إذا (LLVM_LIKELY(is_Single_لفظ()))
    U.VAL += RHS.U.VAL;
  وإلا
    tc_جوڑو(U.pVal, RHS.U.pVal, 0, get_عدد_الفاظ());
  أرجع clear_بن_ورتیا_بٹ();
}

APInt& APInt::عامل+=(uint64_t RHS) {
  إذا (LLVM_LIKELY(is_Single_لفظ()))
    U.VAL += RHS;
  وإلا
    tc_جوڑو_Part(U.pVal, RHS, get_عدد_الفاظ());
  أرجع clear_بن_ورتیا_بٹ();
}

/// Subtracts the RHS APInt from this APInt
/// @returns this, after subtraction
/// Subtraction assignment operator.
APInt& APInt::عامل-=(ثابت APInt& RHS) {
  أكد(ثنائی_ہندسہ_چوڑائی_2 == RHS.ثنائی_ہندسہ_چوڑائی_2 && "Bit widths must be the same");
  إذا (LLVM_LIKELY(is_Single_لفظ()))
    U.VAL -= RHS.U.VAL;
  وإلا
    tc_گھٹاؤ(U.pVal, RHS.U.pVal, 0, get_عدد_الفاظ());
  أرجع clear_بن_ورتیا_بٹ();
}

APInt& APInt::عامل-=(uint64_t RHS) {
  إذا (LLVM_LIKELY(is_Single_لفظ()))
    U.VAL -= RHS;
  وإلا
    tc_گھٹاؤ_Part(U.pVal, RHS, get_عدد_الفاظ());
  أرجع clear_بن_ورتیا_بٹ();
}

APInt APInt::عامل*(ثابت APInt& RHS) ثابت {
  أكد(ثنائی_ہندسہ_چوڑائی_2 == RHS.ثنائی_ہندسہ_چوڑائی_2 && "Bit widths must be the same");
  إذا (LLVM_LIKELY(is_Single_لفظ()))
    أرجع APInt(ثنائی_ہندسہ_چوڑائی_2, U.VAL * RHS.U.VAL, /*isSigned=*/خطأ,
                 /*implicitTrunc=*/سچ);

  APInt نتیجہ(get_حافظہ(get_عدد_الفاظ()), get_ثنائی_ہندسہ_چوڑائی());
  tcMultiply(نتیجہ.U.pVal, U.pVal, RHS.U.pVal, get_عدد_الفاظ());
  نتیجہ.clear_بن_ورتیا_بٹ();
  أرجع نتیجہ;
}

فراغ APInt::and_سونپو_Slow_Case(ثابت APInt &RHS) {
  لفظ_Type *dst = U.pVal, *rhs = RHS.U.pVal;
  لكل (size_t i = 0, e = get_عدد_الفاظ(); i != e; ++i)
    dst[i] &= rhs[i];
}

فراغ APInt::or_سونپو_Slow_Case(ثابت APInt &RHS) {
  لفظ_Type *dst = U.pVal, *rhs = RHS.U.pVal;
  لكل (size_t i = 0, e = get_عدد_الفاظ(); i != e; ++i)
    dst[i] |= rhs[i];
}

فراغ APInt::xor_سونپو_Slow_Case(ثابت APInt &RHS) {
  لفظ_Type *dst = U.pVal, *rhs = RHS.U.pVal;
  لكل (size_t i = 0, e = get_عدد_الفاظ(); i != e; ++i)
    dst[i] ^= rhs[i];
}

APInt &APInt::عامل*=(ثابت APInt &RHS) {
  *هذا = *هذا * RHS;
  أرجع *هذا;
}

APInt& APInt::عامل*=(uint64_t RHS) {
  إذا (LLVM_LIKELY(is_Single_لفظ())) {
    U.VAL *= RHS;
  } وإلا {
    بنانشانی عدد_الفاظ_2 = get_عدد_الفاظ();
    tcMultiplyPart(U.pVal, U.pVal, RHS, 0, عدد_الفاظ_2, عدد_الفاظ_2, خطأ);
  }
  أرجع clear_بن_ورتیا_بٹ();
}

منطقي APInt::equalSlowCase(ثابت APInt &RHS) ثابت {
  أرجع std::equal(U.pVal, U.pVal + get_عدد_الفاظ(), RHS.U.pVal);
}

صحيح APInt::compare(ثابت APInt& RHS) ثابت {
  أكد(ثنائی_ہندسہ_چوڑائی_2 == RHS.ثنائی_ہندسہ_چوڑائی_2 && "Bit widths must be same for comparison");
  إذا (LLVM_LIKELY(is_Single_لفظ()))
    أرجع U.VAL < RHS.U.VAL ? -1 : U.VAL > RHS.U.VAL;

  أرجع tcCompare(U.pVal, RHS.U.pVal, get_عدد_الفاظ());
}

صحيح APInt::compareSigned(ثابت APInt& RHS) ثابت {
  أكد(ثنائی_ہندسہ_چوڑائی_2 == RHS.ثنائی_ہندسہ_چوڑائی_2 && "Bit widths must be same for comparison");
  إذا (LLVM_LIKELY(is_Single_لفظ())) {
    int64_t lhsSext = SignExtend64(U.VAL, ثنائی_ہندسہ_چوڑائی_2);
    int64_t rhsSext = SignExtend64(RHS.U.VAL, ثنائی_ہندسہ_چوڑائی_2);
    أرجع lhsSext < rhsSext ? -1 : lhsSext > rhsSext;
  }

  منطقي lhsNeg = isNegative();
  منطقي rhsNeg = RHS.isNegative();

  // If the sign bits don't match, then (LHS < RHS) if LHS is negative
  إذا (lhsNeg != rhsNeg)
    أرجع lhsNeg ? -1 : 1;

  // Otherwise we can just use an unsigned comparison, because even negative
  // numbers compare correctly this way if both have the same signed-ness.
  أرجع tcCompare(U.pVal, RHS.U.pVal, get_عدد_الفاظ());
}

فراغ APInt::سیٹ_بٹ_Slow_Case(بنانشانی lo_ثنائی_ہندسہ, بنانشانی hi_ثنائی_ہندسہ) {
  بنانشانی lo_لفظ = which_لفظ(lo_ثنائی_ہندسہ);
  بنانشانی hi_لفظ = which_لفظ(hi_ثنائی_ہندسہ);

  // Create an initial mask for the low word with zeros below loBit.
  uint64_t lo_نقاب = WORDTYPE_MAX << which_ثنائی_ہندسہ(lo_ثنائی_ہندسہ);

  // If hiBit is not aligned, we need a high mask.
  بنانشانی hi_کھسکاؤ_Amt = which_ثنائی_ہندسہ(hi_ثنائی_ہندسہ);
  إذا (hi_کھسکاؤ_Amt != 0) {
    // Create a high mask with zeros above hiBit.
    uint64_t hi_نقاب = WORDTYPE_MAX >> (APINT_بٹ_فی_لفظ - hi_کھسکاؤ_Amt);
    // If loWord and hiWord are equal, then we combine the masks. Otherwise,
    // set the bits in hiWord.
    إذا (hi_لفظ == lo_لفظ)
      lo_نقاب &= hi_نقاب;
    وإلا
      U.pVal[hi_لفظ] |= hi_نقاب;
  }
  // Apply the mask to the low word.
  U.pVal[lo_لفظ] |= lo_نقاب;

  // Fill any words between loWord and hiWord with all ones.
  لكل (بنانشانی لفظ = lo_لفظ + 1; لفظ < hi_لفظ; ++لفظ)
    U.pVal[لفظ] = WORDTYPE_MAX;
}

فراغ APInt::clear_بٹ_Slow_Case(بنانشانی Lo_ثنائی_ہندسہ, بنانشانی Hi_ثنائی_ہندسہ) {
  بنانشانی Lo_لفظ = which_لفظ(Lo_ثنائی_ہندسہ);
  بنانشانی Hi_لفظ = which_لفظ(Hi_ثنائی_ہندسہ);

  // Create an initial mask for the low word with ones below loBit.
  uint64_t Lo_نقاب = ~(WORDTYPE_MAX << which_ثنائی_ہندسہ(Lo_ثنائی_ہندسہ));

  // If HiBit is not aligned, we need a high mask.
  بنانشانی Hi_کھسکاؤ_Amt = which_ثنائی_ہندسہ(Hi_ثنائی_ہندسہ);
  إذا (Hi_کھسکاؤ_Amt != 0) {
    // Create a high mask with ones above HiBit.
    uint64_t Hi_نقاب = ~(WORDTYPE_MAX >> (APINT_بٹ_فی_لفظ - Hi_کھسکاؤ_Amt));
    // If LoWord and HiWord are equal, then we combine the masks. Otherwise,
    // clear the bits in HiWord.
    إذا (Hi_لفظ == Lo_لفظ)
      Lo_نقاب |= Hi_نقاب;
    وإلا
      U.pVal[Hi_لفظ] &= Hi_نقاب;
  }
  // Apply the mask to the low word.
  U.pVal[Lo_لفظ] &= Lo_نقاب;

  // Fill any words between LoWord and HiWord with all zeros.
  لكل (بنانشانی لفظ_2 = Lo_لفظ + 1; لفظ_2 < Hi_لفظ; ++لفظ_2)
    U.pVal[لفظ_2] = 0;
}

// Complement a bignum in-place.
ساكن فراغ tcComplement(APInt::لفظ_Type *dst, بنانشانی parts) {
  لكل (بنانشانی i = 0; i < parts; i++)
    dst[i] = ~dst[i];
}

/// Toggle every bit to its opposite value.
فراغ APInt::flip_All_بٹ_Slow_Case() {
  tcComplement(U.pVal, get_عدد_الفاظ());
  clear_بن_ورتیا_بٹ();
}

/// Concatenate the bits from "NewLSB" onto the bottom of *this.  This is
/// equivalent to:
///   (this->zext(NewWidth) << NewLSB.getBitWidth()) | NewLSB.zext(NewWidth)
/// In the slow case, we know the result is large.
APInt APInt::concatSlowCase(ثابت APInt &NewLSB) ثابت {
  بنانشانی New_چوڑائی = get_ثنائی_ہندسہ_چوڑائی() + NewLSB.get_ثنائی_ہندسہ_چوڑائی();
  APInt نتیجہ = NewLSB.zext(New_چوڑائی);
  نتیجہ.داخل_کرو_بٹ(*هذا, NewLSB.get_ثنائی_ہندسہ_چوڑائی());
  أرجع نتیجہ;
}

/// Toggle a given bit to its opposite value whose position is given
/// as "bitPosition".
/// Toggles a given bit to its opposite value.
فراغ APInt::flip_ثنائی_ہندسہ(بنانشانی ثنائی_ہندسہ_Position) {
  أكد(ثنائی_ہندسہ_Position < ثنائی_ہندسہ_چوڑائی_2 && "Out of the bit-width range!");
  سیٹ_ثنائی_ہندسہ_Val(ثنائی_ہندسہ_Position, !(*هذا)[ثنائی_ہندسہ_Position]);
}

فراغ APInt::داخل_کرو_بٹ(ثابت APInt &sub_بٹ, بنانشانی ثنائی_ہندسہ_Position) {
  بنانشانی sub_ثنائی_ہندسہ_چوڑائی = sub_بٹ.get_ثنائی_ہندسہ_چوڑائی();
  أكد((sub_ثنائی_ہندسہ_چوڑائی + ثنائی_ہندسہ_Position) <= ثنائی_ہندسہ_چوڑائی_2 && "Illegal bit insertion");

  // inserting no bits is a noop.
  إذا (sub_ثنائی_ہندسہ_چوڑائی == 0)
    أرجع;

  // Insertion is a direct copy.
  إذا (sub_ثنائی_ہندسہ_چوڑائی == ثنائی_ہندسہ_چوڑائی_2) {
    *هذا = sub_بٹ;
    أرجع;
  }

  // Single word result can be done as a direct bitmask.
  إذا (LLVM_LIKELY(is_Single_لفظ())) {
    uint64_t نقاب_2 = WORDTYPE_MAX >> (APINT_بٹ_فی_لفظ - sub_ثنائی_ہندسہ_چوڑائی);
    U.VAL &= ~(نقاب_2 << ثنائی_ہندسہ_Position);
    U.VAL |= (sub_بٹ.U.VAL << ثنائی_ہندسہ_Position);
    أرجع;
  }

  بنانشانی lo_ثنائی_ہندسہ = which_ثنائی_ہندسہ(ثنائی_ہندسہ_Position);
  بنانشانی lo_لفظ = which_لفظ(ثنائی_ہندسہ_Position);
  بنانشانی hi1_لفظ = which_لفظ(ثنائی_ہندسہ_Position + sub_ثنائی_ہندسہ_چوڑائی - 1);

  // Insertion within a single word can be done as a direct bitmask.
  إذا (lo_لفظ == hi1_لفظ) {
    uint64_t نقاب_2 = WORDTYPE_MAX >> (APINT_بٹ_فی_لفظ - sub_ثنائی_ہندسہ_چوڑائی);
    U.pVal[lo_لفظ] &= ~(نقاب_2 << lo_ثنائی_ہندسہ);
    U.pVal[lo_لفظ] |= (sub_بٹ.U.VAL << lo_ثنائی_ہندسہ);
    أرجع;
  }

  // Insert on word boundaries.
  إذا (lo_ثنائی_ہندسہ == 0) {
    // Direct copy whole words.
    بنانشانی عدد_سارا_Sub_الفاظ = sub_ثنائی_ہندسہ_چوڑائی / APINT_بٹ_فی_لفظ;
    حافظہ_نقل(U.pVal + lo_لفظ, sub_بٹ.get_Raw_انکڑے(),
           عدد_سارا_Sub_الفاظ * APINT_لفظ_ناپ);

    // Mask+insert remaining bits.
    بنانشانی بچیا_بٹ = sub_ثنائی_ہندسہ_چوڑائی % APINT_بٹ_فی_لفظ;
    إذا (بچیا_بٹ != 0) {
      uint64_t نقاب_2 = WORDTYPE_MAX >> (APINT_بٹ_فی_لفظ - بچیا_بٹ);
      U.pVal[hi1_لفظ] &= ~نقاب_2;
      U.pVal[hi1_لفظ] |= sub_بٹ.get_لفظ(sub_ثنائی_ہندسہ_چوڑائی - 1);
    }
    أرجع;
  }

  // General case - set/clear individual bits in dst based on src.
  // TODO - there is scope for optimization here, but at the moment this code
  // path is barely used so prefer readability over performance.
  لكل (بنانشانی i = 0; i != sub_ثنائی_ہندسہ_چوڑائی; ++i)
    سیٹ_ثنائی_ہندسہ_Val(ثنائی_ہندسہ_Position + i, sub_بٹ[i]);
}

فراغ APInt::داخل_کرو_بٹ(uint64_t sub_بٹ, بنانشانی ثنائی_ہندسہ_Position, بنانشانی عدد_بٹ) {
  uint64_t نقاب_بٹ = نقاب_Trailing_Ones<uint64_t>(عدد_بٹ);
  sub_بٹ &= نقاب_بٹ;
  إذا (LLVM_LIKELY(is_Single_لفظ())) {
    U.VAL &= ~(نقاب_بٹ << ثنائی_ہندسہ_Position);
    U.VAL |= sub_بٹ << ثنائی_ہندسہ_Position;
    أرجع;
  }

  بنانشانی lo_ثنائی_ہندسہ = which_ثنائی_ہندسہ(ثنائی_ہندسہ_Position);
  بنانشانی lo_لفظ = which_لفظ(ثنائی_ہندسہ_Position);
  بنانشانی hi_لفظ = which_لفظ(ثنائی_ہندسہ_Position + عدد_بٹ - 1);
  إذا (lo_لفظ == hi_لفظ) {
    U.pVal[lo_لفظ] &= ~(نقاب_بٹ << lo_ثنائی_ہندسہ);
    U.pVal[lo_لفظ] |= sub_بٹ << lo_ثنائی_ہندسہ;
    أرجع;
  }

  static_assert(8 * حجم(لفظ_Type) <= 64, "This code assumes only two words affected");
  بنانشانی لفظ_بٹ = 8 * حجم(لفظ_Type);
  U.pVal[lo_لفظ] &= ~(نقاب_بٹ << lo_ثنائی_ہندسہ);
  U.pVal[lo_لفظ] |= sub_بٹ << lo_ثنائی_ہندسہ;

  U.pVal[hi_لفظ] &= ~(نقاب_بٹ >> (لفظ_بٹ - lo_ثنائی_ہندسہ));
  U.pVal[hi_لفظ] |= sub_بٹ >> (لفظ_بٹ - lo_ثنائی_ہندسہ);
}

APInt APInt::کڈھو_بٹ(بنانشانی عدد_بٹ, بنانشانی ثنائی_ہندسہ_Position) ثابت {
  أكد(ثنائی_ہندسہ_Position < ثنائی_ہندسہ_چوڑائی_2 && (عدد_بٹ + ثنائی_ہندسہ_Position) <= ثنائی_ہندسہ_چوڑائی_2 &&
         "Illegal bit extraction");

  إذا (LLVM_LIKELY(is_Single_لفظ()))
    أرجع APInt(عدد_بٹ, U.VAL >> ثنائی_ہندسہ_Position, /*isSigned=*/خطأ,
                 /*implicitTrunc=*/سچ);

  بنانشانی lo_ثنائی_ہندسہ = which_ثنائی_ہندسہ(ثنائی_ہندسہ_Position);
  بنانشانی lo_لفظ = which_لفظ(ثنائی_ہندسہ_Position);
  بنانشانی hi_لفظ = which_لفظ(ثنائی_ہندسہ_Position + عدد_بٹ - 1);

  // Single word result extracting bits from a single word source.
  إذا (lo_لفظ == hi_لفظ)
    أرجع APInt(عدد_بٹ, U.pVal[lo_لفظ] >> lo_ثنائی_ہندسہ, /*isSigned=*/خطأ,
                 /*implicitTrunc=*/سچ);

  // Extracting bits that start on a source word boundary can be done
  // as a fast memory copy.
  إذا (lo_ثنائی_ہندسہ == 0)
    أرجع APInt(عدد_بٹ, ArrayRef(U.pVal + lo_لفظ, 1 + hi_لفظ - lo_لفظ));

  // General case - shift + copy source words directly into place.
  APInt نتیجہ(عدد_بٹ, 0);
  بنانشانی عدد_Src_الفاظ = get_عدد_الفاظ();
  بنانشانی عدد_Dst_الفاظ = نتیجہ.get_عدد_الفاظ();

  uint64_t *DestPtr =
      LLVM_LIKELY(نتیجہ.is_Single_لفظ()) ? &نتیجہ.U.VAL : نتیجہ.U.pVal;
  لكل (بنانشانی لفظ = 0; لفظ < عدد_Dst_الفاظ; ++لفظ) {
    uint64_t w0 = U.pVal[lo_لفظ + لفظ];
    uint64_t w1 =
        (lo_لفظ + لفظ + 1) < عدد_Src_الفاظ ? U.pVal[lo_لفظ + لفظ + 1] : 0;
    DestPtr[لفظ] = (w0 >> lo_ثنائی_ہندسہ) | (w1 << (APINT_بٹ_فی_لفظ - lo_ثنائی_ہندسہ));
  }

  أرجع نتیجہ.clear_بن_ورتیا_بٹ();
}

uint64_t APInt::کڈھو_بٹ_As_ZExt_قدر(بنانشانی عدد_بٹ,
                                       بنانشانی ثنائی_ہندسہ_Position) ثابت {
  أكد(ثنائی_ہندسہ_Position < ثنائی_ہندسہ_چوڑائی_2 && (عدد_بٹ + ثنائی_ہندسہ_Position) <= ثنائی_ہندسہ_چوڑائی_2 &&
         "Illegal bit extraction");
  أكد(عدد_بٹ <= 64 && "Illegal bit extraction");

  uint64_t نقاب_بٹ = نقاب_Trailing_Ones<uint64_t>(عدد_بٹ);
  إذا (LLVM_LIKELY(is_Single_لفظ()))
    أرجع (U.VAL >> ثنائی_ہندسہ_Position) & نقاب_بٹ;

  static_assert(APINT_بٹ_فی_لفظ >= 64,
                "This code assumes only two words affected");
  بنانشانی lo_ثنائی_ہندسہ = which_ثنائی_ہندسہ(ثنائی_ہندسہ_Position);
  بنانشانی lo_لفظ = which_لفظ(ثنائی_ہندسہ_Position);
  بنانشانی hi_لفظ = which_لفظ(ثنائی_ہندسہ_Position + عدد_بٹ - 1);
  إذا (lo_لفظ == hi_لفظ)
    أرجع (U.pVal[lo_لفظ] >> lo_ثنائی_ہندسہ) & نقاب_بٹ;

  uint64_t ret_بٹ = U.pVal[lo_لفظ] >> lo_ثنائی_ہندسہ;
  ret_بٹ |= U.pVal[hi_لفظ] << (APINT_بٹ_فی_لفظ - lo_ثنائی_ہندسہ);
  ret_بٹ &= نقاب_بٹ;
  أرجع ret_بٹ;
}

بنانشانی APInt::get_Sufficient_بٹ_Needed(StringRef Str, uint8_t Radix) {
  أكد(!Str.خالی_تھاواں() && "Invalid string length");
  size_t StrLen = Str.ناپ_2();

  // Each computation below needs to know if it's negative.
  بنانشانی IsNegative = خطأ;
  إذا (Str[0] == '-' || Str[0] == '+') {
    IsNegative = Str[0] == '-';
    StrLen--;
    أكد(StrLen && "String is only a sign, needs a value.");
  }

  // For radixes of power-of-two values, the bits required is accurately and
  // easily computed.
  إذا (Radix == 2)
    أرجع StrLen + IsNegative;
  إذا (Radix == 8)
    أرجع StrLen * 3 + IsNegative;
  إذا (Radix == 16)
    أرجع StrLen * 4 + IsNegative;

  // Compute a sufficient number of bits that is always large enough but might
  // be too large. This avoids the assertion in the constructor. This
  // calculation doesn't work appropriately for the numbers 0-9, so just use 4
  // bits in that case.
  إذا (Radix == 10)
    أرجع (StrLen == 1 ? 4 : StrLen * 64 / 18) + IsNegative;

  أكد(Radix == 36);
  أرجع (StrLen == 1 ? 7 : StrLen * 16 / 3) + IsNegative;
}

بنانشانی APInt::get_بٹ_Needed(StringRef str, uint8_t radix) {
  // Compute a sufficient number of bits that is always large enough but might
  // be too large.
  بنانشانی sufficient = get_Sufficient_بٹ_Needed(str, radix);

  // For bases 2, 8, and 16, the sufficient number of bits is exact and we can
  // return the value directly. For bases 10 and 36, we need to do extra work.
  إذا (radix == 2 || radix == 8 || radix == 16)
    أرجع sufficient;

  // This is grossly inefficient but accurate. We could probably do something
  // with a computation of roughly slen*64/20 and then adjust by the value of
  // the first few digits. But, I'm not sure how accurate that could be.
  size_t slen = str.ناپ_2();

  // Each computation below needs to know if it's negative.
  StringRef::iterator p = str.begin();
  بنانشانی isNegative = *p == '-';
  إذا (*p == '-' || *p == '+') {
    p++;
    slen--;
    أكد(slen && "String is only a sign, needs a value.");
  }


  // Convert to the actual binary value.
  APInt tmp(sufficient, StringRef(p, slen), radix);

  // Compute how many bits are required. If the log is infinite, assume we need
  // just bit. If the log is exact and value is negative, then the value is
  // MinSignedValue with (log + 1) bits.
  بنانشانی لوگارتھم = tmp.logBase2();
  إذا (لوگارتھم == (بنانشانی)-1) {
    أرجع isNegative + 1;
  } وإلا إذا (isNegative && tmp.isPowerOf2()) {
    أرجع isNegative + لوگارتھم;
  } وإلا {
    أرجع isNegative + لوگارتھم + 1;
  }
}

hash_کوڈ llvm::hash_قدر(ثابت APInt &Arg) {
  إذا (LLVM_LIKELY(Arg.is_Single_لفظ()))
    أرجع hash_combine(Arg.ثنائی_ہندسہ_چوڑائی_2, Arg.U.VAL);

  أرجع hash_combine(
      Arg.ثنائی_ہندسہ_چوڑائی_2,
      hash_combine_range(Arg.U.pVal, Arg.U.pVal + Arg.get_عدد_الفاظ()));
}

بنانشانی DenseMapInfo<APInt, فراغ>::get_Hash_قدر(ثابت APInt &کلید) {
  أرجع static_cast<بنانشانی>(hash_قدر(کلید));
}

منطقي APInt::isSplat(بنانشانی Splat_ناپ_وچ_بٹ) ثابت {
  أكد(get_ثنائی_ہندسہ_چوڑائی() % Splat_ناپ_وچ_بٹ == 0 &&
         "SplatSizeInBits must divide width!");
  // We can check that all parts of an integer are equal by making use of a
  // little trick: rotate and check if it's still the same value.
  أرجع *هذا == rotl(Splat_ناپ_وچ_بٹ);
}

/// This function returns the high "numBits" bits of this APInt.
APInt APInt::get_Hi_بٹ(بنانشانی عدد_بٹ) ثابت {
  أرجع هذا->lshr(ثنائی_ہندسہ_چوڑائی_2 - عدد_بٹ);
}

/// This function returns the low "numBits" bits of this APInt.
APInt APInt::get_Lo_بٹ(بنانشانی عدد_بٹ) ثابت {
  APInt نتیجہ(get_کم_بٹ_سیٹ(ثنائی_ہندسہ_چوڑائی_2, عدد_بٹ));
  نتیجہ &= *هذا;
  أرجع نتیجہ;
}

/// Return a value containing V broadcasted over NewLen bits.
APInt APInt::getSplat(بنانشانی NewLen, ثابت APInt &V) {
  أكد(NewLen >= V.get_ثنائی_ہندسہ_چوڑائی() && "Can't splat to smaller bit width!");

  APInt Val = V.zext(NewLen);
  لكل (بنانشانی I = V.get_ثنائی_ہندسہ_چوڑائی(); I < NewLen; I <<= 1)
    Val |= Val << I;

  أرجع Val;
}

بنانشانی APInt::گنتی_Leading_Zeros_Slow_Case() ثابت {
  بنانشانی گنتی_2 = 0;
  لكل (صحيح i = get_عدد_الفاظ() - 1; i >= 0; --i) {
    uint64_t V = U.pVal[i];
    إذا (V == 0)
      گنتی_2 += APINT_بٹ_فی_لفظ;
    وإلا {
      گنتی_2 += llvm::countl_zero(V);
      اكسر;
    }
  }
  // Adjust for unused bits in the most significant word (they are zero).
  بنانشانی Mod = ثنائی_ہندسہ_چوڑائی_2 % APINT_بٹ_فی_لفظ;
  گنتی_2 -= Mod > 0 ? APINT_بٹ_فی_لفظ - Mod : 0;
  أرجع گنتی_2;
}

بنانشانی APInt::گنتی_Leading_Ones_Slow_Case() ثابت {
  بنانشانی بلند_لفظ_بٹ = ثنائی_ہندسہ_چوڑائی_2 % APINT_بٹ_فی_لفظ;
  بنانشانی کھسکاؤ;
  إذا (!بلند_لفظ_بٹ) {
    بلند_لفظ_بٹ = APINT_بٹ_فی_لفظ;
    کھسکاؤ = 0;
  } وإلا {
    کھسکاؤ = APINT_بٹ_فی_لفظ - بلند_لفظ_بٹ;
  }
  صحيح i = get_عدد_الفاظ() - 1;
  بنانشانی گنتی_2 = llvm::countl_one(U.pVal[i] << کھسکاؤ);
  إذا (گنتی_2 == بلند_لفظ_بٹ) {
    لكل (i--; i >= 0; --i) {
      إذا (U.pVal[i] == WORDTYPE_MAX)
        گنتی_2 += APINT_بٹ_فی_لفظ;
      وإلا {
        گنتی_2 += llvm::countl_one(U.pVal[i]);
        اكسر;
      }
    }
  }
  أرجع گنتی_2;
}

بنانشانی APInt::گنتی_Trailing_Zeros_Slow_Case() ثابت {
  بنانشانی گنتی_2 = 0;
  بنانشانی i = 0;
  لكل (; i < get_عدد_الفاظ() && U.pVal[i] == 0; ++i)
    گنتی_2 += APINT_بٹ_فی_لفظ;
  إذا (i < get_عدد_الفاظ())
    گنتی_2 += llvm::countr_zero(U.pVal[i]);
  أرجع std::min(گنتی_2, ثنائی_ہندسہ_چوڑائی_2);
}

بنانشانی APInt::گنتی_Trailing_Ones_Slow_Case() ثابت {
  بنانشانی گنتی_2 = 0;
  بنانشانی i = 0;
  لكل (; i < get_عدد_الفاظ() && U.pVal[i] == WORDTYPE_MAX; ++i)
    گنتی_2 += APINT_بٹ_فی_لفظ;
  إذا (i < get_عدد_الفاظ())
    گنتی_2 += llvm::countr_one(U.pVal[i]);
  أكد(گنتی_2 <= ثنائی_ہندسہ_چوڑائی_2);
  أرجع گنتی_2;
}

بنانشانی APInt::گنتی_Population_Slow_Case() ثابت {
  بنانشانی گنتی_2 = 0;
  لكل (بنانشانی i = 0; i < get_عدد_الفاظ(); ++i)
    گنتی_2 += llvm::popcount(U.pVal[i]);
  أرجع گنتی_2;
}

منطقي APInt::isPowerOf2SlowCase() ثابت {
  بنانشانی گنتی_2 = 0;
  لكل (بنانشانی i = 0; i < get_عدد_الفاظ(); ++i) {
    گنتی_2 += llvm::popcount(U.pVal[i]);
    إذا (گنتی_2 > 1)
      أرجع خطأ;
  }
  أرجع گنتی_2 == 1;
}

منطقي APInt::intersectsSlowCase(ثابت APInt &RHS) ثابت {
  لكل (بنانشانی i = 0, e = get_عدد_الفاظ(); i != e; ++i)
    إذا ((U.pVal[i] & RHS.U.pVal[i]) != 0)
      أرجع سچ;

  أرجع خطأ;
}

منطقي APInt::is_Subset_دا_Slow_Case(ثابت APInt &RHS) ثابت {
  لكل (بنانشانی i = 0, e = get_عدد_الفاظ(); i != e; ++i)
    إذا ((U.pVal[i] & ~RHS.U.pVal[i]) != 0)
      أرجع خطأ;

  أرجع سچ;
}

منطقي APInt::is_Inverse_دا_Slow_Case(ثابت APInt &RHS) ثابت {
  ثابت بنانشانی آخری = get_عدد_الفاظ() - 1;
  لكل (بنانشانی I = 0; I != آخری; ++I)
    إذا ((U.pVal[I] ^ RHS.U.pVal[I]) != WORDTYPE_MAX)
      أرجع خطأ;

  بنانشانی پوچھ_بٹ = ثنائی_ہندسہ_چوڑائی_2 - آخری * APINT_بٹ_فی_لفظ;
  لفظ_Type پوچھ_نقاب = llvm::نقاب_Trailing_Ones<لفظ_Type>(پوچھ_بٹ);
  أرجع (U.pVal[آخری] ^ RHS.U.pVal[آخری]) == پوچھ_نقاب;
}

APInt APInt::byte_وٹاندرا() ثابت {
  أكد(ثنائی_ہندسہ_چوڑائی_2 >= 16 && ثنائی_ہندسہ_چوڑائی_2 % 8 == 0 && "Cannot byteswap!");
  إذا (ثنائی_ہندسہ_چوڑائی_2 == 16)
    أرجع APInt(ثنائی_ہندسہ_چوڑائی_2, llvm::byteswap<uint16_t>(U.VAL));
  إذا (ثنائی_ہندسہ_چوڑائی_2 == 32)
    أرجع APInt(ثنائی_ہندسہ_چوڑائی_2, llvm::byteswap<uint32_t>(U.VAL));
  إذا (ثنائی_ہندسہ_چوڑائی_2 <= 64) {
    uint64_t Tmp1 = llvm::byteswap<uint64_t>(U.VAL);
    Tmp1 >>= (64 - ثنائی_ہندسہ_چوڑائی_2);
    أرجع APInt(ثنائی_ہندسہ_چوڑائی_2, Tmp1);
  }

  APInt نتیجہ(get_عدد_الفاظ() * APINT_بٹ_فی_لفظ, 0);
  لكل (بنانشانی I = 0, N = get_عدد_الفاظ(); I != N; ++I)
    نتیجہ.U.pVal[I] = llvm::byteswap<uint64_t>(U.pVal[N - I - 1]);
  إذا (نتیجہ.ثنائی_ہندسہ_چوڑائی_2 != ثنائی_ہندسہ_چوڑائی_2) {
    نتیجہ.lshr_وچ_رکھو(نتیجہ.ثنائی_ہندسہ_چوڑائی_2 - ثنائی_ہندسہ_چوڑائی_2);
    نتیجہ.ثنائی_ہندسہ_چوڑائی_2 = ثنائی_ہندسہ_چوڑائی_2;
  }
  أرجع نتیجہ;
}

APInt APInt::الٹ_بٹ() ثابت {
  إذا (LLVM_LIKELY(is_Single_لفظ())) {
    بدل (ثنائی_ہندسہ_چوڑائی_2) {
    حالة 64:
      أرجع APInt(ثنائی_ہندسہ_چوڑائی_2, llvm::الٹ_بٹ<uint64_t>(U.VAL));
    حالة 32:
      أرجع APInt(ثنائی_ہندسہ_چوڑائی_2, llvm::الٹ_بٹ<uint32_t>(U.VAL));
    حالة 16:
      أرجع APInt(ثنائی_ہندسہ_چوڑائی_2, llvm::الٹ_بٹ<uint16_t>(U.VAL));
    حالة 8:
      أرجع APInt(ثنائی_ہندسہ_چوڑائی_2, llvm::الٹ_بٹ<uint8_t>(U.VAL));
    حالة 1: // fallthrough
    حالة 0:
      أرجع *هذا;
    افتراضي:
      أرجع APInt(ثنائی_ہندسہ_چوڑائی_2,
                   llvm::الٹ_بٹ<uint64_t>(U.VAL) >> (64 - ثنائی_ہندسہ_چوڑائی_2));
    }
  }

  APInt نتیجہ(ثنائی_ہندسہ_چوڑائی_2, 0);
  بنانشانی عدد_الفاظ_2 = get_عدد_الفاظ();
  بنانشانی Excess_بٹ = عدد_الفاظ_2 * APINT_بٹ_فی_لفظ - ثنائی_ہندسہ_چوڑائی_2;
  إذا (Excess_بٹ == 0) {
    // Fast path. No cross-word shift needed.
    لكل (بنانشانی I = 0; I < عدد_الفاظ_2; ++I)
      نتیجہ.U.pVal[I] = llvm::الٹ_بٹ<uint64_t>(U.pVal[عدد_الفاظ_2 - 1 - I]);
    أرجع نتیجہ;
  }
  // Holds reversed bits of the previous (more significant) word.
  uint64_t PrevRev = llvm::الٹ_بٹ<uint64_t>(U.pVal[عدد_الفاظ_2 - 1]);
  لكل (بنانشانی I = 0; I < عدد_الفاظ_2 - 1; ++I) {
    uint64_t CurrRev = llvm::الٹ_بٹ<uint64_t>(U.pVal[عدد_الفاظ_2 - 2 - I]);
    نتیجہ.U.pVal[I] = (PrevRev >> Excess_بٹ) | (CurrRev << (64 - Excess_بٹ));
    PrevRev = CurrRev;
  }
  نتیجہ.U.pVal[عدد_الفاظ_2 - 1] = PrevRev >> Excess_بٹ;
  أرجع نتیجہ;
}

APInt llvm::APIntOps::Greatest_Common_مقسوم_علیہ(APInt A, APInt B, منطقي IsSigned) {
  // Take absolute value if IsSigned.
  إذا (IsSigned) {
    A = A.مطلق_قدر();
    B = B.مطلق_قدر();
  }

  // Fast-path a common case.
  إذا (A == B) أرجع A;

  // Corner cases: if either operand is zero, the other is the gcd.
  إذا (!A) أرجع B;
  إذا (!B) أرجع A;

  // Count common powers of 2 and remove all other powers of 2.
  بنانشانی Pow2;
  {
    بنانشانی Pow2_A = A.countr_zero();
    بنانشانی Pow2_B = B.countr_zero();
    إذا (Pow2_A > Pow2_B) {
      A.lshr_وچ_رکھو(Pow2_A - Pow2_B);
      Pow2 = Pow2_B;
    } وإلا إذا (Pow2_B > Pow2_A) {
      B.lshr_وچ_رکھو(Pow2_B - Pow2_A);
      Pow2 = Pow2_A;
    } وإلا {
      Pow2 = Pow2_A;
    }
  }

  // Both operands are odd multiples of 2^Pow_2:
  //
  //   gcd(a, b) = gcd(|a - b| / 2^i, min(a, b))
  //
  // This is a modified version of Stein's algorithm, taking advantage of
  // efficient countTrailingZeros().
  طالما (A != B) {
    إذا (A.ugt(B)) {
      A -= B;
      A.lshr_وچ_رکھو(A.countr_zero() - Pow2);
    } وإلا {
      B -= A;
      B.lshr_وچ_رکھو(B.countr_zero() - Pow2);
    }
  }

  أرجع A;
}

APInt llvm::APIntOps::گیڑا_Double_منزل_APInt(مزدوج Double, بنانشانی چوڑائی_2) {
  uint64_t I = ثنائی_ہندسہ_cast<uint64_t>(Double);

  // Get the sign bit from the highest order bit
  منطقي isNeg = I >> 63;

  // Get the 11-bit exponent and adjust for the 1023 bit bias
  int64_t قوت_نما = ((I >> 52) & 0x7ff) - 1023;

  // If the exponent is negative, the value is < 0 so just return 0.
  إذا (قوت_نما < 0)
    أرجع APInt(چوڑائی_2, 0u);

  // Extract the mantissa by clearing the top 12 bits (sign + exponent).
  uint64_t mantissa = (I & (~0ULL >> 12)) | 1ULL << 52;

  // If the exponent doesn't shift all bits out of the mantissa
  إذا (قوت_نما < 52)
    أرجع isNeg ? -APInt(چوڑائی_2, mantissa >> (52 - قوت_نما)) :
                    APInt(چوڑائی_2, mantissa >> (52 - قوت_نما));

  // If the client didn't provide enough bits for us to shift the mantissa into
  // then the result is undefined, just return 0
  إذا (چوڑائی_2 <= قوت_نما - 52)
    أرجع APInt(چوڑائی_2, 0);

  // Otherwise, we have to shift the mantissa bits up to the right location
  APInt Tmp(چوڑائی_2, mantissa);
  Tmp <<= (بنانشانی)قوت_نما - 52;
  أرجع isNeg ? -Tmp : Tmp;
}

/// This function converts this APInt to a double.
/// The layout for double is as following (IEEE Standard 754):
///  --------------------------------------
/// |  Sign    Exponent    Fraction    Bias |
/// |-------------------------------------- |
/// |  1[63]   11[62-52]   52[51-00]   1023 |
///  --------------------------------------
مزدوج APInt::گیڑا_منزل_Double(منطقي isSigned) ثابت {
  // Handle the simple case where the value is contained in one uint64_t.
  // It is wrong to optimize getWord(0) to VAL; there might be more than one word.
  إذا (LLVM_LIKELY(is_Single_لفظ() || get_سرگرم_بٹ() <= APINT_بٹ_فی_لفظ)) {
    إذا (isSigned) {
      int64_t sext = SignExtend64(get_لفظ(0), ثنائی_ہندسہ_چوڑائی_2);
      أرجع مزدوج(sext);
    }
    أرجع مزدوج(get_لفظ(0));
  }

  // Determine if the value is negative.
  منطقي isNeg = isSigned ? (*هذا)[ثنائی_ہندسہ_چوڑائی_2-1] : خطأ;

  // Construct the absolute value if we're negative.
  APInt Tmp(isNeg ? -(*هذا) : (*هذا));

  // Figure out how many bits we're using.
  بنانشانی n = Tmp.get_سرگرم_بٹ();

  // The exponent (without bias normalization) is just the number of bits
  // we are using. Note that the sign bit is gone since we constructed the
  // absolute value.
  uint64_t قوت_نما = n;

  // Return infinity for exponent overflow
  إذا (قوت_نما > 1023) {
    إذا (!isSigned || !isNeg)
      أرجع std::numeric_limits<مزدوج>::infinity();
    وإلا
      أرجع -std::numeric_limits<مزدوج>::infinity();
  }
  قوت_نما += 1023; // Increment for 1023 bias

  // Number of bits in mantissa is 52. To obtain the mantissa value, we must
  // extract the high 52 bits from the correct words in pVal.
  uint64_t mantissa;
  بنانشانی hi_لفظ = which_لفظ(n-1);
  إذا (hi_لفظ == 0) {
    mantissa = Tmp.U.pVal[0];
    إذا (n > 52)
      mantissa >>= n - 52; // shift down, we want the top 52 bits.
  } وإلا {
    أكد(hi_لفظ > 0 && "huh?");
    uint64_t hibits = Tmp.U.pVal[hi_لفظ] << (52 - n % APINT_بٹ_فی_لفظ);
    uint64_t lobits = Tmp.U.pVal[hi_لفظ-1] >> (11 + n % APINT_بٹ_فی_لفظ);
    mantissa = hibits | lobits;
  }

  // The leading bit of mantissa is implicit, so get rid of it.
  uint64_t sign = isNeg ? (1ULL << (APINT_بٹ_فی_لفظ - 1)) : 0;
  uint64_t I = sign | (قوت_نما << 52) | mantissa;
  أرجع ثنائی_ہندسہ_cast<مزدوج>(I);
}

// Truncate to new width.
APInt APInt::trunc(بنانشانی چوڑائی_2) ثابت {
  أكد(چوڑائی_2 <= ثنائی_ہندسہ_چوڑائی_2 && "Invalid APInt Truncate request");

  إذا (چوڑائی_2 <= APINT_بٹ_فی_لفظ)
    أرجع APInt(چوڑائی_2, get_Raw_انکڑے()[0], /*isSigned=*/خطأ,
                 /*implicitTrunc=*/سچ);

  إذا (چوڑائی_2 == ثنائی_ہندسہ_چوڑائی_2)
    أرجع *هذا;

  APInt نتیجہ(get_حافظہ(get_عدد_الفاظ(چوڑائی_2)), چوڑائی_2);

  // Copy full words.
  بنانشانی i;
  لكل (i = 0; i != چوڑائی_2 / APINT_بٹ_فی_لفظ; i++)
    نتیجہ.U.pVal[i] = U.pVal[i];

  // Truncate and copy any partial word.
  بنانشانی بٹ = (0 - چوڑائی_2) % APINT_بٹ_فی_لفظ;
  إذا (بٹ != 0)
    نتیجہ.U.pVal[i] = U.pVal[i] << بٹ >> بٹ;

  أرجع نتیجہ;
}

// Truncate to new width with unsigned saturation.
APInt APInt::truncUSat(بنانشانی چوڑائی_2) ثابت {
  أكد(چوڑائی_2 <= ثنائی_ہندسہ_چوڑائی_2 && "Invalid APInt Truncate request");

  // Can we just losslessly truncate it?
  إذا (isIntN(چوڑائی_2))
    أرجع trunc(چوڑائی_2);
  // If not, then just return the new limit.
  أرجع APInt::get_Max_قدر(چوڑائی_2);
}

// Truncate to new width with signed saturation to signed result.
APInt APInt::truncSSat(بنانشانی چوڑائی_2) ثابت {
  أكد(چوڑائی_2 <= ثنائی_ہندسہ_چوڑائی_2 && "Invalid APInt Truncate request");

  // Can we just losslessly truncate it?
  إذا (isSignedIntN(چوڑائی_2))
    أرجع trunc(چوڑائی_2);
  // If not, then just return the new limits.
  أرجع isNegative() ? APInt::get_Signed_Min_قدر(چوڑائی_2)
                      : APInt::get_Signed_Max_قدر(چوڑائی_2);
}

// Truncate to new width with signed saturation to unsigned result.
APInt APInt::truncSSatU(بنانشانی چوڑائی_2) ثابت {
  أكد(چوڑائی_2 <= ثنائی_ہندسہ_چوڑائی_2 && "Invalid APInt Truncate request");

  // Can we just losslessly truncate it?
  إذا (isIntN(چوڑائی_2))
    أرجع trunc(چوڑائی_2);
  // If not, then just return the new limits.
  أرجع isNegative() ? APInt::getZero(چوڑائی_2) : APInt::get_Max_قدر(چوڑائی_2);
}

// Sign extend to a new width.
APInt APInt::sext(بنانشانی چوڑائی) ثابت {
  أكد(چوڑائی >= ثنائی_ہندسہ_چوڑائی_2 && "Invalid APInt SignExtend request");

  إذا (چوڑائی <= APINT_بٹ_فی_لفظ)
    أرجع APInt(چوڑائی, SignExtend64(U.VAL, ثنائی_ہندسہ_چوڑائی_2), /*isSigned=*/سچ);

  إذا (چوڑائی == ثنائی_ہندسہ_چوڑائی_2)
    أرجع *هذا;

  APInt نتیجہ(get_حافظہ(get_عدد_الفاظ(چوڑائی)), چوڑائی);

  // Copy words.
  std::حافظہ_نقل(نتیجہ.U.pVal, get_Raw_انکڑے(), get_عدد_الفاظ() * APINT_لفظ_ناپ);

  // Sign extend the last word since there may be unused bits in the input.
  نتیجہ.U.pVal[get_عدد_الفاظ() - 1] =
      SignExtend64(نتیجہ.U.pVal[get_عدد_الفاظ() - 1],
                   ((ثنائی_ہندسہ_چوڑائی_2 - 1) % APINT_بٹ_فی_لفظ) + 1);

  // Fill with sign bits.
  std::memset(نتیجہ.U.pVal + get_عدد_الفاظ(), isNegative() ? -1 : 0,
              (نتیجہ.get_عدد_الفاظ() - get_عدد_الفاظ()) * APINT_لفظ_ناپ);
  نتیجہ.clear_بن_ورتیا_بٹ();
  أرجع نتیجہ;
}

//  Zero extend to a new width.
APInt APInt::zext(بنانشانی چوڑائی_2) ثابت {
  أكد(چوڑائی_2 >= ثنائی_ہندسہ_چوڑائی_2 && "Invalid APInt ZeroExtend request");

  إذا (چوڑائی_2 <= APINT_بٹ_فی_لفظ)
    أرجع APInt(چوڑائی_2, U.VAL);

  إذا (چوڑائی_2 == ثنائی_ہندسہ_چوڑائی_2)
    أرجع *هذا;

  APInt نتیجہ(get_حافظہ(get_عدد_الفاظ(چوڑائی_2)), چوڑائی_2);

  // Copy words.
  std::حافظہ_نقل(نتیجہ.U.pVal, get_Raw_انکڑے(), get_عدد_الفاظ() * APINT_لفظ_ناپ);

  // Zero remaining words.
  std::memset(نتیجہ.U.pVal + get_عدد_الفاظ(), 0,
              (نتیجہ.get_عدد_الفاظ() - get_عدد_الفاظ()) * APINT_لفظ_ناپ);

  أرجع نتیجہ;
}

APInt APInt::zextOrTrunc(بنانشانی چوڑائی_2) ثابت {
  إذا (ثنائی_ہندسہ_چوڑائی_2 < چوڑائی_2)
    أرجع zext(چوڑائی_2);
  إذا (ثنائی_ہندسہ_چوڑائی_2 > چوڑائی_2)
    أرجع trunc(چوڑائی_2);
  أرجع *هذا;
}

APInt APInt::sextOrTrunc(بنانشانی چوڑائی_2) ثابت {
  إذا (ثنائی_ہندسہ_چوڑائی_2 < چوڑائی_2)
    أرجع sext(چوڑائی_2);
  إذا (ثنائی_ہندسہ_چوڑائی_2 > چوڑائی_2)
    أرجع trunc(چوڑائی_2);
  أرجع *هذا;
}

/// Arithmetic right-shift this APInt by shiftAmt.
/// Arithmetic right-shift function.
فراغ APInt::ashr_وچ_رکھو(ثابت APInt &کھسکاؤ_Amt) {
  ashr_وچ_رکھو((بنانشانی)کھسکاؤ_Amt.get_Limited_قدر(ثنائی_ہندسہ_چوڑائی_2));
}

/// Arithmetic right-shift this APInt by shiftAmt.
/// Arithmetic right-shift function.
فراغ APInt::ashrSlowCase(بنانشانی کھسکاؤ_Amt_2) {
  // Don't bother performing a no-op shift.
  إذا (!کھسکاؤ_Amt_2)
    أرجع;

  // Save the original sign bit for later.
  منطقي Negative = isNegative();

  // WordShift is the inter-part shift; BitShift is intra-part shift.
  بنانشانی لفظ_کھسکاؤ = کھسکاؤ_Amt_2 / APINT_بٹ_فی_لفظ;
  بنانشانی ثنائی_ہندسہ_کھسکاؤ = کھسکاؤ_Amt_2 % APINT_بٹ_فی_لفظ;

  بنانشانی الفاظ_منزل_Move = get_عدد_الفاظ() - لفظ_کھسکاؤ;
  إذا (الفاظ_منزل_Move != 0) {
    // Sign extend the last word to fill in the unused bits.
    U.pVal[get_عدد_الفاظ() - 1] = SignExtend64(
        U.pVal[get_عدد_الفاظ() - 1], ((ثنائی_ہندسہ_چوڑائی_2 - 1) % APINT_بٹ_فی_لفظ) + 1);

    // Fastpath for moving by whole words.
    إذا (ثنائی_ہندسہ_کھسکاؤ == 0) {
      std::memmove(U.pVal, U.pVal + لفظ_کھسکاؤ, الفاظ_منزل_Move * APINT_لفظ_ناپ);
    } وإلا {
      // Move the words containing significant bits.
      لكل (بنانشانی i = 0; i != الفاظ_منزل_Move - 1; ++i)
        U.pVal[i] = (U.pVal[i + لفظ_کھسکاؤ] >> ثنائی_ہندسہ_کھسکاؤ) |
                    (U.pVal[i + لفظ_کھسکاؤ + 1] << (APINT_بٹ_فی_لفظ - ثنائی_ہندسہ_کھسکاؤ));

      // Handle the last word which has no high bits to copy. Use an arithmetic
      // shift to preserve the sign bit.
      U.pVal[الفاظ_منزل_Move - 1] =
          (int64_t)U.pVal[لفظ_کھسکاؤ + الفاظ_منزل_Move - 1] >> ثنائی_ہندسہ_کھسکاؤ;
    }
  }

  // Fill in the remainder based on the original sign.
  std::memset(U.pVal + الفاظ_منزل_Move, Negative ? -1 : 0,
              لفظ_کھسکاؤ * APINT_لفظ_ناپ);
  clear_بن_ورتیا_بٹ();
}

/// Logical right-shift this APInt by shiftAmt.
/// Logical right-shift function.
فراغ APInt::lshr_وچ_رکھو(ثابت APInt &کھسکاؤ_Amt) {
  lshr_وچ_رکھو((بنانشانی)کھسکاؤ_Amt.get_Limited_قدر(ثنائی_ہندسہ_چوڑائی_2));
}

/// Logical right-shift this APInt by shiftAmt.
/// Logical right-shift function.
فراغ APInt::lshrSlowCase(بنانشانی کھسکاؤ_Amt_2) {
  tc_کھسکاؤ_دایاں(U.pVal, get_عدد_الفاظ(), کھسکاؤ_Amt_2);
}

/// Left-shift this APInt by shiftAmt.
/// Left-shift function.
APInt &APInt::عامل<<=(ثابت APInt &کھسکاؤ_Amt) {
  // It's undefined behavior in C to shift by BitWidth or greater.
  *هذا <<= (بنانشانی)کھسکاؤ_Amt.get_Limited_قدر(ثنائی_ہندسہ_چوڑائی_2);
  أرجع *هذا;
}

فراغ APInt::shlSlowCase(بنانشانی کھسکاؤ_Amt_2) {
  tc_کھسکاؤ_بایاں(U.pVal, get_عدد_الفاظ(), کھسکاؤ_Amt_2);
  clear_بن_ورتیا_بٹ();
}

// Calculate the rotate amount modulo the bit width.
ساكن بنانشانی rotateModulo(بنانشانی ثنائی_ہندسہ_چوڑائی_2, ثابت APInt &rotateAmt) {
  إذا (LLVM_UNLIKELY(ثنائی_ہندسہ_چوڑائی_2 == 0))
    أرجع 0;
  بنانشانی rot_ثنائی_ہندسہ_چوڑائی = rotateAmt.get_ثنائی_ہندسہ_چوڑائی();
  APInt rot = rotateAmt;
  إذا (rot_ثنائی_ہندسہ_چوڑائی < ثنائی_ہندسہ_چوڑائی_2) {
    // Extend the rotate APInt, so that the urem doesn't divide by 0.
    // e.g. APInt(1, 32) would give APInt(1, 0).
    rot = rotateAmt.zext(ثنائی_ہندسہ_چوڑائی_2);
  }
  rot = rot.urem(APInt(rot.get_ثنائی_ہندسہ_چوڑائی(), ثنائی_ہندسہ_چوڑائی_2));
  أرجع rot.get_Limited_قدر(ثنائی_ہندسہ_چوڑائی_2);
}

APInt APInt::rotl(ثابت APInt &rotateAmt) ثابت {
  أرجع rotl(rotateModulo(ثنائی_ہندسہ_چوڑائی_2, rotateAmt));
}

APInt APInt::rotl(بنانشانی rotateAmt) ثابت {
  إذا (LLVM_UNLIKELY(ثنائی_ہندسہ_چوڑائی_2 == 0))
    أرجع *هذا;
  rotateAmt %= ثنائی_ہندسہ_چوڑائی_2;
  إذا (rotateAmt == 0)
    أرجع *هذا;
  أرجع shl(rotateAmt) | lshr(ثنائی_ہندسہ_چوڑائی_2 - rotateAmt);
}

APInt APInt::rotr(ثابت APInt &rotateAmt) ثابت {
  أرجع rotr(rotateModulo(ثنائی_ہندسہ_چوڑائی_2, rotateAmt));
}

APInt APInt::rotr(بنانشانی rotateAmt) ثابت {
  إذا (ثنائی_ہندسہ_چوڑائی_2 == 0)
    أرجع *هذا;
  rotateAmt %= ثنائی_ہندسہ_چوڑائی_2;
  إذا (rotateAmt == 0)
    أرجع *هذا;
  أرجع lshr(rotateAmt) | shl(ثنائی_ہندسہ_چوڑائی_2 - rotateAmt);
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
بنانشانی APInt::سبھ_توں_نیڑے_Log_Base2() ثابت {
  // Special case when we have a bitwidth of 1. If VAL is 1, then we
  // get 0. If VAL is 0, we get WORDTYPE_MAX which gets truncated to
  // UINT32_MAX.
  إذا (ثنائی_ہندسہ_چوڑائی_2 == 1)
    أرجع U.VAL - 1;

  // Handle the zero case.
  إذا (isZero())
    أرجع UINT32_MAX;

  // The non-zero case is handled by computing:
  //
  //   nearestLogBase2(x) = logBase2(x) + x[logBase2(x)-1].
  //
  // where x[i] is referring to the value of the ith bit of x.
  بنانشانی lg = logBase2();
  أرجع lg + بنانشانی((*هذا)[lg - 1]);
}

// Square Root - this method computes and returns the square root of "this".
// Three mechanisms are used for computation. For small values (<= 5 bits),
// a table lookup is done. This gets some performance for common cases. For
// values using less than 52 bits, the value is converted to double and then
// the libc sqrt function is called. The result is rounded and then converted
// back to a uint64_t which is then used to construct the result. Finally,
// the Babylonian method for computing square roots is used.
APInt APInt::sqrtFloor() ثابت {

  // Determine the magnitude of the value.
  بنانشانی magnitude = get_سرگرم_بٹ();

  // Use a fast table for some small values. This also gets rid of some
  // rounding errors in libc sqrt for small values.
  إذا (magnitude <= 5) {
    ساكن ثابت uint8_t results[32] = {
        /*     0 */ 0,
        /*  1- 3 */ 1, 1, 1,
        /*  4- 8 */ 2, 2, 2, 2, 2,
        /*  9-15 */ 3, 3, 3, 3, 3, 3, 3,
        /* 16-24 */ 4, 4, 4, 4, 4, 4, 4, 4, 4,
        /* 25-31 */ 5, 5, 5, 5, 5, 5, 5,
    };
    أرجع APInt(ثنائی_ہندسہ_چوڑائی_2,
                 results[(LLVM_LIKELY(is_Single_لفظ()) ? U.VAL : U.pVal[0])]);
  }

  // If the magnitude of the value fits in less than 52 bits (the precision of
  // an IEEE double precision floating point value), then we can use the
  // libc sqrt function which will probably use a hardware sqrt computation.
  // This should be faster than the algorithm below.
  إذا (magnitude < 52) {
    أرجع APInt(ثنائی_ہندسہ_چوڑائی_2,
                 uint64_t(::floor(::جذر(مزدوج(
                     LLVM_LIKELY(is_Single_لفظ()) ? U.VAL : U.pVal[0])))));
  }

  // Okay, all the short cuts are exhausted. We must compute it. The following
  // is a classical Babylonian method for computing the square root. This code
  // was adapted to APInt from a wikipedia article on such computations.
  // See http://www.wikipedia.org/ and go to the page named
  // Calculate_an_integer_square_root.
  بنانشانی nbits = ثنائی_ہندسہ_چوڑائی_2, i = 4;
  APInt testy(ثنائی_ہندسہ_چوڑائی_2, 16);
  APInt x_old(ثنائی_ہندسہ_چوڑائی_2, 1);
  APInt x_new(ثنائی_ہندسہ_چوڑائی_2, 0);
  APInt two(ثنائی_ہندسہ_چوڑائی_2, 2);

  // Select a good starting value using binary logarithms.
  لكل (;; i += 2, testy = testy.shl(2))
    إذا (i >= nbits || هذا->ule(testy)) {
      x_old = x_old.shl(i / 2);
      اكسر;
    }

  // Use the Babylonian method to arrive at the integer square root:
  لكل (;;) {
    x_new = (هذا->udiv(x_old) + x_old).udiv(two);
    إذا (x_old.ule(x_new))
      اكسر;
    x_old = x_new;
  }
  أرجع x_old;
}

/// \returns the multiplicative inverse of an odd APInt modulo 2^BitWidth.
APInt APInt::multiplicativeInverse() ثابت {
  أكد((*هذا)[0] &&
         "multiplicative inverse is only defined for odd numbers!");

  // Use Newton's method.
  APInt ضارب = *هذا;
  APInt T;
  طالما (!(T = *هذا * ضارب).isOne())
    ضارب *= 2 - std::move(T);
  أرجع ضارب;
}

/// Implementation of Knuth's Algorithm D (Division of nonnegative integers)
/// from "Art of Computer Programming, Volume 2", section 4.3.1, p. 272. The
/// variables here have the same names as in the algorithm. Comments explain
/// the algorithm and any deviation from it.
ساكن فراغ KnuthDiv(uint32_t *u, uint32_t *v, uint32_t *q, uint32_t* r,
                     بنانشانی m, بنانشانی n) {
  أكد(u && "Must provide dividend");
  أكد(v && "Must provide divisor");
  أكد(q && "Must provide quotient");
  أكد(u != v && u != q && v != q && "Must use different memory");
  أكد(n>1 && "n must be > 1");

  // b denotes the base of the number system. In our case b is 2^32.
  ثابت uint64_t b = uint64_t(1) << 32;

// The DEBUG macros here tend to be spam in the debug output if you're not
// debugging this code. Disable them unless KNUTH_DEBUG is defined.
#جے_تعریف_شدہ KNUTH_DEBUG
#تعریف_کرو DEBUG_KNUTH(X) LLVM_DEBUG(X)
#وإلا
#تعریف_کرو DEBUG_KNUTH(X) افعل {} طالما(خطأ)
#جے_انت

  DEBUG_KNUTH(dbgs() << "KnuthDiv: m=" << m << " n=" << n << '\n');
  DEBUG_KNUTH(dbgs() << "KnuthDiv: original:");
  DEBUG_KNUTH(لكل (صحيح i = m + n; i >= 0; i--) dbgs() << " " << u[i]);
  DEBUG_KNUTH(dbgs() << " by");
  DEBUG_KNUTH(لكل (صحيح i = n; i > 0; i--) dbgs() << " " << v[i - 1]);
  DEBUG_KNUTH(dbgs() << '\n');
  // D1. [Normalize.] Set d = b / (v[n-1] + 1) and multiply all the digits of
  // u and v by d. Note that we have taken Knuth's advice here to use a power
  // of 2 value for d such that d * v[n-1] >= b/2 (b is the base). A power of
  // 2 allows us to shift instead of multiply and it is easy to determine the
  // shift amount from the leading zeros.  We are basically normalizing the u
  // and v so that its high bits are shifted to the top of v's range without
  // overflow. Note that this can require an extra word in u so that u must
  // be of length m+n+1.
  بنانشانی کھسکاؤ = llvm::countl_zero(v[n - 1]);
  uint32_t v_حاصل = 0;
  uint32_t u_حاصل = 0;
  إذا (کھسکاؤ) {
    لكل (بنانشانی i = 0; i < m+n; ++i) {
      uint32_t u_tmp = u[i] >> (32 - کھسکاؤ);
      u[i] = (u[i] << کھسکاؤ) | u_حاصل;
      u_حاصل = u_tmp;
    }
    لكل (بنانشانی i = 0; i < n; ++i) {
      uint32_t v_tmp = v[i] >> (32 - کھسکاؤ);
      v[i] = (v[i] << کھسکاؤ) | v_حاصل;
      v_حاصل = v_tmp;
    }
  }
  u[m+n] = u_حاصل;

  DEBUG_KNUTH(dbgs() << "KnuthDiv:   normal:");
  DEBUG_KNUTH(لكل (صحيح i = m + n; i >= 0; i--) dbgs() << " " << u[i]);
  DEBUG_KNUTH(dbgs() << " by");
  DEBUG_KNUTH(لكل (صحيح i = n; i > 0; i--) dbgs() << " " << v[i - 1]);
  DEBUG_KNUTH(dbgs() << '\n');

  // D2. [Initialize j.]  Set j to m. This is the loop counter over the places.
  صحيح j = m;
  افعل {
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
    إذا (qp == b || qp*v[n-2] > b*rp + u[j+n-2]) {
      qp--;
      rp += v[n-1];
      إذا (rp < b && (qp == b || qp*v[n-2] > b*rp + u[j+n-2]))
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
    لكل (بنانشانی i = 0; i < n; ++i) {
      uint64_t p = qp * uint64_t(v[i]);
      int64_t subres = int64_t(u[j+i]) - borrow - Lo_32(p);
      u[j+i] = Lo_32(subres);
      borrow = Hi_32(p) - Hi_32(subres);
      DEBUG_KNUTH(dbgs() << "KnuthDiv: u[j+i] = " << u[j + i]
                        << ", borrow = " << borrow << '\n');
    }
    منطقي isNeg = u[j+n] < borrow;
    u[j+n] -= Lo_32(borrow);

    DEBUG_KNUTH(dbgs() << "KnuthDiv: after subtraction:");
    DEBUG_KNUTH(لكل (صحيح i = m + n; i >= 0; i--) dbgs() << " " << u[i]);
    DEBUG_KNUTH(dbgs() << '\n');

    // D5. [Test remainder.] Set q[j] = qp. If the result of step D4 was
    // negative, go to step D6; otherwise go on to step D7.
    q[j] = Lo_32(qp);
    إذا (isNeg) {
      // D6. [Add back]. The probability that this step is necessary is very
      // small, on the order of only 2/b. Make sure that test data accounts for
      // this possibility. Decrease q[j] by 1
      q[j]--;
      // and add (0v[n-1]...v[1]v[0]) to (u[j+n]u[j+n-1]...u[j+1]u[j]).
      // A carry will occur to the left of u[j+n], and it should be ignored
      // since it cancels with the borrow that occurred in D4.
      منطقي حاصل = خطأ;
      لكل (بنانشانی i = 0; i < n; i++) {
        uint32_t حد_بندی = std::min(u[j+i],v[i]);
        u[j+i] += v[i] + حاصل;
        حاصل = u[j+i] < حد_بندی || (حاصل && u[j+i] == حد_بندی);
      }
      u[j+n] += حاصل;
    }
    DEBUG_KNUTH(dbgs() << "KnuthDiv: after correction:");
    DEBUG_KNUTH(لكل (صحيح i = m + n; i >= 0; i--) dbgs() << " " << u[i]);
    DEBUG_KNUTH(dbgs() << "\nKnuthDiv: digit result = " << q[j] << '\n');

    // D7. [Loop on j.]  Decrease j by one. Now if j >= 0, go back to D3.
  } طالما (--j >= 0);

  DEBUG_KNUTH(dbgs() << "KnuthDiv: quotient:");
  DEBUG_KNUTH(لكل (صحيح i = m; i >= 0; i--) dbgs() << " " << q[i]);
  DEBUG_KNUTH(dbgs() << '\n');

  // D8. [Unnormalize]. Now q[...] is the desired quotient, and the desired
  // remainder may be obtained by dividing u[...] by d. If r is non-null we
  // compute the remainder (urem uses this).
  إذا (r) {
    // The value d is expressed by the "shift" value above since we avoided
    // multiplication by d by using a shift left. So, all we have to do is
    // shift right here.
    إذا (کھسکاؤ) {
      uint32_t حاصل = 0;
      DEBUG_KNUTH(dbgs() << "KnuthDiv: remainder:");
      لكل (صحيح i = n-1; i >= 0; i--) {
        r[i] = (u[i] >> کھسکاؤ) | حاصل;
        حاصل = u[i] << (32 - کھسکاؤ);
        DEBUG_KNUTH(dbgs() << " " << r[i]);
      }
    } وإلا {
      لكل (صحيح i = n-1; i >= 0; i--) {
        r[i] = u[i];
        DEBUG_KNUTH(dbgs() << " " << r[i]);
      }
    }
    DEBUG_KNUTH(dbgs() << '\n');
  }
  DEBUG_KNUTH(dbgs() << '\n');
}

فراغ APInt::divide(ثابت لفظ_Type *LHS, بنانشانی lhs_الفاظ, ثابت لفظ_Type *RHS,
                   بنانشانی rhs_الفاظ, لفظ_Type *Quotient, لفظ_Type *Remainder) {
  أكد(lhs_الفاظ >= rhs_الفاظ && "Fractional result");

  // First, compose the values into an array of 32-bit words instead of
  // 64-bit words. This is a necessity of both the "short division" algorithm
  // and the Knuth "classical algorithm" which requires there to be native
  // operations for +, -, and * on an m bit value with an m*2 bit result. We
  // can't use 64-bit operands here because we don't have native results of
  // 128-bits. Furthermore, casting the 64-bit values to 32-bit values won't
  // work on large-endian machines.
  بنانشانی n = rhs_الفاظ * 2;
  بنانشانی m = (lhs_الفاظ * 2) - n;

  // Allocate space for the temporary values we need either on the stack, if
  // it will fit, or on the heap if it won't.
  uint32_t SPACE[128];
  uint32_t *U = خالی_سوچک;
  uint32_t *V = خالی_سوچک;
  uint32_t *Q = خالی_سوچک;
  uint32_t *R = خالی_سوچک;
  إذا ((Remainder?4:3)*n+2*m+1 <= 128) {
    U = &SPACE[0];
    V = &SPACE[m+n+1];
    Q = &SPACE[(m+n+1) + n];
    إذا (Remainder)
      R = &SPACE[(m+n+1) + n + (m+n)];
  } وإلا {
    U = جديد uint32_t[m + n + 1];
    V = جديد uint32_t[n];
    Q = جديد uint32_t[m+n];
    إذا (Remainder)
      R = جديد uint32_t[n];
  }

  // Initialize the dividend
  memset(U, 0, (m+n+1)*حجم(uint32_t));
  لكل (بنانشانی i = 0; i < lhs_الفاظ; ++i) {
    uint64_t tmp = LHS[i];
    U[i * 2] = Lo_32(tmp);
    U[i * 2 + 1] = Hi_32(tmp);
  }
  U[m+n] = 0; // this extra word is for "spill" in the Knuth algorithm.

  // Initialize the divisor
  memset(V, 0, (n)*حجم(uint32_t));
  لكل (بنانشانی i = 0; i < rhs_الفاظ; ++i) {
    uint64_t tmp = RHS[i];
    V[i * 2] = Lo_32(tmp);
    V[i * 2 + 1] = Hi_32(tmp);
  }

  // initialize the quotient and remainder
  memset(Q, 0, (m+n) * حجم(uint32_t));
  إذا (Remainder)
    memset(R, 0, n * حجم(uint32_t));

  // Now, adjust m and n for the Knuth division. n is the number of words in
  // the divisor. m is the number of words by which the dividend exceeds the
  // divisor (i.e. m+n is the length of the dividend). These sizes must not
  // contain any zero words or the Knuth algorithm fails.
  لكل (بنانشانی i = n; i > 0 && V[i-1] == 0; i--) {
    n--;
    m++;
  }
  لكل (بنانشانی i = m+n; i > 0 && U[i-1] == 0; i--)
    m--;

  // If we're left with only a single word for the divisor, Knuth doesn't work
  // so we implement the short division algorithm here. This is much simpler
  // and faster because we are certain that we can divide a 64-bit quantity
  // by a 32-bit quantity at hardware speed and short division is simply a
  // series of such operations. This is just like doing short division but we
  // are using base 2^32 instead of base 10.
  أكد(n != 0 && "Divide by zero?");
  إذا (n == 1) {
    uint32_t مقسوم_علیہ = V[0];
    uint32_t remainder = 0;
    لكل (صحيح i = m; i >= 0; i--) {
      uint64_t partial_dividend = Make_64(remainder, U[i]);
      إذا (partial_dividend == 0) {
        Q[i] = 0;
        remainder = 0;
      } وإلا إذا (partial_dividend < مقسوم_علیہ) {
        Q[i] = 0;
        remainder = Lo_32(partial_dividend);
      } وإلا إذا (partial_dividend == مقسوم_علیہ) {
        Q[i] = 1;
        remainder = 0;
      } وإلا {
        Q[i] = Lo_32(partial_dividend / مقسوم_علیہ);
        remainder = Lo_32(partial_dividend - (Q[i] * مقسوم_علیہ));
      }
    }
    إذا (R)
      R[0] = remainder;
  } وإلا {
    // Now we're ready to invoke the Knuth classical divide algorithm. In this
    // case n > 1.
    KnuthDiv(U, V, Q, R, m, n);
  }

  // If the caller wants the quotient
  إذا (Quotient) {
    لكل (بنانشانی i = 0; i < lhs_الفاظ; ++i)
      Quotient[i] = Make_64(Q[i*2+1], Q[i*2]);
  }

  // If the caller wants the remainder
  إذا (Remainder) {
    لكل (بنانشانی i = 0; i < rhs_الفاظ; ++i)
      Remainder[i] = Make_64(R[i*2+1], R[i*2]);
  }

  // Clean up the memory we allocated.
  إذا (U != &SPACE[0]) {
    مٹاؤ [] U;
    مٹاؤ [] V;
    مٹاؤ [] Q;
    مٹاؤ [] R;
  }
}

APInt APInt::udiv(ثابت APInt &RHS) ثابت {
  أكد(ثنائی_ہندسہ_چوڑائی_2 == RHS.ثنائی_ہندسہ_چوڑائی_2 && "Bit widths must be the same");

  // First, deal with the easy case
  إذا (LLVM_LIKELY(is_Single_لفظ())) {
    أكد(RHS.U.VAL != 0 && "Divide by zero?");
    أرجع APInt(ثنائی_ہندسہ_چوڑائی_2, U.VAL / RHS.U.VAL);
  }

  // Get some facts about the LHS and RHS number of bits and words
  بنانشانی lhs_الفاظ = get_عدد_الفاظ(get_سرگرم_بٹ());
  بنانشانی rhs_بٹ  = RHS.get_سرگرم_بٹ();
  بنانشانی rhs_الفاظ = get_عدد_الفاظ(rhs_بٹ);
  أكد(rhs_الفاظ && "Divided by zero???");

  // Deal with some degenerate cases
  إذا (!lhs_الفاظ)
    // 0 / X ===> 0
    أرجع APInt(ثنائی_ہندسہ_چوڑائی_2, 0);
  إذا (rhs_بٹ == 1)
    // X / 1 ===> X
    أرجع *هذا;
  إذا (lhs_الفاظ < rhs_الفاظ || هذا->ult(RHS))
    // X / Y ===> 0, iff X < Y
    أرجع APInt(ثنائی_ہندسہ_چوڑائی_2, 0);
  إذا (*هذا == RHS)
    // X / X ===> 1
    أرجع APInt(ثنائی_ہندسہ_چوڑائی_2, 1);
  إذا (lhs_الفاظ == 1) // rhsWords is 1 if lhsWords is 1.
    // All high words are zero, just use native divide
    أرجع APInt(ثنائی_ہندسہ_چوڑائی_2, هذا->U.pVal[0] / RHS.U.pVal[0]);

  // We have to compute it the hard way. Invoke the Knuth divide algorithm.
  APInt Quotient(ثنائی_ہندسہ_چوڑائی_2, 0); // to hold result.
  divide(U.pVal, lhs_الفاظ, RHS.U.pVal, rhs_الفاظ, Quotient.U.pVal, خالی_سوچک);
  أرجع Quotient;
}

APInt APInt::udiv(uint64_t RHS) ثابت {
  أكد(RHS != 0 && "Divide by zero?");

  // First, deal with the easy case
  إذا (LLVM_LIKELY(is_Single_لفظ()))
    أرجع APInt(ثنائی_ہندسہ_چوڑائی_2, U.VAL / RHS);

  // Get some facts about the LHS words.
  بنانشانی lhs_الفاظ = get_عدد_الفاظ(get_سرگرم_بٹ());

  // Deal with some degenerate cases
  إذا (!lhs_الفاظ)
    // 0 / X ===> 0
    أرجع APInt(ثنائی_ہندسہ_چوڑائی_2, 0);
  إذا (RHS == 1)
    // X / 1 ===> X
    أرجع *هذا;
  إذا (هذا->ult(RHS))
    // X / Y ===> 0, iff X < Y
    أرجع APInt(ثنائی_ہندسہ_چوڑائی_2, 0);
  إذا (*هذا == RHS)
    // X / X ===> 1
    أرجع APInt(ثنائی_ہندسہ_چوڑائی_2, 1);
  إذا (lhs_الفاظ == 1) // rhsWords is 1 if lhsWords is 1.
    // All high words are zero, just use native divide
    أرجع APInt(ثنائی_ہندسہ_چوڑائی_2, هذا->U.pVal[0] / RHS);

  // We have to compute it the hard way. Invoke the Knuth divide algorithm.
  APInt Quotient(ثنائی_ہندسہ_چوڑائی_2, 0); // to hold result.
  divide(U.pVal, lhs_الفاظ, &RHS, 1, Quotient.U.pVal, خالی_سوچک);
  أرجع Quotient;
}

APInt APInt::sdiv(ثابت APInt &RHS) ثابت {
  إذا (isNegative()) {
    إذا (RHS.isNegative())
      أرجع (-(*هذا)).udiv(-RHS);
    أرجع -((-(*هذا)).udiv(RHS));
  }
  إذا (RHS.isNegative())
    أرجع -(هذا->udiv(-RHS));
  أرجع هذا->udiv(RHS);
}

APInt APInt::sdiv(int64_t RHS) ثابت {
  إذا (isNegative()) {
    إذا (RHS < 0)
      أرجع (-(*هذا)).udiv(-RHS);
    أرجع -((-(*هذا)).udiv(RHS));
  }
  إذا (RHS < 0)
    أرجع -(هذا->udiv(-RHS));
  أرجع هذا->udiv(RHS);
}

APInt APInt::urem(ثابت APInt &RHS) ثابت {
  أكد(ثنائی_ہندسہ_چوڑائی_2 == RHS.ثنائی_ہندسہ_چوڑائی_2 && "Bit widths must be the same");
  إذا (LLVM_LIKELY(is_Single_لفظ())) {
    أكد(RHS.U.VAL != 0 && "Remainder by zero?");
    أرجع APInt(ثنائی_ہندسہ_چوڑائی_2, U.VAL % RHS.U.VAL);
  }

  // Get some facts about the LHS
  بنانشانی lhs_الفاظ = get_عدد_الفاظ(get_سرگرم_بٹ());

  // Get some facts about the RHS
  بنانشانی rhs_بٹ = RHS.get_سرگرم_بٹ();
  بنانشانی rhs_الفاظ = get_عدد_الفاظ(rhs_بٹ);
  أكد(rhs_الفاظ && "Performing remainder operation by zero ???");

  // Check the degenerate cases
  إذا (lhs_الفاظ == 0)
    // 0 % Y ===> 0
    أرجع APInt(ثنائی_ہندسہ_چوڑائی_2, 0);
  إذا (rhs_بٹ == 1)
    // X % 1 ===> 0
    أرجع APInt(ثنائی_ہندسہ_چوڑائی_2, 0);
  إذا (lhs_الفاظ < rhs_الفاظ || هذا->ult(RHS))
    // X % Y ===> X, iff X < Y
    أرجع *هذا;
  إذا (*هذا == RHS)
    // X % X == 0;
    أرجع APInt(ثنائی_ہندسہ_چوڑائی_2, 0);
  إذا (lhs_الفاظ == 1)
    // All high words are zero, just use native remainder
    أرجع APInt(ثنائی_ہندسہ_چوڑائی_2, U.pVal[0] % RHS.U.pVal[0]);
  إذا (RHS.isPowerOf2()) {
    // X % 2^w ===> X & (2^w - 1)
    APInt نتیجہ(*هذا);
    نتیجہ.clear_بٹ(RHS.logBase2(), ثنائی_ہندسہ_چوڑائی_2);
    أرجع نتیجہ;
  }

  // We have to compute it the hard way. Invoke the Knuth divide algorithm.
  APInt Remainder(ثنائی_ہندسہ_چوڑائی_2, 0);
  divide(U.pVal, lhs_الفاظ, RHS.U.pVal, rhs_الفاظ, خالی_سوچک, Remainder.U.pVal);
  أرجع Remainder;
}

uint64_t APInt::urem(uint64_t RHS) ثابت {
  أكد(RHS != 0 && "Remainder by zero?");

  إذا (LLVM_LIKELY(is_Single_لفظ()))
    أرجع U.VAL % RHS;

  // Get some facts about the LHS
  بنانشانی lhs_الفاظ = get_عدد_الفاظ(get_سرگرم_بٹ());

  // Check the degenerate cases
  إذا (lhs_الفاظ == 0)
    // 0 % Y ===> 0
    أرجع 0;
  إذا (RHS == 1)
    // X % 1 ===> 0
    أرجع 0;
  إذا (هذا->ult(RHS))
    // X % Y ===> X, iff X < Y
    أرجع get_ZExt_قدر();
  إذا (*هذا == RHS)
    // X % X == 0;
    أرجع 0;
  إذا (lhs_الفاظ == 1)
    // All high words are zero, just use native remainder
    أرجع U.pVal[0] % RHS;
  إذا (llvm::isPowerOf2_64(RHS))
    // X % 2^w ===> X & (2^w - 1)
    أرجع U.pVal[0] & (RHS - 1);

  // We have to compute it the hard way. Invoke the Knuth divide algorithm.
  uint64_t Remainder;
  divide(U.pVal, lhs_الفاظ, &RHS, 1, خالی_سوچک, &Remainder);
  أرجع Remainder;
}

APInt APInt::srem(ثابت APInt &RHS) ثابت {
  إذا (isNegative()) {
    إذا (RHS.isNegative())
      أرجع -((-(*هذا)).urem(-RHS));
    أرجع -((-(*هذا)).urem(RHS));
  }
  إذا (RHS.isNegative())
    أرجع هذا->urem(-RHS);
  أرجع هذا->urem(RHS);
}

int64_t APInt::srem(int64_t RHS) ثابت {
  إذا (isNegative()) {
    إذا (RHS < 0)
      أرجع -((-(*هذا)).urem(-RHS));
    أرجع -((-(*هذا)).urem(RHS));
  }
  إذا (RHS < 0)
    أرجع هذا->urem(-RHS);
  أرجع هذا->urem(RHS);
}

فراغ APInt::udivrem(ثابت APInt &LHS, ثابت APInt &RHS,
                    APInt &Quotient, APInt &Remainder) {
  أكد(LHS.ثنائی_ہندسہ_چوڑائی_2 == RHS.ثنائی_ہندسہ_چوڑائی_2 && "Bit widths must be the same");
  بنانشانی ثنائی_ہندسہ_چوڑائی_2 = LHS.ثنائی_ہندسہ_چوڑائی_2;

  // First, deal with the easy case
  إذا (LLVM_LIKELY(LHS.is_Single_لفظ())) {
    أكد(RHS.U.VAL != 0 && "Divide by zero?");
    uint64_t QuotVal = LHS.U.VAL / RHS.U.VAL;
    uint64_t RemVal = LHS.U.VAL % RHS.U.VAL;
    Quotient = APInt(ثنائی_ہندسہ_چوڑائی_2, QuotVal);
    Remainder = APInt(ثنائی_ہندسہ_چوڑائی_2, RemVal);
    أرجع;
  }

  // Get some size facts about the dividend and divisor
  بنانشانی lhs_الفاظ = get_عدد_الفاظ(LHS.get_سرگرم_بٹ());
  بنانشانی rhs_بٹ  = RHS.get_سرگرم_بٹ();
  بنانشانی rhs_الفاظ = get_عدد_الفاظ(rhs_بٹ);
  أكد(rhs_الفاظ && "Performing divrem operation by zero ???");

  // Check the degenerate cases
  إذا (lhs_الفاظ == 0) {
    Quotient = APInt(ثنائی_ہندسہ_چوڑائی_2, 0);    // 0 / Y ===> 0
    Remainder = APInt(ثنائی_ہندسہ_چوڑائی_2, 0);   // 0 % Y ===> 0
    أرجع;
  }

  إذا (rhs_بٹ == 1) {
    Quotient = LHS;                   // X / 1 ===> X
    Remainder = APInt(ثنائی_ہندسہ_چوڑائی_2, 0);   // X % 1 ===> 0
  }

  إذا (lhs_الفاظ < rhs_الفاظ || LHS.ult(RHS)) {
    Remainder = LHS;                  // X % Y ===> X, iff X < Y
    Quotient = APInt(ثنائی_ہندسہ_چوڑائی_2, 0);    // X / Y ===> 0, iff X < Y
    أرجع;
  }

  إذا (LHS == RHS) {
    Quotient  = APInt(ثنائی_ہندسہ_چوڑائی_2, 1);   // X / X ===> 1
    Remainder = APInt(ثنائی_ہندسہ_چوڑائی_2, 0);   // X % X ===> 0;
    أرجع;
  }

  // Make sure there is enough space to hold the results.
  // NOTE: This assumes that reallocate won't affect any bits if it doesn't
  // change the size. This is necessary if Quotient or Remainder is aliased
  // with LHS or RHS.
  Quotient.reallocate(ثنائی_ہندسہ_چوڑائی_2);
  Remainder.reallocate(ثنائی_ہندسہ_چوڑائی_2);

  إذا (lhs_الفاظ == 1) { // rhsWords is 1 if lhsWords is 1.
    // There is only one word to consider so use the native versions.
    uint64_t lhs_قدر = LHS.U.pVal[0];
    uint64_t rhs_قدر = RHS.U.pVal[0];
    Quotient = lhs_قدر / rhs_قدر;
    Remainder = lhs_قدر % rhs_قدر;
    أرجع;
  }

  // Okay, lets do it the long way
  divide(LHS.U.pVal, lhs_الفاظ, RHS.U.pVal, rhs_الفاظ, Quotient.U.pVal,
         Remainder.U.pVal);
  // Clear the rest of the Quotient and Remainder.
  std::memset(Quotient.U.pVal + lhs_الفاظ, 0,
              (get_عدد_الفاظ(ثنائی_ہندسہ_چوڑائی_2) - lhs_الفاظ) * APINT_لفظ_ناپ);
  std::memset(Remainder.U.pVal + rhs_الفاظ, 0,
              (get_عدد_الفاظ(ثنائی_ہندسہ_چوڑائی_2) - rhs_الفاظ) * APINT_لفظ_ناپ);
}

فراغ APInt::udivrem(ثابت APInt &LHS, uint64_t RHS, APInt &Quotient,
                    uint64_t &Remainder) {
  أكد(RHS != 0 && "Divide by zero?");
  بنانشانی ثنائی_ہندسہ_چوڑائی_2 = LHS.ثنائی_ہندسہ_چوڑائی_2;

  // First, deal with the easy case
  إذا (LLVM_LIKELY(LHS.is_Single_لفظ())) {
    uint64_t QuotVal = LHS.U.VAL / RHS;
    Remainder = LHS.U.VAL % RHS;
    Quotient = APInt(ثنائی_ہندسہ_چوڑائی_2, QuotVal);
    أرجع;
  }

  // Get some size facts about the dividend and divisor
  بنانشانی lhs_الفاظ = get_عدد_الفاظ(LHS.get_سرگرم_بٹ());

  // Check the degenerate cases
  إذا (lhs_الفاظ == 0) {
    Quotient = APInt(ثنائی_ہندسہ_چوڑائی_2, 0);    // 0 / Y ===> 0
    Remainder = 0;                    // 0 % Y ===> 0
    أرجع;
  }

  إذا (RHS == 1) {
    Quotient = LHS;                   // X / 1 ===> X
    Remainder = 0;                    // X % 1 ===> 0
    أرجع;
  }

  إذا (LHS.ult(RHS)) {
    Remainder = LHS.get_ZExt_قدر();   // X % Y ===> X, iff X < Y
    Quotient = APInt(ثنائی_ہندسہ_چوڑائی_2, 0);    // X / Y ===> 0, iff X < Y
    أرجع;
  }

  إذا (LHS == RHS) {
    Quotient  = APInt(ثنائی_ہندسہ_چوڑائی_2, 1);   // X / X ===> 1
    Remainder = 0;                    // X % X ===> 0;
    أرجع;
  }

  // Make sure there is enough space to hold the results.
  // NOTE: This assumes that reallocate won't affect any bits if it doesn't
  // change the size. This is necessary if Quotient is aliased with LHS.
  Quotient.reallocate(ثنائی_ہندسہ_چوڑائی_2);

  إذا (lhs_الفاظ == 1) { // rhsWords is 1 if lhsWords is 1.
    // There is only one word to consider so use the native versions.
    uint64_t lhs_قدر = LHS.U.pVal[0];
    Quotient = lhs_قدر / RHS;
    Remainder = lhs_قدر % RHS;
    أرجع;
  }

  // Okay, lets do it the long way
  divide(LHS.U.pVal, lhs_الفاظ, &RHS, 1, Quotient.U.pVal, &Remainder);
  // Clear the rest of the Quotient.
  std::memset(Quotient.U.pVal + lhs_الفاظ, 0,
              (get_عدد_الفاظ(ثنائی_ہندسہ_چوڑائی_2) - lhs_الفاظ) * APINT_لفظ_ناپ);
}

فراغ APInt::sdivrem(ثابت APInt &LHS, ثابت APInt &RHS,
                    APInt &Quotient, APInt &Remainder) {
  إذا (LHS.isNegative()) {
    إذا (RHS.isNegative())
      APInt::udivrem(-LHS, -RHS, Quotient, Remainder);
    وإلا {
      APInt::udivrem(-LHS, RHS, Quotient, Remainder);
      Quotient.negate();
    }
    Remainder.negate();
  } وإلا إذا (RHS.isNegative()) {
    APInt::udivrem(LHS, -RHS, Quotient, Remainder);
    Quotient.negate();
  } وإلا {
    APInt::udivrem(LHS, RHS, Quotient, Remainder);
  }
}

فراغ APInt::sdivrem(ثابت APInt &LHS, int64_t RHS,
                    APInt &Quotient, int64_t &Remainder) {
  uint64_t R = Remainder;
  إذا (LHS.isNegative()) {
    إذا (RHS < 0)
      APInt::udivrem(-LHS, -RHS, Quotient, R);
    وإلا {
      APInt::udivrem(-LHS, RHS, Quotient, R);
      Quotient.negate();
    }
    R = -R;
  } وإلا إذا (RHS < 0) {
    APInt::udivrem(LHS, -RHS, Quotient, R);
    Quotient.negate();
  } وإلا {
    APInt::udivrem(LHS, RHS, Quotient, R);
  }
  Remainder = R;
}

APInt APInt::sadd_ov(ثابت APInt &RHS, منطقي &حد_توں_ودھ) ثابت {
  APInt Res = *هذا+RHS;
  حد_توں_ودھ = isNonNegative() == RHS.isNonNegative() &&
             Res.isNonNegative() != isNonNegative();
  أرجع Res;
}

APInt APInt::uadd_ov(ثابت APInt &RHS, منطقي &حد_توں_ودھ) ثابت {
  APInt Res = *هذا+RHS;
  حد_توں_ودھ = Res.ult(RHS);
  أرجع Res;
}

APInt APInt::ssub_ov(ثابت APInt &RHS, منطقي &حد_توں_ودھ) ثابت {
  APInt Res = *هذا - RHS;
  حد_توں_ودھ = isNonNegative() != RHS.isNonNegative() &&
             Res.isNonNegative() != isNonNegative();
  أرجع Res;
}

APInt APInt::usub_ov(ثابت APInt &RHS, منطقي &حد_توں_ودھ) ثابت {
  APInt Res = *هذا-RHS;
  حد_توں_ودھ = Res.ugt(*هذا);
  أرجع Res;
}

APInt APInt::sdiv_ov(ثابت APInt &RHS, منطقي &حد_توں_ودھ) ثابت {
  // MININT/-1  -->  overflow.
  حد_توں_ودھ = is_Min_Signed_قدر() && RHS.isAllOnes();
  أرجع sdiv(RHS);
}

APInt APInt::smul_ov(ثابت APInt &RHS, منطقي &حد_توں_ودھ) ثابت {
  APInt Res = *هذا * RHS;

  إذا (RHS != 0)
    حد_توں_ودھ = Res.sdiv(RHS) != *هذا ||
               (is_Min_Signed_قدر() && RHS.isAllOnes());
  وإلا
    حد_توں_ودھ = خطأ;
  أرجع Res;
}

APInt APInt::umul_ov(ثابت APInt &RHS, منطقي &حد_توں_ودھ) ثابت {
  إذا (countl_zero() + RHS.countl_zero() + 2 <= ثنائی_ہندسہ_چوڑائی_2) {
    حد_توں_ودھ = سچ;
    أرجع *هذا * RHS;
  }

  APInt Res = lshr(1) * RHS;
  حد_توں_ودھ = Res.isNegative();
  Res <<= 1;
  إذا ((*هذا)[0]) {
    Res += RHS;
    إذا (Res.ult(RHS))
      حد_توں_ودھ = سچ;
  }
  أرجع Res;
}

APInt APInt::sshl_ov(ثابت APInt &ShAmt, منطقي &حد_توں_ودھ) ثابت {
  أرجع sshl_ov(ShAmt.get_Limited_قدر(get_ثنائی_ہندسہ_چوڑائی()), حد_توں_ودھ);
}

APInt APInt::sshl_ov(بنانشانی ShAmt, منطقي &حد_توں_ودھ) ثابت {
  حد_توں_ودھ = ShAmt >= get_ثنائی_ہندسہ_چوڑائی();
  إذا (حد_توں_ودھ)
    أرجع APInt(ثنائی_ہندسہ_چوڑائی_2, 0);

  إذا (isNonNegative()) // Don't allow sign change.
    حد_توں_ودھ = ShAmt >= countl_zero();
  وإلا
    حد_توں_ودھ = ShAmt >= countl_one();

  أرجع *هذا << ShAmt;
}

APInt APInt::ushl_ov(ثابت APInt &ShAmt, منطقي &حد_توں_ودھ) ثابت {
  أرجع ushl_ov(ShAmt.get_Limited_قدر(get_ثنائی_ہندسہ_چوڑائی()), حد_توں_ودھ);
}

APInt APInt::ushl_ov(بنانشانی ShAmt, منطقي &حد_توں_ودھ) ثابت {
  حد_توں_ودھ = ShAmt >= get_ثنائی_ہندسہ_چوڑائی();
  إذا (حد_توں_ودھ)
    أرجع APInt(ثنائی_ہندسہ_چوڑائی_2, 0);

  حد_توں_ودھ = ShAmt > countl_zero();

  أرجع *هذا << ShAmt;
}

APInt APInt::sfloordiv_ov(ثابت APInt &RHS, منطقي &حد_توں_ودھ) ثابت {
  APInt quotient = sdiv_ov(RHS, حد_توں_ودھ);
  إذا ((quotient * RHS != *هذا) && (isNegative() != RHS.isNegative()))
    أرجع quotient - 1;
  أرجع quotient;
}

APInt APInt::sadd_sat(ثابت APInt &RHS) ثابت {
  منطقي حد_توں_ودھ;
  APInt Res = sadd_ov(RHS, حد_توں_ودھ);
  إذا (!حد_توں_ودھ)
    أرجع Res;

  أرجع isNegative() ? APInt::get_Signed_Min_قدر(ثنائی_ہندسہ_چوڑائی_2)
                      : APInt::get_Signed_Max_قدر(ثنائی_ہندسہ_چوڑائی_2);
}

APInt APInt::uadd_sat(ثابت APInt &RHS) ثابت {
  منطقي حد_توں_ودھ;
  APInt Res = uadd_ov(RHS, حد_توں_ودھ);
  إذا (!حد_توں_ودھ)
    أرجع Res;

  أرجع APInt::get_Max_قدر(ثنائی_ہندسہ_چوڑائی_2);
}

APInt APInt::ssub_sat(ثابت APInt &RHS) ثابت {
  منطقي حد_توں_ودھ;
  APInt Res = ssub_ov(RHS, حد_توں_ودھ);
  إذا (!حد_توں_ودھ)
    أرجع Res;

  أرجع isNegative() ? APInt::get_Signed_Min_قدر(ثنائی_ہندسہ_چوڑائی_2)
                      : APInt::get_Signed_Max_قدر(ثنائی_ہندسہ_چوڑائی_2);
}

APInt APInt::usub_sat(ثابت APInt &RHS) ثابت {
  منطقي حد_توں_ودھ;
  APInt Res = usub_ov(RHS, حد_توں_ودھ);
  إذا (!حد_توں_ودھ)
    أرجع Res;

  أرجع APInt(ثنائی_ہندسہ_چوڑائی_2, 0);
}

APInt APInt::smul_sat(ثابت APInt &RHS) ثابت {
  منطقي حد_توں_ودھ;
  APInt Res = smul_ov(RHS, حد_توں_ودھ);
  إذا (!حد_توں_ودھ)
    أرجع Res;

  // The result is negative if one and only one of inputs is negative.
  منطقي ResIsNegative = isNegative() ^ RHS.isNegative();

  أرجع ResIsNegative ? APInt::get_Signed_Min_قدر(ثنائی_ہندسہ_چوڑائی_2)
                       : APInt::get_Signed_Max_قدر(ثنائی_ہندسہ_چوڑائی_2);
}

APInt APInt::umul_sat(ثابت APInt &RHS) ثابت {
  منطقي حد_توں_ودھ;
  APInt Res = umul_ov(RHS, حد_توں_ودھ);
  إذا (!حد_توں_ودھ)
    أرجع Res;

  أرجع APInt::get_Max_قدر(ثنائی_ہندسہ_چوڑائی_2);
}

APInt APInt::sshl_sat(ثابت APInt &RHS) ثابت {
  أرجع sshl_sat(RHS.get_Limited_قدر(get_ثنائی_ہندسہ_چوڑائی()));
}

APInt APInt::sshl_sat(بنانشانی RHS) ثابت {
  منطقي حد_توں_ودھ;
  APInt Res = sshl_ov(RHS, حد_توں_ودھ);
  إذا (!حد_توں_ودھ)
    أرجع Res;

  أرجع isNegative() ? APInt::get_Signed_Min_قدر(ثنائی_ہندسہ_چوڑائی_2)
                      : APInt::get_Signed_Max_قدر(ثنائی_ہندسہ_چوڑائی_2);
}

APInt APInt::ushl_sat(ثابت APInt &RHS) ثابت {
  أرجع ushl_sat(RHS.get_Limited_قدر(get_ثنائی_ہندسہ_چوڑائی()));
}

APInt APInt::ushl_sat(بنانشانی RHS) ثابت {
  منطقي حد_توں_ودھ;
  APInt Res = ushl_ov(RHS, حد_توں_ودھ);
  إذا (!حد_توں_ودھ)
    أرجع Res;

  أرجع APInt::get_Max_قدر(ثنائی_ہندسہ_چوڑائی_2);
}

فراغ APInt::fromString(بنانشانی numbits, StringRef str, uint8_t radix) {
  // Check our assumptions here
  أكد(!str.خالی_تھاواں() && "Invalid string length");
  أكد((radix == 10 || radix == 8 || radix == 16 || radix == 2 ||
          radix == 36) &&
         "Radix should be 2, 8, 10, 16, or 36!");

  StringRef::iterator p = str.begin();
  size_t slen = str.ناپ_2();
  منطقي isNeg = *p == '-';
  إذا (*p == '-' || *p == '+') {
    p++;
    slen--;
    أكد(slen && "String is only a sign, needs a value.");
  }
  أكد((slen <= numbits || radix != 2) && "Insufficient bit width");
  أكد(((slen-1)*3 <= numbits || radix != 8) && "Insufficient bit width");
  أكد(((slen-1)*4 <= numbits || radix != 16) && "Insufficient bit width");
  أكد((((slen-1)*64)/22 <= numbits || radix != 10) &&
         "Insufficient bit width");

  // Allocate memory if needed
  إذا (LLVM_LIKELY(is_Single_لفظ()))
    U.VAL = 0;
  وإلا
    U.pVal = get_Cleared_حافظہ(get_عدد_الفاظ());

  // Figure out if we can shift instead of multiply
  بنانشانی کھسکاؤ = (radix == 16 ? 4 : radix == 8 ? 3 : radix == 2 ? 1 : 0);

  // Enter digit traversal loop
  لكل (StringRef::iterator e = str.سرا(); p != e; ++p) {
    بنانشانی digit = getDigit(*p, radix);
    أكد(digit < radix && "Invalid character in digit string");

    // Shift or multiply the value by the radix
    إذا (slen > 1) {
      إذا (کھسکاؤ)
        *هذا <<= کھسکاؤ;
      وإلا
        *هذا *= radix;
    }

    // Add in the digit we just interpreted
    *هذا += digit;
  }
  // If its negative, put it in two's complement form
  إذا (isNeg)
    هذا->negate();
}

فراغ APInt::منزل_String(Small_سمتیہ_Impl<حرف> &Str, بنانشانی Radix, منطقي Signed,
                     منطقي formatAsCLiteral, منطقي اتلا_Case_2,
                     منطقي داخل_کرو_Separators) ثابت {
  أكد((Radix == 10 || Radix == 8 || Radix == 16 || Radix == 2 ||
          Radix == 36) &&
         "Radix should be 2, 8, 10, 16, or 36!");

  ثابت حرف *اگیتر = "";
  إذا (formatAsCLiteral) {
    بدل (Radix) {
      حالة 2:
        // Binary literals are a non-standard extension added in gcc 4.3:
        // http://gcc.gnu.org/onlinedocs/gcc-4.3.0/gcc/Binary-constants.html
        اگیتر = "0b";
        اكسر;
      حالة 8:
        اگیتر = "0";
        اكسر;
      حالة 10:
        اكسر; // No prefix
      حالة 16:
        اگیتر = "0x";
        اكسر;
      افتراضي:
        llvm_unreachable("Invalid radix!");
    }
  }

  // Number of digits in a group between separators.
  بنانشانی Grouping = (Radix == 8 || Radix == 10) ? 3 : 4;

  // First, check for a zero value and just short circuit the logic below.
  إذا (isZero()) {
    طالما (*اگیتر) {
      Str.ڈالو_پٹھ(*اگیتر);
      ++اگیتر;
    };
    Str.ڈالو_پٹھ('0');
    أرجع;
  }

  ساكن ثابت حرف BothDigits[] = "0123456789abcdefghijklmnopqrstuvwxyz"
                                   "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ";
  ثابت حرف *Digits = BothDigits + (اتلا_Case_2 ? 36 : 0);

  إذا (LLVM_LIKELY(is_Single_لفظ())) {
    حرف بفر[65];
    حرف *BufPtr = std::سرا(بفر);

    uint64_t N;
    إذا (!Signed) {
      N = get_ZExt_قدر();
    } وإلا {
      int64_t I = get_SExt_قدر();
      إذا (I >= 0) {
        N = I;
      } وإلا {
        Str.ڈالو_پٹھ('-');
        N = -(uint64_t)I;
      }
    }

    طالما (*اگیتر) {
      Str.ڈالو_پٹھ(*اگیتر);
      ++اگیتر;
    };

    صحيح Pos = 0;
    طالما (N) {
      إذا (داخل_کرو_Separators && Pos % Grouping == 0 && Pos > 0)
        *--BufPtr = '\'';
      *--BufPtr = Digits[N % Radix];
      N /= Radix;
      Pos++;
    }
    Str.append(BufPtr, std::سرا(بفر));
    أرجع;
  }

  APInt Tmp(*هذا);

  إذا (Signed && isNegative()) {
    // They want to print the signed version and it is a negative value
    // Flip the bits and add one to turn it into the equivalent positive
    // value and put a '-' in the result.
    Tmp.negate();
    Str.ڈالو_پٹھ('-');
  }

  طالما (*اگیتر) {
    Str.ڈالو_پٹھ(*اگیتر);
    ++اگیتر;
  }

  // We insert the digits backward, then reverse them to get the right order.
  بنانشانی شروع_Dig = Str.ناپ_2();

  // For the 2, 8 and 16 bit cases, we can just shift instead of divide
  // because the number of bits per digit (1, 3 and 4 respectively) divides
  // equally.  We just shift until the value is zero.
  إذا (Radix == 2 || Radix == 8 || Radix == 16) {
    // Just shift tmp right for each digit width until it becomes zero
    بنانشانی کھسکاؤ_Amt_2 = (Radix == 16 ? 4 : (Radix == 8 ? 3 : 1));
    بنانشانی نقاب_Amt = Radix - 1;

    صحيح Pos = 0;
    طالما (Tmp.get_Bool_قدر()) {
      بنانشانی Digit = بنانشانی(Tmp.get_Raw_انکڑے()[0]) & نقاب_Amt;
      إذا (داخل_کرو_Separators && Pos % Grouping == 0 && Pos > 0)
        Str.ڈالو_پٹھ('\'');

      Str.ڈالو_پٹھ(Digits[Digit]);
      Tmp.lshr_وچ_رکھو(کھسکاؤ_Amt_2);
      Pos++;
    }
  } وإلا {
    صحيح Pos = 0;
    طالما (Tmp.get_Bool_قدر()) {
      uint64_t Digit;
      udivrem(Tmp, Radix, Tmp, Digit);
      أكد(Digit < Radix && "divide failed");
      إذا (داخل_کرو_Separators && Pos % Grouping == 0 && Pos > 0)
        Str.ڈالو_پٹھ('\'');

      Str.ڈالو_پٹھ(Digits[Digit]);
      Pos++;
    }
  }

  // Reverse the digits before returning.
  std::الٹ(Str.begin()+شروع_Dig, Str.سرا());
}

#إذا !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)
LLVM_DUMP_طریقہ فراغ APInt::dump() ثابت {
  SmallString<40> S, U;
  هذا->منزل_String_Unsigned(U);
  هذا->منزل_String_Signed(S);
  dbgs() << "APInt(" << ثنائی_ہندسہ_چوڑائی_2 << "b, "
         << U << "u " << S << "s)\n";
}
#جے_انت

فراغ APInt::print(raw_ostream &OS, منطقي isSigned) ثابت {
  SmallString<40> S;
  هذا->منزل_String(S, 10, isSigned, /* formatAsCLiteral = */خطأ);
  OS << S;
}

// This implements a variety of operations on a representation of
// arbitrary precision, two's-complement, bignum integer values.

// Assumed by lowHalf, highHalf, partMSB and partLSB.  A fairly safe
// and unrestricting assumption.
static_assert(APInt::APINT_بٹ_فی_لفظ % 2 == 0,
              "Part width must be divisible by 2!");

// Returns the integer part with the least significant BITS set.
// BITS cannot be zero.
ساكن مضمن APInt::لفظ_Type کم_ثنائی_ہندسہ_نقاب(بنانشانی بٹ) {
  أكد(بٹ != 0 && بٹ <= APInt::APINT_بٹ_فی_لفظ);
  أرجع ~(APInt::لفظ_Type) 0 >> (APInt::APINT_بٹ_فی_لفظ - بٹ);
}

/// Returns the value of the lower half of PART.
ساكن مضمن APInt::لفظ_Type کم_Half(APInt::لفظ_Type part) {
  أرجع part & کم_ثنائی_ہندسہ_نقاب(APInt::APINT_بٹ_فی_لفظ / 2);
}

/// Returns the value of the upper half of PART.
ساكن مضمن APInt::لفظ_Type بلند_Half(APInt::لفظ_Type part) {
  أرجع part >> (APInt::APINT_بٹ_فی_لفظ / 2);
}

/// Sets the least significant part of a bignum to the input value, and zeroes
/// out higher parts.
فراغ APInt::tc_سیٹ(لفظ_Type *dst, لفظ_Type part, بنانشانی parts) {
  أكد(parts > 0);
  dst[0] = part;
  لكل (بنانشانی i = 1; i < parts; i++)
    dst[i] = 0;
}

/// Assign one bignum to another.
فراغ APInt::tc_سونپو(لفظ_Type *dst, ثابت لفظ_Type *src, بنانشانی parts) {
  لكل (بنانشانی i = 0; i < parts; i++)
    dst[i] = src[i];
}

/// Returns true if a bignum is zero, false otherwise.
منطقي APInt::tcIsZero(ثابت لفظ_Type *src, بنانشانی parts) {
  لكل (بنانشانی i = 0; i < parts; i++)
    إذا (src[i])
      أرجع خطأ;

  أرجع سچ;
}

/// Extract the given bit of a bignum; returns 0 or 1.
صحيح APInt::tc_کڈھو_ثنائی_ہندسہ(ثابت لفظ_Type *parts, بنانشانی ثنائی_ہندسہ) {
  أرجع (parts[which_لفظ(ثنائی_ہندسہ)] & نقاب_ثنائی_ہندسہ(ثنائی_ہندسہ)) != 0;
}

/// Set the given bit of a bignum.
فراغ APInt::tc_سیٹ_ثنائی_ہندسہ(لفظ_Type *parts, بنانشانی ثنائی_ہندسہ) {
  parts[which_لفظ(ثنائی_ہندسہ)] |= نقاب_ثنائی_ہندسہ(ثنائی_ہندسہ);
}

/// Clears the given bit of a bignum.
فراغ APInt::tc_Clear_ثنائی_ہندسہ(لفظ_Type *parts, بنانشانی ثنائی_ہندسہ) {
  parts[which_لفظ(ثنائی_ہندسہ)] &= ~نقاب_ثنائی_ہندسہ(ثنائی_ہندسہ);
}

/// Returns the bit number of the least significant set bit of a number.  If the
/// input number has no bits set UINT_MAX is returned.
بنانشانی APInt::tcLSB(ثابت لفظ_Type *parts, بنانشانی n) {
  لكل (بنانشانی i = 0; i < n; i++) {
    إذا (parts[i] != 0) {
      بنانشانی lsb = llvm::countr_zero(parts[i]);
      أرجع lsb + i * APINT_بٹ_فی_لفظ;
    }
  }

  أرجع UINT_MAX;
}

/// Returns the bit number of the most significant set bit of a number.
/// If the input number has no bits set UINT_MAX is returned.
بنانشانی APInt::tcMSB(ثابت لفظ_Type *parts, بنانشانی n) {
  افعل {
    --n;

    إذا (parts[n] != 0) {
      static_assert(حجم(parts[n]) <= حجم(uint64_t));
      بنانشانی msb = llvm::Log2_64(parts[n]);

      أرجع msb + n * APINT_بٹ_فی_لفظ;
    }
  } طالما (n);

  أرجع UINT_MAX;
}

/// Copy the bit vector of width srcBITS from SRC, starting at bit srcLSB, to
/// DST, of dstCOUNT parts, such that the bit srcLSB becomes the least
/// significant bit of DST.  All high bits above srcBITS in DST are zero-filled.
/// */
فراغ
APInt::tc_کڈھو(لفظ_Type *dst, بنانشانی dst_گنتی, ثابت لفظ_Type *src,
                 بنانشانی src_بٹ, بنانشانی srcLSB) {
  بنانشانی dstParts = (src_بٹ + APINT_بٹ_فی_لفظ - 1) / APINT_بٹ_فی_لفظ;
  أكد(dstParts <= dst_گنتی);

  بنانشانی پہلا_Src_Part = srcLSB / APINT_بٹ_فی_لفظ;
  tc_سونپو(dst, src + پہلا_Src_Part, dstParts);

  بنانشانی کھسکاؤ = srcLSB % APINT_بٹ_فی_لفظ;
  tc_کھسکاؤ_دایاں(dst, dstParts, کھسکاؤ);

  // We now have (dstParts * APINT_BITS_PER_WORD - shift) bits from SRC
  // in DST.  If this is less that srcBits, append the rest, else
  // clear the high bits.
  بنانشانی n = dstParts * APINT_بٹ_فی_لفظ - کھسکاؤ;
  إذا (n < src_بٹ) {
    لفظ_Type نقاب_2 = کم_ثنائی_ہندسہ_نقاب (src_بٹ - n);
    dst[dstParts - 1] |= ((src[پہلا_Src_Part + dstParts] & نقاب_2)
                          << n % APINT_بٹ_فی_لفظ);
  } وإلا إذا (n > src_بٹ) {
    إذا (src_بٹ % APINT_بٹ_فی_لفظ)
      dst[dstParts - 1] &= کم_ثنائی_ہندسہ_نقاب (src_بٹ % APINT_بٹ_فی_لفظ);
  }

  // Clear high parts.
  طالما (dstParts < dst_گنتی)
    dst[dstParts++] = 0;
}

//// DST += RHS + C where C is zero or one.  Returns the carry flag.
APInt::لفظ_Type APInt::tc_جوڑو(لفظ_Type *dst, ثابت لفظ_Type *rhs,
                             لفظ_Type c, بنانشانی parts) {
  أكد(c <= 1);

  لكل (بنانشانی i = 0; i < parts; i++) {
    لفظ_Type l = dst[i];
    إذا (c) {
      dst[i] += rhs[i] + 1;
      c = (dst[i] <= l);
    } وإلا {
      dst[i] += rhs[i];
      c = (dst[i] < l);
    }
  }

  أرجع c;
}

/// This function adds a single "word" integer, src, to the multiple
/// "word" integer array, dst[]. dst[] is modified to reflect the addition and
/// 1 is returned if there is a carry out, otherwise 0 is returned.
/// @returns the carry of the addition.
APInt::لفظ_Type APInt::tc_جوڑو_Part(لفظ_Type *dst, لفظ_Type src,
                                 بنانشانی parts) {
  لكل (بنانشانی i = 0; i < parts; ++i) {
    dst[i] += src;
    إذا (dst[i] >= src)
      أرجع 0; // No need to carry so exit early.
    src = 1; // Carry one to next digit.
  }

  أرجع 1;
}

/// DST -= RHS + C where C is zero or one.  Returns the carry flag.
APInt::لفظ_Type APInt::tc_گھٹاؤ(لفظ_Type *dst, ثابت لفظ_Type *rhs,
                                  لفظ_Type c, بنانشانی parts) {
  أكد(c <= 1);

  لكل (بنانشانی i = 0; i < parts; i++) {
    لفظ_Type l = dst[i];
    إذا (c) {
      dst[i] -= rhs[i] + 1;
      c = (dst[i] >= l);
    } وإلا {
      dst[i] -= rhs[i];
      c = (dst[i] > l);
    }
  }

  أرجع c;
}

/// This function subtracts a single "word" (64-bit word), src, from
/// the multi-word integer array, dst[], propagating the borrowed 1 value until
/// no further borrowing is needed or it runs out of "words" in dst.  The result
/// is 1 if "borrowing" exhausted the digits in dst, or 0 if dst was not
/// exhausted. In other words, if src > dst then this function returns 1,
/// otherwise 0.
/// @returns the borrow out of the subtraction
APInt::لفظ_Type APInt::tc_گھٹاؤ_Part(لفظ_Type *dst, لفظ_Type src,
                                      بنانشانی parts) {
  لكل (بنانشانی i = 0; i < parts; ++i) {
    لفظ_Type Dst = dst[i];
    dst[i] -= src;
    إذا (src <= Dst)
      أرجع 0; // No need to borrow so exit early.
    src = 1; // We have to "borrow 1" from next "word"
  }

  أرجع 1;
}

/// Negate a bignum in-place.
فراغ APInt::tcNegate(لفظ_Type *dst, بنانشانی parts) {
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
صحيح APInt::tcMultiplyPart(لفظ_Type *dst, ثابت لفظ_Type *src,
                          لفظ_Type multiplier, لفظ_Type حاصل,
                          بنانشانی srcParts, بنانشانی dstParts,
                          منطقي جوڑو) {
  // Otherwise our writes of DST kill our later reads of SRC.
  أكد(dst <= src || dst >= src + srcParts);
  أكد(dstParts <= srcParts + 1);

  // N loops; minimum of dstParts and srcParts.
  بنانشانی n = std::min(dstParts, srcParts);

  لكل (بنانشانی i = 0; i < n; i++) {
    // [LOW, HIGH] = MULTIPLIER * SRC[i] + DST[i] + CARRY.
    // This cannot overflow, because:
    //   (n - 1) * (n - 1) + 2 (n - 1) = (n - 1) * (n + 1)
    // which is less than n^2.
    لفظ_Type srcPart = src[i];
    لفظ_Type کم, mid, بلند;
    إذا (multiplier == 0 || srcPart == 0) {
      کم = حاصل;
      بلند = 0;
    } وإلا {
      کم = کم_Half(srcPart) * کم_Half(multiplier);
      بلند = بلند_Half(srcPart) * بلند_Half(multiplier);

      mid = کم_Half(srcPart) * بلند_Half(multiplier);
      بلند += بلند_Half(mid);
      mid <<= APINT_بٹ_فی_لفظ / 2;
      إذا (کم + mid < کم)
        بلند++;
      کم += mid;

      mid = بلند_Half(srcPart) * کم_Half(multiplier);
      بلند += بلند_Half(mid);
      mid <<= APINT_بٹ_فی_لفظ / 2;
      إذا (کم + mid < کم)
        بلند++;
      کم += mid;

      // Now add carry.
      إذا (کم + حاصل < کم)
        بلند++;
      کم += حاصل;
    }

    إذا (جوڑو) {
      // And now DST[i], and store the new low part there.
      إذا (کم + dst[i] < کم)
        بلند++;
      dst[i] += کم;
    } وإلا {
      dst[i] = کم;
    }

    حاصل = بلند;
  }

  إذا (srcParts < dstParts) {
    // Full multiplication, there is no overflow.
    أكد(srcParts + 1 == dstParts);
    dst[srcParts] = حاصل;
    أرجع 0;
  }

  // We overflowed if there is carry.
  إذا (حاصل)
    أرجع 1;

  // We would overflow if any significant unwritten parts would be
  // non-zero.  This is true if any remaining src parts are non-zero
  // and the multiplier is non-zero.
  إذا (multiplier)
    لكل (بنانشانی i = dstParts; i < srcParts; i++)
      إذا (src[i])
        أرجع 1;

  // We fitted in the narrow destination.
  أرجع 0;
}

/// DST = LHS * RHS, where DST has the same width as the operands and
/// is filled with the least significant parts of the result.  Returns
/// one if overflow occurred, otherwise zero.  DST must be disjoint
/// from both operands.
صحيح APInt::tcMultiply(لفظ_Type *dst, ثابت لفظ_Type *lhs,
                      ثابت لفظ_Type *rhs, بنانشانی parts) {
  أكد(dst != lhs && dst != rhs);

  صحيح حد_توں_ودھ_2 = 0;

  لكل (بنانشانی i = 0; i < parts; i++) {
    // Don't accumulate on the first iteration so we don't need to initalize
    // dst to 0.
    حد_توں_ودھ_2 |=
        tcMultiplyPart(&dst[i], lhs, rhs[i], 0, parts, parts - i, i != 0);
  }

  أرجع حد_توں_ودھ_2;
}

/// DST = LHS * RHS, where DST has width the sum of the widths of the
/// operands. No overflow occurs. DST must be disjoint from both operands.
فراغ APInt::tc_بھریا_Multiply(لفظ_Type *dst, ثابت لفظ_Type *lhs,
                           ثابت لفظ_Type *rhs, بنانشانی lhsParts,
                           بنانشانی rhsParts) {
  // Put the narrower number on the LHS for less loops below.
  إذا (lhsParts > rhsParts)
    أرجع tc_بھریا_Multiply (dst, rhs, lhs, rhsParts, lhsParts);

  أكد(dst != lhs && dst != rhs);

  لكل (بنانشانی i = 0; i < lhsParts; i++) {
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
صحيح APInt::tcDivide(لفظ_Type *lhs, ثابت لفظ_Type *rhs,
                    لفظ_Type *remainder, لفظ_Type *srhs,
                    بنانشانی parts) {
  أكد(lhs != remainder && lhs != srhs && remainder != srhs);

  بنانشانی کھسکاؤ_گنتی = tcMSB(rhs, parts) + 1;
  إذا (کھسکاؤ_گنتی == 0)
    أرجع سچ;

  کھسکاؤ_گنتی = parts * APINT_بٹ_فی_لفظ - کھسکاؤ_گنتی;
  بنانشانی n = کھسکاؤ_گنتی / APINT_بٹ_فی_لفظ;
  لفظ_Type نقاب_2 = (لفظ_Type) 1 << (کھسکاؤ_گنتی % APINT_بٹ_فی_لفظ);

  tc_سونپو(srhs, rhs, parts);
  tc_کھسکاؤ_بایاں(srhs, parts, کھسکاؤ_گنتی);
  tc_سونپو(remainder, lhs, parts);
  tc_سیٹ(lhs, 0, parts);

  // Loop, subtracting SRHS if REMAINDER is greater and adding that to the
  // total.
  لكل (;;) {
    صحيح compare = tcCompare(remainder, srhs, parts);
    إذا (compare >= 0) {
      tc_گھٹاؤ(remainder, srhs, 0, parts);
      lhs[n] |= نقاب_2;
    }

    إذا (کھسکاؤ_گنتی == 0)
      اكسر;
    کھسکاؤ_گنتی--;
    tc_کھسکاؤ_دایاں(srhs, parts, 1);
    إذا ((نقاب_2 >>= 1) == 0) {
      نقاب_2 = (لفظ_Type) 1 << (APINT_بٹ_فی_لفظ - 1);
      n--;
    }
  }

  أرجع خطأ;
}

/// Shift a bignum left Count bits in-place. Shifted in bits are zero. There are
/// no restrictions on Count.
فراغ APInt::tc_کھسکاؤ_بایاں(لفظ_Type *Dst, بنانشانی الفاظ_2, بنانشانی گنتی_2) {
  // Don't bother performing a no-op shift.
  إذا (!گنتی_2)
    أرجع;

  // WordShift is the inter-part shift; BitShift is the intra-part shift.
  بنانشانی لفظ_کھسکاؤ = std::min(گنتی_2 / APINT_بٹ_فی_لفظ, الفاظ_2);
  بنانشانی ثنائی_ہندسہ_کھسکاؤ = گنتی_2 % APINT_بٹ_فی_لفظ;

  // Fastpath for moving by whole words.
  إذا (ثنائی_ہندسہ_کھسکاؤ == 0) {
    std::memmove(Dst + لفظ_کھسکاؤ, Dst, (الفاظ_2 - لفظ_کھسکاؤ) * APINT_لفظ_ناپ);
  } وإلا {
    طالما (الفاظ_2-- > لفظ_کھسکاؤ) {
      Dst[الفاظ_2] = Dst[الفاظ_2 - لفظ_کھسکاؤ] << ثنائی_ہندسہ_کھسکاؤ;
      إذا (الفاظ_2 > لفظ_کھسکاؤ)
        Dst[الفاظ_2] |=
          Dst[الفاظ_2 - لفظ_کھسکاؤ - 1] >> (APINT_بٹ_فی_لفظ - ثنائی_ہندسہ_کھسکاؤ);
    }
  }

  // Fill in the remainder with 0s.
  std::memset(Dst, 0, لفظ_کھسکاؤ * APINT_لفظ_ناپ);
}

/// Shift a bignum right Count bits in-place. Shifted in bits are zero. There
/// are no restrictions on Count.
فراغ APInt::tc_کھسکاؤ_دایاں(لفظ_Type *Dst, بنانشانی الفاظ_2, بنانشانی گنتی_2) {
  // Don't bother performing a no-op shift.
  إذا (!گنتی_2)
    أرجع;

  // WordShift is the inter-part shift; BitShift is the intra-part shift.
  بنانشانی لفظ_کھسکاؤ = std::min(گنتی_2 / APINT_بٹ_فی_لفظ, الفاظ_2);
  بنانشانی ثنائی_ہندسہ_کھسکاؤ = گنتی_2 % APINT_بٹ_فی_لفظ;

  بنانشانی الفاظ_منزل_Move = الفاظ_2 - لفظ_کھسکاؤ;
  // Fastpath for moving by whole words.
  إذا (ثنائی_ہندسہ_کھسکاؤ == 0) {
    std::memmove(Dst, Dst + لفظ_کھسکاؤ, الفاظ_منزل_Move * APINT_لفظ_ناپ);
  } وإلا {
    لكل (بنانشانی i = 0; i != الفاظ_منزل_Move; ++i) {
      Dst[i] = Dst[i + لفظ_کھسکاؤ] >> ثنائی_ہندسہ_کھسکاؤ;
      إذا (i + 1 != الفاظ_منزل_Move)
        Dst[i] |= Dst[i + لفظ_کھسکاؤ + 1] << (APINT_بٹ_فی_لفظ - ثنائی_ہندسہ_کھسکاؤ);
    }
  }

  // Fill in the remainder with 0s.
  std::memset(Dst + الفاظ_منزل_Move, 0, لفظ_کھسکاؤ * APINT_لفظ_ناپ);
}

// Comparison (unsigned) of two bignums.
صحيح APInt::tcCompare(ثابت لفظ_Type *lhs, ثابت لفظ_Type *rhs,
                     بنانشانی parts) {
  طالما (parts) {
    parts--;
    إذا (lhs[parts] != rhs[parts])
      أرجع (lhs[parts] > rhs[parts]) ? 1 : -1;
  }

  أرجع 0;
}

APInt llvm::APIntOps::RoundingUDiv(ثابت APInt &A, ثابت APInt &B,
                                   APInt::Rounding RM) {
  // Currently udivrem always rounds down.
  بدل (RM) {
  حالة APInt::Rounding::تھلے:
  حالة APInt::Rounding::TOWARD_ZERO:
    أرجع A.udiv(B);
  حالة APInt::Rounding::اتے: {
    APInt Quo, Rem;
    APInt::udivrem(A, B, Quo, Rem);
    إذا (Rem.isZero())
      أرجع Quo;
    أرجع Quo + 1;
  }
  }
  llvm_unreachable("Unknown APInt::Rounding enum");
}

APInt llvm::APIntOps::RoundingSDiv(ثابت APInt &A, ثابت APInt &B,
                                   APInt::Rounding RM) {
  بدل (RM) {
  حالة APInt::Rounding::تھلے:
  حالة APInt::Rounding::اتے: {
    APInt Quo, Rem;
    APInt::sdivrem(A, B, Quo, Rem);
    إذا (Rem.isZero())
      أرجع Quo;
    // This algorithm deals with arbitrary rounding mode used by sdivrem.
    // We want to check whether the non-integer part of the mathematical value
    // is negative or not. If the non-integer part is negative, we need to round
    // down from Quo; otherwise, if it's positive or 0, we return Quo, as it's
    // already rounded down.
    إذا (RM == APInt::Rounding::تھلے) {
      إذا (Rem.isNegative() != B.isNegative())
        أرجع Quo - 1;
      أرجع Quo;
    }
    إذا (Rem.isNegative() != B.isNegative())
      أرجع Quo;
    أرجع Quo + 1;
  }
  // Currently sdiv rounds towards zero.
  حالة APInt::Rounding::TOWARD_ZERO:
    أرجع A.sdiv(B);
  }
  llvm_unreachable("Unknown APInt::Rounding enum");
}

std::optional<APInt>
llvm::APIntOps::SolveQuadraticEquationWrap(APInt A, APInt B, APInt C,
                                           بنانشانی Range_چوڑائی) {
  بنانشانی Coeff_چوڑائی = A.get_ثنائی_ہندسہ_چوڑائی();
  أكد(Coeff_چوڑائی == B.get_ثنائی_ہندسہ_چوڑائی() && Coeff_چوڑائی == C.get_ثنائی_ہندسہ_چوڑائی());
  أكد(Range_چوڑائی <= Coeff_چوڑائی &&
         "Value range width should be less than coefficient width");
  أكد(Range_چوڑائی > 1 && "Value range bit width should be > 1");

  LLVM_DEBUG(dbgs() << __func__ << ": solving " << A << "x^2 + " << B
                    << "x + " << C << ", rw:" << Range_چوڑائی << '\n');

  // Identify 0 as a (non)solution immediately.
  إذا (C.sextOrTrunc(Range_چوڑائی).isZero()) {
    LLVM_DEBUG(dbgs() << __func__ << ": zero solution\n");
    أرجع APInt(Coeff_چوڑائی, 0);
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
  Coeff_چوڑائی *= 3;
  A = A.sext(Coeff_چوڑائی);
  B = B.sext(Coeff_چوڑائی);
  C = C.sext(Coeff_چوڑائی);

  // Make A > 0 for simplicity. Negate cannot overflow at this point because
  // the bit width has increased.
  إذا (A.isNegative()) {
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
  APInt R = APInt::get_One_ثنائی_ہندسہ_سیٹ(Coeff_چوڑائی, Range_چوڑائی);
  APInt TwoA = 2 * A;
  APInt SqrB = B * B;
  منطقي Pick_کم;

  خودکار گیڑا_اتے_2 = [] (ثابت APInt &V, ثابت APInt &A) -> APInt {
    أكد(A.is_Strictly_مثبت());
    APInt T = V.مطلق_قدر().urem(A);
    إذا (T.isZero())
      أرجع V;
    أرجع V.isNegative() ? V+T : V+(A-T);
  };

  // The vertex of the parabola is at -B/2A, but since A > 0, it's negative
  // iff B is positive.
  إذا (B.isNonNegative()) {
    // If B >= 0, the vertex it at a negative location (or at 0), so in
    // order to have a non-negative solution we need to pick k that makes
    // C-kR negative. To satisfy all the requirements for the solution
    // that we are looking for, it needs to be closest to 0 of all k.
    C = C.srem(R);
    إذا (C.is_Strictly_مثبت())
      C -= R;
    // Pick the greater solution.
    Pick_کم = خطأ;
  } وإلا {
    // If B < 0, the vertex is at a positive location. For any solution
    // to exist, the discriminant must be non-negative. This means that
    // C-kR <= B^2/4A is a necessary condition for k, i.e. there is a
    // lower bound on values of k: kR >= C - B^2/4A.
    APInt LowkR = C - SqrB.udiv(2*TwoA); // udiv because all values > 0.
    // Round LowkR up (towards +inf) to the nearest kR.
    LowkR = گیڑا_اتے_2(LowkR, R);

    // If there exists k meeting the condition above, and such that
    // C-kR > 0, there will be two positive real number solutions of
    // q(x) = kR. Out of all such values of k, pick the one that makes
    // C-kR closest to 0, (i.e. pick maximum k such that C-kR > 0).
    // In other words, find maximum k such that LowkR <= kR < C.
    إذا (C.sgt(LowkR)) {
      // If LowkR < C, then such a k is guaranteed to exist because
      // LowkR itself is a multiple of R.
      C -= -گیڑا_اتے_2(-C, R);      // C = C - RoundDown(C, R)
      // Pick the smaller solution.
      Pick_کم = سچ;
    } وإلا {
      // If C-kR < 0 for all potential k's, it means that one solution
      // will be negative, while the other will be positive. The positive
      // solution will shift towards 0 if the parabola is moved up.
      // Pick the kR closest to the lower bound (i.e. make C-kR closest
      // to 0, or in other words, out of all parabolas that have solutions,
      // pick the one that is the farthest "up").
      // Since LowkR is itself a multiple of R, simply take C-LowkR.
      C -= LowkR;
      // Pick the greater solution.
      Pick_کم = خطأ;
    }
  }

  LLVM_DEBUG(dbgs() << __func__ << ": updated coefficients " << A << "x^2 + "
                    << B << "x + " << C << ", rw:" << Range_چوڑائی << '\n');

  APInt D = SqrB - 4*A*C;
  أكد(D.isNonNegative() && "Negative discriminant");
  APInt SQ = D.sqrtFloor();

  APInt Q = SQ * SQ;
  منطقي InexactSQ = Q != D;

  APInt X;
  APInt Rem;

  // SQ is rounded down (i.e SQ * SQ <= D), so the roots may be inexact.
  // When using the quadratic formula directly, the calculated low root
  // may be greater than the exact one, since we would be subtracting SQ.
  // To make sure that the calculated root is not greater than the exact
  // one, subtract SQ+1 when calculating the low root (for inexact value
  // of SQ).
  إذا (Pick_کم)
    APInt::sdivrem(-B - (SQ+InexactSQ), TwoA, X, Rem);
  وإلا
    APInt::sdivrem(-B + SQ, TwoA, X, Rem);

  // The updated coefficients should be such that the (exact) solution is
  // positive. Since APInt division rounds towards 0, the calculated one
  // can be 0, but cannot be negative.
  أكد(X.isNonNegative() && "Solution should be non-negative");

  إذا (!InexactSQ && Rem.isZero()) {
    LLVM_DEBUG(dbgs() << __func__ << ": solution (root): " << X << '\n');
    أرجع X;
  }

  أكد((SQ*SQ).sle(D) && "SQ = |_sqrt(D)_|, so SQ*SQ <= D");
  // The exact value of the square root of D should be between SQ and SQ+1.
  // This implies that the solution should be between that corresponding to
  // SQ (i.e. X) and that corresponding to SQ+1.
  //
  // The calculated X cannot be greater than the exact (real) solution.
  // Actually it must be strictly less than the exact solution, while
  // X+1 will be greater than or equal to it.

  APInt VX = (A*X + B)*X + C;
  APInt VY = VX + TwoA*X + A + B;
  منطقي SignChange =
      VX.isNegative() != VY.isNegative() || VX.isZero() != VY.isZero();
  // If the sign did not change between X and X+1, X is not a valid solution.
  // This could happen when the actual (exact) roots don't have an integer
  // between them, so they would both be contained between X and X+1.
  إذا (!SignChange) {
    LLVM_DEBUG(dbgs() << __func__ << ": no valid solution\n");
    أرجع std::nullopt;
  }

  X += 1;
  LLVM_DEBUG(dbgs() << __func__ << ": solution (wrap): " << X << '\n');
  أرجع X;
}

std::optional<بنانشانی>
llvm::APIntOps::Get_Most_Significant_Different_ثنائی_ہندسہ(ثابت APInt &A, ثابت APInt &B) {
  أكد(A.get_ثنائی_ہندسہ_چوڑائی() == B.get_ثنائی_ہندسہ_چوڑائی() && "Must have the same bitwidth");
  إذا (A == B)
    أرجع std::nullopt;
  أرجع A.get_ثنائی_ہندسہ_چوڑائی() - ((A ^ B).countl_zero() + 1);
}

APInt llvm::APIntOps::Scale_ثنائی_ہندسہ_نقاب(ثابت APInt &A, بنانشانی New_ثنائی_ہندسہ_چوڑائی,
                                   منطقي میل_All_بٹ) {
  بنانشانی Old_ثنائی_ہندسہ_چوڑائی = A.get_ثنائی_ہندسہ_چوڑائی();
  أكد((((Old_ثنائی_ہندسہ_چوڑائی % New_ثنائی_ہندسہ_چوڑائی) == 0) ||
          ((New_ثنائی_ہندسہ_چوڑائی % Old_ثنائی_ہندسہ_چوڑائی) == 0)) &&
         "One size should be a multiple of the other one. "
         "Can't do fractional scaling.");

  // Check for matching bitwidths.
  إذا (Old_ثنائی_ہندسہ_چوڑائی == New_ثنائی_ہندسہ_چوڑائی)
    أرجع A;

  APInt NewA = APInt::getZero(New_ثنائی_ہندسہ_چوڑائی);

  // Check for null input.
  إذا (A.isZero())
    أرجع NewA;

  إذا (New_ثنائی_ہندسہ_چوڑائی > Old_ثنائی_ہندسہ_چوڑائی) {
    // Repeat bits.
    بنانشانی Scale = New_ثنائی_ہندسہ_چوڑائی / Old_ثنائی_ہندسہ_چوڑائی;
    لكل (بنانشانی i = 0; i != Old_ثنائی_ہندسہ_چوڑائی; ++i)
      إذا (A[i])
        NewA.سیٹ_بٹ(i * Scale, (i + 1) * Scale);
  } وإلا {
    بنانشانی Scale = Old_ثنائی_ہندسہ_چوڑائی / New_ثنائی_ہندسہ_چوڑائی;
    لكل (بنانشانی i = 0; i != New_ثنائی_ہندسہ_چوڑائی; ++i) {
      إذا (میل_All_بٹ) {
        إذا (A.کڈھو_بٹ(Scale, i * Scale).isAllOnes())
          NewA.سیٹ_ثنائی_ہندسہ(i);
      } وإلا {
        إذا (!A.کڈھو_بٹ(Scale, i * Scale).isZero())
          NewA.سیٹ_ثنائی_ہندسہ(i);
      }
    }
  }

  أرجع NewA;
}

/// StoreIntToMemory - Fills the StoreBytes bytes of memory starting from Dst
/// with the integer held in IntVal.
فراغ llvm::Store_Int_منزل_حافظہ(ثابت APInt &IntVal, uint8_t *Dst,
                            بنانشانی Store_بائٹاں) {
  أكد((IntVal.get_ثنائی_ہندسہ_چوڑائی()+7)/8 >= Store_بائٹاں && "Integer too small!");
  ثابت uint8_t *Src = (ثابت uint8_t *)IntVal.get_Raw_انکڑے();

  إذا (sys::IsLittleEndianHost) {
    // Little-endian host - the source is ordered from LSB to MSB.  Order the
    // destination from LSB to MSB: Do a straight copy.
    حافظہ_نقل(Dst, Src, Store_بائٹاں);
  } وإلا {
    // Big-endian host - the source is an array of 64 bit words ordered from
    // LSW to MSW.  Each word is ordered from MSB to LSB.  Order the destination
    // from MSB to LSB: Reverse the word order, but not the bytes in a word.
    طالما (Store_بائٹاں > حجم(uint64_t)) {
      Store_بائٹاں -= حجم(uint64_t);
      // May not be aligned so use memcpy.
      حافظہ_نقل(Dst + Store_بائٹاں, Src, حجم(uint64_t));
      Src += حجم(uint64_t);
    }

    حافظہ_نقل(Dst, Src + حجم(uint64_t) - Store_بائٹاں, Store_بائٹاں);
  }
}

/// LoadIntFromMemory - Loads the integer stored in the LoadBytes bytes starting
/// from Src into IntVal, which is assumed to be wide enough and to hold zero.
فراغ llvm::لدو_Int_From_حافظہ(APInt &IntVal, ثابت uint8_t *Src,
                             بنانشانی لدو_بائٹاں) {
  أكد((IntVal.get_ثنائی_ہندسہ_چوڑائی()+7)/8 >= لدو_بائٹاں && "Integer too small!");
  uint8_t *Dst = reinterpret_cast<uint8_t *>(
                   const_cast<uint64_t *>(IntVal.get_Raw_انکڑے()));

  إذا (sys::IsLittleEndianHost)
    // Little-endian host - the destination must be ordered from LSB to MSB.
    // The source is ordered from LSB to MSB: Do a straight copy.
    حافظہ_نقل(Dst, Src, لدو_بائٹاں);
  وإلا {
    // Big-endian - the destination is an array of 64 bit words ordered from
    // LSW to MSW.  Each word must be ordered from MSB to LSB.  The source is
    // ordered from MSB to LSB: Reverse the word order, but not the bytes in
    // a word.
    طالما (لدو_بائٹاں > حجم(uint64_t)) {
      لدو_بائٹاں -= حجم(uint64_t);
      // May not be aligned so use memcpy.
      حافظہ_نقل(Dst, Src + لدو_بائٹاں, حجم(uint64_t));
      Dst += حجم(uint64_t);
    }

    حافظہ_نقل(Dst + حجم(uint64_t) - لدو_بائٹاں, Src, لدو_بائٹاں);
  }
}

APInt APIntOps::avgFloorS(ثابت APInt &C1, ثابت APInt &C2) {
  // Return floor((C1 + C2) / 2)
  أرجع (C1 & C2) + (C1 ^ C2).ashr(1);
}

APInt APIntOps::avgFloorU(ثابت APInt &C1, ثابت APInt &C2) {
  // Return floor((C1 + C2) / 2)
  أرجع (C1 & C2) + (C1 ^ C2).lshr(1);
}

APInt APIntOps::avgCeilS(ثابت APInt &C1, ثابت APInt &C2) {
  // Return ceil((C1 + C2) / 2)
  أرجع (C1 | C2) - (C1 ^ C2).ashr(1);
}

APInt APIntOps::avgCeilU(ثابت APInt &C1, ثابت APInt &C2) {
  // Return ceil((C1 + C2) / 2)
  أرجع (C1 | C2) - (C1 ^ C2).lshr(1);
}

APInt APIntOps::avgSubFloorS(ثابت APInt &C1, ثابت APInt &C2) {
  // Return floor((C1 - C2) / 2), using C1 - C2 == (C1 ^ C2) - 2 * (~C1 & C2).
  أرجع (C1 ^ C2).ashr(1) - (~C1 & C2);
}

APInt APIntOps::avgSubFloorU(ثابت APInt &C1, ثابت APInt &C2) {
  // Return floor((C1 - C2) / 2), using C1 - C2 == (C1 ^ C2) - 2 * (~C1 & C2).
  أرجع (C1 ^ C2).lshr(1) - (~C1 & C2);
}

APInt APIntOps::mulhs(ثابت APInt &C1, ثابت APInt &C2) {
  أكد(C1.get_ثنائی_ہندسہ_چوڑائی() == C2.get_ثنائی_ہندسہ_چوڑائی() && "Unequal bitwidths");
  بنانشانی بھریا_چوڑائی = C1.get_ثنائی_ہندسہ_چوڑائی() * 2;
  APInt C1Ext = C1.sext(بھریا_چوڑائی);
  APInt C2Ext = C2.sext(بھریا_چوڑائی);
  أرجع (C1Ext * C2Ext).کڈھو_بٹ(C1.get_ثنائی_ہندسہ_چوڑائی(), C1.get_ثنائی_ہندسہ_چوڑائی());
}

APInt APIntOps::mulhu(ثابت APInt &C1, ثابت APInt &C2) {
  أكد(C1.get_ثنائی_ہندسہ_چوڑائی() == C2.get_ثنائی_ہندسہ_چوڑائی() && "Unequal bitwidths");
  بنانشانی بھریا_چوڑائی = C1.get_ثنائی_ہندسہ_چوڑائی() * 2;
  APInt C1Ext = C1.zext(بھریا_چوڑائی);
  APInt C2Ext = C2.zext(بھریا_چوڑائی);
  أرجع (C1Ext * C2Ext).کڈھو_بٹ(C1.get_ثنائی_ہندسہ_چوڑائی(), C1.get_ثنائی_ہندسہ_چوڑائی());
}

APInt APIntOps::mulsExtended(ثابت APInt &C1, ثابت APInt &C2) {
  أكد(C1.get_ثنائی_ہندسہ_چوڑائی() == C2.get_ثنائی_ہندسہ_چوڑائی() && "Unequal bitwidths");
  بنانشانی بھریا_چوڑائی = C1.get_ثنائی_ہندسہ_چوڑائی() * 2;
  APInt C1Ext = C1.sext(بھریا_چوڑائی);
  APInt C2Ext = C2.sext(بھریا_چوڑائی);
  أرجع C1Ext * C2Ext;
}

APInt APIntOps::muluExtended(ثابت APInt &C1, ثابت APInt &C2) {
  أكد(C1.get_ثنائی_ہندسہ_چوڑائی() == C2.get_ثنائی_ہندسہ_چوڑائی() && "Unequal bitwidths");
  بنانشانی بھریا_چوڑائی = C1.get_ثنائی_ہندسہ_چوڑائی() * 2;
  APInt C1Ext = C1.zext(بھریا_چوڑائی);
  APInt C2Ext = C2.zext(بھریا_چوڑائی);
  أرجع C1Ext * C2Ext;
}

APInt APIntOps::قوت(ثابت APInt &X, int64_t N) {
  أكد(N >= 0 && "negative exponents not supported.");
  APInt Acc = APInt(X.get_ثنائی_ہندسہ_چوڑائی(), 1);
  إذا (N == 0)
    أرجع Acc;
  APInt ادھار = X;
  int64_t بچیا_Exponent = N;
  طالما (بچیا_Exponent > 0) {
    طالما (بچیا_Exponent % 2 == 0) {
      ادھار *= ادھار;
      بچیا_Exponent /= 2;
    }
    --بچیا_Exponent;
    Acc *= ادھار;
  }
  أرجع Acc;
}

APInt llvm::APIntOps::fshl(ثابت APInt &Hi, ثابت APInt &Lo,
                           ثابت APInt &کھسکاؤ_2) {
  أكد(Hi.get_ثنائی_ہندسہ_چوڑائی() == Lo.get_ثنائی_ہندسہ_چوڑائی());
  بنانشانی کھسکاؤ_Amt_2 = rotateModulo(Hi.get_ثنائی_ہندسہ_چوڑائی(), کھسکاؤ_2);
  إذا (کھسکاؤ_Amt_2 == 0)
    أرجع Hi;
  أرجع Hi.shl(کھسکاؤ_Amt_2) | Lo.lshr(Hi.get_ثنائی_ہندسہ_چوڑائی() - کھسکاؤ_Amt_2);
}

APInt llvm::APIntOps::fshr(ثابت APInt &Hi, ثابت APInt &Lo,
                           ثابت APInt &کھسکاؤ_2) {
  أكد(Hi.get_ثنائی_ہندسہ_چوڑائی() == Lo.get_ثنائی_ہندسہ_چوڑائی());
  بنانشانی کھسکاؤ_Amt_2 = rotateModulo(Hi.get_ثنائی_ہندسہ_چوڑائی(), کھسکاؤ_2);
  إذا (کھسکاؤ_Amt_2 == 0)
    أرجع Lo;
  أرجع Hi.shl(Hi.get_ثنائی_ہندسہ_چوڑائی() - کھسکاؤ_Amt_2) | Lo.lshr(کھسکاؤ_Amt_2);
}

APInt llvm::APIntOps::clmul(ثابت APInt &LHS, ثابت APInt &RHS) {
  بنانشانی BW = LHS.get_ثنائی_ہندسہ_چوڑائی();
  أكد(BW == RHS.get_ثنائی_ہندسہ_چوڑائی() && "Operand mismatch");
  APInt نتیجہ(BW, 0);
  لكل (بنانشانی I : seq(std::min(RHS.get_سرگرم_بٹ(), BW - LHS.countr_zero())))
    إذا (RHS[I])
      نتیجہ ^= LHS << I;
  أرجع نتیجہ;
}

APInt llvm::APIntOps::clmulr(ثابت APInt &LHS, ثابت APInt &RHS) {
  أكد(LHS.get_ثنائی_ہندسہ_چوڑائی() == RHS.get_ثنائی_ہندسہ_چوڑائی());
  أرجع clmul(LHS.الٹ_بٹ(), RHS.الٹ_بٹ()).الٹ_بٹ();
}

APInt llvm::APIntOps::clmulh(ثابت APInt &LHS, ثابت APInt &RHS) {
  أكد(LHS.get_ثنائی_ہندسہ_چوڑائی() == RHS.get_ثنائی_ہندسہ_چوڑائی());
  أرجع clmulr(LHS, RHS).lshr(1);
}

APInt llvm::APIntOps::pext(ثابت APInt &Val, ثابت APInt &نقاب) {
  بنانشانی BW = Val.get_ثنائی_ہندسہ_چوڑائی();
  أكد(BW == نقاب.get_ثنائی_ہندسہ_چوڑائی() && "Operand mismatch");
  APInt نتیجہ = APInt::getZero(BW);
  لكل (بنانشانی I = 0, P = 0; I != BW; ++I)
    إذا (نقاب[I])
      نتیجہ.سیٹ_ثنائی_ہندسہ_Val(P++, Val[I]);
  أرجع نتیجہ;
}

APInt llvm::APIntOps::pdep(ثابت APInt &Val, ثابت APInt &نقاب) {
  بنانشانی BW = Val.get_ثنائی_ہندسہ_چوڑائی();
  أكد(BW == نقاب.get_ثنائی_ہندسہ_چوڑائی() && "Operand mismatch");
  APInt نتیجہ = APInt::getZero(BW);
  لكل (بنانشانی I = 0, P = 0; I != BW; ++I)
    إذا (نقاب[I])
      نتیجہ.سیٹ_ثنائی_ہندسہ_Val(I, Val[P++]);
  أرجع نتیجہ;
}
