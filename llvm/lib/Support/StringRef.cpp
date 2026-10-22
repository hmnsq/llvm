//===-- StringRef.cpp - Lightweight String References ---------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#અન્તર્ભાવ "llvm/ADT/StringRef.h"
#અન્તર્ભાવ "llvm/ADT/APFloat.h"
#અન્તર્ભાવ "llvm/ADT/APInt.h"
#અન્તર્ભાવ "llvm/ADT/Hashing.h"
#અન્તર્ભાવ "llvm/ADT/StringExtras.h"
#અન્તર્ભાવ "llvm/ADT/edit_distance.h"
#અન્તર્ભાવ "llvm/Support/Error.h"
#અન્તર્ભાવ <bitset>

using નામાકાશ llvm;

// strncasecmp() is not available on non-POSIX systems, so define an
// alternative function here.
સ્થિર પૂર્ણાંક ascii_strncasecmp(StringRef LHS, StringRef RHS) {
  માટે (auto [LC, RC] : zip_equal(LHS, RHS)) {
    unsigned અક્ષર LHC = toLower(LC);
    unsigned અક્ષર RHC = toLower(RC);
    જો (LHC != RHC)
      પાછા LHC < RHC ? -1 : 1;
  }
  પાછા 0;
}

પૂર્ણાંક StringRef::compare_insensitive(StringRef RHS) અચલ {
  size_t Min = std::min(size(), RHS.size());
  જો (પૂર્ણાંક Res = ascii_strncasecmp(take_front(Min), RHS.take_front(Min)))
    પાછા Res;
  જો (size() == RHS.size())
    પાછા 0;
  પાછા size() < RHS.size() ? -1 : 1;
}

તાર્કિક StringRef::starts_with_insensitive(StringRef Prefix) અચલ {
  પાછા size() >= Prefix.size() &&
         ascii_strncasecmp(take_front(Prefix.size()), Prefix) == 0;
}

તાર્કિક StringRef::ends_with_insensitive(StringRef Suffix) અચલ {
  પાછા size() >= Suffix.size() &&
         ascii_strncasecmp(take_back(Suffix.size()), Suffix) == 0;
}

size_t StringRef::find_insensitive(અક્ષર C, size_t From) અચલ {
  અક્ષર L = toLower(C);
  પાછા find_if([L](અક્ષર D) { પાછા toLower(D) == L; }, From);
}

/// compare_numeric - Compare strings, handle embedded numbers.
પૂર્ણાંક StringRef::compare_numeric(StringRef RHS) અચલ {
  માટે (size_t I = 0, E = std::min(size(), RHS.size()); I != E; ++I) {
    // Check for sequences of digits.
    જો (isDigit(data()[I]) && isDigit(RHS.data()[I])) {
      // The longer sequence of numbers is considered larger.
      // This doesn't really handle prefixed zeros well.
      size_t J;
      માટે (J = I + 1; J != E + 1; ++J) {
        તાર્કિક ld = J < size() && isDigit(data()[J]);
        તાર્કિક rd = J < RHS.size() && isDigit(RHS.data()[J]);
        જો (ld != rd)
          પાછા rd ? -1 : 1;
        જો (!rd)
          તોડો;
      }
      // The two number sequences have the same length (J-I), just memcmp them.
      જો (પૂર્ણાંક Res = compareMemory(data() + I, RHS.data() + I, J - I))
        પાછા Res < 0 ? -1 : 1;
      // Identical number sequences, continue search after the numbers.
      I = J - 1;
      ચાલુ;
    }
    જો (data()[I] != RHS.data()[I])
      પાછા (unsigned અક્ષર)data()[I] < (unsigned અક્ષર)RHS.data()[I] ? -1 : 1;
  }
  જો (size() == RHS.size())
    પાછા 0;
  પાછા size() < RHS.size() ? -1 : 1;
}

// Compute the edit distance between the two given strings.
unsigned StringRef::edit_distance(llvm::StringRef Other,
                                  તાર્કિક AllowReplacements,
                                  unsigned MaxEditDistance) અચલ {
  પાછા llvm::ComputeEditDistance(ArrayRef(data(), size()),
                                   ArrayRef(Other.data(), Other.size()),
                                   AllowReplacements, MaxEditDistance);
}

unsigned llvm::StringRef::edit_distance_insensitive(
    StringRef Other, તાર્કિક AllowReplacements, unsigned MaxEditDistance) અચલ {
  પાછા llvm::ComputeMappedEditDistance(
      ArrayRef(data(), size()), ArrayRef(Other.data(), Other.size()),
      llvm::toLower, AllowReplacements, MaxEditDistance);
}

