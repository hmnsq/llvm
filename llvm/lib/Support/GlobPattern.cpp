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

using ನಾಮಾಕಾಶ llvm;

ಸ್ಥಿರ constexpr ವರ್ಣ PrefixMetacharacters[] = "?*[{\\";
ಸ್ಥಿರ constexpr ವರ್ಣ SuffixMetacharacters[] = "?*[]{}\\";
ಸ್ಥಿರ constexpr ವರ್ಣ PrefixMetacharactersWithSlash[] = "?*[{\\/";
ಸ್ಥಿರ constexpr ವರ್ಣ SuffixMetacharactersWithSlash[] = "?*[]{}\\/";

// Expands character ranges and returns a bitmap.
// For example, "a-cf-hz" is expanded to "abcfghz".
ಸ್ಥಿರ Expected<BitVector> expand(StringRef S, StringRef Original) {
  BitVector BV(256, ಅಸತ್ಯ);

  // Expand X-Y.
  ಪ್ರತಿ (;;) {
    ಯದಿ (S.size() < 3)
      ಭಙ್ಗ;

    uint8_t Start = S[0];
    uint8_t End = S[2];

    // If it doesn't start with something like X-Y,
    // consume the first character and proceed.
    ಯದಿ (S[1] != '-') {
      BV[Start] = ಸತ್ಯ;
      S = S.substr(1);
      ಅನುವರ್ತ;
    }

    // It must be in the form of X-Y.
    // Validate it and then interpret the range.
    ಯದಿ (Start > End)
      ಪ್ರತ್ಯಾಗಮ make_error<StringError>("invalid glob pattern: " + Original,
                                     errc::invalid_argument);

    ಪ್ರತಿ (ಪೂರ್ಣಾಙ್ಕ C = Start; C <= End; ++C)
      BV[(uint8_t)C] = ಸತ್ಯ;
    S = S.substr(3);
  }

  ಪ್ರತಿ (ವರ್ಣ C : S)
    BV[(uint8_t)C] = ಸತ್ಯ;
  ಪ್ರತ್ಯಾಗಮ BV;
}

// Identify brace expansions in S and return the list of patterns they expand
// into.
ಸ್ಥಿರ Expected<SmallVector<std::string, 1>>
parseBraceExpansions(StringRef S, std::optional<size_t> MaxSubPatterns) {
  SmallVector<std::string> SubPatterns = {S.str()};
  ಯದಿ (!MaxSubPatterns || !S.contains('{'))
    ಪ್ರತ್ಯಾಗಮ std::move(SubPatterns);

  ಸಂರಚನಾ BraceExpansion {
    size_t Start;
    size_t Length;
    SmallVector<StringRef, 2> Terms;
  };
  SmallVector<BraceExpansion, 0> BraceExpansions;

  BraceExpansion *CurrentBE = nullptr;
  size_t TermBegin;
  ಪ್ರತಿ (size_t I = 0, E = S.size(); I != E; ++I) {
    ಯದಿ (S[I] == '[') {
      I = S.find(']', I + 2);
      ಯದಿ (I == std::string::npos)
        ಪ್ರತ್ಯಾಗಮ make_error<StringError>("invalid glob pattern, unmatched '['",
                                       errc::invalid_argument);
    } ಅನ್ಯಥಾ ಯದಿ (S[I] == '{') {
      ಯದಿ (CurrentBE)
        ಪ್ರತ್ಯಾಗಮ make_error<StringError>(
            "nested brace expansions are not supported",
            errc::invalid_argument);
      CurrentBE = &BraceExpansions.emplace_back();
      CurrentBE->Start = I;
      TermBegin = I + 1;
    } ಅನ್ಯಥಾ ಯದಿ (S[I] == ',') {
      ಯದಿ (!CurrentBE)
        ಅನುವರ್ತ;
      CurrentBE->Terms.ತಳ್ಳು_back(S.substr(TermBegin, I - TermBegin));
      TermBegin = I + 1;
    } ಅನ್ಯಥಾ ಯದಿ (S[I] == '}') {
      ಯದಿ (!CurrentBE)
        ಅನುವರ್ತ;
      ಯದಿ (CurrentBE->Terms.empty())
        ಪ್ರತ್ಯಾಗಮ make_error<StringError>(
            "empty or singleton brace expansions are not supported",
            errc::invalid_argument);
      CurrentBE->Terms.ತಳ್ಳು_back(S.substr(TermBegin, I - TermBegin));
      CurrentBE->Length = I - CurrentBE->Start + 1;
      CurrentBE = nullptr;
    } ಅನ್ಯಥಾ ಯದಿ (S[I] == '\\') {
      ಯದಿ (++I == E)
        ಪ್ರತ್ಯಾಗಮ make_error<StringError>("invalid glob pattern, stray '\\'",
                                       errc::invalid_argument);
    }
  }
  ಯದಿ (CurrentBE)
    ಪ್ರತ್ಯಾಗಮ make_error<StringError>("incomplete brace expansion",
                                   errc::invalid_argument);

  size_t ಅಂಕೆ_Sub_Patterns = 1;
  ಪ್ರತಿ (auto &BE : BraceExpansions) {
    ಯದಿ (ಅಂಕೆ_Sub_Patterns > std::numeric_limits<size_t>::max() / BE.Terms.size()) {
      ಅಂಕೆ_Sub_Patterns = std::numeric_limits<size_t>::max();
      ಭಙ್ಗ;
    }
    ಅಂಕೆ_Sub_Patterns *= BE.Terms.size();
  }
  ಯದಿ (ಅಂಕೆ_Sub_Patterns > *MaxSubPatterns)
    ಪ್ರತ್ಯಾಗಮ make_error<StringError>("too many brace expansions",
                                   errc::invalid_argument);
  // Replace brace expansions in reverse order so that we don't invalidate
  // earlier start indices
  ಪ್ರತಿ (auto &BE : reverse(BraceExpansions)) {
    SmallVector<std::string> OrigSubPatterns;
    std::swap(SubPatterns, OrigSubPatterns);
    ಪ್ರತಿ (StringRef Term : BE.Terms)
      ಪ್ರತಿ (StringRef Orig : OrigSubPatterns)
        SubPatterns.emplace_back(Orig).replace(BE.Start, BE.Length, Term);
  }
  ಪ್ರತ್ಯಾಗಮ std::move(SubPatterns);
}

