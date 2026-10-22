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
  BitVector BV(256, मिच्छ);

  // Expand X-Y.
  प्रति (;;) {
    जइ (S.size() < 3)
      भङ्ग;

    uint8_t Start = S[0];
    uint8_t End = S[2];

    // If it doesn't start with something like X-Y,
    // consume the first character and proceed.
    जइ (S[1] != '-') {
      BV[Start] = सच्च;
      S = S.substr(1);
      अनुवर्त;
    }

    // It must be in the form of X-Y.
    // Validate it and then interpret the range.
    जइ (Start > End)
      प्रत्यागम make_error<StringError>("invalid glob pattern: " + Original,
                                     errc::invalid_argument);

    प्रति (पूर्णाङ्क C = Start; C <= End; ++C)
      BV[(uint8_t)C] = सच्च;
    S = S.substr(3);
  }

  प्रति (वर्ण C : S)
    BV[(uint8_t)C] = सच्च;
  प्रत्यागम BV;
}

// Identify brace expansions in S and return the list of patterns they expand
// into.
स्थिर Expected<SmallVector<std::string, 1>>
parseBraceExpansions(StringRef S, std::optional<size_t> MaxSubPatterns) {
  SmallVector<std::string> SubPatterns = {S.str()};
  जइ (!MaxSubPatterns || !S.contains('{'))
    प्रत्यागम std::move(SubPatterns);

  संरचना BraceExpansion {
    size_t Start;
    size_t Length;
    SmallVector<StringRef, 2> Terms;
  };
  SmallVector<BraceExpansion, 0> BraceExpansions;

  BraceExpansion *CurrentBE = nullptr;
  size_t TermBegin;
  प्रति (size_t I = 0, E = S.size(); I != E; ++I) {
    जइ (S[I] == '[') {
      I = S.find(']', I + 2);
      जइ (I == std::string::npos)
        प्रत्यागम make_error<StringError>("invalid glob pattern, unmatched '['",
                                       errc::invalid_argument);
    } अण्णहा जइ (S[I] == '{') {
      जइ (CurrentBE)
        प्रत्यागम make_error<StringError>(
            "nested brace expansions are not supported",
            errc::invalid_argument);
      CurrentBE = &BraceExpansions.emplace_back();
      CurrentBE->Start = I;
      TermBegin = I + 1;
    } अण्णहा जइ (S[I] == ',') {
      जइ (!CurrentBE)
        अनुवर्त;
      CurrentBE->Terms.प्रक्षिप_back(S.substr(TermBegin, I - TermBegin));
      TermBegin = I + 1;
    } अण्णहा जइ (S[I] == '}') {
      जइ (!CurrentBE)
        अनुवर्त;
      जइ (CurrentBE->Terms.empty())
        प्रत्यागम make_error<StringError>(
            "empty or singleton brace expansions are not supported",
            errc::invalid_argument);
      CurrentBE->Terms.प्रक्षिप_back(S.substr(TermBegin, I - TermBegin));
      CurrentBE->Length = I - CurrentBE->Start + 1;
      CurrentBE = nullptr;
    } अण्णहा जइ (S[I] == '\\') {
      जइ (++I == E)
        प्रत्यागम make_error<StringError>("invalid glob pattern, stray '\\'",
                                       errc::invalid_argument);
    }
  }
  जइ (CurrentBE)
    प्रत्यागम make_error<StringError>("incomplete brace expansion",
                                   errc::invalid_argument);

  size_t अङ्कः_Sub_Patterns = 1;
  प्रति (auto &BE : BraceExpansions) {
    जइ (अङ्कः_Sub_Patterns > std::numeric_limits<size_t>::max() / BE.Terms.size()) {
      अङ्कः_Sub_Patterns = std::numeric_limits<size_t>::max();
      भङ्ग;
    }
    अङ्कः_Sub_Patterns *= BE.Terms.size();
  }
  जइ (अङ्कः_Sub_Patterns > *MaxSubPatterns)
    प्रत्यागम make_error<StringError>("too many brace expansions",
                                   errc::invalid_argument);
  // Replace brace expansions in reverse order so that we don't invalidate
  // earlier start indices
  प्रति (auto &BE : reverse(BraceExpansions)) {
    SmallVector<std::string> OrigSubPatterns;
    std::swap(SubPatterns, OrigSubPatterns);
    प्रति (StringRef Term : BE.Terms)
      प्रति (StringRef Orig : OrigSubPatterns)
        SubPatterns.emplace_back(Orig).replace(BE.Start, BE.Length, Term);
  }
  प्रत्यागम std::move(SubPatterns);
}

