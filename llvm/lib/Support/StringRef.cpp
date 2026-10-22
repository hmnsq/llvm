//===-- StringRef.cpp - Lightweight String References ---------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#समावेश "llvm/ADT/StringRef.h"
#समावेश "llvm/ADT/APFloat.h"
#समावेश "llvm/ADT/APInt.h"
#समावेश "llvm/ADT/Hashing.h"
#समावेश "llvm/ADT/StringExtras.h"
#समावेश "llvm/ADT/edit_distance.h"
#समावेश "llvm/Support/Error.h"
#समावेश <bitset>

using namespace llvm;

// strncasecmp() is not available on non-POSIX systems, so define an
// alternative function here.
अटल पूर्णांक ascii_strncasecmp(StringRef LHS, StringRef RHS) {
  क्रम (स्वतः [LC, RC] : zip_equal(LHS, RHS)) {
    अचिन्हित अक्षर LHC = toLower(LC);
    अचिन्हित अक्षर RHC = toLower(RC);
    अगर (LHC != RHC)
      वापस LHC < RHC ? -1 : 1;
  }
  वापस 0;
}

पूर्णांक StringRef::compare_insensitive(StringRef RHS) स्थिर {
  माप_प्रकार Min = std::min(आकार_2(), RHS.आकार_2());
  अगर (पूर्णांक Res = ascii_strncasecmp(take_front(Min), RHS.take_front(Min)))
    वापस Res;
  अगर (आकार_2() == RHS.आकार_2())
    वापस 0;
  वापस आकार_2() < RHS.आकार_2() ? -1 : 1;
}

bool StringRef::starts_with_insensitive(StringRef Prefix) स्थिर {
  वापस आकार_2() >= Prefix.आकार_2() &&
         ascii_strncasecmp(take_front(Prefix.आकार_2()), Prefix) == 0;
}

bool StringRef::ends_with_insensitive(StringRef Suffix) स्थिर {
  वापस आकार_2() >= Suffix.आकार_2() &&
         ascii_strncasecmp(take_back(Suffix.आकार_2()), Suffix) == 0;
}

माप_प्रकार StringRef::find_insensitive(अक्षर C, माप_प्रकार From) स्थिर {
  अक्षर L = toLower(C);
  वापस find_if([L](अक्षर D) { वापस toLower(D) == L; }, From);
}

/// compare_numeric - Compare strings, handle embedded numbers.
पूर्णांक StringRef::compare_numeric(StringRef RHS) स्थिर {
  क्रम (माप_प्रकार I = 0, E = std::min(आकार_2(), RHS.आकार_2()); I != E; ++I) {
    // Check for sequences of digits.
    अगर (isDigit(तथ्य()[I]) && isDigit(RHS.तथ्य()[I])) {
      // The longer sequence of numbers is considered larger.
      // This doesn't really handle prefixed zeros well.
      माप_प्रकार J;
      क्रम (J = I + 1; J != E + 1; ++J) {
        bool ld = J < आकार_2() && isDigit(तथ्य()[J]);
        bool rd = J < RHS.आकार_2() && isDigit(RHS.तथ्य()[J]);
        अगर (ld != rd)
          वापस rd ? -1 : 1;
        अगर (!rd)
          अवरोध;
      }
      // The two number sequences have the same length (J-I), just memcmp them.
      अगर (पूर्णांक Res = compareMemory(तथ्य() + I, RHS.तथ्य() + I, J - I))
        वापस Res < 0 ? -1 : 1;
      // Identical number sequences, continue search after the numbers.
      I = J - 1;
      जारी;
    }
    अगर (तथ्य()[I] != RHS.तथ्य()[I])
      वापस (अचिन्हित अक्षर)तथ्य()[I] < (अचिन्हित अक्षर)RHS.तथ्य()[I] ? -1 : 1;
  }
  अगर (आकार_2() == RHS.आकार_2())
    वापस 0;
  वापस आकार_2() < RHS.आकार_2() ? -1 : 1;
}

