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

#include "llvm/ADT/APInt.h"
#include "llvm/ADT/ArrayRef.h"
#include "llvm/ADT/FoldingSet.h"
#include "llvm/ADT/Hashing.h"
#include "llvm/ADT/Sequence.h"
#include "llvm/ADT/SmallString.h"
#include "llvm/ADT/StringRef.h"
#include "llvm/ADT/bit.h"
#include "llvm/Support/Alignment.h"
#include "llvm/Support/Debug.h"
#include "llvm/Support/ErrorHandling.h"
#include "llvm/Support/MathExtras.h"
#include "llvm/Support/SwapByteOrder.h"
#include "llvm/Support/raw_ostream.h"
#include <cmath>
#include <optional>

using namespace llvm;

#define DEBUG_TYPE "apint"

/// A utility function for allocating memory, checking for allocation failures,
/// and ensuring the contents are zeroed.
inline static uint64_t* getClearedMemory(unsigned numWords) {
  врати new uint64_t[numWords]();
}

/// A utility function for allocating memory and checking for allocation
/// failure.  The content is not zeroed.
inline static uint64_t* getMemory(unsigned numWords) {
  врати new uint64_t[numWords];
}

/// A utility function that converts a character to a digit.
inline static unsigned getDigit(char cdigit, uint8_t radix) {
  unsigned r;

  ако (radix == 16 || radix == 36) {
    r = cdigit - '0';
    ако (r <= 9)
      врати r;

    r = cdigit - 'A';
    ако (r <= radix - 11U)
      врати r + 10;

    r = cdigit - 'a';
    ако (r <= radix - 11U)
      врати r + 10;

    radix = 10;
  }

  r = cdigit - '0';
  ако (r < radix)
    врати r;

  врати UINT_MAX;
}


void APInt::initSlowCase(uint64_t val, bool isSigned) {
  ако (isSigned && int64_t(val) < 0) {
    U.pVal = getMemory(getNumWords());
    U.pVal[0] = val;
    memset(&U.pVal[1], 0xFF, APINT_WORD_SIZE * (getNumWords() - 1));
    clearUnusedBits();
  } инако {
    U.pVal = getClearedMemory(getNumWords());
    U.pVal[0] = val;
  }
}

void APInt::initSlowCase(const APInt& that) {
  U.pVal = getMemory(getNumWords());
  memcpy(U.pVal, that.U.pVal, getNumWords() * APINT_WORD_SIZE);
}

void APInt::initFromArray(ArrayRef<uint64_t> bigVal) {
  assert(bigVal.data() && "Null pointer detected!");
  ако (LLVM_LIKELY(isSingleWord()))
    U.VAL = bigVal[0];
  инако {
    // Get memory, cleared to 0
    U.pVal = getClearedMemory(getNumWords());
    // Calculate the number of words to copy
    unsigned words = std::min<unsigned>(bigVal.size(), getNumWords());
    // Copy the words from bigVal to pVal
    memcpy(U.pVal, bigVal.data(), words * APINT_WORD_SIZE);
  }
  // Make sure unused high bits are cleared
  clearUnusedBits();
}

APInt::APInt(unsigned numBits, ArrayRef<uint64_t> bigVal) : BitWidth(numBits) {
  initFromArray(bigVal);
}

APInt::APInt(unsigned numbits, StringRef Str, uint8_t radix)
    : BitWidth(numbits) {
  fromString(numbits, Str, radix);
}

void APInt::reallocate(unsigned NewBitWidth) {
  // If the number of words is the same we can just change the width and stop.
  ако (getNumWords() == getNumWords(NewBitWidth)) {
    BitWidth = NewBitWidth;
    врати;
  }

  // If we have an allocation, delete it.
  ако (LLVM_UNLIKELY(!isSingleWord()))
    delete [] U.pVal;

  // Update BitWidth.
  BitWidth = NewBitWidth;

  // If we are supposed to have an allocation, create it.
  ако (LLVM_UNLIKELY(!isSingleWord()))
    U.pVal = getMemory(getNumWords());
}

void APInt::assignSlowCase(const APInt &RHS) {
  // Don't do anything for X = X
  ако (this == &RHS)
    врати;

  // Adjust the bit width and handle allocations as necessary.
  reallocate(RHS.getBitWidth());

  // Copy the data.
  ако (LLVM_LIKELY(isSingleWord()))
    U.VAL = RHS.U.VAL;
  инако
    memcpy(U.pVal, RHS.U.pVal, getNumWords() * APINT_WORD_SIZE);
}

/// This method 'profiles' an APInt for use with FoldingSet.
void APInt::Profile(FoldingSetNodeID& ID) const {
  ID.AddInteger(BitWidth);

  ако (LLVM_LIKELY(isSingleWord())) {
    ID.AddInteger(U.VAL);
    врати;
  }

  unsigned NumWords = getNumWords();
  за (unsigned i = 0; i < NumWords; ++i)
    ID.AddInteger(U.pVal[i]);
}

bool APInt::isAligned(Align A) const {
  ако (isZero())
    врати true;
  const unsigned TrailingZeroes = countr_zero();
  const unsigned MinimumTrailingZeroes = Log2(A);
  врати TrailingZeroes >= MinimumTrailingZeroes;
}

/// Prefix increment operator. Increments the APInt by one.
APInt& APInt::operator++() {
  ако (LLVM_LIKELY(isSingleWord()))
    ++U.VAL;
  инако
    tcIncrement(U.pVal, getNumWords());
  врати clearUnusedBits();
}

/// Prefix decrement operator. Decrements the APInt by one.
APInt& APInt::operator--() {
  ако (LLVM_LIKELY(isSingleWord()))
    --U.VAL;
  инако
    tcDecrement(U.pVal, getNumWords());
  врати clearUnusedBits();
}

/// Adds the RHS APInt to this APInt.
/// @returns this, after addition of RHS.
/// Addition assignment operator.
APInt& APInt::operator+=(const APInt& RHS) {
  assert(BitWidth == RHS.BitWidth && "Bit widths must be the same");
  ако (LLVM_LIKELY(isSingleWord()))
    U.VAL += RHS.U.VAL;
  инако
    tcAdd(U.pVal, RHS.U.pVal, 0, getNumWords());
  врати clearUnusedBits();
}

APInt& APInt::operator+=(uint64_t RHS) {
  ако (LLVM_LIKELY(isSingleWord()))
    U.VAL += RHS;
  инако
    tcAddPart(U.pVal, RHS, getNumWords());
  врати clearUnusedBits();
}

/// Subtracts the RHS APInt from this APInt
/// @returns this, after subtraction
/// Subtraction assignment operator.
APInt& APInt::operator-=(const APInt& RHS) {
  assert(BitWidth == RHS.BitWidth && "Bit widths must be the same");
  ако (LLVM_LIKELY(isSingleWord()))
    U.VAL -= RHS.U.VAL;
  инако
    tcSubtract(U.pVal, RHS.U.pVal, 0, getNumWords());
  врати clearUnusedBits();
}

APInt& APInt::operator-=(uint64_t RHS) {
  ако (LLVM_LIKELY(isSingleWord()))
    U.VAL -= RHS;
  инако
    tcSubtractPart(U.pVal, RHS, getNumWords());
  врати clearUnusedBits();
}

APInt APInt::operator*(const APInt& RHS) const {
  assert(BitWidth == RHS.BitWidth && "Bit widths must be the same");
  ако (LLVM_LIKELY(isSingleWord()))
    врати APInt(BitWidth, U.VAL * RHS.U.VAL, /*isSigned=*/false,
                 /*implicitTrunc=*/true);

  APInt Result(getMemory(getNumWords()), getBitWidth());
  tcMultiply(Result.U.pVal, U.pVal, RHS.U.pVal, getNumWords());
  Result.clearUnusedBits();
  врати Result;
}

void APInt::andAssignSlowCase(const APInt &RHS) {
  WordType *dst = U.pVal, *rhs = RHS.U.pVal;
  за (size_t i = 0, e = getNumWords(); i != e; ++i)
    dst[i] &= rhs[i];
}

void APInt::orAssignSlowCase(const APInt &RHS) {
  WordType *dst = U.pVal, *rhs = RHS.U.pVal;
  за (size_t i = 0, e = getNumWords(); i != e; ++i)
    dst[i] |= rhs[i];
}

void APInt::xorAssignSlowCase(const APInt &RHS) {
  WordType *dst = U.pVal, *rhs = RHS.U.pVal;
  за (size_t i = 0, e = getNumWords(); i != e; ++i)
    dst[i] ^= rhs[i];
}

APInt &APInt::operator*=(const APInt &RHS) {
  *this = *this * RHS;
  врати *this;
}

APInt& APInt::operator*=(uint64_t RHS) {
  ако (LLVM_LIKELY(isSingleWord())) {
    U.VAL *= RHS;
  } инако {
    unsigned NumWords = getNumWords();
    tcMultiplyPart(U.pVal, U.pVal, RHS, 0, NumWords, NumWords, false);
  }
  врати clearUnusedBits();
}

bool APInt::equalSlowCase(const APInt &RHS) const {
  врати std::equal(U.pVal, U.pVal + getNumWords(), RHS.U.pVal);
}

цео APInt::compare(const APInt& RHS) const {
  assert(BitWidth == RHS.BitWidth && "Bit widths must be same for comparison");
  ако (LLVM_LIKELY(isSingleWord()))
    врати U.VAL < RHS.U.VAL ? -1 : U.VAL > RHS.U.VAL;

  врати tcCompare(U.pVal, RHS.U.pVal, getNumWords());
}

цео APInt::compareSigned(const APInt& RHS) const {
  assert(BitWidth == RHS.BitWidth && "Bit widths must be same for comparison");
  ако (LLVM_LIKELY(isSingleWord())) {
    int64_t lhsSext = SignExtend64(U.VAL, BitWidth);
    int64_t rhsSext = SignExtend64(RHS.U.VAL, BitWidth);
    врати lhsSext < rhsSext ? -1 : lhsSext > rhsSext;
  }

  bool lhsNeg = isNegative();
  bool rhsNeg = RHS.isNegative();

  // If the sign bits don't match, then (LHS < RHS) if LHS is negative
  ако (lhsNeg != rhsNeg)
    врати lhsNeg ? -1 : 1;

  // Otherwise we can just use an unsigned comparison, because even negative
  // numbers compare correctly this way if both have the same signed-ness.
  врати tcCompare(U.pVal, RHS.U.pVal, getNumWords());
}

void APInt::setBitsSlowCase(unsigned loBit, unsigned hiBit) {
  unsigned loWord = whichWord(loBit);
  unsigned hiWord = whichWord(hiBit);

  // Create an initial mask for the low word with zeros below loBit.
  uint64_t loMask = WORDTYPE_MAX << whichBit(loBit);

  // If hiBit is not aligned, we need a high mask.
  unsigned hiShiftAmt = whichBit(hiBit);
  ако (hiShiftAmt != 0) {
    // Create a high mask with zeros above hiBit.
    uint64_t hiMask = WORDTYPE_MAX >> (APINT_BITS_PER_WORD - hiShiftAmt);
    // If loWord and hiWord are equal, then we combine the masks. Otherwise,
    // set the bits in hiWord.
    ако (hiWord == loWord)
      loMask &= hiMask;
    инако
      U.pVal[hiWord] |= hiMask;
  }
  // Apply the mask to the low word.
  U.pVal[loWord] |= loMask;

  // Fill any words between loWord and hiWord with all ones.
  за (unsigned word = loWord + 1; word < hiWord; ++word)
    U.pVal[word] = WORDTYPE_MAX;
}

void APInt::clearBitsSlowCase(unsigned LoBit, unsigned HiBit) {
  unsigned LoWord = whichWord(LoBit);
  unsigned HiWord = whichWord(HiBit);

  // Create an initial mask for the low word with ones below loBit.
  uint64_t LoMask = ~(WORDTYPE_MAX << whichBit(LoBit));

  // If HiBit is not aligned, we need a high mask.
  unsigned HiShiftAmt = whichBit(HiBit);
  ако (HiShiftAmt != 0) {
    // Create a high mask with ones above HiBit.
    uint64_t HiMask = ~(WORDTYPE_MAX >> (APINT_BITS_PER_WORD - HiShiftAmt));
    // If LoWord and HiWord are equal, then we combine the masks. Otherwise,
    // clear the bits in HiWord.
    ако (HiWord == LoWord)
      LoMask |= HiMask;
    инако
      U.pVal[HiWord] &= HiMask;
  }
  // Apply the mask to the low word.
  U.pVal[LoWord] &= LoMask;

  // Fill any words between LoWord and HiWord with all zeros.
  за (unsigned Word = LoWord + 1; Word < HiWord; ++Word)
    U.pVal[Word] = 0;
}

// Complement a bignum in-place.
static void tcComplement(APInt::WordType *dst, unsigned parts) {
  за (unsigned i = 0; i < parts; i++)
    dst[i] = ~dst[i];
}

/// Toggle every bit to its opposite value.
void APInt::flipAllBitsSlowCase() {
  tcComplement(U.pVal, getNumWords());
  clearUnusedBits();
}

/// Concatenate the bits from "NewLSB" onto the bottom of *this.  This is
/// equivalent to:
///   (this->zext(NewWidth) << NewLSB.getBitWidth()) | NewLSB.zext(NewWidth)
/// In the slow case, we know the result is large.
APInt APInt::concatSlowCase(const APInt &NewLSB) const {
  unsigned NewWidth = getBitWidth() + NewLSB.getBitWidth();
  APInt Result = NewLSB.zext(NewWidth);
  Result.insertBits(*this, NewLSB.getBitWidth());
  врати Result;
}

/// Toggle a given bit to its opposite value whose position is given
/// as "bitPosition".
/// Toggles a given bit to its opposite value.
void APInt::flipBit(unsigned bitPosition) {
  assert(bitPosition < BitWidth && "Out of the bit-width range!");
  setBitVal(bitPosition, !(*this)[bitPosition]);
}

