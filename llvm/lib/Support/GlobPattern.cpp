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

#অন্তর্ভাৱ "llvm/Support/GlobPattern.h"
#অন্তর্ভাৱ "llvm/ADT/StringRef.h"
#অন্তর্ভাৱ "llvm/Support/Errc.h"

using নামাকাশ llvm;

স্থির constexpr ৱর্ণ PrefixMetacharacters[] = "?*[{\\";
স্থির constexpr ৱর্ণ SuffixMetacharacters[] = "?*[]{}\\";
স্থির constexpr ৱর্ণ PrefixMetacharactersWithSlash[] = "?*[{\\/";
স্থির constexpr ৱর্ণ SuffixMetacharactersWithSlash[] = "?*[]{}\\/";

// Expands character ranges and returns a bitmap.
// For example, "a-cf-hz" is expanded to "abcfghz".
স্থির Expected<BitVector> expand(StringRef S, StringRef Original) {
  BitVector BV(256, মিছা);

  // Expand X-Y.
  বাবে (;;) {
    যদি (S.size() < 3)
      ভঙ্গ;

    uint8_t Start = S[0];
    uint8_t End = S[2];

    // If it doesn't start with something like X-Y,
    // consume the first character and proceed.
    যদি (S[1] != '-') {
      BV[Start] = সঁচা;
      S = S.substr(1);
      অনুৱর্ত;
    }

    // It must be in the form of X-Y.
    // Validate it and then interpret the range.
    যদি (Start > End)
      ঘূৰাই make_error<StringError>("invalid glob pattern: " + Original,
                                     errc::invalid_argument);

    বাবে (পূর্ণাঙ্ক C = Start; C <= End; ++C)
      BV[(uint8_t)C] = সঁচা;
    S = S.substr(3);
  }

  বাবে (ৱর্ণ C : S)
    BV[(uint8_t)C] = সঁচা;
  ঘূৰাই BV;
}

// Identify brace expansions in S and return the list of patterns they expand
// into.
স্থির Expected<SmallVector<std::string, 1>>
parseBraceExpansions(StringRef S, std::optional<size_t> MaxSubPatterns) {
  SmallVector<std::string> SubPatterns = {S.str()};
  যদি (!MaxSubPatterns || !S.contains('{'))
    ঘূৰাই std::move(SubPatterns);

  সংরচনা BraceExpansion {
    size_t Start;
    size_t Length;
    SmallVector<StringRef, 2> Terms;
  };
  SmallVector<BraceExpansion, 0> BraceExpansions;

  BraceExpansion *CurrentBE = nullptr;
  size_t TermBegin;
  বাবে (size_t I = 0, E = S.size(); I != E; ++I) {
    যদি (S[I] == '[') {
      I = S.find(']', I + 2);
      যদি (I == std::string::npos)
        ঘূৰাই make_error<StringError>("invalid glob pattern, unmatched '['",
                                       errc::invalid_argument);
    } অন্যথা যদি (S[I] == '{') {
      যদি (CurrentBE)
        ঘূৰাই make_error<StringError>(
            "nested brace expansions are not supported",
            errc::invalid_argument);
      CurrentBE = &BraceExpansions.emplace_back();
      CurrentBE->Start = I;
      TermBegin = I + 1;
    } অন্যথা যদি (S[I] == ',') {
      যদি (!CurrentBE)
        অনুৱর্ত;
      CurrentBE->Terms.ঠেলো_back(S.substr(TermBegin, I - TermBegin));
      TermBegin = I + 1;
    } অন্যথা যদি (S[I] == '}') {
      যদি (!CurrentBE)
        অনুৱর্ত;
      যদি (CurrentBE->Terms.empty())
        ঘূৰাই make_error<StringError>(
            "empty or singleton brace expansions are not supported",
            errc::invalid_argument);
      CurrentBE->Terms.ঠেলো_back(S.substr(TermBegin, I - TermBegin));
      CurrentBE->Length = I - CurrentBE->Start + 1;
      CurrentBE = nullptr;
    } অন্যথা যদি (S[I] == '\\') {
      যদি (++I == E)
        ঘূৰাই make_error<StringError>("invalid glob pattern, stray '\\'",
                                       errc::invalid_argument);
    }
  }
  যদি (CurrentBE)
    ঘূৰাই make_error<StringError>("incomplete brace expansion",
                                   errc::invalid_argument);

  size_t অঙ্ক_Sub_Patterns = 1;
  বাবে (auto &BE : BraceExpansions) {
    যদি (অঙ্ক_Sub_Patterns > std::numeric_limits<size_t>::max() / BE.Terms.size()) {
      অঙ্ক_Sub_Patterns = std::numeric_limits<size_t>::max();
      ভঙ্গ;
    }
    অঙ্ক_Sub_Patterns *= BE.Terms.size();
  }
  যদি (অঙ্ক_Sub_Patterns > *MaxSubPatterns)
    ঘূৰাই make_error<StringError>("too many brace expansions",
                                   errc::invalid_argument);
  // Replace brace expansions in reverse order so that we don't invalidate
  // earlier start indices
  বাবে (auto &BE : reverse(BraceExpansions)) {
    SmallVector<std::string> OrigSubPatterns;
    std::swap(SubPatterns, OrigSubPatterns);
    বাবে (StringRef Term : BE.Terms)
      বাবে (StringRef Orig : OrigSubPatterns)
        SubPatterns.emplace_back(Orig).replace(BE.Start, BE.Length, Term);
  }
  ঘূৰাই std::move(SubPatterns);
}

