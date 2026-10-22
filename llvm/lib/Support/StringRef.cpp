//===-- StringRef.cpp - Lightweight String References ---------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#அந்தர்பாவ "llvm/ADT/StringRef.h"
#அந்தர்பாவ "llvm/ADT/APFloat.h"
#அந்தர்பாவ "llvm/ADT/APInt.h"
#அந்தர்பாவ "llvm/ADT/Hashing.h"
#அந்தர்பாவ "llvm/ADT/StringExtras.h"
#அந்தர்பாவ "llvm/ADT/edit_distance.h"
#அந்தர்பாவ "llvm/Support/Error.h"
#அந்தர்பாவ <bitset>

using நாமாகாஶ llvm;

// strncasecmp() is not available on non-POSIX systems, so define an
// alternative function here.
நிலையான முழு ascii_strncasecmp(StringRef LHS, StringRef RHS) {
  க்கு (auto [LC, RC] : zip_equal(LHS, RHS)) {
    unsigned எழுத்து LHC = toLower(LC);
    unsigned எழுத்து RHC = toLower(RC);
    எனில் (LHC != RHC)
      திரும்பு LHC < RHC ? -1 : 1;
  }
  திரும்பு 0;
}

முழு StringRef::compare_insensitive(StringRef RHS) மாறிலி {
  size_t Min = std::min(size(), RHS.size());
  எனில் (முழு Res = ascii_strncasecmp(take_front(Min), RHS.take_front(Min)))
    திரும்பு Res;
  எனில் (size() == RHS.size())
    திரும்பு 0;
  திரும்பு size() < RHS.size() ? -1 : 1;
}

தருக்க StringRef::starts_with_insensitive(StringRef Prefix) மாறிலி {
  திரும்பு size() >= Prefix.size() &&
         ascii_strncasecmp(take_front(Prefix.size()), Prefix) == 0;
}

தருக்க StringRef::ends_with_insensitive(StringRef Suffix) மாறிலி {
  திரும்பு size() >= Suffix.size() &&
         ascii_strncasecmp(take_back(Suffix.size()), Suffix) == 0;
}

size_t StringRef::find_insensitive(எழுத்து C, size_t From) மாறிலி {
  எழுத்து L = toLower(C);
  திரும்பு find_if([L](எழுத்து D) { திரும்பு toLower(D) == L; }, From);
}

/// compare_numeric - Compare strings, handle embedded numbers.
முழு StringRef::compare_numeric(StringRef RHS) மாறிலி {
  க்கு (size_t I = 0, E = std::min(size(), RHS.size()); I != E; ++I) {
    // Check for sequences of digits.
    எனில் (isDigit(data()[I]) && isDigit(RHS.data()[I])) {
      // The longer sequence of numbers is considered larger.
      // This doesn't really handle prefixed zeros well.
      size_t J;
      க்கு (J = I + 1; J != E + 1; ++J) {
        தருக்க ld = J < size() && isDigit(data()[J]);
        தருக்க rd = J < RHS.size() && isDigit(RHS.data()[J]);
        எனில் (ld != rd)
          திரும்பு rd ? -1 : 1;
        எனில் (!rd)
          நிறுத்து;
      }
      // The two number sequences have the same length (J-I), just memcmp them.
      எனில் (முழு Res = compareMemory(data() + I, RHS.data() + I, J - I))
        திரும்பு Res < 0 ? -1 : 1;
      // Identical number sequences, continue search after the numbers.
      I = J - 1;
      தொடர்;
    }
    எனில் (data()[I] != RHS.data()[I])
      திரும்பு (unsigned எழுத்து)data()[I] < (unsigned எழுத்து)RHS.data()[I] ? -1 : 1;
  }
  எனில் (size() == RHS.size())
    திரும்பு 0;
  திரும்பு size() < RHS.size() ? -1 : 1;
}

