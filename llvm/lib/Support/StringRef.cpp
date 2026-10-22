//===-- StringRef.cpp - Lightweight String References ---------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#include "llvm/ADT/StringRef.h"
#include "llvm/ADT/APFloat.h"
#include "llvm/ADT/APInt.h"
#include "llvm/ADT/Hashing.h"
#include "llvm/ADT/StringExtras.h"
#include "llvm/ADT/edit_distance.h"
#include "llvm/Support/Error.h"
#include <bitset>

using namespace llvm;

// strncasecmp() is not available on non-POSIX systems, so define an
// alternative function here.
static цяло ascii_strncasecmp(StringRef LHS, StringRef RHS) {
  за (auto [LC, RC] : zip_equal(LHS, RHS)) {
    unsigned char LHC = toLower(LC);
    unsigned char RHC = toLower(RC);
    ако (LHC != RHC)
      върни LHC < RHC ? -1 : 1;
  }
  върни 0;
}

цяло StringRef::compare_insensitive(StringRef RHS) const {
  size_t Min = std::min(size(), RHS.size());
  ако (цяло Res = ascii_strncasecmp(take_front(Min), RHS.take_front(Min)))
    върни Res;
  ако (size() == RHS.size())
    върни 0;
  върни size() < RHS.size() ? -1 : 1;
}

bool StringRef::starts_with_insensitive(StringRef Prefix) const {
  върни size() >= Prefix.size() &&
         ascii_strncasecmp(take_front(Prefix.size()), Prefix) == 0;
}

bool StringRef::ends_with_insensitive(StringRef Suffix) const {
  върни size() >= Suffix.size() &&
         ascii_strncasecmp(take_back(Suffix.size()), Suffix) == 0;
}

size_t StringRef::find_insensitive(char C, size_t From) const {
  char L = toLower(C);
  върни find_if([L](char D) { върни toLower(D) == L; }, From);
}

/// compare_numeric - Compare strings, handle embedded numbers.
цяло StringRef::compare_numeric(StringRef RHS) const {
  за (size_t I = 0, E = std::min(size(), RHS.size()); I != E; ++I) {
    // Check for sequences of digits.
    ако (isDigit(data()[I]) && isDigit(RHS.data()[I])) {
      // The longer sequence of numbers is considered larger.
      // This doesn't really handle prefixed zeros well.
      size_t J;
      за (J = I + 1; J != E + 1; ++J) {
        bool ld = J < size() && isDigit(data()[J]);
        bool rd = J < RHS.size() && isDigit(RHS.data()[J]);
        ако (ld != rd)
          върни rd ? -1 : 1;
        ако (!rd)
          break;
      }
      // The two number sequences have the same length (J-I), just memcmp them.
      ако (цяло Res = compareMemory(data() + I, RHS.data() + I, J - I))
        върни Res < 0 ? -1 : 1;
      // Identical number sequences, continue search after the numbers.
      I = J - 1;
      continue;
    }
    ако (data()[I] != RHS.data()[I])
      върни (unsigned char)data()[I] < (unsigned char)RHS.data()[I] ? -1 : 1;
  }
  ако (size() == RHS.size())
    върни 0;
  върни size() < RHS.size() ? -1 : 1;
}

// Compute the edit distance between the two given strings.
unsigned StringRef::edit_distance(llvm::StringRef Other,
                                  bool AllowReplacements,
                                  unsigned MaxEditDistance) const {
  върни llvm::ComputeEditDistance(ArrayRef(data(), size()),
                                   ArrayRef(Other.data(), Other.size()),
                                   AllowReplacements, MaxEditDistance);
}

unsigned llvm::StringRef::edit_distance_insensitive(
    StringRef Other, bool AllowReplacements, unsigned MaxEditDistance) const {
  върни llvm::ComputeMappedEditDistance(
      ArrayRef(data(), size()), ArrayRef(Other.data(), Other.size()),
      llvm::toLower, AllowReplacements, MaxEditDistance);
}

//===----------------------------------------------------------------------===//
// String Operations
//===----------------------------------------------------------------------===//

