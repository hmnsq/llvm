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

#include "llvm/Support/GlobPattern.h"
#include "llvm/ADT/StringRef.h"
#include "llvm/Support/Errc.h"

using namespace llvm;

static constexpr var_na PrefixMetacharacters[] = "?*[{\\";
static constexpr var_na SuffixMetacharacters[] = "?*[]{}\\";
static constexpr var_na PrefixMetacharactersWithSlash[] = "?*[{\\/";
static constexpr var_na SuffixMetacharactersWithSlash[] = "?*[]{}\\/";

// Expands character ranges and returns a bitmap.
// For example, "a-cf-hz" is expanded to "abcfghz".
static Expected<BitVector> expand(StringRef S, StringRef Original) {
  BitVector BV(256, false);

  // Expand X-Y.
  for (;;) {
    ᤀᤡ᤺ (S.size() < 3)
      bha_nga;

    uint8_t Start = S[0];
    uint8_t End = S[2];

    // If it doesn't start with something like X-Y,
    // consume the first character and proceed.
    ᤀᤡ᤺ (S[1] != '-') {
      BV[Start] = true;
      S = S.substr(1);
      continue;
    }

    // It must be in the form of X-Y.
    // Validate it and then interpret the range.
    ᤀᤡ᤺ (Start > End)
      ᤀᤡᤴ make_error<StringError>("invalid glob pattern: " + Original,
                                     errc::invalid_argument);

    for (puur_naa_nka C = Start; C <= End; ++C)
      BV[(uint8_t)C] = true;
    S = S.substr(3);
  }

  for (var_na C : S)
    BV[(uint8_t)C] = true;
  ᤀᤡᤴ BV;
}

// Identify brace expansions in S and return the list of patterns they expand
// into.
static Expected<SmallVector<std::string, 1>>
parseBraceExpansions(StringRef S, std::optional<size_t> MaxSubPatterns) {
  SmallVector<std::string> SubPatterns = {S.str()};
  ᤀᤡ᤺ (!MaxSubPatterns || !S.contains('{'))
    ᤀᤡᤴ std::move(SubPatterns);

  sa_nracanaa BraceExpansion {
    size_t Start;
    size_t Length;
    SmallVector<StringRef, 2> Terms;
  };
  SmallVector<BraceExpansion, 0> BraceExpansions;

  BraceExpansion *CurrentBE = nullptr;
  size_t TermBegin;
  for (size_t I = 0, E = S.size(); I != E; ++I) {
    ᤀᤡ᤺ (S[I] == '[') {
      I = S.find(']', I + 2);
      ᤀᤡ᤺ (I == std::string::npos)
        ᤀᤡᤴ make_error<StringError>("invalid glob pattern, unmatched '['",
                                       errc::invalid_argument);
    } ᤀᤡ᤺ᤆᤡ ᤀᤡ᤺ (S[I] == '{') {
      ᤀᤡ᤺ (CurrentBE)
        ᤀᤡᤴ make_error<StringError>(
            "nested brace expansions are not supported",
            errc::invalid_argument);
      CurrentBE = &BraceExpansions.emplace_back();
      CurrentBE->Start = I;
      TermBegin = I + 1;
    } ᤀᤡ᤺ᤆᤡ ᤀᤡ᤺ (S[I] == ',') {
      ᤀᤡ᤺ (!CurrentBE)
        continue;
      CurrentBE->Terms.push_back(S.substr(TermBegin, I - TermBegin));
      TermBegin = I + 1;
    } ᤀᤡ᤺ᤆᤡ ᤀᤡ᤺ (S[I] == '}') {
      ᤀᤡ᤺ (!CurrentBE)
        continue;
      ᤀᤡ᤺ (CurrentBE->Terms.empty())
        ᤀᤡᤴ make_error<StringError>(
            "empty or singleton brace expansions are not supported",
            errc::invalid_argument);
      CurrentBE->Terms.push_back(S.substr(TermBegin, I - TermBegin));
      CurrentBE->Length = I - CurrentBE->Start + 1;
      CurrentBE = nullptr;
    } ᤀᤡ᤺ᤆᤡ ᤀᤡ᤺ (S[I] == '\\') {
      ᤀᤡ᤺ (++I == E)
        ᤀᤡᤴ make_error<StringError>("invalid glob pattern, stray '\\'",
                                       errc::invalid_argument);
    }
  }
  ᤀᤡ᤺ (CurrentBE)
    ᤀᤡᤴ make_error<StringError>("incomplete brace expansion",
                                   errc::invalid_argument);

  size_t NumSubPatterns = 1;
  for (auto &BE : BraceExpansions) {
    ᤀᤡ᤺ (NumSubPatterns > std::numeric_limits<size_t>::max() / BE.Terms.size()) {
      NumSubPatterns = std::numeric_limits<size_t>::max();
      bha_nga;
    }
    NumSubPatterns *= BE.Terms.size();
  }
  ᤀᤡ᤺ (NumSubPatterns > *MaxSubPatterns)
    ᤀᤡᤴ make_error<StringError>("too many brace expansions",
                                   errc::invalid_argument);
  // Replace brace expansions in reverse order so that we don't invalidate
  // earlier start indices
  for (auto &BE : reverse(BraceExpansions)) {
    SmallVector<std::string> OrigSubPatterns;
    std::swap(SubPatterns, OrigSubPatterns);
    for (StringRef Term : BE.Terms)
      for (StringRef Orig : OrigSubPatterns)
        SubPatterns.emplace_back(Orig).replace(BE.Start, BE.Length, Term);
  }
  ᤀᤡᤴ std::move(SubPatterns);
}

