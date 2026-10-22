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

#شامل "llvm/Support/GlobPattern.h"
#شامل "llvm/ADT/StringRef.h"
#شامل "llvm/Support/Errc.h"

using فضاینام llvm;

ایستا constexpr نویسه PrefixMetacharacters[] = "?*[{\\";
ایستا constexpr نویسه SuffixMetacharacters[] = "?*[]{}\\";
ایستا constexpr نویسه PrefixMetacharactersWithSlash[] = "?*[{\\/";
ایستا constexpr نویسه SuffixMetacharactersWithSlash[] = "?*[]{}\\/";

// Expands character ranges and returns a bitmap.
// For example, "a-cf-hz" is expanded to "abcfghz".
ایستا Expected<BitVector> expand(StringRef S, StringRef Original) {
  BitVector BV(256, نادرست);

  // Expand X-Y.
  برای (;;) {
    اگر (S.size() < 3)
      بشکن;

    uint8_t Start = S[0];
    uint8_t End = S[2];

    // If it doesn't start with something like X-Y,
    // consume the first character and proceed.
    اگر (S[1] != '-') {
      BV[Start] = درست;
      S = S.substr(1);
      ادامه;
    }

    // It must be in the form of X-Y.
    // Validate it and then interpret the range.
    اگر (Start > End)
      بازگشت make_error<StringError>("invalid glob pattern: " + Original,
                                     errc::invalid_argument);

    برای (صحیح C = Start; C <= End; ++C)
      BV[(uint8_t)C] = درست;
    S = S.substr(3);
  }

  برای (نویسه C : S)
    BV[(uint8_t)C] = درست;
  بازگشت BV;
}

// Identify brace expansions in S and return the list of patterns they expand
// into.
ایستا Expected<SmallVector<std::string, 1>>
parseBraceExpansions(StringRef S, std::optional<size_t> MaxSubPatterns) {
  SmallVector<std::string> SubPatterns = {S.str()};
  اگر (!MaxSubPatterns || !S.contains('{'))
    بازگشت std::move(SubPatterns);

  ساختار BraceExpansion {
    size_t Start;
    size_t Length;
    SmallVector<StringRef, 2> Terms;
  };
  SmallVector<BraceExpansion, 0> BraceExpansions;

  BraceExpansion *CurrentBE = nullptr;
  size_t TermBegin;
  برای (size_t I = 0, E = S.size(); I != E; ++I) {
    اگر (S[I] == '[') {
      I = S.find(']', I + 2);
      اگر (I == std::string::npos)
        بازگشت make_error<StringError>("invalid glob pattern, unmatched '['",
                                       errc::invalid_argument);
    } وگرنه اگر (S[I] == '{') {
      اگر (CurrentBE)
        بازگشت make_error<StringError>(
            "nested brace expansions are not supported",
            errc::invalid_argument);
      CurrentBE = &BraceExpansions.emplace_back();
      CurrentBE->Start = I;
      TermBegin = I + 1;
    } وگرنه اگر (S[I] == ',') {
      اگر (!CurrentBE)
        ادامه;
      CurrentBE->Terms.افزودن_back(S.substr(TermBegin, I - TermBegin));
      TermBegin = I + 1;
    } وگرنه اگر (S[I] == '}') {
      اگر (!CurrentBE)
        ادامه;
      اگر (CurrentBE->Terms.empty())
        بازگشت make_error<StringError>(
            "empty or singleton brace expansions are not supported",
            errc::invalid_argument);
      CurrentBE->Terms.افزودن_back(S.substr(TermBegin, I - TermBegin));
      CurrentBE->Length = I - CurrentBE->Start + 1;
      CurrentBE = nullptr;
    } وگرنه اگر (S[I] == '\\') {
      اگر (++I == E)
        بازگشت make_error<StringError>("invalid glob pattern, stray '\\'",
                                       errc::invalid_argument);
    }
  }
  اگر (CurrentBE)
    بازگشت make_error<StringError>("incomplete brace expansion",
                                   errc::invalid_argument);

  size_t رقم_Sub_Patterns = 1;
  برای (auto &BE : BraceExpansions) {
    اگر (رقم_Sub_Patterns > std::numeric_limits<size_t>::max() / BE.Terms.size()) {
      رقم_Sub_Patterns = std::numeric_limits<size_t>::max();
      بشکن;
    }
    رقم_Sub_Patterns *= BE.Terms.size();
  }
  اگر (رقم_Sub_Patterns > *MaxSubPatterns)
    بازگشت make_error<StringError>("too many brace expansions",
                                   errc::invalid_argument);
  // Replace brace expansions in reverse order so that we don't invalidate
  // earlier start indices
  برای (auto &BE : reverse(BraceExpansions)) {
    SmallVector<std::string> OrigSubPatterns;
    std::swap(SubPatterns, OrigSubPatterns);
    برای (StringRef Term : BE.Terms)
      برای (StringRef Orig : OrigSubPatterns)
        SubPatterns.emplace_back(Orig).replace(BE.Start, BE.Length, Term);
  }
  بازگشت std::move(SubPatterns);
}

