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
  लागि (स्वतः [LC, RC] : zip_equal(LHS, RHS)) {
    अचिन्हित वर्ण LHC = toLower(LC);
    अचिन्हित वर्ण RHC = toLower(RC);
    यदि (LHC != RHC)
      फर्काऊ LHC < RHC ? -1 : 1;
  }
  फर्काऊ 0;
}

पूर्णाङ्क StringRef::compare_insensitive(StringRef RHS) अचर {
  माप_प्रकार Min = std::min(size(), RHS.size());
  यदि (पूर्णाङ्क Res = ascii_strncasecmp(take_front(Min), RHS.take_front(Min)))
    फर्काऊ Res;
  यदि (size() == RHS.size())
    फर्काऊ 0;
  फर्काऊ size() < RHS.size() ? -1 : 1;
}

तार्किक StringRef::starts_with_insensitive(StringRef Prefix) अचर {
  फर्काऊ size() >= Prefix.size() &&
         ascii_strncasecmp(take_front(Prefix.size()), Prefix) == 0;
}

तार्किक StringRef::ends_with_insensitive(StringRef Suffix) अचर {
  फर्काऊ size() >= Suffix.size() &&
         ascii_strncasecmp(take_back(Suffix.size()), Suffix) == 0;
}

माप_प्रकार StringRef::find_insensitive(वर्ण C, माप_प्रकार From) अचर {
  वर्ण L = toLower(C);
  फर्काऊ find_if([L](वर्ण D) { फर्काऊ toLower(D) == L; }, From);
}

/// compare_numeric - Compare strings, handle embedded numbers.
पूर्णाङ्क StringRef::compare_numeric(StringRef RHS) अचर {
  लागि (माप_प्रकार I = 0, E = std::min(size(), RHS.size()); I != E; ++I) {
    // Check for sequences of digits.
    यदि (isDigit(data()[I]) && isDigit(RHS.data()[I])) {
      // The longer sequence of numbers is considered larger.
      // This doesn't really handle prefixed zeros well.
      माप_प्रकार J;
      लागि (J = I + 1; J != E + 1; ++J) {
        तार्किक ld = J < size() && isDigit(data()[J]);
        तार्किक rd = J < RHS.size() && isDigit(RHS.data()[J]);
        यदि (ld != rd)
          फर्काऊ rd ? -1 : 1;
        यदि (!rd)
          रोक;
      }
      // The two number sequences have the same length (J-I), just memcmp them.
      यदि (पूर्णाङ्क Res = compareMemory(data() + I, RHS.data() + I, J - I))
        फर्काऊ Res < 0 ? -1 : 1;
      // Identical number sequences, continue search after the numbers.
      I = J - 1;
      जारी;
    }
    यदि (data()[I] != RHS.data()[I])
      फर्काऊ (अचिन्हित वर्ण)data()[I] < (अचिन्हित वर्ण)RHS.data()[I] ? -1 : 1;
  }
  यदि (size() == RHS.size())
    फर्काऊ 0;
  फर्काऊ size() < RHS.size() ? -1 : 1;
}

// Compute the edit distance between the two given strings.
अचिन्हित StringRef::edit_distance(llvm::StringRef Other,
                                  तार्किक AllowReplacements,
                                  अचिन्हित MaxEditDistance) अचर {
  फर्काऊ llvm::ComputeEditDistance(ArrayRef(data(), size()),
                                   ArrayRef(Other.data(), Other.size()),
                                   AllowReplacements, MaxEditDistance);
}

अचिन्हित llvm::StringRef::edit_distance_insensitive(
    StringRef Other, तार्किक AllowReplacements, अचिन्हित MaxEditDistance) अचर {
  फर्काऊ llvm::ComputeMappedEditDistance(
      ArrayRef(data(), size()), ArrayRef(Other.data(), Other.size()),
      llvm::toLower, AllowReplacements, MaxEditDistance);
}

//===----------------------------------------------------------------------===//
// String Operations
//===----------------------------------------------------------------------===//

std::string StringRef::lower() अचर {
  फर्काऊ std::string(map_iterator(begin(), toLower),
                     map_iterator(end(), toLower));
}

std::string StringRef::upper() अचर {
  फर्काऊ std::string(map_iterator(begin(), toUpper),
                     map_iterator(end(), toUpper));
}

//===----------------------------------------------------------------------===//
// String Searching
//===----------------------------------------------------------------------===//


