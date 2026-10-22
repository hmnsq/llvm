//===-- StringRef.cpp - Lightweight String References ---------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#شامل "llvm/ADT/StringRef.h"
#شامل "llvm/ADT/APFloat.h"
#شامل "llvm/ADT/APInt.h"
#شامل "llvm/ADT/Hashing.h"
#شامل "llvm/ADT/StringExtras.h"
#شامل "llvm/ADT/edit_distance.h"
#شامل "llvm/Support/Error.h"
#شامل <bitset>

using فضاینام llvm;

// strncasecmp() is not available on non-POSIX systems, so define an
// alternative function here.
ایستا صحیح ascii_strncasecmp(StringRef LHS, StringRef RHS) {
  برای (auto [LC, RC] : zip_equal(LHS, RHS)) {
    unsigned نویسه LHC = toLower(LC);
    unsigned نویسه RHC = toLower(RC);
    اگر (LHC != RHC)
      بازگشت LHC < RHC ? -1 : 1;
  }
  بازگشت 0;
}

صحیح StringRef::compare_insensitive(StringRef RHS) ثابت {
  size_t Min = std::min(size(), RHS.size());
  اگر (صحیح Res = ascii_strncasecmp(take_front(Min), RHS.take_front(Min)))
    بازگشت Res;
  اگر (size() == RHS.size())
    بازگشت 0;
  بازگشت size() < RHS.size() ? -1 : 1;
}

منطقی StringRef::starts_with_insensitive(StringRef Prefix) ثابت {
  بازگشت size() >= Prefix.size() &&
         ascii_strncasecmp(take_front(Prefix.size()), Prefix) == 0;
}

منطقی StringRef::ends_with_insensitive(StringRef Suffix) ثابت {
  بازگشت size() >= Suffix.size() &&
         ascii_strncasecmp(take_back(Suffix.size()), Suffix) == 0;
}

size_t StringRef::find_insensitive(نویسه C, size_t From) ثابت {
  نویسه L = toLower(C);
  بازگشت find_if([L](نویسه D) { بازگشت toLower(D) == L; }, From);
}

/// compare_numeric - Compare strings, handle embedded numbers.
صحیح StringRef::compare_numeric(StringRef RHS) ثابت {
  برای (size_t I = 0, E = std::min(size(), RHS.size()); I != E; ++I) {
    // Check for sequences of digits.
    اگر (isDigit(data()[I]) && isDigit(RHS.data()[I])) {
      // The longer sequence of numbers is considered larger.
      // This doesn't really handle prefixed zeros well.
      size_t J;
      برای (J = I + 1; J != E + 1; ++J) {
        منطقی ld = J < size() && isDigit(data()[J]);
        منطقی rd = J < RHS.size() && isDigit(RHS.data()[J]);
        اگر (ld != rd)
          بازگشت rd ? -1 : 1;
        اگر (!rd)
          بشکن;
      }
      // The two number sequences have the same length (J-I), just memcmp them.
      اگر (صحیح Res = compareMemory(data() + I, RHS.data() + I, J - I))
        بازگشت Res < 0 ? -1 : 1;
      // Identical number sequences, continue search after the numbers.
      I = J - 1;
      ادامه;
    }
    اگر (data()[I] != RHS.data()[I])
      بازگشت (unsigned نویسه)data()[I] < (unsigned نویسه)RHS.data()[I] ? -1 : 1;
  }
  اگر (size() == RHS.size())
    بازگشت 0;
  بازگشت size() < RHS.size() ? -1 : 1;
}

// Compute the edit distance between the two given strings.
unsigned StringRef::edit_distance(llvm::StringRef Other,
                                  منطقی AllowReplacements,
                                  unsigned MaxEditDistance) ثابت {
  بازگشت llvm::ComputeEditDistance(ArrayRef(data(), size()),
                                   ArrayRef(Other.data(), Other.size()),
                                   AllowReplacements, MaxEditDistance);
}

unsigned llvm::StringRef::edit_distance_insensitive(
    StringRef Other, منطقی AllowReplacements, unsigned MaxEditDistance) ثابت {
  بازگشت llvm::ComputeMappedEditDistance(
      ArrayRef(data(), size()), ArrayRef(Other.data(), Other.size()),
      llvm::toLower, AllowReplacements, MaxEditDistance);
}

//===----------------------------------------------------------------------===//
// String Operations
//===----------------------------------------------------------------------===//

