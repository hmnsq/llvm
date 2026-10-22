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

کارول نطاق llvm;

// strncasecmp() is not available on non-POSIX systems, so define an
// alternative function here.
ساكن صحيح ascii_strncasecmp(StringRef LHS, StringRef RHS) {
  لكل (خپلکاری [LC, RC] : zip_equal(LHS, RHS)) {
    بینښه حرف LHC = منزل_ښکتنی(LC);
    بینښه حرف RHC = منزل_ښکتنی(RC);
    إذا (LHC != RHC)
      أرجع LHC < RHC ? -1 : 1;
  }
  أرجع 0;
}

صحيح StringRef::compare_insensitive(StringRef RHS) ثابت {
  size_t Min = std::min(اندازه_2(), RHS.اندازه_2());
  إذا (صحيح Res = ascii_strncasecmp(take_front(Min), RHS.take_front(Min)))
    أرجع Res;
  إذا (اندازه_2() == RHS.اندازه_2())
    أرجع 0;
  أرجع اندازه_2() < RHS.اندازه_2() ? -1 : 1;
}

منطقي StringRef::starts_with_insensitive(StringRef مختاړی) ثابت {
  أرجع اندازه_2() >= مختاړی.اندازه_2() &&
         ascii_strncasecmp(take_front(مختاړی.اندازه_2()), مختاړی) == 0;
}

منطقي StringRef::پایونه_with_insensitive(StringRef Suffix) ثابت {
  أرجع اندازه_2() >= Suffix.اندازه_2() &&
         ascii_strncasecmp(take_شا(Suffix.اندازه_2()), Suffix) == 0;
}

size_t StringRef::پیدا_کړه_insensitive(حرف C, size_t From) ثابت {
  حرف L = منزل_ښکتنی(C);
  أرجع پیدا_کړه_if([L](حرف D) { أرجع منزل_ښکتنی(D) == L; }, From);
}

/// compare_numeric - Compare strings, handle embedded numbers.
صحيح StringRef::compare_numeric(StringRef RHS) ثابت {
  لكل (size_t I = 0, E = std::min(اندازه_2(), RHS.اندازه_2()); I != E; ++I) {
    // Check for sequences of digits.
    إذا (isDigit(معلومات()[I]) && isDigit(RHS.معلومات()[I])) {
      // The longer sequence of numbers is considered larger.
      // This doesn't really handle prefixed zeros well.
      size_t J;
      لكل (J = I + 1; J != E + 1; ++J) {
        منطقي ld = J < اندازه_2() && isDigit(معلومات()[J]);
        منطقي rd = J < RHS.اندازه_2() && isDigit(RHS.معلومات()[J]);
        إذا (ld != rd)
          أرجع rd ? -1 : 1;
        إذا (!rd)
          اكسر;
      }
      // The two number sequences have the same length (J-I), just memcmp them.
      إذا (صحيح Res = compare_حافظه(معلومات() + I, RHS.معلومات() + I, J - I))
        أرجع Res < 0 ? -1 : 1;
      // Identical number sequences, continue search after the numbers.
      I = J - 1;
      استمر;
    }
    إذا (معلومات()[I] != RHS.معلومات()[I])
      أرجع (بینښه حرف)معلومات()[I] < (بینښه حرف)RHS.معلومات()[I] ? -1 : 1;
  }
  إذا (اندازه_2() == RHS.اندازه_2())
    أرجع 0;
  أرجع اندازه_2() < RHS.اندازه_2() ? -1 : 1;
}

// Compute the edit distance between the two given strings.
بینښه StringRef::edit_واټن(llvm::StringRef Other,
                                  منطقي AllowReplacements,
                                  بینښه Max_Edit_واټن) ثابت {
  أرجع llvm::Compute_Edit_واټن(ArrayRef(معلومات(), اندازه_2()),
                                   ArrayRef(Other.معلومات(), Other.اندازه_2()),
                                   AllowReplacements, Max_Edit_واټن);
}

بینښه llvm::StringRef::edit_واټن_insensitive(
    StringRef Other, منطقي AllowReplacements, بینښه Max_Edit_واټن) ثابت {
  أرجع llvm::Compute_Mapped_Edit_واټن(
      ArrayRef(معلومات(), اندازه_2()), ArrayRef(Other.معلومات(), Other.اندازه_2()),
      llvm::منزل_ښکتنی, AllowReplacements, Max_Edit_واټن);
}

//===----------------------------------------------------------------------===//
// String Operations
//===----------------------------------------------------------------------===//

