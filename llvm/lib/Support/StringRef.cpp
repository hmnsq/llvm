//===-- StringRef.cpp - Lightweight String References ---------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#അന്തര്ഭാവ "llvm/ADT/StringRef.h"
#അന്തര്ഭാവ "llvm/ADT/APFloat.h"
#അന്തര്ഭാവ "llvm/ADT/APInt.h"
#അന്തര്ഭാവ "llvm/ADT/Hashing.h"
#അന്തര്ഭാവ "llvm/ADT/StringExtras.h"
#അന്തര്ഭാവ "llvm/ADT/edit_distance.h"
#അന്തര്ഭാവ "llvm/Support/Error.h"
#അന്തര്ഭാവ <bitset>

using നാമാകാശ llvm;

// strncasecmp() is not available on non-POSIX systems, so define an
// alternative function here.
സ്ഥിതം പൂർണ്ണസംഖ്യ ascii_strncasecmp(StringRef LHS, StringRef RHS) {
  വേണ്ടി (auto [LC, RC] : zip_equal(LHS, RHS)) {
    unsigned അക്ഷരം LHC = toLower(LC);
    unsigned അക്ഷരം RHC = toLower(RC);
    എങ്കിൽ (LHC != RHC)
      തിരികെ LHC < RHC ? -1 : 1;
  }
  തിരികെ 0;
}

പൂർണ്ണസംഖ്യ StringRef::compare_insensitive(StringRef RHS) സ്ഥിരം {
  size_t Min = std::min(size(), RHS.size());
  എങ്കിൽ (പൂർണ്ണസംഖ്യ Res = ascii_strncasecmp(take_front(Min), RHS.take_front(Min)))
    തിരികെ Res;
  എങ്കിൽ (size() == RHS.size())
    തിരികെ 0;
  തിരികെ size() < RHS.size() ? -1 : 1;
}

തർക്കം StringRef::starts_with_insensitive(StringRef Prefix) സ്ഥിരം {
  തിരികെ size() >= Prefix.size() &&
         ascii_strncasecmp(take_front(Prefix.size()), Prefix) == 0;
}

തർക്കം StringRef::ends_with_insensitive(StringRef Suffix) സ്ഥിരം {
  തിരികെ size() >= Suffix.size() &&
         ascii_strncasecmp(take_back(Suffix.size()), Suffix) == 0;
}

size_t StringRef::find_insensitive(അക്ഷരം C, size_t From) സ്ഥിരം {
  അക്ഷരം L = toLower(C);
  തിരികെ find_if([L](അക്ഷരം D) { തിരികെ toLower(D) == L; }, From);
}

/// compare_numeric - Compare strings, handle embedded numbers.
പൂർണ്ണസംഖ്യ StringRef::compare_numeric(StringRef RHS) സ്ഥിരം {
  വേണ്ടി (size_t I = 0, E = std::min(size(), RHS.size()); I != E; ++I) {
    // Check for sequences of digits.
    എങ്കിൽ (isDigit(data()[I]) && isDigit(RHS.data()[I])) {
      // The longer sequence of numbers is considered larger.
      // This doesn't really handle prefixed zeros well.
      size_t J;
      വേണ്ടി (J = I + 1; J != E + 1; ++J) {
        തർക്കം ld = J < size() && isDigit(data()[J]);
        തർക്കം rd = J < RHS.size() && isDigit(RHS.data()[J]);
        എങ്കിൽ (ld != rd)
          തിരികെ rd ? -1 : 1;
        എങ്കിൽ (!rd)
          നിർത്തുക;
      }
      // The two number sequences have the same length (J-I), just memcmp them.
      എങ്കിൽ (പൂർണ്ണസംഖ്യ Res = compareMemory(data() + I, RHS.data() + I, J - I))
        തിരികെ Res < 0 ? -1 : 1;
      // Identical number sequences, continue search after the numbers.
      I = J - 1;
      തുടരുക;
    }
    എങ്കിൽ (data()[I] != RHS.data()[I])
      തിരികെ (unsigned അക്ഷരം)data()[I] < (unsigned അക്ഷരം)RHS.data()[I] ? -1 : 1;
  }
  എങ്കിൽ (size() == RHS.size())
    തിരികെ 0;
  തിരികെ size() < RHS.size() ? -1 : 1;
}

