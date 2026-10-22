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

#ਅਨ੍ਤਰ੍ਭਾਵ "llvm/Support/GlobPattern.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "llvm/ADT/StringRef.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "llvm/Support/Errc.h"

ਵਰਤੋਂ ਨਾਮਾਕਾਸ਼ llvm;

ਸਥਿਰ constexpr ਅੱਖਰ ਅਗੇਤਰ_Metacharacters[] = "?*[{\\";
ਸਥਿਰ constexpr ਅੱਖਰ SuffixMetacharacters[] = "?*[]{}\\";
ਸਥਿਰ constexpr ਅੱਖਰ ਅਗੇਤਰ_Metacharacters_With_Slash[] = "?*[{\\/";
ਸਥਿਰ constexpr ਅੱਖਰ SuffixMetacharactersWithSlash[] = "?*[]{}\\/";

// Expands character ranges and returns a bitmap.
// For example, "a-cf-hz" is expanded to "abcfghz".
ਸਥਿਰ Expected<ਦੋਆਧਾਰੀ_ਅੰਕ_ਸਦਿਸ਼> expand(StringRef S, StringRef Original) {
  ਦੋਆਧਾਰੀ_ਅੰਕ_ਸਦਿਸ਼ BV(256, ਝੂਠ);

  // Expand X-Y.
  ਲਈ (;;) {
    ਜੇ (S.ਮਾਪ_2() < 3)
      ਰੋਕੋ;

    uint8_t ਸ਼ੁਰੂ = S[0];
    uint8_t ਸਿਰਾ_2 = S[2];

    // If it doesn't start with something like X-Y,
    // consume the first character and proceed.
    ਜੇ (S[1] != '-') {
      BV[ਸ਼ੁਰੂ] = ਸੱਚ;
      S = S.substr(1);
      ਜਾਰੀ;
    }

    // It must be in the form of X-Y.
    // Validate it and then interpret the range.
    ਜੇ (ਸ਼ੁਰੂ > ਸਿਰਾ_2)
      ਵਾਪਸ make_error<StringError>("invalid glob pattern: " + Original,
                                     errc::invalid_ਦਲੀਲ);

    ਲਈ (ਪੂਰਨ C = ਸ਼ੁਰੂ; C <= ਸਿਰਾ_2; ++C)
      BV[(uint8_t)C] = ਸੱਚ;
    S = S.substr(3);
  }

  ਲਈ (ਅੱਖਰ C : S)
    BV[(uint8_t)C] = ਸੱਚ;
  ਵਾਪਸ BV;
}

