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

استعمال نطاق llvm;

// strncasecmp() is not available on non-POSIX systems, so define an
// alternative function here.
ساكن صحيح ascii_strncasecmp(StringRef LHS, StringRef RHS) {
  لكل (خودڪار [LC, RC] : zip_equal(LHS, RHS)) {
    بنانشان حرف LHC = منزل_هيٺاهون(LC);
    بنانشان حرف RHC = منزل_هيٺاهون(RC);
    إذا (LHC != RHC)
      أرجع LHC < RHC ? -1 : 1;
  }
  أرجع 0;
}

صحيح StringRef::compare_insensitive(StringRef RHS) ثابت {
  size_t Min = std::min(سائيز_2(), RHS.سائيز_2());
  إذا (صحيح Res = ascii_strncasecmp(take_front(Min), RHS.take_front(Min)))
    أرجع Res;
  إذا (سائيز_2() == RHS.سائيز_2())
    أرجع 0;
  أرجع سائيز_2() < RHS.سائيز_2() ? -1 : 1;
}

منطقي StringRef::starts_with_insensitive(StringRef اڳياڙي) ثابت {
  أرجع سائيز_2() >= اڳياڙي.سائيز_2() &&
         ascii_strncasecmp(take_front(اڳياڙي.سائيز_2()), اڳياڙي) == 0;
}

منطقي StringRef::پڇاڙيون_with_insensitive(StringRef Suffix) ثابت {
  أرجع سائيز_2() >= Suffix.سائيز_2() &&
         ascii_strncasecmp(take_پٺ(Suffix.سائيز_2()), Suffix) == 0;
}

size_t StringRef::ڳوليو_insensitive(حرف C, size_t From) ثابت {
  حرف L = منزل_هيٺاهون(C);
  أرجع ڳوليو_if([L](حرف D) { أرجع منزل_هيٺاهون(D) == L; }, From);
}

/// compare_numeric - Compare strings, handle embedded numbers.
صحيح StringRef::compare_numeric(StringRef RHS) ثابت {
  لكل (size_t I = 0, E = std::min(سائيز_2(), RHS.سائيز_2()); I != E; ++I) {
    // Check for sequences of digits.
    إذا (isDigit(معلومات()[I]) && isDigit(RHS.معلومات()[I])) {
      // The longer sequence of numbers is considered larger.
      // This doesn't really handle prefixed zeros well.
      size_t J;
      لكل (J = I + 1; J != E + 1; ++J) {
        منطقي ld = J < سائيز_2() && isDigit(معلومات()[J]);
        منطقي rd = J < RHS.سائيز_2() && isDigit(RHS.معلومات()[J]);
        إذا (ld != rd)
          أرجع rd ? -1 : 1;
        إذا (!rd)
          اكسر;
      }
      // The two number sequences have the same length (J-I), just memcmp them.
      إذا (صحيح Res = compare_يادگيري(معلومات() + I, RHS.معلومات() + I, J - I))
        أرجع Res < 0 ? -1 : 1;
      // Identical number sequences, continue search after the numbers.
      I = J - 1;
      استمر;
    }
    إذا (معلومات()[I] != RHS.معلومات()[I])
      أرجع (بنانشان حرف)معلومات()[I] < (بنانشان حرف)RHS.معلومات()[I] ? -1 : 1;
  }
  إذا (سائيز_2() == RHS.سائيز_2())
    أرجع 0;
  أرجع سائيز_2() < RHS.سائيز_2() ? -1 : 1;
}

// Compute the edit distance between the two given strings.
بنانشان StringRef::edit_مفاصلو(llvm::StringRef Other,
                                  منطقي AllowReplacements,
                                  بنانشان Max_Edit_مفاصلو) ثابت {
  أرجع llvm::Compute_Edit_مفاصلو(ArrayRef(معلومات(), سائيز_2()),
                                   ArrayRef(Other.معلومات(), Other.سائيز_2()),
                                   AllowReplacements, Max_Edit_مفاصلو);
}

بنانشان llvm::StringRef::edit_مفاصلو_insensitive(
    StringRef Other, منطقي AllowReplacements, بنانشان Max_Edit_مفاصلو) ثابت {
  أرجع llvm::Compute_Mapped_Edit_مفاصلو(
      ArrayRef(معلومات(), سائيز_2()), ArrayRef(Other.معلومات(), Other.سائيز_2()),
      llvm::منزل_هيٺاهون, AllowReplacements, Max_Edit_مفاصلو);
}

//===----------------------------------------------------------------------===//
// String Operations
//===----------------------------------------------------------------------===//

