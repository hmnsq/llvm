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
خارجي ثابت uint8_t *ثابت Unicode_Name_منزل_Codepoint_اشاريو;
خارجي ثابت std::size_t Unicode_Name_منزل_Codepoint_اشاريو_سائيز;
خارجي ثابت std::size_t Unicode_Name_منزل_Codepoint_سڀ_کان_وڏو_Name_سائيز;

استعمال بفر_Type = SmallString<64>;

بنية ڳنڍ {
  منطقي Is_پاڙ = خطأ;
  char32_t قدر = 0xFFFFFFFF;
  uint32_t Children_وٿي = 0;
  منطقي HasSibling = خطأ;
  uint32_t سائيز = 0;
  StringRef Name;
  ثابت ڳنڍ *والد_2 = خالي_اشارو;

  constexpr منطقي isValid() ثابت {
    أرجع !Name.خالي_جايون() || قدر == 0xFFFFFFFF;
  }
  constexpr منطقي hasChildren() ثابت { أرجع Children_وٿي != 0 || Is_پاڙ; }

  std::string ڀريل_Name() ثابت {
    std::string S;
    // Reserve enough space for most unicode code points.
    // The chosen value represent the 99th percentile of name size as of
    // Unicode 15.0.
    S.reserve(46);
    ثابت ڳنڍ *N = هذا;
    طالما (N) {
      std::ابتو_copy(N->Name.begin(), N->Name.پڇاڙي(), std::پٺ_inserter(S));
      N = N->والد_2;
    }
    std::ابتو(S.begin(), S.پڇاڙي());
    أرجع S;
  }
};

ساكن ڳنڍ create_پاڙ() {
  ڳنڍ N;
  N.Is_پاڙ = سچ;
  N.Children_وٿي = 1;
  N.سائيز = 1;
  أرجع N;
}

ساكن ڳنڍ read_ڳنڍ(uint32_t وٿي, ثابت ڳنڍ *والد_2 = خالي_اشارو) {
  إذا (وٿي == 0)
    أرجع create_پاڙ();

  uint32_t Origin = وٿي;
  ڳنڍ N;
  N.والد_2 = والد_2;
  uint8_t NameInfo = Unicode_Name_منزل_Codepoint_اشاريو[وٿي++];
  إذا (وٿي + 6 >= Unicode_Name_منزل_Codepoint_اشاريو_سائيز)
    أرجع N;

  منطقي LongName = NameInfo & 0x40;
  منطقي Has_قدر = NameInfo & 0x80;
  std::size_t سائيز = NameInfo & ~0xC0;
  إذا (LongName) {
    uint32_t Name_وٿي = (Unicode_Name_منزل_Codepoint_اشاريو[وٿي++] << 8);
    Name_وٿي |= Unicode_Name_منزل_Codepoint_اشاريو[وٿي++];
    N.Name = StringRef(Unicode_Name_منزل_Codepoint_Dict + Name_وٿي, سائيز);
  } وإلا {
    N.Name = StringRef(Unicode_Name_منزل_Codepoint_Dict + سائيز, 1);
  }
  إذا (Has_قدر) {
    uint8_t H = Unicode_Name_منزل_Codepoint_اشاريو[وٿي++];
    uint8_t M = Unicode_Name_منزل_Codepoint_اشاريو[وٿي++];
    uint8_t L = Unicode_Name_منزل_Codepoint_اشاريو[وٿي++];
    N.قدر = ((H << 16) | (M << 8) | L) >> 3;

    منطقي HasChildren = L & 0x02;
    N.HasSibling = L & 0x01;

    إذا (HasChildren) {
      N.Children_وٿي = Unicode_Name_منزل_Codepoint_اشاريو[وٿي++] << 16;
      N.Children_وٿي |= Unicode_Name_منزل_Codepoint_اشاريو[وٿي++] << 8;
      N.Children_وٿي |= Unicode_Name_منزل_Codepoint_اشاريو[وٿي++];
    }
  } وإلا {
    uint8_t H = Unicode_Name_منزل_Codepoint_اشاريو[وٿي++];
    N.HasSibling = H & 0x80;
    منطقي HasChildren = H & 0x40;
    H &= uint8_t(~0xC0);
    إذا (HasChildren) {
      N.Children_وٿي = (H << 16);
      N.Children_وٿي |=
          (uint32_t(Unicode_Name_منزل_Codepoint_اشاريو[وٿي++]) << 8);
      N.Children_وٿي |= Unicode_Name_منزل_Codepoint_اشاريو[وٿي++];
    }
  }
  N.سائيز = وٿي - Origin;
  أرجع N;
}

