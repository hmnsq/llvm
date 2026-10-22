//===-- StringRef.cpp - Lightweight String References ---------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#अन्तर्भाव "llvm/ADT/StringRef.h"
#अन्तर्भाव "llvm/ADT/APFloat.h"
#अन्तर्भाव "llvm/ADT/APInt.h"
#अन्तर्भाव "llvm/ADT/Hashing.h"
#अन्तर्भाव "llvm/ADT/StringExtras.h"
#अन्तर्भाव "llvm/ADT/edit_distance.h"
#अन्तर्भाव "llvm/Support/Error.h"
#अन्तर्भाव <bitset>

using नामाकाश llvm;

// strncasecmp() is not available on non-POSIX systems, so define an
// alternative function here.
स्थिर पूर्णाङ्क ascii_strncasecmp(StringRef LHS, StringRef RHS) {
  अत्थाय (auto [LC, RC] : zip_equal(LHS, RHS)) {
    unsigned वर्ण LHC = toLower(LC);
    unsigned वर्ण RHC = toLower(RC);
    सचे (LHC != RHC)
      पटिनिवत्त LHC < RHC ? -1 : 1;
  }
  पटिनिवत्त 0;
}

पूर्णाङ्क StringRef::compare_insensitive(StringRef RHS) ध्रुव {
  size_t Min = std::min(size(), RHS.size());
  सचे (पूर्णाङ्क Res = ascii_strncasecmp(take_front(Min), RHS.take_front(Min)))
    पटिनिवत्त Res;
  सचे (size() == RHS.size())
    पटिनिवत्त 0;
  पटिनिवत्त size() < RHS.size() ? -1 : 1;
}

तार्किक StringRef::starts_with_insensitive(StringRef Prefix) ध्रुव {
  पटिनिवत्त size() >= Prefix.size() &&
         ascii_strncasecmp(take_front(Prefix.size()), Prefix) == 0;
}

तार्किक StringRef::ends_with_insensitive(StringRef Suffix) ध्रुव {
  पटिनिवत्त size() >= Suffix.size() &&
         ascii_strncasecmp(take_back(Suffix.size()), Suffix) == 0;
}

size_t StringRef::find_insensitive(वर्ण C, size_t From) ध्रुव {
  वर्ण L = toLower(C);
  पटिनिवत्त find_if([L](वर्ण D) { पटिनिवत्त toLower(D) == L; }, From);
}

/// compare_numeric - Compare strings, handle embedded numbers.
पूर्णाङ्क StringRef::compare_numeric(StringRef RHS) ध्रुव {
  अत्थाय (size_t I = 0, E = std::min(size(), RHS.size()); I != E; ++I) {
    // Check for sequences of digits.
    सचे (isDigit(data()[I]) && isDigit(RHS.data()[I])) {
      // The longer sequence of numbers is considered larger.
      // This doesn't really handle prefixed zeros well.
      size_t J;
      अत्थाय (J = I + 1; J != E + 1; ++J) {
        तार्किक ld = J < size() && isDigit(data()[J]);
        तार्किक rd = J < RHS.size() && isDigit(RHS.data()[J]);
        सचे (ld != rd)
          पटिनिवत्त rd ? -1 : 1;
        सचे (!rd)
          भङ्ग;
      }
      // The two number sequences have the same length (J-I), just memcmp them.
      सचे (पूर्णाङ्क Res = compareMemory(data() + I, RHS.data() + I, J - I))
        पटिनिवत्त Res < 0 ? -1 : 1;
      // Identical number sequences, continue search after the numbers.
      I = J - 1;
      अनुवर्त;
    }
    सचे (data()[I] != RHS.data()[I])
      पटिनिवत्त (unsigned वर्ण)data()[I] < (unsigned वर्ण)RHS.data()[I] ? -1 : 1;
  }
  सचे (size() == RHS.size())
    पटिनिवत्त 0;
  पटिनिवत्त size() < RHS.size() ? -1 : 1;
}

