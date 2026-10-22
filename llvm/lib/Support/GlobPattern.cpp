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

#അന്തര്ഭാവ "llvm/Support/GlobPattern.h"
#അന്തര്ഭാവ "llvm/ADT/StringRef.h"
#അന്തര്ഭാവ "llvm/Support/Errc.h"

using നാമാകാശ llvm;

സ്ഥിതം constexpr അക്ഷരം PrefixMetacharacters[] = "?*[{\\";
സ്ഥിതം constexpr അക്ഷരം SuffixMetacharacters[] = "?*[]{}\\";
സ്ഥിതം constexpr അക്ഷരം PrefixMetacharactersWithSlash[] = "?*[{\\/";
സ്ഥിതം constexpr അക്ഷരം SuffixMetacharactersWithSlash[] = "?*[]{}\\/";

// Expands character ranges and returns a bitmap.
// For example, "a-cf-hz" is expanded to "abcfghz".
സ്ഥിതം Expected<BitVector> expand(StringRef S, StringRef Original) {
  BitVector BV(256, അസത്യം);

  // Expand X-Y.
  വേണ്ടി (;;) {
    എങ്കിൽ (S.size() < 3)
      നിർത്തുക;

    uint8_t Start = S[0];
    uint8_t End = S[2];

    // If it doesn't start with something like X-Y,
    // consume the first character and proceed.
    എങ്കിൽ (S[1] != '-') {
      BV[Start] = സത്യം;
      S = S.substr(1);
      തുടരുക;
    }

    // It must be in the form of X-Y.
    // Validate it and then interpret the range.
    എങ്കിൽ (Start > End)
      തിരികെ make_error<StringError>("invalid glob pattern: " + Original,
                                     errc::invalid_argument);

    വേണ്ടി (പൂർണ്ണസംഖ്യ C = Start; C <= End; ++C)
      BV[(uint8_t)C] = സത്യം;
    S = S.substr(3);
  }

  വേണ്ടി (അക്ഷരം C : S)
    BV[(uint8_t)C] = സത്യം;
  തിരികെ BV;
}

// Identify brace expansions in S and return the list of patterns they expand
// into.
സ്ഥിതം Expected<SmallVector<std::string, 1>>
parseBraceExpansions(StringRef S, std::optional<size_t> MaxSubPatterns) {
  SmallVector<std::string> SubPatterns = {S.str()};
  എങ്കിൽ (!MaxSubPatterns || !S.contains('{'))
    തിരികെ std::move(SubPatterns);

  ഘടന BraceExpansion {
    size_t Start;
    size_t Length;
    SmallVector<StringRef, 2> Terms;
  };
  SmallVector<BraceExpansion, 0> BraceExpansions;

  BraceExpansion *CurrentBE = nullptr;
  size_t TermBegin;
  വേണ്ടി (size_t I = 0, E = S.size(); I != E; ++I) {
    എങ്കിൽ (S[I] == '[') {
      I = S.find(']', I + 2);
      എങ്കിൽ (I == std::string::npos)
        തിരികെ make_error<StringError>("invalid glob pattern, unmatched '['",
                                       errc::invalid_argument);
    } അല്ലെങ്കിൽ എങ്കിൽ (S[I] == '{') {
      എങ്കിൽ (CurrentBE)
        തിരികെ make_error<StringError>(
            "nested brace expansions are not supported",
            errc::invalid_argument);
      CurrentBE = &BraceExpansions.emplace_back();
      CurrentBE->Start = I;
      TermBegin = I + 1;
    } അല്ലെങ്കിൽ എങ്കിൽ (S[I] == ',') {
      എങ്കിൽ (!CurrentBE)
        തുടരുക;
      CurrentBE->Terms.തള്ളുക_back(S.substr(TermBegin, I - TermBegin));
      TermBegin = I + 1;
    } അല്ലെങ്കിൽ എങ്കിൽ (S[I] == '}') {
      എങ്കിൽ (!CurrentBE)
        തുടരുക;
      എങ്കിൽ (CurrentBE->Terms.empty())
        തിരികെ make_error<StringError>(
            "empty or singleton brace expansions are not supported",
            errc::invalid_argument);
      CurrentBE->Terms.തള്ളുക_back(S.substr(TermBegin, I - TermBegin));
      CurrentBE->Length = I - CurrentBE->Start + 1;
      CurrentBE = nullptr;
    } അല്ലെങ്കിൽ എങ്കിൽ (S[I] == '\\') {
      എങ്കിൽ (++I == E)
        തിരികെ make_error<StringError>("invalid glob pattern, stray '\\'",
                                       errc::invalid_argument);
    }
  }
  എങ്കിൽ (CurrentBE)
    തിരികെ make_error<StringError>("incomplete brace expansion",
                                   errc::invalid_argument);

  size_t അക്കം_Sub_Patterns = 1;
  വേണ്ടി (auto &BE : BraceExpansions) {
    എങ്കിൽ (അക്കം_Sub_Patterns > std::numeric_limits<size_t>::max() / BE.Terms.size()) {
      അക്കം_Sub_Patterns = std::numeric_limits<size_t>::max();
      നിർത്തുക;
    }
    അക്കം_Sub_Patterns *= BE.Terms.size();
  }
  എങ്കിൽ (അക്കം_Sub_Patterns > *MaxSubPatterns)
    തിരികെ make_error<StringError>("too many brace expansions",
                                   errc::invalid_argument);
  // Replace brace expansions in reverse order so that we don't invalidate
  // earlier start indices
  വേണ്ടി (auto &BE : reverse(BraceExpansions)) {
    SmallVector<std::string> OrigSubPatterns;
    std::swap(SubPatterns, OrigSubPatterns);
    വേണ്ടി (StringRef Term : BE.Terms)
      വേണ്ടി (StringRef Orig : OrigSubPatterns)
        SubPatterns.emplace_back(Orig).replace(BE.Start, BE.Length, Term);
  }
  തിരികെ std::move(SubPatterns);
}

