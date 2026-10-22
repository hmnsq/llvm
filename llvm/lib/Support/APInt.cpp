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

استفاده نطاق llvm;

#تعریف_کن DEBUG_TYPE "apint"

/// A utility function for allocating memory, checking for allocation failures,
/// and ensuring the contents are zeroed.
مضمن ساكن uint64_t* get_Cleared_حافظه(بیعلامت رقم_واژگان) {
  أرجع جديد uint64_t[رقم_واژگان]();
}

/// A utility function for allocating memory and checking for allocation
/// failure.  The content is not zeroed.
مضمن ساكن uint64_t* get_حافظه(بیعلامت رقم_واژگان) {
  أرجع جديد uint64_t[رقم_واژگان];
}

/// A utility function that converts a character to a digit.
مضمن ساكن بیعلامت getDigit(حرف cdigit, uint8_t radix) {
  بیعلامت r;

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
    U.pVal = get_حافظه(get_رقم_واژگان());
    U.pVal[0] = val;
    memset(&U.pVal[1], 0xFF, APINT_کلمه_اندازه * (get_رقم_واژگان() - 1));
    clear_استفادهنشده_بیتها();
  } وإلا {
    U.pVal = get_Cleared_حافظه(get_رقم_واژگان());
    U.pVal[0] = val;
  }
}

فراغ APInt::initSlowCase(ثابت APInt& that) {
  U.pVal = get_حافظه(get_رقم_واژگان());
  کپی_حافظه(U.pVal, that.U.pVal, get_رقم_واژگان() * APINT_کلمه_اندازه);
}

فراغ APInt::initFromArray(ArrayRef<uint64_t> bigVal) {
  أكد(bigVal.داده() && "Null pointer detected!");
  إذا (LLVM_LIKELY(is_Single_کلمه()))
    U.VAL = bigVal[0];
  وإلا {
    // Get memory, cleared to 0
    U.pVal = get_Cleared_حافظه(get_رقم_واژگان());
    // Calculate the number of words to copy
    بیعلامت واژگان = std::min<بیعلامت>(bigVal.اندازه_2(), get_رقم_واژگان());
    // Copy the words from bigVal to pVal
    کپی_حافظه(U.pVal, bigVal.داده(), واژگان * APINT_کلمه_اندازه);
  }
  // Make sure unused high bits are cleared
  clear_استفادهنشده_بیتها();
}

APInt::APInt(بیعلامت رقم_بیتها, ArrayRef<uint64_t> bigVal) : بیت_عرض_2(رقم_بیتها) {
  initFromArray(bigVal);
}

APInt::APInt(بیعلامت numbits, StringRef Str, uint8_t radix)
    : بیت_عرض_2(numbits) {
  fromString(numbits, Str, radix);
}

فراغ APInt::reallocate(بیعلامت New_بیت_عرض) {
  // If the number of words is the same we can just change the width and stop.
  إذا (get_رقم_واژگان() == get_رقم_واژگان(New_بیت_عرض)) {
    بیت_عرض_2 = New_بیت_عرض;
    أرجع;
  }

  // If we have an allocation, delete it.
  إذا (LLVM_UNLIKELY(!is_Single_کلمه()))
    حذف [] U.pVal;

  // Update BitWidth.
  بیت_عرض_2 = New_بیت_عرض;

  // If we are supposed to have an allocation, create it.
  إذا (LLVM_UNLIKELY(!is_Single_کلمه()))
    U.pVal = get_حافظه(get_رقم_واژگان());
}

فراغ APInt::تخصیص_بده_Slow_Case(ثابت APInt &RHS) {
  // Don't do anything for X = X
  إذا (هذا == &RHS)
    أرجع;

  // Adjust the bit width and handle allocations as necessary.
  reallocate(RHS.get_بیت_عرض());

  // Copy the data.
  إذا (LLVM_LIKELY(is_Single_کلمه()))
    U.VAL = RHS.U.VAL;
  وإلا
    کپی_حافظه(U.pVal, RHS.U.pVal, get_رقم_واژگان() * APINT_کلمه_اندازه);
}

/// This method 'profiles' an APInt for use with FoldingSet.
فراغ APInt::Profile(Folding_مجموعه_گره_ID& ID) ثابت {
  ID.جمع_کن_Integer(بیت_عرض_2);

  إذا (LLVM_LIKELY(is_Single_کلمه())) {
    ID.جمع_کن_Integer(U.VAL);
    أرجع;
  }

  بیعلامت رقم_واژگان_2 = get_رقم_واژگان();
  لكل (بیعلامت i = 0; i < رقم_واژگان_2; ++i)
    ID.جمع_کن_Integer(U.pVal[i]);
}

منطقي APInt::isAligned(Align A) ثابت {
  إذا (isZero())
    أرجع درست;
  ثابت بیعلامت TrailingZeroes = countr_zero();
  ثابت بیعلامت MinimumTrailingZeroes = Log2(A);
  أرجع TrailingZeroes >= MinimumTrailingZeroes;
}

/// Prefix increment operator. Increments the APInt by one.
APInt& APInt::عملگر++() {
  إذا (LLVM_LIKELY(is_Single_کلمه()))
    ++U.VAL;
  وإلا
    tcIncrement(U.pVal, get_رقم_واژگان());
  أرجع clear_استفادهنشده_بیتها();
}

/// Prefix decrement operator. Decrements the APInt by one.
APInt& APInt::عملگر--() {
  إذا (LLVM_LIKELY(is_Single_کلمه()))
    --U.VAL;
  وإلا
    tcDecrement(U.pVal, get_رقم_واژگان());
  أرجع clear_استفادهنشده_بیتها();
}

/// Adds the RHS APInt to this APInt.
/// @returns this, after addition of RHS.
/// Addition assignment operator.
APInt& APInt::عملگر+=(ثابت APInt& RHS) {
  أكد(بیت_عرض_2 == RHS.بیت_عرض_2 && "Bit widths must be the same");
  إذا (LLVM_LIKELY(is_Single_کلمه()))
    U.VAL += RHS.U.VAL;
  وإلا
    tc_جمع_کن(U.pVal, RHS.U.pVal, 0, get_رقم_واژگان());
  أرجع clear_استفادهنشده_بیتها();
}

APInt& APInt::عملگر+=(uint64_t RHS) {
  إذا (LLVM_LIKELY(is_Single_کلمه()))
    U.VAL += RHS;
  وإلا
    tc_جمع_کن_Part(U.pVal, RHS, get_رقم_واژگان());
  أرجع clear_استفادهنشده_بیتها();
}

/// Subtracts the RHS APInt from this APInt
/// @returns this, after subtraction
/// Subtraction assignment operator.
APInt& APInt::عملگر-=(ثابت APInt& RHS) {
  أكد(بیت_عرض_2 == RHS.بیت_عرض_2 && "Bit widths must be the same");
  إذا (LLVM_LIKELY(is_Single_کلمه()))
    U.VAL -= RHS.U.VAL;
  وإلا
    tc_تفریق(U.pVal, RHS.U.pVal, 0, get_رقم_واژگان());
  أرجع clear_استفادهنشده_بیتها();
}

APInt& APInt::عملگر-=(uint64_t RHS) {
  إذا (LLVM_LIKELY(is_Single_کلمه()))
    U.VAL -= RHS;
  وإلا
    tc_تفریق_Part(U.pVal, RHS, get_رقم_واژگان());
  أرجع clear_استفادهنشده_بیتها();
}

APInt APInt::عملگر*(ثابت APInt& RHS) ثابت {
  أكد(بیت_عرض_2 == RHS.بیت_عرض_2 && "Bit widths must be the same");
  إذا (LLVM_LIKELY(is_Single_کلمه()))
    أرجع APInt(بیت_عرض_2, U.VAL * RHS.U.VAL, /*isSigned=*/خطأ,
                 /*implicitTrunc=*/درست);

  APInt نتیجه(get_حافظه(get_رقم_واژگان()), get_بیت_عرض());
  tcMultiply(نتیجه.U.pVal, U.pVal, RHS.U.pVal, get_رقم_واژگان());
  نتیجه.clear_استفادهنشده_بیتها();
  أرجع نتیجه;
}

فراغ APInt::and_تخصیص_بده_Slow_Case(ثابت APInt &RHS) {
  کلمه_Type *dst = U.pVal, *rhs = RHS.U.pVal;
  لكل (size_t i = 0, e = get_رقم_واژگان(); i != e; ++i)
    dst[i] &= rhs[i];
}

فراغ APInt::or_تخصیص_بده_Slow_Case(ثابت APInt &RHS) {
  کلمه_Type *dst = U.pVal, *rhs = RHS.U.pVal;
  لكل (size_t i = 0, e = get_رقم_واژگان(); i != e; ++i)
    dst[i] |= rhs[i];
}

فراغ APInt::xor_تخصیص_بده_Slow_Case(ثابت APInt &RHS) {
  کلمه_Type *dst = U.pVal, *rhs = RHS.U.pVal;
  لكل (size_t i = 0, e = get_رقم_واژگان(); i != e; ++i)
    dst[i] ^= rhs[i];
}

APInt &APInt::عملگر*=(ثابت APInt &RHS) {
  *هذا = *هذا * RHS;
  أرجع *هذا;
}

APInt& APInt::عملگر*=(uint64_t RHS) {
  إذا (LLVM_LIKELY(is_Single_کلمه())) {
    U.VAL *= RHS;
  } وإلا {
    بیعلامت رقم_واژگان_2 = get_رقم_واژگان();
    tcMultiplyPart(U.pVal, U.pVal, RHS, 0, رقم_واژگان_2, رقم_واژگان_2, خطأ);
  }
  أرجع clear_استفادهنشده_بیتها();
}

منطقي APInt::equalSlowCase(ثابت APInt &RHS) ثابت {
  أرجع std::equal(U.pVal, U.pVal + get_رقم_واژگان(), RHS.U.pVal);
}

صحيح APInt::compare(ثابت APInt& RHS) ثابت {
  أكد(بیت_عرض_2 == RHS.بیت_عرض_2 && "Bit widths must be same for comparison");
  إذا (LLVM_LIKELY(is_Single_کلمه()))
    أرجع U.VAL < RHS.U.VAL ? -1 : U.VAL > RHS.U.VAL;

  أرجع tcCompare(U.pVal, RHS.U.pVal, get_رقم_واژگان());
}

صحيح APInt::compareSigned(ثابت APInt& RHS) ثابت {
  أكد(بیت_عرض_2 == RHS.بیت_عرض_2 && "Bit widths must be same for comparison");
  إذا (LLVM_LIKELY(is_Single_کلمه())) {
    int64_t lhsSext = SignExtend64(U.VAL, بیت_عرض_2);
    int64_t rhsSext = SignExtend64(RHS.U.VAL, بیت_عرض_2);
    أرجع lhsSext < rhsSext ? -1 : lhsSext > rhsSext;
  }

  منطقي lhsNeg = isNegative();
  منطقي rhsNeg = RHS.isNegative();

  // If the sign bits don't match, then (LHS < RHS) if LHS is negative
  إذا (lhsNeg != rhsNeg)
    أرجع lhsNeg ? -1 : 1;

  // Otherwise we can just use an unsigned comparison, because even negative
  // numbers compare correctly this way if both have the same signed-ness.
  أرجع tcCompare(U.pVal, RHS.U.pVal, get_رقم_واژگان());
}

