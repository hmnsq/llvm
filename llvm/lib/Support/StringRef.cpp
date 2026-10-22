//===-- StringRef.cpp - Lightweight String References ---------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#شامل "llvm/ADT/StringRef.h"
#شامل "llvm/ADT/APFloat.h"
#شامل "llvm/ADT/APInt.h"
#شامل "llvm/ADT/Hashing.h"
#شامل "llvm/ADT/StringExtras.h"
#شامل "llvm/ADT/edit_distance.h"
#شامل "llvm/Support/Error.h"
#شامل <bitset>

استعمال نامخلا llvm;

// strncasecmp() is not available on non-POSIX systems, so define an
// alternative function here.
ساکن صحیح ascii_strncasecmp(StringRef LHS, StringRef RHS) {
  برائے (خودکار [LC, RC] : zip_equal(LHS, RHS)) {
    غیرعلامتی حرف LHC = مقصد_زیریں(LC);
    غیرعلامتی حرف RHC = مقصد_زیریں(RC);
    اگر (LHC != RHC)
      واپس LHC < RHC ? -1 : 1;
  }
  واپس 0;
}

صحیح StringRef::compare_insensitive(StringRef RHS) ثابت {
  size_t Min = std::min(سائز_2(), RHS.سائز_2());
  اگر (صحیح Res = ascii_strncasecmp(take_front(Min), RHS.take_front(Min)))
    واپس Res;
  اگر (سائز_2() == RHS.سائز_2())
    واپس 0;
  واپس سائز_2() < RHS.سائز_2() ? -1 : 1;
}

منطقی StringRef::starts_with_insensitive(StringRef سابقہ) ثابت {
  واپس سائز_2() >= سابقہ.سائز_2() &&
         ascii_strncasecmp(take_front(سابقہ.سائز_2()), سابقہ) == 0;
}

منطقی StringRef::سرے_with_insensitive(StringRef Suffix) ثابت {
  واپس سائز_2() >= Suffix.سائز_2() &&
         ascii_strncasecmp(take_عقب(Suffix.سائز_2()), Suffix) == 0;
}

size_t StringRef::ڈھونڈو_insensitive(حرف C, size_t From) ثابت {
  حرف L = مقصد_زیریں(C);
  واپس ڈھونڈو_if([L](حرف D) { واپس مقصد_زیریں(D) == L; }, From);
}

/// compare_numeric - Compare strings, handle embedded numbers.
صحیح StringRef::compare_numeric(StringRef RHS) ثابت {
  برائے (size_t I = 0, E = std::min(سائز_2(), RHS.سائز_2()); I != E; ++I) {
    // Check for sequences of digits.
    اگر (isDigit(ڈیٹا()[I]) && isDigit(RHS.ڈیٹا()[I])) {
      // The longer sequence of numbers is considered larger.
      // This doesn't really handle prefixed zeros well.
      size_t J;
      برائے (J = I + 1; J != E + 1; ++J) {
        منطقی ld = J < سائز_2() && isDigit(ڈیٹا()[J]);
        منطقی rd = J < RHS.سائز_2() && isDigit(RHS.ڈیٹا()[J]);
        اگر (ld != rd)
          واپس rd ? -1 : 1;
        اگر (!rd)
          توڑو;
      }
      // The two number sequences have the same length (J-I), just memcmp them.
      اگر (صحیح Res = compare_حافظہ(ڈیٹا() + I, RHS.ڈیٹا() + I, J - I))
        واپس Res < 0 ? -1 : 1;
      // Identical number sequences, continue search after the numbers.
      I = J - 1;
      جاری;
    }
    اگر (ڈیٹا()[I] != RHS.ڈیٹا()[I])
      واپس (غیرعلامتی حرف)ڈیٹا()[I] < (غیرعلامتی حرف)RHS.ڈیٹا()[I] ? -1 : 1;
  }
  اگر (سائز_2() == RHS.سائز_2())
    واپس 0;
  واپس سائز_2() < RHS.سائز_2() ? -1 : 1;
}

