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

#شامل "llvm/Support/GlobPattern.h"
#شامل "llvm/ADT/StringRef.h"
#شامل "llvm/Support/Errc.h"

استعمال نامخلا llvm;

ساکن constexpr حرف سابقہ_Metacharacters[] = "?*[{\\";
ساکن constexpr حرف SuffixMetacharacters[] = "?*[]{}\\";
ساکن constexpr حرف سابقہ_Metacharacters_With_Slash[] = "?*[{\\/";
ساکن constexpr حرف SuffixMetacharactersWithSlash[] = "?*[]{}\\/";

// Expands character ranges and returns a bitmap.
// For example, "a-cf-hz" is expanded to "abcfghz".
ساکن Expected<ثنائی_ہندسہ_سمتیہ> expand(StringRef S, StringRef Original) {
  ثنائی_ہندسہ_سمتیہ BV(256, جھوٹ);

  // Expand X-Y.
  برائے (;;) {
    اگر (S.سائز_2() < 3)
      توڑو;

    uint8_t آغاز = S[0];
    uint8_t انتہا_2 = S[2];

    // If it doesn't start with something like X-Y,
    // consume the first character and proceed.
    اگر (S[1] != '-') {
      BV[آغاز] = سچ;
      S = S.substr(1);
      جاری;
    }

    // It must be in the form of X-Y.
    // Validate it and then interpret the range.
    اگر (آغاز > انتہا_2)
      واپس make_error<StringError>("invalid glob pattern: " + Original,
                                     errc::invalid_دلیل);

    برائے (صحیح C = آغاز; C <= انتہا_2; ++C)
      BV[(uint8_t)C] = سچ;
    S = S.substr(3);
  }

  برائے (حرف C : S)
    BV[(uint8_t)C] = سچ;
  واپس BV;
}

// Identify brace expansions in S and return the list of patterns they expand
// into.
ساکن Expected<Small_سمتیہ<std::string, 1>>
parseBraceExpansions(StringRef S, std::optional<size_t> MaxSubPatterns) {
  Small_سمتیہ<std::string> SubPatterns = {S.str()};
  اگر (!MaxSubPatterns || !S.شامل_ہے('{'))
    واپس std::move(SubPatterns);

  ساخت BraceExpansion {
    size_t آغاز;
    size_t لمبائی;
    Small_سمتیہ<StringRef, 2> Terms;
  };
  Small_سمتیہ<BraceExpansion, 0> BraceExpansions;

  BraceExpansion *CurrentBE = خالی_اشاریہ;
  size_t رکن_Begin;
  برائے (size_t I = 0, E = S.سائز_2(); I != E; ++I) {
    اگر (S[I] == '[') {
      I = S.ڈھونڈو(']', I + 2);
      اگر (I == std::string::npos)
        واپس make_error<StringError>("invalid glob pattern, unmatched '['",
                                       errc::invalid_دلیل);
    } ورنہ اگر (S[I] == '{') {
      اگر (CurrentBE)
        واپس make_error<StringError>(
            "nested brace expansions are not supported",
            errc::invalid_دلیل);
      CurrentBE = &BraceExpansions.emplace_عقب();
      CurrentBE->آغاز = I;
      رکن_Begin = I + 1;
    } ورنہ اگر (S[I] == ',') {
      اگر (!CurrentBE)
        جاری;
      CurrentBE->Terms.ڈالو_عقب(S.substr(رکن_Begin, I - رکن_Begin));
      رکن_Begin = I + 1;
    } ورنہ اگر (S[I] == '}') {
      اگر (!CurrentBE)
        جاری;
      اگر (CurrentBE->Terms.خالی_خانے())
        واپس make_error<StringError>(
            "empty or singleton brace expansions are not supported",
            errc::invalid_دلیل);
      CurrentBE->Terms.ڈالو_عقب(S.substr(رکن_Begin, I - رکن_Begin));
      CurrentBE->لمبائی = I - CurrentBE->آغاز + 1;
      CurrentBE = خالی_اشاریہ;
    } ورنہ اگر (S[I] == '\\') {
      اگر (++I == E)
        واپس make_error<StringError>("invalid glob pattern, stray '\\'",
                                       errc::invalid_دلیل);
    }
  }
  اگر (CurrentBE)
    واپس make_error<StringError>("incomplete brace expansion",
                                   errc::invalid_دلیل);

  size_t عدد_Sub_Patterns = 1;
  برائے (خودکار &BE : BraceExpansions) {
    اگر (عدد_Sub_Patterns > std::numeric_limits<size_t>::max() / BE.Terms.سائز_2()) {
      عدد_Sub_Patterns = std::numeric_limits<size_t>::max();
      توڑو;
    }
    عدد_Sub_Patterns *= BE.Terms.سائز_2();
  }
  اگر (عدد_Sub_Patterns > *MaxSubPatterns)
    واپس make_error<StringError>("too many brace expansions",
                                   errc::invalid_دلیل);
  // Replace brace expansions in reverse order so that we don't invalidate
  // earlier start indices
  برائے (خودکار &BE : الٹ(BraceExpansions)) {
    Small_سمتیہ<std::string> OrigSubPatterns;
    std::تبادلہ(SubPatterns, OrigSubPatterns);
    برائے (StringRef رکن : BE.Terms)
      برائے (StringRef Orig : OrigSubPatterns)
        SubPatterns.emplace_عقب(Orig).replace(BE.آغاز, BE.لمبائی, رکن);
  }
  واپس std::move(SubPatterns);
}

