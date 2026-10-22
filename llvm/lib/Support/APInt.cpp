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

#ଅନ୍ତର୍ଭାଵ "llvm/ADT/APInt.h"
#ଅନ୍ତର୍ଭାଵ "llvm/ADT/ArrayRef.h"
#ଅନ୍ତର୍ଭାଵ "llvm/ADT/FoldingSet.h"
#ଅନ୍ତର୍ଭାଵ "llvm/ADT/Hashing.h"
#ଅନ୍ତର୍ଭାଵ "llvm/ADT/Sequence.h"
#ଅନ୍ତର୍ଭାଵ "llvm/ADT/SmallString.h"
#ଅନ୍ତର୍ଭାଵ "llvm/ADT/StringRef.h"
#ଅନ୍ତର୍ଭାଵ "llvm/ADT/bit.h"
#ଅନ୍ତର୍ଭାଵ "llvm/Support/Alignment.h"
#ଅନ୍ତର୍ଭାଵ "llvm/Support/Debug.h"
#ଅନ୍ତର୍ଭାଵ "llvm/Support/ErrorHandling.h"
#ଅନ୍ତର୍ଭାଵ "llvm/Support/MathExtras.h"
#ଅନ୍ତର୍ଭାଵ "llvm/Support/SwapByteOrder.h"
#ଅନ୍ତର୍ଭାଵ "llvm/Support/raw_ostream.h"
#ଅନ୍ତର୍ଭାଵ <cmath>
#ଅନ୍ତର୍ଭାଵ <optional>

using ନାମାକାଶ llvm;

#define DEBUG_TYPE "apint"

/// A utility function for allocating memory, checking for allocation failures,
/// and ensuring the contents are zeroed.
ଧାଡ଼ିଭିତରେ ସ୍ଥିର uint64_t* getClearedMemory(unsigned ଅଙ୍କ_ଶବ୍ଦଗୁଡ଼ିକ) {
  ଫେରନ୍ତୁ ନୂତନ uint64_t[ଅଙ୍କ_ଶବ୍ଦଗୁଡ଼ିକ]();
}

/// A utility function for allocating memory and checking for allocation
/// failure.  The content is not zeroed.
ଧାଡ଼ିଭିତରେ ସ୍ଥିର uint64_t* getMemory(unsigned ଅଙ୍କ_ଶବ୍ଦଗୁଡ଼ିକ) {
  ଫେରନ୍ତୁ ନୂତନ uint64_t[ଅଙ୍କ_ଶବ୍ଦଗୁଡ଼ିକ];
}

/// A utility function that converts a character to a digit.
ଧାଡ଼ିଭିତରେ ସ୍ଥିର unsigned getDigit(ଅକ୍ଷର cdigit, uint8_t radix) {
  unsigned r;

  ଯଦି (radix == 16 || radix == 36) {
    r = cdigit - '0';
    ଯଦି (r <= 9)
      ଫେରନ୍ତୁ r;

    r = cdigit - 'A';
    ଯଦି (r <= radix - 11U)
      ଫେରନ୍ତୁ r + 10;

    r = cdigit - 'a';
    ଯଦି (r <= radix - 11U)
      ଫେରନ୍ତୁ r + 10;

    radix = 10;
  }

  r = cdigit - '0';
  ଯଦି (r < radix)
    ଫେରନ୍ତୁ r;

  ଫେରନ୍ତୁ UINT_MAX;
}


ଶୂନ୍ୟ APInt::initSlowCase(uint64_t val, ତାର୍କିକ isSigned) {
  ଯଦି (isSigned && int64_t(val) < 0) {
    U.pVal = getMemory(get_ଅଙ୍କ_ଶବ୍ଦଗୁଡ଼ିକ());
    U.pVal[0] = val;
    memset(&U.pVal[1], 0xFF, APINT_WORD_SIZE * (get_ଅଙ୍କ_ଶବ୍ଦଗୁଡ଼ିକ() - 1));
    clearUnusedBits();
  } ଅନ୍ୟଥା {
    U.pVal = getClearedMemory(get_ଅଙ୍କ_ଶବ୍ଦଗୁଡ଼ିକ());
    U.pVal[0] = val;
  }
}

ଶୂନ୍ୟ APInt::initSlowCase(ଅଚଳ APInt& that) {
  U.pVal = getMemory(get_ଅଙ୍କ_ଶବ୍ଦଗୁଡ଼ିକ());
  memcpy(U.pVal, that.U.pVal, get_ଅଙ୍କ_ଶବ୍ଦଗୁଡ଼ିକ() * APINT_WORD_SIZE);
}

ଶୂନ୍ୟ APInt::initFromArray(ArrayRef<uint64_t> bigVal) {
  ପ୍ରତିଜ୍ଞା(bigVal.data() && "Null pointer detected!");
  ଯଦି (LLVM_LIKELY(isSingleWord()))
    U.VAL = bigVal[0];
  ଅନ୍ୟଥା {
    // Get memory, cleared to 0
    U.pVal = getClearedMemory(get_ଅଙ୍କ_ଶବ୍ଦଗୁଡ଼ିକ());
    // Calculate the number of words to copy
    unsigned ଶବ୍ଦଗୁଡ଼ିକ = std::min<unsigned>(bigVal.size(), get_ଅଙ୍କ_ଶବ୍ଦଗୁଡ଼ିକ());
    // Copy the words from bigVal to pVal
    memcpy(U.pVal, bigVal.data(), ଶବ୍ଦଗୁଡ଼ିକ * APINT_WORD_SIZE);
  }
  // Make sure unused high bits are cleared
  clearUnusedBits();
}

APInt::APInt(unsigned ଅଙ୍କ_Bits, ArrayRef<uint64_t> bigVal) : BitWidth(ଅଙ୍କ_Bits) {
  initFromArray(bigVal);
}

APInt::APInt(unsigned numbits, StringRef Str, uint8_t radix)
    : BitWidth(numbits) {
  fromString(numbits, Str, radix);
}

ଶୂନ୍ୟ APInt::reallocate(unsigned NewBitWidth) {
  // If the number of words is the same we can just change the width and stop.
  ଯଦି (get_ଅଙ୍କ_ଶବ୍ଦଗୁଡ଼ିକ() == get_ଅଙ୍କ_ଶବ୍ଦଗୁଡ଼ିକ(NewBitWidth)) {
    BitWidth = NewBitWidth;
    ଫେରନ୍ତୁ;
  }

  // If we have an allocation, delete it.
  ଯଦି (LLVM_UNLIKELY(!isSingleWord()))
    delete [] U.pVal;

  // Update BitWidth.
  BitWidth = NewBitWidth;

  // If we are supposed to have an allocation, create it.
  ଯଦି (LLVM_UNLIKELY(!isSingleWord()))
    U.pVal = getMemory(get_ଅଙ୍କ_ଶବ୍ଦଗୁଡ଼ିକ());
}

ଶୂନ୍ୟ APInt::assignSlowCase(ଅଚଳ APInt &RHS) {
  // Don't do anything for X = X
  ଯଦି (ଏହି == &RHS)
    ଫେରନ୍ତୁ;

  // Adjust the bit width and handle allocations as necessary.
  reallocate(RHS.getBitWidth());

  // Copy the data.
  ଯଦି (LLVM_LIKELY(isSingleWord()))
    U.VAL = RHS.U.VAL;
  ଅନ୍ୟଥା
    memcpy(U.pVal, RHS.U.pVal, get_ଅଙ୍କ_ଶବ୍ଦଗୁଡ଼ିକ() * APINT_WORD_SIZE);
}

/// This method 'profiles' an APInt for use with FoldingSet.
ଶୂନ୍ୟ APInt::Profile(Folding_Set_ଗଣ୍ଠି_ID& ID) ଅଚଳ {
  ID.AddInteger(BitWidth);

  ଯଦି (LLVM_LIKELY(isSingleWord())) {
    ID.AddInteger(U.VAL);
    ଫେରନ୍ତୁ;
  }

  unsigned ଅଙ୍କ_ଶବ୍ଦଗୁଡ଼ିକ_2 = get_ଅଙ୍କ_ଶବ୍ଦଗୁଡ଼ିକ();
  ପାଇଁ (unsigned i = 0; i < ଅଙ୍କ_ଶବ୍ଦଗୁଡ଼ିକ_2; ++i)
    ID.AddInteger(U.pVal[i]);
}

ତାର୍କିକ APInt::isAligned(Align A) ଅଚଳ {
  ଯଦି (isZero())
    ଫେରନ୍ତୁ ସତ୍ୟ;
  ଅଚଳ unsigned TrailingZeroes = countr_zero();
  ଅଚଳ unsigned MinimumTrailingZeroes = Log2(A);
  ଫେରନ୍ତୁ TrailingZeroes >= MinimumTrailingZeroes;
}

/// Prefix increment operator. Increments the APInt by one.
APInt& APInt::operator++() {
  ଯଦି (LLVM_LIKELY(isSingleWord()))
    ++U.VAL;
  ଅନ୍ୟଥା
    tcIncrement(U.pVal, get_ଅଙ୍କ_ଶବ୍ଦଗୁଡ଼ିକ());
  ଫେରନ୍ତୁ clearUnusedBits();
}

/// Prefix decrement operator. Decrements the APInt by one.
APInt& APInt::operator--() {
  ଯଦି (LLVM_LIKELY(isSingleWord()))
    --U.VAL;
  ଅନ୍ୟଥା
    tcDecrement(U.pVal, get_ଅଙ୍କ_ଶବ୍ଦଗୁଡ଼ିକ());
  ଫେରନ୍ତୁ clearUnusedBits();
}

/// Adds the RHS APInt to this APInt.
/// @returns this, after addition of RHS.
/// Addition assignment operator.
APInt& APInt::operator+=(ଅଚଳ APInt& RHS) {
  ପ୍ରତିଜ୍ଞା(BitWidth == RHS.BitWidth && "Bit widths must be the same");
  ଯଦି (LLVM_LIKELY(isSingleWord()))
    U.VAL += RHS.U.VAL;
  ଅନ୍ୟଥା
    tcAdd(U.pVal, RHS.U.pVal, 0, get_ଅଙ୍କ_ଶବ୍ଦଗୁଡ଼ିକ());
  ଫେରନ୍ତୁ clearUnusedBits();
}

APInt& APInt::operator+=(uint64_t RHS) {
  ଯଦି (LLVM_LIKELY(isSingleWord()))
    U.VAL += RHS;
  ଅନ୍ୟଥା
    tcAddPart(U.pVal, RHS, get_ଅଙ୍କ_ଶବ୍ଦଗୁଡ଼ିକ());
  ଫେରନ୍ତୁ clearUnusedBits();
}

/// Subtracts the RHS APInt from this APInt
/// @returns this, after subtraction
/// Subtraction assignment operator.
APInt& APInt::operator-=(ଅଚଳ APInt& RHS) {
  ପ୍ରତିଜ୍ଞା(BitWidth == RHS.BitWidth && "Bit widths must be the same");
  ଯଦି (LLVM_LIKELY(isSingleWord()))
    U.VAL -= RHS.U.VAL;
  ଅନ୍ୟଥା
    tcSubtract(U.pVal, RHS.U.pVal, 0, get_ଅଙ୍କ_ଶବ୍ଦଗୁଡ଼ିକ());
  ଫେରନ୍ତୁ clearUnusedBits();
}

APInt& APInt::operator-=(uint64_t RHS) {
  ଯଦି (LLVM_LIKELY(isSingleWord()))
    U.VAL -= RHS;
  ଅନ୍ୟଥା
    tcSubtractPart(U.pVal, RHS, get_ଅଙ୍କ_ଶବ୍ଦଗୁଡ଼ିକ());
  ଫେରନ୍ତୁ clearUnusedBits();
}

APInt APInt::operator*(ଅଚଳ APInt& RHS) ଅଚଳ {
  ପ୍ରତିଜ୍ଞା(BitWidth == RHS.BitWidth && "Bit widths must be the same");
  ଯଦି (LLVM_LIKELY(isSingleWord()))
    ଫେରନ୍ତୁ APInt(BitWidth, U.VAL * RHS.U.VAL, /*isSigned=*/ମିଥ୍ୟା,
                 /*implicitTrunc=*/ସତ୍ୟ);

  APInt Result(getMemory(get_ଅଙ୍କ_ଶବ୍ଦଗୁଡ଼ିକ()), getBitWidth());
  tcMultiply(Result.U.pVal, U.pVal, RHS.U.pVal, get_ଅଙ୍କ_ଶବ୍ଦଗୁଡ଼ିକ());
  Result.clearUnusedBits();
  ଫେରନ୍ତୁ Result;
}

ଶୂନ୍ୟ APInt::andAssignSlowCase(ଅଚଳ APInt &RHS) {
  WordType *dst = U.pVal, *rhs = RHS.U.pVal;
  ପାଇଁ (size_t i = 0, e = get_ଅଙ୍କ_ଶବ୍ଦଗୁଡ଼ିକ(); i != e; ++i)
    dst[i] &= rhs[i];
}

ଶୂନ୍ୟ APInt::orAssignSlowCase(ଅଚଳ APInt &RHS) {
  WordType *dst = U.pVal, *rhs = RHS.U.pVal;
  ପାଇଁ (size_t i = 0, e = get_ଅଙ୍କ_ଶବ୍ଦଗୁଡ଼ିକ(); i != e; ++i)
    dst[i] |= rhs[i];
}

ଶୂନ୍ୟ APInt::xorAssignSlowCase(ଅଚଳ APInt &RHS) {
  WordType *dst = U.pVal, *rhs = RHS.U.pVal;
  ପାଇଁ (size_t i = 0, e = get_ଅଙ୍କ_ଶବ୍ଦଗୁଡ଼ିକ(); i != e; ++i)
    dst[i] ^= rhs[i];
}

APInt &APInt::operator*=(ଅଚଳ APInt &RHS) {
  *ଏହି = *ଏହି * RHS;
  ଫେରନ୍ତୁ *ଏହି;
}

APInt& APInt::operator*=(uint64_t RHS) {
  ଯଦି (LLVM_LIKELY(isSingleWord())) {
    U.VAL *= RHS;
  } ଅନ୍ୟଥା {
    unsigned ଅଙ୍କ_ଶବ୍ଦଗୁଡ଼ିକ_2 = get_ଅଙ୍କ_ଶବ୍ଦଗୁଡ଼ିକ();
    tcMultiplyPart(U.pVal, U.pVal, RHS, 0, ଅଙ୍କ_ଶବ୍ଦଗୁଡ଼ିକ_2, ଅଙ୍କ_ଶବ୍ଦଗୁଡ଼ିକ_2, ମିଥ୍ୟା);
  }
  ଫେରନ୍ତୁ clearUnusedBits();
}

ତାର୍କିକ APInt::equalSlowCase(ଅଚଳ APInt &RHS) ଅଚଳ {
  ଫେରନ୍ତୁ std::equal(U.pVal, U.pVal + get_ଅଙ୍କ_ଶବ୍ଦଗୁଡ଼ିକ(), RHS.U.pVal);
}

ପୂର୍ଣ୍ଣସଂଖ୍ୟା APInt::compare(ଅଚଳ APInt& RHS) ଅଚଳ {
  ପ୍ରତିଜ୍ଞା(BitWidth == RHS.BitWidth && "Bit widths must be same for comparison");
  ଯଦି (LLVM_LIKELY(isSingleWord()))
    ଫେରନ୍ତୁ U.VAL < RHS.U.VAL ? -1 : U.VAL > RHS.U.VAL;

  ଫେରନ୍ତୁ tcCompare(U.pVal, RHS.U.pVal, get_ଅଙ୍କ_ଶବ୍ଦଗୁଡ଼ିକ());
}

ପୂର୍ଣ୍ଣସଂଖ୍ୟା APInt::compareSigned(ଅଚଳ APInt& RHS) ଅଚଳ {
  ପ୍ରତିଜ୍ଞା(BitWidth == RHS.BitWidth && "Bit widths must be same for comparison");
  ଯଦି (LLVM_LIKELY(isSingleWord())) {
    int64_t lhsSext = SignExtend64(U.VAL, BitWidth);
    int64_t rhsSext = SignExtend64(RHS.U.VAL, BitWidth);
    ଫେରନ୍ତୁ lhsSext < rhsSext ? -1 : lhsSext > rhsSext;
  }

  ତାର୍କିକ lhsNeg = isNegative();
  ତାର୍କିକ rhsNeg = RHS.isNegative();

  // If the sign bits don't match, then (LHS < RHS) if LHS is negative
  ଯଦି (lhsNeg != rhsNeg)
    ଫେରନ୍ତୁ lhsNeg ? -1 : 1;

  // Otherwise we can just use an unsigned comparison, because even negative
  // numbers compare correctly this way if both have the same signed-ness.
  ଫେରନ୍ତୁ tcCompare(U.pVal, RHS.U.pVal, get_ଅଙ୍କ_ଶବ୍ଦଗୁଡ଼ିକ());
}

ଶୂନ୍ୟ APInt::setBitsSlowCase(unsigned loBit, unsigned hiBit) {
  unsigned loWord = whichWord(loBit);
  unsigned hiWord = whichWord(hiBit);

  // Create an initial mask for the low word with zeros below loBit.
  uint64_t loMask = WORDTYPE_MAX << whichBit(loBit);

  // If hiBit is not aligned, we need a high mask.
  unsigned hiShiftAmt = whichBit(hiBit);
  ଯଦି (hiShiftAmt != 0) {
    // Create a high mask with zeros above hiBit.
    uint64_t hiMask = WORDTYPE_MAX >> (APINT_BITS_PER_WORD - hiShiftAmt);
    // If loWord and hiWord are equal, then we combine the masks. Otherwise,
    // set the bits in hiWord.
    ଯଦି (hiWord == loWord)
      loMask &= hiMask;
    ଅନ୍ୟଥା
      U.pVal[hiWord] |= hiMask;
  }
  // Apply the mask to the low word.
  U.pVal[loWord] |= loMask;

  // Fill any words between loWord and hiWord with all ones.
  ପାଇଁ (unsigned word = loWord + 1; word < hiWord; ++word)
    U.pVal[word] = WORDTYPE_MAX;
}

ଶୂନ୍ୟ APInt::clearBitsSlowCase(unsigned LoBit, unsigned HiBit) {
  unsigned LoWord = whichWord(LoBit);
  unsigned HiWord = whichWord(HiBit);

  // Create an initial mask for the low word with ones below loBit.
  uint64_t LoMask = ~(WORDTYPE_MAX << whichBit(LoBit));

  // If HiBit is not aligned, we need a high mask.
  unsigned HiShiftAmt = whichBit(HiBit);
  ଯଦି (HiShiftAmt != 0) {
    // Create a high mask with ones above HiBit.
    uint64_t HiMask = ~(WORDTYPE_MAX >> (APINT_BITS_PER_WORD - HiShiftAmt));
    // If LoWord and HiWord are equal, then we combine the masks. Otherwise,
    // clear the bits in HiWord.
    ଯଦି (HiWord == LoWord)
      LoMask |= HiMask;
    ଅନ୍ୟଥା
      U.pVal[HiWord] &= HiMask;
  }
  // Apply the mask to the low word.
  U.pVal[LoWord] &= LoMask;

  // Fill any words between LoWord and HiWord with all zeros.
  ପାଇଁ (unsigned Word = LoWord + 1; Word < HiWord; ++Word)
    U.pVal[Word] = 0;
}

// Complement a bignum in-place.
ସ୍ଥିର ଶୂନ୍ୟ tcComplement(APInt::WordType *dst, unsigned parts) {
  ପାଇଁ (unsigned i = 0; i < parts; i++)
    dst[i] = ~dst[i];
}

/// Toggle every bit to its opposite value.
ଶୂନ୍ୟ APInt::flipAllBitsSlowCase() {
  tcComplement(U.pVal, get_ଅଙ୍କ_ଶବ୍ଦଗୁଡ଼ିକ());
  clearUnusedBits();
}

/// Concatenate the bits from "NewLSB" onto the bottom of *this.  This is
/// equivalent to:
///   (this->zext(NewWidth) << NewLSB.getBitWidth()) | NewLSB.zext(NewWidth)
/// In the slow case, we know the result is large.
APInt APInt::concatSlowCase(ଅଚଳ APInt &NewLSB) ଅଚଳ {
  unsigned NewWidth = getBitWidth() + NewLSB.getBitWidth();
  APInt Result = NewLSB.zext(NewWidth);
  Result.ଭର୍ତ୍ତି_Bits(*ଏହି, NewLSB.getBitWidth());
  ଫେରନ୍ତୁ Result;
}

