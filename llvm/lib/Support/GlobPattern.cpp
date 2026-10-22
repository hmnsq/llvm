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

کارول نطاق llvm;

ساكن constexpr حرف مختاړی_Metacharacters[] = "?*[{\\";
ساكن constexpr حرف SuffixMetacharacters[] = "?*[]{}\\";
ساكن constexpr حرف مختاړی_Metacharacters_With_Slash[] = "?*[{\\/";
ساكن constexpr حرف SuffixMetacharactersWithSlash[] = "?*[]{}\\/";

// Expands character ranges and returns a bitmap.
// For example, "a-cf-hz" is expanded to "abcfghz".
ساكن Expected<بټ_ویکټور> expand(StringRef S, StringRef Original) {
  بټ_ویکټور BV(256, خطأ);

  // Expand X-Y.
  لكل (;;) {
    إذا (S.اندازه_2() < 3)
      اكسر;

    uint8_t پیل = S[0];
    uint8_t پای_ټکی_2 = S[2];

    // If it doesn't start with something like X-Y,
    // consume the first character and proceed.
    إذا (S[1] != '-') {
      BV[پیل] = ریښتیا;
      S = S.substr(1);
      استمر;
    }

    // It must be in the form of X-Y.
    // Validate it and then interpret the range.
    إذا (پیل > پای_ټکی_2)
      أرجع make_error<StringError>("invalid glob pattern: " + Original,
                                     errc::invalid_دلیل);

    لكل (صحيح C = پیل; C <= پای_ټکی_2; ++C)
      BV[(uint8_t)C] = ریښتیا;
    S = S.substr(3);
  }

  لكل (حرف C : S)
    BV[(uint8_t)C] = ریښتیا;
  أرجع BV;
}

// Identify brace expansions in S and return the list of patterns they expand
// into.
ساكن Expected<Small_ویکټور<std::string, 1>>
parseBraceExpansions(StringRef S, std::optional<size_t> MaxSubPatterns) {
  Small_ویکټور<std::string> SubPatterns = {S.str()};
  إذا (!MaxSubPatterns || !S.لري('{'))
    أرجع std::move(SubPatterns);

  بنية BraceExpansion {
    size_t پیل;
    size_t اوږدتیا;
    Small_ویکټور<StringRef, 2> Terms;
  };
  Small_ویکټور<BraceExpansion, 0> BraceExpansions;

  BraceExpansion *CurrentBE = تش_اشاره;
  size_t غړی_Begin;
  لكل (size_t I = 0, E = S.اندازه_2(); I != E; ++I) {
    إذا (S[I] == '[') {
      I = S.پیدا_کړه(']', I + 2);
      إذا (I == std::string::npos)
        أرجع make_error<StringError>("invalid glob pattern, unmatched '['",
                                       errc::invalid_دلیل);
    } وإلا إذا (S[I] == '{') {
      إذا (CurrentBE)
        أرجع make_error<StringError>(
            "nested brace expansions are not supported",
            errc::invalid_دلیل);
      CurrentBE = &BraceExpansions.emplace_شا();
      CurrentBE->پیل = I;
      غړی_Begin = I + 1;
    } وإلا إذا (S[I] == ',') {
      إذا (!CurrentBE)
        استمر;
      CurrentBE->Terms.ټېل_شا(S.substr(غړی_Begin, I - غړی_Begin));
      غړی_Begin = I + 1;
    } وإلا إذا (S[I] == '}') {
      إذا (!CurrentBE)
        استمر;
      إذا (CurrentBE->Terms.تشه())
        أرجع make_error<StringError>(
            "empty or singleton brace expansions are not supported",
            errc::invalid_دلیل);
      CurrentBE->Terms.ټېل_شا(S.substr(غړی_Begin, I - غړی_Begin));
      CurrentBE->اوږدتیا = I - CurrentBE->پیل + 1;
      CurrentBE = تش_اشاره;
    } وإلا إذا (S[I] == '\\') {
      إذا (++I == E)
        أرجع make_error<StringError>("invalid glob pattern, stray '\\'",
                                       errc::invalid_دلیل);
    }
  }
  إذا (CurrentBE)
    أرجع make_error<StringError>("incomplete brace expansion",
                                   errc::invalid_دلیل);

  size_t عدد_Sub_Patterns = 1;
  لكل (خپلکاری &BE : BraceExpansions) {
    إذا (عدد_Sub_Patterns > std::numeric_limits<size_t>::max() / BE.Terms.اندازه_2()) {
      عدد_Sub_Patterns = std::numeric_limits<size_t>::max();
      اكسر;
    }
    عدد_Sub_Patterns *= BE.Terms.اندازه_2();
  }
  إذا (عدد_Sub_Patterns > *MaxSubPatterns)
    أرجع make_error<StringError>("too many brace expansions",
                                   errc::invalid_دلیل);
  // Replace brace expansions in reverse order so that we don't invalidate
  // earlier start indices
  لكل (خپلکاری &BE : سرچپه(BraceExpansions)) {
    Small_ویکټور<std::string> OrigSubPatterns;
    std::بدلون(SubPatterns, OrigSubPatterns);
    لكل (StringRef غړی : BE.Terms)
      لكل (StringRef Orig : OrigSubPatterns)
        SubPatterns.emplace_شا(Orig).replace(BE.پیل, BE.اوږدتیا, غړی);
  }
  أرجع std::move(SubPatterns);
}

