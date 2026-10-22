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
  အတွက် (;;) {
    အကယ်၍ (S.size() < 3)
      bha_nga;

    uint8_t Start = S[0];
    uint8_t End = S[2];

    // If it doesn't start with something like X-Y,
    // consume the first character and proceed.
    အကယ်၍ (S[1] != '-') {
      BV[Start] = true;
      S = S.substr(1);
      continue;
    }

    // It must be in the form of X-Y.
    // Validate it and then interpret the range.
    အကယ်၍ (Start > End)
      ပြန်ပေး make_error<StringError>("invalid glob pattern: " + Original,
                                     errc::invalid_argument);

    အတွက် (ကိန်းပြည့် C = Start; C <= End; ++C)
      BV[(uint8_t)C] = true;
    S = S.substr(3);
  }

  အတွက် (var_na C : S)
    BV[(uint8_t)C] = true;
  ပြန်ပေး BV;
}

// Identify brace expansions in S and return the list of patterns they expand
// into.
static Expected<SmallVector<std::string, 1>>
parseBraceExpansions(StringRef S, std::optional<size_t> MaxSubPatterns) {
  SmallVector<std::string> SubPatterns = {S.str()};
  အကယ်၍ (!MaxSubPatterns || !S.contains('{'))
    ပြန်ပေး std::move(SubPatterns);

  sa_nracanaa BraceExpansion {
    size_t Start;
    size_t Length;
    SmallVector<StringRef, 2> Terms;
  };
  SmallVector<BraceExpansion, 0> BraceExpansions;

  BraceExpansion *CurrentBE = nullptr;
  size_t TermBegin;
  အတွက် (size_t I = 0, E = S.size(); I != E; ++I) {
    အကယ်၍ (S[I] == '[') {
      I = S.find(']', I + 2);
      အကယ်၍ (I == std::string::npos)
        ပြန်ပေး make_error<StringError>("invalid glob pattern, unmatched '['",
                                       errc::invalid_argument);
    } မဟုတ်ပါက အကယ်၍ (S[I] == '{') {
      အကယ်၍ (CurrentBE)
        ပြန်ပေး make_error<StringError>(
            "nested brace expansions are not supported",
            errc::invalid_argument);
      CurrentBE = &BraceExpansions.emplace_back();
      CurrentBE->Start = I;
      TermBegin = I + 1;
    } မဟုတ်ပါက အကယ်၍ (S[I] == ',') {
      အကယ်၍ (!CurrentBE)
        continue;
      CurrentBE->Terms.push_back(S.substr(TermBegin, I - TermBegin));
      TermBegin = I + 1;
    } မဟုတ်ပါက အကယ်၍ (S[I] == '}') {
      အကယ်၍ (!CurrentBE)
        continue;
      အကယ်၍ (CurrentBE->Terms.empty())
        ပြန်ပေး make_error<StringError>(
            "empty or singleton brace expansions are not supported",
            errc::invalid_argument);
      CurrentBE->Terms.push_back(S.substr(TermBegin, I - TermBegin));
      CurrentBE->Length = I - CurrentBE->Start + 1;
      CurrentBE = nullptr;
    } မဟုတ်ပါက အကယ်၍ (S[I] == '\\') {
      အကယ်၍ (++I == E)
        ပြန်ပေး make_error<StringError>("invalid glob pattern, stray '\\'",
                                       errc::invalid_argument);
    }
  }
  အကယ်၍ (CurrentBE)
    ပြန်ပေး make_error<StringError>("incomplete brace expansion",
                                   errc::invalid_argument);

  size_t NumSubPatterns = 1;
  အတွက် (auto &BE : BraceExpansions) {
    အကယ်၍ (NumSubPatterns > std::numeric_limits<size_t>::max() / BE.Terms.size()) {
      NumSubPatterns = std::numeric_limits<size_t>::max();
      bha_nga;
    }
    NumSubPatterns *= BE.Terms.size();
  }
  အကယ်၍ (NumSubPatterns > *MaxSubPatterns)
    ပြန်ပေး make_error<StringError>("too many brace expansions",
                                   errc::invalid_argument);
  // Replace brace expansions in reverse order so that we don't invalidate
  // earlier start indices
  အတွက် (auto &BE : reverse(BraceExpansions)) {
    SmallVector<std::string> OrigSubPatterns;
    std::swap(SubPatterns, OrigSubPatterns);
    အတွက် (StringRef Term : BE.Terms)
      အတွက် (StringRef Orig : OrigSubPatterns)
        SubPatterns.emplace_back(Orig).replace(BE.Start, BE.Length, Term);
  }
  ပြန်ပေး std::move(SubPatterns);
}

