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

#অন্তর্ভাব "llvm/ADT/APInt.h"
#অন্তর্ভাব "llvm/ADT/ArrayRef.h"
#অন্তর্ভাব "llvm/ADT/FoldingSet.h"
#অন্তর্ভাব "llvm/ADT/Hashing.h"
#অন্তর্ভাব "llvm/ADT/Sequence.h"
#অন্তর্ভাব "llvm/ADT/SmallString.h"
#অন্তর্ভাব "llvm/ADT/StringRef.h"
#অন্তর্ভাব "llvm/ADT/bit.h"
#অন্তর্ভাব "llvm/Support/Alignment.h"
#অন্তর্ভাব "llvm/Support/Debug.h"
#অন্তর্ভাব "llvm/Support/ErrorHandling.h"
#অন্তর্ভাব "llvm/Support/MathExtras.h"
#অন্তর্ভাব "llvm/Support/SwapByteOrder.h"
#অন্তর্ভাব "llvm/Support/raw_ostream.h"
#অন্তর্ভাব <cmath>
#অন্তর্ভাব <optional>

using নামাকাশ llvm;

#define DEBUG_TYPE "apint"

/// A utility function for allocating memory, checking for allocation failures,
/// and ensuring the contents are zeroed.
অন্তঃরেখ স্থির uint64_t* getClearedMemory(unsigned অঙ্ক_শব্দগুলি) {
  ফেরত নতুন uint64_t[অঙ্ক_শব্দগুলি]();
}

/// A utility function for allocating memory and checking for allocation
/// failure.  The content is not zeroed.
অন্তঃরেখ স্থির uint64_t* getMemory(unsigned অঙ্ক_শব্দগুলি) {
  ফেরত নতুন uint64_t[অঙ্ক_শব্দগুলি];
}

/// A utility function that converts a character to a digit.
অন্তঃরেখ স্থির unsigned getDigit(অক্ষর cdigit, uint8_t radix) {
  unsigned r;

  যদি (radix == 16 || radix == 36) {
    r = cdigit - '0';
    যদি (r <= 9)
      ফেরত r;

    r = cdigit - 'A';
    যদি (r <= radix - 11U)
      ফেরত r + 10;

    r = cdigit - 'a';
    যদি (r <= radix - 11U)
      ফেরত r + 10;

    radix = 10;
  }

  r = cdigit - '0';
  যদি (r < radix)
    ফেরত r;

  ফেরত UINT_MAX;
}


শূন্য APInt::initSlowCase(uint64_t val, যৌক্তিক isSigned) {
  যদি (isSigned && int64_t(val) < 0) {
    U.pVal = getMemory(get_অঙ্ক_শব্দগুলি());
    U.pVal[0] = val;
    memset(&U.pVal[1], 0xFF, APINT_WORD_আকার * (get_অঙ্ক_শব্দগুলি() - 1));
    clearUnusedBits();
  } অন্যথা {
    U.pVal = getClearedMemory(get_অঙ্ক_শব্দগুলি());
    U.pVal[0] = val;
  }
}

শূন্য APInt::initSlowCase(ধ্রুবক APInt& that) {
  U.pVal = getMemory(get_অঙ্ক_শব্দগুলি());
  memcpy(U.pVal, that.U.pVal, get_অঙ্ক_শব্দগুলি() * APINT_WORD_আকার);
}

শূন্য APInt::initFromArray(ArrayRef<uint64_t> bigVal) {
  প্রতিজ্ঞা(bigVal.তথ্য() && "Null pointer detected!");
  যদি (LLVM_LIKELY(isSingleWord()))
    U.VAL = bigVal[0];
  অন্যথা {
    // Get memory, cleared to 0
    U.pVal = getClearedMemory(get_অঙ্ক_শব্দগুলি());
    // Calculate the number of words to copy
    unsigned শব্দগুলি = std::min<unsigned>(bigVal.আকার_3(), get_অঙ্ক_শব্দগুলি());
    // Copy the words from bigVal to pVal
    memcpy(U.pVal, bigVal.তথ্য(), শব্দগুলি * APINT_WORD_আকার);
  }
  // Make sure unused high bits are cleared
  clearUnusedBits();
}

APInt::APInt(unsigned অঙ্ক_Bits, ArrayRef<uint64_t> bigVal) : BitWidth(অঙ্ক_Bits) {
  initFromArray(bigVal);
}

APInt::APInt(unsigned numbits, StringRef Str, uint8_t radix)
    : BitWidth(numbits) {
  fromString(numbits, Str, radix);
}

শূন্য APInt::reallocate(unsigned NewBitWidth) {
  // If the number of words is the same we can just change the width and stop.
  যদি (get_অঙ্ক_শব্দগুলি() == get_অঙ্ক_শব্দগুলি(NewBitWidth)) {
    BitWidth = NewBitWidth;
    ফেরত;
  }

  // If we have an allocation, delete it.
  যদি (LLVM_UNLIKELY(!isSingleWord()))
    delete [] U.pVal;

  // Update BitWidth.
  BitWidth = NewBitWidth;

  // If we are supposed to have an allocation, create it.
  যদি (LLVM_UNLIKELY(!isSingleWord()))
    U.pVal = getMemory(get_অঙ্ক_শব্দগুলি());
}

শূন্য APInt::assignSlowCase(ধ্রুবক APInt &RHS) {
  // Don't do anything for X = X
  যদি (এই == &RHS)
    ফেরত;

  // Adjust the bit width and handle allocations as necessary.
  reallocate(RHS.getBitWidth());

  // Copy the data.
  যদি (LLVM_LIKELY(isSingleWord()))
    U.VAL = RHS.U.VAL;
  অন্যথা
    memcpy(U.pVal, RHS.U.pVal, get_অঙ্ক_শব্দগুলি() * APINT_WORD_আকার);
}

/// This method 'profiles' an APInt for use with FoldingSet.
শূন্য APInt::Profile(Folding_Set_গ্রন্থি_ID& ID) ধ্রুবক {
  ID.AddInteger(BitWidth);

  যদি (LLVM_LIKELY(isSingleWord())) {
    ID.AddInteger(U.VAL);
    ফেরত;
  }

  unsigned অঙ্ক_শব্দগুলি_2 = get_অঙ্ক_শব্দগুলি();
  জন্য (unsigned i = 0; i < অঙ্ক_শব্দগুলি_2; ++i)
    ID.AddInteger(U.pVal[i]);
}

যৌক্তিক APInt::isAligned(Align A) ধ্রুবক {
  যদি (isZero())
    ফেরত সত্য;
  ধ্রুবক unsigned TrailingZeroes = countr_zero();
  ধ্রুবক unsigned MinimumTrailingZeroes = Log2(A);
  ফেরত TrailingZeroes >= MinimumTrailingZeroes;
}

/// Prefix increment operator. Increments the APInt by one.
APInt& APInt::operator++() {
  যদি (LLVM_LIKELY(isSingleWord()))
    ++U.VAL;
  অন্যথা
    tcIncrement(U.pVal, get_অঙ্ক_শব্দগুলি());
  ফেরত clearUnusedBits();
}

/// Prefix decrement operator. Decrements the APInt by one.
APInt& APInt::operator--() {
  যদি (LLVM_LIKELY(isSingleWord()))
    --U.VAL;
  অন্যথা
    tcDecrement(U.pVal, get_অঙ্ক_শব্দগুলি());
  ফেরত clearUnusedBits();
}

/// Adds the RHS APInt to this APInt.
/// @returns this, after addition of RHS.
/// Addition assignment operator.
APInt& APInt::operator+=(ধ্রুবক APInt& RHS) {
  প্রতিজ্ঞা(BitWidth == RHS.BitWidth && "Bit widths must be the same");
  যদি (LLVM_LIKELY(isSingleWord()))
    U.VAL += RHS.U.VAL;
  অন্যথা
    tcAdd(U.pVal, RHS.U.pVal, 0, get_অঙ্ক_শব্দগুলি());
  ফেরত clearUnusedBits();
}

APInt& APInt::operator+=(uint64_t RHS) {
  যদি (LLVM_LIKELY(isSingleWord()))
    U.VAL += RHS;
  অন্যথা
    tcAddPart(U.pVal, RHS, get_অঙ্ক_শব্দগুলি());
  ফেরত clearUnusedBits();
}

/// Subtracts the RHS APInt from this APInt
/// @returns this, after subtraction
/// Subtraction assignment operator.
APInt& APInt::operator-=(ধ্রুবক APInt& RHS) {
  প্রতিজ্ঞা(BitWidth == RHS.BitWidth && "Bit widths must be the same");
  যদি (LLVM_LIKELY(isSingleWord()))
    U.VAL -= RHS.U.VAL;
  অন্যথা
    tcSubtract(U.pVal, RHS.U.pVal, 0, get_অঙ্ক_শব্দগুলি());
  ফেরত clearUnusedBits();
}

APInt& APInt::operator-=(uint64_t RHS) {
  যদি (LLVM_LIKELY(isSingleWord()))
    U.VAL -= RHS;
  অন্যথা
    tcSubtractPart(U.pVal, RHS, get_অঙ্ক_শব্দগুলি());
  ফেরত clearUnusedBits();
}

APInt APInt::operator*(ধ্রুবক APInt& RHS) ধ্রুবক {
  প্রতিজ্ঞা(BitWidth == RHS.BitWidth && "Bit widths must be the same");
  যদি (LLVM_LIKELY(isSingleWord()))
    ফেরত APInt(BitWidth, U.VAL * RHS.U.VAL, /*isSigned=*/মিথ্যা,
                 /*implicitTrunc=*/সত্য);

  APInt ফলাফল(getMemory(get_অঙ্ক_শব্দগুলি()), getBitWidth());
  tcMultiply(ফলাফল.U.pVal, U.pVal, RHS.U.pVal, get_অঙ্ক_শব্দগুলি());
  ফলাফল.clearUnusedBits();
  ফেরত ফলাফল;
}

শূন্য APInt::andAssignSlowCase(ধ্রুবক APInt &RHS) {
  WordType *dst = U.pVal, *rhs = RHS.U.pVal;
  জন্য (size_t i = 0, e = get_অঙ্ক_শব্দগুলি(); i != e; ++i)
    dst[i] &= rhs[i];
}

শূন্য APInt::orAssignSlowCase(ধ্রুবক APInt &RHS) {
  WordType *dst = U.pVal, *rhs = RHS.U.pVal;
  জন্য (size_t i = 0, e = get_অঙ্ক_শব্দগুলি(); i != e; ++i)
    dst[i] |= rhs[i];
}

শূন্য APInt::xorAssignSlowCase(ধ্রুবক APInt &RHS) {
  WordType *dst = U.pVal, *rhs = RHS.U.pVal;
  জন্য (size_t i = 0, e = get_অঙ্ক_শব্দগুলি(); i != e; ++i)
    dst[i] ^= rhs[i];
}

APInt &APInt::operator*=(ধ্রুবক APInt &RHS) {
  *এই = *এই * RHS;
  ফেরত *এই;
}

APInt& APInt::operator*=(uint64_t RHS) {
  যদি (LLVM_LIKELY(isSingleWord())) {
    U.VAL *= RHS;
  } অন্যথা {
    unsigned অঙ্ক_শব্দগুলি_2 = get_অঙ্ক_শব্দগুলি();
    tcMultiplyPart(U.pVal, U.pVal, RHS, 0, অঙ্ক_শব্দগুলি_2, অঙ্ক_শব্দগুলি_2, মিথ্যা);
  }
  ফেরত clearUnusedBits();
}

যৌক্তিক APInt::equalSlowCase(ধ্রুবক APInt &RHS) ধ্রুবক {
  ফেরত std::equal(U.pVal, U.pVal + get_অঙ্ক_শব্দগুলি(), RHS.U.pVal);
}

পূর্ণসংখ্যা APInt::compare(ধ্রুবক APInt& RHS) ধ্রুবক {
  প্রতিজ্ঞা(BitWidth == RHS.BitWidth && "Bit widths must be same for comparison");
  যদি (LLVM_LIKELY(isSingleWord()))
    ফেরত U.VAL < RHS.U.VAL ? -1 : U.VAL > RHS.U.VAL;

  ফেরত tcCompare(U.pVal, RHS.U.pVal, get_অঙ্ক_শব্দগুলি());
}

পূর্ণসংখ্যা APInt::compareSigned(ধ্রুবক APInt& RHS) ধ্রুবক {
  প্রতিজ্ঞা(BitWidth == RHS.BitWidth && "Bit widths must be same for comparison");
  যদি (LLVM_LIKELY(isSingleWord())) {
    int64_t lhsSext = SignExtend64(U.VAL, BitWidth);
    int64_t rhsSext = SignExtend64(RHS.U.VAL, BitWidth);
    ফেরত lhsSext < rhsSext ? -1 : lhsSext > rhsSext;
  }

  যৌক্তিক lhsNeg = isNegative();
  যৌক্তিক rhsNeg = RHS.isNegative();

  // If the sign bits don't match, then (LHS < RHS) if LHS is negative
  যদি (lhsNeg != rhsNeg)
    ফেরত lhsNeg ? -1 : 1;

  // Otherwise we can just use an unsigned comparison, because even negative
  // numbers compare correctly this way if both have the same signed-ness.
  ফেরত tcCompare(U.pVal, RHS.U.pVal, get_অঙ্ক_শব্দগুলি());
}

শূন্য APInt::setBitsSlowCase(unsigned loBit, unsigned hiBit) {
  unsigned loWord = whichWord(loBit);
  unsigned hiWord = whichWord(hiBit);

  // Create an initial mask for the low word with zeros below loBit.
  uint64_t loMask = WORDTYPE_MAX << whichBit(loBit);

  // If hiBit is not aligned, we need a high mask.
  unsigned hiShiftAmt = whichBit(hiBit);
  যদি (hiShiftAmt != 0) {
    // Create a high mask with zeros above hiBit.
    uint64_t hiMask = WORDTYPE_MAX >> (APINT_BITS_PER_WORD - hiShiftAmt);
    // If loWord and hiWord are equal, then we combine the masks. Otherwise,
    // set the bits in hiWord.
    যদি (hiWord == loWord)
      loMask &= hiMask;
    অন্যথা
      U.pVal[hiWord] |= hiMask;
  }
  // Apply the mask to the low word.
  U.pVal[loWord] |= loMask;

  // Fill any words between loWord and hiWord with all ones.
  জন্য (unsigned word = loWord + 1; word < hiWord; ++word)
    U.pVal[word] = WORDTYPE_MAX;
}

শূন্য APInt::clearBitsSlowCase(unsigned LoBit, unsigned HiBit) {
  unsigned LoWord = whichWord(LoBit);
  unsigned HiWord = whichWord(HiBit);

  // Create an initial mask for the low word with ones below loBit.
  uint64_t LoMask = ~(WORDTYPE_MAX << whichBit(LoBit));

  // If HiBit is not aligned, we need a high mask.
  unsigned HiShiftAmt = whichBit(HiBit);
  যদি (HiShiftAmt != 0) {
    // Create a high mask with ones above HiBit.
    uint64_t HiMask = ~(WORDTYPE_MAX >> (APINT_BITS_PER_WORD - HiShiftAmt));
    // If LoWord and HiWord are equal, then we combine the masks. Otherwise,
    // clear the bits in HiWord.
    যদি (HiWord == LoWord)
      LoMask |= HiMask;
    অন্যথা
      U.pVal[HiWord] &= HiMask;
  }
  // Apply the mask to the low word.
  U.pVal[LoWord] &= LoMask;

  // Fill any words between LoWord and HiWord with all zeros.
  জন্য (unsigned Word = LoWord + 1; Word < HiWord; ++Word)
    U.pVal[Word] = 0;
}

// Complement a bignum in-place.
স্থির শূন্য tcComplement(APInt::WordType *dst, unsigned parts) {
  জন্য (unsigned i = 0; i < parts; i++)
    dst[i] = ~dst[i];
}

/// Toggle every bit to its opposite value.
শূন্য APInt::flipAllBitsSlowCase() {
  tcComplement(U.pVal, get_অঙ্ক_শব্দগুলি());
  clearUnusedBits();
}

/// Concatenate the bits from "NewLSB" onto the bottom of *this.  This is
/// equivalent to:
///   (this->zext(NewWidth) << NewLSB.getBitWidth()) | NewLSB.zext(NewWidth)
/// In the slow case, we know the result is large.
APInt APInt::concatSlowCase(ধ্রুবক APInt &NewLSB) ধ্রুবক {
  unsigned NewWidth = getBitWidth() + NewLSB.getBitWidth();
  APInt ফলাফল = NewLSB.zext(NewWidth);
  ফলাফল.ঢোকাও_Bits(*এই, NewLSB.getBitWidth());
  ফেরত ফলাফল;
}

/// Toggle a given bit to its opposite value whose position is given
/// as "bitPosition".
/// Toggles a given bit to its opposite value.
শূন্য APInt::flipBit(unsigned bitPosition) {
  প্রতিজ্ঞা(bitPosition < BitWidth && "Out of the bit-width range!");
  setBitVal(bitPosition, !(*এই)[bitPosition]);
}

