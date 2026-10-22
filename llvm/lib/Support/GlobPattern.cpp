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
  BitVector BV(256, असत्य);

  // Expand X-Y.
  लागि (;;) {
    यदि (S.size() < 3)
      रोक;

    uint8_t Start = S[0];
    uint8_t End = S[2];

    // If it doesn't start with something like X-Y,
    // consume the first character and proceed.
    यदि (S[1] != '-') {
      BV[Start] = सत्य;
      S = S.substr(1);
      जारी;
    }

    // It must be in the form of X-Y.
    // Validate it and then interpret the range.
    यदि (Start > End)
      फर्काऊ make_error<StringError>("invalid glob pattern: " + Original,
                                     errc::invalid_argument);

    लागि (पूर्णाङ्क C = Start; C <= End; ++C)
      BV[(uint8_t)C] = सत्य;
    S = S.substr(3);
  }

  लागि (वर्ण C : S)
    BV[(uint8_t)C] = सत्य;
  फर्काऊ BV;
}

// Identify brace expansions in S and return the list of patterns they expand
// into.
स्थिर Expected<SmallVector<std::string, 1>>
parseBraceExpansions(StringRef S, std::optional<माप_प्रकार> MaxSubPatterns) {
  SmallVector<std::string> SubPatterns = {S.str()};
  यदि (!MaxSubPatterns || !S.contains('{'))
    फर्काऊ std::move(SubPatterns);

  संरचना BraceExpansion {
    माप_प्रकार Start;
    माप_प्रकार Length;
    SmallVector<StringRef, 2> Terms;
  };
  SmallVector<BraceExpansion, 0> BraceExpansions;

  BraceExpansion *CurrentBE = nullptr;
  माप_प्रकार TermBegin;
  लागि (माप_प्रकार I = 0, E = S.size(); I != E; ++I) {
    यदि (S[I] == '[') {
      I = S.find(']', I + 2);
      यदि (I == std::string::npos)
        फर्काऊ make_error<StringError>("invalid glob pattern, unmatched '['",
                                       errc::invalid_argument);
    } अन्यथा यदि (S[I] == '{') {
      यदि (CurrentBE)
        फर्काऊ make_error<StringError>(
            "nested brace expansions are not supported",
            errc::invalid_argument);
      CurrentBE = &BraceExpansions.emplace_back();
      CurrentBE->Start = I;
      TermBegin = I + 1;
    } अन्यथा यदि (S[I] == ',') {
      यदि (!CurrentBE)
        जारी;
      CurrentBE->Terms.थप_back(S.substr(TermBegin, I - TermBegin));
      TermBegin = I + 1;
    } अन्यथा यदि (S[I] == '}') {
      यदि (!CurrentBE)
        जारी;
      यदि (CurrentBE->Terms.empty())
        फर्काऊ make_error<StringError>(
            "empty or singleton brace expansions are not supported",
            errc::invalid_argument);
      CurrentBE->Terms.थप_back(S.substr(TermBegin, I - TermBegin));
      CurrentBE->Length = I - CurrentBE->Start + 1;
      CurrentBE = nullptr;
    } अन्यथा यदि (S[I] == '\\') {
      यदि (++I == E)
        फर्काऊ make_error<StringError>("invalid glob pattern, stray '\\'",
                                       errc::invalid_argument);
    }
  }
  यदि (CurrentBE)
    फर्काऊ make_error<StringError>("incomplete brace expansion",
                                   errc::invalid_argument);

  माप_प्रकार अङ्क_Sub_Patterns = 1;
  लागि (स्वतः &BE : BraceExpansions) {
    यदि (अङ्क_Sub_Patterns > std::numeric_limits<माप_प्रकार>::max() / BE.Terms.size()) {
      अङ्क_Sub_Patterns = std::numeric_limits<माप_प्रकार>::max();
      रोक;
    }
    अङ्क_Sub_Patterns *= BE.Terms.size();
  }
  यदि (अङ्क_Sub_Patterns > *MaxSubPatterns)
    फर्काऊ make_error<StringError>("too many brace expansions",
                                   errc::invalid_argument);
  // Replace brace expansions in reverse order so that we don't invalidate
  // earlier start indices
  लागि (स्वतः &BE : reverse(BraceExpansions)) {
    SmallVector<std::string> OrigSubPatterns;
    std::swap(SubPatterns, OrigSubPatterns);
    लागि (StringRef Term : BE.Terms)
      लागि (StringRef Orig : OrigSubPatterns)
        SubPatterns.emplace_back(Orig).replace(BE.Start, BE.Length, Term);
  }
  फर्काऊ std::move(SubPatterns);
}

