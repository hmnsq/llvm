//===-- StringRef.cpp - Lightweight String References ---------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#ಅನ್ತರ್ಭಾವ "llvm/ADT/StringRef.h"
#ಅನ್ತರ್ಭಾವ "llvm/ADT/APFloat.h"
#ಅನ್ತರ್ಭಾವ "llvm/ADT/APInt.h"
#ಅನ್ತರ್ಭಾವ "llvm/ADT/Hashing.h"
#ಅನ್ತರ್ಭಾವ "llvm/ADT/StringExtras.h"
#ಅನ್ತರ್ಭಾವ "llvm/ADT/edit_distance.h"
#ಅನ್ತರ್ಭಾವ "llvm/Support/Error.h"
#ಅನ್ತರ್ಭಾವ <bitset>

using ನಾಮಾಕಾಶ llvm;

// strncasecmp() is not available on non-POSIX systems, so define an
// alternative function here.
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ ascii_strncasecmp(StringRef LHS, StringRef RHS) {
  ಪ್ರತಿ (auto [LC, RC] : zip_equal(LHS, RHS)) {
    unsigned ವರ್ಣ LHC = toLower(LC);
    unsigned ವರ್ಣ RHC = toLower(RC);
    ಯದಿ (LHC != RHC)
      ಪ್ರತ್ಯಾಗಮ LHC < RHC ? -1 : 1;
  }
  ಪ್ರತ್ಯಾಗಮ 0;
}

ಪೂರ್ಣಾಙ್ಕ StringRef::compare_insensitive(StringRef RHS) ಧ್ರುವ {
  size_t Min = std::min(size(), RHS.size());
  ಯದಿ (ಪೂರ್ಣಾಙ್ಕ Res = ascii_strncasecmp(take_front(Min), RHS.take_front(Min)))
    ಪ್ರತ್ಯಾಗಮ Res;
  ಯದಿ (size() == RHS.size())
    ಪ್ರತ್ಯಾಗಮ 0;
  ಪ್ರತ್ಯಾಗಮ size() < RHS.size() ? -1 : 1;
}

ತಾರ್ಕಿಕ StringRef::starts_with_insensitive(StringRef Prefix) ಧ್ರುವ {
  ಪ್ರತ್ಯಾಗಮ size() >= Prefix.size() &&
         ascii_strncasecmp(take_front(Prefix.size()), Prefix) == 0;
}

ತಾರ್ಕಿಕ StringRef::ends_with_insensitive(StringRef Suffix) ಧ್ರುವ {
  ಪ್ರತ್ಯಾಗಮ size() >= Suffix.size() &&
         ascii_strncasecmp(take_back(Suffix.size()), Suffix) == 0;
}

size_t StringRef::find_insensitive(ವರ್ಣ C, size_t From) ಧ್ರುವ {
  ವರ್ಣ L = toLower(C);
  ಪ್ರತ್ಯಾಗಮ find_if([L](ವರ್ಣ D) { ಪ್ರತ್ಯಾಗಮ toLower(D) == L; }, From);
}

/// compare_numeric - Compare strings, handle embedded numbers.
ಪೂರ್ಣಾಙ್ಕ StringRef::compare_numeric(StringRef RHS) ಧ್ರುವ {
  ಪ್ರತಿ (size_t I = 0, E = std::min(size(), RHS.size()); I != E; ++I) {
    // Check for sequences of digits.
    ಯದಿ (isDigit(data()[I]) && isDigit(RHS.data()[I])) {
      // The longer sequence of numbers is considered larger.
      // This doesn't really handle prefixed zeros well.
      size_t J;
      ಪ್ರತಿ (J = I + 1; J != E + 1; ++J) {
        ತಾರ್ಕಿಕ ld = J < size() && isDigit(data()[J]);
        ತಾರ್ಕಿಕ rd = J < RHS.size() && isDigit(RHS.data()[J]);
        ಯದಿ (ld != rd)
          ಪ್ರತ್ಯಾಗಮ rd ? -1 : 1;
        ಯದಿ (!rd)
          ಭಙ್ಗ;
      }
      // The two number sequences have the same length (J-I), just memcmp them.
      ಯದಿ (ಪೂರ್ಣಾಙ್ಕ Res = compareMemory(data() + I, RHS.data() + I, J - I))
        ಪ್ರತ್ಯಾಗಮ Res < 0 ? -1 : 1;
      // Identical number sequences, continue search after the numbers.
      I = J - 1;
      ಅನುವರ್ತ;
    }
    ಯದಿ (data()[I] != RHS.data()[I])
      ಪ್ರತ್ಯಾಗಮ (unsigned ವರ್ಣ)data()[I] < (unsigned ವರ್ಣ)RHS.data()[I] ? -1 : 1;
  }
  ಯದಿ (size() == RHS.size())
    ಪ್ರತ್ಯಾಗಮ 0;
  ಪ್ರತ್ಯಾಗಮ size() < RHS.size() ? -1 : 1;
}