//===----------------------------------------------------------------------===//
// String Operations
//===----------------------------------------------------------------------===//

std::string StringRef::lower() અચલ {
  પાછા std::string(map_iterator(begin(), toLower),
                     map_iterator(end(), toLower));
}

std::string StringRef::upper() અચલ {
  પાછા std::string(map_iterator(begin(), toUpper),
                     map_iterator(end(), toUpper));
}

//===----------------------------------------------------------------------===//
// String Searching
//===----------------------------------------------------------------------===//


/// find - Search for the first string \arg Str in the string.
///
/// \return - The index of the first occurrence of \arg Str, or npos if not
/// found.
size_t StringRef::find(StringRef Str, size_t From) અચલ {
  જો (From > size())
    પાછા npos;

  અચલ અક્ષર *Start = data() + From;
  size_t Size = size() - From;

  અચલ અક્ષર *Needle = Str.data();
  size_t N = Str.size();
  જો (N == 0)
    પાછા From;
  જો (Size < N)
    પાછા npos;
  જો (N == 1) {
    અચલ અક્ષર *Ptr = (અચલ અક્ષર *)::memchr(Start, Needle[0], Size);
    પાછા Ptr == nullptr ? npos : Ptr - data();
  }

  અચલ અક્ષર *Stop = Start + (Size - N + 1);

  જો (N == 2) {
    // Provide a fast path for newline finding (CRLF case) in InclusionRewriter.
    // Not the most optimized strategy, but getting memcmp inlined should be
    // good enough.
    કરો {
      જો (std::memcmp(Start, Needle, 2) == 0)
        પાછા Start - data();
      ++Start;
    } જ્યારે (Start < Stop);
    પાછા npos;
  }

  // For short haystacks or unsupported needles fall back to the naive algorithm
  જો (Size < 16 || N > 255) {
    કરો {
      જો (std::memcmp(Start, Needle, N) == 0)
        પાછા Start - data();
      ++Start;
    } જ્યારે (Start < Stop);
    પાછા npos;
  }

  // Build the bad char heuristic table, with uint8_t to reduce cache thrashing.
  uint8_t BadCharSkip[256];
  std::memset(BadCharSkip, N, 256);
  માટે (unsigned i = 0; i != N-1; ++i)
    BadCharSkip[(uint8_t)Str[i]] = N-1-i;

  કરો {
    uint8_t Last = Start[N - 1];
    જો (LLVM_UNLIKELY(Last == (uint8_t)Needle[N - 1]))
      જો (std::memcmp(Start, Needle, N - 1) == 0)
        પાછા Start - data();

    // Otherwise skip the appropriate number of bytes.
    Start += BadCharSkip[Last];
  } જ્યારે (Start < Stop);

  પાછા npos;
}

size_t StringRef::find_insensitive(StringRef Str, size_t From) અચલ {
  StringRef This = substr(From);
  જ્યારે (This.size() >= Str.size()) {
    જો (This.starts_with_insensitive(Str))
      પાછા From;
    This = This.drop_front();
    ++From;
  }
  પાછા npos;
}

size_t StringRef::rfind_insensitive(અક્ષર C, size_t From) અચલ {
  From = std::min(From, size());
  size_t i = From;
  જ્યારે (i != 0) {
    --i;
    જો (toLower(data()[i]) == toLower(C))
      પાછા i;
  }
  પાછા npos;
}

/// rfind - Search for the last string \arg Str in the string.
///
/// \return - The index of the last occurrence of \arg Str, or npos if not
/// found.
size_t StringRef::rfind(StringRef Str) અચલ {
  પાછા std::string_view(*આ).rfind(Str);
}

size_t StringRef::rfind_insensitive(StringRef Str) અચલ {
  size_t N = Str.size();
  જો (N > size())
    પાછા npos;
  માટે (size_t i = size() - N + 1, e = 0; i != e;) {
    --i;
    જો (substr(i, N).equals_insensitive(Str))
      પાછા i;
  }
  પાછા npos;
}

/// find_first_of - Find the first character in the string that is in \arg
/// Chars, or npos if not found.
///
/// Note: O(size() + Chars.size())
StringRef::size_type StringRef::find_first_of(StringRef Chars,
                                              size_t From) અચલ {
  std::bitset<1 << CHAR_BIT> CharBits;
  માટે (અક્ષર C : Chars)
    CharBits.set((unsigned અક્ષર)C);

  માટે (size_type i = std::min(From, size()), e = size(); i != e; ++i)
    જો (CharBits.test((unsigned અક્ષર)data()[i]))
      પાછા i;
  પાછા npos;
}

