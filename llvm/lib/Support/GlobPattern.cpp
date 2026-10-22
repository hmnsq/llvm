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

#ضمن "llvm/Support/GlobPattern.h"
#ضمن "llvm/ADT/StringRef.h"
#ضمن "llvm/Support/Errc.h"

using نطاق llvm;

ساكن constexpr حرف PrefixMetacharacters[] = "?*[{\\";
ساكن constexpr حرف SuffixMetacharacters[] = "?*[]{}\\";
ساكن constexpr حرف PrefixMetacharactersWithSlash[] = "?*[{\\/";
ساكن constexpr حرف SuffixMetacharactersWithSlash[] = "?*[]{}\\/";

// Expands character ranges and returns a bitmap.
// For example, "a-cf-hz" is expanded to "abcfghz".
ساكن Expected<BitVector> expand(StringRef S, StringRef Original) {
  BitVector BV(256, שקר);

  // Expand X-Y.
  עבור (;;) {
    אם (S.size() < 3)
      اكسر;

    uint8_t Start = S[0];
    uint8_t End = S[2];

    // If it doesn't start with something like X-Y,
    // consume the first character and proceed.
    אם (S[1] != '-') {
      BV[Start] = אמת;
      S = S.substr(1);
      استمر;
    }

    // It must be in the form of X-Y.
    // Validate it and then interpret the range.
    אם (Start > End)
      החזר make_error<StringError>("invalid glob pattern: " + Original,
                                     errc::invalid_argument);

    עבור (صحيح C = Start; C <= End; ++C)
      BV[(uint8_t)C] = אמת;
    S = S.substr(3);
  }

  עבור (حرف C : S)
    BV[(uint8_t)C] = אמת;
  החזר BV;
}

// Identify brace expansions in S and return the list of patterns they expand
// into.
ساكن Expected<SmallVector<std::string, 1>>
parseBraceExpansions(StringRef S, std::optional<size_t> MaxSubPatterns) {
  SmallVector<std::string> SubPatterns = {S.str()};
  אם (!MaxSubPatterns || !S.contains('{'))
    החזר std::move(SubPatterns);

  بنية BraceExpansion {
    size_t Start;
    size_t Length;
    SmallVector<StringRef, 2> Terms;
  };
  SmallVector<BraceExpansion, 0> BraceExpansions;

  BraceExpansion *CurrentBE = nullptr;
  size_t TermBegin;
  עבור (size_t I = 0, E = S.size(); I != E; ++I) {
    אם (S[I] == '[') {
      I = S.find(']', I + 2);
      אם (I == std::string::npos)
        החזר make_error<StringError>("invalid glob pattern, unmatched '['",
                                       errc::invalid_argument);
    } אחרת אם (S[I] == '{') {
      אם (CurrentBE)
        החזר make_error<StringError>(
            "nested brace expansions are not supported",
            errc::invalid_argument);
      CurrentBE = &BraceExpansions.emplace_back();
      CurrentBE->Start = I;
      TermBegin = I + 1;
    } אחרת אם (S[I] == ',') {
      אם (!CurrentBE)
        استمر;
      CurrentBE->Terms.push_back(S.substr(TermBegin, I - TermBegin));
      TermBegin = I + 1;
    } אחרת אם (S[I] == '}') {
      אם (!CurrentBE)
        استمر;
      אם (CurrentBE->Terms.empty())
        החזר make_error<StringError>(
            "empty or singleton brace expansions are not supported",
            errc::invalid_argument);
      CurrentBE->Terms.push_back(S.substr(TermBegin, I - TermBegin));
      CurrentBE->Length = I - CurrentBE->Start + 1;
      CurrentBE = nullptr;
    } אחרת אם (S[I] == '\\') {
      אם (++I == E)
        החזר make_error<StringError>("invalid glob pattern, stray '\\'",
                                       errc::invalid_argument);
    }
  }
  אם (CurrentBE)
    החזר make_error<StringError>("incomplete brace expansion",
                                   errc::invalid_argument);

  size_t NumSubPatterns = 1;
  עבור (auto &BE : BraceExpansions) {
    אם (NumSubPatterns > std::numeric_limits<size_t>::max() / BE.Terms.size()) {
      NumSubPatterns = std::numeric_limits<size_t>::max();
      اكسر;
    }
    NumSubPatterns *= BE.Terms.size();
  }
  אם (NumSubPatterns > *MaxSubPatterns)
    החזר make_error<StringError>("too many brace expansions",
                                   errc::invalid_argument);
  // Replace brace expansions in reverse order so that we don't invalidate
  // earlier start indices
  עבור (auto &BE : reverse(BraceExpansions)) {
    SmallVector<std::string> OrigSubPatterns;
    std::swap(SubPatterns, OrigSubPatterns);
    עבור (StringRef Term : BE.Terms)
      עבור (StringRef Orig : OrigSubPatterns)
        SubPatterns.emplace_back(Orig).replace(BE.Start, BE.Length, Term);
  }
  החזר std::move(SubPatterns);
}

