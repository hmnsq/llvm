//===-- StringRef.cpp - Lightweight String References ---------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#অন্তর্ভাব "llvm/ADT/StringRef.h"
#অন্তর্ভাব "llvm/ADT/APFloat.h"
#অন্তর্ভাব "llvm/ADT/APInt.h"
#অন্তর্ভাব "llvm/ADT/Hashing.h"
#অন্তর্ভাব "llvm/ADT/StringExtras.h"
#অন্তর্ভাব "llvm/ADT/edit_distance.h"
#অন্তর্ভাব "llvm/Support/Error.h"
#অন্তর্ভাব <bitset>

using নামাকাশ llvm;

// strncasecmp() is not available on non-POSIX systems, so define an
// alternative function here.
স্থির পূর্ণসংখ্যা ascii_strncasecmp(StringRef LHS, StringRef RHS) {
  জন্য (auto [LC, RC] : zip_equal(LHS, RHS)) {
    unsigned অক্ষর LHC = toLower(LC);
    unsigned অক্ষর RHC = toLower(RC);
    যদি (LHC != RHC)
      ফেরত LHC < RHC ? -1 : 1;
  }
  ফেরত 0;
}

পূর্ণসংখ্যা StringRef::compare_insensitive(StringRef RHS) ধ্রুবক {
  size_t Min = std::min(আকার_3(), RHS.আকার_3());
  যদি (পূর্ণসংখ্যা Res = ascii_strncasecmp(take_front(Min), RHS.take_front(Min)))
    ফেরত Res;
  যদি (আকার_3() == RHS.আকার_3())
    ফেরত 0;
  ফেরত আকার_3() < RHS.আকার_3() ? -1 : 1;
}

যৌক্তিক StringRef::starts_with_insensitive(StringRef Prefix) ধ্রুবক {
  ফেরত আকার_3() >= Prefix.আকার_3() &&
         ascii_strncasecmp(take_front(Prefix.আকার_3()), Prefix) == 0;
}

যৌক্তিক StringRef::ends_with_insensitive(StringRef Suffix) ধ্রুবক {
  ফেরত আকার_3() >= Suffix.আকার_3() &&
         ascii_strncasecmp(take_back(Suffix.আকার_3()), Suffix) == 0;
}

size_t StringRef::find_insensitive(অক্ষর C, size_t From) ধ্রুবক {
  অক্ষর L = toLower(C);
  ফেরত find_if([L](অক্ষর D) { ফেরত toLower(D) == L; }, From);
}

/// compare_numeric - Compare strings, handle embedded numbers.
পূর্ণসংখ্যা StringRef::compare_numeric(StringRef RHS) ধ্রুবক {
  জন্য (size_t I = 0, E = std::min(আকার_3(), RHS.আকার_3()); I != E; ++I) {
    // Check for sequences of digits.
    যদি (isDigit(তথ্য()[I]) && isDigit(RHS.তথ্য()[I])) {
      // The longer sequence of numbers is considered larger.
      // This doesn't really handle prefixed zeros well.
      size_t J;
      জন্য (J = I + 1; J != E + 1; ++J) {
        যৌক্তিক ld = J < আকার_3() && isDigit(তথ্য()[J]);
        যৌক্তিক rd = J < RHS.আকার_3() && isDigit(RHS.তথ্য()[J]);
        যদি (ld != rd)
          ফেরত rd ? -1 : 1;
        যদি (!rd)
          বিরতি;
      }
      // The two number sequences have the same length (J-I), just memcmp them.
      যদি (পূর্ণসংখ্যা Res = compareMemory(তথ্য() + I, RHS.তথ্য() + I, J - I))
        ফেরত Res < 0 ? -1 : 1;
      // Identical number sequences, continue search after the numbers.
      I = J - 1;
      চালু;
    }
    যদি (তথ্য()[I] != RHS.তথ্য()[I])
      ফেরত (unsigned অক্ষর)তথ্য()[I] < (unsigned অক্ষর)RHS.তথ্য()[I] ? -1 : 1;
  }
  যদি (আকার_3() == RHS.আকার_3())
    ফেরত 0;
  ফেরত আকার_3() < RHS.আকার_3() ? -1 : 1;
}

