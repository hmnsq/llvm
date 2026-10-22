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
  प्रति (auto [LC, RC] : zip_equal(LHS, RHS)) {
    unsigned वर्ण LHC = toLower(LC);
    unsigned वर्ण RHC = toLower(RC);
    यदि (LHC != RHC)
      प्रत्यागम LHC < RHC ? -1 : 1;
  }
  प्रत्यागम 0;
}

पूर्णाङ्क StringRef::compare_insensitive(StringRef RHS) ध्रुव {
  size_t Min = std::min(size(), RHS.size());
  यदि (पूर्णाङ्क Res = ascii_strncasecmp(take_front(Min), RHS.take_front(Min)))
    प्रत्यागम Res;
  यदि (size() == RHS.size())
    प्रत्यागम 0;
  प्रत्यागम size() < RHS.size() ? -1 : 1;
}

तार्किक StringRef::starts_with_insensitive(StringRef Prefix) ध्रुव {
  प्रत्यागम size() >= Prefix.size() &&
         ascii_strncasecmp(take_front(Prefix.size()), Prefix) == 0;
}

तार्किक StringRef::ends_with_insensitive(StringRef Suffix) ध्रुव {
  प्रत्यागम size() >= Suffix.size() &&
         ascii_strncasecmp(take_back(Suffix.size()), Suffix) == 0;
}

size_t StringRef::find_insensitive(वर्ण C, size_t From) ध्रुव {
  वर्ण L = toLower(C);
  प्रत्यागम find_if([L](वर्ण D) { प्रत्यागम toLower(D) == L; }, From);
}

/// compare_numeric - Compare strings, handle embedded numbers.
पूर्णाङ्क StringRef::compare_numeric(StringRef RHS) ध्रुव {
  प्रति (size_t I = 0, E = std::min(size(), RHS.size()); I != E; ++I) {
    // Check for sequences of digits.
    यदि (isDigit(data()[I]) && isDigit(RHS.data()[I])) {
      // The longer sequence of numbers is considered larger.
      // This doesn't really handle prefixed zeros well.
      size_t J;
      प्रति (J = I + 1; J != E + 1; ++J) {
        तार्किक ld = J < size() && isDigit(data()[J]);
        तार्किक rd = J < RHS.size() && isDigit(RHS.data()[J]);
        यदि (ld != rd)
          प्रत्यागम rd ? -1 : 1;
        यदि (!rd)
          भङ्ग;
      }
      // The two number sequences have the same length (J-I), just memcmp them.
      यदि (पूर्णाङ्क Res = compareMemory(data() + I, RHS.data() + I, J - I))
        प्रत्यागम Res < 0 ? -1 : 1;
      // Identical number sequences, continue search after the numbers.
      I = J - 1;
      अनुवर्त;
    }
    यदि (data()[I] != RHS.data()[I])
      प्रत्यागम (unsigned वर्ण)data()[I] < (unsigned वर्ण)RHS.data()[I] ? -1 : 1;
  }
  यदि (size() == RHS.size())
    प्रत्यागम 0;
  प्रत्यागम size() < RHS.size() ? -1 : 1;
}

// Compute the edit distance between the two given strings.
unsigned StringRef::edit_distance(llvm::StringRef Other,
                                  तार्किक AllowReplacements,
                                  unsigned MaxEditDistance) ध्रुव {
  प्रत्यागम llvm::ComputeEditDistance(ArrayRef(data(), size()),
                                   ArrayRef(Other.data(), Other.size()),
                                   AllowReplacements, MaxEditDistance);
}

unsigned llvm::StringRef::edit_distance_insensitive(
    StringRef Other, तार्किक AllowReplacements, unsigned MaxEditDistance) ध्रुव {
  प्रत्यागम llvm::ComputeMappedEditDistance(
      ArrayRef(data(), size()), ArrayRef(Other.data(), Other.size()),
      llvm::toLower, AllowReplacements, MaxEditDistance);
}

//===----------------------------------------------------------------------===//
// String Operations
//===----------------------------------------------------------------------===//

std::string StringRef::lower() ध्रुव {
  प्रत्यागम std::string(map_iterator(begin(), toLower),
                     map_iterator(end(), toLower));
}

