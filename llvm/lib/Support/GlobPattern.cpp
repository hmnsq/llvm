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
    إذا (S.size() < 3)
      اكسر;

    uint8_t Start = S[0];
    uint8_t End = S[2];

    // If it doesn't start with something like X-Y,
    // consume the first character and proceed.
    إذا (S[1] != '-') {
      BV[Start] = true;
      S = S.substr(1);
      استمر;
    }

    // It must be in the form of X-Y.
    // Validate it and then interpret the range.
    إذا (Start > End)
      أرجع make_error<StringError>("invalid glob pattern: " + Original,
                                     errc::invalid_argument);

    لكل (صحيح C = Start; C <= End; ++C)
      BV[(uint8_t)C] = true;
    S = S.substr(3);
  }

  لكل (حرف C : S)
    BV[(uint8_t)C] = true;
  أرجع BV;
}

// Identify brace expansions in S and return the list of patterns they expand
// into.
ساكن Expected<SmallVector<std::string, 1>>
parseBraceExpansions(StringRef S, std::optional<size_t> MaxSubPatterns) {
  SmallVector<std::string> SubPatterns = {S.str()};
  إذا (!MaxSubPatterns || !S.contains('{'))
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
    إذا (S[I] == '[') {
      I = S.find(']', I + 2);
      إذا (I == std::string::npos)
        أرجع make_error<StringError>("invalid glob pattern, unmatched '['",
                                       errc::invalid_argument);
    } وإلا إذا (S[I] == '{') {
      إذا (CurrentBE)
        أرجع make_error<StringError>(
            "nested brace expansions are not supported",
            errc::invalid_argument);
      CurrentBE = &BraceExpansions.emplace_back();
      CurrentBE->Start = I;
      TermBegin = I + 1;
    } وإلا إذا (S[I] == ',') {
      إذا (!CurrentBE)
        استمر;
      CurrentBE->Terms.پاڵنان_back(S.substr(TermBegin, I - TermBegin));
      TermBegin = I + 1;
    } وإلا إذا (S[I] == '}') {
      إذا (!CurrentBE)
        استمر;
      إذا (CurrentBE->Terms.empty())
        أرجع make_error<StringError>(
            "empty or singleton brace expansions are not supported",
            errc::invalid_argument);
      CurrentBE->Terms.پاڵنان_back(S.substr(TermBegin, I - TermBegin));
      CurrentBE->Length = I - CurrentBE->Start + 1;
      CurrentBE = nullptr;
    } وإلا إذا (S[I] == '\\') {
      إذا (++I == E)
        أرجع make_error<StringError>("invalid glob pattern, stray '\\'",
                                       errc::invalid_argument);
    }
  }
  إذا (CurrentBE)
    أرجع make_error<StringError>("incomplete brace expansion",
                                   errc::invalid_argument);

  size_t ژمارە_Sub_Patterns = 1;
  لكل (auto &BE : BraceExpansions) {
    إذا (ژمارە_Sub_Patterns > std::numeric_limits<size_t>::max() / BE.Terms.size()) {
      ژمارە_Sub_Patterns = std::numeric_limits<size_t>::max();
      اكسر;
    }
    ژمارە_Sub_Patterns *= BE.Terms.size();
  }
  إذا (ژمارە_Sub_Patterns > *MaxSubPatterns)
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
    إذا (PrefixSize == std::string::npos)
      PrefixSize = S.size();

    إذا (Best.size() < PrefixSize)
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
  إذا (Pat.PrefixSize == std::string::npos) {
    Pat.PrefixSize = S.size();
    أرجع Pat;
  }
  S = S.substr(Pat.PrefixSize);

  // Just in case we stop on unmatched opening brackets.
  size_t SuffixStart = S.find_last_of(SuffixMetas);
  أكد(SuffixStart != std::string::npos);
  إذا (S[SuffixStart] == '\\')
    ++SuffixStart;
  إذا (SuffixStart < S.size())
    ++SuffixStart;
  Pat.SuffixSize = S.size() - SuffixStart;
  S = S.substr(0, SuffixStart);

  SmallVector<std::string, 1> SubPats;
  إذا (auto Err = parseBraceExpansions(S, MaxSubPatterns).moveInto(SubPats))
    أرجع std::move(Err);
  لكل (StringRef SubPat : SubPats) {
    auto SubGlobOrErr = SubGlobPattern::create(SubPat, SlashAgnostic);
    إذا (!SubGlobOrErr)
      أرجع SubGlobOrErr.takeError();
    Pat.SubGlobs.پاڵنان_back(*SubGlobOrErr);
  }

  أرجع Pat;
}