ಸ್ಥಿರ StringRef maxPlainSubstring(StringRef S, ತಾರ್ಕಿಕ SlashAgnostic) {
  ಧ್ರುವ ವರ್ಣ *Metas =
      SlashAgnostic ? PrefixMetacharactersWithSlash : PrefixMetacharacters;
  StringRef Best;
  ಯಾವತ್ (!S.empty()) {
    size_t PrefixSize = S.find_first_of(Metas);
    ಯದಿ (PrefixSize == std::string::npos)
      PrefixSize = S.size();

    ಯದಿ (Best.size() < PrefixSize)
      Best = S.take_front(PrefixSize);

    S = S.drop_front(PrefixSize);

    // It's impossible, as the first and last characters of the input string
    // must be Glob special characters, otherwise they would be parts of
    // the prefix or the suffix.
    ಪ್ರತಿಜ್ಞಾ(!S.empty());

    ವಿಕಲ್ಪ (S.front()) {
    ಪಕ್ಷ '\\':
      S = S.drop_front(2);
      ಭಙ್ಗ;
    ಪಕ್ಷ '[': {
      // Drop '[' and the first character which can be ']'.
      S = S.drop_front(2);
      size_t EndBracket = S.find_first_of("]");
      // Should not be possible, SubGlobPattern::create should fail on invalid
      // pattern before we get here.
      ಪ್ರತಿಜ್ಞಾ(EndBracket != std::string::npos);
      S = S.drop_front(EndBracket + 1);
      ಭಙ್ಗ;
    }
    ಪಕ್ಷ '{':
      // TODO: implement.
      // Fallback to whatever is best for now.
      ಪ್ರತ್ಯಾಗಮ Best;
    ಸ್ವಭಾವ:
      S = S.drop_front(1);
    }
  }

  ಪ್ರತ್ಯಾಗಮ Best;
}