std::string StringRef::upper() ध्रुव {
  प्रत्यागम std::string(map_iterator(begin(), toUpper),
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
  यदि (From > size())
    प्रत्यागम npos;

  ध्रुव वर्ण *Start = data() + From;
  size_t Size = size() - From;

  ध्रुव वर्ण *Needle = Str.data();
  size_t N = Str.size();
  यदि (N == 0)
    प्रत्यागम From;
  यदि (Size < N)
    प्रत्यागम npos;
  यदि (N == 1) {
    ध्रुव वर्ण *Ptr = (ध्रुव वर्ण *)::memchr(Start, Needle[0], Size);
    प्रत्यागम Ptr == nullptr ? npos : Ptr - data();
  }

  ध्रुव वर्ण *Stop = Start + (Size - N + 1);

  यदि (N == 2) {
    // Provide a fast path for newline finding (CRLF case) in InclusionRewriter.
    // Not the most optimized strategy, but getting memcmp inlined should be
    // good enough.
    कुरु {
      यदि (std::memcmp(Start, Needle, 2) == 0)
        प्रत्यागम Start - data();
      ++Start;
    } यावत् (Start < Stop);
    प्रत्यागम npos;
  }

  // For short haystacks or unsupported needles fall back to the naive algorithm
  यदि (Size < 16 || N > 255) {
    कुरु {
      यदि (std::memcmp(Start, Needle, N) == 0)
        प्रत्यागम Start - data();
      ++Start;
    } यावत् (Start < Stop);
    प्रत्यागम npos;
  }

  // Build the bad char heuristic table, with uint8_t to reduce cache thrashing.
  uint8_t BadCharSkip[256];
  std::memset(BadCharSkip, N, 256);
  प्रति (unsigned i = 0; i != N-1; ++i)
    BadCharSkip[(uint8_t)Str[i]] = N-1-i;

  कुरु {
    uint8_t Last = Start[N - 1];
    यदि (LLVM_UNLIKELY(Last == (uint8_t)Needle[N - 1]))
      यदि (std::memcmp(Start, Needle, N - 1) == 0)
        प्रत्यागम Start - data();

    // Otherwise skip the appropriate number of bytes.
    Start += BadCharSkip[Last];
  } यावत् (Start < Stop);

  प्रत्यागम npos;
}

size_t StringRef::find_insensitive(StringRef Str, size_t From) ध्रुव {
  StringRef This = substr(From);
  यावत् (This.size() >= Str.size()) {
    यदि (This.starts_with_insensitive(Str))
      प्रत्यागम From;
    This = This.drop_front();
    ++From;
  }
  प्रत्यागम npos;
}

size_t StringRef::rfind_insensitive(वर्ण C, size_t From) ध्रुव {
  From = std::min(From, size());
  size_t i = From;
  यावत् (i != 0) {
    --i;
    यदि (toLower(data()[i]) == toLower(C))
      प्रत्यागम i;
  }
  प्रत्यागम npos;
}

/// rfind - Search for the last string \arg Str in the string.
///
/// \return - The index of the last occurrence of \arg Str, or npos if not
/// found.
size_t StringRef::rfind(StringRef Str) ध्रुव {
  प्रत्यागम std::string_view(*एतत्).rfind(Str);
}

size_t StringRef::rfind_insensitive(StringRef Str) ध्रुव {
  size_t N = Str.size();
  यदि (N > size())
    प्रत्यागम npos;
  प्रति (size_t i = size() - N + 1, e = 0; i != e;) {
    --i;
    यदि (substr(i, N).equals_insensitive(Str))
      प्रत्यागम i;
  }
  प्रत्यागम npos;
}

/// find_first_of - Find the first character in the string that is in \arg
/// Chars, or npos if not found.
///
/// Note: O(size() + Chars.size())
StringRef::size_type StringRef::find_first_of(StringRef Chars,
                                              size_t From) ध्रुव {
  std::bitset<1 << CHAR_BIT> CharBits;
  प्रति (वर्ण C : Chars)
    CharBits.set((unsigned वर्ण)C);

  प्रति (size_type i = std::min(From, size()), e = size(); i != e; ++i)
    यदि (CharBits.test((unsigned वर्ण)data()[i]))
      प्रत्यागम i;
  प्रत्यागम npos;
}

/// find_first_not_of - Find the first character in the string that is not
/// \arg C or npos if not found.
StringRef::size_type StringRef::find_first_not_of(वर्ण C, size_t From) ध्रुव {
  प्रत्यागम std::string_view(*एतत्).find_first_not_of(C, From);
}

/// find_first_not_of - Find the first character in the string that is not
/// in the string \arg Chars, or npos if not found.
///
/// Note: O(size() + Chars.size())
StringRef::size_type StringRef::find_first_not_of(StringRef Chars,
                                                  size_t From) ध्रुव {
  std::bitset<1 << CHAR_BIT> CharBits;
  प्रति (वर्ण C : Chars)
    CharBits.set((unsigned वर्ण)C);

  प्रति (size_type i = std::min(From, size()), e = size(); i != e; ++i)
    यदि (!CharBits.test((unsigned वर्ण)data()[i]))
      प्रत्यागम i;
  प्रत्यागम npos;
}

/// find_last_of - Find the last character in the string that is in \arg C,
/// or npos if not found.
///
/// Note: O(size() + Chars.size())
StringRef::size_type StringRef::find_last_of(StringRef Chars,
                                             size_t From) ध्रुव {
  std::bitset<1 << CHAR_BIT> CharBits;
  प्रति (वर्ण C : Chars)
    CharBits.set((unsigned वर्ण)C);

  प्रति (size_type i = std::min(From, size()) - 1, e = -1; i != e; --i)
    यदि (CharBits.test((unsigned वर्ण)data()[i]))
      प्रत्यागम i;
  प्रत्यागम npos;
}

/// find_last_not_of - Find the last character in the string that is not
/// \arg C, or npos if not found.
StringRef::size_type StringRef::find_last_not_of(वर्ण C, size_t From) ध्रुव {
  प्रति (size_type i = std::min(From, size()) - 1, e = -1; i != e; --i)
    यदि (data()[i] != C)
      प्रत्यागम i;
  प्रत्यागम npos;
}

/// find_last_not_of - Find the last character in the string that is not in
/// \arg Chars, or npos if not found.
///
/// Note: O(size() + Chars.size())
StringRef::size_type StringRef::find_last_not_of(StringRef Chars,
                                                 size_t From) ध्रुव {
  std::bitset<1 << CHAR_BIT> CharBits;
  प्रति (वर्ण C : Chars)
    CharBits.set((unsigned वर्ण)C);

  प्रति (size_type i = std::min(From, size()) - 1, e = -1; i != e; --i)
    यदि (!CharBits.test((unsigned वर्ण)data()[i]))
      प्रत्यागम i;
  प्रत्यागम npos;
}

शून्य StringRef::split(SmallVectorImpl<StringRef> &A,
                      StringRef Separator, पूर्णाङ्क MaxSplit,
                      तार्किक KeepEmpty) ध्रुव {
  StringRef S = *एतत्;

  // Count down from MaxSplit. When MaxSplit is -1, this will just split
  // "forever". This doesn't support splitting more than 2^31 times
  // intentionally; if we ever want that we can make MaxSplit a 64-bit integer
  // but that seems unlikely to be useful.
  यावत् (MaxSplit-- != 0) {
    size_t Idx = S.find(Separator);
    यदि (Idx == npos)
      भङ्ग;

    // Push this split.
    यदि (KeepEmpty || Idx > 0)
      A.ढकला_back(S.slice(0, Idx));

    // Jump forward.
    S = S.substr(Idx + Separator.size());
  }

  // Push the tail.
  यदि (KeepEmpty || !S.empty())
    A.ढकला_back(S);
}

शून्य StringRef::split(SmallVectorImpl<StringRef> &A, वर्ण Separator,
                      पूर्णाङ्क MaxSplit, तार्किक KeepEmpty) ध्रुव {
  StringRef S = *एतत्;

  // Count down from MaxSplit. When MaxSplit is -1, this will just split
  // "forever". This doesn't support splitting more than 2^31 times
  // intentionally; if we ever want that we can make MaxSplit a 64-bit integer
  // but that seems unlikely to be useful.
  यावत् (MaxSplit-- != 0) {
    size_t Idx = S.find(Separator);
    यदि (Idx == npos)
      भङ्ग;

    // Push this split.
    यदि (KeepEmpty || Idx > 0)
      A.ढकला_back(S.slice(0, Idx));

    // Jump forward.
    S = S.substr(Idx + 1);
  }

  // Push the tail.
  यदि (KeepEmpty || !S.empty())
    A.ढकला_back(S);
}

//===----------------------------------------------------------------------===//
// Helpful Algorithms
//===----------------------------------------------------------------------===//

/// count - Return the number of non-overlapped occurrences of \arg Str in
/// the string.
size_t StringRef::मोजणी(StringRef Str) ध्रुव {
  size_t मोजणी_2 = 0;
  size_t Pos = 0;
  size_t N = Str.size();
  // TODO: For an empty `Str` we return 0 for legacy reasons. Consider changing
  //       this to `Length + 1` which is more in-line with the function
  //       description.
  यदि (!N)
    प्रत्यागम 0;
  यावत् ((Pos = find(Str, Pos)) != npos) {
    ++मोजणी_2;
    Pos += N;
  }
  प्रत्यागम मोजणी_2;
}

unsigned llvm::getAutoSenseRadix(StringRef &Str) {
  यदि (Str.empty())
    प्रत्यागम 10;

  यदि (Str.consume_front_insensitive("0x"))
    प्रत्यागम 16;

  यदि (Str.consume_front_insensitive("0b"))
    प्रत्यागम 2;

  यदि (Str.consume_front("0o"))
    प्रत्यागम 8;

  यदि (Str[0] == '0' && Str.size() > 1 && isDigit(Str[1])) {
    Str = Str.substr(1);
    प्रत्यागम 8;
  }

  प्रत्यागम 10;
}

तार्किक llvm::consumeUnsignedInteger(StringRef &Str, unsigned Radix,
                                  unsigned long long &Result) {
  // Autosense radix if not specified.
  यदि (Radix == 0)
    Radix = getAutoSenseRadix(Str);

  // Empty strings (after the radix autosense) are invalid.
  यदि (Str.empty()) प्रत्यागम सत्य;

  // Parse all the bytes of the string given this radix.  Watch for overflow.
  StringRef Str2 = Str;
  Result = 0;
  यावत् (!Str2.empty()) {
    unsigned CharVal;
    यदि (Str2[0] >= '0' && Str2[0] <= '9')
      CharVal = Str2[0] - '0';
    अन्यथा यदि (Str2[0] >= 'a' && Str2[0] <= 'z')
      CharVal = Str2[0] - 'a' + 10;
    अन्यथा यदि (Str2[0] >= 'A' && Str2[0] <= 'Z')
      CharVal = Str2[0] - 'A' + 10;
    अन्यथा
      भङ्ग;

    // If the parsed value is larger than the integer radix, we cannot
    // consume any more characters.
    यदि (CharVal >= Radix)
      भङ्ग;

    // Add in this character.
    unsigned long long PrevResult = Result;
    Result = Result * Radix + CharVal;

    // Check for overflow by shifting back and seeing if bits were lost.
    यदि (Result / Radix < PrevResult)
      प्रत्यागम सत्य;

    Str2 = Str2.substr(1);
  }

  // We consider the operation a failure if no characters were consumed
  // successfully.
  यदि (Str.size() == Str2.size())
    प्रत्यागम सत्य;

  Str = Str2;
  प्रत्यागम असत्य;
}

तार्किक llvm::consumeSignedInteger(StringRef &Str, unsigned Radix,
                                long long &Result) {
  unsigned long long ULLVal;

  // Handle positive strings first.
  यदि (!Str.starts_with("-")) {
    यदि (consumeUnsignedInteger(Str, Radix, ULLVal) ||
        // Check for value so large it overflows a signed value.
        (long long)ULLVal < 0)
      प्रत्यागम सत्य;
    Result = ULLVal;
    प्रत्यागम असत्य;
  }

  // Get the positive part of the value.
  StringRef Str2 = Str.drop_front(1);
  यदि (consumeUnsignedInteger(Str2, Radix, ULLVal) ||
      // Reject values so large they'd overflow as negative signed, but allow
      // "-0".  This negates the unsigned so that the negative isn't undefined
      // on signed overflow.
      (long long)-ULLVal > 0)
    प्रत्यागम सत्य;

  Str = Str2;
  Result = -ULLVal;
  प्रत्यागम असत्य;
}

/// GetAsUnsignedInteger - Workhorse method that converts a integer character
/// sequence of radix up to 36 to an unsigned long long value.
तार्किक llvm::getAsUnsignedInteger(StringRef Str, unsigned Radix,
                                unsigned long long &Result) {
  यदि (consumeUnsignedInteger(Str, Radix, Result))
    प्रत्यागम सत्य;

  // For getAsUnsignedInteger, we require the whole string to be consumed or
  // else we consider it a failure.
  प्रत्यागम !Str.empty();
}

तार्किक llvm::getAsSignedInteger(StringRef Str, unsigned Radix,
                              long long &Result) {
  यदि (consumeSignedInteger(Str, Radix, Result))
    प्रत्यागम सत्य;

  // For getAsSignedInteger, we require the whole string to be consumed or else
  // we consider it a failure.
  प्रत्यागम !Str.empty();
}

तार्किक StringRef::consumeInteger(unsigned Radix, APInt &Result) {
  StringRef Str = *एतत्;

  // Autosense radix if not specified.
  यदि (Radix == 0)
    Radix = getAutoSenseRadix(Str);

  प्रतिज्ञा(Radix > 1 && Radix <= 36);

  // Empty strings (after the radix autosense) are invalid.
  यदि (Str.empty()) प्रत्यागम सत्य;

  // Skip leading zeroes.  This can be a significant improvement if
  // it means we don't need > 64 bits.
  Str = Str.ltrim('0');

  // If it was nothing but zeroes....
  यदि (Str.empty()) {
    Result = APInt(64, 0);
    *एतत् = Str;
    प्रत्यागम असत्य;
  }

  unsigned Log2Radix = 0;
  यावत् ((1U << Log2Radix) < Radix) Log2Radix++;
  तार्किक IsPowerOf2Radix = ((1U << Log2Radix) == Radix);

  // Initialize Result to a reasonable starting width (at least 64 bits),
  // but do not shrink it if it was already larger.
  unsigned BitWidth = std::max(64U, Result.getBitWidth());

  यदि (Result.getBitWidth() < BitWidth) {
    Result = Result.zext(BitWidth);
  }

  APInt RadixAP, CharAP; // unused unless !IsPowerOf2Radix
  यदि (!IsPowerOf2Radix) {
    // These must have the same bit-width as Result.
    RadixAP = APInt(BitWidth, Radix);
    CharAP = APInt(BitWidth, 0);
  }

  // Parse all the bytes of the string given this radix.
  Result = 0;
  यावत् (!Str.empty()) {
    unsigned CharVal;
    यदि (Str[0] >= '0' && Str[0] <= '9')
      CharVal = Str[0]-'0';
    अन्यथा यदि (Str[0] >= 'a' && Str[0] <= 'z')
      CharVal = Str[0]-'a'+10;
    अन्यथा यदि (Str[0] >= 'A' && Str[0] <= 'Z')
      CharVal = Str[0]-'A'+10;
    अन्यथा
      भङ्ग;

    // If the parsed value is larger than the integer radix, the string is
    // invalid.
    यदि (CharVal >= Radix)
      भङ्ग;

    // Check if one more digit will overflow, and grow if so.
    यदि (Result.getActiveBits() + Log2Radix > Result.getBitWidth()) {
      unsigned NewWidth = Result.getBitWidth() * 2;

      // Guard against overflow of the NewWidth itself
      यदि (NewWidth < Result.getBitWidth())
        प्रत्यागम सत्य;

      Result = Result.zext(NewWidth);
      यदि (!IsPowerOf2Radix) {
        RadixAP = RadixAP.zext(NewWidth);
        CharAP = CharAP.zext(NewWidth);
      }
    }

    // Add in this character.
    यदि (IsPowerOf2Radix) {
      Result <<= Log2Radix;
      Result |= CharVal;
    } अन्यथा {
      Result *= RadixAP;
      CharAP = CharVal;
      Result += CharAP;
    }

    Str = Str.substr(1);
  }

  // We consider the operation a failure if no characters were consumed
  // successfully.
  यदि (size() == Str.size())
    प्रत्यागम सत्य;

  *एतत् = Str;
  प्रत्यागम असत्य;
}

तार्किक StringRef::getAsInteger(unsigned Radix, APInt &Result) ध्रुव {
  StringRef Str = *एतत्;
  यदि (Str.consumeInteger(Radix, Result))
    प्रत्यागम सत्य;

  // For getAsInteger, we require the whole string to be consumed or else we
  // consider it a failure.
  प्रत्यागम !Str.empty();
}

तार्किक StringRef::getAsDouble(द्विगुण &Result, तार्किक AllowInexact) ध्रुव {
  APFloat F(0.0);
  auto StatusOrErr = F.convertFromString(*एतत्, APFloat::rmNearestTiesToEven);
  यदि (errorToBool(StatusOrErr.takeError()))
    प्रत्यागम सत्य;

  APFloat::opStatus Status = *StatusOrErr;
  यदि (Status != APFloat::opOK) {
    यदि (!AllowInexact || !(Status & APFloat::opInexact))
      प्रत्यागम सत्य;
  }

  Result = F.convertToDouble();
  प्रत्यागम असत्य;
}

// Implementation of StringRef hashing.
hash_code llvm::hash_मूल्य(StringRef S) { प्रत्यागम hash_combine_range(S); }

unsigned DenseMapInfo<StringRef, शून्य>::get_Hash_मूल्य(StringRef Val) {
  प्रत्यागम (unsigned)(hash_मूल्य(Val));
}