ساكن منطقي startsWith(StringRef Name, StringRef Needle, منطقي Strict,
                       std::size_t &Consummed, حرف &اڳيون_Char_اندريون_Name,
                       منطقي Is_اڳياڙي = خطأ) {

  Consummed = 0;
  إذا (Strict) {
    إذا (!Name.starts_with(Needle))
      أرجع خطأ;
    Consummed = Needle.سائيز_2();
    أرجع سچ;
  }
  إذا (Needle.خالي_جايون())
    أرجع سچ;

  خودڪار NamePos = Name.begin();
  خودڪار NeedlePos = Needle.begin();

  حرف اڳيون_Char_اندريون_Name_Origin = اڳيون_Char_اندريون_Name;
  حرف اڳيون_Char_اندريون_Needle = *Needle.begin();
  خودڪار IgnoreSpaces = [](خودڪار It, خودڪار پڇاڙي_2, حرف &اڳيون_Char,
                         منطقي Is_اڳياڙي = خطأ) {
    طالما (It != پڇاڙي_2) {
      ثابت خودڪار ايندڙ_2 = std::ايندڙ(It);
      // Ignore spaces, underscore, medial hyphens
      // The generator ensures a needle never ends (or starts) by a medial
      // hyphen https://unicode.org/reports/tr44/#UAX44-LM2.
      منطقي Ignore =
          *It == ' ' || *It == '_' ||
          (*It == '-' && isAlnum(اڳيون_Char) &&
           ((ايندڙ_2 != پڇاڙي_2 && isAlnum(*ايندڙ_2)) || (ايندڙ_2 == پڇاڙي_2 && Is_اڳياڙي)));
      اڳيون_Char = *It;
      إذا (!Ignore)
        اكسر;
      ++It;
    }
    أرجع It;
  };

  طالما (سچ) {
    NamePos = IgnoreSpaces(NamePos, Name.پڇاڙي(), اڳيون_Char_اندريون_Name);
    NeedlePos =
        IgnoreSpaces(NeedlePos, Needle.پڇاڙي(), اڳيون_Char_اندريون_Needle, Is_اڳياڙي);
    إذا (NeedlePos == Needle.پڇاڙي())
      اكسر;
    إذا (NamePos == Name.پڇاڙي())
      اكسر;
    إذا (منزل_مٿانهون(*NeedlePos) != منزل_مٿانهون(*NamePos))
      اكسر;
    NeedlePos++;
    NamePos++;
  }
  Consummed = std::مفاصلو(Name.begin(), NamePos);
  إذا (NeedlePos != Needle.پڇاڙي()) {
    اڳيون_Char_اندريون_Name = اڳيون_Char_اندريون_Name_Origin;
  }
  أرجع NeedlePos == Needle.پڇاڙي();
}

ساكن std::tuple<ڳنڍ, منطقي, uint32_t>
compare_ڳنڍ(uint32_t وٿي, StringRef Name, منطقي Strict,
            حرف اڳيون_Char_اندريون_Name, بفر_Type &بفر,
            ثابت ڳنڍ *والد_2 = خالي_اشارو) {
  ڳنڍ N = read_ڳنڍ(وٿي, والد_2);
  std::size_t Consummed = 0;
  منطقي Does_شروعات_With = N.Is_پاڙ || startsWith(Name, N.Name, Strict, Consummed,
                                              اڳيون_Char_اندريون_Name);
  إذا (!Does_شروعات_With)
    أرجع std::make_tuple(N, خطأ, 0);

  إذا (Name.سائيز_2() - Consummed == 0 && N.قدر != 0xFFFFFFFF)
    أرجع std::make_tuple(N, سچ, N.قدر);

  إذا (N.hasChildren()) {
    uint32_t ٻار_وٿي = N.Children_وٿي;
    لكل (;;) {
      ڳنڍ C;
      منطقي Matches;
      uint32_t قدر;
      std::tie(C, Matches, قدر) =
          compare_ڳنڍ(ٻار_وٿي, Name.substr(Consummed), Strict,
                      اڳيون_Char_اندريون_Name, بفر, &N);
      إذا (Matches) {
        std::ابتو_copy(C.Name.begin(), C.Name.پڇاڙي(),
                          std::پٺ_inserter(بفر));
        أرجع std::make_tuple(N, سچ, قدر);
      }
      ٻار_وٿي += C.سائيز;
      إذا (!C.HasSibling)
        اكسر;
    }
  }
  أرجع std::make_tuple(N, خطأ, 0);
}

