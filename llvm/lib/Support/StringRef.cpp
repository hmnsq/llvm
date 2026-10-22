//===-- StringRef.cpp - Lightweight String References ---------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#ਅਨ੍ਤਰ੍ਭਾਵ "llvm/ADT/StringRef.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "llvm/ADT/APFloat.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "llvm/ADT/APInt.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "llvm/ADT/Hashing.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "llvm/ADT/StringExtras.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "llvm/ADT/edit_distance.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "llvm/Support/Error.h"
#ਅਨ੍ਤਰ੍ਭਾਵ <bitset>

ਵਰਤੋਂ ਨਾਮਾਕਾਸ਼ llvm;

// strncasecmp() is not available on non-POSIX systems, so define an
// alternative function here.
ਸਥਿਰ ਪੂਰਨ ascii_strncasecmp(StringRef LHS, StringRef RHS) {
  ਲਈ (ਸਵੈਚਾਲਿਤ [LC, RC] : zip_equal(LHS, RHS)) {
    ਬਿਨਾਨਿਸ਼ਾਨੀ ਅੱਖਰ LHC = ਮੰਜ਼ਿਲ_ਹੇਠਲਾ(LC);
    ਬਿਨਾਨਿਸ਼ਾਨੀ ਅੱਖਰ RHC = ਮੰਜ਼ਿਲ_ਹੇਠਲਾ(RC);
    ਜੇ (LHC != RHC)
      ਵਾਪਸ LHC < RHC ? -1 : 1;
  }
  ਵਾਪਸ 0;
}

ਪੂਰਨ StringRef::compare_insensitive(StringRef RHS) ਅਚਲ {
  size_t Min = std::min(ਮਾਪ_2(), RHS.ਮਾਪ_2());
  ਜੇ (ਪੂਰਨ Res = ascii_strncasecmp(take_front(Min), RHS.take_front(Min)))
    ਵਾਪਸ Res;
  ਜੇ (ਮਾਪ_2() == RHS.ਮਾਪ_2())
    ਵਾਪਸ 0;
  ਵਾਪਸ ਮਾਪ_2() < RHS.ਮਾਪ_2() ? -1 : 1;
}

ਤਾਰਕਿਕ StringRef::starts_with_insensitive(StringRef ਅਗੇਤਰ) ਅਚਲ {
  ਵਾਪਸ ਮਾਪ_2() >= ਅਗੇਤਰ.ਮਾਪ_2() &&
         ascii_strncasecmp(take_front(ਅਗੇਤਰ.ਮਾਪ_2()), ਅਗੇਤਰ) == 0;
}

ਤਾਰਕਿਕ StringRef::ਸਿਰੇ_with_insensitive(StringRef Suffix) ਅਚਲ {
  ਵਾਪਸ ਮਾਪ_2() >= Suffix.ਮਾਪ_2() &&
         ascii_strncasecmp(take_ਪਿੱਠ(Suffix.ਮਾਪ_2()), Suffix) == 0;
}

size_t StringRef::ਲੱਭੋ_insensitive(ਅੱਖਰ C, size_t From) ਅਚਲ {
  ਅੱਖਰ L = ਮੰਜ਼ਿਲ_ਹੇਠਲਾ(C);
  ਵਾਪਸ ਲੱਭੋ_if([L](ਅੱਖਰ D) { ਵਾਪਸ ਮੰਜ਼ਿਲ_ਹੇਠਲਾ(D) == L; }, From);
}

/// compare_numeric - Compare strings, handle embedded numbers.
ਪੂਰਨ StringRef::compare_numeric(StringRef RHS) ਅਚਲ {
  ਲਈ (size_t I = 0, E = std::min(ਮਾਪ_2(), RHS.ਮਾਪ_2()); I != E; ++I) {
    // Check for sequences of digits.
    ਜੇ (isDigit(ਅੰਕੜੇ()[I]) && isDigit(RHS.ਅੰਕੜੇ()[I])) {
      // The longer sequence of numbers is considered larger.
      // This doesn't really handle prefixed zeros well.
      size_t J;
      ਲਈ (J = I + 1; J != E + 1; ++J) {
        ਤਾਰਕਿਕ ld = J < ਮਾਪ_2() && isDigit(ਅੰਕੜੇ()[J]);
        ਤਾਰਕਿਕ rd = J < RHS.ਮਾਪ_2() && isDigit(RHS.ਅੰਕੜੇ()[J]);
        ਜੇ (ld != rd)
          ਵਾਪਸ rd ? -1 : 1;
        ਜੇ (!rd)
          ਰੋਕੋ;
      }
      // The two number sequences have the same length (J-I), just memcmp them.
      ਜੇ (ਪੂਰਨ Res = compare_ਸਮ੍ਰਿਤੀ(ਅੰਕੜੇ() + I, RHS.ਅੰਕੜੇ() + I, J - I))
        ਵਾਪਸ Res < 0 ? -1 : 1;
      // Identical number sequences, continue search after the numbers.
      I = J - 1;
      ਜਾਰੀ;
    }
    ਜੇ (ਅੰਕੜੇ()[I] != RHS.ਅੰਕੜੇ()[I])
      ਵਾਪਸ (ਬਿਨਾਨਿਸ਼ਾਨੀ ਅੱਖਰ)ਅੰਕੜੇ()[I] < (ਬਿਨਾਨਿਸ਼ਾਨੀ ਅੱਖਰ)RHS.ਅੰਕੜੇ()[I] ? -1 : 1;
  }
  ਜੇ (ਮਾਪ_2() == RHS.ਮਾਪ_2())
    ਵਾਪਸ 0;
  ਵਾਪਸ ਮਾਪ_2() < RHS.ਮਾਪ_2() ? -1 : 1;
}

