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

#அந்தர்பாவ "llvm/ADT/APInt.h"
#அந்தர்பாவ "llvm/ADT/ArrayRef.h"
#அந்தர்பாவ "llvm/ADT/FoldingSet.h"
#அந்தர்பாவ "llvm/ADT/Hashing.h"
#அந்தர்பாவ "llvm/ADT/Sequence.h"
#அந்தர்பாவ "llvm/ADT/SmallString.h"
#அந்தர்பாவ "llvm/ADT/StringRef.h"
#அந்தர்பாவ "llvm/ADT/bit.h"
#அந்தர்பாவ "llvm/Support/Alignment.h"
#அந்தர்பாவ "llvm/Support/Debug.h"
#அந்தர்பாவ "llvm/Support/ErrorHandling.h"
#அந்தர்பாவ "llvm/Support/MathExtras.h"
#அந்தர்பாவ "llvm/Support/SwapByteOrder.h"
#அந்தர்பாவ "llvm/Support/raw_ostream.h"
#அந்தர்பாவ <cmath>
#அந்தர்பாவ <optional>

using நாமாகாஶ llvm;

#define DEBUG_TYPE "apint"

/// A utility function for allocating memory, checking for allocation failures,
/// and ensuring the contents are zeroed.
உள்வரி நிலையான uint64_t* getClearedMemory(unsigned இலக்கம்_சொற்கள்) {
  திரும்பு புதிய uint64_t[இலக்கம்_சொற்கள்]();
}

/// A utility function for allocating memory and checking for allocation
/// failure.  The content is not zeroed.
உள்வரி நிலையான uint64_t* getMemory(unsigned இலக்கம்_சொற்கள்) {
  திரும்பு புதிய uint64_t[இலக்கம்_சொற்கள்];
}

/// A utility function that converts a character to a digit.
உள்வரி நிலையான unsigned getDigit(எழுத்து cdigit, uint8_t radix) {
  unsigned r;

  எனில் (radix == 16 || radix == 36) {
    r = cdigit - '0';
    எனில் (r <= 9)
      திரும்பு r;

    r = cdigit - 'A';
    எனில் (r <= radix - 11U)
      திரும்பு r + 10;

    r = cdigit - 'a';
    எனில் (r <= radix - 11U)
      திரும்பு r + 10;

    radix = 10;
  }

  r = cdigit - '0';
  எனில் (r < radix)
    திரும்பு r;

  திரும்பு UINT_MAX;
}


வெறுமை APInt::initSlowCase(uint64_t val, தருக்க isSigned) {
  எனில் (isSigned && int64_t(val) < 0) {
    U.pVal = getMemory(get_இலக்கம்_சொற்கள்());
    U.pVal[0] = val;
    memset(&U.pVal[1], 0xFF, APINT_WORD_SIZE * (get_இலக்கம்_சொற்கள்() - 1));
    clearUnusedBits();
  } இல்லையெனில் {
    U.pVal = getClearedMemory(get_இலக்கம்_சொற்கள்());
    U.pVal[0] = val;
  }
}

வெறுமை APInt::initSlowCase(மாறிலி APInt& that) {
  U.pVal = getMemory(get_இலக்கம்_சொற்கள்());
  memcpy(U.pVal, that.U.pVal, get_இலக்கம்_சொற்கள்() * APINT_WORD_SIZE);
}

வெறுமை APInt::initFromArray(ArrayRef<uint64_t> bigVal) {
  ப்ரதிஜ்ஞா(bigVal.data() && "Null pointer detected!");
  எனில் (LLVM_LIKELY(isSingleWord()))
    U.VAL = bigVal[0];
  இல்லையெனில் {
    // Get memory, cleared to 0
    U.pVal = getClearedMemory(get_இலக்கம்_சொற்கள்());
    // Calculate the number of words to copy
    unsigned சொற்கள் = std::min<unsigned>(bigVal.size(), get_இலக்கம்_சொற்கள்());
    // Copy the words from bigVal to pVal
    memcpy(U.pVal, bigVal.data(), சொற்கள் * APINT_WORD_SIZE);
  }
  // Make sure unused high bits are cleared
  clearUnusedBits();
}

APInt::APInt(unsigned இலக்கம்_Bits, ArrayRef<uint64_t> bigVal) : BitWidth(இலக்கம்_Bits) {
  initFromArray(bigVal);
}

APInt::APInt(unsigned numbits, StringRef Str, uint8_t radix)
    : BitWidth(numbits) {
  fromString(numbits, Str, radix);
}

வெறுமை APInt::reallocate(unsigned NewBitWidth) {
  // If the number of words is the same we can just change the width and stop.
  எனில் (get_இலக்கம்_சொற்கள்() == get_இலக்கம்_சொற்கள்(NewBitWidth)) {
    BitWidth = NewBitWidth;
    திரும்பு;
  }

  // If we have an allocation, delete it.
  எனில் (LLVM_UNLIKELY(!isSingleWord()))
    delete [] U.pVal;

  // Update BitWidth.
  BitWidth = NewBitWidth;

  // If we are supposed to have an allocation, create it.
  எனில் (LLVM_UNLIKELY(!isSingleWord()))
    U.pVal = getMemory(get_இலக்கம்_சொற்கள்());
}

வெறுமை APInt::assignSlowCase(மாறிலி APInt &RHS) {
  // Don't do anything for X = X
  எனில் (இது == &RHS)
    திரும்பு;

  // Adjust the bit width and handle allocations as necessary.
  reallocate(RHS.getBitWidth());

  // Copy the data.
  எனில் (LLVM_LIKELY(isSingleWord()))
    U.VAL = RHS.U.VAL;
  இல்லையெனில்
    memcpy(U.pVal, RHS.U.pVal, get_இலக்கம்_சொற்கள்() * APINT_WORD_SIZE);
}

/// This method 'profiles' an APInt for use with FoldingSet.
வெறுமை APInt::Profile(Folding_Set_முனை_ID& ID) மாறிலி {
  ID.AddInteger(BitWidth);

  எனில் (LLVM_LIKELY(isSingleWord())) {
    ID.AddInteger(U.VAL);
    திரும்பு;
  }

  unsigned இலக்கம்_சொற்கள்_2 = get_இலக்கம்_சொற்கள்();
  க்கு (unsigned i = 0; i < இலக்கம்_சொற்கள்_2; ++i)
    ID.AddInteger(U.pVal[i]);
}

தருக்க APInt::isAligned(Align A) மாறிலி {
  எனில் (isZero())
    திரும்பு உண்மை;
  மாறிலி unsigned TrailingZeroes = countr_zero();
  மாறிலி unsigned MinimumTrailingZeroes = Log2(A);
  திரும்பு TrailingZeroes >= MinimumTrailingZeroes;
}

/// Prefix increment operator. Increments the APInt by one.
APInt& APInt::operator++() {
  எனில் (LLVM_LIKELY(isSingleWord()))
    ++U.VAL;
  இல்லையெனில்
    tcIncrement(U.pVal, get_இலக்கம்_சொற்கள்());
  திரும்பு clearUnusedBits();
}

/// Prefix decrement operator. Decrements the APInt by one.
APInt& APInt::operator--() {
  எனில் (LLVM_LIKELY(isSingleWord()))
    --U.VAL;
  இல்லையெனில்
    tcDecrement(U.pVal, get_இலக்கம்_சொற்கள்());
  திரும்பு clearUnusedBits();
}

/// Adds the RHS APInt to this APInt.
/// @returns this, after addition of RHS.
/// Addition assignment operator.
APInt& APInt::operator+=(மாறிலி APInt& RHS) {
  ப்ரதிஜ்ஞா(BitWidth == RHS.BitWidth && "Bit widths must be the same");
  எனில் (LLVM_LIKELY(isSingleWord()))
    U.VAL += RHS.U.VAL;
  இல்லையெனில்
    tcAdd(U.pVal, RHS.U.pVal, 0, get_இலக்கம்_சொற்கள்());
  திரும்பு clearUnusedBits();
}

APInt& APInt::operator+=(uint64_t RHS) {
  எனில் (LLVM_LIKELY(isSingleWord()))
    U.VAL += RHS;
  இல்லையெனில்
    tcAddPart(U.pVal, RHS, get_இலக்கம்_சொற்கள்());
  திரும்பு clearUnusedBits();
}

/// Subtracts the RHS APInt from this APInt
/// @returns this, after subtraction
/// Subtraction assignment operator.
APInt& APInt::operator-=(மாறிலி APInt& RHS) {
  ப்ரதிஜ்ஞா(BitWidth == RHS.BitWidth && "Bit widths must be the same");
  எனில் (LLVM_LIKELY(isSingleWord()))
    U.VAL -= RHS.U.VAL;
  இல்லையெனில்
    tcSubtract(U.pVal, RHS.U.pVal, 0, get_இலக்கம்_சொற்கள்());
  திரும்பு clearUnusedBits();
}

APInt& APInt::operator-=(uint64_t RHS) {
  எனில் (LLVM_LIKELY(isSingleWord()))
    U.VAL -= RHS;
  இல்லையெனில்
    tcSubtractPart(U.pVal, RHS, get_இலக்கம்_சொற்கள்());
  திரும்பு clearUnusedBits();
}

APInt APInt::operator*(மாறிலி APInt& RHS) மாறிலி {
  ப்ரதிஜ்ஞா(BitWidth == RHS.BitWidth && "Bit widths must be the same");
  எனில் (LLVM_LIKELY(isSingleWord()))
    திரும்பு APInt(BitWidth, U.VAL * RHS.U.VAL, /*isSigned=*/பொய்,
                 /*implicitTrunc=*/உண்மை);

  APInt Result(getMemory(get_இலக்கம்_சொற்கள்()), getBitWidth());
  tcMultiply(Result.U.pVal, U.pVal, RHS.U.pVal, get_இலக்கம்_சொற்கள்());
  Result.clearUnusedBits();
  திரும்பு Result;
}

வெறுமை APInt::andAssignSlowCase(மாறிலி APInt &RHS) {
  WordType *dst = U.pVal, *rhs = RHS.U.pVal;
  க்கு (size_t i = 0, e = get_இலக்கம்_சொற்கள்(); i != e; ++i)
    dst[i] &= rhs[i];
}

வெறுமை APInt::orAssignSlowCase(மாறிலி APInt &RHS) {
  WordType *dst = U.pVal, *rhs = RHS.U.pVal;
  க்கு (size_t i = 0, e = get_இலக்கம்_சொற்கள்(); i != e; ++i)
    dst[i] |= rhs[i];
}

வெறுமை APInt::xorAssignSlowCase(மாறிலி APInt &RHS) {
  WordType *dst = U.pVal, *rhs = RHS.U.pVal;
  க்கு (size_t i = 0, e = get_இலக்கம்_சொற்கள்(); i != e; ++i)
    dst[i] ^= rhs[i];
}

APInt &APInt::operator*=(மாறிலி APInt &RHS) {
  *இது = *இது * RHS;
  திரும்பு *இது;
}

APInt& APInt::operator*=(uint64_t RHS) {
  எனில் (LLVM_LIKELY(isSingleWord())) {
    U.VAL *= RHS;
  } இல்லையெனில் {
    unsigned இலக்கம்_சொற்கள்_2 = get_இலக்கம்_சொற்கள்();
    tcMultiplyPart(U.pVal, U.pVal, RHS, 0, இலக்கம்_சொற்கள்_2, இலக்கம்_சொற்கள்_2, பொய்);
  }
  திரும்பு clearUnusedBits();
}

தருக்க APInt::equalSlowCase(மாறிலி APInt &RHS) மாறிலி {
  திரும்பு std::equal(U.pVal, U.pVal + get_இலக்கம்_சொற்கள்(), RHS.U.pVal);
}

முழு APInt::compare(மாறிலி APInt& RHS) மாறிலி {
  ப்ரதிஜ்ஞா(BitWidth == RHS.BitWidth && "Bit widths must be same for comparison");
  எனில் (LLVM_LIKELY(isSingleWord()))
    திரும்பு U.VAL < RHS.U.VAL ? -1 : U.VAL > RHS.U.VAL;

  திரும்பு tcCompare(U.pVal, RHS.U.pVal, get_இலக்கம்_சொற்கள்());
}

முழு APInt::compareSigned(மாறிலி APInt& RHS) மாறிலி {
  ப்ரதிஜ்ஞா(BitWidth == RHS.BitWidth && "Bit widths must be same for comparison");
  எனில் (LLVM_LIKELY(isSingleWord())) {
    int64_t lhsSext = SignExtend64(U.VAL, BitWidth);
    int64_t rhsSext = SignExtend64(RHS.U.VAL, BitWidth);
    திரும்பு lhsSext < rhsSext ? -1 : lhsSext > rhsSext;
  }

  தருக்க lhsNeg = isNegative();
  தருக்க rhsNeg = RHS.isNegative();

  // If the sign bits don't match, then (LHS < RHS) if LHS is negative
  எனில் (lhsNeg != rhsNeg)
    திரும்பு lhsNeg ? -1 : 1;

  // Otherwise we can just use an unsigned comparison, because even negative
  // numbers compare correctly this way if both have the same signed-ness.
  திரும்பு tcCompare(U.pVal, RHS.U.pVal, get_இலக்கம்_சொற்கள்());
}

வெறுமை APInt::setBitsSlowCase(unsigned loBit, unsigned hiBit) {
  unsigned loWord = whichWord(loBit);
  unsigned hiWord = whichWord(hiBit);

  // Create an initial mask for the low word with zeros below loBit.
  uint64_t loMask = WORDTYPE_MAX << whichBit(loBit);

  // If hiBit is not aligned, we need a high mask.
  unsigned hiShiftAmt = whichBit(hiBit);
  எனில் (hiShiftAmt != 0) {
    // Create a high mask with zeros above hiBit.
    uint64_t hiMask = WORDTYPE_MAX >> (APINT_BITS_PER_WORD - hiShiftAmt);
    // If loWord and hiWord are equal, then we combine the masks. Otherwise,
    // set the bits in hiWord.
    எனில் (hiWord == loWord)
      loMask &= hiMask;
    இல்லையெனில்
      U.pVal[hiWord] |= hiMask;
  }
  // Apply the mask to the low word.
  U.pVal[loWord] |= loMask;

  // Fill any words between loWord and hiWord with all ones.
  க்கு (unsigned word = loWord + 1; word < hiWord; ++word)
    U.pVal[word] = WORDTYPE_MAX;
}

வெறுமை APInt::clearBitsSlowCase(unsigned LoBit, unsigned HiBit) {
  unsigned LoWord = whichWord(LoBit);
  unsigned HiWord = whichWord(HiBit);

  // Create an initial mask for the low word with ones below loBit.
  uint64_t LoMask = ~(WORDTYPE_MAX << whichBit(LoBit));

  // If HiBit is not aligned, we need a high mask.
  unsigned HiShiftAmt = whichBit(HiBit);
  எனில் (HiShiftAmt != 0) {
    // Create a high mask with ones above HiBit.
    uint64_t HiMask = ~(WORDTYPE_MAX >> (APINT_BITS_PER_WORD - HiShiftAmt));
    // If LoWord and HiWord are equal, then we combine the masks. Otherwise,
    // clear the bits in HiWord.
    எனில் (HiWord == LoWord)
      LoMask |= HiMask;
    இல்லையெனில்
      U.pVal[HiWord] &= HiMask;
  }
  // Apply the mask to the low word.
  U.pVal[LoWord] &= LoMask;

  // Fill any words between LoWord and HiWord with all zeros.
  க்கு (unsigned Word = LoWord + 1; Word < HiWord; ++Word)
    U.pVal[Word] = 0;
}

// Complement a bignum in-place.
நிலையான வெறுமை tcComplement(APInt::WordType *dst, unsigned parts) {
  க்கு (unsigned i = 0; i < parts; i++)
    dst[i] = ~dst[i];
}

/// Toggle every bit to its opposite value.
வெறுமை APInt::flipAllBitsSlowCase() {
  tcComplement(U.pVal, get_இலக்கம்_சொற்கள்());
  clearUnusedBits();
}

/// Concatenate the bits from "NewLSB" onto the bottom of *this.  This is
/// equivalent to:
///   (this->zext(NewWidth) << NewLSB.getBitWidth()) | NewLSB.zext(NewWidth)
/// In the slow case, we know the result is large.
APInt APInt::concatSlowCase(மாறிலி APInt &NewLSB) மாறிலி {
  unsigned NewWidth = getBitWidth() + NewLSB.getBitWidth();
  APInt Result = NewLSB.zext(NewWidth);
  Result.செருகு_Bits(*இது, NewLSB.getBitWidth());
  திரும்பு Result;
}

/// Toggle a given bit to its opposite value whose position is given
/// as "bitPosition".
/// Toggles a given bit to its opposite value.
வெறுமை APInt::flipBit(unsigned bitPosition) {
  ப்ரதிஜ்ஞா(bitPosition < BitWidth && "Out of the bit-width range!");
  setBitVal(bitPosition, !(*இது)[bitPosition]);
}