std::string StringRef::lower() ثابت {
  بازگشت std::string(map_iterator(begin(), toLower),
                     map_iterator(end(), toLower));
}

std::string StringRef::upper() ثابت {
  بازگشت std::string(map_iterator(begin(), toUpper),
                     map_iterator(end(), toUpper));
}

//===----------------------------------------------------------------------===//
// String Searching
//===----------------------------------------------------------------------===//


/// find - Search for the first string \arg Str in the string.
///
/// \return - The index of the first occurrence of \arg Str, or npos if not
/// found.
size_t StringRef::find(StringRef Str, size_t From) ثابت {
  اگر (From > size())
    بازگشت npos;

  ثابت نویسه *Start = data() + From;
  size_t Size = size() - From;

  ثابت نویسه *Needle = Str.data();
  size_t N = Str.size();
  اگر (N == 0)
    بازگشت From;
  اگر (Size < N)
    بازگشت npos;
  اگر (N == 1) {
    ثابت نویسه *Ptr = (ثابت نویسه *)::memchr(Start, Needle[0], Size);
    بازگشت Ptr == nullptr ? npos : Ptr - data();
  }

  ثابت نویسه *Stop = Start + (Size - N + 1);

  اگر (N == 2) {
    // Provide a fast path for newline finding (CRLF case) in InclusionRewriter.
    // Not the most optimized strategy, but getting memcmp inlined should be
    // good enough.
    انجامبده {
      اگر (std::memcmp(Start, Needle, 2) == 0)
        بازگشت Start - data();
      ++Start;
    } وقتی (Start < Stop);
    بازگشت npos;
  }

  // For short haystacks or unsupported needles fall back to the naive algorithm
  اگر (Size < 16 || N > 255) {
    انجامبده {
      اگر (std::memcmp(Start, Needle, N) == 0)
        بازگشت Start - data();
      ++Start;
    } وقتی (Start < Stop);
    بازگشت npos;
  }

  // Build the bad char heuristic table, with uint8_t to reduce cache thrashing.
  uint8_t BadCharSkip[256];
  std::memset(BadCharSkip, N, 256);
  برای (unsigned i = 0; i != N-1; ++i)
    BadCharSkip[(uint8_t)Str[i]] = N-1-i;

  انجامبده {
    uint8_t Last = Start[N - 1];
    اگر (LLVM_UNLIKELY(Last == (uint8_t)Needle[N - 1]))
      اگر (std::memcmp(Start, Needle, N - 1) == 0)
        بازگشت Start - data();

    // Otherwise skip the appropriate number of bytes.
    Start += BadCharSkip[Last];
  } وقتی (Start < Stop);

  بازگشت npos;
}

size_t StringRef::find_insensitive(StringRef Str, size_t From) ثابت {
  StringRef This = substr(From);
  وقتی (This.size() >= Str.size()) {
    اگر (This.starts_with_insensitive(Str))
      بازگشت From;
    This = This.drop_front();
    ++From;
  }
  بازگشت npos;
}

size_t StringRef::rfind_insensitive(نویسه C, size_t From) ثابت {
  From = std::min(From, size());
  size_t i = From;
  وقتی (i != 0) {
    --i;
    اگر (toLower(data()[i]) == toLower(C))
      بازگشت i;
  }
  بازگشت npos;
}

/// rfind - Search for the last string \arg Str in the string.
///
/// \return - The index of the last occurrence of \arg Str, or npos if not
/// found.
size_t StringRef::rfind(StringRef Str) ثابت {
  بازگشت std::string_view(*این).rfind(Str);
}

size_t StringRef::rfind_insensitive(StringRef Str) ثابت {
  size_t N = Str.size();
  اگر (N > size())
    بازگشت npos;
  برای (size_t i = size() - N + 1, e = 0; i != e;) {
    --i;
    اگر (substr(i, N).equals_insensitive(Str))
      بازگشت i;
  }
  بازگشت npos;
}

/// find_first_of - Find the first character in the string that is in \arg
/// Chars, or npos if not found.
///
/// Note: O(size() + Chars.size())
StringRef::size_type StringRef::find_first_of(StringRef Chars,
                                              size_t From) ثابت {
  std::bitset<1 << CHAR_BIT> CharBits;
  برای (نویسه C : Chars)
    CharBits.set((unsigned نویسه)C);

  برای (size_type i = std::min(From, size()), e = size(); i != e; ++i)
    اگر (CharBits.test((unsigned نویسه)data()[i]))
      بازگشت i;
  بازگشت npos;
}

