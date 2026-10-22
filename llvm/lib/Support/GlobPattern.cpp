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

#ଅନ୍ତର୍ଭାଵ "llvm/Support/GlobPattern.h"
#ଅନ୍ତର୍ଭାଵ "llvm/ADT/StringRef.h"
#ଅନ୍ତର୍ଭାଵ "llvm/Support/Errc.h"

using ନାମାକାଶ llvm;

ସ୍ଥିର constexpr ଅକ୍ଷର PrefixMetacharacters[] = "?*[{\\";
ସ୍ଥିର constexpr ଅକ୍ଷର SuffixMetacharacters[] = "?*[]{}\\";
ସ୍ଥିର constexpr ଅକ୍ଷର PrefixMetacharactersWithSlash[] = "?*[{\\/";
ସ୍ଥିର constexpr ଅକ୍ଷର SuffixMetacharactersWithSlash[] = "?*[]{}\\/";

// Expands character ranges and returns a bitmap.
// For example, "a-cf-hz" is expanded to "abcfghz".
ସ୍ଥିର Expected<BitVector> expand(StringRef S, StringRef Original) {
  BitVector BV(256, ମିଥ୍ୟା);

  // Expand X-Y.
  ପାଇଁ (;;) {
    ଯଦି (S.size() < 3)
      ଭାଙ୍ଗ;

    uint8_t Start = S[0];
    uint8_t End = S[2];

    // If it doesn't start with something like X-Y,
    // consume the first character and proceed.
    ଯଦି (S[1] != '-') {
      BV[Start] = ସତ୍ୟ;
      S = S.substr(1);
      ଜାରି;
    }

    // It must be in the form of X-Y.
    // Validate it and then interpret the range.
    ଯଦି (Start > End)
      ଫେରନ୍ତୁ make_error<StringError>("invalid glob pattern: " + Original,
                                     errc::invalid_argument);

    ପାଇଁ (ପୂର୍ଣ୍ଣସଂଖ୍ୟା C = Start; C <= End; ++C)
      BV[(uint8_t)C] = ସତ୍ୟ;
    S = S.substr(3);
  }

  ପାଇଁ (ଅକ୍ଷର C : S)
    BV[(uint8_t)C] = ସତ୍ୟ;
  ଫେରନ୍ତୁ BV;
}

// Identify brace expansions in S and return the list of patterns they expand
// into.
ସ୍ଥିର Expected<SmallVector<std::string, 1>>
parseBraceExpansions(StringRef S, std::optional<size_t> MaxSubPatterns) {
  SmallVector<std::string> SubPatterns = {S.str()};
  ଯଦି (!MaxSubPatterns || !S.contains('{'))
    ଫେରନ୍ତୁ std::move(SubPatterns);

  ସଂରଚନା BraceExpansion {
    size_t Start;
    size_t Length;
    SmallVector<StringRef, 2> Terms;
  };
  SmallVector<BraceExpansion, 0> BraceExpansions;

  BraceExpansion *CurrentBE = nullptr;
  size_t TermBegin;
  ପାଇଁ (size_t I = 0, E = S.size(); I != E; ++I) {
    ଯଦି (S[I] == '[') {
      I = S.find(']', I + 2);
      ଯଦି (I == std::string::npos)
        ଫେରନ୍ତୁ make_error<StringError>("invalid glob pattern, unmatched '['",
                                       errc::invalid_argument);
    } ଅନ୍ୟଥା ଯଦି (S[I] == '{') {
      ଯଦି (CurrentBE)
        ଫେରନ୍ତୁ make_error<StringError>(
            "nested brace expansions are not supported",
            errc::invalid_argument);
      CurrentBE = &BraceExpansions.emplace_back();
      CurrentBE->Start = I;
      TermBegin = I + 1;
    } ଅନ୍ୟଥା ଯଦି (S[I] == ',') {
      ଯଦି (!CurrentBE)
        ଜାରି;
      CurrentBE->Terms.ଠେଲ_back(S.substr(TermBegin, I - TermBegin));
      TermBegin = I + 1;
    } ଅନ୍ୟଥା ଯଦି (S[I] == '}') {
      ଯଦି (!CurrentBE)
        ଜାରି;
      ଯଦି (CurrentBE->Terms.empty())
        ଫେରନ୍ତୁ make_error<StringError>(
            "empty or singleton brace expansions are not supported",
            errc::invalid_argument);
      CurrentBE->Terms.ଠେଲ_back(S.substr(TermBegin, I - TermBegin));
      CurrentBE->Length = I - CurrentBE->Start + 1;
      CurrentBE = nullptr;
    } ଅନ୍ୟଥା ଯଦି (S[I] == '\\') {
      ଯଦି (++I == E)
        ଫେରନ୍ତୁ make_error<StringError>("invalid glob pattern, stray '\\'",
                                       errc::invalid_argument);
    }
  }
  ଯଦି (CurrentBE)
    ଫେରନ୍ତୁ make_error<StringError>("incomplete brace expansion",
                                   errc::invalid_argument);

  size_t ଅଙ୍କ_Sub_Patterns = 1;
  ପାଇଁ (auto &BE : BraceExpansions) {
    ଯଦି (ଅଙ୍କ_Sub_Patterns > std::numeric_limits<size_t>::max() / BE.Terms.size()) {
      ଅଙ୍କ_Sub_Patterns = std::numeric_limits<size_t>::max();
      ଭାଙ୍ଗ;
    }
    ଅଙ୍କ_Sub_Patterns *= BE.Terms.size();
  }
  ଯଦି (ଅଙ୍କ_Sub_Patterns > *MaxSubPatterns)
    ଫେରନ୍ତୁ make_error<StringError>("too many brace expansions",
                                   errc::invalid_argument);
  // Replace brace expansions in reverse order so that we don't invalidate
  // earlier start indices
  ପାଇଁ (auto &BE : reverse(BraceExpansions)) {
    SmallVector<std::string> OrigSubPatterns;
    std::swap(SubPatterns, OrigSubPatterns);
    ପାଇଁ (StringRef Term : BE.Terms)
      ପାଇଁ (StringRef Orig : OrigSubPatterns)
        SubPatterns.emplace_back(Orig).replace(BE.Start, BE.Length, Term);
  }
  ଫେରନ୍ତୁ std::move(SubPatterns);
}