// Compute the edit distance between the two given strings.
unsigned StringRef::edit_distance(llvm::StringRef Other,
                                  തർക്കം AllowReplacements,
                                  unsigned MaxEditDistance) സ്ഥിരം {
  തിരികെ llvm::ComputeEditDistance(ArrayRef(data(), size()),
                                   ArrayRef(Other.data(), Other.size()),
                                   AllowReplacements, MaxEditDistance);
}

unsigned llvm::StringRef::edit_distance_insensitive(
    StringRef Other, തർക്കം AllowReplacements, unsigned MaxEditDistance) സ്ഥിരം {
  തിരികെ llvm::ComputeMappedEditDistance(
      ArrayRef(data(), size()), ArrayRef(Other.data(), Other.size()),
      llvm::toLower, AllowReplacements, MaxEditDistance);
}

//===----------------------------------------------------------------------===//
// String Operations
//===----------------------------------------------------------------------===//

std::string StringRef::lower() സ്ഥിരം {
  തിരികെ std::string(map_iterator(begin(), toLower),
                     map_iterator(end(), toLower));
}

std::string StringRef::upper() സ്ഥിരം {
  തിരികെ std::string(map_iterator(begin(), toUpper),
                     map_iterator(end(), toUpper));
}

//===----------------------------------------------------------------------===//
// String Searching
//===----------------------------------------------------------------------===//


/// find - Search for the first string \arg Str in the string.
///
/// \return - The index of the first occurrence of \arg Str, or npos if not
/// found.
size_t StringRef::find(StringRef Str, size_t From) സ്ഥിരം {
  എങ്കിൽ (From > size())
    തിരികെ npos;

  സ്ഥിരം അക്ഷരം *Start = data() + From;
  size_t Size = size() - From;

  സ്ഥിരം അക്ഷരം *Needle = Str.data();
  size_t N = Str.size();
  എങ്കിൽ (N == 0)
    തിരികെ From;
  എങ്കിൽ (Size < N)
    തിരികെ npos;
  എങ്കിൽ (N == 1) {
    സ്ഥിരം അക്ഷരം *Ptr = (സ്ഥിരം അക്ഷരം *)::memchr(Start, Needle[0], Size);
    തിരികെ Ptr == nullptr ? npos : Ptr - data();
  }

  സ്ഥിരം അക്ഷരം *Stop = Start + (Size - N + 1);

  എങ്കിൽ (N == 2) {
    // Provide a fast path for newline finding (CRLF case) in InclusionRewriter.
    // Not the most optimized strategy, but getting memcmp inlined should be
    // good enough.
    ചെയ്യുക {
      എങ്കിൽ (std::memcmp(Start, Needle, 2) == 0)
        തിരികെ Start - data();
      ++Start;
    } ആകുമ്പോൾ (Start < Stop);
    തിരികെ npos;
  }

  // For short haystacks or unsupported needles fall back to the naive algorithm
  എങ്കിൽ (Size < 16 || N > 255) {
    ചെയ്യുക {
      എങ്കിൽ (std::memcmp(Start, Needle, N) == 0)
        തിരികെ Start - data();
      ++Start;
    } ആകുമ്പോൾ (Start < Stop);
    തിരികെ npos;
  }

  // Build the bad char heuristic table, with uint8_t to reduce cache thrashing.
  uint8_t BadCharSkip[256];
  std::memset(BadCharSkip, N, 256);
  വേണ്ടി (unsigned i = 0; i != N-1; ++i)
    BadCharSkip[(uint8_t)Str[i]] = N-1-i;

  ചെയ്യുക {
    uint8_t Last = Start[N - 1];
    എങ്കിൽ (LLVM_UNLIKELY(Last == (uint8_t)Needle[N - 1]))
      എങ്കിൽ (std::memcmp(Start, Needle, N - 1) == 0)
        തിരികെ Start - data();

    // Otherwise skip the appropriate number of bytes.
    Start += BadCharSkip[Last];
  } ആകുമ്പോൾ (Start < Stop);

  തിരികെ npos;
}

size_t StringRef::find_insensitive(StringRef Str, size_t From) സ്ഥിരം {
  StringRef This = substr(From);
  ആകുമ്പോൾ (This.size() >= Str.size()) {
    എങ്കിൽ (This.starts_with_insensitive(Str))
      തിരികെ From;
    This = This.drop_front();
    ++From;
  }
  തിരികെ npos;
}