std::string StringRef::ښکتنی() ثابت {
  أرجع std::string(map_iterator(begin(), منزل_ښکتنی),
                     map_iterator(پای_ټکی(), منزل_ښکتنی));
}

std::string StringRef::پاسنی() ثابت {
  أرجع std::string(map_iterator(begin(), منزل_پاسنی),
                     map_iterator(پای_ټکی(), منزل_پاسنی));
}

//===----------------------------------------------------------------------===//
// String Searching
//===----------------------------------------------------------------------===//


/// find - Search for the first string \arg Str in the string.
///
/// \return - The index of the first occurrence of \arg Str, or npos if not
/// found.
size_t StringRef::پیدا_کړه(StringRef Str, size_t From) ثابت {
  إذا (From > اندازه_2())
    أرجع npos;

  ثابت حرف *پیل = معلومات() + From;
  size_t اندازه = اندازه_2() - From;

  ثابت حرف *Needle = Str.معلومات();
  size_t N = Str.اندازه_2();
  إذا (N == 0)
    أرجع From;
  إذا (اندازه < N)
    أرجع npos;
  إذا (N == 1) {
    ثابت حرف *Ptr = (ثابت حرف *)::memchr(پیل, Needle[0], اندازه);
    أرجع Ptr == تش_اشاره ? npos : Ptr - معلومات();
  }

  ثابت حرف *Stop = پیل + (اندازه - N + 1);

  إذا (N == 2) {
    // Provide a fast path for newline finding (CRLF case) in InclusionRewriter.
    // Not the most optimized strategy, but getting memcmp inlined should be
    // good enough.
    افعل {
      إذا (std::memcmp(پیل, Needle, 2) == 0)
        أرجع پیل - معلومات();
      ++پیل;
    } طالما (پیل < Stop);
    أرجع npos;
  }

  // For short haystacks or unsupported needles fall back to the naive algorithm
  إذا (اندازه < 16 || N > 255) {
    افعل {
      إذا (std::memcmp(پیل, Needle, N) == 0)
        أرجع پیل - معلومات();
      ++پیل;
    } طالما (پیل < Stop);
    أرجع npos;
  }

  // Build the bad char heuristic table, with uint8_t to reduce cache thrashing.
  uint8_t BadCharSkip[256];
  std::memset(BadCharSkip, N, 256);
  لكل (بینښه i = 0; i != N-1; ++i)
    BadCharSkip[(uint8_t)Str[i]] = N-1-i;

  افعل {
    uint8_t وروستی = پیل[N - 1];
    إذا (LLVM_UNLIKELY(وروستی == (uint8_t)Needle[N - 1]))
      إذا (std::memcmp(پیل, Needle, N - 1) == 0)
        أرجع پیل - معلومات();

    // Otherwise skip the appropriate number of bytes.
    پیل += BadCharSkip[وروستی];
  } طالما (پیل < Stop);

  أرجع npos;
}

size_t StringRef::پیدا_کړه_insensitive(StringRef Str, size_t From) ثابت {
  StringRef This = substr(From);
  طالما (This.اندازه_2() >= Str.اندازه_2()) {
    إذا (This.starts_with_insensitive(Str))
      أرجع From;
    This = This.drop_front();
    ++From;
  }
  أرجع npos;
}

size_t StringRef::rfind_insensitive(حرف C, size_t From) ثابت {
  From = std::min(From, اندازه_2());
  size_t i = From;
  طالما (i != 0) {
    --i;
    إذا (منزل_ښکتنی(معلومات()[i]) == منزل_ښکتنی(C))
      أرجع i;
  }
  أرجع npos;
}

/// rfind - Search for the last string \arg Str in the string.
///
/// \return - The index of the last occurrence of \arg Str, or npos if not
/// found.
size_t StringRef::rfind(StringRef Str) ثابت {
  أرجع std::string_view(*هذا).rfind(Str);
}

size_t StringRef::rfind_insensitive(StringRef Str) ثابت {
  size_t N = Str.اندازه_2();
  إذا (N > اندازه_2())
    أرجع npos;
  لكل (size_t i = اندازه_2() - N + 1, e = 0; i != e;) {
    --i;
    إذا (substr(i, N).equals_insensitive(Str))
      أرجع i;
  }
  أرجع npos;
}

/// find_first_of - Find the first character in the string that is in \arg
/// Chars, or npos if not found.
///
/// Note: O(size() + Chars.size())
StringRef::اندازه_type StringRef::پیدا_کړه_لومړی_د(StringRef Chars,
                                              size_t From) ثابت {
  std::bitset<1 << CHAR_BIT> Char_بټونه;
  لكل (حرف C : Chars)
    Char_بټونه.سېټ((بینښه حرف)C);

  لكل (اندازه_type i = std::min(From, اندازه_2()), e = اندازه_2(); i != e; ++i)
    إذا (Char_بټونه.test((بینښه حرف)معلومات()[i]))
      أرجع i;
  أرجع npos;
}

