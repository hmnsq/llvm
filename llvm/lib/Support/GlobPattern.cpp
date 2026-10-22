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

#అన్తర్భావ "llvm/Support/GlobPattern.h"
#అన్తర్భావ "llvm/ADT/StringRef.h"
#అన్తర్భావ "llvm/Support/Errc.h"

ఉపయోగించి నామాకాశ llvm;

స్థిర constexpr అక్షరం ఉపసర్గ_Metacharacters[] = "?*[{\\";
స్థిర constexpr అక్షరం SuffixMetacharacters[] = "?*[]{}\\";
స్థిర constexpr అక్షరం ఉపసర్గ_Metacharacters_With_Slash[] = "?*[{\\/";
స్థిర constexpr అక్షరం SuffixMetacharactersWithSlash[] = "?*[]{}\\/";

// Expands character ranges and returns a bitmap.
// For example, "a-cf-hz" is expanded to "abcfghz".
స్థిర Expected<బిట్టు_సదిశ> expand(StringRef S, StringRef Original) {
  బిట్టు_సదిశ BV(256, అబద్ధం);

  // Expand X-Y.
  కొరకు (;;) {
    ఒకవేళ (S.కొలత_2() < 3)
      ఆపు;

    uint8_t ప్రారంభం = S[0];
    uint8_t చివర_2 = S[2];

    // If it doesn't start with something like X-Y,
    // consume the first character and proceed.
    ఒకవేళ (S[1] != '-') {
      BV[ప్రారంభం] = నిజం;
      S = S.substr(1);
      కొనసాగు;
    }

    // It must be in the form of X-Y.
    // Validate it and then interpret the range.
    ఒకవేళ (ప్రారంభం > చివర_2)
      తిరిగి make_error<StringError>("invalid glob pattern: " + Original,
                                     errc::invalid_వాదం);

    కొరకు (పూర్ణాంకం C = ప్రారంభం; C <= చివర_2; ++C)
      BV[(uint8_t)C] = నిజం;
    S = S.substr(3);
  }

  కొరకు (అక్షరం C : S)
    BV[(uint8_t)C] = నిజం;
  తిరిగి BV;
}

// Identify brace expansions in S and return the list of patterns they expand
// into.
స్థిర Expected<Small_సదిశ<std::string, 1>>
parseBraceExpansions(StringRef S, std::optional<size_t> MaxSubPatterns) {
  Small_సదిశ<std::string> SubPatterns = {S.str()};
  ఒకవేళ (!MaxSubPatterns || !S.కలిగియుంది('{'))
    తిరిగి std::move(SubPatterns);

  నిర్మాణం BraceExpansion {
    size_t ప్రారంభం;
    size_t నిడివి;
    Small_సదిశ<StringRef, 2> Terms;
  };
  Small_సదిశ<BraceExpansion, 0> BraceExpansions;

  BraceExpansion *CurrentBE = శూన్యసూచిక;
  size_t పదం_Begin;
  కొరకు (size_t I = 0, E = S.కొలత_2(); I != E; ++I) {
    ఒకవేళ (S[I] == '[') {
      I = S.కనుగొను(']', I + 2);
      ఒకవేళ (I == std::string::npos)
        తిరిగి make_error<StringError>("invalid glob pattern, unmatched '['",
                                       errc::invalid_వాదం);
    } లేకపోతే ఒకవేళ (S[I] == '{') {
      ఒకవేళ (CurrentBE)
        తిరిగి make_error<StringError>(
            "nested brace expansions are not supported",
            errc::invalid_వాదం);
      CurrentBE = &BraceExpansions.emplace_వెనుక();
      CurrentBE->ప్రారంభం = I;
      పదం_Begin = I + 1;
    } లేకపోతే ఒకవేళ (S[I] == ',') {
      ఒకవేళ (!CurrentBE)
        కొనసాగు;
      CurrentBE->Terms.నెట్టు_వెనుక(S.substr(పదం_Begin, I - పదం_Begin));
      పదం_Begin = I + 1;
    } లేకపోతే ఒకవేళ (S[I] == '}') {
      ఒకవేళ (!CurrentBE)
        కొనసాగు;
      ఒకవేళ (CurrentBE->Terms.ఖాళీలు())
        తిరిగి make_error<StringError>(
            "empty or singleton brace expansions are not supported",
            errc::invalid_వాదం);
      CurrentBE->Terms.నెట్టు_వెనుక(S.substr(పదం_Begin, I - పదం_Begin));
      CurrentBE->నిడివి = I - CurrentBE->ప్రారంభం + 1;
      CurrentBE = శూన్యసూచిక;
    } లేకపోతే ఒకవేళ (S[I] == '\\') {
      ఒకవేళ (++I == E)
        తిరిగి make_error<StringError>("invalid glob pattern, stray '\\'",
                                       errc::invalid_వాదం);
    }
  }
  ఒకవేళ (CurrentBE)
    తిరిగి make_error<StringError>("incomplete brace expansion",
                                   errc::invalid_వాదం);

  size_t అంకె_Sub_Patterns = 1;
  కొరకు (స్వయంచాలక &BE : BraceExpansions) {
    ఒకవేళ (అంకె_Sub_Patterns > std::numeric_limits<size_t>::max() / BE.Terms.కొలత_2()) {
      అంకె_Sub_Patterns = std::numeric_limits<size_t>::max();
      ఆపు;
    }
    అంకె_Sub_Patterns *= BE.Terms.కొలత_2();
  }
  ఒకవేళ (అంకె_Sub_Patterns > *MaxSubPatterns)
    తిరిగి make_error<StringError>("too many brace expansions",
                                   errc::invalid_వాదం);
  // Replace brace expansions in reverse order so that we don't invalidate
  // earlier start indices
  కొరకు (స్వయంచాలక &BE : తిరగేయి(BraceExpansions)) {
    Small_సదిశ<std::string> OrigSubPatterns;
    std::మార్పిడి(SubPatterns, OrigSubPatterns);
    కొరకు (StringRef పదం : BE.Terms)
      కొరకు (StringRef Orig : OrigSubPatterns)
        SubPatterns.emplace_వెనుక(Orig).replace(BE.ప్రారంభం, BE.నిడివి, పదం);
  }
  తిరిగి std::move(SubPatterns);
}

