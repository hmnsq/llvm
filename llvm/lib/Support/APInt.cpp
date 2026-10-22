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

using नामाकाश llvm;

#define DEBUG_TYPE "apint"

/// A utility function for allocating memory, checking for allocation failures,
/// and ensuring the contents are zeroed.
अन्तःस्थ स्थिर uint64_t* getClearedMemory(unsigned अंक_शब्दगण) {
  प्रत्यागम नूतन uint64_t[अंक_शब्दगण]();
}

/// A utility function for allocating memory and checking for allocation
/// failure.  The content is not zeroed.
अन्तःस्थ स्थिर uint64_t* getMemory(unsigned अंक_शब्दगण) {
  प्रत्यागम नूतन uint64_t[अंक_शब्दगण];
}

/// A utility function that converts a character to a digit.
अन्तःस्थ स्थिर unsigned getDigit(वर्ण cdigit, uint8_t radix) {
  unsigned r;

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
    U.pVal = getMemory(get_अंक_शब्दगण());
    U.pVal[0] = val;
    memset(&U.pVal[1], 0xFF, APINT_WORD_SIZE * (get_अंक_शब्दगण() - 1));
    clearUnusedBits();
  } अन्यथा {
    U.pVal = getClearedMemory(get_अंक_शब्दगण());
    U.pVal[0] = val;
  }
}

शून्य APInt::initSlowCase(ध्रुव APInt& that) {
  U.pVal = getMemory(get_अंक_शब्दगण());
  memcpy(U.pVal, that.U.pVal, get_अंक_शब्दगण() * APINT_WORD_SIZE);
}

शून्य APInt::initFromArray(ArrayRef<uint64_t> bigVal) {
  प्रतिज्ञा(bigVal.data() && "Null pointer detected!");
  यदि (LLVM_LIKELY(isSingleWord()))
    U.VAL = bigVal[0];
  अन्यथा {
    // Get memory, cleared to 0
    U.pVal = getClearedMemory(get_अंक_शब्दगण());
    // Calculate the number of words to copy
    unsigned शब्दगण = std::min<unsigned>(bigVal.size(), get_अंक_शब्दगण());
    // Copy the words from bigVal to pVal
    memcpy(U.pVal, bigVal.data(), शब्दगण * APINT_WORD_SIZE);
  }
  // Make sure unused high bits are cleared
  clearUnusedBits();
}

APInt::APInt(unsigned अंक_Bits, ArrayRef<uint64_t> bigVal) : BitWidth(अंक_Bits) {
  initFromArray(bigVal);
}

APInt::APInt(unsigned numbits, StringRef Str, uint8_t radix)
    : BitWidth(numbits) {
  fromString(numbits, Str, radix);
}

शून्य APInt::reallocate(unsigned NewBitWidth) {
  // If the number of words is the same we can just change the width and stop.
  यदि (get_अंक_शब्दगण() == get_अंक_शब्दगण(NewBitWidth)) {
    BitWidth = NewBitWidth;
    प्रत्यागम;
  }

  // If we have an allocation, delete it.
  यदि (LLVM_UNLIKELY(!isSingleWord()))
    delete [] U.pVal;

  // Update BitWidth.
  BitWidth = NewBitWidth;

  // If we are supposed to have an allocation, create it.
  यदि (LLVM_UNLIKELY(!isSingleWord()))
    U.pVal = getMemory(get_अंक_शब्दगण());
}

शून्य APInt::assignSlowCase(ध्रुव APInt &RHS) {
  // Don't do anything for X = X
  यदि (एतत् == &RHS)
    प्रत्यागम;

  // Adjust the bit width and handle allocations as necessary.
  reallocate(RHS.getBitWidth());

  // Copy the data.
  यदि (LLVM_LIKELY(isSingleWord()))
    U.VAL = RHS.U.VAL;
  अन्यथा
    memcpy(U.pVal, RHS.U.pVal, get_अंक_शब्दगण() * APINT_WORD_SIZE);
}

/// This method 'profiles' an APInt for use with FoldingSet.
शून्य APInt::Profile(Folding_Set_गाँठ_ID& ID) ध्रुव {
  ID.AddInteger(BitWidth);

  यदि (LLVM_LIKELY(isSingleWord())) {
    ID.AddInteger(U.VAL);
    प्रत्यागम;
  }

  unsigned अंक_शब्दगण_2 = get_अंक_शब्दगण();
  प्रति (unsigned i = 0; i < अंक_शब्दगण_2; ++i)
    ID.AddInteger(U.pVal[i]);
}

तार्किक APInt::isAligned(Align A) ध्रुव {
  यदि (isZero())
    प्रत्यागम सत्य;
  ध्रुव unsigned TrailingZeroes = countr_zero();
  ध्रुव unsigned MinimumTrailingZeroes = Log2(A);
  प्रत्यागम TrailingZeroes >= MinimumTrailingZeroes;
}

/// Prefix increment operator. Increments the APInt by one.
APInt& APInt::operator++() {
  यदि (LLVM_LIKELY(isSingleWord()))
    ++U.VAL;
  अन्यथा
    tcIncrement(U.pVal, get_अंक_शब्दगण());
  प्रत्यागम clearUnusedBits();
}

/// Prefix decrement operator. Decrements the APInt by one.
APInt& APInt::operator--() {
  यदि (LLVM_LIKELY(isSingleWord()))
    --U.VAL;
  अन्यथा
    tcDecrement(U.pVal, get_अंक_शब्दगण());
  प्रत्यागम clearUnusedBits();
}

/// Adds the RHS APInt to this APInt.
/// @returns this, after addition of RHS.
/// Addition assignment operator.
APInt& APInt::operator+=(ध्रुव APInt& RHS) {
  प्रतिज्ञा(BitWidth == RHS.BitWidth && "Bit widths must be the same");
  यदि (LLVM_LIKELY(isSingleWord()))
    U.VAL += RHS.U.VAL;
  अन्यथा
    tcAdd(U.pVal, RHS.U.pVal, 0, get_अंक_शब्दगण());
  प्रत्यागम clearUnusedBits();
}

APInt& APInt::operator+=(uint64_t RHS) {
  यदि (LLVM_LIKELY(isSingleWord()))
    U.VAL += RHS;
  अन्यथा
    tcAddPart(U.pVal, RHS, get_अंक_शब्दगण());
  प्रत्यागम clearUnusedBits();
}

/// Subtracts the RHS APInt from this APInt
/// @returns this, after subtraction
/// Subtraction assignment operator.
APInt& APInt::operator-=(ध्रुव APInt& RHS) {
  प्रतिज्ञा(BitWidth == RHS.BitWidth && "Bit widths must be the same");
  यदि (LLVM_LIKELY(isSingleWord()))
    U.VAL -= RHS.U.VAL;
  अन्यथा
    tcSubtract(U.pVal, RHS.U.pVal, 0, get_अंक_शब्दगण());
  प्रत्यागम clearUnusedBits();
}

APInt& APInt::operator-=(uint64_t RHS) {
  यदि (LLVM_LIKELY(isSingleWord()))
    U.VAL -= RHS;
  अन्यथा
    tcSubtractPart(U.pVal, RHS, get_अंक_शब्दगण());
  प्रत्यागम clearUnusedBits();
}

APInt APInt::operator*(ध्रुव APInt& RHS) ध्रुव {
  प्रतिज्ञा(BitWidth == RHS.BitWidth && "Bit widths must be the same");
  यदि (LLVM_LIKELY(isSingleWord()))
    प्रत्यागम APInt(BitWidth, U.VAL * RHS.U.VAL, /*isSigned=*/असत्य,
                 /*implicitTrunc=*/सत्य);

  APInt Result(getMemory(get_अंक_शब्दगण()), getBitWidth());
  tcMultiply(Result.U.pVal, U.pVal, RHS.U.pVal, get_अंक_शब्दगण());
  Result.clearUnusedBits();
  प्रत्यागम Result;
}

शून्य APInt::andAssignSlowCase(ध्रुव APInt &RHS) {
  WordType *dst = U.pVal, *rhs = RHS.U.pVal;
  प्रति (size_t i = 0, e = get_अंक_शब्दगण(); i != e; ++i)
    dst[i] &= rhs[i];
}

शून्य APInt::orAssignSlowCase(ध्रुव APInt &RHS) {
  WordType *dst = U.pVal, *rhs = RHS.U.pVal;
  प्रति (size_t i = 0, e = get_अंक_शब्दगण(); i != e; ++i)
    dst[i] |= rhs[i];
}

शून्य APInt::xorAssignSlowCase(ध्रुव APInt &RHS) {
  WordType *dst = U.pVal, *rhs = RHS.U.pVal;
  प्रति (size_t i = 0, e = get_अंक_शब्दगण(); i != e; ++i)
    dst[i] ^= rhs[i];
}

APInt &APInt::operator*=(ध्रुव APInt &RHS) {
  *एतत् = *एतत् * RHS;
  प्रत्यागम *एतत्;
}

APInt& APInt::operator*=(uint64_t RHS) {
  यदि (LLVM_LIKELY(isSingleWord())) {
    U.VAL *= RHS;
  } अन्यथा {
    unsigned अंक_शब्दगण_2 = get_अंक_शब्दगण();
    tcMultiplyPart(U.pVal, U.pVal, RHS, 0, अंक_शब्दगण_2, अंक_शब्दगण_2, असत्य);
  }
  प्रत्यागम clearUnusedBits();
}

तार्किक APInt::equalSlowCase(ध्रुव APInt &RHS) ध्रुव {
  प्रत्यागम std::equal(U.pVal, U.pVal + get_अंक_शब्दगण(), RHS.U.pVal);
}

पूर्णाङ्क APInt::compare(ध्रुव APInt& RHS) ध्रुव {
  प्रतिज्ञा(BitWidth == RHS.BitWidth && "Bit widths must be same for comparison");
  यदि (LLVM_LIKELY(isSingleWord()))
    प्रत्यागम U.VAL < RHS.U.VAL ? -1 : U.VAL > RHS.U.VAL;

  प्रत्यागम tcCompare(U.pVal, RHS.U.pVal, get_अंक_शब्दगण());
}

पूर्णाङ्क APInt::compareSigned(ध्रुव APInt& RHS) ध्रुव {
  प्रतिज्ञा(BitWidth == RHS.BitWidth && "Bit widths must be same for comparison");
  यदि (LLVM_LIKELY(isSingleWord())) {
    int64_t lhsSext = SignExtend64(U.VAL, BitWidth);
    int64_t rhsSext = SignExtend64(RHS.U.VAL, BitWidth);
    प्रत्यागम lhsSext < rhsSext ? -1 : lhsSext > rhsSext;
  }

  तार्किक lhsNeg = isNegative();
  तार्किक rhsNeg = RHS.isNegative();

  // If the sign bits don't match, then (LHS < RHS) if LHS is negative
  यदि (lhsNeg != rhsNeg)
    प्रत्यागम lhsNeg ? -1 : 1;

  // Otherwise we can just use an unsigned comparison, because even negative
  // numbers compare correctly this way if both have the same signed-ness.
  प्रत्यागम tcCompare(U.pVal, RHS.U.pVal, get_अंक_शब्दगण());
}

शून्य APInt::setBitsSlowCase(unsigned loBit, unsigned hiBit) {
  unsigned loWord = whichWord(loBit);
  unsigned hiWord = whichWord(hiBit);

  // Create an initial mask for the low word with zeros below loBit.
  uint64_t loMask = WORDTYPE_MAX << whichBit(loBit);

  // If hiBit is not aligned, we need a high mask.
  unsigned hiShiftAmt = whichBit(hiBit);
  यदि (hiShiftAmt != 0) {
    // Create a high mask with zeros above hiBit.
    uint64_t hiMask = WORDTYPE_MAX >> (APINT_BITS_PER_WORD - hiShiftAmt);
    // If loWord and hiWord are equal, then we combine the masks. Otherwise,
    // set the bits in hiWord.
    यदि (hiWord == loWord)
      loMask &= hiMask;
    अन्यथा
      U.pVal[hiWord] |= hiMask;
  }
  // Apply the mask to the low word.
  U.pVal[loWord] |= loMask;

  // Fill any words between loWord and hiWord with all ones.
  प्रति (unsigned word = loWord + 1; word < hiWord; ++word)
    U.pVal[word] = WORDTYPE_MAX;
}

शून्य APInt::clearBitsSlowCase(unsigned LoBit, unsigned HiBit) {
  unsigned LoWord = whichWord(LoBit);
  unsigned HiWord = whichWord(HiBit);

  // Create an initial mask for the low word with ones below loBit.
  uint64_t LoMask = ~(WORDTYPE_MAX << whichBit(LoBit));

  // If HiBit is not aligned, we need a high mask.
  unsigned HiShiftAmt = whichBit(HiBit);
  यदि (HiShiftAmt != 0) {
    // Create a high mask with ones above HiBit.
    uint64_t HiMask = ~(WORDTYPE_MAX >> (APINT_BITS_PER_WORD - HiShiftAmt));
    // If LoWord and HiWord are equal, then we combine the masks. Otherwise,
    // clear the bits in HiWord.
    यदि (HiWord == LoWord)
      LoMask |= HiMask;
    अन्यथा
      U.pVal[HiWord] &= HiMask;
  }
  // Apply the mask to the low word.
  U.pVal[LoWord] &= LoMask;

  // Fill any words between LoWord and HiWord with all zeros.
  प्रति (unsigned Word = LoWord + 1; Word < HiWord; ++Word)
    U.pVal[Word] = 0;
}

// Complement a bignum in-place.
स्थिर शून्य tcComplement(APInt::WordType *dst, unsigned parts) {
  प्रति (unsigned i = 0; i < parts; i++)
    dst[i] = ~dst[i];
}

/// Toggle every bit to its opposite value.
शून्य APInt::flipAllBitsSlowCase() {
  tcComplement(U.pVal, get_अंक_शब्दगण());
  clearUnusedBits();
}

