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

उपयुज्य नामाकाश llvm;

// strncasecmp() is not available on non-POSIX systems, so define an
// alternative function here.
स्थिर पूर्णाङ्क ascii_strncasecmp(StringRef LHS, StringRef RHS) {
  प्रति (स्वचालित [LC, RC] : zip_equal(LHS, RHS)) {
    अचिह्नित वर्ण LHC = गन्तव्यम्_अधस्तनम्(LC);
    अचिह्नित वर्ण RHC = गन्तव्यम्_अधस्तनम्(RC);
    यदि (LHC != RHC)
      प्रत्यागम LHC < RHC ? -1 : 1;
  }
  प्रत्यागम 0;
}

पूर्णाङ्क StringRef::compare_insensitive(StringRef RHS) ध्रुव {
  size_t Min = std::min(आकार_2(), RHS.आकार_2());
  यदि (पूर्णाङ्क Res = ascii_strncasecmp(take_front(Min), RHS.take_front(Min)))
    प्रत्यागम Res;
  यदि (आकार_2() == RHS.आकार_2())
    प्रत्यागम 0;
  प्रत्यागम आकार_2() < RHS.आकार_2() ? -1 : 1;
}

तार्किक StringRef::starts_with_insensitive(StringRef उपसर्गः) ध्रुव {
  प्रत्यागम आकार_2() >= उपसर्गः.आकार_2() &&
         ascii_strncasecmp(take_front(उपसर्गः.आकार_2()), उपसर्गः) == 0;
}

तार्किक StringRef::अन्ताः_with_insensitive(StringRef Suffix) ध्रुव {
  प्रत्यागम आकार_2() >= Suffix.आकार_2() &&
         ascii_strncasecmp(take_पृष्ठभागः(Suffix.आकार_2()), Suffix) == 0;
}

size_t StringRef::find_insensitive(वर्ण C, size_t From) ध्रुव {
  वर्ण L = गन्तव्यम्_अधस्तनम्(C);
  प्रत्यागम find_if([L](वर्ण D) { प्रत्यागम गन्तव्यम्_अधस्तनम्(D) == L; }, From);
}

/// compare_numeric - Compare strings, handle embedded numbers.
पूर्णाङ्क StringRef::compare_numeric(StringRef RHS) ध्रुव {
  प्रति (size_t I = 0, E = std::min(आकार_2(), RHS.आकार_2()); I != E; ++I) {
    // Check for sequences of digits.
    यदि (isDigit(दत्त()[I]) && isDigit(RHS.दत्त()[I])) {
      // The longer sequence of numbers is considered larger.
      // This doesn't really handle prefixed zeros well.
      size_t J;
      प्रति (J = I + 1; J != E + 1; ++J) {
        तार्किक ld = J < आकार_2() && isDigit(दत्त()[J]);
        तार्किक rd = J < RHS.आकार_2() && isDigit(RHS.दत्त()[J]);
        यदि (ld != rd)
          प्रत्यागम rd ? -1 : 1;
        यदि (!rd)
          भङ्ग;
      }
      // The two number sequences have the same length (J-I), just memcmp them.
      यदि (पूर्णाङ्क Res = compare_स्मृतिः(दत्त() + I, RHS.दत्त() + I, J - I))
        प्रत्यागम Res < 0 ? -1 : 1;
      // Identical number sequences, continue search after the numbers.
      I = J - 1;
      अनुवर्त;
    }
    यदि (दत्त()[I] != RHS.दत्त()[I])
      प्रत्यागम (अचिह्नित वर्ण)दत्त()[I] < (अचिह्नित वर्ण)RHS.दत्त()[I] ? -1 : 1;
  }
  यदि (आकार_2() == RHS.आकार_2())
    प्रत्यागम 0;
  प्रत्यागम आकार_2() < RHS.आकार_2() ? -1 : 1;
}

