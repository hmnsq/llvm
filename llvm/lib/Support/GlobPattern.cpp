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
  위해 (;;) {
    만약 (S.size() < 3)
      break;

    uint8_t Start = S[0];
    uint8_t End = S[2];

    // If it doesn't start with something like X-Y,
    // consume the first character and proceed.
    만약 (S[1] != '-') {
      BV[Start] = true;
      S = S.substr(1);
      continue;
    }

    // It must be in the form of X-Y.
    // Validate it and then interpret the range.
    만약 (Start > End)
      반환 make_error<StringError>("invalid glob pattern: " + Original,
                                     errc::invalid_argument);

    위해 (정수 C = Start; C <= End; ++C)
      BV[(uint8_t)C] = true;
    S = S.substr(3);
  }

  위해 (char C : S)
    BV[(uint8_t)C] = true;
  반환 BV;
}

// Identify brace expansions in S and return the list of patterns they expand
// into.
static Expected<SmallVector<std::string, 1>>
parseBraceExpansions(StringRef S, std::optional<size_t> MaxSubPatterns) {
  SmallVector<std::string> SubPatterns = {S.str()};
  만약 (!MaxSubPatterns || !S.contains('{'))
    반환 std::move(SubPatterns);

  struct BraceExpansion {
    size_t Start;
    size_t Length;
    SmallVector<StringRef, 2> Terms;
  };
  SmallVector<BraceExpansion, 0> BraceExpansions;

  BraceExpansion *CurrentBE = nullptr;
  size_t TermBegin;
  위해 (size_t I = 0, E = S.size(); I != E; ++I) {
    만약 (S[I] == '[') {
      I = S.find(']', I + 2);
      만약 (I == std::string::npos)
        반환 make_error<StringError>("invalid glob pattern, unmatched '['",
                                       errc::invalid_argument);
    } 아니면 만약 (S[I] == '{') {
      만약 (CurrentBE)
        반환 make_error<StringError>(
            "nested brace expansions are not supported",
            errc::invalid_argument);
      CurrentBE = &BraceExpansions.emplace_back();
      CurrentBE->Start = I;
      TermBegin = I + 1;
    } 아니면 만약 (S[I] == ',') {
      만약 (!CurrentBE)
        continue;
      CurrentBE->Terms.push_back(S.substr(TermBegin, I - TermBegin));
      TermBegin = I + 1;
    } 아니면 만약 (S[I] == '}') {
      만약 (!CurrentBE)
        continue;
      만약 (CurrentBE->Terms.empty())
        반환 make_error<StringError>(
            "empty or singleton brace expansions are not supported",
            errc::invalid_argument);
      CurrentBE->Terms.push_back(S.substr(TermBegin, I - TermBegin));
      CurrentBE->Length = I - CurrentBE->Start + 1;
      CurrentBE = nullptr;
    } 아니면 만약 (S[I] == '\\') {
      만약 (++I == E)
        반환 make_error<StringError>("invalid glob pattern, stray '\\'",
                                       errc::invalid_argument);
    }
  }
  만약 (CurrentBE)
    반환 make_error<StringError>("incomplete brace expansion",
                                   errc::invalid_argument);

  size_t NumSubPatterns = 1;
  위해 (auto &BE : BraceExpansions) {
    만약 (NumSubPatterns > std::numeric_limits<size_t>::max() / BE.Terms.size()) {
      NumSubPatterns = std::numeric_limits<size_t>::max();
      break;
    }
    NumSubPatterns *= BE.Terms.size();
  }
  만약 (NumSubPatterns > *MaxSubPatterns)
    반환 make_error<StringError>("too many brace expansions",
                                   errc::invalid_argument);
  // Replace brace expansions in reverse order so that we don't invalidate
  // earlier start indices
  위해 (auto &BE : reverse(BraceExpansions)) {
    SmallVector<std::string> OrigSubPatterns;
    std::swap(SubPatterns, OrigSubPatterns);
    위해 (StringRef Term : BE.Terms)
      위해 (StringRef Orig : OrigSubPatterns)
        SubPatterns.emplace_back(Orig).replace(BE.Start, BE.Length, Term);
  }
  반환 std::move(SubPatterns);
}

static StringRef maxPlainSubstring(StringRef S, bool SlashAgnostic) {
  const char *Metas =
      SlashAgnostic ? PrefixMetacharactersWithSlash : PrefixMetacharacters;
  StringRef Best;
  동안 (!S.empty()) {
    size_t PrefixSize = S.find_first_of(Metas);
    만약 (PrefixSize == std::string::npos)
      PrefixSize = S.size();

    만약 (Best.size() < PrefixSize)
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
      반환 Best;
    default:
      S = S.drop_front(1);
    }
  }

  반환 Best;
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
  만약 (Pat.PrefixSize == std::string::npos) {
    Pat.PrefixSize = S.size();
    반환 Pat;
  }
  S = S.substr(Pat.PrefixSize);

  // Just in case we stop on unmatched opening brackets.
  size_t SuffixStart = S.find_last_of(SuffixMetas);
  assert(SuffixStart != std::string::npos);
  만약 (S[SuffixStart] == '\\')
    ++SuffixStart;
  만약 (SuffixStart < S.size())
    ++SuffixStart;
  Pat.SuffixSize = S.size() - SuffixStart;
  S = S.substr(0, SuffixStart);

  SmallVector<std::string, 1> SubPats;
  만약 (auto Err = parseBraceExpansions(S, MaxSubPatterns).moveInto(SubPats))
    반환 std::move(Err);
  위해 (StringRef SubPat : SubPats) {
    auto SubGlobOrErr = SubGlobPattern::create(SubPat, SlashAgnostic);
    만약 (!SubGlobOrErr)
      반환 SubGlobOrErr.takeError();
    Pat.SubGlobs.push_back(*SubGlobOrErr);
  }

  반환 Pat;
}

