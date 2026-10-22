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

using ನಾಮಾಕಾಶ llvm;

#define DEBUG_TYPE "apint"

/// A utility function for allocating memory, checking for allocation failures,
/// and ensuring the contents are zeroed.
ಅನ್ತಃಸ್ಥ ಸ್ಥಿರ uint64_t* getClearedMemory(unsigned ಅಂಕೆ_ಪದಗಳು) {
  ಪ್ರತ್ಯಾಗಮ ನೂತನ uint64_t[ಅಂಕೆ_ಪದಗಳು]();
}

/// A utility function for allocating memory and checking for allocation
/// failure.  The content is not zeroed.
ಅನ್ತಃಸ್ಥ ಸ್ಥಿರ uint64_t* getMemory(unsigned ಅಂಕೆ_ಪದಗಳು) {
  ಪ್ರತ್ಯಾಗಮ ನೂತನ uint64_t[ಅಂಕೆ_ಪದಗಳು];
}

/// A utility function that converts a character to a digit.
ಅನ್ತಃಸ್ಥ ಸ್ಥಿರ unsigned getDigit(ವರ್ಣ cdigit, uint8_t radix) {
  unsigned r;

  ಯದಿ (radix == 16 || radix == 36) {
    r = cdigit - '0';
    ಯದಿ (r <= 9)
      ಪ್ರತ್ಯಾಗಮ r;

    r = cdigit - 'A';
    ಯದಿ (r <= radix - 11U)
      ಪ್ರತ್ಯಾಗಮ r + 10;

    r = cdigit - 'a';
    ಯದಿ (r <= radix - 11U)
      ಪ್ರತ್ಯಾಗಮ r + 10;

    radix = 10;
  }

  r = cdigit - '0';
  ಯದಿ (r < radix)
    ಪ್ರತ್ಯಾಗಮ r;

  ಪ್ರತ್ಯಾಗಮ UINT_MAX;
}


ಶೂನ್ಯ APInt::initSlowCase(uint64_t val, ತಾರ್ಕಿಕ isSigned) {
  ಯದಿ (isSigned && int64_t(val) < 0) {
    U.pVal = getMemory(get_ಅಂಕೆ_ಪದಗಳು());
    U.pVal[0] = val;
    memset(&U.pVal[1], 0xFF, APINT_WORD_SIZE * (get_ಅಂಕೆ_ಪದಗಳು() - 1));
    clearUnusedBits();
  } ಅನ್ಯಥಾ {
    U.pVal = getClearedMemory(get_ಅಂಕೆ_ಪದಗಳು());
    U.pVal[0] = val;
  }
}

ಶೂನ್ಯ APInt::initSlowCase(ಧ್ರುವ APInt& that) {
  U.pVal = getMemory(get_ಅಂಕೆ_ಪದಗಳು());
  memcpy(U.pVal, that.U.pVal, get_ಅಂಕೆ_ಪದಗಳು() * APINT_WORD_SIZE);
}

ಶೂನ್ಯ APInt::initFromArray(ArrayRef<uint64_t> bigVal) {
  ಪ್ರತಿಜ್ಞಾ(bigVal.data() && "Null pointer detected!");
  ಯದಿ (LLVM_LIKELY(isSingleWord()))
    U.VAL = bigVal[0];
  ಅನ್ಯಥಾ {
    // Get memory, cleared to 0
    U.pVal = getClearedMemory(get_ಅಂಕೆ_ಪದಗಳು());
    // Calculate the number of words to copy
    unsigned ಪದಗಳು = std::min<unsigned>(bigVal.size(), get_ಅಂಕೆ_ಪದಗಳು());
    // Copy the words from bigVal to pVal
    memcpy(U.pVal, bigVal.data(), ಪದಗಳು * APINT_WORD_SIZE);
  }
  // Make sure unused high bits are cleared
  clearUnusedBits();
}

APInt::APInt(unsigned ಅಂಕೆ_Bits, ArrayRef<uint64_t> bigVal) : BitWidth(ಅಂಕೆ_Bits) {
  initFromArray(bigVal);
}

APInt::APInt(unsigned numbits, StringRef Str, uint8_t radix)
    : BitWidth(numbits) {
  fromString(numbits, Str, radix);
}

ಶೂನ್ಯ APInt::reallocate(unsigned NewBitWidth) {
  // If the number of words is the same we can just change the width and stop.
  ಯದಿ (get_ಅಂಕೆ_ಪದಗಳು() == get_ಅಂಕೆ_ಪದಗಳು(NewBitWidth)) {
    BitWidth = NewBitWidth;
    ಪ್ರತ್ಯಾಗಮ;
  }

  // If we have an allocation, delete it.
  ಯದಿ (LLVM_UNLIKELY(!isSingleWord()))
    delete [] U.pVal;

  // Update BitWidth.
  BitWidth = NewBitWidth;

  // If we are supposed to have an allocation, create it.
  ಯದಿ (LLVM_UNLIKELY(!isSingleWord()))
    U.pVal = getMemory(get_ಅಂಕೆ_ಪದಗಳು());
}

ಶೂನ್ಯ APInt::assignSlowCase(ಧ್ರುವ APInt &RHS) {
  // Don't do anything for X = X
  ಯದಿ (ಏತತ್ == &RHS)
    ಪ್ರತ್ಯಾಗಮ;

  // Adjust the bit width and handle allocations as necessary.
  reallocate(RHS.getBitWidth());

  // Copy the data.
  ಯದಿ (LLVM_LIKELY(isSingleWord()))
    U.VAL = RHS.U.VAL;
  ಅನ್ಯಥಾ
    memcpy(U.pVal, RHS.U.pVal, get_ಅಂಕೆ_ಪದಗಳು() * APINT_WORD_SIZE);
}

/// This method 'profiles' an APInt for use with FoldingSet.
ಶೂನ್ಯ APInt::Profile(Folding_Set_ಗಂಟು_ID& ID) ಧ್ರುವ {
  ID.AddInteger(BitWidth);

  ಯದಿ (LLVM_LIKELY(isSingleWord())) {
    ID.AddInteger(U.VAL);
    ಪ್ರತ್ಯಾಗಮ;
  }

  unsigned ಅಂಕೆ_ಪದಗಳು_2 = get_ಅಂಕೆ_ಪದಗಳು();
  ಪ್ರತಿ (unsigned i = 0; i < ಅಂಕೆ_ಪದಗಳು_2; ++i)
    ID.AddInteger(U.pVal[i]);
}

ತಾರ್ಕಿಕ APInt::isAligned(Align A) ಧ್ರುವ {
  ಯದಿ (isZero())
    ಪ್ರತ್ಯಾಗಮ ಸತ್ಯ;
  ಧ್ರುವ unsigned TrailingZeroes = countr_zero();
  ಧ್ರುವ unsigned MinimumTrailingZeroes = Log2(A);
  ಪ್ರತ್ಯಾಗಮ TrailingZeroes >= MinimumTrailingZeroes;
}

/// Prefix increment operator. Increments the APInt by one.
APInt& APInt::operator++() {
  ಯದಿ (LLVM_LIKELY(isSingleWord()))
    ++U.VAL;
  ಅನ್ಯಥಾ
    tcIncrement(U.pVal, get_ಅಂಕೆ_ಪದಗಳು());
  ಪ್ರತ್ಯಾಗಮ clearUnusedBits();
}

/// Prefix decrement operator. Decrements the APInt by one.
APInt& APInt::operator--() {
  ಯದಿ (LLVM_LIKELY(isSingleWord()))
    --U.VAL;
  ಅನ್ಯಥಾ
    tcDecrement(U.pVal, get_ಅಂಕೆ_ಪದಗಳು());
  ಪ್ರತ್ಯಾಗಮ clearUnusedBits();
}

/// Adds the RHS APInt to this APInt.
/// @returns this, after addition of RHS.
/// Addition assignment operator.
APInt& APInt::operator+=(ಧ್ರುವ APInt& RHS) {
  ಪ್ರತಿಜ್ಞಾ(BitWidth == RHS.BitWidth && "Bit widths must be the same");
  ಯದಿ (LLVM_LIKELY(isSingleWord()))
    U.VAL += RHS.U.VAL;
  ಅನ್ಯಥಾ
    tcAdd(U.pVal, RHS.U.pVal, 0, get_ಅಂಕೆ_ಪದಗಳು());
  ಪ್ರತ್ಯಾಗಮ clearUnusedBits();
}

APInt& APInt::operator+=(uint64_t RHS) {
  ಯದಿ (LLVM_LIKELY(isSingleWord()))
    U.VAL += RHS;
  ಅನ್ಯಥಾ
    tcAddPart(U.pVal, RHS, get_ಅಂಕೆ_ಪದಗಳು());
  ಪ್ರತ್ಯಾಗಮ clearUnusedBits();
}

/// Subtracts the RHS APInt from this APInt
/// @returns this, after subtraction
/// Subtraction assignment operator.
APInt& APInt::operator-=(ಧ್ರುವ APInt& RHS) {
  ಪ್ರತಿಜ್ಞಾ(BitWidth == RHS.BitWidth && "Bit widths must be the same");
  ಯದಿ (LLVM_LIKELY(isSingleWord()))
    U.VAL -= RHS.U.VAL;
  ಅನ್ಯಥಾ
    tcSubtract(U.pVal, RHS.U.pVal, 0, get_ಅಂಕೆ_ಪದಗಳು());
  ಪ್ರತ್ಯಾಗಮ clearUnusedBits();
}

APInt& APInt::operator-=(uint64_t RHS) {
  ಯದಿ (LLVM_LIKELY(isSingleWord()))
    U.VAL -= RHS;
  ಅನ್ಯಥಾ
    tcSubtractPart(U.pVal, RHS, get_ಅಂಕೆ_ಪದಗಳು());
  ಪ್ರತ್ಯಾಗಮ clearUnusedBits();
}

APInt APInt::operator*(ಧ್ರುವ APInt& RHS) ಧ್ರುವ {
  ಪ್ರತಿಜ್ಞಾ(BitWidth == RHS.BitWidth && "Bit widths must be the same");
  ಯದಿ (LLVM_LIKELY(isSingleWord()))
    ಪ್ರತ್ಯಾಗಮ APInt(BitWidth, U.VAL * RHS.U.VAL, /*isSigned=*/ಅಸತ್ಯ,
                 /*implicitTrunc=*/ಸತ್ಯ);

  APInt Result(getMemory(get_ಅಂಕೆ_ಪದಗಳು()), getBitWidth());
  tcMultiply(Result.U.pVal, U.pVal, RHS.U.pVal, get_ಅಂಕೆ_ಪದಗಳು());
  Result.clearUnusedBits();
  ಪ್ರತ್ಯಾಗಮ Result;
}

ಶೂನ್ಯ APInt::andAssignSlowCase(ಧ್ರುವ APInt &RHS) {
  WordType *dst = U.pVal, *rhs = RHS.U.pVal;
  ಪ್ರತಿ (size_t i = 0, e = get_ಅಂಕೆ_ಪದಗಳು(); i != e; ++i)
    dst[i] &= rhs[i];
}

ಶೂನ್ಯ APInt::orAssignSlowCase(ಧ್ರುವ APInt &RHS) {
  WordType *dst = U.pVal, *rhs = RHS.U.pVal;
  ಪ್ರತಿ (size_t i = 0, e = get_ಅಂಕೆ_ಪದಗಳು(); i != e; ++i)
    dst[i] |= rhs[i];
}

ಶೂನ್ಯ APInt::xorAssignSlowCase(ಧ್ರುವ APInt &RHS) {
  WordType *dst = U.pVal, *rhs = RHS.U.pVal;
  ಪ್ರತಿ (size_t i = 0, e = get_ಅಂಕೆ_ಪದಗಳು(); i != e; ++i)
    dst[i] ^= rhs[i];
}

APInt &APInt::operator*=(ಧ್ರುವ APInt &RHS) {
  *ಏತತ್ = *ಏತತ್ * RHS;
  ಪ್ರತ್ಯಾಗಮ *ಏತತ್;
}

APInt& APInt::operator*=(uint64_t RHS) {
  ಯದಿ (LLVM_LIKELY(isSingleWord())) {
    U.VAL *= RHS;
  } ಅನ್ಯಥಾ {
    unsigned ಅಂಕೆ_ಪದಗಳು_2 = get_ಅಂಕೆ_ಪದಗಳು();
    tcMultiplyPart(U.pVal, U.pVal, RHS, 0, ಅಂಕೆ_ಪದಗಳು_2, ಅಂಕೆ_ಪದಗಳು_2, ಅಸತ್ಯ);
  }
  ಪ್ರತ್ಯಾಗಮ clearUnusedBits();
}

ತಾರ್ಕಿಕ APInt::equalSlowCase(ಧ್ರುವ APInt &RHS) ಧ್ರುವ {
  ಪ್ರತ್ಯಾಗಮ std::equal(U.pVal, U.pVal + get_ಅಂಕೆ_ಪದಗಳು(), RHS.U.pVal);
}

ಪೂರ್ಣಾಙ್ಕ APInt::compare(ಧ್ರುವ APInt& RHS) ಧ್ರುವ {
  ಪ್ರತಿಜ್ಞಾ(BitWidth == RHS.BitWidth && "Bit widths must be same for comparison");
  ಯದಿ (LLVM_LIKELY(isSingleWord()))
    ಪ್ರತ್ಯಾಗಮ U.VAL < RHS.U.VAL ? -1 : U.VAL > RHS.U.VAL;

  ಪ್ರತ್ಯಾಗಮ tcCompare(U.pVal, RHS.U.pVal, get_ಅಂಕೆ_ಪದಗಳು());
}

ಪೂರ್ಣಾಙ್ಕ APInt::compareSigned(ಧ್ರುವ APInt& RHS) ಧ್ರುವ {
  ಪ್ರತಿಜ್ಞಾ(BitWidth == RHS.BitWidth && "Bit widths must be same for comparison");
  ಯದಿ (LLVM_LIKELY(isSingleWord())) {
    int64_t lhsSext = SignExtend64(U.VAL, BitWidth);
    int64_t rhsSext = SignExtend64(RHS.U.VAL, BitWidth);
    ಪ್ರತ್ಯಾಗಮ lhsSext < rhsSext ? -1 : lhsSext > rhsSext;
  }

  ತಾರ್ಕಿಕ lhsNeg = isNegative();
  ತಾರ್ಕಿಕ rhsNeg = RHS.isNegative();

  // If the sign bits don't match, then (LHS < RHS) if LHS is negative
  ಯದಿ (lhsNeg != rhsNeg)
    ಪ್ರತ್ಯಾಗಮ lhsNeg ? -1 : 1;

  // Otherwise we can just use an unsigned comparison, because even negative
  // numbers compare correctly this way if both have the same signed-ness.
  ಪ್ರತ್ಯಾಗಮ tcCompare(U.pVal, RHS.U.pVal, get_ಅಂಕೆ_ಪದಗಳು());
}

ಶೂನ್ಯ APInt::setBitsSlowCase(unsigned loBit, unsigned hiBit) {
  unsigned loWord = whichWord(loBit);
  unsigned hiWord = whichWord(hiBit);

  // Create an initial mask for the low word with zeros below loBit.
  uint64_t loMask = WORDTYPE_MAX << whichBit(loBit);

  // If hiBit is not aligned, we need a high mask.
  unsigned hiShiftAmt = whichBit(hiBit);
  ಯದಿ (hiShiftAmt != 0) {
    // Create a high mask with zeros above hiBit.
    uint64_t hiMask = WORDTYPE_MAX >> (APINT_BITS_PER_WORD - hiShiftAmt);
    // If loWord and hiWord are equal, then we combine the masks. Otherwise,
    // set the bits in hiWord.
    ಯದಿ (hiWord == loWord)
      loMask &= hiMask;
    ಅನ್ಯಥಾ
      U.pVal[hiWord] |= hiMask;
  }
  // Apply the mask to the low word.
  U.pVal[loWord] |= loMask;

  // Fill any words between loWord and hiWord with all ones.
  ಪ್ರತಿ (unsigned word = loWord + 1; word < hiWord; ++word)
    U.pVal[word] = WORDTYPE_MAX;
}

ಶೂನ್ಯ APInt::clearBitsSlowCase(unsigned LoBit, unsigned HiBit) {
  unsigned LoWord = whichWord(LoBit);
  unsigned HiWord = whichWord(HiBit);

  // Create an initial mask for the low word with ones below loBit.
  uint64_t LoMask = ~(WORDTYPE_MAX << whichBit(LoBit));

  // If HiBit is not aligned, we need a high mask.
  unsigned HiShiftAmt = whichBit(HiBit);
  ಯದಿ (HiShiftAmt != 0) {
    // Create a high mask with ones above HiBit.
    uint64_t HiMask = ~(WORDTYPE_MAX >> (APINT_BITS_PER_WORD - HiShiftAmt));
    // If LoWord and HiWord are equal, then we combine the masks. Otherwise,
    // clear the bits in HiWord.
    ಯದಿ (HiWord == LoWord)
      LoMask |= HiMask;
    ಅನ್ಯಥಾ
      U.pVal[HiWord] &= HiMask;
  }
  // Apply the mask to the low word.
  U.pVal[LoWord] &= LoMask;

  // Fill any words between LoWord and HiWord with all zeros.
  ಪ್ರತಿ (unsigned Word = LoWord + 1; Word < HiWord; ++Word)
    U.pVal[Word] = 0;
}

// Complement a bignum in-place.
ಸ್ಥಿರ ಶೂನ್ಯ tcComplement(APInt::WordType *dst, unsigned parts) {
  ಪ್ರತಿ (unsigned i = 0; i < parts; i++)
    dst[i] = ~dst[i];
}

/// Toggle every bit to its opposite value.
ಶೂನ್ಯ APInt::flipAllBitsSlowCase() {
  tcComplement(U.pVal, get_ಅಂಕೆ_ಪದಗಳು());
  clearUnusedBits();
}

/// Concatenate the bits from "NewLSB" onto the bottom of *this.  This is
/// equivalent to:
///   (this->zext(NewWidth) << NewLSB.getBitWidth()) | NewLSB.zext(NewWidth)
/// In the slow case, we know the result is large.
APInt APInt::concatSlowCase(ಧ್ರುವ APInt &NewLSB) ಧ್ರುವ {
  unsigned NewWidth = getBitWidth() + NewLSB.getBitWidth();
  APInt Result = NewLSB.zext(NewWidth);
  Result.ಸೇರಿಸು_Bits(*ಏತತ್, NewLSB.getBitWidth());
  ಪ್ರತ್ಯಾಗಮ Result;
}

/// Toggle a given bit to its opposite value whose position is given
/// as "bitPosition".
/// Toggles a given bit to its opposite value.
ಶೂನ್ಯ APInt::flipBit(unsigned bitPosition) {
  ಪ್ರತಿಜ್ಞಾ(bitPosition < BitWidth && "Out of the bit-width range!");
  setBitVal(bitPosition, !(*ಏತತ್)[bitPosition]);
}

