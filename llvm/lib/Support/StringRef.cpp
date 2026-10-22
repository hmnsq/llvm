//===-- StringRef.cpp - Lightweight String References ---------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#ଅନ୍ତର୍ଭାଵ "llvm/ADT/StringRef.h"
#ଅନ୍ତର୍ଭାଵ "llvm/ADT/APFloat.h"
#ଅନ୍ତର୍ଭାଵ "llvm/ADT/APInt.h"
#ଅନ୍ତର୍ଭାଵ "llvm/ADT/Hashing.h"
#ଅନ୍ତର୍ଭାଵ "llvm/ADT/StringExtras.h"
#ଅନ୍ତର୍ଭାଵ "llvm/ADT/edit_distance.h"
#ଅନ୍ତର୍ଭାଵ "llvm/Support/Error.h"
#ଅନ୍ତର୍ଭାଵ <bitset>

using ନାମାକାଶ llvm;

// strncasecmp() is not available on non-POSIX systems, so define an
// alternative function here.
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା ascii_strncasecmp(StringRef LHS, StringRef RHS) {
  ପାଇଁ (auto [LC, RC] : zip_equal(LHS, RHS)) {
    unsigned ଅକ୍ଷର LHC = toLower(LC);
    unsigned ଅକ୍ଷର RHC = toLower(RC);
    ଯଦି (LHC != RHC)
      ଫେରନ୍ତୁ LHC < RHC ? -1 : 1;
  }
  ଫେରନ୍ତୁ 0;
}

ପୂର୍ଣ୍ଣସଂଖ୍ୟା StringRef::compare_insensitive(StringRef RHS) ଅଚଳ {
  size_t Min = std::min(size(), RHS.size());
  ଯଦି (ପୂର୍ଣ୍ଣସଂଖ୍ୟା Res = ascii_strncasecmp(take_front(Min), RHS.take_front(Min)))
    ଫେରନ୍ତୁ Res;
  ଯଦି (size() == RHS.size())
    ଫେରନ୍ତୁ 0;
  ଫେରନ୍ତୁ size() < RHS.size() ? -1 : 1;
}

ତାର୍କିକ StringRef::starts_with_insensitive(StringRef Prefix) ଅଚଳ {
  ଫେରନ୍ତୁ size() >= Prefix.size() &&
         ascii_strncasecmp(take_front(Prefix.size()), Prefix) == 0;
}

ତାର୍କିକ StringRef::ends_with_insensitive(StringRef Suffix) ଅଚଳ {
  ଫେରନ୍ତୁ size() >= Suffix.size() &&
         ascii_strncasecmp(take_back(Suffix.size()), Suffix) == 0;
}

size_t StringRef::find_insensitive(ଅକ୍ଷର C, size_t From) ଅଚଳ {
  ଅକ୍ଷର L = toLower(C);
  ଫେରନ୍ତୁ find_if([L](ଅକ୍ଷର D) { ଫେରନ୍ତୁ toLower(D) == L; }, From);
}

/// compare_numeric - Compare strings, handle embedded numbers.
ପୂର୍ଣ୍ଣସଂଖ୍ୟା StringRef::compare_numeric(StringRef RHS) ଅଚଳ {
  ପାଇଁ (size_t I = 0, E = std::min(size(), RHS.size()); I != E; ++I) {
    // Check for sequences of digits.
    ଯଦି (isDigit(data()[I]) && isDigit(RHS.data()[I])) {
      // The longer sequence of numbers is considered larger.
      // This doesn't really handle prefixed zeros well.
      size_t J;
      ପାଇଁ (J = I + 1; J != E + 1; ++J) {
        ତାର୍କିକ ld = J < size() && isDigit(data()[J]);
        ତାର୍କିକ rd = J < RHS.size() && isDigit(RHS.data()[J]);
        ଯଦି (ld != rd)
          ଫେରନ୍ତୁ rd ? -1 : 1;
        ଯଦି (!rd)
          ଭାଙ୍ଗ;
      }
      // The two number sequences have the same length (J-I), just memcmp them.
      ଯଦି (ପୂର୍ଣ୍ଣସଂଖ୍ୟା Res = compareMemory(data() + I, RHS.data() + I, J - I))
        ଫେରନ୍ତୁ Res < 0 ? -1 : 1;
      // Identical number sequences, continue search after the numbers.
      I = J - 1;
      ଜାରି;
    }
    ଯଦି (data()[I] != RHS.data()[I])
      ଫେରନ୍ତୁ (unsigned ଅକ୍ଷର)data()[I] < (unsigned ଅକ୍ଷର)RHS.data()[I] ? -1 : 1;
  }
  ଯଦି (size() == RHS.size())
    ଫେରନ୍ତୁ 0;
  ଫେରନ୍ତୁ size() < RHS.size() ? -1 : 1;
}