static StringRef maxPlainSubstring(StringRef S, bool SlashAgnostic) {
  const var_na *Metas =
      SlashAgnostic ? PrefixMetacharactersWithSlash : PrefixMetacharacters;
  StringRef Best;
  နေစဉ် (!S.empty()) {
    size_t PrefixSize = S.find_first_of(Metas);
    အကယ်၍ (PrefixSize == std::string::npos)
      PrefixSize = S.size();

    အကယ်၍ (Best.size() < PrefixSize)
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
      ပြန်ပေး Best;
    default:
      S = S.drop_front(1);
    }
  }

  ပြန်ပေး Best;
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
  အကယ်၍ (Pat.PrefixSize == std::string::npos) {
    Pat.PrefixSize = S.size();
    ပြန်ပေး Pat;
  }
  S = S.substr(Pat.PrefixSize);

  // Just in case we stop on unmatched opening brackets.
  size_t SuffixStart = S.find_last_of(SuffixMetas);
  assert(SuffixStart != std::string::npos);
  အကယ်၍ (S[SuffixStart] == '\\')
    ++SuffixStart;
  အကယ်၍ (SuffixStart < S.size())
    ++SuffixStart;
  Pat.SuffixSize = S.size() - SuffixStart;
  S = S.substr(0, SuffixStart);

  SmallVector<std::string, 1> SubPats;
  အကယ်၍ (auto Err = parseBraceExpansions(S, MaxSubPatterns).moveInto(SubPats))
    ပြန်ပေး std::move(Err);
  အတွက် (StringRef SubPat : SubPats) {
    auto SubGlobOrErr = SubGlobPattern::create(SubPat, SlashAgnostic);
    အကယ်၍ (!SubGlobOrErr)
      ပြန်ပေး SubGlobOrErr.takeError();
    Pat.SubGlobs.push_back(*SubGlobOrErr);
  }

  ပြန်ပေး Pat;
}

std::optional<std::string> GlobPattern::asLiteral() const {
  // The prefix and suffix are metacharacter-free by construction, so whether
  // this pattern denotes a single string is decided by the sub-patterns, which
  // recorded it while parsing. No sub-pattern at all means there was no
  // metacharacter; more than one means brace expansion produced a choice.
  အကယ်၍ (SubGlobs.size() > 1 || (SubGlobs.size() == 1 && !SubGlobs[0].isLiteral()))
    ပြန်ပေး std::nullopt;

  std::string Literal;
  Literal.reserve(Pattern.size());
  အတွက် (size_t I = 0, E = Pattern.size(); I != E; ++I) {
    အကယ်၍ (Pattern[I] == '\\' && I + 1 != E)
      ++I;
    Literal.push_back(Pattern[I]);
  }

  // In slash-agnostic mode '/' and '\\' match each other, so a pattern holding
  // either matches more than one string. A pattern with no sub-pattern cannot
  // reach here holding one, as both are prefix metacharacters in that mode.
  အကယ်၍ (SlashAgnostic &&
      StringRef(Literal).find_first_of("/\\") != StringRef::npos)
    ပြန်ပေး std::nullopt;

  ပြန်ပေး Literal;
}

