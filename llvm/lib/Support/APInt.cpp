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

#അന്തര്ഭാവ "llvm/ADT/APInt.h"
#അന്തര്ഭാവ "llvm/ADT/ArrayRef.h"
#അന്തര്ഭാവ "llvm/ADT/FoldingSet.h"
#അന്തര്ഭാവ "llvm/ADT/Hashing.h"
#അന്തര്ഭാവ "llvm/ADT/Sequence.h"
#അന്തര്ഭാവ "llvm/ADT/SmallString.h"
#അന്തര്ഭാവ "llvm/ADT/StringRef.h"
#അന്തര്ഭാവ "llvm/ADT/bit.h"
#അന്തര്ഭാവ "llvm/Support/Alignment.h"
#അന്തര്ഭാവ "llvm/Support/Debug.h"
#അന്തര്ഭാവ "llvm/Support/ErrorHandling.h"
#അന്തര്ഭാവ "llvm/Support/MathExtras.h"
#അന്തര്ഭാവ "llvm/Support/SwapByteOrder.h"
#അന്തര്ഭാവ "llvm/Support/raw_ostream.h"
#അന്തര്ഭാവ <cmath>
#അന്തര്ഭാവ <optional>

using നാമാകാശ llvm;

#define DEBUG_TYPE "apint"

/// A utility function for allocating memory, checking for allocation failures,
/// and ensuring the contents are zeroed.
വരിയിൽ സ്ഥിതം uint64_t* getClearedMemory(unsigned അക്കം_വാക്കുകൾ) {
  തിരികെ പുതിയ uint64_t[അക്കം_വാക്കുകൾ]();
}

/// A utility function for allocating memory and checking for allocation
/// failure.  The content is not zeroed.
വരിയിൽ സ്ഥിതം uint64_t* getMemory(unsigned അക്കം_വാക്കുകൾ) {
  തിരികെ പുതിയ uint64_t[അക്കം_വാക്കുകൾ];
}

/// A utility function that converts a character to a digit.
വരിയിൽ സ്ഥിതം unsigned getDigit(അക്ഷരം cdigit, uint8_t radix) {
  unsigned r;

  എങ്കിൽ (radix == 16 || radix == 36) {
    r = cdigit - '0';
    എങ്കിൽ (r <= 9)
      തിരികെ r;

    r = cdigit - 'A';
    എങ്കിൽ (r <= radix - 11U)
      തിരികെ r + 10;

    r = cdigit - 'a';
    എങ്കിൽ (r <= radix - 11U)
      തിരികെ r + 10;

    radix = 10;
  }

  r = cdigit - '0';
  എങ്കിൽ (r < radix)
    തിരികെ r;

  തിരികെ UINT_MAX;
}


ശൂന്യം APInt::initSlowCase(uint64_t val, തർക്കം isSigned) {
  എങ്കിൽ (isSigned && int64_t(val) < 0) {
    U.pVal = getMemory(get_അക്കം_വാക്കുകൾ());
    U.pVal[0] = val;
    memset(&U.pVal[1], 0xFF, APINT_WORD_SIZE * (get_അക്കം_വാക്കുകൾ() - 1));
    clearUnusedBits();
  } അല്ലെങ്കിൽ {
    U.pVal = getClearedMemory(get_അക്കം_വാക്കുകൾ());
    U.pVal[0] = val;
  }
}

ശൂന്യം APInt::initSlowCase(സ്ഥിരം APInt& that) {
  U.pVal = getMemory(get_അക്കം_വാക്കുകൾ());
  memcpy(U.pVal, that.U.pVal, get_അക്കം_വാക്കുകൾ() * APINT_WORD_SIZE);
}

ശൂന്യം APInt::initFromArray(ArrayRef<uint64_t> bigVal) {
  പ്രതിജ്ഞാ(bigVal.data() && "Null pointer detected!");
  എങ്കിൽ (LLVM_LIKELY(isSingleWord()))
    U.VAL = bigVal[0];
  അല്ലെങ്കിൽ {
    // Get memory, cleared to 0
    U.pVal = getClearedMemory(get_അക്കം_വാക്കുകൾ());
    // Calculate the number of words to copy
    unsigned വാക്കുകൾ = std::min<unsigned>(bigVal.size(), get_അക്കം_വാക്കുകൾ());
    // Copy the words from bigVal to pVal
    memcpy(U.pVal, bigVal.data(), വാക്കുകൾ * APINT_WORD_SIZE);
  }
  // Make sure unused high bits are cleared
  clearUnusedBits();
}

APInt::APInt(unsigned അക്കം_Bits, ArrayRef<uint64_t> bigVal) : BitWidth(അക്കം_Bits) {
  initFromArray(bigVal);
}

APInt::APInt(unsigned numbits, StringRef Str, uint8_t radix)
    : BitWidth(numbits) {
  fromString(numbits, Str, radix);
}

ശൂന്യം APInt::reallocate(unsigned NewBitWidth) {
  // If the number of words is the same we can just change the width and stop.
  എങ്കിൽ (get_അക്കം_വാക്കുകൾ() == get_അക്കം_വാക്കുകൾ(NewBitWidth)) {
    BitWidth = NewBitWidth;
    തിരികെ;
  }

  // If we have an allocation, delete it.
  എങ്കിൽ (LLVM_UNLIKELY(!isSingleWord()))
    delete [] U.pVal;

  // Update BitWidth.
  BitWidth = NewBitWidth;

  // If we are supposed to have an allocation, create it.
  എങ്കിൽ (LLVM_UNLIKELY(!isSingleWord()))
    U.pVal = getMemory(get_അക്കം_വാക്കുകൾ());
}

ശൂന്യം APInt::assignSlowCase(സ്ഥിരം APInt &RHS) {
  // Don't do anything for X = X
  എങ്കിൽ (ഇത് == &RHS)
    തിരികെ;

  // Adjust the bit width and handle allocations as necessary.
  reallocate(RHS.getBitWidth());

  // Copy the data.
  എങ്കിൽ (LLVM_LIKELY(isSingleWord()))
    U.VAL = RHS.U.VAL;
  അല്ലെങ്കിൽ
    memcpy(U.pVal, RHS.U.pVal, get_അക്കം_വാക്കുകൾ() * APINT_WORD_SIZE);
}

/// This method 'profiles' an APInt for use with FoldingSet.
ശൂന്യം APInt::Profile(Folding_Set_കണ്ണി_ID& ID) സ്ഥിരം {
  ID.AddInteger(BitWidth);

  എങ്കിൽ (LLVM_LIKELY(isSingleWord())) {
    ID.AddInteger(U.VAL);
    തിരികെ;
  }

  unsigned അക്കം_വാക്കുകൾ_2 = get_അക്കം_വാക്കുകൾ();
  വേണ്ടി (unsigned i = 0; i < അക്കം_വാക്കുകൾ_2; ++i)
    ID.AddInteger(U.pVal[i]);
}

തർക്കം APInt::isAligned(Align A) സ്ഥിരം {
  എങ്കിൽ (isZero())
    തിരികെ സത്യം;
  സ്ഥിരം unsigned TrailingZeroes = countr_zero();
  സ്ഥിരം unsigned MinimumTrailingZeroes = Log2(A);
  തിരികെ TrailingZeroes >= MinimumTrailingZeroes;
}

/// Prefix increment operator. Increments the APInt by one.
APInt& APInt::operator++() {
  എങ്കിൽ (LLVM_LIKELY(isSingleWord()))
    ++U.VAL;
  അല്ലെങ്കിൽ
    tcIncrement(U.pVal, get_അക്കം_വാക്കുകൾ());
  തിരികെ clearUnusedBits();
}

/// Prefix decrement operator. Decrements the APInt by one.
APInt& APInt::operator--() {
  എങ്കിൽ (LLVM_LIKELY(isSingleWord()))
    --U.VAL;
  അല്ലെങ്കിൽ
    tcDecrement(U.pVal, get_അക്കം_വാക്കുകൾ());
  തിരികെ clearUnusedBits();
}

/// Adds the RHS APInt to this APInt.
/// @returns this, after addition of RHS.
/// Addition assignment operator.
APInt& APInt::operator+=(സ്ഥിരം APInt& RHS) {
  പ്രതിജ്ഞാ(BitWidth == RHS.BitWidth && "Bit widths must be the same");
  എങ്കിൽ (LLVM_LIKELY(isSingleWord()))
    U.VAL += RHS.U.VAL;
  അല്ലെങ്കിൽ
    tcAdd(U.pVal, RHS.U.pVal, 0, get_അക്കം_വാക്കുകൾ());
  തിരികെ clearUnusedBits();
}

APInt& APInt::operator+=(uint64_t RHS) {
  എങ്കിൽ (LLVM_LIKELY(isSingleWord()))
    U.VAL += RHS;
  അല്ലെങ്കിൽ
    tcAddPart(U.pVal, RHS, get_അക്കം_വാക്കുകൾ());
  തിരികെ clearUnusedBits();
}

/// Subtracts the RHS APInt from this APInt
/// @returns this, after subtraction
/// Subtraction assignment operator.
APInt& APInt::operator-=(സ്ഥിരം APInt& RHS) {
  പ്രതിജ്ഞാ(BitWidth == RHS.BitWidth && "Bit widths must be the same");
  എങ്കിൽ (LLVM_LIKELY(isSingleWord()))
    U.VAL -= RHS.U.VAL;
  അല്ലെങ്കിൽ
    tcSubtract(U.pVal, RHS.U.pVal, 0, get_അക്കം_വാക്കുകൾ());
  തിരികെ clearUnusedBits();
}

APInt& APInt::operator-=(uint64_t RHS) {
  എങ്കിൽ (LLVM_LIKELY(isSingleWord()))
    U.VAL -= RHS;
  അല്ലെങ്കിൽ
    tcSubtractPart(U.pVal, RHS, get_അക്കം_വാക്കുകൾ());
  തിരികെ clearUnusedBits();
}

APInt APInt::operator*(സ്ഥിരം APInt& RHS) സ്ഥിരം {
  പ്രതിജ്ഞാ(BitWidth == RHS.BitWidth && "Bit widths must be the same");
  എങ്കിൽ (LLVM_LIKELY(isSingleWord()))
    തിരികെ APInt(BitWidth, U.VAL * RHS.U.VAL, /*isSigned=*/അസത്യം,
                 /*implicitTrunc=*/സത്യം);

  APInt Result(getMemory(get_അക്കം_വാക്കുകൾ()), getBitWidth());
  tcMultiply(Result.U.pVal, U.pVal, RHS.U.pVal, get_അക്കം_വാക്കുകൾ());
  Result.clearUnusedBits();
  തിരികെ Result;
}

ശൂന്യം APInt::andAssignSlowCase(സ്ഥിരം APInt &RHS) {
  WordType *dst = U.pVal, *rhs = RHS.U.pVal;
  വേണ്ടി (size_t i = 0, e = get_അക്കം_വാക്കുകൾ(); i != e; ++i)
    dst[i] &= rhs[i];
}

ശൂന്യം APInt::orAssignSlowCase(സ്ഥിരം APInt &RHS) {
  WordType *dst = U.pVal, *rhs = RHS.U.pVal;
  വേണ്ടി (size_t i = 0, e = get_അക്കം_വാക്കുകൾ(); i != e; ++i)
    dst[i] |= rhs[i];
}

ശൂന്യം APInt::xorAssignSlowCase(സ്ഥിരം APInt &RHS) {
  WordType *dst = U.pVal, *rhs = RHS.U.pVal;
  വേണ്ടി (size_t i = 0, e = get_അക്കം_വാക്കുകൾ(); i != e; ++i)
    dst[i] ^= rhs[i];
}

APInt &APInt::operator*=(സ്ഥിരം APInt &RHS) {
  *ഇത് = *ഇത് * RHS;
  തിരികെ *ഇത്;
}

APInt& APInt::operator*=(uint64_t RHS) {
  എങ്കിൽ (LLVM_LIKELY(isSingleWord())) {
    U.VAL *= RHS;
  } അല്ലെങ്കിൽ {
    unsigned അക്കം_വാക്കുകൾ_2 = get_അക്കം_വാക്കുകൾ();
    tcMultiplyPart(U.pVal, U.pVal, RHS, 0, അക്കം_വാക്കുകൾ_2, അക്കം_വാക്കുകൾ_2, അസത്യം);
  }
  തിരികെ clearUnusedBits();
}

തർക്കം APInt::equalSlowCase(സ്ഥിരം APInt &RHS) സ്ഥിരം {
  തിരികെ std::equal(U.pVal, U.pVal + get_അക്കം_വാക്കുകൾ(), RHS.U.pVal);
}

പൂർണ്ണസംഖ്യ APInt::compare(സ്ഥിരം APInt& RHS) സ്ഥിരം {
  പ്രതിജ്ഞാ(BitWidth == RHS.BitWidth && "Bit widths must be same for comparison");
  എങ്കിൽ (LLVM_LIKELY(isSingleWord()))
    തിരികെ U.VAL < RHS.U.VAL ? -1 : U.VAL > RHS.U.VAL;

  തിരികെ tcCompare(U.pVal, RHS.U.pVal, get_അക്കം_വാക്കുകൾ());
}

പൂർണ്ണസംഖ്യ APInt::compareSigned(സ്ഥിരം APInt& RHS) സ്ഥിരം {
  പ്രതിജ്ഞാ(BitWidth == RHS.BitWidth && "Bit widths must be same for comparison");
  എങ്കിൽ (LLVM_LIKELY(isSingleWord())) {
    int64_t lhsSext = SignExtend64(U.VAL, BitWidth);
    int64_t rhsSext = SignExtend64(RHS.U.VAL, BitWidth);
    തിരികെ lhsSext < rhsSext ? -1 : lhsSext > rhsSext;
  }

  തർക്കം lhsNeg = isNegative();
  തർക്കം rhsNeg = RHS.isNegative();

  // If the sign bits don't match, then (LHS < RHS) if LHS is negative
  എങ്കിൽ (lhsNeg != rhsNeg)
    തിരികെ lhsNeg ? -1 : 1;

  // Otherwise we can just use an unsigned comparison, because even negative
  // numbers compare correctly this way if both have the same signed-ness.
  തിരികെ tcCompare(U.pVal, RHS.U.pVal, get_അക്കം_വാക്കുകൾ());
}

ശൂന്യം APInt::setBitsSlowCase(unsigned loBit, unsigned hiBit) {
  unsigned loWord = whichWord(loBit);
  unsigned hiWord = whichWord(hiBit);

  // Create an initial mask for the low word with zeros below loBit.
  uint64_t loMask = WORDTYPE_MAX << whichBit(loBit);

  // If hiBit is not aligned, we need a high mask.
  unsigned hiShiftAmt = whichBit(hiBit);
  എങ്കിൽ (hiShiftAmt != 0) {
    // Create a high mask with zeros above hiBit.
    uint64_t hiMask = WORDTYPE_MAX >> (APINT_BITS_PER_WORD - hiShiftAmt);
    // If loWord and hiWord are equal, then we combine the masks. Otherwise,
    // set the bits in hiWord.
    എങ്കിൽ (hiWord == loWord)
      loMask &= hiMask;
    അല്ലെങ്കിൽ
      U.pVal[hiWord] |= hiMask;
  }
  // Apply the mask to the low word.
  U.pVal[loWord] |= loMask;

  // Fill any words between loWord and hiWord with all ones.
  വേണ്ടി (unsigned word = loWord + 1; word < hiWord; ++word)
    U.pVal[word] = WORDTYPE_MAX;
}

ശൂന്യം APInt::clearBitsSlowCase(unsigned LoBit, unsigned HiBit) {
  unsigned LoWord = whichWord(LoBit);
  unsigned HiWord = whichWord(HiBit);

  // Create an initial mask for the low word with ones below loBit.
  uint64_t LoMask = ~(WORDTYPE_MAX << whichBit(LoBit));

  // If HiBit is not aligned, we need a high mask.
  unsigned HiShiftAmt = whichBit(HiBit);
  എങ്കിൽ (HiShiftAmt != 0) {
    // Create a high mask with ones above HiBit.
    uint64_t HiMask = ~(WORDTYPE_MAX >> (APINT_BITS_PER_WORD - HiShiftAmt));
    // If LoWord and HiWord are equal, then we combine the masks. Otherwise,
    // clear the bits in HiWord.
    എങ്കിൽ (HiWord == LoWord)
      LoMask |= HiMask;
    അല്ലെങ്കിൽ
      U.pVal[HiWord] &= HiMask;
  }
  // Apply the mask to the low word.
  U.pVal[LoWord] &= LoMask;

  // Fill any words between LoWord and HiWord with all zeros.
  വേണ്ടി (unsigned Word = LoWord + 1; Word < HiWord; ++Word)
    U.pVal[Word] = 0;
}

// Complement a bignum in-place.
സ്ഥിതം ശൂന്യം tcComplement(APInt::WordType *dst, unsigned parts) {
  വേണ്ടി (unsigned i = 0; i < parts; i++)
    dst[i] = ~dst[i];
}

/// Toggle every bit to its opposite value.
ശൂന്യം APInt::flipAllBitsSlowCase() {
  tcComplement(U.pVal, get_അക്കം_വാക്കുകൾ());
  clearUnusedBits();
}

/// Concatenate the bits from "NewLSB" onto the bottom of *this.  This is
/// equivalent to:
///   (this->zext(NewWidth) << NewLSB.getBitWidth()) | NewLSB.zext(NewWidth)
/// In the slow case, we know the result is large.
APInt APInt::concatSlowCase(സ്ഥിരം APInt &NewLSB) സ്ഥിരം {
  unsigned NewWidth = getBitWidth() + NewLSB.getBitWidth();
  APInt Result = NewLSB.zext(NewWidth);
  Result.തിരുകുക_Bits(*ഇത്, NewLSB.getBitWidth());
  തിരികെ Result;
}

/// Toggle a given bit to its opposite value whose position is given
/// as "bitPosition".
/// Toggles a given bit to its opposite value.
ശൂന്യം APInt::flipBit(unsigned bitPosition) {
  പ്രതിജ്ഞാ(bitPosition < BitWidth && "Out of the bit-width range!");
  setBitVal(bitPosition, !(*ഇത്)[bitPosition]);
}