// Compute the edit distance between the two given strings.
unsigned StringRef::edit_distance(llvm::StringRef Other,
                                  ತಾರ್ಕಿಕ AllowReplacements,
                                  unsigned MaxEditDistance) ಧ್ರುವ {
  ಪ್ರತ್ಯಾಗಮ llvm::ComputeEditDistance(ArrayRef(data(), size()),
                                   ArrayRef(Other.data(), Other.size()),
                                   AllowReplacements, MaxEditDistance);
}

unsigned llvm::StringRef::edit_distance_insensitive(
    StringRef Other, ತಾರ್ಕಿಕ AllowReplacements, unsigned MaxEditDistance) ಧ್ರುವ {
  ಪ್ರತ್ಯಾಗಮ llvm::ComputeMappedEditDistance(
      ArrayRef(data(), size()), ArrayRef(Other.data(), Other.size()),
      llvm::toLower, AllowReplacements, MaxEditDistance);
}

//===----------------------------------------------------------------------===//
// String Operations
//===----------------------------------------------------------------------===//

std::string StringRef::lower() ಧ್ರುವ {
  ಪ್ರತ್ಯಾಗಮ std::string(map_iterator(begin(), toLower),
                     map_iterator(end(), toLower));
}

std::string StringRef::upper() ಧ್ರುವ {
  ಪ್ರತ್ಯಾಗಮ std::string(map_iterator(begin(), toUpper),
                     map_iterator(end(), toUpper));
}

//===----------------------------------------------------------------------===//
// String Searching
//===----------------------------------------------------------------------===//


/// find - Search for the first string \arg Str in the string.
///
/// \return - The index of the first occurrence of \arg Str, or npos if not
/// found.
size_t StringRef::find(StringRef Str, size_t From) ಧ್ರುವ {
  ಯದಿ (From > size())
    ಪ್ರತ್ಯಾಗಮ npos;

  ಧ್ರುವ ವರ್ಣ *Start = data() + From;
  size_t Size = size() - From;

  ಧ್ರುವ ವರ್ಣ *Needle = Str.data();
  size_t N = Str.size();
  ಯದಿ (N == 0)
    ಪ್ರತ್ಯಾಗಮ From;
  ಯದಿ (Size < N)
    ಪ್ರತ್ಯಾಗಮ npos;
  ಯದಿ (N == 1) {
    ಧ್ರುವ ವರ್ಣ *Ptr = (ಧ್ರುವ ವರ್ಣ *)::memchr(Start, Needle[0], Size);
    ಪ್ರತ್ಯಾಗಮ Ptr == nullptr ? npos : Ptr - data();
  }

  ಧ್ರುವ ವರ್ಣ *Stop = Start + (Size - N + 1);

  ಯದಿ (N == 2) {
    // Provide a fast path for newline finding (CRLF case) in InclusionRewriter.
    // Not the most optimized strategy, but getting memcmp inlined should be
    // good enough.
    ಕುರು {
      ಯದಿ (std::memcmp(Start, Needle, 2) == 0)
        ಪ್ರತ್ಯಾಗಮ Start - data();
      ++Start;
    } ಯಾವತ್ (Start < Stop);
    ಪ್ರತ್ಯಾಗಮ npos;
  }

  // For short haystacks or unsupported needles fall back to the naive algorithm
  ಯದಿ (Size < 16 || N > 255) {
    ಕುರು {
      ಯದಿ (std::memcmp(Start, Needle, N) == 0)
        ಪ್ರತ್ಯಾಗಮ Start - data();
      ++Start;
    } ಯಾವತ್ (Start < Stop);
    ಪ್ರತ್ಯಾಗಮ npos;
  }

  // Build the bad char heuristic table, with uint8_t to reduce cache thrashing.
  uint8_t BadCharSkip[256];
  std::memset(BadCharSkip, N, 256);
  ಪ್ರತಿ (unsigned i = 0; i != N-1; ++i)
    BadCharSkip[(uint8_t)Str[i]] = N-1-i;

  ಕುರು {
    uint8_t Last = Start[N - 1];
    ಯದಿ (LLVM_UNLIKELY(Last == (uint8_t)Needle[N - 1]))
      ಯದಿ (std::memcmp(Start, Needle, N - 1) == 0)
        ಪ್ರತ್ಯಾಗಮ Start - data();

    // Otherwise skip the appropriate number of bytes.
    Start += BadCharSkip[Last];
  } ಯಾವತ್ (Start < Stop);

  ಪ್ರತ್ಯಾಗಮ npos;
}