// Identify brace expansions in S and return the list of patterns they expand
// into.
ਸਥਿਰ Expected<Small_ਸਦਿਸ਼<std::string, 1>>
parseBraceExpansions(StringRef S, std::optional<size_t> MaxSubPatterns) {
  Small_ਸਦਿਸ਼<std::string> SubPatterns = {S.str()};
  ਜੇ (!MaxSubPatterns || !S.ਸ਼ਾਮਲ_ਹੈ('{'))
    ਵਾਪਸ std::move(SubPatterns);

  ਬਣਤਰ BraceExpansion {
    size_t ਸ਼ੁਰੂ;
    size_t ਲੰਮਾਈ;
    Small_ਸਦਿਸ਼<StringRef, 2> Terms;
  };
  Small_ਸਦਿਸ਼<BraceExpansion, 0> BraceExpansions;

  BraceExpansion *CurrentBE = ਖਾਲੀ_ਸੂਚਕ;
  size_t ਪਦ_Begin;
  ਲਈ (size_t I = 0, E = S.ਮਾਪ_2(); I != E; ++I) {
    ਜੇ (S[I] == '[') {
      I = S.ਲੱਭੋ(']', I + 2);
      ਜੇ (I == std::string::npos)
        ਵਾਪਸ make_error<StringError>("invalid glob pattern, unmatched '['",
                                       errc::invalid_ਦਲੀਲ);
    } ਨਹੀਂਤਾਂ ਜੇ (S[I] == '{') {
      ਜੇ (CurrentBE)
        ਵਾਪਸ make_error<StringError>(
            "nested brace expansions are not supported",
            errc::invalid_ਦਲੀਲ);
      CurrentBE = &BraceExpansions.emplace_ਪਿੱਠ();
      CurrentBE->ਸ਼ੁਰੂ = I;
      ਪਦ_Begin = I + 1;
    } ਨਹੀਂਤਾਂ ਜੇ (S[I] == ',') {
      ਜੇ (!CurrentBE)
        ਜਾਰੀ;
      CurrentBE->Terms.ਧੱਕੋ_ਪਿੱਠ(S.substr(ਪਦ_Begin, I - ਪਦ_Begin));
      ਪਦ_Begin = I + 1;
    } ਨਹੀਂਤਾਂ ਜੇ (S[I] == '}') {
      ਜੇ (!CurrentBE)
        ਜਾਰੀ;
      ਜੇ (CurrentBE->Terms.ਖਾਲੀ_ਥਾਵਾਂ())
        ਵਾਪਸ make_error<StringError>(
            "empty or singleton brace expansions are not supported",
            errc::invalid_ਦਲੀਲ);
      CurrentBE->Terms.ਧੱਕੋ_ਪਿੱਠ(S.substr(ਪਦ_Begin, I - ਪਦ_Begin));
      CurrentBE->ਲੰਮਾਈ = I - CurrentBE->ਸ਼ੁਰੂ + 1;
      CurrentBE = ਖਾਲੀ_ਸੂਚਕ;
    } ਨਹੀਂਤਾਂ ਜੇ (S[I] == '\\') {
      ਜੇ (++I == E)
        ਵਾਪਸ make_error<StringError>("invalid glob pattern, stray '\\'",
                                       errc::invalid_ਦਲੀਲ);
    }
  }
  ਜੇ (CurrentBE)
    ਵਾਪਸ make_error<StringError>("incomplete brace expansion",
                                   errc::invalid_ਦਲੀਲ);

  size_t ਅੰਕ_Sub_Patterns = 1;
  ਲਈ (ਸਵੈਚਾਲਿਤ &BE : BraceExpansions) {
    ਜੇ (ਅੰਕ_Sub_Patterns > std::numeric_limits<size_t>::max() / BE.Terms.ਮਾਪ_2()) {
      ਅੰਕ_Sub_Patterns = std::numeric_limits<size_t>::max();
      ਰੋਕੋ;
    }
    ਅੰਕ_Sub_Patterns *= BE.Terms.ਮਾਪ_2();
  }
  ਜੇ (ਅੰਕ_Sub_Patterns > *MaxSubPatterns)
    ਵਾਪਸ make_error<StringError>("too many brace expansions",
                                   errc::invalid_ਦਲੀਲ);
  // Replace brace expansions in reverse order so that we don't invalidate
  // earlier start indices
  ਲਈ (ਸਵੈਚਾਲਿਤ &BE : ਉਲਟ(BraceExpansions)) {
    Small_ਸਦਿਸ਼<std::string> OrigSubPatterns;
    std::ਅਦਲਾ_ਬਦਲੀ(SubPatterns, OrigSubPatterns);
    ਲਈ (StringRef ਪਦ : BE.Terms)
      ਲਈ (StringRef Orig : OrigSubPatterns)
        SubPatterns.emplace_ਪਿੱਠ(Orig).replace(BE.ਸ਼ੁਰੂ, BE.ਲੰਮਾਈ, ਪਦ);
  }
  ਵਾਪਸ std::move(SubPatterns);
}