// Compute the edit distance between the two given strings.
अचिह्नित StringRef::edit_distance(llvm::StringRef Other,
                                  तार्किक AllowReplacements,
                                  अचिह्नित MaxEditDistance) ध्रुव {
  प्रत्यागम llvm::ComputeEditDistance(ArrayRef(दत्त(), आकार_2()),
                                   ArrayRef(Other.दत्त(), Other.आकार_2()),
                                   AllowReplacements, MaxEditDistance);
}

अचिह्नित llvm::StringRef::edit_distance_insensitive(
    StringRef Other, तार्किक AllowReplacements, अचिह्नित MaxEditDistance) ध्रुव {
  प्रत्यागम llvm::ComputeMappedEditDistance(
      ArrayRef(दत्त(), आकार_2()), ArrayRef(Other.दत्त(), Other.आकार_2()),
      llvm::गन्तव्यम्_अधस्तनम्, AllowReplacements, MaxEditDistance);
}

//===----------------------------------------------------------------------===//
// String Operations
//===----------------------------------------------------------------------===//

std::string StringRef::अधस्तनम्() ध्रुव {
  प्रत्यागम std::string(map_iterator(begin(), गन्तव्यम्_अधस्तनम्),
                     map_iterator(अन्तः(), गन्तव्यम्_अधस्तनम्));
}

std::string StringRef::ऊर्ध्वस्थम्() ध्रुव {
  प्रत्यागम std::string(map_iterator(begin(), गन्तव्यम्_ऊर्ध्वस्थम्),
                     map_iterator(अन्तः(), गन्तव्यम्_ऊर्ध्वस्थम्));
}

//===----------------------------------------------------------------------===//
// String Searching
//===----------------------------------------------------------------------===//


/// find - Search for the first string \arg Str in the string.
///
/// \return - The index of the first occurrence of \arg Str, or npos if not
/// found.
size_t StringRef::find(StringRef Str, size_t From) ध्रुव {
  यदि (From > आकार_2())
    प्रत्यागम npos;

  ध्रुव वर्ण *Start = दत्त() + From;
  size_t आकार = आकार_2() - From;

  ध्रुव वर्ण *Needle = Str.दत्त();
  size_t N = Str.आकार_2();
  यदि (N == 0)
    प्रत्यागम From;
  यदि (आकार < N)
    प्रत्यागम npos;
  यदि (N == 1) {
    ध्रुव वर्ण *Ptr = (ध्रुव वर्ण *)::memchr(Start, Needle[0], आकार);
    प्रत्यागम Ptr == शून्यसूचकः ? npos : Ptr - दत्त();
  }

  ध्रुव वर्ण *Stop = Start + (आकार - N + 1);

  यदि (N == 2) {
    // Provide a fast path for newline finding (CRLF case) in InclusionRewriter.
    // Not the most optimized strategy, but getting memcmp inlined should be
    // good enough.
    कुरु {
      यदि (std::memcmp(Start, Needle, 2) == 0)
        प्रत्यागम Start - दत्त();
      ++Start;
    } यावत् (Start < Stop);
    प्रत्यागम npos;
  }

  // For short haystacks or unsupported needles fall back to the naive algorithm
  यदि (आकार < 16 || N > 255) {
    कुरु {
      यदि (std::memcmp(Start, Needle, N) == 0)
        प्रत्यागम Start - दत्त();
      ++Start;
    } यावत् (Start < Stop);
    प्रत्यागम npos;
  }

  // Build the bad char heuristic table, with uint8_t to reduce cache thrashing.
  uint8_t BadCharSkip[256];
  std::स्मृतिं_पूरय(BadCharSkip, N, 256);
  प्रति (अचिह्नित i = 0; i != N-1; ++i)
    BadCharSkip[(uint8_t)Str[i]] = N-1-i;

  कुरु {
    uint8_t चरमः = Start[N - 1];
    यदि (LLVM_UNLIKELY(चरमः == (uint8_t)Needle[N - 1]))
      यदि (std::memcmp(Start, Needle, N - 1) == 0)
        प्रत्यागम Start - दत्त();

    // Otherwise skip the appropriate number of bytes.
    Start += BadCharSkip[चरमः];
  } यावत् (Start < Stop);

  प्रत्यागम npos;
}

