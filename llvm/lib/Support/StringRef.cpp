//===-- StringRef.cpp - Lightweight String References ---------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#ضمن "llvm/ADT/StringRef.h"
#ضمن "llvm/ADT/APFloat.h"
#ضمن "llvm/ADT/APInt.h"
#ضمن "llvm/ADT/Hashing.h"
#ضمن "llvm/ADT/StringExtras.h"
#ضمن "llvm/ADT/edit_distance.h"
#ضمن "llvm/Support/Error.h"
#ضمن <bitset>

using نطاق llvm;

// strncasecmp() is not available on non-POSIX systems, so define an
// alternative function here.
ساكن صحيح ascii_strncasecmp(StringRef LHS, StringRef RHS) {
  لكل (auto [LC, RC] : zip_equal(LHS, RHS)) {
    unsigned حرف LHC = toLower(LC);
    unsigned حرف RHC = toLower(RC);
    إذا (LHC != RHC)
      إرجاع LHC < RHC ? -1 : 1;
  }
  إرجاع 0;
}

صحيح StringRef::compare_insensitive(StringRef RHS) ثابت {
  size_t Min = std::min(size(), RHS.size());
  إذا (صحيح Res = ascii_strncasecmp(take_front(Min), RHS.take_front(Min)))
    إرجاع Res;
  إذا (size() == RHS.size())
    إرجاع 0;
  إرجاع size() < RHS.size() ? -1 : 1;
}

منطقي StringRef::starts_with_insensitive(StringRef Prefix) ثابت {
  إرجاع size() >= Prefix.size() &&
         ascii_strncasecmp(take_front(Prefix.size()), Prefix) == 0;
}

منطقي StringRef::ends_with_insensitive(StringRef Suffix) ثابت {
  إرجاع size() >= Suffix.size() &&
         ascii_strncasecmp(take_back(Suffix.size()), Suffix) == 0;
}

size_t StringRef::find_insensitive(حرف C, size_t From) ثابت {
  حرف L = toLower(C);
  إرجاع find_if([L](حرف D) { إرجاع toLower(D) == L; }, From);
}

/// compare_numeric - Compare strings, handle embedded numbers.
صحيح StringRef::compare_numeric(StringRef RHS) ثابت {
  لكل (size_t I = 0, E = std::min(size(), RHS.size()); I != E; ++I) {
    // Check for sequences of digits.
    إذا (isDigit(data()[I]) && isDigit(RHS.data()[I])) {
      // The longer sequence of numbers is considered larger.
      // This doesn't really handle prefixed zeros well.
      size_t J;
      لكل (J = I + 1; J != E + 1; ++J) {
        منطقي ld = J < size() && isDigit(data()[J]);
        منطقي rd = J < RHS.size() && isDigit(RHS.data()[J]);
        إذا (ld != rd)
          إرجاع rd ? -1 : 1;
        إذا (!rd)
          اكسر;
      }
      // The two number sequences have the same length (J-I), just memcmp them.
      إذا (صحيح Res = compareMemory(data() + I, RHS.data() + I, J - I))
        إرجاع Res < 0 ? -1 : 1;
      // Identical number sequences, continue search after the numbers.
      I = J - 1;
      استمر;
    }
    إذا (data()[I] != RHS.data()[I])
      إرجاع (unsigned حرف)data()[I] < (unsigned حرف)RHS.data()[I] ? -1 : 1;
  }
  إذا (size() == RHS.size())
    إرجاع 0;
  إرجاع size() < RHS.size() ? -1 : 1;
}

// Compute the edit distance between the two given strings.
unsigned StringRef::edit_distance(llvm::StringRef Other,
                                  منطقي AllowReplacements,
                                  unsigned MaxEditDistance) ثابت {
  إرجاع llvm::ComputeEditDistance(ArrayRef(data(), size()),
                                   ArrayRef(Other.data(), Other.size()),
                                   AllowReplacements, MaxEditDistance);
}

unsigned llvm::StringRef::edit_distance_insensitive(
    StringRef Other, منطقي AllowReplacements, unsigned MaxEditDistance) ثابت {
  إرجاع llvm::ComputeMappedEditDistance(
      ArrayRef(data(), size()), ArrayRef(Other.data(), Other.size()),
      llvm::toLower, AllowReplacements, MaxEditDistance);
}

//===----------------------------------------------------------------------===//
// String Operations
//===----------------------------------------------------------------------===//