// Compute the edit distance between the two given strings.
अचिन्हित StringRef::edit_distance(llvm::StringRef Other,
                                  bool AllowReplacements,
                                  अचिन्हित MaxEditDistance) स्थिर {
  वापस llvm::ComputeEditDistance(ArrayRef(तथ्य(), आकार_2()),
                                   ArrayRef(Other.तथ्य(), Other.आकार_2()),
                                   AllowReplacements, MaxEditDistance);
}

अचिन्हित llvm::StringRef::edit_distance_insensitive(
    StringRef Other, bool AllowReplacements, अचिन्हित MaxEditDistance) स्थिर {
  वापस llvm::ComputeMappedEditDistance(
      ArrayRef(तथ्य(), आकार_2()), ArrayRef(Other.तथ्य(), Other.आकार_2()),
      llvm::toLower, AllowReplacements, MaxEditDistance);
}

//===----------------------------------------------------------------------===//
// String Operations
//===----------------------------------------------------------------------===//

std::string StringRef::lower() स्थिर {
  वापस std::string(map_iterator(begin(), toLower),
                     map_iterator(end(), toLower));
}

std::string StringRef::upper() स्थिर {
  वापस std::string(map_iterator(begin(), toUpper),
                     map_iterator(end(), toUpper));
}

//===----------------------------------------------------------------------===//
// String Searching
//===----------------------------------------------------------------------===//


/// find - Search for the first string \arg Str in the string.
///
/// \return - The index of the first occurrence of \arg Str, or npos if not
/// found.
माप_प्रकार StringRef::find(StringRef Str, माप_प्रकार From) स्थिर {
  अगर (From > आकार_2())
    वापस npos;

  स्थिर अक्षर *Start = तथ्य() + From;
  माप_प्रकार आकार = आकार_2() - From;

  स्थिर अक्षर *Needle = Str.तथ्य();
  माप_प्रकार N = Str.आकार_2();
  अगर (N == 0)
    वापस From;
  अगर (आकार < N)
    वापस npos;
  अगर (N == 1) {
    स्थिर अक्षर *Ptr = (स्थिर अक्षर *)::स_प्रथम(Start, Needle[0], आकार);
    वापस Ptr == nullptr ? npos : Ptr - तथ्य();
  }

  स्थिर अक्षर *Stop = Start + (आकार - N + 1);

  अगर (N == 2) {
    // Provide a fast path for newline finding (CRLF case) in InclusionRewriter.
    // Not the most optimized strategy, but getting memcmp inlined should be
    // good enough.
    करो {
      अगर (std::स_भेद(Start, Needle, 2) == 0)
        वापस Start - तथ्य();
      ++Start;
    } जबतक (Start < Stop);
    वापस npos;
  }

  // For short haystacks or unsupported needles fall back to the naive algorithm
  अगर (आकार < 16 || N > 255) {
    करो {
      अगर (std::स_भेद(Start, Needle, N) == 0)
        वापस Start - तथ्य();
      ++Start;
    } जबतक (Start < Stop);
    वापस npos;
  }

  // Build the bad char heuristic table, with uint8_t to reduce cache thrashing.
  uint8_t BadCharSkip[256];
  std::स_रखो(BadCharSkip, N, 256);
  क्रम (अचिन्हित i = 0; i != N-1; ++i)
    BadCharSkip[(uint8_t)Str[i]] = N-1-i;

  करो {
    uint8_t Last = Start[N - 1];
    अगर (LLVM_UNLIKELY(Last == (uint8_t)Needle[N - 1]))
      अगर (std::स_भेद(Start, Needle, N - 1) == 0)
        वापस Start - तथ्य();

    // Otherwise skip the appropriate number of bytes.
    Start += BadCharSkip[Last];
  } जबतक (Start < Stop);

  वापस npos;
}

माप_प्रकार StringRef::find_insensitive(StringRef Str, माप_प्रकार From) स्थिर {
  StringRef This = substr(From);
  जबतक (This.आकार_2() >= Str.आकार_2()) {
    अगर (This.starts_with_insensitive(Str))
      वापस From;
    This = This.drop_front();
    ++From;
  }
  वापस npos;
}

