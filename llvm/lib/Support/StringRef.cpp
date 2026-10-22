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
स्थिर पूर्णांक ascii_strncasecmp(StringRef LHS, StringRef RHS) {
  साठी (स्वतः [LC, RC] : zip_equal(LHS, RHS)) {
    अचिन्हित अक्षर LHC = toLower(LC);
    अचिन्हित अक्षर RHC = toLower(RC);
    जर (LHC != RHC)
      परत LHC < RHC ? -1 : 1;
  }
  परत 0;
}

पूर्णांक StringRef::compare_insensitive(StringRef RHS) स्थिरांक {
  माप_प्रकार Min = std::min(size(), RHS.size());
  जर (पूर्णांक Res = ascii_strncasecmp(take_front(Min), RHS.take_front(Min)))
    परत Res;
  जर (size() == RHS.size())
    परत 0;
  परत size() < RHS.size() ? -1 : 1;
}

तार्किक StringRef::starts_with_insensitive(StringRef Prefix) स्थिरांक {
  परत size() >= Prefix.size() &&
         ascii_strncasecmp(take_front(Prefix.size()), Prefix) == 0;
}

तार्किक StringRef::ends_with_insensitive(StringRef Suffix) स्थिरांक {
  परत size() >= Suffix.size() &&
         ascii_strncasecmp(take_back(Suffix.size()), Suffix) == 0;
}

माप_प्रकार StringRef::find_insensitive(अक्षर C, माप_प्रकार From) स्थिरांक {
  अक्षर L = toLower(C);
  परत find_if([L](अक्षर D) { परत toLower(D) == L; }, From);
}

/// compare_numeric - Compare strings, handle embedded numbers.
पूर्णांक StringRef::compare_numeric(StringRef RHS) स्थिरांक {
  साठी (माप_प्रकार I = 0, E = std::min(size(), RHS.size()); I != E; ++I) {
    // Check for sequences of digits.
    जर (isDigit(data()[I]) && isDigit(RHS.data()[I])) {
      // The longer sequence of numbers is considered larger.
      // This doesn't really handle prefixed zeros well.
      माप_प्रकार J;
      साठी (J = I + 1; J != E + 1; ++J) {
        तार्किक ld = J < size() && isDigit(data()[J]);
        तार्किक rd = J < RHS.size() && isDigit(RHS.data()[J]);
        जर (ld != rd)
          परत rd ? -1 : 1;
        जर (!rd)
          थांबा;
      }
      // The two number sequences have the same length (J-I), just memcmp them.
      जर (पूर्णांक Res = compareMemory(data() + I, RHS.data() + I, J - I))
        परत Res < 0 ? -1 : 1;
      // Identical number sequences, continue search after the numbers.
      I = J - 1;
      सुरूठेवा;
    }
    जर (data()[I] != RHS.data()[I])
      परत (अचिन्हित अक्षर)data()[I] < (अचिन्हित अक्षर)RHS.data()[I] ? -1 : 1;
  }
  जर (size() == RHS.size())
    परत 0;
  परत size() < RHS.size() ? -1 : 1;
}

// Compute the edit distance between the two given strings.
अचिन्हित StringRef::edit_distance(llvm::StringRef Other,
                                  तार्किक AllowReplacements,
                                  अचिन्हित MaxEditDistance) स्थिरांक {
  परत llvm::ComputeEditDistance(ArrayRef(data(), size()),
                                   ArrayRef(Other.data(), Other.size()),
                                   AllowReplacements, MaxEditDistance);
}

अचिन्हित llvm::StringRef::edit_distance_insensitive(
    StringRef Other, तार्किक AllowReplacements, अचिन्हित MaxEditDistance) स्थिरांक {
  परत llvm::ComputeMappedEditDistance(
      ArrayRef(data(), size()), ArrayRef(Other.data(), Other.size()),
      llvm::toLower, AllowReplacements, MaxEditDistance);
}

//===----------------------------------------------------------------------===//
// String Operations
//===----------------------------------------------------------------------===//

std::string StringRef::lower() स्थिरांक {
  परत std::string(map_iterator(begin(), toLower),
                     map_iterator(end(), toLower));
}

std::string StringRef::upper() स्थिरांक {
  परत std::string(map_iterator(begin(), toUpper),
                     map_iterator(end(), toUpper));
}

//===----------------------------------------------------------------------===//
// String Searching
//===----------------------------------------------------------------------===//