స్థిర StringRef maxPlainSubstring(StringRef S, తార్కిక SlashAgnostic) {
  స్థిరరాశి అక్షరం *Metas =
      SlashAgnostic ? ఉపసర్గ_Metacharacters_With_Slash : ఉపసర్గ_Metacharacters;
  StringRef ఉత్తమ;
  అయితే (!S.ఖాళీలు()) {
    size_t ఉపసర్గ_కొలత = S.కనుగొను_మొదటి_యొక్క(Metas);
    ఒకవేళ (ఉపసర్గ_కొలత == std::string::npos)
      ఉపసర్గ_కొలత = S.కొలత_2();

    ఒకవేళ (ఉత్తమ.కొలత_2() < ఉపసర్గ_కొలత)
      ఉత్తమ = S.take_front(ఉపసర్గ_కొలత);

    S = S.drop_front(ఉపసర్గ_కొలత);

    // It's impossible, as the first and last characters of the input string
    // must be Glob special characters, otherwise they would be parts of
    // the prefix or the suffix.
    ప్రతిజ్ఞా(!S.ఖాళీలు());

    ఎంపిక (S.front()) {
    స్థితి '\\':
      S = S.drop_front(2);
      ఆపు;
    స్థితి '[': {
      // Drop '[' and the first character which can be ']'.
      S = S.drop_front(2);
      size_t చివర_Bracket = S.కనుగొను_మొదటి_యొక్క("]");
      // Should not be possible, SubGlobPattern::create should fail on invalid
      // pattern before we get here.
      ప్రతిజ్ఞా(చివర_Bracket != std::string::npos);
      S = S.drop_front(చివర_Bracket + 1);
      ఆపు;
    }
    స్థితి '{':
      // TODO: implement.
      // Fallback to whatever is best for now.
      తిరిగి ఉత్తమ;
    అప్రమేయ:
      S = S.drop_front(1);
    }
  }

  తిరిగి ఉత్తమ;
}

