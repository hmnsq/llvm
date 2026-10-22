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

استفاده نطاق llvm;

ساكن constexpr حرف پیشوند_Metacharacters[] = "?*[{\\";
ساكن constexpr حرف SuffixMetacharacters[] = "?*[]{}\\";
ساكن constexpr حرف پیشوند_Metacharacters_With_Slash[] = "?*[{\\/";
ساكن constexpr حرف SuffixMetacharactersWithSlash[] = "?*[]{}\\/";

// Expands character ranges and returns a bitmap.
// For example, "a-cf-hz" is expanded to "abcfghz".
ساكن Expected<بیت_وکتور> expand(StringRef S, StringRef Original) {
  بیت_وکتور BV(256, خطأ);

  // Expand X-Y.
  لكل (;;) {
    إذا (S.اندازه_2() < 3)
      اكسر;

    uint8_t شروع = S[0];
    uint8_t انتها_2 = S[2];

    // If it doesn't start with something like X-Y,
    // consume the first character and proceed.
    إذا (S[1] != '-') {
      BV[شروع] = درست;
      S = S.substr(1);
      استمر;
    }

    // It must be in the form of X-Y.
    // Validate it and then interpret the range.
    إذا (شروع > انتها_2)
      أرجع make_error<StringError>("invalid glob pattern: " + Original,
                                     errc::invalid_آرگومان);

    لكل (صحيح C = شروع; C <= انتها_2; ++C)
      BV[(uint8_t)C] = درست;
    S = S.substr(3);
  }

  لكل (حرف C : S)
    BV[(uint8_t)C] = درست;
  أرجع BV;
}

// Identify brace expansions in S and return the list of patterns they expand
// into.
ساكن Expected<Small_وکتور<std::string, 1>>
parseBraceExpansions(StringRef S, std::optional<size_t> MaxSubPatterns) {
  Small_وکتور<std::string> SubPatterns = {S.str()};
  إذا (!MaxSubPatterns || !S.شامل_است('{'))
    أرجع std::move(SubPatterns);

  بنية BraceExpansion {
    size_t شروع;
    size_t درازی;
    Small_وکتور<StringRef, 2> Terms;
  };
  Small_وکتور<BraceExpansion, 0> BraceExpansions;

  BraceExpansion *CurrentBE = اشاره_تهی;
  size_t جمله_Begin;
  لكل (size_t I = 0, E = S.اندازه_2(); I != E; ++I) {
    إذا (S[I] == '[') {
      I = S.پیدا_کن(']', I + 2);
      إذا (I == std::string::npos)
        أرجع make_error<StringError>("invalid glob pattern, unmatched '['",
                                       errc::invalid_آرگومان);
    } وإلا إذا (S[I] == '{') {
      إذا (CurrentBE)
        أرجع make_error<StringError>(
            "nested brace expansions are not supported",
            errc::invalid_آرگومان);
      CurrentBE = &BraceExpansions.emplace_پشت();
      CurrentBE->شروع = I;
      جمله_Begin = I + 1;
    } وإلا إذا (S[I] == ',') {
      إذا (!CurrentBE)
        استمر;
      CurrentBE->Terms.افزودن_پشت(S.substr(جمله_Begin, I - جمله_Begin));
      جمله_Begin = I + 1;
    } وإلا إذا (S[I] == '}') {
      إذا (!CurrentBE)
        استمر;
      إذا (CurrentBE->Terms.خالی())
        أرجع make_error<StringError>(
            "empty or singleton brace expansions are not supported",
            errc::invalid_آرگومان);
      CurrentBE->Terms.افزودن_پشت(S.substr(جمله_Begin, I - جمله_Begin));
      CurrentBE->درازی = I - CurrentBE->شروع + 1;
      CurrentBE = اشاره_تهی;
    } وإلا إذا (S[I] == '\\') {
      إذا (++I == E)
        أرجع make_error<StringError>("invalid glob pattern, stray '\\'",
                                       errc::invalid_آرگومان);
    }
  }
  إذا (CurrentBE)
    أرجع make_error<StringError>("incomplete brace expansion",
                                   errc::invalid_آرگومان);

  size_t رقم_Sub_Patterns = 1;
  لكل (خودکار &BE : BraceExpansions) {
    إذا (رقم_Sub_Patterns > std::numeric_limits<size_t>::max() / BE.Terms.اندازه_2()) {
      رقم_Sub_Patterns = std::numeric_limits<size_t>::max();
      اكسر;
    }
    رقم_Sub_Patterns *= BE.Terms.اندازه_2();
  }
  إذا (رقم_Sub_Patterns > *MaxSubPatterns)
    أرجع make_error<StringError>("too many brace expansions",
                                   errc::invalid_آرگومان);
  // Replace brace expansions in reverse order so that we don't invalidate
  // earlier start indices
  لكل (خودکار &BE : معکوس(BraceExpansions)) {
    Small_وکتور<std::string> OrigSubPatterns;
    std::جابجا(SubPatterns, OrigSubPatterns);
    لكل (StringRef جمله : BE.Terms)
      لكل (StringRef Orig : OrigSubPatterns)
        SubPatterns.emplace_پشت(Orig).replace(BE.شروع, BE.درازی, جمله);
  }
  أرجع std::move(SubPatterns);
}

