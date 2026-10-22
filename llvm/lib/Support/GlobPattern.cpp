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

स्थिर constexpr अक्षर PrefixMetacharacters[] = "?*[{\\";
स्थिर constexpr अक्षर SuffixMetacharacters[] = "?*[]{}\\";
स्थिर constexpr अक्षर PrefixMetacharactersWithSlash[] = "?*[{\\/";
स्थिर constexpr अक्षर SuffixMetacharactersWithSlash[] = "?*[]{}\\/";

// Expands character ranges and returns a bitmap.
// For example, "a-cf-hz" is expanded to "abcfghz".
स्थिर Expected<BitVector> expand(StringRef S, StringRef Original) {
  BitVector BV(256, असत्य);

  // Expand X-Y.
  साठी (;;) {
    जर (S.size() < 3)
      थांबा;

    uint8_t Start = S[0];
    uint8_t End = S[2];

    // If it doesn't start with something like X-Y,
    // consume the first character and proceed.
    जर (S[1] != '-') {
      BV[Start] = सत्य;
      S = S.substr(1);
      सुरूठेवा;
    }

    // It must be in the form of X-Y.
    // Validate it and then interpret the range.
    जर (Start > End)
      परत make_error<StringError>("invalid glob pattern: " + Original,
                                     errc::invalid_argument);

    साठी (पूर्णांक C = Start; C <= End; ++C)
      BV[(uint8_t)C] = सत्य;
    S = S.substr(3);
  }

  साठी (अक्षर C : S)
    BV[(uint8_t)C] = सत्य;
  परत BV;
}

// Identify brace expansions in S and return the list of patterns they expand
// into.
स्थिर Expected<SmallVector<std::string, 1>>
parseBraceExpansions(StringRef S, std::optional<माप_प्रकार> MaxSubPatterns) {
  SmallVector<std::string> SubPatterns = {S.str()};
  जर (!MaxSubPatterns || !S.contains('{'))
    परत std::move(SubPatterns);

  संरचना BraceExpansion {
    माप_प्रकार Start;
    माप_प्रकार Length;
    SmallVector<StringRef, 2> Terms;
  };
  SmallVector<BraceExpansion, 0> BraceExpansions;

  BraceExpansion *CurrentBE = nullptr;
  माप_प्रकार TermBegin;
  साठी (माप_प्रकार I = 0, E = S.size(); I != E; ++I) {
    जर (S[I] == '[') {
      I = S.find(']', I + 2);
      जर (I == std::string::npos)
        परत make_error<StringError>("invalid glob pattern, unmatched '['",
                                       errc::invalid_argument);
    } अन्यथा जर (S[I] == '{') {
      जर (CurrentBE)
        परत make_error<StringError>(
            "nested brace expansions are not supported",
            errc::invalid_argument);
      CurrentBE = &BraceExpansions.emplace_back();
      CurrentBE->Start = I;
      TermBegin = I + 1;
    } अन्यथा जर (S[I] == ',') {
      जर (!CurrentBE)
        सुरूठेवा;
      CurrentBE->Terms.ढकला_back(S.substr(TermBegin, I - TermBegin));
      TermBegin = I + 1;
    } अन्यथा जर (S[I] == '}') {
      जर (!CurrentBE)
        सुरूठेवा;
      जर (CurrentBE->Terms.empty())
        परत make_error<StringError>(
            "empty or singleton brace expansions are not supported",
            errc::invalid_argument);
      CurrentBE->Terms.ढकला_back(S.substr(TermBegin, I - TermBegin));
      CurrentBE->Length = I - CurrentBE->Start + 1;
      CurrentBE = nullptr;
    } अन्यथा जर (S[I] == '\\') {
      जर (++I == E)
        परत make_error<StringError>("invalid glob pattern, stray '\\'",
                                       errc::invalid_argument);
    }
  }
  जर (CurrentBE)
    परत make_error<StringError>("incomplete brace expansion",
                                   errc::invalid_argument);

  माप_प्रकार अंक_Sub_Patterns = 1;
  साठी (स्वतः &BE : BraceExpansions) {
    जर (अंक_Sub_Patterns > std::numeric_limits<माप_प्रकार>::max() / BE.Terms.size()) {
      अंक_Sub_Patterns = std::numeric_limits<माप_प्रकार>::max();
      थांबा;
    }
    अंक_Sub_Patterns *= BE.Terms.size();
  }
  जर (अंक_Sub_Patterns > *MaxSubPatterns)
    परत make_error<StringError>("too many brace expansions",
                                   errc::invalid_argument);
  // Replace brace expansions in reverse order so that we don't invalidate
  // earlier start indices
  साठी (स्वतः &BE : reverse(BraceExpansions)) {
    SmallVector<std::string> OrigSubPatterns;
    std::swap(SubPatterns, OrigSubPatterns);
    साठी (StringRef Term : BE.Terms)
      साठी (StringRef Orig : OrigSubPatterns)
        SubPatterns.emplace_back(Orig).replace(BE.Start, BE.Length, Term);
  }
  परत std::move(SubPatterns);
}

