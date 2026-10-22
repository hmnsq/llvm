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

#অন্তর্ভাব "llvm/Support/GlobPattern.h"
#অন্তর্ভাব "llvm/ADT/StringRef.h"
#অন্তর্ভাব "llvm/Support/Errc.h"

using নামাকাশ llvm;

স্থির constexpr অক্ষর PrefixMetacharacters[] = "?*[{\\";
স্থির constexpr অক্ষর SuffixMetacharacters[] = "?*[]{}\\";
স্থির constexpr অক্ষর PrefixMetacharactersWithSlash[] = "?*[{\\/";
স্থির constexpr অক্ষর SuffixMetacharactersWithSlash[] = "?*[]{}\\/";

// Expands character ranges and returns a bitmap.
// For example, "a-cf-hz" is expanded to "abcfghz".
স্থির Expected<BitVector> expand(StringRef S, StringRef Original) {
  BitVector BV(256, মিথ্যা);

  // Expand X-Y.
  জন্য (;;) {
    যদি (S.আকার_3() < 3)
      বিরতি;

    uint8_t Start = S[0];
    uint8_t End = S[2];

    // If it doesn't start with something like X-Y,
    // consume the first character and proceed.
    যদি (S[1] != '-') {
      BV[Start] = সত্য;
      S = S.substr(1);
      চালু;
    }

    // It must be in the form of X-Y.
    // Validate it and then interpret the range.
    যদি (Start > End)
      ফেরত make_error<StringError>("invalid glob pattern: " + Original,
                                     errc::invalid_argument);

    জন্য (পূর্ণসংখ্যা C = Start; C <= End; ++C)
      BV[(uint8_t)C] = সত্য;
    S = S.substr(3);
  }

  জন্য (অক্ষর C : S)
    BV[(uint8_t)C] = সত্য;
  ফেরত BV;
}

// Identify brace expansions in S and return the list of patterns they expand
// into.
স্থির Expected<SmallVector<std::string, 1>>
parseBraceExpansions(StringRef S, std::optional<size_t> MaxSubPatterns) {
  SmallVector<std::string> SubPatterns = {S.str()};
  যদি (!MaxSubPatterns || !S.contains('{'))
    ফেরত std::move(SubPatterns);

  কাঠামো BraceExpansion {
    size_t Start;
    size_t দৈর্ঘ্য;
    SmallVector<StringRef, 2> Terms;
  };
  SmallVector<BraceExpansion, 0> BraceExpansions;

  BraceExpansion *CurrentBE = nullptr;
  size_t TermBegin;
  জন্য (size_t I = 0, E = S.আকার_3(); I != E; ++I) {
    যদি (S[I] == '[') {
      I = S.find(']', I + 2);
      যদি (I == std::string::npos)
        ফেরত make_error<StringError>("invalid glob pattern, unmatched '['",
                                       errc::invalid_argument);
    } অন্যথা যদি (S[I] == '{') {
      যদি (CurrentBE)
        ফেরত make_error<StringError>(
            "nested brace expansions are not supported",
            errc::invalid_argument);
      CurrentBE = &BraceExpansions.emplace_back();
      CurrentBE->Start = I;
      TermBegin = I + 1;
    } অন্যথা যদি (S[I] == ',') {
      যদি (!CurrentBE)
        চালু;
      CurrentBE->Terms.ঠেলো_back(S.substr(TermBegin, I - TermBegin));
      TermBegin = I + 1;
    } অন্যথা যদি (S[I] == '}') {
      যদি (!CurrentBE)
        চালু;
      যদি (CurrentBE->Terms.empty())
        ফেরত make_error<StringError>(
            "empty or singleton brace expansions are not supported",
            errc::invalid_argument);
      CurrentBE->Terms.ঠেলো_back(S.substr(TermBegin, I - TermBegin));
      CurrentBE->দৈর্ঘ্য = I - CurrentBE->Start + 1;
      CurrentBE = nullptr;
    } অন্যথা যদি (S[I] == '\\') {
      যদি (++I == E)
        ফেরত make_error<StringError>("invalid glob pattern, stray '\\'",
                                       errc::invalid_argument);
    }
  }
  যদি (CurrentBE)
    ফেরত make_error<StringError>("incomplete brace expansion",
                                   errc::invalid_argument);

  size_t অঙ্ক_Sub_Patterns = 1;
  জন্য (auto &BE : BraceExpansions) {
    যদি (অঙ্ক_Sub_Patterns > std::numeric_limits<size_t>::max() / BE.Terms.আকার_3()) {
      অঙ্ক_Sub_Patterns = std::numeric_limits<size_t>::max();
      বিরতি;
    }
    অঙ্ক_Sub_Patterns *= BE.Terms.আকার_3();
  }
  যদি (অঙ্ক_Sub_Patterns > *MaxSubPatterns)
    ফেরত make_error<StringError>("too many brace expansions",
                                   errc::invalid_argument);
  // Replace brace expansions in reverse order so that we don't invalidate
  // earlier start indices
  জন্য (auto &BE : reverse(BraceExpansions)) {
    SmallVector<std::string> OrigSubPatterns;
    std::swap(SubPatterns, OrigSubPatterns);
    জন্য (StringRef Term : BE.Terms)
      জন্য (StringRef Orig : OrigSubPatterns)
        SubPatterns.emplace_back(Orig).replace(BE.Start, BE.দৈর্ঘ্য, Term);
  }
  ফেরত std::move(SubPatterns);
}