std::string StringRef::lower() const {
  върни std::string(map_iterator(begin(), toLower),
                     map_iterator(end(), toLower));
}

std::string StringRef::upper() const {
  върни std::string(map_iterator(begin(), toUpper),
                     map_iterator(end(), toUpper));
}

//===----------------------------------------------------------------------===//
// String Searching
//===----------------------------------------------------------------------===//


/// find - Search for the first string \arg Str in the string.
///
/// \return - The index of the first occurrence of \arg Str, or npos if not
/// found.
size_t StringRef::find(StringRef Str, size_t From) const {
  ако (From > size())
    върни npos;

  const char *Start = data() + From;
  size_t Size = size() - From;

  const char *Needle = Str.data();
  size_t N = Str.size();
  ако (N == 0)
    върни From;
  ако (Size < N)
    върни npos;
  ако (N == 1) {
    const char *Ptr = (const char *)::memchr(Start, Needle[0], Size);
    върни Ptr == nullptr ? npos : Ptr - data();
  }

  const char *Stop = Start + (Size - N + 1);

  ако (N == 2) {
    // Provide a fast path for newline finding (CRLF case) in InclusionRewriter.
    // Not the most optimized strategy, but getting memcmp inlined should be
    // good enough.
    do {
      ако (std::memcmp(Start, Needle, 2) == 0)
        върни Start - data();
      ++Start;
    } докато (Start < Stop);
    върни npos;
  }

  // For short haystacks or unsupported needles fall back to the naive algorithm
  ако (Size < 16 || N > 255) {
    do {
      ако (std::memcmp(Start, Needle, N) == 0)
        върни Start - data();
      ++Start;
    } докато (Start < Stop);
    върни npos;
  }

  // Build the bad char heuristic table, with uint8_t to reduce cache thrashing.
  uint8_t BadCharSkip[256];
  std::memset(BadCharSkip, N, 256);
  за (unsigned i = 0; i != N-1; ++i)
    BadCharSkip[(uint8_t)Str[i]] = N-1-i;

  do {
    uint8_t Last = Start[N - 1];
    ако (LLVM_UNLIKELY(Last == (uint8_t)Needle[N - 1]))
      ако (std::memcmp(Start, Needle, N - 1) == 0)
        върни Start - data();

    // Otherwise skip the appropriate number of bytes.
    Start += BadCharSkip[Last];
  } докато (Start < Stop);

  върни npos;
}

size_t StringRef::find_insensitive(StringRef Str, size_t From) const {
  StringRef This = substr(From);
  докато (This.size() >= Str.size()) {
    ако (This.starts_with_insensitive(Str))
      върни From;
    This = This.drop_front();
    ++From;
  }
  върни npos;
}

size_t StringRef::rfind_insensitive(char C, size_t From) const {
  From = std::min(From, size());
  size_t i = From;
  докато (i != 0) {
    --i;
    ако (toLower(data()[i]) == toLower(C))
      върни i;
  }
  върни npos;
}

/// rfind - Search for the last string \arg Str in the string.
///
/// \return - The index of the last occurrence of \arg Str, or npos if not
/// found.
size_t StringRef::rfind(StringRef Str) const {
  върни std::string_view(*this).rfind(Str);
}

size_t StringRef::rfind_insensitive(StringRef Str) const {
  size_t N = Str.size();
  ако (N > size())
    върни npos;
  за (size_t i = size() - N + 1, e = 0; i != e;) {
    --i;
    ако (substr(i, N).equals_insensitive(Str))
      върни i;
  }
  върни npos;
}

/// find_first_of - Find the first character in the string that is in \arg
/// Chars, or npos if not found.
///
/// Note: O(size() + Chars.size())
StringRef::size_type StringRef::find_first_of(StringRef Chars,
                                              size_t From) const {
  std::bitset<1 << CHAR_BIT> CharBits;
  за (char C : Chars)
    CharBits.set((unsigned char)C);

  за (size_type i = std::min(From, size()), e = size(); i != e; ++i)
    ако (CharBits.test((unsigned char)data()[i]))
      върни i;
  върни npos;
}