ساكن StringRef maxPlainSubstring(StringRef S, منطقي SlashAgnostic) {
  ثابت حرف *Metas =
      SlashAgnostic ? پیشوند_Metacharacters_With_Slash : پیشوند_Metacharacters;
  StringRef بهترین;
  طالما (!S.خالی()) {
    size_t پیشوند_اندازه = S.پیدا_کن_اولی_از(Metas);
    إذا (پیشوند_اندازه == std::string::npos)
      پیشوند_اندازه = S.اندازه_2();

    إذا (بهترین.اندازه_2() < پیشوند_اندازه)
      بهترین = S.take_front(پیشوند_اندازه);

    S = S.drop_front(پیشوند_اندازه);

    // It's impossible, as the first and last characters of the input string
    // must be Glob special characters, otherwise they would be parts of
    // the prefix or the suffix.
    أكد(!S.خالی());

    بدل (S.front()) {
    حالة '\\':
      S = S.drop_front(2);
      اكسر;
    حالة '[': {
      // Drop '[' and the first character which can be ']'.
      S = S.drop_front(2);
      size_t انتها_Bracket = S.پیدا_کن_اولی_از("]");
      // Should not be possible, SubGlobPattern::create should fail on invalid
      // pattern before we get here.
      أكد(انتها_Bracket != std::string::npos);
      S = S.drop_front(انتها_Bracket + 1);
      اكسر;
    }
    حالة '{':
      // TODO: implement.
      // Fallback to whatever is best for now.
      أرجع بهترین;
    افتراضي:
      S = S.drop_front(1);
    }
  }

  أرجع بهترین;
}

Expected<Glob_الگو> Glob_الگو::create(StringRef S,
                                          std::optional<size_t> MaxSubPatterns,
                                          منطقي SlashAgnostic) {
  Glob_الگو Pat;
  Pat.SlashAgnostic = SlashAgnostic;
  Pat.الگو = S;

  ثابت حرف *پیشوند_Metas =
      SlashAgnostic ? پیشوند_Metacharacters_With_Slash : پیشوند_Metacharacters;
  ثابت حرف *SuffixMetas =
      SlashAgnostic ? SuffixMetacharactersWithSlash : SuffixMetacharacters;

  // Store the prefix that does not contain any metacharacter.
  Pat.پیشوند_اندازه = S.پیدا_کن_اولی_از(پیشوند_Metas);
  إذا (Pat.پیشوند_اندازه == std::string::npos) {
    Pat.پیشوند_اندازه = S.اندازه_2();
    أرجع Pat;
  }
  S = S.substr(Pat.پیشوند_اندازه);

  // Just in case we stop on unmatched opening brackets.
  size_t Suffix_شروع = S.پیدا_کن_آخرین_از(SuffixMetas);
  أكد(Suffix_شروع != std::string::npos);
  إذا (S[Suffix_شروع] == '\\')
    ++Suffix_شروع;
  إذا (Suffix_شروع < S.اندازه_2())
    ++Suffix_شروع;
  Pat.Suffix_اندازه = S.اندازه_2() - Suffix_شروع;
  S = S.substr(0, Suffix_شروع);

  Small_وکتور<std::string, 1> SubPats;
  إذا (خودکار Err = parseBraceExpansions(S, MaxSubPatterns).moveInto(SubPats))
    أرجع std::move(Err);
  لكل (StringRef SubPat : SubPats) {
    خودکار SubGlobOrErr = Sub_Glob_الگو::create(SubPat, SlashAgnostic);
    إذا (!SubGlobOrErr)
      أرجع SubGlobOrErr.takeError();
    Pat.SubGlobs.افزودن_پشت(*SubGlobOrErr);
  }

  أرجع Pat;
}