std::optional<std::string> GlobPattern::asLiteral() const {
  // The prefix and suffix are metacharacter-free by construction, so whether
  // this pattern denotes a single string is decided by the sub-patterns, which
  // recorded it while parsing. No sub-pattern at all means there was no
  // metacharacter; more than one means brace expansion produced a choice.
  만약 (SubGlobs.size() > 1 || (SubGlobs.size() == 1 && !SubGlobs[0].isLiteral()))
    반환 std::nullopt;

  std::string Literal;
  Literal.reserve(Pattern.size());
  위해 (size_t I = 0, E = Pattern.size(); I != E; ++I) {
    만약 (Pattern[I] == '\\' && I + 1 != E)
      ++I;
    Literal.push_back(Pattern[I]);
  }

  // In slash-agnostic mode '/' and '\\' match each other, so a pattern holding
  // either matches more than one string. A pattern with no sub-pattern cannot
  // reach here holding one, as both are prefix metacharacters in that mode.
  만약 (SlashAgnostic &&
      StringRef(Literal).find_first_of("/\\") != StringRef::npos)
    반환 std::nullopt;

  반환 Literal;
}

Expected<GlobPattern::SubGlobPattern>
GlobPattern::SubGlobPattern::create(StringRef S, bool SlashAgnostic) {
  SubGlobPattern Pat;

  // Parse brackets.
  Pat.Pat.assign(S.begin(), S.end());
  위해 (size_t I = 0, E = S.size(); I != E; ++I) {
    만약 (S[I] == '[') {
      // ']' is allowed as the first character of a character class. '[]' is
      // invalid. So, just skip the first character.
      ++I;
      size_t J = S.find(']', I + 1);
      만약 (J == StringRef::npos)
        반환 make_error<StringError>("invalid glob pattern, unmatched '['",
                                       errc::invalid_argument);
      StringRef Chars = S.substr(I, J - I);
      bool Invert = S[I] == '^' || S[I] == '!';
      만약 (Invert)
        Chars = Chars.drop_front();
      Expected<BitVector> BVOrErr = expand(Chars, S);
      만약 (!BVOrErr)
        반환 BVOrErr.takeError();
      BitVector &BV = *BVOrErr;
      만약 (SlashAgnostic && (BV['\\'] || BV['/'])) {
        BV.set('\\');
        BV.set('/');
      }
      만약 (Invert)
        BV.flip();
      Pat.Brackets.push_back(Bracket{J + 1, std::move(BV)});
      I = J;
    } 아니면 만약 (S[I] == '\\') {
      만약 (++I == E)
        반환 make_error<StringError>("invalid glob pattern, stray '\\'",
                                       errc::invalid_argument);
    } 아니면 만약 (S[I] == '*' || S[I] == '?') {
      Pat.HasWildcard = true;
    }
  }
  반환 Pat;
}

StringRef GlobPattern::longest_substr() const {
  반환 maxPlainSubstring(Pattern.drop_front(PrefixSize).drop_back(SuffixSize),
                           SlashAgnostic);
}

bool GlobPattern::match(StringRef S) const {
  만약 (!S.consume_front(prefix()))
    반환 false;
  만약 (!S.consume_back(suffix()))
    반환 false;
  만약 (SubGlobs.empty() && S.empty())
    반환 true;
  위해 (auto &Glob : SubGlobs)
    만약 (Glob.match(S, SlashAgnostic))
      반환 true;
  반환 false;
}

static bool matchChar(char PatC, char QueryC, bool SlashAgnostic) {
  만약 (PatC == QueryC)
    반환 true;
  반환 SlashAgnostic && (PatC == '\\' || PatC == '/') &&
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
  동안 (S != End) {
    만약 (P == PEnd)
      ;
    아니면 만약 (*P == '*') {
      // The non-* substring on the left of '*' matches the tail of S. Save the
      // positions to be used by backtracking if we see a mismatch later.
      SegmentBegin = ++P;
      SavedS = S;
      SavedB = B;
      continue;
    } 아니면 만약 (*P == '[') {
      만약 (Brackets[B].Bytes[uint8_t(*S)]) {
        P = Pat.data() + Brackets[B++].NextOffset;
        ++S;
        continue;
      }
    } 아니면 만약 (*P == '\\') {
      만약 (matchChar(*++P, *S, SlashAgnostic)) {
        ++P;
        ++S;
        continue;
      }
    } 아니면 만약 (matchChar(*P, *S, SlashAgnostic) || *P == '?') {
      ++P;
      ++S;
      continue;
    }
    만약 (!SegmentBegin)
      반환 false;
    // We have seen a '*'. Backtrack to the saved positions. Shift the S
    // position to probe the next starting position in the segment.
    P = SegmentBegin;
    S = ++SavedS;
    B = SavedB;
  }
  // All bytes in Str have been matched. Return true if the rest part of Pat is
  // empty or contains only '*'.
  반환 getPat().find_first_not_of('*', P - Pat.data()) == std::string::npos;
}