ಶೂನ್ಯ APInt::ಸೇರಿಸು_Bits(ಧ್ರುವ APInt &subBits, unsigned bitPosition) {
  unsigned subBitWidth = subBits.getBitWidth();
  ಪ್ರತಿಜ್ಞಾ((subBitWidth + bitPosition) <= BitWidth && "Illegal bit insertion");

  // inserting no bits is a noop.
  ಯದಿ (subBitWidth == 0)
    ಪ್ರತ್ಯಾಗಮ;

  // Insertion is a direct copy.
  ಯದಿ (subBitWidth == BitWidth) {
    *ಏತತ್ = subBits;
    ಪ್ರತ್ಯಾಗಮ;
  }

  // Single word result can be done as a direct bitmask.
  ಯದಿ (LLVM_LIKELY(isSingleWord())) {
    uint64_t mask = WORDTYPE_MAX >> (APINT_BITS_PER_WORD - subBitWidth);
    U.VAL &= ~(mask << bitPosition);
    U.VAL |= (subBits.U.VAL << bitPosition);
    ಪ್ರತ್ಯಾಗಮ;
  }

  unsigned loBit = whichBit(bitPosition);
  unsigned loWord = whichWord(bitPosition);
  unsigned hi1Word = whichWord(bitPosition + subBitWidth - 1);

  // Insertion within a single word can be done as a direct bitmask.
  ಯದಿ (loWord == hi1Word) {
    uint64_t mask = WORDTYPE_MAX >> (APINT_BITS_PER_WORD - subBitWidth);
    U.pVal[loWord] &= ~(mask << loBit);
    U.pVal[loWord] |= (subBits.U.VAL << loBit);
    ಪ್ರತ್ಯಾಗಮ;
  }

  // Insert on word boundaries.
  ಯದಿ (loBit == 0) {
    // Direct copy whole words.
    unsigned ಅಂಕೆ_Whole_Sub_ಪದಗಳು = subBitWidth / APINT_BITS_PER_WORD;
    memcpy(U.pVal + loWord, subBits.getRawData(),
           ಅಂಕೆ_Whole_Sub_ಪದಗಳು * APINT_WORD_SIZE);

    // Mask+insert remaining bits.
    unsigned remainingBits = subBitWidth % APINT_BITS_PER_WORD;
    ಯದಿ (remainingBits != 0) {
      uint64_t mask = WORDTYPE_MAX >> (APINT_BITS_PER_WORD - remainingBits);
      U.pVal[hi1Word] &= ~mask;
      U.pVal[hi1Word] |= subBits.getWord(subBitWidth - 1);
    }
    ಪ್ರತ್ಯಾಗಮ;
  }

  // General case - set/clear individual bits in dst based on src.
  // TODO - there is scope for optimization here, but at the moment this code
  // path is barely used so prefer readability over performance.
  ಪ್ರತಿ (unsigned i = 0; i != subBitWidth; ++i)
    setBitVal(bitPosition + i, subBits[i]);
}

ಶೂನ್ಯ APInt::ಸೇರಿಸು_Bits(uint64_t subBits, unsigned bitPosition, unsigned ಅಂಕೆ_Bits) {
  uint64_t maskBits = maskTrailingOnes<uint64_t>(ಅಂಕೆ_Bits);
  subBits &= maskBits;
  ಯದಿ (LLVM_LIKELY(isSingleWord())) {
    U.VAL &= ~(maskBits << bitPosition);
    U.VAL |= subBits << bitPosition;
    ಪ್ರತ್ಯಾಗಮ;
  }

  unsigned loBit = whichBit(bitPosition);
  unsigned loWord = whichWord(bitPosition);
  unsigned hiWord = whichWord(bitPosition + ಅಂಕೆ_Bits - 1);
  ಯದಿ (loWord == hiWord) {
    U.pVal[loWord] &= ~(maskBits << loBit);
    U.pVal[loWord] |= subBits << loBit;
    ಪ್ರತ್ಯಾಗಮ;
  }

  static_assert(8 * ಪರಿಮಾಣ(WordType) <= 64, "This code assumes only two words affected");
  unsigned wordBits = 8 * ಪರಿಮಾಣ(WordType);
  U.pVal[loWord] &= ~(maskBits << loBit);
  U.pVal[loWord] |= subBits << loBit;

  U.pVal[hiWord] &= ~(maskBits >> (wordBits - loBit));
  U.pVal[hiWord] |= subBits >> (wordBits - loBit);
}

APInt APInt::extractBits(unsigned ಅಂಕೆ_Bits, unsigned bitPosition) ಧ್ರುವ {
  ಪ್ರತಿಜ್ಞಾ(bitPosition < BitWidth && (ಅಂಕೆ_Bits + bitPosition) <= BitWidth &&
         "Illegal bit extraction");

  ಯದಿ (LLVM_LIKELY(isSingleWord()))
    ಪ್ರತ್ಯಾಗಮ APInt(ಅಂಕೆ_Bits, U.VAL >> bitPosition, /*isSigned=*/ಅಸತ್ಯ,
                 /*implicitTrunc=*/ಸತ್ಯ);

  unsigned loBit = whichBit(bitPosition);
  unsigned loWord = whichWord(bitPosition);
  unsigned hiWord = whichWord(bitPosition + ಅಂಕೆ_Bits - 1);

  // Single word result extracting bits from a single word source.
  ಯದಿ (loWord == hiWord)
    ಪ್ರತ್ಯಾಗಮ APInt(ಅಂಕೆ_Bits, U.pVal[loWord] >> loBit, /*isSigned=*/ಅಸತ್ಯ,
                 /*implicitTrunc=*/ಸತ್ಯ);

  // Extracting bits that start on a source word boundary can be done
  // as a fast memory copy.
  ಯದಿ (loBit == 0)
    ಪ್ರತ್ಯಾಗಮ APInt(ಅಂಕೆ_Bits, ArrayRef(U.pVal + loWord, 1 + hiWord - loWord));

  // General case - shift + copy source words directly into place.
  APInt Result(ಅಂಕೆ_Bits, 0);
  unsigned ಅಂಕೆ_Src_ಪದಗಳು = get_ಅಂಕೆ_ಪದಗಳು();
  unsigned ಅಂಕೆ_Dst_ಪದಗಳು = Result.get_ಅಂಕೆ_ಪದಗಳು();

  uint64_t *DestPtr =
      LLVM_LIKELY(Result.isSingleWord()) ? &Result.U.VAL : Result.U.pVal;
  ಪ್ರತಿ (unsigned word = 0; word < ಅಂಕೆ_Dst_ಪದಗಳು; ++word) {
    uint64_t w0 = U.pVal[loWord + word];
    uint64_t w1 =
        (loWord + word + 1) < ಅಂಕೆ_Src_ಪದಗಳು ? U.pVal[loWord + word + 1] : 0;
    DestPtr[word] = (w0 >> loBit) | (w1 << (APINT_BITS_PER_WORD - loBit));
  }

  ಪ್ರತ್ಯಾಗಮ Result.clearUnusedBits();
}

uint64_t APInt::extract_Bits_As_ZExt_ಮೌಲ್ಯ(unsigned ಅಂಕೆ_Bits,
                                       unsigned bitPosition) ಧ್ರುವ {
  ಪ್ರತಿಜ್ಞಾ(bitPosition < BitWidth && (ಅಂಕೆ_Bits + bitPosition) <= BitWidth &&
         "Illegal bit extraction");
  ಪ್ರತಿಜ್ಞಾ(ಅಂಕೆ_Bits <= 64 && "Illegal bit extraction");

  uint64_t maskBits = maskTrailingOnes<uint64_t>(ಅಂಕೆ_Bits);
  ಯದಿ (LLVM_LIKELY(isSingleWord()))
    ಪ್ರತ್ಯಾಗಮ (U.VAL >> bitPosition) & maskBits;

  static_assert(APINT_BITS_PER_WORD >= 64,
                "This code assumes only two words affected");
  unsigned loBit = whichBit(bitPosition);
  unsigned loWord = whichWord(bitPosition);
  unsigned hiWord = whichWord(bitPosition + ಅಂಕೆ_Bits - 1);
  ಯದಿ (loWord == hiWord)
    ಪ್ರತ್ಯಾಗಮ (U.pVal[loWord] >> loBit) & maskBits;

  uint64_t retBits = U.pVal[loWord] >> loBit;
  retBits |= U.pVal[hiWord] << (APINT_BITS_PER_WORD - loBit);
  retBits &= maskBits;
  ಪ್ರತ್ಯಾಗಮ retBits;
}

unsigned APInt::getSufficientBitsNeeded(StringRef Str, uint8_t Radix) {
  ಪ್ರತಿಜ್ಞಾ(!Str.empty() && "Invalid string length");
  size_t StrLen = Str.size();

  // Each computation below needs to know if it's negative.
  unsigned IsNegative = ಅಸತ್ಯ;
  ಯದಿ (Str[0] == '-' || Str[0] == '+') {
    IsNegative = Str[0] == '-';
    StrLen--;
    ಪ್ರತಿಜ್ಞಾ(StrLen && "String is only a sign, needs a value.");
  }

  // For radixes of power-of-two values, the bits required is accurately and
  // easily computed.
  ಯದಿ (Radix == 2)
    ಪ್ರತ್ಯಾಗಮ StrLen + IsNegative;
  ಯದಿ (Radix == 8)
    ಪ್ರತ್ಯಾಗಮ StrLen * 3 + IsNegative;
  ಯದಿ (Radix == 16)
    ಪ್ರತ್ಯಾಗಮ StrLen * 4 + IsNegative;

  // Compute a sufficient number of bits that is always large enough but might
  // be too large. This avoids the assertion in the constructor. This
  // calculation doesn't work appropriately for the numbers 0-9, so just use 4
  // bits in that case.
  ಯದಿ (Radix == 10)
    ಪ್ರತ್ಯಾಗಮ (StrLen == 1 ? 4 : StrLen * 64 / 18) + IsNegative;

  ಪ್ರತಿಜ್ಞಾ(Radix == 36);
  ಪ್ರತ್ಯಾಗಮ (StrLen == 1 ? 7 : StrLen * 16 / 3) + IsNegative;
}

unsigned APInt::getBitsNeeded(StringRef str, uint8_t radix) {
  // Compute a sufficient number of bits that is always large enough but might
  // be too large.
  unsigned sufficient = getSufficientBitsNeeded(str, radix);

  // For bases 2, 8, and 16, the sufficient number of bits is exact and we can
  // return the value directly. For bases 10 and 36, we need to do extra work.
  ಯದಿ (radix == 2 || radix == 8 || radix == 16)
    ಪ್ರತ್ಯಾಗಮ sufficient;

  // This is grossly inefficient but accurate. We could probably do something
  // with a computation of roughly slen*64/20 and then adjust by the value of
  // the first few digits. But, I'm not sure how accurate that could be.
  size_t slen = str.size();

  // Each computation below needs to know if it's negative.
  StringRef::iterator p = str.begin();
  unsigned isNegative = *p == '-';
  ಯದಿ (*p == '-' || *p == '+') {
    p++;
    slen--;
    ಪ್ರತಿಜ್ಞಾ(slen && "String is only a sign, needs a value.");
  }


  // Convert to the actual binary value.
  APInt tmp(sufficient, StringRef(p, slen), radix);

  // Compute how many bits are required. If the log is infinite, assume we need
  // just bit. If the log is exact and value is negative, then the value is
  // MinSignedValue with (log + 1) bits.
  unsigned log = tmp.logBase2();
  ಯದಿ (log == (unsigned)-1) {
    ಪ್ರತ್ಯಾಗಮ isNegative + 1;
  } ಅನ್ಯಥಾ ಯದಿ (isNegative && tmp.isPowerOf2()) {
    ಪ್ರತ್ಯಾಗಮ isNegative + log;
  } ಅನ್ಯಥಾ {
    ಪ್ರತ್ಯಾಗಮ isNegative + log + 1;
  }
}

hash_code llvm::hash_ಮೌಲ್ಯ(ಧ್ರುವ APInt &Arg) {
  ಯದಿ (LLVM_LIKELY(Arg.isSingleWord()))
    ಪ್ರತ್ಯಾಗಮ hash_combine(Arg.BitWidth, Arg.U.VAL);

  ಪ್ರತ್ಯಾಗಮ hash_combine(
      Arg.BitWidth,
      hash_combine_range(Arg.U.pVal, Arg.U.pVal + Arg.get_ಅಂಕೆ_ಪದಗಳು()));
}

unsigned DenseMapInfo<APInt, ಶೂನ್ಯ>::get_Hash_ಮೌಲ್ಯ(ಧ್ರುವ APInt &ಕೀಲಿ) {
  ಪ್ರತ್ಯಾಗಮ static_cast<unsigned>(hash_ಮೌಲ್ಯ(ಕೀಲಿ));
}

ತಾರ್ಕಿಕ APInt::isSplat(unsigned SplatSizeInBits) ಧ್ರುವ {
  ಪ್ರತಿಜ್ಞಾ(getBitWidth() % SplatSizeInBits == 0 &&
         "SplatSizeInBits must divide width!");
  // We can check that all parts of an integer are equal by making use of a
  // little trick: rotate and check if it's still the same value.
  ಪ್ರತ್ಯಾಗಮ *ಏತತ್ == rotl(SplatSizeInBits);
}

/// This function returns the high "numBits" bits of this APInt.
APInt APInt::getHiBits(unsigned ಅಂಕೆ_Bits) ಧ್ರುವ {
  ಪ್ರತ್ಯಾಗಮ ಏತತ್->lshr(BitWidth - ಅಂಕೆ_Bits);
}

/// This function returns the low "numBits" bits of this APInt.
APInt APInt::getLoBits(unsigned ಅಂಕೆ_Bits) ಧ್ರುವ {
  APInt Result(get_ಕೆಳಗಿನ_Bits_Set(BitWidth, ಅಂಕೆ_Bits));
  Result &= *ಏತತ್;
  ಪ್ರತ್ಯಾಗಮ Result;
}

/// Return a value containing V broadcasted over NewLen bits.
APInt APInt::getSplat(unsigned NewLen, ಧ್ರುವ APInt &V) {
  ಪ್ರತಿಜ್ಞಾ(NewLen >= V.getBitWidth() && "Can't splat to smaller bit width!");

  APInt Val = V.zext(NewLen);
  ಪ್ರತಿ (unsigned I = V.getBitWidth(); I < NewLen; I <<= 1)
    Val |= Val << I;

  ಪ್ರತ್ಯಾಗಮ Val;
}

unsigned APInt::ಎಣಿಕೆ_Leading_Zeros_Slow_Case() ಧ್ರುವ {
  unsigned ಎಣಿಕೆ_2 = 0;
  ಪ್ರತಿ (ಪೂರ್ಣಾಙ್ಕ i = get_ಅಂಕೆ_ಪದಗಳು() - 1; i >= 0; --i) {
    uint64_t V = U.pVal[i];
    ಯದಿ (V == 0)
      ಎಣಿಕೆ_2 += APINT_BITS_PER_WORD;
    ಅನ್ಯಥಾ {
      ಎಣಿಕೆ_2 += llvm::countl_zero(V);
      ಭಙ್ಗ;
    }
  }
  // Adjust for unused bits in the most significant word (they are zero).
  unsigned Mod = BitWidth % APINT_BITS_PER_WORD;
  ಎಣಿಕೆ_2 -= Mod > 0 ? APINT_BITS_PER_WORD - Mod : 0;
  ಪ್ರತ್ಯಾಗಮ ಎಣಿಕೆ_2;
}

unsigned APInt::ಎಣಿಕೆ_Leading_Ones_Slow_Case() ಧ್ರುವ {
  unsigned ಮೇಲಿನ_Word_Bits = BitWidth % APINT_BITS_PER_WORD;
  unsigned shift;
  ಯದಿ (!ಮೇಲಿನ_Word_Bits) {
    ಮೇಲಿನ_Word_Bits = APINT_BITS_PER_WORD;
    shift = 0;
  } ಅನ್ಯಥಾ {
    shift = APINT_BITS_PER_WORD - ಮೇಲಿನ_Word_Bits;
  }
  ಪೂರ್ಣಾಙ್ಕ i = get_ಅಂಕೆ_ಪದಗಳು() - 1;
  unsigned ಎಣಿಕೆ_2 = llvm::countl_one(U.pVal[i] << shift);
  ಯದಿ (ಎಣಿಕೆ_2 == ಮೇಲಿನ_Word_Bits) {
    ಪ್ರತಿ (i--; i >= 0; --i) {
      ಯದಿ (U.pVal[i] == WORDTYPE_MAX)
        ಎಣಿಕೆ_2 += APINT_BITS_PER_WORD;
      ಅನ್ಯಥಾ {
        ಎಣಿಕೆ_2 += llvm::countl_one(U.pVal[i]);
        ಭಙ್ಗ;
      }
    }
  }
  ಪ್ರತ್ಯಾಗಮ ಎಣಿಕೆ_2;
}

unsigned APInt::ಎಣಿಕೆ_Trailing_Zeros_Slow_Case() ಧ್ರುವ {
  unsigned ಎಣಿಕೆ_2 = 0;
  unsigned i = 0;
  ಪ್ರತಿ (; i < get_ಅಂಕೆ_ಪದಗಳು() && U.pVal[i] == 0; ++i)
    ಎಣಿಕೆ_2 += APINT_BITS_PER_WORD;
  ಯದಿ (i < get_ಅಂಕೆ_ಪದಗಳು())
    ಎಣಿಕೆ_2 += llvm::countr_zero(U.pVal[i]);
  ಪ್ರತ್ಯಾಗಮ std::min(ಎಣಿಕೆ_2, BitWidth);
}

unsigned APInt::ಎಣಿಕೆ_Trailing_Ones_Slow_Case() ಧ್ರುವ {
  unsigned ಎಣಿಕೆ_2 = 0;
  unsigned i = 0;
  ಪ್ರತಿ (; i < get_ಅಂಕೆ_ಪದಗಳು() && U.pVal[i] == WORDTYPE_MAX; ++i)
    ಎಣಿಕೆ_2 += APINT_BITS_PER_WORD;
  ಯದಿ (i < get_ಅಂಕೆ_ಪದಗಳು())
    ಎಣಿಕೆ_2 += llvm::countr_one(U.pVal[i]);
  ಪ್ರತಿಜ್ಞಾ(ಎಣಿಕೆ_2 <= BitWidth);
  ಪ್ರತ್ಯಾಗಮ ಎಣಿಕೆ_2;
}

unsigned APInt::ಎಣಿಕೆ_Population_Slow_Case() ಧ್ರುವ {
  unsigned ಎಣಿಕೆ_2 = 0;
  ಪ್ರತಿ (unsigned i = 0; i < get_ಅಂಕೆ_ಪದಗಳು(); ++i)
    ಎಣಿಕೆ_2 += llvm::popcount(U.pVal[i]);
  ಪ್ರತ್ಯಾಗಮ ಎಣಿಕೆ_2;
}

ತಾರ್ಕಿಕ APInt::isPowerOf2SlowCase() ಧ್ರುವ {
  unsigned ಎಣಿಕೆ_2 = 0;
  ಪ್ರತಿ (unsigned i = 0; i < get_ಅಂಕೆ_ಪದಗಳು(); ++i) {
    ಎಣಿಕೆ_2 += llvm::popcount(U.pVal[i]);
    ಯದಿ (ಎಣಿಕೆ_2 > 1)
      ಪ್ರತ್ಯಾಗಮ ಅಸತ್ಯ;
  }
  ಪ್ರತ್ಯಾಗಮ ಎಣಿಕೆ_2 == 1;
}

ತಾರ್ಕಿಕ APInt::intersectsSlowCase(ಧ್ರುವ APInt &RHS) ಧ್ರುವ {
  ಪ್ರತಿ (unsigned i = 0, e = get_ಅಂಕೆ_ಪದಗಳು(); i != e; ++i)
    ಯದಿ ((U.pVal[i] & RHS.U.pVal[i]) != 0)
      ಪ್ರತ್ಯಾಗಮ ಸತ್ಯ;

  ಪ್ರತ್ಯಾಗಮ ಅಸತ್ಯ;
}

ತಾರ್ಕಿಕ APInt::isSubsetOfSlowCase(ಧ್ರುವ APInt &RHS) ಧ್ರುವ {
  ಪ್ರತಿ (unsigned i = 0, e = get_ಅಂಕೆ_ಪದಗಳು(); i != e; ++i)
    ಯದಿ ((U.pVal[i] & ~RHS.U.pVal[i]) != 0)
      ಪ್ರತ್ಯಾಗಮ ಅಸತ್ಯ;

  ಪ್ರತ್ಯಾಗಮ ಸತ್ಯ;
}

ತಾರ್ಕಿಕ APInt::isInverseOfSlowCase(ಧ್ರುವ APInt &RHS) ಧ್ರುವ {
  ಧ್ರುವ unsigned Last = get_ಅಂಕೆ_ಪದಗಳು() - 1;
  ಪ್ರತಿ (unsigned I = 0; I != Last; ++I)
    ಯದಿ ((U.pVal[I] ^ RHS.U.pVal[I]) != WORDTYPE_MAX)
      ಪ್ರತ್ಯಾಗಮ ಅಸತ್ಯ;

  unsigned TailBits = BitWidth - Last * APINT_BITS_PER_WORD;
  WordType TailMask = llvm::maskTrailingOnes<WordType>(TailBits);
  ಪ್ರತ್ಯಾಗಮ (U.pVal[Last] ^ RHS.U.pVal[Last]) == TailMask;
}