ساکن StringRef maxPlainSubstring(StringRef S, منطقی SlashAgnostic) {
  ثابت حرف *Metas =
      SlashAgnostic ? سابقہ_Metacharacters_With_Slash : سابقہ_Metacharacters;
  StringRef بہترین;
  جب (!S.خالی_خانے()) {
    size_t سابقہ_سائز = S.ڈھونڈو_پہلا_کا(Metas);
    اگر (سابقہ_سائز == std::string::npos)
      سابقہ_سائز = S.سائز_2();

    اگر (بہترین.سائز_2() < سابقہ_سائز)
      بہترین = S.take_front(سابقہ_سائز);

    S = S.drop_front(سابقہ_سائز);

    // It's impossible, as the first and last characters of the input string
    // must be Glob special characters, otherwise they would be parts of
    // the prefix or the suffix.
    أكد(!S.خالی_خانے());

    انتخاب (S.front()) {
    حالت '\\':
      S = S.drop_front(2);
      توڑو;
    حالت '[': {
      // Drop '[' and the first character which can be ']'.
      S = S.drop_front(2);
      size_t انتہا_Bracket = S.ڈھونڈو_پہلا_کا("]");
      // Should not be possible, SubGlobPattern::create should fail on invalid
      // pattern before we get here.
      أكد(انتہا_Bracket != std::string::npos);
      S = S.drop_front(انتہا_Bracket + 1);
      توڑو;
    }
    حالت '{':
      // TODO: implement.
      // Fallback to whatever is best for now.
      واپس بہترین;
    طےشدہ:
      S = S.drop_front(1);
    }
  }

  واپس بہترین;
}