/// Toggle a given bit to its opposite value whose position is given
/// as "bitPosition".
/// Toggles a given bit to its opposite value.
ଶୂନ୍ୟ APInt::flipBit(unsigned bitPosition) {
  ପ୍ରତିଜ୍ଞା(bitPosition < BitWidth && "Out of the bit-width range!");
  setBitVal(bitPosition, !(*ଏହି)[bitPosition]);
}

ଶୂନ୍ୟ APInt::ଭର୍ତ୍ତି_Bits(ଅଚଳ APInt &subBits, unsigned bitPosition) {
  unsigned subBitWidth = subBits.getBitWidth();
  ପ୍ରତିଜ୍ଞା((subBitWidth + bitPosition) <= BitWidth && "Illegal bit insertion");

  // inserting no bits is a noop.
  ଯଦି (subBitWidth == 0)
    ଫେରନ୍ତୁ;

  // Insertion is a direct copy.
  ଯଦି (subBitWidth == BitWidth) {
    *ଏହି = subBits;
    ଫେରନ୍ତୁ;
  }

  // Single word result can be done as a direct bitmask.
  ଯଦି (LLVM_LIKELY(isSingleWord())) {
    uint64_t mask = WORDTYPE_MAX >> (APINT_BITS_PER_WORD - subBitWidth);
    U.VAL &= ~(mask << bitPosition);
    U.VAL |= (subBits.U.VAL << bitPosition);
    ଫେରନ୍ତୁ;
  }

  unsigned loBit = whichBit(bitPosition);
  unsigned loWord = whichWord(bitPosition);
  unsigned hi1Word = whichWord(bitPosition + subBitWidth - 1);

  // Insertion within a single word can be done as a direct bitmask.
  ଯଦି (loWord == hi1Word) {
    uint64_t mask = WORDTYPE_MAX >> (APINT_BITS_PER_WORD - subBitWidth);
    U.pVal[loWord] &= ~(mask << loBit);
    U.pVal[loWord] |= (subBits.U.VAL << loBit);
    ଫେରନ୍ତୁ;
  }

  // Insert on word boundaries.
  ଯଦି (loBit == 0) {
    // Direct copy whole words.
    unsigned ଅଙ୍କ_Whole_Sub_ଶବ୍ଦଗୁଡ଼ିକ = subBitWidth / APINT_BITS_PER_WORD;
    memcpy(U.pVal + loWord, subBits.getRawData(),
           ଅଙ୍କ_Whole_Sub_ଶବ୍ଦଗୁଡ଼ିକ * APINT_WORD_SIZE);

    // Mask+insert remaining bits.
    unsigned remainingBits = subBitWidth % APINT_BITS_PER_WORD;
    ଯଦି (remainingBits != 0) {
      uint64_t mask = WORDTYPE_MAX >> (APINT_BITS_PER_WORD - remainingBits);
      U.pVal[hi1Word] &= ~mask;
      U.pVal[hi1Word] |= subBits.getWord(subBitWidth - 1);
    }
    ଫେରନ୍ତୁ;
  }

  // General case - set/clear individual bits in dst based on src.
  // TODO - there is scope for optimization here, but at the moment this code
  // path is barely used so prefer readability over performance.
  ପାଇଁ (unsigned i = 0; i != subBitWidth; ++i)
    setBitVal(bitPosition + i, subBits[i]);
}

ଶୂନ୍ୟ APInt::ଭର୍ତ୍ତି_Bits(uint64_t subBits, unsigned bitPosition, unsigned ଅଙ୍କ_Bits) {
  uint64_t maskBits = maskTrailingOnes<uint64_t>(ଅଙ୍କ_Bits);
  subBits &= maskBits;
  ଯଦି (LLVM_LIKELY(isSingleWord())) {
    U.VAL &= ~(maskBits << bitPosition);
    U.VAL |= subBits << bitPosition;
    ଫେରନ୍ତୁ;
  }

  unsigned loBit = whichBit(bitPosition);
  unsigned loWord = whichWord(bitPosition);
  unsigned hiWord = whichWord(bitPosition + ଅଙ୍କ_Bits - 1);
  ଯଦି (loWord == hiWord) {
    U.pVal[loWord] &= ~(maskBits << loBit);
    U.pVal[loWord] |= subBits << loBit;
    ଫେରନ୍ତୁ;
  }

  static_assert(8 * ଆକାର(WordType) <= 64, "This code assumes only two words affected");
  unsigned wordBits = 8 * ଆକାର(WordType);
  U.pVal[loWord] &= ~(maskBits << loBit);
  U.pVal[loWord] |= subBits << loBit;

  U.pVal[hiWord] &= ~(maskBits >> (wordBits - loBit));
  U.pVal[hiWord] |= subBits >> (wordBits - loBit);
}

APInt APInt::extractBits(unsigned ଅଙ୍କ_Bits, unsigned bitPosition) ଅଚଳ {
  ପ୍ରତିଜ୍ଞା(bitPosition < BitWidth && (ଅଙ୍କ_Bits + bitPosition) <= BitWidth &&
         "Illegal bit extraction");

  ଯଦି (LLVM_LIKELY(isSingleWord()))
    ଫେରନ୍ତୁ APInt(ଅଙ୍କ_Bits, U.VAL >> bitPosition, /*isSigned=*/ମିଥ୍ୟା,
                 /*implicitTrunc=*/ସତ୍ୟ);

  unsigned loBit = whichBit(bitPosition);
  unsigned loWord = whichWord(bitPosition);
  unsigned hiWord = whichWord(bitPosition + ଅଙ୍କ_Bits - 1);

  // Single word result extracting bits from a single word source.
  ଯଦି (loWord == hiWord)
    ଫେରନ୍ତୁ APInt(ଅଙ୍କ_Bits, U.pVal[loWord] >> loBit, /*isSigned=*/ମିଥ୍ୟା,
                 /*implicitTrunc=*/ସତ୍ୟ);

  // Extracting bits that start on a source word boundary can be done
  // as a fast memory copy.
  ଯଦି (loBit == 0)
    ଫେରନ୍ତୁ APInt(ଅଙ୍କ_Bits, ArrayRef(U.pVal + loWord, 1 + hiWord - loWord));

  // General case - shift + copy source words directly into place.
  APInt Result(ଅଙ୍କ_Bits, 0);
  unsigned ଅଙ୍କ_Src_ଶବ୍ଦଗୁଡ଼ିକ = get_ଅଙ୍କ_ଶବ୍ଦଗୁଡ଼ିକ();
  unsigned ଅଙ୍କ_Dst_ଶବ୍ଦଗୁଡ଼ିକ = Result.get_ଅଙ୍କ_ଶବ୍ଦଗୁଡ଼ିକ();

  uint64_t *DestPtr =
      LLVM_LIKELY(Result.isSingleWord()) ? &Result.U.VAL : Result.U.pVal;
  ପାଇଁ (unsigned word = 0; word < ଅଙ୍କ_Dst_ଶବ୍ଦଗୁଡ଼ିକ; ++word) {
    uint64_t w0 = U.pVal[loWord + word];
    uint64_t w1 =
        (loWord + word + 1) < ଅଙ୍କ_Src_ଶବ୍ଦଗୁଡ଼ିକ ? U.pVal[loWord + word + 1] : 0;
    DestPtr[word] = (w0 >> loBit) | (w1 << (APINT_BITS_PER_WORD - loBit));
  }

  ଫେରନ୍ତୁ Result.clearUnusedBits();
}

uint64_t APInt::extract_Bits_As_ZExt_ମୂଲ୍ୟ(unsigned ଅଙ୍କ_Bits,
                                       unsigned bitPosition) ଅଚଳ {
  ପ୍ରତିଜ୍ଞା(bitPosition < BitWidth && (ଅଙ୍କ_Bits + bitPosition) <= BitWidth &&
         "Illegal bit extraction");
  ପ୍ରତିଜ୍ଞା(ଅଙ୍କ_Bits <= 64 && "Illegal bit extraction");

  uint64_t maskBits = maskTrailingOnes<uint64_t>(ଅଙ୍କ_Bits);
  ଯଦି (LLVM_LIKELY(isSingleWord()))
    ଫେରନ୍ତୁ (U.VAL >> bitPosition) & maskBits;

  static_assert(APINT_BITS_PER_WORD >= 64,
                "This code assumes only two words affected");
  unsigned loBit = whichBit(bitPosition);
  unsigned loWord = whichWord(bitPosition);
  unsigned hiWord = whichWord(bitPosition + ଅଙ୍କ_Bits - 1);
  ଯଦି (loWord == hiWord)
    ଫେରନ୍ତୁ (U.pVal[loWord] >> loBit) & maskBits;

  uint64_t retBits = U.pVal[loWord] >> loBit;
  retBits |= U.pVal[hiWord] << (APINT_BITS_PER_WORD - loBit);
  retBits &= maskBits;
  ଫେରନ୍ତୁ retBits;
}

unsigned APInt::getSufficientBitsNeeded(StringRef Str, uint8_t Radix) {
  ପ୍ରତିଜ୍ଞା(!Str.empty() && "Invalid string length");
  size_t StrLen = Str.size();

  // Each computation below needs to know if it's negative.
  unsigned IsNegative = ମିଥ୍ୟା;
  ଯଦି (Str[0] == '-' || Str[0] == '+') {
    IsNegative = Str[0] == '-';
    StrLen--;
    ପ୍ରତିଜ୍ଞା(StrLen && "String is only a sign, needs a value.");
  }

  // For radixes of power-of-two values, the bits required is accurately and
  // easily computed.
  ଯଦି (Radix == 2)
    ଫେରନ୍ତୁ StrLen + IsNegative;
  ଯଦି (Radix == 8)
    ଫେରନ୍ତୁ StrLen * 3 + IsNegative;
  ଯଦି (Radix == 16)
    ଫେରନ୍ତୁ StrLen * 4 + IsNegative;

  // Compute a sufficient number of bits that is always large enough but might
  // be too large. This avoids the assertion in the constructor. This
  // calculation doesn't work appropriately for the numbers 0-9, so just use 4
  // bits in that case.
  ଯଦି (Radix == 10)
    ଫେରନ୍ତୁ (StrLen == 1 ? 4 : StrLen * 64 / 18) + IsNegative;

  ପ୍ରତିଜ୍ଞା(Radix == 36);
  ଫେରନ୍ତୁ (StrLen == 1 ? 7 : StrLen * 16 / 3) + IsNegative;
}

unsigned APInt::getBitsNeeded(StringRef str, uint8_t radix) {
  // Compute a sufficient number of bits that is always large enough but might
  // be too large.
  unsigned sufficient = getSufficientBitsNeeded(str, radix);

  // For bases 2, 8, and 16, the sufficient number of bits is exact and we can
  // return the value directly. For bases 10 and 36, we need to do extra work.
  ଯଦି (radix == 2 || radix == 8 || radix == 16)
    ଫେରନ୍ତୁ sufficient;

  // This is grossly inefficient but accurate. We could probably do something
  // with a computation of roughly slen*64/20 and then adjust by the value of
  // the first few digits. But, I'm not sure how accurate that could be.
  size_t slen = str.size();

  // Each computation below needs to know if it's negative.
  StringRef::iterator p = str.begin();
  unsigned isNegative = *p == '-';
  ଯଦି (*p == '-' || *p == '+') {
    p++;
    slen--;
    ପ୍ରତିଜ୍ଞା(slen && "String is only a sign, needs a value.");
  }


  // Convert to the actual binary value.
  APInt tmp(sufficient, StringRef(p, slen), radix);

  // Compute how many bits are required. If the log is infinite, assume we need
  // just bit. If the log is exact and value is negative, then the value is
  // MinSignedValue with (log + 1) bits.
  unsigned log = tmp.logBase2();
  ଯଦି (log == (unsigned)-1) {
    ଫେରନ୍ତୁ isNegative + 1;
  } ଅନ୍ୟଥା ଯଦି (isNegative && tmp.isPowerOf2()) {
    ଫେରନ୍ତୁ isNegative + log;
  } ଅନ୍ୟଥା {
    ଫେରନ୍ତୁ isNegative + log + 1;
  }
}

hash_code llvm::hash_ମୂଲ୍ୟ(ଅଚଳ APInt &Arg) {
  ଯଦି (LLVM_LIKELY(Arg.isSingleWord()))
    ଫେରନ୍ତୁ hash_combine(Arg.BitWidth, Arg.U.VAL);

  ଫେରନ୍ତୁ hash_combine(
      Arg.BitWidth,
      hash_combine_range(Arg.U.pVal, Arg.U.pVal + Arg.get_ଅଙ୍କ_ଶବ୍ଦଗୁଡ଼ିକ()));
}

unsigned DenseMapInfo<APInt, ଶୂନ୍ୟ>::get_Hash_ମୂଲ୍ୟ(ଅଚଳ APInt &ଚାବି) {
  ଫେରନ୍ତୁ static_cast<unsigned>(hash_ମୂଲ୍ୟ(ଚାବି));
}

ତାର୍କିକ APInt::isSplat(unsigned SplatSizeInBits) ଅଚଳ {
  ପ୍ରତିଜ୍ଞା(getBitWidth() % SplatSizeInBits == 0 &&
         "SplatSizeInBits must divide width!");
  // We can check that all parts of an integer are equal by making use of a
  // little trick: rotate and check if it's still the same value.
  ଫେରନ୍ତୁ *ଏହି == rotl(SplatSizeInBits);
}

/// This function returns the high "numBits" bits of this APInt.
APInt APInt::getHiBits(unsigned ଅଙ୍କ_Bits) ଅଚଳ {
  ଫେରନ୍ତୁ ଏହି->lshr(BitWidth - ଅଙ୍କ_Bits);
}

/// This function returns the low "numBits" bits of this APInt.
APInt APInt::getLoBits(unsigned ଅଙ୍କ_Bits) ଅଚଳ {
  APInt Result(get_ତଳ_Bits_Set(BitWidth, ଅଙ୍କ_Bits));
  Result &= *ଏହି;
  ଫେରନ୍ତୁ Result;
}

/// Return a value containing V broadcasted over NewLen bits.
APInt APInt::getSplat(unsigned NewLen, ଅଚଳ APInt &V) {
  ପ୍ରତିଜ୍ଞା(NewLen >= V.getBitWidth() && "Can't splat to smaller bit width!");

  APInt Val = V.zext(NewLen);
  ପାଇଁ (unsigned I = V.getBitWidth(); I < NewLen; I <<= 1)
    Val |= Val << I;

  ଫେରନ୍ତୁ Val;
}

unsigned APInt::ଗଣନା_Leading_Zeros_Slow_Case() ଅଚଳ {
  unsigned ଗଣନା_3 = 0;
  ପାଇଁ (ପୂର୍ଣ୍ଣସଂଖ୍ୟା i = get_ଅଙ୍କ_ଶବ୍ଦଗୁଡ଼ିକ() - 1; i >= 0; --i) {
    uint64_t V = U.pVal[i];
    ଯଦି (V == 0)
      ଗଣନା_3 += APINT_BITS_PER_WORD;
    ଅନ୍ୟଥା {
      ଗଣନା_3 += llvm::countl_zero(V);
      ଭାଙ୍ଗ;
    }
  }
  // Adjust for unused bits in the most significant word (they are zero).
  unsigned Mod = BitWidth % APINT_BITS_PER_WORD;
  ଗଣନା_3 -= Mod > 0 ? APINT_BITS_PER_WORD - Mod : 0;
  ଫେରନ୍ତୁ ଗଣନା_3;
}

unsigned APInt::ଗଣନା_Leading_Ones_Slow_Case() ଅଚଳ {
  unsigned ଉପର_Word_Bits = BitWidth % APINT_BITS_PER_WORD;
  unsigned shift;
  ଯଦି (!ଉପର_Word_Bits) {
    ଉପର_Word_Bits = APINT_BITS_PER_WORD;
    shift = 0;
  } ଅନ୍ୟଥା {
    shift = APINT_BITS_PER_WORD - ଉପର_Word_Bits;
  }
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା i = get_ଅଙ୍କ_ଶବ୍ଦଗୁଡ଼ିକ() - 1;
  unsigned ଗଣନା_3 = llvm::countl_one(U.pVal[i] << shift);
  ଯଦି (ଗଣନା_3 == ଉପର_Word_Bits) {
    ପାଇଁ (i--; i >= 0; --i) {
      ଯଦି (U.pVal[i] == WORDTYPE_MAX)
        ଗଣନା_3 += APINT_BITS_PER_WORD;
      ଅନ୍ୟଥା {
        ଗଣନା_3 += llvm::countl_one(U.pVal[i]);
        ଭାଙ୍ଗ;
      }
    }
  }
  ଫେରନ୍ତୁ ଗଣନା_3;
}

unsigned APInt::ଗଣନା_Trailing_Zeros_Slow_Case() ଅଚଳ {
  unsigned ଗଣନା_3 = 0;
  unsigned i = 0;
  ପାଇଁ (; i < get_ଅଙ୍କ_ଶବ୍ଦଗୁଡ଼ିକ() && U.pVal[i] == 0; ++i)
    ଗଣନା_3 += APINT_BITS_PER_WORD;
  ଯଦି (i < get_ଅଙ୍କ_ଶବ୍ଦଗୁଡ଼ିକ())
    ଗଣନା_3 += llvm::countr_zero(U.pVal[i]);
  ଫେରନ୍ତୁ std::min(ଗଣନା_3, BitWidth);
}

unsigned APInt::ଗଣନା_Trailing_Ones_Slow_Case() ଅଚଳ {
  unsigned ଗଣନା_3 = 0;
  unsigned i = 0;
  ପାଇଁ (; i < get_ଅଙ୍କ_ଶବ୍ଦଗୁଡ଼ିକ() && U.pVal[i] == WORDTYPE_MAX; ++i)
    ଗଣନା_3 += APINT_BITS_PER_WORD;
  ଯଦି (i < get_ଅଙ୍କ_ଶବ୍ଦଗୁଡ଼ିକ())
    ଗଣନା_3 += llvm::countr_one(U.pVal[i]);
  ପ୍ରତିଜ୍ଞା(ଗଣନା_3 <= BitWidth);
  ଫେରନ୍ତୁ ଗଣନା_3;
}

unsigned APInt::ଗଣନା_Population_Slow_Case() ଅଚଳ {
  unsigned ଗଣନା_3 = 0;
  ପାଇଁ (unsigned i = 0; i < get_ଅଙ୍କ_ଶବ୍ଦଗୁଡ଼ିକ(); ++i)
    ଗଣନା_3 += llvm::popcount(U.pVal[i]);
  ଫେରନ୍ତୁ ଗଣନା_3;
}

ତାର୍କିକ APInt::isPowerOf2SlowCase() ଅଚଳ {
  unsigned ଗଣନା_3 = 0;
  ପାଇଁ (unsigned i = 0; i < get_ଅଙ୍କ_ଶବ୍ଦଗୁଡ଼ିକ(); ++i) {
    ଗଣନା_3 += llvm::popcount(U.pVal[i]);
    ଯଦି (ଗଣନା_3 > 1)
      ଫେରନ୍ତୁ ମିଥ୍ୟା;
  }
  ଫେରନ୍ତୁ ଗଣନା_3 == 1;
}

ତାର୍କିକ APInt::intersectsSlowCase(ଅଚଳ APInt &RHS) ଅଚଳ {
  ପାଇଁ (unsigned i = 0, e = get_ଅଙ୍କ_ଶବ୍ଦଗୁଡ଼ିକ(); i != e; ++i)
    ଯଦି ((U.pVal[i] & RHS.U.pVal[i]) != 0)
      ଫେରନ୍ତୁ ସତ୍ୟ;

  ଫେରନ୍ତୁ ମିଥ୍ୟା;
}

ତାର୍କିକ APInt::isSubsetOfSlowCase(ଅଚଳ APInt &RHS) ଅଚଳ {
  ପାଇଁ (unsigned i = 0, e = get_ଅଙ୍କ_ଶବ୍ଦଗୁଡ଼ିକ(); i != e; ++i)
    ଯଦି ((U.pVal[i] & ~RHS.U.pVal[i]) != 0)
      ଫେରନ୍ତୁ ମିଥ୍ୟା;

  ଫେରନ୍ତୁ ସତ୍ୟ;
}