ସ୍ଥିର StringRef maxPlainSubstring(StringRef S, ତାର୍କିକ SlashAgnostic) {
  ଅଚଳ ଅକ୍ଷର *Metas =
      SlashAgnostic ? PrefixMetacharactersWithSlash : PrefixMetacharacters;
  StringRef Best;
  ଯେତେବେଳେ (!S.empty()) {
    size_t PrefixSize = S.find_first_of(Metas);
    ଯଦି (PrefixSize == std::string::npos)
      PrefixSize = S.size();

    ଯଦି (Best.size() < PrefixSize)
      Best = S.take_front(PrefixSize);

    S = S.drop_front(PrefixSize);

    // It's impossible, as the first and last characters of the input string
    // must be Glob special characters, otherwise they would be parts of
    // the prefix or the suffix.
    ପ୍ରତିଜ୍ଞା(!S.empty());

    ବାଛ (S.front()) {
    ସ୍ଥିତି '\\':
      S = S.drop_front(2);
      ଭାଙ୍ଗ;
    ସ୍ଥିତି '[': {
      // Drop '[' and the first character which can be ']'.
      S = S.drop_front(2);
      size_t EndBracket = S.find_first_of("]");
      // Should not be possible, SubGlobPattern::create should fail on invalid
      // pattern before we get here.
      ପ୍ରତିଜ୍ଞା(EndBracket != std::string::npos);
      S = S.drop_front(EndBracket + 1);
      ଭାଙ୍ଗ;
    }
    ସ୍ଥିତି '{':
      // TODO: implement.
      // Fallback to whatever is best for now.
      ଫେରନ୍ତୁ Best;
    ପୂର୍ବନିର୍ଦ୍ଧାରିତ:
      S = S.drop_front(1);
    }
  }

  ଫେରନ୍ତୁ Best;
}