// Compute the edit distance between the two given strings.
غیرعلامتی StringRef::edit_فاصلہ(llvm::StringRef Other,
                                  منطقی AllowReplacements,
                                  غیرعلامتی Max_Edit_فاصلہ) ثابت {
  واپس llvm::Compute_Edit_فاصلہ(ArrayRef(ڈیٹا(), سائز_2()),
                                   ArrayRef(Other.ڈیٹا(), Other.سائز_2()),
                                   AllowReplacements, Max_Edit_فاصلہ);
}

غیرعلامتی llvm::StringRef::edit_فاصلہ_insensitive(
    StringRef Other, منطقی AllowReplacements, غیرعلامتی Max_Edit_فاصلہ) ثابت {
  واپس llvm::Compute_Mapped_Edit_فاصلہ(
      ArrayRef(ڈیٹا(), سائز_2()), ArrayRef(Other.ڈیٹا(), Other.سائز_2()),
      llvm::مقصد_زیریں, AllowReplacements, Max_Edit_فاصلہ);
}

//===----------------------------------------------------------------------===//
// String Operations
//===----------------------------------------------------------------------===//

std::string StringRef::زیریں() ثابت {
  واپس std::string(map_iterator(begin(), مقصد_زیریں),
                     map_iterator(انتہا(), مقصد_زیریں));
}

std::string StringRef::بالائی() ثابت {
  واپس std::string(map_iterator(begin(), مقصد_بالائی),
                     map_iterator(انتہا(), مقصد_بالائی));
}

//===----------------------------------------------------------------------===//
// String Searching
//===----------------------------------------------------------------------===//


/// find - Search for the first string \arg Str in the string.
///
/// \return - The index of the first occurrence of \arg Str, or npos if not
/// found.
size_t StringRef::ڈھونڈو(StringRef Str, size_t From) ثابت {
  اگر (From > سائز_2())
    واپس npos;

  ثابت حرف *آغاز = ڈیٹا() + From;
  size_t سائز = سائز_2() - From;

  ثابت حرف *Needle = Str.ڈیٹا();
  size_t N = Str.سائز_2();
  اگر (N == 0)
    واپس From;
  اگر (سائز < N)
    واپس npos;
  اگر (N == 1) {
    ثابت حرف *Ptr = (ثابت حرف *)::memchr(آغاز, Needle[0], سائز);
    واپس Ptr == خالی_اشاریہ ? npos : Ptr - ڈیٹا();
  }

  ثابت حرف *Stop = آغاز + (سائز - N + 1);

  اگر (N == 2) {
    // Provide a fast path for newline finding (CRLF case) in InclusionRewriter.
    // Not the most optimized strategy, but getting memcmp inlined should be
    // good enough.
    کرو {
      اگر (std::memcmp(آغاز, Needle, 2) == 0)
        واپس آغاز - ڈیٹا();
      ++آغاز;
    } جب (آغاز < Stop);
    واپس npos;
  }

  // For short haystacks or unsupported needles fall back to the naive algorithm
  اگر (سائز < 16 || N > 255) {
    کرو {
      اگر (std::memcmp(آغاز, Needle, N) == 0)
        واپس آغاز - ڈیٹا();
      ++آغاز;
    } جب (آغاز < Stop);
    واپس npos;
  }

  // Build the bad char heuristic table, with uint8_t to reduce cache thrashing.
  uint8_t BadCharSkip[256];
  std::memset(BadCharSkip, N, 256);
  برائے (غیرعلامتی i = 0; i != N-1; ++i)
    BadCharSkip[(uint8_t)Str[i]] = N-1-i;

  کرو {
    uint8_t آخری = آغاز[N - 1];
    اگر (LLVM_UNLIKELY(آخری == (uint8_t)Needle[N - 1]))
      اگر (std::memcmp(آغاز, Needle, N - 1) == 0)
        واپس آغاز - ڈیٹا();

    // Otherwise skip the appropriate number of bytes.
    آغاز += BadCharSkip[آخری];
  } جب (آغاز < Stop);

  واپس npos;
}

size_t StringRef::ڈھونڈو_insensitive(StringRef Str, size_t From) ثابت {
  StringRef This = substr(From);
  جب (This.سائز_2() >= Str.سائز_2()) {
    اگر (This.starts_with_insensitive(Str))
      واپس From;
    This = This.drop_front();
    ++From;
  }
  واپس npos;
}

