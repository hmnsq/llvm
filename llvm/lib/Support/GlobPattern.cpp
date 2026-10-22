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

#अन्तर्भाव "llvm/Support/GlobPattern.h"
#अन्तर्भाव "llvm/ADT/StringRef.h"
#अन्तर्भाव "llvm/Support/Errc.h"

using नामाकाश llvm;

स्थिर constexpr वर्ण PrefixMetacharacters[] = "?*[{\\";
स्थिर constexpr वर्ण SuffixMetacharacters[] = "?*[]{}\\";
स्थिर constexpr वर्ण PrefixMetacharactersWithSlash[] = "?*[{\\/";
स्थिर constexpr वर्ण SuffixMetacharactersWithSlash[] = "?*[]{}\\/";

// Expands character ranges and returns a bitmap.
// For example, "a-cf-hz" is expanded to "abcfghz".
स्थिर Expected<BitVector> expand(StringRef S, StringRef Original) {
  BitVector BV(256, मिच्छा);

  // Expand X-Y.
  अत्थाय (;;) {
    सचे (S.size() < 3)
      भङ्ग;

    uint8_t Start = S[0];
    uint8_t End = S[2];

    // If it doesn't start with something like X-Y,
    // consume the first character and proceed.
    सचे (S[1] != '-') {
      BV[Start] = सच्च;
      S = S.substr(1);
      अनुवर्त;
    }

    // It must be in the form of X-Y.
    // Validate it and then interpret the range.
    सचे (Start > End)
      पटिनिवत्त make_error<StringError>("invalid glob pattern: " + Original,
                                     errc::invalid_argument);

    अत्थाय (पूर्णाङ्क C = Start; C <= End; ++C)
      BV[(uint8_t)C] = सच्च;
    S = S.substr(3);
  }

  अत्थाय (वर्ण C : S)
    BV[(uint8_t)C] = सच्च;
  पटिनिवत्त BV;
}

// Identify brace expansions in S and return the list of patterns they expand
// into.
स्थिर Expected<SmallVector<std::string, 1>>
parseBraceExpansions(StringRef S, std::optional<size_t> MaxSubPatterns) {
  SmallVector<std::string> SubPatterns = {S.str()};
  सचे (!MaxSubPatterns || !S.contains('{'))
    पटिनिवत्त std::move(SubPatterns);

  संरचना BraceExpansion {
    size_t Start;
    size_t Length;
    SmallVector<StringRef, 2> Terms;
  };
  SmallVector<BraceExpansion, 0> BraceExpansions;

  BraceExpansion *CurrentBE = nullptr;
  size_t TermBegin;
  अत्थाय (size_t I = 0, E = S.size(); I != E; ++I) {
    सचे (S[I] == '[') {
      I = S.find(']', I + 2);
      सचे (I == std::string::npos)
        पटिनिवत्त make_error<StringError>("invalid glob pattern, unmatched '['",
                                       errc::invalid_argument);
    } अञ्ञथा सचे (S[I] == '{') {
      सचे (CurrentBE)
        पटिनिवत्त make_error<StringError>(
            "nested brace expansions are not supported",
            errc::invalid_argument);
      CurrentBE = &BraceExpansions.emplace_back();
      CurrentBE->Start = I;
      TermBegin = I + 1;
    } अञ्ञथा सचे (S[I] == ',') {
      सचे (!CurrentBE)
        अनुवर्त;
      CurrentBE->Terms.प्रक्षिप_back(S.substr(TermBegin, I - TermBegin));
      TermBegin = I + 1;
    } अञ्ञथा सचे (S[I] == '}') {
      सचे (!CurrentBE)
        अनुवर्त;
      सचे (CurrentBE->Terms.empty())
        पटिनिवत्त make_error<StringError>(
            "empty or singleton brace expansions are not supported",
            errc::invalid_argument);
      CurrentBE->Terms.प्रक्षिप_back(S.substr(TermBegin, I - TermBegin));
      CurrentBE->Length = I - CurrentBE->Start + 1;
      CurrentBE = nullptr;
    } अञ्ञथा सचे (S[I] == '\\') {
      सचे (++I == E)
        पटिनिवत्त make_error<StringError>("invalid glob pattern, stray '\\'",
                                       errc::invalid_argument);
    }
  }
  सचे (CurrentBE)
    पटिनिवत्त make_error<StringError>("incomplete brace expansion",
                                   errc::invalid_argument);

  size_t अङ्कः_Sub_Patterns = 1;
  अत्थाय (auto &BE : BraceExpansions) {
    सचे (अङ्कः_Sub_Patterns > std::numeric_limits<size_t>::max() / BE.Terms.size()) {
      अङ्कः_Sub_Patterns = std::numeric_limits<size_t>::max();
      भङ्ग;
    }
    अङ्कः_Sub_Patterns *= BE.Terms.size();
  }
  सचे (अङ्कः_Sub_Patterns > *MaxSubPatterns)
    पटिनिवत्त make_error<StringError>("too many brace expansions",
                                   errc::invalid_argument);
  // Replace brace expansions in reverse order so that we don't invalidate
  // earlier start indices
  अत्थाय (auto &BE : reverse(BraceExpansions)) {
    SmallVector<std::string> OrigSubPatterns;
    std::swap(SubPatterns, OrigSubPatterns);
    अत्थाय (StringRef Term : BE.Terms)
      अत्थाय (StringRef Orig : OrigSubPatterns)
        SubPatterns.emplace_back(Orig).replace(BE.Start, BE.Length, Term);
  }
  पटिनिवत्त std::move(SubPatterns);
}