// Compute the edit distance between the two given strings.
unsigned StringRef::edit_distance(llvm::StringRef Other,
                                  যৌক্তিক AllowReplacements,
                                  unsigned MaxEditDistance) ধ্রুবক {
  ফেরত llvm::ComputeEditDistance(ArrayRef(তথ্য(), আকার_3()),
                                   ArrayRef(Other.তথ্য(), Other.আকার_3()),
                                   AllowReplacements, MaxEditDistance);
}

unsigned llvm::StringRef::edit_distance_insensitive(
    StringRef Other, যৌক্তিক AllowReplacements, unsigned MaxEditDistance) ধ্রুবক {
  ফেরত llvm::ComputeMappedEditDistance(
      ArrayRef(তথ্য(), আকার_3()), ArrayRef(Other.তথ্য(), Other.আকার_3()),
      llvm::toLower, AllowReplacements, MaxEditDistance);
}

//===----------------------------------------------------------------------===//
// String Operations
//===----------------------------------------------------------------------===//

std::string StringRef::lower() ধ্রুবক {
  ফেরত std::string(map_iterator(begin(), toLower),
                     map_iterator(end(), toLower));
}

std::string StringRef::upper() ধ্রুবক {
  ফেরত std::string(map_iterator(begin(), toUpper),
                     map_iterator(end(), toUpper));
}

//===----------------------------------------------------------------------===//
// String Searching
//===----------------------------------------------------------------------===//


/// find - Search for the first string \arg Str in the string.
///
/// \return - The index of the first occurrence of \arg Str, or npos if not
/// found.
size_t StringRef::find(StringRef Str, size_t From) ধ্রুবক {
  যদি (From > আকার_3())
    ফেরত npos;

  ধ্রুবক অক্ষর *Start = তথ্য() + From;
  size_t আকার_2 = আকার_3() - From;

  ধ্রুবক অক্ষর *Needle = Str.তথ্য();
  size_t N = Str.আকার_3();
  যদি (N == 0)
    ফেরত From;
  যদি (আকার_2 < N)
    ফেরত npos;
  যদি (N == 1) {
    ধ্রুবক অক্ষর *Ptr = (ধ্রুবক অক্ষর *)::memchr(Start, Needle[0], আকার_2);
    ফেরত Ptr == nullptr ? npos : Ptr - তথ্য();
  }

  ধ্রুবক অক্ষর *Stop = Start + (আকার_2 - N + 1);

  যদি (N == 2) {
    // Provide a fast path for newline finding (CRLF case) in InclusionRewriter.
    // Not the most optimized strategy, but getting memcmp inlined should be
    // good enough.
    করো {
      যদি (std::memcmp(Start, Needle, 2) == 0)
        ফেরত Start - তথ্য();
      ++Start;
    } যখন (Start < Stop);
    ফেরত npos;
  }

  // For short haystacks or unsupported needles fall back to the naive algorithm
  যদি (আকার_2 < 16 || N > 255) {
    করো {
      যদি (std::memcmp(Start, Needle, N) == 0)
        ফেরত Start - তথ্য();
      ++Start;
    } যখন (Start < Stop);
    ফেরত npos;
  }

  // Build the bad char heuristic table, with uint8_t to reduce cache thrashing.
  uint8_t BadCharSkip[256];
  std::memset(BadCharSkip, N, 256);
  জন্য (unsigned i = 0; i != N-1; ++i)
    BadCharSkip[(uint8_t)Str[i]] = N-1-i;

  করো {
    uint8_t Last = Start[N - 1];
    যদি (LLVM_UNLIKELY(Last == (uint8_t)Needle[N - 1]))
      যদি (std::memcmp(Start, Needle, N - 1) == 0)
        ফেরত Start - তথ্য();

    // Otherwise skip the appropriate number of bytes.
    Start += BadCharSkip[Last];
  } যখন (Start < Stop);

  ফেরত npos;
}