// Compute the edit distance between the two given strings.
unsigned StringRef::edit_distance(llvm::StringRef Other,
                                  தருக்க AllowReplacements,
                                  unsigned MaxEditDistance) மாறிலி {
  திரும்பு llvm::ComputeEditDistance(ArrayRef(data(), size()),
                                   ArrayRef(Other.data(), Other.size()),
                                   AllowReplacements, MaxEditDistance);
}

unsigned llvm::StringRef::edit_distance_insensitive(
    StringRef Other, தருக்க AllowReplacements, unsigned MaxEditDistance) மாறிலி {
  திரும்பு llvm::ComputeMappedEditDistance(
      ArrayRef(data(), size()), ArrayRef(Other.data(), Other.size()),
      llvm::toLower, AllowReplacements, MaxEditDistance);
}

//===----------------------------------------------------------------------===//
// String Operations
//===----------------------------------------------------------------------===//

std::string StringRef::lower() மாறிலி {
  திரும்பு std::string(map_iterator(begin(), toLower),
                     map_iterator(end(), toLower));
}

std::string StringRef::upper() மாறிலி {
  திரும்பு std::string(map_iterator(begin(), toUpper),
                     map_iterator(end(), toUpper));
}

//===----------------------------------------------------------------------===//
// String Searching
//===----------------------------------------------------------------------===//


/// find - Search for the first string \arg Str in the string.
///
/// \return - The index of the first occurrence of \arg Str, or npos if not
/// found.
size_t StringRef::find(StringRef Str, size_t From) மாறிலி {
  எனில் (From > size())
    திரும்பு npos;

  மாறிலி எழுத்து *Start = data() + From;
  size_t Size = size() - From;

  மாறிலி எழுத்து *Needle = Str.data();
  size_t N = Str.size();
  எனில் (N == 0)
    திரும்பு From;
  எனில் (Size < N)
    திரும்பு npos;
  எனில் (N == 1) {
    மாறிலி எழுத்து *Ptr = (மாறிலி எழுத்து *)::memchr(Start, Needle[0], Size);
    திரும்பு Ptr == nullptr ? npos : Ptr - data();
  }

  மாறிலி எழுத்து *Stop = Start + (Size - N + 1);

  எனில் (N == 2) {
    // Provide a fast path for newline finding (CRLF case) in InclusionRewriter.
    // Not the most optimized strategy, but getting memcmp inlined should be
    // good enough.
    செய் {
      எனில் (std::memcmp(Start, Needle, 2) == 0)
        திரும்பு Start - data();
      ++Start;
    } போது (Start < Stop);
    திரும்பு npos;
  }

  // For short haystacks or unsupported needles fall back to the naive algorithm
  எனில் (Size < 16 || N > 255) {
    செய் {
      எனில் (std::memcmp(Start, Needle, N) == 0)
        திரும்பு Start - data();
      ++Start;
    } போது (Start < Stop);
    திரும்பு npos;
  }

  // Build the bad char heuristic table, with uint8_t to reduce cache thrashing.
  uint8_t BadCharSkip[256];
  std::memset(BadCharSkip, N, 256);
  க்கு (unsigned i = 0; i != N-1; ++i)
    BadCharSkip[(uint8_t)Str[i]] = N-1-i;

  செய் {
    uint8_t Last = Start[N - 1];
    எனில் (LLVM_UNLIKELY(Last == (uint8_t)Needle[N - 1]))
      எனில் (std::memcmp(Start, Needle, N - 1) == 0)
        திரும்பு Start - data();

    // Otherwise skip the appropriate number of bytes.
    Start += BadCharSkip[Last];
  } போது (Start < Stop);

  திரும்பு npos;
}

size_t StringRef::find_insensitive(StringRef Str, size_t From) மாறிலி {
  StringRef This = substr(From);
  போது (This.size() >= Str.size()) {
    எனில் (This.starts_with_insensitive(Str))
      திரும்பு From;
    This = This.drop_front();
    ++From;
  }
  திரும்பு npos;
}

