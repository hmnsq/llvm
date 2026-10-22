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

#شامل "llvm/ADT/STLExtras.h"
#شامل "llvm/ADT/StringExtras.h"
#شامل "llvm/ADT/StringRef.h"
#شامل "llvm/Support/Unicode.h"

نامخلا llvm {
نامخلا sys {
نامخلا unicode {

خارجی ثابت حرف *ثابت Unicode_Name_مقصد_Codepoint_Dict;
خارجی ثابت uint8_t *ثابت Unicode_Name_مقصد_Codepoint_اشاریہ;
خارجی ثابت std::size_t Unicode_Name_مقصد_Codepoint_اشاریہ_سائز;
خارجی ثابت std::size_t Unicode_Name_مقصد_Codepoint_سب_سے_بڑا_Name_سائز;

استعمال بفر_Type = SmallString<64>;

ساخت گرہ {
  منطقی Is_جڑ = جھوٹ;
  char32_t قدر = 0xFFFFFFFF;
  uint32_t Children_انحراف = 0;
  منطقی HasSibling = جھوٹ;
  uint32_t سائز = 0;
  StringRef Name;
  ثابت گرہ *والد_2 = خالی_اشاریہ;

  constexpr منطقی isValid() ثابت {
    واپس !Name.خالی_خانے() || قدر == 0xFFFFFFFF;
  }
  constexpr منطقی hasChildren() ثابت { واپس Children_انحراف != 0 || Is_جڑ; }

  std::string بھرا_Name() ثابت {
    std::string S;
    // Reserve enough space for most unicode code points.
    // The chosen value represent the 99th percentile of name size as of
    // Unicode 15.0.
    S.reserve(46);
    ثابت گرہ *N = یہ;
    جب (N) {
      std::الٹ_copy(N->Name.begin(), N->Name.انتہا(), std::عقب_inserter(S));
      N = N->والد_2;
    }
    std::الٹ(S.begin(), S.انتہا());
    واپس S;
  }
};

ساکن گرہ create_جڑ() {
  گرہ N;
  N.Is_جڑ = سچ;
  N.Children_انحراف = 1;
  N.سائز = 1;
  واپس N;
}

ساکن گرہ read_گرہ(uint32_t انحراف, ثابت گرہ *والد_2 = خالی_اشاریہ) {
  اگر (انحراف == 0)
    واپس create_جڑ();

  uint32_t Origin = انحراف;
  گرہ N;
  N.والد_2 = والد_2;
  uint8_t NameInfo = Unicode_Name_مقصد_Codepoint_اشاریہ[انحراف++];
  اگر (انحراف + 6 >= Unicode_Name_مقصد_Codepoint_اشاریہ_سائز)
    واپس N;

  منطقی LongName = NameInfo & 0x40;
  منطقی Has_قدر = NameInfo & 0x80;
  std::size_t سائز = NameInfo & ~0xC0;
  اگر (LongName) {
    uint32_t Name_انحراف = (Unicode_Name_مقصد_Codepoint_اشاریہ[انحراف++] << 8);
    Name_انحراف |= Unicode_Name_مقصد_Codepoint_اشاریہ[انحراف++];
    N.Name = StringRef(Unicode_Name_مقصد_Codepoint_Dict + Name_انحراف, سائز);
  } ورنہ {
    N.Name = StringRef(Unicode_Name_مقصد_Codepoint_Dict + سائز, 1);
  }
  اگر (Has_قدر) {
    uint8_t H = Unicode_Name_مقصد_Codepoint_اشاریہ[انحراف++];
    uint8_t M = Unicode_Name_مقصد_Codepoint_اشاریہ[انحراف++];
    uint8_t L = Unicode_Name_مقصد_Codepoint_اشاریہ[انحراف++];
    N.قدر = ((H << 16) | (M << 8) | L) >> 3;

    منطقی HasChildren = L & 0x02;
    N.HasSibling = L & 0x01;

    اگر (HasChildren) {
      N.Children_انحراف = Unicode_Name_مقصد_Codepoint_اشاریہ[انحراف++] << 16;
      N.Children_انحراف |= Unicode_Name_مقصد_Codepoint_اشاریہ[انحراف++] << 8;
      N.Children_انحراف |= Unicode_Name_مقصد_Codepoint_اشاریہ[انحراف++];
    }
  } ورنہ {
    uint8_t H = Unicode_Name_مقصد_Codepoint_اشاریہ[انحراف++];
    N.HasSibling = H & 0x80;
    منطقی HasChildren = H & 0x40;
    H &= uint8_t(~0xC0);
    اگر (HasChildren) {
      N.Children_انحراف = (H << 16);
      N.Children_انحراف |=
          (uint32_t(Unicode_Name_مقصد_Codepoint_اشاریہ[انحراف++]) << 8);
      N.Children_انحراف |= Unicode_Name_مقصد_Codepoint_اشاریہ[انحراف++];
    }
  }
  N.سائز = انحراف - Origin;
  واپس N;
}

ساکن منطقی startsWith(StringRef Name, StringRef Needle, منطقی Strict,
                       std::size_t &Consummed, حرف &پچھلا_Char_اندر_Name,
                       منطقی Is_سابقہ = جھوٹ) {

  Consummed = 0;
  اگر (Strict) {
    اگر (!Name.starts_with(Needle))
      واپس جھوٹ;
    Consummed = Needle.سائز_2();
    واپس سچ;
  }
  اگر (Needle.خالی_خانے())
    واپس سچ;

  خودکار NamePos = Name.begin();
  خودکار NeedlePos = Needle.begin();

  حرف پچھلا_Char_اندر_Name_Origin = پچھلا_Char_اندر_Name;
  حرف پچھلا_Char_اندر_Needle = *Needle.begin();
  خودکار IgnoreSpaces = [](خودکار It, خودکار انتہا_2, حرف &پچھلا_Char,
                         منطقی Is_سابقہ = جھوٹ) {
    جب (It != انتہا_2) {
      ثابت خودکار اگلا_2 = std::اگلا(It);
      // Ignore spaces, underscore, medial hyphens
      // The generator ensures a needle never ends (or starts) by a medial
      // hyphen https://unicode.org/reports/tr44/#UAX44-LM2.
      منطقی Ignore =
          *It == ' ' || *It == '_' ||
          (*It == '-' && isAlnum(پچھلا_Char) &&
           ((اگلا_2 != انتہا_2 && isAlnum(*اگلا_2)) || (اگلا_2 == انتہا_2 && Is_سابقہ)));
      پچھلا_Char = *It;
      اگر (!Ignore)
        توڑو;
      ++It;
    }
    واپس It;
  };

  جب (سچ) {
    NamePos = IgnoreSpaces(NamePos, Name.انتہا(), پچھلا_Char_اندر_Name);
    NeedlePos =
        IgnoreSpaces(NeedlePos, Needle.انتہا(), پچھلا_Char_اندر_Needle, Is_سابقہ);
    اگر (NeedlePos == Needle.انتہا())
      توڑو;
    اگر (NamePos == Name.انتہا())
      توڑو;
    اگر (مقصد_بالائی(*NeedlePos) != مقصد_بالائی(*NamePos))
      توڑو;
    NeedlePos++;
    NamePos++;
  }
  Consummed = std::فاصلہ(Name.begin(), NamePos);
  اگر (NeedlePos != Needle.انتہا()) {
    پچھلا_Char_اندر_Name = پچھلا_Char_اندر_Name_Origin;
  }
  واپس NeedlePos == Needle.انتہا();
}

ساکن std::tuple<گرہ, منطقی, uint32_t>
compare_گرہ(uint32_t انحراف, StringRef Name, منطقی Strict,
            حرف پچھلا_Char_اندر_Name, بفر_Type &بفر,
            ثابت گرہ *والد_2 = خالی_اشاریہ) {
  گرہ N = read_گرہ(انحراف, والد_2);
  std::size_t Consummed = 0;
  منطقی Does_آغاز_With = N.Is_جڑ || startsWith(Name, N.Name, Strict, Consummed,
                                              پچھلا_Char_اندر_Name);
  اگر (!Does_آغاز_With)
    واپس std::make_tuple(N, جھوٹ, 0);

  اگر (Name.سائز_2() - Consummed == 0 && N.قدر != 0xFFFFFFFF)
    واپس std::make_tuple(N, سچ, N.قدر);

  اگر (N.hasChildren()) {
    uint32_t بچہ_انحراف = N.Children_انحراف;
    برائے (;;) {
      گرہ C;
      منطقی Matches;
      uint32_t قدر;
      std::tie(C, Matches, قدر) =
          compare_گرہ(بچہ_انحراف, Name.substr(Consummed), Strict,
                      پچھلا_Char_اندر_Name, بفر, &N);
      اگر (Matches) {
        std::الٹ_copy(C.Name.begin(), C.Name.انتہا(),
                          std::عقب_inserter(بفر));
        واپس std::make_tuple(N, سچ, قدر);
      }
      بچہ_انحراف += C.سائز;
      اگر (!C.HasSibling)
        توڑو;
    }
  }
  واپس std::make_tuple(N, جھوٹ, 0);
}

ساکن std::tuple<گرہ, منطقی, uint32_t>
compare_گرہ(uint32_t انحراف, StringRef Name, منطقی Strict, بفر_Type &بفر) {
  واپس compare_گرہ(انحراف, Name, Strict, 0, بفر);
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

ساکن std::size_t ڈھونڈو_Syllable(StringRef Name, منطقی Strict,
                                حرف &پچھلا_اندر_Name, صحیح &Pos, صحیح عمود) {
  أكد(عمود == 0 || عمود == 1 || عمود == 2);
  ساکن std::size_t گنتی_فی_عمود[] = {LCount, VCount, TCount};
  صحیح Len = -1;
  صحیح پچھلا_2 = پچھلا_اندر_Name;
  برائے (std::size_t I = 0; I < گنتی_فی_عمود[عمود]; I++) {
    StringRef Syllable(HangulSyllables[I][عمود]);
    اگر (صحیح(Syllable.سائز_2()) <= Len)
      جاری;
    std::size_t Consummed = 0;
    حرف پچھلا_اندر_Name_Copy = پچھلا_اندر_Name;
    منطقی Does_آغاز_With =
        startsWith(Name, Syllable, Strict, Consummed, پچھلا_اندر_Name_Copy);
    اگر (!Does_آغاز_With)
      جاری;
    Len = Consummed;
    Pos = I;
    پچھلا_2 = پچھلا_اندر_Name_Copy;
  }
  اگر (Len == -1)
    واپس 0;
  پچھلا_اندر_Name = پچھلا_2;
  واپس size_t(Len);
}

ساکن std::optional<char32_t>
name_مقصد_Hangul_رمز_نقطہ(StringRef Name, منطقی Strict, بفر_Type &بفر) {
  بفر.clear();
  // Hangul Syllable Decomposition
  std::size_t Consummed = 0;
  حرف Name_آغاز = 0;
  منطقی Does_آغاز_With =
      startsWith(Name, "HANGUL SYLLABLE ", Strict, Consummed, Name_آغاز);
  اگر (!Does_آغاز_With)
    واپس std::nullopt;
  Name = Name.substr(Consummed);
  صحیح L = -1, V = -1, T = -1;
  Name = Name.substr(ڈھونڈو_Syllable(Name, Strict, Name_آغاز, L, 0));
  Name = Name.substr(ڈھونڈو_Syllable(Name, Strict, Name_آغاز, V, 1));
  Name = Name.substr(ڈھونڈو_Syllable(Name, Strict, Name_آغاز, T, 2));
  اگر (L != -1 && V != -1 && T != -1 && Name.خالی_خانے()) {
    اگر (!Strict) {
      بفر.append("HANGUL SYLLABLE ");
      اگر (L != -1)
        بفر.append(HangulSyllables[L][0]);
      اگر (V != -1)
        بفر.append(HangulSyllables[V][1]);
      اگر (T != -1)
        بفر.append(HangulSyllables[T][2]);
    }
    واپس SBase + (std::uint32_t(L) * VCount + std::uint32_t(V)) * TCount +
           std::uint32_t(T);
  }
  // Otherwise, it's an illegal syllable name.
  واپس std::nullopt;
}

ساخت Generated_نام_ڈیٹا {
  StringRef سابقہ;
  uint32_t آغاز;
  uint32_t انتہا_2;
};

// Unicode 18.0 Name Derivation Rule Prefix Strings (extracted/DerivedName.txt)
ساکن ثابت Generated_نام_ڈیٹا Generated_نام_ڈیٹا_جدول[] = {
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

ساکن std::optional<char32_t>
name_مقصد_Generated_رمز_نقطہ(StringRef Name, منطقی Strict, بفر_Type &بفر) {
  برائے (خودکار &&شے : Generated_نام_ڈیٹا_جدول) {
    بفر.clear();
    std::size_t Consummed = 0;
    حرف Name_آغاز = 0;
    منطقی Does_آغاز_With = startsWith(Name, شے.سابقہ, Strict, Consummed,
                                    Name_آغاز, /*IsPrefix=*/سچ);
    اگر (!Does_آغاز_With)
      جاری;
    خودکار نمبر = Name.substr(Consummed);
    غیرعلامتی طویل طویل V = 0;
    // Be consistent about mandating upper casing.
    اگر (Strict &&
        llvm::any_کا(نمبر, [](حرف C) { واپس C >= 'a' && C <= 'f'; }))
      واپس {};
    اگر (getAsUnsignedInteger(نمبر, 16, V) || V < شے.آغاز || V > شے.انتہا_2)
      جاری;
    اگر (!Strict) {
      بفر.append(شے.سابقہ);
      بفر.append(utohexstr(V, سچ));
    }
    واپس V;
  }
  واپس std::nullopt;
}

ساکن std::optional<char32_t> name_مقصد_Codepoint(StringRef Name, منطقی Strict,
                                               بفر_Type &بفر) {
  اگر (Name.خالی_خانے())
    واپس std::nullopt;

  std::optional<char32_t> Res = name_مقصد_Hangul_رمز_نقطہ(Name, Strict, بفر);
  اگر (!Res)
    Res = name_مقصد_Generated_رمز_نقطہ(Name, Strict, بفر);
  اگر (Res)
    واپس *Res;

  بفر.clear();
  گرہ گرہ;
  منطقی Matches;
  uint32_t قدر;
  std::tie(گرہ, Matches, قدر) = compare_گرہ(0, Name, Strict, بفر);
  اگر (Matches) {
    std::الٹ(بفر.begin(), بفر.انتہا());
    // UAX44-LM2. Ignore case, whitespace, underscore ('_'), and all medial
    // hyphens except the hyphen in U+1180 HANGUL JUNGSEONG O-E.
    اگر (!Strict && قدر == 0x116c && Name.شامل_ہے_insensitive("O-E")) {
      بفر = "HANGUL JUNGSEONG O-E";
      قدر = 0x1180;
    }
    واپس قدر;
  }
  واپس std::nullopt;
}

std::optional<char32_t> name_مقصد_Codepoint_Strict(StringRef Name) {

  بفر_Type بفر;
  خودکار Opt = name_مقصد_Codepoint(Name, سچ, بفر);
  واپس Opt;
}

std::optional<Loose_Matching_نتیجہ>
name_مقصد_Codepoint_Loose_Matching(StringRef Name) {
  بفر_Type بفر;
  خودکار Opt = name_مقصد_Codepoint(Name, جھوٹ, بفر);
  اگر (!Opt)
    واپس std::nullopt;
  واپس Loose_Matching_نتیجہ{*Opt, بفر};
}

// Find the unicode character whose editing distance to Pattern
// is shortest, using the Wagner–Fischer algorithm.
llvm::Small_سمتیہ<مطابقت_For_Codepoint_Name>
قریب_ترین_Matches_For_Codepoint_Name(StringRef نمونہ, std::size_t Max_Matches_گنتی) {
  // We maintain a fixed size vector of matches,
  // sorted by distance
  // The worst match (with the biggest distance) are discarded when new elements
  // are added.
  std::size_t سب_سے_بڑا_Edit_فاصلہ = 0;
  llvm::Small_سمتیہ<مطابقت_For_Codepoint_Name> Matches;
  Matches.reserve(Max_Matches_گنتی + 1);

  خودکار داخل_کرو_2 = [&](ثابت گرہ &گرہ, uint32_t فاصلہ_2,
                    char32_t قدر) -> منطقی {
    اگر (فاصلہ_2 > سب_سے_بڑا_Edit_فاصلہ) {
      اگر (Matches.سائز_2() == Max_Matches_گنتی)
        واپس جھوٹ;
      سب_سے_بڑا_Edit_فاصلہ = فاصلہ_2;
    }
    // To avoid allocations, the creation of the name is delayed
    // as much as possible.
    std::string Name;
    خودکار GetName = [&] {
      اگر (Name.خالی_خانے())
        Name = گرہ.بھرا_Name();
      واپس Name;
    };

    خودکار It = llvm::زیریں_bound(
        Matches, فاصلہ_2,
        [&](ثابت مطابقت_For_Codepoint_Name &a, std::size_t فاصلہ_2) {
          اگر (فاصلہ_2 == a.فاصلہ_2)
            واپس a.Name < GetName();
          واپس a.فاصلہ_2 < فاصلہ_2;
        });
    اگر (It == Matches.انتہا() && Matches.سائز_2() == Max_Matches_گنتی)
      واپس جھوٹ;

    مطابقت_For_Codepoint_Name M{GetName(), فاصلہ_2, قدر};
    Matches.داخل_کرو(It, std::move(M));
    اگر (Matches.سائز_2() > Max_Matches_گنتی)
      Matches.نکالو_عقب();
    واپس سچ;
  };

  // We ignore case, space, hyphens, etc,
  // in both the search pattern and the prospective names.
  خودکار Normalize = [](StringRef Name) {
    std::string برآمد;
    برآمد.reserve(Name.سائز_2());
    برائے (حرف C : Name) {
      اگر (isAlnum(C))
        برآمد.ڈالو_عقب(مقصد_بالائی(C));
    }
    واپس برآمد;
  };
  std::string NormalizedName = Normalize(نمونہ);

  // Allocate a matrix big enough for longest names.
  ثابت std::size_t Columns =
      std::min(NormalizedName.سائز_2(), Unicode_Name_مقصد_Codepoint_سب_سے_بڑا_Name_سائز) +
      1;

  [[maybe_غیر_مستعمل]] ساکن std::size_t صفیں_2 =
      Unicode_Name_مقصد_Codepoint_سب_سے_بڑا_Name_سائز + 1;

  std::سمتیہ<حرف> Distances(
      Columns * (Unicode_Name_مقصد_Codepoint_سب_سے_بڑا_Name_سائز + 1), 0);

  خودکار Get = [&Distances, Columns](size_t عمود, std::size_t صف) -> حرف & {
    أكد(عمود < Columns);
    أكد(صف < صفیں_2);
    واپس Distances[صف * Columns + عمود];
  };

  برائے (std::size_t I = 0; I < Columns; I++)
    Get(I, 0) = I;

  // Visit the childrens,
  // Filling (and overriding) the matrix for the name fragment of each node
  // iteratively. CompleteName is used to collect the actual name of potential
  // match, respecting case and spacing.
  خودکار دورہ_گرہ = [&](ثابت گرہ &N, std::size_t صف,
                       خودکار &دورہ_گرہ) -> خالی {
    std::size_t J = 0;
    برائے (; J < N.Name.سائز_2(); J++) {
      اگر (!isAlnum(N.Name[J]))
        جاری;

      Get(0, صف) = صف;

      برائے (std::size_t I = 1; I < Columns; I++) {
        ثابت صحیح Delete = Get(I - 1, صف) + 1;
        ثابت صحیح داخل_کرو_2 = Get(I, صف - 1) + 1;

        ثابت صحیح Replace =
            Get(I - 1, صف - 1) + (NormalizedName[I - 1] != N.Name[J] ? 1 : 0);

        Get(I, صف) = std::min({داخل_کرو_2, Delete, Replace});
      }

      صف++;
    }

    غیرعلامتی لاگت = Get(Columns - 1, صف - 1);
    اگر (N.قدر != 0xFFFFFFFF) {
      داخل_کرو_2(N, لاگت, N.قدر);
    }

    اگر (N.hasChildren()) {
      خودکار بچہ_انحراف = N.Children_انحراف;
      برائے (;;) {
        گرہ C = read_گرہ(بچہ_انحراف, &N);
        بچہ_انحراف += C.سائز;
        اگر (!C.isValid())
          توڑو;
        دورہ_گرہ(C, صف, دورہ_گرہ);
        اگر (!C.HasSibling)
          توڑو;
      }
    }
  };

  گرہ جڑ_2 = create_جڑ();
  دورہ_گرہ(جڑ_2, 1, دورہ_گرہ);
  واپس Matches;
}

} // namespace unicode

} // namespace sys
} // namespace llvm
