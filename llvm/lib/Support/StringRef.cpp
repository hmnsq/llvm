//===-- StringRef.cpp - Lightweight String References ---------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#అన్తర్భావ "llvm/ADT/StringRef.h"
#అన్తర్భావ "llvm/ADT/APFloat.h"
#అన్తర్భావ "llvm/ADT/APInt.h"
#అన్తర్భావ "llvm/ADT/Hashing.h"
#అన్తర్భావ "llvm/ADT/StringExtras.h"
#అన్తర్భావ "llvm/ADT/edit_distance.h"
#అన్తర్భావ "llvm/Support/Error.h"
#అన్తర్భావ <bitset>

ఉపయోగించి నామాకాశ llvm;

// strncasecmp() is not available on non-POSIX systems, so define an
// alternative function here.
స్థిర పూర్ణాంకం ascii_strncasecmp(StringRef LHS, StringRef RHS) {
  కొరకు (స్వయంచాలక [LC, RC] : zip_equal(LHS, RHS)) {
    గుర్తులేని అక్షరం LHC = గమ్యం_క్రింది(LC);
    గుర్తులేని అక్షరం RHC = గమ్యం_క్రింది(RC);
    ఒకవేళ (LHC != RHC)
      తిరిగి LHC < RHC ? -1 : 1;
  }
  తిరిగి 0;
}

పూర్ణాంకం StringRef::compare_insensitive(StringRef RHS) స్థిరరాశి {
  size_t Min = std::min(కొలత_2(), RHS.కొలత_2());
  ఒకవేళ (పూర్ణాంకం Res = ascii_strncasecmp(take_front(Min), RHS.take_front(Min)))
    తిరిగి Res;
  ఒకవేళ (కొలత_2() == RHS.కొలత_2())
    తిరిగి 0;
  తిరిగి కొలత_2() < RHS.కొలత_2() ? -1 : 1;
}

తార్కిక StringRef::starts_with_insensitive(StringRef ఉపసర్గ) స్థిరరాశి {
  తిరిగి కొలత_2() >= ఉపసర్గ.కొలత_2() &&
         ascii_strncasecmp(take_front(ఉపసర్గ.కొలత_2()), ఉపసర్గ) == 0;
}

తార్కిక StringRef::చివర్లు_with_insensitive(StringRef Suffix) స్థిరరాశి {
  తిరిగి కొలత_2() >= Suffix.కొలత_2() &&
         ascii_strncasecmp(take_వెనుక(Suffix.కొలత_2()), Suffix) == 0;
}

size_t StringRef::కనుగొను_insensitive(అక్షరం C, size_t From) స్థిరరాశి {
  అక్షరం L = గమ్యం_క్రింది(C);
  తిరిగి కనుగొను_if([L](అక్షరం D) { తిరిగి గమ్యం_క్రింది(D) == L; }, From);
}

/// compare_numeric - Compare strings, handle embedded numbers.
పూర్ణాంకం StringRef::compare_numeric(StringRef RHS) స్థిరరాశి {
  కొరకు (size_t I = 0, E = std::min(కొలత_2(), RHS.కొలత_2()); I != E; ++I) {
    // Check for sequences of digits.
    ఒకవేళ (isDigit(దత్తాంశం()[I]) && isDigit(RHS.దత్తాంశం()[I])) {
      // The longer sequence of numbers is considered larger.
      // This doesn't really handle prefixed zeros well.
      size_t J;
      కొరకు (J = I + 1; J != E + 1; ++J) {
        తార్కిక ld = J < కొలత_2() && isDigit(దత్తాంశం()[J]);
        తార్కిక rd = J < RHS.కొలత_2() && isDigit(RHS.దత్తాంశం()[J]);
        ఒకవేళ (ld != rd)
          తిరిగి rd ? -1 : 1;
        ఒకవేళ (!rd)
          ఆపు;
      }
      // The two number sequences have the same length (J-I), just memcmp them.
      ఒకవేళ (పూర్ణాంకం Res = compare_జ్ఞాపకం(దత్తాంశం() + I, RHS.దత్తాంశం() + I, J - I))
        తిరిగి Res < 0 ? -1 : 1;
      // Identical number sequences, continue search after the numbers.
      I = J - 1;
      కొనసాగు;
    }
    ఒకవేళ (దత్తాంశం()[I] != RHS.దత్తాంశం()[I])
      తిరిగి (గుర్తులేని అక్షరం)దత్తాంశం()[I] < (గుర్తులేని అక్షరం)RHS.దత్తాంశం()[I] ? -1 : 1;
  }
  ఒకవేళ (కొలత_2() == RHS.కొలత_2())
    తిరిగి 0;
  తిరిగి కొలత_2() < RHS.కొలత_2() ? -1 : 1;
}