Expected<Glob_نمونہ> Glob_نمونہ::create(StringRef S,
                                          std::optional<size_t> MaxSubPatterns,
                                          منطقی SlashAgnostic) {
  Glob_نمونہ Pat;
  Pat.SlashAgnostic = SlashAgnostic;
  Pat.نمونہ = S;

  ثابت حرف *سابقہ_Metas =
      SlashAgnostic ? سابقہ_Metacharacters_With_Slash : سابقہ_Metacharacters;
  ثابت حرف *SuffixMetas =
      SlashAgnostic ? SuffixMetacharactersWithSlash : SuffixMetacharacters;

  // Store the prefix that does not contain any metacharacter.
  Pat.سابقہ_سائز = S.ڈھونڈو_پہلا_کا(سابقہ_Metas);
  اگر (Pat.سابقہ_سائز == std::string::npos) {
    Pat.سابقہ_سائز = S.سائز_2();
    واپس Pat;
  }
  S = S.substr(Pat.سابقہ_سائز);

  // Just in case we stop on unmatched opening brackets.
  size_t Suffix_آغاز = S.ڈھونڈو_آخری_کا(SuffixMetas);
  أكد(Suffix_آغاز != std::string::npos);
  اگر (S[Suffix_آغاز] == '\\')
    ++Suffix_آغاز;
  اگر (Suffix_آغاز < S.سائز_2())
    ++Suffix_آغاز;
  Pat.Suffix_سائز = S.سائز_2() - Suffix_آغاز;
  S = S.substr(0, Suffix_آغاز);

  Small_سمتیہ<std::string, 1> SubPats;
  اگر (خودکار Err = parseBraceExpansions(S, MaxSubPatterns).moveInto(SubPats))
    واپس std::move(Err);
  برائے (StringRef SubPat : SubPats) {
    خودکار SubGlobOrErr = Sub_Glob_نمونہ::create(SubPat, SlashAgnostic);
    اگر (!SubGlobOrErr)
      واپس SubGlobOrErr.takeError();
    Pat.SubGlobs.ڈالو_عقب(*SubGlobOrErr);
  }

  واپس Pat;
}

std::optional<std::string> Glob_نمونہ::asLiteral() ثابت {
  // The prefix and suffix are metacharacter-free by construction, so whether
  // this pattern denotes a single string is decided by the sub-patterns, which
  // recorded it while parsing. No sub-pattern at all means there was no
  // metacharacter; more than one means brace expansion produced a choice.
  اگر (SubGlobs.سائز_2() > 1 || (SubGlobs.سائز_2() == 1 && !SubGlobs[0].isLiteral()))
    واپس std::nullopt;

  std::string Literal;
  Literal.reserve(نمونہ.سائز_2());
  برائے (size_t I = 0, E = نمونہ.سائز_2(); I != E; ++I) {
    اگر (نمونہ[I] == '\\' && I + 1 != E)
      ++I;
    Literal.ڈالو_عقب(نمونہ[I]);
  }

  // In slash-agnostic mode '/' and '\\' match each other, so a pattern holding
  // either matches more than one string. A pattern with no sub-pattern cannot
  // reach here holding one, as both are prefix metacharacters in that mode.
  اگر (SlashAgnostic &&
      StringRef(Literal).ڈھونڈو_پہلا_کا("/\\") != StringRef::npos)
    واپس std::nullopt;

  واپس Literal;
}

Expected<Glob_نمونہ::Sub_Glob_نمونہ>
Glob_نمونہ::Sub_Glob_نمونہ::create(StringRef S, منطقی SlashAgnostic) {
  Sub_Glob_نمونہ Pat;

  // Parse brackets.
  Pat.Pat.تفویض(S.begin(), S.انتہا());
  برائے (size_t I = 0, E = S.سائز_2(); I != E; ++I) {
    اگر (S[I] == '[') {
      // ']' is allowed as the first character of a character class. '[]' is
      // invalid. So, just skip the first character.
      ++I;
      size_t J = S.ڈھونڈو(']', I + 1);
      اگر (J == StringRef::npos)
        واپس make_error<StringError>("invalid glob pattern, unmatched '['",
                                       errc::invalid_دلیل);
      StringRef Chars = S.substr(I, J - I);
      منطقی Invert = S[I] == '^' || S[I] == '!';
      اگر (Invert)
        Chars = Chars.drop_front();
      Expected<ثنائی_ہندسہ_سمتیہ> BVOrErr = expand(Chars, S);
      اگر (!BVOrErr)
        واپس BVOrErr.takeError();
      ثنائی_ہندسہ_سمتیہ &BV = *BVOrErr;
      اگر (SlashAgnostic && (BV['\\'] || BV['/'])) {
        BV.سیٹ('\\');
        BV.سیٹ('/');
      }
      اگر (Invert)
        BV.flip();
      Pat.Brackets.ڈالو_عقب(Bracket{J + 1, std::move(BV)});
      I = J;
    } ورنہ اگر (S[I] == '\\') {
      اگر (++I == E)
        واپس make_error<StringError>("invalid glob pattern, stray '\\'",
                                       errc::invalid_دلیل);
    } ورنہ اگر (S[I] == '*' || S[I] == '?') {
      Pat.HasWildcard = سچ;
    }
  }
  واپس Pat;
}