// Compute the edit distance between the two given strings.
unsigned StringRef::edit_distance(llvm::StringRef Other,
                                  तार्किक AllowReplacements,
                                  unsigned MaxEditDistance) ध्रुव {
  पटिनिवत्त llvm::ComputeEditDistance(ArrayRef(data(), size()),
                                   ArrayRef(Other.data(), Other.size()),
                                   AllowReplacements, MaxEditDistance);
}

unsigned llvm::StringRef::edit_distance_insensitive(
    StringRef Other, तार्किक AllowReplacements, unsigned MaxEditDistance) ध्रुव {
  पटिनिवत्त llvm::ComputeMappedEditDistance(
      ArrayRef(data(), size()), ArrayRef(Other.data(), Other.size()),
      llvm::toLower, AllowReplacements, MaxEditDistance);
}

//===----------------------------------------------------------------------===//
// String Operations
//===----------------------------------------------------------------------===//

std::string StringRef::lower() ध्रुव {
  पटिनिवत्त std::string(map_iterator(begin(), toLower),
                     map_iterator(end(), toLower));
}

std::string StringRef::upper() ध्रुव {
  पटिनिवत्त std::string(map_iterator(begin(), toUpper),
                     map_iterator(end(), toUpper));
}

//===----------------------------------------------------------------------===//
// String Searching
//===----------------------------------------------------------------------===//


/// find - Search for the first string \arg Str in the string.
///
/// \return - The index of the first occurrence of \arg Str, or npos if not
/// found.
size_t StringRef::find(StringRef Str, size_t From) ध्रुव {
  सचे (From > size())
    पटिनिवत्त npos;

  ध्रुव वर्ण *Start = data() + From;
  size_t Size = size() - From;

  ध्रुव वर्ण *Needle = Str.data();
  size_t N = Str.size();
  सचे (N == 0)
    पटिनिवत्त From;
  सचे (Size < N)
    पटिनिवत्त npos;
  सचे (N == 1) {
    ध्रुव वर्ण *Ptr = (ध्रुव वर्ण *)::memchr(Start, Needle[0], Size);
    पटिनिवत्त Ptr == nullptr ? npos : Ptr - data();
  }

  ध्रुव वर्ण *Stop = Start + (Size - N + 1);

  सचे (N == 2) {
    // Provide a fast path for newline finding (CRLF case) in InclusionRewriter.
    // Not the most optimized strategy, but getting memcmp inlined should be
    // good enough.
    करोहि {
      सचे (std::memcmp(Start, Needle, 2) == 0)
        पटिनिवत्त Start - data();
      ++Start;
    } याव (Start < Stop);
    पटिनिवत्त npos;
  }

  // For short haystacks or unsupported needles fall back to the naive algorithm
  सचे (Size < 16 || N > 255) {
    करोहि {
      सचे (std::memcmp(Start, Needle, N) == 0)
        पटिनिवत्त Start - data();
      ++Start;
    } याव (Start < Stop);
    पटिनिवत्त npos;
  }

  // Build the bad char heuristic table, with uint8_t to reduce cache thrashing.
  uint8_t BadCharSkip[256];
  std::memset(BadCharSkip, N, 256);
  अत्थाय (unsigned i = 0; i != N-1; ++i)
    BadCharSkip[(uint8_t)Str[i]] = N-1-i;

  करोहि {
    uint8_t Last = Start[N - 1];
    सचे (LLVM_UNLIKELY(Last == (uint8_t)Needle[N - 1]))
      सचे (std::memcmp(Start, Needle, N - 1) == 0)
        पटिनिवत्त Start - data();

    // Otherwise skip the appropriate number of bytes.
    Start += BadCharSkip[Last];
  } याव (Start < Stop);

  पटिनिवत्त npos;
}

size_t StringRef::find_insensitive(StringRef Str, size_t From) ध्रुव {
  StringRef This = substr(From);
  याव (This.size() >= Str.size()) {
    सचे (This.starts_with_insensitive(Str))
      पटिनिवत्त From;
    This = This.drop_front();
    ++From;
  }
  पटिनिवत्त npos;
}