size_t StringRef::rfind_insensitive(حرف C, size_t From) ثابت {
  From = std::min(From, سائز_2());
  size_t i = From;
  جب (i != 0) {
    --i;
    اگر (مقصد_زیریں(ڈیٹا()[i]) == مقصد_زیریں(C))
      واپس i;
  }
  واپس npos;
}

/// rfind - Search for the last string \arg Str in the string.
///
/// \return - The index of the last occurrence of \arg Str, or npos if not
/// found.
size_t StringRef::rfind(StringRef Str) ثابت {
  واپس std::string_view(*یہ).rfind(Str);
}

size_t StringRef::rfind_insensitive(StringRef Str) ثابت {
  size_t N = Str.سائز_2();
  اگر (N > سائز_2())
    واپس npos;
  برائے (size_t i = سائز_2() - N + 1, e = 0; i != e;) {
    --i;
    اگر (substr(i, N).equals_insensitive(Str))
      واپس i;
  }
  واپس npos;
}

/// find_first_of - Find the first character in the string that is in \arg
/// Chars, or npos if not found.
///
/// Note: O(size() + Chars.size())
StringRef::سائز_type StringRef::ڈھونڈو_پہلا_کا(StringRef Chars,
                                              size_t From) ثابت {
  std::bitset<1 << CHAR_BIT> Char_بٹ;
  برائے (حرف C : Chars)
    Char_بٹ.سیٹ((غیرعلامتی حرف)C);

  برائے (سائز_type i = std::min(From, سائز_2()), e = سائز_2(); i != e; ++i)
    اگر (Char_بٹ.test((غیرعلامتی حرف)ڈیٹا()[i]))
      واپس i;
  واپس npos;
}

/// find_first_not_of - Find the first character in the string that is not
/// \arg C or npos if not found.
StringRef::سائز_type StringRef::ڈھونڈو_پہلا_not_کا(حرف C, size_t From) ثابت {
  واپس std::string_view(*یہ).ڈھونڈو_پہلا_not_کا(C, From);
}

/// find_first_not_of - Find the first character in the string that is not
/// in the string \arg Chars, or npos if not found.
///
/// Note: O(size() + Chars.size())
StringRef::سائز_type StringRef::ڈھونڈو_پہلا_not_کا(StringRef Chars,
                                                  size_t From) ثابت {
  std::bitset<1 << CHAR_BIT> Char_بٹ;
  برائے (حرف C : Chars)
    Char_بٹ.سیٹ((غیرعلامتی حرف)C);

  برائے (سائز_type i = std::min(From, سائز_2()), e = سائز_2(); i != e; ++i)
    اگر (!Char_بٹ.test((غیرعلامتی حرف)ڈیٹا()[i]))
      واپس i;
  واپس npos;
}

/// find_last_of - Find the last character in the string that is in \arg C,
/// or npos if not found.
///
/// Note: O(size() + Chars.size())
StringRef::سائز_type StringRef::ڈھونڈو_آخری_کا(StringRef Chars,
                                             size_t From) ثابت {
  std::bitset<1 << CHAR_BIT> Char_بٹ;
  برائے (حرف C : Chars)
    Char_بٹ.سیٹ((غیرعلامتی حرف)C);

  برائے (سائز_type i = std::min(From, سائز_2()) - 1, e = -1; i != e; --i)
    اگر (Char_بٹ.test((غیرعلامتی حرف)ڈیٹا()[i]))
      واپس i;
  واپس npos;
}

/// find_last_not_of - Find the last character in the string that is not
/// \arg C, or npos if not found.
StringRef::سائز_type StringRef::ڈھونڈو_آخری_not_کا(حرف C, size_t From) ثابت {
  برائے (سائز_type i = std::min(From, سائز_2()) - 1, e = -1; i != e; --i)
    اگر (ڈیٹا()[i] != C)
      واپس i;
  واپس npos;
}

/// find_last_not_of - Find the last character in the string that is not in
/// \arg Chars, or npos if not found.
///
/// Note: O(size() + Chars.size())
StringRef::سائز_type StringRef::ڈھونڈو_آخری_not_کا(StringRef Chars,
                                                 size_t From) ثابت {
  std::bitset<1 << CHAR_BIT> Char_بٹ;
  برائے (حرف C : Chars)
    Char_بٹ.سیٹ((غیرعلامتی حرف)C);

  برائے (سائز_type i = std::min(From, سائز_2()) - 1, e = -1; i != e; --i)
    اگر (!Char_بٹ.test((غیرعلامتی حرف)ڈیٹا()[i]))
      واپس i;
  واپس npos;
}