/// find - Search for the first string \arg Str in the string.
///
/// \return - The index of the first occurrence of \arg Str, or npos if not
/// found.
माप_प्रकार StringRef::find(StringRef Str, माप_प्रकार From) स्थिरांक {
  जर (From > size())
    परत npos;

  स्थिरांक अक्षर *Start = data() + From;
  माप_प्रकार Size = size() - From;

  स्थिरांक अक्षर *Needle = Str.data();
  माप_प्रकार N = Str.size();
  जर (N == 0)
    परत From;
  जर (Size < N)
    परत npos;
  जर (N == 1) {
    स्थिरांक अक्षर *Ptr = (स्थिरांक अक्षर *)::स_प्रथम(Start, Needle[0], Size);
    परत Ptr == nullptr ? npos : Ptr - data();
  }

  स्थिरांक अक्षर *Stop = Start + (Size - N + 1);

  जर (N == 2) {
    // Provide a fast path for newline finding (CRLF case) in InclusionRewriter.
    // Not the most optimized strategy, but getting memcmp inlined should be
    // good enough.
    करा {
      जर (std::स_भेद(Start, Needle, 2) == 0)
        परत Start - data();
      ++Start;
    } पर्यंत (Start < Stop);
    परत npos;
  }

  // For short haystacks or unsupported needles fall back to the naive algorithm
  जर (Size < 16 || N > 255) {
    करा {
      जर (std::स_भेद(Start, Needle, N) == 0)
        परत Start - data();
      ++Start;
    } पर्यंत (Start < Stop);
    परत npos;
  }

  // Build the bad char heuristic table, with uint8_t to reduce cache thrashing.
  uint8_t BadCharSkip[256];
  std::स_रखो(BadCharSkip, N, 256);
  साठी (अचिन्हित i = 0; i != N-1; ++i)
    BadCharSkip[(uint8_t)Str[i]] = N-1-i;

  करा {
    uint8_t Last = Start[N - 1];
    जर (LLVM_UNLIKELY(Last == (uint8_t)Needle[N - 1]))
      जर (std::स_भेद(Start, Needle, N - 1) == 0)
        परत Start - data();

    // Otherwise skip the appropriate number of bytes.
    Start += BadCharSkip[Last];
  } पर्यंत (Start < Stop);

  परत npos;
}

माप_प्रकार StringRef::find_insensitive(StringRef Str, माप_प्रकार From) स्थिरांक {
  StringRef This = substr(From);
  पर्यंत (This.size() >= Str.size()) {
    जर (This.starts_with_insensitive(Str))
      परत From;
    This = This.drop_front();
    ++From;
  }
  परत npos;
}

माप_प्रकार StringRef::rfind_insensitive(अक्षर C, माप_प्रकार From) स्थिरांक {
  From = std::min(From, size());
  माप_प्रकार i = From;
  पर्यंत (i != 0) {
    --i;
    जर (toLower(data()[i]) == toLower(C))
      परत i;
  }
  परत npos;
}

/// rfind - Search for the last string \arg Str in the string.
///
/// \return - The index of the last occurrence of \arg Str, or npos if not
/// found.
माप_प्रकार StringRef::rfind(StringRef Str) स्थिरांक {
  परत std::string_view(*हे).rfind(Str);
}

माप_प्रकार StringRef::rfind_insensitive(StringRef Str) स्थिरांक {
  माप_प्रकार N = Str.size();
  जर (N > size())
    परत npos;
  साठी (माप_प्रकार i = size() - N + 1, e = 0; i != e;) {
    --i;
    जर (substr(i, N).equals_insensitive(Str))
      परत i;
  }
  परत npos;
}

/// find_first_of - Find the first character in the string that is in \arg
/// Chars, or npos if not found.
///
/// Note: O(size() + Chars.size())
StringRef::size_type StringRef::find_first_of(StringRef Chars,
                                              माप_प्रकार From) स्थिरांक {
  std::bitset<1 << अक्षर_बिट> CharBits;
  साठी (अक्षर C : Chars)
    CharBits.set((अचिन्हित अक्षर)C);

  साठी (size_type i = std::min(From, size()), e = size(); i != e; ++i)
    जर (CharBits.test((अचिन्हित अक्षर)data()[i]))
      परत i;
  परत npos;
}

/// find_first_not_of - Find the first character in the string that is not
/// \arg C or npos if not found.
StringRef::size_type StringRef::find_first_not_of(अक्षर C, माप_प्रकार From) स्थिरांक {
  परत std::string_view(*हे).find_first_not_of(C, From);
}