// Compute the edit distance between the two given strings.
గుర్తులేని StringRef::edit_దూరం(llvm::StringRef Other,
                                  తార్కిక AllowReplacements,
                                  గుర్తులేని Max_Edit_దూరం) స్థిరరాశి {
  తిరిగి llvm::Compute_Edit_దూరం(ArrayRef(దత్తాంశం(), కొలత_2()),
                                   ArrayRef(Other.దత్తాంశం(), Other.కొలత_2()),
                                   AllowReplacements, Max_Edit_దూరం);
}

గుర్తులేని llvm::StringRef::edit_దూరం_insensitive(
    StringRef Other, తార్కిక AllowReplacements, గుర్తులేని Max_Edit_దూరం) స్థిరరాశి {
  తిరిగి llvm::Compute_Mapped_Edit_దూరం(
      ArrayRef(దత్తాంశం(), కొలత_2()), ArrayRef(Other.దత్తాంశం(), Other.కొలత_2()),
      llvm::గమ్యం_క్రింది, AllowReplacements, Max_Edit_దూరం);
}

//===----------------------------------------------------------------------===//
// String Operations
//===----------------------------------------------------------------------===//

std::string StringRef::క్రింది() స్థిరరాశి {
  తిరిగి std::string(map_iterator(begin(), గమ్యం_క్రింది),
                     map_iterator(చివర(), గమ్యం_క్రింది));
}

std::string StringRef::పై() స్థిరరాశి {
  తిరిగి std::string(map_iterator(begin(), గమ్యం_పై),
                     map_iterator(చివర(), గమ్యం_పై));
}

//===----------------------------------------------------------------------===//
// String Searching
//===----------------------------------------------------------------------===//


/// find - Search for the first string \arg Str in the string.
///
/// \return - The index of the first occurrence of \arg Str, or npos if not
/// found.
size_t StringRef::కనుగొను(StringRef Str, size_t From) స్థిరరాశి {
  ఒకవేళ (From > కొలత_2())
    తిరిగి npos;

  స్థిరరాశి అక్షరం *ప్రారంభం = దత్తాంశం() + From;
  size_t కొలత = కొలత_2() - From;

  స్థిరరాశి అక్షరం *Needle = Str.దత్తాంశం();
  size_t N = Str.కొలత_2();
  ఒకవేళ (N == 0)
    తిరిగి From;
  ఒకవేళ (కొలత < N)
    తిరిగి npos;
  ఒకవేళ (N == 1) {
    స్థిరరాశి అక్షరం *Ptr = (స్థిరరాశి అక్షరం *)::memchr(ప్రారంభం, Needle[0], కొలత);
    తిరిగి Ptr == శూన్యసూచిక ? npos : Ptr - దత్తాంశం();
  }

  స్థిరరాశి అక్షరం *Stop = ప్రారంభం + (కొలత - N + 1);

  ఒకవేళ (N == 2) {
    // Provide a fast path for newline finding (CRLF case) in InclusionRewriter.
    // Not the most optimized strategy, but getting memcmp inlined should be
    // good enough.
    చేయి {
      ఒకవేళ (std::memcmp(ప్రారంభం, Needle, 2) == 0)
        తిరిగి ప్రారంభం - దత్తాంశం();
      ++ప్రారంభం;
    } అయితే (ప్రారంభం < Stop);
    తిరిగి npos;
  }

  // For short haystacks or unsupported needles fall back to the naive algorithm
  ఒకవేళ (కొలత < 16 || N > 255) {
    చేయి {
      ఒకవేళ (std::memcmp(ప్రారంభం, Needle, N) == 0)
        తిరిగి ప్రారంభం - దత్తాంశం();
      ++ప్రారంభం;
    } అయితే (ప్రారంభం < Stop);
    తిరిగి npos;
  }

  // Build the bad char heuristic table, with uint8_t to reduce cache thrashing.
  uint8_t BadCharSkip[256];
  std::memset(BadCharSkip, N, 256);
  కొరకు (గుర్తులేని i = 0; i != N-1; ++i)
    BadCharSkip[(uint8_t)Str[i]] = N-1-i;

  చేయి {
    uint8_t చివరి = ప్రారంభం[N - 1];
    ఒకవేళ (LLVM_UNLIKELY(చివరి == (uint8_t)Needle[N - 1]))
      ఒకవేళ (std::memcmp(ప్రారంభం, Needle, N - 1) == 0)
        తిరిగి ప్రారంభం - దత్తాంశం();

    // Otherwise skip the appropriate number of bytes.
    ప్రారంభం += BadCharSkip[చివరి];
  } అయితే (ప్రారంభం < Stop);

  తిరిగి npos;
}