माप_प्रकार StringRef::rfind_insensitive(अक्षर C, माप_प्रकार From) स्थिर {
  From = std::min(From, आकार_2());
  माप_प्रकार i = From;
  जबतक (i != 0) {
    --i;
    अगर (toLower(तथ्य()[i]) == toLower(C))
      वापस i;
  }
  वापस npos;
}

/// rfind - Search for the last string \arg Str in the string.
///
/// \return - The index of the last occurrence of \arg Str, or npos if not
/// found.
माप_प्रकार StringRef::rfind(StringRef Str) स्थिर {
  वापस std::string_view(*यह).rfind(Str);
}

माप_प्रकार StringRef::rfind_insensitive(StringRef Str) स्थिर {
  माप_प्रकार N = Str.आकार_2();
  अगर (N > आकार_2())
    वापस npos;
  क्रम (माप_प्रकार i = आकार_2() - N + 1, e = 0; i != e;) {
    --i;
    अगर (substr(i, N).equals_insensitive(Str))
      वापस i;
  }
  वापस npos;
}

/// find_first_of - Find the first character in the string that is in \arg
/// Chars, or npos if not found.
///
/// Note: O(size() + Chars.size())
StringRef::आकार_type StringRef::find_first_of(StringRef Chars,
                                              माप_प्रकार From) स्थिर {
  std::bitset<1 << अक्षर_बिट> CharBits;
  क्रम (अक्षर C : Chars)
    CharBits.set((अचिन्हित अक्षर)C);

  क्रम (आकार_type i = std::min(From, आकार_2()), e = आकार_2(); i != e; ++i)
    अगर (CharBits.test((अचिन्हित अक्षर)तथ्य()[i]))
      वापस i;
  वापस npos;
}

/// find_first_not_of - Find the first character in the string that is not
/// \arg C or npos if not found.
StringRef::आकार_type StringRef::find_first_not_of(अक्षर C, माप_प्रकार From) स्थिर {
  वापस std::string_view(*यह).find_first_not_of(C, From);
}

/// find_first_not_of - Find the first character in the string that is not
/// in the string \arg Chars, or npos if not found.
///
/// Note: O(size() + Chars.size())
StringRef::आकार_type StringRef::find_first_not_of(StringRef Chars,
                                                  माप_प्रकार From) स्थिर {
  std::bitset<1 << अक्षर_बिट> CharBits;
  क्रम (अक्षर C : Chars)
    CharBits.set((अचिन्हित अक्षर)C);

  क्रम (आकार_type i = std::min(From, आकार_2()), e = आकार_2(); i != e; ++i)
    अगर (!CharBits.test((अचिन्हित अक्षर)तथ्य()[i]))
      वापस i;
  वापस npos;
}

/// find_last_of - Find the last character in the string that is in \arg C,
/// or npos if not found.
///
/// Note: O(size() + Chars.size())
StringRef::आकार_type StringRef::find_last_of(StringRef Chars,
                                             माप_प्रकार From) स्थिर {
  std::bitset<1 << अक्षर_बिट> CharBits;
  क्रम (अक्षर C : Chars)
    CharBits.set((अचिन्हित अक्षर)C);

  क्रम (आकार_type i = std::min(From, आकार_2()) - 1, e = -1; i != e; --i)
    अगर (CharBits.test((अचिन्हित अक्षर)तथ्य()[i]))
      वापस i;
  वापस npos;
}

/// find_last_not_of - Find the last character in the string that is not
/// \arg C, or npos if not found.
StringRef::आकार_type StringRef::find_last_not_of(अक्षर C, माप_प्रकार From) स्थिर {
  क्रम (आकार_type i = std::min(From, आकार_2()) - 1, e = -1; i != e; --i)
    अगर (तथ्य()[i] != C)
      वापस i;
  वापस npos;
}