APInt APInt::byteSwap() ಧ್ರುವ {
  ಪ್ರತಿಜ್ಞಾ(BitWidth >= 16 && BitWidth % 8 == 0 && "Cannot byteswap!");
  ಯದಿ (BitWidth == 16)
    ಪ್ರತ್ಯಾಗಮ APInt(BitWidth, llvm::byteswap<uint16_t>(U.VAL));
  ಯದಿ (BitWidth == 32)
    ಪ್ರತ್ಯಾಗಮ APInt(BitWidth, llvm::byteswap<uint32_t>(U.VAL));
  ಯದಿ (BitWidth <= 64) {
    uint64_t Tmp1 = llvm::byteswap<uint64_t>(U.VAL);
    Tmp1 >>= (64 - BitWidth);
    ಪ್ರತ್ಯಾಗಮ APInt(BitWidth, Tmp1);
  }

  APInt Result(get_ಅಂಕೆ_ಪದಗಳು() * APINT_BITS_PER_WORD, 0);
  ಪ್ರತಿ (unsigned I = 0, N = get_ಅಂಕೆ_ಪದಗಳು(); I != N; ++I)
    Result.U.pVal[I] = llvm::byteswap<uint64_t>(U.pVal[N - I - 1]);
  ಯದಿ (Result.BitWidth != BitWidth) {
    Result.lshrInPlace(Result.BitWidth - BitWidth);
    Result.BitWidth = BitWidth;
  }
  ಪ್ರತ್ಯಾಗಮ Result;
}

APInt APInt::reverseBits() ಧ್ರುವ {
  ಯದಿ (LLVM_LIKELY(isSingleWord())) {
    ವಿಕಲ್ಪ (BitWidth) {
    ಪಕ್ಷ 64:
      ಪ್ರತ್ಯಾಗಮ APInt(BitWidth, llvm::reverseBits<uint64_t>(U.VAL));
    ಪಕ್ಷ 32:
      ಪ್ರತ್ಯಾಗಮ APInt(BitWidth, llvm::reverseBits<uint32_t>(U.VAL));
    ಪಕ್ಷ 16:
      ಪ್ರತ್ಯಾಗಮ APInt(BitWidth, llvm::reverseBits<uint16_t>(U.VAL));
    ಪಕ್ಷ 8:
      ಪ್ರತ್ಯಾಗಮ APInt(BitWidth, llvm::reverseBits<uint8_t>(U.VAL));
    ಪಕ್ಷ 1: // fallthrough
    ಪಕ್ಷ 0:
      ಪ್ರತ್ಯಾಗಮ *ಏತತ್;
    ಸ್ವಭಾವ:
      ಪ್ರತ್ಯಾಗಮ APInt(BitWidth,
                   llvm::reverseBits<uint64_t>(U.VAL) >> (64 - BitWidth));
    }
  }

  APInt Result(BitWidth, 0);
  unsigned ಅಂಕೆ_ಪದಗಳು_2 = get_ಅಂಕೆ_ಪದಗಳು();
  unsigned ExcessBits = ಅಂಕೆ_ಪದಗಳು_2 * APINT_BITS_PER_WORD - BitWidth;
  ಯದಿ (ExcessBits == 0) {
    // Fast path. No cross-word shift needed.
    ಪ್ರತಿ (unsigned I = 0; I < ಅಂಕೆ_ಪದಗಳು_2; ++I)
      Result.U.pVal[I] = llvm::reverseBits<uint64_t>(U.pVal[ಅಂಕೆ_ಪದಗಳು_2 - 1 - I]);
    ಪ್ರತ್ಯಾಗಮ Result;
  }
  // Holds reversed bits of the previous (more significant) word.
  uint64_t PrevRev = llvm::reverseBits<uint64_t>(U.pVal[ಅಂಕೆ_ಪದಗಳು_2 - 1]);
  ಪ್ರತಿ (unsigned I = 0; I < ಅಂಕೆ_ಪದಗಳು_2 - 1; ++I) {
    uint64_t CurrRev = llvm::reverseBits<uint64_t>(U.pVal[ಅಂಕೆ_ಪದಗಳು_2 - 2 - I]);
    Result.U.pVal[I] = (PrevRev >> ExcessBits) | (CurrRev << (64 - ExcessBits));
    PrevRev = CurrRev;
  }
  Result.U.pVal[ಅಂಕೆ_ಪದಗಳು_2 - 1] = PrevRev >> ExcessBits;
  ಪ್ರತ್ಯಾಗಮ Result;
}

APInt llvm::APIntOps::Greatest_Common_ಭಾಜಕ(APInt A, APInt B, ತಾರ್ಕಿಕ IsSigned) {
  // Take absolute value if IsSigned.
  ಯದಿ (IsSigned) {
    A = A.abs();
    B = B.abs();
  }

  // Fast-path a common case.
  ಯದಿ (A == B) ಪ್ರತ್ಯಾಗಮ A;

  // Corner cases: if either operand is zero, the other is the gcd.
  ಯದಿ (!A) ಪ್ರತ್ಯಾಗಮ B;
  ಯದಿ (!B) ಪ್ರತ್ಯಾಗಮ A;

  // Count common powers of 2 and remove all other powers of 2.
  unsigned Pow2;
  {
    unsigned Pow2_A = A.countr_zero();
    unsigned Pow2_B = B.countr_zero();
    ಯದಿ (Pow2_A > Pow2_B) {
      A.lshrInPlace(Pow2_A - Pow2_B);
      Pow2 = Pow2_B;
    } ಅನ್ಯಥಾ ಯದಿ (Pow2_B > Pow2_A) {
      B.lshrInPlace(Pow2_B - Pow2_A);
      Pow2 = Pow2_A;
    } ಅನ್ಯಥಾ {
      Pow2 = Pow2_A;
    }
  }

  // Both operands are odd multiples of 2^Pow_2:
  //
  //   gcd(a, b) = gcd(|a - b| / 2^i, min(a, b))
  //
  // This is a modified version of Stein's algorithm, taking advantage of
  // efficient countTrailingZeros().
  ಯಾವತ್ (A != B) {
    ಯದಿ (A.ugt(B)) {
      A -= B;
      A.lshrInPlace(A.countr_zero() - Pow2);
    } ಅನ್ಯಥಾ {
      B -= A;
      B.lshrInPlace(B.countr_zero() - Pow2);
    }
  }

  ಪ್ರತ್ಯಾಗಮ A;
}

APInt llvm::APIntOps::RoundDoubleToAPInt(ದ್ವಿಗುಣ Double, unsigned width) {
  uint64_t I = bit_cast<uint64_t>(Double);

  // Get the sign bit from the highest order bit
  ತಾರ್ಕಿಕ isNeg = I >> 63;

  // Get the 11-bit exponent and adjust for the 1023 bit bias
  int64_t exp = ((I >> 52) & 0x7ff) - 1023;

  // If the exponent is negative, the value is < 0 so just return 0.
  ಯದಿ (exp < 0)
    ಪ್ರತ್ಯಾಗಮ APInt(width, 0u);

  // Extract the mantissa by clearing the top 12 bits (sign + exponent).
  uint64_t mantissa = (I & (~0ULL >> 12)) | 1ULL << 52;

  // If the exponent doesn't shift all bits out of the mantissa
  ಯದಿ (exp < 52)
    ಪ್ರತ್ಯಾಗಮ isNeg ? -APInt(width, mantissa >> (52 - exp)) :
                    APInt(width, mantissa >> (52 - exp));

  // If the client didn't provide enough bits for us to shift the mantissa into
  // then the result is undefined, just return 0
  ಯದಿ (width <= exp - 52)
    ಪ್ರತ್ಯಾಗಮ APInt(width, 0);

  // Otherwise, we have to shift the mantissa bits up to the right location
  APInt Tmp(width, mantissa);
  Tmp <<= (unsigned)exp - 52;
  ಪ್ರತ್ಯಾಗಮ isNeg ? -Tmp : Tmp;
}

/// This function converts this APInt to a double.
/// The layout for double is as following (IEEE Standard 754):
///  --------------------------------------
/// |  Sign    Exponent    Fraction    Bias |
/// |-------------------------------------- |
/// |  1[63]   11[62-52]   52[51-00]   1023 |
///  --------------------------------------
ದ್ವಿಗುಣ APInt::roundToDouble(ತಾರ್ಕಿಕ isSigned) ಧ್ರುವ {
  // Handle the simple case where the value is contained in one uint64_t.
  // It is wrong to optimize getWord(0) to VAL; there might be more than one word.
  ಯದಿ (LLVM_LIKELY(isSingleWord() || getActiveBits() <= APINT_BITS_PER_WORD)) {
    ಯದಿ (isSigned) {
      int64_t sext = SignExtend64(getWord(0), BitWidth);
      ಪ್ರತ್ಯಾಗಮ ದ್ವಿಗುಣ(sext);
    }
    ಪ್ರತ್ಯಾಗಮ ದ್ವಿಗುಣ(getWord(0));
  }

  // Determine if the value is negative.
  ತಾರ್ಕಿಕ isNeg = isSigned ? (*ಏತತ್)[BitWidth-1] : ಅಸತ್ಯ;

  // Construct the absolute value if we're negative.
  APInt Tmp(isNeg ? -(*ಏತತ್) : (*ಏತತ್));

  // Figure out how many bits we're using.
  unsigned n = Tmp.getActiveBits();

  // The exponent (without bias normalization) is just the number of bits
  // we are using. Note that the sign bit is gone since we constructed the
  // absolute value.
  uint64_t exp = n;

  // Return infinity for exponent overflow
  ಯದಿ (exp > 1023) {
    ಯದಿ (!isSigned || !isNeg)
      ಪ್ರತ್ಯಾಗಮ std::numeric_limits<ದ್ವಿಗುಣ>::infinity();
    ಅನ್ಯಥಾ
      ಪ್ರತ್ಯಾಗಮ -std::numeric_limits<ದ್ವಿಗುಣ>::infinity();
  }
  exp += 1023; // Increment for 1023 bias

  // Number of bits in mantissa is 52. To obtain the mantissa value, we must
  // extract the high 52 bits from the correct words in pVal.
  uint64_t mantissa;
  unsigned hiWord = whichWord(n-1);
  ಯದಿ (hiWord == 0) {
    mantissa = Tmp.U.pVal[0];
    ಯದಿ (n > 52)
      mantissa >>= n - 52; // shift down, we want the top 52 bits.
  } ಅನ್ಯಥಾ {
    ಪ್ರತಿಜ್ಞಾ(hiWord > 0 && "huh?");
    uint64_t hibits = Tmp.U.pVal[hiWord] << (52 - n % APINT_BITS_PER_WORD);
    uint64_t lobits = Tmp.U.pVal[hiWord-1] >> (11 + n % APINT_BITS_PER_WORD);
    mantissa = hibits | lobits;
  }

  // The leading bit of mantissa is implicit, so get rid of it.
  uint64_t sign = isNeg ? (1ULL << (APINT_BITS_PER_WORD - 1)) : 0;
  uint64_t I = sign | (exp << 52) | mantissa;
  ಪ್ರತ್ಯಾಗಮ bit_cast<ದ್ವಿಗುಣ>(I);
}

// Truncate to new width.
APInt APInt::trunc(unsigned width) ಧ್ರುವ {
  ಪ್ರತಿಜ್ಞಾ(width <= BitWidth && "Invalid APInt Truncate request");

  ಯದಿ (width <= APINT_BITS_PER_WORD)
    ಪ್ರತ್ಯಾಗಮ APInt(width, getRawData()[0], /*isSigned=*/ಅಸತ್ಯ,
                 /*implicitTrunc=*/ಸತ್ಯ);

  ಯದಿ (width == BitWidth)
    ಪ್ರತ್ಯಾಗಮ *ಏತತ್;

  APInt Result(getMemory(get_ಅಂಕೆ_ಪದಗಳು(width)), width);

  // Copy full words.
  unsigned i;
  ಪ್ರತಿ (i = 0; i != width / APINT_BITS_PER_WORD; i++)
    Result.U.pVal[i] = U.pVal[i];

  // Truncate and copy any partial word.
  unsigned bits = (0 - width) % APINT_BITS_PER_WORD;
  ಯದಿ (bits != 0)
    Result.U.pVal[i] = U.pVal[i] << bits >> bits;

  ಪ್ರತ್ಯಾಗಮ Result;
}

// Truncate to new width with unsigned saturation.
APInt APInt::truncUSat(unsigned width) ಧ್ರುವ {
  ಪ್ರತಿಜ್ಞಾ(width <= BitWidth && "Invalid APInt Truncate request");

  // Can we just losslessly truncate it?
  ಯದಿ (isIntN(width))
    ಪ್ರತ್ಯಾಗಮ trunc(width);
  // If not, then just return the new limit.
  ಪ್ರತ್ಯಾಗಮ APInt::get_Max_ಮೌಲ್ಯ(width);
}

// Truncate to new width with signed saturation to signed result.
APInt APInt::truncSSat(unsigned width) ಧ್ರುವ {
  ಪ್ರತಿಜ್ಞಾ(width <= BitWidth && "Invalid APInt Truncate request");

  // Can we just losslessly truncate it?
  ಯದಿ (isSignedIntN(width))
    ಪ್ರತ್ಯಾಗಮ trunc(width);
  // If not, then just return the new limits.
  ಪ್ರತ್ಯಾಗಮ isNegative() ? APInt::get_Signed_Min_ಮೌಲ್ಯ(width)
                      : APInt::get_Signed_Max_ಮೌಲ್ಯ(width);
}

// Truncate to new width with signed saturation to unsigned result.
APInt APInt::truncSSatU(unsigned width) ಧ್ರುವ {
  ಪ್ರತಿಜ್ಞಾ(width <= BitWidth && "Invalid APInt Truncate request");

  // Can we just losslessly truncate it?
  ಯದಿ (isIntN(width))
    ಪ್ರತ್ಯಾಗಮ trunc(width);
  // If not, then just return the new limits.
  ಪ್ರತ್ಯಾಗಮ isNegative() ? APInt::getZero(width) : APInt::get_Max_ಮೌಲ್ಯ(width);
}

// Sign extend to a new width.
APInt APInt::sext(unsigned Width) ಧ್ರುವ {
  ಪ್ರತಿಜ್ಞಾ(Width >= BitWidth && "Invalid APInt SignExtend request");

  ಯದಿ (Width <= APINT_BITS_PER_WORD)
    ಪ್ರತ್ಯಾಗಮ APInt(Width, SignExtend64(U.VAL, BitWidth), /*isSigned=*/ಸತ್ಯ);

  ಯದಿ (Width == BitWidth)
    ಪ್ರತ್ಯಾಗಮ *ಏತತ್;

  APInt Result(getMemory(get_ಅಂಕೆ_ಪದಗಳು(Width)), Width);

  // Copy words.
  std::memcpy(Result.U.pVal, getRawData(), get_ಅಂಕೆ_ಪದಗಳು() * APINT_WORD_SIZE);

  // Sign extend the last word since there may be unused bits in the input.
  Result.U.pVal[get_ಅಂಕೆ_ಪದಗಳು() - 1] =
      SignExtend64(Result.U.pVal[get_ಅಂಕೆ_ಪದಗಳು() - 1],
                   ((BitWidth - 1) % APINT_BITS_PER_WORD) + 1);

  // Fill with sign bits.
  std::memset(Result.U.pVal + get_ಅಂಕೆ_ಪದಗಳು(), isNegative() ? -1 : 0,
              (Result.get_ಅಂಕೆ_ಪದಗಳು() - get_ಅಂಕೆ_ಪದಗಳು()) * APINT_WORD_SIZE);
  Result.clearUnusedBits();
  ಪ್ರತ್ಯಾಗಮ Result;
}

//  Zero extend to a new width.
APInt APInt::zext(unsigned width) ಧ್ರುವ {
  ಪ್ರತಿಜ್ಞಾ(width >= BitWidth && "Invalid APInt ZeroExtend request");

  ಯದಿ (width <= APINT_BITS_PER_WORD)
    ಪ್ರತ್ಯಾಗಮ APInt(width, U.VAL);

  ಯದಿ (width == BitWidth)
    ಪ್ರತ್ಯಾಗಮ *ಏತತ್;

  APInt Result(getMemory(get_ಅಂಕೆ_ಪದಗಳು(width)), width);

  // Copy words.
  std::memcpy(Result.U.pVal, getRawData(), get_ಅಂಕೆ_ಪದಗಳು() * APINT_WORD_SIZE);

  // Zero remaining words.
  std::memset(Result.U.pVal + get_ಅಂಕೆ_ಪದಗಳು(), 0,
              (Result.get_ಅಂಕೆ_ಪದಗಳು() - get_ಅಂಕೆ_ಪದಗಳು()) * APINT_WORD_SIZE);

  ಪ್ರತ್ಯಾಗಮ Result;
}

APInt APInt::zextOrTrunc(unsigned width) ಧ್ರುವ {
  ಯದಿ (BitWidth < width)
    ಪ್ರತ್ಯಾಗಮ zext(width);
  ಯದಿ (BitWidth > width)
    ಪ್ರತ್ಯಾಗಮ trunc(width);
  ಪ್ರತ್ಯಾಗಮ *ಏತತ್;
}

APInt APInt::sextOrTrunc(unsigned width) ಧ್ರುವ {
  ಯದಿ (BitWidth < width)
    ಪ್ರತ್ಯಾಗಮ sext(width);
  ಯದಿ (BitWidth > width)
    ಪ್ರತ್ಯಾಗಮ trunc(width);
  ಪ್ರತ್ಯಾಗಮ *ಏತತ್;
}

/// Arithmetic right-shift this APInt by shiftAmt.
/// Arithmetic right-shift function.
ಶೂನ್ಯ APInt::ashrInPlace(ಧ್ರುವ APInt &shiftAmt) {
  ashrInPlace((unsigned)shiftAmt.get_Limited_ಮೌಲ್ಯ(BitWidth));
}

/// Arithmetic right-shift this APInt by shiftAmt.
/// Arithmetic right-shift function.
ಶೂನ್ಯ APInt::ashrSlowCase(unsigned ShiftAmt) {
  // Don't bother performing a no-op shift.
  ಯದಿ (!ShiftAmt)
    ಪ್ರತ್ಯಾಗಮ;

  // Save the original sign bit for later.
  ತಾರ್ಕಿಕ Negative = isNegative();

  // WordShift is the inter-part shift; BitShift is intra-part shift.
  unsigned WordShift = ShiftAmt / APINT_BITS_PER_WORD;
  unsigned BitShift = ShiftAmt % APINT_BITS_PER_WORD;

  unsigned ಪದಗಳು_To_Move = get_ಅಂಕೆ_ಪದಗಳು() - WordShift;
  ಯದಿ (ಪದಗಳು_To_Move != 0) {
    // Sign extend the last word to fill in the unused bits.
    U.pVal[get_ಅಂಕೆ_ಪದಗಳು() - 1] = SignExtend64(
        U.pVal[get_ಅಂಕೆ_ಪದಗಳು() - 1], ((BitWidth - 1) % APINT_BITS_PER_WORD) + 1);

    // Fastpath for moving by whole words.
    ಯದಿ (BitShift == 0) {
      std::memmove(U.pVal, U.pVal + WordShift, ಪದಗಳು_To_Move * APINT_WORD_SIZE);
    } ಅನ್ಯಥಾ {
      // Move the words containing significant bits.
      ಪ್ರತಿ (unsigned i = 0; i != ಪದಗಳು_To_Move - 1; ++i)
        U.pVal[i] = (U.pVal[i + WordShift] >> BitShift) |
                    (U.pVal[i + WordShift + 1] << (APINT_BITS_PER_WORD - BitShift));

      // Handle the last word which has no high bits to copy. Use an arithmetic
      // shift to preserve the sign bit.
      U.pVal[ಪದಗಳು_To_Move - 1] =
          (int64_t)U.pVal[WordShift + ಪದಗಳು_To_Move - 1] >> BitShift;
    }
  }

  // Fill in the remainder based on the original sign.
  std::memset(U.pVal + ಪದಗಳು_To_Move, Negative ? -1 : 0,
              WordShift * APINT_WORD_SIZE);
  clearUnusedBits();
}