size_t StringRef::కనుగొను_insensitive(StringRef Str, size_t From) స్థిరరాశి {
  StringRef This = substr(From);
  అయితే (This.కొలత_2() >= Str.కొలత_2()) {
    ఒకవేళ (This.starts_with_insensitive(Str))
      తిరిగి From;
    This = This.drop_front();
    ++From;
  }
  తిరిగి npos;
}

size_t StringRef::rfind_insensitive(అక్షరం C, size_t From) స్థిరరాశి {
  From = std::min(From, కొలత_2());
  size_t i = From;
  అయితే (i != 0) {
    --i;
    ఒకవేళ (గమ్యం_క్రింది(దత్తాంశం()[i]) == గమ్యం_క్రింది(C))
      తిరిగి i;
  }
  తిరిగి npos;
}

/// rfind - Search for the last string \arg Str in the string.
///
/// \return - The index of the last occurrence of \arg Str, or npos if not
/// found.
size_t StringRef::rfind(StringRef Str) స్థిరరాశి {
  తిరిగి std::string_view(*ఇది).rfind(Str);
}

size_t StringRef::rfind_insensitive(StringRef Str) స్థిరరాశి {
  size_t N = Str.కొలత_2();
  ఒకవేళ (N > కొలత_2())
    తిరిగి npos;
  కొరకు (size_t i = కొలత_2() - N + 1, e = 0; i != e;) {
    --i;
    ఒకవేళ (substr(i, N).equals_insensitive(Str))
      తిరిగి i;
  }
  తిరిగి npos;
}

/// find_first_of - Find the first character in the string that is in \arg
/// Chars, or npos if not found.
///
/// Note: O(size() + Chars.size())
StringRef::కొలత_type StringRef::కనుగొను_మొదటి_యొక్క(StringRef Chars,
                                              size_t From) స్థిరరాశి {
  std::bitset<1 << CHAR_BIT> Char_బిట్లు;
  కొరకు (అక్షరం C : Chars)
    Char_బిట్లు.సమితి((గుర్తులేని అక్షరం)C);

  కొరకు (కొలత_type i = std::min(From, కొలత_2()), e = కొలత_2(); i != e; ++i)
    ఒకవేళ (Char_బిట్లు.test((గుర్తులేని అక్షరం)దత్తాంశం()[i]))
      తిరిగి i;
  తిరిగి npos;
}

/// find_first_not_of - Find the first character in the string that is not
/// \arg C or npos if not found.
StringRef::కొలత_type StringRef::కనుగొను_మొదటి_not_యొక్క(అక్షరం C, size_t From) స్థిరరాశి {
  తిరిగి std::string_view(*ఇది).కనుగొను_మొదటి_not_యొక్క(C, From);
}