ശൂന്യം APInt::തിരുകുക_Bits(സ്ഥിരം APInt &subBits, unsigned bitPosition) {
  unsigned subBitWidth = subBits.getBitWidth();
  പ്രതിജ്ഞാ((subBitWidth + bitPosition) <= BitWidth && "Illegal bit insertion");

  // inserting no bits is a noop.
  എങ്കിൽ (subBitWidth == 0)
    തിരികെ;

  // Insertion is a direct copy.
  എങ്കിൽ (subBitWidth == BitWidth) {
    *ഇത് = subBits;
    തിരികെ;
  }

  // Single word result can be done as a direct bitmask.
  എങ്കിൽ (LLVM_LIKELY(isSingleWord())) {
    uint64_t mask = WORDTYPE_MAX >> (APINT_BITS_PER_WORD - subBitWidth);
    U.VAL &= ~(mask << bitPosition);
    U.VAL |= (subBits.U.VAL << bitPosition);
    തിരികെ;
  }

  unsigned loBit = whichBit(bitPosition);
  unsigned loWord = whichWord(bitPosition);
  unsigned hi1Word = whichWord(bitPosition + subBitWidth - 1);

  // Insertion within a single word can be done as a direct bitmask.
  എങ്കിൽ (loWord == hi1Word) {
    uint64_t mask = WORDTYPE_MAX >> (APINT_BITS_PER_WORD - subBitWidth);
    U.pVal[loWord] &= ~(mask << loBit);
    U.pVal[loWord] |= (subBits.U.VAL << loBit);
    തിരികെ;
  }

  // Insert on word boundaries.
  എങ്കിൽ (loBit == 0) {
    // Direct copy whole words.
    unsigned അക്കം_Whole_Sub_വാക്കുകൾ = subBitWidth / APINT_BITS_PER_WORD;
    memcpy(U.pVal + loWord, subBits.getRawData(),
           അക്കം_Whole_Sub_വാക്കുകൾ * APINT_WORD_SIZE);

    // Mask+insert remaining bits.
    unsigned remainingBits = subBitWidth % APINT_BITS_PER_WORD;
    എങ്കിൽ (remainingBits != 0) {
      uint64_t mask = WORDTYPE_MAX >> (APINT_BITS_PER_WORD - remainingBits);
      U.pVal[hi1Word] &= ~mask;
      U.pVal[hi1Word] |= subBits.getWord(subBitWidth - 1);
    }
    തിരികെ;
  }

  // General case - set/clear individual bits in dst based on src.
  // TODO - there is scope for optimization here, but at the moment this code
  // path is barely used so prefer readability over performance.
  വേണ്ടി (unsigned i = 0; i != subBitWidth; ++i)
    setBitVal(bitPosition + i, subBits[i]);
}

ശൂന്യം APInt::തിരുകുക_Bits(uint64_t subBits, unsigned bitPosition, unsigned അക്കം_Bits) {
  uint64_t maskBits = maskTrailingOnes<uint64_t>(അക്കം_Bits);
  subBits &= maskBits;
  എങ്കിൽ (LLVM_LIKELY(isSingleWord())) {
    U.VAL &= ~(maskBits << bitPosition);
    U.VAL |= subBits << bitPosition;
    തിരികെ;
  }

  unsigned loBit = whichBit(bitPosition);
  unsigned loWord = whichWord(bitPosition);
  unsigned hiWord = whichWord(bitPosition + അക്കം_Bits - 1);
  എങ്കിൽ (loWord == hiWord) {
    U.pVal[loWord] &= ~(maskBits << loBit);
    U.pVal[loWord] |= subBits << loBit;
    തിരികെ;
  }

  static_assert(8 * വലുപ്പം(WordType) <= 64, "This code assumes only two words affected");
  unsigned wordBits = 8 * വലുപ്പം(WordType);
  U.pVal[loWord] &= ~(maskBits << loBit);
  U.pVal[loWord] |= subBits << loBit;

  U.pVal[hiWord] &= ~(maskBits >> (wordBits - loBit));
  U.pVal[hiWord] |= subBits >> (wordBits - loBit);
}

APInt APInt::extractBits(unsigned അക്കം_Bits, unsigned bitPosition) സ്ഥിരം {
  പ്രതിജ്ഞാ(bitPosition < BitWidth && (അക്കം_Bits + bitPosition) <= BitWidth &&
         "Illegal bit extraction");

  എങ്കിൽ (LLVM_LIKELY(isSingleWord()))
    തിരികെ APInt(അക്കം_Bits, U.VAL >> bitPosition, /*isSigned=*/അസത്യം,
                 /*implicitTrunc=*/സത്യം);

  unsigned loBit = whichBit(bitPosition);
  unsigned loWord = whichWord(bitPosition);
  unsigned hiWord = whichWord(bitPosition + അക്കം_Bits - 1);

  // Single word result extracting bits from a single word source.
  എങ്കിൽ (loWord == hiWord)
    തിരികെ APInt(അക്കം_Bits, U.pVal[loWord] >> loBit, /*isSigned=*/അസത്യം,
                 /*implicitTrunc=*/സത്യം);

  // Extracting bits that start on a source word boundary can be done
  // as a fast memory copy.
  എങ്കിൽ (loBit == 0)
    തിരികെ APInt(അക്കം_Bits, ArrayRef(U.pVal + loWord, 1 + hiWord - loWord));

  // General case - shift + copy source words directly into place.
  APInt Result(അക്കം_Bits, 0);
  unsigned അക്കം_Src_വാക്കുകൾ = get_അക്കം_വാക്കുകൾ();
  unsigned അക്കം_Dst_വാക്കുകൾ = Result.get_അക്കം_വാക്കുകൾ();

  uint64_t *DestPtr =
      LLVM_LIKELY(Result.isSingleWord()) ? &Result.U.VAL : Result.U.pVal;
  വേണ്ടി (unsigned word = 0; word < അക്കം_Dst_വാക്കുകൾ; ++word) {
    uint64_t w0 = U.pVal[loWord + word];
    uint64_t w1 =
        (loWord + word + 1) < അക്കം_Src_വാക്കുകൾ ? U.pVal[loWord + word + 1] : 0;
    DestPtr[word] = (w0 >> loBit) | (w1 << (APINT_BITS_PER_WORD - loBit));
  }

  തിരികെ Result.clearUnusedBits();
}

uint64_t APInt::extract_Bits_As_ZExt_മൂല്യം(unsigned അക്കം_Bits,
                                       unsigned bitPosition) സ്ഥിരം {
  പ്രതിജ്ഞാ(bitPosition < BitWidth && (അക്കം_Bits + bitPosition) <= BitWidth &&
         "Illegal bit extraction");
  പ്രതിജ്ഞാ(അക്കം_Bits <= 64 && "Illegal bit extraction");

  uint64_t maskBits = maskTrailingOnes<uint64_t>(അക്കം_Bits);
  എങ്കിൽ (LLVM_LIKELY(isSingleWord()))
    തിരികെ (U.VAL >> bitPosition) & maskBits;

  static_assert(APINT_BITS_PER_WORD >= 64,
                "This code assumes only two words affected");
  unsigned loBit = whichBit(bitPosition);
  unsigned loWord = whichWord(bitPosition);
  unsigned hiWord = whichWord(bitPosition + അക്കം_Bits - 1);
  എങ്കിൽ (loWord == hiWord)
    തിരികെ (U.pVal[loWord] >> loBit) & maskBits;

  uint64_t retBits = U.pVal[loWord] >> loBit;
  retBits |= U.pVal[hiWord] << (APINT_BITS_PER_WORD - loBit);
  retBits &= maskBits;
  തിരികെ retBits;
}

unsigned APInt::getSufficientBitsNeeded(StringRef Str, uint8_t Radix) {
  പ്രതിജ്ഞാ(!Str.empty() && "Invalid string length");
  size_t StrLen = Str.size();

  // Each computation below needs to know if it's negative.
  unsigned IsNegative = അസത്യം;
  എങ്കിൽ (Str[0] == '-' || Str[0] == '+') {
    IsNegative = Str[0] == '-';
    StrLen--;
    പ്രതിജ്ഞാ(StrLen && "String is only a sign, needs a value.");
  }

  // For radixes of power-of-two values, the bits required is accurately and
  // easily computed.
  എങ്കിൽ (Radix == 2)
    തിരികെ StrLen + IsNegative;
  എങ്കിൽ (Radix == 8)
    തിരികെ StrLen * 3 + IsNegative;
  എങ്കിൽ (Radix == 16)
    തിരികെ StrLen * 4 + IsNegative;

  // Compute a sufficient number of bits that is always large enough but might
  // be too large. This avoids the assertion in the constructor. This
  // calculation doesn't work appropriately for the numbers 0-9, so just use 4
  // bits in that case.
  എങ്കിൽ (Radix == 10)
    തിരികെ (StrLen == 1 ? 4 : StrLen * 64 / 18) + IsNegative;

  പ്രതിജ്ഞാ(Radix == 36);
  തിരികെ (StrLen == 1 ? 7 : StrLen * 16 / 3) + IsNegative;
}

unsigned APInt::getBitsNeeded(StringRef str, uint8_t radix) {
  // Compute a sufficient number of bits that is always large enough but might
  // be too large.
  unsigned sufficient = getSufficientBitsNeeded(str, radix);

  // For bases 2, 8, and 16, the sufficient number of bits is exact and we can
  // return the value directly. For bases 10 and 36, we need to do extra work.
  എങ്കിൽ (radix == 2 || radix == 8 || radix == 16)
    തിരികെ sufficient;

  // This is grossly inefficient but accurate. We could probably do something
  // with a computation of roughly slen*64/20 and then adjust by the value of
  // the first few digits. But, I'm not sure how accurate that could be.
  size_t slen = str.size();

  // Each computation below needs to know if it's negative.
  StringRef::iterator p = str.begin();
  unsigned isNegative = *p == '-';
  എങ്കിൽ (*p == '-' || *p == '+') {
    p++;
    slen--;
    പ്രതിജ്ഞാ(slen && "String is only a sign, needs a value.");
  }


  // Convert to the actual binary value.
  APInt tmp(sufficient, StringRef(p, slen), radix);

  // Compute how many bits are required. If the log is infinite, assume we need
  // just bit. If the log is exact and value is negative, then the value is
  // MinSignedValue with (log + 1) bits.
  unsigned log = tmp.logBase2();
  എങ്കിൽ (log == (unsigned)-1) {
    തിരികെ isNegative + 1;
  } അല്ലെങ്കിൽ എങ്കിൽ (isNegative && tmp.isPowerOf2()) {
    തിരികെ isNegative + log;
  } അല്ലെങ്കിൽ {
    തിരികെ isNegative + log + 1;
  }
}

hash_code llvm::hash_മൂല്യം(സ്ഥിരം APInt &Arg) {
  എങ്കിൽ (LLVM_LIKELY(Arg.isSingleWord()))
    തിരികെ hash_combine(Arg.BitWidth, Arg.U.VAL);

  തിരികെ hash_combine(
      Arg.BitWidth,
      hash_combine_range(Arg.U.pVal, Arg.U.pVal + Arg.get_അക്കം_വാക്കുകൾ()));
}

unsigned DenseMapInfo<APInt, ശൂന്യം>::get_Hash_മൂല്യം(സ്ഥിരം APInt &താക്കോൽ) {
  തിരികെ static_cast<unsigned>(hash_മൂല്യം(താക്കോൽ));
}

തർക്കം APInt::isSplat(unsigned SplatSizeInBits) സ്ഥിരം {
  പ്രതിജ്ഞാ(getBitWidth() % SplatSizeInBits == 0 &&
         "SplatSizeInBits must divide width!");
  // We can check that all parts of an integer are equal by making use of a
  // little trick: rotate and check if it's still the same value.
  തിരികെ *ഇത് == rotl(SplatSizeInBits);
}

/// This function returns the high "numBits" bits of this APInt.
APInt APInt::getHiBits(unsigned അക്കം_Bits) സ്ഥിരം {
  തിരികെ ഇത്->lshr(BitWidth - അക്കം_Bits);
}

/// This function returns the low "numBits" bits of this APInt.
APInt APInt::getLoBits(unsigned അക്കം_Bits) സ്ഥിരം {
  APInt Result(get_താഴെയുള്ള_Bits_Set(BitWidth, അക്കം_Bits));
  Result &= *ഇത്;
  തിരികെ Result;
}

/// Return a value containing V broadcasted over NewLen bits.
APInt APInt::getSplat(unsigned NewLen, സ്ഥിരം APInt &V) {
  പ്രതിജ്ഞാ(NewLen >= V.getBitWidth() && "Can't splat to smaller bit width!");

  APInt Val = V.zext(NewLen);
  വേണ്ടി (unsigned I = V.getBitWidth(); I < NewLen; I <<= 1)
    Val |= Val << I;

  തിരികെ Val;
}

unsigned APInt::എണ്ണം_Leading_Zeros_Slow_Case() സ്ഥിരം {
  unsigned എണ്ണം_2 = 0;
  വേണ്ടി (പൂർണ്ണസംഖ്യ i = get_അക്കം_വാക്കുകൾ() - 1; i >= 0; --i) {
    uint64_t V = U.pVal[i];
    എങ്കിൽ (V == 0)
      എണ്ണം_2 += APINT_BITS_PER_WORD;
    അല്ലെങ്കിൽ {
      എണ്ണം_2 += llvm::countl_zero(V);
      നിർത്തുക;
    }
  }
  // Adjust for unused bits in the most significant word (they are zero).
  unsigned Mod = BitWidth % APINT_BITS_PER_WORD;
  എണ്ണം_2 -= Mod > 0 ? APINT_BITS_PER_WORD - Mod : 0;
  തിരികെ എണ്ണം_2;
}

unsigned APInt::എണ്ണം_Leading_Ones_Slow_Case() സ്ഥിരം {
  unsigned മുകളിലെ_Word_Bits = BitWidth % APINT_BITS_PER_WORD;
  unsigned shift;
  എങ്കിൽ (!മുകളിലെ_Word_Bits) {
    മുകളിലെ_Word_Bits = APINT_BITS_PER_WORD;
    shift = 0;
  } അല്ലെങ്കിൽ {
    shift = APINT_BITS_PER_WORD - മുകളിലെ_Word_Bits;
  }
  പൂർണ്ണസംഖ്യ i = get_അക്കം_വാക്കുകൾ() - 1;
  unsigned എണ്ണം_2 = llvm::countl_one(U.pVal[i] << shift);
  എങ്കിൽ (എണ്ണം_2 == മുകളിലെ_Word_Bits) {
    വേണ്ടി (i--; i >= 0; --i) {
      എങ്കിൽ (U.pVal[i] == WORDTYPE_MAX)
        എണ്ണം_2 += APINT_BITS_PER_WORD;
      അല്ലെങ്കിൽ {
        എണ്ണം_2 += llvm::countl_one(U.pVal[i]);
        നിർത്തുക;
      }
    }
  }
  തിരികെ എണ്ണം_2;
}

unsigned APInt::എണ്ണം_Trailing_Zeros_Slow_Case() സ്ഥിരം {
  unsigned എണ്ണം_2 = 0;
  unsigned i = 0;
  വേണ്ടി (; i < get_അക്കം_വാക്കുകൾ() && U.pVal[i] == 0; ++i)
    എണ്ണം_2 += APINT_BITS_PER_WORD;
  എങ്കിൽ (i < get_അക്കം_വാക്കുകൾ())
    എണ്ണം_2 += llvm::countr_zero(U.pVal[i]);
  തിരികെ std::min(എണ്ണം_2, BitWidth);
}

unsigned APInt::എണ്ണം_Trailing_Ones_Slow_Case() സ്ഥിരം {
  unsigned എണ്ണം_2 = 0;
  unsigned i = 0;
  വേണ്ടി (; i < get_അക്കം_വാക്കുകൾ() && U.pVal[i] == WORDTYPE_MAX; ++i)
    എണ്ണം_2 += APINT_BITS_PER_WORD;
  എങ്കിൽ (i < get_അക്കം_വാക്കുകൾ())
    എണ്ണം_2 += llvm::countr_one(U.pVal[i]);
  പ്രതിജ്ഞാ(എണ്ണം_2 <= BitWidth);
  തിരികെ എണ്ണം_2;
}

unsigned APInt::എണ്ണം_Population_Slow_Case() സ്ഥിരം {
  unsigned എണ്ണം_2 = 0;
  വേണ്ടി (unsigned i = 0; i < get_അക്കം_വാക്കുകൾ(); ++i)
    എണ്ണം_2 += llvm::popcount(U.pVal[i]);
  തിരികെ എണ്ണം_2;
}

തർക്കം APInt::isPowerOf2SlowCase() സ്ഥിരം {
  unsigned എണ്ണം_2 = 0;
  വേണ്ടി (unsigned i = 0; i < get_അക്കം_വാക്കുകൾ(); ++i) {
    എണ്ണം_2 += llvm::popcount(U.pVal[i]);
    എങ്കിൽ (എണ്ണം_2 > 1)
      തിരികെ അസത്യം;
  }
  തിരികെ എണ്ണം_2 == 1;
}

തർക്കം APInt::intersectsSlowCase(സ്ഥിരം APInt &RHS) സ്ഥിരം {
  വേണ്ടി (unsigned i = 0, e = get_അക്കം_വാക്കുകൾ(); i != e; ++i)
    എങ്കിൽ ((U.pVal[i] & RHS.U.pVal[i]) != 0)
      തിരികെ സത്യം;

  തിരികെ അസത്യം;
}

തർക്കം APInt::isSubsetOfSlowCase(സ്ഥിരം APInt &RHS) സ്ഥിരം {
  വേണ്ടി (unsigned i = 0, e = get_അക്കം_വാക്കുകൾ(); i != e; ++i)
    എങ്കിൽ ((U.pVal[i] & ~RHS.U.pVal[i]) != 0)
      തിരികെ അസത്യം;

  തിരികെ സത്യം;
}

തർക്കം APInt::isInverseOfSlowCase(സ്ഥിരം APInt &RHS) സ്ഥിരം {
  സ്ഥിരം unsigned Last = get_അക്കം_വാക്കുകൾ() - 1;
  വേണ്ടി (unsigned I = 0; I != Last; ++I)
    എങ്കിൽ ((U.pVal[I] ^ RHS.U.pVal[I]) != WORDTYPE_MAX)
      തിരികെ അസത്യം;

  unsigned TailBits = BitWidth - Last * APINT_BITS_PER_WORD;
  WordType TailMask = llvm::maskTrailingOnes<WordType>(TailBits);
  തിരികെ (U.pVal[Last] ^ RHS.U.pVal[Last]) == TailMask;
}

