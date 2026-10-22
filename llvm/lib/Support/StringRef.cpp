//===-- StringRef.cpp - Lightweight String References ---------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#অন্তর্ভাৱ "llvm/ADT/StringRef.h"
#অন্তর্ভাৱ "llvm/ADT/APFloat.h"
#অন্তর্ভাৱ "llvm/ADT/APInt.h"
#অন্তর্ভাৱ "llvm/ADT/Hashing.h"
#অন্তর্ভাৱ "llvm/ADT/StringExtras.h"
#অন্তর্ভাৱ "llvm/ADT/edit_distance.h"
#অন্তর্ভাৱ "llvm/Support/Error.h"
#অন্তর্ভাৱ <bitset>

using নামাকাশ llvm;

// strncasecmp() is not available on non-POSIX systems, so define an
// alternative function here.
স্থির পূর্ণাঙ্ক ascii_strncasecmp(StringRef LHS, StringRef RHS) {
  বাবে (auto [LC, RC] : zip_equal(LHS, RHS)) {
    unsigned ৱর্ণ LHC = toLower(LC);
    unsigned ৱর্ণ RHC = toLower(RC);
    যদি (LHC != RHC)
      ঘূৰাই LHC < RHC ? -1 : 1;
  }
  ঘূৰাই 0;
}

পূর্ণাঙ্ক StringRef::compare_insensitive(StringRef RHS) ধ্রুৱ {
  size_t Min = std::min(size(), RHS.size());
  যদি (পূর্ণাঙ্ক Res = ascii_strncasecmp(take_front(Min), RHS.take_front(Min)))
    ঘূৰাই Res;
  যদি (size() == RHS.size())
    ঘূৰাই 0;
  ঘূৰাই size() < RHS.size() ? -1 : 1;
}

তার্কিক StringRef::starts_with_insensitive(StringRef Prefix) ধ্রুৱ {
  ঘূৰাই size() >= Prefix.size() &&
         ascii_strncasecmp(take_front(Prefix.size()), Prefix) == 0;
}

তার্কিক StringRef::ends_with_insensitive(StringRef Suffix) ধ্রুৱ {
  ঘূৰাই size() >= Suffix.size() &&
         ascii_strncasecmp(take_back(Suffix.size()), Suffix) == 0;
}

size_t StringRef::find_insensitive(ৱর্ণ C, size_t From) ধ্রুৱ {
  ৱর্ণ L = toLower(C);
  ঘূৰাই find_if([L](ৱর্ণ D) { ঘূৰাই toLower(D) == L; }, From);
}

/// compare_numeric - Compare strings, handle embedded numbers.
পূর্ণাঙ্ক StringRef::compare_numeric(StringRef RHS) ধ্রুৱ {
  বাবে (size_t I = 0, E = std::min(size(), RHS.size()); I != E; ++I) {
    // Check for sequences of digits.
    যদি (isDigit(data()[I]) && isDigit(RHS.data()[I])) {
      // The longer sequence of numbers is considered larger.
      // This doesn't really handle prefixed zeros well.
      size_t J;
      বাবে (J = I + 1; J != E + 1; ++J) {
        তার্কিক ld = J < size() && isDigit(data()[J]);
        তার্কিক rd = J < RHS.size() && isDigit(RHS.data()[J]);
        যদি (ld != rd)
          ঘূৰাই rd ? -1 : 1;
        যদি (!rd)
          ভঙ্গ;
      }
      // The two number sequences have the same length (J-I), just memcmp them.
      যদি (পূর্ণাঙ্ক Res = compareMemory(data() + I, RHS.data() + I, J - I))
        ঘূৰাই Res < 0 ? -1 : 1;
      // Identical number sequences, continue search after the numbers.
      I = J - 1;
      অনুৱর্ত;
    }
    যদি (data()[I] != RHS.data()[I])
      ঘূৰাই (unsigned ৱর্ণ)data()[I] < (unsigned ৱর্ণ)RHS.data()[I] ? -1 : 1;
  }
  যদি (size() == RHS.size())
    ঘূৰাই 0;
  ঘূৰাই size() < RHS.size() ? -1 : 1;
}