स्थिर StringRef maxPlainSubstring(StringRef S, तार्किक SlashAgnostic) {
  ध्रुव वर्ण *Metas =
      SlashAgnostic ? PrefixMetacharactersWithSlash : PrefixMetacharacters;
  StringRef Best;
  जाव (!S.empty()) {
    size_t PrefixSize = S.find_first_of(Metas);
    जइ (PrefixSize == std::string::npos)
      PrefixSize = S.size();

    जइ (Best.size() < PrefixSize)
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
      प्रत्यागम Best;
    स्वभाव:
      S = S.drop_front(1);
    }
  }

  प्रत्यागम Best;
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
  जइ (Pat.PrefixSize == std::string::npos) {
    Pat.PrefixSize = S.size();
    प्रत्यागम Pat;
  }
  S = S.substr(Pat.PrefixSize);

  // Just in case we stop on unmatched opening brackets.
  size_t SuffixStart = S.find_last_of(SuffixMetas);
  प्रतिज्ञा(SuffixStart != std::string::npos);
  जइ (S[SuffixStart] == '\\')
    ++SuffixStart;
  जइ (SuffixStart < S.size())
    ++SuffixStart;
  Pat.SuffixSize = S.size() - SuffixStart;
  S = S.substr(0, SuffixStart);

  SmallVector<std::string, 1> SubPats;
  जइ (auto Err = parseBraceExpansions(S, MaxSubPatterns).moveInto(SubPats))
    प्रत्यागम std::move(Err);
  प्रति (StringRef SubPat : SubPats) {
    auto SubGlobOrErr = SubGlobPattern::create(SubPat, SlashAgnostic);
    जइ (!SubGlobOrErr)
      प्रत्यागम SubGlobOrErr.takeError();
    Pat.SubGlobs.प्रक्षिप_back(*SubGlobOrErr);
  }

  प्रत्यागम Pat;
}

std::optional<std::string> GlobPattern::asLiteral() ध्रुव {
  // The prefix and suffix are metacharacter-free by construction, so whether
  // this pattern denotes a single string is decided by the sub-patterns, which
  // recorded it while parsing. No sub-pattern at all means there was no
  // metacharacter; more than one means brace expansion produced a choice.
  जइ (SubGlobs.size() > 1 || (SubGlobs.size() == 1 && !SubGlobs[0].isLiteral()))
    प्रत्यागम std::nullopt;

  std::string Literal;
  Literal.reserve(Pattern.size());
  प्रति (size_t I = 0, E = Pattern.size(); I != E; ++I) {
    जइ (Pattern[I] == '\\' && I + 1 != E)
      ++I;
    Literal.प्रक्षिप_back(Pattern[I]);
  }

  // In slash-agnostic mode '/' and '\\' match each other, so a pattern holding
  // either matches more than one string. A pattern with no sub-pattern cannot
  // reach here holding one, as both are prefix metacharacters in that mode.
  जइ (SlashAgnostic &&
      StringRef(Literal).find_first_of("/\\") != StringRef::npos)
    प्रत्यागम std::nullopt;

  प्रत्यागम Literal;
}

