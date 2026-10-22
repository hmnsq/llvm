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

#अन्तर्भाव "llvm/ADT/APInt.h"
#अन्तर्भाव "llvm/ADT/ArrayRef.h"
#अन्तर्भाव "llvm/ADT/FoldingSet.h"
#अन्तर्भाव "llvm/ADT/Hashing.h"
#अन्तर्भाव "llvm/ADT/Sequence.h"
#अन्तर्भाव "llvm/ADT/SmallString.h"
#अन्तर्भाव "llvm/ADT/StringRef.h"
#अन्तर्भाव "llvm/ADT/bit.h"
#अन्तर्भाव "llvm/Support/Alignment.h"
#अन्तर्भाव "llvm/Support/Debug.h"
#अन्तर्भाव "llvm/Support/ErrorHandling.h"
#अन्तर्भाव "llvm/Support/MathExtras.h"
#अन्तर्भाव "llvm/Support/SwapByteOrder.h"
#अन्तर्भाव "llvm/Support/raw_ostream.h"
#अन्तर्भाव <cmath>
#अन्तर्भाव <optional>

उपयुज्य नामाकाश llvm;

#परिभाषय DEBUG_TYPE "apint"

/// A utility function for allocating memory, checking for allocation failures,
/// and ensuring the contents are zeroed.
अन्तःस्थ स्थिर uint64_t* get_Cleared_स्मृतिः(अचिह्नित अङ्कः_शब्दाः) {
  प्रत्यागम नूतन uint64_t[अङ्कः_शब्दाः]();
}

/// A utility function for allocating memory and checking for allocation
/// failure.  The content is not zeroed.
अन्तःस्थ स्थिर uint64_t* get_स्मृतिः(अचिह्नित अङ्कः_शब्दाः) {
  प्रत्यागम नूतन uint64_t[अङ्कः_शब्दाः];
}

/// A utility function that converts a character to a digit.
अन्तःस्थ स्थिर अचिह्नित getDigit(वर्ण cdigit, uint8_t radix) {
  अचिह्नित r;

  यदि (radix == 16 || radix == 36) {
    r = cdigit - '0';
    यदि (r <= 9)
      प्रत्यागम r;

    r = cdigit - 'A';
    यदि (r <= radix - 11U)
      प्रत्यागम r + 10;

    r = cdigit - 'a';
    यदि (r <= radix - 11U)
      प्रत्यागम r + 10;

    radix = 10;
  }

  r = cdigit - '0';
  यदि (r < radix)
    प्रत्यागम r;

  प्रत्यागम UINT_MAX;
}


शून्य APInt::initSlowCase(uint64_t val, तार्किक isSigned) {
  यदि (isSigned && int64_t(val) < 0) {
    U.pVal = get_स्मृतिः(get_अङ्कः_शब्दाः());
    U.pVal[0] = val;
    स्मृतिं_पूरय(&U.pVal[1], 0xFF, APINT_WORD_आकार * (get_अङ्कः_शब्दाः() - 1));
    clear_अप्रयुक्तम्_द्वयङ्काः();
  } अन्यथा {
    U.pVal = get_Cleared_स्मृतिः(get_अङ्कः_शब्दाः());
    U.pVal[0] = val;
  }
}

शून्य APInt::initSlowCase(ध्रुव APInt& that) {
  U.pVal = get_स्मृतिः(get_अङ्कः_शब्दाः());
  स्मृतिं_प्रतिलिख(U.pVal, that.U.pVal, get_अङ्कः_शब्दाः() * APINT_WORD_आकार);
}

शून्य APInt::initFromArray(ArrayRef<uint64_t> bigVal) {
  प्रतिज्ञा(bigVal.दत्त() && "Null pointer detected!");
  यदि (LLVM_LIKELY(isSingleWord()))
    U.VAL = bigVal[0];
  अन्यथा {
    // Get memory, cleared to 0
    U.pVal = get_Cleared_स्मृतिः(get_अङ्कः_शब्दाः());
    // Calculate the number of words to copy
    अचिह्नित शब्दाः = std::min<अचिह्नित>(bigVal.आकार_2(), get_अङ्कः_शब्दाः());
    // Copy the words from bigVal to pVal
    स्मृतिं_प्रतिलिख(U.pVal, bigVal.दत्त(), शब्दाः * APINT_WORD_आकार);
  }
  // Make sure unused high bits are cleared
  clear_अप्रयुक्तम्_द्वयङ्काः();
}

APInt::APInt(अचिह्नित अङ्कः_द्वयङ्काः, ArrayRef<uint64_t> bigVal) : Bit_विस्तारः(अङ्कः_द्वयङ्काः) {
  initFromArray(bigVal);
}

APInt::APInt(अचिह्नित numbits, StringRef Str, uint8_t radix)
    : Bit_विस्तारः(numbits) {
  fromString(numbits, Str, radix);
}

शून्य APInt::reallocate(अचिह्नित New_Bit_विस्तारः) {
  // If the number of words is the same we can just change the width and stop.
  यदि (get_अङ्कः_शब्दाः() == get_अङ्कः_शब्दाः(New_Bit_विस्तारः)) {
    Bit_विस्तारः = New_Bit_विस्तारः;
    प्रत्यागम;
  }

  // If we have an allocation, delete it.
  यदि (LLVM_UNLIKELY(!isSingleWord()))
    विलोपय [] U.pVal;

  // Update BitWidth.
  Bit_विस्तारः = New_Bit_विस्तारः;

  // If we are supposed to have an allocation, create it.
  यदि (LLVM_UNLIKELY(!isSingleWord()))
    U.pVal = get_स्मृतिः(get_अङ्कः_शब्दाः());
}

शून्य APInt::नियोजय_Slow_Case(ध्रुव APInt &RHS) {
  // Don't do anything for X = X
  यदि (एतत् == &RHS)
    प्रत्यागम;

  // Adjust the bit width and handle allocations as necessary.
  reallocate(RHS.get_Bit_विस्तारः());

  // Copy the data.
  यदि (LLVM_LIKELY(isSingleWord()))
    U.VAL = RHS.U.VAL;
  अन्यथा
    स्मृतिं_प्रतिलिख(U.pVal, RHS.U.pVal, get_अङ्कः_शब्दाः() * APINT_WORD_आकार);
}

/// This method 'profiles' an APInt for use with FoldingSet.
शून्य APInt::Profile(Folding_समुच्चयः_ग्रन्थिः_ID& ID) ध्रुव {
  ID.सङ्कलय_Integer(Bit_विस्तारः);

  यदि (LLVM_LIKELY(isSingleWord())) {
    ID.सङ्कलय_Integer(U.VAL);
    प्रत्यागम;
  }

  अचिह्नित अङ्कः_शब्दाः_2 = get_अङ्कः_शब्दाः();
  प्रति (अचिह्नित i = 0; i < अङ्कः_शब्दाः_2; ++i)
    ID.सङ्कलय_Integer(U.pVal[i]);
}

तार्किक APInt::isAligned(Align A) ध्रुव {
  यदि (isZero())
    प्रत्यागम सत्य;
  ध्रुव अचिह्नित TrailingZeroes = countr_zero();
  ध्रुव अचिह्नित MinimumTrailingZeroes = Log2(A);
  प्रत्यागम TrailingZeroes >= MinimumTrailingZeroes;
}

/// Prefix increment operator. Increments the APInt by one.
APInt& APInt::संक्रियकः++() {
  यदि (LLVM_LIKELY(isSingleWord()))
    ++U.VAL;
  अन्यथा
    tcIncrement(U.pVal, get_अङ्कः_शब्दाः());
  प्रत्यागम clear_अप्रयुक्तम्_द्वयङ्काः();
}

/// Prefix decrement operator. Decrements the APInt by one.
APInt& APInt::संक्रियकः--() {
  यदि (LLVM_LIKELY(isSingleWord()))
    --U.VAL;
  अन्यथा
    tcDecrement(U.pVal, get_अङ्कः_शब्दाः());
  प्रत्यागम clear_अप्रयुक्तम्_द्वयङ्काः();
}

/// Adds the RHS APInt to this APInt.
/// @returns this, after addition of RHS.
/// Addition assignment operator.
APInt& APInt::संक्रियकः+=(ध्रुव APInt& RHS) {
  प्रतिज्ञा(Bit_विस्तारः == RHS.Bit_विस्तारः && "Bit widths must be the same");
  यदि (LLVM_LIKELY(isSingleWord()))
    U.VAL += RHS.U.VAL;
  अन्यथा
    tc_सङ्कलय(U.pVal, RHS.U.pVal, 0, get_अङ्कः_शब्दाः());
  प्रत्यागम clear_अप्रयुक्तम्_द्वयङ्काः();
}

APInt& APInt::संक्रियकः+=(uint64_t RHS) {
  यदि (LLVM_LIKELY(isSingleWord()))
    U.VAL += RHS;
  अन्यथा
    tc_सङ्कलय_Part(U.pVal, RHS, get_अङ्कः_शब्दाः());
  प्रत्यागम clear_अप्रयुक्तम्_द्वयङ्काः();
}

/// Subtracts the RHS APInt from this APInt
/// @returns this, after subtraction
/// Subtraction assignment operator.
APInt& APInt::संक्रियकः-=(ध्रुव APInt& RHS) {
  प्रतिज्ञा(Bit_विस्तारः == RHS.Bit_विस्तारः && "Bit widths must be the same");
  यदि (LLVM_LIKELY(isSingleWord()))
    U.VAL -= RHS.U.VAL;
  अन्यथा
    tc_व्यवकलय(U.pVal, RHS.U.pVal, 0, get_अङ्कः_शब्दाः());
  प्रत्यागम clear_अप्रयुक्तम्_द्वयङ्काः();
}

APInt& APInt::संक्रियकः-=(uint64_t RHS) {
  यदि (LLVM_LIKELY(isSingleWord()))
    U.VAL -= RHS;
  अन्यथा
    tc_व्यवकलय_Part(U.pVal, RHS, get_अङ्कः_शब्दाः());
  प्रत्यागम clear_अप्रयुक्तम्_द्वयङ्काः();
}

APInt APInt::संक्रियकः*(ध्रुव APInt& RHS) ध्रुव {
  प्रतिज्ञा(Bit_विस्तारः == RHS.Bit_विस्तारः && "Bit widths must be the same");
  यदि (LLVM_LIKELY(isSingleWord()))
    प्रत्यागम APInt(Bit_विस्तारः, U.VAL * RHS.U.VAL, /*isSigned=*/असत्य,
                 /*implicitTrunc=*/सत्य);

  APInt परिणाम(get_स्मृतिः(get_अङ्कः_शब्दाः()), get_Bit_विस्तारः());
  tcMultiply(परिणाम.U.pVal, U.pVal, RHS.U.pVal, get_अङ्कः_शब्दाः());
  परिणाम.clear_अप्रयुक्तम्_द्वयङ्काः();
  प्रत्यागम परिणाम;
}

शून्य APInt::and_नियोजय_Slow_Case(ध्रुव APInt &RHS) {
  WordType *dst = U.pVal, *rhs = RHS.U.pVal;
  प्रति (size_t i = 0, e = get_अङ्कः_शब्दाः(); i != e; ++i)
    dst[i] &= rhs[i];
}

शून्य APInt::or_नियोजय_Slow_Case(ध्रुव APInt &RHS) {
  WordType *dst = U.pVal, *rhs = RHS.U.pVal;
  प्रति (size_t i = 0, e = get_अङ्कः_शब्दाः(); i != e; ++i)
    dst[i] |= rhs[i];
}

शून्य APInt::xor_नियोजय_Slow_Case(ध्रुव APInt &RHS) {
  WordType *dst = U.pVal, *rhs = RHS.U.pVal;
  प्रति (size_t i = 0, e = get_अङ्कः_शब्दाः(); i != e; ++i)
    dst[i] ^= rhs[i];
}

APInt &APInt::संक्रियकः*=(ध्रुव APInt &RHS) {
  *एतत् = *एतत् * RHS;
  प्रत्यागम *एतत्;
}

APInt& APInt::संक्रियकः*=(uint64_t RHS) {
  यदि (LLVM_LIKELY(isSingleWord())) {
    U.VAL *= RHS;
  } अन्यथा {
    अचिह्नित अङ्कः_शब्दाः_2 = get_अङ्कः_शब्दाः();
    tcMultiplyPart(U.pVal, U.pVal, RHS, 0, अङ्कः_शब्दाः_2, अङ्कः_शब्दाः_2, असत्य);
  }
  प्रत्यागम clear_अप्रयुक्तम्_द्वयङ्काः();
}

तार्किक APInt::equalSlowCase(ध्रुव APInt &RHS) ध्रुव {
  प्रत्यागम std::equal(U.pVal, U.pVal + get_अङ्कः_शब्दाः(), RHS.U.pVal);
}

पूर्णाङ्क APInt::compare(ध्रुव APInt& RHS) ध्रुव {
  प्रतिज्ञा(Bit_विस्तारः == RHS.Bit_विस्तारः && "Bit widths must be same for comparison");
  यदि (LLVM_LIKELY(isSingleWord()))
    प्रत्यागम U.VAL < RHS.U.VAL ? -1 : U.VAL > RHS.U.VAL;

  प्रत्यागम tcCompare(U.pVal, RHS.U.pVal, get_अङ्कः_शब्दाः());
}

पूर्णाङ्क APInt::compareSigned(ध्रुव APInt& RHS) ध्रुव {
  प्रतिज्ञा(Bit_विस्तारः == RHS.Bit_विस्तारः && "Bit widths must be same for comparison");
  यदि (LLVM_LIKELY(isSingleWord())) {
    int64_t lhsSext = SignExtend64(U.VAL, Bit_विस्तारः);
    int64_t rhsSext = SignExtend64(RHS.U.VAL, Bit_विस्तारः);
    प्रत्यागम lhsSext < rhsSext ? -1 : lhsSext > rhsSext;
  }

  तार्किक lhsNeg = isNegative();
  तार्किक rhsNeg = RHS.isNegative();

  // If the sign bits don't match, then (LHS < RHS) if LHS is negative
  यदि (lhsNeg != rhsNeg)
    प्रत्यागम lhsNeg ? -1 : 1;

  // Otherwise we can just use an unsigned comparison, because even negative
  // numbers compare correctly this way if both have the same signed-ness.
  प्रत्यागम tcCompare(U.pVal, RHS.U.pVal, get_अङ्कः_शब्दाः());
}

शून्य APInt::समुच्चयः_द्वयङ्काः_Slow_Case(अचिह्नित loBit, अचिह्नित hiBit) {
  अचिह्नित loWord = whichWord(loBit);
  अचिह्नित hiWord = whichWord(hiBit);

  // Create an initial mask for the low word with zeros below loBit.
  uint64_t lo_आवरणम् = WORDTYPE_MAX << whichBit(loBit);

  // If hiBit is not aligned, we need a high mask.
  अचिह्नित hi_सरणम्_Amt = whichBit(hiBit);
  यदि (hi_सरणम्_Amt != 0) {
    // Create a high mask with zeros above hiBit.
    uint64_t hi_आवरणम् = WORDTYPE_MAX >> (APINT_द्वयङ्काः_प्रत्येकम्_WORD - hi_सरणम्_Amt);
    // If loWord and hiWord are equal, then we combine the masks. Otherwise,
    // set the bits in hiWord.
    यदि (hiWord == loWord)
      lo_आवरणम् &= hi_आवरणम्;
    अन्यथा
      U.pVal[hiWord] |= hi_आवरणम्;
  }
  // Apply the mask to the low word.
  U.pVal[loWord] |= lo_आवरणम्;

  // Fill any words between loWord and hiWord with all ones.
  प्रति (अचिह्नित word = loWord + 1; word < hiWord; ++word)
    U.pVal[word] = WORDTYPE_MAX;
}

शून्य APInt::clear_द्वयङ्काः_Slow_Case(अचिह्नित LoBit, अचिह्नित HiBit) {
  अचिह्नित LoWord = whichWord(LoBit);
  अचिह्नित HiWord = whichWord(HiBit);

  // Create an initial mask for the low word with ones below loBit.
  uint64_t Lo_आवरणम् = ~(WORDTYPE_MAX << whichBit(LoBit));

  // If HiBit is not aligned, we need a high mask.
  अचिह्नित Hi_सरणम्_Amt = whichBit(HiBit);
  यदि (Hi_सरणम्_Amt != 0) {
    // Create a high mask with ones above HiBit.
    uint64_t Hi_आवरणम् = ~(WORDTYPE_MAX >> (APINT_द्वयङ्काः_प्रत्येकम्_WORD - Hi_सरणम्_Amt));
    // If LoWord and HiWord are equal, then we combine the masks. Otherwise,
    // clear the bits in HiWord.
    यदि (HiWord == LoWord)
      Lo_आवरणम् |= Hi_आवरणम्;
    अन्यथा
      U.pVal[HiWord] &= Hi_आवरणम्;
  }
  // Apply the mask to the low word.
  U.pVal[LoWord] &= Lo_आवरणम्;

  // Fill any words between LoWord and HiWord with all zeros.
  प्रति (अचिह्नित Word = LoWord + 1; Word < HiWord; ++Word)
    U.pVal[Word] = 0;
}

// Complement a bignum in-place.
स्थिर शून्य tcComplement(APInt::WordType *dst, अचिह्नित parts) {
  प्रति (अचिह्नित i = 0; i < parts; i++)
    dst[i] = ~dst[i];
}

/// Toggle every bit to its opposite value.
शून्य APInt::flip_All_द्वयङ्काः_Slow_Case() {
  tcComplement(U.pVal, get_अङ्कः_शब्दाः());
  clear_अप्रयुक्तम्_द्वयङ्काः();
}

/// Concatenate the bits from "NewLSB" onto the bottom of *this.  This is
/// equivalent to:
///   (this->zext(NewWidth) << NewLSB.getBitWidth()) | NewLSB.zext(NewWidth)
/// In the slow case, we know the result is large.
APInt APInt::concatSlowCase(ध्रुव APInt &NewLSB) ध्रुव {
  अचिह्नित New_विस्तारः = get_Bit_विस्तारः() + NewLSB.get_Bit_विस्तारः();
  APInt परिणाम = NewLSB.zext(New_विस्तारः);
  परिणाम.निवेशय_द्वयङ्काः(*एतत्, NewLSB.get_Bit_विस्तारः());
  प्रत्यागम परिणाम;
}

/// Toggle a given bit to its opposite value whose position is given
/// as "bitPosition".
/// Toggles a given bit to its opposite value.
शून्य APInt::flipBit(अचिह्नित bitPosition) {
  प्रतिज्ञा(bitPosition < Bit_विस्तारः && "Out of the bit-width range!");
  समुच्चयः_Bit_Val(bitPosition, !(*एतत्)[bitPosition]);
}