// Compute the edit distance between the two given strings.
unsigned StringRef::edit_distance(llvm::StringRef Other,
                                  ତାର୍କିକ AllowReplacements,
                                  unsigned MaxEditDistance) ଅଚଳ {
  ଫେରନ୍ତୁ llvm::ComputeEditDistance(ArrayRef(data(), size()),
                                   ArrayRef(Other.data(), Other.size()),
                                   AllowReplacements, MaxEditDistance);
}

unsigned llvm::StringRef::edit_distance_insensitive(
    StringRef Other, ତାର୍କିକ AllowReplacements, unsigned MaxEditDistance) ଅଚଳ {
  ଫେରନ୍ତୁ llvm::ComputeMappedEditDistance(
      ArrayRef(data(), size()), ArrayRef(Other.data(), Other.size()),
      llvm::toLower, AllowReplacements, MaxEditDistance);
}

//===----------------------------------------------------------------------===//
// String Operations
//===----------------------------------------------------------------------===//

std::string StringRef::lower() ଅଚଳ {
  ଫେରନ୍ତୁ std::string(map_iterator(begin(), toLower),
                     map_iterator(end(), toLower));
}

std::string StringRef::upper() ଅଚଳ {
  ଫେରନ୍ତୁ std::string(map_iterator(begin(), toUpper),
                     map_iterator(end(), toUpper));
}

//===----------------------------------------------------------------------===//
// String Searching
//===----------------------------------------------------------------------===//


/// find - Search for the first string \arg Str in the string.
///
/// \return - The index of the first occurrence of \arg Str, or npos if not
/// found.
size_t StringRef::find(StringRef Str, size_t From) ଅଚଳ {
  ଯଦି (From > size())
    ଫେରନ୍ତୁ npos;

  ଅଚଳ ଅକ୍ଷର *Start = data() + From;
  size_t Size = size() - From;

  ଅଚଳ ଅକ୍ଷର *Needle = Str.data();
  size_t N = Str.size();
  ଯଦି (N == 0)
    ଫେରନ୍ତୁ From;
  ଯଦି (Size < N)
    ଫେରନ୍ତୁ npos;
  ଯଦି (N == 1) {
    ଅଚଳ ଅକ୍ଷର *Ptr = (ଅଚଳ ଅକ୍ଷର *)::memchr(Start, Needle[0], Size);
    ଫେରନ୍ତୁ Ptr == nullptr ? npos : Ptr - data();
  }

  ଅଚଳ ଅକ୍ଷର *Stop = Start + (Size - N + 1);

  ଯଦି (N == 2) {
    // Provide a fast path for newline finding (CRLF case) in InclusionRewriter.
    // Not the most optimized strategy, but getting memcmp inlined should be
    // good enough.
    କର {
      ଯଦି (std::memcmp(Start, Needle, 2) == 0)
        ଫେରନ୍ତୁ Start - data();
      ++Start;
    } ଯେତେବେଳେ (Start < Stop);
    ଫେରନ୍ତୁ npos;
  }

  // For short haystacks or unsupported needles fall back to the naive algorithm
  ଯଦି (Size < 16 || N > 255) {
    କର {
      ଯଦି (std::memcmp(Start, Needle, N) == 0)
        ଫେରନ୍ତୁ Start - data();
      ++Start;
    } ଯେତେବେଳେ (Start < Stop);
    ଫେରନ୍ତୁ npos;
  }

  // Build the bad char heuristic table, with uint8_t to reduce cache thrashing.
  uint8_t BadCharSkip[256];
  std::memset(BadCharSkip, N, 256);
  ପାଇଁ (unsigned i = 0; i != N-1; ++i)
    BadCharSkip[(uint8_t)Str[i]] = N-1-i;

  କର {
    uint8_t Last = Start[N - 1];
    ଯଦି (LLVM_UNLIKELY(Last == (uint8_t)Needle[N - 1]))
      ଯଦି (std::memcmp(Start, Needle, N - 1) == 0)
        ଫେରନ୍ତୁ Start - data();

    // Otherwise skip the appropriate number of bytes.
    Start += BadCharSkip[Last];
  } ଯେତେବେଳେ (Start < Stop);

  ଫେରନ୍ତୁ npos;
}