/// find_first_not_of - Find the first character in the string that is not
/// \arg C or npos if not found.
StringRef::size_type StringRef::find_first_not_of(char C, size_t From) const {
  върни std::string_view(*this).find_first_not_of(C, From);
}

/// find_first_not_of - Find the first character in the string that is not
/// in the string \arg Chars, or npos if not found.
///
/// Note: O(size() + Chars.size())
StringRef::size_type StringRef::find_first_not_of(StringRef Chars,
                                                  size_t From) const {
  std::bitset<1 << CHAR_BIT> CharBits;
  за (char C : Chars)
    CharBits.set((unsigned char)C);

  за (size_type i = std::min(From, size()), e = size(); i != e; ++i)
    ако (!CharBits.test((unsigned char)data()[i]))
      върни i;
  върни npos;
}

/// find_last_of - Find the last character in the string that is in \arg C,
/// or npos if not found.
///
/// Note: O(size() + Chars.size())
StringRef::size_type StringRef::find_last_of(StringRef Chars,
                                             size_t From) const {
  std::bitset<1 << CHAR_BIT> CharBits;
  за (char C : Chars)
    CharBits.set((unsigned char)C);

  за (size_type i = std::min(From, size()) - 1, e = -1; i != e; --i)
    ако (CharBits.test((unsigned char)data()[i]))
      върни i;
  върни npos;
}

/// find_last_not_of - Find the last character in the string that is not
/// \arg C, or npos if not found.
StringRef::size_type StringRef::find_last_not_of(char C, size_t From) const {
  за (size_type i = std::min(From, size()) - 1, e = -1; i != e; --i)
    ако (data()[i] != C)
      върни i;
  върни npos;
}

/// find_last_not_of - Find the last character in the string that is not in
/// \arg Chars, or npos if not found.
///
/// Note: O(size() + Chars.size())
StringRef::size_type StringRef::find_last_not_of(StringRef Chars,
                                                 size_t From) const {
  std::bitset<1 << CHAR_BIT> CharBits;
  за (char C : Chars)
    CharBits.set((unsigned char)C);

  за (size_type i = std::min(From, size()) - 1, e = -1; i != e; --i)
    ако (!CharBits.test((unsigned char)data()[i]))
      върни i;
  върни npos;
}

void StringRef::split(SmallVectorImpl<StringRef> &A,
                      StringRef Separator, цяло MaxSplit,
                      bool KeepEmpty) const {
  StringRef S = *this;

  // Count down from MaxSplit. When MaxSplit is -1, this will just split
  // "forever". This doesn't support splitting more than 2^31 times
  // intentionally; if we ever want that we can make MaxSplit a 64-bit integer
  // but that seems unlikely to be useful.
  докато (MaxSplit-- != 0) {
    size_t Idx = S.find(Separator);
    ако (Idx == npos)
      break;

    // Push this split.
    ако (KeepEmpty || Idx > 0)
      A.push_back(S.slice(0, Idx));

    // Jump forward.
    S = S.substr(Idx + Separator.size());
  }

  // Push the tail.
  ако (KeepEmpty || !S.empty())
    A.push_back(S);
}

void StringRef::split(SmallVectorImpl<StringRef> &A, char Separator,
                      цяло MaxSplit, bool KeepEmpty) const {
  StringRef S = *this;

  // Count down from MaxSplit. When MaxSplit is -1, this will just split
  // "forever". This doesn't support splitting more than 2^31 times
  // intentionally; if we ever want that we can make MaxSplit a 64-bit integer
  // but that seems unlikely to be useful.
  докато (MaxSplit-- != 0) {
    size_t Idx = S.find(Separator);
    ако (Idx == npos)
      break;

    // Push this split.
    ако (KeepEmpty || Idx > 0)
      A.push_back(S.slice(0, Idx));

    // Jump forward.
    S = S.substr(Idx + 1);
  }

  // Push the tail.
  ако (KeepEmpty || !S.empty())
    A.push_back(S);
}

//===----------------------------------------------------------------------===//
// Helpful Algorithms
//===----------------------------------------------------------------------===//