// Compute the edit distance between the two given strings.
unsigned StringRef::edit_distance(llvm::StringRef Other,
                                  তার্কিক AllowReplacements,
                                  unsigned MaxEditDistance) ধ্রুৱ {
  ঘূৰাই llvm::ComputeEditDistance(ArrayRef(data(), size()),
                                   ArrayRef(Other.data(), Other.size()),
                                   AllowReplacements, MaxEditDistance);
}

unsigned llvm::StringRef::edit_distance_insensitive(
    StringRef Other, তার্কিক AllowReplacements, unsigned MaxEditDistance) ধ্রুৱ {
  ঘূৰাই llvm::ComputeMappedEditDistance(
      ArrayRef(data(), size()), ArrayRef(Other.data(), Other.size()),
      llvm::toLower, AllowReplacements, MaxEditDistance);
}

//===----------------------------------------------------------------------===//
// String Operations
//===----------------------------------------------------------------------===//

std::string StringRef::lower() ধ্রুৱ {
  ঘূৰাই std::string(map_iterator(begin(), toLower),
                     map_iterator(end(), toLower));
}

std::string StringRef::upper() ধ্রুৱ {
  ঘূৰাই std::string(map_iterator(begin(), toUpper),
                     map_iterator(end(), toUpper));
}

//===----------------------------------------------------------------------===//
// String Searching
//===----------------------------------------------------------------------===//


/// find - Search for the first string \arg Str in the string.
///
/// \return - The index of the first occurrence of \arg Str, or npos if not
/// found.
size_t StringRef::find(StringRef Str, size_t From) ধ্রুৱ {
  যদি (From > size())
    ঘূৰাই npos;

  ধ্রুৱ ৱর্ণ *Start = data() + From;
  size_t Size = size() - From;

  ধ্রুৱ ৱর্ণ *Needle = Str.data();
  size_t N = Str.size();
  যদি (N == 0)
    ঘূৰাই From;
  যদি (Size < N)
    ঘূৰাই npos;
  যদি (N == 1) {
    ধ্রুৱ ৱর্ণ *Ptr = (ধ্রুৱ ৱর্ণ *)::memchr(Start, Needle[0], Size);
    ঘূৰাই Ptr == nullptr ? npos : Ptr - data();
  }

  ধ্রুৱ ৱর্ণ *Stop = Start + (Size - N + 1);

  যদি (N == 2) {
    // Provide a fast path for newline finding (CRLF case) in InclusionRewriter.
    // Not the most optimized strategy, but getting memcmp inlined should be
    // good enough.
    কৰা {
      যদি (std::memcmp(Start, Needle, 2) == 0)
        ঘূৰাই Start - data();
      ++Start;
    } যেতিয়ালৈ (Start < Stop);
    ঘূৰাই npos;
  }

  // For short haystacks or unsupported needles fall back to the naive algorithm
  যদি (Size < 16 || N > 255) {
    কৰা {
      যদি (std::memcmp(Start, Needle, N) == 0)
        ঘূৰাই Start - data();
      ++Start;
    } যেতিয়ালৈ (Start < Stop);
    ঘূৰাই npos;
  }

  // Build the bad char heuristic table, with uint8_t to reduce cache thrashing.
  uint8_t BadCharSkip[256];
  std::memset(BadCharSkip, N, 256);
  বাবে (unsigned i = 0; i != N-1; ++i)
    BadCharSkip[(uint8_t)Str[i]] = N-1-i;

  কৰা {
    uint8_t Last = Start[N - 1];
    যদি (LLVM_UNLIKELY(Last == (uint8_t)Needle[N - 1]))
      যদি (std::memcmp(Start, Needle, N - 1) == 0)
        ঘূৰাই Start - data();

    // Otherwise skip the appropriate number of bytes.
    Start += BadCharSkip[Last];
  } যেতিয়ালৈ (Start < Stop);

  ঘূৰাই npos;
}

size_t StringRef::find_insensitive(StringRef Str, size_t From) ধ্রুৱ {
  StringRef This = substr(From);
  যেতিয়ালৈ (This.size() >= Str.size()) {
    যদি (This.starts_with_insensitive(Str))
      ঘূৰাই From;
    This = This.drop_front();
    ++From;
  }
  ঘূৰাই npos;
}