Expected<Glob_నమూనా> Glob_నమూనా::create(StringRef S,
                                          std::optional<size_t> MaxSubPatterns,
                                          తార్కిక SlashAgnostic) {
  Glob_నమూనా Pat;
  Pat.SlashAgnostic = SlashAgnostic;
  Pat.నమూనా = S;

  స్థిరరాశి అక్షరం *ఉపసర్గ_Metas =
      SlashAgnostic ? ఉపసర్గ_Metacharacters_With_Slash : ఉపసర్గ_Metacharacters;
  స్థిరరాశి అక్షరం *SuffixMetas =
      SlashAgnostic ? SuffixMetacharactersWithSlash : SuffixMetacharacters;

  // Store the prefix that does not contain any metacharacter.
  Pat.ఉపసర్గ_కొలత = S.కనుగొను_మొదటి_యొక్క(ఉపసర్గ_Metas);
  ఒకవేళ (Pat.ఉపసర్గ_కొలత == std::string::npos) {
    Pat.ఉపసర్గ_కొలత = S.కొలత_2();
    తిరిగి Pat;
  }
  S = S.substr(Pat.ఉపసర్గ_కొలత);

  // Just in case we stop on unmatched opening brackets.
  size_t Suffix_ప్రారంభం = S.కనుగొను_చివరి_యొక్క(SuffixMetas);
  ప్రతిజ్ఞా(Suffix_ప్రారంభం != std::string::npos);
  ఒకవేళ (S[Suffix_ప్రారంభం] == '\\')
    ++Suffix_ప్రారంభం;
  ఒకవేళ (Suffix_ప్రారంభం < S.కొలత_2())
    ++Suffix_ప్రారంభం;
  Pat.Suffix_కొలత = S.కొలత_2() - Suffix_ప్రారంభం;
  S = S.substr(0, Suffix_ప్రారంభం);

  Small_సదిశ<std::string, 1> SubPats;
  ఒకవేళ (స్వయంచాలక Err = parseBraceExpansions(S, MaxSubPatterns).moveInto(SubPats))
    తిరిగి std::move(Err);
  కొరకు (StringRef SubPat : SubPats) {
    స్వయంచాలక SubGlobOrErr = Sub_Glob_నమూనా::create(SubPat, SlashAgnostic);
    ఒకవేళ (!SubGlobOrErr)
      తిరిగి SubGlobOrErr.takeError();
    Pat.SubGlobs.నెట్టు_వెనుక(*SubGlobOrErr);
  }

  తిరిగి Pat;
}

std::optional<std::string> Glob_నమూనా::asLiteral() స్థిరరాశి {
  // The prefix and suffix are metacharacter-free by construction, so whether
  // this pattern denotes a single string is decided by the sub-patterns, which
  // recorded it while parsing. No sub-pattern at all means there was no
  // metacharacter; more than one means brace expansion produced a choice.
  ఒకవేళ (SubGlobs.కొలత_2() > 1 || (SubGlobs.కొలత_2() == 1 && !SubGlobs[0].isLiteral()))
    తిరిగి std::nullopt;

  std::string Literal;
  Literal.reserve(నమూనా.కొలత_2());
  కొరకు (size_t I = 0, E = నమూనా.కొలత_2(); I != E; ++I) {
    ఒకవేళ (నమూనా[I] == '\\' && I + 1 != E)
      ++I;
    Literal.నెట్టు_వెనుక(నమూనా[I]);
  }

  // In slash-agnostic mode '/' and '\\' match each other, so a pattern holding
  // either matches more than one string. A pattern with no sub-pattern cannot
  // reach here holding one, as both are prefix metacharacters in that mode.
  ఒకవేళ (SlashAgnostic &&
      StringRef(Literal).కనుగొను_మొదటి_యొక్క("/\\") != StringRef::npos)
    తిరిగి std::nullopt;

  తిరిగి Literal;
}

Expected<Glob_నమూనా::Sub_Glob_నమూనా>
Glob_నమూనా::Sub_Glob_నమూనా::create(StringRef S, తార్కిక SlashAgnostic) {
  Sub_Glob_నమూనా Pat;

  // Parse brackets.
  Pat.Pat.కేటాయించు(S.begin(), S.చివర());
  కొరకు (size_t I = 0, E = S.కొలత_2(); I != E; ++I) {
    ఒకవేళ (S[I] == '[') {
      // ']' is allowed as the first character of a character class. '[]' is
      // invalid. So, just skip the first character.
      ++I;
      size_t J = S.కనుగొను(']', I + 1);
      ఒకవేళ (J == StringRef::npos)
        తిరిగి make_error<StringError>("invalid glob pattern, unmatched '['",
                                       errc::invalid_వాదం);
      StringRef Chars = S.substr(I, J - I);
      తార్కిక Invert = S[I] == '^' || S[I] == '!';
      ఒకవేళ (Invert)
        Chars = Chars.drop_front();
      Expected<బిట్టు_సదిశ> BVOrErr = expand(Chars, S);
      ఒకవేళ (!BVOrErr)
        తిరిగి BVOrErr.takeError();
      బిట్టు_సదిశ &BV = *BVOrErr;
      ఒకవేళ (SlashAgnostic && (BV['\\'] || BV['/'])) {
        BV.సమితి('\\');
        BV.సమితి('/');
      }
      ఒకవేళ (Invert)
        BV.flip();
      Pat.Brackets.నెట్టు_వెనుక(Bracket{J + 1, std::move(BV)});
      I = J;
    } లేకపోతే ఒకవేళ (S[I] == '\\') {
      ఒకవేళ (++I == E)
        తిరిగి make_error<StringError>("invalid glob pattern, stray '\\'",
                                       errc::invalid_వాదం);
    } లేకపోతే ఒకవేళ (S[I] == '*' || S[I] == '?') {
      Pat.HasWildcard = నిజం;
    }
  }
  తిరిగి Pat;
}