/// count - Return the number of non-overlapped occurrences of \arg Str in
/// the string.
size_t StringRef::count(StringRef Str) const {
  size_t Count = 0;
  size_t Pos = 0;
  size_t N = Str.size();
  // TODO: For an empty `Str` we return 0 for legacy reasons. Consider changing
  //       this to `Length + 1` which is more in-line with the function
  //       description.
  ако (!N)
    върни 0;
  докато ((Pos = find(Str, Pos)) != npos) {
    ++Count;
    Pos += N;
  }
  върни Count;
}

unsigned llvm::getAutoSenseRadix(StringRef &Str) {
  ако (Str.empty())
    върни 10;

  ако (Str.consume_front_insensitive("0x"))
    върни 16;

  ако (Str.consume_front_insensitive("0b"))
    върни 2;

  ако (Str.consume_front("0o"))
    върни 8;

  ако (Str[0] == '0' && Str.size() > 1 && isDigit(Str[1])) {
    Str = Str.substr(1);
    върни 8;
  }

  върни 10;
}

bool llvm::consumeUnsignedInteger(StringRef &Str, unsigned Radix,
                                  unsigned long long &Result) {
  // Autosense radix if not specified.
  ако (Radix == 0)
    Radix = getAutoSenseRadix(Str);

  // Empty strings (after the radix autosense) are invalid.
  ако (Str.empty()) върни true;

  // Parse all the bytes of the string given this radix.  Watch for overflow.
  StringRef Str2 = Str;
  Result = 0;
  докато (!Str2.empty()) {
    unsigned CharVal;
    ако (Str2[0] >= '0' && Str2[0] <= '9')
      CharVal = Str2[0] - '0';
    иначе ако (Str2[0] >= 'a' && Str2[0] <= 'z')
      CharVal = Str2[0] - 'a' + 10;
    иначе ако (Str2[0] >= 'A' && Str2[0] <= 'Z')
      CharVal = Str2[0] - 'A' + 10;
    иначе
      break;

    // If the parsed value is larger than the integer radix, we cannot
    // consume any more characters.
    ако (CharVal >= Radix)
      break;

    // Add in this character.
    unsigned long long PrevResult = Result;
    Result = Result * Radix + CharVal;

    // Check for overflow by shifting back and seeing if bits were lost.
    ако (Result / Radix < PrevResult)
      върни true;

    Str2 = Str2.substr(1);
  }

  // We consider the operation a failure if no characters were consumed
  // successfully.
  ако (Str.size() == Str2.size())
    върни true;

  Str = Str2;
  върни false;
}

bool llvm::consumeSignedInteger(StringRef &Str, unsigned Radix,
                                long long &Result) {
  unsigned long long ULLVal;

  // Handle positive strings first.
  ако (!Str.starts_with("-")) {
    ако (consumeUnsignedInteger(Str, Radix, ULLVal) ||
        // Check for value so large it overflows a signed value.
        (long long)ULLVal < 0)
      върни true;
    Result = ULLVal;
    върни false;
  }

  // Get the positive part of the value.
  StringRef Str2 = Str.drop_front(1);
  ако (consumeUnsignedInteger(Str2, Radix, ULLVal) ||
      // Reject values so large they'd overflow as negative signed, but allow
      // "-0".  This negates the unsigned so that the negative isn't undefined
      // on signed overflow.
      (long long)-ULLVal > 0)
    върни true;

  Str = Str2;
  Result = -ULLVal;
  върни false;
}

/// GetAsUnsignedInteger - Workhorse method that converts a integer character
/// sequence of radix up to 36 to an unsigned long long value.
bool llvm::getAsUnsignedInteger(StringRef Str, unsigned Radix,
                                unsigned long long &Result) {
  ако (consumeUnsignedInteger(Str, Radix, Result))
    върни true;

  // For getAsUnsignedInteger, we require the whole string to be consumed or
  // else we consider it a failure.
  върни !Str.empty();
}

bool llvm::getAsSignedInteger(StringRef Str, unsigned Radix,
                              long long &Result) {
  ако (consumeSignedInteger(Str, Radix, Result))
    върни true;

  // For getAsSignedInteger, we require the whole string to be consumed or else
  // we consider it a failure.
  върни !Str.empty();
}