size_t StringRef::rfind_insensitive(எழுத்து C, size_t From) மாறிலி {
  From = std::min(From, size());
  size_t i = From;
  போது (i != 0) {
    --i;
    எனில் (toLower(data()[i]) == toLower(C))
      திரும்பு i;
  }
  திரும்பு npos;
}

/// rfind - Search for the last string \arg Str in the string.
///
/// \return - The index of the last occurrence of \arg Str, or npos if not
/// found.
size_t StringRef::rfind(StringRef Str) மாறிலி {
  திரும்பு std::string_view(*இது).rfind(Str);
}

size_t StringRef::rfind_insensitive(StringRef Str) மாறிலி {
  size_t N = Str.size();
  எனில் (N > size())
    திரும்பு npos;
  க்கு (size_t i = size() - N + 1, e = 0; i != e;) {
    --i;
    எனில் (substr(i, N).equals_insensitive(Str))
      திரும்பு i;
  }
  திரும்பு npos;
}

/// find_first_of - Find the first character in the string that is in \arg
/// Chars, or npos if not found.
///
/// Note: O(size() + Chars.size())
StringRef::size_type StringRef::find_first_of(StringRef Chars,
                                              size_t From) மாறிலி {
  std::bitset<1 << CHAR_BIT> CharBits;
  க்கு (எழுத்து C : Chars)
    CharBits.set((unsigned எழுத்து)C);

  க்கு (size_type i = std::min(From, size()), e = size(); i != e; ++i)
    எனில் (CharBits.test((unsigned எழுத்து)data()[i]))
      திரும்பு i;
  திரும்பு npos;
}

/// find_first_not_of - Find the first character in the string that is not
/// \arg C or npos if not found.
StringRef::size_type StringRef::find_first_not_of(எழுத்து C, size_t From) மாறிலி {
  திரும்பு std::string_view(*இது).find_first_not_of(C, From);
}

/// find_first_not_of - Find the first character in the string that is not
/// in the string \arg Chars, or npos if not found.
///
/// Note: O(size() + Chars.size())
StringRef::size_type StringRef::find_first_not_of(StringRef Chars,
                                                  size_t From) மாறிலி {
  std::bitset<1 << CHAR_BIT> CharBits;
  க்கு (எழுத்து C : Chars)
    CharBits.set((unsigned எழுத்து)C);

  க்கு (size_type i = std::min(From, size()), e = size(); i != e; ++i)
    எனில் (!CharBits.test((unsigned எழுத்து)data()[i]))
      திரும்பு i;
  திரும்பு npos;
}

/// find_last_of - Find the last character in the string that is in \arg C,
/// or npos if not found.
///
/// Note: O(size() + Chars.size())
StringRef::size_type StringRef::find_last_of(StringRef Chars,
                                             size_t From) மாறிலி {
  std::bitset<1 << CHAR_BIT> CharBits;
  க்கு (எழுத்து C : Chars)
    CharBits.set((unsigned எழுத்து)C);

  க்கு (size_type i = std::min(From, size()) - 1, e = -1; i != e; --i)
    எனில் (CharBits.test((unsigned எழுத்து)data()[i]))
      திரும்பு i;
  திரும்பு npos;
}

/// find_last_not_of - Find the last character in the string that is not
/// \arg C, or npos if not found.
StringRef::size_type StringRef::find_last_not_of(எழுத்து C, size_t From) மாறிலி {
  க்கு (size_type i = std::min(From, size()) - 1, e = -1; i != e; --i)
    எனில் (data()[i] != C)
      திரும்பு i;
  திரும்பு npos;
}

/// find_last_not_of - Find the last character in the string that is not in
/// \arg Chars, or npos if not found.
///
/// Note: O(size() + Chars.size())
StringRef::size_type StringRef::find_last_not_of(StringRef Chars,
                                                 size_t From) மாறிலி {
  std::bitset<1 << CHAR_BIT> CharBits;
  க்கு (எழுத்து C : Chars)
    CharBits.set((unsigned எழுத்து)C);

  க்கு (size_type i = std::min(From, size()) - 1, e = -1; i != e; --i)
    எனில் (!CharBits.test((unsigned எழுத்து)data()[i]))
      திரும்பு i;
  திரும்பு npos;
}