خالی StringRef::بانٹو(Small_سمتیہ_Impl<StringRef> &A,
                      StringRef Separator, صحیح Max_بانٹو,
                      منطقی Keep_خالی_خانے) ثابت {
  StringRef S = *یہ;

  // Count down from MaxSplit. When MaxSplit is -1, this will just split
  // "forever". This doesn't support splitting more than 2^31 times
  // intentionally; if we ever want that we can make MaxSplit a 64-bit integer
  // but that seems unlikely to be useful.
  جب (Max_بانٹو-- != 0) {
    size_t Idx = S.ڈھونڈو(Separator);
    اگر (Idx == npos)
      توڑو;

    // Push this split.
    اگر (Keep_خالی_خانے || Idx > 0)
      A.ڈالو_عقب(S.قاش(0, Idx));

    // Jump forward.
    S = S.substr(Idx + Separator.سائز_2());
  }

  // Push the tail.
  اگر (Keep_خالی_خانے || !S.خالی_خانے())
    A.ڈالو_عقب(S);
}

خالی StringRef::بانٹو(Small_سمتیہ_Impl<StringRef> &A, حرف Separator,
                      صحیح Max_بانٹو, منطقی Keep_خالی_خانے) ثابت {
  StringRef S = *یہ;

  // Count down from MaxSplit. When MaxSplit is -1, this will just split
  // "forever". This doesn't support splitting more than 2^31 times
  // intentionally; if we ever want that we can make MaxSplit a 64-bit integer
  // but that seems unlikely to be useful.
  جب (Max_بانٹو-- != 0) {
    size_t Idx = S.ڈھونڈو(Separator);
    اگر (Idx == npos)
      توڑو;

    // Push this split.
    اگر (Keep_خالی_خانے || Idx > 0)
      A.ڈالو_عقب(S.قاش(0, Idx));

    // Jump forward.
    S = S.substr(Idx + 1);
  }

  // Push the tail.
  اگر (Keep_خالی_خانے || !S.خالی_خانے())
    A.ڈالو_عقب(S);
}

//===----------------------------------------------------------------------===//
// Helpful Algorithms
//===----------------------------------------------------------------------===//

/// count - Return the number of non-overlapped occurrences of \arg Str in
/// the string.
size_t StringRef::گنتی(StringRef Str) ثابت {
  size_t گنتی_2 = 0;
  size_t Pos = 0;
  size_t N = Str.سائز_2();
  // TODO: For an empty `Str` we return 0 for legacy reasons. Consider changing
  //       this to `Length + 1` which is more in-line with the function
  //       description.
  اگر (!N)
    واپس 0;
  جب ((Pos = ڈھونڈو(Str, Pos)) != npos) {
    ++گنتی_2;
    Pos += N;
  }
  واپس گنتی_2;
}

غیرعلامتی llvm::getAutoSenseRadix(StringRef &Str) {
  اگر (Str.خالی_خانے())
    واپس 10;

  اگر (Str.consume_front_insensitive("0x"))
    واپس 16;

  اگر (Str.consume_front_insensitive("0b"))
    واپس 2;

  اگر (Str.consume_front("0o"))
    واپس 8;

  اگر (Str[0] == '0' && Str.سائز_2() > 1 && isDigit(Str[1])) {
    Str = Str.substr(1);
    واپس 8;
  }

  واپس 10;
}