void APInt::insertBits(const APInt &subBits, unsigned bitPosition) {
  unsigned subBitWidth = subBits.getBitWidth();
  assert((subBitWidth + bitPosition) <= BitWidth && "Illegal bit insertion");

  // inserting no bits is a noop.
  ако (subBitWidth == 0)
    врати;

  // Insertion is a direct copy.
  ако (subBitWidth == BitWidth) {
    *this = subBits;
    врати;
  }

  // Single word result can be done as a direct bitmask.
  ако (LLVM_LIKELY(isSingleWord())) {
    uint64_t mask = WORDTYPE_MAX >> (APINT_BITS_PER_WORD - subBitWidth);
    U.VAL &= ~(mask << bitPosition);
    U.VAL |= (subBits.U.VAL << bitPosition);
    врати;
  }

  unsigned loBit = whichBit(bitPosition);
  unsigned loWord = whichWord(bitPosition);
  unsigned hi1Word = whichWord(bitPosition + subBitWidth - 1);

  // Insertion within a single word can be done as a direct bitmask.
  ако (loWord == hi1Word) {
    uint64_t mask = WORDTYPE_MAX >> (APINT_BITS_PER_WORD - subBitWidth);
    U.pVal[loWord] &= ~(mask << loBit);
    U.pVal[loWord] |= (subBits.U.VAL << loBit);
    врати;
  }

  // Insert on word boundaries.
  ако (loBit == 0) {
    // Direct copy whole words.
    unsigned numWholeSubWords = subBitWidth / APINT_BITS_PER_WORD;
    memcpy(U.pVal + loWord, subBits.getRawData(),
           numWholeSubWords * APINT_WORD_SIZE);

    // Mask+insert remaining bits.
    unsigned remainingBits = subBitWidth % APINT_BITS_PER_WORD;
    ако (remainingBits != 0) {
      uint64_t mask = WORDTYPE_MAX >> (APINT_BITS_PER_WORD - remainingBits);
      U.pVal[hi1Word] &= ~mask;
      U.pVal[hi1Word] |= subBits.getWord(subBitWidth - 1);
    }
    врати;
  }

  // General case - set/clear individual bits in dst based on src.
  // TODO - there is scope for optimization here, but at the moment this code
  // path is barely used so prefer readability over performance.
  за (unsigned i = 0; i != subBitWidth; ++i)
    setBitVal(bitPosition + i, subBits[i]);
}

void APInt::insertBits(uint64_t subBits, unsigned bitPosition, unsigned numBits) {
  uint64_t maskBits = maskTrailingOnes<uint64_t>(numBits);
  subBits &= maskBits;
  ако (LLVM_LIKELY(isSingleWord())) {
    U.VAL &= ~(maskBits << bitPosition);
    U.VAL |= subBits << bitPosition;
    врати;
  }

  unsigned loBit = whichBit(bitPosition);
  unsigned loWord = whichWord(bitPosition);
  unsigned hiWord = whichWord(bitPosition + numBits - 1);
  ако (loWord == hiWord) {
    U.pVal[loWord] &= ~(maskBits << loBit);
    U.pVal[loWord] |= subBits << loBit;
    врати;
  }

  static_assert(8 * sizeof(WordType) <= 64, "This code assumes only two words affected");
  unsigned wordBits = 8 * sizeof(WordType);
  U.pVal[loWord] &= ~(maskBits << loBit);
  U.pVal[loWord] |= subBits << loBit;

  U.pVal[hiWord] &= ~(maskBits >> (wordBits - loBit));
  U.pVal[hiWord] |= subBits >> (wordBits - loBit);
}

APInt APInt::extractBits(unsigned numBits, unsigned bitPosition) const {
  assert(bitPosition < BitWidth && (numBits + bitPosition) <= BitWidth &&
         "Illegal bit extraction");

  ако (LLVM_LIKELY(isSingleWord()))
    врати APInt(numBits, U.VAL >> bitPosition, /*isSigned=*/false,
                 /*implicitTrunc=*/true);

  unsigned loBit = whichBit(bitPosition);
  unsigned loWord = whichWord(bitPosition);
  unsigned hiWord = whichWord(bitPosition + numBits - 1);

  // Single word result extracting bits from a single word source.
  ако (loWord == hiWord)
    врати APInt(numBits, U.pVal[loWord] >> loBit, /*isSigned=*/false,
                 /*implicitTrunc=*/true);

  // Extracting bits that start on a source word boundary can be done
  // as a fast memory copy.
  ако (loBit == 0)
    врати APInt(numBits, ArrayRef(U.pVal + loWord, 1 + hiWord - loWord));

  // General case - shift + copy source words directly into place.
  APInt Result(numBits, 0);
  unsigned NumSrcWords = getNumWords();
  unsigned NumDstWords = Result.getNumWords();

  uint64_t *DestPtr =
      LLVM_LIKELY(Result.isSingleWord()) ? &Result.U.VAL : Result.U.pVal;
  за (unsigned word = 0; word < NumDstWords; ++word) {
    uint64_t w0 = U.pVal[loWord + word];
    uint64_t w1 =
        (loWord + word + 1) < NumSrcWords ? U.pVal[loWord + word + 1] : 0;
    DestPtr[word] = (w0 >> loBit) | (w1 << (APINT_BITS_PER_WORD - loBit));
  }

  врати Result.clearUnusedBits();
}

uint64_t APInt::extractBitsAsZExtValue(unsigned numBits,
                                       unsigned bitPosition) const {
  assert(bitPosition < BitWidth && (numBits + bitPosition) <= BitWidth &&
         "Illegal bit extraction");
  assert(numBits <= 64 && "Illegal bit extraction");

  uint64_t maskBits = maskTrailingOnes<uint64_t>(numBits);
  ако (LLVM_LIKELY(isSingleWord()))
    врати (U.VAL >> bitPosition) & maskBits;

  static_assert(APINT_BITS_PER_WORD >= 64,
                "This code assumes only two words affected");
  unsigned loBit = whichBit(bitPosition);
  unsigned loWord = whichWord(bitPosition);
  unsigned hiWord = whichWord(bitPosition + numBits - 1);
  ако (loWord == hiWord)
    врати (U.pVal[loWord] >> loBit) & maskBits;

  uint64_t retBits = U.pVal[loWord] >> loBit;
  retBits |= U.pVal[hiWord] << (APINT_BITS_PER_WORD - loBit);
  retBits &= maskBits;
  врати retBits;
}

unsigned APInt::getSufficientBitsNeeded(StringRef Str, uint8_t Radix) {
  assert(!Str.empty() && "Invalid string length");
  size_t StrLen = Str.size();

  // Each computation below needs to know if it's negative.
  unsigned IsNegative = false;
  ако (Str[0] == '-' || Str[0] == '+') {
    IsNegative = Str[0] == '-';
    StrLen--;
    assert(StrLen && "String is only a sign, needs a value.");
  }

  // For radixes of power-of-two values, the bits required is accurately and
  // easily computed.
  ако (Radix == 2)
    врати StrLen + IsNegative;
  ако (Radix == 8)
    врати StrLen * 3 + IsNegative;
  ако (Radix == 16)
    врати StrLen * 4 + IsNegative;

  // Compute a sufficient number of bits that is always large enough but might
  // be too large. This avoids the assertion in the constructor. This
  // calculation doesn't work appropriately for the numbers 0-9, so just use 4
  // bits in that case.
  ако (Radix == 10)
    врати (StrLen == 1 ? 4 : StrLen * 64 / 18) + IsNegative;

  assert(Radix == 36);
  врати (StrLen == 1 ? 7 : StrLen * 16 / 3) + IsNegative;
}

unsigned APInt::getBitsNeeded(StringRef str, uint8_t radix) {
  // Compute a sufficient number of bits that is always large enough but might
  // be too large.
  unsigned sufficient = getSufficientBitsNeeded(str, radix);

  // For bases 2, 8, and 16, the sufficient number of bits is exact and we can
  // return the value directly. For bases 10 and 36, we need to do extra work.
  ако (radix == 2 || radix == 8 || radix == 16)
    врати sufficient;

  // This is grossly inefficient but accurate. We could probably do something
  // with a computation of roughly slen*64/20 and then adjust by the value of
  // the first few digits. But, I'm not sure how accurate that could be.
  size_t slen = str.size();

  // Each computation below needs to know if it's negative.
  StringRef::iterator p = str.begin();
  unsigned isNegative = *p == '-';
  ако (*p == '-' || *p == '+') {
    p++;
    slen--;
    assert(slen && "String is only a sign, needs a value.");
  }


  // Convert to the actual binary value.
  APInt tmp(sufficient, StringRef(p, slen), radix);

  // Compute how many bits are required. If the log is infinite, assume we need
  // just bit. If the log is exact and value is negative, then the value is
  // MinSignedValue with (log + 1) bits.
  unsigned log = tmp.logBase2();
  ако (log == (unsigned)-1) {
    врати isNegative + 1;
  } инако ако (isNegative && tmp.isPowerOf2()) {
    врати isNegative + log;
  } инако {
    врати isNegative + log + 1;
  }
}

hash_code llvm::hash_value(const APInt &Arg) {
  ако (LLVM_LIKELY(Arg.isSingleWord()))
    врати hash_combine(Arg.BitWidth, Arg.U.VAL);

  врати hash_combine(
      Arg.BitWidth,
      hash_combine_range(Arg.U.pVal, Arg.U.pVal + Arg.getNumWords()));
}

unsigned DenseMapInfo<APInt, void>::getHashValue(const APInt &Key) {
  врати static_cast<unsigned>(hash_value(Key));
}

bool APInt::isSplat(unsigned SplatSizeInBits) const {
  assert(getBitWidth() % SplatSizeInBits == 0 &&
         "SplatSizeInBits must divide width!");
  // We can check that all parts of an integer are equal by making use of a
  // little trick: rotate and check if it's still the same value.
  врати *this == rotl(SplatSizeInBits);
}

/// This function returns the high "numBits" bits of this APInt.
APInt APInt::getHiBits(unsigned numBits) const {
  врати this->lshr(BitWidth - numBits);
}

/// This function returns the low "numBits" bits of this APInt.
APInt APInt::getLoBits(unsigned numBits) const {
  APInt Result(getLowBitsSet(BitWidth, numBits));
  Result &= *this;
  врати Result;
}

/// Return a value containing V broadcasted over NewLen bits.
APInt APInt::getSplat(unsigned NewLen, const APInt &V) {
  assert(NewLen >= V.getBitWidth() && "Can't splat to smaller bit width!");

  APInt Val = V.zext(NewLen);
  за (unsigned I = V.getBitWidth(); I < NewLen; I <<= 1)
    Val |= Val << I;

  врати Val;
}

unsigned APInt::countLeadingZerosSlowCase() const {
  unsigned Count = 0;
  за (цео i = getNumWords() - 1; i >= 0; --i) {
    uint64_t V = U.pVal[i];
    ако (V == 0)
      Count += APINT_BITS_PER_WORD;
    инако {
      Count += llvm::countl_zero(V);
      break;
    }
  }
  // Adjust for unused bits in the most significant word (they are zero).
  unsigned Mod = BitWidth % APINT_BITS_PER_WORD;
  Count -= Mod > 0 ? APINT_BITS_PER_WORD - Mod : 0;
  врати Count;
}

unsigned APInt::countLeadingOnesSlowCase() const {
  unsigned highWordBits = BitWidth % APINT_BITS_PER_WORD;
  unsigned shift;
  ако (!highWordBits) {
    highWordBits = APINT_BITS_PER_WORD;
    shift = 0;
  } инако {
    shift = APINT_BITS_PER_WORD - highWordBits;
  }
  цео i = getNumWords() - 1;
  unsigned Count = llvm::countl_one(U.pVal[i] << shift);
  ако (Count == highWordBits) {
    за (i--; i >= 0; --i) {
      ако (U.pVal[i] == WORDTYPE_MAX)
        Count += APINT_BITS_PER_WORD;
      инако {
        Count += llvm::countl_one(U.pVal[i]);
        break;
      }
    }
  }
  врати Count;
}

unsigned APInt::countTrailingZerosSlowCase() const {
  unsigned Count = 0;
  unsigned i = 0;
  за (; i < getNumWords() && U.pVal[i] == 0; ++i)
    Count += APINT_BITS_PER_WORD;
  ако (i < getNumWords())
    Count += llvm::countr_zero(U.pVal[i]);
  врати std::min(Count, BitWidth);
}

unsigned APInt::countTrailingOnesSlowCase() const {
  unsigned Count = 0;
  unsigned i = 0;
  за (; i < getNumWords() && U.pVal[i] == WORDTYPE_MAX; ++i)
    Count += APINT_BITS_PER_WORD;
  ако (i < getNumWords())
    Count += llvm::countr_one(U.pVal[i]);
  assert(Count <= BitWidth);
  врати Count;
}

unsigned APInt::countPopulationSlowCase() const {
  unsigned Count = 0;
  за (unsigned i = 0; i < getNumWords(); ++i)
    Count += llvm::popcount(U.pVal[i]);
  врати Count;
}

bool APInt::isPowerOf2SlowCase() const {
  unsigned Count = 0;
  за (unsigned i = 0; i < getNumWords(); ++i) {
    Count += llvm::popcount(U.pVal[i]);
    ако (Count > 1)
      врати false;
  }
  врати Count == 1;
}

bool APInt::intersectsSlowCase(const APInt &RHS) const {
  за (unsigned i = 0, e = getNumWords(); i != e; ++i)
    ако ((U.pVal[i] & RHS.U.pVal[i]) != 0)
      врати true;

  врати false;
}

bool APInt::isSubsetOfSlowCase(const APInt &RHS) const {
  за (unsigned i = 0, e = getNumWords(); i != e; ++i)
    ако ((U.pVal[i] & ~RHS.U.pVal[i]) != 0)
      врати false;

  врати true;
}

bool APInt::isInverseOfSlowCase(const APInt &RHS) const {
  const unsigned Last = getNumWords() - 1;
  за (unsigned I = 0; I != Last; ++I)
    ако ((U.pVal[I] ^ RHS.U.pVal[I]) != WORDTYPE_MAX)
      врати false;

  unsigned TailBits = BitWidth - Last * APINT_BITS_PER_WORD;
  WordType TailMask = llvm::maskTrailingOnes<WordType>(TailBits);
  врати (U.pVal[Last] ^ RHS.U.pVal[Last]) == TailMask;
}

APInt APInt::byteSwap() const {
  assert(BitWidth >= 16 && BitWidth % 8 == 0 && "Cannot byteswap!");
  ако (BitWidth == 16)
    врати APInt(BitWidth, llvm::byteswap<uint16_t>(U.VAL));
  ако (BitWidth == 32)
    врати APInt(BitWidth, llvm::byteswap<uint32_t>(U.VAL));
  ако (BitWidth <= 64) {
    uint64_t Tmp1 = llvm::byteswap<uint64_t>(U.VAL);
    Tmp1 >>= (64 - BitWidth);
    врати APInt(BitWidth, Tmp1);
  }

  APInt Result(getNumWords() * APINT_BITS_PER_WORD, 0);
  за (unsigned I = 0, N = getNumWords(); I != N; ++I)
    Result.U.pVal[I] = llvm::byteswap<uint64_t>(U.pVal[N - I - 1]);
  ако (Result.BitWidth != BitWidth) {
    Result.lshrInPlace(Result.BitWidth - BitWidth);
    Result.BitWidth = BitWidth;
  }
  врати Result;
}