स्थिर StringRef maxPlainSubstring(StringRef S, तार्किक SlashAgnostic) {
  ध्रुव वर्ण *Metas =
      SlashAgnostic ? PrefixMetacharactersWithSlash : PrefixMetacharacters;
  StringRef Best;
  याव (!S.empty()) {
    size_t PrefixSize = S.find_first_of(Metas);
    सचे (PrefixSize == std::string::npos)
      PrefixSize = S.size();

    सचे (Best.size() < PrefixSize)
      Best = S.take_front(PrefixSize);

    S = S.drop_front(PrefixSize);

    // It's impossible, as the first and last characters of the input string
    // must be Glob special characters, otherwise they would be parts of
    // the prefix or the suffix.
    प्रतिज्ञा(!S.empty());

    विकल्प (S.front()) {
    पक्ष '\\':
      S = S.drop_front(2);
      भङ्ग;
    पक्ष '[': {
      // Drop '[' and the first character which can be ']'.
      S = S.drop_front(2);
      size_t EndBracket = S.find_first_of("]");
      // Should not be possible, SubGlobPattern::create should fail on invalid
      // pattern before we get here.
      प्रतिज्ञा(EndBracket != std::string::npos);
      S = S.drop_front(EndBracket + 1);
      भङ्ग;
    }
    पक्ष '{':
      // TODO: implement.
      // Fallback to whatever is best for now.
      पटिनिवत्त Best;
    स्वभाव:
      S = S.drop_front(1);
    }
  }

  पटिनिवत्त Best;
}

Expected<GlobPattern> GlobPattern::create(StringRef S,
                                          std::optional<size_t> MaxSubPatterns,
                                          तार्किक SlashAgnostic) {
  GlobPattern Pat;
  Pat.SlashAgnostic = SlashAgnostic;
  Pat.Pattern = S;

  ध्रुव वर्ण *PrefixMetas =
      SlashAgnostic ? PrefixMetacharactersWithSlash : PrefixMetacharacters;
  ध्रुव वर्ण *SuffixMetas =
      SlashAgnostic ? SuffixMetacharactersWithSlash : SuffixMetacharacters;

  // Store the prefix that does not contain any metacharacter.
  Pat.PrefixSize = S.find_first_of(PrefixMetas);
  सचे (Pat.PrefixSize == std::string::npos) {
    Pat.PrefixSize = S.size();
    पटिनिवत्त Pat;
  }
  S = S.substr(Pat.PrefixSize);

  // Just in case we stop on unmatched opening brackets.
  size_t SuffixStart = S.find_last_of(SuffixMetas);
  प्रतिज्ञा(SuffixStart != std::string::npos);
  सचे (S[SuffixStart] == '\\')
    ++SuffixStart;
  सचे (SuffixStart < S.size())
    ++SuffixStart;
  Pat.SuffixSize = S.size() - SuffixStart;
  S = S.substr(0, SuffixStart);

  SmallVector<std::string, 1> SubPats;
  सचे (auto Err = parseBraceExpansions(S, MaxSubPatterns).moveInto(SubPats))
    पटिनिवत्त std::move(Err);
  अत्थाय (StringRef SubPat : SubPats) {
    auto SubGlobOrErr = SubGlobPattern::create(SubPat, SlashAgnostic);
    सचे (!SubGlobOrErr)
      पटिनिवत्त SubGlobOrErr.takeError();
    Pat.SubGlobs.प्रक्षिप_back(*SubGlobOrErr);
  }

  पटिनिवत्त Pat;
}

std::optional<std::string> GlobPattern::asLiteral() ध्रुव {
  // The prefix and suffix are metacharacter-free by construction, so whether
  // this pattern denotes a single string is decided by the sub-patterns, which
  // recorded it while parsing. No sub-pattern at all means there was no
  // metacharacter; more than one means brace expansion produced a choice.
  सचे (SubGlobs.size() > 1 || (SubGlobs.size() == 1 && !SubGlobs[0].isLiteral()))
    पटिनिवत्त std::nullopt;

  std::string Literal;
  Literal.reserve(Pattern.size());
  अत्थाय (size_t I = 0, E = Pattern.size(); I != E; ++I) {
    सचे (Pattern[I] == '\\' && I + 1 != E)
      ++I;
    Literal.प्रक्षिप_back(Pattern[I]);
  }

  // In slash-agnostic mode '/' and '\\' match each other, so a pattern holding
  // either matches more than one string. A pattern with no sub-pattern cannot
  // reach here holding one, as both are prefix metacharacters in that mode.
  सचे (SlashAgnostic &&
      StringRef(Literal).find_first_of("/\\") != StringRef::npos)
    पटिनिवत्त std::nullopt;

  पटिनिवत्त Literal;
}