Expected<GlobPattern> GlobPattern::create(StringRef S,
                                          std::optional<size_t> MaxSubPatterns,
                                          ತಾರ್ಕಿಕ SlashAgnostic) {
  GlobPattern Pat;
  Pat.SlashAgnostic = SlashAgnostic;
  Pat.Pattern = S;

  ಧ್ರುವ ವರ್ಣ *PrefixMetas =
      SlashAgnostic ? PrefixMetacharactersWithSlash : PrefixMetacharacters;
  ಧ್ರುವ ವರ್ಣ *SuffixMetas =
      SlashAgnostic ? SuffixMetacharactersWithSlash : SuffixMetacharacters;

  // Store the prefix that does not contain any metacharacter.
  Pat.PrefixSize = S.find_first_of(PrefixMetas);
  ಯದಿ (Pat.PrefixSize == std::string::npos) {
    Pat.PrefixSize = S.size();
    ಪ್ರತ್ಯಾಗಮ Pat;
  }
  S = S.substr(Pat.PrefixSize);

  // Just in case we stop on unmatched opening brackets.
  size_t SuffixStart = S.find_last_of(SuffixMetas);
  ಪ್ರತಿಜ್ಞಾ(SuffixStart != std::string::npos);
  ಯದಿ (S[SuffixStart] == '\\')
    ++SuffixStart;
  ಯದಿ (SuffixStart < S.size())
    ++SuffixStart;
  Pat.SuffixSize = S.size() - SuffixStart;
  S = S.substr(0, SuffixStart);

  SmallVector<std::string, 1> SubPats;
  ಯದಿ (auto Err = parseBraceExpansions(S, MaxSubPatterns).moveInto(SubPats))
    ಪ್ರತ್ಯಾಗಮ std::move(Err);
  ಪ್ರತಿ (StringRef SubPat : SubPats) {
    auto SubGlobOrErr = SubGlobPattern::create(SubPat, SlashAgnostic);
    ಯದಿ (!SubGlobOrErr)
      ಪ್ರತ್ಯಾಗಮ SubGlobOrErr.takeError();
    Pat.SubGlobs.ತಳ್ಳು_back(*SubGlobOrErr);
  }

  ಪ್ರತ್ಯಾಗಮ Pat;
}

std::optional<std::string> GlobPattern::asLiteral() ಧ್ರುವ {
  // The prefix and suffix are metacharacter-free by construction, so whether
  // this pattern denotes a single string is decided by the sub-patterns, which
  // recorded it while parsing. No sub-pattern at all means there was no
  // metacharacter; more than one means brace expansion produced a choice.
  ಯದಿ (SubGlobs.size() > 1 || (SubGlobs.size() == 1 && !SubGlobs[0].isLiteral()))
    ಪ್ರತ್ಯಾಗಮ std::nullopt;

  std::string Literal;
  Literal.reserve(Pattern.size());
  ಪ್ರತಿ (size_t I = 0, E = Pattern.size(); I != E; ++I) {
    ಯದಿ (Pattern[I] == '\\' && I + 1 != E)
      ++I;
    Literal.ತಳ್ಳು_back(Pattern[I]);
  }

  // In slash-agnostic mode '/' and '\\' match each other, so a pattern holding
  // either matches more than one string. A pattern with no sub-pattern cannot
  // reach here holding one, as both are prefix metacharacters in that mode.
  ಯದಿ (SlashAgnostic &&
      StringRef(Literal).find_first_of("/\\") != StringRef::npos)
    ಪ್ರತ್ಯಾಗಮ std::nullopt;

  ಪ್ರತ್ಯಾಗಮ Literal;
}

Expected<GlobPattern::SubGlobPattern>
GlobPattern::SubGlobPattern::create(StringRef S, ತಾರ್ಕಿಕ SlashAgnostic) {
  SubGlobPattern Pat;

  // Parse brackets.
  Pat.Pat.assign(S.begin(), S.end());
  ಪ್ರತಿ (size_t I = 0, E = S.size(); I != E; ++I) {
    ಯದಿ (S[I] == '[') {
      // ']' is allowed as the first character of a character class. '[]' is
      // invalid. So, just skip the first character.
      ++I;
      size_t J = S.find(']', I + 1);
      ಯದಿ (J == StringRef::npos)
        ಪ್ರತ್ಯಾಗಮ make_error<StringError>("invalid glob pattern, unmatched '['",
                                       errc::invalid_argument);
      StringRef Chars = S.substr(I, J - I);
      ತಾರ್ಕಿಕ Invert = S[I] == '^' || S[I] == '!';
      ಯದಿ (Invert)
        Chars = Chars.drop_front();
      Expected<BitVector> BVOrErr = expand(Chars, S);
      ಯದಿ (!BVOrErr)
        ಪ್ರತ್ಯಾಗಮ BVOrErr.takeError();
      BitVector &BV = *BVOrErr;
      ಯದಿ (SlashAgnostic && (BV['\\'] || BV['/'])) {
        BV.set('\\');
        BV.set('/');
      }
      ಯದಿ (Invert)
        BV.flip();
      Pat.Brackets.ತಳ್ಳು_back(Bracket{J + 1, std::move(BV)});
      I = J;
    } ಅನ್ಯಥಾ ಯದಿ (S[I] == '\\') {
      ಯದಿ (++I == E)
        ಪ್ರತ್ಯಾಗಮ make_error<StringError>("invalid glob pattern, stray '\\'",
                                       errc::invalid_argument);
    } ಅನ್ಯಥಾ ಯದಿ (S[I] == '*' || S[I] == '?') {
      Pat.HasWildcard = ಸತ್ಯ;
    }
  }
  ಪ್ರತ್ಯಾಗಮ Pat;
}