size_t StringRef::find_insensitive(StringRef Str, size_t From) ಧ್ರುವ {
  StringRef This = substr(From);
  ಯಾವತ್ (This.size() >= Str.size()) {
    ಯದಿ (This.starts_with_insensitive(Str))
      ಪ್ರತ್ಯಾಗಮ From;
    This = This.drop_front();
    ++From;
  }
  ಪ್ರತ್ಯಾಗಮ npos;
}

size_t StringRef::rfind_insensitive(ವರ್ಣ C, size_t From) ಧ್ರುವ {
  From = std::min(From, size());
  size_t i = From;
  ಯಾವತ್ (i != 0) {
    --i;
    ಯದಿ (toLower(data()[i]) == toLower(C))
      ಪ್ರತ್ಯಾಗಮ i;
  }
  ಪ್ರತ್ಯಾಗಮ npos;
}

/// rfind - Search for the last string \arg Str in the string.
///
/// \return - The index of the last occurrence of \arg Str, or npos if not
/// found.
size_t StringRef::rfind(StringRef Str) ಧ್ರುವ {
  ಪ್ರತ್ಯಾಗಮ std::string_view(*ಏತತ್).rfind(Str);
}

size_t StringRef::rfind_insensitive(StringRef Str) ಧ್ರುವ {
  size_t N = Str.size();
  ಯದಿ (N > size())
    ಪ್ರತ್ಯಾಗಮ npos;
  ಪ್ರತಿ (size_t i = size() - N + 1, e = 0; i != e;) {
    --i;
    ಯದಿ (substr(i, N).equals_insensitive(Str))
      ಪ್ರತ್ಯಾಗಮ i;
  }
  ಪ್ರತ್ಯಾಗಮ npos;
}

/// find_first_of - Find the first character in the string that is in \arg
/// Chars, or npos if not found.
///
/// Note: O(size() + Chars.size())
StringRef::size_type StringRef::find_first_of(StringRef Chars,
                                              size_t From) ಧ್ರುವ {
  std::bitset<1 << CHAR_BIT> CharBits;
  ಪ್ರತಿ (ವರ್ಣ C : Chars)
    CharBits.set((unsigned ವರ್ಣ)C);

  ಪ್ರತಿ (size_type i = std::min(From, size()), e = size(); i != e; ++i)
    ಯದಿ (CharBits.test((unsigned ವರ್ಣ)data()[i]))
      ಪ್ರತ್ಯಾಗಮ i;
  ಪ್ರತ್ಯಾಗಮ npos;
}

/// find_first_not_of - Find the first character in the string that is not
/// \arg C or npos if not found.
StringRef::size_type StringRef::find_first_not_of(ವರ್ಣ C, size_t From) ಧ್ರುವ {
  ಪ್ರತ್ಯಾಗಮ std::string_view(*ಏತತ್).find_first_not_of(C, From);
}

