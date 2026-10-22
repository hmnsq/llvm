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

#అన్తర్భావ "llvm/ADT/APInt.h"
#అన్తర్భావ "llvm/ADT/ArrayRef.h"
#అన్తర్భావ "llvm/ADT/FoldingSet.h"
#అన్తర్భావ "llvm/ADT/Hashing.h"
#అన్తర్భావ "llvm/ADT/Sequence.h"
#అన్తర్భావ "llvm/ADT/SmallString.h"
#అన్తర్భావ "llvm/ADT/StringRef.h"
#అన్తర్భావ "llvm/ADT/bit.h"
#అన్తర్భావ "llvm/Support/Alignment.h"
#అన్తర్భావ "llvm/Support/Debug.h"
#అన్తర్భావ "llvm/Support/ErrorHandling.h"
#అన్తర్భావ "llvm/Support/MathExtras.h"
#అన్తర్భావ "llvm/Support/SwapByteOrder.h"
#అన్తర్భావ "llvm/Support/raw_ostream.h"
#అన్తర్భావ <cmath>
#అన్తర్భావ <optional>

ఉపయోగించి నామాకాశ llvm;

#నిర్వచించు DEBUG_TYPE "apint"

/// A utility function for allocating memory, checking for allocation failures,
/// and ensuring the contents are zeroed.
అంతఃవరుస స్థిర uint64_t* get_Cleared_జ్ఞాపకం(గుర్తులేని అంకె_పదాలు) {
  తిరిగి కొత్త uint64_t[అంకె_పదాలు]();
}

/// A utility function for allocating memory and checking for allocation
/// failure.  The content is not zeroed.
అంతఃవరుస స్థిర uint64_t* get_జ్ఞాపకం(గుర్తులేని అంకె_పదాలు) {
  తిరిగి కొత్త uint64_t[అంకె_పదాలు];
}

/// A utility function that converts a character to a digit.
అంతఃవరుస స్థిర గుర్తులేని getDigit(అక్షరం cdigit, uint8_t radix) {
  గుర్తులేని r;

  ఒకవేళ (radix == 16 || radix == 36) {
    r = cdigit - '0';
    ఒకవేళ (r <= 9)
      తిరిగి r;

    r = cdigit - 'A';
    ఒకవేళ (r <= radix - 11U)
      తిరిగి r + 10;

    r = cdigit - 'a';
    ఒకవేళ (r <= radix - 11U)
      తిరిగి r + 10;

    radix = 10;
  }

  r = cdigit - '0';
  ఒకవేళ (r < radix)
    తిరిగి r;

  తిరిగి UINT_MAX;
}


శూన్య APInt::initSlowCase(uint64_t val, తార్కిక isSigned) {
  ఒకవేళ (isSigned && int64_t(val) < 0) {
    U.pVal = get_జ్ఞాపకం(get_అంకె_పదాలు());
    U.pVal[0] = val;
    memset(&U.pVal[1], 0xFF, APINT_మాట_కొలత * (get_అంకె_పదాలు() - 1));
    clear_వాడని_బిట్లు();
  } లేకపోతే {
    U.pVal = get_Cleared_జ్ఞాపకం(get_అంకె_పదాలు());
    U.pVal[0] = val;
  }
}

శూన్య APInt::initSlowCase(స్థిరరాశి APInt& that) {
  U.pVal = get_జ్ఞాపకం(get_అంకె_పదాలు());
  జ్ఞాపకం_నకలు(U.pVal, that.U.pVal, get_అంకె_పదాలు() * APINT_మాట_కొలత);
}

శూన్య APInt::initFromArray(ArrayRef<uint64_t> bigVal) {
  ప్రతిజ్ఞా(bigVal.దత్తాంశం() && "Null pointer detected!");
  ఒకవేళ (LLVM_LIKELY(is_Single_మాట()))
    U.VAL = bigVal[0];
  లేకపోతే {
    // Get memory, cleared to 0
    U.pVal = get_Cleared_జ్ఞాపకం(get_అంకె_పదాలు());
    // Calculate the number of words to copy
    గుర్తులేని పదాలు = std::min<గుర్తులేని>(bigVal.కొలత_2(), get_అంకె_పదాలు());
    // Copy the words from bigVal to pVal
    జ్ఞాపకం_నకలు(U.pVal, bigVal.దత్తాంశం(), పదాలు * APINT_మాట_కొలత);
  }
  // Make sure unused high bits are cleared
  clear_వాడని_బిట్లు();
}

APInt::APInt(గుర్తులేని అంకె_బిట్లు, ArrayRef<uint64_t> bigVal) : బిట్టు_వెడల్పు_2(అంకె_బిట్లు) {
  initFromArray(bigVal);
}

APInt::APInt(గుర్తులేని numbits, StringRef Str, uint8_t radix)
    : బిట్టు_వెడల్పు_2(numbits) {
  fromString(numbits, Str, radix);
}

శూన్య APInt::reallocate(గుర్తులేని New_బిట్టు_వెడల్పు) {
  // If the number of words is the same we can just change the width and stop.
  ఒకవేళ (get_అంకె_పదాలు() == get_అంకె_పదాలు(New_బిట్టు_వెడల్పు)) {
    బిట్టు_వెడల్పు_2 = New_బిట్టు_వెడల్పు;
    తిరిగి;
  }

  // If we have an allocation, delete it.
  ఒకవేళ (LLVM_UNLIKELY(!is_Single_మాట()))
    తొలగించు [] U.pVal;

  // Update BitWidth.
  బిట్టు_వెడల్పు_2 = New_బిట్టు_వెడల్పు;

  // If we are supposed to have an allocation, create it.
  ఒకవేళ (LLVM_UNLIKELY(!is_Single_మాట()))
    U.pVal = get_జ్ఞాపకం(get_అంకె_పదాలు());
}

శూన్య APInt::కేటాయించు_Slow_Case(స్థిరరాశి APInt &RHS) {
  // Don't do anything for X = X
  ఒకవేళ (ఇది == &RHS)
    తిరిగి;

  // Adjust the bit width and handle allocations as necessary.
  reallocate(RHS.get_బిట్టు_వెడల్పు());

  // Copy the data.
  ఒకవేళ (LLVM_LIKELY(is_Single_మాట()))
    U.VAL = RHS.U.VAL;
  లేకపోతే
    జ్ఞాపకం_నకలు(U.pVal, RHS.U.pVal, get_అంకె_పదాలు() * APINT_మాట_కొలత);
}

/// This method 'profiles' an APInt for use with FoldingSet.
శూన్య APInt::Profile(Folding_సమితి_కణుపు_ID& ID) స్థిరరాశి {
  ID.కూడు_Integer(బిట్టు_వెడల్పు_2);

  ఒకవేళ (LLVM_LIKELY(is_Single_మాట())) {
    ID.కూడు_Integer(U.VAL);
    తిరిగి;
  }

  గుర్తులేని అంకె_పదాలు_2 = get_అంకె_పదాలు();
  కొరకు (గుర్తులేని i = 0; i < అంకె_పదాలు_2; ++i)
    ID.కూడు_Integer(U.pVal[i]);
}

తార్కిక APInt::isAligned(Align A) స్థిరరాశి {
  ఒకవేళ (isZero())
    తిరిగి నిజం;
  స్థిరరాశి గుర్తులేని TrailingZeroes = countr_zero();
  స్థిరరాశి గుర్తులేని MinimumTrailingZeroes = Log2(A);
  తిరిగి TrailingZeroes >= MinimumTrailingZeroes;
}

/// Prefix increment operator. Increments the APInt by one.
APInt& APInt::క్రియాకారకం++() {
  ఒకవేళ (LLVM_LIKELY(is_Single_మాట()))
    ++U.VAL;
  లేకపోతే
    tcIncrement(U.pVal, get_అంకె_పదాలు());
  తిరిగి clear_వాడని_బిట్లు();
}

/// Prefix decrement operator. Decrements the APInt by one.
APInt& APInt::క్రియాకారకం--() {
  ఒకవేళ (LLVM_LIKELY(is_Single_మాట()))
    --U.VAL;
  లేకపోతే
    tcDecrement(U.pVal, get_అంకె_పదాలు());
  తిరిగి clear_వాడని_బిట్లు();
}

/// Adds the RHS APInt to this APInt.
/// @returns this, after addition of RHS.
/// Addition assignment operator.
APInt& APInt::క్రియాకారకం+=(స్థిరరాశి APInt& RHS) {
  ప్రతిజ్ఞా(బిట్టు_వెడల్పు_2 == RHS.బిట్టు_వెడల్పు_2 && "Bit widths must be the same");
  ఒకవేళ (LLVM_LIKELY(is_Single_మాట()))
    U.VAL += RHS.U.VAL;
  లేకపోతే
    tc_కూడు(U.pVal, RHS.U.pVal, 0, get_అంకె_పదాలు());
  తిరిగి clear_వాడని_బిట్లు();
}

APInt& APInt::క్రియాకారకం+=(uint64_t RHS) {
  ఒకవేళ (LLVM_LIKELY(is_Single_మాట()))
    U.VAL += RHS;
  లేకపోతే
    tc_కూడు_Part(U.pVal, RHS, get_అంకె_పదాలు());
  తిరిగి clear_వాడని_బిట్లు();
}

/// Subtracts the RHS APInt from this APInt
/// @returns this, after subtraction
/// Subtraction assignment operator.
APInt& APInt::క్రియాకారకం-=(స్థిరరాశి APInt& RHS) {
  ప్రతిజ్ఞా(బిట్టు_వెడల్పు_2 == RHS.బిట్టు_వెడల్పు_2 && "Bit widths must be the same");
  ఒకవేళ (LLVM_LIKELY(is_Single_మాట()))
    U.VAL -= RHS.U.VAL;
  లేకపోతే
    tc_వ్యవకలనం(U.pVal, RHS.U.pVal, 0, get_అంకె_పదాలు());
  తిరిగి clear_వాడని_బిట్లు();
}

APInt& APInt::క్రియాకారకం-=(uint64_t RHS) {
  ఒకవేళ (LLVM_LIKELY(is_Single_మాట()))
    U.VAL -= RHS;
  లేకపోతే
    tc_వ్యవకలనం_Part(U.pVal, RHS, get_అంకె_పదాలు());
  తిరిగి clear_వాడని_బిట్లు();
}

APInt APInt::క్రియాకారకం*(స్థిరరాశి APInt& RHS) స్థిరరాశి {
  ప్రతిజ్ఞా(బిట్టు_వెడల్పు_2 == RHS.బిట్టు_వెడల్పు_2 && "Bit widths must be the same");
  ఒకవేళ (LLVM_LIKELY(is_Single_మాట()))
    తిరిగి APInt(బిట్టు_వెడల్పు_2, U.VAL * RHS.U.VAL, /*isSigned=*/అబద్ధం,
                 /*implicitTrunc=*/నిజం);

  APInt ఫలితం(get_జ్ఞాపకం(get_అంకె_పదాలు()), get_బిట్టు_వెడల్పు());
  tcMultiply(ఫలితం.U.pVal, U.pVal, RHS.U.pVal, get_అంకె_పదాలు());
  ఫలితం.clear_వాడని_బిట్లు();
  తిరిగి ఫలితం;
}

శూన్య APInt::and_కేటాయించు_Slow_Case(స్థిరరాశి APInt &RHS) {
  మాట_Type *dst = U.pVal, *rhs = RHS.U.pVal;
  కొరకు (size_t i = 0, e = get_అంకె_పదాలు(); i != e; ++i)
    dst[i] &= rhs[i];
}

శూన్య APInt::or_కేటాయించు_Slow_Case(స్థిరరాశి APInt &RHS) {
  మాట_Type *dst = U.pVal, *rhs = RHS.U.pVal;
  కొరకు (size_t i = 0, e = get_అంకె_పదాలు(); i != e; ++i)
    dst[i] |= rhs[i];
}

శూన్య APInt::xor_కేటాయించు_Slow_Case(స్థిరరాశి APInt &RHS) {
  మాట_Type *dst = U.pVal, *rhs = RHS.U.pVal;
  కొరకు (size_t i = 0, e = get_అంకె_పదాలు(); i != e; ++i)
    dst[i] ^= rhs[i];
}

APInt &APInt::క్రియాకారకం*=(స్థిరరాశి APInt &RHS) {
  *ఇది = *ఇది * RHS;
  తిరిగి *ఇది;
}

APInt& APInt::క్రియాకారకం*=(uint64_t RHS) {
  ఒకవేళ (LLVM_LIKELY(is_Single_మాట())) {
    U.VAL *= RHS;
  } లేకపోతే {
    గుర్తులేని అంకె_పదాలు_2 = get_అంకె_పదాలు();
    tcMultiplyPart(U.pVal, U.pVal, RHS, 0, అంకె_పదాలు_2, అంకె_పదాలు_2, అబద్ధం);
  }
  తిరిగి clear_వాడని_బిట్లు();
}

తార్కిక APInt::equalSlowCase(స్థిరరాశి APInt &RHS) స్థిరరాశి {
  తిరిగి std::equal(U.pVal, U.pVal + get_అంకె_పదాలు(), RHS.U.pVal);
}

పూర్ణాంకం APInt::compare(స్థిరరాశి APInt& RHS) స్థిరరాశి {
  ప్రతిజ్ఞా(బిట్టు_వెడల్పు_2 == RHS.బిట్టు_వెడల్పు_2 && "Bit widths must be same for comparison");
  ఒకవేళ (LLVM_LIKELY(is_Single_మాట()))
    తిరిగి U.VAL < RHS.U.VAL ? -1 : U.VAL > RHS.U.VAL;

  తిరిగి tcCompare(U.pVal, RHS.U.pVal, get_అంకె_పదాలు());
}

పూర్ణాంకం APInt::compareSigned(స్థిరరాశి APInt& RHS) స్థిరరాశి {
  ప్రతిజ్ఞా(బిట్టు_వెడల్పు_2 == RHS.బిట్టు_వెడల్పు_2 && "Bit widths must be same for comparison");
  ఒకవేళ (LLVM_LIKELY(is_Single_మాట())) {
    int64_t lhsSext = SignExtend64(U.VAL, బిట్టు_వెడల్పు_2);
    int64_t rhsSext = SignExtend64(RHS.U.VAL, బిట్టు_వెడల్పు_2);
    తిరిగి lhsSext < rhsSext ? -1 : lhsSext > rhsSext;
  }

  తార్కిక lhsNeg = isNegative();
  తార్కిక rhsNeg = RHS.isNegative();

  // If the sign bits don't match, then (LHS < RHS) if LHS is negative
  ఒకవేళ (lhsNeg != rhsNeg)
    తిరిగి lhsNeg ? -1 : 1;

  // Otherwise we can just use an unsigned comparison, because even negative
  // numbers compare correctly this way if both have the same signed-ness.
  తిరిగి tcCompare(U.pVal, RHS.U.pVal, get_అంకె_పదాలు());
}

శూన్య APInt::సమితి_బిట్లు_Slow_Case(గుర్తులేని lo_బిట్టు, గుర్తులేని hi_బిట్టు) {
  గుర్తులేని lo_మాట = which_మాట(lo_బిట్టు);
  గుర్తులేని hi_మాట = which_మాట(hi_బిట్టు);

  // Create an initial mask for the low word with zeros below loBit.
  uint64_t lo_ముసుగు = WORDTYPE_MAX << which_బిట్టు(lo_బిట్టు);

  // If hiBit is not aligned, we need a high mask.
  గుర్తులేని hi_జరుపు_Amt = which_బిట్టు(hi_బిట్టు);
  ఒకవేళ (hi_జరుపు_Amt != 0) {
    // Create a high mask with zeros above hiBit.
    uint64_t hi_ముసుగు = WORDTYPE_MAX >> (APINT_బిట్లు_చొప్పున_మాట - hi_జరుపు_Amt);
    // If loWord and hiWord are equal, then we combine the masks. Otherwise,
    // set the bits in hiWord.
    ఒకవేళ (hi_మాట == lo_మాట)
      lo_ముసుగు &= hi_ముసుగు;
    లేకపోతే
      U.pVal[hi_మాట] |= hi_ముసుగు;
  }
  // Apply the mask to the low word.
  U.pVal[lo_మాట] |= lo_ముసుగు;

  // Fill any words between loWord and hiWord with all ones.
  కొరకు (గుర్తులేని మాట = lo_మాట + 1; మాట < hi_మాట; ++మాట)
    U.pVal[మాట] = WORDTYPE_MAX;
}

శూన్య APInt::clear_బిట్లు_Slow_Case(గుర్తులేని Lo_బిట్టు, గుర్తులేని Hi_బిట్టు) {
  గుర్తులేని Lo_మాట = which_మాట(Lo_బిట్టు);
  గుర్తులేని Hi_మాట = which_మాట(Hi_బిట్టు);

  // Create an initial mask for the low word with ones below loBit.
  uint64_t Lo_ముసుగు = ~(WORDTYPE_MAX << which_బిట్టు(Lo_బిట్టు));

  // If HiBit is not aligned, we need a high mask.
  గుర్తులేని Hi_జరుపు_Amt = which_బిట్టు(Hi_బిట్టు);
  ఒకవేళ (Hi_జరుపు_Amt != 0) {
    // Create a high mask with ones above HiBit.
    uint64_t Hi_ముసుగు = ~(WORDTYPE_MAX >> (APINT_బిట్లు_చొప్పున_మాట - Hi_జరుపు_Amt));
    // If LoWord and HiWord are equal, then we combine the masks. Otherwise,
    // clear the bits in HiWord.
    ఒకవేళ (Hi_మాట == Lo_మాట)
      Lo_ముసుగు |= Hi_ముసుగు;
    లేకపోతే
      U.pVal[Hi_మాట] &= Hi_ముసుగు;
  }
  // Apply the mask to the low word.
  U.pVal[Lo_మాట] &= Lo_ముసుగు;

  // Fill any words between LoWord and HiWord with all zeros.
  కొరకు (గుర్తులేని మాట_2 = Lo_మాట + 1; మాట_2 < Hi_మాట; ++మాట_2)
    U.pVal[మాట_2] = 0;
}

// Complement a bignum in-place.
స్థిర శూన్య tcComplement(APInt::మాట_Type *dst, గుర్తులేని parts) {
  కొరకు (గుర్తులేని i = 0; i < parts; i++)
    dst[i] = ~dst[i];
}

/// Toggle every bit to its opposite value.
శూన్య APInt::flip_All_బిట్లు_Slow_Case() {
  tcComplement(U.pVal, get_అంకె_పదాలు());
  clear_వాడని_బిట్లు();
}

/// Concatenate the bits from "NewLSB" onto the bottom of *this.  This is
/// equivalent to:
///   (this->zext(NewWidth) << NewLSB.getBitWidth()) | NewLSB.zext(NewWidth)
/// In the slow case, we know the result is large.
APInt APInt::concatSlowCase(స్థిరరాశి APInt &NewLSB) స్థిరరాశి {
  గుర్తులేని New_వెడల్పు = get_బిట్టు_వెడల్పు() + NewLSB.get_బిట్టు_వెడల్పు();
  APInt ఫలితం = NewLSB.zext(New_వెడల్పు);
  ఫలితం.చొప్పించు_బిట్లు(*ఇది, NewLSB.get_బిట్టు_వెడల్పు());
  తిరిగి ఫలితం;
}

/// Toggle a given bit to its opposite value whose position is given
/// as "bitPosition".
/// Toggles a given bit to its opposite value.
శూన్య APInt::flip_బిట్టు(గుర్తులేని బిట్టు_Position) {
  ప్రతిజ్ఞా(బిట్టు_Position < బిట్టు_వెడల్పు_2 && "Out of the bit-width range!");
  సమితి_బిట్టు_Val(బిట్టు_Position, !(*ఇది)[బిట్టు_Position]);
}

