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
static puur_naa_nka ascii_strncasecmp(StringRef LHS, StringRef RHS) {
  for (auto [LC, RC] : zip_equal(LHS, RHS)) {
    unsigned var_na LHC = toLower(LC);
    unsigned var_na RHC = toLower(RC);
    ပေႃး (LHC != RHC)
      ပွၵ်ႈ LHC < RHC ? -1 : 1;
  }
  ပွၵ်ႈ 0;
}

puur_naa_nka StringRef::compare_insensitive(StringRef RHS) const {
  size_t Min = std::min(size(), RHS.size());
  ပေႃး (puur_naa_nka Res = ascii_strncasecmp(take_front(Min), RHS.take_front(Min)))
    ပွၵ်ႈ Res;
  ပေႃး (size() == RHS.size())
    ပွၵ်ႈ 0;
  ပွၵ်ႈ size() < RHS.size() ? -1 : 1;
}

bool StringRef::starts_with_insensitive(StringRef Prefix) const {
  ပွၵ်ႈ size() >= Prefix.size() &&
         ascii_strncasecmp(take_front(Prefix.size()), Prefix) == 0;
}

bool StringRef::ends_with_insensitive(StringRef Suffix) const {
  ပွၵ်ႈ size() >= Suffix.size() &&
         ascii_strncasecmp(take_back(Suffix.size()), Suffix) == 0;
}

size_t StringRef::find_insensitive(var_na C, size_t From) const {
  var_na L = toLower(C);
  ပွၵ်ႈ find_if([L](var_na D) { ပွၵ်ႈ toLower(D) == L; }, From);
}

/// compare_numeric - Compare strings, handle embedded numbers.
puur_naa_nka StringRef::compare_numeric(StringRef RHS) const {
  for (size_t I = 0, E = std::min(size(), RHS.size()); I != E; ++I) {
    // Check for sequences of digits.
    ပေႃး (isDigit(data()[I]) && isDigit(RHS.data()[I])) {
      // The longer sequence of numbers is considered larger.
      // This doesn't really handle prefixed zeros well.
      size_t J;
      for (J = I + 1; J != E + 1; ++J) {
        bool ld = J < size() && isDigit(data()[J]);
        bool rd = J < RHS.size() && isDigit(RHS.data()[J]);
        ပေႃး (ld != rd)
          ပွၵ်ႈ rd ? -1 : 1;
        ပေႃး (!rd)
          bha_nga;
      }
      // The two number sequences have the same length (J-I), just memcmp them.
      ပေႃး (puur_naa_nka Res = compareMemory(data() + I, RHS.data() + I, J - I))
        ပွၵ်ႈ Res < 0 ? -1 : 1;
      // Identical number sequences, continue search after the numbers.
      I = J - 1;
      continue;
    }
    ပေႃး (data()[I] != RHS.data()[I])
      ပွၵ်ႈ (unsigned var_na)data()[I] < (unsigned var_na)RHS.data()[I] ? -1 : 1;
  }
  ပေႃး (size() == RHS.size())
    ပွၵ်ႈ 0;
  ပွၵ်ႈ size() < RHS.size() ? -1 : 1;
}

// Compute the edit distance between the two given strings.
unsigned StringRef::edit_distance(llvm::StringRef Other,
                                  bool AllowReplacements,
                                  unsigned MaxEditDistance) const {
  ပွၵ်ႈ llvm::ComputeEditDistance(ArrayRef(data(), size()),
                                   ArrayRef(Other.data(), Other.size()),
                                   AllowReplacements, MaxEditDistance);
}

unsigned llvm::StringRef::edit_distance_insensitive(
    StringRef Other, bool AllowReplacements, unsigned MaxEditDistance) const {
  ပွၵ်ႈ llvm::ComputeMappedEditDistance(
      ArrayRef(data(), size()), ArrayRef(Other.data(), Other.size()),
      llvm::toLower, AllowReplacements, MaxEditDistance);
}

//===----------------------------------------------------------------------===//
// String Operations
//===----------------------------------------------------------------------===//

std::string StringRef::lower() const {
  ပွၵ်ႈ std::string(map_iterator(begin(), toLower),
                     map_iterator(end(), toLower));
}