ତାର୍କିକ APInt::isInverseOfSlowCase(ଅଚଳ APInt &RHS) ଅଚଳ {
  ଅଚଳ unsigned Last = get_ଅଙ୍କ_ଶବ୍ଦଗୁଡ଼ିକ() - 1;
  ପାଇଁ (unsigned I = 0; I != Last; ++I)
    ଯଦି ((U.pVal[I] ^ RHS.U.pVal[I]) != WORDTYPE_MAX)
      ଫେରନ୍ତୁ ମିଥ୍ୟା;

  unsigned TailBits = BitWidth - Last * APINT_BITS_PER_WORD;
  WordType TailMask = llvm::maskTrailingOnes<WordType>(TailBits);
  ଫେରନ୍ତୁ (U.pVal[Last] ^ RHS.U.pVal[Last]) == TailMask;
}

APInt APInt::byteSwap() ଅଚଳ {
  ପ୍ରତିଜ୍ଞା(BitWidth >= 16 && BitWidth % 8 == 0 && "Cannot byteswap!");
  ଯଦି (BitWidth == 16)
    ଫେରନ୍ତୁ APInt(BitWidth, llvm::byteswap<uint16_t>(U.VAL));
  ଯଦି (BitWidth == 32)
    ଫେରନ୍ତୁ APInt(BitWidth, llvm::byteswap<uint32_t>(U.VAL));
  ଯଦି (BitWidth <= 64) {
    uint64_t Tmp1 = llvm::byteswap<uint64_t>(U.VAL);
    Tmp1 >>= (64 - BitWidth);
    ଫେରନ୍ତୁ APInt(BitWidth, Tmp1);
  }

  APInt Result(get_ଅଙ୍କ_ଶବ୍ଦଗୁଡ଼ିକ() * APINT_BITS_PER_WORD, 0);
  ପାଇଁ (unsigned I = 0, N = get_ଅଙ୍କ_ଶବ୍ଦଗୁଡ଼ିକ(); I != N; ++I)
    Result.U.pVal[I] = llvm::byteswap<uint64_t>(U.pVal[N - I - 1]);
  ଯଦି (Result.BitWidth != BitWidth) {
    Result.lshrInPlace(Result.BitWidth - BitWidth);
    Result.BitWidth = BitWidth;
  }
  ଫେରନ୍ତୁ Result;
}

APInt APInt::reverseBits() ଅଚଳ {
  ଯଦି (LLVM_LIKELY(isSingleWord())) {
    ବାଛ (BitWidth) {
    ସ୍ଥିତି 64:
      ଫେରନ୍ତୁ APInt(BitWidth, llvm::reverseBits<uint64_t>(U.VAL));
    ସ୍ଥିତି 32:
      ଫେରନ୍ତୁ APInt(BitWidth, llvm::reverseBits<uint32_t>(U.VAL));
    ସ୍ଥିତି 16:
      ଫେରନ୍ତୁ APInt(BitWidth, llvm::reverseBits<uint16_t>(U.VAL));
    ସ୍ଥିତି 8:
      ଫେରନ୍ତୁ APInt(BitWidth, llvm::reverseBits<uint8_t>(U.VAL));
    ସ୍ଥିତି 1: // fallthrough
    ସ୍ଥିତି 0:
      ଫେରନ୍ତୁ *ଏହି;
    ପୂର୍ବନିର୍ଦ୍ଧାରିତ:
      ଫେରନ୍ତୁ APInt(BitWidth,
                   llvm::reverseBits<uint64_t>(U.VAL) >> (64 - BitWidth));
    }
  }

  APInt Result(BitWidth, 0);
  unsigned ଅଙ୍କ_ଶବ୍ଦଗୁଡ଼ିକ_2 = get_ଅଙ୍କ_ଶବ୍ଦଗୁଡ଼ିକ();
  unsigned ExcessBits = ଅଙ୍କ_ଶବ୍ଦଗୁଡ଼ିକ_2 * APINT_BITS_PER_WORD - BitWidth;
  ଯଦି (ExcessBits == 0) {
    // Fast path. No cross-word shift needed.
    ପାଇଁ (unsigned I = 0; I < ଅଙ୍କ_ଶବ୍ଦଗୁଡ଼ିକ_2; ++I)
      Result.U.pVal[I] = llvm::reverseBits<uint64_t>(U.pVal[ଅଙ୍କ_ଶବ୍ଦଗୁଡ଼ିକ_2 - 1 - I]);
    ଫେରନ୍ତୁ Result;
  }
  // Holds reversed bits of the previous (more significant) word.
  uint64_t PrevRev = llvm::reverseBits<uint64_t>(U.pVal[ଅଙ୍କ_ଶବ୍ଦଗୁଡ଼ିକ_2 - 1]);
  ପାଇଁ (unsigned I = 0; I < ଅଙ୍କ_ଶବ୍ଦଗୁଡ଼ିକ_2 - 1; ++I) {
    uint64_t CurrRev = llvm::reverseBits<uint64_t>(U.pVal[ଅଙ୍କ_ଶବ୍ଦଗୁଡ଼ିକ_2 - 2 - I]);
    Result.U.pVal[I] = (PrevRev >> ExcessBits) | (CurrRev << (64 - ExcessBits));
    PrevRev = CurrRev;
  }
  Result.U.pVal[ଅଙ୍କ_ଶବ୍ଦଗୁଡ଼ିକ_2 - 1] = PrevRev >> ExcessBits;
  ଫେରନ୍ତୁ Result;
}

APInt llvm::APIntOps::Greatest_Common_ଭାଜକ(APInt A, APInt B, ତାର୍କିକ IsSigned) {
  // Take absolute value if IsSigned.
  ଯଦି (IsSigned) {
    A = A.abs();
    B = B.abs();
  }

  // Fast-path a common case.
  ଯଦି (A == B) ଫେରନ୍ତୁ A;

  // Corner cases: if either operand is zero, the other is the gcd.
  ଯଦି (!A) ଫେରନ୍ତୁ B;
  ଯଦି (!B) ଫେରନ୍ତୁ A;

  // Count common powers of 2 and remove all other powers of 2.
  unsigned Pow2;
  {
    unsigned Pow2_A = A.countr_zero();
    unsigned Pow2_B = B.countr_zero();
    ଯଦି (Pow2_A > Pow2_B) {
      A.lshrInPlace(Pow2_A - Pow2_B);
      Pow2 = Pow2_B;
    } ଅନ୍ୟଥା ଯଦି (Pow2_B > Pow2_A) {
      B.lshrInPlace(Pow2_B - Pow2_A);
      Pow2 = Pow2_A;
    } ଅନ୍ୟଥା {
      Pow2 = Pow2_A;
    }
  }

  // Both operands are odd multiples of 2^Pow_2:
  //
  //   gcd(a, b) = gcd(|a - b| / 2^i, min(a, b))
  //
  // This is a modified version of Stein's algorithm, taking advantage of
  // efficient countTrailingZeros().
  ଯେତେବେଳେ (A != B) {
    ଯଦି (A.ugt(B)) {
      A -= B;
      A.lshrInPlace(A.countr_zero() - Pow2);
    } ଅନ୍ୟଥା {
      B -= A;
      B.lshrInPlace(B.countr_zero() - Pow2);
    }
  }

  ଫେରନ୍ତୁ A;
}

APInt llvm::APIntOps::RoundDoubleToAPInt(ଦ୍ୱିଗୁଣ Double, unsigned width) {
  uint64_t I = bit_cast<uint64_t>(Double);

  // Get the sign bit from the highest order bit
  ତାର୍କିକ isNeg = I >> 63;

  // Get the 11-bit exponent and adjust for the 1023 bit bias
  int64_t exp = ((I >> 52) & 0x7ff) - 1023;

  // If the exponent is negative, the value is < 0 so just return 0.
  ଯଦି (exp < 0)
    ଫେରନ୍ତୁ APInt(width, 0u);

  // Extract the mantissa by clearing the top 12 bits (sign + exponent).
  uint64_t mantissa = (I & (~0ULL >> 12)) | 1ULL << 52;

  // If the exponent doesn't shift all bits out of the mantissa
  ଯଦି (exp < 52)
    ଫେରନ୍ତୁ isNeg ? -APInt(width, mantissa >> (52 - exp)) :
                    APInt(width, mantissa >> (52 - exp));

  // If the client didn't provide enough bits for us to shift the mantissa into
  // then the result is undefined, just return 0
  ଯଦି (width <= exp - 52)
    ଫେରନ୍ତୁ APInt(width, 0);

  // Otherwise, we have to shift the mantissa bits up to the right location
  APInt Tmp(width, mantissa);
  Tmp <<= (unsigned)exp - 52;
  ଫେରନ୍ତୁ isNeg ? -Tmp : Tmp;
}

/// This function converts this APInt to a double.
/// The layout for double is as following (IEEE Standard 754):
///  --------------------------------------
/// |  Sign    Exponent    Fraction    Bias |
/// |-------------------------------------- |
/// |  1[63]   11[62-52]   52[51-00]   1023 |
///  --------------------------------------
ଦ୍ୱିଗୁଣ APInt::roundToDouble(ତାର୍କିକ isSigned) ଅଚଳ {
  // Handle the simple case where the value is contained in one uint64_t.
  // It is wrong to optimize getWord(0) to VAL; there might be more than one word.
  ଯଦି (LLVM_LIKELY(isSingleWord() || getActiveBits() <= APINT_BITS_PER_WORD)) {
    ଯଦି (isSigned) {
      int64_t sext = SignExtend64(getWord(0), BitWidth);
      ଫେରନ୍ତୁ ଦ୍ୱିଗୁଣ(sext);
    }
    ଫେରନ୍ତୁ ଦ୍ୱିଗୁଣ(getWord(0));
  }

  // Determine if the value is negative.
  ତାର୍କିକ isNeg = isSigned ? (*ଏହି)[BitWidth-1] : ମିଥ୍ୟା;

  // Construct the absolute value if we're negative.
  APInt Tmp(isNeg ? -(*ଏହି) : (*ଏହି));

  // Figure out how many bits we're using.
  unsigned n = Tmp.getActiveBits();

  // The exponent (without bias normalization) is just the number of bits
  // we are using. Note that the sign bit is gone since we constructed the
  // absolute value.
  uint64_t exp = n;

  // Return infinity for exponent overflow
  ଯଦି (exp > 1023) {
    ଯଦି (!isSigned || !isNeg)
      ଫେରନ୍ତୁ std::numeric_limits<ଦ୍ୱିଗୁଣ>::infinity();
    ଅନ୍ୟଥା
      ଫେରନ୍ତୁ -std::numeric_limits<ଦ୍ୱିଗୁଣ>::infinity();
  }
  exp += 1023; // Increment for 1023 bias

  // Number of bits in mantissa is 52. To obtain the mantissa value, we must
  // extract the high 52 bits from the correct words in pVal.
  uint64_t mantissa;
  unsigned hiWord = whichWord(n-1);
  ଯଦି (hiWord == 0) {
    mantissa = Tmp.U.pVal[0];
    ଯଦି (n > 52)
      mantissa >>= n - 52; // shift down, we want the top 52 bits.
  } ଅନ୍ୟଥା {
    ପ୍ରତିଜ୍ଞା(hiWord > 0 && "huh?");
    uint64_t hibits = Tmp.U.pVal[hiWord] << (52 - n % APINT_BITS_PER_WORD);
    uint64_t lobits = Tmp.U.pVal[hiWord-1] >> (11 + n % APINT_BITS_PER_WORD);
    mantissa = hibits | lobits;
  }

  // The leading bit of mantissa is implicit, so get rid of it.
  uint64_t sign = isNeg ? (1ULL << (APINT_BITS_PER_WORD - 1)) : 0;
  uint64_t I = sign | (exp << 52) | mantissa;
  ଫେରନ୍ତୁ bit_cast<ଦ୍ୱିଗୁଣ>(I);
}

// Truncate to new width.
APInt APInt::trunc(unsigned width) ଅଚଳ {
  ପ୍ରତିଜ୍ଞା(width <= BitWidth && "Invalid APInt Truncate request");

  ଯଦି (width <= APINT_BITS_PER_WORD)
    ଫେରନ୍ତୁ APInt(width, getRawData()[0], /*isSigned=*/ମିଥ୍ୟା,
                 /*implicitTrunc=*/ସତ୍ୟ);

  ଯଦି (width == BitWidth)
    ଫେରନ୍ତୁ *ଏହି;

  APInt Result(getMemory(get_ଅଙ୍କ_ଶବ୍ଦଗୁଡ଼ିକ(width)), width);

  // Copy full words.
  unsigned i;
  ପାଇଁ (i = 0; i != width / APINT_BITS_PER_WORD; i++)
    Result.U.pVal[i] = U.pVal[i];

  // Truncate and copy any partial word.
  unsigned bits = (0 - width) % APINT_BITS_PER_WORD;
  ଯଦି (bits != 0)
    Result.U.pVal[i] = U.pVal[i] << bits >> bits;

  ଫେରନ୍ତୁ Result;
}

// Truncate to new width with unsigned saturation.
APInt APInt::truncUSat(unsigned width) ଅଚଳ {
  ପ୍ରତିଜ୍ଞା(width <= BitWidth && "Invalid APInt Truncate request");

  // Can we just losslessly truncate it?
  ଯଦି (isIntN(width))
    ଫେରନ୍ତୁ trunc(width);
  // If not, then just return the new limit.
  ଫେରନ୍ତୁ APInt::get_Max_ମୂଲ୍ୟ(width);
}

// Truncate to new width with signed saturation to signed result.
APInt APInt::truncSSat(unsigned width) ଅଚଳ {
  ପ୍ରତିଜ୍ଞା(width <= BitWidth && "Invalid APInt Truncate request");

  // Can we just losslessly truncate it?
  ଯଦି (isSignedIntN(width))
    ଫେରନ୍ତୁ trunc(width);
  // If not, then just return the new limits.
  ଫେରନ୍ତୁ isNegative() ? APInt::get_Signed_Min_ମୂଲ୍ୟ(width)
                      : APInt::get_Signed_Max_ମୂଲ୍ୟ(width);
}

// Truncate to new width with signed saturation to unsigned result.
APInt APInt::truncSSatU(unsigned width) ଅଚଳ {
  ପ୍ରତିଜ୍ଞା(width <= BitWidth && "Invalid APInt Truncate request");

  // Can we just losslessly truncate it?
  ଯଦି (isIntN(width))
    ଫେରନ୍ତୁ trunc(width);
  // If not, then just return the new limits.
  ଫେରନ୍ତୁ isNegative() ? APInt::getZero(width) : APInt::get_Max_ମୂଲ୍ୟ(width);
}

// Sign extend to a new width.
APInt APInt::sext(unsigned Width) ଅଚଳ {
  ପ୍ରତିଜ୍ଞା(Width >= BitWidth && "Invalid APInt SignExtend request");

  ଯଦି (Width <= APINT_BITS_PER_WORD)
    ଫେରନ୍ତୁ APInt(Width, SignExtend64(U.VAL, BitWidth), /*isSigned=*/ସତ୍ୟ);

  ଯଦି (Width == BitWidth)
    ଫେରନ୍ତୁ *ଏହି;

  APInt Result(getMemory(get_ଅଙ୍କ_ଶବ୍ଦଗୁଡ଼ିକ(Width)), Width);

  // Copy words.
  std::memcpy(Result.U.pVal, getRawData(), get_ଅଙ୍କ_ଶବ୍ଦଗୁଡ଼ିକ() * APINT_WORD_SIZE);

  // Sign extend the last word since there may be unused bits in the input.
  Result.U.pVal[get_ଅଙ୍କ_ଶବ୍ଦଗୁଡ଼ିକ() - 1] =
      SignExtend64(Result.U.pVal[get_ଅଙ୍କ_ଶବ୍ଦଗୁଡ଼ିକ() - 1],
                   ((BitWidth - 1) % APINT_BITS_PER_WORD) + 1);

  // Fill with sign bits.
  std::memset(Result.U.pVal + get_ଅଙ୍କ_ଶବ୍ଦଗୁଡ଼ିକ(), isNegative() ? -1 : 0,
              (Result.get_ଅଙ୍କ_ଶବ୍ଦଗୁଡ଼ିକ() - get_ଅଙ୍କ_ଶବ୍ଦଗୁଡ଼ିକ()) * APINT_WORD_SIZE);
  Result.clearUnusedBits();
  ଫେରନ୍ତୁ Result;
}

//  Zero extend to a new width.
APInt APInt::zext(unsigned width) ଅଚଳ {
  ପ୍ରତିଜ୍ଞା(width >= BitWidth && "Invalid APInt ZeroExtend request");

  ଯଦି (width <= APINT_BITS_PER_WORD)
    ଫେରନ୍ତୁ APInt(width, U.VAL);

  ଯଦି (width == BitWidth)
    ଫେରନ୍ତୁ *ଏହି;

  APInt Result(getMemory(get_ଅଙ୍କ_ଶବ୍ଦଗୁଡ଼ିକ(width)), width);

  // Copy words.
  std::memcpy(Result.U.pVal, getRawData(), get_ଅଙ୍କ_ଶବ୍ଦଗୁଡ଼ିକ() * APINT_WORD_SIZE);

  // Zero remaining words.
  std::memset(Result.U.pVal + get_ଅଙ୍କ_ଶବ୍ଦଗୁଡ଼ିକ(), 0,
              (Result.get_ଅଙ୍କ_ଶବ୍ଦଗୁଡ଼ିକ() - get_ଅଙ୍କ_ଶବ୍ଦଗୁଡ଼ିକ()) * APINT_WORD_SIZE);

  ଫେରନ୍ତୁ Result;
}

APInt APInt::zextOrTrunc(unsigned width) ଅଚଳ {
  ଯଦି (BitWidth < width)
    ଫେରନ୍ତୁ zext(width);
  ଯଦି (BitWidth > width)
    ଫେରନ୍ତୁ trunc(width);
  ଫେରନ୍ତୁ *ଏହି;
}

APInt APInt::sextOrTrunc(unsigned width) ଅଚଳ {
  ଯଦି (BitWidth < width)
    ଫେରନ୍ତୁ sext(width);
  ଯଦି (BitWidth > width)
    ଫେରନ୍ତୁ trunc(width);
  ଫେରନ୍ତୁ *ଏହି;
}

/// Arithmetic right-shift this APInt by shiftAmt.
/// Arithmetic right-shift function.
ଶୂନ୍ୟ APInt::ashrInPlace(ଅଚଳ APInt &shiftAmt) {
  ashrInPlace((unsigned)shiftAmt.get_Limited_ମୂଲ୍ୟ(BitWidth));
}

/// Arithmetic right-shift this APInt by shiftAmt.
/// Arithmetic right-shift function.
ଶୂନ୍ୟ APInt::ashrSlowCase(unsigned ShiftAmt) {
  // Don't bother performing a no-op shift.
  ଯଦି (!ShiftAmt)
    ଫେରନ୍ତୁ;

  // Save the original sign bit for later.
  ତାର୍କିକ Negative = isNegative();

  // WordShift is the inter-part shift; BitShift is intra-part shift.
  unsigned WordShift = ShiftAmt / APINT_BITS_PER_WORD;
  unsigned BitShift = ShiftAmt % APINT_BITS_PER_WORD;

  unsigned ଶବ୍ଦଗୁଡ଼ିକ_To_Move = get_ଅଙ୍କ_ଶବ୍ଦଗୁଡ଼ିକ() - WordShift;
  ଯଦି (ଶବ୍ଦଗୁଡ଼ିକ_To_Move != 0) {
    // Sign extend the last word to fill in the unused bits.
    U.pVal[get_ଅଙ୍କ_ଶବ୍ଦଗୁଡ଼ିକ() - 1] = SignExtend64(
        U.pVal[get_ଅଙ୍କ_ଶବ୍ଦଗୁଡ଼ିକ() - 1], ((BitWidth - 1) % APINT_BITS_PER_WORD) + 1);

    // Fastpath for moving by whole words.
    ଯଦି (BitShift == 0) {
      std::memmove(U.pVal, U.pVal + WordShift, ଶବ୍ଦଗୁଡ଼ିକ_To_Move * APINT_WORD_SIZE);
    } ଅନ୍ୟଥା {
      // Move the words containing significant bits.
      ପାଇଁ (unsigned i = 0; i != ଶବ୍ଦଗୁଡ଼ିକ_To_Move - 1; ++i)
        U.pVal[i] = (U.pVal[i + WordShift] >> BitShift) |
                    (U.pVal[i + WordShift + 1] << (APINT_BITS_PER_WORD - BitShift));

      // Handle the last word which has no high bits to copy. Use an arithmetic
      // shift to preserve the sign bit.
      U.pVal[ଶବ୍ଦଗୁଡ଼ିକ_To_Move - 1] =
          (int64_t)U.pVal[WordShift + ଶବ୍ଦଗୁଡ଼ିକ_To_Move - 1] >> BitShift;
    }
  }

  // Fill in the remainder based on the original sign.
  std::memset(U.pVal + ଶବ୍ଦଗୁଡ଼ିକ_To_Move, Negative ? -1 : 0,
              WordShift * APINT_WORD_SIZE);
  clearUnusedBits();
}