size_t StringRef::rfind_insensitive(അക്ഷരം C, size_t From) സ്ഥിരം {
  From = std::min(From, size());
  size_t i = From;
  ആകുമ്പോൾ (i != 0) {
    --i;
    എങ്കിൽ (toLower(data()[i]) == toLower(C))
      തിരികെ i;
  }
  തിരികെ npos;
}

/// rfind - Search for the last string \arg Str in the string.
///
/// \return - The index of the last occurrence of \arg Str, or npos if not
/// found.
size_t StringRef::rfind(StringRef Str) സ്ഥിരം {
  തിരികെ std::string_view(*ഇത്).rfind(Str);
}

size_t StringRef::rfind_insensitive(StringRef Str) സ്ഥിരം {
  size_t N = Str.size();
  എങ്കിൽ (N > size())
    തിരികെ npos;
  വേണ്ടി (size_t i = size() - N + 1, e = 0; i != e;) {
    --i;
    എങ്കിൽ (substr(i, N).equals_insensitive(Str))
      തിരികെ i;
  }
  തിരികെ npos;
}

/// find_first_of - Find the first character in the string that is in \arg
/// Chars, or npos if not found.
///
/// Note: O(size() + Chars.size())
StringRef::size_type StringRef::find_first_of(StringRef Chars,
                                              size_t From) സ്ഥിരം {
  std::bitset<1 << CHAR_BIT> CharBits;
  വേണ്ടി (അക്ഷരം C : Chars)
    CharBits.set((unsigned അക്ഷരം)C);

  വേണ്ടി (size_type i = std::min(From, size()), e = size(); i != e; ++i)
    എങ്കിൽ (CharBits.test((unsigned അക്ഷരം)data()[i]))
      തിരികെ i;
  തിരികെ npos;
}

/// find_first_not_of - Find the first character in the string that is not
/// \arg C or npos if not found.
StringRef::size_type StringRef::find_first_not_of(അക്ഷരം C, size_t From) സ്ഥിരം {
  തിരികെ std::string_view(*ഇത്).find_first_not_of(C, From);
}

/// find_first_not_of - Find the first character in the string that is not
/// in the string \arg Chars, or npos if not found.
///
/// Note: O(size() + Chars.size())
StringRef::size_type StringRef::find_first_not_of(StringRef Chars,
                                                  size_t From) സ്ഥിരം {
  std::bitset<1 << CHAR_BIT> CharBits;
  വേണ്ടി (അക്ഷരം C : Chars)
    CharBits.set((unsigned അക്ഷരം)C);

  വേണ്ടി (size_type i = std::min(From, size()), e = size(); i != e; ++i)
    എങ്കിൽ (!CharBits.test((unsigned അക്ഷരം)data()[i]))
      തിരികെ i;
  തിരികെ npos;
}

/// find_last_of - Find the last character in the string that is in \arg C,
/// or npos if not found.
///
/// Note: O(size() + Chars.size())
StringRef::size_type StringRef::find_last_of(StringRef Chars,
                                             size_t From) സ്ഥിരം {
  std::bitset<1 << CHAR_BIT> CharBits;
  വേണ്ടി (അക്ഷരം C : Chars)
    CharBits.set((unsigned അക്ഷരം)C);

  വേണ്ടി (size_type i = std::min(From, size()) - 1, e = -1; i != e; --i)
    എങ്കിൽ (CharBits.test((unsigned അക്ഷരം)data()[i]))
      തിരികെ i;
  തിരികെ npos;
}

/// find_last_not_of - Find the last character in the string that is not
/// \arg C, or npos if not found.
StringRef::size_type StringRef::find_last_not_of(അക്ഷരം C, size_t From) സ്ഥിരം {
  വേണ്ടി (size_type i = std::min(From, size()) - 1, e = -1; i != e; --i)
    എങ്കിൽ (data()[i] != C)
      തിരികെ i;
  തിരികെ npos;
}

