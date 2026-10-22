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

#समावेश "llvm/ADT/APInt.h"
#समावेश "llvm/ADT/ArrayRef.h"
#समावेश "llvm/ADT/FoldingSet.h"
#समावेश "llvm/ADT/Hashing.h"
#समावेश "llvm/ADT/Sequence.h"
#समावेश "llvm/ADT/SmallString.h"
#समावेश "llvm/ADT/StringRef.h"
#समावेश "llvm/ADT/bit.h"
#समावेश "llvm/Support/Alignment.h"
#समावेश "llvm/Support/Debug.h"
#समावेश "llvm/Support/ErrorHandling.h"
#समावेश "llvm/Support/MathExtras.h"
#समावेश "llvm/Support/SwapByteOrder.h"
#समावेश "llvm/Support/raw_ostream.h"
#समावेश <cmath>
#समावेश <optional>

using namespace llvm;

#घोषणा DEBUG_TYPE "apint"

/// A utility function for allocating memory, checking for allocation failures,
/// and ensuring the contents are zeroed.
अंतरभूत अटल uint64_t* getClearedMemory(अचिन्हित अंक_शब्दगण) {
  वापस नया uint64_t[अंक_शब्दगण]();
}

/// A utility function for allocating memory and checking for allocation
/// failure.  The content is not zeroed.
अंतरभूत अटल uint64_t* getMemory(अचिन्हित अंक_शब्दगण) {
  वापस नया uint64_t[अंक_शब्दगण];
}

/// A utility function that converts a character to a digit.
अंतरभूत अटल अचिन्हित getDigit(अक्षर cdigit, uint8_t radix) {
  अचिन्हित r;

  अगर (radix == 16 || radix == 36) {
    r = cdigit - '0';
    अगर (r <= 9)
      वापस r;

    r = cdigit - 'A';
    अगर (r <= radix - 11U)
      वापस r + 10;

    r = cdigit - 'a';
    अगर (r <= radix - 11U)
      वापस r + 10;

    radix = 10;
  }

  r = cdigit - '0';
  अगर (r < radix)
    वापस r;

  वापस अच_पूर्णांक_उच्च;
}


व्योम APInt::initSlowCase(uint64_t val, bool isSigned) {
  अगर (isSigned && int64_t(val) < 0) {
    U.pVal = getMemory(get_अंक_शब्दगण());
    U.pVal[0] = val;
    स_रखो(&U.pVal[1], 0xFF, APINT_WORD_आकार * (get_अंक_शब्दगण() - 1));
    clearUnusedBits();
  } अन्यथा {
    U.pVal = getClearedMemory(get_अंक_शब्दगण());
    U.pVal[0] = val;
  }
}

व्योम APInt::initSlowCase(स्थिर APInt& that) {
  U.pVal = getMemory(get_अंक_शब्दगण());
  स_नकल(U.pVal, that.U.pVal, get_अंक_शब्दगण() * APINT_WORD_आकार);
}

व्योम APInt::initFromArray(ArrayRef<uint64_t> bigVal) {
  निश्चित(bigVal.तथ्य() && "Null pointer detected!");
  अगर (LLVM_LIKELY(isSingleWord()))
    U.VAL = bigVal[0];
  अन्यथा {
    // Get memory, cleared to 0
    U.pVal = getClearedMemory(get_अंक_शब्दगण());
    // Calculate the number of words to copy
    अचिन्हित शब्दगण = std::min<अचिन्हित>(bigVal.आकार_2(), get_अंक_शब्दगण());
    // Copy the words from bigVal to pVal
    स_नकल(U.pVal, bigVal.तथ्य(), शब्दगण * APINT_WORD_आकार);
  }
  // Make sure unused high bits are cleared
  clearUnusedBits();
}

APInt::APInt(अचिन्हित अंक_Bits, ArrayRef<uint64_t> bigVal) : BitWidth(अंक_Bits) {
  initFromArray(bigVal);
}

APInt::APInt(अचिन्हित numbits, StringRef Str, uint8_t radix)
    : BitWidth(numbits) {
  fromString(numbits, Str, radix);
}

व्योम APInt::reallocate(अचिन्हित NewBitWidth) {
  // If the number of words is the same we can just change the width and stop.
  अगर (get_अंक_शब्दगण() == get_अंक_शब्दगण(NewBitWidth)) {
    BitWidth = NewBitWidth;
    वापस;
  }

  // If we have an allocation, delete it.
  अगर (LLVM_UNLIKELY(!isSingleWord()))
    मिटाओ [] U.pVal;

  // Update BitWidth.
  BitWidth = NewBitWidth;

  // If we are supposed to have an allocation, create it.
  अगर (LLVM_UNLIKELY(!isSingleWord()))
    U.pVal = getMemory(get_अंक_शब्दगण());
}

व्योम APInt::assignSlowCase(स्थिर APInt &RHS) {
  // Don't do anything for X = X
  अगर (यह == &RHS)
    वापस;

  // Adjust the bit width and handle allocations as necessary.
  reallocate(RHS.getBitWidth());

  // Copy the data.
  अगर (LLVM_LIKELY(isSingleWord()))
    U.VAL = RHS.U.VAL;
  अन्यथा
    स_नकल(U.pVal, RHS.U.pVal, get_अंक_शब्दगण() * APINT_WORD_आकार);
}

/// This method 'profiles' an APInt for use with FoldingSet.
व्योम APInt::Profile(Folding_Set_गाँठ_ID& ID) स्थिर {
  ID.AddInteger(BitWidth);

  अगर (LLVM_LIKELY(isSingleWord())) {
    ID.AddInteger(U.VAL);
    वापस;
  }

  अचिन्हित अंक_शब्दगण_2 = get_अंक_शब्दगण();
  क्रम (अचिन्हित i = 0; i < अंक_शब्दगण_2; ++i)
    ID.AddInteger(U.pVal[i]);
}

bool APInt::isAligned(Align A) स्थिर {
  अगर (isZero())
    वापस true;
  स्थिर अचिन्हित TrailingZeroes = countr_zero();
  स्थिर अचिन्हित MinimumTrailingZeroes = Log2(A);
  वापस TrailingZeroes >= MinimumTrailingZeroes;
}

/// Prefix increment operator. Increments the APInt by one.
APInt& APInt::चालक++() {
  अगर (LLVM_LIKELY(isSingleWord()))
    ++U.VAL;
  अन्यथा
    tcIncrement(U.pVal, get_अंक_शब्दगण());
  वापस clearUnusedBits();
}

/// Prefix decrement operator. Decrements the APInt by one.
APInt& APInt::चालक--() {
  अगर (LLVM_LIKELY(isSingleWord()))
    --U.VAL;
  अन्यथा
    tcDecrement(U.pVal, get_अंक_शब्दगण());
  वापस clearUnusedBits();
}

/// Adds the RHS APInt to this APInt.
/// @returns this, after addition of RHS.
/// Addition assignment operator.
APInt& APInt::चालक+=(स्थिर APInt& RHS) {
  निश्चित(BitWidth == RHS.BitWidth && "Bit widths must be the same");
  अगर (LLVM_LIKELY(isSingleWord()))
    U.VAL += RHS.U.VAL;
  अन्यथा
    tcAdd(U.pVal, RHS.U.pVal, 0, get_अंक_शब्दगण());
  वापस clearUnusedBits();
}

APInt& APInt::चालक+=(uint64_t RHS) {
  अगर (LLVM_LIKELY(isSingleWord()))
    U.VAL += RHS;
  अन्यथा
    tcAddPart(U.pVal, RHS, get_अंक_शब्दगण());
  वापस clearUnusedBits();
}

/// Subtracts the RHS APInt from this APInt
/// @returns this, after subtraction
/// Subtraction assignment operator.
APInt& APInt::चालक-=(स्थिर APInt& RHS) {
  निश्चित(BitWidth == RHS.BitWidth && "Bit widths must be the same");
  अगर (LLVM_LIKELY(isSingleWord()))
    U.VAL -= RHS.U.VAL;
  अन्यथा
    tcSubtract(U.pVal, RHS.U.pVal, 0, get_अंक_शब्दगण());
  वापस clearUnusedBits();
}

APInt& APInt::चालक-=(uint64_t RHS) {
  अगर (LLVM_LIKELY(isSingleWord()))
    U.VAL -= RHS;
  अन्यथा
    tcSubtractPart(U.pVal, RHS, get_अंक_शब्दगण());
  वापस clearUnusedBits();
}

APInt APInt::चालक*(स्थिर APInt& RHS) स्थिर {
  निश्चित(BitWidth == RHS.BitWidth && "Bit widths must be the same");
  अगर (LLVM_LIKELY(isSingleWord()))
    वापस APInt(BitWidth, U.VAL * RHS.U.VAL, /*isSigned=*/false,
                 /*implicitTrunc=*/true);

  APInt परिणाम(getMemory(get_अंक_शब्दगण()), getBitWidth());
  tcMultiply(परिणाम.U.pVal, U.pVal, RHS.U.pVal, get_अंक_शब्दगण());
  परिणाम.clearUnusedBits();
  वापस परिणाम;
}

व्योम APInt::andAssignSlowCase(स्थिर APInt &RHS) {
  WordType *dst = U.pVal, *rhs = RHS.U.pVal;
  क्रम (माप_प्रकार i = 0, e = get_अंक_शब्दगण(); i != e; ++i)
    dst[i] &= rhs[i];
}

व्योम APInt::orAssignSlowCase(स्थिर APInt &RHS) {
  WordType *dst = U.pVal, *rhs = RHS.U.pVal;
  क्रम (माप_प्रकार i = 0, e = get_अंक_शब्दगण(); i != e; ++i)
    dst[i] |= rhs[i];
}

व्योम APInt::xorAssignSlowCase(स्थिर APInt &RHS) {
  WordType *dst = U.pVal, *rhs = RHS.U.pVal;
  क्रम (माप_प्रकार i = 0, e = get_अंक_शब्दगण(); i != e; ++i)
    dst[i] ^= rhs[i];
}

APInt &APInt::चालक*=(स्थिर APInt &RHS) {
  *यह = *यह * RHS;
  वापस *यह;
}

APInt& APInt::चालक*=(uint64_t RHS) {
  अगर (LLVM_LIKELY(isSingleWord())) {
    U.VAL *= RHS;
  } अन्यथा {
    अचिन्हित अंक_शब्दगण_2 = get_अंक_शब्दगण();
    tcMultiplyPart(U.pVal, U.pVal, RHS, 0, अंक_शब्दगण_2, अंक_शब्दगण_2, false);
  }
  वापस clearUnusedBits();
}

bool APInt::equalSlowCase(स्थिर APInt &RHS) स्थिर {
  वापस std::equal(U.pVal, U.pVal + get_अंक_शब्दगण(), RHS.U.pVal);
}

पूर्णांक APInt::compare(स्थिर APInt& RHS) स्थिर {
  निश्चित(BitWidth == RHS.BitWidth && "Bit widths must be same for comparison");
  अगर (LLVM_LIKELY(isSingleWord()))
    वापस U.VAL < RHS.U.VAL ? -1 : U.VAL > RHS.U.VAL;

  वापस tcCompare(U.pVal, RHS.U.pVal, get_अंक_शब्दगण());
}

पूर्णांक APInt::compareSigned(स्थिर APInt& RHS) स्थिर {
  निश्चित(BitWidth == RHS.BitWidth && "Bit widths must be same for comparison");
  अगर (LLVM_LIKELY(isSingleWord())) {
    int64_t lhsSext = SignExtend64(U.VAL, BitWidth);
    int64_t rhsSext = SignExtend64(RHS.U.VAL, BitWidth);
    वापस lhsSext < rhsSext ? -1 : lhsSext > rhsSext;
  }

  bool lhsNeg = isNegative();
  bool rhsNeg = RHS.isNegative();

  // If the sign bits don't match, then (LHS < RHS) if LHS is negative
  अगर (lhsNeg != rhsNeg)
    वापस lhsNeg ? -1 : 1;

  // Otherwise we can just use an unsigned comparison, because even negative
  // numbers compare correctly this way if both have the same signed-ness.
  वापस tcCompare(U.pVal, RHS.U.pVal, get_अंक_शब्दगण());
}

व्योम APInt::setBitsSlowCase(अचिन्हित loBit, अचिन्हित hiBit) {
  अचिन्हित loWord = whichWord(loBit);
  अचिन्हित hiWord = whichWord(hiBit);

  // Create an initial mask for the low word with zeros below loBit.
  uint64_t loMask = WORDTYPE_MAX << whichBit(loBit);

  // If hiBit is not aligned, we need a high mask.
  अचिन्हित hiShiftAmt = whichBit(hiBit);
  अगर (hiShiftAmt != 0) {
    // Create a high mask with zeros above hiBit.
    uint64_t hiMask = WORDTYPE_MAX >> (APINT_BITS_PER_WORD - hiShiftAmt);
    // If loWord and hiWord are equal, then we combine the masks. Otherwise,
    // set the bits in hiWord.
    अगर (hiWord == loWord)
      loMask &= hiMask;
    अन्यथा
      U.pVal[hiWord] |= hiMask;
  }
  // Apply the mask to the low word.
  U.pVal[loWord] |= loMask;

  // Fill any words between loWord and hiWord with all ones.
  क्रम (अचिन्हित word = loWord + 1; word < hiWord; ++word)
    U.pVal[word] = WORDTYPE_MAX;
}

व्योम APInt::clearBitsSlowCase(अचिन्हित LoBit, अचिन्हित HiBit) {
  अचिन्हित LoWord = whichWord(LoBit);
  अचिन्हित HiWord = whichWord(HiBit);

  // Create an initial mask for the low word with ones below loBit.
  uint64_t LoMask = ~(WORDTYPE_MAX << whichBit(LoBit));

  // If HiBit is not aligned, we need a high mask.
  अचिन्हित HiShiftAmt = whichBit(HiBit);
  अगर (HiShiftAmt != 0) {
    // Create a high mask with ones above HiBit.
    uint64_t HiMask = ~(WORDTYPE_MAX >> (APINT_BITS_PER_WORD - HiShiftAmt));
    // If LoWord and HiWord are equal, then we combine the masks. Otherwise,
    // clear the bits in HiWord.
    अगर (HiWord == LoWord)
      LoMask |= HiMask;
    अन्यथा
      U.pVal[HiWord] &= HiMask;
  }
  // Apply the mask to the low word.
  U.pVal[LoWord] &= LoMask;

  // Fill any words between LoWord and HiWord with all zeros.
  क्रम (अचिन्हित Word = LoWord + 1; Word < HiWord; ++Word)
    U.pVal[Word] = 0;
}

// Complement a bignum in-place.
अटल व्योम tcComplement(APInt::WordType *dst, अचिन्हित parts) {
  क्रम (अचिन्हित i = 0; i < parts; i++)
    dst[i] = ~dst[i];
}

/// Toggle every bit to its opposite value.
व्योम APInt::flipAllBitsSlowCase() {
  tcComplement(U.pVal, get_अंक_शब्दगण());
  clearUnusedBits();
}

/// Concatenate the bits from "NewLSB" onto the bottom of *this.  This is
/// equivalent to:
///   (this->zext(NewWidth) << NewLSB.getBitWidth()) | NewLSB.zext(NewWidth)
/// In the slow case, we know the result is large.
APInt APInt::concatSlowCase(स्थिर APInt &NewLSB) स्थिर {
  अचिन्हित NewWidth = getBitWidth() + NewLSB.getBitWidth();
  APInt परिणाम = NewLSB.zext(NewWidth);
  परिणाम.जोड़ो_Bits(*यह, NewLSB.getBitWidth());
  वापस परिणाम;
}

/// Toggle a given bit to its opposite value whose position is given
/// as "bitPosition".
/// Toggles a given bit to its opposite value.
व्योम APInt::flipBit(अचिन्हित bitPosition) {
  निश्चित(bitPosition < BitWidth && "Out of the bit-width range!");
  setBitVal(bitPosition, !(*यह)[bitPosition]);
}