/// Logical right-shift this APInt by shiftAmt.
/// Logical right-shift function.
ଶୂନ୍ୟ APInt::lshrInPlace(ଅଚଳ APInt &shiftAmt) {
  lshrInPlace((unsigned)shiftAmt.get_Limited_ମୂଲ୍ୟ(BitWidth));
}

/// Logical right-shift this APInt by shiftAmt.
/// Logical right-shift function.
ଶୂନ୍ୟ APInt::lshrSlowCase(unsigned ShiftAmt) {
  tc_Shift_ଡାହାଣ(U.pVal, get_ଅଙ୍କ_ଶବ୍ଦଗୁଡ଼ିକ(), ShiftAmt);
}

/// Left-shift this APInt by shiftAmt.
/// Left-shift function.
APInt &APInt::operator<<=(ଅଚଳ APInt &shiftAmt) {
  // It's undefined behavior in C to shift by BitWidth or greater.
  *ଏହି <<= (unsigned)shiftAmt.get_Limited_ମୂଲ୍ୟ(BitWidth);
  ଫେରନ୍ତୁ *ଏହି;
}

ଶୂନ୍ୟ APInt::shlSlowCase(unsigned ShiftAmt) {
  tc_Shift_ବାମ(U.pVal, get_ଅଙ୍କ_ଶବ୍ଦଗୁଡ଼ିକ(), ShiftAmt);
  clearUnusedBits();
}

// Calculate the rotate amount modulo the bit width.
ସ୍ଥିର unsigned rotateModulo(unsigned BitWidth, ଅଚଳ APInt &rotateAmt) {
  ଯଦି (LLVM_UNLIKELY(BitWidth == 0))
    ଫେରନ୍ତୁ 0;
  unsigned rotBitWidth = rotateAmt.getBitWidth();
  APInt rot = rotateAmt;
  ଯଦି (rotBitWidth < BitWidth) {
    // Extend the rotate APInt, so that the urem doesn't divide by 0.
    // e.g. APInt(1, 32) would give APInt(1, 0).
    rot = rotateAmt.zext(BitWidth);
  }
  rot = rot.urem(APInt(rot.getBitWidth(), BitWidth));
  ଫେରନ୍ତୁ rot.get_Limited_ମୂଲ୍ୟ(BitWidth);
}

APInt APInt::rotl(ଅଚଳ APInt &rotateAmt) ଅଚଳ {
  ଫେରନ୍ତୁ rotl(rotateModulo(BitWidth, rotateAmt));
}

APInt APInt::rotl(unsigned rotateAmt) ଅଚଳ {
  ଯଦି (LLVM_UNLIKELY(BitWidth == 0))
    ଫେରନ୍ତୁ *ଏହି;
  rotateAmt %= BitWidth;
  ଯଦି (rotateAmt == 0)
    ଫେରନ୍ତୁ *ଏହି;
  ଫେରନ୍ତୁ shl(rotateAmt) | lshr(BitWidth - rotateAmt);
}

APInt APInt::rotr(ଅଚଳ APInt &rotateAmt) ଅଚଳ {
  ଫେରନ୍ତୁ rotr(rotateModulo(BitWidth, rotateAmt));
}

