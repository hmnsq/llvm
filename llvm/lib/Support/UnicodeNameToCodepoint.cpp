//===- llvm/Support/UnicodeNameToCodepoint.cpp - Unicode character properties
//-*- C++ -*-===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
// This file implements functions to map the name or alias of a unicode
// character to its codepoint.
//
//===----------------------------------------------------------------------===//

#ضمن "llvm/ADT/STLExtras.h"
#ضمن "llvm/ADT/StringExtras.h"
#ضمن "llvm/ADT/StringRef.h"
#ضمن "llvm/Support/Unicode.h"

نطاق llvm {
نطاق sys {
نطاق unicode {

خارجي ثابت حرف *ثابت Unicode_Name_منزل_Codepoint_Dict;
خارجي ثابت uint8_t *ثابت Unicode_Name_منزل_Codepoint_شاخص;
خارجي ثابت std::size_t Unicode_Name_منزل_Codepoint_شاخص_اندازه;
خارجي ثابت std::size_t Unicode_Name_منزل_Codepoint_تر_ټولو_لوی_Name_اندازه;

کارول بفر_Type = SmallString<64>;

بنية غوټه {
  منطقي Is_ریښه = خطأ;
  char32_t ارزښت = 0xFFFFFFFF;
  uint32_t Children_دننه_واټن = 0;
  منطقي HasSibling = خطأ;
  uint32_t اندازه = 0;
  StringRef Name;
  ثابت غوټه *پلار_2 = تش_اشاره;

  constexpr منطقي isValid() ثابت {
    أرجع !Name.تشه() || ارزښت == 0xFFFFFFFF;
  }
  constexpr منطقي hasChildren() ثابت { أرجع Children_دننه_واټن != 0 || Is_ریښه; }

  std::string ډک_Name() ثابت {
    std::string S;
    // Reserve enough space for most unicode code points.
    // The chosen value represent the 99th percentile of name size as of
    // Unicode 15.0.
    S.reserve(46);
    ثابت غوټه *N = هذا;
    طالما (N) {
      std::سرچپه_copy(N->Name.begin(), N->Name.پای_ټکی(), std::شا_inserter(S));
      N = N->پلار_2;
    }
    std::سرچپه(S.begin(), S.پای_ټکی());
    أرجع S;
  }
};

ساكن غوټه create_ریښه() {
  غوټه N;
  N.Is_ریښه = ریښتیا;
  N.Children_دننه_واټن = 1;
  N.اندازه = 1;
  أرجع N;
}

ساكن غوټه read_غوټه(uint32_t دننه_واټن, ثابت غوټه *پلار_2 = تش_اشاره) {
  إذا (دننه_واټن == 0)
    أرجع create_ریښه();

  uint32_t Origin = دننه_واټن;
  غوټه N;
  N.پلار_2 = پلار_2;
  uint8_t NameInfo = Unicode_Name_منزل_Codepoint_شاخص[دننه_واټن++];
  إذا (دننه_واټن + 6 >= Unicode_Name_منزل_Codepoint_شاخص_اندازه)
    أرجع N;

  منطقي LongName = NameInfo & 0x40;
  منطقي Has_ارزښت = NameInfo & 0x80;
  std::size_t اندازه = NameInfo & ~0xC0;
  إذا (LongName) {
    uint32_t Name_دننه_واټن = (Unicode_Name_منزل_Codepoint_شاخص[دننه_واټن++] << 8);
    Name_دننه_واټن |= Unicode_Name_منزل_Codepoint_شاخص[دننه_واټن++];
    N.Name = StringRef(Unicode_Name_منزل_Codepoint_Dict + Name_دننه_واټن, اندازه);
  } وإلا {
    N.Name = StringRef(Unicode_Name_منزل_Codepoint_Dict + اندازه, 1);
  }
  إذا (Has_ارزښت) {
    uint8_t H = Unicode_Name_منزل_Codepoint_شاخص[دننه_واټن++];
    uint8_t M = Unicode_Name_منزل_Codepoint_شاخص[دننه_واټن++];
    uint8_t L = Unicode_Name_منزل_Codepoint_شاخص[دننه_واټن++];
    N.ارزښت = ((H << 16) | (M << 8) | L) >> 3;

    منطقي HasChildren = L & 0x02;
    N.HasSibling = L & 0x01;

    إذا (HasChildren) {
      N.Children_دننه_واټن = Unicode_Name_منزل_Codepoint_شاخص[دننه_واټن++] << 16;
      N.Children_دننه_واټن |= Unicode_Name_منزل_Codepoint_شاخص[دننه_واټن++] << 8;
      N.Children_دننه_واټن |= Unicode_Name_منزل_Codepoint_شاخص[دننه_واټن++];
    }
  } وإلا {
    uint8_t H = Unicode_Name_منزل_Codepoint_شاخص[دننه_واټن++];
    N.HasSibling = H & 0x80;
    منطقي HasChildren = H & 0x40;
    H &= uint8_t(~0xC0);
    إذا (HasChildren) {
      N.Children_دننه_واټن = (H << 16);
      N.Children_دننه_واټن |=
          (uint32_t(Unicode_Name_منزل_Codepoint_شاخص[دننه_واټن++]) << 8);
      N.Children_دننه_واټن |= Unicode_Name_منزل_Codepoint_شاخص[دننه_واټن++];
    }
  }
  N.اندازه = دننه_واټن - Origin;
  أرجع N;
}

ساكن منطقي startsWith(StringRef Name, StringRef Needle, منطقي Strict,
                       std::size_t &Consummed, حرف &پخوانی_Char_دننه_Name,
                       منطقي Is_مختاړی = خطأ) {

  Consummed = 0;
  إذا (Strict) {
    إذا (!Name.starts_with(Needle))
      أرجع خطأ;
    Consummed = Needle.اندازه_2();
    أرجع ریښتیا;
  }
  إذا (Needle.تشه())
    أرجع ریښتیا;

  خپلکاری NamePos = Name.begin();
  خپلکاری NeedlePos = Needle.begin();

  حرف پخوانی_Char_دننه_Name_Origin = پخوانی_Char_دننه_Name;
  حرف پخوانی_Char_دننه_Needle = *Needle.begin();
  خپلکاری IgnoreSpaces = [](خپلکاری It, خپلکاری پای_ټکی_2, حرف &پخوانی_Char,
                         منطقي Is_مختاړی = خطأ) {
    طالما (It != پای_ټکی_2) {
      ثابت خپلکاری بل_2 = std::بل(It);
      // Ignore spaces, underscore, medial hyphens
      // The generator ensures a needle never ends (or starts) by a medial
      // hyphen https://unicode.org/reports/tr44/#UAX44-LM2.
      منطقي Ignore =
          *It == ' ' || *It == '_' ||
          (*It == '-' && isAlnum(پخوانی_Char) &&
           ((بل_2 != پای_ټکی_2 && isAlnum(*بل_2)) || (بل_2 == پای_ټکی_2 && Is_مختاړی)));
      پخوانی_Char = *It;
      إذا (!Ignore)
        اكسر;
      ++It;
    }
    أرجع It;
  };

  طالما (ریښتیا) {
    NamePos = IgnoreSpaces(NamePos, Name.پای_ټکی(), پخوانی_Char_دننه_Name);
    NeedlePos =
        IgnoreSpaces(NeedlePos, Needle.پای_ټکی(), پخوانی_Char_دننه_Needle, Is_مختاړی);
    إذا (NeedlePos == Needle.پای_ټکی())
      اكسر;
    إذا (NamePos == Name.پای_ټکی())
      اكسر;
    إذا (منزل_پاسنی(*NeedlePos) != منزل_پاسنی(*NamePos))
      اكسر;
    NeedlePos++;
    NamePos++;
  }
  Consummed = std::واټن(Name.begin(), NamePos);
  إذا (NeedlePos != Needle.پای_ټکی()) {
    پخوانی_Char_دننه_Name = پخوانی_Char_دننه_Name_Origin;
  }
  أرجع NeedlePos == Needle.پای_ټکی();
}

ساكن std::tuple<غوټه, منطقي, uint32_t>
compare_غوټه(uint32_t دننه_واټن, StringRef Name, منطقي Strict,
            حرف پخوانی_Char_دننه_Name, بفر_Type &بفر,
            ثابت غوټه *پلار_2 = تش_اشاره) {
  غوټه N = read_غوټه(دننه_واټن, پلار_2);
  std::size_t Consummed = 0;
  منطقي Does_پیل_With = N.Is_ریښه || startsWith(Name, N.Name, Strict, Consummed,
                                              پخوانی_Char_دننه_Name);
  إذا (!Does_پیل_With)
    أرجع std::make_tuple(N, خطأ, 0);

  إذا (Name.اندازه_2() - Consummed == 0 && N.ارزښت != 0xFFFFFFFF)
    أرجع std::make_tuple(N, ریښتیا, N.ارزښت);

  إذا (N.hasChildren()) {
    uint32_t ماشوم_دننه_واټن = N.Children_دننه_واټن;
    لكل (;;) {
      غوټه C;
      منطقي Matches;
      uint32_t ارزښت;
      std::tie(C, Matches, ارزښت) =
          compare_غوټه(ماشوم_دننه_واټن, Name.substr(Consummed), Strict,
                      پخوانی_Char_دننه_Name, بفر, &N);
      إذا (Matches) {
        std::سرچپه_copy(C.Name.begin(), C.Name.پای_ټکی(),
                          std::شا_inserter(بفر));
        أرجع std::make_tuple(N, ریښتیا, ارزښت);
      }
      ماشوم_دننه_واټن += C.اندازه;
      إذا (!C.HasSibling)
        اكسر;
    }
  }
  أرجع std::make_tuple(N, خطأ, 0);
}

ساكن std::tuple<غوټه, منطقي, uint32_t>
compare_غوټه(uint32_t دننه_واټن, StringRef Name, منطقي Strict, بفر_Type &بفر) {
  أرجع compare_غوټه(دننه_واټن, Name, Strict, 0, بفر);
}

// clang-format off
constexpr ثابت حرف *ثابت HangulSyllables[][3] = {
    { "G",  "A",   ""   },
    { "GG", "AE",  "G"  },
    { "N",  "YA",  "GG" },
    { "D",  "YAE", "GS" },
    { "DD", "EO",  "N", },
    { "R",  "E",   "NJ" },
    { "M",  "YEO", "NH" },
    { "B",  "YE",  "D"  },
    { "BB", "O",   "L"  },
    { "S",  "WA",  "LG" },
    { "SS", "WAE", "LM" },
    { "",   "OE",  "LB" },
    { "J",  "YO",  "LS" },
    { "JJ", "U",   "LT" },
    { "C",  "WEO", "LP" },
    { "K",  "WE",  "LH" },
    { "T",  "WI",  "M"  },
    { "P",  "YU",  "B"  },
    { "H",  "EU",  "BS" },
    { 0,    "YI",  "S"  },
    { 0,    "I",   "SS" },
    { 0,    0,     "NG" },
    { 0,    0,     "J"  },
    { 0,    0,     "C"  },
    { 0,    0,     "K"  },
    { 0,    0,     "T"  },
    { 0,    0,     "P"  },
    { 0,    0,     "H"  }
    };
// clang-format on

// Unicode 18.0
// 3.12 Conjoining Jamo Behavior Common constants
constexpr char32_t SBase = 0xAC00;
constexpr uint32_t LCount = 19;
constexpr uint32_t VCount = 21;
constexpr uint32_t TCount = 28;

ساكن std::size_t پیدا_کړه_Syllable(StringRef Name, منطقي Strict,
                                حرف &پخوانی_دننه_Name, صحيح &Pos, صحيح ستنه) {
  أكد(ستنه == 0 || ستنه == 1 || ستنه == 2);
  ساكن std::size_t شمېر_فی_ستنه[] = {LCount, VCount, TCount};
  صحيح Len = -1;
  صحيح Prev = پخوانی_دننه_Name;
  لكل (std::size_t I = 0; I < شمېر_فی_ستنه[ستنه]; I++) {
    StringRef Syllable(HangulSyllables[I][ستنه]);
    إذا (صحيح(Syllable.اندازه_2()) <= Len)
      استمر;
    std::size_t Consummed = 0;
    حرف پخوانی_دننه_Name_Copy = پخوانی_دننه_Name;
    منطقي Does_پیل_With =
        startsWith(Name, Syllable, Strict, Consummed, پخوانی_دننه_Name_Copy);
    إذا (!Does_پیل_With)
      استمر;
    Len = Consummed;
    Pos = I;
    Prev = پخوانی_دننه_Name_Copy;
  }
  إذا (Len == -1)
    أرجع 0;
  پخوانی_دننه_Name = Prev;
  أرجع size_t(Len);
}

ساكن std::optional<char32_t>
name_منزل_Hangul_کوډ_ټکی(StringRef Name, منطقي Strict, بفر_Type &بفر) {
  بفر.clear();
  // Hangul Syllable Decomposition
  std::size_t Consummed = 0;
  حرف Name_پیل = 0;
  منطقي Does_پیل_With =
      startsWith(Name, "HANGUL SYLLABLE ", Strict, Consummed, Name_پیل);
  إذا (!Does_پیل_With)
    أرجع std::nullopt;
  Name = Name.substr(Consummed);
  صحيح L = -1, V = -1, T = -1;
  Name = Name.substr(پیدا_کړه_Syllable(Name, Strict, Name_پیل, L, 0));
  Name = Name.substr(پیدا_کړه_Syllable(Name, Strict, Name_پیل, V, 1));
  Name = Name.substr(پیدا_کړه_Syllable(Name, Strict, Name_پیل, T, 2));
  إذا (L != -1 && V != -1 && T != -1 && Name.تشه()) {
    إذا (!Strict) {
      بفر.append("HANGUL SYLLABLE ");
      إذا (L != -1)
        بفر.append(HangulSyllables[L][0]);
      إذا (V != -1)
        بفر.append(HangulSyllables[V][1]);
      إذا (T != -1)
        بفر.append(HangulSyllables[T][2]);
    }
    أرجع SBase + (std::uint32_t(L) * VCount + std::uint32_t(V)) * TCount +
           std::uint32_t(T);
  }
  // Otherwise, it's an illegal syllable name.
  أرجع std::nullopt;
}

بنية Generated_نومونه_معلومات {
  StringRef مختاړی;
  uint32_t پیل;
  uint32_t پای_ټکی_2;
};

// Unicode 18.0 Name Derivation Rule Prefix Strings (extracted/DerivedName.txt)
ساكن ثابت Generated_نومونه_معلومات Generated_نومونه_معلومات_جدول[] = {
    {"CJK UNIFIED IDEOGRAPH-", 0x3400, 0x4DBF},
    {"CJK UNIFIED IDEOGRAPH-", 0x4E00, 0x9FFF},
    {"CJK UNIFIED IDEOGRAPH-", 0x20000, 0x2A6DF},
    {"CJK UNIFIED IDEOGRAPH-", 0x2A700, 0x2B73F},
    {"CJK UNIFIED IDEOGRAPH-", 0x2B740, 0x2B81E},
    {"CJK UNIFIED IDEOGRAPH-", 0x2B820, 0x2CEAD},
    {"CJK UNIFIED IDEOGRAPH-", 0x2CEB0, 0x2EBE0},
    {"CJK UNIFIED IDEOGRAPH-", 0x2EBF0, 0x2EE5D},
    {"CJK UNIFIED IDEOGRAPH-", 0x30000, 0x3134A},
    {"CJK UNIFIED IDEOGRAPH-", 0x31350, 0x323AF},
    {"CJK UNIFIED IDEOGRAPH-", 0x323B0, 0x33479},
    {"EGYPTIAN HIEROGLYPH-", 0x13460, 0x143FA},
    {"TANGUT IDEOGRAPH-", 0x17000, 0x187FF},
    {"TANGUT IDEOGRAPH-", 0x18D00, 0x18D20},
    {"KHITAN SMALL SCRIPT CHARACTER-", 0x18B00, 0x18CDA},
    {"KHITAN SMALL SCRIPT CHARACTER-", 0x18CFF, 0x18CFF},
    {"JURCHEN CHARACTER-", 0x18E00, 0x19191},
    {"NUSHU CHARACTER-", 0x1B170, 0x1B2FB},
    {"SMALL SEAL CHARACTER-", 0x3D000, 0x3FC3F},
    {"CJK COMPATIBILITY IDEOGRAPH-", 0xF900, 0xFA6D},
    {"CJK COMPATIBILITY IDEOGRAPH-", 0xFA70, 0xFAD9},
    {"CJK COMPATIBILITY IDEOGRAPH-", 0x2F800, 0x2FA1D},
};

ساكن std::optional<char32_t>
name_منزل_Generated_کوډ_ټکی(StringRef Name, منطقي Strict, بفر_Type &بفر) {
  لكل (خپلکاری &&توکی : Generated_نومونه_معلومات_جدول) {
    بفر.clear();
    std::size_t Consummed = 0;
    حرف Name_پیل = 0;
    منطقي Does_پیل_With = startsWith(Name, توکی.مختاړی, Strict, Consummed,
                                    Name_پیل, /*IsPrefix=*/ریښتیا);
    إذا (!Does_پیل_With)
      استمر;
    خپلکاری شمېره = Name.substr(Consummed);
    بینښه اوږد اوږد V = 0;
    // Be consistent about mandating upper casing.
    إذا (Strict &&
        llvm::any_د(شمېره, [](حرف C) { أرجع C >= 'a' && C <= 'f'; }))
      أرجع {};
    إذا (getAsUnsignedInteger(شمېره, 16, V) || V < توکی.پیل || V > توکی.پای_ټکی_2)
      استمر;
    إذا (!Strict) {
      بفر.append(توکی.مختاړی);
      بفر.append(utohexstr(V, ریښتیا));
    }
    أرجع V;
  }
  أرجع std::nullopt;
}

ساكن std::optional<char32_t> name_منزل_Codepoint(StringRef Name, منطقي Strict,
                                               بفر_Type &بفر) {
  إذا (Name.تشه())
    أرجع std::nullopt;

  std::optional<char32_t> Res = name_منزل_Hangul_کوډ_ټکی(Name, Strict, بفر);
  إذا (!Res)
    Res = name_منزل_Generated_کوډ_ټکی(Name, Strict, بفر);
  إذا (Res)
    أرجع *Res;

  بفر.clear();
  غوټه غوټه;
  منطقي Matches;
  uint32_t ارزښت;
  std::tie(غوټه, Matches, ارزښت) = compare_غوټه(0, Name, Strict, بفر);
  إذا (Matches) {
    std::سرچپه(بفر.begin(), بفر.پای_ټکی());
    // UAX44-LM2. Ignore case, whitespace, underscore ('_'), and all medial
    // hyphens except the hyphen in U+1180 HANGUL JUNGSEONG O-E.
    إذا (!Strict && ارزښت == 0x116c && Name.لري_insensitive("O-E")) {
      بفر = "HANGUL JUNGSEONG O-E";
      ارزښت = 0x1180;
    }
    أرجع ارزښت;
  }
  أرجع std::nullopt;
}

std::optional<char32_t> name_منزل_Codepoint_Strict(StringRef Name) {

  بفر_Type بفر;
  خپلکاری Opt = name_منزل_Codepoint(Name, ریښتیا, بفر);
  أرجع Opt;
}

std::optional<Loose_Matching_پایله>
name_منزل_Codepoint_Loose_Matching(StringRef Name) {
  بفر_Type بفر;
  خپلکاری Opt = name_منزل_Codepoint(Name, خطأ, بفر);
  إذا (!Opt)
    أرجع std::nullopt;
  أرجع Loose_Matching_پایله{*Opt, بفر};
}

// Find the unicode character whose editing distance to Pattern
// is shortest, using the Wagner–Fischer algorithm.
llvm::Small_ویکټور<سمون_For_Codepoint_Name>
تر_ټولو_نږدې_Matches_For_Codepoint_Name(StringRef نمونه, std::size_t Max_Matches_شمېر) {
  // We maintain a fixed size vector of matches,
  // sorted by distance
  // The worst match (with the biggest distance) are discarded when new elements
  // are added.
  std::size_t تر_ټولو_لوی_Edit_واټن = 0;
  llvm::Small_ویکټور<سمون_For_Codepoint_Name> Matches;
  Matches.reserve(Max_Matches_شمېر + 1);

  خپلکاری ورننباسه_2 = [&](ثابت غوټه &غوټه, uint32_t واټن_2,
                    char32_t ارزښت) -> منطقي {
    إذا (واټن_2 > تر_ټولو_لوی_Edit_واټن) {
      إذا (Matches.اندازه_2() == Max_Matches_شمېر)
        أرجع خطأ;
      تر_ټولو_لوی_Edit_واټن = واټن_2;
    }
    // To avoid allocations, the creation of the name is delayed
    // as much as possible.
    std::string Name;
    خپلکاری GetName = [&] {
      إذا (Name.تشه())
        Name = غوټه.ډک_Name();
      أرجع Name;
    };

    خپلکاری It = llvm::ښکتنی_bound(
        Matches, واټن_2,
        [&](ثابت سمون_For_Codepoint_Name &a, std::size_t واټن_2) {
          إذا (واټن_2 == a.واټن_2)
            أرجع a.Name < GetName();
          أرجع a.واټن_2 < واټن_2;
        });
    إذا (It == Matches.پای_ټکی() && Matches.اندازه_2() == Max_Matches_شمېر)
      أرجع خطأ;

    سمون_For_Codepoint_Name M{GetName(), واټن_2, ارزښت};
    Matches.ورننباسه(It, std::move(M));
    إذا (Matches.اندازه_2() > Max_Matches_شمېر)
      Matches.راوباسه_شا();
    أرجع ریښتیا;
  };

  // We ignore case, space, hyphens, etc,
  // in both the search pattern and the prospective names.
  خپلکاری Normalize = [](StringRef Name) {
    std::string بهر;
    بهر.reserve(Name.اندازه_2());
    لكل (حرف C : Name) {
      إذا (isAlnum(C))
        بهر.ټېل_شا(منزل_پاسنی(C));
    }
    أرجع بهر;
  };
  std::string NormalizedName = Normalize(نمونه);

  // Allocate a matrix big enough for longest names.
  ثابت std::size_t Columns =
      std::min(NormalizedName.اندازه_2(), Unicode_Name_منزل_Codepoint_تر_ټولو_لوی_Name_اندازه) +
      1;

  [[maybe_ناکارول_شوی]] ساكن std::size_t لیکې_2 =
      Unicode_Name_منزل_Codepoint_تر_ټولو_لوی_Name_اندازه + 1;

  std::ویکټور<حرف> Distances(
      Columns * (Unicode_Name_منزل_Codepoint_تر_ټولو_لوی_Name_اندازه + 1), 0);

  خپلکاری Get = [&Distances, Columns](size_t ستنه, std::size_t لیکه) -> حرف & {
    أكد(ستنه < Columns);
    أكد(لیکه < لیکې_2);
    أرجع Distances[لیکه * Columns + ستنه];
  };

  لكل (std::size_t I = 0; I < Columns; I++)
    Get(I, 0) = I;

  // Visit the childrens,
  // Filling (and overriding) the matrix for the name fragment of each node
  // iteratively. CompleteName is used to collect the actual name of potential
  // match, respecting case and spacing.
  خپلکاری لیدنه_غوټه = [&](ثابت غوټه &N, std::size_t لیکه,
                       خپلکاری &لیدنه_غوټه) -> فراغ {
    std::size_t J = 0;
    لكل (; J < N.Name.اندازه_2(); J++) {
      إذا (!isAlnum(N.Name[J]))
        استمر;

      Get(0, لیکه) = لیکه;

      لكل (std::size_t I = 1; I < Columns; I++) {
        ثابت صحيح Delete = Get(I - 1, لیکه) + 1;
        ثابت صحيح ورننباسه_2 = Get(I, لیکه - 1) + 1;

        ثابت صحيح Replace =
            Get(I - 1, لیکه - 1) + (NormalizedName[I - 1] != N.Name[J] ? 1 : 0);

        Get(I, لیکه) = std::min({ورننباسه_2, Delete, Replace});
      }

      لیکه++;
    }

    بینښه لګښت = Get(Columns - 1, لیکه - 1);
    إذا (N.ارزښت != 0xFFFFFFFF) {
      ورننباسه_2(N, لګښت, N.ارزښت);
    }

    إذا (N.hasChildren()) {
      خپلکاری ماشوم_دننه_واټن = N.Children_دننه_واټن;
      لكل (;;) {
        غوټه C = read_غوټه(ماشوم_دننه_واټن, &N);
        ماشوم_دننه_واټن += C.اندازه;
        إذا (!C.isValid())
          اكسر;
        لیدنه_غوټه(C, لیکه, لیدنه_غوټه);
        إذا (!C.HasSibling)
          اكسر;
      }
    }
  };

  غوټه ریښه_2 = create_ریښه();
  لیدنه_غوټه(ریښه_2, 1, لیدنه_غوټه);
  أرجع Matches;
}

} // namespace unicode

} // namespace sys
} // namespace llvm