StringRef Glob_నమూనా::longest_substr() స్థిరరాశి {
  తిరిగి maxPlainSubstring(నమూనా.drop_front(ఉపసర్గ_కొలత).drop_వెనుక(Suffix_కొలత),
                           SlashAgnostic);
}

తార్కిక Glob_నమూనా::సరిపోలిక(StringRef S) స్థిరరాశి {
  ఒకవేళ (!S.consume_front(ఉపసర్గ_2()))
    తిరిగి అబద్ధం;
  ఒకవేళ (!S.consume_వెనుక(suffix()))
    తిరిగి అబద్ధం;
  ఒకవేళ (SubGlobs.ఖాళీలు() && S.ఖాళీలు())
    తిరిగి నిజం;
  కొరకు (స్వయంచాలక &Glob : SubGlobs)
    ఒకవేళ (Glob.సరిపోలిక(S, SlashAgnostic))
      తిరిగి నిజం;
  తిరిగి అబద్ధం;
}

స్థిర తార్కిక సరిపోలిక_Char(అక్షరం PatC, అక్షరం QueryC, తార్కిక SlashAgnostic) {
  ఒకవేళ (PatC == QueryC)
    తిరిగి నిజం;
  తిరిగి SlashAgnostic && (PatC == '\\' || PatC == '/') &&
         (QueryC == '\\' || QueryC == '/');
}

// Factor the pattern into segments split by '*'. The segment is matched
// sequentianlly by finding the first occurrence past the end of the previous
// match.
తార్కిక Glob_నమూనా::Sub_Glob_నమూనా::సరిపోలిక(StringRef Str,
                                        తార్కిక SlashAgnostic) స్థిరరాశి {
  స్థిరరాశి అక్షరం *P = Pat.దత్తాంశం(), *SegmentBegin = శూన్యసూచిక, *S = Str.దత్తాంశం(),
             *SavedS = S;
  స్థిరరాశి అక్షరం *స్థిరరాశి PEnd = P + Pat.కొలత_2(), *స్థిరరాశి చివర_2 = S + Str.కొలత_2();
  size_t B = 0, SavedB = 0;
  అయితే (S != చివర_2) {
    ఒకవేళ (P == PEnd)
      ;
    లేకపోతే ఒకవేళ (*P == '*') {
      // The non-* substring on the left of '*' matches the tail of S. Save the
      // positions to be used by backtracking if we see a mismatch later.
      SegmentBegin = ++P;
      SavedS = S;
      SavedB = B;
      కొనసాగు;
    } లేకపోతే ఒకవేళ (*P == '[') {
      ఒకవేళ (Brackets[B].బైట్లు_2[uint8_t(*S)]) {
        P = Pat.దత్తాంశం() + Brackets[B++].తదుపరి_స్థానభ్రంశం;
        ++S;
        కొనసాగు;
      }
    } లేకపోతే ఒకవేళ (*P == '\\') {
      ఒకవేళ (సరిపోలిక_Char(*++P, *S, SlashAgnostic)) {
        ++P;
        ++S;
        కొనసాగు;
      }
    } లేకపోతే ఒకవేళ (సరిపోలిక_Char(*P, *S, SlashAgnostic) || *P == '?') {
      ++P;
      ++S;
      కొనసాగు;
    }
    ఒకవేళ (!SegmentBegin)
      తిరిగి అబద్ధం;
    // We have seen a '*'. Backtrack to the saved positions. Shift the S
    // position to probe the next starting position in the segment.
    P = SegmentBegin;
    S = ++SavedS;
    B = SavedB;
  }
  // All bytes in Str have been matched. Return true if the rest part of Pat is
  // empty or contains only '*'.
  తిరిగి getPat().కనుగొను_మొదటి_not_యొక్క('*', P - Pat.దత్తాంశం()) == std::string::npos;
}