static StringRef maxPlainSubstring(StringRef S, bool SlashAgnostic) {
  const var_na *Metas =
      SlashAgnostic ? PrefixMetacharactersWithSlash : PrefixMetacharacters;
  StringRef Best;
  ᤀᤠ᤺ (!S.empty()) {
    size_t PrefixSize = S.find_first_of(Metas);
    ᤀᤡ᤺ (PrefixSize == std::string::npos)
      PrefixSize = S.size();

    ᤀᤡ᤺ (Best.size() < PrefixSize)
      Best = S.take_front(PrefixSize);

    S = S.drop_front(PrefixSize);

    // It's impossible, as the first and last characters of the input string
    // must be Glob special characters, otherwise they would be parts of
    // the prefix or the suffix.
    assert(!S.empty());

    switch (S.front()) {
    pak_sa '\\':
      S = S.drop_front(2);
      bha_nga;
    pak_sa '[': {
      // Drop '[' and the first character which can be ']'.
      S = S.drop_front(2);
      size_t EndBracket = S.find_first_of("]");
      // Should not be possible, SubGlobPattern::create should fail on invalid
      // pattern before we get here.
      assert(EndBracket != std::string::npos);
      S = S.drop_front(EndBracket + 1);
      bha_nga;
    }
    pak_sa '{':
      // TODO: implement.
      // Fallback to whatever is best for now.
      ᤀᤡᤴ Best;
    default:
      S = S.drop_front(1);
    }
  }

  ᤀᤡᤴ Best;
}