/// find_first_not_of - Find the first character in the string that is not
/// in the string \arg Chars, or npos if not found.
///
/// Note: O(size() + Chars.size())
StringRef::size_type StringRef::find_first_not_of(StringRef Chars,
                                                  size_t From) ಧ್ರುವ {
  std::bitset<1 << CHAR_BIT> CharBits;
  ಪ್ರತಿ (ವರ್ಣ C : Chars)
    CharBits.set((unsigned ವರ್ಣ)C);

  ಪ್ರತಿ (size_type i = std::min(From, size()), e = size(); i != e; ++i)
    ಯದಿ (!CharBits.test((unsigned ವರ್ಣ)data()[i]))
      ಪ್ರತ್ಯಾಗಮ i;
  ಪ್ರತ್ಯಾಗಮ npos;
}

/// find_last_of - Find the last character in the string that is in \arg C,
/// or npos if not found.
///
/// Note: O(size() + Chars.size())
StringRef::size_type StringRef::find_last_of(StringRef Chars,
                                             size_t From) ಧ್ರುವ {
  std::bitset<1 << CHAR_BIT> CharBits;
  ಪ್ರತಿ (ವರ್ಣ C : Chars)
    CharBits.set((unsigned ವರ್ಣ)C);

  ಪ್ರತಿ (size_type i = std::min(From, size()) - 1, e = -1; i != e; --i)
    ಯದಿ (CharBits.test((unsigned ವರ್ಣ)data()[i]))
      ಪ್ರತ್ಯಾಗಮ i;
  ಪ್ರತ್ಯಾಗಮ npos;
}

/// find_last_not_of - Find the last character in the string that is not
/// \arg C, or npos if not found.
StringRef::size_type StringRef::find_last_not_of(ವರ್ಣ C, size_t From) ಧ್ರುವ {
  ಪ್ರತಿ (size_type i = std::min(From, size()) - 1, e = -1; i != e; --i)
    ಯದಿ (data()[i] != C)
      ಪ್ರತ್ಯಾಗಮ i;
  ಪ್ರತ್ಯಾಗಮ npos;
}

/// find_last_not_of - Find the last character in the string that is not in
/// \arg Chars, or npos if not found.
///
/// Note: O(size() + Chars.size())
StringRef::size_type StringRef::find_last_not_of(StringRef Chars,
                                                 size_t From) ಧ್ರುವ {
  std::bitset<1 << CHAR_BIT> CharBits;
  ಪ್ರತಿ (ವರ್ಣ C : Chars)
    CharBits.set((unsigned ವರ್ಣ)C);

  ಪ್ರತಿ (size_type i = std::min(From, size()) - 1, e = -1; i != e; --i)
    ಯದಿ (!CharBits.test((unsigned ವರ್ಣ)data()[i]))
      ಪ್ರತ್ಯಾಗಮ i;
  ಪ್ರತ್ಯಾಗಮ npos;
}

ಶೂನ್ಯ StringRef::split(SmallVectorImpl<StringRef> &A,
                      StringRef Separator, ಪೂರ್ಣಾಙ್ಕ MaxSplit,
                      ತಾರ್ಕಿಕ KeepEmpty) ಧ್ರುವ {
  StringRef S = *ಏತತ್;

  // Count down from MaxSplit. When MaxSplit is -1, this will just split
  // "forever". This doesn't support splitting more than 2^31 times
  // intentionally; if we ever want that we can make MaxSplit a 64-bit integer
  // but that seems unlikely to be useful.
  ಯಾವತ್ (MaxSplit-- != 0) {
    size_t Idx = S.find(Separator);
    ಯದಿ (Idx == npos)
      ಭಙ್ಗ;

    // Push this split.
    ಯದಿ (KeepEmpty || Idx > 0)
      A.ತಳ್ಳು_back(S.slice(0, Idx));

    // Jump forward.
    S = S.substr(Idx + Separator.size());
  }

  // Push the tail.
  ಯದಿ (KeepEmpty || !S.empty())
    A.ತಳ್ಳು_back(S);
}