स्थिर StringRef maxPlainSubstring(StringRef S, तार्किक SlashAgnostic) {
  अचर वर्ण *Metas =
      SlashAgnostic ? PrefixMetacharactersWithSlash : PrefixMetacharacters;
  StringRef Best;
  जबसम्म (!S.empty()) {
    माप_प्रकार PrefixSize = S.find_first_of(Metas);
    यदि (PrefixSize == std::string::npos)
      PrefixSize = S.size();

    यदि (Best.size() < PrefixSize)
      Best = S.take_front(PrefixSize);

    S = S.drop_front(PrefixSize);

    // It's impossible, as the first and last characters of the input string
    // must be Glob special characters, otherwise they would be parts of
    // the prefix or the suffix.
    प्रतिज्ञा(!S.empty());

    विकल्प (S.front()) {
    पक्ष '\\':
      S = S.drop_front(2);
      रोक;
    पक्ष '[': {
      // Drop '[' and the first character which can be ']'.
      S = S.drop_front(2);
      माप_प्रकार EndBracket = S.find_first_of("]");
      // Should not be possible, SubGlobPattern::create should fail on invalid
      // pattern before we get here.
      प्रतिज्ञा(EndBracket != std::string::npos);
      S = S.drop_front(EndBracket + 1);
      रोक;
    }
    पक्ष '{':
      // TODO: implement.
      // Fallback to whatever is best for now.
      फर्काऊ Best;
    स्वभाव:
      S = S.drop_front(1);
    }
  }

  फर्काऊ Best;
}

Expected<GlobPattern> GlobPattern::create(StringRef S,
                                          std::optional<माप_प्रकार> MaxSubPatterns,
                                          तार्किक SlashAgnostic) {
  GlobPattern Pat;
  Pat.SlashAgnostic = SlashAgnostic;
  Pat.Pattern = S;

  अचर वर्ण *PrefixMetas =
      SlashAgnostic ? PrefixMetacharactersWithSlash : PrefixMetacharacters;
  अचर वर्ण *SuffixMetas =
      SlashAgnostic ? SuffixMetacharactersWithSlash : SuffixMetacharacters;

  // Store the prefix that does not contain any metacharacter.
  Pat.PrefixSize = S.find_first_of(PrefixMetas);
  यदि (Pat.PrefixSize == std::string::npos) {
    Pat.PrefixSize = S.size();
    फर्काऊ Pat;
  }
  S = S.substr(Pat.PrefixSize);

  // Just in case we stop on unmatched opening brackets.
  माप_प्रकार SuffixStart = S.find_last_of(SuffixMetas);
  प्रतिज्ञा(SuffixStart != std::string::npos);
  यदि (S[SuffixStart] == '\\')
    ++SuffixStart;
  यदि (SuffixStart < S.size())
    ++SuffixStart;
  Pat.SuffixSize = S.size() - SuffixStart;
  S = S.substr(0, SuffixStart);

  SmallVector<std::string, 1> SubPats;
  यदि (स्वतः Err = parseBraceExpansions(S, MaxSubPatterns).moveInto(SubPats))
    फर्काऊ std::move(Err);
  लागि (StringRef SubPat : SubPats) {
    स्वतः SubGlobOrErr = SubGlobPattern::create(SubPat, SlashAgnostic);
    यदि (!SubGlobOrErr)
      फर्काऊ SubGlobOrErr.takeError();
    Pat.SubGlobs.थप_back(*SubGlobOrErr);
  }

  फर्काऊ Pat;
}

std::optional<std::string> GlobPattern::asLiteral() अचर {
  // The prefix and suffix are metacharacter-free by construction, so whether
  // this pattern denotes a single string is decided by the sub-patterns, which
  // recorded it while parsing. No sub-pattern at all means there was no
  // metacharacter; more than one means brace expansion produced a choice.
  यदि (SubGlobs.size() > 1 || (SubGlobs.size() == 1 && !SubGlobs[0].isLiteral()))
    फर्काऊ std::nullopt;

  std::string Literal;
  Literal.reserve(Pattern.size());
  लागि (माप_प्रकार I = 0, E = Pattern.size(); I != E; ++I) {
    यदि (Pattern[I] == '\\' && I + 1 != E)
      ++I;
    Literal.थप_back(Pattern[I]);
  }

  // In slash-agnostic mode '/' and '\\' match each other, so a pattern holding
  // either matches more than one string. A pattern with no sub-pattern cannot
  // reach here holding one, as both are prefix metacharacters in that mode.
  यदि (SlashAgnostic &&
      StringRef(Literal).find_first_of("/\\") != StringRef::npos)
    फर्काऊ std::nullopt;

  फर्काऊ Literal;
}