வெறுமை APInt::செருகு_Bits(மாறிலி APInt &subBits, unsigned bitPosition) {
  unsigned subBitWidth = subBits.getBitWidth();
  ப்ரதிஜ்ஞா((subBitWidth + bitPosition) <= BitWidth && "Illegal bit insertion");

  // inserting no bits is a noop.
  எனில் (subBitWidth == 0)
    திரும்பு;

  // Insertion is a direct copy.
  எனில் (subBitWidth == BitWidth) {
    *இது = subBits;
    திரும்பு;
  }

  // Single word result can be done as a direct bitmask.
  எனில் (LLVM_LIKELY(isSingleWord())) {
    uint64_t mask = WORDTYPE_MAX >> (APINT_BITS_PER_WORD - subBitWidth);
    U.VAL &= ~(mask << bitPosition);
    U.VAL |= (subBits.U.VAL << bitPosition);
    திரும்பு;
  }

  unsigned loBit = whichBit(bitPosition);
  unsigned loWord = whichWord(bitPosition);
  unsigned hi1Word = whichWord(bitPosition + subBitWidth - 1);

  // Insertion within a single word can be done as a direct bitmask.
  எனில் (loWord == hi1Word) {
    uint64_t mask = WORDTYPE_MAX >> (APINT_BITS_PER_WORD - subBitWidth);
    U.pVal[loWord] &= ~(mask << loBit);
    U.pVal[loWord] |= (subBits.U.VAL << loBit);
    திரும்பு;
  }

  // Insert on word boundaries.
  எனில் (loBit == 0) {
    // Direct copy whole words.
    unsigned இலக்கம்_Whole_Sub_சொற்கள் = subBitWidth / APINT_BITS_PER_WORD;
    memcpy(U.pVal + loWord, subBits.getRawData(),
           இலக்கம்_Whole_Sub_சொற்கள் * APINT_WORD_SIZE);

    // Mask+insert remaining bits.
    unsigned remainingBits = subBitWidth % APINT_BITS_PER_WORD;
    எனில் (remainingBits != 0) {
      uint64_t mask = WORDTYPE_MAX >> (APINT_BITS_PER_WORD - remainingBits);
      U.pVal[hi1Word] &= ~mask;
      U.pVal[hi1Word] |= subBits.getWord(subBitWidth - 1);
    }
    திரும்பு;
  }

  // General case - set/clear individual bits in dst based on src.
  // TODO - there is scope for optimization here, but at the moment this code
  // path is barely used so prefer readability over performance.
  க்கு (unsigned i = 0; i != subBitWidth; ++i)
    setBitVal(bitPosition + i, subBits[i]);
}

வெறுமை APInt::செருகு_Bits(uint64_t subBits, unsigned bitPosition, unsigned இலக்கம்_Bits) {
  uint64_t maskBits = maskTrailingOnes<uint64_t>(இலக்கம்_Bits);
  subBits &= maskBits;
  எனில் (LLVM_LIKELY(isSingleWord())) {
    U.VAL &= ~(maskBits << bitPosition);
    U.VAL |= subBits << bitPosition;
    திரும்பு;
  }

  unsigned loBit = whichBit(bitPosition);
  unsigned loWord = whichWord(bitPosition);
  unsigned hiWord = whichWord(bitPosition + இலக்கம்_Bits - 1);
  எனில் (loWord == hiWord) {
    U.pVal[loWord] &= ~(maskBits << loBit);
    U.pVal[loWord] |= subBits << loBit;
    திரும்பு;
  }

  static_assert(8 * அளவு(WordType) <= 64, "This code assumes only two words affected");
  unsigned wordBits = 8 * அளவு(WordType);
  U.pVal[loWord] &= ~(maskBits << loBit);
  U.pVal[loWord] |= subBits << loBit;

  U.pVal[hiWord] &= ~(maskBits >> (wordBits - loBit));
  U.pVal[hiWord] |= subBits >> (wordBits - loBit);
}

APInt APInt::extractBits(unsigned இலக்கம்_Bits, unsigned bitPosition) மாறிலி {
  ப்ரதிஜ்ஞா(bitPosition < BitWidth && (இலக்கம்_Bits + bitPosition) <= BitWidth &&
         "Illegal bit extraction");

  எனில் (LLVM_LIKELY(isSingleWord()))
    திரும்பு APInt(இலக்கம்_Bits, U.VAL >> bitPosition, /*isSigned=*/பொய்,
                 /*implicitTrunc=*/உண்மை);

  unsigned loBit = whichBit(bitPosition);
  unsigned loWord = whichWord(bitPosition);
  unsigned hiWord = whichWord(bitPosition + இலக்கம்_Bits - 1);

  // Single word result extracting bits from a single word source.
  எனில் (loWord == hiWord)
    திரும்பு APInt(இலக்கம்_Bits, U.pVal[loWord] >> loBit, /*isSigned=*/பொய்,
                 /*implicitTrunc=*/உண்மை);

  // Extracting bits that start on a source word boundary can be done
  // as a fast memory copy.
  எனில் (loBit == 0)
    திரும்பு APInt(இலக்கம்_Bits, ArrayRef(U.pVal + loWord, 1 + hiWord - loWord));

  // General case - shift + copy source words directly into place.
  APInt Result(இலக்கம்_Bits, 0);
  unsigned இலக்கம்_Src_சொற்கள் = get_இலக்கம்_சொற்கள்();
  unsigned இலக்கம்_Dst_சொற்கள் = Result.get_இலக்கம்_சொற்கள்();

  uint64_t *DestPtr =
      LLVM_LIKELY(Result.isSingleWord()) ? &Result.U.VAL : Result.U.pVal;
  க்கு (unsigned word = 0; word < இலக்கம்_Dst_சொற்கள்; ++word) {
    uint64_t w0 = U.pVal[loWord + word];
    uint64_t w1 =
        (loWord + word + 1) < இலக்கம்_Src_சொற்கள் ? U.pVal[loWord + word + 1] : 0;
    DestPtr[word] = (w0 >> loBit) | (w1 << (APINT_BITS_PER_WORD - loBit));
  }

  திரும்பு Result.clearUnusedBits();
}

uint64_t APInt::extract_Bits_As_ZExt_மதிப்பு(unsigned இலக்கம்_Bits,
                                       unsigned bitPosition) மாறிலி {
  ப்ரதிஜ்ஞா(bitPosition < BitWidth && (இலக்கம்_Bits + bitPosition) <= BitWidth &&
         "Illegal bit extraction");
  ப்ரதிஜ்ஞா(இலக்கம்_Bits <= 64 && "Illegal bit extraction");

  uint64_t maskBits = maskTrailingOnes<uint64_t>(இலக்கம்_Bits);
  எனில் (LLVM_LIKELY(isSingleWord()))
    திரும்பு (U.VAL >> bitPosition) & maskBits;

  static_assert(APINT_BITS_PER_WORD >= 64,
                "This code assumes only two words affected");
  unsigned loBit = whichBit(bitPosition);
  unsigned loWord = whichWord(bitPosition);
  unsigned hiWord = whichWord(bitPosition + இலக்கம்_Bits - 1);
  எனில் (loWord == hiWord)
    திரும்பு (U.pVal[loWord] >> loBit) & maskBits;

  uint64_t retBits = U.pVal[loWord] >> loBit;
  retBits |= U.pVal[hiWord] << (APINT_BITS_PER_WORD - loBit);
  retBits &= maskBits;
  திரும்பு retBits;
}

unsigned APInt::getSufficientBitsNeeded(StringRef Str, uint8_t Radix) {
  ப்ரதிஜ்ஞா(!Str.empty() && "Invalid string length");
  size_t StrLen = Str.size();

  // Each computation below needs to know if it's negative.
  unsigned IsNegative = பொய்;
  எனில் (Str[0] == '-' || Str[0] == '+') {
    IsNegative = Str[0] == '-';
    StrLen--;
    ப்ரதிஜ்ஞா(StrLen && "String is only a sign, needs a value.");
  }

  // For radixes of power-of-two values, the bits required is accurately and
  // easily computed.
  எனில் (Radix == 2)
    திரும்பு StrLen + IsNegative;
  எனில் (Radix == 8)
    திரும்பு StrLen * 3 + IsNegative;
  எனில் (Radix == 16)
    திரும்பு StrLen * 4 + IsNegative;

  // Compute a sufficient number of bits that is always large enough but might
  // be too large. This avoids the assertion in the constructor. This
  // calculation doesn't work appropriately for the numbers 0-9, so just use 4
  // bits in that case.
  எனில் (Radix == 10)
    திரும்பு (StrLen == 1 ? 4 : StrLen * 64 / 18) + IsNegative;

  ப்ரதிஜ்ஞா(Radix == 36);
  திரும்பு (StrLen == 1 ? 7 : StrLen * 16 / 3) + IsNegative;
}

unsigned APInt::getBitsNeeded(StringRef str, uint8_t radix) {
  // Compute a sufficient number of bits that is always large enough but might
  // be too large.
  unsigned sufficient = getSufficientBitsNeeded(str, radix);

  // For bases 2, 8, and 16, the sufficient number of bits is exact and we can
  // return the value directly. For bases 10 and 36, we need to do extra work.
  எனில் (radix == 2 || radix == 8 || radix == 16)
    திரும்பு sufficient;

  // This is grossly inefficient but accurate. We could probably do something
  // with a computation of roughly slen*64/20 and then adjust by the value of
  // the first few digits. But, I'm not sure how accurate that could be.
  size_t slen = str.size();

  // Each computation below needs to know if it's negative.
  StringRef::iterator p = str.begin();
  unsigned isNegative = *p == '-';
  எனில் (*p == '-' || *p == '+') {
    p++;
    slen--;
    ப்ரதிஜ்ஞா(slen && "String is only a sign, needs a value.");
  }


  // Convert to the actual binary value.
  APInt tmp(sufficient, StringRef(p, slen), radix);

  // Compute how many bits are required. If the log is infinite, assume we need
  // just bit. If the log is exact and value is negative, then the value is
  // MinSignedValue with (log + 1) bits.
  unsigned log = tmp.logBase2();
  எனில் (log == (unsigned)-1) {
    திரும்பு isNegative + 1;
  } இல்லையெனில் எனில் (isNegative && tmp.isPowerOf2()) {
    திரும்பு isNegative + log;
  } இல்லையெனில் {
    திரும்பு isNegative + log + 1;
  }
}

hash_code llvm::hash_மதிப்பு(மாறிலி APInt &Arg) {
  எனில் (LLVM_LIKELY(Arg.isSingleWord()))
    திரும்பு hash_combine(Arg.BitWidth, Arg.U.VAL);

  திரும்பு hash_combine(
      Arg.BitWidth,
      hash_combine_range(Arg.U.pVal, Arg.U.pVal + Arg.get_இலக்கம்_சொற்கள்()));
}

unsigned DenseMapInfo<APInt, வெறுமை>::get_Hash_மதிப்பு(மாறிலி APInt &சாவி) {
  திரும்பு static_cast<unsigned>(hash_மதிப்பு(சாவி));
}

தருக்க APInt::isSplat(unsigned SplatSizeInBits) மாறிலி {
  ப்ரதிஜ்ஞா(getBitWidth() % SplatSizeInBits == 0 &&
         "SplatSizeInBits must divide width!");
  // We can check that all parts of an integer are equal by making use of a
  // little trick: rotate and check if it's still the same value.
  திரும்பு *இது == rotl(SplatSizeInBits);
}

/// This function returns the high "numBits" bits of this APInt.
APInt APInt::getHiBits(unsigned இலக்கம்_Bits) மாறிலி {
  திரும்பு இது->lshr(BitWidth - இலக்கம்_Bits);
}

/// This function returns the low "numBits" bits of this APInt.
APInt APInt::getLoBits(unsigned இலக்கம்_Bits) மாறிலி {
  APInt Result(get_கீழ்_Bits_Set(BitWidth, இலக்கம்_Bits));
  Result &= *இது;
  திரும்பு Result;
}

/// Return a value containing V broadcasted over NewLen bits.
APInt APInt::getSplat(unsigned NewLen, மாறிலி APInt &V) {
  ப்ரதிஜ்ஞா(NewLen >= V.getBitWidth() && "Can't splat to smaller bit width!");

  APInt Val = V.zext(NewLen);
  க்கு (unsigned I = V.getBitWidth(); I < NewLen; I <<= 1)
    Val |= Val << I;

  திரும்பு Val;
}

unsigned APInt::மொத்தம்_Leading_Zeros_Slow_Case() மாறிலி {
  unsigned மொத்தம்_2 = 0;
  க்கு (முழு i = get_இலக்கம்_சொற்கள்() - 1; i >= 0; --i) {
    uint64_t V = U.pVal[i];
    எனில் (V == 0)
      மொத்தம்_2 += APINT_BITS_PER_WORD;
    இல்லையெனில் {
      மொத்தம்_2 += llvm::countl_zero(V);
      நிறுத்து;
    }
  }
  // Adjust for unused bits in the most significant word (they are zero).
  unsigned Mod = BitWidth % APINT_BITS_PER_WORD;
  மொத்தம்_2 -= Mod > 0 ? APINT_BITS_PER_WORD - Mod : 0;
  திரும்பு மொத்தம்_2;
}

unsigned APInt::மொத்தம்_Leading_Ones_Slow_Case() மாறிலி {
  unsigned உயர்வு_Word_Bits = BitWidth % APINT_BITS_PER_WORD;
  unsigned shift;
  எனில் (!உயர்வு_Word_Bits) {
    உயர்வு_Word_Bits = APINT_BITS_PER_WORD;
    shift = 0;
  } இல்லையெனில் {
    shift = APINT_BITS_PER_WORD - உயர்வு_Word_Bits;
  }
  முழு i = get_இலக்கம்_சொற்கள்() - 1;
  unsigned மொத்தம்_2 = llvm::countl_one(U.pVal[i] << shift);
  எனில் (மொத்தம்_2 == உயர்வு_Word_Bits) {
    க்கு (i--; i >= 0; --i) {
      எனில் (U.pVal[i] == WORDTYPE_MAX)
        மொத்தம்_2 += APINT_BITS_PER_WORD;
      இல்லையெனில் {
        மொத்தம்_2 += llvm::countl_one(U.pVal[i]);
        நிறுத்து;
      }
    }
  }
  திரும்பு மொத்தம்_2;
}

unsigned APInt::மொத்தம்_Trailing_Zeros_Slow_Case() மாறிலி {
  unsigned மொத்தம்_2 = 0;
  unsigned i = 0;
  க்கு (; i < get_இலக்கம்_சொற்கள்() && U.pVal[i] == 0; ++i)
    மொத்தம்_2 += APINT_BITS_PER_WORD;
  எனில் (i < get_இலக்கம்_சொற்கள்())
    மொத்தம்_2 += llvm::countr_zero(U.pVal[i]);
  திரும்பு std::min(மொத்தம்_2, BitWidth);
}

unsigned APInt::மொத்தம்_Trailing_Ones_Slow_Case() மாறிலி {
  unsigned மொத்தம்_2 = 0;
  unsigned i = 0;
  க்கு (; i < get_இலக்கம்_சொற்கள்() && U.pVal[i] == WORDTYPE_MAX; ++i)
    மொத்தம்_2 += APINT_BITS_PER_WORD;
  எனில் (i < get_இலக்கம்_சொற்கள்())
    மொத்தம்_2 += llvm::countr_one(U.pVal[i]);
  ப்ரதிஜ்ஞா(மொத்தம்_2 <= BitWidth);
  திரும்பு மொத்தம்_2;
}

unsigned APInt::மொத்தம்_Population_Slow_Case() மாறிலி {
  unsigned மொத்தம்_2 = 0;
  க்கு (unsigned i = 0; i < get_இலக்கம்_சொற்கள்(); ++i)
    மொத்தம்_2 += llvm::popcount(U.pVal[i]);
  திரும்பு மொத்தம்_2;
}

தருக்க APInt::isPowerOf2SlowCase() மாறிலி {
  unsigned மொத்தம்_2 = 0;
  க்கு (unsigned i = 0; i < get_இலக்கம்_சொற்கள்(); ++i) {
    மொத்தம்_2 += llvm::popcount(U.pVal[i]);
    எனில் (மொத்தம்_2 > 1)
      திரும்பு பொய்;
  }
  திரும்பு மொத்தம்_2 == 1;
}

தருக்க APInt::intersectsSlowCase(மாறிலி APInt &RHS) மாறிலி {
  க்கு (unsigned i = 0, e = get_இலக்கம்_சொற்கள்(); i != e; ++i)
    எனில் ((U.pVal[i] & RHS.U.pVal[i]) != 0)
      திரும்பு உண்மை;

  திரும்பு பொய்;
}

தருக்க APInt::isSubsetOfSlowCase(மாறிலி APInt &RHS) மாறிலி {
  க்கு (unsigned i = 0, e = get_இலக்கம்_சொற்கள்(); i != e; ++i)
    எனில் ((U.pVal[i] & ~RHS.U.pVal[i]) != 0)
      திரும்பு பொய்;

  திரும்பு உண்மை;
}

தருக்க APInt::isInverseOfSlowCase(மாறிலி APInt &RHS) மாறிலி {
  மாறிலி unsigned Last = get_இலக்கம்_சொற்கள்() - 1;
  க்கு (unsigned I = 0; I != Last; ++I)
    எனில் ((U.pVal[I] ^ RHS.U.pVal[I]) != WORDTYPE_MAX)
      திரும்பு பொய்;

  unsigned TailBits = BitWidth - Last * APINT_BITS_PER_WORD;
  WordType TailMask = llvm::maskTrailingOnes<WordType>(TailBits);
  திரும்பு (U.pVal[Last] ^ RHS.U.pVal[Last]) == TailMask;
}