منطقی llvm::consumeUnsignedInteger(StringRef &Str, غیرعلامتی Radix,
                                  غیرعلامتی طویل طویل &نتیجہ) {
  // Autosense radix if not specified.
  اگر (Radix == 0)
    Radix = getAutoSenseRadix(Str);

  // Empty strings (after the radix autosense) are invalid.
  اگر (Str.خالی_خانے()) واپس سچ;

  // Parse all the bytes of the string given this radix.  Watch for overflow.
  StringRef Str2 = Str;
  نتیجہ = 0;
  جب (!Str2.خالی_خانے()) {
    غیرعلامتی CharVal;
    اگر (Str2[0] >= '0' && Str2[0] <= '9')
      CharVal = Str2[0] - '0';
    ورنہ اگر (Str2[0] >= 'a' && Str2[0] <= 'z')
      CharVal = Str2[0] - 'a' + 10;
    ورنہ اگر (Str2[0] >= 'A' && Str2[0] <= 'Z')
      CharVal = Str2[0] - 'A' + 10;
    ورنہ
      توڑو;

    // If the parsed value is larger than the integer radix, we cannot
    // consume any more characters.
    اگر (CharVal >= Radix)
      توڑو;

    // Add in this character.
    غیرعلامتی طویل طویل پچھلا_نتیجہ = نتیجہ;
    نتیجہ = نتیجہ * Radix + CharVal;

    // Check for overflow by shifting back and seeing if bits were lost.
    اگر (نتیجہ / Radix < پچھلا_نتیجہ)
      واپس سچ;

    Str2 = Str2.substr(1);
  }

  // We consider the operation a failure if no characters were consumed
  // successfully.
  اگر (Str.سائز_2() == Str2.سائز_2())
    واپس سچ;

  Str = Str2;
  واپس جھوٹ;
}

منطقی llvm::consumeSignedInteger(StringRef &Str, غیرعلامتی Radix,
                                طویل طویل &نتیجہ) {
  غیرعلامتی طویل طویل ULLVal;

  // Handle positive strings first.
  اگر (!Str.starts_with("-")) {
    اگر (consumeUnsignedInteger(Str, Radix, ULLVal) ||
        // Check for value so large it overflows a signed value.
        (طویل طویل)ULLVal < 0)
      واپس سچ;
    نتیجہ = ULLVal;
    واپس جھوٹ;
  }

  // Get the positive part of the value.
  StringRef Str2 = Str.drop_front(1);
  اگر (consumeUnsignedInteger(Str2, Radix, ULLVal) ||
      // Reject values so large they'd overflow as negative signed, but allow
      // "-0".  This negates the unsigned so that the negative isn't undefined
      // on signed overflow.
      (طویل طویل)-ULLVal > 0)
    واپس سچ;

  Str = Str2;
  نتیجہ = -ULLVal;
  واپس جھوٹ;
}

/// GetAsUnsignedInteger - Workhorse method that converts a integer character
/// sequence of radix up to 36 to an unsigned long long value.
منطقی llvm::getAsUnsignedInteger(StringRef Str, غیرعلامتی Radix,
                                غیرعلامتی طویل طویل &نتیجہ) {
  اگر (consumeUnsignedInteger(Str, Radix, نتیجہ))
    واپس سچ;

  // For getAsUnsignedInteger, we require the whole string to be consumed or
  // else we consider it a failure.
  واپس !Str.خالی_خانے();
}

منطقی llvm::getAsSignedInteger(StringRef Str, غیرعلامتی Radix,
                              طویل طویل &نتیجہ) {
  اگر (consumeSignedInteger(Str, Radix, نتیجہ))
    واپس سچ;

  // For getAsSignedInteger, we require the whole string to be consumed or else
  // we consider it a failure.
  واپس !Str.خالی_خانے();
}

