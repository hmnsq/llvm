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

#அந்தர்பாவ "llvm/Support/GlobPattern.h"
#அந்தர்பாவ "llvm/ADT/StringRef.h"
#அந்தர்பாவ "llvm/Support/Errc.h"

using நாமாகாஶ llvm;

நிலையான constexpr எழுத்து PrefixMetacharacters[] = "?*[{\\";
நிலையான constexpr எழுத்து SuffixMetacharacters[] = "?*[]{}\\";
நிலையான constexpr எழுத்து PrefixMetacharactersWithSlash[] = "?*[{\\/";
நிலையான constexpr எழுத்து SuffixMetacharactersWithSlash[] = "?*[]{}\\/";

// Expands character ranges and returns a bitmap.
// For example, "a-cf-hz" is expanded to "abcfghz".
நிலையான Expected<BitVector> expand(StringRef S, StringRef Original) {
  BitVector BV(256, பொய்);

  // Expand X-Y.
  க்கு (;;) {
    எனில் (S.size() < 3)
      நிறுத்து;

    uint8_t Start = S[0];
    uint8_t End = S[2];

    // If it doesn't start with something like X-Y,
    // consume the first character and proceed.
    எனில் (S[1] != '-') {
      BV[Start] = உண்மை;
      S = S.substr(1);
      தொடர்;
    }

    // It must be in the form of X-Y.
    // Validate it and then interpret the range.
    எனில் (Start > End)
      திரும்பு make_error<StringError>("invalid glob pattern: " + Original,
                                     errc::invalid_argument);

    க்கு (முழு C = Start; C <= End; ++C)
      BV[(uint8_t)C] = உண்மை;
    S = S.substr(3);
  }

  க்கு (எழுத்து C : S)
    BV[(uint8_t)C] = உண்மை;
  திரும்பு BV;
}

// Identify brace expansions in S and return the list of patterns they expand
// into.
நிலையான Expected<SmallVector<std::string, 1>>
parseBraceExpansions(StringRef S, std::optional<size_t> MaxSubPatterns) {
  SmallVector<std::string> SubPatterns = {S.str()};
  எனில் (!MaxSubPatterns || !S.contains('{'))
    திரும்பு std::move(SubPatterns);

  கட்டமைப்பு BraceExpansion {
    size_t Start;
    size_t Length;
    SmallVector<StringRef, 2> Terms;
  };
  SmallVector<BraceExpansion, 0> BraceExpansions;

  BraceExpansion *CurrentBE = nullptr;
  size_t TermBegin;
  க்கு (size_t I = 0, E = S.size(); I != E; ++I) {
    எனில் (S[I] == '[') {
      I = S.find(']', I + 2);
      எனில் (I == std::string::npos)
        திரும்பு make_error<StringError>("invalid glob pattern, unmatched '['",
                                       errc::invalid_argument);
    } இல்லையெனில் எனில் (S[I] == '{') {
      எனில் (CurrentBE)
        திரும்பு make_error<StringError>(
            "nested brace expansions are not supported",
            errc::invalid_argument);
      CurrentBE = &BraceExpansions.emplace_back();
      CurrentBE->Start = I;
      TermBegin = I + 1;
    } இல்லையெனில் எனில் (S[I] == ',') {
      எனில் (!CurrentBE)
        தொடர்;
      CurrentBE->Terms.தள்ளு_back(S.substr(TermBegin, I - TermBegin));
      TermBegin = I + 1;
    } இல்லையெனில் எனில் (S[I] == '}') {
      எனில் (!CurrentBE)
        தொடர்;
      எனில் (CurrentBE->Terms.empty())
        திரும்பு make_error<StringError>(
            "empty or singleton brace expansions are not supported",
            errc::invalid_argument);
      CurrentBE->Terms.தள்ளு_back(S.substr(TermBegin, I - TermBegin));
      CurrentBE->Length = I - CurrentBE->Start + 1;
      CurrentBE = nullptr;
    } இல்லையெனில் எனில் (S[I] == '\\') {
      எனில் (++I == E)
        திரும்பு make_error<StringError>("invalid glob pattern, stray '\\'",
                                       errc::invalid_argument);
    }
  }
  எனில் (CurrentBE)
    திரும்பு make_error<StringError>("incomplete brace expansion",
                                   errc::invalid_argument);

  size_t இலக்கம்_Sub_Patterns = 1;
  க்கு (auto &BE : BraceExpansions) {
    எனில் (இலக்கம்_Sub_Patterns > std::numeric_limits<size_t>::max() / BE.Terms.size()) {
      இலக்கம்_Sub_Patterns = std::numeric_limits<size_t>::max();
      நிறுத்து;
    }
    இலக்கம்_Sub_Patterns *= BE.Terms.size();
  }
  எனில் (இலக்கம்_Sub_Patterns > *MaxSubPatterns)
    திரும்பு make_error<StringError>("too many brace expansions",
                                   errc::invalid_argument);
  // Replace brace expansions in reverse order so that we don't invalidate
  // earlier start indices
  க்கு (auto &BE : reverse(BraceExpansions)) {
    SmallVector<std::string> OrigSubPatterns;
    std::swap(SubPatterns, OrigSubPatterns);
    க்கு (StringRef Term : BE.Terms)
      க்கு (StringRef Orig : OrigSubPatterns)
        SubPatterns.emplace_back(Orig).replace(BE.Start, BE.Length, Term);
  }
  திரும்பு std::move(SubPatterns);
}