size_t StringRef::rfind_insensitive(वर्ण C, size_t From) ध्रुव {
  From = std::min(From, size());
  size_t i = From;
  याव (i != 0) {
    --i;
    सचे (toLower(data()[i]) == toLower(C))
      पटिनिवत्त i;
  }
  पटिनिवत्त npos;
}

/// rfind - Search for the last string \arg Str in the string.
///
/// \return - The index of the last occurrence of \arg Str, or npos if not
/// found.
size_t StringRef::rfind(StringRef Str) ध्रुव {
  पटिनिवत्त std::string_view(*एतत्).rfind(Str);
}

size_t StringRef::rfind_insensitive(StringRef Str) ध्रुव {
  size_t N = Str.size();
  सचे (N > size())
    पटिनिवत्त npos;
  अत्थाय (size_t i = size() - N + 1, e = 0; i != e;) {
    --i;
    सचे (substr(i, N).equals_insensitive(Str))
      पटिनिवत्त i;
  }
  पटिनिवत्त npos;
}

/// find_first_of - Find the first character in the string that is in \arg
/// Chars, or npos if not found.
///
/// Note: O(size() + Chars.size())
StringRef::size_type StringRef::find_first_of(StringRef Chars,
                                              size_t From) ध्रुव {
  std::bitset<1 << CHAR_BIT> CharBits;
  अत्थाय (वर्ण C : Chars)
    CharBits.set((unsigned वर्ण)C);

  अत्थाय (size_type i = std::min(From, size()), e = size(); i != e; ++i)
    सचे (CharBits.test((unsigned वर्ण)data()[i]))
      पटिनिवत्त i;
  पटिनिवत्त npos;
}

/// find_first_not_of - Find the first character in the string that is not
/// \arg C or npos if not found.
StringRef::size_type StringRef::find_first_not_of(वर्ण C, size_t From) ध्रुव {
  पटिनिवत्त std::string_view(*एतत्).find_first_not_of(C, From);
}

/// find_first_not_of - Find the first character in the string that is not
/// in the string \arg Chars, or npos if not found.
///
/// Note: O(size() + Chars.size())
StringRef::size_type StringRef::find_first_not_of(StringRef Chars,
                                                  size_t From) ध्रुव {
  std::bitset<1 << CHAR_BIT> CharBits;
  अत्थाय (वर्ण C : Chars)
    CharBits.set((unsigned वर्ण)C);

  अत्थाय (size_type i = std::min(From, size()), e = size(); i != e; ++i)
    सचे (!CharBits.test((unsigned वर्ण)data()[i]))
      पटिनिवत्त i;
  पटिनिवत्त npos;
}

/// find_last_of - Find the last character in the string that is in \arg C,
/// or npos if not found.
///
/// Note: O(size() + Chars.size())
StringRef::size_type StringRef::find_last_of(StringRef Chars,
                                             size_t From) ध्रुव {
  std::bitset<1 << CHAR_BIT> CharBits;
  अत्थाय (वर्ण C : Chars)
    CharBits.set((unsigned वर्ण)C);

  अत्थाय (size_type i = std::min(From, size()) - 1, e = -1; i != e; --i)
    सचे (CharBits.test((unsigned वर्ण)data()[i]))
      पटिनिवत्त i;
  पटिनिवत्त npos;
}

/// find_last_not_of - Find the last character in the string that is not
/// \arg C, or npos if not found.
StringRef::size_type StringRef::find_last_not_of(वर्ण C, size_t From) ध्रुव {
  अत्थाय (size_type i = std::min(From, size()) - 1, e = -1; i != e; --i)
    सचे (data()[i] != C)
      पटिनिवत्त i;
  पटिनिवत्त npos;
}