സ്ഥിതം StringRef maxPlainSubstring(StringRef S, തർക്കം SlashAgnostic) {
  സ്ഥിരം അക്ഷരം *Metas =
      SlashAgnostic ? PrefixMetacharactersWithSlash : PrefixMetacharacters;
  StringRef Best;
  ആകുമ്പോൾ (!S.empty()) {
    size_t PrefixSize = S.find_first_of(Metas);
    എങ്കിൽ (PrefixSize == std::string::npos)
      PrefixSize = S.size();

    എങ്കിൽ (Best.size() < PrefixSize)
      Best = S.take_front(PrefixSize);

    S = S.drop_front(PrefixSize);

    // It's impossible, as the first and last characters of the input string
    // must be Glob special characters, otherwise they would be parts of
    // the prefix or the suffix.
    പ്രതിജ്ഞാ(!S.empty());

    തിരഞ്ഞെടുപ്പ് (S.front()) {
    അവസ്ഥ '\\':
      S = S.drop_front(2);
      നിർത്തുക;
    അവസ്ഥ '[': {
      // Drop '[' and the first character which can be ']'.
      S = S.drop_front(2);
      size_t EndBracket = S.find_first_of("]");
      // Should not be possible, SubGlobPattern::create should fail on invalid
      // pattern before we get here.
      പ്രതിജ്ഞാ(EndBracket != std::string::npos);
      S = S.drop_front(EndBracket + 1);
      നിർത്തുക;
    }
    അവസ്ഥ '{':
      // TODO: implement.
      // Fallback to whatever is best for now.
      തിരികെ Best;
    സ്ഥിരസ്ഥിതി:
      S = S.drop_front(1);
    }
  }

  തിരികെ Best;
}