स्थिर StringRef maxPlainSubstring(StringRef S, तार्किक SlashAgnostic) {
  स्थिरांक अक्षर *Metas =
      SlashAgnostic ? PrefixMetacharactersWithSlash : PrefixMetacharacters;
  StringRef Best;
  पर्यंत (!S.empty()) {
    माप_प्रकार PrefixSize = S.find_first_of(Metas);
    जर (PrefixSize == std::string::npos)
      PrefixSize = S.size();

    जर (Best.size() < PrefixSize)
      Best = S.take_front(PrefixSize);

    S = S.drop_front(PrefixSize);

    // It's impossible, as the first and last characters of the input string
    // must be Glob special characters, otherwise they would be parts of
    // the prefix or the suffix.
    प्रतिज्ञा(!S.empty());

    निवड (S.front()) {
    स्थिती '\\':
      S = S.drop_front(2);
      थांबा;
    स्थिती '[': {
      // Drop '[' and the first character which can be ']'.
      S = S.drop_front(2);
      माप_प्रकार EndBracket = S.find_first_of("]");
      // Should not be possible, SubGlobPattern::create should fail on invalid
      // pattern before we get here.
      प्रतिज्ञा(EndBracket != std::string::npos);
      S = S.drop_front(EndBracket + 1);
      थांबा;
    }
    स्थिती '{':
      // TODO: implement.
      // Fallback to whatever is best for now.
      परत Best;
    पूर्वनिर्धारित:
      S = S.drop_front(1);
    }
  }

  परत Best;
}

Expected<GlobPattern> GlobPattern::create(StringRef S,
                                          std::optional<माप_प्रकार> MaxSubPatterns,
                                          तार्किक SlashAgnostic) {
  GlobPattern Pat;
  Pat.SlashAgnostic = SlashAgnostic;
  Pat.Pattern = S;

  स्थिरांक अक्षर *PrefixMetas =
      SlashAgnostic ? PrefixMetacharactersWithSlash : PrefixMetacharacters;
  स्थिरांक अक्षर *SuffixMetas =
      SlashAgnostic ? SuffixMetacharactersWithSlash : SuffixMetacharacters;

  // Store the prefix that does not contain any metacharacter.
  Pat.PrefixSize = S.find_first_of(PrefixMetas);
  जर (Pat.PrefixSize == std::string::npos) {
    Pat.PrefixSize = S.size();
    परत Pat;
  }
  S = S.substr(Pat.PrefixSize);

  // Just in case we stop on unmatched opening brackets.
  माप_प्रकार SuffixStart = S.find_last_of(SuffixMetas);
  प्रतिज्ञा(SuffixStart != std::string::npos);
  जर (S[SuffixStart] == '\\')
    ++SuffixStart;
  जर (SuffixStart < S.size())
    ++SuffixStart;
  Pat.SuffixSize = S.size() - SuffixStart;
  S = S.substr(0, SuffixStart);

  SmallVector<std::string, 1> SubPats;
  जर (स्वतः Err = parseBraceExpansions(S, MaxSubPatterns).moveInto(SubPats))
    परत std::move(Err);
  साठी (StringRef SubPat : SubPats) {
    स्वतः SubGlobOrErr = SubGlobPattern::create(SubPat, SlashAgnostic);
    जर (!SubGlobOrErr)
      परत SubGlobOrErr.takeError();
    Pat.SubGlobs.ढकला_back(*SubGlobOrErr);
  }

  परत Pat;
}

std::optional<std::string> GlobPattern::asLiteral() स्थिरांक {
  // The prefix and suffix are metacharacter-free by construction, so whether
  // this pattern denotes a single string is decided by the sub-patterns, which
  // recorded it while parsing. No sub-pattern at all means there was no
  // metacharacter; more than one means brace expansion produced a choice.
  जर (SubGlobs.size() > 1 || (SubGlobs.size() == 1 && !SubGlobs[0].isLiteral()))
    परत std::nullopt;

  std::string Literal;
  Literal.reserve(Pattern.size());
  साठी (माप_प्रकार I = 0, E = Pattern.size(); I != E; ++I) {
    जर (Pattern[I] == '\\' && I + 1 != E)
      ++I;
    Literal.ढकला_back(Pattern[I]);
  }

  // In slash-agnostic mode '/' and '\\' match each other, so a pattern holding
  // either matches more than one string. A pattern with no sub-pattern cannot
  // reach here holding one, as both are prefix metacharacters in that mode.
  जर (SlashAgnostic &&
      StringRef(Literal).find_first_of("/\\") != StringRef::npos)
    परत std::nullopt;

  परत Literal;
}