ساكن StringRef maxPlainSubstring(StringRef S, منطقي SlashAgnostic) {
  ثابت حرف *Metas =
      SlashAgnostic ? PrefixMetacharactersWithSlash : PrefixMetacharacters;
  StringRef Best;
  כלעוד (!S.empty()) {
    size_t PrefixSize = S.find_first_of(Metas);
    אם (PrefixSize == std::string::npos)
      PrefixSize = S.size();

    אם (Best.size() < PrefixSize)
      Best = S.take_front(PrefixSize);

    S = S.drop_front(PrefixSize);

    // It's impossible, as the first and last characters of the input string
    // must be Glob special characters, otherwise they would be parts of
    // the prefix or the suffix.
    أكد(!S.empty());

    بدل (S.front()) {
    حالة '\\':
      S = S.drop_front(2);
      اكسر;
    حالة '[': {
      // Drop '[' and the first character which can be ']'.
      S = S.drop_front(2);
      size_t EndBracket = S.find_first_of("]");
      // Should not be possible, SubGlobPattern::create should fail on invalid
      // pattern before we get here.
      أكد(EndBracket != std::string::npos);
      S = S.drop_front(EndBracket + 1);
      اكسر;
    }
    حالة '{':
      // TODO: implement.
      // Fallback to whatever is best for now.
      החזר Best;
    افتراضي:
      S = S.drop_front(1);
    }
  }

  החזר Best;
}

Expected<GlobPattern> GlobPattern::create(StringRef S,
                                          std::optional<size_t> MaxSubPatterns,
                                          منطقي SlashAgnostic) {
  GlobPattern Pat;
  Pat.SlashAgnostic = SlashAgnostic;
  Pat.Pattern = S;

  ثابت حرف *PrefixMetas =
      SlashAgnostic ? PrefixMetacharactersWithSlash : PrefixMetacharacters;
  ثابت حرف *SuffixMetas =
      SlashAgnostic ? SuffixMetacharactersWithSlash : SuffixMetacharacters;

  // Store the prefix that does not contain any metacharacter.
  Pat.PrefixSize = S.find_first_of(PrefixMetas);
  אם (Pat.PrefixSize == std::string::npos) {
    Pat.PrefixSize = S.size();
    החזר Pat;
  }
  S = S.substr(Pat.PrefixSize);

  // Just in case we stop on unmatched opening brackets.
  size_t SuffixStart = S.find_last_of(SuffixMetas);
  أكد(SuffixStart != std::string::npos);
  אם (S[SuffixStart] == '\\')
    ++SuffixStart;
  אם (SuffixStart < S.size())
    ++SuffixStart;
  Pat.SuffixSize = S.size() - SuffixStart;
  S = S.substr(0, SuffixStart);

  SmallVector<std::string, 1> SubPats;
  אם (auto Err = parseBraceExpansions(S, MaxSubPatterns).moveInto(SubPats))
    החזר std::move(Err);
  עבור (StringRef SubPat : SubPats) {
    auto SubGlobOrErr = SubGlobPattern::create(SubPat, SlashAgnostic);
    אם (!SubGlobOrErr)
      החזר SubGlobOrErr.takeError();
    Pat.SubGlobs.push_back(*SubGlobOrErr);
  }

  החזר Pat;
}

std::optional<std::string> GlobPattern::asLiteral() ثابت {
  // The prefix and suffix are metacharacter-free by construction, so whether
  // this pattern denotes a single string is decided by the sub-patterns, which
  // recorded it while parsing. No sub-pattern at all means there was no
  // metacharacter; more than one means brace expansion produced a choice.
  אם (SubGlobs.size() > 1 || (SubGlobs.size() == 1 && !SubGlobs[0].isLiteral()))
    החזר std::nullopt;

  std::string Literal;
  Literal.reserve(Pattern.size());
  עבור (size_t I = 0, E = Pattern.size(); I != E; ++I) {
    אם (Pattern[I] == '\\' && I + 1 != E)
      ++I;
    Literal.push_back(Pattern[I]);
  }

  // In slash-agnostic mode '/' and '\\' match each other, so a pattern holding
  // either matches more than one string. A pattern with no sub-pattern cannot
  // reach here holding one, as both are prefix metacharacters in that mode.
  אם (SlashAgnostic &&
      StringRef(Literal).find_first_of("/\\") != StringRef::npos)
    החזר std::nullopt;

  החזר Literal;
}