நிலையான StringRef maxPlainSubstring(StringRef S, தருக்க SlashAgnostic) {
  மாறிலி எழுத்து *Metas =
      SlashAgnostic ? PrefixMetacharactersWithSlash : PrefixMetacharacters;
  StringRef Best;
  போது (!S.empty()) {
    size_t PrefixSize = S.find_first_of(Metas);
    எனில் (PrefixSize == std::string::npos)
      PrefixSize = S.size();

    எனில் (Best.size() < PrefixSize)
      Best = S.take_front(PrefixSize);

    S = S.drop_front(PrefixSize);

    // It's impossible, as the first and last characters of the input string
    // must be Glob special characters, otherwise they would be parts of
    // the prefix or the suffix.
    ப்ரதிஜ்ஞா(!S.empty());

    தேர்வு (S.front()) {
    நிலை '\\':
      S = S.drop_front(2);
      நிறுத்து;
    நிலை '[': {
      // Drop '[' and the first character which can be ']'.
      S = S.drop_front(2);
      size_t EndBracket = S.find_first_of("]");
      // Should not be possible, SubGlobPattern::create should fail on invalid
      // pattern before we get here.
      ப்ரதிஜ்ஞா(EndBracket != std::string::npos);
      S = S.drop_front(EndBracket + 1);
      நிறுத்து;
    }
    நிலை '{':
      // TODO: implement.
      // Fallback to whatever is best for now.
      திரும்பு Best;
    இயல்பு:
      S = S.drop_front(1);
    }
  }

  திரும்பு Best;
}

Expected<GlobPattern> GlobPattern::create(StringRef S,
                                          std::optional<size_t> MaxSubPatterns,
                                          தருக்க SlashAgnostic) {
  GlobPattern Pat;
  Pat.SlashAgnostic = SlashAgnostic;
  Pat.Pattern = S;

  மாறிலி எழுத்து *PrefixMetas =
      SlashAgnostic ? PrefixMetacharactersWithSlash : PrefixMetacharacters;
  மாறிலி எழுத்து *SuffixMetas =
      SlashAgnostic ? SuffixMetacharactersWithSlash : SuffixMetacharacters;

  // Store the prefix that does not contain any metacharacter.
  Pat.PrefixSize = S.find_first_of(PrefixMetas);
  எனில் (Pat.PrefixSize == std::string::npos) {
    Pat.PrefixSize = S.size();
    திரும்பு Pat;
  }
  S = S.substr(Pat.PrefixSize);

  // Just in case we stop on unmatched opening brackets.
  size_t SuffixStart = S.find_last_of(SuffixMetas);
  ப்ரதிஜ்ஞா(SuffixStart != std::string::npos);
  எனில் (S[SuffixStart] == '\\')
    ++SuffixStart;
  எனில் (SuffixStart < S.size())
    ++SuffixStart;
  Pat.SuffixSize = S.size() - SuffixStart;
  S = S.substr(0, SuffixStart);

  SmallVector<std::string, 1> SubPats;
  எனில் (auto Err = parseBraceExpansions(S, MaxSubPatterns).moveInto(SubPats))
    திரும்பு std::move(Err);
  க்கு (StringRef SubPat : SubPats) {
    auto SubGlobOrErr = SubGlobPattern::create(SubPat, SlashAgnostic);
    எனில் (!SubGlobOrErr)
      திரும்பு SubGlobOrErr.takeError();
    Pat.SubGlobs.தள்ளு_back(*SubGlobOrErr);
  }

  திரும்பு Pat;
}

std::optional<std::string> GlobPattern::asLiteral() மாறிலி {
  // The prefix and suffix are metacharacter-free by construction, so whether
  // this pattern denotes a single string is decided by the sub-patterns, which
  // recorded it while parsing. No sub-pattern at all means there was no
  // metacharacter; more than one means brace expansion produced a choice.
  எனில் (SubGlobs.size() > 1 || (SubGlobs.size() == 1 && !SubGlobs[0].isLiteral()))
    திரும்பு std::nullopt;

  std::string Literal;
  Literal.reserve(Pattern.size());
  க்கு (size_t I = 0, E = Pattern.size(); I != E; ++I) {
    எனில் (Pattern[I] == '\\' && I + 1 != E)
      ++I;
    Literal.தள்ளு_back(Pattern[I]);
  }

  // In slash-agnostic mode '/' and '\\' match each other, so a pattern holding
  // either matches more than one string. A pattern with no sub-pattern cannot
  // reach here holding one, as both are prefix metacharacters in that mode.
  எனில் (SlashAgnostic &&
      StringRef(Literal).find_first_of("/\\") != StringRef::npos)
    திரும்பு std::nullopt;

  திரும்பு Literal;
}

