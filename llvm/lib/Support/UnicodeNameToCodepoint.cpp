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
خارجي ثابت uint8_t *ثابت Unicode_Name_منزل_Codepoint_اشاریہ;
خارجي ثابت std::size_t Unicode_Name_منزل_Codepoint_اشاریہ_ناپ;
خارجي ثابت std::size_t Unicode_Name_منزل_Codepoint_سبھ_توں_وڈا_Name_ناپ;

ورتوں بفر_Type = SmallString<64>;

بنية گرہ {
  منطقي Is_جڑ = خطأ;
  char32_t قدر = 0xFFFFFFFF;
  uint32_t Children_وتھ = 0;
  منطقي HasSibling = خطأ;
  uint32_t ناپ = 0;
  StringRef Name;
  ثابت گرہ *ماپا_2 = خالی_سوچک;

  constexpr منطقي isValid() ثابت {
    أرجع !Name.خالی_تھاواں() || قدر == 0xFFFFFFFF;
  }
  constexpr منطقي hasChildren() ثابت { أرجع Children_وتھ != 0 || Is_جڑ; }

  std::string بھریا_Name() ثابت {
    std::string S;
    // Reserve enough space for most unicode code points.
    // The chosen value represent the 99th percentile of name size as of
    // Unicode 15.0.
    S.reserve(46);
    ثابت گرہ *N = هذا;
    طالما (N) {
      std::الٹ_copy(N->Name.begin(), N->Name.سرا(), std::پٹھ_inserter(S));
      N = N->ماپا_2;
    }
    std::الٹ(S.begin(), S.سرا());
    أرجع S;
  }
};

ساكن گرہ create_جڑ() {
  گرہ N;
  N.Is_جڑ = سچ;
  N.Children_وتھ = 1;
  N.ناپ = 1;
  أرجع N;
}

ساكن گرہ read_گرہ(uint32_t وتھ, ثابت گرہ *ماپا_2 = خالی_سوچک) {
  إذا (وتھ == 0)
    أرجع create_جڑ();

  uint32_t Origin = وتھ;
  گرہ N;
  N.ماپا_2 = ماپا_2;
  uint8_t NameInfo = Unicode_Name_منزل_Codepoint_اشاریہ[وتھ++];
  إذا (وتھ + 6 >= Unicode_Name_منزل_Codepoint_اشاریہ_ناپ)
    أرجع N;

  منطقي LongName = NameInfo & 0x40;
  منطقي Has_قدر = NameInfo & 0x80;
  std::size_t ناپ = NameInfo & ~0xC0;
  إذا (LongName) {
    uint32_t Name_وتھ = (Unicode_Name_منزل_Codepoint_اشاریہ[وتھ++] << 8);
    Name_وتھ |= Unicode_Name_منزل_Codepoint_اشاریہ[وتھ++];
    N.Name = StringRef(Unicode_Name_منزل_Codepoint_Dict + Name_وتھ, ناپ);
  } وإلا {
    N.Name = StringRef(Unicode_Name_منزل_Codepoint_Dict + ناپ, 1);
  }
  إذا (Has_قدر) {
    uint8_t H = Unicode_Name_منزل_Codepoint_اشاریہ[وتھ++];
    uint8_t M = Unicode_Name_منزل_Codepoint_اشاریہ[وتھ++];
    uint8_t L = Unicode_Name_منزل_Codepoint_اشاریہ[وتھ++];
    N.قدر = ((H << 16) | (M << 8) | L) >> 3;

    منطقي HasChildren = L & 0x02;
    N.HasSibling = L & 0x01;

    إذا (HasChildren) {
      N.Children_وتھ = Unicode_Name_منزل_Codepoint_اشاریہ[وتھ++] << 16;
      N.Children_وتھ |= Unicode_Name_منزل_Codepoint_اشاریہ[وتھ++] << 8;
      N.Children_وتھ |= Unicode_Name_منزل_Codepoint_اشاریہ[وتھ++];
    }
  } وإلا {
    uint8_t H = Unicode_Name_منزل_Codepoint_اشاریہ[وتھ++];
    N.HasSibling = H & 0x80;
    منطقي HasChildren = H & 0x40;
    H &= uint8_t(~0xC0);
    إذا (HasChildren) {
      N.Children_وتھ = (H << 16);
      N.Children_وتھ |=
          (uint32_t(Unicode_Name_منزل_Codepoint_اشاریہ[وتھ++]) << 8);
      N.Children_وتھ |= Unicode_Name_منزل_Codepoint_اشاریہ[وتھ++];
    }
  }
  N.ناپ = وتھ - Origin;
  أرجع N;
}