bool StringRef::consumeInteger(unsigned Radix, APInt &Result) {
  StringRef Str = *this;

  // Autosense radix if not specified.
  ако (Radix == 0)
    Radix = getAutoSenseRadix(Str);

  assert(Radix > 1 && Radix <= 36);

  // Empty strings (after the radix autosense) are invalid.
  ако (Str.empty()) върни true;

  // Skip leading zeroes.  This can be a significant improvement if
  // it means we don't need > 64 bits.
  Str = Str.ltrim('0');

  // If it was nothing but zeroes....
  ако (Str.empty()) {
    Result = APInt(64, 0);
    *this = Str;
    върни false;
  }

  unsigned Log2Radix = 0;
  докато ((1U << Log2Radix) < Radix) Log2Radix++;
  bool IsPowerOf2Radix = ((1U << Log2Radix) == Radix);

  // Initialize Result to a reasonable starting width (at least 64 bits),
  // but do not shrink it if it was already larger.
  unsigned BitWidth = std::max(64U, Result.getBitWidth());

  ако (Result.getBitWidth() < BitWidth) {
    Result = Result.zext(BitWidth);
  }

  APInt RadixAP, CharAP; // unused unless !IsPowerOf2Radix
  ако (!IsPowerOf2Radix) {
    // These must have the same bit-width as Result.
    RadixAP = APInt(BitWidth, Radix);
    CharAP = APInt(BitWidth, 0);
  }

  // Parse all the bytes of the string given this radix.
  Result = 0;
  докато (!Str.empty()) {
    unsigned CharVal;
    ако (Str[0] >= '0' && Str[0] <= '9')
      CharVal = Str[0]-'0';
    иначе ако (Str[0] >= 'a' && Str[0] <= 'z')
      CharVal = Str[0]-'a'+10;
    иначе ако (Str[0] >= 'A' && Str[0] <= 'Z')
      CharVal = Str[0]-'A'+10;
    иначе
      break;

    // If the parsed value is larger than the integer radix, the string is
    // invalid.
    ако (CharVal >= Radix)
      break;

    // Check if one more digit will overflow, and grow if so.
    ако (Result.getActiveBits() + Log2Radix > Result.getBitWidth()) {
      unsigned NewWidth = Result.getBitWidth() * 2;

      // Guard against overflow of the NewWidth itself
      ако (NewWidth < Result.getBitWidth())
        върни true;

      Result = Result.zext(NewWidth);
      ако (!IsPowerOf2Radix) {
        RadixAP = RadixAP.zext(NewWidth);
        CharAP = CharAP.zext(NewWidth);
      }
    }

    // Add in this character.
    ако (IsPowerOf2Radix) {
      Result <<= Log2Radix;
      Result |= CharVal;
    } иначе {
      Result *= RadixAP;
      CharAP = CharVal;
      Result += CharAP;
    }

    Str = Str.substr(1);
  }

  // We consider the operation a failure if no characters were consumed
  // successfully.
  ако (size() == Str.size())
    върни true;

  *this = Str;
  върни false;
}

bool StringRef::getAsInteger(unsigned Radix, APInt &Result) const {
  StringRef Str = *this;
  ако (Str.consumeInteger(Radix, Result))
    върни true;

  // For getAsInteger, we require the whole string to be consumed or else we
  // consider it a failure.
  върни !Str.empty();
}

bool StringRef::getAsDouble(double &Result, bool AllowInexact) const {
  APFloat F(0.0);
  auto StatusOrErr = F.convertFromString(*this, APFloat::rmNearestTiesToEven);
  ако (errorToBool(StatusOrErr.takeError()))
    върни true;

  APFloat::opStatus Status = *StatusOrErr;
  ако (Status != APFloat::opOK) {
    ако (!AllowInexact || !(Status & APFloat::opInexact))
      върни true;
  }

  Result = F.convertToDouble();
  върни false;
}

// Implementation of StringRef hashing.
hash_code llvm::hash_value(StringRef S) { върни hash_combine_range(S); }

unsigned DenseMapInfo<StringRef, void>::getHashValue(StringRef Val) {
  върни (unsigned)(hash_value(Val));
}