Expected<GlobPattern> GlobPattern::create(StringRef S,
                                          std::optional<size_t> MaxSubPatterns,
                                          ତାର୍କିକ SlashAgnostic) {
  GlobPattern Pat;
  Pat.SlashAgnostic = SlashAgnostic;
  Pat.Pattern = S;

  ଅଚଳ ଅକ୍ଷର *PrefixMetas =
      SlashAgnostic ? PrefixMetacharactersWithSlash : PrefixMetacharacters;
  ଅଚଳ ଅକ୍ଷର *SuffixMetas =
      SlashAgnostic ? SuffixMetacharactersWithSlash : SuffixMetacharacters;

  // Store the prefix that does not contain any metacharacter.
  Pat.PrefixSize = S.find_first_of(PrefixMetas);
  ଯଦି (Pat.PrefixSize == std::string::npos) {
    Pat.PrefixSize = S.size();
    ଫେରନ୍ତୁ Pat;
  }
  S = S.substr(Pat.PrefixSize);

  // Just in case we stop on unmatched opening brackets.
  size_t SuffixStart = S.find_last_of(SuffixMetas);
  ପ୍ରତିଜ୍ଞା(SuffixStart != std::string::npos);
  ଯଦି (S[SuffixStart] == '\\')
    ++SuffixStart;
  ଯଦି (SuffixStart < S.size())
    ++SuffixStart;
  Pat.SuffixSize = S.size() - SuffixStart;
  S = S.substr(0, SuffixStart);

  SmallVector<std::string, 1> SubPats;
  ଯଦି (auto Err = parseBraceExpansions(S, MaxSubPatterns).moveInto(SubPats))
    ଫେରନ୍ତୁ std::move(Err);
  ପାଇଁ (StringRef SubPat : SubPats) {
    auto SubGlobOrErr = SubGlobPattern::create(SubPat, SlashAgnostic);
    ଯଦି (!SubGlobOrErr)
      ଫେରନ୍ତୁ SubGlobOrErr.takeError();
    Pat.SubGlobs.ଠେଲ_back(*SubGlobOrErr);
  }

  ଫେରନ୍ତୁ Pat;
}

std::optional<std::string> GlobPattern::asLiteral() ଅଚଳ {
  // The prefix and suffix are metacharacter-free by construction, so whether
  // this pattern denotes a single string is decided by the sub-patterns, which
  // recorded it while parsing. No sub-pattern at all means there was no
  // metacharacter; more than one means brace expansion produced a choice.
  ଯଦି (SubGlobs.size() > 1 || (SubGlobs.size() == 1 && !SubGlobs[0].isLiteral()))
    ଫେରନ୍ତୁ std::nullopt;

  std::string Literal;
  Literal.reserve(Pattern.size());
  ପାଇଁ (size_t I = 0, E = Pattern.size(); I != E; ++I) {
    ଯଦି (Pattern[I] == '\\' && I + 1 != E)
      ++I;
    Literal.ଠେଲ_back(Pattern[I]);
  }

  // In slash-agnostic mode '/' and '\\' match each other, so a pattern holding
  // either matches more than one string. A pattern with no sub-pattern cannot
  // reach here holding one, as both are prefix metacharacters in that mode.
  ଯଦି (SlashAgnostic &&
      StringRef(Literal).find_first_of("/\\") != StringRef::npos)
    ଫେରନ୍ତୁ std::nullopt;

  ଫେରନ୍ତୁ Literal;
}

Expected<GlobPattern::SubGlobPattern>
GlobPattern::SubGlobPattern::create(StringRef S, ତାର୍କିକ SlashAgnostic) {
  SubGlobPattern Pat;

  // Parse brackets.
  Pat.Pat.assign(S.begin(), S.end());
  ପାଇଁ (size_t I = 0, E = S.size(); I != E; ++I) {
    ଯଦି (S[I] == '[') {
      // ']' is allowed as the first character of a character class. '[]' is
      // invalid. So, just skip the first character.
      ++I;
      size_t J = S.find(']', I + 1);
      ଯଦି (J == StringRef::npos)
        ଫେରନ୍ତୁ make_error<StringError>("invalid glob pattern, unmatched '['",
                                       errc::invalid_argument);
      StringRef Chars = S.substr(I, J - I);
      ତାର୍କିକ Invert = S[I] == '^' || S[I] == '!';
      ଯଦି (Invert)
        Chars = Chars.drop_front();
      Expected<BitVector> BVOrErr = expand(Chars, S);
      ଯଦି (!BVOrErr)
        ଫେରନ୍ତୁ BVOrErr.takeError();
      BitVector &BV = *BVOrErr;
      ଯଦି (SlashAgnostic && (BV['\\'] || BV['/'])) {
        BV.set('\\');
        BV.set('/');
      }
      ଯଦି (Invert)
        BV.flip();
      Pat.Brackets.ଠେଲ_back(Bracket{J + 1, std::move(BV)});
      I = J;
    } ଅନ୍ୟଥା ଯଦି (S[I] == '\\') {
      ଯଦି (++I == E)
        ଫେରନ୍ତୁ make_error<StringError>("invalid glob pattern, stray '\\'",
                                       errc::invalid_argument);
    } ଅନ୍ୟଥା ଯଦି (S[I] == '*' || S[I] == '?') {
      Pat.HasWildcard = ସତ୍ୟ;
    }
  }
  ଫେରନ୍ତୁ Pat;
}

