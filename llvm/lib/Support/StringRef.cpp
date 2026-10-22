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
static ціле ascii_strncasecmp(StringRef LHS, StringRef RHS) {
  для (auto [LC, RC] : zip_equal(LHS, RHS)) {
    unsigned char LHC = toLower(LC);
    unsigned char RHC = toLower(RC);
    якщо (LHC != RHC)
      повернути LHC < RHC ? -1 : 1;
  }
  повернути 0;
}

ціле StringRef::compare_insensitive(StringRef RHS) const {
  size_t Min = std::min(size(), RHS.size());
  якщо (ціле Res = ascii_strncasecmp(take_front(Min), RHS.take_front(Min)))
    повернути Res;
  якщо (size() == RHS.size())
    повернути 0;
  повернути size() < RHS.size() ? -1 : 1;
}

bool StringRef::starts_with_insensitive(StringRef Prefix) const {
  повернути size() >= Prefix.size() &&
         ascii_strncasecmp(take_front(Prefix.size()), Prefix) == 0;
}

bool StringRef::ends_with_insensitive(StringRef Suffix) const {
  повернути size() >= Suffix.size() &&
         ascii_strncasecmp(take_back(Suffix.size()), Suffix) == 0;
}

size_t StringRef::find_insensitive(char C, size_t From) const {
  char L = toLower(C);
  повернути find_if([L](char D) { повернути toLower(D) == L; }, From);
}

/// compare_numeric - Compare strings, handle embedded numbers.
ціле StringRef::compare_numeric(StringRef RHS) const {
  для (size_t I = 0, E = std::min(size(), RHS.size()); I != E; ++I) {
    // Check for sequences of digits.
    якщо (isDigit(data()[I]) && isDigit(RHS.data()[I])) {
      // The longer sequence of numbers is considered larger.
      // This doesn't really handle prefixed zeros well.
      size_t J;
      для (J = I + 1; J != E + 1; ++J) {
        bool ld = J < size() && isDigit(data()[J]);
        bool rd = J < RHS.size() && isDigit(RHS.data()[J]);
        якщо (ld != rd)
          повернути rd ? -1 : 1;
        якщо (!rd)
          break;
      }
      // The two number sequences have the same length (J-I), just memcmp them.
      якщо (ціле Res = compareMemory(data() + I, RHS.data() + I, J - I))
        повернути Res < 0 ? -1 : 1;
      // Identical number sequences, continue search after the numbers.
      I = J - 1;
      continue;
    }
    якщо (data()[I] != RHS.data()[I])
      повернути (unsigned char)data()[I] < (unsigned char)RHS.data()[I] ? -1 : 1;
  }
  якщо (size() == RHS.size())
    повернути 0;
  повернути size() < RHS.size() ? -1 : 1;
}

// Compute the edit distance between the two given strings.
unsigned StringRef::edit_distance(llvm::StringRef Other,
                                  bool AllowReplacements,
                                  unsigned MaxEditDistance) const {
  повернути llvm::ComputeEditDistance(ArrayRef(data(), size()),
                                   ArrayRef(Other.data(), Other.size()),
                                   AllowReplacements, MaxEditDistance);
}

unsigned llvm::StringRef::edit_distance_insensitive(
    StringRef Other, bool AllowReplacements, unsigned MaxEditDistance) const {
  повернути llvm::ComputeMappedEditDistance(
      ArrayRef(data(), size()), ArrayRef(Other.data(), Other.size()),
      llvm::toLower, AllowReplacements, MaxEditDistance);
}

//===----------------------------------------------------------------------===//
// String Operations
//===----------------------------------------------------------------------===//

std::string StringRef::lower() const {
  повернути std::string(map_iterator(begin(), toLower),
                     map_iterator(end(), toLower));
}

