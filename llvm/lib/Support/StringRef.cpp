//===-- StringRef.cpp - Lightweight String References ---------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#ಅನ್ತರ್ಭಾವ "llvm/ADT/StringRef.h"
#ಅನ್ತರ್ಭಾವ "llvm/ADT/APFloat.h"
#ಅನ್ತರ್ಭಾವ "llvm/ADT/APInt.h"
#ಅನ್ತರ್ಭಾವ "llvm/ADT/Hashing.h"
#ಅನ್ತರ್ಭಾವ "llvm/ADT/StringExtras.h"
#ಅನ್ತರ್ಭಾವ "llvm/ADT/edit_distance.h"
#ಅನ್ತರ್ಭಾವ "llvm/Support/Error.h"
#ಅನ್ತರ್ಭಾವ <bitset>

ಬಳಸಿ ನಾಮಾಕಾಶ llvm;

// strncasecmp() is not available on non-POSIX systems, so define an
// alternative function here.
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ ascii_strncasecmp(StringRef LHS, StringRef RHS) {
  ಗಾಗಿ (ಸ್ವಯಂಚಾಲಿತ [LC, RC] : zip_equal(LHS, RHS)) {
    ಚಿಹ್ನೆರಹಿತ ಅಕ್ಷರ LHC = ಗಮ್ಯ_ಅಧೋ(LC);
    ಚಿಹ್ನೆರಹಿತ ಅಕ್ಷರ RHC = ಗಮ್ಯ_ಅಧೋ(RC);
    ಆದರೆ (LHC != RHC)
      ಹಿಂತಿರುಗಿ LHC < RHC ? -1 : 1;
  }
  ಹಿಂತಿರುಗಿ 0;
}

ಪೂರ್ಣಾಂಕ StringRef::compare_insensitive(StringRef RHS) ಸ್ಥಿರಾಂಕ {
  size_t Min = std::min(ಗಾತ್ರ_3(), RHS.ಗಾತ್ರ_3());
  ಆದರೆ (ಪೂರ್ಣಾಂಕ Res = ascii_strncasecmp(take_front(Min), RHS.take_front(Min)))
    ಹಿಂತಿರುಗಿ Res;
  ಆದರೆ (ಗಾತ್ರ_3() == RHS.ಗಾತ್ರ_3())
    ಹಿಂತಿರುಗಿ 0;
  ಹಿಂತಿರುಗಿ ಗಾತ್ರ_3() < RHS.ಗಾತ್ರ_3() ? -1 : 1;
}

ತಾರ್ಕಿಕ StringRef::starts_with_insensitive(StringRef ಪೂರ್ವಪ್ರತ್ಯಯ) ಸ್ಥಿರಾಂಕ {
  ಹಿಂತಿರುಗಿ ಗಾತ್ರ_3() >= ಪೂರ್ವಪ್ರತ್ಯಯ.ಗಾತ್ರ_3() &&
         ascii_strncasecmp(take_front(ಪೂರ್ವಪ್ರತ್ಯಯ.ಗಾತ್ರ_3()), ಪೂರ್ವಪ್ರತ್ಯಯ) == 0;
}

ತಾರ್ಕಿಕ StringRef::ತುದಿಗಳು_with_insensitive(StringRef Suffix) ಸ್ಥಿರಾಂಕ {
  ಹಿಂತಿರುಗಿ ಗಾತ್ರ_3() >= Suffix.ಗಾತ್ರ_3() &&
         ascii_strncasecmp(take_ಹಿಂಭಾಗ(Suffix.ಗಾತ್ರ_3()), Suffix) == 0;
}

size_t StringRef::ಕಂಡುಹಿಡಿ_insensitive(ಅಕ್ಷರ C, size_t From) ಸ್ಥಿರಾಂಕ {
  ಅಕ್ಷರ L = ಗಮ್ಯ_ಅಧೋ(C);
  ಹಿಂತಿರುಗಿ ಕಂಡುಹಿಡಿ_if([L](ಅಕ್ಷರ D) { ಹಿಂತಿರುಗಿ ಗಮ್ಯ_ಅಧೋ(D) == L; }, From);
}