ساكن منطقي startsWith(StringRef Name, StringRef Needle, منطقي Strict,
                       std::size_t &Consummed, حرف &پچھلا_Char_وچ_Name,
                       منطقي Is_اگیتر = خطأ) {

  Consummed = 0;
  إذا (Strict) {
    إذا (!Name.starts_with(Needle))
      أرجع خطأ;
    Consummed = Needle.ناپ_2();
    أرجع سچ;
  }
  إذا (Needle.خالی_تھاواں())
    أرجع سچ;

  خودکار NamePos = Name.begin();
  خودکار NeedlePos = Needle.begin();

  حرف پچھلا_Char_وچ_Name_Origin = پچھلا_Char_وچ_Name;
  حرف پچھلا_Char_وچ_Needle = *Needle.begin();
  خودکار IgnoreSpaces = [](خودکار It, خودکار سرا_2, حرف &پچھلا_Char,
                         منطقي Is_اگیتر = خطأ) {
    طالما (It != سرا_2) {
      ثابت خودکار اگلا_2 = std::اگلا(It);
      // Ignore spaces, underscore, medial hyphens
      // The generator ensures a needle never ends (or starts) by a medial
      // hyphen https://unicode.org/reports/tr44/#UAX44-LM2.
      منطقي Ignore =
          *It == ' ' || *It == '_' ||
          (*It == '-' && isAlnum(پچھلا_Char) &&
           ((اگلا_2 != سرا_2 && isAlnum(*اگلا_2)) || (اگلا_2 == سرا_2 && Is_اگیتر)));
      پچھلا_Char = *It;
      إذا (!Ignore)
        اكسر;
      ++It;
    }
    أرجع It;
  };

  طالما (سچ) {
    NamePos = IgnoreSpaces(NamePos, Name.سرا(), پچھلا_Char_وچ_Name);
    NeedlePos =
        IgnoreSpaces(NeedlePos, Needle.سرا(), پچھلا_Char_وچ_Needle, Is_اگیتر);
    إذا (NeedlePos == Needle.سرا())
      اكسر;
    إذا (NamePos == Name.سرا())
      اكسر;
    إذا (منزل_اتلا(*NeedlePos) != منزل_اتلا(*NamePos))
      اكسر;
    NeedlePos++;
    NamePos++;
  }
  Consummed = std::فاصلہ(Name.begin(), NamePos);
  إذا (NeedlePos != Needle.سرا()) {
    پچھلا_Char_وچ_Name = پچھلا_Char_وچ_Name_Origin;
  }
  أرجع NeedlePos == Needle.سرا();
}

ساكن std::tuple<گرہ, منطقي, uint32_t>
compare_گرہ(uint32_t وتھ, StringRef Name, منطقي Strict,
            حرف پچھلا_Char_وچ_Name, بفر_Type &بفر,
            ثابت گرہ *ماپا_2 = خالی_سوچک) {
  گرہ N = read_گرہ(وتھ, ماپا_2);
  std::size_t Consummed = 0;
  منطقي Does_شروع_With = N.Is_جڑ || startsWith(Name, N.Name, Strict, Consummed,
                                              پچھلا_Char_وچ_Name);
  إذا (!Does_شروع_With)
    أرجع std::make_tuple(N, خطأ, 0);

  إذا (Name.ناپ_2() - Consummed == 0 && N.قدر != 0xFFFFFFFF)
    أرجع std::make_tuple(N, سچ, N.قدر);

  إذا (N.hasChildren()) {
    uint32_t بچہ_وتھ = N.Children_وتھ;
    لكل (;;) {
      گرہ C;
      منطقي Matches;
      uint32_t قدر;
      std::tie(C, Matches, قدر) =
          compare_گرہ(بچہ_وتھ, Name.substr(Consummed), Strict,
                      پچھلا_Char_وچ_Name, بفر, &N);
      إذا (Matches) {
        std::الٹ_copy(C.Name.begin(), C.Name.سرا(),
                          std::پٹھ_inserter(بفر));
        أرجع std::make_tuple(N, سچ, قدر);
      }
      بچہ_وتھ += C.ناپ;
      إذا (!C.HasSibling)
        اكسر;
    }
  }
  أرجع std::make_tuple(N, خطأ, 0);
}

