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
  ສຳລັບ (;;) {
    ຖ້າ (S.size() < 3)
      bha_nga;

    uint8_t Start = S[0];
    uint8_t End = S[2];

    // If it doesn't start with something like X-Y,
    // consume the first character and proceed.
    ຖ້າ (S[1] != '-') {
      BV[Start] = true;
      S = S.substr(1);
      continue;
    }

    // It must be in the form of X-Y.
    // Validate it and then interpret the range.
    ຖ້າ (Start > End)
      ສົ່ງຄືນ make_error<StringError>("invalid glob pattern: " + Original,
                                     errc::invalid_argument);

    ສຳລັບ (ຈຳນວນເຕັມ C = Start; C <= End; ++C)
      BV[(uint8_t)C] = true;
    S = S.substr(3);
  }

  ສຳລັບ (var_na C : S)
    BV[(uint8_t)C] = true;
  ສົ່ງຄືນ BV;
}

// Identify brace expansions in S and return the list of patterns they expand
// into.
static Expected<SmallVector<std::string, 1>>
parseBraceExpansions(StringRef S, std::optional<size_t> MaxSubPatterns) {
  SmallVector<std::string> SubPatterns = {S.str()};
  ຖ້າ (!MaxSubPatterns || !S.contains('{'))
    ສົ່ງຄືນ std::move(SubPatterns);

  sa_nracanaa BraceExpansion {
    size_t Start;
    size_t Length;
    SmallVector<StringRef, 2> Terms;
  };
  SmallVector<BraceExpansion, 0> BraceExpansions;

  BraceExpansion *CurrentBE = nullptr;
  size_t TermBegin;
  ສຳລັບ (size_t I = 0, E = S.size(); I != E; ++I) {
    ຖ້າ (S[I] == '[') {
      I = S.find(']', I + 2);
      ຖ້າ (I == std::string::npos)
        ສົ່ງຄືນ make_error<StringError>("invalid glob pattern, unmatched '['",
                                       errc::invalid_argument);
    } ບໍ່ດັ່ງນັ້ນ ຖ້າ (S[I] == '{') {
      ຖ້າ (CurrentBE)
        ສົ່ງຄືນ make_error<StringError>(
            "nested brace expansions are not supported",
            errc::invalid_argument);
      CurrentBE = &BraceExpansions.emplace_back();
      CurrentBE->Start = I;
      TermBegin = I + 1;
    } ບໍ່ດັ່ງນັ້ນ ຖ້າ (S[I] == ',') {
      ຖ້າ (!CurrentBE)
        continue;
      CurrentBE->Terms.push_back(S.substr(TermBegin, I - TermBegin));
      TermBegin = I + 1;
    } ບໍ່ດັ່ງນັ້ນ ຖ້າ (S[I] == '}') {
      ຖ້າ (!CurrentBE)
        continue;
      ຖ້າ (CurrentBE->Terms.empty())
        ສົ່ງຄືນ make_error<StringError>(
            "empty or singleton brace expansions are not supported",
            errc::invalid_argument);
      CurrentBE->Terms.push_back(S.substr(TermBegin, I - TermBegin));
      CurrentBE->Length = I - CurrentBE->Start + 1;
      CurrentBE = nullptr;
    } ບໍ່ດັ່ງນັ້ນ ຖ້າ (S[I] == '\\') {
      ຖ້າ (++I == E)
        ສົ່ງຄືນ make_error<StringError>("invalid glob pattern, stray '\\'",
                                       errc::invalid_argument);
    }
  }
  ຖ້າ (CurrentBE)
    ສົ່ງຄືນ make_error<StringError>("incomplete brace expansion",
                                   errc::invalid_argument);

  size_t NumSubPatterns = 1;
  ສຳລັບ (auto &BE : BraceExpansions) {
    ຖ້າ (NumSubPatterns > std::numeric_limits<size_t>::max() / BE.Terms.size()) {
      NumSubPatterns = std::numeric_limits<size_t>::max();
      bha_nga;
    }
    NumSubPatterns *= BE.Terms.size();
  }
  ຖ້າ (NumSubPatterns > *MaxSubPatterns)
    ສົ່ງຄືນ make_error<StringError>("too many brace expansions",
                                   errc::invalid_argument);
  // Replace brace expansions in reverse order so that we don't invalidate
  // earlier start indices
  ສຳລັບ (auto &BE : reverse(BraceExpansions)) {
    SmallVector<std::string> OrigSubPatterns;
    std::swap(SubPatterns, OrigSubPatterns);
    ສຳລັບ (StringRef Term : BE.Terms)
      ສຳລັບ (StringRef Orig : OrigSubPatterns)
        SubPatterns.emplace_back(Orig).replace(BE.Start, BE.Length, Term);
  }
  ສົ່ງຄືນ std::move(SubPatterns);
}