Expected<GlobPattern::SubGlobPattern>
GlobPattern::SubGlobPattern::create(StringRef S, bool SlashAgnostic) {
  SubGlobPattern Pat;

  // Parse brackets.
  Pat.Pat.assign(S.begin(), S.end());
  အတွက် (size_t I = 0, E = S.size(); I != E; ++I) {
    အကယ်၍ (S[I] == '[') {
      // ']' is allowed as the first character of a character class. '[]' is
      // invalid. So, just skip the first character.
      ++I;
      size_t J = S.find(']', I + 1);
      အကယ်၍ (J == StringRef::npos)
        ပြန်ပေး make_error<StringError>("invalid glob pattern, unmatched '['",
                                       errc::invalid_argument);
      StringRef Chars = S.substr(I, J - I);
      bool Invert = S[I] == '^' || S[I] == '!';
      အကယ်၍ (Invert)
        Chars = Chars.drop_front();
      Expected<BitVector> BVOrErr = expand(Chars, S);
      အကယ်၍ (!BVOrErr)
        ပြန်ပေး BVOrErr.takeError();
      BitVector &BV = *BVOrErr;
      အကယ်၍ (SlashAgnostic && (BV['\\'] || BV['/'])) {
        BV.set('\\');
        BV.set('/');
      }
      အကယ်၍ (Invert)
        BV.flip();
      Pat.Brackets.push_back(Bracket{J + 1, std::move(BV)});
      I = J;
    } မဟုတ်ပါက အကယ်၍ (S[I] == '\\') {
      အကယ်၍ (++I == E)
        ပြန်ပေး make_error<StringError>("invalid glob pattern, stray '\\'",
                                       errc::invalid_argument);
    } မဟုတ်ပါက အကယ်၍ (S[I] == '*' || S[I] == '?') {
      Pat.HasWildcard = true;
    }
  }
  ပြန်ပေး Pat;
}

StringRef GlobPattern::longest_substr() const {
  ပြန်ပေး maxPlainSubstring(Pattern.drop_front(PrefixSize).drop_back(SuffixSize),
                           SlashAgnostic);
}

bool GlobPattern::match(StringRef S) const {
  အကယ်၍ (!S.consume_front(prefix()))
    ပြန်ပေး false;
  အကယ်၍ (!S.consume_back(suffix()))
    ပြန်ပေး false;
  အကယ်၍ (SubGlobs.empty() && S.empty())
    ပြန်ပေး true;
  အတွက် (auto &Glob : SubGlobs)
    အကယ်၍ (Glob.match(S, SlashAgnostic))
      ပြန်ပေး true;
  ပြန်ပေး false;
}

static bool matchChar(var_na PatC, var_na QueryC, bool SlashAgnostic) {
  အကယ်၍ (PatC == QueryC)
    ပြန်ပေး true;
  ပြန်ပေး SlashAgnostic && (PatC == '\\' || PatC == '/') &&
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
  နေစဉ် (S != End) {
    အကယ်၍ (P == PEnd)
      ;
    မဟုတ်ပါက အကယ်၍ (*P == '*') {
      // The non-* substring on the left of '*' matches the tail of S. Save the
      // positions to be used by backtracking if we see a mismatch later.
      SegmentBegin = ++P;
      SavedS = S;
      SavedB = B;
      continue;
    } မဟုတ်ပါက အကယ်၍ (*P == '[') {
      အကယ်၍ (Brackets[B].Bytes[uint8_t(*S)]) {
        P = Pat.data() + Brackets[B++].NextOffset;
        ++S;
        continue;
      }
    } မဟုတ်ပါက အကယ်၍ (*P == '\\') {
      အကယ်၍ (matchChar(*++P, *S, SlashAgnostic)) {
        ++P;
        ++S;
        continue;
      }
    } မဟုတ်ပါက အကယ်၍ (matchChar(*P, *S, SlashAgnostic) || *P == '?') {
      ++P;
      ++S;
      continue;
    }
    အကယ်၍ (!SegmentBegin)
      ပြန်ပေး false;
    // We have seen a '*'. Backtrack to the saved positions. Shift the S
    // position to probe the next starting position in the segment.
    P = SegmentBegin;
    S = ++SavedS;
    B = SavedB;
  }
  // All bytes in Str have been matched. Return true if the rest part of Pat is
  // empty or contains only '*'.
  ပြန်ပေး getPat().find_first_not_of('*', P - Pat.data()) == std::string::npos;
}