APInt APInt::rotr(unsigned rotateAmt) ଅଚଳ {
  ଯଦି (BitWidth == 0)
    ଫେରନ୍ତୁ *ଏହି;
  rotateAmt %= BitWidth;
  ଯଦି (rotateAmt == 0)
    ଫେରନ୍ତୁ *ଏହି;
  ଫେରନ୍ତୁ lshr(rotateAmt) | shl(BitWidth - rotateAmt);
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
unsigned APInt::nearestLogBase2() ଅଚଳ {
  // Special case when we have a bitwidth of 1. If VAL is 1, then we
  // get 0. If VAL is 0, we get WORDTYPE_MAX which gets truncated to
  // UINT32_MAX.
  ଯଦି (BitWidth == 1)
    ଫେରନ୍ତୁ U.VAL - 1;

  // Handle the zero case.
  ଯଦି (isZero())
    ଫେରନ୍ତୁ UINT32_MAX;

  // The non-zero case is handled by computing:
  //
  //   nearestLogBase2(x) = logBase2(x) + x[logBase2(x)-1].
  //
  // where x[i] is referring to the value of the ith bit of x.
  unsigned lg = logBase2();
  ଫେରନ୍ତୁ lg + unsigned((*ଏହି)[lg - 1]);
}

// Square Root - this method computes and returns the square root of "this".
// Three mechanisms are used for computation. For small values (<= 5 bits),
// a table lookup is done. This gets some performance for common cases. For
// values using less than 52 bits, the value is converted to double and then
// the libc sqrt function is called. The result is rounded and then converted
// back to a uint64_t which is then used to construct the result. Finally,
// the Babylonian method for computing square roots is used.
APInt APInt::sqrtFloor() ଅଚଳ {

  // Determine the magnitude of the value.
  unsigned magnitude = getActiveBits();

  // Use a fast table for some small values. This also gets rid of some
  // rounding errors in libc sqrt for small values.
  ଯଦି (magnitude <= 5) {
    ସ୍ଥିର ଅଚଳ uint8_t results[32] = {
        /*     0 */ 0,
        /*  1- 3 */ 1, 1, 1,
        /*  4- 8 */ 2, 2, 2, 2, 2,
        /*  9-15 */ 3, 3, 3, 3, 3, 3, 3,
        /* 16-24 */ 4, 4, 4, 4, 4, 4, 4, 4, 4,
        /* 25-31 */ 5, 5, 5, 5, 5, 5, 5,
    };
    ଫେରନ୍ତୁ APInt(BitWidth,
                 results[(LLVM_LIKELY(isSingleWord()) ? U.VAL : U.pVal[0])]);
  }

  // If the magnitude of the value fits in less than 52 bits (the precision of
  // an IEEE double precision floating point value), then we can use the
  // libc sqrt function which will probably use a hardware sqrt computation.
  // This should be faster than the algorithm below.
  ଯଦି (magnitude < 52) {
    ଫେରନ୍ତୁ APInt(BitWidth,
                 uint64_t(::floor(::sqrt(ଦ୍ୱିଗୁଣ(
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
  ପାଇଁ (;; i += 2, testy = testy.shl(2))
    ଯଦି (i >= nbits || ଏହି->ule(testy)) {
      x_old = x_old.shl(i / 2);
      ଭାଙ୍ଗ;
    }

  // Use the Babylonian method to arrive at the integer square root:
  ପାଇଁ (;;) {
    x_new = (ଏହି->udiv(x_old) + x_old).udiv(two);
    ଯଦି (x_old.ule(x_new))
      ଭାଙ୍ଗ;
    x_old = x_new;
  }
  ଫେରନ୍ତୁ x_old;
}

/// \returns the multiplicative inverse of an odd APInt modulo 2^BitWidth.
APInt APInt::multiplicativeInverse() ଅଚଳ {
  ପ୍ରତିଜ୍ଞା((*ଏହି)[0] &&
         "multiplicative inverse is only defined for odd numbers!");

  // Use Newton's method.
  APInt Factor = *ଏହି;
  APInt T;
  ଯେତେବେଳେ (!(T = *ଏହି * Factor).isOne())
    Factor *= 2 - std::move(T);
  ଫେରନ୍ତୁ Factor;
}

/// Implementation of Knuth's Algorithm D (Division of nonnegative integers)
/// from "Art of Computer Programming, Volume 2", section 4.3.1, p. 272. The
/// variables here have the same names as in the algorithm. Comments explain
/// the algorithm and any deviation from it.
ସ୍ଥିର ଶୂନ୍ୟ KnuthDiv(uint32_t *u, uint32_t *v, uint32_t *q, uint32_t* r,
                     unsigned m, unsigned n) {
  ପ୍ରତିଜ୍ଞା(u && "Must provide dividend");
  ପ୍ରତିଜ୍ଞା(v && "Must provide divisor");
  ପ୍ରତିଜ୍ଞା(q && "Must provide quotient");
  ପ୍ରତିଜ୍ଞା(u != v && u != q && v != q && "Must use different memory");
  ପ୍ରତିଜ୍ଞା(n>1 && "n must be > 1");

  // b denotes the base of the number system. In our case b is 2^32.
  ଅଚଳ uint64_t b = uint64_t(1) << 32;

// The DEBUG macros here tend to be spam in the debug output if you're not
// debugging this code. Disable them unless KNUTH_DEBUG is defined.
#ifdef KNUTH_DEBUG
#define DEBUG_KNUTH(X) LLVM_DEBUG(X)
#ଅନ୍ୟଥା
#define DEBUG_KNUTH(X) କର {} ଯେତେବେଳେ(ମିଥ୍ୟା)
#endif

  DEBUG_KNUTH(dbgs() << "KnuthDiv: m=" << m << " n=" << n << '\n');
  DEBUG_KNUTH(dbgs() << "KnuthDiv: original:");
  DEBUG_KNUTH(ପାଇଁ (ପୂର୍ଣ୍ଣସଂଖ୍ୟା i = m + n; i >= 0; i--) dbgs() << " " << u[i]);
  DEBUG_KNUTH(dbgs() << " by");
  DEBUG_KNUTH(ପାଇଁ (ପୂର୍ଣ୍ଣସଂଖ୍ୟା i = n; i > 0; i--) dbgs() << " " << v[i - 1]);
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
  ଯଦି (shift) {
    ପାଇଁ (unsigned i = 0; i < m+n; ++i) {
      uint32_t u_tmp = u[i] >> (32 - shift);
      u[i] = (u[i] << shift) | u_carry;
      u_carry = u_tmp;
    }
    ପାଇଁ (unsigned i = 0; i < n; ++i) {
      uint32_t v_tmp = v[i] >> (32 - shift);
      v[i] = (v[i] << shift) | v_carry;
      v_carry = v_tmp;
    }
  }
  u[m+n] = u_carry;

  DEBUG_KNUTH(dbgs() << "KnuthDiv:   normal:");
  DEBUG_KNUTH(ପାଇଁ (ପୂର୍ଣ୍ଣସଂଖ୍ୟା i = m + n; i >= 0; i--) dbgs() << " " << u[i]);
  DEBUG_KNUTH(dbgs() << " by");
  DEBUG_KNUTH(ପାଇଁ (ପୂର୍ଣ୍ଣସଂଖ୍ୟା i = n; i > 0; i--) dbgs() << " " << v[i - 1]);
  DEBUG_KNUTH(dbgs() << '\n');

  // D2. [Initialize j.]  Set j to m. This is the loop counter over the places.
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା j = m;
  କର {
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
    ଯଦି (qp == b || qp*v[n-2] > b*rp + u[j+n-2]) {
      qp--;
      rp += v[n-1];
      ଯଦି (rp < b && (qp == b || qp*v[n-2] > b*rp + u[j+n-2]))
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
    ପାଇଁ (unsigned i = 0; i < n; ++i) {
      uint64_t p = qp * uint64_t(v[i]);
      int64_t subres = int64_t(u[j+i]) - borrow - Lo_32(p);
      u[j+i] = Lo_32(subres);
      borrow = Hi_32(p) - Hi_32(subres);
      DEBUG_KNUTH(dbgs() << "KnuthDiv: u[j+i] = " << u[j + i]
                        << ", borrow = " << borrow << '\n');
    }
    ତାର୍କିକ isNeg = u[j+n] < borrow;
    u[j+n] -= Lo_32(borrow);

    DEBUG_KNUTH(dbgs() << "KnuthDiv: after subtraction:");
    DEBUG_KNUTH(ପାଇଁ (ପୂର୍ଣ୍ଣସଂଖ୍ୟା i = m + n; i >= 0; i--) dbgs() << " " << u[i]);
    DEBUG_KNUTH(dbgs() << '\n');

    // D5. [Test remainder.] Set q[j] = qp. If the result of step D4 was
    // negative, go to step D6; otherwise go on to step D7.
    q[j] = Lo_32(qp);
    ଯଦି (isNeg) {
      // D6. [Add back]. The probability that this step is necessary is very
      // small, on the order of only 2/b. Make sure that test data accounts for
      // this possibility. Decrease q[j] by 1
      q[j]--;
      // and add (0v[n-1]...v[1]v[0]) to (u[j+n]u[j+n-1]...u[j+1]u[j]).
      // A carry will occur to the left of u[j+n], and it should be ignored
      // since it cancels with the borrow that occurred in D4.
      ତାର୍କିକ carry = ମିଥ୍ୟା;
      ପାଇଁ (unsigned i = 0; i < n; i++) {
        uint32_t limit = std::min(u[j+i],v[i]);
        u[j+i] += v[i] + carry;
        carry = u[j+i] < limit || (carry && u[j+i] == limit);
      }
      u[j+n] += carry;
    }
    DEBUG_KNUTH(dbgs() << "KnuthDiv: after correction:");
    DEBUG_KNUTH(ପାଇଁ (ପୂର୍ଣ୍ଣସଂଖ୍ୟା i = m + n; i >= 0; i--) dbgs() << " " << u[i]);
    DEBUG_KNUTH(dbgs() << "\nKnuthDiv: digit result = " << q[j] << '\n');

    // D7. [Loop on j.]  Decrease j by one. Now if j >= 0, go back to D3.
  } ଯେତେବେଳେ (--j >= 0);

  DEBUG_KNUTH(dbgs() << "KnuthDiv: quotient:");
  DEBUG_KNUTH(ପାଇଁ (ପୂର୍ଣ୍ଣସଂଖ୍ୟା i = m; i >= 0; i--) dbgs() << " " << q[i]);
  DEBUG_KNUTH(dbgs() << '\n');

  // D8. [Unnormalize]. Now q[...] is the desired quotient, and the desired
  // remainder may be obtained by dividing u[...] by d. If r is non-null we
  // compute the remainder (urem uses this).
  ଯଦି (r) {
    // The value d is expressed by the "shift" value above since we avoided
    // multiplication by d by using a shift left. So, all we have to do is
    // shift right here.
    ଯଦି (shift) {
      uint32_t carry = 0;
      DEBUG_KNUTH(dbgs() << "KnuthDiv: remainder:");
      ପାଇଁ (ପୂର୍ଣ୍ଣସଂଖ୍ୟା i = n-1; i >= 0; i--) {
        r[i] = (u[i] >> shift) | carry;
        carry = u[i] << (32 - shift);
        DEBUG_KNUTH(dbgs() << " " << r[i]);
      }
    } ଅନ୍ୟଥା {
      ପାଇଁ (ପୂର୍ଣ୍ଣସଂଖ୍ୟା i = n-1; i >= 0; i--) {
        r[i] = u[i];
        DEBUG_KNUTH(dbgs() << " " << r[i]);
      }
    }
    DEBUG_KNUTH(dbgs() << '\n');
  }
  DEBUG_KNUTH(dbgs() << '\n');
}

ଶୂନ୍ୟ APInt::divide(ଅଚଳ WordType *LHS, unsigned lhs_ଶବ୍ଦଗୁଡ଼ିକ, ଅଚଳ WordType *RHS,
                   unsigned rhs_ଶବ୍ଦଗୁଡ଼ିକ, WordType *Quotient, WordType *Remainder) {
  ପ୍ରତିଜ୍ଞା(lhs_ଶବ୍ଦଗୁଡ଼ିକ >= rhs_ଶବ୍ଦଗୁଡ଼ିକ && "Fractional result");

  // First, compose the values into an array of 32-bit words instead of
  // 64-bit words. This is a necessity of both the "short division" algorithm
  // and the Knuth "classical algorithm" which requires there to be native
  // operations for +, -, and * on an m bit value with an m*2 bit result. We
  // can't use 64-bit operands here because we don't have native results of
  // 128-bits. Furthermore, casting the 64-bit values to 32-bit values won't
  // work on large-endian machines.
  unsigned n = rhs_ଶବ୍ଦଗୁଡ଼ିକ * 2;
  unsigned m = (lhs_ଶବ୍ଦଗୁଡ଼ିକ * 2) - n;

  // Allocate space for the temporary values we need either on the stack, if
  // it will fit, or on the heap if it won't.
  uint32_t SPACE[128];
  uint32_t *U = nullptr;
  uint32_t *V = nullptr;
  uint32_t *Q = nullptr;
  uint32_t *R = nullptr;
  ଯଦି ((Remainder?4:3)*n+2*m+1 <= 128) {
    U = &SPACE[0];
    V = &SPACE[m+n+1];
    Q = &SPACE[(m+n+1) + n];
    ଯଦି (Remainder)
      R = &SPACE[(m+n+1) + n + (m+n)];
  } ଅନ୍ୟଥା {
    U = ନୂତନ uint32_t[m + n + 1];
    V = ନୂତନ uint32_t[n];
    Q = ନୂତନ uint32_t[m+n];
    ଯଦି (Remainder)
      R = ନୂତନ uint32_t[n];
  }

  // Initialize the dividend
  memset(U, 0, (m+n+1)*ଆକାର(uint32_t));
  ପାଇଁ (unsigned i = 0; i < lhs_ଶବ୍ଦଗୁଡ଼ିକ; ++i) {
    uint64_t tmp = LHS[i];
    U[i * 2] = Lo_32(tmp);
    U[i * 2 + 1] = Hi_32(tmp);
  }
  U[m+n] = 0; // this extra word is for "spill" in the Knuth algorithm.

  // Initialize the divisor
  memset(V, 0, (n)*ଆକାର(uint32_t));
  ପାଇଁ (unsigned i = 0; i < rhs_ଶବ୍ଦଗୁଡ଼ିକ; ++i) {
    uint64_t tmp = RHS[i];
    V[i * 2] = Lo_32(tmp);
    V[i * 2 + 1] = Hi_32(tmp);
  }

  // initialize the quotient and remainder
  memset(Q, 0, (m+n) * ଆକାର(uint32_t));
  ଯଦି (Remainder)
    memset(R, 0, n * ଆକାର(uint32_t));

  // Now, adjust m and n for the Knuth division. n is the number of words in
  // the divisor. m is the number of words by which the dividend exceeds the
  // divisor (i.e. m+n is the length of the dividend). These sizes must not
  // contain any zero words or the Knuth algorithm fails.
  ପାଇଁ (unsigned i = n; i > 0 && V[i-1] == 0; i--) {
    n--;
    m++;
  }
  ପାଇଁ (unsigned i = m+n; i > 0 && U[i-1] == 0; i--)
    m--;

  // If we're left with only a single word for the divisor, Knuth doesn't work
  // so we implement the short division algorithm here. This is much simpler
  // and faster because we are certain that we can divide a 64-bit quantity
  // by a 32-bit quantity at hardware speed and short division is simply a
  // series of such operations. This is just like doing short division but we
  // are using base 2^32 instead of base 10.
  ପ୍ରତିଜ୍ଞା(n != 0 && "Divide by zero?");
  ଯଦି (n == 1) {
    uint32_t ଭାଜକ = V[0];
    uint32_t remainder = 0;
    ପାଇଁ (ପୂର୍ଣ୍ଣସଂଖ୍ୟା i = m; i >= 0; i--) {
      uint64_t partial_dividend = Make_64(remainder, U[i]);
      ଯଦି (partial_dividend == 0) {
        Q[i] = 0;
        remainder = 0;
      } ଅନ୍ୟଥା ଯଦି (partial_dividend < ଭାଜକ) {
        Q[i] = 0;
        remainder = Lo_32(partial_dividend);
      } ଅନ୍ୟଥା ଯଦି (partial_dividend == ଭାଜକ) {
        Q[i] = 1;
        remainder = 0;
      } ଅନ୍ୟଥା {
        Q[i] = Lo_32(partial_dividend / ଭାଜକ);
        remainder = Lo_32(partial_dividend - (Q[i] * ଭାଜକ));
      }
    }
    ଯଦି (R)
      R[0] = remainder;
  } ଅନ୍ୟଥା {
    // Now we're ready to invoke the Knuth classical divide algorithm. In this
    // case n > 1.
    KnuthDiv(U, V, Q, R, m, n);
  }

  // If the caller wants the quotient
  ଯଦି (Quotient) {
    ପାଇଁ (unsigned i = 0; i < lhs_ଶବ୍ଦଗୁଡ଼ିକ; ++i)
      Quotient[i] = Make_64(Q[i*2+1], Q[i*2]);
  }

  // If the caller wants the remainder
  ଯଦି (Remainder) {
    ପାଇଁ (unsigned i = 0; i < rhs_ଶବ୍ଦଗୁଡ଼ିକ; ++i)
      Remainder[i] = Make_64(R[i*2+1], R[i*2]);
  }

  // Clean up the memory we allocated.
  ଯଦି (U != &SPACE[0]) {
    delete [] U;
    delete [] V;
    delete [] Q;
    delete [] R;
  }
}

APInt APInt::udiv(ଅଚଳ APInt &RHS) ଅଚଳ {
  ପ୍ରତିଜ୍ଞା(BitWidth == RHS.BitWidth && "Bit widths must be the same");

  // First, deal with the easy case
  ଯଦି (LLVM_LIKELY(isSingleWord())) {
    ପ୍ରତିଜ୍ଞା(RHS.U.VAL != 0 && "Divide by zero?");
    ଫେରନ୍ତୁ APInt(BitWidth, U.VAL / RHS.U.VAL);
  }

  // Get some facts about the LHS and RHS number of bits and words
  unsigned lhs_ଶବ୍ଦଗୁଡ଼ିକ = get_ଅଙ୍କ_ଶବ୍ଦଗୁଡ଼ିକ(getActiveBits());
  unsigned rhsBits  = RHS.getActiveBits();
  unsigned rhs_ଶବ୍ଦଗୁଡ଼ିକ = get_ଅଙ୍କ_ଶବ୍ଦଗୁଡ଼ିକ(rhsBits);
  ପ୍ରତିଜ୍ଞା(rhs_ଶବ୍ଦଗୁଡ଼ିକ && "Divided by zero???");

  // Deal with some degenerate cases
  ଯଦି (!lhs_ଶବ୍ଦଗୁଡ଼ିକ)
    // 0 / X ===> 0
    ଫେରନ୍ତୁ APInt(BitWidth, 0);
  ଯଦି (rhsBits == 1)
    // X / 1 ===> X
    ଫେରନ୍ତୁ *ଏହି;
  ଯଦି (lhs_ଶବ୍ଦଗୁଡ଼ିକ < rhs_ଶବ୍ଦଗୁଡ଼ିକ || ଏହି->ult(RHS))
    // X / Y ===> 0, iff X < Y
    ଫେରନ୍ତୁ APInt(BitWidth, 0);
  ଯଦି (*ଏହି == RHS)
    // X / X ===> 1
    ଫେରନ୍ତୁ APInt(BitWidth, 1);
  ଯଦି (lhs_ଶବ୍ଦଗୁଡ଼ିକ == 1) // rhsWords is 1 if lhsWords is 1.
    // All high words are zero, just use native divide
    ଫେରନ୍ତୁ APInt(BitWidth, ଏହି->U.pVal[0] / RHS.U.pVal[0]);

  // We have to compute it the hard way. Invoke the Knuth divide algorithm.
  APInt Quotient(BitWidth, 0); // to hold result.
  divide(U.pVal, lhs_ଶବ୍ଦଗୁଡ଼ିକ, RHS.U.pVal, rhs_ଶବ୍ଦଗୁଡ଼ିକ, Quotient.U.pVal, nullptr);
  ଫେରନ୍ତୁ Quotient;
}

APInt APInt::udiv(uint64_t RHS) ଅଚଳ {
  ପ୍ରତିଜ୍ଞା(RHS != 0 && "Divide by zero?");

  // First, deal with the easy case
  ଯଦି (LLVM_LIKELY(isSingleWord()))
    ଫେରନ୍ତୁ APInt(BitWidth, U.VAL / RHS);

  // Get some facts about the LHS words.
  unsigned lhs_ଶବ୍ଦଗୁଡ଼ିକ = get_ଅଙ୍କ_ଶବ୍ଦଗୁଡ଼ିକ(getActiveBits());

  // Deal with some degenerate cases
  ଯଦି (!lhs_ଶବ୍ଦଗୁଡ଼ିକ)
    // 0 / X ===> 0
    ଫେରନ୍ତୁ APInt(BitWidth, 0);
  ଯଦି (RHS == 1)
    // X / 1 ===> X
    ଫେରନ୍ତୁ *ଏହି;
  ଯଦି (ଏହି->ult(RHS))
    // X / Y ===> 0, iff X < Y
    ଫେରନ୍ତୁ APInt(BitWidth, 0);
  ଯଦି (*ଏହି == RHS)
    // X / X ===> 1
    ଫେରନ୍ତୁ APInt(BitWidth, 1);
  ଯଦି (lhs_ଶବ୍ଦଗୁଡ଼ିକ == 1) // rhsWords is 1 if lhsWords is 1.
    // All high words are zero, just use native divide
    ଫେରନ୍ତୁ APInt(BitWidth, ଏହି->U.pVal[0] / RHS);

  // We have to compute it the hard way. Invoke the Knuth divide algorithm.
  APInt Quotient(BitWidth, 0); // to hold result.
  divide(U.pVal, lhs_ଶବ୍ଦଗୁଡ଼ିକ, &RHS, 1, Quotient.U.pVal, nullptr);
  ଫେରନ୍ତୁ Quotient;
}

APInt APInt::sdiv(ଅଚଳ APInt &RHS) ଅଚଳ {
  ଯଦି (isNegative()) {
    ଯଦି (RHS.isNegative())
      ଫେରନ୍ତୁ (-(*ଏହି)).udiv(-RHS);
    ଫେରନ୍ତୁ -((-(*ଏହି)).udiv(RHS));
  }
  ଯଦି (RHS.isNegative())
    ଫେରନ୍ତୁ -(ଏହି->udiv(-RHS));
  ଫେରନ୍ତୁ ଏହି->udiv(RHS);
}

APInt APInt::sdiv(int64_t RHS) ଅଚଳ {
  ଯଦି (isNegative()) {
    ଯଦି (RHS < 0)
      ଫେରନ୍ତୁ (-(*ଏହି)).udiv(-RHS);
    ଫେରନ୍ତୁ -((-(*ଏହି)).udiv(RHS));
  }
  ଯଦି (RHS < 0)
    ଫେରନ୍ତୁ -(ଏହି->udiv(-RHS));
  ଫେରନ୍ତୁ ଏହି->udiv(RHS);
}

APInt APInt::urem(ଅଚଳ APInt &RHS) ଅଚଳ {
  ପ୍ରତିଜ୍ଞା(BitWidth == RHS.BitWidth && "Bit widths must be the same");
  ଯଦି (LLVM_LIKELY(isSingleWord())) {
    ପ୍ରତିଜ୍ଞା(RHS.U.VAL != 0 && "Remainder by zero?");
    ଫେରନ୍ତୁ APInt(BitWidth, U.VAL % RHS.U.VAL);
  }

  // Get some facts about the LHS
  unsigned lhs_ଶବ୍ଦଗୁଡ଼ିକ = get_ଅଙ୍କ_ଶବ୍ଦଗୁଡ଼ିକ(getActiveBits());

  // Get some facts about the RHS
  unsigned rhsBits = RHS.getActiveBits();
  unsigned rhs_ଶବ୍ଦଗୁଡ଼ିକ = get_ଅଙ୍କ_ଶବ୍ଦଗୁଡ଼ିକ(rhsBits);
  ପ୍ରତିଜ୍ଞା(rhs_ଶବ୍ଦଗୁଡ଼ିକ && "Performing remainder operation by zero ???");

  // Check the degenerate cases
  ଯଦି (lhs_ଶବ୍ଦଗୁଡ଼ିକ == 0)
    // 0 % Y ===> 0
    ଫେରନ୍ତୁ APInt(BitWidth, 0);
  ଯଦି (rhsBits == 1)
    // X % 1 ===> 0
    ଫେରନ୍ତୁ APInt(BitWidth, 0);
  ଯଦି (lhs_ଶବ୍ଦଗୁଡ଼ିକ < rhs_ଶବ୍ଦଗୁଡ଼ିକ || ଏହି->ult(RHS))
    // X % Y ===> X, iff X < Y
    ଫେରନ୍ତୁ *ଏହି;
  ଯଦି (*ଏହି == RHS)
    // X % X == 0;
    ଫେରନ୍ତୁ APInt(BitWidth, 0);
  ଯଦି (lhs_ଶବ୍ଦଗୁଡ଼ିକ == 1)
    // All high words are zero, just use native remainder
    ଫେରନ୍ତୁ APInt(BitWidth, U.pVal[0] % RHS.U.pVal[0]);
  ଯଦି (RHS.isPowerOf2()) {
    // X % 2^w ===> X & (2^w - 1)
    APInt Result(*ଏହି);
    Result.clearBits(RHS.logBase2(), BitWidth);
    ଫେରନ୍ତୁ Result;
  }

  // We have to compute it the hard way. Invoke the Knuth divide algorithm.
  APInt Remainder(BitWidth, 0);
  divide(U.pVal, lhs_ଶବ୍ଦଗୁଡ଼ିକ, RHS.U.pVal, rhs_ଶବ୍ଦଗୁଡ଼ିକ, nullptr, Remainder.U.pVal);
  ଫେରନ୍ତୁ Remainder;
}

uint64_t APInt::urem(uint64_t RHS) ଅଚଳ {
  ପ୍ରତିଜ୍ଞା(RHS != 0 && "Remainder by zero?");

  ଯଦି (LLVM_LIKELY(isSingleWord()))
    ଫେରନ୍ତୁ U.VAL % RHS;

  // Get some facts about the LHS
  unsigned lhs_ଶବ୍ଦଗୁଡ଼ିକ = get_ଅଙ୍କ_ଶବ୍ଦଗୁଡ଼ିକ(getActiveBits());

  // Check the degenerate cases
  ଯଦି (lhs_ଶବ୍ଦଗୁଡ଼ିକ == 0)
    // 0 % Y ===> 0
    ଫେରନ୍ତୁ 0;
  ଯଦି (RHS == 1)
    // X % 1 ===> 0
    ଫେରନ୍ତୁ 0;
  ଯଦି (ଏହି->ult(RHS))
    // X % Y ===> X, iff X < Y
    ଫେରନ୍ତୁ get_ZExt_ମୂଲ୍ୟ();
  ଯଦି (*ଏହି == RHS)
    // X % X == 0;
    ଫେରନ୍ତୁ 0;
  ଯଦି (lhs_ଶବ୍ଦଗୁଡ଼ିକ == 1)
    // All high words are zero, just use native remainder
    ଫେରନ୍ତୁ U.pVal[0] % RHS;
  ଯଦି (llvm::isPowerOf2_64(RHS))
    // X % 2^w ===> X & (2^w - 1)
    ଫେରନ୍ତୁ U.pVal[0] & (RHS - 1);

  // We have to compute it the hard way. Invoke the Knuth divide algorithm.
  uint64_t Remainder;
  divide(U.pVal, lhs_ଶବ୍ଦଗୁଡ଼ିକ, &RHS, 1, nullptr, &Remainder);
  ଫେରନ୍ତୁ Remainder;
}

APInt APInt::srem(ଅଚଳ APInt &RHS) ଅଚଳ {
  ଯଦି (isNegative()) {
    ଯଦି (RHS.isNegative())
      ଫେରନ୍ତୁ -((-(*ଏହି)).urem(-RHS));
    ଫେରନ୍ତୁ -((-(*ଏହି)).urem(RHS));
  }
  ଯଦି (RHS.isNegative())
    ଫେରନ୍ତୁ ଏହି->urem(-RHS);
  ଫେରନ୍ତୁ ଏହି->urem(RHS);
}

int64_t APInt::srem(int64_t RHS) ଅଚଳ {
  ଯଦି (isNegative()) {
    ଯଦି (RHS < 0)
      ଫେରନ୍ତୁ -((-(*ଏହି)).urem(-RHS));
    ଫେରନ୍ତୁ -((-(*ଏହି)).urem(RHS));
  }
  ଯଦି (RHS < 0)
    ଫେରନ୍ତୁ ଏହି->urem(-RHS);
  ଫେରନ୍ତୁ ଏହି->urem(RHS);
}

ଶୂନ୍ୟ APInt::udivrem(ଅଚଳ APInt &LHS, ଅଚଳ APInt &RHS,
                    APInt &Quotient, APInt &Remainder) {
  ପ୍ରତିଜ୍ଞା(LHS.BitWidth == RHS.BitWidth && "Bit widths must be the same");
  unsigned BitWidth = LHS.BitWidth;

  // First, deal with the easy case
  ଯଦି (LLVM_LIKELY(LHS.isSingleWord())) {
    ପ୍ରତିଜ୍ଞା(RHS.U.VAL != 0 && "Divide by zero?");
    uint64_t QuotVal = LHS.U.VAL / RHS.U.VAL;
    uint64_t RemVal = LHS.U.VAL % RHS.U.VAL;
    Quotient = APInt(BitWidth, QuotVal);
    Remainder = APInt(BitWidth, RemVal);
    ଫେରନ୍ତୁ;
  }

  // Get some size facts about the dividend and divisor
  unsigned lhs_ଶବ୍ଦଗୁଡ଼ିକ = get_ଅଙ୍କ_ଶବ୍ଦଗୁଡ଼ିକ(LHS.getActiveBits());
  unsigned rhsBits  = RHS.getActiveBits();
  unsigned rhs_ଶବ୍ଦଗୁଡ଼ିକ = get_ଅଙ୍କ_ଶବ୍ଦଗୁଡ଼ିକ(rhsBits);
  ପ୍ରତିଜ୍ଞା(rhs_ଶବ୍ଦଗୁଡ଼ିକ && "Performing divrem operation by zero ???");

  // Check the degenerate cases
  ଯଦି (lhs_ଶବ୍ଦଗୁଡ଼ିକ == 0) {
    Quotient = APInt(BitWidth, 0);    // 0 / Y ===> 0
    Remainder = APInt(BitWidth, 0);   // 0 % Y ===> 0
    ଫେରନ୍ତୁ;
  }

  ଯଦି (rhsBits == 1) {
    Quotient = LHS;                   // X / 1 ===> X
    Remainder = APInt(BitWidth, 0);   // X % 1 ===> 0
  }

  ଯଦି (lhs_ଶବ୍ଦଗୁଡ଼ିକ < rhs_ଶବ୍ଦଗୁଡ଼ିକ || LHS.ult(RHS)) {
    Remainder = LHS;                  // X % Y ===> X, iff X < Y
    Quotient = APInt(BitWidth, 0);    // X / Y ===> 0, iff X < Y
    ଫେରନ୍ତୁ;
  }

  ଯଦି (LHS == RHS) {
    Quotient  = APInt(BitWidth, 1);   // X / X ===> 1
    Remainder = APInt(BitWidth, 0);   // X % X ===> 0;
    ଫେରନ୍ତୁ;
  }

  // Make sure there is enough space to hold the results.
  // NOTE: This assumes that reallocate won't affect any bits if it doesn't
  // change the size. This is necessary if Quotient or Remainder is aliased
  // with LHS or RHS.
  Quotient.reallocate(BitWidth);
  Remainder.reallocate(BitWidth);

  ଯଦି (lhs_ଶବ୍ଦଗୁଡ଼ିକ == 1) { // rhsWords is 1 if lhsWords is 1.
    // There is only one word to consider so use the native versions.
    uint64_t lhs_ମୂଲ୍ୟ = LHS.U.pVal[0];
    uint64_t rhs_ମୂଲ୍ୟ = RHS.U.pVal[0];
    Quotient = lhs_ମୂଲ୍ୟ / rhs_ମୂଲ୍ୟ;
    Remainder = lhs_ମୂଲ୍ୟ % rhs_ମୂଲ୍ୟ;
    ଫେରନ୍ତୁ;
  }

  // Okay, lets do it the long way
  divide(LHS.U.pVal, lhs_ଶବ୍ଦଗୁଡ଼ିକ, RHS.U.pVal, rhs_ଶବ୍ଦଗୁଡ଼ିକ, Quotient.U.pVal,
         Remainder.U.pVal);
  // Clear the rest of the Quotient and Remainder.
  std::memset(Quotient.U.pVal + lhs_ଶବ୍ଦଗୁଡ଼ିକ, 0,
              (get_ଅଙ୍କ_ଶବ୍ଦଗୁଡ଼ିକ(BitWidth) - lhs_ଶବ୍ଦଗୁଡ଼ିକ) * APINT_WORD_SIZE);
  std::memset(Remainder.U.pVal + rhs_ଶବ୍ଦଗୁଡ଼ିକ, 0,
              (get_ଅଙ୍କ_ଶବ୍ଦଗୁଡ଼ିକ(BitWidth) - rhs_ଶବ୍ଦଗୁଡ଼ିକ) * APINT_WORD_SIZE);
}

ଶୂନ୍ୟ APInt::udivrem(ଅଚଳ APInt &LHS, uint64_t RHS, APInt &Quotient,
                    uint64_t &Remainder) {
  ପ୍ରତିଜ୍ଞା(RHS != 0 && "Divide by zero?");
  unsigned BitWidth = LHS.BitWidth;

  // First, deal with the easy case
  ଯଦି (LLVM_LIKELY(LHS.isSingleWord())) {
    uint64_t QuotVal = LHS.U.VAL / RHS;
    Remainder = LHS.U.VAL % RHS;
    Quotient = APInt(BitWidth, QuotVal);
    ଫେରନ୍ତୁ;
  }

  // Get some size facts about the dividend and divisor
  unsigned lhs_ଶବ୍ଦଗୁଡ଼ିକ = get_ଅଙ୍କ_ଶବ୍ଦଗୁଡ଼ିକ(LHS.getActiveBits());

  // Check the degenerate cases
  ଯଦି (lhs_ଶବ୍ଦଗୁଡ଼ିକ == 0) {
    Quotient = APInt(BitWidth, 0);    // 0 / Y ===> 0
    Remainder = 0;                    // 0 % Y ===> 0
    ଫେରନ୍ତୁ;
  }

  ଯଦି (RHS == 1) {
    Quotient = LHS;                   // X / 1 ===> X
    Remainder = 0;                    // X % 1 ===> 0
    ଫେରନ୍ତୁ;
  }

  ଯଦି (LHS.ult(RHS)) {
    Remainder = LHS.get_ZExt_ମୂଲ୍ୟ();   // X % Y ===> X, iff X < Y
    Quotient = APInt(BitWidth, 0);    // X / Y ===> 0, iff X < Y
    ଫେରନ୍ତୁ;
  }

  ଯଦି (LHS == RHS) {
    Quotient  = APInt(BitWidth, 1);   // X / X ===> 1
    Remainder = 0;                    // X % X ===> 0;
    ଫେରନ୍ତୁ;
  }

  // Make sure there is enough space to hold the results.
  // NOTE: This assumes that reallocate won't affect any bits if it doesn't
  // change the size. This is necessary if Quotient is aliased with LHS.
  Quotient.reallocate(BitWidth);

  ଯଦି (lhs_ଶବ୍ଦଗୁଡ଼ିକ == 1) { // rhsWords is 1 if lhsWords is 1.
    // There is only one word to consider so use the native versions.
    uint64_t lhs_ମୂଲ୍ୟ = LHS.U.pVal[0];
    Quotient = lhs_ମୂଲ୍ୟ / RHS;
    Remainder = lhs_ମୂଲ୍ୟ % RHS;
    ଫେରନ୍ତୁ;
  }

  // Okay, lets do it the long way
  divide(LHS.U.pVal, lhs_ଶବ୍ଦଗୁଡ଼ିକ, &RHS, 1, Quotient.U.pVal, &Remainder);
  // Clear the rest of the Quotient.
  std::memset(Quotient.U.pVal + lhs_ଶବ୍ଦଗୁଡ଼ିକ, 0,
              (get_ଅଙ୍କ_ଶବ୍ଦଗୁଡ଼ିକ(BitWidth) - lhs_ଶବ୍ଦଗୁଡ଼ିକ) * APINT_WORD_SIZE);
}

ଶୂନ୍ୟ APInt::sdivrem(ଅଚଳ APInt &LHS, ଅଚଳ APInt &RHS,
                    APInt &Quotient, APInt &Remainder) {
  ଯଦି (LHS.isNegative()) {
    ଯଦି (RHS.isNegative())
      APInt::udivrem(-LHS, -RHS, Quotient, Remainder);
    ଅନ୍ୟଥା {
      APInt::udivrem(-LHS, RHS, Quotient, Remainder);
      Quotient.negate();
    }
    Remainder.negate();
  } ଅନ୍ୟଥା ଯଦି (RHS.isNegative()) {
    APInt::udivrem(LHS, -RHS, Quotient, Remainder);
    Quotient.negate();
  } ଅନ୍ୟଥା {
    APInt::udivrem(LHS, RHS, Quotient, Remainder);
  }
}

ଶୂନ୍ୟ APInt::sdivrem(ଅଚଳ APInt &LHS, int64_t RHS,
                    APInt &Quotient, int64_t &Remainder) {
  uint64_t R = Remainder;
  ଯଦି (LHS.isNegative()) {
    ଯଦି (RHS < 0)
      APInt::udivrem(-LHS, -RHS, Quotient, R);
    ଅନ୍ୟଥା {
      APInt::udivrem(-LHS, RHS, Quotient, R);
      Quotient.negate();
    }
    R = -R;
  } ଅନ୍ୟଥା ଯଦି (RHS < 0) {
    APInt::udivrem(LHS, -RHS, Quotient, R);
    Quotient.negate();
  } ଅନ୍ୟଥା {
    APInt::udivrem(LHS, RHS, Quotient, R);
  }
  Remainder = R;
}

APInt APInt::sadd_ov(ଅଚଳ APInt &RHS, ତାର୍କିକ &Overflow) ଅଚଳ {
  APInt Res = *ଏହି+RHS;
  Overflow = isNonNegative() == RHS.isNonNegative() &&
             Res.isNonNegative() != isNonNegative();
  ଫେରନ୍ତୁ Res;
}

APInt APInt::uadd_ov(ଅଚଳ APInt &RHS, ତାର୍କିକ &Overflow) ଅଚଳ {
  APInt Res = *ଏହି+RHS;
  Overflow = Res.ult(RHS);
  ଫେରନ୍ତୁ Res;
}

APInt APInt::ssub_ov(ଅଚଳ APInt &RHS, ତାର୍କିକ &Overflow) ଅଚଳ {
  APInt Res = *ଏହି - RHS;
  Overflow = isNonNegative() != RHS.isNonNegative() &&
             Res.isNonNegative() != isNonNegative();
  ଫେରନ୍ତୁ Res;
}

APInt APInt::usub_ov(ଅଚଳ APInt &RHS, ତାର୍କିକ &Overflow) ଅଚଳ {
  APInt Res = *ଏହି-RHS;
  Overflow = Res.ugt(*ଏହି);
  ଫେରନ୍ତୁ Res;
}

APInt APInt::sdiv_ov(ଅଚଳ APInt &RHS, ତାର୍କିକ &Overflow) ଅଚଳ {
  // MININT/-1  -->  overflow.
  Overflow = is_Min_Signed_ମୂଲ୍ୟ() && RHS.isAllOnes();
  ଫେରନ୍ତୁ sdiv(RHS);
}

APInt APInt::smul_ov(ଅଚଳ APInt &RHS, ତାର୍କିକ &Overflow) ଅଚଳ {
  APInt Res = *ଏହି * RHS;

  ଯଦି (RHS != 0)
    Overflow = Res.sdiv(RHS) != *ଏହି ||
               (is_Min_Signed_ମୂଲ୍ୟ() && RHS.isAllOnes());
  ଅନ୍ୟଥା
    Overflow = ମିଥ୍ୟା;
  ଫେରନ୍ତୁ Res;
}

APInt APInt::umul_ov(ଅଚଳ APInt &RHS, ତାର୍କିକ &Overflow) ଅଚଳ {
  ଯଦି (countl_zero() + RHS.countl_zero() + 2 <= BitWidth) {
    Overflow = ସତ୍ୟ;
    ଫେରନ୍ତୁ *ଏହି * RHS;
  }

  APInt Res = lshr(1) * RHS;
  Overflow = Res.isNegative();
  Res <<= 1;
  ଯଦି ((*ଏହି)[0]) {
    Res += RHS;
    ଯଦି (Res.ult(RHS))
      Overflow = ସତ୍ୟ;
  }
  ଫେରନ୍ତୁ Res;
}

APInt APInt::sshl_ov(ଅଚଳ APInt &ShAmt, ତାର୍କିକ &Overflow) ଅଚଳ {
  ଫେରନ୍ତୁ sshl_ov(ShAmt.get_Limited_ମୂଲ୍ୟ(getBitWidth()), Overflow);
}

APInt APInt::sshl_ov(unsigned ShAmt, ତାର୍କିକ &Overflow) ଅଚଳ {
  Overflow = ShAmt >= getBitWidth();
  ଯଦି (Overflow)
    ଫେରନ୍ତୁ APInt(BitWidth, 0);

  ଯଦି (isNonNegative()) // Don't allow sign change.
    Overflow = ShAmt >= countl_zero();
  ଅନ୍ୟଥା
    Overflow = ShAmt >= countl_one();

  ଫେରନ୍ତୁ *ଏହି << ShAmt;
}

APInt APInt::ushl_ov(ଅଚଳ APInt &ShAmt, ତାର୍କିକ &Overflow) ଅଚଳ {
  ଫେରନ୍ତୁ ushl_ov(ShAmt.get_Limited_ମୂଲ୍ୟ(getBitWidth()), Overflow);
}

APInt APInt::ushl_ov(unsigned ShAmt, ତାର୍କିକ &Overflow) ଅଚଳ {
  Overflow = ShAmt >= getBitWidth();
  ଯଦି (Overflow)
    ଫେରନ୍ତୁ APInt(BitWidth, 0);

  Overflow = ShAmt > countl_zero();

  ଫେରନ୍ତୁ *ଏହି << ShAmt;
}

APInt APInt::sfloordiv_ov(ଅଚଳ APInt &RHS, ତାର୍କିକ &Overflow) ଅଚଳ {
  APInt quotient = sdiv_ov(RHS, Overflow);
  ଯଦି ((quotient * RHS != *ଏହି) && (isNegative() != RHS.isNegative()))
    ଫେରନ୍ତୁ quotient - 1;
  ଫେରନ୍ତୁ quotient;
}

APInt APInt::sadd_sat(ଅଚଳ APInt &RHS) ଅଚଳ {
  ତାର୍କିକ Overflow;
  APInt Res = sadd_ov(RHS, Overflow);
  ଯଦି (!Overflow)
    ଫେରନ୍ତୁ Res;

  ଫେରନ୍ତୁ isNegative() ? APInt::get_Signed_Min_ମୂଲ୍ୟ(BitWidth)
                      : APInt::get_Signed_Max_ମୂଲ୍ୟ(BitWidth);
}

APInt APInt::uadd_sat(ଅଚଳ APInt &RHS) ଅଚଳ {
  ତାର୍କିକ Overflow;
  APInt Res = uadd_ov(RHS, Overflow);
  ଯଦି (!Overflow)
    ଫେରନ୍ତୁ Res;

  ଫେରନ୍ତୁ APInt::get_Max_ମୂଲ୍ୟ(BitWidth);
}

APInt APInt::ssub_sat(ଅଚଳ APInt &RHS) ଅଚଳ {
  ତାର୍କିକ Overflow;
  APInt Res = ssub_ov(RHS, Overflow);
  ଯଦି (!Overflow)
    ଫେରନ୍ତୁ Res;

  ଫେରନ୍ତୁ isNegative() ? APInt::get_Signed_Min_ମୂଲ୍ୟ(BitWidth)
                      : APInt::get_Signed_Max_ମୂଲ୍ୟ(BitWidth);
}

APInt APInt::usub_sat(ଅଚଳ APInt &RHS) ଅଚଳ {
  ତାର୍କିକ Overflow;
  APInt Res = usub_ov(RHS, Overflow);
  ଯଦି (!Overflow)
    ଫେରନ୍ତୁ Res;

  ଫେରନ୍ତୁ APInt(BitWidth, 0);
}

APInt APInt::smul_sat(ଅଚଳ APInt &RHS) ଅଚଳ {
  ତାର୍କିକ Overflow;
  APInt Res = smul_ov(RHS, Overflow);
  ଯଦି (!Overflow)
    ଫେରନ୍ତୁ Res;

  // The result is negative if one and only one of inputs is negative.
  ତାର୍କିକ ResIsNegative = isNegative() ^ RHS.isNegative();

  ଫେରନ୍ତୁ ResIsNegative ? APInt::get_Signed_Min_ମୂଲ୍ୟ(BitWidth)
                       : APInt::get_Signed_Max_ମୂଲ୍ୟ(BitWidth);
}

APInt APInt::umul_sat(ଅଚଳ APInt &RHS) ଅଚଳ {
  ତାର୍କିକ Overflow;
  APInt Res = umul_ov(RHS, Overflow);
  ଯଦି (!Overflow)
    ଫେରନ୍ତୁ Res;

  ଫେରନ୍ତୁ APInt::get_Max_ମୂଲ୍ୟ(BitWidth);
}

APInt APInt::sshl_sat(ଅଚଳ APInt &RHS) ଅଚଳ {
  ଫେରନ୍ତୁ sshl_sat(RHS.get_Limited_ମୂଲ୍ୟ(getBitWidth()));
}

APInt APInt::sshl_sat(unsigned RHS) ଅଚଳ {
  ତାର୍କିକ Overflow;
  APInt Res = sshl_ov(RHS, Overflow);
  ଯଦି (!Overflow)
    ଫେରନ୍ତୁ Res;

  ଫେରନ୍ତୁ isNegative() ? APInt::get_Signed_Min_ମୂଲ୍ୟ(BitWidth)
                      : APInt::get_Signed_Max_ମୂଲ୍ୟ(BitWidth);
}

APInt APInt::ushl_sat(ଅଚଳ APInt &RHS) ଅଚଳ {
  ଫେରନ୍ତୁ ushl_sat(RHS.get_Limited_ମୂଲ୍ୟ(getBitWidth()));
}

APInt APInt::ushl_sat(unsigned RHS) ଅଚଳ {
  ତାର୍କିକ Overflow;
  APInt Res = ushl_ov(RHS, Overflow);
  ଯଦି (!Overflow)
    ଫେରନ୍ତୁ Res;

  ଫେରନ୍ତୁ APInt::get_Max_ମୂଲ୍ୟ(BitWidth);
}

ଶୂନ୍ୟ APInt::fromString(unsigned numbits, StringRef str, uint8_t radix) {
  // Check our assumptions here
  ପ୍ରତିଜ୍ଞା(!str.empty() && "Invalid string length");
  ପ୍ରତିଜ୍ଞା((radix == 10 || radix == 8 || radix == 16 || radix == 2 ||
          radix == 36) &&
         "Radix should be 2, 8, 10, 16, or 36!");

  StringRef::iterator p = str.begin();
  size_t slen = str.size();
  ତାର୍କିକ isNeg = *p == '-';
  ଯଦି (*p == '-' || *p == '+') {
    p++;
    slen--;
    ପ୍ରତିଜ୍ଞା(slen && "String is only a sign, needs a value.");
  }
  ପ୍ରତିଜ୍ଞା((slen <= numbits || radix != 2) && "Insufficient bit width");
  ପ୍ରତିଜ୍ଞା(((slen-1)*3 <= numbits || radix != 8) && "Insufficient bit width");
  ପ୍ରତିଜ୍ଞା(((slen-1)*4 <= numbits || radix != 16) && "Insufficient bit width");
  ପ୍ରତିଜ୍ଞା((((slen-1)*64)/22 <= numbits || radix != 10) &&
         "Insufficient bit width");

  // Allocate memory if needed
  ଯଦି (LLVM_LIKELY(isSingleWord()))
    U.VAL = 0;
  ଅନ୍ୟଥା
    U.pVal = getClearedMemory(get_ଅଙ୍କ_ଶବ୍ଦଗୁଡ଼ିକ());

  // Figure out if we can shift instead of multiply
  unsigned shift = (radix == 16 ? 4 : radix == 8 ? 3 : radix == 2 ? 1 : 0);

  // Enter digit traversal loop
  ପାଇଁ (StringRef::iterator e = str.end(); p != e; ++p) {
    unsigned digit = getDigit(*p, radix);
    ପ୍ରତିଜ୍ଞା(digit < radix && "Invalid character in digit string");

    // Shift or multiply the value by the radix
    ଯଦି (slen > 1) {
      ଯଦି (shift)
        *ଏହି <<= shift;
      ଅନ୍ୟଥା
        *ଏହି *= radix;
    }

    // Add in the digit we just interpreted
    *ଏହି += digit;
  }
  // If its negative, put it in two's complement form
  ଯଦି (isNeg)
    ଏହି->negate();
}

ଶୂନ୍ୟ APInt::toString(SmallVectorImpl<ଅକ୍ଷର> &Str, unsigned Radix, ତାର୍କିକ Signed,
                     ତାର୍କିକ formatAsCLiteral, ତାର୍କିକ UpperCase,
                     ତାର୍କିକ ଭର୍ତ୍ତି_Separators) ଅଚଳ {
  ପ୍ରତିଜ୍ଞା((Radix == 10 || Radix == 8 || Radix == 16 || Radix == 2 ||
          Radix == 36) &&
         "Radix should be 2, 8, 10, 16, or 36!");

  ଅଚଳ ଅକ୍ଷର *Prefix = "";
  ଯଦି (formatAsCLiteral) {
    ବାଛ (Radix) {
      ସ୍ଥିତି 2:
        // Binary literals are a non-standard extension added in gcc 4.3:
        // http://gcc.gnu.org/onlinedocs/gcc-4.3.0/gcc/Binary-constants.html
        Prefix = "0b";
        ଭାଙ୍ଗ;
      ସ୍ଥିତି 8:
        Prefix = "0";
        ଭାଙ୍ଗ;
      ସ୍ଥିତି 10:
        ଭାଙ୍ଗ; // No prefix
      ସ୍ଥିତି 16:
        Prefix = "0x";
        ଭାଙ୍ଗ;
      ପୂର୍ବନିର୍ଦ୍ଧାରିତ:
        llvm_unreachable("Invalid radix!");
    }
  }

  // Number of digits in a group between separators.
  unsigned Grouping = (Radix == 8 || Radix == 10) ? 3 : 4;

  // First, check for a zero value and just short circuit the logic below.
  ଯଦି (isZero()) {
    ଯେତେବେଳେ (*Prefix) {
      Str.ଠେଲ_back(*Prefix);
      ++Prefix;
    };
    Str.ଠେଲ_back('0');
    ଫେରନ୍ତୁ;
  }

  ସ୍ଥିର ଅଚଳ ଅକ୍ଷର BothDigits[] = "0123456789abcdefghijklmnopqrstuvwxyz"
                                   "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ";
  ଅଚଳ ଅକ୍ଷର *Digits = BothDigits + (UpperCase ? 36 : 0);

  ଯଦି (LLVM_LIKELY(isSingleWord())) {
    ଅକ୍ଷର Buffer[65];
    ଅକ୍ଷର *BufPtr = std::end(Buffer);

    uint64_t N;
    ଯଦି (!Signed) {
      N = get_ZExt_ମୂଲ୍ୟ();
    } ଅନ୍ୟଥା {
      int64_t I = get_SExt_ମୂଲ୍ୟ();
      ଯଦି (I >= 0) {
        N = I;
      } ଅନ୍ୟଥା {
        Str.ଠେଲ_back('-');
        N = -(uint64_t)I;
      }
    }

    ଯେତେବେଳେ (*Prefix) {
      Str.ଠେଲ_back(*Prefix);
      ++Prefix;
    };

    ପୂର୍ଣ୍ଣସଂଖ୍ୟା Pos = 0;
    ଯେତେବେଳେ (N) {
      ଯଦି (ଭର୍ତ୍ତି_Separators && Pos % Grouping == 0 && Pos > 0)
        *--BufPtr = '\'';
      *--BufPtr = Digits[N % Radix];
      N /= Radix;
      Pos++;
    }
    Str.append(BufPtr, std::end(Buffer));
    ଫେରନ୍ତୁ;
  }

  APInt Tmp(*ଏହି);

  ଯଦି (Signed && isNegative()) {
    // They want to print the signed version and it is a negative value
    // Flip the bits and add one to turn it into the equivalent positive
    // value and put a '-' in the result.
    Tmp.negate();
    Str.ଠେଲ_back('-');
  }

  ଯେତେବେଳେ (*Prefix) {
    Str.ଠେଲ_back(*Prefix);
    ++Prefix;
  }

  // We insert the digits backward, then reverse them to get the right order.
  unsigned StartDig = Str.size();

  // For the 2, 8 and 16 bit cases, we can just shift instead of divide
  // because the number of bits per digit (1, 3 and 4 respectively) divides
  // equally.  We just shift until the value is zero.
  ଯଦି (Radix == 2 || Radix == 8 || Radix == 16) {
    // Just shift tmp right for each digit width until it becomes zero
    unsigned ShiftAmt = (Radix == 16 ? 4 : (Radix == 8 ? 3 : 1));
    unsigned MaskAmt = Radix - 1;

    ପୂର୍ଣ୍ଣସଂଖ୍ୟା Pos = 0;
    ଯେତେବେଳେ (Tmp.get_Bool_ମୂଲ୍ୟ()) {
      unsigned Digit = unsigned(Tmp.getRawData()[0]) & MaskAmt;
      ଯଦି (ଭର୍ତ୍ତି_Separators && Pos % Grouping == 0 && Pos > 0)
        Str.ଠେଲ_back('\'');

      Str.ଠେଲ_back(Digits[Digit]);
      Tmp.lshrInPlace(ShiftAmt);
      Pos++;
    }
  } ଅନ୍ୟଥା {
    ପୂର୍ଣ୍ଣସଂଖ୍ୟା Pos = 0;
    ଯେତେବେଳେ (Tmp.get_Bool_ମୂଲ୍ୟ()) {
      uint64_t Digit;
      udivrem(Tmp, Radix, Tmp, Digit);
      ପ୍ରତିଜ୍ଞା(Digit < Radix && "divide failed");
      ଯଦି (ଭର୍ତ୍ତି_Separators && Pos % Grouping == 0 && Pos > 0)
        Str.ଠେଲ_back('\'');

      Str.ଠେଲ_back(Digits[Digit]);
      Pos++;
    }
  }

  // Reverse the digits before returning.
  std::reverse(Str.begin()+StartDig, Str.end());
}

#ଯଦି !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)
LLVM_DUMP_METHOD ଶୂନ୍ୟ APInt::dump() ଅଚଳ {
  SmallString<40> S, U;
  ଏହି->toStringUnsigned(U);
  ଏହି->toStringSigned(S);
  dbgs() << "APInt(" << BitWidth << "b, "
         << U << "u " << S << "s)\n";
}
#endif

ଶୂନ୍ୟ APInt::print(raw_ostream &OS, ତାର୍କିକ isSigned) ଅଚଳ {
  SmallString<40> S;
  ଏହି->toString(S, 10, isSigned, /* formatAsCLiteral = */ମିଥ୍ୟା);
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
ସ୍ଥିର ଧାଡ଼ିଭିତରେ APInt::WordType ତଳ_Bit_Mask(unsigned bits) {
  ପ୍ରତିଜ୍ଞା(bits != 0 && bits <= APInt::APINT_BITS_PER_WORD);
  ଫେରନ୍ତୁ ~(APInt::WordType) 0 >> (APInt::APINT_BITS_PER_WORD - bits);
}

/// Returns the value of the lower half of PART.
ସ୍ଥିର ଧାଡ଼ିଭିତରେ APInt::WordType ତଳ_Half(APInt::WordType part) {
  ଫେରନ୍ତୁ part & ତଳ_Bit_Mask(APInt::APINT_BITS_PER_WORD / 2);
}

/// Returns the value of the upper half of PART.
ସ୍ଥିର ଧାଡ଼ିଭିତରେ APInt::WordType ଉପର_Half(APInt::WordType part) {
  ଫେରନ୍ତୁ part >> (APInt::APINT_BITS_PER_WORD / 2);
}

/// Sets the least significant part of a bignum to the input value, and zeroes
/// out higher parts.
ଶୂନ୍ୟ APInt::tcSet(WordType *dst, WordType part, unsigned parts) {
  ପ୍ରତିଜ୍ଞା(parts > 0);
  dst[0] = part;
  ପାଇଁ (unsigned i = 1; i < parts; i++)
    dst[i] = 0;
}

/// Assign one bignum to another.
ଶୂନ୍ୟ APInt::tcAssign(WordType *dst, ଅଚଳ WordType *src, unsigned parts) {
  ପାଇଁ (unsigned i = 0; i < parts; i++)
    dst[i] = src[i];
}

/// Returns true if a bignum is zero, false otherwise.
ତାର୍କିକ APInt::tcIsZero(ଅଚଳ WordType *src, unsigned parts) {
  ପାଇଁ (unsigned i = 0; i < parts; i++)
    ଯଦି (src[i])
      ଫେରନ୍ତୁ ମିଥ୍ୟା;

  ଫେରନ୍ତୁ ସତ୍ୟ;
}

/// Extract the given bit of a bignum; returns 0 or 1.
ପୂର୍ଣ୍ଣସଂଖ୍ୟା APInt::tcExtractBit(ଅଚଳ WordType *parts, unsigned bit) {
  ଫେରନ୍ତୁ (parts[whichWord(bit)] & maskBit(bit)) != 0;
}

/// Set the given bit of a bignum.
ଶୂନ୍ୟ APInt::tcSetBit(WordType *parts, unsigned bit) {
  parts[whichWord(bit)] |= maskBit(bit);
}

/// Clears the given bit of a bignum.
ଶୂନ୍ୟ APInt::tcClearBit(WordType *parts, unsigned bit) {
  parts[whichWord(bit)] &= ~maskBit(bit);
}

/// Returns the bit number of the least significant set bit of a number.  If the
/// input number has no bits set UINT_MAX is returned.
unsigned APInt::tcLSB(ଅଚଳ WordType *parts, unsigned n) {
  ପାଇଁ (unsigned i = 0; i < n; i++) {
    ଯଦି (parts[i] != 0) {
      unsigned lsb = llvm::countr_zero(parts[i]);
      ଫେରନ୍ତୁ lsb + i * APINT_BITS_PER_WORD;
    }
  }

  ଫେରନ୍ତୁ UINT_MAX;
}

/// Returns the bit number of the most significant set bit of a number.
/// If the input number has no bits set UINT_MAX is returned.
unsigned APInt::tcMSB(ଅଚଳ WordType *parts, unsigned n) {
  କର {
    --n;

    ଯଦି (parts[n] != 0) {
      static_assert(ଆକାର(parts[n]) <= ଆକାର(uint64_t));
      unsigned msb = llvm::Log2_64(parts[n]);

      ଫେରନ୍ତୁ msb + n * APINT_BITS_PER_WORD;
    }
  } ଯେତେବେଳେ (n);

  ଫେରନ୍ତୁ UINT_MAX;
}

/// Copy the bit vector of width srcBITS from SRC, starting at bit srcLSB, to
/// DST, of dstCOUNT parts, such that the bit srcLSB becomes the least
/// significant bit of DST.  All high bits above srcBITS in DST are zero-filled.
/// */
ଶୂନ୍ୟ
APInt::tcExtract(WordType *dst, unsigned dst_ଗଣନା, ଅଚଳ WordType *src,
                 unsigned srcBits, unsigned srcLSB) {
  unsigned dstParts = (srcBits + APINT_BITS_PER_WORD - 1) / APINT_BITS_PER_WORD;
  ପ୍ରତିଜ୍ଞା(dstParts <= dst_ଗଣନା);

  unsigned firstSrcPart = srcLSB / APINT_BITS_PER_WORD;
  tcAssign(dst, src + firstSrcPart, dstParts);

  unsigned shift = srcLSB % APINT_BITS_PER_WORD;
  tc_Shift_ଡାହାଣ(dst, dstParts, shift);

  // We now have (dstParts * APINT_BITS_PER_WORD - shift) bits from SRC
  // in DST.  If this is less that srcBits, append the rest, else
  // clear the high bits.
  unsigned n = dstParts * APINT_BITS_PER_WORD - shift;
  ଯଦି (n < srcBits) {
    WordType mask = ତଳ_Bit_Mask (srcBits - n);
    dst[dstParts - 1] |= ((src[firstSrcPart + dstParts] & mask)
                          << n % APINT_BITS_PER_WORD);
  } ଅନ୍ୟଥା ଯଦି (n > srcBits) {
    ଯଦି (srcBits % APINT_BITS_PER_WORD)
      dst[dstParts - 1] &= ତଳ_Bit_Mask (srcBits % APINT_BITS_PER_WORD);
  }

  // Clear high parts.
  ଯେତେବେଳେ (dstParts < dst_ଗଣନା)
    dst[dstParts++] = 0;
}

//// DST += RHS + C where C is zero or one.  Returns the carry flag.
APInt::WordType APInt::tcAdd(WordType *dst, ଅଚଳ WordType *rhs,
                             WordType c, unsigned parts) {
  ପ୍ରତିଜ୍ଞା(c <= 1);

  ପାଇଁ (unsigned i = 0; i < parts; i++) {
    WordType l = dst[i];
    ଯଦି (c) {
      dst[i] += rhs[i] + 1;
      c = (dst[i] <= l);
    } ଅନ୍ୟଥା {
      dst[i] += rhs[i];
      c = (dst[i] < l);
    }
  }

  ଫେରନ୍ତୁ c;
}

/// This function adds a single "word" integer, src, to the multiple
/// "word" integer array, dst[]. dst[] is modified to reflect the addition and
/// 1 is returned if there is a carry out, otherwise 0 is returned.
/// @returns the carry of the addition.
APInt::WordType APInt::tcAddPart(WordType *dst, WordType src,
                                 unsigned parts) {
  ପାଇଁ (unsigned i = 0; i < parts; ++i) {
    dst[i] += src;
    ଯଦି (dst[i] >= src)
      ଫେରନ୍ତୁ 0; // No need to carry so exit early.
    src = 1; // Carry one to next digit.
  }

  ଫେରନ୍ତୁ 1;
}

/// DST -= RHS + C where C is zero or one.  Returns the carry flag.
APInt::WordType APInt::tcSubtract(WordType *dst, ଅଚଳ WordType *rhs,
                                  WordType c, unsigned parts) {
  ପ୍ରତିଜ୍ଞା(c <= 1);

  ପାଇଁ (unsigned i = 0; i < parts; i++) {
    WordType l = dst[i];
    ଯଦି (c) {
      dst[i] -= rhs[i] + 1;
      c = (dst[i] >= l);
    } ଅନ୍ୟଥା {
      dst[i] -= rhs[i];
      c = (dst[i] > l);
    }
  }

  ଫେରନ୍ତୁ c;
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
  ପାଇଁ (unsigned i = 0; i < parts; ++i) {
    WordType Dst = dst[i];
    dst[i] -= src;
    ଯଦି (src <= Dst)
      ଫେରନ୍ତୁ 0; // No need to borrow so exit early.
    src = 1; // We have to "borrow 1" from next "word"
  }

  ଫେରନ୍ତୁ 1;
}

/// Negate a bignum in-place.
ଶୂନ୍ୟ APInt::tcNegate(WordType *dst, unsigned parts) {
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
ପୂର୍ଣ୍ଣସଂଖ୍ୟା APInt::tcMultiplyPart(WordType *dst, ଅଚଳ WordType *src,
                          WordType multiplier, WordType carry,
                          unsigned srcParts, unsigned dstParts,
                          ତାର୍କିକ add) {
  // Otherwise our writes of DST kill our later reads of SRC.
  ପ୍ରତିଜ୍ଞା(dst <= src || dst >= src + srcParts);
  ପ୍ରତିଜ୍ଞା(dstParts <= srcParts + 1);

  // N loops; minimum of dstParts and srcParts.
  unsigned n = std::min(dstParts, srcParts);

  ପାଇଁ (unsigned i = 0; i < n; i++) {
    // [LOW, HIGH] = MULTIPLIER * SRC[i] + DST[i] + CARRY.
    // This cannot overflow, because:
    //   (n - 1) * (n - 1) + 2 (n - 1) = (n - 1) * (n + 1)
    // which is less than n^2.
    WordType srcPart = src[i];
    WordType ତଳ, mid, ଉପର;
    ଯଦି (multiplier == 0 || srcPart == 0) {
      ତଳ = carry;
      ଉପର = 0;
    } ଅନ୍ୟଥା {
      ତଳ = ତଳ_Half(srcPart) * ତଳ_Half(multiplier);
      ଉପର = ଉପର_Half(srcPart) * ଉପର_Half(multiplier);

      mid = ତଳ_Half(srcPart) * ଉପର_Half(multiplier);
      ଉପର += ଉପର_Half(mid);
      mid <<= APINT_BITS_PER_WORD / 2;
      ଯଦି (ତଳ + mid < ତଳ)
        ଉପର++;
      ତଳ += mid;

      mid = ଉପର_Half(srcPart) * ତଳ_Half(multiplier);
      ଉପର += ଉପର_Half(mid);
      mid <<= APINT_BITS_PER_WORD / 2;
      ଯଦି (ତଳ + mid < ତଳ)
        ଉପର++;
      ତଳ += mid;

      // Now add carry.
      ଯଦି (ତଳ + carry < ତଳ)
        ଉପର++;
      ତଳ += carry;
    }

    ଯଦି (add) {
      // And now DST[i], and store the new low part there.
      ଯଦି (ତଳ + dst[i] < ତଳ)
        ଉପର++;
      dst[i] += ତଳ;
    } ଅନ୍ୟଥା {
      dst[i] = ତଳ;
    }

    carry = ଉପର;
  }

  ଯଦି (srcParts < dstParts) {
    // Full multiplication, there is no overflow.
    ପ୍ରତିଜ୍ଞା(srcParts + 1 == dstParts);
    dst[srcParts] = carry;
    ଫେରନ୍ତୁ 0;
  }

  // We overflowed if there is carry.
  ଯଦି (carry)
    ଫେରନ୍ତୁ 1;

  // We would overflow if any significant unwritten parts would be
  // non-zero.  This is true if any remaining src parts are non-zero
  // and the multiplier is non-zero.
  ଯଦି (multiplier)
    ପାଇଁ (unsigned i = dstParts; i < srcParts; i++)
      ଯଦି (src[i])
        ଫେରନ୍ତୁ 1;

  // We fitted in the narrow destination.
  ଫେରନ୍ତୁ 0;
}

/// DST = LHS * RHS, where DST has the same width as the operands and
/// is filled with the least significant parts of the result.  Returns
/// one if overflow occurred, otherwise zero.  DST must be disjoint
/// from both operands.
ପୂର୍ଣ୍ଣସଂଖ୍ୟା APInt::tcMultiply(WordType *dst, ଅଚଳ WordType *lhs,
                      ଅଚଳ WordType *rhs, unsigned parts) {
  ପ୍ରତିଜ୍ଞା(dst != lhs && dst != rhs);

  ପୂର୍ଣ୍ଣସଂଖ୍ୟା overflow = 0;

  ପାଇଁ (unsigned i = 0; i < parts; i++) {
    // Don't accumulate on the first iteration so we don't need to initalize
    // dst to 0.
    overflow |=
        tcMultiplyPart(&dst[i], lhs, rhs[i], 0, parts, parts - i, i != 0);
  }

  ଫେରନ୍ତୁ overflow;
}

/// DST = LHS * RHS, where DST has width the sum of the widths of the
/// operands. No overflow occurs. DST must be disjoint from both operands.
ଶୂନ୍ୟ APInt::tcFullMultiply(WordType *dst, ଅଚଳ WordType *lhs,
                           ଅଚଳ WordType *rhs, unsigned lhsParts,
                           unsigned rhsParts) {
  // Put the narrower number on the LHS for less loops below.
  ଯଦି (lhsParts > rhsParts)
    ଫେରନ୍ତୁ tcFullMultiply (dst, rhs, lhs, rhsParts, lhsParts);

  ପ୍ରତିଜ୍ଞା(dst != lhs && dst != rhs);

  ପାଇଁ (unsigned i = 0; i < lhsParts; i++) {
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
ପୂର୍ଣ୍ଣସଂଖ୍ୟା APInt::tcDivide(WordType *lhs, ଅଚଳ WordType *rhs,
                    WordType *remainder, WordType *srhs,
                    unsigned parts) {
  ପ୍ରତିଜ୍ଞା(lhs != remainder && lhs != srhs && remainder != srhs);

  unsigned shift_ଗଣନା = tcMSB(rhs, parts) + 1;
  ଯଦି (shift_ଗଣନା == 0)
    ଫେରନ୍ତୁ ସତ୍ୟ;

  shift_ଗଣନା = parts * APINT_BITS_PER_WORD - shift_ଗଣନା;
  unsigned n = shift_ଗଣନା / APINT_BITS_PER_WORD;
  WordType mask = (WordType) 1 << (shift_ଗଣନା % APINT_BITS_PER_WORD);

  tcAssign(srhs, rhs, parts);
  tc_Shift_ବାମ(srhs, parts, shift_ଗଣନା);
  tcAssign(remainder, lhs, parts);
  tcSet(lhs, 0, parts);

  // Loop, subtracting SRHS if REMAINDER is greater and adding that to the
  // total.
  ପାଇଁ (;;) {
    ପୂର୍ଣ୍ଣସଂଖ୍ୟା compare = tcCompare(remainder, srhs, parts);
    ଯଦି (compare >= 0) {
      tcSubtract(remainder, srhs, 0, parts);
      lhs[n] |= mask;
    }

    ଯଦି (shift_ଗଣନା == 0)
      ଭାଙ୍ଗ;
    shift_ଗଣନା--;
    tc_Shift_ଡାହାଣ(srhs, parts, 1);
    ଯଦି ((mask >>= 1) == 0) {
      mask = (WordType) 1 << (APINT_BITS_PER_WORD - 1);
      n--;
    }
  }

  ଫେରନ୍ତୁ ମିଥ୍ୟା;
}

/// Shift a bignum left Count bits in-place. Shifted in bits are zero. There are
/// no restrictions on Count.
ଶୂନ୍ୟ APInt::tc_Shift_ବାମ(WordType *Dst, unsigned ଶବ୍ଦଗୁଡ଼ିକ_2, unsigned ଗଣନା_3) {
  // Don't bother performing a no-op shift.
  ଯଦି (!ଗଣନା_3)
    ଫେରନ୍ତୁ;

  // WordShift is the inter-part shift; BitShift is the intra-part shift.
  unsigned WordShift = std::min(ଗଣନା_3 / APINT_BITS_PER_WORD, ଶବ୍ଦଗୁଡ଼ିକ_2);
  unsigned BitShift = ଗଣନା_3 % APINT_BITS_PER_WORD;

  // Fastpath for moving by whole words.
  ଯଦି (BitShift == 0) {
    std::memmove(Dst + WordShift, Dst, (ଶବ୍ଦଗୁଡ଼ିକ_2 - WordShift) * APINT_WORD_SIZE);
  } ଅନ୍ୟଥା {
    ଯେତେବେଳେ (ଶବ୍ଦଗୁଡ଼ିକ_2-- > WordShift) {
      Dst[ଶବ୍ଦଗୁଡ଼ିକ_2] = Dst[ଶବ୍ଦଗୁଡ଼ିକ_2 - WordShift] << BitShift;
      ଯଦି (ଶବ୍ଦଗୁଡ଼ିକ_2 > WordShift)
        Dst[ଶବ୍ଦଗୁଡ଼ିକ_2] |=
          Dst[ଶବ୍ଦଗୁଡ଼ିକ_2 - WordShift - 1] >> (APINT_BITS_PER_WORD - BitShift);
    }
  }

  // Fill in the remainder with 0s.
  std::memset(Dst, 0, WordShift * APINT_WORD_SIZE);
}

/// Shift a bignum right Count bits in-place. Shifted in bits are zero. There
/// are no restrictions on Count.
ଶୂନ୍ୟ APInt::tc_Shift_ଡାହାଣ(WordType *Dst, unsigned ଶବ୍ଦଗୁଡ଼ିକ_2, unsigned ଗଣନା_3) {
  // Don't bother performing a no-op shift.
  ଯଦି (!ଗଣନା_3)
    ଫେରନ୍ତୁ;

  // WordShift is the inter-part shift; BitShift is the intra-part shift.
  unsigned WordShift = std::min(ଗଣନା_3 / APINT_BITS_PER_WORD, ଶବ୍ଦଗୁଡ଼ିକ_2);
  unsigned BitShift = ଗଣନା_3 % APINT_BITS_PER_WORD;

  unsigned ଶବ୍ଦଗୁଡ଼ିକ_To_Move = ଶବ୍ଦଗୁଡ଼ିକ_2 - WordShift;
  // Fastpath for moving by whole words.
  ଯଦି (BitShift == 0) {
    std::memmove(Dst, Dst + WordShift, ଶବ୍ଦଗୁଡ଼ିକ_To_Move * APINT_WORD_SIZE);
  } ଅନ୍ୟଥା {
    ପାଇଁ (unsigned i = 0; i != ଶବ୍ଦଗୁଡ଼ିକ_To_Move; ++i) {
      Dst[i] = Dst[i + WordShift] >> BitShift;
      ଯଦି (i + 1 != ଶବ୍ଦଗୁଡ଼ିକ_To_Move)
        Dst[i] |= Dst[i + WordShift + 1] << (APINT_BITS_PER_WORD - BitShift);
    }
  }

  // Fill in the remainder with 0s.
  std::memset(Dst + ଶବ୍ଦଗୁଡ଼ିକ_To_Move, 0, WordShift * APINT_WORD_SIZE);
}

// Comparison (unsigned) of two bignums.
ପୂର୍ଣ୍ଣସଂଖ୍ୟା APInt::tcCompare(ଅଚଳ WordType *lhs, ଅଚଳ WordType *rhs,
                     unsigned parts) {
  ଯେତେବେଳେ (parts) {
    parts--;
    ଯଦି (lhs[parts] != rhs[parts])
      ଫେରନ୍ତୁ (lhs[parts] > rhs[parts]) ? 1 : -1;
  }

  ଫେରନ୍ତୁ 0;
}

APInt llvm::APIntOps::RoundingUDiv(ଅଚଳ APInt &A, ଅଚଳ APInt &B,
                                   APInt::Rounding RM) {
  // Currently udivrem always rounds down.
  ବାଛ (RM) {
  ସ୍ଥିତି APInt::Rounding::DOWN:
  ସ୍ଥିତି APInt::Rounding::TOWARD_ZERO:
    ଫେରନ୍ତୁ A.udiv(B);
  ସ୍ଥିତି APInt::Rounding::UP: {
    APInt Quo, Rem;
    APInt::udivrem(A, B, Quo, Rem);
    ଯଦି (Rem.isZero())
      ଫେରନ୍ତୁ Quo;
    ଫେରନ୍ତୁ Quo + 1;
  }
  }
  llvm_unreachable("Unknown APInt::Rounding enum");
}

APInt llvm::APIntOps::RoundingSDiv(ଅଚଳ APInt &A, ଅଚଳ APInt &B,
                                   APInt::Rounding RM) {
  ବାଛ (RM) {
  ସ୍ଥିତି APInt::Rounding::DOWN:
  ସ୍ଥିତି APInt::Rounding::UP: {
    APInt Quo, Rem;
    APInt::sdivrem(A, B, Quo, Rem);
    ଯଦି (Rem.isZero())
      ଫେରନ୍ତୁ Quo;
    // This algorithm deals with arbitrary rounding mode used by sdivrem.
    // We want to check whether the non-integer part of the mathematical value
    // is negative or not. If the non-integer part is negative, we need to round
    // down from Quo; otherwise, if it's positive or 0, we return Quo, as it's
    // already rounded down.
    ଯଦି (RM == APInt::Rounding::DOWN) {
      ଯଦି (Rem.isNegative() != B.isNegative())
        ଫେରନ୍ତୁ Quo - 1;
      ଫେରନ୍ତୁ Quo;
    }
    ଯଦି (Rem.isNegative() != B.isNegative())
      ଫେରନ୍ତୁ Quo;
    ଫେରନ୍ତୁ Quo + 1;
  }
  // Currently sdiv rounds towards zero.
  ସ୍ଥିତି APInt::Rounding::TOWARD_ZERO:
    ଫେରନ୍ତୁ A.sdiv(B);
  }
  llvm_unreachable("Unknown APInt::Rounding enum");
}

std::optional<APInt>
llvm::APIntOps::SolveQuadraticEquationWrap(APInt A, APInt B, APInt C,
                                           unsigned RangeWidth) {
  unsigned CoeffWidth = A.getBitWidth();
  ପ୍ରତିଜ୍ଞା(CoeffWidth == B.getBitWidth() && CoeffWidth == C.getBitWidth());
  ପ୍ରତିଜ୍ଞା(RangeWidth <= CoeffWidth &&
         "Value range width should be less than coefficient width");
  ପ୍ରତିଜ୍ଞା(RangeWidth > 1 && "Value range bit width should be > 1");

  LLVM_DEBUG(dbgs() << __func__ << ": solving " << A << "x^2 + " << B
                    << "x + " << C << ", rw:" << RangeWidth << '\n');

  // Identify 0 as a (non)solution immediately.
  ଯଦି (C.sextOrTrunc(RangeWidth).isZero()) {
    LLVM_DEBUG(dbgs() << __func__ << ": zero solution\n");
    ଫେରନ୍ତୁ APInt(CoeffWidth, 0);
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
  ଯଦି (A.isNegative()) {
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
  ତାର୍କିକ Pick_ତଳ;

  auto RoundUp = [] (ଅଚଳ APInt &V, ଅଚଳ APInt &A) -> APInt {
    ପ୍ରତିଜ୍ଞା(A.isStrictlyPositive());
    APInt T = V.abs().urem(A);
    ଯଦି (T.isZero())
      ଫେରନ୍ତୁ V;
    ଫେରନ୍ତୁ V.isNegative() ? V+T : V+(A-T);
  };

  // The vertex of the parabola is at -B/2A, but since A > 0, it's negative
  // iff B is positive.
  ଯଦି (B.isNonNegative()) {
    // If B >= 0, the vertex it at a negative location (or at 0), so in
    // order to have a non-negative solution we need to pick k that makes
    // C-kR negative. To satisfy all the requirements for the solution
    // that we are looking for, it needs to be closest to 0 of all k.
    C = C.srem(R);
    ଯଦି (C.isStrictlyPositive())
      C -= R;
    // Pick the greater solution.
    Pick_ତଳ = ମିଥ୍ୟା;
  } ଅନ୍ୟଥା {
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
    ଯଦି (C.sgt(LowkR)) {
      // If LowkR < C, then such a k is guaranteed to exist because
      // LowkR itself is a multiple of R.
      C -= -RoundUp(-C, R);      // C = C - RoundDown(C, R)
      // Pick the smaller solution.
      Pick_ତଳ = ସତ୍ୟ;
    } ଅନ୍ୟଥା {
      // If C-kR < 0 for all potential k's, it means that one solution
      // will be negative, while the other will be positive. The positive
      // solution will shift towards 0 if the parabola is moved up.
      // Pick the kR closest to the lower bound (i.e. make C-kR closest
      // to 0, or in other words, out of all parabolas that have solutions,
      // pick the one that is the farthest "up").
      // Since LowkR is itself a multiple of R, simply take C-LowkR.
      C -= LowkR;
      // Pick the greater solution.
      Pick_ତଳ = ମିଥ୍ୟା;
    }
  }

  LLVM_DEBUG(dbgs() << __func__ << ": updated coefficients " << A << "x^2 + "
                    << B << "x + " << C << ", rw:" << RangeWidth << '\n');

  APInt D = SqrB - 4*A*C;
  ପ୍ରତିଜ୍ଞା(D.isNonNegative() && "Negative discriminant");
  APInt SQ = D.sqrtFloor();

  APInt Q = SQ * SQ;
  ତାର୍କିକ InexactSQ = Q != D;

  APInt X;
  APInt Rem;

  // SQ is rounded down (i.e SQ * SQ <= D), so the roots may be inexact.
  // When using the quadratic formula directly, the calculated low root
  // may be greater than the exact one, since we would be subtracting SQ.
  // To make sure that the calculated root is not greater than the exact
  // one, subtract SQ+1 when calculating the low root (for inexact value
  // of SQ).
  ଯଦି (Pick_ତଳ)
    APInt::sdivrem(-B - (SQ+InexactSQ), TwoA, X, Rem);
  ଅନ୍ୟଥା
    APInt::sdivrem(-B + SQ, TwoA, X, Rem);

  // The updated coefficients should be such that the (exact) solution is
  // positive. Since APInt division rounds towards 0, the calculated one
  // can be 0, but cannot be negative.
  ପ୍ରତିଜ୍ଞା(X.isNonNegative() && "Solution should be non-negative");

  ଯଦି (!InexactSQ && Rem.isZero()) {
    LLVM_DEBUG(dbgs() << __func__ << ": solution (root): " << X << '\n');
    ଫେରନ୍ତୁ X;
  }

  ପ୍ରତିଜ୍ଞା((SQ*SQ).sle(D) && "SQ = |_sqrt(D)_|, so SQ*SQ <= D");
  // The exact value of the square root of D should be between SQ and SQ+1.
  // This implies that the solution should be between that corresponding to
  // SQ (i.e. X) and that corresponding to SQ+1.
  //
  // The calculated X cannot be greater than the exact (real) solution.
  // Actually it must be strictly less than the exact solution, while
  // X+1 will be greater than or equal to it.

  APInt VX = (A*X + B)*X + C;
  APInt VY = VX + TwoA*X + A + B;
  ତାର୍କିକ SignChange =
      VX.isNegative() != VY.isNegative() || VX.isZero() != VY.isZero();
  // If the sign did not change between X and X+1, X is not a valid solution.
  // This could happen when the actual (exact) roots don't have an integer
  // between them, so they would both be contained between X and X+1.
  ଯଦି (!SignChange) {
    LLVM_DEBUG(dbgs() << __func__ << ": no valid solution\n");
    ଫେରନ୍ତୁ std::nullopt;
  }

  X += 1;
  LLVM_DEBUG(dbgs() << __func__ << ": solution (wrap): " << X << '\n');
  ଫେରନ୍ତୁ X;
}

std::optional<unsigned>
llvm::APIntOps::GetMostSignificantDifferentBit(ଅଚଳ APInt &A, ଅଚଳ APInt &B) {
  ପ୍ରତିଜ୍ଞା(A.getBitWidth() == B.getBitWidth() && "Must have the same bitwidth");
  ଯଦି (A == B)
    ଫେରନ୍ତୁ std::nullopt;
  ଫେରନ୍ତୁ A.getBitWidth() - ((A ^ B).countl_zero() + 1);
}

APInt llvm::APIntOps::ScaleBitMask(ଅଚଳ APInt &A, unsigned NewBitWidth,
                                   ତାର୍କିକ MatchAllBits) {
  unsigned OldBitWidth = A.getBitWidth();
  ପ୍ରତିଜ୍ଞା((((OldBitWidth % NewBitWidth) == 0) ||
          ((NewBitWidth % OldBitWidth) == 0)) &&
         "One size should be a multiple of the other one. "
         "Can't do fractional scaling.");

  // Check for matching bitwidths.
  ଯଦି (OldBitWidth == NewBitWidth)
    ଫେରନ୍ତୁ A;

  APInt NewA = APInt::getZero(NewBitWidth);

  // Check for null input.
  ଯଦି (A.isZero())
    ଫେରନ୍ତୁ NewA;

  ଯଦି (NewBitWidth > OldBitWidth) {
    // Repeat bits.
    unsigned Scale = NewBitWidth / OldBitWidth;
    ପାଇଁ (unsigned i = 0; i != OldBitWidth; ++i)
      ଯଦି (A[i])
        NewA.setBits(i * Scale, (i + 1) * Scale);
  } ଅନ୍ୟଥା {
    unsigned Scale = OldBitWidth / NewBitWidth;
    ପାଇଁ (unsigned i = 0; i != NewBitWidth; ++i) {
      ଯଦି (MatchAllBits) {
        ଯଦି (A.extractBits(Scale, i * Scale).isAllOnes())
          NewA.setBit(i);
      } ଅନ୍ୟଥା {
        ଯଦି (!A.extractBits(Scale, i * Scale).isZero())
          NewA.setBit(i);
      }
    }
  }

  ଫେରନ୍ତୁ NewA;
}

/// StoreIntToMemory - Fills the StoreBytes bytes of memory starting from Dst
/// with the integer held in IntVal.
ଶୂନ୍ୟ llvm::StoreIntToMemory(ଅଚଳ APInt &IntVal, uint8_t *Dst,
                            unsigned StoreBytes) {
  ପ୍ରତିଜ୍ଞା((IntVal.getBitWidth()+7)/8 >= StoreBytes && "Integer too small!");
  ଅଚଳ uint8_t *Src = (ଅଚଳ uint8_t *)IntVal.getRawData();

  ଯଦି (sys::IsLittleEndianHost) {
    // Little-endian host - the source is ordered from LSB to MSB.  Order the
    // destination from LSB to MSB: Do a straight copy.
    memcpy(Dst, Src, StoreBytes);
  } ଅନ୍ୟଥା {
    // Big-endian host - the source is an array of 64 bit words ordered from
    // LSW to MSW.  Each word is ordered from MSB to LSB.  Order the destination
    // from MSB to LSB: Reverse the word order, but not the bytes in a word.
    ଯେତେବେଳେ (StoreBytes > ଆକାର(uint64_t)) {
      StoreBytes -= ଆକାର(uint64_t);
      // May not be aligned so use memcpy.
      memcpy(Dst + StoreBytes, Src, ଆକାର(uint64_t));
      Src += ଆକାର(uint64_t);
    }

    memcpy(Dst, Src + ଆକାର(uint64_t) - StoreBytes, StoreBytes);
  }
}

/// LoadIntFromMemory - Loads the integer stored in the LoadBytes bytes starting
/// from Src into IntVal, which is assumed to be wide enough and to hold zero.
ଶୂନ୍ୟ llvm::LoadIntFromMemory(APInt &IntVal, ଅଚଳ uint8_t *Src,
                             unsigned LoadBytes) {
  ପ୍ରତିଜ୍ଞା((IntVal.getBitWidth()+7)/8 >= LoadBytes && "Integer too small!");
  uint8_t *Dst = reinterpret_cast<uint8_t *>(
                   const_cast<uint64_t *>(IntVal.getRawData()));

  ଯଦି (sys::IsLittleEndianHost)
    // Little-endian host - the destination must be ordered from LSB to MSB.
    // The source is ordered from LSB to MSB: Do a straight copy.
    memcpy(Dst, Src, LoadBytes);
  ଅନ୍ୟଥା {
    // Big-endian - the destination is an array of 64 bit words ordered from
    // LSW to MSW.  Each word must be ordered from MSB to LSB.  The source is
    // ordered from MSB to LSB: Reverse the word order, but not the bytes in
    // a word.
    ଯେତେବେଳେ (LoadBytes > ଆକାର(uint64_t)) {
      LoadBytes -= ଆକାର(uint64_t);
      // May not be aligned so use memcpy.
      memcpy(Dst, Src + LoadBytes, ଆକାର(uint64_t));
      Dst += ଆକାର(uint64_t);
    }

    memcpy(Dst + ଆକାର(uint64_t) - LoadBytes, Src, LoadBytes);
  }
}

APInt APIntOps::avgFloorS(ଅଚଳ APInt &C1, ଅଚଳ APInt &C2) {
  // Return floor((C1 + C2) / 2)
  ଫେରନ୍ତୁ (C1 & C2) + (C1 ^ C2).ashr(1);
}

APInt APIntOps::avgFloorU(ଅଚଳ APInt &C1, ଅଚଳ APInt &C2) {
  // Return floor((C1 + C2) / 2)
  ଫେରନ୍ତୁ (C1 & C2) + (C1 ^ C2).lshr(1);
}

APInt APIntOps::avgCeilS(ଅଚଳ APInt &C1, ଅଚଳ APInt &C2) {
  // Return ceil((C1 + C2) / 2)
  ଫେରନ୍ତୁ (C1 | C2) - (C1 ^ C2).ashr(1);
}

APInt APIntOps::avgCeilU(ଅଚଳ APInt &C1, ଅଚଳ APInt &C2) {
  // Return ceil((C1 + C2) / 2)
  ଫେରନ୍ତୁ (C1 | C2) - (C1 ^ C2).lshr(1);
}

APInt APIntOps::avgSubFloorS(ଅଚଳ APInt &C1, ଅଚଳ APInt &C2) {
  // Return floor((C1 - C2) / 2), using C1 - C2 == (C1 ^ C2) - 2 * (~C1 & C2).
  ଫେରନ୍ତୁ (C1 ^ C2).ashr(1) - (~C1 & C2);
}

APInt APIntOps::avgSubFloorU(ଅଚଳ APInt &C1, ଅଚଳ APInt &C2) {
  // Return floor((C1 - C2) / 2), using C1 - C2 == (C1 ^ C2) - 2 * (~C1 & C2).
  ଫେରନ୍ତୁ (C1 ^ C2).lshr(1) - (~C1 & C2);
}

APInt APIntOps::mulhs(ଅଚଳ APInt &C1, ଅଚଳ APInt &C2) {
  ପ୍ରତିଜ୍ଞା(C1.getBitWidth() == C2.getBitWidth() && "Unequal bitwidths");
  unsigned FullWidth = C1.getBitWidth() * 2;
  APInt C1Ext = C1.sext(FullWidth);
  APInt C2Ext = C2.sext(FullWidth);
  ଫେରନ୍ତୁ (C1Ext * C2Ext).extractBits(C1.getBitWidth(), C1.getBitWidth());
}

APInt APIntOps::mulhu(ଅଚଳ APInt &C1, ଅଚଳ APInt &C2) {
  ପ୍ରତିଜ୍ଞା(C1.getBitWidth() == C2.getBitWidth() && "Unequal bitwidths");
  unsigned FullWidth = C1.getBitWidth() * 2;
  APInt C1Ext = C1.zext(FullWidth);
  APInt C2Ext = C2.zext(FullWidth);
  ଫେରନ୍ତୁ (C1Ext * C2Ext).extractBits(C1.getBitWidth(), C1.getBitWidth());
}

APInt APIntOps::mulsExtended(ଅଚଳ APInt &C1, ଅଚଳ APInt &C2) {
  ପ୍ରତିଜ୍ଞା(C1.getBitWidth() == C2.getBitWidth() && "Unequal bitwidths");
  unsigned FullWidth = C1.getBitWidth() * 2;
  APInt C1Ext = C1.sext(FullWidth);
  APInt C2Ext = C2.sext(FullWidth);
  ଫେରନ୍ତୁ C1Ext * C2Ext;
}

APInt APIntOps::muluExtended(ଅଚଳ APInt &C1, ଅଚଳ APInt &C2) {
  ପ୍ରତିଜ୍ଞା(C1.getBitWidth() == C2.getBitWidth() && "Unequal bitwidths");
  unsigned FullWidth = C1.getBitWidth() * 2;
  APInt C1Ext = C1.zext(FullWidth);
  APInt C2Ext = C2.zext(FullWidth);
  ଫେରନ୍ତୁ C1Ext * C2Ext;
}

APInt APIntOps::pow(ଅଚଳ APInt &X, int64_t N) {
  ପ୍ରତିଜ୍ଞା(N >= 0 && "negative exponents not supported.");
  APInt Acc = APInt(X.getBitWidth(), 1);
  ଯଦି (N == 0)
    ଫେରନ୍ତୁ Acc;
  APInt Base = X;
  int64_t RemainingExponent = N;
  ଯେତେବେଳେ (RemainingExponent > 0) {
    ଯେତେବେଳେ (RemainingExponent % 2 == 0) {
      Base *= Base;
      RemainingExponent /= 2;
    }
    --RemainingExponent;
    Acc *= Base;
  }
  ଫେରନ୍ତୁ Acc;
}

APInt llvm::APIntOps::fshl(ଅଚଳ APInt &Hi, ଅଚଳ APInt &Lo,
                           ଅଚଳ APInt &Shift) {
  ପ୍ରତିଜ୍ଞା(Hi.getBitWidth() == Lo.getBitWidth());
  unsigned ShiftAmt = rotateModulo(Hi.getBitWidth(), Shift);
  ଯଦି (ShiftAmt == 0)
    ଫେରନ୍ତୁ Hi;
  ଫେରନ୍ତୁ Hi.shl(ShiftAmt) | Lo.lshr(Hi.getBitWidth() - ShiftAmt);
}

APInt llvm::APIntOps::fshr(ଅଚଳ APInt &Hi, ଅଚଳ APInt &Lo,
                           ଅଚଳ APInt &Shift) {
  ପ୍ରତିଜ୍ଞା(Hi.getBitWidth() == Lo.getBitWidth());
  unsigned ShiftAmt = rotateModulo(Hi.getBitWidth(), Shift);
  ଯଦି (ShiftAmt == 0)
    ଫେରନ୍ତୁ Lo;
  ଫେରନ୍ତୁ Hi.shl(Hi.getBitWidth() - ShiftAmt) | Lo.lshr(ShiftAmt);
}

APInt llvm::APIntOps::clmul(ଅଚଳ APInt &LHS, ଅଚଳ APInt &RHS) {
  unsigned BW = LHS.getBitWidth();
  ପ୍ରତିଜ୍ଞା(BW == RHS.getBitWidth() && "Operand mismatch");
  APInt Result(BW, 0);
  ପାଇଁ (unsigned I : seq(std::min(RHS.getActiveBits(), BW - LHS.countr_zero())))
    ଯଦି (RHS[I])
      Result ^= LHS << I;
  ଫେରନ୍ତୁ Result;
}

APInt llvm::APIntOps::clmulr(ଅଚଳ APInt &LHS, ଅଚଳ APInt &RHS) {
  ପ୍ରତିଜ୍ଞା(LHS.getBitWidth() == RHS.getBitWidth());
  ଫେରନ୍ତୁ clmul(LHS.reverseBits(), RHS.reverseBits()).reverseBits();
}

APInt llvm::APIntOps::clmulh(ଅଚଳ APInt &LHS, ଅଚଳ APInt &RHS) {
  ପ୍ରତିଜ୍ଞା(LHS.getBitWidth() == RHS.getBitWidth());
  ଫେରନ୍ତୁ clmulr(LHS, RHS).lshr(1);
}

APInt llvm::APIntOps::pext(ଅଚଳ APInt &Val, ଅଚଳ APInt &Mask) {
  unsigned BW = Val.getBitWidth();
  ପ୍ରତିଜ୍ଞା(BW == Mask.getBitWidth() && "Operand mismatch");
  APInt Result = APInt::getZero(BW);
  ପାଇଁ (unsigned I = 0, P = 0; I != BW; ++I)
    ଯଦି (Mask[I])
      Result.setBitVal(P++, Val[I]);
  ଫେରନ୍ତୁ Result;
}

APInt llvm::APIntOps::pdep(ଅଚଳ APInt &Val, ଅଚଳ APInt &Mask) {
  unsigned BW = Val.getBitWidth();
  ପ୍ରତିଜ୍ଞା(BW == Mask.getBitWidth() && "Operand mismatch");
  APInt Result = APInt::getZero(BW);
  ପାଇଁ (unsigned I = 0, P = 0; I != BW; ++I)
    ଯଦି (Mask[I])
      Result.setBitVal(I, Val[P++]);
  ଫେରନ୍ତୁ Result;
}
