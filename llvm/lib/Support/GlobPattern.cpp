//===-- GlobPattern.cpp - Glob pattern matcher implementation -------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
// This file implements a glob pattern matcher.
//
//===----------------------------------------------------------------------===//

#ಅನ್ತರ್ಭಾವ "llvm/Support/GlobPattern.h"
#ಅನ್ತರ್ಭಾವ "llvm/ADT/StringRef.h"
#ಅನ್ತರ್ಭಾವ "llvm/Support/Errc.h"

ಬಳಸಿ ನಾಮಾಕಾಶ llvm;

ಸ್ಥಿರ constexpr ಅಕ್ಷರ ಪೂರ್ವಪ್ರತ್ಯಯ_Metacharacters[] = "?*[{\\";
ಸ್ಥಿರ constexpr ಅಕ್ಷರ SuffixMetacharacters[] = "?*[]{}\\";
ಸ್ಥಿರ constexpr ಅಕ್ಷರ ಪೂರ್ವಪ್ರತ್ಯಯ_Metacharacters_With_Slash[] = "?*[{\\/";
ಸ್ಥಿರ constexpr ಅಕ್ಷರ SuffixMetacharactersWithSlash[] = "?*[]{}\\/";

// Expands character ranges and returns a bitmap.
// For example, "a-cf-hz" is expanded to "abcfghz".
ಸ್ಥಿರ Expected<ಬಿಟ್_ಸದಿಶ> expand(StringRef S, StringRef Original) {
  ಬಿಟ್_ಸದಿಶ BV(256, ಸುಳ್ಳು);

  // Expand X-Y.
  ಗಾಗಿ (;;) {
    ಆದರೆ (S.ಗಾತ್ರ_3() < 3)
      ನಿಲ್ಲಿಸು;

    uint8_t ಆರಂಭ = S[0];
    uint8_t ಕೊನೆ_2 = S[2];

    // If it doesn't start with something like X-Y,
    // consume the first character and proceed.
    ಆದರೆ (S[1] != '-') {
      BV[ಆರಂಭ] = ನಿಜ;
      S = S.substr(1);
      ಮುಂದುವರಿ;
    }

    // It must be in the form of X-Y.
    // Validate it and then interpret the range.
    ಆದರೆ (ಆರಂಭ > ಕೊನೆ_2)
      ಹಿಂತಿರುಗಿ make_error<StringError>("invalid glob pattern: " + Original,
                                     errc::invalid_ವಾದ);

    ಗಾಗಿ (ಪೂರ್ಣಾಂಕ C = ಆರಂಭ; C <= ಕೊನೆ_2; ++C)
      BV[(uint8_t)C] = ನಿಜ;
    S = S.substr(3);
  }

  ಗಾಗಿ (ಅಕ್ಷರ C : S)
    BV[(uint8_t)C] = ನಿಜ;
  ಹಿಂತಿರುಗಿ BV;
}