శూన్య APInt::చొప్పించు_బిట్లు(స్థిరరాశి APInt &sub_బిట్లు, గుర్తులేని బిట్టు_Position) {
  గుర్తులేని sub_బిట్టు_వెడల్పు = sub_బిట్లు.get_బిట్టు_వెడల్పు();
  ప్రతిజ్ఞా((sub_బిట్టు_వెడల్పు + బిట్టు_Position) <= బిట్టు_వెడల్పు_2 && "Illegal bit insertion");

  // inserting no bits is a noop.
  ఒకవేళ (sub_బిట్టు_వెడల్పు == 0)
    తిరిగి;

  // Insertion is a direct copy.
  ఒకవేళ (sub_బిట్టు_వెడల్పు == బిట్టు_వెడల్పు_2) {
    *ఇది = sub_బిట్లు;
    తిరిగి;
  }

  // Single word result can be done as a direct bitmask.
  ఒకవేళ (LLVM_LIKELY(is_Single_మాట())) {
    uint64_t ముసుగు_2 = WORDTYPE_MAX >> (APINT_బిట్లు_చొప్పున_మాట - sub_బిట్టు_వెడల్పు);
    U.VAL &= ~(ముసుగు_2 << బిట్టు_Position);
    U.VAL |= (sub_బిట్లు.U.VAL << బిట్టు_Position);
    తిరిగి;
  }

  గుర్తులేని lo_బిట్టు = which_బిట్టు(బిట్టు_Position);
  గుర్తులేని lo_మాట = which_మాట(బిట్టు_Position);
  గుర్తులేని hi1_మాట = which_మాట(బిట్టు_Position + sub_బిట్టు_వెడల్పు - 1);

  // Insertion within a single word can be done as a direct bitmask.
  ఒకవేళ (lo_మాట == hi1_మాట) {
    uint64_t ముసుగు_2 = WORDTYPE_MAX >> (APINT_బిట్లు_చొప్పున_మాట - sub_బిట్టు_వెడల్పు);
    U.pVal[lo_మాట] &= ~(ముసుగు_2 << lo_బిట్టు);
    U.pVal[lo_మాట] |= (sub_బిట్లు.U.VAL << lo_బిట్టు);
    తిరిగి;
  }

  // Insert on word boundaries.
  ఒకవేళ (lo_బిట్టు == 0) {
    // Direct copy whole words.
    గుర్తులేని అంకె_సంపూర్ణ_Sub_పదాలు = sub_బిట్టు_వెడల్పు / APINT_బిట్లు_చొప్పున_మాట;
    జ్ఞాపకం_నకలు(U.pVal + lo_మాట, sub_బిట్లు.get_Raw_దత్తాంశం(),
           అంకె_సంపూర్ణ_Sub_పదాలు * APINT_మాట_కొలత);

    // Mask+insert remaining bits.
    గుర్తులేని మిగిలిన_బిట్లు = sub_బిట్టు_వెడల్పు % APINT_బిట్లు_చొప్పున_మాట;
    ఒకవేళ (మిగిలిన_బిట్లు != 0) {
      uint64_t ముసుగు_2 = WORDTYPE_MAX >> (APINT_బిట్లు_చొప్పున_మాట - మిగిలిన_బిట్లు);
      U.pVal[hi1_మాట] &= ~ముసుగు_2;
      U.pVal[hi1_మాట] |= sub_బిట్లు.get_మాట(sub_బిట్టు_వెడల్పు - 1);
    }
    తిరిగి;
  }

  // General case - set/clear individual bits in dst based on src.
  // TODO - there is scope for optimization here, but at the moment this code
  // path is barely used so prefer readability over performance.
  కొరకు (గుర్తులేని i = 0; i != sub_బిట్టు_వెడల్పు; ++i)
    సమితి_బిట్టు_Val(బిట్టు_Position + i, sub_బిట్లు[i]);
}

శూన్య APInt::చొప్పించు_బిట్లు(uint64_t sub_బిట్లు, గుర్తులేని బిట్టు_Position, గుర్తులేని అంకె_బిట్లు) {
  uint64_t ముసుగు_బిట్లు = ముసుగు_Trailing_Ones<uint64_t>(అంకె_బిట్లు);
  sub_బిట్లు &= ముసుగు_బిట్లు;
  ఒకవేళ (LLVM_LIKELY(is_Single_మాట())) {
    U.VAL &= ~(ముసుగు_బిట్లు << బిట్టు_Position);
    U.VAL |= sub_బిట్లు << బిట్టు_Position;
    తిరిగి;
  }

  గుర్తులేని lo_బిట్టు = which_బిట్టు(బిట్టు_Position);
  గుర్తులేని lo_మాట = which_మాట(బిట్టు_Position);
  గుర్తులేని hi_మాట = which_మాట(బిట్టు_Position + అంకె_బిట్లు - 1);
  ఒకవేళ (lo_మాట == hi_మాట) {
    U.pVal[lo_మాట] &= ~(ముసుగు_బిట్లు << lo_బిట్టు);
    U.pVal[lo_మాట] |= sub_బిట్లు << lo_బిట్టు;
    తిరిగి;
  }

  static_assert(8 * పరిమాణం(మాట_Type) <= 64, "This code assumes only two words affected");
  గుర్తులేని మాట_బిట్లు = 8 * పరిమాణం(మాట_Type);
  U.pVal[lo_మాట] &= ~(ముసుగు_బిట్లు << lo_బిట్టు);
  U.pVal[lo_మాట] |= sub_బిట్లు << lo_బిట్టు;

  U.pVal[hi_మాట] &= ~(ముసుగు_బిట్లు >> (మాట_బిట్లు - lo_బిట్టు));
  U.pVal[hi_మాట] |= sub_బిట్లు >> (మాట_బిట్లు - lo_బిట్టు);
}

APInt APInt::వెలికితీయి_బిట్లు(గుర్తులేని అంకె_బిట్లు, గుర్తులేని బిట్టు_Position) స్థిరరాశి {
  ప్రతిజ్ఞా(బిట్టు_Position < బిట్టు_వెడల్పు_2 && (అంకె_బిట్లు + బిట్టు_Position) <= బిట్టు_వెడల్పు_2 &&
         "Illegal bit extraction");

  ఒకవేళ (LLVM_LIKELY(is_Single_మాట()))
    తిరిగి APInt(అంకె_బిట్లు, U.VAL >> బిట్టు_Position, /*isSigned=*/అబద్ధం,
                 /*implicitTrunc=*/నిజం);

  గుర్తులేని lo_బిట్టు = which_బిట్టు(బిట్టు_Position);
  గుర్తులేని lo_మాట = which_మాట(బిట్టు_Position);
  గుర్తులేని hi_మాట = which_మాట(బిట్టు_Position + అంకె_బిట్లు - 1);

  // Single word result extracting bits from a single word source.
  ఒకవేళ (lo_మాట == hi_మాట)
    తిరిగి APInt(అంకె_బిట్లు, U.pVal[lo_మాట] >> lo_బిట్టు, /*isSigned=*/అబద్ధం,
                 /*implicitTrunc=*/నిజం);

  // Extracting bits that start on a source word boundary can be done
  // as a fast memory copy.
  ఒకవేళ (lo_బిట్టు == 0)
    తిరిగి APInt(అంకె_బిట్లు, ArrayRef(U.pVal + lo_మాట, 1 + hi_మాట - lo_మాట));

  // General case - shift + copy source words directly into place.
  APInt ఫలితం(అంకె_బిట్లు, 0);
  గుర్తులేని అంకె_Src_పదాలు = get_అంకె_పదాలు();
  గుర్తులేని అంకె_Dst_పదాలు = ఫలితం.get_అంకె_పదాలు();

  uint64_t *DestPtr =
      LLVM_LIKELY(ఫలితం.is_Single_మాట()) ? &ఫలితం.U.VAL : ఫలితం.U.pVal;
  కొరకు (గుర్తులేని మాట = 0; మాట < అంకె_Dst_పదాలు; ++మాట) {
    uint64_t w0 = U.pVal[lo_మాట + మాట];
    uint64_t w1 =
        (lo_మాట + మాట + 1) < అంకె_Src_పదాలు ? U.pVal[lo_మాట + మాట + 1] : 0;
    DestPtr[మాట] = (w0 >> lo_బిట్టు) | (w1 << (APINT_బిట్లు_చొప్పున_మాట - lo_బిట్టు));
  }

  తిరిగి ఫలితం.clear_వాడని_బిట్లు();
}

uint64_t APInt::వెలికితీయి_బిట్లు_As_ZExt_విలువ(గుర్తులేని అంకె_బిట్లు,
                                       గుర్తులేని బిట్టు_Position) స్థిరరాశి {
  ప్రతిజ్ఞా(బిట్టు_Position < బిట్టు_వెడల్పు_2 && (అంకె_బిట్లు + బిట్టు_Position) <= బిట్టు_వెడల్పు_2 &&
         "Illegal bit extraction");
  ప్రతిజ్ఞా(అంకె_బిట్లు <= 64 && "Illegal bit extraction");

  uint64_t ముసుగు_బిట్లు = ముసుగు_Trailing_Ones<uint64_t>(అంకె_బిట్లు);
  ఒకవేళ (LLVM_LIKELY(is_Single_మాట()))
    తిరిగి (U.VAL >> బిట్టు_Position) & ముసుగు_బిట్లు;

  static_assert(APINT_బిట్లు_చొప్పున_మాట >= 64,
                "This code assumes only two words affected");
  గుర్తులేని lo_బిట్టు = which_బిట్టు(బిట్టు_Position);
  గుర్తులేని lo_మాట = which_మాట(బిట్టు_Position);
  గుర్తులేని hi_మాట = which_మాట(బిట్టు_Position + అంకె_బిట్లు - 1);
  ఒకవేళ (lo_మాట == hi_మాట)
    తిరిగి (U.pVal[lo_మాట] >> lo_బిట్టు) & ముసుగు_బిట్లు;

  uint64_t ret_బిట్లు = U.pVal[lo_మాట] >> lo_బిట్టు;
  ret_బిట్లు |= U.pVal[hi_మాట] << (APINT_బిట్లు_చొప్పున_మాట - lo_బిట్టు);
  ret_బిట్లు &= ముసుగు_బిట్లు;
  తిరిగి ret_బిట్లు;
}

గుర్తులేని APInt::get_Sufficient_బిట్లు_Needed(StringRef Str, uint8_t Radix) {
  ప్రతిజ్ఞా(!Str.ఖాళీలు() && "Invalid string length");
  size_t StrLen = Str.కొలత_2();

  // Each computation below needs to know if it's negative.
  గుర్తులేని IsNegative = అబద్ధం;
  ఒకవేళ (Str[0] == '-' || Str[0] == '+') {
    IsNegative = Str[0] == '-';
    StrLen--;
    ప్రతిజ్ఞా(StrLen && "String is only a sign, needs a value.");
  }

  // For radixes of power-of-two values, the bits required is accurately and
  // easily computed.
  ఒకవేళ (Radix == 2)
    తిరిగి StrLen + IsNegative;
  ఒకవేళ (Radix == 8)
    తిరిగి StrLen * 3 + IsNegative;
  ఒకవేళ (Radix == 16)
    తిరిగి StrLen * 4 + IsNegative;

  // Compute a sufficient number of bits that is always large enough but might
  // be too large. This avoids the assertion in the constructor. This
  // calculation doesn't work appropriately for the numbers 0-9, so just use 4
  // bits in that case.
  ఒకవేళ (Radix == 10)
    తిరిగి (StrLen == 1 ? 4 : StrLen * 64 / 18) + IsNegative;

  ప్రతిజ్ఞా(Radix == 36);
  తిరిగి (StrLen == 1 ? 7 : StrLen * 16 / 3) + IsNegative;
}

గుర్తులేని APInt::get_బిట్లు_Needed(StringRef str, uint8_t radix) {
  // Compute a sufficient number of bits that is always large enough but might
  // be too large.
  గుర్తులేని sufficient = get_Sufficient_బిట్లు_Needed(str, radix);

  // For bases 2, 8, and 16, the sufficient number of bits is exact and we can
  // return the value directly. For bases 10 and 36, we need to do extra work.
  ఒకవేళ (radix == 2 || radix == 8 || radix == 16)
    తిరిగి sufficient;

  // This is grossly inefficient but accurate. We could probably do something
  // with a computation of roughly slen*64/20 and then adjust by the value of
  // the first few digits. But, I'm not sure how accurate that could be.
  size_t slen = str.కొలత_2();

  // Each computation below needs to know if it's negative.
  StringRef::iterator p = str.begin();
  గుర్తులేని isNegative = *p == '-';
  ఒకవేళ (*p == '-' || *p == '+') {
    p++;
    slen--;
    ప్రతిజ్ఞా(slen && "String is only a sign, needs a value.");
  }


  // Convert to the actual binary value.
  APInt tmp(sufficient, StringRef(p, slen), radix);

  // Compute how many bits are required. If the log is infinite, assume we need
  // just bit. If the log is exact and value is negative, then the value is
  // MinSignedValue with (log + 1) bits.
  గుర్తులేని సంవర్గమానం = tmp.logBase2();
  ఒకవేళ (సంవర్గమానం == (గుర్తులేని)-1) {
    తిరిగి isNegative + 1;
  } లేకపోతే ఒకవేళ (isNegative && tmp.isPowerOf2()) {
    తిరిగి isNegative + సంవర్గమానం;
  } లేకపోతే {
    తిరిగి isNegative + సంవర్గమానం + 1;
  }
}

hash_సంకేతం llvm::hash_విలువ(స్థిరరాశి APInt &Arg) {
  ఒకవేళ (LLVM_LIKELY(Arg.is_Single_మాట()))
    తిరిగి hash_combine(Arg.బిట్టు_వెడల్పు_2, Arg.U.VAL);

  తిరిగి hash_combine(
      Arg.బిట్టు_వెడల్పు_2,
      hash_combine_range(Arg.U.pVal, Arg.U.pVal + Arg.get_అంకె_పదాలు()));
}

గుర్తులేని DenseMapInfo<APInt, శూన్య>::get_Hash_విలువ(స్థిరరాశి APInt &కీలకం) {
  తిరిగి static_cast<గుర్తులేని>(hash_విలువ(కీలకం));
}

తార్కిక APInt::isSplat(గుర్తులేని Splat_కొలత_లోపల_బిట్లు) స్థిరరాశి {
  ప్రతిజ్ఞా(get_బిట్టు_వెడల్పు() % Splat_కొలత_లోపల_బిట్లు == 0 &&
         "SplatSizeInBits must divide width!");
  // We can check that all parts of an integer are equal by making use of a
  // little trick: rotate and check if it's still the same value.
  తిరిగి *ఇది == rotl(Splat_కొలత_లోపల_బిట్లు);
}

/// This function returns the high "numBits" bits of this APInt.
APInt APInt::get_Hi_బిట్లు(గుర్తులేని అంకె_బిట్లు) స్థిరరాశి {
  తిరిగి ఇది->lshr(బిట్టు_వెడల్పు_2 - అంకె_బిట్లు);
}

/// This function returns the low "numBits" bits of this APInt.
APInt APInt::get_Lo_బిట్లు(గుర్తులేని అంకె_బిట్లు) స్థిరరాశి {
  APInt ఫలితం(get_దిగువ_బిట్లు_సమితి(బిట్టు_వెడల్పు_2, అంకె_బిట్లు));
  ఫలితం &= *ఇది;
  తిరిగి ఫలితం;
}

/// Return a value containing V broadcasted over NewLen bits.
APInt APInt::getSplat(గుర్తులేని NewLen, స్థిరరాశి APInt &V) {
  ప్రతిజ్ఞా(NewLen >= V.get_బిట్టు_వెడల్పు() && "Can't splat to smaller bit width!");

  APInt Val = V.zext(NewLen);
  కొరకు (గుర్తులేని I = V.get_బిట్టు_వెడల్పు(); I < NewLen; I <<= 1)
    Val |= Val << I;

  తిరిగి Val;
}

గుర్తులేని APInt::లెక్క_Leading_Zeros_Slow_Case() స్థిరరాశి {
  గుర్తులేని లెక్క_2 = 0;
  కొరకు (పూర్ణాంకం i = get_అంకె_పదాలు() - 1; i >= 0; --i) {
    uint64_t V = U.pVal[i];
    ఒకవేళ (V == 0)
      లెక్క_2 += APINT_బిట్లు_చొప్పున_మాట;
    లేకపోతే {
      లెక్క_2 += llvm::countl_zero(V);
      ఆపు;
    }
  }
  // Adjust for unused bits in the most significant word (they are zero).
  గుర్తులేని Mod = బిట్టు_వెడల్పు_2 % APINT_బిట్లు_చొప్పున_మాట;
  లెక్క_2 -= Mod > 0 ? APINT_బిట్లు_చొప్పున_మాట - Mod : 0;
  తిరిగి లెక్క_2;
}

గుర్తులేని APInt::లెక్క_Leading_Ones_Slow_Case() స్థిరరాశి {
  గుర్తులేని ఎగువ_మాట_బిట్లు = బిట్టు_వెడల్పు_2 % APINT_బిట్లు_చొప్పున_మాట;
  గుర్తులేని జరుపు;
  ఒకవేళ (!ఎగువ_మాట_బిట్లు) {
    ఎగువ_మాట_బిట్లు = APINT_బిట్లు_చొప్పున_మాట;
    జరుపు = 0;
  } లేకపోతే {
    జరుపు = APINT_బిట్లు_చొప్పున_మాట - ఎగువ_మాట_బిట్లు;
  }
  పూర్ణాంకం i = get_అంకె_పదాలు() - 1;
  గుర్తులేని లెక్క_2 = llvm::countl_one(U.pVal[i] << జరుపు);
  ఒకవేళ (లెక్క_2 == ఎగువ_మాట_బిట్లు) {
    కొరకు (i--; i >= 0; --i) {
      ఒకవేళ (U.pVal[i] == WORDTYPE_MAX)
        లెక్క_2 += APINT_బిట్లు_చొప్పున_మాట;
      లేకపోతే {
        లెక్క_2 += llvm::countl_one(U.pVal[i]);
        ఆపు;
      }
    }
  }
  తిరిగి లెక్క_2;
}

గుర్తులేని APInt::లెక్క_Trailing_Zeros_Slow_Case() స్థిరరాశి {
  గుర్తులేని లెక్క_2 = 0;
  గుర్తులేని i = 0;
  కొరకు (; i < get_అంకె_పదాలు() && U.pVal[i] == 0; ++i)
    లెక్క_2 += APINT_బిట్లు_చొప్పున_మాట;
  ఒకవేళ (i < get_అంకె_పదాలు())
    లెక్క_2 += llvm::countr_zero(U.pVal[i]);
  తిరిగి std::min(లెక్క_2, బిట్టు_వెడల్పు_2);
}

గుర్తులేని APInt::లెక్క_Trailing_Ones_Slow_Case() స్థిరరాశి {
  గుర్తులేని లెక్క_2 = 0;
  గుర్తులేని i = 0;
  కొరకు (; i < get_అంకె_పదాలు() && U.pVal[i] == WORDTYPE_MAX; ++i)
    లెక్క_2 += APINT_బిట్లు_చొప్పున_మాట;
  ఒకవేళ (i < get_అంకె_పదాలు())
    లెక్క_2 += llvm::countr_one(U.pVal[i]);
  ప్రతిజ్ఞా(లెక్క_2 <= బిట్టు_వెడల్పు_2);
  తిరిగి లెక్క_2;
}

గుర్తులేని APInt::లెక్క_Population_Slow_Case() స్థిరరాశి {
  గుర్తులేని లెక్క_2 = 0;
  కొరకు (గుర్తులేని i = 0; i < get_అంకె_పదాలు(); ++i)
    లెక్క_2 += llvm::popcount(U.pVal[i]);
  తిరిగి లెక్క_2;
}

తార్కిక APInt::isPowerOf2SlowCase() స్థిరరాశి {
  గుర్తులేని లెక్క_2 = 0;
  కొరకు (గుర్తులేని i = 0; i < get_అంకె_పదాలు(); ++i) {
    లెక్క_2 += llvm::popcount(U.pVal[i]);
    ఒకవేళ (లెక్క_2 > 1)
      తిరిగి అబద్ధం;
  }
  తిరిగి లెక్క_2 == 1;
}

తార్కిక APInt::intersectsSlowCase(స్థిరరాశి APInt &RHS) స్థిరరాశి {
  కొరకు (గుర్తులేని i = 0, e = get_అంకె_పదాలు(); i != e; ++i)
    ఒకవేళ ((U.pVal[i] & RHS.U.pVal[i]) != 0)
      తిరిగి నిజం;

  తిరిగి అబద్ధం;
}

తార్కిక APInt::is_Subset_యొక్క_Slow_Case(స్థిరరాశి APInt &RHS) స్థిరరాశి {
  కొరకు (గుర్తులేని i = 0, e = get_అంకె_పదాలు(); i != e; ++i)
    ఒకవేళ ((U.pVal[i] & ~RHS.U.pVal[i]) != 0)
      తిరిగి అబద్ధం;

  తిరిగి నిజం;
}

తార్కిక APInt::is_Inverse_యొక్క_Slow_Case(స్థిరరాశి APInt &RHS) స్థిరరాశి {
  స్థిరరాశి గుర్తులేని చివరి = get_అంకె_పదాలు() - 1;
  కొరకు (గుర్తులేని I = 0; I != చివరి; ++I)
    ఒకవేళ ((U.pVal[I] ^ RHS.U.pVal[I]) != WORDTYPE_MAX)
      తిరిగి అబద్ధం;

  గుర్తులేని తోక_బిట్లు = బిట్టు_వెడల్పు_2 - చివరి * APINT_బిట్లు_చొప్పున_మాట;
  మాట_Type తోక_ముసుగు = llvm::ముసుగు_Trailing_Ones<మాట_Type>(తోక_బిట్లు);
  తిరిగి (U.pVal[చివరి] ^ RHS.U.pVal[చివరి]) == తోక_ముసుగు;
}