/// find_last_not_of - Find the last character in the string that is not in
/// \arg Chars, or npos if not found.
///
/// Note: O(size() + Chars.size())
StringRef::size_type StringRef::find_last_not_of(StringRef Chars,
                                                 size_t From) സ്ഥിരം {
  std::bitset<1 << CHAR_BIT> CharBits;
  വേണ്ടി (അക്ഷരം C : Chars)
    CharBits.set((unsigned അക്ഷരം)C);

  വേണ്ടി (size_type i = std::min(From, size()) - 1, e = -1; i != e; --i)
    എങ്കിൽ (!CharBits.test((unsigned അക്ഷരം)data()[i]))
      തിരികെ i;
  തിരികെ npos;
}

ശൂന്യം StringRef::split(SmallVectorImpl<StringRef> &A,
                      StringRef Separator, പൂർണ്ണസംഖ്യ MaxSplit,
                      തർക്കം KeepEmpty) സ്ഥിരം {
  StringRef S = *ഇത്;

  // Count down from MaxSplit. When MaxSplit is -1, this will just split
  // "forever". This doesn't support splitting more than 2^31 times
  // intentionally; if we ever want that we can make MaxSplit a 64-bit integer
  // but that seems unlikely to be useful.
  ആകുമ്പോൾ (MaxSplit-- != 0) {
    size_t Idx = S.find(Separator);
    എങ്കിൽ (Idx == npos)
      നിർത്തുക;

    // Push this split.
    എങ്കിൽ (KeepEmpty || Idx > 0)
      A.തള്ളുക_back(S.slice(0, Idx));

    // Jump forward.
    S = S.substr(Idx + Separator.size());
  }

  // Push the tail.
  എങ്കിൽ (KeepEmpty || !S.empty())
    A.തള്ളുക_back(S);
}

ശൂന്യം StringRef::split(SmallVectorImpl<StringRef> &A, അക്ഷരം Separator,
                      പൂർണ്ണസംഖ്യ MaxSplit, തർക്കം KeepEmpty) സ്ഥിരം {
  StringRef S = *ഇത്;

  // Count down from MaxSplit. When MaxSplit is -1, this will just split
  // "forever". This doesn't support splitting more than 2^31 times
  // intentionally; if we ever want that we can make MaxSplit a 64-bit integer
  // but that seems unlikely to be useful.
  ആകുമ്പോൾ (MaxSplit-- != 0) {
    size_t Idx = S.find(Separator);
    എങ്കിൽ (Idx == npos)
      നിർത്തുക;

    // Push this split.
    എങ്കിൽ (KeepEmpty || Idx > 0)
      A.തള്ളുക_back(S.slice(0, Idx));

    // Jump forward.
    S = S.substr(Idx + 1);
  }

  // Push the tail.
  എങ്കിൽ (KeepEmpty || !S.empty())
    A.തള്ളുക_back(S);
}

//===----------------------------------------------------------------------===//
// Helpful Algorithms
//===----------------------------------------------------------------------===//

/// count - Return the number of non-overlapped occurrences of \arg Str in
/// the string.
size_t StringRef::എണ്ണം(StringRef Str) സ്ഥിരം {
  size_t എണ്ണം_2 = 0;
  size_t Pos = 0;
  size_t N = Str.size();
  // TODO: For an empty `Str` we return 0 for legacy reasons. Consider changing
  //       this to `Length + 1` which is more in-line with the function
  //       description.
  എങ്കിൽ (!N)
    തിരികെ 0;
  ആകുമ്പോൾ ((Pos = find(Str, Pos)) != npos) {
    ++എണ്ണം_2;
    Pos += N;
  }
  തിരികെ എണ്ണം_2;
}

unsigned llvm::getAutoSenseRadix(StringRef &Str) {
  എങ്കിൽ (Str.empty())
    തിരികെ 10;

  എങ്കിൽ (Str.consume_front_insensitive("0x"))
    തിരികെ 16;

  എങ്കിൽ (Str.consume_front_insensitive("0b"))
    തിരികെ 2;

  എങ്കിൽ (Str.consume_front("0o"))
    തിരികെ 8;

  എങ്കിൽ (Str[0] == '0' && Str.size() > 1 && isDigit(Str[1])) {
    Str = Str.substr(1);
    തിരികെ 8;
  }

  തിരികെ 10;
}