StringRef GlobPattern::longest_substr() ଅଚଳ {
  ଫେରନ୍ତୁ maxPlainSubstring(Pattern.drop_front(PrefixSize).drop_back(SuffixSize),
                           SlashAgnostic);
}

ତାର୍କିକ GlobPattern::match(StringRef S) ଅଚଳ {
  ଯଦି (!S.consume_front(prefix()))
    ଫେରନ୍ତୁ ମିଥ୍ୟା;
  ଯଦି (!S.consume_back(suffix()))
    ଫେରନ୍ତୁ ମିଥ୍ୟା;
  ଯଦି (SubGlobs.empty() && S.empty())
    ଫେରନ୍ତୁ ସତ୍ୟ;
  ପାଇଁ (auto &Glob : SubGlobs)
    ଯଦି (Glob.match(S, SlashAgnostic))
      ଫେରନ୍ତୁ ସତ୍ୟ;
  ଫେରନ୍ତୁ ମିଥ୍ୟା;
}

ସ୍ଥିର ତାର୍କିକ matchChar(ଅକ୍ଷର PatC, ଅକ୍ଷର QueryC, ତାର୍କିକ SlashAgnostic) {
  ଯଦି (PatC == QueryC)
    ଫେରନ୍ତୁ ସତ୍ୟ;
  ଫେରନ୍ତୁ SlashAgnostic && (PatC == '\\' || PatC == '/') &&
         (QueryC == '\\' || QueryC == '/');
}

// Factor the pattern into segments split by '*'. The segment is matched
// sequentianlly by finding the first occurrence past the end of the previous
// match.
ତାର୍କିକ GlobPattern::SubGlobPattern::match(StringRef Str,
                                        ତାର୍କିକ SlashAgnostic) ଅଚଳ {
  ଅଚଳ ଅକ୍ଷର *P = Pat.data(), *SegmentBegin = nullptr, *S = Str.data(),
             *SavedS = S;
  ଅଚଳ ଅକ୍ଷର *ଅଚଳ PEnd = P + Pat.size(), *ଅଚଳ End = S + Str.size();
  size_t B = 0, SavedB = 0;
  ଯେତେବେଳେ (S != End) {
    ଯଦି (P == PEnd)
      ;
    ଅନ୍ୟଥା ଯଦି (*P == '*') {
      // The non-* substring on the left of '*' matches the tail of S. Save the
      // positions to be used by backtracking if we see a mismatch later.
      SegmentBegin = ++P;
      SavedS = S;
      SavedB = B;
      ଜାରି;
    } ଅନ୍ୟଥା ଯଦି (*P == '[') {
      ଯଦି (Brackets[B].Bytes[uint8_t(*S)]) {
        P = Pat.data() + Brackets[B++].NextOffset;
        ++S;
        ଜାରି;
      }
    } ଅନ୍ୟଥା ଯଦି (*P == '\\') {
      ଯଦି (matchChar(*++P, *S, SlashAgnostic)) {
        ++P;
        ++S;
        ଜାରି;
      }
    } ଅନ୍ୟଥା ଯଦି (matchChar(*P, *S, SlashAgnostic) || *P == '?') {
      ++P;
      ++S;
      ଜାରି;
    }
    ଯଦି (!SegmentBegin)
      ଫେରନ୍ତୁ ମିଥ୍ୟା;
    // We have seen a '*'. Backtrack to the saved positions. Shift the S
    // position to probe the next starting position in the segment.
    P = SegmentBegin;
    S = ++SavedS;
    B = SavedB;
  }
  // All bytes in Str have been matched. Return true if the rest part of Pat is
  // empty or contains only '*'.
  ଫେରନ୍ତୁ getPat().find_first_not_of('*', P - Pat.data()) == std::string::npos;
}