শূন্য APInt::ঢোকাও_Bits(ধ্রুবক APInt &subBits, unsigned bitPosition) {
  unsigned subBitWidth = subBits.getBitWidth();
  প্রতিজ্ঞা((subBitWidth + bitPosition) <= BitWidth && "Illegal bit insertion");

  // inserting no bits is a noop.
  যদি (subBitWidth == 0)
    ফেরত;

  // Insertion is a direct copy.
  যদি (subBitWidth == BitWidth) {
    *এই = subBits;
    ফেরত;
  }

  // Single word result can be done as a direct bitmask.
  যদি (LLVM_LIKELY(isSingleWord())) {
    uint64_t mask = WORDTYPE_MAX >> (APINT_BITS_PER_WORD - subBitWidth);
    U.VAL &= ~(mask << bitPosition);
    U.VAL |= (subBits.U.VAL << bitPosition);
    ফেরত;
  }

  unsigned loBit = whichBit(bitPosition);
  unsigned loWord = whichWord(bitPosition);
  unsigned hi1Word = whichWord(bitPosition + subBitWidth - 1);

  // Insertion within a single word can be done as a direct bitmask.
  যদি (loWord == hi1Word) {
    uint64_t mask = WORDTYPE_MAX >> (APINT_BITS_PER_WORD - subBitWidth);
    U.pVal[loWord] &= ~(mask << loBit);
    U.pVal[loWord] |= (subBits.U.VAL << loBit);
    ফেরত;
  }

  // Insert on word boundaries.
  যদি (loBit == 0) {
    // Direct copy whole words.
    unsigned অঙ্ক_Whole_Sub_শব্দগুলি = subBitWidth / APINT_BITS_PER_WORD;
    memcpy(U.pVal + loWord, subBits.get_Raw_তথ্য(),
           অঙ্ক_Whole_Sub_শব্দগুলি * APINT_WORD_আকার);

    // Mask+insert remaining bits.
    unsigned remainingBits = subBitWidth % APINT_BITS_PER_WORD;
    যদি (remainingBits != 0) {
      uint64_t mask = WORDTYPE_MAX >> (APINT_BITS_PER_WORD - remainingBits);
      U.pVal[hi1Word] &= ~mask;
      U.pVal[hi1Word] |= subBits.getWord(subBitWidth - 1);
    }
    ফেরত;
  }

  // General case - set/clear individual bits in dst based on src.
  // TODO - there is scope for optimization here, but at the moment this code
  // path is barely used so prefer readability over performance.
  জন্য (unsigned i = 0; i != subBitWidth; ++i)
    setBitVal(bitPosition + i, subBits[i]);
}

শূন্য APInt::ঢোকাও_Bits(uint64_t subBits, unsigned bitPosition, unsigned অঙ্ক_Bits) {
  uint64_t maskBits = maskTrailingOnes<uint64_t>(অঙ্ক_Bits);
  subBits &= maskBits;
  যদি (LLVM_LIKELY(isSingleWord())) {
    U.VAL &= ~(maskBits << bitPosition);
    U.VAL |= subBits << bitPosition;
    ফেরত;
  }

  unsigned loBit = whichBit(bitPosition);
  unsigned loWord = whichWord(bitPosition);
  unsigned hiWord = whichWord(bitPosition + অঙ্ক_Bits - 1);
  যদি (loWord == hiWord) {
    U.pVal[loWord] &= ~(maskBits << loBit);
    U.pVal[loWord] |= subBits << loBit;
    ফেরত;
  }

  static_assert(8 * আকার(WordType) <= 64, "This code assumes only two words affected");
  unsigned wordBits = 8 * আকার(WordType);
  U.pVal[loWord] &= ~(maskBits << loBit);
  U.pVal[loWord] |= subBits << loBit;

  U.pVal[hiWord] &= ~(maskBits >> (wordBits - loBit));
  U.pVal[hiWord] |= subBits >> (wordBits - loBit);
}

APInt APInt::extractBits(unsigned অঙ্ক_Bits, unsigned bitPosition) ধ্রুবক {
  প্রতিজ্ঞা(bitPosition < BitWidth && (অঙ্ক_Bits + bitPosition) <= BitWidth &&
         "Illegal bit extraction");

  যদি (LLVM_LIKELY(isSingleWord()))
    ফেরত APInt(অঙ্ক_Bits, U.VAL >> bitPosition, /*isSigned=*/মিথ্যা,
                 /*implicitTrunc=*/সত্য);

  unsigned loBit = whichBit(bitPosition);
  unsigned loWord = whichWord(bitPosition);
  unsigned hiWord = whichWord(bitPosition + অঙ্ক_Bits - 1);

  // Single word result extracting bits from a single word source.
  যদি (loWord == hiWord)
    ফেরত APInt(অঙ্ক_Bits, U.pVal[loWord] >> loBit, /*isSigned=*/মিথ্যা,
                 /*implicitTrunc=*/সত্য);

  // Extracting bits that start on a source word boundary can be done
  // as a fast memory copy.
  যদি (loBit == 0)
    ফেরত APInt(অঙ্ক_Bits, ArrayRef(U.pVal + loWord, 1 + hiWord - loWord));

  // General case - shift + copy source words directly into place.
  APInt ফলাফল(অঙ্ক_Bits, 0);
  unsigned অঙ্ক_Src_শব্দগুলি = get_অঙ্ক_শব্দগুলি();
  unsigned অঙ্ক_Dst_শব্দগুলি = ফলাফল.get_অঙ্ক_শব্দগুলি();

  uint64_t *DestPtr =
      LLVM_LIKELY(ফলাফল.isSingleWord()) ? &ফলাফল.U.VAL : ফলাফল.U.pVal;
  জন্য (unsigned word = 0; word < অঙ্ক_Dst_শব্দগুলি; ++word) {
    uint64_t w0 = U.pVal[loWord + word];
    uint64_t w1 =
        (loWord + word + 1) < অঙ্ক_Src_শব্দগুলি ? U.pVal[loWord + word + 1] : 0;
    DestPtr[word] = (w0 >> loBit) | (w1 << (APINT_BITS_PER_WORD - loBit));
  }

  ফেরত ফলাফল.clearUnusedBits();
}

uint64_t APInt::extract_Bits_As_ZExt_মান(unsigned অঙ্ক_Bits,
                                       unsigned bitPosition) ধ্রুবক {
  প্রতিজ্ঞা(bitPosition < BitWidth && (অঙ্ক_Bits + bitPosition) <= BitWidth &&
         "Illegal bit extraction");
  প্রতিজ্ঞা(অঙ্ক_Bits <= 64 && "Illegal bit extraction");

  uint64_t maskBits = maskTrailingOnes<uint64_t>(অঙ্ক_Bits);
  যদি (LLVM_LIKELY(isSingleWord()))
    ফেরত (U.VAL >> bitPosition) & maskBits;

  static_assert(APINT_BITS_PER_WORD >= 64,
                "This code assumes only two words affected");
  unsigned loBit = whichBit(bitPosition);
  unsigned loWord = whichWord(bitPosition);
  unsigned hiWord = whichWord(bitPosition + অঙ্ক_Bits - 1);
  যদি (loWord == hiWord)
    ফেরত (U.pVal[loWord] >> loBit) & maskBits;

  uint64_t retBits = U.pVal[loWord] >> loBit;
  retBits |= U.pVal[hiWord] << (APINT_BITS_PER_WORD - loBit);
  retBits &= maskBits;
  ফেরত retBits;
}

unsigned APInt::getSufficientBitsNeeded(StringRef Str, uint8_t Radix) {
  প্রতিজ্ঞা(!Str.empty() && "Invalid string length");
  size_t StrLen = Str.আকার_3();

  // Each computation below needs to know if it's negative.
  unsigned IsNegative = মিথ্যা;
  যদি (Str[0] == '-' || Str[0] == '+') {
    IsNegative = Str[0] == '-';
    StrLen--;
    প্রতিজ্ঞা(StrLen && "String is only a sign, needs a value.");
  }

  // For radixes of power-of-two values, the bits required is accurately and
  // easily computed.
  যদি (Radix == 2)
    ফেরত StrLen + IsNegative;
  যদি (Radix == 8)
    ফেরত StrLen * 3 + IsNegative;
  যদি (Radix == 16)
    ফেরত StrLen * 4 + IsNegative;

  // Compute a sufficient number of bits that is always large enough but might
  // be too large. This avoids the assertion in the constructor. This
  // calculation doesn't work appropriately for the numbers 0-9, so just use 4
  // bits in that case.
  যদি (Radix == 10)
    ফেরত (StrLen == 1 ? 4 : StrLen * 64 / 18) + IsNegative;

  প্রতিজ্ঞা(Radix == 36);
  ফেরত (StrLen == 1 ? 7 : StrLen * 16 / 3) + IsNegative;
}

unsigned APInt::getBitsNeeded(StringRef str, uint8_t radix) {
  // Compute a sufficient number of bits that is always large enough but might
  // be too large.
  unsigned sufficient = getSufficientBitsNeeded(str, radix);

  // For bases 2, 8, and 16, the sufficient number of bits is exact and we can
  // return the value directly. For bases 10 and 36, we need to do extra work.
  যদি (radix == 2 || radix == 8 || radix == 16)
    ফেরত sufficient;

  // This is grossly inefficient but accurate. We could probably do something
  // with a computation of roughly slen*64/20 and then adjust by the value of
  // the first few digits. But, I'm not sure how accurate that could be.
  size_t slen = str.আকার_3();

  // Each computation below needs to know if it's negative.
  StringRef::iterator p = str.begin();
  unsigned isNegative = *p == '-';
  যদি (*p == '-' || *p == '+') {
    p++;
    slen--;
    প্রতিজ্ঞা(slen && "String is only a sign, needs a value.");
  }


  // Convert to the actual binary value.
  APInt tmp(sufficient, StringRef(p, slen), radix);

  // Compute how many bits are required. If the log is infinite, assume we need
  // just bit. If the log is exact and value is negative, then the value is
  // MinSignedValue with (log + 1) bits.
  unsigned log = tmp.logBase2();
  যদি (log == (unsigned)-1) {
    ফেরত isNegative + 1;
  } অন্যথা যদি (isNegative && tmp.isPowerOf2()) {
    ফেরত isNegative + log;
  } অন্যথা {
    ফেরত isNegative + log + 1;
  }
}

hash_code llvm::hash_মান(ধ্রুবক APInt &Arg) {
  যদি (LLVM_LIKELY(Arg.isSingleWord()))
    ফেরত hash_combine(Arg.BitWidth, Arg.U.VAL);

  ফেরত hash_combine(
      Arg.BitWidth,
      hash_combine_range(Arg.U.pVal, Arg.U.pVal + Arg.get_অঙ্ক_শব্দগুলি()));
}

unsigned DenseMapInfo<APInt, শূন্য>::get_Hash_মান(ধ্রুবক APInt &চাবি) {
  ফেরত static_cast<unsigned>(hash_মান(চাবি));
}

যৌক্তিক APInt::isSplat(unsigned Splat_আকার_In_Bits) ধ্রুবক {
  প্রতিজ্ঞা(getBitWidth() % Splat_আকার_In_Bits == 0 &&
         "SplatSizeInBits must divide width!");
  // We can check that all parts of an integer are equal by making use of a
  // little trick: rotate and check if it's still the same value.
  ফেরত *এই == rotl(Splat_আকার_In_Bits);
}

/// This function returns the high "numBits" bits of this APInt.
APInt APInt::getHiBits(unsigned অঙ্ক_Bits) ধ্রুবক {
  ফেরত এই->lshr(BitWidth - অঙ্ক_Bits);
}

/// This function returns the low "numBits" bits of this APInt.
APInt APInt::getLoBits(unsigned অঙ্ক_Bits) ধ্রুবক {
  APInt ফলাফল(get_নিচের_Bits_Set(BitWidth, অঙ্ক_Bits));
  ফলাফল &= *এই;
  ফেরত ফলাফল;
}

/// Return a value containing V broadcasted over NewLen bits.
APInt APInt::getSplat(unsigned NewLen, ধ্রুবক APInt &V) {
  প্রতিজ্ঞা(NewLen >= V.getBitWidth() && "Can't splat to smaller bit width!");

  APInt Val = V.zext(NewLen);
  জন্য (unsigned I = V.getBitWidth(); I < NewLen; I <<= 1)
    Val |= Val << I;

  ফেরত Val;
}

unsigned APInt::গুনতি_Leading_Zeros_Slow_Case() ধ্রুবক {
  unsigned গুনতি_2 = 0;
  জন্য (পূর্ণসংখ্যা i = get_অঙ্ক_শব্দগুলি() - 1; i >= 0; --i) {
    uint64_t V = U.pVal[i];
    যদি (V == 0)
      গুনতি_2 += APINT_BITS_PER_WORD;
    অন্যথা {
      গুনতি_2 += llvm::countl_zero(V);
      বিরতি;
    }
  }
  // Adjust for unused bits in the most significant word (they are zero).
  unsigned Mod = BitWidth % APINT_BITS_PER_WORD;
  গুনতি_2 -= Mod > 0 ? APINT_BITS_PER_WORD - Mod : 0;
  ফেরত গুনতি_2;
}

unsigned APInt::গুনতি_Leading_Ones_Slow_Case() ধ্রুবক {
  unsigned উপরের_Word_Bits = BitWidth % APINT_BITS_PER_WORD;
  unsigned shift;
  যদি (!উপরের_Word_Bits) {
    উপরের_Word_Bits = APINT_BITS_PER_WORD;
    shift = 0;
  } অন্যথা {
    shift = APINT_BITS_PER_WORD - উপরের_Word_Bits;
  }
  পূর্ণসংখ্যা i = get_অঙ্ক_শব্দগুলি() - 1;
  unsigned গুনতি_2 = llvm::countl_one(U.pVal[i] << shift);
  যদি (গুনতি_2 == উপরের_Word_Bits) {
    জন্য (i--; i >= 0; --i) {
      যদি (U.pVal[i] == WORDTYPE_MAX)
        গুনতি_2 += APINT_BITS_PER_WORD;
      অন্যথা {
        গুনতি_2 += llvm::countl_one(U.pVal[i]);
        বিরতি;
      }
    }
  }
  ফেরত গুনতি_2;
}

unsigned APInt::গুনতি_Trailing_Zeros_Slow_Case() ধ্রুবক {
  unsigned গুনতি_2 = 0;
  unsigned i = 0;
  জন্য (; i < get_অঙ্ক_শব্দগুলি() && U.pVal[i] == 0; ++i)
    গুনতি_2 += APINT_BITS_PER_WORD;
  যদি (i < get_অঙ্ক_শব্দগুলি())
    গুনতি_2 += llvm::countr_zero(U.pVal[i]);
  ফেরত std::min(গুনতি_2, BitWidth);
}

unsigned APInt::গুনতি_Trailing_Ones_Slow_Case() ধ্রুবক {
  unsigned গুনতি_2 = 0;
  unsigned i = 0;
  জন্য (; i < get_অঙ্ক_শব্দগুলি() && U.pVal[i] == WORDTYPE_MAX; ++i)
    গুনতি_2 += APINT_BITS_PER_WORD;
  যদি (i < get_অঙ্ক_শব্দগুলি())
    গুনতি_2 += llvm::countr_one(U.pVal[i]);
  প্রতিজ্ঞা(গুনতি_2 <= BitWidth);
  ফেরত গুনতি_2;
}

unsigned APInt::গুনতি_Population_Slow_Case() ধ্রুবক {
  unsigned গুনতি_2 = 0;
  জন্য (unsigned i = 0; i < get_অঙ্ক_শব্দগুলি(); ++i)
    গুনতি_2 += llvm::popcount(U.pVal[i]);
  ফেরত গুনতি_2;
}

যৌক্তিক APInt::isPowerOf2SlowCase() ধ্রুবক {
  unsigned গুনতি_2 = 0;
  জন্য (unsigned i = 0; i < get_অঙ্ক_শব্দগুলি(); ++i) {
    গুনতি_2 += llvm::popcount(U.pVal[i]);
    যদি (গুনতি_2 > 1)
      ফেরত মিথ্যা;
  }
  ফেরত গুনতি_2 == 1;
}

যৌক্তিক APInt::intersectsSlowCase(ধ্রুবক APInt &RHS) ধ্রুবক {
  জন্য (unsigned i = 0, e = get_অঙ্ক_শব্দগুলি(); i != e; ++i)
    যদি ((U.pVal[i] & RHS.U.pVal[i]) != 0)
      ফেরত সত্য;

  ফেরত মিথ্যা;
}

যৌক্তিক APInt::isSubsetOfSlowCase(ধ্রুবক APInt &RHS) ধ্রুবক {
  জন্য (unsigned i = 0, e = get_অঙ্ক_শব্দগুলি(); i != e; ++i)
    যদি ((U.pVal[i] & ~RHS.U.pVal[i]) != 0)
      ফেরত মিথ্যা;

  ফেরত সত্য;
}

যৌক্তিক APInt::isInverseOfSlowCase(ধ্রুবক APInt &RHS) ধ্রুবক {
  ধ্রুবক unsigned Last = get_অঙ্ক_শব্দগুলি() - 1;
  জন্য (unsigned I = 0; I != Last; ++I)
    যদি ((U.pVal[I] ^ RHS.U.pVal[I]) != WORDTYPE_MAX)
      ফেরত মিথ্যা;

  unsigned লেজ_Bits = BitWidth - Last * APINT_BITS_PER_WORD;
  WordType লেজ_Mask = llvm::maskTrailingOnes<WordType>(লেজ_Bits);
  ফেরত (U.pVal[Last] ^ RHS.U.pVal[Last]) == লেজ_Mask;
}