std::string StringRef::upper() const {
  повернути std::string(map_iterator(begin(), toUpper),
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
  якщо (From > size())
    повернути npos;

  const char *Start = data() + From;
  size_t Size = size() - From;

  const char *Needle = Str.data();
  size_t N = Str.size();
  якщо (N == 0)
    повернути From;
  якщо (Size < N)
    повернути npos;
  якщо (N == 1) {
    const char *Ptr = (const char *)::memchr(Start, Needle[0], Size);
    повернути Ptr == nullptr ? npos : Ptr - data();
  }

  const char *Stop = Start + (Size - N + 1);

  якщо (N == 2) {
    // Provide a fast path for newline finding (CRLF case) in InclusionRewriter.
    // Not the most optimized strategy, but getting memcmp inlined should be
    // good enough.
    do {
      якщо (std::memcmp(Start, Needle, 2) == 0)
        повернути Start - data();
      ++Start;
    } поки (Start < Stop);
    повернути npos;
  }

  // For short haystacks or unsupported needles fall back to the naive algorithm
  якщо (Size < 16 || N > 255) {
    do {
      якщо (std::memcmp(Start, Needle, N) == 0)
        повернути Start - data();
      ++Start;
    } поки (Start < Stop);
    повернути npos;
  }

  // Build the bad char heuristic table, with uint8_t to reduce cache thrashing.
  uint8_t BadCharSkip[256];
  std::memset(BadCharSkip, N, 256);
  для (unsigned i = 0; i != N-1; ++i)
    BadCharSkip[(uint8_t)Str[i]] = N-1-i;

  do {
    uint8_t Last = Start[N - 1];
    якщо (LLVM_UNLIKELY(Last == (uint8_t)Needle[N - 1]))
      якщо (std::memcmp(Start, Needle, N - 1) == 0)
        повернути Start - data();

    // Otherwise skip the appropriate number of bytes.
    Start += BadCharSkip[Last];
  } поки (Start < Stop);

  повернути npos;
}

size_t StringRef::find_insensitive(StringRef Str, size_t From) const {
  StringRef This = substr(From);
  поки (This.size() >= Str.size()) {
    якщо (This.starts_with_insensitive(Str))
      повернути From;
    This = This.drop_front();
    ++From;
  }
  повернути npos;
}

size_t StringRef::rfind_insensitive(char C, size_t From) const {
  From = std::min(From, size());
  size_t i = From;
  поки (i != 0) {
    --i;
    якщо (toLower(data()[i]) == toLower(C))
      повернути i;
  }
  повернути npos;
}

/// rfind - Search for the last string \arg Str in the string.
///
/// \return - The index of the last occurrence of \arg Str, or npos if not
/// found.
size_t StringRef::rfind(StringRef Str) const {
  повернути std::string_view(*this).rfind(Str);
}

size_t StringRef::rfind_insensitive(StringRef Str) const {
  size_t N = Str.size();
  якщо (N > size())
    повернути npos;
  для (size_t i = size() - N + 1, e = 0; i != e;) {
    --i;
    якщо (substr(i, N).equals_insensitive(Str))
      повернути i;
  }
  повернути npos;
}

/// find_first_of - Find the first character in the string that is in \arg
/// Chars, or npos if not found.
///
/// Note: O(size() + Chars.size())
StringRef::size_type StringRef::find_first_of(StringRef Chars,
                                              size_t From) const {
  std::bitset<1 << CHAR_BIT> CharBits;
  для (char C : Chars)
    CharBits.set((unsigned char)C);

  для (size_type i = std::min(From, size()), e = size(); i != e; ++i)
    якщо (CharBits.test((unsigned char)data()[i]))
      повернути i;
  повернути npos;
}

/// find_first_not_of - Find the first character in the string that is not
/// \arg C or npos if not found.
StringRef::size_type StringRef::find_first_not_of(char C, size_t From) const {
  повернути std::string_view(*this).find_first_not_of(C, From);
}

/// find_first_not_of - Find the first character in the string that is not
/// in the string \arg Chars, or npos if not found.
///
/// Note: O(size() + Chars.size())
StringRef::size_type StringRef::find_first_not_of(StringRef Chars,
                                                  size_t From) const {
  std::bitset<1 << CHAR_BIT> CharBits;
  для (char C : Chars)
    CharBits.set((unsigned char)C);

  для (size_type i = std::min(From, size()), e = size(); i != e; ++i)
    якщо (!CharBits.test((unsigned char)data()[i]))
      повернути i;
  повернути npos;
}

/// find_last_of - Find the last character in the string that is in \arg C,
/// or npos if not found.
///
/// Note: O(size() + Chars.size())
StringRef::size_type StringRef::find_last_of(StringRef Chars,
                                             size_t From) const {
  std::bitset<1 << CHAR_BIT> CharBits;
  для (char C : Chars)
    CharBits.set((unsigned char)C);

  для (size_type i = std::min(From, size()) - 1, e = -1; i != e; --i)
    якщо (CharBits.test((unsigned char)data()[i]))
      повернути i;
  повернути npos;
}

/// find_last_not_of - Find the last character in the string that is not
/// \arg C, or npos if not found.
StringRef::size_type StringRef::find_last_not_of(char C, size_t From) const {
  для (size_type i = std::min(From, size()) - 1, e = -1; i != e; --i)
    якщо (data()[i] != C)
      повернути i;
  повернути npos;
}

/// find_last_not_of - Find the last character in the string that is not in
/// \arg Chars, or npos if not found.
///
/// Note: O(size() + Chars.size())
StringRef::size_type StringRef::find_last_not_of(StringRef Chars,
                                                 size_t From) const {
  std::bitset<1 << CHAR_BIT> CharBits;
  для (char C : Chars)
    CharBits.set((unsigned char)C);

  для (size_type i = std::min(From, size()) - 1, e = -1; i != e; --i)
    якщо (!CharBits.test((unsigned char)data()[i]))
      повернути i;
  повернути npos;
}

void StringRef::split(SmallVectorImpl<StringRef> &A,
                      StringRef Separator, ціле MaxSplit,
                      bool KeepEmpty) const {
  StringRef S = *this;

  // Count down from MaxSplit. When MaxSplit is -1, this will just split
  // "forever". This doesn't support splitting more than 2^31 times
  // intentionally; if we ever want that we can make MaxSplit a 64-bit integer
  // but that seems unlikely to be useful.
  поки (MaxSplit-- != 0) {
    size_t Idx = S.find(Separator);
    якщо (Idx == npos)
      break;

    // Push this split.
    якщо (KeepEmpty || Idx > 0)
      A.push_back(S.slice(0, Idx));

    // Jump forward.
    S = S.substr(Idx + Separator.size());
  }

  // Push the tail.
  якщо (KeepEmpty || !S.empty())
    A.push_back(S);
}

void StringRef::split(SmallVectorImpl<StringRef> &A, char Separator,
                      ціле MaxSplit, bool KeepEmpty) const {
  StringRef S = *this;

  // Count down from MaxSplit. When MaxSplit is -1, this will just split
  // "forever". This doesn't support splitting more than 2^31 times
  // intentionally; if we ever want that we can make MaxSplit a 64-bit integer
  // but that seems unlikely to be useful.
  поки (MaxSplit-- != 0) {
    size_t Idx = S.find(Separator);
    якщо (Idx == npos)
      break;

    // Push this split.
    якщо (KeepEmpty || Idx > 0)
      A.push_back(S.slice(0, Idx));

    // Jump forward.
    S = S.substr(Idx + 1);
  }

  // Push the tail.
  якщо (KeepEmpty || !S.empty())
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
  якщо (!N)
    повернути 0;
  поки ((Pos = find(Str, Pos)) != npos) {
    ++Count;
    Pos += N;
  }
  повернути Count;
}

unsigned llvm::getAutoSenseRadix(StringRef &Str) {
  якщо (Str.empty())
    повернути 10;

  якщо (Str.consume_front_insensitive("0x"))
    повернути 16;

  якщо (Str.consume_front_insensitive("0b"))
    повернути 2;

  якщо (Str.consume_front("0o"))
    повернути 8;

  якщо (Str[0] == '0' && Str.size() > 1 && isDigit(Str[1])) {
    Str = Str.substr(1);
    повернути 8;
  }

  повернути 10;
}

bool llvm::consumeUnsignedInteger(StringRef &Str, unsigned Radix,
                                  unsigned long long &Result) {
  // Autosense radix if not specified.
  якщо (Radix == 0)
    Radix = getAutoSenseRadix(Str);

  // Empty strings (after the radix autosense) are invalid.
  якщо (Str.empty()) повернути true;

  // Parse all the bytes of the string given this radix.  Watch for overflow.
  StringRef Str2 = Str;
  Result = 0;
  поки (!Str2.empty()) {
    unsigned CharVal;
    якщо (Str2[0] >= '0' && Str2[0] <= '9')
      CharVal = Str2[0] - '0';
    інакше якщо (Str2[0] >= 'a' && Str2[0] <= 'z')
      CharVal = Str2[0] - 'a' + 10;
    інакше якщо (Str2[0] >= 'A' && Str2[0] <= 'Z')
      CharVal = Str2[0] - 'A' + 10;
    інакше
      break;

    // If the parsed value is larger than the integer radix, we cannot
    // consume any more characters.
    якщо (CharVal >= Radix)
      break;

    // Add in this character.
    unsigned long long PrevResult = Result;
    Result = Result * Radix + CharVal;

    // Check for overflow by shifting back and seeing if bits were lost.
    якщо (Result / Radix < PrevResult)
      повернути true;

    Str2 = Str2.substr(1);
  }

  // We consider the operation a failure if no characters were consumed
  // successfully.
  якщо (Str.size() == Str2.size())
    повернути true;

  Str = Str2;
  повернути false;
}

bool llvm::consumeSignedInteger(StringRef &Str, unsigned Radix,
                                long long &Result) {
  unsigned long long ULLVal;

  // Handle positive strings first.
  якщо (!Str.starts_with("-")) {
    якщо (consumeUnsignedInteger(Str, Radix, ULLVal) ||
        // Check for value so large it overflows a signed value.
        (long long)ULLVal < 0)
      повернути true;
    Result = ULLVal;
    повернути false;
  }

  // Get the positive part of the value.
  StringRef Str2 = Str.drop_front(1);
  якщо (consumeUnsignedInteger(Str2, Radix, ULLVal) ||
      // Reject values so large they'd overflow as negative signed, but allow
      // "-0".  This negates the unsigned so that the negative isn't undefined
      // on signed overflow.
      (long long)-ULLVal > 0)
    повернути true;

  Str = Str2;
  Result = -ULLVal;
  повернути false;
}

/// GetAsUnsignedInteger - Workhorse method that converts a integer character
/// sequence of radix up to 36 to an unsigned long long value.
bool llvm::getAsUnsignedInteger(StringRef Str, unsigned Radix,
                                unsigned long long &Result) {
  якщо (consumeUnsignedInteger(Str, Radix, Result))
    повернути true;

  // For getAsUnsignedInteger, we require the whole string to be consumed or
  // else we consider it a failure.
  повернути !Str.empty();
}

bool llvm::getAsSignedInteger(StringRef Str, unsigned Radix,
                              long long &Result) {
  якщо (consumeSignedInteger(Str, Radix, Result))
    повернути true;

  // For getAsSignedInteger, we require the whole string to be consumed or else
  // we consider it a failure.
  повернути !Str.empty();
}

bool StringRef::consumeInteger(unsigned Radix, APInt &Result) {
  StringRef Str = *this;

  // Autosense radix if not specified.
  якщо (Radix == 0)
    Radix = getAutoSenseRadix(Str);

  assert(Radix > 1 && Radix <= 36);

  // Empty strings (after the radix autosense) are invalid.
  якщо (Str.empty()) повернути true;

  // Skip leading zeroes.  This can be a significant improvement if
  // it means we don't need > 64 bits.
  Str = Str.ltrim('0');

  // If it was nothing but zeroes....
  якщо (Str.empty()) {
    Result = APInt(64, 0);
    *this = Str;
    повернути false;
  }

  unsigned Log2Radix = 0;
  поки ((1U << Log2Radix) < Radix) Log2Radix++;
  bool IsPowerOf2Radix = ((1U << Log2Radix) == Radix);

  // Initialize Result to a reasonable starting width (at least 64 bits),
  // but do not shrink it if it was already larger.
  unsigned BitWidth = std::max(64U, Result.getBitWidth());

  якщо (Result.getBitWidth() < BitWidth) {
    Result = Result.zext(BitWidth);
  }

  APInt RadixAP, CharAP; // unused unless !IsPowerOf2Radix
  якщо (!IsPowerOf2Radix) {
    // These must have the same bit-width as Result.
    RadixAP = APInt(BitWidth, Radix);
    CharAP = APInt(BitWidth, 0);
  }

  // Parse all the bytes of the string given this radix.
  Result = 0;
  поки (!Str.empty()) {
    unsigned CharVal;
    якщо (Str[0] >= '0' && Str[0] <= '9')
      CharVal = Str[0]-'0';
    інакше якщо (Str[0] >= 'a' && Str[0] <= 'z')
      CharVal = Str[0]-'a'+10;
    інакше якщо (Str[0] >= 'A' && Str[0] <= 'Z')
      CharVal = Str[0]-'A'+10;
    інакше
      break;

    // If the parsed value is larger than the integer radix, the string is
    // invalid.
    якщо (CharVal >= Radix)
      break;

    // Check if one more digit will overflow, and grow if so.
    якщо (Result.getActiveBits() + Log2Radix > Result.getBitWidth()) {
      unsigned NewWidth = Result.getBitWidth() * 2;

      // Guard against overflow of the NewWidth itself
      якщо (NewWidth < Result.getBitWidth())
        повернути true;

      Result = Result.zext(NewWidth);
      якщо (!IsPowerOf2Radix) {
        RadixAP = RadixAP.zext(NewWidth);
        CharAP = CharAP.zext(NewWidth);
      }
    }

    // Add in this character.
    якщо (IsPowerOf2Radix) {
      Result <<= Log2Radix;
      Result |= CharVal;
    } інакше {
      Result *= RadixAP;
      CharAP = CharVal;
      Result += CharAP;
    }

    Str = Str.substr(1);
  }

  // We consider the operation a failure if no characters were consumed
  // successfully.
  якщо (size() == Str.size())
    повернути true;

  *this = Str;
  повернути false;
}

bool StringRef::getAsInteger(unsigned Radix, APInt &Result) const {
  StringRef Str = *this;
  якщо (Str.consumeInteger(Radix, Result))
    повернути true;

  // For getAsInteger, we require the whole string to be consumed or else we
  // consider it a failure.
  повернути !Str.empty();
}

bool StringRef::getAsDouble(double &Result, bool AllowInexact) const {
  APFloat F(0.0);
  auto StatusOrErr = F.convertFromString(*this, APFloat::rmNearestTiesToEven);
  якщо (errorToBool(StatusOrErr.takeError()))
    повернути true;

  APFloat::opStatus Status = *StatusOrErr;
  якщо (Status != APFloat::opOK) {
    якщо (!AllowInexact || !(Status & APFloat::opInexact))
      повернути true;
  }

  Result = F.convertToDouble();
  повернути false;
}

// Implementation of StringRef hashing.
hash_code llvm::hash_value(StringRef S) { повернути hash_combine_range(S); }

unsigned DenseMapInfo<StringRef, void>::getHashValue(StringRef Val) {
  повернути (unsigned)(hash_value(Val));
}