size_t StringRef::find_insensitive(StringRef Str, size_t From) ଅଚଳ {
  StringRef This = substr(From);
  ଯେତେବେଳେ (This.size() >= Str.size()) {
    ଯଦି (This.starts_with_insensitive(Str))
      ଫେରନ୍ତୁ From;
    This = This.drop_front();
    ++From;
  }
  ଫେରନ୍ତୁ npos;
}

size_t StringRef::rfind_insensitive(ଅକ୍ଷର C, size_t From) ଅଚଳ {
  From = std::min(From, size());
  size_t i = From;
  ଯେତେବେଳେ (i != 0) {
    --i;
    ଯଦି (toLower(data()[i]) == toLower(C))
      ଫେରନ୍ତୁ i;
  }
  ଫେରନ୍ତୁ npos;
}

/// rfind - Search for the last string \arg Str in the string.
///
/// \return - The index of the last occurrence of \arg Str, or npos if not
/// found.
size_t StringRef::rfind(StringRef Str) ଅଚଳ {
  ଫେରନ୍ତୁ std::string_view(*ଏହି).rfind(Str);
}

size_t StringRef::rfind_insensitive(StringRef Str) ଅଚଳ {
  size_t N = Str.size();
  ଯଦି (N > size())
    ଫେରନ୍ତୁ npos;
  ପାଇଁ (size_t i = size() - N + 1, e = 0; i != e;) {
    --i;
    ଯଦି (substr(i, N).equals_insensitive(Str))
      ଫେରନ୍ତୁ i;
  }
  ଫେରନ୍ତୁ npos;
}

/// find_first_of - Find the first character in the string that is in \arg
/// Chars, or npos if not found.
///
/// Note: O(size() + Chars.size())
StringRef::size_type StringRef::find_first_of(StringRef Chars,
                                              size_t From) ଅଚଳ {
  std::bitset<1 << CHAR_BIT> CharBits;
  ପାଇଁ (ଅକ୍ଷର C : Chars)
    CharBits.set((unsigned ଅକ୍ଷର)C);

  ପାଇଁ (size_type i = std::min(From, size()), e = size(); i != e; ++i)
    ଯଦି (CharBits.test((unsigned ଅକ୍ଷର)data()[i]))
      ଫେରନ୍ତୁ i;
  ଫେରନ୍ତୁ npos;
}

/// find_first_not_of - Find the first character in the string that is not
/// \arg C or npos if not found.
StringRef::size_type StringRef::find_first_not_of(ଅକ୍ଷର C, size_t From) ଅଚଳ {
  ଫେରନ୍ତୁ std::string_view(*ଏହି).find_first_not_of(C, From);
}