// Compute the edit distance between the two given strings.
ਬਿਨਾਨਿਸ਼ਾਨੀ StringRef::edit_ਦੂਰੀ(llvm::StringRef Other,
                                  ਤਾਰਕਿਕ AllowReplacements,
                                  ਬਿਨਾਨਿਸ਼ਾਨੀ Max_Edit_ਦੂਰੀ) ਅਚਲ {
  ਵਾਪਸ llvm::Compute_Edit_ਦੂਰੀ(ArrayRef(ਅੰਕੜੇ(), ਮਾਪ_2()),
                                   ArrayRef(Other.ਅੰਕੜੇ(), Other.ਮਾਪ_2()),
                                   AllowReplacements, Max_Edit_ਦੂਰੀ);
}

ਬਿਨਾਨਿਸ਼ਾਨੀ llvm::StringRef::edit_ਦੂਰੀ_insensitive(
    StringRef Other, ਤਾਰਕਿਕ AllowReplacements, ਬਿਨਾਨਿਸ਼ਾਨੀ Max_Edit_ਦੂਰੀ) ਅਚਲ {
  ਵਾਪਸ llvm::Compute_Mapped_Edit_ਦੂਰੀ(
      ArrayRef(ਅੰਕੜੇ(), ਮਾਪ_2()), ArrayRef(Other.ਅੰਕੜੇ(), Other.ਮਾਪ_2()),
      llvm::ਮੰਜ਼ਿਲ_ਹੇਠਲਾ, AllowReplacements, Max_Edit_ਦੂਰੀ);
}

//===----------------------------------------------------------------------===//
// String Operations
//===----------------------------------------------------------------------===//

std::string StringRef::ਹੇਠਲਾ() ਅਚਲ {
  ਵਾਪਸ std::string(map_iterator(begin(), ਮੰਜ਼ਿਲ_ਹੇਠਲਾ),
                     map_iterator(ਸਿਰਾ(), ਮੰਜ਼ਿਲ_ਹੇਠਲਾ));
}

std::string StringRef::ਉੱਤਲਾ() ਅਚਲ {
  ਵਾਪਸ std::string(map_iterator(begin(), ਮੰਜ਼ਿਲ_ਉੱਤਲਾ),
                     map_iterator(ਸਿਰਾ(), ਮੰਜ਼ਿਲ_ਉੱਤਲਾ));
}

//===----------------------------------------------------------------------===//
// String Searching
//===----------------------------------------------------------------------===//