size_t StringRef::rfind_insensitive(ৱর্ণ C, size_t From) ধ্রুৱ {
  From = std::min(From, size());
  size_t i = From;
  যেতিয়ালৈ (i != 0) {
    --i;
    যদি (toLower(data()[i]) == toLower(C))
      ঘূৰাই i;
  }
  ঘূৰাই npos;
}

/// rfind - Search for the last string \arg Str in the string.
///
/// \return - The index of the last occurrence of \arg Str, or npos if not
/// found.
size_t StringRef::rfind(StringRef Str) ধ্রুৱ {
  ঘূৰাই std::string_view(*এতত্).rfind(Str);
}

size_t StringRef::rfind_insensitive(StringRef Str) ধ্রুৱ {
  size_t N = Str.size();
  যদি (N > size())
    ঘূৰাই npos;
  বাবে (size_t i = size() - N + 1, e = 0; i != e;) {
    --i;
    যদি (substr(i, N).equals_insensitive(Str))
      ঘূৰাই i;
  }
  ঘূৰাই npos;
}

/// find_first_of - Find the first character in the string that is in \arg
/// Chars, or npos if not found.
///
/// Note: O(size() + Chars.size())
StringRef::size_type StringRef::find_first_of(StringRef Chars,
                                              size_t From) ধ্রুৱ {
  std::bitset<1 << CHAR_BIT> CharBits;
  বাবে (ৱর্ণ C : Chars)
    CharBits.set((unsigned ৱর্ণ)C);

  বাবে (size_type i = std::min(From, size()), e = size(); i != e; ++i)
    যদি (CharBits.test((unsigned ৱর্ণ)data()[i]))
      ঘূৰাই i;
  ঘূৰাই npos;
}

/// find_first_not_of - Find the first character in the string that is not
/// \arg C or npos if not found.
StringRef::size_type StringRef::find_first_not_of(ৱর্ণ C, size_t From) ধ্রুৱ {
  ঘূৰাই std::string_view(*এতত্).find_first_not_of(C, From);
}

/// find_first_not_of - Find the first character in the string that is not
/// in the string \arg Chars, or npos if not found.
///
/// Note: O(size() + Chars.size())
StringRef::size_type StringRef::find_first_not_of(StringRef Chars,
                                                  size_t From) ধ্রুৱ {
  std::bitset<1 << CHAR_BIT> CharBits;
  বাবে (ৱর্ণ C : Chars)
    CharBits.set((unsigned ৱর্ণ)C);

  বাবে (size_type i = std::min(From, size()), e = size(); i != e; ++i)
    যদি (!CharBits.test((unsigned ৱর্ণ)data()[i]))
      ঘূৰাই i;
  ঘূৰাই npos;
}

/// find_last_of - Find the last character in the string that is in \arg C,
/// or npos if not found.
///
/// Note: O(size() + Chars.size())
StringRef::size_type StringRef::find_last_of(StringRef Chars,
                                             size_t From) ধ্রুৱ {
  std::bitset<1 << CHAR_BIT> CharBits;
  বাবে (ৱর্ণ C : Chars)
    CharBits.set((unsigned ৱর্ণ)C);

  বাবে (size_type i = std::min(From, size()) - 1, e = -1; i != e; --i)
    যদি (CharBits.test((unsigned ৱর্ণ)data()[i]))
      ঘূৰাই i;
  ঘূৰাই npos;
}

/// find_last_not_of - Find the last character in the string that is not
/// \arg C, or npos if not found.
StringRef::size_type StringRef::find_last_not_of(ৱর্ণ C, size_t From) ধ্রুৱ {
  বাবে (size_type i = std::min(From, size()) - 1, e = -1; i != e; --i)
    যদি (data()[i] != C)
      ঘূৰাই i;
  ঘূৰাই npos;
}