std::optional<std::string> Glob_الگو::asLiteral() ثابت {
  // The prefix and suffix are metacharacter-free by construction, so whether
  // this pattern denotes a single string is decided by the sub-patterns, which
  // recorded it while parsing. No sub-pattern at all means there was no
  // metacharacter; more than one means brace expansion produced a choice.
  إذا (SubGlobs.اندازه_2() > 1 || (SubGlobs.اندازه_2() == 1 && !SubGlobs[0].isLiteral()))
    أرجع std::nullopt;

  std::string Literal;
  Literal.reserve(الگو.اندازه_2());
  لكل (size_t I = 0, E = الگو.اندازه_2(); I != E; ++I) {
    إذا (الگو[I] == '\\' && I + 1 != E)
      ++I;
    Literal.افزودن_پشت(الگو[I]);
  }

  // In slash-agnostic mode '/' and '\\' match each other, so a pattern holding
  // either matches more than one string. A pattern with no sub-pattern cannot
  // reach here holding one, as both are prefix metacharacters in that mode.
  إذا (SlashAgnostic &&
      StringRef(Literal).پیدا_کن_اولی_از("/\\") != StringRef::npos)
    أرجع std::nullopt;

  أرجع Literal;
}

Expected<Glob_الگو::Sub_Glob_الگو>
Glob_الگو::Sub_Glob_الگو::create(StringRef S, منطقي SlashAgnostic) {
  Sub_Glob_الگو Pat;

  // Parse brackets.
  Pat.Pat.تخصیص_بده(S.begin(), S.انتها());
  لكل (size_t I = 0, E = S.اندازه_2(); I != E; ++I) {
    إذا (S[I] == '[') {
      // ']' is allowed as the first character of a character class. '[]' is
      // invalid. So, just skip the first character.
      ++I;
      size_t J = S.پیدا_کن(']', I + 1);
      إذا (J == StringRef::npos)
        أرجع make_error<StringError>("invalid glob pattern, unmatched '['",
                                       errc::invalid_آرگومان);
      StringRef Chars = S.substr(I, J - I);
      منطقي Invert = S[I] == '^' || S[I] == '!';
      إذا (Invert)
        Chars = Chars.drop_front();
      Expected<بیت_وکتور> BVOrErr = expand(Chars, S);
      إذا (!BVOrErr)
        أرجع BVOrErr.takeError();
      بیت_وکتور &BV = *BVOrErr;
      إذا (SlashAgnostic && (BV['\\'] || BV['/'])) {
        BV.مجموعه('\\');
        BV.مجموعه('/');
      }
      إذا (Invert)
        BV.flip();
      Pat.Brackets.افزودن_پشت(Bracket{J + 1, std::move(BV)});
      I = J;
    } وإلا إذا (S[I] == '\\') {
      إذا (++I == E)
        أرجع make_error<StringError>("invalid glob pattern, stray '\\'",
                                       errc::invalid_آرگومان);
    } وإلا إذا (S[I] == '*' || S[I] == '?') {
      Pat.HasWildcard = درست;
    }
  }
  أرجع Pat;
}

StringRef Glob_الگو::longest_substr() ثابت {
  أرجع maxPlainSubstring(الگو.drop_front(پیشوند_اندازه).drop_پشت(Suffix_اندازه),
                           SlashAgnostic);
}

منطقي Glob_الگو::تطابق(StringRef S) ثابت {
  إذا (!S.consume_front(پیشوند_2()))
    أرجع خطأ;
  إذا (!S.consume_پشت(suffix()))
    أرجع خطأ;
  إذا (SubGlobs.خالی() && S.خالی())
    أرجع درست;
  لكل (خودکار &Glob : SubGlobs)
    إذا (Glob.تطابق(S, SlashAgnostic))
      أرجع درست;
  أرجع خطأ;
}

ساكن منطقي تطابق_Char(حرف PatC, حرف QueryC, منطقي SlashAgnostic) {
  إذا (PatC == QueryC)
    أرجع درست;
  أرجع SlashAgnostic && (PatC == '\\' || PatC == '/') &&
         (QueryC == '\\' || QueryC == '/');
}

// Factor the pattern into segments split by '*'. The segment is matched
// sequentianlly by finding the first occurrence past the end of the previous
// match.
منطقي Glob_الگو::Sub_Glob_الگو::تطابق(StringRef Str,
                                        منطقي SlashAgnostic) ثابت {
  ثابت حرف *P = Pat.داده(), *SegmentBegin = اشاره_تهی, *S = Str.داده(),
             *SavedS = S;
  ثابت حرف *ثابت PEnd = P + Pat.اندازه_2(), *ثابت انتها_2 = S + Str.اندازه_2();
  size_t B = 0, SavedB = 0;
  طالما (S != انتها_2) {
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
      إذا (Brackets[B].بایتها_2[uint8_t(*S)]) {
        P = Pat.داده() + Brackets[B++].بعدی_جابه_جایی;
        ++S;
        استمر;
      }
    } وإلا إذا (*P == '\\') {
      إذا (تطابق_Char(*++P, *S, SlashAgnostic)) {
        ++P;
        ++S;
        استمر;
      }
    } وإلا إذا (تطابق_Char(*P, *S, SlashAgnostic) || *P == '?') {
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
  أرجع getPat().پیدا_کن_اولی_not_از('*', P - Pat.داده()) == std::string::npos;
}