Expected<GlobPattern::SubGlobPattern>
GlobPattern::SubGlobPattern::create(StringRef S, तार्किक SlashAgnostic) {
  SubGlobPattern Pat;

  // Parse brackets.
  Pat.Pat.assign(S.begin(), S.end());
  प्रति (size_t I = 0, E = S.size(); I != E; ++I) {
    जइ (S[I] == '[') {
      // ']' is allowed as the first character of a character class. '[]' is
      // invalid. So, just skip the first character.
      ++I;
      size_t J = S.find(']', I + 1);
      जइ (J == StringRef::npos)
        प्रत्यागम make_error<StringError>("invalid glob pattern, unmatched '['",
                                       errc::invalid_argument);
      StringRef Chars = S.substr(I, J - I);
      तार्किक Invert = S[I] == '^' || S[I] == '!';
      जइ (Invert)
        Chars = Chars.drop_front();
      Expected<BitVector> BVOrErr = expand(Chars, S);
      जइ (!BVOrErr)
        प्रत्यागम BVOrErr.takeError();
      BitVector &BV = *BVOrErr;
      जइ (SlashAgnostic && (BV['\\'] || BV['/'])) {
        BV.set('\\');
        BV.set('/');
      }
      जइ (Invert)
        BV.flip();
      Pat.Brackets.प्रक्षिप_back(Bracket{J + 1, std::move(BV)});
      I = J;
    } अण्णहा जइ (S[I] == '\\') {
      जइ (++I == E)
        प्रत्यागम make_error<StringError>("invalid glob pattern, stray '\\'",
                                       errc::invalid_argument);
    } अण्णहा जइ (S[I] == '*' || S[I] == '?') {
      Pat.HasWildcard = सच्च;
    }
  }
  प्रत्यागम Pat;
}

StringRef GlobPattern::longest_substr() ध्रुव {
  प्रत्यागम maxPlainSubstring(Pattern.drop_front(PrefixSize).drop_back(SuffixSize),
                           SlashAgnostic);
}

तार्किक GlobPattern::match(StringRef S) ध्रुव {
  जइ (!S.consume_front(prefix()))
    प्रत्यागम मिच्छ;
  जइ (!S.consume_back(suffix()))
    प्रत्यागम मिच्छ;
  जइ (SubGlobs.empty() && S.empty())
    प्रत्यागम सच्च;
  प्रति (auto &Glob : SubGlobs)
    जइ (Glob.match(S, SlashAgnostic))
      प्रत्यागम सच्च;
  प्रत्यागम मिच्छ;
}

स्थिर तार्किक matchChar(वर्ण PatC, वर्ण QueryC, तार्किक SlashAgnostic) {
  जइ (PatC == QueryC)
    प्रत्यागम सच्च;
  प्रत्यागम SlashAgnostic && (PatC == '\\' || PatC == '/') &&
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
  जाव (S != End) {
    जइ (P == PEnd)
      ;
    अण्णहा जइ (*P == '*') {
      // The non-* substring on the left of '*' matches the tail of S. Save the
      // positions to be used by backtracking if we see a mismatch later.
      SegmentBegin = ++P;
      SavedS = S;
      SavedB = B;
      अनुवर्त;
    } अण्णहा जइ (*P == '[') {
      जइ (Brackets[B].Bytes[uint8_t(*S)]) {
        P = Pat.data() + Brackets[B++].NextOffset;
        ++S;
        अनुवर्त;
      }
    } अण्णहा जइ (*P == '\\') {
      जइ (matchChar(*++P, *S, SlashAgnostic)) {
        ++P;
        ++S;
        अनुवर्त;
      }
    } अण्णहा जइ (matchChar(*P, *S, SlashAgnostic) || *P == '?') {
      ++P;
      ++S;
      अनुवर्त;
    }
    जइ (!SegmentBegin)
      प्रत्यागम मिच्छ;
    // We have seen a '*'. Backtrack to the saved positions. Shift the S
    // position to probe the next starting position in the segment.
    P = SegmentBegin;
    S = ++SavedS;
    B = SavedB;
  }
  // All bytes in Str have been matched. Return true if the rest part of Pat is
  // empty or contains only '*'.
  प्रत्यागम getPat().find_first_not_of('*', P - Pat.data()) == std::string::npos;
}