Expected<GlobPattern::SubGlobPattern>
GlobPattern::SubGlobPattern::create(StringRef S, तार्किक SlashAgnostic) {
  SubGlobPattern Pat;

  // Parse brackets.
  Pat.Pat.assign(S.begin(), S.end());
  अत्थाय (size_t I = 0, E = S.size(); I != E; ++I) {
    सचे (S[I] == '[') {
      // ']' is allowed as the first character of a character class. '[]' is
      // invalid. So, just skip the first character.
      ++I;
      size_t J = S.find(']', I + 1);
      सचे (J == StringRef::npos)
        पटिनिवत्त make_error<StringError>("invalid glob pattern, unmatched '['",
                                       errc::invalid_argument);
      StringRef Chars = S.substr(I, J - I);
      तार्किक Invert = S[I] == '^' || S[I] == '!';
      सचे (Invert)
        Chars = Chars.drop_front();
      Expected<BitVector> BVOrErr = expand(Chars, S);
      सचे (!BVOrErr)
        पटिनिवत्त BVOrErr.takeError();
      BitVector &BV = *BVOrErr;
      सचे (SlashAgnostic && (BV['\\'] || BV['/'])) {
        BV.set('\\');
        BV.set('/');
      }
      सचे (Invert)
        BV.flip();
      Pat.Brackets.प्रक्षिप_back(Bracket{J + 1, std::move(BV)});
      I = J;
    } अञ्ञथा सचे (S[I] == '\\') {
      सचे (++I == E)
        पटिनिवत्त make_error<StringError>("invalid glob pattern, stray '\\'",
                                       errc::invalid_argument);
    } अञ्ञथा सचे (S[I] == '*' || S[I] == '?') {
      Pat.HasWildcard = सच्च;
    }
  }
  पटिनिवत्त Pat;
}

StringRef GlobPattern::longest_substr() ध्रुव {
  पटिनिवत्त maxPlainSubstring(Pattern.drop_front(PrefixSize).drop_back(SuffixSize),
                           SlashAgnostic);
}

तार्किक GlobPattern::match(StringRef S) ध्रुव {
  सचे (!S.consume_front(prefix()))
    पटिनिवत्त मिच्छा;
  सचे (!S.consume_back(suffix()))
    पटिनिवत्त मिच्छा;
  सचे (SubGlobs.empty() && S.empty())
    पटिनिवत्त सच्च;
  अत्थाय (auto &Glob : SubGlobs)
    सचे (Glob.match(S, SlashAgnostic))
      पटिनिवत्त सच्च;
  पटिनिवत्त मिच्छा;
}

स्थिर तार्किक matchChar(वर्ण PatC, वर्ण QueryC, तार्किक SlashAgnostic) {
  सचे (PatC == QueryC)
    पटिनिवत्त सच्च;
  पटिनिवत्त SlashAgnostic && (PatC == '\\' || PatC == '/') &&
         (QueryC == '\\' || QueryC == '/');
}

// Factor the pattern into segments split by '*'. The segment is matched
// sequentianlly by finding the first occurrence past the end of the previous
// match.
तार्किक GlobPattern::SubGlobPattern::match(StringRef Str,
                                        तार्किक SlashAgnostic) ध्रुव {
  ध्रुव वर्ण *P = Pat.data(), *SegmentBegin = nullptr, *S = Str.data(),
             *SavedS = S;
  ध्रुव वर्ण *ध्रुव PEnd = P + Pat.size(), *ध्रुव End = S + Str.size();
  size_t B = 0, SavedB = 0;
  याव (S != End) {
    सचे (P == PEnd)
      ;
    अञ्ञथा सचे (*P == '*') {
      // The non-* substring on the left of '*' matches the tail of S. Save the
      // positions to be used by backtracking if we see a mismatch later.
      SegmentBegin = ++P;
      SavedS = S;
      SavedB = B;
      अनुवर्त;
    } अञ्ञथा सचे (*P == '[') {
      सचे (Brackets[B].Bytes[uint8_t(*S)]) {
        P = Pat.data() + Brackets[B++].NextOffset;
        ++S;
        अनुवर्त;
      }
    } अञ्ञथा सचे (*P == '\\') {
      सचे (matchChar(*++P, *S, SlashAgnostic)) {
        ++P;
        ++S;
        अनुवर्त;
      }
    } अञ्ञथा सचे (matchChar(*P, *S, SlashAgnostic) || *P == '?') {
      ++P;
      ++S;
      अनुवर्त;
    }
    सचे (!SegmentBegin)
      पटिनिवत्त मिच्छा;
    // We have seen a '*'. Backtrack to the saved positions. Shift the S
    // position to probe the next starting position in the segment.
    P = SegmentBegin;
    S = ++SavedS;
    B = SavedB;
  }
  // All bytes in Str have been matched. Return true if the rest part of Pat is
  // empty or contains only '*'.
  पटिनिवत्त getPat().find_first_not_of('*', P - Pat.data()) == std::string::npos;
}