فراغ APInt::مجموعه_بیتها_Slow_Case(بیعلامت lo_بیت, بیعلامت hi_بیت) {
  بیعلامت lo_کلمه = which_کلمه(lo_بیت);
  بیعلامت hi_کلمه = which_کلمه(hi_بیت);

  // Create an initial mask for the low word with zeros below loBit.
  uint64_t lo_ماسک = WORDTYPE_MAX << which_بیت(lo_بیت);

  // If hiBit is not aligned, we need a high mask.
  بیعلامت hi_لغزش_Amt = which_بیت(hi_بیت);
  إذا (hi_لغزش_Amt != 0) {
    // Create a high mask with zeros above hiBit.
    uint64_t hi_ماسک = WORDTYPE_MAX >> (APINT_بیتها_در_هر_کلمه - hi_لغزش_Amt);
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
  لكل (بیعلامت کلمه = lo_کلمه + 1; کلمه < hi_کلمه; ++کلمه)
    U.pVal[کلمه] = WORDTYPE_MAX;
}

فراغ APInt::clear_بیتها_Slow_Case(بیعلامت Lo_بیت, بیعلامت Hi_بیت) {
  بیعلامت Lo_کلمه = which_کلمه(Lo_بیت);
  بیعلامت Hi_کلمه = which_کلمه(Hi_بیت);

  // Create an initial mask for the low word with ones below loBit.
  uint64_t Lo_ماسک = ~(WORDTYPE_MAX << which_بیت(Lo_بیت));

  // If HiBit is not aligned, we need a high mask.
  بیعلامت Hi_لغزش_Amt = which_بیت(Hi_بیت);
  إذا (Hi_لغزش_Amt != 0) {
    // Create a high mask with ones above HiBit.
    uint64_t Hi_ماسک = ~(WORDTYPE_MAX >> (APINT_بیتها_در_هر_کلمه - Hi_لغزش_Amt));
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
  لكل (بیعلامت کلمه_2 = Lo_کلمه + 1; کلمه_2 < Hi_کلمه; ++کلمه_2)
    U.pVal[کلمه_2] = 0;
}

// Complement a bignum in-place.
ساكن فراغ tcComplement(APInt::کلمه_Type *dst, بیعلامت parts) {
  لكل (بیعلامت i = 0; i < parts; i++)
    dst[i] = ~dst[i];
}

/// Toggle every bit to its opposite value.
فراغ APInt::flip_All_بیتها_Slow_Case() {
  tcComplement(U.pVal, get_رقم_واژگان());
  clear_استفادهنشده_بیتها();
}

/// Concatenate the bits from "NewLSB" onto the bottom of *this.  This is
/// equivalent to:
///   (this->zext(NewWidth) << NewLSB.getBitWidth()) | NewLSB.zext(NewWidth)
/// In the slow case, we know the result is large.
APInt APInt::concatSlowCase(ثابت APInt &NewLSB) ثابت {
  بیعلامت New_عرض = get_بیت_عرض() + NewLSB.get_بیت_عرض();
  APInt نتیجه = NewLSB.zext(New_عرض);
  نتیجه.درج_بیتها(*هذا, NewLSB.get_بیت_عرض());
  أرجع نتیجه;
}

/// Toggle a given bit to its opposite value whose position is given
/// as "bitPosition".
/// Toggles a given bit to its opposite value.
فراغ APInt::flip_بیت(بیعلامت بیت_Position) {
  أكد(بیت_Position < بیت_عرض_2 && "Out of the bit-width range!");
  مجموعه_بیت_Val(بیت_Position, !(*هذا)[بیت_Position]);
}

فراغ APInt::درج_بیتها(ثابت APInt &sub_بیتها, بیعلامت بیت_Position) {
  بیعلامت sub_بیت_عرض = sub_بیتها.get_بیت_عرض();
  أكد((sub_بیت_عرض + بیت_Position) <= بیت_عرض_2 && "Illegal bit insertion");

  // inserting no bits is a noop.
  إذا (sub_بیت_عرض == 0)
    أرجع;

  // Insertion is a direct copy.
  إذا (sub_بیت_عرض == بیت_عرض_2) {
    *هذا = sub_بیتها;
    أرجع;
  }

  // Single word result can be done as a direct bitmask.
  إذا (LLVM_LIKELY(is_Single_کلمه())) {
    uint64_t ماسک_2 = WORDTYPE_MAX >> (APINT_بیتها_در_هر_کلمه - sub_بیت_عرض);
    U.VAL &= ~(ماسک_2 << بیت_Position);
    U.VAL |= (sub_بیتها.U.VAL << بیت_Position);
    أرجع;
  }

  بیعلامت lo_بیت = which_بیت(بیت_Position);
  بیعلامت lo_کلمه = which_کلمه(بیت_Position);
  بیعلامت hi1_کلمه = which_کلمه(بیت_Position + sub_بیت_عرض - 1);

  // Insertion within a single word can be done as a direct bitmask.
  إذا (lo_کلمه == hi1_کلمه) {
    uint64_t ماسک_2 = WORDTYPE_MAX >> (APINT_بیتها_در_هر_کلمه - sub_بیت_عرض);
    U.pVal[lo_کلمه] &= ~(ماسک_2 << lo_بیت);
    U.pVal[lo_کلمه] |= (sub_بیتها.U.VAL << lo_بیت);
    أرجع;
  }

  // Insert on word boundaries.
  إذا (lo_بیت == 0) {
    // Direct copy whole words.
    بیعلامت رقم_کامل_Sub_واژگان = sub_بیت_عرض / APINT_بیتها_در_هر_کلمه;
    کپی_حافظه(U.pVal + lo_کلمه, sub_بیتها.get_Raw_داده(),
           رقم_کامل_Sub_واژگان * APINT_کلمه_اندازه);

    // Mask+insert remaining bits.
    بیعلامت مانده_بیتها = sub_بیت_عرض % APINT_بیتها_در_هر_کلمه;
    إذا (مانده_بیتها != 0) {
      uint64_t ماسک_2 = WORDTYPE_MAX >> (APINT_بیتها_در_هر_کلمه - مانده_بیتها);
      U.pVal[hi1_کلمه] &= ~ماسک_2;
      U.pVal[hi1_کلمه] |= sub_بیتها.get_کلمه(sub_بیت_عرض - 1);
    }
    أرجع;
  }

  // General case - set/clear individual bits in dst based on src.
  // TODO - there is scope for optimization here, but at the moment this code
  // path is barely used so prefer readability over performance.
  لكل (بیعلامت i = 0; i != sub_بیت_عرض; ++i)
    مجموعه_بیت_Val(بیت_Position + i, sub_بیتها[i]);
}

فراغ APInt::درج_بیتها(uint64_t sub_بیتها, بیعلامت بیت_Position, بیعلامت رقم_بیتها) {
  uint64_t ماسک_بیتها = ماسک_Trailing_Ones<uint64_t>(رقم_بیتها);
  sub_بیتها &= ماسک_بیتها;
  إذا (LLVM_LIKELY(is_Single_کلمه())) {
    U.VAL &= ~(ماسک_بیتها << بیت_Position);
    U.VAL |= sub_بیتها << بیت_Position;
    أرجع;
  }

  بیعلامت lo_بیت = which_بیت(بیت_Position);
  بیعلامت lo_کلمه = which_کلمه(بیت_Position);
  بیعلامت hi_کلمه = which_کلمه(بیت_Position + رقم_بیتها - 1);
  إذا (lo_کلمه == hi_کلمه) {
    U.pVal[lo_کلمه] &= ~(ماسک_بیتها << lo_بیت);
    U.pVal[lo_کلمه] |= sub_بیتها << lo_بیت;
    أرجع;
  }

  static_assert(8 * حجم(کلمه_Type) <= 64, "This code assumes only two words affected");
  بیعلامت کلمه_بیتها = 8 * حجم(کلمه_Type);
  U.pVal[lo_کلمه] &= ~(ماسک_بیتها << lo_بیت);
  U.pVal[lo_کلمه] |= sub_بیتها << lo_بیت;

  U.pVal[hi_کلمه] &= ~(ماسک_بیتها >> (کلمه_بیتها - lo_بیت));
  U.pVal[hi_کلمه] |= sub_بیتها >> (کلمه_بیتها - lo_بیت);
}

APInt APInt::استخراج_بیتها(بیعلامت رقم_بیتها, بیعلامت بیت_Position) ثابت {
  أكد(بیت_Position < بیت_عرض_2 && (رقم_بیتها + بیت_Position) <= بیت_عرض_2 &&
         "Illegal bit extraction");

  إذا (LLVM_LIKELY(is_Single_کلمه()))
    أرجع APInt(رقم_بیتها, U.VAL >> بیت_Position, /*isSigned=*/خطأ,
                 /*implicitTrunc=*/درست);

  بیعلامت lo_بیت = which_بیت(بیت_Position);
  بیعلامت lo_کلمه = which_کلمه(بیت_Position);
  بیعلامت hi_کلمه = which_کلمه(بیت_Position + رقم_بیتها - 1);

  // Single word result extracting bits from a single word source.
  إذا (lo_کلمه == hi_کلمه)
    أرجع APInt(رقم_بیتها, U.pVal[lo_کلمه] >> lo_بیت, /*isSigned=*/خطأ,
                 /*implicitTrunc=*/درست);

  // Extracting bits that start on a source word boundary can be done
  // as a fast memory copy.
  إذا (lo_بیت == 0)
    أرجع APInt(رقم_بیتها, ArrayRef(U.pVal + lo_کلمه, 1 + hi_کلمه - lo_کلمه));

  // General case - shift + copy source words directly into place.
  APInt نتیجه(رقم_بیتها, 0);
  بیعلامت رقم_Src_واژگان = get_رقم_واژگان();
  بیعلامت رقم_Dst_واژگان = نتیجه.get_رقم_واژگان();

  uint64_t *DestPtr =
      LLVM_LIKELY(نتیجه.is_Single_کلمه()) ? &نتیجه.U.VAL : نتیجه.U.pVal;
  لكل (بیعلامت کلمه = 0; کلمه < رقم_Dst_واژگان; ++کلمه) {
    uint64_t w0 = U.pVal[lo_کلمه + کلمه];
    uint64_t w1 =
        (lo_کلمه + کلمه + 1) < رقم_Src_واژگان ? U.pVal[lo_کلمه + کلمه + 1] : 0;
    DestPtr[کلمه] = (w0 >> lo_بیت) | (w1 << (APINT_بیتها_در_هر_کلمه - lo_بیت));
  }

  أرجع نتیجه.clear_استفادهنشده_بیتها();
}

uint64_t APInt::استخراج_بیتها_As_ZExt_مقدار(بیعلامت رقم_بیتها,
                                       بیعلامت بیت_Position) ثابت {
  أكد(بیت_Position < بیت_عرض_2 && (رقم_بیتها + بیت_Position) <= بیت_عرض_2 &&
         "Illegal bit extraction");
  أكد(رقم_بیتها <= 64 && "Illegal bit extraction");

  uint64_t ماسک_بیتها = ماسک_Trailing_Ones<uint64_t>(رقم_بیتها);
  إذا (LLVM_LIKELY(is_Single_کلمه()))
    أرجع (U.VAL >> بیت_Position) & ماسک_بیتها;

  static_assert(APINT_بیتها_در_هر_کلمه >= 64,
                "This code assumes only two words affected");
  بیعلامت lo_بیت = which_بیت(بیت_Position);
  بیعلامت lo_کلمه = which_کلمه(بیت_Position);
  بیعلامت hi_کلمه = which_کلمه(بیت_Position + رقم_بیتها - 1);
  إذا (lo_کلمه == hi_کلمه)
    أرجع (U.pVal[lo_کلمه] >> lo_بیت) & ماسک_بیتها;

  uint64_t ret_بیتها = U.pVal[lo_کلمه] >> lo_بیت;
  ret_بیتها |= U.pVal[hi_کلمه] << (APINT_بیتها_در_هر_کلمه - lo_بیت);
  ret_بیتها &= ماسک_بیتها;
  أرجع ret_بیتها;
}

بیعلامت APInt::get_Sufficient_بیتها_Needed(StringRef Str, uint8_t Radix) {
  أكد(!Str.خالی() && "Invalid string length");
  size_t StrLen = Str.اندازه_2();

  // Each computation below needs to know if it's negative.
  بیعلامت IsNegative = خطأ;
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

بیعلامت APInt::get_بیتها_Needed(StringRef str, uint8_t radix) {
  // Compute a sufficient number of bits that is always large enough but might
  // be too large.
  بیعلامت sufficient = get_Sufficient_بیتها_Needed(str, radix);

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
  بیعلامت isNegative = *p == '-';
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
  بیعلامت لگاریتم = tmp.logBase2();
  إذا (لگاریتم == (بیعلامت)-1) {
    أرجع isNegative + 1;
  } وإلا إذا (isNegative && tmp.isPowerOf2()) {
    أرجع isNegative + لگاریتم;
  } وإلا {
    أرجع isNegative + لگاریتم + 1;
  }
}

hash_کد llvm::hash_مقدار(ثابت APInt &Arg) {
  إذا (LLVM_LIKELY(Arg.is_Single_کلمه()))
    أرجع hash_combine(Arg.بیت_عرض_2, Arg.U.VAL);

  أرجع hash_combine(
      Arg.بیت_عرض_2,
      hash_combine_range(Arg.U.pVal, Arg.U.pVal + Arg.get_رقم_واژگان()));
}

بیعلامت DenseMapInfo<APInt, فراغ>::get_Hash_مقدار(ثابت APInt &کلید) {
  أرجع static_cast<بیعلامت>(hash_مقدار(کلید));
}

منطقي APInt::isSplat(بیعلامت Splat_اندازه_درون_بیتها) ثابت {
  أكد(get_بیت_عرض() % Splat_اندازه_درون_بیتها == 0 &&
         "SplatSizeInBits must divide width!");
  // We can check that all parts of an integer are equal by making use of a
  // little trick: rotate and check if it's still the same value.
  أرجع *هذا == rotl(Splat_اندازه_درون_بیتها);
}

/// This function returns the high "numBits" bits of this APInt.
APInt APInt::get_Hi_بیتها(بیعلامت رقم_بیتها) ثابت {
  أرجع هذا->lshr(بیت_عرض_2 - رقم_بیتها);
}

/// This function returns the low "numBits" bits of this APInt.
APInt APInt::get_Lo_بیتها(بیعلامت رقم_بیتها) ثابت {
  APInt نتیجه(get_کم_بیتها_مجموعه(بیت_عرض_2, رقم_بیتها));
  نتیجه &= *هذا;
  أرجع نتیجه;
}

/// Return a value containing V broadcasted over NewLen bits.
APInt APInt::getSplat(بیعلامت NewLen, ثابت APInt &V) {
  أكد(NewLen >= V.get_بیت_عرض() && "Can't splat to smaller bit width!");

  APInt Val = V.zext(NewLen);
  لكل (بیعلامت I = V.get_بیت_عرض(); I < NewLen; I <<= 1)
    Val |= Val << I;

  أرجع Val;
}

بیعلامت APInt::شمار_Leading_Zeros_Slow_Case() ثابت {
  بیعلامت شمار_2 = 0;
  لكل (صحيح i = get_رقم_واژگان() - 1; i >= 0; --i) {
    uint64_t V = U.pVal[i];
    إذا (V == 0)
      شمار_2 += APINT_بیتها_در_هر_کلمه;
    وإلا {
      شمار_2 += llvm::countl_zero(V);
      اكسر;
    }
  }
  // Adjust for unused bits in the most significant word (they are zero).
  بیعلامت Mod = بیت_عرض_2 % APINT_بیتها_در_هر_کلمه;
  شمار_2 -= Mod > 0 ? APINT_بیتها_در_هر_کلمه - Mod : 0;
  أرجع شمار_2;
}

بیعلامت APInt::شمار_Leading_Ones_Slow_Case() ثابت {
  بیعلامت بالا_کلمه_بیتها = بیت_عرض_2 % APINT_بیتها_در_هر_کلمه;
  بیعلامت لغزش;
  إذا (!بالا_کلمه_بیتها) {
    بالا_کلمه_بیتها = APINT_بیتها_در_هر_کلمه;
    لغزش = 0;
  } وإلا {
    لغزش = APINT_بیتها_در_هر_کلمه - بالا_کلمه_بیتها;
  }
  صحيح i = get_رقم_واژگان() - 1;
  بیعلامت شمار_2 = llvm::countl_one(U.pVal[i] << لغزش);
  إذا (شمار_2 == بالا_کلمه_بیتها) {
    لكل (i--; i >= 0; --i) {
      إذا (U.pVal[i] == WORDTYPE_MAX)
        شمار_2 += APINT_بیتها_در_هر_کلمه;
      وإلا {
        شمار_2 += llvm::countl_one(U.pVal[i]);
        اكسر;
      }
    }
  }
  أرجع شمار_2;
}

بیعلامت APInt::شمار_Trailing_Zeros_Slow_Case() ثابت {
  بیعلامت شمار_2 = 0;
  بیعلامت i = 0;
  لكل (; i < get_رقم_واژگان() && U.pVal[i] == 0; ++i)
    شمار_2 += APINT_بیتها_در_هر_کلمه;
  إذا (i < get_رقم_واژگان())
    شمار_2 += llvm::countr_zero(U.pVal[i]);
  أرجع std::min(شمار_2, بیت_عرض_2);
}

بیعلامت APInt::شمار_Trailing_Ones_Slow_Case() ثابت {
  بیعلامت شمار_2 = 0;
  بیعلامت i = 0;
  لكل (; i < get_رقم_واژگان() && U.pVal[i] == WORDTYPE_MAX; ++i)
    شمار_2 += APINT_بیتها_در_هر_کلمه;
  إذا (i < get_رقم_واژگان())
    شمار_2 += llvm::countr_one(U.pVal[i]);
  أكد(شمار_2 <= بیت_عرض_2);
  أرجع شمار_2;
}

بیعلامت APInt::شمار_Population_Slow_Case() ثابت {
  بیعلامت شمار_2 = 0;
  لكل (بیعلامت i = 0; i < get_رقم_واژگان(); ++i)
    شمار_2 += llvm::popcount(U.pVal[i]);
  أرجع شمار_2;
}

منطقي APInt::isPowerOf2SlowCase() ثابت {
  بیعلامت شمار_2 = 0;
  لكل (بیعلامت i = 0; i < get_رقم_واژگان(); ++i) {
    شمار_2 += llvm::popcount(U.pVal[i]);
    إذا (شمار_2 > 1)
      أرجع خطأ;
  }
  أرجع شمار_2 == 1;
}

منطقي APInt::intersectsSlowCase(ثابت APInt &RHS) ثابت {
  لكل (بیعلامت i = 0, e = get_رقم_واژگان(); i != e; ++i)
    إذا ((U.pVal[i] & RHS.U.pVal[i]) != 0)
      أرجع درست;

  أرجع خطأ;
}

منطقي APInt::is_Subset_از_Slow_Case(ثابت APInt &RHS) ثابت {
  لكل (بیعلامت i = 0, e = get_رقم_واژگان(); i != e; ++i)
    إذا ((U.pVal[i] & ~RHS.U.pVal[i]) != 0)
      أرجع خطأ;

  أرجع درست;
}

منطقي APInt::is_Inverse_از_Slow_Case(ثابت APInt &RHS) ثابت {
  ثابت بیعلامت آخرین = get_رقم_واژگان() - 1;
  لكل (بیعلامت I = 0; I != آخرین; ++I)
    إذا ((U.pVal[I] ^ RHS.U.pVal[I]) != WORDTYPE_MAX)
      أرجع خطأ;

  بیعلامت دم_بیتها = بیت_عرض_2 - آخرین * APINT_بیتها_در_هر_کلمه;
  کلمه_Type دم_ماسک = llvm::ماسک_Trailing_Ones<کلمه_Type>(دم_بیتها);
  أرجع (U.pVal[آخرین] ^ RHS.U.pVal[آخرین]) == دم_ماسک;
}

APInt APInt::byte_جابجا() ثابت {
  أكد(بیت_عرض_2 >= 16 && بیت_عرض_2 % 8 == 0 && "Cannot byteswap!");
  إذا (بیت_عرض_2 == 16)
    أرجع APInt(بیت_عرض_2, llvm::byteswap<uint16_t>(U.VAL));
  إذا (بیت_عرض_2 == 32)
    أرجع APInt(بیت_عرض_2, llvm::byteswap<uint32_t>(U.VAL));
  إذا (بیت_عرض_2 <= 64) {
    uint64_t Tmp1 = llvm::byteswap<uint64_t>(U.VAL);
    Tmp1 >>= (64 - بیت_عرض_2);
    أرجع APInt(بیت_عرض_2, Tmp1);
  }

  APInt نتیجه(get_رقم_واژگان() * APINT_بیتها_در_هر_کلمه, 0);
  لكل (بیعلامت I = 0, N = get_رقم_واژگان(); I != N; ++I)
    نتیجه.U.pVal[I] = llvm::byteswap<uint64_t>(U.pVal[N - I - 1]);
  إذا (نتیجه.بیت_عرض_2 != بیت_عرض_2) {
    نتیجه.lshr_درون_قراربده(نتیجه.بیت_عرض_2 - بیت_عرض_2);
    نتیجه.بیت_عرض_2 = بیت_عرض_2;
  }
  أرجع نتیجه;
}

APInt APInt::معکوس_بیتها() ثابت {
  إذا (LLVM_LIKELY(is_Single_کلمه())) {
    بدل (بیت_عرض_2) {
    حالة 64:
      أرجع APInt(بیت_عرض_2, llvm::معکوس_بیتها<uint64_t>(U.VAL));
    حالة 32:
      أرجع APInt(بیت_عرض_2, llvm::معکوس_بیتها<uint32_t>(U.VAL));
    حالة 16:
      أرجع APInt(بیت_عرض_2, llvm::معکوس_بیتها<uint16_t>(U.VAL));
    حالة 8:
      أرجع APInt(بیت_عرض_2, llvm::معکوس_بیتها<uint8_t>(U.VAL));
    حالة 1: // fallthrough
    حالة 0:
      أرجع *هذا;
    افتراضي:
      أرجع APInt(بیت_عرض_2,
                   llvm::معکوس_بیتها<uint64_t>(U.VAL) >> (64 - بیت_عرض_2));
    }
  }

  APInt نتیجه(بیت_عرض_2, 0);
  بیعلامت رقم_واژگان_2 = get_رقم_واژگان();
  بیعلامت Excess_بیتها = رقم_واژگان_2 * APINT_بیتها_در_هر_کلمه - بیت_عرض_2;
  إذا (Excess_بیتها == 0) {
    // Fast path. No cross-word shift needed.
    لكل (بیعلامت I = 0; I < رقم_واژگان_2; ++I)
      نتیجه.U.pVal[I] = llvm::معکوس_بیتها<uint64_t>(U.pVal[رقم_واژگان_2 - 1 - I]);
    أرجع نتیجه;
  }
  // Holds reversed bits of the previous (more significant) word.
  uint64_t PrevRev = llvm::معکوس_بیتها<uint64_t>(U.pVal[رقم_واژگان_2 - 1]);
  لكل (بیعلامت I = 0; I < رقم_واژگان_2 - 1; ++I) {
    uint64_t CurrRev = llvm::معکوس_بیتها<uint64_t>(U.pVal[رقم_واژگان_2 - 2 - I]);
    نتیجه.U.pVal[I] = (PrevRev >> Excess_بیتها) | (CurrRev << (64 - Excess_بیتها));
    PrevRev = CurrRev;
  }
  نتیجه.U.pVal[رقم_واژگان_2 - 1] = PrevRev >> Excess_بیتها;
  أرجع نتیجه;
}

APInt llvm::APIntOps::Greatest_Common_مقسوم_علیه(APInt A, APInt B, منطقي IsSigned) {
  // Take absolute value if IsSigned.
  إذا (IsSigned) {
    A = A.قدر_مطلق();
    B = B.قدر_مطلق();
  }

  // Fast-path a common case.
  إذا (A == B) أرجع A;

  // Corner cases: if either operand is zero, the other is the gcd.
  إذا (!A) أرجع B;
  إذا (!B) أرجع A;

  // Count common powers of 2 and remove all other powers of 2.
  بیعلامت Pow2;
  {
    بیعلامت Pow2_A = A.countr_zero();
    بیعلامت Pow2_B = B.countr_zero();
    إذا (Pow2_A > Pow2_B) {
      A.lshr_درون_قراربده(Pow2_A - Pow2_B);
      Pow2 = Pow2_B;
    } وإلا إذا (Pow2_B > Pow2_A) {
      B.lshr_درون_قراربده(Pow2_B - Pow2_A);
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
      A.lshr_درون_قراربده(A.countr_zero() - Pow2);
    } وإلا {
      B -= A;
      B.lshr_درون_قراربده(B.countr_zero() - Pow2);
    }
  }

  أرجع A;
}

APInt llvm::APIntOps::دور_Double_مقصد_APInt(مزدوج Double, بیعلامت عرض_2) {
  uint64_t I = بیت_cast<uint64_t>(Double);

  // Get the sign bit from the highest order bit
  منطقي isNeg = I >> 63;

  // Get the 11-bit exponent and adjust for the 1023 bit bias
  int64_t نمایی_تابع = ((I >> 52) & 0x7ff) - 1023;

  // If the exponent is negative, the value is < 0 so just return 0.
  إذا (نمایی_تابع < 0)
    أرجع APInt(عرض_2, 0u);

  // Extract the mantissa by clearing the top 12 bits (sign + exponent).
  uint64_t mantissa = (I & (~0ULL >> 12)) | 1ULL << 52;

  // If the exponent doesn't shift all bits out of the mantissa
  إذا (نمایی_تابع < 52)
    أرجع isNeg ? -APInt(عرض_2, mantissa >> (52 - نمایی_تابع)) :
                    APInt(عرض_2, mantissa >> (52 - نمایی_تابع));

  // If the client didn't provide enough bits for us to shift the mantissa into
  // then the result is undefined, just return 0
  إذا (عرض_2 <= نمایی_تابع - 52)
    أرجع APInt(عرض_2, 0);

  // Otherwise, we have to shift the mantissa bits up to the right location
  APInt Tmp(عرض_2, mantissa);
  Tmp <<= (بیعلامت)نمایی_تابع - 52;
  أرجع isNeg ? -Tmp : Tmp;
}

/// This function converts this APInt to a double.
/// The layout for double is as following (IEEE Standard 754):
///  --------------------------------------
/// |  Sign    Exponent    Fraction    Bias |
/// |-------------------------------------- |
/// |  1[63]   11[62-52]   52[51-00]   1023 |
///  --------------------------------------
مزدوج APInt::دور_مقصد_Double(منطقي isSigned) ثابت {
  // Handle the simple case where the value is contained in one uint64_t.
  // It is wrong to optimize getWord(0) to VAL; there might be more than one word.
  إذا (LLVM_LIKELY(is_Single_کلمه() || get_فعال_بیتها() <= APINT_بیتها_در_هر_کلمه)) {
    إذا (isSigned) {
      int64_t sext = SignExtend64(get_کلمه(0), بیت_عرض_2);
      أرجع مزدوج(sext);
    }
    أرجع مزدوج(get_کلمه(0));
  }

  // Determine if the value is negative.
  منطقي isNeg = isSigned ? (*هذا)[بیت_عرض_2-1] : خطأ;

  // Construct the absolute value if we're negative.
  APInt Tmp(isNeg ? -(*هذا) : (*هذا));

  // Figure out how many bits we're using.
  بیعلامت n = Tmp.get_فعال_بیتها();

  // The exponent (without bias normalization) is just the number of bits
  // we are using. Note that the sign bit is gone since we constructed the
  // absolute value.
  uint64_t نمایی_تابع = n;

  // Return infinity for exponent overflow
  إذا (نمایی_تابع > 1023) {
    إذا (!isSigned || !isNeg)
      أرجع std::numeric_limits<مزدوج>::infinity();
    وإلا
      أرجع -std::numeric_limits<مزدوج>::infinity();
  }
  نمایی_تابع += 1023; // Increment for 1023 bias

  // Number of bits in mantissa is 52. To obtain the mantissa value, we must
  // extract the high 52 bits from the correct words in pVal.
  uint64_t mantissa;
  بیعلامت hi_کلمه = which_کلمه(n-1);
  إذا (hi_کلمه == 0) {
    mantissa = Tmp.U.pVal[0];
    إذا (n > 52)
      mantissa >>= n - 52; // shift down, we want the top 52 bits.
  } وإلا {
    أكد(hi_کلمه > 0 && "huh?");
    uint64_t hibits = Tmp.U.pVal[hi_کلمه] << (52 - n % APINT_بیتها_در_هر_کلمه);
    uint64_t lobits = Tmp.U.pVal[hi_کلمه-1] >> (11 + n % APINT_بیتها_در_هر_کلمه);
    mantissa = hibits | lobits;
  }

  // The leading bit of mantissa is implicit, so get rid of it.
  uint64_t sign = isNeg ? (1ULL << (APINT_بیتها_در_هر_کلمه - 1)) : 0;
  uint64_t I = sign | (نمایی_تابع << 52) | mantissa;
  أرجع بیت_cast<مزدوج>(I);
}

// Truncate to new width.
APInt APInt::trunc(بیعلامت عرض_2) ثابت {
  أكد(عرض_2 <= بیت_عرض_2 && "Invalid APInt Truncate request");

  إذا (عرض_2 <= APINT_بیتها_در_هر_کلمه)
    أرجع APInt(عرض_2, get_Raw_داده()[0], /*isSigned=*/خطأ,
                 /*implicitTrunc=*/درست);

  إذا (عرض_2 == بیت_عرض_2)
    أرجع *هذا;

  APInt نتیجه(get_حافظه(get_رقم_واژگان(عرض_2)), عرض_2);

  // Copy full words.
  بیعلامت i;
  لكل (i = 0; i != عرض_2 / APINT_بیتها_در_هر_کلمه; i++)
    نتیجه.U.pVal[i] = U.pVal[i];

  // Truncate and copy any partial word.
  بیعلامت بیتها = (0 - عرض_2) % APINT_بیتها_در_هر_کلمه;
  إذا (بیتها != 0)
    نتیجه.U.pVal[i] = U.pVal[i] << بیتها >> بیتها;

  أرجع نتیجه;
}

// Truncate to new width with unsigned saturation.
APInt APInt::truncUSat(بیعلامت عرض_2) ثابت {
  أكد(عرض_2 <= بیت_عرض_2 && "Invalid APInt Truncate request");

  // Can we just losslessly truncate it?
  إذا (isIntN(عرض_2))
    أرجع trunc(عرض_2);
  // If not, then just return the new limit.
  أرجع APInt::get_Max_مقدار(عرض_2);
}

// Truncate to new width with signed saturation to signed result.
APInt APInt::truncSSat(بیعلامت عرض_2) ثابت {
  أكد(عرض_2 <= بیت_عرض_2 && "Invalid APInt Truncate request");

  // Can we just losslessly truncate it?
  إذا (isSignedIntN(عرض_2))
    أرجع trunc(عرض_2);
  // If not, then just return the new limits.
  أرجع isNegative() ? APInt::get_Signed_Min_مقدار(عرض_2)
                      : APInt::get_Signed_Max_مقدار(عرض_2);
}

// Truncate to new width with signed saturation to unsigned result.
APInt APInt::truncSSatU(بیعلامت عرض_2) ثابت {
  أكد(عرض_2 <= بیت_عرض_2 && "Invalid APInt Truncate request");

  // Can we just losslessly truncate it?
  إذا (isIntN(عرض_2))
    أرجع trunc(عرض_2);
  // If not, then just return the new limits.
  أرجع isNegative() ? APInt::getZero(عرض_2) : APInt::get_Max_مقدار(عرض_2);
}

// Sign extend to a new width.
APInt APInt::sext(بیعلامت عرض) ثابت {
  أكد(عرض >= بیت_عرض_2 && "Invalid APInt SignExtend request");

  إذا (عرض <= APINT_بیتها_در_هر_کلمه)
    أرجع APInt(عرض, SignExtend64(U.VAL, بیت_عرض_2), /*isSigned=*/درست);

  إذا (عرض == بیت_عرض_2)
    أرجع *هذا;

  APInt نتیجه(get_حافظه(get_رقم_واژگان(عرض)), عرض);

  // Copy words.
  std::کپی_حافظه(نتیجه.U.pVal, get_Raw_داده(), get_رقم_واژگان() * APINT_کلمه_اندازه);

  // Sign extend the last word since there may be unused bits in the input.
  نتیجه.U.pVal[get_رقم_واژگان() - 1] =
      SignExtend64(نتیجه.U.pVal[get_رقم_واژگان() - 1],
                   ((بیت_عرض_2 - 1) % APINT_بیتها_در_هر_کلمه) + 1);

  // Fill with sign bits.
  std::memset(نتیجه.U.pVal + get_رقم_واژگان(), isNegative() ? -1 : 0,
              (نتیجه.get_رقم_واژگان() - get_رقم_واژگان()) * APINT_کلمه_اندازه);
  نتیجه.clear_استفادهنشده_بیتها();
  أرجع نتیجه;
}

//  Zero extend to a new width.
APInt APInt::zext(بیعلامت عرض_2) ثابت {
  أكد(عرض_2 >= بیت_عرض_2 && "Invalid APInt ZeroExtend request");

  إذا (عرض_2 <= APINT_بیتها_در_هر_کلمه)
    أرجع APInt(عرض_2, U.VAL);

  إذا (عرض_2 == بیت_عرض_2)
    أرجع *هذا;

  APInt نتیجه(get_حافظه(get_رقم_واژگان(عرض_2)), عرض_2);

  // Copy words.
  std::کپی_حافظه(نتیجه.U.pVal, get_Raw_داده(), get_رقم_واژگان() * APINT_کلمه_اندازه);

  // Zero remaining words.
  std::memset(نتیجه.U.pVal + get_رقم_واژگان(), 0,
              (نتیجه.get_رقم_واژگان() - get_رقم_واژگان()) * APINT_کلمه_اندازه);

  أرجع نتیجه;
}

APInt APInt::zextOrTrunc(بیعلامت عرض_2) ثابت {
  إذا (بیت_عرض_2 < عرض_2)
    أرجع zext(عرض_2);
  إذا (بیت_عرض_2 > عرض_2)
    أرجع trunc(عرض_2);
  أرجع *هذا;
}

APInt APInt::sextOrTrunc(بیعلامت عرض_2) ثابت {
  إذا (بیت_عرض_2 < عرض_2)
    أرجع sext(عرض_2);
  إذا (بیت_عرض_2 > عرض_2)
    أرجع trunc(عرض_2);
  أرجع *هذا;
}

/// Arithmetic right-shift this APInt by shiftAmt.
/// Arithmetic right-shift function.
فراغ APInt::ashr_درون_قراربده(ثابت APInt &لغزش_Amt) {
  ashr_درون_قراربده((بیعلامت)لغزش_Amt.get_Limited_مقدار(بیت_عرض_2));
}

/// Arithmetic right-shift this APInt by shiftAmt.
/// Arithmetic right-shift function.
فراغ APInt::ashrSlowCase(بیعلامت لغزش_Amt_2) {
  // Don't bother performing a no-op shift.
  إذا (!لغزش_Amt_2)
    أرجع;

  // Save the original sign bit for later.
  منطقي Negative = isNegative();

  // WordShift is the inter-part shift; BitShift is intra-part shift.
  بیعلامت کلمه_لغزش = لغزش_Amt_2 / APINT_بیتها_در_هر_کلمه;
  بیعلامت بیت_لغزش = لغزش_Amt_2 % APINT_بیتها_در_هر_کلمه;

  بیعلامت واژگان_مقصد_Move = get_رقم_واژگان() - کلمه_لغزش;
  إذا (واژگان_مقصد_Move != 0) {
    // Sign extend the last word to fill in the unused bits.
    U.pVal[get_رقم_واژگان() - 1] = SignExtend64(
        U.pVal[get_رقم_واژگان() - 1], ((بیت_عرض_2 - 1) % APINT_بیتها_در_هر_کلمه) + 1);

    // Fastpath for moving by whole words.
    إذا (بیت_لغزش == 0) {
      std::memmove(U.pVal, U.pVal + کلمه_لغزش, واژگان_مقصد_Move * APINT_کلمه_اندازه);
    } وإلا {
      // Move the words containing significant bits.
      لكل (بیعلامت i = 0; i != واژگان_مقصد_Move - 1; ++i)
        U.pVal[i] = (U.pVal[i + کلمه_لغزش] >> بیت_لغزش) |
                    (U.pVal[i + کلمه_لغزش + 1] << (APINT_بیتها_در_هر_کلمه - بیت_لغزش));

      // Handle the last word which has no high bits to copy. Use an arithmetic
      // shift to preserve the sign bit.
      U.pVal[واژگان_مقصد_Move - 1] =
          (int64_t)U.pVal[کلمه_لغزش + واژگان_مقصد_Move - 1] >> بیت_لغزش;
    }
  }

  // Fill in the remainder based on the original sign.
  std::memset(U.pVal + واژگان_مقصد_Move, Negative ? -1 : 0,
              کلمه_لغزش * APINT_کلمه_اندازه);
  clear_استفادهنشده_بیتها();
}

/// Logical right-shift this APInt by shiftAmt.
/// Logical right-shift function.
فراغ APInt::lshr_درون_قراربده(ثابت APInt &لغزش_Amt) {
  lshr_درون_قراربده((بیعلامت)لغزش_Amt.get_Limited_مقدار(بیت_عرض_2));
}

/// Logical right-shift this APInt by shiftAmt.
/// Logical right-shift function.
فراغ APInt::lshrSlowCase(بیعلامت لغزش_Amt_2) {
  tc_لغزش_راست(U.pVal, get_رقم_واژگان(), لغزش_Amt_2);
}

/// Left-shift this APInt by shiftAmt.
/// Left-shift function.
APInt &APInt::عملگر<<=(ثابت APInt &لغزش_Amt) {
  // It's undefined behavior in C to shift by BitWidth or greater.
  *هذا <<= (بیعلامت)لغزش_Amt.get_Limited_مقدار(بیت_عرض_2);
  أرجع *هذا;
}

فراغ APInt::shlSlowCase(بیعلامت لغزش_Amt_2) {
  tc_لغزش_چپ(U.pVal, get_رقم_واژگان(), لغزش_Amt_2);
  clear_استفادهنشده_بیتها();
}

// Calculate the rotate amount modulo the bit width.
ساكن بیعلامت rotateModulo(بیعلامت بیت_عرض_2, ثابت APInt &rotateAmt) {
  إذا (LLVM_UNLIKELY(بیت_عرض_2 == 0))
    أرجع 0;
  بیعلامت rot_بیت_عرض = rotateAmt.get_بیت_عرض();
  APInt rot = rotateAmt;
  إذا (rot_بیت_عرض < بیت_عرض_2) {
    // Extend the rotate APInt, so that the urem doesn't divide by 0.
    // e.g. APInt(1, 32) would give APInt(1, 0).
    rot = rotateAmt.zext(بیت_عرض_2);
  }
  rot = rot.urem(APInt(rot.get_بیت_عرض(), بیت_عرض_2));
  أرجع rot.get_Limited_مقدار(بیت_عرض_2);
}

APInt APInt::rotl(ثابت APInt &rotateAmt) ثابت {
  أرجع rotl(rotateModulo(بیت_عرض_2, rotateAmt));
}

APInt APInt::rotl(بیعلامت rotateAmt) ثابت {
  إذا (LLVM_UNLIKELY(بیت_عرض_2 == 0))
    أرجع *هذا;
  rotateAmt %= بیت_عرض_2;
  إذا (rotateAmt == 0)
    أرجع *هذا;
  أرجع shl(rotateAmt) | lshr(بیت_عرض_2 - rotateAmt);
}

APInt APInt::rotr(ثابت APInt &rotateAmt) ثابت {
  أرجع rotr(rotateModulo(بیت_عرض_2, rotateAmt));
}

APInt APInt::rotr(بیعلامت rotateAmt) ثابت {
  إذا (بیت_عرض_2 == 0)
    أرجع *هذا;
  rotateAmt %= بیت_عرض_2;
  إذا (rotateAmt == 0)
    أرجع *هذا;
  أرجع lshr(rotateAmt) | shl(بیت_عرض_2 - rotateAmt);
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
بیعلامت APInt::نزدیکترین_Log_Base2() ثابت {
  // Special case when we have a bitwidth of 1. If VAL is 1, then we
  // get 0. If VAL is 0, we get WORDTYPE_MAX which gets truncated to
  // UINT32_MAX.
  إذا (بیت_عرض_2 == 1)
    أرجع U.VAL - 1;

  // Handle the zero case.
  إذا (isZero())
    أرجع UINT32_MAX;

  // The non-zero case is handled by computing:
  //
  //   nearestLogBase2(x) = logBase2(x) + x[logBase2(x)-1].
  //
  // where x[i] is referring to the value of the ith bit of x.
  بیعلامت lg = logBase2();
  أرجع lg + بیعلامت((*هذا)[lg - 1]);
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
  بیعلامت magnitude = get_فعال_بیتها();

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
    أرجع APInt(بیت_عرض_2,
                 results[(LLVM_LIKELY(is_Single_کلمه()) ? U.VAL : U.pVal[0])]);
  }

  // If the magnitude of the value fits in less than 52 bits (the precision of
  // an IEEE double precision floating point value), then we can use the
  // libc sqrt function which will probably use a hardware sqrt computation.
  // This should be faster than the algorithm below.
  إذا (magnitude < 52) {
    أرجع APInt(بیت_عرض_2,
                 uint64_t(::floor(::جذر(مزدوج(
                     LLVM_LIKELY(is_Single_کلمه()) ? U.VAL : U.pVal[0])))));
  }

  // Okay, all the short cuts are exhausted. We must compute it. The following
  // is a classical Babylonian method for computing the square root. This code
  // was adapted to APInt from a wikipedia article on such computations.
  // See http://www.wikipedia.org/ and go to the page named
  // Calculate_an_integer_square_root.
  بیعلامت nbits = بیت_عرض_2, i = 4;
  APInt testy(بیت_عرض_2, 16);
  APInt x_old(بیت_عرض_2, 1);
  APInt x_new(بیت_عرض_2, 0);
  APInt two(بیت_عرض_2, 2);

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
                     بیعلامت m, بیعلامت n) {
  أكد(u && "Must provide dividend");
  أكد(v && "Must provide divisor");
  أكد(q && "Must provide quotient");
  أكد(u != v && u != q && v != q && "Must use different memory");
  أكد(n>1 && "n must be > 1");

  // b denotes the base of the number system. In our case b is 2^32.
  ثابت uint64_t b = uint64_t(1) << 32;

// The DEBUG macros here tend to be spam in the debug output if you're not
// debugging this code. Disable them unless KNUTH_DEBUG is defined.
#اگر_تعریف_شده KNUTH_DEBUG
#تعریف_کن DEBUG_KNUTH(X) LLVM_DEBUG(X)
#وإلا
#تعریف_کن DEBUG_KNUTH(X) افعل {} طالما(خطأ)
#پایان_اگر

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
  بیعلامت لغزش = llvm::countl_zero(v[n - 1]);
  uint32_t v_انتقال = 0;
  uint32_t u_انتقال = 0;
  إذا (لغزش) {
    لكل (بیعلامت i = 0; i < m+n; ++i) {
      uint32_t u_tmp = u[i] >> (32 - لغزش);
      u[i] = (u[i] << لغزش) | u_انتقال;
      u_انتقال = u_tmp;
    }
    لكل (بیعلامت i = 0; i < n; ++i) {
      uint32_t v_tmp = v[i] >> (32 - لغزش);
      v[i] = (v[i] << لغزش) | v_انتقال;
      v_انتقال = v_tmp;
    }
  }
  u[m+n] = u_انتقال;

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
    لكل (بیعلامت i = 0; i < n; ++i) {
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
      منطقي انتقال = خطأ;
      لكل (بیعلامت i = 0; i < n; i++) {
        uint32_t حد = std::min(u[j+i],v[i]);
        u[j+i] += v[i] + انتقال;
        انتقال = u[j+i] < حد || (انتقال && u[j+i] == حد);
      }
      u[j+n] += انتقال;
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
    إذا (لغزش) {
      uint32_t انتقال = 0;
      DEBUG_KNUTH(dbgs() << "KnuthDiv: remainder:");
      لكل (صحيح i = n-1; i >= 0; i--) {
        r[i] = (u[i] >> لغزش) | انتقال;
        انتقال = u[i] << (32 - لغزش);
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

فراغ APInt::divide(ثابت کلمه_Type *LHS, بیعلامت lhs_واژگان, ثابت کلمه_Type *RHS,
                   بیعلامت rhs_واژگان, کلمه_Type *Quotient, کلمه_Type *Remainder) {
  أكد(lhs_واژگان >= rhs_واژگان && "Fractional result");

  // First, compose the values into an array of 32-bit words instead of
  // 64-bit words. This is a necessity of both the "short division" algorithm
  // and the Knuth "classical algorithm" which requires there to be native
  // operations for +, -, and * on an m bit value with an m*2 bit result. We
  // can't use 64-bit operands here because we don't have native results of
  // 128-bits. Furthermore, casting the 64-bit values to 32-bit values won't
  // work on large-endian machines.
  بیعلامت n = rhs_واژگان * 2;
  بیعلامت m = (lhs_واژگان * 2) - n;

  // Allocate space for the temporary values we need either on the stack, if
  // it will fit, or on the heap if it won't.
  uint32_t SPACE[128];
  uint32_t *U = اشاره_تهی;
  uint32_t *V = اشاره_تهی;
  uint32_t *Q = اشاره_تهی;
  uint32_t *R = اشاره_تهی;
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
  لكل (بیعلامت i = 0; i < lhs_واژگان; ++i) {
    uint64_t tmp = LHS[i];
    U[i * 2] = Lo_32(tmp);
    U[i * 2 + 1] = Hi_32(tmp);
  }
  U[m+n] = 0; // this extra word is for "spill" in the Knuth algorithm.

  // Initialize the divisor
  memset(V, 0, (n)*حجم(uint32_t));
  لكل (بیعلامت i = 0; i < rhs_واژگان; ++i) {
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
  لكل (بیعلامت i = n; i > 0 && V[i-1] == 0; i--) {
    n--;
    m++;
  }
  لكل (بیعلامت i = m+n; i > 0 && U[i-1] == 0; i--)
    m--;

  // If we're left with only a single word for the divisor, Knuth doesn't work
  // so we implement the short division algorithm here. This is much simpler
  // and faster because we are certain that we can divide a 64-bit quantity
  // by a 32-bit quantity at hardware speed and short division is simply a
  // series of such operations. This is just like doing short division but we
  // are using base 2^32 instead of base 10.
  أكد(n != 0 && "Divide by zero?");
  إذا (n == 1) {
    uint32_t مقسوم_علیه = V[0];
    uint32_t remainder = 0;
    لكل (صحيح i = m; i >= 0; i--) {
      uint64_t partial_dividend = Make_64(remainder, U[i]);
      إذا (partial_dividend == 0) {
        Q[i] = 0;
        remainder = 0;
      } وإلا إذا (partial_dividend < مقسوم_علیه) {
        Q[i] = 0;
        remainder = Lo_32(partial_dividend);
      } وإلا إذا (partial_dividend == مقسوم_علیه) {
        Q[i] = 1;
        remainder = 0;
      } وإلا {
        Q[i] = Lo_32(partial_dividend / مقسوم_علیه);
        remainder = Lo_32(partial_dividend - (Q[i] * مقسوم_علیه));
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
    لكل (بیعلامت i = 0; i < lhs_واژگان; ++i)
      Quotient[i] = Make_64(Q[i*2+1], Q[i*2]);
  }

  // If the caller wants the remainder
  إذا (Remainder) {
    لكل (بیعلامت i = 0; i < rhs_واژگان; ++i)
      Remainder[i] = Make_64(R[i*2+1], R[i*2]);
  }

  // Clean up the memory we allocated.
  إذا (U != &SPACE[0]) {
    حذف [] U;
    حذف [] V;
    حذف [] Q;
    حذف [] R;
  }
}

APInt APInt::udiv(ثابت APInt &RHS) ثابت {
  أكد(بیت_عرض_2 == RHS.بیت_عرض_2 && "Bit widths must be the same");

  // First, deal with the easy case
  إذا (LLVM_LIKELY(is_Single_کلمه())) {
    أكد(RHS.U.VAL != 0 && "Divide by zero?");
    أرجع APInt(بیت_عرض_2, U.VAL / RHS.U.VAL);
  }

  // Get some facts about the LHS and RHS number of bits and words
  بیعلامت lhs_واژگان = get_رقم_واژگان(get_فعال_بیتها());
  بیعلامت rhs_بیتها  = RHS.get_فعال_بیتها();
  بیعلامت rhs_واژگان = get_رقم_واژگان(rhs_بیتها);
  أكد(rhs_واژگان && "Divided by zero???");

  // Deal with some degenerate cases
  إذا (!lhs_واژگان)
    // 0 / X ===> 0
    أرجع APInt(بیت_عرض_2, 0);
  إذا (rhs_بیتها == 1)
    // X / 1 ===> X
    أرجع *هذا;
  إذا (lhs_واژگان < rhs_واژگان || هذا->ult(RHS))
    // X / Y ===> 0, iff X < Y
    أرجع APInt(بیت_عرض_2, 0);
  إذا (*هذا == RHS)
    // X / X ===> 1
    أرجع APInt(بیت_عرض_2, 1);
  إذا (lhs_واژگان == 1) // rhsWords is 1 if lhsWords is 1.
    // All high words are zero, just use native divide
    أرجع APInt(بیت_عرض_2, هذا->U.pVal[0] / RHS.U.pVal[0]);

  // We have to compute it the hard way. Invoke the Knuth divide algorithm.
  APInt Quotient(بیت_عرض_2, 0); // to hold result.
  divide(U.pVal, lhs_واژگان, RHS.U.pVal, rhs_واژگان, Quotient.U.pVal, اشاره_تهی);
  أرجع Quotient;
}

APInt APInt::udiv(uint64_t RHS) ثابت {
  أكد(RHS != 0 && "Divide by zero?");

  // First, deal with the easy case
  إذا (LLVM_LIKELY(is_Single_کلمه()))
    أرجع APInt(بیت_عرض_2, U.VAL / RHS);

  // Get some facts about the LHS words.
  بیعلامت lhs_واژگان = get_رقم_واژگان(get_فعال_بیتها());

  // Deal with some degenerate cases
  إذا (!lhs_واژگان)
    // 0 / X ===> 0
    أرجع APInt(بیت_عرض_2, 0);
  إذا (RHS == 1)
    // X / 1 ===> X
    أرجع *هذا;
  إذا (هذا->ult(RHS))
    // X / Y ===> 0, iff X < Y
    أرجع APInt(بیت_عرض_2, 0);
  إذا (*هذا == RHS)
    // X / X ===> 1
    أرجع APInt(بیت_عرض_2, 1);
  إذا (lhs_واژگان == 1) // rhsWords is 1 if lhsWords is 1.
    // All high words are zero, just use native divide
    أرجع APInt(بیت_عرض_2, هذا->U.pVal[0] / RHS);

  // We have to compute it the hard way. Invoke the Knuth divide algorithm.
  APInt Quotient(بیت_عرض_2, 0); // to hold result.
  divide(U.pVal, lhs_واژگان, &RHS, 1, Quotient.U.pVal, اشاره_تهی);
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
  أكد(بیت_عرض_2 == RHS.بیت_عرض_2 && "Bit widths must be the same");
  إذا (LLVM_LIKELY(is_Single_کلمه())) {
    أكد(RHS.U.VAL != 0 && "Remainder by zero?");
    أرجع APInt(بیت_عرض_2, U.VAL % RHS.U.VAL);
  }

  // Get some facts about the LHS
  بیعلامت lhs_واژگان = get_رقم_واژگان(get_فعال_بیتها());

  // Get some facts about the RHS
  بیعلامت rhs_بیتها = RHS.get_فعال_بیتها();
  بیعلامت rhs_واژگان = get_رقم_واژگان(rhs_بیتها);
  أكد(rhs_واژگان && "Performing remainder operation by zero ???");

  // Check the degenerate cases
  إذا (lhs_واژگان == 0)
    // 0 % Y ===> 0
    أرجع APInt(بیت_عرض_2, 0);
  إذا (rhs_بیتها == 1)
    // X % 1 ===> 0
    أرجع APInt(بیت_عرض_2, 0);
  إذا (lhs_واژگان < rhs_واژگان || هذا->ult(RHS))
    // X % Y ===> X, iff X < Y
    أرجع *هذا;
  إذا (*هذا == RHS)
    // X % X == 0;
    أرجع APInt(بیت_عرض_2, 0);
  إذا (lhs_واژگان == 1)
    // All high words are zero, just use native remainder
    أرجع APInt(بیت_عرض_2, U.pVal[0] % RHS.U.pVal[0]);
  إذا (RHS.isPowerOf2()) {
    // X % 2^w ===> X & (2^w - 1)
    APInt نتیجه(*هذا);
    نتیجه.clear_بیتها(RHS.logBase2(), بیت_عرض_2);
    أرجع نتیجه;
  }

  // We have to compute it the hard way. Invoke the Knuth divide algorithm.
  APInt Remainder(بیت_عرض_2, 0);
  divide(U.pVal, lhs_واژگان, RHS.U.pVal, rhs_واژگان, اشاره_تهی, Remainder.U.pVal);
  أرجع Remainder;
}

uint64_t APInt::urem(uint64_t RHS) ثابت {
  أكد(RHS != 0 && "Remainder by zero?");

  إذا (LLVM_LIKELY(is_Single_کلمه()))
    أرجع U.VAL % RHS;

  // Get some facts about the LHS
  بیعلامت lhs_واژگان = get_رقم_واژگان(get_فعال_بیتها());

  // Check the degenerate cases
  إذا (lhs_واژگان == 0)
    // 0 % Y ===> 0
    أرجع 0;
  إذا (RHS == 1)
    // X % 1 ===> 0
    أرجع 0;
  إذا (هذا->ult(RHS))
    // X % Y ===> X, iff X < Y
    أرجع get_ZExt_مقدار();
  إذا (*هذا == RHS)
    // X % X == 0;
    أرجع 0;
  إذا (lhs_واژگان == 1)
    // All high words are zero, just use native remainder
    أرجع U.pVal[0] % RHS;
  إذا (llvm::isPowerOf2_64(RHS))
    // X % 2^w ===> X & (2^w - 1)
    أرجع U.pVal[0] & (RHS - 1);

  // We have to compute it the hard way. Invoke the Knuth divide algorithm.
  uint64_t Remainder;
  divide(U.pVal, lhs_واژگان, &RHS, 1, اشاره_تهی, &Remainder);
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
  أكد(LHS.بیت_عرض_2 == RHS.بیت_عرض_2 && "Bit widths must be the same");
  بیعلامت بیت_عرض_2 = LHS.بیت_عرض_2;

  // First, deal with the easy case
  إذا (LLVM_LIKELY(LHS.is_Single_کلمه())) {
    أكد(RHS.U.VAL != 0 && "Divide by zero?");
    uint64_t QuotVal = LHS.U.VAL / RHS.U.VAL;
    uint64_t RemVal = LHS.U.VAL % RHS.U.VAL;
    Quotient = APInt(بیت_عرض_2, QuotVal);
    Remainder = APInt(بیت_عرض_2, RemVal);
    أرجع;
  }

  // Get some size facts about the dividend and divisor
  بیعلامت lhs_واژگان = get_رقم_واژگان(LHS.get_فعال_بیتها());
  بیعلامت rhs_بیتها  = RHS.get_فعال_بیتها();
  بیعلامت rhs_واژگان = get_رقم_واژگان(rhs_بیتها);
  أكد(rhs_واژگان && "Performing divrem operation by zero ???");

  // Check the degenerate cases
  إذا (lhs_واژگان == 0) {
    Quotient = APInt(بیت_عرض_2, 0);    // 0 / Y ===> 0
    Remainder = APInt(بیت_عرض_2, 0);   // 0 % Y ===> 0
    أرجع;
  }

  إذا (rhs_بیتها == 1) {
    Quotient = LHS;                   // X / 1 ===> X
    Remainder = APInt(بیت_عرض_2, 0);   // X % 1 ===> 0
  }

  إذا (lhs_واژگان < rhs_واژگان || LHS.ult(RHS)) {
    Remainder = LHS;                  // X % Y ===> X, iff X < Y
    Quotient = APInt(بیت_عرض_2, 0);    // X / Y ===> 0, iff X < Y
    أرجع;
  }

  إذا (LHS == RHS) {
    Quotient  = APInt(بیت_عرض_2, 1);   // X / X ===> 1
    Remainder = APInt(بیت_عرض_2, 0);   // X % X ===> 0;
    أرجع;
  }

  // Make sure there is enough space to hold the results.
  // NOTE: This assumes that reallocate won't affect any bits if it doesn't
  // change the size. This is necessary if Quotient or Remainder is aliased
  // with LHS or RHS.
  Quotient.reallocate(بیت_عرض_2);
  Remainder.reallocate(بیت_عرض_2);

  إذا (lhs_واژگان == 1) { // rhsWords is 1 if lhsWords is 1.
    // There is only one word to consider so use the native versions.
    uint64_t lhs_مقدار = LHS.U.pVal[0];
    uint64_t rhs_مقدار = RHS.U.pVal[0];
    Quotient = lhs_مقدار / rhs_مقدار;
    Remainder = lhs_مقدار % rhs_مقدار;
    أرجع;
  }

  // Okay, lets do it the long way
  divide(LHS.U.pVal, lhs_واژگان, RHS.U.pVal, rhs_واژگان, Quotient.U.pVal,
         Remainder.U.pVal);
  // Clear the rest of the Quotient and Remainder.
  std::memset(Quotient.U.pVal + lhs_واژگان, 0,
              (get_رقم_واژگان(بیت_عرض_2) - lhs_واژگان) * APINT_کلمه_اندازه);
  std::memset(Remainder.U.pVal + rhs_واژگان, 0,
              (get_رقم_واژگان(بیت_عرض_2) - rhs_واژگان) * APINT_کلمه_اندازه);
}

فراغ APInt::udivrem(ثابت APInt &LHS, uint64_t RHS, APInt &Quotient,
                    uint64_t &Remainder) {
  أكد(RHS != 0 && "Divide by zero?");
  بیعلامت بیت_عرض_2 = LHS.بیت_عرض_2;

  // First, deal with the easy case
  إذا (LLVM_LIKELY(LHS.is_Single_کلمه())) {
    uint64_t QuotVal = LHS.U.VAL / RHS;
    Remainder = LHS.U.VAL % RHS;
    Quotient = APInt(بیت_عرض_2, QuotVal);
    أرجع;
  }

  // Get some size facts about the dividend and divisor
  بیعلامت lhs_واژگان = get_رقم_واژگان(LHS.get_فعال_بیتها());

  // Check the degenerate cases
  إذا (lhs_واژگان == 0) {
    Quotient = APInt(بیت_عرض_2, 0);    // 0 / Y ===> 0
    Remainder = 0;                    // 0 % Y ===> 0
    أرجع;
  }

  إذا (RHS == 1) {
    Quotient = LHS;                   // X / 1 ===> X
    Remainder = 0;                    // X % 1 ===> 0
    أرجع;
  }

  إذا (LHS.ult(RHS)) {
    Remainder = LHS.get_ZExt_مقدار();   // X % Y ===> X, iff X < Y
    Quotient = APInt(بیت_عرض_2, 0);    // X / Y ===> 0, iff X < Y
    أرجع;
  }

  إذا (LHS == RHS) {
    Quotient  = APInt(بیت_عرض_2, 1);   // X / X ===> 1
    Remainder = 0;                    // X % X ===> 0;
    أرجع;
  }

  // Make sure there is enough space to hold the results.
  // NOTE: This assumes that reallocate won't affect any bits if it doesn't
  // change the size. This is necessary if Quotient is aliased with LHS.
  Quotient.reallocate(بیت_عرض_2);

  إذا (lhs_واژگان == 1) { // rhsWords is 1 if lhsWords is 1.
    // There is only one word to consider so use the native versions.
    uint64_t lhs_مقدار = LHS.U.pVal[0];
    Quotient = lhs_مقدار / RHS;
    Remainder = lhs_مقدار % RHS;
    أرجع;
  }

  // Okay, lets do it the long way
  divide(LHS.U.pVal, lhs_واژگان, &RHS, 1, Quotient.U.pVal, &Remainder);
  // Clear the rest of the Quotient.
  std::memset(Quotient.U.pVal + lhs_واژگان, 0,
              (get_رقم_واژگان(بیت_عرض_2) - lhs_واژگان) * APINT_کلمه_اندازه);
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

APInt APInt::sadd_ov(ثابت APInt &RHS, منطقي &سرریز) ثابت {
  APInt Res = *هذا+RHS;
  سرریز = isNonNegative() == RHS.isNonNegative() &&
             Res.isNonNegative() != isNonNegative();
  أرجع Res;
}

APInt APInt::uadd_ov(ثابت APInt &RHS, منطقي &سرریز) ثابت {
  APInt Res = *هذا+RHS;
  سرریز = Res.ult(RHS);
  أرجع Res;
}

APInt APInt::ssub_ov(ثابت APInt &RHS, منطقي &سرریز) ثابت {
  APInt Res = *هذا - RHS;
  سرریز = isNonNegative() != RHS.isNonNegative() &&
             Res.isNonNegative() != isNonNegative();
  أرجع Res;
}

APInt APInt::usub_ov(ثابت APInt &RHS, منطقي &سرریز) ثابت {
  APInt Res = *هذا-RHS;
  سرریز = Res.ugt(*هذا);
  أرجع Res;
}

APInt APInt::sdiv_ov(ثابت APInt &RHS, منطقي &سرریز) ثابت {
  // MININT/-1  -->  overflow.
  سرریز = is_Min_Signed_مقدار() && RHS.isAllOnes();
  أرجع sdiv(RHS);
}

APInt APInt::smul_ov(ثابت APInt &RHS, منطقي &سرریز) ثابت {
  APInt Res = *هذا * RHS;

  إذا (RHS != 0)
    سرریز = Res.sdiv(RHS) != *هذا ||
               (is_Min_Signed_مقدار() && RHS.isAllOnes());
  وإلا
    سرریز = خطأ;
  أرجع Res;
}

APInt APInt::umul_ov(ثابت APInt &RHS, منطقي &سرریز) ثابت {
  إذا (countl_zero() + RHS.countl_zero() + 2 <= بیت_عرض_2) {
    سرریز = درست;
    أرجع *هذا * RHS;
  }

  APInt Res = lshr(1) * RHS;
  سرریز = Res.isNegative();
  Res <<= 1;
  إذا ((*هذا)[0]) {
    Res += RHS;
    إذا (Res.ult(RHS))
      سرریز = درست;
  }
  أرجع Res;
}

APInt APInt::sshl_ov(ثابت APInt &ShAmt, منطقي &سرریز) ثابت {
  أرجع sshl_ov(ShAmt.get_Limited_مقدار(get_بیت_عرض()), سرریز);
}

APInt APInt::sshl_ov(بیعلامت ShAmt, منطقي &سرریز) ثابت {
  سرریز = ShAmt >= get_بیت_عرض();
  إذا (سرریز)
    أرجع APInt(بیت_عرض_2, 0);

  إذا (isNonNegative()) // Don't allow sign change.
    سرریز = ShAmt >= countl_zero();
  وإلا
    سرریز = ShAmt >= countl_one();

  أرجع *هذا << ShAmt;
}

APInt APInt::ushl_ov(ثابت APInt &ShAmt, منطقي &سرریز) ثابت {
  أرجع ushl_ov(ShAmt.get_Limited_مقدار(get_بیت_عرض()), سرریز);
}

APInt APInt::ushl_ov(بیعلامت ShAmt, منطقي &سرریز) ثابت {
  سرریز = ShAmt >= get_بیت_عرض();
  إذا (سرریز)
    أرجع APInt(بیت_عرض_2, 0);

  سرریز = ShAmt > countl_zero();

  أرجع *هذا << ShAmt;
}

APInt APInt::sfloordiv_ov(ثابت APInt &RHS, منطقي &سرریز) ثابت {
  APInt quotient = sdiv_ov(RHS, سرریز);
  إذا ((quotient * RHS != *هذا) && (isNegative() != RHS.isNegative()))
    أرجع quotient - 1;
  أرجع quotient;
}

APInt APInt::sadd_sat(ثابت APInt &RHS) ثابت {
  منطقي سرریز;
  APInt Res = sadd_ov(RHS, سرریز);
  إذا (!سرریز)
    أرجع Res;

  أرجع isNegative() ? APInt::get_Signed_Min_مقدار(بیت_عرض_2)
                      : APInt::get_Signed_Max_مقدار(بیت_عرض_2);
}

APInt APInt::uadd_sat(ثابت APInt &RHS) ثابت {
  منطقي سرریز;
  APInt Res = uadd_ov(RHS, سرریز);
  إذا (!سرریز)
    أرجع Res;

  أرجع APInt::get_Max_مقدار(بیت_عرض_2);
}

APInt APInt::ssub_sat(ثابت APInt &RHS) ثابت {
  منطقي سرریز;
  APInt Res = ssub_ov(RHS, سرریز);
  إذا (!سرریز)
    أرجع Res;

  أرجع isNegative() ? APInt::get_Signed_Min_مقدار(بیت_عرض_2)
                      : APInt::get_Signed_Max_مقدار(بیت_عرض_2);
}

APInt APInt::usub_sat(ثابت APInt &RHS) ثابت {
  منطقي سرریز;
  APInt Res = usub_ov(RHS, سرریز);
  إذا (!سرریز)
    أرجع Res;

  أرجع APInt(بیت_عرض_2, 0);
}

APInt APInt::smul_sat(ثابت APInt &RHS) ثابت {
  منطقي سرریز;
  APInt Res = smul_ov(RHS, سرریز);
  إذا (!سرریز)
    أرجع Res;

  // The result is negative if one and only one of inputs is negative.
  منطقي ResIsNegative = isNegative() ^ RHS.isNegative();

  أرجع ResIsNegative ? APInt::get_Signed_Min_مقدار(بیت_عرض_2)
                       : APInt::get_Signed_Max_مقدار(بیت_عرض_2);
}

APInt APInt::umul_sat(ثابت APInt &RHS) ثابت {
  منطقي سرریز;
  APInt Res = umul_ov(RHS, سرریز);
  إذا (!سرریز)
    أرجع Res;

  أرجع APInt::get_Max_مقدار(بیت_عرض_2);
}

APInt APInt::sshl_sat(ثابت APInt &RHS) ثابت {
  أرجع sshl_sat(RHS.get_Limited_مقدار(get_بیت_عرض()));
}

APInt APInt::sshl_sat(بیعلامت RHS) ثابت {
  منطقي سرریز;
  APInt Res = sshl_ov(RHS, سرریز);
  إذا (!سرریز)
    أرجع Res;

  أرجع isNegative() ? APInt::get_Signed_Min_مقدار(بیت_عرض_2)
                      : APInt::get_Signed_Max_مقدار(بیت_عرض_2);
}

APInt APInt::ushl_sat(ثابت APInt &RHS) ثابت {
  أرجع ushl_sat(RHS.get_Limited_مقدار(get_بیت_عرض()));
}

APInt APInt::ushl_sat(بیعلامت RHS) ثابت {
  منطقي سرریز;
  APInt Res = ushl_ov(RHS, سرریز);
  إذا (!سرریز)
    أرجع Res;

  أرجع APInt::get_Max_مقدار(بیت_عرض_2);
}

فراغ APInt::fromString(بیعلامت numbits, StringRef str, uint8_t radix) {
  // Check our assumptions here
  أكد(!str.خالی() && "Invalid string length");
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
    U.pVal = get_Cleared_حافظه(get_رقم_واژگان());

  // Figure out if we can shift instead of multiply
  بیعلامت لغزش = (radix == 16 ? 4 : radix == 8 ? 3 : radix == 2 ? 1 : 0);

  // Enter digit traversal loop
  لكل (StringRef::iterator e = str.انتها(); p != e; ++p) {
    بیعلامت digit = getDigit(*p, radix);
    أكد(digit < radix && "Invalid character in digit string");

    // Shift or multiply the value by the radix
    إذا (slen > 1) {
      إذا (لغزش)
        *هذا <<= لغزش;
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

فراغ APInt::مقصد_String(Small_وکتور_Impl<حرف> &Str, بیعلامت Radix, منطقي Signed,
                     منطقي formatAsCLiteral, منطقي فوقانی_Case_2,
                     منطقي درج_Separators) ثابت {
  أكد((Radix == 10 || Radix == 8 || Radix == 16 || Radix == 2 ||
          Radix == 36) &&
         "Radix should be 2, 8, 10, 16, or 36!");

  ثابت حرف *پیشوند = "";
  إذا (formatAsCLiteral) {
    بدل (Radix) {
      حالة 2:
        // Binary literals are a non-standard extension added in gcc 4.3:
        // http://gcc.gnu.org/onlinedocs/gcc-4.3.0/gcc/Binary-constants.html
        پیشوند = "0b";
        اكسر;
      حالة 8:
        پیشوند = "0";
        اكسر;
      حالة 10:
        اكسر; // No prefix
      حالة 16:
        پیشوند = "0x";
        اكسر;
      افتراضي:
        llvm_unreachable("Invalid radix!");
    }
  }

  // Number of digits in a group between separators.
  بیعلامت Grouping = (Radix == 8 || Radix == 10) ? 3 : 4;

  // First, check for a zero value and just short circuit the logic below.
  إذا (isZero()) {
    طالما (*پیشوند) {
      Str.افزودن_پشت(*پیشوند);
      ++پیشوند;
    };
    Str.افزودن_پشت('0');
    أرجع;
  }

  ساكن ثابت حرف BothDigits[] = "0123456789abcdefghijklmnopqrstuvwxyz"
                                   "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ";
  ثابت حرف *Digits = BothDigits + (فوقانی_Case_2 ? 36 : 0);

  إذا (LLVM_LIKELY(is_Single_کلمه())) {
    حرف میانگیر[65];
    حرف *BufPtr = std::انتها(میانگیر);

    uint64_t N;
    إذا (!Signed) {
      N = get_ZExt_مقدار();
    } وإلا {
      int64_t I = get_SExt_مقدار();
      إذا (I >= 0) {
        N = I;
      } وإلا {
        Str.افزودن_پشت('-');
        N = -(uint64_t)I;
      }
    }

    طالما (*پیشوند) {
      Str.افزودن_پشت(*پیشوند);
      ++پیشوند;
    };

    صحيح Pos = 0;
    طالما (N) {
      إذا (درج_Separators && Pos % Grouping == 0 && Pos > 0)
        *--BufPtr = '\'';
      *--BufPtr = Digits[N % Radix];
      N /= Radix;
      Pos++;
    }
    Str.append(BufPtr, std::انتها(میانگیر));
    أرجع;
  }

  APInt Tmp(*هذا);

  إذا (Signed && isNegative()) {
    // They want to print the signed version and it is a negative value
    // Flip the bits and add one to turn it into the equivalent positive
    // value and put a '-' in the result.
    Tmp.negate();
    Str.افزودن_پشت('-');
  }

  طالما (*پیشوند) {
    Str.افزودن_پشت(*پیشوند);
    ++پیشوند;
  }

  // We insert the digits backward, then reverse them to get the right order.
  بیعلامت شروع_Dig = Str.اندازه_2();

  // For the 2, 8 and 16 bit cases, we can just shift instead of divide
  // because the number of bits per digit (1, 3 and 4 respectively) divides
  // equally.  We just shift until the value is zero.
  إذا (Radix == 2 || Radix == 8 || Radix == 16) {
    // Just shift tmp right for each digit width until it becomes zero
    بیعلامت لغزش_Amt_2 = (Radix == 16 ? 4 : (Radix == 8 ? 3 : 1));
    بیعلامت ماسک_Amt = Radix - 1;

    صحيح Pos = 0;
    طالما (Tmp.get_Bool_مقدار()) {
      بیعلامت Digit = بیعلامت(Tmp.get_Raw_داده()[0]) & ماسک_Amt;
      إذا (درج_Separators && Pos % Grouping == 0 && Pos > 0)
        Str.افزودن_پشت('\'');

      Str.افزودن_پشت(Digits[Digit]);
      Tmp.lshr_درون_قراربده(لغزش_Amt_2);
      Pos++;
    }
  } وإلا {
    صحيح Pos = 0;
    طالما (Tmp.get_Bool_مقدار()) {
      uint64_t Digit;
      udivrem(Tmp, Radix, Tmp, Digit);
      أكد(Digit < Radix && "divide failed");
      إذا (درج_Separators && Pos % Grouping == 0 && Pos > 0)
        Str.افزودن_پشت('\'');

      Str.افزودن_پشت(Digits[Digit]);
      Pos++;
    }
  }

  // Reverse the digits before returning.
  std::معکوس(Str.begin()+شروع_Dig, Str.انتها());
}

#إذا !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)
LLVM_DUMP_روش فراغ APInt::dump() ثابت {
  SmallString<40> S, U;
  هذا->مقصد_String_Unsigned(U);
  هذا->مقصد_String_Signed(S);
  dbgs() << "APInt(" << بیت_عرض_2 << "b, "
         << U << "u " << S << "s)\n";
}
#پایان_اگر

فراغ APInt::print(raw_ostream &OS, منطقي isSigned) ثابت {
  SmallString<40> S;
  هذا->مقصد_String(S, 10, isSigned, /* formatAsCLiteral = */خطأ);
  OS << S;
}

// This implements a variety of operations on a representation of
// arbitrary precision, two's-complement, bignum integer values.

// Assumed by lowHalf, highHalf, partMSB and partLSB.  A fairly safe
// and unrestricting assumption.
static_assert(APInt::APINT_بیتها_در_هر_کلمه % 2 == 0,
              "Part width must be divisible by 2!");

// Returns the integer part with the least significant BITS set.
// BITS cannot be zero.
ساكن مضمن APInt::کلمه_Type کم_بیت_ماسک(بیعلامت بیتها) {
  أكد(بیتها != 0 && بیتها <= APInt::APINT_بیتها_در_هر_کلمه);
  أرجع ~(APInt::کلمه_Type) 0 >> (APInt::APINT_بیتها_در_هر_کلمه - بیتها);
}

/// Returns the value of the lower half of PART.
ساكن مضمن APInt::کلمه_Type کم_Half(APInt::کلمه_Type part) {
  أرجع part & کم_بیت_ماسک(APInt::APINT_بیتها_در_هر_کلمه / 2);
}

/// Returns the value of the upper half of PART.
ساكن مضمن APInt::کلمه_Type بالا_Half(APInt::کلمه_Type part) {
  أرجع part >> (APInt::APINT_بیتها_در_هر_کلمه / 2);
}

/// Sets the least significant part of a bignum to the input value, and zeroes
/// out higher parts.
فراغ APInt::tc_مجموعه(کلمه_Type *dst, کلمه_Type part, بیعلامت parts) {
  أكد(parts > 0);
  dst[0] = part;
  لكل (بیعلامت i = 1; i < parts; i++)
    dst[i] = 0;
}

/// Assign one bignum to another.
فراغ APInt::tc_تخصیص_بده(کلمه_Type *dst, ثابت کلمه_Type *src, بیعلامت parts) {
  لكل (بیعلامت i = 0; i < parts; i++)
    dst[i] = src[i];
}

/// Returns true if a bignum is zero, false otherwise.
منطقي APInt::tcIsZero(ثابت کلمه_Type *src, بیعلامت parts) {
  لكل (بیعلامت i = 0; i < parts; i++)
    إذا (src[i])
      أرجع خطأ;

  أرجع درست;
}

/// Extract the given bit of a bignum; returns 0 or 1.
صحيح APInt::tc_استخراج_بیت(ثابت کلمه_Type *parts, بیعلامت بیت) {
  أرجع (parts[which_کلمه(بیت)] & ماسک_بیت(بیت)) != 0;
}

/// Set the given bit of a bignum.
فراغ APInt::tc_مجموعه_بیت(کلمه_Type *parts, بیعلامت بیت) {
  parts[which_کلمه(بیت)] |= ماسک_بیت(بیت);
}

/// Clears the given bit of a bignum.
فراغ APInt::tc_Clear_بیت(کلمه_Type *parts, بیعلامت بیت) {
  parts[which_کلمه(بیت)] &= ~ماسک_بیت(بیت);
}

/// Returns the bit number of the least significant set bit of a number.  If the
/// input number has no bits set UINT_MAX is returned.
بیعلامت APInt::tcLSB(ثابت کلمه_Type *parts, بیعلامت n) {
  لكل (بیعلامت i = 0; i < n; i++) {
    إذا (parts[i] != 0) {
      بیعلامت lsb = llvm::countr_zero(parts[i]);
      أرجع lsb + i * APINT_بیتها_در_هر_کلمه;
    }
  }

  أرجع UINT_MAX;
}

/// Returns the bit number of the most significant set bit of a number.
/// If the input number has no bits set UINT_MAX is returned.
بیعلامت APInt::tcMSB(ثابت کلمه_Type *parts, بیعلامت n) {
  افعل {
    --n;

    إذا (parts[n] != 0) {
      static_assert(حجم(parts[n]) <= حجم(uint64_t));
      بیعلامت msb = llvm::Log2_64(parts[n]);

      أرجع msb + n * APINT_بیتها_در_هر_کلمه;
    }
  } طالما (n);

  أرجع UINT_MAX;
}

/// Copy the bit vector of width srcBITS from SRC, starting at bit srcLSB, to
/// DST, of dstCOUNT parts, such that the bit srcLSB becomes the least
/// significant bit of DST.  All high bits above srcBITS in DST are zero-filled.
/// */
فراغ
APInt::tc_استخراج(کلمه_Type *dst, بیعلامت dst_شمار, ثابت کلمه_Type *src,
                 بیعلامت src_بیتها, بیعلامت srcLSB) {
  بیعلامت dstParts = (src_بیتها + APINT_بیتها_در_هر_کلمه - 1) / APINT_بیتها_در_هر_کلمه;
  أكد(dstParts <= dst_شمار);

  بیعلامت اولی_Src_Part = srcLSB / APINT_بیتها_در_هر_کلمه;
  tc_تخصیص_بده(dst, src + اولی_Src_Part, dstParts);

  بیعلامت لغزش = srcLSB % APINT_بیتها_در_هر_کلمه;
  tc_لغزش_راست(dst, dstParts, لغزش);

  // We now have (dstParts * APINT_BITS_PER_WORD - shift) bits from SRC
  // in DST.  If this is less that srcBits, append the rest, else
  // clear the high bits.
  بیعلامت n = dstParts * APINT_بیتها_در_هر_کلمه - لغزش;
  إذا (n < src_بیتها) {
    کلمه_Type ماسک_2 = کم_بیت_ماسک (src_بیتها - n);
    dst[dstParts - 1] |= ((src[اولی_Src_Part + dstParts] & ماسک_2)
                          << n % APINT_بیتها_در_هر_کلمه);
  } وإلا إذا (n > src_بیتها) {
    إذا (src_بیتها % APINT_بیتها_در_هر_کلمه)
      dst[dstParts - 1] &= کم_بیت_ماسک (src_بیتها % APINT_بیتها_در_هر_کلمه);
  }

  // Clear high parts.
  طالما (dstParts < dst_شمار)
    dst[dstParts++] = 0;
}

//// DST += RHS + C where C is zero or one.  Returns the carry flag.
APInt::کلمه_Type APInt::tc_جمع_کن(کلمه_Type *dst, ثابت کلمه_Type *rhs,
                             کلمه_Type c, بیعلامت parts) {
  أكد(c <= 1);

  لكل (بیعلامت i = 0; i < parts; i++) {
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
APInt::کلمه_Type APInt::tc_جمع_کن_Part(کلمه_Type *dst, کلمه_Type src,
                                 بیعلامت parts) {
  لكل (بیعلامت i = 0; i < parts; ++i) {
    dst[i] += src;
    إذا (dst[i] >= src)
      أرجع 0; // No need to carry so exit early.
    src = 1; // Carry one to next digit.
  }

  أرجع 1;
}

/// DST -= RHS + C where C is zero or one.  Returns the carry flag.
APInt::کلمه_Type APInt::tc_تفریق(کلمه_Type *dst, ثابت کلمه_Type *rhs,
                                  کلمه_Type c, بیعلامت parts) {
  أكد(c <= 1);

  لكل (بیعلامت i = 0; i < parts; i++) {
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
APInt::کلمه_Type APInt::tc_تفریق_Part(کلمه_Type *dst, کلمه_Type src,
                                      بیعلامت parts) {
  لكل (بیعلامت i = 0; i < parts; ++i) {
    کلمه_Type Dst = dst[i];
    dst[i] -= src;
    إذا (src <= Dst)
      أرجع 0; // No need to borrow so exit early.
    src = 1; // We have to "borrow 1" from next "word"
  }

  أرجع 1;
}

/// Negate a bignum in-place.
فراغ APInt::tcNegate(کلمه_Type *dst, بیعلامت parts) {
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
                          کلمه_Type multiplier, کلمه_Type انتقال,
                          بیعلامت srcParts, بیعلامت dstParts,
                          منطقي جمع_کن) {
  // Otherwise our writes of DST kill our later reads of SRC.
  أكد(dst <= src || dst >= src + srcParts);
  أكد(dstParts <= srcParts + 1);

  // N loops; minimum of dstParts and srcParts.
  بیعلامت n = std::min(dstParts, srcParts);

  لكل (بیعلامت i = 0; i < n; i++) {
    // [LOW, HIGH] = MULTIPLIER * SRC[i] + DST[i] + CARRY.
    // This cannot overflow, because:
    //   (n - 1) * (n - 1) + 2 (n - 1) = (n - 1) * (n + 1)
    // which is less than n^2.
    کلمه_Type srcPart = src[i];
    کلمه_Type کم, mid, بالا;
    إذا (multiplier == 0 || srcPart == 0) {
      کم = انتقال;
      بالا = 0;
    } وإلا {
      کم = کم_Half(srcPart) * کم_Half(multiplier);
      بالا = بالا_Half(srcPart) * بالا_Half(multiplier);

      mid = کم_Half(srcPart) * بالا_Half(multiplier);
      بالا += بالا_Half(mid);
      mid <<= APINT_بیتها_در_هر_کلمه / 2;
      إذا (کم + mid < کم)
        بالا++;
      کم += mid;

      mid = بالا_Half(srcPart) * کم_Half(multiplier);
      بالا += بالا_Half(mid);
      mid <<= APINT_بیتها_در_هر_کلمه / 2;
      إذا (کم + mid < کم)
        بالا++;
      کم += mid;

      // Now add carry.
      إذا (کم + انتقال < کم)
        بالا++;
      کم += انتقال;
    }

    إذا (جمع_کن) {
      // And now DST[i], and store the new low part there.
      إذا (کم + dst[i] < کم)
        بالا++;
      dst[i] += کم;
    } وإلا {
      dst[i] = کم;
    }

    انتقال = بالا;
  }

  إذا (srcParts < dstParts) {
    // Full multiplication, there is no overflow.
    أكد(srcParts + 1 == dstParts);
    dst[srcParts] = انتقال;
    أرجع 0;
  }

  // We overflowed if there is carry.
  إذا (انتقال)
    أرجع 1;

  // We would overflow if any significant unwritten parts would be
  // non-zero.  This is true if any remaining src parts are non-zero
  // and the multiplier is non-zero.
  إذا (multiplier)
    لكل (بیعلامت i = dstParts; i < srcParts; i++)
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
                      ثابت کلمه_Type *rhs, بیعلامت parts) {
  أكد(dst != lhs && dst != rhs);

  صحيح سرریز_2 = 0;

  لكل (بیعلامت i = 0; i < parts; i++) {
    // Don't accumulate on the first iteration so we don't need to initalize
    // dst to 0.
    سرریز_2 |=
        tcMultiplyPart(&dst[i], lhs, rhs[i], 0, parts, parts - i, i != 0);
  }

  أرجع سرریز_2;
}

/// DST = LHS * RHS, where DST has width the sum of the widths of the
/// operands. No overflow occurs. DST must be disjoint from both operands.
فراغ APInt::tc_پر_Multiply(کلمه_Type *dst, ثابت کلمه_Type *lhs,
                           ثابت کلمه_Type *rhs, بیعلامت lhsParts,
                           بیعلامت rhsParts) {
  // Put the narrower number on the LHS for less loops below.
  إذا (lhsParts > rhsParts)
    أرجع tc_پر_Multiply (dst, rhs, lhs, rhsParts, lhsParts);

  أكد(dst != lhs && dst != rhs);

  لكل (بیعلامت i = 0; i < lhsParts; i++) {
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
                    بیعلامت parts) {
  أكد(lhs != remainder && lhs != srhs && remainder != srhs);

  بیعلامت لغزش_شمار = tcMSB(rhs, parts) + 1;
  إذا (لغزش_شمار == 0)
    أرجع درست;

  لغزش_شمار = parts * APINT_بیتها_در_هر_کلمه - لغزش_شمار;
  بیعلامت n = لغزش_شمار / APINT_بیتها_در_هر_کلمه;
  کلمه_Type ماسک_2 = (کلمه_Type) 1 << (لغزش_شمار % APINT_بیتها_در_هر_کلمه);

  tc_تخصیص_بده(srhs, rhs, parts);
  tc_لغزش_چپ(srhs, parts, لغزش_شمار);
  tc_تخصیص_بده(remainder, lhs, parts);
  tc_مجموعه(lhs, 0, parts);

  // Loop, subtracting SRHS if REMAINDER is greater and adding that to the
  // total.
  لكل (;;) {
    صحيح compare = tcCompare(remainder, srhs, parts);
    إذا (compare >= 0) {
      tc_تفریق(remainder, srhs, 0, parts);
      lhs[n] |= ماسک_2;
    }

    إذا (لغزش_شمار == 0)
      اكسر;
    لغزش_شمار--;
    tc_لغزش_راست(srhs, parts, 1);
    إذا ((ماسک_2 >>= 1) == 0) {
      ماسک_2 = (کلمه_Type) 1 << (APINT_بیتها_در_هر_کلمه - 1);
      n--;
    }
  }

  أرجع خطأ;
}

/// Shift a bignum left Count bits in-place. Shifted in bits are zero. There are
/// no restrictions on Count.
فراغ APInt::tc_لغزش_چپ(کلمه_Type *Dst, بیعلامت واژگان_2, بیعلامت شمار_2) {
  // Don't bother performing a no-op shift.
  إذا (!شمار_2)
    أرجع;

  // WordShift is the inter-part shift; BitShift is the intra-part shift.
  بیعلامت کلمه_لغزش = std::min(شمار_2 / APINT_بیتها_در_هر_کلمه, واژگان_2);
  بیعلامت بیت_لغزش = شمار_2 % APINT_بیتها_در_هر_کلمه;

  // Fastpath for moving by whole words.
  إذا (بیت_لغزش == 0) {
    std::memmove(Dst + کلمه_لغزش, Dst, (واژگان_2 - کلمه_لغزش) * APINT_کلمه_اندازه);
  } وإلا {
    طالما (واژگان_2-- > کلمه_لغزش) {
      Dst[واژگان_2] = Dst[واژگان_2 - کلمه_لغزش] << بیت_لغزش;
      إذا (واژگان_2 > کلمه_لغزش)
        Dst[واژگان_2] |=
          Dst[واژگان_2 - کلمه_لغزش - 1] >> (APINT_بیتها_در_هر_کلمه - بیت_لغزش);
    }
  }

  // Fill in the remainder with 0s.
  std::memset(Dst, 0, کلمه_لغزش * APINT_کلمه_اندازه);
}

/// Shift a bignum right Count bits in-place. Shifted in bits are zero. There
/// are no restrictions on Count.
فراغ APInt::tc_لغزش_راست(کلمه_Type *Dst, بیعلامت واژگان_2, بیعلامت شمار_2) {
  // Don't bother performing a no-op shift.
  إذا (!شمار_2)
    أرجع;

  // WordShift is the inter-part shift; BitShift is the intra-part shift.
  بیعلامت کلمه_لغزش = std::min(شمار_2 / APINT_بیتها_در_هر_کلمه, واژگان_2);
  بیعلامت بیت_لغزش = شمار_2 % APINT_بیتها_در_هر_کلمه;

  بیعلامت واژگان_مقصد_Move = واژگان_2 - کلمه_لغزش;
  // Fastpath for moving by whole words.
  إذا (بیت_لغزش == 0) {
    std::memmove(Dst, Dst + کلمه_لغزش, واژگان_مقصد_Move * APINT_کلمه_اندازه);
  } وإلا {
    لكل (بیعلامت i = 0; i != واژگان_مقصد_Move; ++i) {
      Dst[i] = Dst[i + کلمه_لغزش] >> بیت_لغزش;
      إذا (i + 1 != واژگان_مقصد_Move)
        Dst[i] |= Dst[i + کلمه_لغزش + 1] << (APINT_بیتها_در_هر_کلمه - بیت_لغزش);
    }
  }

  // Fill in the remainder with 0s.
  std::memset(Dst + واژگان_مقصد_Move, 0, کلمه_لغزش * APINT_کلمه_اندازه);
}

// Comparison (unsigned) of two bignums.
صحيح APInt::tcCompare(ثابت کلمه_Type *lhs, ثابت کلمه_Type *rhs,
                     بیعلامت parts) {
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
  حالة APInt::Rounding::پایین:
  حالة APInt::Rounding::TOWARD_ZERO:
    أرجع A.udiv(B);
  حالة APInt::Rounding::به_بالا: {
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
  حالة APInt::Rounding::پایین:
  حالة APInt::Rounding::به_بالا: {
    APInt Quo, Rem;
    APInt::sdivrem(A, B, Quo, Rem);
    إذا (Rem.isZero())
      أرجع Quo;
    // This algorithm deals with arbitrary rounding mode used by sdivrem.
    // We want to check whether the non-integer part of the mathematical value
    // is negative or not. If the non-integer part is negative, we need to round
    // down from Quo; otherwise, if it's positive or 0, we return Quo, as it's
    // already rounded down.
    إذا (RM == APInt::Rounding::پایین) {
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
                                           بیعلامت Range_عرض) {
  بیعلامت Coeff_عرض = A.get_بیت_عرض();
  أكد(Coeff_عرض == B.get_بیت_عرض() && Coeff_عرض == C.get_بیت_عرض());
  أكد(Range_عرض <= Coeff_عرض &&
         "Value range width should be less than coefficient width");
  أكد(Range_عرض > 1 && "Value range bit width should be > 1");

  LLVM_DEBUG(dbgs() << __func__ << ": solving " << A << "x^2 + " << B
                    << "x + " << C << ", rw:" << Range_عرض << '\n');

  // Identify 0 as a (non)solution immediately.
  إذا (C.sextOrTrunc(Range_عرض).isZero()) {
    LLVM_DEBUG(dbgs() << __func__ << ": zero solution\n");
    أرجع APInt(Coeff_عرض, 0);
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
  Coeff_عرض *= 3;
  A = A.sext(Coeff_عرض);
  B = B.sext(Coeff_عرض);
  C = C.sext(Coeff_عرض);

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
  APInt R = APInt::get_One_بیت_مجموعه(Coeff_عرض, Range_عرض);
  APInt TwoA = 2 * A;
  APInt SqrB = B * B;
  منطقي Pick_کم;

  خودکار دور_به_بالا_2 = [] (ثابت APInt &V, ثابت APInt &A) -> APInt {
    أكد(A.is_Strictly_مثبت());
    APInt T = V.قدر_مطلق().urem(A);
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
    LowkR = دور_به_بالا_2(LowkR, R);

    // If there exists k meeting the condition above, and such that
    // C-kR > 0, there will be two positive real number solutions of
    // q(x) = kR. Out of all such values of k, pick the one that makes
    // C-kR closest to 0, (i.e. pick maximum k such that C-kR > 0).
    // In other words, find maximum k such that LowkR <= kR < C.
    إذا (C.sgt(LowkR)) {
      // If LowkR < C, then such a k is guaranteed to exist because
      // LowkR itself is a multiple of R.
      C -= -دور_به_بالا_2(-C, R);      // C = C - RoundDown(C, R)
      // Pick the smaller solution.
      Pick_کم = درست;
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
                    << B << "x + " << C << ", rw:" << Range_عرض << '\n');

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

std::optional<بیعلامت>
llvm::APIntOps::Get_Most_Significant_Different_بیت(ثابت APInt &A, ثابت APInt &B) {
  أكد(A.get_بیت_عرض() == B.get_بیت_عرض() && "Must have the same bitwidth");
  إذا (A == B)
    أرجع std::nullopt;
  أرجع A.get_بیت_عرض() - ((A ^ B).countl_zero() + 1);
}

APInt llvm::APIntOps::Scale_بیت_ماسک(ثابت APInt &A, بیعلامت New_بیت_عرض,
                                   منطقي تطابق_All_بیتها) {
  بیعلامت Old_بیت_عرض = A.get_بیت_عرض();
  أكد((((Old_بیت_عرض % New_بیت_عرض) == 0) ||
          ((New_بیت_عرض % Old_بیت_عرض) == 0)) &&
         "One size should be a multiple of the other one. "
         "Can't do fractional scaling.");

  // Check for matching bitwidths.
  إذا (Old_بیت_عرض == New_بیت_عرض)
    أرجع A;

  APInt NewA = APInt::getZero(New_بیت_عرض);

  // Check for null input.
  إذا (A.isZero())
    أرجع NewA;

  إذا (New_بیت_عرض > Old_بیت_عرض) {
    // Repeat bits.
    بیعلامت Scale = New_بیت_عرض / Old_بیت_عرض;
    لكل (بیعلامت i = 0; i != Old_بیت_عرض; ++i)
      إذا (A[i])
        NewA.مجموعه_بیتها(i * Scale, (i + 1) * Scale);
  } وإلا {
    بیعلامت Scale = Old_بیت_عرض / New_بیت_عرض;
    لكل (بیعلامت i = 0; i != New_بیت_عرض; ++i) {
      إذا (تطابق_All_بیتها) {
        إذا (A.استخراج_بیتها(Scale, i * Scale).isAllOnes())
          NewA.مجموعه_بیت(i);
      } وإلا {
        إذا (!A.استخراج_بیتها(Scale, i * Scale).isZero())
          NewA.مجموعه_بیت(i);
      }
    }
  }

  أرجع NewA;
}

/// StoreIntToMemory - Fills the StoreBytes bytes of memory starting from Dst
/// with the integer held in IntVal.
فراغ llvm::Store_Int_مقصد_حافظه(ثابت APInt &IntVal, uint8_t *Dst,
                            بیعلامت Store_بایتها) {
  أكد((IntVal.get_بیت_عرض()+7)/8 >= Store_بایتها && "Integer too small!");
  ثابت uint8_t *Src = (ثابت uint8_t *)IntVal.get_Raw_داده();

  إذا (sys::IsLittleEndianHost) {
    // Little-endian host - the source is ordered from LSB to MSB.  Order the
    // destination from LSB to MSB: Do a straight copy.
    کپی_حافظه(Dst, Src, Store_بایتها);
  } وإلا {
    // Big-endian host - the source is an array of 64 bit words ordered from
    // LSW to MSW.  Each word is ordered from MSB to LSB.  Order the destination
    // from MSB to LSB: Reverse the word order, but not the bytes in a word.
    طالما (Store_بایتها > حجم(uint64_t)) {
      Store_بایتها -= حجم(uint64_t);
      // May not be aligned so use memcpy.
      کپی_حافظه(Dst + Store_بایتها, Src, حجم(uint64_t));
      Src += حجم(uint64_t);
    }

    کپی_حافظه(Dst, Src + حجم(uint64_t) - Store_بایتها, Store_بایتها);
  }
}

/// LoadIntFromMemory - Loads the integer stored in the LoadBytes bytes starting
/// from Src into IntVal, which is assumed to be wide enough and to hold zero.
فراغ llvm::بارگیری_Int_From_حافظه(APInt &IntVal, ثابت uint8_t *Src,
                             بیعلامت بارگیری_بایتها) {
  أكد((IntVal.get_بیت_عرض()+7)/8 >= بارگیری_بایتها && "Integer too small!");
  uint8_t *Dst = reinterpret_cast<uint8_t *>(
                   const_cast<uint64_t *>(IntVal.get_Raw_داده()));

  إذا (sys::IsLittleEndianHost)
    // Little-endian host - the destination must be ordered from LSB to MSB.
    // The source is ordered from LSB to MSB: Do a straight copy.
    کپی_حافظه(Dst, Src, بارگیری_بایتها);
  وإلا {
    // Big-endian - the destination is an array of 64 bit words ordered from
    // LSW to MSW.  Each word must be ordered from MSB to LSB.  The source is
    // ordered from MSB to LSB: Reverse the word order, but not the bytes in
    // a word.
    طالما (بارگیری_بایتها > حجم(uint64_t)) {
      بارگیری_بایتها -= حجم(uint64_t);
      // May not be aligned so use memcpy.
      کپی_حافظه(Dst, Src + بارگیری_بایتها, حجم(uint64_t));
      Dst += حجم(uint64_t);
    }

    کپی_حافظه(Dst + حجم(uint64_t) - بارگیری_بایتها, Src, بارگیری_بایتها);
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
  أكد(C1.get_بیت_عرض() == C2.get_بیت_عرض() && "Unequal bitwidths");
  بیعلامت پر_عرض = C1.get_بیت_عرض() * 2;
  APInt C1Ext = C1.sext(پر_عرض);
  APInt C2Ext = C2.sext(پر_عرض);
  أرجع (C1Ext * C2Ext).استخراج_بیتها(C1.get_بیت_عرض(), C1.get_بیت_عرض());
}

APInt APIntOps::mulhu(ثابت APInt &C1, ثابت APInt &C2) {
  أكد(C1.get_بیت_عرض() == C2.get_بیت_عرض() && "Unequal bitwidths");
  بیعلامت پر_عرض = C1.get_بیت_عرض() * 2;
  APInt C1Ext = C1.zext(پر_عرض);
  APInt C2Ext = C2.zext(پر_عرض);
  أرجع (C1Ext * C2Ext).استخراج_بیتها(C1.get_بیت_عرض(), C1.get_بیت_عرض());
}

APInt APIntOps::mulsExtended(ثابت APInt &C1, ثابت APInt &C2) {
  أكد(C1.get_بیت_عرض() == C2.get_بیت_عرض() && "Unequal bitwidths");
  بیعلامت پر_عرض = C1.get_بیت_عرض() * 2;
  APInt C1Ext = C1.sext(پر_عرض);
  APInt C2Ext = C2.sext(پر_عرض);
  أرجع C1Ext * C2Ext;
}

APInt APIntOps::muluExtended(ثابت APInt &C1, ثابت APInt &C2) {
  أكد(C1.get_بیت_عرض() == C2.get_بیت_عرض() && "Unequal bitwidths");
  بیعلامت پر_عرض = C1.get_بیت_عرض() * 2;
  APInt C1Ext = C1.zext(پر_عرض);
  APInt C2Ext = C2.zext(پر_عرض);
  أرجع C1Ext * C2Ext;
}

APInt APIntOps::توان(ثابت APInt &X, int64_t N) {
  أكد(N >= 0 && "negative exponents not supported.");
  APInt Acc = APInt(X.get_بیت_عرض(), 1);
  إذا (N == 0)
    أرجع Acc;
  APInt پایه = X;
  int64_t مانده_Exponent = N;
  طالما (مانده_Exponent > 0) {
    طالما (مانده_Exponent % 2 == 0) {
      پایه *= پایه;
      مانده_Exponent /= 2;
    }
    --مانده_Exponent;
    Acc *= پایه;
  }
  أرجع Acc;
}

APInt llvm::APIntOps::fshl(ثابت APInt &Hi, ثابت APInt &Lo,
                           ثابت APInt &لغزش_2) {
  أكد(Hi.get_بیت_عرض() == Lo.get_بیت_عرض());
  بیعلامت لغزش_Amt_2 = rotateModulo(Hi.get_بیت_عرض(), لغزش_2);
  إذا (لغزش_Amt_2 == 0)
    أرجع Hi;
  أرجع Hi.shl(لغزش_Amt_2) | Lo.lshr(Hi.get_بیت_عرض() - لغزش_Amt_2);
}

APInt llvm::APIntOps::fshr(ثابت APInt &Hi, ثابت APInt &Lo,
                           ثابت APInt &لغزش_2) {
  أكد(Hi.get_بیت_عرض() == Lo.get_بیت_عرض());
  بیعلامت لغزش_Amt_2 = rotateModulo(Hi.get_بیت_عرض(), لغزش_2);
  إذا (لغزش_Amt_2 == 0)
    أرجع Lo;
  أرجع Hi.shl(Hi.get_بیت_عرض() - لغزش_Amt_2) | Lo.lshr(لغزش_Amt_2);
}

APInt llvm::APIntOps::clmul(ثابت APInt &LHS, ثابت APInt &RHS) {
  بیعلامت BW = LHS.get_بیت_عرض();
  أكد(BW == RHS.get_بیت_عرض() && "Operand mismatch");
  APInt نتیجه(BW, 0);
  لكل (بیعلامت I : seq(std::min(RHS.get_فعال_بیتها(), BW - LHS.countr_zero())))
    إذا (RHS[I])
      نتیجه ^= LHS << I;
  أرجع نتیجه;
}

APInt llvm::APIntOps::clmulr(ثابت APInt &LHS, ثابت APInt &RHS) {
  أكد(LHS.get_بیت_عرض() == RHS.get_بیت_عرض());
  أرجع clmul(LHS.معکوس_بیتها(), RHS.معکوس_بیتها()).معکوس_بیتها();
}

APInt llvm::APIntOps::clmulh(ثابت APInt &LHS, ثابت APInt &RHS) {
  أكد(LHS.get_بیت_عرض() == RHS.get_بیت_عرض());
  أرجع clmulr(LHS, RHS).lshr(1);
}

APInt llvm::APIntOps::pext(ثابت APInt &Val, ثابت APInt &ماسک) {
  بیعلامت BW = Val.get_بیت_عرض();
  أكد(BW == ماسک.get_بیت_عرض() && "Operand mismatch");
  APInt نتیجه = APInt::getZero(BW);
  لكل (بیعلامت I = 0, P = 0; I != BW; ++I)
    إذا (ماسک[I])
      نتیجه.مجموعه_بیت_Val(P++, Val[I]);
  أرجع نتیجه;
}

APInt llvm::APIntOps::pdep(ثابت APInt &Val, ثابت APInt &ماسک) {
  بیعلامت BW = Val.get_بیت_عرض();
  أكد(BW == ماسک.get_بیت_عرض() && "Operand mismatch");
  APInt نتیجه = APInt::getZero(BW);
  لكل (بیعلامت I = 0, P = 0; I != BW; ++I)
    إذا (ماسک[I])
      نتیجه.مجموعه_بیت_Val(I, Val[P++]);
  أرجع نتیجه;
}
