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

using نطاق llvm;

#define DEBUG_TYPE "apint"

/// A utility function for allocating memory, checking for allocation failures,
/// and ensuring the contents are zeroed.
مضمن ساكن uint64_t* getClearedMemory(unsigned لمب_كلموم) {
  أرجع جديد uint64_t[لمب_كلموم]();
}

/// A utility function for allocating memory and checking for allocation
/// failure.  The content is not zeroed.
مضمن ساكن uint64_t* getMemory(unsigned لمب_كلموم) {
  أرجع جديد uint64_t[لمب_كلموم];
}

/// A utility function that converts a character to a digit.
مضمن ساكن unsigned getDigit(حرف cdigit, uint8_t radix) {
  unsigned r;

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
    U.pVal = getMemory(get_لمب_كلموم());
    U.pVal[0] = val;
    memset(&U.pVal[1], 0xFF, APINT_WORD_SIZE * (get_لمب_كلموم() - 1));
    clearUnusedBits();
  } وإلا {
    U.pVal = getClearedMemory(get_لمب_كلموم());
    U.pVal[0] = val;
  }
}

فراغ APInt::initSlowCase(ثابت APInt& that) {
  U.pVal = getMemory(get_لمب_كلموم());
  memcpy(U.pVal, that.U.pVal, get_لمب_كلموم() * APINT_WORD_SIZE);
}

فراغ APInt::initFromArray(ArrayRef<uint64_t> bigVal) {
  أكد(bigVal.data() && "Null pointer detected!");
  إذا (LLVM_LIKELY(isSingleWord()))
    U.VAL = bigVal[0];
  وإلا {
    // Get memory, cleared to 0
    U.pVal = getClearedMemory(get_لمب_كلموم());
    // Calculate the number of words to copy
    unsigned كلموم = std::min<unsigned>(bigVal.size(), get_لمب_كلموم());
    // Copy the words from bigVal to pVal
    memcpy(U.pVal, bigVal.data(), كلموم * APINT_WORD_SIZE);
  }
  // Make sure unused high bits are cleared
  clearUnusedBits();
}

APInt::APInt(unsigned لمب_Bits, ArrayRef<uint64_t> bigVal) : BitWidth(لمب_Bits) {
  initFromArray(bigVal);
}

APInt::APInt(unsigned numbits, StringRef Str, uint8_t radix)
    : BitWidth(numbits) {
  fromString(numbits, Str, radix);
}

فراغ APInt::reallocate(unsigned NewBitWidth) {
  // If the number of words is the same we can just change the width and stop.
  إذا (get_لمب_كلموم() == get_لمب_كلموم(NewBitWidth)) {
    BitWidth = NewBitWidth;
    أرجع;
  }

  // If we have an allocation, delete it.
  إذا (LLVM_UNLIKELY(!isSingleWord()))
    delete [] U.pVal;

  // Update BitWidth.
  BitWidth = NewBitWidth;

  // If we are supposed to have an allocation, create it.
  إذا (LLVM_UNLIKELY(!isSingleWord()))
    U.pVal = getMemory(get_لمب_كلموم());
}

فراغ APInt::assignSlowCase(ثابت APInt &RHS) {
  // Don't do anything for X = X
  إذا (هذا == &RHS)
    أرجع;

  // Adjust the bit width and handle allocations as necessary.
  reallocate(RHS.getBitWidth());

  // Copy the data.
  إذا (LLVM_LIKELY(isSingleWord()))
    U.VAL = RHS.U.VAL;
  وإلا
    memcpy(U.pVal, RHS.U.pVal, get_لمب_كلموم() * APINT_WORD_SIZE);
}

/// This method 'profiles' an APInt for use with FoldingSet.
فراغ APInt::Profile(Folding_Set_كلي_ID& ID) ثابت {
  ID.AddInteger(BitWidth);

  إذا (LLVM_LIKELY(isSingleWord())) {
    ID.AddInteger(U.VAL);
    أرجع;
  }

  unsigned لمب_كلموم_2 = get_لمب_كلموم();
  لكل (unsigned i = 0; i < لمب_كلموم_2; ++i)
    ID.AddInteger(U.pVal[i]);
}

منطقي APInt::isAligned(Align A) ثابت {
  إذا (isZero())
    أرجع true;
  ثابت unsigned TrailingZeroes = countr_zero();
  ثابت unsigned MinimumTrailingZeroes = Log2(A);
  أرجع TrailingZeroes >= MinimumTrailingZeroes;
}

/// Prefix increment operator. Increments the APInt by one.
APInt& APInt::operator++() {
  إذا (LLVM_LIKELY(isSingleWord()))
    ++U.VAL;
  وإلا
    tcIncrement(U.pVal, get_لمب_كلموم());
  أرجع clearUnusedBits();
}

/// Prefix decrement operator. Decrements the APInt by one.
APInt& APInt::operator--() {
  إذا (LLVM_LIKELY(isSingleWord()))
    --U.VAL;
  وإلا
    tcDecrement(U.pVal, get_لمب_كلموم());
  أرجع clearUnusedBits();
}

/// Adds the RHS APInt to this APInt.
/// @returns this, after addition of RHS.
/// Addition assignment operator.
APInt& APInt::operator+=(ثابت APInt& RHS) {
  أكد(BitWidth == RHS.BitWidth && "Bit widths must be the same");
  إذا (LLVM_LIKELY(isSingleWord()))
    U.VAL += RHS.U.VAL;
  وإلا
    tcAdd(U.pVal, RHS.U.pVal, 0, get_لمب_كلموم());
  أرجع clearUnusedBits();
}

APInt& APInt::operator+=(uint64_t RHS) {
  إذا (LLVM_LIKELY(isSingleWord()))
    U.VAL += RHS;
  وإلا
    tcAddPart(U.pVal, RHS, get_لمب_كلموم());
  أرجع clearUnusedBits();
}

/// Subtracts the RHS APInt from this APInt
/// @returns this, after subtraction
/// Subtraction assignment operator.
APInt& APInt::operator-=(ثابت APInt& RHS) {
  أكد(BitWidth == RHS.BitWidth && "Bit widths must be the same");
  إذا (LLVM_LIKELY(isSingleWord()))
    U.VAL -= RHS.U.VAL;
  وإلا
    tcSubtract(U.pVal, RHS.U.pVal, 0, get_لمب_كلموم());
  أرجع clearUnusedBits();
}

APInt& APInt::operator-=(uint64_t RHS) {
  إذا (LLVM_LIKELY(isSingleWord()))
    U.VAL -= RHS;
  وإلا
    tcSubtractPart(U.pVal, RHS, get_لمب_كلموم());
  أرجع clearUnusedBits();
}

APInt APInt::operator*(ثابت APInt& RHS) ثابت {
  أكد(BitWidth == RHS.BitWidth && "Bit widths must be the same");
  إذا (LLVM_LIKELY(isSingleWord()))
    أرجع APInt(BitWidth, U.VAL * RHS.U.VAL, /*isSigned=*/خطأ,
                 /*implicitTrunc=*/true);

  APInt Result(getMemory(get_لمب_كلموم()), getBitWidth());
  tcMultiply(Result.U.pVal, U.pVal, RHS.U.pVal, get_لمب_كلموم());
  Result.clearUnusedBits();
  أرجع Result;
}

فراغ APInt::andAssignSlowCase(ثابت APInt &RHS) {
  WordType *dst = U.pVal, *rhs = RHS.U.pVal;
  لكل (size_t i = 0, e = get_لمب_كلموم(); i != e; ++i)
    dst[i] &= rhs[i];
}

فراغ APInt::orAssignSlowCase(ثابت APInt &RHS) {
  WordType *dst = U.pVal, *rhs = RHS.U.pVal;
  لكل (size_t i = 0, e = get_لمب_كلموم(); i != e; ++i)
    dst[i] |= rhs[i];
}

فراغ APInt::xorAssignSlowCase(ثابت APInt &RHS) {
  WordType *dst = U.pVal, *rhs = RHS.U.pVal;
  لكل (size_t i = 0, e = get_لمب_كلموم(); i != e; ++i)
    dst[i] ^= rhs[i];
}

APInt &APInt::operator*=(ثابت APInt &RHS) {
  *هذا = *هذا * RHS;
  أرجع *هذا;
}

APInt& APInt::operator*=(uint64_t RHS) {
  إذا (LLVM_LIKELY(isSingleWord())) {
    U.VAL *= RHS;
  } وإلا {
    unsigned لمب_كلموم_2 = get_لمب_كلموم();
    tcMultiplyPart(U.pVal, U.pVal, RHS, 0, لمب_كلموم_2, لمب_كلموم_2, خطأ);
  }
  أرجع clearUnusedBits();
}

منطقي APInt::equalSlowCase(ثابت APInt &RHS) ثابت {
  أرجع std::equal(U.pVal, U.pVal + get_لمب_كلموم(), RHS.U.pVal);
}

صحيح APInt::compare(ثابت APInt& RHS) ثابت {
  أكد(BitWidth == RHS.BitWidth && "Bit widths must be same for comparison");
  إذا (LLVM_LIKELY(isSingleWord()))
    أرجع U.VAL < RHS.U.VAL ? -1 : U.VAL > RHS.U.VAL;

  أرجع tcCompare(U.pVal, RHS.U.pVal, get_لمب_كلموم());
}

صحيح APInt::compareSigned(ثابت APInt& RHS) ثابت {
  أكد(BitWidth == RHS.BitWidth && "Bit widths must be same for comparison");
  إذا (LLVM_LIKELY(isSingleWord())) {
    int64_t lhsSext = SignExtend64(U.VAL, BitWidth);
    int64_t rhsSext = SignExtend64(RHS.U.VAL, BitWidth);
    أرجع lhsSext < rhsSext ? -1 : lhsSext > rhsSext;
  }

  منطقي lhsNeg = isNegative();
  منطقي rhsNeg = RHS.isNegative();

  // If the sign bits don't match, then (LHS < RHS) if LHS is negative
  إذا (lhsNeg != rhsNeg)
    أرجع lhsNeg ? -1 : 1;

  // Otherwise we can just use an unsigned comparison, because even negative
  // numbers compare correctly this way if both have the same signed-ness.
  أرجع tcCompare(U.pVal, RHS.U.pVal, get_لمب_كلموم());
}

فراغ APInt::setBitsSlowCase(unsigned loBit, unsigned hiBit) {
  unsigned loWord = whichWord(loBit);
  unsigned hiWord = whichWord(hiBit);

  // Create an initial mask for the low word with zeros below loBit.
  uint64_t loMask = WORDTYPE_MAX << whichBit(loBit);

  // If hiBit is not aligned, we need a high mask.
  unsigned hiShiftAmt = whichBit(hiBit);
  إذا (hiShiftAmt != 0) {
    // Create a high mask with zeros above hiBit.
    uint64_t hiMask = WORDTYPE_MAX >> (APINT_BITS_PER_WORD - hiShiftAmt);
    // If loWord and hiWord are equal, then we combine the masks. Otherwise,
    // set the bits in hiWord.
    إذا (hiWord == loWord)
      loMask &= hiMask;
    وإلا
      U.pVal[hiWord] |= hiMask;
  }
  // Apply the mask to the low word.
  U.pVal[loWord] |= loMask;

  // Fill any words between loWord and hiWord with all ones.
  لكل (unsigned word = loWord + 1; word < hiWord; ++word)
    U.pVal[word] = WORDTYPE_MAX;
}

فراغ APInt::clearBitsSlowCase(unsigned LoBit, unsigned HiBit) {
  unsigned LoWord = whichWord(LoBit);
  unsigned HiWord = whichWord(HiBit);

  // Create an initial mask for the low word with ones below loBit.
  uint64_t LoMask = ~(WORDTYPE_MAX << whichBit(LoBit));

  // If HiBit is not aligned, we need a high mask.
  unsigned HiShiftAmt = whichBit(HiBit);
  إذا (HiShiftAmt != 0) {
    // Create a high mask with ones above HiBit.
    uint64_t HiMask = ~(WORDTYPE_MAX >> (APINT_BITS_PER_WORD - HiShiftAmt));
    // If LoWord and HiWord are equal, then we combine the masks. Otherwise,
    // clear the bits in HiWord.
    إذا (HiWord == LoWord)
      LoMask |= HiMask;
    وإلا
      U.pVal[HiWord] &= HiMask;
  }
  // Apply the mask to the low word.
  U.pVal[LoWord] &= LoMask;

  // Fill any words between LoWord and HiWord with all zeros.
  لكل (unsigned Word = LoWord + 1; Word < HiWord; ++Word)
    U.pVal[Word] = 0;
}

// Complement a bignum in-place.
ساكن فراغ tcComplement(APInt::WordType *dst, unsigned parts) {
  لكل (unsigned i = 0; i < parts; i++)
    dst[i] = ~dst[i];
}

/// Toggle every bit to its opposite value.
فراغ APInt::flipAllBitsSlowCase() {
  tcComplement(U.pVal, get_لمب_كلموم());
  clearUnusedBits();
}

