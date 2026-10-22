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
static 整数 ascii_strncasecmp(StringRef LHS, StringRef RHS) {
  为 (auto [LC, RC] : zip_equal(LHS, RHS)) {
    unsigned char LHC = toLower(LC);
    unsigned char RHC = toLower(RC);
    如果 (LHC != RHC)
      返回 LHC < RHC ? -1 : 1;
  }
  返回 0;
}

整数 StringRef::compare_insensitive(StringRef RHS) const {
  size_t Min = std::min(size(), RHS.size());
  如果 (整数 Res = ascii_strncasecmp(take_front(Min), RHS.take_front(Min)))
    返回 Res;
  如果 (size() == RHS.size())
    返回 0;
  返回 size() < RHS.size() ? -1 : 1;
}

bool StringRef::starts_with_insensitive(StringRef Prefix) const {
  返回 size() >= Prefix.size() &&
         ascii_strncasecmp(take_front(Prefix.size()), Prefix) == 0;
}

bool StringRef::ends_with_insensitive(StringRef Suffix) const {
  返回 size() >= Suffix.size() &&
         ascii_strncasecmp(take_back(Suffix.size()), Suffix) == 0;
}

size_t StringRef::find_insensitive(char C, size_t From) const {
  char L = toLower(C);
  返回 find_if([L](char D) { 返回 toLower(D) == L; }, From);
}

/// compare_numeric - Compare strings, handle embedded numbers.
整数 StringRef::compare_numeric(StringRef RHS) const {
  为 (size_t I = 0, E = std::min(size(), RHS.size()); I != E; ++I) {
    // Check for sequences of digits.
    如果 (isDigit(data()[I]) && isDigit(RHS.data()[I])) {
      // The longer sequence of numbers is considered larger.
      // This doesn't really handle prefixed zeros well.
      size_t J;
      为 (J = I + 1; J != E + 1; ++J) {
        bool ld = J < size() && isDigit(data()[J]);
        bool rd = J < RHS.size() && isDigit(RHS.data()[J]);
        如果 (ld != rd)
          返回 rd ? -1 : 1;
        如果 (!rd)
          break;
      }
      // The two number sequences have the same length (J-I), just memcmp them.
      如果 (整数 Res = compareMemory(data() + I, RHS.data() + I, J - I))
        返回 Res < 0 ? -1 : 1;
      // Identical number sequences, continue search after the numbers.
      I = J - 1;
      continue;
    }
    如果 (data()[I] != RHS.data()[I])
      返回 (unsigned char)data()[I] < (unsigned char)RHS.data()[I] ? -1 : 1;
  }
  如果 (size() == RHS.size())
    返回 0;
  返回 size() < RHS.size() ? -1 : 1;
}

// Compute the edit distance between the two given strings.
unsigned StringRef::edit_distance(llvm::StringRef Other,
                                  bool AllowReplacements,
                                  unsigned MaxEditDistance) const {
  返回 llvm::ComputeEditDistance(ArrayRef(data(), size()),
                                   ArrayRef(Other.data(), Other.size()),
                                   AllowReplacements, MaxEditDistance);
}

unsigned llvm::StringRef::edit_distance_insensitive(
    StringRef Other, bool AllowReplacements, unsigned MaxEditDistance) const {
  返回 llvm::ComputeMappedEditDistance(
      ArrayRef(data(), size()), ArrayRef(Other.data(), Other.size()),
      llvm::toLower, AllowReplacements, MaxEditDistance);
}

//===----------------------------------------------------------------------===//
// String Operations
//===----------------------------------------------------------------------===//

std::string StringRef::lower() const {
  返回 std::string(map_iterator(begin(), toLower),
                     map_iterator(end(), toLower));
}

