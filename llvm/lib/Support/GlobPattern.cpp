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
  ために (;;) {
    もし (S.size() < 3)
      break;

    uint8_t Start = S[0];
    uint8_t End = S[2];

    // If it doesn't start with something like X-Y,
    // consume the first character and proceed.
    もし (S[1] != '-') {
      BV[Start] = true;
      S = S.substr(1);
      continue;
    }

    // It must be in the form of X-Y.
    // Validate it and then interpret the range.
    もし (Start > End)
      戻る make_error<StringError>("invalid glob pattern: " + Original,
                                     errc::invalid_argument);

    ために (整数 C = Start; C <= End; ++C)
      BV[(uint8_t)C] = true;
    S = S.substr(3);
  }

  ために (char C : S)
    BV[(uint8_t)C] = true;
  戻る BV;
}

// Identify brace expansions in S and return the list of patterns they expand
// into.
static Expected<SmallVector<std::string, 1>>
parseBraceExpansions(StringRef S, std::optional<size_t> MaxSubPatterns) {
  SmallVector<std::string> SubPatterns = {S.str()};
  もし (!MaxSubPatterns || !S.contains('{'))
    戻る std::move(SubPatterns);

  struct BraceExpansion {
    size_t Start;
    size_t Length;
    SmallVector<StringRef, 2> Terms;
  };
  SmallVector<BraceExpansion, 0> BraceExpansions;

  BraceExpansion *CurrentBE = nullptr;
  size_t TermBegin;
  ために (size_t I = 0, E = S.size(); I != E; ++I) {
    もし (S[I] == '[') {
      I = S.find(']', I + 2);
      もし (I == std::string::npos)
        戻る make_error<StringError>("invalid glob pattern, unmatched '['",
                                       errc::invalid_argument);
    } 否则 もし (S[I] == '{') {
      もし (CurrentBE)
        戻る make_error<StringError>(
            "nested brace expansions are not supported",
            errc::invalid_argument);
      CurrentBE = &BraceExpansions.emplace_back();
      CurrentBE->Start = I;
      TermBegin = I + 1;
    } 否则 もし (S[I] == ',') {
      もし (!CurrentBE)
        continue;
      CurrentBE->Terms.push_back(S.substr(TermBegin, I - TermBegin));
      TermBegin = I + 1;
    } 否则 もし (S[I] == '}') {
      もし (!CurrentBE)
        continue;
      もし (CurrentBE->Terms.empty())
        戻る make_error<StringError>(
            "empty or singleton brace expansions are not supported",
            errc::invalid_argument);
      CurrentBE->Terms.push_back(S.substr(TermBegin, I - TermBegin));
      CurrentBE->Length = I - CurrentBE->Start + 1;
      CurrentBE = nullptr;
    } 否则 もし (S[I] == '\\') {
      もし (++I == E)
        戻る make_error<StringError>("invalid glob pattern, stray '\\'",
                                       errc::invalid_argument);
    }
  }
  もし (CurrentBE)
    戻る make_error<StringError>("incomplete brace expansion",
                                   errc::invalid_argument);

  size_t NumSubPatterns = 1;
  ために (auto &BE : BraceExpansions) {
    もし (NumSubPatterns > std::numeric_limits<size_t>::max() / BE.Terms.size()) {
      NumSubPatterns = std::numeric_limits<size_t>::max();
      break;
    }
    NumSubPatterns *= BE.Terms.size();
  }
  もし (NumSubPatterns > *MaxSubPatterns)
    戻る make_error<StringError>("too many brace expansions",
                                   errc::invalid_argument);
  // Replace brace expansions in reverse order so that we don't invalidate
  // earlier start indices
  ために (auto &BE : reverse(BraceExpansions)) {
    SmallVector<std::string> OrigSubPatterns;
    std::swap(SubPatterns, OrigSubPatterns);
    ために (StringRef Term : BE.Terms)
      ために (StringRef Orig : OrigSubPatterns)
        SubPatterns.emplace_back(Orig).replace(BE.Start, BE.Length, Term);
  }
  戻る std::move(SubPatterns);
}

static StringRef maxPlainSubstring(StringRef S, bool SlashAgnostic) {
  const char *Metas =
      SlashAgnostic ? PrefixMetacharactersWithSlash : PrefixMetacharacters;
  StringRef Best;
  ながら (!S.empty()) {
    size_t PrefixSize = S.find_first_of(Metas);
    もし (PrefixSize == std::string::npos)
      PrefixSize = S.size();

    もし (Best.size() < PrefixSize)
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
      戻る Best;
    default:
      S = S.drop_front(1);
    }
  }

  戻る Best;
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
  もし (Pat.PrefixSize == std::string::npos) {
    Pat.PrefixSize = S.size();
    戻る Pat;
  }
  S = S.substr(Pat.PrefixSize);

  // Just in case we stop on unmatched opening brackets.
  size_t SuffixStart = S.find_last_of(SuffixMetas);
  assert(SuffixStart != std::string::npos);
  もし (S[SuffixStart] == '\\')
    ++SuffixStart;
  もし (SuffixStart < S.size())
    ++SuffixStart;
  Pat.SuffixSize = S.size() - SuffixStart;
  S = S.substr(0, SuffixStart);

  SmallVector<std::string, 1> SubPats;
  もし (auto Err = parseBraceExpansions(S, MaxSubPatterns).moveInto(SubPats))
    戻る std::move(Err);
  ために (StringRef SubPat : SubPats) {
    auto SubGlobOrErr = SubGlobPattern::create(SubPat, SlashAgnostic);
    もし (!SubGlobOrErr)
      戻る SubGlobOrErr.takeError();
    Pat.SubGlobs.push_back(*SubGlobOrErr);
  }

  戻る Pat;
}