// Identify brace expansions in S and return the list of patterns they expand
// into.
ಸ್ಥಿರ Expected<Small_ಸದಿಶ<std::string, 1>>
parseBraceExpansions(StringRef S, std::optional<size_t> MaxSubPatterns) {
  Small_ಸದಿಶ<std::string> SubPatterns = {S.str()};
  ಆದರೆ (!MaxSubPatterns || !S.ಒಳಗೊಂಡಿದೆ('{'))
    ಹಿಂತಿರುಗಿ std::move(SubPatterns);

  ರಚನೆ BraceExpansion {
    size_t ಆರಂಭ;
    size_t ಉದ್ದಳತೆ;
    Small_ಸದಿಶ<StringRef, 2> Terms;
  };
  Small_ಸದಿಶ<BraceExpansion, 0> BraceExpansions;

  BraceExpansion *CurrentBE = ಶೂನ್ಯಸೂಚಕ;
  size_t ಪದ_Begin;
  ಗಾಗಿ (size_t I = 0, E = S.ಗಾತ್ರ_3(); I != E; ++I) {
    ಆದರೆ (S[I] == '[') {
      I = S.ಕಂಡುಹಿಡಿ(']', I + 2);
      ಆದರೆ (I == std::string::npos)
        ಹಿಂತಿರುಗಿ make_error<StringError>("invalid glob pattern, unmatched '['",
                                       errc::invalid_ವಾದ);
    } ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (S[I] == '{') {
      ಆದರೆ (CurrentBE)
        ಹಿಂತಿರುಗಿ make_error<StringError>(
            "nested brace expansions are not supported",
            errc::invalid_ವಾದ);
      CurrentBE = &BraceExpansions.emplace_ಹಿಂಭಾಗ();
      CurrentBE->ಆರಂಭ = I;
      ಪದ_Begin = I + 1;
    } ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (S[I] == ',') {
      ಆದರೆ (!CurrentBE)
        ಮುಂದುವರಿ;
      CurrentBE->Terms.ತಳ್ಳು_ಹಿಂಭಾಗ(S.substr(ಪದ_Begin, I - ಪದ_Begin));
      ಪದ_Begin = I + 1;
    } ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (S[I] == '}') {
      ಆದರೆ (!CurrentBE)
        ಮುಂದುವರಿ;
      ಆದರೆ (CurrentBE->Terms.ಖಾಲಿಗಳು())
        ಹಿಂತಿರುಗಿ make_error<StringError>(
            "empty or singleton brace expansions are not supported",
            errc::invalid_ವಾದ);
      CurrentBE->Terms.ತಳ್ಳು_ಹಿಂಭಾಗ(S.substr(ಪದ_Begin, I - ಪದ_Begin));
      CurrentBE->ಉದ್ದಳತೆ = I - CurrentBE->ಆರಂಭ + 1;
      CurrentBE = ಶೂನ್ಯಸೂಚಕ;
    } ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (S[I] == '\\') {
      ಆದರೆ (++I == E)
        ಹಿಂತಿರುಗಿ make_error<StringError>("invalid glob pattern, stray '\\'",
                                       errc::invalid_ವಾದ);
    }
  }
  ಆದರೆ (CurrentBE)
    ಹಿಂತಿರುಗಿ make_error<StringError>("incomplete brace expansion",
                                   errc::invalid_ವಾದ);

  size_t ಅಂಕೆ_Sub_Patterns = 1;
  ಗಾಗಿ (ಸ್ವಯಂಚಾಲಿತ &BE : BraceExpansions) {
    ಆದರೆ (ಅಂಕೆ_Sub_Patterns > std::numeric_limits<size_t>::max() / BE.Terms.ಗಾತ್ರ_3()) {
      ಅಂಕೆ_Sub_Patterns = std::numeric_limits<size_t>::max();
      ನಿಲ್ಲಿಸು;
    }
    ಅಂಕೆ_Sub_Patterns *= BE.Terms.ಗಾತ್ರ_3();
  }
  ಆದರೆ (ಅಂಕೆ_Sub_Patterns > *MaxSubPatterns)
    ಹಿಂತಿರುಗಿ make_error<StringError>("too many brace expansions",
                                   errc::invalid_ವಾದ);
  // Replace brace expansions in reverse order so that we don't invalidate
  // earlier start indices
  ಗಾಗಿ (ಸ್ವಯಂಚಾಲಿತ &BE : ಹಿಮ್ಮುಖ(BraceExpansions)) {
    Small_ಸದಿಶ<std::string> OrigSubPatterns;
    std::ಅದಲುಬದಲು(SubPatterns, OrigSubPatterns);
    ಗಾಗಿ (StringRef ಪದ : BE.Terms)
      ಗಾಗಿ (StringRef Orig : OrigSubPatterns)
        SubPatterns.emplace_ಹಿಂಭಾಗ(Orig).replace(BE.ಆರಂಭ, BE.ಉದ್ದಳತೆ, ಪದ);
  }
  ಹಿಂತಿರುಗಿ std::move(SubPatterns);
}