शून्य APInt::निवेशय_द्वयङ्काः(ध्रुव APInt &sub_द्वयङ्काः, अचिह्नित bitPosition) {
  अचिह्नित sub_Bit_विस्तारः = sub_द्वयङ्काः.get_Bit_विस्तारः();
  प्रतिज्ञा((sub_Bit_विस्तारः + bitPosition) <= Bit_विस्तारः && "Illegal bit insertion");

  // inserting no bits is a noop.
  यदि (sub_Bit_विस्तारः == 0)
    प्रत्यागम;

  // Insertion is a direct copy.
  यदि (sub_Bit_विस्तारः == Bit_विस्तारः) {
    *एतत् = sub_द्वयङ्काः;
    प्रत्यागम;
  }

  // Single word result can be done as a direct bitmask.
  यदि (LLVM_LIKELY(isSingleWord())) {
    uint64_t आवरणम्_2 = WORDTYPE_MAX >> (APINT_द्वयङ्काः_प्रत्येकम्_WORD - sub_Bit_विस्तारः);
    U.VAL &= ~(आवरणम्_2 << bitPosition);
    U.VAL |= (sub_द्वयङ्काः.U.VAL << bitPosition);
    प्रत्यागम;
  }

  अचिह्नित loBit = whichBit(bitPosition);
  अचिह्नित loWord = whichWord(bitPosition);
  अचिह्नित hi1Word = whichWord(bitPosition + sub_Bit_विस्तारः - 1);

  // Insertion within a single word can be done as a direct bitmask.
  यदि (loWord == hi1Word) {
    uint64_t आवरणम्_2 = WORDTYPE_MAX >> (APINT_द्वयङ्काः_प्रत्येकम्_WORD - sub_Bit_विस्तारः);
    U.pVal[loWord] &= ~(आवरणम्_2 << loBit);
    U.pVal[loWord] |= (sub_द्वयङ्काः.U.VAL << loBit);
    प्रत्यागम;
  }

  // Insert on word boundaries.
  यदि (loBit == 0) {
    // Direct copy whole words.
    अचिह्नित अङ्कः_सम्पूर्णम्_Sub_शब्दाः = sub_Bit_विस्तारः / APINT_द्वयङ्काः_प्रत्येकम्_WORD;
    स्मृतिं_प्रतिलिख(U.pVal + loWord, sub_द्वयङ्काः.get_Raw_दत्त(),
           अङ्कः_सम्पूर्णम्_Sub_शब्दाः * APINT_WORD_आकार);

    // Mask+insert remaining bits.
    अचिह्नित remaining_द्वयङ्काः = sub_Bit_विस्तारः % APINT_द्वयङ्काः_प्रत्येकम्_WORD;
    यदि (remaining_द्वयङ्काः != 0) {
      uint64_t आवरणम्_2 = WORDTYPE_MAX >> (APINT_द्वयङ्काः_प्रत्येकम्_WORD - remaining_द्वयङ्काः);
      U.pVal[hi1Word] &= ~आवरणम्_2;
      U.pVal[hi1Word] |= sub_द्वयङ्काः.getWord(sub_Bit_विस्तारः - 1);
    }
    प्रत्यागम;
  }

  // General case - set/clear individual bits in dst based on src.
  // TODO - there is scope for optimization here, but at the moment this code
  // path is barely used so prefer readability over performance.
  प्रति (अचिह्नित i = 0; i != sub_Bit_विस्तारः; ++i)
    समुच्चयः_Bit_Val(bitPosition + i, sub_द्वयङ्काः[i]);
}

शून्य APInt::निवेशय_द्वयङ्काः(uint64_t sub_द्वयङ्काः, अचिह्नित bitPosition, अचिह्नित अङ्कः_द्वयङ्काः) {
  uint64_t आवरणम्_द्वयङ्काः = आवरणम्_Trailing_Ones<uint64_t>(अङ्कः_द्वयङ्काः);
  sub_द्वयङ्काः &= आवरणम्_द्वयङ्काः;
  यदि (LLVM_LIKELY(isSingleWord())) {
    U.VAL &= ~(आवरणम्_द्वयङ्काः << bitPosition);
    U.VAL |= sub_द्वयङ्काः << bitPosition;
    प्रत्यागम;
  }

  अचिह्नित loBit = whichBit(bitPosition);
  अचिह्नित loWord = whichWord(bitPosition);
  अचिह्नित hiWord = whichWord(bitPosition + अङ्कः_द्वयङ्काः - 1);
  यदि (loWord == hiWord) {
    U.pVal[loWord] &= ~(आवरणम्_द्वयङ्काः << loBit);
    U.pVal[loWord] |= sub_द्वयङ्काः << loBit;
    प्रत्यागम;
  }

  static_assert(8 * परिमाण(WordType) <= 64, "This code assumes only two words affected");
  अचिह्नित word_द्वयङ्काः = 8 * परिमाण(WordType);
  U.pVal[loWord] &= ~(आवरणम्_द्वयङ्काः << loBit);
  U.pVal[loWord] |= sub_द्वयङ्काः << loBit;

  U.pVal[hiWord] &= ~(आवरणम्_द्वयङ्काः >> (word_द्वयङ्काः - loBit));
  U.pVal[hiWord] |= sub_द्वयङ्काः >> (word_द्वयङ्काः - loBit);
}

APInt APInt::extract_द्वयङ्काः(अचिह्नित अङ्कः_द्वयङ्काः, अचिह्नित bitPosition) ध्रुव {
  प्रतिज्ञा(bitPosition < Bit_विस्तारः && (अङ्कः_द्वयङ्काः + bitPosition) <= Bit_विस्तारः &&
         "Illegal bit extraction");

  यदि (LLVM_LIKELY(isSingleWord()))
    प्रत्यागम APInt(अङ्कः_द्वयङ्काः, U.VAL >> bitPosition, /*isSigned=*/असत्य,
                 /*implicitTrunc=*/सत्य);

  अचिह्नित loBit = whichBit(bitPosition);
  अचिह्नित loWord = whichWord(bitPosition);
  अचिह्नित hiWord = whichWord(bitPosition + अङ्कः_द्वयङ्काः - 1);

  // Single word result extracting bits from a single word source.
  यदि (loWord == hiWord)
    प्रत्यागम APInt(अङ्कः_द्वयङ्काः, U.pVal[loWord] >> loBit, /*isSigned=*/असत्य,
                 /*implicitTrunc=*/सत्य);

  // Extracting bits that start on a source word boundary can be done
  // as a fast memory copy.
  यदि (loBit == 0)
    प्रत्यागम APInt(अङ्कः_द्वयङ्काः, ArrayRef(U.pVal + loWord, 1 + hiWord - loWord));

  // General case - shift + copy source words directly into place.
  APInt परिणाम(अङ्कः_द्वयङ्काः, 0);
  अचिह्नित अङ्कः_Src_शब्दाः = get_अङ्कः_शब्दाः();
  अचिह्नित अङ्कः_Dst_शब्दाः = परिणाम.get_अङ्कः_शब्दाः();

  uint64_t *DestPtr =
      LLVM_LIKELY(परिणाम.isSingleWord()) ? &परिणाम.U.VAL : परिणाम.U.pVal;
  प्रति (अचिह्नित word = 0; word < अङ्कः_Dst_शब्दाः; ++word) {
    uint64_t w0 = U.pVal[loWord + word];
    uint64_t w1 =
        (loWord + word + 1) < अङ्कः_Src_शब्दाः ? U.pVal[loWord + word + 1] : 0;
    DestPtr[word] = (w0 >> loBit) | (w1 << (APINT_द्वयङ्काः_प्रत्येकम्_WORD - loBit));
  }

  प्रत्यागम परिणाम.clear_अप्रयुक्तम्_द्वयङ्काः();
}

uint64_t APInt::extract_द्वयङ्काः_As_ZExt_मानम्(अचिह्नित अङ्कः_द्वयङ्काः,
                                       अचिह्नित bitPosition) ध्रुव {
  प्रतिज्ञा(bitPosition < Bit_विस्तारः && (अङ्कः_द्वयङ्काः + bitPosition) <= Bit_विस्तारः &&
         "Illegal bit extraction");
  प्रतिज्ञा(अङ्कः_द्वयङ्काः <= 64 && "Illegal bit extraction");

  uint64_t आवरणम्_द्वयङ्काः = आवरणम्_Trailing_Ones<uint64_t>(अङ्कः_द्वयङ्काः);
  यदि (LLVM_LIKELY(isSingleWord()))
    प्रत्यागम (U.VAL >> bitPosition) & आवरणम्_द्वयङ्काः;

  static_assert(APINT_द्वयङ्काः_प्रत्येकम्_WORD >= 64,
                "This code assumes only two words affected");
  अचिह्नित loBit = whichBit(bitPosition);
  अचिह्नित loWord = whichWord(bitPosition);
  अचिह्नित hiWord = whichWord(bitPosition + अङ्कः_द्वयङ्काः - 1);
  यदि (loWord == hiWord)
    प्रत्यागम (U.pVal[loWord] >> loBit) & आवरणम्_द्वयङ्काः;

  uint64_t ret_द्वयङ्काः = U.pVal[loWord] >> loBit;
  ret_द्वयङ्काः |= U.pVal[hiWord] << (APINT_द्वयङ्काः_प्रत्येकम्_WORD - loBit);
  ret_द्वयङ्काः &= आवरणम्_द्वयङ्काः;
  प्रत्यागम ret_द्वयङ्काः;
}

अचिह्नित APInt::get_Sufficient_द्वयङ्काः_Needed(StringRef Str, uint8_t Radix) {
  प्रतिज्ञा(!Str.रिक्तानि() && "Invalid string length");
  size_t StrLen = Str.आकार_2();

  // Each computation below needs to know if it's negative.
  अचिह्नित IsNegative = असत्य;
  यदि (Str[0] == '-' || Str[0] == '+') {
    IsNegative = Str[0] == '-';
    StrLen--;
    प्रतिज्ञा(StrLen && "String is only a sign, needs a value.");
  }

  // For radixes of power-of-two values, the bits required is accurately and
  // easily computed.
  यदि (Radix == 2)
    प्रत्यागम StrLen + IsNegative;
  यदि (Radix == 8)
    प्रत्यागम StrLen * 3 + IsNegative;
  यदि (Radix == 16)
    प्रत्यागम StrLen * 4 + IsNegative;

  // Compute a sufficient number of bits that is always large enough but might
  // be too large. This avoids the assertion in the constructor. This
  // calculation doesn't work appropriately for the numbers 0-9, so just use 4
  // bits in that case.
  यदि (Radix == 10)
    प्रत्यागम (StrLen == 1 ? 4 : StrLen * 64 / 18) + IsNegative;

  प्रतिज्ञा(Radix == 36);
  प्रत्यागम (StrLen == 1 ? 7 : StrLen * 16 / 3) + IsNegative;
}

अचिह्नित APInt::get_द्वयङ्काः_Needed(StringRef str, uint8_t radix) {
  // Compute a sufficient number of bits that is always large enough but might
  // be too large.
  अचिह्नित sufficient = get_Sufficient_द्वयङ्काः_Needed(str, radix);

  // For bases 2, 8, and 16, the sufficient number of bits is exact and we can
  // return the value directly. For bases 10 and 36, we need to do extra work.
  यदि (radix == 2 || radix == 8 || radix == 16)
    प्रत्यागम sufficient;

  // This is grossly inefficient but accurate. We could probably do something
  // with a computation of roughly slen*64/20 and then adjust by the value of
  // the first few digits. But, I'm not sure how accurate that could be.
  size_t slen = str.आकार_2();

  // Each computation below needs to know if it's negative.
  StringRef::iterator p = str.begin();
  अचिह्नित isNegative = *p == '-';
  यदि (*p == '-' || *p == '+') {
    p++;
    slen--;
    प्रतिज्ञा(slen && "String is only a sign, needs a value.");
  }


  // Convert to the actual binary value.
  APInt tmp(sufficient, StringRef(p, slen), radix);

  // Compute how many bits are required. If the log is infinite, assume we need
  // just bit. If the log is exact and value is negative, then the value is
  // MinSignedValue with (log + 1) bits.
  अचिह्नित लघुगणकः = tmp.logBase2();
  यदि (लघुगणकः == (अचिह्नित)-1) {
    प्रत्यागम isNegative + 1;
  } अन्यथा यदि (isNegative && tmp.isPowerOf2()) {
    प्रत्यागम isNegative + लघुगणकः;
  } अन्यथा {
    प्रत्यागम isNegative + लघुगणकः + 1;
  }
}

hash_संहिता llvm::hash_मानम्(ध्रुव APInt &Arg) {
  यदि (LLVM_LIKELY(Arg.isSingleWord()))
    प्रत्यागम hash_combine(Arg.Bit_विस्तारः, Arg.U.VAL);

  प्रत्यागम hash_combine(
      Arg.Bit_विस्तारः,
      hash_combine_range(Arg.U.pVal, Arg.U.pVal + Arg.get_अङ्कः_शब्दाः()));
}

अचिह्नित DenseMapInfo<APInt, शून्य>::get_Hash_मानम्(ध्रुव APInt &कुञ्चिका) {
  प्रत्यागम static_cast<अचिह्नित>(hash_मानम्(कुञ्चिका));
}

तार्किक APInt::isSplat(अचिह्नित Splat_आकार_अन्तर्_द्वयङ्काः) ध्रुव {
  प्रतिज्ञा(get_Bit_विस्तारः() % Splat_आकार_अन्तर्_द्वयङ्काः == 0 &&
         "SplatSizeInBits must divide width!");
  // We can check that all parts of an integer are equal by making use of a
  // little trick: rotate and check if it's still the same value.
  प्रत्यागम *एतत् == rotl(Splat_आकार_अन्तर्_द्वयङ्काः);
}

/// This function returns the high "numBits" bits of this APInt.
APInt APInt::get_Hi_द्वयङ्काः(अचिह्नित अङ्कः_द्वयङ्काः) ध्रुव {
  प्रत्यागम एतत्->lshr(Bit_विस्तारः - अङ्कः_द्वयङ्काः);
}

/// This function returns the low "numBits" bits of this APInt.
APInt APInt::get_Lo_द्वयङ्काः(अचिह्नित अङ्कः_द्वयङ्काः) ध्रुव {
  APInt परिणाम(get_नीचः_द्वयङ्काः_समुच्चयः(Bit_विस्तारः, अङ्कः_द्वयङ्काः));
  परिणाम &= *एतत्;
  प्रत्यागम परिणाम;
}

/// Return a value containing V broadcasted over NewLen bits.
APInt APInt::getSplat(अचिह्नित NewLen, ध्रुव APInt &V) {
  प्रतिज्ञा(NewLen >= V.get_Bit_विस्तारः() && "Can't splat to smaller bit width!");

  APInt Val = V.zext(NewLen);
  प्रति (अचिह्नित I = V.get_Bit_विस्तारः(); I < NewLen; I <<= 1)
    Val |= Val << I;

  प्रत्यागम Val;
}

अचिह्नित APInt::गणनम्_Leading_Zeros_Slow_Case() ध्रुव {
  अचिह्नित गणनम्_2 = 0;
  प्रति (पूर्णाङ्क i = get_अङ्कः_शब्दाः() - 1; i >= 0; --i) {
    uint64_t V = U.pVal[i];
    यदि (V == 0)
      गणनम्_2 += APINT_द्वयङ्काः_प्रत्येकम्_WORD;
    अन्यथा {
      गणनम्_2 += llvm::countl_zero(V);
      भङ्ग;
    }
  }
  // Adjust for unused bits in the most significant word (they are zero).
  अचिह्नित Mod = Bit_विस्तारः % APINT_द्वयङ्काः_प्रत्येकम्_WORD;
  गणनम्_2 -= Mod > 0 ? APINT_द्वयङ्काः_प्रत्येकम्_WORD - Mod : 0;
  प्रत्यागम गणनम्_2;
}

अचिह्नित APInt::गणनम्_Leading_Ones_Slow_Case() ध्रुव {
  अचिह्नित उच्चः_Word_द्वयङ्काः = Bit_विस्तारः % APINT_द्वयङ्काः_प्रत्येकम्_WORD;
  अचिह्नित सरणम्;
  यदि (!उच्चः_Word_द्वयङ्काः) {
    उच्चः_Word_द्वयङ्काः = APINT_द्वयङ्काः_प्रत्येकम्_WORD;
    सरणम् = 0;
  } अन्यथा {
    सरणम् = APINT_द्वयङ्काः_प्रत्येकम्_WORD - उच्चः_Word_द्वयङ्काः;
  }
  पूर्णाङ्क i = get_अङ्कः_शब्दाः() - 1;
  अचिह्नित गणनम्_2 = llvm::countl_one(U.pVal[i] << सरणम्);
  यदि (गणनम्_2 == उच्चः_Word_द्वयङ्काः) {
    प्रति (i--; i >= 0; --i) {
      यदि (U.pVal[i] == WORDTYPE_MAX)
        गणनम्_2 += APINT_द्वयङ्काः_प्रत्येकम्_WORD;
      अन्यथा {
        गणनम्_2 += llvm::countl_one(U.pVal[i]);
        भङ्ग;
      }
    }
  }
  प्रत्यागम गणनम्_2;
}

अचिह्नित APInt::गणनम्_Trailing_Zeros_Slow_Case() ध्रुव {
  अचिह्नित गणनम्_2 = 0;
  अचिह्नित i = 0;
  प्रति (; i < get_अङ्कः_शब्दाः() && U.pVal[i] == 0; ++i)
    गणनम्_2 += APINT_द्वयङ्काः_प्रत्येकम्_WORD;
  यदि (i < get_अङ्कः_शब्दाः())
    गणनम्_2 += llvm::countr_zero(U.pVal[i]);
  प्रत्यागम std::min(गणनम्_2, Bit_विस्तारः);
}

अचिह्नित APInt::गणनम्_Trailing_Ones_Slow_Case() ध्रुव {
  अचिह्नित गणनम्_2 = 0;
  अचिह्नित i = 0;
  प्रति (; i < get_अङ्कः_शब्दाः() && U.pVal[i] == WORDTYPE_MAX; ++i)
    गणनम्_2 += APINT_द्वयङ्काः_प्रत्येकम्_WORD;
  यदि (i < get_अङ्कः_शब्दाः())
    गणनम्_2 += llvm::countr_one(U.pVal[i]);
  प्रतिज्ञा(गणनम्_2 <= Bit_विस्तारः);
  प्रत्यागम गणनम्_2;
}

अचिह्नित APInt::गणनम्_Population_Slow_Case() ध्रुव {
  अचिह्नित गणनम्_2 = 0;
  प्रति (अचिह्नित i = 0; i < get_अङ्कः_शब्दाः(); ++i)
    गणनम्_2 += llvm::popcount(U.pVal[i]);
  प्रत्यागम गणनम्_2;
}

तार्किक APInt::isPowerOf2SlowCase() ध्रुव {
  अचिह्नित गणनम्_2 = 0;
  प्रति (अचिह्नित i = 0; i < get_अङ्कः_शब्दाः(); ++i) {
    गणनम्_2 += llvm::popcount(U.pVal[i]);
    यदि (गणनम्_2 > 1)
      प्रत्यागम असत्य;
  }
  प्रत्यागम गणनम्_2 == 1;
}

तार्किक APInt::intersectsSlowCase(ध्रुव APInt &RHS) ध्रुव {
  प्रति (अचिह्नित i = 0, e = get_अङ्कः_शब्दाः(); i != e; ++i)
    यदि ((U.pVal[i] & RHS.U.pVal[i]) != 0)
      प्रत्यागम सत्य;

  प्रत्यागम असत्य;
}

तार्किक APInt::is_Subset_सम्बन्धि_Slow_Case(ध्रुव APInt &RHS) ध्रुव {
  प्रति (अचिह्नित i = 0, e = get_अङ्कः_शब्दाः(); i != e; ++i)
    यदि ((U.pVal[i] & ~RHS.U.pVal[i]) != 0)
      प्रत्यागम असत्य;

  प्रत्यागम सत्य;
}

तार्किक APInt::is_Inverse_सम्बन्धि_Slow_Case(ध्रुव APInt &RHS) ध्रुव {
  ध्रुव अचिह्नित चरमः = get_अङ्कः_शब्दाः() - 1;
  प्रति (अचिह्नित I = 0; I != चरमः; ++I)
    यदि ((U.pVal[I] ^ RHS.U.pVal[I]) != WORDTYPE_MAX)
      प्रत्यागम असत्य;

  अचिह्नित पुच्छ_द्वयङ्काः = Bit_विस्तारः - चरमः * APINT_द्वयङ्काः_प्रत्येकम्_WORD;
  WordType पुच्छ_आवरणम् = llvm::आवरणम्_Trailing_Ones<WordType>(पुच्छ_द्वयङ्काः);
  प्रत्यागम (U.pVal[चरमः] ^ RHS.U.pVal[चरमः]) == पुच्छ_आवरणम्;
}