/// find_first_not_of - Find the first character in the string that is not
/// \arg C or npos if not found.
StringRef::size_type StringRef::find_first_not_of(نویسه C, size_t From) ثابت {
  بازگشت std::string_view(*این).find_first_not_of(C, From);
}

/// find_first_not_of - Find the first character in the string that is not
/// in the string \arg Chars, or npos if not found.
///
/// Note: O(size() + Chars.size())
StringRef::size_type StringRef::find_first_not_of(StringRef Chars,
                                                  size_t From) ثابت {
  std::bitset<1 << CHAR_BIT> CharBits;
  برای (نویسه C : Chars)
    CharBits.set((unsigned نویسه)C);

  برای (size_type i = std::min(From, size()), e = size(); i != e; ++i)
    اگر (!CharBits.test((unsigned نویسه)data()[i]))
      بازگشت i;
  بازگشت npos;
}

/// find_last_of - Find the last character in the string that is in \arg C,
/// or npos if not found.
///
/// Note: O(size() + Chars.size())
StringRef::size_type StringRef::find_last_of(StringRef Chars,
                                             size_t From) ثابت {
  std::bitset<1 << CHAR_BIT> CharBits;
  برای (نویسه C : Chars)
    CharBits.set((unsigned نویسه)C);

  برای (size_type i = std::min(From, size()) - 1, e = -1; i != e; --i)
    اگر (CharBits.test((unsigned نویسه)data()[i]))
      بازگشت i;
  بازگشت npos;
}

/// find_last_not_of - Find the last character in the string that is not
/// \arg C, or npos if not found.
StringRef::size_type StringRef::find_last_not_of(نویسه C, size_t From) ثابت {
  برای (size_type i = std::min(From, size()) - 1, e = -1; i != e; --i)
    اگر (data()[i] != C)
      بازگشت i;
  بازگشت npos;
}

/// find_last_not_of - Find the last character in the string that is not in
/// \arg Chars, or npos if not found.
///
/// Note: O(size() + Chars.size())
StringRef::size_type StringRef::find_last_not_of(StringRef Chars,
                                                 size_t From) ثابت {
  std::bitset<1 << CHAR_BIT> CharBits;
  برای (نویسه C : Chars)
    CharBits.set((unsigned نویسه)C);

  برای (size_type i = std::min(From, size()) - 1, e = -1; i != e; --i)
    اگر (!CharBits.test((unsigned نویسه)data()[i]))
      بازگشت i;
  بازگشت npos;
}

تهی StringRef::split(SmallVectorImpl<StringRef> &A,
                      StringRef Separator, صحیح MaxSplit,
                      منطقی KeepEmpty) ثابت {
  StringRef S = *این;

  // Count down from MaxSplit. When MaxSplit is -1, this will just split
  // "forever". This doesn't support splitting more than 2^31 times
  // intentionally; if we ever want that we can make MaxSplit a 64-bit integer
  // but that seems unlikely to be useful.
  وقتی (MaxSplit-- != 0) {
    size_t Idx = S.find(Separator);
    اگر (Idx == npos)
      بشکن;

    // Push this split.
    اگر (KeepEmpty || Idx > 0)
      A.افزودن_back(S.slice(0, Idx));

    // Jump forward.
    S = S.substr(Idx + Separator.size());
  }

  // Push the tail.
  اگر (KeepEmpty || !S.empty())
    A.افزودن_back(S);
}

تهی StringRef::split(SmallVectorImpl<StringRef> &A, نویسه Separator,
                      صحیح MaxSplit, منطقی KeepEmpty) ثابت {
  StringRef S = *این;

  // Count down from MaxSplit. When MaxSplit is -1, this will just split
  // "forever". This doesn't support splitting more than 2^31 times
  // intentionally; if we ever want that we can make MaxSplit a 64-bit integer
  // but that seems unlikely to be useful.
  وقتی (MaxSplit-- != 0) {
    size_t Idx = S.find(Separator);
    اگر (Idx == npos)
      بشکن;

    // Push this split.
    اگر (KeepEmpty || Idx > 0)
      A.افزودن_back(S.slice(0, Idx));

    // Jump forward.
    S = S.substr(Idx + 1);
  }

  // Push the tail.
  اگر (KeepEmpty || !S.empty())
    A.افزودن_back(S);
}

//===----------------------------------------------------------------------===//
// Helpful Algorithms
//===----------------------------------------------------------------------===//

