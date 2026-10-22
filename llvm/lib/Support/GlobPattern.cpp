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

static constexpr char PrefixMetacharacters[] = "?*[{\\";
static constexpr char SuffixMetacharacters[] = "?*[]{}\\";
static constexpr char PrefixMetacharactersWithSlash[] = "?*[{\\/";
static constexpr char SuffixMetacharactersWithSlash[] = "?*[]{}\\/";

// Expands character ranges and returns a bitmap.
// For example, "a-cf-hz" is expanded to "abcfghz".
static Expected<BitVector> expand(StringRef S, StringRef Original) {
  BitVector BV(256, false);

  // Expand X-Y.
  cho (;;) {
    nếu (S.size() < 3)
      break;

    uint8_t Start = S[0];
    uint8_t End = S[2];

    // If it doesn't start with something like X-Y,
    // consume the first character and proceed.
    nếu (S[1] != '-') {
      BV[Start] = true;
      S = S.substr(1);
      continue;
    }

    // It must be in the form of X-Y.
    // Validate it and then interpret the range.
    nếu (Start > End)
      trả_về make_error<StringError>("invalid glob pattern: " + Original,
                                     errc::invalid_argument);

    cho (số_nguyên C = Start; C <= End; ++C)
      BV[(uint8_t)C] = true;
    S = S.substr(3);
  }

  cho (char C : S)
    BV[(uint8_t)C] = true;
  trả_về BV;
}

// Identify brace expansions in S and return the list of patterns they expand
// into.
static Expected<SmallVector<std::string, 1>>
parseBraceExpansions(StringRef S, std::optional<size_t> MaxSubPatterns) {
  SmallVector<std::string> SubPatterns = {S.str()};
  nếu (!MaxSubPatterns || !S.contains('{'))
    trả_về std::move(SubPatterns);

  struct BraceExpansion {
    size_t Start;
    size_t Length;
    SmallVector<StringRef, 2> Terms;
  };
  SmallVector<BraceExpansion, 0> BraceExpansions;

  BraceExpansion *CurrentBE = nullptr;
  size_t TermBegin;
  cho (size_t I = 0, E = S.size(); I != E; ++I) {
    nếu (S[I] == '[') {
      I = S.find(']', I + 2);
      nếu (I == std::string::npos)
        trả_về make_error<StringError>("invalid glob pattern, unmatched '['",
                                       errc::invalid_argument);
    } không_thì nếu (S[I] == '{') {
      nếu (CurrentBE)
        trả_về make_error<StringError>(
            "nested brace expansions are not supported",
            errc::invalid_argument);
      CurrentBE = &BraceExpansions.emplace_back();
      CurrentBE->Start = I;
      TermBegin = I + 1;
    } không_thì nếu (S[I] == ',') {
      nếu (!CurrentBE)
        continue;
      CurrentBE->Terms.push_back(S.substr(TermBegin, I - TermBegin));
      TermBegin = I + 1;
    } không_thì nếu (S[I] == '}') {
      nếu (!CurrentBE)
        continue;
      nếu (CurrentBE->Terms.empty())
        trả_về make_error<StringError>(
            "empty or singleton brace expansions are not supported",
            errc::invalid_argument);
      CurrentBE->Terms.push_back(S.substr(TermBegin, I - TermBegin));
      CurrentBE->Length = I - CurrentBE->Start + 1;
      CurrentBE = nullptr;
    } không_thì nếu (S[I] == '\\') {
      nếu (++I == E)
        trả_về make_error<StringError>("invalid glob pattern, stray '\\'",
                                       errc::invalid_argument);
    }
  }
  nếu (CurrentBE)
    trả_về make_error<StringError>("incomplete brace expansion",
                                   errc::invalid_argument);

  size_t NumSubPatterns = 1;
  cho (auto &BE : BraceExpansions) {
    nếu (NumSubPatterns > std::numeric_limits<size_t>::max() / BE.Terms.size()) {
      NumSubPatterns = std::numeric_limits<size_t>::max();
      break;
    }
    NumSubPatterns *= BE.Terms.size();
  }
  nếu (NumSubPatterns > *MaxSubPatterns)
    trả_về make_error<StringError>("too many brace expansions",
                                   errc::invalid_argument);
  // Replace brace expansions in reverse order so that we don't invalidate
  // earlier start indices
  cho (auto &BE : reverse(BraceExpansions)) {
    SmallVector<std::string> OrigSubPatterns;
    std::swap(SubPatterns, OrigSubPatterns);
    cho (StringRef Term : BE.Terms)
      cho (StringRef Orig : OrigSubPatterns)
        SubPatterns.emplace_back(Orig).replace(BE.Start, BE.Length, Term);
  }
  trả_về std::move(SubPatterns);
}