/// find_first_not_of - Find the first character in the string that is not
/// \arg C or npos if not found.
StringRef::size_type StringRef::find_first_not_of(અક્ષર C, size_t From) અચલ {
  પાછા std::string_view(*આ).find_first_not_of(C, From);
}

/// find_first_not_of - Find the first character in the string that is not
/// in the string \arg Chars, or npos if not found.
///
/// Note: O(size() + Chars.size())
StringRef::size_type StringRef::find_first_not_of(StringRef Chars,
                                                  size_t From) અચલ {
  std::bitset<1 << CHAR_BIT> CharBits;
  માટે (અક્ષર C : Chars)
    CharBits.set((unsigned અક્ષર)C);

  માટે (size_type i = std::min(From, size()), e = size(); i != e; ++i)
    જો (!CharBits.test((unsigned અક્ષર)data()[i]))
      પાછા i;
  પાછા npos;
}

/// find_last_of - Find the last character in the string that is in \arg C,
/// or npos if not found.
///
/// Note: O(size() + Chars.size())
StringRef::size_type StringRef::find_last_of(StringRef Chars,
                                             size_t From) અચલ {
  std::bitset<1 << CHAR_BIT> CharBits;
  માટે (અક્ષર C : Chars)
    CharBits.set((unsigned અક્ષર)C);

  માટે (size_type i = std::min(From, size()) - 1, e = -1; i != e; --i)
    જો (CharBits.test((unsigned અક્ષર)data()[i]))
      પાછા i;
  પાછા npos;
}

/// find_last_not_of - Find the last character in the string that is not
/// \arg C, or npos if not found.
StringRef::size_type StringRef::find_last_not_of(અક્ષર C, size_t From) અચલ {
  માટે (size_type i = std::min(From, size()) - 1, e = -1; i != e; --i)
    જો (data()[i] != C)
      પાછા i;
  પાછા npos;
}

/// find_last_not_of - Find the last character in the string that is not in
/// \arg Chars, or npos if not found.
///
/// Note: O(size() + Chars.size())
StringRef::size_type StringRef::find_last_not_of(StringRef Chars,
                                                 size_t From) અચલ {
  std::bitset<1 << CHAR_BIT> CharBits;
  માટે (અક્ષર C : Chars)
    CharBits.set((unsigned અક્ષર)C);

  માટે (size_type i = std::min(From, size()) - 1, e = -1; i != e; --i)
    જો (!CharBits.test((unsigned અક્ષર)data()[i]))
      પાછા i;
  પાછા npos;
}

શૂન્ય StringRef::split(SmallVectorImpl<StringRef> &A,
                      StringRef Separator, પૂર્ણાંક MaxSplit,
                      તાર્કિક KeepEmpty) અચલ {
  StringRef S = *આ;

  // Count down from MaxSplit. When MaxSplit is -1, this will just split
  // "forever". This doesn't support splitting more than 2^31 times
  // intentionally; if we ever want that we can make MaxSplit a 64-bit integer
  // but that seems unlikely to be useful.
  જ્યારે (MaxSplit-- != 0) {
    size_t Idx = S.find(Separator);
    જો (Idx == npos)
      તોડો;

    // Push this split.
    જો (KeepEmpty || Idx > 0)
      A.ધકેલો_back(S.slice(0, Idx));

    // Jump forward.
    S = S.substr(Idx + Separator.size());
  }

  // Push the tail.
  જો (KeepEmpty || !S.empty())
    A.ધકેલો_back(S);
}

શૂન્ય StringRef::split(SmallVectorImpl<StringRef> &A, અક્ષર Separator,
                      પૂર્ણાંક MaxSplit, તાર્કિક KeepEmpty) અચલ {
  StringRef S = *આ;

  // Count down from MaxSplit. When MaxSplit is -1, this will just split
  // "forever". This doesn't support splitting more than 2^31 times
  // intentionally; if we ever want that we can make MaxSplit a 64-bit integer
  // but that seems unlikely to be useful.
  જ્યારે (MaxSplit-- != 0) {
    size_t Idx = S.find(Separator);
    જો (Idx == npos)
      તોડો;

    // Push this split.
    જો (KeepEmpty || Idx > 0)
      A.ધકેલો_back(S.slice(0, Idx));

    // Jump forward.
    S = S.substr(Idx + 1);
  }

  // Push the tail.
  જો (KeepEmpty || !S.empty())
    A.ધકેલો_back(S);
}

//===----------------------------------------------------------------------===//
// Helpful Algorithms
//===----------------------------------------------------------------------===//