/// find - Search for the first string \arg Str in the string.
///
/// \return - The index of the first occurrence of \arg Str, or npos if not
/// found.
माप_प्रकार StringRef::find(StringRef Str, माप_प्रकार From) अचर {
  यदि (From > size())
    फर्काऊ npos;

  अचर वर्ण *Start = data() + From;
  माप_प्रकार Size = size() - From;

  अचर वर्ण *Needle = Str.data();
  माप_प्रकार N = Str.size();
  यदि (N == 0)
    फर्काऊ From;
  यदि (Size < N)
    फर्काऊ npos;
  यदि (N == 1) {
    अचर वर्ण *Ptr = (अचर वर्ण *)::स_प्रथम(Start, Needle[0], Size);
    फर्काऊ Ptr == nullptr ? npos : Ptr - data();
  }

  अचर वर्ण *Stop = Start + (Size - N + 1);

  यदि (N == 2) {
    // Provide a fast path for newline finding (CRLF case) in InclusionRewriter.
    // Not the most optimized strategy, but getting memcmp inlined should be
    // good enough.
    गर {
      यदि (std::स_भेद(Start, Needle, 2) == 0)
        फर्काऊ Start - data();
      ++Start;
    } जबसम्म (Start < Stop);
    फर्काऊ npos;
  }

  // For short haystacks or unsupported needles fall back to the naive algorithm
  यदि (Size < 16 || N > 255) {
    गर {
      यदि (std::स_भेद(Start, Needle, N) == 0)
        फर्काऊ Start - data();
      ++Start;
    } जबसम्म (Start < Stop);
    फर्काऊ npos;
  }

  // Build the bad char heuristic table, with uint8_t to reduce cache thrashing.
  uint8_t BadCharSkip[256];
  std::स_रखो(BadCharSkip, N, 256);
  लागि (अचिन्हित i = 0; i != N-1; ++i)
    BadCharSkip[(uint8_t)Str[i]] = N-1-i;

  गर {
    uint8_t Last = Start[N - 1];
    यदि (LLVM_UNLIKELY(Last == (uint8_t)Needle[N - 1]))
      यदि (std::स_भेद(Start, Needle, N - 1) == 0)
        फर्काऊ Start - data();

    // Otherwise skip the appropriate number of bytes.
    Start += BadCharSkip[Last];
  } जबसम्म (Start < Stop);

  फर्काऊ npos;
}

माप_प्रकार StringRef::find_insensitive(StringRef Str, माप_प्रकार From) अचर {
  StringRef This = substr(From);
  जबसम्म (This.size() >= Str.size()) {
    यदि (This.starts_with_insensitive(Str))
      फर्काऊ From;
    This = This.drop_front();
    ++From;
  }
  फर्काऊ npos;
}

माप_प्रकार StringRef::rfind_insensitive(वर्ण C, माप_प्रकार From) अचर {
  From = std::min(From, size());
  माप_प्रकार i = From;
  जबसम्म (i != 0) {
    --i;
    यदि (toLower(data()[i]) == toLower(C))
      फर्काऊ i;
  }
  फर्काऊ npos;
}

/// rfind - Search for the last string \arg Str in the string.
///
/// \return - The index of the last occurrence of \arg Str, or npos if not
/// found.
माप_प्रकार StringRef::rfind(StringRef Str) अचर {
  फर्काऊ std::string_view(*एतत्).rfind(Str);
}

माप_प्रकार StringRef::rfind_insensitive(StringRef Str) अचर {
  माप_प्रकार N = Str.size();
  यदि (N > size())
    फर्काऊ npos;
  लागि (माप_प्रकार i = size() - N + 1, e = 0; i != e;) {
    --i;
    यदि (substr(i, N).equals_insensitive(Str))
      फर्काऊ i;
  }
  फर्काऊ npos;
}

/// find_first_of - Find the first character in the string that is in \arg
/// Chars, or npos if not found.
///
/// Note: O(size() + Chars.size())
StringRef::size_type StringRef::find_first_of(StringRef Chars,
                                              माप_प्रकार From) अचर {
  std::bitset<1 << अक्षर_बिट> CharBits;
  लागि (वर्ण C : Chars)
    CharBits.set((अचिन्हित वर्ण)C);

  लागि (size_type i = std::min(From, size()), e = size(); i != e; ++i)
    यदि (CharBits.test((अचिन्हित वर्ण)data()[i]))
      फर्काऊ i;
  फर्काऊ npos;
}