व्योम APInt::जोड़ो_Bits(स्थिर APInt &subBits, अचिन्हित bitPosition) {
  अचिन्हित subBitWidth = subBits.getBitWidth();
  निश्चित((subBitWidth + bitPosition) <= BitWidth && "Illegal bit insertion");

  // inserting no bits is a noop.
  अगर (subBitWidth == 0)
    वापस;

  // Insertion is a direct copy.
  अगर (subBitWidth == BitWidth) {
    *यह = subBits;
    वापस;
  }

  // Single word result can be done as a direct bitmask.
  अगर (LLVM_LIKELY(isSingleWord())) {
    uint64_t mask = WORDTYPE_MAX >> (APINT_BITS_PER_WORD - subBitWidth);
    U.VAL &= ~(mask << bitPosition);
    U.VAL |= (subBits.U.VAL << bitPosition);
    वापस;
  }

  अचिन्हित loBit = whichBit(bitPosition);
  अचिन्हित loWord = whichWord(bitPosition);
  अचिन्हित hi1Word = whichWord(bitPosition + subBitWidth - 1);

  // Insertion within a single word can be done as a direct bitmask.
  अगर (loWord == hi1Word) {
    uint64_t mask = WORDTYPE_MAX >> (APINT_BITS_PER_WORD - subBitWidth);
    U.pVal[loWord] &= ~(mask << loBit);
    U.pVal[loWord] |= (subBits.U.VAL << loBit);
    वापस;
  }

  // Insert on word boundaries.
  अगर (loBit == 0) {
    // Direct copy whole words.
    अचिन्हित अंक_Whole_Sub_शब्दगण = subBitWidth / APINT_BITS_PER_WORD;
    स_नकल(U.pVal + loWord, subBits.get_Raw_तथ्य(),
           अंक_Whole_Sub_शब्दगण * APINT_WORD_आकार);

    // Mask+insert remaining bits.
    अचिन्हित remainingBits = subBitWidth % APINT_BITS_PER_WORD;
    अगर (remainingBits != 0) {
      uint64_t mask = WORDTYPE_MAX >> (APINT_BITS_PER_WORD - remainingBits);
      U.pVal[hi1Word] &= ~mask;
      U.pVal[hi1Word] |= subBits.getWord(subBitWidth - 1);
    }
    वापस;
  }

  // General case - set/clear individual bits in dst based on src.
  // TODO - there is scope for optimization here, but at the moment this code
  // path is barely used so prefer readability over performance.
  क्रम (अचिन्हित i = 0; i != subBitWidth; ++i)
    setBitVal(bitPosition + i, subBits[i]);
}

व्योम APInt::जोड़ो_Bits(uint64_t subBits, अचिन्हित bitPosition, अचिन्हित अंक_Bits) {
  uint64_t maskBits = maskTrailingOnes<uint64_t>(अंक_Bits);
  subBits &= maskBits;
  अगर (LLVM_LIKELY(isSingleWord())) {
    U.VAL &= ~(maskBits << bitPosition);
    U.VAL |= subBits << bitPosition;
    वापस;
  }

  अचिन्हित loBit = whichBit(bitPosition);
  अचिन्हित loWord = whichWord(bitPosition);
  अचिन्हित hiWord = whichWord(bitPosition + अंक_Bits - 1);
  अगर (loWord == hiWord) {
    U.pVal[loWord] &= ~(maskBits << loBit);
    U.pVal[loWord] |= subBits << loBit;
    वापस;
  }

  static_assert(8 * माप(WordType) <= 64, "This code assumes only two words affected");
  अचिन्हित wordBits = 8 * माप(WordType);
  U.pVal[loWord] &= ~(maskBits << loBit);
  U.pVal[loWord] |= subBits << loBit;

  U.pVal[hiWord] &= ~(maskBits >> (wordBits - loBit));
  U.pVal[hiWord] |= subBits >> (wordBits - loBit);
}

APInt APInt::extractBits(अचिन्हित अंक_Bits, अचिन्हित bitPosition) स्थिर {
  निश्चित(bitPosition < BitWidth && (अंक_Bits + bitPosition) <= BitWidth &&
         "Illegal bit extraction");

  अगर (LLVM_LIKELY(isSingleWord()))
    वापस APInt(अंक_Bits, U.VAL >> bitPosition, /*isSigned=*/false,
                 /*implicitTrunc=*/true);

  अचिन्हित loBit = whichBit(bitPosition);
  अचिन्हित loWord = whichWord(bitPosition);
  अचिन्हित hiWord = whichWord(bitPosition + अंक_Bits - 1);

  // Single word result extracting bits from a single word source.
  अगर (loWord == hiWord)
    वापस APInt(अंक_Bits, U.pVal[loWord] >> loBit, /*isSigned=*/false,
                 /*implicitTrunc=*/true);

  // Extracting bits that start on a source word boundary can be done
  // as a fast memory copy.
  अगर (loBit == 0)
    वापस APInt(अंक_Bits, ArrayRef(U.pVal + loWord, 1 + hiWord - loWord));

  // General case - shift + copy source words directly into place.
  APInt परिणाम(अंक_Bits, 0);
  अचिन्हित अंक_Src_शब्दगण = get_अंक_शब्दगण();
  अचिन्हित अंक_Dst_शब्दगण = परिणाम.get_अंक_शब्दगण();

  uint64_t *DestPtr =
      LLVM_LIKELY(परिणाम.isSingleWord()) ? &परिणाम.U.VAL : परिणाम.U.pVal;
  क्रम (अचिन्हित word = 0; word < अंक_Dst_शब्दगण; ++word) {
    uint64_t w0 = U.pVal[loWord + word];
    uint64_t w1 =
        (loWord + word + 1) < अंक_Src_शब्दगण ? U.pVal[loWord + word + 1] : 0;
    DestPtr[word] = (w0 >> loBit) | (w1 << (APINT_BITS_PER_WORD - loBit));
  }

  वापस परिणाम.clearUnusedBits();
}

uint64_t APInt::extract_Bits_As_ZExt_मूल्य(अचिन्हित अंक_Bits,
                                       अचिन्हित bitPosition) स्थिर {
  निश्चित(bitPosition < BitWidth && (अंक_Bits + bitPosition) <= BitWidth &&
         "Illegal bit extraction");
  निश्चित(अंक_Bits <= 64 && "Illegal bit extraction");

  uint64_t maskBits = maskTrailingOnes<uint64_t>(अंक_Bits);
  अगर (LLVM_LIKELY(isSingleWord()))
    वापस (U.VAL >> bitPosition) & maskBits;

  static_assert(APINT_BITS_PER_WORD >= 64,
                "This code assumes only two words affected");
  अचिन्हित loBit = whichBit(bitPosition);
  अचिन्हित loWord = whichWord(bitPosition);
  अचिन्हित hiWord = whichWord(bitPosition + अंक_Bits - 1);
  अगर (loWord == hiWord)
    वापस (U.pVal[loWord] >> loBit) & maskBits;

  uint64_t retBits = U.pVal[loWord] >> loBit;
  retBits |= U.pVal[hiWord] << (APINT_BITS_PER_WORD - loBit);
  retBits &= maskBits;
  वापस retBits;
}

अचिन्हित APInt::getSufficientBitsNeeded(StringRef Str, uint8_t Radix) {
  निश्चित(!Str.empty() && "Invalid string length");
  माप_प्रकार StrLen = Str.आकार_2();

  // Each computation below needs to know if it's negative.
  अचिन्हित IsNegative = false;
  अगर (Str[0] == '-' || Str[0] == '+') {
    IsNegative = Str[0] == '-';
    StrLen--;
    निश्चित(StrLen && "String is only a sign, needs a value.");
  }

  // For radixes of power-of-two values, the bits required is accurately and
  // easily computed.
  अगर (Radix == 2)
    वापस StrLen + IsNegative;
  अगर (Radix == 8)
    वापस StrLen * 3 + IsNegative;
  अगर (Radix == 16)
    वापस StrLen * 4 + IsNegative;

  // Compute a sufficient number of bits that is always large enough but might
  // be too large. This avoids the assertion in the constructor. This
  // calculation doesn't work appropriately for the numbers 0-9, so just use 4
  // bits in that case.
  अगर (Radix == 10)
    वापस (StrLen == 1 ? 4 : StrLen * 64 / 18) + IsNegative;

  निश्चित(Radix == 36);
  वापस (StrLen == 1 ? 7 : StrLen * 16 / 3) + IsNegative;
}

अचिन्हित APInt::getBitsNeeded(StringRef str, uint8_t radix) {
  // Compute a sufficient number of bits that is always large enough but might
  // be too large.
  अचिन्हित sufficient = getSufficientBitsNeeded(str, radix);

  // For bases 2, 8, and 16, the sufficient number of bits is exact and we can
  // return the value directly. For bases 10 and 36, we need to do extra work.
  अगर (radix == 2 || radix == 8 || radix == 16)
    वापस sufficient;

  // This is grossly inefficient but accurate. We could probably do something
  // with a computation of roughly slen*64/20 and then adjust by the value of
  // the first few digits. But, I'm not sure how accurate that could be.
  माप_प्रकार slen = str.आकार_2();

  // Each computation below needs to know if it's negative.
  StringRef::iterator p = str.begin();
  अचिन्हित isNegative = *p == '-';
  अगर (*p == '-' || *p == '+') {
    p++;
    slen--;
    निश्चित(slen && "String is only a sign, needs a value.");
  }


  // Convert to the actual binary value.
  APInt tmp(sufficient, StringRef(p, slen), radix);

  // Compute how many bits are required. If the log is infinite, assume we need
  // just bit. If the log is exact and value is negative, then the value is
  // MinSignedValue with (log + 1) bits.
  अचिन्हित log = tmp.logBase2();
  अगर (log == (अचिन्हित)-1) {
    वापस isNegative + 1;
  } अन्यथा अगर (isNegative && tmp.isPowerOf2()) {
    वापस isNegative + log;
  } अन्यथा {
    वापस isNegative + log + 1;
  }
}

hash_code llvm::hash_मूल्य(स्थिर APInt &Arg) {
  अगर (LLVM_LIKELY(Arg.isSingleWord()))
    वापस hash_combine(Arg.BitWidth, Arg.U.VAL);

  वापस hash_combine(
      Arg.BitWidth,
      hash_combine_range(Arg.U.pVal, Arg.U.pVal + Arg.get_अंक_शब्दगण()));
}

अचिन्हित DenseMapInfo<APInt, व्योम>::get_Hash_मूल्य(स्थिर APInt &कुंजी) {
  वापस static_cast<अचिन्हित>(hash_मूल्य(कुंजी));
}

bool APInt::isSplat(अचिन्हित Splat_आकार_In_Bits) स्थिर {
  निश्चित(getBitWidth() % Splat_आकार_In_Bits == 0 &&
         "SplatSizeInBits must divide width!");
  // We can check that all parts of an integer are equal by making use of a
  // little trick: rotate and check if it's still the same value.
  वापस *यह == rotl(Splat_आकार_In_Bits);
}

/// This function returns the high "numBits" bits of this APInt.
APInt APInt::getHiBits(अचिन्हित अंक_Bits) स्थिर {
  वापस यह->lshr(BitWidth - अंक_Bits);
}

/// This function returns the low "numBits" bits of this APInt.
APInt APInt::getLoBits(अचिन्हित अंक_Bits) स्थिर {
  APInt परिणाम(get_निचला_Bits_Set(BitWidth, अंक_Bits));
  परिणाम &= *यह;
  वापस परिणाम;
}

/// Return a value containing V broadcasted over NewLen bits.
APInt APInt::getSplat(अचिन्हित NewLen, स्थिर APInt &V) {
  निश्चित(NewLen >= V.getBitWidth() && "Can't splat to smaller bit width!");

  APInt Val = V.zext(NewLen);
  क्रम (अचिन्हित I = V.getBitWidth(); I < NewLen; I <<= 1)
    Val |= Val << I;

  वापस Val;
}

अचिन्हित APInt::गिनती_Leading_Zeros_Slow_Case() स्थिर {
  अचिन्हित गिनती_2 = 0;
  क्रम (पूर्णांक i = get_अंक_शब्दगण() - 1; i >= 0; --i) {
    uint64_t V = U.pVal[i];
    अगर (V == 0)
      गिनती_2 += APINT_BITS_PER_WORD;
    अन्यथा {
      गिनती_2 += llvm::countl_zero(V);
      अवरोध;
    }
  }
  // Adjust for unused bits in the most significant word (they are zero).
  अचिन्हित Mod = BitWidth % APINT_BITS_PER_WORD;
  गिनती_2 -= Mod > 0 ? APINT_BITS_PER_WORD - Mod : 0;
  वापस गिनती_2;
}

अचिन्हित APInt::गिनती_Leading_Ones_Slow_Case() स्थिर {
  अचिन्हित ऊपरी_Word_Bits = BitWidth % APINT_BITS_PER_WORD;
  अचिन्हित shift;
  अगर (!ऊपरी_Word_Bits) {
    ऊपरी_Word_Bits = APINT_BITS_PER_WORD;
    shift = 0;
  } अन्यथा {
    shift = APINT_BITS_PER_WORD - ऊपरी_Word_Bits;
  }
  पूर्णांक i = get_अंक_शब्दगण() - 1;
  अचिन्हित गिनती_2 = llvm::countl_one(U.pVal[i] << shift);
  अगर (गिनती_2 == ऊपरी_Word_Bits) {
    क्रम (i--; i >= 0; --i) {
      अगर (U.pVal[i] == WORDTYPE_MAX)
        गिनती_2 += APINT_BITS_PER_WORD;
      अन्यथा {
        गिनती_2 += llvm::countl_one(U.pVal[i]);
        अवरोध;
      }
    }
  }
  वापस गिनती_2;
}

अचिन्हित APInt::गिनती_Trailing_Zeros_Slow_Case() स्थिर {
  अचिन्हित गिनती_2 = 0;
  अचिन्हित i = 0;
  क्रम (; i < get_अंक_शब्दगण() && U.pVal[i] == 0; ++i)
    गिनती_2 += APINT_BITS_PER_WORD;
  अगर (i < get_अंक_शब्दगण())
    गिनती_2 += llvm::countr_zero(U.pVal[i]);
  वापस std::min(गिनती_2, BitWidth);
}

अचिन्हित APInt::गिनती_Trailing_Ones_Slow_Case() स्थिर {
  अचिन्हित गिनती_2 = 0;
  अचिन्हित i = 0;
  क्रम (; i < get_अंक_शब्दगण() && U.pVal[i] == WORDTYPE_MAX; ++i)
    गिनती_2 += APINT_BITS_PER_WORD;
  अगर (i < get_अंक_शब्दगण())
    गिनती_2 += llvm::countr_one(U.pVal[i]);
  निश्चित(गिनती_2 <= BitWidth);
  वापस गिनती_2;
}

अचिन्हित APInt::गिनती_Population_Slow_Case() स्थिर {
  अचिन्हित गिनती_2 = 0;
  क्रम (अचिन्हित i = 0; i < get_अंक_शब्दगण(); ++i)
    गिनती_2 += llvm::popcount(U.pVal[i]);
  वापस गिनती_2;
}

bool APInt::isPowerOf2SlowCase() स्थिर {
  अचिन्हित गिनती_2 = 0;
  क्रम (अचिन्हित i = 0; i < get_अंक_शब्दगण(); ++i) {
    गिनती_2 += llvm::popcount(U.pVal[i]);
    अगर (गिनती_2 > 1)
      वापस false;
  }
  वापस गिनती_2 == 1;
}

bool APInt::intersectsSlowCase(स्थिर APInt &RHS) स्थिर {
  क्रम (अचिन्हित i = 0, e = get_अंक_शब्दगण(); i != e; ++i)
    अगर ((U.pVal[i] & RHS.U.pVal[i]) != 0)
      वापस true;

  वापस false;
}

bool APInt::isSubsetOfSlowCase(स्थिर APInt &RHS) स्थिर {
  क्रम (अचिन्हित i = 0, e = get_अंक_शब्दगण(); i != e; ++i)
    अगर ((U.pVal[i] & ~RHS.U.pVal[i]) != 0)
      वापस false;

  वापस true;
}

bool APInt::isInverseOfSlowCase(स्थिर APInt &RHS) स्थिर {
  स्थिर अचिन्हित Last = get_अंक_शब्दगण() - 1;
  क्रम (अचिन्हित I = 0; I != Last; ++I)
    अगर ((U.pVal[I] ^ RHS.U.pVal[I]) != WORDTYPE_MAX)
      वापस false;

  अचिन्हित पुच्छ_Bits = BitWidth - Last * APINT_BITS_PER_WORD;
  WordType पुच्छ_Mask = llvm::maskTrailingOnes<WordType>(पुच्छ_Bits);
  वापस (U.pVal[Last] ^ RHS.U.pVal[Last]) == पुच्छ_Mask;
}