/// find_last_not_of - Find the last character in the string that is not in
/// \arg Chars, or npos if not found.
///
/// Note: O(size() + Chars.size())
StringRef::size_type StringRef::find_last_not_of(StringRef Chars,
                                                 size_t From) ध्रुव {
  std::bitset<1 << CHAR_BIT> CharBits;
  अत्थाय (वर्ण C : Chars)
    CharBits.set((unsigned वर्ण)C);

  अत्थाय (size_type i = std::min(From, size()) - 1, e = -1; i != e; --i)
    सचे (!CharBits.test((unsigned वर्ण)data()[i]))
      पटिनिवत्त i;
  पटिनिवत्त npos;
}

शून्य StringRef::split(SmallVectorImpl<StringRef> &A,
                      StringRef Separator, पूर्णाङ्क MaxSplit,
                      तार्किक KeepEmpty) ध्रुव {
  StringRef S = *एतत्;

  // Count down from MaxSplit. When MaxSplit is -1, this will just split
  // "forever". This doesn't support splitting more than 2^31 times
  // intentionally; if we ever want that we can make MaxSplit a 64-bit integer
  // but that seems unlikely to be useful.
  याव (MaxSplit-- != 0) {
    size_t Idx = S.find(Separator);
    सचे (Idx == npos)
      भङ्ग;

    // Push this split.
    सचे (KeepEmpty || Idx > 0)
      A.प्रक्षिप_back(S.slice(0, Idx));

    // Jump forward.
    S = S.substr(Idx + Separator.size());
  }

  // Push the tail.
  सचे (KeepEmpty || !S.empty())
    A.प्रक्षिप_back(S);
}

शून्य StringRef::split(SmallVectorImpl<StringRef> &A, वर्ण Separator,
                      पूर्णाङ्क MaxSplit, तार्किक KeepEmpty) ध्रुव {
  StringRef S = *एतत्;

  // Count down from MaxSplit. When MaxSplit is -1, this will just split
  // "forever". This doesn't support splitting more than 2^31 times
  // intentionally; if we ever want that we can make MaxSplit a 64-bit integer
  // but that seems unlikely to be useful.
  याव (MaxSplit-- != 0) {
    size_t Idx = S.find(Separator);
    सचे (Idx == npos)
      भङ्ग;

    // Push this split.
    सचे (KeepEmpty || Idx > 0)
      A.प्रक्षिप_back(S.slice(0, Idx));

    // Jump forward.
    S = S.substr(Idx + 1);
  }

  // Push the tail.
  सचे (KeepEmpty || !S.empty())
    A.प्रक्षिप_back(S);
}

//===----------------------------------------------------------------------===//
// Helpful Algorithms
//===----------------------------------------------------------------------===//

/// count - Return the number of non-overlapped occurrences of \arg Str in
/// the string.
size_t StringRef::गणनम्(StringRef Str) ध्रुव {
  size_t गणनम्_2 = 0;
  size_t Pos = 0;
  size_t N = Str.size();
  // TODO: For an empty `Str` we return 0 for legacy reasons. Consider changing
  //       this to `Length + 1` which is more in-line with the function
  //       description.
  सचे (!N)
    पटिनिवत्त 0;
  याव ((Pos = find(Str, Pos)) != npos) {
    ++गणनम्_2;
    Pos += N;
  }
  पटिनिवत्त गणनम्_2;
}

unsigned llvm::getAutoSenseRadix(StringRef &Str) {
  सचे (Str.empty())
    पटिनिवत्त 10;

  सचे (Str.consume_front_insensitive("0x"))
    पटिनिवत्त 16;

  सचे (Str.consume_front_insensitive("0b"))
    पटिनिवत्त 2;

  सचे (Str.consume_front("0o"))
    पटिनिवत्त 8;

  सचे (Str[0] == '0' && Str.size() > 1 && isDigit(Str[1])) {
    Str = Str.substr(1);
    पटिनिवत्त 8;
  }

  पटिनिवत्त 10;
}

