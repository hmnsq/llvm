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
  प्रति (;;) {
    यदि (S.size() < 3)
      भङ्ग;

    uint8_t Start = S[0];
    uint8_t End = S[2];

    // If it doesn't start with something like X-Y,
    // consume the first character and proceed.
    यदि (S[1] != '-') {
      BV[Start] = सत्य;
      S = S.substr(1);
      अनुवर्त;
    }

    // It must be in the form of X-Y.
    // Validate it and then interpret the range.
    यदि (Start > End)
      प्रत्यागम make_error<StringError>("invalid glob pattern: " + Original,
                                     errc::invalid_argument);

    प्रति (पूर्णाङ्क C = Start; C <= End; ++C)
      BV[(uint8_t)C] = सत्य;
    S = S.substr(3);
  }

  प्रति (वर्ण C : S)
    BV[(uint8_t)C] = सत्य;
  प्रत्यागम BV;
}

// Identify brace expansions in S and return the list of patterns they expand
// into.
स्थिर Expected<SmallVector<std::string, 1>>
parseBraceExpansions(StringRef S, std::optional<size_t> MaxSubPatterns) {
  SmallVector<std::string> SubPatterns = {S.str()};
  यदि (!MaxSubPatterns || !S.contains('{'))
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
    यदि (S[I] == '[') {
      I = S.find(']', I + 2);
      यदि (I == std::string::npos)
        प्रत्यागम make_error<StringError>("invalid glob pattern, unmatched '['",
                                       errc::invalid_argument);
    } अन्यथा यदि (S[I] == '{') {
      यदि (CurrentBE)
        प्रत्यागम make_error<StringError>(
            "nested brace expansions are not supported",
            errc::invalid_argument);
      CurrentBE = &BraceExpansions.emplace_back();
      CurrentBE->Start = I;
      TermBegin = I + 1;
    } अन्यथा यदि (S[I] == ',') {
      यदि (!CurrentBE)
        अनुवर्त;
      CurrentBE->Terms.धकेलो_back(S.substr(TermBegin, I - TermBegin));
      TermBegin = I + 1;
    } अन्यथा यदि (S[I] == '}') {
      यदि (!CurrentBE)
        अनुवर्त;
      यदि (CurrentBE->Terms.empty())
        प्रत्यागम make_error<StringError>(
            "empty or singleton brace expansions are not supported",
            errc::invalid_argument);
      CurrentBE->Terms.धकेलो_back(S.substr(TermBegin, I - TermBegin));
      CurrentBE->Length = I - CurrentBE->Start + 1;
      CurrentBE = nullptr;
    } अन्यथा यदि (S[I] == '\\') {
      यदि (++I == E)
        प्रत्यागम make_error<StringError>("invalid glob pattern, stray '\\'",
                                       errc::invalid_argument);
    }
  }
  यदि (CurrentBE)
    प्रत्यागम make_error<StringError>("incomplete brace expansion",
                                   errc::invalid_argument);

  size_t अंक_Sub_Patterns = 1;
  प्रति (auto &BE : BraceExpansions) {
    यदि (अंक_Sub_Patterns > std::numeric_limits<size_t>::max() / BE.Terms.size()) {
      अंक_Sub_Patterns = std::numeric_limits<size_t>::max();
      भङ्ग;
    }
    अंक_Sub_Patterns *= BE.Terms.size();
  }
  यदि (अंक_Sub_Patterns > *MaxSubPatterns)
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
  यावत् (!S.empty()) {
    size_t PrefixSize = S.find_first_of(Metas);
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
  यदि (Pat.PrefixSize == std::string::npos) {
    Pat.PrefixSize = S.size();
    प्रत्यागम Pat;
  }
  S = S.substr(Pat.PrefixSize);

  // Just in case we stop on unmatched opening brackets.
  size_t SuffixStart = S.find_last_of(SuffixMetas);
  प्रतिज्ञा(SuffixStart != std::string::npos);
  यदि (S[SuffixStart] == '\\')
    ++SuffixStart;
  यदि (SuffixStart < S.size())
    ++SuffixStart;
  Pat.SuffixSize = S.size() - SuffixStart;
  S = S.substr(0, SuffixStart);

  SmallVector<std::string, 1> SubPats;
  यदि (auto Err = parseBraceExpansions(S, MaxSubPatterns).moveInto(SubPats))
    प्रत्यागम std::move(Err);
  प्रति (StringRef SubPat : SubPats) {
    auto SubGlobOrErr = SubGlobPattern::create(SubPat, SlashAgnostic);
    यदि (!SubGlobOrErr)
      प्रत्यागम SubGlobOrErr.takeError();
    Pat.SubGlobs.धकेलो_back(*SubGlobOrErr);
  }

  प्रत्यागम Pat;
}