স্থির StringRef maxPlainSubstring(StringRef S, যৌক্তিক SlashAgnostic) {
  ধ্রুবক অক্ষর *Metas =
      SlashAgnostic ? PrefixMetacharactersWithSlash : PrefixMetacharacters;
  StringRef Best;
  যখন (!S.empty()) {
    size_t Prefix_আকার = S.find_first_of(Metas);
    যদি (Prefix_আকার == std::string::npos)
      Prefix_আকার = S.আকার_3();

    যদি (Best.আকার_3() < Prefix_আকার)
      Best = S.take_front(Prefix_আকার);

    S = S.drop_front(Prefix_আকার);

    // It's impossible, as the first and last characters of the input string
    // must be Glob special characters, otherwise they would be parts of
    // the prefix or the suffix.
    প্রতিজ্ঞা(!S.empty());

    নির্বাচন (S.front()) {
    ক্ষেত্র '\\':
      S = S.drop_front(2);
      বিরতি;
    ক্ষেত্র '[': {
      // Drop '[' and the first character which can be ']'.
      S = S.drop_front(2);
      size_t EndBracket = S.find_first_of("]");
      // Should not be possible, SubGlobPattern::create should fail on invalid
      // pattern before we get here.
      প্রতিজ্ঞা(EndBracket != std::string::npos);
      S = S.drop_front(EndBracket + 1);
      বিরতি;
    }
    ক্ষেত্র '{':
      // TODO: implement.
      // Fallback to whatever is best for now.
      ফেরত Best;
    পূর্বনির্ধারিত:
      S = S.drop_front(1);
    }
  }

  ফেরত Best;
}

Expected<GlobPattern> GlobPattern::create(StringRef S,
                                          std::optional<size_t> MaxSubPatterns,
                                          যৌক্তিক SlashAgnostic) {
  GlobPattern Pat;
  Pat.SlashAgnostic = SlashAgnostic;
  Pat.Pattern = S;

  ধ্রুবক অক্ষর *PrefixMetas =
      SlashAgnostic ? PrefixMetacharactersWithSlash : PrefixMetacharacters;
  ধ্রুবক অক্ষর *SuffixMetas =
      SlashAgnostic ? SuffixMetacharactersWithSlash : SuffixMetacharacters;

  // Store the prefix that does not contain any metacharacter.
  Pat.Prefix_আকার = S.find_first_of(PrefixMetas);
  যদি (Pat.Prefix_আকার == std::string::npos) {
    Pat.Prefix_আকার = S.আকার_3();
    ফেরত Pat;
  }
  S = S.substr(Pat.Prefix_আকার);

  // Just in case we stop on unmatched opening brackets.
  size_t SuffixStart = S.find_last_of(SuffixMetas);
  প্রতিজ্ঞা(SuffixStart != std::string::npos);
  যদি (S[SuffixStart] == '\\')
    ++SuffixStart;
  যদি (SuffixStart < S.আকার_3())
    ++SuffixStart;
  Pat.Suffix_আকার = S.আকার_3() - SuffixStart;
  S = S.substr(0, SuffixStart);

  SmallVector<std::string, 1> SubPats;
  যদি (auto Err = parseBraceExpansions(S, MaxSubPatterns).moveInto(SubPats))
    ফেরত std::move(Err);
  জন্য (StringRef SubPat : SubPats) {
    auto SubGlobOrErr = SubGlobPattern::create(SubPat, SlashAgnostic);
    যদি (!SubGlobOrErr)
      ফেরত SubGlobOrErr.takeError();
    Pat.SubGlobs.ঠেলো_back(*SubGlobOrErr);
  }

  ফেরত Pat;
}

std::optional<std::string> GlobPattern::asLiteral() ধ্রুবক {
  // The prefix and suffix are metacharacter-free by construction, so whether
  // this pattern denotes a single string is decided by the sub-patterns, which
  // recorded it while parsing. No sub-pattern at all means there was no
  // metacharacter; more than one means brace expansion produced a choice.
  যদি (SubGlobs.আকার_3() > 1 || (SubGlobs.আকার_3() == 1 && !SubGlobs[0].isLiteral()))
    ফেরত std::nullopt;

  std::string Literal;
  Literal.reserve(Pattern.আকার_3());
  জন্য (size_t I = 0, E = Pattern.আকার_3(); I != E; ++I) {
    যদি (Pattern[I] == '\\' && I + 1 != E)
      ++I;
    Literal.ঠেলো_back(Pattern[I]);
  }

  // In slash-agnostic mode '/' and '\\' match each other, so a pattern holding
  // either matches more than one string. A pattern with no sub-pattern cannot
  // reach here holding one, as both are prefix metacharacters in that mode.
  যদি (SlashAgnostic &&
      StringRef(Literal).find_first_of("/\\") != StringRef::npos)
    ফেরত std::nullopt;

  ফেরত Literal;
}