/// find_first_not_of - Find the first character in the string that is not
/// in the string \arg Chars, or npos if not found.
///
/// Note: O(size() + Chars.size())
StringRef::size_type StringRef::find_first_not_of(StringRef Chars,
                                                  माप_प्रकार From) स्थिरांक {
  std::bitset<1 << अक्षर_बिट> CharBits;
  साठी (अक्षर C : Chars)
    CharBits.set((अचिन्हित अक्षर)C);

  साठी (size_type i = std::min(From, size()), e = size(); i != e; ++i)
    जर (!CharBits.test((अचिन्हित अक्षर)data()[i]))
      परत i;
  परत npos;
}

/// find_last_of - Find the last character in the string that is in \arg C,
/// or npos if not found.
///
/// Note: O(size() + Chars.size())
StringRef::size_type StringRef::find_last_of(StringRef Chars,
                                             माप_प्रकार From) स्थिरांक {
  std::bitset<1 << अक्षर_बिट> CharBits;
  साठी (अक्षर C : Chars)
    CharBits.set((अचिन्हित अक्षर)C);

  साठी (size_type i = std::min(From, size()) - 1, e = -1; i != e; --i)
    जर (CharBits.test((अचिन्हित अक्षर)data()[i]))
      परत i;
  परत npos;
}

/// find_last_not_of - Find the last character in the string that is not
/// \arg C, or npos if not found.
StringRef::size_type StringRef::find_last_not_of(अक्षर C, माप_प्रकार From) स्थिरांक {
  साठी (size_type i = std::min(From, size()) - 1, e = -1; i != e; --i)
    जर (data()[i] != C)
      परत i;
  परत npos;
}

/// find_last_not_of - Find the last character in the string that is not in
/// \arg Chars, or npos if not found.
///
/// Note: O(size() + Chars.size())
StringRef::size_type StringRef::find_last_not_of(StringRef Chars,
                                                 माप_प्रकार From) स्थिरांक {
  std::bitset<1 << अक्षर_बिट> CharBits;
  साठी (अक्षर C : Chars)
    CharBits.set((अचिन्हित अक्षर)C);

  साठी (size_type i = std::min(From, size()) - 1, e = -1; i != e; --i)
    जर (!CharBits.test((अचिन्हित अक्षर)data()[i]))
      परत i;
  परत npos;
}

शून्य StringRef::split(SmallVectorImpl<StringRef> &A,
                      StringRef Separator, पूर्णांक MaxSplit,
                      तार्किक KeepEmpty) स्थिरांक {
  StringRef S = *हे;

  // Count down from MaxSplit. When MaxSplit is -1, this will just split
  // "forever". This doesn't support splitting more than 2^31 times
  // intentionally; if we ever want that we can make MaxSplit a 64-bit integer
  // but that seems unlikely to be useful.
  पर्यंत (MaxSplit-- != 0) {
    माप_प्रकार Idx = S.find(Separator);
    जर (Idx == npos)
      थांबा;

    // Push this split.
    जर (KeepEmpty || Idx > 0)
      A.ढकला_back(S.slice(0, Idx));

    // Jump forward.
    S = S.substr(Idx + Separator.size());
  }

  // Push the tail.
  जर (KeepEmpty || !S.empty())
    A.ढकला_back(S);
}

शून्य StringRef::split(SmallVectorImpl<StringRef> &A, अक्षर Separator,
                      पूर्णांक MaxSplit, तार्किक KeepEmpty) स्थिरांक {
  StringRef S = *हे;

  // Count down from MaxSplit. When MaxSplit is -1, this will just split
  // "forever". This doesn't support splitting more than 2^31 times
  // intentionally; if we ever want that we can make MaxSplit a 64-bit integer
  // but that seems unlikely to be useful.
  पर्यंत (MaxSplit-- != 0) {
    माप_प्रकार Idx = S.find(Separator);
    जर (Idx == npos)
      थांबा;

    // Push this split.
    जर (KeepEmpty || Idx > 0)
      A.ढकला_back(S.slice(0, Idx));

    // Jump forward.
    S = S.substr(Idx + 1);
  }

  // Push the tail.
  जर (KeepEmpty || !S.empty())
    A.ढकला_back(S);
}

//===----------------------------------------------------------------------===//
// Helpful Algorithms
//===----------------------------------------------------------------------===//