/// find_last_not_of - Find the last character in the string that is not in
/// \arg Chars, or npos if not found.
///
/// Note: O(size() + Chars.size())
StringRef::size_type StringRef::find_last_not_of(StringRef Chars,
                                                 size_t From) ধ্রুৱ {
  std::bitset<1 << CHAR_BIT> CharBits;
  বাবে (ৱর্ণ C : Chars)
    CharBits.set((unsigned ৱর্ণ)C);

  বাবে (size_type i = std::min(From, size()) - 1, e = -1; i != e; --i)
    যদি (!CharBits.test((unsigned ৱর্ণ)data()[i]))
      ঘূৰাই i;
  ঘূৰাই npos;
}

শূন্য StringRef::split(SmallVectorImpl<StringRef> &A,
                      StringRef Separator, পূর্ণাঙ্ক MaxSplit,
                      তার্কিক KeepEmpty) ধ্রুৱ {
  StringRef S = *এতত্;

  // Count down from MaxSplit. When MaxSplit is -1, this will just split
  // "forever". This doesn't support splitting more than 2^31 times
  // intentionally; if we ever want that we can make MaxSplit a 64-bit integer
  // but that seems unlikely to be useful.
  যেতিয়ালৈ (MaxSplit-- != 0) {
    size_t Idx = S.find(Separator);
    যদি (Idx == npos)
      ভঙ্গ;

    // Push this split.
    যদি (KeepEmpty || Idx > 0)
      A.ঠেলো_back(S.slice(0, Idx));

    // Jump forward.
    S = S.substr(Idx + Separator.size());
  }

  // Push the tail.
  যদি (KeepEmpty || !S.empty())
    A.ঠেলো_back(S);
}

শূন্য StringRef::split(SmallVectorImpl<StringRef> &A, ৱর্ণ Separator,
                      পূর্ণাঙ্ক MaxSplit, তার্কিক KeepEmpty) ধ্রুৱ {
  StringRef S = *এতত্;

  // Count down from MaxSplit. When MaxSplit is -1, this will just split
  // "forever". This doesn't support splitting more than 2^31 times
  // intentionally; if we ever want that we can make MaxSplit a 64-bit integer
  // but that seems unlikely to be useful.
  যেতিয়ালৈ (MaxSplit-- != 0) {
    size_t Idx = S.find(Separator);
    যদি (Idx == npos)
      ভঙ্গ;

    // Push this split.
    যদি (KeepEmpty || Idx > 0)
      A.ঠেলো_back(S.slice(0, Idx));

    // Jump forward.
    S = S.substr(Idx + 1);
  }

  // Push the tail.
  যদি (KeepEmpty || !S.empty())
    A.ঠেলো_back(S);
}

//===----------------------------------------------------------------------===//
// Helpful Algorithms
//===----------------------------------------------------------------------===//

/// count - Return the number of non-overlapped occurrences of \arg Str in
/// the string.
size_t StringRef::গুনতি(StringRef Str) ধ্রুৱ {
  size_t গুনতি_2 = 0;
  size_t Pos = 0;
  size_t N = Str.size();
  // TODO: For an empty `Str` we return 0 for legacy reasons. Consider changing
  //       this to `Length + 1` which is more in-line with the function
  //       description.
  যদি (!N)
    ঘূৰাই 0;
  যেতিয়ালৈ ((Pos = find(Str, Pos)) != npos) {
    ++গুনতি_2;
    Pos += N;
  }
  ঘূৰাই গুনতি_2;
}

unsigned llvm::getAutoSenseRadix(StringRef &Str) {
  যদি (Str.empty())
    ঘূৰাই 10;

  যদি (Str.consume_front_insensitive("0x"))
    ঘূৰাই 16;

  যদি (Str.consume_front_insensitive("0b"))
    ঘূৰাই 2;

  যদি (Str.consume_front("0o"))
    ঘূৰাই 8;

  যদি (Str[0] == '0' && Str.size() > 1 && isDigit(Str[1])) {
    Str = Str.substr(1);
    ঘূৰাই 8;
  }

  ঘূৰাই 10;
}