Expected<GlobPattern::SubGlobPattern>
GlobPattern::SubGlobPattern::create(StringRef S, منطقي SlashAgnostic) {
  SubGlobPattern Pat;

  // Parse brackets.
  Pat.Pat.assign(S.begin(), S.end());
  עבור (size_t I = 0, E = S.size(); I != E; ++I) {
    אם (S[I] == '[') {
      // ']' is allowed as the first character of a character class. '[]' is
      // invalid. So, just skip the first character.
      ++I;
      size_t J = S.find(']', I + 1);
      אם (J == StringRef::npos)
        החזר make_error<StringError>("invalid glob pattern, unmatched '['",
                                       errc::invalid_argument);
      StringRef Chars = S.substr(I, J - I);
      منطقي Invert = S[I] == '^' || S[I] == '!';
      אם (Invert)
        Chars = Chars.drop_front();
      Expected<BitVector> BVOrErr = expand(Chars, S);
      אם (!BVOrErr)
        החזר BVOrErr.takeError();
      BitVector &BV = *BVOrErr;
      אם (SlashAgnostic && (BV['\\'] || BV['/'])) {
        BV.set('\\');
        BV.set('/');
      }
      אם (Invert)
        BV.flip();
      Pat.Brackets.push_back(Bracket{J + 1, std::move(BV)});
      I = J;
    } אחרת אם (S[I] == '\\') {
      אם (++I == E)
        החזר make_error<StringError>("invalid glob pattern, stray '\\'",
                                       errc::invalid_argument);
    } אחרת אם (S[I] == '*' || S[I] == '?') {
      Pat.HasWildcard = אמת;
    }
  }
  החזר Pat;
}

StringRef GlobPattern::longest_substr() ثابت {
  החזר maxPlainSubstring(Pattern.drop_front(PrefixSize).drop_back(SuffixSize),
                           SlashAgnostic);
}

منطقي GlobPattern::match(StringRef S) ثابت {
  אם (!S.consume_front(prefix()))
    החזר שקר;
  אם (!S.consume_back(suffix()))
    החזר שקר;
  אם (SubGlobs.empty() && S.empty())
    החזר אמת;
  עבור (auto &Glob : SubGlobs)
    אם (Glob.match(S, SlashAgnostic))
      החזר אמת;
  החזר שקר;
}

ساكن منطقي matchChar(حرف PatC, حرف QueryC, منطقي SlashAgnostic) {
  אם (PatC == QueryC)
    החזר אמת;
  החזר SlashAgnostic && (PatC == '\\' || PatC == '/') &&
         (QueryC == '\\' || QueryC == '/');
}

// Factor the pattern into segments split by '*'. The segment is matched
// sequentianlly by finding the first occurrence past the end of the previous
// match.
منطقي GlobPattern::SubGlobPattern::match(StringRef Str,
                                        منطقي SlashAgnostic) ثابت {
  ثابت حرف *P = Pat.data(), *SegmentBegin = nullptr, *S = Str.data(),
             *SavedS = S;
  ثابت حرف *ثابت PEnd = P + Pat.size(), *ثابت End = S + Str.size();
  size_t B = 0, SavedB = 0;
  כלעוד (S != End) {
    אם (P == PEnd)
      ;
    אחרת אם (*P == '*') {
      // The non-* substring on the left of '*' matches the tail of S. Save the
      // positions to be used by backtracking if we see a mismatch later.
      SegmentBegin = ++P;
      SavedS = S;
      SavedB = B;
      استمر;
    } אחרת אם (*P == '[') {
      אם (Brackets[B].Bytes[uint8_t(*S)]) {
        P = Pat.data() + Brackets[B++].NextOffset;
        ++S;
        استمر;
      }
    } אחרת אם (*P == '\\') {
      אם (matchChar(*++P, *S, SlashAgnostic)) {
        ++P;
        ++S;
        استمر;
      }
    } אחרת אם (matchChar(*P, *S, SlashAgnostic) || *P == '?') {
      ++P;
      ++S;
      استمر;
    }
    אם (!SegmentBegin)
      החזר שקר;
    // We have seen a '*'. Backtrack to the saved positions. Shift the S
    // position to probe the next starting position in the segment.
    P = SegmentBegin;
    S = ++SavedS;
    B = SavedB;
  }
  // All bytes in Str have been matched. Return true if the rest part of Pat is
  // empty or contains only '*'.
  החזר getPat().find_first_not_of('*', P - Pat.data()) == std::string::npos;
}