/// count - Return the number of non-overlapped occurrences of \arg Str in
/// the string.
size_t StringRef::ગણતરી(StringRef Str) અચલ {
  size_t ગણતરી_2 = 0;
  size_t Pos = 0;
  size_t N = Str.size();
  // TODO: For an empty `Str` we return 0 for legacy reasons. Consider changing
  //       this to `Length + 1` which is more in-line with the function
  //       description.
  જો (!N)
    પાછા 0;
  જ્યારે ((Pos = find(Str, Pos)) != npos) {
    ++ગણતરી_2;
    Pos += N;
  }
  પાછા ગણતરી_2;
}

unsigned llvm::getAutoSenseRadix(StringRef &Str) {
  જો (Str.empty())
    પાછા 10;

  જો (Str.consume_front_insensitive("0x"))
    પાછા 16;

  જો (Str.consume_front_insensitive("0b"))
    પાછા 2;

  જો (Str.consume_front("0o"))
    પાછા 8;

  જો (Str[0] == '0' && Str.size() > 1 && isDigit(Str[1])) {
    Str = Str.substr(1);
    પાછા 8;
  }

  પાછા 10;
}

તાર્કિક llvm::consumeUnsignedInteger(StringRef &Str, unsigned Radix,
                                  unsigned long long &Result) {
  // Autosense radix if not specified.
  જો (Radix == 0)
    Radix = getAutoSenseRadix(Str);

  // Empty strings (after the radix autosense) are invalid.
  જો (Str.empty()) પાછા સાચું;

  // Parse all the bytes of the string given this radix.  Watch for overflow.
  StringRef Str2 = Str;
  Result = 0;
  જ્યારે (!Str2.empty()) {
    unsigned CharVal;
    જો (Str2[0] >= '0' && Str2[0] <= '9')
      CharVal = Str2[0] - '0';
    નહીંતર જો (Str2[0] >= 'a' && Str2[0] <= 'z')
      CharVal = Str2[0] - 'a' + 10;
    નહીંતર જો (Str2[0] >= 'A' && Str2[0] <= 'Z')
      CharVal = Str2[0] - 'A' + 10;
    નહીંતર
      તોડો;

    // If the parsed value is larger than the integer radix, we cannot
    // consume any more characters.
    જો (CharVal >= Radix)
      તોડો;

    // Add in this character.
    unsigned long long PrevResult = Result;
    Result = Result * Radix + CharVal;

    // Check for overflow by shifting back and seeing if bits were lost.
    જો (Result / Radix < PrevResult)
      પાછા સાચું;

    Str2 = Str2.substr(1);
  }

  // We consider the operation a failure if no characters were consumed
  // successfully.
  જો (Str.size() == Str2.size())
    પાછા સાચું;

  Str = Str2;
  પાછા ખોટું;
}

તાર્કિક llvm::consumeSignedInteger(StringRef &Str, unsigned Radix,
                                long long &Result) {
  unsigned long long ULLVal;

  // Handle positive strings first.
  જો (!Str.starts_with("-")) {
    જો (consumeUnsignedInteger(Str, Radix, ULLVal) ||
        // Check for value so large it overflows a signed value.
        (long long)ULLVal < 0)
      પાછા સાચું;
    Result = ULLVal;
    પાછા ખોટું;
  }

  // Get the positive part of the value.
  StringRef Str2 = Str.drop_front(1);
  જો (consumeUnsignedInteger(Str2, Radix, ULLVal) ||
      // Reject values so large they'd overflow as negative signed, but allow
      // "-0".  This negates the unsigned so that the negative isn't undefined
      // on signed overflow.
      (long long)-ULLVal > 0)
    પાછા સાચું;

  Str = Str2;
  Result = -ULLVal;
  પાછા ખોટું;
}

/// GetAsUnsignedInteger - Workhorse method that converts a integer character
/// sequence of radix up to 36 to an unsigned long long value.
તાર્કિક llvm::getAsUnsignedInteger(StringRef Str, unsigned Radix,
                                unsigned long long &Result) {
  જો (consumeUnsignedInteger(Str, Radix, Result))
    પાછા સાચું;

  // For getAsUnsignedInteger, we require the whole string to be consumed or
  // else we consider it a failure.
  પાછા !Str.empty();
}

તાર્કિક llvm::getAsSignedInteger(StringRef Str, unsigned Radix,
                              long long &Result) {
  જો (consumeSignedInteger(Str, Radix, Result))
    પાછા સાચું;

  // For getAsSignedInteger, we require the whole string to be consumed or else
  // we consider it a failure.
  પાછા !Str.empty();
}