/// find_first_not_of - Find the first character in the string that is not
/// in the string \arg Chars, or npos if not found.
///
/// Note: O(size() + Chars.size())
StringRef::కొలత_type StringRef::కనుగొను_మొదటి_not_యొక్క(StringRef Chars,
                                                  size_t From) స్థిరరాశి {
  std::bitset<1 << CHAR_BIT> Char_బిట్లు;
  కొరకు (అక్షరం C : Chars)
    Char_బిట్లు.సమితి((గుర్తులేని అక్షరం)C);

  కొరకు (కొలత_type i = std::min(From, కొలత_2()), e = కొలత_2(); i != e; ++i)
    ఒకవేళ (!Char_బిట్లు.test((గుర్తులేని అక్షరం)దత్తాంశం()[i]))
      తిరిగి i;
  తిరిగి npos;
}

/// find_last_of - Find the last character in the string that is in \arg C,
/// or npos if not found.
///
/// Note: O(size() + Chars.size())
StringRef::కొలత_type StringRef::కనుగొను_చివరి_యొక్క(StringRef Chars,
                                             size_t From) స్థిరరాశి {
  std::bitset<1 << CHAR_BIT> Char_బిట్లు;
  కొరకు (అక్షరం C : Chars)
    Char_బిట్లు.సమితి((గుర్తులేని అక్షరం)C);

  కొరకు (కొలత_type i = std::min(From, కొలత_2()) - 1, e = -1; i != e; --i)
    ఒకవేళ (Char_బిట్లు.test((గుర్తులేని అక్షరం)దత్తాంశం()[i]))
      తిరిగి i;
  తిరిగి npos;
}

/// find_last_not_of - Find the last character in the string that is not
/// \arg C, or npos if not found.
StringRef::కొలత_type StringRef::కనుగొను_చివరి_not_యొక్క(అక్షరం C, size_t From) స్థిరరాశి {
  కొరకు (కొలత_type i = std::min(From, కొలత_2()) - 1, e = -1; i != e; --i)
    ఒకవేళ (దత్తాంశం()[i] != C)
      తిరిగి i;
  తిరిగి npos;
}

/// find_last_not_of - Find the last character in the string that is not in
/// \arg Chars, or npos if not found.
///
/// Note: O(size() + Chars.size())
StringRef::కొలత_type StringRef::కనుగొను_చివరి_not_యొక్క(StringRef Chars,
                                                 size_t From) స్థిరరాశి {
  std::bitset<1 << CHAR_BIT> Char_బిట్లు;
  కొరకు (అక్షరం C : Chars)
    Char_బిట్లు.సమితి((గుర్తులేని అక్షరం)C);

  కొరకు (కొలత_type i = std::min(From, కొలత_2()) - 1, e = -1; i != e; --i)
    ఒకవేళ (!Char_బిట్లు.test((గుర్తులేని అక్షరం)దత్తాంశం()[i]))
      తిరిగి i;
  తిరిగి npos;
}

శూన్య StringRef::విభజించు(Small_సదిశ_Impl<StringRef> &A,
                      StringRef Separator, పూర్ణాంకం Max_విభజించు,
                      తార్కిక Keep_ఖాళీలు) స్థిరరాశి {
  StringRef S = *ఇది;

  // Count down from MaxSplit. When MaxSplit is -1, this will just split
  // "forever". This doesn't support splitting more than 2^31 times
  // intentionally; if we ever want that we can make MaxSplit a 64-bit integer
  // but that seems unlikely to be useful.
  అయితే (Max_విభజించు-- != 0) {
    size_t Idx = S.కనుగొను(Separator);
    ఒకవేళ (Idx == npos)
      ఆపు;

    // Push this split.
    ఒకవేళ (Keep_ఖాళీలు || Idx > 0)
      A.నెట్టు_వెనుక(S.ముక్క(0, Idx));

    // Jump forward.
    S = S.substr(Idx + Separator.కొలత_2());
  }

  // Push the tail.
  ఒకవేళ (Keep_ఖాళీలు || !S.ఖాళీలు())
    A.నెట్టు_వెనుక(S);
}