/// Concatenate the bits from "NewLSB" onto the bottom of *this.  This is
/// equivalent to:
///   (this->zext(NewWidth) << NewLSB.getBitWidth()) | NewLSB.zext(NewWidth)
/// In the slow case, we know the result is large.
APInt APInt::concatSlowCase(ثابت APInt &NewLSB) ثابت {
  unsigned NewWidth = getBitWidth() + NewLSB.getBitWidth();
  APInt Result = NewLSB.zext(NewWidth);
  Result.سك_Bits(*هذا, NewLSB.getBitWidth());
  أرجع Result;
}

/// Toggle a given bit to its opposite value whose position is given
/// as "bitPosition".
/// Toggles a given bit to its opposite value.
فراغ APInt::flipBit(unsigned bitPosition) {
  أكد(bitPosition < BitWidth && "Out of the bit-width range!");
  setBitVal(bitPosition, !(*هذا)[bitPosition]);
}

فراغ APInt::سك_Bits(ثابت APInt &subBits, unsigned bitPosition) {
  unsigned subBitWidth = subBits.getBitWidth();
  أكد((subBitWidth + bitPosition) <= BitWidth && "Illegal bit insertion");

  // inserting no bits is a noop.
  إذا (subBitWidth == 0)
    أرجع;

  // Insertion is a direct copy.
  إذا (subBitWidth == BitWidth) {
    *هذا = subBits;
    أرجع;
  }

  // Single word result can be done as a direct bitmask.
  إذا (LLVM_LIKELY(isSingleWord())) {
    uint64_t mask = WORDTYPE_MAX >> (APINT_BITS_PER_WORD - subBitWidth);
    U.VAL &= ~(mask << bitPosition);
    U.VAL |= (subBits.U.VAL << bitPosition);
    أرجع;
  }

  unsigned loBit = whichBit(bitPosition);
  unsigned loWord = whichWord(bitPosition);
  unsigned hi1Word = whichWord(bitPosition + subBitWidth - 1);

  // Insertion within a single word can be done as a direct bitmask.
  إذا (loWord == hi1Word) {
    uint64_t mask = WORDTYPE_MAX >> (APINT_BITS_PER_WORD - subBitWidth);
    U.pVal[loWord] &= ~(mask << loBit);
    U.pVal[loWord] |= (subBits.U.VAL << loBit);
    أرجع;
  }

  // Insert on word boundaries.
  إذا (loBit == 0) {
    // Direct copy whole words.
    unsigned لمب_Whole_Sub_كلموم = subBitWidth / APINT_BITS_PER_WORD;
    memcpy(U.pVal + loWord, subBits.getRawData(),
           لمب_Whole_Sub_كلموم * APINT_WORD_SIZE);

    // Mask+insert remaining bits.
    unsigned remainingBits = subBitWidth % APINT_BITS_PER_WORD;
    إذا (remainingBits != 0) {
      uint64_t mask = WORDTYPE_MAX >> (APINT_BITS_PER_WORD - remainingBits);
      U.pVal[hi1Word] &= ~mask;
      U.pVal[hi1Word] |= subBits.getWord(subBitWidth - 1);
    }
    أرجع;
  }

  // General case - set/clear individual bits in dst based on src.
  // TODO - there is scope for optimization here, but at the moment this code
  // path is barely used so prefer readability over performance.
  لكل (unsigned i = 0; i != subBitWidth; ++i)
    setBitVal(bitPosition + i, subBits[i]);
}

فراغ APInt::سك_Bits(uint64_t subBits, unsigned bitPosition, unsigned لمب_Bits) {
  uint64_t maskBits = maskTrailingOnes<uint64_t>(لمب_Bits);
  subBits &= maskBits;
  إذا (LLVM_LIKELY(isSingleWord())) {
    U.VAL &= ~(maskBits << bitPosition);
    U.VAL |= subBits << bitPosition;
    أرجع;
  }

  unsigned loBit = whichBit(bitPosition);
  unsigned loWord = whichWord(bitPosition);
  unsigned hiWord = whichWord(bitPosition + لمب_Bits - 1);
  إذا (loWord == hiWord) {
    U.pVal[loWord] &= ~(maskBits << loBit);
    U.pVal[loWord] |= subBits << loBit;
    أرجع;
  }

  static_assert(8 * حجم(WordType) <= 64, "This code assumes only two words affected");
  unsigned wordBits = 8 * حجم(WordType);
  U.pVal[loWord] &= ~(maskBits << loBit);
  U.pVal[loWord] |= subBits << loBit;

  U.pVal[hiWord] &= ~(maskBits >> (wordBits - loBit));
  U.pVal[hiWord] |= subBits >> (wordBits - loBit);
}

APInt APInt::extractBits(unsigned لمب_Bits, unsigned bitPosition) ثابت {
  أكد(bitPosition < BitWidth && (لمب_Bits + bitPosition) <= BitWidth &&
         "Illegal bit extraction");

  إذا (LLVM_LIKELY(isSingleWord()))
    أرجع APInt(لمب_Bits, U.VAL >> bitPosition, /*isSigned=*/خطأ,
                 /*implicitTrunc=*/true);

  unsigned loBit = whichBit(bitPosition);
  unsigned loWord = whichWord(bitPosition);
  unsigned hiWord = whichWord(bitPosition + لمب_Bits - 1);

  // Single word result extracting bits from a single word source.
  إذا (loWord == hiWord)
    أرجع APInt(لمب_Bits, U.pVal[loWord] >> loBit, /*isSigned=*/خطأ,
                 /*implicitTrunc=*/true);

  // Extracting bits that start on a source word boundary can be done
  // as a fast memory copy.
  إذا (loBit == 0)
    أرجع APInt(لمب_Bits, ArrayRef(U.pVal + loWord, 1 + hiWord - loWord));

  // General case - shift + copy source words directly into place.
  APInt Result(لمب_Bits, 0);
  unsigned لمب_Src_كلموم = get_لمب_كلموم();
  unsigned لمب_Dst_كلموم = Result.get_لمب_كلموم();

  uint64_t *DestPtr =
      LLVM_LIKELY(Result.isSingleWord()) ? &Result.U.VAL : Result.U.pVal;
  لكل (unsigned word = 0; word < لمب_Dst_كلموم; ++word) {
    uint64_t w0 = U.pVal[loWord + word];
    uint64_t w1 =
        (loWord + word + 1) < لمب_Src_كلموم ? U.pVal[loWord + word + 1] : 0;
    DestPtr[word] = (w0 >> loBit) | (w1 << (APINT_BITS_PER_WORD - loBit));
  }

  أرجع Result.clearUnusedBits();
}

uint64_t APInt::extract_Bits_As_ZExt_درج(unsigned لمب_Bits,
                                       unsigned bitPosition) ثابت {
  أكد(bitPosition < BitWidth && (لمب_Bits + bitPosition) <= BitWidth &&
         "Illegal bit extraction");
  أكد(لمب_Bits <= 64 && "Illegal bit extraction");

  uint64_t maskBits = maskTrailingOnes<uint64_t>(لمب_Bits);
  إذا (LLVM_LIKELY(isSingleWord()))
    أرجع (U.VAL >> bitPosition) & maskBits;

  static_assert(APINT_BITS_PER_WORD >= 64,
                "This code assumes only two words affected");
  unsigned loBit = whichBit(bitPosition);
  unsigned loWord = whichWord(bitPosition);
  unsigned hiWord = whichWord(bitPosition + لمب_Bits - 1);
  إذا (loWord == hiWord)
    أرجع (U.pVal[loWord] >> loBit) & maskBits;

  uint64_t retBits = U.pVal[loWord] >> loBit;
  retBits |= U.pVal[hiWord] << (APINT_BITS_PER_WORD - loBit);
  retBits &= maskBits;
  أرجع retBits;
}