તાર્કિક StringRef::consumeInteger(unsigned Radix, APInt &Result) {
  StringRef Str = *આ;

  // Autosense radix if not specified.
  જો (Radix == 0)
    Radix = getAutoSenseRadix(Str);

  પ્રતિજ્ઞા(Radix > 1 && Radix <= 36);

  // Empty strings (after the radix autosense) are invalid.
  જો (Str.empty()) પાછા સાચું;

  // Skip leading zeroes.  This can be a significant improvement if
  // it means we don't need > 64 bits.
  Str = Str.ltrim('0');

  // If it was nothing but zeroes....
  જો (Str.empty()) {
    Result = APInt(64, 0);
    *આ = Str;
    પાછા ખોટું;
  }

  unsigned Log2Radix = 0;
  જ્યારે ((1U << Log2Radix) < Radix) Log2Radix++;
  તાર્કિક IsPowerOf2Radix = ((1U << Log2Radix) == Radix);

  // Initialize Result to a reasonable starting width (at least 64 bits),
  // but do not shrink it if it was already larger.
  unsigned BitWidth = std::max(64U, Result.getBitWidth());

  જો (Result.getBitWidth() < BitWidth) {
    Result = Result.zext(BitWidth);
  }

  APInt RadixAP, CharAP; // unused unless !IsPowerOf2Radix
  જો (!IsPowerOf2Radix) {
    // These must have the same bit-width as Result.
    RadixAP = APInt(BitWidth, Radix);
    CharAP = APInt(BitWidth, 0);
  }

  // Parse all the bytes of the string given this radix.
  Result = 0;
  જ્યારે (!Str.empty()) {
    unsigned CharVal;
    જો (Str[0] >= '0' && Str[0] <= '9')
      CharVal = Str[0]-'0';
    નહીંતર જો (Str[0] >= 'a' && Str[0] <= 'z')
      CharVal = Str[0]-'a'+10;
    નહીંતર જો (Str[0] >= 'A' && Str[0] <= 'Z')
      CharVal = Str[0]-'A'+10;
    નહીંતર
      તોડો;

    // If the parsed value is larger than the integer radix, the string is
    // invalid.
    જો (CharVal >= Radix)
      તોડો;

    // Check if one more digit will overflow, and grow if so.
    જો (Result.getActiveBits() + Log2Radix > Result.getBitWidth()) {
      unsigned NewWidth = Result.getBitWidth() * 2;

      // Guard against overflow of the NewWidth itself
      જો (NewWidth < Result.getBitWidth())
        પાછા સાચું;

      Result = Result.zext(NewWidth);
      જો (!IsPowerOf2Radix) {
        RadixAP = RadixAP.zext(NewWidth);
        CharAP = CharAP.zext(NewWidth);
      }
    }

    // Add in this character.
    જો (IsPowerOf2Radix) {
      Result <<= Log2Radix;
      Result |= CharVal;
    } નહીંતર {
      Result *= RadixAP;
      CharAP = CharVal;
      Result += CharAP;
    }

    Str = Str.substr(1);
  }

  // We consider the operation a failure if no characters were consumed
  // successfully.
  જો (size() == Str.size())
    પાછા સાચું;

  *આ = Str;
  પાછા ખોટું;
}

તાર્કિક StringRef::getAsInteger(unsigned Radix, APInt &Result) અચલ {
  StringRef Str = *આ;
  જો (Str.consumeInteger(Radix, Result))
    પાછા સાચું;

  // For getAsInteger, we require the whole string to be consumed or else we
  // consider it a failure.
  પાછા !Str.empty();
}

તાર્કિક StringRef::getAsDouble(દ્વિગુણ &Result, તાર્કિક AllowInexact) અચલ {
  APFloat F(0.0);
  auto StatusOrErr = F.convertFromString(*આ, APFloat::rmNearestTiesToEven);
  જો (errorToBool(StatusOrErr.takeError()))
    પાછા સાચું;

  APFloat::opStatus Status = *StatusOrErr;
  જો (Status != APFloat::opOK) {
    જો (!AllowInexact || !(Status & APFloat::opInexact))
      પાછા સાચું;
  }

  Result = F.convertToDouble();
  પાછા ખોટું;
}

// Implementation of StringRef hashing.
hash_code llvm::hash_કિંમત(StringRef S) { પાછા hash_combine_range(S); }

unsigned DenseMapInfo<StringRef, શૂન્ય>::get_Hash_કિંમત(StringRef Val) {
  પાછા (unsigned)(hash_કિંમત(Val));
}