வெறுமை StringRef::split(SmallVectorImpl<StringRef> &A,
                      StringRef Separator, முழு MaxSplit,
                      தருக்க KeepEmpty) மாறிலி {
  StringRef S = *இது;

  // Count down from MaxSplit. When MaxSplit is -1, this will just split
  // "forever". This doesn't support splitting more than 2^31 times
  // intentionally; if we ever want that we can make MaxSplit a 64-bit integer
  // but that seems unlikely to be useful.
  போது (MaxSplit-- != 0) {
    size_t Idx = S.find(Separator);
    எனில் (Idx == npos)
      நிறுத்து;

    // Push this split.
    எனில் (KeepEmpty || Idx > 0)
      A.தள்ளு_back(S.slice(0, Idx));

    // Jump forward.
    S = S.substr(Idx + Separator.size());
  }

  // Push the tail.
  எனில் (KeepEmpty || !S.empty())
    A.தள்ளு_back(S);
}

வெறுமை StringRef::split(SmallVectorImpl<StringRef> &A, எழுத்து Separator,
                      முழு MaxSplit, தருக்க KeepEmpty) மாறிலி {
  StringRef S = *இது;

  // Count down from MaxSplit. When MaxSplit is -1, this will just split
  // "forever". This doesn't support splitting more than 2^31 times
  // intentionally; if we ever want that we can make MaxSplit a 64-bit integer
  // but that seems unlikely to be useful.
  போது (MaxSplit-- != 0) {
    size_t Idx = S.find(Separator);
    எனில் (Idx == npos)
      நிறுத்து;

    // Push this split.
    எனில் (KeepEmpty || Idx > 0)
      A.தள்ளு_back(S.slice(0, Idx));

    // Jump forward.
    S = S.substr(Idx + 1);
  }

  // Push the tail.
  எனில் (KeepEmpty || !S.empty())
    A.தள்ளு_back(S);
}

//===----------------------------------------------------------------------===//
// Helpful Algorithms
//===----------------------------------------------------------------------===//

/// count - Return the number of non-overlapped occurrences of \arg Str in
/// the string.
size_t StringRef::மொத்தம்(StringRef Str) மாறிலி {
  size_t மொத்தம்_2 = 0;
  size_t Pos = 0;
  size_t N = Str.size();
  // TODO: For an empty `Str` we return 0 for legacy reasons. Consider changing
  //       this to `Length + 1` which is more in-line with the function
  //       description.
  எனில் (!N)
    திரும்பு 0;
  போது ((Pos = find(Str, Pos)) != npos) {
    ++மொத்தம்_2;
    Pos += N;
  }
  திரும்பு மொத்தம்_2;
}

unsigned llvm::getAutoSenseRadix(StringRef &Str) {
  எனில் (Str.empty())
    திரும்பு 10;

  எனில் (Str.consume_front_insensitive("0x"))
    திரும்பு 16;

  எனில் (Str.consume_front_insensitive("0b"))
    திரும்பு 2;

  எனில் (Str.consume_front("0o"))
    திரும்பு 8;

  எனில் (Str[0] == '0' && Str.size() > 1 && isDigit(Str[1])) {
    Str = Str.substr(1);
    திரும்பு 8;
  }

  திரும்பு 10;
}