Expected<GlobPattern::SubGlobPattern>
GlobPattern::SubGlobPattern::create(StringRef S, तार्किक SlashAgnostic) {
  SubGlobPattern Pat;

  // Parse brackets.
  Pat.Pat.assign(S.begin(), S.end());
  साठी (माप_प्रकार I = 0, E = S.size(); I != E; ++I) {
    जर (S[I] == '[') {
      // ']' is allowed as the first character of a character class. '[]' is
      // invalid. So, just skip the first character.
      ++I;
      माप_प्रकार J = S.find(']', I + 1);
      जर (J == StringRef::npos)
        परत make_error<StringError>("invalid glob pattern, unmatched '['",
                                       errc::invalid_argument);
      StringRef Chars = S.substr(I, J - I);
      तार्किक Invert = S[I] == '^' || S[I] == '!';
      जर (Invert)
        Chars = Chars.drop_front();
      Expected<BitVector> BVOrErr = expand(Chars, S);
      जर (!BVOrErr)
        परत BVOrErr.takeError();
      BitVector &BV = *BVOrErr;
      जर (SlashAgnostic && (BV['\\'] || BV['/'])) {
        BV.set('\\');
        BV.set('/');
      }
      जर (Invert)
        BV.flip();
      Pat.Brackets.ढकला_back(Bracket{J + 1, std::move(BV)});
      I = J;
    } अन्यथा जर (S[I] == '\\') {
      जर (++I == E)
        परत make_error<StringError>("invalid glob pattern, stray '\\'",
                                       errc::invalid_argument);
    } अन्यथा जर (S[I] == '*' || S[I] == '?') {
      Pat.HasWildcard = सत्य;
    }
  }
  परत Pat;
}

StringRef GlobPattern::longest_substr() स्थिरांक {
  परत maxPlainSubstring(Pattern.drop_front(PrefixSize).drop_back(SuffixSize),
                           SlashAgnostic);
}

तार्किक GlobPattern::match(StringRef S) स्थिरांक {
  जर (!S.consume_front(prefix()))
    परत असत्य;
  जर (!S.consume_back(suffix()))
    परत असत्य;
  जर (SubGlobs.empty() && S.empty())
    परत सत्य;
  साठी (स्वतः &Glob : SubGlobs)
    जर (Glob.match(S, SlashAgnostic))
      परत सत्य;
  परत असत्य;
}

स्थिर तार्किक matchChar(अक्षर PatC, अक्षर QueryC, तार्किक SlashAgnostic) {
  जर (PatC == QueryC)
    परत सत्य;
  परत SlashAgnostic && (PatC == '\\' || PatC == '/') &&
         (QueryC == '\\' || QueryC == '/');
}

// Factor the pattern into segments split by '*'. The segment is matched
// sequentianlly by finding the first occurrence past the end of the previous
// match.
तार्किक GlobPattern::SubGlobPattern::match(StringRef Str,
                                        तार्किक SlashAgnostic) स्थिरांक {
  स्थिरांक अक्षर *P = Pat.data(), *SegmentBegin = nullptr, *S = Str.data(),
             *SavedS = S;
  स्थिरांक अक्षर *स्थिरांक PEnd = P + Pat.size(), *स्थिरांक End = S + Str.size();
  माप_प्रकार B = 0, SavedB = 0;
  पर्यंत (S != End) {
    जर (P == PEnd)
      ;
    अन्यथा जर (*P == '*') {
      // The non-* substring on the left of '*' matches the tail of S. Save the
      // positions to be used by backtracking if we see a mismatch later.
      SegmentBegin = ++P;
      SavedS = S;
      SavedB = B;
      सुरूठेवा;
    } अन्यथा जर (*P == '[') {
      जर (Brackets[B].Bytes[uint8_t(*S)]) {
        P = Pat.data() + Brackets[B++].NextOffset;
        ++S;
        सुरूठेवा;
      }
    } अन्यथा जर (*P == '\\') {
      जर (matchChar(*++P, *S, SlashAgnostic)) {
        ++P;
        ++S;
        सुरूठेवा;
      }
    } अन्यथा जर (matchChar(*P, *S, SlashAgnostic) || *P == '?') {
      ++P;
      ++S;
      सुरूठेवा;
    }
    जर (!SegmentBegin)
      परत असत्य;
    // We have seen a '*'. Backtrack to the saved positions. Shift the S
    // position to probe the next starting position in the segment.
    P = SegmentBegin;
    S = ++SavedS;
    B = SavedB;
  }
  // All bytes in Str have been matched. Return true if the rest part of Pat is
  // empty or contains only '*'.
  परत getPat().find_first_not_of('*', P - Pat.data()) == std::string::npos;
}