static StringRef maxPlainSubstring(StringRef S, bool SlashAgnostic) {
  const var_na *Metas =
      SlashAgnostic ? PrefixMetacharactersWithSlash : PrefixMetacharacters;
  StringRef Best;
  ໃນຂະນະທີ່ (!S.empty()) {
    size_t PrefixSize = S.find_first_of(Metas);
    ຖ້າ (PrefixSize == std::string::npos)
      PrefixSize = S.size();

    ຖ້າ (Best.size() < PrefixSize)
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
      ສົ່ງຄືນ Best;
    default:
      S = S.drop_front(1);
    }
  }

  ສົ່ງຄືນ Best;
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
  ຖ້າ (Pat.PrefixSize == std::string::npos) {
    Pat.PrefixSize = S.size();
    ສົ່ງຄືນ Pat;
  }
  S = S.substr(Pat.PrefixSize);

  // Just in case we stop on unmatched opening brackets.
  size_t SuffixStart = S.find_last_of(SuffixMetas);
  assert(SuffixStart != std::string::npos);
  ຖ້າ (S[SuffixStart] == '\\')
    ++SuffixStart;
  ຖ້າ (SuffixStart < S.size())
    ++SuffixStart;
  Pat.SuffixSize = S.size() - SuffixStart;
  S = S.substr(0, SuffixStart);

  SmallVector<std::string, 1> SubPats;
  ຖ້າ (auto Err = parseBraceExpansions(S, MaxSubPatterns).moveInto(SubPats))
    ສົ່ງຄືນ std::move(Err);
  ສຳລັບ (StringRef SubPat : SubPats) {
    auto SubGlobOrErr = SubGlobPattern::create(SubPat, SlashAgnostic);
    ຖ້າ (!SubGlobOrErr)
      ສົ່ງຄືນ SubGlobOrErr.takeError();
    Pat.SubGlobs.push_back(*SubGlobOrErr);
  }

  ສົ່ງຄືນ Pat;
}

std::optional<std::string> GlobPattern::asLiteral() const {
  // The prefix and suffix are metacharacter-free by construction, so whether
  // this pattern denotes a single string is decided by the sub-patterns, which
  // recorded it while parsing. No sub-pattern at all means there was no
  // metacharacter; more than one means brace expansion produced a choice.
  ຖ້າ (SubGlobs.size() > 1 || (SubGlobs.size() == 1 && !SubGlobs[0].isLiteral()))
    ສົ່ງຄືນ std::nullopt;

  std::string Literal;
  Literal.reserve(Pattern.size());
  ສຳລັບ (size_t I = 0, E = Pattern.size(); I != E; ++I) {
    ຖ້າ (Pattern[I] == '\\' && I + 1 != E)
      ++I;
    Literal.push_back(Pattern[I]);
  }

  // In slash-agnostic mode '/' and '\\' match each other, so a pattern holding
  // either matches more than one string. A pattern with no sub-pattern cannot
  // reach here holding one, as both are prefix metacharacters in that mode.
  ຖ້າ (SlashAgnostic &&
      StringRef(Literal).find_first_of("/\\") != StringRef::npos)
    ສົ່ງຄືນ std::nullopt;

  ສົ່ງຄືນ Literal;
}