ಸ್ಥಿರ StringRef maxPlainSubstring(StringRef S, ತಾರ್ಕಿಕ SlashAgnostic) {
  ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *Metas =
      SlashAgnostic ? ಪೂರ್ವಪ್ರತ್ಯಯ_Metacharacters_With_Slash : ಪೂರ್ವಪ್ರತ್ಯಯ_Metacharacters;
  StringRef ಉತ್ತಮ;
  ಆಗ (!S.ಖಾಲಿಗಳು()) {
    size_t ಪೂರ್ವಪ್ರತ್ಯಯ_ಗಾತ್ರ = S.ಕಂಡುಹಿಡಿ_ಮೊದಲ_ದ(Metas);
    ಆದರೆ (ಪೂರ್ವಪ್ರತ್ಯಯ_ಗಾತ್ರ == std::string::npos)
      ಪೂರ್ವಪ್ರತ್ಯಯ_ಗಾತ್ರ = S.ಗಾತ್ರ_3();

    ಆದರೆ (ಉತ್ತಮ.ಗಾತ್ರ_3() < ಪೂರ್ವಪ್ರತ್ಯಯ_ಗಾತ್ರ)
      ಉತ್ತಮ = S.take_front(ಪೂರ್ವಪ್ರತ್ಯಯ_ಗಾತ್ರ);

    S = S.drop_front(ಪೂರ್ವಪ್ರತ್ಯಯ_ಗಾತ್ರ);

    // It's impossible, as the first and last characters of the input string
    // must be Glob special characters, otherwise they would be parts of
    // the prefix or the suffix.
    ಪ್ರತಿಜ್ಞಾ(!S.ಖಾಲಿಗಳು());

    ಆಯ್ಕೆ (S.front()) {
    ಸ್ಥಿತಿ '\\':
      S = S.drop_front(2);
      ನಿಲ್ಲಿಸು;
    ಸ್ಥಿತಿ '[': {
      // Drop '[' and the first character which can be ']'.
      S = S.drop_front(2);
      size_t ಕೊನೆ_Bracket = S.ಕಂಡುಹಿಡಿ_ಮೊದಲ_ದ("]");
      // Should not be possible, SubGlobPattern::create should fail on invalid
      // pattern before we get here.
      ಪ್ರತಿಜ್ಞಾ(ಕೊನೆ_Bracket != std::string::npos);
      S = S.drop_front(ಕೊನೆ_Bracket + 1);
      ನಿಲ್ಲಿಸು;
    }
    ಸ್ಥಿತಿ '{':
      // TODO: implement.
      // Fallback to whatever is best for now.
      ಹಿಂತಿರುಗಿ ಉತ್ತಮ;
    ಪೂರ್ವನಿಯೋಜಿತ:
      S = S.drop_front(1);
    }
  }

  ಹಿಂತಿರುಗಿ ಉತ್ತಮ;
}

Expected<Glob_ನಮೂನೆ> Glob_ನಮೂನೆ::create(StringRef S,
                                          std::optional<size_t> MaxSubPatterns,
                                          ತಾರ್ಕಿಕ SlashAgnostic) {
  Glob_ನಮೂನೆ Pat;
  Pat.SlashAgnostic = SlashAgnostic;
  Pat.ನಮೂನೆ = S;

  ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *ಪೂರ್ವಪ್ರತ್ಯಯ_Metas =
      SlashAgnostic ? ಪೂರ್ವಪ್ರತ್ಯಯ_Metacharacters_With_Slash : ಪೂರ್ವಪ್ರತ್ಯಯ_Metacharacters;
  ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *SuffixMetas =
      SlashAgnostic ? SuffixMetacharactersWithSlash : SuffixMetacharacters;

  // Store the prefix that does not contain any metacharacter.
  Pat.ಪೂರ್ವಪ್ರತ್ಯಯ_ಗಾತ್ರ = S.ಕಂಡುಹಿಡಿ_ಮೊದಲ_ದ(ಪೂರ್ವಪ್ರತ್ಯಯ_Metas);
  ಆದರೆ (Pat.ಪೂರ್ವಪ್ರತ್ಯಯ_ಗಾತ್ರ == std::string::npos) {
    Pat.ಪೂರ್ವಪ್ರತ್ಯಯ_ಗಾತ್ರ = S.ಗಾತ್ರ_3();
    ಹಿಂತಿರುಗಿ Pat;
  }
  S = S.substr(Pat.ಪೂರ್ವಪ್ರತ್ಯಯ_ಗಾತ್ರ);

  // Just in case we stop on unmatched opening brackets.
  size_t Suffix_ಆರಂಭ = S.ಕಂಡುಹಿಡಿ_ಕೊನೆಯ_ದ(SuffixMetas);
  ಪ್ರತಿಜ್ಞಾ(Suffix_ಆರಂಭ != std::string::npos);
  ಆದರೆ (S[Suffix_ಆರಂಭ] == '\\')
    ++Suffix_ಆರಂಭ;
  ಆದರೆ (Suffix_ಆರಂಭ < S.ಗಾತ್ರ_3())
    ++Suffix_ಆರಂಭ;
  Pat.Suffix_ಗಾತ್ರ = S.ಗಾತ್ರ_3() - Suffix_ಆರಂಭ;
  S = S.substr(0, Suffix_ಆರಂಭ);

  Small_ಸದಿಶ<std::string, 1> SubPats;
  ಆದರೆ (ಸ್ವಯಂಚಾಲಿತ Err = parseBraceExpansions(S, MaxSubPatterns).moveInto(SubPats))
    ಹಿಂತಿರುಗಿ std::move(Err);
  ಗಾಗಿ (StringRef SubPat : SubPats) {
    ಸ್ವಯಂಚಾಲಿತ SubGlobOrErr = Sub_Glob_ನಮೂನೆ::create(SubPat, SlashAgnostic);
    ಆದರೆ (!SubGlobOrErr)
      ಹಿಂತಿರುಗಿ SubGlobOrErr.takeError();
    Pat.SubGlobs.ತಳ್ಳು_ಹಿಂಭಾಗ(*SubGlobOrErr);
  }

  ಹಿಂತಿರುಗಿ Pat;
}