/// find_first_not_of - Find the first character in the string that is not
/// \arg C or npos if not found.
StringRef::اندازه_type StringRef::پیدا_کړه_لومړی_not_د(حرف C, size_t From) ثابت {
  أرجع std::string_view(*هذا).پیدا_کړه_لومړی_not_د(C, From);
}

/// find_first_not_of - Find the first character in the string that is not
/// in the string \arg Chars, or npos if not found.
///
/// Note: O(size() + Chars.size())
StringRef::اندازه_type StringRef::پیدا_کړه_لومړی_not_د(StringRef Chars,
                                                  size_t From) ثابت {
  std::bitset<1 << CHAR_BIT> Char_بټونه;
  لكل (حرف C : Chars)
    Char_بټونه.سېټ((بینښه حرف)C);

  لكل (اندازه_type i = std::min(From, اندازه_2()), e = اندازه_2(); i != e; ++i)
    إذا (!Char_بټونه.test((بینښه حرف)معلومات()[i]))
      أرجع i;
  أرجع npos;
}

/// find_last_of - Find the last character in the string that is in \arg C,
/// or npos if not found.
///
/// Note: O(size() + Chars.size())
StringRef::اندازه_type StringRef::پیدا_کړه_وروستی_د(StringRef Chars,
                                             size_t From) ثابت {
  std::bitset<1 << CHAR_BIT> Char_بټونه;
  لكل (حرف C : Chars)
    Char_بټونه.سېټ((بینښه حرف)C);

  لكل (اندازه_type i = std::min(From, اندازه_2()) - 1, e = -1; i != e; --i)
    إذا (Char_بټونه.test((بینښه حرف)معلومات()[i]))
      أرجع i;
  أرجع npos;
}

/// find_last_not_of - Find the last character in the string that is not
/// \arg C, or npos if not found.
StringRef::اندازه_type StringRef::پیدا_کړه_وروستی_not_د(حرف C, size_t From) ثابت {
  لكل (اندازه_type i = std::min(From, اندازه_2()) - 1, e = -1; i != e; --i)
    إذا (معلومات()[i] != C)
      أرجع i;
  أرجع npos;
}

/// find_last_not_of - Find the last character in the string that is not in
/// \arg Chars, or npos if not found.
///
/// Note: O(size() + Chars.size())
StringRef::اندازه_type StringRef::پیدا_کړه_وروستی_not_د(StringRef Chars,
                                                 size_t From) ثابت {
  std::bitset<1 << CHAR_BIT> Char_بټونه;
  لكل (حرف C : Chars)
    Char_بټونه.سېټ((بینښه حرف)C);

  لكل (اندازه_type i = std::min(From, اندازه_2()) - 1, e = -1; i != e; --i)
    إذا (!Char_بټونه.test((بینښه حرف)معلومات()[i]))
      أرجع i;
  أرجع npos;
}

فراغ StringRef::وېشل(Small_ویکټور_Impl<StringRef> &A,
                      StringRef Separator, صحيح Max_وېشل,
                      منطقي Keep_تشه) ثابت {
  StringRef S = *هذا;

  // Count down from MaxSplit. When MaxSplit is -1, this will just split
  // "forever". This doesn't support splitting more than 2^31 times
  // intentionally; if we ever want that we can make MaxSplit a 64-bit integer
  // but that seems unlikely to be useful.
  طالما (Max_وېشل-- != 0) {
    size_t Idx = S.پیدا_کړه(Separator);
    إذا (Idx == npos)
      اكسر;

    // Push this split.
    إذا (Keep_تشه || Idx > 0)
      A.ټېل_شا(S.ټوټه(0, Idx));

    // Jump forward.
    S = S.substr(Idx + Separator.اندازه_2());
  }

  // Push the tail.
  إذا (Keep_تشه || !S.تشه())
    A.ټېل_شا(S);
}