ਸਥਿਰ StringRef maxPlainSubstring(StringRef S, ਤਾਰਕਿਕ SlashAgnostic) {
  ਅਚਲ ਅੱਖਰ *Metas =
      SlashAgnostic ? ਅਗੇਤਰ_Metacharacters_With_Slash : ਅਗੇਤਰ_Metacharacters;
  StringRef ਉੱਤਮ;
  ਜਦੋਂ (!S.ਖਾਲੀ_ਥਾਵਾਂ()) {
    size_t ਅਗੇਤਰ_ਮਾਪ = S.ਲੱਭੋ_ਪਹਿਲਾ_ਦਾ(Metas);
    ਜੇ (ਅਗੇਤਰ_ਮਾਪ == std::string::npos)
      ਅਗੇਤਰ_ਮਾਪ = S.ਮਾਪ_2();

    ਜੇ (ਉੱਤਮ.ਮਾਪ_2() < ਅਗੇਤਰ_ਮਾਪ)
      ਉੱਤਮ = S.take_front(ਅਗੇਤਰ_ਮਾਪ);

    S = S.drop_front(ਅਗੇਤਰ_ਮਾਪ);

    // It's impossible, as the first and last characters of the input string
    // must be Glob special characters, otherwise they would be parts of
    // the prefix or the suffix.
    ਪ੍ਰਤਿਜ੍ਞਾ(!S.ਖਾਲੀ_ਥਾਵਾਂ());

    ਚੋਣ (S.front()) {
    ਹਾਲਤ '\\':
      S = S.drop_front(2);
      ਰੋਕੋ;
    ਹਾਲਤ '[': {
      // Drop '[' and the first character which can be ']'.
      S = S.drop_front(2);
      size_t ਸਿਰਾ_Bracket = S.ਲੱਭੋ_ਪਹਿਲਾ_ਦਾ("]");
      // Should not be possible, SubGlobPattern::create should fail on invalid
      // pattern before we get here.
      ਪ੍ਰਤਿਜ੍ਞਾ(ਸਿਰਾ_Bracket != std::string::npos);
      S = S.drop_front(ਸਿਰਾ_Bracket + 1);
      ਰੋਕੋ;
    }
    ਹਾਲਤ '{':
      // TODO: implement.
      // Fallback to whatever is best for now.
      ਵਾਪਸ ਉੱਤਮ;
    ਮੂਲ:
      S = S.drop_front(1);
    }
  }

  ਵਾਪਸ ਉੱਤਮ;
}

Expected<Glob_ਨਮੂਨਾ> Glob_ਨਮੂਨਾ::create(StringRef S,
                                          std::optional<size_t> MaxSubPatterns,
                                          ਤਾਰਕਿਕ SlashAgnostic) {
  Glob_ਨਮੂਨਾ Pat;
  Pat.SlashAgnostic = SlashAgnostic;
  Pat.ਨਮੂਨਾ = S;

  ਅਚਲ ਅੱਖਰ *ਅਗੇਤਰ_Metas =
      SlashAgnostic ? ਅਗੇਤਰ_Metacharacters_With_Slash : ਅਗੇਤਰ_Metacharacters;
  ਅਚਲ ਅੱਖਰ *SuffixMetas =
      SlashAgnostic ? SuffixMetacharactersWithSlash : SuffixMetacharacters;

  // Store the prefix that does not contain any metacharacter.
  Pat.ਅਗੇਤਰ_ਮਾਪ = S.ਲੱਭੋ_ਪਹਿਲਾ_ਦਾ(ਅਗੇਤਰ_Metas);
  ਜੇ (Pat.ਅਗੇਤਰ_ਮਾਪ == std::string::npos) {
    Pat.ਅਗੇਤਰ_ਮਾਪ = S.ਮਾਪ_2();
    ਵਾਪਸ Pat;
  }
  S = S.substr(Pat.ਅਗੇਤਰ_ਮਾਪ);

  // Just in case we stop on unmatched opening brackets.
  size_t Suffix_ਸ਼ੁਰੂ = S.ਲੱਭੋ_ਆਖਰੀ_ਦਾ(SuffixMetas);
  ਪ੍ਰਤਿਜ੍ਞਾ(Suffix_ਸ਼ੁਰੂ != std::string::npos);
  ਜੇ (S[Suffix_ਸ਼ੁਰੂ] == '\\')
    ++Suffix_ਸ਼ੁਰੂ;
  ਜੇ (Suffix_ਸ਼ੁਰੂ < S.ਮਾਪ_2())
    ++Suffix_ਸ਼ੁਰੂ;
  Pat.Suffix_ਮਾਪ = S.ਮਾਪ_2() - Suffix_ਸ਼ੁਰੂ;
  S = S.substr(0, Suffix_ਸ਼ੁਰੂ);

  Small_ਸਦਿਸ਼<std::string, 1> SubPats;
  ਜੇ (ਸਵੈਚਾਲਿਤ Err = parseBraceExpansions(S, MaxSubPatterns).moveInto(SubPats))
    ਵਾਪਸ std::move(Err);
  ਲਈ (StringRef SubPat : SubPats) {
    ਸਵੈਚਾਲਿਤ SubGlobOrErr = Sub_Glob_ਨਮੂਨਾ::create(SubPat, SlashAgnostic);
    ਜੇ (!SubGlobOrErr)
      ਵਾਪਸ SubGlobOrErr.takeError();
    Pat.SubGlobs.ਧੱਕੋ_ਪਿੱਠ(*SubGlobOrErr);
  }

  ਵਾਪਸ Pat;
}