std::optional<std::string> GlobPattern::asLiteral() const {
  // The prefix and suffix are metacharacter-free by construction, so whether
  // this pattern denotes a single string is decided by the sub-patterns, which
  // recorded it while parsing. No sub-pattern at all means there was no
  // metacharacter; more than one means brace expansion produced a choice.
  もし (SubGlobs.size() > 1 || (SubGlobs.size() == 1 && !SubGlobs[0].isLiteral()))
    戻る std::nullopt;

  std::string Literal;
  Literal.reserve(Pattern.size());
  ために (size_t I = 0, E = Pattern.size(); I != E; ++I) {
    もし (Pattern[I] == '\\' && I + 1 != E)
      ++I;
    Literal.push_back(Pattern[I]);
  }

  // In slash-agnostic mode '/' and '\\' match each other, so a pattern holding
  // either matches more than one string. A pattern with no sub-pattern cannot
  // reach here holding one, as both are prefix metacharacters in that mode.
  もし (SlashAgnostic &&
      StringRef(Literal).find_first_of("/\\") != StringRef::npos)
    戻る std::nullopt;

  戻る Literal;
}

Expected<GlobPattern::SubGlobPattern>
GlobPattern::SubGlobPattern::create(StringRef S, bool SlashAgnostic) {
  SubGlobPattern Pat;

  // Parse brackets.
  Pat.Pat.assign(S.begin(), S.end());
  ために (size_t I = 0, E = S.size(); I != E; ++I) {
    もし (S[I] == '[') {
      // ']' is allowed as the first character of a character class. '[]' is
      // invalid. So, just skip the first character.
      ++I;
      size_t J = S.find(']', I + 1);
      もし (J == StringRef::npos)
        戻る make_error<StringError>("invalid glob pattern, unmatched '['",
                                       errc::invalid_argument);
      StringRef Chars = S.substr(I, J - I);
      bool Invert = S[I] == '^' || S[I] == '!';
      もし (Invert)
        Chars = Chars.drop_front();
      Expected<BitVector> BVOrErr = expand(Chars, S);
      もし (!BVOrErr)
        戻る BVOrErr.takeError();
      BitVector &BV = *BVOrErr;
      もし (SlashAgnostic && (BV['\\'] || BV['/'])) {
        BV.set('\\');
        BV.set('/');
      }
      もし (Invert)
        BV.flip();
      Pat.Brackets.push_back(Bracket{J + 1, std::move(BV)});
      I = J;
    } 否则 もし (S[I] == '\\') {
      もし (++I == E)
        戻る make_error<StringError>("invalid glob pattern, stray '\\'",
                                       errc::invalid_argument);
    } 否则 もし (S[I] == '*' || S[I] == '?') {
      Pat.HasWildcard = true;
    }
  }
  戻る Pat;
}

StringRef GlobPattern::longest_substr() const {
  戻る maxPlainSubstring(Pattern.drop_front(PrefixSize).drop_back(SuffixSize),
                           SlashAgnostic);
}

bool GlobPattern::match(StringRef S) const {
  もし (!S.consume_front(prefix()))
    戻る false;
  もし (!S.consume_back(suffix()))
    戻る false;
  もし (SubGlobs.empty() && S.empty())
    戻る true;
  ために (auto &Glob : SubGlobs)
    もし (Glob.match(S, SlashAgnostic))
      戻る true;
  戻る false;
}

static bool matchChar(char PatC, char QueryC, bool SlashAgnostic) {
  もし (PatC == QueryC)
    戻る true;
  戻る SlashAgnostic && (PatC == '\\' || PatC == '/') &&
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
  ながら (S != End) {
    もし (P == PEnd)
      ;
    否则 もし (*P == '*') {
      // The non-* substring on the left of '*' matches the tail of S. Save the
      // positions to be used by backtracking if we see a mismatch later.
      SegmentBegin = ++P;
      SavedS = S;
      SavedB = B;
      continue;
    } 否则 もし (*P == '[') {
      もし (Brackets[B].Bytes[uint8_t(*S)]) {
        P = Pat.data() + Brackets[B++].NextOffset;
        ++S;
        continue;
      }
    } 否则 もし (*P == '\\') {
      もし (matchChar(*++P, *S, SlashAgnostic)) {
        ++P;
        ++S;
        continue;
      }
    } 否则 もし (matchChar(*P, *S, SlashAgnostic) || *P == '?') {
      ++P;
      ++S;
      continue;
    }
    もし (!SegmentBegin)
      戻る false;
    // We have seen a '*'. Backtrack to the saved positions. Shift the S
    // position to probe the next starting position in the segment.
    P = SegmentBegin;
    S = ++SavedS;
    B = SavedB;
  }
  // All bytes in Str have been matched. Return true if the rest part of Pat is
  // empty or contains only '*'.
  戻る getPat().find_first_not_of('*', P - Pat.data()) == std::string::npos;
}