Expected<GlobPattern> GlobPattern::create(StringRef S,
                                          std::optional<size_t> MaxSubPatterns,
                                          തർക്കം SlashAgnostic) {
  GlobPattern Pat;
  Pat.SlashAgnostic = SlashAgnostic;
  Pat.Pattern = S;

  സ്ഥിരം അക്ഷരം *PrefixMetas =
      SlashAgnostic ? PrefixMetacharactersWithSlash : PrefixMetacharacters;
  സ്ഥിരം അക്ഷരം *SuffixMetas =
      SlashAgnostic ? SuffixMetacharactersWithSlash : SuffixMetacharacters;

  // Store the prefix that does not contain any metacharacter.
  Pat.PrefixSize = S.find_first_of(PrefixMetas);
  എങ്കിൽ (Pat.PrefixSize == std::string::npos) {
    Pat.PrefixSize = S.size();
    തിരികെ Pat;
  }
  S = S.substr(Pat.PrefixSize);

  // Just in case we stop on unmatched opening brackets.
  size_t SuffixStart = S.find_last_of(SuffixMetas);
  പ്രതിജ്ഞാ(SuffixStart != std::string::npos);
  എങ്കിൽ (S[SuffixStart] == '\\')
    ++SuffixStart;
  എങ്കിൽ (SuffixStart < S.size())
    ++SuffixStart;
  Pat.SuffixSize = S.size() - SuffixStart;
  S = S.substr(0, SuffixStart);

  SmallVector<std::string, 1> SubPats;
  എങ്കിൽ (auto Err = parseBraceExpansions(S, MaxSubPatterns).moveInto(SubPats))
    തിരികെ std::move(Err);
  വേണ്ടി (StringRef SubPat : SubPats) {
    auto SubGlobOrErr = SubGlobPattern::create(SubPat, SlashAgnostic);
    എങ്കിൽ (!SubGlobOrErr)
      തിരികെ SubGlobOrErr.takeError();
    Pat.SubGlobs.തള്ളുക_back(*SubGlobOrErr);
  }

  തിരികെ Pat;
}

std::optional<std::string> GlobPattern::asLiteral() സ്ഥിരം {
  // The prefix and suffix are metacharacter-free by construction, so whether
  // this pattern denotes a single string is decided by the sub-patterns, which
  // recorded it while parsing. No sub-pattern at all means there was no
  // metacharacter; more than one means brace expansion produced a choice.
  എങ്കിൽ (SubGlobs.size() > 1 || (SubGlobs.size() == 1 && !SubGlobs[0].isLiteral()))
    തിരികെ std::nullopt;

  std::string Literal;
  Literal.reserve(Pattern.size());
  വേണ്ടി (size_t I = 0, E = Pattern.size(); I != E; ++I) {
    എങ്കിൽ (Pattern[I] == '\\' && I + 1 != E)
      ++I;
    Literal.തള്ളുക_back(Pattern[I]);
  }

  // In slash-agnostic mode '/' and '\\' match each other, so a pattern holding
  // either matches more than one string. A pattern with no sub-pattern cannot
  // reach here holding one, as both are prefix metacharacters in that mode.
  എങ്കിൽ (SlashAgnostic &&
      StringRef(Literal).find_first_of("/\\") != StringRef::npos)
    തിരികെ std::nullopt;

  തിരികെ Literal;
}

Expected<GlobPattern::SubGlobPattern>
GlobPattern::SubGlobPattern::create(StringRef S, തർക്കം SlashAgnostic) {
  SubGlobPattern Pat;

  // Parse brackets.
  Pat.Pat.assign(S.begin(), S.end());
  വേണ്ടി (size_t I = 0, E = S.size(); I != E; ++I) {
    എങ്കിൽ (S[I] == '[') {
      // ']' is allowed as the first character of a character class. '[]' is
      // invalid. So, just skip the first character.
      ++I;
      size_t J = S.find(']', I + 1);
      എങ്കിൽ (J == StringRef::npos)
        തിരികെ make_error<StringError>("invalid glob pattern, unmatched '['",
                                       errc::invalid_argument);
      StringRef Chars = S.substr(I, J - I);
      തർക്കം Invert = S[I] == '^' || S[I] == '!';
      എങ്കിൽ (Invert)
        Chars = Chars.drop_front();
      Expected<BitVector> BVOrErr = expand(Chars, S);
      എങ്കിൽ (!BVOrErr)
        തിരികെ BVOrErr.takeError();
      BitVector &BV = *BVOrErr;
      എങ്കിൽ (SlashAgnostic && (BV['\\'] || BV['/'])) {
        BV.set('\\');
        BV.set('/');
      }
      എങ്കിൽ (Invert)
        BV.flip();
      Pat.Brackets.തള്ളുക_back(Bracket{J + 1, std::move(BV)});
      I = J;
    } അല്ലെങ്കിൽ എങ്കിൽ (S[I] == '\\') {
      എങ്കിൽ (++I == E)
        തിരികെ make_error<StringError>("invalid glob pattern, stray '\\'",
                                       errc::invalid_argument);
    } അല്ലെങ്കിൽ എങ്കിൽ (S[I] == '*' || S[I] == '?') {
      Pat.HasWildcard = സത്യം;
    }
  }
  തിരികെ Pat;
}