APInt APInt::byteSwap() ধ্রুবক {
  প্রতিজ্ঞা(BitWidth >= 16 && BitWidth % 8 == 0 && "Cannot byteswap!");
  যদি (BitWidth == 16)
    ফেরত APInt(BitWidth, llvm::byteswap<uint16_t>(U.VAL));
  যদি (BitWidth == 32)
    ফেরত APInt(BitWidth, llvm::byteswap<uint32_t>(U.VAL));
  যদি (BitWidth <= 64) {
    uint64_t Tmp1 = llvm::byteswap<uint64_t>(U.VAL);
    Tmp1 >>= (64 - BitWidth);
    ফেরত APInt(BitWidth, Tmp1);
  }

  APInt ফলাফল(get_অঙ্ক_শব্দগুলি() * APINT_BITS_PER_WORD, 0);
  জন্য (unsigned I = 0, N = get_অঙ্ক_শব্দগুলি(); I != N; ++I)
    ফলাফল.U.pVal[I] = llvm::byteswap<uint64_t>(U.pVal[N - I - 1]);
  যদি (ফলাফল.BitWidth != BitWidth) {
    ফলাফল.lshrInPlace(ফলাফল.BitWidth - BitWidth);
    ফলাফল.BitWidth = BitWidth;
  }
  ফেরত ফলাফল;
}

APInt APInt::reverseBits() ধ্রুবক {
  যদি (LLVM_LIKELY(isSingleWord())) {
    নির্বাচন (BitWidth) {
    ক্ষেত্র 64:
      ফেরত APInt(BitWidth, llvm::reverseBits<uint64_t>(U.VAL));
    ক্ষেত্র 32:
      ফেরত APInt(BitWidth, llvm::reverseBits<uint32_t>(U.VAL));
    ক্ষেত্র 16:
      ফেরত APInt(BitWidth, llvm::reverseBits<uint16_t>(U.VAL));
    ক্ষেত্র 8:
      ফেরত APInt(BitWidth, llvm::reverseBits<uint8_t>(U.VAL));
    ক্ষেত্র 1: // fallthrough
    ক্ষেত্র 0:
      ফেরত *এই;
    পূর্বনির্ধারিত:
      ফেরত APInt(BitWidth,
                   llvm::reverseBits<uint64_t>(U.VAL) >> (64 - BitWidth));
    }
  }

  APInt ফলাফল(BitWidth, 0);
  unsigned অঙ্ক_শব্দগুলি_2 = get_অঙ্ক_শব্দগুলি();
  unsigned ExcessBits = অঙ্ক_শব্দগুলি_2 * APINT_BITS_PER_WORD - BitWidth;
  যদি (ExcessBits == 0) {
    // Fast path. No cross-word shift needed.
    জন্য (unsigned I = 0; I < অঙ্ক_শব্দগুলি_2; ++I)
      ফলাফল.U.pVal[I] = llvm::reverseBits<uint64_t>(U.pVal[অঙ্ক_শব্দগুলি_2 - 1 - I]);
    ফেরত ফলাফল;
  }
  // Holds reversed bits of the previous (more significant) word.
  uint64_t পূর্ব_Rev = llvm::reverseBits<uint64_t>(U.pVal[অঙ্ক_শব্দগুলি_2 - 1]);
  জন্য (unsigned I = 0; I < অঙ্ক_শব্দগুলি_2 - 1; ++I) {
    uint64_t CurrRev = llvm::reverseBits<uint64_t>(U.pVal[অঙ্ক_শব্দগুলি_2 - 2 - I]);
    ফলাফল.U.pVal[I] = (পূর্ব_Rev >> ExcessBits) | (CurrRev << (64 - ExcessBits));
    পূর্ব_Rev = CurrRev;
  }
  ফলাফল.U.pVal[অঙ্ক_শব্দগুলি_2 - 1] = পূর্ব_Rev >> ExcessBits;
  ফেরত ফলাফল;
}

APInt llvm::APIntOps::Greatest_Common_ভাজক(APInt A, APInt B, যৌক্তিক IsSigned) {
  // Take absolute value if IsSigned.
  যদি (IsSigned) {
    A = A.abs();
    B = B.abs();
  }

  // Fast-path a common case.
  যদি (A == B) ফেরত A;

  // Corner cases: if either operand is zero, the other is the gcd.
  যদি (!A) ফেরত B;
  যদি (!B) ফেরত A;

  // Count common powers of 2 and remove all other powers of 2.
  unsigned Pow2;
  {
    unsigned Pow2_A = A.countr_zero();
    unsigned Pow2_B = B.countr_zero();
    যদি (Pow2_A > Pow2_B) {
      A.lshrInPlace(Pow2_A - Pow2_B);
      Pow2 = Pow2_B;
    } অন্যথা যদি (Pow2_B > Pow2_A) {
      B.lshrInPlace(Pow2_B - Pow2_A);
      Pow2 = Pow2_A;
    } অন্যথা {
      Pow2 = Pow2_A;
    }
  }

  // Both operands are odd multiples of 2^Pow_2:
  //
  //   gcd(a, b) = gcd(|a - b| / 2^i, min(a, b))
  //
  // This is a modified version of Stein's algorithm, taking advantage of
  // efficient countTrailingZeros().
  যখন (A != B) {
    যদি (A.ugt(B)) {
      A -= B;
      A.lshrInPlace(A.countr_zero() - Pow2);
    } অন্যথা {
      B -= A;
      B.lshrInPlace(B.countr_zero() - Pow2);
    }
  }

  ফেরত A;
}

APInt llvm::APIntOps::RoundDoubleToAPInt(দ্বিগুণ Double, unsigned width) {
  uint64_t I = bit_cast<uint64_t>(Double);

  // Get the sign bit from the highest order bit
  যৌক্তিক isNeg = I >> 63;

  // Get the 11-bit exponent and adjust for the 1023 bit bias
  int64_t exp = ((I >> 52) & 0x7ff) - 1023;

  // If the exponent is negative, the value is < 0 so just return 0.
  যদি (exp < 0)
    ফেরত APInt(width, 0u);

  // Extract the mantissa by clearing the top 12 bits (sign + exponent).
  uint64_t mantissa = (I & (~0ULL >> 12)) | 1ULL << 52;

  // If the exponent doesn't shift all bits out of the mantissa
  যদি (exp < 52)
    ফেরত isNeg ? -APInt(width, mantissa >> (52 - exp)) :
                    APInt(width, mantissa >> (52 - exp));

  // If the client didn't provide enough bits for us to shift the mantissa into
  // then the result is undefined, just return 0
  যদি (width <= exp - 52)
    ফেরত APInt(width, 0);

  // Otherwise, we have to shift the mantissa bits up to the right location
  APInt Tmp(width, mantissa);
  Tmp <<= (unsigned)exp - 52;
  ফেরত isNeg ? -Tmp : Tmp;
}

/// This function converts this APInt to a double.
/// The layout for double is as following (IEEE Standard 754):
///  --------------------------------------
/// |  Sign    Exponent    Fraction    Bias |
/// |-------------------------------------- |
/// |  1[63]   11[62-52]   52[51-00]   1023 |
///  --------------------------------------
দ্বিগুণ APInt::roundToDouble(যৌক্তিক isSigned) ধ্রুবক {
  // Handle the simple case where the value is contained in one uint64_t.
  // It is wrong to optimize getWord(0) to VAL; there might be more than one word.
  যদি (LLVM_LIKELY(isSingleWord() || getActiveBits() <= APINT_BITS_PER_WORD)) {
    যদি (isSigned) {
      int64_t sext = SignExtend64(getWord(0), BitWidth);
      ফেরত দ্বিগুণ(sext);
    }
    ফেরত দ্বিগুণ(getWord(0));
  }

  // Determine if the value is negative.
  যৌক্তিক isNeg = isSigned ? (*এই)[BitWidth-1] : মিথ্যা;

  // Construct the absolute value if we're negative.
  APInt Tmp(isNeg ? -(*এই) : (*এই));

  // Figure out how many bits we're using.
  unsigned n = Tmp.getActiveBits();

  // The exponent (without bias normalization) is just the number of bits
  // we are using. Note that the sign bit is gone since we constructed the
  // absolute value.
  uint64_t exp = n;

  // Return infinity for exponent overflow
  যদি (exp > 1023) {
    যদি (!isSigned || !isNeg)
      ফেরত std::numeric_limits<দ্বিগুণ>::infinity();
    অন্যথা
      ফেরত -std::numeric_limits<দ্বিগুণ>::infinity();
  }
  exp += 1023; // Increment for 1023 bias

  // Number of bits in mantissa is 52. To obtain the mantissa value, we must
  // extract the high 52 bits from the correct words in pVal.
  uint64_t mantissa;
  unsigned hiWord = whichWord(n-1);
  যদি (hiWord == 0) {
    mantissa = Tmp.U.pVal[0];
    যদি (n > 52)
      mantissa >>= n - 52; // shift down, we want the top 52 bits.
  } অন্যথা {
    প্রতিজ্ঞা(hiWord > 0 && "huh?");
    uint64_t hibits = Tmp.U.pVal[hiWord] << (52 - n % APINT_BITS_PER_WORD);
    uint64_t lobits = Tmp.U.pVal[hiWord-1] >> (11 + n % APINT_BITS_PER_WORD);
    mantissa = hibits | lobits;
  }

  // The leading bit of mantissa is implicit, so get rid of it.
  uint64_t sign = isNeg ? (1ULL << (APINT_BITS_PER_WORD - 1)) : 0;
  uint64_t I = sign | (exp << 52) | mantissa;
  ফেরত bit_cast<দ্বিগুণ>(I);
}

// Truncate to new width.
APInt APInt::trunc(unsigned width) ধ্রুবক {
  প্রতিজ্ঞা(width <= BitWidth && "Invalid APInt Truncate request");

  যদি (width <= APINT_BITS_PER_WORD)
    ফেরত APInt(width, get_Raw_তথ্য()[0], /*isSigned=*/মিথ্যা,
                 /*implicitTrunc=*/সত্য);

  যদি (width == BitWidth)
    ফেরত *এই;

  APInt ফলাফল(getMemory(get_অঙ্ক_শব্দগুলি(width)), width);

  // Copy full words.
  unsigned i;
  জন্য (i = 0; i != width / APINT_BITS_PER_WORD; i++)
    ফলাফল.U.pVal[i] = U.pVal[i];

  // Truncate and copy any partial word.
  unsigned bits = (0 - width) % APINT_BITS_PER_WORD;
  যদি (bits != 0)
    ফলাফল.U.pVal[i] = U.pVal[i] << bits >> bits;

  ফেরত ফলাফল;
}

// Truncate to new width with unsigned saturation.
APInt APInt::truncUSat(unsigned width) ধ্রুবক {
  প্রতিজ্ঞা(width <= BitWidth && "Invalid APInt Truncate request");

  // Can we just losslessly truncate it?
  যদি (isIntN(width))
    ফেরত trunc(width);
  // If not, then just return the new limit.
  ফেরত APInt::get_Max_মান(width);
}

// Truncate to new width with signed saturation to signed result.
APInt APInt::truncSSat(unsigned width) ধ্রুবক {
  প্রতিজ্ঞা(width <= BitWidth && "Invalid APInt Truncate request");

  // Can we just losslessly truncate it?
  যদি (isSignedIntN(width))
    ফেরত trunc(width);
  // If not, then just return the new limits.
  ফেরত isNegative() ? APInt::get_Signed_Min_মান(width)
                      : APInt::get_Signed_Max_মান(width);
}

// Truncate to new width with signed saturation to unsigned result.
APInt APInt::truncSSatU(unsigned width) ধ্রুবক {
  প্রতিজ্ঞা(width <= BitWidth && "Invalid APInt Truncate request");

  // Can we just losslessly truncate it?
  যদি (isIntN(width))
    ফেরত trunc(width);
  // If not, then just return the new limits.
  ফেরত isNegative() ? APInt::getZero(width) : APInt::get_Max_মান(width);
}

// Sign extend to a new width.
APInt APInt::sext(unsigned Width) ধ্রুবক {
  প্রতিজ্ঞা(Width >= BitWidth && "Invalid APInt SignExtend request");

  যদি (Width <= APINT_BITS_PER_WORD)
    ফেরত APInt(Width, SignExtend64(U.VAL, BitWidth), /*isSigned=*/সত্য);

  যদি (Width == BitWidth)
    ফেরত *এই;

  APInt ফলাফল(getMemory(get_অঙ্ক_শব্দগুলি(Width)), Width);

  // Copy words.
  std::memcpy(ফলাফল.U.pVal, get_Raw_তথ্য(), get_অঙ্ক_শব্দগুলি() * APINT_WORD_আকার);

  // Sign extend the last word since there may be unused bits in the input.
  ফলাফল.U.pVal[get_অঙ্ক_শব্দগুলি() - 1] =
      SignExtend64(ফলাফল.U.pVal[get_অঙ্ক_শব্দগুলি() - 1],
                   ((BitWidth - 1) % APINT_BITS_PER_WORD) + 1);

  // Fill with sign bits.
  std::memset(ফলাফল.U.pVal + get_অঙ্ক_শব্দগুলি(), isNegative() ? -1 : 0,
              (ফলাফল.get_অঙ্ক_শব্দগুলি() - get_অঙ্ক_শব্দগুলি()) * APINT_WORD_আকার);
  ফলাফল.clearUnusedBits();
  ফেরত ফলাফল;
}

//  Zero extend to a new width.
APInt APInt::zext(unsigned width) ধ্রুবক {
  প্রতিজ্ঞা(width >= BitWidth && "Invalid APInt ZeroExtend request");

  যদি (width <= APINT_BITS_PER_WORD)
    ফেরত APInt(width, U.VAL);

  যদি (width == BitWidth)
    ফেরত *এই;

  APInt ফলাফল(getMemory(get_অঙ্ক_শব্দগুলি(width)), width);

  // Copy words.
  std::memcpy(ফলাফল.U.pVal, get_Raw_তথ্য(), get_অঙ্ক_শব্দগুলি() * APINT_WORD_আকার);

  // Zero remaining words.
  std::memset(ফলাফল.U.pVal + get_অঙ্ক_শব্দগুলি(), 0,
              (ফলাফল.get_অঙ্ক_শব্দগুলি() - get_অঙ্ক_শব্দগুলি()) * APINT_WORD_আকার);

  ফেরত ফলাফল;
}

APInt APInt::zextOrTrunc(unsigned width) ধ্রুবক {
  যদি (BitWidth < width)
    ফেরত zext(width);
  যদি (BitWidth > width)
    ফেরত trunc(width);
  ফেরত *এই;
}

APInt APInt::sextOrTrunc(unsigned width) ধ্রুবক {
  যদি (BitWidth < width)
    ফেরত sext(width);
  যদি (BitWidth > width)
    ফেরত trunc(width);
  ফেরত *এই;
}

/// Arithmetic right-shift this APInt by shiftAmt.
/// Arithmetic right-shift function.
শূন্য APInt::ashrInPlace(ধ্রুবক APInt &shiftAmt) {
  ashrInPlace((unsigned)shiftAmt.get_Limited_মান(BitWidth));
}

/// Arithmetic right-shift this APInt by shiftAmt.
/// Arithmetic right-shift function.
শূন্য APInt::ashrSlowCase(unsigned ShiftAmt) {
  // Don't bother performing a no-op shift.
  যদি (!ShiftAmt)
    ফেরত;

  // Save the original sign bit for later.
  যৌক্তিক Negative = isNegative();

  // WordShift is the inter-part shift; BitShift is intra-part shift.
  unsigned WordShift = ShiftAmt / APINT_BITS_PER_WORD;
  unsigned BitShift = ShiftAmt % APINT_BITS_PER_WORD;

  unsigned শব্দগুলি_To_Move = get_অঙ্ক_শব্দগুলি() - WordShift;
  যদি (শব্দগুলি_To_Move != 0) {
    // Sign extend the last word to fill in the unused bits.
    U.pVal[get_অঙ্ক_শব্দগুলি() - 1] = SignExtend64(
        U.pVal[get_অঙ্ক_শব্দগুলি() - 1], ((BitWidth - 1) % APINT_BITS_PER_WORD) + 1);

    // Fastpath for moving by whole words.
    যদি (BitShift == 0) {
      std::memmove(U.pVal, U.pVal + WordShift, শব্দগুলি_To_Move * APINT_WORD_আকার);
    } অন্যথা {
      // Move the words containing significant bits.
      জন্য (unsigned i = 0; i != শব্দগুলি_To_Move - 1; ++i)
        U.pVal[i] = (U.pVal[i + WordShift] >> BitShift) |
                    (U.pVal[i + WordShift + 1] << (APINT_BITS_PER_WORD - BitShift));

      // Handle the last word which has no high bits to copy. Use an arithmetic
      // shift to preserve the sign bit.
      U.pVal[শব্দগুলি_To_Move - 1] =
          (int64_t)U.pVal[WordShift + শব্দগুলি_To_Move - 1] >> BitShift;
    }
  }

  // Fill in the remainder based on the original sign.
  std::memset(U.pVal + শব্দগুলি_To_Move, Negative ? -1 : 0,
              WordShift * APINT_WORD_আকার);
  clearUnusedBits();
}

/// Logical right-shift this APInt by shiftAmt.
/// Logical right-shift function.
শূন্য APInt::lshrInPlace(ধ্রুবক APInt &shiftAmt) {
  lshrInPlace((unsigned)shiftAmt.get_Limited_মান(BitWidth));
}

/// Logical right-shift this APInt by shiftAmt.
/// Logical right-shift function.
শূন্য APInt::lshrSlowCase(unsigned ShiftAmt) {
  tc_Shift_ডান(U.pVal, get_অঙ্ক_শব্দগুলি(), ShiftAmt);
}

/// Left-shift this APInt by shiftAmt.
/// Left-shift function.
APInt &APInt::operator<<=(ধ্রুবক APInt &shiftAmt) {
  // It's undefined behavior in C to shift by BitWidth or greater.
  *এই <<= (unsigned)shiftAmt.get_Limited_মান(BitWidth);
  ফেরত *এই;
}