/// count - Return the number of non-overlapped occurrences of \arg Str in
/// the string.
माप_प्रकार StringRef::मोजणी(StringRef Str) स्थिरांक {
  माप_प्रकार मोजणी_2 = 0;
  माप_प्रकार Pos = 0;
  माप_प्रकार N = Str.size();
  // TODO: For an empty `Str` we return 0 for legacy reasons. Consider changing
  //       this to `Length + 1` which is more in-line with the function
  //       description.
  जर (!N)
    परत 0;
  पर्यंत ((Pos = find(Str, Pos)) != npos) {
    ++मोजणी_2;
    Pos += N;
  }
  परत मोजणी_2;
}

अचिन्हित llvm::getAutoSenseRadix(StringRef &Str) {
  जर (Str.empty())
    परत 10;

  जर (Str.consume_front_insensitive("0x"))
    परत 16;

  जर (Str.consume_front_insensitive("0b"))
    परत 2;

  जर (Str.consume_front("0o"))
    परत 8;

  जर (Str[0] == '0' && Str.size() > 1 && isDigit(Str[1])) {
    Str = Str.substr(1);
    परत 8;
  }

  परत 10;
}

तार्किक llvm::consumeUnsignedInteger(StringRef &Str, अचिन्हित Radix,
                                  अचिन्हित दीर्घ दीर्घ &Result) {
  // Autosense radix if not specified.
  जर (Radix == 0)
    Radix = getAutoSenseRadix(Str);

  // Empty strings (after the radix autosense) are invalid.
  जर (Str.empty()) परत सत्य;

  // Parse all the bytes of the string given this radix.  Watch for overflow.
  StringRef Str2 = Str;
  Result = 0;
  पर्यंत (!Str2.empty()) {
    अचिन्हित CharVal;
    जर (Str2[0] >= '0' && Str2[0] <= '9')
      CharVal = Str2[0] - '0';
    अन्यथा जर (Str2[0] >= 'a' && Str2[0] <= 'z')
      CharVal = Str2[0] - 'a' + 10;
    अन्यथा जर (Str2[0] >= 'A' && Str2[0] <= 'Z')
      CharVal = Str2[0] - 'A' + 10;
    अन्यथा
      थांबा;

    // If the parsed value is larger than the integer radix, we cannot
    // consume any more characters.
    जर (CharVal >= Radix)
      थांबा;

    // Add in this character.
    अचिन्हित दीर्घ दीर्घ PrevResult = Result;
    Result = Result * Radix + CharVal;

    // Check for overflow by shifting back and seeing if bits were lost.
    जर (Result / Radix < PrevResult)
      परत सत्य;

    Str2 = Str2.substr(1);
  }

  // We consider the operation a failure if no characters were consumed
  // successfully.
  जर (Str.size() == Str2.size())
    परत सत्य;

  Str = Str2;
  परत असत्य;
}

तार्किक llvm::consumeSignedInteger(StringRef &Str, अचिन्हित Radix,
                                दीर्घ दीर्घ &Result) {
  अचिन्हित दीर्घ दीर्घ ULLVal;

  // Handle positive strings first.
  जर (!Str.starts_with("-")) {
    जर (consumeUnsignedInteger(Str, Radix, ULLVal) ||
        // Check for value so large it overflows a signed value.
        (दीर्घ दीर्घ)ULLVal < 0)
      परत सत्य;
    Result = ULLVal;
    परत असत्य;
  }

  // Get the positive part of the value.
  StringRef Str2 = Str.drop_front(1);
  जर (consumeUnsignedInteger(Str2, Radix, ULLVal) ||
      // Reject values so large they'd overflow as negative signed, but allow
      // "-0".  This negates the unsigned so that the negative isn't undefined
      // on signed overflow.
      (दीर्घ दीर्घ)-ULLVal > 0)
    परत सत्य;

  Str = Str2;
  Result = -ULLVal;
  परत असत्य;
}

/// GetAsUnsignedInteger - Workhorse method that converts a integer character
/// sequence of radix up to 36 to an unsigned long long value.
तार्किक llvm::getAsUnsignedInteger(StringRef Str, अचिन्हित Radix,
                                अचिन्हित दीर्घ दीर्घ &Result) {
  जर (consumeUnsignedInteger(Str, Radix, Result))
    परत सत्य;

  // For getAsUnsignedInteger, we require the whole string to be consumed or
  // else we consider it a failure.
  परत !Str.empty();
}