Expected<GlobPattern::SubGlobPattern>
GlobPattern::SubGlobPattern::create(StringRef S, தருக்க SlashAgnostic) {
  SubGlobPattern Pat;

  // Parse brackets.
  Pat.Pat.assign(S.begin(), S.end());
  க்கு (size_t I = 0, E = S.size(); I != E; ++I) {
    எனில் (S[I] == '[') {
      // ']' is allowed as the first character of a character class. '[]' is
      // invalid. So, just skip the first character.
      ++I;
      size_t J = S.find(']', I + 1);
      எனில் (J == StringRef::npos)
        திரும்பு make_error<StringError>("invalid glob pattern, unmatched '['",
                                       errc::invalid_argument);
      StringRef Chars = S.substr(I, J - I);
      தருக்க Invert = S[I] == '^' || S[I] == '!';
      எனில் (Invert)
        Chars = Chars.drop_front();
      Expected<BitVector> BVOrErr = expand(Chars, S);
      எனில் (!BVOrErr)
        திரும்பு BVOrErr.takeError();
      BitVector &BV = *BVOrErr;
      எனில் (SlashAgnostic && (BV['\\'] || BV['/'])) {
        BV.set('\\');
        BV.set('/');
      }
      எனில் (Invert)
        BV.flip();
      Pat.Brackets.தள்ளு_back(Bracket{J + 1, std::move(BV)});
      I = J;
    } இல்லையெனில் எனில் (S[I] == '\\') {
      எனில் (++I == E)
        திரும்பு make_error<StringError>("invalid glob pattern, stray '\\'",
                                       errc::invalid_argument);
    } இல்லையெனில் எனில் (S[I] == '*' || S[I] == '?') {
      Pat.HasWildcard = உண்மை;
    }
  }
  திரும்பு Pat;
}

StringRef GlobPattern::longest_substr() மாறிலி {
  திரும்பு maxPlainSubstring(Pattern.drop_front(PrefixSize).drop_back(SuffixSize),
                           SlashAgnostic);
}

தருக்க GlobPattern::match(StringRef S) மாறிலி {
  எனில் (!S.consume_front(prefix()))
    திரும்பு பொய்;
  எனில் (!S.consume_back(suffix()))
    திரும்பு பொய்;
  எனில் (SubGlobs.empty() && S.empty())
    திரும்பு உண்மை;
  க்கு (auto &Glob : SubGlobs)
    எனில் (Glob.match(S, SlashAgnostic))
      திரும்பு உண்மை;
  திரும்பு பொய்;
}

நிலையான தருக்க matchChar(எழுத்து PatC, எழுத்து QueryC, தருக்க SlashAgnostic) {
  எனில் (PatC == QueryC)
    திரும்பு உண்மை;
  திரும்பு SlashAgnostic && (PatC == '\\' || PatC == '/') &&
         (QueryC == '\\' || QueryC == '/');
}

// Factor the pattern into segments split by '*'. The segment is matched
// sequentianlly by finding the first occurrence past the end of the previous
// match.
தருக்க GlobPattern::SubGlobPattern::match(StringRef Str,
                                        தருக்க SlashAgnostic) மாறிலி {
  மாறிலி எழுத்து *P = Pat.data(), *SegmentBegin = nullptr, *S = Str.data(),
             *SavedS = S;
  மாறிலி எழுத்து *மாறிலி PEnd = P + Pat.size(), *மாறிலி End = S + Str.size();
  size_t B = 0, SavedB = 0;
  போது (S != End) {
    எனில் (P == PEnd)
      ;
    இல்லையெனில் எனில் (*P == '*') {
      // The non-* substring on the left of '*' matches the tail of S. Save the
      // positions to be used by backtracking if we see a mismatch later.
      SegmentBegin = ++P;
      SavedS = S;
      SavedB = B;
      தொடர்;
    } இல்லையெனில் எனில் (*P == '[') {
      எனில் (Brackets[B].Bytes[uint8_t(*S)]) {
        P = Pat.data() + Brackets[B++].NextOffset;
        ++S;
        தொடர்;
      }
    } இல்லையெனில் எனில் (*P == '\\') {
      எனில் (matchChar(*++P, *S, SlashAgnostic)) {
        ++P;
        ++S;
        தொடர்;
      }
    } இல்லையெனில் எனில் (matchChar(*P, *S, SlashAgnostic) || *P == '?') {
      ++P;
      ++S;
      தொடர்;
    }
    எனில் (!SegmentBegin)
      திரும்பு பொய்;
    // We have seen a '*'. Backtrack to the saved positions. Shift the S
    // position to probe the next starting position in the segment.
    P = SegmentBegin;
    S = ++SavedS;
    B = SavedB;
  }
  // All bytes in Str have been matched. Return true if the rest part of Pat is
  // empty or contains only '*'.
  திரும்பு getPat().find_first_not_of('*', P - Pat.data()) == std::string::npos;
}
