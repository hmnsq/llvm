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

خارجي ثابت حرف *ثابت Unicode_Name_مقصد_Codepoint_Dict;
خارجي ثابت uint8_t *ثابت Unicode_Name_مقصد_Codepoint_شاخص;
خارجي ثابت std::size_t Unicode_Name_مقصد_Codepoint_شاخص_اندازه;
خارجي ثابت std::size_t Unicode_Name_مقصد_Codepoint_بزرگترین_Name_اندازه;

استفاده میانگیر_Type = SmallString<64>;

بنية گره {
  منطقي Is_ریشه = خطأ;
  char32_t مقدار = 0xFFFFFFFF;
  uint32_t Children_جابه_جایی = 0;
  منطقي HasSibling = خطأ;
  uint32_t اندازه = 0;
  StringRef Name;
  ثابت گره *والد_2 = اشاره_تهی;

  constexpr منطقي isValid() ثابت {
    أرجع !Name.خالی() || مقدار == 0xFFFFFFFF;
  }
  constexpr منطقي hasChildren() ثابت { أرجع Children_جابه_جایی != 0 || Is_ریشه; }

  std::string پر_Name() ثابت {
    std::string S;
    // Reserve enough space for most unicode code points.
    // The chosen value represent the 99th percentile of name size as of
    // Unicode 15.0.
    S.reserve(46);
    ثابت گره *N = هذا;
    طالما (N) {
      std::معکوس_copy(N->Name.begin(), N->Name.انتها(), std::پشت_inserter(S));
      N = N->والد_2;
    }
    std::معکوس(S.begin(), S.انتها());
    أرجع S;
  }
};

ساكن گره create_ریشه() {
  گره N;
  N.Is_ریشه = درست;
  N.Children_جابه_جایی = 1;
  N.اندازه = 1;
  أرجع N;
}

ساكن گره read_گره(uint32_t جابه_جایی, ثابت گره *والد_2 = اشاره_تهی) {
  إذا (جابه_جایی == 0)
    أرجع create_ریشه();

  uint32_t Origin = جابه_جایی;
  گره N;
  N.والد_2 = والد_2;
  uint8_t NameInfo = Unicode_Name_مقصد_Codepoint_شاخص[جابه_جایی++];
  إذا (جابه_جایی + 6 >= Unicode_Name_مقصد_Codepoint_شاخص_اندازه)
    أرجع N;

  منطقي LongName = NameInfo & 0x40;
  منطقي Has_مقدار = NameInfo & 0x80;
  std::size_t اندازه = NameInfo & ~0xC0;
  إذا (LongName) {
    uint32_t Name_جابه_جایی = (Unicode_Name_مقصد_Codepoint_شاخص[جابه_جایی++] << 8);
    Name_جابه_جایی |= Unicode_Name_مقصد_Codepoint_شاخص[جابه_جایی++];
    N.Name = StringRef(Unicode_Name_مقصد_Codepoint_Dict + Name_جابه_جایی, اندازه);
  } وإلا {
    N.Name = StringRef(Unicode_Name_مقصد_Codepoint_Dict + اندازه, 1);
  }
  إذا (Has_مقدار) {
    uint8_t H = Unicode_Name_مقصد_Codepoint_شاخص[جابه_جایی++];
    uint8_t M = Unicode_Name_مقصد_Codepoint_شاخص[جابه_جایی++];
    uint8_t L = Unicode_Name_مقصد_Codepoint_شاخص[جابه_جایی++];
    N.مقدار = ((H << 16) | (M << 8) | L) >> 3;

    منطقي HasChildren = L & 0x02;
    N.HasSibling = L & 0x01;

    إذا (HasChildren) {
      N.Children_جابه_جایی = Unicode_Name_مقصد_Codepoint_شاخص[جابه_جایی++] << 16;
      N.Children_جابه_جایی |= Unicode_Name_مقصد_Codepoint_شاخص[جابه_جایی++] << 8;
      N.Children_جابه_جایی |= Unicode_Name_مقصد_Codepoint_شاخص[جابه_جایی++];
    }
  } وإلا {
    uint8_t H = Unicode_Name_مقصد_Codepoint_شاخص[جابه_جایی++];
    N.HasSibling = H & 0x80;
    منطقي HasChildren = H & 0x40;
    H &= uint8_t(~0xC0);
    إذا (HasChildren) {
      N.Children_جابه_جایی = (H << 16);
      N.Children_جابه_جایی |=
          (uint32_t(Unicode_Name_مقصد_Codepoint_شاخص[جابه_جایی++]) << 8);
      N.Children_جابه_جایی |= Unicode_Name_مقصد_Codepoint_شاخص[جابه_جایی++];
    }
  }
  N.اندازه = جابه_جایی - Origin;
  أرجع N;
}