std::optional<std::string> Glob_ನಮೂನೆ::asLiteral() ಸ್ಥಿರಾಂಕ {
  // The prefix and suffix are metacharacter-free by construction, so whether
  // this pattern denotes a single string is decided by the sub-patterns, which
  // recorded it while parsing. No sub-pattern at all means there was no
  // metacharacter; more than one means brace expansion produced a choice.
  ಆದರೆ (SubGlobs.ಗಾತ್ರ_3() > 1 || (SubGlobs.ಗಾತ್ರ_3() == 1 && !SubGlobs[0].isLiteral()))
    ಹಿಂತಿರುಗಿ std::nullopt;

  std::string Literal;
  Literal.reserve(ನಮೂನೆ.ಗಾತ್ರ_3());
  ಗಾಗಿ (size_t I = 0, E = ನಮೂನೆ.ಗಾತ್ರ_3(); I != E; ++I) {
    ಆದರೆ (ನಮೂನೆ[I] == '\\' && I + 1 != E)
      ++I;
    Literal.ತಳ್ಳು_ಹಿಂಭಾಗ(ನಮೂನೆ[I]);
  }

  // In slash-agnostic mode '/' and '\\' match each other, so a pattern holding
  // either matches more than one string. A pattern with no sub-pattern cannot
  // reach here holding one, as both are prefix metacharacters in that mode.
  ಆದರೆ (SlashAgnostic &&
      StringRef(Literal).ಕಂಡುಹಿಡಿ_ಮೊದಲ_ದ("/\\") != StringRef::npos)
    ಹಿಂತಿರುಗಿ std::nullopt;

  ಹಿಂತಿರುಗಿ Literal;
}

Expected<Glob_ನಮೂನೆ::Sub_Glob_ನಮೂನೆ>
Glob_ನಮೂನೆ::Sub_Glob_ನಮೂನೆ::create(StringRef S, ತಾರ್ಕಿಕ SlashAgnostic) {
  Sub_Glob_ನಮೂನೆ Pat;

  // Parse brackets.
  Pat.Pat.ನಿಯೋಜಿಸು(S.begin(), S.ಕೊನೆ());
  ಗಾಗಿ (size_t I = 0, E = S.ಗಾತ್ರ_3(); I != E; ++I) {
    ಆದರೆ (S[I] == '[') {
      // ']' is allowed as the first character of a character class. '[]' is
      // invalid. So, just skip the first character.
      ++I;
      size_t J = S.ಕಂಡುಹಿಡಿ(']', I + 1);
      ಆದರೆ (J == StringRef::npos)
        ಹಿಂತಿರುಗಿ make_error<StringError>("invalid glob pattern, unmatched '['",
                                       errc::invalid_ವಾದ);
      StringRef Chars = S.substr(I, J - I);
      ತಾರ್ಕಿಕ Invert = S[I] == '^' || S[I] == '!';
      ಆದರೆ (Invert)
        Chars = Chars.drop_front();
      Expected<ಬಿಟ್_ಸದಿಶ> BVOrErr = expand(Chars, S);
      ಆದರೆ (!BVOrErr)
        ಹಿಂತಿರುಗಿ BVOrErr.takeError();
      ಬಿಟ್_ಸದಿಶ &BV = *BVOrErr;
      ಆದರೆ (SlashAgnostic && (BV['\\'] || BV['/'])) {
        BV.ಗಣ('\\');
        BV.ಗಣ('/');
      }
      ಆದರೆ (Invert)
        BV.flip();
      Pat.Brackets.ತಳ್ಳು_ಹಿಂಭಾಗ(Bracket{J + 1, std::move(BV)});
      I = J;
    } ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (S[I] == '\\') {
      ಆದರೆ (++I == E)
        ಹಿಂತಿರುಗಿ make_error<StringError>("invalid glob pattern, stray '\\'",
                                       errc::invalid_ವಾದ);
    } ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (S[I] == '*' || S[I] == '?') {
      Pat.HasWildcard = ನಿಜ;
    }
  }
  ಹಿಂತಿರುಗಿ Pat;
}

StringRef Glob_ನಮೂನೆ::longest_substr() ಸ್ಥಿರಾಂಕ {
  ಹಿಂತಿರುಗಿ maxPlainSubstring(ನಮೂನೆ.drop_front(ಪೂರ್ವಪ್ರತ್ಯಯ_ಗಾತ್ರ).drop_ಹಿಂಭಾಗ(Suffix_ಗಾತ್ರ),
                           SlashAgnostic);
}