/// compare_numeric - Compare strings, handle embedded numbers.
ಪೂರ್ಣಾಂಕ StringRef::compare_numeric(StringRef RHS) ಸ್ಥಿರಾಂಕ {
  ಗಾಗಿ (size_t I = 0, E = std::min(ಗಾತ್ರ_3(), RHS.ಗಾತ್ರ_3()); I != E; ++I) {
    // Check for sequences of digits.
    ಆದರೆ (isDigit(ದತ್ತಾಂಶ()[I]) && isDigit(RHS.ದತ್ತಾಂಶ()[I])) {
      // The longer sequence of numbers is considered larger.
      // This doesn't really handle prefixed zeros well.
      size_t J;
      ಗಾಗಿ (J = I + 1; J != E + 1; ++J) {
        ತಾರ್ಕಿಕ ld = J < ಗಾತ್ರ_3() && isDigit(ದತ್ತಾಂಶ()[J]);
        ತಾರ್ಕಿಕ rd = J < RHS.ಗಾತ್ರ_3() && isDigit(RHS.ದತ್ತಾಂಶ()[J]);
        ಆದರೆ (ld != rd)
          ಹಿಂತಿರುಗಿ rd ? -1 : 1;
        ಆದರೆ (!rd)
          ನಿಲ್ಲಿಸು;
      }
      // The two number sequences have the same length (J-I), just memcmp them.
      ಆದರೆ (ಪೂರ್ಣಾಂಕ Res = compare_ಸ್ಮರಣೆ(ದತ್ತಾಂಶ() + I, RHS.ದತ್ತಾಂಶ() + I, J - I))
        ಹಿಂತಿರುಗಿ Res < 0 ? -1 : 1;
      // Identical number sequences, continue search after the numbers.
      I = J - 1;
      ಮುಂದುವರಿ;
    }
    ಆದರೆ (ದತ್ತಾಂಶ()[I] != RHS.ದತ್ತಾಂಶ()[I])
      ಹಿಂತಿರುಗಿ (ಚಿಹ್ನೆರಹಿತ ಅಕ್ಷರ)ದತ್ತಾಂಶ()[I] < (ಚಿಹ್ನೆರಹಿತ ಅಕ್ಷರ)RHS.ದತ್ತಾಂಶ()[I] ? -1 : 1;
  }
  ಆದರೆ (ಗಾತ್ರ_3() == RHS.ಗಾತ್ರ_3())
    ಹಿಂತಿರುಗಿ 0;
  ಹಿಂತಿರುಗಿ ಗಾತ್ರ_3() < RHS.ಗಾತ್ರ_3() ? -1 : 1;
}

// Compute the edit distance between the two given strings.
ಚಿಹ್ನೆರಹಿತ StringRef::edit_ದೂರ(llvm::StringRef Other,
                                  ತಾರ್ಕಿಕ AllowReplacements,
                                  ಚಿಹ್ನೆರಹಿತ Max_Edit_ದೂರ) ಸ್ಥಿರಾಂಕ {
  ಹಿಂತಿರುಗಿ llvm::Compute_Edit_ದೂರ(ArrayRef(ದತ್ತಾಂಶ(), ಗಾತ್ರ_3()),
                                   ArrayRef(Other.ದತ್ತಾಂಶ(), Other.ಗಾತ್ರ_3()),
                                   AllowReplacements, Max_Edit_ದೂರ);
}

ಚಿಹ್ನೆರಹಿತ llvm::StringRef::edit_ದೂರ_insensitive(
    StringRef Other, ತಾರ್ಕಿಕ AllowReplacements, ಚಿಹ್ನೆರಹಿತ Max_Edit_ದೂರ) ಸ್ಥಿರಾಂಕ {
  ಹಿಂತಿರುಗಿ llvm::Compute_Mapped_Edit_ದೂರ(
      ArrayRef(ದತ್ತಾಂಶ(), ಗಾತ್ರ_3()), ArrayRef(Other.ದತ್ತಾಂಶ(), Other.ಗಾತ್ರ_3()),
      llvm::ಗಮ್ಯ_ಅಧೋ, AllowReplacements, Max_Edit_ದೂರ);
}

//===----------------------------------------------------------------------===//
// String Operations
//===----------------------------------------------------------------------===//

std::string StringRef::ಅಧೋ() ಸ್ಥಿರಾಂಕ {
  ಹಿಂತಿರುಗಿ std::string(map_iterator(begin(), ಗಮ್ಯ_ಅಧೋ),
                     map_iterator(ಕೊನೆ(), ಗಮ್ಯ_ಅಧೋ));
}

std::string StringRef::ಊರ್ಧ್ವ() ಸ್ಥಿರಾಂಕ {
  ಹಿಂತಿರುಗಿ std::string(map_iterator(begin(), ಗಮ್ಯ_ಊರ್ಧ್ವ),
                     map_iterator(ಕೊನೆ(), ಗಮ್ಯ_ಊರ್ಧ್ವ));
}

//===----------------------------------------------------------------------===//
// String Searching
//===----------------------------------------------------------------------===//