size_t StringRef::find_insensitive(StringRef Str, size_t From) ধ্রুবক {
  StringRef This = substr(From);
  যখন (This.আকার_3() >= Str.আকার_3()) {
    যদি (This.starts_with_insensitive(Str))
      ফেরত From;
    This = This.drop_front();
    ++From;
  }
  ফেরত npos;
}

size_t StringRef::rfind_insensitive(অক্ষর C, size_t From) ধ্রুবক {
  From = std::min(From, আকার_3());
  size_t i = From;
  যখন (i != 0) {
    --i;
    যদি (toLower(তথ্য()[i]) == toLower(C))
      ফেরত i;
  }
  ফেরত npos;
}

/// rfind - Search for the last string \arg Str in the string.
///
/// \return - The index of the last occurrence of \arg Str, or npos if not
/// found.
size_t StringRef::rfind(StringRef Str) ধ্রুবক {
  ফেরত std::string_view(*এই).rfind(Str);
}

size_t StringRef::rfind_insensitive(StringRef Str) ধ্রুবক {
  size_t N = Str.আকার_3();
  যদি (N > আকার_3())
    ফেরত npos;
  জন্য (size_t i = আকার_3() - N + 1, e = 0; i != e;) {
    --i;
    যদি (substr(i, N).equals_insensitive(Str))
      ফেরত i;
  }
  ফেরত npos;
}

/// find_first_of - Find the first character in the string that is in \arg
/// Chars, or npos if not found.
///
/// Note: O(size() + Chars.size())
StringRef::আকার_type StringRef::find_first_of(StringRef Chars,
                                              size_t From) ধ্রুবক {
  std::bitset<1 << CHAR_BIT> CharBits;
  জন্য (অক্ষর C : Chars)
    CharBits.set((unsigned অক্ষর)C);

  জন্য (আকার_type i = std::min(From, আকার_3()), e = আকার_3(); i != e; ++i)
    যদি (CharBits.test((unsigned অক্ষর)তথ্য()[i]))
      ফেরত i;
  ফেরত npos;
}

/// find_first_not_of - Find the first character in the string that is not
/// \arg C or npos if not found.
StringRef::আকার_type StringRef::find_first_not_of(অক্ষর C, size_t From) ধ্রুবক {
  ফেরত std::string_view(*এই).find_first_not_of(C, From);
}

/// find_first_not_of - Find the first character in the string that is not
/// in the string \arg Chars, or npos if not found.
///
/// Note: O(size() + Chars.size())
StringRef::আকার_type StringRef::find_first_not_of(StringRef Chars,
                                                  size_t From) ধ্রুবক {
  std::bitset<1 << CHAR_BIT> CharBits;
  জন্য (অক্ষর C : Chars)
    CharBits.set((unsigned অক্ষর)C);

  জন্য (আকার_type i = std::min(From, আকার_3()), e = আকার_3(); i != e; ++i)
    যদি (!CharBits.test((unsigned অক্ষর)তথ্য()[i]))
      ফেরত i;
  ফেরত npos;
}

/// find_last_of - Find the last character in the string that is in \arg C,
/// or npos if not found.
///
/// Note: O(size() + Chars.size())
StringRef::আকার_type StringRef::find_last_of(StringRef Chars,
                                             size_t From) ধ্রুবক {
  std::bitset<1 << CHAR_BIT> CharBits;
  জন্য (অক্ষর C : Chars)
    CharBits.set((unsigned অক্ষর)C);

  জন্য (আকার_type i = std::min(From, আকার_3()) - 1, e = -1; i != e; --i)
    যদি (CharBits.test((unsigned অক্ষর)তথ্য()[i]))
      ফেরত i;
  ফেরত npos;
}

/// find_last_not_of - Find the last character in the string that is not
/// \arg C, or npos if not found.
StringRef::আকার_type StringRef::find_last_not_of(অক্ষর C, size_t From) ধ্রুবক {
  জন্য (আকার_type i = std::min(From, আকার_3()) - 1, e = -1; i != e; --i)
    যদি (তথ্য()[i] != C)
      ফেরত i;
  ফেরত npos;
}