ساكن std::tuple<گرہ, منطقي, uint32_t>
compare_گرہ(uint32_t وتھ, StringRef Name, منطقي Strict, بفر_Type &بفر) {
  أرجع compare_گرہ(وتھ, Name, Strict, 0, بفر);
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

ساكن std::size_t لبھو_Syllable(StringRef Name, منطقي Strict,
                                حرف &پچھلا_وچ_Name, صحيح &Pos, صحيح کالم) {
  أكد(کالم == 0 || کالم == 1 || کالم == 2);
  ساكن std::size_t گنتی_فی_کالم[] = {LCount, VCount, TCount};
  صحيح Len = -1;
  صحيح Prev = پچھلا_وچ_Name;
  لكل (std::size_t I = 0; I < گنتی_فی_کالم[کالم]; I++) {
    StringRef Syllable(HangulSyllables[I][کالم]);
    إذا (صحيح(Syllable.ناپ_2()) <= Len)
      استمر;
    std::size_t Consummed = 0;
    حرف پچھلا_وچ_Name_Copy = پچھلا_وچ_Name;
    منطقي Does_شروع_With =
        startsWith(Name, Syllable, Strict, Consummed, پچھلا_وچ_Name_Copy);
    إذا (!Does_شروع_With)
      استمر;
    Len = Consummed;
    Pos = I;
    Prev = پچھلا_وچ_Name_Copy;
  }
  إذا (Len == -1)
    أرجع 0;
  پچھلا_وچ_Name = Prev;
  أرجع size_t(Len);
}

ساكن std::optional<char32_t>
name_منزل_Hangul_کوڈ_نقطہ(StringRef Name, منطقي Strict, بفر_Type &بفر) {
  بفر.clear();
  // Hangul Syllable Decomposition
  std::size_t Consummed = 0;
  حرف Name_شروع = 0;
  منطقي Does_شروع_With =
      startsWith(Name, "HANGUL SYLLABLE ", Strict, Consummed, Name_شروع);
  إذا (!Does_شروع_With)
    أرجع std::nullopt;
  Name = Name.substr(Consummed);
  صحيح L = -1, V = -1, T = -1;
  Name = Name.substr(لبھو_Syllable(Name, Strict, Name_شروع, L, 0));
  Name = Name.substr(لبھو_Syllable(Name, Strict, Name_شروع, V, 1));
  Name = Name.substr(لبھو_Syllable(Name, Strict, Name_شروع, T, 2));
  إذا (L != -1 && V != -1 && T != -1 && Name.خالی_تھاواں()) {
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

بنية Generated_ناں_انکڑے {
  StringRef اگیتر;
  uint32_t شروع;
  uint32_t سرا_2;
};

// Unicode 18.0 Name Derivation Rule Prefix Strings (extracted/DerivedName.txt)
ساكن ثابت Generated_ناں_انکڑے Generated_ناں_انکڑے_جدول[] = {
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
name_منزل_Generated_کوڈ_نقطہ(StringRef Name, منطقي Strict, بفر_Type &بفر) {
  لكل (خودکار &&شے : Generated_ناں_انکڑے_جدول) {
    بفر.clear();
    std::size_t Consummed = 0;
    حرف Name_شروع = 0;
    منطقي Does_شروع_With = startsWith(Name, شے.اگیتر, Strict, Consummed,
                                    Name_شروع, /*IsPrefix=*/سچ);
    إذا (!Does_شروع_With)
      استمر;
    خودکار نمبر = Name.substr(Consummed);
    بنانشانی لما لما V = 0;
    // Be consistent about mandating upper casing.
    إذا (Strict &&
        llvm::any_دا(نمبر, [](حرف C) { أرجع C >= 'a' && C <= 'f'; }))
      أرجع {};
    إذا (getAsUnsignedInteger(نمبر, 16, V) || V < شے.شروع || V > شے.سرا_2)
      استمر;
    إذا (!Strict) {
      بفر.append(شے.اگیتر);
      بفر.append(utohexstr(V, سچ));
    }
    أرجع V;
  }
  أرجع std::nullopt;
}

ساكن std::optional<char32_t> name_منزل_Codepoint(StringRef Name, منطقي Strict,
                                               بفر_Type &بفر) {
  إذا (Name.خالی_تھاواں())
    أرجع std::nullopt;

  std::optional<char32_t> Res = name_منزل_Hangul_کوڈ_نقطہ(Name, Strict, بفر);
  إذا (!Res)
    Res = name_منزل_Generated_کوڈ_نقطہ(Name, Strict, بفر);
  إذا (Res)
    أرجع *Res;

  بفر.clear();
  گرہ گرہ;
  منطقي Matches;
  uint32_t قدر;
  std::tie(گرہ, Matches, قدر) = compare_گرہ(0, Name, Strict, بفر);
  إذا (Matches) {
    std::الٹ(بفر.begin(), بفر.سرا());
    // UAX44-LM2. Ignore case, whitespace, underscore ('_'), and all medial
    // hyphens except the hyphen in U+1180 HANGUL JUNGSEONG O-E.
    إذا (!Strict && قدر == 0x116c && Name.شامل_اے_insensitive("O-E")) {
      بفر = "HANGUL JUNGSEONG O-E";
      قدر = 0x1180;
    }
    أرجع قدر;
  }
  أرجع std::nullopt;
}

std::optional<char32_t> name_منزل_Codepoint_Strict(StringRef Name) {

  بفر_Type بفر;
  خودکار Opt = name_منزل_Codepoint(Name, سچ, بفر);
  أرجع Opt;
}

std::optional<Loose_Matching_نتیجہ>
name_منزل_Codepoint_Loose_Matching(StringRef Name) {
  بفر_Type بفر;
  خودکار Opt = name_منزل_Codepoint(Name, خطأ, بفر);
  إذا (!Opt)
    أرجع std::nullopt;
  أرجع Loose_Matching_نتیجہ{*Opt, بفر};
}

// Find the unicode character whose editing distance to Pattern
// is shortest, using the Wagner–Fischer algorithm.
llvm::Small_سمتیہ<میل_For_Codepoint_Name>
سبھ_توں_نیڑے_Matches_For_Codepoint_Name(StringRef نمونہ, std::size_t Max_Matches_گنتی) {
  // We maintain a fixed size vector of matches,
  // sorted by distance
  // The worst match (with the biggest distance) are discarded when new elements
  // are added.
  std::size_t سبھ_توں_وڈا_Edit_فاصلہ = 0;
  llvm::Small_سمتیہ<میل_For_Codepoint_Name> Matches;
  Matches.reserve(Max_Matches_گنتی + 1);

  خودکار داخل_کرو_2 = [&](ثابت گرہ &گرہ, uint32_t فاصلہ_2,
                    char32_t قدر) -> منطقي {
    إذا (فاصلہ_2 > سبھ_توں_وڈا_Edit_فاصلہ) {
      إذا (Matches.ناپ_2() == Max_Matches_گنتی)
        أرجع خطأ;
      سبھ_توں_وڈا_Edit_فاصلہ = فاصلہ_2;
    }
    // To avoid allocations, the creation of the name is delayed
    // as much as possible.
    std::string Name;
    خودکار GetName = [&] {
      إذا (Name.خالی_تھاواں())
        Name = گرہ.بھریا_Name();
      أرجع Name;
    };

    خودکار It = llvm::ہیٹھلا_bound(
        Matches, فاصلہ_2,
        [&](ثابت میل_For_Codepoint_Name &a, std::size_t فاصلہ_2) {
          إذا (فاصلہ_2 == a.فاصلہ_2)
            أرجع a.Name < GetName();
          أرجع a.فاصلہ_2 < فاصلہ_2;
        });
    إذا (It == Matches.سرا() && Matches.ناپ_2() == Max_Matches_گنتی)
      أرجع خطأ;

    میل_For_Codepoint_Name M{GetName(), فاصلہ_2, قدر};
    Matches.داخل_کرو(It, std::move(M));
    إذا (Matches.ناپ_2() > Max_Matches_گنتی)
      Matches.نکالو_پٹھ();
    أرجع سچ;
  };

  // We ignore case, space, hyphens, etc,
  // in both the search pattern and the prospective names.
  خودکار Normalize = [](StringRef Name) {
    std::string باہرلا;
    باہرلا.reserve(Name.ناپ_2());
    لكل (حرف C : Name) {
      إذا (isAlnum(C))
        باہرلا.ڈالو_پٹھ(منزل_اتلا(C));
    }
    أرجع باہرلا;
  };
  std::string NormalizedName = Normalize(نمونہ);

  // Allocate a matrix big enough for longest names.
  ثابت std::size_t Columns =
      std::min(NormalizedName.ناپ_2(), Unicode_Name_منزل_Codepoint_سبھ_توں_وڈا_Name_ناپ) +
      1;

  [[maybe_بن_ورتیا]] ساكن std::size_t صفاں_2 =
      Unicode_Name_منزل_Codepoint_سبھ_توں_وڈا_Name_ناپ + 1;

  std::سمتیہ<حرف> Distances(
      Columns * (Unicode_Name_منزل_Codepoint_سبھ_توں_وڈا_Name_ناپ + 1), 0);

  خودکار Get = [&Distances, Columns](size_t کالم, std::size_t صف) -> حرف & {
    أكد(کالم < Columns);
    أكد(صف < صفاں_2);
    أرجع Distances[صف * Columns + کالم];
  };

  لكل (std::size_t I = 0; I < Columns; I++)
    Get(I, 0) = I;

  // Visit the childrens,
  // Filling (and overriding) the matrix for the name fragment of each node
  // iteratively. CompleteName is used to collect the actual name of potential
  // match, respecting case and spacing.
  خودکار ویکھو_گرہ = [&](ثابت گرہ &N, std::size_t صف,
                       خودکار &ویکھو_گرہ) -> فراغ {
    std::size_t J = 0;
    لكل (; J < N.Name.ناپ_2(); J++) {
      إذا (!isAlnum(N.Name[J]))
        استمر;

      Get(0, صف) = صف;

      لكل (std::size_t I = 1; I < Columns; I++) {
        ثابت صحيح Delete = Get(I - 1, صف) + 1;
        ثابت صحيح داخل_کرو_2 = Get(I, صف - 1) + 1;

        ثابت صحيح Replace =
            Get(I - 1, صف - 1) + (NormalizedName[I - 1] != N.Name[J] ? 1 : 0);

        Get(I, صف) = std::min({داخل_کرو_2, Delete, Replace});
      }

      صف++;
    }

    بنانشانی لاگت = Get(Columns - 1, صف - 1);
    إذا (N.قدر != 0xFFFFFFFF) {
      داخل_کرو_2(N, لاگت, N.قدر);
    }

    إذا (N.hasChildren()) {
      خودکار بچہ_وتھ = N.Children_وتھ;
      لكل (;;) {
        گرہ C = read_گرہ(بچہ_وتھ, &N);
        بچہ_وتھ += C.ناپ;
        إذا (!C.isValid())
          اكسر;
        ویکھو_گرہ(C, صف, ویکھو_گرہ);
        إذا (!C.HasSibling)
          اكسر;
      }
    }
  };

  گرہ جڑ_2 = create_جڑ();
  ویکھو_گرہ(جڑ_2, 1, ویکھو_گرہ);
  أرجع Matches;
}

} // namespace unicode

} // namespace sys
} // namespace llvm
