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

کارول نطاق llvm;

#تعریف_کړه DEBUG_TYPE "apint"

/// A utility function for allocating memory, checking for allocation failures,
/// and ensuring the contents are zeroed.
مضمن ساكن uint64_t* get_Cleared_حافظه(بینښه عدد_ټکي) {
  أرجع جديد uint64_t[عدد_ټکي]();
}

/// A utility function for allocating memory and checking for allocation
/// failure.  The content is not zeroed.
مضمن ساكن uint64_t* get_حافظه(بینښه عدد_ټکي) {
  أرجع جديد uint64_t[عدد_ټکي];
}

/// A utility function that converts a character to a digit.
مضمن ساكن بینښه getDigit(حرف cdigit, uint8_t radix) {
  بینښه r;

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
    U.pVal = get_حافظه(get_عدد_ټکي());
    U.pVal[0] = val;
    memset(&U.pVal[1], 0xFF, APINT_کلمه_اندازه * (get_عدد_ټکي() - 1));
    clear_ناکارول_شوی_بټونه();
  } وإلا {
    U.pVal = get_Cleared_حافظه(get_عدد_ټکي());
    U.pVal[0] = val;
  }
}

فراغ APInt::initSlowCase(ثابت APInt& that) {
  U.pVal = get_حافظه(get_عدد_ټکي());
  حافظه_نقل(U.pVal, that.U.pVal, get_عدد_ټکي() * APINT_کلمه_اندازه);
}

فراغ APInt::initFromArray(ArrayRef<uint64_t> bigVal) {
  أكد(bigVal.معلومات() && "Null pointer detected!");
  إذا (LLVM_LIKELY(is_Single_کلمه()))
    U.VAL = bigVal[0];
  وإلا {
    // Get memory, cleared to 0
    U.pVal = get_Cleared_حافظه(get_عدد_ټکي());
    // Calculate the number of words to copy
    بینښه ټکي = std::min<بینښه>(bigVal.اندازه_2(), get_عدد_ټکي());
    // Copy the words from bigVal to pVal
    حافظه_نقل(U.pVal, bigVal.معلومات(), ټکي * APINT_کلمه_اندازه);
  }
  // Make sure unused high bits are cleared
  clear_ناکارول_شوی_بټونه();
}

APInt::APInt(بینښه عدد_بټونه, ArrayRef<uint64_t> bigVal) : بټ_پلنوالی_2(عدد_بټونه) {
  initFromArray(bigVal);
}

APInt::APInt(بینښه numbits, StringRef Str, uint8_t radix)
    : بټ_پلنوالی_2(numbits) {
  fromString(numbits, Str, radix);
}

فراغ APInt::reallocate(بینښه New_بټ_پلنوالی) {
  // If the number of words is the same we can just change the width and stop.
  إذا (get_عدد_ټکي() == get_عدد_ټکي(New_بټ_پلنوالی)) {
    بټ_پلنوالی_2 = New_بټ_پلنوالی;
    أرجع;
  }

  // If we have an allocation, delete it.
  إذا (LLVM_UNLIKELY(!is_Single_کلمه()))
    ړنګول [] U.pVal;

  // Update BitWidth.
  بټ_پلنوالی_2 = New_بټ_پلنوالی;

  // If we are supposed to have an allocation, create it.
  إذا (LLVM_UNLIKELY(!is_Single_کلمه()))
    U.pVal = get_حافظه(get_عدد_ټکي());
}

فراغ APInt::ټاکل_Slow_Case(ثابت APInt &RHS) {
  // Don't do anything for X = X
  إذا (هذا == &RHS)
    أرجع;

  // Adjust the bit width and handle allocations as necessary.
  reallocate(RHS.get_بټ_پلنوالی());

  // Copy the data.
  إذا (LLVM_LIKELY(is_Single_کلمه()))
    U.VAL = RHS.U.VAL;
  وإلا
    حافظه_نقل(U.pVal, RHS.U.pVal, get_عدد_ټکي() * APINT_کلمه_اندازه);
}

/// This method 'profiles' an APInt for use with FoldingSet.
فراغ APInt::Profile(Folding_سېټ_غوټه_ID& ID) ثابت {
  ID.جمع_کړه_Integer(بټ_پلنوالی_2);

  إذا (LLVM_LIKELY(is_Single_کلمه())) {
    ID.جمع_کړه_Integer(U.VAL);
    أرجع;
  }

  بینښه عدد_ټکي_2 = get_عدد_ټکي();
  لكل (بینښه i = 0; i < عدد_ټکي_2; ++i)
    ID.جمع_کړه_Integer(U.pVal[i]);
}

منطقي APInt::isAligned(Align A) ثابت {
  إذا (isZero())
    أرجع ریښتیا;
  ثابت بینښه TrailingZeroes = countr_zero();
  ثابت بینښه MinimumTrailingZeroes = Log2(A);
  أرجع TrailingZeroes >= MinimumTrailingZeroes;
}

/// Prefix increment operator. Increments the APInt by one.
APInt& APInt::چلوونکی++() {
  إذا (LLVM_LIKELY(is_Single_کلمه()))
    ++U.VAL;
  وإلا
    tcIncrement(U.pVal, get_عدد_ټکي());
  أرجع clear_ناکارول_شوی_بټونه();
}

/// Prefix decrement operator. Decrements the APInt by one.
APInt& APInt::چلوونکی--() {
  إذا (LLVM_LIKELY(is_Single_کلمه()))
    --U.VAL;
  وإلا
    tcDecrement(U.pVal, get_عدد_ټکي());
  أرجع clear_ناکارول_شوی_بټونه();
}

/// Adds the RHS APInt to this APInt.
/// @returns this, after addition of RHS.
/// Addition assignment operator.
APInt& APInt::چلوونکی+=(ثابت APInt& RHS) {
  أكد(بټ_پلنوالی_2 == RHS.بټ_پلنوالی_2 && "Bit widths must be the same");
  إذا (LLVM_LIKELY(is_Single_کلمه()))
    U.VAL += RHS.U.VAL;
  وإلا
    tc_جمع_کړه(U.pVal, RHS.U.pVal, 0, get_عدد_ټکي());
  أرجع clear_ناکارول_شوی_بټونه();
}

APInt& APInt::چلوونکی+=(uint64_t RHS) {
  إذا (LLVM_LIKELY(is_Single_کلمه()))
    U.VAL += RHS;
  وإلا
    tc_جمع_کړه_Part(U.pVal, RHS, get_عدد_ټکي());
  أرجع clear_ناکارول_شوی_بټونه();
}

/// Subtracts the RHS APInt from this APInt
/// @returns this, after subtraction
/// Subtraction assignment operator.
APInt& APInt::چلوونکی-=(ثابت APInt& RHS) {
  أكد(بټ_پلنوالی_2 == RHS.بټ_پلنوالی_2 && "Bit widths must be the same");
  إذا (LLVM_LIKELY(is_Single_کلمه()))
    U.VAL -= RHS.U.VAL;
  وإلا
    tc_کمول(U.pVal, RHS.U.pVal, 0, get_عدد_ټکي());
  أرجع clear_ناکارول_شوی_بټونه();
}

APInt& APInt::چلوونکی-=(uint64_t RHS) {
  إذا (LLVM_LIKELY(is_Single_کلمه()))
    U.VAL -= RHS;
  وإلا
    tc_کمول_Part(U.pVal, RHS, get_عدد_ټکي());
  أرجع clear_ناکارول_شوی_بټونه();
}

APInt APInt::چلوونکی*(ثابت APInt& RHS) ثابت {
  أكد(بټ_پلنوالی_2 == RHS.بټ_پلنوالی_2 && "Bit widths must be the same");
  إذا (LLVM_LIKELY(is_Single_کلمه()))
    أرجع APInt(بټ_پلنوالی_2, U.VAL * RHS.U.VAL, /*isSigned=*/خطأ,
                 /*implicitTrunc=*/ریښتیا);

  APInt پایله(get_حافظه(get_عدد_ټکي()), get_بټ_پلنوالی());
  tcMultiply(پایله.U.pVal, U.pVal, RHS.U.pVal, get_عدد_ټکي());
  پایله.clear_ناکارول_شوی_بټونه();
  أرجع پایله;
}

فراغ APInt::and_ټاکل_Slow_Case(ثابت APInt &RHS) {
  کلمه_Type *dst = U.pVal, *rhs = RHS.U.pVal;
  لكل (size_t i = 0, e = get_عدد_ټکي(); i != e; ++i)
    dst[i] &= rhs[i];
}

فراغ APInt::or_ټاکل_Slow_Case(ثابت APInt &RHS) {
  کلمه_Type *dst = U.pVal, *rhs = RHS.U.pVal;
  لكل (size_t i = 0, e = get_عدد_ټکي(); i != e; ++i)
    dst[i] |= rhs[i];
}

فراغ APInt::xor_ټاکل_Slow_Case(ثابت APInt &RHS) {
  کلمه_Type *dst = U.pVal, *rhs = RHS.U.pVal;
  لكل (size_t i = 0, e = get_عدد_ټکي(); i != e; ++i)
    dst[i] ^= rhs[i];
}

APInt &APInt::چلوونکی*=(ثابت APInt &RHS) {
  *هذا = *هذا * RHS;
  أرجع *هذا;
}

APInt& APInt::چلوونکی*=(uint64_t RHS) {
  إذا (LLVM_LIKELY(is_Single_کلمه())) {
    U.VAL *= RHS;
  } وإلا {
    بینښه عدد_ټکي_2 = get_عدد_ټکي();
    tcMultiplyPart(U.pVal, U.pVal, RHS, 0, عدد_ټکي_2, عدد_ټکي_2, خطأ);
  }
  أرجع clear_ناکارول_شوی_بټونه();
}

منطقي APInt::equalSlowCase(ثابت APInt &RHS) ثابت {
  أرجع std::equal(U.pVal, U.pVal + get_عدد_ټکي(), RHS.U.pVal);
}

صحيح APInt::compare(ثابت APInt& RHS) ثابت {
  أكد(بټ_پلنوالی_2 == RHS.بټ_پلنوالی_2 && "Bit widths must be same for comparison");
  إذا (LLVM_LIKELY(is_Single_کلمه()))
    أرجع U.VAL < RHS.U.VAL ? -1 : U.VAL > RHS.U.VAL;

  أرجع tcCompare(U.pVal, RHS.U.pVal, get_عدد_ټکي());
}

صحيح APInt::compareSigned(ثابت APInt& RHS) ثابت {
  أكد(بټ_پلنوالی_2 == RHS.بټ_پلنوالی_2 && "Bit widths must be same for comparison");
  إذا (LLVM_LIKELY(is_Single_کلمه())) {
    int64_t lhsSext = SignExtend64(U.VAL, بټ_پلنوالی_2);
    int64_t rhsSext = SignExtend64(RHS.U.VAL, بټ_پلنوالی_2);
    أرجع lhsSext < rhsSext ? -1 : lhsSext > rhsSext;
  }

  منطقي lhsNeg = isNegative();
  منطقي rhsNeg = RHS.isNegative();

  // If the sign bits don't match, then (LHS < RHS) if LHS is negative
  إذا (lhsNeg != rhsNeg)
    أرجع lhsNeg ? -1 : 1;

  // Otherwise we can just use an unsigned comparison, because even negative
  // numbers compare correctly this way if both have the same signed-ness.
  أرجع tcCompare(U.pVal, RHS.U.pVal, get_عدد_ټکي());
}

فراغ APInt::سېټ_بټونه_Slow_Case(بینښه lo_بټ, بینښه hi_بټ) {
  بینښه lo_کلمه = which_کلمه(lo_بټ);
  بینښه hi_کلمه = which_کلمه(hi_بټ);

  // Create an initial mask for the low word with zeros below loBit.
  uint64_t lo_ماسک = WORDTYPE_MAX << which_بټ(lo_بټ);

  // If hiBit is not aligned, we need a high mask.
  بینښه hi_ښويدنه_Amt = which_بټ(hi_بټ);
  إذا (hi_ښويدنه_Amt != 0) {
    // Create a high mask with zeros above hiBit.
    uint64_t hi_ماسک = WORDTYPE_MAX >> (APINT_بټونه_فی_کلمه - hi_ښويدنه_Amt);
    // If loWord and hiWord are equal, then we combine the masks. Otherwise,
    // set the bits in hiWord.
    إذا (hi_کلمه == lo_کلمه)
      lo_ماسک &= hi_ماسک;
    وإلا
      U.pVal[hi_کلمه] |= hi_ماسک;
  }
  // Apply the mask to the low word.
  U.pVal[lo_کلمه] |= lo_ماسک;

  // Fill any words between loWord and hiWord with all ones.
  لكل (بینښه کلمه = lo_کلمه + 1; کلمه < hi_کلمه; ++کلمه)
    U.pVal[کلمه] = WORDTYPE_MAX;
}

فراغ APInt::clear_بټونه_Slow_Case(بینښه Lo_بټ, بینښه Hi_بټ) {
  بینښه Lo_کلمه = which_کلمه(Lo_بټ);
  بینښه Hi_کلمه = which_کلمه(Hi_بټ);

  // Create an initial mask for the low word with ones below loBit.
  uint64_t Lo_ماسک = ~(WORDTYPE_MAX << which_بټ(Lo_بټ));

  // If HiBit is not aligned, we need a high mask.
  بینښه Hi_ښويدنه_Amt = which_بټ(Hi_بټ);
  إذا (Hi_ښويدنه_Amt != 0) {
    // Create a high mask with ones above HiBit.
    uint64_t Hi_ماسک = ~(WORDTYPE_MAX >> (APINT_بټونه_فی_کلمه - Hi_ښويدنه_Amt));
    // If LoWord and HiWord are equal, then we combine the masks. Otherwise,
    // clear the bits in HiWord.
    إذا (Hi_کلمه == Lo_کلمه)
      Lo_ماسک |= Hi_ماسک;
    وإلا
      U.pVal[Hi_کلمه] &= Hi_ماسک;
  }
  // Apply the mask to the low word.
  U.pVal[Lo_کلمه] &= Lo_ماسک;

  // Fill any words between LoWord and HiWord with all zeros.
  لكل (بینښه کلمه_2 = Lo_کلمه + 1; کلمه_2 < Hi_کلمه; ++کلمه_2)
    U.pVal[کلمه_2] = 0;
}

// Complement a bignum in-place.
ساكن فراغ tcComplement(APInt::کلمه_Type *dst, بینښه parts) {
  لكل (بینښه i = 0; i < parts; i++)
    dst[i] = ~dst[i];
}

/// Toggle every bit to its opposite value.
فراغ APInt::flip_All_بټونه_Slow_Case() {
  tcComplement(U.pVal, get_عدد_ټکي());
  clear_ناکارول_شوی_بټونه();
}

/// Concatenate the bits from "NewLSB" onto the bottom of *this.  This is
/// equivalent to:
///   (this->zext(NewWidth) << NewLSB.getBitWidth()) | NewLSB.zext(NewWidth)
/// In the slow case, we know the result is large.
APInt APInt::concatSlowCase(ثابت APInt &NewLSB) ثابت {
  بینښه New_پلنوالی = get_بټ_پلنوالی() + NewLSB.get_بټ_پلنوالی();
  APInt پایله = NewLSB.zext(New_پلنوالی);
  پایله.ورننباسه_بټونه(*هذا, NewLSB.get_بټ_پلنوالی());
  أرجع پایله;
}