APInt APInt::byteSwap() स्थिर {
  निश्चित(BitWidth >= 16 && BitWidth % 8 == 0 && "Cannot byteswap!");
  अगर (BitWidth == 16)
    वापस APInt(BitWidth, llvm::byteswap<uint16_t>(U.VAL));
  अगर (BitWidth == 32)
    वापस APInt(BitWidth, llvm::byteswap<uint32_t>(U.VAL));
  अगर (BitWidth <= 64) {
    uint64_t Tmp1 = llvm::byteswap<uint64_t>(U.VAL);
    Tmp1 >>= (64 - BitWidth);
    वापस APInt(BitWidth, Tmp1);
  }

  APInt परिणाम(get_अंक_शब्दगण() * APINT_BITS_PER_WORD, 0);
  क्रम (अचिन्हित I = 0, N = get_अंक_शब्दगण(); I != N; ++I)
    परिणाम.U.pVal[I] = llvm::byteswap<uint64_t>(U.pVal[N - I - 1]);
  अगर (परिणाम.BitWidth != BitWidth) {
    परिणाम.lshrInPlace(परिणाम.BitWidth - BitWidth);
    परिणाम.BitWidth = BitWidth;
  }
  वापस परिणाम;
}

APInt APInt::reverseBits() स्थिर {
  अगर (LLVM_LIKELY(isSingleWord())) {
    चयन (BitWidth) {
    हाल 64:
      वापस APInt(BitWidth, llvm::reverseBits<uint64_t>(U.VAL));
    हाल 32:
      वापस APInt(BitWidth, llvm::reverseBits<uint32_t>(U.VAL));
    हाल 16:
      वापस APInt(BitWidth, llvm::reverseBits<uint16_t>(U.VAL));
    हाल 8:
      वापस APInt(BitWidth, llvm::reverseBits<uint8_t>(U.VAL));
    हाल 1: // fallthrough
    हाल 0:
      वापस *यह;
    शेष:
      वापस APInt(BitWidth,
                   llvm::reverseBits<uint64_t>(U.VAL) >> (64 - BitWidth));
    }
  }

  APInt परिणाम(BitWidth, 0);
  अचिन्हित अंक_शब्दगण_2 = get_अंक_शब्दगण();
  अचिन्हित ExcessBits = अंक_शब्दगण_2 * APINT_BITS_PER_WORD - BitWidth;
  अगर (ExcessBits == 0) {
    // Fast path. No cross-word shift needed.
    क्रम (अचिन्हित I = 0; I < अंक_शब्दगण_2; ++I)
      परिणाम.U.pVal[I] = llvm::reverseBits<uint64_t>(U.pVal[अंक_शब्दगण_2 - 1 - I]);
    वापस परिणाम;
  }
  // Holds reversed bits of the previous (more significant) word.
  uint64_t पूर्व_Rev = llvm::reverseBits<uint64_t>(U.pVal[अंक_शब्दगण_2 - 1]);
  क्रम (अचिन्हित I = 0; I < अंक_शब्दगण_2 - 1; ++I) {
    uint64_t CurrRev = llvm::reverseBits<uint64_t>(U.pVal[अंक_शब्दगण_2 - 2 - I]);
    परिणाम.U.pVal[I] = (पूर्व_Rev >> ExcessBits) | (CurrRev << (64 - ExcessBits));
    पूर्व_Rev = CurrRev;
  }
  परिणाम.U.pVal[अंक_शब्दगण_2 - 1] = पूर्व_Rev >> ExcessBits;
  वापस परिणाम;
}