فراغ StringRef::وېشل(Small_ویکټور_Impl<StringRef> &A, حرف Separator,
                      صحيح Max_وېشل, منطقي Keep_تشه) ثابت {
  StringRef S = *هذا;

  // Count down from MaxSplit. When MaxSplit is -1, this will just split
  // "forever". This doesn't support splitting more than 2^31 times
  // intentionally; if we ever want that we can make MaxSplit a 64-bit integer
  // but that seems unlikely to be useful.
  طالما (Max_وېشل-- != 0) {
    size_t Idx = S.پیدا_کړه(Separator);
    إذا (Idx == npos)
      اكسر;

    // Push this split.
    إذا (Keep_تشه || Idx > 0)
      A.ټېل_شا(S.ټوټه(0, Idx));

    // Jump forward.
    S = S.substr(Idx + 1);
  }

  // Push the tail.
  إذا (Keep_تشه || !S.تشه())
    A.ټېل_شا(S);
}

//===----------------------------------------------------------------------===//
// Helpful Algorithms
//===----------------------------------------------------------------------===//

/// count - Return the number of non-overlapped occurrences of \arg Str in
/// the string.
size_t StringRef::شمېر(StringRef Str) ثابت {
  size_t شمېر_2 = 0;
  size_t Pos = 0;
  size_t N = Str.اندازه_2();
  // TODO: For an empty `Str` we return 0 for legacy reasons. Consider changing
  //       this to `Length + 1` which is more in-line with the function
  //       description.
  إذا (!N)
    أرجع 0;
  طالما ((Pos = پیدا_کړه(Str, Pos)) != npos) {
    ++شمېر_2;
    Pos += N;
  }
  أرجع شمېر_2;
}

بینښه llvm::getAutoSenseRadix(StringRef &Str) {
  إذا (Str.تشه())
    أرجع 10;

  إذا (Str.consume_front_insensitive("0x"))
    أرجع 16;

  إذا (Str.consume_front_insensitive("0b"))
    أرجع 2;

  إذا (Str.consume_front("0o"))
    أرجع 8;

  إذا (Str[0] == '0' && Str.اندازه_2() > 1 && isDigit(Str[1])) {
    Str = Str.substr(1);
    أرجع 8;
  }

  أرجع 10;
}

منطقي llvm::consumeUnsignedInteger(StringRef &Str, بینښه Radix,
                                  بینښه اوږد اوږد &پایله) {
  // Autosense radix if not specified.
  إذا (Radix == 0)
    Radix = getAutoSenseRadix(Str);

  // Empty strings (after the radix autosense) are invalid.
  إذا (Str.تشه()) أرجع ریښتیا;

  // Parse all the bytes of the string given this radix.  Watch for overflow.
  StringRef Str2 = Str;
  پایله = 0;
  طالما (!Str2.تشه()) {
    بینښه CharVal;
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
    بینښه اوږد اوږد Prev_پایله = پایله;
    پایله = پایله * Radix + CharVal;

    // Check for overflow by shifting back and seeing if bits were lost.
    إذا (پایله / Radix < Prev_پایله)
      أرجع ریښتیا;

    Str2 = Str2.substr(1);
  }

  // We consider the operation a failure if no characters were consumed
  // successfully.
  إذا (Str.اندازه_2() == Str2.اندازه_2())
    أرجع ریښتیا;

  Str = Str2;
  أرجع خطأ;
}

منطقي llvm::consumeSignedInteger(StringRef &Str, بینښه Radix,
                                اوږد اوږد &پایله) {
  بینښه اوږد اوږد ULLVal;

  // Handle positive strings first.
  إذا (!Str.starts_with("-")) {
    إذا (consumeUnsignedInteger(Str, Radix, ULLVal) ||
        // Check for value so large it overflows a signed value.
        (اوږد اوږد)ULLVal < 0)
      أرجع ریښتیا;
    پایله = ULLVal;
    أرجع خطأ;
  }

  // Get the positive part of the value.
  StringRef Str2 = Str.drop_front(1);
  إذا (consumeUnsignedInteger(Str2, Radix, ULLVal) ||
      // Reject values so large they'd overflow as negative signed, but allow
      // "-0".  This negates the unsigned so that the negative isn't undefined
      // on signed overflow.
      (اوږد اوږد)-ULLVal > 0)
    أرجع ریښتیا;

  Str = Str2;
  پایله = -ULLVal;
  أرجع خطأ;
}

/// GetAsUnsignedInteger - Workhorse method that converts a integer character
/// sequence of radix up to 36 to an unsigned long long value.
منطقي llvm::getAsUnsignedInteger(StringRef Str, بینښه Radix,
                                بینښه اوږد اوږد &پایله) {
  إذا (consumeUnsignedInteger(Str, Radix, پایله))
    أرجع ریښتیا;

  // For getAsUnsignedInteger, we require the whole string to be consumed or
  // else we consider it a failure.
  أرجع !Str.تشه();
}