APInt APInt::reverseBits() const {
  ако (LLVM_LIKELY(isSingleWord())) {
    switch (BitWidth) {
    case 64:
      врати APInt(BitWidth, llvm::reverseBits<uint64_t>(U.VAL));
    case 32:
      врати APInt(BitWidth, llvm::reverseBits<uint32_t>(U.VAL));
    case 16:
      врати APInt(BitWidth, llvm::reverseBits<uint16_t>(U.VAL));
    case 8:
      врати APInt(BitWidth, llvm::reverseBits<uint8_t>(U.VAL));
    case 1: // fallthrough
    case 0:
      врати *this;
    default:
      врати APInt(BitWidth,
                   llvm::reverseBits<uint64_t>(U.VAL) >> (64 - BitWidth));
    }
  }

  APInt Result(BitWidth, 0);
  unsigned NumWords = getNumWords();
  unsigned ExcessBits = NumWords * APINT_BITS_PER_WORD - BitWidth;
  ако (ExcessBits == 0) {
    // Fast path. No cross-word shift needed.
    за (unsigned I = 0; I < NumWords; ++I)
      Result.U.pVal[I] = llvm::reverseBits<uint64_t>(U.pVal[NumWords - 1 - I]);
    врати Result;
  }
  // Holds reversed bits of the previous (more significant) word.
  uint64_t PrevRev = llvm::reverseBits<uint64_t>(U.pVal[NumWords - 1]);
  за (unsigned I = 0; I < NumWords - 1; ++I) {
    uint64_t CurrRev = llvm::reverseBits<uint64_t>(U.pVal[NumWords - 2 - I]);
    Result.U.pVal[I] = (PrevRev >> ExcessBits) | (CurrRev << (64 - ExcessBits));
    PrevRev = CurrRev;
  }
  Result.U.pVal[NumWords - 1] = PrevRev >> ExcessBits;
  врати Result;
}

APInt llvm::APIntOps::GreatestCommonDivisor(APInt A, APInt B, bool IsSigned) {
  // Take absolute value if IsSigned.
  ако (IsSigned) {
    A = A.abs();
    B = B.abs();
  }

  // Fast-path a common case.
  ако (A == B) врати A;

  // Corner cases: if either operand is zero, the other is the gcd.
  ако (!A) врати B;
  ако (!B) врати A;

  // Count common powers of 2 and remove all other powers of 2.
  unsigned Pow2;
  {
    unsigned Pow2_A = A.countr_zero();
    unsigned Pow2_B = B.countr_zero();
    ако (Pow2_A > Pow2_B) {
      A.lshrInPlace(Pow2_A - Pow2_B);
      Pow2 = Pow2_B;
    } инако ако (Pow2_B > Pow2_A) {
      B.lshrInPlace(Pow2_B - Pow2_A);
      Pow2 = Pow2_A;
    } инако {
      Pow2 = Pow2_A;
    }
  }

  // Both operands are odd multiples of 2^Pow_2:
  //
  //   gcd(a, b) = gcd(|a - b| / 2^i, min(a, b))
  //
  // This is a modified version of Stein's algorithm, taking advantage of
  // efficient countTrailingZeros().
  док (A != B) {
    ако (A.ugt(B)) {
      A -= B;
      A.lshrInPlace(A.countr_zero() - Pow2);
    } инако {
      B -= A;
      B.lshrInPlace(B.countr_zero() - Pow2);
    }
  }

  врати A;
}

APInt llvm::APIntOps::RoundDoubleToAPInt(double Double, unsigned width) {
  uint64_t I = bit_cast<uint64_t>(Double);

  // Get the sign bit from the highest order bit
  bool isNeg = I >> 63;

  // Get the 11-bit exponent and adjust for the 1023 bit bias
  int64_t exp = ((I >> 52) & 0x7ff) - 1023;

  // If the exponent is negative, the value is < 0 so just return 0.
  ако (exp < 0)
    врати APInt(width, 0u);

  // Extract the mantissa by clearing the top 12 bits (sign + exponent).
  uint64_t mantissa = (I & (~0ULL >> 12)) | 1ULL << 52;

  // If the exponent doesn't shift all bits out of the mantissa
  ако (exp < 52)
    врати isNeg ? -APInt(width, mantissa >> (52 - exp)) :
                    APInt(width, mantissa >> (52 - exp));

  // If the client didn't provide enough bits for us to shift the mantissa into
  // then the result is undefined, just return 0
  ако (width <= exp - 52)
    врати APInt(width, 0);

  // Otherwise, we have to shift the mantissa bits up to the right location
  APInt Tmp(width, mantissa);
  Tmp <<= (unsigned)exp - 52;
  врати isNeg ? -Tmp : Tmp;
}

/// This function converts this APInt to a double.
/// The layout for double is as following (IEEE Standard 754):
///  --------------------------------------
/// |  Sign    Exponent    Fraction    Bias |
/// |-------------------------------------- |
/// |  1[63]   11[62-52]   52[51-00]   1023 |
///  --------------------------------------
double APInt::roundToDouble(bool isSigned) const {
  // Handle the simple case where the value is contained in one uint64_t.
  // It is wrong to optimize getWord(0) to VAL; there might be more than one word.
  ако (LLVM_LIKELY(isSingleWord() || getActiveBits() <= APINT_BITS_PER_WORD)) {
    ако (isSigned) {
      int64_t sext = SignExtend64(getWord(0), BitWidth);
      врати double(sext);
    }
    врати double(getWord(0));
  }

  // Determine if the value is negative.
  bool isNeg = isSigned ? (*this)[BitWidth-1] : false;

  // Construct the absolute value if we're negative.
  APInt Tmp(isNeg ? -(*this) : (*this));

  // Figure out how many bits we're using.
  unsigned n = Tmp.getActiveBits();

  // The exponent (without bias normalization) is just the number of bits
  // we are using. Note that the sign bit is gone since we constructed the
  // absolute value.
  uint64_t exp = n;

  // Return infinity for exponent overflow
  ако (exp > 1023) {
    ако (!isSigned || !isNeg)
      врати std::numeric_limits<double>::infinity();
    инако
      врати -std::numeric_limits<double>::infinity();
  }
  exp += 1023; // Increment for 1023 bias

  // Number of bits in mantissa is 52. To obtain the mantissa value, we must
  // extract the high 52 bits from the correct words in pVal.
  uint64_t mantissa;
  unsigned hiWord = whichWord(n-1);
  ако (hiWord == 0) {
    mantissa = Tmp.U.pVal[0];
    ако (n > 52)
      mantissa >>= n - 52; // shift down, we want the top 52 bits.
  } инако {
    assert(hiWord > 0 && "huh?");
    uint64_t hibits = Tmp.U.pVal[hiWord] << (52 - n % APINT_BITS_PER_WORD);
    uint64_t lobits = Tmp.U.pVal[hiWord-1] >> (11 + n % APINT_BITS_PER_WORD);
    mantissa = hibits | lobits;
  }

  // The leading bit of mantissa is implicit, so get rid of it.
  uint64_t sign = isNeg ? (1ULL << (APINT_BITS_PER_WORD - 1)) : 0;
  uint64_t I = sign | (exp << 52) | mantissa;
  врати bit_cast<double>(I);
}

// Truncate to new width.
APInt APInt::trunc(unsigned width) const {
  assert(width <= BitWidth && "Invalid APInt Truncate request");

  ако (width <= APINT_BITS_PER_WORD)
    врати APInt(width, getRawData()[0], /*isSigned=*/false,
                 /*implicitTrunc=*/true);

  ако (width == BitWidth)
    врати *this;

  APInt Result(getMemory(getNumWords(width)), width);

  // Copy full words.
  unsigned i;
  за (i = 0; i != width / APINT_BITS_PER_WORD; i++)
    Result.U.pVal[i] = U.pVal[i];

  // Truncate and copy any partial word.
  unsigned bits = (0 - width) % APINT_BITS_PER_WORD;
  ако (bits != 0)
    Result.U.pVal[i] = U.pVal[i] << bits >> bits;

  врати Result;
}

// Truncate to new width with unsigned saturation.
APInt APInt::truncUSat(unsigned width) const {
  assert(width <= BitWidth && "Invalid APInt Truncate request");

  // Can we just losslessly truncate it?
  ако (isIntN(width))
    врати trunc(width);
  // If not, then just return the new limit.
  врати APInt::getMaxValue(width);
}

// Truncate to new width with signed saturation to signed result.
APInt APInt::truncSSat(unsigned width) const {
  assert(width <= BitWidth && "Invalid APInt Truncate request");

  // Can we just losslessly truncate it?
  ако (isSignedIntN(width))
    врати trunc(width);
  // If not, then just return the new limits.
  врати isNegative() ? APInt::getSignedMinValue(width)
                      : APInt::getSignedMaxValue(width);
}

// Truncate to new width with signed saturation to unsigned result.
APInt APInt::truncSSatU(unsigned width) const {
  assert(width <= BitWidth && "Invalid APInt Truncate request");

  // Can we just losslessly truncate it?
  ако (isIntN(width))
    врати trunc(width);
  // If not, then just return the new limits.
  врати isNegative() ? APInt::getZero(width) : APInt::getMaxValue(width);
}

// Sign extend to a new width.
APInt APInt::sext(unsigned Width) const {
  assert(Width >= BitWidth && "Invalid APInt SignExtend request");

  ако (Width <= APINT_BITS_PER_WORD)
    врати APInt(Width, SignExtend64(U.VAL, BitWidth), /*isSigned=*/true);

  ако (Width == BitWidth)
    врати *this;

  APInt Result(getMemory(getNumWords(Width)), Width);

  // Copy words.
  std::memcpy(Result.U.pVal, getRawData(), getNumWords() * APINT_WORD_SIZE);

  // Sign extend the last word since there may be unused bits in the input.
  Result.U.pVal[getNumWords() - 1] =
      SignExtend64(Result.U.pVal[getNumWords() - 1],
                   ((BitWidth - 1) % APINT_BITS_PER_WORD) + 1);

  // Fill with sign bits.
  std::memset(Result.U.pVal + getNumWords(), isNegative() ? -1 : 0,
              (Result.getNumWords() - getNumWords()) * APINT_WORD_SIZE);
  Result.clearUnusedBits();
  врати Result;
}

//  Zero extend to a new width.
APInt APInt::zext(unsigned width) const {
  assert(width >= BitWidth && "Invalid APInt ZeroExtend request");

  ако (width <= APINT_BITS_PER_WORD)
    врати APInt(width, U.VAL);

  ако (width == BitWidth)
    врати *this;

  APInt Result(getMemory(getNumWords(width)), width);

  // Copy words.
  std::memcpy(Result.U.pVal, getRawData(), getNumWords() * APINT_WORD_SIZE);

  // Zero remaining words.
  std::memset(Result.U.pVal + getNumWords(), 0,
              (Result.getNumWords() - getNumWords()) * APINT_WORD_SIZE);

  врати Result;
}

APInt APInt::zextOrTrunc(unsigned width) const {
  ако (BitWidth < width)
    врати zext(width);
  ако (BitWidth > width)
    врати trunc(width);
  врати *this;
}

APInt APInt::sextOrTrunc(unsigned width) const {
  ако (BitWidth < width)
    врати sext(width);
  ако (BitWidth > width)
    врати trunc(width);
  врати *this;
}

/// Arithmetic right-shift this APInt by shiftAmt.
/// Arithmetic right-shift function.
void APInt::ashrInPlace(const APInt &shiftAmt) {
  ashrInPlace((unsigned)shiftAmt.getLimitedValue(BitWidth));
}

/// Arithmetic right-shift this APInt by shiftAmt.
/// Arithmetic right-shift function.
void APInt::ashrSlowCase(unsigned ShiftAmt) {
  // Don't bother performing a no-op shift.
  ако (!ShiftAmt)
    врати;

  // Save the original sign bit for later.
  bool Negative = isNegative();

  // WordShift is the inter-part shift; BitShift is intra-part shift.
  unsigned WordShift = ShiftAmt / APINT_BITS_PER_WORD;
  unsigned BitShift = ShiftAmt % APINT_BITS_PER_WORD;

  unsigned WordsToMove = getNumWords() - WordShift;
  ако (WordsToMove != 0) {
    // Sign extend the last word to fill in the unused bits.
    U.pVal[getNumWords() - 1] = SignExtend64(
        U.pVal[getNumWords() - 1], ((BitWidth - 1) % APINT_BITS_PER_WORD) + 1);

    // Fastpath for moving by whole words.
    ако (BitShift == 0) {
      std::memmove(U.pVal, U.pVal + WordShift, WordsToMove * APINT_WORD_SIZE);
    } инако {
      // Move the words containing significant bits.
      за (unsigned i = 0; i != WordsToMove - 1; ++i)
        U.pVal[i] = (U.pVal[i + WordShift] >> BitShift) |
                    (U.pVal[i + WordShift + 1] << (APINT_BITS_PER_WORD - BitShift));

      // Handle the last word which has no high bits to copy. Use an arithmetic
      // shift to preserve the sign bit.
      U.pVal[WordsToMove - 1] =
          (int64_t)U.pVal[WordShift + WordsToMove - 1] >> BitShift;
    }
  }

  // Fill in the remainder based on the original sign.
  std::memset(U.pVal + WordsToMove, Negative ? -1 : 0,
              WordShift * APINT_WORD_SIZE);
  clearUnusedBits();
}

/// Logical right-shift this APInt by shiftAmt.
/// Logical right-shift function.
void APInt::lshrInPlace(const APInt &shiftAmt) {
  lshrInPlace((unsigned)shiftAmt.getLimitedValue(BitWidth));
}

/// Logical right-shift this APInt by shiftAmt.
/// Logical right-shift function.
void APInt::lshrSlowCase(unsigned ShiftAmt) {
  tcShiftRight(U.pVal, getNumWords(), ShiftAmt);
}

/// Left-shift this APInt by shiftAmt.
/// Left-shift function.
APInt &APInt::operator<<=(const APInt &shiftAmt) {
  // It's undefined behavior in C to shift by BitWidth or greater.
  *this <<= (unsigned)shiftAmt.getLimitedValue(BitWidth);
  врати *this;
}

void APInt::shlSlowCase(unsigned ShiftAmt) {
  tcShiftLeft(U.pVal, getNumWords(), ShiftAmt);
  clearUnusedBits();
}