শূন্য APInt::shlSlowCase(unsigned ShiftAmt) {
  tc_Shift_বাম(U.pVal, get_অঙ্ক_শব্দগুলি(), ShiftAmt);
  clearUnusedBits();
}

// Calculate the rotate amount modulo the bit width.
স্থির unsigned rotateModulo(unsigned BitWidth, ধ্রুবক APInt &rotateAmt) {
  যদি (LLVM_UNLIKELY(BitWidth == 0))
    ফেরত 0;
  unsigned rotBitWidth = rotateAmt.getBitWidth();
  APInt rot = rotateAmt;
  যদি (rotBitWidth < BitWidth) {
    // Extend the rotate APInt, so that the urem doesn't divide by 0.
    // e.g. APInt(1, 32) would give APInt(1, 0).
    rot = rotateAmt.zext(BitWidth);
  }
  rot = rot.urem(APInt(rot.getBitWidth(), BitWidth));
  ফেরত rot.get_Limited_মান(BitWidth);
}

APInt APInt::rotl(ধ্রুবক APInt &rotateAmt) ধ্রুবক {
  ফেরত rotl(rotateModulo(BitWidth, rotateAmt));
}

APInt APInt::rotl(unsigned rotateAmt) ধ্রুবক {
  যদি (LLVM_UNLIKELY(BitWidth == 0))
    ফেরত *এই;
  rotateAmt %= BitWidth;
  যদি (rotateAmt == 0)
    ফেরত *এই;
  ফেরত shl(rotateAmt) | lshr(BitWidth - rotateAmt);
}

APInt APInt::rotr(ধ্রুবক APInt &rotateAmt) ধ্রুবক {
  ফেরত rotr(rotateModulo(BitWidth, rotateAmt));
}