APInt APInt::byteSwap() மாறிலி {
  ப்ரதிஜ்ஞா(BitWidth >= 16 && BitWidth % 8 == 0 && "Cannot byteswap!");
  எனில் (BitWidth == 16)
    திரும்பு APInt(BitWidth, llvm::byteswap<uint16_t>(U.VAL));
  எனில் (BitWidth == 32)
    திரும்பு APInt(BitWidth, llvm::byteswap<uint32_t>(U.VAL));
  எனில் (BitWidth <= 64) {
    uint64_t Tmp1 = llvm::byteswap<uint64_t>(U.VAL);
    Tmp1 >>= (64 - BitWidth);
    திரும்பு APInt(BitWidth, Tmp1);
  }

  APInt Result(get_இலக்கம்_சொற்கள்() * APINT_BITS_PER_WORD, 0);
  க்கு (unsigned I = 0, N = get_இலக்கம்_சொற்கள்(); I != N; ++I)
    Result.U.pVal[I] = llvm::byteswap<uint64_t>(U.pVal[N - I - 1]);
  எனில் (Result.BitWidth != BitWidth) {
    Result.lshrInPlace(Result.BitWidth - BitWidth);
    Result.BitWidth = BitWidth;
  }
  திரும்பு Result;
}

APInt APInt::reverseBits() மாறிலி {
  எனில் (LLVM_LIKELY(isSingleWord())) {
    தேர்வு (BitWidth) {
    நிலை 64:
      திரும்பு APInt(BitWidth, llvm::reverseBits<uint64_t>(U.VAL));
    நிலை 32:
      திரும்பு APInt(BitWidth, llvm::reverseBits<uint32_t>(U.VAL));
    நிலை 16:
      திரும்பு APInt(BitWidth, llvm::reverseBits<uint16_t>(U.VAL));
    நிலை 8:
      திரும்பு APInt(BitWidth, llvm::reverseBits<uint8_t>(U.VAL));
    நிலை 1: // fallthrough
    நிலை 0:
      திரும்பு *இது;
    இயல்பு:
      திரும்பு APInt(BitWidth,
                   llvm::reverseBits<uint64_t>(U.VAL) >> (64 - BitWidth));
    }
  }

  APInt Result(BitWidth, 0);
  unsigned இலக்கம்_சொற்கள்_2 = get_இலக்கம்_சொற்கள்();
  unsigned ExcessBits = இலக்கம்_சொற்கள்_2 * APINT_BITS_PER_WORD - BitWidth;
  எனில் (ExcessBits == 0) {
    // Fast path. No cross-word shift needed.
    க்கு (unsigned I = 0; I < இலக்கம்_சொற்கள்_2; ++I)
      Result.U.pVal[I] = llvm::reverseBits<uint64_t>(U.pVal[இலக்கம்_சொற்கள்_2 - 1 - I]);
    திரும்பு Result;
  }
  // Holds reversed bits of the previous (more significant) word.
  uint64_t PrevRev = llvm::reverseBits<uint64_t>(U.pVal[இலக்கம்_சொற்கள்_2 - 1]);
  க்கு (unsigned I = 0; I < இலக்கம்_சொற்கள்_2 - 1; ++I) {
    uint64_t CurrRev = llvm::reverseBits<uint64_t>(U.pVal[இலக்கம்_சொற்கள்_2 - 2 - I]);
    Result.U.pVal[I] = (PrevRev >> ExcessBits) | (CurrRev << (64 - ExcessBits));
    PrevRev = CurrRev;
  }
  Result.U.pVal[இலக்கம்_சொற்கள்_2 - 1] = PrevRev >> ExcessBits;
  திரும்பு Result;
}

APInt llvm::APIntOps::Greatest_Common_வகுஎண்(APInt A, APInt B, தருக்க IsSigned) {
  // Take absolute value if IsSigned.
  எனில் (IsSigned) {
    A = A.abs();
    B = B.abs();
  }

  // Fast-path a common case.
  எனில் (A == B) திரும்பு A;

  // Corner cases: if either operand is zero, the other is the gcd.
  எனில் (!A) திரும்பு B;
  எனில் (!B) திரும்பு A;

  // Count common powers of 2 and remove all other powers of 2.
  unsigned Pow2;
  {
    unsigned Pow2_A = A.countr_zero();
    unsigned Pow2_B = B.countr_zero();
    எனில் (Pow2_A > Pow2_B) {
      A.lshrInPlace(Pow2_A - Pow2_B);
      Pow2 = Pow2_B;
    } இல்லையெனில் எனில் (Pow2_B > Pow2_A) {
      B.lshrInPlace(Pow2_B - Pow2_A);
      Pow2 = Pow2_A;
    } இல்லையெனில் {
      Pow2 = Pow2_A;
    }
  }

  // Both operands are odd multiples of 2^Pow_2:
  //
  //   gcd(a, b) = gcd(|a - b| / 2^i, min(a, b))
  //
  // This is a modified version of Stein's algorithm, taking advantage of
  // efficient countTrailingZeros().
  போது (A != B) {
    எனில் (A.ugt(B)) {
      A -= B;
      A.lshrInPlace(A.countr_zero() - Pow2);
    } இல்லையெனில் {
      B -= A;
      B.lshrInPlace(B.countr_zero() - Pow2);
    }
  }

  திரும்பு A;
}

APInt llvm::APIntOps::RoundDoubleToAPInt(இரட்டை Double, unsigned width) {
  uint64_t I = bit_cast<uint64_t>(Double);

  // Get the sign bit from the highest order bit
  தருக்க isNeg = I >> 63;

  // Get the 11-bit exponent and adjust for the 1023 bit bias
  int64_t exp = ((I >> 52) & 0x7ff) - 1023;

  // If the exponent is negative, the value is < 0 so just return 0.
  எனில் (exp < 0)
    திரும்பு APInt(width, 0u);

  // Extract the mantissa by clearing the top 12 bits (sign + exponent).
  uint64_t mantissa = (I & (~0ULL >> 12)) | 1ULL << 52;

  // If the exponent doesn't shift all bits out of the mantissa
  எனில் (exp < 52)
    திரும்பு isNeg ? -APInt(width, mantissa >> (52 - exp)) :
                    APInt(width, mantissa >> (52 - exp));

  // If the client didn't provide enough bits for us to shift the mantissa into
  // then the result is undefined, just return 0
  எனில் (width <= exp - 52)
    திரும்பு APInt(width, 0);

  // Otherwise, we have to shift the mantissa bits up to the right location
  APInt Tmp(width, mantissa);
  Tmp <<= (unsigned)exp - 52;
  திரும்பு isNeg ? -Tmp : Tmp;
}

/// This function converts this APInt to a double.
/// The layout for double is as following (IEEE Standard 754):
///  --------------------------------------
/// |  Sign    Exponent    Fraction    Bias |
/// |-------------------------------------- |
/// |  1[63]   11[62-52]   52[51-00]   1023 |
///  --------------------------------------
இரட்டை APInt::roundToDouble(தருக்க isSigned) மாறிலி {
  // Handle the simple case where the value is contained in one uint64_t.
  // It is wrong to optimize getWord(0) to VAL; there might be more than one word.
  எனில் (LLVM_LIKELY(isSingleWord() || getActiveBits() <= APINT_BITS_PER_WORD)) {
    எனில் (isSigned) {
      int64_t sext = SignExtend64(getWord(0), BitWidth);
      திரும்பு இரட்டை(sext);
    }
    திரும்பு இரட்டை(getWord(0));
  }

  // Determine if the value is negative.
  தருக்க isNeg = isSigned ? (*இது)[BitWidth-1] : பொய்;

  // Construct the absolute value if we're negative.
  APInt Tmp(isNeg ? -(*இது) : (*இது));

  // Figure out how many bits we're using.
  unsigned n = Tmp.getActiveBits();

  // The exponent (without bias normalization) is just the number of bits
  // we are using. Note that the sign bit is gone since we constructed the
  // absolute value.
  uint64_t exp = n;

  // Return infinity for exponent overflow
  எனில் (exp > 1023) {
    எனில் (!isSigned || !isNeg)
      திரும்பு std::numeric_limits<இரட்டை>::infinity();
    இல்லையெனில்
      திரும்பு -std::numeric_limits<இரட்டை>::infinity();
  }
  exp += 1023; // Increment for 1023 bias

  // Number of bits in mantissa is 52. To obtain the mantissa value, we must
  // extract the high 52 bits from the correct words in pVal.
  uint64_t mantissa;
  unsigned hiWord = whichWord(n-1);
  எனில் (hiWord == 0) {
    mantissa = Tmp.U.pVal[0];
    எனில் (n > 52)
      mantissa >>= n - 52; // shift down, we want the top 52 bits.
  } இல்லையெனில் {
    ப்ரதிஜ்ஞா(hiWord > 0 && "huh?");
    uint64_t hibits = Tmp.U.pVal[hiWord] << (52 - n % APINT_BITS_PER_WORD);
    uint64_t lobits = Tmp.U.pVal[hiWord-1] >> (11 + n % APINT_BITS_PER_WORD);
    mantissa = hibits | lobits;
  }

  // The leading bit of mantissa is implicit, so get rid of it.
  uint64_t sign = isNeg ? (1ULL << (APINT_BITS_PER_WORD - 1)) : 0;
  uint64_t I = sign | (exp << 52) | mantissa;
  திரும்பு bit_cast<இரட்டை>(I);
}

// Truncate to new width.
APInt APInt::trunc(unsigned width) மாறிலி {
  ப்ரதிஜ்ஞா(width <= BitWidth && "Invalid APInt Truncate request");

  எனில் (width <= APINT_BITS_PER_WORD)
    திரும்பு APInt(width, getRawData()[0], /*isSigned=*/பொய்,
                 /*implicitTrunc=*/உண்மை);

  எனில் (width == BitWidth)
    திரும்பு *இது;

  APInt Result(getMemory(get_இலக்கம்_சொற்கள்(width)), width);

  // Copy full words.
  unsigned i;
  க்கு (i = 0; i != width / APINT_BITS_PER_WORD; i++)
    Result.U.pVal[i] = U.pVal[i];

  // Truncate and copy any partial word.
  unsigned bits = (0 - width) % APINT_BITS_PER_WORD;
  எனில் (bits != 0)
    Result.U.pVal[i] = U.pVal[i] << bits >> bits;

  திரும்பு Result;
}

// Truncate to new width with unsigned saturation.
APInt APInt::truncUSat(unsigned width) மாறிலி {
  ப்ரதிஜ்ஞா(width <= BitWidth && "Invalid APInt Truncate request");

  // Can we just losslessly truncate it?
  எனில் (isIntN(width))
    திரும்பு trunc(width);
  // If not, then just return the new limit.
  திரும்பு APInt::get_Max_மதிப்பு(width);
}

// Truncate to new width with signed saturation to signed result.
APInt APInt::truncSSat(unsigned width) மாறிலி {
  ப்ரதிஜ்ஞா(width <= BitWidth && "Invalid APInt Truncate request");

  // Can we just losslessly truncate it?
  எனில் (isSignedIntN(width))
    திரும்பு trunc(width);
  // If not, then just return the new limits.
  திரும்பு isNegative() ? APInt::get_Signed_Min_மதிப்பு(width)
                      : APInt::get_Signed_Max_மதிப்பு(width);
}

// Truncate to new width with signed saturation to unsigned result.
APInt APInt::truncSSatU(unsigned width) மாறிலி {
  ப்ரதிஜ்ஞா(width <= BitWidth && "Invalid APInt Truncate request");

  // Can we just losslessly truncate it?
  எனில் (isIntN(width))
    திரும்பு trunc(width);
  // If not, then just return the new limits.
  திரும்பு isNegative() ? APInt::getZero(width) : APInt::get_Max_மதிப்பு(width);
}

// Sign extend to a new width.
APInt APInt::sext(unsigned Width) மாறிலி {
  ப்ரதிஜ்ஞா(Width >= BitWidth && "Invalid APInt SignExtend request");

  எனில் (Width <= APINT_BITS_PER_WORD)
    திரும்பு APInt(Width, SignExtend64(U.VAL, BitWidth), /*isSigned=*/உண்மை);

  எனில் (Width == BitWidth)
    திரும்பு *இது;

  APInt Result(getMemory(get_இலக்கம்_சொற்கள்(Width)), Width);

  // Copy words.
  std::memcpy(Result.U.pVal, getRawData(), get_இலக்கம்_சொற்கள்() * APINT_WORD_SIZE);

  // Sign extend the last word since there may be unused bits in the input.
  Result.U.pVal[get_இலக்கம்_சொற்கள்() - 1] =
      SignExtend64(Result.U.pVal[get_இலக்கம்_சொற்கள்() - 1],
                   ((BitWidth - 1) % APINT_BITS_PER_WORD) + 1);

  // Fill with sign bits.
  std::memset(Result.U.pVal + get_இலக்கம்_சொற்கள்(), isNegative() ? -1 : 0,
              (Result.get_இலக்கம்_சொற்கள்() - get_இலக்கம்_சொற்கள்()) * APINT_WORD_SIZE);
  Result.clearUnusedBits();
  திரும்பு Result;
}

//  Zero extend to a new width.
APInt APInt::zext(unsigned width) மாறிலி {
  ப்ரதிஜ்ஞா(width >= BitWidth && "Invalid APInt ZeroExtend request");

  எனில் (width <= APINT_BITS_PER_WORD)
    திரும்பு APInt(width, U.VAL);

  எனில் (width == BitWidth)
    திரும்பு *இது;

  APInt Result(getMemory(get_இலக்கம்_சொற்கள்(width)), width);

  // Copy words.
  std::memcpy(Result.U.pVal, getRawData(), get_இலக்கம்_சொற்கள்() * APINT_WORD_SIZE);

  // Zero remaining words.
  std::memset(Result.U.pVal + get_இலக்கம்_சொற்கள்(), 0,
              (Result.get_இலக்கம்_சொற்கள்() - get_இலக்கம்_சொற்கள்()) * APINT_WORD_SIZE);

  திரும்பு Result;
}

APInt APInt::zextOrTrunc(unsigned width) மாறிலி {
  எனில் (BitWidth < width)
    திரும்பு zext(width);
  எனில் (BitWidth > width)
    திரும்பு trunc(width);
  திரும்பு *இது;
}

APInt APInt::sextOrTrunc(unsigned width) மாறிலி {
  எனில் (BitWidth < width)
    திரும்பு sext(width);
  எனில் (BitWidth > width)
    திரும்பு trunc(width);
  திரும்பு *இது;
}

/// Arithmetic right-shift this APInt by shiftAmt.
/// Arithmetic right-shift function.
வெறுமை APInt::ashrInPlace(மாறிலி APInt &shiftAmt) {
  ashrInPlace((unsigned)shiftAmt.get_Limited_மதிப்பு(BitWidth));
}

/// Arithmetic right-shift this APInt by shiftAmt.
/// Arithmetic right-shift function.
வெறுமை APInt::ashrSlowCase(unsigned ShiftAmt) {
  // Don't bother performing a no-op shift.
  எனில் (!ShiftAmt)
    திரும்பு;

  // Save the original sign bit for later.
  தருக்க Negative = isNegative();

  // WordShift is the inter-part shift; BitShift is intra-part shift.
  unsigned WordShift = ShiftAmt / APINT_BITS_PER_WORD;
  unsigned BitShift = ShiftAmt % APINT_BITS_PER_WORD;

  unsigned சொற்கள்_To_Move = get_இலக்கம்_சொற்கள்() - WordShift;
  எனில் (சொற்கள்_To_Move != 0) {
    // Sign extend the last word to fill in the unused bits.
    U.pVal[get_இலக்கம்_சொற்கள்() - 1] = SignExtend64(
        U.pVal[get_இலக்கம்_சொற்கள்() - 1], ((BitWidth - 1) % APINT_BITS_PER_WORD) + 1);

    // Fastpath for moving by whole words.
    எனில் (BitShift == 0) {
      std::memmove(U.pVal, U.pVal + WordShift, சொற்கள்_To_Move * APINT_WORD_SIZE);
    } இல்லையெனில் {
      // Move the words containing significant bits.
      க்கு (unsigned i = 0; i != சொற்கள்_To_Move - 1; ++i)
        U.pVal[i] = (U.pVal[i + WordShift] >> BitShift) |
                    (U.pVal[i + WordShift + 1] << (APINT_BITS_PER_WORD - BitShift));

      // Handle the last word which has no high bits to copy. Use an arithmetic
      // shift to preserve the sign bit.
      U.pVal[சொற்கள்_To_Move - 1] =
          (int64_t)U.pVal[WordShift + சொற்கள்_To_Move - 1] >> BitShift;
    }
  }

  // Fill in the remainder based on the original sign.
  std::memset(U.pVal + சொற்கள்_To_Move, Negative ? -1 : 0,
              WordShift * APINT_WORD_SIZE);
  clearUnusedBits();
}