Expected<GlobPattern::SubGlobPattern>
GlobPattern::SubGlobPattern::create(StringRef S, तार्किक SlashAgnostic) {
  SubGlobPattern Pat;

  // Parse brackets.
  Pat.Pat.assign(S.begin(), S.end());
  लागि (माप_प्रकार I = 0, E = S.size(); I != E; ++I) {
    यदि (S[I] == '[') {
      // ']' is allowed as the first character of a character class. '[]' is
      // invalid. So, just skip the first character.
      ++I;
      माप_प्रकार J = S.find(']', I + 1);
      यदि (J == StringRef::npos)
        फर्काऊ make_error<StringError>("invalid glob pattern, unmatched '['",
                                       errc::invalid_argument);
      StringRef Chars = S.substr(I, J - I);
      तार्किक Invert = S[I] == '^' || S[I] == '!';
      यदि (Invert)
        Chars = Chars.drop_front();
      Expected<BitVector> BVOrErr = expand(Chars, S);
      यदि (!BVOrErr)
        फर्काऊ BVOrErr.takeError();
      BitVector &BV = *BVOrErr;
      यदि (SlashAgnostic && (BV['\\'] || BV['/'])) {
        BV.set('\\');
        BV.set('/');
      }
      यदि (Invert)
        BV.flip();
      Pat.Brackets.थप_back(Bracket{J + 1, std::move(BV)});
      I = J;
    } अन्यथा यदि (S[I] == '\\') {
      यदि (++I == E)
        फर्काऊ make_error<StringError>("invalid glob pattern, stray '\\'",
                                       errc::invalid_argument);
    } अन्यथा यदि (S[I] == '*' || S[I] == '?') {
      Pat.HasWildcard = सत्य;
    }
  }
  फर्काऊ Pat;
}

StringRef GlobPattern::longest_substr() अचर {
  फर्काऊ maxPlainSubstring(Pattern.drop_front(PrefixSize).drop_back(SuffixSize),
                           SlashAgnostic);
}

तार्किक GlobPattern::match(StringRef S) अचर {
  यदि (!S.consume_front(prefix()))
    फर्काऊ असत्य;
  यदि (!S.consume_back(suffix()))
    फर्काऊ असत्य;
  यदि (SubGlobs.empty() && S.empty())
    फर्काऊ सत्य;
  लागि (स्वतः &Glob : SubGlobs)
    यदि (Glob.match(S, SlashAgnostic))
      फर्काऊ सत्य;
  फर्काऊ असत्य;
}

स्थिर तार्किक matchChar(वर्ण PatC, वर्ण QueryC, तार्किक SlashAgnostic) {
  यदि (PatC == QueryC)
    फर्काऊ सत्य;
  फर्काऊ SlashAgnostic && (PatC == '\\' || PatC == '/') &&
         (QueryC == '\\' || QueryC == '/');
}

// Factor the pattern into segments split by '*'. The segment is matched
// sequentianlly by finding the first occurrence past the end of the previous
// match.
तार्किक GlobPattern::SubGlobPattern::match(StringRef Str,
                                        तार्किक SlashAgnostic) अचर {
  अचर वर्ण *P = Pat.data(), *SegmentBegin = nullptr, *S = Str.data(),
             *SavedS = S;
  अचर वर्ण *अचर PEnd = P + Pat.size(), *अचर End = S + Str.size();
  माप_प्रकार B = 0, SavedB = 0;
  जबसम्म (S != End) {
    यदि (P == PEnd)
      ;
    अन्यथा यदि (*P == '*') {
      // The non-* substring on the left of '*' matches the tail of S. Save the
      // positions to be used by backtracking if we see a mismatch later.
      SegmentBegin = ++P;
      SavedS = S;
      SavedB = B;
      जारी;
    } अन्यथा यदि (*P == '[') {
      यदि (Brackets[B].Bytes[uint8_t(*S)]) {
        P = Pat.data() + Brackets[B++].NextOffset;
        ++S;
        जारी;
      }
    } अन्यथा यदि (*P == '\\') {
      यदि (matchChar(*++P, *S, SlashAgnostic)) {
        ++P;
        ++S;
        जारी;
      }
    } अन्यथा यदि (matchChar(*P, *S, SlashAgnostic) || *P == '?') {
      ++P;
      ++S;
      जारी;
    }
    यदि (!SegmentBegin)
      फर्काऊ असत्य;
    // We have seen a '*'. Backtrack to the saved positions. Shift the S
    // position to probe the next starting position in the segment.
    P = SegmentBegin;
    S = ++SavedS;
    B = SavedB;
  }
  // All bytes in Str have been matched. Return true if the rest part of Pat is
  // empty or contains only '*'.
  फर्काऊ getPat().find_first_not_of('*', P - Pat.data()) == std::string::npos;
}