unsigned APInt::getSufficientBitsNeeded(StringRef Str, uint8_t Radix) {
  أكد(!Str.empty() && "Invalid string length");
  size_t StrLen = Str.size();

  // Each computation below needs to know if it's negative.
  unsigned IsNegative = خطأ;
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

unsigned APInt::getBitsNeeded(StringRef str, uint8_t radix) {
  // Compute a sufficient number of bits that is always large enough but might
  // be too large.
  unsigned sufficient = getSufficientBitsNeeded(str, radix);

  // For bases 2, 8, and 16, the sufficient number of bits is exact and we can
  // return the value directly. For bases 10 and 36, we need to do extra work.
  إذا (radix == 2 || radix == 8 || radix == 16)
    أرجع sufficient;

  // This is grossly inefficient but accurate. We could probably do something
  // with a computation of roughly slen*64/20 and then adjust by the value of
  // the first few digits. But, I'm not sure how accurate that could be.
  size_t slen = str.size();

  // Each computation below needs to know if it's negative.
  StringRef::iterator p = str.begin();
  unsigned isNegative = *p == '-';
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
  unsigned log = tmp.logBase2();
  إذا (log == (unsigned)-1) {
    أرجع isNegative + 1;
  } وإلا إذا (isNegative && tmp.isPowerOf2()) {
    أرجع isNegative + log;
  } وإلا {
    أرجع isNegative + log + 1;
  }
}

hash_code llvm::hash_درج(ثابت APInt &Arg) {
  إذا (LLVM_LIKELY(Arg.isSingleWord()))
    أرجع hash_combine(Arg.BitWidth, Arg.U.VAL);

  أرجع hash_combine(
      Arg.BitWidth,
      hash_combine_range(Arg.U.pVal, Arg.U.pVal + Arg.get_لمب_كلموم()));
}

unsigned DenseMapInfo<APInt, فراغ>::get_Hash_درج(ثابت APInt &مبودي) {
  أرجع static_cast<unsigned>(hash_درج(مبودي));
}

منطقي APInt::isSplat(unsigned SplatSizeInBits) ثابت {
  أكد(getBitWidth() % SplatSizeInBits == 0 &&
         "SplatSizeInBits must divide width!");
  // We can check that all parts of an integer are equal by making use of a
  // little trick: rotate and check if it's still the same value.
  أرجع *هذا == rotl(SplatSizeInBits);
}

/// This function returns the high "numBits" bits of this APInt.
APInt APInt::getHiBits(unsigned لمب_Bits) ثابت {
  أرجع هذا->lshr(BitWidth - لمب_Bits);
}

/// This function returns the low "numBits" bits of this APInt.
APInt APInt::getLoBits(unsigned لمب_Bits) ثابت {
  APInt Result(get_كس_Bits_Set(BitWidth, لمب_Bits));
  Result &= *هذا;
  أرجع Result;
}

/// Return a value containing V broadcasted over NewLen bits.
APInt APInt::getSplat(unsigned NewLen, ثابت APInt &V) {
  أكد(NewLen >= V.getBitWidth() && "Can't splat to smaller bit width!");

  APInt Val = V.zext(NewLen);
  لكل (unsigned I = V.getBitWidth(); I < NewLen; I <<= 1)
    Val |= Val << I;

  أرجع Val;
}

unsigned APInt::كدي_Leading_Zeros_Slow_Case() ثابت {
  unsigned كدي_2 = 0;
  لكل (صحيح i = get_لمب_كلموم() - 1; i >= 0; --i) {
    uint64_t V = U.pVal[i];
    إذا (V == 0)
      كدي_2 += APINT_BITS_PER_WORD;
    وإلا {
      كدي_2 += llvm::countl_zero(V);
      اكسر;
    }
  }
  // Adjust for unused bits in the most significant word (they are zero).
  unsigned Mod = BitWidth % APINT_BITS_PER_WORD;
  كدي_2 -= Mod > 0 ? APINT_BITS_PER_WORD - Mod : 0;
  أرجع كدي_2;
}

unsigned APInt::كدي_Leading_Ones_Slow_Case() ثابت {
  unsigned سم_Word_Bits = BitWidth % APINT_BITS_PER_WORD;
  unsigned shift;
  إذا (!سم_Word_Bits) {
    سم_Word_Bits = APINT_BITS_PER_WORD;
    shift = 0;
  } وإلا {
    shift = APINT_BITS_PER_WORD - سم_Word_Bits;
  }
  صحيح i = get_لمب_كلموم() - 1;
  unsigned كدي_2 = llvm::countl_one(U.pVal[i] << shift);
  إذا (كدي_2 == سم_Word_Bits) {
    لكل (i--; i >= 0; --i) {
      إذا (U.pVal[i] == WORDTYPE_MAX)
        كدي_2 += APINT_BITS_PER_WORD;
      وإلا {
        كدي_2 += llvm::countl_one(U.pVal[i]);
        اكسر;
      }
    }
  }
  أرجع كدي_2;
}

unsigned APInt::كدي_Trailing_Zeros_Slow_Case() ثابت {
  unsigned كدي_2 = 0;
  unsigned i = 0;
  لكل (; i < get_لمب_كلموم() && U.pVal[i] == 0; ++i)
    كدي_2 += APINT_BITS_PER_WORD;
  إذا (i < get_لمب_كلموم())
    كدي_2 += llvm::countr_zero(U.pVal[i]);
  أرجع std::min(كدي_2, BitWidth);
}

unsigned APInt::كدي_Trailing_Ones_Slow_Case() ثابت {
  unsigned كدي_2 = 0;
  unsigned i = 0;
  لكل (; i < get_لمب_كلموم() && U.pVal[i] == WORDTYPE_MAX; ++i)
    كدي_2 += APINT_BITS_PER_WORD;
  إذا (i < get_لمب_كلموم())
    كدي_2 += llvm::countr_one(U.pVal[i]);
  أكد(كدي_2 <= BitWidth);
  أرجع كدي_2;
}

unsigned APInt::كدي_Population_Slow_Case() ثابت {
  unsigned كدي_2 = 0;
  لكل (unsigned i = 0; i < get_لمب_كلموم(); ++i)
    كدي_2 += llvm::popcount(U.pVal[i]);
  أرجع كدي_2;
}

منطقي APInt::isPowerOf2SlowCase() ثابت {
  unsigned كدي_2 = 0;
  لكل (unsigned i = 0; i < get_لمب_كلموم(); ++i) {
    كدي_2 += llvm::popcount(U.pVal[i]);
    إذا (كدي_2 > 1)
      أرجع خطأ;
  }
  أرجع كدي_2 == 1;
}

منطقي APInt::intersectsSlowCase(ثابت APInt &RHS) ثابت {
  لكل (unsigned i = 0, e = get_لمب_كلموم(); i != e; ++i)
    إذا ((U.pVal[i] & RHS.U.pVal[i]) != 0)
      أرجع true;

  أرجع خطأ;
}

منطقي APInt::isSubsetOfSlowCase(ثابت APInt &RHS) ثابت {
  لكل (unsigned i = 0, e = get_لمب_كلموم(); i != e; ++i)
    إذا ((U.pVal[i] & ~RHS.U.pVal[i]) != 0)
      أرجع خطأ;

  أرجع true;
}

منطقي APInt::isInverseOfSlowCase(ثابت APInt &RHS) ثابت {
  ثابت unsigned Last = get_لمب_كلموم() - 1;
  لكل (unsigned I = 0; I != Last; ++I)
    إذا ((U.pVal[I] ^ RHS.U.pVal[I]) != WORDTYPE_MAX)
      أرجع خطأ;

  unsigned TailBits = BitWidth - Last * APINT_BITS_PER_WORD;
  WordType TailMask = llvm::maskTrailingOnes<WordType>(TailBits);
  أرجع (U.pVal[Last] ^ RHS.U.pVal[Last]) == TailMask;
}

APInt APInt::byteSwap() ثابت {
  أكد(BitWidth >= 16 && BitWidth % 8 == 0 && "Cannot byteswap!");
  إذا (BitWidth == 16)
    أرجع APInt(BitWidth, llvm::byteswap<uint16_t>(U.VAL));
  إذا (BitWidth == 32)
    أرجع APInt(BitWidth, llvm::byteswap<uint32_t>(U.VAL));
  إذا (BitWidth <= 64) {
    uint64_t Tmp1 = llvm::byteswap<uint64_t>(U.VAL);
    Tmp1 >>= (64 - BitWidth);
    أرجع APInt(BitWidth, Tmp1);
  }

  APInt Result(get_لمب_كلموم() * APINT_BITS_PER_WORD, 0);
  لكل (unsigned I = 0, N = get_لمب_كلموم(); I != N; ++I)
    Result.U.pVal[I] = llvm::byteswap<uint64_t>(U.pVal[N - I - 1]);
  إذا (Result.BitWidth != BitWidth) {
    Result.lshrInPlace(Result.BitWidth - BitWidth);
    Result.BitWidth = BitWidth;
  }
  أرجع Result;
}

APInt APInt::reverseBits() ثابت {
  إذا (LLVM_LIKELY(isSingleWord())) {
    بدل (BitWidth) {
    حالة 64:
      أرجع APInt(BitWidth, llvm::reverseBits<uint64_t>(U.VAL));
    حالة 32:
      أرجع APInt(BitWidth, llvm::reverseBits<uint32_t>(U.VAL));
    حالة 16:
      أرجع APInt(BitWidth, llvm::reverseBits<uint16_t>(U.VAL));
    حالة 8:
      أرجع APInt(BitWidth, llvm::reverseBits<uint8_t>(U.VAL));
    حالة 1: // fallthrough
    حالة 0:
      أرجع *هذا;
    افتراضي:
      أرجع APInt(BitWidth,
                   llvm::reverseBits<uint64_t>(U.VAL) >> (64 - BitWidth));
    }
  }

  APInt Result(BitWidth, 0);
  unsigned لمب_كلموم_2 = get_لمب_كلموم();
  unsigned ExcessBits = لمب_كلموم_2 * APINT_BITS_PER_WORD - BitWidth;
  إذا (ExcessBits == 0) {
    // Fast path. No cross-word shift needed.
    لكل (unsigned I = 0; I < لمب_كلموم_2; ++I)
      Result.U.pVal[I] = llvm::reverseBits<uint64_t>(U.pVal[لمب_كلموم_2 - 1 - I]);
    أرجع Result;
  }
  // Holds reversed bits of the previous (more significant) word.
  uint64_t PrevRev = llvm::reverseBits<uint64_t>(U.pVal[لمب_كلموم_2 - 1]);
  لكل (unsigned I = 0; I < لمب_كلموم_2 - 1; ++I) {
    uint64_t CurrRev = llvm::reverseBits<uint64_t>(U.pVal[لمب_كلموم_2 - 2 - I]);
    Result.U.pVal[I] = (PrevRev >> ExcessBits) | (CurrRev << (64 - ExcessBits));
    PrevRev = CurrRev;
  }
  Result.U.pVal[لمب_كلموم_2 - 1] = PrevRev >> ExcessBits;
  أرجع Result;
}

APInt llvm::APIntOps::Greatest_Common_مي_ربو(APInt A, APInt B, منطقي IsSigned) {
  // Take absolute value if IsSigned.
  إذا (IsSigned) {
    A = A.abs();
    B = B.abs();
  }

  // Fast-path a common case.
  إذا (A == B) أرجع A;

  // Corner cases: if either operand is zero, the other is the gcd.
  إذا (!A) أرجع B;
  إذا (!B) أرجع A;

  // Count common powers of 2 and remove all other powers of 2.
  unsigned Pow2;
  {
    unsigned Pow2_A = A.countr_zero();
    unsigned Pow2_B = B.countr_zero();
    إذا (Pow2_A > Pow2_B) {
      A.lshrInPlace(Pow2_A - Pow2_B);
      Pow2 = Pow2_B;
    } وإلا إذا (Pow2_B > Pow2_A) {
      B.lshrInPlace(Pow2_B - Pow2_A);
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
      A.lshrInPlace(A.countr_zero() - Pow2);
    } وإلا {
      B -= A;
      B.lshrInPlace(B.countr_zero() - Pow2);
    }
  }

  أرجع A;
}

APInt llvm::APIntOps::RoundDoubleToAPInt(مزدوج Double, unsigned width) {
  uint64_t I = bit_cast<uint64_t>(Double);

  // Get the sign bit from the highest order bit
  منطقي isNeg = I >> 63;

  // Get the 11-bit exponent and adjust for the 1023 bit bias
  int64_t exp = ((I >> 52) & 0x7ff) - 1023;

  // If the exponent is negative, the value is < 0 so just return 0.
  إذا (exp < 0)
    أرجع APInt(width, 0u);

  // Extract the mantissa by clearing the top 12 bits (sign + exponent).
  uint64_t mantissa = (I & (~0ULL >> 12)) | 1ULL << 52;

  // If the exponent doesn't shift all bits out of the mantissa
  إذا (exp < 52)
    أرجع isNeg ? -APInt(width, mantissa >> (52 - exp)) :
                    APInt(width, mantissa >> (52 - exp));

  // If the client didn't provide enough bits for us to shift the mantissa into
  // then the result is undefined, just return 0
  إذا (width <= exp - 52)
    أرجع APInt(width, 0);

  // Otherwise, we have to shift the mantissa bits up to the right location
  APInt Tmp(width, mantissa);
  Tmp <<= (unsigned)exp - 52;
  أرجع isNeg ? -Tmp : Tmp;
}

/// This function converts this APInt to a double.
/// The layout for double is as following (IEEE Standard 754):
///  --------------------------------------
/// |  Sign    Exponent    Fraction    Bias |
/// |-------------------------------------- |
/// |  1[63]   11[62-52]   52[51-00]   1023 |
///  --------------------------------------
مزدوج APInt::roundToDouble(منطقي isSigned) ثابت {
  // Handle the simple case where the value is contained in one uint64_t.
  // It is wrong to optimize getWord(0) to VAL; there might be more than one word.
  إذا (LLVM_LIKELY(isSingleWord() || getActiveBits() <= APINT_BITS_PER_WORD)) {
    إذا (isSigned) {
      int64_t sext = SignExtend64(getWord(0), BitWidth);
      أرجع مزدوج(sext);
    }
    أرجع مزدوج(getWord(0));
  }

  // Determine if the value is negative.
  منطقي isNeg = isSigned ? (*هذا)[BitWidth-1] : خطأ;

  // Construct the absolute value if we're negative.
  APInt Tmp(isNeg ? -(*هذا) : (*هذا));

  // Figure out how many bits we're using.
  unsigned n = Tmp.getActiveBits();

  // The exponent (without bias normalization) is just the number of bits
  // we are using. Note that the sign bit is gone since we constructed the
  // absolute value.
  uint64_t exp = n;

  // Return infinity for exponent overflow
  إذا (exp > 1023) {
    إذا (!isSigned || !isNeg)
      أرجع std::numeric_limits<مزدوج>::infinity();
    وإلا
      أرجع -std::numeric_limits<مزدوج>::infinity();
  }
  exp += 1023; // Increment for 1023 bias

  // Number of bits in mantissa is 52. To obtain the mantissa value, we must
  // extract the high 52 bits from the correct words in pVal.
  uint64_t mantissa;
  unsigned hiWord = whichWord(n-1);
  إذا (hiWord == 0) {
    mantissa = Tmp.U.pVal[0];
    إذا (n > 52)
      mantissa >>= n - 52; // shift down, we want the top 52 bits.
  } وإلا {
    أكد(hiWord > 0 && "huh?");
    uint64_t hibits = Tmp.U.pVal[hiWord] << (52 - n % APINT_BITS_PER_WORD);
    uint64_t lobits = Tmp.U.pVal[hiWord-1] >> (11 + n % APINT_BITS_PER_WORD);
    mantissa = hibits | lobits;
  }

  // The leading bit of mantissa is implicit, so get rid of it.
  uint64_t sign = isNeg ? (1ULL << (APINT_BITS_PER_WORD - 1)) : 0;
  uint64_t I = sign | (exp << 52) | mantissa;
  أرجع bit_cast<مزدوج>(I);
}

// Truncate to new width.
APInt APInt::trunc(unsigned width) ثابت {
  أكد(width <= BitWidth && "Invalid APInt Truncate request");

  إذا (width <= APINT_BITS_PER_WORD)
    أرجع APInt(width, getRawData()[0], /*isSigned=*/خطأ,
                 /*implicitTrunc=*/true);

  إذا (width == BitWidth)
    أرجع *هذا;

  APInt Result(getMemory(get_لمب_كلموم(width)), width);

  // Copy full words.
  unsigned i;
  لكل (i = 0; i != width / APINT_BITS_PER_WORD; i++)
    Result.U.pVal[i] = U.pVal[i];

  // Truncate and copy any partial word.
  unsigned bits = (0 - width) % APINT_BITS_PER_WORD;
  إذا (bits != 0)
    Result.U.pVal[i] = U.pVal[i] << bits >> bits;

  أرجع Result;
}

// Truncate to new width with unsigned saturation.
APInt APInt::truncUSat(unsigned width) ثابت {
  أكد(width <= BitWidth && "Invalid APInt Truncate request");

  // Can we just losslessly truncate it?
  إذا (isIntN(width))
    أرجع trunc(width);
  // If not, then just return the new limit.
  أرجع APInt::get_Max_درج(width);
}

// Truncate to new width with signed saturation to signed result.
APInt APInt::truncSSat(unsigned width) ثابت {
  أكد(width <= BitWidth && "Invalid APInt Truncate request");

  // Can we just losslessly truncate it?
  إذا (isSignedIntN(width))
    أرجع trunc(width);
  // If not, then just return the new limits.
  أرجع isNegative() ? APInt::get_Signed_Min_درج(width)
                      : APInt::get_Signed_Max_درج(width);
}

// Truncate to new width with signed saturation to unsigned result.
APInt APInt::truncSSatU(unsigned width) ثابت {
  أكد(width <= BitWidth && "Invalid APInt Truncate request");

  // Can we just losslessly truncate it?
  إذا (isIntN(width))
    أرجع trunc(width);
  // If not, then just return the new limits.
  أرجع isNegative() ? APInt::getZero(width) : APInt::get_Max_درج(width);
}

// Sign extend to a new width.
APInt APInt::sext(unsigned Width) ثابت {
  أكد(Width >= BitWidth && "Invalid APInt SignExtend request");

  إذا (Width <= APINT_BITS_PER_WORD)
    أرجع APInt(Width, SignExtend64(U.VAL, BitWidth), /*isSigned=*/true);

  إذا (Width == BitWidth)
    أرجع *هذا;

  APInt Result(getMemory(get_لمب_كلموم(Width)), Width);

  // Copy words.
  std::memcpy(Result.U.pVal, getRawData(), get_لمب_كلموم() * APINT_WORD_SIZE);

  // Sign extend the last word since there may be unused bits in the input.
  Result.U.pVal[get_لمب_كلموم() - 1] =
      SignExtend64(Result.U.pVal[get_لمب_كلموم() - 1],
                   ((BitWidth - 1) % APINT_BITS_PER_WORD) + 1);

  // Fill with sign bits.
  std::memset(Result.U.pVal + get_لمب_كلموم(), isNegative() ? -1 : 0,
              (Result.get_لمب_كلموم() - get_لمب_كلموم()) * APINT_WORD_SIZE);
  Result.clearUnusedBits();
  أرجع Result;
}

//  Zero extend to a new width.
APInt APInt::zext(unsigned width) ثابت {
  أكد(width >= BitWidth && "Invalid APInt ZeroExtend request");

  إذا (width <= APINT_BITS_PER_WORD)
    أرجع APInt(width, U.VAL);

  إذا (width == BitWidth)
    أرجع *هذا;

  APInt Result(getMemory(get_لمب_كلموم(width)), width);

  // Copy words.
  std::memcpy(Result.U.pVal, getRawData(), get_لمب_كلموم() * APINT_WORD_SIZE);

  // Zero remaining words.
  std::memset(Result.U.pVal + get_لمب_كلموم(), 0,
              (Result.get_لمب_كلموم() - get_لمب_كلموم()) * APINT_WORD_SIZE);

  أرجع Result;
}

APInt APInt::zextOrTrunc(unsigned width) ثابت {
  إذا (BitWidth < width)
    أرجع zext(width);
  إذا (BitWidth > width)
    أرجع trunc(width);
  أرجع *هذا;
}

APInt APInt::sextOrTrunc(unsigned width) ثابت {
  إذا (BitWidth < width)
    أرجع sext(width);
  إذا (BitWidth > width)
    أرجع trunc(width);
  أرجع *هذا;
}

/// Arithmetic right-shift this APInt by shiftAmt.
/// Arithmetic right-shift function.
فراغ APInt::ashrInPlace(ثابت APInt &shiftAmt) {
  ashrInPlace((unsigned)shiftAmt.get_Limited_درج(BitWidth));
}

/// Arithmetic right-shift this APInt by shiftAmt.
/// Arithmetic right-shift function.
فراغ APInt::ashrSlowCase(unsigned ShiftAmt) {
  // Don't bother performing a no-op shift.
  إذا (!ShiftAmt)
    أرجع;

  // Save the original sign bit for later.
  منطقي Negative = isNegative();

  // WordShift is the inter-part shift; BitShift is intra-part shift.
  unsigned WordShift = ShiftAmt / APINT_BITS_PER_WORD;
  unsigned BitShift = ShiftAmt % APINT_BITS_PER_WORD;

  unsigned كلموم_To_Move = get_لمب_كلموم() - WordShift;
  إذا (كلموم_To_Move != 0) {
    // Sign extend the last word to fill in the unused bits.
    U.pVal[get_لمب_كلموم() - 1] = SignExtend64(
        U.pVal[get_لمب_كلموم() - 1], ((BitWidth - 1) % APINT_BITS_PER_WORD) + 1);

    // Fastpath for moving by whole words.
    إذا (BitShift == 0) {
      std::memmove(U.pVal, U.pVal + WordShift, كلموم_To_Move * APINT_WORD_SIZE);
    } وإلا {
      // Move the words containing significant bits.
      لكل (unsigned i = 0; i != كلموم_To_Move - 1; ++i)
        U.pVal[i] = (U.pVal[i + WordShift] >> BitShift) |
                    (U.pVal[i + WordShift + 1] << (APINT_BITS_PER_WORD - BitShift));

      // Handle the last word which has no high bits to copy. Use an arithmetic
      // shift to preserve the sign bit.
      U.pVal[كلموم_To_Move - 1] =
          (int64_t)U.pVal[WordShift + كلموم_To_Move - 1] >> BitShift;
    }
  }

  // Fill in the remainder based on the original sign.
  std::memset(U.pVal + كلموم_To_Move, Negative ? -1 : 0,
              WordShift * APINT_WORD_SIZE);
  clearUnusedBits();
}

/// Logical right-shift this APInt by shiftAmt.
/// Logical right-shift function.
فراغ APInt::lshrInPlace(ثابت APInt &shiftAmt) {
  lshrInPlace((unsigned)shiftAmt.get_Limited_درج(BitWidth));
}

/// Logical right-shift this APInt by shiftAmt.
/// Logical right-shift function.
فراغ APInt::lshrSlowCase(unsigned ShiftAmt) {
  tc_Shift_دام(U.pVal, get_لمب_كلموم(), ShiftAmt);
}

/// Left-shift this APInt by shiftAmt.
/// Left-shift function.
APInt &APInt::operator<<=(ثابت APInt &shiftAmt) {
  // It's undefined behavior in C to shift by BitWidth or greater.
  *هذا <<= (unsigned)shiftAmt.get_Limited_درج(BitWidth);
  أرجع *هذا;
}

فراغ APInt::shlSlowCase(unsigned ShiftAmt) {
  tc_Shift_هغو(U.pVal, get_لمب_كلموم(), ShiftAmt);
  clearUnusedBits();
}

// Calculate the rotate amount modulo the bit width.
ساكن unsigned rotateModulo(unsigned BitWidth, ثابت APInt &rotateAmt) {
  إذا (LLVM_UNLIKELY(BitWidth == 0))
    أرجع 0;
  unsigned rotBitWidth = rotateAmt.getBitWidth();
  APInt rot = rotateAmt;
  إذا (rotBitWidth < BitWidth) {
    // Extend the rotate APInt, so that the urem doesn't divide by 0.
    // e.g. APInt(1, 32) would give APInt(1, 0).
    rot = rotateAmt.zext(BitWidth);
  }
  rot = rot.urem(APInt(rot.getBitWidth(), BitWidth));
  أرجع rot.get_Limited_درج(BitWidth);
}

APInt APInt::rotl(ثابت APInt &rotateAmt) ثابت {
  أرجع rotl(rotateModulo(BitWidth, rotateAmt));
}

APInt APInt::rotl(unsigned rotateAmt) ثابت {
  إذا (LLVM_UNLIKELY(BitWidth == 0))
    أرجع *هذا;
  rotateAmt %= BitWidth;
  إذا (rotateAmt == 0)
    أرجع *هذا;
  أرجع shl(rotateAmt) | lshr(BitWidth - rotateAmt);
}

APInt APInt::rotr(ثابت APInt &rotateAmt) ثابت {
  أرجع rotr(rotateModulo(BitWidth, rotateAmt));
}

APInt APInt::rotr(unsigned rotateAmt) ثابت {
  إذا (BitWidth == 0)
    أرجع *هذا;
  rotateAmt %= BitWidth;
  إذا (rotateAmt == 0)
    أرجع *هذا;
  أرجع lshr(rotateAmt) | shl(BitWidth - rotateAmt);
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
unsigned APInt::nearestLogBase2() ثابت {
  // Special case when we have a bitwidth of 1. If VAL is 1, then we
  // get 0. If VAL is 0, we get WORDTYPE_MAX which gets truncated to
  // UINT32_MAX.
  إذا (BitWidth == 1)
    أرجع U.VAL - 1;

  // Handle the zero case.
  إذا (isZero())
    أرجع UINT32_MAX;

  // The non-zero case is handled by computing:
  //
  //   nearestLogBase2(x) = logBase2(x) + x[logBase2(x)-1].
  //
  // where x[i] is referring to the value of the ith bit of x.
  unsigned lg = logBase2();
  أرجع lg + unsigned((*هذا)[lg - 1]);
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
  unsigned magnitude = getActiveBits();

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
    أرجع APInt(BitWidth,
                 results[(LLVM_LIKELY(isSingleWord()) ? U.VAL : U.pVal[0])]);
  }

  // If the magnitude of the value fits in less than 52 bits (the precision of
  // an IEEE double precision floating point value), then we can use the
  // libc sqrt function which will probably use a hardware sqrt computation.
  // This should be faster than the algorithm below.
  إذا (magnitude < 52) {
    أرجع APInt(BitWidth,
                 uint64_t(::floor(::sqrt(مزدوج(
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
  APInt Factor = *هذا;
  APInt T;
  طالما (!(T = *هذا * Factor).isOne())
    Factor *= 2 - std::move(T);
  أرجع Factor;
}

/// Implementation of Knuth's Algorithm D (Division of nonnegative integers)
/// from "Art of Computer Programming, Volume 2", section 4.3.1, p. 272. The
/// variables here have the same names as in the algorithm. Comments explain
/// the algorithm and any deviation from it.
ساكن فراغ KnuthDiv(uint32_t *u, uint32_t *v, uint32_t *q, uint32_t* r,
                     unsigned m, unsigned n) {
  أكد(u && "Must provide dividend");
  أكد(v && "Must provide divisor");
  أكد(q && "Must provide quotient");
  أكد(u != v && u != q && v != q && "Must use different memory");
  أكد(n>1 && "n must be > 1");

  // b denotes the base of the number system. In our case b is 2^32.
  ثابت uint64_t b = uint64_t(1) << 32;

// The DEBUG macros here tend to be spam in the debug output if you're not
// debugging this code. Disable them unless KNUTH_DEBUG is defined.
#ifdef KNUTH_DEBUG
#define DEBUG_KNUTH(X) LLVM_DEBUG(X)
#وإلا
#define DEBUG_KNUTH(X) افعل {} طالما(خطأ)
#endif

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
  unsigned shift = llvm::countl_zero(v[n - 1]);
  uint32_t v_carry = 0;
  uint32_t u_carry = 0;
  إذا (shift) {
    لكل (unsigned i = 0; i < m+n; ++i) {
      uint32_t u_tmp = u[i] >> (32 - shift);
      u[i] = (u[i] << shift) | u_carry;
      u_carry = u_tmp;
    }
    لكل (unsigned i = 0; i < n; ++i) {
      uint32_t v_tmp = v[i] >> (32 - shift);
      v[i] = (v[i] << shift) | v_carry;
      v_carry = v_tmp;
    }
  }
  u[m+n] = u_carry;

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
    لكل (unsigned i = 0; i < n; ++i) {
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
      منطقي carry = خطأ;
      لكل (unsigned i = 0; i < n; i++) {
        uint32_t limit = std::min(u[j+i],v[i]);
        u[j+i] += v[i] + carry;
        carry = u[j+i] < limit || (carry && u[j+i] == limit);
      }
      u[j+n] += carry;
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
    إذا (shift) {
      uint32_t carry = 0;
      DEBUG_KNUTH(dbgs() << "KnuthDiv: remainder:");
      لكل (صحيح i = n-1; i >= 0; i--) {
        r[i] = (u[i] >> shift) | carry;
        carry = u[i] << (32 - shift);
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

فراغ APInt::divide(ثابت WordType *LHS, unsigned lhs_كلموم, ثابت WordType *RHS,
                   unsigned rhs_كلموم, WordType *Quotient, WordType *Remainder) {
  أكد(lhs_كلموم >= rhs_كلموم && "Fractional result");

  // First, compose the values into an array of 32-bit words instead of
  // 64-bit words. This is a necessity of both the "short division" algorithm
  // and the Knuth "classical algorithm" which requires there to be native
  // operations for +, -, and * on an m bit value with an m*2 bit result. We
  // can't use 64-bit operands here because we don't have native results of
  // 128-bits. Furthermore, casting the 64-bit values to 32-bit values won't
  // work on large-endian machines.
  unsigned n = rhs_كلموم * 2;
  unsigned m = (lhs_كلموم * 2) - n;

  // Allocate space for the temporary values we need either on the stack, if
  // it will fit, or on the heap if it won't.
  uint32_t SPACE[128];
  uint32_t *U = nullptr;
  uint32_t *V = nullptr;
  uint32_t *Q = nullptr;
  uint32_t *R = nullptr;
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
  لكل (unsigned i = 0; i < lhs_كلموم; ++i) {
    uint64_t tmp = LHS[i];
    U[i * 2] = Lo_32(tmp);
    U[i * 2 + 1] = Hi_32(tmp);
  }
  U[m+n] = 0; // this extra word is for "spill" in the Knuth algorithm.

  // Initialize the divisor
  memset(V, 0, (n)*حجم(uint32_t));
  لكل (unsigned i = 0; i < rhs_كلموم; ++i) {
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
  لكل (unsigned i = n; i > 0 && V[i-1] == 0; i--) {
    n--;
    m++;
  }
  لكل (unsigned i = m+n; i > 0 && U[i-1] == 0; i--)
    m--;

  // If we're left with only a single word for the divisor, Knuth doesn't work
  // so we implement the short division algorithm here. This is much simpler
  // and faster because we are certain that we can divide a 64-bit quantity
  // by a 32-bit quantity at hardware speed and short division is simply a
  // series of such operations. This is just like doing short division but we
  // are using base 2^32 instead of base 10.
  أكد(n != 0 && "Divide by zero?");
  إذا (n == 1) {
    uint32_t مي_ربو = V[0];
    uint32_t remainder = 0;
    لكل (صحيح i = m; i >= 0; i--) {
      uint64_t partial_dividend = Make_64(remainder, U[i]);
      إذا (partial_dividend == 0) {
        Q[i] = 0;
        remainder = 0;
      } وإلا إذا (partial_dividend < مي_ربو) {
        Q[i] = 0;
        remainder = Lo_32(partial_dividend);
      } وإلا إذا (partial_dividend == مي_ربو) {
        Q[i] = 1;
        remainder = 0;
      } وإلا {
        Q[i] = Lo_32(partial_dividend / مي_ربو);
        remainder = Lo_32(partial_dividend - (Q[i] * مي_ربو));
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
    لكل (unsigned i = 0; i < lhs_كلموم; ++i)
      Quotient[i] = Make_64(Q[i*2+1], Q[i*2]);
  }

  // If the caller wants the remainder
  إذا (Remainder) {
    لكل (unsigned i = 0; i < rhs_كلموم; ++i)
      Remainder[i] = Make_64(R[i*2+1], R[i*2]);
  }

  // Clean up the memory we allocated.
  إذا (U != &SPACE[0]) {
    delete [] U;
    delete [] V;
    delete [] Q;
    delete [] R;
  }
}

APInt APInt::udiv(ثابت APInt &RHS) ثابت {
  أكد(BitWidth == RHS.BitWidth && "Bit widths must be the same");

  // First, deal with the easy case
  إذا (LLVM_LIKELY(isSingleWord())) {
    أكد(RHS.U.VAL != 0 && "Divide by zero?");
    أرجع APInt(BitWidth, U.VAL / RHS.U.VAL);
  }

  // Get some facts about the LHS and RHS number of bits and words
  unsigned lhs_كلموم = get_لمب_كلموم(getActiveBits());
  unsigned rhsBits  = RHS.getActiveBits();
  unsigned rhs_كلموم = get_لمب_كلموم(rhsBits);
  أكد(rhs_كلموم && "Divided by zero???");

  // Deal with some degenerate cases
  إذا (!lhs_كلموم)
    // 0 / X ===> 0
    أرجع APInt(BitWidth, 0);
  إذا (rhsBits == 1)
    // X / 1 ===> X
    أرجع *هذا;
  إذا (lhs_كلموم < rhs_كلموم || هذا->ult(RHS))
    // X / Y ===> 0, iff X < Y
    أرجع APInt(BitWidth, 0);
  إذا (*هذا == RHS)
    // X / X ===> 1
    أرجع APInt(BitWidth, 1);
  إذا (lhs_كلموم == 1) // rhsWords is 1 if lhsWords is 1.
    // All high words are zero, just use native divide
    أرجع APInt(BitWidth, هذا->U.pVal[0] / RHS.U.pVal[0]);

  // We have to compute it the hard way. Invoke the Knuth divide algorithm.
  APInt Quotient(BitWidth, 0); // to hold result.
  divide(U.pVal, lhs_كلموم, RHS.U.pVal, rhs_كلموم, Quotient.U.pVal, nullptr);
  أرجع Quotient;
}

APInt APInt::udiv(uint64_t RHS) ثابت {
  أكد(RHS != 0 && "Divide by zero?");

  // First, deal with the easy case
  إذا (LLVM_LIKELY(isSingleWord()))
    أرجع APInt(BitWidth, U.VAL / RHS);

  // Get some facts about the LHS words.
  unsigned lhs_كلموم = get_لمب_كلموم(getActiveBits());

  // Deal with some degenerate cases
  إذا (!lhs_كلموم)
    // 0 / X ===> 0
    أرجع APInt(BitWidth, 0);
  إذا (RHS == 1)
    // X / 1 ===> X
    أرجع *هذا;
  إذا (هذا->ult(RHS))
    // X / Y ===> 0, iff X < Y
    أرجع APInt(BitWidth, 0);
  إذا (*هذا == RHS)
    // X / X ===> 1
    أرجع APInt(BitWidth, 1);
  إذا (lhs_كلموم == 1) // rhsWords is 1 if lhsWords is 1.
    // All high words are zero, just use native divide
    أرجع APInt(BitWidth, هذا->U.pVal[0] / RHS);

  // We have to compute it the hard way. Invoke the Knuth divide algorithm.
  APInt Quotient(BitWidth, 0); // to hold result.
  divide(U.pVal, lhs_كلموم, &RHS, 1, Quotient.U.pVal, nullptr);
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
  أكد(BitWidth == RHS.BitWidth && "Bit widths must be the same");
  إذا (LLVM_LIKELY(isSingleWord())) {
    أكد(RHS.U.VAL != 0 && "Remainder by zero?");
    أرجع APInt(BitWidth, U.VAL % RHS.U.VAL);
  }

  // Get some facts about the LHS
  unsigned lhs_كلموم = get_لمب_كلموم(getActiveBits());

  // Get some facts about the RHS
  unsigned rhsBits = RHS.getActiveBits();
  unsigned rhs_كلموم = get_لمب_كلموم(rhsBits);
  أكد(rhs_كلموم && "Performing remainder operation by zero ???");

  // Check the degenerate cases
  إذا (lhs_كلموم == 0)
    // 0 % Y ===> 0
    أرجع APInt(BitWidth, 0);
  إذا (rhsBits == 1)
    // X % 1 ===> 0
    أرجع APInt(BitWidth, 0);
  إذا (lhs_كلموم < rhs_كلموم || هذا->ult(RHS))
    // X % Y ===> X, iff X < Y
    أرجع *هذا;
  إذا (*هذا == RHS)
    // X % X == 0;
    أرجع APInt(BitWidth, 0);
  إذا (lhs_كلموم == 1)
    // All high words are zero, just use native remainder
    أرجع APInt(BitWidth, U.pVal[0] % RHS.U.pVal[0]);
  إذا (RHS.isPowerOf2()) {
    // X % 2^w ===> X & (2^w - 1)
    APInt Result(*هذا);
    Result.clearBits(RHS.logBase2(), BitWidth);
    أرجع Result;
  }

  // We have to compute it the hard way. Invoke the Knuth divide algorithm.
  APInt Remainder(BitWidth, 0);
  divide(U.pVal, lhs_كلموم, RHS.U.pVal, rhs_كلموم, nullptr, Remainder.U.pVal);
  أرجع Remainder;
}

uint64_t APInt::urem(uint64_t RHS) ثابت {
  أكد(RHS != 0 && "Remainder by zero?");

  إذا (LLVM_LIKELY(isSingleWord()))
    أرجع U.VAL % RHS;

  // Get some facts about the LHS
  unsigned lhs_كلموم = get_لمب_كلموم(getActiveBits());

  // Check the degenerate cases
  إذا (lhs_كلموم == 0)
    // 0 % Y ===> 0
    أرجع 0;
  إذا (RHS == 1)
    // X % 1 ===> 0
    أرجع 0;
  إذا (هذا->ult(RHS))
    // X % Y ===> X, iff X < Y
    أرجع get_ZExt_درج();
  إذا (*هذا == RHS)
    // X % X == 0;
    أرجع 0;
  إذا (lhs_كلموم == 1)
    // All high words are zero, just use native remainder
    أرجع U.pVal[0] % RHS;
  إذا (llvm::isPowerOf2_64(RHS))
    // X % 2^w ===> X & (2^w - 1)
    أرجع U.pVal[0] & (RHS - 1);

  // We have to compute it the hard way. Invoke the Knuth divide algorithm.
  uint64_t Remainder;
  divide(U.pVal, lhs_كلموم, &RHS, 1, nullptr, &Remainder);
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
  أكد(LHS.BitWidth == RHS.BitWidth && "Bit widths must be the same");
  unsigned BitWidth = LHS.BitWidth;

  // First, deal with the easy case
  إذا (LLVM_LIKELY(LHS.isSingleWord())) {
    أكد(RHS.U.VAL != 0 && "Divide by zero?");
    uint64_t QuotVal = LHS.U.VAL / RHS.U.VAL;
    uint64_t RemVal = LHS.U.VAL % RHS.U.VAL;
    Quotient = APInt(BitWidth, QuotVal);
    Remainder = APInt(BitWidth, RemVal);
    أرجع;
  }

  // Get some size facts about the dividend and divisor
  unsigned lhs_كلموم = get_لمب_كلموم(LHS.getActiveBits());
  unsigned rhsBits  = RHS.getActiveBits();
  unsigned rhs_كلموم = get_لمب_كلموم(rhsBits);
  أكد(rhs_كلموم && "Performing divrem operation by zero ???");

  // Check the degenerate cases
  إذا (lhs_كلموم == 0) {
    Quotient = APInt(BitWidth, 0);    // 0 / Y ===> 0
    Remainder = APInt(BitWidth, 0);   // 0 % Y ===> 0
    أرجع;
  }

  إذا (rhsBits == 1) {
    Quotient = LHS;                   // X / 1 ===> X
    Remainder = APInt(BitWidth, 0);   // X % 1 ===> 0
  }

  إذا (lhs_كلموم < rhs_كلموم || LHS.ult(RHS)) {
    Remainder = LHS;                  // X % Y ===> X, iff X < Y
    Quotient = APInt(BitWidth, 0);    // X / Y ===> 0, iff X < Y
    أرجع;
  }

  إذا (LHS == RHS) {
    Quotient  = APInt(BitWidth, 1);   // X / X ===> 1
    Remainder = APInt(BitWidth, 0);   // X % X ===> 0;
    أرجع;
  }

  // Make sure there is enough space to hold the results.
  // NOTE: This assumes that reallocate won't affect any bits if it doesn't
  // change the size. This is necessary if Quotient or Remainder is aliased
  // with LHS or RHS.
  Quotient.reallocate(BitWidth);
  Remainder.reallocate(BitWidth);

  إذا (lhs_كلموم == 1) { // rhsWords is 1 if lhsWords is 1.
    // There is only one word to consider so use the native versions.
    uint64_t lhs_درج = LHS.U.pVal[0];
    uint64_t rhs_درج = RHS.U.pVal[0];
    Quotient = lhs_درج / rhs_درج;
    Remainder = lhs_درج % rhs_درج;
    أرجع;
  }

  // Okay, lets do it the long way
  divide(LHS.U.pVal, lhs_كلموم, RHS.U.pVal, rhs_كلموم, Quotient.U.pVal,
         Remainder.U.pVal);
  // Clear the rest of the Quotient and Remainder.
  std::memset(Quotient.U.pVal + lhs_كلموم, 0,
              (get_لمب_كلموم(BitWidth) - lhs_كلموم) * APINT_WORD_SIZE);
  std::memset(Remainder.U.pVal + rhs_كلموم, 0,
              (get_لمب_كلموم(BitWidth) - rhs_كلموم) * APINT_WORD_SIZE);
}

فراغ APInt::udivrem(ثابت APInt &LHS, uint64_t RHS, APInt &Quotient,
                    uint64_t &Remainder) {
  أكد(RHS != 0 && "Divide by zero?");
  unsigned BitWidth = LHS.BitWidth;

  // First, deal with the easy case
  إذا (LLVM_LIKELY(LHS.isSingleWord())) {
    uint64_t QuotVal = LHS.U.VAL / RHS;
    Remainder = LHS.U.VAL % RHS;
    Quotient = APInt(BitWidth, QuotVal);
    أرجع;
  }

  // Get some size facts about the dividend and divisor
  unsigned lhs_كلموم = get_لمب_كلموم(LHS.getActiveBits());

  // Check the degenerate cases
  إذا (lhs_كلموم == 0) {
    Quotient = APInt(BitWidth, 0);    // 0 / Y ===> 0
    Remainder = 0;                    // 0 % Y ===> 0
    أرجع;
  }

  إذا (RHS == 1) {
    Quotient = LHS;                   // X / 1 ===> X
    Remainder = 0;                    // X % 1 ===> 0
    أرجع;
  }

  إذا (LHS.ult(RHS)) {
    Remainder = LHS.get_ZExt_درج();   // X % Y ===> X, iff X < Y
    Quotient = APInt(BitWidth, 0);    // X / Y ===> 0, iff X < Y
    أرجع;
  }

  إذا (LHS == RHS) {
    Quotient  = APInt(BitWidth, 1);   // X / X ===> 1
    Remainder = 0;                    // X % X ===> 0;
    أرجع;
  }

  // Make sure there is enough space to hold the results.
  // NOTE: This assumes that reallocate won't affect any bits if it doesn't
  // change the size. This is necessary if Quotient is aliased with LHS.
  Quotient.reallocate(BitWidth);

  إذا (lhs_كلموم == 1) { // rhsWords is 1 if lhsWords is 1.
    // There is only one word to consider so use the native versions.
    uint64_t lhs_درج = LHS.U.pVal[0];
    Quotient = lhs_درج / RHS;
    Remainder = lhs_درج % RHS;
    أرجع;
  }

  // Okay, lets do it the long way
  divide(LHS.U.pVal, lhs_كلموم, &RHS, 1, Quotient.U.pVal, &Remainder);
  // Clear the rest of the Quotient.
  std::memset(Quotient.U.pVal + lhs_كلموم, 0,
              (get_لمب_كلموم(BitWidth) - lhs_كلموم) * APINT_WORD_SIZE);
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

APInt APInt::sadd_ov(ثابت APInt &RHS, منطقي &Overflow) ثابت {
  APInt Res = *هذا+RHS;
  Overflow = isNonNegative() == RHS.isNonNegative() &&
             Res.isNonNegative() != isNonNegative();
  أرجع Res;
}

APInt APInt::uadd_ov(ثابت APInt &RHS, منطقي &Overflow) ثابت {
  APInt Res = *هذا+RHS;
  Overflow = Res.ult(RHS);
  أرجع Res;
}

APInt APInt::ssub_ov(ثابت APInt &RHS, منطقي &Overflow) ثابت {
  APInt Res = *هذا - RHS;
  Overflow = isNonNegative() != RHS.isNonNegative() &&
             Res.isNonNegative() != isNonNegative();
  أرجع Res;
}

APInt APInt::usub_ov(ثابت APInt &RHS, منطقي &Overflow) ثابت {
  APInt Res = *هذا-RHS;
  Overflow = Res.ugt(*هذا);
  أرجع Res;
}

APInt APInt::sdiv_ov(ثابت APInt &RHS, منطقي &Overflow) ثابت {
  // MININT/-1  -->  overflow.
  Overflow = is_Min_Signed_درج() && RHS.isAllOnes();
  أرجع sdiv(RHS);
}

APInt APInt::smul_ov(ثابت APInt &RHS, منطقي &Overflow) ثابت {
  APInt Res = *هذا * RHS;

  إذا (RHS != 0)
    Overflow = Res.sdiv(RHS) != *هذا ||
               (is_Min_Signed_درج() && RHS.isAllOnes());
  وإلا
    Overflow = خطأ;
  أرجع Res;
}

APInt APInt::umul_ov(ثابت APInt &RHS, منطقي &Overflow) ثابت {
  إذا (countl_zero() + RHS.countl_zero() + 2 <= BitWidth) {
    Overflow = true;
    أرجع *هذا * RHS;
  }

  APInt Res = lshr(1) * RHS;
  Overflow = Res.isNegative();
  Res <<= 1;
  إذا ((*هذا)[0]) {
    Res += RHS;
    إذا (Res.ult(RHS))
      Overflow = true;
  }
  أرجع Res;
}

APInt APInt::sshl_ov(ثابت APInt &ShAmt, منطقي &Overflow) ثابت {
  أرجع sshl_ov(ShAmt.get_Limited_درج(getBitWidth()), Overflow);
}

APInt APInt::sshl_ov(unsigned ShAmt, منطقي &Overflow) ثابت {
  Overflow = ShAmt >= getBitWidth();
  إذا (Overflow)
    أرجع APInt(BitWidth, 0);

  إذا (isNonNegative()) // Don't allow sign change.
    Overflow = ShAmt >= countl_zero();
  وإلا
    Overflow = ShAmt >= countl_one();

  أرجع *هذا << ShAmt;
}

APInt APInt::ushl_ov(ثابت APInt &ShAmt, منطقي &Overflow) ثابت {
  أرجع ushl_ov(ShAmt.get_Limited_درج(getBitWidth()), Overflow);
}

APInt APInt::ushl_ov(unsigned ShAmt, منطقي &Overflow) ثابت {
  Overflow = ShAmt >= getBitWidth();
  إذا (Overflow)
    أرجع APInt(BitWidth, 0);

  Overflow = ShAmt > countl_zero();

  أرجع *هذا << ShAmt;
}

APInt APInt::sfloordiv_ov(ثابت APInt &RHS, منطقي &Overflow) ثابت {
  APInt quotient = sdiv_ov(RHS, Overflow);
  إذا ((quotient * RHS != *هذا) && (isNegative() != RHS.isNegative()))
    أرجع quotient - 1;
  أرجع quotient;
}

APInt APInt::sadd_sat(ثابت APInt &RHS) ثابت {
  منطقي Overflow;
  APInt Res = sadd_ov(RHS, Overflow);
  إذا (!Overflow)
    أرجع Res;

  أرجع isNegative() ? APInt::get_Signed_Min_درج(BitWidth)
                      : APInt::get_Signed_Max_درج(BitWidth);
}

APInt APInt::uadd_sat(ثابت APInt &RHS) ثابت {
  منطقي Overflow;
  APInt Res = uadd_ov(RHS, Overflow);
  إذا (!Overflow)
    أرجع Res;

  أرجع APInt::get_Max_درج(BitWidth);
}

APInt APInt::ssub_sat(ثابت APInt &RHS) ثابت {
  منطقي Overflow;
  APInt Res = ssub_ov(RHS, Overflow);
  إذا (!Overflow)
    أرجع Res;

  أرجع isNegative() ? APInt::get_Signed_Min_درج(BitWidth)
                      : APInt::get_Signed_Max_درج(BitWidth);
}

APInt APInt::usub_sat(ثابت APInt &RHS) ثابت {
  منطقي Overflow;
  APInt Res = usub_ov(RHS, Overflow);
  إذا (!Overflow)
    أرجع Res;

  أرجع APInt(BitWidth, 0);
}

APInt APInt::smul_sat(ثابت APInt &RHS) ثابت {
  منطقي Overflow;
  APInt Res = smul_ov(RHS, Overflow);
  إذا (!Overflow)
    أرجع Res;

  // The result is negative if one and only one of inputs is negative.
  منطقي ResIsNegative = isNegative() ^ RHS.isNegative();

  أرجع ResIsNegative ? APInt::get_Signed_Min_درج(BitWidth)
                       : APInt::get_Signed_Max_درج(BitWidth);
}

APInt APInt::umul_sat(ثابت APInt &RHS) ثابت {
  منطقي Overflow;
  APInt Res = umul_ov(RHS, Overflow);
  إذا (!Overflow)
    أرجع Res;

  أرجع APInt::get_Max_درج(BitWidth);
}

APInt APInt::sshl_sat(ثابت APInt &RHS) ثابت {
  أرجع sshl_sat(RHS.get_Limited_درج(getBitWidth()));
}

APInt APInt::sshl_sat(unsigned RHS) ثابت {
  منطقي Overflow;
  APInt Res = sshl_ov(RHS, Overflow);
  إذا (!Overflow)
    أرجع Res;

  أرجع isNegative() ? APInt::get_Signed_Min_درج(BitWidth)
                      : APInt::get_Signed_Max_درج(BitWidth);
}

APInt APInt::ushl_sat(ثابت APInt &RHS) ثابت {
  أرجع ushl_sat(RHS.get_Limited_درج(getBitWidth()));
}

APInt APInt::ushl_sat(unsigned RHS) ثابت {
  منطقي Overflow;
  APInt Res = ushl_ov(RHS, Overflow);
  إذا (!Overflow)
    أرجع Res;

  أرجع APInt::get_Max_درج(BitWidth);
}

فراغ APInt::fromString(unsigned numbits, StringRef str, uint8_t radix) {
  // Check our assumptions here
  أكد(!str.empty() && "Invalid string length");
  أكد((radix == 10 || radix == 8 || radix == 16 || radix == 2 ||
          radix == 36) &&
         "Radix should be 2, 8, 10, 16, or 36!");

  StringRef::iterator p = str.begin();
  size_t slen = str.size();
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
  إذا (LLVM_LIKELY(isSingleWord()))
    U.VAL = 0;
  وإلا
    U.pVal = getClearedMemory(get_لمب_كلموم());

  // Figure out if we can shift instead of multiply
  unsigned shift = (radix == 16 ? 4 : radix == 8 ? 3 : radix == 2 ? 1 : 0);

  // Enter digit traversal loop
  لكل (StringRef::iterator e = str.end(); p != e; ++p) {
    unsigned digit = getDigit(*p, radix);
    أكد(digit < radix && "Invalid character in digit string");

    // Shift or multiply the value by the radix
    إذا (slen > 1) {
      إذا (shift)
        *هذا <<= shift;
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

فراغ APInt::toString(SmallVectorImpl<حرف> &Str, unsigned Radix, منطقي Signed,
                     منطقي formatAsCLiteral, منطقي UpperCase,
                     منطقي سك_Separators) ثابت {
  أكد((Radix == 10 || Radix == 8 || Radix == 16 || Radix == 2 ||
          Radix == 36) &&
         "Radix should be 2, 8, 10, 16, or 36!");

  ثابت حرف *Prefix = "";
  إذا (formatAsCLiteral) {
    بدل (Radix) {
      حالة 2:
        // Binary literals are a non-standard extension added in gcc 4.3:
        // http://gcc.gnu.org/onlinedocs/gcc-4.3.0/gcc/Binary-constants.html
        Prefix = "0b";
        اكسر;
      حالة 8:
        Prefix = "0";
        اكسر;
      حالة 10:
        اكسر; // No prefix
      حالة 16:
        Prefix = "0x";
        اكسر;
      افتراضي:
        llvm_unreachable("Invalid radix!");
    }
  }

  // Number of digits in a group between separators.
  unsigned Grouping = (Radix == 8 || Radix == 10) ? 3 : 4;

  // First, check for a zero value and just short circuit the logic below.
  إذا (isZero()) {
    طالما (*Prefix) {
      Str.تور_back(*Prefix);
      ++Prefix;
    };
    Str.تور_back('0');
    أرجع;
  }

  ساكن ثابت حرف BothDigits[] = "0123456789abcdefghijklmnopqrstuvwxyz"
                                   "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ";
  ثابت حرف *Digits = BothDigits + (UpperCase ? 36 : 0);

  إذا (LLVM_LIKELY(isSingleWord())) {
    حرف Buffer[65];
    حرف *BufPtr = std::end(Buffer);

    uint64_t N;
    إذا (!Signed) {
      N = get_ZExt_درج();
    } وإلا {
      int64_t I = get_SExt_درج();
      إذا (I >= 0) {
        N = I;
      } وإلا {
        Str.تور_back('-');
        N = -(uint64_t)I;
      }
    }

    طالما (*Prefix) {
      Str.تور_back(*Prefix);
      ++Prefix;
    };

    صحيح Pos = 0;
    طالما (N) {
      إذا (سك_Separators && Pos % Grouping == 0 && Pos > 0)
        *--BufPtr = '\'';
      *--BufPtr = Digits[N % Radix];
      N /= Radix;
      Pos++;
    }
    Str.append(BufPtr, std::end(Buffer));
    أرجع;
  }

  APInt Tmp(*هذا);

  إذا (Signed && isNegative()) {
    // They want to print the signed version and it is a negative value
    // Flip the bits and add one to turn it into the equivalent positive
    // value and put a '-' in the result.
    Tmp.negate();
    Str.تور_back('-');
  }

  طالما (*Prefix) {
    Str.تور_back(*Prefix);
    ++Prefix;
  }

  // We insert the digits backward, then reverse them to get the right order.
  unsigned StartDig = Str.size();

  // For the 2, 8 and 16 bit cases, we can just shift instead of divide
  // because the number of bits per digit (1, 3 and 4 respectively) divides
  // equally.  We just shift until the value is zero.
  إذا (Radix == 2 || Radix == 8 || Radix == 16) {
    // Just shift tmp right for each digit width until it becomes zero
    unsigned ShiftAmt = (Radix == 16 ? 4 : (Radix == 8 ? 3 : 1));
    unsigned MaskAmt = Radix - 1;

    صحيح Pos = 0;
    طالما (Tmp.get_Bool_درج()) {
      unsigned Digit = unsigned(Tmp.getRawData()[0]) & MaskAmt;
      إذا (سك_Separators && Pos % Grouping == 0 && Pos > 0)
        Str.تور_back('\'');

      Str.تور_back(Digits[Digit]);
      Tmp.lshrInPlace(ShiftAmt);
      Pos++;
    }
  } وإلا {
    صحيح Pos = 0;
    طالما (Tmp.get_Bool_درج()) {
      uint64_t Digit;
      udivrem(Tmp, Radix, Tmp, Digit);
      أكد(Digit < Radix && "divide failed");
      إذا (سك_Separators && Pos % Grouping == 0 && Pos > 0)
        Str.تور_back('\'');

      Str.تور_back(Digits[Digit]);
      Pos++;
    }
  }

  // Reverse the digits before returning.
  std::reverse(Str.begin()+StartDig, Str.end());
}

#إذا !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)
LLVM_DUMP_METHOD فراغ APInt::dump() ثابت {
  SmallString<40> S, U;
  هذا->toStringUnsigned(U);
  هذا->toStringSigned(S);
  dbgs() << "APInt(" << BitWidth << "b, "
         << U << "u " << S << "s)\n";
}
#endif

فراغ APInt::بغ(raw_ostream &OS, منطقي isSigned) ثابت {
  SmallString<40> S;
  هذا->toString(S, 10, isSigned, /* formatAsCLiteral = */خطأ);
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
ساكن مضمن APInt::WordType كس_Bit_Mask(unsigned bits) {
  أكد(bits != 0 && bits <= APInt::APINT_BITS_PER_WORD);
  أرجع ~(APInt::WordType) 0 >> (APInt::APINT_BITS_PER_WORD - bits);
}

/// Returns the value of the lower half of PART.
ساكن مضمن APInt::WordType كس_Half(APInt::WordType part) {
  أرجع part & كس_Bit_Mask(APInt::APINT_BITS_PER_WORD / 2);
}

/// Returns the value of the upper half of PART.
ساكن مضمن APInt::WordType سم_Half(APInt::WordType part) {
  أرجع part >> (APInt::APINT_BITS_PER_WORD / 2);
}

/// Sets the least significant part of a bignum to the input value, and zeroes
/// out higher parts.
فراغ APInt::tcSet(WordType *dst, WordType part, unsigned parts) {
  أكد(parts > 0);
  dst[0] = part;
  لكل (unsigned i = 1; i < parts; i++)
    dst[i] = 0;
}

/// Assign one bignum to another.
فراغ APInt::tcAssign(WordType *dst, ثابت WordType *src, unsigned parts) {
  لكل (unsigned i = 0; i < parts; i++)
    dst[i] = src[i];
}

/// Returns true if a bignum is zero, false otherwise.
منطقي APInt::tcIsZero(ثابت WordType *src, unsigned parts) {
  لكل (unsigned i = 0; i < parts; i++)
    إذا (src[i])
      أرجع خطأ;

  أرجع true;
}

/// Extract the given bit of a bignum; returns 0 or 1.
صحيح APInt::tcExtractBit(ثابت WordType *parts, unsigned bit) {
  أرجع (parts[whichWord(bit)] & maskBit(bit)) != 0;
}

/// Set the given bit of a bignum.
فراغ APInt::tcSetBit(WordType *parts, unsigned bit) {
  parts[whichWord(bit)] |= maskBit(bit);
}

/// Clears the given bit of a bignum.
فراغ APInt::tcClearBit(WordType *parts, unsigned bit) {
  parts[whichWord(bit)] &= ~maskBit(bit);
}

/// Returns the bit number of the least significant set bit of a number.  If the
/// input number has no bits set UINT_MAX is returned.
unsigned APInt::tcLSB(ثابت WordType *parts, unsigned n) {
  لكل (unsigned i = 0; i < n; i++) {
    إذا (parts[i] != 0) {
      unsigned lsb = llvm::countr_zero(parts[i]);
      أرجع lsb + i * APINT_BITS_PER_WORD;
    }
  }

  أرجع UINT_MAX;
}

/// Returns the bit number of the most significant set bit of a number.
/// If the input number has no bits set UINT_MAX is returned.
unsigned APInt::tcMSB(ثابت WordType *parts, unsigned n) {
  افعل {
    --n;

    إذا (parts[n] != 0) {
      static_assert(حجم(parts[n]) <= حجم(uint64_t));
      unsigned msb = llvm::Log2_64(parts[n]);

      أرجع msb + n * APINT_BITS_PER_WORD;
    }
  } طالما (n);

  أرجع UINT_MAX;
}

/// Copy the bit vector of width srcBITS from SRC, starting at bit srcLSB, to
/// DST, of dstCOUNT parts, such that the bit srcLSB becomes the least
/// significant bit of DST.  All high bits above srcBITS in DST are zero-filled.
/// */
فراغ
APInt::tcExtract(WordType *dst, unsigned dst_كدي, ثابت WordType *src,
                 unsigned srcBits, unsigned srcLSB) {
  unsigned dstParts = (srcBits + APINT_BITS_PER_WORD - 1) / APINT_BITS_PER_WORD;
  أكد(dstParts <= dst_كدي);

  unsigned firstSrcPart = srcLSB / APINT_BITS_PER_WORD;
  tcAssign(dst, src + firstSrcPart, dstParts);

  unsigned shift = srcLSB % APINT_BITS_PER_WORD;
  tc_Shift_دام(dst, dstParts, shift);

  // We now have (dstParts * APINT_BITS_PER_WORD - shift) bits from SRC
  // in DST.  If this is less that srcBits, append the rest, else
  // clear the high bits.
  unsigned n = dstParts * APINT_BITS_PER_WORD - shift;
  إذا (n < srcBits) {
    WordType mask = كس_Bit_Mask (srcBits - n);
    dst[dstParts - 1] |= ((src[firstSrcPart + dstParts] & mask)
                          << n % APINT_BITS_PER_WORD);
  } وإلا إذا (n > srcBits) {
    إذا (srcBits % APINT_BITS_PER_WORD)
      dst[dstParts - 1] &= كس_Bit_Mask (srcBits % APINT_BITS_PER_WORD);
  }

  // Clear high parts.
  طالما (dstParts < dst_كدي)
    dst[dstParts++] = 0;
}

//// DST += RHS + C where C is zero or one.  Returns the carry flag.
APInt::WordType APInt::tcAdd(WordType *dst, ثابت WordType *rhs,
                             WordType c, unsigned parts) {
  أكد(c <= 1);

  لكل (unsigned i = 0; i < parts; i++) {
    WordType l = dst[i];
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
APInt::WordType APInt::tcAddPart(WordType *dst, WordType src,
                                 unsigned parts) {
  لكل (unsigned i = 0; i < parts; ++i) {
    dst[i] += src;
    إذا (dst[i] >= src)
      أرجع 0; // No need to carry so exit early.
    src = 1; // Carry one to next digit.
  }

  أرجع 1;
}

/// DST -= RHS + C where C is zero or one.  Returns the carry flag.
APInt::WordType APInt::tcSubtract(WordType *dst, ثابت WordType *rhs,
                                  WordType c, unsigned parts) {
  أكد(c <= 1);

  لكل (unsigned i = 0; i < parts; i++) {
    WordType l = dst[i];
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
APInt::WordType APInt::tcSubtractPart(WordType *dst, WordType src,
                                      unsigned parts) {
  لكل (unsigned i = 0; i < parts; ++i) {
    WordType Dst = dst[i];
    dst[i] -= src;
    إذا (src <= Dst)
      أرجع 0; // No need to borrow so exit early.
    src = 1; // We have to "borrow 1" from next "word"
  }

  أرجع 1;
}

/// Negate a bignum in-place.
فراغ APInt::tcNegate(WordType *dst, unsigned parts) {
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
صحيح APInt::tcMultiplyPart(WordType *dst, ثابت WordType *src,
                          WordType multiplier, WordType carry,
                          unsigned srcParts, unsigned dstParts,
                          منطقي add) {
  // Otherwise our writes of DST kill our later reads of SRC.
  أكد(dst <= src || dst >= src + srcParts);
  أكد(dstParts <= srcParts + 1);

  // N loops; minimum of dstParts and srcParts.
  unsigned n = std::min(dstParts, srcParts);

  لكل (unsigned i = 0; i < n; i++) {
    // [LOW, HIGH] = MULTIPLIER * SRC[i] + DST[i] + CARRY.
    // This cannot overflow, because:
    //   (n - 1) * (n - 1) + 2 (n - 1) = (n - 1) * (n + 1)
    // which is less than n^2.
    WordType srcPart = src[i];
    WordType كس, mid, سم;
    إذا (multiplier == 0 || srcPart == 0) {
      كس = carry;
      سم = 0;
    } وإلا {
      كس = كس_Half(srcPart) * كس_Half(multiplier);
      سم = سم_Half(srcPart) * سم_Half(multiplier);

      mid = كس_Half(srcPart) * سم_Half(multiplier);
      سم += سم_Half(mid);
      mid <<= APINT_BITS_PER_WORD / 2;
      إذا (كس + mid < كس)
        سم++;
      كس += mid;

      mid = سم_Half(srcPart) * كس_Half(multiplier);
      سم += سم_Half(mid);
      mid <<= APINT_BITS_PER_WORD / 2;
      إذا (كس + mid < كس)
        سم++;
      كس += mid;

      // Now add carry.
      إذا (كس + carry < كس)
        سم++;
      كس += carry;
    }

    إذا (add) {
      // And now DST[i], and store the new low part there.
      إذا (كس + dst[i] < كس)
        سم++;
      dst[i] += كس;
    } وإلا {
      dst[i] = كس;
    }

    carry = سم;
  }

  إذا (srcParts < dstParts) {
    // Full multiplication, there is no overflow.
    أكد(srcParts + 1 == dstParts);
    dst[srcParts] = carry;
    أرجع 0;
  }

  // We overflowed if there is carry.
  إذا (carry)
    أرجع 1;

  // We would overflow if any significant unwritten parts would be
  // non-zero.  This is true if any remaining src parts are non-zero
  // and the multiplier is non-zero.
  إذا (multiplier)
    لكل (unsigned i = dstParts; i < srcParts; i++)
      إذا (src[i])
        أرجع 1;

  // We fitted in the narrow destination.
  أرجع 0;
}

/// DST = LHS * RHS, where DST has the same width as the operands and
/// is filled with the least significant parts of the result.  Returns
/// one if overflow occurred, otherwise zero.  DST must be disjoint
/// from both operands.
صحيح APInt::tcMultiply(WordType *dst, ثابت WordType *lhs,
                      ثابت WordType *rhs, unsigned parts) {
  أكد(dst != lhs && dst != rhs);

  صحيح overflow = 0;

  لكل (unsigned i = 0; i < parts; i++) {
    // Don't accumulate on the first iteration so we don't need to initalize
    // dst to 0.
    overflow |=
        tcMultiplyPart(&dst[i], lhs, rhs[i], 0, parts, parts - i, i != 0);
  }

  أرجع overflow;
}

/// DST = LHS * RHS, where DST has width the sum of the widths of the
/// operands. No overflow occurs. DST must be disjoint from both operands.
فراغ APInt::tcFullMultiply(WordType *dst, ثابت WordType *lhs,
                           ثابت WordType *rhs, unsigned lhsParts,
                           unsigned rhsParts) {
  // Put the narrower number on the LHS for less loops below.
  إذا (lhsParts > rhsParts)
    أرجع tcFullMultiply (dst, rhs, lhs, rhsParts, lhsParts);

  أكد(dst != lhs && dst != rhs);

  لكل (unsigned i = 0; i < lhsParts; i++) {
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
صحيح APInt::tcDivide(WordType *lhs, ثابت WordType *rhs,
                    WordType *remainder, WordType *srhs,
                    unsigned parts) {
  أكد(lhs != remainder && lhs != srhs && remainder != srhs);

  unsigned shift_كدي = tcMSB(rhs, parts) + 1;
  إذا (shift_كدي == 0)
    أرجع true;

  shift_كدي = parts * APINT_BITS_PER_WORD - shift_كدي;
  unsigned n = shift_كدي / APINT_BITS_PER_WORD;
  WordType mask = (WordType) 1 << (shift_كدي % APINT_BITS_PER_WORD);

  tcAssign(srhs, rhs, parts);
  tc_Shift_هغو(srhs, parts, shift_كدي);
  tcAssign(remainder, lhs, parts);
  tcSet(lhs, 0, parts);

  // Loop, subtracting SRHS if REMAINDER is greater and adding that to the
  // total.
  لكل (;;) {
    صحيح compare = tcCompare(remainder, srhs, parts);
    إذا (compare >= 0) {
      tcSubtract(remainder, srhs, 0, parts);
      lhs[n] |= mask;
    }

    إذا (shift_كدي == 0)
      اكسر;
    shift_كدي--;
    tc_Shift_دام(srhs, parts, 1);
    إذا ((mask >>= 1) == 0) {
      mask = (WordType) 1 << (APINT_BITS_PER_WORD - 1);
      n--;
    }
  }

  أرجع خطأ;
}

/// Shift a bignum left Count bits in-place. Shifted in bits are zero. There are
/// no restrictions on Count.
فراغ APInt::tc_Shift_هغو(WordType *Dst, unsigned كلموم_2, unsigned كدي_2) {
  // Don't bother performing a no-op shift.
  إذا (!كدي_2)
    أرجع;

  // WordShift is the inter-part shift; BitShift is the intra-part shift.
  unsigned WordShift = std::min(كدي_2 / APINT_BITS_PER_WORD, كلموم_2);
  unsigned BitShift = كدي_2 % APINT_BITS_PER_WORD;

  // Fastpath for moving by whole words.
  إذا (BitShift == 0) {
    std::memmove(Dst + WordShift, Dst, (كلموم_2 - WordShift) * APINT_WORD_SIZE);
  } وإلا {
    طالما (كلموم_2-- > WordShift) {
      Dst[كلموم_2] = Dst[كلموم_2 - WordShift] << BitShift;
      إذا (كلموم_2 > WordShift)
        Dst[كلموم_2] |=
          Dst[كلموم_2 - WordShift - 1] >> (APINT_BITS_PER_WORD - BitShift);
    }
  }

  // Fill in the remainder with 0s.
  std::memset(Dst, 0, WordShift * APINT_WORD_SIZE);
}

/// Shift a bignum right Count bits in-place. Shifted in bits are zero. There
/// are no restrictions on Count.
فراغ APInt::tc_Shift_دام(WordType *Dst, unsigned كلموم_2, unsigned كدي_2) {
  // Don't bother performing a no-op shift.
  إذا (!كدي_2)
    أرجع;

  // WordShift is the inter-part shift; BitShift is the intra-part shift.
  unsigned WordShift = std::min(كدي_2 / APINT_BITS_PER_WORD, كلموم_2);
  unsigned BitShift = كدي_2 % APINT_BITS_PER_WORD;

  unsigned كلموم_To_Move = كلموم_2 - WordShift;
  // Fastpath for moving by whole words.
  إذا (BitShift == 0) {
    std::memmove(Dst, Dst + WordShift, كلموم_To_Move * APINT_WORD_SIZE);
  } وإلا {
    لكل (unsigned i = 0; i != كلموم_To_Move; ++i) {
      Dst[i] = Dst[i + WordShift] >> BitShift;
      إذا (i + 1 != كلموم_To_Move)
        Dst[i] |= Dst[i + WordShift + 1] << (APINT_BITS_PER_WORD - BitShift);
    }
  }

  // Fill in the remainder with 0s.
  std::memset(Dst + كلموم_To_Move, 0, WordShift * APINT_WORD_SIZE);
}

// Comparison (unsigned) of two bignums.
صحيح APInt::tcCompare(ثابت WordType *lhs, ثابت WordType *rhs,
                     unsigned parts) {
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
  حالة APInt::Rounding::DOWN:
  حالة APInt::Rounding::TOWARD_ZERO:
    أرجع A.udiv(B);
  حالة APInt::Rounding::UP: {
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
  حالة APInt::Rounding::DOWN:
  حالة APInt::Rounding::UP: {
    APInt Quo, Rem;
    APInt::sdivrem(A, B, Quo, Rem);
    إذا (Rem.isZero())
      أرجع Quo;
    // This algorithm deals with arbitrary rounding mode used by sdivrem.
    // We want to check whether the non-integer part of the mathematical value
    // is negative or not. If the non-integer part is negative, we need to round
    // down from Quo; otherwise, if it's positive or 0, we return Quo, as it's
    // already rounded down.
    إذا (RM == APInt::Rounding::DOWN) {
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
                                           unsigned RangeWidth) {
  unsigned CoeffWidth = A.getBitWidth();
  أكد(CoeffWidth == B.getBitWidth() && CoeffWidth == C.getBitWidth());
  أكد(RangeWidth <= CoeffWidth &&
         "Value range width should be less than coefficient width");
  أكد(RangeWidth > 1 && "Value range bit width should be > 1");

  LLVM_DEBUG(dbgs() << __func__ << ": solving " << A << "x^2 + " << B
                    << "x + " << C << ", rw:" << RangeWidth << '\n');

  // Identify 0 as a (non)solution immediately.
  إذا (C.sextOrTrunc(RangeWidth).isZero()) {
    LLVM_DEBUG(dbgs() << __func__ << ": zero solution\n");
    أرجع APInt(CoeffWidth, 0);
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
  APInt R = APInt::getOneBitSet(CoeffWidth, RangeWidth);
  APInt TwoA = 2 * A;
  APInt SqrB = B * B;
  منطقي Pick_كس;

  auto RoundUp = [] (ثابت APInt &V, ثابت APInt &A) -> APInt {
    أكد(A.isStrictlyPositive());
    APInt T = V.abs().urem(A);
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
    إذا (C.isStrictlyPositive())
      C -= R;
    // Pick the greater solution.
    Pick_كس = خطأ;
  } وإلا {
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
    إذا (C.sgt(LowkR)) {
      // If LowkR < C, then such a k is guaranteed to exist because
      // LowkR itself is a multiple of R.
      C -= -RoundUp(-C, R);      // C = C - RoundDown(C, R)
      // Pick the smaller solution.
      Pick_كس = true;
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
      Pick_كس = خطأ;
    }
  }

  LLVM_DEBUG(dbgs() << __func__ << ": updated coefficients " << A << "x^2 + "
                    << B << "x + " << C << ", rw:" << RangeWidth << '\n');

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
  إذا (Pick_كس)
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

std::optional<unsigned>
llvm::APIntOps::GetMostSignificantDifferentBit(ثابت APInt &A, ثابت APInt &B) {
  أكد(A.getBitWidth() == B.getBitWidth() && "Must have the same bitwidth");
  إذا (A == B)
    أرجع std::nullopt;
  أرجع A.getBitWidth() - ((A ^ B).countl_zero() + 1);
}

APInt llvm::APIntOps::ScaleBitMask(ثابت APInt &A, unsigned NewBitWidth,
                                   منطقي MatchAllBits) {
  unsigned OldBitWidth = A.getBitWidth();
  أكد((((OldBitWidth % NewBitWidth) == 0) ||
          ((NewBitWidth % OldBitWidth) == 0)) &&
         "One size should be a multiple of the other one. "
         "Can't do fractional scaling.");

  // Check for matching bitwidths.
  إذا (OldBitWidth == NewBitWidth)
    أرجع A;

  APInt NewA = APInt::getZero(NewBitWidth);

  // Check for null input.
  إذا (A.isZero())
    أرجع NewA;

  إذا (NewBitWidth > OldBitWidth) {
    // Repeat bits.
    unsigned Scale = NewBitWidth / OldBitWidth;
    لكل (unsigned i = 0; i != OldBitWidth; ++i)
      إذا (A[i])
        NewA.setBits(i * Scale, (i + 1) * Scale);
  } وإلا {
    unsigned Scale = OldBitWidth / NewBitWidth;
    لكل (unsigned i = 0; i != NewBitWidth; ++i) {
      إذا (MatchAllBits) {
        إذا (A.extractBits(Scale, i * Scale).isAllOnes())
          NewA.setBit(i);
      } وإلا {
        إذا (!A.extractBits(Scale, i * Scale).isZero())
          NewA.setBit(i);
      }
    }
  }

  أرجع NewA;
}

/// StoreIntToMemory - Fills the StoreBytes bytes of memory starting from Dst
/// with the integer held in IntVal.
فراغ llvm::StoreIntToMemory(ثابت APInt &IntVal, uint8_t *Dst,
                            unsigned StoreBytes) {
  أكد((IntVal.getBitWidth()+7)/8 >= StoreBytes && "Integer too small!");
  ثابت uint8_t *Src = (ثابت uint8_t *)IntVal.getRawData();

  إذا (sys::IsLittleEndianHost) {
    // Little-endian host - the source is ordered from LSB to MSB.  Order the
    // destination from LSB to MSB: Do a straight copy.
    memcpy(Dst, Src, StoreBytes);
  } وإلا {
    // Big-endian host - the source is an array of 64 bit words ordered from
    // LSW to MSW.  Each word is ordered from MSB to LSB.  Order the destination
    // from MSB to LSB: Reverse the word order, but not the bytes in a word.
    طالما (StoreBytes > حجم(uint64_t)) {
      StoreBytes -= حجم(uint64_t);
      // May not be aligned so use memcpy.
      memcpy(Dst + StoreBytes, Src, حجم(uint64_t));
      Src += حجم(uint64_t);
    }

    memcpy(Dst, Src + حجم(uint64_t) - StoreBytes, StoreBytes);
  }
}

/// LoadIntFromMemory - Loads the integer stored in the LoadBytes bytes starting
/// from Src into IntVal, which is assumed to be wide enough and to hold zero.
فراغ llvm::LoadIntFromMemory(APInt &IntVal, ثابت uint8_t *Src,
                             unsigned LoadBytes) {
  أكد((IntVal.getBitWidth()+7)/8 >= LoadBytes && "Integer too small!");
  uint8_t *Dst = reinterpret_cast<uint8_t *>(
                   const_cast<uint64_t *>(IntVal.getRawData()));

  إذا (sys::IsLittleEndianHost)
    // Little-endian host - the destination must be ordered from LSB to MSB.
    // The source is ordered from LSB to MSB: Do a straight copy.
    memcpy(Dst, Src, LoadBytes);
  وإلا {
    // Big-endian - the destination is an array of 64 bit words ordered from
    // LSW to MSW.  Each word must be ordered from MSB to LSB.  The source is
    // ordered from MSB to LSB: Reverse the word order, but not the bytes in
    // a word.
    طالما (LoadBytes > حجم(uint64_t)) {
      LoadBytes -= حجم(uint64_t);
      // May not be aligned so use memcpy.
      memcpy(Dst, Src + LoadBytes, حجم(uint64_t));
      Dst += حجم(uint64_t);
    }

    memcpy(Dst + حجم(uint64_t) - LoadBytes, Src, LoadBytes);
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
  أكد(C1.getBitWidth() == C2.getBitWidth() && "Unequal bitwidths");
  unsigned FullWidth = C1.getBitWidth() * 2;
  APInt C1Ext = C1.sext(FullWidth);
  APInt C2Ext = C2.sext(FullWidth);
  أرجع (C1Ext * C2Ext).extractBits(C1.getBitWidth(), C1.getBitWidth());
}

APInt APIntOps::mulhu(ثابت APInt &C1, ثابت APInt &C2) {
  أكد(C1.getBitWidth() == C2.getBitWidth() && "Unequal bitwidths");
  unsigned FullWidth = C1.getBitWidth() * 2;
  APInt C1Ext = C1.zext(FullWidth);
  APInt C2Ext = C2.zext(FullWidth);
  أرجع (C1Ext * C2Ext).extractBits(C1.getBitWidth(), C1.getBitWidth());
}

APInt APIntOps::mulsExtended(ثابت APInt &C1, ثابت APInt &C2) {
  أكد(C1.getBitWidth() == C2.getBitWidth() && "Unequal bitwidths");
  unsigned FullWidth = C1.getBitWidth() * 2;
  APInt C1Ext = C1.sext(FullWidth);
  APInt C2Ext = C2.sext(FullWidth);
  أرجع C1Ext * C2Ext;
}

APInt APIntOps::muluExtended(ثابت APInt &C1, ثابت APInt &C2) {
  أكد(C1.getBitWidth() == C2.getBitWidth() && "Unequal bitwidths");
  unsigned FullWidth = C1.getBitWidth() * 2;
  APInt C1Ext = C1.zext(FullWidth);
  APInt C2Ext = C2.zext(FullWidth);
  أرجع C1Ext * C2Ext;
}

APInt APIntOps::pow(ثابت APInt &X, int64_t N) {
  أكد(N >= 0 && "negative exponents not supported.");
  APInt Acc = APInt(X.getBitWidth(), 1);
  إذا (N == 0)
    أرجع Acc;
  APInt Base = X;
  int64_t RemainingExponent = N;
  طالما (RemainingExponent > 0) {
    طالما (RemainingExponent % 2 == 0) {
      Base *= Base;
      RemainingExponent /= 2;
    }
    --RemainingExponent;
    Acc *= Base;
  }
  أرجع Acc;
}

APInt llvm::APIntOps::fshl(ثابت APInt &Hi, ثابت APInt &Lo,
                           ثابت APInt &Shift) {
  أكد(Hi.getBitWidth() == Lo.getBitWidth());
  unsigned ShiftAmt = rotateModulo(Hi.getBitWidth(), Shift);
  إذا (ShiftAmt == 0)
    أرجع Hi;
  أرجع Hi.shl(ShiftAmt) | Lo.lshr(Hi.getBitWidth() - ShiftAmt);
}

APInt llvm::APIntOps::fshr(ثابت APInt &Hi, ثابت APInt &Lo,
                           ثابت APInt &Shift) {
  أكد(Hi.getBitWidth() == Lo.getBitWidth());
  unsigned ShiftAmt = rotateModulo(Hi.getBitWidth(), Shift);
  إذا (ShiftAmt == 0)
    أرجع Lo;
  أرجع Hi.shl(Hi.getBitWidth() - ShiftAmt) | Lo.lshr(ShiftAmt);
}

APInt llvm::APIntOps::clmul(ثابت APInt &LHS, ثابت APInt &RHS) {
  unsigned BW = LHS.getBitWidth();
  أكد(BW == RHS.getBitWidth() && "Operand mismatch");
  APInt Result(BW, 0);
  لكل (unsigned I : seq(std::min(RHS.getActiveBits(), BW - LHS.countr_zero())))
    إذا (RHS[I])
      Result ^= LHS << I;
  أرجع Result;
}

APInt llvm::APIntOps::clmulr(ثابت APInt &LHS, ثابت APInt &RHS) {
  أكد(LHS.getBitWidth() == RHS.getBitWidth());
  أرجع clmul(LHS.reverseBits(), RHS.reverseBits()).reverseBits();
}

APInt llvm::APIntOps::clmulh(ثابت APInt &LHS, ثابت APInt &RHS) {
  أكد(LHS.getBitWidth() == RHS.getBitWidth());
  أرجع clmulr(LHS, RHS).lshr(1);
}

APInt llvm::APIntOps::pext(ثابت APInt &Val, ثابت APInt &Mask) {
  unsigned BW = Val.getBitWidth();
  أكد(BW == Mask.getBitWidth() && "Operand mismatch");
  APInt Result = APInt::getZero(BW);
  لكل (unsigned I = 0, P = 0; I != BW; ++I)
    إذا (Mask[I])
      Result.setBitVal(P++, Val[I]);
  أرجع Result;
}

APInt llvm::APIntOps::pdep(ثابت APInt &Val, ثابت APInt &Mask) {
  unsigned BW = Val.getBitWidth();
  أكد(BW == Mask.getBitWidth() && "Operand mismatch");
  APInt Result = APInt::getZero(BW);
  لكل (unsigned I = 0, P = 0; I != BW; ++I)
    إذا (Mask[I])
      Result.setBitVal(I, Val[P++]);
  أرجع Result;
}