/// find - Search for the first string \arg Str in the string.
///
/// \return - The index of the first occurrence of \arg Str, or npos if not
/// found.
size_t StringRef::ਲੱਭੋ(StringRef Str, size_t From) ਅਚਲ {
  ਜੇ (From > ਮਾਪ_2())
    ਵਾਪਸ npos;

  ਅਚਲ ਅੱਖਰ *ਸ਼ੁਰੂ = ਅੰਕੜੇ() + From;
  size_t ਮਾਪ = ਮਾਪ_2() - From;

  ਅਚਲ ਅੱਖਰ *Needle = Str.ਅੰਕੜੇ();
  size_t N = Str.ਮਾਪ_2();
  ਜੇ (N == 0)
    ਵਾਪਸ From;
  ਜੇ (ਮਾਪ < N)
    ਵਾਪਸ npos;
  ਜੇ (N == 1) {
    ਅਚਲ ਅੱਖਰ *Ptr = (ਅਚਲ ਅੱਖਰ *)::memchr(ਸ਼ੁਰੂ, Needle[0], ਮਾਪ);
    ਵਾਪਸ Ptr == ਖਾਲੀ_ਸੂਚਕ ? npos : Ptr - ਅੰਕੜੇ();
  }

  ਅਚਲ ਅੱਖਰ *Stop = ਸ਼ੁਰੂ + (ਮਾਪ - N + 1);

  ਜੇ (N == 2) {
    // Provide a fast path for newline finding (CRLF case) in InclusionRewriter.
    // Not the most optimized strategy, but getting memcmp inlined should be
    // good enough.
    ਕਰੋ {
      ਜੇ (std::memcmp(ਸ਼ੁਰੂ, Needle, 2) == 0)
        ਵਾਪਸ ਸ਼ੁਰੂ - ਅੰਕੜੇ();
      ++ਸ਼ੁਰੂ;
    } ਜਦੋਂ (ਸ਼ੁਰੂ < Stop);
    ਵਾਪਸ npos;
  }

  // For short haystacks or unsupported needles fall back to the naive algorithm
  ਜੇ (ਮਾਪ < 16 || N > 255) {
    ਕਰੋ {
      ਜੇ (std::memcmp(ਸ਼ੁਰੂ, Needle, N) == 0)
        ਵਾਪਸ ਸ਼ੁਰੂ - ਅੰਕੜੇ();
      ++ਸ਼ੁਰੂ;
    } ਜਦੋਂ (ਸ਼ੁਰੂ < Stop);
    ਵਾਪਸ npos;
  }

  // Build the bad char heuristic table, with uint8_t to reduce cache thrashing.
  uint8_t BadCharSkip[256];
  std::memset(BadCharSkip, N, 256);
  ਲਈ (ਬਿਨਾਨਿਸ਼ਾਨੀ i = 0; i != N-1; ++i)
    BadCharSkip[(uint8_t)Str[i]] = N-1-i;

  ਕਰੋ {
    uint8_t ਆਖਰੀ = ਸ਼ੁਰੂ[N - 1];
    ਜੇ (LLVM_UNLIKELY(ਆਖਰੀ == (uint8_t)Needle[N - 1]))
      ਜੇ (std::memcmp(ਸ਼ੁਰੂ, Needle, N - 1) == 0)
        ਵਾਪਸ ਸ਼ੁਰੂ - ਅੰਕੜੇ();

    // Otherwise skip the appropriate number of bytes.
    ਸ਼ੁਰੂ += BadCharSkip[ਆਖਰੀ];
  } ਜਦੋਂ (ਸ਼ੁਰੂ < Stop);

  ਵਾਪਸ npos;
}

size_t StringRef::ਲੱਭੋ_insensitive(StringRef Str, size_t From) ਅਚਲ {
  StringRef This = substr(From);
  ਜਦੋਂ (This.ਮਾਪ_2() >= Str.ਮਾਪ_2()) {
    ਜੇ (This.starts_with_insensitive(Str))
      ਵਾਪਸ From;
    This = This.drop_front();
    ++From;
  }
  ਵਾਪਸ npos;
}

size_t StringRef::rfind_insensitive(ਅੱਖਰ C, size_t From) ਅਚਲ {
  From = std::min(From, ਮਾਪ_2());
  size_t i = From;
  ਜਦੋਂ (i != 0) {
    --i;
    ਜੇ (ਮੰਜ਼ਿਲ_ਹੇਠਲਾ(ਅੰਕੜੇ()[i]) == ਮੰਜ਼ਿਲ_ਹੇਠਲਾ(C))
      ਵਾਪਸ i;
  }
  ਵਾਪਸ npos;
}

/// rfind - Search for the last string \arg Str in the string.
///
/// \return - The index of the last occurrence of \arg Str, or npos if not
/// found.
size_t StringRef::rfind(StringRef Str) ਅਚਲ {
  ਵਾਪਸ std::string_view(*ਇਹ).rfind(Str);
}

size_t StringRef::rfind_insensitive(StringRef Str) ਅਚਲ {
  size_t N = Str.ਮਾਪ_2();
  ਜੇ (N > ਮਾਪ_2())
    ਵਾਪਸ npos;
  ਲਈ (size_t i = ਮਾਪ_2() - N + 1, e = 0; i != e;) {
    --i;
    ਜੇ (substr(i, N).equals_insensitive(Str))
      ਵਾਪਸ i;
  }
  ਵਾਪਸ npos;
}

