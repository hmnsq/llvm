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

ورتوں نطاق llvm;

// strncasecmp() is not available on non-POSIX systems, so define an
// alternative function here.
ساكن صحيح ascii_strncasecmp(StringRef LHS, StringRef RHS) {
  لكل (خودکار [LC, RC] : zip_equal(LHS, RHS)) {
    بنانشانی حرف LHC = منزل_ہیٹھلا(LC);
    بنانشانی حرف RHC = منزل_ہیٹھلا(RC);
    إذا (LHC != RHC)
      أرجع LHC < RHC ? -1 : 1;
  }
  أرجع 0;
}

صحيح StringRef::compare_insensitive(StringRef RHS) ثابت {
  size_t Min = std::min(ناپ_2(), RHS.ناپ_2());
  إذا (صحيح Res = ascii_strncasecmp(take_front(Min), RHS.take_front(Min)))
    أرجع Res;
  إذا (ناپ_2() == RHS.ناپ_2())
    أرجع 0;
  أرجع ناپ_2() < RHS.ناپ_2() ? -1 : 1;
}

منطقي StringRef::starts_with_insensitive(StringRef اگیتر) ثابت {
  أرجع ناپ_2() >= اگیتر.ناپ_2() &&
         ascii_strncasecmp(take_front(اگیتر.ناپ_2()), اگیتر) == 0;
}

منطقي StringRef::سرے_with_insensitive(StringRef Suffix) ثابت {
  أرجع ناپ_2() >= Suffix.ناپ_2() &&
         ascii_strncasecmp(take_پٹھ(Suffix.ناپ_2()), Suffix) == 0;
}

size_t StringRef::لبھو_insensitive(حرف C, size_t From) ثابت {
  حرف L = منزل_ہیٹھلا(C);
  أرجع لبھو_if([L](حرف D) { أرجع منزل_ہیٹھلا(D) == L; }, From);
}

/// compare_numeric - Compare strings, handle embedded numbers.
صحيح StringRef::compare_numeric(StringRef RHS) ثابت {
  لكل (size_t I = 0, E = std::min(ناپ_2(), RHS.ناپ_2()); I != E; ++I) {
    // Check for sequences of digits.
    إذا (isDigit(انکڑے()[I]) && isDigit(RHS.انکڑے()[I])) {
      // The longer sequence of numbers is considered larger.
      // This doesn't really handle prefixed zeros well.
      size_t J;
      لكل (J = I + 1; J != E + 1; ++J) {
        منطقي ld = J < ناپ_2() && isDigit(انکڑے()[J]);
        منطقي rd = J < RHS.ناپ_2() && isDigit(RHS.انکڑے()[J]);
        إذا (ld != rd)
          أرجع rd ? -1 : 1;
        إذا (!rd)
          اكسر;
      }
      // The two number sequences have the same length (J-I), just memcmp them.
      إذا (صحيح Res = compare_حافظہ(انکڑے() + I, RHS.انکڑے() + I, J - I))
        أرجع Res < 0 ? -1 : 1;
      // Identical number sequences, continue search after the numbers.
      I = J - 1;
      استمر;
    }
    إذا (انکڑے()[I] != RHS.انکڑے()[I])
      أرجع (بنانشانی حرف)انکڑے()[I] < (بنانشانی حرف)RHS.انکڑے()[I] ? -1 : 1;
  }
  إذا (ناپ_2() == RHS.ناپ_2())
    أرجع 0;
  أرجع ناپ_2() < RHS.ناپ_2() ? -1 : 1;
}

// Compute the edit distance between the two given strings.
بنانشانی StringRef::edit_فاصلہ(llvm::StringRef Other,
                                  منطقي AllowReplacements,
                                  بنانشانی Max_Edit_فاصلہ) ثابت {
  أرجع llvm::Compute_Edit_فاصلہ(ArrayRef(انکڑے(), ناپ_2()),
                                   ArrayRef(Other.انکڑے(), Other.ناپ_2()),
                                   AllowReplacements, Max_Edit_فاصلہ);
}

بنانشانی llvm::StringRef::edit_فاصلہ_insensitive(
    StringRef Other, منطقي AllowReplacements, بنانشانی Max_Edit_فاصلہ) ثابت {
  أرجع llvm::Compute_Mapped_Edit_فاصلہ(
      ArrayRef(انکڑے(), ناپ_2()), ArrayRef(Other.انکڑے(), Other.ناپ_2()),
      llvm::منزل_ہیٹھلا, AllowReplacements, Max_Edit_فاصلہ);
}

//===----------------------------------------------------------------------===//
// String Operations
//===----------------------------------------------------------------------===//