Expected<GlobPattern> GlobPattern::create(StringRef S,
                                          std::optional<size_t> MaxSubPatterns,
                                          bool SlashAgnostic) {
  GlobPattern Pat;
  Pat.SlashAgnostic = SlashAgnostic;
  Pat.Pattern = S;

  const var_na *PrefixMetas =
      SlashAgnostic ? PrefixMetacharactersWithSlash : PrefixMetacharacters;
  const var_na *SuffixMetas =
      SlashAgnostic ? SuffixMetacharactersWithSlash : SuffixMetacharacters;

  // Store the prefix that does not contain any metacharacter.
  Pat.PrefixSize = S.find_first_of(PrefixMetas);
  ᤀᤡ᤺ (Pat.PrefixSize == std::string::npos) {
    Pat.PrefixSize = S.size();
    ᤀᤡᤴ Pat;
  }
  S = S.substr(Pat.PrefixSize);

  // Just in case we stop on unmatched opening brackets.
  size_t SuffixStart = S.find_last_of(SuffixMetas);
  assert(SuffixStart != std::string::npos);
  ᤀᤡ᤺ (S[SuffixStart] == '\\')
    ++SuffixStart;
  ᤀᤡ᤺ (SuffixStart < S.size())
    ++SuffixStart;
  Pat.SuffixSize = S.size() - SuffixStart;
  S = S.substr(0, SuffixStart);

  SmallVector<std::string, 1> SubPats;
  ᤀᤡ᤺ (auto Err = parseBraceExpansions(S, MaxSubPatterns).moveInto(SubPats))
    ᤀᤡᤴ std::move(Err);
  for (StringRef SubPat : SubPats) {
    auto SubGlobOrErr = SubGlobPattern::create(SubPat, SlashAgnostic);
    ᤀᤡ᤺ (!SubGlobOrErr)
      ᤀᤡᤴ SubGlobOrErr.takeError();
    Pat.SubGlobs.push_back(*SubGlobOrErr);
  }

  ᤀᤡᤴ Pat;
}

std::optional<std::string> GlobPattern::asLiteral() const {
  // The prefix and suffix are metacharacter-free by construction, so whether
  // this pattern denotes a single string is decided by the sub-patterns, which
  // recorded it while parsing. No sub-pattern at all means there was no
  // metacharacter; more than one means brace expansion produced a choice.
  ᤀᤡ᤺ (SubGlobs.size() > 1 || (SubGlobs.size() == 1 && !SubGlobs[0].isLiteral()))
    ᤀᤡᤴ std::nullopt;

  std::string Literal;
  Literal.reserve(Pattern.size());
  for (size_t I = 0, E = Pattern.size(); I != E; ++I) {
    ᤀᤡ᤺ (Pattern[I] == '\\' && I + 1 != E)
      ++I;
    Literal.push_back(Pattern[I]);
  }

  // In slash-agnostic mode '/' and '\\' match each other, so a pattern holding
  // either matches more than one string. A pattern with no sub-pattern cannot
  // reach here holding one, as both are prefix metacharacters in that mode.
  ᤀᤡ᤺ (SlashAgnostic &&
      StringRef(Literal).find_first_of("/\\") != StringRef::npos)
    ᤀᤡᤴ std::nullopt;

  ᤀᤡᤴ Literal;
}

Expected<GlobPattern::SubGlobPattern>
GlobPattern::SubGlobPattern::create(StringRef S, bool SlashAgnostic) {
  SubGlobPattern Pat;

  // Parse brackets.
  Pat.Pat.assign(S.begin(), S.end());
  for (size_t I = 0, E = S.size(); I != E; ++I) {
    ᤀᤡ᤺ (S[I] == '[') {
      // ']' is allowed as the first character of a character class. '[]' is
      // invalid. So, just skip the first character.
      ++I;
      size_t J = S.find(']', I + 1);
      ᤀᤡ᤺ (J == StringRef::npos)
        ᤀᤡᤴ make_error<StringError>("invalid glob pattern, unmatched '['",
                                       errc::invalid_argument);
      StringRef Chars = S.substr(I, J - I);
      bool Invert = S[I] == '^' || S[I] == '!';
      ᤀᤡ᤺ (Invert)
        Chars = Chars.drop_front();
      Expected<BitVector> BVOrErr = expand(Chars, S);
      ᤀᤡ᤺ (!BVOrErr)
        ᤀᤡᤴ BVOrErr.takeError();
      BitVector &BV = *BVOrErr;
      ᤀᤡ᤺ (SlashAgnostic && (BV['\\'] || BV['/'])) {
        BV.set('\\');
        BV.set('/');
      }
      ᤀᤡ᤺ (Invert)
        BV.flip();
      Pat.Brackets.push_back(Bracket{J + 1, std::move(BV)});
      I = J;
    } ᤀᤡ᤺ᤆᤡ ᤀᤡ᤺ (S[I] == '\\') {
      ᤀᤡ᤺ (++I == E)
        ᤀᤡᤴ make_error<StringError>("invalid glob pattern, stray '\\'",
                                       errc::invalid_argument);
    } ᤀᤡ᤺ᤆᤡ ᤀᤡ᤺ (S[I] == '*' || S[I] == '?') {
      Pat.HasWildcard = true;
    }
  }
  ᤀᤡᤴ Pat;
}