/// count - Return the number of non-overlapped occurrences of \arg Str in
/// the string.
size_t StringRef::شمار(StringRef Str) ثابت {
  size_t شمار_2 = 0;
  size_t Pos = 0;
  size_t N = Str.size();
  // TODO: For an empty `Str` we return 0 for legacy reasons. Consider changing
  //       this to `Length + 1` which is more in-line with the function
  //       description.
  اگر (!N)
    بازگشت 0;
  وقتی ((Pos = find(Str, Pos)) != npos) {
    ++شمار_2;
    Pos += N;
  }
  بازگشت شمار_2;
}

unsigned llvm::getAutoSenseRadix(StringRef &Str) {
  اگر (Str.empty())
    بازگشت 10;

  اگر (Str.consume_front_insensitive("0x"))
    بازگشت 16;

  اگر (Str.consume_front_insensitive("0b"))
    بازگشت 2;

  اگر (Str.consume_front("0o"))
    بازگشت 8;

  اگر (Str[0] == '0' && Str.size() > 1 && isDigit(Str[1])) {
    Str = Str.substr(1);
    بازگشت 8;
  }

  بازگشت 10;
}

منطقی llvm::consumeUnsignedInteger(StringRef &Str, unsigned Radix,
                                  unsigned long long &Result) {
  // Autosense radix if not specified.
  اگر (Radix == 0)
    Radix = getAutoSenseRadix(Str);

  // Empty strings (after the radix autosense) are invalid.
  اگر (Str.empty()) بازگشت درست;

  // Parse all the bytes of the string given this radix.  Watch for overflow.
  StringRef Str2 = Str;
  Result = 0;
  وقتی (!Str2.empty()) {
    unsigned CharVal;
    اگر (Str2[0] >= '0' && Str2[0] <= '9')
      CharVal = Str2[0] - '0';
    وگرنه اگر (Str2[0] >= 'a' && Str2[0] <= 'z')
      CharVal = Str2[0] - 'a' + 10;
    وگرنه اگر (Str2[0] >= 'A' && Str2[0] <= 'Z')
      CharVal = Str2[0] - 'A' + 10;
    وگرنه
      بشکن;

    // If the parsed value is larger than the integer radix, we cannot
    // consume any more characters.
    اگر (CharVal >= Radix)
      بشکن;

    // Add in this character.
    unsigned long long PrevResult = Result;
    Result = Result * Radix + CharVal;

    // Check for overflow by shifting back and seeing if bits were lost.
    اگر (Result / Radix < PrevResult)
      بازگشت درست;

    Str2 = Str2.substr(1);
  }

  // We consider the operation a failure if no characters were consumed
  // successfully.
  اگر (Str.size() == Str2.size())
    بازگشت درست;

  Str = Str2;
  بازگشت نادرست;
}

منطقی llvm::consumeSignedInteger(StringRef &Str, unsigned Radix,
                                long long &Result) {
  unsigned long long ULLVal;

  // Handle positive strings first.
  اگر (!Str.starts_with("-")) {
    اگر (consumeUnsignedInteger(Str, Radix, ULLVal) ||
        // Check for value so large it overflows a signed value.
        (long long)ULLVal < 0)
      بازگشت درست;
    Result = ULLVal;
    بازگشت نادرست;
  }

  // Get the positive part of the value.
  StringRef Str2 = Str.drop_front(1);
  اگر (consumeUnsignedInteger(Str2, Radix, ULLVal) ||
      // Reject values so large they'd overflow as negative signed, but allow
      // "-0".  This negates the unsigned so that the negative isn't undefined
      // on signed overflow.
      (long long)-ULLVal > 0)
    بازگشت درست;

  Str = Str2;
  Result = -ULLVal;
  بازگشت نادرست;
}

/// GetAsUnsignedInteger - Workhorse method that converts a integer character
/// sequence of radix up to 36 to an unsigned long long value.
منطقی llvm::getAsUnsignedInteger(StringRef Str, unsigned Radix,
                                unsigned long long &Result) {
  اگر (consumeUnsignedInteger(Str, Radix, Result))
    بازگشت درست;

  // For getAsUnsignedInteger, we require the whole string to be consumed or
  // else we consider it a failure.
  بازگشت !Str.empty();
}

منطقی llvm::getAsSignedInteger(StringRef Str, unsigned Radix,
                              long long &Result) {
  اگر (consumeSignedInteger(Str, Radix, Result))
    بازگشت درست;

  // For getAsSignedInteger, we require the whole string to be consumed or else
  // we consider it a failure.
  بازگشت !Str.empty();
}