/// find_first_not_of - Find the first character in the string that is not
/// in the string \arg Chars, or npos if not found.
///
/// Note: O(size() + Chars.size())
StringRef::size_type StringRef::find_first_not_of(StringRef Chars,
                                                  size_t From) ଅଚଳ {
  std::bitset<1 << CHAR_BIT> CharBits;
  ପାଇଁ (ଅକ୍ଷର C : Chars)
    CharBits.set((unsigned ଅକ୍ଷର)C);

  ପାଇଁ (size_type i = std::min(From, size()), e = size(); i != e; ++i)
    ଯଦି (!CharBits.test((unsigned ଅକ୍ଷର)data()[i]))
      ଫେରନ୍ତୁ i;
  ଫେରନ୍ତୁ npos;
}

/// find_last_of - Find the last character in the string that is in \arg C,
/// or npos if not found.
///
/// Note: O(size() + Chars.size())
StringRef::size_type StringRef::find_last_of(StringRef Chars,
                                             size_t From) ଅଚଳ {
  std::bitset<1 << CHAR_BIT> CharBits;
  ପାଇଁ (ଅକ୍ଷର C : Chars)
    CharBits.set((unsigned ଅକ୍ଷର)C);

  ପାଇଁ (size_type i = std::min(From, size()) - 1, e = -1; i != e; --i)
    ଯଦି (CharBits.test((unsigned ଅକ୍ଷର)data()[i]))
      ଫେରନ୍ତୁ i;
  ଫେରନ୍ତୁ npos;
}

/// find_last_not_of - Find the last character in the string that is not
/// \arg C, or npos if not found.
StringRef::size_type StringRef::find_last_not_of(ଅକ୍ଷର C, size_t From) ଅଚଳ {
  ପାଇଁ (size_type i = std::min(From, size()) - 1, e = -1; i != e; --i)
    ଯଦି (data()[i] != C)
      ଫେରନ୍ତୁ i;
  ଫେରନ୍ତୁ npos;
}

/// find_last_not_of - Find the last character in the string that is not in
/// \arg Chars, or npos if not found.
///
/// Note: O(size() + Chars.size())
StringRef::size_type StringRef::find_last_not_of(StringRef Chars,
                                                 size_t From) ଅଚଳ {
  std::bitset<1 << CHAR_BIT> CharBits;
  ପାଇଁ (ଅକ୍ଷର C : Chars)
    CharBits.set((unsigned ଅକ୍ଷର)C);

  ପାଇଁ (size_type i = std::min(From, size()) - 1, e = -1; i != e; --i)
    ଯଦି (!CharBits.test((unsigned ଅକ୍ଷର)data()[i]))
      ଫେରନ୍ତୁ i;
  ଫେରନ୍ତୁ npos;
}

ଶୂନ୍ୟ StringRef::split(SmallVectorImpl<StringRef> &A,
                      StringRef Separator, ପୂର୍ଣ୍ଣସଂଖ୍ୟା MaxSplit,
                      ତାର୍କିକ KeepEmpty) ଅଚଳ {
  StringRef S = *ଏହି;

  // Count down from MaxSplit. When MaxSplit is -1, this will just split
  // "forever". This doesn't support splitting more than 2^31 times
  // intentionally; if we ever want that we can make MaxSplit a 64-bit integer
  // but that seems unlikely to be useful.
  ଯେତେବେଳେ (MaxSplit-- != 0) {
    size_t Idx = S.find(Separator);
    ଯଦି (Idx == npos)
      ଭାଙ୍ଗ;

    // Push this split.
    ଯଦି (KeepEmpty || Idx > 0)
      A.ଠେଲ_back(S.slice(0, Idx));

    // Jump forward.
    S = S.substr(Idx + Separator.size());
  }

  // Push the tail.
  ଯଦି (KeepEmpty || !S.empty())
    A.ଠେଲ_back(S);
}