std::optional<std::string> Glob_ਨਮੂਨਾ::asLiteral() ਅਚਲ {
  // The prefix and suffix are metacharacter-free by construction, so whether
  // this pattern denotes a single string is decided by the sub-patterns, which
  // recorded it while parsing. No sub-pattern at all means there was no
  // metacharacter; more than one means brace expansion produced a choice.
  ਜੇ (SubGlobs.ਮਾਪ_2() > 1 || (SubGlobs.ਮਾਪ_2() == 1 && !SubGlobs[0].isLiteral()))
    ਵਾਪਸ std::nullopt;

  std::string Literal;
  Literal.reserve(ਨਮੂਨਾ.ਮਾਪ_2());
  ਲਈ (size_t I = 0, E = ਨਮੂਨਾ.ਮਾਪ_2(); I != E; ++I) {
    ਜੇ (ਨਮੂਨਾ[I] == '\\' && I + 1 != E)
      ++I;
    Literal.ਧੱਕੋ_ਪਿੱਠ(ਨਮੂਨਾ[I]);
  }

  // In slash-agnostic mode '/' and '\\' match each other, so a pattern holding
  // either matches more than one string. A pattern with no sub-pattern cannot
  // reach here holding one, as both are prefix metacharacters in that mode.
  ਜੇ (SlashAgnostic &&
      StringRef(Literal).ਲੱਭੋ_ਪਹਿਲਾ_ਦਾ("/\\") != StringRef::npos)
    ਵਾਪਸ std::nullopt;

  ਵਾਪਸ Literal;
}

Expected<Glob_ਨਮੂਨਾ::Sub_Glob_ਨਮੂਨਾ>
Glob_ਨਮੂਨਾ::Sub_Glob_ਨਮੂਨਾ::create(StringRef S, ਤਾਰਕਿਕ SlashAgnostic) {
  Sub_Glob_ਨਮੂਨਾ Pat;

  // Parse brackets.
  Pat.Pat.ਸੌਂਪੋ(S.begin(), S.ਸਿਰਾ());
  ਲਈ (size_t I = 0, E = S.ਮਾਪ_2(); I != E; ++I) {
    ਜੇ (S[I] == '[') {
      // ']' is allowed as the first character of a character class. '[]' is
      // invalid. So, just skip the first character.
      ++I;
      size_t J = S.ਲੱਭੋ(']', I + 1);
      ਜੇ (J == StringRef::npos)
        ਵਾਪਸ make_error<StringError>("invalid glob pattern, unmatched '['",
                                       errc::invalid_ਦਲੀਲ);
      StringRef Chars = S.substr(I, J - I);
      ਤਾਰਕਿਕ Invert = S[I] == '^' || S[I] == '!';
      ਜੇ (Invert)
        Chars = Chars.drop_front();
      Expected<ਦੋਆਧਾਰੀ_ਅੰਕ_ਸਦਿਸ਼> BVOrErr = expand(Chars, S);
      ਜੇ (!BVOrErr)
        ਵਾਪਸ BVOrErr.takeError();
      ਦੋਆਧਾਰੀ_ਅੰਕ_ਸਦਿਸ਼ &BV = *BVOrErr;
      ਜੇ (SlashAgnostic && (BV['\\'] || BV['/'])) {
        BV.ਸੈੱਟ('\\');
        BV.ਸੈੱਟ('/');
      }
      ਜੇ (Invert)
        BV.flip();
      Pat.Brackets.ਧੱਕੋ_ਪਿੱਠ(Bracket{J + 1, std::move(BV)});
      I = J;
    } ਨਹੀਂਤਾਂ ਜੇ (S[I] == '\\') {
      ਜੇ (++I == E)
        ਵਾਪਸ make_error<StringError>("invalid glob pattern, stray '\\'",
                                       errc::invalid_ਦਲੀਲ);
    } ਨਹੀਂਤਾਂ ਜੇ (S[I] == '*' || S[I] == '?') {
      Pat.HasWildcard = ਸੱਚ;
    }
  }
  ਵਾਪਸ Pat;
}

StringRef Glob_ਨਮੂਨਾ::longest_substr() ਅਚਲ {
  ਵਾਪਸ maxPlainSubstring(ਨਮੂਨਾ.drop_front(ਅਗੇਤਰ_ਮਾਪ).drop_ਪਿੱਠ(Suffix_ਮਾਪ),
                           SlashAgnostic);
}