தருக்க llvm::consumeUnsignedInteger(StringRef &Str, unsigned Radix,
                                  unsigned long long &Result) {
  // Autosense radix if not specified.
  எனில் (Radix == 0)
    Radix = getAutoSenseRadix(Str);

  // Empty strings (after the radix autosense) are invalid.
  எனில் (Str.empty()) திரும்பு உண்மை;

  // Parse all the bytes of the string given this radix.  Watch for overflow.
  StringRef Str2 = Str;
  Result = 0;
  போது (!Str2.empty()) {
    unsigned CharVal;
    எனில் (Str2[0] >= '0' && Str2[0] <= '9')
      CharVal = Str2[0] - '0';
    இல்லையெனில் எனில் (Str2[0] >= 'a' && Str2[0] <= 'z')
      CharVal = Str2[0] - 'a' + 10;
    இல்லையெனில் எனில் (Str2[0] >= 'A' && Str2[0] <= 'Z')
      CharVal = Str2[0] - 'A' + 10;
    இல்லையெனில்
      நிறுத்து;

    // If the parsed value is larger than the integer radix, we cannot
    // consume any more characters.
    எனில் (CharVal >= Radix)
      நிறுத்து;

    // Add in this character.
    unsigned long long PrevResult = Result;
    Result = Result * Radix + CharVal;

    // Check for overflow by shifting back and seeing if bits were lost.
    எனில் (Result / Radix < PrevResult)
      திரும்பு உண்மை;

    Str2 = Str2.substr(1);
  }

  // We consider the operation a failure if no characters were consumed
  // successfully.
  எனில் (Str.size() == Str2.size())
    திரும்பு உண்மை;

  Str = Str2;
  திரும்பு பொய்;
}

தருக்க llvm::consumeSignedInteger(StringRef &Str, unsigned Radix,
                                long long &Result) {
  unsigned long long ULLVal;

  // Handle positive strings first.
  எனில் (!Str.starts_with("-")) {
    எனில் (consumeUnsignedInteger(Str, Radix, ULLVal) ||
        // Check for value so large it overflows a signed value.
        (long long)ULLVal < 0)
      திரும்பு உண்மை;
    Result = ULLVal;
    திரும்பு பொய்;
  }

  // Get the positive part of the value.
  StringRef Str2 = Str.drop_front(1);
  எனில் (consumeUnsignedInteger(Str2, Radix, ULLVal) ||
      // Reject values so large they'd overflow as negative signed, but allow
      // "-0".  This negates the unsigned so that the negative isn't undefined
      // on signed overflow.
      (long long)-ULLVal > 0)
    திரும்பு உண்மை;

  Str = Str2;
  Result = -ULLVal;
  திரும்பு பொய்;
}

/// GetAsUnsignedInteger - Workhorse method that converts a integer character
/// sequence of radix up to 36 to an unsigned long long value.
தருக்க llvm::getAsUnsignedInteger(StringRef Str, unsigned Radix,
                                unsigned long long &Result) {
  எனில் (consumeUnsignedInteger(Str, Radix, Result))
    திரும்பு உண்மை;

  // For getAsUnsignedInteger, we require the whole string to be consumed or
  // else we consider it a failure.
  திரும்பு !Str.empty();
}

தருக்க llvm::getAsSignedInteger(StringRef Str, unsigned Radix,
                              long long &Result) {
  எனில் (consumeSignedInteger(Str, Radix, Result))
    திரும்பு உண்மை;

  // For getAsSignedInteger, we require the whole string to be consumed or else
  // we consider it a failure.
  திரும்பு !Str.empty();
}