/// find_first_not_of - Find the first character in the string that is not
/// \arg C or npos if not found.
StringRef::size_type StringRef::find_first_not_of(वर्ण C, माप_प्रकार From) अचर {
  फर्काऊ std::string_view(*एतत्).find_first_not_of(C, From);
}

/// find_first_not_of - Find the first character in the string that is not
/// in the string \arg Chars, or npos if not found.
///
/// Note: O(size() + Chars.size())
StringRef::size_type StringRef::find_first_not_of(StringRef Chars,
                                                  माप_प्रकार From) अचर {
  std::bitset<1 << अक्षर_बिट> CharBits;
  लागि (वर्ण C : Chars)
    CharBits.set((अचिन्हित वर्ण)C);

  लागि (size_type i = std::min(From, size()), e = size(); i != e; ++i)
    यदि (!CharBits.test((अचिन्हित वर्ण)data()[i]))
      फर्काऊ i;
  फर्काऊ npos;
}

/// find_last_of - Find the last character in the string that is in \arg C,
/// or npos if not found.
///
/// Note: O(size() + Chars.size())
StringRef::size_type StringRef::find_last_of(StringRef Chars,
                                             माप_प्रकार From) अचर {
  std::bitset<1 << अक्षर_बिट> CharBits;
  लागि (वर्ण C : Chars)
    CharBits.set((अचिन्हित वर्ण)C);

  लागि (size_type i = std::min(From, size()) - 1, e = -1; i != e; --i)
    यदि (CharBits.test((अचिन्हित वर्ण)data()[i]))
      फर्काऊ i;
  फर्काऊ npos;
}

/// find_last_not_of - Find the last character in the string that is not
/// \arg C, or npos if not found.
StringRef::size_type StringRef::find_last_not_of(वर्ण C, माप_प्रकार From) अचर {
  लागि (size_type i = std::min(From, size()) - 1, e = -1; i != e; --i)
    यदि (data()[i] != C)
      फर्काऊ i;
  फर्काऊ npos;
}

/// find_last_not_of - Find the last character in the string that is not in
/// \arg Chars, or npos if not found.
///
/// Note: O(size() + Chars.size())
StringRef::size_type StringRef::find_last_not_of(StringRef Chars,
                                                 माप_प्रकार From) अचर {
  std::bitset<1 << अक्षर_बिट> CharBits;
  लागि (वर्ण C : Chars)
    CharBits.set((अचिन्हित वर्ण)C);

  लागि (size_type i = std::min(From, size()) - 1, e = -1; i != e; --i)
    यदि (!CharBits.test((अचिन्हित वर्ण)data()[i]))
      फर्काऊ i;
  फर्काऊ npos;
}

शून्य StringRef::split(SmallVectorImpl<StringRef> &A,
                      StringRef Separator, पूर्णाङ्क MaxSplit,
                      तार्किक KeepEmpty) अचर {
  StringRef S = *एतत्;

  // Count down from MaxSplit. When MaxSplit is -1, this will just split
  // "forever". This doesn't support splitting more than 2^31 times
  // intentionally; if we ever want that we can make MaxSplit a 64-bit integer
  // but that seems unlikely to be useful.
  जबसम्म (MaxSplit-- != 0) {
    माप_प्रकार Idx = S.find(Separator);
    यदि (Idx == npos)
      रोक;

    // Push this split.
    यदि (KeepEmpty || Idx > 0)
      A.थप_back(S.slice(0, Idx));

    // Jump forward.
    S = S.substr(Idx + Separator.size());
  }

  // Push the tail.
  यदि (KeepEmpty || !S.empty())
    A.थप_back(S);
}

शून्य StringRef::split(SmallVectorImpl<StringRef> &A, वर्ण Separator,
                      पूर्णाङ्क MaxSplit, तार्किक KeepEmpty) अचर {
  StringRef S = *एतत्;

  // Count down from MaxSplit. When MaxSplit is -1, this will just split
  // "forever". This doesn't support splitting more than 2^31 times
  // intentionally; if we ever want that we can make MaxSplit a 64-bit integer
  // but that seems unlikely to be useful.
  जबसम्म (MaxSplit-- != 0) {
    माप_प्रकार Idx = S.find(Separator);
    यदि (Idx == npos)
      रोक;

    // Push this split.
    यदि (KeepEmpty || Idx > 0)
      A.थप_back(S.slice(0, Idx));

    // Jump forward.
    S = S.substr(Idx + 1);
  }

  // Push the tail.
  यदि (KeepEmpty || !S.empty())
    A.थप_back(S);
}