APInt APInt::byteSwap() ध्रुव {
  प्रतिज्ञा(Bit_विस्तारः >= 16 && Bit_विस्तारः % 8 == 0 && "Cannot byteswap!");
  यदि (Bit_विस्तारः == 16)
    प्रत्यागम APInt(Bit_विस्तारः, llvm::byteswap<uint16_t>(U.VAL));
  यदि (Bit_विस्तारः == 32)
    प्रत्यागम APInt(Bit_विस्तारः, llvm::byteswap<uint32_t>(U.VAL));
  यदि (Bit_विस्तारः <= 64) {
    uint64_t Tmp1 = llvm::byteswap<uint64_t>(U.VAL);
    Tmp1 >>= (64 - Bit_विस्तारः);
    प्रत्यागम APInt(Bit_विस्तारः, Tmp1);
  }

  APInt परिणाम(get_अङ्कः_शब्दाः() * APINT_द्वयङ्काः_प्रत्येकम्_WORD, 0);
  प्रति (अचिह्नित I = 0, N = get_अङ्कः_शब्दाः(); I != N; ++I)
    परिणाम.U.pVal[I] = llvm::byteswap<uint64_t>(U.pVal[N - I - 1]);
  यदि (परिणाम.Bit_विस्तारः != Bit_विस्तारः) {
    परिणाम.lshr_अन्तर्_Place(परिणाम.Bit_विस्तारः - Bit_विस्तारः);
    परिणाम.Bit_विस्तारः = Bit_विस्तारः;
  }
  प्रत्यागम परिणाम;
}

APInt APInt::reverse_द्वयङ्काः() ध्रुव {
  यदि (LLVM_LIKELY(isSingleWord())) {
    विकल्प (Bit_विस्तारः) {
    पक्ष 64:
      प्रत्यागम APInt(Bit_विस्तारः, llvm::reverse_द्वयङ्काः<uint64_t>(U.VAL));
    पक्ष 32:
      प्रत्यागम APInt(Bit_विस्तारः, llvm::reverse_द्वयङ्काः<uint32_t>(U.VAL));
    पक्ष 16:
      प्रत्यागम APInt(Bit_विस्तारः, llvm::reverse_द्वयङ्काः<uint16_t>(U.VAL));
    पक्ष 8:
      प्रत्यागम APInt(Bit_विस्तारः, llvm::reverse_द्वयङ्काः<uint8_t>(U.VAL));
    पक्ष 1: // fallthrough
    पक्ष 0:
      प्रत्यागम *एतत्;
    स्वभाव:
      प्रत्यागम APInt(Bit_विस्तारः,
                   llvm::reverse_द्वयङ्काः<uint64_t>(U.VAL) >> (64 - Bit_विस्तारः));
    }
  }

  APInt परिणाम(Bit_विस्तारः, 0);
  अचिह्नित अङ्कः_शब्दाः_2 = get_अङ्कः_शब्दाः();
  अचिह्नित Excess_द्वयङ्काः = अङ्कः_शब्दाः_2 * APINT_द्वयङ्काः_प्रत्येकम्_WORD - Bit_विस्तारः;
  यदि (Excess_द्वयङ्काः == 0) {
    // Fast path. No cross-word shift needed.
    प्रति (अचिह्नित I = 0; I < अङ्कः_शब्दाः_2; ++I)
      परिणाम.U.pVal[I] = llvm::reverse_द्वयङ्काः<uint64_t>(U.pVal[अङ्कः_शब्दाः_2 - 1 - I]);
    प्रत्यागम परिणाम;
  }
  // Holds reversed bits of the previous (more significant) word.
  uint64_t पूर्व_Rev = llvm::reverse_द्वयङ्काः<uint64_t>(U.pVal[अङ्कः_शब्दाः_2 - 1]);
  प्रति (अचिह्नित I = 0; I < अङ्कः_शब्दाः_2 - 1; ++I) {
    uint64_t CurrRev = llvm::reverse_द्वयङ्काः<uint64_t>(U.pVal[अङ्कः_शब्दाः_2 - 2 - I]);
    परिणाम.U.pVal[I] = (पूर्व_Rev >> Excess_द्वयङ्काः) | (CurrRev << (64 - Excess_द्वयङ्काः));
    पूर्व_Rev = CurrRev;
  }
  परिणाम.U.pVal[अङ्कः_शब्दाः_2 - 1] = पूर्व_Rev >> Excess_द्वयङ्काः;
  प्रत्यागम परिणाम;
}

APInt llvm::APIntOps::Greatest_Common_भाजकः(APInt A, APInt B, तार्किक IsSigned) {
  // Take absolute value if IsSigned.
  यदि (IsSigned) {
    A = A.निरपेक्षम्();
    B = B.निरपेक्षम्();
  }

  // Fast-path a common case.
  यदि (A == B) प्रत्यागम A;

  // Corner cases: if either operand is zero, the other is the gcd.
  यदि (!A) प्रत्यागम B;
  यदि (!B) प्रत्यागम A;

  // Count common powers of 2 and remove all other powers of 2.
  अचिह्नित Pow2;
  {
    अचिह्नित Pow2_A = A.countr_zero();
    अचिह्नित Pow2_B = B.countr_zero();
    यदि (Pow2_A > Pow2_B) {
      A.lshr_अन्तर्_Place(Pow2_A - Pow2_B);
      Pow2 = Pow2_B;
    } अन्यथा यदि (Pow2_B > Pow2_A) {
      B.lshr_अन्तर्_Place(Pow2_B - Pow2_A);
      Pow2 = Pow2_A;
    } अन्यथा {
      Pow2 = Pow2_A;
    }
  }

  // Both operands are odd multiples of 2^Pow_2:
  //
  //   gcd(a, b) = gcd(|a - b| / 2^i, min(a, b))
  //
  // This is a modified version of Stein's algorithm, taking advantage of
  // efficient countTrailingZeros().
  यावत् (A != B) {
    यदि (A.ugt(B)) {
      A -= B;
      A.lshr_अन्तर्_Place(A.countr_zero() - Pow2);
    } अन्यथा {
      B -= A;
      B.lshr_अन्तर्_Place(B.countr_zero() - Pow2);
    }
  }

  प्रत्यागम A;
}

APInt llvm::APIntOps::आवर्तनम्_Double_गन्तव्यम्_APInt(द्विगुण Double, अचिह्नित विस्तारः_2) {
  uint64_t I = bit_cast<uint64_t>(Double);

  // Get the sign bit from the highest order bit
  तार्किक isNeg = I >> 63;

  // Get the 11-bit exponent and adjust for the 1023 bit bias
  int64_t घातांकः = ((I >> 52) & 0x7ff) - 1023;

  // If the exponent is negative, the value is < 0 so just return 0.
  यदि (घातांकः < 0)
    प्रत्यागम APInt(विस्तारः_2, 0u);

  // Extract the mantissa by clearing the top 12 bits (sign + exponent).
  uint64_t mantissa = (I & (~0ULL >> 12)) | 1ULL << 52;

  // If the exponent doesn't shift all bits out of the mantissa
  यदि (घातांकः < 52)
    प्रत्यागम isNeg ? -APInt(विस्तारः_2, mantissa >> (52 - घातांकः)) :
                    APInt(विस्तारः_2, mantissa >> (52 - घातांकः));

  // If the client didn't provide enough bits for us to shift the mantissa into
  // then the result is undefined, just return 0
  यदि (विस्तारः_2 <= घातांकः - 52)
    प्रत्यागम APInt(विस्तारः_2, 0);

  // Otherwise, we have to shift the mantissa bits up to the right location
  APInt Tmp(विस्तारः_2, mantissa);
  Tmp <<= (अचिह्नित)घातांकः - 52;
  प्रत्यागम isNeg ? -Tmp : Tmp;
}

/// This function converts this APInt to a double.
/// The layout for double is as following (IEEE Standard 754):
///  --------------------------------------
/// |  Sign    Exponent    Fraction    Bias |
/// |-------------------------------------- |
/// |  1[63]   11[62-52]   52[51-00]   1023 |
///  --------------------------------------
द्विगुण APInt::आवर्तनम्_गन्तव्यम्_Double(तार्किक isSigned) ध्रुव {
  // Handle the simple case where the value is contained in one uint64_t.
  // It is wrong to optimize getWord(0) to VAL; there might be more than one word.
  यदि (LLVM_LIKELY(isSingleWord() || get_सक्रिय_द्वयङ्काः() <= APINT_द्वयङ्काः_प्रत्येकम्_WORD)) {
    यदि (isSigned) {
      int64_t sext = SignExtend64(getWord(0), Bit_विस्तारः);
      प्रत्यागम द्विगुण(sext);
    }
    प्रत्यागम द्विगुण(getWord(0));
  }

  // Determine if the value is negative.
  तार्किक isNeg = isSigned ? (*एतत्)[Bit_विस्तारः-1] : असत्य;

  // Construct the absolute value if we're negative.
  APInt Tmp(isNeg ? -(*एतत्) : (*एतत्));

  // Figure out how many bits we're using.
  अचिह्नित n = Tmp.get_सक्रिय_द्वयङ्काः();

  // The exponent (without bias normalization) is just the number of bits
  // we are using. Note that the sign bit is gone since we constructed the
  // absolute value.
  uint64_t घातांकः = n;

  // Return infinity for exponent overflow
  यदि (घातांकः > 1023) {
    यदि (!isSigned || !isNeg)
      प्रत्यागम std::numeric_limits<द्विगुण>::infinity();
    अन्यथा
      प्रत्यागम -std::numeric_limits<द्विगुण>::infinity();
  }
  घातांकः += 1023; // Increment for 1023 bias

  // Number of bits in mantissa is 52. To obtain the mantissa value, we must
  // extract the high 52 bits from the correct words in pVal.
  uint64_t mantissa;
  अचिह्नित hiWord = whichWord(n-1);
  यदि (hiWord == 0) {
    mantissa = Tmp.U.pVal[0];
    यदि (n > 52)
      mantissa >>= n - 52; // shift down, we want the top 52 bits.
  } अन्यथा {
    प्रतिज्ञा(hiWord > 0 && "huh?");
    uint64_t hibits = Tmp.U.pVal[hiWord] << (52 - n % APINT_द्वयङ्काः_प्रत्येकम्_WORD);
    uint64_t lobits = Tmp.U.pVal[hiWord-1] >> (11 + n % APINT_द्वयङ्काः_प्रत्येकम्_WORD);
    mantissa = hibits | lobits;
  }

  // The leading bit of mantissa is implicit, so get rid of it.
  uint64_t sign = isNeg ? (1ULL << (APINT_द्वयङ्काः_प्रत्येकम्_WORD - 1)) : 0;
  uint64_t I = sign | (घातांकः << 52) | mantissa;
  प्रत्यागम bit_cast<द्विगुण>(I);
}

// Truncate to new width.
APInt APInt::trunc(अचिह्नित विस्तारः_2) ध्रुव {
  प्रतिज्ञा(विस्तारः_2 <= Bit_विस्तारः && "Invalid APInt Truncate request");

  यदि (विस्तारः_2 <= APINT_द्वयङ्काः_प्रत्येकम्_WORD)
    प्रत्यागम APInt(विस्तारः_2, get_Raw_दत्त()[0], /*isSigned=*/असत्य,
                 /*implicitTrunc=*/सत्य);

  यदि (विस्तारः_2 == Bit_विस्तारः)
    प्रत्यागम *एतत्;

  APInt परिणाम(get_स्मृतिः(get_अङ्कः_शब्दाः(विस्तारः_2)), विस्तारः_2);

  // Copy full words.
  अचिह्नित i;
  प्रति (i = 0; i != विस्तारः_2 / APINT_द्वयङ्काः_प्रत्येकम्_WORD; i++)
    परिणाम.U.pVal[i] = U.pVal[i];

  // Truncate and copy any partial word.
  अचिह्नित द्वयङ्काः = (0 - विस्तारः_2) % APINT_द्वयङ्काः_प्रत्येकम्_WORD;
  यदि (द्वयङ्काः != 0)
    परिणाम.U.pVal[i] = U.pVal[i] << द्वयङ्काः >> द्वयङ्काः;

  प्रत्यागम परिणाम;
}

// Truncate to new width with unsigned saturation.
APInt APInt::truncUSat(अचिह्नित विस्तारः_2) ध्रुव {
  प्रतिज्ञा(विस्तारः_2 <= Bit_विस्तारः && "Invalid APInt Truncate request");

  // Can we just losslessly truncate it?
  यदि (isIntN(विस्तारः_2))
    प्रत्यागम trunc(विस्तारः_2);
  // If not, then just return the new limit.
  प्रत्यागम APInt::get_Max_मानम्(विस्तारः_2);
}

// Truncate to new width with signed saturation to signed result.
APInt APInt::truncSSat(अचिह्नित विस्तारः_2) ध्रुव {
  प्रतिज्ञा(विस्तारः_2 <= Bit_विस्तारः && "Invalid APInt Truncate request");

  // Can we just losslessly truncate it?
  यदि (isSignedIntN(विस्तारः_2))
    प्रत्यागम trunc(विस्तारः_2);
  // If not, then just return the new limits.
  प्रत्यागम isNegative() ? APInt::get_Signed_Min_मानम्(विस्तारः_2)
                      : APInt::get_Signed_Max_मानम्(विस्तारः_2);
}

// Truncate to new width with signed saturation to unsigned result.
APInt APInt::truncSSatU(अचिह्नित विस्तारः_2) ध्रुव {
  प्रतिज्ञा(विस्तारः_2 <= Bit_विस्तारः && "Invalid APInt Truncate request");

  // Can we just losslessly truncate it?
  यदि (isIntN(विस्तारः_2))
    प्रत्यागम trunc(विस्तारः_2);
  // If not, then just return the new limits.
  प्रत्यागम isNegative() ? APInt::getZero(विस्तारः_2) : APInt::get_Max_मानम्(विस्तारः_2);
}

// Sign extend to a new width.
APInt APInt::sext(अचिह्नित विस्तारः) ध्रुव {
  प्रतिज्ञा(विस्तारः >= Bit_विस्तारः && "Invalid APInt SignExtend request");

  यदि (विस्तारः <= APINT_द्वयङ्काः_प्रत्येकम्_WORD)
    प्रत्यागम APInt(विस्तारः, SignExtend64(U.VAL, Bit_विस्तारः), /*isSigned=*/सत्य);

  यदि (विस्तारः == Bit_विस्तारः)
    प्रत्यागम *एतत्;

  APInt परिणाम(get_स्मृतिः(get_अङ्कः_शब्दाः(विस्तारः)), विस्तारः);

  // Copy words.
  std::स्मृतिं_प्रतिलिख(परिणाम.U.pVal, get_Raw_दत्त(), get_अङ्कः_शब्दाः() * APINT_WORD_आकार);

  // Sign extend the last word since there may be unused bits in the input.
  परिणाम.U.pVal[get_अङ्कः_शब्दाः() - 1] =
      SignExtend64(परिणाम.U.pVal[get_अङ्कः_शब्दाः() - 1],
                   ((Bit_विस्तारः - 1) % APINT_द्वयङ्काः_प्रत्येकम्_WORD) + 1);

  // Fill with sign bits.
  std::स्मृतिं_पूरय(परिणाम.U.pVal + get_अङ्कः_शब्दाः(), isNegative() ? -1 : 0,
              (परिणाम.get_अङ्कः_शब्दाः() - get_अङ्कः_शब्दाः()) * APINT_WORD_आकार);
  परिणाम.clear_अप्रयुक्तम्_द्वयङ्काः();
  प्रत्यागम परिणाम;
}

//  Zero extend to a new width.
APInt APInt::zext(अचिह्नित विस्तारः_2) ध्रुव {
  प्रतिज्ञा(विस्तारः_2 >= Bit_विस्तारः && "Invalid APInt ZeroExtend request");

  यदि (विस्तारः_2 <= APINT_द्वयङ्काः_प्रत्येकम्_WORD)
    प्रत्यागम APInt(विस्तारः_2, U.VAL);

  यदि (विस्तारः_2 == Bit_विस्तारः)
    प्रत्यागम *एतत्;

  APInt परिणाम(get_स्मृतिः(get_अङ्कः_शब्दाः(विस्तारः_2)), विस्तारः_2);

  // Copy words.
  std::स्मृतिं_प्रतिलिख(परिणाम.U.pVal, get_Raw_दत्त(), get_अङ्कः_शब्दाः() * APINT_WORD_आकार);

  // Zero remaining words.
  std::स्मृतिं_पूरय(परिणाम.U.pVal + get_अङ्कः_शब्दाः(), 0,
              (परिणाम.get_अङ्कः_शब्दाः() - get_अङ्कः_शब्दाः()) * APINT_WORD_आकार);

  प्रत्यागम परिणाम;
}

APInt APInt::zextOrTrunc(अचिह्नित विस्तारः_2) ध्रुव {
  यदि (Bit_विस्तारः < विस्तारः_2)
    प्रत्यागम zext(विस्तारः_2);
  यदि (Bit_विस्तारः > विस्तारः_2)
    प्रत्यागम trunc(विस्तारः_2);
  प्रत्यागम *एतत्;
}

APInt APInt::sextOrTrunc(अचिह्नित विस्तारः_2) ध्रुव {
  यदि (Bit_विस्तारः < विस्तारः_2)
    प्रत्यागम sext(विस्तारः_2);
  यदि (Bit_विस्तारः > विस्तारः_2)
    प्रत्यागम trunc(विस्तारः_2);
  प्रत्यागम *एतत्;
}

/// Arithmetic right-shift this APInt by shiftAmt.
/// Arithmetic right-shift function.
शून्य APInt::ashr_अन्तर्_Place(ध्रुव APInt &सरणम्_Amt) {
  ashr_अन्तर्_Place((अचिह्नित)सरणम्_Amt.get_Limited_मानम्(Bit_विस्तारः));
}

/// Arithmetic right-shift this APInt by shiftAmt.
/// Arithmetic right-shift function.
शून्य APInt::ashrSlowCase(अचिह्नित सरणम्_Amt_2) {
  // Don't bother performing a no-op shift.
  यदि (!सरणम्_Amt_2)
    प्रत्यागम;

  // Save the original sign bit for later.
  तार्किक Negative = isNegative();

  // WordShift is the inter-part shift; BitShift is intra-part shift.
  अचिह्नित Word_सरणम् = सरणम्_Amt_2 / APINT_द्वयङ्काः_प्रत्येकम्_WORD;
  अचिह्नित Bit_सरणम् = सरणम्_Amt_2 % APINT_द्वयङ्काः_प्रत्येकम्_WORD;

  अचिह्नित शब्दाः_गन्तव्यम्_Move = get_अङ्कः_शब्दाः() - Word_सरणम्;
  यदि (शब्दाः_गन्तव्यम्_Move != 0) {
    // Sign extend the last word to fill in the unused bits.
    U.pVal[get_अङ्कः_शब्दाः() - 1] = SignExtend64(
        U.pVal[get_अङ्कः_शब्दाः() - 1], ((Bit_विस्तारः - 1) % APINT_द्वयङ्काः_प्रत्येकम्_WORD) + 1);

    // Fastpath for moving by whole words.
    यदि (Bit_सरणम् == 0) {
      std::memmove(U.pVal, U.pVal + Word_सरणम्, शब्दाः_गन्तव्यम्_Move * APINT_WORD_आकार);
    } अन्यथा {
      // Move the words containing significant bits.
      प्रति (अचिह्नित i = 0; i != शब्दाः_गन्तव्यम्_Move - 1; ++i)
        U.pVal[i] = (U.pVal[i + Word_सरणम्] >> Bit_सरणम्) |
                    (U.pVal[i + Word_सरणम् + 1] << (APINT_द्वयङ्काः_प्रत्येकम्_WORD - Bit_सरणम्));

      // Handle the last word which has no high bits to copy. Use an arithmetic
      // shift to preserve the sign bit.
      U.pVal[शब्दाः_गन्तव्यम्_Move - 1] =
          (int64_t)U.pVal[Word_सरणम् + शब्दाः_गन्तव्यम्_Move - 1] >> Bit_सरणम्;
    }
  }

  // Fill in the remainder based on the original sign.
  std::स्मृतिं_पूरय(U.pVal + शब्दाः_गन्तव्यम्_Move, Negative ? -1 : 0,
              Word_सरणम् * APINT_WORD_आकार);
  clear_अप्रयुक्तम्_द्वयङ्काः();
}