/// Logical right-shift this APInt by shiftAmt.
/// Logical right-shift function.
வெறுமை APInt::lshrInPlace(மாறிலி APInt &shiftAmt) {
  lshrInPlace((unsigned)shiftAmt.get_Limited_மதிப்பு(BitWidth));
}

/// Logical right-shift this APInt by shiftAmt.
/// Logical right-shift function.
வெறுமை APInt::lshrSlowCase(unsigned ShiftAmt) {
  tc_Shift_வலது(U.pVal, get_இலக்கம்_சொற்கள்(), ShiftAmt);
}

/// Left-shift this APInt by shiftAmt.
/// Left-shift function.
APInt &APInt::operator<<=(மாறிலி APInt &shiftAmt) {
  // It's undefined behavior in C to shift by BitWidth or greater.
  *இது <<= (unsigned)shiftAmt.get_Limited_மதிப்பு(BitWidth);
  திரும்பு *இது;
}

வெறுமை APInt::shlSlowCase(unsigned ShiftAmt) {
  tc_Shift_இடது(U.pVal, get_இலக்கம்_சொற்கள்(), ShiftAmt);
  clearUnusedBits();
}

// Calculate the rotate amount modulo the bit width.
நிலையான unsigned rotateModulo(unsigned BitWidth, மாறிலி APInt &rotateAmt) {
  எனில் (LLVM_UNLIKELY(BitWidth == 0))
    திரும்பு 0;
  unsigned rotBitWidth = rotateAmt.getBitWidth();
  APInt rot = rotateAmt;
  எனில் (rotBitWidth < BitWidth) {
    // Extend the rotate APInt, so that the urem doesn't divide by 0.
    // e.g. APInt(1, 32) would give APInt(1, 0).
    rot = rotateAmt.zext(BitWidth);
  }
  rot = rot.urem(APInt(rot.getBitWidth(), BitWidth));
  திரும்பு rot.get_Limited_மதிப்பு(BitWidth);
}

APInt APInt::rotl(மாறிலி APInt &rotateAmt) மாறிலி {
  திரும்பு rotl(rotateModulo(BitWidth, rotateAmt));
}

APInt APInt::rotl(unsigned rotateAmt) மாறிலி {
  எனில் (LLVM_UNLIKELY(BitWidth == 0))
    திரும்பு *இது;
  rotateAmt %= BitWidth;
  எனில் (rotateAmt == 0)
    திரும்பு *இது;
  திரும்பு shl(rotateAmt) | lshr(BitWidth - rotateAmt);
}

APInt APInt::rotr(மாறிலி APInt &rotateAmt) மாறிலி {
  திரும்பு rotr(rotateModulo(BitWidth, rotateAmt));
}