/// Toggle a given bit to its opposite value whose position is given
/// as "bitPosition".
/// Toggles a given bit to its opposite value.
فراغ APInt::flip_بټ(بینښه بټ_Position) {
  أكد(بټ_Position < بټ_پلنوالی_2 && "Out of the bit-width range!");
  سېټ_بټ_Val(بټ_Position, !(*هذا)[بټ_Position]);
}

فراغ APInt::ورننباسه_بټونه(ثابت APInt &sub_بټونه, بینښه بټ_Position) {
  بینښه sub_بټ_پلنوالی = sub_بټونه.get_بټ_پلنوالی();
  أكد((sub_بټ_پلنوالی + بټ_Position) <= بټ_پلنوالی_2 && "Illegal bit insertion");

  // inserting no bits is a noop.
  إذا (sub_بټ_پلنوالی == 0)
    أرجع;

  // Insertion is a direct copy.
  إذا (sub_بټ_پلنوالی == بټ_پلنوالی_2) {
    *هذا = sub_بټونه;
    أرجع;
  }

  // Single word result can be done as a direct bitmask.
  إذا (LLVM_LIKELY(is_Single_کلمه())) {
    uint64_t ماسک_2 = WORDTYPE_MAX >> (APINT_بټونه_فی_کلمه - sub_بټ_پلنوالی);
    U.VAL &= ~(ماسک_2 << بټ_Position);
    U.VAL |= (sub_بټونه.U.VAL << بټ_Position);
    أرجع;
  }

  بینښه lo_بټ = which_بټ(بټ_Position);
  بینښه lo_کلمه = which_کلمه(بټ_Position);
  بینښه hi1_کلمه = which_کلمه(بټ_Position + sub_بټ_پلنوالی - 1);

  // Insertion within a single word can be done as a direct bitmask.
  إذا (lo_کلمه == hi1_کلمه) {
    uint64_t ماسک_2 = WORDTYPE_MAX >> (APINT_بټونه_فی_کلمه - sub_بټ_پلنوالی);
    U.pVal[lo_کلمه] &= ~(ماسک_2 << lo_بټ);
    U.pVal[lo_کلمه] |= (sub_بټونه.U.VAL << lo_بټ);
    أرجع;
  }

  // Insert on word boundaries.
  إذا (lo_بټ == 0) {
    // Direct copy whole words.
    بینښه عدد_ټولیز_Sub_ټکي = sub_بټ_پلنوالی / APINT_بټونه_فی_کلمه;
    حافظه_نقل(U.pVal + lo_کلمه, sub_بټونه.get_Raw_معلومات(),
           عدد_ټولیز_Sub_ټکي * APINT_کلمه_اندازه);

    // Mask+insert remaining bits.
    بینښه پاتې_شوی_بټونه = sub_بټ_پلنوالی % APINT_بټونه_فی_کلمه;
    إذا (پاتې_شوی_بټونه != 0) {
      uint64_t ماسک_2 = WORDTYPE_MAX >> (APINT_بټونه_فی_کلمه - پاتې_شوی_بټونه);
      U.pVal[hi1_کلمه] &= ~ماسک_2;
      U.pVal[hi1_کلمه] |= sub_بټونه.get_کلمه(sub_بټ_پلنوالی - 1);
    }
    أرجع;
  }

  // General case - set/clear individual bits in dst based on src.
  // TODO - there is scope for optimization here, but at the moment this code
  // path is barely used so prefer readability over performance.
  لكل (بینښه i = 0; i != sub_بټ_پلنوالی; ++i)
    سېټ_بټ_Val(بټ_Position + i, sub_بټونه[i]);
}

فراغ APInt::ورننباسه_بټونه(uint64_t sub_بټونه, بینښه بټ_Position, بینښه عدد_بټونه) {
  uint64_t ماسک_بټونه = ماسک_Trailing_Ones<uint64_t>(عدد_بټونه);
  sub_بټونه &= ماسک_بټونه;
  إذا (LLVM_LIKELY(is_Single_کلمه())) {
    U.VAL &= ~(ماسک_بټونه << بټ_Position);
    U.VAL |= sub_بټونه << بټ_Position;
    أرجع;
  }

  بینښه lo_بټ = which_بټ(بټ_Position);
  بینښه lo_کلمه = which_کلمه(بټ_Position);
  بینښه hi_کلمه = which_کلمه(بټ_Position + عدد_بټونه - 1);
  إذا (lo_کلمه == hi_کلمه) {
    U.pVal[lo_کلمه] &= ~(ماسک_بټونه << lo_بټ);
    U.pVal[lo_کلمه] |= sub_بټونه << lo_بټ;
    أرجع;
  }

  static_assert(8 * حجم(کلمه_Type) <= 64, "This code assumes only two words affected");
  بینښه کلمه_بټونه = 8 * حجم(کلمه_Type);
  U.pVal[lo_کلمه] &= ~(ماسک_بټونه << lo_بټ);
  U.pVal[lo_کلمه] |= sub_بټونه << lo_بټ;

  U.pVal[hi_کلمه] &= ~(ماسک_بټونه >> (کلمه_بټونه - lo_بټ));
  U.pVal[hi_کلمه] |= sub_بټونه >> (کلمه_بټونه - lo_بټ);
}

APInt APInt::ایستل_بټونه(بینښه عدد_بټونه, بینښه بټ_Position) ثابت {
  أكد(بټ_Position < بټ_پلنوالی_2 && (عدد_بټونه + بټ_Position) <= بټ_پلنوالی_2 &&
         "Illegal bit extraction");

  إذا (LLVM_LIKELY(is_Single_کلمه()))
    أرجع APInt(عدد_بټونه, U.VAL >> بټ_Position, /*isSigned=*/خطأ,
                 /*implicitTrunc=*/ریښتیا);

  بینښه lo_بټ = which_بټ(بټ_Position);
  بینښه lo_کلمه = which_کلمه(بټ_Position);
  بینښه hi_کلمه = which_کلمه(بټ_Position + عدد_بټونه - 1);

  // Single word result extracting bits from a single word source.
  إذا (lo_کلمه == hi_کلمه)
    أرجع APInt(عدد_بټونه, U.pVal[lo_کلمه] >> lo_بټ, /*isSigned=*/خطأ,
                 /*implicitTrunc=*/ریښتیا);

  // Extracting bits that start on a source word boundary can be done
  // as a fast memory copy.
  إذا (lo_بټ == 0)
    أرجع APInt(عدد_بټونه, ArrayRef(U.pVal + lo_کلمه, 1 + hi_کلمه - lo_کلمه));

  // General case - shift + copy source words directly into place.
  APInt پایله(عدد_بټونه, 0);
  بینښه عدد_Src_ټکي = get_عدد_ټکي();
  بینښه عدد_Dst_ټکي = پایله.get_عدد_ټکي();

  uint64_t *DestPtr =
      LLVM_LIKELY(پایله.is_Single_کلمه()) ? &پایله.U.VAL : پایله.U.pVal;
  لكل (بینښه کلمه = 0; کلمه < عدد_Dst_ټکي; ++کلمه) {
    uint64_t w0 = U.pVal[lo_کلمه + کلمه];
    uint64_t w1 =
        (lo_کلمه + کلمه + 1) < عدد_Src_ټکي ? U.pVal[lo_کلمه + کلمه + 1] : 0;
    DestPtr[کلمه] = (w0 >> lo_بټ) | (w1 << (APINT_بټونه_فی_کلمه - lo_بټ));
  }

  أرجع پایله.clear_ناکارول_شوی_بټونه();
}

uint64_t APInt::ایستل_بټونه_As_ZExt_ارزښت(بینښه عدد_بټونه,
                                       بینښه بټ_Position) ثابت {
  أكد(بټ_Position < بټ_پلنوالی_2 && (عدد_بټونه + بټ_Position) <= بټ_پلنوالی_2 &&
         "Illegal bit extraction");
  أكد(عدد_بټونه <= 64 && "Illegal bit extraction");

  uint64_t ماسک_بټونه = ماسک_Trailing_Ones<uint64_t>(عدد_بټونه);
  إذا (LLVM_LIKELY(is_Single_کلمه()))
    أرجع (U.VAL >> بټ_Position) & ماسک_بټونه;

  static_assert(APINT_بټونه_فی_کلمه >= 64,
                "This code assumes only two words affected");
  بینښه lo_بټ = which_بټ(بټ_Position);
  بینښه lo_کلمه = which_کلمه(بټ_Position);
  بینښه hi_کلمه = which_کلمه(بټ_Position + عدد_بټونه - 1);
  إذا (lo_کلمه == hi_کلمه)
    أرجع (U.pVal[lo_کلمه] >> lo_بټ) & ماسک_بټونه;

  uint64_t ret_بټونه = U.pVal[lo_کلمه] >> lo_بټ;
  ret_بټونه |= U.pVal[hi_کلمه] << (APINT_بټونه_فی_کلمه - lo_بټ);
  ret_بټونه &= ماسک_بټونه;
  أرجع ret_بټونه;
}