ଶୂନ୍ୟ StringRef::split(SmallVectorImpl<StringRef> &A, ଅକ୍ଷର Separator,
                      ପୂର୍ଣ୍ଣସଂଖ୍ୟା MaxSplit, ତାର୍କିକ KeepEmpty) ଅଚଳ {
  StringRef S = *ଏହି;

  // Count down from MaxSplit. When MaxSplit is -1, this will just split
  // "forever". This doesn't support splitting more than 2^31 times
  // intentionally; if we ever want that we can make MaxSplit a 64-bit integer
  // but that seems unlikely to be useful.
  ଯେତେବେଳେ (MaxSplit-- != 0) {
    size_t Idx = S.find(Separator);
    ଯଦି (Idx == npos)
      ଭାଙ୍ଗ;

    // Push this split.
    ଯଦି (KeepEmpty || Idx > 0)
      A.ଠେଲ_back(S.slice(0, Idx));

    // Jump forward.
    S = S.substr(Idx + 1);
  }

  // Push the tail.
  ଯଦି (KeepEmpty || !S.empty())
    A.ଠେଲ_back(S);
}

//===----------------------------------------------------------------------===//
// Helpful Algorithms
//===----------------------------------------------------------------------===//

/// count - Return the number of non-overlapped occurrences of \arg Str in
/// the string.
size_t StringRef::ଗଣନା_2(StringRef Str) ଅଚଳ {
  size_t ଗଣନା_3 = 0;
  size_t Pos = 0;
  size_t N = Str.size();
  // TODO: For an empty `Str` we return 0 for legacy reasons. Consider changing
  //       this to `Length + 1` which is more in-line with the function
  //       description.
  ଯଦି (!N)
    ଫେରନ୍ତୁ 0;
  ଯେତେବେଳେ ((Pos = find(Str, Pos)) != npos) {
    ++ଗଣନା_3;
    Pos += N;
  }
  ଫେରନ୍ତୁ ଗଣନା_3;
}

unsigned llvm::getAutoSenseRadix(StringRef &Str) {
  ଯଦି (Str.empty())
    ଫେରନ୍ତୁ 10;

  ଯଦି (Str.consume_front_insensitive("0x"))
    ଫେରନ୍ତୁ 16;

  ଯଦି (Str.consume_front_insensitive("0b"))
    ଫେରନ୍ତୁ 2;

  ଯଦି (Str.consume_front("0o"))
    ଫେରନ୍ତୁ 8;

  ଯଦି (Str[0] == '0' && Str.size() > 1 && isDigit(Str[1])) {
    Str = Str.substr(1);
    ଫେରନ୍ତୁ 8;
  }

  ଫେରନ୍ତୁ 10;
}

ତାର୍କିକ llvm::consumeUnsignedInteger(StringRef &Str, unsigned Radix,
                                  unsigned long long &Result) {
  // Autosense radix if not specified.
  ଯଦି (Radix == 0)
    Radix = getAutoSenseRadix(Str);

  // Empty strings (after the radix autosense) are invalid.
  ଯଦି (Str.empty()) ଫେରନ୍ତୁ ସତ୍ୟ;

  // Parse all the bytes of the string given this radix.  Watch for overflow.
  StringRef Str2 = Str;
  Result = 0;
  ଯେତେବେଳେ (!Str2.empty()) {
    unsigned CharVal;
    ଯଦି (Str2[0] >= '0' && Str2[0] <= '9')
      CharVal = Str2[0] - '0';
    ଅନ୍ୟଥା ଯଦି (Str2[0] >= 'a' && Str2[0] <= 'z')
      CharVal = Str2[0] - 'a' + 10;
    ଅନ୍ୟଥା ଯଦି (Str2[0] >= 'A' && Str2[0] <= 'Z')
      CharVal = Str2[0] - 'A' + 10;
    ଅନ୍ୟଥା
      ଭାଙ୍ଗ;

    // If the parsed value is larger than the integer radix, we cannot
    // consume any more characters.
    ଯଦି (CharVal >= Radix)
      ଭାଙ୍ଗ;

    // Add in this character.
    unsigned long long PrevResult = Result;
    Result = Result * Radix + CharVal;

    // Check for overflow by shifting back and seeing if bits were lost.
    ଯଦି (Result / Radix < PrevResult)
      ଫେରନ୍ତୁ ସତ୍ୟ;

    Str2 = Str2.substr(1);
  }

  // We consider the operation a failure if no characters were consumed
  // successfully.
  ଯଦି (Str.size() == Str2.size())
    ଫେରନ୍ତୁ ସତ୍ୟ;

  Str = Str2;
  ଫେରନ୍ତୁ ମିଥ୍ୟା;
}