/// find - Search for the first string \arg Str in the string.
///
/// \return - The index of the first occurrence of \arg Str, or npos if not
/// found.
size_t StringRef::ಕಂಡುಹಿಡಿ(StringRef Str, size_t From) ಸ್ಥಿರಾಂಕ {
  ಆದರೆ (From > ಗಾತ್ರ_3())
    ಹಿಂತಿರುಗಿ npos;

  ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *ಆರಂಭ = ದತ್ತಾಂಶ() + From;
  size_t ಗಾತ್ರ_2 = ಗಾತ್ರ_3() - From;

  ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *Needle = Str.ದತ್ತಾಂಶ();
  size_t N = Str.ಗಾತ್ರ_3();
  ಆದರೆ (N == 0)
    ಹಿಂತಿರುಗಿ From;
  ಆದರೆ (ಗಾತ್ರ_2 < N)
    ಹಿಂತಿರುಗಿ npos;
  ಆದರೆ (N == 1) {
    ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *Ptr = (ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *)::memchr(ಆರಂಭ, Needle[0], ಗಾತ್ರ_2);
    ಹಿಂತಿರುಗಿ Ptr == ಶೂನ್ಯಸೂಚಕ ? npos : Ptr - ದತ್ತಾಂಶ();
  }

  ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *Stop = ಆರಂಭ + (ಗಾತ್ರ_2 - N + 1);

  ಆದರೆ (N == 2) {
    // Provide a fast path for newline finding (CRLF case) in InclusionRewriter.
    // Not the most optimized strategy, but getting memcmp inlined should be
    // good enough.
    ಮಾಡು {
      ಆದರೆ (std::memcmp(ಆರಂಭ, Needle, 2) == 0)
        ಹಿಂತಿರುಗಿ ಆರಂಭ - ದತ್ತಾಂಶ();
      ++ಆರಂಭ;
    } ಆಗ (ಆರಂಭ < Stop);
    ಹಿಂತಿರುಗಿ npos;
  }

  // For short haystacks or unsupported needles fall back to the naive algorithm
  ಆದರೆ (ಗಾತ್ರ_2 < 16 || N > 255) {
    ಮಾಡು {
      ಆದರೆ (std::memcmp(ಆರಂಭ, Needle, N) == 0)
        ಹಿಂತಿರುಗಿ ಆರಂಭ - ದತ್ತಾಂಶ();
      ++ಆರಂಭ;
    } ಆಗ (ಆರಂಭ < Stop);
    ಹಿಂತಿರುಗಿ npos;
  }

  // Build the bad char heuristic table, with uint8_t to reduce cache thrashing.
  uint8_t BadCharSkip[256];
  std::memset(BadCharSkip, N, 256);
  ಗಾಗಿ (ಚಿಹ್ನೆರಹಿತ i = 0; i != N-1; ++i)
    BadCharSkip[(uint8_t)Str[i]] = N-1-i;

  ಮಾಡು {
    uint8_t ಕೊನೆಯ = ಆರಂಭ[N - 1];
    ಆದರೆ (LLVM_UNLIKELY(ಕೊನೆಯ == (uint8_t)Needle[N - 1]))
      ಆದರೆ (std::memcmp(ಆರಂಭ, Needle, N - 1) == 0)
        ಹಿಂತಿರುಗಿ ಆರಂಭ - ದತ್ತಾಂಶ();

    // Otherwise skip the appropriate number of bytes.
    ಆರಂಭ += BadCharSkip[ಕೊನೆಯ];
  } ಆಗ (ಆರಂಭ < Stop);

  ಹಿಂತಿರುಗಿ npos;
}

size_t StringRef::ಕಂಡುಹಿಡಿ_insensitive(StringRef Str, size_t From) ಸ್ಥಿರಾಂಕ {
  StringRef This = substr(From);
  ಆಗ (This.ಗಾತ್ರ_3() >= Str.ಗಾತ್ರ_3()) {
    ಆದರೆ (This.starts_with_insensitive(Str))
      ಹಿಂತಿರುಗಿ From;
    This = This.drop_front();
    ++From;
  }
  ಹಿಂತಿರುಗಿ npos;
}

size_t StringRef::rfind_insensitive(ಅಕ್ಷರ C, size_t From) ಸ್ಥಿರಾಂಕ {
  From = std::min(From, ಗಾತ್ರ_3());
  size_t i = From;
  ಆಗ (i != 0) {
    --i;
    ಆದರೆ (ಗಮ್ಯ_ಅಧೋ(ದತ್ತಾಂಶ()[i]) == ಗಮ್ಯ_ಅಧೋ(C))
      ಹಿಂತಿರುಗಿ i;
  }
  ಹಿಂತಿರುಗಿ npos;
}

/// rfind - Search for the last string \arg Str in the string.
///
/// \return - The index of the last occurrence of \arg Str, or npos if not
/// found.
size_t StringRef::rfind(StringRef Str) ಸ್ಥಿರಾಂಕ {
  ಹಿಂತಿರುಗಿ std::string_view(*ಇದು).rfind(Str);
}

size_t StringRef::rfind_insensitive(StringRef Str) ಸ್ಥಿರಾಂಕ {
  size_t N = Str.ಗಾತ್ರ_3();
  ಆದರೆ (N > ಗಾತ್ರ_3())
    ಹಿಂತಿರುಗಿ npos;
  ಗಾಗಿ (size_t i = ಗಾತ್ರ_3() - N + 1, e = 0; i != e;) {
    --i;
    ಆದರೆ (substr(i, N).equals_insensitive(Str))
      ಹಿಂತಿರುಗಿ i;
  }
  ಹಿಂತಿರುಗಿ npos;
}