/// Logical right-shift this APInt by shiftAmt.
/// Logical right-shift function.
शून्य APInt::lshr_अन्तर्_Place(ध्रुव APInt &सरणम्_Amt) {
  lshr_अन्तर्_Place((अचिह्नित)सरणम्_Amt.get_Limited_मानम्(Bit_विस्तारः));
}

/// Logical right-shift this APInt by shiftAmt.
/// Logical right-shift function.
शून्य APInt::lshrSlowCase(अचिह्नित सरणम्_Amt_2) {
  tc_सरणम्_दक्षिणः(U.pVal, get_अङ्कः_शब्दाः(), सरणम्_Amt_2);
}

/// Left-shift this APInt by shiftAmt.
/// Left-shift function.
APInt &APInt::संक्रियकः<<=(ध्रुव APInt &सरणम्_Amt) {
  // It's undefined behavior in C to shift by BitWidth or greater.
  *एतत् <<= (अचिह्नित)सरणम्_Amt.get_Limited_मानम्(Bit_विस्तारः);
  प्रत्यागम *एतत्;
}

शून्य APInt::shlSlowCase(अचिह्नित सरणम्_Amt_2) {
  tc_सरणम्_वामः(U.pVal, get_अङ्कः_शब्दाः(), सरणम्_Amt_2);
  clear_अप्रयुक्तम्_द्वयङ्काः();
}

// Calculate the rotate amount modulo the bit width.
स्थिर अचिह्नित rotateModulo(अचिह्नित Bit_विस्तारः, ध्रुव APInt &rotateAmt) {
  यदि (LLVM_UNLIKELY(Bit_विस्तारः == 0))
    प्रत्यागम 0;
  अचिह्नित rot_Bit_विस्तारः = rotateAmt.get_Bit_विस्तारः();
  APInt rot = rotateAmt;
  यदि (rot_Bit_विस्तारः < Bit_विस्तारः) {
    // Extend the rotate APInt, so that the urem doesn't divide by 0.
    // e.g. APInt(1, 32) would give APInt(1, 0).
    rot = rotateAmt.zext(Bit_विस्तारः);
  }
  rot = rot.urem(APInt(rot.get_Bit_विस्तारः(), Bit_विस्तारः));
  प्रत्यागम rot.get_Limited_मानम्(Bit_विस्तारः);
}

APInt APInt::rotl(ध्रुव APInt &rotateAmt) ध्रुव {
  प्रत्यागम rotl(rotateModulo(Bit_विस्तारः, rotateAmt));
}

APInt APInt::rotl(अचिह्नित rotateAmt) ध्रुव {
  यदि (LLVM_UNLIKELY(Bit_विस्तारः == 0))
    प्रत्यागम *एतत्;
  rotateAmt %= Bit_विस्तारः;
  यदि (rotateAmt == 0)
    प्रत्यागम *एतत्;
  प्रत्यागम shl(rotateAmt) | lshr(Bit_विस्तारः - rotateAmt);
}

APInt APInt::rotr(ध्रुव APInt &rotateAmt) ध्रुव {
  प्रत्यागम rotr(rotateModulo(Bit_विस्तारः, rotateAmt));
}