ساكن StringRef maxPlainSubstring(StringRef S, منطقي SlashAgnostic) {
  ثابت حرف *Metas =
      SlashAgnostic ? مختاړی_Metacharacters_With_Slash : مختاړی_Metacharacters;
  StringRef غوره;
  طالما (!S.تشه()) {
    size_t مختاړی_اندازه = S.پیدا_کړه_لومړی_د(Metas);
    إذا (مختاړی_اندازه == std::string::npos)
      مختاړی_اندازه = S.اندازه_2();

    إذا (غوره.اندازه_2() < مختاړی_اندازه)
      غوره = S.take_front(مختاړی_اندازه);

    S = S.drop_front(مختاړی_اندازه);

    // It's impossible, as the first and last characters of the input string
    // must be Glob special characters, otherwise they would be parts of
    // the prefix or the suffix.
    أكد(!S.تشه());

    بدل (S.front()) {
    حالة '\\':
      S = S.drop_front(2);
      اكسر;
    حالة '[': {
      // Drop '[' and the first character which can be ']'.
      S = S.drop_front(2);
      size_t پای_ټکی_Bracket = S.پیدا_کړه_لومړی_د("]");
      // Should not be possible, SubGlobPattern::create should fail on invalid
      // pattern before we get here.
      أكد(پای_ټکی_Bracket != std::string::npos);
      S = S.drop_front(پای_ټکی_Bracket + 1);
      اكسر;
    }
    حالة '{':
      // TODO: implement.
      // Fallback to whatever is best for now.
      أرجع غوره;
    افتراضي:
      S = S.drop_front(1);
    }
  }

  أرجع غوره;
}

Expected<Glob_نمونه> Glob_نمونه::create(StringRef S,
                                          std::optional<size_t> MaxSubPatterns,
                                          منطقي SlashAgnostic) {
  Glob_نمونه Pat;
  Pat.SlashAgnostic = SlashAgnostic;
  Pat.نمونه = S;

  ثابت حرف *مختاړی_Metas =
      SlashAgnostic ? مختاړی_Metacharacters_With_Slash : مختاړی_Metacharacters;
  ثابت حرف *SuffixMetas =
      SlashAgnostic ? SuffixMetacharactersWithSlash : SuffixMetacharacters;

  // Store the prefix that does not contain any metacharacter.
  Pat.مختاړی_اندازه = S.پیدا_کړه_لومړی_د(مختاړی_Metas);
  إذا (Pat.مختاړی_اندازه == std::string::npos) {
    Pat.مختاړی_اندازه = S.اندازه_2();
    أرجع Pat;
  }
  S = S.substr(Pat.مختاړی_اندازه);

  // Just in case we stop on unmatched opening brackets.
  size_t Suffix_پیل = S.پیدا_کړه_وروستی_د(SuffixMetas);
  أكد(Suffix_پیل != std::string::npos);
  إذا (S[Suffix_پیل] == '\\')
    ++Suffix_پیل;
  إذا (Suffix_پیل < S.اندازه_2())
    ++Suffix_پیل;
  Pat.Suffix_اندازه = S.اندازه_2() - Suffix_پیل;
  S = S.substr(0, Suffix_پیل);

  Small_ویکټور<std::string, 1> SubPats;
  إذا (خپلکاری Err = parseBraceExpansions(S, MaxSubPatterns).moveInto(SubPats))
    أرجع std::move(Err);
  لكل (StringRef SubPat : SubPats) {
    خپلکاری SubGlobOrErr = Sub_Glob_نمونه::create(SubPat, SlashAgnostic);
    إذا (!SubGlobOrErr)
      أرجع SubGlobOrErr.takeError();
    Pat.SubGlobs.ټېل_شا(*SubGlobOrErr);
  }

  أرجع Pat;
}