منطقی StringRef::consumeInteger(unsigned Radix, APInt &Result) {
  StringRef Str = *این;

  // Autosense radix if not specified.
  اگر (Radix == 0)
    Radix = getAutoSenseRadix(Str);

  أكد(Radix > 1 && Radix <= 36);

  // Empty strings (after the radix autosense) are invalid.
  اگر (Str.empty()) بازگشت درست;

  // Skip leading zeroes.  This can be a significant improvement if
  // it means we don't need > 64 bits.
  Str = Str.ltrim('0');

  // If it was nothing but zeroes....
  اگر (Str.empty()) {
    Result = APInt(64, 0);
    *این = Str;
    بازگشت نادرست;
  }

  unsigned Log2Radix = 0;
  وقتی ((1U << Log2Radix) < Radix) Log2Radix++;
  منطقی IsPowerOf2Radix = ((1U << Log2Radix) == Radix);

  // Initialize Result to a reasonable starting width (at least 64 bits),
  // but do not shrink it if it was already larger.
  unsigned BitWidth = std::max(64U, Result.getBitWidth());

  اگر (Result.getBitWidth() < BitWidth) {
    Result = Result.zext(BitWidth);
  }

  APInt RadixAP, CharAP; // unused unless !IsPowerOf2Radix
  اگر (!IsPowerOf2Radix) {
    // These must have the same bit-width as Result.
    RadixAP = APInt(BitWidth, Radix);
    CharAP = APInt(BitWidth, 0);
  }

  // Parse all the bytes of the string given this radix.
  Result = 0;
  وقتی (!Str.empty()) {
    unsigned CharVal;
    اگر (Str[0] >= '0' && Str[0] <= '9')
      CharVal = Str[0]-'0';
    وگرنه اگر (Str[0] >= 'a' && Str[0] <= 'z')
      CharVal = Str[0]-'a'+10;
    وگرنه اگر (Str[0] >= 'A' && Str[0] <= 'Z')
      CharVal = Str[0]-'A'+10;
    وگرنه
      بشکن;

    // If the parsed value is larger than the integer radix, the string is
    // invalid.
    اگر (CharVal >= Radix)
      بشکن;

    // Check if one more digit will overflow, and grow if so.
    اگر (Result.getActiveBits() + Log2Radix > Result.getBitWidth()) {
      unsigned NewWidth = Result.getBitWidth() * 2;

      // Guard against overflow of the NewWidth itself
      اگر (NewWidth < Result.getBitWidth())
        بازگشت درست;

      Result = Result.zext(NewWidth);
      اگر (!IsPowerOf2Radix) {
        RadixAP = RadixAP.zext(NewWidth);
        CharAP = CharAP.zext(NewWidth);
      }
    }

    // Add in this character.
    اگر (IsPowerOf2Radix) {
      Result <<= Log2Radix;
      Result |= CharVal;
    } وگرنه {
      Result *= RadixAP;
      CharAP = CharVal;
      Result += CharAP;
    }

    Str = Str.substr(1);
  }

  // We consider the operation a failure if no characters were consumed
  // successfully.
  اگر (size() == Str.size())
    بازگشت درست;

  *این = Str;
  بازگشت نادرست;
}

منطقی StringRef::getAsInteger(unsigned Radix, APInt &Result) ثابت {
  StringRef Str = *این;
  اگر (Str.consumeInteger(Radix, Result))
    بازگشت درست;

  // For getAsInteger, we require the whole string to be consumed or else we
  // consider it a failure.
  بازگشت !Str.empty();
}

منطقی StringRef::getAsDouble(دوگانه &Result, منطقی AllowInexact) ثابت {
  APFloat F(0.0);
  auto StatusOrErr = F.convertFromString(*این, APFloat::rmNearestTiesToEven);
  اگر (errorToBool(StatusOrErr.takeError()))
    بازگشت درست;

  APFloat::opStatus Status = *StatusOrErr;
  اگر (Status != APFloat::opOK) {
    اگر (!AllowInexact || !(Status & APFloat::opInexact))
      بازگشت درست;
  }

  Result = F.convertToDouble();
  بازگشت نادرست;
}

// Implementation of StringRef hashing.
hash_code llvm::hash_مقدار(StringRef S) { بازگشت hash_combine_range(S); }

unsigned DenseMapInfo<StringRef, تهی>::get_Hash_مقدار(StringRef Val) {
  بازگشت (unsigned)(hash_مقدار(Val));
}