std::optional<std::string> GlobPattern::asLiteral() ध्रुव {
  // The prefix and suffix are metacharacter-free by construction, so whether
  // this pattern denotes a single string is decided by the sub-patterns, which
  // recorded it while parsing. No sub-pattern at all means there was no
  // metacharacter; more than one means brace expansion produced a choice.
  यदि (SubGlobs.size() > 1 || (SubGlobs.size() == 1 && !SubGlobs[0].isLiteral()))
    प्रत्यागम std::nullopt;

  std::string Literal;
  Literal.reserve(Pattern.size());
  प्रति (size_t I = 0, E = Pattern.size(); I != E; ++I) {
    यदि (Pattern[I] == '\\' && I + 1 != E)
      ++I;
    Literal.धकेलो_back(Pattern[I]);
  }

  // In slash-agnostic mode '/' and '\\' match each other, so a pattern holding
  // either matches more than one string. A pattern with no sub-pattern cannot
  // reach here holding one, as both are prefix metacharacters in that mode.
  यदि (SlashAgnostic &&
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
    यदि (S[I] == '[') {
      // ']' is allowed as the first character of a character class. '[]' is
      // invalid. So, just skip the first character.
      ++I;
      size_t J = S.find(']', I + 1);
      यदि (J == StringRef::npos)
        प्रत्यागम make_error<StringError>("invalid glob pattern, unmatched '['",
                                       errc::invalid_argument);
      StringRef Chars = S.substr(I, J - I);
      तार्किक Invert = S[I] == '^' || S[I] == '!';
      यदि (Invert)
        Chars = Chars.drop_front();
      Expected<BitVector> BVOrErr = expand(Chars, S);
      यदि (!BVOrErr)
        प्रत्यागम BVOrErr.takeError();
      BitVector &BV = *BVOrErr;
      यदि (SlashAgnostic && (BV['\\'] || BV['/'])) {
        BV.set('\\');
        BV.set('/');
      }
      यदि (Invert)
        BV.flip();
      Pat.Brackets.धकेलो_back(Bracket{J + 1, std::move(BV)});
      I = J;
    } अन्यथा यदि (S[I] == '\\') {
      यदि (++I == E)
        प्रत्यागम make_error<StringError>("invalid glob pattern, stray '\\'",
                                       errc::invalid_argument);
    } अन्यथा यदि (S[I] == '*' || S[I] == '?') {
      Pat.HasWildcard = सत्य;
    }
  }
  प्रत्यागम Pat;
}

StringRef GlobPattern::longest_substr() ध्रुव {
  प्रत्यागम maxPlainSubstring(Pattern.drop_front(PrefixSize).drop_back(SuffixSize),
                           SlashAgnostic);
}

तार्किक GlobPattern::match(StringRef S) ध्रुव {
  यदि (!S.consume_front(prefix()))
    प्रत्यागम असत्य;
  यदि (!S.consume_back(suffix()))
    प्रत्यागम असत्य;
  यदि (SubGlobs.empty() && S.empty())
    प्रत्यागम सत्य;
  प्रति (auto &Glob : SubGlobs)
    यदि (Glob.match(S, SlashAgnostic))
      प्रत्यागम सत्य;
  प्रत्यागम असत्य;
}

स्थिर तार्किक matchChar(वर्ण PatC, वर्ण QueryC, तार्किक SlashAgnostic) {
  यदि (PatC == QueryC)
    प्रत्यागम सत्य;
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
  यावत् (S != End) {
    यदि (P == PEnd)
      ;
    अन्यथा यदि (*P == '*') {
      // The non-* substring on the left of '*' matches the tail of S. Save the
      // positions to be used by backtracking if we see a mismatch later.
      SegmentBegin = ++P;
      SavedS = S;
      SavedB = B;
      अनुवर्त;
    } अन्यथा यदि (*P == '[') {
      यदि (Brackets[B].Bytes[uint8_t(*S)]) {
        P = Pat.data() + Brackets[B++].NextOffset;
        ++S;
        अनुवर्त;
      }
    } अन्यथा यदि (*P == '\\') {
      यदि (matchChar(*++P, *S, SlashAgnostic)) {
        ++P;
        ++S;
        अनुवर्त;
      }
    } अन्यथा यदि (matchChar(*P, *S, SlashAgnostic) || *P == '?') {
      ++P;
      ++S;
      अनुवर्त;
    }
    यदि (!SegmentBegin)
      प्रत्यागम असत्य;
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
