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

उपयुज्य नामाकाश llvm;

स्थिर constexpr वर्ण उपसर्गः_Metacharacters[] = "?*[{\\";
स्थिर constexpr वर्ण SuffixMetacharacters[] = "?*[]{}\\";
स्थिर constexpr वर्ण उपसर्गः_Metacharacters_With_Slash[] = "?*[{\\/";
स्थिर constexpr वर्ण SuffixMetacharactersWithSlash[] = "?*[]{}\\/";

// Expands character ranges and returns a bitmap.
// For example, "a-cf-hz" is expanded to "abcfghz".
स्थिर Expected<Bit_सदिशम्> expand(StringRef S, StringRef Original) {
  Bit_सदिशम् BV(256, असत्य);

  // Expand X-Y.
  प्रति (;;) {
    यदि (S.आकार_2() < 3)
      भङ्ग;

    uint8_t Start = S[0];
    uint8_t अन्तः_2 = S[2];

    // If it doesn't start with something like X-Y,
    // consume the first character and proceed.
    यदि (S[1] != '-') {
      BV[Start] = सत्य;
      S = S.substr(1);
      अनुवर्त;
    }

    // It must be in the form of X-Y.
    // Validate it and then interpret the range.
    यदि (Start > अन्तः_2)
      प्रत्यागम make_error<StringError>("invalid glob pattern: " + Original,
                                     errc::invalid_तर्कः);

    प्रति (पूर्णाङ्क C = Start; C <= अन्तः_2; ++C)
      BV[(uint8_t)C] = सत्य;
    S = S.substr(3);
  }

  प्रति (वर्ण C : S)
    BV[(uint8_t)C] = सत्य;
  प्रत्यागम BV;
}

// Identify brace expansions in S and return the list of patterns they expand
// into.
स्थिर Expected<Small_सदिशम्<std::string, 1>>
parseBraceExpansions(StringRef S, std::optional<size_t> MaxSubPatterns) {
  Small_सदिशम्<std::string> SubPatterns = {S.str()};
  यदि (!MaxSubPatterns || !S.अन्तर्भवति('{'))
    प्रत्यागम std::move(SubPatterns);

  संरचना BraceExpansion {
    size_t Start;
    size_t दीर्घता;
    Small_सदिशम्<StringRef, 2> Terms;
  };
  Small_सदिशम्<BraceExpansion, 0> BraceExpansions;

  BraceExpansion *CurrentBE = शून्यसूचकः;
  size_t पदांशः_Begin;
  प्रति (size_t I = 0, E = S.आकार_2(); I != E; ++I) {
    यदि (S[I] == '[') {
      I = S.find(']', I + 2);
      यदि (I == std::string::npos)
        प्रत्यागम make_error<StringError>("invalid glob pattern, unmatched '['",
                                       errc::invalid_तर्कः);
    } अन्यथा यदि (S[I] == '{') {
      यदि (CurrentBE)
        प्रत्यागम make_error<StringError>(
            "nested brace expansions are not supported",
            errc::invalid_तर्कः);
      CurrentBE = &BraceExpansions.emplace_पृष्ठभागः();
      CurrentBE->Start = I;
      पदांशः_Begin = I + 1;
    } अन्यथा यदि (S[I] == ',') {
      यदि (!CurrentBE)
        अनुवर्त;
      CurrentBE->Terms.प्रक्षिप_पृष्ठभागः(S.substr(पदांशः_Begin, I - पदांशः_Begin));
      पदांशः_Begin = I + 1;
    } अन्यथा यदि (S[I] == '}') {
      यदि (!CurrentBE)
        अनुवर्त;
      यदि (CurrentBE->Terms.रिक्तानि())
        प्रत्यागम make_error<StringError>(
            "empty or singleton brace expansions are not supported",
            errc::invalid_तर्कः);
      CurrentBE->Terms.प्रक्षिप_पृष्ठभागः(S.substr(पदांशः_Begin, I - पदांशः_Begin));
      CurrentBE->दीर्घता = I - CurrentBE->Start + 1;
      CurrentBE = शून्यसूचकः;
    } अन्यथा यदि (S[I] == '\\') {
      यदि (++I == E)
        प्रत्यागम make_error<StringError>("invalid glob pattern, stray '\\'",
                                       errc::invalid_तर्कः);
    }
  }
  यदि (CurrentBE)
    प्रत्यागम make_error<StringError>("incomplete brace expansion",
                                   errc::invalid_तर्कः);

  size_t अङ्कः_Sub_Patterns = 1;
  प्रति (स्वचालित &BE : BraceExpansions) {
    यदि (अङ्कः_Sub_Patterns > std::numeric_limits<size_t>::max() / BE.Terms.आकार_2()) {
      अङ्कः_Sub_Patterns = std::numeric_limits<size_t>::max();
      भङ्ग;
    }
    अङ्कः_Sub_Patterns *= BE.Terms.आकार_2();
  }
  यदि (अङ्कः_Sub_Patterns > *MaxSubPatterns)
    प्रत्यागम make_error<StringError>("too many brace expansions",
                                   errc::invalid_तर्कः);
  // Replace brace expansions in reverse order so that we don't invalidate
  // earlier start indices
  प्रति (स्वचालित &BE : reverse(BraceExpansions)) {
    Small_सदिशम्<std::string> OrigSubPatterns;
    std::swap(SubPatterns, OrigSubPatterns);
    प्रति (StringRef पदांशः : BE.Terms)
      प्रति (StringRef Orig : OrigSubPatterns)
        SubPatterns.emplace_पृष्ठभागः(Orig).replace(BE.Start, BE.दीर्घता, पदांशः);
  }
  प्रत्यागम std::move(SubPatterns);
}