منطقی StringRef::consumeInteger(غیرعلامتی Radix, APInt &نتیجہ) {
  StringRef Str = *یہ;

  // Autosense radix if not specified.
  اگر (Radix == 0)
    Radix = getAutoSenseRadix(Str);

  أكد(Radix > 1 && Radix <= 36);

  // Empty strings (after the radix autosense) are invalid.
  اگر (Str.خالی_خانے()) واپس سچ;

  // Skip leading zeroes.  This can be a significant improvement if
  // it means we don't need > 64 bits.
  Str = Str.ltrim('0');

  // If it was nothing but zeroes....
  اگر (Str.خالی_خانے()) {
    نتیجہ = APInt(64, 0);
    *یہ = Str;
    واپس جھوٹ;
  }

  غیرعلامتی Log2Radix = 0;
  جب ((1U << Log2Radix) < Radix) Log2Radix++;
  منطقی IsPowerOf2Radix = ((1U << Log2Radix) == Radix);

  // Initialize Result to a reasonable starting width (at least 64 bits),
  // but do not shrink it if it was already larger.
  غیرعلامتی ثنائی_ہندسہ_چوڑائی_2 = std::max(64U, نتیجہ.get_ثنائی_ہندسہ_چوڑائی());

  اگر (نتیجہ.get_ثنائی_ہندسہ_چوڑائی() < ثنائی_ہندسہ_چوڑائی_2) {
    نتیجہ = نتیجہ.zext(ثنائی_ہندسہ_چوڑائی_2);
  }

  APInt RadixAP, CharAP; // unused unless !IsPowerOf2Radix
  اگر (!IsPowerOf2Radix) {
    // These must have the same bit-width as Result.
    RadixAP = APInt(ثنائی_ہندسہ_چوڑائی_2, Radix);
    CharAP = APInt(ثنائی_ہندسہ_چوڑائی_2, 0);
  }

  // Parse all the bytes of the string given this radix.
  نتیجہ = 0;
  جب (!Str.خالی_خانے()) {
    غیرعلامتی CharVal;
    اگر (Str[0] >= '0' && Str[0] <= '9')
      CharVal = Str[0]-'0';
    ورنہ اگر (Str[0] >= 'a' && Str[0] <= 'z')
      CharVal = Str[0]-'a'+10;
    ورنہ اگر (Str[0] >= 'A' && Str[0] <= 'Z')
      CharVal = Str[0]-'A'+10;
    ورنہ
      توڑو;

    // If the parsed value is larger than the integer radix, the string is
    // invalid.
    اگر (CharVal >= Radix)
      توڑو;

    // Check if one more digit will overflow, and grow if so.
    اگر (نتیجہ.get_فعال_بٹ() + Log2Radix > نتیجہ.get_ثنائی_ہندسہ_چوڑائی()) {
      غیرعلامتی New_چوڑائی = نتیجہ.get_ثنائی_ہندسہ_چوڑائی() * 2;

      // Guard against overflow of the NewWidth itself
      اگر (New_چوڑائی < نتیجہ.get_ثنائی_ہندسہ_چوڑائی())
        واپس سچ;

      نتیجہ = نتیجہ.zext(New_چوڑائی);
      اگر (!IsPowerOf2Radix) {
        RadixAP = RadixAP.zext(New_چوڑائی);
        CharAP = CharAP.zext(New_چوڑائی);
      }
    }

    // Add in this character.
    اگر (IsPowerOf2Radix) {
      نتیجہ <<= Log2Radix;
      نتیجہ |= CharVal;
    } ورنہ {
      نتیجہ *= RadixAP;
      CharAP = CharVal;
      نتیجہ += CharAP;
    }

    Str = Str.substr(1);
  }

  // We consider the operation a failure if no characters were consumed
  // successfully.
  اگر (سائز_2() == Str.سائز_2())
    واپس سچ;

  *یہ = Str;
  واپس جھوٹ;
}

منطقی StringRef::getAsInteger(غیرعلامتی Radix, APInt &نتیجہ) ثابت {
  StringRef Str = *یہ;
  اگر (Str.consumeInteger(Radix, نتیجہ))
    واپس سچ;

  // For getAsInteger, we require the whole string to be consumed or else we
  // consider it a failure.
  واپس !Str.خالی_خانے();
}

منطقی StringRef::getAsDouble(دوہرا &نتیجہ, منطقی AllowInexact) ثابت {
  APFloat F(0.0);
  خودکار حالت_Or_Err = F.convertFromString(*یہ, APFloat::rm_قریب_ترین_Ties_مقصد_Even);
  اگر (error_مقصد_Bool(حالت_Or_Err.takeError()))
    واپس سچ;

  APFloat::op_حالت حالت_2 = *حالت_Or_Err;
  اگر (حالت_2 != APFloat::opOK) {
    اگر (!AllowInexact || !(حالت_2 & APFloat::opInexact))
      واپس سچ;
  }

  نتیجہ = F.convert_مقصد_Double();
  واپس جھوٹ;
}

// Implementation of StringRef hashing.
hash_رمز llvm::hash_قدر(StringRef S) { واپس hash_combine_range(S); }

غیرعلامتی DenseMapInfo<StringRef, خالی>::get_Hash_قدر(StringRef Val) {
  واپس (غیرعلامتی)(hash_قدر(Val));
}
