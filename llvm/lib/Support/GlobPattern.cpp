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

استعمال نطاق llvm;

ساكن constexpr حرف اڳياڙي_Metacharacters[] = "?*[{\\";
ساكن constexpr حرف SuffixMetacharacters[] = "?*[]{}\\";
ساكن constexpr حرف اڳياڙي_Metacharacters_With_Slash[] = "?*[{\\/";
ساكن constexpr حرف SuffixMetacharactersWithSlash[] = "?*[]{}\\/";

// Expands character ranges and returns a bitmap.
// For example, "a-cf-hz" is expanded to "abcfghz".
ساكن Expected<ٻٽو_انگ_ويڪٽر> expand(StringRef S, StringRef Original) {
  ٻٽو_انگ_ويڪٽر BV(256, خطأ);

  // Expand X-Y.
  لكل (;;) {
    إذا (S.سائيز_2() < 3)
      اكسر;

    uint8_t شروعات = S[0];
    uint8_t پڇاڙي_2 = S[2];

    // If it doesn't start with something like X-Y,
    // consume the first character and proceed.
    إذا (S[1] != '-') {
      BV[شروعات] = سچ;
      S = S.substr(1);
      استمر;
    }

    // It must be in the form of X-Y.
    // Validate it and then interpret the range.
    إذا (شروعات > پڇاڙي_2)
      أرجع make_error<StringError>("invalid glob pattern: " + Original,
                                     errc::invalid_دليل);

    لكل (صحيح C = شروعات; C <= پڇاڙي_2; ++C)
      BV[(uint8_t)C] = سچ;
    S = S.substr(3);
  }

  لكل (حرف C : S)
    BV[(uint8_t)C] = سچ;
  أرجع BV;
}

// Identify brace expansions in S and return the list of patterns they expand
// into.
ساكن Expected<Small_ويڪٽر<std::string, 1>>
parseBraceExpansions(StringRef S, std::optional<size_t> MaxSubPatterns) {
  Small_ويڪٽر<std::string> SubPatterns = {S.str()};
  إذا (!MaxSubPatterns || !S.شامل_آهي('{'))
    أرجع std::move(SubPatterns);

  بنية BraceExpansion {
    size_t شروعات;
    size_t ڊيگهائي;
    Small_ويڪٽر<StringRef, 2> Terms;
  };
  Small_ويڪٽر<BraceExpansion, 0> BraceExpansions;

  BraceExpansion *CurrentBE = خالي_اشارو;
  size_t رڪن_Begin;
  لكل (size_t I = 0, E = S.سائيز_2(); I != E; ++I) {
    إذا (S[I] == '[') {
      I = S.ڳوليو(']', I + 2);
      إذا (I == std::string::npos)
        أرجع make_error<StringError>("invalid glob pattern, unmatched '['",
                                       errc::invalid_دليل);
    } وإلا إذا (S[I] == '{') {
      إذا (CurrentBE)
        أرجع make_error<StringError>(
            "nested brace expansions are not supported",
            errc::invalid_دليل);
      CurrentBE = &BraceExpansions.emplace_پٺ();
      CurrentBE->شروعات = I;
      رڪن_Begin = I + 1;
    } وإلا إذا (S[I] == ',') {
      إذا (!CurrentBE)
        استمر;
      CurrentBE->Terms.ڌڪو_پٺ(S.substr(رڪن_Begin, I - رڪن_Begin));
      رڪن_Begin = I + 1;
    } وإلا إذا (S[I] == '}') {
      إذا (!CurrentBE)
        استمر;
      إذا (CurrentBE->Terms.خالي_جايون())
        أرجع make_error<StringError>(
            "empty or singleton brace expansions are not supported",
            errc::invalid_دليل);
      CurrentBE->Terms.ڌڪو_پٺ(S.substr(رڪن_Begin, I - رڪن_Begin));
      CurrentBE->ڊيگهائي = I - CurrentBE->شروعات + 1;
      CurrentBE = خالي_اشارو;
    } وإلا إذا (S[I] == '\\') {
      إذا (++I == E)
        أرجع make_error<StringError>("invalid glob pattern, stray '\\'",
                                       errc::invalid_دليل);
    }
  }
  إذا (CurrentBE)
    أرجع make_error<StringError>("incomplete brace expansion",
                                   errc::invalid_دليل);

  size_t انگ_Sub_Patterns = 1;
  لكل (خودڪار &BE : BraceExpansions) {
    إذا (انگ_Sub_Patterns > std::numeric_limits<size_t>::max() / BE.Terms.سائيز_2()) {
      انگ_Sub_Patterns = std::numeric_limits<size_t>::max();
      اكسر;
    }
    انگ_Sub_Patterns *= BE.Terms.سائيز_2();
  }
  إذا (انگ_Sub_Patterns > *MaxSubPatterns)
    أرجع make_error<StringError>("too many brace expansions",
                                   errc::invalid_دليل);
  // Replace brace expansions in reverse order so that we don't invalidate
  // earlier start indices
  لكل (خودڪار &BE : ابتو(BraceExpansions)) {
    Small_ويڪٽر<std::string> OrigSubPatterns;
    std::مٽا_سٽا(SubPatterns, OrigSubPatterns);
    لكل (StringRef رڪن : BE.Terms)
      لكل (StringRef Orig : OrigSubPatterns)
        SubPatterns.emplace_پٺ(Orig).replace(BE.شروعات, BE.ڊيگهائي, رڪن);
  }
  أرجع std::move(SubPatterns);
}