/// find_first_of - Find the first character in the string that is in \arg
/// Chars, or npos if not found.
///
/// Note: O(size() + Chars.size())
StringRef::ಗಾತ್ರ_type StringRef::ಕಂಡುಹಿಡಿ_ಮೊದಲ_ದ(StringRef Chars,
                                              size_t From) ಸ್ಥಿರಾಂಕ {
  std::bitset<1 << CHAR_BIT> Char_ಬಿಟ್ಟುಗಳು;
  ಗಾಗಿ (ಅಕ್ಷರ C : Chars)
    Char_ಬಿಟ್ಟುಗಳು.ಗಣ((ಚಿಹ್ನೆರಹಿತ ಅಕ್ಷರ)C);

  ಗಾಗಿ (ಗಾತ್ರ_type i = std::min(From, ಗಾತ್ರ_3()), e = ಗಾತ್ರ_3(); i != e; ++i)
    ಆದರೆ (Char_ಬಿಟ್ಟುಗಳು.test((ಚಿಹ್ನೆರಹಿತ ಅಕ್ಷರ)ದತ್ತಾಂಶ()[i]))
      ಹಿಂತಿರುಗಿ i;
  ಹಿಂತಿರುಗಿ npos;
}

/// find_first_not_of - Find the first character in the string that is not
/// \arg C or npos if not found.
StringRef::ಗಾತ್ರ_type StringRef::ಕಂಡುಹಿಡಿ_ಮೊದಲ_not_ದ(ಅಕ್ಷರ C, size_t From) ಸ್ಥಿರಾಂಕ {
  ಹಿಂತಿರುಗಿ std::string_view(*ಇದು).ಕಂಡುಹಿಡಿ_ಮೊದಲ_not_ದ(C, From);
}

/// find_first_not_of - Find the first character in the string that is not
/// in the string \arg Chars, or npos if not found.
///
/// Note: O(size() + Chars.size())
StringRef::ಗಾತ್ರ_type StringRef::ಕಂಡುಹಿಡಿ_ಮೊದಲ_not_ದ(StringRef Chars,
                                                  size_t From) ಸ್ಥಿರಾಂಕ {
  std::bitset<1 << CHAR_BIT> Char_ಬಿಟ್ಟುಗಳು;
  ಗಾಗಿ (ಅಕ್ಷರ C : Chars)
    Char_ಬಿಟ್ಟುಗಳು.ಗಣ((ಚಿಹ್ನೆರಹಿತ ಅಕ್ಷರ)C);

  ಗಾಗಿ (ಗಾತ್ರ_type i = std::min(From, ಗಾತ್ರ_3()), e = ಗಾತ್ರ_3(); i != e; ++i)
    ಆದರೆ (!Char_ಬಿಟ್ಟುಗಳು.test((ಚಿಹ್ನೆರಹಿತ ಅಕ್ಷರ)ದತ್ತಾಂಶ()[i]))
      ಹಿಂತಿರುಗಿ i;
  ಹಿಂತಿರುಗಿ npos;
}

/// find_last_of - Find the last character in the string that is in \arg C,
/// or npos if not found.
///
/// Note: O(size() + Chars.size())
StringRef::ಗಾತ್ರ_type StringRef::ಕಂಡುಹಿಡಿ_ಕೊನೆಯ_ದ(StringRef Chars,
                                             size_t From) ಸ್ಥಿರಾಂಕ {
  std::bitset<1 << CHAR_BIT> Char_ಬಿಟ್ಟುಗಳು;
  ಗಾಗಿ (ಅಕ್ಷರ C : Chars)
    Char_ಬಿಟ್ಟುಗಳು.ಗಣ((ಚಿಹ್ನೆರಹಿತ ಅಕ್ಷರ)C);

  ಗಾಗಿ (ಗಾತ್ರ_type i = std::min(From, ಗಾತ್ರ_3()) - 1, e = -1; i != e; --i)
    ಆದರೆ (Char_ಬಿಟ್ಟುಗಳು.test((ಚಿಹ್ನೆರಹಿತ ಅಕ್ಷರ)ದತ್ತಾಂಶ()[i]))
      ಹಿಂತಿರುಗಿ i;
  ಹಿಂತಿರುಗಿ npos;
}

/// find_last_not_of - Find the last character in the string that is not
/// \arg C, or npos if not found.
StringRef::ಗಾತ್ರ_type StringRef::ಕಂಡುಹಿಡಿ_ಕೊನೆಯ_not_ದ(ಅಕ್ಷರ C, size_t From) ಸ್ಥಿರಾಂಕ {
  ಗಾಗಿ (ಗಾತ್ರ_type i = std::min(From, ಗಾತ್ರ_3()) - 1, e = -1; i != e; --i)
    ಆದರೆ (ದತ್ತಾಂಶ()[i] != C)
      ಹಿಂತಿರುಗಿ i;
  ಹಿಂತಿರುಗಿ npos;
}