/// find_last_not_of - Find the last character in the string that is not in
/// \arg Chars, or npos if not found.
///
/// Note: O(size() + Chars.size())
StringRef::आकार_type StringRef::find_last_not_of(StringRef Chars,
                                                 माप_प्रकार From) स्थिर {
  std::bitset<1 << अक्षर_बिट> CharBits;
  क्रम (अक्षर C : Chars)
    CharBits.set((अचिन्हित अक्षर)C);

  क्रम (आकार_type i = std::min(From, आकार_2()) - 1, e = -1; i != e; --i)
    अगर (!CharBits.test((अचिन्हित अक्षर)तथ्य()[i]))
      वापस i;
  वापस npos;
}

व्योम StringRef::split(SmallVectorImpl<StringRef> &A,
                      StringRef Separator, पूर्णांक MaxSplit,
                      bool KeepEmpty) स्थिर {
  StringRef S = *यह;

  // Count down from MaxSplit. When MaxSplit is -1, this will just split
  // "forever". This doesn't support splitting more than 2^31 times
  // intentionally; if we ever want that we can make MaxSplit a 64-bit integer
  // but that seems unlikely to be useful.
  जबतक (MaxSplit-- != 0) {
    माप_प्रकार Idx = S.find(Separator);
    अगर (Idx == npos)
      अवरोध;

    // Push this split.
    अगर (KeepEmpty || Idx > 0)
      A.धकेलो_back(S.slice(0, Idx));

    // Jump forward.
    S = S.substr(Idx + Separator.आकार_2());
  }

  // Push the tail.
  अगर (KeepEmpty || !S.empty())
    A.धकेलो_back(S);
}

व्योम StringRef::split(SmallVectorImpl<StringRef> &A, अक्षर Separator,
                      पूर्णांक MaxSplit, bool KeepEmpty) स्थिर {
  StringRef S = *यह;

  // Count down from MaxSplit. When MaxSplit is -1, this will just split
  // "forever". This doesn't support splitting more than 2^31 times
  // intentionally; if we ever want that we can make MaxSplit a 64-bit integer
  // but that seems unlikely to be useful.
  जबतक (MaxSplit-- != 0) {
    माप_प्रकार Idx = S.find(Separator);
    अगर (Idx == npos)
      अवरोध;

    // Push this split.
    अगर (KeepEmpty || Idx > 0)
      A.धकेलो_back(S.slice(0, Idx));

    // Jump forward.
    S = S.substr(Idx + 1);
  }

  // Push the tail.
  अगर (KeepEmpty || !S.empty())
    A.धकेलो_back(S);
}

//===----------------------------------------------------------------------===//
// Helpful Algorithms
//===----------------------------------------------------------------------===//

/// count - Return the number of non-overlapped occurrences of \arg Str in
/// the string.
माप_प्रकार StringRef::गिनती(StringRef Str) स्थिर {
  माप_प्रकार गिनती_2 = 0;
  माप_प्रकार Pos = 0;
  माप_प्रकार N = Str.आकार_2();
  // TODO: For an empty `Str` we return 0 for legacy reasons. Consider changing
  //       this to `Length + 1` which is more in-line with the function
  //       description.
  अगर (!N)
    वापस 0;
  जबतक ((Pos = find(Str, Pos)) != npos) {
    ++गिनती_2;
    Pos += N;
  }
  वापस गिनती_2;
}

अचिन्हित llvm::getAutoSenseRadix(StringRef &Str) {
  अगर (Str.empty())
    वापस 10;

  अगर (Str.consume_front_insensitive("0x"))
    वापस 16;

  अगर (Str.consume_front_insensitive("0b"))
    वापस 2;

  अगर (Str.consume_front("0o"))
    वापस 8;

  अगर (Str[0] == '0' && Str.आकार_2() > 1 && isDigit(Str[1])) {
    Str = Str.substr(1);
    वापस 8;
  }

  वापस 10;
}