ಶೂನ್ಯ StringRef::split(SmallVectorImpl<StringRef> &A, ವರ್ಣ Separator,
                      ಪೂರ್ಣಾಙ್ಕ MaxSplit, ತಾರ್ಕಿಕ KeepEmpty) ಧ್ರುವ {
  StringRef S = *ಏತತ್;

  // Count down from MaxSplit. When MaxSplit is -1, this will just split
  // "forever". This doesn't support splitting more than 2^31 times
  // intentionally; if we ever want that we can make MaxSplit a 64-bit integer
  // but that seems unlikely to be useful.
  ಯಾವತ್ (MaxSplit-- != 0) {
    size_t Idx = S.find(Separator);
    ಯದಿ (Idx == npos)
      ಭಙ್ಗ;

    // Push this split.
    ಯದಿ (KeepEmpty || Idx > 0)
      A.ತಳ್ಳು_back(S.slice(0, Idx));

    // Jump forward.
    S = S.substr(Idx + 1);
  }

  // Push the tail.
  ಯದಿ (KeepEmpty || !S.empty())
    A.ತಳ್ಳು_back(S);
}

//===----------------------------------------------------------------------===//
// Helpful Algorithms
//===----------------------------------------------------------------------===//

/// count - Return the number of non-overlapped occurrences of \arg Str in
/// the string.
size_t StringRef::ಎಣಿಕೆ(StringRef Str) ಧ್ರುವ {
  size_t ಎಣಿಕೆ_2 = 0;
  size_t Pos = 0;
  size_t N = Str.size();
  // TODO: For an empty `Str` we return 0 for legacy reasons. Consider changing
  //       this to `Length + 1` which is more in-line with the function
  //       description.
  ಯದಿ (!N)
    ಪ್ರತ್ಯಾಗಮ 0;
  ಯಾವತ್ ((Pos = find(Str, Pos)) != npos) {
    ++ಎಣಿಕೆ_2;
    Pos += N;
  }
  ಪ್ರತ್ಯಾಗಮ ಎಣಿಕೆ_2;
}

unsigned llvm::getAutoSenseRadix(StringRef &Str) {
  ಯದಿ (Str.empty())
    ಪ್ರತ್ಯಾಗಮ 10;

  ಯದಿ (Str.consume_front_insensitive("0x"))
    ಪ್ರತ್ಯಾಗಮ 16;

  ಯದಿ (Str.consume_front_insensitive("0b"))
    ಪ್ರತ್ಯಾಗಮ 2;

  ಯದಿ (Str.consume_front("0o"))
    ಪ್ರತ್ಯಾಗಮ 8;

  ಯದಿ (Str[0] == '0' && Str.size() > 1 && isDigit(Str[1])) {
    Str = Str.substr(1);
    ಪ್ರತ್ಯಾಗಮ 8;
  }

  ಪ್ರತ್ಯಾಗಮ 10;
}

ತಾರ್ಕಿಕ llvm::consumeUnsignedInteger(StringRef &Str, unsigned Radix,
                                  unsigned long long &Result) {
  // Autosense radix if not specified.
  ಯದಿ (Radix == 0)
    Radix = getAutoSenseRadix(Str);

  // Empty strings (after the radix autosense) are invalid.
  ಯದಿ (Str.empty()) ಪ್ರತ್ಯಾಗಮ ಸತ್ಯ;

  // Parse all the bytes of the string given this radix.  Watch for overflow.
  StringRef Str2 = Str;
  Result = 0;
  ಯಾವತ್ (!Str2.empty()) {
    unsigned CharVal;
    ಯದಿ (Str2[0] >= '0' && Str2[0] <= '9')
      CharVal = Str2[0] - '0';
    ಅನ್ಯಥಾ ಯದಿ (Str2[0] >= 'a' && Str2[0] <= 'z')
      CharVal = Str2[0] - 'a' + 10;
    ಅನ್ಯಥಾ ಯದಿ (Str2[0] >= 'A' && Str2[0] <= 'Z')
      CharVal = Str2[0] - 'A' + 10;
    ಅನ್ಯಥಾ
      ಭಙ್ಗ;

    // If the parsed value is larger than the integer radix, we cannot
    // consume any more characters.
    ಯದಿ (CharVal >= Radix)
      ಭಙ್ಗ;

    // Add in this character.
    unsigned long long PrevResult = Result;
    Result = Result * Radix + CharVal;

    // Check for overflow by shifting back and seeing if bits were lost.
    ಯದಿ (Result / Radix < PrevResult)
      ಪ್ರತ್ಯಾಗಮ ಸತ್ಯ;

    Str2 = Str2.substr(1);
  }

  // We consider the operation a failure if no characters were consumed
  // successfully.
  ಯದಿ (Str.size() == Str2.size())
    ಪ್ರತ್ಯಾಗಮ ಸತ್ಯ;

  Str = Str2;
  ಪ್ರತ್ಯಾಗಮ ಅಸತ್ಯ;
}