तार्किक llvm::consumeUnsignedInteger(StringRef &Str, unsigned Radix,
                                  unsigned long long &Result) {
  // Autosense radix if not specified.
  सचे (Radix == 0)
    Radix = getAutoSenseRadix(Str);

  // Empty strings (after the radix autosense) are invalid.
  सचे (Str.empty()) पटिनिवत्त सच्च;

  // Parse all the bytes of the string given this radix.  Watch for overflow.
  StringRef Str2 = Str;
  Result = 0;
  याव (!Str2.empty()) {
    unsigned CharVal;
    सचे (Str2[0] >= '0' && Str2[0] <= '9')
      CharVal = Str2[0] - '0';
    अञ्ञथा सचे (Str2[0] >= 'a' && Str2[0] <= 'z')
      CharVal = Str2[0] - 'a' + 10;
    अञ्ञथा सचे (Str2[0] >= 'A' && Str2[0] <= 'Z')
      CharVal = Str2[0] - 'A' + 10;
    अञ्ञथा
      भङ्ग;

    // If the parsed value is larger than the integer radix, we cannot
    // consume any more characters.
    सचे (CharVal >= Radix)
      भङ्ग;

    // Add in this character.
    unsigned long long PrevResult = Result;
    Result = Result * Radix + CharVal;

    // Check for overflow by shifting back and seeing if bits were lost.
    सचे (Result / Radix < PrevResult)
      पटिनिवत्त सच्च;

    Str2 = Str2.substr(1);
  }

  // We consider the operation a failure if no characters were consumed
  // successfully.
  सचे (Str.size() == Str2.size())
    पटिनिवत्त सच्च;

  Str = Str2;
  पटिनिवत्त मिच्छा;
}

तार्किक llvm::consumeSignedInteger(StringRef &Str, unsigned Radix,
                                long long &Result) {
  unsigned long long ULLVal;

  // Handle positive strings first.
  सचे (!Str.starts_with("-")) {
    सचे (consumeUnsignedInteger(Str, Radix, ULLVal) ||
        // Check for value so large it overflows a signed value.
        (long long)ULLVal < 0)
      पटिनिवत्त सच्च;
    Result = ULLVal;
    पटिनिवत्त मिच्छा;
  }

  // Get the positive part of the value.
  StringRef Str2 = Str.drop_front(1);
  सचे (consumeUnsignedInteger(Str2, Radix, ULLVal) ||
      // Reject values so large they'd overflow as negative signed, but allow
      // "-0".  This negates the unsigned so that the negative isn't undefined
      // on signed overflow.
      (long long)-ULLVal > 0)
    पटिनिवत्त सच्च;

  Str = Str2;
  Result = -ULLVal;
  पटिनिवत्त मिच्छा;
}

/// GetAsUnsignedInteger - Workhorse method that converts a integer character
/// sequence of radix up to 36 to an unsigned long long value.
तार्किक llvm::getAsUnsignedInteger(StringRef Str, unsigned Radix,
                                unsigned long long &Result) {
  सचे (consumeUnsignedInteger(Str, Radix, Result))
    पटिनिवत्त सच्च;

  // For getAsUnsignedInteger, we require the whole string to be consumed or
  // else we consider it a failure.
  पटिनिवत्त !Str.empty();
}

तार्किक llvm::getAsSignedInteger(StringRef Str, unsigned Radix,
                              long long &Result) {
  सचे (consumeSignedInteger(Str, Radix, Result))
    पटिनिवत्त सच्च;

  // For getAsSignedInteger, we require the whole string to be consumed or else
  // we consider it a failure.
  पटिनिवत्त !Str.empty();
}