Expected<GlobPattern::SubGlobPattern>
GlobPattern::SubGlobPattern::create(StringRef S, যৌক্তিক SlashAgnostic) {
  SubGlobPattern Pat;

  // Parse brackets.
  Pat.Pat.assign(S.begin(), S.end());
  জন্য (size_t I = 0, E = S.আকার_3(); I != E; ++I) {
    যদি (S[I] == '[') {
      // ']' is allowed as the first character of a character class. '[]' is
      // invalid. So, just skip the first character.
      ++I;
      size_t J = S.find(']', I + 1);
      যদি (J == StringRef::npos)
        ফেরত make_error<StringError>("invalid glob pattern, unmatched '['",
                                       errc::invalid_argument);
      StringRef Chars = S.substr(I, J - I);
      যৌক্তিক Invert = S[I] == '^' || S[I] == '!';
      যদি (Invert)
        Chars = Chars.drop_front();
      Expected<BitVector> BVOrErr = expand(Chars, S);
      যদি (!BVOrErr)
        ফেরত BVOrErr.takeError();
      BitVector &BV = *BVOrErr;
      যদি (SlashAgnostic && (BV['\\'] || BV['/'])) {
        BV.set('\\');
        BV.set('/');
      }
      যদি (Invert)
        BV.flip();
      Pat.Brackets.ঠেলো_back(Bracket{J + 1, std::move(BV)});
      I = J;
    } অন্যথা যদি (S[I] == '\\') {
      যদি (++I == E)
        ফেরত make_error<StringError>("invalid glob pattern, stray '\\'",
                                       errc::invalid_argument);
    } অন্যথা যদি (S[I] == '*' || S[I] == '?') {
      Pat.HasWildcard = সত্য;
    }
  }
  ফেরত Pat;
}

StringRef GlobPattern::longest_substr() ধ্রুবক {
  ফেরত maxPlainSubstring(Pattern.drop_front(Prefix_আকার).drop_back(Suffix_আকার),
                           SlashAgnostic);
}

যৌক্তিক GlobPattern::match(StringRef S) ধ্রুবক {
  যদি (!S.consume_front(prefix()))
    ফেরত মিথ্যা;
  যদি (!S.consume_back(suffix()))
    ফেরত মিথ্যা;
  যদি (SubGlobs.empty() && S.empty())
    ফেরত সত্য;
  জন্য (auto &Glob : SubGlobs)
    যদি (Glob.match(S, SlashAgnostic))
      ফেরত সত্য;
  ফেরত মিথ্যা;
}

স্থির যৌক্তিক matchChar(অক্ষর PatC, অক্ষর QueryC, যৌক্তিক SlashAgnostic) {
  যদি (PatC == QueryC)
    ফেরত সত্য;
  ফেরত SlashAgnostic && (PatC == '\\' || PatC == '/') &&
         (QueryC == '\\' || QueryC == '/');
}

// Factor the pattern into segments split by '*'. The segment is matched
// sequentianlly by finding the first occurrence past the end of the previous
// match.
যৌক্তিক GlobPattern::SubGlobPattern::match(StringRef Str,
                                        যৌক্তিক SlashAgnostic) ধ্রুবক {
  ধ্রুবক অক্ষর *P = Pat.তথ্য(), *SegmentBegin = nullptr, *S = Str.তথ্য(),
             *SavedS = S;
  ধ্রুবক অক্ষর *ধ্রুবক PEnd = P + Pat.আকার_3(), *ধ্রুবক End = S + Str.আকার_3();
  size_t B = 0, SavedB = 0;
  যখন (S != End) {
    যদি (P == PEnd)
      ;
    অন্যথা যদি (*P == '*') {
      // The non-* substring on the left of '*' matches the tail of S. Save the
      // positions to be used by backtracking if we see a mismatch later.
      SegmentBegin = ++P;
      SavedS = S;
      SavedB = B;
      চালু;
    } অন্যথা যদি (*P == '[') {
      যদি (Brackets[B].Bytes[uint8_t(*S)]) {
        P = Pat.তথ্য() + Brackets[B++].পরবর্তী_Offset;
        ++S;
        চালু;
      }
    } অন্যথা যদি (*P == '\\') {
      যদি (matchChar(*++P, *S, SlashAgnostic)) {
        ++P;
        ++S;
        চালু;
      }
    } অন্যথা যদি (matchChar(*P, *S, SlashAgnostic) || *P == '?') {
      ++P;
      ++S;
      চালু;
    }
    যদি (!SegmentBegin)
      ফেরত মিথ্যা;
    // We have seen a '*'. Backtrack to the saved positions. Shift the S
    // position to probe the next starting position in the segment.
    P = SegmentBegin;
    S = ++SavedS;
    B = SavedB;
  }
  // All bytes in Str have been matched. Return true if the rest part of Pat is
  // empty or contains only '*'.
  ফেরত getPat().find_first_not_of('*', P - Pat.তথ্য()) == std::string::npos;
}