തർക്കം llvm::consumeUnsignedInteger(StringRef &Str, unsigned Radix,
                                  unsigned long long &Result) {
  // Autosense radix if not specified.
  എങ്കിൽ (Radix == 0)
    Radix = getAutoSenseRadix(Str);

  // Empty strings (after the radix autosense) are invalid.
  എങ്കിൽ (Str.empty()) തിരികെ സത്യം;

  // Parse all the bytes of the string given this radix.  Watch for overflow.
  StringRef Str2 = Str;
  Result = 0;
  ആകുമ്പോൾ (!Str2.empty()) {
    unsigned CharVal;
    എങ്കിൽ (Str2[0] >= '0' && Str2[0] <= '9')
      CharVal = Str2[0] - '0';
    അല്ലെങ്കിൽ എങ്കിൽ (Str2[0] >= 'a' && Str2[0] <= 'z')
      CharVal = Str2[0] - 'a' + 10;
    അല്ലെങ്കിൽ എങ്കിൽ (Str2[0] >= 'A' && Str2[0] <= 'Z')
      CharVal = Str2[0] - 'A' + 10;
    അല്ലെങ്കിൽ
      നിർത്തുക;

    // If the parsed value is larger than the integer radix, we cannot
    // consume any more characters.
    എങ്കിൽ (CharVal >= Radix)
      നിർത്തുക;

    // Add in this character.
    unsigned long long PrevResult = Result;
    Result = Result * Radix + CharVal;

    // Check for overflow by shifting back and seeing if bits were lost.
    എങ്കിൽ (Result / Radix < PrevResult)
      തിരികെ സത്യം;

    Str2 = Str2.substr(1);
  }

  // We consider the operation a failure if no characters were consumed
  // successfully.
  എങ്കിൽ (Str.size() == Str2.size())
    തിരികെ സത്യം;

  Str = Str2;
  തിരികെ അസത്യം;
}

തർക്കം llvm::consumeSignedInteger(StringRef &Str, unsigned Radix,
                                long long &Result) {
  unsigned long long ULLVal;

  // Handle positive strings first.
  എങ്കിൽ (!Str.starts_with("-")) {
    എങ്കിൽ (consumeUnsignedInteger(Str, Radix, ULLVal) ||
        // Check for value so large it overflows a signed value.
        (long long)ULLVal < 0)
      തിരികെ സത്യം;
    Result = ULLVal;
    തിരികെ അസത്യം;
  }

  // Get the positive part of the value.
  StringRef Str2 = Str.drop_front(1);
  എങ്കിൽ (consumeUnsignedInteger(Str2, Radix, ULLVal) ||
      // Reject values so large they'd overflow as negative signed, but allow
      // "-0".  This negates the unsigned so that the negative isn't undefined
      // on signed overflow.
      (long long)-ULLVal > 0)
    തിരികെ സത്യം;

  Str = Str2;
  Result = -ULLVal;
  തിരികെ അസത്യം;
}

/// GetAsUnsignedInteger - Workhorse method that converts a integer character
/// sequence of radix up to 36 to an unsigned long long value.
തർക്കം llvm::getAsUnsignedInteger(StringRef Str, unsigned Radix,
                                unsigned long long &Result) {
  എങ്കിൽ (consumeUnsignedInteger(Str, Radix, Result))
    തിരികെ സത്യം;

  // For getAsUnsignedInteger, we require the whole string to be consumed or
  // else we consider it a failure.
  തിരികെ !Str.empty();
}

തർക്കം llvm::getAsSignedInteger(StringRef Str, unsigned Radix,
                              long long &Result) {
  എങ്കിൽ (consumeSignedInteger(Str, Radix, Result))
    തിരികെ സത്യം;

  // For getAsSignedInteger, we require the whole string to be consumed or else
  // we consider it a failure.
  തിരികെ !Str.empty();
}