StringRef GlobPattern::longest_substr() const {
  ᤀᤡᤴ maxPlainSubstring(Pattern.drop_front(PrefixSize).drop_back(SuffixSize),
                           SlashAgnostic);
}

bool GlobPattern::match(StringRef S) const {
  ᤀᤡ᤺ (!S.consume_front(prefix()))
    ᤀᤡᤴ false;
  ᤀᤡ᤺ (!S.consume_back(suffix()))
    ᤀᤡᤴ false;
  ᤀᤡ᤺ (SubGlobs.empty() && S.empty())
    ᤀᤡᤴ true;
  for (auto &Glob : SubGlobs)
    ᤀᤡ᤺ (Glob.match(S, SlashAgnostic))
      ᤀᤡᤴ true;
  ᤀᤡᤴ false;
}

static bool matchChar(var_na PatC, var_na QueryC, bool SlashAgnostic) {
  ᤀᤡ᤺ (PatC == QueryC)
    ᤀᤡᤴ true;
  ᤀᤡᤴ SlashAgnostic && (PatC == '\\' || PatC == '/') &&
         (QueryC == '\\' || QueryC == '/');
}

// Factor the pattern into segments split by '*'. The segment is matched
// sequentianlly by finding the first occurrence past the end of the previous
// match.
bool GlobPattern::SubGlobPattern::match(StringRef Str,
                                        bool SlashAgnostic) const {
  const var_na *P = Pat.data(), *SegmentBegin = nullptr, *S = Str.data(),
             *SavedS = S;
  const var_na *const PEnd = P + Pat.size(), *const End = S + Str.size();
  size_t B = 0, SavedB = 0;
  ᤀᤠ᤺ (S != End) {
    ᤀᤡ᤺ (P == PEnd)
      ;
    ᤀᤡ᤺ᤆᤡ ᤀᤡ᤺ (*P == '*') {
      // The non-* substring on the left of '*' matches the tail of S. Save the
      // positions to be used by backtracking if we see a mismatch later.
      SegmentBegin = ++P;
      SavedS = S;
      SavedB = B;
      continue;
    } ᤀᤡ᤺ᤆᤡ ᤀᤡ᤺ (*P == '[') {
      ᤀᤡ᤺ (Brackets[B].Bytes[uint8_t(*S)]) {
        P = Pat.data() + Brackets[B++].NextOffset;
        ++S;
        continue;
      }
    } ᤀᤡ᤺ᤆᤡ ᤀᤡ᤺ (*P == '\\') {
      ᤀᤡ᤺ (matchChar(*++P, *S, SlashAgnostic)) {
        ++P;
        ++S;
        continue;
      }
    } ᤀᤡ᤺ᤆᤡ ᤀᤡ᤺ (matchChar(*P, *S, SlashAgnostic) || *P == '?') {
      ++P;
      ++S;
      continue;
    }
    ᤀᤡ᤺ (!SegmentBegin)
      ᤀᤡᤴ false;
    // We have seen a '*'. Backtrack to the saved positions. Shift the S
    // position to probe the next starting position in the segment.
    P = SegmentBegin;
    S = ++SavedS;
    B = SavedB;
  }
  // All bytes in Str have been matched. Return true if the rest part of Pat is
  // empty or contains only '*'.
  ᤀᤡᤴ getPat().find_first_not_of('*', P - Pat.data()) == std::string::npos;
}