శూన్య StringRef::విభజించు(Small_సదిశ_Impl<StringRef> &A, అక్షరం Separator,
                      పూర్ణాంకం Max_విభజించు, తార్కిక Keep_ఖాళీలు) స్థిరరాశి {
  StringRef S = *ఇది;

  // Count down from MaxSplit. When MaxSplit is -1, this will just split
  // "forever". This doesn't support splitting more than 2^31 times
  // intentionally; if we ever want that we can make MaxSplit a 64-bit integer
  // but that seems unlikely to be useful.
  అయితే (Max_విభజించు-- != 0) {
    size_t Idx = S.కనుగొను(Separator);
    ఒకవేళ (Idx == npos)
      ఆపు;

    // Push this split.
    ఒకవేళ (Keep_ఖాళీలు || Idx > 0)
      A.నెట్టు_వెనుక(S.ముక్క(0, Idx));

    // Jump forward.
    S = S.substr(Idx + 1);
  }

  // Push the tail.
  ఒకవేళ (Keep_ఖాళీలు || !S.ఖాళీలు())
    A.నెట్టు_వెనుక(S);
}

//===----------------------------------------------------------------------===//
// Helpful Algorithms
//===----------------------------------------------------------------------===//

/// count - Return the number of non-overlapped occurrences of \arg Str in
/// the string.
size_t StringRef::లెక్క(StringRef Str) స్థిరరాశి {
  size_t లెక్క_2 = 0;
  size_t Pos = 0;
  size_t N = Str.కొలత_2();
  // TODO: For an empty `Str` we return 0 for legacy reasons. Consider changing
  //       this to `Length + 1` which is more in-line with the function
  //       description.
  ఒకవేళ (!N)
    తిరిగి 0;
  అయితే ((Pos = కనుగొను(Str, Pos)) != npos) {
    ++లెక్క_2;
    Pos += N;
  }
  తిరిగి లెక్క_2;
}

గుర్తులేని llvm::getAutoSenseRadix(StringRef &Str) {
  ఒకవేళ (Str.ఖాళీలు())
    తిరిగి 10;

  ఒకవేళ (Str.consume_front_insensitive("0x"))
    తిరిగి 16;

  ఒకవేళ (Str.consume_front_insensitive("0b"))
    తిరిగి 2;

  ఒకవేళ (Str.consume_front("0o"))
    తిరిగి 8;

  ఒకవేళ (Str[0] == '0' && Str.కొలత_2() > 1 && isDigit(Str[1])) {
    Str = Str.substr(1);
    తిరిగి 8;
  }

  తిరిగి 10;
}

తార్కిక llvm::consumeUnsignedInteger(StringRef &Str, గుర్తులేని Radix,
                                  గుర్తులేని దీర్ఘ దీర్ఘ &ఫలితం) {
  // Autosense radix if not specified.
  ఒకవేళ (Radix == 0)
    Radix = getAutoSenseRadix(Str);

  // Empty strings (after the radix autosense) are invalid.
  ఒకవేళ (Str.ఖాళీలు()) తిరిగి నిజం;

  // Parse all the bytes of the string given this radix.  Watch for overflow.
  StringRef Str2 = Str;
  ఫలితం = 0;
  అయితే (!Str2.ఖాళీలు()) {
    గుర్తులేని CharVal;
    ఒకవేళ (Str2[0] >= '0' && Str2[0] <= '9')
      CharVal = Str2[0] - '0';
    లేకపోతే ఒకవేళ (Str2[0] >= 'a' && Str2[0] <= 'z')
      CharVal = Str2[0] - 'a' + 10;
    లేకపోతే ఒకవేళ (Str2[0] >= 'A' && Str2[0] <= 'Z')
      CharVal = Str2[0] - 'A' + 10;
    లేకపోతే
      ఆపు;

    // If the parsed value is larger than the integer radix, we cannot
    // consume any more characters.
    ఒకవేళ (CharVal >= Radix)
      ఆపు;

    // Add in this character.
    గుర్తులేని దీర్ఘ దీర్ఘ Prev_ఫలితం = ఫలితం;
    ఫలితం = ఫలితం * Radix + CharVal;

    // Check for overflow by shifting back and seeing if bits were lost.
    ఒకవేళ (ఫలితం / Radix < Prev_ఫలితం)
      తిరిగి నిజం;

    Str2 = Str2.substr(1);
  }

  // We consider the operation a failure if no characters were consumed
  // successfully.
  ఒకవేళ (Str.కొలత_2() == Str2.కొలత_2())
    తిరిగి నిజం;

  Str = Str2;
  తిరిగి అబద్ధం;
}