ତାର୍କିକ llvm::consumeSignedInteger(StringRef &Str, unsigned Radix,
                                long long &Result) {
  unsigned long long ULLVal;

  // Handle positive strings first.
  ଯଦି (!Str.starts_with("-")) {
    ଯଦି (consumeUnsignedInteger(Str, Radix, ULLVal) ||
        // Check for value so large it overflows a signed value.
        (long long)ULLVal < 0)
      ଫେରନ୍ତୁ ସତ୍ୟ;
    Result = ULLVal;
    ଫେରନ୍ତୁ ମିଥ୍ୟା;
  }

  // Get the positive part of the value.
  StringRef Str2 = Str.drop_front(1);
  ଯଦି (consumeUnsignedInteger(Str2, Radix, ULLVal) ||
      // Reject values so large they'd overflow as negative signed, but allow
      // "-0".  This negates the unsigned so that the negative isn't undefined
      // on signed overflow.
      (long long)-ULLVal > 0)
    ଫେରନ୍ତୁ ସତ୍ୟ;

  Str = Str2;
  Result = -ULLVal;
  ଫେରନ୍ତୁ ମିଥ୍ୟା;
}

/// GetAsUnsignedInteger - Workhorse method that converts a integer character
/// sequence of radix up to 36 to an unsigned long long value.
ତାର୍କିକ llvm::getAsUnsignedInteger(StringRef Str, unsigned Radix,
                                unsigned long long &Result) {
  ଯଦି (consumeUnsignedInteger(Str, Radix, Result))
    ଫେରନ୍ତୁ ସତ୍ୟ;

  // For getAsUnsignedInteger, we require the whole string to be consumed or
  // else we consider it a failure.
  ଫେରନ୍ତୁ !Str.empty();
}

ତାର୍କିକ llvm::getAsSignedInteger(StringRef Str, unsigned Radix,
                              long long &Result) {
  ଯଦି (consumeSignedInteger(Str, Radix, Result))
    ଫେରନ୍ତୁ ସତ୍ୟ;

  // For getAsSignedInteger, we require the whole string to be consumed or else
  // we consider it a failure.
  ଫେରନ୍ତୁ !Str.empty();
}