/// Logical right-shift this APInt by shiftAmt.
/// Logical right-shift function.
ಶೂನ್ಯ APInt::lshrInPlace(ಧ್ರುವ APInt &shiftAmt) {
  lshrInPlace((unsigned)shiftAmt.get_Limited_ಮೌಲ್ಯ(BitWidth));
}

/// Logical right-shift this APInt by shiftAmt.
/// Logical right-shift function.
ಶೂನ್ಯ APInt::lshrSlowCase(unsigned ShiftAmt) {
  tc_Shift_ಬಲ(U.pVal, get_ಅಂಕೆ_ಪದಗಳು(), ShiftAmt);
}

/// Left-shift this APInt by shiftAmt.
/// Left-shift function.
APInt &APInt::operator<<=(ಧ್ರುವ APInt &shiftAmt) {
  // It's undefined behavior in C to shift by BitWidth or greater.
  *ಏತತ್ <<= (unsigned)shiftAmt.get_Limited_ಮೌಲ್ಯ(BitWidth);
  ಪ್ರತ್ಯಾಗಮ *ಏತತ್;
}

ಶೂನ್ಯ APInt::shlSlowCase(unsigned ShiftAmt) {
  tc_Shift_ಎಡ(U.pVal, get_ಅಂಕೆ_ಪದಗಳು(), ShiftAmt);
  clearUnusedBits();
}

// Calculate the rotate amount modulo the bit width.
ಸ್ಥಿರ unsigned rotateModulo(unsigned BitWidth, ಧ್ರುವ APInt &rotateAmt) {
  ಯದಿ (LLVM_UNLIKELY(BitWidth == 0))
    ಪ್ರತ್ಯಾಗಮ 0;
  unsigned rotBitWidth = rotateAmt.getBitWidth();
  APInt rot = rotateAmt;
  ಯದಿ (rotBitWidth < BitWidth) {
    // Extend the rotate APInt, so that the urem doesn't divide by 0.
    // e.g. APInt(1, 32) would give APInt(1, 0).
    rot = rotateAmt.zext(BitWidth);
  }
  rot = rot.urem(APInt(rot.getBitWidth(), BitWidth));
  ಪ್ರತ್ಯಾಗಮ rot.get_Limited_ಮೌಲ್ಯ(BitWidth);
}

APInt APInt::rotl(ಧ್ರುವ APInt &rotateAmt) ಧ್ರುವ {
  ಪ್ರತ್ಯಾಗಮ rotl(rotateModulo(BitWidth, rotateAmt));
}

APInt APInt::rotl(unsigned rotateAmt) ಧ್ರುವ {
  ಯದಿ (LLVM_UNLIKELY(BitWidth == 0))
    ಪ್ರತ್ಯಾಗಮ *ಏತತ್;
  rotateAmt %= BitWidth;
  ಯದಿ (rotateAmt == 0)
    ಪ್ರತ್ಯಾಗಮ *ಏತತ್;
  ಪ್ರತ್ಯಾಗಮ shl(rotateAmt) | lshr(BitWidth - rotateAmt);
}

APInt APInt::rotr(ಧ್ರುವ APInt &rotateAmt) ಧ್ರುವ {
  ಪ್ರತ್ಯಾಗಮ rotr(rotateModulo(BitWidth, rotateAmt));
}