ساكن std::tuple<ڳنڍ, منطقي, uint32_t>
compare_ڳنڍ(uint32_t وٿي, StringRef Name, منطقي Strict, بفر_Type &بفر) {
  أرجع compare_ڳنڍ(وٿي, Name, Strict, 0, بفر);
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

ساكن std::size_t ڳوليو_Syllable(StringRef Name, منطقي Strict,
                                حرف &اڳيون_اندريون_Name, صحيح &Pos, صحيح ڪالم) {
  أكد(ڪالم == 0 || ڪالم == 1 || ڪالم == 2);
  ساكن std::size_t ڳڻپ_في_ڪالم[] = {LCount, VCount, TCount};
  صحيح Len = -1;
  صحيح Prev = اڳيون_اندريون_Name;
  لكل (std::size_t I = 0; I < ڳڻپ_في_ڪالم[ڪالم]; I++) {
    StringRef Syllable(HangulSyllables[I][ڪالم]);
    إذا (صحيح(Syllable.سائيز_2()) <= Len)
      استمر;
    std::size_t Consummed = 0;
    حرف اڳيون_اندريون_Name_Copy = اڳيون_اندريون_Name;
    منطقي Does_شروعات_With =
        startsWith(Name, Syllable, Strict, Consummed, اڳيون_اندريون_Name_Copy);
    إذا (!Does_شروعات_With)
      استمر;
    Len = Consummed;
    Pos = I;
    Prev = اڳيون_اندريون_Name_Copy;
  }
  إذا (Len == -1)
    أرجع 0;
  اڳيون_اندريون_Name = Prev;
  أرجع size_t(Len);
}

ساكن std::optional<char32_t>
name_منزل_Hangul_ڪوڊ_نقطو(StringRef Name, منطقي Strict, بفر_Type &بفر) {
  بفر.clear();
  // Hangul Syllable Decomposition
  std::size_t Consummed = 0;
  حرف Name_شروعات = 0;
  منطقي Does_شروعات_With =
      startsWith(Name, "HANGUL SYLLABLE ", Strict, Consummed, Name_شروعات);
  إذا (!Does_شروعات_With)
    أرجع std::nullopt;
  Name = Name.substr(Consummed);
  صحيح L = -1, V = -1, T = -1;
  Name = Name.substr(ڳوليو_Syllable(Name, Strict, Name_شروعات, L, 0));
  Name = Name.substr(ڳوليو_Syllable(Name, Strict, Name_شروعات, V, 1));
  Name = Name.substr(ڳوليو_Syllable(Name, Strict, Name_شروعات, T, 2));
  إذا (L != -1 && V != -1 && T != -1 && Name.خالي_جايون()) {
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

بنية Generated_نالا_معلومات {
  StringRef اڳياڙي;
  uint32_t شروعات;
  uint32_t پڇاڙي_2;
};

// Unicode 18.0 Name Derivation Rule Prefix Strings (extracted/DerivedName.txt)
ساكن ثابت Generated_نالا_معلومات Generated_نالا_معلومات_جدول[] = {
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
name_منزل_Generated_ڪوڊ_نقطو(StringRef Name, منطقي Strict, بفر_Type &بفر) {
  لكل (خودڪار &&شي : Generated_نالا_معلومات_جدول) {
    بفر.clear();
    std::size_t Consummed = 0;
    حرف Name_شروعات = 0;
    منطقي Does_شروعات_With = startsWith(Name, شي.اڳياڙي, Strict, Consummed,
                                    Name_شروعات, /*IsPrefix=*/سچ);
    إذا (!Does_شروعات_With)
      استمر;
    خودڪار عدد = Name.substr(Consummed);
    بنانشان ڊگهو ڊگهو V = 0;
    // Be consistent about mandating upper casing.
    إذا (Strict &&
        llvm::any_جو(عدد, [](حرف C) { أرجع C >= 'a' && C <= 'f'; }))
      أرجع {};
    إذا (getAsUnsignedInteger(عدد, 16, V) || V < شي.شروعات || V > شي.پڇاڙي_2)
      استمر;
    إذا (!Strict) {
      بفر.append(شي.اڳياڙي);
      بفر.append(utohexstr(V, سچ));
    }
    أرجع V;
  }
  أرجع std::nullopt;
}

ساكن std::optional<char32_t> name_منزل_Codepoint(StringRef Name, منطقي Strict,
                                               بفر_Type &بفر) {
  إذا (Name.خالي_جايون())
    أرجع std::nullopt;

  std::optional<char32_t> Res = name_منزل_Hangul_ڪوڊ_نقطو(Name, Strict, بفر);
  إذا (!Res)
    Res = name_منزل_Generated_ڪوڊ_نقطو(Name, Strict, بفر);
  إذا (Res)
    أرجع *Res;

  بفر.clear();
  ڳنڍ ڳنڍ;
  منطقي Matches;
  uint32_t قدر;
  std::tie(ڳنڍ, Matches, قدر) = compare_ڳنڍ(0, Name, Strict, بفر);
  إذا (Matches) {
    std::ابتو(بفر.begin(), بفر.پڇاڙي());
    // UAX44-LM2. Ignore case, whitespace, underscore ('_'), and all medial
    // hyphens except the hyphen in U+1180 HANGUL JUNGSEONG O-E.
    إذا (!Strict && قدر == 0x116c && Name.شامل_آهي_insensitive("O-E")) {
      بفر = "HANGUL JUNGSEONG O-E";
      قدر = 0x1180;
    }
    أرجع قدر;
  }
  أرجع std::nullopt;
}

std::optional<char32_t> name_منزل_Codepoint_Strict(StringRef Name) {

  بفر_Type بفر;
  خودڪار Opt = name_منزل_Codepoint(Name, سچ, بفر);
  أرجع Opt;
}

std::optional<Loose_Matching_نتيجو>
name_منزل_Codepoint_Loose_Matching(StringRef Name) {
  بفر_Type بفر;
  خودڪار Opt = name_منزل_Codepoint(Name, خطأ, بفر);
  إذا (!Opt)
    أرجع std::nullopt;
  أرجع Loose_Matching_نتيجو{*Opt, بفر};
}

// Find the unicode character whose editing distance to Pattern
// is shortest, using the Wagner–Fischer algorithm.
llvm::Small_ويڪٽر<ميلاپ_For_Codepoint_Name>
سڀ_کان_ويجهو_Matches_For_Codepoint_Name(StringRef نمونو, std::size_t Max_Matches_ڳڻپ) {
  // We maintain a fixed size vector of matches,
  // sorted by distance
  // The worst match (with the biggest distance) are discarded when new elements
  // are added.
  std::size_t سڀ_کان_وڏو_Edit_مفاصلو = 0;
  llvm::Small_ويڪٽر<ميلاپ_For_Codepoint_Name> Matches;
  Matches.reserve(Max_Matches_ڳڻپ + 1);

  خودڪار وجهو_2 = [&](ثابت ڳنڍ &ڳنڍ, uint32_t مفاصلو_2,
                    char32_t قدر) -> منطقي {
    إذا (مفاصلو_2 > سڀ_کان_وڏو_Edit_مفاصلو) {
      إذا (Matches.سائيز_2() == Max_Matches_ڳڻپ)
        أرجع خطأ;
      سڀ_کان_وڏو_Edit_مفاصلو = مفاصلو_2;
    }
    // To avoid allocations, the creation of the name is delayed
    // as much as possible.
    std::string Name;
    خودڪار GetName = [&] {
      إذا (Name.خالي_جايون())
        Name = ڳنڍ.ڀريل_Name();
      أرجع Name;
    };

    خودڪار It = llvm::هيٺاهون_bound(
        Matches, مفاصلو_2,
        [&](ثابت ميلاپ_For_Codepoint_Name &a, std::size_t مفاصلو_2) {
          إذا (مفاصلو_2 == a.مفاصلو_2)
            أرجع a.Name < GetName();
          أرجع a.مفاصلو_2 < مفاصلو_2;
        });
    إذا (It == Matches.پڇاڙي() && Matches.سائيز_2() == Max_Matches_ڳڻپ)
      أرجع خطأ;

    ميلاپ_For_Codepoint_Name M{GetName(), مفاصلو_2, قدر};
    Matches.وجهو(It, std::move(M));
    إذا (Matches.سائيز_2() > Max_Matches_ڳڻپ)
      Matches.ڪڍو_پٺ();
    أرجع سچ;
  };

  // We ignore case, space, hyphens, etc,
  // in both the search pattern and the prospective names.
  خودڪار Normalize = [](StringRef Name) {
    std::string ٻاهريون;
    ٻاهريون.reserve(Name.سائيز_2());
    لكل (حرف C : Name) {
      إذا (isAlnum(C))
        ٻاهريون.ڌڪو_پٺ(منزل_مٿانهون(C));
    }
    أرجع ٻاهريون;
  };
  std::string NormalizedName = Normalize(نمونو);

  // Allocate a matrix big enough for longest names.
  ثابت std::size_t Columns =
      std::min(NormalizedName.سائيز_2(), Unicode_Name_منزل_Codepoint_سڀ_کان_وڏو_Name_سائيز) +
      1;

  [[maybe_اڻ_استعمال_ٿيل]] ساكن std::size_t صفون_2 =
      Unicode_Name_منزل_Codepoint_سڀ_کان_وڏو_Name_سائيز + 1;

  std::ويڪٽر<حرف> Distances(
      Columns * (Unicode_Name_منزل_Codepoint_سڀ_کان_وڏو_Name_سائيز + 1), 0);

  خودڪار Get = [&Distances, Columns](size_t ڪالم, std::size_t صف) -> حرف & {
    أكد(ڪالم < Columns);
    أكد(صف < صفون_2);
    أرجع Distances[صف * Columns + ڪالم];
  };

  لكل (std::size_t I = 0; I < Columns; I++)
    Get(I, 0) = I;

  // Visit the childrens,
  // Filling (and overriding) the matrix for the name fragment of each node
  // iteratively. CompleteName is used to collect the actual name of potential
  // match, respecting case and spacing.
  خودڪار گهمو_ڳنڍ = [&](ثابت ڳنڍ &N, std::size_t صف,
                       خودڪار &گهمو_ڳنڍ) -> فراغ {
    std::size_t J = 0;
    لكل (; J < N.Name.سائيز_2(); J++) {
      إذا (!isAlnum(N.Name[J]))
        استمر;

      Get(0, صف) = صف;

      لكل (std::size_t I = 1; I < Columns; I++) {
        ثابت صحيح Delete = Get(I - 1, صف) + 1;
        ثابت صحيح وجهو_2 = Get(I, صف - 1) + 1;

        ثابت صحيح Replace =
            Get(I - 1, صف - 1) + (NormalizedName[I - 1] != N.Name[J] ? 1 : 0);

        Get(I, صف) = std::min({وجهو_2, Delete, Replace});
      }

      صف++;
    }

    بنانشان لاڳت = Get(Columns - 1, صف - 1);
    إذا (N.قدر != 0xFFFFFFFF) {
      وجهو_2(N, لاڳت, N.قدر);
    }

    إذا (N.hasChildren()) {
      خودڪار ٻار_وٿي = N.Children_وٿي;
      لكل (;;) {
        ڳنڍ C = read_ڳنڍ(ٻار_وٿي, &N);
        ٻار_وٿي += C.سائيز;
        إذا (!C.isValid())
          اكسر;
        گهمو_ڳنڍ(C, صف, گهمو_ڳنڍ);
        إذا (!C.HasSibling)
          اكسر;
      }
    }
  };

  ڳنڍ پاڙ_2 = create_پاڙ();
  گهمو_ڳنڍ(پاڙ_2, 1, گهمو_ڳنڍ);
  أرجع Matches;
}

} // namespace unicode

} // namespace sys
} // namespace llvm