/// find_last_not_of - Find the last character in the string that is not in
/// \arg Chars, or npos if not found.
///
/// Note: O(size() + Chars.size())
StringRef::আকার_type StringRef::find_last_not_of(StringRef Chars,
                                                 size_t From) ধ্রুবক {
  std::bitset<1 << CHAR_BIT> CharBits;
  জন্য (অক্ষর C : Chars)
    CharBits.set((unsigned অক্ষর)C);

  জন্য (আকার_type i = std::min(From, আকার_3()) - 1, e = -1; i != e; --i)
    যদি (!CharBits.test((unsigned অক্ষর)তথ্য()[i]))
      ফেরত i;
  ফেরত npos;
}

শূন্য StringRef::split(SmallVectorImpl<StringRef> &A,
                      StringRef Separator, পূর্ণসংখ্যা MaxSplit,
                      যৌক্তিক KeepEmpty) ধ্রুবক {
  StringRef S = *এই;

  // Count down from MaxSplit. When MaxSplit is -1, this will just split
  // "forever". This doesn't support splitting more than 2^31 times
  // intentionally; if we ever want that we can make MaxSplit a 64-bit integer
  // but that seems unlikely to be useful.
  যখন (MaxSplit-- != 0) {
    size_t Idx = S.find(Separator);
    যদি (Idx == npos)
      বিরতি;

    // Push this split.
    যদি (KeepEmpty || Idx > 0)
      A.ঠেলো_back(S.slice(0, Idx));

    // Jump forward.
    S = S.substr(Idx + Separator.আকার_3());
  }

  // Push the tail.
  যদি (KeepEmpty || !S.empty())
    A.ঠেলো_back(S);
}

শূন্য StringRef::split(SmallVectorImpl<StringRef> &A, অক্ষর Separator,
                      পূর্ণসংখ্যা MaxSplit, যৌক্তিক KeepEmpty) ধ্রুবক {
  StringRef S = *এই;

  // Count down from MaxSplit. When MaxSplit is -1, this will just split
  // "forever". This doesn't support splitting more than 2^31 times
  // intentionally; if we ever want that we can make MaxSplit a 64-bit integer
  // but that seems unlikely to be useful.
  যখন (MaxSplit-- != 0) {
    size_t Idx = S.find(Separator);
    যদি (Idx == npos)
      বিরতি;

    // Push this split.
    যদি (KeepEmpty || Idx > 0)
      A.ঠেলো_back(S.slice(0, Idx));

    // Jump forward.
    S = S.substr(Idx + 1);
  }

  // Push the tail.
  যদি (KeepEmpty || !S.empty())
    A.ঠেলো_back(S);
}

//===----------------------------------------------------------------------===//
// Helpful Algorithms
//===----------------------------------------------------------------------===//

/// count - Return the number of non-overlapped occurrences of \arg Str in
/// the string.
size_t StringRef::গুনতি(StringRef Str) ধ্রুবক {
  size_t গুনতি_2 = 0;
  size_t Pos = 0;
  size_t N = Str.আকার_3();
  // TODO: For an empty `Str` we return 0 for legacy reasons. Consider changing
  //       this to `Length + 1` which is more in-line with the function
  //       description.
  যদি (!N)
    ফেরত 0;
  যখন ((Pos = find(Str, Pos)) != npos) {
    ++গুনতি_2;
    Pos += N;
  }
  ফেরত গুনতি_2;
}

unsigned llvm::getAutoSenseRadix(StringRef &Str) {
  যদি (Str.empty())
    ফেরত 10;

  যদি (Str.consume_front_insensitive("0x"))
    ফেরত 16;

  যদি (Str.consume_front_insensitive("0b"))
    ফেরত 2;

  যদি (Str.consume_front("0o"))
    ফেরত 8;

  যদি (Str[0] == '0' && Str.আকার_3() > 1 && isDigit(Str[1])) {
    Str = Str.substr(1);
    ফেরত 8;
  }

  ফেরত 10;
}