তার্কিক llvm::consumeUnsignedInteger(StringRef &Str, unsigned Radix,
                                  unsigned long long &Result) {
  // Autosense radix if not specified.
  যদি (Radix == 0)
    Radix = getAutoSenseRadix(Str);

  // Empty strings (after the radix autosense) are invalid.
  যদি (Str.empty()) ঘূৰাই সঁচা;

  // Parse all the bytes of the string given this radix.  Watch for overflow.
  StringRef Str2 = Str;
  Result = 0;
  যেতিয়ালৈ (!Str2.empty()) {
    unsigned CharVal;
    যদি (Str2[0] >= '0' && Str2[0] <= '9')
      CharVal = Str2[0] - '0';
    অন্যথা যদি (Str2[0] >= 'a' && Str2[0] <= 'z')
      CharVal = Str2[0] - 'a' + 10;
    অন্যথা যদি (Str2[0] >= 'A' && Str2[0] <= 'Z')
      CharVal = Str2[0] - 'A' + 10;
    অন্যথা
      ভঙ্গ;

    // If the parsed value is larger than the integer radix, we cannot
    // consume any more characters.
    যদি (CharVal >= Radix)
      ভঙ্গ;

    // Add in this character.
    unsigned long long PrevResult = Result;
    Result = Result * Radix + CharVal;

    // Check for overflow by shifting back and seeing if bits were lost.
    যদি (Result / Radix < PrevResult)
      ঘূৰাই সঁচা;

    Str2 = Str2.substr(1);
  }

  // We consider the operation a failure if no characters were consumed
  // successfully.
  যদি (Str.size() == Str2.size())
    ঘূৰাই সঁচা;

  Str = Str2;
  ঘূৰাই মিছা;
}

তার্কিক llvm::consumeSignedInteger(StringRef &Str, unsigned Radix,
                                long long &Result) {
  unsigned long long ULLVal;

  // Handle positive strings first.
  যদি (!Str.starts_with("-")) {
    যদি (consumeUnsignedInteger(Str, Radix, ULLVal) ||
        // Check for value so large it overflows a signed value.
        (long long)ULLVal < 0)
      ঘূৰাই সঁচা;
    Result = ULLVal;
    ঘূৰাই মিছা;
  }

  // Get the positive part of the value.
  StringRef Str2 = Str.drop_front(1);
  যদি (consumeUnsignedInteger(Str2, Radix, ULLVal) ||
      // Reject values so large they'd overflow as negative signed, but allow
      // "-0".  This negates the unsigned so that the negative isn't undefined
      // on signed overflow.
      (long long)-ULLVal > 0)
    ঘূৰাই সঁচা;

  Str = Str2;
  Result = -ULLVal;
  ঘূৰাই মিছা;
}

/// GetAsUnsignedInteger - Workhorse method that converts a integer character
/// sequence of radix up to 36 to an unsigned long long value.
তার্কিক llvm::getAsUnsignedInteger(StringRef Str, unsigned Radix,
                                unsigned long long &Result) {
  যদি (consumeUnsignedInteger(Str, Radix, Result))
    ঘূৰাই সঁচা;

  // For getAsUnsignedInteger, we require the whole string to be consumed or
  // else we consider it a failure.
  ঘূৰাই !Str.empty();
}

তার্কিক llvm::getAsSignedInteger(StringRef Str, unsigned Radix,
                              long long &Result) {
  যদি (consumeSignedInteger(Str, Radix, Result))
    ঘূৰাই সঁচা;

  // For getAsSignedInteger, we require the whole string to be consumed or else
  // we consider it a failure.
  ঘূৰাই !Str.empty();
}