std::string StringRef::هيٺاهون() ثابت {
  أرجع std::string(map_iterator(begin(), منزل_هيٺاهون),
                     map_iterator(پڇاڙي(), منزل_هيٺاهون));
}

std::string StringRef::مٿانهون() ثابت {
  أرجع std::string(map_iterator(begin(), منزل_مٿانهون),
                     map_iterator(پڇاڙي(), منزل_مٿانهون));
}

//===----------------------------------------------------------------------===//
// String Searching
//===----------------------------------------------------------------------===//


/// find - Search for the first string \arg Str in the string.
///
/// \return - The index of the first occurrence of \arg Str, or npos if not
/// found.
size_t StringRef::ڳوليو(StringRef Str, size_t From) ثابت {
  إذا (From > سائيز_2())
    أرجع npos;

  ثابت حرف *شروعات = معلومات() + From;
  size_t سائيز = سائيز_2() - From;

  ثابت حرف *Needle = Str.معلومات();
  size_t N = Str.سائيز_2();
  إذا (N == 0)
    أرجع From;
  إذا (سائيز < N)
    أرجع npos;
  إذا (N == 1) {
    ثابت حرف *Ptr = (ثابت حرف *)::memchr(شروعات, Needle[0], سائيز);
    أرجع Ptr == خالي_اشارو ? npos : Ptr - معلومات();
  }

  ثابت حرف *Stop = شروعات + (سائيز - N + 1);

  إذا (N == 2) {
    // Provide a fast path for newline finding (CRLF case) in InclusionRewriter.
    // Not the most optimized strategy, but getting memcmp inlined should be
    // good enough.
    افعل {
      إذا (std::memcmp(شروعات, Needle, 2) == 0)
        أرجع شروعات - معلومات();
      ++شروعات;
    } طالما (شروعات < Stop);
    أرجع npos;
  }

  // For short haystacks or unsupported needles fall back to the naive algorithm
  إذا (سائيز < 16 || N > 255) {
    افعل {
      إذا (std::memcmp(شروعات, Needle, N) == 0)
        أرجع شروعات - معلومات();
      ++شروعات;
    } طالما (شروعات < Stop);
    أرجع npos;
  }

  // Build the bad char heuristic table, with uint8_t to reduce cache thrashing.
  uint8_t BadCharSkip[256];
  std::memset(BadCharSkip, N, 256);
  لكل (بنانشان i = 0; i != N-1; ++i)
    BadCharSkip[(uint8_t)Str[i]] = N-1-i;

  افعل {
    uint8_t آخري = شروعات[N - 1];
    إذا (LLVM_UNLIKELY(آخري == (uint8_t)Needle[N - 1]))
      إذا (std::memcmp(شروعات, Needle, N - 1) == 0)
        أرجع شروعات - معلومات();

    // Otherwise skip the appropriate number of bytes.
    شروعات += BadCharSkip[آخري];
  } طالما (شروعات < Stop);

  أرجع npos;
}

size_t StringRef::ڳوليو_insensitive(StringRef Str, size_t From) ثابت {
  StringRef This = substr(From);
  طالما (This.سائيز_2() >= Str.سائيز_2()) {
    إذا (This.starts_with_insensitive(Str))
      أرجع From;
    This = This.drop_front();
    ++From;
  }
  أرجع npos;
}