StringRef GlobPattern::longest_substr() ಧ್ರುವ {
  ಪ್ರತ್ಯಾಗಮ maxPlainSubstring(Pattern.drop_front(PrefixSize).drop_back(SuffixSize),
                           SlashAgnostic);
}

ತಾರ್ಕಿಕ GlobPattern::match(StringRef S) ಧ್ರುವ {
  ಯದಿ (!S.consume_front(prefix()))
    ಪ್ರತ್ಯಾಗಮ ಅಸತ್ಯ;
  ಯದಿ (!S.consume_back(suffix()))
    ಪ್ರತ್ಯಾಗಮ ಅಸತ್ಯ;
  ಯದಿ (SubGlobs.empty() && S.empty())
    ಪ್ರತ್ಯಾಗಮ ಸತ್ಯ;
  ಪ್ರತಿ (auto &Glob : SubGlobs)
    ಯದಿ (Glob.match(S, SlashAgnostic))
      ಪ್ರತ್ಯಾಗಮ ಸತ್ಯ;
  ಪ್ರತ್ಯಾಗಮ ಅಸತ್ಯ;
}

ಸ್ಥಿರ ತಾರ್ಕಿಕ matchChar(ವರ್ಣ PatC, ವರ್ಣ QueryC, ತಾರ್ಕಿಕ SlashAgnostic) {
  ಯದಿ (PatC == QueryC)
    ಪ್ರತ್ಯಾಗಮ ಸತ್ಯ;
  ಪ್ರತ್ಯಾಗಮ SlashAgnostic && (PatC == '\\' || PatC == '/') &&
         (QueryC == '\\' || QueryC == '/');
}

// Factor the pattern into segments split by '*'. The segment is matched
// sequentianlly by finding the first occurrence past the end of the previous
// match.
ತಾರ್ಕಿಕ GlobPattern::SubGlobPattern::match(StringRef Str,
                                        ತಾರ್ಕಿಕ SlashAgnostic) ಧ್ರುವ {
  ಧ್ರುವ ವರ್ಣ *P = Pat.data(), *SegmentBegin = nullptr, *S = Str.data(),
             *SavedS = S;
  ಧ್ರುವ ವರ್ಣ *ಧ್ರುವ PEnd = P + Pat.size(), *ಧ್ರುವ End = S + Str.size();
  size_t B = 0, SavedB = 0;
  ಯಾವತ್ (S != End) {
    ಯದಿ (P == PEnd)
      ;
    ಅನ್ಯಥಾ ಯದಿ (*P == '*') {
      // The non-* substring on the left of '*' matches the tail of S. Save the
      // positions to be used by backtracking if we see a mismatch later.
      SegmentBegin = ++P;
      SavedS = S;
      SavedB = B;
      ಅನುವರ್ತ;
    } ಅನ್ಯಥಾ ಯದಿ (*P == '[') {
      ಯದಿ (Brackets[B].Bytes[uint8_t(*S)]) {
        P = Pat.data() + Brackets[B++].NextOffset;
        ++S;
        ಅನುವರ್ತ;
      }
    } ಅನ್ಯಥಾ ಯದಿ (*P == '\\') {
      ಯದಿ (matchChar(*++P, *S, SlashAgnostic)) {
        ++P;
        ++S;
        ಅನುವರ್ತ;
      }
    } ಅನ್ಯಥಾ ಯದಿ (matchChar(*P, *S, SlashAgnostic) || *P == '?') {
      ++P;
      ++S;
      ಅನುವರ್ತ;
    }
    ಯದಿ (!SegmentBegin)
      ಪ್ರತ್ಯಾಗಮ ಅಸತ್ಯ;
    // We have seen a '*'. Backtrack to the saved positions. Shift the S
    // position to probe the next starting position in the segment.
    P = SegmentBegin;
    S = ++SavedS;
    B = SavedB;
  }
  // All bytes in Str have been matched. Return true if the rest part of Pat is
  // empty or contains only '*'.
  ಪ್ರತ್ಯಾಗಮ getPat().find_first_not_of('*', P - Pat.data()) == std::string::npos;
}