bool llvm::consumeUnsignedInteger(StringRef &Str, अचिन्हित Radix,
                                  अचिन्हित दीर्घ दीर्घ &परिणाम) {
  // Autosense radix if not specified.
  अगर (Radix == 0)
    Radix = getAutoSenseRadix(Str);

  // Empty strings (after the radix autosense) are invalid.
  अगर (Str.empty()) वापस true;

  // Parse all the bytes of the string given this radix.  Watch for overflow.
  StringRef Str2 = Str;
  परिणाम = 0;
  जबतक (!Str2.empty()) {
    अचिन्हित CharVal;
    अगर (Str2[0] >= '0' && Str2[0] <= '9')
      CharVal = Str2[0] - '0';
    अन्यथा अगर (Str2[0] >= 'a' && Str2[0] <= 'z')
      CharVal = Str2[0] - 'a' + 10;
    अन्यथा अगर (Str2[0] >= 'A' && Str2[0] <= 'Z')
      CharVal = Str2[0] - 'A' + 10;
    अन्यथा
      अवरोध;

    // If the parsed value is larger than the integer radix, we cannot
    // consume any more characters.
    अगर (CharVal >= Radix)
      अवरोध;

    // Add in this character.
    अचिन्हित दीर्घ दीर्घ पूर्व_परिणाम = परिणाम;
    परिणाम = परिणाम * Radix + CharVal;

    // Check for overflow by shifting back and seeing if bits were lost.
    अगर (परिणाम / Radix < पूर्व_परिणाम)
      वापस true;

    Str2 = Str2.substr(1);
  }

  // We consider the operation a failure if no characters were consumed
  // successfully.
  अगर (Str.आकार_2() == Str2.आकार_2())
    वापस true;

  Str = Str2;
  वापस false;
}

bool llvm::consumeSignedInteger(StringRef &Str, अचिन्हित Radix,
                                दीर्घ दीर्घ &परिणाम) {
  अचिन्हित दीर्घ दीर्घ ULLVal;

  // Handle positive strings first.
  अगर (!Str.starts_with("-")) {
    अगर (consumeUnsignedInteger(Str, Radix, ULLVal) ||
        // Check for value so large it overflows a signed value.
        (दीर्घ दीर्घ)ULLVal < 0)
      वापस true;
    परिणाम = ULLVal;
    वापस false;
  }

  // Get the positive part of the value.
  StringRef Str2 = Str.drop_front(1);
  अगर (consumeUnsignedInteger(Str2, Radix, ULLVal) ||
      // Reject values so large they'd overflow as negative signed, but allow
      // "-0".  This negates the unsigned so that the negative isn't undefined
      // on signed overflow.
      (दीर्घ दीर्घ)-ULLVal > 0)
    वापस true;

  Str = Str2;
  परिणाम = -ULLVal;
  वापस false;
}

/// GetAsUnsignedInteger - Workhorse method that converts a integer character
/// sequence of radix up to 36 to an unsigned long long value.
bool llvm::getAsUnsignedInteger(StringRef Str, अचिन्हित Radix,
                                अचिन्हित दीर्घ दीर्घ &परिणाम) {
  अगर (consumeUnsignedInteger(Str, Radix, परिणाम))
    वापस true;

  // For getAsUnsignedInteger, we require the whole string to be consumed or
  // else we consider it a failure.
  वापस !Str.empty();
}

bool llvm::getAsSignedInteger(StringRef Str, अचिन्हित Radix,
                              दीर्घ दीर्घ &परिणाम) {
  अगर (consumeSignedInteger(Str, Radix, परिणाम))
    वापस true;

  // For getAsSignedInteger, we require the whole string to be consumed or else
  // we consider it a failure.
  वापस !Str.empty();
}