স্থির StringRef maxPlainSubstring(StringRef S, তার্কিক SlashAgnostic) {
  ধ্রুৱ ৱর্ণ *Metas =
      SlashAgnostic ? PrefixMetacharactersWithSlash : PrefixMetacharacters;
  StringRef Best;
  যেতিয়ালৈ (!S.empty()) {
    size_t PrefixSize = S.find_first_of(Metas);
    যদি (PrefixSize == std::string::npos)
      PrefixSize = S.size();

    যদি (Best.size() < PrefixSize)
      Best = S.take_front(PrefixSize);

    S = S.drop_front(PrefixSize);

    // It's impossible, as the first and last characters of the input string
    // must be Glob special characters, otherwise they would be parts of
    // the prefix or the suffix.
    প্রতিজ্ঞা(!S.empty());

    ৱিকল্প (S.front()) {
    পক্ষ '\\':
      S = S.drop_front(2);
      ভঙ্গ;
    পক্ষ '[': {
      // Drop '[' and the first character which can be ']'.
      S = S.drop_front(2);
      size_t EndBracket = S.find_first_of("]");
      // Should not be possible, SubGlobPattern::create should fail on invalid
      // pattern before we get here.
      প্রতিজ্ঞা(EndBracket != std::string::npos);
      S = S.drop_front(EndBracket + 1);
      ভঙ্গ;
    }
    পক্ষ '{':
      // TODO: implement.
      // Fallback to whatever is best for now.
      ঘূৰাই Best;
    স্ৱভাৱ:
      S = S.drop_front(1);
    }
  }

  ঘূৰাই Best;
}

Expected<GlobPattern> GlobPattern::create(StringRef S,
                                          std::optional<size_t> MaxSubPatterns,
                                          তার্কিক SlashAgnostic) {
  GlobPattern Pat;
  Pat.SlashAgnostic = SlashAgnostic;
  Pat.Pattern = S;

  ধ্রুৱ ৱর্ণ *PrefixMetas =
      SlashAgnostic ? PrefixMetacharactersWithSlash : PrefixMetacharacters;
  ধ্রুৱ ৱর্ণ *SuffixMetas =
      SlashAgnostic ? SuffixMetacharactersWithSlash : SuffixMetacharacters;

  // Store the prefix that does not contain any metacharacter.
  Pat.PrefixSize = S.find_first_of(PrefixMetas);
  যদি (Pat.PrefixSize == std::string::npos) {
    Pat.PrefixSize = S.size();
    ঘূৰাই Pat;
  }
  S = S.substr(Pat.PrefixSize);

  // Just in case we stop on unmatched opening brackets.
  size_t SuffixStart = S.find_last_of(SuffixMetas);
  প্রতিজ্ঞা(SuffixStart != std::string::npos);
  যদি (S[SuffixStart] == '\\')
    ++SuffixStart;
  যদি (SuffixStart < S.size())
    ++SuffixStart;
  Pat.SuffixSize = S.size() - SuffixStart;
  S = S.substr(0, SuffixStart);

  SmallVector<std::string, 1> SubPats;
  যদি (auto Err = parseBraceExpansions(S, MaxSubPatterns).moveInto(SubPats))
    ঘূৰাই std::move(Err);
  বাবে (StringRef SubPat : SubPats) {
    auto SubGlobOrErr = SubGlobPattern::create(SubPat, SlashAgnostic);
    যদি (!SubGlobOrErr)
      ঘূৰাই SubGlobOrErr.takeError();
    Pat.SubGlobs.ঠেলো_back(*SubGlobOrErr);
  }

  ঘূৰাই Pat;
}

std::optional<std::string> GlobPattern::asLiteral() ধ্রুৱ {
  // The prefix and suffix are metacharacter-free by construction, so whether
  // this pattern denotes a single string is decided by the sub-patterns, which
  // recorded it while parsing. No sub-pattern at all means there was no
  // metacharacter; more than one means brace expansion produced a choice.
  যদি (SubGlobs.size() > 1 || (SubGlobs.size() == 1 && !SubGlobs[0].isLiteral()))
    ঘূৰাই std::nullopt;

  std::string Literal;
  Literal.reserve(Pattern.size());
  বাবে (size_t I = 0, E = Pattern.size(); I != E; ++I) {
    যদি (Pattern[I] == '\\' && I + 1 != E)
      ++I;
    Literal.ঠেলো_back(Pattern[I]);
  }

  // In slash-agnostic mode '/' and '\\' match each other, so a pattern holding
  // either matches more than one string. A pattern with no sub-pattern cannot
  // reach here holding one, as both are prefix metacharacters in that mode.
  যদি (SlashAgnostic &&
      StringRef(Literal).find_first_of("/\\") != StringRef::npos)
    ঘূৰাই std::nullopt;

  ঘূৰাই Literal;
}