منطقي llvm::getAsSignedInteger(StringRef Str, بینښه Radix,
                              اوږد اوږد &پایله) {
  إذا (consumeSignedInteger(Str, Radix, پایله))
    أرجع ریښتیا;

  // For getAsSignedInteger, we require the whole string to be consumed or else
  // we consider it a failure.
  أرجع !Str.تشه();
}

منطقي StringRef::consumeInteger(بینښه Radix, APInt &پایله) {
  StringRef Str = *هذا;

  // Autosense radix if not specified.
  إذا (Radix == 0)
    Radix = getAutoSenseRadix(Str);

  أكد(Radix > 1 && Radix <= 36);

  // Empty strings (after the radix autosense) are invalid.
  إذا (Str.تشه()) أرجع ریښتیا;

  // Skip leading zeroes.  This can be a significant improvement if
  // it means we don't need > 64 bits.
  Str = Str.ltrim('0');

  // If it was nothing but zeroes....
  إذا (Str.تشه()) {
    پایله = APInt(64, 0);
    *هذا = Str;
    أرجع خطأ;
  }

  بینښه Log2Radix = 0;
  طالما ((1U << Log2Radix) < Radix) Log2Radix++;
  منطقي IsPowerOf2Radix = ((1U << Log2Radix) == Radix);

  // Initialize Result to a reasonable starting width (at least 64 bits),
  // but do not shrink it if it was already larger.
  بینښه بټ_پلنوالی_2 = std::max(64U, پایله.get_بټ_پلنوالی());

  إذا (پایله.get_بټ_پلنوالی() < بټ_پلنوالی_2) {
    پایله = پایله.zext(بټ_پلنوالی_2);
  }

  APInt RadixAP, CharAP; // unused unless !IsPowerOf2Radix
  إذا (!IsPowerOf2Radix) {
    // These must have the same bit-width as Result.
    RadixAP = APInt(بټ_پلنوالی_2, Radix);
    CharAP = APInt(بټ_پلنوالی_2, 0);
  }

  // Parse all the bytes of the string given this radix.
  پایله = 0;
  طالما (!Str.تشه()) {
    بینښه CharVal;
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
    إذا (پایله.get_فعال_بټونه() + Log2Radix > پایله.get_بټ_پلنوالی()) {
      بینښه New_پلنوالی = پایله.get_بټ_پلنوالی() * 2;

      // Guard against overflow of the NewWidth itself
      إذا (New_پلنوالی < پایله.get_بټ_پلنوالی())
        أرجع ریښتیا;

      پایله = پایله.zext(New_پلنوالی);
      إذا (!IsPowerOf2Radix) {
        RadixAP = RadixAP.zext(New_پلنوالی);
        CharAP = CharAP.zext(New_پلنوالی);
      }
    }

    // Add in this character.
    إذا (IsPowerOf2Radix) {
      پایله <<= Log2Radix;
      پایله |= CharVal;
    } وإلا {
      پایله *= RadixAP;
      CharAP = CharVal;
      پایله += CharAP;
    }

    Str = Str.substr(1);
  }

  // We consider the operation a failure if no characters were consumed
  // successfully.
  إذا (اندازه_2() == Str.اندازه_2())
    أرجع ریښتیا;

  *هذا = Str;
  أرجع خطأ;
}

منطقي StringRef::getAsInteger(بینښه Radix, APInt &پایله) ثابت {
  StringRef Str = *هذا;
  إذا (Str.consumeInteger(Radix, پایله))
    أرجع ریښتیا;

  // For getAsInteger, we require the whole string to be consumed or else we
  // consider it a failure.
  أرجع !Str.تشه();
}

منطقي StringRef::getAsDouble(مزدوج &پایله, منطقي AllowInexact) ثابت {
  APFloat F(0.0);
  خپلکاری حالت_Or_Err = F.convertFromString(*هذا, APFloat::rm_تر_ټولو_نږدې_Ties_منزل_Even);
  إذا (error_منزل_Bool(حالت_Or_Err.takeError()))
    أرجع ریښتیا;

  APFloat::op_حالت حالت = *حالت_Or_Err;
  إذا (حالت != APFloat::opOK) {
    إذا (!AllowInexact || !(حالت & APFloat::opInexact))
      أرجع ریښتیا;
  }

  پایله = F.convert_منزل_Double();
  أرجع خطأ;
}

// Implementation of StringRef hashing.
hash_کوډ llvm::hash_ارزښت(StringRef S) { أرجع hash_combine_range(S); }

بینښه DenseMapInfo<StringRef, فراغ>::get_Hash_ارزښت(StringRef Val) {
  أرجع (بینښه)(hash_ارزښت(Val));
}
