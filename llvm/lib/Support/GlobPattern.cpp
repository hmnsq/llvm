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

ورتوں نطاق llvm;

ساكن constexpr حرف اگیتر_Metacharacters[] = "?*[{\\";
ساكن constexpr حرف SuffixMetacharacters[] = "?*[]{}\\";
ساكن constexpr حرف اگیتر_Metacharacters_With_Slash[] = "?*[{\\/";
ساكن constexpr حرف SuffixMetacharactersWithSlash[] = "?*[]{}\\/";

// Expands character ranges and returns a bitmap.
// For example, "a-cf-hz" is expanded to "abcfghz".
ساكن Expected<ثنائی_ہندسہ_سمتیہ> expand(StringRef S, StringRef Original) {
  ثنائی_ہندسہ_سمتیہ BV(256, خطأ);

  // Expand X-Y.
  لكل (;;) {
    إذا (S.ناپ_2() < 3)
      اكسر;

    uint8_t شروع = S[0];
    uint8_t سرا_2 = S[2];

    // If it doesn't start with something like X-Y,
    // consume the first character and proceed.
    إذا (S[1] != '-') {
      BV[شروع] = سچ;
      S = S.substr(1);
      استمر;
    }

    // It must be in the form of X-Y.
    // Validate it and then interpret the range.
    إذا (شروع > سرا_2)
      أرجع make_error<StringError>("invalid glob pattern: " + Original,
                                     errc::invalid_دلیل);

    لكل (صحيح C = شروع; C <= سرا_2; ++C)
      BV[(uint8_t)C] = سچ;
    S = S.substr(3);
  }

  لكل (حرف C : S)
    BV[(uint8_t)C] = سچ;
  أرجع BV;
}