/// find_first_of - Find the first character in the string that is in \arg
/// Chars, or npos if not found.
///
/// Note: O(size() + Chars.size())
StringRef::ਮਾਪ_type StringRef::ਲੱਭੋ_ਪਹਿਲਾ_ਦਾ(StringRef Chars,
                                              size_t From) ਅਚਲ {
  std::bitset<1 << CHAR_BIT> Char_ਬਿੱਟ;
  ਲਈ (ਅੱਖਰ C : Chars)
    Char_ਬਿੱਟ.ਸੈੱਟ((ਬਿਨਾਨਿਸ਼ਾਨੀ ਅੱਖਰ)C);

  ਲਈ (ਮਾਪ_type i = std::min(From, ਮਾਪ_2()), e = ਮਾਪ_2(); i != e; ++i)
    ਜੇ (Char_ਬਿੱਟ.test((ਬਿਨਾਨਿਸ਼ਾਨੀ ਅੱਖਰ)ਅੰਕੜੇ()[i]))
      ਵਾਪਸ i;
  ਵਾਪਸ npos;
}

/// find_first_not_of - Find the first character in the string that is not
/// \arg C or npos if not found.
StringRef::ਮਾਪ_type StringRef::ਲੱਭੋ_ਪਹਿਲਾ_not_ਦਾ(ਅੱਖਰ C, size_t From) ਅਚਲ {
  ਵਾਪਸ std::string_view(*ਇਹ).ਲੱਭੋ_ਪਹਿਲਾ_not_ਦਾ(C, From);
}

/// find_first_not_of - Find the first character in the string that is not
/// in the string \arg Chars, or npos if not found.
///
/// Note: O(size() + Chars.size())
StringRef::ਮਾਪ_type StringRef::ਲੱਭੋ_ਪਹਿਲਾ_not_ਦਾ(StringRef Chars,
                                                  size_t From) ਅਚਲ {
  std::bitset<1 << CHAR_BIT> Char_ਬਿੱਟ;
  ਲਈ (ਅੱਖਰ C : Chars)
    Char_ਬਿੱਟ.ਸੈੱਟ((ਬਿਨਾਨਿਸ਼ਾਨੀ ਅੱਖਰ)C);

  ਲਈ (ਮਾਪ_type i = std::min(From, ਮਾਪ_2()), e = ਮਾਪ_2(); i != e; ++i)
    ਜੇ (!Char_ਬਿੱਟ.test((ਬਿਨਾਨਿਸ਼ਾਨੀ ਅੱਖਰ)ਅੰਕੜੇ()[i]))
      ਵਾਪਸ i;
  ਵਾਪਸ npos;
}

/// find_last_of - Find the last character in the string that is in \arg C,
/// or npos if not found.
///
/// Note: O(size() + Chars.size())
StringRef::ਮਾਪ_type StringRef::ਲੱਭੋ_ਆਖਰੀ_ਦਾ(StringRef Chars,
                                             size_t From) ਅਚਲ {
  std::bitset<1 << CHAR_BIT> Char_ਬਿੱਟ;
  ਲਈ (ਅੱਖਰ C : Chars)
    Char_ਬਿੱਟ.ਸੈੱਟ((ਬਿਨਾਨਿਸ਼ਾਨੀ ਅੱਖਰ)C);

  ਲਈ (ਮਾਪ_type i = std::min(From, ਮਾਪ_2()) - 1, e = -1; i != e; --i)
    ਜੇ (Char_ਬਿੱਟ.test((ਬਿਨਾਨਿਸ਼ਾਨੀ ਅੱਖਰ)ਅੰਕੜੇ()[i]))
      ਵਾਪਸ i;
  ਵਾਪਸ npos;
}

/// find_last_not_of - Find the last character in the string that is not
/// \arg C, or npos if not found.
StringRef::ਮਾਪ_type StringRef::ਲੱਭੋ_ਆਖਰੀ_not_ਦਾ(ਅੱਖਰ C, size_t From) ਅਚਲ {
  ਲਈ (ਮਾਪ_type i = std::min(From, ਮਾਪ_2()) - 1, e = -1; i != e; --i)
    ਜੇ (ਅੰਕੜੇ()[i] != C)
      ਵਾਪਸ i;
  ਵਾਪਸ npos;
}

/// find_last_not_of - Find the last character in the string that is not in
/// \arg Chars, or npos if not found.
///
/// Note: O(size() + Chars.size())
StringRef::ਮਾਪ_type StringRef::ਲੱਭੋ_ਆਖਰੀ_not_ਦਾ(StringRef Chars,
                                                 size_t From) ਅਚਲ {
  std::bitset<1 << CHAR_BIT> Char_ਬਿੱਟ;
  ਲਈ (ਅੱਖਰ C : Chars)
    Char_ਬਿੱਟ.ਸੈੱਟ((ਬਿਨਾਨਿਸ਼ਾਨੀ ਅੱਖਰ)C);

  ਲਈ (ਮਾਪ_type i = std::min(From, ਮਾਪ_2()) - 1, e = -1; i != e; --i)
    ਜੇ (!Char_ਬਿੱਟ.test((ਬਿਨਾਨਿਸ਼ਾਨੀ ਅੱਖਰ)ਅੰਕੜੇ()[i]))
      ਵਾਪਸ i;
  ਵਾਪਸ npos;
}