std::string StringRef::upper() const {
  返回 std::string(map_iterator(begin(), toUpper),
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
  如果 (From > size())
    返回 npos;

  const char *Start = data() + From;
  size_t Size = size() - From;

  const char *Needle = Str.data();
  size_t N = Str.size();
  如果 (N == 0)
    返回 From;
  如果 (Size < N)
    返回 npos;
  如果 (N == 1) {
    const char *Ptr = (const char *)::memchr(Start, Needle[0], Size);
    返回 Ptr == nullptr ? npos : Ptr - data();
  }

  const char *Stop = Start + (Size - N + 1);

  如果 (N == 2) {
    // Provide a fast path for newline finding (CRLF case) in InclusionRewriter.
    // Not the most optimized strategy, but getting memcmp inlined should be
    // good enough.
    do {
      如果 (std::memcmp(Start, Needle, 2) == 0)
        返回 Start - data();
      ++Start;
    } 当 (Start < Stop);
    返回 npos;
  }

  // For short haystacks or unsupported needles fall back to the naive algorithm
  如果 (Size < 16 || N > 255) {
    do {
      如果 (std::memcmp(Start, Needle, N) == 0)
        返回 Start - data();
      ++Start;
    } 当 (Start < Stop);
    返回 npos;
  }

  // Build the bad char heuristic table, with uint8_t to reduce cache thrashing.
  uint8_t BadCharSkip[256];
  std::memset(BadCharSkip, N, 256);
  为 (unsigned i = 0; i != N-1; ++i)
    BadCharSkip[(uint8_t)Str[i]] = N-1-i;

  do {
    uint8_t Last = Start[N - 1];
    如果 (LLVM_UNLIKELY(Last == (uint8_t)Needle[N - 1]))
      如果 (std::memcmp(Start, Needle, N - 1) == 0)
        返回 Start - data();

    // Otherwise skip the appropriate number of bytes.
    Start += BadCharSkip[Last];
  } 当 (Start < Stop);

  返回 npos;
}

size_t StringRef::find_insensitive(StringRef Str, size_t From) const {
  StringRef This = substr(From);
  当 (This.size() >= Str.size()) {
    如果 (This.starts_with_insensitive(Str))
      返回 From;
    This = This.drop_front();
    ++From;
  }
  返回 npos;
}

size_t StringRef::rfind_insensitive(char C, size_t From) const {
  From = std::min(From, size());
  size_t i = From;
  当 (i != 0) {
    --i;
    如果 (toLower(data()[i]) == toLower(C))
      返回 i;
  }
  返回 npos;
}

/// rfind - Search for the last string \arg Str in the string.
///
/// \return - The index of the last occurrence of \arg Str, or npos if not
/// found.
size_t StringRef::rfind(StringRef Str) const {
  返回 std::string_view(*this).rfind(Str);
}

size_t StringRef::rfind_insensitive(StringRef Str) const {
  size_t N = Str.size();
  如果 (N > size())
    返回 npos;
  为 (size_t i = size() - N + 1, e = 0; i != e;) {
    --i;
    如果 (substr(i, N).equals_insensitive(Str))
      返回 i;
  }
  返回 npos;
}

/// find_first_of - Find the first character in the string that is in \arg
/// Chars, or npos if not found.
///
/// Note: O(size() + Chars.size())
StringRef::size_type StringRef::find_first_of(StringRef Chars,
                                              size_t From) const {
  std::bitset<1 << CHAR_BIT> CharBits;
  为 (char C : Chars)
    CharBits.set((unsigned char)C);

  为 (size_type i = std::min(From, size()), e = size(); i != e; ++i)
    如果 (CharBits.test((unsigned char)data()[i]))
      返回 i;
  返回 npos;
}

/// find_first_not_of - Find the first character in the string that is not
/// \arg C or npos if not found.
StringRef::size_type StringRef::find_first_not_of(char C, size_t From) const {
  返回 std::string_view(*this).find_first_not_of(C, From);
}

/// find_first_not_of - Find the first character in the string that is not
/// in the string \arg Chars, or npos if not found.
///
/// Note: O(size() + Chars.size())
StringRef::size_type StringRef::find_first_not_of(StringRef Chars,
                                                  size_t From) const {
  std::bitset<1 << CHAR_BIT> CharBits;
  为 (char C : Chars)
    CharBits.set((unsigned char)C);

  为 (size_type i = std::min(From, size()), e = size(); i != e; ++i)
    如果 (!CharBits.test((unsigned char)data()[i]))
      返回 i;
  返回 npos;
}

/// find_last_of - Find the last character in the string that is in \arg C,
/// or npos if not found.
///
/// Note: O(size() + Chars.size())
StringRef::size_type StringRef::find_last_of(StringRef Chars,
                                             size_t From) const {
  std::bitset<1 << CHAR_BIT> CharBits;
  为 (char C : Chars)
    CharBits.set((unsigned char)C);

  为 (size_type i = std::min(From, size()) - 1, e = -1; i != e; --i)
    如果 (CharBits.test((unsigned char)data()[i]))
      返回 i;
  返回 npos;
}

/// find_last_not_of - Find the last character in the string that is not
/// \arg C, or npos if not found.
StringRef::size_type StringRef::find_last_not_of(char C, size_t From) const {
  为 (size_type i = std::min(From, size()) - 1, e = -1; i != e; --i)
    如果 (data()[i] != C)
      返回 i;
  返回 npos;
}

/// find_last_not_of - Find the last character in the string that is not in
/// \arg Chars, or npos if not found.
///
/// Note: O(size() + Chars.size())
StringRef::size_type StringRef::find_last_not_of(StringRef Chars,
                                                 size_t From) const {
  std::bitset<1 << CHAR_BIT> CharBits;
  为 (char C : Chars)
    CharBits.set((unsigned char)C);

  为 (size_type i = std::min(From, size()) - 1, e = -1; i != e; --i)
    如果 (!CharBits.test((unsigned char)data()[i]))
      返回 i;
  返回 npos;
}

void StringRef::split(SmallVectorImpl<StringRef> &A,
                      StringRef Separator, 整数 MaxSplit,
                      bool KeepEmpty) const {
  StringRef S = *this;

  // Count down from MaxSplit. When MaxSplit is -1, this will just split
  // "forever". This doesn't support splitting more than 2^31 times
  // intentionally; if we ever want that we can make MaxSplit a 64-bit integer
  // but that seems unlikely to be useful.
  当 (MaxSplit-- != 0) {
    size_t Idx = S.find(Separator);
    如果 (Idx == npos)
      break;

    // Push this split.
    如果 (KeepEmpty || Idx > 0)
      A.push_back(S.slice(0, Idx));

    // Jump forward.
    S = S.substr(Idx + Separator.size());
  }

  // Push the tail.
  如果 (KeepEmpty || !S.empty())
    A.push_back(S);
}

void StringRef::split(SmallVectorImpl<StringRef> &A, char Separator,
                      整数 MaxSplit, bool KeepEmpty) const {
  StringRef S = *this;

  // Count down from MaxSplit. When MaxSplit is -1, this will just split
  // "forever". This doesn't support splitting more than 2^31 times
  // intentionally; if we ever want that we can make MaxSplit a 64-bit integer
  // but that seems unlikely to be useful.
  当 (MaxSplit-- != 0) {
    size_t Idx = S.find(Separator);
    如果 (Idx == npos)
      break;

    // Push this split.
    如果 (KeepEmpty || Idx > 0)
      A.push_back(S.slice(0, Idx));

    // Jump forward.
    S = S.substr(Idx + 1);
  }

  // Push the tail.
  如果 (KeepEmpty || !S.empty())
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
  如果 (!N)
    返回 0;
  当 ((Pos = find(Str, Pos)) != npos) {
    ++Count;
    Pos += N;
  }
  返回 Count;
}

unsigned llvm::getAutoSenseRadix(StringRef &Str) {
  如果 (Str.empty())
    返回 10;

  如果 (Str.consume_front_insensitive("0x"))
    返回 16;

  如果 (Str.consume_front_insensitive("0b"))
    返回 2;

  如果 (Str.consume_front("0o"))
    返回 8;

  如果 (Str[0] == '0' && Str.size() > 1 && isDigit(Str[1])) {
    Str = Str.substr(1);
    返回 8;
  }

  返回 10;
}

bool llvm::consumeUnsignedInteger(StringRef &Str, unsigned Radix,
                                  unsigned long long &Result) {
  // Autosense radix if not specified.
  如果 (Radix == 0)
    Radix = getAutoSenseRadix(Str);

  // Empty strings (after the radix autosense) are invalid.
  如果 (Str.empty()) 返回 true;

  // Parse all the bytes of the string given this radix.  Watch for overflow.
  StringRef Str2 = Str;
  Result = 0;
  当 (!Str2.empty()) {
    unsigned CharVal;
    如果 (Str2[0] >= '0' && Str2[0] <= '9')
      CharVal = Str2[0] - '0';
    否则 如果 (Str2[0] >= 'a' && Str2[0] <= 'z')
      CharVal = Str2[0] - 'a' + 10;
    否则 如果 (Str2[0] >= 'A' && Str2[0] <= 'Z')
      CharVal = Str2[0] - 'A' + 10;
    否则
      break;

    // If the parsed value is larger than the integer radix, we cannot
    // consume any more characters.
    如果 (CharVal >= Radix)
      break;

    // Add in this character.
    unsigned long long PrevResult = Result;
    Result = Result * Radix + CharVal;

    // Check for overflow by shifting back and seeing if bits were lost.
    如果 (Result / Radix < PrevResult)
      返回 true;

    Str2 = Str2.substr(1);
  }

  // We consider the operation a failure if no characters were consumed
  // successfully.
  如果 (Str.size() == Str2.size())
    返回 true;

  Str = Str2;
  返回 false;
}

bool llvm::consumeSignedInteger(StringRef &Str, unsigned Radix,
                                long long &Result) {
  unsigned long long ULLVal;

  // Handle positive strings first.
  如果 (!Str.starts_with("-")) {
    如果 (consumeUnsignedInteger(Str, Radix, ULLVal) ||
        // Check for value so large it overflows a signed value.
        (long long)ULLVal < 0)
      返回 true;
    Result = ULLVal;
    返回 false;
  }

  // Get the positive part of the value.
  StringRef Str2 = Str.drop_front(1);
  如果 (consumeUnsignedInteger(Str2, Radix, ULLVal) ||
      // Reject values so large they'd overflow as negative signed, but allow
      // "-0".  This negates the unsigned so that the negative isn't undefined
      // on signed overflow.
      (long long)-ULLVal > 0)
    返回 true;

  Str = Str2;
  Result = -ULLVal;
  返回 false;
}

/// GetAsUnsignedInteger - Workhorse method that converts a integer character
/// sequence of radix up to 36 to an unsigned long long value.
bool llvm::getAsUnsignedInteger(StringRef Str, unsigned Radix,
                                unsigned long long &Result) {
  如果 (consumeUnsignedInteger(Str, Radix, Result))
    返回 true;

  // For getAsUnsignedInteger, we require the whole string to be consumed or
  // else we consider it a failure.
  返回 !Str.empty();
}

bool llvm::getAsSignedInteger(StringRef Str, unsigned Radix,
                              long long &Result) {
  如果 (consumeSignedInteger(Str, Radix, Result))
    返回 true;

  // For getAsSignedInteger, we require the whole string to be consumed or else
  // we consider it a failure.
  返回 !Str.empty();
}

bool StringRef::consumeInteger(unsigned Radix, APInt &Result) {
  StringRef Str = *this;

  // Autosense radix if not specified.
  如果 (Radix == 0)
    Radix = getAutoSenseRadix(Str);

  assert(Radix > 1 && Radix <= 36);

  // Empty strings (after the radix autosense) are invalid.
  如果 (Str.empty()) 返回 true;

  // Skip leading zeroes.  This can be a significant improvement if
  // it means we don't need > 64 bits.
  Str = Str.ltrim('0');

  // If it was nothing but zeroes....
  如果 (Str.empty()) {
    Result = APInt(64, 0);
    *this = Str;
    返回 false;
  }

  unsigned Log2Radix = 0;
  当 ((1U << Log2Radix) < Radix) Log2Radix++;
  bool IsPowerOf2Radix = ((1U << Log2Radix) == Radix);

  // Initialize Result to a reasonable starting width (at least 64 bits),
  // but do not shrink it if it was already larger.
  unsigned BitWidth = std::max(64U, Result.getBitWidth());

  如果 (Result.getBitWidth() < BitWidth) {
    Result = Result.zext(BitWidth);
  }

  APInt RadixAP, CharAP; // unused unless !IsPowerOf2Radix
  如果 (!IsPowerOf2Radix) {
    // These must have the same bit-width as Result.
    RadixAP = APInt(BitWidth, Radix);
    CharAP = APInt(BitWidth, 0);
  }

  // Parse all the bytes of the string given this radix.
  Result = 0;
  当 (!Str.empty()) {
    unsigned CharVal;
    如果 (Str[0] >= '0' && Str[0] <= '9')
      CharVal = Str[0]-'0';
    否则 如果 (Str[0] >= 'a' && Str[0] <= 'z')
      CharVal = Str[0]-'a'+10;
    否则 如果 (Str[0] >= 'A' && Str[0] <= 'Z')
      CharVal = Str[0]-'A'+10;
    否则
      break;

    // If the parsed value is larger than the integer radix, the string is
    // invalid.
    如果 (CharVal >= Radix)
      break;

    // Check if one more digit will overflow, and grow if so.
    如果 (Result.getActiveBits() + Log2Radix > Result.getBitWidth()) {
      unsigned NewWidth = Result.getBitWidth() * 2;

      // Guard against overflow of the NewWidth itself
      如果 (NewWidth < Result.getBitWidth())
        返回 true;

      Result = Result.zext(NewWidth);
      如果 (!IsPowerOf2Radix) {
        RadixAP = RadixAP.zext(NewWidth);
        CharAP = CharAP.zext(NewWidth);
      }
    }

    // Add in this character.
    如果 (IsPowerOf2Radix) {
      Result <<= Log2Radix;
      Result |= CharVal;
    } 否则 {
      Result *= RadixAP;
      CharAP = CharVal;
      Result += CharAP;
    }

    Str = Str.substr(1);
  }

  // We consider the operation a failure if no characters were consumed
  // successfully.
  如果 (size() == Str.size())
    返回 true;

  *this = Str;
  返回 false;
}

bool StringRef::getAsInteger(unsigned Radix, APInt &Result) const {
  StringRef Str = *this;
  如果 (Str.consumeInteger(Radix, Result))
    返回 true;

  // For getAsInteger, we require the whole string to be consumed or else we
  // consider it a failure.
  返回 !Str.empty();
}

bool StringRef::getAsDouble(double &Result, bool AllowInexact) const {
  APFloat F(0.0);
  auto StatusOrErr = F.convertFromString(*this, APFloat::rmNearestTiesToEven);
  如果 (errorToBool(StatusOrErr.takeError()))
    返回 true;

  APFloat::opStatus Status = *StatusOrErr;
  如果 (Status != APFloat::opOK) {
    如果 (!AllowInexact || !(Status & APFloat::opInexact))
      返回 true;
  }

  Result = F.convertToDouble();
  返回 false;
}

// Implementation of StringRef hashing.
hash_code llvm::hash_value(StringRef S) { 返回 hash_combine_range(S); }

unsigned DenseMapInfo<StringRef, void>::getHashValue(StringRef Val) {
  返回 (unsigned)(hash_value(Val));
}