ತಾರ್ಕಿಕ llvm::consumeSignedInteger(StringRef &Str, unsigned Radix,
                                long long &Result) {
  unsigned long long ULLVal;

  // Handle positive strings first.
  ಯದಿ (!Str.starts_with("-")) {
    ಯದಿ (consumeUnsignedInteger(Str, Radix, ULLVal) ||
        // Check for value so large it overflows a signed value.
        (long long)ULLVal < 0)
      ಪ್ರತ್ಯಾಗಮ ಸತ್ಯ;
    Result = ULLVal;
    ಪ್ರತ್ಯಾಗಮ ಅಸತ್ಯ;
  }

  // Get the positive part of the value.
  StringRef Str2 = Str.drop_front(1);
  ಯದಿ (consumeUnsignedInteger(Str2, Radix, ULLVal) ||
      // Reject values so large they'd overflow as negative signed, but allow
      // "-0".  This negates the unsigned so that the negative isn't undefined
      // on signed overflow.
      (long long)-ULLVal > 0)
    ಪ್ರತ್ಯಾಗಮ ಸತ್ಯ;

  Str = Str2;
  Result = -ULLVal;
  ಪ್ರತ್ಯಾಗಮ ಅಸತ್ಯ;
}

/// GetAsUnsignedInteger - Workhorse method that converts a integer character
/// sequence of radix up to 36 to an unsigned long long value.
ತಾರ್ಕಿಕ llvm::getAsUnsignedInteger(StringRef Str, unsigned Radix,
                                unsigned long long &Result) {
  ಯದಿ (consumeUnsignedInteger(Str, Radix, Result))
    ಪ್ರತ್ಯಾಗಮ ಸತ್ಯ;

  // For getAsUnsignedInteger, we require the whole string to be consumed or
  // else we consider it a failure.
  ಪ್ರತ್ಯಾಗಮ !Str.empty();
}

ತಾರ್ಕಿಕ llvm::getAsSignedInteger(StringRef Str, unsigned Radix,
                              long long &Result) {
  ಯದಿ (consumeSignedInteger(Str, Radix, Result))
    ಪ್ರತ್ಯಾಗಮ ಸತ್ಯ;

  // For getAsSignedInteger, we require the whole string to be consumed or else
  // we consider it a failure.
  ಪ್ರತ್ಯಾಗಮ !Str.empty();
}