தருக்க StringRef::consumeInteger(unsigned Radix, APInt &Result) {
  StringRef Str = *இது;

  // Autosense radix if not specified.
  எனில் (Radix == 0)
    Radix = getAutoSenseRadix(Str);

  ப்ரதிஜ்ஞா(Radix > 1 && Radix <= 36);

  // Empty strings (after the radix autosense) are invalid.
  எனில் (Str.empty()) திரும்பு உண்மை;

  // Skip leading zeroes.  This can be a significant improvement if
  // it means we don't need > 64 bits.
  Str = Str.ltrim('0');

  // If it was nothing but zeroes....
  எனில் (Str.empty()) {
    Result = APInt(64, 0);
    *இது = Str;
    திரும்பு பொய்;
  }

  unsigned Log2Radix = 0;
  போது ((1U << Log2Radix) < Radix) Log2Radix++;
  தருக்க IsPowerOf2Radix = ((1U << Log2Radix) == Radix);

  // Initialize Result to a reasonable starting width (at least 64 bits),
  // but do not shrink it if it was already larger.
  unsigned BitWidth = std::max(64U, Result.getBitWidth());

  எனில் (Result.getBitWidth() < BitWidth) {
    Result = Result.zext(BitWidth);
  }

  APInt RadixAP, CharAP; // unused unless !IsPowerOf2Radix
  எனில் (!IsPowerOf2Radix) {
    // These must have the same bit-width as Result.
    RadixAP = APInt(BitWidth, Radix);
    CharAP = APInt(BitWidth, 0);
  }

  // Parse all the bytes of the string given this radix.
  Result = 0;
  போது (!Str.empty()) {
    unsigned CharVal;
    எனில் (Str[0] >= '0' && Str[0] <= '9')
      CharVal = Str[0]-'0';
    இல்லையெனில் எனில் (Str[0] >= 'a' && Str[0] <= 'z')
      CharVal = Str[0]-'a'+10;
    இல்லையெனில் எனில் (Str[0] >= 'A' && Str[0] <= 'Z')
      CharVal = Str[0]-'A'+10;
    இல்லையெனில்
      நிறுத்து;

    // If the parsed value is larger than the integer radix, the string is
    // invalid.
    எனில் (CharVal >= Radix)
      நிறுத்து;

    // Check if one more digit will overflow, and grow if so.
    எனில் (Result.getActiveBits() + Log2Radix > Result.getBitWidth()) {
      unsigned NewWidth = Result.getBitWidth() * 2;

      // Guard against overflow of the NewWidth itself
      எனில் (NewWidth < Result.getBitWidth())
        திரும்பு உண்மை;

      Result = Result.zext(NewWidth);
      எனில் (!IsPowerOf2Radix) {
        RadixAP = RadixAP.zext(NewWidth);
        CharAP = CharAP.zext(NewWidth);
      }
    }

    // Add in this character.
    எனில் (IsPowerOf2Radix) {
      Result <<= Log2Radix;
      Result |= CharVal;
    } இல்லையெனில் {
      Result *= RadixAP;
      CharAP = CharVal;
      Result += CharAP;
    }

    Str = Str.substr(1);
  }

  // We consider the operation a failure if no characters were consumed
  // successfully.
  எனில் (size() == Str.size())
    திரும்பு உண்மை;

  *இது = Str;
  திரும்பு பொய்;
}

தருக்க StringRef::getAsInteger(unsigned Radix, APInt &Result) மாறிலி {
  StringRef Str = *இது;
  எனில் (Str.consumeInteger(Radix, Result))
    திரும்பு உண்மை;

  // For getAsInteger, we require the whole string to be consumed or else we
  // consider it a failure.
  திரும்பு !Str.empty();
}

தருக்க StringRef::getAsDouble(இரட்டை &Result, தருக்க AllowInexact) மாறிலி {
  APFloat F(0.0);
  auto StatusOrErr = F.convertFromString(*இது, APFloat::rmNearestTiesToEven);
  எனில் (errorToBool(StatusOrErr.takeError()))
    திரும்பு உண்மை;

  APFloat::opStatus Status = *StatusOrErr;
  எனில் (Status != APFloat::opOK) {
    எனில் (!AllowInexact || !(Status & APFloat::opInexact))
      திரும்பு உண்மை;
  }

  Result = F.convertToDouble();
  திரும்பு பொய்;
}

// Implementation of StringRef hashing.
hash_code llvm::hash_மதிப்பு(StringRef S) { திரும்பு hash_combine_range(S); }

unsigned DenseMapInfo<StringRef, வெறுமை>::get_Hash_மதிப்பு(StringRef Val) {
  திரும்பு (unsigned)(hash_மதிப்பு(Val));
}