तार्किक llvm::getAsSignedInteger(StringRef Str, अचिन्हित Radix,
                              दीर्घ दीर्घ &Result) {
  जर (consumeSignedInteger(Str, Radix, Result))
    परत सत्य;

  // For getAsSignedInteger, we require the whole string to be consumed or else
  // we consider it a failure.
  परत !Str.empty();
}

तार्किक StringRef::consumeInteger(अचिन्हित Radix, APInt &Result) {
  StringRef Str = *हे;

  // Autosense radix if not specified.
  जर (Radix == 0)
    Radix = getAutoSenseRadix(Str);

  प्रतिज्ञा(Radix > 1 && Radix <= 36);

  // Empty strings (after the radix autosense) are invalid.
  जर (Str.empty()) परत सत्य;

  // Skip leading zeroes.  This can be a significant improvement if
  // it means we don't need > 64 bits.
  Str = Str.ltrim('0');

  // If it was nothing but zeroes....
  जर (Str.empty()) {
    Result = APInt(64, 0);
    *हे = Str;
    परत असत्य;
  }

  अचिन्हित Log2Radix = 0;
  पर्यंत ((1U << Log2Radix) < Radix) Log2Radix++;
  तार्किक IsPowerOf2Radix = ((1U << Log2Radix) == Radix);

  // Initialize Result to a reasonable starting width (at least 64 bits),
  // but do not shrink it if it was already larger.
  अचिन्हित BitWidth = std::max(64U, Result.getBitWidth());

  जर (Result.getBitWidth() < BitWidth) {
    Result = Result.zext(BitWidth);
  }

  APInt RadixAP, CharAP; // unused unless !IsPowerOf2Radix
  जर (!IsPowerOf2Radix) {
    // These must have the same bit-width as Result.
    RadixAP = APInt(BitWidth, Radix);
    CharAP = APInt(BitWidth, 0);
  }

  // Parse all the bytes of the string given this radix.
  Result = 0;
  पर्यंत (!Str.empty()) {
    अचिन्हित CharVal;
    जर (Str[0] >= '0' && Str[0] <= '9')
      CharVal = Str[0]-'0';
    अन्यथा जर (Str[0] >= 'a' && Str[0] <= 'z')
      CharVal = Str[0]-'a'+10;
    अन्यथा जर (Str[0] >= 'A' && Str[0] <= 'Z')
      CharVal = Str[0]-'A'+10;
    अन्यथा
      थांबा;

    // If the parsed value is larger than the integer radix, the string is
    // invalid.
    जर (CharVal >= Radix)
      थांबा;

    // Check if one more digit will overflow, and grow if so.
    जर (Result.getActiveBits() + Log2Radix > Result.getBitWidth()) {
      अचिन्हित NewWidth = Result.getBitWidth() * 2;

      // Guard against overflow of the NewWidth itself
      जर (NewWidth < Result.getBitWidth())
        परत सत्य;

      Result = Result.zext(NewWidth);
      जर (!IsPowerOf2Radix) {
        RadixAP = RadixAP.zext(NewWidth);
        CharAP = CharAP.zext(NewWidth);
      }
    }

    // Add in this character.
    जर (IsPowerOf2Radix) {
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
  जर (size() == Str.size())
    परत सत्य;

  *हे = Str;
  परत असत्य;
}

तार्किक StringRef::getAsInteger(अचिन्हित Radix, APInt &Result) स्थिरांक {
  StringRef Str = *हे;
  जर (Str.consumeInteger(Radix, Result))
    परत सत्य;

  // For getAsInteger, we require the whole string to be consumed or else we
  // consider it a failure.
  परत !Str.empty();
}

तार्किक StringRef::getAsDouble(द्विगुण &Result, तार्किक AllowInexact) स्थिरांक {
  APFloat F(0.0);
  स्वतः StatusOrErr = F.convertFromString(*हे, APFloat::rmNearestTiesToEven);
  जर (errorToBool(StatusOrErr.takeError()))
    परत सत्य;

  APFloat::opStatus Status = *StatusOrErr;
  जर (Status != APFloat::opOK) {
    जर (!AllowInexact || !(Status & APFloat::opInexact))
      परत सत्य;
  }

  Result = F.convertToDouble();
  परत असत्य;
}

// Implementation of StringRef hashing.
hash_code llvm::hash_मूल्य(StringRef S) { परत hash_combine_range(S); }

अचिन्हित DenseMapInfo<StringRef, शून्य>::get_Hash_मूल्य(StringRef Val) {
  परत (अचिन्हित)(hash_मूल्य(Val));
}