തർക്കം StringRef::consumeInteger(unsigned Radix, APInt &Result) {
  StringRef Str = *ഇത്;

  // Autosense radix if not specified.
  എങ്കിൽ (Radix == 0)
    Radix = getAutoSenseRadix(Str);

  പ്രതിജ്ഞാ(Radix > 1 && Radix <= 36);

  // Empty strings (after the radix autosense) are invalid.
  എങ്കിൽ (Str.empty()) തിരികെ സത്യം;

  // Skip leading zeroes.  This can be a significant improvement if
  // it means we don't need > 64 bits.
  Str = Str.ltrim('0');

  // If it was nothing but zeroes....
  എങ്കിൽ (Str.empty()) {
    Result = APInt(64, 0);
    *ഇത് = Str;
    തിരികെ അസത്യം;
  }

  unsigned Log2Radix = 0;
  ആകുമ്പോൾ ((1U << Log2Radix) < Radix) Log2Radix++;
  തർക്കം IsPowerOf2Radix = ((1U << Log2Radix) == Radix);

  // Initialize Result to a reasonable starting width (at least 64 bits),
  // but do not shrink it if it was already larger.
  unsigned BitWidth = std::max(64U, Result.getBitWidth());

  എങ്കിൽ (Result.getBitWidth() < BitWidth) {
    Result = Result.zext(BitWidth);
  }

  APInt RadixAP, CharAP; // unused unless !IsPowerOf2Radix
  എങ്കിൽ (!IsPowerOf2Radix) {
    // These must have the same bit-width as Result.
    RadixAP = APInt(BitWidth, Radix);
    CharAP = APInt(BitWidth, 0);
  }

  // Parse all the bytes of the string given this radix.
  Result = 0;
  ആകുമ്പോൾ (!Str.empty()) {
    unsigned CharVal;
    എങ്കിൽ (Str[0] >= '0' && Str[0] <= '9')
      CharVal = Str[0]-'0';
    അല്ലെങ്കിൽ എങ്കിൽ (Str[0] >= 'a' && Str[0] <= 'z')
      CharVal = Str[0]-'a'+10;
    അല്ലെങ്കിൽ എങ്കിൽ (Str[0] >= 'A' && Str[0] <= 'Z')
      CharVal = Str[0]-'A'+10;
    അല്ലെങ്കിൽ
      നിർത്തുക;

    // If the parsed value is larger than the integer radix, the string is
    // invalid.
    എങ്കിൽ (CharVal >= Radix)
      നിർത്തുക;

    // Check if one more digit will overflow, and grow if so.
    എങ്കിൽ (Result.getActiveBits() + Log2Radix > Result.getBitWidth()) {
      unsigned NewWidth = Result.getBitWidth() * 2;

      // Guard against overflow of the NewWidth itself
      എങ്കിൽ (NewWidth < Result.getBitWidth())
        തിരികെ സത്യം;

      Result = Result.zext(NewWidth);
      എങ്കിൽ (!IsPowerOf2Radix) {
        RadixAP = RadixAP.zext(NewWidth);
        CharAP = CharAP.zext(NewWidth);
      }
    }

    // Add in this character.
    എങ്കിൽ (IsPowerOf2Radix) {
      Result <<= Log2Radix;
      Result |= CharVal;
    } അല്ലെങ്കിൽ {
      Result *= RadixAP;
      CharAP = CharVal;
      Result += CharAP;
    }

    Str = Str.substr(1);
  }

  // We consider the operation a failure if no characters were consumed
  // successfully.
  എങ്കിൽ (size() == Str.size())
    തിരികെ സത്യം;

  *ഇത് = Str;
  തിരികെ അസത്യം;
}

തർക്കം StringRef::getAsInteger(unsigned Radix, APInt &Result) സ്ഥിരം {
  StringRef Str = *ഇത്;
  എങ്കിൽ (Str.consumeInteger(Radix, Result))
    തിരികെ സത്യം;

  // For getAsInteger, we require the whole string to be consumed or else we
  // consider it a failure.
  തിരികെ !Str.empty();
}

തർക്കം StringRef::getAsDouble(ഇരട്ട &Result, തർക്കം AllowInexact) സ്ഥിരം {
  APFloat F(0.0);
  auto StatusOrErr = F.convertFromString(*ഇത്, APFloat::rmNearestTiesToEven);
  എങ്കിൽ (errorToBool(StatusOrErr.takeError()))
    തിരികെ സത്യം;

  APFloat::opStatus Status = *StatusOrErr;
  എങ്കിൽ (Status != APFloat::opOK) {
    എങ്കിൽ (!AllowInexact || !(Status & APFloat::opInexact))
      തിരികെ സത്യം;
  }

  Result = F.convertToDouble();
  തിരികെ അസത്യം;
}

// Implementation of StringRef hashing.
hash_code llvm::hash_മൂല്യം(StringRef S) { തിരികെ hash_combine_range(S); }

unsigned DenseMapInfo<StringRef, ശൂന്യം>::get_Hash_മൂല്യം(StringRef Val) {
  തിരികെ (unsigned)(hash_മൂല്യം(Val));
}