যৌক্তিক llvm::consumeUnsignedInteger(StringRef &Str, unsigned Radix,
                                  unsigned long long &ফলাফল) {
  // Autosense radix if not specified.
  যদি (Radix == 0)
    Radix = getAutoSenseRadix(Str);

  // Empty strings (after the radix autosense) are invalid.
  যদি (Str.empty()) ফেরত সত্য;

  // Parse all the bytes of the string given this radix.  Watch for overflow.
  StringRef Str2 = Str;
  ফলাফল = 0;
  যখন (!Str2.empty()) {
    unsigned CharVal;
    যদি (Str2[0] >= '0' && Str2[0] <= '9')
      CharVal = Str2[0] - '0';
    অন্যথা যদি (Str2[0] >= 'a' && Str2[0] <= 'z')
      CharVal = Str2[0] - 'a' + 10;
    অন্যথা যদি (Str2[0] >= 'A' && Str2[0] <= 'Z')
      CharVal = Str2[0] - 'A' + 10;
    অন্যথা
      বিরতি;

    // If the parsed value is larger than the integer radix, we cannot
    // consume any more characters.
    যদি (CharVal >= Radix)
      বিরতি;

    // Add in this character.
    unsigned long long পূর্ব_ফলাফল = ফলাফল;
    ফলাফল = ফলাফল * Radix + CharVal;

    // Check for overflow by shifting back and seeing if bits were lost.
    যদি (ফলাফল / Radix < পূর্ব_ফলাফল)
      ফেরত সত্য;

    Str2 = Str2.substr(1);
  }

  // We consider the operation a failure if no characters were consumed
  // successfully.
  যদি (Str.আকার_3() == Str2.আকার_3())
    ফেরত সত্য;

  Str = Str2;
  ফেরত মিথ্যা;
}

যৌক্তিক llvm::consumeSignedInteger(StringRef &Str, unsigned Radix,
                                long long &ফলাফল) {
  unsigned long long ULLVal;

  // Handle positive strings first.
  যদি (!Str.starts_with("-")) {
    যদি (consumeUnsignedInteger(Str, Radix, ULLVal) ||
        // Check for value so large it overflows a signed value.
        (long long)ULLVal < 0)
      ফেরত সত্য;
    ফলাফল = ULLVal;
    ফেরত মিথ্যা;
  }

  // Get the positive part of the value.
  StringRef Str2 = Str.drop_front(1);
  যদি (consumeUnsignedInteger(Str2, Radix, ULLVal) ||
      // Reject values so large they'd overflow as negative signed, but allow
      // "-0".  This negates the unsigned so that the negative isn't undefined
      // on signed overflow.
      (long long)-ULLVal > 0)
    ফেরত সত্য;

  Str = Str2;
  ফলাফল = -ULLVal;
  ফেরত মিথ্যা;
}

/// GetAsUnsignedInteger - Workhorse method that converts a integer character
/// sequence of radix up to 36 to an unsigned long long value.
যৌক্তিক llvm::getAsUnsignedInteger(StringRef Str, unsigned Radix,
                                unsigned long long &ফলাফল) {
  যদি (consumeUnsignedInteger(Str, Radix, ফলাফল))
    ফেরত সত্য;

  // For getAsUnsignedInteger, we require the whole string to be consumed or
  // else we consider it a failure.
  ফেরত !Str.empty();
}

যৌক্তিক llvm::getAsSignedInteger(StringRef Str, unsigned Radix,
                              long long &ফলাফল) {
  যদি (consumeSignedInteger(Str, Radix, ফলাফল))
    ফেরত সত্য;

  // For getAsSignedInteger, we require the whole string to be consumed or else
  // we consider it a failure.
  ফেরত !Str.empty();
}