static StringRef maxPlainSubstring(StringRef S, bool SlashAgnostic) {
  const char *Metas =
      SlashAgnostic ? PrefixMetacharactersWithSlash : PrefixMetacharacters;
  StringRef Best;
  trong_khi (!S.empty()) {
    size_t PrefixSize = S.find_first_of(Metas);
    nếu (PrefixSize == std::string::npos)
      PrefixSize = S.size();

    nếu (Best.size() < PrefixSize)
      Best = S.take_front(PrefixSize);

    S = S.drop_front(PrefixSize);

    // It's impossible, as the first and last characters of the input string
    // must be Glob special characters, otherwise they would be parts of
    // the prefix or the suffix.
    assert(!S.empty());

    switch (S.front()) {
    case '\\':
      S = S.drop_front(2);
      break;
    case '[': {
      // Drop '[' and the first character which can be ']'.
      S = S.drop_front(2);
      size_t EndBracket = S.find_first_of("]");
      // Should not be possible, SubGlobPattern::create should fail on invalid
      // pattern before we get here.
      assert(EndBracket != std::string::npos);
      S = S.drop_front(EndBracket + 1);
      break;
    }
    case '{':
      // TODO: implement.
      // Fallback to whatever is best for now.
      trả_về Best;
    default:
      S = S.drop_front(1);
    }
  }

  trả_về Best;
}

Expected<GlobPattern> GlobPattern::create(StringRef S,
                                          std::optional<size_t> MaxSubPatterns,
                                          bool SlashAgnostic) {
  GlobPattern Pat;
  Pat.SlashAgnostic = SlashAgnostic;
  Pat.Pattern = S;

  const char *PrefixMetas =
      SlashAgnostic ? PrefixMetacharactersWithSlash : PrefixMetacharacters;
  const char *SuffixMetas =
      SlashAgnostic ? SuffixMetacharactersWithSlash : SuffixMetacharacters;

  // Store the prefix that does not contain any metacharacter.
  Pat.PrefixSize = S.find_first_of(PrefixMetas);
  nếu (Pat.PrefixSize == std::string::npos) {
    Pat.PrefixSize = S.size();
    trả_về Pat;
  }
  S = S.substr(Pat.PrefixSize);

  // Just in case we stop on unmatched opening brackets.
  size_t SuffixStart = S.find_last_of(SuffixMetas);
  assert(SuffixStart != std::string::npos);
  nếu (S[SuffixStart] == '\\')
    ++SuffixStart;
  nếu (SuffixStart < S.size())
    ++SuffixStart;
  Pat.SuffixSize = S.size() - SuffixStart;
  S = S.substr(0, SuffixStart);

  SmallVector<std::string, 1> SubPats;
  nếu (auto Err = parseBraceExpansions(S, MaxSubPatterns).moveInto(SubPats))
    trả_về std::move(Err);
  cho (StringRef SubPat : SubPats) {
    auto SubGlobOrErr = SubGlobPattern::create(SubPat, SlashAgnostic);
    nếu (!SubGlobOrErr)
      trả_về SubGlobOrErr.takeError();
    Pat.SubGlobs.push_back(*SubGlobOrErr);
  }

  trả_về Pat;
}

std::optional<std::string> GlobPattern::asLiteral() const {
  // The prefix and suffix are metacharacter-free by construction, so whether
  // this pattern denotes a single string is decided by the sub-patterns, which
  // recorded it while parsing. No sub-pattern at all means there was no
  // metacharacter; more than one means brace expansion produced a choice.
  nếu (SubGlobs.size() > 1 || (SubGlobs.size() == 1 && !SubGlobs[0].isLiteral()))
    trả_về std::nullopt;

  std::string Literal;
  Literal.reserve(Pattern.size());
  cho (size_t I = 0, E = Pattern.size(); I != E; ++I) {
    nếu (Pattern[I] == '\\' && I + 1 != E)
      ++I;
    Literal.push_back(Pattern[I]);
  }

  // In slash-agnostic mode '/' and '\\' match each other, so a pattern holding
  // either matches more than one string. A pattern with no sub-pattern cannot
  // reach here holding one, as both are prefix metacharacters in that mode.
  nếu (SlashAgnostic &&
      StringRef(Literal).find_first_of("/\\") != StringRef::npos)
    trả_về std::nullopt;

  trả_về Literal;
}