// Calculate the rotate amount modulo the bit width.
static unsigned rotateModulo(unsigned BitWidth, const APInt &rotateAmt) {
  ако (LLVM_UNLIKELY(BitWidth == 0))
    врати 0;
  unsigned rotBitWidth = rotateAmt.getBitWidth();
  APInt rot = rotateAmt;
  ако (rotBitWidth < BitWidth) {
    // Extend the rotate APInt, so that the urem doesn't divide by 0.
    // e.g. APInt(1, 32) would give APInt(1, 0).
    rot = rotateAmt.zext(BitWidth);
  }
  rot = rot.urem(APInt(rot.getBitWidth(), BitWidth));
  врати rot.getLimitedValue(BitWidth);
}

APInt APInt::rotl(const APInt &rotateAmt) const {
  врати rotl(rotateModulo(BitWidth, rotateAmt));
}

APInt APInt::rotl(unsigned rotateAmt) const {
  ако (LLVM_UNLIKELY(BitWidth == 0))
    врати *this;
  rotateAmt %= BitWidth;
  ако (rotateAmt == 0)
    врати *this;
  врати shl(rotateAmt) | lshr(BitWidth - rotateAmt);
}

APInt APInt::rotr(const APInt &rotateAmt) const {
  врати rotr(rotateModulo(BitWidth, rotateAmt));
}