Expected<GlobPattern::SubGlobPattern>
GlobPattern::SubGlobPattern::create(StringRef S, তার্কিক SlashAgnostic) {
  SubGlobPattern Pat;

  // Parse brackets.
  Pat.Pat.assign(S.begin(), S.end());
  বাবে (size_t I = 0, E = S.size(); I != E; ++I) {
    যদি (S[I] == '[') {
      // ']' is allowed as the first character of a character class. '[]' is
      // invalid. So, just skip the first character.
      ++I;
      size_t J = S.find(']', I + 1);
      যদি (J == StringRef::npos)
        ঘূৰাই make_error<StringError>("invalid glob pattern, unmatched '['",
                                       errc::invalid_argument);
      StringRef Chars = S.substr(I, J - I);
      তার্কিক Invert = S[I] == '^' || S[I] == '!';
      যদি (Invert)
        Chars = Chars.drop_front();
      Expected<BitVector> BVOrErr = expand(Chars, S);
      যদি (!BVOrErr)
        ঘূৰাই BVOrErr.takeError();
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
        ঘূৰাই make_error<StringError>("invalid glob pattern, stray '\\'",
                                       errc::invalid_argument);
    } অন্যথা যদি (S[I] == '*' || S[I] == '?') {
      Pat.HasWildcard = সঁচা;
    }
  }
  ঘূৰাই Pat;
}

StringRef GlobPattern::longest_substr() ধ্রুৱ {
  ঘূৰাই maxPlainSubstring(Pattern.drop_front(PrefixSize).drop_back(SuffixSize),
                           SlashAgnostic);
}

তার্কিক GlobPattern::match(StringRef S) ধ্রুৱ {
  যদি (!S.consume_front(prefix()))
    ঘূৰাই মিছা;
  যদি (!S.consume_back(suffix()))
    ঘূৰাই মিছা;
  যদি (SubGlobs.empty() && S.empty())
    ঘূৰাই সঁচা;
  বাবে (auto &Glob : SubGlobs)
    যদি (Glob.match(S, SlashAgnostic))
      ঘূৰাই সঁচা;
  ঘূৰাই মিছা;
}

স্থির তার্কিক matchChar(ৱর্ণ PatC, ৱর্ণ QueryC, তার্কিক SlashAgnostic) {
  যদি (PatC == QueryC)
    ঘূৰাই সঁচা;
  ঘূৰাই SlashAgnostic && (PatC == '\\' || PatC == '/') &&
         (QueryC == '\\' || QueryC == '/');
}

// Factor the pattern into segments split by '*'. The segment is matched
// sequentianlly by finding the first occurrence past the end of the previous
// match.
তার্কিক GlobPattern::SubGlobPattern::match(StringRef Str,
                                        তার্কিক SlashAgnostic) ধ্রুৱ {
  ধ্রুৱ ৱর্ণ *P = Pat.data(), *SegmentBegin = nullptr, *S = Str.data(),
             *SavedS = S;
  ধ্রুৱ ৱর্ণ *ধ্রুৱ PEnd = P + Pat.size(), *ধ্রুৱ End = S + Str.size();
  size_t B = 0, SavedB = 0;
  যেতিয়ালৈ (S != End) {
    যদি (P == PEnd)
      ;
    অন্যথা যদি (*P == '*') {
      // The non-* substring on the left of '*' matches the tail of S. Save the
      // positions to be used by backtracking if we see a mismatch later.
      SegmentBegin = ++P;
      SavedS = S;
      SavedB = B;
      অনুৱর্ত;
    } অন্যথা যদি (*P == '[') {
      যদি (Brackets[B].Bytes[uint8_t(*S)]) {
        P = Pat.data() + Brackets[B++].NextOffset;
        ++S;
        অনুৱর্ত;
      }
    } অন্যথা যদি (*P == '\\') {
      যদি (matchChar(*++P, *S, SlashAgnostic)) {
        ++P;
        ++S;
        অনুৱর্ত;
      }
    } অন্যথা যদি (matchChar(*P, *S, SlashAgnostic) || *P == '?') {
      ++P;
      ++S;
      অনুৱর্ত;
    }
    যদি (!SegmentBegin)
      ঘূৰাই মিছা;
    // We have seen a '*'. Backtrack to the saved positions. Shift the S
    // position to probe the next starting position in the segment.
    P = SegmentBegin;
    S = ++SavedS;
    B = SavedB;
  }
  // All bytes in Str have been matched. Return true if the rest part of Pat is
  // empty or contains only '*'.
  ঘূৰাই getPat().find_first_not_of('*', P - Pat.data()) == std::string::npos;
}