ਖਾਲੀ StringRef::ਵੰਡੋ(Small_ਸਦਿਸ਼_Impl<StringRef> &A,
                      StringRef Separator, ਪੂਰਨ Max_ਵੰਡੋ,
                      ਤਾਰਕਿਕ Keep_ਖਾਲੀ_ਥਾਵਾਂ) ਅਚਲ {
  StringRef S = *ਇਹ;

  // Count down from MaxSplit. When MaxSplit is -1, this will just split
  // "forever". This doesn't support splitting more than 2^31 times
  // intentionally; if we ever want that we can make MaxSplit a 64-bit integer
  // but that seems unlikely to be useful.
  ਜਦੋਂ (Max_ਵੰਡੋ-- != 0) {
    size_t Idx = S.ਲੱਭੋ(Separator);
    ਜੇ (Idx == npos)
      ਰੋਕੋ;

    // Push this split.
    ਜੇ (Keep_ਖਾਲੀ_ਥਾਵਾਂ || Idx > 0)
      A.ਧੱਕੋ_ਪਿੱਠ(S.ਟੁਕੜਾ(0, Idx));

    // Jump forward.
    S = S.substr(Idx + Separator.ਮਾਪ_2());
  }

  // Push the tail.
  ਜੇ (Keep_ਖਾਲੀ_ਥਾਵਾਂ || !S.ਖਾਲੀ_ਥਾਵਾਂ())
    A.ਧੱਕੋ_ਪਿੱਠ(S);
}

ਖਾਲੀ StringRef::ਵੰਡੋ(Small_ਸਦਿਸ਼_Impl<StringRef> &A, ਅੱਖਰ Separator,
                      ਪੂਰਨ Max_ਵੰਡੋ, ਤਾਰਕਿਕ Keep_ਖਾਲੀ_ਥਾਵਾਂ) ਅਚਲ {
  StringRef S = *ਇਹ;

  // Count down from MaxSplit. When MaxSplit is -1, this will just split
  // "forever". This doesn't support splitting more than 2^31 times
  // intentionally; if we ever want that we can make MaxSplit a 64-bit integer
  // but that seems unlikely to be useful.
  ਜਦੋਂ (Max_ਵੰਡੋ-- != 0) {
    size_t Idx = S.ਲੱਭੋ(Separator);
    ਜੇ (Idx == npos)
      ਰੋਕੋ;

    // Push this split.
    ਜੇ (Keep_ਖਾਲੀ_ਥਾਵਾਂ || Idx > 0)
      A.ਧੱਕੋ_ਪਿੱਠ(S.ਟੁਕੜਾ(0, Idx));

    // Jump forward.
    S = S.substr(Idx + 1);
  }

  // Push the tail.
  ਜੇ (Keep_ਖਾਲੀ_ਥਾਵਾਂ || !S.ਖਾਲੀ_ਥਾਵਾਂ())
    A.ਧੱਕੋ_ਪਿੱਠ(S);
}

//===----------------------------------------------------------------------===//
// Helpful Algorithms
//===----------------------------------------------------------------------===//

/// count - Return the number of non-overlapped occurrences of \arg Str in
/// the string.
size_t StringRef::ਗਿਣਤੀ_2(StringRef Str) ਅਚਲ {
  size_t ਗਿਣਤੀ_3 = 0;
  size_t Pos = 0;
  size_t N = Str.ਮਾਪ_2();
  // TODO: For an empty `Str` we return 0 for legacy reasons. Consider changing
  //       this to `Length + 1` which is more in-line with the function
  //       description.
  ਜੇ (!N)
    ਵਾਪਸ 0;
  ਜਦੋਂ ((Pos = ਲੱਭੋ(Str, Pos)) != npos) {
    ++ਗਿਣਤੀ_3;
    Pos += N;
  }
  ਵਾਪਸ ਗਿਣਤੀ_3;
}

ਬਿਨਾਨਿਸ਼ਾਨੀ llvm::getAutoSenseRadix(StringRef &Str) {
  ਜੇ (Str.ਖਾਲੀ_ਥਾਵਾਂ())
    ਵਾਪਸ 10;

  ਜੇ (Str.consume_front_insensitive("0x"))
    ਵਾਪਸ 16;

  ਜੇ (Str.consume_front_insensitive("0b"))
    ਵਾਪਸ 2;

  ਜੇ (Str.consume_front("0o"))
    ਵਾਪਸ 8;

  ਜੇ (Str[0] == '0' && Str.ਮਾਪ_2() > 1 && isDigit(Str[1])) {
    Str = Str.substr(1);
    ਵਾਪਸ 8;
  }

  ਵਾਪਸ 10;
}

