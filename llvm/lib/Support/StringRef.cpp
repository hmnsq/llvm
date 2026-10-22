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

استفاده نطاق llvm;

// strncasecmp() is not available on non-POSIX systems, so define an
// alternative function here.
ساكن صحيح ascii_strncasecmp(StringRef LHS, StringRef RHS) {
  لكل (خودکار [LC, RC] : zip_equal(LHS, RHS)) {
    بیعلامت حرف LHC = مقصد_پایینی(LC);
    بیعلامت حرف RHC = مقصد_پایینی(RC);
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

منطقي StringRef::starts_with_insensitive(StringRef پیشوند) ثابت {
  أرجع اندازه_2() >= پیشوند.اندازه_2() &&
         ascii_strncasecmp(take_front(پیشوند.اندازه_2()), پیشوند) == 0;
}

منطقي StringRef::دوسر_with_insensitive(StringRef Suffix) ثابت {
  أرجع اندازه_2() >= Suffix.اندازه_2() &&
         ascii_strncasecmp(take_پشت(Suffix.اندازه_2()), Suffix) == 0;
}

size_t StringRef::پیدا_کن_insensitive(حرف C, size_t From) ثابت {
  حرف L = مقصد_پایینی(C);
  أرجع پیدا_کن_if([L](حرف D) { أرجع مقصد_پایینی(D) == L; }, From);
}

/// compare_numeric - Compare strings, handle embedded numbers.
صحيح StringRef::compare_numeric(StringRef RHS) ثابت {
  لكل (size_t I = 0, E = std::min(اندازه_2(), RHS.اندازه_2()); I != E; ++I) {
    // Check for sequences of digits.
    إذا (isDigit(داده()[I]) && isDigit(RHS.داده()[I])) {
      // The longer sequence of numbers is considered larger.
      // This doesn't really handle prefixed zeros well.
      size_t J;
      لكل (J = I + 1; J != E + 1; ++J) {
        منطقي ld = J < اندازه_2() && isDigit(داده()[J]);
        منطقي rd = J < RHS.اندازه_2() && isDigit(RHS.داده()[J]);
        إذا (ld != rd)
          أرجع rd ? -1 : 1;
        إذا (!rd)
          اكسر;
      }
      // The two number sequences have the same length (J-I), just memcmp them.
      إذا (صحيح Res = compare_حافظه(داده() + I, RHS.داده() + I, J - I))
        أرجع Res < 0 ? -1 : 1;
      // Identical number sequences, continue search after the numbers.
      I = J - 1;
      استمر;
    }
    إذا (داده()[I] != RHS.داده()[I])
      أرجع (بیعلامت حرف)داده()[I] < (بیعلامت حرف)RHS.داده()[I] ? -1 : 1;
  }
  إذا (اندازه_2() == RHS.اندازه_2())
    أرجع 0;
  أرجع اندازه_2() < RHS.اندازه_2() ? -1 : 1;
}

// Compute the edit distance between the two given strings.
بیعلامت StringRef::edit_فاصله(llvm::StringRef Other,
                                  منطقي AllowReplacements,
                                  بیعلامت Max_Edit_فاصله) ثابت {
  أرجع llvm::Compute_Edit_فاصله(ArrayRef(داده(), اندازه_2()),
                                   ArrayRef(Other.داده(), Other.اندازه_2()),
                                   AllowReplacements, Max_Edit_فاصله);
}

بیعلامت llvm::StringRef::edit_فاصله_insensitive(
    StringRef Other, منطقي AllowReplacements, بیعلامت Max_Edit_فاصله) ثابت {
  أرجع llvm::Compute_Mapped_Edit_فاصله(
      ArrayRef(داده(), اندازه_2()), ArrayRef(Other.داده(), Other.اندازه_2()),
      llvm::مقصد_پایینی, AllowReplacements, Max_Edit_فاصله);
}

//===----------------------------------------------------------------------===//
// String Operations
//===----------------------------------------------------------------------===//

std::string StringRef::پایینی() ثابت {
  أرجع std::string(map_iterator(begin(), مقصد_پایینی),
                     map_iterator(انتها(), مقصد_پایینی));
}

std::string StringRef::فوقانی() ثابت {
  أرجع std::string(map_iterator(begin(), مقصد_فوقانی),
                     map_iterator(انتها(), مقصد_فوقانی));
}

//===----------------------------------------------------------------------===//
// String Searching
//===----------------------------------------------------------------------===//


/// find - Search for the first string \arg Str in the string.
///
/// \return - The index of the first occurrence of \arg Str, or npos if not
/// found.
size_t StringRef::پیدا_کن(StringRef Str, size_t From) ثابت {
  إذا (From > اندازه_2())
    أرجع npos;

  ثابت حرف *شروع = داده() + From;
  size_t اندازه = اندازه_2() - From;

  ثابت حرف *Needle = Str.داده();
  size_t N = Str.اندازه_2();
  إذا (N == 0)
    أرجع From;
  إذا (اندازه < N)
    أرجع npos;
  إذا (N == 1) {
    ثابت حرف *Ptr = (ثابت حرف *)::memchr(شروع, Needle[0], اندازه);
    أرجع Ptr == اشاره_تهی ? npos : Ptr - داده();
  }

  ثابت حرف *Stop = شروع + (اندازه - N + 1);

  إذا (N == 2) {
    // Provide a fast path for newline finding (CRLF case) in InclusionRewriter.
    // Not the most optimized strategy, but getting memcmp inlined should be
    // good enough.
    افعل {
      إذا (std::memcmp(شروع, Needle, 2) == 0)
        أرجع شروع - داده();
      ++شروع;
    } طالما (شروع < Stop);
    أرجع npos;
  }

  // For short haystacks or unsupported needles fall back to the naive algorithm
  إذا (اندازه < 16 || N > 255) {
    افعل {
      إذا (std::memcmp(شروع, Needle, N) == 0)
        أرجع شروع - داده();
      ++شروع;
    } طالما (شروع < Stop);
    أرجع npos;
  }

  // Build the bad char heuristic table, with uint8_t to reduce cache thrashing.
  uint8_t BadCharSkip[256];
  std::memset(BadCharSkip, N, 256);
  لكل (بیعلامت i = 0; i != N-1; ++i)
    BadCharSkip[(uint8_t)Str[i]] = N-1-i;

  افعل {
    uint8_t آخرین = شروع[N - 1];
    إذا (LLVM_UNLIKELY(آخرین == (uint8_t)Needle[N - 1]))
      إذا (std::memcmp(شروع, Needle, N - 1) == 0)
        أرجع شروع - داده();

    // Otherwise skip the appropriate number of bytes.
    شروع += BadCharSkip[آخرین];
  } طالما (شروع < Stop);

  أرجع npos;
}

size_t StringRef::پیدا_کن_insensitive(StringRef Str, size_t From) ثابت {
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
    إذا (مقصد_پایینی(داده()[i]) == مقصد_پایینی(C))
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
StringRef::اندازه_type StringRef::پیدا_کن_اولی_از(StringRef Chars,
                                              size_t From) ثابت {
  std::bitset<1 << CHAR_BIT> Char_بیتها;
  لكل (حرف C : Chars)
    Char_بیتها.مجموعه((بیعلامت حرف)C);

  لكل (اندازه_type i = std::min(From, اندازه_2()), e = اندازه_2(); i != e; ++i)
    إذا (Char_بیتها.test((بیعلامت حرف)داده()[i]))
      أرجع i;
  أرجع npos;
}

/// find_first_not_of - Find the first character in the string that is not
/// \arg C or npos if not found.
StringRef::اندازه_type StringRef::پیدا_کن_اولی_not_از(حرف C, size_t From) ثابت {
  أرجع std::string_view(*هذا).پیدا_کن_اولی_not_از(C, From);
}

/// find_first_not_of - Find the first character in the string that is not
/// in the string \arg Chars, or npos if not found.
///
/// Note: O(size() + Chars.size())
StringRef::اندازه_type StringRef::پیدا_کن_اولی_not_از(StringRef Chars,
                                                  size_t From) ثابت {
  std::bitset<1 << CHAR_BIT> Char_بیتها;
  لكل (حرف C : Chars)
    Char_بیتها.مجموعه((بیعلامت حرف)C);

  لكل (اندازه_type i = std::min(From, اندازه_2()), e = اندازه_2(); i != e; ++i)
    إذا (!Char_بیتها.test((بیعلامت حرف)داده()[i]))
      أرجع i;
  أرجع npos;
}

/// find_last_of - Find the last character in the string that is in \arg C,
/// or npos if not found.
///
/// Note: O(size() + Chars.size())
StringRef::اندازه_type StringRef::پیدا_کن_آخرین_از(StringRef Chars,
                                             size_t From) ثابت {
  std::bitset<1 << CHAR_BIT> Char_بیتها;
  لكل (حرف C : Chars)
    Char_بیتها.مجموعه((بیعلامت حرف)C);

  لكل (اندازه_type i = std::min(From, اندازه_2()) - 1, e = -1; i != e; --i)
    إذا (Char_بیتها.test((بیعلامت حرف)داده()[i]))
      أرجع i;
  أرجع npos;
}

/// find_last_not_of - Find the last character in the string that is not
/// \arg C, or npos if not found.
StringRef::اندازه_type StringRef::پیدا_کن_آخرین_not_از(حرف C, size_t From) ثابت {
  لكل (اندازه_type i = std::min(From, اندازه_2()) - 1, e = -1; i != e; --i)
    إذا (داده()[i] != C)
      أرجع i;
  أرجع npos;
}

/// find_last_not_of - Find the last character in the string that is not in
/// \arg Chars, or npos if not found.
///
/// Note: O(size() + Chars.size())
StringRef::اندازه_type StringRef::پیدا_کن_آخرین_not_از(StringRef Chars,
                                                 size_t From) ثابت {
  std::bitset<1 << CHAR_BIT> Char_بیتها;
  لكل (حرف C : Chars)
    Char_بیتها.مجموعه((بیعلامت حرف)C);

  لكل (اندازه_type i = std::min(From, اندازه_2()) - 1, e = -1; i != e; --i)
    إذا (!Char_بیتها.test((بیعلامت حرف)داده()[i]))
      أرجع i;
  أرجع npos;
}

فراغ StringRef::تقسیم(Small_وکتور_Impl<StringRef> &A,
                      StringRef Separator, صحيح Max_تقسیم,
                      منطقي Keep_خالی) ثابت {
  StringRef S = *هذا;

  // Count down from MaxSplit. When MaxSplit is -1, this will just split
  // "forever". This doesn't support splitting more than 2^31 times
  // intentionally; if we ever want that we can make MaxSplit a 64-bit integer
  // but that seems unlikely to be useful.
  طالما (Max_تقسیم-- != 0) {
    size_t Idx = S.پیدا_کن(Separator);
    إذا (Idx == npos)
      اكسر;

    // Push this split.
    إذا (Keep_خالی || Idx > 0)
      A.افزودن_پشت(S.برش(0, Idx));

    // Jump forward.
    S = S.substr(Idx + Separator.اندازه_2());
  }

  // Push the tail.
  إذا (Keep_خالی || !S.خالی())
    A.افزودن_پشت(S);
}

فراغ StringRef::تقسیم(Small_وکتور_Impl<StringRef> &A, حرف Separator,
                      صحيح Max_تقسیم, منطقي Keep_خالی) ثابت {
  StringRef S = *هذا;

  // Count down from MaxSplit. When MaxSplit is -1, this will just split
  // "forever". This doesn't support splitting more than 2^31 times
  // intentionally; if we ever want that we can make MaxSplit a 64-bit integer
  // but that seems unlikely to be useful.
  طالما (Max_تقسیم-- != 0) {
    size_t Idx = S.پیدا_کن(Separator);
    إذا (Idx == npos)
      اكسر;

    // Push this split.
    إذا (Keep_خالی || Idx > 0)
      A.افزودن_پشت(S.برش(0, Idx));

    // Jump forward.
    S = S.substr(Idx + 1);
  }

  // Push the tail.
  إذا (Keep_خالی || !S.خالی())
    A.افزودن_پشت(S);
}

//===----------------------------------------------------------------------===//
// Helpful Algorithms
//===----------------------------------------------------------------------===//

/// count - Return the number of non-overlapped occurrences of \arg Str in
/// the string.
size_t StringRef::شمار(StringRef Str) ثابت {
  size_t شمار_2 = 0;
  size_t Pos = 0;
  size_t N = Str.اندازه_2();
  // TODO: For an empty `Str` we return 0 for legacy reasons. Consider changing
  //       this to `Length + 1` which is more in-line with the function
  //       description.
  إذا (!N)
    أرجع 0;
  طالما ((Pos = پیدا_کن(Str, Pos)) != npos) {
    ++شمار_2;
    Pos += N;
  }
  أرجع شمار_2;
}

بیعلامت llvm::getAutoSenseRadix(StringRef &Str) {
  إذا (Str.خالی())
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

منطقي llvm::consumeUnsignedInteger(StringRef &Str, بیعلامت Radix,
                                  بیعلامت دراز دراز &نتیجه) {
  // Autosense radix if not specified.
  إذا (Radix == 0)
    Radix = getAutoSenseRadix(Str);

  // Empty strings (after the radix autosense) are invalid.
  إذا (Str.خالی()) أرجع درست;

  // Parse all the bytes of the string given this radix.  Watch for overflow.
  StringRef Str2 = Str;
  نتیجه = 0;
  طالما (!Str2.خالی()) {
    بیعلامت CharVal;
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
    بیعلامت دراز دراز Prev_نتیجه = نتیجه;
    نتیجه = نتیجه * Radix + CharVal;

    // Check for overflow by shifting back and seeing if bits were lost.
    إذا (نتیجه / Radix < Prev_نتیجه)
      أرجع درست;

    Str2 = Str2.substr(1);
  }

  // We consider the operation a failure if no characters were consumed
  // successfully.
  إذا (Str.اندازه_2() == Str2.اندازه_2())
    أرجع درست;

  Str = Str2;
  أرجع خطأ;
}

منطقي llvm::consumeSignedInteger(StringRef &Str, بیعلامت Radix,
                                دراز دراز &نتیجه) {
  بیعلامت دراز دراز ULLVal;

  // Handle positive strings first.
  إذا (!Str.starts_with("-")) {
    إذا (consumeUnsignedInteger(Str, Radix, ULLVal) ||
        // Check for value so large it overflows a signed value.
        (دراز دراز)ULLVal < 0)
      أرجع درست;
    نتیجه = ULLVal;
    أرجع خطأ;
  }

  // Get the positive part of the value.
  StringRef Str2 = Str.drop_front(1);
  إذا (consumeUnsignedInteger(Str2, Radix, ULLVal) ||
      // Reject values so large they'd overflow as negative signed, but allow
      // "-0".  This negates the unsigned so that the negative isn't undefined
      // on signed overflow.
      (دراز دراز)-ULLVal > 0)
    أرجع درست;

  Str = Str2;
  نتیجه = -ULLVal;
  أرجع خطأ;
}

/// GetAsUnsignedInteger - Workhorse method that converts a integer character
/// sequence of radix up to 36 to an unsigned long long value.
منطقي llvm::getAsUnsignedInteger(StringRef Str, بیعلامت Radix,
                                بیعلامت دراز دراز &نتیجه) {
  إذا (consumeUnsignedInteger(Str, Radix, نتیجه))
    أرجع درست;

  // For getAsUnsignedInteger, we require the whole string to be consumed or
  // else we consider it a failure.
  أرجع !Str.خالی();
}

منطقي llvm::getAsSignedInteger(StringRef Str, بیعلامت Radix,
                              دراز دراز &نتیجه) {
  إذا (consumeSignedInteger(Str, Radix, نتیجه))
    أرجع درست;

  // For getAsSignedInteger, we require the whole string to be consumed or else
  // we consider it a failure.
  أرجع !Str.خالی();
}

منطقي StringRef::consumeInteger(بیعلامت Radix, APInt &نتیجه) {
  StringRef Str = *هذا;

  // Autosense radix if not specified.
  إذا (Radix == 0)
    Radix = getAutoSenseRadix(Str);

  أكد(Radix > 1 && Radix <= 36);

  // Empty strings (after the radix autosense) are invalid.
  إذا (Str.خالی()) أرجع درست;

  // Skip leading zeroes.  This can be a significant improvement if
  // it means we don't need > 64 bits.
  Str = Str.ltrim('0');

  // If it was nothing but zeroes....
  إذا (Str.خالی()) {
    نتیجه = APInt(64, 0);
    *هذا = Str;
    أرجع خطأ;
  }

  بیعلامت Log2Radix = 0;
  طالما ((1U << Log2Radix) < Radix) Log2Radix++;
  منطقي IsPowerOf2Radix = ((1U << Log2Radix) == Radix);

  // Initialize Result to a reasonable starting width (at least 64 bits),
  // but do not shrink it if it was already larger.
  بیعلامت بیت_عرض_2 = std::max(64U, نتیجه.get_بیت_عرض());

  إذا (نتیجه.get_بیت_عرض() < بیت_عرض_2) {
    نتیجه = نتیجه.zext(بیت_عرض_2);
  }

  APInt RadixAP, CharAP; // unused unless !IsPowerOf2Radix
  إذا (!IsPowerOf2Radix) {
    // These must have the same bit-width as Result.
    RadixAP = APInt(بیت_عرض_2, Radix);
    CharAP = APInt(بیت_عرض_2, 0);
  }

  // Parse all the bytes of the string given this radix.
  نتیجه = 0;
  طالما (!Str.خالی()) {
    بیعلامت CharVal;
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
    إذا (نتیجه.get_فعال_بیتها() + Log2Radix > نتیجه.get_بیت_عرض()) {
      بیعلامت New_عرض = نتیجه.get_بیت_عرض() * 2;

      // Guard against overflow of the NewWidth itself
      إذا (New_عرض < نتیجه.get_بیت_عرض())
        أرجع درست;

      نتیجه = نتیجه.zext(New_عرض);
      إذا (!IsPowerOf2Radix) {
        RadixAP = RadixAP.zext(New_عرض);
        CharAP = CharAP.zext(New_عرض);
      }
    }

    // Add in this character.
    إذا (IsPowerOf2Radix) {
      نتیجه <<= Log2Radix;
      نتیجه |= CharVal;
    } وإلا {
      نتیجه *= RadixAP;
      CharAP = CharVal;
      نتیجه += CharAP;
    }

    Str = Str.substr(1);
  }

  // We consider the operation a failure if no characters were consumed
  // successfully.
  إذا (اندازه_2() == Str.اندازه_2())
    أرجع درست;

  *هذا = Str;
  أرجع خطأ;
}

منطقي StringRef::getAsInteger(بیعلامت Radix, APInt &نتیجه) ثابت {
  StringRef Str = *هذا;
  إذا (Str.consumeInteger(Radix, نتیجه))
    أرجع درست;

  // For getAsInteger, we require the whole string to be consumed or else we
  // consider it a failure.
  أرجع !Str.خالی();
}

منطقي StringRef::getAsDouble(مزدوج &نتیجه, منطقي AllowInexact) ثابت {
  APFloat F(0.0);
  خودکار وضعیت_Or_Err = F.convertFromString(*هذا, APFloat::rm_نزدیکترین_Ties_مقصد_Even);
  إذا (error_مقصد_Bool(وضعیت_Or_Err.takeError()))
    أرجع درست;

  APFloat::op_وضعیت وضعیت = *وضعیت_Or_Err;
  إذا (وضعیت != APFloat::opOK) {
    إذا (!AllowInexact || !(وضعیت & APFloat::opInexact))
      أرجع درست;
  }

  نتیجه = F.convert_مقصد_Double();
  أرجع خطأ;
}

// Implementation of StringRef hashing.
hash_کد llvm::hash_مقدار(StringRef S) { أرجع hash_combine_range(S); }

بیعلامت DenseMapInfo<StringRef, فراغ>::get_Hash_مقدار(StringRef Val) {
  أرجع (بیعلامت)(hash_مقدار(Val));
}