তার্কিক StringRef::consumeInteger(unsigned Radix, APInt &Result) {
  StringRef Str = *এতত্;

  // Autosense radix if not specified.
  যদি (Radix == 0)
    Radix = getAutoSenseRadix(Str);

  প্রতিজ্ঞা(Radix > 1 && Radix <= 36);

  // Empty strings (after the radix autosense) are invalid.
  যদি (Str.empty()) ঘূৰাই সঁচা;

  // Skip leading zeroes.  This can be a significant improvement if
  // it means we don't need > 64 bits.
  Str = Str.ltrim('0');

  // If it was nothing but zeroes....
  যদি (Str.empty()) {
    Result = APInt(64, 0);
    *এতত্ = Str;
    ঘূৰাই মিছা;
  }

  unsigned Log2Radix = 0;
  যেতিয়ালৈ ((1U << Log2Radix) < Radix) Log2Radix++;
  তার্কিক IsPowerOf2Radix = ((1U << Log2Radix) == Radix);

  // Initialize Result to a reasonable starting width (at least 64 bits),
  // but do not shrink it if it was already larger.
  unsigned BitWidth = std::max(64U, Result.getBitWidth());

  যদি (Result.getBitWidth() < BitWidth) {
    Result = Result.zext(BitWidth);
  }

  APInt RadixAP, CharAP; // unused unless !IsPowerOf2Radix
  যদি (!IsPowerOf2Radix) {
    // These must have the same bit-width as Result.
    RadixAP = APInt(BitWidth, Radix);
    CharAP = APInt(BitWidth, 0);
  }

  // Parse all the bytes of the string given this radix.
  Result = 0;
  যেতিয়ালৈ (!Str.empty()) {
    unsigned CharVal;
    যদি (Str[0] >= '0' && Str[0] <= '9')
      CharVal = Str[0]-'0';
    অন্যথা যদি (Str[0] >= 'a' && Str[0] <= 'z')
      CharVal = Str[0]-'a'+10;
    অন্যথা যদি (Str[0] >= 'A' && Str[0] <= 'Z')
      CharVal = Str[0]-'A'+10;
    অন্যথা
      ভঙ্গ;

    // If the parsed value is larger than the integer radix, the string is
    // invalid.
    যদি (CharVal >= Radix)
      ভঙ্গ;

    // Check if one more digit will overflow, and grow if so.
    যদি (Result.getActiveBits() + Log2Radix > Result.getBitWidth()) {
      unsigned NewWidth = Result.getBitWidth() * 2;

      // Guard against overflow of the NewWidth itself
      যদি (NewWidth < Result.getBitWidth())
        ঘূৰাই সঁচা;

      Result = Result.zext(NewWidth);
      যদি (!IsPowerOf2Radix) {
        RadixAP = RadixAP.zext(NewWidth);
        CharAP = CharAP.zext(NewWidth);
      }
    }

    // Add in this character.
    যদি (IsPowerOf2Radix) {
      Result <<= Log2Radix;
      Result |= CharVal;
    } অন্যথা {
      Result *= RadixAP;
      CharAP = CharVal;
      Result += CharAP;
    }

    Str = Str.substr(1);
  }

  // We consider the operation a failure if no characters were consumed
  // successfully.
  যদি (size() == Str.size())
    ঘূৰাই সঁচা;

  *এতত্ = Str;
  ঘূৰাই মিছা;
}

তার্কিক StringRef::getAsInteger(unsigned Radix, APInt &Result) ধ্রুৱ {
  StringRef Str = *এতত্;
  যদি (Str.consumeInteger(Radix, Result))
    ঘূৰাই সঁচা;

  // For getAsInteger, we require the whole string to be consumed or else we
  // consider it a failure.
  ঘূৰাই !Str.empty();
}

তার্কিক StringRef::getAsDouble(দ্ৱিগুণ &Result, তার্কিক AllowInexact) ধ্রুৱ {
  APFloat F(0.0);
  auto StatusOrErr = F.convertFromString(*এতত্, APFloat::rmNearestTiesToEven);
  যদি (errorToBool(StatusOrErr.takeError()))
    ঘূৰাই সঁচা;

  APFloat::opStatus Status = *StatusOrErr;
  যদি (Status != APFloat::opOK) {
    যদি (!AllowInexact || !(Status & APFloat::opInexact))
      ঘূৰাই সঁচা;
  }

  Result = F.convertToDouble();
  ঘূৰাই মিছা;
}

// Implementation of StringRef hashing.
hash_code llvm::hash_মান(StringRef S) { ঘূৰাই hash_combine_range(S); }

unsigned DenseMapInfo<StringRef, শূন্য>::get_Hash_মান(StringRef Val) {
  ঘূৰাই (unsigned)(hash_মান(Val));
}