ਤਾਰਕਿਕ llvm::consumeUnsignedInteger(StringRef &Str, ਬਿਨਾਨਿਸ਼ਾਨੀ Radix,
                                  ਬਿਨਾਨਿਸ਼ਾਨੀ ਲੰਮਾ ਲੰਮਾ &ਨਤੀਜਾ) {
  // Autosense radix if not specified.
  ਜੇ (Radix == 0)
    Radix = getAutoSenseRadix(Str);

  // Empty strings (after the radix autosense) are invalid.
  ਜੇ (Str.ਖਾਲੀ_ਥਾਵਾਂ()) ਵਾਪਸ ਸੱਚ;

  // Parse all the bytes of the string given this radix.  Watch for overflow.
  StringRef Str2 = Str;
  ਨਤੀਜਾ = 0;
  ਜਦੋਂ (!Str2.ਖਾਲੀ_ਥਾਵਾਂ()) {
    ਬਿਨਾਨਿਸ਼ਾਨੀ CharVal;
    ਜੇ (Str2[0] >= '0' && Str2[0] <= '9')
      CharVal = Str2[0] - '0';
    ਨਹੀਂਤਾਂ ਜੇ (Str2[0] >= 'a' && Str2[0] <= 'z')
      CharVal = Str2[0] - 'a' + 10;
    ਨਹੀਂਤਾਂ ਜੇ (Str2[0] >= 'A' && Str2[0] <= 'Z')
      CharVal = Str2[0] - 'A' + 10;
    ਨਹੀਂਤਾਂ
      ਰੋਕੋ;

    // If the parsed value is larger than the integer radix, we cannot
    // consume any more characters.
    ਜੇ (CharVal >= Radix)
      ਰੋਕੋ;

    // Add in this character.
    ਬਿਨਾਨਿਸ਼ਾਨੀ ਲੰਮਾ ਲੰਮਾ Prev_ਨਤੀਜਾ = ਨਤੀਜਾ;
    ਨਤੀਜਾ = ਨਤੀਜਾ * Radix + CharVal;

    // Check for overflow by shifting back and seeing if bits were lost.
    ਜੇ (ਨਤੀਜਾ / Radix < Prev_ਨਤੀਜਾ)
      ਵਾਪਸ ਸੱਚ;

    Str2 = Str2.substr(1);
  }

  // We consider the operation a failure if no characters were consumed
  // successfully.
  ਜੇ (Str.ਮਾਪ_2() == Str2.ਮਾਪ_2())
    ਵਾਪਸ ਸੱਚ;

  Str = Str2;
  ਵਾਪਸ ਝੂਠ;
}

ਤਾਰਕਿਕ llvm::consumeSignedInteger(StringRef &Str, ਬਿਨਾਨਿਸ਼ਾਨੀ Radix,
                                ਲੰਮਾ ਲੰਮਾ &ਨਤੀਜਾ) {
  ਬਿਨਾਨਿਸ਼ਾਨੀ ਲੰਮਾ ਲੰਮਾ ULLVal;

  // Handle positive strings first.
  ਜੇ (!Str.starts_with("-")) {
    ਜੇ (consumeUnsignedInteger(Str, Radix, ULLVal) ||
        // Check for value so large it overflows a signed value.
        (ਲੰਮਾ ਲੰਮਾ)ULLVal < 0)
      ਵਾਪਸ ਸੱਚ;
    ਨਤੀਜਾ = ULLVal;
    ਵਾਪਸ ਝੂਠ;
  }

  // Get the positive part of the value.
  StringRef Str2 = Str.drop_front(1);
  ਜੇ (consumeUnsignedInteger(Str2, Radix, ULLVal) ||
      // Reject values so large they'd overflow as negative signed, but allow
      // "-0".  This negates the unsigned so that the negative isn't undefined
      // on signed overflow.
      (ਲੰਮਾ ਲੰਮਾ)-ULLVal > 0)
    ਵਾਪਸ ਸੱਚ;

  Str = Str2;
  ਨਤੀਜਾ = -ULLVal;
  ਵਾਪਸ ਝੂਠ;
}