APInt APInt::rotr(unsigned rotateAmt) மாறிலி {
  எனில் (BitWidth == 0)
    திரும்பு *இது;
  rotateAmt %= BitWidth;
  எனில் (rotateAmt == 0)
    திரும்பு *இது;
  திரும்பு lshr(rotateAmt) | shl(BitWidth - rotateAmt);
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
unsigned APInt::nearestLogBase2() மாறிலி {
  // Special case when we have a bitwidth of 1. If VAL is 1, then we
  // get 0. If VAL is 0, we get WORDTYPE_MAX which gets truncated to
  // UINT32_MAX.
  எனில் (BitWidth == 1)
    திரும்பு U.VAL - 1;

  // Handle the zero case.
  எனில் (isZero())
    திரும்பு UINT32_MAX;

  // The non-zero case is handled by computing:
  //
  //   nearestLogBase2(x) = logBase2(x) + x[logBase2(x)-1].
  //
  // where x[i] is referring to the value of the ith bit of x.
  unsigned lg = logBase2();
  திரும்பு lg + unsigned((*இது)[lg - 1]);
}

// Square Root - this method computes and returns the square root of "this".
// Three mechanisms are used for computation. For small values (<= 5 bits),
// a table lookup is done. This gets some performance for common cases. For
// values using less than 52 bits, the value is converted to double and then
// the libc sqrt function is called. The result is rounded and then converted
// back to a uint64_t which is then used to construct the result. Finally,
// the Babylonian method for computing square roots is used.
APInt APInt::sqrtFloor() மாறிலி {

  // Determine the magnitude of the value.
  unsigned magnitude = getActiveBits();

  // Use a fast table for some small values. This also gets rid of some
  // rounding errors in libc sqrt for small values.
  எனில் (magnitude <= 5) {
    நிலையான மாறிலி uint8_t results[32] = {
        /*     0 */ 0,
        /*  1- 3 */ 1, 1, 1,
        /*  4- 8 */ 2, 2, 2, 2, 2,
        /*  9-15 */ 3, 3, 3, 3, 3, 3, 3,
        /* 16-24 */ 4, 4, 4, 4, 4, 4, 4, 4, 4,
        /* 25-31 */ 5, 5, 5, 5, 5, 5, 5,
    };
    திரும்பு APInt(BitWidth,
                 results[(LLVM_LIKELY(isSingleWord()) ? U.VAL : U.pVal[0])]);
  }

  // If the magnitude of the value fits in less than 52 bits (the precision of
  // an IEEE double precision floating point value), then we can use the
  // libc sqrt function which will probably use a hardware sqrt computation.
  // This should be faster than the algorithm below.
  எனில் (magnitude < 52) {
    திரும்பு APInt(BitWidth,
                 uint64_t(::floor(::sqrt(இரட்டை(
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
  க்கு (;; i += 2, testy = testy.shl(2))
    எனில் (i >= nbits || இது->ule(testy)) {
      x_old = x_old.shl(i / 2);
      நிறுத்து;
    }

  // Use the Babylonian method to arrive at the integer square root:
  க்கு (;;) {
    x_new = (இது->udiv(x_old) + x_old).udiv(two);
    எனில் (x_old.ule(x_new))
      நிறுத்து;
    x_old = x_new;
  }
  திரும்பு x_old;
}

/// \returns the multiplicative inverse of an odd APInt modulo 2^BitWidth.
APInt APInt::multiplicativeInverse() மாறிலி {
  ப்ரதிஜ்ஞா((*இது)[0] &&
         "multiplicative inverse is only defined for odd numbers!");

  // Use Newton's method.
  APInt Factor = *இது;
  APInt T;
  போது (!(T = *இது * Factor).isOne())
    Factor *= 2 - std::move(T);
  திரும்பு Factor;
}

/// Implementation of Knuth's Algorithm D (Division of nonnegative integers)
/// from "Art of Computer Programming, Volume 2", section 4.3.1, p. 272. The
/// variables here have the same names as in the algorithm. Comments explain
/// the algorithm and any deviation from it.
நிலையான வெறுமை KnuthDiv(uint32_t *u, uint32_t *v, uint32_t *q, uint32_t* r,
                     unsigned m, unsigned n) {
  ப்ரதிஜ்ஞா(u && "Must provide dividend");
  ப்ரதிஜ்ஞா(v && "Must provide divisor");
  ப்ரதிஜ்ஞா(q && "Must provide quotient");
  ப்ரதிஜ்ஞா(u != v && u != q && v != q && "Must use different memory");
  ப்ரதிஜ்ஞா(n>1 && "n must be > 1");

  // b denotes the base of the number system. In our case b is 2^32.
  மாறிலி uint64_t b = uint64_t(1) << 32;

// The DEBUG macros here tend to be spam in the debug output if you're not
// debugging this code. Disable them unless KNUTH_DEBUG is defined.
#ifdef KNUTH_DEBUG
#define DEBUG_KNUTH(X) LLVM_DEBUG(X)
#இல்லையெனில்
#define DEBUG_KNUTH(X) செய் {} போது(பொய்)
#endif

  DEBUG_KNUTH(dbgs() << "KnuthDiv: m=" << m << " n=" << n << '\n');
  DEBUG_KNUTH(dbgs() << "KnuthDiv: original:");
  DEBUG_KNUTH(க்கு (முழு i = m + n; i >= 0; i--) dbgs() << " " << u[i]);
  DEBUG_KNUTH(dbgs() << " by");
  DEBUG_KNUTH(க்கு (முழு i = n; i > 0; i--) dbgs() << " " << v[i - 1]);
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
  எனில் (shift) {
    க்கு (unsigned i = 0; i < m+n; ++i) {
      uint32_t u_tmp = u[i] >> (32 - shift);
      u[i] = (u[i] << shift) | u_carry;
      u_carry = u_tmp;
    }
    க்கு (unsigned i = 0; i < n; ++i) {
      uint32_t v_tmp = v[i] >> (32 - shift);
      v[i] = (v[i] << shift) | v_carry;
      v_carry = v_tmp;
    }
  }
  u[m+n] = u_carry;

  DEBUG_KNUTH(dbgs() << "KnuthDiv:   normal:");
  DEBUG_KNUTH(க்கு (முழு i = m + n; i >= 0; i--) dbgs() << " " << u[i]);
  DEBUG_KNUTH(dbgs() << " by");
  DEBUG_KNUTH(க்கு (முழு i = n; i > 0; i--) dbgs() << " " << v[i - 1]);
  DEBUG_KNUTH(dbgs() << '\n');

  // D2. [Initialize j.]  Set j to m. This is the loop counter over the places.
  முழு j = m;
  செய் {
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
    எனில் (qp == b || qp*v[n-2] > b*rp + u[j+n-2]) {
      qp--;
      rp += v[n-1];
      எனில் (rp < b && (qp == b || qp*v[n-2] > b*rp + u[j+n-2]))
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
    க்கு (unsigned i = 0; i < n; ++i) {
      uint64_t p = qp * uint64_t(v[i]);
      int64_t subres = int64_t(u[j+i]) - borrow - Lo_32(p);
      u[j+i] = Lo_32(subres);
      borrow = Hi_32(p) - Hi_32(subres);
      DEBUG_KNUTH(dbgs() << "KnuthDiv: u[j+i] = " << u[j + i]
                        << ", borrow = " << borrow << '\n');
    }
    தருக்க isNeg = u[j+n] < borrow;
    u[j+n] -= Lo_32(borrow);

    DEBUG_KNUTH(dbgs() << "KnuthDiv: after subtraction:");
    DEBUG_KNUTH(க்கு (முழு i = m + n; i >= 0; i--) dbgs() << " " << u[i]);
    DEBUG_KNUTH(dbgs() << '\n');

    // D5. [Test remainder.] Set q[j] = qp. If the result of step D4 was
    // negative, go to step D6; otherwise go on to step D7.
    q[j] = Lo_32(qp);
    எனில் (isNeg) {
      // D6. [Add back]. The probability that this step is necessary is very
      // small, on the order of only 2/b. Make sure that test data accounts for
      // this possibility. Decrease q[j] by 1
      q[j]--;
      // and add (0v[n-1]...v[1]v[0]) to (u[j+n]u[j+n-1]...u[j+1]u[j]).
      // A carry will occur to the left of u[j+n], and it should be ignored
      // since it cancels with the borrow that occurred in D4.
      தருக்க carry = பொய்;
      க்கு (unsigned i = 0; i < n; i++) {
        uint32_t limit = std::min(u[j+i],v[i]);
        u[j+i] += v[i] + carry;
        carry = u[j+i] < limit || (carry && u[j+i] == limit);
      }
      u[j+n] += carry;
    }
    DEBUG_KNUTH(dbgs() << "KnuthDiv: after correction:");
    DEBUG_KNUTH(க்கு (முழு i = m + n; i >= 0; i--) dbgs() << " " << u[i]);
    DEBUG_KNUTH(dbgs() << "\nKnuthDiv: digit result = " << q[j] << '\n');

    // D7. [Loop on j.]  Decrease j by one. Now if j >= 0, go back to D3.
  } போது (--j >= 0);

  DEBUG_KNUTH(dbgs() << "KnuthDiv: quotient:");
  DEBUG_KNUTH(க்கு (முழு i = m; i >= 0; i--) dbgs() << " " << q[i]);
  DEBUG_KNUTH(dbgs() << '\n');

  // D8. [Unnormalize]. Now q[...] is the desired quotient, and the desired
  // remainder may be obtained by dividing u[...] by d. If r is non-null we
  // compute the remainder (urem uses this).
  எனில் (r) {
    // The value d is expressed by the "shift" value above since we avoided
    // multiplication by d by using a shift left. So, all we have to do is
    // shift right here.
    எனில் (shift) {
      uint32_t carry = 0;
      DEBUG_KNUTH(dbgs() << "KnuthDiv: remainder:");
      க்கு (முழு i = n-1; i >= 0; i--) {
        r[i] = (u[i] >> shift) | carry;
        carry = u[i] << (32 - shift);
        DEBUG_KNUTH(dbgs() << " " << r[i]);
      }
    } இல்லையெனில் {
      க்கு (முழு i = n-1; i >= 0; i--) {
        r[i] = u[i];
        DEBUG_KNUTH(dbgs() << " " << r[i]);
      }
    }
    DEBUG_KNUTH(dbgs() << '\n');
  }
  DEBUG_KNUTH(dbgs() << '\n');
}

வெறுமை APInt::divide(மாறிலி WordType *LHS, unsigned lhs_சொற்கள், மாறிலி WordType *RHS,
                   unsigned rhs_சொற்கள், WordType *Quotient, WordType *Remainder) {
  ப்ரதிஜ்ஞா(lhs_சொற்கள் >= rhs_சொற்கள் && "Fractional result");

  // First, compose the values into an array of 32-bit words instead of
  // 64-bit words. This is a necessity of both the "short division" algorithm
  // and the Knuth "classical algorithm" which requires there to be native
  // operations for +, -, and * on an m bit value with an m*2 bit result. We
  // can't use 64-bit operands here because we don't have native results of
  // 128-bits. Furthermore, casting the 64-bit values to 32-bit values won't
  // work on large-endian machines.
  unsigned n = rhs_சொற்கள் * 2;
  unsigned m = (lhs_சொற்கள் * 2) - n;

  // Allocate space for the temporary values we need either on the stack, if
  // it will fit, or on the heap if it won't.
  uint32_t SPACE[128];
  uint32_t *U = nullptr;
  uint32_t *V = nullptr;
  uint32_t *Q = nullptr;
  uint32_t *R = nullptr;
  எனில் ((Remainder?4:3)*n+2*m+1 <= 128) {
    U = &SPACE[0];
    V = &SPACE[m+n+1];
    Q = &SPACE[(m+n+1) + n];
    எனில் (Remainder)
      R = &SPACE[(m+n+1) + n + (m+n)];
  } இல்லையெனில் {
    U = புதிய uint32_t[m + n + 1];
    V = புதிய uint32_t[n];
    Q = புதிய uint32_t[m+n];
    எனில் (Remainder)
      R = புதிய uint32_t[n];
  }

  // Initialize the dividend
  memset(U, 0, (m+n+1)*அளவு(uint32_t));
  க்கு (unsigned i = 0; i < lhs_சொற்கள்; ++i) {
    uint64_t tmp = LHS[i];
    U[i * 2] = Lo_32(tmp);
    U[i * 2 + 1] = Hi_32(tmp);
  }
  U[m+n] = 0; // this extra word is for "spill" in the Knuth algorithm.

  // Initialize the divisor
  memset(V, 0, (n)*அளவு(uint32_t));
  க்கு (unsigned i = 0; i < rhs_சொற்கள்; ++i) {
    uint64_t tmp = RHS[i];
    V[i * 2] = Lo_32(tmp);
    V[i * 2 + 1] = Hi_32(tmp);
  }

  // initialize the quotient and remainder
  memset(Q, 0, (m+n) * அளவு(uint32_t));
  எனில் (Remainder)
    memset(R, 0, n * அளவு(uint32_t));

  // Now, adjust m and n for the Knuth division. n is the number of words in
  // the divisor. m is the number of words by which the dividend exceeds the
  // divisor (i.e. m+n is the length of the dividend). These sizes must not
  // contain any zero words or the Knuth algorithm fails.
  க்கு (unsigned i = n; i > 0 && V[i-1] == 0; i--) {
    n--;
    m++;
  }
  க்கு (unsigned i = m+n; i > 0 && U[i-1] == 0; i--)
    m--;

  // If we're left with only a single word for the divisor, Knuth doesn't work
  // so we implement the short division algorithm here. This is much simpler
  // and faster because we are certain that we can divide a 64-bit quantity
  // by a 32-bit quantity at hardware speed and short division is simply a
  // series of such operations. This is just like doing short division but we
  // are using base 2^32 instead of base 10.
  ப்ரதிஜ்ஞா(n != 0 && "Divide by zero?");
  எனில் (n == 1) {
    uint32_t வகுஎண் = V[0];
    uint32_t remainder = 0;
    க்கு (முழு i = m; i >= 0; i--) {
      uint64_t partial_dividend = Make_64(remainder, U[i]);
      எனில் (partial_dividend == 0) {
        Q[i] = 0;
        remainder = 0;
      } இல்லையெனில் எனில் (partial_dividend < வகுஎண்) {
        Q[i] = 0;
        remainder = Lo_32(partial_dividend);
      } இல்லையெனில் எனில் (partial_dividend == வகுஎண்) {
        Q[i] = 1;
        remainder = 0;
      } இல்லையெனில் {
        Q[i] = Lo_32(partial_dividend / வகுஎண்);
        remainder = Lo_32(partial_dividend - (Q[i] * வகுஎண்));
      }
    }
    எனில் (R)
      R[0] = remainder;
  } இல்லையெனில் {
    // Now we're ready to invoke the Knuth classical divide algorithm. In this
    // case n > 1.
    KnuthDiv(U, V, Q, R, m, n);
  }

  // If the caller wants the quotient
  எனில் (Quotient) {
    க்கு (unsigned i = 0; i < lhs_சொற்கள்; ++i)
      Quotient[i] = Make_64(Q[i*2+1], Q[i*2]);
  }

  // If the caller wants the remainder
  எனில் (Remainder) {
    க்கு (unsigned i = 0; i < rhs_சொற்கள்; ++i)
      Remainder[i] = Make_64(R[i*2+1], R[i*2]);
  }

  // Clean up the memory we allocated.
  எனில் (U != &SPACE[0]) {
    delete [] U;
    delete [] V;
    delete [] Q;
    delete [] R;
  }
}

APInt APInt::udiv(மாறிலி APInt &RHS) மாறிலி {
  ப்ரதிஜ்ஞா(BitWidth == RHS.BitWidth && "Bit widths must be the same");

  // First, deal with the easy case
  எனில் (LLVM_LIKELY(isSingleWord())) {
    ப்ரதிஜ்ஞா(RHS.U.VAL != 0 && "Divide by zero?");
    திரும்பு APInt(BitWidth, U.VAL / RHS.U.VAL);
  }

  // Get some facts about the LHS and RHS number of bits and words
  unsigned lhs_சொற்கள் = get_இலக்கம்_சொற்கள்(getActiveBits());
  unsigned rhsBits  = RHS.getActiveBits();
  unsigned rhs_சொற்கள் = get_இலக்கம்_சொற்கள்(rhsBits);
  ப்ரதிஜ்ஞா(rhs_சொற்கள் && "Divided by zero???");

  // Deal with some degenerate cases
  எனில் (!lhs_சொற்கள்)
    // 0 / X ===> 0
    திரும்பு APInt(BitWidth, 0);
  எனில் (rhsBits == 1)
    // X / 1 ===> X
    திரும்பு *இது;
  எனில் (lhs_சொற்கள் < rhs_சொற்கள் || இது->ult(RHS))
    // X / Y ===> 0, iff X < Y
    திரும்பு APInt(BitWidth, 0);
  எனில் (*இது == RHS)
    // X / X ===> 1
    திரும்பு APInt(BitWidth, 1);
  எனில் (lhs_சொற்கள் == 1) // rhsWords is 1 if lhsWords is 1.
    // All high words are zero, just use native divide
    திரும்பு APInt(BitWidth, இது->U.pVal[0] / RHS.U.pVal[0]);

  // We have to compute it the hard way. Invoke the Knuth divide algorithm.
  APInt Quotient(BitWidth, 0); // to hold result.
  divide(U.pVal, lhs_சொற்கள், RHS.U.pVal, rhs_சொற்கள், Quotient.U.pVal, nullptr);
  திரும்பு Quotient;
}

APInt APInt::udiv(uint64_t RHS) மாறிலி {
  ப்ரதிஜ்ஞா(RHS != 0 && "Divide by zero?");

  // First, deal with the easy case
  எனில் (LLVM_LIKELY(isSingleWord()))
    திரும்பு APInt(BitWidth, U.VAL / RHS);

  // Get some facts about the LHS words.
  unsigned lhs_சொற்கள் = get_இலக்கம்_சொற்கள்(getActiveBits());

  // Deal with some degenerate cases
  எனில் (!lhs_சொற்கள்)
    // 0 / X ===> 0
    திரும்பு APInt(BitWidth, 0);
  எனில் (RHS == 1)
    // X / 1 ===> X
    திரும்பு *இது;
  எனில் (இது->ult(RHS))
    // X / Y ===> 0, iff X < Y
    திரும்பு APInt(BitWidth, 0);
  எனில் (*இது == RHS)
    // X / X ===> 1
    திரும்பு APInt(BitWidth, 1);
  எனில் (lhs_சொற்கள் == 1) // rhsWords is 1 if lhsWords is 1.
    // All high words are zero, just use native divide
    திரும்பு APInt(BitWidth, இது->U.pVal[0] / RHS);

  // We have to compute it the hard way. Invoke the Knuth divide algorithm.
  APInt Quotient(BitWidth, 0); // to hold result.
  divide(U.pVal, lhs_சொற்கள், &RHS, 1, Quotient.U.pVal, nullptr);
  திரும்பு Quotient;
}

APInt APInt::sdiv(மாறிலி APInt &RHS) மாறிலி {
  எனில் (isNegative()) {
    எனில் (RHS.isNegative())
      திரும்பு (-(*இது)).udiv(-RHS);
    திரும்பு -((-(*இது)).udiv(RHS));
  }
  எனில் (RHS.isNegative())
    திரும்பு -(இது->udiv(-RHS));
  திரும்பு இது->udiv(RHS);
}

APInt APInt::sdiv(int64_t RHS) மாறிலி {
  எனில் (isNegative()) {
    எனில் (RHS < 0)
      திரும்பு (-(*இது)).udiv(-RHS);
    திரும்பு -((-(*இது)).udiv(RHS));
  }
  எனில் (RHS < 0)
    திரும்பு -(இது->udiv(-RHS));
  திரும்பு இது->udiv(RHS);
}

APInt APInt::urem(மாறிலி APInt &RHS) மாறிலி {
  ப்ரதிஜ்ஞா(BitWidth == RHS.BitWidth && "Bit widths must be the same");
  எனில் (LLVM_LIKELY(isSingleWord())) {
    ப்ரதிஜ்ஞா(RHS.U.VAL != 0 && "Remainder by zero?");
    திரும்பு APInt(BitWidth, U.VAL % RHS.U.VAL);
  }

  // Get some facts about the LHS
  unsigned lhs_சொற்கள் = get_இலக்கம்_சொற்கள்(getActiveBits());

  // Get some facts about the RHS
  unsigned rhsBits = RHS.getActiveBits();
  unsigned rhs_சொற்கள் = get_இலக்கம்_சொற்கள்(rhsBits);
  ப்ரதிஜ்ஞா(rhs_சொற்கள் && "Performing remainder operation by zero ???");

  // Check the degenerate cases
  எனில் (lhs_சொற்கள் == 0)
    // 0 % Y ===> 0
    திரும்பு APInt(BitWidth, 0);
  எனில் (rhsBits == 1)
    // X % 1 ===> 0
    திரும்பு APInt(BitWidth, 0);
  எனில் (lhs_சொற்கள் < rhs_சொற்கள் || இது->ult(RHS))
    // X % Y ===> X, iff X < Y
    திரும்பு *இது;
  எனில் (*இது == RHS)
    // X % X == 0;
    திரும்பு APInt(BitWidth, 0);
  எனில் (lhs_சொற்கள் == 1)
    // All high words are zero, just use native remainder
    திரும்பு APInt(BitWidth, U.pVal[0] % RHS.U.pVal[0]);
  எனில் (RHS.isPowerOf2()) {
    // X % 2^w ===> X & (2^w - 1)
    APInt Result(*இது);
    Result.clearBits(RHS.logBase2(), BitWidth);
    திரும்பு Result;
  }

  // We have to compute it the hard way. Invoke the Knuth divide algorithm.
  APInt Remainder(BitWidth, 0);
  divide(U.pVal, lhs_சொற்கள், RHS.U.pVal, rhs_சொற்கள், nullptr, Remainder.U.pVal);
  திரும்பு Remainder;
}

uint64_t APInt::urem(uint64_t RHS) மாறிலி {
  ப்ரதிஜ்ஞா(RHS != 0 && "Remainder by zero?");

  எனில் (LLVM_LIKELY(isSingleWord()))
    திரும்பு U.VAL % RHS;

  // Get some facts about the LHS
  unsigned lhs_சொற்கள் = get_இலக்கம்_சொற்கள்(getActiveBits());

  // Check the degenerate cases
  எனில் (lhs_சொற்கள் == 0)
    // 0 % Y ===> 0
    திரும்பு 0;
  எனில் (RHS == 1)
    // X % 1 ===> 0
    திரும்பு 0;
  எனில் (இது->ult(RHS))
    // X % Y ===> X, iff X < Y
    திரும்பு get_ZExt_மதிப்பு();
  எனில் (*இது == RHS)
    // X % X == 0;
    திரும்பு 0;
  எனில் (lhs_சொற்கள் == 1)
    // All high words are zero, just use native remainder
    திரும்பு U.pVal[0] % RHS;
  எனில் (llvm::isPowerOf2_64(RHS))
    // X % 2^w ===> X & (2^w - 1)
    திரும்பு U.pVal[0] & (RHS - 1);

  // We have to compute it the hard way. Invoke the Knuth divide algorithm.
  uint64_t Remainder;
  divide(U.pVal, lhs_சொற்கள், &RHS, 1, nullptr, &Remainder);
  திரும்பு Remainder;
}

APInt APInt::srem(மாறிலி APInt &RHS) மாறிலி {
  எனில் (isNegative()) {
    எனில் (RHS.isNegative())
      திரும்பு -((-(*இது)).urem(-RHS));
    திரும்பு -((-(*இது)).urem(RHS));
  }
  எனில் (RHS.isNegative())
    திரும்பு இது->urem(-RHS);
  திரும்பு இது->urem(RHS);
}

int64_t APInt::srem(int64_t RHS) மாறிலி {
  எனில் (isNegative()) {
    எனில் (RHS < 0)
      திரும்பு -((-(*இது)).urem(-RHS));
    திரும்பு -((-(*இது)).urem(RHS));
  }
  எனில் (RHS < 0)
    திரும்பு இது->urem(-RHS);
  திரும்பு இது->urem(RHS);
}

வெறுமை APInt::udivrem(மாறிலி APInt &LHS, மாறிலி APInt &RHS,
                    APInt &Quotient, APInt &Remainder) {
  ப்ரதிஜ்ஞா(LHS.BitWidth == RHS.BitWidth && "Bit widths must be the same");
  unsigned BitWidth = LHS.BitWidth;

  // First, deal with the easy case
  எனில் (LLVM_LIKELY(LHS.isSingleWord())) {
    ப்ரதிஜ்ஞா(RHS.U.VAL != 0 && "Divide by zero?");
    uint64_t QuotVal = LHS.U.VAL / RHS.U.VAL;
    uint64_t RemVal = LHS.U.VAL % RHS.U.VAL;
    Quotient = APInt(BitWidth, QuotVal);
    Remainder = APInt(BitWidth, RemVal);
    திரும்பு;
  }

  // Get some size facts about the dividend and divisor
  unsigned lhs_சொற்கள் = get_இலக்கம்_சொற்கள்(LHS.getActiveBits());
  unsigned rhsBits  = RHS.getActiveBits();
  unsigned rhs_சொற்கள் = get_இலக்கம்_சொற்கள்(rhsBits);
  ப்ரதிஜ்ஞா(rhs_சொற்கள் && "Performing divrem operation by zero ???");

  // Check the degenerate cases
  எனில் (lhs_சொற்கள் == 0) {
    Quotient = APInt(BitWidth, 0);    // 0 / Y ===> 0
    Remainder = APInt(BitWidth, 0);   // 0 % Y ===> 0
    திரும்பு;
  }

  எனில் (rhsBits == 1) {
    Quotient = LHS;                   // X / 1 ===> X
    Remainder = APInt(BitWidth, 0);   // X % 1 ===> 0
  }

  எனில் (lhs_சொற்கள் < rhs_சொற்கள் || LHS.ult(RHS)) {
    Remainder = LHS;                  // X % Y ===> X, iff X < Y
    Quotient = APInt(BitWidth, 0);    // X / Y ===> 0, iff X < Y
    திரும்பு;
  }

  எனில் (LHS == RHS) {
    Quotient  = APInt(BitWidth, 1);   // X / X ===> 1
    Remainder = APInt(BitWidth, 0);   // X % X ===> 0;
    திரும்பு;
  }

  // Make sure there is enough space to hold the results.
  // NOTE: This assumes that reallocate won't affect any bits if it doesn't
  // change the size. This is necessary if Quotient or Remainder is aliased
  // with LHS or RHS.
  Quotient.reallocate(BitWidth);
  Remainder.reallocate(BitWidth);

  எனில் (lhs_சொற்கள் == 1) { // rhsWords is 1 if lhsWords is 1.
    // There is only one word to consider so use the native versions.
    uint64_t lhs_மதிப்பு = LHS.U.pVal[0];
    uint64_t rhs_மதிப்பு = RHS.U.pVal[0];
    Quotient = lhs_மதிப்பு / rhs_மதிப்பு;
    Remainder = lhs_மதிப்பு % rhs_மதிப்பு;
    திரும்பு;
  }

  // Okay, lets do it the long way
  divide(LHS.U.pVal, lhs_சொற்கள், RHS.U.pVal, rhs_சொற்கள், Quotient.U.pVal,
         Remainder.U.pVal);
  // Clear the rest of the Quotient and Remainder.
  std::memset(Quotient.U.pVal + lhs_சொற்கள், 0,
              (get_இலக்கம்_சொற்கள்(BitWidth) - lhs_சொற்கள்) * APINT_WORD_SIZE);
  std::memset(Remainder.U.pVal + rhs_சொற்கள், 0,
              (get_இலக்கம்_சொற்கள்(BitWidth) - rhs_சொற்கள்) * APINT_WORD_SIZE);
}

வெறுமை APInt::udivrem(மாறிலி APInt &LHS, uint64_t RHS, APInt &Quotient,
                    uint64_t &Remainder) {
  ப்ரதிஜ்ஞா(RHS != 0 && "Divide by zero?");
  unsigned BitWidth = LHS.BitWidth;

  // First, deal with the easy case
  எனில் (LLVM_LIKELY(LHS.isSingleWord())) {
    uint64_t QuotVal = LHS.U.VAL / RHS;
    Remainder = LHS.U.VAL % RHS;
    Quotient = APInt(BitWidth, QuotVal);
    திரும்பு;
  }

  // Get some size facts about the dividend and divisor
  unsigned lhs_சொற்கள் = get_இலக்கம்_சொற்கள்(LHS.getActiveBits());

  // Check the degenerate cases
  எனில் (lhs_சொற்கள் == 0) {
    Quotient = APInt(BitWidth, 0);    // 0 / Y ===> 0
    Remainder = 0;                    // 0 % Y ===> 0
    திரும்பு;
  }

  எனில் (RHS == 1) {
    Quotient = LHS;                   // X / 1 ===> X
    Remainder = 0;                    // X % 1 ===> 0
    திரும்பு;
  }

  எனில் (LHS.ult(RHS)) {
    Remainder = LHS.get_ZExt_மதிப்பு();   // X % Y ===> X, iff X < Y
    Quotient = APInt(BitWidth, 0);    // X / Y ===> 0, iff X < Y
    திரும்பு;
  }

  எனில் (LHS == RHS) {
    Quotient  = APInt(BitWidth, 1);   // X / X ===> 1
    Remainder = 0;                    // X % X ===> 0;
    திரும்பு;
  }

  // Make sure there is enough space to hold the results.
  // NOTE: This assumes that reallocate won't affect any bits if it doesn't
  // change the size. This is necessary if Quotient is aliased with LHS.
  Quotient.reallocate(BitWidth);

  எனில் (lhs_சொற்கள் == 1) { // rhsWords is 1 if lhsWords is 1.
    // There is only one word to consider so use the native versions.
    uint64_t lhs_மதிப்பு = LHS.U.pVal[0];
    Quotient = lhs_மதிப்பு / RHS;
    Remainder = lhs_மதிப்பு % RHS;
    திரும்பு;
  }

  // Okay, lets do it the long way
  divide(LHS.U.pVal, lhs_சொற்கள், &RHS, 1, Quotient.U.pVal, &Remainder);
  // Clear the rest of the Quotient.
  std::memset(Quotient.U.pVal + lhs_சொற்கள், 0,
              (get_இலக்கம்_சொற்கள்(BitWidth) - lhs_சொற்கள்) * APINT_WORD_SIZE);
}

வெறுமை APInt::sdivrem(மாறிலி APInt &LHS, மாறிலி APInt &RHS,
                    APInt &Quotient, APInt &Remainder) {
  எனில் (LHS.isNegative()) {
    எனில் (RHS.isNegative())
      APInt::udivrem(-LHS, -RHS, Quotient, Remainder);
    இல்லையெனில் {
      APInt::udivrem(-LHS, RHS, Quotient, Remainder);
      Quotient.negate();
    }
    Remainder.negate();
  } இல்லையெனில் எனில் (RHS.isNegative()) {
    APInt::udivrem(LHS, -RHS, Quotient, Remainder);
    Quotient.negate();
  } இல்லையெனில் {
    APInt::udivrem(LHS, RHS, Quotient, Remainder);
  }
}

வெறுமை APInt::sdivrem(மாறிலி APInt &LHS, int64_t RHS,
                    APInt &Quotient, int64_t &Remainder) {
  uint64_t R = Remainder;
  எனில் (LHS.isNegative()) {
    எனில் (RHS < 0)
      APInt::udivrem(-LHS, -RHS, Quotient, R);
    இல்லையெனில் {
      APInt::udivrem(-LHS, RHS, Quotient, R);
      Quotient.negate();
    }
    R = -R;
  } இல்லையெனில் எனில் (RHS < 0) {
    APInt::udivrem(LHS, -RHS, Quotient, R);
    Quotient.negate();
  } இல்லையெனில் {
    APInt::udivrem(LHS, RHS, Quotient, R);
  }
  Remainder = R;
}

APInt APInt::sadd_ov(மாறிலி APInt &RHS, தருக்க &Overflow) மாறிலி {
  APInt Res = *இது+RHS;
  Overflow = isNonNegative() == RHS.isNonNegative() &&
             Res.isNonNegative() != isNonNegative();
  திரும்பு Res;
}

APInt APInt::uadd_ov(மாறிலி APInt &RHS, தருக்க &Overflow) மாறிலி {
  APInt Res = *இது+RHS;
  Overflow = Res.ult(RHS);
  திரும்பு Res;
}

APInt APInt::ssub_ov(மாறிலி APInt &RHS, தருக்க &Overflow) மாறிலி {
  APInt Res = *இது - RHS;
  Overflow = isNonNegative() != RHS.isNonNegative() &&
             Res.isNonNegative() != isNonNegative();
  திரும்பு Res;
}

APInt APInt::usub_ov(மாறிலி APInt &RHS, தருக்க &Overflow) மாறிலி {
  APInt Res = *இது-RHS;
  Overflow = Res.ugt(*இது);
  திரும்பு Res;
}

APInt APInt::sdiv_ov(மாறிலி APInt &RHS, தருக்க &Overflow) மாறிலி {
  // MININT/-1  -->  overflow.
  Overflow = is_Min_Signed_மதிப்பு() && RHS.isAllOnes();
  திரும்பு sdiv(RHS);
}

APInt APInt::smul_ov(மாறிலி APInt &RHS, தருக்க &Overflow) மாறிலி {
  APInt Res = *இது * RHS;

  எனில் (RHS != 0)
    Overflow = Res.sdiv(RHS) != *இது ||
               (is_Min_Signed_மதிப்பு() && RHS.isAllOnes());
  இல்லையெனில்
    Overflow = பொய்;
  திரும்பு Res;
}

APInt APInt::umul_ov(மாறிலி APInt &RHS, தருக்க &Overflow) மாறிலி {
  எனில் (countl_zero() + RHS.countl_zero() + 2 <= BitWidth) {
    Overflow = உண்மை;
    திரும்பு *இது * RHS;
  }

  APInt Res = lshr(1) * RHS;
  Overflow = Res.isNegative();
  Res <<= 1;
  எனில் ((*இது)[0]) {
    Res += RHS;
    எனில் (Res.ult(RHS))
      Overflow = உண்மை;
  }
  திரும்பு Res;
}

APInt APInt::sshl_ov(மாறிலி APInt &ShAmt, தருக்க &Overflow) மாறிலி {
  திரும்பு sshl_ov(ShAmt.get_Limited_மதிப்பு(getBitWidth()), Overflow);
}

APInt APInt::sshl_ov(unsigned ShAmt, தருக்க &Overflow) மாறிலி {
  Overflow = ShAmt >= getBitWidth();
  எனில் (Overflow)
    திரும்பு APInt(BitWidth, 0);

  எனில் (isNonNegative()) // Don't allow sign change.
    Overflow = ShAmt >= countl_zero();
  இல்லையெனில்
    Overflow = ShAmt >= countl_one();

  திரும்பு *இது << ShAmt;
}

APInt APInt::ushl_ov(மாறிலி APInt &ShAmt, தருக்க &Overflow) மாறிலி {
  திரும்பு ushl_ov(ShAmt.get_Limited_மதிப்பு(getBitWidth()), Overflow);
}

APInt APInt::ushl_ov(unsigned ShAmt, தருக்க &Overflow) மாறிலி {
  Overflow = ShAmt >= getBitWidth();
  எனில் (Overflow)
    திரும்பு APInt(BitWidth, 0);

  Overflow = ShAmt > countl_zero();

  திரும்பு *இது << ShAmt;
}

APInt APInt::sfloordiv_ov(மாறிலி APInt &RHS, தருக்க &Overflow) மாறிலி {
  APInt quotient = sdiv_ov(RHS, Overflow);
  எனில் ((quotient * RHS != *இது) && (isNegative() != RHS.isNegative()))
    திரும்பு quotient - 1;
  திரும்பு quotient;
}

APInt APInt::sadd_sat(மாறிலி APInt &RHS) மாறிலி {
  தருக்க Overflow;
  APInt Res = sadd_ov(RHS, Overflow);
  எனில் (!Overflow)
    திரும்பு Res;

  திரும்பு isNegative() ? APInt::get_Signed_Min_மதிப்பு(BitWidth)
                      : APInt::get_Signed_Max_மதிப்பு(BitWidth);
}

APInt APInt::uadd_sat(மாறிலி APInt &RHS) மாறிலி {
  தருக்க Overflow;
  APInt Res = uadd_ov(RHS, Overflow);
  எனில் (!Overflow)
    திரும்பு Res;

  திரும்பு APInt::get_Max_மதிப்பு(BitWidth);
}

APInt APInt::ssub_sat(மாறிலி APInt &RHS) மாறிலி {
  தருக்க Overflow;
  APInt Res = ssub_ov(RHS, Overflow);
  எனில் (!Overflow)
    திரும்பு Res;

  திரும்பு isNegative() ? APInt::get_Signed_Min_மதிப்பு(BitWidth)
                      : APInt::get_Signed_Max_மதிப்பு(BitWidth);
}

APInt APInt::usub_sat(மாறிலி APInt &RHS) மாறிலி {
  தருக்க Overflow;
  APInt Res = usub_ov(RHS, Overflow);
  எனில் (!Overflow)
    திரும்பு Res;

  திரும்பு APInt(BitWidth, 0);
}

APInt APInt::smul_sat(மாறிலி APInt &RHS) மாறிலி {
  தருக்க Overflow;
  APInt Res = smul_ov(RHS, Overflow);
  எனில் (!Overflow)
    திரும்பு Res;

  // The result is negative if one and only one of inputs is negative.
  தருக்க ResIsNegative = isNegative() ^ RHS.isNegative();

  திரும்பு ResIsNegative ? APInt::get_Signed_Min_மதிப்பு(BitWidth)
                       : APInt::get_Signed_Max_மதிப்பு(BitWidth);
}

APInt APInt::umul_sat(மாறிலி APInt &RHS) மாறிலி {
  தருக்க Overflow;
  APInt Res = umul_ov(RHS, Overflow);
  எனில் (!Overflow)
    திரும்பு Res;

  திரும்பு APInt::get_Max_மதிப்பு(BitWidth);
}

APInt APInt::sshl_sat(மாறிலி APInt &RHS) மாறிலி {
  திரும்பு sshl_sat(RHS.get_Limited_மதிப்பு(getBitWidth()));
}

APInt APInt::sshl_sat(unsigned RHS) மாறிலி {
  தருக்க Overflow;
  APInt Res = sshl_ov(RHS, Overflow);
  எனில் (!Overflow)
    திரும்பு Res;

  திரும்பு isNegative() ? APInt::get_Signed_Min_மதிப்பு(BitWidth)
                      : APInt::get_Signed_Max_மதிப்பு(BitWidth);
}

APInt APInt::ushl_sat(மாறிலி APInt &RHS) மாறிலி {
  திரும்பு ushl_sat(RHS.get_Limited_மதிப்பு(getBitWidth()));
}

APInt APInt::ushl_sat(unsigned RHS) மாறிலி {
  தருக்க Overflow;
  APInt Res = ushl_ov(RHS, Overflow);
  எனில் (!Overflow)
    திரும்பு Res;

  திரும்பு APInt::get_Max_மதிப்பு(BitWidth);
}

வெறுமை APInt::fromString(unsigned numbits, StringRef str, uint8_t radix) {
  // Check our assumptions here
  ப்ரதிஜ்ஞா(!str.empty() && "Invalid string length");
  ப்ரதிஜ்ஞா((radix == 10 || radix == 8 || radix == 16 || radix == 2 ||
          radix == 36) &&
         "Radix should be 2, 8, 10, 16, or 36!");

  StringRef::iterator p = str.begin();
  size_t slen = str.size();
  தருக்க isNeg = *p == '-';
  எனில் (*p == '-' || *p == '+') {
    p++;
    slen--;
    ப்ரதிஜ்ஞா(slen && "String is only a sign, needs a value.");
  }
  ப்ரதிஜ்ஞா((slen <= numbits || radix != 2) && "Insufficient bit width");
  ப்ரதிஜ்ஞா(((slen-1)*3 <= numbits || radix != 8) && "Insufficient bit width");
  ப்ரதிஜ்ஞா(((slen-1)*4 <= numbits || radix != 16) && "Insufficient bit width");
  ப்ரதிஜ்ஞா((((slen-1)*64)/22 <= numbits || radix != 10) &&
         "Insufficient bit width");

  // Allocate memory if needed
  எனில் (LLVM_LIKELY(isSingleWord()))
    U.VAL = 0;
  இல்லையெனில்
    U.pVal = getClearedMemory(get_இலக்கம்_சொற்கள்());

  // Figure out if we can shift instead of multiply
  unsigned shift = (radix == 16 ? 4 : radix == 8 ? 3 : radix == 2 ? 1 : 0);

  // Enter digit traversal loop
  க்கு (StringRef::iterator e = str.end(); p != e; ++p) {
    unsigned digit = getDigit(*p, radix);
    ப்ரதிஜ்ஞா(digit < radix && "Invalid character in digit string");

    // Shift or multiply the value by the radix
    எனில் (slen > 1) {
      எனில் (shift)
        *இது <<= shift;
      இல்லையெனில்
        *இது *= radix;
    }

    // Add in the digit we just interpreted
    *இது += digit;
  }
  // If its negative, put it in two's complement form
  எனில் (isNeg)
    இது->negate();
}

வெறுமை APInt::toString(SmallVectorImpl<எழுத்து> &Str, unsigned Radix, தருக்க Signed,
                     தருக்க formatAsCLiteral, தருக்க UpperCase,
                     தருக்க செருகு_Separators) மாறிலி {
  ப்ரதிஜ்ஞா((Radix == 10 || Radix == 8 || Radix == 16 || Radix == 2 ||
          Radix == 36) &&
         "Radix should be 2, 8, 10, 16, or 36!");

  மாறிலி எழுத்து *Prefix = "";
  எனில் (formatAsCLiteral) {
    தேர்வு (Radix) {
      நிலை 2:
        // Binary literals are a non-standard extension added in gcc 4.3:
        // http://gcc.gnu.org/onlinedocs/gcc-4.3.0/gcc/Binary-constants.html
        Prefix = "0b";
        நிறுத்து;
      நிலை 8:
        Prefix = "0";
        நிறுத்து;
      நிலை 10:
        நிறுத்து; // No prefix
      நிலை 16:
        Prefix = "0x";
        நிறுத்து;
      இயல்பு:
        llvm_unreachable("Invalid radix!");
    }
  }

  // Number of digits in a group between separators.
  unsigned Grouping = (Radix == 8 || Radix == 10) ? 3 : 4;

  // First, check for a zero value and just short circuit the logic below.
  எனில் (isZero()) {
    போது (*Prefix) {
      Str.தள்ளு_back(*Prefix);
      ++Prefix;
    };
    Str.தள்ளு_back('0');
    திரும்பு;
  }

  நிலையான மாறிலி எழுத்து BothDigits[] = "0123456789abcdefghijklmnopqrstuvwxyz"
                                   "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ";
  மாறிலி எழுத்து *Digits = BothDigits + (UpperCase ? 36 : 0);

  எனில் (LLVM_LIKELY(isSingleWord())) {
    எழுத்து Buffer[65];
    எழுத்து *BufPtr = std::end(Buffer);

    uint64_t N;
    எனில் (!Signed) {
      N = get_ZExt_மதிப்பு();
    } இல்லையெனில் {
      int64_t I = get_SExt_மதிப்பு();
      எனில் (I >= 0) {
        N = I;
      } இல்லையெனில் {
        Str.தள்ளு_back('-');
        N = -(uint64_t)I;
      }
    }

    போது (*Prefix) {
      Str.தள்ளு_back(*Prefix);
      ++Prefix;
    };

    முழு Pos = 0;
    போது (N) {
      எனில் (செருகு_Separators && Pos % Grouping == 0 && Pos > 0)
        *--BufPtr = '\'';
      *--BufPtr = Digits[N % Radix];
      N /= Radix;
      Pos++;
    }
    Str.append(BufPtr, std::end(Buffer));
    திரும்பு;
  }

  APInt Tmp(*இது);

  எனில் (Signed && isNegative()) {
    // They want to print the signed version and it is a negative value
    // Flip the bits and add one to turn it into the equivalent positive
    // value and put a '-' in the result.
    Tmp.negate();
    Str.தள்ளு_back('-');
  }

  போது (*Prefix) {
    Str.தள்ளு_back(*Prefix);
    ++Prefix;
  }

  // We insert the digits backward, then reverse them to get the right order.
  unsigned StartDig = Str.size();

  // For the 2, 8 and 16 bit cases, we can just shift instead of divide
  // because the number of bits per digit (1, 3 and 4 respectively) divides
  // equally.  We just shift until the value is zero.
  எனில் (Radix == 2 || Radix == 8 || Radix == 16) {
    // Just shift tmp right for each digit width until it becomes zero
    unsigned ShiftAmt = (Radix == 16 ? 4 : (Radix == 8 ? 3 : 1));
    unsigned MaskAmt = Radix - 1;

    முழு Pos = 0;
    போது (Tmp.get_Bool_மதிப்பு()) {
      unsigned Digit = unsigned(Tmp.getRawData()[0]) & MaskAmt;
      எனில் (செருகு_Separators && Pos % Grouping == 0 && Pos > 0)
        Str.தள்ளு_back('\'');

      Str.தள்ளு_back(Digits[Digit]);
      Tmp.lshrInPlace(ShiftAmt);
      Pos++;
    }
  } இல்லையெனில் {
    முழு Pos = 0;
    போது (Tmp.get_Bool_மதிப்பு()) {
      uint64_t Digit;
      udivrem(Tmp, Radix, Tmp, Digit);
      ப்ரதிஜ்ஞா(Digit < Radix && "divide failed");
      எனில் (செருகு_Separators && Pos % Grouping == 0 && Pos > 0)
        Str.தள்ளு_back('\'');

      Str.தள்ளு_back(Digits[Digit]);
      Pos++;
    }
  }

  // Reverse the digits before returning.
  std::reverse(Str.begin()+StartDig, Str.end());
}

#எனில் !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)
LLVM_DUMP_METHOD வெறுமை APInt::dump() மாறிலி {
  SmallString<40> S, U;
  இது->toStringUnsigned(U);
  இது->toStringSigned(S);
  dbgs() << "APInt(" << BitWidth << "b, "
         << U << "u " << S << "s)\n";
}
#endif

வெறுமை APInt::print(raw_ostream &OS, தருக்க isSigned) மாறிலி {
  SmallString<40> S;
  இது->toString(S, 10, isSigned, /* formatAsCLiteral = */பொய்);
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
நிலையான உள்வரி APInt::WordType கீழ்_Bit_Mask(unsigned bits) {
  ப்ரதிஜ்ஞா(bits != 0 && bits <= APInt::APINT_BITS_PER_WORD);
  திரும்பு ~(APInt::WordType) 0 >> (APInt::APINT_BITS_PER_WORD - bits);
}

/// Returns the value of the lower half of PART.
நிலையான உள்வரி APInt::WordType கீழ்_Half(APInt::WordType part) {
  திரும்பு part & கீழ்_Bit_Mask(APInt::APINT_BITS_PER_WORD / 2);
}

/// Returns the value of the upper half of PART.
நிலையான உள்வரி APInt::WordType உயர்வு_Half(APInt::WordType part) {
  திரும்பு part >> (APInt::APINT_BITS_PER_WORD / 2);
}

/// Sets the least significant part of a bignum to the input value, and zeroes
/// out higher parts.
வெறுமை APInt::tcSet(WordType *dst, WordType part, unsigned parts) {
  ப்ரதிஜ்ஞா(parts > 0);
  dst[0] = part;
  க்கு (unsigned i = 1; i < parts; i++)
    dst[i] = 0;
}

/// Assign one bignum to another.
வெறுமை APInt::tcAssign(WordType *dst, மாறிலி WordType *src, unsigned parts) {
  க்கு (unsigned i = 0; i < parts; i++)
    dst[i] = src[i];
}

/// Returns true if a bignum is zero, false otherwise.
தருக்க APInt::tcIsZero(மாறிலி WordType *src, unsigned parts) {
  க்கு (unsigned i = 0; i < parts; i++)
    எனில் (src[i])
      திரும்பு பொய்;

  திரும்பு உண்மை;
}

/// Extract the given bit of a bignum; returns 0 or 1.
முழு APInt::tcExtractBit(மாறிலி WordType *parts, unsigned bit) {
  திரும்பு (parts[whichWord(bit)] & maskBit(bit)) != 0;
}

/// Set the given bit of a bignum.
வெறுமை APInt::tcSetBit(WordType *parts, unsigned bit) {
  parts[whichWord(bit)] |= maskBit(bit);
}

/// Clears the given bit of a bignum.
வெறுமை APInt::tcClearBit(WordType *parts, unsigned bit) {
  parts[whichWord(bit)] &= ~maskBit(bit);
}

/// Returns the bit number of the least significant set bit of a number.  If the
/// input number has no bits set UINT_MAX is returned.
unsigned APInt::tcLSB(மாறிலி WordType *parts, unsigned n) {
  க்கு (unsigned i = 0; i < n; i++) {
    எனில் (parts[i] != 0) {
      unsigned lsb = llvm::countr_zero(parts[i]);
      திரும்பு lsb + i * APINT_BITS_PER_WORD;
    }
  }

  திரும்பு UINT_MAX;
}

/// Returns the bit number of the most significant set bit of a number.
/// If the input number has no bits set UINT_MAX is returned.
unsigned APInt::tcMSB(மாறிலி WordType *parts, unsigned n) {
  செய் {
    --n;

    எனில் (parts[n] != 0) {
      static_assert(அளவு(parts[n]) <= அளவு(uint64_t));
      unsigned msb = llvm::Log2_64(parts[n]);

      திரும்பு msb + n * APINT_BITS_PER_WORD;
    }
  } போது (n);

  திரும்பு UINT_MAX;
}

/// Copy the bit vector of width srcBITS from SRC, starting at bit srcLSB, to
/// DST, of dstCOUNT parts, such that the bit srcLSB becomes the least
/// significant bit of DST.  All high bits above srcBITS in DST are zero-filled.
/// */
வெறுமை
APInt::tcExtract(WordType *dst, unsigned dst_மொத்தம், மாறிலி WordType *src,
                 unsigned srcBits, unsigned srcLSB) {
  unsigned dstParts = (srcBits + APINT_BITS_PER_WORD - 1) / APINT_BITS_PER_WORD;
  ப்ரதிஜ்ஞா(dstParts <= dst_மொத்தம்);

  unsigned firstSrcPart = srcLSB / APINT_BITS_PER_WORD;
  tcAssign(dst, src + firstSrcPart, dstParts);

  unsigned shift = srcLSB % APINT_BITS_PER_WORD;
  tc_Shift_வலது(dst, dstParts, shift);

  // We now have (dstParts * APINT_BITS_PER_WORD - shift) bits from SRC
  // in DST.  If this is less that srcBits, append the rest, else
  // clear the high bits.
  unsigned n = dstParts * APINT_BITS_PER_WORD - shift;
  எனில் (n < srcBits) {
    WordType mask = கீழ்_Bit_Mask (srcBits - n);
    dst[dstParts - 1] |= ((src[firstSrcPart + dstParts] & mask)
                          << n % APINT_BITS_PER_WORD);
  } இல்லையெனில் எனில் (n > srcBits) {
    எனில் (srcBits % APINT_BITS_PER_WORD)
      dst[dstParts - 1] &= கீழ்_Bit_Mask (srcBits % APINT_BITS_PER_WORD);
  }

  // Clear high parts.
  போது (dstParts < dst_மொத்தம்)
    dst[dstParts++] = 0;
}

//// DST += RHS + C where C is zero or one.  Returns the carry flag.
APInt::WordType APInt::tcAdd(WordType *dst, மாறிலி WordType *rhs,
                             WordType c, unsigned parts) {
  ப்ரதிஜ்ஞா(c <= 1);

  க்கு (unsigned i = 0; i < parts; i++) {
    WordType l = dst[i];
    எனில் (c) {
      dst[i] += rhs[i] + 1;
      c = (dst[i] <= l);
    } இல்லையெனில் {
      dst[i] += rhs[i];
      c = (dst[i] < l);
    }
  }

  திரும்பு c;
}

/// This function adds a single "word" integer, src, to the multiple
/// "word" integer array, dst[]. dst[] is modified to reflect the addition and
/// 1 is returned if there is a carry out, otherwise 0 is returned.
/// @returns the carry of the addition.
APInt::WordType APInt::tcAddPart(WordType *dst, WordType src,
                                 unsigned parts) {
  க்கு (unsigned i = 0; i < parts; ++i) {
    dst[i] += src;
    எனில் (dst[i] >= src)
      திரும்பு 0; // No need to carry so exit early.
    src = 1; // Carry one to next digit.
  }

  திரும்பு 1;
}

/// DST -= RHS + C where C is zero or one.  Returns the carry flag.
APInt::WordType APInt::tcSubtract(WordType *dst, மாறிலி WordType *rhs,
                                  WordType c, unsigned parts) {
  ப்ரதிஜ்ஞா(c <= 1);

  க்கு (unsigned i = 0; i < parts; i++) {
    WordType l = dst[i];
    எனில் (c) {
      dst[i] -= rhs[i] + 1;
      c = (dst[i] >= l);
    } இல்லையெனில் {
      dst[i] -= rhs[i];
      c = (dst[i] > l);
    }
  }

  திரும்பு c;
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
  க்கு (unsigned i = 0; i < parts; ++i) {
    WordType Dst = dst[i];
    dst[i] -= src;
    எனில் (src <= Dst)
      திரும்பு 0; // No need to borrow so exit early.
    src = 1; // We have to "borrow 1" from next "word"
  }

  திரும்பு 1;
}

/// Negate a bignum in-place.
வெறுமை APInt::tcNegate(WordType *dst, unsigned parts) {
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
முழு APInt::tcMultiplyPart(WordType *dst, மாறிலி WordType *src,
                          WordType multiplier, WordType carry,
                          unsigned srcParts, unsigned dstParts,
                          தருக்க add) {
  // Otherwise our writes of DST kill our later reads of SRC.
  ப்ரதிஜ்ஞா(dst <= src || dst >= src + srcParts);
  ப்ரதிஜ்ஞா(dstParts <= srcParts + 1);

  // N loops; minimum of dstParts and srcParts.
  unsigned n = std::min(dstParts, srcParts);

  க்கு (unsigned i = 0; i < n; i++) {
    // [LOW, HIGH] = MULTIPLIER * SRC[i] + DST[i] + CARRY.
    // This cannot overflow, because:
    //   (n - 1) * (n - 1) + 2 (n - 1) = (n - 1) * (n + 1)
    // which is less than n^2.
    WordType srcPart = src[i];
    WordType கீழ், mid, உயர்வு;
    எனில் (multiplier == 0 || srcPart == 0) {
      கீழ் = carry;
      உயர்வு = 0;
    } இல்லையெனில் {
      கீழ் = கீழ்_Half(srcPart) * கீழ்_Half(multiplier);
      உயர்வு = உயர்வு_Half(srcPart) * உயர்வு_Half(multiplier);

      mid = கீழ்_Half(srcPart) * உயர்வு_Half(multiplier);
      உயர்வு += உயர்வு_Half(mid);
      mid <<= APINT_BITS_PER_WORD / 2;
      எனில் (கீழ் + mid < கீழ்)
        உயர்வு++;
      கீழ் += mid;

      mid = உயர்வு_Half(srcPart) * கீழ்_Half(multiplier);
      உயர்வு += உயர்வு_Half(mid);
      mid <<= APINT_BITS_PER_WORD / 2;
      எனில் (கீழ் + mid < கீழ்)
        உயர்வு++;
      கீழ் += mid;

      // Now add carry.
      எனில் (கீழ் + carry < கீழ்)
        உயர்வு++;
      கீழ் += carry;
    }

    எனில் (add) {
      // And now DST[i], and store the new low part there.
      எனில் (கீழ் + dst[i] < கீழ்)
        உயர்வு++;
      dst[i] += கீழ்;
    } இல்லையெனில் {
      dst[i] = கீழ்;
    }

    carry = உயர்வு;
  }

  எனில் (srcParts < dstParts) {
    // Full multiplication, there is no overflow.
    ப்ரதிஜ்ஞா(srcParts + 1 == dstParts);
    dst[srcParts] = carry;
    திரும்பு 0;
  }

  // We overflowed if there is carry.
  எனில் (carry)
    திரும்பு 1;

  // We would overflow if any significant unwritten parts would be
  // non-zero.  This is true if any remaining src parts are non-zero
  // and the multiplier is non-zero.
  எனில் (multiplier)
    க்கு (unsigned i = dstParts; i < srcParts; i++)
      எனில் (src[i])
        திரும்பு 1;

  // We fitted in the narrow destination.
  திரும்பு 0;
}

/// DST = LHS * RHS, where DST has the same width as the operands and
/// is filled with the least significant parts of the result.  Returns
/// one if overflow occurred, otherwise zero.  DST must be disjoint
/// from both operands.
முழு APInt::tcMultiply(WordType *dst, மாறிலி WordType *lhs,
                      மாறிலி WordType *rhs, unsigned parts) {
  ப்ரதிஜ்ஞா(dst != lhs && dst != rhs);

  முழு overflow = 0;

  க்கு (unsigned i = 0; i < parts; i++) {
    // Don't accumulate on the first iteration so we don't need to initalize
    // dst to 0.
    overflow |=
        tcMultiplyPart(&dst[i], lhs, rhs[i], 0, parts, parts - i, i != 0);
  }

  திரும்பு overflow;
}

/// DST = LHS * RHS, where DST has width the sum of the widths of the
/// operands. No overflow occurs. DST must be disjoint from both operands.
வெறுமை APInt::tcFullMultiply(WordType *dst, மாறிலி WordType *lhs,
                           மாறிலி WordType *rhs, unsigned lhsParts,
                           unsigned rhsParts) {
  // Put the narrower number on the LHS for less loops below.
  எனில் (lhsParts > rhsParts)
    திரும்பு tcFullMultiply (dst, rhs, lhs, rhsParts, lhsParts);

  ப்ரதிஜ்ஞா(dst != lhs && dst != rhs);

  க்கு (unsigned i = 0; i < lhsParts; i++) {
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
முழு APInt::tcDivide(WordType *lhs, மாறிலி WordType *rhs,
                    WordType *remainder, WordType *srhs,
                    unsigned parts) {
  ப்ரதிஜ்ஞா(lhs != remainder && lhs != srhs && remainder != srhs);

  unsigned shift_மொத்தம் = tcMSB(rhs, parts) + 1;
  எனில் (shift_மொத்தம் == 0)
    திரும்பு உண்மை;

  shift_மொத்தம் = parts * APINT_BITS_PER_WORD - shift_மொத்தம்;
  unsigned n = shift_மொத்தம் / APINT_BITS_PER_WORD;
  WordType mask = (WordType) 1 << (shift_மொத்தம் % APINT_BITS_PER_WORD);

  tcAssign(srhs, rhs, parts);
  tc_Shift_இடது(srhs, parts, shift_மொத்தம்);
  tcAssign(remainder, lhs, parts);
  tcSet(lhs, 0, parts);

  // Loop, subtracting SRHS if REMAINDER is greater and adding that to the
  // total.
  க்கு (;;) {
    முழு compare = tcCompare(remainder, srhs, parts);
    எனில் (compare >= 0) {
      tcSubtract(remainder, srhs, 0, parts);
      lhs[n] |= mask;
    }

    எனில் (shift_மொத்தம் == 0)
      நிறுத்து;
    shift_மொத்தம்--;
    tc_Shift_வலது(srhs, parts, 1);
    எனில் ((mask >>= 1) == 0) {
      mask = (WordType) 1 << (APINT_BITS_PER_WORD - 1);
      n--;
    }
  }

  திரும்பு பொய்;
}

/// Shift a bignum left Count bits in-place. Shifted in bits are zero. There are
/// no restrictions on Count.
வெறுமை APInt::tc_Shift_இடது(WordType *Dst, unsigned சொற்கள்_2, unsigned மொத்தம்_2) {
  // Don't bother performing a no-op shift.
  எனில் (!மொத்தம்_2)
    திரும்பு;

  // WordShift is the inter-part shift; BitShift is the intra-part shift.
  unsigned WordShift = std::min(மொத்தம்_2 / APINT_BITS_PER_WORD, சொற்கள்_2);
  unsigned BitShift = மொத்தம்_2 % APINT_BITS_PER_WORD;

  // Fastpath for moving by whole words.
  எனில் (BitShift == 0) {
    std::memmove(Dst + WordShift, Dst, (சொற்கள்_2 - WordShift) * APINT_WORD_SIZE);
  } இல்லையெனில் {
    போது (சொற்கள்_2-- > WordShift) {
      Dst[சொற்கள்_2] = Dst[சொற்கள்_2 - WordShift] << BitShift;
      எனில் (சொற்கள்_2 > WordShift)
        Dst[சொற்கள்_2] |=
          Dst[சொற்கள்_2 - WordShift - 1] >> (APINT_BITS_PER_WORD - BitShift);
    }
  }

  // Fill in the remainder with 0s.
  std::memset(Dst, 0, WordShift * APINT_WORD_SIZE);
}

/// Shift a bignum right Count bits in-place. Shifted in bits are zero. There
/// are no restrictions on Count.
வெறுமை APInt::tc_Shift_வலது(WordType *Dst, unsigned சொற்கள்_2, unsigned மொத்தம்_2) {
  // Don't bother performing a no-op shift.
  எனில் (!மொத்தம்_2)
    திரும்பு;

  // WordShift is the inter-part shift; BitShift is the intra-part shift.
  unsigned WordShift = std::min(மொத்தம்_2 / APINT_BITS_PER_WORD, சொற்கள்_2);
  unsigned BitShift = மொத்தம்_2 % APINT_BITS_PER_WORD;

  unsigned சொற்கள்_To_Move = சொற்கள்_2 - WordShift;
  // Fastpath for moving by whole words.
  எனில் (BitShift == 0) {
    std::memmove(Dst, Dst + WordShift, சொற்கள்_To_Move * APINT_WORD_SIZE);
  } இல்லையெனில் {
    க்கு (unsigned i = 0; i != சொற்கள்_To_Move; ++i) {
      Dst[i] = Dst[i + WordShift] >> BitShift;
      எனில் (i + 1 != சொற்கள்_To_Move)
        Dst[i] |= Dst[i + WordShift + 1] << (APINT_BITS_PER_WORD - BitShift);
    }
  }

  // Fill in the remainder with 0s.
  std::memset(Dst + சொற்கள்_To_Move, 0, WordShift * APINT_WORD_SIZE);
}

// Comparison (unsigned) of two bignums.
முழு APInt::tcCompare(மாறிலி WordType *lhs, மாறிலி WordType *rhs,
                     unsigned parts) {
  போது (parts) {
    parts--;
    எனில் (lhs[parts] != rhs[parts])
      திரும்பு (lhs[parts] > rhs[parts]) ? 1 : -1;
  }

  திரும்பு 0;
}

APInt llvm::APIntOps::RoundingUDiv(மாறிலி APInt &A, மாறிலி APInt &B,
                                   APInt::Rounding RM) {
  // Currently udivrem always rounds down.
  தேர்வு (RM) {
  நிலை APInt::Rounding::DOWN:
  நிலை APInt::Rounding::TOWARD_ZERO:
    திரும்பு A.udiv(B);
  நிலை APInt::Rounding::UP: {
    APInt Quo, Rem;
    APInt::udivrem(A, B, Quo, Rem);
    எனில் (Rem.isZero())
      திரும்பு Quo;
    திரும்பு Quo + 1;
  }
  }
  llvm_unreachable("Unknown APInt::Rounding enum");
}

APInt llvm::APIntOps::RoundingSDiv(மாறிலி APInt &A, மாறிலி APInt &B,
                                   APInt::Rounding RM) {
  தேர்வு (RM) {
  நிலை APInt::Rounding::DOWN:
  நிலை APInt::Rounding::UP: {
    APInt Quo, Rem;
    APInt::sdivrem(A, B, Quo, Rem);
    எனில் (Rem.isZero())
      திரும்பு Quo;
    // This algorithm deals with arbitrary rounding mode used by sdivrem.
    // We want to check whether the non-integer part of the mathematical value
    // is negative or not. If the non-integer part is negative, we need to round
    // down from Quo; otherwise, if it's positive or 0, we return Quo, as it's
    // already rounded down.
    எனில் (RM == APInt::Rounding::DOWN) {
      எனில் (Rem.isNegative() != B.isNegative())
        திரும்பு Quo - 1;
      திரும்பு Quo;
    }
    எனில் (Rem.isNegative() != B.isNegative())
      திரும்பு Quo;
    திரும்பு Quo + 1;
  }
  // Currently sdiv rounds towards zero.
  நிலை APInt::Rounding::TOWARD_ZERO:
    திரும்பு A.sdiv(B);
  }
  llvm_unreachable("Unknown APInt::Rounding enum");
}

std::optional<APInt>
llvm::APIntOps::SolveQuadraticEquationWrap(APInt A, APInt B, APInt C,
                                           unsigned RangeWidth) {
  unsigned CoeffWidth = A.getBitWidth();
  ப்ரதிஜ்ஞா(CoeffWidth == B.getBitWidth() && CoeffWidth == C.getBitWidth());
  ப்ரதிஜ்ஞா(RangeWidth <= CoeffWidth &&
         "Value range width should be less than coefficient width");
  ப்ரதிஜ்ஞா(RangeWidth > 1 && "Value range bit width should be > 1");

  LLVM_DEBUG(dbgs() << __func__ << ": solving " << A << "x^2 + " << B
                    << "x + " << C << ", rw:" << RangeWidth << '\n');

  // Identify 0 as a (non)solution immediately.
  எனில் (C.sextOrTrunc(RangeWidth).isZero()) {
    LLVM_DEBUG(dbgs() << __func__ << ": zero solution\n");
    திரும்பு APInt(CoeffWidth, 0);
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
  எனில் (A.isNegative()) {
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
  தருக்க Pick_கீழ்;

  auto RoundUp = [] (மாறிலி APInt &V, மாறிலி APInt &A) -> APInt {
    ப்ரதிஜ்ஞா(A.isStrictlyPositive());
    APInt T = V.abs().urem(A);
    எனில் (T.isZero())
      திரும்பு V;
    திரும்பு V.isNegative() ? V+T : V+(A-T);
  };

  // The vertex of the parabola is at -B/2A, but since A > 0, it's negative
  // iff B is positive.
  எனில் (B.isNonNegative()) {
    // If B >= 0, the vertex it at a negative location (or at 0), so in
    // order to have a non-negative solution we need to pick k that makes
    // C-kR negative. To satisfy all the requirements for the solution
    // that we are looking for, it needs to be closest to 0 of all k.
    C = C.srem(R);
    எனில் (C.isStrictlyPositive())
      C -= R;
    // Pick the greater solution.
    Pick_கீழ் = பொய்;
  } இல்லையெனில் {
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
    எனில் (C.sgt(LowkR)) {
      // If LowkR < C, then such a k is guaranteed to exist because
      // LowkR itself is a multiple of R.
      C -= -RoundUp(-C, R);      // C = C - RoundDown(C, R)
      // Pick the smaller solution.
      Pick_கீழ் = உண்மை;
    } இல்லையெனில் {
      // If C-kR < 0 for all potential k's, it means that one solution
      // will be negative, while the other will be positive. The positive
      // solution will shift towards 0 if the parabola is moved up.
      // Pick the kR closest to the lower bound (i.e. make C-kR closest
      // to 0, or in other words, out of all parabolas that have solutions,
      // pick the one that is the farthest "up").
      // Since LowkR is itself a multiple of R, simply take C-LowkR.
      C -= LowkR;
      // Pick the greater solution.
      Pick_கீழ் = பொய்;
    }
  }

  LLVM_DEBUG(dbgs() << __func__ << ": updated coefficients " << A << "x^2 + "
                    << B << "x + " << C << ", rw:" << RangeWidth << '\n');

  APInt D = SqrB - 4*A*C;
  ப்ரதிஜ்ஞா(D.isNonNegative() && "Negative discriminant");
  APInt SQ = D.sqrtFloor();

  APInt Q = SQ * SQ;
  தருக்க InexactSQ = Q != D;

  APInt X;
  APInt Rem;

  // SQ is rounded down (i.e SQ * SQ <= D), so the roots may be inexact.
  // When using the quadratic formula directly, the calculated low root
  // may be greater than the exact one, since we would be subtracting SQ.
  // To make sure that the calculated root is not greater than the exact
  // one, subtract SQ+1 when calculating the low root (for inexact value
  // of SQ).
  எனில் (Pick_கீழ்)
    APInt::sdivrem(-B - (SQ+InexactSQ), TwoA, X, Rem);
  இல்லையெனில்
    APInt::sdivrem(-B + SQ, TwoA, X, Rem);

  // The updated coefficients should be such that the (exact) solution is
  // positive. Since APInt division rounds towards 0, the calculated one
  // can be 0, but cannot be negative.
  ப்ரதிஜ்ஞா(X.isNonNegative() && "Solution should be non-negative");

  எனில் (!InexactSQ && Rem.isZero()) {
    LLVM_DEBUG(dbgs() << __func__ << ": solution (root): " << X << '\n');
    திரும்பு X;
  }

  ப்ரதிஜ்ஞா((SQ*SQ).sle(D) && "SQ = |_sqrt(D)_|, so SQ*SQ <= D");
  // The exact value of the square root of D should be between SQ and SQ+1.
  // This implies that the solution should be between that corresponding to
  // SQ (i.e. X) and that corresponding to SQ+1.
  //
  // The calculated X cannot be greater than the exact (real) solution.
  // Actually it must be strictly less than the exact solution, while
  // X+1 will be greater than or equal to it.

  APInt VX = (A*X + B)*X + C;
  APInt VY = VX + TwoA*X + A + B;
  தருக்க SignChange =
      VX.isNegative() != VY.isNegative() || VX.isZero() != VY.isZero();
  // If the sign did not change between X and X+1, X is not a valid solution.
  // This could happen when the actual (exact) roots don't have an integer
  // between them, so they would both be contained between X and X+1.
  எனில் (!SignChange) {
    LLVM_DEBUG(dbgs() << __func__ << ": no valid solution\n");
    திரும்பு std::nullopt;
  }

  X += 1;
  LLVM_DEBUG(dbgs() << __func__ << ": solution (wrap): " << X << '\n');
  திரும்பு X;
}

std::optional<unsigned>
llvm::APIntOps::GetMostSignificantDifferentBit(மாறிலி APInt &A, மாறிலி APInt &B) {
  ப்ரதிஜ்ஞா(A.getBitWidth() == B.getBitWidth() && "Must have the same bitwidth");
  எனில் (A == B)
    திரும்பு std::nullopt;
  திரும்பு A.getBitWidth() - ((A ^ B).countl_zero() + 1);
}

APInt llvm::APIntOps::ScaleBitMask(மாறிலி APInt &A, unsigned NewBitWidth,
                                   தருக்க MatchAllBits) {
  unsigned OldBitWidth = A.getBitWidth();
  ப்ரதிஜ்ஞா((((OldBitWidth % NewBitWidth) == 0) ||
          ((NewBitWidth % OldBitWidth) == 0)) &&
         "One size should be a multiple of the other one. "
         "Can't do fractional scaling.");

  // Check for matching bitwidths.
  எனில் (OldBitWidth == NewBitWidth)
    திரும்பு A;

  APInt NewA = APInt::getZero(NewBitWidth);

  // Check for null input.
  எனில் (A.isZero())
    திரும்பு NewA;

  எனில் (NewBitWidth > OldBitWidth) {
    // Repeat bits.
    unsigned Scale = NewBitWidth / OldBitWidth;
    க்கு (unsigned i = 0; i != OldBitWidth; ++i)
      எனில் (A[i])
        NewA.setBits(i * Scale, (i + 1) * Scale);
  } இல்லையெனில் {
    unsigned Scale = OldBitWidth / NewBitWidth;
    க்கு (unsigned i = 0; i != NewBitWidth; ++i) {
      எனில் (MatchAllBits) {
        எனில் (A.extractBits(Scale, i * Scale).isAllOnes())
          NewA.setBit(i);
      } இல்லையெனில் {
        எனில் (!A.extractBits(Scale, i * Scale).isZero())
          NewA.setBit(i);
      }
    }
  }

  திரும்பு NewA;
}

/// StoreIntToMemory - Fills the StoreBytes bytes of memory starting from Dst
/// with the integer held in IntVal.
வெறுமை llvm::StoreIntToMemory(மாறிலி APInt &IntVal, uint8_t *Dst,
                            unsigned StoreBytes) {
  ப்ரதிஜ்ஞா((IntVal.getBitWidth()+7)/8 >= StoreBytes && "Integer too small!");
  மாறிலி uint8_t *Src = (மாறிலி uint8_t *)IntVal.getRawData();

  எனில் (sys::IsLittleEndianHost) {
    // Little-endian host - the source is ordered from LSB to MSB.  Order the
    // destination from LSB to MSB: Do a straight copy.
    memcpy(Dst, Src, StoreBytes);
  } இல்லையெனில் {
    // Big-endian host - the source is an array of 64 bit words ordered from
    // LSW to MSW.  Each word is ordered from MSB to LSB.  Order the destination
    // from MSB to LSB: Reverse the word order, but not the bytes in a word.
    போது (StoreBytes > அளவு(uint64_t)) {
      StoreBytes -= அளவு(uint64_t);
      // May not be aligned so use memcpy.
      memcpy(Dst + StoreBytes, Src, அளவு(uint64_t));
      Src += அளவு(uint64_t);
    }

    memcpy(Dst, Src + அளவு(uint64_t) - StoreBytes, StoreBytes);
  }
}

/// LoadIntFromMemory - Loads the integer stored in the LoadBytes bytes starting
/// from Src into IntVal, which is assumed to be wide enough and to hold zero.
வெறுமை llvm::LoadIntFromMemory(APInt &IntVal, மாறிலி uint8_t *Src,
                             unsigned LoadBytes) {
  ப்ரதிஜ்ஞா((IntVal.getBitWidth()+7)/8 >= LoadBytes && "Integer too small!");
  uint8_t *Dst = reinterpret_cast<uint8_t *>(
                   const_cast<uint64_t *>(IntVal.getRawData()));

  எனில் (sys::IsLittleEndianHost)
    // Little-endian host - the destination must be ordered from LSB to MSB.
    // The source is ordered from LSB to MSB: Do a straight copy.
    memcpy(Dst, Src, LoadBytes);
  இல்லையெனில் {
    // Big-endian - the destination is an array of 64 bit words ordered from
    // LSW to MSW.  Each word must be ordered from MSB to LSB.  The source is
    // ordered from MSB to LSB: Reverse the word order, but not the bytes in
    // a word.
    போது (LoadBytes > அளவு(uint64_t)) {
      LoadBytes -= அளவு(uint64_t);
      // May not be aligned so use memcpy.
      memcpy(Dst, Src + LoadBytes, அளவு(uint64_t));
      Dst += அளவு(uint64_t);
    }

    memcpy(Dst + அளவு(uint64_t) - LoadBytes, Src, LoadBytes);
  }
}

APInt APIntOps::avgFloorS(மாறிலி APInt &C1, மாறிலி APInt &C2) {
  // Return floor((C1 + C2) / 2)
  திரும்பு (C1 & C2) + (C1 ^ C2).ashr(1);
}

APInt APIntOps::avgFloorU(மாறிலி APInt &C1, மாறிலி APInt &C2) {
  // Return floor((C1 + C2) / 2)
  திரும்பு (C1 & C2) + (C1 ^ C2).lshr(1);
}

APInt APIntOps::avgCeilS(மாறிலி APInt &C1, மாறிலி APInt &C2) {
  // Return ceil((C1 + C2) / 2)
  திரும்பு (C1 | C2) - (C1 ^ C2).ashr(1);
}

APInt APIntOps::avgCeilU(மாறிலி APInt &C1, மாறிலி APInt &C2) {
  // Return ceil((C1 + C2) / 2)
  திரும்பு (C1 | C2) - (C1 ^ C2).lshr(1);
}

APInt APIntOps::avgSubFloorS(மாறிலி APInt &C1, மாறிலி APInt &C2) {
  // Return floor((C1 - C2) / 2), using C1 - C2 == (C1 ^ C2) - 2 * (~C1 & C2).
  திரும்பு (C1 ^ C2).ashr(1) - (~C1 & C2);
}

APInt APIntOps::avgSubFloorU(மாறிலி APInt &C1, மாறிலி APInt &C2) {
  // Return floor((C1 - C2) / 2), using C1 - C2 == (C1 ^ C2) - 2 * (~C1 & C2).
  திரும்பு (C1 ^ C2).lshr(1) - (~C1 & C2);
}

APInt APIntOps::mulhs(மாறிலி APInt &C1, மாறிலி APInt &C2) {
  ப்ரதிஜ்ஞா(C1.getBitWidth() == C2.getBitWidth() && "Unequal bitwidths");
  unsigned FullWidth = C1.getBitWidth() * 2;
  APInt C1Ext = C1.sext(FullWidth);
  APInt C2Ext = C2.sext(FullWidth);
  திரும்பு (C1Ext * C2Ext).extractBits(C1.getBitWidth(), C1.getBitWidth());
}

APInt APIntOps::mulhu(மாறிலி APInt &C1, மாறிலி APInt &C2) {
  ப்ரதிஜ்ஞா(C1.getBitWidth() == C2.getBitWidth() && "Unequal bitwidths");
  unsigned FullWidth = C1.getBitWidth() * 2;
  APInt C1Ext = C1.zext(FullWidth);
  APInt C2Ext = C2.zext(FullWidth);
  திரும்பு (C1Ext * C2Ext).extractBits(C1.getBitWidth(), C1.getBitWidth());
}

APInt APIntOps::mulsExtended(மாறிலி APInt &C1, மாறிலி APInt &C2) {
  ப்ரதிஜ்ஞா(C1.getBitWidth() == C2.getBitWidth() && "Unequal bitwidths");
  unsigned FullWidth = C1.getBitWidth() * 2;
  APInt C1Ext = C1.sext(FullWidth);
  APInt C2Ext = C2.sext(FullWidth);
  திரும்பு C1Ext * C2Ext;
}

APInt APIntOps::muluExtended(மாறிலி APInt &C1, மாறிலி APInt &C2) {
  ப்ரதிஜ்ஞா(C1.getBitWidth() == C2.getBitWidth() && "Unequal bitwidths");
  unsigned FullWidth = C1.getBitWidth() * 2;
  APInt C1Ext = C1.zext(FullWidth);
  APInt C2Ext = C2.zext(FullWidth);
  திரும்பு C1Ext * C2Ext;
}

APInt APIntOps::pow(மாறிலி APInt &X, int64_t N) {
  ப்ரதிஜ்ஞா(N >= 0 && "negative exponents not supported.");
  APInt Acc = APInt(X.getBitWidth(), 1);
  எனில் (N == 0)
    திரும்பு Acc;
  APInt Base = X;
  int64_t RemainingExponent = N;
  போது (RemainingExponent > 0) {
    போது (RemainingExponent % 2 == 0) {
      Base *= Base;
      RemainingExponent /= 2;
    }
    --RemainingExponent;
    Acc *= Base;
  }
  திரும்பு Acc;
}

APInt llvm::APIntOps::fshl(மாறிலி APInt &Hi, மாறிலி APInt &Lo,
                           மாறிலி APInt &Shift) {
  ப்ரதிஜ்ஞா(Hi.getBitWidth() == Lo.getBitWidth());
  unsigned ShiftAmt = rotateModulo(Hi.getBitWidth(), Shift);
  எனில் (ShiftAmt == 0)
    திரும்பு Hi;
  திரும்பு Hi.shl(ShiftAmt) | Lo.lshr(Hi.getBitWidth() - ShiftAmt);
}

APInt llvm::APIntOps::fshr(மாறிலி APInt &Hi, மாறிலி APInt &Lo,
                           மாறிலி APInt &Shift) {
  ப்ரதிஜ்ஞா(Hi.getBitWidth() == Lo.getBitWidth());
  unsigned ShiftAmt = rotateModulo(Hi.getBitWidth(), Shift);
  எனில் (ShiftAmt == 0)
    திரும்பு Lo;
  திரும்பு Hi.shl(Hi.getBitWidth() - ShiftAmt) | Lo.lshr(ShiftAmt);
}

APInt llvm::APIntOps::clmul(மாறிலி APInt &LHS, மாறிலி APInt &RHS) {
  unsigned BW = LHS.getBitWidth();
  ப்ரதிஜ்ஞா(BW == RHS.getBitWidth() && "Operand mismatch");
  APInt Result(BW, 0);
  க்கு (unsigned I : seq(std::min(RHS.getActiveBits(), BW - LHS.countr_zero())))
    எனில் (RHS[I])
      Result ^= LHS << I;
  திரும்பு Result;
}

APInt llvm::APIntOps::clmulr(மாறிலி APInt &LHS, மாறிலி APInt &RHS) {
  ப்ரதிஜ்ஞா(LHS.getBitWidth() == RHS.getBitWidth());
  திரும்பு clmul(LHS.reverseBits(), RHS.reverseBits()).reverseBits();
}

APInt llvm::APIntOps::clmulh(மாறிலி APInt &LHS, மாறிலி APInt &RHS) {
  ப்ரதிஜ்ஞா(LHS.getBitWidth() == RHS.getBitWidth());
  திரும்பு clmulr(LHS, RHS).lshr(1);
}

APInt llvm::APIntOps::pext(மாறிலி APInt &Val, மாறிலி APInt &Mask) {
  unsigned BW = Val.getBitWidth();
  ப்ரதிஜ்ஞா(BW == Mask.getBitWidth() && "Operand mismatch");
  APInt Result = APInt::getZero(BW);
  க்கு (unsigned I = 0, P = 0; I != BW; ++I)
    எனில் (Mask[I])
      Result.setBitVal(P++, Val[I]);
  திரும்பு Result;
}

APInt llvm::APIntOps::pdep(மாறிலி APInt &Val, மாறிலி APInt &Mask) {
  unsigned BW = Val.getBitWidth();
  ப்ரதிஜ்ஞா(BW == Mask.getBitWidth() && "Operand mismatch");
  APInt Result = APInt::getZero(BW);
  க்கு (unsigned I = 0, P = 0; I != BW; ++I)
    எனில் (Mask[I])
      Result.setBitVal(I, Val[P++]);
  திரும்பு Result;
}