APInt APInt::byte_మార్పిడి() స్థిరరాశి {
  ప్రతిజ్ఞా(బిట్టు_వెడల్పు_2 >= 16 && బిట్టు_వెడల్పు_2 % 8 == 0 && "Cannot byteswap!");
  ఒకవేళ (బిట్టు_వెడల్పు_2 == 16)
    తిరిగి APInt(బిట్టు_వెడల్పు_2, llvm::byteswap<uint16_t>(U.VAL));
  ఒకవేళ (బిట్టు_వెడల్పు_2 == 32)
    తిరిగి APInt(బిట్టు_వెడల్పు_2, llvm::byteswap<uint32_t>(U.VAL));
  ఒకవేళ (బిట్టు_వెడల్పు_2 <= 64) {
    uint64_t Tmp1 = llvm::byteswap<uint64_t>(U.VAL);
    Tmp1 >>= (64 - బిట్టు_వెడల్పు_2);
    తిరిగి APInt(బిట్టు_వెడల్పు_2, Tmp1);
  }

  APInt ఫలితం(get_అంకె_పదాలు() * APINT_బిట్లు_చొప్పున_మాట, 0);
  కొరకు (గుర్తులేని I = 0, N = get_అంకె_పదాలు(); I != N; ++I)
    ఫలితం.U.pVal[I] = llvm::byteswap<uint64_t>(U.pVal[N - I - 1]);
  ఒకవేళ (ఫలితం.బిట్టు_వెడల్పు_2 != బిట్టు_వెడల్పు_2) {
    ఫలితం.lshr_లోపల_ఉంచు(ఫలితం.బిట్టు_వెడల్పు_2 - బిట్టు_వెడల్పు_2);
    ఫలితం.బిట్టు_వెడల్పు_2 = బిట్టు_వెడల్పు_2;
  }
  తిరిగి ఫలితం;
}

APInt APInt::తిరగేయి_బిట్లు() స్థిరరాశి {
  ఒకవేళ (LLVM_LIKELY(is_Single_మాట())) {
    ఎంపిక (బిట్టు_వెడల్పు_2) {
    స్థితి 64:
      తిరిగి APInt(బిట్టు_వెడల్పు_2, llvm::తిరగేయి_బిట్లు<uint64_t>(U.VAL));
    స్థితి 32:
      తిరిగి APInt(బిట్టు_వెడల్పు_2, llvm::తిరగేయి_బిట్లు<uint32_t>(U.VAL));
    స్థితి 16:
      తిరిగి APInt(బిట్టు_వెడల్పు_2, llvm::తిరగేయి_బిట్లు<uint16_t>(U.VAL));
    స్థితి 8:
      తిరిగి APInt(బిట్టు_వెడల్పు_2, llvm::తిరగేయి_బిట్లు<uint8_t>(U.VAL));
    స్థితి 1: // fallthrough
    స్థితి 0:
      తిరిగి *ఇది;
    అప్రమేయ:
      తిరిగి APInt(బిట్టు_వెడల్పు_2,
                   llvm::తిరగేయి_బిట్లు<uint64_t>(U.VAL) >> (64 - బిట్టు_వెడల్పు_2));
    }
  }

  APInt ఫలితం(బిట్టు_వెడల్పు_2, 0);
  గుర్తులేని అంకె_పదాలు_2 = get_అంకె_పదాలు();
  గుర్తులేని Excess_బిట్లు = అంకె_పదాలు_2 * APINT_బిట్లు_చొప్పున_మాట - బిట్టు_వెడల్పు_2;
  ఒకవేళ (Excess_బిట్లు == 0) {
    // Fast path. No cross-word shift needed.
    కొరకు (గుర్తులేని I = 0; I < అంకె_పదాలు_2; ++I)
      ఫలితం.U.pVal[I] = llvm::తిరగేయి_బిట్లు<uint64_t>(U.pVal[అంకె_పదాలు_2 - 1 - I]);
    తిరిగి ఫలితం;
  }
  // Holds reversed bits of the previous (more significant) word.
  uint64_t PrevRev = llvm::తిరగేయి_బిట్లు<uint64_t>(U.pVal[అంకె_పదాలు_2 - 1]);
  కొరకు (గుర్తులేని I = 0; I < అంకె_పదాలు_2 - 1; ++I) {
    uint64_t CurrRev = llvm::తిరగేయి_బిట్లు<uint64_t>(U.pVal[అంకె_పదాలు_2 - 2 - I]);
    ఫలితం.U.pVal[I] = (PrevRev >> Excess_బిట్లు) | (CurrRev << (64 - Excess_బిట్లు));
    PrevRev = CurrRev;
  }
  ఫలితం.U.pVal[అంకె_పదాలు_2 - 1] = PrevRev >> Excess_బిట్లు;
  తిరిగి ఫలితం;
}

APInt llvm::APIntOps::Greatest_Common_భాజకం(APInt A, APInt B, తార్కిక IsSigned) {
  // Take absolute value if IsSigned.
  ఒకవేళ (IsSigned) {
    A = A.నిరపేక్ష();
    B = B.నిరపేక్ష();
  }

  // Fast-path a common case.
  ఒకవేళ (A == B) తిరిగి A;

  // Corner cases: if either operand is zero, the other is the gcd.
  ఒకవేళ (!A) తిరిగి B;
  ఒకవేళ (!B) తిరిగి A;

  // Count common powers of 2 and remove all other powers of 2.
  గుర్తులేని Pow2;
  {
    గుర్తులేని Pow2_A = A.countr_zero();
    గుర్తులేని Pow2_B = B.countr_zero();
    ఒకవేళ (Pow2_A > Pow2_B) {
      A.lshr_లోపల_ఉంచు(Pow2_A - Pow2_B);
      Pow2 = Pow2_B;
    } లేకపోతే ఒకవేళ (Pow2_B > Pow2_A) {
      B.lshr_లోపల_ఉంచు(Pow2_B - Pow2_A);
      Pow2 = Pow2_A;
    } లేకపోతే {
      Pow2 = Pow2_A;
    }
  }

  // Both operands are odd multiples of 2^Pow_2:
  //
  //   gcd(a, b) = gcd(|a - b| / 2^i, min(a, b))
  //
  // This is a modified version of Stein's algorithm, taking advantage of
  // efficient countTrailingZeros().
  అయితే (A != B) {
    ఒకవేళ (A.ugt(B)) {
      A -= B;
      A.lshr_లోపల_ఉంచు(A.countr_zero() - Pow2);
    } లేకపోతే {
      B -= A;
      B.lshr_లోపల_ఉంచు(B.countr_zero() - Pow2);
    }
  }

  తిరిగి A;
}

APInt llvm::APIntOps::విడత_Double_గమ్యం_APInt(ద్విగుణ Double, గుర్తులేని వెడల్పు_2) {
  uint64_t I = బిట్టు_cast<uint64_t>(Double);

  // Get the sign bit from the highest order bit
  తార్కిక isNeg = I >> 63;

  // Get the 11-bit exponent and adjust for the 1023 bit bias
  int64_t ఘాతాంకం = ((I >> 52) & 0x7ff) - 1023;

  // If the exponent is negative, the value is < 0 so just return 0.
  ఒకవేళ (ఘాతాంకం < 0)
    తిరిగి APInt(వెడల్పు_2, 0u);

  // Extract the mantissa by clearing the top 12 bits (sign + exponent).
  uint64_t mantissa = (I & (~0ULL >> 12)) | 1ULL << 52;

  // If the exponent doesn't shift all bits out of the mantissa
  ఒకవేళ (ఘాతాంకం < 52)
    తిరిగి isNeg ? -APInt(వెడల్పు_2, mantissa >> (52 - ఘాతాంకం)) :
                    APInt(వెడల్పు_2, mantissa >> (52 - ఘాతాంకం));

  // If the client didn't provide enough bits for us to shift the mantissa into
  // then the result is undefined, just return 0
  ఒకవేళ (వెడల్పు_2 <= ఘాతాంకం - 52)
    తిరిగి APInt(వెడల్పు_2, 0);

  // Otherwise, we have to shift the mantissa bits up to the right location
  APInt Tmp(వెడల్పు_2, mantissa);
  Tmp <<= (గుర్తులేని)ఘాతాంకం - 52;
  తిరిగి isNeg ? -Tmp : Tmp;
}

/// This function converts this APInt to a double.
/// The layout for double is as following (IEEE Standard 754):
///  --------------------------------------
/// |  Sign    Exponent    Fraction    Bias |
/// |-------------------------------------- |
/// |  1[63]   11[62-52]   52[51-00]   1023 |
///  --------------------------------------
ద్విగుణ APInt::విడత_గమ్యం_Double(తార్కిక isSigned) స్థిరరాశి {
  // Handle the simple case where the value is contained in one uint64_t.
  // It is wrong to optimize getWord(0) to VAL; there might be more than one word.
  ఒకవేళ (LLVM_LIKELY(is_Single_మాట() || get_చురుకైన_బిట్లు() <= APINT_బిట్లు_చొప్పున_మాట)) {
    ఒకవేళ (isSigned) {
      int64_t sext = SignExtend64(get_మాట(0), బిట్టు_వెడల్పు_2);
      తిరిగి ద్విగుణ(sext);
    }
    తిరిగి ద్విగుణ(get_మాట(0));
  }

  // Determine if the value is negative.
  తార్కిక isNeg = isSigned ? (*ఇది)[బిట్టు_వెడల్పు_2-1] : అబద్ధం;

  // Construct the absolute value if we're negative.
  APInt Tmp(isNeg ? -(*ఇది) : (*ఇది));

  // Figure out how many bits we're using.
  గుర్తులేని n = Tmp.get_చురుకైన_బిట్లు();

  // The exponent (without bias normalization) is just the number of bits
  // we are using. Note that the sign bit is gone since we constructed the
  // absolute value.
  uint64_t ఘాతాంకం = n;

  // Return infinity for exponent overflow
  ఒకవేళ (ఘాతాంకం > 1023) {
    ఒకవేళ (!isSigned || !isNeg)
      తిరిగి std::numeric_limits<ద్విగుణ>::infinity();
    లేకపోతే
      తిరిగి -std::numeric_limits<ద్విగుణ>::infinity();
  }
  ఘాతాంకం += 1023; // Increment for 1023 bias

  // Number of bits in mantissa is 52. To obtain the mantissa value, we must
  // extract the high 52 bits from the correct words in pVal.
  uint64_t mantissa;
  గుర్తులేని hi_మాట = which_మాట(n-1);
  ఒకవేళ (hi_మాట == 0) {
    mantissa = Tmp.U.pVal[0];
    ఒకవేళ (n > 52)
      mantissa >>= n - 52; // shift down, we want the top 52 bits.
  } లేకపోతే {
    ప్రతిజ్ఞా(hi_మాట > 0 && "huh?");
    uint64_t hibits = Tmp.U.pVal[hi_మాట] << (52 - n % APINT_బిట్లు_చొప్పున_మాట);
    uint64_t lobits = Tmp.U.pVal[hi_మాట-1] >> (11 + n % APINT_బిట్లు_చొప్పున_మాట);
    mantissa = hibits | lobits;
  }

  // The leading bit of mantissa is implicit, so get rid of it.
  uint64_t sign = isNeg ? (1ULL << (APINT_బిట్లు_చొప్పున_మాట - 1)) : 0;
  uint64_t I = sign | (ఘాతాంకం << 52) | mantissa;
  తిరిగి బిట్టు_cast<ద్విగుణ>(I);
}

// Truncate to new width.
APInt APInt::trunc(గుర్తులేని వెడల్పు_2) స్థిరరాశి {
  ప్రతిజ్ఞా(వెడల్పు_2 <= బిట్టు_వెడల్పు_2 && "Invalid APInt Truncate request");

  ఒకవేళ (వెడల్పు_2 <= APINT_బిట్లు_చొప్పున_మాట)
    తిరిగి APInt(వెడల్పు_2, get_Raw_దత్తాంశం()[0], /*isSigned=*/అబద్ధం,
                 /*implicitTrunc=*/నిజం);

  ఒకవేళ (వెడల్పు_2 == బిట్టు_వెడల్పు_2)
    తిరిగి *ఇది;

  APInt ఫలితం(get_జ్ఞాపకం(get_అంకె_పదాలు(వెడల్పు_2)), వెడల్పు_2);

  // Copy full words.
  గుర్తులేని i;
  కొరకు (i = 0; i != వెడల్పు_2 / APINT_బిట్లు_చొప్పున_మాట; i++)
    ఫలితం.U.pVal[i] = U.pVal[i];

  // Truncate and copy any partial word.
  గుర్తులేని బిట్లు = (0 - వెడల్పు_2) % APINT_బిట్లు_చొప్పున_మాట;
  ఒకవేళ (బిట్లు != 0)
    ఫలితం.U.pVal[i] = U.pVal[i] << బిట్లు >> బిట్లు;

  తిరిగి ఫలితం;
}

// Truncate to new width with unsigned saturation.
APInt APInt::truncUSat(గుర్తులేని వెడల్పు_2) స్థిరరాశి {
  ప్రతిజ్ఞా(వెడల్పు_2 <= బిట్టు_వెడల్పు_2 && "Invalid APInt Truncate request");

  // Can we just losslessly truncate it?
  ఒకవేళ (isIntN(వెడల్పు_2))
    తిరిగి trunc(వెడల్పు_2);
  // If not, then just return the new limit.
  తిరిగి APInt::get_Max_విలువ(వెడల్పు_2);
}

// Truncate to new width with signed saturation to signed result.
APInt APInt::truncSSat(గుర్తులేని వెడల్పు_2) స్థిరరాశి {
  ప్రతిజ్ఞా(వెడల్పు_2 <= బిట్టు_వెడల్పు_2 && "Invalid APInt Truncate request");

  // Can we just losslessly truncate it?
  ఒకవేళ (isSignedIntN(వెడల్పు_2))
    తిరిగి trunc(వెడల్పు_2);
  // If not, then just return the new limits.
  తిరిగి isNegative() ? APInt::get_Signed_Min_విలువ(వెడల్పు_2)
                      : APInt::get_Signed_Max_విలువ(వెడల్పు_2);
}

// Truncate to new width with signed saturation to unsigned result.
APInt APInt::truncSSatU(గుర్తులేని వెడల్పు_2) స్థిరరాశి {
  ప్రతిజ్ఞా(వెడల్పు_2 <= బిట్టు_వెడల్పు_2 && "Invalid APInt Truncate request");

  // Can we just losslessly truncate it?
  ఒకవేళ (isIntN(వెడల్పు_2))
    తిరిగి trunc(వెడల్పు_2);
  // If not, then just return the new limits.
  తిరిగి isNegative() ? APInt::getZero(వెడల్పు_2) : APInt::get_Max_విలువ(వెడల్పు_2);
}

// Sign extend to a new width.
APInt APInt::sext(గుర్తులేని వెడల్పు) స్థిరరాశి {
  ప్రతిజ్ఞా(వెడల్పు >= బిట్టు_వెడల్పు_2 && "Invalid APInt SignExtend request");

  ఒకవేళ (వెడల్పు <= APINT_బిట్లు_చొప్పున_మాట)
    తిరిగి APInt(వెడల్పు, SignExtend64(U.VAL, బిట్టు_వెడల్పు_2), /*isSigned=*/నిజం);

  ఒకవేళ (వెడల్పు == బిట్టు_వెడల్పు_2)
    తిరిగి *ఇది;

  APInt ఫలితం(get_జ్ఞాపకం(get_అంకె_పదాలు(వెడల్పు)), వెడల్పు);

  // Copy words.
  std::జ్ఞాపకం_నకలు(ఫలితం.U.pVal, get_Raw_దత్తాంశం(), get_అంకె_పదాలు() * APINT_మాట_కొలత);

  // Sign extend the last word since there may be unused bits in the input.
  ఫలితం.U.pVal[get_అంకె_పదాలు() - 1] =
      SignExtend64(ఫలితం.U.pVal[get_అంకె_పదాలు() - 1],
                   ((బిట్టు_వెడల్పు_2 - 1) % APINT_బిట్లు_చొప్పున_మాట) + 1);

  // Fill with sign bits.
  std::memset(ఫలితం.U.pVal + get_అంకె_పదాలు(), isNegative() ? -1 : 0,
              (ఫలితం.get_అంకె_పదాలు() - get_అంకె_పదాలు()) * APINT_మాట_కొలత);
  ఫలితం.clear_వాడని_బిట్లు();
  తిరిగి ఫలితం;
}

//  Zero extend to a new width.
APInt APInt::zext(గుర్తులేని వెడల్పు_2) స్థిరరాశి {
  ప్రతిజ్ఞా(వెడల్పు_2 >= బిట్టు_వెడల్పు_2 && "Invalid APInt ZeroExtend request");

  ఒకవేళ (వెడల్పు_2 <= APINT_బిట్లు_చొప్పున_మాట)
    తిరిగి APInt(వెడల్పు_2, U.VAL);

  ఒకవేళ (వెడల్పు_2 == బిట్టు_వెడల్పు_2)
    తిరిగి *ఇది;

  APInt ఫలితం(get_జ్ఞాపకం(get_అంకె_పదాలు(వెడల్పు_2)), వెడల్పు_2);

  // Copy words.
  std::జ్ఞాపకం_నకలు(ఫలితం.U.pVal, get_Raw_దత్తాంశం(), get_అంకె_పదాలు() * APINT_మాట_కొలత);

  // Zero remaining words.
  std::memset(ఫలితం.U.pVal + get_అంకె_పదాలు(), 0,
              (ఫలితం.get_అంకె_పదాలు() - get_అంకె_పదాలు()) * APINT_మాట_కొలత);

  తిరిగి ఫలితం;
}

APInt APInt::zextOrTrunc(గుర్తులేని వెడల్పు_2) స్థిరరాశి {
  ఒకవేళ (బిట్టు_వెడల్పు_2 < వెడల్పు_2)
    తిరిగి zext(వెడల్పు_2);
  ఒకవేళ (బిట్టు_వెడల్పు_2 > వెడల్పు_2)
    తిరిగి trunc(వెడల్పు_2);
  తిరిగి *ఇది;
}

APInt APInt::sextOrTrunc(గుర్తులేని వెడల్పు_2) స్థిరరాశి {
  ఒకవేళ (బిట్టు_వెడల్పు_2 < వెడల్పు_2)
    తిరిగి sext(వెడల్పు_2);
  ఒకవేళ (బిట్టు_వెడల్పు_2 > వెడల్పు_2)
    తిరిగి trunc(వెడల్పు_2);
  తిరిగి *ఇది;
}

/// Arithmetic right-shift this APInt by shiftAmt.
/// Arithmetic right-shift function.
శూన్య APInt::ashr_లోపల_ఉంచు(స్థిరరాశి APInt &జరుపు_Amt) {
  ashr_లోపల_ఉంచు((గుర్తులేని)జరుపు_Amt.get_Limited_విలువ(బిట్టు_వెడల్పు_2));
}

/// Arithmetic right-shift this APInt by shiftAmt.
/// Arithmetic right-shift function.
శూన్య APInt::ashrSlowCase(గుర్తులేని జరుపు_Amt_2) {
  // Don't bother performing a no-op shift.
  ఒకవేళ (!జరుపు_Amt_2)
    తిరిగి;

  // Save the original sign bit for later.
  తార్కిక Negative = isNegative();

  // WordShift is the inter-part shift; BitShift is intra-part shift.
  గుర్తులేని మాట_జరుపు = జరుపు_Amt_2 / APINT_బిట్లు_చొప్పున_మాట;
  గుర్తులేని బిట్టు_జరుపు = జరుపు_Amt_2 % APINT_బిట్లు_చొప్పున_మాట;

  గుర్తులేని పదాలు_గమ్యం_Move = get_అంకె_పదాలు() - మాట_జరుపు;
  ఒకవేళ (పదాలు_గమ్యం_Move != 0) {
    // Sign extend the last word to fill in the unused bits.
    U.pVal[get_అంకె_పదాలు() - 1] = SignExtend64(
        U.pVal[get_అంకె_పదాలు() - 1], ((బిట్టు_వెడల్పు_2 - 1) % APINT_బిట్లు_చొప్పున_మాట) + 1);

    // Fastpath for moving by whole words.
    ఒకవేళ (బిట్టు_జరుపు == 0) {
      std::memmove(U.pVal, U.pVal + మాట_జరుపు, పదాలు_గమ్యం_Move * APINT_మాట_కొలత);
    } లేకపోతే {
      // Move the words containing significant bits.
      కొరకు (గుర్తులేని i = 0; i != పదాలు_గమ్యం_Move - 1; ++i)
        U.pVal[i] = (U.pVal[i + మాట_జరుపు] >> బిట్టు_జరుపు) |
                    (U.pVal[i + మాట_జరుపు + 1] << (APINT_బిట్లు_చొప్పున_మాట - బిట్టు_జరుపు));

      // Handle the last word which has no high bits to copy. Use an arithmetic
      // shift to preserve the sign bit.
      U.pVal[పదాలు_గమ్యం_Move - 1] =
          (int64_t)U.pVal[మాట_జరుపు + పదాలు_గమ్యం_Move - 1] >> బిట్టు_జరుపు;
    }
  }

  // Fill in the remainder based on the original sign.
  std::memset(U.pVal + పదాలు_గమ్యం_Move, Negative ? -1 : 0,
              మాట_జరుపు * APINT_మాట_కొలత);
  clear_వాడని_బిట్లు();
}