//===----------------------------------------------------------------------===//
// Helpful Algorithms
//===----------------------------------------------------------------------===//

/// count - Return the number of non-overlapped occurrences of \arg Str in
/// the string.
माप_प्रकार StringRef::गन्ती(StringRef Str) अचर {
  माप_प्रकार गन्ती_2 = 0;
  माप_प्रकार Pos = 0;
  माप_प्रकार N = Str.size();
  // TODO: For an empty `Str` we return 0 for legacy reasons. Consider changing
  //       this to `Length + 1` which is more in-line with the function
  //       description.
  यदि (!N)
    फर्काऊ 0;
  जबसम्म ((Pos = find(Str, Pos)) != npos) {
    ++गन्ती_2;
    Pos += N;
  }
  फर्काऊ गन्ती_2;
}

अचिन्हित llvm::getAutoSenseRadix(StringRef &Str) {
  यदि (Str.empty())
    फर्काऊ 10;

  यदि (Str.consume_front_insensitive("0x"))
    फर्काऊ 16;

  यदि (Str.consume_front_insensitive("0b"))
    फर्काऊ 2;

  यदि (Str.consume_front("0o"))
    फर्काऊ 8;

  यदि (Str[0] == '0' && Str.size() > 1 && isDigit(Str[1])) {
    Str = Str.substr(1);
    फर्काऊ 8;
  }

  फर्काऊ 10;
}

तार्किक llvm::consumeUnsignedInteger(StringRef &Str, अचिन्हित Radix,
                                  अचिन्हित दीर्घ दीर्घ &Result) {
  // Autosense radix if not specified.
  यदि (Radix == 0)
    Radix = getAutoSenseRadix(Str);

  // Empty strings (after the radix autosense) are invalid.
  यदि (Str.empty()) फर्काऊ सत्य;

  // Parse all the bytes of the string given this radix.  Watch for overflow.
  StringRef Str2 = Str;
  Result = 0;
  जबसम्म (!Str2.empty()) {
    अचिन्हित CharVal;
    यदि (Str2[0] >= '0' && Str2[0] <= '9')
      CharVal = Str2[0] - '0';
    अन्यथा यदि (Str2[0] >= 'a' && Str2[0] <= 'z')
      CharVal = Str2[0] - 'a' + 10;
    अन्यथा यदि (Str2[0] >= 'A' && Str2[0] <= 'Z')
      CharVal = Str2[0] - 'A' + 10;
    अन्यथा
      रोक;

    // If the parsed value is larger than the integer radix, we cannot
    // consume any more characters.
    यदि (CharVal >= Radix)
      रोक;

    // Add in this character.
    अचिन्हित दीर्घ दीर्घ PrevResult = Result;
    Result = Result * Radix + CharVal;

    // Check for overflow by shifting back and seeing if bits were lost.
    यदि (Result / Radix < PrevResult)
      फर्काऊ सत्य;

    Str2 = Str2.substr(1);
  }

  // We consider the operation a failure if no characters were consumed
  // successfully.
  यदि (Str.size() == Str2.size())
    फर्काऊ सत्य;

  Str = Str2;
  फर्काऊ असत्य;
}

तार्किक llvm::consumeSignedInteger(StringRef &Str, अचिन्हित Radix,
                                दीर्घ दीर्घ &Result) {
  अचिन्हित दीर्घ दीर्घ ULLVal;

  // Handle positive strings first.
  यदि (!Str.starts_with("-")) {
    यदि (consumeUnsignedInteger(Str, Radix, ULLVal) ||
        // Check for value so large it overflows a signed value.
        (दीर्घ दीर्घ)ULLVal < 0)
      फर्काऊ सत्य;
    Result = ULLVal;
    फर्काऊ असत्य;
  }

  // Get the positive part of the value.
  StringRef Str2 = Str.drop_front(1);
  यदि (consumeUnsignedInteger(Str2, Radix, ULLVal) ||
      // Reject values so large they'd overflow as negative signed, but allow
      // "-0".  This negates the unsigned so that the negative isn't undefined
      // on signed overflow.
      (दीर्घ दीर्घ)-ULLVal > 0)
    फर्काऊ सत्य;

  Str = Str2;
  Result = -ULLVal;
  फर्काऊ असत्य;
}