ایستا StringRef maxPlainSubstring(StringRef S, منطقی SlashAgnostic) {
  ثابت نویسه *Metas =
      SlashAgnostic ? PrefixMetacharactersWithSlash : PrefixMetacharacters;
  StringRef Best;
  وقتی (!S.empty()) {
    size_t PrefixSize = S.find_first_of(Metas);
    اگر (PrefixSize == std::string::npos)
      PrefixSize = S.size();

    اگر (Best.size() < PrefixSize)
      Best = S.take_front(PrefixSize);

    S = S.drop_front(PrefixSize);

    // It's impossible, as the first and last characters of the input string
    // must be Glob special characters, otherwise they would be parts of
    // the prefix or the suffix.
    أكد(!S.empty());

    انتخاب (S.front()) {
    حالت '\\':
      S = S.drop_front(2);
      بشکن;
    حالت '[': {
      // Drop '[' and the first character which can be ']'.
      S = S.drop_front(2);
      size_t EndBracket = S.find_first_of("]");
      // Should not be possible, SubGlobPattern::create should fail on invalid
      // pattern before we get here.
      أكد(EndBracket != std::string::npos);
      S = S.drop_front(EndBracket + 1);
      بشکن;
    }
    حالت '{':
      // TODO: implement.
      // Fallback to whatever is best for now.
      بازگشت Best;
    پیشفرض:
      S = S.drop_front(1);
    }
  }

  بازگشت Best;
}

Expected<GlobPattern> GlobPattern::create(StringRef S,
                                          std::optional<size_t> MaxSubPatterns,
                                          منطقی SlashAgnostic) {
  GlobPattern Pat;
  Pat.SlashAgnostic = SlashAgnostic;
  Pat.Pattern = S;

  ثابت نویسه *PrefixMetas =
      SlashAgnostic ? PrefixMetacharactersWithSlash : PrefixMetacharacters;
  ثابت نویسه *SuffixMetas =
      SlashAgnostic ? SuffixMetacharactersWithSlash : SuffixMetacharacters;

  // Store the prefix that does not contain any metacharacter.
  Pat.PrefixSize = S.find_first_of(PrefixMetas);
  اگر (Pat.PrefixSize == std::string::npos) {
    Pat.PrefixSize = S.size();
    بازگشت Pat;
  }
  S = S.substr(Pat.PrefixSize);

  // Just in case we stop on unmatched opening brackets.
  size_t SuffixStart = S.find_last_of(SuffixMetas);
  أكد(SuffixStart != std::string::npos);
  اگر (S[SuffixStart] == '\\')
    ++SuffixStart;
  اگر (SuffixStart < S.size())
    ++SuffixStart;
  Pat.SuffixSize = S.size() - SuffixStart;
  S = S.substr(0, SuffixStart);

  SmallVector<std::string, 1> SubPats;
  اگر (auto Err = parseBraceExpansions(S, MaxSubPatterns).moveInto(SubPats))
    بازگشت std::move(Err);
  برای (StringRef SubPat : SubPats) {
    auto SubGlobOrErr = SubGlobPattern::create(SubPat, SlashAgnostic);
    اگر (!SubGlobOrErr)
      بازگشت SubGlobOrErr.takeError();
    Pat.SubGlobs.افزودن_back(*SubGlobOrErr);
  }

  بازگشت Pat;
}

std::optional<std::string> GlobPattern::asLiteral() ثابت {
  // The prefix and suffix are metacharacter-free by construction, so whether
  // this pattern denotes a single string is decided by the sub-patterns, which
  // recorded it while parsing. No sub-pattern at all means there was no
  // metacharacter; more than one means brace expansion produced a choice.
  اگر (SubGlobs.size() > 1 || (SubGlobs.size() == 1 && !SubGlobs[0].isLiteral()))
    بازگشت std::nullopt;

  std::string Literal;
  Literal.reserve(Pattern.size());
  برای (size_t I = 0, E = Pattern.size(); I != E; ++I) {
    اگر (Pattern[I] == '\\' && I + 1 != E)
      ++I;
    Literal.افزودن_back(Pattern[I]);
  }

  // In slash-agnostic mode '/' and '\\' match each other, so a pattern holding
  // either matches more than one string. A pattern with no sub-pattern cannot
  // reach here holding one, as both are prefix metacharacters in that mode.
  اگر (SlashAgnostic &&
      StringRef(Literal).find_first_of("/\\") != StringRef::npos)
    بازگشت std::nullopt;

  بازگشت Literal;
}