APInt APInt::byteSwap() സ്ഥിരം {
  പ്രതിജ്ഞാ(BitWidth >= 16 && BitWidth % 8 == 0 && "Cannot byteswap!");
  എങ്കിൽ (BitWidth == 16)
    തിരികെ APInt(BitWidth, llvm::byteswap<uint16_t>(U.VAL));
  എങ്കിൽ (BitWidth == 32)
    തിരികെ APInt(BitWidth, llvm::byteswap<uint32_t>(U.VAL));
  എങ്കിൽ (BitWidth <= 64) {
    uint64_t Tmp1 = llvm::byteswap<uint64_t>(U.VAL);
    Tmp1 >>= (64 - BitWidth);
    തിരികെ APInt(BitWidth, Tmp1);
  }

  APInt Result(get_അക്കം_വാക്കുകൾ() * APINT_BITS_PER_WORD, 0);
  വേണ്ടി (unsigned I = 0, N = get_അക്കം_വാക്കുകൾ(); I != N; ++I)
    Result.U.pVal[I] = llvm::byteswap<uint64_t>(U.pVal[N - I - 1]);
  എങ്കിൽ (Result.BitWidth != BitWidth) {
    Result.lshrInPlace(Result.BitWidth - BitWidth);
    Result.BitWidth = BitWidth;
  }
  തിരികെ Result;
}

APInt APInt::reverseBits() സ്ഥിരം {
  എങ്കിൽ (LLVM_LIKELY(isSingleWord())) {
    തിരഞ്ഞെടുപ്പ് (BitWidth) {
    അവസ്ഥ 64:
      തിരികെ APInt(BitWidth, llvm::reverseBits<uint64_t>(U.VAL));
    അവസ്ഥ 32:
      തിരികെ APInt(BitWidth, llvm::reverseBits<uint32_t>(U.VAL));
    അവസ്ഥ 16:
      തിരികെ APInt(BitWidth, llvm::reverseBits<uint16_t>(U.VAL));
    അവസ്ഥ 8:
      തിരികെ APInt(BitWidth, llvm::reverseBits<uint8_t>(U.VAL));
    അവസ്ഥ 1: // fallthrough
    അവസ്ഥ 0:
      തിരികെ *ഇത്;
    സ്ഥിരസ്ഥിതി:
      തിരികെ APInt(BitWidth,
                   llvm::reverseBits<uint64_t>(U.VAL) >> (64 - BitWidth));
    }
  }

  APInt Result(BitWidth, 0);
  unsigned അക്കം_വാക്കുകൾ_2 = get_അക്കം_വാക്കുകൾ();
  unsigned ExcessBits = അക്കം_വാക്കുകൾ_2 * APINT_BITS_PER_WORD - BitWidth;
  എങ്കിൽ (ExcessBits == 0) {
    // Fast path. No cross-word shift needed.
    വേണ്ടി (unsigned I = 0; I < അക്കം_വാക്കുകൾ_2; ++I)
      Result.U.pVal[I] = llvm::reverseBits<uint64_t>(U.pVal[അക്കം_വാക്കുകൾ_2 - 1 - I]);
    തിരികെ Result;
  }
  // Holds reversed bits of the previous (more significant) word.
  uint64_t PrevRev = llvm::reverseBits<uint64_t>(U.pVal[അക്കം_വാക്കുകൾ_2 - 1]);
  വേണ്ടി (unsigned I = 0; I < അക്കം_വാക്കുകൾ_2 - 1; ++I) {
    uint64_t CurrRev = llvm::reverseBits<uint64_t>(U.pVal[അക്കം_വാക്കുകൾ_2 - 2 - I]);
    Result.U.pVal[I] = (PrevRev >> ExcessBits) | (CurrRev << (64 - ExcessBits));
    PrevRev = CurrRev;
  }
  Result.U.pVal[അക്കം_വാക്കുകൾ_2 - 1] = PrevRev >> ExcessBits;
  തിരികെ Result;
}

APInt llvm::APIntOps::Greatest_Common_ഹാരകം(APInt A, APInt B, തർക്കം IsSigned) {
  // Take absolute value if IsSigned.
  എങ്കിൽ (IsSigned) {
    A = A.abs();
    B = B.abs();
  }

  // Fast-path a common case.
  എങ്കിൽ (A == B) തിരികെ A;

  // Corner cases: if either operand is zero, the other is the gcd.
  എങ്കിൽ (!A) തിരികെ B;
  എങ്കിൽ (!B) തിരികെ A;

  // Count common powers of 2 and remove all other powers of 2.
  unsigned Pow2;
  {
    unsigned Pow2_A = A.countr_zero();
    unsigned Pow2_B = B.countr_zero();
    എങ്കിൽ (Pow2_A > Pow2_B) {
      A.lshrInPlace(Pow2_A - Pow2_B);
      Pow2 = Pow2_B;
    } അല്ലെങ്കിൽ എങ്കിൽ (Pow2_B > Pow2_A) {
      B.lshrInPlace(Pow2_B - Pow2_A);
      Pow2 = Pow2_A;
    } അല്ലെങ്കിൽ {
      Pow2 = Pow2_A;
    }
  }

  // Both operands are odd multiples of 2^Pow_2:
  //
  //   gcd(a, b) = gcd(|a - b| / 2^i, min(a, b))
  //
  // This is a modified version of Stein's algorithm, taking advantage of
  // efficient countTrailingZeros().
  ആകുമ്പോൾ (A != B) {
    എങ്കിൽ (A.ugt(B)) {
      A -= B;
      A.lshrInPlace(A.countr_zero() - Pow2);
    } അല്ലെങ്കിൽ {
      B -= A;
      B.lshrInPlace(B.countr_zero() - Pow2);
    }
  }

  തിരികെ A;
}

APInt llvm::APIntOps::RoundDoubleToAPInt(ഇരട്ട Double, unsigned width) {
  uint64_t I = bit_cast<uint64_t>(Double);

  // Get the sign bit from the highest order bit
  തർക്കം isNeg = I >> 63;

  // Get the 11-bit exponent and adjust for the 1023 bit bias
  int64_t exp = ((I >> 52) & 0x7ff) - 1023;

  // If the exponent is negative, the value is < 0 so just return 0.
  എങ്കിൽ (exp < 0)
    തിരികെ APInt(width, 0u);

  // Extract the mantissa by clearing the top 12 bits (sign + exponent).
  uint64_t mantissa = (I & (~0ULL >> 12)) | 1ULL << 52;

  // If the exponent doesn't shift all bits out of the mantissa
  എങ്കിൽ (exp < 52)
    തിരികെ isNeg ? -APInt(width, mantissa >> (52 - exp)) :
                    APInt(width, mantissa >> (52 - exp));

  // If the client didn't provide enough bits for us to shift the mantissa into
  // then the result is undefined, just return 0
  എങ്കിൽ (width <= exp - 52)
    തിരികെ APInt(width, 0);

  // Otherwise, we have to shift the mantissa bits up to the right location
  APInt Tmp(width, mantissa);
  Tmp <<= (unsigned)exp - 52;
  തിരികെ isNeg ? -Tmp : Tmp;
}

/// This function converts this APInt to a double.
/// The layout for double is as following (IEEE Standard 754):
///  --------------------------------------
/// |  Sign    Exponent    Fraction    Bias |
/// |-------------------------------------- |
/// |  1[63]   11[62-52]   52[51-00]   1023 |
///  --------------------------------------
ഇരട്ട APInt::roundToDouble(തർക്കം isSigned) സ്ഥിരം {
  // Handle the simple case where the value is contained in one uint64_t.
  // It is wrong to optimize getWord(0) to VAL; there might be more than one word.
  എങ്കിൽ (LLVM_LIKELY(isSingleWord() || getActiveBits() <= APINT_BITS_PER_WORD)) {
    എങ്കിൽ (isSigned) {
      int64_t sext = SignExtend64(getWord(0), BitWidth);
      തിരികെ ഇരട്ട(sext);
    }
    തിരികെ ഇരട്ട(getWord(0));
  }

  // Determine if the value is negative.
  തർക്കം isNeg = isSigned ? (*ഇത്)[BitWidth-1] : അസത്യം;

  // Construct the absolute value if we're negative.
  APInt Tmp(isNeg ? -(*ഇത്) : (*ഇത്));

  // Figure out how many bits we're using.
  unsigned n = Tmp.getActiveBits();

  // The exponent (without bias normalization) is just the number of bits
  // we are using. Note that the sign bit is gone since we constructed the
  // absolute value.
  uint64_t exp = n;

  // Return infinity for exponent overflow
  എങ്കിൽ (exp > 1023) {
    എങ്കിൽ (!isSigned || !isNeg)
      തിരികെ std::numeric_limits<ഇരട്ട>::infinity();
    അല്ലെങ്കിൽ
      തിരികെ -std::numeric_limits<ഇരട്ട>::infinity();
  }
  exp += 1023; // Increment for 1023 bias

  // Number of bits in mantissa is 52. To obtain the mantissa value, we must
  // extract the high 52 bits from the correct words in pVal.
  uint64_t mantissa;
  unsigned hiWord = whichWord(n-1);
  എങ്കിൽ (hiWord == 0) {
    mantissa = Tmp.U.pVal[0];
    എങ്കിൽ (n > 52)
      mantissa >>= n - 52; // shift down, we want the top 52 bits.
  } അല്ലെങ്കിൽ {
    പ്രതിജ്ഞാ(hiWord > 0 && "huh?");
    uint64_t hibits = Tmp.U.pVal[hiWord] << (52 - n % APINT_BITS_PER_WORD);
    uint64_t lobits = Tmp.U.pVal[hiWord-1] >> (11 + n % APINT_BITS_PER_WORD);
    mantissa = hibits | lobits;
  }

  // The leading bit of mantissa is implicit, so get rid of it.
  uint64_t sign = isNeg ? (1ULL << (APINT_BITS_PER_WORD - 1)) : 0;
  uint64_t I = sign | (exp << 52) | mantissa;
  തിരികെ bit_cast<ഇരട്ട>(I);
}

// Truncate to new width.
APInt APInt::trunc(unsigned width) സ്ഥിരം {
  പ്രതിജ്ഞാ(width <= BitWidth && "Invalid APInt Truncate request");

  എങ്കിൽ (width <= APINT_BITS_PER_WORD)
    തിരികെ APInt(width, getRawData()[0], /*isSigned=*/അസത്യം,
                 /*implicitTrunc=*/സത്യം);

  എങ്കിൽ (width == BitWidth)
    തിരികെ *ഇത്;

  APInt Result(getMemory(get_അക്കം_വാക്കുകൾ(width)), width);

  // Copy full words.
  unsigned i;
  വേണ്ടി (i = 0; i != width / APINT_BITS_PER_WORD; i++)
    Result.U.pVal[i] = U.pVal[i];

  // Truncate and copy any partial word.
  unsigned bits = (0 - width) % APINT_BITS_PER_WORD;
  എങ്കിൽ (bits != 0)
    Result.U.pVal[i] = U.pVal[i] << bits >> bits;

  തിരികെ Result;
}

// Truncate to new width with unsigned saturation.
APInt APInt::truncUSat(unsigned width) സ്ഥിരം {
  പ്രതിജ്ഞാ(width <= BitWidth && "Invalid APInt Truncate request");

  // Can we just losslessly truncate it?
  എങ്കിൽ (isIntN(width))
    തിരികെ trunc(width);
  // If not, then just return the new limit.
  തിരികെ APInt::get_Max_മൂല്യം(width);
}

// Truncate to new width with signed saturation to signed result.
APInt APInt::truncSSat(unsigned width) സ്ഥിരം {
  പ്രതിജ്ഞാ(width <= BitWidth && "Invalid APInt Truncate request");

  // Can we just losslessly truncate it?
  എങ്കിൽ (isSignedIntN(width))
    തിരികെ trunc(width);
  // If not, then just return the new limits.
  തിരികെ isNegative() ? APInt::get_Signed_Min_മൂല്യം(width)
                      : APInt::get_Signed_Max_മൂല്യം(width);
}

// Truncate to new width with signed saturation to unsigned result.
APInt APInt::truncSSatU(unsigned width) സ്ഥിരം {
  പ്രതിജ്ഞാ(width <= BitWidth && "Invalid APInt Truncate request");

  // Can we just losslessly truncate it?
  എങ്കിൽ (isIntN(width))
    തിരികെ trunc(width);
  // If not, then just return the new limits.
  തിരികെ isNegative() ? APInt::getZero(width) : APInt::get_Max_മൂല്യം(width);
}

// Sign extend to a new width.
APInt APInt::sext(unsigned Width) സ്ഥിരം {
  പ്രതിജ്ഞാ(Width >= BitWidth && "Invalid APInt SignExtend request");

  എങ്കിൽ (Width <= APINT_BITS_PER_WORD)
    തിരികെ APInt(Width, SignExtend64(U.VAL, BitWidth), /*isSigned=*/സത്യം);

  എങ്കിൽ (Width == BitWidth)
    തിരികെ *ഇത്;

  APInt Result(getMemory(get_അക്കം_വാക്കുകൾ(Width)), Width);

  // Copy words.
  std::memcpy(Result.U.pVal, getRawData(), get_അക്കം_വാക്കുകൾ() * APINT_WORD_SIZE);

  // Sign extend the last word since there may be unused bits in the input.
  Result.U.pVal[get_അക്കം_വാക്കുകൾ() - 1] =
      SignExtend64(Result.U.pVal[get_അക്കം_വാക്കുകൾ() - 1],
                   ((BitWidth - 1) % APINT_BITS_PER_WORD) + 1);

  // Fill with sign bits.
  std::memset(Result.U.pVal + get_അക്കം_വാക്കുകൾ(), isNegative() ? -1 : 0,
              (Result.get_അക്കം_വാക്കുകൾ() - get_അക്കം_വാക്കുകൾ()) * APINT_WORD_SIZE);
  Result.clearUnusedBits();
  തിരികെ Result;
}

//  Zero extend to a new width.
APInt APInt::zext(unsigned width) സ്ഥിരം {
  പ്രതിജ്ഞാ(width >= BitWidth && "Invalid APInt ZeroExtend request");

  എങ്കിൽ (width <= APINT_BITS_PER_WORD)
    തിരികെ APInt(width, U.VAL);

  എങ്കിൽ (width == BitWidth)
    തിരികെ *ഇത്;

  APInt Result(getMemory(get_അക്കം_വാക്കുകൾ(width)), width);

  // Copy words.
  std::memcpy(Result.U.pVal, getRawData(), get_അക്കം_വാക്കുകൾ() * APINT_WORD_SIZE);

  // Zero remaining words.
  std::memset(Result.U.pVal + get_അക്കം_വാക്കുകൾ(), 0,
              (Result.get_അക്കം_വാക്കുകൾ() - get_അക്കം_വാക്കുകൾ()) * APINT_WORD_SIZE);

  തിരികെ Result;
}

APInt APInt::zextOrTrunc(unsigned width) സ്ഥിരം {
  എങ്കിൽ (BitWidth < width)
    തിരികെ zext(width);
  എങ്കിൽ (BitWidth > width)
    തിരികെ trunc(width);
  തിരികെ *ഇത്;
}

APInt APInt::sextOrTrunc(unsigned width) സ്ഥിരം {
  എങ്കിൽ (BitWidth < width)
    തിരികെ sext(width);
  എങ്കിൽ (BitWidth > width)
    തിരികെ trunc(width);
  തിരികെ *ഇത്;
}

/// Arithmetic right-shift this APInt by shiftAmt.
/// Arithmetic right-shift function.
ശൂന്യം APInt::ashrInPlace(സ്ഥിരം APInt &shiftAmt) {
  ashrInPlace((unsigned)shiftAmt.get_Limited_മൂല്യം(BitWidth));
}

/// Arithmetic right-shift this APInt by shiftAmt.
/// Arithmetic right-shift function.
ശൂന്യം APInt::ashrSlowCase(unsigned ShiftAmt) {
  // Don't bother performing a no-op shift.
  എങ്കിൽ (!ShiftAmt)
    തിരികെ;

  // Save the original sign bit for later.
  തർക്കം Negative = isNegative();

  // WordShift is the inter-part shift; BitShift is intra-part shift.
  unsigned WordShift = ShiftAmt / APINT_BITS_PER_WORD;
  unsigned BitShift = ShiftAmt % APINT_BITS_PER_WORD;

  unsigned വാക്കുകൾ_To_Move = get_അക്കം_വാക്കുകൾ() - WordShift;
  എങ്കിൽ (വാക്കുകൾ_To_Move != 0) {
    // Sign extend the last word to fill in the unused bits.
    U.pVal[get_അക്കം_വാക്കുകൾ() - 1] = SignExtend64(
        U.pVal[get_അക്കം_വാക്കുകൾ() - 1], ((BitWidth - 1) % APINT_BITS_PER_WORD) + 1);

    // Fastpath for moving by whole words.
    എങ്കിൽ (BitShift == 0) {
      std::memmove(U.pVal, U.pVal + WordShift, വാക്കുകൾ_To_Move * APINT_WORD_SIZE);
    } അല്ലെങ്കിൽ {
      // Move the words containing significant bits.
      വേണ്ടി (unsigned i = 0; i != വാക്കുകൾ_To_Move - 1; ++i)
        U.pVal[i] = (U.pVal[i + WordShift] >> BitShift) |
                    (U.pVal[i + WordShift + 1] << (APINT_BITS_PER_WORD - BitShift));

      // Handle the last word which has no high bits to copy. Use an arithmetic
      // shift to preserve the sign bit.
      U.pVal[വാക്കുകൾ_To_Move - 1] =
          (int64_t)U.pVal[WordShift + വാക്കുകൾ_To_Move - 1] >> BitShift;
    }
  }

  // Fill in the remainder based on the original sign.
  std::memset(U.pVal + വാക്കുകൾ_To_Move, Negative ? -1 : 0,
              WordShift * APINT_WORD_SIZE);
  clearUnusedBits();
}

/// Logical right-shift this APInt by shiftAmt.
/// Logical right-shift function.
ശൂന്യം APInt::lshrInPlace(സ്ഥിരം APInt &shiftAmt) {
  lshrInPlace((unsigned)shiftAmt.get_Limited_മൂല്യം(BitWidth));
}

/// Logical right-shift this APInt by shiftAmt.
/// Logical right-shift function.
ശൂന്യം APInt::lshrSlowCase(unsigned ShiftAmt) {
  tc_Shift_വലത്(U.pVal, get_അക്കം_വാക്കുകൾ(), ShiftAmt);
}

/// Left-shift this APInt by shiftAmt.
/// Left-shift function.
APInt &APInt::operator<<=(സ്ഥിരം APInt &shiftAmt) {
  // It's undefined behavior in C to shift by BitWidth or greater.
  *ഇത് <<= (unsigned)shiftAmt.get_Limited_മൂല്യം(BitWidth);
  തിരികെ *ഇത്;
}