తార్కిక llvm::consumeSignedInteger(StringRef &Str, గుర్తులేని Radix,
                                దీర్ఘ దీర్ఘ &ఫలితం) {
  గుర్తులేని దీర్ఘ దీర్ఘ ULLVal;

  // Handle positive strings first.
  ఒకవేళ (!Str.starts_with("-")) {
    ఒకవేళ (consumeUnsignedInteger(Str, Radix, ULLVal) ||
        // Check for value so large it overflows a signed value.
        (దీర్ఘ దీర్ఘ)ULLVal < 0)
      తిరిగి నిజం;
    ఫలితం = ULLVal;
    తిరిగి అబద్ధం;
  }

  // Get the positive part of the value.
  StringRef Str2 = Str.drop_front(1);
  ఒకవేళ (consumeUnsignedInteger(Str2, Radix, ULLVal) ||
      // Reject values so large they'd overflow as negative signed, but allow
      // "-0".  This negates the unsigned so that the negative isn't undefined
      // on signed overflow.
      (దీర్ఘ దీర్ఘ)-ULLVal > 0)
    తిరిగి నిజం;

  Str = Str2;
  ఫలితం = -ULLVal;
  తిరిగి అబద్ధం;
}

/// GetAsUnsignedInteger - Workhorse method that converts a integer character
/// sequence of radix up to 36 to an unsigned long long value.
తార్కిక llvm::getAsUnsignedInteger(StringRef Str, గుర్తులేని Radix,
                                గుర్తులేని దీర్ఘ దీర్ఘ &ఫలితం) {
  ఒకవేళ (consumeUnsignedInteger(Str, Radix, ఫలితం))
    తిరిగి నిజం;

  // For getAsUnsignedInteger, we require the whole string to be consumed or
  // else we consider it a failure.
  తిరిగి !Str.ఖాళీలు();
}

తార్కిక llvm::getAsSignedInteger(StringRef Str, గుర్తులేని Radix,
                              దీర్ఘ దీర్ఘ &ఫలితం) {
  ఒకవేళ (consumeSignedInteger(Str, Radix, ఫలితం))
    తిరిగి నిజం;

  // For getAsSignedInteger, we require the whole string to be consumed or else
  // we consider it a failure.
  తిరిగి !Str.ఖాళీలు();
}