ತಾರ್ಕಿಕ StringRef::consumeInteger(unsigned Radix, APInt &Result) {
  StringRef Str = *ಏತತ್;

  // Autosense radix if not specified.
  ಯದಿ (Radix == 0)
    Radix = getAutoSenseRadix(Str);

  ಪ್ರತಿಜ್ಞಾ(Radix > 1 && Radix <= 36);

  // Empty strings (after the radix autosense) are invalid.
  ಯದಿ (Str.empty()) ಪ್ರತ್ಯಾಗಮ ಸತ್ಯ;

  // Skip leading zeroes.  This can be a significant improvement if
  // it means we don't need > 64 bits.
  Str = Str.ltrim('0');

  // If it was nothing but zeroes....
  ಯದಿ (Str.empty()) {
    Result = APInt(64, 0);
    *ಏತತ್ = Str;
    ಪ್ರತ್ಯಾಗಮ ಅಸತ್ಯ;
  }

  unsigned Log2Radix = 0;
  ಯಾವತ್ ((1U << Log2Radix) < Radix) Log2Radix++;
  ತಾರ್ಕಿಕ IsPowerOf2Radix = ((1U << Log2Radix) == Radix);

  // Initialize Result to a reasonable starting width (at least 64 bits),
  // but do not shrink it if it was already larger.
  unsigned BitWidth = std::max(64U, Result.getBitWidth());

  ಯದಿ (Result.getBitWidth() < BitWidth) {
    Result = Result.zext(BitWidth);
  }

  APInt RadixAP, CharAP; // unused unless !IsPowerOf2Radix
  ಯದಿ (!IsPowerOf2Radix) {
    // These must have the same bit-width as Result.
    RadixAP = APInt(BitWidth, Radix);
    CharAP = APInt(BitWidth, 0);
  }

  // Parse all the bytes of the string given this radix.
  Result = 0;
  ಯಾವತ್ (!Str.empty()) {
    unsigned CharVal;
    ಯದಿ (Str[0] >= '0' && Str[0] <= '9')
      CharVal = Str[0]-'0';
    ಅನ್ಯಥಾ ಯದಿ (Str[0] >= 'a' && Str[0] <= 'z')
      CharVal = Str[0]-'a'+10;
    ಅನ್ಯಥಾ ಯದಿ (Str[0] >= 'A' && Str[0] <= 'Z')
      CharVal = Str[0]-'A'+10;
    ಅನ್ಯಥಾ
      ಭಙ್ಗ;

    // If the parsed value is larger than the integer radix, the string is
    // invalid.
    ಯದಿ (CharVal >= Radix)
      ಭಙ್ಗ;

    // Check if one more digit will overflow, and grow if so.
    ಯದಿ (Result.getActiveBits() + Log2Radix > Result.getBitWidth()) {
      unsigned NewWidth = Result.getBitWidth() * 2;

      // Guard against overflow of the NewWidth itself
      ಯದಿ (NewWidth < Result.getBitWidth())
        ಪ್ರತ್ಯಾಗಮ ಸತ್ಯ;

      Result = Result.zext(NewWidth);
      ಯದಿ (!IsPowerOf2Radix) {
        RadixAP = RadixAP.zext(NewWidth);
        CharAP = CharAP.zext(NewWidth);
      }
    }

    // Add in this character.
    ಯದಿ (IsPowerOf2Radix) {
      Result <<= Log2Radix;
      Result |= CharVal;
    } ಅನ್ಯಥಾ {
      Result *= RadixAP;
      CharAP = CharVal;
      Result += CharAP;
    }

    Str = Str.substr(1);
  }

  // We consider the operation a failure if no characters were consumed
  // successfully.
  ಯದಿ (size() == Str.size())
    ಪ್ರತ್ಯಾಗಮ ಸತ್ಯ;

  *ಏತತ್ = Str;
  ಪ್ರತ್ಯಾಗಮ ಅಸತ್ಯ;
}

ತಾರ್ಕಿಕ StringRef::getAsInteger(unsigned Radix, APInt &Result) ಧ್ರುವ {
  StringRef Str = *ಏತತ್;
  ಯದಿ (Str.consumeInteger(Radix, Result))
    ಪ್ರತ್ಯಾಗಮ ಸತ್ಯ;

  // For getAsInteger, we require the whole string to be consumed or else we
  // consider it a failure.
  ಪ್ರತ್ಯಾಗಮ !Str.empty();
}

ತಾರ್ಕಿಕ StringRef::getAsDouble(ದ್ವಿಗುಣ &Result, ತಾರ್ಕಿಕ AllowInexact) ಧ್ರುವ {
  APFloat F(0.0);
  auto StatusOrErr = F.convertFromString(*ಏತತ್, APFloat::rmNearestTiesToEven);
  ಯದಿ (errorToBool(StatusOrErr.takeError()))
    ಪ್ರತ್ಯಾಗಮ ಸತ್ಯ;

  APFloat::opStatus Status = *StatusOrErr;
  ಯದಿ (Status != APFloat::opOK) {
    ಯದಿ (!AllowInexact || !(Status & APFloat::opInexact))
      ಪ್ರತ್ಯಾಗಮ ಸತ್ಯ;
  }

  Result = F.convertToDouble();
  ಪ್ರತ್ಯಾಗಮ ಅಸತ್ಯ;
}

// Implementation of StringRef hashing.
hash_code llvm::hash_ಮೌಲ್ಯ(StringRef S) { ಪ್ರತ್ಯಾಗಮ hash_combine_range(S); }

unsigned DenseMapInfo<StringRef, ಶೂನ್ಯ>::get_Hash_ಮೌಲ್ಯ(StringRef Val) {
  ಪ್ರತ್ಯಾಗಮ (unsigned)(hash_ಮೌಲ್ಯ(Val));
}