/// GetAsUnsignedInteger - Workhorse method that converts a integer character
/// sequence of radix up to 36 to an unsigned long long value.
ਤਾਰਕਿਕ llvm::getAsUnsignedInteger(StringRef Str, ਬਿਨਾਨਿਸ਼ਾਨੀ Radix,
                                ਬਿਨਾਨਿਸ਼ਾਨੀ ਲੰਮਾ ਲੰਮਾ &ਨਤੀਜਾ) {
  ਜੇ (consumeUnsignedInteger(Str, Radix, ਨਤੀਜਾ))
    ਵਾਪਸ ਸੱਚ;

  // For getAsUnsignedInteger, we require the whole string to be consumed or
  // else we consider it a failure.
  ਵਾਪਸ !Str.ਖਾਲੀ_ਥਾਵਾਂ();
}

ਤਾਰਕਿਕ llvm::getAsSignedInteger(StringRef Str, ਬਿਨਾਨਿਸ਼ਾਨੀ Radix,
                              ਲੰਮਾ ਲੰਮਾ &ਨਤੀਜਾ) {
  ਜੇ (consumeSignedInteger(Str, Radix, ਨਤੀਜਾ))
    ਵਾਪਸ ਸੱਚ;

  // For getAsSignedInteger, we require the whole string to be consumed or else
  // we consider it a failure.
  ਵਾਪਸ !Str.ਖਾਲੀ_ਥਾਵਾਂ();
}

ਤਾਰਕਿਕ StringRef::consumeInteger(ਬਿਨਾਨਿਸ਼ਾਨੀ Radix, APInt &ਨਤੀਜਾ) {
  StringRef Str = *ਇਹ;

  // Autosense radix if not specified.
  ਜੇ (Radix == 0)
    Radix = getAutoSenseRadix(Str);

  ਪ੍ਰਤਿਜ੍ਞਾ(Radix > 1 && Radix <= 36);

  // Empty strings (after the radix autosense) are invalid.
  ਜੇ (Str.ਖਾਲੀ_ਥਾਵਾਂ()) ਵਾਪਸ ਸੱਚ;

  // Skip leading zeroes.  This can be a significant improvement if
  // it means we don't need > 64 bits.
  Str = Str.ltrim('0');

  // If it was nothing but zeroes....
  ਜੇ (Str.ਖਾਲੀ_ਥਾਵਾਂ()) {
    ਨਤੀਜਾ = APInt(64, 0);
    *ਇਹ = Str;
    ਵਾਪਸ ਝੂਠ;
  }

  ਬਿਨਾਨਿਸ਼ਾਨੀ Log2Radix = 0;
  ਜਦੋਂ ((1U << Log2Radix) < Radix) Log2Radix++;
  ਤਾਰਕਿਕ IsPowerOf2Radix = ((1U << Log2Radix) == Radix);

  // Initialize Result to a reasonable starting width (at least 64 bits),
  // but do not shrink it if it was already larger.
  ਬਿਨਾਨਿਸ਼ਾਨੀ ਦੋਆਧਾਰੀ_ਅੰਕ_ਚੌੜਾਈ_2 = std::max(64U, ਨਤੀਜਾ.get_ਦੋਆਧਾਰੀ_ਅੰਕ_ਚੌੜਾਈ());

  ਜੇ (ਨਤੀਜਾ.get_ਦੋਆਧਾਰੀ_ਅੰਕ_ਚੌੜਾਈ() < ਦੋਆਧਾਰੀ_ਅੰਕ_ਚੌੜਾਈ_2) {
    ਨਤੀਜਾ = ਨਤੀਜਾ.zext(ਦੋਆਧਾਰੀ_ਅੰਕ_ਚੌੜਾਈ_2);
  }

  APInt RadixAP, CharAP; // unused unless !IsPowerOf2Radix
  ਜੇ (!IsPowerOf2Radix) {
    // These must have the same bit-width as Result.
    RadixAP = APInt(ਦੋਆਧਾਰੀ_ਅੰਕ_ਚੌੜਾਈ_2, Radix);
    CharAP = APInt(ਦੋਆਧਾਰੀ_ਅੰਕ_ਚੌੜਾਈ_2, 0);
  }

  // Parse all the bytes of the string given this radix.
  ਨਤੀਜਾ = 0;
  ਜਦੋਂ (!Str.ਖਾਲੀ_ਥਾਵਾਂ()) {
    ਬਿਨਾਨਿਸ਼ਾਨੀ CharVal;
    ਜੇ (Str[0] >= '0' && Str[0] <= '9')
      CharVal = Str[0]-'0';
    ਨਹੀਂਤਾਂ ਜੇ (Str[0] >= 'a' && Str[0] <= 'z')
      CharVal = Str[0]-'a'+10;
    ਨਹੀਂਤਾਂ ਜੇ (Str[0] >= 'A' && Str[0] <= 'Z')
      CharVal = Str[0]-'A'+10;
    ਨਹੀਂਤਾਂ
      ਰੋਕੋ;

    // If the parsed value is larger than the integer radix, the string is
    // invalid.
    ਜੇ (CharVal >= Radix)
      ਰੋਕੋ;

    // Check if one more digit will overflow, and grow if so.
    ਜੇ (ਨਤੀਜਾ.get_ਸਰਗਰਮ_ਬਿੱਟ() + Log2Radix > ਨਤੀਜਾ.get_ਦੋਆਧਾਰੀ_ਅੰਕ_ਚੌੜਾਈ()) {
      ਬਿਨਾਨਿਸ਼ਾਨੀ New_ਚੌੜਾਈ = ਨਤੀਜਾ.get_ਦੋਆਧਾਰੀ_ਅੰਕ_ਚੌੜਾਈ() * 2;

      // Guard against overflow of the NewWidth itself
      ਜੇ (New_ਚੌੜਾਈ < ਨਤੀਜਾ.get_ਦੋਆਧਾਰੀ_ਅੰਕ_ਚੌੜਾਈ())
        ਵਾਪਸ ਸੱਚ;

      ਨਤੀਜਾ = ਨਤੀਜਾ.zext(New_ਚੌੜਾਈ);
      ਜੇ (!IsPowerOf2Radix) {
        RadixAP = RadixAP.zext(New_ਚੌੜਾਈ);
        CharAP = CharAP.zext(New_ਚੌੜਾਈ);
      }
    }

    // Add in this character.
    ਜੇ (IsPowerOf2Radix) {
      ਨਤੀਜਾ <<= Log2Radix;
      ਨਤੀਜਾ |= CharVal;
    } ਨਹੀਂਤਾਂ {
      ਨਤੀਜਾ *= RadixAP;
      CharAP = CharVal;
      ਨਤੀਜਾ += CharAP;
    }

    Str = Str.substr(1);
  }

  // We consider the operation a failure if no characters were consumed
  // successfully.
  ਜੇ (ਮਾਪ_2() == Str.ਮਾਪ_2())
    ਵਾਪਸ ਸੱਚ;

  *ਇਹ = Str;
  ਵਾਪਸ ਝੂਠ;
}

