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
  BitVector BV(256, خطأ);

  // Expand X-Y.
  لكل (;;) {
    𐡄𐡍 (S.size() < 3)
      اكسر;

    uint8_t Start = S[0];
    uint8_t End = S[2];

    // If it doesn't start with something like X-Y,
    // consume the first character and proceed.
    𐡄𐡍 (S[1] != '-') {
      BV[Start] = 𐡒𐡔𐡈;
      S = S.substr(1);
      استمر;
    }

    // It must be in the form of X-Y.
    // Validate it and then interpret the range.
    𐡄𐡍 (Start > End)
      أرجع make_error<StringError>("invalid glob pattern: " + Original,
                                     errc::invalid_argument);

    لكل (صحيح C = Start; C <= End; ++C)
      BV[(uint8_t)C] = 𐡒𐡔𐡈;
    S = S.substr(3);
  }

  لكل (حرف C : S)
    BV[(uint8_t)C] = 𐡒𐡔𐡈;
  أرجع BV;
}

// Identify brace expansions in S and return the list of patterns they expand
// into.
ساكن Expected<SmallVector<std::string, 1>>
parseBraceExpansions(StringRef S, std::optional<size_t> MaxSubPatterns) {
  SmallVector<std::string> SubPatterns = {S.str()};
  𐡄𐡍 (!MaxSubPatterns || !S.contains('{'))
    أرجع std::move(SubPatterns);

  بنية BraceExpansion {
    size_t Start;
    size_t Length;
    SmallVector<StringRef, 2> Terms;
  };
  SmallVector<BraceExpansion, 0> BraceExpansions;

  BraceExpansion *CurrentBE = nullptr;
  size_t TermBegin;
  لكل (size_t I = 0, E = S.size(); I != E; ++I) {
    𐡄𐡍 (S[I] == '[') {
      I = S.find(']', I + 2);
      𐡄𐡍 (I == std::string::npos)
        أرجع make_error<StringError>("invalid glob pattern, unmatched '['",
                                       errc::invalid_argument);
    } وإلا 𐡄𐡍 (S[I] == '{') {
      𐡄𐡍 (CurrentBE)
        أرجع make_error<StringError>(
            "nested brace expansions are not supported",
            errc::invalid_argument);
      CurrentBE = &BraceExpansions.emplace_back();
      CurrentBE->Start = I;
      TermBegin = I + 1;
    } وإلا 𐡄𐡍 (S[I] == ',') {
      𐡄𐡍 (!CurrentBE)
        استمر;
      CurrentBE->Terms.push_back(S.substr(TermBegin, I - TermBegin));
      TermBegin = I + 1;
    } وإلا 𐡄𐡍 (S[I] == '}') {
      𐡄𐡍 (!CurrentBE)
        استمر;
      𐡄𐡍 (CurrentBE->Terms.empty())
        أرجع make_error<StringError>(
            "empty or singleton brace expansions are not supported",
            errc::invalid_argument);
      CurrentBE->Terms.push_back(S.substr(TermBegin, I - TermBegin));
      CurrentBE->Length = I - CurrentBE->Start + 1;
      CurrentBE = nullptr;
    } وإلا 𐡄𐡍 (S[I] == '\\') {
      𐡄𐡍 (++I == E)
        أرجع make_error<StringError>("invalid glob pattern, stray '\\'",
                                       errc::invalid_argument);
    }
  }
  𐡄𐡍 (CurrentBE)
    أرجع make_error<StringError>("incomplete brace expansion",
                                   errc::invalid_argument);

  size_t NumSubPatterns = 1;
  لكل (auto &BE : BraceExpansions) {
    𐡄𐡍 (NumSubPatterns > std::numeric_limits<size_t>::max() / BE.Terms.size()) {
      NumSubPatterns = std::numeric_limits<size_t>::max();
      اكسر;
    }
    NumSubPatterns *= BE.Terms.size();
  }
  𐡄𐡍 (NumSubPatterns > *MaxSubPatterns)
    أرجع make_error<StringError>("too many brace expansions",
                                   errc::invalid_argument);
  // Replace brace expansions in reverse order so that we don't invalidate
  // earlier start indices
  لكل (auto &BE : reverse(BraceExpansions)) {
    SmallVector<std::string> OrigSubPatterns;
    std::swap(SubPatterns, OrigSubPatterns);
    لكل (StringRef Term : BE.Terms)
      لكل (StringRef Orig : OrigSubPatterns)
        SubPatterns.emplace_back(Orig).replace(BE.Start, BE.Length, Term);
  }
  أرجع std::move(SubPatterns);
}