ശൂന്യം APInt::shlSlowCase(unsigned ShiftAmt) {
  tc_Shift_ഇടത്(U.pVal, get_അക്കം_വാക്കുകൾ(), ShiftAmt);
  clearUnusedBits();
}

// Calculate the rotate amount modulo the bit width.
സ്ഥിതം unsigned rotateModulo(unsigned BitWidth, സ്ഥിരം APInt &rotateAmt) {
  എങ്കിൽ (LLVM_UNLIKELY(BitWidth == 0))
    തിരികെ 0;
  unsigned rotBitWidth = rotateAmt.getBitWidth();
  APInt rot = rotateAmt;
  എങ്കിൽ (rotBitWidth < BitWidth) {
    // Extend the rotate APInt, so that the urem doesn't divide by 0.
    // e.g. APInt(1, 32) would give APInt(1, 0).
    rot = rotateAmt.zext(BitWidth);
  }
  rot = rot.urem(APInt(rot.getBitWidth(), BitWidth));
  തിരികെ rot.get_Limited_മൂല്യം(BitWidth);
}

APInt APInt::rotl(സ്ഥിരം APInt &rotateAmt) സ്ഥിരം {
  തിരികെ rotl(rotateModulo(BitWidth, rotateAmt));
}

APInt APInt::rotl(unsigned rotateAmt) സ്ഥിരം {
  എങ്കിൽ (LLVM_UNLIKELY(BitWidth == 0))
    തിരികെ *ഇത്;
  rotateAmt %= BitWidth;
  എങ്കിൽ (rotateAmt == 0)
    തിരികെ *ഇത്;
  തിരികെ shl(rotateAmt) | lshr(BitWidth - rotateAmt);
}

APInt APInt::rotr(സ്ഥിരം APInt &rotateAmt) സ്ഥിരം {
  തിരികെ rotr(rotateModulo(BitWidth, rotateAmt));
}