ساكن منطقي startsWith(StringRef Name, StringRef Needle, منطقي Strict,
                       std::size_t &Consummed, حرف &قبلی_Char_درون_Name,
                       منطقي Is_پیشوند = خطأ) {

  Consummed = 0;
  إذا (Strict) {
    إذا (!Name.starts_with(Needle))
      أرجع خطأ;
    Consummed = Needle.اندازه_2();
    أرجع درست;
  }
  إذا (Needle.خالی())
    أرجع درست;

  خودکار NamePos = Name.begin();
  خودکار NeedlePos = Needle.begin();

  حرف قبلی_Char_درون_Name_Origin = قبلی_Char_درون_Name;
  حرف قبلی_Char_درون_Needle = *Needle.begin();
  خودکار IgnoreSpaces = [](خودکار It, خودکار انتها_2, حرف &قبلی_Char,
                         منطقي Is_پیشوند = خطأ) {
    طالما (It != انتها_2) {
      ثابت خودکار بعدی_2 = std::بعدی(It);
      // Ignore spaces, underscore, medial hyphens
      // The generator ensures a needle never ends (or starts) by a medial
      // hyphen https://unicode.org/reports/tr44/#UAX44-LM2.
      منطقي Ignore =
          *It == ' ' || *It == '_' ||
          (*It == '-' && isAlnum(قبلی_Char) &&
           ((بعدی_2 != انتها_2 && isAlnum(*بعدی_2)) || (بعدی_2 == انتها_2 && Is_پیشوند)));
      قبلی_Char = *It;
      إذا (!Ignore)
        اكسر;
      ++It;
    }
    أرجع It;
  };

  طالما (درست) {
    NamePos = IgnoreSpaces(NamePos, Name.انتها(), قبلی_Char_درون_Name);
    NeedlePos =
        IgnoreSpaces(NeedlePos, Needle.انتها(), قبلی_Char_درون_Needle, Is_پیشوند);
    إذا (NeedlePos == Needle.انتها())
      اكسر;
    إذا (NamePos == Name.انتها())
      اكسر;
    إذا (مقصد_فوقانی(*NeedlePos) != مقصد_فوقانی(*NamePos))
      اكسر;
    NeedlePos++;
    NamePos++;
  }
  Consummed = std::فاصله(Name.begin(), NamePos);
  إذا (NeedlePos != Needle.انتها()) {
    قبلی_Char_درون_Name = قبلی_Char_درون_Name_Origin;
  }
  أرجع NeedlePos == Needle.انتها();
}

ساكن std::tuple<گره, منطقي, uint32_t>
compare_گره(uint32_t جابه_جایی, StringRef Name, منطقي Strict,
            حرف قبلی_Char_درون_Name, میانگیر_Type &میانگیر,
            ثابت گره *والد_2 = اشاره_تهی) {
  گره N = read_گره(جابه_جایی, والد_2);
  std::size_t Consummed = 0;
  منطقي Does_شروع_With = N.Is_ریشه || startsWith(Name, N.Name, Strict, Consummed,
                                              قبلی_Char_درون_Name);
  إذا (!Does_شروع_With)
    أرجع std::make_tuple(N, خطأ, 0);

  إذا (Name.اندازه_2() - Consummed == 0 && N.مقدار != 0xFFFFFFFF)
    أرجع std::make_tuple(N, درست, N.مقدار);

  إذا (N.hasChildren()) {
    uint32_t فرزند_جابه_جایی = N.Children_جابه_جایی;
    لكل (;;) {
      گره C;
      منطقي Matches;
      uint32_t مقدار;
      std::tie(C, Matches, مقدار) =
          compare_گره(فرزند_جابه_جایی, Name.substr(Consummed), Strict,
                      قبلی_Char_درون_Name, میانگیر, &N);
      إذا (Matches) {
        std::معکوس_copy(C.Name.begin(), C.Name.انتها(),
                          std::پشت_inserter(میانگیر));
        أرجع std::make_tuple(N, درست, مقدار);
      }
      فرزند_جابه_جایی += C.اندازه;
      إذا (!C.HasSibling)
        اكسر;
    }
  }
  أرجع std::make_tuple(N, خطأ, 0);
}