ਤਾਰਕਿਕ StringRef::getAsInteger(ਬਿਨਾਨਿਸ਼ਾਨੀ Radix, APInt &ਨਤੀਜਾ) ਅਚਲ {
  StringRef Str = *ਇਹ;
  ਜੇ (Str.consumeInteger(Radix, ਨਤੀਜਾ))
    ਵਾਪਸ ਸੱਚ;

  // For getAsInteger, we require the whole string to be consumed or else we
  // consider it a failure.
  ਵਾਪਸ !Str.ਖਾਲੀ_ਥਾਵਾਂ();
}

ਤਾਰਕਿਕ StringRef::getAsDouble(ਦੁੱਗਣਾ &ਨਤੀਜਾ, ਤਾਰਕਿਕ AllowInexact) ਅਚਲ {
  APFloat F(0.0);
  ਸਵੈਚਾਲਿਤ ਸਥਿਤੀ_Or_Err = F.convertFromString(*ਇਹ, APFloat::rm_ਸਭ_ਤੋਂ_ਨੇੜੇ_Ties_ਮੰਜ਼ਿਲ_Even);
  ਜੇ (error_ਮੰਜ਼ਿਲ_Bool(ਸਥਿਤੀ_Or_Err.takeError()))
    ਵਾਪਸ ਸੱਚ;

  APFloat::op_ਸਥਿਤੀ ਸਥਿਤੀ = *ਸਥਿਤੀ_Or_Err;
  ਜੇ (ਸਥਿਤੀ != APFloat::opOK) {
    ਜੇ (!AllowInexact || !(ਸਥਿਤੀ & APFloat::opInexact))
      ਵਾਪਸ ਸੱਚ;
  }

  ਨਤੀਜਾ = F.convert_ਮੰਜ਼ਿਲ_Double();
  ਵਾਪਸ ਝੂਠ;
}

// Implementation of StringRef hashing.
hash_ਕੋਡ llvm::hash_ਮੁੱਲ(StringRef S) { ਵਾਪਸ hash_combine_range(S); }

ਬਿਨਾਨਿਸ਼ਾਨੀ DenseMapInfo<StringRef, ਖਾਲੀ>::get_Hash_ਮੁੱਲ(StringRef Val) {
  ਵਾਪਸ (ਬਿਨਾਨਿਸ਼ਾਨੀ)(hash_ਮੁੱਲ(Val));
}