తార్కిక StringRef::consumeInteger(గుర్తులేని Radix, APInt &ఫలితం) {
  StringRef Str = *ఇది;

  // Autosense radix if not specified.
  ఒకవేళ (Radix == 0)
    Radix = getAutoSenseRadix(Str);

  ప్రతిజ్ఞా(Radix > 1 && Radix <= 36);

  // Empty strings (after the radix autosense) are invalid.
  ఒకవేళ (Str.ఖాళీలు()) తిరిగి నిజం;

  // Skip leading zeroes.  This can be a significant improvement if
  // it means we don't need > 64 bits.
  Str = Str.ltrim('0');

  // If it was nothing but zeroes....
  ఒకవేళ (Str.ఖాళీలు()) {
    ఫలితం = APInt(64, 0);
    *ఇది = Str;
    తిరిగి అబద్ధం;
  }

  గుర్తులేని Log2Radix = 0;
  అయితే ((1U << Log2Radix) < Radix) Log2Radix++;
  తార్కిక IsPowerOf2Radix = ((1U << Log2Radix) == Radix);

  // Initialize Result to a reasonable starting width (at least 64 bits),
  // but do not shrink it if it was already larger.
  గుర్తులేని బిట్టు_వెడల్పు_2 = std::max(64U, ఫలితం.get_బిట్టు_వెడల్పు());

  ఒకవేళ (ఫలితం.get_బిట్టు_వెడల్పు() < బిట్టు_వెడల్పు_2) {
    ఫలితం = ఫలితం.zext(బిట్టు_వెడల్పు_2);
  }

  APInt RadixAP, CharAP; // unused unless !IsPowerOf2Radix
  ఒకవేళ (!IsPowerOf2Radix) {
    // These must have the same bit-width as Result.
    RadixAP = APInt(బిట్టు_వెడల్పు_2, Radix);
    CharAP = APInt(బిట్టు_వెడల్పు_2, 0);
  }

  // Parse all the bytes of the string given this radix.
  ఫలితం = 0;
  అయితే (!Str.ఖాళీలు()) {
    గుర్తులేని CharVal;
    ఒకవేళ (Str[0] >= '0' && Str[0] <= '9')
      CharVal = Str[0]-'0';
    లేకపోతే ఒకవేళ (Str[0] >= 'a' && Str[0] <= 'z')
      CharVal = Str[0]-'a'+10;
    లేకపోతే ఒకవేళ (Str[0] >= 'A' && Str[0] <= 'Z')
      CharVal = Str[0]-'A'+10;
    లేకపోతే
      ఆపు;

    // If the parsed value is larger than the integer radix, the string is
    // invalid.
    ఒకవేళ (CharVal >= Radix)
      ఆపు;

    // Check if one more digit will overflow, and grow if so.
    ఒకవేళ (ఫలితం.get_చురుకైన_బిట్లు() + Log2Radix > ఫలితం.get_బిట్టు_వెడల్పు()) {
      గుర్తులేని New_వెడల్పు = ఫలితం.get_బిట్టు_వెడల్పు() * 2;

      // Guard against overflow of the NewWidth itself
      ఒకవేళ (New_వెడల్పు < ఫలితం.get_బిట్టు_వెడల్పు())
        తిరిగి నిజం;

      ఫలితం = ఫలితం.zext(New_వెడల్పు);
      ఒకవేళ (!IsPowerOf2Radix) {
        RadixAP = RadixAP.zext(New_వెడల్పు);
        CharAP = CharAP.zext(New_వెడల్పు);
      }
    }

    // Add in this character.
    ఒకవేళ (IsPowerOf2Radix) {
      ఫలితం <<= Log2Radix;
      ఫలితం |= CharVal;
    } లేకపోతే {
      ఫలితం *= RadixAP;
      CharAP = CharVal;
      ఫలితం += CharAP;
    }

    Str = Str.substr(1);
  }

  // We consider the operation a failure if no characters were consumed
  // successfully.
  ఒకవేళ (కొలత_2() == Str.కొలత_2())
    తిరిగి నిజం;

  *ఇది = Str;
  తిరిగి అబద్ధం;
}

తార్కిక StringRef::getAsInteger(గుర్తులేని Radix, APInt &ఫలితం) స్థిరరాశి {
  StringRef Str = *ఇది;
  ఒకవేళ (Str.consumeInteger(Radix, ఫలితం))
    తిరిగి నిజం;

  // For getAsInteger, we require the whole string to be consumed or else we
  // consider it a failure.
  తిరిగి !Str.ఖాళీలు();
}

తార్కిక StringRef::getAsDouble(ద్విగుణ &ఫలితం, తార్కిక AllowInexact) స్థిరరాశి {
  APFloat F(0.0);
  స్వయంచాలక స్థితి_Or_Err = F.convertFromString(*ఇది, APFloat::rm_సమీప_Ties_గమ్యం_Even);
  ఒకవేళ (error_గమ్యం_Bool(స్థితి_Or_Err.takeError()))
    తిరిగి నిజం;

  APFloat::op_స్థితి స్థితి_2 = *స్థితి_Or_Err;
  ఒకవేళ (స్థితి_2 != APFloat::opOK) {
    ఒకవేళ (!AllowInexact || !(స్థితి_2 & APFloat::opInexact))
      తిరిగి నిజం;
  }

  ఫలితం = F.convert_గమ్యం_Double();
  తిరిగి అబద్ధం;
}

// Implementation of StringRef hashing.
hash_సంకేతం llvm::hash_విలువ(StringRef S) { తిరిగి hash_combine_range(S); }

గుర్తులేని DenseMapInfo<StringRef, శూన్య>::get_Hash_విలువ(StringRef Val) {
  తిరిగి (గుర్తులేని)(hash_విలువ(Val));
}