/// Logical right-shift this APInt by shiftAmt.
/// Logical right-shift function.
శూన్య APInt::lshr_లోపల_ఉంచు(స్థిరరాశి APInt &జరుపు_Amt) {
  lshr_లోపల_ఉంచు((గుర్తులేని)జరుపు_Amt.get_Limited_విలువ(బిట్టు_వెడల్పు_2));
}

/// Logical right-shift this APInt by shiftAmt.
/// Logical right-shift function.
శూన్య APInt::lshrSlowCase(గుర్తులేని జరుపు_Amt_2) {
  tc_జరుపు_కుడి(U.pVal, get_అంకె_పదాలు(), జరుపు_Amt_2);
}

/// Left-shift this APInt by shiftAmt.
/// Left-shift function.
APInt &APInt::క్రియాకారకం<<=(స్థిరరాశి APInt &జరుపు_Amt) {
  // It's undefined behavior in C to shift by BitWidth or greater.
  *ఇది <<= (గుర్తులేని)జరుపు_Amt.get_Limited_విలువ(బిట్టు_వెడల్పు_2);
  తిరిగి *ఇది;
}

శూన్య APInt::shlSlowCase(గుర్తులేని జరుపు_Amt_2) {
  tc_జరుపు_ఎడమ(U.pVal, get_అంకె_పదాలు(), జరుపు_Amt_2);
  clear_వాడని_బిట్లు();
}

// Calculate the rotate amount modulo the bit width.
స్థిర గుర్తులేని rotateModulo(గుర్తులేని బిట్టు_వెడల్పు_2, స్థిరరాశి APInt &rotateAmt) {
  ఒకవేళ (LLVM_UNLIKELY(బిట్టు_వెడల్పు_2 == 0))
    తిరిగి 0;
  గుర్తులేని rot_బిట్టు_వెడల్పు = rotateAmt.get_బిట్టు_వెడల్పు();
  APInt rot = rotateAmt;
  ఒకవేళ (rot_బిట్టు_వెడల్పు < బిట్టు_వెడల్పు_2) {
    // Extend the rotate APInt, so that the urem doesn't divide by 0.
    // e.g. APInt(1, 32) would give APInt(1, 0).
    rot = rotateAmt.zext(బిట్టు_వెడల్పు_2);
  }
  rot = rot.urem(APInt(rot.get_బిట్టు_వెడల్పు(), బిట్టు_వెడల్పు_2));
  తిరిగి rot.get_Limited_విలువ(బిట్టు_వెడల్పు_2);
}

APInt APInt::rotl(స్థిరరాశి APInt &rotateAmt) స్థిరరాశి {
  తిరిగి rotl(rotateModulo(బిట్టు_వెడల్పు_2, rotateAmt));
}

APInt APInt::rotl(గుర్తులేని rotateAmt) స్థిరరాశి {
  ఒకవేళ (LLVM_UNLIKELY(బిట్టు_వెడల్పు_2 == 0))
    తిరిగి *ఇది;
  rotateAmt %= బిట్టు_వెడల్పు_2;
  ఒకవేళ (rotateAmt == 0)
    తిరిగి *ఇది;
  తిరిగి shl(rotateAmt) | lshr(బిట్టు_వెడల్పు_2 - rotateAmt);
}

APInt APInt::rotr(స్థిరరాశి APInt &rotateAmt) స్థిరరాశి {
  తిరిగి rotr(rotateModulo(బిట్టు_వెడల్పు_2, rotateAmt));
}