ತಾರ್ಕಿಕ Glob_ನಮೂನೆ::ಹೊಂದಿಕೆ(StringRef S) ಸ್ಥಿರಾಂಕ {
  ಆದರೆ (!S.consume_front(ಪೂರ್ವಪ್ರತ್ಯಯ_2()))
    ಹಿಂತಿರುಗಿ ಸುಳ್ಳು;
  ಆದರೆ (!S.consume_ಹಿಂಭಾಗ(suffix()))
    ಹಿಂತಿರುಗಿ ಸುಳ್ಳು;
  ಆದರೆ (SubGlobs.ಖಾಲಿಗಳು() && S.ಖಾಲಿಗಳು())
    ಹಿಂತಿರುಗಿ ನಿಜ;
  ಗಾಗಿ (ಸ್ವಯಂಚಾಲಿತ &Glob : SubGlobs)
    ಆದರೆ (Glob.ಹೊಂದಿಕೆ(S, SlashAgnostic))
      ಹಿಂತಿರುಗಿ ನಿಜ;
  ಹಿಂತಿರುಗಿ ಸುಳ್ಳು;
}

ಸ್ಥಿರ ತಾರ್ಕಿಕ ಹೊಂದಿಕೆ_Char(ಅಕ್ಷರ PatC, ಅಕ್ಷರ QueryC, ತಾರ್ಕಿಕ SlashAgnostic) {
  ಆದರೆ (PatC == QueryC)
    ಹಿಂತಿರುಗಿ ನಿಜ;
  ಹಿಂತಿರುಗಿ SlashAgnostic && (PatC == '\\' || PatC == '/') &&
         (QueryC == '\\' || QueryC == '/');
}

// Factor the pattern into segments split by '*'. The segment is matched
// sequentianlly by finding the first occurrence past the end of the previous
// match.
ತಾರ್ಕಿಕ Glob_ನಮೂನೆ::Sub_Glob_ನಮೂನೆ::ಹೊಂದಿಕೆ(StringRef Str,
                                        ತಾರ್ಕಿಕ SlashAgnostic) ಸ್ಥಿರಾಂಕ {
  ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *P = Pat.ದತ್ತಾಂಶ(), *SegmentBegin = ಶೂನ್ಯಸೂಚಕ, *S = Str.ದತ್ತಾಂಶ(),
             *SavedS = S;
  ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *ಸ್ಥಿರಾಂಕ PEnd = P + Pat.ಗಾತ್ರ_3(), *ಸ್ಥಿರಾಂಕ ಕೊನೆ_2 = S + Str.ಗಾತ್ರ_3();
  size_t B = 0, SavedB = 0;
  ಆಗ (S != ಕೊನೆ_2) {
    ಆದರೆ (P == PEnd)
      ;
    ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (*P == '*') {
      // The non-* substring on the left of '*' matches the tail of S. Save the
      // positions to be used by backtracking if we see a mismatch later.
      SegmentBegin = ++P;
      SavedS = S;
      SavedB = B;
      ಮುಂದುವರಿ;
    } ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (*P == '[') {
      ಆದರೆ (Brackets[B].ಬೈಟುಗಳು_2[uint8_t(*S)]) {
        P = Pat.ದತ್ತಾಂಶ() + Brackets[B++].ಮುಂದಿನ_ಸ್ಥಾನಾಂತರ;
        ++S;
        ಮುಂದುವರಿ;
      }
    } ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (*P == '\\') {
      ಆದರೆ (ಹೊಂದಿಕೆ_Char(*++P, *S, SlashAgnostic)) {
        ++P;
        ++S;
        ಮುಂದುವರಿ;
      }
    } ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (ಹೊಂದಿಕೆ_Char(*P, *S, SlashAgnostic) || *P == '?') {
      ++P;
      ++S;
      ಮುಂದುವರಿ;
    }
    ಆದರೆ (!SegmentBegin)
      ಹಿಂತಿರುಗಿ ಸುಳ್ಳು;
    // We have seen a '*'. Backtrack to the saved positions. Shift the S
    // position to probe the next starting position in the segment.
    P = SegmentBegin;
    S = ++SavedS;
    B = SavedB;
  }
  // All bytes in Str have been matched. Return true if the rest part of Pat is
  // empty or contains only '*'.
  ಹಿಂತಿರುಗಿ getPat().ಕಂಡುಹಿಡಿ_ಮೊದಲ_not_ದ('*', P - Pat.ದತ್ತಾಂಶ()) == std::string::npos;
}