/// Concatenate the bits from "NewLSB" onto the bottom of *this.  This is
/// equivalent to:
///   (this->zext(NewWidth) << NewLSB.getBitWidth()) | NewLSB.zext(NewWidth)
/// In the slow case, we know the result is large.
APInt APInt::concatSlowCase(ध्रुव APInt &NewLSB) ध्रुव {
  unsigned NewWidth = getBitWidth() + NewLSB.getBitWidth();
  APInt Result = NewLSB.zext(NewWidth);
  Result.जोड़ो_Bits(*एतत्, NewLSB.getBitWidth());
  प्रत्यागम Result;
}

/// Toggle a given bit to its opposite value whose position is given
/// as "bitPosition".
/// Toggles a given bit to its opposite value.
शून्य APInt::flipBit(unsigned bitPosition) {
  प्रतिज्ञा(bitPosition < BitWidth && "Out of the bit-width range!");
  setBitVal(bitPosition, !(*एतत्)[bitPosition]);
}

शून्य APInt::जोड़ो_Bits(ध्रुव APInt &subBits, unsigned bitPosition) {
  unsigned subBitWidth = subBits.getBitWidth();
  प्रतिज्ञा((subBitWidth + bitPosition) <= BitWidth && "Illegal bit insertion");

  // inserting no bits is a noop.
  यदि (subBitWidth == 0)
    प्रत्यागम;

  // Insertion is a direct copy.
  यदि (subBitWidth == BitWidth) {
    *एतत् = subBits;
    प्रत्यागम;
  }

  // Single word result can be done as a direct bitmask.
  यदि (LLVM_LIKELY(isSingleWord())) {
    uint64_t mask = WORDTYPE_MAX >> (APINT_BITS_PER_WORD - subBitWidth);
    U.VAL &= ~(mask << bitPosition);
    U.VAL |= (subBits.U.VAL << bitPosition);
    प्रत्यागम;
  }

  unsigned loBit = whichBit(bitPosition);
  unsigned loWord = whichWord(bitPosition);
  unsigned hi1Word = whichWord(bitPosition + subBitWidth - 1);

  // Insertion within a single word can be done as a direct bitmask.
  यदि (loWord == hi1Word) {
    uint64_t mask = WORDTYPE_MAX >> (APINT_BITS_PER_WORD - subBitWidth);
    U.pVal[loWord] &= ~(mask << loBit);
    U.pVal[loWord] |= (subBits.U.VAL << loBit);
    प्रत्यागम;
  }

  // Insert on word boundaries.
  यदि (loBit == 0) {
    // Direct copy whole words.
    unsigned अंक_Whole_Sub_शब्दगण = subBitWidth / APINT_BITS_PER_WORD;
    memcpy(U.pVal + loWord, subBits.getRawData(),
           अंक_Whole_Sub_शब्दगण * APINT_WORD_SIZE);

    // Mask+insert remaining bits.
    unsigned remainingBits = subBitWidth % APINT_BITS_PER_WORD;
    यदि (remainingBits != 0) {
      uint64_t mask = WORDTYPE_MAX >> (APINT_BITS_PER_WORD - remainingBits);
      U.pVal[hi1Word] &= ~mask;
      U.pVal[hi1Word] |= subBits.getWord(subBitWidth - 1);
    }
    प्रत्यागम;
  }

  // General case - set/clear individual bits in dst based on src.
  // TODO - there is scope for optimization here, but at the moment this code
  // path is barely used so prefer readability over performance.
  प्रति (unsigned i = 0; i != subBitWidth; ++i)
    setBitVal(bitPosition + i, subBits[i]);
}

शून्य APInt::जोड़ो_Bits(uint64_t subBits, unsigned bitPosition, unsigned अंक_Bits) {
  uint64_t maskBits = maskTrailingOnes<uint64_t>(अंक_Bits);
  subBits &= maskBits;
  यदि (LLVM_LIKELY(isSingleWord())) {
    U.VAL &= ~(maskBits << bitPosition);
    U.VAL |= subBits << bitPosition;
    प्रत्यागम;
  }

  unsigned loBit = whichBit(bitPosition);
  unsigned loWord = whichWord(bitPosition);
  unsigned hiWord = whichWord(bitPosition + अंक_Bits - 1);
  यदि (loWord == hiWord) {
    U.pVal[loWord] &= ~(maskBits << loBit);
    U.pVal[loWord] |= subBits << loBit;
    प्रत्यागम;
  }

  static_assert(8 * परिमाण(WordType) <= 64, "This code assumes only two words affected");
  unsigned wordBits = 8 * परिमाण(WordType);
  U.pVal[loWord] &= ~(maskBits << loBit);
  U.pVal[loWord] |= subBits << loBit;

  U.pVal[hiWord] &= ~(maskBits >> (wordBits - loBit));
  U.pVal[hiWord] |= subBits >> (wordBits - loBit);
}

APInt APInt::extractBits(unsigned अंक_Bits, unsigned bitPosition) ध्रुव {
  प्रतिज्ञा(bitPosition < BitWidth && (अंक_Bits + bitPosition) <= BitWidth &&
         "Illegal bit extraction");

  यदि (LLVM_LIKELY(isSingleWord()))
    प्रत्यागम APInt(अंक_Bits, U.VAL >> bitPosition, /*isSigned=*/असत्य,
                 /*implicitTrunc=*/सत्य);

  unsigned loBit = whichBit(bitPosition);
  unsigned loWord = whichWord(bitPosition);
  unsigned hiWord = whichWord(bitPosition + अंक_Bits - 1);

  // Single word result extracting bits from a single word source.
  यदि (loWord == hiWord)
    प्रत्यागम APInt(अंक_Bits, U.pVal[loWord] >> loBit, /*isSigned=*/असत्य,
                 /*implicitTrunc=*/सत्य);

  // Extracting bits that start on a source word boundary can be done
  // as a fast memory copy.
  यदि (loBit == 0)
    प्रत्यागम APInt(अंक_Bits, ArrayRef(U.pVal + loWord, 1 + hiWord - loWord));

  // General case - shift + copy source words directly into place.
  APInt Result(अंक_Bits, 0);
  unsigned अंक_Src_शब्दगण = get_अंक_शब्दगण();
  unsigned अंक_Dst_शब्दगण = Result.get_अंक_शब्दगण();

  uint64_t *DestPtr =
      LLVM_LIKELY(Result.isSingleWord()) ? &Result.U.VAL : Result.U.pVal;
  प्रति (unsigned word = 0; word < अंक_Dst_शब्दगण; ++word) {
    uint64_t w0 = U.pVal[loWord + word];
    uint64_t w1 =
        (loWord + word + 1) < अंक_Src_शब्दगण ? U.pVal[loWord + word + 1] : 0;
    DestPtr[word] = (w0 >> loBit) | (w1 << (APINT_BITS_PER_WORD - loBit));
  }

  प्रत्यागम Result.clearUnusedBits();
}

uint64_t APInt::extract_Bits_As_ZExt_मूल्य(unsigned अंक_Bits,
                                       unsigned bitPosition) ध्रुव {
  प्रतिज्ञा(bitPosition < BitWidth && (अंक_Bits + bitPosition) <= BitWidth &&
         "Illegal bit extraction");
  प्रतिज्ञा(अंक_Bits <= 64 && "Illegal bit extraction");

  uint64_t maskBits = maskTrailingOnes<uint64_t>(अंक_Bits);
  यदि (LLVM_LIKELY(isSingleWord()))
    प्रत्यागम (U.VAL >> bitPosition) & maskBits;

  static_assert(APINT_BITS_PER_WORD >= 64,
                "This code assumes only two words affected");
  unsigned loBit = whichBit(bitPosition);
  unsigned loWord = whichWord(bitPosition);
  unsigned hiWord = whichWord(bitPosition + अंक_Bits - 1);
  यदि (loWord == hiWord)
    प्रत्यागम (U.pVal[loWord] >> loBit) & maskBits;

  uint64_t retBits = U.pVal[loWord] >> loBit;
  retBits |= U.pVal[hiWord] << (APINT_BITS_PER_WORD - loBit);
  retBits &= maskBits;
  प्रत्यागम retBits;
}