std::string StringRef::lower() ثابت {
  إرجاع std::string(map_iterator(begin(), toLower),
                     map_iterator(end(), toLower));
}

std::string StringRef::upper() ثابت {
  إرجاع std::string(map_iterator(begin(), toUpper),
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
  إذا (From > size())
    إرجاع npos;

  ثابت حرف *Start = data() + From;
  size_t Size = size() - From;

  ثابت حرف *Needle = Str.data();
  size_t N = Str.size();
  إذا (N == 0)
    إرجاع From;
  إذا (Size < N)
    إرجاع npos;
  إذا (N == 1) {
    ثابت حرف *Ptr = (ثابت حرف *)::memchr(Start, Needle[0], Size);
    إرجاع Ptr == nullptr ? npos : Ptr - data();
  }

  ثابت حرف *Stop = Start + (Size - N + 1);

  إذا (N == 2) {
    // Provide a fast path for newline finding (CRLF case) in InclusionRewriter.
    // Not the most optimized strategy, but getting memcmp inlined should be
    // good enough.
    افعل {
      إذا (std::memcmp(Start, Needle, 2) == 0)
        إرجاع Start - data();
      ++Start;
    } بينما (Start < Stop);
    إرجاع npos;
  }

  // For short haystacks or unsupported needles fall back to the naive algorithm
  إذا (Size < 16 || N > 255) {
    افعل {
      إذا (std::memcmp(Start, Needle, N) == 0)
        إرجاع Start - data();
      ++Start;
    } بينما (Start < Stop);
    إرجاع npos;
  }

  // Build the bad char heuristic table, with uint8_t to reduce cache thrashing.
  uint8_t BadCharSkip[256];
  std::memset(BadCharSkip, N, 256);
  لكل (unsigned i = 0; i != N-1; ++i)
    BadCharSkip[(uint8_t)Str[i]] = N-1-i;

  افعل {
    uint8_t Last = Start[N - 1];
    إذا (LLVM_UNLIKELY(Last == (uint8_t)Needle[N - 1]))
      إذا (std::memcmp(Start, Needle, N - 1) == 0)
        إرجاع Start - data();

    // Otherwise skip the appropriate number of bytes.
    Start += BadCharSkip[Last];
  } بينما (Start < Stop);

  إرجاع npos;
}

size_t StringRef::find_insensitive(StringRef Str, size_t From) ثابت {
  StringRef This = substr(From);
  بينما (This.size() >= Str.size()) {
    إذا (This.starts_with_insensitive(Str))
      إرجاع From;
    This = This.drop_front();
    ++From;
  }
  إرجاع npos;
}

size_t StringRef::rfind_insensitive(حرف C, size_t From) ثابت {
  From = std::min(From, size());
  size_t i = From;
  بينما (i != 0) {
    --i;
    إذا (toLower(data()[i]) == toLower(C))
      إرجاع i;
  }
  إرجاع npos;
}

/// rfind - Search for the last string \arg Str in the string.
///
/// \return - The index of the last occurrence of \arg Str, or npos if not
/// found.
size_t StringRef::rfind(StringRef Str) ثابت {
  إرجاع std::string_view(*هذا).rfind(Str);
}

size_t StringRef::rfind_insensitive(StringRef Str) ثابت {
  size_t N = Str.size();
  إذا (N > size())
    إرجاع npos;
  لكل (size_t i = size() - N + 1, e = 0; i != e;) {
    --i;
    إذا (substr(i, N).equals_insensitive(Str))
      إرجاع i;
  }
  إرجاع npos;
}

/// find_first_of - Find the first character in the string that is in \arg
/// Chars, or npos if not found.
///
/// Note: O(size() + Chars.size())
StringRef::size_type StringRef::find_first_of(StringRef Chars,
                                              size_t From) ثابت {
  std::bitset<1 << CHAR_BIT> CharBits;
  لكل (حرف C : Chars)
    CharBits.set((unsigned حرف)C);

  لكل (size_type i = std::min(From, size()), e = size(); i != e; ++i)
    إذا (CharBits.test((unsigned حرف)data()[i]))
      إرجاع i;
  إرجاع npos;
}

/// find_first_not_of - Find the first character in the string that is not
/// \arg C or npos if not found.
StringRef::size_type StringRef::find_first_not_of(حرف C, size_t From) ثابت {
  إرجاع std::string_view(*هذا).find_first_not_of(C, From);
}

/// find_first_not_of - Find the first character in the string that is not
/// in the string \arg Chars, or npos if not found.
///
/// Note: O(size() + Chars.size())
StringRef::size_type StringRef::find_first_not_of(StringRef Chars,
                                                  size_t From) ثابت {
  std::bitset<1 << CHAR_BIT> CharBits;
  لكل (حرف C : Chars)
    CharBits.set((unsigned حرف)C);

  لكل (size_type i = std::min(From, size()), e = size(); i != e; ++i)
    إذا (!CharBits.test((unsigned حرف)data()[i]))
      إرجاع i;
  إرجاع npos;
}

/// find_last_of - Find the last character in the string that is in \arg C,
/// or npos if not found.
///
/// Note: O(size() + Chars.size())
StringRef::size_type StringRef::find_last_of(StringRef Chars,
                                             size_t From) ثابت {
  std::bitset<1 << CHAR_BIT> CharBits;
  لكل (حرف C : Chars)
    CharBits.set((unsigned حرف)C);

  لكل (size_type i = std::min(From, size()) - 1, e = -1; i != e; --i)
    إذا (CharBits.test((unsigned حرف)data()[i]))
      إرجاع i;
  إرجاع npos;
}

/// find_last_not_of - Find the last character in the string that is not
/// \arg C, or npos if not found.
StringRef::size_type StringRef::find_last_not_of(حرف C, size_t From) ثابت {
  لكل (size_type i = std::min(From, size()) - 1, e = -1; i != e; --i)
    إذا (data()[i] != C)
      إرجاع i;
  إرجاع npos;
}

/// find_last_not_of - Find the last character in the string that is not in
/// \arg Chars, or npos if not found.
///
/// Note: O(size() + Chars.size())
StringRef::size_type StringRef::find_last_not_of(StringRef Chars,
                                                 size_t From) ثابت {
  std::bitset<1 << CHAR_BIT> CharBits;
  لكل (حرف C : Chars)
    CharBits.set((unsigned حرف)C);

  لكل (size_type i = std::min(From, size()) - 1, e = -1; i != e; --i)
    إذا (!CharBits.test((unsigned حرف)data()[i]))
      إرجاع i;
  إرجاع npos;
}

فراغ StringRef::split(SmallVectorImpl<StringRef> &A,
                      StringRef Separator, صحيح MaxSplit,
                      منطقي KeepEmpty) ثابت {
  StringRef S = *هذا;

  // Count down from MaxSplit. When MaxSplit is -1, this will just split
  // "forever". This doesn't support splitting more than 2^31 times
  // intentionally; if we ever want that we can make MaxSplit a 64-bit integer
  // but that seems unlikely to be useful.
  بينما (MaxSplit-- != 0) {
    size_t Idx = S.find(Separator);
    إذا (Idx == npos)
      اكسر;

    // Push this split.
    إذا (KeepEmpty || Idx > 0)
      A.ادفع_back(S.slice(0, Idx));

    // Jump forward.
    S = S.substr(Idx + Separator.size());
  }

  // Push the tail.
  إذا (KeepEmpty || !S.empty())
    A.ادفع_back(S);
}

فراغ StringRef::split(SmallVectorImpl<StringRef> &A, حرف Separator,
                      صحيح MaxSplit, منطقي KeepEmpty) ثابت {
  StringRef S = *هذا;

  // Count down from MaxSplit. When MaxSplit is -1, this will just split
  // "forever". This doesn't support splitting more than 2^31 times
  // intentionally; if we ever want that we can make MaxSplit a 64-bit integer
  // but that seems unlikely to be useful.
  بينما (MaxSplit-- != 0) {
    size_t Idx = S.find(Separator);
    إذا (Idx == npos)
      اكسر;

    // Push this split.
    إذا (KeepEmpty || Idx > 0)
      A.ادفع_back(S.slice(0, Idx));

    // Jump forward.
    S = S.substr(Idx + 1);
  }

  // Push the tail.
  إذا (KeepEmpty || !S.empty())
    A.ادفع_back(S);
}

//===----------------------------------------------------------------------===//
// Helpful Algorithms
//===----------------------------------------------------------------------===//

/// count - Return the number of non-overlapped occurrences of \arg Str in
/// the string.
size_t StringRef::تعداد_2(StringRef Str) ثابت {
  size_t تعداد_3 = 0;
  size_t Pos = 0;
  size_t N = Str.size();
  // TODO: For an empty `Str` we return 0 for legacy reasons. Consider changing
  //       this to `Length + 1` which is more in-line with the function
  //       description.
  إذا (!N)
    إرجاع 0;
  بينما ((Pos = find(Str, Pos)) != npos) {
    ++تعداد_3;
    Pos += N;
  }
  إرجاع تعداد_3;
}

unsigned llvm::getAutoSenseRadix(StringRef &Str) {
  إذا (Str.empty())
    إرجاع 10;

  إذا (Str.consume_front_insensitive("0x"))
    إرجاع 16;

  إذا (Str.consume_front_insensitive("0b"))
    إرجاع 2;

  إذا (Str.consume_front("0o"))
    إرجاع 8;

  إذا (Str[0] == '0' && Str.size() > 1 && isDigit(Str[1])) {
    Str = Str.substr(1);
    إرجاع 8;
  }

  إرجاع 10;
}

منطقي llvm::consumeUnsignedInteger(StringRef &Str, unsigned Radix,
                                  unsigned long long &Result) {
  // Autosense radix if not specified.
  إذا (Radix == 0)
    Radix = getAutoSenseRadix(Str);

  // Empty strings (after the radix autosense) are invalid.
  إذا (Str.empty()) إرجاع true;

  // Parse all the bytes of the string given this radix.  Watch for overflow.
  StringRef Str2 = Str;
  Result = 0;
  بينما (!Str2.empty()) {
    unsigned CharVal;
    إذا (Str2[0] >= '0' && Str2[0] <= '9')
      CharVal = Str2[0] - '0';
    وإلا إذا (Str2[0] >= 'a' && Str2[0] <= 'z')
      CharVal = Str2[0] - 'a' + 10;
    وإلا إذا (Str2[0] >= 'A' && Str2[0] <= 'Z')
      CharVal = Str2[0] - 'A' + 10;
    وإلا
      اكسر;

    // If the parsed value is larger than the integer radix, we cannot
    // consume any more characters.
    إذا (CharVal >= Radix)
      اكسر;

    // Add in this character.
    unsigned long long PrevResult = Result;
    Result = Result * Radix + CharVal;

    // Check for overflow by shifting back and seeing if bits were lost.
    إذا (Result / Radix < PrevResult)
      إرجاع true;

    Str2 = Str2.substr(1);
  }

  // We consider the operation a failure if no characters were consumed
  // successfully.
  إذا (Str.size() == Str2.size())
    إرجاع true;

  Str = Str2;
  إرجاع خطأ;
}

منطقي llvm::consumeSignedInteger(StringRef &Str, unsigned Radix,
                                long long &Result) {
  unsigned long long ULLVal;

  // Handle positive strings first.
  إذا (!Str.starts_with("-")) {
    إذا (consumeUnsignedInteger(Str, Radix, ULLVal) ||
        // Check for value so large it overflows a signed value.
        (long long)ULLVal < 0)
      إرجاع true;
    Result = ULLVal;
    إرجاع خطأ;
  }

  // Get the positive part of the value.
  StringRef Str2 = Str.drop_front(1);
  إذا (consumeUnsignedInteger(Str2, Radix, ULLVal) ||
      // Reject values so large they'd overflow as negative signed, but allow
      // "-0".  This negates the unsigned so that the negative isn't undefined
      // on signed overflow.
      (long long)-ULLVal > 0)
    إرجاع true;

  Str = Str2;
  Result = -ULLVal;
  إرجاع خطأ;
}

/// GetAsUnsignedInteger - Workhorse method that converts a integer character
/// sequence of radix up to 36 to an unsigned long long value.
منطقي llvm::getAsUnsignedInteger(StringRef Str, unsigned Radix,
                                unsigned long long &Result) {
  إذا (consumeUnsignedInteger(Str, Radix, Result))
    إرجاع true;

  // For getAsUnsignedInteger, we require the whole string to be consumed or
  // else we consider it a failure.
  إرجاع !Str.empty();
}

منطقي llvm::getAsSignedInteger(StringRef Str, unsigned Radix,
                              long long &Result) {
  إذا (consumeSignedInteger(Str, Radix, Result))
    إرجاع true;

  // For getAsSignedInteger, we require the whole string to be consumed or else
  // we consider it a failure.
  إرجاع !Str.empty();
}

منطقي StringRef::consumeInteger(unsigned Radix, APInt &Result) {
  StringRef Str = *هذا;

  // Autosense radix if not specified.
  إذا (Radix == 0)
    Radix = getAutoSenseRadix(Str);

  أكد(Radix > 1 && Radix <= 36);

  // Empty strings (after the radix autosense) are invalid.
  إذا (Str.empty()) إرجاع true;

  // Skip leading zeroes.  This can be a significant improvement if
  // it means we don't need > 64 bits.
  Str = Str.ltrim('0');

  // If it was nothing but zeroes....
  إذا (Str.empty()) {
    Result = APInt(64, 0);
    *هذا = Str;
    إرجاع خطأ;
  }

  unsigned Log2Radix = 0;
  بينما ((1U << Log2Radix) < Radix) Log2Radix++;
  منطقي IsPowerOf2Radix = ((1U << Log2Radix) == Radix);

  // Initialize Result to a reasonable starting width (at least 64 bits),
  // but do not shrink it if it was already larger.
  unsigned BitWidth = std::max(64U, Result.getBitWidth());

  إذا (Result.getBitWidth() < BitWidth) {
    Result = Result.zext(BitWidth);
  }

  APInt RadixAP, CharAP; // unused unless !IsPowerOf2Radix
  إذا (!IsPowerOf2Radix) {
    // These must have the same bit-width as Result.
    RadixAP = APInt(BitWidth, Radix);
    CharAP = APInt(BitWidth, 0);
  }

  // Parse all the bytes of the string given this radix.
  Result = 0;
  بينما (!Str.empty()) {
    unsigned CharVal;
    إذا (Str[0] >= '0' && Str[0] <= '9')
      CharVal = Str[0]-'0';
    وإلا إذا (Str[0] >= 'a' && Str[0] <= 'z')
      CharVal = Str[0]-'a'+10;
    وإلا إذا (Str[0] >= 'A' && Str[0] <= 'Z')
      CharVal = Str[0]-'A'+10;
    وإلا
      اكسر;

    // If the parsed value is larger than the integer radix, the string is
    // invalid.
    إذا (CharVal >= Radix)
      اكسر;

    // Check if one more digit will overflow, and grow if so.
    إذا (Result.getActiveBits() + Log2Radix > Result.getBitWidth()) {
      unsigned NewWidth = Result.getBitWidth() * 2;

      // Guard against overflow of the NewWidth itself
      إذا (NewWidth < Result.getBitWidth())
        إرجاع true;

      Result = Result.zext(NewWidth);
      إذا (!IsPowerOf2Radix) {
        RadixAP = RadixAP.zext(NewWidth);
        CharAP = CharAP.zext(NewWidth);
      }
    }

    // Add in this character.
    إذا (IsPowerOf2Radix) {
      Result <<= Log2Radix;
      Result |= CharVal;
    } وإلا {
      Result *= RadixAP;
      CharAP = CharVal;
      Result += CharAP;
    }

    Str = Str.substr(1);
  }

  // We consider the operation a failure if no characters were consumed
  // successfully.
  إذا (size() == Str.size())
    إرجاع true;

  *هذا = Str;
  إرجاع خطأ;
}

منطقي StringRef::getAsInteger(unsigned Radix, APInt &Result) ثابت {
  StringRef Str = *هذا;
  إذا (Str.consumeInteger(Radix, Result))
    إرجاع true;

  // For getAsInteger, we require the whole string to be consumed or else we
  // consider it a failure.
  إرجاع !Str.empty();
}

منطقي StringRef::getAsDouble(مزدوج &Result, منطقي AllowInexact) ثابت {
  APFloat F(0.0);
  auto StatusOrErr = F.convertFromString(*هذا, APFloat::rmNearestTiesToEven);
  إذا (errorToBool(StatusOrErr.takeError()))
    إرجاع true;

  APFloat::opStatus Status = *StatusOrErr;
  إذا (Status != APFloat::opOK) {
    إذا (!AllowInexact || !(Status & APFloat::opInexact))
      إرجاع true;
  }

  Result = F.convertToDouble();
  إرجاع خطأ;
}

// Implementation of StringRef hashing.
hash_code llvm::hash_قيمة(StringRef S) { إرجاع hash_combine_range(S); }

unsigned DenseMapInfo<StringRef, فراغ>::get_Hash_قيمة(StringRef Val) {
  إرجاع (unsigned)(hash_قيمة(Val));
}