std::string StringRef::ہیٹھلا() ثابت {
  أرجع std::string(map_iterator(begin(), منزل_ہیٹھلا),
                     map_iterator(سرا(), منزل_ہیٹھلا));
}

std::string StringRef::اتلا() ثابت {
  أرجع std::string(map_iterator(begin(), منزل_اتلا),
                     map_iterator(سرا(), منزل_اتلا));
}

//===----------------------------------------------------------------------===//
// String Searching
//===----------------------------------------------------------------------===//


/// find - Search for the first string \arg Str in the string.
///
/// \return - The index of the first occurrence of \arg Str, or npos if not
/// found.
size_t StringRef::لبھو(StringRef Str, size_t From) ثابت {
  إذا (From > ناپ_2())
    أرجع npos;

  ثابت حرف *شروع = انکڑے() + From;
  size_t ناپ = ناپ_2() - From;

  ثابت حرف *Needle = Str.انکڑے();
  size_t N = Str.ناپ_2();
  إذا (N == 0)
    أرجع From;
  إذا (ناپ < N)
    أرجع npos;
  إذا (N == 1) {
    ثابت حرف *Ptr = (ثابت حرف *)::memchr(شروع, Needle[0], ناپ);
    أرجع Ptr == خالی_سوچک ? npos : Ptr - انکڑے();
  }

  ثابت حرف *Stop = شروع + (ناپ - N + 1);

  إذا (N == 2) {
    // Provide a fast path for newline finding (CRLF case) in InclusionRewriter.
    // Not the most optimized strategy, but getting memcmp inlined should be
    // good enough.
    افعل {
      إذا (std::memcmp(شروع, Needle, 2) == 0)
        أرجع شروع - انکڑے();
      ++شروع;
    } طالما (شروع < Stop);
    أرجع npos;
  }

  // For short haystacks or unsupported needles fall back to the naive algorithm
  إذا (ناپ < 16 || N > 255) {
    افعل {
      إذا (std::memcmp(شروع, Needle, N) == 0)
        أرجع شروع - انکڑے();
      ++شروع;
    } طالما (شروع < Stop);
    أرجع npos;
  }

  // Build the bad char heuristic table, with uint8_t to reduce cache thrashing.
  uint8_t BadCharSkip[256];
  std::memset(BadCharSkip, N, 256);
  لكل (بنانشانی i = 0; i != N-1; ++i)
    BadCharSkip[(uint8_t)Str[i]] = N-1-i;

  افعل {
    uint8_t آخری = شروع[N - 1];
    إذا (LLVM_UNLIKELY(آخری == (uint8_t)Needle[N - 1]))
      إذا (std::memcmp(شروع, Needle, N - 1) == 0)
        أرجع شروع - انکڑے();

    // Otherwise skip the appropriate number of bytes.
    شروع += BadCharSkip[آخری];
  } طالما (شروع < Stop);

  أرجع npos;
}

size_t StringRef::لبھو_insensitive(StringRef Str, size_t From) ثابت {
  StringRef This = substr(From);
  طالما (This.ناپ_2() >= Str.ناپ_2()) {
    إذا (This.starts_with_insensitive(Str))
      أرجع From;
    This = This.drop_front();
    ++From;
  }
  أرجع npos;
}