तार्किक StringRef::consumeInteger(unsigned Radix, APInt &Result) {
  StringRef Str = *एतत्;

  // Autosense radix if not specified.
  सचे (Radix == 0)
    Radix = getAutoSenseRadix(Str);

  प्रतिज्ञा(Radix > 1 && Radix <= 36);

  // Empty strings (after the radix autosense) are invalid.
  सचे (Str.empty()) पटिनिवत्त सच्च;

  // Skip leading zeroes.  This can be a significant improvement if
  // it means we don't need > 64 bits.
  Str = Str.ltrim('0');

  // If it was nothing but zeroes....
  सचे (Str.empty()) {
    Result = APInt(64, 0);
    *एतत् = Str;
    पटिनिवत्त मिच्छा;
  }

  unsigned Log2Radix = 0;
  याव ((1U << Log2Radix) < Radix) Log2Radix++;
  तार्किक IsPowerOf2Radix = ((1U << Log2Radix) == Radix);

  // Initialize Result to a reasonable starting width (at least 64 bits),
  // but do not shrink it if it was already larger.
  unsigned BitWidth = std::max(64U, Result.getBitWidth());

  सचे (Result.getBitWidth() < BitWidth) {
    Result = Result.zext(BitWidth);
  }

  APInt RadixAP, CharAP; // unused unless !IsPowerOf2Radix
  सचे (!IsPowerOf2Radix) {
    // These must have the same bit-width as Result.
    RadixAP = APInt(BitWidth, Radix);
    CharAP = APInt(BitWidth, 0);
  }

  // Parse all the bytes of the string given this radix.
  Result = 0;
  याव (!Str.empty()) {
    unsigned CharVal;
    सचे (Str[0] >= '0' && Str[0] <= '9')
      CharVal = Str[0]-'0';
    अञ्ञथा सचे (Str[0] >= 'a' && Str[0] <= 'z')
      CharVal = Str[0]-'a'+10;
    अञ्ञथा सचे (Str[0] >= 'A' && Str[0] <= 'Z')
      CharVal = Str[0]-'A'+10;
    अञ्ञथा
      भङ्ग;

    // If the parsed value is larger than the integer radix, the string is
    // invalid.
    सचे (CharVal >= Radix)
      भङ्ग;

    // Check if one more digit will overflow, and grow if so.
    सचे (Result.getActiveBits() + Log2Radix > Result.getBitWidth()) {
      unsigned NewWidth = Result.getBitWidth() * 2;

      // Guard against overflow of the NewWidth itself
      सचे (NewWidth < Result.getBitWidth())
        पटिनिवत्त सच्च;

      Result = Result.zext(NewWidth);
      सचे (!IsPowerOf2Radix) {
        RadixAP = RadixAP.zext(NewWidth);
        CharAP = CharAP.zext(NewWidth);
      }
    }

    // Add in this character.
    सचे (IsPowerOf2Radix) {
      Result <<= Log2Radix;
      Result |= CharVal;
    } अञ्ञथा {
      Result *= RadixAP;
      CharAP = CharVal;
      Result += CharAP;
    }

    Str = Str.substr(1);
  }

  // We consider the operation a failure if no characters were consumed
  // successfully.
  सचे (size() == Str.size())
    पटिनिवत्त सच्च;

  *एतत् = Str;
  पटिनिवत्त मिच्छा;
}

तार्किक StringRef::getAsInteger(unsigned Radix, APInt &Result) ध्रुव {
  StringRef Str = *एतत्;
  सचे (Str.consumeInteger(Radix, Result))
    पटिनिवत्त सच्च;

  // For getAsInteger, we require the whole string to be consumed or else we
  // consider it a failure.
  पटिनिवत्त !Str.empty();
}

तार्किक StringRef::getAsDouble(द्विगुण &Result, तार्किक AllowInexact) ध्रुव {
  APFloat F(0.0);
  auto StatusOrErr = F.convertFromString(*एतत्, APFloat::rmNearestTiesToEven);
  सचे (errorToBool(StatusOrErr.takeError()))
    पटिनिवत्त सच्च;

  APFloat::opStatus Status = *StatusOrErr;
  सचे (Status != APFloat::opOK) {
    सचे (!AllowInexact || !(Status & APFloat::opInexact))
      पटिनिवत्त सच्च;
  }

  Result = F.convertToDouble();
  पटिनिवत्त मिच्छा;
}

// Implementation of StringRef hashing.
hash_code llvm::hash_मानम्(StringRef S) { पटिनिवत्त hash_combine_range(S); }

unsigned DenseMapInfo<StringRef, शून्य>::get_Hash_मानम्(StringRef Val) {
  पटिनिवत्त (unsigned)(hash_मानम्(Val));
}