ਤਾਰਕਿਕ Glob_ਨਮੂਨਾ::ਮੇਲ(StringRef S) ਅਚਲ {
  ਜੇ (!S.consume_front(ਅਗੇਤਰ_2()))
    ਵਾਪਸ ਝੂਠ;
  ਜੇ (!S.consume_ਪਿੱਠ(suffix()))
    ਵਾਪਸ ਝੂਠ;
  ਜੇ (SubGlobs.ਖਾਲੀ_ਥਾਵਾਂ() && S.ਖਾਲੀ_ਥਾਵਾਂ())
    ਵਾਪਸ ਸੱਚ;
  ਲਈ (ਸਵੈਚਾਲਿਤ &Glob : SubGlobs)
    ਜੇ (Glob.ਮੇਲ(S, SlashAgnostic))
      ਵਾਪਸ ਸੱਚ;
  ਵਾਪਸ ਝੂਠ;
}

ਸਥਿਰ ਤਾਰਕਿਕ ਮੇਲ_Char(ਅੱਖਰ PatC, ਅੱਖਰ QueryC, ਤਾਰਕਿਕ SlashAgnostic) {
  ਜੇ (PatC == QueryC)
    ਵਾਪਸ ਸੱਚ;
  ਵਾਪਸ SlashAgnostic && (PatC == '\\' || PatC == '/') &&
         (QueryC == '\\' || QueryC == '/');
}

// Factor the pattern into segments split by '*'. The segment is matched
// sequentianlly by finding the first occurrence past the end of the previous
// match.
ਤਾਰਕਿਕ Glob_ਨਮੂਨਾ::Sub_Glob_ਨਮੂਨਾ::ਮੇਲ(StringRef Str,
                                        ਤਾਰਕਿਕ SlashAgnostic) ਅਚਲ {
  ਅਚਲ ਅੱਖਰ *P = Pat.ਅੰਕੜੇ(), *SegmentBegin = ਖਾਲੀ_ਸੂਚਕ, *S = Str.ਅੰਕੜੇ(),
             *SavedS = S;
  ਅਚਲ ਅੱਖਰ *ਅਚਲ PEnd = P + Pat.ਮਾਪ_2(), *ਅਚਲ ਸਿਰਾ_2 = S + Str.ਮਾਪ_2();
  size_t B = 0, SavedB = 0;
  ਜਦੋਂ (S != ਸਿਰਾ_2) {
    ਜੇ (P == PEnd)
      ;
    ਨਹੀਂਤਾਂ ਜੇ (*P == '*') {
      // The non-* substring on the left of '*' matches the tail of S. Save the
      // positions to be used by backtracking if we see a mismatch later.
      SegmentBegin = ++P;
      SavedS = S;
      SavedB = B;
      ਜਾਰੀ;
    } ਨਹੀਂਤਾਂ ਜੇ (*P == '[') {
      ਜੇ (Brackets[B].ਬਾਈਟਾਂ_2[uint8_t(*S)]) {
        P = Pat.ਅੰਕੜੇ() + Brackets[B++].ਅਗਲਾ_ਵਿੱਥ;
        ++S;
        ਜਾਰੀ;
      }
    } ਨਹੀਂਤਾਂ ਜੇ (*P == '\\') {
      ਜੇ (ਮੇਲ_Char(*++P, *S, SlashAgnostic)) {
        ++P;
        ++S;
        ਜਾਰੀ;
      }
    } ਨਹੀਂਤਾਂ ਜੇ (ਮੇਲ_Char(*P, *S, SlashAgnostic) || *P == '?') {
      ++P;
      ++S;
      ਜਾਰੀ;
    }
    ਜੇ (!SegmentBegin)
      ਵਾਪਸ ਝੂਠ;
    // We have seen a '*'. Backtrack to the saved positions. Shift the S
    // position to probe the next starting position in the segment.
    P = SegmentBegin;
    S = ++SavedS;
    B = SavedB;
  }
  // All bytes in Str have been matched. Return true if the rest part of Pat is
  // empty or contains only '*'.
  ਵਾਪਸ getPat().ਲੱਭੋ_ਪਹਿਲਾ_not_ਦਾ('*', P - Pat.ਅੰਕੜੇ()) == std::string::npos;
}