bool StringRef::consumeInteger(अचिन्हित Radix, APInt &परिणाम) {
  StringRef Str = *यह;

  // Autosense radix if not specified.
  अगर (Radix == 0)
    Radix = getAutoSenseRadix(Str);

  निश्चित(Radix > 1 && Radix <= 36);

  // Empty strings (after the radix autosense) are invalid.
  अगर (Str.empty()) वापस true;

  // Skip leading zeroes.  This can be a significant improvement if
  // it means we don't need > 64 bits.
  Str = Str.ltrim('0');

  // If it was nothing but zeroes....
  अगर (Str.empty()) {
    परिणाम = APInt(64, 0);
    *यह = Str;
    वापस false;
  }

  अचिन्हित Log2Radix = 0;
  जबतक ((1U << Log2Radix) < Radix) Log2Radix++;
  bool IsPowerOf2Radix = ((1U << Log2Radix) == Radix);

  // Initialize Result to a reasonable starting width (at least 64 bits),
  // but do not shrink it if it was already larger.
  अचिन्हित BitWidth = std::max(64U, परिणाम.getBitWidth());

  अगर (परिणाम.getBitWidth() < BitWidth) {
    परिणाम = परिणाम.zext(BitWidth);
  }

  APInt RadixAP, CharAP; // unused unless !IsPowerOf2Radix
  अगर (!IsPowerOf2Radix) {
    // These must have the same bit-width as Result.
    RadixAP = APInt(BitWidth, Radix);
    CharAP = APInt(BitWidth, 0);
  }

  // Parse all the bytes of the string given this radix.
  परिणाम = 0;
  जबतक (!Str.empty()) {
    अचिन्हित CharVal;
    अगर (Str[0] >= '0' && Str[0] <= '9')
      CharVal = Str[0]-'0';
    अन्यथा अगर (Str[0] >= 'a' && Str[0] <= 'z')
      CharVal = Str[0]-'a'+10;
    अन्यथा अगर (Str[0] >= 'A' && Str[0] <= 'Z')
      CharVal = Str[0]-'A'+10;
    अन्यथा
      अवरोध;

    // If the parsed value is larger than the integer radix, the string is
    // invalid.
    अगर (CharVal >= Radix)
      अवरोध;

    // Check if one more digit will overflow, and grow if so.
    अगर (परिणाम.getActiveBits() + Log2Radix > परिणाम.getBitWidth()) {
      अचिन्हित NewWidth = परिणाम.getBitWidth() * 2;

      // Guard against overflow of the NewWidth itself
      अगर (NewWidth < परिणाम.getBitWidth())
        वापस true;

      परिणाम = परिणाम.zext(NewWidth);
      अगर (!IsPowerOf2Radix) {
        RadixAP = RadixAP.zext(NewWidth);
        CharAP = CharAP.zext(NewWidth);
      }
    }

    // Add in this character.
    अगर (IsPowerOf2Radix) {
      परिणाम <<= Log2Radix;
      परिणाम |= CharVal;
    } अन्यथा {
      परिणाम *= RadixAP;
      CharAP = CharVal;
      परिणाम += CharAP;
    }

    Str = Str.substr(1);
  }

  // We consider the operation a failure if no characters were consumed
  // successfully.
  अगर (आकार_2() == Str.आकार_2())
    वापस true;

  *यह = Str;
  वापस false;
}

bool StringRef::getAsInteger(अचिन्हित Radix, APInt &परिणाम) स्थिर {
  StringRef Str = *यह;
  अगर (Str.consumeInteger(Radix, परिणाम))
    वापस true;

  // For getAsInteger, we require the whole string to be consumed or else we
  // consider it a failure.
  वापस !Str.empty();
}

bool StringRef::getAsDouble(द्विगुन &परिणाम, bool AllowInexact) स्थिर {
  APFloat F(0.0);
  स्वतः StatusOrErr = F.convertFromString(*यह, APFloat::rmNearestTiesToEven);
  अगर (errorToBool(StatusOrErr.takeError()))
    वापस true;

  APFloat::opStatus Status = *StatusOrErr;
  अगर (Status != APFloat::opOK) {
    अगर (!AllowInexact || !(Status & APFloat::opInexact))
      वापस true;
  }

  परिणाम = F.convertToDouble();
  वापस false;
}

// Implementation of StringRef hashing.
hash_code llvm::hash_मूल्य(StringRef S) { वापस hash_combine_range(S); }

अचिन्हित DenseMapInfo<StringRef, व्योम>::get_Hash_मूल्य(StringRef Val) {
  वापस (अचिन्हित)(hash_मूल्य(Val));
}