Expected<GlobPattern::SubGlobPattern>
GlobPattern::SubGlobPattern::create(StringRef S, bool SlashAgnostic) {
  SubGlobPattern Pat;

  // Parse brackets.
  Pat.Pat.assign(S.begin(), S.end());
  cho (size_t I = 0, E = S.size(); I != E; ++I) {
    nếu (S[I] == '[') {
      // ']' is allowed as the first character of a character class. '[]' is
      // invalid. So, just skip the first character.
      ++I;
      size_t J = S.find(']', I + 1);
      nếu (J == StringRef::npos)
        trả_về make_error<StringError>("invalid glob pattern, unmatched '['",
                                       errc::invalid_argument);
      StringRef Chars = S.substr(I, J - I);
      bool Invert = S[I] == '^' || S[I] == '!';
      nếu (Invert)
        Chars = Chars.drop_front();
      Expected<BitVector> BVOrErr = expand(Chars, S);
      nếu (!BVOrErr)
        trả_về BVOrErr.takeError();
      BitVector &BV = *BVOrErr;
      nếu (SlashAgnostic && (BV['\\'] || BV['/'])) {
        BV.set('\\');
        BV.set('/');
      }
      nếu (Invert)
        BV.flip();
      Pat.Brackets.push_back(Bracket{J + 1, std::move(BV)});
      I = J;
    } không_thì nếu (S[I] == '\\') {
      nếu (++I == E)
        trả_về make_error<StringError>("invalid glob pattern, stray '\\'",
                                       errc::invalid_argument);
    } không_thì nếu (S[I] == '*' || S[I] == '?') {
      Pat.HasWildcard = true;
    }
  }
  trả_về Pat;
}

StringRef GlobPattern::longest_substr() const {
  trả_về maxPlainSubstring(Pattern.drop_front(PrefixSize).drop_back(SuffixSize),
                           SlashAgnostic);
}

bool GlobPattern::match(StringRef S) const {
  nếu (!S.consume_front(prefix()))
    trả_về false;
  nếu (!S.consume_back(suffix()))
    trả_về false;
  nếu (SubGlobs.empty() && S.empty())
    trả_về true;
  cho (auto &Glob : SubGlobs)
    nếu (Glob.match(S, SlashAgnostic))
      trả_về true;
  trả_về false;
}

static bool matchChar(char PatC, char QueryC, bool SlashAgnostic) {
  nếu (PatC == QueryC)
    trả_về true;
  trả_về SlashAgnostic && (PatC == '\\' || PatC == '/') &&
         (QueryC == '\\' || QueryC == '/');
}

// Factor the pattern into segments split by '*'. The segment is matched
// sequentianlly by finding the first occurrence past the end of the previous
// match.
bool GlobPattern::SubGlobPattern::match(StringRef Str,
                                        bool SlashAgnostic) const {
  const char *P = Pat.data(), *SegmentBegin = nullptr, *S = Str.data(),
             *SavedS = S;
  const char *const PEnd = P + Pat.size(), *const End = S + Str.size();
  size_t B = 0, SavedB = 0;
  trong_khi (S != End) {
    nếu (P == PEnd)
      ;
    không_thì nếu (*P == '*') {
      // The non-* substring on the left of '*' matches the tail of S. Save the
      // positions to be used by backtracking if we see a mismatch later.
      SegmentBegin = ++P;
      SavedS = S;
      SavedB = B;
      continue;
    } không_thì nếu (*P == '[') {
      nếu (Brackets[B].Bytes[uint8_t(*S)]) {
        P = Pat.data() + Brackets[B++].NextOffset;
        ++S;
        continue;
      }
    } không_thì nếu (*P == '\\') {
      nếu (matchChar(*++P, *S, SlashAgnostic)) {
        ++P;
        ++S;
        continue;
      }
    } không_thì nếu (matchChar(*P, *S, SlashAgnostic) || *P == '?') {
      ++P;
      ++S;
      continue;
    }
    nếu (!SegmentBegin)
      trả_về false;
    // We have seen a '*'. Backtrack to the saved positions. Shift the S
    // position to probe the next starting position in the segment.
    P = SegmentBegin;
    S = ++SavedS;
    B = SavedB;
  }
  // All bytes in Str have been matched. Return true if the rest part of Pat is
  // empty or contains only '*'.
  trả_về getPat().find_first_not_of('*', P - Pat.data()) == std::string::npos;
}