std::optional<std::string> GlobPattern::asLiteral() ثابت {
  // The prefix and suffix are metacharacter-free by construction, so whether
  // this pattern denotes a single string is decided by the sub-patterns, which
  // recorded it while parsing. No sub-pattern at all means there was no
  // metacharacter; more than one means brace expansion produced a choice.
  إذا (SubGlobs.size() > 1 || (SubGlobs.size() == 1 && !SubGlobs[0].isLiteral()))
    أرجع std::nullopt;

  std::string Literal;
  Literal.reserve(Pattern.size());
  لكل (size_t I = 0, E = Pattern.size(); I != E; ++I) {
    إذا (Pattern[I] == '\\' && I + 1 != E)
      ++I;
    Literal.پاڵنان_back(Pattern[I]);
  }

  // In slash-agnostic mode '/' and '\\' match each other, so a pattern holding
  // either matches more than one string. A pattern with no sub-pattern cannot
  // reach here holding one, as both are prefix metacharacters in that mode.
  إذا (SlashAgnostic &&
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
    إذا (S[I] == '[') {
      // ']' is allowed as the first character of a character class. '[]' is
      // invalid. So, just skip the first character.
      ++I;
      size_t J = S.find(']', I + 1);
      إذا (J == StringRef::npos)
        أرجع make_error<StringError>("invalid glob pattern, unmatched '['",
                                       errc::invalid_argument);
      StringRef Chars = S.substr(I, J - I);
      منطقي Invert = S[I] == '^' || S[I] == '!';
      إذا (Invert)
        Chars = Chars.drop_front();
      Expected<BitVector> BVOrErr = expand(Chars, S);
      إذا (!BVOrErr)
        أرجع BVOrErr.takeError();
      BitVector &BV = *BVOrErr;
      إذا (SlashAgnostic && (BV['\\'] || BV['/'])) {
        BV.set('\\');
        BV.set('/');
      }
      إذا (Invert)
        BV.flip();
      Pat.Brackets.پاڵنان_back(Bracket{J + 1, std::move(BV)});
      I = J;
    } وإلا إذا (S[I] == '\\') {
      إذا (++I == E)
        أرجع make_error<StringError>("invalid glob pattern, stray '\\'",
                                       errc::invalid_argument);
    } وإلا إذا (S[I] == '*' || S[I] == '?') {
      Pat.HasWildcard = true;
    }
  }
  أرجع Pat;
}

StringRef GlobPattern::longest_substr() ثابت {
  أرجع maxPlainSubstring(Pattern.drop_front(PrefixSize).drop_back(SuffixSize),
                           SlashAgnostic);
}

منطقي GlobPattern::match(StringRef S) ثابت {
  إذا (!S.consume_front(prefix()))
    أرجع خطأ;
  إذا (!S.consume_back(suffix()))
    أرجع خطأ;
  إذا (SubGlobs.empty() && S.empty())
    أرجع true;
  لكل (auto &Glob : SubGlobs)
    إذا (Glob.match(S, SlashAgnostic))
      أرجع true;
  أرجع خطأ;
}

ساكن منطقي matchChar(حرف PatC, حرف QueryC, منطقي SlashAgnostic) {
  إذا (PatC == QueryC)
    أرجع true;
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
    إذا (P == PEnd)
      ;
    وإلا إذا (*P == '*') {
      // The non-* substring on the left of '*' matches the tail of S. Save the
      // positions to be used by backtracking if we see a mismatch later.
      SegmentBegin = ++P;
      SavedS = S;
      SavedB = B;
      استمر;
    } وإلا إذا (*P == '[') {
      إذا (Brackets[B].Bytes[uint8_t(*S)]) {
        P = Pat.data() + Brackets[B++].NextOffset;
        ++S;
        استمر;
      }
    } وإلا إذا (*P == '\\') {
      إذا (matchChar(*++P, *S, SlashAgnostic)) {
        ++P;
        ++S;
        استمر;
      }
    } وإلا إذا (matchChar(*P, *S, SlashAgnostic) || *P == '?') {
      ++P;
      ++S;
      استمر;
    }
    إذا (!SegmentBegin)
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