APInt APInt::rotr(unsigned rotateAmt) ಧ್ರುವ {
  ಯದಿ (BitWidth == 0)
    ಪ್ರತ್ಯಾಗಮ *ಏತತ್;
  rotateAmt %= BitWidth;
  ಯದಿ (rotateAmt == 0)
    ಪ್ರತ್ಯಾಗಮ *ಏತತ್;
  ಪ್ರತ್ಯಾಗಮ lshr(rotateAmt) | shl(BitWidth - rotateAmt);
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
unsigned APInt::nearestLogBase2() ಧ್ರುವ {
  // Special case when we have a bitwidth of 1. If VAL is 1, then we
  // get 0. If VAL is 0, we get WORDTYPE_MAX which gets truncated to
  // UINT32_MAX.
  ಯದಿ (BitWidth == 1)
    ಪ್ರತ್ಯಾಗಮ U.VAL - 1;

  // Handle the zero case.
  ಯದಿ (isZero())
    ಪ್ರತ್ಯಾಗಮ UINT32_MAX;

  // The non-zero case is handled by computing:
  //
  //   nearestLogBase2(x) = logBase2(x) + x[logBase2(x)-1].
  //
  // where x[i] is referring to the value of the ith bit of x.
  unsigned lg = logBase2();
  ಪ್ರತ್ಯಾಗಮ lg + unsigned((*ಏತತ್)[lg - 1]);
}

// Square Root - this method computes and returns the square root of "this".
// Three mechanisms are used for computation. For small values (<= 5 bits),
// a table lookup is done. This gets some performance for common cases. For
// values using less than 52 bits, the value is converted to double and then
// the libc sqrt function is called. The result is rounded and then converted
// back to a uint64_t which is then used to construct the result. Finally,
// the Babylonian method for computing square roots is used.
APInt APInt::sqrtFloor() ಧ್ರುವ {

  // Determine the magnitude of the value.
  unsigned magnitude = getActiveBits();

  // Use a fast table for some small values. This also gets rid of some
  // rounding errors in libc sqrt for small values.
  ಯದಿ (magnitude <= 5) {
    ಸ್ಥಿರ ಧ್ರುವ uint8_t results[32] = {
        /*     0 */ 0,
        /*  1- 3 */ 1, 1, 1,
        /*  4- 8 */ 2, 2, 2, 2, 2,
        /*  9-15 */ 3, 3, 3, 3, 3, 3, 3,
        /* 16-24 */ 4, 4, 4, 4, 4, 4, 4, 4, 4,
        /* 25-31 */ 5, 5, 5, 5, 5, 5, 5,
    };
    ಪ್ರತ್ಯಾಗಮ APInt(BitWidth,
                 results[(LLVM_LIKELY(isSingleWord()) ? U.VAL : U.pVal[0])]);
  }

  // If the magnitude of the value fits in less than 52 bits (the precision of
  // an IEEE double precision floating point value), then we can use the
  // libc sqrt function which will probably use a hardware sqrt computation.
  // This should be faster than the algorithm below.
  ಯದಿ (magnitude < 52) {
    ಪ್ರತ್ಯಾಗಮ APInt(BitWidth,
                 uint64_t(::floor(::sqrt(ದ್ವಿಗುಣ(
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
  ಪ್ರತಿ (;; i += 2, testy = testy.shl(2))
    ಯದಿ (i >= nbits || ಏತತ್->ule(testy)) {
      x_old = x_old.shl(i / 2);
      ಭಙ್ಗ;
    }

  // Use the Babylonian method to arrive at the integer square root:
  ಪ್ರತಿ (;;) {
    x_new = (ಏತತ್->udiv(x_old) + x_old).udiv(two);
    ಯದಿ (x_old.ule(x_new))
      ಭಙ್ಗ;
    x_old = x_new;
  }
  ಪ್ರತ್ಯಾಗಮ x_old;
}

/// \returns the multiplicative inverse of an odd APInt modulo 2^BitWidth.
APInt APInt::multiplicativeInverse() ಧ್ರುವ {
  ಪ್ರತಿಜ್ಞಾ((*ಏತತ್)[0] &&
         "multiplicative inverse is only defined for odd numbers!");

  // Use Newton's method.
  APInt Factor = *ಏತತ್;
  APInt T;
  ಯಾವತ್ (!(T = *ಏತತ್ * Factor).isOne())
    Factor *= 2 - std::move(T);
  ಪ್ರತ್ಯಾಗಮ Factor;
}

/// Implementation of Knuth's Algorithm D (Division of nonnegative integers)
/// from "Art of Computer Programming, Volume 2", section 4.3.1, p. 272. The
/// variables here have the same names as in the algorithm. Comments explain
/// the algorithm and any deviation from it.
ಸ್ಥಿರ ಶೂನ್ಯ KnuthDiv(uint32_t *u, uint32_t *v, uint32_t *q, uint32_t* r,
                     unsigned m, unsigned n) {
  ಪ್ರತಿಜ್ಞಾ(u && "Must provide dividend");
  ಪ್ರತಿಜ್ಞಾ(v && "Must provide divisor");
  ಪ್ರತಿಜ್ಞಾ(q && "Must provide quotient");
  ಪ್ರತಿಜ್ಞಾ(u != v && u != q && v != q && "Must use different memory");
  ಪ್ರತಿಜ್ಞಾ(n>1 && "n must be > 1");

  // b denotes the base of the number system. In our case b is 2^32.
  ಧ್ರುವ uint64_t b = uint64_t(1) << 32;

// The DEBUG macros here tend to be spam in the debug output if you're not
// debugging this code. Disable them unless KNUTH_DEBUG is defined.
#ifdef KNUTH_DEBUG
#define DEBUG_KNUTH(X) LLVM_DEBUG(X)
#ಅನ್ಯಥಾ
#define DEBUG_KNUTH(X) ಕುರು {} ಯಾವತ್(ಅಸತ್ಯ)
#endif

  DEBUG_KNUTH(dbgs() << "KnuthDiv: m=" << m << " n=" << n << '\n');
  DEBUG_KNUTH(dbgs() << "KnuthDiv: original:");
  DEBUG_KNUTH(ಪ್ರತಿ (ಪೂರ್ಣಾಙ್ಕ i = m + n; i >= 0; i--) dbgs() << " " << u[i]);
  DEBUG_KNUTH(dbgs() << " by");
  DEBUG_KNUTH(ಪ್ರತಿ (ಪೂರ್ಣಾಙ್ಕ i = n; i > 0; i--) dbgs() << " " << v[i - 1]);
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
  ಯದಿ (shift) {
    ಪ್ರತಿ (unsigned i = 0; i < m+n; ++i) {
      uint32_t u_tmp = u[i] >> (32 - shift);
      u[i] = (u[i] << shift) | u_carry;
      u_carry = u_tmp;
    }
    ಪ್ರತಿ (unsigned i = 0; i < n; ++i) {
      uint32_t v_tmp = v[i] >> (32 - shift);
      v[i] = (v[i] << shift) | v_carry;
      v_carry = v_tmp;
    }
  }
  u[m+n] = u_carry;

  DEBUG_KNUTH(dbgs() << "KnuthDiv:   normal:");
  DEBUG_KNUTH(ಪ್ರತಿ (ಪೂರ್ಣಾಙ್ಕ i = m + n; i >= 0; i--) dbgs() << " " << u[i]);
  DEBUG_KNUTH(dbgs() << " by");
  DEBUG_KNUTH(ಪ್ರತಿ (ಪೂರ್ಣಾಙ್ಕ i = n; i > 0; i--) dbgs() << " " << v[i - 1]);
  DEBUG_KNUTH(dbgs() << '\n');

  // D2. [Initialize j.]  Set j to m. This is the loop counter over the places.
  ಪೂರ್ಣಾಙ್ಕ j = m;
  ಕುರು {
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
    ಯದಿ (qp == b || qp*v[n-2] > b*rp + u[j+n-2]) {
      qp--;
      rp += v[n-1];
      ಯದಿ (rp < b && (qp == b || qp*v[n-2] > b*rp + u[j+n-2]))
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
    ಪ್ರತಿ (unsigned i = 0; i < n; ++i) {
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
    DEBUG_KNUTH(ಪ್ರತಿ (ಪೂರ್ಣಾಙ್ಕ i = m + n; i >= 0; i--) dbgs() << " " << u[i]);
    DEBUG_KNUTH(dbgs() << '\n');

    // D5. [Test remainder.] Set q[j] = qp. If the result of step D4 was
    // negative, go to step D6; otherwise go on to step D7.
    q[j] = Lo_32(qp);
    ಯದಿ (isNeg) {
      // D6. [Add back]. The probability that this step is necessary is very
      // small, on the order of only 2/b. Make sure that test data accounts for
      // this possibility. Decrease q[j] by 1
      q[j]--;
      // and add (0v[n-1]...v[1]v[0]) to (u[j+n]u[j+n-1]...u[j+1]u[j]).
      // A carry will occur to the left of u[j+n], and it should be ignored
      // since it cancels with the borrow that occurred in D4.
      ತಾರ್ಕಿಕ carry = ಅಸತ್ಯ;
      ಪ್ರತಿ (unsigned i = 0; i < n; i++) {
        uint32_t limit = std::min(u[j+i],v[i]);
        u[j+i] += v[i] + carry;
        carry = u[j+i] < limit || (carry && u[j+i] == limit);
      }
      u[j+n] += carry;
    }
    DEBUG_KNUTH(dbgs() << "KnuthDiv: after correction:");
    DEBUG_KNUTH(ಪ್ರತಿ (ಪೂರ್ಣಾಙ್ಕ i = m + n; i >= 0; i--) dbgs() << " " << u[i]);
    DEBUG_KNUTH(dbgs() << "\nKnuthDiv: digit result = " << q[j] << '\n');

    // D7. [Loop on j.]  Decrease j by one. Now if j >= 0, go back to D3.
  } ಯಾವತ್ (--j >= 0);

  DEBUG_KNUTH(dbgs() << "KnuthDiv: quotient:");
  DEBUG_KNUTH(ಪ್ರತಿ (ಪೂರ್ಣಾಙ್ಕ i = m; i >= 0; i--) dbgs() << " " << q[i]);
  DEBUG_KNUTH(dbgs() << '\n');

  // D8. [Unnormalize]. Now q[...] is the desired quotient, and the desired
  // remainder may be obtained by dividing u[...] by d. If r is non-null we
  // compute the remainder (urem uses this).
  ಯದಿ (r) {
    // The value d is expressed by the "shift" value above since we avoided
    // multiplication by d by using a shift left. So, all we have to do is
    // shift right here.
    ಯದಿ (shift) {
      uint32_t carry = 0;
      DEBUG_KNUTH(dbgs() << "KnuthDiv: remainder:");
      ಪ್ರತಿ (ಪೂರ್ಣಾಙ್ಕ i = n-1; i >= 0; i--) {
        r[i] = (u[i] >> shift) | carry;
        carry = u[i] << (32 - shift);
        DEBUG_KNUTH(dbgs() << " " << r[i]);
      }
    } ಅನ್ಯಥಾ {
      ಪ್ರತಿ (ಪೂರ್ಣಾಙ್ಕ i = n-1; i >= 0; i--) {
        r[i] = u[i];
        DEBUG_KNUTH(dbgs() << " " << r[i]);
      }
    }
    DEBUG_KNUTH(dbgs() << '\n');
  }
  DEBUG_KNUTH(dbgs() << '\n');
}

ಶೂನ್ಯ APInt::divide(ಧ್ರುವ WordType *LHS, unsigned lhs_ಪದಗಳು, ಧ್ರುವ WordType *RHS,
                   unsigned rhs_ಪದಗಳು, WordType *Quotient, WordType *Remainder) {
  ಪ್ರತಿಜ್ಞಾ(lhs_ಪದಗಳು >= rhs_ಪದಗಳು && "Fractional result");

  // First, compose the values into an array of 32-bit words instead of
  // 64-bit words. This is a necessity of both the "short division" algorithm
  // and the Knuth "classical algorithm" which requires there to be native
  // operations for +, -, and * on an m bit value with an m*2 bit result. We
  // can't use 64-bit operands here because we don't have native results of
  // 128-bits. Furthermore, casting the 64-bit values to 32-bit values won't
  // work on large-endian machines.
  unsigned n = rhs_ಪದಗಳು * 2;
  unsigned m = (lhs_ಪದಗಳು * 2) - n;

  // Allocate space for the temporary values we need either on the stack, if
  // it will fit, or on the heap if it won't.
  uint32_t SPACE[128];
  uint32_t *U = nullptr;
  uint32_t *V = nullptr;
  uint32_t *Q = nullptr;
  uint32_t *R = nullptr;
  ಯದಿ ((Remainder?4:3)*n+2*m+1 <= 128) {
    U = &SPACE[0];
    V = &SPACE[m+n+1];
    Q = &SPACE[(m+n+1) + n];
    ಯದಿ (Remainder)
      R = &SPACE[(m+n+1) + n + (m+n)];
  } ಅನ್ಯಥಾ {
    U = ನೂತನ uint32_t[m + n + 1];
    V = ನೂತನ uint32_t[n];
    Q = ನೂತನ uint32_t[m+n];
    ಯದಿ (Remainder)
      R = ನೂತನ uint32_t[n];
  }

  // Initialize the dividend
  memset(U, 0, (m+n+1)*ಪರಿಮಾಣ(uint32_t));
  ಪ್ರತಿ (unsigned i = 0; i < lhs_ಪದಗಳು; ++i) {
    uint64_t tmp = LHS[i];
    U[i * 2] = Lo_32(tmp);
    U[i * 2 + 1] = Hi_32(tmp);
  }
  U[m+n] = 0; // this extra word is for "spill" in the Knuth algorithm.

  // Initialize the divisor
  memset(V, 0, (n)*ಪರಿಮಾಣ(uint32_t));
  ಪ್ರತಿ (unsigned i = 0; i < rhs_ಪದಗಳು; ++i) {
    uint64_t tmp = RHS[i];
    V[i * 2] = Lo_32(tmp);
    V[i * 2 + 1] = Hi_32(tmp);
  }

  // initialize the quotient and remainder
  memset(Q, 0, (m+n) * ಪರಿಮಾಣ(uint32_t));
  ಯದಿ (Remainder)
    memset(R, 0, n * ಪರಿಮಾಣ(uint32_t));

  // Now, adjust m and n for the Knuth division. n is the number of words in
  // the divisor. m is the number of words by which the dividend exceeds the
  // divisor (i.e. m+n is the length of the dividend). These sizes must not
  // contain any zero words or the Knuth algorithm fails.
  ಪ್ರತಿ (unsigned i = n; i > 0 && V[i-1] == 0; i--) {
    n--;
    m++;
  }
  ಪ್ರತಿ (unsigned i = m+n; i > 0 && U[i-1] == 0; i--)
    m--;

  // If we're left with only a single word for the divisor, Knuth doesn't work
  // so we implement the short division algorithm here. This is much simpler
  // and faster because we are certain that we can divide a 64-bit quantity
  // by a 32-bit quantity at hardware speed and short division is simply a
  // series of such operations. This is just like doing short division but we
  // are using base 2^32 instead of base 10.
  ಪ್ರತಿಜ್ಞಾ(n != 0 && "Divide by zero?");
  ಯದಿ (n == 1) {
    uint32_t ಭಾಜಕ = V[0];
    uint32_t remainder = 0;
    ಪ್ರತಿ (ಪೂರ್ಣಾಙ್ಕ i = m; i >= 0; i--) {
      uint64_t partial_dividend = Make_64(remainder, U[i]);
      ಯದಿ (partial_dividend == 0) {
        Q[i] = 0;
        remainder = 0;
      } ಅನ್ಯಥಾ ಯದಿ (partial_dividend < ಭಾಜಕ) {
        Q[i] = 0;
        remainder = Lo_32(partial_dividend);
      } ಅನ್ಯಥಾ ಯದಿ (partial_dividend == ಭಾಜಕ) {
        Q[i] = 1;
        remainder = 0;
      } ಅನ್ಯಥಾ {
        Q[i] = Lo_32(partial_dividend / ಭಾಜಕ);
        remainder = Lo_32(partial_dividend - (Q[i] * ಭಾಜಕ));
      }
    }
    ಯದಿ (R)
      R[0] = remainder;
  } ಅನ್ಯಥಾ {
    // Now we're ready to invoke the Knuth classical divide algorithm. In this
    // case n > 1.
    KnuthDiv(U, V, Q, R, m, n);
  }

  // If the caller wants the quotient
  ಯದಿ (Quotient) {
    ಪ್ರತಿ (unsigned i = 0; i < lhs_ಪದಗಳು; ++i)
      Quotient[i] = Make_64(Q[i*2+1], Q[i*2]);
  }

  // If the caller wants the remainder
  ಯದಿ (Remainder) {
    ಪ್ರತಿ (unsigned i = 0; i < rhs_ಪದಗಳು; ++i)
      Remainder[i] = Make_64(R[i*2+1], R[i*2]);
  }

  // Clean up the memory we allocated.
  ಯದಿ (U != &SPACE[0]) {
    delete [] U;
    delete [] V;
    delete [] Q;
    delete [] R;
  }
}

APInt APInt::udiv(ಧ್ರುವ APInt &RHS) ಧ್ರುವ {
  ಪ್ರತಿಜ್ಞಾ(BitWidth == RHS.BitWidth && "Bit widths must be the same");

  // First, deal with the easy case
  ಯದಿ (LLVM_LIKELY(isSingleWord())) {
    ಪ್ರತಿಜ್ಞಾ(RHS.U.VAL != 0 && "Divide by zero?");
    ಪ್ರತ್ಯಾಗಮ APInt(BitWidth, U.VAL / RHS.U.VAL);
  }

  // Get some facts about the LHS and RHS number of bits and words
  unsigned lhs_ಪದಗಳು = get_ಅಂಕೆ_ಪದಗಳು(getActiveBits());
  unsigned rhsBits  = RHS.getActiveBits();
  unsigned rhs_ಪದಗಳು = get_ಅಂಕೆ_ಪದಗಳು(rhsBits);
  ಪ್ರತಿಜ್ಞಾ(rhs_ಪದಗಳು && "Divided by zero???");

  // Deal with some degenerate cases
  ಯದಿ (!lhs_ಪದಗಳು)
    // 0 / X ===> 0
    ಪ್ರತ್ಯಾಗಮ APInt(BitWidth, 0);
  ಯದಿ (rhsBits == 1)
    // X / 1 ===> X
    ಪ್ರತ್ಯಾಗಮ *ಏತತ್;
  ಯದಿ (lhs_ಪದಗಳು < rhs_ಪದಗಳು || ಏತತ್->ult(RHS))
    // X / Y ===> 0, iff X < Y
    ಪ್ರತ್ಯಾಗಮ APInt(BitWidth, 0);
  ಯದಿ (*ಏತತ್ == RHS)
    // X / X ===> 1
    ಪ್ರತ್ಯಾಗಮ APInt(BitWidth, 1);
  ಯದಿ (lhs_ಪದಗಳು == 1) // rhsWords is 1 if lhsWords is 1.
    // All high words are zero, just use native divide
    ಪ್ರತ್ಯಾಗಮ APInt(BitWidth, ಏತತ್->U.pVal[0] / RHS.U.pVal[0]);

  // We have to compute it the hard way. Invoke the Knuth divide algorithm.
  APInt Quotient(BitWidth, 0); // to hold result.
  divide(U.pVal, lhs_ಪದಗಳು, RHS.U.pVal, rhs_ಪದಗಳು, Quotient.U.pVal, nullptr);
  ಪ್ರತ್ಯಾಗಮ Quotient;
}

APInt APInt::udiv(uint64_t RHS) ಧ್ರುವ {
  ಪ್ರತಿಜ್ಞಾ(RHS != 0 && "Divide by zero?");

  // First, deal with the easy case
  ಯದಿ (LLVM_LIKELY(isSingleWord()))
    ಪ್ರತ್ಯಾಗಮ APInt(BitWidth, U.VAL / RHS);

  // Get some facts about the LHS words.
  unsigned lhs_ಪದಗಳು = get_ಅಂಕೆ_ಪದಗಳು(getActiveBits());

  // Deal with some degenerate cases
  ಯದಿ (!lhs_ಪದಗಳು)
    // 0 / X ===> 0
    ಪ್ರತ್ಯಾಗಮ APInt(BitWidth, 0);
  ಯದಿ (RHS == 1)
    // X / 1 ===> X
    ಪ್ರತ್ಯಾಗಮ *ಏತತ್;
  ಯದಿ (ಏತತ್->ult(RHS))
    // X / Y ===> 0, iff X < Y
    ಪ್ರತ್ಯಾಗಮ APInt(BitWidth, 0);
  ಯದಿ (*ಏತತ್ == RHS)
    // X / X ===> 1
    ಪ್ರತ್ಯಾಗಮ APInt(BitWidth, 1);
  ಯದಿ (lhs_ಪದಗಳು == 1) // rhsWords is 1 if lhsWords is 1.
    // All high words are zero, just use native divide
    ಪ್ರತ್ಯಾಗಮ APInt(BitWidth, ಏತತ್->U.pVal[0] / RHS);

  // We have to compute it the hard way. Invoke the Knuth divide algorithm.
  APInt Quotient(BitWidth, 0); // to hold result.
  divide(U.pVal, lhs_ಪದಗಳು, &RHS, 1, Quotient.U.pVal, nullptr);
  ಪ್ರತ್ಯಾಗಮ Quotient;
}

APInt APInt::sdiv(ಧ್ರುವ APInt &RHS) ಧ್ರುವ {
  ಯದಿ (isNegative()) {
    ಯದಿ (RHS.isNegative())
      ಪ್ರತ್ಯಾಗಮ (-(*ಏತತ್)).udiv(-RHS);
    ಪ್ರತ್ಯಾಗಮ -((-(*ಏತತ್)).udiv(RHS));
  }
  ಯದಿ (RHS.isNegative())
    ಪ್ರತ್ಯಾಗಮ -(ಏತತ್->udiv(-RHS));
  ಪ್ರತ್ಯಾಗಮ ಏತತ್->udiv(RHS);
}

APInt APInt::sdiv(int64_t RHS) ಧ್ರುವ {
  ಯದಿ (isNegative()) {
    ಯದಿ (RHS < 0)
      ಪ್ರತ್ಯಾಗಮ (-(*ಏತತ್)).udiv(-RHS);
    ಪ್ರತ್ಯಾಗಮ -((-(*ಏತತ್)).udiv(RHS));
  }
  ಯದಿ (RHS < 0)
    ಪ್ರತ್ಯಾಗಮ -(ಏತತ್->udiv(-RHS));
  ಪ್ರತ್ಯಾಗಮ ಏತತ್->udiv(RHS);
}

APInt APInt::urem(ಧ್ರುವ APInt &RHS) ಧ್ರುವ {
  ಪ್ರತಿಜ್ಞಾ(BitWidth == RHS.BitWidth && "Bit widths must be the same");
  ಯದಿ (LLVM_LIKELY(isSingleWord())) {
    ಪ್ರತಿಜ್ಞಾ(RHS.U.VAL != 0 && "Remainder by zero?");
    ಪ್ರತ್ಯಾಗಮ APInt(BitWidth, U.VAL % RHS.U.VAL);
  }

  // Get some facts about the LHS
  unsigned lhs_ಪದಗಳು = get_ಅಂಕೆ_ಪದಗಳು(getActiveBits());

  // Get some facts about the RHS
  unsigned rhsBits = RHS.getActiveBits();
  unsigned rhs_ಪದಗಳು = get_ಅಂಕೆ_ಪದಗಳು(rhsBits);
  ಪ್ರತಿಜ್ಞಾ(rhs_ಪದಗಳು && "Performing remainder operation by zero ???");

  // Check the degenerate cases
  ಯದಿ (lhs_ಪದಗಳು == 0)
    // 0 % Y ===> 0
    ಪ್ರತ್ಯಾಗಮ APInt(BitWidth, 0);
  ಯದಿ (rhsBits == 1)
    // X % 1 ===> 0
    ಪ್ರತ್ಯಾಗಮ APInt(BitWidth, 0);
  ಯದಿ (lhs_ಪದಗಳು < rhs_ಪದಗಳು || ಏತತ್->ult(RHS))
    // X % Y ===> X, iff X < Y
    ಪ್ರತ್ಯಾಗಮ *ಏತತ್;
  ಯದಿ (*ಏತತ್ == RHS)
    // X % X == 0;
    ಪ್ರತ್ಯಾಗಮ APInt(BitWidth, 0);
  ಯದಿ (lhs_ಪದಗಳು == 1)
    // All high words are zero, just use native remainder
    ಪ್ರತ್ಯಾಗಮ APInt(BitWidth, U.pVal[0] % RHS.U.pVal[0]);
  ಯದಿ (RHS.isPowerOf2()) {
    // X % 2^w ===> X & (2^w - 1)
    APInt Result(*ಏತತ್);
    Result.clearBits(RHS.logBase2(), BitWidth);
    ಪ್ರತ್ಯಾಗಮ Result;
  }

  // We have to compute it the hard way. Invoke the Knuth divide algorithm.
  APInt Remainder(BitWidth, 0);
  divide(U.pVal, lhs_ಪದಗಳು, RHS.U.pVal, rhs_ಪದಗಳು, nullptr, Remainder.U.pVal);
  ಪ್ರತ್ಯಾಗಮ Remainder;
}

uint64_t APInt::urem(uint64_t RHS) ಧ್ರುವ {
  ಪ್ರತಿಜ್ಞಾ(RHS != 0 && "Remainder by zero?");

  ಯದಿ (LLVM_LIKELY(isSingleWord()))
    ಪ್ರತ್ಯಾಗಮ U.VAL % RHS;

  // Get some facts about the LHS
  unsigned lhs_ಪದಗಳು = get_ಅಂಕೆ_ಪದಗಳು(getActiveBits());

  // Check the degenerate cases
  ಯದಿ (lhs_ಪದಗಳು == 0)
    // 0 % Y ===> 0
    ಪ್ರತ್ಯಾಗಮ 0;
  ಯದಿ (RHS == 1)
    // X % 1 ===> 0
    ಪ್ರತ್ಯಾಗಮ 0;
  ಯದಿ (ಏತತ್->ult(RHS))
    // X % Y ===> X, iff X < Y
    ಪ್ರತ್ಯಾಗಮ get_ZExt_ಮೌಲ್ಯ();
  ಯದಿ (*ಏತತ್ == RHS)
    // X % X == 0;
    ಪ್ರತ್ಯಾಗಮ 0;
  ಯದಿ (lhs_ಪದಗಳು == 1)
    // All high words are zero, just use native remainder
    ಪ್ರತ್ಯಾಗಮ U.pVal[0] % RHS;
  ಯದಿ (llvm::isPowerOf2_64(RHS))
    // X % 2^w ===> X & (2^w - 1)
    ಪ್ರತ್ಯಾಗಮ U.pVal[0] & (RHS - 1);

  // We have to compute it the hard way. Invoke the Knuth divide algorithm.
  uint64_t Remainder;
  divide(U.pVal, lhs_ಪದಗಳು, &RHS, 1, nullptr, &Remainder);
  ಪ್ರತ್ಯಾಗಮ Remainder;
}

APInt APInt::srem(ಧ್ರುವ APInt &RHS) ಧ್ರುವ {
  ಯದಿ (isNegative()) {
    ಯದಿ (RHS.isNegative())
      ಪ್ರತ್ಯಾಗಮ -((-(*ಏತತ್)).urem(-RHS));
    ಪ್ರತ್ಯಾಗಮ -((-(*ಏತತ್)).urem(RHS));
  }
  ಯದಿ (RHS.isNegative())
    ಪ್ರತ್ಯಾಗಮ ಏತತ್->urem(-RHS);
  ಪ್ರತ್ಯಾಗಮ ಏತತ್->urem(RHS);
}

int64_t APInt::srem(int64_t RHS) ಧ್ರುವ {
  ಯದಿ (isNegative()) {
    ಯದಿ (RHS < 0)
      ಪ್ರತ್ಯಾಗಮ -((-(*ಏತತ್)).urem(-RHS));
    ಪ್ರತ್ಯಾಗಮ -((-(*ಏತತ್)).urem(RHS));
  }
  ಯದಿ (RHS < 0)
    ಪ್ರತ್ಯಾಗಮ ಏತತ್->urem(-RHS);
  ಪ್ರತ್ಯಾಗಮ ಏತತ್->urem(RHS);
}

ಶೂನ್ಯ APInt::udivrem(ಧ್ರುವ APInt &LHS, ಧ್ರುವ APInt &RHS,
                    APInt &Quotient, APInt &Remainder) {
  ಪ್ರತಿಜ್ಞಾ(LHS.BitWidth == RHS.BitWidth && "Bit widths must be the same");
  unsigned BitWidth = LHS.BitWidth;

  // First, deal with the easy case
  ಯದಿ (LLVM_LIKELY(LHS.isSingleWord())) {
    ಪ್ರತಿಜ್ಞಾ(RHS.U.VAL != 0 && "Divide by zero?");
    uint64_t QuotVal = LHS.U.VAL / RHS.U.VAL;
    uint64_t RemVal = LHS.U.VAL % RHS.U.VAL;
    Quotient = APInt(BitWidth, QuotVal);
    Remainder = APInt(BitWidth, RemVal);
    ಪ್ರತ್ಯಾಗಮ;
  }

  // Get some size facts about the dividend and divisor
  unsigned lhs_ಪದಗಳು = get_ಅಂಕೆ_ಪದಗಳು(LHS.getActiveBits());
  unsigned rhsBits  = RHS.getActiveBits();
  unsigned rhs_ಪದಗಳು = get_ಅಂಕೆ_ಪದಗಳು(rhsBits);
  ಪ್ರತಿಜ್ಞಾ(rhs_ಪದಗಳು && "Performing divrem operation by zero ???");

  // Check the degenerate cases
  ಯದಿ (lhs_ಪದಗಳು == 0) {
    Quotient = APInt(BitWidth, 0);    // 0 / Y ===> 0
    Remainder = APInt(BitWidth, 0);   // 0 % Y ===> 0
    ಪ್ರತ್ಯಾಗಮ;
  }

  ಯದಿ (rhsBits == 1) {
    Quotient = LHS;                   // X / 1 ===> X
    Remainder = APInt(BitWidth, 0);   // X % 1 ===> 0
  }

  ಯದಿ (lhs_ಪದಗಳು < rhs_ಪದಗಳು || LHS.ult(RHS)) {
    Remainder = LHS;                  // X % Y ===> X, iff X < Y
    Quotient = APInt(BitWidth, 0);    // X / Y ===> 0, iff X < Y
    ಪ್ರತ್ಯಾಗಮ;
  }

  ಯದಿ (LHS == RHS) {
    Quotient  = APInt(BitWidth, 1);   // X / X ===> 1
    Remainder = APInt(BitWidth, 0);   // X % X ===> 0;
    ಪ್ರತ್ಯಾಗಮ;
  }

  // Make sure there is enough space to hold the results.
  // NOTE: This assumes that reallocate won't affect any bits if it doesn't
  // change the size. This is necessary if Quotient or Remainder is aliased
  // with LHS or RHS.
  Quotient.reallocate(BitWidth);
  Remainder.reallocate(BitWidth);

  ಯದಿ (lhs_ಪದಗಳು == 1) { // rhsWords is 1 if lhsWords is 1.
    // There is only one word to consider so use the native versions.
    uint64_t lhs_ಮೌಲ್ಯ = LHS.U.pVal[0];
    uint64_t rhs_ಮೌಲ್ಯ = RHS.U.pVal[0];
    Quotient = lhs_ಮೌಲ್ಯ / rhs_ಮೌಲ್ಯ;
    Remainder = lhs_ಮೌಲ್ಯ % rhs_ಮೌಲ್ಯ;
    ಪ್ರತ್ಯಾಗಮ;
  }

  // Okay, lets do it the long way
  divide(LHS.U.pVal, lhs_ಪದಗಳು, RHS.U.pVal, rhs_ಪದಗಳು, Quotient.U.pVal,
         Remainder.U.pVal);
  // Clear the rest of the Quotient and Remainder.
  std::memset(Quotient.U.pVal + lhs_ಪದಗಳು, 0,
              (get_ಅಂಕೆ_ಪದಗಳು(BitWidth) - lhs_ಪದಗಳು) * APINT_WORD_SIZE);
  std::memset(Remainder.U.pVal + rhs_ಪದಗಳು, 0,
              (get_ಅಂಕೆ_ಪದಗಳು(BitWidth) - rhs_ಪದಗಳು) * APINT_WORD_SIZE);
}

ಶೂನ್ಯ APInt::udivrem(ಧ್ರುವ APInt &LHS, uint64_t RHS, APInt &Quotient,
                    uint64_t &Remainder) {
  ಪ್ರತಿಜ್ಞಾ(RHS != 0 && "Divide by zero?");
  unsigned BitWidth = LHS.BitWidth;

  // First, deal with the easy case
  ಯದಿ (LLVM_LIKELY(LHS.isSingleWord())) {
    uint64_t QuotVal = LHS.U.VAL / RHS;
    Remainder = LHS.U.VAL % RHS;
    Quotient = APInt(BitWidth, QuotVal);
    ಪ್ರತ್ಯಾಗಮ;
  }

  // Get some size facts about the dividend and divisor
  unsigned lhs_ಪದಗಳು = get_ಅಂಕೆ_ಪದಗಳು(LHS.getActiveBits());

  // Check the degenerate cases
  ಯದಿ (lhs_ಪದಗಳು == 0) {
    Quotient = APInt(BitWidth, 0);    // 0 / Y ===> 0
    Remainder = 0;                    // 0 % Y ===> 0
    ಪ್ರತ್ಯಾಗಮ;
  }

  ಯದಿ (RHS == 1) {
    Quotient = LHS;                   // X / 1 ===> X
    Remainder = 0;                    // X % 1 ===> 0
    ಪ್ರತ್ಯಾಗಮ;
  }

  ಯದಿ (LHS.ult(RHS)) {
    Remainder = LHS.get_ZExt_ಮೌಲ್ಯ();   // X % Y ===> X, iff X < Y
    Quotient = APInt(BitWidth, 0);    // X / Y ===> 0, iff X < Y
    ಪ್ರತ್ಯಾಗಮ;
  }

  ಯದಿ (LHS == RHS) {
    Quotient  = APInt(BitWidth, 1);   // X / X ===> 1
    Remainder = 0;                    // X % X ===> 0;
    ಪ್ರತ್ಯಾಗಮ;
  }

  // Make sure there is enough space to hold the results.
  // NOTE: This assumes that reallocate won't affect any bits if it doesn't
  // change the size. This is necessary if Quotient is aliased with LHS.
  Quotient.reallocate(BitWidth);

  ಯದಿ (lhs_ಪದಗಳು == 1) { // rhsWords is 1 if lhsWords is 1.
    // There is only one word to consider so use the native versions.
    uint64_t lhs_ಮೌಲ್ಯ = LHS.U.pVal[0];
    Quotient = lhs_ಮೌಲ್ಯ / RHS;
    Remainder = lhs_ಮೌಲ್ಯ % RHS;
    ಪ್ರತ್ಯಾಗಮ;
  }

  // Okay, lets do it the long way
  divide(LHS.U.pVal, lhs_ಪದಗಳು, &RHS, 1, Quotient.U.pVal, &Remainder);
  // Clear the rest of the Quotient.
  std::memset(Quotient.U.pVal + lhs_ಪದಗಳು, 0,
              (get_ಅಂಕೆ_ಪದಗಳು(BitWidth) - lhs_ಪದಗಳು) * APINT_WORD_SIZE);
}

ಶೂನ್ಯ APInt::sdivrem(ಧ್ರುವ APInt &LHS, ಧ್ರುವ APInt &RHS,
                    APInt &Quotient, APInt &Remainder) {
  ಯದಿ (LHS.isNegative()) {
    ಯದಿ (RHS.isNegative())
      APInt::udivrem(-LHS, -RHS, Quotient, Remainder);
    ಅನ್ಯಥಾ {
      APInt::udivrem(-LHS, RHS, Quotient, Remainder);
      Quotient.negate();
    }
    Remainder.negate();
  } ಅನ್ಯಥಾ ಯದಿ (RHS.isNegative()) {
    APInt::udivrem(LHS, -RHS, Quotient, Remainder);
    Quotient.negate();
  } ಅನ್ಯಥಾ {
    APInt::udivrem(LHS, RHS, Quotient, Remainder);
  }
}

ಶೂನ್ಯ APInt::sdivrem(ಧ್ರುವ APInt &LHS, int64_t RHS,
                    APInt &Quotient, int64_t &Remainder) {
  uint64_t R = Remainder;
  ಯದಿ (LHS.isNegative()) {
    ಯದಿ (RHS < 0)
      APInt::udivrem(-LHS, -RHS, Quotient, R);
    ಅನ್ಯಥಾ {
      APInt::udivrem(-LHS, RHS, Quotient, R);
      Quotient.negate();
    }
    R = -R;
  } ಅನ್ಯಥಾ ಯದಿ (RHS < 0) {
    APInt::udivrem(LHS, -RHS, Quotient, R);
    Quotient.negate();
  } ಅನ್ಯಥಾ {
    APInt::udivrem(LHS, RHS, Quotient, R);
  }
  Remainder = R;
}

APInt APInt::sadd_ov(ಧ್ರುವ APInt &RHS, ತಾರ್ಕಿಕ &Overflow) ಧ್ರುವ {
  APInt Res = *ಏತತ್+RHS;
  Overflow = isNonNegative() == RHS.isNonNegative() &&
             Res.isNonNegative() != isNonNegative();
  ಪ್ರತ್ಯಾಗಮ Res;
}

APInt APInt::uadd_ov(ಧ್ರುವ APInt &RHS, ತಾರ್ಕಿಕ &Overflow) ಧ್ರುವ {
  APInt Res = *ಏತತ್+RHS;
  Overflow = Res.ult(RHS);
  ಪ್ರತ್ಯಾಗಮ Res;
}

APInt APInt::ssub_ov(ಧ್ರುವ APInt &RHS, ತಾರ್ಕಿಕ &Overflow) ಧ್ರುವ {
  APInt Res = *ಏತತ್ - RHS;
  Overflow = isNonNegative() != RHS.isNonNegative() &&
             Res.isNonNegative() != isNonNegative();
  ಪ್ರತ್ಯಾಗಮ Res;
}

APInt APInt::usub_ov(ಧ್ರುವ APInt &RHS, ತಾರ್ಕಿಕ &Overflow) ಧ್ರುವ {
  APInt Res = *ಏತತ್-RHS;
  Overflow = Res.ugt(*ಏತತ್);
  ಪ್ರತ್ಯಾಗಮ Res;
}

APInt APInt::sdiv_ov(ಧ್ರುವ APInt &RHS, ತಾರ್ಕಿಕ &Overflow) ಧ್ರುವ {
  // MININT/-1  -->  overflow.
  Overflow = is_Min_Signed_ಮೌಲ್ಯ() && RHS.isAllOnes();
  ಪ್ರತ್ಯಾಗಮ sdiv(RHS);
}

APInt APInt::smul_ov(ಧ್ರುವ APInt &RHS, ತಾರ್ಕಿಕ &Overflow) ಧ್ರುವ {
  APInt Res = *ಏತತ್ * RHS;

  ಯದಿ (RHS != 0)
    Overflow = Res.sdiv(RHS) != *ಏತತ್ ||
               (is_Min_Signed_ಮೌಲ್ಯ() && RHS.isAllOnes());
  ಅನ್ಯಥಾ
    Overflow = ಅಸತ್ಯ;
  ಪ್ರತ್ಯಾಗಮ Res;
}

APInt APInt::umul_ov(ಧ್ರುವ APInt &RHS, ತಾರ್ಕಿಕ &Overflow) ಧ್ರುವ {
  ಯದಿ (countl_zero() + RHS.countl_zero() + 2 <= BitWidth) {
    Overflow = ಸತ್ಯ;
    ಪ್ರತ್ಯಾಗಮ *ಏತತ್ * RHS;
  }

  APInt Res = lshr(1) * RHS;
  Overflow = Res.isNegative();
  Res <<= 1;
  ಯದಿ ((*ಏತತ್)[0]) {
    Res += RHS;
    ಯದಿ (Res.ult(RHS))
      Overflow = ಸತ್ಯ;
  }
  ಪ್ರತ್ಯಾಗಮ Res;
}

APInt APInt::sshl_ov(ಧ್ರುವ APInt &ShAmt, ತಾರ್ಕಿಕ &Overflow) ಧ್ರುವ {
  ಪ್ರತ್ಯಾಗಮ sshl_ov(ShAmt.get_Limited_ಮೌಲ್ಯ(getBitWidth()), Overflow);
}

APInt APInt::sshl_ov(unsigned ShAmt, ತಾರ್ಕಿಕ &Overflow) ಧ್ರುವ {
  Overflow = ShAmt >= getBitWidth();
  ಯದಿ (Overflow)
    ಪ್ರತ್ಯಾಗಮ APInt(BitWidth, 0);

  ಯದಿ (isNonNegative()) // Don't allow sign change.
    Overflow = ShAmt >= countl_zero();
  ಅನ್ಯಥಾ
    Overflow = ShAmt >= countl_one();

  ಪ್ರತ್ಯಾಗಮ *ಏತತ್ << ShAmt;
}

APInt APInt::ushl_ov(ಧ್ರುವ APInt &ShAmt, ತಾರ್ಕಿಕ &Overflow) ಧ್ರುವ {
  ಪ್ರತ್ಯಾಗಮ ushl_ov(ShAmt.get_Limited_ಮೌಲ್ಯ(getBitWidth()), Overflow);
}

APInt APInt::ushl_ov(unsigned ShAmt, ತಾರ್ಕಿಕ &Overflow) ಧ್ರುವ {
  Overflow = ShAmt >= getBitWidth();
  ಯದಿ (Overflow)
    ಪ್ರತ್ಯಾಗಮ APInt(BitWidth, 0);

  Overflow = ShAmt > countl_zero();

  ಪ್ರತ್ಯಾಗಮ *ಏತತ್ << ShAmt;
}

APInt APInt::sfloordiv_ov(ಧ್ರುವ APInt &RHS, ತಾರ್ಕಿಕ &Overflow) ಧ್ರುವ {
  APInt quotient = sdiv_ov(RHS, Overflow);
  ಯದಿ ((quotient * RHS != *ಏತತ್) && (isNegative() != RHS.isNegative()))
    ಪ್ರತ್ಯಾಗಮ quotient - 1;
  ಪ್ರತ್ಯಾಗಮ quotient;
}

APInt APInt::sadd_sat(ಧ್ರುವ APInt &RHS) ಧ್ರುವ {
  ತಾರ್ಕಿಕ Overflow;
  APInt Res = sadd_ov(RHS, Overflow);
  ಯದಿ (!Overflow)
    ಪ್ರತ್ಯಾಗಮ Res;

  ಪ್ರತ್ಯಾಗಮ isNegative() ? APInt::get_Signed_Min_ಮೌಲ್ಯ(BitWidth)
                      : APInt::get_Signed_Max_ಮೌಲ್ಯ(BitWidth);
}

APInt APInt::uadd_sat(ಧ್ರುವ APInt &RHS) ಧ್ರುವ {
  ತಾರ್ಕಿಕ Overflow;
  APInt Res = uadd_ov(RHS, Overflow);
  ಯದಿ (!Overflow)
    ಪ್ರತ್ಯಾಗಮ Res;

  ಪ್ರತ್ಯಾಗಮ APInt::get_Max_ಮೌಲ್ಯ(BitWidth);
}

APInt APInt::ssub_sat(ಧ್ರುವ APInt &RHS) ಧ್ರುವ {
  ತಾರ್ಕಿಕ Overflow;
  APInt Res = ssub_ov(RHS, Overflow);
  ಯದಿ (!Overflow)
    ಪ್ರತ್ಯಾಗಮ Res;

  ಪ್ರತ್ಯಾಗಮ isNegative() ? APInt::get_Signed_Min_ಮೌಲ್ಯ(BitWidth)
                      : APInt::get_Signed_Max_ಮೌಲ್ಯ(BitWidth);
}

APInt APInt::usub_sat(ಧ್ರುವ APInt &RHS) ಧ್ರುವ {
  ತಾರ್ಕಿಕ Overflow;
  APInt Res = usub_ov(RHS, Overflow);
  ಯದಿ (!Overflow)
    ಪ್ರತ್ಯಾಗಮ Res;

  ಪ್ರತ್ಯಾಗಮ APInt(BitWidth, 0);
}

APInt APInt::smul_sat(ಧ್ರುವ APInt &RHS) ಧ್ರುವ {
  ತಾರ್ಕಿಕ Overflow;
  APInt Res = smul_ov(RHS, Overflow);
  ಯದಿ (!Overflow)
    ಪ್ರತ್ಯಾಗಮ Res;

  // The result is negative if one and only one of inputs is negative.
  ತಾರ್ಕಿಕ ResIsNegative = isNegative() ^ RHS.isNegative();

  ಪ್ರತ್ಯಾಗಮ ResIsNegative ? APInt::get_Signed_Min_ಮೌಲ್ಯ(BitWidth)
                       : APInt::get_Signed_Max_ಮೌಲ್ಯ(BitWidth);
}

APInt APInt::umul_sat(ಧ್ರುವ APInt &RHS) ಧ್ರುವ {
  ತಾರ್ಕಿಕ Overflow;
  APInt Res = umul_ov(RHS, Overflow);
  ಯದಿ (!Overflow)
    ಪ್ರತ್ಯಾಗಮ Res;

  ಪ್ರತ್ಯಾಗಮ APInt::get_Max_ಮೌಲ್ಯ(BitWidth);
}

APInt APInt::sshl_sat(ಧ್ರುವ APInt &RHS) ಧ್ರುವ {
  ಪ್ರತ್ಯಾಗಮ sshl_sat(RHS.get_Limited_ಮೌಲ್ಯ(getBitWidth()));
}

APInt APInt::sshl_sat(unsigned RHS) ಧ್ರುವ {
  ತಾರ್ಕಿಕ Overflow;
  APInt Res = sshl_ov(RHS, Overflow);
  ಯದಿ (!Overflow)
    ಪ್ರತ್ಯಾಗಮ Res;

  ಪ್ರತ್ಯಾಗಮ isNegative() ? APInt::get_Signed_Min_ಮೌಲ್ಯ(BitWidth)
                      : APInt::get_Signed_Max_ಮೌಲ್ಯ(BitWidth);
}

APInt APInt::ushl_sat(ಧ್ರುವ APInt &RHS) ಧ್ರುವ {
  ಪ್ರತ್ಯಾಗಮ ushl_sat(RHS.get_Limited_ಮೌಲ್ಯ(getBitWidth()));
}

APInt APInt::ushl_sat(unsigned RHS) ಧ್ರುವ {
  ತಾರ್ಕಿಕ Overflow;
  APInt Res = ushl_ov(RHS, Overflow);
  ಯದಿ (!Overflow)
    ಪ್ರತ್ಯಾಗಮ Res;

  ಪ್ರತ್ಯಾಗಮ APInt::get_Max_ಮೌಲ್ಯ(BitWidth);
}

ಶೂನ್ಯ APInt::fromString(unsigned numbits, StringRef str, uint8_t radix) {
  // Check our assumptions here
  ಪ್ರತಿಜ್ಞಾ(!str.empty() && "Invalid string length");
  ಪ್ರತಿಜ್ಞಾ((radix == 10 || radix == 8 || radix == 16 || radix == 2 ||
          radix == 36) &&
         "Radix should be 2, 8, 10, 16, or 36!");

  StringRef::iterator p = str.begin();
  size_t slen = str.size();
  ತಾರ್ಕಿಕ isNeg = *p == '-';
  ಯದಿ (*p == '-' || *p == '+') {
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
  ಯದಿ (LLVM_LIKELY(isSingleWord()))
    U.VAL = 0;
  ಅನ್ಯಥಾ
    U.pVal = getClearedMemory(get_ಅಂಕೆ_ಪದಗಳು());

  // Figure out if we can shift instead of multiply
  unsigned shift = (radix == 16 ? 4 : radix == 8 ? 3 : radix == 2 ? 1 : 0);

  // Enter digit traversal loop
  ಪ್ರತಿ (StringRef::iterator e = str.end(); p != e; ++p) {
    unsigned digit = getDigit(*p, radix);
    ಪ್ರತಿಜ್ಞಾ(digit < radix && "Invalid character in digit string");

    // Shift or multiply the value by the radix
    ಯದಿ (slen > 1) {
      ಯದಿ (shift)
        *ಏತತ್ <<= shift;
      ಅನ್ಯಥಾ
        *ಏತತ್ *= radix;
    }

    // Add in the digit we just interpreted
    *ಏತತ್ += digit;
  }
  // If its negative, put it in two's complement form
  ಯದಿ (isNeg)
    ಏತತ್->negate();
}

ಶೂನ್ಯ APInt::toString(SmallVectorImpl<ವರ್ಣ> &Str, unsigned Radix, ತಾರ್ಕಿಕ Signed,
                     ತಾರ್ಕಿಕ formatAsCLiteral, ತಾರ್ಕಿಕ UpperCase,
                     ತಾರ್ಕಿಕ ಸೇರಿಸು_Separators) ಧ್ರುವ {
  ಪ್ರತಿಜ್ಞಾ((Radix == 10 || Radix == 8 || Radix == 16 || Radix == 2 ||
          Radix == 36) &&
         "Radix should be 2, 8, 10, 16, or 36!");

  ಧ್ರುವ ವರ್ಣ *Prefix = "";
  ಯದಿ (formatAsCLiteral) {
    ವಿಕಲ್ಪ (Radix) {
      ಪಕ್ಷ 2:
        // Binary literals are a non-standard extension added in gcc 4.3:
        // http://gcc.gnu.org/onlinedocs/gcc-4.3.0/gcc/Binary-constants.html
        Prefix = "0b";
        ಭಙ್ಗ;
      ಪಕ್ಷ 8:
        Prefix = "0";
        ಭಙ್ಗ;
      ಪಕ್ಷ 10:
        ಭಙ್ಗ; // No prefix
      ಪಕ್ಷ 16:
        Prefix = "0x";
        ಭಙ್ಗ;
      ಸ್ವಭಾವ:
        llvm_unreachable("Invalid radix!");
    }
  }

  // Number of digits in a group between separators.
  unsigned Grouping = (Radix == 8 || Radix == 10) ? 3 : 4;

  // First, check for a zero value and just short circuit the logic below.
  ಯದಿ (isZero()) {
    ಯಾವತ್ (*Prefix) {
      Str.ತಳ್ಳು_back(*Prefix);
      ++Prefix;
    };
    Str.ತಳ್ಳು_back('0');
    ಪ್ರತ್ಯಾಗಮ;
  }

  ಸ್ಥಿರ ಧ್ರುವ ವರ್ಣ BothDigits[] = "0123456789abcdefghijklmnopqrstuvwxyz"
                                   "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ";
  ಧ್ರುವ ವರ್ಣ *Digits = BothDigits + (UpperCase ? 36 : 0);

  ಯದಿ (LLVM_LIKELY(isSingleWord())) {
    ವರ್ಣ Buffer[65];
    ವರ್ಣ *BufPtr = std::end(Buffer);

    uint64_t N;
    ಯದಿ (!Signed) {
      N = get_ZExt_ಮೌಲ್ಯ();
    } ಅನ್ಯಥಾ {
      int64_t I = get_SExt_ಮೌಲ್ಯ();
      ಯದಿ (I >= 0) {
        N = I;
      } ಅನ್ಯಥಾ {
        Str.ತಳ್ಳು_back('-');
        N = -(uint64_t)I;
      }
    }

    ಯಾವತ್ (*Prefix) {
      Str.ತಳ್ಳು_back(*Prefix);
      ++Prefix;
    };

    ಪೂರ್ಣಾಙ್ಕ Pos = 0;
    ಯಾವತ್ (N) {
      ಯದಿ (ಸೇರಿಸು_Separators && Pos % Grouping == 0 && Pos > 0)
        *--BufPtr = '\'';
      *--BufPtr = Digits[N % Radix];
      N /= Radix;
      Pos++;
    }
    Str.append(BufPtr, std::end(Buffer));
    ಪ್ರತ್ಯಾಗಮ;
  }

  APInt Tmp(*ಏತತ್);

  ಯದಿ (Signed && isNegative()) {
    // They want to print the signed version and it is a negative value
    // Flip the bits and add one to turn it into the equivalent positive
    // value and put a '-' in the result.
    Tmp.negate();
    Str.ತಳ್ಳು_back('-');
  }

  ಯಾವತ್ (*Prefix) {
    Str.ತಳ್ಳು_back(*Prefix);
    ++Prefix;
  }

  // We insert the digits backward, then reverse them to get the right order.
  unsigned StartDig = Str.size();

  // For the 2, 8 and 16 bit cases, we can just shift instead of divide
  // because the number of bits per digit (1, 3 and 4 respectively) divides
  // equally.  We just shift until the value is zero.
  ಯದಿ (Radix == 2 || Radix == 8 || Radix == 16) {
    // Just shift tmp right for each digit width until it becomes zero
    unsigned ShiftAmt = (Radix == 16 ? 4 : (Radix == 8 ? 3 : 1));
    unsigned MaskAmt = Radix - 1;

    ಪೂರ್ಣಾಙ್ಕ Pos = 0;
    ಯಾವತ್ (Tmp.get_Bool_ಮೌಲ್ಯ()) {
      unsigned Digit = unsigned(Tmp.getRawData()[0]) & MaskAmt;
      ಯದಿ (ಸೇರಿಸು_Separators && Pos % Grouping == 0 && Pos > 0)
        Str.ತಳ್ಳು_back('\'');

      Str.ತಳ್ಳು_back(Digits[Digit]);
      Tmp.lshrInPlace(ShiftAmt);
      Pos++;
    }
  } ಅನ್ಯಥಾ {
    ಪೂರ್ಣಾಙ್ಕ Pos = 0;
    ಯಾವತ್ (Tmp.get_Bool_ಮೌಲ್ಯ()) {
      uint64_t Digit;
      udivrem(Tmp, Radix, Tmp, Digit);
      ಪ್ರತಿಜ್ಞಾ(Digit < Radix && "divide failed");
      ಯದಿ (ಸೇರಿಸು_Separators && Pos % Grouping == 0 && Pos > 0)
        Str.ತಳ್ಳು_back('\'');

      Str.ತಳ್ಳು_back(Digits[Digit]);
      Pos++;
    }
  }

  // Reverse the digits before returning.
  std::reverse(Str.begin()+StartDig, Str.end());
}

#ಯದಿ !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)
LLVM_DUMP_METHOD ಶೂನ್ಯ APInt::dump() ಧ್ರುವ {
  SmallString<40> S, U;
  ಏತತ್->toStringUnsigned(U);
  ಏತತ್->toStringSigned(S);
  dbgs() << "APInt(" << BitWidth << "b, "
         << U << "u " << S << "s)\n";
}
#endif

ಶೂನ್ಯ APInt::print(raw_ostream &OS, ತಾರ್ಕಿಕ isSigned) ಧ್ರುವ {
  SmallString<40> S;
  ಏತತ್->toString(S, 10, isSigned, /* formatAsCLiteral = */ಅಸತ್ಯ);
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
ಸ್ಥಿರ ಅನ್ತಃಸ್ಥ APInt::WordType ಕೆಳಗಿನ_Bit_Mask(unsigned bits) {
  ಪ್ರತಿಜ್ಞಾ(bits != 0 && bits <= APInt::APINT_BITS_PER_WORD);
  ಪ್ರತ್ಯಾಗಮ ~(APInt::WordType) 0 >> (APInt::APINT_BITS_PER_WORD - bits);
}

/// Returns the value of the lower half of PART.
ಸ್ಥಿರ ಅನ್ತಃಸ್ಥ APInt::WordType ಕೆಳಗಿನ_Half(APInt::WordType part) {
  ಪ್ರತ್ಯಾಗಮ part & ಕೆಳಗಿನ_Bit_Mask(APInt::APINT_BITS_PER_WORD / 2);
}

/// Returns the value of the upper half of PART.
ಸ್ಥಿರ ಅನ್ತಃಸ್ಥ APInt::WordType ಮೇಲಿನ_Half(APInt::WordType part) {
  ಪ್ರತ್ಯಾಗಮ part >> (APInt::APINT_BITS_PER_WORD / 2);
}

/// Sets the least significant part of a bignum to the input value, and zeroes
/// out higher parts.
ಶೂನ್ಯ APInt::tcSet(WordType *dst, WordType part, unsigned parts) {
  ಪ್ರತಿಜ್ಞಾ(parts > 0);
  dst[0] = part;
  ಪ್ರತಿ (unsigned i = 1; i < parts; i++)
    dst[i] = 0;
}

/// Assign one bignum to another.
ಶೂನ್ಯ APInt::tcAssign(WordType *dst, ಧ್ರುವ WordType *src, unsigned parts) {
  ಪ್ರತಿ (unsigned i = 0; i < parts; i++)
    dst[i] = src[i];
}

/// Returns true if a bignum is zero, false otherwise.
ತಾರ್ಕಿಕ APInt::tcIsZero(ಧ್ರುವ WordType *src, unsigned parts) {
  ಪ್ರತಿ (unsigned i = 0; i < parts; i++)
    ಯದಿ (src[i])
      ಪ್ರತ್ಯಾಗಮ ಅಸತ್ಯ;

  ಪ್ರತ್ಯಾಗಮ ಸತ್ಯ;
}

/// Extract the given bit of a bignum; returns 0 or 1.
ಪೂರ್ಣಾಙ್ಕ APInt::tcExtractBit(ಧ್ರುವ WordType *parts, unsigned bit) {
  ಪ್ರತ್ಯಾಗಮ (parts[whichWord(bit)] & maskBit(bit)) != 0;
}

/// Set the given bit of a bignum.
ಶೂನ್ಯ APInt::tcSetBit(WordType *parts, unsigned bit) {
  parts[whichWord(bit)] |= maskBit(bit);
}

/// Clears the given bit of a bignum.
ಶೂನ್ಯ APInt::tcClearBit(WordType *parts, unsigned bit) {
  parts[whichWord(bit)] &= ~maskBit(bit);
}

/// Returns the bit number of the least significant set bit of a number.  If the
/// input number has no bits set UINT_MAX is returned.
unsigned APInt::tcLSB(ಧ್ರುವ WordType *parts, unsigned n) {
  ಪ್ರತಿ (unsigned i = 0; i < n; i++) {
    ಯದಿ (parts[i] != 0) {
      unsigned lsb = llvm::countr_zero(parts[i]);
      ಪ್ರತ್ಯಾಗಮ lsb + i * APINT_BITS_PER_WORD;
    }
  }

  ಪ್ರತ್ಯಾಗಮ UINT_MAX;
}

/// Returns the bit number of the most significant set bit of a number.
/// If the input number has no bits set UINT_MAX is returned.
unsigned APInt::tcMSB(ಧ್ರುವ WordType *parts, unsigned n) {
  ಕುರು {
    --n;

    ಯದಿ (parts[n] != 0) {
      static_assert(ಪರಿಮಾಣ(parts[n]) <= ಪರಿಮಾಣ(uint64_t));
      unsigned msb = llvm::Log2_64(parts[n]);

      ಪ್ರತ್ಯಾಗಮ msb + n * APINT_BITS_PER_WORD;
    }
  } ಯಾವತ್ (n);

  ಪ್ರತ್ಯಾಗಮ UINT_MAX;
}

/// Copy the bit vector of width srcBITS from SRC, starting at bit srcLSB, to
/// DST, of dstCOUNT parts, such that the bit srcLSB becomes the least
/// significant bit of DST.  All high bits above srcBITS in DST are zero-filled.
/// */
ಶೂನ್ಯ
APInt::tcExtract(WordType *dst, unsigned dst_ಎಣಿಕೆ, ಧ್ರುವ WordType *src,
                 unsigned srcBits, unsigned srcLSB) {
  unsigned dstParts = (srcBits + APINT_BITS_PER_WORD - 1) / APINT_BITS_PER_WORD;
  ಪ್ರತಿಜ್ಞಾ(dstParts <= dst_ಎಣಿಕೆ);

  unsigned firstSrcPart = srcLSB / APINT_BITS_PER_WORD;
  tcAssign(dst, src + firstSrcPart, dstParts);

  unsigned shift = srcLSB % APINT_BITS_PER_WORD;
  tc_Shift_ಬಲ(dst, dstParts, shift);

  // We now have (dstParts * APINT_BITS_PER_WORD - shift) bits from SRC
  // in DST.  If this is less that srcBits, append the rest, else
  // clear the high bits.
  unsigned n = dstParts * APINT_BITS_PER_WORD - shift;
  ಯದಿ (n < srcBits) {
    WordType mask = ಕೆಳಗಿನ_Bit_Mask (srcBits - n);
    dst[dstParts - 1] |= ((src[firstSrcPart + dstParts] & mask)
                          << n % APINT_BITS_PER_WORD);
  } ಅನ್ಯಥಾ ಯದಿ (n > srcBits) {
    ಯದಿ (srcBits % APINT_BITS_PER_WORD)
      dst[dstParts - 1] &= ಕೆಳಗಿನ_Bit_Mask (srcBits % APINT_BITS_PER_WORD);
  }

  // Clear high parts.
  ಯಾವತ್ (dstParts < dst_ಎಣಿಕೆ)
    dst[dstParts++] = 0;
}

//// DST += RHS + C where C is zero or one.  Returns the carry flag.
APInt::WordType APInt::tcAdd(WordType *dst, ಧ್ರುವ WordType *rhs,
                             WordType c, unsigned parts) {
  ಪ್ರತಿಜ್ಞಾ(c <= 1);

  ಪ್ರತಿ (unsigned i = 0; i < parts; i++) {
    WordType l = dst[i];
    ಯದಿ (c) {
      dst[i] += rhs[i] + 1;
      c = (dst[i] <= l);
    } ಅನ್ಯಥಾ {
      dst[i] += rhs[i];
      c = (dst[i] < l);
    }
  }

  ಪ್ರತ್ಯಾಗಮ c;
}

/// This function adds a single "word" integer, src, to the multiple
/// "word" integer array, dst[]. dst[] is modified to reflect the addition and
/// 1 is returned if there is a carry out, otherwise 0 is returned.
/// @returns the carry of the addition.
APInt::WordType APInt::tcAddPart(WordType *dst, WordType src,
                                 unsigned parts) {
  ಪ್ರತಿ (unsigned i = 0; i < parts; ++i) {
    dst[i] += src;
    ಯದಿ (dst[i] >= src)
      ಪ್ರತ್ಯಾಗಮ 0; // No need to carry so exit early.
    src = 1; // Carry one to next digit.
  }

  ಪ್ರತ್ಯಾಗಮ 1;
}

/// DST -= RHS + C where C is zero or one.  Returns the carry flag.
APInt::WordType APInt::tcSubtract(WordType *dst, ಧ್ರುವ WordType *rhs,
                                  WordType c, unsigned parts) {
  ಪ್ರತಿಜ್ಞಾ(c <= 1);

  ಪ್ರತಿ (unsigned i = 0; i < parts; i++) {
    WordType l = dst[i];
    ಯದಿ (c) {
      dst[i] -= rhs[i] + 1;
      c = (dst[i] >= l);
    } ಅನ್ಯಥಾ {
      dst[i] -= rhs[i];
      c = (dst[i] > l);
    }
  }

  ಪ್ರತ್ಯಾಗಮ c;
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
  ಪ್ರತಿ (unsigned i = 0; i < parts; ++i) {
    WordType Dst = dst[i];
    dst[i] -= src;
    ಯದಿ (src <= Dst)
      ಪ್ರತ್ಯಾಗಮ 0; // No need to borrow so exit early.
    src = 1; // We have to "borrow 1" from next "word"
  }

  ಪ್ರತ್ಯಾಗಮ 1;
}

/// Negate a bignum in-place.
ಶೂನ್ಯ APInt::tcNegate(WordType *dst, unsigned parts) {
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
ಪೂರ್ಣಾಙ್ಕ APInt::tcMultiplyPart(WordType *dst, ಧ್ರುವ WordType *src,
                          WordType multiplier, WordType carry,
                          unsigned srcParts, unsigned dstParts,
                          ತಾರ್ಕಿಕ add) {
  // Otherwise our writes of DST kill our later reads of SRC.
  ಪ್ರತಿಜ್ಞಾ(dst <= src || dst >= src + srcParts);
  ಪ್ರತಿಜ್ಞಾ(dstParts <= srcParts + 1);

  // N loops; minimum of dstParts and srcParts.
  unsigned n = std::min(dstParts, srcParts);

  ಪ್ರತಿ (unsigned i = 0; i < n; i++) {
    // [LOW, HIGH] = MULTIPLIER * SRC[i] + DST[i] + CARRY.
    // This cannot overflow, because:
    //   (n - 1) * (n - 1) + 2 (n - 1) = (n - 1) * (n + 1)
    // which is less than n^2.
    WordType srcPart = src[i];
    WordType ಕೆಳಗಿನ, mid, ಮೇಲಿನ;
    ಯದಿ (multiplier == 0 || srcPart == 0) {
      ಕೆಳಗಿನ = carry;
      ಮೇಲಿನ = 0;
    } ಅನ್ಯಥಾ {
      ಕೆಳಗಿನ = ಕೆಳಗಿನ_Half(srcPart) * ಕೆಳಗಿನ_Half(multiplier);
      ಮೇಲಿನ = ಮೇಲಿನ_Half(srcPart) * ಮೇಲಿನ_Half(multiplier);

      mid = ಕೆಳಗಿನ_Half(srcPart) * ಮೇಲಿನ_Half(multiplier);
      ಮೇಲಿನ += ಮೇಲಿನ_Half(mid);
      mid <<= APINT_BITS_PER_WORD / 2;
      ಯದಿ (ಕೆಳಗಿನ + mid < ಕೆಳಗಿನ)
        ಮೇಲಿನ++;
      ಕೆಳಗಿನ += mid;

      mid = ಮೇಲಿನ_Half(srcPart) * ಕೆಳಗಿನ_Half(multiplier);
      ಮೇಲಿನ += ಮೇಲಿನ_Half(mid);
      mid <<= APINT_BITS_PER_WORD / 2;
      ಯದಿ (ಕೆಳಗಿನ + mid < ಕೆಳಗಿನ)
        ಮೇಲಿನ++;
      ಕೆಳಗಿನ += mid;

      // Now add carry.
      ಯದಿ (ಕೆಳಗಿನ + carry < ಕೆಳಗಿನ)
        ಮೇಲಿನ++;
      ಕೆಳಗಿನ += carry;
    }

    ಯದಿ (add) {
      // And now DST[i], and store the new low part there.
      ಯದಿ (ಕೆಳಗಿನ + dst[i] < ಕೆಳಗಿನ)
        ಮೇಲಿನ++;
      dst[i] += ಕೆಳಗಿನ;
    } ಅನ್ಯಥಾ {
      dst[i] = ಕೆಳಗಿನ;
    }

    carry = ಮೇಲಿನ;
  }

  ಯದಿ (srcParts < dstParts) {
    // Full multiplication, there is no overflow.
    ಪ್ರತಿಜ್ಞಾ(srcParts + 1 == dstParts);
    dst[srcParts] = carry;
    ಪ್ರತ್ಯಾಗಮ 0;
  }

  // We overflowed if there is carry.
  ಯದಿ (carry)
    ಪ್ರತ್ಯಾಗಮ 1;

  // We would overflow if any significant unwritten parts would be
  // non-zero.  This is true if any remaining src parts are non-zero
  // and the multiplier is non-zero.
  ಯದಿ (multiplier)
    ಪ್ರತಿ (unsigned i = dstParts; i < srcParts; i++)
      ಯದಿ (src[i])
        ಪ್ರತ್ಯಾಗಮ 1;

  // We fitted in the narrow destination.
  ಪ್ರತ್ಯಾಗಮ 0;
}

/// DST = LHS * RHS, where DST has the same width as the operands and
/// is filled with the least significant parts of the result.  Returns
/// one if overflow occurred, otherwise zero.  DST must be disjoint
/// from both operands.
ಪೂರ್ಣಾಙ್ಕ APInt::tcMultiply(WordType *dst, ಧ್ರುವ WordType *lhs,
                      ಧ್ರುವ WordType *rhs, unsigned parts) {
  ಪ್ರತಿಜ್ಞಾ(dst != lhs && dst != rhs);

  ಪೂರ್ಣಾಙ್ಕ overflow = 0;

  ಪ್ರತಿ (unsigned i = 0; i < parts; i++) {
    // Don't accumulate on the first iteration so we don't need to initalize
    // dst to 0.
    overflow |=
        tcMultiplyPart(&dst[i], lhs, rhs[i], 0, parts, parts - i, i != 0);
  }

  ಪ್ರತ್ಯಾಗಮ overflow;
}

/// DST = LHS * RHS, where DST has width the sum of the widths of the
/// operands. No overflow occurs. DST must be disjoint from both operands.
ಶೂನ್ಯ APInt::tcFullMultiply(WordType *dst, ಧ್ರುವ WordType *lhs,
                           ಧ್ರುವ WordType *rhs, unsigned lhsParts,
                           unsigned rhsParts) {
  // Put the narrower number on the LHS for less loops below.
  ಯದಿ (lhsParts > rhsParts)
    ಪ್ರತ್ಯಾಗಮ tcFullMultiply (dst, rhs, lhs, rhsParts, lhsParts);

  ಪ್ರತಿಜ್ಞಾ(dst != lhs && dst != rhs);

  ಪ್ರತಿ (unsigned i = 0; i < lhsParts; i++) {
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
ಪೂರ್ಣಾಙ್ಕ APInt::tcDivide(WordType *lhs, ಧ್ರುವ WordType *rhs,
                    WordType *remainder, WordType *srhs,
                    unsigned parts) {
  ಪ್ರತಿಜ್ಞಾ(lhs != remainder && lhs != srhs && remainder != srhs);

  unsigned shift_ಎಣಿಕೆ = tcMSB(rhs, parts) + 1;
  ಯದಿ (shift_ಎಣಿಕೆ == 0)
    ಪ್ರತ್ಯಾಗಮ ಸತ್ಯ;

  shift_ಎಣಿಕೆ = parts * APINT_BITS_PER_WORD - shift_ಎಣಿಕೆ;
  unsigned n = shift_ಎಣಿಕೆ / APINT_BITS_PER_WORD;
  WordType mask = (WordType) 1 << (shift_ಎಣಿಕೆ % APINT_BITS_PER_WORD);

  tcAssign(srhs, rhs, parts);
  tc_Shift_ಎಡ(srhs, parts, shift_ಎಣಿಕೆ);
  tcAssign(remainder, lhs, parts);
  tcSet(lhs, 0, parts);

  // Loop, subtracting SRHS if REMAINDER is greater and adding that to the
  // total.
  ಪ್ರತಿ (;;) {
    ಪೂರ್ಣಾಙ್ಕ compare = tcCompare(remainder, srhs, parts);
    ಯದಿ (compare >= 0) {
      tcSubtract(remainder, srhs, 0, parts);
      lhs[n] |= mask;
    }

    ಯದಿ (shift_ಎಣಿಕೆ == 0)
      ಭಙ್ಗ;
    shift_ಎಣಿಕೆ--;
    tc_Shift_ಬಲ(srhs, parts, 1);
    ಯದಿ ((mask >>= 1) == 0) {
      mask = (WordType) 1 << (APINT_BITS_PER_WORD - 1);
      n--;
    }
  }

  ಪ್ರತ್ಯಾಗಮ ಅಸತ್ಯ;
}

/// Shift a bignum left Count bits in-place. Shifted in bits are zero. There are
/// no restrictions on Count.
ಶೂನ್ಯ APInt::tc_Shift_ಎಡ(WordType *Dst, unsigned ಪದಗಳು_2, unsigned ಎಣಿಕೆ_2) {
  // Don't bother performing a no-op shift.
  ಯದಿ (!ಎಣಿಕೆ_2)
    ಪ್ರತ್ಯಾಗಮ;

  // WordShift is the inter-part shift; BitShift is the intra-part shift.
  unsigned WordShift = std::min(ಎಣಿಕೆ_2 / APINT_BITS_PER_WORD, ಪದಗಳು_2);
  unsigned BitShift = ಎಣಿಕೆ_2 % APINT_BITS_PER_WORD;

  // Fastpath for moving by whole words.
  ಯದಿ (BitShift == 0) {
    std::memmove(Dst + WordShift, Dst, (ಪದಗಳು_2 - WordShift) * APINT_WORD_SIZE);
  } ಅನ್ಯಥಾ {
    ಯಾವತ್ (ಪದಗಳು_2-- > WordShift) {
      Dst[ಪದಗಳು_2] = Dst[ಪದಗಳು_2 - WordShift] << BitShift;
      ಯದಿ (ಪದಗಳು_2 > WordShift)
        Dst[ಪದಗಳು_2] |=
          Dst[ಪದಗಳು_2 - WordShift - 1] >> (APINT_BITS_PER_WORD - BitShift);
    }
  }

  // Fill in the remainder with 0s.
  std::memset(Dst, 0, WordShift * APINT_WORD_SIZE);
}

/// Shift a bignum right Count bits in-place. Shifted in bits are zero. There
/// are no restrictions on Count.
ಶೂನ್ಯ APInt::tc_Shift_ಬಲ(WordType *Dst, unsigned ಪದಗಳು_2, unsigned ಎಣಿಕೆ_2) {
  // Don't bother performing a no-op shift.
  ಯದಿ (!ಎಣಿಕೆ_2)
    ಪ್ರತ್ಯಾಗಮ;

  // WordShift is the inter-part shift; BitShift is the intra-part shift.
  unsigned WordShift = std::min(ಎಣಿಕೆ_2 / APINT_BITS_PER_WORD, ಪದಗಳು_2);
  unsigned BitShift = ಎಣಿಕೆ_2 % APINT_BITS_PER_WORD;

  unsigned ಪದಗಳು_To_Move = ಪದಗಳು_2 - WordShift;
  // Fastpath for moving by whole words.
  ಯದಿ (BitShift == 0) {
    std::memmove(Dst, Dst + WordShift, ಪದಗಳು_To_Move * APINT_WORD_SIZE);
  } ಅನ್ಯಥಾ {
    ಪ್ರತಿ (unsigned i = 0; i != ಪದಗಳು_To_Move; ++i) {
      Dst[i] = Dst[i + WordShift] >> BitShift;
      ಯದಿ (i + 1 != ಪದಗಳು_To_Move)
        Dst[i] |= Dst[i + WordShift + 1] << (APINT_BITS_PER_WORD - BitShift);
    }
  }

  // Fill in the remainder with 0s.
  std::memset(Dst + ಪದಗಳು_To_Move, 0, WordShift * APINT_WORD_SIZE);
}

// Comparison (unsigned) of two bignums.
ಪೂರ್ಣಾಙ್ಕ APInt::tcCompare(ಧ್ರುವ WordType *lhs, ಧ್ರುವ WordType *rhs,
                     unsigned parts) {
  ಯಾವತ್ (parts) {
    parts--;
    ಯದಿ (lhs[parts] != rhs[parts])
      ಪ್ರತ್ಯಾಗಮ (lhs[parts] > rhs[parts]) ? 1 : -1;
  }

  ಪ್ರತ್ಯಾಗಮ 0;
}

APInt llvm::APIntOps::RoundingUDiv(ಧ್ರುವ APInt &A, ಧ್ರುವ APInt &B,
                                   APInt::Rounding RM) {
  // Currently udivrem always rounds down.
  ವಿಕಲ್ಪ (RM) {
  ಪಕ್ಷ APInt::Rounding::DOWN:
  ಪಕ್ಷ APInt::Rounding::TOWARD_ZERO:
    ಪ್ರತ್ಯಾಗಮ A.udiv(B);
  ಪಕ್ಷ APInt::Rounding::UP: {
    APInt Quo, Rem;
    APInt::udivrem(A, B, Quo, Rem);
    ಯದಿ (Rem.isZero())
      ಪ್ರತ್ಯಾಗಮ Quo;
    ಪ್ರತ್ಯಾಗಮ Quo + 1;
  }
  }
  llvm_unreachable("Unknown APInt::Rounding enum");
}

APInt llvm::APIntOps::RoundingSDiv(ಧ್ರುವ APInt &A, ಧ್ರುವ APInt &B,
                                   APInt::Rounding RM) {
  ವಿಕಲ್ಪ (RM) {
  ಪಕ್ಷ APInt::Rounding::DOWN:
  ಪಕ್ಷ APInt::Rounding::UP: {
    APInt Quo, Rem;
    APInt::sdivrem(A, B, Quo, Rem);
    ಯದಿ (Rem.isZero())
      ಪ್ರತ್ಯಾಗಮ Quo;
    // This algorithm deals with arbitrary rounding mode used by sdivrem.
    // We want to check whether the non-integer part of the mathematical value
    // is negative or not. If the non-integer part is negative, we need to round
    // down from Quo; otherwise, if it's positive or 0, we return Quo, as it's
    // already rounded down.
    ಯದಿ (RM == APInt::Rounding::DOWN) {
      ಯದಿ (Rem.isNegative() != B.isNegative())
        ಪ್ರತ್ಯಾಗಮ Quo - 1;
      ಪ್ರತ್ಯಾಗಮ Quo;
    }
    ಯದಿ (Rem.isNegative() != B.isNegative())
      ಪ್ರತ್ಯಾಗಮ Quo;
    ಪ್ರತ್ಯಾಗಮ Quo + 1;
  }
  // Currently sdiv rounds towards zero.
  ಪಕ್ಷ APInt::Rounding::TOWARD_ZERO:
    ಪ್ರತ್ಯಾಗಮ A.sdiv(B);
  }
  llvm_unreachable("Unknown APInt::Rounding enum");
}

std::optional<APInt>
llvm::APIntOps::SolveQuadraticEquationWrap(APInt A, APInt B, APInt C,
                                           unsigned RangeWidth) {
  unsigned CoeffWidth = A.getBitWidth();
  ಪ್ರತಿಜ್ಞಾ(CoeffWidth == B.getBitWidth() && CoeffWidth == C.getBitWidth());
  ಪ್ರತಿಜ್ಞಾ(RangeWidth <= CoeffWidth &&
         "Value range width should be less than coefficient width");
  ಪ್ರತಿಜ್ಞಾ(RangeWidth > 1 && "Value range bit width should be > 1");

  LLVM_DEBUG(dbgs() << __func__ << ": solving " << A << "x^2 + " << B
                    << "x + " << C << ", rw:" << RangeWidth << '\n');

  // Identify 0 as a (non)solution immediately.
  ಯದಿ (C.sextOrTrunc(RangeWidth).isZero()) {
    LLVM_DEBUG(dbgs() << __func__ << ": zero solution\n");
    ಪ್ರತ್ಯಾಗಮ APInt(CoeffWidth, 0);
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
  ಯದಿ (A.isNegative()) {
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
  ತಾರ್ಕಿಕ Pick_ಕೆಳಗಿನ;

  auto RoundUp = [] (ಧ್ರುವ APInt &V, ಧ್ರುವ APInt &A) -> APInt {
    ಪ್ರತಿಜ್ಞಾ(A.isStrictlyPositive());
    APInt T = V.abs().urem(A);
    ಯದಿ (T.isZero())
      ಪ್ರತ್ಯಾಗಮ V;
    ಪ್ರತ್ಯಾಗಮ V.isNegative() ? V+T : V+(A-T);
  };

  // The vertex of the parabola is at -B/2A, but since A > 0, it's negative
  // iff B is positive.
  ಯದಿ (B.isNonNegative()) {
    // If B >= 0, the vertex it at a negative location (or at 0), so in
    // order to have a non-negative solution we need to pick k that makes
    // C-kR negative. To satisfy all the requirements for the solution
    // that we are looking for, it needs to be closest to 0 of all k.
    C = C.srem(R);
    ಯದಿ (C.isStrictlyPositive())
      C -= R;
    // Pick the greater solution.
    Pick_ಕೆಳಗಿನ = ಅಸತ್ಯ;
  } ಅನ್ಯಥಾ {
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
    ಯದಿ (C.sgt(LowkR)) {
      // If LowkR < C, then such a k is guaranteed to exist because
      // LowkR itself is a multiple of R.
      C -= -RoundUp(-C, R);      // C = C - RoundDown(C, R)
      // Pick the smaller solution.
      Pick_ಕೆಳಗಿನ = ಸತ್ಯ;
    } ಅನ್ಯಥಾ {
      // If C-kR < 0 for all potential k's, it means that one solution
      // will be negative, while the other will be positive. The positive
      // solution will shift towards 0 if the parabola is moved up.
      // Pick the kR closest to the lower bound (i.e. make C-kR closest
      // to 0, or in other words, out of all parabolas that have solutions,
      // pick the one that is the farthest "up").
      // Since LowkR is itself a multiple of R, simply take C-LowkR.
      C -= LowkR;
      // Pick the greater solution.
      Pick_ಕೆಳಗಿನ = ಅಸತ್ಯ;
    }
  }

  LLVM_DEBUG(dbgs() << __func__ << ": updated coefficients " << A << "x^2 + "
                    << B << "x + " << C << ", rw:" << RangeWidth << '\n');

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
  ಯದಿ (Pick_ಕೆಳಗಿನ)
    APInt::sdivrem(-B - (SQ+InexactSQ), TwoA, X, Rem);
  ಅನ್ಯಥಾ
    APInt::sdivrem(-B + SQ, TwoA, X, Rem);

  // The updated coefficients should be such that the (exact) solution is
  // positive. Since APInt division rounds towards 0, the calculated one
  // can be 0, but cannot be negative.
  ಪ್ರತಿಜ್ಞಾ(X.isNonNegative() && "Solution should be non-negative");

  ಯದಿ (!InexactSQ && Rem.isZero()) {
    LLVM_DEBUG(dbgs() << __func__ << ": solution (root): " << X << '\n');
    ಪ್ರತ್ಯಾಗಮ X;
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
  ಯದಿ (!SignChange) {
    LLVM_DEBUG(dbgs() << __func__ << ": no valid solution\n");
    ಪ್ರತ್ಯಾಗಮ std::nullopt;
  }

  X += 1;
  LLVM_DEBUG(dbgs() << __func__ << ": solution (wrap): " << X << '\n');
  ಪ್ರತ್ಯಾಗಮ X;
}

std::optional<unsigned>
llvm::APIntOps::GetMostSignificantDifferentBit(ಧ್ರುವ APInt &A, ಧ್ರುವ APInt &B) {
  ಪ್ರತಿಜ್ಞಾ(A.getBitWidth() == B.getBitWidth() && "Must have the same bitwidth");
  ಯದಿ (A == B)
    ಪ್ರತ್ಯಾಗಮ std::nullopt;
  ಪ್ರತ್ಯಾಗಮ A.getBitWidth() - ((A ^ B).countl_zero() + 1);
}

APInt llvm::APIntOps::ScaleBitMask(ಧ್ರುವ APInt &A, unsigned NewBitWidth,
                                   ತಾರ್ಕಿಕ MatchAllBits) {
  unsigned OldBitWidth = A.getBitWidth();
  ಪ್ರತಿಜ್ಞಾ((((OldBitWidth % NewBitWidth) == 0) ||
          ((NewBitWidth % OldBitWidth) == 0)) &&
         "One size should be a multiple of the other one. "
         "Can't do fractional scaling.");

  // Check for matching bitwidths.
  ಯದಿ (OldBitWidth == NewBitWidth)
    ಪ್ರತ್ಯಾಗಮ A;

  APInt NewA = APInt::getZero(NewBitWidth);

  // Check for null input.
  ಯದಿ (A.isZero())
    ಪ್ರತ್ಯಾಗಮ NewA;

  ಯದಿ (NewBitWidth > OldBitWidth) {
    // Repeat bits.
    unsigned Scale = NewBitWidth / OldBitWidth;
    ಪ್ರತಿ (unsigned i = 0; i != OldBitWidth; ++i)
      ಯದಿ (A[i])
        NewA.setBits(i * Scale, (i + 1) * Scale);
  } ಅನ್ಯಥಾ {
    unsigned Scale = OldBitWidth / NewBitWidth;
    ಪ್ರತಿ (unsigned i = 0; i != NewBitWidth; ++i) {
      ಯದಿ (MatchAllBits) {
        ಯದಿ (A.extractBits(Scale, i * Scale).isAllOnes())
          NewA.setBit(i);
      } ಅನ್ಯಥಾ {
        ಯದಿ (!A.extractBits(Scale, i * Scale).isZero())
          NewA.setBit(i);
      }
    }
  }

  ಪ್ರತ್ಯಾಗಮ NewA;
}

/// StoreIntToMemory - Fills the StoreBytes bytes of memory starting from Dst
/// with the integer held in IntVal.
ಶೂನ್ಯ llvm::StoreIntToMemory(ಧ್ರುವ APInt &IntVal, uint8_t *Dst,
                            unsigned StoreBytes) {
  ಪ್ರತಿಜ್ಞಾ((IntVal.getBitWidth()+7)/8 >= StoreBytes && "Integer too small!");
  ಧ್ರುವ uint8_t *Src = (ಧ್ರುವ uint8_t *)IntVal.getRawData();

  ಯದಿ (sys::IsLittleEndianHost) {
    // Little-endian host - the source is ordered from LSB to MSB.  Order the
    // destination from LSB to MSB: Do a straight copy.
    memcpy(Dst, Src, StoreBytes);
  } ಅನ್ಯಥಾ {
    // Big-endian host - the source is an array of 64 bit words ordered from
    // LSW to MSW.  Each word is ordered from MSB to LSB.  Order the destination
    // from MSB to LSB: Reverse the word order, but not the bytes in a word.
    ಯಾವತ್ (StoreBytes > ಪರಿಮಾಣ(uint64_t)) {
      StoreBytes -= ಪರಿಮಾಣ(uint64_t);
      // May not be aligned so use memcpy.
      memcpy(Dst + StoreBytes, Src, ಪರಿಮಾಣ(uint64_t));
      Src += ಪರಿಮಾಣ(uint64_t);
    }

    memcpy(Dst, Src + ಪರಿಮಾಣ(uint64_t) - StoreBytes, StoreBytes);
  }
}

/// LoadIntFromMemory - Loads the integer stored in the LoadBytes bytes starting
/// from Src into IntVal, which is assumed to be wide enough and to hold zero.
ಶೂನ್ಯ llvm::LoadIntFromMemory(APInt &IntVal, ಧ್ರುವ uint8_t *Src,
                             unsigned LoadBytes) {
  ಪ್ರತಿಜ್ಞಾ((IntVal.getBitWidth()+7)/8 >= LoadBytes && "Integer too small!");
  uint8_t *Dst = reinterpret_cast<uint8_t *>(
                   const_cast<uint64_t *>(IntVal.getRawData()));

  ಯದಿ (sys::IsLittleEndianHost)
    // Little-endian host - the destination must be ordered from LSB to MSB.
    // The source is ordered from LSB to MSB: Do a straight copy.
    memcpy(Dst, Src, LoadBytes);
  ಅನ್ಯಥಾ {
    // Big-endian - the destination is an array of 64 bit words ordered from
    // LSW to MSW.  Each word must be ordered from MSB to LSB.  The source is
    // ordered from MSB to LSB: Reverse the word order, but not the bytes in
    // a word.
    ಯಾವತ್ (LoadBytes > ಪರಿಮಾಣ(uint64_t)) {
      LoadBytes -= ಪರಿಮಾಣ(uint64_t);
      // May not be aligned so use memcpy.
      memcpy(Dst, Src + LoadBytes, ಪರಿಮಾಣ(uint64_t));
      Dst += ಪರಿಮಾಣ(uint64_t);
    }

    memcpy(Dst + ಪರಿಮಾಣ(uint64_t) - LoadBytes, Src, LoadBytes);
  }
}

APInt APIntOps::avgFloorS(ಧ್ರುವ APInt &C1, ಧ್ರುವ APInt &C2) {
  // Return floor((C1 + C2) / 2)
  ಪ್ರತ್ಯಾಗಮ (C1 & C2) + (C1 ^ C2).ashr(1);
}

APInt APIntOps::avgFloorU(ಧ್ರುವ APInt &C1, ಧ್ರುವ APInt &C2) {
  // Return floor((C1 + C2) / 2)
  ಪ್ರತ್ಯಾಗಮ (C1 & C2) + (C1 ^ C2).lshr(1);
}

APInt APIntOps::avgCeilS(ಧ್ರುವ APInt &C1, ಧ್ರುವ APInt &C2) {
  // Return ceil((C1 + C2) / 2)
  ಪ್ರತ್ಯಾಗಮ (C1 | C2) - (C1 ^ C2).ashr(1);
}

APInt APIntOps::avgCeilU(ಧ್ರುವ APInt &C1, ಧ್ರುವ APInt &C2) {
  // Return ceil((C1 + C2) / 2)
  ಪ್ರತ್ಯಾಗಮ (C1 | C2) - (C1 ^ C2).lshr(1);
}

APInt APIntOps::avgSubFloorS(ಧ್ರುವ APInt &C1, ಧ್ರುವ APInt &C2) {
  // Return floor((C1 - C2) / 2), using C1 - C2 == (C1 ^ C2) - 2 * (~C1 & C2).
  ಪ್ರತ್ಯಾಗಮ (C1 ^ C2).ashr(1) - (~C1 & C2);
}

APInt APIntOps::avgSubFloorU(ಧ್ರುವ APInt &C1, ಧ್ರುವ APInt &C2) {
  // Return floor((C1 - C2) / 2), using C1 - C2 == (C1 ^ C2) - 2 * (~C1 & C2).
  ಪ್ರತ್ಯಾಗಮ (C1 ^ C2).lshr(1) - (~C1 & C2);
}

APInt APIntOps::mulhs(ಧ್ರುವ APInt &C1, ಧ್ರುವ APInt &C2) {
  ಪ್ರತಿಜ್ಞಾ(C1.getBitWidth() == C2.getBitWidth() && "Unequal bitwidths");
  unsigned FullWidth = C1.getBitWidth() * 2;
  APInt C1Ext = C1.sext(FullWidth);
  APInt C2Ext = C2.sext(FullWidth);
  ಪ್ರತ್ಯಾಗಮ (C1Ext * C2Ext).extractBits(C1.getBitWidth(), C1.getBitWidth());
}

APInt APIntOps::mulhu(ಧ್ರುವ APInt &C1, ಧ್ರುವ APInt &C2) {
  ಪ್ರತಿಜ್ಞಾ(C1.getBitWidth() == C2.getBitWidth() && "Unequal bitwidths");
  unsigned FullWidth = C1.getBitWidth() * 2;
  APInt C1Ext = C1.zext(FullWidth);
  APInt C2Ext = C2.zext(FullWidth);
  ಪ್ರತ್ಯಾಗಮ (C1Ext * C2Ext).extractBits(C1.getBitWidth(), C1.getBitWidth());
}

APInt APIntOps::mulsExtended(ಧ್ರುವ APInt &C1, ಧ್ರುವ APInt &C2) {
  ಪ್ರತಿಜ್ಞಾ(C1.getBitWidth() == C2.getBitWidth() && "Unequal bitwidths");
  unsigned FullWidth = C1.getBitWidth() * 2;
  APInt C1Ext = C1.sext(FullWidth);
  APInt C2Ext = C2.sext(FullWidth);
  ಪ್ರತ್ಯಾಗಮ C1Ext * C2Ext;
}

APInt APIntOps::muluExtended(ಧ್ರುವ APInt &C1, ಧ್ರುವ APInt &C2) {
  ಪ್ರತಿಜ್ಞಾ(C1.getBitWidth() == C2.getBitWidth() && "Unequal bitwidths");
  unsigned FullWidth = C1.getBitWidth() * 2;
  APInt C1Ext = C1.zext(FullWidth);
  APInt C2Ext = C2.zext(FullWidth);
  ಪ್ರತ್ಯಾಗಮ C1Ext * C2Ext;
}

APInt APIntOps::pow(ಧ್ರುವ APInt &X, int64_t N) {
  ಪ್ರತಿಜ್ಞಾ(N >= 0 && "negative exponents not supported.");
  APInt Acc = APInt(X.getBitWidth(), 1);
  ಯದಿ (N == 0)
    ಪ್ರತ್ಯಾಗಮ Acc;
  APInt Base = X;
  int64_t RemainingExponent = N;
  ಯಾವತ್ (RemainingExponent > 0) {
    ಯಾವತ್ (RemainingExponent % 2 == 0) {
      Base *= Base;
      RemainingExponent /= 2;
    }
    --RemainingExponent;
    Acc *= Base;
  }
  ಪ್ರತ್ಯಾಗಮ Acc;
}

APInt llvm::APIntOps::fshl(ಧ್ರುವ APInt &Hi, ಧ್ರುವ APInt &Lo,
                           ಧ್ರುವ APInt &Shift) {
  ಪ್ರತಿಜ್ಞಾ(Hi.getBitWidth() == Lo.getBitWidth());
  unsigned ShiftAmt = rotateModulo(Hi.getBitWidth(), Shift);
  ಯದಿ (ShiftAmt == 0)
    ಪ್ರತ್ಯಾಗಮ Hi;
  ಪ್ರತ್ಯಾಗಮ Hi.shl(ShiftAmt) | Lo.lshr(Hi.getBitWidth() - ShiftAmt);
}

APInt llvm::APIntOps::fshr(ಧ್ರುವ APInt &Hi, ಧ್ರುವ APInt &Lo,
                           ಧ್ರುವ APInt &Shift) {
  ಪ್ರತಿಜ್ಞಾ(Hi.getBitWidth() == Lo.getBitWidth());
  unsigned ShiftAmt = rotateModulo(Hi.getBitWidth(), Shift);
  ಯದಿ (ShiftAmt == 0)
    ಪ್ರತ್ಯಾಗಮ Lo;
  ಪ್ರತ್ಯಾಗಮ Hi.shl(Hi.getBitWidth() - ShiftAmt) | Lo.lshr(ShiftAmt);
}

APInt llvm::APIntOps::clmul(ಧ್ರುವ APInt &LHS, ಧ್ರುವ APInt &RHS) {
  unsigned BW = LHS.getBitWidth();
  ಪ್ರತಿಜ್ಞಾ(BW == RHS.getBitWidth() && "Operand mismatch");
  APInt Result(BW, 0);
  ಪ್ರತಿ (unsigned I : seq(std::min(RHS.getActiveBits(), BW - LHS.countr_zero())))
    ಯದಿ (RHS[I])
      Result ^= LHS << I;
  ಪ್ರತ್ಯಾಗಮ Result;
}

APInt llvm::APIntOps::clmulr(ಧ್ರುವ APInt &LHS, ಧ್ರುವ APInt &RHS) {
  ಪ್ರತಿಜ್ಞಾ(LHS.getBitWidth() == RHS.getBitWidth());
  ಪ್ರತ್ಯಾಗಮ clmul(LHS.reverseBits(), RHS.reverseBits()).reverseBits();
}

APInt llvm::APIntOps::clmulh(ಧ್ರುವ APInt &LHS, ಧ್ರುವ APInt &RHS) {
  ಪ್ರತಿಜ್ಞಾ(LHS.getBitWidth() == RHS.getBitWidth());
  ಪ್ರತ್ಯಾಗಮ clmulr(LHS, RHS).lshr(1);
}

APInt llvm::APIntOps::pext(ಧ್ರುವ APInt &Val, ಧ್ರುವ APInt &Mask) {
  unsigned BW = Val.getBitWidth();
  ಪ್ರತಿಜ್ಞಾ(BW == Mask.getBitWidth() && "Operand mismatch");
  APInt Result = APInt::getZero(BW);
  ಪ್ರತಿ (unsigned I = 0, P = 0; I != BW; ++I)
    ಯದಿ (Mask[I])
      Result.setBitVal(P++, Val[I]);
  ಪ್ರತ್ಯಾಗಮ Result;
}

APInt llvm::APIntOps::pdep(ಧ್ರುವ APInt &Val, ಧ್ರುವ APInt &Mask) {
  unsigned BW = Val.getBitWidth();
  ಪ್ರತಿಜ್ಞಾ(BW == Mask.getBitWidth() && "Operand mismatch");
  APInt Result = APInt::getZero(BW);
  ಪ್ರತಿ (unsigned I = 0, P = 0; I != BW; ++I)
    ಯದಿ (Mask[I])
      Result.setBitVal(I, Val[P++]);
  ಪ್ರತ್ಯಾಗಮ Result;
}