unsigned APInt::getSufficientBitsNeeded(StringRef Str, uint8_t Radix) {
  प्रतिज्ञा(!Str.empty() && "Invalid string length");
  size_t StrLen = Str.size();

  // Each computation below needs to know if it's negative.
  unsigned IsNegative = असत्य;
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

unsigned APInt::getBitsNeeded(StringRef str, uint8_t radix) {
  // Compute a sufficient number of bits that is always large enough but might
  // be too large.
  unsigned sufficient = getSufficientBitsNeeded(str, radix);

  // For bases 2, 8, and 16, the sufficient number of bits is exact and we can
  // return the value directly. For bases 10 and 36, we need to do extra work.
  यदि (radix == 2 || radix == 8 || radix == 16)
    प्रत्यागम sufficient;

  // This is grossly inefficient but accurate. We could probably do something
  // with a computation of roughly slen*64/20 and then adjust by the value of
  // the first few digits. But, I'm not sure how accurate that could be.
  size_t slen = str.size();

  // Each computation below needs to know if it's negative.
  StringRef::iterator p = str.begin();
  unsigned isNegative = *p == '-';
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
  unsigned log = tmp.logBase2();
  यदि (log == (unsigned)-1) {
    प्रत्यागम isNegative + 1;
  } अन्यथा यदि (isNegative && tmp.isPowerOf2()) {
    प्रत्यागम isNegative + log;
  } अन्यथा {
    प्रत्यागम isNegative + log + 1;
  }
}

hash_code llvm::hash_मूल्य(ध्रुव APInt &Arg) {
  यदि (LLVM_LIKELY(Arg.isSingleWord()))
    प्रत्यागम hash_combine(Arg.BitWidth, Arg.U.VAL);

  प्रत्यागम hash_combine(
      Arg.BitWidth,
      hash_combine_range(Arg.U.pVal, Arg.U.pVal + Arg.get_अंक_शब्दगण()));
}

unsigned DenseMapInfo<APInt, शून्य>::get_Hash_मूल्य(ध्रुव APInt &कुंजी) {
  प्रत्यागम static_cast<unsigned>(hash_मूल्य(कुंजी));
}

तार्किक APInt::isSplat(unsigned SplatSizeInBits) ध्रुव {
  प्रतिज्ञा(getBitWidth() % SplatSizeInBits == 0 &&
         "SplatSizeInBits must divide width!");
  // We can check that all parts of an integer are equal by making use of a
  // little trick: rotate and check if it's still the same value.
  प्रत्यागम *एतत् == rotl(SplatSizeInBits);
}

/// This function returns the high "numBits" bits of this APInt.
APInt APInt::getHiBits(unsigned अंक_Bits) ध्रुव {
  प्रत्यागम एतत्->lshr(BitWidth - अंक_Bits);
}

/// This function returns the low "numBits" bits of this APInt.
APInt APInt::getLoBits(unsigned अंक_Bits) ध्रुव {
  APInt Result(get_निचला_Bits_Set(BitWidth, अंक_Bits));
  Result &= *एतत्;
  प्रत्यागम Result;
}

/// Return a value containing V broadcasted over NewLen bits.
APInt APInt::getSplat(unsigned NewLen, ध्रुव APInt &V) {
  प्रतिज्ञा(NewLen >= V.getBitWidth() && "Can't splat to smaller bit width!");

  APInt Val = V.zext(NewLen);
  प्रति (unsigned I = V.getBitWidth(); I < NewLen; I <<= 1)
    Val |= Val << I;

  प्रत्यागम Val;
}

unsigned APInt::गिनती_Leading_Zeros_Slow_Case() ध्रुव {
  unsigned गिनती_2 = 0;
  प्रति (पूर्णाङ्क i = get_अंक_शब्दगण() - 1; i >= 0; --i) {
    uint64_t V = U.pVal[i];
    यदि (V == 0)
      गिनती_2 += APINT_BITS_PER_WORD;
    अन्यथा {
      गिनती_2 += llvm::countl_zero(V);
      भङ्ग;
    }
  }
  // Adjust for unused bits in the most significant word (they are zero).
  unsigned Mod = BitWidth % APINT_BITS_PER_WORD;
  गिनती_2 -= Mod > 0 ? APINT_BITS_PER_WORD - Mod : 0;
  प्रत्यागम गिनती_2;
}

unsigned APInt::गिनती_Leading_Ones_Slow_Case() ध्रुव {
  unsigned ऊपरी_Word_Bits = BitWidth % APINT_BITS_PER_WORD;
  unsigned shift;
  यदि (!ऊपरी_Word_Bits) {
    ऊपरी_Word_Bits = APINT_BITS_PER_WORD;
    shift = 0;
  } अन्यथा {
    shift = APINT_BITS_PER_WORD - ऊपरी_Word_Bits;
  }
  पूर्णाङ्क i = get_अंक_शब्दगण() - 1;
  unsigned गिनती_2 = llvm::countl_one(U.pVal[i] << shift);
  यदि (गिनती_2 == ऊपरी_Word_Bits) {
    प्रति (i--; i >= 0; --i) {
      यदि (U.pVal[i] == WORDTYPE_MAX)
        गिनती_2 += APINT_BITS_PER_WORD;
      अन्यथा {
        गिनती_2 += llvm::countl_one(U.pVal[i]);
        भङ्ग;
      }
    }
  }
  प्रत्यागम गिनती_2;
}

unsigned APInt::गिनती_Trailing_Zeros_Slow_Case() ध्रुव {
  unsigned गिनती_2 = 0;
  unsigned i = 0;
  प्रति (; i < get_अंक_शब्दगण() && U.pVal[i] == 0; ++i)
    गिनती_2 += APINT_BITS_PER_WORD;
  यदि (i < get_अंक_शब्दगण())
    गिनती_2 += llvm::countr_zero(U.pVal[i]);
  प्रत्यागम std::min(गिनती_2, BitWidth);
}

unsigned APInt::गिनती_Trailing_Ones_Slow_Case() ध्रुव {
  unsigned गिनती_2 = 0;
  unsigned i = 0;
  प्रति (; i < get_अंक_शब्दगण() && U.pVal[i] == WORDTYPE_MAX; ++i)
    गिनती_2 += APINT_BITS_PER_WORD;
  यदि (i < get_अंक_शब्दगण())
    गिनती_2 += llvm::countr_one(U.pVal[i]);
  प्रतिज्ञा(गिनती_2 <= BitWidth);
  प्रत्यागम गिनती_2;
}

unsigned APInt::गिनती_Population_Slow_Case() ध्रुव {
  unsigned गिनती_2 = 0;
  प्रति (unsigned i = 0; i < get_अंक_शब्दगण(); ++i)
    गिनती_2 += llvm::popcount(U.pVal[i]);
  प्रत्यागम गिनती_2;
}

तार्किक APInt::isPowerOf2SlowCase() ध्रुव {
  unsigned गिनती_2 = 0;
  प्रति (unsigned i = 0; i < get_अंक_शब्दगण(); ++i) {
    गिनती_2 += llvm::popcount(U.pVal[i]);
    यदि (गिनती_2 > 1)
      प्रत्यागम असत्य;
  }
  प्रत्यागम गिनती_2 == 1;
}

तार्किक APInt::intersectsSlowCase(ध्रुव APInt &RHS) ध्रुव {
  प्रति (unsigned i = 0, e = get_अंक_शब्दगण(); i != e; ++i)
    यदि ((U.pVal[i] & RHS.U.pVal[i]) != 0)
      प्रत्यागम सत्य;

  प्रत्यागम असत्य;
}

तार्किक APInt::isSubsetOfSlowCase(ध्रुव APInt &RHS) ध्रुव {
  प्रति (unsigned i = 0, e = get_अंक_शब्दगण(); i != e; ++i)
    यदि ((U.pVal[i] & ~RHS.U.pVal[i]) != 0)
      प्रत्यागम असत्य;

  प्रत्यागम सत्य;
}

तार्किक APInt::isInverseOfSlowCase(ध्रुव APInt &RHS) ध्रुव {
  ध्रुव unsigned Last = get_अंक_शब्दगण() - 1;
  प्रति (unsigned I = 0; I != Last; ++I)
    यदि ((U.pVal[I] ^ RHS.U.pVal[I]) != WORDTYPE_MAX)
      प्रत्यागम असत्य;

  unsigned TailBits = BitWidth - Last * APINT_BITS_PER_WORD;
  WordType TailMask = llvm::maskTrailingOnes<WordType>(TailBits);
  प्रत्यागम (U.pVal[Last] ^ RHS.U.pVal[Last]) == TailMask;
}

APInt APInt::byteSwap() ध्रुव {
  प्रतिज्ञा(BitWidth >= 16 && BitWidth % 8 == 0 && "Cannot byteswap!");
  यदि (BitWidth == 16)
    प्रत्यागम APInt(BitWidth, llvm::byteswap<uint16_t>(U.VAL));
  यदि (BitWidth == 32)
    प्रत्यागम APInt(BitWidth, llvm::byteswap<uint32_t>(U.VAL));
  यदि (BitWidth <= 64) {
    uint64_t Tmp1 = llvm::byteswap<uint64_t>(U.VAL);
    Tmp1 >>= (64 - BitWidth);
    प्रत्यागम APInt(BitWidth, Tmp1);
  }

  APInt Result(get_अंक_शब्दगण() * APINT_BITS_PER_WORD, 0);
  प्रति (unsigned I = 0, N = get_अंक_शब्दगण(); I != N; ++I)
    Result.U.pVal[I] = llvm::byteswap<uint64_t>(U.pVal[N - I - 1]);
  यदि (Result.BitWidth != BitWidth) {
    Result.lshrInPlace(Result.BitWidth - BitWidth);
    Result.BitWidth = BitWidth;
  }
  प्रत्यागम Result;
}

APInt APInt::reverseBits() ध्रुव {
  यदि (LLVM_LIKELY(isSingleWord())) {
    विकल्प (BitWidth) {
    पक्ष 64:
      प्रत्यागम APInt(BitWidth, llvm::reverseBits<uint64_t>(U.VAL));
    पक्ष 32:
      प्रत्यागम APInt(BitWidth, llvm::reverseBits<uint32_t>(U.VAL));
    पक्ष 16:
      प्रत्यागम APInt(BitWidth, llvm::reverseBits<uint16_t>(U.VAL));
    पक्ष 8:
      प्रत्यागम APInt(BitWidth, llvm::reverseBits<uint8_t>(U.VAL));
    पक्ष 1: // fallthrough
    पक्ष 0:
      प्रत्यागम *एतत्;
    स्वभाव:
      प्रत्यागम APInt(BitWidth,
                   llvm::reverseBits<uint64_t>(U.VAL) >> (64 - BitWidth));
    }
  }

  APInt Result(BitWidth, 0);
  unsigned अंक_शब्दगण_2 = get_अंक_शब्दगण();
  unsigned ExcessBits = अंक_शब्दगण_2 * APINT_BITS_PER_WORD - BitWidth;
  यदि (ExcessBits == 0) {
    // Fast path. No cross-word shift needed.
    प्रति (unsigned I = 0; I < अंक_शब्दगण_2; ++I)
      Result.U.pVal[I] = llvm::reverseBits<uint64_t>(U.pVal[अंक_शब्दगण_2 - 1 - I]);
    प्रत्यागम Result;
  }
  // Holds reversed bits of the previous (more significant) word.
  uint64_t PrevRev = llvm::reverseBits<uint64_t>(U.pVal[अंक_शब्दगण_2 - 1]);
  प्रति (unsigned I = 0; I < अंक_शब्दगण_2 - 1; ++I) {
    uint64_t CurrRev = llvm::reverseBits<uint64_t>(U.pVal[अंक_शब्दगण_2 - 2 - I]);
    Result.U.pVal[I] = (PrevRev >> ExcessBits) | (CurrRev << (64 - ExcessBits));
    PrevRev = CurrRev;
  }
  Result.U.pVal[अंक_शब्दगण_2 - 1] = PrevRev >> ExcessBits;
  प्रत्यागम Result;
}

APInt llvm::APIntOps::Greatest_Common_भाजक(APInt A, APInt B, तार्किक IsSigned) {
  // Take absolute value if IsSigned.
  यदि (IsSigned) {
    A = A.abs();
    B = B.abs();
  }

  // Fast-path a common case.
  यदि (A == B) प्रत्यागम A;

  // Corner cases: if either operand is zero, the other is the gcd.
  यदि (!A) प्रत्यागम B;
  यदि (!B) प्रत्यागम A;

  // Count common powers of 2 and remove all other powers of 2.
  unsigned Pow2;
  {
    unsigned Pow2_A = A.countr_zero();
    unsigned Pow2_B = B.countr_zero();
    यदि (Pow2_A > Pow2_B) {
      A.lshrInPlace(Pow2_A - Pow2_B);
      Pow2 = Pow2_B;
    } अन्यथा यदि (Pow2_B > Pow2_A) {
      B.lshrInPlace(Pow2_B - Pow2_A);
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
      A.lshrInPlace(A.countr_zero() - Pow2);
    } अन्यथा {
      B -= A;
      B.lshrInPlace(B.countr_zero() - Pow2);
    }
  }

  प्रत्यागम A;
}

APInt llvm::APIntOps::RoundDoubleToAPInt(द्विगुण Double, unsigned width) {
  uint64_t I = bit_cast<uint64_t>(Double);

  // Get the sign bit from the highest order bit
  तार्किक isNeg = I >> 63;

  // Get the 11-bit exponent and adjust for the 1023 bit bias
  int64_t exp = ((I >> 52) & 0x7ff) - 1023;

  // If the exponent is negative, the value is < 0 so just return 0.
  यदि (exp < 0)
    प्रत्यागम APInt(width, 0u);

  // Extract the mantissa by clearing the top 12 bits (sign + exponent).
  uint64_t mantissa = (I & (~0ULL >> 12)) | 1ULL << 52;

  // If the exponent doesn't shift all bits out of the mantissa
  यदि (exp < 52)
    प्रत्यागम isNeg ? -APInt(width, mantissa >> (52 - exp)) :
                    APInt(width, mantissa >> (52 - exp));

  // If the client didn't provide enough bits for us to shift the mantissa into
  // then the result is undefined, just return 0
  यदि (width <= exp - 52)
    प्रत्यागम APInt(width, 0);

  // Otherwise, we have to shift the mantissa bits up to the right location
  APInt Tmp(width, mantissa);
  Tmp <<= (unsigned)exp - 52;
  प्रत्यागम isNeg ? -Tmp : Tmp;
}

/// This function converts this APInt to a double.
/// The layout for double is as following (IEEE Standard 754):
///  --------------------------------------
/// |  Sign    Exponent    Fraction    Bias |
/// |-------------------------------------- |
/// |  1[63]   11[62-52]   52[51-00]   1023 |
///  --------------------------------------
द्विगुण APInt::roundToDouble(तार्किक isSigned) ध्रुव {
  // Handle the simple case where the value is contained in one uint64_t.
  // It is wrong to optimize getWord(0) to VAL; there might be more than one word.
  यदि (LLVM_LIKELY(isSingleWord() || getActiveBits() <= APINT_BITS_PER_WORD)) {
    यदि (isSigned) {
      int64_t sext = SignExtend64(getWord(0), BitWidth);
      प्रत्यागम द्विगुण(sext);
    }
    प्रत्यागम द्विगुण(getWord(0));
  }

  // Determine if the value is negative.
  तार्किक isNeg = isSigned ? (*एतत्)[BitWidth-1] : असत्य;

  // Construct the absolute value if we're negative.
  APInt Tmp(isNeg ? -(*एतत्) : (*एतत्));

  // Figure out how many bits we're using.
  unsigned n = Tmp.getActiveBits();

  // The exponent (without bias normalization) is just the number of bits
  // we are using. Note that the sign bit is gone since we constructed the
  // absolute value.
  uint64_t exp = n;

  // Return infinity for exponent overflow
  यदि (exp > 1023) {
    यदि (!isSigned || !isNeg)
      प्रत्यागम std::numeric_limits<द्विगुण>::infinity();
    अन्यथा
      प्रत्यागम -std::numeric_limits<द्विगुण>::infinity();
  }
  exp += 1023; // Increment for 1023 bias

  // Number of bits in mantissa is 52. To obtain the mantissa value, we must
  // extract the high 52 bits from the correct words in pVal.
  uint64_t mantissa;
  unsigned hiWord = whichWord(n-1);
  यदि (hiWord == 0) {
    mantissa = Tmp.U.pVal[0];
    यदि (n > 52)
      mantissa >>= n - 52; // shift down, we want the top 52 bits.
  } अन्यथा {
    प्रतिज्ञा(hiWord > 0 && "huh?");
    uint64_t hibits = Tmp.U.pVal[hiWord] << (52 - n % APINT_BITS_PER_WORD);
    uint64_t lobits = Tmp.U.pVal[hiWord-1] >> (11 + n % APINT_BITS_PER_WORD);
    mantissa = hibits | lobits;
  }

  // The leading bit of mantissa is implicit, so get rid of it.
  uint64_t sign = isNeg ? (1ULL << (APINT_BITS_PER_WORD - 1)) : 0;
  uint64_t I = sign | (exp << 52) | mantissa;
  प्रत्यागम bit_cast<द्विगुण>(I);
}

// Truncate to new width.
APInt APInt::trunc(unsigned width) ध्रुव {
  प्रतिज्ञा(width <= BitWidth && "Invalid APInt Truncate request");

  यदि (width <= APINT_BITS_PER_WORD)
    प्रत्यागम APInt(width, getRawData()[0], /*isSigned=*/असत्य,
                 /*implicitTrunc=*/सत्य);

  यदि (width == BitWidth)
    प्रत्यागम *एतत्;

  APInt Result(getMemory(get_अंक_शब्दगण(width)), width);

  // Copy full words.
  unsigned i;
  प्रति (i = 0; i != width / APINT_BITS_PER_WORD; i++)
    Result.U.pVal[i] = U.pVal[i];

  // Truncate and copy any partial word.
  unsigned bits = (0 - width) % APINT_BITS_PER_WORD;
  यदि (bits != 0)
    Result.U.pVal[i] = U.pVal[i] << bits >> bits;

  प्रत्यागम Result;
}

// Truncate to new width with unsigned saturation.
APInt APInt::truncUSat(unsigned width) ध्रुव {
  प्रतिज्ञा(width <= BitWidth && "Invalid APInt Truncate request");

  // Can we just losslessly truncate it?
  यदि (isIntN(width))
    प्रत्यागम trunc(width);
  // If not, then just return the new limit.
  प्रत्यागम APInt::get_Max_मूल्य(width);
}

// Truncate to new width with signed saturation to signed result.
APInt APInt::truncSSat(unsigned width) ध्रुव {
  प्रतिज्ञा(width <= BitWidth && "Invalid APInt Truncate request");

  // Can we just losslessly truncate it?
  यदि (isSignedIntN(width))
    प्रत्यागम trunc(width);
  // If not, then just return the new limits.
  प्रत्यागम isNegative() ? APInt::get_Signed_Min_मूल्य(width)
                      : APInt::get_Signed_Max_मूल्य(width);
}

// Truncate to new width with signed saturation to unsigned result.
APInt APInt::truncSSatU(unsigned width) ध्रुव {
  प्रतिज्ञा(width <= BitWidth && "Invalid APInt Truncate request");

  // Can we just losslessly truncate it?
  यदि (isIntN(width))
    प्रत्यागम trunc(width);
  // If not, then just return the new limits.
  प्रत्यागम isNegative() ? APInt::getZero(width) : APInt::get_Max_मूल्य(width);
}

// Sign extend to a new width.
APInt APInt::sext(unsigned Width) ध्रुव {
  प्रतिज्ञा(Width >= BitWidth && "Invalid APInt SignExtend request");

  यदि (Width <= APINT_BITS_PER_WORD)
    प्रत्यागम APInt(Width, SignExtend64(U.VAL, BitWidth), /*isSigned=*/सत्य);

  यदि (Width == BitWidth)
    प्रत्यागम *एतत्;

  APInt Result(getMemory(get_अंक_शब्दगण(Width)), Width);

  // Copy words.
  std::memcpy(Result.U.pVal, getRawData(), get_अंक_शब्दगण() * APINT_WORD_SIZE);

  // Sign extend the last word since there may be unused bits in the input.
  Result.U.pVal[get_अंक_शब्दगण() - 1] =
      SignExtend64(Result.U.pVal[get_अंक_शब्दगण() - 1],
                   ((BitWidth - 1) % APINT_BITS_PER_WORD) + 1);

  // Fill with sign bits.
  std::memset(Result.U.pVal + get_अंक_शब्दगण(), isNegative() ? -1 : 0,
              (Result.get_अंक_शब्दगण() - get_अंक_शब्दगण()) * APINT_WORD_SIZE);
  Result.clearUnusedBits();
  प्रत्यागम Result;
}

//  Zero extend to a new width.
APInt APInt::zext(unsigned width) ध्रुव {
  प्रतिज्ञा(width >= BitWidth && "Invalid APInt ZeroExtend request");

  यदि (width <= APINT_BITS_PER_WORD)
    प्रत्यागम APInt(width, U.VAL);

  यदि (width == BitWidth)
    प्रत्यागम *एतत्;

  APInt Result(getMemory(get_अंक_शब्दगण(width)), width);

  // Copy words.
  std::memcpy(Result.U.pVal, getRawData(), get_अंक_शब्दगण() * APINT_WORD_SIZE);

  // Zero remaining words.
  std::memset(Result.U.pVal + get_अंक_शब्दगण(), 0,
              (Result.get_अंक_शब्दगण() - get_अंक_शब्दगण()) * APINT_WORD_SIZE);

  प्रत्यागम Result;
}

APInt APInt::zextOrTrunc(unsigned width) ध्रुव {
  यदि (BitWidth < width)
    प्रत्यागम zext(width);
  यदि (BitWidth > width)
    प्रत्यागम trunc(width);
  प्रत्यागम *एतत्;
}

APInt APInt::sextOrTrunc(unsigned width) ध्रुव {
  यदि (BitWidth < width)
    प्रत्यागम sext(width);
  यदि (BitWidth > width)
    प्रत्यागम trunc(width);
  प्रत्यागम *एतत्;
}

/// Arithmetic right-shift this APInt by shiftAmt.
/// Arithmetic right-shift function.
शून्य APInt::ashrInPlace(ध्रुव APInt &shiftAmt) {
  ashrInPlace((unsigned)shiftAmt.get_Limited_मूल्य(BitWidth));
}

/// Arithmetic right-shift this APInt by shiftAmt.
/// Arithmetic right-shift function.
शून्य APInt::ashrSlowCase(unsigned ShiftAmt) {
  // Don't bother performing a no-op shift.
  यदि (!ShiftAmt)
    प्रत्यागम;

  // Save the original sign bit for later.
  तार्किक Negative = isNegative();

  // WordShift is the inter-part shift; BitShift is intra-part shift.
  unsigned WordShift = ShiftAmt / APINT_BITS_PER_WORD;
  unsigned BitShift = ShiftAmt % APINT_BITS_PER_WORD;

  unsigned शब्दगण_To_Move = get_अंक_शब्दगण() - WordShift;
  यदि (शब्दगण_To_Move != 0) {
    // Sign extend the last word to fill in the unused bits.
    U.pVal[get_अंक_शब्दगण() - 1] = SignExtend64(
        U.pVal[get_अंक_शब्दगण() - 1], ((BitWidth - 1) % APINT_BITS_PER_WORD) + 1);

    // Fastpath for moving by whole words.
    यदि (BitShift == 0) {
      std::memmove(U.pVal, U.pVal + WordShift, शब्दगण_To_Move * APINT_WORD_SIZE);
    } अन्यथा {
      // Move the words containing significant bits.
      प्रति (unsigned i = 0; i != शब्दगण_To_Move - 1; ++i)
        U.pVal[i] = (U.pVal[i + WordShift] >> BitShift) |
                    (U.pVal[i + WordShift + 1] << (APINT_BITS_PER_WORD - BitShift));

      // Handle the last word which has no high bits to copy. Use an arithmetic
      // shift to preserve the sign bit.
      U.pVal[शब्दगण_To_Move - 1] =
          (int64_t)U.pVal[WordShift + शब्दगण_To_Move - 1] >> BitShift;
    }
  }

  // Fill in the remainder based on the original sign.
  std::memset(U.pVal + शब्दगण_To_Move, Negative ? -1 : 0,
              WordShift * APINT_WORD_SIZE);
  clearUnusedBits();
}

/// Logical right-shift this APInt by shiftAmt.
/// Logical right-shift function.
शून्य APInt::lshrInPlace(ध्रुव APInt &shiftAmt) {
  lshrInPlace((unsigned)shiftAmt.get_Limited_मूल्य(BitWidth));
}

/// Logical right-shift this APInt by shiftAmt.
/// Logical right-shift function.
शून्य APInt::lshrSlowCase(unsigned ShiftAmt) {
  tc_Shift_दायाँ(U.pVal, get_अंक_शब्दगण(), ShiftAmt);
}

/// Left-shift this APInt by shiftAmt.
/// Left-shift function.
APInt &APInt::operator<<=(ध्रुव APInt &shiftAmt) {
  // It's undefined behavior in C to shift by BitWidth or greater.
  *एतत् <<= (unsigned)shiftAmt.get_Limited_मूल्य(BitWidth);
  प्रत्यागम *एतत्;
}

शून्य APInt::shlSlowCase(unsigned ShiftAmt) {
  tc_Shift_बायाँ(U.pVal, get_अंक_शब्दगण(), ShiftAmt);
  clearUnusedBits();
}

// Calculate the rotate amount modulo the bit width.
स्थिर unsigned rotateModulo(unsigned BitWidth, ध्रुव APInt &rotateAmt) {
  यदि (LLVM_UNLIKELY(BitWidth == 0))
    प्रत्यागम 0;
  unsigned rotBitWidth = rotateAmt.getBitWidth();
  APInt rot = rotateAmt;
  यदि (rotBitWidth < BitWidth) {
    // Extend the rotate APInt, so that the urem doesn't divide by 0.
    // e.g. APInt(1, 32) would give APInt(1, 0).
    rot = rotateAmt.zext(BitWidth);
  }
  rot = rot.urem(APInt(rot.getBitWidth(), BitWidth));
  प्रत्यागम rot.get_Limited_मूल्य(BitWidth);
}

APInt APInt::rotl(ध्रुव APInt &rotateAmt) ध्रुव {
  प्रत्यागम rotl(rotateModulo(BitWidth, rotateAmt));
}

APInt APInt::rotl(unsigned rotateAmt) ध्रुव {
  यदि (LLVM_UNLIKELY(BitWidth == 0))
    प्रत्यागम *एतत्;
  rotateAmt %= BitWidth;
  यदि (rotateAmt == 0)
    प्रत्यागम *एतत्;
  प्रत्यागम shl(rotateAmt) | lshr(BitWidth - rotateAmt);
}

APInt APInt::rotr(ध्रुव APInt &rotateAmt) ध्रुव {
  प्रत्यागम rotr(rotateModulo(BitWidth, rotateAmt));
}

APInt APInt::rotr(unsigned rotateAmt) ध्रुव {
  यदि (BitWidth == 0)
    प्रत्यागम *एतत्;
  rotateAmt %= BitWidth;
  यदि (rotateAmt == 0)
    प्रत्यागम *एतत्;
  प्रत्यागम lshr(rotateAmt) | shl(BitWidth - rotateAmt);
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
unsigned APInt::nearestLogBase2() ध्रुव {
  // Special case when we have a bitwidth of 1. If VAL is 1, then we
  // get 0. If VAL is 0, we get WORDTYPE_MAX which gets truncated to
  // UINT32_MAX.
  यदि (BitWidth == 1)
    प्रत्यागम U.VAL - 1;

  // Handle the zero case.
  यदि (isZero())
    प्रत्यागम UINT32_MAX;

  // The non-zero case is handled by computing:
  //
  //   nearestLogBase2(x) = logBase2(x) + x[logBase2(x)-1].
  //
  // where x[i] is referring to the value of the ith bit of x.
  unsigned lg = logBase2();
  प्रत्यागम lg + unsigned((*एतत्)[lg - 1]);
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
  unsigned magnitude = getActiveBits();

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
    प्रत्यागम APInt(BitWidth,
                 results[(LLVM_LIKELY(isSingleWord()) ? U.VAL : U.pVal[0])]);
  }

  // If the magnitude of the value fits in less than 52 bits (the precision of
  // an IEEE double precision floating point value), then we can use the
  // libc sqrt function which will probably use a hardware sqrt computation.
  // This should be faster than the algorithm below.
  यदि (magnitude < 52) {
    प्रत्यागम APInt(BitWidth,
                 uint64_t(::floor(::sqrt(द्विगुण(
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
  APInt Factor = *एतत्;
  APInt T;
  यावत् (!(T = *एतत् * Factor).isOne())
    Factor *= 2 - std::move(T);
  प्रत्यागम Factor;
}

/// Implementation of Knuth's Algorithm D (Division of nonnegative integers)
/// from "Art of Computer Programming, Volume 2", section 4.3.1, p. 272. The
/// variables here have the same names as in the algorithm. Comments explain
/// the algorithm and any deviation from it.
स्थिर शून्य KnuthDiv(uint32_t *u, uint32_t *v, uint32_t *q, uint32_t* r,
                     unsigned m, unsigned n) {
  प्रतिज्ञा(u && "Must provide dividend");
  प्रतिज्ञा(v && "Must provide divisor");
  प्रतिज्ञा(q && "Must provide quotient");
  प्रतिज्ञा(u != v && u != q && v != q && "Must use different memory");
  प्रतिज्ञा(n>1 && "n must be > 1");

  // b denotes the base of the number system. In our case b is 2^32.
  ध्रुव uint64_t b = uint64_t(1) << 32;

// The DEBUG macros here tend to be spam in the debug output if you're not
// debugging this code. Disable them unless KNUTH_DEBUG is defined.
#ifdef KNUTH_DEBUG
#define DEBUG_KNUTH(X) LLVM_DEBUG(X)
#अन्यथा
#define DEBUG_KNUTH(X) कुरु {} यावत्(असत्य)
#endif

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
  unsigned shift = llvm::countl_zero(v[n - 1]);
  uint32_t v_carry = 0;
  uint32_t u_carry = 0;
  यदि (shift) {
    प्रति (unsigned i = 0; i < m+n; ++i) {
      uint32_t u_tmp = u[i] >> (32 - shift);
      u[i] = (u[i] << shift) | u_carry;
      u_carry = u_tmp;
    }
    प्रति (unsigned i = 0; i < n; ++i) {
      uint32_t v_tmp = v[i] >> (32 - shift);
      v[i] = (v[i] << shift) | v_carry;
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
    प्रति (unsigned i = 0; i < n; ++i) {
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
      प्रति (unsigned i = 0; i < n; i++) {
        uint32_t limit = std::min(u[j+i],v[i]);
        u[j+i] += v[i] + carry;
        carry = u[j+i] < limit || (carry && u[j+i] == limit);
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
    यदि (shift) {
      uint32_t carry = 0;
      DEBUG_KNUTH(dbgs() << "KnuthDiv: remainder:");
      प्रति (पूर्णाङ्क i = n-1; i >= 0; i--) {
        r[i] = (u[i] >> shift) | carry;
        carry = u[i] << (32 - shift);
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

शून्य APInt::divide(ध्रुव WordType *LHS, unsigned lhs_शब्दगण, ध्रुव WordType *RHS,
                   unsigned rhs_शब्दगण, WordType *Quotient, WordType *Remainder) {
  प्रतिज्ञा(lhs_शब्दगण >= rhs_शब्दगण && "Fractional result");

  // First, compose the values into an array of 32-bit words instead of
  // 64-bit words. This is a necessity of both the "short division" algorithm
  // and the Knuth "classical algorithm" which requires there to be native
  // operations for +, -, and * on an m bit value with an m*2 bit result. We
  // can't use 64-bit operands here because we don't have native results of
  // 128-bits. Furthermore, casting the 64-bit values to 32-bit values won't
  // work on large-endian machines.
  unsigned n = rhs_शब्दगण * 2;
  unsigned m = (lhs_शब्दगण * 2) - n;

  // Allocate space for the temporary values we need either on the stack, if
  // it will fit, or on the heap if it won't.
  uint32_t SPACE[128];
  uint32_t *U = nullptr;
  uint32_t *V = nullptr;
  uint32_t *Q = nullptr;
  uint32_t *R = nullptr;
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
  memset(U, 0, (m+n+1)*परिमाण(uint32_t));
  प्रति (unsigned i = 0; i < lhs_शब्दगण; ++i) {
    uint64_t tmp = LHS[i];
    U[i * 2] = Lo_32(tmp);
    U[i * 2 + 1] = Hi_32(tmp);
  }
  U[m+n] = 0; // this extra word is for "spill" in the Knuth algorithm.

  // Initialize the divisor
  memset(V, 0, (n)*परिमाण(uint32_t));
  प्रति (unsigned i = 0; i < rhs_शब्दगण; ++i) {
    uint64_t tmp = RHS[i];
    V[i * 2] = Lo_32(tmp);
    V[i * 2 + 1] = Hi_32(tmp);
  }

  // initialize the quotient and remainder
  memset(Q, 0, (m+n) * परिमाण(uint32_t));
  यदि (Remainder)
    memset(R, 0, n * परिमाण(uint32_t));

  // Now, adjust m and n for the Knuth division. n is the number of words in
  // the divisor. m is the number of words by which the dividend exceeds the
  // divisor (i.e. m+n is the length of the dividend). These sizes must not
  // contain any zero words or the Knuth algorithm fails.
  प्रति (unsigned i = n; i > 0 && V[i-1] == 0; i--) {
    n--;
    m++;
  }
  प्रति (unsigned i = m+n; i > 0 && U[i-1] == 0; i--)
    m--;

  // If we're left with only a single word for the divisor, Knuth doesn't work
  // so we implement the short division algorithm here. This is much simpler
  // and faster because we are certain that we can divide a 64-bit quantity
  // by a 32-bit quantity at hardware speed and short division is simply a
  // series of such operations. This is just like doing short division but we
  // are using base 2^32 instead of base 10.
  प्रतिज्ञा(n != 0 && "Divide by zero?");
  यदि (n == 1) {
    uint32_t भाजक = V[0];
    uint32_t remainder = 0;
    प्रति (पूर्णाङ्क i = m; i >= 0; i--) {
      uint64_t partial_dividend = Make_64(remainder, U[i]);
      यदि (partial_dividend == 0) {
        Q[i] = 0;
        remainder = 0;
      } अन्यथा यदि (partial_dividend < भाजक) {
        Q[i] = 0;
        remainder = Lo_32(partial_dividend);
      } अन्यथा यदि (partial_dividend == भाजक) {
        Q[i] = 1;
        remainder = 0;
      } अन्यथा {
        Q[i] = Lo_32(partial_dividend / भाजक);
        remainder = Lo_32(partial_dividend - (Q[i] * भाजक));
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
    प्रति (unsigned i = 0; i < lhs_शब्दगण; ++i)
      Quotient[i] = Make_64(Q[i*2+1], Q[i*2]);
  }

  // If the caller wants the remainder
  यदि (Remainder) {
    प्रति (unsigned i = 0; i < rhs_शब्दगण; ++i)
      Remainder[i] = Make_64(R[i*2+1], R[i*2]);
  }

  // Clean up the memory we allocated.
  यदि (U != &SPACE[0]) {
    delete [] U;
    delete [] V;
    delete [] Q;
    delete [] R;
  }
}

APInt APInt::udiv(ध्रुव APInt &RHS) ध्रुव {
  प्रतिज्ञा(BitWidth == RHS.BitWidth && "Bit widths must be the same");

  // First, deal with the easy case
  यदि (LLVM_LIKELY(isSingleWord())) {
    प्रतिज्ञा(RHS.U.VAL != 0 && "Divide by zero?");
    प्रत्यागम APInt(BitWidth, U.VAL / RHS.U.VAL);
  }

  // Get some facts about the LHS and RHS number of bits and words
  unsigned lhs_शब्दगण = get_अंक_शब्दगण(getActiveBits());
  unsigned rhsBits  = RHS.getActiveBits();
  unsigned rhs_शब्दगण = get_अंक_शब्दगण(rhsBits);
  प्रतिज्ञा(rhs_शब्दगण && "Divided by zero???");

  // Deal with some degenerate cases
  यदि (!lhs_शब्दगण)
    // 0 / X ===> 0
    प्रत्यागम APInt(BitWidth, 0);
  यदि (rhsBits == 1)
    // X / 1 ===> X
    प्रत्यागम *एतत्;
  यदि (lhs_शब्दगण < rhs_शब्दगण || एतत्->ult(RHS))
    // X / Y ===> 0, iff X < Y
    प्रत्यागम APInt(BitWidth, 0);
  यदि (*एतत् == RHS)
    // X / X ===> 1
    प्रत्यागम APInt(BitWidth, 1);
  यदि (lhs_शब्दगण == 1) // rhsWords is 1 if lhsWords is 1.
    // All high words are zero, just use native divide
    प्रत्यागम APInt(BitWidth, एतत्->U.pVal[0] / RHS.U.pVal[0]);

  // We have to compute it the hard way. Invoke the Knuth divide algorithm.
  APInt Quotient(BitWidth, 0); // to hold result.
  divide(U.pVal, lhs_शब्दगण, RHS.U.pVal, rhs_शब्दगण, Quotient.U.pVal, nullptr);
  प्रत्यागम Quotient;
}

APInt APInt::udiv(uint64_t RHS) ध्रुव {
  प्रतिज्ञा(RHS != 0 && "Divide by zero?");

  // First, deal with the easy case
  यदि (LLVM_LIKELY(isSingleWord()))
    प्रत्यागम APInt(BitWidth, U.VAL / RHS);

  // Get some facts about the LHS words.
  unsigned lhs_शब्दगण = get_अंक_शब्दगण(getActiveBits());

  // Deal with some degenerate cases
  यदि (!lhs_शब्दगण)
    // 0 / X ===> 0
    प्रत्यागम APInt(BitWidth, 0);
  यदि (RHS == 1)
    // X / 1 ===> X
    प्रत्यागम *एतत्;
  यदि (एतत्->ult(RHS))
    // X / Y ===> 0, iff X < Y
    प्रत्यागम APInt(BitWidth, 0);
  यदि (*एतत् == RHS)
    // X / X ===> 1
    प्रत्यागम APInt(BitWidth, 1);
  यदि (lhs_शब्दगण == 1) // rhsWords is 1 if lhsWords is 1.
    // All high words are zero, just use native divide
    प्रत्यागम APInt(BitWidth, एतत्->U.pVal[0] / RHS);

  // We have to compute it the hard way. Invoke the Knuth divide algorithm.
  APInt Quotient(BitWidth, 0); // to hold result.
  divide(U.pVal, lhs_शब्दगण, &RHS, 1, Quotient.U.pVal, nullptr);
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
  प्रतिज्ञा(BitWidth == RHS.BitWidth && "Bit widths must be the same");
  यदि (LLVM_LIKELY(isSingleWord())) {
    प्रतिज्ञा(RHS.U.VAL != 0 && "Remainder by zero?");
    प्रत्यागम APInt(BitWidth, U.VAL % RHS.U.VAL);
  }

  // Get some facts about the LHS
  unsigned lhs_शब्दगण = get_अंक_शब्दगण(getActiveBits());

  // Get some facts about the RHS
  unsigned rhsBits = RHS.getActiveBits();
  unsigned rhs_शब्दगण = get_अंक_शब्दगण(rhsBits);
  प्रतिज्ञा(rhs_शब्दगण && "Performing remainder operation by zero ???");

  // Check the degenerate cases
  यदि (lhs_शब्दगण == 0)
    // 0 % Y ===> 0
    प्रत्यागम APInt(BitWidth, 0);
  यदि (rhsBits == 1)
    // X % 1 ===> 0
    प्रत्यागम APInt(BitWidth, 0);
  यदि (lhs_शब्दगण < rhs_शब्दगण || एतत्->ult(RHS))
    // X % Y ===> X, iff X < Y
    प्रत्यागम *एतत्;
  यदि (*एतत् == RHS)
    // X % X == 0;
    प्रत्यागम APInt(BitWidth, 0);
  यदि (lhs_शब्दगण == 1)
    // All high words are zero, just use native remainder
    प्रत्यागम APInt(BitWidth, U.pVal[0] % RHS.U.pVal[0]);
  यदि (RHS.isPowerOf2()) {
    // X % 2^w ===> X & (2^w - 1)
    APInt Result(*एतत्);
    Result.clearBits(RHS.logBase2(), BitWidth);
    प्रत्यागम Result;
  }

  // We have to compute it the hard way. Invoke the Knuth divide algorithm.
  APInt Remainder(BitWidth, 0);
  divide(U.pVal, lhs_शब्दगण, RHS.U.pVal, rhs_शब्दगण, nullptr, Remainder.U.pVal);
  प्रत्यागम Remainder;
}

uint64_t APInt::urem(uint64_t RHS) ध्रुव {
  प्रतिज्ञा(RHS != 0 && "Remainder by zero?");

  यदि (LLVM_LIKELY(isSingleWord()))
    प्रत्यागम U.VAL % RHS;

  // Get some facts about the LHS
  unsigned lhs_शब्दगण = get_अंक_शब्दगण(getActiveBits());

  // Check the degenerate cases
  यदि (lhs_शब्दगण == 0)
    // 0 % Y ===> 0
    प्रत्यागम 0;
  यदि (RHS == 1)
    // X % 1 ===> 0
    प्रत्यागम 0;
  यदि (एतत्->ult(RHS))
    // X % Y ===> X, iff X < Y
    प्रत्यागम get_ZExt_मूल्य();
  यदि (*एतत् == RHS)
    // X % X == 0;
    प्रत्यागम 0;
  यदि (lhs_शब्दगण == 1)
    // All high words are zero, just use native remainder
    प्रत्यागम U.pVal[0] % RHS;
  यदि (llvm::isPowerOf2_64(RHS))
    // X % 2^w ===> X & (2^w - 1)
    प्रत्यागम U.pVal[0] & (RHS - 1);

  // We have to compute it the hard way. Invoke the Knuth divide algorithm.
  uint64_t Remainder;
  divide(U.pVal, lhs_शब्दगण, &RHS, 1, nullptr, &Remainder);
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
  प्रतिज्ञा(LHS.BitWidth == RHS.BitWidth && "Bit widths must be the same");
  unsigned BitWidth = LHS.BitWidth;

  // First, deal with the easy case
  यदि (LLVM_LIKELY(LHS.isSingleWord())) {
    प्रतिज्ञा(RHS.U.VAL != 0 && "Divide by zero?");
    uint64_t QuotVal = LHS.U.VAL / RHS.U.VAL;
    uint64_t RemVal = LHS.U.VAL % RHS.U.VAL;
    Quotient = APInt(BitWidth, QuotVal);
    Remainder = APInt(BitWidth, RemVal);
    प्रत्यागम;
  }

  // Get some size facts about the dividend and divisor
  unsigned lhs_शब्दगण = get_अंक_शब्दगण(LHS.getActiveBits());
  unsigned rhsBits  = RHS.getActiveBits();
  unsigned rhs_शब्दगण = get_अंक_शब्दगण(rhsBits);
  प्रतिज्ञा(rhs_शब्दगण && "Performing divrem operation by zero ???");

  // Check the degenerate cases
  यदि (lhs_शब्दगण == 0) {
    Quotient = APInt(BitWidth, 0);    // 0 / Y ===> 0
    Remainder = APInt(BitWidth, 0);   // 0 % Y ===> 0
    प्रत्यागम;
  }

  यदि (rhsBits == 1) {
    Quotient = LHS;                   // X / 1 ===> X
    Remainder = APInt(BitWidth, 0);   // X % 1 ===> 0
  }

  यदि (lhs_शब्दगण < rhs_शब्दगण || LHS.ult(RHS)) {
    Remainder = LHS;                  // X % Y ===> X, iff X < Y
    Quotient = APInt(BitWidth, 0);    // X / Y ===> 0, iff X < Y
    प्रत्यागम;
  }

  यदि (LHS == RHS) {
    Quotient  = APInt(BitWidth, 1);   // X / X ===> 1
    Remainder = APInt(BitWidth, 0);   // X % X ===> 0;
    प्रत्यागम;
  }

  // Make sure there is enough space to hold the results.
  // NOTE: This assumes that reallocate won't affect any bits if it doesn't
  // change the size. This is necessary if Quotient or Remainder is aliased
  // with LHS or RHS.
  Quotient.reallocate(BitWidth);
  Remainder.reallocate(BitWidth);

  यदि (lhs_शब्दगण == 1) { // rhsWords is 1 if lhsWords is 1.
    // There is only one word to consider so use the native versions.
    uint64_t lhs_मूल्य = LHS.U.pVal[0];
    uint64_t rhs_मूल्य = RHS.U.pVal[0];
    Quotient = lhs_मूल्य / rhs_मूल्य;
    Remainder = lhs_मूल्य % rhs_मूल्य;
    प्रत्यागम;
  }

  // Okay, lets do it the long way
  divide(LHS.U.pVal, lhs_शब्दगण, RHS.U.pVal, rhs_शब्दगण, Quotient.U.pVal,
         Remainder.U.pVal);
  // Clear the rest of the Quotient and Remainder.
  std::memset(Quotient.U.pVal + lhs_शब्दगण, 0,
              (get_अंक_शब्दगण(BitWidth) - lhs_शब्दगण) * APINT_WORD_SIZE);
  std::memset(Remainder.U.pVal + rhs_शब्दगण, 0,
              (get_अंक_शब्दगण(BitWidth) - rhs_शब्दगण) * APINT_WORD_SIZE);
}

शून्य APInt::udivrem(ध्रुव APInt &LHS, uint64_t RHS, APInt &Quotient,
                    uint64_t &Remainder) {
  प्रतिज्ञा(RHS != 0 && "Divide by zero?");
  unsigned BitWidth = LHS.BitWidth;

  // First, deal with the easy case
  यदि (LLVM_LIKELY(LHS.isSingleWord())) {
    uint64_t QuotVal = LHS.U.VAL / RHS;
    Remainder = LHS.U.VAL % RHS;
    Quotient = APInt(BitWidth, QuotVal);
    प्रत्यागम;
  }

  // Get some size facts about the dividend and divisor
  unsigned lhs_शब्दगण = get_अंक_शब्दगण(LHS.getActiveBits());

  // Check the degenerate cases
  यदि (lhs_शब्दगण == 0) {
    Quotient = APInt(BitWidth, 0);    // 0 / Y ===> 0
    Remainder = 0;                    // 0 % Y ===> 0
    प्रत्यागम;
  }

  यदि (RHS == 1) {
    Quotient = LHS;                   // X / 1 ===> X
    Remainder = 0;                    // X % 1 ===> 0
    प्रत्यागम;
  }

  यदि (LHS.ult(RHS)) {
    Remainder = LHS.get_ZExt_मूल्य();   // X % Y ===> X, iff X < Y
    Quotient = APInt(BitWidth, 0);    // X / Y ===> 0, iff X < Y
    प्रत्यागम;
  }

  यदि (LHS == RHS) {
    Quotient  = APInt(BitWidth, 1);   // X / X ===> 1
    Remainder = 0;                    // X % X ===> 0;
    प्रत्यागम;
  }

  // Make sure there is enough space to hold the results.
  // NOTE: This assumes that reallocate won't affect any bits if it doesn't
  // change the size. This is necessary if Quotient is aliased with LHS.
  Quotient.reallocate(BitWidth);

  यदि (lhs_शब्दगण == 1) { // rhsWords is 1 if lhsWords is 1.
    // There is only one word to consider so use the native versions.
    uint64_t lhs_मूल्य = LHS.U.pVal[0];
    Quotient = lhs_मूल्य / RHS;
    Remainder = lhs_मूल्य % RHS;
    प्रत्यागम;
  }

  // Okay, lets do it the long way
  divide(LHS.U.pVal, lhs_शब्दगण, &RHS, 1, Quotient.U.pVal, &Remainder);
  // Clear the rest of the Quotient.
  std::memset(Quotient.U.pVal + lhs_शब्दगण, 0,
              (get_अंक_शब्दगण(BitWidth) - lhs_शब्दगण) * APINT_WORD_SIZE);
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

APInt APInt::sadd_ov(ध्रुव APInt &RHS, तार्किक &Overflow) ध्रुव {
  APInt Res = *एतत्+RHS;
  Overflow = isNonNegative() == RHS.isNonNegative() &&
             Res.isNonNegative() != isNonNegative();
  प्रत्यागम Res;
}

APInt APInt::uadd_ov(ध्रुव APInt &RHS, तार्किक &Overflow) ध्रुव {
  APInt Res = *एतत्+RHS;
  Overflow = Res.ult(RHS);
  प्रत्यागम Res;
}

APInt APInt::ssub_ov(ध्रुव APInt &RHS, तार्किक &Overflow) ध्रुव {
  APInt Res = *एतत् - RHS;
  Overflow = isNonNegative() != RHS.isNonNegative() &&
             Res.isNonNegative() != isNonNegative();
  प्रत्यागम Res;
}

APInt APInt::usub_ov(ध्रुव APInt &RHS, तार्किक &Overflow) ध्रुव {
  APInt Res = *एतत्-RHS;
  Overflow = Res.ugt(*एतत्);
  प्रत्यागम Res;
}

APInt APInt::sdiv_ov(ध्रुव APInt &RHS, तार्किक &Overflow) ध्रुव {
  // MININT/-1  -->  overflow.
  Overflow = is_Min_Signed_मूल्य() && RHS.isAllOnes();
  प्रत्यागम sdiv(RHS);
}

APInt APInt::smul_ov(ध्रुव APInt &RHS, तार्किक &Overflow) ध्रुव {
  APInt Res = *एतत् * RHS;

  यदि (RHS != 0)
    Overflow = Res.sdiv(RHS) != *एतत् ||
               (is_Min_Signed_मूल्य() && RHS.isAllOnes());
  अन्यथा
    Overflow = असत्य;
  प्रत्यागम Res;
}

APInt APInt::umul_ov(ध्रुव APInt &RHS, तार्किक &Overflow) ध्रुव {
  यदि (countl_zero() + RHS.countl_zero() + 2 <= BitWidth) {
    Overflow = सत्य;
    प्रत्यागम *एतत् * RHS;
  }

  APInt Res = lshr(1) * RHS;
  Overflow = Res.isNegative();
  Res <<= 1;
  यदि ((*एतत्)[0]) {
    Res += RHS;
    यदि (Res.ult(RHS))
      Overflow = सत्य;
  }
  प्रत्यागम Res;
}

APInt APInt::sshl_ov(ध्रुव APInt &ShAmt, तार्किक &Overflow) ध्रुव {
  प्रत्यागम sshl_ov(ShAmt.get_Limited_मूल्य(getBitWidth()), Overflow);
}

APInt APInt::sshl_ov(unsigned ShAmt, तार्किक &Overflow) ध्रुव {
  Overflow = ShAmt >= getBitWidth();
  यदि (Overflow)
    प्रत्यागम APInt(BitWidth, 0);

  यदि (isNonNegative()) // Don't allow sign change.
    Overflow = ShAmt >= countl_zero();
  अन्यथा
    Overflow = ShAmt >= countl_one();

  प्रत्यागम *एतत् << ShAmt;
}

APInt APInt::ushl_ov(ध्रुव APInt &ShAmt, तार्किक &Overflow) ध्रुव {
  प्रत्यागम ushl_ov(ShAmt.get_Limited_मूल्य(getBitWidth()), Overflow);
}

APInt APInt::ushl_ov(unsigned ShAmt, तार्किक &Overflow) ध्रुव {
  Overflow = ShAmt >= getBitWidth();
  यदि (Overflow)
    प्रत्यागम APInt(BitWidth, 0);

  Overflow = ShAmt > countl_zero();

  प्रत्यागम *एतत् << ShAmt;
}

APInt APInt::sfloordiv_ov(ध्रुव APInt &RHS, तार्किक &Overflow) ध्रुव {
  APInt quotient = sdiv_ov(RHS, Overflow);
  यदि ((quotient * RHS != *एतत्) && (isNegative() != RHS.isNegative()))
    प्रत्यागम quotient - 1;
  प्रत्यागम quotient;
}

APInt APInt::sadd_sat(ध्रुव APInt &RHS) ध्रुव {
  तार्किक Overflow;
  APInt Res = sadd_ov(RHS, Overflow);
  यदि (!Overflow)
    प्रत्यागम Res;

  प्रत्यागम isNegative() ? APInt::get_Signed_Min_मूल्य(BitWidth)
                      : APInt::get_Signed_Max_मूल्य(BitWidth);
}

APInt APInt::uadd_sat(ध्रुव APInt &RHS) ध्रुव {
  तार्किक Overflow;
  APInt Res = uadd_ov(RHS, Overflow);
  यदि (!Overflow)
    प्रत्यागम Res;

  प्रत्यागम APInt::get_Max_मूल्य(BitWidth);
}

APInt APInt::ssub_sat(ध्रुव APInt &RHS) ध्रुव {
  तार्किक Overflow;
  APInt Res = ssub_ov(RHS, Overflow);
  यदि (!Overflow)
    प्रत्यागम Res;

  प्रत्यागम isNegative() ? APInt::get_Signed_Min_मूल्य(BitWidth)
                      : APInt::get_Signed_Max_मूल्य(BitWidth);
}

APInt APInt::usub_sat(ध्रुव APInt &RHS) ध्रुव {
  तार्किक Overflow;
  APInt Res = usub_ov(RHS, Overflow);
  यदि (!Overflow)
    प्रत्यागम Res;

  प्रत्यागम APInt(BitWidth, 0);
}

APInt APInt::smul_sat(ध्रुव APInt &RHS) ध्रुव {
  तार्किक Overflow;
  APInt Res = smul_ov(RHS, Overflow);
  यदि (!Overflow)
    प्रत्यागम Res;

  // The result is negative if one and only one of inputs is negative.
  तार्किक ResIsNegative = isNegative() ^ RHS.isNegative();

  प्रत्यागम ResIsNegative ? APInt::get_Signed_Min_मूल्य(BitWidth)
                       : APInt::get_Signed_Max_मूल्य(BitWidth);
}

APInt APInt::umul_sat(ध्रुव APInt &RHS) ध्रुव {
  तार्किक Overflow;
  APInt Res = umul_ov(RHS, Overflow);
  यदि (!Overflow)
    प्रत्यागम Res;

  प्रत्यागम APInt::get_Max_मूल्य(BitWidth);
}

APInt APInt::sshl_sat(ध्रुव APInt &RHS) ध्रुव {
  प्रत्यागम sshl_sat(RHS.get_Limited_मूल्य(getBitWidth()));
}

APInt APInt::sshl_sat(unsigned RHS) ध्रुव {
  तार्किक Overflow;
  APInt Res = sshl_ov(RHS, Overflow);
  यदि (!Overflow)
    प्रत्यागम Res;

  प्रत्यागम isNegative() ? APInt::get_Signed_Min_मूल्य(BitWidth)
                      : APInt::get_Signed_Max_मूल्य(BitWidth);
}

APInt APInt::ushl_sat(ध्रुव APInt &RHS) ध्रुव {
  प्रत्यागम ushl_sat(RHS.get_Limited_मूल्य(getBitWidth()));
}

APInt APInt::ushl_sat(unsigned RHS) ध्रुव {
  तार्किक Overflow;
  APInt Res = ushl_ov(RHS, Overflow);
  यदि (!Overflow)
    प्रत्यागम Res;

  प्रत्यागम APInt::get_Max_मूल्य(BitWidth);
}

शून्य APInt::fromString(unsigned numbits, StringRef str, uint8_t radix) {
  // Check our assumptions here
  प्रतिज्ञा(!str.empty() && "Invalid string length");
  प्रतिज्ञा((radix == 10 || radix == 8 || radix == 16 || radix == 2 ||
          radix == 36) &&
         "Radix should be 2, 8, 10, 16, or 36!");

  StringRef::iterator p = str.begin();
  size_t slen = str.size();
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
    U.pVal = getClearedMemory(get_अंक_शब्दगण());

  // Figure out if we can shift instead of multiply
  unsigned shift = (radix == 16 ? 4 : radix == 8 ? 3 : radix == 2 ? 1 : 0);

  // Enter digit traversal loop
  प्रति (StringRef::iterator e = str.end(); p != e; ++p) {
    unsigned digit = getDigit(*p, radix);
    प्रतिज्ञा(digit < radix && "Invalid character in digit string");

    // Shift or multiply the value by the radix
    यदि (slen > 1) {
      यदि (shift)
        *एतत् <<= shift;
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

शून्य APInt::toString(SmallVectorImpl<वर्ण> &Str, unsigned Radix, तार्किक Signed,
                     तार्किक formatAsCLiteral, तार्किक UpperCase,
                     तार्किक जोड़ो_Separators) ध्रुव {
  प्रतिज्ञा((Radix == 10 || Radix == 8 || Radix == 16 || Radix == 2 ||
          Radix == 36) &&
         "Radix should be 2, 8, 10, 16, or 36!");

  ध्रुव वर्ण *Prefix = "";
  यदि (formatAsCLiteral) {
    विकल्प (Radix) {
      पक्ष 2:
        // Binary literals are a non-standard extension added in gcc 4.3:
        // http://gcc.gnu.org/onlinedocs/gcc-4.3.0/gcc/Binary-constants.html
        Prefix = "0b";
        भङ्ग;
      पक्ष 8:
        Prefix = "0";
        भङ्ग;
      पक्ष 10:
        भङ्ग; // No prefix
      पक्ष 16:
        Prefix = "0x";
        भङ्ग;
      स्वभाव:
        llvm_unreachable("Invalid radix!");
    }
  }

  // Number of digits in a group between separators.
  unsigned Grouping = (Radix == 8 || Radix == 10) ? 3 : 4;

  // First, check for a zero value and just short circuit the logic below.
  यदि (isZero()) {
    यावत् (*Prefix) {
      Str.धकेलो_back(*Prefix);
      ++Prefix;
    };
    Str.धकेलो_back('0');
    प्रत्यागम;
  }

  स्थिर ध्रुव वर्ण BothDigits[] = "0123456789abcdefghijklmnopqrstuvwxyz"
                                   "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ";
  ध्रुव वर्ण *Digits = BothDigits + (UpperCase ? 36 : 0);

  यदि (LLVM_LIKELY(isSingleWord())) {
    वर्ण Buffer[65];
    वर्ण *BufPtr = std::end(Buffer);

    uint64_t N;
    यदि (!Signed) {
      N = get_ZExt_मूल्य();
    } अन्यथा {
      int64_t I = get_SExt_मूल्य();
      यदि (I >= 0) {
        N = I;
      } अन्यथा {
        Str.धकेलो_back('-');
        N = -(uint64_t)I;
      }
    }

    यावत् (*Prefix) {
      Str.धकेलो_back(*Prefix);
      ++Prefix;
    };

    पूर्णाङ्क Pos = 0;
    यावत् (N) {
      यदि (जोड़ो_Separators && Pos % Grouping == 0 && Pos > 0)
        *--BufPtr = '\'';
      *--BufPtr = Digits[N % Radix];
      N /= Radix;
      Pos++;
    }
    Str.append(BufPtr, std::end(Buffer));
    प्रत्यागम;
  }

  APInt Tmp(*एतत्);

  यदि (Signed && isNegative()) {
    // They want to print the signed version and it is a negative value
    // Flip the bits and add one to turn it into the equivalent positive
    // value and put a '-' in the result.
    Tmp.negate();
    Str.धकेलो_back('-');
  }

  यावत् (*Prefix) {
    Str.धकेलो_back(*Prefix);
    ++Prefix;
  }

  // We insert the digits backward, then reverse them to get the right order.
  unsigned StartDig = Str.size();

  // For the 2, 8 and 16 bit cases, we can just shift instead of divide
  // because the number of bits per digit (1, 3 and 4 respectively) divides
  // equally.  We just shift until the value is zero.
  यदि (Radix == 2 || Radix == 8 || Radix == 16) {
    // Just shift tmp right for each digit width until it becomes zero
    unsigned ShiftAmt = (Radix == 16 ? 4 : (Radix == 8 ? 3 : 1));
    unsigned MaskAmt = Radix - 1;

    पूर्णाङ्क Pos = 0;
    यावत् (Tmp.get_Bool_मूल्य()) {
      unsigned Digit = unsigned(Tmp.getRawData()[0]) & MaskAmt;
      यदि (जोड़ो_Separators && Pos % Grouping == 0 && Pos > 0)
        Str.धकेलो_back('\'');

      Str.धकेलो_back(Digits[Digit]);
      Tmp.lshrInPlace(ShiftAmt);
      Pos++;
    }
  } अन्यथा {
    पूर्णाङ्क Pos = 0;
    यावत् (Tmp.get_Bool_मूल्य()) {
      uint64_t Digit;
      udivrem(Tmp, Radix, Tmp, Digit);
      प्रतिज्ञा(Digit < Radix && "divide failed");
      यदि (जोड़ो_Separators && Pos % Grouping == 0 && Pos > 0)
        Str.धकेलो_back('\'');

      Str.धकेलो_back(Digits[Digit]);
      Pos++;
    }
  }

  // Reverse the digits before returning.
  std::reverse(Str.begin()+StartDig, Str.end());
}

#यदि !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)
LLVM_DUMP_METHOD शून्य APInt::dump() ध्रुव {
  SmallString<40> S, U;
  एतत्->toStringUnsigned(U);
  एतत्->toStringSigned(S);
  dbgs() << "APInt(" << BitWidth << "b, "
         << U << "u " << S << "s)\n";
}
#endif

शून्य APInt::print(raw_ostream &OS, तार्किक isSigned) ध्रुव {
  SmallString<40> S;
  एतत्->toString(S, 10, isSigned, /* formatAsCLiteral = */असत्य);
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
स्थिर अन्तःस्थ APInt::WordType निचला_Bit_Mask(unsigned bits) {
  प्रतिज्ञा(bits != 0 && bits <= APInt::APINT_BITS_PER_WORD);
  प्रत्यागम ~(APInt::WordType) 0 >> (APInt::APINT_BITS_PER_WORD - bits);
}

/// Returns the value of the lower half of PART.
स्थिर अन्तःस्थ APInt::WordType निचला_Half(APInt::WordType part) {
  प्रत्यागम part & निचला_Bit_Mask(APInt::APINT_BITS_PER_WORD / 2);
}

/// Returns the value of the upper half of PART.
स्थिर अन्तःस्थ APInt::WordType ऊपरी_Half(APInt::WordType part) {
  प्रत्यागम part >> (APInt::APINT_BITS_PER_WORD / 2);
}

/// Sets the least significant part of a bignum to the input value, and zeroes
/// out higher parts.
शून्य APInt::tcSet(WordType *dst, WordType part, unsigned parts) {
  प्रतिज्ञा(parts > 0);
  dst[0] = part;
  प्रति (unsigned i = 1; i < parts; i++)
    dst[i] = 0;
}

/// Assign one bignum to another.
शून्य APInt::tcAssign(WordType *dst, ध्रुव WordType *src, unsigned parts) {
  प्रति (unsigned i = 0; i < parts; i++)
    dst[i] = src[i];
}

/// Returns true if a bignum is zero, false otherwise.
तार्किक APInt::tcIsZero(ध्रुव WordType *src, unsigned parts) {
  प्रति (unsigned i = 0; i < parts; i++)
    यदि (src[i])
      प्रत्यागम असत्य;

  प्रत्यागम सत्य;
}

/// Extract the given bit of a bignum; returns 0 or 1.
पूर्णाङ्क APInt::tcExtractBit(ध्रुव WordType *parts, unsigned bit) {
  प्रत्यागम (parts[whichWord(bit)] & maskBit(bit)) != 0;
}

/// Set the given bit of a bignum.
शून्य APInt::tcSetBit(WordType *parts, unsigned bit) {
  parts[whichWord(bit)] |= maskBit(bit);
}

/// Clears the given bit of a bignum.
शून्य APInt::tcClearBit(WordType *parts, unsigned bit) {
  parts[whichWord(bit)] &= ~maskBit(bit);
}

/// Returns the bit number of the least significant set bit of a number.  If the
/// input number has no bits set UINT_MAX is returned.
unsigned APInt::tcLSB(ध्रुव WordType *parts, unsigned n) {
  प्रति (unsigned i = 0; i < n; i++) {
    यदि (parts[i] != 0) {
      unsigned lsb = llvm::countr_zero(parts[i]);
      प्रत्यागम lsb + i * APINT_BITS_PER_WORD;
    }
  }

  प्रत्यागम UINT_MAX;
}

/// Returns the bit number of the most significant set bit of a number.
/// If the input number has no bits set UINT_MAX is returned.
unsigned APInt::tcMSB(ध्रुव WordType *parts, unsigned n) {
  कुरु {
    --n;

    यदि (parts[n] != 0) {
      static_assert(परिमाण(parts[n]) <= परिमाण(uint64_t));
      unsigned msb = llvm::Log2_64(parts[n]);

      प्रत्यागम msb + n * APINT_BITS_PER_WORD;
    }
  } यावत् (n);

  प्रत्यागम UINT_MAX;
}

/// Copy the bit vector of width srcBITS from SRC, starting at bit srcLSB, to
/// DST, of dstCOUNT parts, such that the bit srcLSB becomes the least
/// significant bit of DST.  All high bits above srcBITS in DST are zero-filled.
/// */
शून्य
APInt::tcExtract(WordType *dst, unsigned dst_गिनती, ध्रुव WordType *src,
                 unsigned srcBits, unsigned srcLSB) {
  unsigned dstParts = (srcBits + APINT_BITS_PER_WORD - 1) / APINT_BITS_PER_WORD;
  प्रतिज्ञा(dstParts <= dst_गिनती);

  unsigned firstSrcPart = srcLSB / APINT_BITS_PER_WORD;
  tcAssign(dst, src + firstSrcPart, dstParts);

  unsigned shift = srcLSB % APINT_BITS_PER_WORD;
  tc_Shift_दायाँ(dst, dstParts, shift);

  // We now have (dstParts * APINT_BITS_PER_WORD - shift) bits from SRC
  // in DST.  If this is less that srcBits, append the rest, else
  // clear the high bits.
  unsigned n = dstParts * APINT_BITS_PER_WORD - shift;
  यदि (n < srcBits) {
    WordType mask = निचला_Bit_Mask (srcBits - n);
    dst[dstParts - 1] |= ((src[firstSrcPart + dstParts] & mask)
                          << n % APINT_BITS_PER_WORD);
  } अन्यथा यदि (n > srcBits) {
    यदि (srcBits % APINT_BITS_PER_WORD)
      dst[dstParts - 1] &= निचला_Bit_Mask (srcBits % APINT_BITS_PER_WORD);
  }

  // Clear high parts.
  यावत् (dstParts < dst_गिनती)
    dst[dstParts++] = 0;
}

//// DST += RHS + C where C is zero or one.  Returns the carry flag.
APInt::WordType APInt::tcAdd(WordType *dst, ध्रुव WordType *rhs,
                             WordType c, unsigned parts) {
  प्रतिज्ञा(c <= 1);

  प्रति (unsigned i = 0; i < parts; i++) {
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
APInt::WordType APInt::tcAddPart(WordType *dst, WordType src,
                                 unsigned parts) {
  प्रति (unsigned i = 0; i < parts; ++i) {
    dst[i] += src;
    यदि (dst[i] >= src)
      प्रत्यागम 0; // No need to carry so exit early.
    src = 1; // Carry one to next digit.
  }

  प्रत्यागम 1;
}

/// DST -= RHS + C where C is zero or one.  Returns the carry flag.
APInt::WordType APInt::tcSubtract(WordType *dst, ध्रुव WordType *rhs,
                                  WordType c, unsigned parts) {
  प्रतिज्ञा(c <= 1);

  प्रति (unsigned i = 0; i < parts; i++) {
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
APInt::WordType APInt::tcSubtractPart(WordType *dst, WordType src,
                                      unsigned parts) {
  प्रति (unsigned i = 0; i < parts; ++i) {
    WordType Dst = dst[i];
    dst[i] -= src;
    यदि (src <= Dst)
      प्रत्यागम 0; // No need to borrow so exit early.
    src = 1; // We have to "borrow 1" from next "word"
  }

  प्रत्यागम 1;
}

/// Negate a bignum in-place.
शून्य APInt::tcNegate(WordType *dst, unsigned parts) {
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
                          unsigned srcParts, unsigned dstParts,
                          तार्किक add) {
  // Otherwise our writes of DST kill our later reads of SRC.
  प्रतिज्ञा(dst <= src || dst >= src + srcParts);
  प्रतिज्ञा(dstParts <= srcParts + 1);

  // N loops; minimum of dstParts and srcParts.
  unsigned n = std::min(dstParts, srcParts);

  प्रति (unsigned i = 0; i < n; i++) {
    // [LOW, HIGH] = MULTIPLIER * SRC[i] + DST[i] + CARRY.
    // This cannot overflow, because:
    //   (n - 1) * (n - 1) + 2 (n - 1) = (n - 1) * (n + 1)
    // which is less than n^2.
    WordType srcPart = src[i];
    WordType निचला, mid, ऊपरी;
    यदि (multiplier == 0 || srcPart == 0) {
      निचला = carry;
      ऊपरी = 0;
    } अन्यथा {
      निचला = निचला_Half(srcPart) * निचला_Half(multiplier);
      ऊपरी = ऊपरी_Half(srcPart) * ऊपरी_Half(multiplier);

      mid = निचला_Half(srcPart) * ऊपरी_Half(multiplier);
      ऊपरी += ऊपरी_Half(mid);
      mid <<= APINT_BITS_PER_WORD / 2;
      यदि (निचला + mid < निचला)
        ऊपरी++;
      निचला += mid;

      mid = ऊपरी_Half(srcPart) * निचला_Half(multiplier);
      ऊपरी += ऊपरी_Half(mid);
      mid <<= APINT_BITS_PER_WORD / 2;
      यदि (निचला + mid < निचला)
        ऊपरी++;
      निचला += mid;

      // Now add carry.
      यदि (निचला + carry < निचला)
        ऊपरी++;
      निचला += carry;
    }

    यदि (add) {
      // And now DST[i], and store the new low part there.
      यदि (निचला + dst[i] < निचला)
        ऊपरी++;
      dst[i] += निचला;
    } अन्यथा {
      dst[i] = निचला;
    }

    carry = ऊपरी;
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
    प्रति (unsigned i = dstParts; i < srcParts; i++)
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
                      ध्रुव WordType *rhs, unsigned parts) {
  प्रतिज्ञा(dst != lhs && dst != rhs);

  पूर्णाङ्क overflow = 0;

  प्रति (unsigned i = 0; i < parts; i++) {
    // Don't accumulate on the first iteration so we don't need to initalize
    // dst to 0.
    overflow |=
        tcMultiplyPart(&dst[i], lhs, rhs[i], 0, parts, parts - i, i != 0);
  }

  प्रत्यागम overflow;
}

/// DST = LHS * RHS, where DST has width the sum of the widths of the
/// operands. No overflow occurs. DST must be disjoint from both operands.
शून्य APInt::tcFullMultiply(WordType *dst, ध्रुव WordType *lhs,
                           ध्रुव WordType *rhs, unsigned lhsParts,
                           unsigned rhsParts) {
  // Put the narrower number on the LHS for less loops below.
  यदि (lhsParts > rhsParts)
    प्रत्यागम tcFullMultiply (dst, rhs, lhs, rhsParts, lhsParts);

  प्रतिज्ञा(dst != lhs && dst != rhs);

  प्रति (unsigned i = 0; i < lhsParts; i++) {
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
                    unsigned parts) {
  प्रतिज्ञा(lhs != remainder && lhs != srhs && remainder != srhs);

  unsigned shift_गिनती = tcMSB(rhs, parts) + 1;
  यदि (shift_गिनती == 0)
    प्रत्यागम सत्य;

  shift_गिनती = parts * APINT_BITS_PER_WORD - shift_गिनती;
  unsigned n = shift_गिनती / APINT_BITS_PER_WORD;
  WordType mask = (WordType) 1 << (shift_गिनती % APINT_BITS_PER_WORD);

  tcAssign(srhs, rhs, parts);
  tc_Shift_बायाँ(srhs, parts, shift_गिनती);
  tcAssign(remainder, lhs, parts);
  tcSet(lhs, 0, parts);

  // Loop, subtracting SRHS if REMAINDER is greater and adding that to the
  // total.
  प्रति (;;) {
    पूर्णाङ्क compare = tcCompare(remainder, srhs, parts);
    यदि (compare >= 0) {
      tcSubtract(remainder, srhs, 0, parts);
      lhs[n] |= mask;
    }

    यदि (shift_गिनती == 0)
      भङ्ग;
    shift_गिनती--;
    tc_Shift_दायाँ(srhs, parts, 1);
    यदि ((mask >>= 1) == 0) {
      mask = (WordType) 1 << (APINT_BITS_PER_WORD - 1);
      n--;
    }
  }

  प्रत्यागम असत्य;
}

/// Shift a bignum left Count bits in-place. Shifted in bits are zero. There are
/// no restrictions on Count.
शून्य APInt::tc_Shift_बायाँ(WordType *Dst, unsigned शब्दगण_2, unsigned गिनती_2) {
  // Don't bother performing a no-op shift.
  यदि (!गिनती_2)
    प्रत्यागम;

  // WordShift is the inter-part shift; BitShift is the intra-part shift.
  unsigned WordShift = std::min(गिनती_2 / APINT_BITS_PER_WORD, शब्दगण_2);
  unsigned BitShift = गिनती_2 % APINT_BITS_PER_WORD;

  // Fastpath for moving by whole words.
  यदि (BitShift == 0) {
    std::memmove(Dst + WordShift, Dst, (शब्दगण_2 - WordShift) * APINT_WORD_SIZE);
  } अन्यथा {
    यावत् (शब्दगण_2-- > WordShift) {
      Dst[शब्दगण_2] = Dst[शब्दगण_2 - WordShift] << BitShift;
      यदि (शब्दगण_2 > WordShift)
        Dst[शब्दगण_2] |=
          Dst[शब्दगण_2 - WordShift - 1] >> (APINT_BITS_PER_WORD - BitShift);
    }
  }

  // Fill in the remainder with 0s.
  std::memset(Dst, 0, WordShift * APINT_WORD_SIZE);
}

/// Shift a bignum right Count bits in-place. Shifted in bits are zero. There
/// are no restrictions on Count.
शून्य APInt::tc_Shift_दायाँ(WordType *Dst, unsigned शब्दगण_2, unsigned गिनती_2) {
  // Don't bother performing a no-op shift.
  यदि (!गिनती_2)
    प्रत्यागम;

  // WordShift is the inter-part shift; BitShift is the intra-part shift.
  unsigned WordShift = std::min(गिनती_2 / APINT_BITS_PER_WORD, शब्दगण_2);
  unsigned BitShift = गिनती_2 % APINT_BITS_PER_WORD;

  unsigned शब्दगण_To_Move = शब्दगण_2 - WordShift;
  // Fastpath for moving by whole words.
  यदि (BitShift == 0) {
    std::memmove(Dst, Dst + WordShift, शब्दगण_To_Move * APINT_WORD_SIZE);
  } अन्यथा {
    प्रति (unsigned i = 0; i != शब्दगण_To_Move; ++i) {
      Dst[i] = Dst[i + WordShift] >> BitShift;
      यदि (i + 1 != शब्दगण_To_Move)
        Dst[i] |= Dst[i + WordShift + 1] << (APINT_BITS_PER_WORD - BitShift);
    }
  }

  // Fill in the remainder with 0s.
  std::memset(Dst + शब्दगण_To_Move, 0, WordShift * APINT_WORD_SIZE);
}

// Comparison (unsigned) of two bignums.
पूर्णाङ्क APInt::tcCompare(ध्रुव WordType *lhs, ध्रुव WordType *rhs,
                     unsigned parts) {
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
  पक्ष APInt::Rounding::DOWN:
  पक्ष APInt::Rounding::TOWARD_ZERO:
    प्रत्यागम A.udiv(B);
  पक्ष APInt::Rounding::UP: {
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
  पक्ष APInt::Rounding::DOWN:
  पक्ष APInt::Rounding::UP: {
    APInt Quo, Rem;
    APInt::sdivrem(A, B, Quo, Rem);
    यदि (Rem.isZero())
      प्रत्यागम Quo;
    // This algorithm deals with arbitrary rounding mode used by sdivrem.
    // We want to check whether the non-integer part of the mathematical value
    // is negative or not. If the non-integer part is negative, we need to round
    // down from Quo; otherwise, if it's positive or 0, we return Quo, as it's
    // already rounded down.
    यदि (RM == APInt::Rounding::DOWN) {
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
                                           unsigned RangeWidth) {
  unsigned CoeffWidth = A.getBitWidth();
  प्रतिज्ञा(CoeffWidth == B.getBitWidth() && CoeffWidth == C.getBitWidth());
  प्रतिज्ञा(RangeWidth <= CoeffWidth &&
         "Value range width should be less than coefficient width");
  प्रतिज्ञा(RangeWidth > 1 && "Value range bit width should be > 1");

  LLVM_DEBUG(dbgs() << __func__ << ": solving " << A << "x^2 + " << B
                    << "x + " << C << ", rw:" << RangeWidth << '\n');

  // Identify 0 as a (non)solution immediately.
  यदि (C.sextOrTrunc(RangeWidth).isZero()) {
    LLVM_DEBUG(dbgs() << __func__ << ": zero solution\n");
    प्रत्यागम APInt(CoeffWidth, 0);
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
  APInt R = APInt::getOneBitSet(CoeffWidth, RangeWidth);
  APInt TwoA = 2 * A;
  APInt SqrB = B * B;
  तार्किक Pick_निचला;

  auto RoundUp = [] (ध्रुव APInt &V, ध्रुव APInt &A) -> APInt {
    प्रतिज्ञा(A.isStrictlyPositive());
    APInt T = V.abs().urem(A);
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
    यदि (C.isStrictlyPositive())
      C -= R;
    // Pick the greater solution.
    Pick_निचला = असत्य;
  } अन्यथा {
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
    यदि (C.sgt(LowkR)) {
      // If LowkR < C, then such a k is guaranteed to exist because
      // LowkR itself is a multiple of R.
      C -= -RoundUp(-C, R);      // C = C - RoundDown(C, R)
      // Pick the smaller solution.
      Pick_निचला = सत्य;
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
      Pick_निचला = असत्य;
    }
  }

  LLVM_DEBUG(dbgs() << __func__ << ": updated coefficients " << A << "x^2 + "
                    << B << "x + " << C << ", rw:" << RangeWidth << '\n');

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
  यदि (Pick_निचला)
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

std::optional<unsigned>
llvm::APIntOps::GetMostSignificantDifferentBit(ध्रुव APInt &A, ध्रुव APInt &B) {
  प्रतिज्ञा(A.getBitWidth() == B.getBitWidth() && "Must have the same bitwidth");
  यदि (A == B)
    प्रत्यागम std::nullopt;
  प्रत्यागम A.getBitWidth() - ((A ^ B).countl_zero() + 1);
}

APInt llvm::APIntOps::ScaleBitMask(ध्रुव APInt &A, unsigned NewBitWidth,
                                   तार्किक MatchAllBits) {
  unsigned OldBitWidth = A.getBitWidth();
  प्रतिज्ञा((((OldBitWidth % NewBitWidth) == 0) ||
          ((NewBitWidth % OldBitWidth) == 0)) &&
         "One size should be a multiple of the other one. "
         "Can't do fractional scaling.");

  // Check for matching bitwidths.
  यदि (OldBitWidth == NewBitWidth)
    प्रत्यागम A;

  APInt NewA = APInt::getZero(NewBitWidth);

  // Check for null input.
  यदि (A.isZero())
    प्रत्यागम NewA;

  यदि (NewBitWidth > OldBitWidth) {
    // Repeat bits.
    unsigned Scale = NewBitWidth / OldBitWidth;
    प्रति (unsigned i = 0; i != OldBitWidth; ++i)
      यदि (A[i])
        NewA.setBits(i * Scale, (i + 1) * Scale);
  } अन्यथा {
    unsigned Scale = OldBitWidth / NewBitWidth;
    प्रति (unsigned i = 0; i != NewBitWidth; ++i) {
      यदि (MatchAllBits) {
        यदि (A.extractBits(Scale, i * Scale).isAllOnes())
          NewA.setBit(i);
      } अन्यथा {
        यदि (!A.extractBits(Scale, i * Scale).isZero())
          NewA.setBit(i);
      }
    }
  }

  प्रत्यागम NewA;
}

/// StoreIntToMemory - Fills the StoreBytes bytes of memory starting from Dst
/// with the integer held in IntVal.
शून्य llvm::StoreIntToMemory(ध्रुव APInt &IntVal, uint8_t *Dst,
                            unsigned StoreBytes) {
  प्रतिज्ञा((IntVal.getBitWidth()+7)/8 >= StoreBytes && "Integer too small!");
  ध्रुव uint8_t *Src = (ध्रुव uint8_t *)IntVal.getRawData();

  यदि (sys::IsLittleEndianHost) {
    // Little-endian host - the source is ordered from LSB to MSB.  Order the
    // destination from LSB to MSB: Do a straight copy.
    memcpy(Dst, Src, StoreBytes);
  } अन्यथा {
    // Big-endian host - the source is an array of 64 bit words ordered from
    // LSW to MSW.  Each word is ordered from MSB to LSB.  Order the destination
    // from MSB to LSB: Reverse the word order, but not the bytes in a word.
    यावत् (StoreBytes > परिमाण(uint64_t)) {
      StoreBytes -= परिमाण(uint64_t);
      // May not be aligned so use memcpy.
      memcpy(Dst + StoreBytes, Src, परिमाण(uint64_t));
      Src += परिमाण(uint64_t);
    }

    memcpy(Dst, Src + परिमाण(uint64_t) - StoreBytes, StoreBytes);
  }
}

/// LoadIntFromMemory - Loads the integer stored in the LoadBytes bytes starting
/// from Src into IntVal, which is assumed to be wide enough and to hold zero.
शून्य llvm::LoadIntFromMemory(APInt &IntVal, ध्रुव uint8_t *Src,
                             unsigned LoadBytes) {
  प्रतिज्ञा((IntVal.getBitWidth()+7)/8 >= LoadBytes && "Integer too small!");
  uint8_t *Dst = reinterpret_cast<uint8_t *>(
                   const_cast<uint64_t *>(IntVal.getRawData()));

  यदि (sys::IsLittleEndianHost)
    // Little-endian host - the destination must be ordered from LSB to MSB.
    // The source is ordered from LSB to MSB: Do a straight copy.
    memcpy(Dst, Src, LoadBytes);
  अन्यथा {
    // Big-endian - the destination is an array of 64 bit words ordered from
    // LSW to MSW.  Each word must be ordered from MSB to LSB.  The source is
    // ordered from MSB to LSB: Reverse the word order, but not the bytes in
    // a word.
    यावत् (LoadBytes > परिमाण(uint64_t)) {
      LoadBytes -= परिमाण(uint64_t);
      // May not be aligned so use memcpy.
      memcpy(Dst, Src + LoadBytes, परिमाण(uint64_t));
      Dst += परिमाण(uint64_t);
    }

    memcpy(Dst + परिमाण(uint64_t) - LoadBytes, Src, LoadBytes);
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
  प्रतिज्ञा(C1.getBitWidth() == C2.getBitWidth() && "Unequal bitwidths");
  unsigned FullWidth = C1.getBitWidth() * 2;
  APInt C1Ext = C1.sext(FullWidth);
  APInt C2Ext = C2.sext(FullWidth);
  प्रत्यागम (C1Ext * C2Ext).extractBits(C1.getBitWidth(), C1.getBitWidth());
}

APInt APIntOps::mulhu(ध्रुव APInt &C1, ध्रुव APInt &C2) {
  प्रतिज्ञा(C1.getBitWidth() == C2.getBitWidth() && "Unequal bitwidths");
  unsigned FullWidth = C1.getBitWidth() * 2;
  APInt C1Ext = C1.zext(FullWidth);
  APInt C2Ext = C2.zext(FullWidth);
  प्रत्यागम (C1Ext * C2Ext).extractBits(C1.getBitWidth(), C1.getBitWidth());
}

APInt APIntOps::mulsExtended(ध्रुव APInt &C1, ध्रुव APInt &C2) {
  प्रतिज्ञा(C1.getBitWidth() == C2.getBitWidth() && "Unequal bitwidths");
  unsigned FullWidth = C1.getBitWidth() * 2;
  APInt C1Ext = C1.sext(FullWidth);
  APInt C2Ext = C2.sext(FullWidth);
  प्रत्यागम C1Ext * C2Ext;
}

APInt APIntOps::muluExtended(ध्रुव APInt &C1, ध्रुव APInt &C2) {
  प्रतिज्ञा(C1.getBitWidth() == C2.getBitWidth() && "Unequal bitwidths");
  unsigned FullWidth = C1.getBitWidth() * 2;
  APInt C1Ext = C1.zext(FullWidth);
  APInt C2Ext = C2.zext(FullWidth);
  प्रत्यागम C1Ext * C2Ext;
}

APInt APIntOps::pow(ध्रुव APInt &X, int64_t N) {
  प्रतिज्ञा(N >= 0 && "negative exponents not supported.");
  APInt Acc = APInt(X.getBitWidth(), 1);
  यदि (N == 0)
    प्रत्यागम Acc;
  APInt Base = X;
  int64_t RemainingExponent = N;
  यावत् (RemainingExponent > 0) {
    यावत् (RemainingExponent % 2 == 0) {
      Base *= Base;
      RemainingExponent /= 2;
    }
    --RemainingExponent;
    Acc *= Base;
  }
  प्रत्यागम Acc;
}

APInt llvm::APIntOps::fshl(ध्रुव APInt &Hi, ध्रुव APInt &Lo,
                           ध्रुव APInt &Shift) {
  प्रतिज्ञा(Hi.getBitWidth() == Lo.getBitWidth());
  unsigned ShiftAmt = rotateModulo(Hi.getBitWidth(), Shift);
  यदि (ShiftAmt == 0)
    प्रत्यागम Hi;
  प्रत्यागम Hi.shl(ShiftAmt) | Lo.lshr(Hi.getBitWidth() - ShiftAmt);
}

APInt llvm::APIntOps::fshr(ध्रुव APInt &Hi, ध्रुव APInt &Lo,
                           ध्रुव APInt &Shift) {
  प्रतिज्ञा(Hi.getBitWidth() == Lo.getBitWidth());
  unsigned ShiftAmt = rotateModulo(Hi.getBitWidth(), Shift);
  यदि (ShiftAmt == 0)
    प्रत्यागम Lo;
  प्रत्यागम Hi.shl(Hi.getBitWidth() - ShiftAmt) | Lo.lshr(ShiftAmt);
}

APInt llvm::APIntOps::clmul(ध्रुव APInt &LHS, ध्रुव APInt &RHS) {
  unsigned BW = LHS.getBitWidth();
  प्रतिज्ञा(BW == RHS.getBitWidth() && "Operand mismatch");
  APInt Result(BW, 0);
  प्रति (unsigned I : seq(std::min(RHS.getActiveBits(), BW - LHS.countr_zero())))
    यदि (RHS[I])
      Result ^= LHS << I;
  प्रत्यागम Result;
}

APInt llvm::APIntOps::clmulr(ध्रुव APInt &LHS, ध्रुव APInt &RHS) {
  प्रतिज्ञा(LHS.getBitWidth() == RHS.getBitWidth());
  प्रत्यागम clmul(LHS.reverseBits(), RHS.reverseBits()).reverseBits();
}

APInt llvm::APIntOps::clmulh(ध्रुव APInt &LHS, ध्रुव APInt &RHS) {
  प्रतिज्ञा(LHS.getBitWidth() == RHS.getBitWidth());
  प्रत्यागम clmulr(LHS, RHS).lshr(1);
}

APInt llvm::APIntOps::pext(ध्रुव APInt &Val, ध्रुव APInt &Mask) {
  unsigned BW = Val.getBitWidth();
  प्रतिज्ञा(BW == Mask.getBitWidth() && "Operand mismatch");
  APInt Result = APInt::getZero(BW);
  प्रति (unsigned I = 0, P = 0; I != BW; ++I)
    यदि (Mask[I])
      Result.setBitVal(P++, Val[I]);
  प्रत्यागम Result;
}

APInt llvm::APIntOps::pdep(ध्रुव APInt &Val, ध्रुव APInt &Mask) {
  unsigned BW = Val.getBitWidth();
  प्रतिज्ञा(BW == Mask.getBitWidth() && "Operand mismatch");
  APInt Result = APInt::getZero(BW);
  प्रति (unsigned I = 0, P = 0; I != BW; ++I)
    यदि (Mask[I])
      Result.setBitVal(I, Val[P++]);
  प्रत्यागम Result;
}