size_t StringRef::find_insensitive(StringRef Str, size_t From) ध्रुव {
  StringRef This = substr(From);
  यावत् (This.आकार_2() >= Str.आकार_2()) {
    यदि (This.starts_with_insensitive(Str))
      प्रत्यागम From;
    This = This.drop_front();
    ++From;
  }
  प्रत्यागम npos;
}

size_t StringRef::rfind_insensitive(वर्ण C, size_t From) ध्रुव {
  From = std::min(From, आकार_2());
  size_t i = From;
  यावत् (i != 0) {
    --i;
    यदि (गन्तव्यम्_अधस्तनम्(दत्त()[i]) == गन्तव्यम्_अधस्तनम्(C))
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
  size_t N = Str.आकार_2();
  यदि (N > आकार_2())
    प्रत्यागम npos;
  प्रति (size_t i = आकार_2() - N + 1, e = 0; i != e;) {
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
StringRef::आकार_type StringRef::find_first_सम्बन्धि(StringRef Chars,
                                              size_t From) ध्रुव {
  std::bitset<1 << CHAR_BIT> Char_द्वयङ्काः;
  प्रति (वर्ण C : Chars)
    Char_द्वयङ्काः.समुच्चयः((अचिह्नित वर्ण)C);

  प्रति (आकार_type i = std::min(From, आकार_2()), e = आकार_2(); i != e; ++i)
    यदि (Char_द्वयङ्काः.test((अचिह्नित वर्ण)दत्त()[i]))
      प्रत्यागम i;
  प्रत्यागम npos;
}

/// find_first_not_of - Find the first character in the string that is not
/// \arg C or npos if not found.
StringRef::आकार_type StringRef::find_first_not_सम्बन्धि(वर्ण C, size_t From) ध्रुव {
  प्रत्यागम std::string_view(*एतत्).find_first_not_सम्बन्धि(C, From);
}

/// find_first_not_of - Find the first character in the string that is not
/// in the string \arg Chars, or npos if not found.
///
/// Note: O(size() + Chars.size())
StringRef::आकार_type StringRef::find_first_not_सम्बन्धि(StringRef Chars,
                                                  size_t From) ध्रुव {
  std::bitset<1 << CHAR_BIT> Char_द्वयङ्काः;
  प्रति (वर्ण C : Chars)
    Char_द्वयङ्काः.समुच्चयः((अचिह्नित वर्ण)C);

  प्रति (आकार_type i = std::min(From, आकार_2()), e = आकार_2(); i != e; ++i)
    यदि (!Char_द्वयङ्काः.test((अचिह्नित वर्ण)दत्त()[i]))
      प्रत्यागम i;
  प्रत्यागम npos;
}

/// find_last_of - Find the last character in the string that is in \arg C,
/// or npos if not found.
///
/// Note: O(size() + Chars.size())
StringRef::आकार_type StringRef::find_चरमः_सम्बन्धि(StringRef Chars,
                                             size_t From) ध्रुव {
  std::bitset<1 << CHAR_BIT> Char_द्वयङ्काः;
  प्रति (वर्ण C : Chars)
    Char_द्वयङ्काः.समुच्चयः((अचिह्नित वर्ण)C);

  प्रति (आकार_type i = std::min(From, आकार_2()) - 1, e = -1; i != e; --i)
    यदि (Char_द्वयङ्काः.test((अचिह्नित वर्ण)दत्त()[i]))
      प्रत्यागम i;
  प्रत्यागम npos;
}

/// find_last_not_of - Find the last character in the string that is not
/// \arg C, or npos if not found.
StringRef::आकार_type StringRef::find_चरमः_not_सम्बन्धि(वर्ण C, size_t From) ध्रुव {
  प्रति (आकार_type i = std::min(From, आकार_2()) - 1, e = -1; i != e; --i)
    यदि (दत्त()[i] != C)
      प्रत्यागम i;
  प्रत्यागम npos;
}

/// find_last_not_of - Find the last character in the string that is not in
/// \arg Chars, or npos if not found.
///
/// Note: O(size() + Chars.size())
StringRef::आकार_type StringRef::find_चरमः_not_सम्बन्धि(StringRef Chars,
                                                 size_t From) ध्रुव {
  std::bitset<1 << CHAR_BIT> Char_द्वयङ्काः;
  प्रति (वर्ण C : Chars)
    Char_द्वयङ्काः.समुच्चयः((अचिह्नित वर्ण)C);

  प्रति (आकार_type i = std::min(From, आकार_2()) - 1, e = -1; i != e; --i)
    यदि (!Char_द्वयङ्काः.test((अचिह्नित वर्ण)दत्त()[i]))
      प्रत्यागम i;
  प्रत्यागम npos;
}

शून्य StringRef::विभज(Small_सदिशम्_Impl<StringRef> &A,
                      StringRef Separator, पूर्णाङ्क Max_विभज,
                      तार्किक Keep_रिक्तानि) ध्रुव {
  StringRef S = *एतत्;

  // Count down from MaxSplit. When MaxSplit is -1, this will just split
  // "forever". This doesn't support splitting more than 2^31 times
  // intentionally; if we ever want that we can make MaxSplit a 64-bit integer
  // but that seems unlikely to be useful.
  यावत् (Max_विभज-- != 0) {
    size_t Idx = S.find(Separator);
    यदि (Idx == npos)
      भङ्ग;

    // Push this split.
    यदि (Keep_रिक्तानि || Idx > 0)
      A.प्रक्षिप_पृष्ठभागः(S.खण्डिका(0, Idx));

    // Jump forward.
    S = S.substr(Idx + Separator.आकार_2());
  }

  // Push the tail.
  यदि (Keep_रिक्तानि || !S.रिक्तानि())
    A.प्रक्षिप_पृष्ठभागः(S);
}

शून्य StringRef::विभज(Small_सदिशम्_Impl<StringRef> &A, वर्ण Separator,
                      पूर्णाङ्क Max_विभज, तार्किक Keep_रिक्तानि) ध्रुव {
  StringRef S = *एतत्;

  // Count down from MaxSplit. When MaxSplit is -1, this will just split
  // "forever". This doesn't support splitting more than 2^31 times
  // intentionally; if we ever want that we can make MaxSplit a 64-bit integer
  // but that seems unlikely to be useful.
  यावत् (Max_विभज-- != 0) {
    size_t Idx = S.find(Separator);
    यदि (Idx == npos)
      भङ्ग;

    // Push this split.
    यदि (Keep_रिक्तानि || Idx > 0)
      A.प्रक्षिप_पृष्ठभागः(S.खण्डिका(0, Idx));

    // Jump forward.
    S = S.substr(Idx + 1);
  }

  // Push the tail.
  यदि (Keep_रिक्तानि || !S.रिक्तानि())
    A.प्रक्षिप_पृष्ठभागः(S);
}

//===----------------------------------------------------------------------===//
// Helpful Algorithms
//===----------------------------------------------------------------------===//

/// count - Return the number of non-overlapped occurrences of \arg Str in
/// the string.
size_t StringRef::गणनम्(StringRef Str) ध्रुव {
  size_t गणनम्_2 = 0;
  size_t Pos = 0;
  size_t N = Str.आकार_2();
  // TODO: For an empty `Str` we return 0 for legacy reasons. Consider changing
  //       this to `Length + 1` which is more in-line with the function
  //       description.
  यदि (!N)
    प्रत्यागम 0;
  यावत् ((Pos = find(Str, Pos)) != npos) {
    ++गणनम्_2;
    Pos += N;
  }
  प्रत्यागम गणनम्_2;
}

अचिह्नित llvm::getAutoSenseRadix(StringRef &Str) {
  यदि (Str.रिक्तानि())
    प्रत्यागम 10;

  यदि (Str.consume_front_insensitive("0x"))
    प्रत्यागम 16;

  यदि (Str.consume_front_insensitive("0b"))
    प्रत्यागम 2;

  यदि (Str.consume_front("0o"))
    प्रत्यागम 8;

  यदि (Str[0] == '0' && Str.आकार_2() > 1 && isDigit(Str[1])) {
    Str = Str.substr(1);
    प्रत्यागम 8;
  }

  प्रत्यागम 10;
}

तार्किक llvm::consumeUnsignedInteger(StringRef &Str, अचिह्नित Radix,
                                  अचिह्नित दीर्घ दीर्घ &परिणाम) {
  // Autosense radix if not specified.
  यदि (Radix == 0)
    Radix = getAutoSenseRadix(Str);

  // Empty strings (after the radix autosense) are invalid.
  यदि (Str.रिक्तानि()) प्रत्यागम सत्य;

  // Parse all the bytes of the string given this radix.  Watch for overflow.
  StringRef Str2 = Str;
  परिणाम = 0;
  यावत् (!Str2.रिक्तानि()) {
    अचिह्नित CharVal;
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
    अचिह्नित दीर्घ दीर्घ पूर्व_परिणाम = परिणाम;
    परिणाम = परिणाम * Radix + CharVal;

    // Check for overflow by shifting back and seeing if bits were lost.
    यदि (परिणाम / Radix < पूर्व_परिणाम)
      प्रत्यागम सत्य;

    Str2 = Str2.substr(1);
  }

  // We consider the operation a failure if no characters were consumed
  // successfully.
  यदि (Str.आकार_2() == Str2.आकार_2())
    प्रत्यागम सत्य;

  Str = Str2;
  प्रत्यागम असत्य;
}

तार्किक llvm::consumeSignedInteger(StringRef &Str, अचिह्नित Radix,
                                दीर्घ दीर्घ &परिणाम) {
  अचिह्नित दीर्घ दीर्घ ULLVal;

  // Handle positive strings first.
  यदि (!Str.starts_with("-")) {
    यदि (consumeUnsignedInteger(Str, Radix, ULLVal) ||
        // Check for value so large it overflows a signed value.
        (दीर्घ दीर्घ)ULLVal < 0)
      प्रत्यागम सत्य;
    परिणाम = ULLVal;
    प्रत्यागम असत्य;
  }

  // Get the positive part of the value.
  StringRef Str2 = Str.drop_front(1);
  यदि (consumeUnsignedInteger(Str2, Radix, ULLVal) ||
      // Reject values so large they'd overflow as negative signed, but allow
      // "-0".  This negates the unsigned so that the negative isn't undefined
      // on signed overflow.
      (दीर्घ दीर्घ)-ULLVal > 0)
    प्रत्यागम सत्य;

  Str = Str2;
  परिणाम = -ULLVal;
  प्रत्यागम असत्य;
}

/// GetAsUnsignedInteger - Workhorse method that converts a integer character
/// sequence of radix up to 36 to an unsigned long long value.
तार्किक llvm::getAsUnsignedInteger(StringRef Str, अचिह्नित Radix,
                                अचिह्नित दीर्घ दीर्घ &परिणाम) {
  यदि (consumeUnsignedInteger(Str, Radix, परिणाम))
    प्रत्यागम सत्य;

  // For getAsUnsignedInteger, we require the whole string to be consumed or
  // else we consider it a failure.
  प्रत्यागम !Str.रिक्तानि();
}

तार्किक llvm::getAsSignedInteger(StringRef Str, अचिह्नित Radix,
                              दीर्घ दीर्घ &परिणाम) {
  यदि (consumeSignedInteger(Str, Radix, परिणाम))
    प्रत्यागम सत्य;

  // For getAsSignedInteger, we require the whole string to be consumed or else
  // we consider it a failure.
  प्रत्यागम !Str.रिक्तानि();
}

तार्किक StringRef::consumeInteger(अचिह्नित Radix, APInt &परिणाम) {
  StringRef Str = *एतत्;

  // Autosense radix if not specified.
  यदि (Radix == 0)
    Radix = getAutoSenseRadix(Str);

  प्रतिज्ञा(Radix > 1 && Radix <= 36);

  // Empty strings (after the radix autosense) are invalid.
  यदि (Str.रिक्तानि()) प्रत्यागम सत्य;

  // Skip leading zeroes.  This can be a significant improvement if
  // it means we don't need > 64 bits.
  Str = Str.ltrim('0');

  // If it was nothing but zeroes....
  यदि (Str.रिक्तानि()) {
    परिणाम = APInt(64, 0);
    *एतत् = Str;
    प्रत्यागम असत्य;
  }

  अचिह्नित Log2Radix = 0;
  यावत् ((1U << Log2Radix) < Radix) Log2Radix++;
  तार्किक IsPowerOf2Radix = ((1U << Log2Radix) == Radix);

  // Initialize Result to a reasonable starting width (at least 64 bits),
  // but do not shrink it if it was already larger.
  अचिह्नित Bit_विस्तारः = std::max(64U, परिणाम.get_Bit_विस्तारः());

  यदि (परिणाम.get_Bit_विस्तारः() < Bit_विस्तारः) {
    परिणाम = परिणाम.zext(Bit_विस्तारः);
  }

  APInt RadixAP, CharAP; // unused unless !IsPowerOf2Radix
  यदि (!IsPowerOf2Radix) {
    // These must have the same bit-width as Result.
    RadixAP = APInt(Bit_विस्तारः, Radix);
    CharAP = APInt(Bit_विस्तारः, 0);
  }

  // Parse all the bytes of the string given this radix.
  परिणाम = 0;
  यावत् (!Str.रिक्तानि()) {
    अचिह्नित CharVal;
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
    यदि (परिणाम.get_सक्रिय_द्वयङ्काः() + Log2Radix > परिणाम.get_Bit_विस्तारः()) {
      अचिह्नित New_विस्तारः = परिणाम.get_Bit_विस्तारः() * 2;

      // Guard against overflow of the NewWidth itself
      यदि (New_विस्तारः < परिणाम.get_Bit_विस्तारः())
        प्रत्यागम सत्य;

      परिणाम = परिणाम.zext(New_विस्तारः);
      यदि (!IsPowerOf2Radix) {
        RadixAP = RadixAP.zext(New_विस्तारः);
        CharAP = CharAP.zext(New_विस्तारः);
      }
    }

    // Add in this character.
    यदि (IsPowerOf2Radix) {
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
  यदि (आकार_2() == Str.आकार_2())
    प्रत्यागम सत्य;

  *एतत् = Str;
  प्रत्यागम असत्य;
}

तार्किक StringRef::getAsInteger(अचिह्नित Radix, APInt &परिणाम) ध्रुव {
  StringRef Str = *एतत्;
  यदि (Str.consumeInteger(Radix, परिणाम))
    प्रत्यागम सत्य;

  // For getAsInteger, we require the whole string to be consumed or else we
  // consider it a failure.
  प्रत्यागम !Str.रिक्तानि();
}

तार्किक StringRef::getAsDouble(द्विगुण &परिणाम, तार्किक AllowInexact) ध्रुव {
  APFloat F(0.0);
  स्वचालित स्थितिः_Or_Err = F.convertFromString(*एतत्, APFloat::rm_निकटतमः_Ties_गन्तव्यम्_Even);
  यदि (error_गन्तव्यम्_Bool(स्थितिः_Or_Err.takeError()))
    प्रत्यागम सत्य;

  APFloat::op_स्थितिः स्थितिः = *स्थितिः_Or_Err;
  यदि (स्थितिः != APFloat::opOK) {
    यदि (!AllowInexact || !(स्थितिः & APFloat::opInexact))
      प्रत्यागम सत्य;
  }

  परिणाम = F.convert_गन्तव्यम्_Double();
  प्रत्यागम असत्य;
}

// Implementation of StringRef hashing.
hash_संहिता llvm::hash_मानम्(StringRef S) { प्रत्यागम hash_combine_range(S); }

अचिह्नित DenseMapInfo<StringRef, शून्य>::get_Hash_मानम्(StringRef Val) {
  प्रत्यागम (अचिह्नित)(hash_मानम्(Val));
}