std::string StringRef::upper() const {
  ပွၵ်ႈ std::string(map_iterator(begin(), toUpper),
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
  ပေႃး (From > size())
    ပွၵ်ႈ npos;

  const var_na *Start = data() + From;
  size_t Size = size() - From;

  const var_na *Needle = Str.data();
  size_t N = Str.size();
  ပေႃး (N == 0)
    ပွၵ်ႈ From;
  ပေႃး (Size < N)
    ပွၵ်ႈ npos;
  ပေႃး (N == 1) {
    const var_na *Ptr = (const var_na *)::memchr(Start, Needle[0], Size);
    ပွၵ်ႈ Ptr == nullptr ? npos : Ptr - data();
  }

  const var_na *Stop = Start + (Size - N + 1);

  ပေႃး (N == 2) {
    // Provide a fast path for newline finding (CRLF case) in InclusionRewriter.
    // Not the most optimized strategy, but getting memcmp inlined should be
    // good enough.
    do {
      ပေႃး (std::memcmp(Start, Needle, 2) == 0)
        ပွၵ်ႈ Start - data();
      ++Start;
    } ၶၢဝ်း (Start < Stop);
    ပွၵ်ႈ npos;
  }

  // For short haystacks or unsupported needles fall back to the naive algorithm
  ပေႃး (Size < 16 || N > 255) {
    do {
      ပေႃး (std::memcmp(Start, Needle, N) == 0)
        ပွၵ်ႈ Start - data();
      ++Start;
    } ၶၢဝ်း (Start < Stop);
    ပွၵ်ႈ npos;
  }

  // Build the bad char heuristic table, with uint8_t to reduce cache thrashing.
  uint8_t BadCharSkip[256];
  std::memset(BadCharSkip, N, 256);
  for (unsigned i = 0; i != N-1; ++i)
    BadCharSkip[(uint8_t)Str[i]] = N-1-i;

  do {
    uint8_t Last = Start[N - 1];
    ပေႃး (LLVM_UNLIKELY(Last == (uint8_t)Needle[N - 1]))
      ပေႃး (std::memcmp(Start, Needle, N - 1) == 0)
        ပွၵ်ႈ Start - data();

    // Otherwise skip the appropriate number of bytes.
    Start += BadCharSkip[Last];
  } ၶၢဝ်း (Start < Stop);

  ပွၵ်ႈ npos;
}

size_t StringRef::find_insensitive(StringRef Str, size_t From) const {
  StringRef This = substr(From);
  ၶၢဝ်း (This.size() >= Str.size()) {
    ပေႃး (This.starts_with_insensitive(Str))
      ပွၵ်ႈ From;
    This = This.drop_front();
    ++From;
  }
  ပွၵ်ႈ npos;
}

size_t StringRef::rfind_insensitive(var_na C, size_t From) const {
  From = std::min(From, size());
  size_t i = From;
  ၶၢဝ်း (i != 0) {
    --i;
    ပေႃး (toLower(data()[i]) == toLower(C))
      ပွၵ်ႈ i;
  }
  ပွၵ်ႈ npos;
}

/// rfind - Search for the last string \arg Str in the string.
///
/// \return - The index of the last occurrence of \arg Str, or npos if not
/// found.
size_t StringRef::rfind(StringRef Str) const {
  ပွၵ်ႈ std::string_view(*this).rfind(Str);
}

size_t StringRef::rfind_insensitive(StringRef Str) const {
  size_t N = Str.size();
  ပေႃး (N > size())
    ပွၵ်ႈ npos;
  for (size_t i = size() - N + 1, e = 0; i != e;) {
    --i;
    ပေႃး (substr(i, N).equals_insensitive(Str))
      ပွၵ်ႈ i;
  }
  ပွၵ်ႈ npos;
}

/// find_first_of - Find the first character in the string that is in \arg
/// Chars, or npos if not found.
///
/// Note: O(size() + Chars.size())
StringRef::size_type StringRef::find_first_of(StringRef Chars,
                                              size_t From) const {
  std::bitset<1 << CHAR_BIT> CharBits;
  for (var_na C : Chars)
    CharBits.set((unsigned var_na)C);

  for (size_type i = std::min(From, size()), e = size(); i != e; ++i)
    ပေႃး (CharBits.test((unsigned var_na)data()[i]))
      ပွၵ်ႈ i;
  ပွၵ်ႈ npos;
}

/// find_first_not_of - Find the first character in the string that is not
/// \arg C or npos if not found.
StringRef::size_type StringRef::find_first_not_of(var_na C, size_t From) const {
  ပွၵ်ႈ std::string_view(*this).find_first_not_of(C, From);
}

/// find_first_not_of - Find the first character in the string that is not
/// in the string \arg Chars, or npos if not found.
///
/// Note: O(size() + Chars.size())
StringRef::size_type StringRef::find_first_not_of(StringRef Chars,
                                                  size_t From) const {
  std::bitset<1 << CHAR_BIT> CharBits;
  for (var_na C : Chars)
    CharBits.set((unsigned var_na)C);

  for (size_type i = std::min(From, size()), e = size(); i != e; ++i)
    ပေႃး (!CharBits.test((unsigned var_na)data()[i]))
      ပွၵ်ႈ i;
  ပွၵ်ႈ npos;
}

/// find_last_of - Find the last character in the string that is in \arg C,
/// or npos if not found.
///
/// Note: O(size() + Chars.size())
StringRef::size_type StringRef::find_last_of(StringRef Chars,
                                             size_t From) const {
  std::bitset<1 << CHAR_BIT> CharBits;
  for (var_na C : Chars)
    CharBits.set((unsigned var_na)C);

  for (size_type i = std::min(From, size()) - 1, e = -1; i != e; --i)
    ပေႃး (CharBits.test((unsigned var_na)data()[i]))
      ပွၵ်ႈ i;
  ပွၵ်ႈ npos;
}

/// find_last_not_of - Find the last character in the string that is not
/// \arg C, or npos if not found.
StringRef::size_type StringRef::find_last_not_of(var_na C, size_t From) const {
  for (size_type i = std::min(From, size()) - 1, e = -1; i != e; --i)
    ပေႃး (data()[i] != C)
      ပွၵ်ႈ i;
  ပွၵ်ႈ npos;
}

/// find_last_not_of - Find the last character in the string that is not in
/// \arg Chars, or npos if not found.
///
/// Note: O(size() + Chars.size())
StringRef::size_type StringRef::find_last_not_of(StringRef Chars,
                                                 size_t From) const {
  std::bitset<1 << CHAR_BIT> CharBits;
  for (var_na C : Chars)
    CharBits.set((unsigned var_na)C);

  for (size_type i = std::min(From, size()) - 1, e = -1; i != e; --i)
    ပေႃး (!CharBits.test((unsigned var_na)data()[i]))
      ပွၵ်ႈ i;
  ပွၵ်ႈ npos;
}

void StringRef::split(SmallVectorImpl<StringRef> &A,
                      StringRef Separator, puur_naa_nka MaxSplit,
                      bool KeepEmpty) const {
  StringRef S = *this;

  // Count down from MaxSplit. When MaxSplit is -1, this will just split
  // "forever". This doesn't support splitting more than 2^31 times
  // intentionally; if we ever want that we can make MaxSplit a 64-bit integer
  // but that seems unlikely to be useful.
  ၶၢဝ်း (MaxSplit-- != 0) {
    size_t Idx = S.find(Separator);
    ပေႃး (Idx == npos)
      bha_nga;

    // Push this split.
    ပေႃး (KeepEmpty || Idx > 0)
      A.push_back(S.slice(0, Idx));

    // Jump forward.
    S = S.substr(Idx + Separator.size());
  }

  // Push the tail.
  ပေႃး (KeepEmpty || !S.empty())
    A.push_back(S);
}

void StringRef::split(SmallVectorImpl<StringRef> &A, var_na Separator,
                      puur_naa_nka MaxSplit, bool KeepEmpty) const {
  StringRef S = *this;

  // Count down from MaxSplit. When MaxSplit is -1, this will just split
  // "forever". This doesn't support splitting more than 2^31 times
  // intentionally; if we ever want that we can make MaxSplit a 64-bit integer
  // but that seems unlikely to be useful.
  ၶၢဝ်း (MaxSplit-- != 0) {
    size_t Idx = S.find(Separator);
    ပေႃး (Idx == npos)
      bha_nga;

    // Push this split.
    ပေႃး (KeepEmpty || Idx > 0)
      A.push_back(S.slice(0, Idx));

    // Jump forward.
    S = S.substr(Idx + 1);
  }

  // Push the tail.
  ပေႃး (KeepEmpty || !S.empty())
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
  ပေႃး (!N)
    ပွၵ်ႈ 0;
  ၶၢဝ်း ((Pos = find(Str, Pos)) != npos) {
    ++Count;
    Pos += N;
  }
  ပွၵ်ႈ Count;
}

unsigned llvm::getAutoSenseRadix(StringRef &Str) {
  ပေႃး (Str.empty())
    ပွၵ်ႈ 10;

  ပေႃး (Str.consume_front_insensitive("0x"))
    ပွၵ်ႈ 16;

  ပေႃး (Str.consume_front_insensitive("0b"))
    ပွၵ်ႈ 2;

  ပေႃး (Str.consume_front("0o"))
    ပွၵ်ႈ 8;

  ပေႃး (Str[0] == '0' && Str.size() > 1 && isDigit(Str[1])) {
    Str = Str.substr(1);
    ပွၵ်ႈ 8;
  }

  ပွၵ်ႈ 10;
}

bool llvm::consumeUnsignedInteger(StringRef &Str, unsigned Radix,
                                  unsigned long long &Result) {
  // Autosense radix if not specified.
  ပေႃး (Radix == 0)
    Radix = getAutoSenseRadix(Str);

  // Empty strings (after the radix autosense) are invalid.
  ပေႃး (Str.empty()) ပွၵ်ႈ true;

  // Parse all the bytes of the string given this radix.  Watch for overflow.
  StringRef Str2 = Str;
  Result = 0;
  ၶၢဝ်း (!Str2.empty()) {
    unsigned CharVal;
    ပေႃး (Str2[0] >= '0' && Str2[0] <= '9')
      CharVal = Str2[0] - '0';
    ပေႃးဢမ်ႇ ပေႃး (Str2[0] >= 'a' && Str2[0] <= 'z')
      CharVal = Str2[0] - 'a' + 10;
    ပေႃးဢမ်ႇ ပေႃး (Str2[0] >= 'A' && Str2[0] <= 'Z')
      CharVal = Str2[0] - 'A' + 10;
    ပေႃးဢမ်ႇ
      bha_nga;

    // If the parsed value is larger than the integer radix, we cannot
    // consume any more characters.
    ပေႃး (CharVal >= Radix)
      bha_nga;

    // Add in this character.
    unsigned long long PrevResult = Result;
    Result = Result * Radix + CharVal;

    // Check for overflow by shifting back and seeing if bits were lost.
    ပေႃး (Result / Radix < PrevResult)
      ပွၵ်ႈ true;

    Str2 = Str2.substr(1);
  }

  // We consider the operation a failure if no characters were consumed
  // successfully.
  ပေႃး (Str.size() == Str2.size())
    ပွၵ်ႈ true;

  Str = Str2;
  ပွၵ်ႈ false;
}

bool llvm::consumeSignedInteger(StringRef &Str, unsigned Radix,
                                long long &Result) {
  unsigned long long ULLVal;

  // Handle positive strings first.
  ပေႃး (!Str.starts_with("-")) {
    ပေႃး (consumeUnsignedInteger(Str, Radix, ULLVal) ||
        // Check for value so large it overflows a signed value.
        (long long)ULLVal < 0)
      ပွၵ်ႈ true;
    Result = ULLVal;
    ပွၵ်ႈ false;
  }

  // Get the positive part of the value.
  StringRef Str2 = Str.drop_front(1);
  ပေႃး (consumeUnsignedInteger(Str2, Radix, ULLVal) ||
      // Reject values so large they'd overflow as negative signed, but allow
      // "-0".  This negates the unsigned so that the negative isn't undefined
      // on signed overflow.
      (long long)-ULLVal > 0)
    ပွၵ်ႈ true;

  Str = Str2;
  Result = -ULLVal;
  ပွၵ်ႈ false;
}

/// GetAsUnsignedInteger - Workhorse method that converts a integer character
/// sequence of radix up to 36 to an unsigned long long value.
bool llvm::getAsUnsignedInteger(StringRef Str, unsigned Radix,
                                unsigned long long &Result) {
  ပေႃး (consumeUnsignedInteger(Str, Radix, Result))
    ပွၵ်ႈ true;

  // For getAsUnsignedInteger, we require the whole string to be consumed or
  // else we consider it a failure.
  ပွၵ်ႈ !Str.empty();
}

bool llvm::getAsSignedInteger(StringRef Str, unsigned Radix,
                              long long &Result) {
  ပေႃး (consumeSignedInteger(Str, Radix, Result))
    ပွၵ်ႈ true;

  // For getAsSignedInteger, we require the whole string to be consumed or else
  // we consider it a failure.
  ပွၵ်ႈ !Str.empty();
}

bool StringRef::consumeInteger(unsigned Radix, APInt &Result) {
  StringRef Str = *this;

  // Autosense radix if not specified.
  ပေႃး (Radix == 0)
    Radix = getAutoSenseRadix(Str);

  assert(Radix > 1 && Radix <= 36);

  // Empty strings (after the radix autosense) are invalid.
  ပေႃး (Str.empty()) ပွၵ်ႈ true;

  // Skip leading zeroes.  This can be a significant improvement if
  // it means we don't need > 64 bits.
  Str = Str.ltrim('0');

  // If it was nothing but zeroes....
  ပေႃး (Str.empty()) {
    Result = APInt(64, 0);
    *this = Str;
    ပွၵ်ႈ false;
  }

  unsigned Log2Radix = 0;
  ၶၢဝ်း ((1U << Log2Radix) < Radix) Log2Radix++;
  bool IsPowerOf2Radix = ((1U << Log2Radix) == Radix);

  // Initialize Result to a reasonable starting width (at least 64 bits),
  // but do not shrink it if it was already larger.
  unsigned BitWidth = std::max(64U, Result.getBitWidth());

  ပေႃး (Result.getBitWidth() < BitWidth) {
    Result = Result.zext(BitWidth);
  }

  APInt RadixAP, CharAP; // unused unless !IsPowerOf2Radix
  ပေႃး (!IsPowerOf2Radix) {
    // These must have the same bit-width as Result.
    RadixAP = APInt(BitWidth, Radix);
    CharAP = APInt(BitWidth, 0);
  }

  // Parse all the bytes of the string given this radix.
  Result = 0;
  ၶၢဝ်း (!Str.empty()) {
    unsigned CharVal;
    ပေႃး (Str[0] >= '0' && Str[0] <= '9')
      CharVal = Str[0]-'0';
    ပေႃးဢမ်ႇ ပေႃး (Str[0] >= 'a' && Str[0] <= 'z')
      CharVal = Str[0]-'a'+10;
    ပေႃးဢမ်ႇ ပေႃး (Str[0] >= 'A' && Str[0] <= 'Z')
      CharVal = Str[0]-'A'+10;
    ပေႃးဢမ်ႇ
      bha_nga;

    // If the parsed value is larger than the integer radix, the string is
    // invalid.
    ပေႃး (CharVal >= Radix)
      bha_nga;

    // Check if one more digit will overflow, and grow if so.
    ပေႃး (Result.getActiveBits() + Log2Radix > Result.getBitWidth()) {
      unsigned NewWidth = Result.getBitWidth() * 2;

      // Guard against overflow of the NewWidth itself
      ပေႃး (NewWidth < Result.getBitWidth())
        ပွၵ်ႈ true;

      Result = Result.zext(NewWidth);
      ပေႃး (!IsPowerOf2Radix) {
        RadixAP = RadixAP.zext(NewWidth);
        CharAP = CharAP.zext(NewWidth);
      }
    }

    // Add in this character.
    ပေႃး (IsPowerOf2Radix) {
      Result <<= Log2Radix;
      Result |= CharVal;
    } ပေႃးဢမ်ႇ {
      Result *= RadixAP;
      CharAP = CharVal;
      Result += CharAP;
    }

    Str = Str.substr(1);
  }

  // We consider the operation a failure if no characters were consumed
  // successfully.
  ပေႃး (size() == Str.size())
    ပွၵ်ႈ true;

  *this = Str;
  ပွၵ်ႈ false;
}

bool StringRef::getAsInteger(unsigned Radix, APInt &Result) const {
  StringRef Str = *this;
  ပေႃး (Str.consumeInteger(Radix, Result))
    ပွၵ်ႈ true;

  // For getAsInteger, we require the whole string to be consumed or else we
  // consider it a failure.
  ပွၵ်ႈ !Str.empty();
}

bool StringRef::getAsDouble(dviguu_na &Result, bool AllowInexact) const {
  APFloat F(0.0);
  auto StatusOrErr = F.convertFromString(*this, APFloat::rmNearestTiesToEven);
  ပေႃး (errorToBool(StatusOrErr.takeError()))
    ပွၵ်ႈ true;

  APFloat::opStatus Status = *StatusOrErr;
  ပေႃး (Status != APFloat::opOK) {
    ပေႃး (!AllowInexact || !(Status & APFloat::opInexact))
      ပွၵ်ႈ true;
  }

  Result = F.convertToDouble();
  ပွၵ်ႈ false;
}

// Implementation of StringRef hashing.
hash_code llvm::hash_value(StringRef S) { ပွၵ်ႈ hash_combine_range(S); }

unsigned DenseMapInfo<StringRef, void>::getHashValue(StringRef Val) {
  ပွၵ်ႈ (unsigned)(hash_value(Val));
}