StringRef Glob_نمونہ::longest_substr() ثابت {
  واپس maxPlainSubstring(نمونہ.drop_front(سابقہ_سائز).drop_عقب(Suffix_سائز),
                           SlashAgnostic);
}

منطقی Glob_نمونہ::مطابقت(StringRef S) ثابت {
  اگر (!S.consume_front(سابقہ_2()))
    واپس جھوٹ;
  اگر (!S.consume_عقب(suffix()))
    واپس جھوٹ;
  اگر (SubGlobs.خالی_خانے() && S.خالی_خانے())
    واپس سچ;
  برائے (خودکار &Glob : SubGlobs)
    اگر (Glob.مطابقت(S, SlashAgnostic))
      واپس سچ;
  واپس جھوٹ;
}

ساکن منطقی مطابقت_Char(حرف PatC, حرف QueryC, منطقی SlashAgnostic) {
  اگر (PatC == QueryC)
    واپس سچ;
  واپس SlashAgnostic && (PatC == '\\' || PatC == '/') &&
         (QueryC == '\\' || QueryC == '/');
}

// Factor the pattern into segments split by '*'. The segment is matched
// sequentianlly by finding the first occurrence past the end of the previous
// match.
منطقی Glob_نمونہ::Sub_Glob_نمونہ::مطابقت(StringRef Str,
                                        منطقی SlashAgnostic) ثابت {
  ثابت حرف *P = Pat.ڈیٹا(), *SegmentBegin = خالی_اشاریہ, *S = Str.ڈیٹا(),
             *SavedS = S;
  ثابت حرف *ثابت PEnd = P + Pat.سائز_2(), *ثابت انتہا_2 = S + Str.سائز_2();
  size_t B = 0, SavedB = 0;
  جب (S != انتہا_2) {
    اگر (P == PEnd)
      ;
    ورنہ اگر (*P == '*') {
      // The non-* substring on the left of '*' matches the tail of S. Save the
      // positions to be used by backtracking if we see a mismatch later.
      SegmentBegin = ++P;
      SavedS = S;
      SavedB = B;
      جاری;
    } ورنہ اگر (*P == '[') {
      اگر (Brackets[B].بائٹیں_2[uint8_t(*S)]) {
        P = Pat.ڈیٹا() + Brackets[B++].اگلا_انحراف;
        ++S;
        جاری;
      }
    } ورنہ اگر (*P == '\\') {
      اگر (مطابقت_Char(*++P, *S, SlashAgnostic)) {
        ++P;
        ++S;
        جاری;
      }
    } ورنہ اگر (مطابقت_Char(*P, *S, SlashAgnostic) || *P == '?') {
      ++P;
      ++S;
      جاری;
    }
    اگر (!SegmentBegin)
      واپس جھوٹ;
    // We have seen a '*'. Backtrack to the saved positions. Shift the S
    // position to probe the next starting position in the segment.
    P = SegmentBegin;
    S = ++SavedS;
    B = SavedB;
  }
  // All bytes in Str have been matched. Return true if the rest part of Pat is
  // empty or contains only '*'.
  واپس getPat().ڈھونڈو_پہلا_not_کا('*', P - Pat.ڈیٹا()) == std::string::npos;
}