APInt APInt::rotr(अचिह्नित rotateAmt) ध्रुव {
  यदि (Bit_विस्तारः == 0)
    प्रत्यागम *एतत्;
  rotateAmt %= Bit_विस्तारः;
  यदि (rotateAmt == 0)
    प्रत्यागम *एतत्;
  प्रत्यागम lshr(rotateAmt) | shl(Bit_विस्तारः - rotateAmt);
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
अचिह्नित APInt::निकटतमः_Log_Base2() ध्रुव {
  // Special case when we have a bitwidth of 1. If VAL is 1, then we
  // get 0. If VAL is 0, we get WORDTYPE_MAX which gets truncated to
  // UINT32_MAX.
  यदि (Bit_विस्तारः == 1)
    प्रत्यागम U.VAL - 1;

  // Handle the zero case.
  यदि (isZero())
    प्रत्यागम UINT32_MAX;

  // The non-zero case is handled by computing:
  //
  //   nearestLogBase2(x) = logBase2(x) + x[logBase2(x)-1].
  //
  // where x[i] is referring to the value of the ith bit of x.
  अचिह्नित lg = logBase2();
  प्रत्यागम lg + अचिह्नित((*एतत्)[lg - 1]);
}

// Square Root - this method computes and returns the square root of "this".
// Three mechanisms are used for computation. For small values (<= 5 bits),
// a table lookup is done. This gets some performance for common cases. For
// values using less than 52 bits, the value is converted to double and then
// the libc sqrt function is called. The result is rounded and then converted
// back to a uint64_t which is then used to construct the result. Finally,
// the Babylonian method for computing square roots is used.
APInt APInt::sqrtFloor() ध्रुव {

  // Determine the magnitude of the value.
  अचिह्नित magnitude = get_सक्रिय_द्वयङ्काः();

  // Use a fast table for some small values. This also gets rid of some
  // rounding errors in libc sqrt for small values.
  यदि (magnitude <= 5) {
    स्थिर ध्रुव uint8_t results[32] = {
        /*     0 */ 0,
        /*  1- 3 */ 1, 1, 1,
        /*  4- 8 */ 2, 2, 2, 2, 2,
        /*  9-15 */ 3, 3, 3, 3, 3, 3, 3,
        /* 16-24 */ 4, 4, 4, 4, 4, 4, 4, 4, 4,
        /* 25-31 */ 5, 5, 5, 5, 5, 5, 5,
    };
    प्रत्यागम APInt(Bit_विस्तारः,
                 results[(LLVM_LIKELY(isSingleWord()) ? U.VAL : U.pVal[0])]);
  }

  // If the magnitude of the value fits in less than 52 bits (the precision of
  // an IEEE double precision floating point value), then we can use the
  // libc sqrt function which will probably use a hardware sqrt computation.
  // This should be faster than the algorithm below.
  यदि (magnitude < 52) {
    प्रत्यागम APInt(Bit_विस्तारः,
                 uint64_t(::floor(::वर्गमूलम्(द्विगुण(
                     LLVM_LIKELY(isSingleWord()) ? U.VAL : U.pVal[0])))));
  }

  // Okay, all the short cuts are exhausted. We must compute it. The following
  // is a classical Babylonian method for computing the square root. This code
  // was adapted to APInt from a wikipedia article on such computations.
  // See http://www.wikipedia.org/ and go to the page named
  // Calculate_an_integer_square_root.
  अचिह्नित nbits = Bit_विस्तारः, i = 4;
  APInt testy(Bit_विस्तारः, 16);
  APInt x_old(Bit_विस्तारः, 1);
  APInt x_new(Bit_विस्तारः, 0);
  APInt two(Bit_विस्तारः, 2);

  // Select a good starting value using binary logarithms.
  प्रति (;; i += 2, testy = testy.shl(2))
    यदि (i >= nbits || एतत्->ule(testy)) {
      x_old = x_old.shl(i / 2);
      भङ्ग;
    }

  // Use the Babylonian method to arrive at the integer square root:
  प्रति (;;) {
    x_new = (एतत्->udiv(x_old) + x_old).udiv(two);
    यदि (x_old.ule(x_new))
      भङ्ग;
    x_old = x_new;
  }
  प्रत्यागम x_old;
}

/// \returns the multiplicative inverse of an odd APInt modulo 2^BitWidth.
APInt APInt::multiplicativeInverse() ध्रुव {
  प्रतिज्ञा((*एतत्)[0] &&
         "multiplicative inverse is only defined for odd numbers!");

  // Use Newton's method.
  APInt गुणकः = *एतत्;
  APInt T;
  यावत् (!(T = *एतत् * गुणकः).isOne())
    गुणकः *= 2 - std::move(T);
  प्रत्यागम गुणकः;
}

/// Implementation of Knuth's Algorithm D (Division of nonnegative integers)
/// from "Art of Computer Programming, Volume 2", section 4.3.1, p. 272. The
/// variables here have the same names as in the algorithm. Comments explain
/// the algorithm and any deviation from it.
स्थिर शून्य KnuthDiv(uint32_t *u, uint32_t *v, uint32_t *q, uint32_t* r,
                     अचिह्नित m, अचिह्नित n) {
  प्रतिज्ञा(u && "Must provide dividend");
  प्रतिज्ञा(v && "Must provide divisor");
  प्रतिज्ञा(q && "Must provide quotient");
  प्रतिज्ञा(u != v && u != q && v != q && "Must use different memory");
  प्रतिज्ञा(n>1 && "n must be > 1");

  // b denotes the base of the number system. In our case b is 2^32.
  ध्रुव uint64_t b = uint64_t(1) << 32;

// The DEBUG macros here tend to be spam in the debug output if you're not
// debugging this code. Disable them unless KNUTH_DEBUG is defined.
#यदि_परिभाषितम् KNUTH_DEBUG
#परिभाषय DEBUG_KNUTH(X) LLVM_DEBUG(X)
#अन्यथा
#परिभाषय DEBUG_KNUTH(X) कुरु {} यावत्(असत्य)
#यदि_अन्तः

  DEBUG_KNUTH(dbgs() << "KnuthDiv: m=" << m << " n=" << n << '\n');
  DEBUG_KNUTH(dbgs() << "KnuthDiv: original:");
  DEBUG_KNUTH(प्रति (पूर्णाङ्क i = m + n; i >= 0; i--) dbgs() << " " << u[i]);
  DEBUG_KNUTH(dbgs() << " by");
  DEBUG_KNUTH(प्रति (पूर्णाङ्क i = n; i > 0; i--) dbgs() << " " << v[i - 1]);
  DEBUG_KNUTH(dbgs() << '\n');
  // D1. [Normalize.] Set d = b / (v[n-1] + 1) and multiply all the digits of
  // u and v by d. Note that we have taken Knuth's advice here to use a power
  // of 2 value for d such that d * v[n-1] >= b/2 (b is the base). A power of
  // 2 allows us to shift instead of multiply and it is easy to determine the
  // shift amount from the leading zeros.  We are basically normalizing the u
  // and v so that its high bits are shifted to the top of v's range without
  // overflow. Note that this can require an extra word in u so that u must
  // be of length m+n+1.
  अचिह्नित सरणम् = llvm::countl_zero(v[n - 1]);
  uint32_t v_carry = 0;
  uint32_t u_carry = 0;
  यदि (सरणम्) {
    प्रति (अचिह्नित i = 0; i < m+n; ++i) {
      uint32_t u_tmp = u[i] >> (32 - सरणम्);
      u[i] = (u[i] << सरणम्) | u_carry;
      u_carry = u_tmp;
    }
    प्रति (अचिह्नित i = 0; i < n; ++i) {
      uint32_t v_tmp = v[i] >> (32 - सरणम्);
      v[i] = (v[i] << सरणम्) | v_carry;
      v_carry = v_tmp;
    }
  }
  u[m+n] = u_carry;

  DEBUG_KNUTH(dbgs() << "KnuthDiv:   normal:");
  DEBUG_KNUTH(प्रति (पूर्णाङ्क i = m + n; i >= 0; i--) dbgs() << " " << u[i]);
  DEBUG_KNUTH(dbgs() << " by");
  DEBUG_KNUTH(प्रति (पूर्णाङ्क i = n; i > 0; i--) dbgs() << " " << v[i - 1]);
  DEBUG_KNUTH(dbgs() << '\n');

  // D2. [Initialize j.]  Set j to m. This is the loop counter over the places.
  पूर्णाङ्क j = m;
  कुरु {
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
    यदि (qp == b || qp*v[n-2] > b*rp + u[j+n-2]) {
      qp--;
      rp += v[n-1];
      यदि (rp < b && (qp == b || qp*v[n-2] > b*rp + u[j+n-2]))
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
    प्रति (अचिह्नित i = 0; i < n; ++i) {
      uint64_t p = qp * uint64_t(v[i]);
      int64_t subres = int64_t(u[j+i]) - borrow - Lo_32(p);
      u[j+i] = Lo_32(subres);
      borrow = Hi_32(p) - Hi_32(subres);
      DEBUG_KNUTH(dbgs() << "KnuthDiv: u[j+i] = " << u[j + i]
                        << ", borrow = " << borrow << '\n');
    }
    तार्किक isNeg = u[j+n] < borrow;
    u[j+n] -= Lo_32(borrow);

    DEBUG_KNUTH(dbgs() << "KnuthDiv: after subtraction:");
    DEBUG_KNUTH(प्रति (पूर्णाङ्क i = m + n; i >= 0; i--) dbgs() << " " << u[i]);
    DEBUG_KNUTH(dbgs() << '\n');

    // D5. [Test remainder.] Set q[j] = qp. If the result of step D4 was
    // negative, go to step D6; otherwise go on to step D7.
    q[j] = Lo_32(qp);
    यदि (isNeg) {
      // D6. [Add back]. The probability that this step is necessary is very
      // small, on the order of only 2/b. Make sure that test data accounts for
      // this possibility. Decrease q[j] by 1
      q[j]--;
      // and add (0v[n-1]...v[1]v[0]) to (u[j+n]u[j+n-1]...u[j+1]u[j]).
      // A carry will occur to the left of u[j+n], and it should be ignored
      // since it cancels with the borrow that occurred in D4.
      तार्किक carry = असत्य;
      प्रति (अचिह्नित i = 0; i < n; i++) {
        uint32_t सीमा = std::min(u[j+i],v[i]);
        u[j+i] += v[i] + carry;
        carry = u[j+i] < सीमा || (carry && u[j+i] == सीमा);
      }
      u[j+n] += carry;
    }
    DEBUG_KNUTH(dbgs() << "KnuthDiv: after correction:");
    DEBUG_KNUTH(प्रति (पूर्णाङ्क i = m + n; i >= 0; i--) dbgs() << " " << u[i]);
    DEBUG_KNUTH(dbgs() << "\nKnuthDiv: digit result = " << q[j] << '\n');

    // D7. [Loop on j.]  Decrease j by one. Now if j >= 0, go back to D3.
  } यावत् (--j >= 0);

  DEBUG_KNUTH(dbgs() << "KnuthDiv: quotient:");
  DEBUG_KNUTH(प्रति (पूर्णाङ्क i = m; i >= 0; i--) dbgs() << " " << q[i]);
  DEBUG_KNUTH(dbgs() << '\n');

  // D8. [Unnormalize]. Now q[...] is the desired quotient, and the desired
  // remainder may be obtained by dividing u[...] by d. If r is non-null we
  // compute the remainder (urem uses this).
  यदि (r) {
    // The value d is expressed by the "shift" value above since we avoided
    // multiplication by d by using a shift left. So, all we have to do is
    // shift right here.
    यदि (सरणम्) {
      uint32_t carry = 0;
      DEBUG_KNUTH(dbgs() << "KnuthDiv: remainder:");
      प्रति (पूर्णाङ्क i = n-1; i >= 0; i--) {
        r[i] = (u[i] >> सरणम्) | carry;
        carry = u[i] << (32 - सरणम्);
        DEBUG_KNUTH(dbgs() << " " << r[i]);
      }
    } अन्यथा {
      प्रति (पूर्णाङ्क i = n-1; i >= 0; i--) {
        r[i] = u[i];
        DEBUG_KNUTH(dbgs() << " " << r[i]);
      }
    }
    DEBUG_KNUTH(dbgs() << '\n');
  }
  DEBUG_KNUTH(dbgs() << '\n');
}

शून्य APInt::divide(ध्रुव WordType *LHS, अचिह्नित lhs_शब्दाः, ध्रुव WordType *RHS,
                   अचिह्नित rhs_शब्दाः, WordType *Quotient, WordType *Remainder) {
  प्रतिज्ञा(lhs_शब्दाः >= rhs_शब्दाः && "Fractional result");

  // First, compose the values into an array of 32-bit words instead of
  // 64-bit words. This is a necessity of both the "short division" algorithm
  // and the Knuth "classical algorithm" which requires there to be native
  // operations for +, -, and * on an m bit value with an m*2 bit result. We
  // can't use 64-bit operands here because we don't have native results of
  // 128-bits. Furthermore, casting the 64-bit values to 32-bit values won't
  // work on large-endian machines.
  अचिह्नित n = rhs_शब्दाः * 2;
  अचिह्नित m = (lhs_शब्दाः * 2) - n;

  // Allocate space for the temporary values we need either on the stack, if
  // it will fit, or on the heap if it won't.
  uint32_t SPACE[128];
  uint32_t *U = शून्यसूचकः;
  uint32_t *V = शून्यसूचकः;
  uint32_t *Q = शून्यसूचकः;
  uint32_t *R = शून्यसूचकः;
  यदि ((Remainder?4:3)*n+2*m+1 <= 128) {
    U = &SPACE[0];
    V = &SPACE[m+n+1];
    Q = &SPACE[(m+n+1) + n];
    यदि (Remainder)
      R = &SPACE[(m+n+1) + n + (m+n)];
  } अन्यथा {
    U = नूतन uint32_t[m + n + 1];
    V = नूतन uint32_t[n];
    Q = नूतन uint32_t[m+n];
    यदि (Remainder)
      R = नूतन uint32_t[n];
  }

  // Initialize the dividend
  स्मृतिं_पूरय(U, 0, (m+n+1)*परिमाण(uint32_t));
  प्रति (अचिह्नित i = 0; i < lhs_शब्दाः; ++i) {
    uint64_t tmp = LHS[i];
    U[i * 2] = Lo_32(tmp);
    U[i * 2 + 1] = Hi_32(tmp);
  }
  U[m+n] = 0; // this extra word is for "spill" in the Knuth algorithm.

  // Initialize the divisor
  स्मृतिं_पूरय(V, 0, (n)*परिमाण(uint32_t));
  प्रति (अचिह्नित i = 0; i < rhs_शब्दाः; ++i) {
    uint64_t tmp = RHS[i];
    V[i * 2] = Lo_32(tmp);
    V[i * 2 + 1] = Hi_32(tmp);
  }

  // initialize the quotient and remainder
  स्मृतिं_पूरय(Q, 0, (m+n) * परिमाण(uint32_t));
  यदि (Remainder)
    स्मृतिं_पूरय(R, 0, n * परिमाण(uint32_t));

  // Now, adjust m and n for the Knuth division. n is the number of words in
  // the divisor. m is the number of words by which the dividend exceeds the
  // divisor (i.e. m+n is the length of the dividend). These sizes must not
  // contain any zero words or the Knuth algorithm fails.
  प्रति (अचिह्नित i = n; i > 0 && V[i-1] == 0; i--) {
    n--;
    m++;
  }
  प्रति (अचिह्नित i = m+n; i > 0 && U[i-1] == 0; i--)
    m--;

  // If we're left with only a single word for the divisor, Knuth doesn't work
  // so we implement the short division algorithm here. This is much simpler
  // and faster because we are certain that we can divide a 64-bit quantity
  // by a 32-bit quantity at hardware speed and short division is simply a
  // series of such operations. This is just like doing short division but we
  // are using base 2^32 instead of base 10.
  प्रतिज्ञा(n != 0 && "Divide by zero?");
  यदि (n == 1) {
    uint32_t भाजकः = V[0];
    uint32_t remainder = 0;
    प्रति (पूर्णाङ्क i = m; i >= 0; i--) {
      uint64_t partial_dividend = Make_64(remainder, U[i]);
      यदि (partial_dividend == 0) {
        Q[i] = 0;
        remainder = 0;
      } अन्यथा यदि (partial_dividend < भाजकः) {
        Q[i] = 0;
        remainder = Lo_32(partial_dividend);
      } अन्यथा यदि (partial_dividend == भाजकः) {
        Q[i] = 1;
        remainder = 0;
      } अन्यथा {
        Q[i] = Lo_32(partial_dividend / भाजकः);
        remainder = Lo_32(partial_dividend - (Q[i] * भाजकः));
      }
    }
    यदि (R)
      R[0] = remainder;
  } अन्यथा {
    // Now we're ready to invoke the Knuth classical divide algorithm. In this
    // case n > 1.
    KnuthDiv(U, V, Q, R, m, n);
  }

  // If the caller wants the quotient
  यदि (Quotient) {
    प्रति (अचिह्नित i = 0; i < lhs_शब्दाः; ++i)
      Quotient[i] = Make_64(Q[i*2+1], Q[i*2]);
  }

  // If the caller wants the remainder
  यदि (Remainder) {
    प्रति (अचिह्नित i = 0; i < rhs_शब्दाः; ++i)
      Remainder[i] = Make_64(R[i*2+1], R[i*2]);
  }

  // Clean up the memory we allocated.
  यदि (U != &SPACE[0]) {
    विलोपय [] U;
    विलोपय [] V;
    विलोपय [] Q;
    विलोपय [] R;
  }
}

APInt APInt::udiv(ध्रुव APInt &RHS) ध्रुव {
  प्रतिज्ञा(Bit_विस्तारः == RHS.Bit_विस्तारः && "Bit widths must be the same");

  // First, deal with the easy case
  यदि (LLVM_LIKELY(isSingleWord())) {
    प्रतिज्ञा(RHS.U.VAL != 0 && "Divide by zero?");
    प्रत्यागम APInt(Bit_विस्तारः, U.VAL / RHS.U.VAL);
  }

  // Get some facts about the LHS and RHS number of bits and words
  अचिह्नित lhs_शब्दाः = get_अङ्कः_शब्दाः(get_सक्रिय_द्वयङ्काः());
  अचिह्नित rhs_द्वयङ्काः  = RHS.get_सक्रिय_द्वयङ्काः();
  अचिह्नित rhs_शब्दाः = get_अङ्कः_शब्दाः(rhs_द्वयङ्काः);
  प्रतिज्ञा(rhs_शब्दाः && "Divided by zero???");

  // Deal with some degenerate cases
  यदि (!lhs_शब्दाः)
    // 0 / X ===> 0
    प्रत्यागम APInt(Bit_विस्तारः, 0);
  यदि (rhs_द्वयङ्काः == 1)
    // X / 1 ===> X
    प्रत्यागम *एतत्;
  यदि (lhs_शब्दाः < rhs_शब्दाः || एतत्->ult(RHS))
    // X / Y ===> 0, iff X < Y
    प्रत्यागम APInt(Bit_विस्तारः, 0);
  यदि (*एतत् == RHS)
    // X / X ===> 1
    प्रत्यागम APInt(Bit_विस्तारः, 1);
  यदि (lhs_शब्दाः == 1) // rhsWords is 1 if lhsWords is 1.
    // All high words are zero, just use native divide
    प्रत्यागम APInt(Bit_विस्तारः, एतत्->U.pVal[0] / RHS.U.pVal[0]);

  // We have to compute it the hard way. Invoke the Knuth divide algorithm.
  APInt Quotient(Bit_विस्तारः, 0); // to hold result.
  divide(U.pVal, lhs_शब्दाः, RHS.U.pVal, rhs_शब्दाः, Quotient.U.pVal, शून्यसूचकः);
  प्रत्यागम Quotient;
}

APInt APInt::udiv(uint64_t RHS) ध्रुव {
  प्रतिज्ञा(RHS != 0 && "Divide by zero?");

  // First, deal with the easy case
  यदि (LLVM_LIKELY(isSingleWord()))
    प्रत्यागम APInt(Bit_विस्तारः, U.VAL / RHS);

  // Get some facts about the LHS words.
  अचिह्नित lhs_शब्दाः = get_अङ्कः_शब्दाः(get_सक्रिय_द्वयङ्काः());

  // Deal with some degenerate cases
  यदि (!lhs_शब्दाः)
    // 0 / X ===> 0
    प्रत्यागम APInt(Bit_विस्तारः, 0);
  यदि (RHS == 1)
    // X / 1 ===> X
    प्रत्यागम *एतत्;
  यदि (एतत्->ult(RHS))
    // X / Y ===> 0, iff X < Y
    प्रत्यागम APInt(Bit_विस्तारः, 0);
  यदि (*एतत् == RHS)
    // X / X ===> 1
    प्रत्यागम APInt(Bit_विस्तारः, 1);
  यदि (lhs_शब्दाः == 1) // rhsWords is 1 if lhsWords is 1.
    // All high words are zero, just use native divide
    प्रत्यागम APInt(Bit_विस्तारः, एतत्->U.pVal[0] / RHS);

  // We have to compute it the hard way. Invoke the Knuth divide algorithm.
  APInt Quotient(Bit_विस्तारः, 0); // to hold result.
  divide(U.pVal, lhs_शब्दाः, &RHS, 1, Quotient.U.pVal, शून्यसूचकः);
  प्रत्यागम Quotient;
}

APInt APInt::sdiv(ध्रुव APInt &RHS) ध्रुव {
  यदि (isNegative()) {
    यदि (RHS.isNegative())
      प्रत्यागम (-(*एतत्)).udiv(-RHS);
    प्रत्यागम -((-(*एतत्)).udiv(RHS));
  }
  यदि (RHS.isNegative())
    प्रत्यागम -(एतत्->udiv(-RHS));
  प्रत्यागम एतत्->udiv(RHS);
}

APInt APInt::sdiv(int64_t RHS) ध्रुव {
  यदि (isNegative()) {
    यदि (RHS < 0)
      प्रत्यागम (-(*एतत्)).udiv(-RHS);
    प्रत्यागम -((-(*एतत्)).udiv(RHS));
  }
  यदि (RHS < 0)
    प्रत्यागम -(एतत्->udiv(-RHS));
  प्रत्यागम एतत्->udiv(RHS);
}

APInt APInt::urem(ध्रुव APInt &RHS) ध्रुव {
  प्रतिज्ञा(Bit_विस्तारः == RHS.Bit_विस्तारः && "Bit widths must be the same");
  यदि (LLVM_LIKELY(isSingleWord())) {
    प्रतिज्ञा(RHS.U.VAL != 0 && "Remainder by zero?");
    प्रत्यागम APInt(Bit_विस्तारः, U.VAL % RHS.U.VAL);
  }

  // Get some facts about the LHS
  अचिह्नित lhs_शब्दाः = get_अङ्कः_शब्दाः(get_सक्रिय_द्वयङ्काः());

  // Get some facts about the RHS
  अचिह्नित rhs_द्वयङ्काः = RHS.get_सक्रिय_द्वयङ्काः();
  अचिह्नित rhs_शब्दाः = get_अङ्कः_शब्दाः(rhs_द्वयङ्काः);
  प्रतिज्ञा(rhs_शब्दाः && "Performing remainder operation by zero ???");

  // Check the degenerate cases
  यदि (lhs_शब्दाः == 0)
    // 0 % Y ===> 0
    प्रत्यागम APInt(Bit_विस्तारः, 0);
  यदि (rhs_द्वयङ्काः == 1)
    // X % 1 ===> 0
    प्रत्यागम APInt(Bit_विस्तारः, 0);
  यदि (lhs_शब्दाः < rhs_शब्दाः || एतत्->ult(RHS))
    // X % Y ===> X, iff X < Y
    प्रत्यागम *एतत्;
  यदि (*एतत् == RHS)
    // X % X == 0;
    प्रत्यागम APInt(Bit_विस्तारः, 0);
  यदि (lhs_शब्दाः == 1)
    // All high words are zero, just use native remainder
    प्रत्यागम APInt(Bit_विस्तारः, U.pVal[0] % RHS.U.pVal[0]);
  यदि (RHS.isPowerOf2()) {
    // X % 2^w ===> X & (2^w - 1)
    APInt परिणाम(*एतत्);
    परिणाम.clear_द्वयङ्काः(RHS.logBase2(), Bit_विस्तारः);
    प्रत्यागम परिणाम;
  }

  // We have to compute it the hard way. Invoke the Knuth divide algorithm.
  APInt Remainder(Bit_विस्तारः, 0);
  divide(U.pVal, lhs_शब्दाः, RHS.U.pVal, rhs_शब्दाः, शून्यसूचकः, Remainder.U.pVal);
  प्रत्यागम Remainder;
}

uint64_t APInt::urem(uint64_t RHS) ध्रुव {
  प्रतिज्ञा(RHS != 0 && "Remainder by zero?");

  यदि (LLVM_LIKELY(isSingleWord()))
    प्रत्यागम U.VAL % RHS;

  // Get some facts about the LHS
  अचिह्नित lhs_शब्दाः = get_अङ्कः_शब्दाः(get_सक्रिय_द्वयङ्काः());

  // Check the degenerate cases
  यदि (lhs_शब्दाः == 0)
    // 0 % Y ===> 0
    प्रत्यागम 0;
  यदि (RHS == 1)
    // X % 1 ===> 0
    प्रत्यागम 0;
  यदि (एतत्->ult(RHS))
    // X % Y ===> X, iff X < Y
    प्रत्यागम get_ZExt_मानम्();
  यदि (*एतत् == RHS)
    // X % X == 0;
    प्रत्यागम 0;
  यदि (lhs_शब्दाः == 1)
    // All high words are zero, just use native remainder
    प्रत्यागम U.pVal[0] % RHS;
  यदि (llvm::isPowerOf2_64(RHS))
    // X % 2^w ===> X & (2^w - 1)
    प्रत्यागम U.pVal[0] & (RHS - 1);

  // We have to compute it the hard way. Invoke the Knuth divide algorithm.
  uint64_t Remainder;
  divide(U.pVal, lhs_शब्दाः, &RHS, 1, शून्यसूचकः, &Remainder);
  प्रत्यागम Remainder;
}

APInt APInt::srem(ध्रुव APInt &RHS) ध्रुव {
  यदि (isNegative()) {
    यदि (RHS.isNegative())
      प्रत्यागम -((-(*एतत्)).urem(-RHS));
    प्रत्यागम -((-(*एतत्)).urem(RHS));
  }
  यदि (RHS.isNegative())
    प्रत्यागम एतत्->urem(-RHS);
  प्रत्यागम एतत्->urem(RHS);
}

int64_t APInt::srem(int64_t RHS) ध्रुव {
  यदि (isNegative()) {
    यदि (RHS < 0)
      प्रत्यागम -((-(*एतत्)).urem(-RHS));
    प्रत्यागम -((-(*एतत्)).urem(RHS));
  }
  यदि (RHS < 0)
    प्रत्यागम एतत्->urem(-RHS);
  प्रत्यागम एतत्->urem(RHS);
}

शून्य APInt::udivrem(ध्रुव APInt &LHS, ध्रुव APInt &RHS,
                    APInt &Quotient, APInt &Remainder) {
  प्रतिज्ञा(LHS.Bit_विस्तारः == RHS.Bit_विस्तारः && "Bit widths must be the same");
  अचिह्नित Bit_विस्तारः = LHS.Bit_विस्तारः;

  // First, deal with the easy case
  यदि (LLVM_LIKELY(LHS.isSingleWord())) {
    प्रतिज्ञा(RHS.U.VAL != 0 && "Divide by zero?");
    uint64_t QuotVal = LHS.U.VAL / RHS.U.VAL;
    uint64_t RemVal = LHS.U.VAL % RHS.U.VAL;
    Quotient = APInt(Bit_विस्तारः, QuotVal);
    Remainder = APInt(Bit_विस्तारः, RemVal);
    प्रत्यागम;
  }

  // Get some size facts about the dividend and divisor
  अचिह्नित lhs_शब्दाः = get_अङ्कः_शब्दाः(LHS.get_सक्रिय_द्वयङ्काः());
  अचिह्नित rhs_द्वयङ्काः  = RHS.get_सक्रिय_द्वयङ्काः();
  अचिह्नित rhs_शब्दाः = get_अङ्कः_शब्दाः(rhs_द्वयङ्काः);
  प्रतिज्ञा(rhs_शब्दाः && "Performing divrem operation by zero ???");

  // Check the degenerate cases
  यदि (lhs_शब्दाः == 0) {
    Quotient = APInt(Bit_विस्तारः, 0);    // 0 / Y ===> 0
    Remainder = APInt(Bit_विस्तारः, 0);   // 0 % Y ===> 0
    प्रत्यागम;
  }

  यदि (rhs_द्वयङ्काः == 1) {
    Quotient = LHS;                   // X / 1 ===> X
    Remainder = APInt(Bit_विस्तारः, 0);   // X % 1 ===> 0
  }

  यदि (lhs_शब्दाः < rhs_शब्दाः || LHS.ult(RHS)) {
    Remainder = LHS;                  // X % Y ===> X, iff X < Y
    Quotient = APInt(Bit_विस्तारः, 0);    // X / Y ===> 0, iff X < Y
    प्रत्यागम;
  }

  यदि (LHS == RHS) {
    Quotient  = APInt(Bit_विस्तारः, 1);   // X / X ===> 1
    Remainder = APInt(Bit_विस्तारः, 0);   // X % X ===> 0;
    प्रत्यागम;
  }

  // Make sure there is enough space to hold the results.
  // NOTE: This assumes that reallocate won't affect any bits if it doesn't
  // change the size. This is necessary if Quotient or Remainder is aliased
  // with LHS or RHS.
  Quotient.reallocate(Bit_विस्तारः);
  Remainder.reallocate(Bit_विस्तारः);

  यदि (lhs_शब्दाः == 1) { // rhsWords is 1 if lhsWords is 1.
    // There is only one word to consider so use the native versions.
    uint64_t lhs_मानम् = LHS.U.pVal[0];
    uint64_t rhs_मानम् = RHS.U.pVal[0];
    Quotient = lhs_मानम् / rhs_मानम्;
    Remainder = lhs_मानम् % rhs_मानम्;
    प्रत्यागम;
  }

  // Okay, lets do it the long way
  divide(LHS.U.pVal, lhs_शब्दाः, RHS.U.pVal, rhs_शब्दाः, Quotient.U.pVal,
         Remainder.U.pVal);
  // Clear the rest of the Quotient and Remainder.
  std::स्मृतिं_पूरय(Quotient.U.pVal + lhs_शब्दाः, 0,
              (get_अङ्कः_शब्दाः(Bit_विस्तारः) - lhs_शब्दाः) * APINT_WORD_आकार);
  std::स्मृतिं_पूरय(Remainder.U.pVal + rhs_शब्दाः, 0,
              (get_अङ्कः_शब्दाः(Bit_विस्तारः) - rhs_शब्दाः) * APINT_WORD_आकार);
}

शून्य APInt::udivrem(ध्रुव APInt &LHS, uint64_t RHS, APInt &Quotient,
                    uint64_t &Remainder) {
  प्रतिज्ञा(RHS != 0 && "Divide by zero?");
  अचिह्नित Bit_विस्तारः = LHS.Bit_विस्तारः;

  // First, deal with the easy case
  यदि (LLVM_LIKELY(LHS.isSingleWord())) {
    uint64_t QuotVal = LHS.U.VAL / RHS;
    Remainder = LHS.U.VAL % RHS;
    Quotient = APInt(Bit_विस्तारः, QuotVal);
    प्रत्यागम;
  }

  // Get some size facts about the dividend and divisor
  अचिह्नित lhs_शब्दाः = get_अङ्कः_शब्दाः(LHS.get_सक्रिय_द्वयङ्काः());

  // Check the degenerate cases
  यदि (lhs_शब्दाः == 0) {
    Quotient = APInt(Bit_विस्तारः, 0);    // 0 / Y ===> 0
    Remainder = 0;                    // 0 % Y ===> 0
    प्रत्यागम;
  }

  यदि (RHS == 1) {
    Quotient = LHS;                   // X / 1 ===> X
    Remainder = 0;                    // X % 1 ===> 0
    प्रत्यागम;
  }

  यदि (LHS.ult(RHS)) {
    Remainder = LHS.get_ZExt_मानम्();   // X % Y ===> X, iff X < Y
    Quotient = APInt(Bit_विस्तारः, 0);    // X / Y ===> 0, iff X < Y
    प्रत्यागम;
  }

  यदि (LHS == RHS) {
    Quotient  = APInt(Bit_विस्तारः, 1);   // X / X ===> 1
    Remainder = 0;                    // X % X ===> 0;
    प्रत्यागम;
  }

  // Make sure there is enough space to hold the results.
  // NOTE: This assumes that reallocate won't affect any bits if it doesn't
  // change the size. This is necessary if Quotient is aliased with LHS.
  Quotient.reallocate(Bit_विस्तारः);

  यदि (lhs_शब्दाः == 1) { // rhsWords is 1 if lhsWords is 1.
    // There is only one word to consider so use the native versions.
    uint64_t lhs_मानम् = LHS.U.pVal[0];
    Quotient = lhs_मानम् / RHS;
    Remainder = lhs_मानम् % RHS;
    प्रत्यागम;
  }

  // Okay, lets do it the long way
  divide(LHS.U.pVal, lhs_शब्दाः, &RHS, 1, Quotient.U.pVal, &Remainder);
  // Clear the rest of the Quotient.
  std::स्मृतिं_पूरय(Quotient.U.pVal + lhs_शब्दाः, 0,
              (get_अङ्कः_शब्दाः(Bit_विस्तारः) - lhs_शब्दाः) * APINT_WORD_आकार);
}

शून्य APInt::sdivrem(ध्रुव APInt &LHS, ध्रुव APInt &RHS,
                    APInt &Quotient, APInt &Remainder) {
  यदि (LHS.isNegative()) {
    यदि (RHS.isNegative())
      APInt::udivrem(-LHS, -RHS, Quotient, Remainder);
    अन्यथा {
      APInt::udivrem(-LHS, RHS, Quotient, Remainder);
      Quotient.negate();
    }
    Remainder.negate();
  } अन्यथा यदि (RHS.isNegative()) {
    APInt::udivrem(LHS, -RHS, Quotient, Remainder);
    Quotient.negate();
  } अन्यथा {
    APInt::udivrem(LHS, RHS, Quotient, Remainder);
  }
}

शून्य APInt::sdivrem(ध्रुव APInt &LHS, int64_t RHS,
                    APInt &Quotient, int64_t &Remainder) {
  uint64_t R = Remainder;
  यदि (LHS.isNegative()) {
    यदि (RHS < 0)
      APInt::udivrem(-LHS, -RHS, Quotient, R);
    अन्यथा {
      APInt::udivrem(-LHS, RHS, Quotient, R);
      Quotient.negate();
    }
    R = -R;
  } अन्यथा यदि (RHS < 0) {
    APInt::udivrem(LHS, -RHS, Quotient, R);
    Quotient.negate();
  } अन्यथा {
    APInt::udivrem(LHS, RHS, Quotient, R);
  }
  Remainder = R;
}

APInt APInt::sadd_ov(ध्रुव APInt &RHS, तार्किक &अतिप्रवाहः) ध्रुव {
  APInt Res = *एतत्+RHS;
  अतिप्रवाहः = isNonNegative() == RHS.isNonNegative() &&
             Res.isNonNegative() != isNonNegative();
  प्रत्यागम Res;
}

APInt APInt::uadd_ov(ध्रुव APInt &RHS, तार्किक &अतिप्रवाहः) ध्रुव {
  APInt Res = *एतत्+RHS;
  अतिप्रवाहः = Res.ult(RHS);
  प्रत्यागम Res;
}

APInt APInt::ssub_ov(ध्रुव APInt &RHS, तार्किक &अतिप्रवाहः) ध्रुव {
  APInt Res = *एतत् - RHS;
  अतिप्रवाहः = isNonNegative() != RHS.isNonNegative() &&
             Res.isNonNegative() != isNonNegative();
  प्रत्यागम Res;
}

APInt APInt::usub_ov(ध्रुव APInt &RHS, तार्किक &अतिप्रवाहः) ध्रुव {
  APInt Res = *एतत्-RHS;
  अतिप्रवाहः = Res.ugt(*एतत्);
  प्रत्यागम Res;
}

APInt APInt::sdiv_ov(ध्रुव APInt &RHS, तार्किक &अतिप्रवाहः) ध्रुव {
  // MININT/-1  -->  overflow.
  अतिप्रवाहः = is_Min_Signed_मानम्() && RHS.isAllOnes();
  प्रत्यागम sdiv(RHS);
}

APInt APInt::smul_ov(ध्रुव APInt &RHS, तार्किक &अतिप्रवाहः) ध्रुव {
  APInt Res = *एतत् * RHS;

  यदि (RHS != 0)
    अतिप्रवाहः = Res.sdiv(RHS) != *एतत् ||
               (is_Min_Signed_मानम्() && RHS.isAllOnes());
  अन्यथा
    अतिप्रवाहः = असत्य;
  प्रत्यागम Res;
}

APInt APInt::umul_ov(ध्रुव APInt &RHS, तार्किक &अतिप्रवाहः) ध्रुव {
  यदि (countl_zero() + RHS.countl_zero() + 2 <= Bit_विस्तारः) {
    अतिप्रवाहः = सत्य;
    प्रत्यागम *एतत् * RHS;
  }

  APInt Res = lshr(1) * RHS;
  अतिप्रवाहः = Res.isNegative();
  Res <<= 1;
  यदि ((*एतत्)[0]) {
    Res += RHS;
    यदि (Res.ult(RHS))
      अतिप्रवाहः = सत्य;
  }
  प्रत्यागम Res;
}

APInt APInt::sshl_ov(ध्रुव APInt &ShAmt, तार्किक &अतिप्रवाहः) ध्रुव {
  प्रत्यागम sshl_ov(ShAmt.get_Limited_मानम्(get_Bit_विस्तारः()), अतिप्रवाहः);
}

APInt APInt::sshl_ov(अचिह्नित ShAmt, तार्किक &अतिप्रवाहः) ध्रुव {
  अतिप्रवाहः = ShAmt >= get_Bit_विस्तारः();
  यदि (अतिप्रवाहः)
    प्रत्यागम APInt(Bit_विस्तारः, 0);

  यदि (isNonNegative()) // Don't allow sign change.
    अतिप्रवाहः = ShAmt >= countl_zero();
  अन्यथा
    अतिप्रवाहः = ShAmt >= countl_one();

  प्रत्यागम *एतत् << ShAmt;
}

APInt APInt::ushl_ov(ध्रुव APInt &ShAmt, तार्किक &अतिप्रवाहः) ध्रुव {
  प्रत्यागम ushl_ov(ShAmt.get_Limited_मानम्(get_Bit_विस्तारः()), अतिप्रवाहः);
}

APInt APInt::ushl_ov(अचिह्नित ShAmt, तार्किक &अतिप्रवाहः) ध्रुव {
  अतिप्रवाहः = ShAmt >= get_Bit_विस्तारः();
  यदि (अतिप्रवाहः)
    प्रत्यागम APInt(Bit_विस्तारः, 0);

  अतिप्रवाहः = ShAmt > countl_zero();

  प्रत्यागम *एतत् << ShAmt;
}

APInt APInt::sfloordiv_ov(ध्रुव APInt &RHS, तार्किक &अतिप्रवाहः) ध्रुव {
  APInt quotient = sdiv_ov(RHS, अतिप्रवाहः);
  यदि ((quotient * RHS != *एतत्) && (isNegative() != RHS.isNegative()))
    प्रत्यागम quotient - 1;
  प्रत्यागम quotient;
}

APInt APInt::sadd_sat(ध्रुव APInt &RHS) ध्रुव {
  तार्किक अतिप्रवाहः;
  APInt Res = sadd_ov(RHS, अतिप्रवाहः);
  यदि (!अतिप्रवाहः)
    प्रत्यागम Res;

  प्रत्यागम isNegative() ? APInt::get_Signed_Min_मानम्(Bit_विस्तारः)
                      : APInt::get_Signed_Max_मानम्(Bit_विस्तारः);
}

APInt APInt::uadd_sat(ध्रुव APInt &RHS) ध्रुव {
  तार्किक अतिप्रवाहः;
  APInt Res = uadd_ov(RHS, अतिप्रवाहः);
  यदि (!अतिप्रवाहः)
    प्रत्यागम Res;

  प्रत्यागम APInt::get_Max_मानम्(Bit_विस्तारः);
}

APInt APInt::ssub_sat(ध्रुव APInt &RHS) ध्रुव {
  तार्किक अतिप्रवाहः;
  APInt Res = ssub_ov(RHS, अतिप्रवाहः);
  यदि (!अतिप्रवाहः)
    प्रत्यागम Res;

  प्रत्यागम isNegative() ? APInt::get_Signed_Min_मानम्(Bit_विस्तारः)
                      : APInt::get_Signed_Max_मानम्(Bit_विस्तारः);
}

APInt APInt::usub_sat(ध्रुव APInt &RHS) ध्रुव {
  तार्किक अतिप्रवाहः;
  APInt Res = usub_ov(RHS, अतिप्रवाहः);
  यदि (!अतिप्रवाहः)
    प्रत्यागम Res;

  प्रत्यागम APInt(Bit_विस्तारः, 0);
}

APInt APInt::smul_sat(ध्रुव APInt &RHS) ध्रुव {
  तार्किक अतिप्रवाहः;
  APInt Res = smul_ov(RHS, अतिप्रवाहः);
  यदि (!अतिप्रवाहः)
    प्रत्यागम Res;

  // The result is negative if one and only one of inputs is negative.
  तार्किक ResIsNegative = isNegative() ^ RHS.isNegative();

  प्रत्यागम ResIsNegative ? APInt::get_Signed_Min_मानम्(Bit_विस्तारः)
                       : APInt::get_Signed_Max_मानम्(Bit_विस्तारः);
}

APInt APInt::umul_sat(ध्रुव APInt &RHS) ध्रुव {
  तार्किक अतिप्रवाहः;
  APInt Res = umul_ov(RHS, अतिप्रवाहः);
  यदि (!अतिप्रवाहः)
    प्रत्यागम Res;

  प्रत्यागम APInt::get_Max_मानम्(Bit_विस्तारः);
}

APInt APInt::sshl_sat(ध्रुव APInt &RHS) ध्रुव {
  प्रत्यागम sshl_sat(RHS.get_Limited_मानम्(get_Bit_विस्तारः()));
}

APInt APInt::sshl_sat(अचिह्नित RHS) ध्रुव {
  तार्किक अतिप्रवाहः;
  APInt Res = sshl_ov(RHS, अतिप्रवाहः);
  यदि (!अतिप्रवाहः)
    प्रत्यागम Res;

  प्रत्यागम isNegative() ? APInt::get_Signed_Min_मानम्(Bit_विस्तारः)
                      : APInt::get_Signed_Max_मानम्(Bit_विस्तारः);
}

APInt APInt::ushl_sat(ध्रुव APInt &RHS) ध्रुव {
  प्रत्यागम ushl_sat(RHS.get_Limited_मानम्(get_Bit_विस्तारः()));
}

APInt APInt::ushl_sat(अचिह्नित RHS) ध्रुव {
  तार्किक अतिप्रवाहः;
  APInt Res = ushl_ov(RHS, अतिप्रवाहः);
  यदि (!अतिप्रवाहः)
    प्रत्यागम Res;

  प्रत्यागम APInt::get_Max_मानम्(Bit_विस्तारः);
}

शून्य APInt::fromString(अचिह्नित numbits, StringRef str, uint8_t radix) {
  // Check our assumptions here
  प्रतिज्ञा(!str.रिक्तानि() && "Invalid string length");
  प्रतिज्ञा((radix == 10 || radix == 8 || radix == 16 || radix == 2 ||
          radix == 36) &&
         "Radix should be 2, 8, 10, 16, or 36!");

  StringRef::iterator p = str.begin();
  size_t slen = str.आकार_2();
  तार्किक isNeg = *p == '-';
  यदि (*p == '-' || *p == '+') {
    p++;
    slen--;
    प्रतिज्ञा(slen && "String is only a sign, needs a value.");
  }
  प्रतिज्ञा((slen <= numbits || radix != 2) && "Insufficient bit width");
  प्रतिज्ञा(((slen-1)*3 <= numbits || radix != 8) && "Insufficient bit width");
  प्रतिज्ञा(((slen-1)*4 <= numbits || radix != 16) && "Insufficient bit width");
  प्रतिज्ञा((((slen-1)*64)/22 <= numbits || radix != 10) &&
         "Insufficient bit width");

  // Allocate memory if needed
  यदि (LLVM_LIKELY(isSingleWord()))
    U.VAL = 0;
  अन्यथा
    U.pVal = get_Cleared_स्मृतिः(get_अङ्कः_शब्दाः());

  // Figure out if we can shift instead of multiply
  अचिह्नित सरणम् = (radix == 16 ? 4 : radix == 8 ? 3 : radix == 2 ? 1 : 0);

  // Enter digit traversal loop
  प्रति (StringRef::iterator e = str.अन्तः(); p != e; ++p) {
    अचिह्नित digit = getDigit(*p, radix);
    प्रतिज्ञा(digit < radix && "Invalid character in digit string");

    // Shift or multiply the value by the radix
    यदि (slen > 1) {
      यदि (सरणम्)
        *एतत् <<= सरणम्;
      अन्यथा
        *एतत् *= radix;
    }

    // Add in the digit we just interpreted
    *एतत् += digit;
  }
  // If its negative, put it in two's complement form
  यदि (isNeg)
    एतत्->negate();
}

शून्य APInt::गन्तव्यम्_String(Small_सदिशम्_Impl<वर्ण> &Str, अचिह्नित Radix, तार्किक Signed,
                     तार्किक formatAsCLiteral, तार्किक ऊर्ध्वस्थम्_Case_2,
                     तार्किक निवेशय_Separators) ध्रुव {
  प्रतिज्ञा((Radix == 10 || Radix == 8 || Radix == 16 || Radix == 2 ||
          Radix == 36) &&
         "Radix should be 2, 8, 10, 16, or 36!");

  ध्रुव वर्ण *उपसर्गः = "";
  यदि (formatAsCLiteral) {
    विकल्प (Radix) {
      पक्ष 2:
        // Binary literals are a non-standard extension added in gcc 4.3:
        // http://gcc.gnu.org/onlinedocs/gcc-4.3.0/gcc/Binary-constants.html
        उपसर्गः = "0b";
        भङ्ग;
      पक्ष 8:
        उपसर्गः = "0";
        भङ्ग;
      पक्ष 10:
        भङ्ग; // No prefix
      पक्ष 16:
        उपसर्गः = "0x";
        भङ्ग;
      स्वभाव:
        llvm_unreachable("Invalid radix!");
    }
  }

  // Number of digits in a group between separators.
  अचिह्नित Grouping = (Radix == 8 || Radix == 10) ? 3 : 4;

  // First, check for a zero value and just short circuit the logic below.
  यदि (isZero()) {
    यावत् (*उपसर्गः) {
      Str.प्रक्षिप_पृष्ठभागः(*उपसर्गः);
      ++उपसर्गः;
    };
    Str.प्रक्षिप_पृष्ठभागः('0');
    प्रत्यागम;
  }

  स्थिर ध्रुव वर्ण BothDigits[] = "0123456789abcdefghijklmnopqrstuvwxyz"
                                   "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ";
  ध्रुव वर्ण *Digits = BothDigits + (ऊर्ध्वस्थम्_Case_2 ? 36 : 0);

  यदि (LLVM_LIKELY(isSingleWord())) {
    वर्ण बफर[65];
    वर्ण *BufPtr = std::अन्तः(बफर);

    uint64_t N;
    यदि (!Signed) {
      N = get_ZExt_मानम्();
    } अन्यथा {
      int64_t I = get_SExt_मानम्();
      यदि (I >= 0) {
        N = I;
      } अन्यथा {
        Str.प्रक्षिप_पृष्ठभागः('-');
        N = -(uint64_t)I;
      }
    }

    यावत् (*उपसर्गः) {
      Str.प्रक्षिप_पृष्ठभागः(*उपसर्गः);
      ++उपसर्गः;
    };

    पूर्णाङ्क Pos = 0;
    यावत् (N) {
      यदि (निवेशय_Separators && Pos % Grouping == 0 && Pos > 0)
        *--BufPtr = '\'';
      *--BufPtr = Digits[N % Radix];
      N /= Radix;
      Pos++;
    }
    Str.append(BufPtr, std::अन्तः(बफर));
    प्रत्यागम;
  }

  APInt Tmp(*एतत्);

  यदि (Signed && isNegative()) {
    // They want to print the signed version and it is a negative value
    // Flip the bits and add one to turn it into the equivalent positive
    // value and put a '-' in the result.
    Tmp.negate();
    Str.प्रक्षिप_पृष्ठभागः('-');
  }

  यावत् (*उपसर्गः) {
    Str.प्रक्षिप_पृष्ठभागः(*उपसर्गः);
    ++उपसर्गः;
  }

  // We insert the digits backward, then reverse them to get the right order.
  अचिह्नित StartDig = Str.आकार_2();

  // For the 2, 8 and 16 bit cases, we can just shift instead of divide
  // because the number of bits per digit (1, 3 and 4 respectively) divides
  // equally.  We just shift until the value is zero.
  यदि (Radix == 2 || Radix == 8 || Radix == 16) {
    // Just shift tmp right for each digit width until it becomes zero
    अचिह्नित सरणम्_Amt_2 = (Radix == 16 ? 4 : (Radix == 8 ? 3 : 1));
    अचिह्नित आवरणम्_Amt = Radix - 1;

    पूर्णाङ्क Pos = 0;
    यावत् (Tmp.get_Bool_मानम्()) {
      अचिह्नित Digit = अचिह्नित(Tmp.get_Raw_दत्त()[0]) & आवरणम्_Amt;
      यदि (निवेशय_Separators && Pos % Grouping == 0 && Pos > 0)
        Str.प्रक्षिप_पृष्ठभागः('\'');

      Str.प्रक्षिप_पृष्ठभागः(Digits[Digit]);
      Tmp.lshr_अन्तर्_Place(सरणम्_Amt_2);
      Pos++;
    }
  } अन्यथा {
    पूर्णाङ्क Pos = 0;
    यावत् (Tmp.get_Bool_मानम्()) {
      uint64_t Digit;
      udivrem(Tmp, Radix, Tmp, Digit);
      प्रतिज्ञा(Digit < Radix && "divide failed");
      यदि (निवेशय_Separators && Pos % Grouping == 0 && Pos > 0)
        Str.प्रक्षिप_पृष्ठभागः('\'');

      Str.प्रक्षिप_पृष्ठभागः(Digits[Digit]);
      Pos++;
    }
  }

  // Reverse the digits before returning.
  std::reverse(Str.begin()+StartDig, Str.अन्तः());
}

#यदि !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)
LLVM_DUMP_विधानम् शून्य APInt::dump() ध्रुव {
  SmallString<40> S, U;
  एतत्->गन्तव्यम्_String_Unsigned(U);
  एतत्->गन्तव्यम्_String_Signed(S);
  dbgs() << "APInt(" << Bit_विस्तारः << "b, "
         << U << "u " << S << "s)\n";
}
#यदि_अन्तः

शून्य APInt::print(raw_ostream &OS, तार्किक isSigned) ध्रुव {
  SmallString<40> S;
  एतत्->गन्तव्यम्_String(S, 10, isSigned, /* formatAsCLiteral = */असत्य);
  OS << S;
}

// This implements a variety of operations on a representation of
// arbitrary precision, two's-complement, bignum integer values.

// Assumed by lowHalf, highHalf, partMSB and partLSB.  A fairly safe
// and unrestricting assumption.
static_assert(APInt::APINT_द्वयङ्काः_प्रत्येकम्_WORD % 2 == 0,
              "Part width must be divisible by 2!");

// Returns the integer part with the least significant BITS set.
// BITS cannot be zero.
स्थिर अन्तःस्थ APInt::WordType नीचः_Bit_आवरणम्(अचिह्नित द्वयङ्काः) {
  प्रतिज्ञा(द्वयङ्काः != 0 && द्वयङ्काः <= APInt::APINT_द्वयङ्काः_प्रत्येकम्_WORD);
  प्रत्यागम ~(APInt::WordType) 0 >> (APInt::APINT_द्वयङ्काः_प्रत्येकम्_WORD - द्वयङ्काः);
}

/// Returns the value of the lower half of PART.
स्थिर अन्तःस्थ APInt::WordType नीचः_Half(APInt::WordType part) {
  प्रत्यागम part & नीचः_Bit_आवरणम्(APInt::APINT_द्वयङ्काः_प्रत्येकम्_WORD / 2);
}

/// Returns the value of the upper half of PART.
स्थिर अन्तःस्थ APInt::WordType उच्चः_Half(APInt::WordType part) {
  प्रत्यागम part >> (APInt::APINT_द्वयङ्काः_प्रत्येकम्_WORD / 2);
}

/// Sets the least significant part of a bignum to the input value, and zeroes
/// out higher parts.
शून्य APInt::tc_समुच्चयः(WordType *dst, WordType part, अचिह्नित parts) {
  प्रतिज्ञा(parts > 0);
  dst[0] = part;
  प्रति (अचिह्नित i = 1; i < parts; i++)
    dst[i] = 0;
}

/// Assign one bignum to another.
शून्य APInt::tc_नियोजय(WordType *dst, ध्रुव WordType *src, अचिह्नित parts) {
  प्रति (अचिह्नित i = 0; i < parts; i++)
    dst[i] = src[i];
}

/// Returns true if a bignum is zero, false otherwise.
तार्किक APInt::tcIsZero(ध्रुव WordType *src, अचिह्नित parts) {
  प्रति (अचिह्नित i = 0; i < parts; i++)
    यदि (src[i])
      प्रत्यागम असत्य;

  प्रत्यागम सत्य;
}

/// Extract the given bit of a bignum; returns 0 or 1.
पूर्णाङ्क APInt::tcExtractBit(ध्रुव WordType *parts, अचिह्नित bit) {
  प्रत्यागम (parts[whichWord(bit)] & आवरणम्_Bit(bit)) != 0;
}

/// Set the given bit of a bignum.
शून्य APInt::tc_समुच्चयः_Bit(WordType *parts, अचिह्नित bit) {
  parts[whichWord(bit)] |= आवरणम्_Bit(bit);
}

/// Clears the given bit of a bignum.
शून्य APInt::tcClearBit(WordType *parts, अचिह्नित bit) {
  parts[whichWord(bit)] &= ~आवरणम्_Bit(bit);
}

/// Returns the bit number of the least significant set bit of a number.  If the
/// input number has no bits set UINT_MAX is returned.
अचिह्नित APInt::tcLSB(ध्रुव WordType *parts, अचिह्नित n) {
  प्रति (अचिह्नित i = 0; i < n; i++) {
    यदि (parts[i] != 0) {
      अचिह्नित lsb = llvm::countr_zero(parts[i]);
      प्रत्यागम lsb + i * APINT_द्वयङ्काः_प्रत्येकम्_WORD;
    }
  }

  प्रत्यागम UINT_MAX;
}

/// Returns the bit number of the most significant set bit of a number.
/// If the input number has no bits set UINT_MAX is returned.
अचिह्नित APInt::tcMSB(ध्रुव WordType *parts, अचिह्नित n) {
  कुरु {
    --n;

    यदि (parts[n] != 0) {
      static_assert(परिमाण(parts[n]) <= परिमाण(uint64_t));
      अचिह्नित msb = llvm::Log2_64(parts[n]);

      प्रत्यागम msb + n * APINT_द्वयङ्काः_प्रत्येकम्_WORD;
    }
  } यावत् (n);

  प्रत्यागम UINT_MAX;
}

/// Copy the bit vector of width srcBITS from SRC, starting at bit srcLSB, to
/// DST, of dstCOUNT parts, such that the bit srcLSB becomes the least
/// significant bit of DST.  All high bits above srcBITS in DST are zero-filled.
/// */
शून्य
APInt::tcExtract(WordType *dst, अचिह्नित dst_गणनम्, ध्रुव WordType *src,
                 अचिह्नित src_द्वयङ्काः, अचिह्नित srcLSB) {
  अचिह्नित dstParts = (src_द्वयङ्काः + APINT_द्वयङ्काः_प्रत्येकम्_WORD - 1) / APINT_द्वयङ्काः_प्रत्येकम्_WORD;
  प्रतिज्ञा(dstParts <= dst_गणनम्);

  अचिह्नित firstSrcPart = srcLSB / APINT_द्वयङ्काः_प्रत्येकम्_WORD;
  tc_नियोजय(dst, src + firstSrcPart, dstParts);

  अचिह्नित सरणम् = srcLSB % APINT_द्वयङ्काः_प्रत्येकम्_WORD;
  tc_सरणम्_दक्षिणः(dst, dstParts, सरणम्);

  // We now have (dstParts * APINT_BITS_PER_WORD - shift) bits from SRC
  // in DST.  If this is less that srcBits, append the rest, else
  // clear the high bits.
  अचिह्नित n = dstParts * APINT_द्वयङ्काः_प्रत्येकम्_WORD - सरणम्;
  यदि (n < src_द्वयङ्काः) {
    WordType आवरणम्_2 = नीचः_Bit_आवरणम् (src_द्वयङ्काः - n);
    dst[dstParts - 1] |= ((src[firstSrcPart + dstParts] & आवरणम्_2)
                          << n % APINT_द्वयङ्काः_प्रत्येकम्_WORD);
  } अन्यथा यदि (n > src_द्वयङ्काः) {
    यदि (src_द्वयङ्काः % APINT_द्वयङ्काः_प्रत्येकम्_WORD)
      dst[dstParts - 1] &= नीचः_Bit_आवरणम् (src_द्वयङ्काः % APINT_द्वयङ्काः_प्रत्येकम्_WORD);
  }

  // Clear high parts.
  यावत् (dstParts < dst_गणनम्)
    dst[dstParts++] = 0;
}

//// DST += RHS + C where C is zero or one.  Returns the carry flag.
APInt::WordType APInt::tc_सङ्कलय(WordType *dst, ध्रुव WordType *rhs,
                             WordType c, अचिह्नित parts) {
  प्रतिज्ञा(c <= 1);

  प्रति (अचिह्नित i = 0; i < parts; i++) {
    WordType l = dst[i];
    यदि (c) {
      dst[i] += rhs[i] + 1;
      c = (dst[i] <= l);
    } अन्यथा {
      dst[i] += rhs[i];
      c = (dst[i] < l);
    }
  }

  प्रत्यागम c;
}

/// This function adds a single "word" integer, src, to the multiple
/// "word" integer array, dst[]. dst[] is modified to reflect the addition and
/// 1 is returned if there is a carry out, otherwise 0 is returned.
/// @returns the carry of the addition.
APInt::WordType APInt::tc_सङ्कलय_Part(WordType *dst, WordType src,
                                 अचिह्नित parts) {
  प्रति (अचिह्नित i = 0; i < parts; ++i) {
    dst[i] += src;
    यदि (dst[i] >= src)
      प्रत्यागम 0; // No need to carry so exit early.
    src = 1; // Carry one to next digit.
  }

  प्रत्यागम 1;
}

/// DST -= RHS + C where C is zero or one.  Returns the carry flag.
APInt::WordType APInt::tc_व्यवकलय(WordType *dst, ध्रुव WordType *rhs,
                                  WordType c, अचिह्नित parts) {
  प्रतिज्ञा(c <= 1);

  प्रति (अचिह्नित i = 0; i < parts; i++) {
    WordType l = dst[i];
    यदि (c) {
      dst[i] -= rhs[i] + 1;
      c = (dst[i] >= l);
    } अन्यथा {
      dst[i] -= rhs[i];
      c = (dst[i] > l);
    }
  }

  प्रत्यागम c;
}

/// This function subtracts a single "word" (64-bit word), src, from
/// the multi-word integer array, dst[], propagating the borrowed 1 value until
/// no further borrowing is needed or it runs out of "words" in dst.  The result
/// is 1 if "borrowing" exhausted the digits in dst, or 0 if dst was not
/// exhausted. In other words, if src > dst then this function returns 1,
/// otherwise 0.
/// @returns the borrow out of the subtraction
APInt::WordType APInt::tc_व्यवकलय_Part(WordType *dst, WordType src,
                                      अचिह्नित parts) {
  प्रति (अचिह्नित i = 0; i < parts; ++i) {
    WordType Dst = dst[i];
    dst[i] -= src;
    यदि (src <= Dst)
      प्रत्यागम 0; // No need to borrow so exit early.
    src = 1; // We have to "borrow 1" from next "word"
  }

  प्रत्यागम 1;
}

/// Negate a bignum in-place.
शून्य APInt::tcNegate(WordType *dst, अचिह्नित parts) {
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
पूर्णाङ्क APInt::tcMultiplyPart(WordType *dst, ध्रुव WordType *src,
                          WordType multiplier, WordType carry,
                          अचिह्नित srcParts, अचिह्नित dstParts,
                          तार्किक सङ्कलय) {
  // Otherwise our writes of DST kill our later reads of SRC.
  प्रतिज्ञा(dst <= src || dst >= src + srcParts);
  प्रतिज्ञा(dstParts <= srcParts + 1);

  // N loops; minimum of dstParts and srcParts.
  अचिह्नित n = std::min(dstParts, srcParts);

  प्रति (अचिह्नित i = 0; i < n; i++) {
    // [LOW, HIGH] = MULTIPLIER * SRC[i] + DST[i] + CARRY.
    // This cannot overflow, because:
    //   (n - 1) * (n - 1) + 2 (n - 1) = (n - 1) * (n + 1)
    // which is less than n^2.
    WordType srcPart = src[i];
    WordType नीचः, mid, उच्चः;
    यदि (multiplier == 0 || srcPart == 0) {
      नीचः = carry;
      उच्चः = 0;
    } अन्यथा {
      नीचः = नीचः_Half(srcPart) * नीचः_Half(multiplier);
      उच्चः = उच्चः_Half(srcPart) * उच्चः_Half(multiplier);

      mid = नीचः_Half(srcPart) * उच्चः_Half(multiplier);
      उच्चः += उच्चः_Half(mid);
      mid <<= APINT_द्वयङ्काः_प्रत्येकम्_WORD / 2;
      यदि (नीचः + mid < नीचः)
        उच्चः++;
      नीचः += mid;

      mid = उच्चः_Half(srcPart) * नीचः_Half(multiplier);
      उच्चः += उच्चः_Half(mid);
      mid <<= APINT_द्वयङ्काः_प्रत्येकम्_WORD / 2;
      यदि (नीचः + mid < नीचः)
        उच्चः++;
      नीचः += mid;

      // Now add carry.
      यदि (नीचः + carry < नीचः)
        उच्चः++;
      नीचः += carry;
    }

    यदि (सङ्कलय) {
      // And now DST[i], and store the new low part there.
      यदि (नीचः + dst[i] < नीचः)
        उच्चः++;
      dst[i] += नीचः;
    } अन्यथा {
      dst[i] = नीचः;
    }

    carry = उच्चः;
  }

  यदि (srcParts < dstParts) {
    // Full multiplication, there is no overflow.
    प्रतिज्ञा(srcParts + 1 == dstParts);
    dst[srcParts] = carry;
    प्रत्यागम 0;
  }

  // We overflowed if there is carry.
  यदि (carry)
    प्रत्यागम 1;

  // We would overflow if any significant unwritten parts would be
  // non-zero.  This is true if any remaining src parts are non-zero
  // and the multiplier is non-zero.
  यदि (multiplier)
    प्रति (अचिह्नित i = dstParts; i < srcParts; i++)
      यदि (src[i])
        प्रत्यागम 1;

  // We fitted in the narrow destination.
  प्रत्यागम 0;
}

/// DST = LHS * RHS, where DST has the same width as the operands and
/// is filled with the least significant parts of the result.  Returns
/// one if overflow occurred, otherwise zero.  DST must be disjoint
/// from both operands.
पूर्णाङ्क APInt::tcMultiply(WordType *dst, ध्रुव WordType *lhs,
                      ध्रुव WordType *rhs, अचिह्नित parts) {
  प्रतिज्ञा(dst != lhs && dst != rhs);

  पूर्णाङ्क अतिप्रवाहः_2 = 0;

  प्रति (अचिह्नित i = 0; i < parts; i++) {
    // Don't accumulate on the first iteration so we don't need to initalize
    // dst to 0.
    अतिप्रवाहः_2 |=
        tcMultiplyPart(&dst[i], lhs, rhs[i], 0, parts, parts - i, i != 0);
  }

  प्रत्यागम अतिप्रवाहः_2;
}

/// DST = LHS * RHS, where DST has width the sum of the widths of the
/// operands. No overflow occurs. DST must be disjoint from both operands.
शून्य APInt::tc_पूर्णम्_Multiply(WordType *dst, ध्रुव WordType *lhs,
                           ध्रुव WordType *rhs, अचिह्नित lhsParts,
                           अचिह्नित rhsParts) {
  // Put the narrower number on the LHS for less loops below.
  यदि (lhsParts > rhsParts)
    प्रत्यागम tc_पूर्णम्_Multiply (dst, rhs, lhs, rhsParts, lhsParts);

  प्रतिज्ञा(dst != lhs && dst != rhs);

  प्रति (अचिह्नित i = 0; i < lhsParts; i++) {
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
पूर्णाङ्क APInt::tcDivide(WordType *lhs, ध्रुव WordType *rhs,
                    WordType *remainder, WordType *srhs,
                    अचिह्नित parts) {
  प्रतिज्ञा(lhs != remainder && lhs != srhs && remainder != srhs);

  अचिह्नित सरणम्_गणनम् = tcMSB(rhs, parts) + 1;
  यदि (सरणम्_गणनम् == 0)
    प्रत्यागम सत्य;

  सरणम्_गणनम् = parts * APINT_द्वयङ्काः_प्रत्येकम्_WORD - सरणम्_गणनम्;
  अचिह्नित n = सरणम्_गणनम् / APINT_द्वयङ्काः_प्रत्येकम्_WORD;
  WordType आवरणम्_2 = (WordType) 1 << (सरणम्_गणनम् % APINT_द्वयङ्काः_प्रत्येकम्_WORD);

  tc_नियोजय(srhs, rhs, parts);
  tc_सरणम्_वामः(srhs, parts, सरणम्_गणनम्);
  tc_नियोजय(remainder, lhs, parts);
  tc_समुच्चयः(lhs, 0, parts);

  // Loop, subtracting SRHS if REMAINDER is greater and adding that to the
  // total.
  प्रति (;;) {
    पूर्णाङ्क compare = tcCompare(remainder, srhs, parts);
    यदि (compare >= 0) {
      tc_व्यवकलय(remainder, srhs, 0, parts);
      lhs[n] |= आवरणम्_2;
    }

    यदि (सरणम्_गणनम् == 0)
      भङ्ग;
    सरणम्_गणनम्--;
    tc_सरणम्_दक्षिणः(srhs, parts, 1);
    यदि ((आवरणम्_2 >>= 1) == 0) {
      आवरणम्_2 = (WordType) 1 << (APINT_द्वयङ्काः_प्रत्येकम्_WORD - 1);
      n--;
    }
  }

  प्रत्यागम असत्य;
}

/// Shift a bignum left Count bits in-place. Shifted in bits are zero. There are
/// no restrictions on Count.
शून्य APInt::tc_सरणम्_वामः(WordType *Dst, अचिह्नित शब्दाः_2, अचिह्नित गणनम्_2) {
  // Don't bother performing a no-op shift.
  यदि (!गणनम्_2)
    प्रत्यागम;

  // WordShift is the inter-part shift; BitShift is the intra-part shift.
  अचिह्नित Word_सरणम् = std::min(गणनम्_2 / APINT_द्वयङ्काः_प्रत्येकम्_WORD, शब्दाः_2);
  अचिह्नित Bit_सरणम् = गणनम्_2 % APINT_द्वयङ्काः_प्रत्येकम्_WORD;

  // Fastpath for moving by whole words.
  यदि (Bit_सरणम् == 0) {
    std::memmove(Dst + Word_सरणम्, Dst, (शब्दाः_2 - Word_सरणम्) * APINT_WORD_आकार);
  } अन्यथा {
    यावत् (शब्दाः_2-- > Word_सरणम्) {
      Dst[शब्दाः_2] = Dst[शब्दाः_2 - Word_सरणम्] << Bit_सरणम्;
      यदि (शब्दाः_2 > Word_सरणम्)
        Dst[शब्दाः_2] |=
          Dst[शब्दाः_2 - Word_सरणम् - 1] >> (APINT_द्वयङ्काः_प्रत्येकम्_WORD - Bit_सरणम्);
    }
  }

  // Fill in the remainder with 0s.
  std::स्मृतिं_पूरय(Dst, 0, Word_सरणम् * APINT_WORD_आकार);
}

/// Shift a bignum right Count bits in-place. Shifted in bits are zero. There
/// are no restrictions on Count.
शून्य APInt::tc_सरणम्_दक्षिणः(WordType *Dst, अचिह्नित शब्दाः_2, अचिह्नित गणनम्_2) {
  // Don't bother performing a no-op shift.
  यदि (!गणनम्_2)
    प्रत्यागम;

  // WordShift is the inter-part shift; BitShift is the intra-part shift.
  अचिह्नित Word_सरणम् = std::min(गणनम्_2 / APINT_द्वयङ्काः_प्रत्येकम्_WORD, शब्दाः_2);
  अचिह्नित Bit_सरणम् = गणनम्_2 % APINT_द्वयङ्काः_प्रत्येकम्_WORD;

  अचिह्नित शब्दाः_गन्तव्यम्_Move = शब्दाः_2 - Word_सरणम्;
  // Fastpath for moving by whole words.
  यदि (Bit_सरणम् == 0) {
    std::memmove(Dst, Dst + Word_सरणम्, शब्दाः_गन्तव्यम्_Move * APINT_WORD_आकार);
  } अन्यथा {
    प्रति (अचिह्नित i = 0; i != शब्दाः_गन्तव्यम्_Move; ++i) {
      Dst[i] = Dst[i + Word_सरणम्] >> Bit_सरणम्;
      यदि (i + 1 != शब्दाः_गन्तव्यम्_Move)
        Dst[i] |= Dst[i + Word_सरणम् + 1] << (APINT_द्वयङ्काः_प्रत्येकम्_WORD - Bit_सरणम्);
    }
  }

  // Fill in the remainder with 0s.
  std::स्मृतिं_पूरय(Dst + शब्दाः_गन्तव्यम्_Move, 0, Word_सरणम् * APINT_WORD_आकार);
}

// Comparison (unsigned) of two bignums.
पूर्णाङ्क APInt::tcCompare(ध्रुव WordType *lhs, ध्रुव WordType *rhs,
                     अचिह्नित parts) {
  यावत् (parts) {
    parts--;
    यदि (lhs[parts] != rhs[parts])
      प्रत्यागम (lhs[parts] > rhs[parts]) ? 1 : -1;
  }

  प्रत्यागम 0;
}

APInt llvm::APIntOps::RoundingUDiv(ध्रुव APInt &A, ध्रुव APInt &B,
                                   APInt::Rounding RM) {
  // Currently udivrem always rounds down.
  विकल्प (RM) {
  पक्ष APInt::Rounding::अधस्तात्:
  पक्ष APInt::Rounding::TOWARD_ZERO:
    प्रत्यागम A.udiv(B);
  पक्ष APInt::Rounding::ऊर्ध्वम्: {
    APInt Quo, Rem;
    APInt::udivrem(A, B, Quo, Rem);
    यदि (Rem.isZero())
      प्रत्यागम Quo;
    प्रत्यागम Quo + 1;
  }
  }
  llvm_unreachable("Unknown APInt::Rounding enum");
}

APInt llvm::APIntOps::RoundingSDiv(ध्रुव APInt &A, ध्रुव APInt &B,
                                   APInt::Rounding RM) {
  विकल्प (RM) {
  पक्ष APInt::Rounding::अधस्तात्:
  पक्ष APInt::Rounding::ऊर्ध्वम्: {
    APInt Quo, Rem;
    APInt::sdivrem(A, B, Quo, Rem);
    यदि (Rem.isZero())
      प्रत्यागम Quo;
    // This algorithm deals with arbitrary rounding mode used by sdivrem.
    // We want to check whether the non-integer part of the mathematical value
    // is negative or not. If the non-integer part is negative, we need to round
    // down from Quo; otherwise, if it's positive or 0, we return Quo, as it's
    // already rounded down.
    यदि (RM == APInt::Rounding::अधस्तात्) {
      यदि (Rem.isNegative() != B.isNegative())
        प्रत्यागम Quo - 1;
      प्रत्यागम Quo;
    }
    यदि (Rem.isNegative() != B.isNegative())
      प्रत्यागम Quo;
    प्रत्यागम Quo + 1;
  }
  // Currently sdiv rounds towards zero.
  पक्ष APInt::Rounding::TOWARD_ZERO:
    प्रत्यागम A.sdiv(B);
  }
  llvm_unreachable("Unknown APInt::Rounding enum");
}

std::optional<APInt>
llvm::APIntOps::SolveQuadraticEquationWrap(APInt A, APInt B, APInt C,
                                           अचिह्नित Range_विस्तारः) {
  अचिह्नित Coeff_विस्तारः = A.get_Bit_विस्तारः();
  प्रतिज्ञा(Coeff_विस्तारः == B.get_Bit_विस्तारः() && Coeff_विस्तारः == C.get_Bit_विस्तारः());
  प्रतिज्ञा(Range_विस्तारः <= Coeff_विस्तारः &&
         "Value range width should be less than coefficient width");
  प्रतिज्ञा(Range_विस्तारः > 1 && "Value range bit width should be > 1");

  LLVM_DEBUG(dbgs() << __func__ << ": solving " << A << "x^2 + " << B
                    << "x + " << C << ", rw:" << Range_विस्तारः << '\n');

  // Identify 0 as a (non)solution immediately.
  यदि (C.sextOrTrunc(Range_विस्तारः).isZero()) {
    LLVM_DEBUG(dbgs() << __func__ << ": zero solution\n");
    प्रत्यागम APInt(Coeff_विस्तारः, 0);
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
  Coeff_विस्तारः *= 3;
  A = A.sext(Coeff_विस्तारः);
  B = B.sext(Coeff_विस्तारः);
  C = C.sext(Coeff_विस्तारः);

  // Make A > 0 for simplicity. Negate cannot overflow at this point because
  // the bit width has increased.
  यदि (A.isNegative()) {
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
  APInt R = APInt::get_One_Bit_समुच्चयः(Coeff_विस्तारः, Range_विस्तारः);
  APInt TwoA = 2 * A;
  APInt SqrB = B * B;
  तार्किक Pick_नीचः;

  स्वचालित आवर्तनम्_ऊर्ध्वम्_2 = [] (ध्रुव APInt &V, ध्रुव APInt &A) -> APInt {
    प्रतिज्ञा(A.is_Strictly_धनात्मकम्());
    APInt T = V.निरपेक्षम्().urem(A);
    यदि (T.isZero())
      प्रत्यागम V;
    प्रत्यागम V.isNegative() ? V+T : V+(A-T);
  };

  // The vertex of the parabola is at -B/2A, but since A > 0, it's negative
  // iff B is positive.
  यदि (B.isNonNegative()) {
    // If B >= 0, the vertex it at a negative location (or at 0), so in
    // order to have a non-negative solution we need to pick k that makes
    // C-kR negative. To satisfy all the requirements for the solution
    // that we are looking for, it needs to be closest to 0 of all k.
    C = C.srem(R);
    यदि (C.is_Strictly_धनात्मकम्())
      C -= R;
    // Pick the greater solution.
    Pick_नीचः = असत्य;
  } अन्यथा {
    // If B < 0, the vertex is at a positive location. For any solution
    // to exist, the discriminant must be non-negative. This means that
    // C-kR <= B^2/4A is a necessary condition for k, i.e. there is a
    // lower bound on values of k: kR >= C - B^2/4A.
    APInt LowkR = C - SqrB.udiv(2*TwoA); // udiv because all values > 0.
    // Round LowkR up (towards +inf) to the nearest kR.
    LowkR = आवर्तनम्_ऊर्ध्वम्_2(LowkR, R);

    // If there exists k meeting the condition above, and such that
    // C-kR > 0, there will be two positive real number solutions of
    // q(x) = kR. Out of all such values of k, pick the one that makes
    // C-kR closest to 0, (i.e. pick maximum k such that C-kR > 0).
    // In other words, find maximum k such that LowkR <= kR < C.
    यदि (C.sgt(LowkR)) {
      // If LowkR < C, then such a k is guaranteed to exist because
      // LowkR itself is a multiple of R.
      C -= -आवर्तनम्_ऊर्ध्वम्_2(-C, R);      // C = C - RoundDown(C, R)
      // Pick the smaller solution.
      Pick_नीचः = सत्य;
    } अन्यथा {
      // If C-kR < 0 for all potential k's, it means that one solution
      // will be negative, while the other will be positive. The positive
      // solution will shift towards 0 if the parabola is moved up.
      // Pick the kR closest to the lower bound (i.e. make C-kR closest
      // to 0, or in other words, out of all parabolas that have solutions,
      // pick the one that is the farthest "up").
      // Since LowkR is itself a multiple of R, simply take C-LowkR.
      C -= LowkR;
      // Pick the greater solution.
      Pick_नीचः = असत्य;
    }
  }

  LLVM_DEBUG(dbgs() << __func__ << ": updated coefficients " << A << "x^2 + "
                    << B << "x + " << C << ", rw:" << Range_विस्तारः << '\n');

  APInt D = SqrB - 4*A*C;
  प्रतिज्ञा(D.isNonNegative() && "Negative discriminant");
  APInt SQ = D.sqrtFloor();

  APInt Q = SQ * SQ;
  तार्किक InexactSQ = Q != D;

  APInt X;
  APInt Rem;

  // SQ is rounded down (i.e SQ * SQ <= D), so the roots may be inexact.
  // When using the quadratic formula directly, the calculated low root
  // may be greater than the exact one, since we would be subtracting SQ.
  // To make sure that the calculated root is not greater than the exact
  // one, subtract SQ+1 when calculating the low root (for inexact value
  // of SQ).
  यदि (Pick_नीचः)
    APInt::sdivrem(-B - (SQ+InexactSQ), TwoA, X, Rem);
  अन्यथा
    APInt::sdivrem(-B + SQ, TwoA, X, Rem);

  // The updated coefficients should be such that the (exact) solution is
  // positive. Since APInt division rounds towards 0, the calculated one
  // can be 0, but cannot be negative.
  प्रतिज्ञा(X.isNonNegative() && "Solution should be non-negative");

  यदि (!InexactSQ && Rem.isZero()) {
    LLVM_DEBUG(dbgs() << __func__ << ": solution (root): " << X << '\n');
    प्रत्यागम X;
  }

  प्रतिज्ञा((SQ*SQ).sle(D) && "SQ = |_sqrt(D)_|, so SQ*SQ <= D");
  // The exact value of the square root of D should be between SQ and SQ+1.
  // This implies that the solution should be between that corresponding to
  // SQ (i.e. X) and that corresponding to SQ+1.
  //
  // The calculated X cannot be greater than the exact (real) solution.
  // Actually it must be strictly less than the exact solution, while
  // X+1 will be greater than or equal to it.

  APInt VX = (A*X + B)*X + C;
  APInt VY = VX + TwoA*X + A + B;
  तार्किक SignChange =
      VX.isNegative() != VY.isNegative() || VX.isZero() != VY.isZero();
  // If the sign did not change between X and X+1, X is not a valid solution.
  // This could happen when the actual (exact) roots don't have an integer
  // between them, so they would both be contained between X and X+1.
  यदि (!SignChange) {
    LLVM_DEBUG(dbgs() << __func__ << ": no valid solution\n");
    प्रत्यागम std::nullopt;
  }

  X += 1;
  LLVM_DEBUG(dbgs() << __func__ << ": solution (wrap): " << X << '\n');
  प्रत्यागम X;
}

std::optional<अचिह्नित>
llvm::APIntOps::GetMostSignificantDifferentBit(ध्रुव APInt &A, ध्रुव APInt &B) {
  प्रतिज्ञा(A.get_Bit_विस्तारः() == B.get_Bit_विस्तारः() && "Must have the same bitwidth");
  यदि (A == B)
    प्रत्यागम std::nullopt;
  प्रत्यागम A.get_Bit_विस्तारः() - ((A ^ B).countl_zero() + 1);
}

APInt llvm::APIntOps::Scale_Bit_आवरणम्(ध्रुव APInt &A, अचिह्नित New_Bit_विस्तारः,
                                   तार्किक मेलनम्_All_द्वयङ्काः) {
  अचिह्नित Old_Bit_विस्तारः = A.get_Bit_विस्तारः();
  प्रतिज्ञा((((Old_Bit_विस्तारः % New_Bit_विस्तारः) == 0) ||
          ((New_Bit_विस्तारः % Old_Bit_विस्तारः) == 0)) &&
         "One size should be a multiple of the other one. "
         "Can't do fractional scaling.");

  // Check for matching bitwidths.
  यदि (Old_Bit_विस्तारः == New_Bit_विस्तारः)
    प्रत्यागम A;

  APInt NewA = APInt::getZero(New_Bit_विस्तारः);

  // Check for null input.
  यदि (A.isZero())
    प्रत्यागम NewA;

  यदि (New_Bit_विस्तारः > Old_Bit_विस्तारः) {
    // Repeat bits.
    अचिह्नित Scale = New_Bit_विस्तारः / Old_Bit_विस्तारः;
    प्रति (अचिह्नित i = 0; i != Old_Bit_विस्तारः; ++i)
      यदि (A[i])
        NewA.समुच्चयः_द्वयङ्काः(i * Scale, (i + 1) * Scale);
  } अन्यथा {
    अचिह्नित Scale = Old_Bit_विस्तारः / New_Bit_विस्तारः;
    प्रति (अचिह्नित i = 0; i != New_Bit_विस्तारः; ++i) {
      यदि (मेलनम्_All_द्वयङ्काः) {
        यदि (A.extract_द्वयङ्काः(Scale, i * Scale).isAllOnes())
          NewA.समुच्चयः_Bit(i);
      } अन्यथा {
        यदि (!A.extract_द्वयङ्काः(Scale, i * Scale).isZero())
          NewA.समुच्चयः_Bit(i);
      }
    }
  }

  प्रत्यागम NewA;
}

/// StoreIntToMemory - Fills the StoreBytes bytes of memory starting from Dst
/// with the integer held in IntVal.
शून्य llvm::Store_Int_गन्तव्यम्_स्मृतिः(ध्रुव APInt &IntVal, uint8_t *Dst,
                            अचिह्नित Store_अष्टकानि) {
  प्रतिज्ञा((IntVal.get_Bit_विस्तारः()+7)/8 >= Store_अष्टकानि && "Integer too small!");
  ध्रुव uint8_t *Src = (ध्रुव uint8_t *)IntVal.get_Raw_दत्त();

  यदि (sys::IsLittleEndianHost) {
    // Little-endian host - the source is ordered from LSB to MSB.  Order the
    // destination from LSB to MSB: Do a straight copy.
    स्मृतिं_प्रतिलिख(Dst, Src, Store_अष्टकानि);
  } अन्यथा {
    // Big-endian host - the source is an array of 64 bit words ordered from
    // LSW to MSW.  Each word is ordered from MSB to LSB.  Order the destination
    // from MSB to LSB: Reverse the word order, but not the bytes in a word.
    यावत् (Store_अष्टकानि > परिमाण(uint64_t)) {
      Store_अष्टकानि -= परिमाण(uint64_t);
      // May not be aligned so use memcpy.
      स्मृतिं_प्रतिलिख(Dst + Store_अष्टकानि, Src, परिमाण(uint64_t));
      Src += परिमाण(uint64_t);
    }

    स्मृतिं_प्रतिलिख(Dst, Src + परिमाण(uint64_t) - Store_अष्टकानि, Store_अष्टकानि);
  }
}

/// LoadIntFromMemory - Loads the integer stored in the LoadBytes bytes starting
/// from Src into IntVal, which is assumed to be wide enough and to hold zero.
शून्य llvm::आरोपय_Int_From_स्मृतिः(APInt &IntVal, ध्रुव uint8_t *Src,
                             अचिह्नित आरोपय_अष्टकानि) {
  प्रतिज्ञा((IntVal.get_Bit_विस्तारः()+7)/8 >= आरोपय_अष्टकानि && "Integer too small!");
  uint8_t *Dst = reinterpret_cast<uint8_t *>(
                   const_cast<uint64_t *>(IntVal.get_Raw_दत्त()));

  यदि (sys::IsLittleEndianHost)
    // Little-endian host - the destination must be ordered from LSB to MSB.
    // The source is ordered from LSB to MSB: Do a straight copy.
    स्मृतिं_प्रतिलिख(Dst, Src, आरोपय_अष्टकानि);
  अन्यथा {
    // Big-endian - the destination is an array of 64 bit words ordered from
    // LSW to MSW.  Each word must be ordered from MSB to LSB.  The source is
    // ordered from MSB to LSB: Reverse the word order, but not the bytes in
    // a word.
    यावत् (आरोपय_अष्टकानि > परिमाण(uint64_t)) {
      आरोपय_अष्टकानि -= परिमाण(uint64_t);
      // May not be aligned so use memcpy.
      स्मृतिं_प्रतिलिख(Dst, Src + आरोपय_अष्टकानि, परिमाण(uint64_t));
      Dst += परिमाण(uint64_t);
    }

    स्मृतिं_प्रतिलिख(Dst + परिमाण(uint64_t) - आरोपय_अष्टकानि, Src, आरोपय_अष्टकानि);
  }
}

APInt APIntOps::avgFloorS(ध्रुव APInt &C1, ध्रुव APInt &C2) {
  // Return floor((C1 + C2) / 2)
  प्रत्यागम (C1 & C2) + (C1 ^ C2).ashr(1);
}

APInt APIntOps::avgFloorU(ध्रुव APInt &C1, ध्रुव APInt &C2) {
  // Return floor((C1 + C2) / 2)
  प्रत्यागम (C1 & C2) + (C1 ^ C2).lshr(1);
}

APInt APIntOps::avgCeilS(ध्रुव APInt &C1, ध्रुव APInt &C2) {
  // Return ceil((C1 + C2) / 2)
  प्रत्यागम (C1 | C2) - (C1 ^ C2).ashr(1);
}

APInt APIntOps::avgCeilU(ध्रुव APInt &C1, ध्रुव APInt &C2) {
  // Return ceil((C1 + C2) / 2)
  प्रत्यागम (C1 | C2) - (C1 ^ C2).lshr(1);
}

APInt APIntOps::avgSubFloorS(ध्रुव APInt &C1, ध्रुव APInt &C2) {
  // Return floor((C1 - C2) / 2), using C1 - C2 == (C1 ^ C2) - 2 * (~C1 & C2).
  प्रत्यागम (C1 ^ C2).ashr(1) - (~C1 & C2);
}

APInt APIntOps::avgSubFloorU(ध्रुव APInt &C1, ध्रुव APInt &C2) {
  // Return floor((C1 - C2) / 2), using C1 - C2 == (C1 ^ C2) - 2 * (~C1 & C2).
  प्रत्यागम (C1 ^ C2).lshr(1) - (~C1 & C2);
}

APInt APIntOps::mulhs(ध्रुव APInt &C1, ध्रुव APInt &C2) {
  प्रतिज्ञा(C1.get_Bit_विस्तारः() == C2.get_Bit_विस्तारः() && "Unequal bitwidths");
  अचिह्नित पूर्णम्_विस्तारः = C1.get_Bit_विस्तारः() * 2;
  APInt C1Ext = C1.sext(पूर्णम्_विस्तारः);
  APInt C2Ext = C2.sext(पूर्णम्_विस्तारः);
  प्रत्यागम (C1Ext * C2Ext).extract_द्वयङ्काः(C1.get_Bit_विस्तारः(), C1.get_Bit_विस्तारः());
}

APInt APIntOps::mulhu(ध्रुव APInt &C1, ध्रुव APInt &C2) {
  प्रतिज्ञा(C1.get_Bit_विस्तारः() == C2.get_Bit_विस्तारः() && "Unequal bitwidths");
  अचिह्नित पूर्णम्_विस्तारः = C1.get_Bit_विस्तारः() * 2;
  APInt C1Ext = C1.zext(पूर्णम्_विस्तारः);
  APInt C2Ext = C2.zext(पूर्णम्_विस्तारः);
  प्रत्यागम (C1Ext * C2Ext).extract_द्वयङ्काः(C1.get_Bit_विस्तारः(), C1.get_Bit_विस्तारः());
}

APInt APIntOps::mulsExtended(ध्रुव APInt &C1, ध्रुव APInt &C2) {
  प्रतिज्ञा(C1.get_Bit_विस्तारः() == C2.get_Bit_विस्तारः() && "Unequal bitwidths");
  अचिह्नित पूर्णम्_विस्तारः = C1.get_Bit_विस्तारः() * 2;
  APInt C1Ext = C1.sext(पूर्णम्_विस्तारः);
  APInt C2Ext = C2.sext(पूर्णम्_विस्तारः);
  प्रत्यागम C1Ext * C2Ext;
}

APInt APIntOps::muluExtended(ध्रुव APInt &C1, ध्रुव APInt &C2) {
  प्रतिज्ञा(C1.get_Bit_विस्तारः() == C2.get_Bit_विस्तारः() && "Unequal bitwidths");
  अचिह्नित पूर्णम्_विस्तारः = C1.get_Bit_विस्तारः() * 2;
  APInt C1Ext = C1.zext(पूर्णम्_विस्तारः);
  APInt C2Ext = C2.zext(पूर्णम्_विस्तारः);
  प्रत्यागम C1Ext * C2Ext;
}

APInt APIntOps::घातः(ध्रुव APInt &X, int64_t N) {
  प्रतिज्ञा(N >= 0 && "negative exponents not supported.");
  APInt Acc = APInt(X.get_Bit_विस्तारः(), 1);
  यदि (N == 0)
    प्रत्यागम Acc;
  APInt आधारः = X;
  int64_t RemainingExponent = N;
  यावत् (RemainingExponent > 0) {
    यावत् (RemainingExponent % 2 == 0) {
      आधारः *= आधारः;
      RemainingExponent /= 2;
    }
    --RemainingExponent;
    Acc *= आधारः;
  }
  प्रत्यागम Acc;
}

APInt llvm::APIntOps::fshl(ध्रुव APInt &Hi, ध्रुव APInt &Lo,
                           ध्रुव APInt &सरणम्_2) {
  प्रतिज्ञा(Hi.get_Bit_विस्तारः() == Lo.get_Bit_विस्तारः());
  अचिह्नित सरणम्_Amt_2 = rotateModulo(Hi.get_Bit_विस्तारः(), सरणम्_2);
  यदि (सरणम्_Amt_2 == 0)
    प्रत्यागम Hi;
  प्रत्यागम Hi.shl(सरणम्_Amt_2) | Lo.lshr(Hi.get_Bit_विस्तारः() - सरणम्_Amt_2);
}

APInt llvm::APIntOps::fshr(ध्रुव APInt &Hi, ध्रुव APInt &Lo,
                           ध्रुव APInt &सरणम्_2) {
  प्रतिज्ञा(Hi.get_Bit_विस्तारः() == Lo.get_Bit_विस्तारः());
  अचिह्नित सरणम्_Amt_2 = rotateModulo(Hi.get_Bit_विस्तारः(), सरणम्_2);
  यदि (सरणम्_Amt_2 == 0)
    प्रत्यागम Lo;
  प्रत्यागम Hi.shl(Hi.get_Bit_विस्तारः() - सरणम्_Amt_2) | Lo.lshr(सरणम्_Amt_2);
}

APInt llvm::APIntOps::clmul(ध्रुव APInt &LHS, ध्रुव APInt &RHS) {
  अचिह्नित BW = LHS.get_Bit_विस्तारः();
  प्रतिज्ञा(BW == RHS.get_Bit_विस्तारः() && "Operand mismatch");
  APInt परिणाम(BW, 0);
  प्रति (अचिह्नित I : seq(std::min(RHS.get_सक्रिय_द्वयङ्काः(), BW - LHS.countr_zero())))
    यदि (RHS[I])
      परिणाम ^= LHS << I;
  प्रत्यागम परिणाम;
}

APInt llvm::APIntOps::clmulr(ध्रुव APInt &LHS, ध्रुव APInt &RHS) {
  प्रतिज्ञा(LHS.get_Bit_विस्तारः() == RHS.get_Bit_विस्तारः());
  प्रत्यागम clmul(LHS.reverse_द्वयङ्काः(), RHS.reverse_द्वयङ्काः()).reverse_द्वयङ्काः();
}

APInt llvm::APIntOps::clmulh(ध्रुव APInt &LHS, ध्रुव APInt &RHS) {
  प्रतिज्ञा(LHS.get_Bit_विस्तारः() == RHS.get_Bit_विस्तारः());
  प्रत्यागम clmulr(LHS, RHS).lshr(1);
}

APInt llvm::APIntOps::pext(ध्रुव APInt &Val, ध्रुव APInt &आवरणम्) {
  अचिह्नित BW = Val.get_Bit_विस्तारः();
  प्रतिज्ञा(BW == आवरणम्.get_Bit_विस्तारः() && "Operand mismatch");
  APInt परिणाम = APInt::getZero(BW);
  प्रति (अचिह्नित I = 0, P = 0; I != BW; ++I)
    यदि (आवरणम्[I])
      परिणाम.समुच्चयः_Bit_Val(P++, Val[I]);
  प्रत्यागम परिणाम;
}

APInt llvm::APIntOps::pdep(ध्रुव APInt &Val, ध्रुव APInt &आवरणम्) {
  अचिह्नित BW = Val.get_Bit_विस्तारः();
  प्रतिज्ञा(BW == आवरणम्.get_Bit_विस्तारः() && "Operand mismatch");
  APInt परिणाम = APInt::getZero(BW);
  प्रति (अचिह्नित I = 0, P = 0; I != BW; ++I)
    यदि (आवरणम्[I])
      परिणाम.समुच्चयः_Bit_Val(I, Val[P++]);
  प्रत्यागम परिणाम;
}