/// GetAsUnsignedInteger - Workhorse method that converts a integer character
/// sequence of radix up to 36 to an unsigned long long value.
तार्किक llvm::getAsUnsignedInteger(StringRef Str, अचिन्हित Radix,
                                अचिन्हित दीर्घ दीर्घ &Result) {
  यदि (consumeUnsignedInteger(Str, Radix, Result))
    फर्काऊ सत्य;

  // For getAsUnsignedInteger, we require the whole string to be consumed or
  // else we consider it a failure.
  फर्काऊ !Str.empty();
}

तार्किक llvm::getAsSignedInteger(StringRef Str, अचिन्हित Radix,
                              दीर्घ दीर्घ &Result) {
  यदि (consumeSignedInteger(Str, Radix, Result))
    फर्काऊ सत्य;

  // For getAsSignedInteger, we require the whole string to be consumed or else
  // we consider it a failure.
  फर्काऊ !Str.empty();
}

तार्किक StringRef::consumeInteger(अचिन्हित Radix, APInt &Result) {
  StringRef Str = *एतत्;

  // Autosense radix if not specified.
  यदि (Radix == 0)
    Radix = getAutoSenseRadix(Str);

  प्रतिज्ञा(Radix > 1 && Radix <= 36);

  // Empty strings (after the radix autosense) are invalid.
  यदि (Str.empty()) फर्काऊ सत्य;

  // Skip leading zeroes.  This can be a significant improvement if
  // it means we don't need > 64 bits.
  Str = Str.ltrim('0');

  // If it was nothing but zeroes....
  यदि (Str.empty()) {
    Result = APInt(64, 0);
    *एतत् = Str;
    फर्काऊ असत्य;
  }

  अचिन्हित Log2Radix = 0;
  जबसम्म ((1U << Log2Radix) < Radix) Log2Radix++;
  तार्किक IsPowerOf2Radix = ((1U << Log2Radix) == Radix);

  // Initialize Result to a reasonable starting width (at least 64 bits),
  // but do not shrink it if it was already larger.
  अचिन्हित BitWidth = std::max(64U, Result.getBitWidth());

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
  जबसम्म (!Str.empty()) {
    अचिन्हित CharVal;
    यदि (Str[0] >= '0' && Str[0] <= '9')
      CharVal = Str[0]-'0';
    अन्यथा यदि (Str[0] >= 'a' && Str[0] <= 'z')
      CharVal = Str[0]-'a'+10;
    अन्यथा यदि (Str[0] >= 'A' && Str[0] <= 'Z')
      CharVal = Str[0]-'A'+10;
    अन्यथा
      रोक;

    // If the parsed value is larger than the integer radix, the string is
    // invalid.
    यदि (CharVal >= Radix)
      रोक;

    // Check if one more digit will overflow, and grow if so.
    यदि (Result.getActiveBits() + Log2Radix > Result.getBitWidth()) {
      अचिन्हित NewWidth = Result.getBitWidth() * 2;

      // Guard against overflow of the NewWidth itself
      यदि (NewWidth < Result.getBitWidth())
        फर्काऊ सत्य;

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
    फर्काऊ सत्य;

  *एतत् = Str;
  फर्काऊ असत्य;
}

तार्किक StringRef::getAsInteger(अचिन्हित Radix, APInt &Result) अचर {
  StringRef Str = *एतत्;
  यदि (Str.consumeInteger(Radix, Result))
    फर्काऊ सत्य;

  // For getAsInteger, we require the whole string to be consumed or else we
  // consider it a failure.
  फर्काऊ !Str.empty();
}

तार्किक StringRef::getAsDouble(द्विगुण &Result, तार्किक AllowInexact) अचर {
  APFloat F(0.0);
  स्वतः StatusOrErr = F.convertFromString(*एतत्, APFloat::rmNearestTiesToEven);
  यदि (errorToBool(StatusOrErr.takeError()))
    फर्काऊ सत्य;

  APFloat::opStatus Status = *StatusOrErr;
  यदि (Status != APFloat::opOK) {
    यदि (!AllowInexact || !(Status & APFloat::opInexact))
      फर्काऊ सत्य;
  }

  Result = F.convertToDouble();
  फर्काऊ असत्य;
}

// Implementation of StringRef hashing.
hash_code llvm::hash_मान(StringRef S) { फर्काऊ hash_combine_range(S); }

अचिन्हित DenseMapInfo<StringRef, शून्य>::get_Hash_मान(StringRef Val) {
  फर्काऊ (अचिन्हित)(hash_मान(Val));
}