যৌক্তিক StringRef::consumeInteger(unsigned Radix, APInt &ফলাফল) {
  StringRef Str = *এই;

  // Autosense radix if not specified.
  যদি (Radix == 0)
    Radix = getAutoSenseRadix(Str);

  প্রতিজ্ঞা(Radix > 1 && Radix <= 36);

  // Empty strings (after the radix autosense) are invalid.
  যদি (Str.empty()) ফেরত সত্য;

  // Skip leading zeroes.  This can be a significant improvement if
  // it means we don't need > 64 bits.
  Str = Str.ltrim('0');

  // If it was nothing but zeroes....
  যদি (Str.empty()) {
    ফলাফল = APInt(64, 0);
    *এই = Str;
    ফেরত মিথ্যা;
  }

  unsigned Log2Radix = 0;
  যখন ((1U << Log2Radix) < Radix) Log2Radix++;
  যৌক্তিক IsPowerOf2Radix = ((1U << Log2Radix) == Radix);

  // Initialize Result to a reasonable starting width (at least 64 bits),
  // but do not shrink it if it was already larger.
  unsigned BitWidth = std::max(64U, ফলাফল.getBitWidth());

  যদি (ফলাফল.getBitWidth() < BitWidth) {
    ফলাফল = ফলাফল.zext(BitWidth);
  }

  APInt RadixAP, CharAP; // unused unless !IsPowerOf2Radix
  যদি (!IsPowerOf2Radix) {
    // These must have the same bit-width as Result.
    RadixAP = APInt(BitWidth, Radix);
    CharAP = APInt(BitWidth, 0);
  }

  // Parse all the bytes of the string given this radix.
  ফলাফল = 0;
  যখন (!Str.empty()) {
    unsigned CharVal;
    যদি (Str[0] >= '0' && Str[0] <= '9')
      CharVal = Str[0]-'0';
    অন্যথা যদি (Str[0] >= 'a' && Str[0] <= 'z')
      CharVal = Str[0]-'a'+10;
    অন্যথা যদি (Str[0] >= 'A' && Str[0] <= 'Z')
      CharVal = Str[0]-'A'+10;
    অন্যথা
      বিরতি;

    // If the parsed value is larger than the integer radix, the string is
    // invalid.
    যদি (CharVal >= Radix)
      বিরতি;

    // Check if one more digit will overflow, and grow if so.
    যদি (ফলাফল.getActiveBits() + Log2Radix > ফলাফল.getBitWidth()) {
      unsigned NewWidth = ফলাফল.getBitWidth() * 2;

      // Guard against overflow of the NewWidth itself
      যদি (NewWidth < ফলাফল.getBitWidth())
        ফেরত সত্য;

      ফলাফল = ফলাফল.zext(NewWidth);
      যদি (!IsPowerOf2Radix) {
        RadixAP = RadixAP.zext(NewWidth);
        CharAP = CharAP.zext(NewWidth);
      }
    }

    // Add in this character.
    যদি (IsPowerOf2Radix) {
      ফলাফল <<= Log2Radix;
      ফলাফল |= CharVal;
    } অন্যথা {
      ফলাফল *= RadixAP;
      CharAP = CharVal;
      ফলাফল += CharAP;
    }

    Str = Str.substr(1);
  }

  // We consider the operation a failure if no characters were consumed
  // successfully.
  যদি (আকার_3() == Str.আকার_3())
    ফেরত সত্য;

  *এই = Str;
  ফেরত মিথ্যা;
}

যৌক্তিক StringRef::getAsInteger(unsigned Radix, APInt &ফলাফল) ধ্রুবক {
  StringRef Str = *এই;
  যদি (Str.consumeInteger(Radix, ফলাফল))
    ফেরত সত্য;

  // For getAsInteger, we require the whole string to be consumed or else we
  // consider it a failure.
  ফেরত !Str.empty();
}

যৌক্তিক StringRef::getAsDouble(দ্বিগুণ &ফলাফল, যৌক্তিক AllowInexact) ধ্রুবক {
  APFloat F(0.0);
  auto StatusOrErr = F.convertFromString(*এই, APFloat::rmNearestTiesToEven);
  যদি (errorToBool(StatusOrErr.takeError()))
    ফেরত সত্য;

  APFloat::opStatus Status = *StatusOrErr;
  যদি (Status != APFloat::opOK) {
    যদি (!AllowInexact || !(Status & APFloat::opInexact))
      ফেরত সত্য;
  }

  ফলাফল = F.convertToDouble();
  ফেরত মিথ্যা;
}

// Implementation of StringRef hashing.
hash_code llvm::hash_মান(StringRef S) { ফেরত hash_combine_range(S); }

unsigned DenseMapInfo<StringRef, শূন্য>::get_Hash_মান(StringRef Val) {
  ফেরত (unsigned)(hash_মান(Val));
}