APInt APInt::rotr(unsigned rotateAmt) സ്ഥിരം {
  എങ്കിൽ (BitWidth == 0)
    തിരികെ *ഇത്;
  rotateAmt %= BitWidth;
  എങ്കിൽ (rotateAmt == 0)
    തിരികെ *ഇത്;
  തിരികെ lshr(rotateAmt) | shl(BitWidth - rotateAmt);
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
unsigned APInt::nearestLogBase2() സ്ഥിരം {
  // Special case when we have a bitwidth of 1. If VAL is 1, then we
  // get 0. If VAL is 0, we get WORDTYPE_MAX which gets truncated to
  // UINT32_MAX.
  എങ്കിൽ (BitWidth == 1)
    തിരികെ U.VAL - 1;

  // Handle the zero case.
  എങ്കിൽ (isZero())
    തിരികെ UINT32_MAX;

  // The non-zero case is handled by computing:
  //
  //   nearestLogBase2(x) = logBase2(x) + x[logBase2(x)-1].
  //
  // where x[i] is referring to the value of the ith bit of x.
  unsigned lg = logBase2();
  തിരികെ lg + unsigned((*ഇത്)[lg - 1]);
}

// Square Root - this method computes and returns the square root of "this".
// Three mechanisms are used for computation. For small values (<= 5 bits),
// a table lookup is done. This gets some performance for common cases. For
// values using less than 52 bits, the value is converted to double and then
// the libc sqrt function is called. The result is rounded and then converted
// back to a uint64_t which is then used to construct the result. Finally,
// the Babylonian method for computing square roots is used.
APInt APInt::sqrtFloor() സ്ഥിരം {

  // Determine the magnitude of the value.
  unsigned magnitude = getActiveBits();

  // Use a fast table for some small values. This also gets rid of some
  // rounding errors in libc sqrt for small values.
  എങ്കിൽ (magnitude <= 5) {
    സ്ഥിതം സ്ഥിരം uint8_t results[32] = {
        /*     0 */ 0,
        /*  1- 3 */ 1, 1, 1,
        /*  4- 8 */ 2, 2, 2, 2, 2,
        /*  9-15 */ 3, 3, 3, 3, 3, 3, 3,
        /* 16-24 */ 4, 4, 4, 4, 4, 4, 4, 4, 4,
        /* 25-31 */ 5, 5, 5, 5, 5, 5, 5,
    };
    തിരികെ APInt(BitWidth,
                 results[(LLVM_LIKELY(isSingleWord()) ? U.VAL : U.pVal[0])]);
  }

  // If the magnitude of the value fits in less than 52 bits (the precision of
  // an IEEE double precision floating point value), then we can use the
  // libc sqrt function which will probably use a hardware sqrt computation.
  // This should be faster than the algorithm below.
  എങ്കിൽ (magnitude < 52) {
    തിരികെ APInt(BitWidth,
                 uint64_t(::floor(::sqrt(ഇരട്ട(
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
  വേണ്ടി (;; i += 2, testy = testy.shl(2))
    എങ്കിൽ (i >= nbits || ഇത്->ule(testy)) {
      x_old = x_old.shl(i / 2);
      നിർത്തുക;
    }

  // Use the Babylonian method to arrive at the integer square root:
  വേണ്ടി (;;) {
    x_new = (ഇത്->udiv(x_old) + x_old).udiv(two);
    എങ്കിൽ (x_old.ule(x_new))
      നിർത്തുക;
    x_old = x_new;
  }
  തിരികെ x_old;
}

/// \returns the multiplicative inverse of an odd APInt modulo 2^BitWidth.
APInt APInt::multiplicativeInverse() സ്ഥിരം {
  പ്രതിജ്ഞാ((*ഇത്)[0] &&
         "multiplicative inverse is only defined for odd numbers!");

  // Use Newton's method.
  APInt Factor = *ഇത്;
  APInt T;
  ആകുമ്പോൾ (!(T = *ഇത് * Factor).isOne())
    Factor *= 2 - std::move(T);
  തിരികെ Factor;
}

/// Implementation of Knuth's Algorithm D (Division of nonnegative integers)
/// from "Art of Computer Programming, Volume 2", section 4.3.1, p. 272. The
/// variables here have the same names as in the algorithm. Comments explain
/// the algorithm and any deviation from it.
സ്ഥിതം ശൂന്യം KnuthDiv(uint32_t *u, uint32_t *v, uint32_t *q, uint32_t* r,
                     unsigned m, unsigned n) {
  പ്രതിജ്ഞാ(u && "Must provide dividend");
  പ്രതിജ്ഞാ(v && "Must provide divisor");
  പ്രതിജ്ഞാ(q && "Must provide quotient");
  പ്രതിജ്ഞാ(u != v && u != q && v != q && "Must use different memory");
  പ്രതിജ്ഞാ(n>1 && "n must be > 1");

  // b denotes the base of the number system. In our case b is 2^32.
  സ്ഥിരം uint64_t b = uint64_t(1) << 32;

// The DEBUG macros here tend to be spam in the debug output if you're not
// debugging this code. Disable them unless KNUTH_DEBUG is defined.
#ifdef KNUTH_DEBUG
#define DEBUG_KNUTH(X) LLVM_DEBUG(X)
#അല്ലെങ്കിൽ
#define DEBUG_KNUTH(X) ചെയ്യുക {} ആകുമ്പോൾ(അസത്യം)
#endif

  DEBUG_KNUTH(dbgs() << "KnuthDiv: m=" << m << " n=" << n << '\n');
  DEBUG_KNUTH(dbgs() << "KnuthDiv: original:");
  DEBUG_KNUTH(വേണ്ടി (പൂർണ്ണസംഖ്യ i = m + n; i >= 0; i--) dbgs() << " " << u[i]);
  DEBUG_KNUTH(dbgs() << " by");
  DEBUG_KNUTH(വേണ്ടി (പൂർണ്ണസംഖ്യ i = n; i > 0; i--) dbgs() << " " << v[i - 1]);
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
  എങ്കിൽ (shift) {
    വേണ്ടി (unsigned i = 0; i < m+n; ++i) {
      uint32_t u_tmp = u[i] >> (32 - shift);
      u[i] = (u[i] << shift) | u_carry;
      u_carry = u_tmp;
    }
    വേണ്ടി (unsigned i = 0; i < n; ++i) {
      uint32_t v_tmp = v[i] >> (32 - shift);
      v[i] = (v[i] << shift) | v_carry;
      v_carry = v_tmp;
    }
  }
  u[m+n] = u_carry;

  DEBUG_KNUTH(dbgs() << "KnuthDiv:   normal:");
  DEBUG_KNUTH(വേണ്ടി (പൂർണ്ണസംഖ്യ i = m + n; i >= 0; i--) dbgs() << " " << u[i]);
  DEBUG_KNUTH(dbgs() << " by");
  DEBUG_KNUTH(വേണ്ടി (പൂർണ്ണസംഖ്യ i = n; i > 0; i--) dbgs() << " " << v[i - 1]);
  DEBUG_KNUTH(dbgs() << '\n');

  // D2. [Initialize j.]  Set j to m. This is the loop counter over the places.
  പൂർണ്ണസംഖ്യ j = m;
  ചെയ്യുക {
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
    എങ്കിൽ (qp == b || qp*v[n-2] > b*rp + u[j+n-2]) {
      qp--;
      rp += v[n-1];
      എങ്കിൽ (rp < b && (qp == b || qp*v[n-2] > b*rp + u[j+n-2]))
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
    വേണ്ടി (unsigned i = 0; i < n; ++i) {
      uint64_t p = qp * uint64_t(v[i]);
      int64_t subres = int64_t(u[j+i]) - borrow - Lo_32(p);
      u[j+i] = Lo_32(subres);
      borrow = Hi_32(p) - Hi_32(subres);
      DEBUG_KNUTH(dbgs() << "KnuthDiv: u[j+i] = " << u[j + i]
                        << ", borrow = " << borrow << '\n');
    }
    തർക്കം isNeg = u[j+n] < borrow;
    u[j+n] -= Lo_32(borrow);

    DEBUG_KNUTH(dbgs() << "KnuthDiv: after subtraction:");
    DEBUG_KNUTH(വേണ്ടി (പൂർണ്ണസംഖ്യ i = m + n; i >= 0; i--) dbgs() << " " << u[i]);
    DEBUG_KNUTH(dbgs() << '\n');

    // D5. [Test remainder.] Set q[j] = qp. If the result of step D4 was
    // negative, go to step D6; otherwise go on to step D7.
    q[j] = Lo_32(qp);
    എങ്കിൽ (isNeg) {
      // D6. [Add back]. The probability that this step is necessary is very
      // small, on the order of only 2/b. Make sure that test data accounts for
      // this possibility. Decrease q[j] by 1
      q[j]--;
      // and add (0v[n-1]...v[1]v[0]) to (u[j+n]u[j+n-1]...u[j+1]u[j]).
      // A carry will occur to the left of u[j+n], and it should be ignored
      // since it cancels with the borrow that occurred in D4.
      തർക്കം carry = അസത്യം;
      വേണ്ടി (unsigned i = 0; i < n; i++) {
        uint32_t limit = std::min(u[j+i],v[i]);
        u[j+i] += v[i] + carry;
        carry = u[j+i] < limit || (carry && u[j+i] == limit);
      }
      u[j+n] += carry;
    }
    DEBUG_KNUTH(dbgs() << "KnuthDiv: after correction:");
    DEBUG_KNUTH(വേണ്ടി (പൂർണ്ണസംഖ്യ i = m + n; i >= 0; i--) dbgs() << " " << u[i]);
    DEBUG_KNUTH(dbgs() << "\nKnuthDiv: digit result = " << q[j] << '\n');

    // D7. [Loop on j.]  Decrease j by one. Now if j >= 0, go back to D3.
  } ആകുമ്പോൾ (--j >= 0);

  DEBUG_KNUTH(dbgs() << "KnuthDiv: quotient:");
  DEBUG_KNUTH(വേണ്ടി (പൂർണ്ണസംഖ്യ i = m; i >= 0; i--) dbgs() << " " << q[i]);
  DEBUG_KNUTH(dbgs() << '\n');

  // D8. [Unnormalize]. Now q[...] is the desired quotient, and the desired
  // remainder may be obtained by dividing u[...] by d. If r is non-null we
  // compute the remainder (urem uses this).
  എങ്കിൽ (r) {
    // The value d is expressed by the "shift" value above since we avoided
    // multiplication by d by using a shift left. So, all we have to do is
    // shift right here.
    എങ്കിൽ (shift) {
      uint32_t carry = 0;
      DEBUG_KNUTH(dbgs() << "KnuthDiv: remainder:");
      വേണ്ടി (പൂർണ്ണസംഖ്യ i = n-1; i >= 0; i--) {
        r[i] = (u[i] >> shift) | carry;
        carry = u[i] << (32 - shift);
        DEBUG_KNUTH(dbgs() << " " << r[i]);
      }
    } അല്ലെങ്കിൽ {
      വേണ്ടി (പൂർണ്ണസംഖ്യ i = n-1; i >= 0; i--) {
        r[i] = u[i];
        DEBUG_KNUTH(dbgs() << " " << r[i]);
      }
    }
    DEBUG_KNUTH(dbgs() << '\n');
  }
  DEBUG_KNUTH(dbgs() << '\n');
}

ശൂന്യം APInt::divide(സ്ഥിരം WordType *LHS, unsigned lhs_വാക്കുകൾ, സ്ഥിരം WordType *RHS,
                   unsigned rhs_വാക്കുകൾ, WordType *Quotient, WordType *Remainder) {
  പ്രതിജ്ഞാ(lhs_വാക്കുകൾ >= rhs_വാക്കുകൾ && "Fractional result");

  // First, compose the values into an array of 32-bit words instead of
  // 64-bit words. This is a necessity of both the "short division" algorithm
  // and the Knuth "classical algorithm" which requires there to be native
  // operations for +, -, and * on an m bit value with an m*2 bit result. We
  // can't use 64-bit operands here because we don't have native results of
  // 128-bits. Furthermore, casting the 64-bit values to 32-bit values won't
  // work on large-endian machines.
  unsigned n = rhs_വാക്കുകൾ * 2;
  unsigned m = (lhs_വാക്കുകൾ * 2) - n;

  // Allocate space for the temporary values we need either on the stack, if
  // it will fit, or on the heap if it won't.
  uint32_t SPACE[128];
  uint32_t *U = nullptr;
  uint32_t *V = nullptr;
  uint32_t *Q = nullptr;
  uint32_t *R = nullptr;
  എങ്കിൽ ((Remainder?4:3)*n+2*m+1 <= 128) {
    U = &SPACE[0];
    V = &SPACE[m+n+1];
    Q = &SPACE[(m+n+1) + n];
    എങ്കിൽ (Remainder)
      R = &SPACE[(m+n+1) + n + (m+n)];
  } അല്ലെങ്കിൽ {
    U = പുതിയ uint32_t[m + n + 1];
    V = പുതിയ uint32_t[n];
    Q = പുതിയ uint32_t[m+n];
    എങ്കിൽ (Remainder)
      R = പുതിയ uint32_t[n];
  }

  // Initialize the dividend
  memset(U, 0, (m+n+1)*വലുപ്പം(uint32_t));
  വേണ്ടി (unsigned i = 0; i < lhs_വാക്കുകൾ; ++i) {
    uint64_t tmp = LHS[i];
    U[i * 2] = Lo_32(tmp);
    U[i * 2 + 1] = Hi_32(tmp);
  }
  U[m+n] = 0; // this extra word is for "spill" in the Knuth algorithm.

  // Initialize the divisor
  memset(V, 0, (n)*വലുപ്പം(uint32_t));
  വേണ്ടി (unsigned i = 0; i < rhs_വാക്കുകൾ; ++i) {
    uint64_t tmp = RHS[i];
    V[i * 2] = Lo_32(tmp);
    V[i * 2 + 1] = Hi_32(tmp);
  }

  // initialize the quotient and remainder
  memset(Q, 0, (m+n) * വലുപ്പം(uint32_t));
  എങ്കിൽ (Remainder)
    memset(R, 0, n * വലുപ്പം(uint32_t));

  // Now, adjust m and n for the Knuth division. n is the number of words in
  // the divisor. m is the number of words by which the dividend exceeds the
  // divisor (i.e. m+n is the length of the dividend). These sizes must not
  // contain any zero words or the Knuth algorithm fails.
  വേണ്ടി (unsigned i = n; i > 0 && V[i-1] == 0; i--) {
    n--;
    m++;
  }
  വേണ്ടി (unsigned i = m+n; i > 0 && U[i-1] == 0; i--)
    m--;

  // If we're left with only a single word for the divisor, Knuth doesn't work
  // so we implement the short division algorithm here. This is much simpler
  // and faster because we are certain that we can divide a 64-bit quantity
  // by a 32-bit quantity at hardware speed and short division is simply a
  // series of such operations. This is just like doing short division but we
  // are using base 2^32 instead of base 10.
  പ്രതിജ്ഞാ(n != 0 && "Divide by zero?");
  എങ്കിൽ (n == 1) {
    uint32_t ഹാരകം = V[0];
    uint32_t remainder = 0;
    വേണ്ടി (പൂർണ്ണസംഖ്യ i = m; i >= 0; i--) {
      uint64_t partial_dividend = Make_64(remainder, U[i]);
      എങ്കിൽ (partial_dividend == 0) {
        Q[i] = 0;
        remainder = 0;
      } അല്ലെങ്കിൽ എങ്കിൽ (partial_dividend < ഹാരകം) {
        Q[i] = 0;
        remainder = Lo_32(partial_dividend);
      } അല്ലെങ്കിൽ എങ്കിൽ (partial_dividend == ഹാരകം) {
        Q[i] = 1;
        remainder = 0;
      } അല്ലെങ്കിൽ {
        Q[i] = Lo_32(partial_dividend / ഹാരകം);
        remainder = Lo_32(partial_dividend - (Q[i] * ഹാരകം));
      }
    }
    എങ്കിൽ (R)
      R[0] = remainder;
  } അല്ലെങ്കിൽ {
    // Now we're ready to invoke the Knuth classical divide algorithm. In this
    // case n > 1.
    KnuthDiv(U, V, Q, R, m, n);
  }

  // If the caller wants the quotient
  എങ്കിൽ (Quotient) {
    വേണ്ടി (unsigned i = 0; i < lhs_വാക്കുകൾ; ++i)
      Quotient[i] = Make_64(Q[i*2+1], Q[i*2]);
  }

  // If the caller wants the remainder
  എങ്കിൽ (Remainder) {
    വേണ്ടി (unsigned i = 0; i < rhs_വാക്കുകൾ; ++i)
      Remainder[i] = Make_64(R[i*2+1], R[i*2]);
  }

  // Clean up the memory we allocated.
  എങ്കിൽ (U != &SPACE[0]) {
    delete [] U;
    delete [] V;
    delete [] Q;
    delete [] R;
  }
}

APInt APInt::udiv(സ്ഥിരം APInt &RHS) സ്ഥിരം {
  പ്രതിജ്ഞാ(BitWidth == RHS.BitWidth && "Bit widths must be the same");

  // First, deal with the easy case
  എങ്കിൽ (LLVM_LIKELY(isSingleWord())) {
    പ്രതിജ്ഞാ(RHS.U.VAL != 0 && "Divide by zero?");
    തിരികെ APInt(BitWidth, U.VAL / RHS.U.VAL);
  }

  // Get some facts about the LHS and RHS number of bits and words
  unsigned lhs_വാക്കുകൾ = get_അക്കം_വാക്കുകൾ(getActiveBits());
  unsigned rhsBits  = RHS.getActiveBits();
  unsigned rhs_വാക്കുകൾ = get_അക്കം_വാക്കുകൾ(rhsBits);
  പ്രതിജ്ഞാ(rhs_വാക്കുകൾ && "Divided by zero???");

  // Deal with some degenerate cases
  എങ്കിൽ (!lhs_വാക്കുകൾ)
    // 0 / X ===> 0
    തിരികെ APInt(BitWidth, 0);
  എങ്കിൽ (rhsBits == 1)
    // X / 1 ===> X
    തിരികെ *ഇത്;
  എങ്കിൽ (lhs_വാക്കുകൾ < rhs_വാക്കുകൾ || ഇത്->ult(RHS))
    // X / Y ===> 0, iff X < Y
    തിരികെ APInt(BitWidth, 0);
  എങ്കിൽ (*ഇത് == RHS)
    // X / X ===> 1
    തിരികെ APInt(BitWidth, 1);
  എങ്കിൽ (lhs_വാക്കുകൾ == 1) // rhsWords is 1 if lhsWords is 1.
    // All high words are zero, just use native divide
    തിരികെ APInt(BitWidth, ഇത്->U.pVal[0] / RHS.U.pVal[0]);

  // We have to compute it the hard way. Invoke the Knuth divide algorithm.
  APInt Quotient(BitWidth, 0); // to hold result.
  divide(U.pVal, lhs_വാക്കുകൾ, RHS.U.pVal, rhs_വാക്കുകൾ, Quotient.U.pVal, nullptr);
  തിരികെ Quotient;
}

APInt APInt::udiv(uint64_t RHS) സ്ഥിരം {
  പ്രതിജ്ഞാ(RHS != 0 && "Divide by zero?");

  // First, deal with the easy case
  എങ്കിൽ (LLVM_LIKELY(isSingleWord()))
    തിരികെ APInt(BitWidth, U.VAL / RHS);

  // Get some facts about the LHS words.
  unsigned lhs_വാക്കുകൾ = get_അക്കം_വാക്കുകൾ(getActiveBits());

  // Deal with some degenerate cases
  എങ്കിൽ (!lhs_വാക്കുകൾ)
    // 0 / X ===> 0
    തിരികെ APInt(BitWidth, 0);
  എങ്കിൽ (RHS == 1)
    // X / 1 ===> X
    തിരികെ *ഇത്;
  എങ്കിൽ (ഇത്->ult(RHS))
    // X / Y ===> 0, iff X < Y
    തിരികെ APInt(BitWidth, 0);
  എങ്കിൽ (*ഇത് == RHS)
    // X / X ===> 1
    തിരികെ APInt(BitWidth, 1);
  എങ്കിൽ (lhs_വാക്കുകൾ == 1) // rhsWords is 1 if lhsWords is 1.
    // All high words are zero, just use native divide
    തിരികെ APInt(BitWidth, ഇത്->U.pVal[0] / RHS);

  // We have to compute it the hard way. Invoke the Knuth divide algorithm.
  APInt Quotient(BitWidth, 0); // to hold result.
  divide(U.pVal, lhs_വാക്കുകൾ, &RHS, 1, Quotient.U.pVal, nullptr);
  തിരികെ Quotient;
}

APInt APInt::sdiv(സ്ഥിരം APInt &RHS) സ്ഥിരം {
  എങ്കിൽ (isNegative()) {
    എങ്കിൽ (RHS.isNegative())
      തിരികെ (-(*ഇത്)).udiv(-RHS);
    തിരികെ -((-(*ഇത്)).udiv(RHS));
  }
  എങ്കിൽ (RHS.isNegative())
    തിരികെ -(ഇത്->udiv(-RHS));
  തിരികെ ഇത്->udiv(RHS);
}

APInt APInt::sdiv(int64_t RHS) സ്ഥിരം {
  എങ്കിൽ (isNegative()) {
    എങ്കിൽ (RHS < 0)
      തിരികെ (-(*ഇത്)).udiv(-RHS);
    തിരികെ -((-(*ഇത്)).udiv(RHS));
  }
  എങ്കിൽ (RHS < 0)
    തിരികെ -(ഇത്->udiv(-RHS));
  തിരികെ ഇത്->udiv(RHS);
}

APInt APInt::urem(സ്ഥിരം APInt &RHS) സ്ഥിരം {
  പ്രതിജ്ഞാ(BitWidth == RHS.BitWidth && "Bit widths must be the same");
  എങ്കിൽ (LLVM_LIKELY(isSingleWord())) {
    പ്രതിജ്ഞാ(RHS.U.VAL != 0 && "Remainder by zero?");
    തിരികെ APInt(BitWidth, U.VAL % RHS.U.VAL);
  }

  // Get some facts about the LHS
  unsigned lhs_വാക്കുകൾ = get_അക്കം_വാക്കുകൾ(getActiveBits());

  // Get some facts about the RHS
  unsigned rhsBits = RHS.getActiveBits();
  unsigned rhs_വാക്കുകൾ = get_അക്കം_വാക്കുകൾ(rhsBits);
  പ്രതിജ്ഞാ(rhs_വാക്കുകൾ && "Performing remainder operation by zero ???");

  // Check the degenerate cases
  എങ്കിൽ (lhs_വാക്കുകൾ == 0)
    // 0 % Y ===> 0
    തിരികെ APInt(BitWidth, 0);
  എങ്കിൽ (rhsBits == 1)
    // X % 1 ===> 0
    തിരികെ APInt(BitWidth, 0);
  എങ്കിൽ (lhs_വാക്കുകൾ < rhs_വാക്കുകൾ || ഇത്->ult(RHS))
    // X % Y ===> X, iff X < Y
    തിരികെ *ഇത്;
  എങ്കിൽ (*ഇത് == RHS)
    // X % X == 0;
    തിരികെ APInt(BitWidth, 0);
  എങ്കിൽ (lhs_വാക്കുകൾ == 1)
    // All high words are zero, just use native remainder
    തിരികെ APInt(BitWidth, U.pVal[0] % RHS.U.pVal[0]);
  എങ്കിൽ (RHS.isPowerOf2()) {
    // X % 2^w ===> X & (2^w - 1)
    APInt Result(*ഇത്);
    Result.clearBits(RHS.logBase2(), BitWidth);
    തിരികെ Result;
  }

  // We have to compute it the hard way. Invoke the Knuth divide algorithm.
  APInt Remainder(BitWidth, 0);
  divide(U.pVal, lhs_വാക്കുകൾ, RHS.U.pVal, rhs_വാക്കുകൾ, nullptr, Remainder.U.pVal);
  തിരികെ Remainder;
}

uint64_t APInt::urem(uint64_t RHS) സ്ഥിരം {
  പ്രതിജ്ഞാ(RHS != 0 && "Remainder by zero?");

  എങ്കിൽ (LLVM_LIKELY(isSingleWord()))
    തിരികെ U.VAL % RHS;

  // Get some facts about the LHS
  unsigned lhs_വാക്കുകൾ = get_അക്കം_വാക്കുകൾ(getActiveBits());

  // Check the degenerate cases
  എങ്കിൽ (lhs_വാക്കുകൾ == 0)
    // 0 % Y ===> 0
    തിരികെ 0;
  എങ്കിൽ (RHS == 1)
    // X % 1 ===> 0
    തിരികെ 0;
  എങ്കിൽ (ഇത്->ult(RHS))
    // X % Y ===> X, iff X < Y
    തിരികെ get_ZExt_മൂല്യം();
  എങ്കിൽ (*ഇത് == RHS)
    // X % X == 0;
    തിരികെ 0;
  എങ്കിൽ (lhs_വാക്കുകൾ == 1)
    // All high words are zero, just use native remainder
    തിരികെ U.pVal[0] % RHS;
  എങ്കിൽ (llvm::isPowerOf2_64(RHS))
    // X % 2^w ===> X & (2^w - 1)
    തിരികെ U.pVal[0] & (RHS - 1);

  // We have to compute it the hard way. Invoke the Knuth divide algorithm.
  uint64_t Remainder;
  divide(U.pVal, lhs_വാക്കുകൾ, &RHS, 1, nullptr, &Remainder);
  തിരികെ Remainder;
}

APInt APInt::srem(സ്ഥിരം APInt &RHS) സ്ഥിരം {
  എങ്കിൽ (isNegative()) {
    എങ്കിൽ (RHS.isNegative())
      തിരികെ -((-(*ഇത്)).urem(-RHS));
    തിരികെ -((-(*ഇത്)).urem(RHS));
  }
  എങ്കിൽ (RHS.isNegative())
    തിരികെ ഇത്->urem(-RHS);
  തിരികെ ഇത്->urem(RHS);
}

int64_t APInt::srem(int64_t RHS) സ്ഥിരം {
  എങ്കിൽ (isNegative()) {
    എങ്കിൽ (RHS < 0)
      തിരികെ -((-(*ഇത്)).urem(-RHS));
    തിരികെ -((-(*ഇത്)).urem(RHS));
  }
  എങ്കിൽ (RHS < 0)
    തിരികെ ഇത്->urem(-RHS);
  തിരികെ ഇത്->urem(RHS);
}

ശൂന്യം APInt::udivrem(സ്ഥിരം APInt &LHS, സ്ഥിരം APInt &RHS,
                    APInt &Quotient, APInt &Remainder) {
  പ്രതിജ്ഞാ(LHS.BitWidth == RHS.BitWidth && "Bit widths must be the same");
  unsigned BitWidth = LHS.BitWidth;

  // First, deal with the easy case
  എങ്കിൽ (LLVM_LIKELY(LHS.isSingleWord())) {
    പ്രതിജ്ഞാ(RHS.U.VAL != 0 && "Divide by zero?");
    uint64_t QuotVal = LHS.U.VAL / RHS.U.VAL;
    uint64_t RemVal = LHS.U.VAL % RHS.U.VAL;
    Quotient = APInt(BitWidth, QuotVal);
    Remainder = APInt(BitWidth, RemVal);
    തിരികെ;
  }

  // Get some size facts about the dividend and divisor
  unsigned lhs_വാക്കുകൾ = get_അക്കം_വാക്കുകൾ(LHS.getActiveBits());
  unsigned rhsBits  = RHS.getActiveBits();
  unsigned rhs_വാക്കുകൾ = get_അക്കം_വാക്കുകൾ(rhsBits);
  പ്രതിജ്ഞാ(rhs_വാക്കുകൾ && "Performing divrem operation by zero ???");

  // Check the degenerate cases
  എങ്കിൽ (lhs_വാക്കുകൾ == 0) {
    Quotient = APInt(BitWidth, 0);    // 0 / Y ===> 0
    Remainder = APInt(BitWidth, 0);   // 0 % Y ===> 0
    തിരികെ;
  }

  എങ്കിൽ (rhsBits == 1) {
    Quotient = LHS;                   // X / 1 ===> X
    Remainder = APInt(BitWidth, 0);   // X % 1 ===> 0
  }

  എങ്കിൽ (lhs_വാക്കുകൾ < rhs_വാക്കുകൾ || LHS.ult(RHS)) {
    Remainder = LHS;                  // X % Y ===> X, iff X < Y
    Quotient = APInt(BitWidth, 0);    // X / Y ===> 0, iff X < Y
    തിരികെ;
  }

  എങ്കിൽ (LHS == RHS) {
    Quotient  = APInt(BitWidth, 1);   // X / X ===> 1
    Remainder = APInt(BitWidth, 0);   // X % X ===> 0;
    തിരികെ;
  }

  // Make sure there is enough space to hold the results.
  // NOTE: This assumes that reallocate won't affect any bits if it doesn't
  // change the size. This is necessary if Quotient or Remainder is aliased
  // with LHS or RHS.
  Quotient.reallocate(BitWidth);
  Remainder.reallocate(BitWidth);

  എങ്കിൽ (lhs_വാക്കുകൾ == 1) { // rhsWords is 1 if lhsWords is 1.
    // There is only one word to consider so use the native versions.
    uint64_t lhs_മൂല്യം = LHS.U.pVal[0];
    uint64_t rhs_മൂല്യം = RHS.U.pVal[0];
    Quotient = lhs_മൂല്യം / rhs_മൂല്യം;
    Remainder = lhs_മൂല്യം % rhs_മൂല്യം;
    തിരികെ;
  }

  // Okay, lets do it the long way
  divide(LHS.U.pVal, lhs_വാക്കുകൾ, RHS.U.pVal, rhs_വാക്കുകൾ, Quotient.U.pVal,
         Remainder.U.pVal);
  // Clear the rest of the Quotient and Remainder.
  std::memset(Quotient.U.pVal + lhs_വാക്കുകൾ, 0,
              (get_അക്കം_വാക്കുകൾ(BitWidth) - lhs_വാക്കുകൾ) * APINT_WORD_SIZE);
  std::memset(Remainder.U.pVal + rhs_വാക്കുകൾ, 0,
              (get_അക്കം_വാക്കുകൾ(BitWidth) - rhs_വാക്കുകൾ) * APINT_WORD_SIZE);
}

ശൂന്യം APInt::udivrem(സ്ഥിരം APInt &LHS, uint64_t RHS, APInt &Quotient,
                    uint64_t &Remainder) {
  പ്രതിജ്ഞാ(RHS != 0 && "Divide by zero?");
  unsigned BitWidth = LHS.BitWidth;

  // First, deal with the easy case
  എങ്കിൽ (LLVM_LIKELY(LHS.isSingleWord())) {
    uint64_t QuotVal = LHS.U.VAL / RHS;
    Remainder = LHS.U.VAL % RHS;
    Quotient = APInt(BitWidth, QuotVal);
    തിരികെ;
  }

  // Get some size facts about the dividend and divisor
  unsigned lhs_വാക്കുകൾ = get_അക്കം_വാക്കുകൾ(LHS.getActiveBits());

  // Check the degenerate cases
  എങ്കിൽ (lhs_വാക്കുകൾ == 0) {
    Quotient = APInt(BitWidth, 0);    // 0 / Y ===> 0
    Remainder = 0;                    // 0 % Y ===> 0
    തിരികെ;
  }

  എങ്കിൽ (RHS == 1) {
    Quotient = LHS;                   // X / 1 ===> X
    Remainder = 0;                    // X % 1 ===> 0
    തിരികെ;
  }

  എങ്കിൽ (LHS.ult(RHS)) {
    Remainder = LHS.get_ZExt_മൂല്യം();   // X % Y ===> X, iff X < Y
    Quotient = APInt(BitWidth, 0);    // X / Y ===> 0, iff X < Y
    തിരികെ;
  }

  എങ്കിൽ (LHS == RHS) {
    Quotient  = APInt(BitWidth, 1);   // X / X ===> 1
    Remainder = 0;                    // X % X ===> 0;
    തിരികെ;
  }

  // Make sure there is enough space to hold the results.
  // NOTE: This assumes that reallocate won't affect any bits if it doesn't
  // change the size. This is necessary if Quotient is aliased with LHS.
  Quotient.reallocate(BitWidth);

  എങ്കിൽ (lhs_വാക്കുകൾ == 1) { // rhsWords is 1 if lhsWords is 1.
    // There is only one word to consider so use the native versions.
    uint64_t lhs_മൂല്യം = LHS.U.pVal[0];
    Quotient = lhs_മൂല്യം / RHS;
    Remainder = lhs_മൂല്യം % RHS;
    തിരികെ;
  }

  // Okay, lets do it the long way
  divide(LHS.U.pVal, lhs_വാക്കുകൾ, &RHS, 1, Quotient.U.pVal, &Remainder);
  // Clear the rest of the Quotient.
  std::memset(Quotient.U.pVal + lhs_വാക്കുകൾ, 0,
              (get_അക്കം_വാക്കുകൾ(BitWidth) - lhs_വാക്കുകൾ) * APINT_WORD_SIZE);
}

ശൂന്യം APInt::sdivrem(സ്ഥിരം APInt &LHS, സ്ഥിരം APInt &RHS,
                    APInt &Quotient, APInt &Remainder) {
  എങ്കിൽ (LHS.isNegative()) {
    എങ്കിൽ (RHS.isNegative())
      APInt::udivrem(-LHS, -RHS, Quotient, Remainder);
    അല്ലെങ്കിൽ {
      APInt::udivrem(-LHS, RHS, Quotient, Remainder);
      Quotient.negate();
    }
    Remainder.negate();
  } അല്ലെങ്കിൽ എങ്കിൽ (RHS.isNegative()) {
    APInt::udivrem(LHS, -RHS, Quotient, Remainder);
    Quotient.negate();
  } അല്ലെങ്കിൽ {
    APInt::udivrem(LHS, RHS, Quotient, Remainder);
  }
}

ശൂന്യം APInt::sdivrem(സ്ഥിരം APInt &LHS, int64_t RHS,
                    APInt &Quotient, int64_t &Remainder) {
  uint64_t R = Remainder;
  എങ്കിൽ (LHS.isNegative()) {
    എങ്കിൽ (RHS < 0)
      APInt::udivrem(-LHS, -RHS, Quotient, R);
    അല്ലെങ്കിൽ {
      APInt::udivrem(-LHS, RHS, Quotient, R);
      Quotient.negate();
    }
    R = -R;
  } അല്ലെങ്കിൽ എങ്കിൽ (RHS < 0) {
    APInt::udivrem(LHS, -RHS, Quotient, R);
    Quotient.negate();
  } അല്ലെങ്കിൽ {
    APInt::udivrem(LHS, RHS, Quotient, R);
  }
  Remainder = R;
}

APInt APInt::sadd_ov(സ്ഥിരം APInt &RHS, തർക്കം &Overflow) സ്ഥിരം {
  APInt Res = *ഇത്+RHS;
  Overflow = isNonNegative() == RHS.isNonNegative() &&
             Res.isNonNegative() != isNonNegative();
  തിരികെ Res;
}

APInt APInt::uadd_ov(സ്ഥിരം APInt &RHS, തർക്കം &Overflow) സ്ഥിരം {
  APInt Res = *ഇത്+RHS;
  Overflow = Res.ult(RHS);
  തിരികെ Res;
}

APInt APInt::ssub_ov(സ്ഥിരം APInt &RHS, തർക്കം &Overflow) സ്ഥിരം {
  APInt Res = *ഇത് - RHS;
  Overflow = isNonNegative() != RHS.isNonNegative() &&
             Res.isNonNegative() != isNonNegative();
  തിരികെ Res;
}

APInt APInt::usub_ov(സ്ഥിരം APInt &RHS, തർക്കം &Overflow) സ്ഥിരം {
  APInt Res = *ഇത്-RHS;
  Overflow = Res.ugt(*ഇത്);
  തിരികെ Res;
}

APInt APInt::sdiv_ov(സ്ഥിരം APInt &RHS, തർക്കം &Overflow) സ്ഥിരം {
  // MININT/-1  -->  overflow.
  Overflow = is_Min_Signed_മൂല്യം() && RHS.isAllOnes();
  തിരികെ sdiv(RHS);
}

APInt APInt::smul_ov(സ്ഥിരം APInt &RHS, തർക്കം &Overflow) സ്ഥിരം {
  APInt Res = *ഇത് * RHS;

  എങ്കിൽ (RHS != 0)
    Overflow = Res.sdiv(RHS) != *ഇത് ||
               (is_Min_Signed_മൂല്യം() && RHS.isAllOnes());
  അല്ലെങ്കിൽ
    Overflow = അസത്യം;
  തിരികെ Res;
}

APInt APInt::umul_ov(സ്ഥിരം APInt &RHS, തർക്കം &Overflow) സ്ഥിരം {
  എങ്കിൽ (countl_zero() + RHS.countl_zero() + 2 <= BitWidth) {
    Overflow = സത്യം;
    തിരികെ *ഇത് * RHS;
  }

  APInt Res = lshr(1) * RHS;
  Overflow = Res.isNegative();
  Res <<= 1;
  എങ്കിൽ ((*ഇത്)[0]) {
    Res += RHS;
    എങ്കിൽ (Res.ult(RHS))
      Overflow = സത്യം;
  }
  തിരികെ Res;
}

APInt APInt::sshl_ov(സ്ഥിരം APInt &ShAmt, തർക്കം &Overflow) സ്ഥിരം {
  തിരികെ sshl_ov(ShAmt.get_Limited_മൂല്യം(getBitWidth()), Overflow);
}

APInt APInt::sshl_ov(unsigned ShAmt, തർക്കം &Overflow) സ്ഥിരം {
  Overflow = ShAmt >= getBitWidth();
  എങ്കിൽ (Overflow)
    തിരികെ APInt(BitWidth, 0);

  എങ്കിൽ (isNonNegative()) // Don't allow sign change.
    Overflow = ShAmt >= countl_zero();
  അല്ലെങ്കിൽ
    Overflow = ShAmt >= countl_one();

  തിരികെ *ഇത് << ShAmt;
}

APInt APInt::ushl_ov(സ്ഥിരം APInt &ShAmt, തർക്കം &Overflow) സ്ഥിരം {
  തിരികെ ushl_ov(ShAmt.get_Limited_മൂല്യം(getBitWidth()), Overflow);
}

APInt APInt::ushl_ov(unsigned ShAmt, തർക്കം &Overflow) സ്ഥിരം {
  Overflow = ShAmt >= getBitWidth();
  എങ്കിൽ (Overflow)
    തിരികെ APInt(BitWidth, 0);

  Overflow = ShAmt > countl_zero();

  തിരികെ *ഇത് << ShAmt;
}

APInt APInt::sfloordiv_ov(സ്ഥിരം APInt &RHS, തർക്കം &Overflow) സ്ഥിരം {
  APInt quotient = sdiv_ov(RHS, Overflow);
  എങ്കിൽ ((quotient * RHS != *ഇത്) && (isNegative() != RHS.isNegative()))
    തിരികെ quotient - 1;
  തിരികെ quotient;
}

APInt APInt::sadd_sat(സ്ഥിരം APInt &RHS) സ്ഥിരം {
  തർക്കം Overflow;
  APInt Res = sadd_ov(RHS, Overflow);
  എങ്കിൽ (!Overflow)
    തിരികെ Res;

  തിരികെ isNegative() ? APInt::get_Signed_Min_മൂല്യം(BitWidth)
                      : APInt::get_Signed_Max_മൂല്യം(BitWidth);
}

APInt APInt::uadd_sat(സ്ഥിരം APInt &RHS) സ്ഥിരം {
  തർക്കം Overflow;
  APInt Res = uadd_ov(RHS, Overflow);
  എങ്കിൽ (!Overflow)
    തിരികെ Res;

  തിരികെ APInt::get_Max_മൂല്യം(BitWidth);
}

APInt APInt::ssub_sat(സ്ഥിരം APInt &RHS) സ്ഥിരം {
  തർക്കം Overflow;
  APInt Res = ssub_ov(RHS, Overflow);
  എങ്കിൽ (!Overflow)
    തിരികെ Res;

  തിരികെ isNegative() ? APInt::get_Signed_Min_മൂല്യം(BitWidth)
                      : APInt::get_Signed_Max_മൂല്യം(BitWidth);
}

APInt APInt::usub_sat(സ്ഥിരം APInt &RHS) സ്ഥിരം {
  തർക്കം Overflow;
  APInt Res = usub_ov(RHS, Overflow);
  എങ്കിൽ (!Overflow)
    തിരികെ Res;

  തിരികെ APInt(BitWidth, 0);
}

APInt APInt::smul_sat(സ്ഥിരം APInt &RHS) സ്ഥിരം {
  തർക്കം Overflow;
  APInt Res = smul_ov(RHS, Overflow);
  എങ്കിൽ (!Overflow)
    തിരികെ Res;

  // The result is negative if one and only one of inputs is negative.
  തർക്കം ResIsNegative = isNegative() ^ RHS.isNegative();

  തിരികെ ResIsNegative ? APInt::get_Signed_Min_മൂല്യം(BitWidth)
                       : APInt::get_Signed_Max_മൂല്യം(BitWidth);
}

APInt APInt::umul_sat(സ്ഥിരം APInt &RHS) സ്ഥിരം {
  തർക്കം Overflow;
  APInt Res = umul_ov(RHS, Overflow);
  എങ്കിൽ (!Overflow)
    തിരികെ Res;

  തിരികെ APInt::get_Max_മൂല്യം(BitWidth);
}

APInt APInt::sshl_sat(സ്ഥിരം APInt &RHS) സ്ഥിരം {
  തിരികെ sshl_sat(RHS.get_Limited_മൂല്യം(getBitWidth()));
}

APInt APInt::sshl_sat(unsigned RHS) സ്ഥിരം {
  തർക്കം Overflow;
  APInt Res = sshl_ov(RHS, Overflow);
  എങ്കിൽ (!Overflow)
    തിരികെ Res;

  തിരികെ isNegative() ? APInt::get_Signed_Min_മൂല്യം(BitWidth)
                      : APInt::get_Signed_Max_മൂല്യം(BitWidth);
}

APInt APInt::ushl_sat(സ്ഥിരം APInt &RHS) സ്ഥിരം {
  തിരികെ ushl_sat(RHS.get_Limited_മൂല്യം(getBitWidth()));
}

APInt APInt::ushl_sat(unsigned RHS) സ്ഥിരം {
  തർക്കം Overflow;
  APInt Res = ushl_ov(RHS, Overflow);
  എങ്കിൽ (!Overflow)
    തിരികെ Res;

  തിരികെ APInt::get_Max_മൂല്യം(BitWidth);
}

ശൂന്യം APInt::fromString(unsigned numbits, StringRef str, uint8_t radix) {
  // Check our assumptions here
  പ്രതിജ്ഞാ(!str.empty() && "Invalid string length");
  പ്രതിജ്ഞാ((radix == 10 || radix == 8 || radix == 16 || radix == 2 ||
          radix == 36) &&
         "Radix should be 2, 8, 10, 16, or 36!");

  StringRef::iterator p = str.begin();
  size_t slen = str.size();
  തർക്കം isNeg = *p == '-';
  എങ്കിൽ (*p == '-' || *p == '+') {
    p++;
    slen--;
    പ്രതിജ്ഞാ(slen && "String is only a sign, needs a value.");
  }
  പ്രതിജ്ഞാ((slen <= numbits || radix != 2) && "Insufficient bit width");
  പ്രതിജ്ഞാ(((slen-1)*3 <= numbits || radix != 8) && "Insufficient bit width");
  പ്രതിജ്ഞാ(((slen-1)*4 <= numbits || radix != 16) && "Insufficient bit width");
  പ്രതിജ്ഞാ((((slen-1)*64)/22 <= numbits || radix != 10) &&
         "Insufficient bit width");

  // Allocate memory if needed
  എങ്കിൽ (LLVM_LIKELY(isSingleWord()))
    U.VAL = 0;
  അല്ലെങ്കിൽ
    U.pVal = getClearedMemory(get_അക്കം_വാക്കുകൾ());

  // Figure out if we can shift instead of multiply
  unsigned shift = (radix == 16 ? 4 : radix == 8 ? 3 : radix == 2 ? 1 : 0);

  // Enter digit traversal loop
  വേണ്ടി (StringRef::iterator e = str.end(); p != e; ++p) {
    unsigned digit = getDigit(*p, radix);
    പ്രതിജ്ഞാ(digit < radix && "Invalid character in digit string");

    // Shift or multiply the value by the radix
    എങ്കിൽ (slen > 1) {
      എങ്കിൽ (shift)
        *ഇത് <<= shift;
      അല്ലെങ്കിൽ
        *ഇത് *= radix;
    }

    // Add in the digit we just interpreted
    *ഇത് += digit;
  }
  // If its negative, put it in two's complement form
  എങ്കിൽ (isNeg)
    ഇത്->negate();
}

ശൂന്യം APInt::toString(SmallVectorImpl<അക്ഷരം> &Str, unsigned Radix, തർക്കം Signed,
                     തർക്കം formatAsCLiteral, തർക്കം UpperCase,
                     തർക്കം തിരുകുക_Separators) സ്ഥിരം {
  പ്രതിജ്ഞാ((Radix == 10 || Radix == 8 || Radix == 16 || Radix == 2 ||
          Radix == 36) &&
         "Radix should be 2, 8, 10, 16, or 36!");

  സ്ഥിരം അക്ഷരം *Prefix = "";
  എങ്കിൽ (formatAsCLiteral) {
    തിരഞ്ഞെടുപ്പ് (Radix) {
      അവസ്ഥ 2:
        // Binary literals are a non-standard extension added in gcc 4.3:
        // http://gcc.gnu.org/onlinedocs/gcc-4.3.0/gcc/Binary-constants.html
        Prefix = "0b";
        നിർത്തുക;
      അവസ്ഥ 8:
        Prefix = "0";
        നിർത്തുക;
      അവസ്ഥ 10:
        നിർത്തുക; // No prefix
      അവസ്ഥ 16:
        Prefix = "0x";
        നിർത്തുക;
      സ്ഥിരസ്ഥിതി:
        llvm_unreachable("Invalid radix!");
    }
  }

  // Number of digits in a group between separators.
  unsigned Grouping = (Radix == 8 || Radix == 10) ? 3 : 4;

  // First, check for a zero value and just short circuit the logic below.
  എങ്കിൽ (isZero()) {
    ആകുമ്പോൾ (*Prefix) {
      Str.തള്ളുക_back(*Prefix);
      ++Prefix;
    };
    Str.തള്ളുക_back('0');
    തിരികെ;
  }

  സ്ഥിതം സ്ഥിരം അക്ഷരം BothDigits[] = "0123456789abcdefghijklmnopqrstuvwxyz"
                                   "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ";
  സ്ഥിരം അക്ഷരം *Digits = BothDigits + (UpperCase ? 36 : 0);

  എങ്കിൽ (LLVM_LIKELY(isSingleWord())) {
    അക്ഷരം Buffer[65];
    അക്ഷരം *BufPtr = std::end(Buffer);

    uint64_t N;
    എങ്കിൽ (!Signed) {
      N = get_ZExt_മൂല്യം();
    } അല്ലെങ്കിൽ {
      int64_t I = get_SExt_മൂല്യം();
      എങ്കിൽ (I >= 0) {
        N = I;
      } അല്ലെങ്കിൽ {
        Str.തള്ളുക_back('-');
        N = -(uint64_t)I;
      }
    }

    ആകുമ്പോൾ (*Prefix) {
      Str.തള്ളുക_back(*Prefix);
      ++Prefix;
    };

    പൂർണ്ണസംഖ്യ Pos = 0;
    ആകുമ്പോൾ (N) {
      എങ്കിൽ (തിരുകുക_Separators && Pos % Grouping == 0 && Pos > 0)
        *--BufPtr = '\'';
      *--BufPtr = Digits[N % Radix];
      N /= Radix;
      Pos++;
    }
    Str.append(BufPtr, std::end(Buffer));
    തിരികെ;
  }

  APInt Tmp(*ഇത്);

  എങ്കിൽ (Signed && isNegative()) {
    // They want to print the signed version and it is a negative value
    // Flip the bits and add one to turn it into the equivalent positive
    // value and put a '-' in the result.
    Tmp.negate();
    Str.തള്ളുക_back('-');
  }

  ആകുമ്പോൾ (*Prefix) {
    Str.തള്ളുക_back(*Prefix);
    ++Prefix;
  }

  // We insert the digits backward, then reverse them to get the right order.
  unsigned StartDig = Str.size();

  // For the 2, 8 and 16 bit cases, we can just shift instead of divide
  // because the number of bits per digit (1, 3 and 4 respectively) divides
  // equally.  We just shift until the value is zero.
  എങ്കിൽ (Radix == 2 || Radix == 8 || Radix == 16) {
    // Just shift tmp right for each digit width until it becomes zero
    unsigned ShiftAmt = (Radix == 16 ? 4 : (Radix == 8 ? 3 : 1));
    unsigned MaskAmt = Radix - 1;

    പൂർണ്ണസംഖ്യ Pos = 0;
    ആകുമ്പോൾ (Tmp.get_Bool_മൂല്യം()) {
      unsigned Digit = unsigned(Tmp.getRawData()[0]) & MaskAmt;
      എങ്കിൽ (തിരുകുക_Separators && Pos % Grouping == 0 && Pos > 0)
        Str.തള്ളുക_back('\'');

      Str.തള്ളുക_back(Digits[Digit]);
      Tmp.lshrInPlace(ShiftAmt);
      Pos++;
    }
  } അല്ലെങ്കിൽ {
    പൂർണ്ണസംഖ്യ Pos = 0;
    ആകുമ്പോൾ (Tmp.get_Bool_മൂല്യം()) {
      uint64_t Digit;
      udivrem(Tmp, Radix, Tmp, Digit);
      പ്രതിജ്ഞാ(Digit < Radix && "divide failed");
      എങ്കിൽ (തിരുകുക_Separators && Pos % Grouping == 0 && Pos > 0)
        Str.തള്ളുക_back('\'');

      Str.തള്ളുക_back(Digits[Digit]);
      Pos++;
    }
  }

  // Reverse the digits before returning.
  std::reverse(Str.begin()+StartDig, Str.end());
}

#എങ്കിൽ !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)
LLVM_DUMP_METHOD ശൂന്യം APInt::dump() സ്ഥിരം {
  SmallString<40> S, U;
  ഇത്->toStringUnsigned(U);
  ഇത്->toStringSigned(S);
  dbgs() << "APInt(" << BitWidth << "b, "
         << U << "u " << S << "s)\n";
}
#endif

ശൂന്യം APInt::print(raw_ostream &OS, തർക്കം isSigned) സ്ഥിരം {
  SmallString<40> S;
  ഇത്->toString(S, 10, isSigned, /* formatAsCLiteral = */അസത്യം);
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
സ്ഥിതം വരിയിൽ APInt::WordType താഴെയുള്ള_Bit_Mask(unsigned bits) {
  പ്രതിജ്ഞാ(bits != 0 && bits <= APInt::APINT_BITS_PER_WORD);
  തിരികെ ~(APInt::WordType) 0 >> (APInt::APINT_BITS_PER_WORD - bits);
}

/// Returns the value of the lower half of PART.
സ്ഥിതം വരിയിൽ APInt::WordType താഴെയുള്ള_Half(APInt::WordType part) {
  തിരികെ part & താഴെയുള്ള_Bit_Mask(APInt::APINT_BITS_PER_WORD / 2);
}

/// Returns the value of the upper half of PART.
സ്ഥിതം വരിയിൽ APInt::WordType മുകളിലെ_Half(APInt::WordType part) {
  തിരികെ part >> (APInt::APINT_BITS_PER_WORD / 2);
}

/// Sets the least significant part of a bignum to the input value, and zeroes
/// out higher parts.
ശൂന്യം APInt::tcSet(WordType *dst, WordType part, unsigned parts) {
  പ്രതിജ്ഞാ(parts > 0);
  dst[0] = part;
  വേണ്ടി (unsigned i = 1; i < parts; i++)
    dst[i] = 0;
}

/// Assign one bignum to another.
ശൂന്യം APInt::tcAssign(WordType *dst, സ്ഥിരം WordType *src, unsigned parts) {
  വേണ്ടി (unsigned i = 0; i < parts; i++)
    dst[i] = src[i];
}

/// Returns true if a bignum is zero, false otherwise.
തർക്കം APInt::tcIsZero(സ്ഥിരം WordType *src, unsigned parts) {
  വേണ്ടി (unsigned i = 0; i < parts; i++)
    എങ്കിൽ (src[i])
      തിരികെ അസത്യം;

  തിരികെ സത്യം;
}

/// Extract the given bit of a bignum; returns 0 or 1.
പൂർണ്ണസംഖ്യ APInt::tcExtractBit(സ്ഥിരം WordType *parts, unsigned bit) {
  തിരികെ (parts[whichWord(bit)] & maskBit(bit)) != 0;
}

/// Set the given bit of a bignum.
ശൂന്യം APInt::tcSetBit(WordType *parts, unsigned bit) {
  parts[whichWord(bit)] |= maskBit(bit);
}

/// Clears the given bit of a bignum.
ശൂന്യം APInt::tcClearBit(WordType *parts, unsigned bit) {
  parts[whichWord(bit)] &= ~maskBit(bit);
}

/// Returns the bit number of the least significant set bit of a number.  If the
/// input number has no bits set UINT_MAX is returned.
unsigned APInt::tcLSB(സ്ഥിരം WordType *parts, unsigned n) {
  വേണ്ടി (unsigned i = 0; i < n; i++) {
    എങ്കിൽ (parts[i] != 0) {
      unsigned lsb = llvm::countr_zero(parts[i]);
      തിരികെ lsb + i * APINT_BITS_PER_WORD;
    }
  }

  തിരികെ UINT_MAX;
}

/// Returns the bit number of the most significant set bit of a number.
/// If the input number has no bits set UINT_MAX is returned.
unsigned APInt::tcMSB(സ്ഥിരം WordType *parts, unsigned n) {
  ചെയ്യുക {
    --n;

    എങ്കിൽ (parts[n] != 0) {
      static_assert(വലുപ്പം(parts[n]) <= വലുപ്പം(uint64_t));
      unsigned msb = llvm::Log2_64(parts[n]);

      തിരികെ msb + n * APINT_BITS_PER_WORD;
    }
  } ആകുമ്പോൾ (n);

  തിരികെ UINT_MAX;
}

/// Copy the bit vector of width srcBITS from SRC, starting at bit srcLSB, to
/// DST, of dstCOUNT parts, such that the bit srcLSB becomes the least
/// significant bit of DST.  All high bits above srcBITS in DST are zero-filled.
/// */
ശൂന്യം
APInt::tcExtract(WordType *dst, unsigned dst_എണ്ണം, സ്ഥിരം WordType *src,
                 unsigned srcBits, unsigned srcLSB) {
  unsigned dstParts = (srcBits + APINT_BITS_PER_WORD - 1) / APINT_BITS_PER_WORD;
  പ്രതിജ്ഞാ(dstParts <= dst_എണ്ണം);

  unsigned firstSrcPart = srcLSB / APINT_BITS_PER_WORD;
  tcAssign(dst, src + firstSrcPart, dstParts);

  unsigned shift = srcLSB % APINT_BITS_PER_WORD;
  tc_Shift_വലത്(dst, dstParts, shift);

  // We now have (dstParts * APINT_BITS_PER_WORD - shift) bits from SRC
  // in DST.  If this is less that srcBits, append the rest, else
  // clear the high bits.
  unsigned n = dstParts * APINT_BITS_PER_WORD - shift;
  എങ്കിൽ (n < srcBits) {
    WordType mask = താഴെയുള്ള_Bit_Mask (srcBits - n);
    dst[dstParts - 1] |= ((src[firstSrcPart + dstParts] & mask)
                          << n % APINT_BITS_PER_WORD);
  } അല്ലെങ്കിൽ എങ്കിൽ (n > srcBits) {
    എങ്കിൽ (srcBits % APINT_BITS_PER_WORD)
      dst[dstParts - 1] &= താഴെയുള്ള_Bit_Mask (srcBits % APINT_BITS_PER_WORD);
  }

  // Clear high parts.
  ആകുമ്പോൾ (dstParts < dst_എണ്ണം)
    dst[dstParts++] = 0;
}

//// DST += RHS + C where C is zero or one.  Returns the carry flag.
APInt::WordType APInt::tcAdd(WordType *dst, സ്ഥിരം WordType *rhs,
                             WordType c, unsigned parts) {
  പ്രതിജ്ഞാ(c <= 1);

  വേണ്ടി (unsigned i = 0; i < parts; i++) {
    WordType l = dst[i];
    എങ്കിൽ (c) {
      dst[i] += rhs[i] + 1;
      c = (dst[i] <= l);
    } അല്ലെങ്കിൽ {
      dst[i] += rhs[i];
      c = (dst[i] < l);
    }
  }

  തിരികെ c;
}

/// This function adds a single "word" integer, src, to the multiple
/// "word" integer array, dst[]. dst[] is modified to reflect the addition and
/// 1 is returned if there is a carry out, otherwise 0 is returned.
/// @returns the carry of the addition.
APInt::WordType APInt::tcAddPart(WordType *dst, WordType src,
                                 unsigned parts) {
  വേണ്ടി (unsigned i = 0; i < parts; ++i) {
    dst[i] += src;
    എങ്കിൽ (dst[i] >= src)
      തിരികെ 0; // No need to carry so exit early.
    src = 1; // Carry one to next digit.
  }

  തിരികെ 1;
}

/// DST -= RHS + C where C is zero or one.  Returns the carry flag.
APInt::WordType APInt::tcSubtract(WordType *dst, സ്ഥിരം WordType *rhs,
                                  WordType c, unsigned parts) {
  പ്രതിജ്ഞാ(c <= 1);

  വേണ്ടി (unsigned i = 0; i < parts; i++) {
    WordType l = dst[i];
    എങ്കിൽ (c) {
      dst[i] -= rhs[i] + 1;
      c = (dst[i] >= l);
    } അല്ലെങ്കിൽ {
      dst[i] -= rhs[i];
      c = (dst[i] > l);
    }
  }

  തിരികെ c;
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
  വേണ്ടി (unsigned i = 0; i < parts; ++i) {
    WordType Dst = dst[i];
    dst[i] -= src;
    എങ്കിൽ (src <= Dst)
      തിരികെ 0; // No need to borrow so exit early.
    src = 1; // We have to "borrow 1" from next "word"
  }

  തിരികെ 1;
}

/// Negate a bignum in-place.
ശൂന്യം APInt::tcNegate(WordType *dst, unsigned parts) {
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
പൂർണ്ണസംഖ്യ APInt::tcMultiplyPart(WordType *dst, സ്ഥിരം WordType *src,
                          WordType multiplier, WordType carry,
                          unsigned srcParts, unsigned dstParts,
                          തർക്കം add) {
  // Otherwise our writes of DST kill our later reads of SRC.
  പ്രതിജ്ഞാ(dst <= src || dst >= src + srcParts);
  പ്രതിജ്ഞാ(dstParts <= srcParts + 1);

  // N loops; minimum of dstParts and srcParts.
  unsigned n = std::min(dstParts, srcParts);

  വേണ്ടി (unsigned i = 0; i < n; i++) {
    // [LOW, HIGH] = MULTIPLIER * SRC[i] + DST[i] + CARRY.
    // This cannot overflow, because:
    //   (n - 1) * (n - 1) + 2 (n - 1) = (n - 1) * (n + 1)
    // which is less than n^2.
    WordType srcPart = src[i];
    WordType താഴെയുള്ള, mid, മുകളിലെ;
    എങ്കിൽ (multiplier == 0 || srcPart == 0) {
      താഴെയുള്ള = carry;
      മുകളിലെ = 0;
    } അല്ലെങ്കിൽ {
      താഴെയുള്ള = താഴെയുള്ള_Half(srcPart) * താഴെയുള്ള_Half(multiplier);
      മുകളിലെ = മുകളിലെ_Half(srcPart) * മുകളിലെ_Half(multiplier);

      mid = താഴെയുള്ള_Half(srcPart) * മുകളിലെ_Half(multiplier);
      മുകളിലെ += മുകളിലെ_Half(mid);
      mid <<= APINT_BITS_PER_WORD / 2;
      എങ്കിൽ (താഴെയുള്ള + mid < താഴെയുള്ള)
        മുകളിലെ++;
      താഴെയുള്ള += mid;

      mid = മുകളിലെ_Half(srcPart) * താഴെയുള്ള_Half(multiplier);
      മുകളിലെ += മുകളിലെ_Half(mid);
      mid <<= APINT_BITS_PER_WORD / 2;
      എങ്കിൽ (താഴെയുള്ള + mid < താഴെയുള്ള)
        മുകളിലെ++;
      താഴെയുള്ള += mid;

      // Now add carry.
      എങ്കിൽ (താഴെയുള്ള + carry < താഴെയുള്ള)
        മുകളിലെ++;
      താഴെയുള്ള += carry;
    }

    എങ്കിൽ (add) {
      // And now DST[i], and store the new low part there.
      എങ്കിൽ (താഴെയുള്ള + dst[i] < താഴെയുള്ള)
        മുകളിലെ++;
      dst[i] += താഴെയുള്ള;
    } അല്ലെങ്കിൽ {
      dst[i] = താഴെയുള്ള;
    }

    carry = മുകളിലെ;
  }

  എങ്കിൽ (srcParts < dstParts) {
    // Full multiplication, there is no overflow.
    പ്രതിജ്ഞാ(srcParts + 1 == dstParts);
    dst[srcParts] = carry;
    തിരികെ 0;
  }

  // We overflowed if there is carry.
  എങ്കിൽ (carry)
    തിരികെ 1;

  // We would overflow if any significant unwritten parts would be
  // non-zero.  This is true if any remaining src parts are non-zero
  // and the multiplier is non-zero.
  എങ്കിൽ (multiplier)
    വേണ്ടി (unsigned i = dstParts; i < srcParts; i++)
      എങ്കിൽ (src[i])
        തിരികെ 1;

  // We fitted in the narrow destination.
  തിരികെ 0;
}

/// DST = LHS * RHS, where DST has the same width as the operands and
/// is filled with the least significant parts of the result.  Returns
/// one if overflow occurred, otherwise zero.  DST must be disjoint
/// from both operands.
പൂർണ്ണസംഖ്യ APInt::tcMultiply(WordType *dst, സ്ഥിരം WordType *lhs,
                      സ്ഥിരം WordType *rhs, unsigned parts) {
  പ്രതിജ്ഞാ(dst != lhs && dst != rhs);

  പൂർണ്ണസംഖ്യ overflow = 0;

  വേണ്ടി (unsigned i = 0; i < parts; i++) {
    // Don't accumulate on the first iteration so we don't need to initalize
    // dst to 0.
    overflow |=
        tcMultiplyPart(&dst[i], lhs, rhs[i], 0, parts, parts - i, i != 0);
  }

  തിരികെ overflow;
}

/// DST = LHS * RHS, where DST has width the sum of the widths of the
/// operands. No overflow occurs. DST must be disjoint from both operands.
ശൂന്യം APInt::tcFullMultiply(WordType *dst, സ്ഥിരം WordType *lhs,
                           സ്ഥിരം WordType *rhs, unsigned lhsParts,
                           unsigned rhsParts) {
  // Put the narrower number on the LHS for less loops below.
  എങ്കിൽ (lhsParts > rhsParts)
    തിരികെ tcFullMultiply (dst, rhs, lhs, rhsParts, lhsParts);

  പ്രതിജ്ഞാ(dst != lhs && dst != rhs);

  വേണ്ടി (unsigned i = 0; i < lhsParts; i++) {
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
പൂർണ്ണസംഖ്യ APInt::tcDivide(WordType *lhs, സ്ഥിരം WordType *rhs,
                    WordType *remainder, WordType *srhs,
                    unsigned parts) {
  പ്രതിജ്ഞാ(lhs != remainder && lhs != srhs && remainder != srhs);

  unsigned shift_എണ്ണം = tcMSB(rhs, parts) + 1;
  എങ്കിൽ (shift_എണ്ണം == 0)
    തിരികെ സത്യം;

  shift_എണ്ണം = parts * APINT_BITS_PER_WORD - shift_എണ്ണം;
  unsigned n = shift_എണ്ണം / APINT_BITS_PER_WORD;
  WordType mask = (WordType) 1 << (shift_എണ്ണം % APINT_BITS_PER_WORD);

  tcAssign(srhs, rhs, parts);
  tc_Shift_ഇടത്(srhs, parts, shift_എണ്ണം);
  tcAssign(remainder, lhs, parts);
  tcSet(lhs, 0, parts);

  // Loop, subtracting SRHS if REMAINDER is greater and adding that to the
  // total.
  വേണ്ടി (;;) {
    പൂർണ്ണസംഖ്യ compare = tcCompare(remainder, srhs, parts);
    എങ്കിൽ (compare >= 0) {
      tcSubtract(remainder, srhs, 0, parts);
      lhs[n] |= mask;
    }

    എങ്കിൽ (shift_എണ്ണം == 0)
      നിർത്തുക;
    shift_എണ്ണം--;
    tc_Shift_വലത്(srhs, parts, 1);
    എങ്കിൽ ((mask >>= 1) == 0) {
      mask = (WordType) 1 << (APINT_BITS_PER_WORD - 1);
      n--;
    }
  }

  തിരികെ അസത്യം;
}

/// Shift a bignum left Count bits in-place. Shifted in bits are zero. There are
/// no restrictions on Count.
ശൂന്യം APInt::tc_Shift_ഇടത്(WordType *Dst, unsigned വാക്കുകൾ_2, unsigned എണ്ണം_2) {
  // Don't bother performing a no-op shift.
  എങ്കിൽ (!എണ്ണം_2)
    തിരികെ;

  // WordShift is the inter-part shift; BitShift is the intra-part shift.
  unsigned WordShift = std::min(എണ്ണം_2 / APINT_BITS_PER_WORD, വാക്കുകൾ_2);
  unsigned BitShift = എണ്ണം_2 % APINT_BITS_PER_WORD;

  // Fastpath for moving by whole words.
  എങ്കിൽ (BitShift == 0) {
    std::memmove(Dst + WordShift, Dst, (വാക്കുകൾ_2 - WordShift) * APINT_WORD_SIZE);
  } അല്ലെങ്കിൽ {
    ആകുമ്പോൾ (വാക്കുകൾ_2-- > WordShift) {
      Dst[വാക്കുകൾ_2] = Dst[വാക്കുകൾ_2 - WordShift] << BitShift;
      എങ്കിൽ (വാക്കുകൾ_2 > WordShift)
        Dst[വാക്കുകൾ_2] |=
          Dst[വാക്കുകൾ_2 - WordShift - 1] >> (APINT_BITS_PER_WORD - BitShift);
    }
  }

  // Fill in the remainder with 0s.
  std::memset(Dst, 0, WordShift * APINT_WORD_SIZE);
}

/// Shift a bignum right Count bits in-place. Shifted in bits are zero. There
/// are no restrictions on Count.
ശൂന്യം APInt::tc_Shift_വലത്(WordType *Dst, unsigned വാക്കുകൾ_2, unsigned എണ്ണം_2) {
  // Don't bother performing a no-op shift.
  എങ്കിൽ (!എണ്ണം_2)
    തിരികെ;

  // WordShift is the inter-part shift; BitShift is the intra-part shift.
  unsigned WordShift = std::min(എണ്ണം_2 / APINT_BITS_PER_WORD, വാക്കുകൾ_2);
  unsigned BitShift = എണ്ണം_2 % APINT_BITS_PER_WORD;

  unsigned വാക്കുകൾ_To_Move = വാക്കുകൾ_2 - WordShift;
  // Fastpath for moving by whole words.
  എങ്കിൽ (BitShift == 0) {
    std::memmove(Dst, Dst + WordShift, വാക്കുകൾ_To_Move * APINT_WORD_SIZE);
  } അല്ലെങ്കിൽ {
    വേണ്ടി (unsigned i = 0; i != വാക്കുകൾ_To_Move; ++i) {
      Dst[i] = Dst[i + WordShift] >> BitShift;
      എങ്കിൽ (i + 1 != വാക്കുകൾ_To_Move)
        Dst[i] |= Dst[i + WordShift + 1] << (APINT_BITS_PER_WORD - BitShift);
    }
  }

  // Fill in the remainder with 0s.
  std::memset(Dst + വാക്കുകൾ_To_Move, 0, WordShift * APINT_WORD_SIZE);
}

// Comparison (unsigned) of two bignums.
പൂർണ്ണസംഖ്യ APInt::tcCompare(സ്ഥിരം WordType *lhs, സ്ഥിരം WordType *rhs,
                     unsigned parts) {
  ആകുമ്പോൾ (parts) {
    parts--;
    എങ്കിൽ (lhs[parts] != rhs[parts])
      തിരികെ (lhs[parts] > rhs[parts]) ? 1 : -1;
  }

  തിരികെ 0;
}

APInt llvm::APIntOps::RoundingUDiv(സ്ഥിരം APInt &A, സ്ഥിരം APInt &B,
                                   APInt::Rounding RM) {
  // Currently udivrem always rounds down.
  തിരഞ്ഞെടുപ്പ് (RM) {
  അവസ്ഥ APInt::Rounding::DOWN:
  അവസ്ഥ APInt::Rounding::TOWARD_ZERO:
    തിരികെ A.udiv(B);
  അവസ്ഥ APInt::Rounding::UP: {
    APInt Quo, Rem;
    APInt::udivrem(A, B, Quo, Rem);
    എങ്കിൽ (Rem.isZero())
      തിരികെ Quo;
    തിരികെ Quo + 1;
  }
  }
  llvm_unreachable("Unknown APInt::Rounding enum");
}

APInt llvm::APIntOps::RoundingSDiv(സ്ഥിരം APInt &A, സ്ഥിരം APInt &B,
                                   APInt::Rounding RM) {
  തിരഞ്ഞെടുപ്പ് (RM) {
  അവസ്ഥ APInt::Rounding::DOWN:
  അവസ്ഥ APInt::Rounding::UP: {
    APInt Quo, Rem;
    APInt::sdivrem(A, B, Quo, Rem);
    എങ്കിൽ (Rem.isZero())
      തിരികെ Quo;
    // This algorithm deals with arbitrary rounding mode used by sdivrem.
    // We want to check whether the non-integer part of the mathematical value
    // is negative or not. If the non-integer part is negative, we need to round
    // down from Quo; otherwise, if it's positive or 0, we return Quo, as it's
    // already rounded down.
    എങ്കിൽ (RM == APInt::Rounding::DOWN) {
      എങ്കിൽ (Rem.isNegative() != B.isNegative())
        തിരികെ Quo - 1;
      തിരികെ Quo;
    }
    എങ്കിൽ (Rem.isNegative() != B.isNegative())
      തിരികെ Quo;
    തിരികെ Quo + 1;
  }
  // Currently sdiv rounds towards zero.
  അവസ്ഥ APInt::Rounding::TOWARD_ZERO:
    തിരികെ A.sdiv(B);
  }
  llvm_unreachable("Unknown APInt::Rounding enum");
}

std::optional<APInt>
llvm::APIntOps::SolveQuadraticEquationWrap(APInt A, APInt B, APInt C,
                                           unsigned RangeWidth) {
  unsigned CoeffWidth = A.getBitWidth();
  പ്രതിജ്ഞാ(CoeffWidth == B.getBitWidth() && CoeffWidth == C.getBitWidth());
  പ്രതിജ്ഞാ(RangeWidth <= CoeffWidth &&
         "Value range width should be less than coefficient width");
  പ്രതിജ്ഞാ(RangeWidth > 1 && "Value range bit width should be > 1");

  LLVM_DEBUG(dbgs() << __func__ << ": solving " << A << "x^2 + " << B
                    << "x + " << C << ", rw:" << RangeWidth << '\n');

  // Identify 0 as a (non)solution immediately.
  എങ്കിൽ (C.sextOrTrunc(RangeWidth).isZero()) {
    LLVM_DEBUG(dbgs() << __func__ << ": zero solution\n");
    തിരികെ APInt(CoeffWidth, 0);
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
  എങ്കിൽ (A.isNegative()) {
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
  തർക്കം Pick_താഴെയുള്ള;

  auto RoundUp = [] (സ്ഥിരം APInt &V, സ്ഥിരം APInt &A) -> APInt {
    പ്രതിജ്ഞാ(A.isStrictlyPositive());
    APInt T = V.abs().urem(A);
    എങ്കിൽ (T.isZero())
      തിരികെ V;
    തിരികെ V.isNegative() ? V+T : V+(A-T);
  };

  // The vertex of the parabola is at -B/2A, but since A > 0, it's negative
  // iff B is positive.
  എങ്കിൽ (B.isNonNegative()) {
    // If B >= 0, the vertex it at a negative location (or at 0), so in
    // order to have a non-negative solution we need to pick k that makes
    // C-kR negative. To satisfy all the requirements for the solution
    // that we are looking for, it needs to be closest to 0 of all k.
    C = C.srem(R);
    എങ്കിൽ (C.isStrictlyPositive())
      C -= R;
    // Pick the greater solution.
    Pick_താഴെയുള്ള = അസത്യം;
  } അല്ലെങ്കിൽ {
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
    എങ്കിൽ (C.sgt(LowkR)) {
      // If LowkR < C, then such a k is guaranteed to exist because
      // LowkR itself is a multiple of R.
      C -= -RoundUp(-C, R);      // C = C - RoundDown(C, R)
      // Pick the smaller solution.
      Pick_താഴെയുള്ള = സത്യം;
    } അല്ലെങ്കിൽ {
      // If C-kR < 0 for all potential k's, it means that one solution
      // will be negative, while the other will be positive. The positive
      // solution will shift towards 0 if the parabola is moved up.
      // Pick the kR closest to the lower bound (i.e. make C-kR closest
      // to 0, or in other words, out of all parabolas that have solutions,
      // pick the one that is the farthest "up").
      // Since LowkR is itself a multiple of R, simply take C-LowkR.
      C -= LowkR;
      // Pick the greater solution.
      Pick_താഴെയുള്ള = അസത്യം;
    }
  }

  LLVM_DEBUG(dbgs() << __func__ << ": updated coefficients " << A << "x^2 + "
                    << B << "x + " << C << ", rw:" << RangeWidth << '\n');

  APInt D = SqrB - 4*A*C;
  പ്രതിജ്ഞാ(D.isNonNegative() && "Negative discriminant");
  APInt SQ = D.sqrtFloor();

  APInt Q = SQ * SQ;
  തർക്കം InexactSQ = Q != D;

  APInt X;
  APInt Rem;

  // SQ is rounded down (i.e SQ * SQ <= D), so the roots may be inexact.
  // When using the quadratic formula directly, the calculated low root
  // may be greater than the exact one, since we would be subtracting SQ.
  // To make sure that the calculated root is not greater than the exact
  // one, subtract SQ+1 when calculating the low root (for inexact value
  // of SQ).
  എങ്കിൽ (Pick_താഴെയുള്ള)
    APInt::sdivrem(-B - (SQ+InexactSQ), TwoA, X, Rem);
  അല്ലെങ്കിൽ
    APInt::sdivrem(-B + SQ, TwoA, X, Rem);

  // The updated coefficients should be such that the (exact) solution is
  // positive. Since APInt division rounds towards 0, the calculated one
  // can be 0, but cannot be negative.
  പ്രതിജ്ഞാ(X.isNonNegative() && "Solution should be non-negative");

  എങ്കിൽ (!InexactSQ && Rem.isZero()) {
    LLVM_DEBUG(dbgs() << __func__ << ": solution (root): " << X << '\n');
    തിരികെ X;
  }

  പ്രതിജ്ഞാ((SQ*SQ).sle(D) && "SQ = |_sqrt(D)_|, so SQ*SQ <= D");
  // The exact value of the square root of D should be between SQ and SQ+1.
  // This implies that the solution should be between that corresponding to
  // SQ (i.e. X) and that corresponding to SQ+1.
  //
  // The calculated X cannot be greater than the exact (real) solution.
  // Actually it must be strictly less than the exact solution, while
  // X+1 will be greater than or equal to it.

  APInt VX = (A*X + B)*X + C;
  APInt VY = VX + TwoA*X + A + B;
  തർക്കം SignChange =
      VX.isNegative() != VY.isNegative() || VX.isZero() != VY.isZero();
  // If the sign did not change between X and X+1, X is not a valid solution.
  // This could happen when the actual (exact) roots don't have an integer
  // between them, so they would both be contained between X and X+1.
  എങ്കിൽ (!SignChange) {
    LLVM_DEBUG(dbgs() << __func__ << ": no valid solution\n");
    തിരികെ std::nullopt;
  }

  X += 1;
  LLVM_DEBUG(dbgs() << __func__ << ": solution (wrap): " << X << '\n');
  തിരികെ X;
}

std::optional<unsigned>
llvm::APIntOps::GetMostSignificantDifferentBit(സ്ഥിരം APInt &A, സ്ഥിരം APInt &B) {
  പ്രതിജ്ഞാ(A.getBitWidth() == B.getBitWidth() && "Must have the same bitwidth");
  എങ്കിൽ (A == B)
    തിരികെ std::nullopt;
  തിരികെ A.getBitWidth() - ((A ^ B).countl_zero() + 1);
}

APInt llvm::APIntOps::ScaleBitMask(സ്ഥിരം APInt &A, unsigned NewBitWidth,
                                   തർക്കം MatchAllBits) {
  unsigned OldBitWidth = A.getBitWidth();
  പ്രതിജ്ഞാ((((OldBitWidth % NewBitWidth) == 0) ||
          ((NewBitWidth % OldBitWidth) == 0)) &&
         "One size should be a multiple of the other one. "
         "Can't do fractional scaling.");

  // Check for matching bitwidths.
  എങ്കിൽ (OldBitWidth == NewBitWidth)
    തിരികെ A;

  APInt NewA = APInt::getZero(NewBitWidth);

  // Check for null input.
  എങ്കിൽ (A.isZero())
    തിരികെ NewA;

  എങ്കിൽ (NewBitWidth > OldBitWidth) {
    // Repeat bits.
    unsigned Scale = NewBitWidth / OldBitWidth;
    വേണ്ടി (unsigned i = 0; i != OldBitWidth; ++i)
      എങ്കിൽ (A[i])
        NewA.setBits(i * Scale, (i + 1) * Scale);
  } അല്ലെങ്കിൽ {
    unsigned Scale = OldBitWidth / NewBitWidth;
    വേണ്ടി (unsigned i = 0; i != NewBitWidth; ++i) {
      എങ്കിൽ (MatchAllBits) {
        എങ്കിൽ (A.extractBits(Scale, i * Scale).isAllOnes())
          NewA.setBit(i);
      } അല്ലെങ്കിൽ {
        എങ്കിൽ (!A.extractBits(Scale, i * Scale).isZero())
          NewA.setBit(i);
      }
    }
  }

  തിരികെ NewA;
}

/// StoreIntToMemory - Fills the StoreBytes bytes of memory starting from Dst
/// with the integer held in IntVal.
ശൂന്യം llvm::StoreIntToMemory(സ്ഥിരം APInt &IntVal, uint8_t *Dst,
                            unsigned StoreBytes) {
  പ്രതിജ്ഞാ((IntVal.getBitWidth()+7)/8 >= StoreBytes && "Integer too small!");
  സ്ഥിരം uint8_t *Src = (സ്ഥിരം uint8_t *)IntVal.getRawData();

  എങ്കിൽ (sys::IsLittleEndianHost) {
    // Little-endian host - the source is ordered from LSB to MSB.  Order the
    // destination from LSB to MSB: Do a straight copy.
    memcpy(Dst, Src, StoreBytes);
  } അല്ലെങ്കിൽ {
    // Big-endian host - the source is an array of 64 bit words ordered from
    // LSW to MSW.  Each word is ordered from MSB to LSB.  Order the destination
    // from MSB to LSB: Reverse the word order, but not the bytes in a word.
    ആകുമ്പോൾ (StoreBytes > വലുപ്പം(uint64_t)) {
      StoreBytes -= വലുപ്പം(uint64_t);
      // May not be aligned so use memcpy.
      memcpy(Dst + StoreBytes, Src, വലുപ്പം(uint64_t));
      Src += വലുപ്പം(uint64_t);
    }

    memcpy(Dst, Src + വലുപ്പം(uint64_t) - StoreBytes, StoreBytes);
  }
}

/// LoadIntFromMemory - Loads the integer stored in the LoadBytes bytes starting
/// from Src into IntVal, which is assumed to be wide enough and to hold zero.
ശൂന്യം llvm::LoadIntFromMemory(APInt &IntVal, സ്ഥിരം uint8_t *Src,
                             unsigned LoadBytes) {
  പ്രതിജ്ഞാ((IntVal.getBitWidth()+7)/8 >= LoadBytes && "Integer too small!");
  uint8_t *Dst = reinterpret_cast<uint8_t *>(
                   const_cast<uint64_t *>(IntVal.getRawData()));

  എങ്കിൽ (sys::IsLittleEndianHost)
    // Little-endian host - the destination must be ordered from LSB to MSB.
    // The source is ordered from LSB to MSB: Do a straight copy.
    memcpy(Dst, Src, LoadBytes);
  അല്ലെങ്കിൽ {
    // Big-endian - the destination is an array of 64 bit words ordered from
    // LSW to MSW.  Each word must be ordered from MSB to LSB.  The source is
    // ordered from MSB to LSB: Reverse the word order, but not the bytes in
    // a word.
    ആകുമ്പോൾ (LoadBytes > വലുപ്പം(uint64_t)) {
      LoadBytes -= വലുപ്പം(uint64_t);
      // May not be aligned so use memcpy.
      memcpy(Dst, Src + LoadBytes, വലുപ്പം(uint64_t));
      Dst += വലുപ്പം(uint64_t);
    }

    memcpy(Dst + വലുപ്പം(uint64_t) - LoadBytes, Src, LoadBytes);
  }
}

APInt APIntOps::avgFloorS(സ്ഥിരം APInt &C1, സ്ഥിരം APInt &C2) {
  // Return floor((C1 + C2) / 2)
  തിരികെ (C1 & C2) + (C1 ^ C2).ashr(1);
}

APInt APIntOps::avgFloorU(സ്ഥിരം APInt &C1, സ്ഥിരം APInt &C2) {
  // Return floor((C1 + C2) / 2)
  തിരികെ (C1 & C2) + (C1 ^ C2).lshr(1);
}

APInt APIntOps::avgCeilS(സ്ഥിരം APInt &C1, സ്ഥിരം APInt &C2) {
  // Return ceil((C1 + C2) / 2)
  തിരികെ (C1 | C2) - (C1 ^ C2).ashr(1);
}

APInt APIntOps::avgCeilU(സ്ഥിരം APInt &C1, സ്ഥിരം APInt &C2) {
  // Return ceil((C1 + C2) / 2)
  തിരികെ (C1 | C2) - (C1 ^ C2).lshr(1);
}

APInt APIntOps::avgSubFloorS(സ്ഥിരം APInt &C1, സ്ഥിരം APInt &C2) {
  // Return floor((C1 - C2) / 2), using C1 - C2 == (C1 ^ C2) - 2 * (~C1 & C2).
  തിരികെ (C1 ^ C2).ashr(1) - (~C1 & C2);
}

APInt APIntOps::avgSubFloorU(സ്ഥിരം APInt &C1, സ്ഥിരം APInt &C2) {
  // Return floor((C1 - C2) / 2), using C1 - C2 == (C1 ^ C2) - 2 * (~C1 & C2).
  തിരികെ (C1 ^ C2).lshr(1) - (~C1 & C2);
}

APInt APIntOps::mulhs(സ്ഥിരം APInt &C1, സ്ഥിരം APInt &C2) {
  പ്രതിജ്ഞാ(C1.getBitWidth() == C2.getBitWidth() && "Unequal bitwidths");
  unsigned FullWidth = C1.getBitWidth() * 2;
  APInt C1Ext = C1.sext(FullWidth);
  APInt C2Ext = C2.sext(FullWidth);
  തിരികെ (C1Ext * C2Ext).extractBits(C1.getBitWidth(), C1.getBitWidth());
}

APInt APIntOps::mulhu(സ്ഥിരം APInt &C1, സ്ഥിരം APInt &C2) {
  പ്രതിജ്ഞാ(C1.getBitWidth() == C2.getBitWidth() && "Unequal bitwidths");
  unsigned FullWidth = C1.getBitWidth() * 2;
  APInt C1Ext = C1.zext(FullWidth);
  APInt C2Ext = C2.zext(FullWidth);
  തിരികെ (C1Ext * C2Ext).extractBits(C1.getBitWidth(), C1.getBitWidth());
}

APInt APIntOps::mulsExtended(സ്ഥിരം APInt &C1, സ്ഥിരം APInt &C2) {
  പ്രതിജ്ഞാ(C1.getBitWidth() == C2.getBitWidth() && "Unequal bitwidths");
  unsigned FullWidth = C1.getBitWidth() * 2;
  APInt C1Ext = C1.sext(FullWidth);
  APInt C2Ext = C2.sext(FullWidth);
  തിരികെ C1Ext * C2Ext;
}

APInt APIntOps::muluExtended(സ്ഥിരം APInt &C1, സ്ഥിരം APInt &C2) {
  പ്രതിജ്ഞാ(C1.getBitWidth() == C2.getBitWidth() && "Unequal bitwidths");
  unsigned FullWidth = C1.getBitWidth() * 2;
  APInt C1Ext = C1.zext(FullWidth);
  APInt C2Ext = C2.zext(FullWidth);
  തിരികെ C1Ext * C2Ext;
}

APInt APIntOps::pow(സ്ഥിരം APInt &X, int64_t N) {
  പ്രതിജ്ഞാ(N >= 0 && "negative exponents not supported.");
  APInt Acc = APInt(X.getBitWidth(), 1);
  എങ്കിൽ (N == 0)
    തിരികെ Acc;
  APInt Base = X;
  int64_t RemainingExponent = N;
  ആകുമ്പോൾ (RemainingExponent > 0) {
    ആകുമ്പോൾ (RemainingExponent % 2 == 0) {
      Base *= Base;
      RemainingExponent /= 2;
    }
    --RemainingExponent;
    Acc *= Base;
  }
  തിരികെ Acc;
}

APInt llvm::APIntOps::fshl(സ്ഥിരം APInt &Hi, സ്ഥിരം APInt &Lo,
                           സ്ഥിരം APInt &Shift) {
  പ്രതിജ്ഞാ(Hi.getBitWidth() == Lo.getBitWidth());
  unsigned ShiftAmt = rotateModulo(Hi.getBitWidth(), Shift);
  എങ്കിൽ (ShiftAmt == 0)
    തിരികെ Hi;
  തിരികെ Hi.shl(ShiftAmt) | Lo.lshr(Hi.getBitWidth() - ShiftAmt);
}

APInt llvm::APIntOps::fshr(സ്ഥിരം APInt &Hi, സ്ഥിരം APInt &Lo,
                           സ്ഥിരം APInt &Shift) {
  പ്രതിജ്ഞാ(Hi.getBitWidth() == Lo.getBitWidth());
  unsigned ShiftAmt = rotateModulo(Hi.getBitWidth(), Shift);
  എങ്കിൽ (ShiftAmt == 0)
    തിരികെ Lo;
  തിരികെ Hi.shl(Hi.getBitWidth() - ShiftAmt) | Lo.lshr(ShiftAmt);
}

APInt llvm::APIntOps::clmul(സ്ഥിരം APInt &LHS, സ്ഥിരം APInt &RHS) {
  unsigned BW = LHS.getBitWidth();
  പ്രതിജ്ഞാ(BW == RHS.getBitWidth() && "Operand mismatch");
  APInt Result(BW, 0);
  വേണ്ടി (unsigned I : seq(std::min(RHS.getActiveBits(), BW - LHS.countr_zero())))
    എങ്കിൽ (RHS[I])
      Result ^= LHS << I;
  തിരികെ Result;
}

APInt llvm::APIntOps::clmulr(സ്ഥിരം APInt &LHS, സ്ഥിരം APInt &RHS) {
  പ്രതിജ്ഞാ(LHS.getBitWidth() == RHS.getBitWidth());
  തിരികെ clmul(LHS.reverseBits(), RHS.reverseBits()).reverseBits();
}

APInt llvm::APIntOps::clmulh(സ്ഥിരം APInt &LHS, സ്ഥിരം APInt &RHS) {
  പ്രതിജ്ഞാ(LHS.getBitWidth() == RHS.getBitWidth());
  തിരികെ clmulr(LHS, RHS).lshr(1);
}

APInt llvm::APIntOps::pext(സ്ഥിരം APInt &Val, സ്ഥിരം APInt &Mask) {
  unsigned BW = Val.getBitWidth();
  പ്രതിജ്ഞാ(BW == Mask.getBitWidth() && "Operand mismatch");
  APInt Result = APInt::getZero(BW);
  വേണ്ടി (unsigned I = 0, P = 0; I != BW; ++I)
    എങ്കിൽ (Mask[I])
      Result.setBitVal(P++, Val[I]);
  തിരികെ Result;
}

APInt llvm::APIntOps::pdep(സ്ഥിരം APInt &Val, സ്ഥിരം APInt &Mask) {
  unsigned BW = Val.getBitWidth();
  പ്രതിജ്ഞാ(BW == Mask.getBitWidth() && "Operand mismatch");
  APInt Result = APInt::getZero(BW);
  വേണ്ടി (unsigned I = 0, P = 0; I != BW; ++I)
    എങ്കിൽ (Mask[I])
      Result.setBitVal(I, Val[P++]);
  തിരികെ Result;
}