APInt APInt::rotr(unsigned rotateAmt) const {
  ако (BitWidth == 0)
    врати *this;
  rotateAmt %= BitWidth;
  ако (rotateAmt == 0)
    врати *this;
  врати lshr(rotateAmt) | shl(BitWidth - rotateAmt);
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
unsigned APInt::nearestLogBase2() const {
  // Special case when we have a bitwidth of 1. If VAL is 1, then we
  // get 0. If VAL is 0, we get WORDTYPE_MAX which gets truncated to
  // UINT32_MAX.
  ако (BitWidth == 1)
    врати U.VAL - 1;

  // Handle the zero case.
  ако (isZero())
    врати UINT32_MAX;

  // The non-zero case is handled by computing:
  //
  //   nearestLogBase2(x) = logBase2(x) + x[logBase2(x)-1].
  //
  // where x[i] is referring to the value of the ith bit of x.
  unsigned lg = logBase2();
  врати lg + unsigned((*this)[lg - 1]);
}

// Square Root - this method computes and returns the square root of "this".
// Three mechanisms are used for computation. For small values (<= 5 bits),
// a table lookup is done. This gets some performance for common cases. For
// values using less than 52 bits, the value is converted to double and then
// the libc sqrt function is called. The result is rounded and then converted
// back to a uint64_t which is then used to construct the result. Finally,
// the Babylonian method for computing square roots is used.
APInt APInt::sqrtFloor() const {

  // Determine the magnitude of the value.
  unsigned magnitude = getActiveBits();

  // Use a fast table for some small values. This also gets rid of some
  // rounding errors in libc sqrt for small values.
  ако (magnitude <= 5) {
    static const uint8_t results[32] = {
        /*     0 */ 0,
        /*  1- 3 */ 1, 1, 1,
        /*  4- 8 */ 2, 2, 2, 2, 2,
        /*  9-15 */ 3, 3, 3, 3, 3, 3, 3,
        /* 16-24 */ 4, 4, 4, 4, 4, 4, 4, 4, 4,
        /* 25-31 */ 5, 5, 5, 5, 5, 5, 5,
    };
    врати APInt(BitWidth,
                 results[(LLVM_LIKELY(isSingleWord()) ? U.VAL : U.pVal[0])]);
  }

  // If the magnitude of the value fits in less than 52 bits (the precision of
  // an IEEE double precision floating point value), then we can use the
  // libc sqrt function which will probably use a hardware sqrt computation.
  // This should be faster than the algorithm below.
  ако (magnitude < 52) {
    врати APInt(BitWidth,
                 uint64_t(::floor(::sqrt(double(
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
  за (;; i += 2, testy = testy.shl(2))
    ако (i >= nbits || this->ule(testy)) {
      x_old = x_old.shl(i / 2);
      break;
    }

  // Use the Babylonian method to arrive at the integer square root:
  за (;;) {
    x_new = (this->udiv(x_old) + x_old).udiv(two);
    ако (x_old.ule(x_new))
      break;
    x_old = x_new;
  }
  врати x_old;
}

/// \returns the multiplicative inverse of an odd APInt modulo 2^BitWidth.
APInt APInt::multiplicativeInverse() const {
  assert((*this)[0] &&
         "multiplicative inverse is only defined for odd numbers!");

  // Use Newton's method.
  APInt Factor = *this;
  APInt T;
  док (!(T = *this * Factor).isOne())
    Factor *= 2 - std::move(T);
  врати Factor;
}

/// Implementation of Knuth's Algorithm D (Division of nonnegative integers)
/// from "Art of Computer Programming, Volume 2", section 4.3.1, p. 272. The
/// variables here have the same names as in the algorithm. Comments explain
/// the algorithm and any deviation from it.
static void KnuthDiv(uint32_t *u, uint32_t *v, uint32_t *q, uint32_t* r,
                     unsigned m, unsigned n) {
  assert(u && "Must provide dividend");
  assert(v && "Must provide divisor");
  assert(q && "Must provide quotient");
  assert(u != v && u != q && v != q && "Must use different memory");
  assert(n>1 && "n must be > 1");

  // b denotes the base of the number system. In our case b is 2^32.
  const uint64_t b = uint64_t(1) << 32;

// The DEBUG macros here tend to be spam in the debug output if you're not
// debugging this code. Disable them unless KNUTH_DEBUG is defined.
#ifdef KNUTH_DEBUG
#define DEBUG_KNUTH(X) LLVM_DEBUG(X)
#инако
#define DEBUG_KNUTH(X) do {} док(false)
#endif

  DEBUG_KNUTH(dbgs() << "KnuthDiv: m=" << m << " n=" << n << '\n');
  DEBUG_KNUTH(dbgs() << "KnuthDiv: original:");
  DEBUG_KNUTH(за (цео i = m + n; i >= 0; i--) dbgs() << " " << u[i]);
  DEBUG_KNUTH(dbgs() << " by");
  DEBUG_KNUTH(за (цео i = n; i > 0; i--) dbgs() << " " << v[i - 1]);
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
  ако (shift) {
    за (unsigned i = 0; i < m+n; ++i) {
      uint32_t u_tmp = u[i] >> (32 - shift);
      u[i] = (u[i] << shift) | u_carry;
      u_carry = u_tmp;
    }
    за (unsigned i = 0; i < n; ++i) {
      uint32_t v_tmp = v[i] >> (32 - shift);
      v[i] = (v[i] << shift) | v_carry;
      v_carry = v_tmp;
    }
  }
  u[m+n] = u_carry;

  DEBUG_KNUTH(dbgs() << "KnuthDiv:   normal:");
  DEBUG_KNUTH(за (цео i = m + n; i >= 0; i--) dbgs() << " " << u[i]);
  DEBUG_KNUTH(dbgs() << " by");
  DEBUG_KNUTH(за (цео i = n; i > 0; i--) dbgs() << " " << v[i - 1]);
  DEBUG_KNUTH(dbgs() << '\n');

  // D2. [Initialize j.]  Set j to m. This is the loop counter over the places.
  цео j = m;
  do {
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
    ако (qp == b || qp*v[n-2] > b*rp + u[j+n-2]) {
      qp--;
      rp += v[n-1];
      ако (rp < b && (qp == b || qp*v[n-2] > b*rp + u[j+n-2]))
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
    за (unsigned i = 0; i < n; ++i) {
      uint64_t p = qp * uint64_t(v[i]);
      int64_t subres = int64_t(u[j+i]) - borrow - Lo_32(p);
      u[j+i] = Lo_32(subres);
      borrow = Hi_32(p) - Hi_32(subres);
      DEBUG_KNUTH(dbgs() << "KnuthDiv: u[j+i] = " << u[j + i]
                        << ", borrow = " << borrow << '\n');
    }
    bool isNeg = u[j+n] < borrow;
    u[j+n] -= Lo_32(borrow);

    DEBUG_KNUTH(dbgs() << "KnuthDiv: after subtraction:");
    DEBUG_KNUTH(за (цео i = m + n; i >= 0; i--) dbgs() << " " << u[i]);
    DEBUG_KNUTH(dbgs() << '\n');

    // D5. [Test remainder.] Set q[j] = qp. If the result of step D4 was
    // negative, go to step D6; otherwise go on to step D7.
    q[j] = Lo_32(qp);
    ако (isNeg) {
      // D6. [Add back]. The probability that this step is necessary is very
      // small, on the order of only 2/b. Make sure that test data accounts for
      // this possibility. Decrease q[j] by 1
      q[j]--;
      // and add (0v[n-1]...v[1]v[0]) to (u[j+n]u[j+n-1]...u[j+1]u[j]).
      // A carry will occur to the left of u[j+n], and it should be ignored
      // since it cancels with the borrow that occurred in D4.
      bool carry = false;
      за (unsigned i = 0; i < n; i++) {
        uint32_t limit = std::min(u[j+i],v[i]);
        u[j+i] += v[i] + carry;
        carry = u[j+i] < limit || (carry && u[j+i] == limit);
      }
      u[j+n] += carry;
    }
    DEBUG_KNUTH(dbgs() << "KnuthDiv: after correction:");
    DEBUG_KNUTH(за (цео i = m + n; i >= 0; i--) dbgs() << " " << u[i]);
    DEBUG_KNUTH(dbgs() << "\nKnuthDiv: digit result = " << q[j] << '\n');

    // D7. [Loop on j.]  Decrease j by one. Now if j >= 0, go back to D3.
  } док (--j >= 0);

  DEBUG_KNUTH(dbgs() << "KnuthDiv: quotient:");
  DEBUG_KNUTH(за (цео i = m; i >= 0; i--) dbgs() << " " << q[i]);
  DEBUG_KNUTH(dbgs() << '\n');

  // D8. [Unnormalize]. Now q[...] is the desired quotient, and the desired
  // remainder may be obtained by dividing u[...] by d. If r is non-null we
  // compute the remainder (urem uses this).
  ако (r) {
    // The value d is expressed by the "shift" value above since we avoided
    // multiplication by d by using a shift left. So, all we have to do is
    // shift right here.
    ако (shift) {
      uint32_t carry = 0;
      DEBUG_KNUTH(dbgs() << "KnuthDiv: remainder:");
      за (цео i = n-1; i >= 0; i--) {
        r[i] = (u[i] >> shift) | carry;
        carry = u[i] << (32 - shift);
        DEBUG_KNUTH(dbgs() << " " << r[i]);
      }
    } инако {
      за (цео i = n-1; i >= 0; i--) {
        r[i] = u[i];
        DEBUG_KNUTH(dbgs() << " " << r[i]);
      }
    }
    DEBUG_KNUTH(dbgs() << '\n');
  }
  DEBUG_KNUTH(dbgs() << '\n');
}

void APInt::divide(const WordType *LHS, unsigned lhsWords, const WordType *RHS,
                   unsigned rhsWords, WordType *Quotient, WordType *Remainder) {
  assert(lhsWords >= rhsWords && "Fractional result");

  // First, compose the values into an array of 32-bit words instead of
  // 64-bit words. This is a necessity of both the "short division" algorithm
  // and the Knuth "classical algorithm" which requires there to be native
  // operations for +, -, and * on an m bit value with an m*2 bit result. We
  // can't use 64-bit operands here because we don't have native results of
  // 128-bits. Furthermore, casting the 64-bit values to 32-bit values won't
  // work on large-endian machines.
  unsigned n = rhsWords * 2;
  unsigned m = (lhsWords * 2) - n;

  // Allocate space for the temporary values we need either on the stack, if
  // it will fit, or on the heap if it won't.
  uint32_t SPACE[128];
  uint32_t *U = nullptr;
  uint32_t *V = nullptr;
  uint32_t *Q = nullptr;
  uint32_t *R = nullptr;
  ако ((Remainder?4:3)*n+2*m+1 <= 128) {
    U = &SPACE[0];
    V = &SPACE[m+n+1];
    Q = &SPACE[(m+n+1) + n];
    ако (Remainder)
      R = &SPACE[(m+n+1) + n + (m+n)];
  } инако {
    U = new uint32_t[m + n + 1];
    V = new uint32_t[n];
    Q = new uint32_t[m+n];
    ако (Remainder)
      R = new uint32_t[n];
  }

  // Initialize the dividend
  memset(U, 0, (m+n+1)*sizeof(uint32_t));
  за (unsigned i = 0; i < lhsWords; ++i) {
    uint64_t tmp = LHS[i];
    U[i * 2] = Lo_32(tmp);
    U[i * 2 + 1] = Hi_32(tmp);
  }
  U[m+n] = 0; // this extra word is for "spill" in the Knuth algorithm.

  // Initialize the divisor
  memset(V, 0, (n)*sizeof(uint32_t));
  за (unsigned i = 0; i < rhsWords; ++i) {
    uint64_t tmp = RHS[i];
    V[i * 2] = Lo_32(tmp);
    V[i * 2 + 1] = Hi_32(tmp);
  }

  // initialize the quotient and remainder
  memset(Q, 0, (m+n) * sizeof(uint32_t));
  ако (Remainder)
    memset(R, 0, n * sizeof(uint32_t));

  // Now, adjust m and n for the Knuth division. n is the number of words in
  // the divisor. m is the number of words by which the dividend exceeds the
  // divisor (i.e. m+n is the length of the dividend). These sizes must not
  // contain any zero words or the Knuth algorithm fails.
  за (unsigned i = n; i > 0 && V[i-1] == 0; i--) {
    n--;
    m++;
  }
  за (unsigned i = m+n; i > 0 && U[i-1] == 0; i--)
    m--;

  // If we're left with only a single word for the divisor, Knuth doesn't work
  // so we implement the short division algorithm here. This is much simpler
  // and faster because we are certain that we can divide a 64-bit quantity
  // by a 32-bit quantity at hardware speed and short division is simply a
  // series of such operations. This is just like doing short division but we
  // are using base 2^32 instead of base 10.
  assert(n != 0 && "Divide by zero?");
  ако (n == 1) {
    uint32_t divisor = V[0];
    uint32_t remainder = 0;
    за (цео i = m; i >= 0; i--) {
      uint64_t partial_dividend = Make_64(remainder, U[i]);
      ако (partial_dividend == 0) {
        Q[i] = 0;
        remainder = 0;
      } инако ако (partial_dividend < divisor) {
        Q[i] = 0;
        remainder = Lo_32(partial_dividend);
      } инако ако (partial_dividend == divisor) {
        Q[i] = 1;
        remainder = 0;
      } инако {
        Q[i] = Lo_32(partial_dividend / divisor);
        remainder = Lo_32(partial_dividend - (Q[i] * divisor));
      }
    }
    ако (R)
      R[0] = remainder;
  } инако {
    // Now we're ready to invoke the Knuth classical divide algorithm. In this
    // case n > 1.
    KnuthDiv(U, V, Q, R, m, n);
  }

  // If the caller wants the quotient
  ако (Quotient) {
    за (unsigned i = 0; i < lhsWords; ++i)
      Quotient[i] = Make_64(Q[i*2+1], Q[i*2]);
  }

  // If the caller wants the remainder
  ако (Remainder) {
    за (unsigned i = 0; i < rhsWords; ++i)
      Remainder[i] = Make_64(R[i*2+1], R[i*2]);
  }

  // Clean up the memory we allocated.
  ако (U != &SPACE[0]) {
    delete [] U;
    delete [] V;
    delete [] Q;
    delete [] R;
  }
}

APInt APInt::udiv(const APInt &RHS) const {
  assert(BitWidth == RHS.BitWidth && "Bit widths must be the same");

  // First, deal with the easy case
  ако (LLVM_LIKELY(isSingleWord())) {
    assert(RHS.U.VAL != 0 && "Divide by zero?");
    врати APInt(BitWidth, U.VAL / RHS.U.VAL);
  }

  // Get some facts about the LHS and RHS number of bits and words
  unsigned lhsWords = getNumWords(getActiveBits());
  unsigned rhsBits  = RHS.getActiveBits();
  unsigned rhsWords = getNumWords(rhsBits);
  assert(rhsWords && "Divided by zero???");

  // Deal with some degenerate cases
  ако (!lhsWords)
    // 0 / X ===> 0
    врати APInt(BitWidth, 0);
  ако (rhsBits == 1)
    // X / 1 ===> X
    врати *this;
  ако (lhsWords < rhsWords || this->ult(RHS))
    // X / Y ===> 0, iff X < Y
    врати APInt(BitWidth, 0);
  ако (*this == RHS)
    // X / X ===> 1
    врати APInt(BitWidth, 1);
  ако (lhsWords == 1) // rhsWords is 1 if lhsWords is 1.
    // All high words are zero, just use native divide
    врати APInt(BitWidth, this->U.pVal[0] / RHS.U.pVal[0]);

  // We have to compute it the hard way. Invoke the Knuth divide algorithm.
  APInt Quotient(BitWidth, 0); // to hold result.
  divide(U.pVal, lhsWords, RHS.U.pVal, rhsWords, Quotient.U.pVal, nullptr);
  врати Quotient;
}

APInt APInt::udiv(uint64_t RHS) const {
  assert(RHS != 0 && "Divide by zero?");

  // First, deal with the easy case
  ако (LLVM_LIKELY(isSingleWord()))
    врати APInt(BitWidth, U.VAL / RHS);

  // Get some facts about the LHS words.
  unsigned lhsWords = getNumWords(getActiveBits());

  // Deal with some degenerate cases
  ако (!lhsWords)
    // 0 / X ===> 0
    врати APInt(BitWidth, 0);
  ако (RHS == 1)
    // X / 1 ===> X
    врати *this;
  ако (this->ult(RHS))
    // X / Y ===> 0, iff X < Y
    врати APInt(BitWidth, 0);
  ако (*this == RHS)
    // X / X ===> 1
    врати APInt(BitWidth, 1);
  ако (lhsWords == 1) // rhsWords is 1 if lhsWords is 1.
    // All high words are zero, just use native divide
    врати APInt(BitWidth, this->U.pVal[0] / RHS);

  // We have to compute it the hard way. Invoke the Knuth divide algorithm.
  APInt Quotient(BitWidth, 0); // to hold result.
  divide(U.pVal, lhsWords, &RHS, 1, Quotient.U.pVal, nullptr);
  врати Quotient;
}

APInt APInt::sdiv(const APInt &RHS) const {
  ако (isNegative()) {
    ако (RHS.isNegative())
      врати (-(*this)).udiv(-RHS);
    врати -((-(*this)).udiv(RHS));
  }
  ако (RHS.isNegative())
    врати -(this->udiv(-RHS));
  врати this->udiv(RHS);
}

APInt APInt::sdiv(int64_t RHS) const {
  ако (isNegative()) {
    ако (RHS < 0)
      врати (-(*this)).udiv(-RHS);
    врати -((-(*this)).udiv(RHS));
  }
  ако (RHS < 0)
    врати -(this->udiv(-RHS));
  врати this->udiv(RHS);
}

APInt APInt::urem(const APInt &RHS) const {
  assert(BitWidth == RHS.BitWidth && "Bit widths must be the same");
  ако (LLVM_LIKELY(isSingleWord())) {
    assert(RHS.U.VAL != 0 && "Remainder by zero?");
    врати APInt(BitWidth, U.VAL % RHS.U.VAL);
  }

  // Get some facts about the LHS
  unsigned lhsWords = getNumWords(getActiveBits());

  // Get some facts about the RHS
  unsigned rhsBits = RHS.getActiveBits();
  unsigned rhsWords = getNumWords(rhsBits);
  assert(rhsWords && "Performing remainder operation by zero ???");

  // Check the degenerate cases
  ако (lhsWords == 0)
    // 0 % Y ===> 0
    врати APInt(BitWidth, 0);
  ако (rhsBits == 1)
    // X % 1 ===> 0
    врати APInt(BitWidth, 0);
  ако (lhsWords < rhsWords || this->ult(RHS))
    // X % Y ===> X, iff X < Y
    врати *this;
  ако (*this == RHS)
    // X % X == 0;
    врати APInt(BitWidth, 0);
  ако (lhsWords == 1)
    // All high words are zero, just use native remainder
    врати APInt(BitWidth, U.pVal[0] % RHS.U.pVal[0]);
  ако (RHS.isPowerOf2()) {
    // X % 2^w ===> X & (2^w - 1)
    APInt Result(*this);
    Result.clearBits(RHS.logBase2(), BitWidth);
    врати Result;
  }

  // We have to compute it the hard way. Invoke the Knuth divide algorithm.
  APInt Remainder(BitWidth, 0);
  divide(U.pVal, lhsWords, RHS.U.pVal, rhsWords, nullptr, Remainder.U.pVal);
  врати Remainder;
}

uint64_t APInt::urem(uint64_t RHS) const {
  assert(RHS != 0 && "Remainder by zero?");

  ако (LLVM_LIKELY(isSingleWord()))
    врати U.VAL % RHS;

  // Get some facts about the LHS
  unsigned lhsWords = getNumWords(getActiveBits());

  // Check the degenerate cases
  ако (lhsWords == 0)
    // 0 % Y ===> 0
    врати 0;
  ако (RHS == 1)
    // X % 1 ===> 0
    врати 0;
  ако (this->ult(RHS))
    // X % Y ===> X, iff X < Y
    врати getZExtValue();
  ако (*this == RHS)
    // X % X == 0;
    врати 0;
  ако (lhsWords == 1)
    // All high words are zero, just use native remainder
    врати U.pVal[0] % RHS;
  ако (llvm::isPowerOf2_64(RHS))
    // X % 2^w ===> X & (2^w - 1)
    врати U.pVal[0] & (RHS - 1);

  // We have to compute it the hard way. Invoke the Knuth divide algorithm.
  uint64_t Remainder;
  divide(U.pVal, lhsWords, &RHS, 1, nullptr, &Remainder);
  врати Remainder;
}

APInt APInt::srem(const APInt &RHS) const {
  ако (isNegative()) {
    ако (RHS.isNegative())
      врати -((-(*this)).urem(-RHS));
    врати -((-(*this)).urem(RHS));
  }
  ако (RHS.isNegative())
    врати this->urem(-RHS);
  врати this->urem(RHS);
}

int64_t APInt::srem(int64_t RHS) const {
  ако (isNegative()) {
    ако (RHS < 0)
      врати -((-(*this)).urem(-RHS));
    врати -((-(*this)).urem(RHS));
  }
  ако (RHS < 0)
    врати this->urem(-RHS);
  врати this->urem(RHS);
}

void APInt::udivrem(const APInt &LHS, const APInt &RHS,
                    APInt &Quotient, APInt &Remainder) {
  assert(LHS.BitWidth == RHS.BitWidth && "Bit widths must be the same");
  unsigned BitWidth = LHS.BitWidth;

  // First, deal with the easy case
  ако (LLVM_LIKELY(LHS.isSingleWord())) {
    assert(RHS.U.VAL != 0 && "Divide by zero?");
    uint64_t QuotVal = LHS.U.VAL / RHS.U.VAL;
    uint64_t RemVal = LHS.U.VAL % RHS.U.VAL;
    Quotient = APInt(BitWidth, QuotVal);
    Remainder = APInt(BitWidth, RemVal);
    врати;
  }

  // Get some size facts about the dividend and divisor
  unsigned lhsWords = getNumWords(LHS.getActiveBits());
  unsigned rhsBits  = RHS.getActiveBits();
  unsigned rhsWords = getNumWords(rhsBits);
  assert(rhsWords && "Performing divrem operation by zero ???");

  // Check the degenerate cases
  ако (lhsWords == 0) {
    Quotient = APInt(BitWidth, 0);    // 0 / Y ===> 0
    Remainder = APInt(BitWidth, 0);   // 0 % Y ===> 0
    врати;
  }

  ако (rhsBits == 1) {
    Quotient = LHS;                   // X / 1 ===> X
    Remainder = APInt(BitWidth, 0);   // X % 1 ===> 0
  }

  ако (lhsWords < rhsWords || LHS.ult(RHS)) {
    Remainder = LHS;                  // X % Y ===> X, iff X < Y
    Quotient = APInt(BitWidth, 0);    // X / Y ===> 0, iff X < Y
    врати;
  }

  ако (LHS == RHS) {
    Quotient  = APInt(BitWidth, 1);   // X / X ===> 1
    Remainder = APInt(BitWidth, 0);   // X % X ===> 0;
    врати;
  }

  // Make sure there is enough space to hold the results.
  // NOTE: This assumes that reallocate won't affect any bits if it doesn't
  // change the size. This is necessary if Quotient or Remainder is aliased
  // with LHS or RHS.
  Quotient.reallocate(BitWidth);
  Remainder.reallocate(BitWidth);

  ако (lhsWords == 1) { // rhsWords is 1 if lhsWords is 1.
    // There is only one word to consider so use the native versions.
    uint64_t lhsValue = LHS.U.pVal[0];
    uint64_t rhsValue = RHS.U.pVal[0];
    Quotient = lhsValue / rhsValue;
    Remainder = lhsValue % rhsValue;
    врати;
  }

  // Okay, lets do it the long way
  divide(LHS.U.pVal, lhsWords, RHS.U.pVal, rhsWords, Quotient.U.pVal,
         Remainder.U.pVal);
  // Clear the rest of the Quotient and Remainder.
  std::memset(Quotient.U.pVal + lhsWords, 0,
              (getNumWords(BitWidth) - lhsWords) * APINT_WORD_SIZE);
  std::memset(Remainder.U.pVal + rhsWords, 0,
              (getNumWords(BitWidth) - rhsWords) * APINT_WORD_SIZE);
}

void APInt::udivrem(const APInt &LHS, uint64_t RHS, APInt &Quotient,
                    uint64_t &Remainder) {
  assert(RHS != 0 && "Divide by zero?");
  unsigned BitWidth = LHS.BitWidth;

  // First, deal with the easy case
  ако (LLVM_LIKELY(LHS.isSingleWord())) {
    uint64_t QuotVal = LHS.U.VAL / RHS;
    Remainder = LHS.U.VAL % RHS;
    Quotient = APInt(BitWidth, QuotVal);
    врати;
  }

  // Get some size facts about the dividend and divisor
  unsigned lhsWords = getNumWords(LHS.getActiveBits());

  // Check the degenerate cases
  ако (lhsWords == 0) {
    Quotient = APInt(BitWidth, 0);    // 0 / Y ===> 0
    Remainder = 0;                    // 0 % Y ===> 0
    врати;
  }

  ако (RHS == 1) {
    Quotient = LHS;                   // X / 1 ===> X
    Remainder = 0;                    // X % 1 ===> 0
    врати;
  }

  ако (LHS.ult(RHS)) {
    Remainder = LHS.getZExtValue();   // X % Y ===> X, iff X < Y
    Quotient = APInt(BitWidth, 0);    // X / Y ===> 0, iff X < Y
    врати;
  }

  ако (LHS == RHS) {
    Quotient  = APInt(BitWidth, 1);   // X / X ===> 1
    Remainder = 0;                    // X % X ===> 0;
    врати;
  }

  // Make sure there is enough space to hold the results.
  // NOTE: This assumes that reallocate won't affect any bits if it doesn't
  // change the size. This is necessary if Quotient is aliased with LHS.
  Quotient.reallocate(BitWidth);

  ако (lhsWords == 1) { // rhsWords is 1 if lhsWords is 1.
    // There is only one word to consider so use the native versions.
    uint64_t lhsValue = LHS.U.pVal[0];
    Quotient = lhsValue / RHS;
    Remainder = lhsValue % RHS;
    врати;
  }

  // Okay, lets do it the long way
  divide(LHS.U.pVal, lhsWords, &RHS, 1, Quotient.U.pVal, &Remainder);
  // Clear the rest of the Quotient.
  std::memset(Quotient.U.pVal + lhsWords, 0,
              (getNumWords(BitWidth) - lhsWords) * APINT_WORD_SIZE);
}

void APInt::sdivrem(const APInt &LHS, const APInt &RHS,
                    APInt &Quotient, APInt &Remainder) {
  ако (LHS.isNegative()) {
    ако (RHS.isNegative())
      APInt::udivrem(-LHS, -RHS, Quotient, Remainder);
    инако {
      APInt::udivrem(-LHS, RHS, Quotient, Remainder);
      Quotient.negate();
    }
    Remainder.negate();
  } инако ако (RHS.isNegative()) {
    APInt::udivrem(LHS, -RHS, Quotient, Remainder);
    Quotient.negate();
  } инако {
    APInt::udivrem(LHS, RHS, Quotient, Remainder);
  }
}

void APInt::sdivrem(const APInt &LHS, int64_t RHS,
                    APInt &Quotient, int64_t &Remainder) {
  uint64_t R = Remainder;
  ако (LHS.isNegative()) {
    ако (RHS < 0)
      APInt::udivrem(-LHS, -RHS, Quotient, R);
    инако {
      APInt::udivrem(-LHS, RHS, Quotient, R);
      Quotient.negate();
    }
    R = -R;
  } инако ако (RHS < 0) {
    APInt::udivrem(LHS, -RHS, Quotient, R);
    Quotient.negate();
  } инако {
    APInt::udivrem(LHS, RHS, Quotient, R);
  }
  Remainder = R;
}

APInt APInt::sadd_ov(const APInt &RHS, bool &Overflow) const {
  APInt Res = *this+RHS;
  Overflow = isNonNegative() == RHS.isNonNegative() &&
             Res.isNonNegative() != isNonNegative();
  врати Res;
}

APInt APInt::uadd_ov(const APInt &RHS, bool &Overflow) const {
  APInt Res = *this+RHS;
  Overflow = Res.ult(RHS);
  врати Res;
}

APInt APInt::ssub_ov(const APInt &RHS, bool &Overflow) const {
  APInt Res = *this - RHS;
  Overflow = isNonNegative() != RHS.isNonNegative() &&
             Res.isNonNegative() != isNonNegative();
  врати Res;
}

APInt APInt::usub_ov(const APInt &RHS, bool &Overflow) const {
  APInt Res = *this-RHS;
  Overflow = Res.ugt(*this);
  врати Res;
}

APInt APInt::sdiv_ov(const APInt &RHS, bool &Overflow) const {
  // MININT/-1  -->  overflow.
  Overflow = isMinSignedValue() && RHS.isAllOnes();
  врати sdiv(RHS);
}

APInt APInt::smul_ov(const APInt &RHS, bool &Overflow) const {
  APInt Res = *this * RHS;

  ако (RHS != 0)
    Overflow = Res.sdiv(RHS) != *this ||
               (isMinSignedValue() && RHS.isAllOnes());
  инако
    Overflow = false;
  врати Res;
}

APInt APInt::umul_ov(const APInt &RHS, bool &Overflow) const {
  ако (countl_zero() + RHS.countl_zero() + 2 <= BitWidth) {
    Overflow = true;
    врати *this * RHS;
  }

  APInt Res = lshr(1) * RHS;
  Overflow = Res.isNegative();
  Res <<= 1;
  ако ((*this)[0]) {
    Res += RHS;
    ако (Res.ult(RHS))
      Overflow = true;
  }
  врати Res;
}

APInt APInt::sshl_ov(const APInt &ShAmt, bool &Overflow) const {
  врати sshl_ov(ShAmt.getLimitedValue(getBitWidth()), Overflow);
}

APInt APInt::sshl_ov(unsigned ShAmt, bool &Overflow) const {
  Overflow = ShAmt >= getBitWidth();
  ако (Overflow)
    врати APInt(BitWidth, 0);

  ако (isNonNegative()) // Don't allow sign change.
    Overflow = ShAmt >= countl_zero();
  инако
    Overflow = ShAmt >= countl_one();

  врати *this << ShAmt;
}

APInt APInt::ushl_ov(const APInt &ShAmt, bool &Overflow) const {
  врати ushl_ov(ShAmt.getLimitedValue(getBitWidth()), Overflow);
}

APInt APInt::ushl_ov(unsigned ShAmt, bool &Overflow) const {
  Overflow = ShAmt >= getBitWidth();
  ако (Overflow)
    врати APInt(BitWidth, 0);

  Overflow = ShAmt > countl_zero();

  врати *this << ShAmt;
}

APInt APInt::sfloordiv_ov(const APInt &RHS, bool &Overflow) const {
  APInt quotient = sdiv_ov(RHS, Overflow);
  ако ((quotient * RHS != *this) && (isNegative() != RHS.isNegative()))
    врати quotient - 1;
  врати quotient;
}

APInt APInt::sadd_sat(const APInt &RHS) const {
  bool Overflow;
  APInt Res = sadd_ov(RHS, Overflow);
  ако (!Overflow)
    врати Res;

  врати isNegative() ? APInt::getSignedMinValue(BitWidth)
                      : APInt::getSignedMaxValue(BitWidth);
}

APInt APInt::uadd_sat(const APInt &RHS) const {
  bool Overflow;
  APInt Res = uadd_ov(RHS, Overflow);
  ако (!Overflow)
    врати Res;

  врати APInt::getMaxValue(BitWidth);
}

APInt APInt::ssub_sat(const APInt &RHS) const {
  bool Overflow;
  APInt Res = ssub_ov(RHS, Overflow);
  ако (!Overflow)
    врати Res;

  врати isNegative() ? APInt::getSignedMinValue(BitWidth)
                      : APInt::getSignedMaxValue(BitWidth);
}

APInt APInt::usub_sat(const APInt &RHS) const {
  bool Overflow;
  APInt Res = usub_ov(RHS, Overflow);
  ако (!Overflow)
    врати Res;

  врати APInt(BitWidth, 0);
}

APInt APInt::smul_sat(const APInt &RHS) const {
  bool Overflow;
  APInt Res = smul_ov(RHS, Overflow);
  ако (!Overflow)
    врати Res;

  // The result is negative if one and only one of inputs is negative.
  bool ResIsNegative = isNegative() ^ RHS.isNegative();

  врати ResIsNegative ? APInt::getSignedMinValue(BitWidth)
                       : APInt::getSignedMaxValue(BitWidth);
}

APInt APInt::umul_sat(const APInt &RHS) const {
  bool Overflow;
  APInt Res = umul_ov(RHS, Overflow);
  ако (!Overflow)
    врати Res;

  врати APInt::getMaxValue(BitWidth);
}

APInt APInt::sshl_sat(const APInt &RHS) const {
  врати sshl_sat(RHS.getLimitedValue(getBitWidth()));
}

APInt APInt::sshl_sat(unsigned RHS) const {
  bool Overflow;
  APInt Res = sshl_ov(RHS, Overflow);
  ако (!Overflow)
    врати Res;

  врати isNegative() ? APInt::getSignedMinValue(BitWidth)
                      : APInt::getSignedMaxValue(BitWidth);
}

APInt APInt::ushl_sat(const APInt &RHS) const {
  врати ushl_sat(RHS.getLimitedValue(getBitWidth()));
}

APInt APInt::ushl_sat(unsigned RHS) const {
  bool Overflow;
  APInt Res = ushl_ov(RHS, Overflow);
  ако (!Overflow)
    врати Res;

  врати APInt::getMaxValue(BitWidth);
}

void APInt::fromString(unsigned numbits, StringRef str, uint8_t radix) {
  // Check our assumptions here
  assert(!str.empty() && "Invalid string length");
  assert((radix == 10 || radix == 8 || radix == 16 || radix == 2 ||
          radix == 36) &&
         "Radix should be 2, 8, 10, 16, or 36!");

  StringRef::iterator p = str.begin();
  size_t slen = str.size();
  bool isNeg = *p == '-';
  ако (*p == '-' || *p == '+') {
    p++;
    slen--;
    assert(slen && "String is only a sign, needs a value.");
  }
  assert((slen <= numbits || radix != 2) && "Insufficient bit width");
  assert(((slen-1)*3 <= numbits || radix != 8) && "Insufficient bit width");
  assert(((slen-1)*4 <= numbits || radix != 16) && "Insufficient bit width");
  assert((((slen-1)*64)/22 <= numbits || radix != 10) &&
         "Insufficient bit width");

  // Allocate memory if needed
  ако (LLVM_LIKELY(isSingleWord()))
    U.VAL = 0;
  инако
    U.pVal = getClearedMemory(getNumWords());

  // Figure out if we can shift instead of multiply
  unsigned shift = (radix == 16 ? 4 : radix == 8 ? 3 : radix == 2 ? 1 : 0);

  // Enter digit traversal loop
  за (StringRef::iterator e = str.end(); p != e; ++p) {
    unsigned digit = getDigit(*p, radix);
    assert(digit < radix && "Invalid character in digit string");

    // Shift or multiply the value by the radix
    ако (slen > 1) {
      ако (shift)
        *this <<= shift;
      инако
        *this *= radix;
    }

    // Add in the digit we just interpreted
    *this += digit;
  }
  // If its negative, put it in two's complement form
  ако (isNeg)
    this->negate();
}

void APInt::toString(SmallVectorImpl<char> &Str, unsigned Radix, bool Signed,
                     bool formatAsCLiteral, bool UpperCase,
                     bool InsertSeparators) const {
  assert((Radix == 10 || Radix == 8 || Radix == 16 || Radix == 2 ||
          Radix == 36) &&
         "Radix should be 2, 8, 10, 16, or 36!");

  const char *Prefix = "";
  ако (formatAsCLiteral) {
    switch (Radix) {
      case 2:
        // Binary literals are a non-standard extension added in gcc 4.3:
        // http://gcc.gnu.org/onlinedocs/gcc-4.3.0/gcc/Binary-constants.html
        Prefix = "0b";
        break;
      case 8:
        Prefix = "0";
        break;
      case 10:
        break; // No prefix
      case 16:
        Prefix = "0x";
        break;
      default:
        llvm_unreachable("Invalid radix!");
    }
  }

  // Number of digits in a group between separators.
  unsigned Grouping = (Radix == 8 || Radix == 10) ? 3 : 4;

  // First, check for a zero value and just short circuit the logic below.
  ако (isZero()) {
    док (*Prefix) {
      Str.push_back(*Prefix);
      ++Prefix;
    };
    Str.push_back('0');
    врати;
  }

  static const char BothDigits[] = "0123456789abcdefghijklmnopqrstuvwxyz"
                                   "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ";
  const char *Digits = BothDigits + (UpperCase ? 36 : 0);

  ако (LLVM_LIKELY(isSingleWord())) {
    char Buffer[65];
    char *BufPtr = std::end(Buffer);

    uint64_t N;
    ако (!Signed) {
      N = getZExtValue();
    } инако {
      int64_t I = getSExtValue();
      ако (I >= 0) {
        N = I;
      } инако {
        Str.push_back('-');
        N = -(uint64_t)I;
      }
    }

    док (*Prefix) {
      Str.push_back(*Prefix);
      ++Prefix;
    };

    цео Pos = 0;
    док (N) {
      ако (InsertSeparators && Pos % Grouping == 0 && Pos > 0)
        *--BufPtr = '\'';
      *--BufPtr = Digits[N % Radix];
      N /= Radix;
      Pos++;
    }
    Str.append(BufPtr, std::end(Buffer));
    врати;
  }

  APInt Tmp(*this);

  ако (Signed && isNegative()) {
    // They want to print the signed version and it is a negative value
    // Flip the bits and add one to turn it into the equivalent positive
    // value and put a '-' in the result.
    Tmp.negate();
    Str.push_back('-');
  }

  док (*Prefix) {
    Str.push_back(*Prefix);
    ++Prefix;
  }

  // We insert the digits backward, then reverse them to get the right order.
  unsigned StartDig = Str.size();

  // For the 2, 8 and 16 bit cases, we can just shift instead of divide
  // because the number of bits per digit (1, 3 and 4 respectively) divides
  // equally.  We just shift until the value is zero.
  ако (Radix == 2 || Radix == 8 || Radix == 16) {
    // Just shift tmp right for each digit width until it becomes zero
    unsigned ShiftAmt = (Radix == 16 ? 4 : (Radix == 8 ? 3 : 1));
    unsigned MaskAmt = Radix - 1;

    цео Pos = 0;
    док (Tmp.getBoolValue()) {
      unsigned Digit = unsigned(Tmp.getRawData()[0]) & MaskAmt;
      ако (InsertSeparators && Pos % Grouping == 0 && Pos > 0)
        Str.push_back('\'');

      Str.push_back(Digits[Digit]);
      Tmp.lshrInPlace(ShiftAmt);
      Pos++;
    }
  } инако {
    цео Pos = 0;
    док (Tmp.getBoolValue()) {
      uint64_t Digit;
      udivrem(Tmp, Radix, Tmp, Digit);
      assert(Digit < Radix && "divide failed");
      ако (InsertSeparators && Pos % Grouping == 0 && Pos > 0)
        Str.push_back('\'');

      Str.push_back(Digits[Digit]);
      Pos++;
    }
  }

  // Reverse the digits before returning.
  std::reverse(Str.begin()+StartDig, Str.end());
}

#ако !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)
LLVM_DUMP_METHOD void APInt::dump() const {
  SmallString<40> S, U;
  this->toStringUnsigned(U);
  this->toStringSigned(S);
  dbgs() << "APInt(" << BitWidth << "b, "
         << U << "u " << S << "s)\n";
}
#endif

void APInt::print(raw_ostream &OS, bool isSigned) const {
  SmallString<40> S;
  this->toString(S, 10, isSigned, /* formatAsCLiteral = */false);
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
static inline APInt::WordType lowBitMask(unsigned bits) {
  assert(bits != 0 && bits <= APInt::APINT_BITS_PER_WORD);
  врати ~(APInt::WordType) 0 >> (APInt::APINT_BITS_PER_WORD - bits);
}

/// Returns the value of the lower half of PART.
static inline APInt::WordType lowHalf(APInt::WordType part) {
  врати part & lowBitMask(APInt::APINT_BITS_PER_WORD / 2);
}

/// Returns the value of the upper half of PART.
static inline APInt::WordType highHalf(APInt::WordType part) {
  врати part >> (APInt::APINT_BITS_PER_WORD / 2);
}

/// Sets the least significant part of a bignum to the input value, and zeroes
/// out higher parts.
void APInt::tcSet(WordType *dst, WordType part, unsigned parts) {
  assert(parts > 0);
  dst[0] = part;
  за (unsigned i = 1; i < parts; i++)
    dst[i] = 0;
}

/// Assign one bignum to another.
void APInt::tcAssign(WordType *dst, const WordType *src, unsigned parts) {
  за (unsigned i = 0; i < parts; i++)
    dst[i] = src[i];
}

/// Returns true if a bignum is zero, false otherwise.
bool APInt::tcIsZero(const WordType *src, unsigned parts) {
  за (unsigned i = 0; i < parts; i++)
    ако (src[i])
      врати false;

  врати true;
}

/// Extract the given bit of a bignum; returns 0 or 1.
цео APInt::tcExtractBit(const WordType *parts, unsigned bit) {
  врати (parts[whichWord(bit)] & maskBit(bit)) != 0;
}

/// Set the given bit of a bignum.
void APInt::tcSetBit(WordType *parts, unsigned bit) {
  parts[whichWord(bit)] |= maskBit(bit);
}

/// Clears the given bit of a bignum.
void APInt::tcClearBit(WordType *parts, unsigned bit) {
  parts[whichWord(bit)] &= ~maskBit(bit);
}

/// Returns the bit number of the least significant set bit of a number.  If the
/// input number has no bits set UINT_MAX is returned.
unsigned APInt::tcLSB(const WordType *parts, unsigned n) {
  за (unsigned i = 0; i < n; i++) {
    ако (parts[i] != 0) {
      unsigned lsb = llvm::countr_zero(parts[i]);
      врати lsb + i * APINT_BITS_PER_WORD;
    }
  }

  врати UINT_MAX;
}

/// Returns the bit number of the most significant set bit of a number.
/// If the input number has no bits set UINT_MAX is returned.
unsigned APInt::tcMSB(const WordType *parts, unsigned n) {
  do {
    --n;

    ако (parts[n] != 0) {
      static_assert(sizeof(parts[n]) <= sizeof(uint64_t));
      unsigned msb = llvm::Log2_64(parts[n]);

      врати msb + n * APINT_BITS_PER_WORD;
    }
  } док (n);

  врати UINT_MAX;
}

/// Copy the bit vector of width srcBITS from SRC, starting at bit srcLSB, to
/// DST, of dstCOUNT parts, such that the bit srcLSB becomes the least
/// significant bit of DST.  All high bits above srcBITS in DST are zero-filled.
/// */
void
APInt::tcExtract(WordType *dst, unsigned dstCount, const WordType *src,
                 unsigned srcBits, unsigned srcLSB) {
  unsigned dstParts = (srcBits + APINT_BITS_PER_WORD - 1) / APINT_BITS_PER_WORD;
  assert(dstParts <= dstCount);

  unsigned firstSrcPart = srcLSB / APINT_BITS_PER_WORD;
  tcAssign(dst, src + firstSrcPart, dstParts);

  unsigned shift = srcLSB % APINT_BITS_PER_WORD;
  tcShiftRight(dst, dstParts, shift);

  // We now have (dstParts * APINT_BITS_PER_WORD - shift) bits from SRC
  // in DST.  If this is less that srcBits, append the rest, else
  // clear the high bits.
  unsigned n = dstParts * APINT_BITS_PER_WORD - shift;
  ако (n < srcBits) {
    WordType mask = lowBitMask (srcBits - n);
    dst[dstParts - 1] |= ((src[firstSrcPart + dstParts] & mask)
                          << n % APINT_BITS_PER_WORD);
  } инако ако (n > srcBits) {
    ако (srcBits % APINT_BITS_PER_WORD)
      dst[dstParts - 1] &= lowBitMask (srcBits % APINT_BITS_PER_WORD);
  }

  // Clear high parts.
  док (dstParts < dstCount)
    dst[dstParts++] = 0;
}

//// DST += RHS + C where C is zero or one.  Returns the carry flag.
APInt::WordType APInt::tcAdd(WordType *dst, const WordType *rhs,
                             WordType c, unsigned parts) {
  assert(c <= 1);

  за (unsigned i = 0; i < parts; i++) {
    WordType l = dst[i];
    ако (c) {
      dst[i] += rhs[i] + 1;
      c = (dst[i] <= l);
    } инако {
      dst[i] += rhs[i];
      c = (dst[i] < l);
    }
  }

  врати c;
}

/// This function adds a single "word" integer, src, to the multiple
/// "word" integer array, dst[]. dst[] is modified to reflect the addition and
/// 1 is returned if there is a carry out, otherwise 0 is returned.
/// @returns the carry of the addition.
APInt::WordType APInt::tcAddPart(WordType *dst, WordType src,
                                 unsigned parts) {
  за (unsigned i = 0; i < parts; ++i) {
    dst[i] += src;
    ако (dst[i] >= src)
      врати 0; // No need to carry so exit early.
    src = 1; // Carry one to next digit.
  }

  врати 1;
}

/// DST -= RHS + C where C is zero or one.  Returns the carry flag.
APInt::WordType APInt::tcSubtract(WordType *dst, const WordType *rhs,
                                  WordType c, unsigned parts) {
  assert(c <= 1);

  за (unsigned i = 0; i < parts; i++) {
    WordType l = dst[i];
    ако (c) {
      dst[i] -= rhs[i] + 1;
      c = (dst[i] >= l);
    } инако {
      dst[i] -= rhs[i];
      c = (dst[i] > l);
    }
  }

  врати c;
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
  за (unsigned i = 0; i < parts; ++i) {
    WordType Dst = dst[i];
    dst[i] -= src;
    ако (src <= Dst)
      врати 0; // No need to borrow so exit early.
    src = 1; // We have to "borrow 1" from next "word"
  }

  врати 1;
}

/// Negate a bignum in-place.
void APInt::tcNegate(WordType *dst, unsigned parts) {
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
цео APInt::tcMultiplyPart(WordType *dst, const WordType *src,
                          WordType multiplier, WordType carry,
                          unsigned srcParts, unsigned dstParts,
                          bool add) {
  // Otherwise our writes of DST kill our later reads of SRC.
  assert(dst <= src || dst >= src + srcParts);
  assert(dstParts <= srcParts + 1);

  // N loops; minimum of dstParts and srcParts.
  unsigned n = std::min(dstParts, srcParts);

  за (unsigned i = 0; i < n; i++) {
    // [LOW, HIGH] = MULTIPLIER * SRC[i] + DST[i] + CARRY.
    // This cannot overflow, because:
    //   (n - 1) * (n - 1) + 2 (n - 1) = (n - 1) * (n + 1)
    // which is less than n^2.
    WordType srcPart = src[i];
    WordType low, mid, high;
    ако (multiplier == 0 || srcPart == 0) {
      low = carry;
      high = 0;
    } инако {
      low = lowHalf(srcPart) * lowHalf(multiplier);
      high = highHalf(srcPart) * highHalf(multiplier);

      mid = lowHalf(srcPart) * highHalf(multiplier);
      high += highHalf(mid);
      mid <<= APINT_BITS_PER_WORD / 2;
      ако (low + mid < low)
        high++;
      low += mid;

      mid = highHalf(srcPart) * lowHalf(multiplier);
      high += highHalf(mid);
      mid <<= APINT_BITS_PER_WORD / 2;
      ако (low + mid < low)
        high++;
      low += mid;

      // Now add carry.
      ако (low + carry < low)
        high++;
      low += carry;
    }

    ако (add) {
      // And now DST[i], and store the new low part there.
      ако (low + dst[i] < low)
        high++;
      dst[i] += low;
    } инако {
      dst[i] = low;
    }

    carry = high;
  }

  ако (srcParts < dstParts) {
    // Full multiplication, there is no overflow.
    assert(srcParts + 1 == dstParts);
    dst[srcParts] = carry;
    врати 0;
  }

  // We overflowed if there is carry.
  ако (carry)
    врати 1;

  // We would overflow if any significant unwritten parts would be
  // non-zero.  This is true if any remaining src parts are non-zero
  // and the multiplier is non-zero.
  ако (multiplier)
    за (unsigned i = dstParts; i < srcParts; i++)
      ако (src[i])
        врати 1;

  // We fitted in the narrow destination.
  врати 0;
}

/// DST = LHS * RHS, where DST has the same width as the operands and
/// is filled with the least significant parts of the result.  Returns
/// one if overflow occurred, otherwise zero.  DST must be disjoint
/// from both operands.
цео APInt::tcMultiply(WordType *dst, const WordType *lhs,
                      const WordType *rhs, unsigned parts) {
  assert(dst != lhs && dst != rhs);

  цео overflow = 0;

  за (unsigned i = 0; i < parts; i++) {
    // Don't accumulate on the first iteration so we don't need to initalize
    // dst to 0.
    overflow |=
        tcMultiplyPart(&dst[i], lhs, rhs[i], 0, parts, parts - i, i != 0);
  }

  врати overflow;
}

/// DST = LHS * RHS, where DST has width the sum of the widths of the
/// operands. No overflow occurs. DST must be disjoint from both operands.
void APInt::tcFullMultiply(WordType *dst, const WordType *lhs,
                           const WordType *rhs, unsigned lhsParts,
                           unsigned rhsParts) {
  // Put the narrower number on the LHS for less loops below.
  ако (lhsParts > rhsParts)
    врати tcFullMultiply (dst, rhs, lhs, rhsParts, lhsParts);

  assert(dst != lhs && dst != rhs);

  за (unsigned i = 0; i < lhsParts; i++) {
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
цео APInt::tcDivide(WordType *lhs, const WordType *rhs,
                    WordType *remainder, WordType *srhs,
                    unsigned parts) {
  assert(lhs != remainder && lhs != srhs && remainder != srhs);

  unsigned shiftCount = tcMSB(rhs, parts) + 1;
  ако (shiftCount == 0)
    врати true;

  shiftCount = parts * APINT_BITS_PER_WORD - shiftCount;
  unsigned n = shiftCount / APINT_BITS_PER_WORD;
  WordType mask = (WordType) 1 << (shiftCount % APINT_BITS_PER_WORD);

  tcAssign(srhs, rhs, parts);
  tcShiftLeft(srhs, parts, shiftCount);
  tcAssign(remainder, lhs, parts);
  tcSet(lhs, 0, parts);

  // Loop, subtracting SRHS if REMAINDER is greater and adding that to the
  // total.
  за (;;) {
    цео compare = tcCompare(remainder, srhs, parts);
    ако (compare >= 0) {
      tcSubtract(remainder, srhs, 0, parts);
      lhs[n] |= mask;
    }

    ако (shiftCount == 0)
      break;
    shiftCount--;
    tcShiftRight(srhs, parts, 1);
    ако ((mask >>= 1) == 0) {
      mask = (WordType) 1 << (APINT_BITS_PER_WORD - 1);
      n--;
    }
  }

  врати false;
}

/// Shift a bignum left Count bits in-place. Shifted in bits are zero. There are
/// no restrictions on Count.
void APInt::tcShiftLeft(WordType *Dst, unsigned Words, unsigned Count) {
  // Don't bother performing a no-op shift.
  ако (!Count)
    врати;

  // WordShift is the inter-part shift; BitShift is the intra-part shift.
  unsigned WordShift = std::min(Count / APINT_BITS_PER_WORD, Words);
  unsigned BitShift = Count % APINT_BITS_PER_WORD;

  // Fastpath for moving by whole words.
  ако (BitShift == 0) {
    std::memmove(Dst + WordShift, Dst, (Words - WordShift) * APINT_WORD_SIZE);
  } инако {
    док (Words-- > WordShift) {
      Dst[Words] = Dst[Words - WordShift] << BitShift;
      ако (Words > WordShift)
        Dst[Words] |=
          Dst[Words - WordShift - 1] >> (APINT_BITS_PER_WORD - BitShift);
    }
  }

  // Fill in the remainder with 0s.
  std::memset(Dst, 0, WordShift * APINT_WORD_SIZE);
}

/// Shift a bignum right Count bits in-place. Shifted in bits are zero. There
/// are no restrictions on Count.
void APInt::tcShiftRight(WordType *Dst, unsigned Words, unsigned Count) {
  // Don't bother performing a no-op shift.
  ако (!Count)
    врати;

  // WordShift is the inter-part shift; BitShift is the intra-part shift.
  unsigned WordShift = std::min(Count / APINT_BITS_PER_WORD, Words);
  unsigned BitShift = Count % APINT_BITS_PER_WORD;

  unsigned WordsToMove = Words - WordShift;
  // Fastpath for moving by whole words.
  ако (BitShift == 0) {
    std::memmove(Dst, Dst + WordShift, WordsToMove * APINT_WORD_SIZE);
  } инако {
    за (unsigned i = 0; i != WordsToMove; ++i) {
      Dst[i] = Dst[i + WordShift] >> BitShift;
      ако (i + 1 != WordsToMove)
        Dst[i] |= Dst[i + WordShift + 1] << (APINT_BITS_PER_WORD - BitShift);
    }
  }

  // Fill in the remainder with 0s.
  std::memset(Dst + WordsToMove, 0, WordShift * APINT_WORD_SIZE);
}

// Comparison (unsigned) of two bignums.
цео APInt::tcCompare(const WordType *lhs, const WordType *rhs,
                     unsigned parts) {
  док (parts) {
    parts--;
    ако (lhs[parts] != rhs[parts])
      врати (lhs[parts] > rhs[parts]) ? 1 : -1;
  }

  врати 0;
}

APInt llvm::APIntOps::RoundingUDiv(const APInt &A, const APInt &B,
                                   APInt::Rounding RM) {
  // Currently udivrem always rounds down.
  switch (RM) {
  case APInt::Rounding::DOWN:
  case APInt::Rounding::TOWARD_ZERO:
    врати A.udiv(B);
  case APInt::Rounding::UP: {
    APInt Quo, Rem;
    APInt::udivrem(A, B, Quo, Rem);
    ако (Rem.isZero())
      врати Quo;
    врати Quo + 1;
  }
  }
  llvm_unreachable("Unknown APInt::Rounding enum");
}

APInt llvm::APIntOps::RoundingSDiv(const APInt &A, const APInt &B,
                                   APInt::Rounding RM) {
  switch (RM) {
  case APInt::Rounding::DOWN:
  case APInt::Rounding::UP: {
    APInt Quo, Rem;
    APInt::sdivrem(A, B, Quo, Rem);
    ако (Rem.isZero())
      врати Quo;
    // This algorithm deals with arbitrary rounding mode used by sdivrem.
    // We want to check whether the non-integer part of the mathematical value
    // is negative or not. If the non-integer part is negative, we need to round
    // down from Quo; otherwise, if it's positive or 0, we return Quo, as it's
    // already rounded down.
    ако (RM == APInt::Rounding::DOWN) {
      ако (Rem.isNegative() != B.isNegative())
        врати Quo - 1;
      врати Quo;
    }
    ако (Rem.isNegative() != B.isNegative())
      врати Quo;
    врати Quo + 1;
  }
  // Currently sdiv rounds towards zero.
  case APInt::Rounding::TOWARD_ZERO:
    врати A.sdiv(B);
  }
  llvm_unreachable("Unknown APInt::Rounding enum");
}

std::optional<APInt>
llvm::APIntOps::SolveQuadraticEquationWrap(APInt A, APInt B, APInt C,
                                           unsigned RangeWidth) {
  unsigned CoeffWidth = A.getBitWidth();
  assert(CoeffWidth == B.getBitWidth() && CoeffWidth == C.getBitWidth());
  assert(RangeWidth <= CoeffWidth &&
         "Value range width should be less than coefficient width");
  assert(RangeWidth > 1 && "Value range bit width should be > 1");

  LLVM_DEBUG(dbgs() << __func__ << ": solving " << A << "x^2 + " << B
                    << "x + " << C << ", rw:" << RangeWidth << '\n');

  // Identify 0 as a (non)solution immediately.
  ако (C.sextOrTrunc(RangeWidth).isZero()) {
    LLVM_DEBUG(dbgs() << __func__ << ": zero solution\n");
    врати APInt(CoeffWidth, 0);
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
  ако (A.isNegative()) {
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
  bool PickLow;

  auto RoundUp = [] (const APInt &V, const APInt &A) -> APInt {
    assert(A.isStrictlyPositive());
    APInt T = V.abs().urem(A);
    ако (T.isZero())
      врати V;
    врати V.isNegative() ? V+T : V+(A-T);
  };

  // The vertex of the parabola is at -B/2A, but since A > 0, it's negative
  // iff B is positive.
  ако (B.isNonNegative()) {
    // If B >= 0, the vertex it at a negative location (or at 0), so in
    // order to have a non-negative solution we need to pick k that makes
    // C-kR negative. To satisfy all the requirements for the solution
    // that we are looking for, it needs to be closest to 0 of all k.
    C = C.srem(R);
    ако (C.isStrictlyPositive())
      C -= R;
    // Pick the greater solution.
    PickLow = false;
  } инако {
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
    ако (C.sgt(LowkR)) {
      // If LowkR < C, then such a k is guaranteed to exist because
      // LowkR itself is a multiple of R.
      C -= -RoundUp(-C, R);      // C = C - RoundDown(C, R)
      // Pick the smaller solution.
      PickLow = true;
    } инако {
      // If C-kR < 0 for all potential k's, it means that one solution
      // will be negative, while the other will be positive. The positive
      // solution will shift towards 0 if the parabola is moved up.
      // Pick the kR closest to the lower bound (i.e. make C-kR closest
      // to 0, or in other words, out of all parabolas that have solutions,
      // pick the one that is the farthest "up").
      // Since LowkR is itself a multiple of R, simply take C-LowkR.
      C -= LowkR;
      // Pick the greater solution.
      PickLow = false;
    }
  }

  LLVM_DEBUG(dbgs() << __func__ << ": updated coefficients " << A << "x^2 + "
                    << B << "x + " << C << ", rw:" << RangeWidth << '\n');

  APInt D = SqrB - 4*A*C;
  assert(D.isNonNegative() && "Negative discriminant");
  APInt SQ = D.sqrtFloor();

  APInt Q = SQ * SQ;
  bool InexactSQ = Q != D;

  APInt X;
  APInt Rem;

  // SQ is rounded down (i.e SQ * SQ <= D), so the roots may be inexact.
  // When using the quadratic formula directly, the calculated low root
  // may be greater than the exact one, since we would be subtracting SQ.
  // To make sure that the calculated root is not greater than the exact
  // one, subtract SQ+1 when calculating the low root (for inexact value
  // of SQ).
  ако (PickLow)
    APInt::sdivrem(-B - (SQ+InexactSQ), TwoA, X, Rem);
  инако
    APInt::sdivrem(-B + SQ, TwoA, X, Rem);

  // The updated coefficients should be such that the (exact) solution is
  // positive. Since APInt division rounds towards 0, the calculated one
  // can be 0, but cannot be negative.
  assert(X.isNonNegative() && "Solution should be non-negative");

  ако (!InexactSQ && Rem.isZero()) {
    LLVM_DEBUG(dbgs() << __func__ << ": solution (root): " << X << '\n');
    врати X;
  }

  assert((SQ*SQ).sle(D) && "SQ = |_sqrt(D)_|, so SQ*SQ <= D");
  // The exact value of the square root of D should be between SQ and SQ+1.
  // This implies that the solution should be between that corresponding to
  // SQ (i.e. X) and that corresponding to SQ+1.
  //
  // The calculated X cannot be greater than the exact (real) solution.
  // Actually it must be strictly less than the exact solution, while
  // X+1 will be greater than or equal to it.

  APInt VX = (A*X + B)*X + C;
  APInt VY = VX + TwoA*X + A + B;
  bool SignChange =
      VX.isNegative() != VY.isNegative() || VX.isZero() != VY.isZero();
  // If the sign did not change between X and X+1, X is not a valid solution.
  // This could happen when the actual (exact) roots don't have an integer
  // between them, so they would both be contained between X and X+1.
  ако (!SignChange) {
    LLVM_DEBUG(dbgs() << __func__ << ": no valid solution\n");
    врати std::nullopt;
  }

  X += 1;
  LLVM_DEBUG(dbgs() << __func__ << ": solution (wrap): " << X << '\n');
  врати X;
}

std::optional<unsigned>
llvm::APIntOps::GetMostSignificantDifferentBit(const APInt &A, const APInt &B) {
  assert(A.getBitWidth() == B.getBitWidth() && "Must have the same bitwidth");
  ако (A == B)
    врати std::nullopt;
  врати A.getBitWidth() - ((A ^ B).countl_zero() + 1);
}

APInt llvm::APIntOps::ScaleBitMask(const APInt &A, unsigned NewBitWidth,
                                   bool MatchAllBits) {
  unsigned OldBitWidth = A.getBitWidth();
  assert((((OldBitWidth % NewBitWidth) == 0) ||
          ((NewBitWidth % OldBitWidth) == 0)) &&
         "One size should be a multiple of the other one. "
         "Can't do fractional scaling.");

  // Check for matching bitwidths.
  ако (OldBitWidth == NewBitWidth)
    врати A;

  APInt NewA = APInt::getZero(NewBitWidth);

  // Check for null input.
  ако (A.isZero())
    врати NewA;

  ако (NewBitWidth > OldBitWidth) {
    // Repeat bits.
    unsigned Scale = NewBitWidth / OldBitWidth;
    за (unsigned i = 0; i != OldBitWidth; ++i)
      ако (A[i])
        NewA.setBits(i * Scale, (i + 1) * Scale);
  } инако {
    unsigned Scale = OldBitWidth / NewBitWidth;
    за (unsigned i = 0; i != NewBitWidth; ++i) {
      ако (MatchAllBits) {
        ако (A.extractBits(Scale, i * Scale).isAllOnes())
          NewA.setBit(i);
      } инако {
        ако (!A.extractBits(Scale, i * Scale).isZero())
          NewA.setBit(i);
      }
    }
  }

  врати NewA;
}

/// StoreIntToMemory - Fills the StoreBytes bytes of memory starting from Dst
/// with the integer held in IntVal.
void llvm::StoreIntToMemory(const APInt &IntVal, uint8_t *Dst,
                            unsigned StoreBytes) {
  assert((IntVal.getBitWidth()+7)/8 >= StoreBytes && "Integer too small!");
  const uint8_t *Src = (const uint8_t *)IntVal.getRawData();

  ако (sys::IsLittleEndianHost) {
    // Little-endian host - the source is ordered from LSB to MSB.  Order the
    // destination from LSB to MSB: Do a straight copy.
    memcpy(Dst, Src, StoreBytes);
  } инако {
    // Big-endian host - the source is an array of 64 bit words ordered from
    // LSW to MSW.  Each word is ordered from MSB to LSB.  Order the destination
    // from MSB to LSB: Reverse the word order, but not the bytes in a word.
    док (StoreBytes > sizeof(uint64_t)) {
      StoreBytes -= sizeof(uint64_t);
      // May not be aligned so use memcpy.
      memcpy(Dst + StoreBytes, Src, sizeof(uint64_t));
      Src += sizeof(uint64_t);
    }

    memcpy(Dst, Src + sizeof(uint64_t) - StoreBytes, StoreBytes);
  }
}

/// LoadIntFromMemory - Loads the integer stored in the LoadBytes bytes starting
/// from Src into IntVal, which is assumed to be wide enough and to hold zero.
void llvm::LoadIntFromMemory(APInt &IntVal, const uint8_t *Src,
                             unsigned LoadBytes) {
  assert((IntVal.getBitWidth()+7)/8 >= LoadBytes && "Integer too small!");
  uint8_t *Dst = reinterpret_cast<uint8_t *>(
                   const_cast<uint64_t *>(IntVal.getRawData()));

  ако (sys::IsLittleEndianHost)
    // Little-endian host - the destination must be ordered from LSB to MSB.
    // The source is ordered from LSB to MSB: Do a straight copy.
    memcpy(Dst, Src, LoadBytes);
  инако {
    // Big-endian - the destination is an array of 64 bit words ordered from
    // LSW to MSW.  Each word must be ordered from MSB to LSB.  The source is
    // ordered from MSB to LSB: Reverse the word order, but not the bytes in
    // a word.
    док (LoadBytes > sizeof(uint64_t)) {
      LoadBytes -= sizeof(uint64_t);
      // May not be aligned so use memcpy.
      memcpy(Dst, Src + LoadBytes, sizeof(uint64_t));
      Dst += sizeof(uint64_t);
    }

    memcpy(Dst + sizeof(uint64_t) - LoadBytes, Src, LoadBytes);
  }
}

APInt APIntOps::avgFloorS(const APInt &C1, const APInt &C2) {
  // Return floor((C1 + C2) / 2)
  врати (C1 & C2) + (C1 ^ C2).ashr(1);
}

APInt APIntOps::avgFloorU(const APInt &C1, const APInt &C2) {
  // Return floor((C1 + C2) / 2)
  врати (C1 & C2) + (C1 ^ C2).lshr(1);
}

APInt APIntOps::avgCeilS(const APInt &C1, const APInt &C2) {
  // Return ceil((C1 + C2) / 2)
  врати (C1 | C2) - (C1 ^ C2).ashr(1);
}

APInt APIntOps::avgCeilU(const APInt &C1, const APInt &C2) {
  // Return ceil((C1 + C2) / 2)
  врати (C1 | C2) - (C1 ^ C2).lshr(1);
}

APInt APIntOps::avgSubFloorS(const APInt &C1, const APInt &C2) {
  // Return floor((C1 - C2) / 2), using C1 - C2 == (C1 ^ C2) - 2 * (~C1 & C2).
  врати (C1 ^ C2).ashr(1) - (~C1 & C2);
}

APInt APIntOps::avgSubFloorU(const APInt &C1, const APInt &C2) {
  // Return floor((C1 - C2) / 2), using C1 - C2 == (C1 ^ C2) - 2 * (~C1 & C2).
  врати (C1 ^ C2).lshr(1) - (~C1 & C2);
}

APInt APIntOps::mulhs(const APInt &C1, const APInt &C2) {
  assert(C1.getBitWidth() == C2.getBitWidth() && "Unequal bitwidths");
  unsigned FullWidth = C1.getBitWidth() * 2;
  APInt C1Ext = C1.sext(FullWidth);
  APInt C2Ext = C2.sext(FullWidth);
  врати (C1Ext * C2Ext).extractBits(C1.getBitWidth(), C1.getBitWidth());
}

APInt APIntOps::mulhu(const APInt &C1, const APInt &C2) {
  assert(C1.getBitWidth() == C2.getBitWidth() && "Unequal bitwidths");
  unsigned FullWidth = C1.getBitWidth() * 2;
  APInt C1Ext = C1.zext(FullWidth);
  APInt C2Ext = C2.zext(FullWidth);
  врати (C1Ext * C2Ext).extractBits(C1.getBitWidth(), C1.getBitWidth());
}

APInt APIntOps::mulsExtended(const APInt &C1, const APInt &C2) {
  assert(C1.getBitWidth() == C2.getBitWidth() && "Unequal bitwidths");
  unsigned FullWidth = C1.getBitWidth() * 2;
  APInt C1Ext = C1.sext(FullWidth);
  APInt C2Ext = C2.sext(FullWidth);
  врати C1Ext * C2Ext;
}

APInt APIntOps::muluExtended(const APInt &C1, const APInt &C2) {
  assert(C1.getBitWidth() == C2.getBitWidth() && "Unequal bitwidths");
  unsigned FullWidth = C1.getBitWidth() * 2;
  APInt C1Ext = C1.zext(FullWidth);
  APInt C2Ext = C2.zext(FullWidth);
  врати C1Ext * C2Ext;
}

APInt APIntOps::pow(const APInt &X, int64_t N) {
  assert(N >= 0 && "negative exponents not supported.");
  APInt Acc = APInt(X.getBitWidth(), 1);
  ако (N == 0)
    врати Acc;
  APInt Base = X;
  int64_t RemainingExponent = N;
  док (RemainingExponent > 0) {
    док (RemainingExponent % 2 == 0) {
      Base *= Base;
      RemainingExponent /= 2;
    }
    --RemainingExponent;
    Acc *= Base;
  }
  врати Acc;
}

APInt llvm::APIntOps::fshl(const APInt &Hi, const APInt &Lo,
                           const APInt &Shift) {
  assert(Hi.getBitWidth() == Lo.getBitWidth());
  unsigned ShiftAmt = rotateModulo(Hi.getBitWidth(), Shift);
  ако (ShiftAmt == 0)
    врати Hi;
  врати Hi.shl(ShiftAmt) | Lo.lshr(Hi.getBitWidth() - ShiftAmt);
}

APInt llvm::APIntOps::fshr(const APInt &Hi, const APInt &Lo,
                           const APInt &Shift) {
  assert(Hi.getBitWidth() == Lo.getBitWidth());
  unsigned ShiftAmt = rotateModulo(Hi.getBitWidth(), Shift);
  ако (ShiftAmt == 0)
    врати Lo;
  врати Hi.shl(Hi.getBitWidth() - ShiftAmt) | Lo.lshr(ShiftAmt);
}

APInt llvm::APIntOps::clmul(const APInt &LHS, const APInt &RHS) {
  unsigned BW = LHS.getBitWidth();
  assert(BW == RHS.getBitWidth() && "Operand mismatch");
  APInt Result(BW, 0);
  за (unsigned I : seq(std::min(RHS.getActiveBits(), BW - LHS.countr_zero())))
    ако (RHS[I])
      Result ^= LHS << I;
  врати Result;
}

APInt llvm::APIntOps::clmulr(const APInt &LHS, const APInt &RHS) {
  assert(LHS.getBitWidth() == RHS.getBitWidth());
  врати clmul(LHS.reverseBits(), RHS.reverseBits()).reverseBits();
}

APInt llvm::APIntOps::clmulh(const APInt &LHS, const APInt &RHS) {
  assert(LHS.getBitWidth() == RHS.getBitWidth());
  врати clmulr(LHS, RHS).lshr(1);
}

APInt llvm::APIntOps::pext(const APInt &Val, const APInt &Mask) {
  unsigned BW = Val.getBitWidth();
  assert(BW == Mask.getBitWidth() && "Operand mismatch");
  APInt Result = APInt::getZero(BW);
  за (unsigned I = 0, P = 0; I != BW; ++I)
    ако (Mask[I])
      Result.setBitVal(P++, Val[I]);
  врати Result;
}

APInt llvm::APIntOps::pdep(const APInt &Val, const APInt &Mask) {
  unsigned BW = Val.getBitWidth();
  assert(BW == Mask.getBitWidth() && "Operand mismatch");
  APInt Result = APInt::getZero(BW);
  за (unsigned I = 0, P = 0; I != BW; ++I)
    ако (Mask[I])
      Result.setBitVal(I, Val[P++]);
  врати Result;
}