ساكن StringRef maxPlainSubstring(StringRef S, منطقي SlashAgnostic) {
  ثابت حرف *Metas =
      SlashAgnostic ? اڳياڙي_Metacharacters_With_Slash : اڳياڙي_Metacharacters;
  StringRef بهترين;
  طالما (!S.خالي_جايون()) {
    size_t اڳياڙي_سائيز = S.ڳوليو_پهريون_جو(Metas);
    إذا (اڳياڙي_سائيز == std::string::npos)
      اڳياڙي_سائيز = S.سائيز_2();

    إذا (بهترين.سائيز_2() < اڳياڙي_سائيز)
      بهترين = S.take_front(اڳياڙي_سائيز);

    S = S.drop_front(اڳياڙي_سائيز);

    // It's impossible, as the first and last characters of the input string
    // must be Glob special characters, otherwise they would be parts of
    // the prefix or the suffix.
    أكد(!S.خالي_جايون());

    بدل (S.front()) {
    حالة '\\':
      S = S.drop_front(2);
      اكسر;
    حالة '[': {
      // Drop '[' and the first character which can be ']'.
      S = S.drop_front(2);
      size_t پڇاڙي_Bracket = S.ڳوليو_پهريون_جو("]");
      // Should not be possible, SubGlobPattern::create should fail on invalid
      // pattern before we get here.
      أكد(پڇاڙي_Bracket != std::string::npos);
      S = S.drop_front(پڇاڙي_Bracket + 1);
      اكسر;
    }
    حالة '{':
      // TODO: implement.
      // Fallback to whatever is best for now.
      أرجع بهترين;
    افتراضي:
      S = S.drop_front(1);
    }
  }

  أرجع بهترين;
}

Expected<Glob_نمونو> Glob_نمونو::create(StringRef S,
                                          std::optional<size_t> MaxSubPatterns,
                                          منطقي SlashAgnostic) {
  Glob_نمونو Pat;
  Pat.SlashAgnostic = SlashAgnostic;
  Pat.نمونو = S;

  ثابت حرف *اڳياڙي_Metas =
      SlashAgnostic ? اڳياڙي_Metacharacters_With_Slash : اڳياڙي_Metacharacters;
  ثابت حرف *SuffixMetas =
      SlashAgnostic ? SuffixMetacharactersWithSlash : SuffixMetacharacters;

  // Store the prefix that does not contain any metacharacter.
  Pat.اڳياڙي_سائيز = S.ڳوليو_پهريون_جو(اڳياڙي_Metas);
  إذا (Pat.اڳياڙي_سائيز == std::string::npos) {
    Pat.اڳياڙي_سائيز = S.سائيز_2();
    أرجع Pat;
  }
  S = S.substr(Pat.اڳياڙي_سائيز);

  // Just in case we stop on unmatched opening brackets.
  size_t Suffix_شروعات = S.ڳوليو_آخري_جو(SuffixMetas);
  أكد(Suffix_شروعات != std::string::npos);
  إذا (S[Suffix_شروعات] == '\\')
    ++Suffix_شروعات;
  إذا (Suffix_شروعات < S.سائيز_2())
    ++Suffix_شروعات;
  Pat.Suffix_سائيز = S.سائيز_2() - Suffix_شروعات;
  S = S.substr(0, Suffix_شروعات);

  Small_ويڪٽر<std::string, 1> SubPats;
  إذا (خودڪار Err = parseBraceExpansions(S, MaxSubPatterns).moveInto(SubPats))
    أرجع std::move(Err);
  لكل (StringRef SubPat : SubPats) {
    خودڪار SubGlobOrErr = Sub_Glob_نمونو::create(SubPat, SlashAgnostic);
    إذا (!SubGlobOrErr)
      أرجع SubGlobOrErr.takeError();
    Pat.SubGlobs.ڌڪو_پٺ(*SubGlobOrErr);
  }

  أرجع Pat;
}