ساكن StringRef maxPlainSubstring(StringRef S, منطقي SlashAgnostic) {
  ثابت حرف *Metas =
      SlashAgnostic ? PrefixMetacharactersWithSlash : PrefixMetacharacters;
  StringRef Best;
  طالما (!S.empty()) {
    size_t PrefixSize = S.find_first_of(Metas);
    𐡄𐡍 (PrefixSize == std::string::npos)
      PrefixSize = S.size();

    𐡄𐡍 (Best.size() < PrefixSize)
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
      أرجع Best;
    افتراضي:
      S = S.drop_front(1);
    }
  }

  أرجع Best;
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
  𐡄𐡍 (Pat.PrefixSize == std::string::npos) {
    Pat.PrefixSize = S.size();
    أرجع Pat;
  }
  S = S.substr(Pat.PrefixSize);

  // Just in case we stop on unmatched opening brackets.
  size_t SuffixStart = S.find_last_of(SuffixMetas);
  أكد(SuffixStart != std::string::npos);
  𐡄𐡍 (S[SuffixStart] == '\\')
    ++SuffixStart;
  𐡄𐡍 (SuffixStart < S.size())
    ++SuffixStart;
  Pat.SuffixSize = S.size() - SuffixStart;
  S = S.substr(0, SuffixStart);

  SmallVector<std::string, 1> SubPats;
  𐡄𐡍 (auto Err = parseBraceExpansions(S, MaxSubPatterns).moveInto(SubPats))
    أرجع std::move(Err);
  لكل (StringRef SubPat : SubPats) {
    auto SubGlobOrErr = SubGlobPattern::create(SubPat, SlashAgnostic);
    𐡄𐡍 (!SubGlobOrErr)
      أرجع SubGlobOrErr.takeError();
    Pat.SubGlobs.push_back(*SubGlobOrErr);
  }

  أرجع Pat;
}

std::optional<std::string> GlobPattern::asLiteral() ثابت {
  // The prefix and suffix are metacharacter-free by construction, so whether
  // this pattern denotes a single string is decided by the sub-patterns, which
  // recorded it while parsing. No sub-pattern at all means there was no
  // metacharacter; more than one means brace expansion produced a choice.
  𐡄𐡍 (SubGlobs.size() > 1 || (SubGlobs.size() == 1 && !SubGlobs[0].isLiteral()))
    أرجع std::nullopt;

  std::string Literal;
  Literal.reserve(Pattern.size());
  لكل (size_t I = 0, E = Pattern.size(); I != E; ++I) {
    𐡄𐡍 (Pattern[I] == '\\' && I + 1 != E)
      ++I;
    Literal.push_back(Pattern[I]);
  }

  // In slash-agnostic mode '/' and '\\' match each other, so a pattern holding
  // either matches more than one string. A pattern with no sub-pattern cannot
  // reach here holding one, as both are prefix metacharacters in that mode.
  𐡄𐡍 (SlashAgnostic &&
      StringRef(Literal).find_first_of("/\\") != StringRef::npos)
    أرجع std::nullopt;

  أرجع Literal;
}