/// find_last_not_of - Find the last character in the string that is not in
/// \arg Chars, or npos if not found.
///
/// Note: O(size() + Chars.size())
StringRef::ಗಾತ್ರ_type StringRef::ಕಂಡುಹಿಡಿ_ಕೊನೆಯ_not_ದ(StringRef Chars,
                                                 size_t From) ಸ್ಥಿರಾಂಕ {
  std::bitset<1 << CHAR_BIT> Char_ಬಿಟ್ಟುಗಳು;
  ಗಾಗಿ (ಅಕ್ಷರ C : Chars)
    Char_ಬಿಟ್ಟುಗಳು.ಗಣ((ಚಿಹ್ನೆರಹಿತ ಅಕ್ಷರ)C);

  ಗಾಗಿ (ಗಾತ್ರ_type i = std::min(From, ಗಾತ್ರ_3()) - 1, e = -1; i != e; --i)
    ಆದರೆ (!Char_ಬಿಟ್ಟುಗಳು.test((ಚಿಹ್ನೆರಹಿತ ಅಕ್ಷರ)ದತ್ತಾಂಶ()[i]))
      ಹಿಂತಿರುಗಿ i;
  ಹಿಂತಿರುಗಿ npos;
}

ಶೂನ್ಯ StringRef::ವಿಭಜಿಸು(Small_ಸದಿಶ_Impl<StringRef> &A,
                      StringRef Separator, ಪೂರ್ಣಾಂಕ Max_ವಿಭಜಿಸು,
                      ತಾರ್ಕಿಕ Keep_ಖಾಲಿಗಳು) ಸ್ಥಿರಾಂಕ {
  StringRef S = *ಇದು;

  // Count down from MaxSplit. When MaxSplit is -1, this will just split
  // "forever". This doesn't support splitting more than 2^31 times
  // intentionally; if we ever want that we can make MaxSplit a 64-bit integer
  // but that seems unlikely to be useful.
  ಆಗ (Max_ವಿಭಜಿಸು-- != 0) {
    size_t Idx = S.ಕಂಡುಹಿಡಿ(Separator);
    ಆದರೆ (Idx == npos)
      ನಿಲ್ಲಿಸು;

    // Push this split.
    ಆದರೆ (Keep_ಖಾಲಿಗಳು || Idx > 0)
      A.ತಳ್ಳು_ಹಿಂಭಾಗ(S.ತುಂಡು(0, Idx));

    // Jump forward.
    S = S.substr(Idx + Separator.ಗಾತ್ರ_3());
  }

  // Push the tail.
  ಆದರೆ (Keep_ಖಾಲಿಗಳು || !S.ಖಾಲಿಗಳು())
    A.ತಳ್ಳು_ಹಿಂಭಾಗ(S);
}

ಶೂನ್ಯ StringRef::ವಿಭಜಿಸು(Small_ಸದಿಶ_Impl<StringRef> &A, ಅಕ್ಷರ Separator,
                      ಪೂರ್ಣಾಂಕ Max_ವಿಭಜಿಸು, ತಾರ್ಕಿಕ Keep_ಖಾಲಿಗಳು) ಸ್ಥಿರಾಂಕ {
  StringRef S = *ಇದು;

  // Count down from MaxSplit. When MaxSplit is -1, this will just split
  // "forever". This doesn't support splitting more than 2^31 times
  // intentionally; if we ever want that we can make MaxSplit a 64-bit integer
  // but that seems unlikely to be useful.
  ಆಗ (Max_ವಿಭಜಿಸು-- != 0) {
    size_t Idx = S.ಕಂಡುಹಿಡಿ(Separator);
    ಆದರೆ (Idx == npos)
      ನಿಲ್ಲಿಸು;

    // Push this split.
    ಆದರೆ (Keep_ಖಾಲಿಗಳು || Idx > 0)
      A.ತಳ್ಳು_ಹಿಂಭಾಗ(S.ತುಂಡು(0, Idx));

    // Jump forward.
    S = S.substr(Idx + 1);
  }

  // Push the tail.
  ಆದರೆ (Keep_ಖಾಲಿಗಳು || !S.ಖಾಲಿಗಳು())
    A.ತಳ್ಳು_ಹಿಂಭಾಗ(S);
}

//===----------------------------------------------------------------------===//
// Helpful Algorithms
//===----------------------------------------------------------------------===//

/// count - Return the number of non-overlapped occurrences of \arg Str in
/// the string.
size_t StringRef::ಎಣಿಕೆ_2(StringRef Str) ಸ್ಥಿರಾಂಕ {
  size_t ಎಣಿಕೆ_3 = 0;
  size_t Pos = 0;
  size_t N = Str.ಗಾತ್ರ_3();
  // TODO: For an empty `Str` we return 0 for legacy reasons. Consider changing
  //       this to `Length + 1` which is more in-line with the function
  //       description.
  ಆದರೆ (!N)
    ಹಿಂತಿರುಗಿ 0;
  ಆಗ ((Pos = ಕಂಡುಹಿಡಿ(Str, Pos)) != npos) {
    ++ಎಣಿಕೆ_3;
    Pos += N;
  }
  ಹಿಂತಿರುಗಿ ಎಣಿಕೆ_3;
}