size_t StringRef::rfind_insensitive(حرف C, size_t From) ثابت {
  From = std::min(From, ناپ_2());
  size_t i = From;
  طالما (i != 0) {
    --i;
    إذا (منزل_ہیٹھلا(انکڑے()[i]) == منزل_ہیٹھلا(C))
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
  size_t N = Str.ناپ_2();
  إذا (N > ناپ_2())
    أرجع npos;
  لكل (size_t i = ناپ_2() - N + 1, e = 0; i != e;) {
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
StringRef::ناپ_type StringRef::لبھو_پہلا_دا(StringRef Chars,
                                              size_t From) ثابت {
  std::bitset<1 << CHAR_BIT> Char_بٹ;
  لكل (حرف C : Chars)
    Char_بٹ.سیٹ((بنانشانی حرف)C);

  لكل (ناپ_type i = std::min(From, ناپ_2()), e = ناپ_2(); i != e; ++i)
    إذا (Char_بٹ.test((بنانشانی حرف)انکڑے()[i]))
      أرجع i;
  أرجع npos;
}

/// find_first_not_of - Find the first character in the string that is not
/// \arg C or npos if not found.
StringRef::ناپ_type StringRef::لبھو_پہلا_not_دا(حرف C, size_t From) ثابت {
  أرجع std::string_view(*هذا).لبھو_پہلا_not_دا(C, From);
}

/// find_first_not_of - Find the first character in the string that is not
/// in the string \arg Chars, or npos if not found.
///
/// Note: O(size() + Chars.size())
StringRef::ناپ_type StringRef::لبھو_پہلا_not_دا(StringRef Chars,
                                                  size_t From) ثابت {
  std::bitset<1 << CHAR_BIT> Char_بٹ;
  لكل (حرف C : Chars)
    Char_بٹ.سیٹ((بنانشانی حرف)C);

  لكل (ناپ_type i = std::min(From, ناپ_2()), e = ناپ_2(); i != e; ++i)
    إذا (!Char_بٹ.test((بنانشانی حرف)انکڑے()[i]))
      أرجع i;
  أرجع npos;
}

/// find_last_of - Find the last character in the string that is in \arg C,
/// or npos if not found.
///
/// Note: O(size() + Chars.size())
StringRef::ناپ_type StringRef::لبھو_آخری_دا(StringRef Chars,
                                             size_t From) ثابت {
  std::bitset<1 << CHAR_BIT> Char_بٹ;
  لكل (حرف C : Chars)
    Char_بٹ.سیٹ((بنانشانی حرف)C);

  لكل (ناپ_type i = std::min(From, ناپ_2()) - 1, e = -1; i != e; --i)
    إذا (Char_بٹ.test((بنانشانی حرف)انکڑے()[i]))
      أرجع i;
  أرجع npos;
}

/// find_last_not_of - Find the last character in the string that is not
/// \arg C, or npos if not found.
StringRef::ناپ_type StringRef::لبھو_آخری_not_دا(حرف C, size_t From) ثابت {
  لكل (ناپ_type i = std::min(From, ناپ_2()) - 1, e = -1; i != e; --i)
    إذا (انکڑے()[i] != C)
      أرجع i;
  أرجع npos;
}

/// find_last_not_of - Find the last character in the string that is not in
/// \arg Chars, or npos if not found.
///
/// Note: O(size() + Chars.size())
StringRef::ناپ_type StringRef::لبھو_آخری_not_دا(StringRef Chars,
                                                 size_t From) ثابت {
  std::bitset<1 << CHAR_BIT> Char_بٹ;
  لكل (حرف C : Chars)
    Char_بٹ.سیٹ((بنانشانی حرف)C);

  لكل (ناپ_type i = std::min(From, ناپ_2()) - 1, e = -1; i != e; --i)
    إذا (!Char_بٹ.test((بنانشانی حرف)انکڑے()[i]))
      أرجع i;
  أرجع npos;
}

فراغ StringRef::ونڈو(Small_سمتیہ_Impl<StringRef> &A,
                      StringRef Separator, صحيح Max_ونڈو,
                      منطقي Keep_خالی_تھاواں) ثابت {
  StringRef S = *هذا;

  // Count down from MaxSplit. When MaxSplit is -1, this will just split
  // "forever". This doesn't support splitting more than 2^31 times
  // intentionally; if we ever want that we can make MaxSplit a 64-bit integer
  // but that seems unlikely to be useful.
  طالما (Max_ونڈو-- != 0) {
    size_t Idx = S.لبھو(Separator);
    إذا (Idx == npos)
      اكسر;

    // Push this split.
    إذا (Keep_خالی_تھاواں || Idx > 0)
      A.ڈالو_پٹھ(S.ٹکڑا(0, Idx));

    // Jump forward.
    S = S.substr(Idx + Separator.ناپ_2());
  }

  // Push the tail.
  إذا (Keep_خالی_تھاواں || !S.خالی_تھاواں())
    A.ڈالو_پٹھ(S);
}

فراغ StringRef::ونڈو(Small_سمتیہ_Impl<StringRef> &A, حرف Separator,
                      صحيح Max_ونڈو, منطقي Keep_خالی_تھاواں) ثابت {
  StringRef S = *هذا;

  // Count down from MaxSplit. When MaxSplit is -1, this will just split
  // "forever". This doesn't support splitting more than 2^31 times
  // intentionally; if we ever want that we can make MaxSplit a 64-bit integer
  // but that seems unlikely to be useful.
  طالما (Max_ونڈو-- != 0) {
    size_t Idx = S.لبھو(Separator);
    إذا (Idx == npos)
      اكسر;

    // Push this split.
    إذا (Keep_خالی_تھاواں || Idx > 0)
      A.ڈالو_پٹھ(S.ٹکڑا(0, Idx));

    // Jump forward.
    S = S.substr(Idx + 1);
  }

  // Push the tail.
  إذا (Keep_خالی_تھاواں || !S.خالی_تھاواں())
    A.ڈالو_پٹھ(S);
}

//===----------------------------------------------------------------------===//
// Helpful Algorithms
//===----------------------------------------------------------------------===//

/// count - Return the number of non-overlapped occurrences of \arg Str in
/// the string.
size_t StringRef::گنتی(StringRef Str) ثابت {
  size_t گنتی_2 = 0;
  size_t Pos = 0;
  size_t N = Str.ناپ_2();
  // TODO: For an empty `Str` we return 0 for legacy reasons. Consider changing
  //       this to `Length + 1` which is more in-line with the function
  //       description.
  إذا (!N)
    أرجع 0;
  طالما ((Pos = لبھو(Str, Pos)) != npos) {
    ++گنتی_2;
    Pos += N;
  }
  أرجع گنتی_2;
}

بنانشانی llvm::getAutoSenseRadix(StringRef &Str) {
  إذا (Str.خالی_تھاواں())
    أرجع 10;

  إذا (Str.consume_front_insensitive("0x"))
    أرجع 16;

  إذا (Str.consume_front_insensitive("0b"))
    أرجع 2;

  إذا (Str.consume_front("0o"))
    أرجع 8;

  إذا (Str[0] == '0' && Str.ناپ_2() > 1 && isDigit(Str[1])) {
    Str = Str.substr(1);
    أرجع 8;
  }

  أرجع 10;
}

منطقي llvm::consumeUnsignedInteger(StringRef &Str, بنانشانی Radix,
                                  بنانشانی لما لما &نتیجہ) {
  // Autosense radix if not specified.
  إذا (Radix == 0)
    Radix = getAutoSenseRadix(Str);

  // Empty strings (after the radix autosense) are invalid.
  إذا (Str.خالی_تھاواں()) أرجع سچ;

  // Parse all the bytes of the string given this radix.  Watch for overflow.
  StringRef Str2 = Str;
  نتیجہ = 0;
  طالما (!Str2.خالی_تھاواں()) {
    بنانشانی CharVal;
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
    بنانشانی لما لما Prev_نتیجہ = نتیجہ;
    نتیجہ = نتیجہ * Radix + CharVal;

    // Check for overflow by shifting back and seeing if bits were lost.
    إذا (نتیجہ / Radix < Prev_نتیجہ)
      أرجع سچ;

    Str2 = Str2.substr(1);
  }

  // We consider the operation a failure if no characters were consumed
  // successfully.
  إذا (Str.ناپ_2() == Str2.ناپ_2())
    أرجع سچ;

  Str = Str2;
  أرجع خطأ;
}

منطقي llvm::consumeSignedInteger(StringRef &Str, بنانشانی Radix,
                                لما لما &نتیجہ) {
  بنانشانی لما لما ULLVal;

  // Handle positive strings first.
  إذا (!Str.starts_with("-")) {
    إذا (consumeUnsignedInteger(Str, Radix, ULLVal) ||
        // Check for value so large it overflows a signed value.
        (لما لما)ULLVal < 0)
      أرجع سچ;
    نتیجہ = ULLVal;
    أرجع خطأ;
  }

  // Get the positive part of the value.
  StringRef Str2 = Str.drop_front(1);
  إذا (consumeUnsignedInteger(Str2, Radix, ULLVal) ||
      // Reject values so large they'd overflow as negative signed, but allow
      // "-0".  This negates the unsigned so that the negative isn't undefined
      // on signed overflow.
      (لما لما)-ULLVal > 0)
    أرجع سچ;

  Str = Str2;
  نتیجہ = -ULLVal;
  أرجع خطأ;
}

/// GetAsUnsignedInteger - Workhorse method that converts a integer character
/// sequence of radix up to 36 to an unsigned long long value.
منطقي llvm::getAsUnsignedInteger(StringRef Str, بنانشانی Radix,
                                بنانشانی لما لما &نتیجہ) {
  إذا (consumeUnsignedInteger(Str, Radix, نتیجہ))
    أرجع سچ;

  // For getAsUnsignedInteger, we require the whole string to be consumed or
  // else we consider it a failure.
  أرجع !Str.خالی_تھاواں();
}

منطقي llvm::getAsSignedInteger(StringRef Str, بنانشانی Radix,
                              لما لما &نتیجہ) {
  إذا (consumeSignedInteger(Str, Radix, نتیجہ))
    أرجع سچ;

  // For getAsSignedInteger, we require the whole string to be consumed or else
  // we consider it a failure.
  أرجع !Str.خالی_تھاواں();
}

منطقي StringRef::consumeInteger(بنانشانی Radix, APInt &نتیجہ) {
  StringRef Str = *هذا;

  // Autosense radix if not specified.
  إذا (Radix == 0)
    Radix = getAutoSenseRadix(Str);

  أكد(Radix > 1 && Radix <= 36);

  // Empty strings (after the radix autosense) are invalid.
  إذا (Str.خالی_تھاواں()) أرجع سچ;

  // Skip leading zeroes.  This can be a significant improvement if
  // it means we don't need > 64 bits.
  Str = Str.ltrim('0');

  // If it was nothing but zeroes....
  إذا (Str.خالی_تھاواں()) {
    نتیجہ = APInt(64, 0);
    *هذا = Str;
    أرجع خطأ;
  }

  بنانشانی Log2Radix = 0;
  طالما ((1U << Log2Radix) < Radix) Log2Radix++;
  منطقي IsPowerOf2Radix = ((1U << Log2Radix) == Radix);

  // Initialize Result to a reasonable starting width (at least 64 bits),
  // but do not shrink it if it was already larger.
  بنانشانی ثنائی_ہندسہ_چوڑائی_2 = std::max(64U, نتیجہ.get_ثنائی_ہندسہ_چوڑائی());

  إذا (نتیجہ.get_ثنائی_ہندسہ_چوڑائی() < ثنائی_ہندسہ_چوڑائی_2) {
    نتیجہ = نتیجہ.zext(ثنائی_ہندسہ_چوڑائی_2);
  }

  APInt RadixAP, CharAP; // unused unless !IsPowerOf2Radix
  إذا (!IsPowerOf2Radix) {
    // These must have the same bit-width as Result.
    RadixAP = APInt(ثنائی_ہندسہ_چوڑائی_2, Radix);
    CharAP = APInt(ثنائی_ہندسہ_چوڑائی_2, 0);
  }

  // Parse all the bytes of the string given this radix.
  نتیجہ = 0;
  طالما (!Str.خالی_تھاواں()) {
    بنانشانی CharVal;
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
    إذا (نتیجہ.get_سرگرم_بٹ() + Log2Radix > نتیجہ.get_ثنائی_ہندسہ_چوڑائی()) {
      بنانشانی New_چوڑائی = نتیجہ.get_ثنائی_ہندسہ_چوڑائی() * 2;

      // Guard against overflow of the NewWidth itself
      إذا (New_چوڑائی < نتیجہ.get_ثنائی_ہندسہ_چوڑائی())
        أرجع سچ;

      نتیجہ = نتیجہ.zext(New_چوڑائی);
      إذا (!IsPowerOf2Radix) {
        RadixAP = RadixAP.zext(New_چوڑائی);
        CharAP = CharAP.zext(New_چوڑائی);
      }
    }

    // Add in this character.
    إذا (IsPowerOf2Radix) {
      نتیجہ <<= Log2Radix;
      نتیجہ |= CharVal;
    } وإلا {
      نتیجہ *= RadixAP;
      CharAP = CharVal;
      نتیجہ += CharAP;
    }

    Str = Str.substr(1);
  }

  // We consider the operation a failure if no characters were consumed
  // successfully.
  إذا (ناپ_2() == Str.ناپ_2())
    أرجع سچ;

  *هذا = Str;
  أرجع خطأ;
}

منطقي StringRef::getAsInteger(بنانشانی Radix, APInt &نتیجہ) ثابت {
  StringRef Str = *هذا;
  إذا (Str.consumeInteger(Radix, نتیجہ))
    أرجع سچ;

  // For getAsInteger, we require the whole string to be consumed or else we
  // consider it a failure.
  أرجع !Str.خالی_تھاواں();
}

منطقي StringRef::getAsDouble(مزدوج &نتیجہ, منطقي AllowInexact) ثابت {
  APFloat F(0.0);
  خودکار حالت_Or_Err = F.convertFromString(*هذا, APFloat::rm_سبھ_توں_نیڑے_Ties_منزل_Even);
  إذا (error_منزل_Bool(حالت_Or_Err.takeError()))
    أرجع سچ;

  APFloat::op_حالت حالت = *حالت_Or_Err;
  إذا (حالت != APFloat::opOK) {
    إذا (!AllowInexact || !(حالت & APFloat::opInexact))
      أرجع سچ;
  }

  نتیجہ = F.convert_منزل_Double();
  أرجع خطأ;
}

// Implementation of StringRef hashing.
hash_کوڈ llvm::hash_قدر(StringRef S) { أرجع hash_combine_range(S); }

بنانشانی DenseMapInfo<StringRef, فراغ>::get_Hash_قدر(StringRef Val) {
  أرجع (بنانشانی)(hash_قدر(Val));
}