std::optional<std::string> Glob_نمونه::asLiteral() ثابت {
  // The prefix and suffix are metacharacter-free by construction, so whether
  // this pattern denotes a single string is decided by the sub-patterns, which
  // recorded it while parsing. No sub-pattern at all means there was no
  // metacharacter; more than one means brace expansion produced a choice.
  إذا (SubGlobs.اندازه_2() > 1 || (SubGlobs.اندازه_2() == 1 && !SubGlobs[0].isLiteral()))
    أرجع std::nullopt;

  std::string Literal;
  Literal.reserve(نمونه.اندازه_2());
  لكل (size_t I = 0, E = نمونه.اندازه_2(); I != E; ++I) {
    إذا (نمونه[I] == '\\' && I + 1 != E)
      ++I;
    Literal.ټېل_شا(نمونه[I]);
  }

  // In slash-agnostic mode '/' and '\\' match each other, so a pattern holding
  // either matches more than one string. A pattern with no sub-pattern cannot
  // reach here holding one, as both are prefix metacharacters in that mode.
  إذا (SlashAgnostic &&
      StringRef(Literal).پیدا_کړه_لومړی_د("/\\") != StringRef::npos)
    أرجع std::nullopt;

  أرجع Literal;
}

Expected<Glob_نمونه::Sub_Glob_نمونه>
Glob_نمونه::Sub_Glob_نمونه::create(StringRef S, منطقي SlashAgnostic) {
  Sub_Glob_نمونه Pat;

  // Parse brackets.
  Pat.Pat.ټاکل(S.begin(), S.پای_ټکی());
  لكل (size_t I = 0, E = S.اندازه_2(); I != E; ++I) {
    إذا (S[I] == '[') {
      // ']' is allowed as the first character of a character class. '[]' is
      // invalid. So, just skip the first character.
      ++I;
      size_t J = S.پیدا_کړه(']', I + 1);
      إذا (J == StringRef::npos)
        أرجع make_error<StringError>("invalid glob pattern, unmatched '['",
                                       errc::invalid_دلیل);
      StringRef Chars = S.substr(I, J - I);
      منطقي Invert = S[I] == '^' || S[I] == '!';
      إذا (Invert)
        Chars = Chars.drop_front();
      Expected<بټ_ویکټور> BVOrErr = expand(Chars, S);
      إذا (!BVOrErr)
        أرجع BVOrErr.takeError();
      بټ_ویکټور &BV = *BVOrErr;
      إذا (SlashAgnostic && (BV['\\'] || BV['/'])) {
        BV.سېټ('\\');
        BV.سېټ('/');
      }
      إذا (Invert)
        BV.flip();
      Pat.Brackets.ټېل_شا(Bracket{J + 1, std::move(BV)});
      I = J;
    } وإلا إذا (S[I] == '\\') {
      إذا (++I == E)
        أرجع make_error<StringError>("invalid glob pattern, stray '\\'",
                                       errc::invalid_دلیل);
    } وإلا إذا (S[I] == '*' || S[I] == '?') {
      Pat.HasWildcard = ریښتیا;
    }
  }
  أرجع Pat;
}

StringRef Glob_نمونه::longest_substr() ثابت {
  أرجع maxPlainSubstring(نمونه.drop_front(مختاړی_اندازه).drop_شا(Suffix_اندازه),
                           SlashAgnostic);
}

منطقي Glob_نمونه::سمون(StringRef S) ثابت {
  إذا (!S.consume_front(مختاړی_2()))
    أرجع خطأ;
  إذا (!S.consume_شا(suffix()))
    أرجع خطأ;
  إذا (SubGlobs.تشه() && S.تشه())
    أرجع ریښتیا;
  لكل (خپلکاری &Glob : SubGlobs)
    إذا (Glob.سمون(S, SlashAgnostic))
      أرجع ریښتیا;
  أرجع خطأ;
}

ساكن منطقي سمون_Char(حرف PatC, حرف QueryC, منطقي SlashAgnostic) {
  إذا (PatC == QueryC)
    أرجع ریښتیا;
  أرجع SlashAgnostic && (PatC == '\\' || PatC == '/') &&
         (QueryC == '\\' || QueryC == '/');
}

// Factor the pattern into segments split by '*'. The segment is matched
// sequentianlly by finding the first occurrence past the end of the previous
// match.
منطقي Glob_نمونه::Sub_Glob_نمونه::سمون(StringRef Str,
                                        منطقي SlashAgnostic) ثابت {
  ثابت حرف *P = Pat.معلومات(), *SegmentBegin = تش_اشاره, *S = Str.معلومات(),
             *SavedS = S;
  ثابت حرف *ثابت PEnd = P + Pat.اندازه_2(), *ثابت پای_ټکی_2 = S + Str.اندازه_2();
  size_t B = 0, SavedB = 0;
  طالما (S != پای_ټکی_2) {
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
      إذا (Brackets[B].بایټونه_2[uint8_t(*S)]) {
        P = Pat.معلومات() + Brackets[B++].بل_دننه_واټن;
        ++S;
        استمر;
      }
    } وإلا إذا (*P == '\\') {
      إذا (سمون_Char(*++P, *S, SlashAgnostic)) {
        ++P;
        ++S;
        استمر;
      }
    } وإلا إذا (سمون_Char(*P, *S, SlashAgnostic) || *P == '?') {
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
  أرجع getPat().پیدا_کړه_لومړی_not_د('*', P - Pat.معلومات()) == std::string::npos;
}