APInt APInt::rotr(గుర్తులేని rotateAmt) స్థిరరాశి {
  ఒకవేళ (బిట్టు_వెడల్పు_2 == 0)
    తిరిగి *ఇది;
  rotateAmt %= బిట్టు_వెడల్పు_2;
  ఒకవేళ (rotateAmt == 0)
    తిరిగి *ఇది;
  తిరిగి lshr(rotateAmt) | shl(బిట్టు_వెడల్పు_2 - rotateAmt);
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
గుర్తులేని APInt::సమీప_Log_Base2() స్థిరరాశి {
  // Special case when we have a bitwidth of 1. If VAL is 1, then we
  // get 0. If VAL is 0, we get WORDTYPE_MAX which gets truncated to
  // UINT32_MAX.
  ఒకవేళ (బిట్టు_వెడల్పు_2 == 1)
    తిరిగి U.VAL - 1;

  // Handle the zero case.
  ఒకవేళ (isZero())
    తిరిగి UINT32_MAX;

  // The non-zero case is handled by computing:
  //
  //   nearestLogBase2(x) = logBase2(x) + x[logBase2(x)-1].
  //
  // where x[i] is referring to the value of the ith bit of x.
  గుర్తులేని lg = logBase2();
  తిరిగి lg + గుర్తులేని((*ఇది)[lg - 1]);
}

// Square Root - this method computes and returns the square root of "this".
// Three mechanisms are used for computation. For small values (<= 5 bits),
// a table lookup is done. This gets some performance for common cases. For
// values using less than 52 bits, the value is converted to double and then
// the libc sqrt function is called. The result is rounded and then converted
// back to a uint64_t which is then used to construct the result. Finally,
// the Babylonian method for computing square roots is used.
APInt APInt::sqrtFloor() స్థిరరాశి {

  // Determine the magnitude of the value.
  గుర్తులేని magnitude = get_చురుకైన_బిట్లు();

  // Use a fast table for some small values. This also gets rid of some
  // rounding errors in libc sqrt for small values.
  ఒకవేళ (magnitude <= 5) {
    స్థిర స్థిరరాశి uint8_t results[32] = {
        /*     0 */ 0,
        /*  1- 3 */ 1, 1, 1,
        /*  4- 8 */ 2, 2, 2, 2, 2,
        /*  9-15 */ 3, 3, 3, 3, 3, 3, 3,
        /* 16-24 */ 4, 4, 4, 4, 4, 4, 4, 4, 4,
        /* 25-31 */ 5, 5, 5, 5, 5, 5, 5,
    };
    తిరిగి APInt(బిట్టు_వెడల్పు_2,
                 results[(LLVM_LIKELY(is_Single_మాట()) ? U.VAL : U.pVal[0])]);
  }

  // If the magnitude of the value fits in less than 52 bits (the precision of
  // an IEEE double precision floating point value), then we can use the
  // libc sqrt function which will probably use a hardware sqrt computation.
  // This should be faster than the algorithm below.
  ఒకవేళ (magnitude < 52) {
    తిరిగి APInt(బిట్టు_వెడల్పు_2,
                 uint64_t(::floor(::వర్గమూలం(ద్విగుణ(
                     LLVM_LIKELY(is_Single_మాట()) ? U.VAL : U.pVal[0])))));
  }

  // Okay, all the short cuts are exhausted. We must compute it. The following
  // is a classical Babylonian method for computing the square root. This code
  // was adapted to APInt from a wikipedia article on such computations.
  // See http://www.wikipedia.org/ and go to the page named
  // Calculate_an_integer_square_root.
  గుర్తులేని nbits = బిట్టు_వెడల్పు_2, i = 4;
  APInt testy(బిట్టు_వెడల్పు_2, 16);
  APInt x_old(బిట్టు_వెడల్పు_2, 1);
  APInt x_new(బిట్టు_వెడల్పు_2, 0);
  APInt two(బిట్టు_వెడల్పు_2, 2);

  // Select a good starting value using binary logarithms.
  కొరకు (;; i += 2, testy = testy.shl(2))
    ఒకవేళ (i >= nbits || ఇది->ule(testy)) {
      x_old = x_old.shl(i / 2);
      ఆపు;
    }

  // Use the Babylonian method to arrive at the integer square root:
  కొరకు (;;) {
    x_new = (ఇది->udiv(x_old) + x_old).udiv(two);
    ఒకవేళ (x_old.ule(x_new))
      ఆపు;
    x_old = x_new;
  }
  తిరిగి x_old;
}

/// \returns the multiplicative inverse of an odd APInt modulo 2^BitWidth.
APInt APInt::multiplicativeInverse() స్థిరరాశి {
  ప్రతిజ్ఞా((*ఇది)[0] &&
         "multiplicative inverse is only defined for odd numbers!");

  // Use Newton's method.
  APInt గుణకం = *ఇది;
  APInt T;
  అయితే (!(T = *ఇది * గుణకం).isOne())
    గుణకం *= 2 - std::move(T);
  తిరిగి గుణకం;
}

/// Implementation of Knuth's Algorithm D (Division of nonnegative integers)
/// from "Art of Computer Programming, Volume 2", section 4.3.1, p. 272. The
/// variables here have the same names as in the algorithm. Comments explain
/// the algorithm and any deviation from it.
స్థిర శూన్య KnuthDiv(uint32_t *u, uint32_t *v, uint32_t *q, uint32_t* r,
                     గుర్తులేని m, గుర్తులేని n) {
  ప్రతిజ్ఞా(u && "Must provide dividend");
  ప్రతిజ్ఞా(v && "Must provide divisor");
  ప్రతిజ్ఞా(q && "Must provide quotient");
  ప్రతిజ్ఞా(u != v && u != q && v != q && "Must use different memory");
  ప్రతిజ్ఞా(n>1 && "n must be > 1");

  // b denotes the base of the number system. In our case b is 2^32.
  స్థిరరాశి uint64_t b = uint64_t(1) << 32;

// The DEBUG macros here tend to be spam in the debug output if you're not
// debugging this code. Disable them unless KNUTH_DEBUG is defined.
#నిర్వచితమైతే KNUTH_DEBUG
#నిర్వచించు DEBUG_KNUTH(X) LLVM_DEBUG(X)
#లేకపోతే
#నిర్వచించు DEBUG_KNUTH(X) చేయి {} అయితే(అబద్ధం)
#అయితే_ముగింపు

  DEBUG_KNUTH(dbgs() << "KnuthDiv: m=" << m << " n=" << n << '\n');
  DEBUG_KNUTH(dbgs() << "KnuthDiv: original:");
  DEBUG_KNUTH(కొరకు (పూర్ణాంకం i = m + n; i >= 0; i--) dbgs() << " " << u[i]);
  DEBUG_KNUTH(dbgs() << " by");
  DEBUG_KNUTH(కొరకు (పూర్ణాంకం i = n; i > 0; i--) dbgs() << " " << v[i - 1]);
  DEBUG_KNUTH(dbgs() << '\n');
  // D1. [Normalize.] Set d = b / (v[n-1] + 1) and multiply all the digits of
  // u and v by d. Note that we have taken Knuth's advice here to use a power
  // of 2 value for d such that d * v[n-1] >= b/2 (b is the base). A power of
  // 2 allows us to shift instead of multiply and it is easy to determine the
  // shift amount from the leading zeros.  We are basically normalizing the u
  // and v so that its high bits are shifted to the top of v's range without
  // overflow. Note that this can require an extra word in u so that u must
  // be of length m+n+1.
  గుర్తులేని జరుపు = llvm::countl_zero(v[n - 1]);
  uint32_t v_వాహకం = 0;
  uint32_t u_వాహకం = 0;
  ఒకవేళ (జరుపు) {
    కొరకు (గుర్తులేని i = 0; i < m+n; ++i) {
      uint32_t u_tmp = u[i] >> (32 - జరుపు);
      u[i] = (u[i] << జరుపు) | u_వాహకం;
      u_వాహకం = u_tmp;
    }
    కొరకు (గుర్తులేని i = 0; i < n; ++i) {
      uint32_t v_tmp = v[i] >> (32 - జరుపు);
      v[i] = (v[i] << జరుపు) | v_వాహకం;
      v_వాహకం = v_tmp;
    }
  }
  u[m+n] = u_వాహకం;

  DEBUG_KNUTH(dbgs() << "KnuthDiv:   normal:");
  DEBUG_KNUTH(కొరకు (పూర్ణాంకం i = m + n; i >= 0; i--) dbgs() << " " << u[i]);
  DEBUG_KNUTH(dbgs() << " by");
  DEBUG_KNUTH(కొరకు (పూర్ణాంకం i = n; i > 0; i--) dbgs() << " " << v[i - 1]);
  DEBUG_KNUTH(dbgs() << '\n');

  // D2. [Initialize j.]  Set j to m. This is the loop counter over the places.
  పూర్ణాంకం j = m;
  చేయి {
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
    ఒకవేళ (qp == b || qp*v[n-2] > b*rp + u[j+n-2]) {
      qp--;
      rp += v[n-1];
      ఒకవేళ (rp < b && (qp == b || qp*v[n-2] > b*rp + u[j+n-2]))
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
    కొరకు (గుర్తులేని i = 0; i < n; ++i) {
      uint64_t p = qp * uint64_t(v[i]);
      int64_t subres = int64_t(u[j+i]) - borrow - Lo_32(p);
      u[j+i] = Lo_32(subres);
      borrow = Hi_32(p) - Hi_32(subres);
      DEBUG_KNUTH(dbgs() << "KnuthDiv: u[j+i] = " << u[j + i]
                        << ", borrow = " << borrow << '\n');
    }
    తార్కిక isNeg = u[j+n] < borrow;
    u[j+n] -= Lo_32(borrow);

    DEBUG_KNUTH(dbgs() << "KnuthDiv: after subtraction:");
    DEBUG_KNUTH(కొరకు (పూర్ణాంకం i = m + n; i >= 0; i--) dbgs() << " " << u[i]);
    DEBUG_KNUTH(dbgs() << '\n');

    // D5. [Test remainder.] Set q[j] = qp. If the result of step D4 was
    // negative, go to step D6; otherwise go on to step D7.
    q[j] = Lo_32(qp);
    ఒకవేళ (isNeg) {
      // D6. [Add back]. The probability that this step is necessary is very
      // small, on the order of only 2/b. Make sure that test data accounts for
      // this possibility. Decrease q[j] by 1
      q[j]--;
      // and add (0v[n-1]...v[1]v[0]) to (u[j+n]u[j+n-1]...u[j+1]u[j]).
      // A carry will occur to the left of u[j+n], and it should be ignored
      // since it cancels with the borrow that occurred in D4.
      తార్కిక వాహకం = అబద్ధం;
      కొరకు (గుర్తులేని i = 0; i < n; i++) {
        uint32_t హద్దు = std::min(u[j+i],v[i]);
        u[j+i] += v[i] + వాహకం;
        వాహకం = u[j+i] < హద్దు || (వాహకం && u[j+i] == హద్దు);
      }
      u[j+n] += వాహకం;
    }
    DEBUG_KNUTH(dbgs() << "KnuthDiv: after correction:");
    DEBUG_KNUTH(కొరకు (పూర్ణాంకం i = m + n; i >= 0; i--) dbgs() << " " << u[i]);
    DEBUG_KNUTH(dbgs() << "\nKnuthDiv: digit result = " << q[j] << '\n');

    // D7. [Loop on j.]  Decrease j by one. Now if j >= 0, go back to D3.
  } అయితే (--j >= 0);

  DEBUG_KNUTH(dbgs() << "KnuthDiv: quotient:");
  DEBUG_KNUTH(కొరకు (పూర్ణాంకం i = m; i >= 0; i--) dbgs() << " " << q[i]);
  DEBUG_KNUTH(dbgs() << '\n');

  // D8. [Unnormalize]. Now q[...] is the desired quotient, and the desired
  // remainder may be obtained by dividing u[...] by d. If r is non-null we
  // compute the remainder (urem uses this).
  ఒకవేళ (r) {
    // The value d is expressed by the "shift" value above since we avoided
    // multiplication by d by using a shift left. So, all we have to do is
    // shift right here.
    ఒకవేళ (జరుపు) {
      uint32_t వాహకం = 0;
      DEBUG_KNUTH(dbgs() << "KnuthDiv: remainder:");
      కొరకు (పూర్ణాంకం i = n-1; i >= 0; i--) {
        r[i] = (u[i] >> జరుపు) | వాహకం;
        వాహకం = u[i] << (32 - జరుపు);
        DEBUG_KNUTH(dbgs() << " " << r[i]);
      }
    } లేకపోతే {
      కొరకు (పూర్ణాంకం i = n-1; i >= 0; i--) {
        r[i] = u[i];
        DEBUG_KNUTH(dbgs() << " " << r[i]);
      }
    }
    DEBUG_KNUTH(dbgs() << '\n');
  }
  DEBUG_KNUTH(dbgs() << '\n');
}

శూన్య APInt::divide(స్థిరరాశి మాట_Type *LHS, గుర్తులేని lhs_పదాలు, స్థిరరాశి మాట_Type *RHS,
                   గుర్తులేని rhs_పదాలు, మాట_Type *Quotient, మాట_Type *Remainder) {
  ప్రతిజ్ఞా(lhs_పదాలు >= rhs_పదాలు && "Fractional result");

  // First, compose the values into an array of 32-bit words instead of
  // 64-bit words. This is a necessity of both the "short division" algorithm
  // and the Knuth "classical algorithm" which requires there to be native
  // operations for +, -, and * on an m bit value with an m*2 bit result. We
  // can't use 64-bit operands here because we don't have native results of
  // 128-bits. Furthermore, casting the 64-bit values to 32-bit values won't
  // work on large-endian machines.
  గుర్తులేని n = rhs_పదాలు * 2;
  గుర్తులేని m = (lhs_పదాలు * 2) - n;

  // Allocate space for the temporary values we need either on the stack, if
  // it will fit, or on the heap if it won't.
  uint32_t SPACE[128];
  uint32_t *U = శూన్యసూచిక;
  uint32_t *V = శూన్యసూచిక;
  uint32_t *Q = శూన్యసూచిక;
  uint32_t *R = శూన్యసూచిక;
  ఒకవేళ ((Remainder?4:3)*n+2*m+1 <= 128) {
    U = &SPACE[0];
    V = &SPACE[m+n+1];
    Q = &SPACE[(m+n+1) + n];
    ఒకవేళ (Remainder)
      R = &SPACE[(m+n+1) + n + (m+n)];
  } లేకపోతే {
    U = కొత్త uint32_t[m + n + 1];
    V = కొత్త uint32_t[n];
    Q = కొత్త uint32_t[m+n];
    ఒకవేళ (Remainder)
      R = కొత్త uint32_t[n];
  }

  // Initialize the dividend
  memset(U, 0, (m+n+1)*పరిమాణం(uint32_t));
  కొరకు (గుర్తులేని i = 0; i < lhs_పదాలు; ++i) {
    uint64_t tmp = LHS[i];
    U[i * 2] = Lo_32(tmp);
    U[i * 2 + 1] = Hi_32(tmp);
  }
  U[m+n] = 0; // this extra word is for "spill" in the Knuth algorithm.

  // Initialize the divisor
  memset(V, 0, (n)*పరిమాణం(uint32_t));
  కొరకు (గుర్తులేని i = 0; i < rhs_పదాలు; ++i) {
    uint64_t tmp = RHS[i];
    V[i * 2] = Lo_32(tmp);
    V[i * 2 + 1] = Hi_32(tmp);
  }

  // initialize the quotient and remainder
  memset(Q, 0, (m+n) * పరిమాణం(uint32_t));
  ఒకవేళ (Remainder)
    memset(R, 0, n * పరిమాణం(uint32_t));

  // Now, adjust m and n for the Knuth division. n is the number of words in
  // the divisor. m is the number of words by which the dividend exceeds the
  // divisor (i.e. m+n is the length of the dividend). These sizes must not
  // contain any zero words or the Knuth algorithm fails.
  కొరకు (గుర్తులేని i = n; i > 0 && V[i-1] == 0; i--) {
    n--;
    m++;
  }
  కొరకు (గుర్తులేని i = m+n; i > 0 && U[i-1] == 0; i--)
    m--;

  // If we're left with only a single word for the divisor, Knuth doesn't work
  // so we implement the short division algorithm here. This is much simpler
  // and faster because we are certain that we can divide a 64-bit quantity
  // by a 32-bit quantity at hardware speed and short division is simply a
  // series of such operations. This is just like doing short division but we
  // are using base 2^32 instead of base 10.
  ప్రతిజ్ఞా(n != 0 && "Divide by zero?");
  ఒకవేళ (n == 1) {
    uint32_t భాజకం = V[0];
    uint32_t remainder = 0;
    కొరకు (పూర్ణాంకం i = m; i >= 0; i--) {
      uint64_t partial_dividend = Make_64(remainder, U[i]);
      ఒకవేళ (partial_dividend == 0) {
        Q[i] = 0;
        remainder = 0;
      } లేకపోతే ఒకవేళ (partial_dividend < భాజకం) {
        Q[i] = 0;
        remainder = Lo_32(partial_dividend);
      } లేకపోతే ఒకవేళ (partial_dividend == భాజకం) {
        Q[i] = 1;
        remainder = 0;
      } లేకపోతే {
        Q[i] = Lo_32(partial_dividend / భాజకం);
        remainder = Lo_32(partial_dividend - (Q[i] * భాజకం));
      }
    }
    ఒకవేళ (R)
      R[0] = remainder;
  } లేకపోతే {
    // Now we're ready to invoke the Knuth classical divide algorithm. In this
    // case n > 1.
    KnuthDiv(U, V, Q, R, m, n);
  }

  // If the caller wants the quotient
  ఒకవేళ (Quotient) {
    కొరకు (గుర్తులేని i = 0; i < lhs_పదాలు; ++i)
      Quotient[i] = Make_64(Q[i*2+1], Q[i*2]);
  }

  // If the caller wants the remainder
  ఒకవేళ (Remainder) {
    కొరకు (గుర్తులేని i = 0; i < rhs_పదాలు; ++i)
      Remainder[i] = Make_64(R[i*2+1], R[i*2]);
  }

  // Clean up the memory we allocated.
  ఒకవేళ (U != &SPACE[0]) {
    తొలగించు [] U;
    తొలగించు [] V;
    తొలగించు [] Q;
    తొలగించు [] R;
  }
}

APInt APInt::udiv(స్థిరరాశి APInt &RHS) స్థిరరాశి {
  ప్రతిజ్ఞా(బిట్టు_వెడల్పు_2 == RHS.బిట్టు_వెడల్పు_2 && "Bit widths must be the same");

  // First, deal with the easy case
  ఒకవేళ (LLVM_LIKELY(is_Single_మాట())) {
    ప్రతిజ్ఞా(RHS.U.VAL != 0 && "Divide by zero?");
    తిరిగి APInt(బిట్టు_వెడల్పు_2, U.VAL / RHS.U.VAL);
  }

  // Get some facts about the LHS and RHS number of bits and words
  గుర్తులేని lhs_పదాలు = get_అంకె_పదాలు(get_చురుకైన_బిట్లు());
  గుర్తులేని rhs_బిట్లు  = RHS.get_చురుకైన_బిట్లు();
  గుర్తులేని rhs_పదాలు = get_అంకె_పదాలు(rhs_బిట్లు);
  ప్రతిజ్ఞా(rhs_పదాలు && "Divided by zero???");

  // Deal with some degenerate cases
  ఒకవేళ (!lhs_పదాలు)
    // 0 / X ===> 0
    తిరిగి APInt(బిట్టు_వెడల్పు_2, 0);
  ఒకవేళ (rhs_బిట్లు == 1)
    // X / 1 ===> X
    తిరిగి *ఇది;
  ఒకవేళ (lhs_పదాలు < rhs_పదాలు || ఇది->ult(RHS))
    // X / Y ===> 0, iff X < Y
    తిరిగి APInt(బిట్టు_వెడల్పు_2, 0);
  ఒకవేళ (*ఇది == RHS)
    // X / X ===> 1
    తిరిగి APInt(బిట్టు_వెడల్పు_2, 1);
  ఒకవేళ (lhs_పదాలు == 1) // rhsWords is 1 if lhsWords is 1.
    // All high words are zero, just use native divide
    తిరిగి APInt(బిట్టు_వెడల్పు_2, ఇది->U.pVal[0] / RHS.U.pVal[0]);

  // We have to compute it the hard way. Invoke the Knuth divide algorithm.
  APInt Quotient(బిట్టు_వెడల్పు_2, 0); // to hold result.
  divide(U.pVal, lhs_పదాలు, RHS.U.pVal, rhs_పదాలు, Quotient.U.pVal, శూన్యసూచిక);
  తిరిగి Quotient;
}

APInt APInt::udiv(uint64_t RHS) స్థిరరాశి {
  ప్రతిజ్ఞా(RHS != 0 && "Divide by zero?");

  // First, deal with the easy case
  ఒకవేళ (LLVM_LIKELY(is_Single_మాట()))
    తిరిగి APInt(బిట్టు_వెడల్పు_2, U.VAL / RHS);

  // Get some facts about the LHS words.
  గుర్తులేని lhs_పదాలు = get_అంకె_పదాలు(get_చురుకైన_బిట్లు());

  // Deal with some degenerate cases
  ఒకవేళ (!lhs_పదాలు)
    // 0 / X ===> 0
    తిరిగి APInt(బిట్టు_వెడల్పు_2, 0);
  ఒకవేళ (RHS == 1)
    // X / 1 ===> X
    తిరిగి *ఇది;
  ఒకవేళ (ఇది->ult(RHS))
    // X / Y ===> 0, iff X < Y
    తిరిగి APInt(బిట్టు_వెడల్పు_2, 0);
  ఒకవేళ (*ఇది == RHS)
    // X / X ===> 1
    తిరిగి APInt(బిట్టు_వెడల్పు_2, 1);
  ఒకవేళ (lhs_పదాలు == 1) // rhsWords is 1 if lhsWords is 1.
    // All high words are zero, just use native divide
    తిరిగి APInt(బిట్టు_వెడల్పు_2, ఇది->U.pVal[0] / RHS);

  // We have to compute it the hard way. Invoke the Knuth divide algorithm.
  APInt Quotient(బిట్టు_వెడల్పు_2, 0); // to hold result.
  divide(U.pVal, lhs_పదాలు, &RHS, 1, Quotient.U.pVal, శూన్యసూచిక);
  తిరిగి Quotient;
}

APInt APInt::sdiv(స్థిరరాశి APInt &RHS) స్థిరరాశి {
  ఒకవేళ (isNegative()) {
    ఒకవేళ (RHS.isNegative())
      తిరిగి (-(*ఇది)).udiv(-RHS);
    తిరిగి -((-(*ఇది)).udiv(RHS));
  }
  ఒకవేళ (RHS.isNegative())
    తిరిగి -(ఇది->udiv(-RHS));
  తిరిగి ఇది->udiv(RHS);
}

APInt APInt::sdiv(int64_t RHS) స్థిరరాశి {
  ఒకవేళ (isNegative()) {
    ఒకవేళ (RHS < 0)
      తిరిగి (-(*ఇది)).udiv(-RHS);
    తిరిగి -((-(*ఇది)).udiv(RHS));
  }
  ఒకవేళ (RHS < 0)
    తిరిగి -(ఇది->udiv(-RHS));
  తిరిగి ఇది->udiv(RHS);
}

APInt APInt::urem(స్థిరరాశి APInt &RHS) స్థిరరాశి {
  ప్రతిజ్ఞా(బిట్టు_వెడల్పు_2 == RHS.బిట్టు_వెడల్పు_2 && "Bit widths must be the same");
  ఒకవేళ (LLVM_LIKELY(is_Single_మాట())) {
    ప్రతిజ్ఞా(RHS.U.VAL != 0 && "Remainder by zero?");
    తిరిగి APInt(బిట్టు_వెడల్పు_2, U.VAL % RHS.U.VAL);
  }

  // Get some facts about the LHS
  గుర్తులేని lhs_పదాలు = get_అంకె_పదాలు(get_చురుకైన_బిట్లు());

  // Get some facts about the RHS
  గుర్తులేని rhs_బిట్లు = RHS.get_చురుకైన_బిట్లు();
  గుర్తులేని rhs_పదాలు = get_అంకె_పదాలు(rhs_బిట్లు);
  ప్రతిజ్ఞా(rhs_పదాలు && "Performing remainder operation by zero ???");

  // Check the degenerate cases
  ఒకవేళ (lhs_పదాలు == 0)
    // 0 % Y ===> 0
    తిరిగి APInt(బిట్టు_వెడల్పు_2, 0);
  ఒకవేళ (rhs_బిట్లు == 1)
    // X % 1 ===> 0
    తిరిగి APInt(బిట్టు_వెడల్పు_2, 0);
  ఒకవేళ (lhs_పదాలు < rhs_పదాలు || ఇది->ult(RHS))
    // X % Y ===> X, iff X < Y
    తిరిగి *ఇది;
  ఒకవేళ (*ఇది == RHS)
    // X % X == 0;
    తిరిగి APInt(బిట్టు_వెడల్పు_2, 0);
  ఒకవేళ (lhs_పదాలు == 1)
    // All high words are zero, just use native remainder
    తిరిగి APInt(బిట్టు_వెడల్పు_2, U.pVal[0] % RHS.U.pVal[0]);
  ఒకవేళ (RHS.isPowerOf2()) {
    // X % 2^w ===> X & (2^w - 1)
    APInt ఫలితం(*ఇది);
    ఫలితం.clear_బిట్లు(RHS.logBase2(), బిట్టు_వెడల్పు_2);
    తిరిగి ఫలితం;
  }

  // We have to compute it the hard way. Invoke the Knuth divide algorithm.
  APInt Remainder(బిట్టు_వెడల్పు_2, 0);
  divide(U.pVal, lhs_పదాలు, RHS.U.pVal, rhs_పదాలు, శూన్యసూచిక, Remainder.U.pVal);
  తిరిగి Remainder;
}

uint64_t APInt::urem(uint64_t RHS) స్థిరరాశి {
  ప్రతిజ్ఞా(RHS != 0 && "Remainder by zero?");

  ఒకవేళ (LLVM_LIKELY(is_Single_మాట()))
    తిరిగి U.VAL % RHS;

  // Get some facts about the LHS
  గుర్తులేని lhs_పదాలు = get_అంకె_పదాలు(get_చురుకైన_బిట్లు());

  // Check the degenerate cases
  ఒకవేళ (lhs_పదాలు == 0)
    // 0 % Y ===> 0
    తిరిగి 0;
  ఒకవేళ (RHS == 1)
    // X % 1 ===> 0
    తిరిగి 0;
  ఒకవేళ (ఇది->ult(RHS))
    // X % Y ===> X, iff X < Y
    తిరిగి get_ZExt_విలువ();
  ఒకవేళ (*ఇది == RHS)
    // X % X == 0;
    తిరిగి 0;
  ఒకవేళ (lhs_పదాలు == 1)
    // All high words are zero, just use native remainder
    తిరిగి U.pVal[0] % RHS;
  ఒకవేళ (llvm::isPowerOf2_64(RHS))
    // X % 2^w ===> X & (2^w - 1)
    తిరిగి U.pVal[0] & (RHS - 1);

  // We have to compute it the hard way. Invoke the Knuth divide algorithm.
  uint64_t Remainder;
  divide(U.pVal, lhs_పదాలు, &RHS, 1, శూన్యసూచిక, &Remainder);
  తిరిగి Remainder;
}

APInt APInt::srem(స్థిరరాశి APInt &RHS) స్థిరరాశి {
  ఒకవేళ (isNegative()) {
    ఒకవేళ (RHS.isNegative())
      తిరిగి -((-(*ఇది)).urem(-RHS));
    తిరిగి -((-(*ఇది)).urem(RHS));
  }
  ఒకవేళ (RHS.isNegative())
    తిరిగి ఇది->urem(-RHS);
  తిరిగి ఇది->urem(RHS);
}

int64_t APInt::srem(int64_t RHS) స్థిరరాశి {
  ఒకవేళ (isNegative()) {
    ఒకవేళ (RHS < 0)
      తిరిగి -((-(*ఇది)).urem(-RHS));
    తిరిగి -((-(*ఇది)).urem(RHS));
  }
  ఒకవేళ (RHS < 0)
    తిరిగి ఇది->urem(-RHS);
  తిరిగి ఇది->urem(RHS);
}

శూన్య APInt::udivrem(స్థిరరాశి APInt &LHS, స్థిరరాశి APInt &RHS,
                    APInt &Quotient, APInt &Remainder) {
  ప్రతిజ్ఞా(LHS.బిట్టు_వెడల్పు_2 == RHS.బిట్టు_వెడల్పు_2 && "Bit widths must be the same");
  గుర్తులేని బిట్టు_వెడల్పు_2 = LHS.బిట్టు_వెడల్పు_2;

  // First, deal with the easy case
  ఒకవేళ (LLVM_LIKELY(LHS.is_Single_మాట())) {
    ప్రతిజ్ఞా(RHS.U.VAL != 0 && "Divide by zero?");
    uint64_t QuotVal = LHS.U.VAL / RHS.U.VAL;
    uint64_t RemVal = LHS.U.VAL % RHS.U.VAL;
    Quotient = APInt(బిట్టు_వెడల్పు_2, QuotVal);
    Remainder = APInt(బిట్టు_వెడల్పు_2, RemVal);
    తిరిగి;
  }

  // Get some size facts about the dividend and divisor
  గుర్తులేని lhs_పదాలు = get_అంకె_పదాలు(LHS.get_చురుకైన_బిట్లు());
  గుర్తులేని rhs_బిట్లు  = RHS.get_చురుకైన_బిట్లు();
  గుర్తులేని rhs_పదాలు = get_అంకె_పదాలు(rhs_బిట్లు);
  ప్రతిజ్ఞా(rhs_పదాలు && "Performing divrem operation by zero ???");

  // Check the degenerate cases
  ఒకవేళ (lhs_పదాలు == 0) {
    Quotient = APInt(బిట్టు_వెడల్పు_2, 0);    // 0 / Y ===> 0
    Remainder = APInt(బిట్టు_వెడల్పు_2, 0);   // 0 % Y ===> 0
    తిరిగి;
  }

  ఒకవేళ (rhs_బిట్లు == 1) {
    Quotient = LHS;                   // X / 1 ===> X
    Remainder = APInt(బిట్టు_వెడల్పు_2, 0);   // X % 1 ===> 0
  }

  ఒకవేళ (lhs_పదాలు < rhs_పదాలు || LHS.ult(RHS)) {
    Remainder = LHS;                  // X % Y ===> X, iff X < Y
    Quotient = APInt(బిట్టు_వెడల్పు_2, 0);    // X / Y ===> 0, iff X < Y
    తిరిగి;
  }

  ఒకవేళ (LHS == RHS) {
    Quotient  = APInt(బిట్టు_వెడల్పు_2, 1);   // X / X ===> 1
    Remainder = APInt(బిట్టు_వెడల్పు_2, 0);   // X % X ===> 0;
    తిరిగి;
  }

  // Make sure there is enough space to hold the results.
  // NOTE: This assumes that reallocate won't affect any bits if it doesn't
  // change the size. This is necessary if Quotient or Remainder is aliased
  // with LHS or RHS.
  Quotient.reallocate(బిట్టు_వెడల్పు_2);
  Remainder.reallocate(బిట్టు_వెడల్పు_2);

  ఒకవేళ (lhs_పదాలు == 1) { // rhsWords is 1 if lhsWords is 1.
    // There is only one word to consider so use the native versions.
    uint64_t lhs_విలువ = LHS.U.pVal[0];
    uint64_t rhs_విలువ = RHS.U.pVal[0];
    Quotient = lhs_విలువ / rhs_విలువ;
    Remainder = lhs_విలువ % rhs_విలువ;
    తిరిగి;
  }

  // Okay, lets do it the long way
  divide(LHS.U.pVal, lhs_పదాలు, RHS.U.pVal, rhs_పదాలు, Quotient.U.pVal,
         Remainder.U.pVal);
  // Clear the rest of the Quotient and Remainder.
  std::memset(Quotient.U.pVal + lhs_పదాలు, 0,
              (get_అంకె_పదాలు(బిట్టు_వెడల్పు_2) - lhs_పదాలు) * APINT_మాట_కొలత);
  std::memset(Remainder.U.pVal + rhs_పదాలు, 0,
              (get_అంకె_పదాలు(బిట్టు_వెడల్పు_2) - rhs_పదాలు) * APINT_మాట_కొలత);
}

శూన్య APInt::udivrem(స్థిరరాశి APInt &LHS, uint64_t RHS, APInt &Quotient,
                    uint64_t &Remainder) {
  ప్రతిజ్ఞా(RHS != 0 && "Divide by zero?");
  గుర్తులేని బిట్టు_వెడల్పు_2 = LHS.బిట్టు_వెడల్పు_2;

  // First, deal with the easy case
  ఒకవేళ (LLVM_LIKELY(LHS.is_Single_మాట())) {
    uint64_t QuotVal = LHS.U.VAL / RHS;
    Remainder = LHS.U.VAL % RHS;
    Quotient = APInt(బిట్టు_వెడల్పు_2, QuotVal);
    తిరిగి;
  }

  // Get some size facts about the dividend and divisor
  గుర్తులేని lhs_పదాలు = get_అంకె_పదాలు(LHS.get_చురుకైన_బిట్లు());

  // Check the degenerate cases
  ఒకవేళ (lhs_పదాలు == 0) {
    Quotient = APInt(బిట్టు_వెడల్పు_2, 0);    // 0 / Y ===> 0
    Remainder = 0;                    // 0 % Y ===> 0
    తిరిగి;
  }

  ఒకవేళ (RHS == 1) {
    Quotient = LHS;                   // X / 1 ===> X
    Remainder = 0;                    // X % 1 ===> 0
    తిరిగి;
  }

  ఒకవేళ (LHS.ult(RHS)) {
    Remainder = LHS.get_ZExt_విలువ();   // X % Y ===> X, iff X < Y
    Quotient = APInt(బిట్టు_వెడల్పు_2, 0);    // X / Y ===> 0, iff X < Y
    తిరిగి;
  }

  ఒకవేళ (LHS == RHS) {
    Quotient  = APInt(బిట్టు_వెడల్పు_2, 1);   // X / X ===> 1
    Remainder = 0;                    // X % X ===> 0;
    తిరిగి;
  }

  // Make sure there is enough space to hold the results.
  // NOTE: This assumes that reallocate won't affect any bits if it doesn't
  // change the size. This is necessary if Quotient is aliased with LHS.
  Quotient.reallocate(బిట్టు_వెడల్పు_2);

  ఒకవేళ (lhs_పదాలు == 1) { // rhsWords is 1 if lhsWords is 1.
    // There is only one word to consider so use the native versions.
    uint64_t lhs_విలువ = LHS.U.pVal[0];
    Quotient = lhs_విలువ / RHS;
    Remainder = lhs_విలువ % RHS;
    తిరిగి;
  }

  // Okay, lets do it the long way
  divide(LHS.U.pVal, lhs_పదాలు, &RHS, 1, Quotient.U.pVal, &Remainder);
  // Clear the rest of the Quotient.
  std::memset(Quotient.U.pVal + lhs_పదాలు, 0,
              (get_అంకె_పదాలు(బిట్టు_వెడల్పు_2) - lhs_పదాలు) * APINT_మాట_కొలత);
}

శూన్య APInt::sdivrem(స్థిరరాశి APInt &LHS, స్థిరరాశి APInt &RHS,
                    APInt &Quotient, APInt &Remainder) {
  ఒకవేళ (LHS.isNegative()) {
    ఒకవేళ (RHS.isNegative())
      APInt::udivrem(-LHS, -RHS, Quotient, Remainder);
    లేకపోతే {
      APInt::udivrem(-LHS, RHS, Quotient, Remainder);
      Quotient.negate();
    }
    Remainder.negate();
  } లేకపోతే ఒకవేళ (RHS.isNegative()) {
    APInt::udivrem(LHS, -RHS, Quotient, Remainder);
    Quotient.negate();
  } లేకపోతే {
    APInt::udivrem(LHS, RHS, Quotient, Remainder);
  }
}

శూన్య APInt::sdivrem(స్థిరరాశి APInt &LHS, int64_t RHS,
                    APInt &Quotient, int64_t &Remainder) {
  uint64_t R = Remainder;
  ఒకవేళ (LHS.isNegative()) {
    ఒకవేళ (RHS < 0)
      APInt::udivrem(-LHS, -RHS, Quotient, R);
    లేకపోతే {
      APInt::udivrem(-LHS, RHS, Quotient, R);
      Quotient.negate();
    }
    R = -R;
  } లేకపోతే ఒకవేళ (RHS < 0) {
    APInt::udivrem(LHS, -RHS, Quotient, R);
    Quotient.negate();
  } లేకపోతే {
    APInt::udivrem(LHS, RHS, Quotient, R);
  }
  Remainder = R;
}

APInt APInt::sadd_ov(స్థిరరాశి APInt &RHS, తార్కిక &అధికప్రవాహం) స్థిరరాశి {
  APInt Res = *ఇది+RHS;
  అధికప్రవాహం = isNonNegative() == RHS.isNonNegative() &&
             Res.isNonNegative() != isNonNegative();
  తిరిగి Res;
}

APInt APInt::uadd_ov(స్థిరరాశి APInt &RHS, తార్కిక &అధికప్రవాహం) స్థిరరాశి {
  APInt Res = *ఇది+RHS;
  అధికప్రవాహం = Res.ult(RHS);
  తిరిగి Res;
}

APInt APInt::ssub_ov(స్థిరరాశి APInt &RHS, తార్కిక &అధికప్రవాహం) స్థిరరాశి {
  APInt Res = *ఇది - RHS;
  అధికప్రవాహం = isNonNegative() != RHS.isNonNegative() &&
             Res.isNonNegative() != isNonNegative();
  తిరిగి Res;
}

APInt APInt::usub_ov(స్థిరరాశి APInt &RHS, తార్కిక &అధికప్రవాహం) స్థిరరాశి {
  APInt Res = *ఇది-RHS;
  అధికప్రవాహం = Res.ugt(*ఇది);
  తిరిగి Res;
}

APInt APInt::sdiv_ov(స్థిరరాశి APInt &RHS, తార్కిక &అధికప్రవాహం) స్థిరరాశి {
  // MININT/-1  -->  overflow.
  అధికప్రవాహం = is_Min_Signed_విలువ() && RHS.isAllOnes();
  తిరిగి sdiv(RHS);
}

APInt APInt::smul_ov(స్థిరరాశి APInt &RHS, తార్కిక &అధికప్రవాహం) స్థిరరాశి {
  APInt Res = *ఇది * RHS;

  ఒకవేళ (RHS != 0)
    అధికప్రవాహం = Res.sdiv(RHS) != *ఇది ||
               (is_Min_Signed_విలువ() && RHS.isAllOnes());
  లేకపోతే
    అధికప్రవాహం = అబద్ధం;
  తిరిగి Res;
}

APInt APInt::umul_ov(స్థిరరాశి APInt &RHS, తార్కిక &అధికప్రవాహం) స్థిరరాశి {
  ఒకవేళ (countl_zero() + RHS.countl_zero() + 2 <= బిట్టు_వెడల్పు_2) {
    అధికప్రవాహం = నిజం;
    తిరిగి *ఇది * RHS;
  }

  APInt Res = lshr(1) * RHS;
  అధికప్రవాహం = Res.isNegative();
  Res <<= 1;
  ఒకవేళ ((*ఇది)[0]) {
    Res += RHS;
    ఒకవేళ (Res.ult(RHS))
      అధికప్రవాహం = నిజం;
  }
  తిరిగి Res;
}

APInt APInt::sshl_ov(స్థిరరాశి APInt &ShAmt, తార్కిక &అధికప్రవాహం) స్థిరరాశి {
  తిరిగి sshl_ov(ShAmt.get_Limited_విలువ(get_బిట్టు_వెడల్పు()), అధికప్రవాహం);
}

APInt APInt::sshl_ov(గుర్తులేని ShAmt, తార్కిక &అధికప్రవాహం) స్థిరరాశి {
  అధికప్రవాహం = ShAmt >= get_బిట్టు_వెడల్పు();
  ఒకవేళ (అధికప్రవాహం)
    తిరిగి APInt(బిట్టు_వెడల్పు_2, 0);

  ఒకవేళ (isNonNegative()) // Don't allow sign change.
    అధికప్రవాహం = ShAmt >= countl_zero();
  లేకపోతే
    అధికప్రవాహం = ShAmt >= countl_one();

  తిరిగి *ఇది << ShAmt;
}

APInt APInt::ushl_ov(స్థిరరాశి APInt &ShAmt, తార్కిక &అధికప్రవాహం) స్థిరరాశి {
  తిరిగి ushl_ov(ShAmt.get_Limited_విలువ(get_బిట్టు_వెడల్పు()), అధికప్రవాహం);
}

APInt APInt::ushl_ov(గుర్తులేని ShAmt, తార్కిక &అధికప్రవాహం) స్థిరరాశి {
  అధికప్రవాహం = ShAmt >= get_బిట్టు_వెడల్పు();
  ఒకవేళ (అధికప్రవాహం)
    తిరిగి APInt(బిట్టు_వెడల్పు_2, 0);

  అధికప్రవాహం = ShAmt > countl_zero();

  తిరిగి *ఇది << ShAmt;
}

APInt APInt::sfloordiv_ov(స్థిరరాశి APInt &RHS, తార్కిక &అధికప్రవాహం) స్థిరరాశి {
  APInt quotient = sdiv_ov(RHS, అధికప్రవాహం);
  ఒకవేళ ((quotient * RHS != *ఇది) && (isNegative() != RHS.isNegative()))
    తిరిగి quotient - 1;
  తిరిగి quotient;
}

APInt APInt::sadd_sat(స్థిరరాశి APInt &RHS) స్థిరరాశి {
  తార్కిక అధికప్రవాహం;
  APInt Res = sadd_ov(RHS, అధికప్రవాహం);
  ఒకవేళ (!అధికప్రవాహం)
    తిరిగి Res;

  తిరిగి isNegative() ? APInt::get_Signed_Min_విలువ(బిట్టు_వెడల్పు_2)
                      : APInt::get_Signed_Max_విలువ(బిట్టు_వెడల్పు_2);
}

APInt APInt::uadd_sat(స్థిరరాశి APInt &RHS) స్థిరరాశి {
  తార్కిక అధికప్రవాహం;
  APInt Res = uadd_ov(RHS, అధికప్రవాహం);
  ఒకవేళ (!అధికప్రవాహం)
    తిరిగి Res;

  తిరిగి APInt::get_Max_విలువ(బిట్టు_వెడల్పు_2);
}

APInt APInt::ssub_sat(స్థిరరాశి APInt &RHS) స్థిరరాశి {
  తార్కిక అధికప్రవాహం;
  APInt Res = ssub_ov(RHS, అధికప్రవాహం);
  ఒకవేళ (!అధికప్రవాహం)
    తిరిగి Res;

  తిరిగి isNegative() ? APInt::get_Signed_Min_విలువ(బిట్టు_వెడల్పు_2)
                      : APInt::get_Signed_Max_విలువ(బిట్టు_వెడల్పు_2);
}

APInt APInt::usub_sat(స్థిరరాశి APInt &RHS) స్థిరరాశి {
  తార్కిక అధికప్రవాహం;
  APInt Res = usub_ov(RHS, అధికప్రవాహం);
  ఒకవేళ (!అధికప్రవాహం)
    తిరిగి Res;

  తిరిగి APInt(బిట్టు_వెడల్పు_2, 0);
}

APInt APInt::smul_sat(స్థిరరాశి APInt &RHS) స్థిరరాశి {
  తార్కిక అధికప్రవాహం;
  APInt Res = smul_ov(RHS, అధికప్రవాహం);
  ఒకవేళ (!అధికప్రవాహం)
    తిరిగి Res;

  // The result is negative if one and only one of inputs is negative.
  తార్కిక ResIsNegative = isNegative() ^ RHS.isNegative();

  తిరిగి ResIsNegative ? APInt::get_Signed_Min_విలువ(బిట్టు_వెడల్పు_2)
                       : APInt::get_Signed_Max_విలువ(బిట్టు_వెడల్పు_2);
}

APInt APInt::umul_sat(స్థిరరాశి APInt &RHS) స్థిరరాశి {
  తార్కిక అధికప్రవాహం;
  APInt Res = umul_ov(RHS, అధికప్రవాహం);
  ఒకవేళ (!అధికప్రవాహం)
    తిరిగి Res;

  తిరిగి APInt::get_Max_విలువ(బిట్టు_వెడల్పు_2);
}

APInt APInt::sshl_sat(స్థిరరాశి APInt &RHS) స్థిరరాశి {
  తిరిగి sshl_sat(RHS.get_Limited_విలువ(get_బిట్టు_వెడల్పు()));
}

APInt APInt::sshl_sat(గుర్తులేని RHS) స్థిరరాశి {
  తార్కిక అధికప్రవాహం;
  APInt Res = sshl_ov(RHS, అధికప్రవాహం);
  ఒకవేళ (!అధికప్రవాహం)
    తిరిగి Res;

  తిరిగి isNegative() ? APInt::get_Signed_Min_విలువ(బిట్టు_వెడల్పు_2)
                      : APInt::get_Signed_Max_విలువ(బిట్టు_వెడల్పు_2);
}

APInt APInt::ushl_sat(స్థిరరాశి APInt &RHS) స్థిరరాశి {
  తిరిగి ushl_sat(RHS.get_Limited_విలువ(get_బిట్టు_వెడల్పు()));
}

APInt APInt::ushl_sat(గుర్తులేని RHS) స్థిరరాశి {
  తార్కిక అధికప్రవాహం;
  APInt Res = ushl_ov(RHS, అధికప్రవాహం);
  ఒకవేళ (!అధికప్రవాహం)
    తిరిగి Res;

  తిరిగి APInt::get_Max_విలువ(బిట్టు_వెడల్పు_2);
}

శూన్య APInt::fromString(గుర్తులేని numbits, StringRef str, uint8_t radix) {
  // Check our assumptions here
  ప్రతిజ్ఞా(!str.ఖాళీలు() && "Invalid string length");
  ప్రతిజ్ఞా((radix == 10 || radix == 8 || radix == 16 || radix == 2 ||
          radix == 36) &&
         "Radix should be 2, 8, 10, 16, or 36!");

  StringRef::iterator p = str.begin();
  size_t slen = str.కొలత_2();
  తార్కిక isNeg = *p == '-';
  ఒకవేళ (*p == '-' || *p == '+') {
    p++;
    slen--;
    ప్రతిజ్ఞా(slen && "String is only a sign, needs a value.");
  }
  ప్రతిజ్ఞా((slen <= numbits || radix != 2) && "Insufficient bit width");
  ప్రతిజ్ఞా(((slen-1)*3 <= numbits || radix != 8) && "Insufficient bit width");
  ప్రతిజ్ఞా(((slen-1)*4 <= numbits || radix != 16) && "Insufficient bit width");
  ప్రతిజ్ఞా((((slen-1)*64)/22 <= numbits || radix != 10) &&
         "Insufficient bit width");

  // Allocate memory if needed
  ఒకవేళ (LLVM_LIKELY(is_Single_మాట()))
    U.VAL = 0;
  లేకపోతే
    U.pVal = get_Cleared_జ్ఞాపకం(get_అంకె_పదాలు());

  // Figure out if we can shift instead of multiply
  గుర్తులేని జరుపు = (radix == 16 ? 4 : radix == 8 ? 3 : radix == 2 ? 1 : 0);

  // Enter digit traversal loop
  కొరకు (StringRef::iterator e = str.చివర(); p != e; ++p) {
    గుర్తులేని digit = getDigit(*p, radix);
    ప్రతిజ్ఞా(digit < radix && "Invalid character in digit string");

    // Shift or multiply the value by the radix
    ఒకవేళ (slen > 1) {
      ఒకవేళ (జరుపు)
        *ఇది <<= జరుపు;
      లేకపోతే
        *ఇది *= radix;
    }

    // Add in the digit we just interpreted
    *ఇది += digit;
  }
  // If its negative, put it in two's complement form
  ఒకవేళ (isNeg)
    ఇది->negate();
}

శూన్య APInt::గమ్యం_String(Small_సదిశ_Impl<అక్షరం> &Str, గుర్తులేని Radix, తార్కిక Signed,
                     తార్కిక formatAsCLiteral, తార్కిక పై_Case_2,
                     తార్కిక చొప్పించు_Separators) స్థిరరాశి {
  ప్రతిజ్ఞా((Radix == 10 || Radix == 8 || Radix == 16 || Radix == 2 ||
          Radix == 36) &&
         "Radix should be 2, 8, 10, 16, or 36!");

  స్థిరరాశి అక్షరం *ఉపసర్గ = "";
  ఒకవేళ (formatAsCLiteral) {
    ఎంపిక (Radix) {
      స్థితి 2:
        // Binary literals are a non-standard extension added in gcc 4.3:
        // http://gcc.gnu.org/onlinedocs/gcc-4.3.0/gcc/Binary-constants.html
        ఉపసర్గ = "0b";
        ఆపు;
      స్థితి 8:
        ఉపసర్గ = "0";
        ఆపు;
      స్థితి 10:
        ఆపు; // No prefix
      స్థితి 16:
        ఉపసర్గ = "0x";
        ఆపు;
      అప్రమేయ:
        llvm_unreachable("Invalid radix!");
    }
  }

  // Number of digits in a group between separators.
  గుర్తులేని Grouping = (Radix == 8 || Radix == 10) ? 3 : 4;

  // First, check for a zero value and just short circuit the logic below.
  ఒకవేళ (isZero()) {
    అయితే (*ఉపసర్గ) {
      Str.నెట్టు_వెనుక(*ఉపసర్గ);
      ++ఉపసర్గ;
    };
    Str.నెట్టు_వెనుక('0');
    తిరిగి;
  }

  స్థిర స్థిరరాశి అక్షరం BothDigits[] = "0123456789abcdefghijklmnopqrstuvwxyz"
                                   "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ";
  స్థిరరాశి అక్షరం *Digits = BothDigits + (పై_Case_2 ? 36 : 0);

  ఒకవేళ (LLVM_LIKELY(is_Single_మాట())) {
    అక్షరం బఫరు[65];
    అక్షరం *BufPtr = std::చివర(బఫరు);

    uint64_t N;
    ఒకవేళ (!Signed) {
      N = get_ZExt_విలువ();
    } లేకపోతే {
      int64_t I = get_SExt_విలువ();
      ఒకవేళ (I >= 0) {
        N = I;
      } లేకపోతే {
        Str.నెట్టు_వెనుక('-');
        N = -(uint64_t)I;
      }
    }

    అయితే (*ఉపసర్గ) {
      Str.నెట్టు_వెనుక(*ఉపసర్గ);
      ++ఉపసర్గ;
    };

    పూర్ణాంకం Pos = 0;
    అయితే (N) {
      ఒకవేళ (చొప్పించు_Separators && Pos % Grouping == 0 && Pos > 0)
        *--BufPtr = '\'';
      *--BufPtr = Digits[N % Radix];
      N /= Radix;
      Pos++;
    }
    Str.append(BufPtr, std::చివర(బఫరు));
    తిరిగి;
  }

  APInt Tmp(*ఇది);

  ఒకవేళ (Signed && isNegative()) {
    // They want to print the signed version and it is a negative value
    // Flip the bits and add one to turn it into the equivalent positive
    // value and put a '-' in the result.
    Tmp.negate();
    Str.నెట్టు_వెనుక('-');
  }

  అయితే (*ఉపసర్గ) {
    Str.నెట్టు_వెనుక(*ఉపసర్గ);
    ++ఉపసర్గ;
  }

  // We insert the digits backward, then reverse them to get the right order.
  గుర్తులేని ప్రారంభం_Dig = Str.కొలత_2();

  // For the 2, 8 and 16 bit cases, we can just shift instead of divide
  // because the number of bits per digit (1, 3 and 4 respectively) divides
  // equally.  We just shift until the value is zero.
  ఒకవేళ (Radix == 2 || Radix == 8 || Radix == 16) {
    // Just shift tmp right for each digit width until it becomes zero
    గుర్తులేని జరుపు_Amt_2 = (Radix == 16 ? 4 : (Radix == 8 ? 3 : 1));
    గుర్తులేని ముసుగు_Amt = Radix - 1;

    పూర్ణాంకం Pos = 0;
    అయితే (Tmp.get_Bool_విలువ()) {
      గుర్తులేని Digit = గుర్తులేని(Tmp.get_Raw_దత్తాంశం()[0]) & ముసుగు_Amt;
      ఒకవేళ (చొప్పించు_Separators && Pos % Grouping == 0 && Pos > 0)
        Str.నెట్టు_వెనుక('\'');

      Str.నెట్టు_వెనుక(Digits[Digit]);
      Tmp.lshr_లోపల_ఉంచు(జరుపు_Amt_2);
      Pos++;
    }
  } లేకపోతే {
    పూర్ణాంకం Pos = 0;
    అయితే (Tmp.get_Bool_విలువ()) {
      uint64_t Digit;
      udivrem(Tmp, Radix, Tmp, Digit);
      ప్రతిజ్ఞా(Digit < Radix && "divide failed");
      ఒకవేళ (చొప్పించు_Separators && Pos % Grouping == 0 && Pos > 0)
        Str.నెట్టు_వెనుక('\'');

      Str.నెట్టు_వెనుక(Digits[Digit]);
      Pos++;
    }
  }

  // Reverse the digits before returning.
  std::తిరగేయి(Str.begin()+ప్రారంభం_Dig, Str.చివర());
}

#ఒకవేళ !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)
LLVM_DUMP_పద్ధతి శూన్య APInt::dump() స్థిరరాశి {
  SmallString<40> S, U;
  ఇది->గమ్యం_String_Unsigned(U);
  ఇది->గమ్యం_String_Signed(S);
  dbgs() << "APInt(" << బిట్టు_వెడల్పు_2 << "b, "
         << U << "u " << S << "s)\n";
}
#అయితే_ముగింపు

శూన్య APInt::print(raw_ostream &OS, తార్కిక isSigned) స్థిరరాశి {
  SmallString<40> S;
  ఇది->గమ్యం_String(S, 10, isSigned, /* formatAsCLiteral = */అబద్ధం);
  OS << S;
}

// This implements a variety of operations on a representation of
// arbitrary precision, two's-complement, bignum integer values.

// Assumed by lowHalf, highHalf, partMSB and partLSB.  A fairly safe
// and unrestricting assumption.
static_assert(APInt::APINT_బిట్లు_చొప్పున_మాట % 2 == 0,
              "Part width must be divisible by 2!");

// Returns the integer part with the least significant BITS set.
// BITS cannot be zero.
స్థిర అంతఃవరుస APInt::మాట_Type దిగువ_బిట్టు_ముసుగు(గుర్తులేని బిట్లు) {
  ప్రతిజ్ఞా(బిట్లు != 0 && బిట్లు <= APInt::APINT_బిట్లు_చొప్పున_మాట);
  తిరిగి ~(APInt::మాట_Type) 0 >> (APInt::APINT_బిట్లు_చొప్పున_మాట - బిట్లు);
}

/// Returns the value of the lower half of PART.
స్థిర అంతఃవరుస APInt::మాట_Type దిగువ_Half(APInt::మాట_Type part) {
  తిరిగి part & దిగువ_బిట్టు_ముసుగు(APInt::APINT_బిట్లు_చొప్పున_మాట / 2);
}

/// Returns the value of the upper half of PART.
స్థిర అంతఃవరుస APInt::మాట_Type ఎగువ_Half(APInt::మాట_Type part) {
  తిరిగి part >> (APInt::APINT_బిట్లు_చొప్పున_మాట / 2);
}

/// Sets the least significant part of a bignum to the input value, and zeroes
/// out higher parts.
శూన్య APInt::tc_సమితి(మాట_Type *dst, మాట_Type part, గుర్తులేని parts) {
  ప్రతిజ్ఞా(parts > 0);
  dst[0] = part;
  కొరకు (గుర్తులేని i = 1; i < parts; i++)
    dst[i] = 0;
}

/// Assign one bignum to another.
శూన్య APInt::tc_కేటాయించు(మాట_Type *dst, స్థిరరాశి మాట_Type *src, గుర్తులేని parts) {
  కొరకు (గుర్తులేని i = 0; i < parts; i++)
    dst[i] = src[i];
}

/// Returns true if a bignum is zero, false otherwise.
తార్కిక APInt::tcIsZero(స్థిరరాశి మాట_Type *src, గుర్తులేని parts) {
  కొరకు (గుర్తులేని i = 0; i < parts; i++)
    ఒకవేళ (src[i])
      తిరిగి అబద్ధం;

  తిరిగి నిజం;
}

/// Extract the given bit of a bignum; returns 0 or 1.
పూర్ణాంకం APInt::tc_వెలికితీయి_బిట్టు(స్థిరరాశి మాట_Type *parts, గుర్తులేని బిట్టు) {
  తిరిగి (parts[which_మాట(బిట్టు)] & ముసుగు_బిట్టు(బిట్టు)) != 0;
}

/// Set the given bit of a bignum.
శూన్య APInt::tc_సమితి_బిట్టు(మాట_Type *parts, గుర్తులేని బిట్టు) {
  parts[which_మాట(బిట్టు)] |= ముసుగు_బిట్టు(బిట్టు);
}

/// Clears the given bit of a bignum.
శూన్య APInt::tc_Clear_బిట్టు(మాట_Type *parts, గుర్తులేని బిట్టు) {
  parts[which_మాట(బిట్టు)] &= ~ముసుగు_బిట్టు(బిట్టు);
}

/// Returns the bit number of the least significant set bit of a number.  If the
/// input number has no bits set UINT_MAX is returned.
గుర్తులేని APInt::tcLSB(స్థిరరాశి మాట_Type *parts, గుర్తులేని n) {
  కొరకు (గుర్తులేని i = 0; i < n; i++) {
    ఒకవేళ (parts[i] != 0) {
      గుర్తులేని lsb = llvm::countr_zero(parts[i]);
      తిరిగి lsb + i * APINT_బిట్లు_చొప్పున_మాట;
    }
  }

  తిరిగి UINT_MAX;
}

/// Returns the bit number of the most significant set bit of a number.
/// If the input number has no bits set UINT_MAX is returned.
గుర్తులేని APInt::tcMSB(స్థిరరాశి మాట_Type *parts, గుర్తులేని n) {
  చేయి {
    --n;

    ఒకవేళ (parts[n] != 0) {
      static_assert(పరిమాణం(parts[n]) <= పరిమాణం(uint64_t));
      గుర్తులేని msb = llvm::Log2_64(parts[n]);

      తిరిగి msb + n * APINT_బిట్లు_చొప్పున_మాట;
    }
  } అయితే (n);

  తిరిగి UINT_MAX;
}

/// Copy the bit vector of width srcBITS from SRC, starting at bit srcLSB, to
/// DST, of dstCOUNT parts, such that the bit srcLSB becomes the least
/// significant bit of DST.  All high bits above srcBITS in DST are zero-filled.
/// */
శూన్య
APInt::tc_వెలికితీయి(మాట_Type *dst, గుర్తులేని dst_లెక్క, స్థిరరాశి మాట_Type *src,
                 గుర్తులేని src_బిట్లు, గుర్తులేని srcLSB) {
  గుర్తులేని dstParts = (src_బిట్లు + APINT_బిట్లు_చొప్పున_మాట - 1) / APINT_బిట్లు_చొప్పున_మాట;
  ప్రతిజ్ఞా(dstParts <= dst_లెక్క);

  గుర్తులేని మొదటి_Src_Part = srcLSB / APINT_బిట్లు_చొప్పున_మాట;
  tc_కేటాయించు(dst, src + మొదటి_Src_Part, dstParts);

  గుర్తులేని జరుపు = srcLSB % APINT_బిట్లు_చొప్పున_మాట;
  tc_జరుపు_కుడి(dst, dstParts, జరుపు);

  // We now have (dstParts * APINT_BITS_PER_WORD - shift) bits from SRC
  // in DST.  If this is less that srcBits, append the rest, else
  // clear the high bits.
  గుర్తులేని n = dstParts * APINT_బిట్లు_చొప్పున_మాట - జరుపు;
  ఒకవేళ (n < src_బిట్లు) {
    మాట_Type ముసుగు_2 = దిగువ_బిట్టు_ముసుగు (src_బిట్లు - n);
    dst[dstParts - 1] |= ((src[మొదటి_Src_Part + dstParts] & ముసుగు_2)
                          << n % APINT_బిట్లు_చొప్పున_మాట);
  } లేకపోతే ఒకవేళ (n > src_బిట్లు) {
    ఒకవేళ (src_బిట్లు % APINT_బిట్లు_చొప్పున_మాట)
      dst[dstParts - 1] &= దిగువ_బిట్టు_ముసుగు (src_బిట్లు % APINT_బిట్లు_చొప్పున_మాట);
  }

  // Clear high parts.
  అయితే (dstParts < dst_లెక్క)
    dst[dstParts++] = 0;
}

//// DST += RHS + C where C is zero or one.  Returns the carry flag.
APInt::మాట_Type APInt::tc_కూడు(మాట_Type *dst, స్థిరరాశి మాట_Type *rhs,
                             మాట_Type c, గుర్తులేని parts) {
  ప్రతిజ్ఞా(c <= 1);

  కొరకు (గుర్తులేని i = 0; i < parts; i++) {
    మాట_Type l = dst[i];
    ఒకవేళ (c) {
      dst[i] += rhs[i] + 1;
      c = (dst[i] <= l);
    } లేకపోతే {
      dst[i] += rhs[i];
      c = (dst[i] < l);
    }
  }

  తిరిగి c;
}

/// This function adds a single "word" integer, src, to the multiple
/// "word" integer array, dst[]. dst[] is modified to reflect the addition and
/// 1 is returned if there is a carry out, otherwise 0 is returned.
/// @returns the carry of the addition.
APInt::మాట_Type APInt::tc_కూడు_Part(మాట_Type *dst, మాట_Type src,
                                 గుర్తులేని parts) {
  కొరకు (గుర్తులేని i = 0; i < parts; ++i) {
    dst[i] += src;
    ఒకవేళ (dst[i] >= src)
      తిరిగి 0; // No need to carry so exit early.
    src = 1; // Carry one to next digit.
  }

  తిరిగి 1;
}

/// DST -= RHS + C where C is zero or one.  Returns the carry flag.
APInt::మాట_Type APInt::tc_వ్యవకలనం(మాట_Type *dst, స్థిరరాశి మాట_Type *rhs,
                                  మాట_Type c, గుర్తులేని parts) {
  ప్రతిజ్ఞా(c <= 1);

  కొరకు (గుర్తులేని i = 0; i < parts; i++) {
    మాట_Type l = dst[i];
    ఒకవేళ (c) {
      dst[i] -= rhs[i] + 1;
      c = (dst[i] >= l);
    } లేకపోతే {
      dst[i] -= rhs[i];
      c = (dst[i] > l);
    }
  }

  తిరిగి c;
}

/// This function subtracts a single "word" (64-bit word), src, from
/// the multi-word integer array, dst[], propagating the borrowed 1 value until
/// no further borrowing is needed or it runs out of "words" in dst.  The result
/// is 1 if "borrowing" exhausted the digits in dst, or 0 if dst was not
/// exhausted. In other words, if src > dst then this function returns 1,
/// otherwise 0.
/// @returns the borrow out of the subtraction
APInt::మాట_Type APInt::tc_వ్యవకలనం_Part(మాట_Type *dst, మాట_Type src,
                                      గుర్తులేని parts) {
  కొరకు (గుర్తులేని i = 0; i < parts; ++i) {
    మాట_Type Dst = dst[i];
    dst[i] -= src;
    ఒకవేళ (src <= Dst)
      తిరిగి 0; // No need to borrow so exit early.
    src = 1; // We have to "borrow 1" from next "word"
  }

  తిరిగి 1;
}

/// Negate a bignum in-place.
శూన్య APInt::tcNegate(మాట_Type *dst, గుర్తులేని parts) {
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
పూర్ణాంకం APInt::tcMultiplyPart(మాట_Type *dst, స్థిరరాశి మాట_Type *src,
                          మాట_Type multiplier, మాట_Type వాహకం,
                          గుర్తులేని srcParts, గుర్తులేని dstParts,
                          తార్కిక కూడు) {
  // Otherwise our writes of DST kill our later reads of SRC.
  ప్రతిజ్ఞా(dst <= src || dst >= src + srcParts);
  ప్రతిజ్ఞా(dstParts <= srcParts + 1);

  // N loops; minimum of dstParts and srcParts.
  గుర్తులేని n = std::min(dstParts, srcParts);

  కొరకు (గుర్తులేని i = 0; i < n; i++) {
    // [LOW, HIGH] = MULTIPLIER * SRC[i] + DST[i] + CARRY.
    // This cannot overflow, because:
    //   (n - 1) * (n - 1) + 2 (n - 1) = (n - 1) * (n + 1)
    // which is less than n^2.
    మాట_Type srcPart = src[i];
    మాట_Type దిగువ, mid, ఎగువ;
    ఒకవేళ (multiplier == 0 || srcPart == 0) {
      దిగువ = వాహకం;
      ఎగువ = 0;
    } లేకపోతే {
      దిగువ = దిగువ_Half(srcPart) * దిగువ_Half(multiplier);
      ఎగువ = ఎగువ_Half(srcPart) * ఎగువ_Half(multiplier);

      mid = దిగువ_Half(srcPart) * ఎగువ_Half(multiplier);
      ఎగువ += ఎగువ_Half(mid);
      mid <<= APINT_బిట్లు_చొప్పున_మాట / 2;
      ఒకవేళ (దిగువ + mid < దిగువ)
        ఎగువ++;
      దిగువ += mid;

      mid = ఎగువ_Half(srcPart) * దిగువ_Half(multiplier);
      ఎగువ += ఎగువ_Half(mid);
      mid <<= APINT_బిట్లు_చొప్పున_మాట / 2;
      ఒకవేళ (దిగువ + mid < దిగువ)
        ఎగువ++;
      దిగువ += mid;

      // Now add carry.
      ఒకవేళ (దిగువ + వాహకం < దిగువ)
        ఎగువ++;
      దిగువ += వాహకం;
    }

    ఒకవేళ (కూడు) {
      // And now DST[i], and store the new low part there.
      ఒకవేళ (దిగువ + dst[i] < దిగువ)
        ఎగువ++;
      dst[i] += దిగువ;
    } లేకపోతే {
      dst[i] = దిగువ;
    }

    వాహకం = ఎగువ;
  }

  ఒకవేళ (srcParts < dstParts) {
    // Full multiplication, there is no overflow.
    ప్రతిజ్ఞా(srcParts + 1 == dstParts);
    dst[srcParts] = వాహకం;
    తిరిగి 0;
  }

  // We overflowed if there is carry.
  ఒకవేళ (వాహకం)
    తిరిగి 1;

  // We would overflow if any significant unwritten parts would be
  // non-zero.  This is true if any remaining src parts are non-zero
  // and the multiplier is non-zero.
  ఒకవేళ (multiplier)
    కొరకు (గుర్తులేని i = dstParts; i < srcParts; i++)
      ఒకవేళ (src[i])
        తిరిగి 1;

  // We fitted in the narrow destination.
  తిరిగి 0;
}

/// DST = LHS * RHS, where DST has the same width as the operands and
/// is filled with the least significant parts of the result.  Returns
/// one if overflow occurred, otherwise zero.  DST must be disjoint
/// from both operands.
పూర్ణాంకం APInt::tcMultiply(మాట_Type *dst, స్థిరరాశి మాట_Type *lhs,
                      స్థిరరాశి మాట_Type *rhs, గుర్తులేని parts) {
  ప్రతిజ్ఞా(dst != lhs && dst != rhs);

  పూర్ణాంకం అధికప్రవాహం_2 = 0;

  కొరకు (గుర్తులేని i = 0; i < parts; i++) {
    // Don't accumulate on the first iteration so we don't need to initalize
    // dst to 0.
    అధికప్రవాహం_2 |=
        tcMultiplyPart(&dst[i], lhs, rhs[i], 0, parts, parts - i, i != 0);
  }

  తిరిగి అధికప్రవాహం_2;
}

/// DST = LHS * RHS, where DST has width the sum of the widths of the
/// operands. No overflow occurs. DST must be disjoint from both operands.
శూన్య APInt::tc_నిండిన_Multiply(మాట_Type *dst, స్థిరరాశి మాట_Type *lhs,
                           స్థిరరాశి మాట_Type *rhs, గుర్తులేని lhsParts,
                           గుర్తులేని rhsParts) {
  // Put the narrower number on the LHS for less loops below.
  ఒకవేళ (lhsParts > rhsParts)
    తిరిగి tc_నిండిన_Multiply (dst, rhs, lhs, rhsParts, lhsParts);

  ప్రతిజ్ఞా(dst != lhs && dst != rhs);

  కొరకు (గుర్తులేని i = 0; i < lhsParts; i++) {
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
పూర్ణాంకం APInt::tcDivide(మాట_Type *lhs, స్థిరరాశి మాట_Type *rhs,
                    మాట_Type *remainder, మాట_Type *srhs,
                    గుర్తులేని parts) {
  ప్రతిజ్ఞా(lhs != remainder && lhs != srhs && remainder != srhs);

  గుర్తులేని జరుపు_లెక్క = tcMSB(rhs, parts) + 1;
  ఒకవేళ (జరుపు_లెక్క == 0)
    తిరిగి నిజం;

  జరుపు_లెక్క = parts * APINT_బిట్లు_చొప్పున_మాట - జరుపు_లెక్క;
  గుర్తులేని n = జరుపు_లెక్క / APINT_బిట్లు_చొప్పున_మాట;
  మాట_Type ముసుగు_2 = (మాట_Type) 1 << (జరుపు_లెక్క % APINT_బిట్లు_చొప్పున_మాట);

  tc_కేటాయించు(srhs, rhs, parts);
  tc_జరుపు_ఎడమ(srhs, parts, జరుపు_లెక్క);
  tc_కేటాయించు(remainder, lhs, parts);
  tc_సమితి(lhs, 0, parts);

  // Loop, subtracting SRHS if REMAINDER is greater and adding that to the
  // total.
  కొరకు (;;) {
    పూర్ణాంకం compare = tcCompare(remainder, srhs, parts);
    ఒకవేళ (compare >= 0) {
      tc_వ్యవకలనం(remainder, srhs, 0, parts);
      lhs[n] |= ముసుగు_2;
    }

    ఒకవేళ (జరుపు_లెక్క == 0)
      ఆపు;
    జరుపు_లెక్క--;
    tc_జరుపు_కుడి(srhs, parts, 1);
    ఒకవేళ ((ముసుగు_2 >>= 1) == 0) {
      ముసుగు_2 = (మాట_Type) 1 << (APINT_బిట్లు_చొప్పున_మాట - 1);
      n--;
    }
  }

  తిరిగి అబద్ధం;
}

/// Shift a bignum left Count bits in-place. Shifted in bits are zero. There are
/// no restrictions on Count.
శూన్య APInt::tc_జరుపు_ఎడమ(మాట_Type *Dst, గుర్తులేని పదాలు_2, గుర్తులేని లెక్క_2) {
  // Don't bother performing a no-op shift.
  ఒకవేళ (!లెక్క_2)
    తిరిగి;

  // WordShift is the inter-part shift; BitShift is the intra-part shift.
  గుర్తులేని మాట_జరుపు = std::min(లెక్క_2 / APINT_బిట్లు_చొప్పున_మాట, పదాలు_2);
  గుర్తులేని బిట్టు_జరుపు = లెక్క_2 % APINT_బిట్లు_చొప్పున_మాట;

  // Fastpath for moving by whole words.
  ఒకవేళ (బిట్టు_జరుపు == 0) {
    std::memmove(Dst + మాట_జరుపు, Dst, (పదాలు_2 - మాట_జరుపు) * APINT_మాట_కొలత);
  } లేకపోతే {
    అయితే (పదాలు_2-- > మాట_జరుపు) {
      Dst[పదాలు_2] = Dst[పదాలు_2 - మాట_జరుపు] << బిట్టు_జరుపు;
      ఒకవేళ (పదాలు_2 > మాట_జరుపు)
        Dst[పదాలు_2] |=
          Dst[పదాలు_2 - మాట_జరుపు - 1] >> (APINT_బిట్లు_చొప్పున_మాట - బిట్టు_జరుపు);
    }
  }

  // Fill in the remainder with 0s.
  std::memset(Dst, 0, మాట_జరుపు * APINT_మాట_కొలత);
}

/// Shift a bignum right Count bits in-place. Shifted in bits are zero. There
/// are no restrictions on Count.
శూన్య APInt::tc_జరుపు_కుడి(మాట_Type *Dst, గుర్తులేని పదాలు_2, గుర్తులేని లెక్క_2) {
  // Don't bother performing a no-op shift.
  ఒకవేళ (!లెక్క_2)
    తిరిగి;

  // WordShift is the inter-part shift; BitShift is the intra-part shift.
  గుర్తులేని మాట_జరుపు = std::min(లెక్క_2 / APINT_బిట్లు_చొప్పున_మాట, పదాలు_2);
  గుర్తులేని బిట్టు_జరుపు = లెక్క_2 % APINT_బిట్లు_చొప్పున_మాట;

  గుర్తులేని పదాలు_గమ్యం_Move = పదాలు_2 - మాట_జరుపు;
  // Fastpath for moving by whole words.
  ఒకవేళ (బిట్టు_జరుపు == 0) {
    std::memmove(Dst, Dst + మాట_జరుపు, పదాలు_గమ్యం_Move * APINT_మాట_కొలత);
  } లేకపోతే {
    కొరకు (గుర్తులేని i = 0; i != పదాలు_గమ్యం_Move; ++i) {
      Dst[i] = Dst[i + మాట_జరుపు] >> బిట్టు_జరుపు;
      ఒకవేళ (i + 1 != పదాలు_గమ్యం_Move)
        Dst[i] |= Dst[i + మాట_జరుపు + 1] << (APINT_బిట్లు_చొప్పున_మాట - బిట్టు_జరుపు);
    }
  }

  // Fill in the remainder with 0s.
  std::memset(Dst + పదాలు_గమ్యం_Move, 0, మాట_జరుపు * APINT_మాట_కొలత);
}

// Comparison (unsigned) of two bignums.
పూర్ణాంకం APInt::tcCompare(స్థిరరాశి మాట_Type *lhs, స్థిరరాశి మాట_Type *rhs,
                     గుర్తులేని parts) {
  అయితే (parts) {
    parts--;
    ఒకవేళ (lhs[parts] != rhs[parts])
      తిరిగి (lhs[parts] > rhs[parts]) ? 1 : -1;
  }

  తిరిగి 0;
}

APInt llvm::APIntOps::RoundingUDiv(స్థిరరాశి APInt &A, స్థిరరాశి APInt &B,
                                   APInt::Rounding RM) {
  // Currently udivrem always rounds down.
  ఎంపిక (RM) {
  స్థితి APInt::Rounding::కిందికి:
  స్థితి APInt::Rounding::TOWARD_ZERO:
    తిరిగి A.udiv(B);
  స్థితి APInt::Rounding::పైకి: {
    APInt Quo, Rem;
    APInt::udivrem(A, B, Quo, Rem);
    ఒకవేళ (Rem.isZero())
      తిరిగి Quo;
    తిరిగి Quo + 1;
  }
  }
  llvm_unreachable("Unknown APInt::Rounding enum");
}

APInt llvm::APIntOps::RoundingSDiv(స్థిరరాశి APInt &A, స్థిరరాశి APInt &B,
                                   APInt::Rounding RM) {
  ఎంపిక (RM) {
  స్థితి APInt::Rounding::కిందికి:
  స్థితి APInt::Rounding::పైకి: {
    APInt Quo, Rem;
    APInt::sdivrem(A, B, Quo, Rem);
    ఒకవేళ (Rem.isZero())
      తిరిగి Quo;
    // This algorithm deals with arbitrary rounding mode used by sdivrem.
    // We want to check whether the non-integer part of the mathematical value
    // is negative or not. If the non-integer part is negative, we need to round
    // down from Quo; otherwise, if it's positive or 0, we return Quo, as it's
    // already rounded down.
    ఒకవేళ (RM == APInt::Rounding::కిందికి) {
      ఒకవేళ (Rem.isNegative() != B.isNegative())
        తిరిగి Quo - 1;
      తిరిగి Quo;
    }
    ఒకవేళ (Rem.isNegative() != B.isNegative())
      తిరిగి Quo;
    తిరిగి Quo + 1;
  }
  // Currently sdiv rounds towards zero.
  స్థితి APInt::Rounding::TOWARD_ZERO:
    తిరిగి A.sdiv(B);
  }
  llvm_unreachable("Unknown APInt::Rounding enum");
}

std::optional<APInt>
llvm::APIntOps::SolveQuadraticEquationWrap(APInt A, APInt B, APInt C,
                                           గుర్తులేని Range_వెడల్పు) {
  గుర్తులేని Coeff_వెడల్పు = A.get_బిట్టు_వెడల్పు();
  ప్రతిజ్ఞా(Coeff_వెడల్పు == B.get_బిట్టు_వెడల్పు() && Coeff_వెడల్పు == C.get_బిట్టు_వెడల్పు());
  ప్రతిజ్ఞా(Range_వెడల్పు <= Coeff_వెడల్పు &&
         "Value range width should be less than coefficient width");
  ప్రతిజ్ఞా(Range_వెడల్పు > 1 && "Value range bit width should be > 1");

  LLVM_DEBUG(dbgs() << __func__ << ": solving " << A << "x^2 + " << B
                    << "x + " << C << ", rw:" << Range_వెడల్పు << '\n');

  // Identify 0 as a (non)solution immediately.
  ఒకవేళ (C.sextOrTrunc(Range_వెడల్పు).isZero()) {
    LLVM_DEBUG(dbgs() << __func__ << ": zero solution\n");
    తిరిగి APInt(Coeff_వెడల్పు, 0);
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
  Coeff_వెడల్పు *= 3;
  A = A.sext(Coeff_వెడల్పు);
  B = B.sext(Coeff_వెడల్పు);
  C = C.sext(Coeff_వెడల్పు);

  // Make A > 0 for simplicity. Negate cannot overflow at this point because
  // the bit width has increased.
  ఒకవేళ (A.isNegative()) {
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
  APInt R = APInt::get_One_బిట్టు_సమితి(Coeff_వెడల్పు, Range_వెడల్పు);
  APInt TwoA = 2 * A;
  APInt SqrB = B * B;
  తార్కిక Pick_దిగువ;

  స్వయంచాలక విడత_పైకి_2 = [] (స్థిరరాశి APInt &V, స్థిరరాశి APInt &A) -> APInt {
    ప్రతిజ్ఞా(A.is_Strictly_ధనాత్మక());
    APInt T = V.నిరపేక్ష().urem(A);
    ఒకవేళ (T.isZero())
      తిరిగి V;
    తిరిగి V.isNegative() ? V+T : V+(A-T);
  };

  // The vertex of the parabola is at -B/2A, but since A > 0, it's negative
  // iff B is positive.
  ఒకవేళ (B.isNonNegative()) {
    // If B >= 0, the vertex it at a negative location (or at 0), so in
    // order to have a non-negative solution we need to pick k that makes
    // C-kR negative. To satisfy all the requirements for the solution
    // that we are looking for, it needs to be closest to 0 of all k.
    C = C.srem(R);
    ఒకవేళ (C.is_Strictly_ధనాత్మక())
      C -= R;
    // Pick the greater solution.
    Pick_దిగువ = అబద్ధం;
  } లేకపోతే {
    // If B < 0, the vertex is at a positive location. For any solution
    // to exist, the discriminant must be non-negative. This means that
    // C-kR <= B^2/4A is a necessary condition for k, i.e. there is a
    // lower bound on values of k: kR >= C - B^2/4A.
    APInt LowkR = C - SqrB.udiv(2*TwoA); // udiv because all values > 0.
    // Round LowkR up (towards +inf) to the nearest kR.
    LowkR = విడత_పైకి_2(LowkR, R);

    // If there exists k meeting the condition above, and such that
    // C-kR > 0, there will be two positive real number solutions of
    // q(x) = kR. Out of all such values of k, pick the one that makes
    // C-kR closest to 0, (i.e. pick maximum k such that C-kR > 0).
    // In other words, find maximum k such that LowkR <= kR < C.
    ఒకవేళ (C.sgt(LowkR)) {
      // If LowkR < C, then such a k is guaranteed to exist because
      // LowkR itself is a multiple of R.
      C -= -విడత_పైకి_2(-C, R);      // C = C - RoundDown(C, R)
      // Pick the smaller solution.
      Pick_దిగువ = నిజం;
    } లేకపోతే {
      // If C-kR < 0 for all potential k's, it means that one solution
      // will be negative, while the other will be positive. The positive
      // solution will shift towards 0 if the parabola is moved up.
      // Pick the kR closest to the lower bound (i.e. make C-kR closest
      // to 0, or in other words, out of all parabolas that have solutions,
      // pick the one that is the farthest "up").
      // Since LowkR is itself a multiple of R, simply take C-LowkR.
      C -= LowkR;
      // Pick the greater solution.
      Pick_దిగువ = అబద్ధం;
    }
  }

  LLVM_DEBUG(dbgs() << __func__ << ": updated coefficients " << A << "x^2 + "
                    << B << "x + " << C << ", rw:" << Range_వెడల్పు << '\n');

  APInt D = SqrB - 4*A*C;
  ప్రతిజ్ఞా(D.isNonNegative() && "Negative discriminant");
  APInt SQ = D.sqrtFloor();

  APInt Q = SQ * SQ;
  తార్కిక InexactSQ = Q != D;

  APInt X;
  APInt Rem;

  // SQ is rounded down (i.e SQ * SQ <= D), so the roots may be inexact.
  // When using the quadratic formula directly, the calculated low root
  // may be greater than the exact one, since we would be subtracting SQ.
  // To make sure that the calculated root is not greater than the exact
  // one, subtract SQ+1 when calculating the low root (for inexact value
  // of SQ).
  ఒకవేళ (Pick_దిగువ)
    APInt::sdivrem(-B - (SQ+InexactSQ), TwoA, X, Rem);
  లేకపోతే
    APInt::sdivrem(-B + SQ, TwoA, X, Rem);

  // The updated coefficients should be such that the (exact) solution is
  // positive. Since APInt division rounds towards 0, the calculated one
  // can be 0, but cannot be negative.
  ప్రతిజ్ఞా(X.isNonNegative() && "Solution should be non-negative");

  ఒకవేళ (!InexactSQ && Rem.isZero()) {
    LLVM_DEBUG(dbgs() << __func__ << ": solution (root): " << X << '\n');
    తిరిగి X;
  }

  ప్రతిజ్ఞా((SQ*SQ).sle(D) && "SQ = |_sqrt(D)_|, so SQ*SQ <= D");
  // The exact value of the square root of D should be between SQ and SQ+1.
  // This implies that the solution should be between that corresponding to
  // SQ (i.e. X) and that corresponding to SQ+1.
  //
  // The calculated X cannot be greater than the exact (real) solution.
  // Actually it must be strictly less than the exact solution, while
  // X+1 will be greater than or equal to it.

  APInt VX = (A*X + B)*X + C;
  APInt VY = VX + TwoA*X + A + B;
  తార్కిక SignChange =
      VX.isNegative() != VY.isNegative() || VX.isZero() != VY.isZero();
  // If the sign did not change between X and X+1, X is not a valid solution.
  // This could happen when the actual (exact) roots don't have an integer
  // between them, so they would both be contained between X and X+1.
  ఒకవేళ (!SignChange) {
    LLVM_DEBUG(dbgs() << __func__ << ": no valid solution\n");
    తిరిగి std::nullopt;
  }

  X += 1;
  LLVM_DEBUG(dbgs() << __func__ << ": solution (wrap): " << X << '\n');
  తిరిగి X;
}

std::optional<గుర్తులేని>
llvm::APIntOps::Get_Most_Significant_Different_బిట్టు(స్థిరరాశి APInt &A, స్థిరరాశి APInt &B) {
  ప్రతిజ్ఞా(A.get_బిట్టు_వెడల్పు() == B.get_బిట్టు_వెడల్పు() && "Must have the same bitwidth");
  ఒకవేళ (A == B)
    తిరిగి std::nullopt;
  తిరిగి A.get_బిట్టు_వెడల్పు() - ((A ^ B).countl_zero() + 1);
}

APInt llvm::APIntOps::Scale_బిట్టు_ముసుగు(స్థిరరాశి APInt &A, గుర్తులేని New_బిట్టు_వెడల్పు,
                                   తార్కిక సరిపోలిక_All_బిట్లు) {
  గుర్తులేని Old_బిట్టు_వెడల్పు = A.get_బిట్టు_వెడల్పు();
  ప్రతిజ్ఞా((((Old_బిట్టు_వెడల్పు % New_బిట్టు_వెడల్పు) == 0) ||
          ((New_బిట్టు_వెడల్పు % Old_బిట్టు_వెడల్పు) == 0)) &&
         "One size should be a multiple of the other one. "
         "Can't do fractional scaling.");

  // Check for matching bitwidths.
  ఒకవేళ (Old_బిట్టు_వెడల్పు == New_బిట్టు_వెడల్పు)
    తిరిగి A;

  APInt NewA = APInt::getZero(New_బిట్టు_వెడల్పు);

  // Check for null input.
  ఒకవేళ (A.isZero())
    తిరిగి NewA;

  ఒకవేళ (New_బిట్టు_వెడల్పు > Old_బిట్టు_వెడల్పు) {
    // Repeat bits.
    గుర్తులేని Scale = New_బిట్టు_వెడల్పు / Old_బిట్టు_వెడల్పు;
    కొరకు (గుర్తులేని i = 0; i != Old_బిట్టు_వెడల్పు; ++i)
      ఒకవేళ (A[i])
        NewA.సమితి_బిట్లు(i * Scale, (i + 1) * Scale);
  } లేకపోతే {
    గుర్తులేని Scale = Old_బిట్టు_వెడల్పు / New_బిట్టు_వెడల్పు;
    కొరకు (గుర్తులేని i = 0; i != New_బిట్టు_వెడల్పు; ++i) {
      ఒకవేళ (సరిపోలిక_All_బిట్లు) {
        ఒకవేళ (A.వెలికితీయి_బిట్లు(Scale, i * Scale).isAllOnes())
          NewA.సమితి_బిట్టు(i);
      } లేకపోతే {
        ఒకవేళ (!A.వెలికితీయి_బిట్లు(Scale, i * Scale).isZero())
          NewA.సమితి_బిట్టు(i);
      }
    }
  }

  తిరిగి NewA;
}

/// StoreIntToMemory - Fills the StoreBytes bytes of memory starting from Dst
/// with the integer held in IntVal.
శూన్య llvm::Store_Int_గమ్యం_జ్ఞాపకం(స్థిరరాశి APInt &IntVal, uint8_t *Dst,
                            గుర్తులేని Store_బైట్లు) {
  ప్రతిజ్ఞా((IntVal.get_బిట్టు_వెడల్పు()+7)/8 >= Store_బైట్లు && "Integer too small!");
  స్థిరరాశి uint8_t *Src = (స్థిరరాశి uint8_t *)IntVal.get_Raw_దత్తాంశం();

  ఒకవేళ (sys::IsLittleEndianHost) {
    // Little-endian host - the source is ordered from LSB to MSB.  Order the
    // destination from LSB to MSB: Do a straight copy.
    జ్ఞాపకం_నకలు(Dst, Src, Store_బైట్లు);
  } లేకపోతే {
    // Big-endian host - the source is an array of 64 bit words ordered from
    // LSW to MSW.  Each word is ordered from MSB to LSB.  Order the destination
    // from MSB to LSB: Reverse the word order, but not the bytes in a word.
    అయితే (Store_బైట్లు > పరిమాణం(uint64_t)) {
      Store_బైట్లు -= పరిమాణం(uint64_t);
      // May not be aligned so use memcpy.
      జ్ఞాపకం_నకలు(Dst + Store_బైట్లు, Src, పరిమాణం(uint64_t));
      Src += పరిమాణం(uint64_t);
    }

    జ్ఞాపకం_నకలు(Dst, Src + పరిమాణం(uint64_t) - Store_బైట్లు, Store_బైట్లు);
  }
}

/// LoadIntFromMemory - Loads the integer stored in the LoadBytes bytes starting
/// from Src into IntVal, which is assumed to be wide enough and to hold zero.
శూన్య llvm::ఎక్కించు_Int_From_జ్ఞాపకం(APInt &IntVal, స్థిరరాశి uint8_t *Src,
                             గుర్తులేని ఎక్కించు_బైట్లు) {
  ప్రతిజ్ఞా((IntVal.get_బిట్టు_వెడల్పు()+7)/8 >= ఎక్కించు_బైట్లు && "Integer too small!");
  uint8_t *Dst = reinterpret_cast<uint8_t *>(
                   const_cast<uint64_t *>(IntVal.get_Raw_దత్తాంశం()));

  ఒకవేళ (sys::IsLittleEndianHost)
    // Little-endian host - the destination must be ordered from LSB to MSB.
    // The source is ordered from LSB to MSB: Do a straight copy.
    జ్ఞాపకం_నకలు(Dst, Src, ఎక్కించు_బైట్లు);
  లేకపోతే {
    // Big-endian - the destination is an array of 64 bit words ordered from
    // LSW to MSW.  Each word must be ordered from MSB to LSB.  The source is
    // ordered from MSB to LSB: Reverse the word order, but not the bytes in
    // a word.
    అయితే (ఎక్కించు_బైట్లు > పరిమాణం(uint64_t)) {
      ఎక్కించు_బైట్లు -= పరిమాణం(uint64_t);
      // May not be aligned so use memcpy.
      జ్ఞాపకం_నకలు(Dst, Src + ఎక్కించు_బైట్లు, పరిమాణం(uint64_t));
      Dst += పరిమాణం(uint64_t);
    }

    జ్ఞాపకం_నకలు(Dst + పరిమాణం(uint64_t) - ఎక్కించు_బైట్లు, Src, ఎక్కించు_బైట్లు);
  }
}

APInt APIntOps::avgFloorS(స్థిరరాశి APInt &C1, స్థిరరాశి APInt &C2) {
  // Return floor((C1 + C2) / 2)
  తిరిగి (C1 & C2) + (C1 ^ C2).ashr(1);
}

APInt APIntOps::avgFloorU(స్థిరరాశి APInt &C1, స్థిరరాశి APInt &C2) {
  // Return floor((C1 + C2) / 2)
  తిరిగి (C1 & C2) + (C1 ^ C2).lshr(1);
}

APInt APIntOps::avgCeilS(స్థిరరాశి APInt &C1, స్థిరరాశి APInt &C2) {
  // Return ceil((C1 + C2) / 2)
  తిరిగి (C1 | C2) - (C1 ^ C2).ashr(1);
}

APInt APIntOps::avgCeilU(స్థిరరాశి APInt &C1, స్థిరరాశి APInt &C2) {
  // Return ceil((C1 + C2) / 2)
  తిరిగి (C1 | C2) - (C1 ^ C2).lshr(1);
}

APInt APIntOps::avgSubFloorS(స్థిరరాశి APInt &C1, స్థిరరాశి APInt &C2) {
  // Return floor((C1 - C2) / 2), using C1 - C2 == (C1 ^ C2) - 2 * (~C1 & C2).
  తిరిగి (C1 ^ C2).ashr(1) - (~C1 & C2);
}

APInt APIntOps::avgSubFloorU(స్థిరరాశి APInt &C1, స్థిరరాశి APInt &C2) {
  // Return floor((C1 - C2) / 2), using C1 - C2 == (C1 ^ C2) - 2 * (~C1 & C2).
  తిరిగి (C1 ^ C2).lshr(1) - (~C1 & C2);
}

APInt APIntOps::mulhs(స్థిరరాశి APInt &C1, స్థిరరాశి APInt &C2) {
  ప్రతిజ్ఞా(C1.get_బిట్టు_వెడల్పు() == C2.get_బిట్టు_వెడల్పు() && "Unequal bitwidths");
  గుర్తులేని నిండిన_వెడల్పు = C1.get_బిట్టు_వెడల్పు() * 2;
  APInt C1Ext = C1.sext(నిండిన_వెడల్పు);
  APInt C2Ext = C2.sext(నిండిన_వెడల్పు);
  తిరిగి (C1Ext * C2Ext).వెలికితీయి_బిట్లు(C1.get_బిట్టు_వెడల్పు(), C1.get_బిట్టు_వెడల్పు());
}

APInt APIntOps::mulhu(స్థిరరాశి APInt &C1, స్థిరరాశి APInt &C2) {
  ప్రతిజ్ఞా(C1.get_బిట్టు_వెడల్పు() == C2.get_బిట్టు_వెడల్పు() && "Unequal bitwidths");
  గుర్తులేని నిండిన_వెడల్పు = C1.get_బిట్టు_వెడల్పు() * 2;
  APInt C1Ext = C1.zext(నిండిన_వెడల్పు);
  APInt C2Ext = C2.zext(నిండిన_వెడల్పు);
  తిరిగి (C1Ext * C2Ext).వెలికితీయి_బిట్లు(C1.get_బిట్టు_వెడల్పు(), C1.get_బిట్టు_వెడల్పు());
}

APInt APIntOps::mulsExtended(స్థిరరాశి APInt &C1, స్థిరరాశి APInt &C2) {
  ప్రతిజ్ఞా(C1.get_బిట్టు_వెడల్పు() == C2.get_బిట్టు_వెడల్పు() && "Unequal bitwidths");
  గుర్తులేని నిండిన_వెడల్పు = C1.get_బిట్టు_వెడల్పు() * 2;
  APInt C1Ext = C1.sext(నిండిన_వెడల్పు);
  APInt C2Ext = C2.sext(నిండిన_వెడల్పు);
  తిరిగి C1Ext * C2Ext;
}

APInt APIntOps::muluExtended(స్థిరరాశి APInt &C1, స్థిరరాశి APInt &C2) {
  ప్రతిజ్ఞా(C1.get_బిట్టు_వెడల్పు() == C2.get_బిట్టు_వెడల్పు() && "Unequal bitwidths");
  గుర్తులేని నిండిన_వెడల్పు = C1.get_బిట్టు_వెడల్పు() * 2;
  APInt C1Ext = C1.zext(నిండిన_వెడల్పు);
  APInt C2Ext = C2.zext(నిండిన_వెడల్పు);
  తిరిగి C1Ext * C2Ext;
}

APInt APIntOps::ఘాతం(స్థిరరాశి APInt &X, int64_t N) {
  ప్రతిజ్ఞా(N >= 0 && "negative exponents not supported.");
  APInt Acc = APInt(X.get_బిట్టు_వెడల్పు(), 1);
  ఒకవేళ (N == 0)
    తిరిగి Acc;
  APInt పునాది = X;
  int64_t మిగిలిన_Exponent = N;
  అయితే (మిగిలిన_Exponent > 0) {
    అయితే (మిగిలిన_Exponent % 2 == 0) {
      పునాది *= పునాది;
      మిగిలిన_Exponent /= 2;
    }
    --మిగిలిన_Exponent;
    Acc *= పునాది;
  }
  తిరిగి Acc;
}

APInt llvm::APIntOps::fshl(స్థిరరాశి APInt &Hi, స్థిరరాశి APInt &Lo,
                           స్థిరరాశి APInt &జరుపు_2) {
  ప్రతిజ్ఞా(Hi.get_బిట్టు_వెడల్పు() == Lo.get_బిట్టు_వెడల్పు());
  గుర్తులేని జరుపు_Amt_2 = rotateModulo(Hi.get_బిట్టు_వెడల్పు(), జరుపు_2);
  ఒకవేళ (జరుపు_Amt_2 == 0)
    తిరిగి Hi;
  తిరిగి Hi.shl(జరుపు_Amt_2) | Lo.lshr(Hi.get_బిట్టు_వెడల్పు() - జరుపు_Amt_2);
}

APInt llvm::APIntOps::fshr(స్థిరరాశి APInt &Hi, స్థిరరాశి APInt &Lo,
                           స్థిరరాశి APInt &జరుపు_2) {
  ప్రతిజ్ఞా(Hi.get_బిట్టు_వెడల్పు() == Lo.get_బిట్టు_వెడల్పు());
  గుర్తులేని జరుపు_Amt_2 = rotateModulo(Hi.get_బిట్టు_వెడల్పు(), జరుపు_2);
  ఒకవేళ (జరుపు_Amt_2 == 0)
    తిరిగి Lo;
  తిరిగి Hi.shl(Hi.get_బిట్టు_వెడల్పు() - జరుపు_Amt_2) | Lo.lshr(జరుపు_Amt_2);
}

APInt llvm::APIntOps::clmul(స్థిరరాశి APInt &LHS, స్థిరరాశి APInt &RHS) {
  గుర్తులేని BW = LHS.get_బిట్టు_వెడల్పు();
  ప్రతిజ్ఞా(BW == RHS.get_బిట్టు_వెడల్పు() && "Operand mismatch");
  APInt ఫలితం(BW, 0);
  కొరకు (గుర్తులేని I : seq(std::min(RHS.get_చురుకైన_బిట్లు(), BW - LHS.countr_zero())))
    ఒకవేళ (RHS[I])
      ఫలితం ^= LHS << I;
  తిరిగి ఫలితం;
}

APInt llvm::APIntOps::clmulr(స్థిరరాశి APInt &LHS, స్థిరరాశి APInt &RHS) {
  ప్రతిజ్ఞా(LHS.get_బిట్టు_వెడల్పు() == RHS.get_బిట్టు_వెడల్పు());
  తిరిగి clmul(LHS.తిరగేయి_బిట్లు(), RHS.తిరగేయి_బిట్లు()).తిరగేయి_బిట్లు();
}

APInt llvm::APIntOps::clmulh(స్థిరరాశి APInt &LHS, స్థిరరాశి APInt &RHS) {
  ప్రతిజ్ఞా(LHS.get_బిట్టు_వెడల్పు() == RHS.get_బిట్టు_వెడల్పు());
  తిరిగి clmulr(LHS, RHS).lshr(1);
}

APInt llvm::APIntOps::pext(స్థిరరాశి APInt &Val, స్థిరరాశి APInt &ముసుగు) {
  గుర్తులేని BW = Val.get_బిట్టు_వెడల్పు();
  ప్రతిజ్ఞా(BW == ముసుగు.get_బిట్టు_వెడల్పు() && "Operand mismatch");
  APInt ఫలితం = APInt::getZero(BW);
  కొరకు (గుర్తులేని I = 0, P = 0; I != BW; ++I)
    ఒకవేళ (ముసుగు[I])
      ఫలితం.సమితి_బిట్టు_Val(P++, Val[I]);
  తిరిగి ఫలితం;
}

APInt llvm::APIntOps::pdep(స్థిరరాశి APInt &Val, స్థిరరాశి APInt &ముసుగు) {
  గుర్తులేని BW = Val.get_బిట్టు_వెడల్పు();
  ప్రతిజ్ఞా(BW == ముసుగు.get_బిట్టు_వెడల్పు() && "Operand mismatch");
  APInt ఫలితం = APInt::getZero(BW);
  కొరకు (గుర్తులేని I = 0, P = 0; I != BW; ++I)
    ఒకవేళ (ముసుగు[I])
      ఫలితం.సమితి_బిట్టు_Val(I, Val[P++]);
  తిరిగి ఫలితం;
}