// Identify brace expansions in S and return the list of patterns they expand
// into.
ساكن Expected<Small_سمتیہ<std::string, 1>>
parseBraceExpansions(StringRef S, std::optional<size_t> MaxSubPatterns) {
  Small_سمتیہ<std::string> SubPatterns = {S.str()};
  إذا (!MaxSubPatterns || !S.شامل_اے('{'))
    أرجع std::move(SubPatterns);

  بنية BraceExpansion {
    size_t شروع;
    size_t لمائی;
    Small_سمتیہ<StringRef, 2> Terms;
  };
  Small_سمتیہ<BraceExpansion, 0> BraceExpansions;

  BraceExpansion *CurrentBE = خالی_سوچک;
  size_t رکن_Begin;
  لكل (size_t I = 0, E = S.ناپ_2(); I != E; ++I) {
    إذا (S[I] == '[') {
      I = S.لبھو(']', I + 2);
      إذا (I == std::string::npos)
        أرجع make_error<StringError>("invalid glob pattern, unmatched '['",
                                       errc::invalid_دلیل);
    } وإلا إذا (S[I] == '{') {
      إذا (CurrentBE)
        أرجع make_error<StringError>(
            "nested brace expansions are not supported",
            errc::invalid_دلیل);
      CurrentBE = &BraceExpansions.emplace_پٹھ();
      CurrentBE->شروع = I;
      رکن_Begin = I + 1;
    } وإلا إذا (S[I] == ',') {
      إذا (!CurrentBE)
        استمر;
      CurrentBE->Terms.ڈالو_پٹھ(S.substr(رکن_Begin, I - رکن_Begin));
      رکن_Begin = I + 1;
    } وإلا إذا (S[I] == '}') {
      إذا (!CurrentBE)
        استمر;
      إذا (CurrentBE->Terms.خالی_تھاواں())
        أرجع make_error<StringError>(
            "empty or singleton brace expansions are not supported",
            errc::invalid_دلیل);
      CurrentBE->Terms.ڈالو_پٹھ(S.substr(رکن_Begin, I - رکن_Begin));
      CurrentBE->لمائی = I - CurrentBE->شروع + 1;
      CurrentBE = خالی_سوچک;
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
  لكل (خودکار &BE : BraceExpansions) {
    إذا (عدد_Sub_Patterns > std::numeric_limits<size_t>::max() / BE.Terms.ناپ_2()) {
      عدد_Sub_Patterns = std::numeric_limits<size_t>::max();
      اكسر;
    }
    عدد_Sub_Patterns *= BE.Terms.ناپ_2();
  }
  إذا (عدد_Sub_Patterns > *MaxSubPatterns)
    أرجع make_error<StringError>("too many brace expansions",
                                   errc::invalid_دلیل);
  // Replace brace expansions in reverse order so that we don't invalidate
  // earlier start indices
  لكل (خودکار &BE : الٹ(BraceExpansions)) {
    Small_سمتیہ<std::string> OrigSubPatterns;
    std::وٹاندرا(SubPatterns, OrigSubPatterns);
    لكل (StringRef رکن : BE.Terms)
      لكل (StringRef Orig : OrigSubPatterns)
        SubPatterns.emplace_پٹھ(Orig).replace(BE.شروع, BE.لمائی, رکن);
  }
  أرجع std::move(SubPatterns);
}

ساكن StringRef maxPlainSubstring(StringRef S, منطقي SlashAgnostic) {
  ثابت حرف *Metas =
      SlashAgnostic ? اگیتر_Metacharacters_With_Slash : اگیتر_Metacharacters;
  StringRef بہترین;
  طالما (!S.خالی_تھاواں()) {
    size_t اگیتر_ناپ = S.لبھو_پہلا_دا(Metas);
    إذا (اگیتر_ناپ == std::string::npos)
      اگیتر_ناپ = S.ناپ_2();

    إذا (بہترین.ناپ_2() < اگیتر_ناپ)
      بہترین = S.take_front(اگیتر_ناپ);

    S = S.drop_front(اگیتر_ناپ);

    // It's impossible, as the first and last characters of the input string
    // must be Glob special characters, otherwise they would be parts of
    // the prefix or the suffix.
    أكد(!S.خالی_تھاواں());

    بدل (S.front()) {
    حالة '\\':
      S = S.drop_front(2);
      اكسر;
    حالة '[': {
      // Drop '[' and the first character which can be ']'.
      S = S.drop_front(2);
      size_t سرا_Bracket = S.لبھو_پہلا_دا("]");
      // Should not be possible, SubGlobPattern::create should fail on invalid
      // pattern before we get here.
      أكد(سرا_Bracket != std::string::npos);
      S = S.drop_front(سرا_Bracket + 1);
      اكسر;
    }
    حالة '{':
      // TODO: implement.
      // Fallback to whatever is best for now.
      أرجع بہترین;
    افتراضي:
      S = S.drop_front(1);
    }
  }

  أرجع بہترین;
}

Expected<Glob_نمونہ> Glob_نمونہ::create(StringRef S,
                                          std::optional<size_t> MaxSubPatterns,
                                          منطقي SlashAgnostic) {
  Glob_نمونہ Pat;
  Pat.SlashAgnostic = SlashAgnostic;
  Pat.نمونہ = S;

  ثابت حرف *اگیتر_Metas =
      SlashAgnostic ? اگیتر_Metacharacters_With_Slash : اگیتر_Metacharacters;
  ثابت حرف *SuffixMetas =
      SlashAgnostic ? SuffixMetacharactersWithSlash : SuffixMetacharacters;

  // Store the prefix that does not contain any metacharacter.
  Pat.اگیتر_ناپ = S.لبھو_پہلا_دا(اگیتر_Metas);
  إذا (Pat.اگیتر_ناپ == std::string::npos) {
    Pat.اگیتر_ناپ = S.ناپ_2();
    أرجع Pat;
  }
  S = S.substr(Pat.اگیتر_ناپ);

  // Just in case we stop on unmatched opening brackets.
  size_t Suffix_شروع = S.لبھو_آخری_دا(SuffixMetas);
  أكد(Suffix_شروع != std::string::npos);
  إذا (S[Suffix_شروع] == '\\')
    ++Suffix_شروع;
  إذا (Suffix_شروع < S.ناپ_2())
    ++Suffix_شروع;
  Pat.Suffix_ناپ = S.ناپ_2() - Suffix_شروع;
  S = S.substr(0, Suffix_شروع);

  Small_سمتیہ<std::string, 1> SubPats;
  إذا (خودکار Err = parseBraceExpansions(S, MaxSubPatterns).moveInto(SubPats))
    أرجع std::move(Err);
  لكل (StringRef SubPat : SubPats) {
    خودکار SubGlobOrErr = Sub_Glob_نمونہ::create(SubPat, SlashAgnostic);
    إذا (!SubGlobOrErr)
      أرجع SubGlobOrErr.takeError();
    Pat.SubGlobs.ڈالو_پٹھ(*SubGlobOrErr);
  }

  أرجع Pat;
}

std::optional<std::string> Glob_نمونہ::asLiteral() ثابت {
  // The prefix and suffix are metacharacter-free by construction, so whether
  // this pattern denotes a single string is decided by the sub-patterns, which
  // recorded it while parsing. No sub-pattern at all means there was no
  // metacharacter; more than one means brace expansion produced a choice.
  إذا (SubGlobs.ناپ_2() > 1 || (SubGlobs.ناپ_2() == 1 && !SubGlobs[0].isLiteral()))
    أرجع std::nullopt;

  std::string Literal;
  Literal.reserve(نمونہ.ناپ_2());
  لكل (size_t I = 0, E = نمونہ.ناپ_2(); I != E; ++I) {
    إذا (نمونہ[I] == '\\' && I + 1 != E)
      ++I;
    Literal.ڈالو_پٹھ(نمونہ[I]);
  }

  // In slash-agnostic mode '/' and '\\' match each other, so a pattern holding
  // either matches more than one string. A pattern with no sub-pattern cannot
  // reach here holding one, as both are prefix metacharacters in that mode.
  إذا (SlashAgnostic &&
      StringRef(Literal).لبھو_پہلا_دا("/\\") != StringRef::npos)
    أرجع std::nullopt;

  أرجع Literal;
}

Expected<Glob_نمونہ::Sub_Glob_نمونہ>
Glob_نمونہ::Sub_Glob_نمونہ::create(StringRef S, منطقي SlashAgnostic) {
  Sub_Glob_نمونہ Pat;

  // Parse brackets.
  Pat.Pat.سونپو(S.begin(), S.سرا());
  لكل (size_t I = 0, E = S.ناپ_2(); I != E; ++I) {
    إذا (S[I] == '[') {
      // ']' is allowed as the first character of a character class. '[]' is
      // invalid. So, just skip the first character.
      ++I;
      size_t J = S.لبھو(']', I + 1);
      إذا (J == StringRef::npos)
        أرجع make_error<StringError>("invalid glob pattern, unmatched '['",
                                       errc::invalid_دلیل);
      StringRef Chars = S.substr(I, J - I);
      منطقي Invert = S[I] == '^' || S[I] == '!';
      إذا (Invert)
        Chars = Chars.drop_front();
      Expected<ثنائی_ہندسہ_سمتیہ> BVOrErr = expand(Chars, S);
      إذا (!BVOrErr)
        أرجع BVOrErr.takeError();
      ثنائی_ہندسہ_سمتیہ &BV = *BVOrErr;
      إذا (SlashAgnostic && (BV['\\'] || BV['/'])) {
        BV.سیٹ('\\');
        BV.سیٹ('/');
      }
      إذا (Invert)
        BV.flip();
      Pat.Brackets.ڈالو_پٹھ(Bracket{J + 1, std::move(BV)});
      I = J;
    } وإلا إذا (S[I] == '\\') {
      إذا (++I == E)
        أرجع make_error<StringError>("invalid glob pattern, stray '\\'",
                                       errc::invalid_دلیل);
    } وإلا إذا (S[I] == '*' || S[I] == '?') {
      Pat.HasWildcard = سچ;
    }
  }
  أرجع Pat;
}

StringRef Glob_نمونہ::longest_substr() ثابت {
  أرجع maxPlainSubstring(نمونہ.drop_front(اگیتر_ناپ).drop_پٹھ(Suffix_ناپ),
                           SlashAgnostic);
}

منطقي Glob_نمونہ::میل(StringRef S) ثابت {
  إذا (!S.consume_front(اگیتر_2()))
    أرجع خطأ;
  إذا (!S.consume_پٹھ(suffix()))
    أرجع خطأ;
  إذا (SubGlobs.خالی_تھاواں() && S.خالی_تھاواں())
    أرجع سچ;
  لكل (خودکار &Glob : SubGlobs)
    إذا (Glob.میل(S, SlashAgnostic))
      أرجع سچ;
  أرجع خطأ;
}

ساكن منطقي میل_Char(حرف PatC, حرف QueryC, منطقي SlashAgnostic) {
  إذا (PatC == QueryC)
    أرجع سچ;
  أرجع SlashAgnostic && (PatC == '\\' || PatC == '/') &&
         (QueryC == '\\' || QueryC == '/');
}

// Factor the pattern into segments split by '*'. The segment is matched
// sequentianlly by finding the first occurrence past the end of the previous
// match.
منطقي Glob_نمونہ::Sub_Glob_نمونہ::میل(StringRef Str,
                                        منطقي SlashAgnostic) ثابت {
  ثابت حرف *P = Pat.انکڑے(), *SegmentBegin = خالی_سوچک, *S = Str.انکڑے(),
             *SavedS = S;
  ثابت حرف *ثابت PEnd = P + Pat.ناپ_2(), *ثابت سرا_2 = S + Str.ناپ_2();
  size_t B = 0, SavedB = 0;
  طالما (S != سرا_2) {
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
      إذا (Brackets[B].بائٹاں_2[uint8_t(*S)]) {
        P = Pat.انکڑے() + Brackets[B++].اگلا_وتھ;
        ++S;
        استمر;
      }
    } وإلا إذا (*P == '\\') {
      إذا (میل_Char(*++P, *S, SlashAgnostic)) {
        ++P;
        ++S;
        استمر;
      }
    } وإلا إذا (میل_Char(*P, *S, SlashAgnostic) || *P == '?') {
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
  أرجع getPat().لبھو_پہلا_not_دا('*', P - Pat.انکڑے()) == std::string::npos;
}