Expected<GlobPattern::SubGlobPattern>
GlobPattern::SubGlobPattern::create(StringRef S, منطقي SlashAgnostic) {
  SubGlobPattern Pat;

  // Parse brackets.
  Pat.Pat.assign(S.begin(), S.end());
  لكل (size_t I = 0, E = S.size(); I != E; ++I) {
    𐡄𐡍 (S[I] == '[') {
      // ']' is allowed as the first character of a character class. '[]' is
      // invalid. So, just skip the first character.
      ++I;
      size_t J = S.find(']', I + 1);
      𐡄𐡍 (J == StringRef::npos)
        أرجع make_error<StringError>("invalid glob pattern, unmatched '['",
                                       errc::invalid_argument);
      StringRef Chars = S.substr(I, J - I);
      منطقي Invert = S[I] == '^' || S[I] == '!';
      𐡄𐡍 (Invert)
        Chars = Chars.drop_front();
      Expected<BitVector> BVOrErr = expand(Chars, S);
      𐡄𐡍 (!BVOrErr)
        أرجع BVOrErr.takeError();
      BitVector &BV = *BVOrErr;
      𐡄𐡍 (SlashAgnostic && (BV['\\'] || BV['/'])) {
        BV.set('\\');
        BV.set('/');
      }
      𐡄𐡍 (Invert)
        BV.flip();
      Pat.Brackets.push_back(Bracket{J + 1, std::move(BV)});
      I = J;
    } وإلا 𐡄𐡍 (S[I] == '\\') {
      𐡄𐡍 (++I == E)
        أرجع make_error<StringError>("invalid glob pattern, stray '\\'",
                                       errc::invalid_argument);
    } وإلا 𐡄𐡍 (S[I] == '*' || S[I] == '?') {
      Pat.HasWildcard = 𐡒𐡔𐡈;
    }
  }
  أرجع Pat;
}

StringRef GlobPattern::longest_substr() ثابت {
  أرجع maxPlainSubstring(Pattern.drop_front(PrefixSize).drop_back(SuffixSize),
                           SlashAgnostic);
}

منطقي GlobPattern::match(StringRef S) ثابت {
  𐡄𐡍 (!S.consume_front(prefix()))
    أرجع خطأ;
  𐡄𐡍 (!S.consume_back(suffix()))
    أرجع خطأ;
  𐡄𐡍 (SubGlobs.empty() && S.empty())
    أرجع 𐡒𐡔𐡈;
  لكل (auto &Glob : SubGlobs)
    𐡄𐡍 (Glob.match(S, SlashAgnostic))
      أرجع 𐡒𐡔𐡈;
  أرجع خطأ;
}

ساكن منطقي matchChar(حرف PatC, حرف QueryC, منطقي SlashAgnostic) {
  𐡄𐡍 (PatC == QueryC)
    أرجع 𐡒𐡔𐡈;
  أرجع SlashAgnostic && (PatC == '\\' || PatC == '/') &&
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
  طالما (S != End) {
    𐡄𐡍 (P == PEnd)
      ;
    وإلا 𐡄𐡍 (*P == '*') {
      // The non-* substring on the left of '*' matches the tail of S. Save the
      // positions to be used by backtracking if we see a mismatch later.
      SegmentBegin = ++P;
      SavedS = S;
      SavedB = B;
      استمر;
    } وإلا 𐡄𐡍 (*P == '[') {
      𐡄𐡍 (Brackets[B].Bytes[uint8_t(*S)]) {
        P = Pat.data() + Brackets[B++].NextOffset;
        ++S;
        استمر;
      }
    } وإلا 𐡄𐡍 (*P == '\\') {
      𐡄𐡍 (matchChar(*++P, *S, SlashAgnostic)) {
        ++P;
        ++S;
        استمر;
      }
    } وإلا 𐡄𐡍 (matchChar(*P, *S, SlashAgnostic) || *P == '?') {
      ++P;
      ++S;
      استمر;
    }
    𐡄𐡍 (!SegmentBegin)
      أرجع خطأ;
    // We have seen a '*'. Backtrack to the saved positions. Shift the S
    // position to probe the next starting position in the segment.
    P = SegmentBegin;
    S = ++SavedS;
    B = SavedB;
  }
  // All bytes in Str have been matched. Return true if the rest part of Pat is
  // empty or contains only '*'.
  أرجع getPat().find_first_not_of('*', P - Pat.data()) == std::string::npos;
}