StringRef GlobPattern::longest_substr() സ്ഥിരം {
  തിരികെ maxPlainSubstring(Pattern.drop_front(PrefixSize).drop_back(SuffixSize),
                           SlashAgnostic);
}

തർക്കം GlobPattern::match(StringRef S) സ്ഥിരം {
  എങ്കിൽ (!S.consume_front(prefix()))
    തിരികെ അസത്യം;
  എങ്കിൽ (!S.consume_back(suffix()))
    തിരികെ അസത്യം;
  എങ്കിൽ (SubGlobs.empty() && S.empty())
    തിരികെ സത്യം;
  വേണ്ടി (auto &Glob : SubGlobs)
    എങ്കിൽ (Glob.match(S, SlashAgnostic))
      തിരികെ സത്യം;
  തിരികെ അസത്യം;
}

സ്ഥിതം തർക്കം matchChar(അക്ഷരം PatC, അക്ഷരം QueryC, തർക്കം SlashAgnostic) {
  എങ്കിൽ (PatC == QueryC)
    തിരികെ സത്യം;
  തിരികെ SlashAgnostic && (PatC == '\\' || PatC == '/') &&
         (QueryC == '\\' || QueryC == '/');
}

// Factor the pattern into segments split by '*'. The segment is matched
// sequentianlly by finding the first occurrence past the end of the previous
// match.
തർക്കം GlobPattern::SubGlobPattern::match(StringRef Str,
                                        തർക്കം SlashAgnostic) സ്ഥിരം {
  സ്ഥിരം അക്ഷരം *P = Pat.data(), *SegmentBegin = nullptr, *S = Str.data(),
             *SavedS = S;
  സ്ഥിരം അക്ഷരം *സ്ഥിരം PEnd = P + Pat.size(), *സ്ഥിരം End = S + Str.size();
  size_t B = 0, SavedB = 0;
  ആകുമ്പോൾ (S != End) {
    എങ്കിൽ (P == PEnd)
      ;
    അല്ലെങ്കിൽ എങ്കിൽ (*P == '*') {
      // The non-* substring on the left of '*' matches the tail of S. Save the
      // positions to be used by backtracking if we see a mismatch later.
      SegmentBegin = ++P;
      SavedS = S;
      SavedB = B;
      തുടരുക;
    } അല്ലെങ്കിൽ എങ്കിൽ (*P == '[') {
      എങ്കിൽ (Brackets[B].Bytes[uint8_t(*S)]) {
        P = Pat.data() + Brackets[B++].NextOffset;
        ++S;
        തുടരുക;
      }
    } അല്ലെങ്കിൽ എങ്കിൽ (*P == '\\') {
      എങ്കിൽ (matchChar(*++P, *S, SlashAgnostic)) {
        ++P;
        ++S;
        തുടരുക;
      }
    } അല്ലെങ്കിൽ എങ്കിൽ (matchChar(*P, *S, SlashAgnostic) || *P == '?') {
      ++P;
      ++S;
      തുടരുക;
    }
    എങ്കിൽ (!SegmentBegin)
      തിരികെ അസത്യം;
    // We have seen a '*'. Backtrack to the saved positions. Shift the S
    // position to probe the next starting position in the segment.
    P = SegmentBegin;
    S = ++SavedS;
    B = SavedB;
  }
  // All bytes in Str have been matched. Return true if the rest part of Pat is
  // empty or contains only '*'.
  തിരികെ getPat().find_first_not_of('*', P - Pat.data()) == std::string::npos;
}