ಚಿಹ್ನೆರಹಿತ llvm::getAutoSenseRadix(StringRef &Str) {
  ಆದರೆ (Str.ಖಾಲಿಗಳು())
    ಹಿಂತಿರುಗಿ 10;

  ಆದರೆ (Str.consume_front_insensitive("0x"))
    ಹಿಂತಿರುಗಿ 16;

  ಆದರೆ (Str.consume_front_insensitive("0b"))
    ಹಿಂತಿರುಗಿ 2;

  ಆದರೆ (Str.consume_front("0o"))
    ಹಿಂತಿರುಗಿ 8;

  ಆದರೆ (Str[0] == '0' && Str.ಗಾತ್ರ_3() > 1 && isDigit(Str[1])) {
    Str = Str.substr(1);
    ಹಿಂತಿರುಗಿ 8;
  }

  ಹಿಂತಿರುಗಿ 10;
}

ತಾರ್ಕಿಕ llvm::consumeUnsignedInteger(StringRef &Str, ಚಿಹ್ನೆರಹಿತ Radix,
                                  ಚಿಹ್ನೆರಹಿತ ದೀರ್ಘ ದೀರ್ಘ &ಫಲಿತಾಂಶ) {
  // Autosense radix if not specified.
  ಆದರೆ (Radix == 0)
    Radix = getAutoSenseRadix(Str);

  // Empty strings (after the radix autosense) are invalid.
  ಆದರೆ (Str.ಖಾಲಿಗಳು()) ಹಿಂತಿರುಗಿ ನಿಜ;

  // Parse all the bytes of the string given this radix.  Watch for overflow.
  StringRef Str2 = Str;
  ಫಲಿತಾಂಶ = 0;
  ಆಗ (!Str2.ಖಾಲಿಗಳು()) {
    ಚಿಹ್ನೆರಹಿತ CharVal;
    ಆದರೆ (Str2[0] >= '0' && Str2[0] <= '9')
      CharVal = Str2[0] - '0';
    ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (Str2[0] >= 'a' && Str2[0] <= 'z')
      CharVal = Str2[0] - 'a' + 10;
    ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (Str2[0] >= 'A' && Str2[0] <= 'Z')
      CharVal = Str2[0] - 'A' + 10;
    ಇಲ್ಲದಿದ್ದರೆ
      ನಿಲ್ಲಿಸು;

    // If the parsed value is larger than the integer radix, we cannot
    // consume any more characters.
    ಆದರೆ (CharVal >= Radix)
      ನಿಲ್ಲಿಸು;

    // Add in this character.
    ಚಿಹ್ನೆರಹಿತ ದೀರ್ಘ ದೀರ್ಘ Prev_ಫಲಿತಾಂಶ = ಫಲಿತಾಂಶ;
    ಫಲಿತಾಂಶ = ಫಲಿತಾಂಶ * Radix + CharVal;

    // Check for overflow by shifting back and seeing if bits were lost.
    ಆದರೆ (ಫಲಿತಾಂಶ / Radix < Prev_ಫಲಿತಾಂಶ)
      ಹಿಂತಿರುಗಿ ನಿಜ;

    Str2 = Str2.substr(1);
  }

  // We consider the operation a failure if no characters were consumed
  // successfully.
  ಆದರೆ (Str.ಗಾತ್ರ_3() == Str2.ಗಾತ್ರ_3())
    ಹಿಂತಿರುಗಿ ನಿಜ;

  Str = Str2;
  ಹಿಂತಿರುಗಿ ಸುಳ್ಳು;
}

ತಾರ್ಕಿಕ llvm::consumeSignedInteger(StringRef &Str, ಚಿಹ್ನೆರಹಿತ Radix,
                                ದೀರ್ಘ ದೀರ್ಘ &ಫಲಿತಾಂಶ) {
  ಚಿಹ್ನೆರಹಿತ ದೀರ್ಘ ದೀರ್ಘ ULLVal;

  // Handle positive strings first.
  ಆದರೆ (!Str.starts_with("-")) {
    ಆದರೆ (consumeUnsignedInteger(Str, Radix, ULLVal) ||
        // Check for value so large it overflows a signed value.
        (ದೀರ್ಘ ದೀರ್ಘ)ULLVal < 0)
      ಹಿಂತಿರುಗಿ ನಿಜ;
    ಫಲಿತಾಂಶ = ULLVal;
    ಹಿಂತಿರುಗಿ ಸುಳ್ಳು;
  }

  // Get the positive part of the value.
  StringRef Str2 = Str.drop_front(1);
  ಆದರೆ (consumeUnsignedInteger(Str2, Radix, ULLVal) ||
      // Reject values so large they'd overflow as negative signed, but allow
      // "-0".  This negates the unsigned so that the negative isn't undefined
      // on signed overflow.
      (ದೀರ್ಘ ದೀರ್ಘ)-ULLVal > 0)
    ಹಿಂತಿರುಗಿ ನಿಜ;

  Str = Str2;
  ಫಲಿತಾಂಶ = -ULLVal;
  ಹಿಂತಿರುಗಿ ಸುಳ್ಳು;
}