APInt llvm::APIntOps::Greatest_Common_भाजक(APInt A, APInt B, bool IsSigned) {
  // Take absolute value if IsSigned.
  अगर (IsSigned) {
    A = A.असल();
    B = B.असल();
  }

  // Fast-path a common case.
  अगर (A == B) वापस A;

  // Corner cases: if either operand is zero, the other is the gcd.
  अगर (!A) वापस B;
  अगर (!B) वापस A;

  // Count common powers of 2 and remove all other powers of 2.
  अचिन्हित Pow2;
  {
    अचिन्हित Pow2_A = A.countr_zero();
    अचिन्हित Pow2_B = B.countr_zero();
    अगर (Pow2_A > Pow2_B) {
      A.lshrInPlace(Pow2_A - Pow2_B);
      Pow2 = Pow2_B;
    } अन्यथा अगर (Pow2_B > Pow2_A) {
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
  जबतक (A != B) {
    अगर (A.ugt(B)) {
      A -= B;
      A.lshrInPlace(A.countr_zero() - Pow2);
    } अन्यथा {
      B -= A;
      B.lshrInPlace(B.countr_zero() - Pow2);
    }
  }

  वापस A;
}

APInt llvm::APIntOps::RoundDoubleToAPInt(द्विगुन Double, अचिन्हित width) {
  uint64_t I = bit_cast<uint64_t>(Double);

  // Get the sign bit from the highest order bit
  bool isNeg = I >> 63;

  // Get the 11-bit exponent and adjust for the 1023 bit bias
  int64_t exp = ((I >> 52) & 0x7ff) - 1023;

  // If the exponent is negative, the value is < 0 so just return 0.
  अगर (exp < 0)
    वापस APInt(width, 0u);

  // Extract the mantissa by clearing the top 12 bits (sign + exponent).
  uint64_t mantissa = (I & (~0ULL >> 12)) | 1ULL << 52;

  // If the exponent doesn't shift all bits out of the mantissa
  अगर (exp < 52)
    वापस isNeg ? -APInt(width, mantissa >> (52 - exp)) :
                    APInt(width, mantissa >> (52 - exp));

  // If the client didn't provide enough bits for us to shift the mantissa into
  // then the result is undefined, just return 0
  अगर (width <= exp - 52)
    वापस APInt(width, 0);

  // Otherwise, we have to shift the mantissa bits up to the right location
  APInt Tmp(width, mantissa);
  Tmp <<= (अचिन्हित)exp - 52;
  वापस isNeg ? -Tmp : Tmp;
}

/// This function converts this APInt to a double.
/// The layout for double is as following (IEEE Standard 754):
///  --------------------------------------
/// |  Sign    Exponent    Fraction    Bias |
/// |-------------------------------------- |
/// |  1[63]   11[62-52]   52[51-00]   1023 |
///  --------------------------------------
द्विगुन APInt::roundToDouble(bool isSigned) स्थिर {
  // Handle the simple case where the value is contained in one uint64_t.
  // It is wrong to optimize getWord(0) to VAL; there might be more than one word.
  अगर (LLVM_LIKELY(isSingleWord() || getActiveBits() <= APINT_BITS_PER_WORD)) {
    अगर (isSigned) {
      int64_t sext = SignExtend64(getWord(0), BitWidth);
      वापस द्विगुन(sext);
    }
    वापस द्विगुन(getWord(0));
  }

  // Determine if the value is negative.
  bool isNeg = isSigned ? (*यह)[BitWidth-1] : false;

  // Construct the absolute value if we're negative.
  APInt Tmp(isNeg ? -(*यह) : (*यह));

  // Figure out how many bits we're using.
  अचिन्हित n = Tmp.getActiveBits();

  // The exponent (without bias normalization) is just the number of bits
  // we are using. Note that the sign bit is gone since we constructed the
  // absolute value.
  uint64_t exp = n;

  // Return infinity for exponent overflow
  अगर (exp > 1023) {
    अगर (!isSigned || !isNeg)
      वापस std::numeric_limits<द्विगुन>::infinity();
    अन्यथा
      वापस -std::numeric_limits<द्विगुन>::infinity();
  }
  exp += 1023; // Increment for 1023 bias

  // Number of bits in mantissa is 52. To obtain the mantissa value, we must
  // extract the high 52 bits from the correct words in pVal.
  uint64_t mantissa;
  अचिन्हित hiWord = whichWord(n-1);
  अगर (hiWord == 0) {
    mantissa = Tmp.U.pVal[0];
    अगर (n > 52)
      mantissa >>= n - 52; // shift down, we want the top 52 bits.
  } अन्यथा {
    निश्चित(hiWord > 0 && "huh?");
    uint64_t hibits = Tmp.U.pVal[hiWord] << (52 - n % APINT_BITS_PER_WORD);
    uint64_t lobits = Tmp.U.pVal[hiWord-1] >> (11 + n % APINT_BITS_PER_WORD);
    mantissa = hibits | lobits;
  }

  // The leading bit of mantissa is implicit, so get rid of it.
  uint64_t sign = isNeg ? (1ULL << (APINT_BITS_PER_WORD - 1)) : 0;
  uint64_t I = sign | (exp << 52) | mantissa;
  वापस bit_cast<द्विगुन>(I);
}

// Truncate to new width.
APInt APInt::छाँटो(अचिन्हित width) स्थिर {
  निश्चित(width <= BitWidth && "Invalid APInt Truncate request");

  अगर (width <= APINT_BITS_PER_WORD)
    वापस APInt(width, get_Raw_तथ्य()[0], /*isSigned=*/false,
                 /*implicitTrunc=*/true);

  अगर (width == BitWidth)
    वापस *यह;

  APInt परिणाम(getMemory(get_अंक_शब्दगण(width)), width);

  // Copy full words.
  अचिन्हित i;
  क्रम (i = 0; i != width / APINT_BITS_PER_WORD; i++)
    परिणाम.U.pVal[i] = U.pVal[i];

  // Truncate and copy any partial word.
  अचिन्हित bits = (0 - width) % APINT_BITS_PER_WORD;
  अगर (bits != 0)
    परिणाम.U.pVal[i] = U.pVal[i] << bits >> bits;

  वापस परिणाम;
}

// Truncate to new width with unsigned saturation.
APInt APInt::truncUSat(अचिन्हित width) स्थिर {
  निश्चित(width <= BitWidth && "Invalid APInt Truncate request");

  // Can we just losslessly truncate it?
  अगर (isIntN(width))
    वापस छाँटो(width);
  // If not, then just return the new limit.
  वापस APInt::get_Max_मूल्य(width);
}

// Truncate to new width with signed saturation to signed result.
APInt APInt::truncSSat(अचिन्हित width) स्थिर {
  निश्चित(width <= BitWidth && "Invalid APInt Truncate request");

  // Can we just losslessly truncate it?
  अगर (isSignedIntN(width))
    वापस छाँटो(width);
  // If not, then just return the new limits.
  वापस isNegative() ? APInt::get_Signed_Min_मूल्य(width)
                      : APInt::get_Signed_Max_मूल्य(width);
}

// Truncate to new width with signed saturation to unsigned result.
APInt APInt::truncSSatU(अचिन्हित width) स्थिर {
  निश्चित(width <= BitWidth && "Invalid APInt Truncate request");

  // Can we just losslessly truncate it?
  अगर (isIntN(width))
    वापस छाँटो(width);
  // If not, then just return the new limits.
  वापस isNegative() ? APInt::getZero(width) : APInt::get_Max_मूल्य(width);
}

// Sign extend to a new width.
APInt APInt::sext(अचिन्हित Width) स्थिर {
  निश्चित(Width >= BitWidth && "Invalid APInt SignExtend request");

  अगर (Width <= APINT_BITS_PER_WORD)
    वापस APInt(Width, SignExtend64(U.VAL, BitWidth), /*isSigned=*/true);

  अगर (Width == BitWidth)
    वापस *यह;

  APInt परिणाम(getMemory(get_अंक_शब्दगण(Width)), Width);

  // Copy words.
  std::स_नकल(परिणाम.U.pVal, get_Raw_तथ्य(), get_अंक_शब्दगण() * APINT_WORD_आकार);

  // Sign extend the last word since there may be unused bits in the input.
  परिणाम.U.pVal[get_अंक_शब्दगण() - 1] =
      SignExtend64(परिणाम.U.pVal[get_अंक_शब्दगण() - 1],
                   ((BitWidth - 1) % APINT_BITS_PER_WORD) + 1);

  // Fill with sign bits.
  std::स_रखो(परिणाम.U.pVal + get_अंक_शब्दगण(), isNegative() ? -1 : 0,
              (परिणाम.get_अंक_शब्दगण() - get_अंक_शब्दगण()) * APINT_WORD_आकार);
  परिणाम.clearUnusedBits();
  वापस परिणाम;
}

//  Zero extend to a new width.
APInt APInt::zext(अचिन्हित width) स्थिर {
  निश्चित(width >= BitWidth && "Invalid APInt ZeroExtend request");

  अगर (width <= APINT_BITS_PER_WORD)
    वापस APInt(width, U.VAL);

  अगर (width == BitWidth)
    वापस *यह;

  APInt परिणाम(getMemory(get_अंक_शब्दगण(width)), width);

  // Copy words.
  std::स_नकल(परिणाम.U.pVal, get_Raw_तथ्य(), get_अंक_शब्दगण() * APINT_WORD_आकार);

  // Zero remaining words.
  std::स_रखो(परिणाम.U.pVal + get_अंक_शब्दगण(), 0,
              (परिणाम.get_अंक_शब्दगण() - get_अंक_शब्दगण()) * APINT_WORD_आकार);

  वापस परिणाम;
}

APInt APInt::zextOrTrunc(अचिन्हित width) स्थिर {
  अगर (BitWidth < width)
    वापस zext(width);
  अगर (BitWidth > width)
    वापस छाँटो(width);
  वापस *यह;
}

APInt APInt::sextOrTrunc(अचिन्हित width) स्थिर {
  अगर (BitWidth < width)
    वापस sext(width);
  अगर (BitWidth > width)
    वापस छाँटो(width);
  वापस *यह;
}

/// Arithmetic right-shift this APInt by shiftAmt.
/// Arithmetic right-shift function.
व्योम APInt::ashrInPlace(स्थिर APInt &shiftAmt) {
  ashrInPlace((अचिन्हित)shiftAmt.get_Limited_मूल्य(BitWidth));
}

/// Arithmetic right-shift this APInt by shiftAmt.
/// Arithmetic right-shift function.
व्योम APInt::ashrSlowCase(अचिन्हित ShiftAmt) {
  // Don't bother performing a no-op shift.
  अगर (!ShiftAmt)
    वापस;

  // Save the original sign bit for later.
  bool Negative = isNegative();

  // WordShift is the inter-part shift; BitShift is intra-part shift.
  अचिन्हित WordShift = ShiftAmt / APINT_BITS_PER_WORD;
  अचिन्हित BitShift = ShiftAmt % APINT_BITS_PER_WORD;

  अचिन्हित शब्दगण_To_Move = get_अंक_शब्दगण() - WordShift;
  अगर (शब्दगण_To_Move != 0) {
    // Sign extend the last word to fill in the unused bits.
    U.pVal[get_अंक_शब्दगण() - 1] = SignExtend64(
        U.pVal[get_अंक_शब्दगण() - 1], ((BitWidth - 1) % APINT_BITS_PER_WORD) + 1);

    // Fastpath for moving by whole words.
    अगर (BitShift == 0) {
      std::स_हटाओ(U.pVal, U.pVal + WordShift, शब्दगण_To_Move * APINT_WORD_आकार);
    } अन्यथा {
      // Move the words containing significant bits.
      क्रम (अचिन्हित i = 0; i != शब्दगण_To_Move - 1; ++i)
        U.pVal[i] = (U.pVal[i + WordShift] >> BitShift) |
                    (U.pVal[i + WordShift + 1] << (APINT_BITS_PER_WORD - BitShift));

      // Handle the last word which has no high bits to copy. Use an arithmetic
      // shift to preserve the sign bit.
      U.pVal[शब्दगण_To_Move - 1] =
          (int64_t)U.pVal[WordShift + शब्दगण_To_Move - 1] >> BitShift;
    }
  }

  // Fill in the remainder based on the original sign.
  std::स_रखो(U.pVal + शब्दगण_To_Move, Negative ? -1 : 0,
              WordShift * APINT_WORD_आकार);
  clearUnusedBits();
}

/// Logical right-shift this APInt by shiftAmt.
/// Logical right-shift function.
व्योम APInt::lshrInPlace(स्थिर APInt &shiftAmt) {
  lshrInPlace((अचिन्हित)shiftAmt.get_Limited_मूल्य(BitWidth));
}

/// Logical right-shift this APInt by shiftAmt.
/// Logical right-shift function.
व्योम APInt::lshrSlowCase(अचिन्हित ShiftAmt) {
  tc_Shift_दायाँ(U.pVal, get_अंक_शब्दगण(), ShiftAmt);
}

/// Left-shift this APInt by shiftAmt.
/// Left-shift function.
APInt &APInt::चालक<<=(स्थिर APInt &shiftAmt) {
  // It's undefined behavior in C to shift by BitWidth or greater.
  *यह <<= (अचिन्हित)shiftAmt.get_Limited_मूल्य(BitWidth);
  वापस *यह;
}

व्योम APInt::shlSlowCase(अचिन्हित ShiftAmt) {
  tc_Shift_बायाँ(U.pVal, get_अंक_शब्दगण(), ShiftAmt);
  clearUnusedBits();
}

// Calculate the rotate amount modulo the bit width.
अटल अचिन्हित rotateModulo(अचिन्हित BitWidth, स्थिर APInt &rotateAmt) {
  अगर (LLVM_UNLIKELY(BitWidth == 0))
    वापस 0;
  अचिन्हित rotBitWidth = rotateAmt.getBitWidth();
  APInt rot = rotateAmt;
  अगर (rotBitWidth < BitWidth) {
    // Extend the rotate APInt, so that the urem doesn't divide by 0.
    // e.g. APInt(1, 32) would give APInt(1, 0).
    rot = rotateAmt.zext(BitWidth);
  }
  rot = rot.urem(APInt(rot.getBitWidth(), BitWidth));
  वापस rot.get_Limited_मूल्य(BitWidth);
}

APInt APInt::rotl(स्थिर APInt &rotateAmt) स्थिर {
  वापस rotl(rotateModulo(BitWidth, rotateAmt));
}

APInt APInt::rotl(अचिन्हित rotateAmt) स्थिर {
  अगर (LLVM_UNLIKELY(BitWidth == 0))
    वापस *यह;
  rotateAmt %= BitWidth;
  अगर (rotateAmt == 0)
    वापस *यह;
  वापस shl(rotateAmt) | lshr(BitWidth - rotateAmt);
}

APInt APInt::rotr(स्थिर APInt &rotateAmt) स्थिर {
  वापस rotr(rotateModulo(BitWidth, rotateAmt));
}

APInt APInt::rotr(अचिन्हित rotateAmt) स्थिर {
  अगर (BitWidth == 0)
    वापस *यह;
  rotateAmt %= BitWidth;
  अगर (rotateAmt == 0)
    वापस *यह;
  वापस lshr(rotateAmt) | shl(BitWidth - rotateAmt);
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
अचिन्हित APInt::nearestLogBase2() स्थिर {
  // Special case when we have a bitwidth of 1. If VAL is 1, then we
  // get 0. If VAL is 0, we get WORDTYPE_MAX which gets truncated to
  // UINT32_MAX.
  अगर (BitWidth == 1)
    वापस U.VAL - 1;

  // Handle the zero case.
  अगर (isZero())
    वापस UINT32_MAX;

  // The non-zero case is handled by computing:
  //
  //   nearestLogBase2(x) = logBase2(x) + x[logBase2(x)-1].
  //
  // where x[i] is referring to the value of the ith bit of x.
  अचिन्हित lg = logBase2();
  वापस lg + अचिन्हित((*यह)[lg - 1]);
}

// Square Root - this method computes and returns the square root of "this".
// Three mechanisms are used for computation. For small values (<= 5 bits),
// a table lookup is done. This gets some performance for common cases. For
// values using less than 52 bits, the value is converted to double and then
// the libc sqrt function is called. The result is rounded and then converted
// back to a uint64_t which is then used to construct the result. Finally,
// the Babylonian method for computing square roots is used.
APInt APInt::sqrtFloor() स्थिर {

  // Determine the magnitude of the value.
  अचिन्हित magnitude = getActiveBits();

  // Use a fast table for some small values. This also gets rid of some
  // rounding errors in libc sqrt for small values.
  अगर (magnitude <= 5) {
    अटल स्थिर uint8_t results[32] = {
        /*     0 */ 0,
        /*  1- 3 */ 1, 1, 1,
        /*  4- 8 */ 2, 2, 2, 2, 2,
        /*  9-15 */ 3, 3, 3, 3, 3, 3, 3,
        /* 16-24 */ 4, 4, 4, 4, 4, 4, 4, 4, 4,
        /* 25-31 */ 5, 5, 5, 5, 5, 5, 5,
    };
    वापस APInt(BitWidth,
                 results[(LLVM_LIKELY(isSingleWord()) ? U.VAL : U.pVal[0])]);
  }

  // If the magnitude of the value fits in less than 52 bits (the precision of
  // an IEEE double precision floating point value), then we can use the
  // libc sqrt function which will probably use a hardware sqrt computation.
  // This should be faster than the algorithm below.
  अगर (magnitude < 52) {
    वापस APInt(BitWidth,
                 uint64_t(::न्यूनमान(::वर्ग_मूल(द्विगुन(
                     LLVM_LIKELY(isSingleWord()) ? U.VAL : U.pVal[0])))));
  }

  // Okay, all the short cuts are exhausted. We must compute it. The following
  // is a classical Babylonian method for computing the square root. This code
  // was adapted to APInt from a wikipedia article on such computations.
  // See http://www.wikipedia.org/ and go to the page named
  // Calculate_an_integer_square_root.
  अचिन्हित nbits = BitWidth, i = 4;
  APInt testy(BitWidth, 16);
  APInt x_old(BitWidth, 1);
  APInt x_new(BitWidth, 0);
  APInt two(BitWidth, 2);

  // Select a good starting value using binary logarithms.
  क्रम (;; i += 2, testy = testy.shl(2))
    अगर (i >= nbits || यह->ule(testy)) {
      x_old = x_old.shl(i / 2);
      अवरोध;
    }

  // Use the Babylonian method to arrive at the integer square root:
  क्रम (;;) {
    x_new = (यह->udiv(x_old) + x_old).udiv(two);
    अगर (x_old.ule(x_new))
      अवरोध;
    x_old = x_new;
  }
  वापस x_old;
}

/// \returns the multiplicative inverse of an odd APInt modulo 2^BitWidth.
APInt APInt::multiplicativeInverse() स्थिर {
  निश्चित((*यह)[0] &&
         "multiplicative inverse is only defined for odd numbers!");

  // Use Newton's method.
  APInt Factor = *यह;
  APInt T;
  जबतक (!(T = *यह * Factor).isOne())
    Factor *= 2 - std::move(T);
  वापस Factor;
}

/// Implementation of Knuth's Algorithm D (Division of nonnegative integers)
/// from "Art of Computer Programming, Volume 2", section 4.3.1, p. 272. The
/// variables here have the same names as in the algorithm. Comments explain
/// the algorithm and any deviation from it.
अटल व्योम KnuthDiv(uint32_t *u, uint32_t *v, uint32_t *q, uint32_t* r,
                     अचिन्हित m, अचिन्हित n) {
  निश्चित(u && "Must provide dividend");
  निश्चित(v && "Must provide divisor");
  निश्चित(q && "Must provide quotient");
  निश्चित(u != v && u != q && v != q && "Must use different memory");
  निश्चित(n>1 && "n must be > 1");

  // b denotes the base of the number system. In our case b is 2^32.
  स्थिर uint64_t b = uint64_t(1) << 32;

// The DEBUG macros here tend to be spam in the debug output if you're not
// debugging this code. Disable them unless KNUTH_DEBUG is defined.
#अगर_घोषित KNUTH_DEBUG
#घोषणा DEBUG_KNUTH(X) LLVM_DEBUG(X)
#अन्यथा
#घोषणा DEBUG_KNUTH(X) करो {} जबतक(false)
#पूर्ण_अगर

  DEBUG_KNUTH(dbgs() << "KnuthDiv: m=" << m << " n=" << n << '\n');
  DEBUG_KNUTH(dbgs() << "KnuthDiv: original:");
  DEBUG_KNUTH(क्रम (पूर्णांक i = m + n; i >= 0; i--) dbgs() << " " << u[i]);
  DEBUG_KNUTH(dbgs() << " by");
  DEBUG_KNUTH(क्रम (पूर्णांक i = n; i > 0; i--) dbgs() << " " << v[i - 1]);
  DEBUG_KNUTH(dbgs() << '\n');
  // D1. [Normalize.] Set d = b / (v[n-1] + 1) and multiply all the digits of
  // u and v by d. Note that we have taken Knuth's advice here to use a power
  // of 2 value for d such that d * v[n-1] >= b/2 (b is the base). A power of
  // 2 allows us to shift instead of multiply and it is easy to determine the
  // shift amount from the leading zeros.  We are basically normalizing the u
  // and v so that its high bits are shifted to the top of v's range without
  // overflow. Note that this can require an extra word in u so that u must
  // be of length m+n+1.
  अचिन्हित shift = llvm::countl_zero(v[n - 1]);
  uint32_t v_carry = 0;
  uint32_t u_carry = 0;
  अगर (shift) {
    क्रम (अचिन्हित i = 0; i < m+n; ++i) {
      uint32_t u_tmp = u[i] >> (32 - shift);
      u[i] = (u[i] << shift) | u_carry;
      u_carry = u_tmp;
    }
    क्रम (अचिन्हित i = 0; i < n; ++i) {
      uint32_t v_tmp = v[i] >> (32 - shift);
      v[i] = (v[i] << shift) | v_carry;
      v_carry = v_tmp;
    }
  }
  u[m+n] = u_carry;

  DEBUG_KNUTH(dbgs() << "KnuthDiv:   normal:");
  DEBUG_KNUTH(क्रम (पूर्णांक i = m + n; i >= 0; i--) dbgs() << " " << u[i]);
  DEBUG_KNUTH(dbgs() << " by");
  DEBUG_KNUTH(क्रम (पूर्णांक i = n; i > 0; i--) dbgs() << " " << v[i - 1]);
  DEBUG_KNUTH(dbgs() << '\n');

  // D2. [Initialize j.]  Set j to m. This is the loop counter over the places.
  पूर्णांक j = m;
  करो {
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
    अगर (qp == b || qp*v[n-2] > b*rp + u[j+n-2]) {
      qp--;
      rp += v[n-1];
      अगर (rp < b && (qp == b || qp*v[n-2] > b*rp + u[j+n-2]))
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
    क्रम (अचिन्हित i = 0; i < n; ++i) {
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
    DEBUG_KNUTH(क्रम (पूर्णांक i = m + n; i >= 0; i--) dbgs() << " " << u[i]);
    DEBUG_KNUTH(dbgs() << '\n');

    // D5. [Test remainder.] Set q[j] = qp. If the result of step D4 was
    // negative, go to step D6; otherwise go on to step D7.
    q[j] = Lo_32(qp);
    अगर (isNeg) {
      // D6. [Add back]. The probability that this step is necessary is very
      // small, on the order of only 2/b. Make sure that test data accounts for
      // this possibility. Decrease q[j] by 1
      q[j]--;
      // and add (0v[n-1]...v[1]v[0]) to (u[j+n]u[j+n-1]...u[j+1]u[j]).
      // A carry will occur to the left of u[j+n], and it should be ignored
      // since it cancels with the borrow that occurred in D4.
      bool carry = false;
      क्रम (अचिन्हित i = 0; i < n; i++) {
        uint32_t limit = std::min(u[j+i],v[i]);
        u[j+i] += v[i] + carry;
        carry = u[j+i] < limit || (carry && u[j+i] == limit);
      }
      u[j+n] += carry;
    }
    DEBUG_KNUTH(dbgs() << "KnuthDiv: after correction:");
    DEBUG_KNUTH(क्रम (पूर्णांक i = m + n; i >= 0; i--) dbgs() << " " << u[i]);
    DEBUG_KNUTH(dbgs() << "\nKnuthDiv: digit result = " << q[j] << '\n');

    // D7. [Loop on j.]  Decrease j by one. Now if j >= 0, go back to D3.
  } जबतक (--j >= 0);

  DEBUG_KNUTH(dbgs() << "KnuthDiv: quotient:");
  DEBUG_KNUTH(क्रम (पूर्णांक i = m; i >= 0; i--) dbgs() << " " << q[i]);
  DEBUG_KNUTH(dbgs() << '\n');

  // D8. [Unnormalize]. Now q[...] is the desired quotient, and the desired
  // remainder may be obtained by dividing u[...] by d. If r is non-null we
  // compute the remainder (urem uses this).
  अगर (r) {
    // The value d is expressed by the "shift" value above since we avoided
    // multiplication by d by using a shift left. So, all we have to do is
    // shift right here.
    अगर (shift) {
      uint32_t carry = 0;
      DEBUG_KNUTH(dbgs() << "KnuthDiv: remainder:");
      क्रम (पूर्णांक i = n-1; i >= 0; i--) {
        r[i] = (u[i] >> shift) | carry;
        carry = u[i] << (32 - shift);
        DEBUG_KNUTH(dbgs() << " " << r[i]);
      }
    } अन्यथा {
      क्रम (पूर्णांक i = n-1; i >= 0; i--) {
        r[i] = u[i];
        DEBUG_KNUTH(dbgs() << " " << r[i]);
      }
    }
    DEBUG_KNUTH(dbgs() << '\n');
  }
  DEBUG_KNUTH(dbgs() << '\n');
}

व्योम APInt::divide(स्थिर WordType *LHS, अचिन्हित lhs_शब्दगण, स्थिर WordType *RHS,
                   अचिन्हित rhs_शब्दगण, WordType *Quotient, WordType *Remainder) {
  निश्चित(lhs_शब्दगण >= rhs_शब्दगण && "Fractional result");

  // First, compose the values into an array of 32-bit words instead of
  // 64-bit words. This is a necessity of both the "short division" algorithm
  // and the Knuth "classical algorithm" which requires there to be native
  // operations for +, -, and * on an m bit value with an m*2 bit result. We
  // can't use 64-bit operands here because we don't have native results of
  // 128-bits. Furthermore, casting the 64-bit values to 32-bit values won't
  // work on large-endian machines.
  अचिन्हित n = rhs_शब्दगण * 2;
  अचिन्हित m = (lhs_शब्दगण * 2) - n;

  // Allocate space for the temporary values we need either on the stack, if
  // it will fit, or on the heap if it won't.
  uint32_t SPACE[128];
  uint32_t *U = nullptr;
  uint32_t *V = nullptr;
  uint32_t *Q = nullptr;
  uint32_t *R = nullptr;
  अगर ((Remainder?4:3)*n+2*m+1 <= 128) {
    U = &SPACE[0];
    V = &SPACE[m+n+1];
    Q = &SPACE[(m+n+1) + n];
    अगर (Remainder)
      R = &SPACE[(m+n+1) + n + (m+n)];
  } अन्यथा {
    U = नया uint32_t[m + n + 1];
    V = नया uint32_t[n];
    Q = नया uint32_t[m+n];
    अगर (Remainder)
      R = नया uint32_t[n];
  }

  // Initialize the dividend
  स_रखो(U, 0, (m+n+1)*माप(uint32_t));
  क्रम (अचिन्हित i = 0; i < lhs_शब्दगण; ++i) {
    uint64_t tmp = LHS[i];
    U[i * 2] = Lo_32(tmp);
    U[i * 2 + 1] = Hi_32(tmp);
  }
  U[m+n] = 0; // this extra word is for "spill" in the Knuth algorithm.

  // Initialize the divisor
  स_रखो(V, 0, (n)*माप(uint32_t));
  क्रम (अचिन्हित i = 0; i < rhs_शब्दगण; ++i) {
    uint64_t tmp = RHS[i];
    V[i * 2] = Lo_32(tmp);
    V[i * 2 + 1] = Hi_32(tmp);
  }

  // initialize the quotient and remainder
  स_रखो(Q, 0, (m+n) * माप(uint32_t));
  अगर (Remainder)
    स_रखो(R, 0, n * माप(uint32_t));

  // Now, adjust m and n for the Knuth division. n is the number of words in
  // the divisor. m is the number of words by which the dividend exceeds the
  // divisor (i.e. m+n is the length of the dividend). These sizes must not
  // contain any zero words or the Knuth algorithm fails.
  क्रम (अचिन्हित i = n; i > 0 && V[i-1] == 0; i--) {
    n--;
    m++;
  }
  क्रम (अचिन्हित i = m+n; i > 0 && U[i-1] == 0; i--)
    m--;

  // If we're left with only a single word for the divisor, Knuth doesn't work
  // so we implement the short division algorithm here. This is much simpler
  // and faster because we are certain that we can divide a 64-bit quantity
  // by a 32-bit quantity at hardware speed and short division is simply a
  // series of such operations. This is just like doing short division but we
  // are using base 2^32 instead of base 10.
  निश्चित(n != 0 && "Divide by zero?");
  अगर (n == 1) {
    uint32_t भाजक = V[0];
    uint32_t remainder = 0;
    क्रम (पूर्णांक i = m; i >= 0; i--) {
      uint64_t partial_dividend = Make_64(remainder, U[i]);
      अगर (partial_dividend == 0) {
        Q[i] = 0;
        remainder = 0;
      } अन्यथा अगर (partial_dividend < भाजक) {
        Q[i] = 0;
        remainder = Lo_32(partial_dividend);
      } अन्यथा अगर (partial_dividend == भाजक) {
        Q[i] = 1;
        remainder = 0;
      } अन्यथा {
        Q[i] = Lo_32(partial_dividend / भाजक);
        remainder = Lo_32(partial_dividend - (Q[i] * भाजक));
      }
    }
    अगर (R)
      R[0] = remainder;
  } अन्यथा {
    // Now we're ready to invoke the Knuth classical divide algorithm. In this
    // case n > 1.
    KnuthDiv(U, V, Q, R, m, n);
  }

  // If the caller wants the quotient
  अगर (Quotient) {
    क्रम (अचिन्हित i = 0; i < lhs_शब्दगण; ++i)
      Quotient[i] = Make_64(Q[i*2+1], Q[i*2]);
  }

  // If the caller wants the remainder
  अगर (Remainder) {
    क्रम (अचिन्हित i = 0; i < rhs_शब्दगण; ++i)
      Remainder[i] = Make_64(R[i*2+1], R[i*2]);
  }

  // Clean up the memory we allocated.
  अगर (U != &SPACE[0]) {
    मिटाओ [] U;
    मिटाओ [] V;
    मिटाओ [] Q;
    मिटाओ [] R;
  }
}

APInt APInt::udiv(स्थिर APInt &RHS) स्थिर {
  निश्चित(BitWidth == RHS.BitWidth && "Bit widths must be the same");

  // First, deal with the easy case
  अगर (LLVM_LIKELY(isSingleWord())) {
    निश्चित(RHS.U.VAL != 0 && "Divide by zero?");
    वापस APInt(BitWidth, U.VAL / RHS.U.VAL);
  }

  // Get some facts about the LHS and RHS number of bits and words
  अचिन्हित lhs_शब्दगण = get_अंक_शब्दगण(getActiveBits());
  अचिन्हित rhsBits  = RHS.getActiveBits();
  अचिन्हित rhs_शब्दगण = get_अंक_शब्दगण(rhsBits);
  निश्चित(rhs_शब्दगण && "Divided by zero???");

  // Deal with some degenerate cases
  अगर (!lhs_शब्दगण)
    // 0 / X ===> 0
    वापस APInt(BitWidth, 0);
  अगर (rhsBits == 1)
    // X / 1 ===> X
    वापस *यह;
  अगर (lhs_शब्दगण < rhs_शब्दगण || यह->ult(RHS))
    // X / Y ===> 0, iff X < Y
    वापस APInt(BitWidth, 0);
  अगर (*यह == RHS)
    // X / X ===> 1
    वापस APInt(BitWidth, 1);
  अगर (lhs_शब्दगण == 1) // rhsWords is 1 if lhsWords is 1.
    // All high words are zero, just use native divide
    वापस APInt(BitWidth, यह->U.pVal[0] / RHS.U.pVal[0]);

  // We have to compute it the hard way. Invoke the Knuth divide algorithm.
  APInt Quotient(BitWidth, 0); // to hold result.
  divide(U.pVal, lhs_शब्दगण, RHS.U.pVal, rhs_शब्दगण, Quotient.U.pVal, nullptr);
  वापस Quotient;
}

APInt APInt::udiv(uint64_t RHS) स्थिर {
  निश्चित(RHS != 0 && "Divide by zero?");

  // First, deal with the easy case
  अगर (LLVM_LIKELY(isSingleWord()))
    वापस APInt(BitWidth, U.VAL / RHS);

  // Get some facts about the LHS words.
  अचिन्हित lhs_शब्दगण = get_अंक_शब्दगण(getActiveBits());

  // Deal with some degenerate cases
  अगर (!lhs_शब्दगण)
    // 0 / X ===> 0
    वापस APInt(BitWidth, 0);
  अगर (RHS == 1)
    // X / 1 ===> X
    वापस *यह;
  अगर (यह->ult(RHS))
    // X / Y ===> 0, iff X < Y
    वापस APInt(BitWidth, 0);
  अगर (*यह == RHS)
    // X / X ===> 1
    वापस APInt(BitWidth, 1);
  अगर (lhs_शब्दगण == 1) // rhsWords is 1 if lhsWords is 1.
    // All high words are zero, just use native divide
    वापस APInt(BitWidth, यह->U.pVal[0] / RHS);

  // We have to compute it the hard way. Invoke the Knuth divide algorithm.
  APInt Quotient(BitWidth, 0); // to hold result.
  divide(U.pVal, lhs_शब्दगण, &RHS, 1, Quotient.U.pVal, nullptr);
  वापस Quotient;
}

APInt APInt::sdiv(स्थिर APInt &RHS) स्थिर {
  अगर (isNegative()) {
    अगर (RHS.isNegative())
      वापस (-(*यह)).udiv(-RHS);
    वापस -((-(*यह)).udiv(RHS));
  }
  अगर (RHS.isNegative())
    वापस -(यह->udiv(-RHS));
  वापस यह->udiv(RHS);
}

APInt APInt::sdiv(int64_t RHS) स्थिर {
  अगर (isNegative()) {
    अगर (RHS < 0)
      वापस (-(*यह)).udiv(-RHS);
    वापस -((-(*यह)).udiv(RHS));
  }
  अगर (RHS < 0)
    वापस -(यह->udiv(-RHS));
  वापस यह->udiv(RHS);
}

APInt APInt::urem(स्थिर APInt &RHS) स्थिर {
  निश्चित(BitWidth == RHS.BitWidth && "Bit widths must be the same");
  अगर (LLVM_LIKELY(isSingleWord())) {
    निश्चित(RHS.U.VAL != 0 && "Remainder by zero?");
    वापस APInt(BitWidth, U.VAL % RHS.U.VAL);
  }

  // Get some facts about the LHS
  अचिन्हित lhs_शब्दगण = get_अंक_शब्दगण(getActiveBits());

  // Get some facts about the RHS
  अचिन्हित rhsBits = RHS.getActiveBits();
  अचिन्हित rhs_शब्दगण = get_अंक_शब्दगण(rhsBits);
  निश्चित(rhs_शब्दगण && "Performing remainder operation by zero ???");

  // Check the degenerate cases
  अगर (lhs_शब्दगण == 0)
    // 0 % Y ===> 0
    वापस APInt(BitWidth, 0);
  अगर (rhsBits == 1)
    // X % 1 ===> 0
    वापस APInt(BitWidth, 0);
  अगर (lhs_शब्दगण < rhs_शब्दगण || यह->ult(RHS))
    // X % Y ===> X, iff X < Y
    वापस *यह;
  अगर (*यह == RHS)
    // X % X == 0;
    वापस APInt(BitWidth, 0);
  अगर (lhs_शब्दगण == 1)
    // All high words are zero, just use native remainder
    वापस APInt(BitWidth, U.pVal[0] % RHS.U.pVal[0]);
  अगर (RHS.isPowerOf2()) {
    // X % 2^w ===> X & (2^w - 1)
    APInt परिणाम(*यह);
    परिणाम.clearBits(RHS.logBase2(), BitWidth);
    वापस परिणाम;
  }

  // We have to compute it the hard way. Invoke the Knuth divide algorithm.
  APInt Remainder(BitWidth, 0);
  divide(U.pVal, lhs_शब्दगण, RHS.U.pVal, rhs_शब्दगण, nullptr, Remainder.U.pVal);
  वापस Remainder;
}

uint64_t APInt::urem(uint64_t RHS) स्थिर {
  निश्चित(RHS != 0 && "Remainder by zero?");

  अगर (LLVM_LIKELY(isSingleWord()))
    वापस U.VAL % RHS;

  // Get some facts about the LHS
  अचिन्हित lhs_शब्दगण = get_अंक_शब्दगण(getActiveBits());

  // Check the degenerate cases
  अगर (lhs_शब्दगण == 0)
    // 0 % Y ===> 0
    वापस 0;
  अगर (RHS == 1)
    // X % 1 ===> 0
    वापस 0;
  अगर (यह->ult(RHS))
    // X % Y ===> X, iff X < Y
    वापस get_ZExt_मूल्य();
  अगर (*यह == RHS)
    // X % X == 0;
    वापस 0;
  अगर (lhs_शब्दगण == 1)
    // All high words are zero, just use native remainder
    वापस U.pVal[0] % RHS;
  अगर (llvm::isPowerOf2_64(RHS))
    // X % 2^w ===> X & (2^w - 1)
    वापस U.pVal[0] & (RHS - 1);

  // We have to compute it the hard way. Invoke the Knuth divide algorithm.
  uint64_t Remainder;
  divide(U.pVal, lhs_शब्दगण, &RHS, 1, nullptr, &Remainder);
  वापस Remainder;
}

APInt APInt::srem(स्थिर APInt &RHS) स्थिर {
  अगर (isNegative()) {
    अगर (RHS.isNegative())
      वापस -((-(*यह)).urem(-RHS));
    वापस -((-(*यह)).urem(RHS));
  }
  अगर (RHS.isNegative())
    वापस यह->urem(-RHS);
  वापस यह->urem(RHS);
}

int64_t APInt::srem(int64_t RHS) स्थिर {
  अगर (isNegative()) {
    अगर (RHS < 0)
      वापस -((-(*यह)).urem(-RHS));
    वापस -((-(*यह)).urem(RHS));
  }
  अगर (RHS < 0)
    वापस यह->urem(-RHS);
  वापस यह->urem(RHS);
}

व्योम APInt::udivrem(स्थिर APInt &LHS, स्थिर APInt &RHS,
                    APInt &Quotient, APInt &Remainder) {
  निश्चित(LHS.BitWidth == RHS.BitWidth && "Bit widths must be the same");
  अचिन्हित BitWidth = LHS.BitWidth;

  // First, deal with the easy case
  अगर (LLVM_LIKELY(LHS.isSingleWord())) {
    निश्चित(RHS.U.VAL != 0 && "Divide by zero?");
    uint64_t QuotVal = LHS.U.VAL / RHS.U.VAL;
    uint64_t RemVal = LHS.U.VAL % RHS.U.VAL;
    Quotient = APInt(BitWidth, QuotVal);
    Remainder = APInt(BitWidth, RemVal);
    वापस;
  }

  // Get some size facts about the dividend and divisor
  अचिन्हित lhs_शब्दगण = get_अंक_शब्दगण(LHS.getActiveBits());
  अचिन्हित rhsBits  = RHS.getActiveBits();
  अचिन्हित rhs_शब्दगण = get_अंक_शब्दगण(rhsBits);
  निश्चित(rhs_शब्दगण && "Performing divrem operation by zero ???");

  // Check the degenerate cases
  अगर (lhs_शब्दगण == 0) {
    Quotient = APInt(BitWidth, 0);    // 0 / Y ===> 0
    Remainder = APInt(BitWidth, 0);   // 0 % Y ===> 0
    वापस;
  }

  अगर (rhsBits == 1) {
    Quotient = LHS;                   // X / 1 ===> X
    Remainder = APInt(BitWidth, 0);   // X % 1 ===> 0
  }

  अगर (lhs_शब्दगण < rhs_शब्दगण || LHS.ult(RHS)) {
    Remainder = LHS;                  // X % Y ===> X, iff X < Y
    Quotient = APInt(BitWidth, 0);    // X / Y ===> 0, iff X < Y
    वापस;
  }

  अगर (LHS == RHS) {
    Quotient  = APInt(BitWidth, 1);   // X / X ===> 1
    Remainder = APInt(BitWidth, 0);   // X % X ===> 0;
    वापस;
  }

  // Make sure there is enough space to hold the results.
  // NOTE: This assumes that reallocate won't affect any bits if it doesn't
  // change the size. This is necessary if Quotient or Remainder is aliased
  // with LHS or RHS.
  Quotient.reallocate(BitWidth);
  Remainder.reallocate(BitWidth);

  अगर (lhs_शब्दगण == 1) { // rhsWords is 1 if lhsWords is 1.
    // There is only one word to consider so use the native versions.
    uint64_t lhs_मूल्य = LHS.U.pVal[0];
    uint64_t rhs_मूल्य = RHS.U.pVal[0];
    Quotient = lhs_मूल्य / rhs_मूल्य;
    Remainder = lhs_मूल्य % rhs_मूल्य;
    वापस;
  }

  // Okay, lets do it the long way
  divide(LHS.U.pVal, lhs_शब्दगण, RHS.U.pVal, rhs_शब्दगण, Quotient.U.pVal,
         Remainder.U.pVal);
  // Clear the rest of the Quotient and Remainder.
  std::स_रखो(Quotient.U.pVal + lhs_शब्दगण, 0,
              (get_अंक_शब्दगण(BitWidth) - lhs_शब्दगण) * APINT_WORD_आकार);
  std::स_रखो(Remainder.U.pVal + rhs_शब्दगण, 0,
              (get_अंक_शब्दगण(BitWidth) - rhs_शब्दगण) * APINT_WORD_आकार);
}

व्योम APInt::udivrem(स्थिर APInt &LHS, uint64_t RHS, APInt &Quotient,
                    uint64_t &Remainder) {
  निश्चित(RHS != 0 && "Divide by zero?");
  अचिन्हित BitWidth = LHS.BitWidth;

  // First, deal with the easy case
  अगर (LLVM_LIKELY(LHS.isSingleWord())) {
    uint64_t QuotVal = LHS.U.VAL / RHS;
    Remainder = LHS.U.VAL % RHS;
    Quotient = APInt(BitWidth, QuotVal);
    वापस;
  }

  // Get some size facts about the dividend and divisor
  अचिन्हित lhs_शब्दगण = get_अंक_शब्दगण(LHS.getActiveBits());

  // Check the degenerate cases
  अगर (lhs_शब्दगण == 0) {
    Quotient = APInt(BitWidth, 0);    // 0 / Y ===> 0
    Remainder = 0;                    // 0 % Y ===> 0
    वापस;
  }

  अगर (RHS == 1) {
    Quotient = LHS;                   // X / 1 ===> X
    Remainder = 0;                    // X % 1 ===> 0
    वापस;
  }

  अगर (LHS.ult(RHS)) {
    Remainder = LHS.get_ZExt_मूल्य();   // X % Y ===> X, iff X < Y
    Quotient = APInt(BitWidth, 0);    // X / Y ===> 0, iff X < Y
    वापस;
  }

  अगर (LHS == RHS) {
    Quotient  = APInt(BitWidth, 1);   // X / X ===> 1
    Remainder = 0;                    // X % X ===> 0;
    वापस;
  }

  // Make sure there is enough space to hold the results.
  // NOTE: This assumes that reallocate won't affect any bits if it doesn't
  // change the size. This is necessary if Quotient is aliased with LHS.
  Quotient.reallocate(BitWidth);

  अगर (lhs_शब्दगण == 1) { // rhsWords is 1 if lhsWords is 1.
    // There is only one word to consider so use the native versions.
    uint64_t lhs_मूल्य = LHS.U.pVal[0];
    Quotient = lhs_मूल्य / RHS;
    Remainder = lhs_मूल्य % RHS;
    वापस;
  }

  // Okay, lets do it the long way
  divide(LHS.U.pVal, lhs_शब्दगण, &RHS, 1, Quotient.U.pVal, &Remainder);
  // Clear the rest of the Quotient.
  std::स_रखो(Quotient.U.pVal + lhs_शब्दगण, 0,
              (get_अंक_शब्दगण(BitWidth) - lhs_शब्दगण) * APINT_WORD_आकार);
}

व्योम APInt::sdivrem(स्थिर APInt &LHS, स्थिर APInt &RHS,
                    APInt &Quotient, APInt &Remainder) {
  अगर (LHS.isNegative()) {
    अगर (RHS.isNegative())
      APInt::udivrem(-LHS, -RHS, Quotient, Remainder);
    अन्यथा {
      APInt::udivrem(-LHS, RHS, Quotient, Remainder);
      Quotient.negate();
    }
    Remainder.negate();
  } अन्यथा अगर (RHS.isNegative()) {
    APInt::udivrem(LHS, -RHS, Quotient, Remainder);
    Quotient.negate();
  } अन्यथा {
    APInt::udivrem(LHS, RHS, Quotient, Remainder);
  }
}

व्योम APInt::sdivrem(स्थिर APInt &LHS, int64_t RHS,
                    APInt &Quotient, int64_t &Remainder) {
  uint64_t R = Remainder;
  अगर (LHS.isNegative()) {
    अगर (RHS < 0)
      APInt::udivrem(-LHS, -RHS, Quotient, R);
    अन्यथा {
      APInt::udivrem(-LHS, RHS, Quotient, R);
      Quotient.negate();
    }
    R = -R;
  } अन्यथा अगर (RHS < 0) {
    APInt::udivrem(LHS, -RHS, Quotient, R);
    Quotient.negate();
  } अन्यथा {
    APInt::udivrem(LHS, RHS, Quotient, R);
  }
  Remainder = R;
}

APInt APInt::sadd_ov(स्थिर APInt &RHS, bool &Overflow) स्थिर {
  APInt Res = *यह+RHS;
  Overflow = isNonNegative() == RHS.isNonNegative() &&
             Res.isNonNegative() != isNonNegative();
  वापस Res;
}

APInt APInt::uadd_ov(स्थिर APInt &RHS, bool &Overflow) स्थिर {
  APInt Res = *यह+RHS;
  Overflow = Res.ult(RHS);
  वापस Res;
}

APInt APInt::ssub_ov(स्थिर APInt &RHS, bool &Overflow) स्थिर {
  APInt Res = *यह - RHS;
  Overflow = isNonNegative() != RHS.isNonNegative() &&
             Res.isNonNegative() != isNonNegative();
  वापस Res;
}

APInt APInt::usub_ov(स्थिर APInt &RHS, bool &Overflow) स्थिर {
  APInt Res = *यह-RHS;
  Overflow = Res.ugt(*यह);
  वापस Res;
}

APInt APInt::sdiv_ov(स्थिर APInt &RHS, bool &Overflow) स्थिर {
  // MININT/-1  -->  overflow.
  Overflow = is_Min_Signed_मूल्य() && RHS.isAllOnes();
  वापस sdiv(RHS);
}

APInt APInt::smul_ov(स्थिर APInt &RHS, bool &Overflow) स्थिर {
  APInt Res = *यह * RHS;

  अगर (RHS != 0)
    Overflow = Res.sdiv(RHS) != *यह ||
               (is_Min_Signed_मूल्य() && RHS.isAllOnes());
  अन्यथा
    Overflow = false;
  वापस Res;
}

APInt APInt::umul_ov(स्थिर APInt &RHS, bool &Overflow) स्थिर {
  अगर (countl_zero() + RHS.countl_zero() + 2 <= BitWidth) {
    Overflow = true;
    वापस *यह * RHS;
  }

  APInt Res = lshr(1) * RHS;
  Overflow = Res.isNegative();
  Res <<= 1;
  अगर ((*यह)[0]) {
    Res += RHS;
    अगर (Res.ult(RHS))
      Overflow = true;
  }
  वापस Res;
}

APInt APInt::sshl_ov(स्थिर APInt &ShAmt, bool &Overflow) स्थिर {
  वापस sshl_ov(ShAmt.get_Limited_मूल्य(getBitWidth()), Overflow);
}

APInt APInt::sshl_ov(अचिन्हित ShAmt, bool &Overflow) स्थिर {
  Overflow = ShAmt >= getBitWidth();
  अगर (Overflow)
    वापस APInt(BitWidth, 0);

  अगर (isNonNegative()) // Don't allow sign change.
    Overflow = ShAmt >= countl_zero();
  अन्यथा
    Overflow = ShAmt >= countl_one();

  वापस *यह << ShAmt;
}

APInt APInt::ushl_ov(स्थिर APInt &ShAmt, bool &Overflow) स्थिर {
  वापस ushl_ov(ShAmt.get_Limited_मूल्य(getBitWidth()), Overflow);
}

APInt APInt::ushl_ov(अचिन्हित ShAmt, bool &Overflow) स्थिर {
  Overflow = ShAmt >= getBitWidth();
  अगर (Overflow)
    वापस APInt(BitWidth, 0);

  Overflow = ShAmt > countl_zero();

  वापस *यह << ShAmt;
}

APInt APInt::sfloordiv_ov(स्थिर APInt &RHS, bool &Overflow) स्थिर {
  APInt quotient = sdiv_ov(RHS, Overflow);
  अगर ((quotient * RHS != *यह) && (isNegative() != RHS.isNegative()))
    वापस quotient - 1;
  वापस quotient;
}

APInt APInt::sadd_sat(स्थिर APInt &RHS) स्थिर {
  bool Overflow;
  APInt Res = sadd_ov(RHS, Overflow);
  अगर (!Overflow)
    वापस Res;

  वापस isNegative() ? APInt::get_Signed_Min_मूल्य(BitWidth)
                      : APInt::get_Signed_Max_मूल्य(BitWidth);
}

APInt APInt::uadd_sat(स्थिर APInt &RHS) स्थिर {
  bool Overflow;
  APInt Res = uadd_ov(RHS, Overflow);
  अगर (!Overflow)
    वापस Res;

  वापस APInt::get_Max_मूल्य(BitWidth);
}

APInt APInt::ssub_sat(स्थिर APInt &RHS) स्थिर {
  bool Overflow;
  APInt Res = ssub_ov(RHS, Overflow);
  अगर (!Overflow)
    वापस Res;

  वापस isNegative() ? APInt::get_Signed_Min_मूल्य(BitWidth)
                      : APInt::get_Signed_Max_मूल्य(BitWidth);
}

APInt APInt::usub_sat(स्थिर APInt &RHS) स्थिर {
  bool Overflow;
  APInt Res = usub_ov(RHS, Overflow);
  अगर (!Overflow)
    वापस Res;

  वापस APInt(BitWidth, 0);
}

APInt APInt::smul_sat(स्थिर APInt &RHS) स्थिर {
  bool Overflow;
  APInt Res = smul_ov(RHS, Overflow);
  अगर (!Overflow)
    वापस Res;

  // The result is negative if one and only one of inputs is negative.
  bool ResIsNegative = isNegative() ^ RHS.isNegative();

  वापस ResIsNegative ? APInt::get_Signed_Min_मूल्य(BitWidth)
                       : APInt::get_Signed_Max_मूल्य(BitWidth);
}

APInt APInt::umul_sat(स्थिर APInt &RHS) स्थिर {
  bool Overflow;
  APInt Res = umul_ov(RHS, Overflow);
  अगर (!Overflow)
    वापस Res;

  वापस APInt::get_Max_मूल्य(BitWidth);
}

APInt APInt::sshl_sat(स्थिर APInt &RHS) स्थिर {
  वापस sshl_sat(RHS.get_Limited_मूल्य(getBitWidth()));
}

APInt APInt::sshl_sat(अचिन्हित RHS) स्थिर {
  bool Overflow;
  APInt Res = sshl_ov(RHS, Overflow);
  अगर (!Overflow)
    वापस Res;

  वापस isNegative() ? APInt::get_Signed_Min_मूल्य(BitWidth)
                      : APInt::get_Signed_Max_मूल्य(BitWidth);
}

APInt APInt::ushl_sat(स्थिर APInt &RHS) स्थिर {
  वापस ushl_sat(RHS.get_Limited_मूल्य(getBitWidth()));
}

APInt APInt::ushl_sat(अचिन्हित RHS) स्थिर {
  bool Overflow;
  APInt Res = ushl_ov(RHS, Overflow);
  अगर (!Overflow)
    वापस Res;

  वापस APInt::get_Max_मूल्य(BitWidth);
}

व्योम APInt::fromString(अचिन्हित numbits, StringRef str, uint8_t radix) {
  // Check our assumptions here
  निश्चित(!str.empty() && "Invalid string length");
  निश्चित((radix == 10 || radix == 8 || radix == 16 || radix == 2 ||
          radix == 36) &&
         "Radix should be 2, 8, 10, 16, or 36!");

  StringRef::iterator p = str.begin();
  माप_प्रकार slen = str.आकार_2();
  bool isNeg = *p == '-';
  अगर (*p == '-' || *p == '+') {
    p++;
    slen--;
    निश्चित(slen && "String is only a sign, needs a value.");
  }
  निश्चित((slen <= numbits || radix != 2) && "Insufficient bit width");
  निश्चित(((slen-1)*3 <= numbits || radix != 8) && "Insufficient bit width");
  निश्चित(((slen-1)*4 <= numbits || radix != 16) && "Insufficient bit width");
  निश्चित((((slen-1)*64)/22 <= numbits || radix != 10) &&
         "Insufficient bit width");

  // Allocate memory if needed
  अगर (LLVM_LIKELY(isSingleWord()))
    U.VAL = 0;
  अन्यथा
    U.pVal = getClearedMemory(get_अंक_शब्दगण());

  // Figure out if we can shift instead of multiply
  अचिन्हित shift = (radix == 16 ? 4 : radix == 8 ? 3 : radix == 2 ? 1 : 0);

  // Enter digit traversal loop
  क्रम (StringRef::iterator e = str.end(); p != e; ++p) {
    अचिन्हित digit = getDigit(*p, radix);
    निश्चित(digit < radix && "Invalid character in digit string");

    // Shift or multiply the value by the radix
    अगर (slen > 1) {
      अगर (shift)
        *यह <<= shift;
      अन्यथा
        *यह *= radix;
    }

    // Add in the digit we just interpreted
    *यह += digit;
  }
  // If its negative, put it in two's complement form
  अगर (isNeg)
    यह->negate();
}

व्योम APInt::toString(SmallVectorImpl<अक्षर> &Str, अचिन्हित Radix, bool Signed,
                     bool formatAsCLiteral, bool UpperCase,
                     bool जोड़ो_Separators) स्थिर {
  निश्चित((Radix == 10 || Radix == 8 || Radix == 16 || Radix == 2 ||
          Radix == 36) &&
         "Radix should be 2, 8, 10, 16, or 36!");

  स्थिर अक्षर *Prefix = "";
  अगर (formatAsCLiteral) {
    चयन (Radix) {
      हाल 2:
        // Binary literals are a non-standard extension added in gcc 4.3:
        // http://gcc.gnu.org/onlinedocs/gcc-4.3.0/gcc/Binary-constants.html
        Prefix = "0b";
        अवरोध;
      हाल 8:
        Prefix = "0";
        अवरोध;
      हाल 10:
        अवरोध; // No prefix
      हाल 16:
        Prefix = "0x";
        अवरोध;
      शेष:
        llvm_unreachable("Invalid radix!");
    }
  }

  // Number of digits in a group between separators.
  अचिन्हित Grouping = (Radix == 8 || Radix == 10) ? 3 : 4;

  // First, check for a zero value and just short circuit the logic below.
  अगर (isZero()) {
    जबतक (*Prefix) {
      Str.धकेलो_back(*Prefix);
      ++Prefix;
    };
    Str.धकेलो_back('0');
    वापस;
  }

  अटल स्थिर अक्षर BothDigits[] = "0123456789abcdefghijklmnopqrstuvwxyz"
                                   "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ";
  स्थिर अक्षर *Digits = BothDigits + (UpperCase ? 36 : 0);

  अगर (LLVM_LIKELY(isSingleWord())) {
    अक्षर बफर[65];
    अक्षर *BufPtr = std::end(बफर);

    uint64_t N;
    अगर (!Signed) {
      N = get_ZExt_मूल्य();
    } अन्यथा {
      int64_t I = get_SExt_मूल्य();
      अगर (I >= 0) {
        N = I;
      } अन्यथा {
        Str.धकेलो_back('-');
        N = -(uint64_t)I;
      }
    }

    जबतक (*Prefix) {
      Str.धकेलो_back(*Prefix);
      ++Prefix;
    };

    पूर्णांक Pos = 0;
    जबतक (N) {
      अगर (जोड़ो_Separators && Pos % Grouping == 0 && Pos > 0)
        *--BufPtr = '\'';
      *--BufPtr = Digits[N % Radix];
      N /= Radix;
      Pos++;
    }
    Str.append(BufPtr, std::end(बफर));
    वापस;
  }

  APInt Tmp(*यह);

  अगर (Signed && isNegative()) {
    // They want to print the signed version and it is a negative value
    // Flip the bits and add one to turn it into the equivalent positive
    // value and put a '-' in the result.
    Tmp.negate();
    Str.धकेलो_back('-');
  }

  जबतक (*Prefix) {
    Str.धकेलो_back(*Prefix);
    ++Prefix;
  }

  // We insert the digits backward, then reverse them to get the right order.
  अचिन्हित StartDig = Str.आकार_2();

  // For the 2, 8 and 16 bit cases, we can just shift instead of divide
  // because the number of bits per digit (1, 3 and 4 respectively) divides
  // equally.  We just shift until the value is zero.
  अगर (Radix == 2 || Radix == 8 || Radix == 16) {
    // Just shift tmp right for each digit width until it becomes zero
    अचिन्हित ShiftAmt = (Radix == 16 ? 4 : (Radix == 8 ? 3 : 1));
    अचिन्हित MaskAmt = Radix - 1;

    पूर्णांक Pos = 0;
    जबतक (Tmp.get_Bool_मूल्य()) {
      अचिन्हित Digit = अचिन्हित(Tmp.get_Raw_तथ्य()[0]) & MaskAmt;
      अगर (जोड़ो_Separators && Pos % Grouping == 0 && Pos > 0)
        Str.धकेलो_back('\'');

      Str.धकेलो_back(Digits[Digit]);
      Tmp.lshrInPlace(ShiftAmt);
      Pos++;
    }
  } अन्यथा {
    पूर्णांक Pos = 0;
    जबतक (Tmp.get_Bool_मूल्य()) {
      uint64_t Digit;
      udivrem(Tmp, Radix, Tmp, Digit);
      निश्चित(Digit < Radix && "divide failed");
      अगर (जोड़ो_Separators && Pos % Grouping == 0 && Pos > 0)
        Str.धकेलो_back('\'');

      Str.धकेलो_back(Digits[Digit]);
      Pos++;
    }
  }

  // Reverse the digits before returning.
  std::reverse(Str.begin()+StartDig, Str.end());
}

#अगर !defined(न_संशोधन) || defined(LLVM_ENABLE_DUMP)
LLVM_DUMP_METHOD व्योम APInt::dump() स्थिर {
  SmallString<40> S, U;
  यह->toStringUnsigned(U);
  यह->toStringSigned(S);
  dbgs() << "APInt(" << BitWidth << "b, "
         << U << "u " << S << "s)\n";
}
#पूर्ण_अगर

व्योम APInt::print(raw_ostream &OS, bool isSigned) स्थिर {
  SmallString<40> S;
  यह->toString(S, 10, isSigned, /* formatAsCLiteral = */false);
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
अटल अंतरभूत APInt::WordType निचला_Bit_Mask(अचिन्हित bits) {
  निश्चित(bits != 0 && bits <= APInt::APINT_BITS_PER_WORD);
  वापस ~(APInt::WordType) 0 >> (APInt::APINT_BITS_PER_WORD - bits);
}

/// Returns the value of the lower half of PART.
अटल अंतरभूत APInt::WordType निचला_Half(APInt::WordType part) {
  वापस part & निचला_Bit_Mask(APInt::APINT_BITS_PER_WORD / 2);
}

/// Returns the value of the upper half of PART.
अटल अंतरभूत APInt::WordType ऊपरी_Half(APInt::WordType part) {
  वापस part >> (APInt::APINT_BITS_PER_WORD / 2);
}

/// Sets the least significant part of a bignum to the input value, and zeroes
/// out higher parts.
व्योम APInt::tcSet(WordType *dst, WordType part, अचिन्हित parts) {
  निश्चित(parts > 0);
  dst[0] = part;
  क्रम (अचिन्हित i = 1; i < parts; i++)
    dst[i] = 0;
}

/// Assign one bignum to another.
व्योम APInt::tcAssign(WordType *dst, स्थिर WordType *src, अचिन्हित parts) {
  क्रम (अचिन्हित i = 0; i < parts; i++)
    dst[i] = src[i];
}

/// Returns true if a bignum is zero, false otherwise.
bool APInt::tcIsZero(स्थिर WordType *src, अचिन्हित parts) {
  क्रम (अचिन्हित i = 0; i < parts; i++)
    अगर (src[i])
      वापस false;

  वापस true;
}

/// Extract the given bit of a bignum; returns 0 or 1.
पूर्णांक APInt::tcExtractBit(स्थिर WordType *parts, अचिन्हित bit) {
  वापस (parts[whichWord(bit)] & maskBit(bit)) != 0;
}

/// Set the given bit of a bignum.
व्योम APInt::tcSetBit(WordType *parts, अचिन्हित bit) {
  parts[whichWord(bit)] |= maskBit(bit);
}

/// Clears the given bit of a bignum.
व्योम APInt::tcClearBit(WordType *parts, अचिन्हित bit) {
  parts[whichWord(bit)] &= ~maskBit(bit);
}

/// Returns the bit number of the least significant set bit of a number.  If the
/// input number has no bits set UINT_MAX is returned.
अचिन्हित APInt::tcLSB(स्थिर WordType *parts, अचिन्हित n) {
  क्रम (अचिन्हित i = 0; i < n; i++) {
    अगर (parts[i] != 0) {
      अचिन्हित lsb = llvm::countr_zero(parts[i]);
      वापस lsb + i * APINT_BITS_PER_WORD;
    }
  }

  वापस अच_पूर्णांक_उच्च;
}

/// Returns the bit number of the most significant set bit of a number.
/// If the input number has no bits set UINT_MAX is returned.
अचिन्हित APInt::tcMSB(स्थिर WordType *parts, अचिन्हित n) {
  करो {
    --n;

    अगर (parts[n] != 0) {
      static_assert(माप(parts[n]) <= माप(uint64_t));
      अचिन्हित msb = llvm::Log2_64(parts[n]);

      वापस msb + n * APINT_BITS_PER_WORD;
    }
  } जबतक (n);

  वापस अच_पूर्णांक_उच्च;
}

/// Copy the bit vector of width srcBITS from SRC, starting at bit srcLSB, to
/// DST, of dstCOUNT parts, such that the bit srcLSB becomes the least
/// significant bit of DST.  All high bits above srcBITS in DST are zero-filled.
/// */
व्योम
APInt::tcExtract(WordType *dst, अचिन्हित dst_गिनती, स्थिर WordType *src,
                 अचिन्हित srcBits, अचिन्हित srcLSB) {
  अचिन्हित dstParts = (srcBits + APINT_BITS_PER_WORD - 1) / APINT_BITS_PER_WORD;
  निश्चित(dstParts <= dst_गिनती);

  अचिन्हित firstSrcPart = srcLSB / APINT_BITS_PER_WORD;
  tcAssign(dst, src + firstSrcPart, dstParts);

  अचिन्हित shift = srcLSB % APINT_BITS_PER_WORD;
  tc_Shift_दायाँ(dst, dstParts, shift);

  // We now have (dstParts * APINT_BITS_PER_WORD - shift) bits from SRC
  // in DST.  If this is less that srcBits, append the rest, else
  // clear the high bits.
  अचिन्हित n = dstParts * APINT_BITS_PER_WORD - shift;
  अगर (n < srcBits) {
    WordType mask = निचला_Bit_Mask (srcBits - n);
    dst[dstParts - 1] |= ((src[firstSrcPart + dstParts] & mask)
                          << n % APINT_BITS_PER_WORD);
  } अन्यथा अगर (n > srcBits) {
    अगर (srcBits % APINT_BITS_PER_WORD)
      dst[dstParts - 1] &= निचला_Bit_Mask (srcBits % APINT_BITS_PER_WORD);
  }

  // Clear high parts.
  जबतक (dstParts < dst_गिनती)
    dst[dstParts++] = 0;
}

//// DST += RHS + C where C is zero or one.  Returns the carry flag.
APInt::WordType APInt::tcAdd(WordType *dst, स्थिर WordType *rhs,
                             WordType c, अचिन्हित parts) {
  निश्चित(c <= 1);

  क्रम (अचिन्हित i = 0; i < parts; i++) {
    WordType l = dst[i];
    अगर (c) {
      dst[i] += rhs[i] + 1;
      c = (dst[i] <= l);
    } अन्यथा {
      dst[i] += rhs[i];
      c = (dst[i] < l);
    }
  }

  वापस c;
}

/// This function adds a single "word" integer, src, to the multiple
/// "word" integer array, dst[]. dst[] is modified to reflect the addition and
/// 1 is returned if there is a carry out, otherwise 0 is returned.
/// @returns the carry of the addition.
APInt::WordType APInt::tcAddPart(WordType *dst, WordType src,
                                 अचिन्हित parts) {
  क्रम (अचिन्हित i = 0; i < parts; ++i) {
    dst[i] += src;
    अगर (dst[i] >= src)
      वापस 0; // No need to carry so exit early.
    src = 1; // Carry one to next digit.
  }

  वापस 1;
}

/// DST -= RHS + C where C is zero or one.  Returns the carry flag.
APInt::WordType APInt::tcSubtract(WordType *dst, स्थिर WordType *rhs,
                                  WordType c, अचिन्हित parts) {
  निश्चित(c <= 1);

  क्रम (अचिन्हित i = 0; i < parts; i++) {
    WordType l = dst[i];
    अगर (c) {
      dst[i] -= rhs[i] + 1;
      c = (dst[i] >= l);
    } अन्यथा {
      dst[i] -= rhs[i];
      c = (dst[i] > l);
    }
  }

  वापस c;
}

/// This function subtracts a single "word" (64-bit word), src, from
/// the multi-word integer array, dst[], propagating the borrowed 1 value until
/// no further borrowing is needed or it runs out of "words" in dst.  The result
/// is 1 if "borrowing" exhausted the digits in dst, or 0 if dst was not
/// exhausted. In other words, if src > dst then this function returns 1,
/// otherwise 0.
/// @returns the borrow out of the subtraction
APInt::WordType APInt::tcSubtractPart(WordType *dst, WordType src,
                                      अचिन्हित parts) {
  क्रम (अचिन्हित i = 0; i < parts; ++i) {
    WordType Dst = dst[i];
    dst[i] -= src;
    अगर (src <= Dst)
      वापस 0; // No need to borrow so exit early.
    src = 1; // We have to "borrow 1" from next "word"
  }

  वापस 1;
}

/// Negate a bignum in-place.
व्योम APInt::tcNegate(WordType *dst, अचिन्हित parts) {
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
पूर्णांक APInt::tcMultiplyPart(WordType *dst, स्थिर WordType *src,
                          WordType multiplier, WordType carry,
                          अचिन्हित srcParts, अचिन्हित dstParts,
                          bool add) {
  // Otherwise our writes of DST kill our later reads of SRC.
  निश्चित(dst <= src || dst >= src + srcParts);
  निश्चित(dstParts <= srcParts + 1);

  // N loops; minimum of dstParts and srcParts.
  अचिन्हित n = std::min(dstParts, srcParts);

  क्रम (अचिन्हित i = 0; i < n; i++) {
    // [LOW, HIGH] = MULTIPLIER * SRC[i] + DST[i] + CARRY.
    // This cannot overflow, because:
    //   (n - 1) * (n - 1) + 2 (n - 1) = (n - 1) * (n + 1)
    // which is less than n^2.
    WordType srcPart = src[i];
    WordType निचला, mid, ऊपरी;
    अगर (multiplier == 0 || srcPart == 0) {
      निचला = carry;
      ऊपरी = 0;
    } अन्यथा {
      निचला = निचला_Half(srcPart) * निचला_Half(multiplier);
      ऊपरी = ऊपरी_Half(srcPart) * ऊपरी_Half(multiplier);

      mid = निचला_Half(srcPart) * ऊपरी_Half(multiplier);
      ऊपरी += ऊपरी_Half(mid);
      mid <<= APINT_BITS_PER_WORD / 2;
      अगर (निचला + mid < निचला)
        ऊपरी++;
      निचला += mid;

      mid = ऊपरी_Half(srcPart) * निचला_Half(multiplier);
      ऊपरी += ऊपरी_Half(mid);
      mid <<= APINT_BITS_PER_WORD / 2;
      अगर (निचला + mid < निचला)
        ऊपरी++;
      निचला += mid;

      // Now add carry.
      अगर (निचला + carry < निचला)
        ऊपरी++;
      निचला += carry;
    }

    अगर (add) {
      // And now DST[i], and store the new low part there.
      अगर (निचला + dst[i] < निचला)
        ऊपरी++;
      dst[i] += निचला;
    } अन्यथा {
      dst[i] = निचला;
    }

    carry = ऊपरी;
  }

  अगर (srcParts < dstParts) {
    // Full multiplication, there is no overflow.
    निश्चित(srcParts + 1 == dstParts);
    dst[srcParts] = carry;
    वापस 0;
  }

  // We overflowed if there is carry.
  अगर (carry)
    वापस 1;

  // We would overflow if any significant unwritten parts would be
  // non-zero.  This is true if any remaining src parts are non-zero
  // and the multiplier is non-zero.
  अगर (multiplier)
    क्रम (अचिन्हित i = dstParts; i < srcParts; i++)
      अगर (src[i])
        वापस 1;

  // We fitted in the narrow destination.
  वापस 0;
}

/// DST = LHS * RHS, where DST has the same width as the operands and
/// is filled with the least significant parts of the result.  Returns
/// one if overflow occurred, otherwise zero.  DST must be disjoint
/// from both operands.
पूर्णांक APInt::tcMultiply(WordType *dst, स्थिर WordType *lhs,
                      स्थिर WordType *rhs, अचिन्हित parts) {
  निश्चित(dst != lhs && dst != rhs);

  पूर्णांक overflow = 0;

  क्रम (अचिन्हित i = 0; i < parts; i++) {
    // Don't accumulate on the first iteration so we don't need to initalize
    // dst to 0.
    overflow |=
        tcMultiplyPart(&dst[i], lhs, rhs[i], 0, parts, parts - i, i != 0);
  }

  वापस overflow;
}

/// DST = LHS * RHS, where DST has width the sum of the widths of the
/// operands. No overflow occurs. DST must be disjoint from both operands.
व्योम APInt::tcFullMultiply(WordType *dst, स्थिर WordType *lhs,
                           स्थिर WordType *rhs, अचिन्हित lhsParts,
                           अचिन्हित rhsParts) {
  // Put the narrower number on the LHS for less loops below.
  अगर (lhsParts > rhsParts)
    वापस tcFullMultiply (dst, rhs, lhs, rhsParts, lhsParts);

  निश्चित(dst != lhs && dst != rhs);

  क्रम (अचिन्हित i = 0; i < lhsParts; i++) {
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
पूर्णांक APInt::tcDivide(WordType *lhs, स्थिर WordType *rhs,
                    WordType *remainder, WordType *srhs,
                    अचिन्हित parts) {
  निश्चित(lhs != remainder && lhs != srhs && remainder != srhs);

  अचिन्हित shift_गिनती = tcMSB(rhs, parts) + 1;
  अगर (shift_गिनती == 0)
    वापस true;

  shift_गिनती = parts * APINT_BITS_PER_WORD - shift_गिनती;
  अचिन्हित n = shift_गिनती / APINT_BITS_PER_WORD;
  WordType mask = (WordType) 1 << (shift_गिनती % APINT_BITS_PER_WORD);

  tcAssign(srhs, rhs, parts);
  tc_Shift_बायाँ(srhs, parts, shift_गिनती);
  tcAssign(remainder, lhs, parts);
  tcSet(lhs, 0, parts);

  // Loop, subtracting SRHS if REMAINDER is greater and adding that to the
  // total.
  क्रम (;;) {
    पूर्णांक compare = tcCompare(remainder, srhs, parts);
    अगर (compare >= 0) {
      tcSubtract(remainder, srhs, 0, parts);
      lhs[n] |= mask;
    }

    अगर (shift_गिनती == 0)
      अवरोध;
    shift_गिनती--;
    tc_Shift_दायाँ(srhs, parts, 1);
    अगर ((mask >>= 1) == 0) {
      mask = (WordType) 1 << (APINT_BITS_PER_WORD - 1);
      n--;
    }
  }

  वापस false;
}

/// Shift a bignum left Count bits in-place. Shifted in bits are zero. There are
/// no restrictions on Count.
व्योम APInt::tc_Shift_बायाँ(WordType *Dst, अचिन्हित शब्दगण_2, अचिन्हित गिनती_2) {
  // Don't bother performing a no-op shift.
  अगर (!गिनती_2)
    वापस;

  // WordShift is the inter-part shift; BitShift is the intra-part shift.
  अचिन्हित WordShift = std::min(गिनती_2 / APINT_BITS_PER_WORD, शब्दगण_2);
  अचिन्हित BitShift = गिनती_2 % APINT_BITS_PER_WORD;

  // Fastpath for moving by whole words.
  अगर (BitShift == 0) {
    std::स_हटाओ(Dst + WordShift, Dst, (शब्दगण_2 - WordShift) * APINT_WORD_आकार);
  } अन्यथा {
    जबतक (शब्दगण_2-- > WordShift) {
      Dst[शब्दगण_2] = Dst[शब्दगण_2 - WordShift] << BitShift;
      अगर (शब्दगण_2 > WordShift)
        Dst[शब्दगण_2] |=
          Dst[शब्दगण_2 - WordShift - 1] >> (APINT_BITS_PER_WORD - BitShift);
    }
  }

  // Fill in the remainder with 0s.
  std::स_रखो(Dst, 0, WordShift * APINT_WORD_आकार);
}

/// Shift a bignum right Count bits in-place. Shifted in bits are zero. There
/// are no restrictions on Count.
व्योम APInt::tc_Shift_दायाँ(WordType *Dst, अचिन्हित शब्दगण_2, अचिन्हित गिनती_2) {
  // Don't bother performing a no-op shift.
  अगर (!गिनती_2)
    वापस;

  // WordShift is the inter-part shift; BitShift is the intra-part shift.
  अचिन्हित WordShift = std::min(गिनती_2 / APINT_BITS_PER_WORD, शब्दगण_2);
  अचिन्हित BitShift = गिनती_2 % APINT_BITS_PER_WORD;

  अचिन्हित शब्दगण_To_Move = शब्दगण_2 - WordShift;
  // Fastpath for moving by whole words.
  अगर (BitShift == 0) {
    std::स_हटाओ(Dst, Dst + WordShift, शब्दगण_To_Move * APINT_WORD_आकार);
  } अन्यथा {
    क्रम (अचिन्हित i = 0; i != शब्दगण_To_Move; ++i) {
      Dst[i] = Dst[i + WordShift] >> BitShift;
      अगर (i + 1 != शब्दगण_To_Move)
        Dst[i] |= Dst[i + WordShift + 1] << (APINT_BITS_PER_WORD - BitShift);
    }
  }

  // Fill in the remainder with 0s.
  std::स_रखो(Dst + शब्दगण_To_Move, 0, WordShift * APINT_WORD_आकार);
}

// Comparison (unsigned) of two bignums.
पूर्णांक APInt::tcCompare(स्थिर WordType *lhs, स्थिर WordType *rhs,
                     अचिन्हित parts) {
  जबतक (parts) {
    parts--;
    अगर (lhs[parts] != rhs[parts])
      वापस (lhs[parts] > rhs[parts]) ? 1 : -1;
  }

  वापस 0;
}

APInt llvm::APIntOps::RoundingUDiv(स्थिर APInt &A, स्थिर APInt &B,
                                   APInt::Rounding RM) {
  // Currently udivrem always rounds down.
  चयन (RM) {
  हाल APInt::Rounding::DOWN:
  हाल APInt::Rounding::TOWARD_ZERO:
    वापस A.udiv(B);
  हाल APInt::Rounding::UP: {
    APInt Quo, Rem;
    APInt::udivrem(A, B, Quo, Rem);
    अगर (Rem.isZero())
      वापस Quo;
    वापस Quo + 1;
  }
  }
  llvm_unreachable("Unknown APInt::Rounding enum");
}

APInt llvm::APIntOps::RoundingSDiv(स्थिर APInt &A, स्थिर APInt &B,
                                   APInt::Rounding RM) {
  चयन (RM) {
  हाल APInt::Rounding::DOWN:
  हाल APInt::Rounding::UP: {
    APInt Quo, Rem;
    APInt::sdivrem(A, B, Quo, Rem);
    अगर (Rem.isZero())
      वापस Quo;
    // This algorithm deals with arbitrary rounding mode used by sdivrem.
    // We want to check whether the non-integer part of the mathematical value
    // is negative or not. If the non-integer part is negative, we need to round
    // down from Quo; otherwise, if it's positive or 0, we return Quo, as it's
    // already rounded down.
    अगर (RM == APInt::Rounding::DOWN) {
      अगर (Rem.isNegative() != B.isNegative())
        वापस Quo - 1;
      वापस Quo;
    }
    अगर (Rem.isNegative() != B.isNegative())
      वापस Quo;
    वापस Quo + 1;
  }
  // Currently sdiv rounds towards zero.
  हाल APInt::Rounding::TOWARD_ZERO:
    वापस A.sdiv(B);
  }
  llvm_unreachable("Unknown APInt::Rounding enum");
}

std::optional<APInt>
llvm::APIntOps::SolveQuadraticEquationWrap(APInt A, APInt B, APInt C,
                                           अचिन्हित RangeWidth) {
  अचिन्हित CoeffWidth = A.getBitWidth();
  निश्चित(CoeffWidth == B.getBitWidth() && CoeffWidth == C.getBitWidth());
  निश्चित(RangeWidth <= CoeffWidth &&
         "Value range width should be less than coefficient width");
  निश्चित(RangeWidth > 1 && "Value range bit width should be > 1");

  LLVM_DEBUG(dbgs() << __func__ << ": solving " << A << "x^2 + " << B
                    << "x + " << C << ", rw:" << RangeWidth << '\n');

  // Identify 0 as a (non)solution immediately.
  अगर (C.sextOrTrunc(RangeWidth).isZero()) {
    LLVM_DEBUG(dbgs() << __func__ << ": zero solution\n");
    वापस APInt(CoeffWidth, 0);
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
  अगर (A.isNegative()) {
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
  bool Pick_निचला;

  स्वतः RoundUp = [] (स्थिर APInt &V, स्थिर APInt &A) -> APInt {
    निश्चित(A.isStrictlyPositive());
    APInt T = V.असल().urem(A);
    अगर (T.isZero())
      वापस V;
    वापस V.isNegative() ? V+T : V+(A-T);
  };

  // The vertex of the parabola is at -B/2A, but since A > 0, it's negative
  // iff B is positive.
  अगर (B.isNonNegative()) {
    // If B >= 0, the vertex it at a negative location (or at 0), so in
    // order to have a non-negative solution we need to pick k that makes
    // C-kR negative. To satisfy all the requirements for the solution
    // that we are looking for, it needs to be closest to 0 of all k.
    C = C.srem(R);
    अगर (C.isStrictlyPositive())
      C -= R;
    // Pick the greater solution.
    Pick_निचला = false;
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
    अगर (C.sgt(LowkR)) {
      // If LowkR < C, then such a k is guaranteed to exist because
      // LowkR itself is a multiple of R.
      C -= -RoundUp(-C, R);      // C = C - RoundDown(C, R)
      // Pick the smaller solution.
      Pick_निचला = true;
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
      Pick_निचला = false;
    }
  }

  LLVM_DEBUG(dbgs() << __func__ << ": updated coefficients " << A << "x^2 + "
                    << B << "x + " << C << ", rw:" << RangeWidth << '\n');

  APInt D = SqrB - 4*A*C;
  निश्चित(D.isNonNegative() && "Negative discriminant");
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
  अगर (Pick_निचला)
    APInt::sdivrem(-B - (SQ+InexactSQ), TwoA, X, Rem);
  अन्यथा
    APInt::sdivrem(-B + SQ, TwoA, X, Rem);

  // The updated coefficients should be such that the (exact) solution is
  // positive. Since APInt division rounds towards 0, the calculated one
  // can be 0, but cannot be negative.
  निश्चित(X.isNonNegative() && "Solution should be non-negative");

  अगर (!InexactSQ && Rem.isZero()) {
    LLVM_DEBUG(dbgs() << __func__ << ": solution (root): " << X << '\n');
    वापस X;
  }

  निश्चित((SQ*SQ).sle(D) && "SQ = |_sqrt(D)_|, so SQ*SQ <= D");
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
  अगर (!SignChange) {
    LLVM_DEBUG(dbgs() << __func__ << ": no valid solution\n");
    वापस std::nullopt;
  }

  X += 1;
  LLVM_DEBUG(dbgs() << __func__ << ": solution (wrap): " << X << '\n');
  वापस X;
}

std::optional<अचिन्हित>
llvm::APIntOps::GetMostSignificantDifferentBit(स्थिर APInt &A, स्थिर APInt &B) {
  निश्चित(A.getBitWidth() == B.getBitWidth() && "Must have the same bitwidth");
  अगर (A == B)
    वापस std::nullopt;
  वापस A.getBitWidth() - ((A ^ B).countl_zero() + 1);
}

APInt llvm::APIntOps::ScaleBitMask(स्थिर APInt &A, अचिन्हित NewBitWidth,
                                   bool MatchAllBits) {
  अचिन्हित OldBitWidth = A.getBitWidth();
  निश्चित((((OldBitWidth % NewBitWidth) == 0) ||
          ((NewBitWidth % OldBitWidth) == 0)) &&
         "One size should be a multiple of the other one. "
         "Can't do fractional scaling.");

  // Check for matching bitwidths.
  अगर (OldBitWidth == NewBitWidth)
    वापस A;

  APInt NewA = APInt::getZero(NewBitWidth);

  // Check for null input.
  अगर (A.isZero())
    वापस NewA;

  अगर (NewBitWidth > OldBitWidth) {
    // Repeat bits.
    अचिन्हित Scale = NewBitWidth / OldBitWidth;
    क्रम (अचिन्हित i = 0; i != OldBitWidth; ++i)
      अगर (A[i])
        NewA.setBits(i * Scale, (i + 1) * Scale);
  } अन्यथा {
    अचिन्हित Scale = OldBitWidth / NewBitWidth;
    क्रम (अचिन्हित i = 0; i != NewBitWidth; ++i) {
      अगर (MatchAllBits) {
        अगर (A.extractBits(Scale, i * Scale).isAllOnes())
          NewA.setBit(i);
      } अन्यथा {
        अगर (!A.extractBits(Scale, i * Scale).isZero())
          NewA.setBit(i);
      }
    }
  }

  वापस NewA;
}

/// StoreIntToMemory - Fills the StoreBytes bytes of memory starting from Dst
/// with the integer held in IntVal.
व्योम llvm::StoreIntToMemory(स्थिर APInt &IntVal, uint8_t *Dst,
                            अचिन्हित StoreBytes) {
  निश्चित((IntVal.getBitWidth()+7)/8 >= StoreBytes && "Integer too small!");
  स्थिर uint8_t *Src = (स्थिर uint8_t *)IntVal.get_Raw_तथ्य();

  अगर (sys::IsLittleEndianHost) {
    // Little-endian host - the source is ordered from LSB to MSB.  Order the
    // destination from LSB to MSB: Do a straight copy.
    स_नकल(Dst, Src, StoreBytes);
  } अन्यथा {
    // Big-endian host - the source is an array of 64 bit words ordered from
    // LSW to MSW.  Each word is ordered from MSB to LSB.  Order the destination
    // from MSB to LSB: Reverse the word order, but not the bytes in a word.
    जबतक (StoreBytes > माप(uint64_t)) {
      StoreBytes -= माप(uint64_t);
      // May not be aligned so use memcpy.
      स_नकल(Dst + StoreBytes, Src, माप(uint64_t));
      Src += माप(uint64_t);
    }

    स_नकल(Dst, Src + माप(uint64_t) - StoreBytes, StoreBytes);
  }
}

/// LoadIntFromMemory - Loads the integer stored in the LoadBytes bytes starting
/// from Src into IntVal, which is assumed to be wide enough and to hold zero.
व्योम llvm::LoadIntFromMemory(APInt &IntVal, स्थिर uint8_t *Src,
                             अचिन्हित LoadBytes) {
  निश्चित((IntVal.getBitWidth()+7)/8 >= LoadBytes && "Integer too small!");
  uint8_t *Dst = reinterpret_cast<uint8_t *>(
                   const_cast<uint64_t *>(IntVal.get_Raw_तथ्य()));

  अगर (sys::IsLittleEndianHost)
    // Little-endian host - the destination must be ordered from LSB to MSB.
    // The source is ordered from LSB to MSB: Do a straight copy.
    स_नकल(Dst, Src, LoadBytes);
  अन्यथा {
    // Big-endian - the destination is an array of 64 bit words ordered from
    // LSW to MSW.  Each word must be ordered from MSB to LSB.  The source is
    // ordered from MSB to LSB: Reverse the word order, but not the bytes in
    // a word.
    जबतक (LoadBytes > माप(uint64_t)) {
      LoadBytes -= माप(uint64_t);
      // May not be aligned so use memcpy.
      स_नकल(Dst, Src + LoadBytes, माप(uint64_t));
      Dst += माप(uint64_t);
    }

    स_नकल(Dst + माप(uint64_t) - LoadBytes, Src, LoadBytes);
  }
}

APInt APIntOps::avgFloorS(स्थिर APInt &C1, स्थिर APInt &C2) {
  // Return floor((C1 + C2) / 2)
  वापस (C1 & C2) + (C1 ^ C2).ashr(1);
}

APInt APIntOps::avgFloorU(स्थिर APInt &C1, स्थिर APInt &C2) {
  // Return floor((C1 + C2) / 2)
  वापस (C1 & C2) + (C1 ^ C2).lshr(1);
}

APInt APIntOps::avgCeilS(स्थिर APInt &C1, स्थिर APInt &C2) {
  // Return ceil((C1 + C2) / 2)
  वापस (C1 | C2) - (C1 ^ C2).ashr(1);
}

APInt APIntOps::avgCeilU(स्थिर APInt &C1, स्थिर APInt &C2) {
  // Return ceil((C1 + C2) / 2)
  वापस (C1 | C2) - (C1 ^ C2).lshr(1);
}

APInt APIntOps::avgSubFloorS(स्थिर APInt &C1, स्थिर APInt &C2) {
  // Return floor((C1 - C2) / 2), using C1 - C2 == (C1 ^ C2) - 2 * (~C1 & C2).
  वापस (C1 ^ C2).ashr(1) - (~C1 & C2);
}

APInt APIntOps::avgSubFloorU(स्थिर APInt &C1, स्थिर APInt &C2) {
  // Return floor((C1 - C2) / 2), using C1 - C2 == (C1 ^ C2) - 2 * (~C1 & C2).
  वापस (C1 ^ C2).lshr(1) - (~C1 & C2);
}

APInt APIntOps::mulhs(स्थिर APInt &C1, स्थिर APInt &C2) {
  निश्चित(C1.getBitWidth() == C2.getBitWidth() && "Unequal bitwidths");
  अचिन्हित FullWidth = C1.getBitWidth() * 2;
  APInt C1Ext = C1.sext(FullWidth);
  APInt C2Ext = C2.sext(FullWidth);
  वापस (C1Ext * C2Ext).extractBits(C1.getBitWidth(), C1.getBitWidth());
}

APInt APIntOps::mulhu(स्थिर APInt &C1, स्थिर APInt &C2) {
  निश्चित(C1.getBitWidth() == C2.getBitWidth() && "Unequal bitwidths");
  अचिन्हित FullWidth = C1.getBitWidth() * 2;
  APInt C1Ext = C1.zext(FullWidth);
  APInt C2Ext = C2.zext(FullWidth);
  वापस (C1Ext * C2Ext).extractBits(C1.getBitWidth(), C1.getBitWidth());
}

APInt APIntOps::mulsExtended(स्थिर APInt &C1, स्थिर APInt &C2) {
  निश्चित(C1.getBitWidth() == C2.getBitWidth() && "Unequal bitwidths");
  अचिन्हित FullWidth = C1.getBitWidth() * 2;
  APInt C1Ext = C1.sext(FullWidth);
  APInt C2Ext = C2.sext(FullWidth);
  वापस C1Ext * C2Ext;
}

APInt APIntOps::muluExtended(स्थिर APInt &C1, स्थिर APInt &C2) {
  निश्चित(C1.getBitWidth() == C2.getBitWidth() && "Unequal bitwidths");
  अचिन्हित FullWidth = C1.getBitWidth() * 2;
  APInt C1Ext = C1.zext(FullWidth);
  APInt C2Ext = C2.zext(FullWidth);
  वापस C1Ext * C2Ext;
}

APInt APIntOps::घात(स्थिर APInt &X, int64_t N) {
  निश्चित(N >= 0 && "negative exponents not supported.");
  APInt Acc = APInt(X.getBitWidth(), 1);
  अगर (N == 0)
    वापस Acc;
  APInt Base = X;
  int64_t RemainingExponent = N;
  जबतक (RemainingExponent > 0) {
    जबतक (RemainingExponent % 2 == 0) {
      Base *= Base;
      RemainingExponent /= 2;
    }
    --RemainingExponent;
    Acc *= Base;
  }
  वापस Acc;
}

APInt llvm::APIntOps::fshl(स्थिर APInt &Hi, स्थिर APInt &Lo,
                           स्थिर APInt &Shift) {
  निश्चित(Hi.getBitWidth() == Lo.getBitWidth());
  अचिन्हित ShiftAmt = rotateModulo(Hi.getBitWidth(), Shift);
  अगर (ShiftAmt == 0)
    वापस Hi;
  वापस Hi.shl(ShiftAmt) | Lo.lshr(Hi.getBitWidth() - ShiftAmt);
}

APInt llvm::APIntOps::fshr(स्थिर APInt &Hi, स्थिर APInt &Lo,
                           स्थिर APInt &Shift) {
  निश्चित(Hi.getBitWidth() == Lo.getBitWidth());
  अचिन्हित ShiftAmt = rotateModulo(Hi.getBitWidth(), Shift);
  अगर (ShiftAmt == 0)
    वापस Lo;
  वापस Hi.shl(Hi.getBitWidth() - ShiftAmt) | Lo.lshr(ShiftAmt);
}

APInt llvm::APIntOps::clmul(स्थिर APInt &LHS, स्थिर APInt &RHS) {
  अचिन्हित BW = LHS.getBitWidth();
  निश्चित(BW == RHS.getBitWidth() && "Operand mismatch");
  APInt परिणाम(BW, 0);
  क्रम (अचिन्हित I : seq(std::min(RHS.getActiveBits(), BW - LHS.countr_zero())))
    अगर (RHS[I])
      परिणाम ^= LHS << I;
  वापस परिणाम;
}

APInt llvm::APIntOps::clmulr(स्थिर APInt &LHS, स्थिर APInt &RHS) {
  निश्चित(LHS.getBitWidth() == RHS.getBitWidth());
  वापस clmul(LHS.reverseBits(), RHS.reverseBits()).reverseBits();
}

APInt llvm::APIntOps::clmulh(स्थिर APInt &LHS, स्थिर APInt &RHS) {
  निश्चित(LHS.getBitWidth() == RHS.getBitWidth());
  वापस clmulr(LHS, RHS).lshr(1);
}

APInt llvm::APIntOps::pext(स्थिर APInt &Val, स्थिर APInt &Mask) {
  अचिन्हित BW = Val.getBitWidth();
  निश्चित(BW == Mask.getBitWidth() && "Operand mismatch");
  APInt परिणाम = APInt::getZero(BW);
  क्रम (अचिन्हित I = 0, P = 0; I != BW; ++I)
    अगर (Mask[I])
      परिणाम.setBitVal(P++, Val[I]);
  वापस परिणाम;
}

APInt llvm::APIntOps::pdep(स्थिर APInt &Val, स्थिर APInt &Mask) {
  अचिन्हित BW = Val.getBitWidth();
  निश्चित(BW == Mask.getBitWidth() && "Operand mismatch");
  APInt परिणाम = APInt::getZero(BW);
  क्रम (अचिन्हित I = 0, P = 0; I != BW; ++I)
    अगर (Mask[I])
      परिणाम.setBitVal(I, Val[P++]);
  वापस परिणाम;
}