APInt APInt::rotr(unsigned rotateAmt) ধ্রুবক {
  যদি (BitWidth == 0)
    ফেরত *এই;
  rotateAmt %= BitWidth;
  যদি (rotateAmt == 0)
    ফেরত *এই;
  ফেরত lshr(rotateAmt) | shl(BitWidth - rotateAmt);
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
unsigned APInt::nearestLogBase2() ধ্রুবক {
  // Special case when we have a bitwidth of 1. If VAL is 1, then we
  // get 0. If VAL is 0, we get WORDTYPE_MAX which gets truncated to
  // UINT32_MAX.
  যদি (BitWidth == 1)
    ফেরত U.VAL - 1;

  // Handle the zero case.
  যদি (isZero())
    ফেরত UINT32_MAX;

  // The non-zero case is handled by computing:
  //
  //   nearestLogBase2(x) = logBase2(x) + x[logBase2(x)-1].
  //
  // where x[i] is referring to the value of the ith bit of x.
  unsigned lg = logBase2();
  ফেরত lg + unsigned((*এই)[lg - 1]);
}

// Square Root - this method computes and returns the square root of "this".
// Three mechanisms are used for computation. For small values (<= 5 bits),
// a table lookup is done. This gets some performance for common cases. For
// values using less than 52 bits, the value is converted to double and then
// the libc sqrt function is called. The result is rounded and then converted
// back to a uint64_t which is then used to construct the result. Finally,
// the Babylonian method for computing square roots is used.
APInt APInt::sqrtFloor() ধ্রুবক {

  // Determine the magnitude of the value.
  unsigned magnitude = getActiveBits();

  // Use a fast table for some small values. This also gets rid of some
  // rounding errors in libc sqrt for small values.
  যদি (magnitude <= 5) {
    স্থির ধ্রুবক uint8_t results[32] = {
        /*     0 */ 0,
        /*  1- 3 */ 1, 1, 1,
        /*  4- 8 */ 2, 2, 2, 2, 2,
        /*  9-15 */ 3, 3, 3, 3, 3, 3, 3,
        /* 16-24 */ 4, 4, 4, 4, 4, 4, 4, 4, 4,
        /* 25-31 */ 5, 5, 5, 5, 5, 5, 5,
    };
    ফেরত APInt(BitWidth,
                 results[(LLVM_LIKELY(isSingleWord()) ? U.VAL : U.pVal[0])]);
  }

  // If the magnitude of the value fits in less than 52 bits (the precision of
  // an IEEE double precision floating point value), then we can use the
  // libc sqrt function which will probably use a hardware sqrt computation.
  // This should be faster than the algorithm below.
  যদি (magnitude < 52) {
    ফেরত APInt(BitWidth,
                 uint64_t(::floor(::sqrt(দ্বিগুণ(
                     LLVM_LIKELY(isSingleWord()) ? U.VAL : U.pVal[0])))));
  }

  // Okay, all the short cuts are exhausted. We must compute it. The following
  // is a classical Babylonian method for computing the square root. This code
  // was adapted to APInt from a wikipedia article on such computations.
  // See http://www.wikipedia.org/ and go to the page named
  // Calculate_an_integer_square_root.
  unsigned nbits = BitWidth, i = 4;
  APInt testy(BitWidth, 16);
  APInt x_old(BitWidth, 1);
  APInt x_new(BitWidth, 0);
  APInt two(BitWidth, 2);

  // Select a good starting value using binary logarithms.
  জন্য (;; i += 2, testy = testy.shl(2))
    যদি (i >= nbits || এই->ule(testy)) {
      x_old = x_old.shl(i / 2);
      বিরতি;
    }

  // Use the Babylonian method to arrive at the integer square root:
  জন্য (;;) {
    x_new = (এই->udiv(x_old) + x_old).udiv(two);
    যদি (x_old.ule(x_new))
      বিরতি;
    x_old = x_new;
  }
  ফেরত x_old;
}

/// \returns the multiplicative inverse of an odd APInt modulo 2^BitWidth.
APInt APInt::multiplicativeInverse() ধ্রুবক {
  প্রতিজ্ঞা((*এই)[0] &&
         "multiplicative inverse is only defined for odd numbers!");

  // Use Newton's method.
  APInt Factor = *এই;
  APInt T;
  যখন (!(T = *এই * Factor).isOne())
    Factor *= 2 - std::move(T);
  ফেরত Factor;
}

/// Implementation of Knuth's Algorithm D (Division of nonnegative integers)
/// from "Art of Computer Programming, Volume 2", section 4.3.1, p. 272. The
/// variables here have the same names as in the algorithm. Comments explain
/// the algorithm and any deviation from it.
স্থির শূন্য KnuthDiv(uint32_t *u, uint32_t *v, uint32_t *q, uint32_t* r,
                     unsigned m, unsigned n) {
  প্রতিজ্ঞা(u && "Must provide dividend");
  প্রতিজ্ঞা(v && "Must provide divisor");
  প্রতিজ্ঞা(q && "Must provide quotient");
  প্রতিজ্ঞা(u != v && u != q && v != q && "Must use different memory");
  প্রতিজ্ঞা(n>1 && "n must be > 1");

  // b denotes the base of the number system. In our case b is 2^32.
  ধ্রুবক uint64_t b = uint64_t(1) << 32;

// The DEBUG macros here tend to be spam in the debug output if you're not
// debugging this code. Disable them unless KNUTH_DEBUG is defined.
#ifdef KNUTH_DEBUG
#define DEBUG_KNUTH(X) LLVM_DEBUG(X)
#অন্যথা
#define DEBUG_KNUTH(X) করো {} যখন(মিথ্যা)
#endif

  DEBUG_KNUTH(dbgs() << "KnuthDiv: m=" << m << " n=" << n << '\n');
  DEBUG_KNUTH(dbgs() << "KnuthDiv: original:");
  DEBUG_KNUTH(জন্য (পূর্ণসংখ্যা i = m + n; i >= 0; i--) dbgs() << " " << u[i]);
  DEBUG_KNUTH(dbgs() << " by");
  DEBUG_KNUTH(জন্য (পূর্ণসংখ্যা i = n; i > 0; i--) dbgs() << " " << v[i - 1]);
  DEBUG_KNUTH(dbgs() << '\n');
  // D1. [Normalize.] Set d = b / (v[n-1] + 1) and multiply all the digits of
  // u and v by d. Note that we have taken Knuth's advice here to use a power
  // of 2 value for d such that d * v[n-1] >= b/2 (b is the base). A power of
  // 2 allows us to shift instead of multiply and it is easy to determine the
  // shift amount from the leading zeros.  We are basically normalizing the u
  // and v so that its high bits are shifted to the top of v's range without
  // overflow. Note that this can require an extra word in u so that u must
  // be of length m+n+1.
  unsigned shift = llvm::countl_zero(v[n - 1]);
  uint32_t v_carry = 0;
  uint32_t u_carry = 0;
  যদি (shift) {
    জন্য (unsigned i = 0; i < m+n; ++i) {
      uint32_t u_tmp = u[i] >> (32 - shift);
      u[i] = (u[i] << shift) | u_carry;
      u_carry = u_tmp;
    }
    জন্য (unsigned i = 0; i < n; ++i) {
      uint32_t v_tmp = v[i] >> (32 - shift);
      v[i] = (v[i] << shift) | v_carry;
      v_carry = v_tmp;
    }
  }
  u[m+n] = u_carry;

  DEBUG_KNUTH(dbgs() << "KnuthDiv:   normal:");
  DEBUG_KNUTH(জন্য (পূর্ণসংখ্যা i = m + n; i >= 0; i--) dbgs() << " " << u[i]);
  DEBUG_KNUTH(dbgs() << " by");
  DEBUG_KNUTH(জন্য (পূর্ণসংখ্যা i = n; i > 0; i--) dbgs() << " " << v[i - 1]);
  DEBUG_KNUTH(dbgs() << '\n');

  // D2. [Initialize j.]  Set j to m. This is the loop counter over the places.
  পূর্ণসংখ্যা j = m;
  করো {
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
    যদি (qp == b || qp*v[n-2] > b*rp + u[j+n-2]) {
      qp--;
      rp += v[n-1];
      যদি (rp < b && (qp == b || qp*v[n-2] > b*rp + u[j+n-2]))
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
    জন্য (unsigned i = 0; i < n; ++i) {
      uint64_t p = qp * uint64_t(v[i]);
      int64_t subres = int64_t(u[j+i]) - borrow - Lo_32(p);
      u[j+i] = Lo_32(subres);
      borrow = Hi_32(p) - Hi_32(subres);
      DEBUG_KNUTH(dbgs() << "KnuthDiv: u[j+i] = " << u[j + i]
                        << ", borrow = " << borrow << '\n');
    }
    যৌক্তিক isNeg = u[j+n] < borrow;
    u[j+n] -= Lo_32(borrow);

    DEBUG_KNUTH(dbgs() << "KnuthDiv: after subtraction:");
    DEBUG_KNUTH(জন্য (পূর্ণসংখ্যা i = m + n; i >= 0; i--) dbgs() << " " << u[i]);
    DEBUG_KNUTH(dbgs() << '\n');

    // D5. [Test remainder.] Set q[j] = qp. If the result of step D4 was
    // negative, go to step D6; otherwise go on to step D7.
    q[j] = Lo_32(qp);
    যদি (isNeg) {
      // D6. [Add back]. The probability that this step is necessary is very
      // small, on the order of only 2/b. Make sure that test data accounts for
      // this possibility. Decrease q[j] by 1
      q[j]--;
      // and add (0v[n-1]...v[1]v[0]) to (u[j+n]u[j+n-1]...u[j+1]u[j]).
      // A carry will occur to the left of u[j+n], and it should be ignored
      // since it cancels with the borrow that occurred in D4.
      যৌক্তিক carry = মিথ্যা;
      জন্য (unsigned i = 0; i < n; i++) {
        uint32_t limit = std::min(u[j+i],v[i]);
        u[j+i] += v[i] + carry;
        carry = u[j+i] < limit || (carry && u[j+i] == limit);
      }
      u[j+n] += carry;
    }
    DEBUG_KNUTH(dbgs() << "KnuthDiv: after correction:");
    DEBUG_KNUTH(জন্য (পূর্ণসংখ্যা i = m + n; i >= 0; i--) dbgs() << " " << u[i]);
    DEBUG_KNUTH(dbgs() << "\nKnuthDiv: digit result = " << q[j] << '\n');

    // D7. [Loop on j.]  Decrease j by one. Now if j >= 0, go back to D3.
  } যখন (--j >= 0);

  DEBUG_KNUTH(dbgs() << "KnuthDiv: quotient:");
  DEBUG_KNUTH(জন্য (পূর্ণসংখ্যা i = m; i >= 0; i--) dbgs() << " " << q[i]);
  DEBUG_KNUTH(dbgs() << '\n');

  // D8. [Unnormalize]. Now q[...] is the desired quotient, and the desired
  // remainder may be obtained by dividing u[...] by d. If r is non-null we
  // compute the remainder (urem uses this).
  যদি (r) {
    // The value d is expressed by the "shift" value above since we avoided
    // multiplication by d by using a shift left. So, all we have to do is
    // shift right here.
    যদি (shift) {
      uint32_t carry = 0;
      DEBUG_KNUTH(dbgs() << "KnuthDiv: remainder:");
      জন্য (পূর্ণসংখ্যা i = n-1; i >= 0; i--) {
        r[i] = (u[i] >> shift) | carry;
        carry = u[i] << (32 - shift);
        DEBUG_KNUTH(dbgs() << " " << r[i]);
      }
    } অন্যথা {
      জন্য (পূর্ণসংখ্যা i = n-1; i >= 0; i--) {
        r[i] = u[i];
        DEBUG_KNUTH(dbgs() << " " << r[i]);
      }
    }
    DEBUG_KNUTH(dbgs() << '\n');
  }
  DEBUG_KNUTH(dbgs() << '\n');
}

শূন্য APInt::divide(ধ্রুবক WordType *LHS, unsigned lhs_শব্দগুলি, ধ্রুবক WordType *RHS,
                   unsigned rhs_শব্দগুলি, WordType *Quotient, WordType *Remainder) {
  প্রতিজ্ঞা(lhs_শব্দগুলি >= rhs_শব্দগুলি && "Fractional result");

  // First, compose the values into an array of 32-bit words instead of
  // 64-bit words. This is a necessity of both the "short division" algorithm
  // and the Knuth "classical algorithm" which requires there to be native
  // operations for +, -, and * on an m bit value with an m*2 bit result. We
  // can't use 64-bit operands here because we don't have native results of
  // 128-bits. Furthermore, casting the 64-bit values to 32-bit values won't
  // work on large-endian machines.
  unsigned n = rhs_শব্দগুলি * 2;
  unsigned m = (lhs_শব্দগুলি * 2) - n;

  // Allocate space for the temporary values we need either on the stack, if
  // it will fit, or on the heap if it won't.
  uint32_t SPACE[128];
  uint32_t *U = nullptr;
  uint32_t *V = nullptr;
  uint32_t *Q = nullptr;
  uint32_t *R = nullptr;
  যদি ((Remainder?4:3)*n+2*m+1 <= 128) {
    U = &SPACE[0];
    V = &SPACE[m+n+1];
    Q = &SPACE[(m+n+1) + n];
    যদি (Remainder)
      R = &SPACE[(m+n+1) + n + (m+n)];
  } অন্যথা {
    U = নতুন uint32_t[m + n + 1];
    V = নতুন uint32_t[n];
    Q = নতুন uint32_t[m+n];
    যদি (Remainder)
      R = নতুন uint32_t[n];
  }

  // Initialize the dividend
  memset(U, 0, (m+n+1)*আকার(uint32_t));
  জন্য (unsigned i = 0; i < lhs_শব্দগুলি; ++i) {
    uint64_t tmp = LHS[i];
    U[i * 2] = Lo_32(tmp);
    U[i * 2 + 1] = Hi_32(tmp);
  }
  U[m+n] = 0; // this extra word is for "spill" in the Knuth algorithm.

  // Initialize the divisor
  memset(V, 0, (n)*আকার(uint32_t));
  জন্য (unsigned i = 0; i < rhs_শব্দগুলি; ++i) {
    uint64_t tmp = RHS[i];
    V[i * 2] = Lo_32(tmp);
    V[i * 2 + 1] = Hi_32(tmp);
  }

  // initialize the quotient and remainder
  memset(Q, 0, (m+n) * আকার(uint32_t));
  যদি (Remainder)
    memset(R, 0, n * আকার(uint32_t));

  // Now, adjust m and n for the Knuth division. n is the number of words in
  // the divisor. m is the number of words by which the dividend exceeds the
  // divisor (i.e. m+n is the length of the dividend). These sizes must not
  // contain any zero words or the Knuth algorithm fails.
  জন্য (unsigned i = n; i > 0 && V[i-1] == 0; i--) {
    n--;
    m++;
  }
  জন্য (unsigned i = m+n; i > 0 && U[i-1] == 0; i--)
    m--;

  // If we're left with only a single word for the divisor, Knuth doesn't work
  // so we implement the short division algorithm here. This is much simpler
  // and faster because we are certain that we can divide a 64-bit quantity
  // by a 32-bit quantity at hardware speed and short division is simply a
  // series of such operations. This is just like doing short division but we
  // are using base 2^32 instead of base 10.
  প্রতিজ্ঞা(n != 0 && "Divide by zero?");
  যদি (n == 1) {
    uint32_t ভাজক = V[0];
    uint32_t remainder = 0;
    জন্য (পূর্ণসংখ্যা i = m; i >= 0; i--) {
      uint64_t partial_dividend = Make_64(remainder, U[i]);
      যদি (partial_dividend == 0) {
        Q[i] = 0;
        remainder = 0;
      } অন্যথা যদি (partial_dividend < ভাজক) {
        Q[i] = 0;
        remainder = Lo_32(partial_dividend);
      } অন্যথা যদি (partial_dividend == ভাজক) {
        Q[i] = 1;
        remainder = 0;
      } অন্যথা {
        Q[i] = Lo_32(partial_dividend / ভাজক);
        remainder = Lo_32(partial_dividend - (Q[i] * ভাজক));
      }
    }
    যদি (R)
      R[0] = remainder;
  } অন্যথা {
    // Now we're ready to invoke the Knuth classical divide algorithm. In this
    // case n > 1.
    KnuthDiv(U, V, Q, R, m, n);
  }

  // If the caller wants the quotient
  যদি (Quotient) {
    জন্য (unsigned i = 0; i < lhs_শব্দগুলি; ++i)
      Quotient[i] = Make_64(Q[i*2+1], Q[i*2]);
  }

  // If the caller wants the remainder
  যদি (Remainder) {
    জন্য (unsigned i = 0; i < rhs_শব্দগুলি; ++i)
      Remainder[i] = Make_64(R[i*2+1], R[i*2]);
  }

  // Clean up the memory we allocated.
  যদি (U != &SPACE[0]) {
    delete [] U;
    delete [] V;
    delete [] Q;
    delete [] R;
  }
}

APInt APInt::udiv(ধ্রুবক APInt &RHS) ধ্রুবক {
  প্রতিজ্ঞা(BitWidth == RHS.BitWidth && "Bit widths must be the same");

  // First, deal with the easy case
  যদি (LLVM_LIKELY(isSingleWord())) {
    প্রতিজ্ঞা(RHS.U.VAL != 0 && "Divide by zero?");
    ফেরত APInt(BitWidth, U.VAL / RHS.U.VAL);
  }

  // Get some facts about the LHS and RHS number of bits and words
  unsigned lhs_শব্দগুলি = get_অঙ্ক_শব্দগুলি(getActiveBits());
  unsigned rhsBits  = RHS.getActiveBits();
  unsigned rhs_শব্দগুলি = get_অঙ্ক_শব্দগুলি(rhsBits);
  প্রতিজ্ঞা(rhs_শব্দগুলি && "Divided by zero???");

  // Deal with some degenerate cases
  যদি (!lhs_শব্দগুলি)
    // 0 / X ===> 0
    ফেরত APInt(BitWidth, 0);
  যদি (rhsBits == 1)
    // X / 1 ===> X
    ফেরত *এই;
  যদি (lhs_শব্দগুলি < rhs_শব্দগুলি || এই->ult(RHS))
    // X / Y ===> 0, iff X < Y
    ফেরত APInt(BitWidth, 0);
  যদি (*এই == RHS)
    // X / X ===> 1
    ফেরত APInt(BitWidth, 1);
  যদি (lhs_শব্দগুলি == 1) // rhsWords is 1 if lhsWords is 1.
    // All high words are zero, just use native divide
    ফেরত APInt(BitWidth, এই->U.pVal[0] / RHS.U.pVal[0]);

  // We have to compute it the hard way. Invoke the Knuth divide algorithm.
  APInt Quotient(BitWidth, 0); // to hold result.
  divide(U.pVal, lhs_শব্দগুলি, RHS.U.pVal, rhs_শব্দগুলি, Quotient.U.pVal, nullptr);
  ফেরত Quotient;
}

APInt APInt::udiv(uint64_t RHS) ধ্রুবক {
  প্রতিজ্ঞা(RHS != 0 && "Divide by zero?");

  // First, deal with the easy case
  যদি (LLVM_LIKELY(isSingleWord()))
    ফেরত APInt(BitWidth, U.VAL / RHS);

  // Get some facts about the LHS words.
  unsigned lhs_শব্দগুলি = get_অঙ্ক_শব্দগুলি(getActiveBits());

  // Deal with some degenerate cases
  যদি (!lhs_শব্দগুলি)
    // 0 / X ===> 0
    ফেরত APInt(BitWidth, 0);
  যদি (RHS == 1)
    // X / 1 ===> X
    ফেরত *এই;
  যদি (এই->ult(RHS))
    // X / Y ===> 0, iff X < Y
    ফেরত APInt(BitWidth, 0);
  যদি (*এই == RHS)
    // X / X ===> 1
    ফেরত APInt(BitWidth, 1);
  যদি (lhs_শব্দগুলি == 1) // rhsWords is 1 if lhsWords is 1.
    // All high words are zero, just use native divide
    ফেরত APInt(BitWidth, এই->U.pVal[0] / RHS);

  // We have to compute it the hard way. Invoke the Knuth divide algorithm.
  APInt Quotient(BitWidth, 0); // to hold result.
  divide(U.pVal, lhs_শব্দগুলি, &RHS, 1, Quotient.U.pVal, nullptr);
  ফেরত Quotient;
}

APInt APInt::sdiv(ধ্রুবক APInt &RHS) ধ্রুবক {
  যদি (isNegative()) {
    যদি (RHS.isNegative())
      ফেরত (-(*এই)).udiv(-RHS);
    ফেরত -((-(*এই)).udiv(RHS));
  }
  যদি (RHS.isNegative())
    ফেরত -(এই->udiv(-RHS));
  ফেরত এই->udiv(RHS);
}

APInt APInt::sdiv(int64_t RHS) ধ্রুবক {
  যদি (isNegative()) {
    যদি (RHS < 0)
      ফেরত (-(*এই)).udiv(-RHS);
    ফেরত -((-(*এই)).udiv(RHS));
  }
  যদি (RHS < 0)
    ফেরত -(এই->udiv(-RHS));
  ফেরত এই->udiv(RHS);
}

APInt APInt::urem(ধ্রুবক APInt &RHS) ধ্রুবক {
  প্রতিজ্ঞা(BitWidth == RHS.BitWidth && "Bit widths must be the same");
  যদি (LLVM_LIKELY(isSingleWord())) {
    প্রতিজ্ঞা(RHS.U.VAL != 0 && "Remainder by zero?");
    ফেরত APInt(BitWidth, U.VAL % RHS.U.VAL);
  }

  // Get some facts about the LHS
  unsigned lhs_শব্দগুলি = get_অঙ্ক_শব্দগুলি(getActiveBits());

  // Get some facts about the RHS
  unsigned rhsBits = RHS.getActiveBits();
  unsigned rhs_শব্দগুলি = get_অঙ্ক_শব্দগুলি(rhsBits);
  প্রতিজ্ঞা(rhs_শব্দগুলি && "Performing remainder operation by zero ???");

  // Check the degenerate cases
  যদি (lhs_শব্দগুলি == 0)
    // 0 % Y ===> 0
    ফেরত APInt(BitWidth, 0);
  যদি (rhsBits == 1)
    // X % 1 ===> 0
    ফেরত APInt(BitWidth, 0);
  যদি (lhs_শব্দগুলি < rhs_শব্দগুলি || এই->ult(RHS))
    // X % Y ===> X, iff X < Y
    ফেরত *এই;
  যদি (*এই == RHS)
    // X % X == 0;
    ফেরত APInt(BitWidth, 0);
  যদি (lhs_শব্দগুলি == 1)
    // All high words are zero, just use native remainder
    ফেরত APInt(BitWidth, U.pVal[0] % RHS.U.pVal[0]);
  যদি (RHS.isPowerOf2()) {
    // X % 2^w ===> X & (2^w - 1)
    APInt ফলাফল(*এই);
    ফলাফল.clearBits(RHS.logBase2(), BitWidth);
    ফেরত ফলাফল;
  }

  // We have to compute it the hard way. Invoke the Knuth divide algorithm.
  APInt Remainder(BitWidth, 0);
  divide(U.pVal, lhs_শব্দগুলি, RHS.U.pVal, rhs_শব্দগুলি, nullptr, Remainder.U.pVal);
  ফেরত Remainder;
}

uint64_t APInt::urem(uint64_t RHS) ধ্রুবক {
  প্রতিজ্ঞা(RHS != 0 && "Remainder by zero?");

  যদি (LLVM_LIKELY(isSingleWord()))
    ফেরত U.VAL % RHS;

  // Get some facts about the LHS
  unsigned lhs_শব্দগুলি = get_অঙ্ক_শব্দগুলি(getActiveBits());

  // Check the degenerate cases
  যদি (lhs_শব্দগুলি == 0)
    // 0 % Y ===> 0
    ফেরত 0;
  যদি (RHS == 1)
    // X % 1 ===> 0
    ফেরত 0;
  যদি (এই->ult(RHS))
    // X % Y ===> X, iff X < Y
    ফেরত get_ZExt_মান();
  যদি (*এই == RHS)
    // X % X == 0;
    ফেরত 0;
  যদি (lhs_শব্দগুলি == 1)
    // All high words are zero, just use native remainder
    ফেরত U.pVal[0] % RHS;
  যদি (llvm::isPowerOf2_64(RHS))
    // X % 2^w ===> X & (2^w - 1)
    ফেরত U.pVal[0] & (RHS - 1);

  // We have to compute it the hard way. Invoke the Knuth divide algorithm.
  uint64_t Remainder;
  divide(U.pVal, lhs_শব্দগুলি, &RHS, 1, nullptr, &Remainder);
  ফেরত Remainder;
}

APInt APInt::srem(ধ্রুবক APInt &RHS) ধ্রুবক {
  যদি (isNegative()) {
    যদি (RHS.isNegative())
      ফেরত -((-(*এই)).urem(-RHS));
    ফেরত -((-(*এই)).urem(RHS));
  }
  যদি (RHS.isNegative())
    ফেরত এই->urem(-RHS);
  ফেরত এই->urem(RHS);
}

int64_t APInt::srem(int64_t RHS) ধ্রুবক {
  যদি (isNegative()) {
    যদি (RHS < 0)
      ফেরত -((-(*এই)).urem(-RHS));
    ফেরত -((-(*এই)).urem(RHS));
  }
  যদি (RHS < 0)
    ফেরত এই->urem(-RHS);
  ফেরত এই->urem(RHS);
}

শূন্য APInt::udivrem(ধ্রুবক APInt &LHS, ধ্রুবক APInt &RHS,
                    APInt &Quotient, APInt &Remainder) {
  প্রতিজ্ঞা(LHS.BitWidth == RHS.BitWidth && "Bit widths must be the same");
  unsigned BitWidth = LHS.BitWidth;

  // First, deal with the easy case
  যদি (LLVM_LIKELY(LHS.isSingleWord())) {
    প্রতিজ্ঞা(RHS.U.VAL != 0 && "Divide by zero?");
    uint64_t QuotVal = LHS.U.VAL / RHS.U.VAL;
    uint64_t RemVal = LHS.U.VAL % RHS.U.VAL;
    Quotient = APInt(BitWidth, QuotVal);
    Remainder = APInt(BitWidth, RemVal);
    ফেরত;
  }

  // Get some size facts about the dividend and divisor
  unsigned lhs_শব্দগুলি = get_অঙ্ক_শব্দগুলি(LHS.getActiveBits());
  unsigned rhsBits  = RHS.getActiveBits();
  unsigned rhs_শব্দগুলি = get_অঙ্ক_শব্দগুলি(rhsBits);
  প্রতিজ্ঞা(rhs_শব্দগুলি && "Performing divrem operation by zero ???");

  // Check the degenerate cases
  যদি (lhs_শব্দগুলি == 0) {
    Quotient = APInt(BitWidth, 0);    // 0 / Y ===> 0
    Remainder = APInt(BitWidth, 0);   // 0 % Y ===> 0
    ফেরত;
  }

  যদি (rhsBits == 1) {
    Quotient = LHS;                   // X / 1 ===> X
    Remainder = APInt(BitWidth, 0);   // X % 1 ===> 0
  }

  যদি (lhs_শব্দগুলি < rhs_শব্দগুলি || LHS.ult(RHS)) {
    Remainder = LHS;                  // X % Y ===> X, iff X < Y
    Quotient = APInt(BitWidth, 0);    // X / Y ===> 0, iff X < Y
    ফেরত;
  }

  যদি (LHS == RHS) {
    Quotient  = APInt(BitWidth, 1);   // X / X ===> 1
    Remainder = APInt(BitWidth, 0);   // X % X ===> 0;
    ফেরত;
  }

  // Make sure there is enough space to hold the results.
  // NOTE: This assumes that reallocate won't affect any bits if it doesn't
  // change the size. This is necessary if Quotient or Remainder is aliased
  // with LHS or RHS.
  Quotient.reallocate(BitWidth);
  Remainder.reallocate(BitWidth);

  যদি (lhs_শব্দগুলি == 1) { // rhsWords is 1 if lhsWords is 1.
    // There is only one word to consider so use the native versions.
    uint64_t lhs_মান = LHS.U.pVal[0];
    uint64_t rhs_মান = RHS.U.pVal[0];
    Quotient = lhs_মান / rhs_মান;
    Remainder = lhs_মান % rhs_মান;
    ফেরত;
  }

  // Okay, lets do it the long way
  divide(LHS.U.pVal, lhs_শব্দগুলি, RHS.U.pVal, rhs_শব্দগুলি, Quotient.U.pVal,
         Remainder.U.pVal);
  // Clear the rest of the Quotient and Remainder.
  std::memset(Quotient.U.pVal + lhs_শব্দগুলি, 0,
              (get_অঙ্ক_শব্দগুলি(BitWidth) - lhs_শব্দগুলি) * APINT_WORD_আকার);
  std::memset(Remainder.U.pVal + rhs_শব্দগুলি, 0,
              (get_অঙ্ক_শব্দগুলি(BitWidth) - rhs_শব্দগুলি) * APINT_WORD_আকার);
}

শূন্য APInt::udivrem(ধ্রুবক APInt &LHS, uint64_t RHS, APInt &Quotient,
                    uint64_t &Remainder) {
  প্রতিজ্ঞা(RHS != 0 && "Divide by zero?");
  unsigned BitWidth = LHS.BitWidth;

  // First, deal with the easy case
  যদি (LLVM_LIKELY(LHS.isSingleWord())) {
    uint64_t QuotVal = LHS.U.VAL / RHS;
    Remainder = LHS.U.VAL % RHS;
    Quotient = APInt(BitWidth, QuotVal);
    ফেরত;
  }

  // Get some size facts about the dividend and divisor
  unsigned lhs_শব্দগুলি = get_অঙ্ক_শব্দগুলি(LHS.getActiveBits());

  // Check the degenerate cases
  যদি (lhs_শব্দগুলি == 0) {
    Quotient = APInt(BitWidth, 0);    // 0 / Y ===> 0
    Remainder = 0;                    // 0 % Y ===> 0
    ফেরত;
  }

  যদি (RHS == 1) {
    Quotient = LHS;                   // X / 1 ===> X
    Remainder = 0;                    // X % 1 ===> 0
    ফেরত;
  }

  যদি (LHS.ult(RHS)) {
    Remainder = LHS.get_ZExt_মান();   // X % Y ===> X, iff X < Y
    Quotient = APInt(BitWidth, 0);    // X / Y ===> 0, iff X < Y
    ফেরত;
  }

  যদি (LHS == RHS) {
    Quotient  = APInt(BitWidth, 1);   // X / X ===> 1
    Remainder = 0;                    // X % X ===> 0;
    ফেরত;
  }

  // Make sure there is enough space to hold the results.
  // NOTE: This assumes that reallocate won't affect any bits if it doesn't
  // change the size. This is necessary if Quotient is aliased with LHS.
  Quotient.reallocate(BitWidth);

  যদি (lhs_শব্দগুলি == 1) { // rhsWords is 1 if lhsWords is 1.
    // There is only one word to consider so use the native versions.
    uint64_t lhs_মান = LHS.U.pVal[0];
    Quotient = lhs_মান / RHS;
    Remainder = lhs_মান % RHS;
    ফেরত;
  }

  // Okay, lets do it the long way
  divide(LHS.U.pVal, lhs_শব্দগুলি, &RHS, 1, Quotient.U.pVal, &Remainder);
  // Clear the rest of the Quotient.
  std::memset(Quotient.U.pVal + lhs_শব্দগুলি, 0,
              (get_অঙ্ক_শব্দগুলি(BitWidth) - lhs_শব্দগুলি) * APINT_WORD_আকার);
}

শূন্য APInt::sdivrem(ধ্রুবক APInt &LHS, ধ্রুবক APInt &RHS,
                    APInt &Quotient, APInt &Remainder) {
  যদি (LHS.isNegative()) {
    যদি (RHS.isNegative())
      APInt::udivrem(-LHS, -RHS, Quotient, Remainder);
    অন্যথা {
      APInt::udivrem(-LHS, RHS, Quotient, Remainder);
      Quotient.negate();
    }
    Remainder.negate();
  } অন্যথা যদি (RHS.isNegative()) {
    APInt::udivrem(LHS, -RHS, Quotient, Remainder);
    Quotient.negate();
  } অন্যথা {
    APInt::udivrem(LHS, RHS, Quotient, Remainder);
  }
}

শূন্য APInt::sdivrem(ধ্রুবক APInt &LHS, int64_t RHS,
                    APInt &Quotient, int64_t &Remainder) {
  uint64_t R = Remainder;
  যদি (LHS.isNegative()) {
    যদি (RHS < 0)
      APInt::udivrem(-LHS, -RHS, Quotient, R);
    অন্যথা {
      APInt::udivrem(-LHS, RHS, Quotient, R);
      Quotient.negate();
    }
    R = -R;
  } অন্যথা যদি (RHS < 0) {
    APInt::udivrem(LHS, -RHS, Quotient, R);
    Quotient.negate();
  } অন্যথা {
    APInt::udivrem(LHS, RHS, Quotient, R);
  }
  Remainder = R;
}

APInt APInt::sadd_ov(ধ্রুবক APInt &RHS, যৌক্তিক &Overflow) ধ্রুবক {
  APInt Res = *এই+RHS;
  Overflow = isNonNegative() == RHS.isNonNegative() &&
             Res.isNonNegative() != isNonNegative();
  ফেরত Res;
}

APInt APInt::uadd_ov(ধ্রুবক APInt &RHS, যৌক্তিক &Overflow) ধ্রুবক {
  APInt Res = *এই+RHS;
  Overflow = Res.ult(RHS);
  ফেরত Res;
}

APInt APInt::ssub_ov(ধ্রুবক APInt &RHS, যৌক্তিক &Overflow) ধ্রুবক {
  APInt Res = *এই - RHS;
  Overflow = isNonNegative() != RHS.isNonNegative() &&
             Res.isNonNegative() != isNonNegative();
  ফেরত Res;
}

APInt APInt::usub_ov(ধ্রুবক APInt &RHS, যৌক্তিক &Overflow) ধ্রুবক {
  APInt Res = *এই-RHS;
  Overflow = Res.ugt(*এই);
  ফেরত Res;
}

APInt APInt::sdiv_ov(ধ্রুবক APInt &RHS, যৌক্তিক &Overflow) ধ্রুবক {
  // MININT/-1  -->  overflow.
  Overflow = is_Min_Signed_মান() && RHS.isAllOnes();
  ফেরত sdiv(RHS);
}

APInt APInt::smul_ov(ধ্রুবক APInt &RHS, যৌক্তিক &Overflow) ধ্রুবক {
  APInt Res = *এই * RHS;

  যদি (RHS != 0)
    Overflow = Res.sdiv(RHS) != *এই ||
               (is_Min_Signed_মান() && RHS.isAllOnes());
  অন্যথা
    Overflow = মিথ্যা;
  ফেরত Res;
}

APInt APInt::umul_ov(ধ্রুবক APInt &RHS, যৌক্তিক &Overflow) ধ্রুবক {
  যদি (countl_zero() + RHS.countl_zero() + 2 <= BitWidth) {
    Overflow = সত্য;
    ফেরত *এই * RHS;
  }

  APInt Res = lshr(1) * RHS;
  Overflow = Res.isNegative();
  Res <<= 1;
  যদি ((*এই)[0]) {
    Res += RHS;
    যদি (Res.ult(RHS))
      Overflow = সত্য;
  }
  ফেরত Res;
}

APInt APInt::sshl_ov(ধ্রুবক APInt &ShAmt, যৌক্তিক &Overflow) ধ্রুবক {
  ফেরত sshl_ov(ShAmt.get_Limited_মান(getBitWidth()), Overflow);
}

APInt APInt::sshl_ov(unsigned ShAmt, যৌক্তিক &Overflow) ধ্রুবক {
  Overflow = ShAmt >= getBitWidth();
  যদি (Overflow)
    ফেরত APInt(BitWidth, 0);

  যদি (isNonNegative()) // Don't allow sign change.
    Overflow = ShAmt >= countl_zero();
  অন্যথা
    Overflow = ShAmt >= countl_one();

  ফেরত *এই << ShAmt;
}

APInt APInt::ushl_ov(ধ্রুবক APInt &ShAmt, যৌক্তিক &Overflow) ধ্রুবক {
  ফেরত ushl_ov(ShAmt.get_Limited_মান(getBitWidth()), Overflow);
}

APInt APInt::ushl_ov(unsigned ShAmt, যৌক্তিক &Overflow) ধ্রুবক {
  Overflow = ShAmt >= getBitWidth();
  যদি (Overflow)
    ফেরত APInt(BitWidth, 0);

  Overflow = ShAmt > countl_zero();

  ফেরত *এই << ShAmt;
}

APInt APInt::sfloordiv_ov(ধ্রুবক APInt &RHS, যৌক্তিক &Overflow) ধ্রুবক {
  APInt quotient = sdiv_ov(RHS, Overflow);
  যদি ((quotient * RHS != *এই) && (isNegative() != RHS.isNegative()))
    ফেরত quotient - 1;
  ফেরত quotient;
}

APInt APInt::sadd_sat(ধ্রুবক APInt &RHS) ধ্রুবক {
  যৌক্তিক Overflow;
  APInt Res = sadd_ov(RHS, Overflow);
  যদি (!Overflow)
    ফেরত Res;

  ফেরত isNegative() ? APInt::get_Signed_Min_মান(BitWidth)
                      : APInt::get_Signed_Max_মান(BitWidth);
}

APInt APInt::uadd_sat(ধ্রুবক APInt &RHS) ধ্রুবক {
  যৌক্তিক Overflow;
  APInt Res = uadd_ov(RHS, Overflow);
  যদি (!Overflow)
    ফেরত Res;

  ফেরত APInt::get_Max_মান(BitWidth);
}

APInt APInt::ssub_sat(ধ্রুবক APInt &RHS) ধ্রুবক {
  যৌক্তিক Overflow;
  APInt Res = ssub_ov(RHS, Overflow);
  যদি (!Overflow)
    ফেরত Res;

  ফেরত isNegative() ? APInt::get_Signed_Min_মান(BitWidth)
                      : APInt::get_Signed_Max_মান(BitWidth);
}

APInt APInt::usub_sat(ধ্রুবক APInt &RHS) ধ্রুবক {
  যৌক্তিক Overflow;
  APInt Res = usub_ov(RHS, Overflow);
  যদি (!Overflow)
    ফেরত Res;

  ফেরত APInt(BitWidth, 0);
}

APInt APInt::smul_sat(ধ্রুবক APInt &RHS) ধ্রুবক {
  যৌক্তিক Overflow;
  APInt Res = smul_ov(RHS, Overflow);
  যদি (!Overflow)
    ফেরত Res;

  // The result is negative if one and only one of inputs is negative.
  যৌক্তিক ResIsNegative = isNegative() ^ RHS.isNegative();

  ফেরত ResIsNegative ? APInt::get_Signed_Min_মান(BitWidth)
                       : APInt::get_Signed_Max_মান(BitWidth);
}

APInt APInt::umul_sat(ধ্রুবক APInt &RHS) ধ্রুবক {
  যৌক্তিক Overflow;
  APInt Res = umul_ov(RHS, Overflow);
  যদি (!Overflow)
    ফেরত Res;

  ফেরত APInt::get_Max_মান(BitWidth);
}

APInt APInt::sshl_sat(ধ্রুবক APInt &RHS) ধ্রুবক {
  ফেরত sshl_sat(RHS.get_Limited_মান(getBitWidth()));
}

APInt APInt::sshl_sat(unsigned RHS) ধ্রুবক {
  যৌক্তিক Overflow;
  APInt Res = sshl_ov(RHS, Overflow);
  যদি (!Overflow)
    ফেরত Res;

  ফেরত isNegative() ? APInt::get_Signed_Min_মান(BitWidth)
                      : APInt::get_Signed_Max_মান(BitWidth);
}

APInt APInt::ushl_sat(ধ্রুবক APInt &RHS) ধ্রুবক {
  ফেরত ushl_sat(RHS.get_Limited_মান(getBitWidth()));
}

APInt APInt::ushl_sat(unsigned RHS) ধ্রুবক {
  যৌক্তিক Overflow;
  APInt Res = ushl_ov(RHS, Overflow);
  যদি (!Overflow)
    ফেরত Res;

  ফেরত APInt::get_Max_মান(BitWidth);
}

শূন্য APInt::fromString(unsigned numbits, StringRef str, uint8_t radix) {
  // Check our assumptions here
  প্রতিজ্ঞা(!str.empty() && "Invalid string length");
  প্রতিজ্ঞা((radix == 10 || radix == 8 || radix == 16 || radix == 2 ||
          radix == 36) &&
         "Radix should be 2, 8, 10, 16, or 36!");

  StringRef::iterator p = str.begin();
  size_t slen = str.আকার_3();
  যৌক্তিক isNeg = *p == '-';
  যদি (*p == '-' || *p == '+') {
    p++;
    slen--;
    প্রতিজ্ঞা(slen && "String is only a sign, needs a value.");
  }
  প্রতিজ্ঞা((slen <= numbits || radix != 2) && "Insufficient bit width");
  প্রতিজ্ঞা(((slen-1)*3 <= numbits || radix != 8) && "Insufficient bit width");
  প্রতিজ্ঞা(((slen-1)*4 <= numbits || radix != 16) && "Insufficient bit width");
  প্রতিজ্ঞা((((slen-1)*64)/22 <= numbits || radix != 10) &&
         "Insufficient bit width");

  // Allocate memory if needed
  যদি (LLVM_LIKELY(isSingleWord()))
    U.VAL = 0;
  অন্যথা
    U.pVal = getClearedMemory(get_অঙ্ক_শব্দগুলি());

  // Figure out if we can shift instead of multiply
  unsigned shift = (radix == 16 ? 4 : radix == 8 ? 3 : radix == 2 ? 1 : 0);

  // Enter digit traversal loop
  জন্য (StringRef::iterator e = str.end(); p != e; ++p) {
    unsigned digit = getDigit(*p, radix);
    প্রতিজ্ঞা(digit < radix && "Invalid character in digit string");

    // Shift or multiply the value by the radix
    যদি (slen > 1) {
      যদি (shift)
        *এই <<= shift;
      অন্যথা
        *এই *= radix;
    }

    // Add in the digit we just interpreted
    *এই += digit;
  }
  // If its negative, put it in two's complement form
  যদি (isNeg)
    এই->negate();
}

শূন্য APInt::toString(SmallVectorImpl<অক্ষর> &Str, unsigned Radix, যৌক্তিক Signed,
                     যৌক্তিক formatAsCLiteral, যৌক্তিক UpperCase,
                     যৌক্তিক ঢোকাও_Separators) ধ্রুবক {
  প্রতিজ্ঞা((Radix == 10 || Radix == 8 || Radix == 16 || Radix == 2 ||
          Radix == 36) &&
         "Radix should be 2, 8, 10, 16, or 36!");

  ধ্রুবক অক্ষর *Prefix = "";
  যদি (formatAsCLiteral) {
    নির্বাচন (Radix) {
      ক্ষেত্র 2:
        // Binary literals are a non-standard extension added in gcc 4.3:
        // http://gcc.gnu.org/onlinedocs/gcc-4.3.0/gcc/Binary-constants.html
        Prefix = "0b";
        বিরতি;
      ক্ষেত্র 8:
        Prefix = "0";
        বিরতি;
      ক্ষেত্র 10:
        বিরতি; // No prefix
      ক্ষেত্র 16:
        Prefix = "0x";
        বিরতি;
      পূর্বনির্ধারিত:
        llvm_unreachable("Invalid radix!");
    }
  }

  // Number of digits in a group between separators.
  unsigned Grouping = (Radix == 8 || Radix == 10) ? 3 : 4;

  // First, check for a zero value and just short circuit the logic below.
  যদি (isZero()) {
    যখন (*Prefix) {
      Str.ঠেলো_back(*Prefix);
      ++Prefix;
    };
    Str.ঠেলো_back('0');
    ফেরত;
  }

  স্থির ধ্রুবক অক্ষর BothDigits[] = "0123456789abcdefghijklmnopqrstuvwxyz"
                                   "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ";
  ধ্রুবক অক্ষর *Digits = BothDigits + (UpperCase ? 36 : 0);

  যদি (LLVM_LIKELY(isSingleWord())) {
    অক্ষর বাফার[65];
    অক্ষর *BufPtr = std::end(বাফার);

    uint64_t N;
    যদি (!Signed) {
      N = get_ZExt_মান();
    } অন্যথা {
      int64_t I = get_SExt_মান();
      যদি (I >= 0) {
        N = I;
      } অন্যথা {
        Str.ঠেলো_back('-');
        N = -(uint64_t)I;
      }
    }

    যখন (*Prefix) {
      Str.ঠেলো_back(*Prefix);
      ++Prefix;
    };

    পূর্ণসংখ্যা Pos = 0;
    যখন (N) {
      যদি (ঢোকাও_Separators && Pos % Grouping == 0 && Pos > 0)
        *--BufPtr = '\'';
      *--BufPtr = Digits[N % Radix];
      N /= Radix;
      Pos++;
    }
    Str.append(BufPtr, std::end(বাফার));
    ফেরত;
  }

  APInt Tmp(*এই);

  যদি (Signed && isNegative()) {
    // They want to print the signed version and it is a negative value
    // Flip the bits and add one to turn it into the equivalent positive
    // value and put a '-' in the result.
    Tmp.negate();
    Str.ঠেলো_back('-');
  }

  যখন (*Prefix) {
    Str.ঠেলো_back(*Prefix);
    ++Prefix;
  }

  // We insert the digits backward, then reverse them to get the right order.
  unsigned StartDig = Str.আকার_3();

  // For the 2, 8 and 16 bit cases, we can just shift instead of divide
  // because the number of bits per digit (1, 3 and 4 respectively) divides
  // equally.  We just shift until the value is zero.
  যদি (Radix == 2 || Radix == 8 || Radix == 16) {
    // Just shift tmp right for each digit width until it becomes zero
    unsigned ShiftAmt = (Radix == 16 ? 4 : (Radix == 8 ? 3 : 1));
    unsigned MaskAmt = Radix - 1;

    পূর্ণসংখ্যা Pos = 0;
    যখন (Tmp.get_Bool_মান()) {
      unsigned Digit = unsigned(Tmp.get_Raw_তথ্য()[0]) & MaskAmt;
      যদি (ঢোকাও_Separators && Pos % Grouping == 0 && Pos > 0)
        Str.ঠেলো_back('\'');

      Str.ঠেলো_back(Digits[Digit]);
      Tmp.lshrInPlace(ShiftAmt);
      Pos++;
    }
  } অন্যথা {
    পূর্ণসংখ্যা Pos = 0;
    যখন (Tmp.get_Bool_মান()) {
      uint64_t Digit;
      udivrem(Tmp, Radix, Tmp, Digit);
      প্রতিজ্ঞা(Digit < Radix && "divide failed");
      যদি (ঢোকাও_Separators && Pos % Grouping == 0 && Pos > 0)
        Str.ঠেলো_back('\'');

      Str.ঠেলো_back(Digits[Digit]);
      Pos++;
    }
  }

  // Reverse the digits before returning.
  std::reverse(Str.begin()+StartDig, Str.end());
}

#যদি !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)
LLVM_DUMP_METHOD শূন্য APInt::dump() ধ্রুবক {
  SmallString<40> S, U;
  এই->toStringUnsigned(U);
  এই->toStringSigned(S);
  dbgs() << "APInt(" << BitWidth << "b, "
         << U << "u " << S << "s)\n";
}
#endif

শূন্য APInt::print(raw_ostream &OS, যৌক্তিক isSigned) ধ্রুবক {
  SmallString<40> S;
  এই->toString(S, 10, isSigned, /* formatAsCLiteral = */মিথ্যা);
  OS << S;
}

// This implements a variety of operations on a representation of
// arbitrary precision, two's-complement, bignum integer values.

// Assumed by lowHalf, highHalf, partMSB and partLSB.  A fairly safe
// and unrestricting assumption.
static_assert(APInt::APINT_BITS_PER_WORD % 2 == 0,
              "Part width must be divisible by 2!");

// Returns the integer part with the least significant BITS set.
// BITS cannot be zero.
স্থির অন্তঃরেখ APInt::WordType নিচের_Bit_Mask(unsigned bits) {
  প্রতিজ্ঞা(bits != 0 && bits <= APInt::APINT_BITS_PER_WORD);
  ফেরত ~(APInt::WordType) 0 >> (APInt::APINT_BITS_PER_WORD - bits);
}

/// Returns the value of the lower half of PART.
স্থির অন্তঃরেখ APInt::WordType নিচের_Half(APInt::WordType part) {
  ফেরত part & নিচের_Bit_Mask(APInt::APINT_BITS_PER_WORD / 2);
}

/// Returns the value of the upper half of PART.
স্থির অন্তঃরেখ APInt::WordType উপরের_Half(APInt::WordType part) {
  ফেরত part >> (APInt::APINT_BITS_PER_WORD / 2);
}

/// Sets the least significant part of a bignum to the input value, and zeroes
/// out higher parts.
শূন্য APInt::tcSet(WordType *dst, WordType part, unsigned parts) {
  প্রতিজ্ঞা(parts > 0);
  dst[0] = part;
  জন্য (unsigned i = 1; i < parts; i++)
    dst[i] = 0;
}

/// Assign one bignum to another.
শূন্য APInt::tcAssign(WordType *dst, ধ্রুবক WordType *src, unsigned parts) {
  জন্য (unsigned i = 0; i < parts; i++)
    dst[i] = src[i];
}

/// Returns true if a bignum is zero, false otherwise.
যৌক্তিক APInt::tcIsZero(ধ্রুবক WordType *src, unsigned parts) {
  জন্য (unsigned i = 0; i < parts; i++)
    যদি (src[i])
      ফেরত মিথ্যা;

  ফেরত সত্য;
}

/// Extract the given bit of a bignum; returns 0 or 1.
পূর্ণসংখ্যা APInt::tcExtractBit(ধ্রুবক WordType *parts, unsigned bit) {
  ফেরত (parts[whichWord(bit)] & maskBit(bit)) != 0;
}

/// Set the given bit of a bignum.
শূন্য APInt::tcSetBit(WordType *parts, unsigned bit) {
  parts[whichWord(bit)] |= maskBit(bit);
}

/// Clears the given bit of a bignum.
শূন্য APInt::tcClearBit(WordType *parts, unsigned bit) {
  parts[whichWord(bit)] &= ~maskBit(bit);
}

/// Returns the bit number of the least significant set bit of a number.  If the
/// input number has no bits set UINT_MAX is returned.
unsigned APInt::tcLSB(ধ্রুবক WordType *parts, unsigned n) {
  জন্য (unsigned i = 0; i < n; i++) {
    যদি (parts[i] != 0) {
      unsigned lsb = llvm::countr_zero(parts[i]);
      ফেরত lsb + i * APINT_BITS_PER_WORD;
    }
  }

  ফেরত UINT_MAX;
}

/// Returns the bit number of the most significant set bit of a number.
/// If the input number has no bits set UINT_MAX is returned.
unsigned APInt::tcMSB(ধ্রুবক WordType *parts, unsigned n) {
  করো {
    --n;

    যদি (parts[n] != 0) {
      static_assert(আকার(parts[n]) <= আকার(uint64_t));
      unsigned msb = llvm::Log2_64(parts[n]);

      ফেরত msb + n * APINT_BITS_PER_WORD;
    }
  } যখন (n);

  ফেরত UINT_MAX;
}

/// Copy the bit vector of width srcBITS from SRC, starting at bit srcLSB, to
/// DST, of dstCOUNT parts, such that the bit srcLSB becomes the least
/// significant bit of DST.  All high bits above srcBITS in DST are zero-filled.
/// */
শূন্য
APInt::tcExtract(WordType *dst, unsigned dst_গুনতি, ধ্রুবক WordType *src,
                 unsigned srcBits, unsigned srcLSB) {
  unsigned dstParts = (srcBits + APINT_BITS_PER_WORD - 1) / APINT_BITS_PER_WORD;
  প্রতিজ্ঞা(dstParts <= dst_গুনতি);

  unsigned firstSrcPart = srcLSB / APINT_BITS_PER_WORD;
  tcAssign(dst, src + firstSrcPart, dstParts);

  unsigned shift = srcLSB % APINT_BITS_PER_WORD;
  tc_Shift_ডান(dst, dstParts, shift);

  // We now have (dstParts * APINT_BITS_PER_WORD - shift) bits from SRC
  // in DST.  If this is less that srcBits, append the rest, else
  // clear the high bits.
  unsigned n = dstParts * APINT_BITS_PER_WORD - shift;
  যদি (n < srcBits) {
    WordType mask = নিচের_Bit_Mask (srcBits - n);
    dst[dstParts - 1] |= ((src[firstSrcPart + dstParts] & mask)
                          << n % APINT_BITS_PER_WORD);
  } অন্যথা যদি (n > srcBits) {
    যদি (srcBits % APINT_BITS_PER_WORD)
      dst[dstParts - 1] &= নিচের_Bit_Mask (srcBits % APINT_BITS_PER_WORD);
  }

  // Clear high parts.
  যখন (dstParts < dst_গুনতি)
    dst[dstParts++] = 0;
}

//// DST += RHS + C where C is zero or one.  Returns the carry flag.
APInt::WordType APInt::tcAdd(WordType *dst, ধ্রুবক WordType *rhs,
                             WordType c, unsigned parts) {
  প্রতিজ্ঞা(c <= 1);

  জন্য (unsigned i = 0; i < parts; i++) {
    WordType l = dst[i];
    যদি (c) {
      dst[i] += rhs[i] + 1;
      c = (dst[i] <= l);
    } অন্যথা {
      dst[i] += rhs[i];
      c = (dst[i] < l);
    }
  }

  ফেরত c;
}

/// This function adds a single "word" integer, src, to the multiple
/// "word" integer array, dst[]. dst[] is modified to reflect the addition and
/// 1 is returned if there is a carry out, otherwise 0 is returned.
/// @returns the carry of the addition.
APInt::WordType APInt::tcAddPart(WordType *dst, WordType src,
                                 unsigned parts) {
  জন্য (unsigned i = 0; i < parts; ++i) {
    dst[i] += src;
    যদি (dst[i] >= src)
      ফেরত 0; // No need to carry so exit early.
    src = 1; // Carry one to next digit.
  }

  ফেরত 1;
}

/// DST -= RHS + C where C is zero or one.  Returns the carry flag.
APInt::WordType APInt::tcSubtract(WordType *dst, ধ্রুবক WordType *rhs,
                                  WordType c, unsigned parts) {
  প্রতিজ্ঞা(c <= 1);

  জন্য (unsigned i = 0; i < parts; i++) {
    WordType l = dst[i];
    যদি (c) {
      dst[i] -= rhs[i] + 1;
      c = (dst[i] >= l);
    } অন্যথা {
      dst[i] -= rhs[i];
      c = (dst[i] > l);
    }
  }

  ফেরত c;
}

/// This function subtracts a single "word" (64-bit word), src, from
/// the multi-word integer array, dst[], propagating the borrowed 1 value until
/// no further borrowing is needed or it runs out of "words" in dst.  The result
/// is 1 if "borrowing" exhausted the digits in dst, or 0 if dst was not
/// exhausted. In other words, if src > dst then this function returns 1,
/// otherwise 0.
/// @returns the borrow out of the subtraction
APInt::WordType APInt::tcSubtractPart(WordType *dst, WordType src,
                                      unsigned parts) {
  জন্য (unsigned i = 0; i < parts; ++i) {
    WordType Dst = dst[i];
    dst[i] -= src;
    যদি (src <= Dst)
      ফেরত 0; // No need to borrow so exit early.
    src = 1; // We have to "borrow 1" from next "word"
  }

  ফেরত 1;
}

/// Negate a bignum in-place.
শূন্য APInt::tcNegate(WordType *dst, unsigned parts) {
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
পূর্ণসংখ্যা APInt::tcMultiplyPart(WordType *dst, ধ্রুবক WordType *src,
                          WordType multiplier, WordType carry,
                          unsigned srcParts, unsigned dstParts,
                          যৌক্তিক add) {
  // Otherwise our writes of DST kill our later reads of SRC.
  প্রতিজ্ঞা(dst <= src || dst >= src + srcParts);
  প্রতিজ্ঞা(dstParts <= srcParts + 1);

  // N loops; minimum of dstParts and srcParts.
  unsigned n = std::min(dstParts, srcParts);

  জন্য (unsigned i = 0; i < n; i++) {
    // [LOW, HIGH] = MULTIPLIER * SRC[i] + DST[i] + CARRY.
    // This cannot overflow, because:
    //   (n - 1) * (n - 1) + 2 (n - 1) = (n - 1) * (n + 1)
    // which is less than n^2.
    WordType srcPart = src[i];
    WordType নিচের, mid, উপরের;
    যদি (multiplier == 0 || srcPart == 0) {
      নিচের = carry;
      উপরের = 0;
    } অন্যথা {
      নিচের = নিচের_Half(srcPart) * নিচের_Half(multiplier);
      উপরের = উপরের_Half(srcPart) * উপরের_Half(multiplier);

      mid = নিচের_Half(srcPart) * উপরের_Half(multiplier);
      উপরের += উপরের_Half(mid);
      mid <<= APINT_BITS_PER_WORD / 2;
      যদি (নিচের + mid < নিচের)
        উপরের++;
      নিচের += mid;

      mid = উপরের_Half(srcPart) * নিচের_Half(multiplier);
      উপরের += উপরের_Half(mid);
      mid <<= APINT_BITS_PER_WORD / 2;
      যদি (নিচের + mid < নিচের)
        উপরের++;
      নিচের += mid;

      // Now add carry.
      যদি (নিচের + carry < নিচের)
        উপরের++;
      নিচের += carry;
    }

    যদি (add) {
      // And now DST[i], and store the new low part there.
      যদি (নিচের + dst[i] < নিচের)
        উপরের++;
      dst[i] += নিচের;
    } অন্যথা {
      dst[i] = নিচের;
    }

    carry = উপরের;
  }

  যদি (srcParts < dstParts) {
    // Full multiplication, there is no overflow.
    প্রতিজ্ঞা(srcParts + 1 == dstParts);
    dst[srcParts] = carry;
    ফেরত 0;
  }

  // We overflowed if there is carry.
  যদি (carry)
    ফেরত 1;

  // We would overflow if any significant unwritten parts would be
  // non-zero.  This is true if any remaining src parts are non-zero
  // and the multiplier is non-zero.
  যদি (multiplier)
    জন্য (unsigned i = dstParts; i < srcParts; i++)
      যদি (src[i])
        ফেরত 1;

  // We fitted in the narrow destination.
  ফেরত 0;
}

/// DST = LHS * RHS, where DST has the same width as the operands and
/// is filled with the least significant parts of the result.  Returns
/// one if overflow occurred, otherwise zero.  DST must be disjoint
/// from both operands.
পূর্ণসংখ্যা APInt::tcMultiply(WordType *dst, ধ্রুবক WordType *lhs,
                      ধ্রুবক WordType *rhs, unsigned parts) {
  প্রতিজ্ঞা(dst != lhs && dst != rhs);

  পূর্ণসংখ্যা overflow = 0;

  জন্য (unsigned i = 0; i < parts; i++) {
    // Don't accumulate on the first iteration so we don't need to initalize
    // dst to 0.
    overflow |=
        tcMultiplyPart(&dst[i], lhs, rhs[i], 0, parts, parts - i, i != 0);
  }

  ফেরত overflow;
}

/// DST = LHS * RHS, where DST has width the sum of the widths of the
/// operands. No overflow occurs. DST must be disjoint from both operands.
শূন্য APInt::tcFullMultiply(WordType *dst, ধ্রুবক WordType *lhs,
                           ধ্রুবক WordType *rhs, unsigned lhsParts,
                           unsigned rhsParts) {
  // Put the narrower number on the LHS for less loops below.
  যদি (lhsParts > rhsParts)
    ফেরত tcFullMultiply (dst, rhs, lhs, rhsParts, lhsParts);

  প্রতিজ্ঞা(dst != lhs && dst != rhs);

  জন্য (unsigned i = 0; i < lhsParts; i++) {
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
পূর্ণসংখ্যা APInt::tcDivide(WordType *lhs, ধ্রুবক WordType *rhs,
                    WordType *remainder, WordType *srhs,
                    unsigned parts) {
  প্রতিজ্ঞা(lhs != remainder && lhs != srhs && remainder != srhs);

  unsigned shift_গুনতি = tcMSB(rhs, parts) + 1;
  যদি (shift_গুনতি == 0)
    ফেরত সত্য;

  shift_গুনতি = parts * APINT_BITS_PER_WORD - shift_গুনতি;
  unsigned n = shift_গুনতি / APINT_BITS_PER_WORD;
  WordType mask = (WordType) 1 << (shift_গুনতি % APINT_BITS_PER_WORD);

  tcAssign(srhs, rhs, parts);
  tc_Shift_বাম(srhs, parts, shift_গুনতি);
  tcAssign(remainder, lhs, parts);
  tcSet(lhs, 0, parts);

  // Loop, subtracting SRHS if REMAINDER is greater and adding that to the
  // total.
  জন্য (;;) {
    পূর্ণসংখ্যা compare = tcCompare(remainder, srhs, parts);
    যদি (compare >= 0) {
      tcSubtract(remainder, srhs, 0, parts);
      lhs[n] |= mask;
    }

    যদি (shift_গুনতি == 0)
      বিরতি;
    shift_গুনতি--;
    tc_Shift_ডান(srhs, parts, 1);
    যদি ((mask >>= 1) == 0) {
      mask = (WordType) 1 << (APINT_BITS_PER_WORD - 1);
      n--;
    }
  }

  ফেরত মিথ্যা;
}

/// Shift a bignum left Count bits in-place. Shifted in bits are zero. There are
/// no restrictions on Count.
শূন্য APInt::tc_Shift_বাম(WordType *Dst, unsigned শব্দগুলি_2, unsigned গুনতি_2) {
  // Don't bother performing a no-op shift.
  যদি (!গুনতি_2)
    ফেরত;

  // WordShift is the inter-part shift; BitShift is the intra-part shift.
  unsigned WordShift = std::min(গুনতি_2 / APINT_BITS_PER_WORD, শব্দগুলি_2);
  unsigned BitShift = গুনতি_2 % APINT_BITS_PER_WORD;

  // Fastpath for moving by whole words.
  যদি (BitShift == 0) {
    std::memmove(Dst + WordShift, Dst, (শব্দগুলি_2 - WordShift) * APINT_WORD_আকার);
  } অন্যথা {
    যখন (শব্দগুলি_2-- > WordShift) {
      Dst[শব্দগুলি_2] = Dst[শব্দগুলি_2 - WordShift] << BitShift;
      যদি (শব্দগুলি_2 > WordShift)
        Dst[শব্দগুলি_2] |=
          Dst[শব্দগুলি_2 - WordShift - 1] >> (APINT_BITS_PER_WORD - BitShift);
    }
  }

  // Fill in the remainder with 0s.
  std::memset(Dst, 0, WordShift * APINT_WORD_আকার);
}

/// Shift a bignum right Count bits in-place. Shifted in bits are zero. There
/// are no restrictions on Count.
শূন্য APInt::tc_Shift_ডান(WordType *Dst, unsigned শব্দগুলি_2, unsigned গুনতি_2) {
  // Don't bother performing a no-op shift.
  যদি (!গুনতি_2)
    ফেরত;

  // WordShift is the inter-part shift; BitShift is the intra-part shift.
  unsigned WordShift = std::min(গুনতি_2 / APINT_BITS_PER_WORD, শব্দগুলি_2);
  unsigned BitShift = গুনতি_2 % APINT_BITS_PER_WORD;

  unsigned শব্দগুলি_To_Move = শব্দগুলি_2 - WordShift;
  // Fastpath for moving by whole words.
  যদি (BitShift == 0) {
    std::memmove(Dst, Dst + WordShift, শব্দগুলি_To_Move * APINT_WORD_আকার);
  } অন্যথা {
    জন্য (unsigned i = 0; i != শব্দগুলি_To_Move; ++i) {
      Dst[i] = Dst[i + WordShift] >> BitShift;
      যদি (i + 1 != শব্দগুলি_To_Move)
        Dst[i] |= Dst[i + WordShift + 1] << (APINT_BITS_PER_WORD - BitShift);
    }
  }

  // Fill in the remainder with 0s.
  std::memset(Dst + শব্দগুলি_To_Move, 0, WordShift * APINT_WORD_আকার);
}

// Comparison (unsigned) of two bignums.
পূর্ণসংখ্যা APInt::tcCompare(ধ্রুবক WordType *lhs, ধ্রুবক WordType *rhs,
                     unsigned parts) {
  যখন (parts) {
    parts--;
    যদি (lhs[parts] != rhs[parts])
      ফেরত (lhs[parts] > rhs[parts]) ? 1 : -1;
  }

  ফেরত 0;
}

APInt llvm::APIntOps::RoundingUDiv(ধ্রুবক APInt &A, ধ্রুবক APInt &B,
                                   APInt::Rounding RM) {
  // Currently udivrem always rounds down.
  নির্বাচন (RM) {
  ক্ষেত্র APInt::Rounding::DOWN:
  ক্ষেত্র APInt::Rounding::TOWARD_ZERO:
    ফেরত A.udiv(B);
  ক্ষেত্র APInt::Rounding::UP: {
    APInt Quo, Rem;
    APInt::udivrem(A, B, Quo, Rem);
    যদি (Rem.isZero())
      ফেরত Quo;
    ফেরত Quo + 1;
  }
  }
  llvm_unreachable("Unknown APInt::Rounding enum");
}

APInt llvm::APIntOps::RoundingSDiv(ধ্রুবক APInt &A, ধ্রুবক APInt &B,
                                   APInt::Rounding RM) {
  নির্বাচন (RM) {
  ক্ষেত্র APInt::Rounding::DOWN:
  ক্ষেত্র APInt::Rounding::UP: {
    APInt Quo, Rem;
    APInt::sdivrem(A, B, Quo, Rem);
    যদি (Rem.isZero())
      ফেরত Quo;
    // This algorithm deals with arbitrary rounding mode used by sdivrem.
    // We want to check whether the non-integer part of the mathematical value
    // is negative or not. If the non-integer part is negative, we need to round
    // down from Quo; otherwise, if it's positive or 0, we return Quo, as it's
    // already rounded down.
    যদি (RM == APInt::Rounding::DOWN) {
      যদি (Rem.isNegative() != B.isNegative())
        ফেরত Quo - 1;
      ফেরত Quo;
    }
    যদি (Rem.isNegative() != B.isNegative())
      ফেরত Quo;
    ফেরত Quo + 1;
  }
  // Currently sdiv rounds towards zero.
  ক্ষেত্র APInt::Rounding::TOWARD_ZERO:
    ফেরত A.sdiv(B);
  }
  llvm_unreachable("Unknown APInt::Rounding enum");
}

std::optional<APInt>
llvm::APIntOps::SolveQuadraticEquationWrap(APInt A, APInt B, APInt C,
                                           unsigned RangeWidth) {
  unsigned CoeffWidth = A.getBitWidth();
  প্রতিজ্ঞা(CoeffWidth == B.getBitWidth() && CoeffWidth == C.getBitWidth());
  প্রতিজ্ঞা(RangeWidth <= CoeffWidth &&
         "Value range width should be less than coefficient width");
  প্রতিজ্ঞা(RangeWidth > 1 && "Value range bit width should be > 1");

  LLVM_DEBUG(dbgs() << __func__ << ": solving " << A << "x^2 + " << B
                    << "x + " << C << ", rw:" << RangeWidth << '\n');

  // Identify 0 as a (non)solution immediately.
  যদি (C.sextOrTrunc(RangeWidth).isZero()) {
    LLVM_DEBUG(dbgs() << __func__ << ": zero solution\n");
    ফেরত APInt(CoeffWidth, 0);
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
  CoeffWidth *= 3;
  A = A.sext(CoeffWidth);
  B = B.sext(CoeffWidth);
  C = C.sext(CoeffWidth);

  // Make A > 0 for simplicity. Negate cannot overflow at this point because
  // the bit width has increased.
  যদি (A.isNegative()) {
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
  APInt R = APInt::getOneBitSet(CoeffWidth, RangeWidth);
  APInt TwoA = 2 * A;
  APInt SqrB = B * B;
  যৌক্তিক Pick_নিচের;

  auto RoundUp = [] (ধ্রুবক APInt &V, ধ্রুবক APInt &A) -> APInt {
    প্রতিজ্ঞা(A.isStrictlyPositive());
    APInt T = V.abs().urem(A);
    যদি (T.isZero())
      ফেরত V;
    ফেরত V.isNegative() ? V+T : V+(A-T);
  };

  // The vertex of the parabola is at -B/2A, but since A > 0, it's negative
  // iff B is positive.
  যদি (B.isNonNegative()) {
    // If B >= 0, the vertex it at a negative location (or at 0), so in
    // order to have a non-negative solution we need to pick k that makes
    // C-kR negative. To satisfy all the requirements for the solution
    // that we are looking for, it needs to be closest to 0 of all k.
    C = C.srem(R);
    যদি (C.isStrictlyPositive())
      C -= R;
    // Pick the greater solution.
    Pick_নিচের = মিথ্যা;
  } অন্যথা {
    // If B < 0, the vertex is at a positive location. For any solution
    // to exist, the discriminant must be non-negative. This means that
    // C-kR <= B^2/4A is a necessary condition for k, i.e. there is a
    // lower bound on values of k: kR >= C - B^2/4A.
    APInt LowkR = C - SqrB.udiv(2*TwoA); // udiv because all values > 0.
    // Round LowkR up (towards +inf) to the nearest kR.
    LowkR = RoundUp(LowkR, R);

    // If there exists k meeting the condition above, and such that
    // C-kR > 0, there will be two positive real number solutions of
    // q(x) = kR. Out of all such values of k, pick the one that makes
    // C-kR closest to 0, (i.e. pick maximum k such that C-kR > 0).
    // In other words, find maximum k such that LowkR <= kR < C.
    যদি (C.sgt(LowkR)) {
      // If LowkR < C, then such a k is guaranteed to exist because
      // LowkR itself is a multiple of R.
      C -= -RoundUp(-C, R);      // C = C - RoundDown(C, R)
      // Pick the smaller solution.
      Pick_নিচের = সত্য;
    } অন্যথা {
      // If C-kR < 0 for all potential k's, it means that one solution
      // will be negative, while the other will be positive. The positive
      // solution will shift towards 0 if the parabola is moved up.
      // Pick the kR closest to the lower bound (i.e. make C-kR closest
      // to 0, or in other words, out of all parabolas that have solutions,
      // pick the one that is the farthest "up").
      // Since LowkR is itself a multiple of R, simply take C-LowkR.
      C -= LowkR;
      // Pick the greater solution.
      Pick_নিচের = মিথ্যা;
    }
  }

  LLVM_DEBUG(dbgs() << __func__ << ": updated coefficients " << A << "x^2 + "
                    << B << "x + " << C << ", rw:" << RangeWidth << '\n');

  APInt D = SqrB - 4*A*C;
  প্রতিজ্ঞা(D.isNonNegative() && "Negative discriminant");
  APInt SQ = D.sqrtFloor();

  APInt Q = SQ * SQ;
  যৌক্তিক InexactSQ = Q != D;

  APInt X;
  APInt Rem;

  // SQ is rounded down (i.e SQ * SQ <= D), so the roots may be inexact.
  // When using the quadratic formula directly, the calculated low root
  // may be greater than the exact one, since we would be subtracting SQ.
  // To make sure that the calculated root is not greater than the exact
  // one, subtract SQ+1 when calculating the low root (for inexact value
  // of SQ).
  যদি (Pick_নিচের)
    APInt::sdivrem(-B - (SQ+InexactSQ), TwoA, X, Rem);
  অন্যথা
    APInt::sdivrem(-B + SQ, TwoA, X, Rem);

  // The updated coefficients should be such that the (exact) solution is
  // positive. Since APInt division rounds towards 0, the calculated one
  // can be 0, but cannot be negative.
  প্রতিজ্ঞা(X.isNonNegative() && "Solution should be non-negative");

  যদি (!InexactSQ && Rem.isZero()) {
    LLVM_DEBUG(dbgs() << __func__ << ": solution (root): " << X << '\n');
    ফেরত X;
  }

  প্রতিজ্ঞা((SQ*SQ).sle(D) && "SQ = |_sqrt(D)_|, so SQ*SQ <= D");
  // The exact value of the square root of D should be between SQ and SQ+1.
  // This implies that the solution should be between that corresponding to
  // SQ (i.e. X) and that corresponding to SQ+1.
  //
  // The calculated X cannot be greater than the exact (real) solution.
  // Actually it must be strictly less than the exact solution, while
  // X+1 will be greater than or equal to it.

  APInt VX = (A*X + B)*X + C;
  APInt VY = VX + TwoA*X + A + B;
  যৌক্তিক SignChange =
      VX.isNegative() != VY.isNegative() || VX.isZero() != VY.isZero();
  // If the sign did not change between X and X+1, X is not a valid solution.
  // This could happen when the actual (exact) roots don't have an integer
  // between them, so they would both be contained between X and X+1.
  যদি (!SignChange) {
    LLVM_DEBUG(dbgs() << __func__ << ": no valid solution\n");
    ফেরত std::nullopt;
  }

  X += 1;
  LLVM_DEBUG(dbgs() << __func__ << ": solution (wrap): " << X << '\n');
  ফেরত X;
}

std::optional<unsigned>
llvm::APIntOps::GetMostSignificantDifferentBit(ধ্রুবক APInt &A, ধ্রুবক APInt &B) {
  প্রতিজ্ঞা(A.getBitWidth() == B.getBitWidth() && "Must have the same bitwidth");
  যদি (A == B)
    ফেরত std::nullopt;
  ফেরত A.getBitWidth() - ((A ^ B).countl_zero() + 1);
}

APInt llvm::APIntOps::ScaleBitMask(ধ্রুবক APInt &A, unsigned NewBitWidth,
                                   যৌক্তিক MatchAllBits) {
  unsigned OldBitWidth = A.getBitWidth();
  প্রতিজ্ঞা((((OldBitWidth % NewBitWidth) == 0) ||
          ((NewBitWidth % OldBitWidth) == 0)) &&
         "One size should be a multiple of the other one. "
         "Can't do fractional scaling.");

  // Check for matching bitwidths.
  যদি (OldBitWidth == NewBitWidth)
    ফেরত A;

  APInt NewA = APInt::getZero(NewBitWidth);

  // Check for null input.
  যদি (A.isZero())
    ফেরত NewA;

  যদি (NewBitWidth > OldBitWidth) {
    // Repeat bits.
    unsigned Scale = NewBitWidth / OldBitWidth;
    জন্য (unsigned i = 0; i != OldBitWidth; ++i)
      যদি (A[i])
        NewA.setBits(i * Scale, (i + 1) * Scale);
  } অন্যথা {
    unsigned Scale = OldBitWidth / NewBitWidth;
    জন্য (unsigned i = 0; i != NewBitWidth; ++i) {
      যদি (MatchAllBits) {
        যদি (A.extractBits(Scale, i * Scale).isAllOnes())
          NewA.setBit(i);
      } অন্যথা {
        যদি (!A.extractBits(Scale, i * Scale).isZero())
          NewA.setBit(i);
      }
    }
  }

  ফেরত NewA;
}

/// StoreIntToMemory - Fills the StoreBytes bytes of memory starting from Dst
/// with the integer held in IntVal.
শূন্য llvm::StoreIntToMemory(ধ্রুবক APInt &IntVal, uint8_t *Dst,
                            unsigned StoreBytes) {
  প্রতিজ্ঞা((IntVal.getBitWidth()+7)/8 >= StoreBytes && "Integer too small!");
  ধ্রুবক uint8_t *Src = (ধ্রুবক uint8_t *)IntVal.get_Raw_তথ্য();

  যদি (sys::IsLittleEndianHost) {
    // Little-endian host - the source is ordered from LSB to MSB.  Order the
    // destination from LSB to MSB: Do a straight copy.
    memcpy(Dst, Src, StoreBytes);
  } অন্যথা {
    // Big-endian host - the source is an array of 64 bit words ordered from
    // LSW to MSW.  Each word is ordered from MSB to LSB.  Order the destination
    // from MSB to LSB: Reverse the word order, but not the bytes in a word.
    যখন (StoreBytes > আকার(uint64_t)) {
      StoreBytes -= আকার(uint64_t);
      // May not be aligned so use memcpy.
      memcpy(Dst + StoreBytes, Src, আকার(uint64_t));
      Src += আকার(uint64_t);
    }

    memcpy(Dst, Src + আকার(uint64_t) - StoreBytes, StoreBytes);
  }
}

/// LoadIntFromMemory - Loads the integer stored in the LoadBytes bytes starting
/// from Src into IntVal, which is assumed to be wide enough and to hold zero.
শূন্য llvm::LoadIntFromMemory(APInt &IntVal, ধ্রুবক uint8_t *Src,
                             unsigned LoadBytes) {
  প্রতিজ্ঞা((IntVal.getBitWidth()+7)/8 >= LoadBytes && "Integer too small!");
  uint8_t *Dst = reinterpret_cast<uint8_t *>(
                   const_cast<uint64_t *>(IntVal.get_Raw_তথ্য()));

  যদি (sys::IsLittleEndianHost)
    // Little-endian host - the destination must be ordered from LSB to MSB.
    // The source is ordered from LSB to MSB: Do a straight copy.
    memcpy(Dst, Src, LoadBytes);
  অন্যথা {
    // Big-endian - the destination is an array of 64 bit words ordered from
    // LSW to MSW.  Each word must be ordered from MSB to LSB.  The source is
    // ordered from MSB to LSB: Reverse the word order, but not the bytes in
    // a word.
    যখন (LoadBytes > আকার(uint64_t)) {
      LoadBytes -= আকার(uint64_t);
      // May not be aligned so use memcpy.
      memcpy(Dst, Src + LoadBytes, আকার(uint64_t));
      Dst += আকার(uint64_t);
    }

    memcpy(Dst + আকার(uint64_t) - LoadBytes, Src, LoadBytes);
  }
}

APInt APIntOps::avgFloorS(ধ্রুবক APInt &C1, ধ্রুবক APInt &C2) {
  // Return floor((C1 + C2) / 2)
  ফেরত (C1 & C2) + (C1 ^ C2).ashr(1);
}

APInt APIntOps::avgFloorU(ধ্রুবক APInt &C1, ধ্রুবক APInt &C2) {
  // Return floor((C1 + C2) / 2)
  ফেরত (C1 & C2) + (C1 ^ C2).lshr(1);
}

APInt APIntOps::avgCeilS(ধ্রুবক APInt &C1, ধ্রুবক APInt &C2) {
  // Return ceil((C1 + C2) / 2)
  ফেরত (C1 | C2) - (C1 ^ C2).ashr(1);
}

APInt APIntOps::avgCeilU(ধ্রুবক APInt &C1, ধ্রুবক APInt &C2) {
  // Return ceil((C1 + C2) / 2)
  ফেরত (C1 | C2) - (C1 ^ C2).lshr(1);
}

APInt APIntOps::avgSubFloorS(ধ্রুবক APInt &C1, ধ্রুবক APInt &C2) {
  // Return floor((C1 - C2) / 2), using C1 - C2 == (C1 ^ C2) - 2 * (~C1 & C2).
  ফেরত (C1 ^ C2).ashr(1) - (~C1 & C2);
}

APInt APIntOps::avgSubFloorU(ধ্রুবক APInt &C1, ধ্রুবক APInt &C2) {
  // Return floor((C1 - C2) / 2), using C1 - C2 == (C1 ^ C2) - 2 * (~C1 & C2).
  ফেরত (C1 ^ C2).lshr(1) - (~C1 & C2);
}

APInt APIntOps::mulhs(ধ্রুবক APInt &C1, ধ্রুবক APInt &C2) {
  প্রতিজ্ঞা(C1.getBitWidth() == C2.getBitWidth() && "Unequal bitwidths");
  unsigned FullWidth = C1.getBitWidth() * 2;
  APInt C1Ext = C1.sext(FullWidth);
  APInt C2Ext = C2.sext(FullWidth);
  ফেরত (C1Ext * C2Ext).extractBits(C1.getBitWidth(), C1.getBitWidth());
}

APInt APIntOps::mulhu(ধ্রুবক APInt &C1, ধ্রুবক APInt &C2) {
  প্রতিজ্ঞা(C1.getBitWidth() == C2.getBitWidth() && "Unequal bitwidths");
  unsigned FullWidth = C1.getBitWidth() * 2;
  APInt C1Ext = C1.zext(FullWidth);
  APInt C2Ext = C2.zext(FullWidth);
  ফেরত (C1Ext * C2Ext).extractBits(C1.getBitWidth(), C1.getBitWidth());
}

APInt APIntOps::mulsExtended(ধ্রুবক APInt &C1, ধ্রুবক APInt &C2) {
  প্রতিজ্ঞা(C1.getBitWidth() == C2.getBitWidth() && "Unequal bitwidths");
  unsigned FullWidth = C1.getBitWidth() * 2;
  APInt C1Ext = C1.sext(FullWidth);
  APInt C2Ext = C2.sext(FullWidth);
  ফেরত C1Ext * C2Ext;
}

APInt APIntOps::muluExtended(ধ্রুবক APInt &C1, ধ্রুবক APInt &C2) {
  প্রতিজ্ঞা(C1.getBitWidth() == C2.getBitWidth() && "Unequal bitwidths");
  unsigned FullWidth = C1.getBitWidth() * 2;
  APInt C1Ext = C1.zext(FullWidth);
  APInt C2Ext = C2.zext(FullWidth);
  ফেরত C1Ext * C2Ext;
}

APInt APIntOps::pow(ধ্রুবক APInt &X, int64_t N) {
  প্রতিজ্ঞা(N >= 0 && "negative exponents not supported.");
  APInt Acc = APInt(X.getBitWidth(), 1);
  যদি (N == 0)
    ফেরত Acc;
  APInt Base = X;
  int64_t RemainingExponent = N;
  যখন (RemainingExponent > 0) {
    যখন (RemainingExponent % 2 == 0) {
      Base *= Base;
      RemainingExponent /= 2;
    }
    --RemainingExponent;
    Acc *= Base;
  }
  ফেরত Acc;
}

APInt llvm::APIntOps::fshl(ধ্রুবক APInt &Hi, ধ্রুবক APInt &Lo,
                           ধ্রুবক APInt &Shift) {
  প্রতিজ্ঞা(Hi.getBitWidth() == Lo.getBitWidth());
  unsigned ShiftAmt = rotateModulo(Hi.getBitWidth(), Shift);
  যদি (ShiftAmt == 0)
    ফেরত Hi;
  ফেরত Hi.shl(ShiftAmt) | Lo.lshr(Hi.getBitWidth() - ShiftAmt);
}

APInt llvm::APIntOps::fshr(ধ্রুবক APInt &Hi, ধ্রুবক APInt &Lo,
                           ধ্রুবক APInt &Shift) {
  প্রতিজ্ঞা(Hi.getBitWidth() == Lo.getBitWidth());
  unsigned ShiftAmt = rotateModulo(Hi.getBitWidth(), Shift);
  যদি (ShiftAmt == 0)
    ফেরত Lo;
  ফেরত Hi.shl(Hi.getBitWidth() - ShiftAmt) | Lo.lshr(ShiftAmt);
}

APInt llvm::APIntOps::clmul(ধ্রুবক APInt &LHS, ধ্রুবক APInt &RHS) {
  unsigned BW = LHS.getBitWidth();
  প্রতিজ্ঞা(BW == RHS.getBitWidth() && "Operand mismatch");
  APInt ফলাফল(BW, 0);
  জন্য (unsigned I : seq(std::min(RHS.getActiveBits(), BW - LHS.countr_zero())))
    যদি (RHS[I])
      ফলাফল ^= LHS << I;
  ফেরত ফলাফল;
}

APInt llvm::APIntOps::clmulr(ধ্রুবক APInt &LHS, ধ্রুবক APInt &RHS) {
  প্রতিজ্ঞা(LHS.getBitWidth() == RHS.getBitWidth());
  ফেরত clmul(LHS.reverseBits(), RHS.reverseBits()).reverseBits();
}

APInt llvm::APIntOps::clmulh(ধ্রুবক APInt &LHS, ধ্রুবক APInt &RHS) {
  প্রতিজ্ঞা(LHS.getBitWidth() == RHS.getBitWidth());
  ফেরত clmulr(LHS, RHS).lshr(1);
}

APInt llvm::APIntOps::pext(ধ্রুবক APInt &Val, ধ্রুবক APInt &Mask) {
  unsigned BW = Val.getBitWidth();
  প্রতিজ্ঞা(BW == Mask.getBitWidth() && "Operand mismatch");
  APInt ফলাফল = APInt::getZero(BW);
  জন্য (unsigned I = 0, P = 0; I != BW; ++I)
    যদি (Mask[I])
      ফলাফল.setBitVal(P++, Val[I]);
  ফেরত ফলাফল;
}

APInt llvm::APIntOps::pdep(ধ্রুবক APInt &Val, ধ্রুবক APInt &Mask) {
  unsigned BW = Val.getBitWidth();
  প্রতিজ্ঞা(BW == Mask.getBitWidth() && "Operand mismatch");
  APInt ফলাফল = APInt::getZero(BW);
  জন্য (unsigned I = 0, P = 0; I != BW; ++I)
    যদি (Mask[I])
      ফলাফল.setBitVal(I, Val[P++]);
  ফেরত ফলাফল;
}