ساكن std::tuple<گره, منطقي, uint32_t>
compare_گره(uint32_t جابه_جایی, StringRef Name, منطقي Strict, میانگیر_Type &میانگیر) {
  أرجع compare_گره(جابه_جایی, Name, Strict, 0, میانگیر);
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

ساكن std::size_t پیدا_کن_Syllable(StringRef Name, منطقي Strict,
                                حرف &قبلی_درون_Name, صحيح &Pos, صحيح ستونی) {
  أكد(ستونی == 0 || ستونی == 1 || ستونی == 2);
  ساكن std::size_t شمار_در_هر_ستونی[] = {LCount, VCount, TCount};
  صحيح Len = -1;
  صحيح Prev = قبلی_درون_Name;
  لكل (std::size_t I = 0; I < شمار_در_هر_ستونی[ستونی]; I++) {
    StringRef Syllable(HangulSyllables[I][ستونی]);
    إذا (صحيح(Syllable.اندازه_2()) <= Len)
      استمر;
    std::size_t Consummed = 0;
    حرف قبلی_درون_Name_Copy = قبلی_درون_Name;
    منطقي Does_شروع_With =
        startsWith(Name, Syllable, Strict, Consummed, قبلی_درون_Name_Copy);
    إذا (!Does_شروع_With)
      استمر;
    Len = Consummed;
    Pos = I;
    Prev = قبلی_درون_Name_Copy;
  }
  إذا (Len == -1)
    أرجع 0;
  قبلی_درون_Name = Prev;
  أرجع size_t(Len);
}

ساكن std::optional<char32_t>
name_مقصد_Hangul_کد_نقطه(StringRef Name, منطقي Strict, میانگیر_Type &میانگیر) {
  میانگیر.clear();
  // Hangul Syllable Decomposition
  std::size_t Consummed = 0;
  حرف Name_شروع = 0;
  منطقي Does_شروع_With =
      startsWith(Name, "HANGUL SYLLABLE ", Strict, Consummed, Name_شروع);
  إذا (!Does_شروع_With)
    أرجع std::nullopt;
  Name = Name.substr(Consummed);
  صحيح L = -1, V = -1, T = -1;
  Name = Name.substr(پیدا_کن_Syllable(Name, Strict, Name_شروع, L, 0));
  Name = Name.substr(پیدا_کن_Syllable(Name, Strict, Name_شروع, V, 1));
  Name = Name.substr(پیدا_کن_Syllable(Name, Strict, Name_شروع, T, 2));
  إذا (L != -1 && V != -1 && T != -1 && Name.خالی()) {
    إذا (!Strict) {
      میانگیر.append("HANGUL SYLLABLE ");
      إذا (L != -1)
        میانگیر.append(HangulSyllables[L][0]);
      إذا (V != -1)
        میانگیر.append(HangulSyllables[V][1]);
      إذا (T != -1)
        میانگیر.append(HangulSyllables[T][2]);
    }
    أرجع SBase + (std::uint32_t(L) * VCount + std::uint32_t(V)) * TCount +
           std::uint32_t(T);
  }
  // Otherwise, it's an illegal syllable name.
  أرجع std::nullopt;
}

بنية Generated_نامها_داده {
  StringRef پیشوند;
  uint32_t شروع;
  uint32_t انتها_2;
};

// Unicode 18.0 Name Derivation Rule Prefix Strings (extracted/DerivedName.txt)
ساكن ثابت Generated_نامها_داده Generated_نامها_داده_جدول[] = {
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
name_مقصد_Generated_کد_نقطه(StringRef Name, منطقي Strict, میانگیر_Type &میانگیر) {
  لكل (خودکار &&قلم : Generated_نامها_داده_جدول) {
    میانگیر.clear();
    std::size_t Consummed = 0;
    حرف Name_شروع = 0;
    منطقي Does_شروع_With = startsWith(Name, قلم.پیشوند, Strict, Consummed,
                                    Name_شروع, /*IsPrefix=*/درست);
    إذا (!Does_شروع_With)
      استمر;
    خودکار عدد = Name.substr(Consummed);
    بیعلامت دراز دراز V = 0;
    // Be consistent about mandating upper casing.
    إذا (Strict &&
        llvm::any_از(عدد, [](حرف C) { أرجع C >= 'a' && C <= 'f'; }))
      أرجع {};
    إذا (getAsUnsignedInteger(عدد, 16, V) || V < قلم.شروع || V > قلم.انتها_2)
      استمر;
    إذا (!Strict) {
      میانگیر.append(قلم.پیشوند);
      میانگیر.append(utohexstr(V, درست));
    }
    أرجع V;
  }
  أرجع std::nullopt;
}

ساكن std::optional<char32_t> name_مقصد_Codepoint(StringRef Name, منطقي Strict,
                                               میانگیر_Type &میانگیر) {
  إذا (Name.خالی())
    أرجع std::nullopt;

  std::optional<char32_t> Res = name_مقصد_Hangul_کد_نقطه(Name, Strict, میانگیر);
  إذا (!Res)
    Res = name_مقصد_Generated_کد_نقطه(Name, Strict, میانگیر);
  إذا (Res)
    أرجع *Res;

  میانگیر.clear();
  گره گره;
  منطقي Matches;
  uint32_t مقدار;
  std::tie(گره, Matches, مقدار) = compare_گره(0, Name, Strict, میانگیر);
  إذا (Matches) {
    std::معکوس(میانگیر.begin(), میانگیر.انتها());
    // UAX44-LM2. Ignore case, whitespace, underscore ('_'), and all medial
    // hyphens except the hyphen in U+1180 HANGUL JUNGSEONG O-E.
    إذا (!Strict && مقدار == 0x116c && Name.شامل_است_insensitive("O-E")) {
      میانگیر = "HANGUL JUNGSEONG O-E";
      مقدار = 0x1180;
    }
    أرجع مقدار;
  }
  أرجع std::nullopt;
}

std::optional<char32_t> name_مقصد_Codepoint_Strict(StringRef Name) {

  میانگیر_Type میانگیر;
  خودکار Opt = name_مقصد_Codepoint(Name, درست, میانگیر);
  أرجع Opt;
}

std::optional<Loose_Matching_نتیجه>
name_مقصد_Codepoint_Loose_Matching(StringRef Name) {
  میانگیر_Type میانگیر;
  خودکار Opt = name_مقصد_Codepoint(Name, خطأ, میانگیر);
  إذا (!Opt)
    أرجع std::nullopt;
  أرجع Loose_Matching_نتیجه{*Opt, میانگیر};
}

// Find the unicode character whose editing distance to Pattern
// is shortest, using the Wagner–Fischer algorithm.
llvm::Small_وکتور<تطابق_For_Codepoint_Name>
نزدیکترین_Matches_For_Codepoint_Name(StringRef الگو, std::size_t Max_Matches_شمار) {
  // We maintain a fixed size vector of matches,
  // sorted by distance
  // The worst match (with the biggest distance) are discarded when new elements
  // are added.
  std::size_t بزرگترین_Edit_فاصله = 0;
  llvm::Small_وکتور<تطابق_For_Codepoint_Name> Matches;
  Matches.reserve(Max_Matches_شمار + 1);

  خودکار درج_2 = [&](ثابت گره &گره, uint32_t فاصله_2,
                    char32_t مقدار) -> منطقي {
    إذا (فاصله_2 > بزرگترین_Edit_فاصله) {
      إذا (Matches.اندازه_2() == Max_Matches_شمار)
        أرجع خطأ;
      بزرگترین_Edit_فاصله = فاصله_2;
    }
    // To avoid allocations, the creation of the name is delayed
    // as much as possible.
    std::string Name;
    خودکار GetName = [&] {
      إذا (Name.خالی())
        Name = گره.پر_Name();
      أرجع Name;
    };

    خودکار It = llvm::پایینی_bound(
        Matches, فاصله_2,
        [&](ثابت تطابق_For_Codepoint_Name &a, std::size_t فاصله_2) {
          إذا (فاصله_2 == a.فاصله_2)
            أرجع a.Name < GetName();
          أرجع a.فاصله_2 < فاصله_2;
        });
    إذا (It == Matches.انتها() && Matches.اندازه_2() == Max_Matches_شمار)
      أرجع خطأ;

    تطابق_For_Codepoint_Name M{GetName(), فاصله_2, مقدار};
    Matches.درج(It, std::move(M));
    إذا (Matches.اندازه_2() > Max_Matches_شمار)
      Matches.برداشت_پشت();
    أرجع درست;
  };

  // We ignore case, space, hyphens, etc,
  // in both the search pattern and the prospective names.
  خودکار Normalize = [](StringRef Name) {
    std::string بیرون;
    بیرون.reserve(Name.اندازه_2());
    لكل (حرف C : Name) {
      إذا (isAlnum(C))
        بیرون.افزودن_پشت(مقصد_فوقانی(C));
    }
    أرجع بیرون;
  };
  std::string NormalizedName = Normalize(الگو);

  // Allocate a matrix big enough for longest names.
  ثابت std::size_t Columns =
      std::min(NormalizedName.اندازه_2(), Unicode_Name_مقصد_Codepoint_بزرگترین_Name_اندازه) +
      1;

  [[maybe_استفادهنشده]] ساكن std::size_t ردیفها_2 =
      Unicode_Name_مقصد_Codepoint_بزرگترین_Name_اندازه + 1;

  std::وکتور<حرف> Distances(
      Columns * (Unicode_Name_مقصد_Codepoint_بزرگترین_Name_اندازه + 1), 0);

  خودکار Get = [&Distances, Columns](size_t ستونی, std::size_t ردیف) -> حرف & {
    أكد(ستونی < Columns);
    أكد(ردیف < ردیفها_2);
    أرجع Distances[ردیف * Columns + ستونی];
  };

  لكل (std::size_t I = 0; I < Columns; I++)
    Get(I, 0) = I;

  // Visit the childrens,
  // Filling (and overriding) the matrix for the name fragment of each node
  // iteratively. CompleteName is used to collect the actual name of potential
  // match, respecting case and spacing.
  خودکار بازدید_گره = [&](ثابت گره &N, std::size_t ردیف,
                       خودکار &بازدید_گره) -> فراغ {
    std::size_t J = 0;
    لكل (; J < N.Name.اندازه_2(); J++) {
      إذا (!isAlnum(N.Name[J]))
        استمر;

      Get(0, ردیف) = ردیف;

      لكل (std::size_t I = 1; I < Columns; I++) {
        ثابت صحيح Delete = Get(I - 1, ردیف) + 1;
        ثابت صحيح درج_2 = Get(I, ردیف - 1) + 1;

        ثابت صحيح Replace =
            Get(I - 1, ردیف - 1) + (NormalizedName[I - 1] != N.Name[J] ? 1 : 0);

        Get(I, ردیف) = std::min({درج_2, Delete, Replace});
      }

      ردیف++;
    }

    بیعلامت هزینه = Get(Columns - 1, ردیف - 1);
    إذا (N.مقدار != 0xFFFFFFFF) {
      درج_2(N, هزینه, N.مقدار);
    }

    إذا (N.hasChildren()) {
      خودکار فرزند_جابه_جایی = N.Children_جابه_جایی;
      لكل (;;) {
        گره C = read_گره(فرزند_جابه_جایی, &N);
        فرزند_جابه_جایی += C.اندازه;
        إذا (!C.isValid())
          اكسر;
        بازدید_گره(C, ردیف, بازدید_گره);
        إذا (!C.HasSibling)
          اكسر;
      }
    }
  };

  گره ریشه_2 = create_ریشه();
  بازدید_گره(ریشه_2, 1, بازدید_گره);
  أرجع Matches;
}

} // namespace unicode

} // namespace sys
} // namespace llvm