size_t StringRef::rfind_insensitive(حرف C, size_t From) ثابت {
  From = std::min(From, سائيز_2());
  size_t i = From;
  طالما (i != 0) {
    --i;
    إذا (منزل_هيٺاهون(معلومات()[i]) == منزل_هيٺاهون(C))
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
  size_t N = Str.سائيز_2();
  إذا (N > سائيز_2())
    أرجع npos;
  لكل (size_t i = سائيز_2() - N + 1, e = 0; i != e;) {
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
StringRef::سائيز_type StringRef::ڳوليو_پهريون_جو(StringRef Chars,
                                              size_t From) ثابت {
  std::bitset<1 << CHAR_BIT> Char_بٽ;
  لكل (حرف C : Chars)
    Char_بٽ.سيٽ((بنانشان حرف)C);

  لكل (سائيز_type i = std::min(From, سائيز_2()), e = سائيز_2(); i != e; ++i)
    إذا (Char_بٽ.test((بنانشان حرف)معلومات()[i]))
      أرجع i;
  أرجع npos;
}

/// find_first_not_of - Find the first character in the string that is not
/// \arg C or npos if not found.
StringRef::سائيز_type StringRef::ڳوليو_پهريون_not_جو(حرف C, size_t From) ثابت {
  أرجع std::string_view(*هذا).ڳوليو_پهريون_not_جو(C, From);
}

/// find_first_not_of - Find the first character in the string that is not
/// in the string \arg Chars, or npos if not found.
///
/// Note: O(size() + Chars.size())
StringRef::سائيز_type StringRef::ڳوليو_پهريون_not_جو(StringRef Chars,
                                                  size_t From) ثابت {
  std::bitset<1 << CHAR_BIT> Char_بٽ;
  لكل (حرف C : Chars)
    Char_بٽ.سيٽ((بنانشان حرف)C);

  لكل (سائيز_type i = std::min(From, سائيز_2()), e = سائيز_2(); i != e; ++i)
    إذا (!Char_بٽ.test((بنانشان حرف)معلومات()[i]))
      أرجع i;
  أرجع npos;
}

/// find_last_of - Find the last character in the string that is in \arg C,
/// or npos if not found.
///
/// Note: O(size() + Chars.size())
StringRef::سائيز_type StringRef::ڳوليو_آخري_جو(StringRef Chars,
                                             size_t From) ثابت {
  std::bitset<1 << CHAR_BIT> Char_بٽ;
  لكل (حرف C : Chars)
    Char_بٽ.سيٽ((بنانشان حرف)C);

  لكل (سائيز_type i = std::min(From, سائيز_2()) - 1, e = -1; i != e; --i)
    إذا (Char_بٽ.test((بنانشان حرف)معلومات()[i]))
      أرجع i;
  أرجع npos;
}

/// find_last_not_of - Find the last character in the string that is not
/// \arg C, or npos if not found.
StringRef::سائيز_type StringRef::ڳوليو_آخري_not_جو(حرف C, size_t From) ثابت {
  لكل (سائيز_type i = std::min(From, سائيز_2()) - 1, e = -1; i != e; --i)
    إذا (معلومات()[i] != C)
      أرجع i;
  أرجع npos;
}

/// find_last_not_of - Find the last character in the string that is not in
/// \arg Chars, or npos if not found.
///
/// Note: O(size() + Chars.size())
StringRef::سائيز_type StringRef::ڳوليو_آخري_not_جو(StringRef Chars,
                                                 size_t From) ثابت {
  std::bitset<1 << CHAR_BIT> Char_بٽ;
  لكل (حرف C : Chars)
    Char_بٽ.سيٽ((بنانشان حرف)C);

  لكل (سائيز_type i = std::min(From, سائيز_2()) - 1, e = -1; i != e; --i)
    إذا (!Char_بٽ.test((بنانشان حرف)معلومات()[i]))
      أرجع i;
  أرجع npos;
}

فراغ StringRef::ورهايو(Small_ويڪٽر_Impl<StringRef> &A,
                      StringRef Separator, صحيح Max_ورهايو,
                      منطقي Keep_خالي_جايون) ثابت {
  StringRef S = *هذا;

  // Count down from MaxSplit. When MaxSplit is -1, this will just split
  // "forever". This doesn't support splitting more than 2^31 times
  // intentionally; if we ever want that we can make MaxSplit a 64-bit integer
  // but that seems unlikely to be useful.
  طالما (Max_ورهايو-- != 0) {
    size_t Idx = S.ڳوليو(Separator);
    إذا (Idx == npos)
      اكسر;

    // Push this split.
    إذا (Keep_خالي_جايون || Idx > 0)
      A.ڌڪو_پٺ(S.ٽڪرو(0, Idx));

    // Jump forward.
    S = S.substr(Idx + Separator.سائيز_2());
  }

  // Push the tail.
  إذا (Keep_خالي_جايون || !S.خالي_جايون())
    A.ڌڪو_پٺ(S);
}

فراغ StringRef::ورهايو(Small_ويڪٽر_Impl<StringRef> &A, حرف Separator,
                      صحيح Max_ورهايو, منطقي Keep_خالي_جايون) ثابت {
  StringRef S = *هذا;

  // Count down from MaxSplit. When MaxSplit is -1, this will just split
  // "forever". This doesn't support splitting more than 2^31 times
  // intentionally; if we ever want that we can make MaxSplit a 64-bit integer
  // but that seems unlikely to be useful.
  طالما (Max_ورهايو-- != 0) {
    size_t Idx = S.ڳوليو(Separator);
    إذا (Idx == npos)
      اكسر;

    // Push this split.
    إذا (Keep_خالي_جايون || Idx > 0)
      A.ڌڪو_پٺ(S.ٽڪرو(0, Idx));

    // Jump forward.
    S = S.substr(Idx + 1);
  }

  // Push the tail.
  إذا (Keep_خالي_جايون || !S.خالي_جايون())
    A.ڌڪو_پٺ(S);
}

//===----------------------------------------------------------------------===//
// Helpful Algorithms
//===----------------------------------------------------------------------===//

/// count - Return the number of non-overlapped occurrences of \arg Str in
/// the string.
size_t StringRef::ڳڻپ(StringRef Str) ثابت {
  size_t ڳڻپ_2 = 0;
  size_t Pos = 0;
  size_t N = Str.سائيز_2();
  // TODO: For an empty `Str` we return 0 for legacy reasons. Consider changing
  //       this to `Length + 1` which is more in-line with the function
  //       description.
  إذا (!N)
    أرجع 0;
  طالما ((Pos = ڳوليو(Str, Pos)) != npos) {
    ++ڳڻپ_2;
    Pos += N;
  }
  أرجع ڳڻپ_2;
}

بنانشان llvm::getAutoSenseRadix(StringRef &Str) {
  إذا (Str.خالي_جايون())
    أرجع 10;

  إذا (Str.consume_front_insensitive("0x"))
    أرجع 16;

  إذا (Str.consume_front_insensitive("0b"))
    أرجع 2;

  إذا (Str.consume_front("0o"))
    أرجع 8;

  إذا (Str[0] == '0' && Str.سائيز_2() > 1 && isDigit(Str[1])) {
    Str = Str.substr(1);
    أرجع 8;
  }

  أرجع 10;
}

منطقي llvm::consumeUnsignedInteger(StringRef &Str, بنانشان Radix,
                                  بنانشان ڊگهو ڊگهو &نتيجو) {
  // Autosense radix if not specified.
  إذا (Radix == 0)
    Radix = getAutoSenseRadix(Str);

  // Empty strings (after the radix autosense) are invalid.
  إذا (Str.خالي_جايون()) أرجع سچ;

  // Parse all the bytes of the string given this radix.  Watch for overflow.
  StringRef Str2 = Str;
  نتيجو = 0;
  طالما (!Str2.خالي_جايون()) {
    بنانشان CharVal;
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
    بنانشان ڊگهو ڊگهو Prev_نتيجو = نتيجو;
    نتيجو = نتيجو * Radix + CharVal;

    // Check for overflow by shifting back and seeing if bits were lost.
    إذا (نتيجو / Radix < Prev_نتيجو)
      أرجع سچ;

    Str2 = Str2.substr(1);
  }

  // We consider the operation a failure if no characters were consumed
  // successfully.
  إذا (Str.سائيز_2() == Str2.سائيز_2())
    أرجع سچ;

  Str = Str2;
  أرجع خطأ;
}

منطقي llvm::consumeSignedInteger(StringRef &Str, بنانشان Radix,
                                ڊگهو ڊگهو &نتيجو) {
  بنانشان ڊگهو ڊگهو ULLVal;

  // Handle positive strings first.
  إذا (!Str.starts_with("-")) {
    إذا (consumeUnsignedInteger(Str, Radix, ULLVal) ||
        // Check for value so large it overflows a signed value.
        (ڊگهو ڊگهو)ULLVal < 0)
      أرجع سچ;
    نتيجو = ULLVal;
    أرجع خطأ;
  }

  // Get the positive part of the value.
  StringRef Str2 = Str.drop_front(1);
  إذا (consumeUnsignedInteger(Str2, Radix, ULLVal) ||
      // Reject values so large they'd overflow as negative signed, but allow
      // "-0".  This negates the unsigned so that the negative isn't undefined
      // on signed overflow.
      (ڊگهو ڊگهو)-ULLVal > 0)
    أرجع سچ;

  Str = Str2;
  نتيجو = -ULLVal;
  أرجع خطأ;
}

/// GetAsUnsignedInteger - Workhorse method that converts a integer character
/// sequence of radix up to 36 to an unsigned long long value.
منطقي llvm::getAsUnsignedInteger(StringRef Str, بنانشان Radix,
                                بنانشان ڊگهو ڊگهو &نتيجو) {
  إذا (consumeUnsignedInteger(Str, Radix, نتيجو))
    أرجع سچ;

  // For getAsUnsignedInteger, we require the whole string to be consumed or
  // else we consider it a failure.
  أرجع !Str.خالي_جايون();
}

منطقي llvm::getAsSignedInteger(StringRef Str, بنانشان Radix,
                              ڊگهو ڊگهو &نتيجو) {
  إذا (consumeSignedInteger(Str, Radix, نتيجو))
    أرجع سچ;

  // For getAsSignedInteger, we require the whole string to be consumed or else
  // we consider it a failure.
  أرجع !Str.خالي_جايون();
}

منطقي StringRef::consumeInteger(بنانشان Radix, APInt &نتيجو) {
  StringRef Str = *هذا;

  // Autosense radix if not specified.
  إذا (Radix == 0)
    Radix = getAutoSenseRadix(Str);

  أكد(Radix > 1 && Radix <= 36);

  // Empty strings (after the radix autosense) are invalid.
  إذا (Str.خالي_جايون()) أرجع سچ;

  // Skip leading zeroes.  This can be a significant improvement if
  // it means we don't need > 64 bits.
  Str = Str.ltrim('0');

  // If it was nothing but zeroes....
  إذا (Str.خالي_جايون()) {
    نتيجو = APInt(64, 0);
    *هذا = Str;
    أرجع خطأ;
  }

  بنانشان Log2Radix = 0;
  طالما ((1U << Log2Radix) < Radix) Log2Radix++;
  منطقي IsPowerOf2Radix = ((1U << Log2Radix) == Radix);

  // Initialize Result to a reasonable starting width (at least 64 bits),
  // but do not shrink it if it was already larger.
  بنانشان ٻٽو_انگ_ويڪر_2 = std::max(64U, نتيجو.get_ٻٽو_انگ_ويڪر());

  إذا (نتيجو.get_ٻٽو_انگ_ويڪر() < ٻٽو_انگ_ويڪر_2) {
    نتيجو = نتيجو.zext(ٻٽو_انگ_ويڪر_2);
  }

  APInt RadixAP, CharAP; // unused unless !IsPowerOf2Radix
  إذا (!IsPowerOf2Radix) {
    // These must have the same bit-width as Result.
    RadixAP = APInt(ٻٽو_انگ_ويڪر_2, Radix);
    CharAP = APInt(ٻٽو_انگ_ويڪر_2, 0);
  }

  // Parse all the bytes of the string given this radix.
  نتيجو = 0;
  طالما (!Str.خالي_جايون()) {
    بنانشان CharVal;
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
    إذا (نتيجو.get_سرگرم_بٽ() + Log2Radix > نتيجو.get_ٻٽو_انگ_ويڪر()) {
      بنانشان New_ويڪر = نتيجو.get_ٻٽو_انگ_ويڪر() * 2;

      // Guard against overflow of the NewWidth itself
      إذا (New_ويڪر < نتيجو.get_ٻٽو_انگ_ويڪر())
        أرجع سچ;

      نتيجو = نتيجو.zext(New_ويڪر);
      إذا (!IsPowerOf2Radix) {
        RadixAP = RadixAP.zext(New_ويڪر);
        CharAP = CharAP.zext(New_ويڪر);
      }
    }

    // Add in this character.
    إذا (IsPowerOf2Radix) {
      نتيجو <<= Log2Radix;
      نتيجو |= CharVal;
    } وإلا {
      نتيجو *= RadixAP;
      CharAP = CharVal;
      نتيجو += CharAP;
    }

    Str = Str.substr(1);
  }

  // We consider the operation a failure if no characters were consumed
  // successfully.
  إذا (سائيز_2() == Str.سائيز_2())
    أرجع سچ;

  *هذا = Str;
  أرجع خطأ;
}

منطقي StringRef::getAsInteger(بنانشان Radix, APInt &نتيجو) ثابت {
  StringRef Str = *هذا;
  إذا (Str.consumeInteger(Radix, نتيجو))
    أرجع سچ;

  // For getAsInteger, we require the whole string to be consumed or else we
  // consider it a failure.
  أرجع !Str.خالي_جايون();
}

منطقي StringRef::getAsDouble(مزدوج &نتيجو, منطقي AllowInexact) ثابت {
  APFloat F(0.0);
  خودڪار حالت_Or_Err = F.convertFromString(*هذا, APFloat::rm_سڀ_کان_ويجهو_Ties_منزل_Even);
  إذا (error_منزل_Bool(حالت_Or_Err.takeError()))
    أرجع سچ;

  APFloat::op_حالت حالت = *حالت_Or_Err;
  إذا (حالت != APFloat::opOK) {
    إذا (!AllowInexact || !(حالت & APFloat::opInexact))
      أرجع سچ;
  }

  نتيجو = F.convert_منزل_Double();
  أرجع خطأ;
}

// Implementation of StringRef hashing.
hash_ڪوڊ llvm::hash_قدر(StringRef S) { أرجع hash_combine_range(S); }

بنانشان DenseMapInfo<StringRef, فراغ>::get_Hash_قدر(StringRef Val) {
  أرجع (بنانشان)(hash_قدر(Val));
}