بینښه APInt::get_Sufficient_بټونه_Needed(StringRef Str, uint8_t Radix) {
  أكد(!Str.تشه() && "Invalid string length");
  size_t StrLen = Str.اندازه_2();

  // Each computation below needs to know if it's negative.
  بینښه IsNegative = خطأ;
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

بینښه APInt::get_بټونه_Needed(StringRef str, uint8_t radix) {
  // Compute a sufficient number of bits that is always large enough but might
  // be too large.
  بینښه sufficient = get_Sufficient_بټونه_Needed(str, radix);

  // For bases 2, 8, and 16, the sufficient number of bits is exact and we can
  // return the value directly. For bases 10 and 36, we need to do extra work.
  إذا (radix == 2 || radix == 8 || radix == 16)
    أرجع sufficient;

  // This is grossly inefficient but accurate. We could probably do something
  // with a computation of roughly slen*64/20 and then adjust by the value of
  // the first few digits. But, I'm not sure how accurate that could be.
  size_t slen = str.اندازه_2();

  // Each computation below needs to know if it's negative.
  StringRef::iterator p = str.begin();
  بینښه isNegative = *p == '-';
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
  بینښه لوګاریتم = tmp.logBase2();
  إذا (لوګاریتم == (بینښه)-1) {
    أرجع isNegative + 1;
  } وإلا إذا (isNegative && tmp.isPowerOf2()) {
    أرجع isNegative + لوګاریتم;
  } وإلا {
    أرجع isNegative + لوګاریتم + 1;
  }
}

hash_کوډ llvm::hash_ارزښت(ثابت APInt &Arg) {
  إذا (LLVM_LIKELY(Arg.is_Single_کلمه()))
    أرجع hash_combine(Arg.بټ_پلنوالی_2, Arg.U.VAL);

  أرجع hash_combine(
      Arg.بټ_پلنوالی_2,
      hash_combine_range(Arg.U.pVal, Arg.U.pVal + Arg.get_عدد_ټکي()));
}

بینښه DenseMapInfo<APInt, فراغ>::get_Hash_ارزښت(ثابت APInt &کلی) {
  أرجع static_cast<بینښه>(hash_ارزښت(کلی));
}

منطقي APInt::isSplat(بینښه Splat_اندازه_دننه_بټونه) ثابت {
  أكد(get_بټ_پلنوالی() % Splat_اندازه_دننه_بټونه == 0 &&
         "SplatSizeInBits must divide width!");
  // We can check that all parts of an integer are equal by making use of a
  // little trick: rotate and check if it's still the same value.
  أرجع *هذا == rotl(Splat_اندازه_دننه_بټونه);
}

/// This function returns the high "numBits" bits of this APInt.
APInt APInt::get_Hi_بټونه(بینښه عدد_بټونه) ثابت {
  أرجع هذا->lshr(بټ_پلنوالی_2 - عدد_بټونه);
}

/// This function returns the low "numBits" bits of this APInt.
APInt APInt::get_Lo_بټونه(بینښه عدد_بټونه) ثابت {
  APInt پایله(get_ټیټ_بټونه_سېټ(بټ_پلنوالی_2, عدد_بټونه));
  پایله &= *هذا;
  أرجع پایله;
}

/// Return a value containing V broadcasted over NewLen bits.
APInt APInt::getSplat(بینښه NewLen, ثابت APInt &V) {
  أكد(NewLen >= V.get_بټ_پلنوالی() && "Can't splat to smaller bit width!");

  APInt Val = V.zext(NewLen);
  لكل (بینښه I = V.get_بټ_پلنوالی(); I < NewLen; I <<= 1)
    Val |= Val << I;

  أرجع Val;
}

بینښه APInt::شمېر_Leading_Zeros_Slow_Case() ثابت {
  بینښه شمېر_2 = 0;
  لكل (صحيح i = get_عدد_ټکي() - 1; i >= 0; --i) {
    uint64_t V = U.pVal[i];
    إذا (V == 0)
      شمېر_2 += APINT_بټونه_فی_کلمه;
    وإلا {
      شمېر_2 += llvm::countl_zero(V);
      اكسر;
    }
  }
  // Adjust for unused bits in the most significant word (they are zero).
  بینښه Mod = بټ_پلنوالی_2 % APINT_بټونه_فی_کلمه;
  شمېر_2 -= Mod > 0 ? APINT_بټونه_فی_کلمه - Mod : 0;
  أرجع شمېر_2;
}

بینښه APInt::شمېر_Leading_Ones_Slow_Case() ثابت {
  بینښه لوړ_کلمه_بټونه = بټ_پلنوالی_2 % APINT_بټونه_فی_کلمه;
  بینښه ښويدنه;
  إذا (!لوړ_کلمه_بټونه) {
    لوړ_کلمه_بټونه = APINT_بټونه_فی_کلمه;
    ښويدنه = 0;
  } وإلا {
    ښويدنه = APINT_بټونه_فی_کلمه - لوړ_کلمه_بټونه;
  }
  صحيح i = get_عدد_ټکي() - 1;
  بینښه شمېر_2 = llvm::countl_one(U.pVal[i] << ښويدنه);
  إذا (شمېر_2 == لوړ_کلمه_بټونه) {
    لكل (i--; i >= 0; --i) {
      إذا (U.pVal[i] == WORDTYPE_MAX)
        شمېر_2 += APINT_بټونه_فی_کلمه;
      وإلا {
        شمېر_2 += llvm::countl_one(U.pVal[i]);
        اكسر;
      }
    }
  }
  أرجع شمېر_2;
}

بینښه APInt::شمېر_Trailing_Zeros_Slow_Case() ثابت {
  بینښه شمېر_2 = 0;
  بینښه i = 0;
  لكل (; i < get_عدد_ټکي() && U.pVal[i] == 0; ++i)
    شمېر_2 += APINT_بټونه_فی_کلمه;
  إذا (i < get_عدد_ټکي())
    شمېر_2 += llvm::countr_zero(U.pVal[i]);
  أرجع std::min(شمېر_2, بټ_پلنوالی_2);
}

بینښه APInt::شمېر_Trailing_Ones_Slow_Case() ثابت {
  بینښه شمېر_2 = 0;
  بینښه i = 0;
  لكل (; i < get_عدد_ټکي() && U.pVal[i] == WORDTYPE_MAX; ++i)
    شمېر_2 += APINT_بټونه_فی_کلمه;
  إذا (i < get_عدد_ټکي())
    شمېر_2 += llvm::countr_one(U.pVal[i]);
  أكد(شمېر_2 <= بټ_پلنوالی_2);
  أرجع شمېر_2;
}

بینښه APInt::شمېر_Population_Slow_Case() ثابت {
  بینښه شمېر_2 = 0;
  لكل (بینښه i = 0; i < get_عدد_ټکي(); ++i)
    شمېر_2 += llvm::popcount(U.pVal[i]);
  أرجع شمېر_2;
}

منطقي APInt::isPowerOf2SlowCase() ثابت {
  بینښه شمېر_2 = 0;
  لكل (بینښه i = 0; i < get_عدد_ټکي(); ++i) {
    شمېر_2 += llvm::popcount(U.pVal[i]);
    إذا (شمېر_2 > 1)
      أرجع خطأ;
  }
  أرجع شمېر_2 == 1;
}

منطقي APInt::intersectsSlowCase(ثابت APInt &RHS) ثابت {
  لكل (بینښه i = 0, e = get_عدد_ټکي(); i != e; ++i)
    إذا ((U.pVal[i] & RHS.U.pVal[i]) != 0)
      أرجع ریښتیا;

  أرجع خطأ;
}

منطقي APInt::is_Subset_د_Slow_Case(ثابت APInt &RHS) ثابت {
  لكل (بینښه i = 0, e = get_عدد_ټکي(); i != e; ++i)
    إذا ((U.pVal[i] & ~RHS.U.pVal[i]) != 0)
      أرجع خطأ;

  أرجع ریښتیا;
}

منطقي APInt::is_Inverse_د_Slow_Case(ثابت APInt &RHS) ثابت {
  ثابت بینښه وروستی = get_عدد_ټکي() - 1;
  لكل (بینښه I = 0; I != وروستی; ++I)
    إذا ((U.pVal[I] ^ RHS.U.pVal[I]) != WORDTYPE_MAX)
      أرجع خطأ;

  بینښه لکۍ_بټونه = بټ_پلنوالی_2 - وروستی * APINT_بټونه_فی_کلمه;
  کلمه_Type لکۍ_ماسک = llvm::ماسک_Trailing_Ones<کلمه_Type>(لکۍ_بټونه);
  أرجع (U.pVal[وروستی] ^ RHS.U.pVal[وروستی]) == لکۍ_ماسک;
}

APInt APInt::byte_بدلون() ثابت {
  أكد(بټ_پلنوالی_2 >= 16 && بټ_پلنوالی_2 % 8 == 0 && "Cannot byteswap!");
  إذا (بټ_پلنوالی_2 == 16)
    أرجع APInt(بټ_پلنوالی_2, llvm::byteswap<uint16_t>(U.VAL));
  إذا (بټ_پلنوالی_2 == 32)
    أرجع APInt(بټ_پلنوالی_2, llvm::byteswap<uint32_t>(U.VAL));
  إذا (بټ_پلنوالی_2 <= 64) {
    uint64_t Tmp1 = llvm::byteswap<uint64_t>(U.VAL);
    Tmp1 >>= (64 - بټ_پلنوالی_2);
    أرجع APInt(بټ_پلنوالی_2, Tmp1);
  }

  APInt پایله(get_عدد_ټکي() * APINT_بټونه_فی_کلمه, 0);
  لكل (بینښه I = 0, N = get_عدد_ټکي(); I != N; ++I)
    پایله.U.pVal[I] = llvm::byteswap<uint64_t>(U.pVal[N - I - 1]);
  إذا (پایله.بټ_پلنوالی_2 != بټ_پلنوالی_2) {
    پایله.lshr_دننه_ځای_پرځای_کړه(پایله.بټ_پلنوالی_2 - بټ_پلنوالی_2);
    پایله.بټ_پلنوالی_2 = بټ_پلنوالی_2;
  }
  أرجع پایله;
}

APInt APInt::سرچپه_بټونه() ثابت {
  إذا (LLVM_LIKELY(is_Single_کلمه())) {
    بدل (بټ_پلنوالی_2) {
    حالة 64:
      أرجع APInt(بټ_پلنوالی_2, llvm::سرچپه_بټونه<uint64_t>(U.VAL));
    حالة 32:
      أرجع APInt(بټ_پلنوالی_2, llvm::سرچپه_بټونه<uint32_t>(U.VAL));
    حالة 16:
      أرجع APInt(بټ_پلنوالی_2, llvm::سرچپه_بټونه<uint16_t>(U.VAL));
    حالة 8:
      أرجع APInt(بټ_پلنوالی_2, llvm::سرچپه_بټونه<uint8_t>(U.VAL));
    حالة 1: // fallthrough
    حالة 0:
      أرجع *هذا;
    افتراضي:
      أرجع APInt(بټ_پلنوالی_2,
                   llvm::سرچپه_بټونه<uint64_t>(U.VAL) >> (64 - بټ_پلنوالی_2));
    }
  }

  APInt پایله(بټ_پلنوالی_2, 0);
  بینښه عدد_ټکي_2 = get_عدد_ټکي();
  بینښه Excess_بټونه = عدد_ټکي_2 * APINT_بټونه_فی_کلمه - بټ_پلنوالی_2;
  إذا (Excess_بټونه == 0) {
    // Fast path. No cross-word shift needed.
    لكل (بینښه I = 0; I < عدد_ټکي_2; ++I)
      پایله.U.pVal[I] = llvm::سرچپه_بټونه<uint64_t>(U.pVal[عدد_ټکي_2 - 1 - I]);
    أرجع پایله;
  }
  // Holds reversed bits of the previous (more significant) word.
  uint64_t PrevRev = llvm::سرچپه_بټونه<uint64_t>(U.pVal[عدد_ټکي_2 - 1]);
  لكل (بینښه I = 0; I < عدد_ټکي_2 - 1; ++I) {
    uint64_t CurrRev = llvm::سرچپه_بټونه<uint64_t>(U.pVal[عدد_ټکي_2 - 2 - I]);
    پایله.U.pVal[I] = (PrevRev >> Excess_بټونه) | (CurrRev << (64 - Excess_بټونه));
    PrevRev = CurrRev;
  }
  پایله.U.pVal[عدد_ټکي_2 - 1] = PrevRev >> Excess_بټونه;
  أرجع پایله;
}

APInt llvm::APIntOps::Greatest_Common_وېشونکی(APInt A, APInt B, منطقي IsSigned) {
  // Take absolute value if IsSigned.
  إذا (IsSigned) {
    A = A.مطلق_قیمت();
    B = B.مطلق_قیمت();
  }

  // Fast-path a common case.
  إذا (A == B) أرجع A;

  // Corner cases: if either operand is zero, the other is the gcd.
  إذا (!A) أرجع B;
  إذا (!B) أرجع A;

  // Count common powers of 2 and remove all other powers of 2.
  بینښه Pow2;
  {
    بینښه Pow2_A = A.countr_zero();
    بینښه Pow2_B = B.countr_zero();
    إذا (Pow2_A > Pow2_B) {
      A.lshr_دننه_ځای_پرځای_کړه(Pow2_A - Pow2_B);
      Pow2 = Pow2_B;
    } وإلا إذا (Pow2_B > Pow2_A) {
      B.lshr_دننه_ځای_پرځای_کړه(Pow2_B - Pow2_A);
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
      A.lshr_دننه_ځای_پرځای_کړه(A.countr_zero() - Pow2);
    } وإلا {
      B -= A;
      B.lshr_دننه_ځای_پرځای_کړه(B.countr_zero() - Pow2);
    }
  }

  أرجع A;
}

APInt llvm::APIntOps::پړاو_Double_منزل_APInt(مزدوج Double, بینښه پلنوالی_2) {
  uint64_t I = بټ_cast<uint64_t>(Double);

  // Get the sign bit from the highest order bit
  منطقي isNeg = I >> 63;

  // Get the 11-bit exponent and adjust for the 1023 bit bias
  int64_t توانیز_تابع = ((I >> 52) & 0x7ff) - 1023;

  // If the exponent is negative, the value is < 0 so just return 0.
  إذا (توانیز_تابع < 0)
    أرجع APInt(پلنوالی_2, 0u);

  // Extract the mantissa by clearing the top 12 bits (sign + exponent).
  uint64_t mantissa = (I & (~0ULL >> 12)) | 1ULL << 52;

  // If the exponent doesn't shift all bits out of the mantissa
  إذا (توانیز_تابع < 52)
    أرجع isNeg ? -APInt(پلنوالی_2, mantissa >> (52 - توانیز_تابع)) :
                    APInt(پلنوالی_2, mantissa >> (52 - توانیز_تابع));

  // If the client didn't provide enough bits for us to shift the mantissa into
  // then the result is undefined, just return 0
  إذا (پلنوالی_2 <= توانیز_تابع - 52)
    أرجع APInt(پلنوالی_2, 0);

  // Otherwise, we have to shift the mantissa bits up to the right location
  APInt Tmp(پلنوالی_2, mantissa);
  Tmp <<= (بینښه)توانیز_تابع - 52;
  أرجع isNeg ? -Tmp : Tmp;
}

/// This function converts this APInt to a double.
/// The layout for double is as following (IEEE Standard 754):
///  --------------------------------------
/// |  Sign    Exponent    Fraction    Bias |
/// |-------------------------------------- |
/// |  1[63]   11[62-52]   52[51-00]   1023 |
///  --------------------------------------
مزدوج APInt::پړاو_منزل_Double(منطقي isSigned) ثابت {
  // Handle the simple case where the value is contained in one uint64_t.
  // It is wrong to optimize getWord(0) to VAL; there might be more than one word.
  إذا (LLVM_LIKELY(is_Single_کلمه() || get_فعال_بټونه() <= APINT_بټونه_فی_کلمه)) {
    إذا (isSigned) {
      int64_t sext = SignExtend64(get_کلمه(0), بټ_پلنوالی_2);
      أرجع مزدوج(sext);
    }
    أرجع مزدوج(get_کلمه(0));
  }

  // Determine if the value is negative.
  منطقي isNeg = isSigned ? (*هذا)[بټ_پلنوالی_2-1] : خطأ;

  // Construct the absolute value if we're negative.
  APInt Tmp(isNeg ? -(*هذا) : (*هذا));

  // Figure out how many bits we're using.
  بینښه n = Tmp.get_فعال_بټونه();

  // The exponent (without bias normalization) is just the number of bits
  // we are using. Note that the sign bit is gone since we constructed the
  // absolute value.
  uint64_t توانیز_تابع = n;

  // Return infinity for exponent overflow
  إذا (توانیز_تابع > 1023) {
    إذا (!isSigned || !isNeg)
      أرجع std::numeric_limits<مزدوج>::infinity();
    وإلا
      أرجع -std::numeric_limits<مزدوج>::infinity();
  }
  توانیز_تابع += 1023; // Increment for 1023 bias

  // Number of bits in mantissa is 52. To obtain the mantissa value, we must
  // extract the high 52 bits from the correct words in pVal.
  uint64_t mantissa;
  بینښه hi_کلمه = which_کلمه(n-1);
  إذا (hi_کلمه == 0) {
    mantissa = Tmp.U.pVal[0];
    إذا (n > 52)
      mantissa >>= n - 52; // shift down, we want the top 52 bits.
  } وإلا {
    أكد(hi_کلمه > 0 && "huh?");
    uint64_t hibits = Tmp.U.pVal[hi_کلمه] << (52 - n % APINT_بټونه_فی_کلمه);
    uint64_t lobits = Tmp.U.pVal[hi_کلمه-1] >> (11 + n % APINT_بټونه_فی_کلمه);
    mantissa = hibits | lobits;
  }

  // The leading bit of mantissa is implicit, so get rid of it.
  uint64_t sign = isNeg ? (1ULL << (APINT_بټونه_فی_کلمه - 1)) : 0;
  uint64_t I = sign | (توانیز_تابع << 52) | mantissa;
  أرجع بټ_cast<مزدوج>(I);
}

// Truncate to new width.
APInt APInt::trunc(بینښه پلنوالی_2) ثابت {
  أكد(پلنوالی_2 <= بټ_پلنوالی_2 && "Invalid APInt Truncate request");

  إذا (پلنوالی_2 <= APINT_بټونه_فی_کلمه)
    أرجع APInt(پلنوالی_2, get_Raw_معلومات()[0], /*isSigned=*/خطأ,
                 /*implicitTrunc=*/ریښتیا);

  إذا (پلنوالی_2 == بټ_پلنوالی_2)
    أرجع *هذا;

  APInt پایله(get_حافظه(get_عدد_ټکي(پلنوالی_2)), پلنوالی_2);

  // Copy full words.
  بینښه i;
  لكل (i = 0; i != پلنوالی_2 / APINT_بټونه_فی_کلمه; i++)
    پایله.U.pVal[i] = U.pVal[i];

  // Truncate and copy any partial word.
  بینښه بټونه = (0 - پلنوالی_2) % APINT_بټونه_فی_کلمه;
  إذا (بټونه != 0)
    پایله.U.pVal[i] = U.pVal[i] << بټونه >> بټونه;

  أرجع پایله;
}

// Truncate to new width with unsigned saturation.
APInt APInt::truncUSat(بینښه پلنوالی_2) ثابت {
  أكد(پلنوالی_2 <= بټ_پلنوالی_2 && "Invalid APInt Truncate request");

  // Can we just losslessly truncate it?
  إذا (isIntN(پلنوالی_2))
    أرجع trunc(پلنوالی_2);
  // If not, then just return the new limit.
  أرجع APInt::get_Max_ارزښت(پلنوالی_2);
}

// Truncate to new width with signed saturation to signed result.
APInt APInt::truncSSat(بینښه پلنوالی_2) ثابت {
  أكد(پلنوالی_2 <= بټ_پلنوالی_2 && "Invalid APInt Truncate request");

  // Can we just losslessly truncate it?
  إذا (isSignedIntN(پلنوالی_2))
    أرجع trunc(پلنوالی_2);
  // If not, then just return the new limits.
  أرجع isNegative() ? APInt::get_Signed_Min_ارزښت(پلنوالی_2)
                      : APInt::get_Signed_Max_ارزښت(پلنوالی_2);
}

// Truncate to new width with signed saturation to unsigned result.
APInt APInt::truncSSatU(بینښه پلنوالی_2) ثابت {
  أكد(پلنوالی_2 <= بټ_پلنوالی_2 && "Invalid APInt Truncate request");

  // Can we just losslessly truncate it?
  إذا (isIntN(پلنوالی_2))
    أرجع trunc(پلنوالی_2);
  // If not, then just return the new limits.
  أرجع isNegative() ? APInt::getZero(پلنوالی_2) : APInt::get_Max_ارزښت(پلنوالی_2);
}

// Sign extend to a new width.
APInt APInt::sext(بینښه پلنوالی) ثابت {
  أكد(پلنوالی >= بټ_پلنوالی_2 && "Invalid APInt SignExtend request");

  إذا (پلنوالی <= APINT_بټونه_فی_کلمه)
    أرجع APInt(پلنوالی, SignExtend64(U.VAL, بټ_پلنوالی_2), /*isSigned=*/ریښتیا);

  إذا (پلنوالی == بټ_پلنوالی_2)
    أرجع *هذا;

  APInt پایله(get_حافظه(get_عدد_ټکي(پلنوالی)), پلنوالی);

  // Copy words.
  std::حافظه_نقل(پایله.U.pVal, get_Raw_معلومات(), get_عدد_ټکي() * APINT_کلمه_اندازه);

  // Sign extend the last word since there may be unused bits in the input.
  پایله.U.pVal[get_عدد_ټکي() - 1] =
      SignExtend64(پایله.U.pVal[get_عدد_ټکي() - 1],
                   ((بټ_پلنوالی_2 - 1) % APINT_بټونه_فی_کلمه) + 1);

  // Fill with sign bits.
  std::memset(پایله.U.pVal + get_عدد_ټکي(), isNegative() ? -1 : 0,
              (پایله.get_عدد_ټکي() - get_عدد_ټکي()) * APINT_کلمه_اندازه);
  پایله.clear_ناکارول_شوی_بټونه();
  أرجع پایله;
}

//  Zero extend to a new width.
APInt APInt::zext(بینښه پلنوالی_2) ثابت {
  أكد(پلنوالی_2 >= بټ_پلنوالی_2 && "Invalid APInt ZeroExtend request");

  إذا (پلنوالی_2 <= APINT_بټونه_فی_کلمه)
    أرجع APInt(پلنوالی_2, U.VAL);

  إذا (پلنوالی_2 == بټ_پلنوالی_2)
    أرجع *هذا;

  APInt پایله(get_حافظه(get_عدد_ټکي(پلنوالی_2)), پلنوالی_2);

  // Copy words.
  std::حافظه_نقل(پایله.U.pVal, get_Raw_معلومات(), get_عدد_ټکي() * APINT_کلمه_اندازه);

  // Zero remaining words.
  std::memset(پایله.U.pVal + get_عدد_ټکي(), 0,
              (پایله.get_عدد_ټکي() - get_عدد_ټکي()) * APINT_کلمه_اندازه);

  أرجع پایله;
}

APInt APInt::zextOrTrunc(بینښه پلنوالی_2) ثابت {
  إذا (بټ_پلنوالی_2 < پلنوالی_2)
    أرجع zext(پلنوالی_2);
  إذا (بټ_پلنوالی_2 > پلنوالی_2)
    أرجع trunc(پلنوالی_2);
  أرجع *هذا;
}

APInt APInt::sextOrTrunc(بینښه پلنوالی_2) ثابت {
  إذا (بټ_پلنوالی_2 < پلنوالی_2)
    أرجع sext(پلنوالی_2);
  إذا (بټ_پلنوالی_2 > پلنوالی_2)
    أرجع trunc(پلنوالی_2);
  أرجع *هذا;
}

/// Arithmetic right-shift this APInt by shiftAmt.
/// Arithmetic right-shift function.
فراغ APInt::ashr_دننه_ځای_پرځای_کړه(ثابت APInt &ښويدنه_Amt) {
  ashr_دننه_ځای_پرځای_کړه((بینښه)ښويدنه_Amt.get_Limited_ارزښت(بټ_پلنوالی_2));
}

/// Arithmetic right-shift this APInt by shiftAmt.
/// Arithmetic right-shift function.
فراغ APInt::ashrSlowCase(بینښه ښويدنه_Amt_2) {
  // Don't bother performing a no-op shift.
  إذا (!ښويدنه_Amt_2)
    أرجع;

  // Save the original sign bit for later.
  منطقي Negative = isNegative();

  // WordShift is the inter-part shift; BitShift is intra-part shift.
  بینښه کلمه_ښويدنه = ښويدنه_Amt_2 / APINT_بټونه_فی_کلمه;
  بینښه بټ_ښويدنه = ښويدنه_Amt_2 % APINT_بټونه_فی_کلمه;

  بینښه ټکي_منزل_Move = get_عدد_ټکي() - کلمه_ښويدنه;
  إذا (ټکي_منزل_Move != 0) {
    // Sign extend the last word to fill in the unused bits.
    U.pVal[get_عدد_ټکي() - 1] = SignExtend64(
        U.pVal[get_عدد_ټکي() - 1], ((بټ_پلنوالی_2 - 1) % APINT_بټونه_فی_کلمه) + 1);

    // Fastpath for moving by whole words.
    إذا (بټ_ښويدنه == 0) {
      std::memmove(U.pVal, U.pVal + کلمه_ښويدنه, ټکي_منزل_Move * APINT_کلمه_اندازه);
    } وإلا {
      // Move the words containing significant bits.
      لكل (بینښه i = 0; i != ټکي_منزل_Move - 1; ++i)
        U.pVal[i] = (U.pVal[i + کلمه_ښويدنه] >> بټ_ښويدنه) |
                    (U.pVal[i + کلمه_ښويدنه + 1] << (APINT_بټونه_فی_کلمه - بټ_ښويدنه));

      // Handle the last word which has no high bits to copy. Use an arithmetic
      // shift to preserve the sign bit.
      U.pVal[ټکي_منزل_Move - 1] =
          (int64_t)U.pVal[کلمه_ښويدنه + ټکي_منزل_Move - 1] >> بټ_ښويدنه;
    }
  }

  // Fill in the remainder based on the original sign.
  std::memset(U.pVal + ټکي_منزل_Move, Negative ? -1 : 0,
              کلمه_ښويدنه * APINT_کلمه_اندازه);
  clear_ناکارول_شوی_بټونه();
}

/// Logical right-shift this APInt by shiftAmt.
/// Logical right-shift function.
فراغ APInt::lshr_دننه_ځای_پرځای_کړه(ثابت APInt &ښويدنه_Amt) {
  lshr_دننه_ځای_پرځای_کړه((بینښه)ښويدنه_Amt.get_Limited_ارزښت(بټ_پلنوالی_2));
}

/// Logical right-shift this APInt by shiftAmt.
/// Logical right-shift function.
فراغ APInt::lshrSlowCase(بینښه ښويدنه_Amt_2) {
  tc_ښويدنه_ښی(U.pVal, get_عدد_ټکي(), ښويدنه_Amt_2);
}

/// Left-shift this APInt by shiftAmt.
/// Left-shift function.
APInt &APInt::چلوونکی<<=(ثابت APInt &ښويدنه_Amt) {
  // It's undefined behavior in C to shift by BitWidth or greater.
  *هذا <<= (بینښه)ښويدنه_Amt.get_Limited_ارزښت(بټ_پلنوالی_2);
  أرجع *هذا;
}

فراغ APInt::shlSlowCase(بینښه ښويدنه_Amt_2) {
  tc_ښويدنه_کیڼ(U.pVal, get_عدد_ټکي(), ښويدنه_Amt_2);
  clear_ناکارول_شوی_بټونه();
}

// Calculate the rotate amount modulo the bit width.
ساكن بینښه rotateModulo(بینښه بټ_پلنوالی_2, ثابت APInt &rotateAmt) {
  إذا (LLVM_UNLIKELY(بټ_پلنوالی_2 == 0))
    أرجع 0;
  بینښه rot_بټ_پلنوالی = rotateAmt.get_بټ_پلنوالی();
  APInt rot = rotateAmt;
  إذا (rot_بټ_پلنوالی < بټ_پلنوالی_2) {
    // Extend the rotate APInt, so that the urem doesn't divide by 0.
    // e.g. APInt(1, 32) would give APInt(1, 0).
    rot = rotateAmt.zext(بټ_پلنوالی_2);
  }
  rot = rot.urem(APInt(rot.get_بټ_پلنوالی(), بټ_پلنوالی_2));
  أرجع rot.get_Limited_ارزښت(بټ_پلنوالی_2);
}

APInt APInt::rotl(ثابت APInt &rotateAmt) ثابت {
  أرجع rotl(rotateModulo(بټ_پلنوالی_2, rotateAmt));
}

APInt APInt::rotl(بینښه rotateAmt) ثابت {
  إذا (LLVM_UNLIKELY(بټ_پلنوالی_2 == 0))
    أرجع *هذا;
  rotateAmt %= بټ_پلنوالی_2;
  إذا (rotateAmt == 0)
    أرجع *هذا;
  أرجع shl(rotateAmt) | lshr(بټ_پلنوالی_2 - rotateAmt);
}

APInt APInt::rotr(ثابت APInt &rotateAmt) ثابت {
  أرجع rotr(rotateModulo(بټ_پلنوالی_2, rotateAmt));
}

APInt APInt::rotr(بینښه rotateAmt) ثابت {
  إذا (بټ_پلنوالی_2 == 0)
    أرجع *هذا;
  rotateAmt %= بټ_پلنوالی_2;
  إذا (rotateAmt == 0)
    أرجع *هذا;
  أرجع lshr(rotateAmt) | shl(بټ_پلنوالی_2 - rotateAmt);
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
بینښه APInt::تر_ټولو_نږدې_Log_Base2() ثابت {
  // Special case when we have a bitwidth of 1. If VAL is 1, then we
  // get 0. If VAL is 0, we get WORDTYPE_MAX which gets truncated to
  // UINT32_MAX.
  إذا (بټ_پلنوالی_2 == 1)
    أرجع U.VAL - 1;

  // Handle the zero case.
  إذا (isZero())
    أرجع UINT32_MAX;

  // The non-zero case is handled by computing:
  //
  //   nearestLogBase2(x) = logBase2(x) + x[logBase2(x)-1].
  //
  // where x[i] is referring to the value of the ith bit of x.
  بینښه lg = logBase2();
  أرجع lg + بینښه((*هذا)[lg - 1]);
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
  بینښه magnitude = get_فعال_بټونه();

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
    أرجع APInt(بټ_پلنوالی_2,
                 results[(LLVM_LIKELY(is_Single_کلمه()) ? U.VAL : U.pVal[0])]);
  }

  // If the magnitude of the value fits in less than 52 bits (the precision of
  // an IEEE double precision floating point value), then we can use the
  // libc sqrt function which will probably use a hardware sqrt computation.
  // This should be faster than the algorithm below.
  إذا (magnitude < 52) {
    أرجع APInt(بټ_پلنوالی_2,
                 uint64_t(::floor(::جذر(مزدوج(
                     LLVM_LIKELY(is_Single_کلمه()) ? U.VAL : U.pVal[0])))));
  }

  // Okay, all the short cuts are exhausted. We must compute it. The following
  // is a classical Babylonian method for computing the square root. This code
  // was adapted to APInt from a wikipedia article on such computations.
  // See http://www.wikipedia.org/ and go to the page named
  // Calculate_an_integer_square_root.
  بینښه nbits = بټ_پلنوالی_2, i = 4;
  APInt testy(بټ_پلنوالی_2, 16);
  APInt x_old(بټ_پلنوالی_2, 1);
  APInt x_new(بټ_پلنوالی_2, 0);
  APInt two(بټ_پلنوالی_2, 2);

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
  APInt ضریب = *هذا;
  APInt T;
  طالما (!(T = *هذا * ضریب).isOne())
    ضریب *= 2 - std::move(T);
  أرجع ضریب;
}

/// Implementation of Knuth's Algorithm D (Division of nonnegative integers)
/// from "Art of Computer Programming, Volume 2", section 4.3.1, p. 272. The
/// variables here have the same names as in the algorithm. Comments explain
/// the algorithm and any deviation from it.
ساكن فراغ KnuthDiv(uint32_t *u, uint32_t *v, uint32_t *q, uint32_t* r,
                     بینښه m, بینښه n) {
  أكد(u && "Must provide dividend");
  أكد(v && "Must provide divisor");
  أكد(q && "Must provide quotient");
  أكد(u != v && u != q && v != q && "Must use different memory");
  أكد(n>1 && "n must be > 1");

  // b denotes the base of the number system. In our case b is 2^32.
  ثابت uint64_t b = uint64_t(1) << 32;

// The DEBUG macros here tend to be spam in the debug output if you're not
// debugging this code. Disable them unless KNUTH_DEBUG is defined.
#که_تعریف_شوی KNUTH_DEBUG
#تعریف_کړه DEBUG_KNUTH(X) LLVM_DEBUG(X)
#وإلا
#تعریف_کړه DEBUG_KNUTH(X) افعل {} طالما(خطأ)
#د_که_پای

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
  بینښه ښويدنه = llvm::countl_zero(v[n - 1]);
  uint32_t v_لېږد = 0;
  uint32_t u_لېږد = 0;
  إذا (ښويدنه) {
    لكل (بینښه i = 0; i < m+n; ++i) {
      uint32_t u_tmp = u[i] >> (32 - ښويدنه);
      u[i] = (u[i] << ښويدنه) | u_لېږد;
      u_لېږد = u_tmp;
    }
    لكل (بینښه i = 0; i < n; ++i) {
      uint32_t v_tmp = v[i] >> (32 - ښويدنه);
      v[i] = (v[i] << ښويدنه) | v_لېږد;
      v_لېږد = v_tmp;
    }
  }
  u[m+n] = u_لېږد;

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
    لكل (بینښه i = 0; i < n; ++i) {
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
      منطقي لېږد = خطأ;
      لكل (بینښه i = 0; i < n; i++) {
        uint32_t حد = std::min(u[j+i],v[i]);
        u[j+i] += v[i] + لېږد;
        لېږد = u[j+i] < حد || (لېږد && u[j+i] == حد);
      }
      u[j+n] += لېږد;
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
    إذا (ښويدنه) {
      uint32_t لېږد = 0;
      DEBUG_KNUTH(dbgs() << "KnuthDiv: remainder:");
      لكل (صحيح i = n-1; i >= 0; i--) {
        r[i] = (u[i] >> ښويدنه) | لېږد;
        لېږد = u[i] << (32 - ښويدنه);
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

فراغ APInt::divide(ثابت کلمه_Type *LHS, بینښه lhs_ټکي, ثابت کلمه_Type *RHS,
                   بینښه rhs_ټکي, کلمه_Type *Quotient, کلمه_Type *Remainder) {
  أكد(lhs_ټکي >= rhs_ټکي && "Fractional result");

  // First, compose the values into an array of 32-bit words instead of
  // 64-bit words. This is a necessity of both the "short division" algorithm
  // and the Knuth "classical algorithm" which requires there to be native
  // operations for +, -, and * on an m bit value with an m*2 bit result. We
  // can't use 64-bit operands here because we don't have native results of
  // 128-bits. Furthermore, casting the 64-bit values to 32-bit values won't
  // work on large-endian machines.
  بینښه n = rhs_ټکي * 2;
  بینښه m = (lhs_ټکي * 2) - n;

  // Allocate space for the temporary values we need either on the stack, if
  // it will fit, or on the heap if it won't.
  uint32_t SPACE[128];
  uint32_t *U = تش_اشاره;
  uint32_t *V = تش_اشاره;
  uint32_t *Q = تش_اشاره;
  uint32_t *R = تش_اشاره;
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
  لكل (بینښه i = 0; i < lhs_ټکي; ++i) {
    uint64_t tmp = LHS[i];
    U[i * 2] = Lo_32(tmp);
    U[i * 2 + 1] = Hi_32(tmp);
  }
  U[m+n] = 0; // this extra word is for "spill" in the Knuth algorithm.

  // Initialize the divisor
  memset(V, 0, (n)*حجم(uint32_t));
  لكل (بینښه i = 0; i < rhs_ټکي; ++i) {
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
  لكل (بینښه i = n; i > 0 && V[i-1] == 0; i--) {
    n--;
    m++;
  }
  لكل (بینښه i = m+n; i > 0 && U[i-1] == 0; i--)
    m--;

  // If we're left with only a single word for the divisor, Knuth doesn't work
  // so we implement the short division algorithm here. This is much simpler
  // and faster because we are certain that we can divide a 64-bit quantity
  // by a 32-bit quantity at hardware speed and short division is simply a
  // series of such operations. This is just like doing short division but we
  // are using base 2^32 instead of base 10.
  أكد(n != 0 && "Divide by zero?");
  إذا (n == 1) {
    uint32_t وېشونکی = V[0];
    uint32_t remainder = 0;
    لكل (صحيح i = m; i >= 0; i--) {
      uint64_t partial_dividend = Make_64(remainder, U[i]);
      إذا (partial_dividend == 0) {
        Q[i] = 0;
        remainder = 0;
      } وإلا إذا (partial_dividend < وېشونکی) {
        Q[i] = 0;
        remainder = Lo_32(partial_dividend);
      } وإلا إذا (partial_dividend == وېشونکی) {
        Q[i] = 1;
        remainder = 0;
      } وإلا {
        Q[i] = Lo_32(partial_dividend / وېشونکی);
        remainder = Lo_32(partial_dividend - (Q[i] * وېشونکی));
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
    لكل (بینښه i = 0; i < lhs_ټکي; ++i)
      Quotient[i] = Make_64(Q[i*2+1], Q[i*2]);
  }

  // If the caller wants the remainder
  إذا (Remainder) {
    لكل (بینښه i = 0; i < rhs_ټکي; ++i)
      Remainder[i] = Make_64(R[i*2+1], R[i*2]);
  }

  // Clean up the memory we allocated.
  إذا (U != &SPACE[0]) {
    ړنګول [] U;
    ړنګول [] V;
    ړنګول [] Q;
    ړنګول [] R;
  }
}

APInt APInt::udiv(ثابت APInt &RHS) ثابت {
  أكد(بټ_پلنوالی_2 == RHS.بټ_پلنوالی_2 && "Bit widths must be the same");

  // First, deal with the easy case
  إذا (LLVM_LIKELY(is_Single_کلمه())) {
    أكد(RHS.U.VAL != 0 && "Divide by zero?");
    أرجع APInt(بټ_پلنوالی_2, U.VAL / RHS.U.VAL);
  }

  // Get some facts about the LHS and RHS number of bits and words
  بینښه lhs_ټکي = get_عدد_ټکي(get_فعال_بټونه());
  بینښه rhs_بټونه  = RHS.get_فعال_بټونه();
  بینښه rhs_ټکي = get_عدد_ټکي(rhs_بټونه);
  أكد(rhs_ټکي && "Divided by zero???");

  // Deal with some degenerate cases
  إذا (!lhs_ټکي)
    // 0 / X ===> 0
    أرجع APInt(بټ_پلنوالی_2, 0);
  إذا (rhs_بټونه == 1)
    // X / 1 ===> X
    أرجع *هذا;
  إذا (lhs_ټکي < rhs_ټکي || هذا->ult(RHS))
    // X / Y ===> 0, iff X < Y
    أرجع APInt(بټ_پلنوالی_2, 0);
  إذا (*هذا == RHS)
    // X / X ===> 1
    أرجع APInt(بټ_پلنوالی_2, 1);
  إذا (lhs_ټکي == 1) // rhsWords is 1 if lhsWords is 1.
    // All high words are zero, just use native divide
    أرجع APInt(بټ_پلنوالی_2, هذا->U.pVal[0] / RHS.U.pVal[0]);

  // We have to compute it the hard way. Invoke the Knuth divide algorithm.
  APInt Quotient(بټ_پلنوالی_2, 0); // to hold result.
  divide(U.pVal, lhs_ټکي, RHS.U.pVal, rhs_ټکي, Quotient.U.pVal, تش_اشاره);
  أرجع Quotient;
}

APInt APInt::udiv(uint64_t RHS) ثابت {
  أكد(RHS != 0 && "Divide by zero?");

  // First, deal with the easy case
  إذا (LLVM_LIKELY(is_Single_کلمه()))
    أرجع APInt(بټ_پلنوالی_2, U.VAL / RHS);

  // Get some facts about the LHS words.
  بینښه lhs_ټکي = get_عدد_ټکي(get_فعال_بټونه());

  // Deal with some degenerate cases
  إذا (!lhs_ټکي)
    // 0 / X ===> 0
    أرجع APInt(بټ_پلنوالی_2, 0);
  إذا (RHS == 1)
    // X / 1 ===> X
    أرجع *هذا;
  إذا (هذا->ult(RHS))
    // X / Y ===> 0, iff X < Y
    أرجع APInt(بټ_پلنوالی_2, 0);
  إذا (*هذا == RHS)
    // X / X ===> 1
    أرجع APInt(بټ_پلنوالی_2, 1);
  إذا (lhs_ټکي == 1) // rhsWords is 1 if lhsWords is 1.
    // All high words are zero, just use native divide
    أرجع APInt(بټ_پلنوالی_2, هذا->U.pVal[0] / RHS);

  // We have to compute it the hard way. Invoke the Knuth divide algorithm.
  APInt Quotient(بټ_پلنوالی_2, 0); // to hold result.
  divide(U.pVal, lhs_ټکي, &RHS, 1, Quotient.U.pVal, تش_اشاره);
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
  أكد(بټ_پلنوالی_2 == RHS.بټ_پلنوالی_2 && "Bit widths must be the same");
  إذا (LLVM_LIKELY(is_Single_کلمه())) {
    أكد(RHS.U.VAL != 0 && "Remainder by zero?");
    أرجع APInt(بټ_پلنوالی_2, U.VAL % RHS.U.VAL);
  }

  // Get some facts about the LHS
  بینښه lhs_ټکي = get_عدد_ټکي(get_فعال_بټونه());

  // Get some facts about the RHS
  بینښه rhs_بټونه = RHS.get_فعال_بټونه();
  بینښه rhs_ټکي = get_عدد_ټکي(rhs_بټونه);
  أكد(rhs_ټکي && "Performing remainder operation by zero ???");

  // Check the degenerate cases
  إذا (lhs_ټکي == 0)
    // 0 % Y ===> 0
    أرجع APInt(بټ_پلنوالی_2, 0);
  إذا (rhs_بټونه == 1)
    // X % 1 ===> 0
    أرجع APInt(بټ_پلنوالی_2, 0);
  إذا (lhs_ټکي < rhs_ټکي || هذا->ult(RHS))
    // X % Y ===> X, iff X < Y
    أرجع *هذا;
  إذا (*هذا == RHS)
    // X % X == 0;
    أرجع APInt(بټ_پلنوالی_2, 0);
  إذا (lhs_ټکي == 1)
    // All high words are zero, just use native remainder
    أرجع APInt(بټ_پلنوالی_2, U.pVal[0] % RHS.U.pVal[0]);
  إذا (RHS.isPowerOf2()) {
    // X % 2^w ===> X & (2^w - 1)
    APInt پایله(*هذا);
    پایله.clear_بټونه(RHS.logBase2(), بټ_پلنوالی_2);
    أرجع پایله;
  }

  // We have to compute it the hard way. Invoke the Knuth divide algorithm.
  APInt Remainder(بټ_پلنوالی_2, 0);
  divide(U.pVal, lhs_ټکي, RHS.U.pVal, rhs_ټکي, تش_اشاره, Remainder.U.pVal);
  أرجع Remainder;
}

uint64_t APInt::urem(uint64_t RHS) ثابت {
  أكد(RHS != 0 && "Remainder by zero?");

  إذا (LLVM_LIKELY(is_Single_کلمه()))
    أرجع U.VAL % RHS;

  // Get some facts about the LHS
  بینښه lhs_ټکي = get_عدد_ټکي(get_فعال_بټونه());

  // Check the degenerate cases
  إذا (lhs_ټکي == 0)
    // 0 % Y ===> 0
    أرجع 0;
  إذا (RHS == 1)
    // X % 1 ===> 0
    أرجع 0;
  إذا (هذا->ult(RHS))
    // X % Y ===> X, iff X < Y
    أرجع get_ZExt_ارزښت();
  إذا (*هذا == RHS)
    // X % X == 0;
    أرجع 0;
  إذا (lhs_ټکي == 1)
    // All high words are zero, just use native remainder
    أرجع U.pVal[0] % RHS;
  إذا (llvm::isPowerOf2_64(RHS))
    // X % 2^w ===> X & (2^w - 1)
    أرجع U.pVal[0] & (RHS - 1);

  // We have to compute it the hard way. Invoke the Knuth divide algorithm.
  uint64_t Remainder;
  divide(U.pVal, lhs_ټکي, &RHS, 1, تش_اشاره, &Remainder);
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
  أكد(LHS.بټ_پلنوالی_2 == RHS.بټ_پلنوالی_2 && "Bit widths must be the same");
  بینښه بټ_پلنوالی_2 = LHS.بټ_پلنوالی_2;

  // First, deal with the easy case
  إذا (LLVM_LIKELY(LHS.is_Single_کلمه())) {
    أكد(RHS.U.VAL != 0 && "Divide by zero?");
    uint64_t QuotVal = LHS.U.VAL / RHS.U.VAL;
    uint64_t RemVal = LHS.U.VAL % RHS.U.VAL;
    Quotient = APInt(بټ_پلنوالی_2, QuotVal);
    Remainder = APInt(بټ_پلنوالی_2, RemVal);
    أرجع;
  }

  // Get some size facts about the dividend and divisor
  بینښه lhs_ټکي = get_عدد_ټکي(LHS.get_فعال_بټونه());
  بینښه rhs_بټونه  = RHS.get_فعال_بټونه();
  بینښه rhs_ټکي = get_عدد_ټکي(rhs_بټونه);
  أكد(rhs_ټکي && "Performing divrem operation by zero ???");

  // Check the degenerate cases
  إذا (lhs_ټکي == 0) {
    Quotient = APInt(بټ_پلنوالی_2, 0);    // 0 / Y ===> 0
    Remainder = APInt(بټ_پلنوالی_2, 0);   // 0 % Y ===> 0
    أرجع;
  }

  إذا (rhs_بټونه == 1) {
    Quotient = LHS;                   // X / 1 ===> X
    Remainder = APInt(بټ_پلنوالی_2, 0);   // X % 1 ===> 0
  }

  إذا (lhs_ټکي < rhs_ټکي || LHS.ult(RHS)) {
    Remainder = LHS;                  // X % Y ===> X, iff X < Y
    Quotient = APInt(بټ_پلنوالی_2, 0);    // X / Y ===> 0, iff X < Y
    أرجع;
  }

  إذا (LHS == RHS) {
    Quotient  = APInt(بټ_پلنوالی_2, 1);   // X / X ===> 1
    Remainder = APInt(بټ_پلنوالی_2, 0);   // X % X ===> 0;
    أرجع;
  }

  // Make sure there is enough space to hold the results.
  // NOTE: This assumes that reallocate won't affect any bits if it doesn't
  // change the size. This is necessary if Quotient or Remainder is aliased
  // with LHS or RHS.
  Quotient.reallocate(بټ_پلنوالی_2);
  Remainder.reallocate(بټ_پلنوالی_2);

  إذا (lhs_ټکي == 1) { // rhsWords is 1 if lhsWords is 1.
    // There is only one word to consider so use the native versions.
    uint64_t lhs_ارزښت = LHS.U.pVal[0];
    uint64_t rhs_ارزښت = RHS.U.pVal[0];
    Quotient = lhs_ارزښت / rhs_ارزښت;
    Remainder = lhs_ارزښت % rhs_ارزښت;
    أرجع;
  }

  // Okay, lets do it the long way
  divide(LHS.U.pVal, lhs_ټکي, RHS.U.pVal, rhs_ټکي, Quotient.U.pVal,
         Remainder.U.pVal);
  // Clear the rest of the Quotient and Remainder.
  std::memset(Quotient.U.pVal + lhs_ټکي, 0,
              (get_عدد_ټکي(بټ_پلنوالی_2) - lhs_ټکي) * APINT_کلمه_اندازه);
  std::memset(Remainder.U.pVal + rhs_ټکي, 0,
              (get_عدد_ټکي(بټ_پلنوالی_2) - rhs_ټکي) * APINT_کلمه_اندازه);
}

فراغ APInt::udivrem(ثابت APInt &LHS, uint64_t RHS, APInt &Quotient,
                    uint64_t &Remainder) {
  أكد(RHS != 0 && "Divide by zero?");
  بینښه بټ_پلنوالی_2 = LHS.بټ_پلنوالی_2;

  // First, deal with the easy case
  إذا (LLVM_LIKELY(LHS.is_Single_کلمه())) {
    uint64_t QuotVal = LHS.U.VAL / RHS;
    Remainder = LHS.U.VAL % RHS;
    Quotient = APInt(بټ_پلنوالی_2, QuotVal);
    أرجع;
  }

  // Get some size facts about the dividend and divisor
  بینښه lhs_ټکي = get_عدد_ټکي(LHS.get_فعال_بټونه());

  // Check the degenerate cases
  إذا (lhs_ټکي == 0) {
    Quotient = APInt(بټ_پلنوالی_2, 0);    // 0 / Y ===> 0
    Remainder = 0;                    // 0 % Y ===> 0
    أرجع;
  }

  إذا (RHS == 1) {
    Quotient = LHS;                   // X / 1 ===> X
    Remainder = 0;                    // X % 1 ===> 0
    أرجع;
  }

  إذا (LHS.ult(RHS)) {
    Remainder = LHS.get_ZExt_ارزښت();   // X % Y ===> X, iff X < Y
    Quotient = APInt(بټ_پلنوالی_2, 0);    // X / Y ===> 0, iff X < Y
    أرجع;
  }

  إذا (LHS == RHS) {
    Quotient  = APInt(بټ_پلنوالی_2, 1);   // X / X ===> 1
    Remainder = 0;                    // X % X ===> 0;
    أرجع;
  }

  // Make sure there is enough space to hold the results.
  // NOTE: This assumes that reallocate won't affect any bits if it doesn't
  // change the size. This is necessary if Quotient is aliased with LHS.
  Quotient.reallocate(بټ_پلنوالی_2);

  إذا (lhs_ټکي == 1) { // rhsWords is 1 if lhsWords is 1.
    // There is only one word to consider so use the native versions.
    uint64_t lhs_ارزښت = LHS.U.pVal[0];
    Quotient = lhs_ارزښت / RHS;
    Remainder = lhs_ارزښت % RHS;
    أرجع;
  }

  // Okay, lets do it the long way
  divide(LHS.U.pVal, lhs_ټکي, &RHS, 1, Quotient.U.pVal, &Remainder);
  // Clear the rest of the Quotient.
  std::memset(Quotient.U.pVal + lhs_ټکي, 0,
              (get_عدد_ټکي(بټ_پلنوالی_2) - lhs_ټکي) * APINT_کلمه_اندازه);
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

APInt APInt::sadd_ov(ثابت APInt &RHS, منطقي &ډکېدنه) ثابت {
  APInt Res = *هذا+RHS;
  ډکېدنه = isNonNegative() == RHS.isNonNegative() &&
             Res.isNonNegative() != isNonNegative();
  أرجع Res;
}

APInt APInt::uadd_ov(ثابت APInt &RHS, منطقي &ډکېدنه) ثابت {
  APInt Res = *هذا+RHS;
  ډکېدنه = Res.ult(RHS);
  أرجع Res;
}

APInt APInt::ssub_ov(ثابت APInt &RHS, منطقي &ډکېدنه) ثابت {
  APInt Res = *هذا - RHS;
  ډکېدنه = isNonNegative() != RHS.isNonNegative() &&
             Res.isNonNegative() != isNonNegative();
  أرجع Res;
}

APInt APInt::usub_ov(ثابت APInt &RHS, منطقي &ډکېدنه) ثابت {
  APInt Res = *هذا-RHS;
  ډکېدنه = Res.ugt(*هذا);
  أرجع Res;
}

APInt APInt::sdiv_ov(ثابت APInt &RHS, منطقي &ډکېدنه) ثابت {
  // MININT/-1  -->  overflow.
  ډکېدنه = is_Min_Signed_ارزښت() && RHS.isAllOnes();
  أرجع sdiv(RHS);
}

APInt APInt::smul_ov(ثابت APInt &RHS, منطقي &ډکېدنه) ثابت {
  APInt Res = *هذا * RHS;

  إذا (RHS != 0)
    ډکېدنه = Res.sdiv(RHS) != *هذا ||
               (is_Min_Signed_ارزښت() && RHS.isAllOnes());
  وإلا
    ډکېدنه = خطأ;
  أرجع Res;
}

APInt APInt::umul_ov(ثابت APInt &RHS, منطقي &ډکېدنه) ثابت {
  إذا (countl_zero() + RHS.countl_zero() + 2 <= بټ_پلنوالی_2) {
    ډکېدنه = ریښتیا;
    أرجع *هذا * RHS;
  }

  APInt Res = lshr(1) * RHS;
  ډکېدنه = Res.isNegative();
  Res <<= 1;
  إذا ((*هذا)[0]) {
    Res += RHS;
    إذا (Res.ult(RHS))
      ډکېدنه = ریښتیا;
  }
  أرجع Res;
}

APInt APInt::sshl_ov(ثابت APInt &ShAmt, منطقي &ډکېدنه) ثابت {
  أرجع sshl_ov(ShAmt.get_Limited_ارزښت(get_بټ_پلنوالی()), ډکېدنه);
}

APInt APInt::sshl_ov(بینښه ShAmt, منطقي &ډکېدنه) ثابت {
  ډکېدنه = ShAmt >= get_بټ_پلنوالی();
  إذا (ډکېدنه)
    أرجع APInt(بټ_پلنوالی_2, 0);

  إذا (isNonNegative()) // Don't allow sign change.
    ډکېدنه = ShAmt >= countl_zero();
  وإلا
    ډکېدنه = ShAmt >= countl_one();

  أرجع *هذا << ShAmt;
}

APInt APInt::ushl_ov(ثابت APInt &ShAmt, منطقي &ډکېدنه) ثابت {
  أرجع ushl_ov(ShAmt.get_Limited_ارزښت(get_بټ_پلنوالی()), ډکېدنه);
}

APInt APInt::ushl_ov(بینښه ShAmt, منطقي &ډکېدنه) ثابت {
  ډکېدنه = ShAmt >= get_بټ_پلنوالی();
  إذا (ډکېدنه)
    أرجع APInt(بټ_پلنوالی_2, 0);

  ډکېدنه = ShAmt > countl_zero();

  أرجع *هذا << ShAmt;
}

APInt APInt::sfloordiv_ov(ثابت APInt &RHS, منطقي &ډکېدنه) ثابت {
  APInt quotient = sdiv_ov(RHS, ډکېدنه);
  إذا ((quotient * RHS != *هذا) && (isNegative() != RHS.isNegative()))
    أرجع quotient - 1;
  أرجع quotient;
}

APInt APInt::sadd_sat(ثابت APInt &RHS) ثابت {
  منطقي ډکېدنه;
  APInt Res = sadd_ov(RHS, ډکېدنه);
  إذا (!ډکېدنه)
    أرجع Res;

  أرجع isNegative() ? APInt::get_Signed_Min_ارزښت(بټ_پلنوالی_2)
                      : APInt::get_Signed_Max_ارزښت(بټ_پلنوالی_2);
}

APInt APInt::uadd_sat(ثابت APInt &RHS) ثابت {
  منطقي ډکېدنه;
  APInt Res = uadd_ov(RHS, ډکېدنه);
  إذا (!ډکېدنه)
    أرجع Res;

  أرجع APInt::get_Max_ارزښت(بټ_پلنوالی_2);
}

APInt APInt::ssub_sat(ثابت APInt &RHS) ثابت {
  منطقي ډکېدنه;
  APInt Res = ssub_ov(RHS, ډکېدنه);
  إذا (!ډکېدنه)
    أرجع Res;

  أرجع isNegative() ? APInt::get_Signed_Min_ارزښت(بټ_پلنوالی_2)
                      : APInt::get_Signed_Max_ارزښت(بټ_پلنوالی_2);
}

APInt APInt::usub_sat(ثابت APInt &RHS) ثابت {
  منطقي ډکېدنه;
  APInt Res = usub_ov(RHS, ډکېدنه);
  إذا (!ډکېدنه)
    أرجع Res;

  أرجع APInt(بټ_پلنوالی_2, 0);
}

APInt APInt::smul_sat(ثابت APInt &RHS) ثابت {
  منطقي ډکېدنه;
  APInt Res = smul_ov(RHS, ډکېدنه);
  إذا (!ډکېدنه)
    أرجع Res;

  // The result is negative if one and only one of inputs is negative.
  منطقي ResIsNegative = isNegative() ^ RHS.isNegative();

  أرجع ResIsNegative ? APInt::get_Signed_Min_ارزښت(بټ_پلنوالی_2)
                       : APInt::get_Signed_Max_ارزښت(بټ_پلنوالی_2);
}

APInt APInt::umul_sat(ثابت APInt &RHS) ثابت {
  منطقي ډکېدنه;
  APInt Res = umul_ov(RHS, ډکېدنه);
  إذا (!ډکېدنه)
    أرجع Res;

  أرجع APInt::get_Max_ارزښت(بټ_پلنوالی_2);
}

APInt APInt::sshl_sat(ثابت APInt &RHS) ثابت {
  أرجع sshl_sat(RHS.get_Limited_ارزښت(get_بټ_پلنوالی()));
}

APInt APInt::sshl_sat(بینښه RHS) ثابت {
  منطقي ډکېدنه;
  APInt Res = sshl_ov(RHS, ډکېدنه);
  إذا (!ډکېدنه)
    أرجع Res;

  أرجع isNegative() ? APInt::get_Signed_Min_ارزښت(بټ_پلنوالی_2)
                      : APInt::get_Signed_Max_ارزښت(بټ_پلنوالی_2);
}

APInt APInt::ushl_sat(ثابت APInt &RHS) ثابت {
  أرجع ushl_sat(RHS.get_Limited_ارزښت(get_بټ_پلنوالی()));
}

APInt APInt::ushl_sat(بینښه RHS) ثابت {
  منطقي ډکېدنه;
  APInt Res = ushl_ov(RHS, ډکېدنه);
  إذا (!ډکېدنه)
    أرجع Res;

  أرجع APInt::get_Max_ارزښت(بټ_پلنوالی_2);
}

فراغ APInt::fromString(بینښه numbits, StringRef str, uint8_t radix) {
  // Check our assumptions here
  أكد(!str.تشه() && "Invalid string length");
  أكد((radix == 10 || radix == 8 || radix == 16 || radix == 2 ||
          radix == 36) &&
         "Radix should be 2, 8, 10, 16, or 36!");

  StringRef::iterator p = str.begin();
  size_t slen = str.اندازه_2();
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
  إذا (LLVM_LIKELY(is_Single_کلمه()))
    U.VAL = 0;
  وإلا
    U.pVal = get_Cleared_حافظه(get_عدد_ټکي());

  // Figure out if we can shift instead of multiply
  بینښه ښويدنه = (radix == 16 ? 4 : radix == 8 ? 3 : radix == 2 ? 1 : 0);

  // Enter digit traversal loop
  لكل (StringRef::iterator e = str.پای_ټکی(); p != e; ++p) {
    بینښه digit = getDigit(*p, radix);
    أكد(digit < radix && "Invalid character in digit string");

    // Shift or multiply the value by the radix
    إذا (slen > 1) {
      إذا (ښويدنه)
        *هذا <<= ښويدنه;
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

فراغ APInt::منزل_String(Small_ویکټور_Impl<حرف> &Str, بینښه Radix, منطقي Signed,
                     منطقي formatAsCLiteral, منطقي پاسنی_Case_2,
                     منطقي ورننباسه_Separators) ثابت {
  أكد((Radix == 10 || Radix == 8 || Radix == 16 || Radix == 2 ||
          Radix == 36) &&
         "Radix should be 2, 8, 10, 16, or 36!");

  ثابت حرف *مختاړی = "";
  إذا (formatAsCLiteral) {
    بدل (Radix) {
      حالة 2:
        // Binary literals are a non-standard extension added in gcc 4.3:
        // http://gcc.gnu.org/onlinedocs/gcc-4.3.0/gcc/Binary-constants.html
        مختاړی = "0b";
        اكسر;
      حالة 8:
        مختاړی = "0";
        اكسر;
      حالة 10:
        اكسر; // No prefix
      حالة 16:
        مختاړی = "0x";
        اكسر;
      افتراضي:
        llvm_unreachable("Invalid radix!");
    }
  }

  // Number of digits in a group between separators.
  بینښه Grouping = (Radix == 8 || Radix == 10) ? 3 : 4;

  // First, check for a zero value and just short circuit the logic below.
  إذا (isZero()) {
    طالما (*مختاړی) {
      Str.ټېل_شا(*مختاړی);
      ++مختاړی;
    };
    Str.ټېل_شا('0');
    أرجع;
  }

  ساكن ثابت حرف BothDigits[] = "0123456789abcdefghijklmnopqrstuvwxyz"
                                   "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ";
  ثابت حرف *Digits = BothDigits + (پاسنی_Case_2 ? 36 : 0);

  إذا (LLVM_LIKELY(is_Single_کلمه())) {
    حرف بفر[65];
    حرف *BufPtr = std::پای_ټکی(بفر);

    uint64_t N;
    إذا (!Signed) {
      N = get_ZExt_ارزښت();
    } وإلا {
      int64_t I = get_SExt_ارزښت();
      إذا (I >= 0) {
        N = I;
      } وإلا {
        Str.ټېل_شا('-');
        N = -(uint64_t)I;
      }
    }

    طالما (*مختاړی) {
      Str.ټېل_شا(*مختاړی);
      ++مختاړی;
    };

    صحيح Pos = 0;
    طالما (N) {
      إذا (ورننباسه_Separators && Pos % Grouping == 0 && Pos > 0)
        *--BufPtr = '\'';
      *--BufPtr = Digits[N % Radix];
      N /= Radix;
      Pos++;
    }
    Str.append(BufPtr, std::پای_ټکی(بفر));
    أرجع;
  }

  APInt Tmp(*هذا);

  إذا (Signed && isNegative()) {
    // They want to print the signed version and it is a negative value
    // Flip the bits and add one to turn it into the equivalent positive
    // value and put a '-' in the result.
    Tmp.negate();
    Str.ټېل_شا('-');
  }

  طالما (*مختاړی) {
    Str.ټېل_شا(*مختاړی);
    ++مختاړی;
  }

  // We insert the digits backward, then reverse them to get the right order.
  بینښه پیل_Dig = Str.اندازه_2();

  // For the 2, 8 and 16 bit cases, we can just shift instead of divide
  // because the number of bits per digit (1, 3 and 4 respectively) divides
  // equally.  We just shift until the value is zero.
  إذا (Radix == 2 || Radix == 8 || Radix == 16) {
    // Just shift tmp right for each digit width until it becomes zero
    بینښه ښويدنه_Amt_2 = (Radix == 16 ? 4 : (Radix == 8 ? 3 : 1));
    بینښه ماسک_Amt = Radix - 1;

    صحيح Pos = 0;
    طالما (Tmp.get_Bool_ارزښت()) {
      بینښه Digit = بینښه(Tmp.get_Raw_معلومات()[0]) & ماسک_Amt;
      إذا (ورننباسه_Separators && Pos % Grouping == 0 && Pos > 0)
        Str.ټېل_شا('\'');

      Str.ټېل_شا(Digits[Digit]);
      Tmp.lshr_دننه_ځای_پرځای_کړه(ښويدنه_Amt_2);
      Pos++;
    }
  } وإلا {
    صحيح Pos = 0;
    طالما (Tmp.get_Bool_ارزښت()) {
      uint64_t Digit;
      udivrem(Tmp, Radix, Tmp, Digit);
      أكد(Digit < Radix && "divide failed");
      إذا (ورننباسه_Separators && Pos % Grouping == 0 && Pos > 0)
        Str.ټېل_شا('\'');

      Str.ټېل_شا(Digits[Digit]);
      Pos++;
    }
  }

  // Reverse the digits before returning.
  std::سرچپه(Str.begin()+پیل_Dig, Str.پای_ټکی());
}

#إذا !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)
LLVM_DUMP_لاره فراغ APInt::dump() ثابت {
  SmallString<40> S, U;
  هذا->منزل_String_Unsigned(U);
  هذا->منزل_String_Signed(S);
  dbgs() << "APInt(" << بټ_پلنوالی_2 << "b, "
         << U << "u " << S << "s)\n";
}
#د_که_پای

فراغ APInt::print(raw_ostream &OS, منطقي isSigned) ثابت {
  SmallString<40> S;
  هذا->منزل_String(S, 10, isSigned, /* formatAsCLiteral = */خطأ);
  OS << S;
}

// This implements a variety of operations on a representation of
// arbitrary precision, two's-complement, bignum integer values.

// Assumed by lowHalf, highHalf, partMSB and partLSB.  A fairly safe
// and unrestricting assumption.
static_assert(APInt::APINT_بټونه_فی_کلمه % 2 == 0,
              "Part width must be divisible by 2!");

// Returns the integer part with the least significant BITS set.
// BITS cannot be zero.
ساكن مضمن APInt::کلمه_Type ټیټ_بټ_ماسک(بینښه بټونه) {
  أكد(بټونه != 0 && بټونه <= APInt::APINT_بټونه_فی_کلمه);
  أرجع ~(APInt::کلمه_Type) 0 >> (APInt::APINT_بټونه_فی_کلمه - بټونه);
}

/// Returns the value of the lower half of PART.
ساكن مضمن APInt::کلمه_Type ټیټ_Half(APInt::کلمه_Type part) {
  أرجع part & ټیټ_بټ_ماسک(APInt::APINT_بټونه_فی_کلمه / 2);
}

/// Returns the value of the upper half of PART.
ساكن مضمن APInt::کلمه_Type لوړ_Half(APInt::کلمه_Type part) {
  أرجع part >> (APInt::APINT_بټونه_فی_کلمه / 2);
}

/// Sets the least significant part of a bignum to the input value, and zeroes
/// out higher parts.
فراغ APInt::tc_سېټ(کلمه_Type *dst, کلمه_Type part, بینښه parts) {
  أكد(parts > 0);
  dst[0] = part;
  لكل (بینښه i = 1; i < parts; i++)
    dst[i] = 0;
}

/// Assign one bignum to another.
فراغ APInt::tc_ټاکل(کلمه_Type *dst, ثابت کلمه_Type *src, بینښه parts) {
  لكل (بینښه i = 0; i < parts; i++)
    dst[i] = src[i];
}

/// Returns true if a bignum is zero, false otherwise.
منطقي APInt::tcIsZero(ثابت کلمه_Type *src, بینښه parts) {
  لكل (بینښه i = 0; i < parts; i++)
    إذا (src[i])
      أرجع خطأ;

  أرجع ریښتیا;
}

/// Extract the given bit of a bignum; returns 0 or 1.
صحيح APInt::tc_ایستل_بټ(ثابت کلمه_Type *parts, بینښه بټ) {
  أرجع (parts[which_کلمه(بټ)] & ماسک_بټ(بټ)) != 0;
}

/// Set the given bit of a bignum.
فراغ APInt::tc_سېټ_بټ(کلمه_Type *parts, بینښه بټ) {
  parts[which_کلمه(بټ)] |= ماسک_بټ(بټ);
}

/// Clears the given bit of a bignum.
فراغ APInt::tc_Clear_بټ(کلمه_Type *parts, بینښه بټ) {
  parts[which_کلمه(بټ)] &= ~ماسک_بټ(بټ);
}

/// Returns the bit number of the least significant set bit of a number.  If the
/// input number has no bits set UINT_MAX is returned.
بینښه APInt::tcLSB(ثابت کلمه_Type *parts, بینښه n) {
  لكل (بینښه i = 0; i < n; i++) {
    إذا (parts[i] != 0) {
      بینښه lsb = llvm::countr_zero(parts[i]);
      أرجع lsb + i * APINT_بټونه_فی_کلمه;
    }
  }

  أرجع UINT_MAX;
}

/// Returns the bit number of the most significant set bit of a number.
/// If the input number has no bits set UINT_MAX is returned.
بینښه APInt::tcMSB(ثابت کلمه_Type *parts, بینښه n) {
  افعل {
    --n;

    إذا (parts[n] != 0) {
      static_assert(حجم(parts[n]) <= حجم(uint64_t));
      بینښه msb = llvm::Log2_64(parts[n]);

      أرجع msb + n * APINT_بټونه_فی_کلمه;
    }
  } طالما (n);

  أرجع UINT_MAX;
}

/// Copy the bit vector of width srcBITS from SRC, starting at bit srcLSB, to
/// DST, of dstCOUNT parts, such that the bit srcLSB becomes the least
/// significant bit of DST.  All high bits above srcBITS in DST are zero-filled.
/// */
فراغ
APInt::tc_ایستل(کلمه_Type *dst, بینښه dst_شمېر, ثابت کلمه_Type *src,
                 بینښه src_بټونه, بینښه srcLSB) {
  بینښه dstParts = (src_بټونه + APINT_بټونه_فی_کلمه - 1) / APINT_بټونه_فی_کلمه;
  أكد(dstParts <= dst_شمېر);

  بینښه لومړی_Src_Part = srcLSB / APINT_بټونه_فی_کلمه;
  tc_ټاکل(dst, src + لومړی_Src_Part, dstParts);

  بینښه ښويدنه = srcLSB % APINT_بټونه_فی_کلمه;
  tc_ښويدنه_ښی(dst, dstParts, ښويدنه);

  // We now have (dstParts * APINT_BITS_PER_WORD - shift) bits from SRC
  // in DST.  If this is less that srcBits, append the rest, else
  // clear the high bits.
  بینښه n = dstParts * APINT_بټونه_فی_کلمه - ښويدنه;
  إذا (n < src_بټونه) {
    کلمه_Type ماسک_2 = ټیټ_بټ_ماسک (src_بټونه - n);
    dst[dstParts - 1] |= ((src[لومړی_Src_Part + dstParts] & ماسک_2)
                          << n % APINT_بټونه_فی_کلمه);
  } وإلا إذا (n > src_بټونه) {
    إذا (src_بټونه % APINT_بټونه_فی_کلمه)
      dst[dstParts - 1] &= ټیټ_بټ_ماسک (src_بټونه % APINT_بټونه_فی_کلمه);
  }

  // Clear high parts.
  طالما (dstParts < dst_شمېر)
    dst[dstParts++] = 0;
}

//// DST += RHS + C where C is zero or one.  Returns the carry flag.
APInt::کلمه_Type APInt::tc_جمع_کړه(کلمه_Type *dst, ثابت کلمه_Type *rhs,
                             کلمه_Type c, بینښه parts) {
  أكد(c <= 1);

  لكل (بینښه i = 0; i < parts; i++) {
    کلمه_Type l = dst[i];
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
APInt::کلمه_Type APInt::tc_جمع_کړه_Part(کلمه_Type *dst, کلمه_Type src,
                                 بینښه parts) {
  لكل (بینښه i = 0; i < parts; ++i) {
    dst[i] += src;
    إذا (dst[i] >= src)
      أرجع 0; // No need to carry so exit early.
    src = 1; // Carry one to next digit.
  }

  أرجع 1;
}

/// DST -= RHS + C where C is zero or one.  Returns the carry flag.
APInt::کلمه_Type APInt::tc_کمول(کلمه_Type *dst, ثابت کلمه_Type *rhs,
                                  کلمه_Type c, بینښه parts) {
  أكد(c <= 1);

  لكل (بینښه i = 0; i < parts; i++) {
    کلمه_Type l = dst[i];
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
APInt::کلمه_Type APInt::tc_کمول_Part(کلمه_Type *dst, کلمه_Type src,
                                      بینښه parts) {
  لكل (بینښه i = 0; i < parts; ++i) {
    کلمه_Type Dst = dst[i];
    dst[i] -= src;
    إذا (src <= Dst)
      أرجع 0; // No need to borrow so exit early.
    src = 1; // We have to "borrow 1" from next "word"
  }

  أرجع 1;
}

/// Negate a bignum in-place.
فراغ APInt::tcNegate(کلمه_Type *dst, بینښه parts) {
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
صحيح APInt::tcMultiplyPart(کلمه_Type *dst, ثابت کلمه_Type *src,
                          کلمه_Type multiplier, کلمه_Type لېږد,
                          بینښه srcParts, بینښه dstParts,
                          منطقي جمع_کړه) {
  // Otherwise our writes of DST kill our later reads of SRC.
  أكد(dst <= src || dst >= src + srcParts);
  أكد(dstParts <= srcParts + 1);

  // N loops; minimum of dstParts and srcParts.
  بینښه n = std::min(dstParts, srcParts);

  لكل (بینښه i = 0; i < n; i++) {
    // [LOW, HIGH] = MULTIPLIER * SRC[i] + DST[i] + CARRY.
    // This cannot overflow, because:
    //   (n - 1) * (n - 1) + 2 (n - 1) = (n - 1) * (n + 1)
    // which is less than n^2.
    کلمه_Type srcPart = src[i];
    کلمه_Type ټیټ, mid, لوړ;
    إذا (multiplier == 0 || srcPart == 0) {
      ټیټ = لېږد;
      لوړ = 0;
    } وإلا {
      ټیټ = ټیټ_Half(srcPart) * ټیټ_Half(multiplier);
      لوړ = لوړ_Half(srcPart) * لوړ_Half(multiplier);

      mid = ټیټ_Half(srcPart) * لوړ_Half(multiplier);
      لوړ += لوړ_Half(mid);
      mid <<= APINT_بټونه_فی_کلمه / 2;
      إذا (ټیټ + mid < ټیټ)
        لوړ++;
      ټیټ += mid;

      mid = لوړ_Half(srcPart) * ټیټ_Half(multiplier);
      لوړ += لوړ_Half(mid);
      mid <<= APINT_بټونه_فی_کلمه / 2;
      إذا (ټیټ + mid < ټیټ)
        لوړ++;
      ټیټ += mid;

      // Now add carry.
      إذا (ټیټ + لېږد < ټیټ)
        لوړ++;
      ټیټ += لېږد;
    }

    إذا (جمع_کړه) {
      // And now DST[i], and store the new low part there.
      إذا (ټیټ + dst[i] < ټیټ)
        لوړ++;
      dst[i] += ټیټ;
    } وإلا {
      dst[i] = ټیټ;
    }

    لېږد = لوړ;
  }

  إذا (srcParts < dstParts) {
    // Full multiplication, there is no overflow.
    أكد(srcParts + 1 == dstParts);
    dst[srcParts] = لېږد;
    أرجع 0;
  }

  // We overflowed if there is carry.
  إذا (لېږد)
    أرجع 1;

  // We would overflow if any significant unwritten parts would be
  // non-zero.  This is true if any remaining src parts are non-zero
  // and the multiplier is non-zero.
  إذا (multiplier)
    لكل (بینښه i = dstParts; i < srcParts; i++)
      إذا (src[i])
        أرجع 1;

  // We fitted in the narrow destination.
  أرجع 0;
}

/// DST = LHS * RHS, where DST has the same width as the operands and
/// is filled with the least significant parts of the result.  Returns
/// one if overflow occurred, otherwise zero.  DST must be disjoint
/// from both operands.
صحيح APInt::tcMultiply(کلمه_Type *dst, ثابت کلمه_Type *lhs,
                      ثابت کلمه_Type *rhs, بینښه parts) {
  أكد(dst != lhs && dst != rhs);

  صحيح ډکېدنه_2 = 0;

  لكل (بینښه i = 0; i < parts; i++) {
    // Don't accumulate on the first iteration so we don't need to initalize
    // dst to 0.
    ډکېدنه_2 |=
        tcMultiplyPart(&dst[i], lhs, rhs[i], 0, parts, parts - i, i != 0);
  }

  أرجع ډکېدنه_2;
}

/// DST = LHS * RHS, where DST has width the sum of the widths of the
/// operands. No overflow occurs. DST must be disjoint from both operands.
فراغ APInt::tc_ډک_Multiply(کلمه_Type *dst, ثابت کلمه_Type *lhs,
                           ثابت کلمه_Type *rhs, بینښه lhsParts,
                           بینښه rhsParts) {
  // Put the narrower number on the LHS for less loops below.
  إذا (lhsParts > rhsParts)
    أرجع tc_ډک_Multiply (dst, rhs, lhs, rhsParts, lhsParts);

  أكد(dst != lhs && dst != rhs);

  لكل (بینښه i = 0; i < lhsParts; i++) {
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
صحيح APInt::tcDivide(کلمه_Type *lhs, ثابت کلمه_Type *rhs,
                    کلمه_Type *remainder, کلمه_Type *srhs,
                    بینښه parts) {
  أكد(lhs != remainder && lhs != srhs && remainder != srhs);

  بینښه ښويدنه_شمېر = tcMSB(rhs, parts) + 1;
  إذا (ښويدنه_شمېر == 0)
    أرجع ریښتیا;

  ښويدنه_شمېر = parts * APINT_بټونه_فی_کلمه - ښويدنه_شمېر;
  بینښه n = ښويدنه_شمېر / APINT_بټونه_فی_کلمه;
  کلمه_Type ماسک_2 = (کلمه_Type) 1 << (ښويدنه_شمېر % APINT_بټونه_فی_کلمه);

  tc_ټاکل(srhs, rhs, parts);
  tc_ښويدنه_کیڼ(srhs, parts, ښويدنه_شمېر);
  tc_ټاکل(remainder, lhs, parts);
  tc_سېټ(lhs, 0, parts);

  // Loop, subtracting SRHS if REMAINDER is greater and adding that to the
  // total.
  لكل (;;) {
    صحيح compare = tcCompare(remainder, srhs, parts);
    إذا (compare >= 0) {
      tc_کمول(remainder, srhs, 0, parts);
      lhs[n] |= ماسک_2;
    }

    إذا (ښويدنه_شمېر == 0)
      اكسر;
    ښويدنه_شمېر--;
    tc_ښويدنه_ښی(srhs, parts, 1);
    إذا ((ماسک_2 >>= 1) == 0) {
      ماسک_2 = (کلمه_Type) 1 << (APINT_بټونه_فی_کلمه - 1);
      n--;
    }
  }

  أرجع خطأ;
}

/// Shift a bignum left Count bits in-place. Shifted in bits are zero. There are
/// no restrictions on Count.
فراغ APInt::tc_ښويدنه_کیڼ(کلمه_Type *Dst, بینښه ټکي_2, بینښه شمېر_2) {
  // Don't bother performing a no-op shift.
  إذا (!شمېر_2)
    أرجع;

  // WordShift is the inter-part shift; BitShift is the intra-part shift.
  بینښه کلمه_ښويدنه = std::min(شمېر_2 / APINT_بټونه_فی_کلمه, ټکي_2);
  بینښه بټ_ښويدنه = شمېر_2 % APINT_بټونه_فی_کلمه;

  // Fastpath for moving by whole words.
  إذا (بټ_ښويدنه == 0) {
    std::memmove(Dst + کلمه_ښويدنه, Dst, (ټکي_2 - کلمه_ښويدنه) * APINT_کلمه_اندازه);
  } وإلا {
    طالما (ټکي_2-- > کلمه_ښويدنه) {
      Dst[ټکي_2] = Dst[ټکي_2 - کلمه_ښويدنه] << بټ_ښويدنه;
      إذا (ټکي_2 > کلمه_ښويدنه)
        Dst[ټکي_2] |=
          Dst[ټکي_2 - کلمه_ښويدنه - 1] >> (APINT_بټونه_فی_کلمه - بټ_ښويدنه);
    }
  }

  // Fill in the remainder with 0s.
  std::memset(Dst, 0, کلمه_ښويدنه * APINT_کلمه_اندازه);
}

/// Shift a bignum right Count bits in-place. Shifted in bits are zero. There
/// are no restrictions on Count.
فراغ APInt::tc_ښويدنه_ښی(کلمه_Type *Dst, بینښه ټکي_2, بینښه شمېر_2) {
  // Don't bother performing a no-op shift.
  إذا (!شمېر_2)
    أرجع;

  // WordShift is the inter-part shift; BitShift is the intra-part shift.
  بینښه کلمه_ښويدنه = std::min(شمېر_2 / APINT_بټونه_فی_کلمه, ټکي_2);
  بینښه بټ_ښويدنه = شمېر_2 % APINT_بټونه_فی_کلمه;

  بینښه ټکي_منزل_Move = ټکي_2 - کلمه_ښويدنه;
  // Fastpath for moving by whole words.
  إذا (بټ_ښويدنه == 0) {
    std::memmove(Dst, Dst + کلمه_ښويدنه, ټکي_منزل_Move * APINT_کلمه_اندازه);
  } وإلا {
    لكل (بینښه i = 0; i != ټکي_منزل_Move; ++i) {
      Dst[i] = Dst[i + کلمه_ښويدنه] >> بټ_ښويدنه;
      إذا (i + 1 != ټکي_منزل_Move)
        Dst[i] |= Dst[i + کلمه_ښويدنه + 1] << (APINT_بټونه_فی_کلمه - بټ_ښويدنه);
    }
  }

  // Fill in the remainder with 0s.
  std::memset(Dst + ټکي_منزل_Move, 0, کلمه_ښويدنه * APINT_کلمه_اندازه);
}

// Comparison (unsigned) of two bignums.
صحيح APInt::tcCompare(ثابت کلمه_Type *lhs, ثابت کلمه_Type *rhs,
                     بینښه parts) {
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
  حالة APInt::Rounding::ښکته:
  حالة APInt::Rounding::TOWARD_ZERO:
    أرجع A.udiv(B);
  حالة APInt::Rounding::پورته: {
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
  حالة APInt::Rounding::ښکته:
  حالة APInt::Rounding::پورته: {
    APInt Quo, Rem;
    APInt::sdivrem(A, B, Quo, Rem);
    إذا (Rem.isZero())
      أرجع Quo;
    // This algorithm deals with arbitrary rounding mode used by sdivrem.
    // We want to check whether the non-integer part of the mathematical value
    // is negative or not. If the non-integer part is negative, we need to round
    // down from Quo; otherwise, if it's positive or 0, we return Quo, as it's
    // already rounded down.
    إذا (RM == APInt::Rounding::ښکته) {
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
                                           بینښه Range_پلنوالی) {
  بینښه Coeff_پلنوالی = A.get_بټ_پلنوالی();
  أكد(Coeff_پلنوالی == B.get_بټ_پلنوالی() && Coeff_پلنوالی == C.get_بټ_پلنوالی());
  أكد(Range_پلنوالی <= Coeff_پلنوالی &&
         "Value range width should be less than coefficient width");
  أكد(Range_پلنوالی > 1 && "Value range bit width should be > 1");

  LLVM_DEBUG(dbgs() << __func__ << ": solving " << A << "x^2 + " << B
                    << "x + " << C << ", rw:" << Range_پلنوالی << '\n');

  // Identify 0 as a (non)solution immediately.
  إذا (C.sextOrTrunc(Range_پلنوالی).isZero()) {
    LLVM_DEBUG(dbgs() << __func__ << ": zero solution\n");
    أرجع APInt(Coeff_پلنوالی, 0);
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
  Coeff_پلنوالی *= 3;
  A = A.sext(Coeff_پلنوالی);
  B = B.sext(Coeff_پلنوالی);
  C = C.sext(Coeff_پلنوالی);

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
  APInt R = APInt::get_One_بټ_سېټ(Coeff_پلنوالی, Range_پلنوالی);
  APInt TwoA = 2 * A;
  APInt SqrB = B * B;
  منطقي Pick_ټیټ;

  خپلکاری پړاو_پورته_2 = [] (ثابت APInt &V, ثابت APInt &A) -> APInt {
    أكد(A.is_Strictly_مثبت());
    APInt T = V.مطلق_قیمت().urem(A);
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
    Pick_ټیټ = خطأ;
  } وإلا {
    // If B < 0, the vertex is at a positive location. For any solution
    // to exist, the discriminant must be non-negative. This means that
    // C-kR <= B^2/4A is a necessary condition for k, i.e. there is a
    // lower bound on values of k: kR >= C - B^2/4A.
    APInt LowkR = C - SqrB.udiv(2*TwoA); // udiv because all values > 0.
    // Round LowkR up (towards +inf) to the nearest kR.
    LowkR = پړاو_پورته_2(LowkR, R);

    // If there exists k meeting the condition above, and such that
    // C-kR > 0, there will be two positive real number solutions of
    // q(x) = kR. Out of all such values of k, pick the one that makes
    // C-kR closest to 0, (i.e. pick maximum k such that C-kR > 0).
    // In other words, find maximum k such that LowkR <= kR < C.
    إذا (C.sgt(LowkR)) {
      // If LowkR < C, then such a k is guaranteed to exist because
      // LowkR itself is a multiple of R.
      C -= -پړاو_پورته_2(-C, R);      // C = C - RoundDown(C, R)
      // Pick the smaller solution.
      Pick_ټیټ = ریښتیا;
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
      Pick_ټیټ = خطأ;
    }
  }

  LLVM_DEBUG(dbgs() << __func__ << ": updated coefficients " << A << "x^2 + "
                    << B << "x + " << C << ", rw:" << Range_پلنوالی << '\n');

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
  إذا (Pick_ټیټ)
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

std::optional<بینښه>
llvm::APIntOps::Get_Most_Significant_Different_بټ(ثابت APInt &A, ثابت APInt &B) {
  أكد(A.get_بټ_پلنوالی() == B.get_بټ_پلنوالی() && "Must have the same bitwidth");
  إذا (A == B)
    أرجع std::nullopt;
  أرجع A.get_بټ_پلنوالی() - ((A ^ B).countl_zero() + 1);
}

APInt llvm::APIntOps::Scale_بټ_ماسک(ثابت APInt &A, بینښه New_بټ_پلنوالی,
                                   منطقي سمون_All_بټونه) {
  بینښه Old_بټ_پلنوالی = A.get_بټ_پلنوالی();
  أكد((((Old_بټ_پلنوالی % New_بټ_پلنوالی) == 0) ||
          ((New_بټ_پلنوالی % Old_بټ_پلنوالی) == 0)) &&
         "One size should be a multiple of the other one. "
         "Can't do fractional scaling.");

  // Check for matching bitwidths.
  إذا (Old_بټ_پلنوالی == New_بټ_پلنوالی)
    أرجع A;

  APInt NewA = APInt::getZero(New_بټ_پلنوالی);

  // Check for null input.
  إذا (A.isZero())
    أرجع NewA;

  إذا (New_بټ_پلنوالی > Old_بټ_پلنوالی) {
    // Repeat bits.
    بینښه Scale = New_بټ_پلنوالی / Old_بټ_پلنوالی;
    لكل (بینښه i = 0; i != Old_بټ_پلنوالی; ++i)
      إذا (A[i])
        NewA.سېټ_بټونه(i * Scale, (i + 1) * Scale);
  } وإلا {
    بینښه Scale = Old_بټ_پلنوالی / New_بټ_پلنوالی;
    لكل (بینښه i = 0; i != New_بټ_پلنوالی; ++i) {
      إذا (سمون_All_بټونه) {
        إذا (A.ایستل_بټونه(Scale, i * Scale).isAllOnes())
          NewA.سېټ_بټ(i);
      } وإلا {
        إذا (!A.ایستل_بټونه(Scale, i * Scale).isZero())
          NewA.سېټ_بټ(i);
      }
    }
  }

  أرجع NewA;
}

/// StoreIntToMemory - Fills the StoreBytes bytes of memory starting from Dst
/// with the integer held in IntVal.
فراغ llvm::Store_Int_منزل_حافظه(ثابت APInt &IntVal, uint8_t *Dst,
                            بینښه Store_بایټونه) {
  أكد((IntVal.get_بټ_پلنوالی()+7)/8 >= Store_بایټونه && "Integer too small!");
  ثابت uint8_t *Src = (ثابت uint8_t *)IntVal.get_Raw_معلومات();

  إذا (sys::IsLittleEndianHost) {
    // Little-endian host - the source is ordered from LSB to MSB.  Order the
    // destination from LSB to MSB: Do a straight copy.
    حافظه_نقل(Dst, Src, Store_بایټونه);
  } وإلا {
    // Big-endian host - the source is an array of 64 bit words ordered from
    // LSW to MSW.  Each word is ordered from MSB to LSB.  Order the destination
    // from MSB to LSB: Reverse the word order, but not the bytes in a word.
    طالما (Store_بایټونه > حجم(uint64_t)) {
      Store_بایټونه -= حجم(uint64_t);
      // May not be aligned so use memcpy.
      حافظه_نقل(Dst + Store_بایټونه, Src, حجم(uint64_t));
      Src += حجم(uint64_t);
    }

    حافظه_نقل(Dst, Src + حجم(uint64_t) - Store_بایټونه, Store_بایټونه);
  }
}

/// LoadIntFromMemory - Loads the integer stored in the LoadBytes bytes starting
/// from Src into IntVal, which is assumed to be wide enough and to hold zero.
فراغ llvm::بارول_Int_From_حافظه(APInt &IntVal, ثابت uint8_t *Src,
                             بینښه بارول_بایټونه) {
  أكد((IntVal.get_بټ_پلنوالی()+7)/8 >= بارول_بایټونه && "Integer too small!");
  uint8_t *Dst = reinterpret_cast<uint8_t *>(
                   const_cast<uint64_t *>(IntVal.get_Raw_معلومات()));

  إذا (sys::IsLittleEndianHost)
    // Little-endian host - the destination must be ordered from LSB to MSB.
    // The source is ordered from LSB to MSB: Do a straight copy.
    حافظه_نقل(Dst, Src, بارول_بایټونه);
  وإلا {
    // Big-endian - the destination is an array of 64 bit words ordered from
    // LSW to MSW.  Each word must be ordered from MSB to LSB.  The source is
    // ordered from MSB to LSB: Reverse the word order, but not the bytes in
    // a word.
    طالما (بارول_بایټونه > حجم(uint64_t)) {
      بارول_بایټونه -= حجم(uint64_t);
      // May not be aligned so use memcpy.
      حافظه_نقل(Dst, Src + بارول_بایټونه, حجم(uint64_t));
      Dst += حجم(uint64_t);
    }

    حافظه_نقل(Dst + حجم(uint64_t) - بارول_بایټونه, Src, بارول_بایټونه);
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
  أكد(C1.get_بټ_پلنوالی() == C2.get_بټ_پلنوالی() && "Unequal bitwidths");
  بینښه ډک_پلنوالی = C1.get_بټ_پلنوالی() * 2;
  APInt C1Ext = C1.sext(ډک_پلنوالی);
  APInt C2Ext = C2.sext(ډک_پلنوالی);
  أرجع (C1Ext * C2Ext).ایستل_بټونه(C1.get_بټ_پلنوالی(), C1.get_بټ_پلنوالی());
}

APInt APIntOps::mulhu(ثابت APInt &C1, ثابت APInt &C2) {
  أكد(C1.get_بټ_پلنوالی() == C2.get_بټ_پلنوالی() && "Unequal bitwidths");
  بینښه ډک_پلنوالی = C1.get_بټ_پلنوالی() * 2;
  APInt C1Ext = C1.zext(ډک_پلنوالی);
  APInt C2Ext = C2.zext(ډک_پلنوالی);
  أرجع (C1Ext * C2Ext).ایستل_بټونه(C1.get_بټ_پلنوالی(), C1.get_بټ_پلنوالی());
}

APInt APIntOps::mulsExtended(ثابت APInt &C1, ثابت APInt &C2) {
  أكد(C1.get_بټ_پلنوالی() == C2.get_بټ_پلنوالی() && "Unequal bitwidths");
  بینښه ډک_پلنوالی = C1.get_بټ_پلنوالی() * 2;
  APInt C1Ext = C1.sext(ډک_پلنوالی);
  APInt C2Ext = C2.sext(ډک_پلنوالی);
  أرجع C1Ext * C2Ext;
}

APInt APIntOps::muluExtended(ثابت APInt &C1, ثابت APInt &C2) {
  أكد(C1.get_بټ_پلنوالی() == C2.get_بټ_پلنوالی() && "Unequal bitwidths");
  بینښه ډک_پلنوالی = C1.get_بټ_پلنوالی() * 2;
  APInt C1Ext = C1.zext(ډک_پلنوالی);
  APInt C2Ext = C2.zext(ډک_پلنوالی);
  أرجع C1Ext * C2Ext;
}

APInt APIntOps::توان(ثابت APInt &X, int64_t N) {
  أكد(N >= 0 && "negative exponents not supported.");
  APInt Acc = APInt(X.get_بټ_پلنوالی(), 1);
  إذا (N == 0)
    أرجع Acc;
  APInt بنسټ = X;
  int64_t پاتې_شوی_Exponent = N;
  طالما (پاتې_شوی_Exponent > 0) {
    طالما (پاتې_شوی_Exponent % 2 == 0) {
      بنسټ *= بنسټ;
      پاتې_شوی_Exponent /= 2;
    }
    --پاتې_شوی_Exponent;
    Acc *= بنسټ;
  }
  أرجع Acc;
}

APInt llvm::APIntOps::fshl(ثابت APInt &Hi, ثابت APInt &Lo,
                           ثابت APInt &ښويدنه_2) {
  أكد(Hi.get_بټ_پلنوالی() == Lo.get_بټ_پلنوالی());
  بینښه ښويدنه_Amt_2 = rotateModulo(Hi.get_بټ_پلنوالی(), ښويدنه_2);
  إذا (ښويدنه_Amt_2 == 0)
    أرجع Hi;
  أرجع Hi.shl(ښويدنه_Amt_2) | Lo.lshr(Hi.get_بټ_پلنوالی() - ښويدنه_Amt_2);
}

APInt llvm::APIntOps::fshr(ثابت APInt &Hi, ثابت APInt &Lo,
                           ثابت APInt &ښويدنه_2) {
  أكد(Hi.get_بټ_پلنوالی() == Lo.get_بټ_پلنوالی());
  بینښه ښويدنه_Amt_2 = rotateModulo(Hi.get_بټ_پلنوالی(), ښويدنه_2);
  إذا (ښويدنه_Amt_2 == 0)
    أرجع Lo;
  أرجع Hi.shl(Hi.get_بټ_پلنوالی() - ښويدنه_Amt_2) | Lo.lshr(ښويدنه_Amt_2);
}

APInt llvm::APIntOps::clmul(ثابت APInt &LHS, ثابت APInt &RHS) {
  بینښه BW = LHS.get_بټ_پلنوالی();
  أكد(BW == RHS.get_بټ_پلنوالی() && "Operand mismatch");
  APInt پایله(BW, 0);
  لكل (بینښه I : seq(std::min(RHS.get_فعال_بټونه(), BW - LHS.countr_zero())))
    إذا (RHS[I])
      پایله ^= LHS << I;
  أرجع پایله;
}

APInt llvm::APIntOps::clmulr(ثابت APInt &LHS, ثابت APInt &RHS) {
  أكد(LHS.get_بټ_پلنوالی() == RHS.get_بټ_پلنوالی());
  أرجع clmul(LHS.سرچپه_بټونه(), RHS.سرچپه_بټونه()).سرچپه_بټونه();
}

APInt llvm::APIntOps::clmulh(ثابت APInt &LHS, ثابت APInt &RHS) {
  أكد(LHS.get_بټ_پلنوالی() == RHS.get_بټ_پلنوالی());
  أرجع clmulr(LHS, RHS).lshr(1);
}

APInt llvm::APIntOps::pext(ثابت APInt &Val, ثابت APInt &ماسک) {
  بینښه BW = Val.get_بټ_پلنوالی();
  أكد(BW == ماسک.get_بټ_پلنوالی() && "Operand mismatch");
  APInt پایله = APInt::getZero(BW);
  لكل (بینښه I = 0, P = 0; I != BW; ++I)
    إذا (ماسک[I])
      پایله.سېټ_بټ_Val(P++, Val[I]);
  أرجع پایله;
}

APInt llvm::APIntOps::pdep(ثابت APInt &Val, ثابت APInt &ماسک) {
  بینښه BW = Val.get_بټ_پلنوالی();
  أكد(BW == ماسک.get_بټ_پلنوالی() && "Operand mismatch");
  APInt پایله = APInt::getZero(BW);
  لكل (بینښه I = 0, P = 0; I != BW; ++I)
    إذا (ماسک[I])
      پایله.سېټ_بټ_Val(I, Val[P++]);
  أرجع پایله;
}