/// GetAsUnsignedInteger - Workhorse method that converts a integer character
/// sequence of radix up to 36 to an unsigned long long value.
ತಾರ್ಕಿಕ llvm::getAsUnsignedInteger(StringRef Str, ಚಿಹ್ನೆರಹಿತ Radix,
                                ಚಿಹ್ನೆರಹಿತ ದೀರ್ಘ ದೀರ್ಘ &ಫಲಿತಾಂಶ) {
  ಆದರೆ (consumeUnsignedInteger(Str, Radix, ಫಲಿತಾಂಶ))
    ಹಿಂತಿರುಗಿ ನಿಜ;

  // For getAsUnsignedInteger, we require the whole string to be consumed or
  // else we consider it a failure.
  ಹಿಂತಿರುಗಿ !Str.ಖಾಲಿಗಳು();
}

ತಾರ್ಕಿಕ llvm::getAsSignedInteger(StringRef Str, ಚಿಹ್ನೆರಹಿತ Radix,
                              ದೀರ್ಘ ದೀರ್ಘ &ಫಲಿತಾಂಶ) {
  ಆದರೆ (consumeSignedInteger(Str, Radix, ಫಲಿತಾಂಶ))
    ಹಿಂತಿರುಗಿ ನಿಜ;

  // For getAsSignedInteger, we require the whole string to be consumed or else
  // we consider it a failure.
  ಹಿಂತಿರುಗಿ !Str.ಖಾಲಿಗಳು();
}

ತಾರ್ಕಿಕ StringRef::consumeInteger(ಚಿಹ್ನೆರಹಿತ Radix, APInt &ಫಲಿತಾಂಶ) {
  StringRef Str = *ಇದು;

  // Autosense radix if not specified.
  ಆದರೆ (Radix == 0)
    Radix = getAutoSenseRadix(Str);

  ಪ್ರತಿಜ್ಞಾ(Radix > 1 && Radix <= 36);

  // Empty strings (after the radix autosense) are invalid.
  ಆದರೆ (Str.ಖಾಲಿಗಳು()) ಹಿಂತಿರುಗಿ ನಿಜ;

  // Skip leading zeroes.  This can be a significant improvement if
  // it means we don't need > 64 bits.
  Str = Str.ltrim('0');

  // If it was nothing but zeroes....
  ಆದರೆ (Str.ಖಾಲಿಗಳು()) {
    ಫಲಿತಾಂಶ = APInt(64, 0);
    *ಇದು = Str;
    ಹಿಂತಿರುಗಿ ಸುಳ್ಳು;
  }

  ಚಿಹ್ನೆರಹಿತ Log2Radix = 0;
  ಆಗ ((1U << Log2Radix) < Radix) Log2Radix++;
  ತಾರ್ಕಿಕ IsPowerOf2Radix = ((1U << Log2Radix) == Radix);

  // Initialize Result to a reasonable starting width (at least 64 bits),
  // but do not shrink it if it was already larger.
  ಚಿಹ್ನೆರಹಿತ ಬಿಟ್_ಅಗಲ_2 = std::max(64U, ಫಲಿತಾಂಶ.get_ಬಿಟ್_ಅಗಲ());

  ಆದರೆ (ಫಲಿತಾಂಶ.get_ಬಿಟ್_ಅಗಲ() < ಬಿಟ್_ಅಗಲ_2) {
    ಫಲಿತಾಂಶ = ಫಲಿತಾಂಶ.zext(ಬಿಟ್_ಅಗಲ_2);
  }

  APInt RadixAP, CharAP; // unused unless !IsPowerOf2Radix
  ಆದರೆ (!IsPowerOf2Radix) {
    // These must have the same bit-width as Result.
    RadixAP = APInt(ಬಿಟ್_ಅಗಲ_2, Radix);
    CharAP = APInt(ಬಿಟ್_ಅಗಲ_2, 0);
  }

  // Parse all the bytes of the string given this radix.
  ಫಲಿತಾಂಶ = 0;
  ಆಗ (!Str.ಖಾಲಿಗಳು()) {
    ಚಿಹ್ನೆರಹಿತ CharVal;
    ಆದರೆ (Str[0] >= '0' && Str[0] <= '9')
      CharVal = Str[0]-'0';
    ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (Str[0] >= 'a' && Str[0] <= 'z')
      CharVal = Str[0]-'a'+10;
    ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (Str[0] >= 'A' && Str[0] <= 'Z')
      CharVal = Str[0]-'A'+10;
    ಇಲ್ಲದಿದ್ದರೆ
      ನಿಲ್ಲಿಸು;

    // If the parsed value is larger than the integer radix, the string is
    // invalid.
    ಆದರೆ (CharVal >= Radix)
      ನಿಲ್ಲಿಸು;

    // Check if one more digit will overflow, and grow if so.
    ಆದರೆ (ಫಲಿತಾಂಶ.get_ಸಕ್ರಿಯ_ಬಿಟ್ಟುಗಳು() + Log2Radix > ಫಲಿತಾಂಶ.get_ಬಿಟ್_ಅಗಲ()) {
      ಚಿಹ್ನೆರಹಿತ New_ಅಗಲ = ಫಲಿತಾಂಶ.get_ಬಿಟ್_ಅಗಲ() * 2;

      // Guard against overflow of the NewWidth itself
      ಆದರೆ (New_ಅಗಲ < ಫಲಿತಾಂಶ.get_ಬಿಟ್_ಅಗಲ())
        ಹಿಂತಿರುಗಿ ನಿಜ;

      ಫಲಿತಾಂಶ = ಫಲಿತಾಂಶ.zext(New_ಅಗಲ);
      ಆದರೆ (!IsPowerOf2Radix) {
        RadixAP = RadixAP.zext(New_ಅಗಲ);
        CharAP = CharAP.zext(New_ಅಗಲ);
      }
    }

    // Add in this character.
    ಆದರೆ (IsPowerOf2Radix) {
      ಫಲಿತಾಂಶ <<= Log2Radix;
      ಫಲಿತಾಂಶ |= CharVal;
    } ಇಲ್ಲದಿದ್ದರೆ {
      ಫಲಿತಾಂಶ *= RadixAP;
      CharAP = CharVal;
      ಫಲಿತಾಂಶ += CharAP;
    }

    Str = Str.substr(1);
  }

  // We consider the operation a failure if no characters were consumed
  // successfully.
  ಆದರೆ (ಗಾತ್ರ_3() == Str.ಗಾತ್ರ_3())
    ಹಿಂತಿರುಗಿ ನಿಜ;

  *ಇದು = Str;
  ಹಿಂತಿರುಗಿ ಸುಳ್ಳು;
}