स्थिर StringRef maxPlainSubstring(StringRef S, तार्किक SlashAgnostic) {
  ध्रुव वर्ण *Metas =
      SlashAgnostic ? उपसर्गः_Metacharacters_With_Slash : उपसर्गः_Metacharacters;
  StringRef Best;
  यावत् (!S.रिक्तानि()) {
    size_t उपसर्गः_आकार = S.find_first_सम्बन्धि(Metas);
    यदि (उपसर्गः_आकार == std::string::npos)
      उपसर्गः_आकार = S.आकार_2();

    यदि (Best.आकार_2() < उपसर्गः_आकार)
      Best = S.take_front(उपसर्गः_आकार);

    S = S.drop_front(उपसर्गः_आकार);

    // It's impossible, as the first and last characters of the input string
    // must be Glob special characters, otherwise they would be parts of
    // the prefix or the suffix.
    प्रतिज्ञा(!S.रिक्तानि());

    विकल्प (S.front()) {
    पक्ष '\\':
      S = S.drop_front(2);
      भङ्ग;
    पक्ष '[': {
      // Drop '[' and the first character which can be ']'.
      S = S.drop_front(2);
      size_t अन्तः_Bracket = S.find_first_सम्बन्धि("]");
      // Should not be possible, SubGlobPattern::create should fail on invalid
      // pattern before we get here.
      प्रतिज्ञा(अन्तः_Bracket != std::string::npos);
      S = S.drop_front(अन्तः_Bracket + 1);
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

Expected<Glob_आदर्शः> Glob_आदर्शः::create(StringRef S,
                                          std::optional<size_t> MaxSubPatterns,
                                          तार्किक SlashAgnostic) {
  Glob_आदर्शः Pat;
  Pat.SlashAgnostic = SlashAgnostic;
  Pat.आदर्शः = S;

  ध्रुव वर्ण *उपसर्गः_Metas =
      SlashAgnostic ? उपसर्गः_Metacharacters_With_Slash : उपसर्गः_Metacharacters;
  ध्रुव वर्ण *SuffixMetas =
      SlashAgnostic ? SuffixMetacharactersWithSlash : SuffixMetacharacters;

  // Store the prefix that does not contain any metacharacter.
  Pat.उपसर्गः_आकार = S.find_first_सम्बन्धि(उपसर्गः_Metas);
  यदि (Pat.उपसर्गः_आकार == std::string::npos) {
    Pat.उपसर्गः_आकार = S.आकार_2();
    प्रत्यागम Pat;
  }
  S = S.substr(Pat.उपसर्गः_आकार);

  // Just in case we stop on unmatched opening brackets.
  size_t SuffixStart = S.find_चरमः_सम्बन्धि(SuffixMetas);
  प्रतिज्ञा(SuffixStart != std::string::npos);
  यदि (S[SuffixStart] == '\\')
    ++SuffixStart;
  यदि (SuffixStart < S.आकार_2())
    ++SuffixStart;
  Pat.Suffix_आकार = S.आकार_2() - SuffixStart;
  S = S.substr(0, SuffixStart);

  Small_सदिशम्<std::string, 1> SubPats;
  यदि (स्वचालित Err = parseBraceExpansions(S, MaxSubPatterns).moveInto(SubPats))
    प्रत्यागम std::move(Err);
  प्रति (StringRef SubPat : SubPats) {
    स्वचालित SubGlobOrErr = Sub_Glob_आदर्शः::create(SubPat, SlashAgnostic);
    यदि (!SubGlobOrErr)
      प्रत्यागम SubGlobOrErr.takeError();
    Pat.SubGlobs.प्रक्षिप_पृष्ठभागः(*SubGlobOrErr);
  }

  प्रत्यागम Pat;
}

std::optional<std::string> Glob_आदर्शः::asLiteral() ध्रुव {
  // The prefix and suffix are metacharacter-free by construction, so whether
  // this pattern denotes a single string is decided by the sub-patterns, which
  // recorded it while parsing. No sub-pattern at all means there was no
  // metacharacter; more than one means brace expansion produced a choice.
  यदि (SubGlobs.आकार_2() > 1 || (SubGlobs.आकार_2() == 1 && !SubGlobs[0].isLiteral()))
    प्रत्यागम std::nullopt;

  std::string Literal;
  Literal.reserve(आदर्शः.आकार_2());
  प्रति (size_t I = 0, E = आदर्शः.आकार_2(); I != E; ++I) {
    यदि (आदर्शः[I] == '\\' && I + 1 != E)
      ++I;
    Literal.प्रक्षिप_पृष्ठभागः(आदर्शः[I]);
  }

  // In slash-agnostic mode '/' and '\\' match each other, so a pattern holding
  // either matches more than one string. A pattern with no sub-pattern cannot
  // reach here holding one, as both are prefix metacharacters in that mode.
  यदि (SlashAgnostic &&
      StringRef(Literal).find_first_सम्बन्धि("/\\") != StringRef::npos)
    प्रत्यागम std::nullopt;

  प्रत्यागम Literal;
}

Expected<Glob_आदर्शः::Sub_Glob_आदर्शः>
Glob_आदर्शः::Sub_Glob_आदर्शः::create(StringRef S, तार्किक SlashAgnostic) {
  Sub_Glob_आदर्शः Pat;

  // Parse brackets.
  Pat.Pat.नियोजय(S.begin(), S.अन्तः());
  प्रति (size_t I = 0, E = S.आकार_2(); I != E; ++I) {
    यदि (S[I] == '[') {
      // ']' is allowed as the first character of a character class. '[]' is
      // invalid. So, just skip the first character.
      ++I;
      size_t J = S.find(']', I + 1);
      यदि (J == StringRef::npos)
        प्रत्यागम make_error<StringError>("invalid glob pattern, unmatched '['",
                                       errc::invalid_तर्कः);
      StringRef Chars = S.substr(I, J - I);
      तार्किक Invert = S[I] == '^' || S[I] == '!';
      यदि (Invert)
        Chars = Chars.drop_front();
      Expected<Bit_सदिशम्> BVOrErr = expand(Chars, S);
      यदि (!BVOrErr)
        प्रत्यागम BVOrErr.takeError();
      Bit_सदिशम् &BV = *BVOrErr;
      यदि (SlashAgnostic && (BV['\\'] || BV['/'])) {
        BV.समुच्चयः('\\');
        BV.समुच्चयः('/');
      }
      यदि (Invert)
        BV.flip();
      Pat.Brackets.प्रक्षिप_पृष्ठभागः(Bracket{J + 1, std::move(BV)});
      I = J;
    } अन्यथा यदि (S[I] == '\\') {
      यदि (++I == E)
        प्रत्यागम make_error<StringError>("invalid glob pattern, stray '\\'",
                                       errc::invalid_तर्कः);
    } अन्यथा यदि (S[I] == '*' || S[I] == '?') {
      Pat.HasWildcard = सत्य;
    }
  }
  प्रत्यागम Pat;
}

StringRef Glob_आदर्शः::longest_substr() ध्रुव {
  प्रत्यागम maxPlainSubstring(आदर्शः.drop_front(उपसर्गः_आकार).drop_पृष्ठभागः(Suffix_आकार),
                           SlashAgnostic);
}

तार्किक Glob_आदर्शः::मेलनम्(StringRef S) ध्रुव {
  यदि (!S.consume_front(उपसर्गः_2()))
    प्रत्यागम असत्य;
  यदि (!S.consume_पृष्ठभागः(suffix()))
    प्रत्यागम असत्य;
  यदि (SubGlobs.रिक्तानि() && S.रिक्तानि())
    प्रत्यागम सत्य;
  प्रति (स्वचालित &Glob : SubGlobs)
    यदि (Glob.मेलनम्(S, SlashAgnostic))
      प्रत्यागम सत्य;
  प्रत्यागम असत्य;
}

स्थिर तार्किक मेलनम्_Char(वर्ण PatC, वर्ण QueryC, तार्किक SlashAgnostic) {
  यदि (PatC == QueryC)
    प्रत्यागम सत्य;
  प्रत्यागम SlashAgnostic && (PatC == '\\' || PatC == '/') &&
         (QueryC == '\\' || QueryC == '/');
}

// Factor the pattern into segments split by '*'. The segment is matched
// sequentianlly by finding the first occurrence past the end of the previous
// match.
तार्किक Glob_आदर्शः::Sub_Glob_आदर्शः::मेलनम्(StringRef Str,
                                        तार्किक SlashAgnostic) ध्रुव {
  ध्रुव वर्ण *P = Pat.दत्त(), *SegmentBegin = शून्यसूचकः, *S = Str.दत्त(),
             *SavedS = S;
  ध्रुव वर्ण *ध्रुव PEnd = P + Pat.आकार_2(), *ध्रुव अन्तः_2 = S + Str.आकार_2();
  size_t B = 0, SavedB = 0;
  यावत् (S != अन्तः_2) {
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
      यदि (Brackets[B].अष्टकानि_2[uint8_t(*S)]) {
        P = Pat.दत्त() + Brackets[B++].अग्र_विस्थापनम्;
        ++S;
        अनुवर्त;
      }
    } अन्यथा यदि (*P == '\\') {
      यदि (मेलनम्_Char(*++P, *S, SlashAgnostic)) {
        ++P;
        ++S;
        अनुवर्त;
      }
    } अन्यथा यदि (मेलनम्_Char(*P, *S, SlashAgnostic) || *P == '?') {
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
  प्रत्यागम getPat().find_first_not_सम्बन्धि('*', P - Pat.दत्त()) == std::string::npos;
}