std::optional<std::string> Glob_نمونو::asLiteral() ثابت {
  // The prefix and suffix are metacharacter-free by construction, so whether
  // this pattern denotes a single string is decided by the sub-patterns, which
  // recorded it while parsing. No sub-pattern at all means there was no
  // metacharacter; more than one means brace expansion produced a choice.
  إذا (SubGlobs.سائيز_2() > 1 || (SubGlobs.سائيز_2() == 1 && !SubGlobs[0].isLiteral()))
    أرجع std::nullopt;

  std::string Literal;
  Literal.reserve(نمونو.سائيز_2());
  لكل (size_t I = 0, E = نمونو.سائيز_2(); I != E; ++I) {
    إذا (نمونو[I] == '\\' && I + 1 != E)
      ++I;
    Literal.ڌڪو_پٺ(نمونو[I]);
  }

  // In slash-agnostic mode '/' and '\\' match each other, so a pattern holding
  // either matches more than one string. A pattern with no sub-pattern cannot
  // reach here holding one, as both are prefix metacharacters in that mode.
  إذا (SlashAgnostic &&
      StringRef(Literal).ڳوليو_پهريون_جو("/\\") != StringRef::npos)
    أرجع std::nullopt;

  أرجع Literal;
}

Expected<Glob_نمونو::Sub_Glob_نمونو>
Glob_نمونو::Sub_Glob_نمونو::create(StringRef S, منطقي SlashAgnostic) {
  Sub_Glob_نمونو Pat;

  // Parse brackets.
  Pat.Pat.سونپيو(S.begin(), S.پڇاڙي());
  لكل (size_t I = 0, E = S.سائيز_2(); I != E; ++I) {
    إذا (S[I] == '[') {
      // ']' is allowed as the first character of a character class. '[]' is
      // invalid. So, just skip the first character.
      ++I;
      size_t J = S.ڳوليو(']', I + 1);
      إذا (J == StringRef::npos)
        أرجع make_error<StringError>("invalid glob pattern, unmatched '['",
                                       errc::invalid_دليل);
      StringRef Chars = S.substr(I, J - I);
      منطقي Invert = S[I] == '^' || S[I] == '!';
      إذا (Invert)
        Chars = Chars.drop_front();
      Expected<ٻٽو_انگ_ويڪٽر> BVOrErr = expand(Chars, S);
      إذا (!BVOrErr)
        أرجع BVOrErr.takeError();
      ٻٽو_انگ_ويڪٽر &BV = *BVOrErr;
      إذا (SlashAgnostic && (BV['\\'] || BV['/'])) {
        BV.سيٽ('\\');
        BV.سيٽ('/');
      }
      إذا (Invert)
        BV.flip();
      Pat.Brackets.ڌڪو_پٺ(Bracket{J + 1, std::move(BV)});
      I = J;
    } وإلا إذا (S[I] == '\\') {
      إذا (++I == E)
        أرجع make_error<StringError>("invalid glob pattern, stray '\\'",
                                       errc::invalid_دليل);
    } وإلا إذا (S[I] == '*' || S[I] == '?') {
      Pat.HasWildcard = سچ;
    }
  }
  أرجع Pat;
}

StringRef Glob_نمونو::longest_substr() ثابت {
  أرجع maxPlainSubstring(نمونو.drop_front(اڳياڙي_سائيز).drop_پٺ(Suffix_سائيز),
                           SlashAgnostic);
}

منطقي Glob_نمونو::ميلاپ(StringRef S) ثابت {
  إذا (!S.consume_front(اڳياڙي_2()))
    أرجع خطأ;
  إذا (!S.consume_پٺ(suffix()))
    أرجع خطأ;
  إذا (SubGlobs.خالي_جايون() && S.خالي_جايون())
    أرجع سچ;
  لكل (خودڪار &Glob : SubGlobs)
    إذا (Glob.ميلاپ(S, SlashAgnostic))
      أرجع سچ;
  أرجع خطأ;
}

ساكن منطقي ميلاپ_Char(حرف PatC, حرف QueryC, منطقي SlashAgnostic) {
  إذا (PatC == QueryC)
    أرجع سچ;
  أرجع SlashAgnostic && (PatC == '\\' || PatC == '/') &&
         (QueryC == '\\' || QueryC == '/');
}

// Factor the pattern into segments split by '*'. The segment is matched
// sequentianlly by finding the first occurrence past the end of the previous
// match.
منطقي Glob_نمونو::Sub_Glob_نمونو::ميلاپ(StringRef Str,
                                        منطقي SlashAgnostic) ثابت {
  ثابت حرف *P = Pat.معلومات(), *SegmentBegin = خالي_اشارو, *S = Str.معلومات(),
             *SavedS = S;
  ثابت حرف *ثابت PEnd = P + Pat.سائيز_2(), *ثابت پڇاڙي_2 = S + Str.سائيز_2();
  size_t B = 0, SavedB = 0;
  طالما (S != پڇاڙي_2) {
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
      إذا (Brackets[B].بائيٽون_2[uint8_t(*S)]) {
        P = Pat.معلومات() + Brackets[B++].ايندڙ_وٿي;
        ++S;
        استمر;
      }
    } وإلا إذا (*P == '\\') {
      إذا (ميلاپ_Char(*++P, *S, SlashAgnostic)) {
        ++P;
        ++S;
        استمر;
      }
    } وإلا إذا (ميلاپ_Char(*P, *S, SlashAgnostic) || *P == '?') {
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
  أرجع getPat().ڳوليو_پهريون_not_جو('*', P - Pat.معلومات()) == std::string::npos;
}