ତାର୍କିକ StringRef::consumeInteger(unsigned Radix, APInt &Result) {
  StringRef Str = *ଏହି;

  // Autosense radix if not specified.
  ଯଦି (Radix == 0)
    Radix = getAutoSenseRadix(Str);

  ପ୍ରତିଜ୍ଞା(Radix > 1 && Radix <= 36);

  // Empty strings (after the radix autosense) are invalid.
  ଯଦି (Str.empty()) ଫେରନ୍ତୁ ସତ୍ୟ;

  // Skip leading zeroes.  This can be a significant improvement if
  // it means we don't need > 64 bits.
  Str = Str.ltrim('0');

  // If it was nothing but zeroes....
  ଯଦି (Str.empty()) {
    Result = APInt(64, 0);
    *ଏହି = Str;
    ଫେରନ୍ତୁ ମିଥ୍ୟା;
  }

  unsigned Log2Radix = 0;
  ଯେତେବେଳେ ((1U << Log2Radix) < Radix) Log2Radix++;
  ତାର୍କିକ IsPowerOf2Radix = ((1U << Log2Radix) == Radix);

  // Initialize Result to a reasonable starting width (at least 64 bits),
  // but do not shrink it if it was already larger.
  unsigned BitWidth = std::max(64U, Result.getBitWidth());

  ଯଦି (Result.getBitWidth() < BitWidth) {
    Result = Result.zext(BitWidth);
  }

  APInt RadixAP, CharAP; // unused unless !IsPowerOf2Radix
  ଯଦି (!IsPowerOf2Radix) {
    // These must have the same bit-width as Result.
    RadixAP = APInt(BitWidth, Radix);
    CharAP = APInt(BitWidth, 0);
  }

  // Parse all the bytes of the string given this radix.
  Result = 0;
  ଯେତେବେଳେ (!Str.empty()) {
    unsigned CharVal;
    ଯଦି (Str[0] >= '0' && Str[0] <= '9')
      CharVal = Str[0]-'0';
    ଅନ୍ୟଥା ଯଦି (Str[0] >= 'a' && Str[0] <= 'z')
      CharVal = Str[0]-'a'+10;
    ଅନ୍ୟଥା ଯଦି (Str[0] >= 'A' && Str[0] <= 'Z')
      CharVal = Str[0]-'A'+10;
    ଅନ୍ୟଥା
      ଭାଙ୍ଗ;

    // If the parsed value is larger than the integer radix, the string is
    // invalid.
    ଯଦି (CharVal >= Radix)
      ଭାଙ୍ଗ;

    // Check if one more digit will overflow, and grow if so.
    ଯଦି (Result.getActiveBits() + Log2Radix > Result.getBitWidth()) {
      unsigned NewWidth = Result.getBitWidth() * 2;

      // Guard against overflow of the NewWidth itself
      ଯଦି (NewWidth < Result.getBitWidth())
        ଫେରନ୍ତୁ ସତ୍ୟ;

      Result = Result.zext(NewWidth);
      ଯଦି (!IsPowerOf2Radix) {
        RadixAP = RadixAP.zext(NewWidth);
        CharAP = CharAP.zext(NewWidth);
      }
    }

    // Add in this character.
    ଯଦି (IsPowerOf2Radix) {
      Result <<= Log2Radix;
      Result |= CharVal;
    } ଅନ୍ୟଥା {
      Result *= RadixAP;
      CharAP = CharVal;
      Result += CharAP;
    }

    Str = Str.substr(1);
  }

  // We consider the operation a failure if no characters were consumed
  // successfully.
  ଯଦି (size() == Str.size())
    ଫେରନ୍ତୁ ସତ୍ୟ;

  *ଏହି = Str;
  ଫେରନ୍ତୁ ମିଥ୍ୟା;
}

ତାର୍କିକ StringRef::getAsInteger(unsigned Radix, APInt &Result) ଅଚଳ {
  StringRef Str = *ଏହି;
  ଯଦି (Str.consumeInteger(Radix, Result))
    ଫେରନ୍ତୁ ସତ୍ୟ;

  // For getAsInteger, we require the whole string to be consumed or else we
  // consider it a failure.
  ଫେରନ୍ତୁ !Str.empty();
}

ତାର୍କିକ StringRef::getAsDouble(ଦ୍ୱିଗୁଣ &Result, ତାର୍କିକ AllowInexact) ଅଚଳ {
  APFloat F(0.0);
  auto StatusOrErr = F.convertFromString(*ଏହି, APFloat::rmNearestTiesToEven);
  ଯଦି (errorToBool(StatusOrErr.takeError()))
    ଫେରନ୍ତୁ ସତ୍ୟ;

  APFloat::opStatus Status = *StatusOrErr;
  ଯଦି (Status != APFloat::opOK) {
    ଯଦି (!AllowInexact || !(Status & APFloat::opInexact))
      ଫେରନ୍ତୁ ସତ୍ୟ;
  }

  Result = F.convertToDouble();
  ଫେରନ୍ତୁ ମିଥ୍ୟା;
}

// Implementation of StringRef hashing.
hash_code llvm::hash_ମୂଲ୍ୟ(StringRef S) { ଫେରନ୍ତୁ hash_combine_range(S); }

unsigned DenseMapInfo<StringRef, ଶୂନ୍ୟ>::get_Hash_ମୂଲ୍ୟ(StringRef Val) {
  ଫେରନ୍ତୁ (unsigned)(hash_ମୂଲ୍ୟ(Val));
}