ತಾರ್ಕಿಕ StringRef::getAsInteger(ಚಿಹ್ನೆರಹಿತ Radix, APInt &ಫಲಿತಾಂಶ) ಸ್ಥಿರಾಂಕ {
  StringRef Str = *ಇದು;
  ಆದರೆ (Str.consumeInteger(Radix, ಫಲಿತಾಂಶ))
    ಹಿಂತಿರುಗಿ ನಿಜ;

  // For getAsInteger, we require the whole string to be consumed or else we
  // consider it a failure.
  ಹಿಂತಿರುಗಿ !Str.ಖಾಲಿಗಳು();
}

ತಾರ್ಕಿಕ StringRef::getAsDouble(ದ್ವಿಗುಣ &ಫಲಿತಾಂಶ, ತಾರ್ಕಿಕ AllowInexact) ಸ್ಥಿರಾಂಕ {
  APFloat F(0.0);
  ಸ್ವಯಂಚಾಲಿತ ಸ್ಥಿತಿ_Or_Err = F.convertFromString(*ಇದು, APFloat::rm_ಹತ್ತಿರದ_Ties_ಗಮ್ಯ_Even);
  ಆದರೆ (error_ಗಮ್ಯ_Bool(ಸ್ಥಿತಿ_Or_Err.takeError()))
    ಹಿಂತಿರುಗಿ ನಿಜ;

  APFloat::op_ಸ್ಥಿತಿ ಸ್ಥಿತಿ_2 = *ಸ್ಥಿತಿ_Or_Err;
  ಆದರೆ (ಸ್ಥಿತಿ_2 != APFloat::opOK) {
    ಆದರೆ (!AllowInexact || !(ಸ್ಥಿತಿ_2 & APFloat::opInexact))
      ಹಿಂತಿರುಗಿ ನಿಜ;
  }

  ಫಲಿತಾಂಶ = F.convert_ಗಮ್ಯ_Double();
  ಹಿಂತಿರುಗಿ ಸುಳ್ಳು;
}

// Implementation of StringRef hashing.
hash_ಸಂಕೇತ llvm::hash_ಮೌಲ್ಯ(StringRef S) { ಹಿಂತಿರುಗಿ hash_combine_range(S); }

ಚಿಹ್ನೆರಹಿತ DenseMapInfo<StringRef, ಶೂನ್ಯ>::get_Hash_ಮೌಲ್ಯ(StringRef Val) {
  ಹಿಂತಿರುಗಿ (ಚಿಹ್ನೆರಹಿತ)(hash_ಮೌಲ್ಯ(Val));
}