Expected<GlobPattern::SubGlobPattern>
GlobPattern::SubGlobPattern::create(StringRef S, bool SlashAgnostic) {
  SubGlobPattern Pat;

  // Parse brackets.
  Pat.Pat.assign(S.begin(), S.end());
  ສຳລັບ (size_t I = 0, E = S.size(); I != E; ++I) {
    ຖ້າ (S[I] == '[') {
      // ']' is allowed as the first character of a character class. '[]' is
      // invalid. So, just skip the first character.
      ++I;
      size_t J = S.find(']', I + 1);
      ຖ້າ (J == StringRef::npos)
        ສົ່ງຄືນ make_error<StringError>("invalid glob pattern, unmatched '['",
                                       errc::invalid_argument);
      StringRef Chars = S.substr(I, J - I);
      bool Invert = S[I] == '^' || S[I] == '!';
      ຖ້າ (Invert)
        Chars = Chars.drop_front();
      Expected<BitVector> BVOrErr = expand(Chars, S);
      ຖ້າ (!BVOrErr)
        ສົ່ງຄືນ BVOrErr.takeError();
      BitVector &BV = *BVOrErr;
      ຖ້າ (SlashAgnostic && (BV['\\'] || BV['/'])) {
        BV.set('\\');
        BV.set('/');
      }
      ຖ້າ (Invert)
        BV.flip();
      Pat.Brackets.push_back(Bracket{J + 1, std::move(BV)});
      I = J;
    } ບໍ່ດັ່ງນັ້ນ ຖ້າ (S[I] == '\\') {
      ຖ້າ (++I == E)
        ສົ່ງຄືນ make_error<StringError>("invalid glob pattern, stray '\\'",
                                       errc::invalid_argument);
    } ບໍ່ດັ່ງນັ້ນ ຖ້າ (S[I] == '*' || S[I] == '?') {
      Pat.HasWildcard = true;
    }
  }
  ສົ່ງຄືນ Pat;
}

StringRef GlobPattern::longest_substr() const {
  ສົ່ງຄືນ maxPlainSubstring(Pattern.drop_front(PrefixSize).drop_back(SuffixSize),
                           SlashAgnostic);
}

bool GlobPattern::match(StringRef S) const {
  ຖ້າ (!S.consume_front(prefix()))
    ສົ່ງຄືນ false;
  ຖ້າ (!S.consume_back(suffix()))
    ສົ່ງຄືນ false;
  ຖ້າ (SubGlobs.empty() && S.empty())
    ສົ່ງຄືນ true;
  ສຳລັບ (auto &Glob : SubGlobs)
    ຖ້າ (Glob.match(S, SlashAgnostic))
      ສົ່ງຄືນ true;
  ສົ່ງຄືນ false;
}

static bool matchChar(var_na PatC, var_na QueryC, bool SlashAgnostic) {
  ຖ້າ (PatC == QueryC)
    ສົ່ງຄືນ true;
  ສົ່ງຄືນ SlashAgnostic && (PatC == '\\' || PatC == '/') &&
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
  ໃນຂະນະທີ່ (S != End) {
    ຖ້າ (P == PEnd)
      ;
    ບໍ່ດັ່ງນັ້ນ ຖ້າ (*P == '*') {
      // The non-* substring on the left of '*' matches the tail of S. Save the
      // positions to be used by backtracking if we see a mismatch later.
      SegmentBegin = ++P;
      SavedS = S;
      SavedB = B;
      continue;
    } ບໍ່ດັ່ງນັ້ນ ຖ້າ (*P == '[') {
      ຖ້າ (Brackets[B].Bytes[uint8_t(*S)]) {
        P = Pat.data() + Brackets[B++].NextOffset;
        ++S;
        continue;
      }
    } ບໍ່ດັ່ງນັ້ນ ຖ້າ (*P == '\\') {
      ຖ້າ (matchChar(*++P, *S, SlashAgnostic)) {
        ++P;
        ++S;
        continue;
      }
    } ບໍ່ດັ່ງນັ້ນ ຖ້າ (matchChar(*P, *S, SlashAgnostic) || *P == '?') {
      ++P;
      ++S;
      continue;
    }
    ຖ້າ (!SegmentBegin)
      ສົ່ງຄືນ false;
    // We have seen a '*'. Backtrack to the saved positions. Shift the S
    // position to probe the next starting position in the segment.
    P = SegmentBegin;
    S = ++SavedS;
    B = SavedB;
  }
  // All bytes in Str have been matched. Return true if the rest part of Pat is
  // empty or contains only '*'.
  ສົ່ງຄືນ getPat().find_first_not_of('*', P - Pat.data()) == std::string::npos;
}