Expected<GlobPattern::SubGlobPattern>
GlobPattern::SubGlobPattern::create(StringRef S, منطقی SlashAgnostic) {
  SubGlobPattern Pat;

  // Parse brackets.
  Pat.Pat.assign(S.begin(), S.end());
  برای (size_t I = 0, E = S.size(); I != E; ++I) {
    اگر (S[I] == '[') {
      // ']' is allowed as the first character of a character class. '[]' is
      // invalid. So, just skip the first character.
      ++I;
      size_t J = S.find(']', I + 1);
      اگر (J == StringRef::npos)
        بازگشت make_error<StringError>("invalid glob pattern, unmatched '['",
                                       errc::invalid_argument);
      StringRef Chars = S.substr(I, J - I);
      منطقی Invert = S[I] == '^' || S[I] == '!';
      اگر (Invert)
        Chars = Chars.drop_front();
      Expected<BitVector> BVOrErr = expand(Chars, S);
      اگر (!BVOrErr)
        بازگشت BVOrErr.takeError();
      BitVector &BV = *BVOrErr;
      اگر (SlashAgnostic && (BV['\\'] || BV['/'])) {
        BV.set('\\');
        BV.set('/');
      }
      اگر (Invert)
        BV.flip();
      Pat.Brackets.افزودن_back(Bracket{J + 1, std::move(BV)});
      I = J;
    } وگرنه اگر (S[I] == '\\') {
      اگر (++I == E)
        بازگشت make_error<StringError>("invalid glob pattern, stray '\\'",
                                       errc::invalid_argument);
    } وگرنه اگر (S[I] == '*' || S[I] == '?') {
      Pat.HasWildcard = درست;
    }
  }
  بازگشت Pat;
}

StringRef GlobPattern::longest_substr() ثابت {
  بازگشت maxPlainSubstring(Pattern.drop_front(PrefixSize).drop_back(SuffixSize),
                           SlashAgnostic);
}

منطقی GlobPattern::match(StringRef S) ثابت {
  اگر (!S.consume_front(prefix()))
    بازگشت نادرست;
  اگر (!S.consume_back(suffix()))
    بازگشت نادرست;
  اگر (SubGlobs.empty() && S.empty())
    بازگشت درست;
  برای (auto &Glob : SubGlobs)
    اگر (Glob.match(S, SlashAgnostic))
      بازگشت درست;
  بازگشت نادرست;
}

ایستا منطقی matchChar(نویسه PatC, نویسه QueryC, منطقی SlashAgnostic) {
  اگر (PatC == QueryC)
    بازگشت درست;
  بازگشت SlashAgnostic && (PatC == '\\' || PatC == '/') &&
         (QueryC == '\\' || QueryC == '/');
}

// Factor the pattern into segments split by '*'. The segment is matched
// sequentianlly by finding the first occurrence past the end of the previous
// match.
منطقی GlobPattern::SubGlobPattern::match(StringRef Str,
                                        منطقی SlashAgnostic) ثابت {
  ثابت نویسه *P = Pat.data(), *SegmentBegin = nullptr, *S = Str.data(),
             *SavedS = S;
  ثابت نویسه *ثابت PEnd = P + Pat.size(), *ثابت End = S + Str.size();
  size_t B = 0, SavedB = 0;
  وقتی (S != End) {
    اگر (P == PEnd)
      ;
    وگرنه اگر (*P == '*') {
      // The non-* substring on the left of '*' matches the tail of S. Save the
      // positions to be used by backtracking if we see a mismatch later.
      SegmentBegin = ++P;
      SavedS = S;
      SavedB = B;
      ادامه;
    } وگرنه اگر (*P == '[') {
      اگر (Brackets[B].Bytes[uint8_t(*S)]) {
        P = Pat.data() + Brackets[B++].NextOffset;
        ++S;
        ادامه;
      }
    } وگرنه اگر (*P == '\\') {
      اگر (matchChar(*++P, *S, SlashAgnostic)) {
        ++P;
        ++S;
        ادامه;
      }
    } وگرنه اگر (matchChar(*P, *S, SlashAgnostic) || *P == '?') {
      ++P;
      ++S;
      ادامه;
    }
    اگر (!SegmentBegin)
      بازگشت نادرست;
    // We have seen a '*'. Backtrack to the saved positions. Shift the S
    // position to probe the next starting position in the segment.
    P = SegmentBegin;
    S = ++SavedS;
    B = SavedB;
  }
  // All bytes in Str have been matched. Return true if the rest part of Pat is
  // empty or contains only '*'.
  بازگشت getPat().find_first_not_of('*', P - Pat.data()) == std::string::npos;
}
