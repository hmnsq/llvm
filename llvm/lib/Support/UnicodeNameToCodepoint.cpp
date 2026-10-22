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

#অন্তর্ভাব "llvm/ADT/STLExtras.h"
#অন্তর্ভাব "llvm/ADT/StringExtras.h"
#অন্তর্ভাব "llvm/ADT/StringRef.h"
#অন্তর্ভাব "llvm/Support/Unicode.h"

নামাকাশ llvm {
নামাকাশ sys {
নামাকাশ unicode {

বাহ্যিক ধ্রুবক অক্ষর *ধ্রুবক UnicodeNameToCodepointDict;
বাহ্যিক ধ্রুবক uint8_t *ধ্রুবক Unicode_Name_To_Codepoint_সূচক;
বাহ্যিক ধ্রুবক std::size_t Unicode_Name_To_Codepoint_সূচক_আকার;
বাহ্যিক ধ্রুবক std::size_t Unicode_Name_To_Codepoint_Largest_Name_আকার;

using বাফার_Type = SmallString<64>;

কাঠামো গ্রন্থি {
  যৌক্তিক Is_মূল = মিথ্যা;
  char32_t মান = 0xFFFFFFFF;
  uint32_t ChildrenOffset = 0;
  যৌক্তিক HasSibling = মিথ্যা;
  uint32_t আকার_2 = 0;
  StringRef Name;
  ধ্রুবক গ্রন্থি *Parent = nullptr;

  constexpr যৌক্তিক isValid() ধ্রুবক {
    ফেরত !Name.empty() || মান == 0xFFFFFFFF;
  }
  constexpr যৌক্তিক hasChildren() ধ্রুবক { ফেরত ChildrenOffset != 0 || Is_মূল; }

  std::string fullName() ধ্রুবক {
    std::string S;
    // Reserve enough space for most unicode code points.
    // The chosen value represent the 99th percentile of name size as of
    // Unicode 15.0.
    S.reserve(46);
    ধ্রুবক গ্রন্থি *N = এই;
    যখন (N) {
      std::reverse_copy(N->Name.begin(), N->Name.end(), std::back_inserter(S));
      N = N->Parent;
    }
    std::reverse(S.begin(), S.end());
    ফেরত S;
  }
};

স্থির গ্রন্থি create_মূল() {
  গ্রন্থি N;
  N.Is_মূল = সত্য;
  N.ChildrenOffset = 1;
  N.আকার_2 = 1;
  ফেরত N;
}

স্থির গ্রন্থি read_গ্রন্থি(uint32_t Offset, ধ্রুবক গ্রন্থি *Parent = nullptr) {
  যদি (Offset == 0)
    ফেরত create_মূল();

  uint32_t Origin = Offset;
  গ্রন্থি N;
  N.Parent = Parent;
  uint8_t NameInfo = Unicode_Name_To_Codepoint_সূচক[Offset++];
  যদি (Offset + 6 >= Unicode_Name_To_Codepoint_সূচক_আকার)
    ফেরত N;

  যৌক্তিক LongName = NameInfo & 0x40;
  যৌক্তিক Has_মান = NameInfo & 0x80;
  std::size_t আকার_2 = NameInfo & ~0xC0;
  যদি (LongName) {
    uint32_t NameOffset = (Unicode_Name_To_Codepoint_সূচক[Offset++] << 8);
    NameOffset |= Unicode_Name_To_Codepoint_সূচক[Offset++];
    N.Name = StringRef(UnicodeNameToCodepointDict + NameOffset, আকার_2);
  } অন্যথা {
    N.Name = StringRef(UnicodeNameToCodepointDict + আকার_2, 1);
  }
  যদি (Has_মান) {
    uint8_t H = Unicode_Name_To_Codepoint_সূচক[Offset++];
    uint8_t M = Unicode_Name_To_Codepoint_সূচক[Offset++];
    uint8_t L = Unicode_Name_To_Codepoint_সূচক[Offset++];
    N.মান = ((H << 16) | (M << 8) | L) >> 3;

    যৌক্তিক HasChildren = L & 0x02;
    N.HasSibling = L & 0x01;

    যদি (HasChildren) {
      N.ChildrenOffset = Unicode_Name_To_Codepoint_সূচক[Offset++] << 16;
      N.ChildrenOffset |= Unicode_Name_To_Codepoint_সূচক[Offset++] << 8;
      N.ChildrenOffset |= Unicode_Name_To_Codepoint_সূচক[Offset++];
    }
  } অন্যথা {
    uint8_t H = Unicode_Name_To_Codepoint_সূচক[Offset++];
    N.HasSibling = H & 0x80;
    যৌক্তিক HasChildren = H & 0x40;
    H &= uint8_t(~0xC0);
    যদি (HasChildren) {
      N.ChildrenOffset = (H << 16);
      N.ChildrenOffset |=
          (uint32_t(Unicode_Name_To_Codepoint_সূচক[Offset++]) << 8);
      N.ChildrenOffset |= Unicode_Name_To_Codepoint_সূচক[Offset++];
    }
  }
  N.আকার_2 = Offset - Origin;
  ফেরত N;
}

স্থির যৌক্তিক startsWith(StringRef Name, StringRef Needle, যৌক্তিক Strict,
                       std::size_t &Consummed, অক্ষর &PreviousCharInName,
                       যৌক্তিক IsPrefix = মিথ্যা) {

  Consummed = 0;
  যদি (Strict) {
    যদি (!Name.starts_with(Needle))
      ফেরত মিথ্যা;
    Consummed = Needle.আকার_3();
    ফেরত সত্য;
  }
  যদি (Needle.empty())
    ফেরত সত্য;

  auto NamePos = Name.begin();
  auto NeedlePos = Needle.begin();

  অক্ষর PreviousCharInNameOrigin = PreviousCharInName;
  অক্ষর PreviousCharInNeedle = *Needle.begin();
  auto IgnoreSpaces = [](auto It, auto End, অক্ষর &PreviousChar,
                         যৌক্তিক IsPrefix = মিথ্যা) {
    যখন (It != End) {
      ধ্রুবক auto পরবর্তী_2 = std::পরবর্তী(It);
      // Ignore spaces, underscore, medial hyphens
      // The generator ensures a needle never ends (or starts) by a medial
      // hyphen https://unicode.org/reports/tr44/#UAX44-LM2.
      যৌক্তিক Ignore =
          *It == ' ' || *It == '_' ||
          (*It == '-' && isAlnum(PreviousChar) &&
           ((পরবর্তী_2 != End && isAlnum(*পরবর্তী_2)) || (পরবর্তী_2 == End && IsPrefix)));
      PreviousChar = *It;
      যদি (!Ignore)
        বিরতি;
      ++It;
    }
    ফেরত It;
  };

  যখন (সত্য) {
    NamePos = IgnoreSpaces(NamePos, Name.end(), PreviousCharInName);
    NeedlePos =
        IgnoreSpaces(NeedlePos, Needle.end(), PreviousCharInNeedle, IsPrefix);
    যদি (NeedlePos == Needle.end())
      বিরতি;
    যদি (NamePos == Name.end())
      বিরতি;
    যদি (toUpper(*NeedlePos) != toUpper(*NamePos))
      বিরতি;
    NeedlePos++;
    NamePos++;
  }
  Consummed = std::distance(Name.begin(), NamePos);
  যদি (NeedlePos != Needle.end()) {
    PreviousCharInName = PreviousCharInNameOrigin;
  }
  ফেরত NeedlePos == Needle.end();
}

স্থির std::tuple<গ্রন্থি, যৌক্তিক, uint32_t>
compare_গ্রন্থি(uint32_t Offset, StringRef Name, যৌক্তিক Strict,
            অক্ষর PreviousCharInName, বাফার_Type &বাফার,
            ধ্রুবক গ্রন্থি *Parent = nullptr) {
  গ্রন্থি N = read_গ্রন্থি(Offset, Parent);
  std::size_t Consummed = 0;
  যৌক্তিক DoesStartWith = N.Is_মূল || startsWith(Name, N.Name, Strict, Consummed,
                                              PreviousCharInName);
  যদি (!DoesStartWith)
    ফেরত std::make_tuple(N, মিথ্যা, 0);

  যদি (Name.আকার_3() - Consummed == 0 && N.মান != 0xFFFFFFFF)
    ফেরত std::make_tuple(N, সত্য, N.মান);

  যদি (N.hasChildren()) {
    uint32_t ChildOffset = N.ChildrenOffset;
    জন্য (;;) {
      গ্রন্থি C;
      যৌক্তিক Matches;
      uint32_t মান;
      std::tie(C, Matches, মান) =
          compare_গ্রন্থি(ChildOffset, Name.substr(Consummed), Strict,
                      PreviousCharInName, বাফার, &N);
      যদি (Matches) {
        std::reverse_copy(C.Name.begin(), C.Name.end(),
                          std::back_inserter(বাফার));
        ফেরত std::make_tuple(N, সত্য, মান);
      }
      ChildOffset += C.আকার_2;
      যদি (!C.HasSibling)
        বিরতি;
    }
  }
  ফেরত std::make_tuple(N, মিথ্যা, 0);
}

স্থির std::tuple<গ্রন্থি, যৌক্তিক, uint32_t>
compare_গ্রন্থি(uint32_t Offset, StringRef Name, যৌক্তিক Strict, বাফার_Type &বাফার) {
  ফেরত compare_গ্রন্থি(Offset, Name, Strict, 0, বাফার);
}

// clang-format off
constexpr ধ্রুবক অক্ষর *ধ্রুবক HangulSyllables[][3] = {
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

স্থির std::size_t findSyllable(StringRef Name, যৌক্তিক Strict,
                                অক্ষর &PreviousInName, পূর্ণসংখ্যা &Pos, পূর্ণসংখ্যা Column) {
  প্রতিজ্ঞা(Column == 0 || Column == 1 || Column == 2);
  স্থির std::size_t গুনতি_Per_Column[] = {LCount, VCount, TCount};
  পূর্ণসংখ্যা Len = -1;
  পূর্ণসংখ্যা পূর্ব_2 = PreviousInName;
  জন্য (std::size_t I = 0; I < গুনতি_Per_Column[Column]; I++) {
    StringRef Syllable(HangulSyllables[I][Column]);
    যদি (পূর্ণসংখ্যা(Syllable.আকার_3()) <= Len)
      চালু;
    std::size_t Consummed = 0;
    অক্ষর PreviousInNameCopy = PreviousInName;
    যৌক্তিক DoesStartWith =
        startsWith(Name, Syllable, Strict, Consummed, PreviousInNameCopy);
    যদি (!DoesStartWith)
      চালু;
    Len = Consummed;
    Pos = I;
    পূর্ব_2 = PreviousInNameCopy;
  }
  যদি (Len == -1)
    ফেরত 0;
  PreviousInName = পূর্ব_2;
  ফেরত size_t(Len);
}

স্থির std::optional<char32_t>
nameToHangulCodePoint(StringRef Name, যৌক্তিক Strict, বাফার_Type &বাফার) {
  বাফার.clear();
  // Hangul Syllable Decomposition
  std::size_t Consummed = 0;
  অক্ষর NameStart = 0;
  যৌক্তিক DoesStartWith =
      startsWith(Name, "HANGUL SYLLABLE ", Strict, Consummed, NameStart);
  যদি (!DoesStartWith)
    ফেরত std::nullopt;
  Name = Name.substr(Consummed);
  পূর্ণসংখ্যা L = -1, V = -1, T = -1;
  Name = Name.substr(findSyllable(Name, Strict, NameStart, L, 0));
  Name = Name.substr(findSyllable(Name, Strict, NameStart, V, 1));
  Name = Name.substr(findSyllable(Name, Strict, NameStart, T, 2));
  যদি (L != -1 && V != -1 && T != -1 && Name.empty()) {
    যদি (!Strict) {
      বাফার.append("HANGUL SYLLABLE ");
      যদি (L != -1)
        বাফার.append(HangulSyllables[L][0]);
      যদি (V != -1)
        বাফার.append(HangulSyllables[V][1]);
      যদি (T != -1)
        বাফার.append(HangulSyllables[T][2]);
    }
    ফেরত SBase + (std::uint32_t(L) * VCount + std::uint32_t(V)) * TCount +
           std::uint32_t(T);
  }
  // Otherwise, it's an illegal syllable name.
  ফেরত std::nullopt;
}

কাঠামো Generated_Names_তথ্য {
  StringRef Prefix;
  uint32_t Start;
  uint32_t End;
};

// Unicode 18.0 Name Derivation Rule Prefix Strings (extracted/DerivedName.txt)
স্থির ধ্রুবক Generated_Names_তথ্য Generated_Names_তথ্য_Table[] = {
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

স্থির std::optional<char32_t>
nameToGeneratedCodePoint(StringRef Name, যৌক্তিক Strict, বাফার_Type &বাফার) {
  জন্য (auto &&Item : Generated_Names_তথ্য_Table) {
    বাফার.clear();
    std::size_t Consummed = 0;
    অক্ষর NameStart = 0;
    যৌক্তিক DoesStartWith = startsWith(Name, Item.Prefix, Strict, Consummed,
                                    NameStart, /*IsPrefix=*/সত্য);
    যদি (!DoesStartWith)
      চালু;
    auto সংখ্যা = Name.substr(Consummed);
    unsigned long long V = 0;
    // Be consistent about mandating upper casing.
    যদি (Strict &&
        llvm::any_of(সংখ্যা, [](অক্ষর C) { ফেরত C >= 'a' && C <= 'f'; }))
      ফেরত {};
    যদি (getAsUnsignedInteger(সংখ্যা, 16, V) || V < Item.Start || V > Item.End)
      চালু;
    যদি (!Strict) {
      বাফার.append(Item.Prefix);
      বাফার.append(utohexstr(V, সত্য));
    }
    ফেরত V;
  }
  ফেরত std::nullopt;
}

স্থির std::optional<char32_t> nameToCodepoint(StringRef Name, যৌক্তিক Strict,
                                               বাফার_Type &বাফার) {
  যদি (Name.empty())
    ফেরত std::nullopt;

  std::optional<char32_t> Res = nameToHangulCodePoint(Name, Strict, বাফার);
  যদি (!Res)
    Res = nameToGeneratedCodePoint(Name, Strict, বাফার);
  যদি (Res)
    ফেরত *Res;

  বাফার.clear();
  গ্রন্থি গ্রন্থি;
  যৌক্তিক Matches;
  uint32_t মান;
  std::tie(গ্রন্থি, Matches, মান) = compare_গ্রন্থি(0, Name, Strict, বাফার);
  যদি (Matches) {
    std::reverse(বাফার.begin(), বাফার.end());
    // UAX44-LM2. Ignore case, whitespace, underscore ('_'), and all medial
    // hyphens except the hyphen in U+1180 HANGUL JUNGSEONG O-E.
    যদি (!Strict && মান == 0x116c && Name.contains_insensitive("O-E")) {
      বাফার = "HANGUL JUNGSEONG O-E";
      মান = 0x1180;
    }
    ফেরত মান;
  }
  ফেরত std::nullopt;
}

std::optional<char32_t> nameToCodepointStrict(StringRef Name) {

  বাফার_Type বাফার;
  auto Opt = nameToCodepoint(Name, সত্য, বাফার);
  ফেরত Opt;
}

std::optional<Loose_Matching_ফলাফল>
nameToCodepointLooseMatching(StringRef Name) {
  বাফার_Type বাফার;
  auto Opt = nameToCodepoint(Name, মিথ্যা, বাফার);
  যদি (!Opt)
    ফেরত std::nullopt;
  ফেরত Loose_Matching_ফলাফল{*Opt, বাফার};
}

// Find the unicode character whose editing distance to Pattern
// is shortest, using the Wagner–Fischer algorithm.
llvm::SmallVector<MatchForCodepointName>
nearestMatchesForCodepointName(StringRef Pattern, std::size_t Max_Matches_গুনতি) {
  // We maintain a fixed size vector of matches,
  // sorted by distance
  // The worst match (with the biggest distance) are discarded when new elements
  // are added.
  std::size_t LargestEditDistance = 0;
  llvm::SmallVector<MatchForCodepointName> Matches;
  Matches.reserve(Max_Matches_গুনতি + 1);

  auto ঢোকাও_2 = [&](ধ্রুবক গ্রন্থি &গ্রন্থি, uint32_t Distance,
                    char32_t মান) -> যৌক্তিক {
    যদি (Distance > LargestEditDistance) {
      যদি (Matches.আকার_3() == Max_Matches_গুনতি)
        ফেরত মিথ্যা;
      LargestEditDistance = Distance;
    }
    // To avoid allocations, the creation of the name is delayed
    // as much as possible.
    std::string Name;
    auto GetName = [&] {
      যদি (Name.empty())
        Name = গ্রন্থি.fullName();
      ফেরত Name;
    };

    auto It = llvm::lower_bound(
        Matches, Distance,
        [&](ধ্রুবক MatchForCodepointName &a, std::size_t Distance) {
          যদি (Distance == a.Distance)
            ফেরত a.Name < GetName();
          ফেরত a.Distance < Distance;
        });
    যদি (It == Matches.end() && Matches.আকার_3() == Max_Matches_গুনতি)
      ফেরত মিথ্যা;

    MatchForCodepointName M{GetName(), Distance, মান};
    Matches.ঢোকাও(It, std::move(M));
    যদি (Matches.আকার_3() > Max_Matches_গুনতি)
      Matches.বের_করো_back();
    ফেরত সত্য;
  };

  // We ignore case, space, hyphens, etc,
  // in both the search pattern and the prospective names.
  auto Normalize = [](StringRef Name) {
    std::string Out;
    Out.reserve(Name.আকার_3());
    জন্য (অক্ষর C : Name) {
      যদি (isAlnum(C))
        Out.ঠেলো_back(toUpper(C));
    }
    ফেরত Out;
  };
  std::string NormalizedName = Normalize(Pattern);

  // Allocate a matrix big enough for longest names.
  ধ্রুবক std::size_t Columns =
      std::min(NormalizedName.আকার_3(), Unicode_Name_To_Codepoint_Largest_Name_আকার) +
      1;

  [[maybe_unused]] স্থির std::size_t Rows =
      Unicode_Name_To_Codepoint_Largest_Name_আকার + 1;

  std::vector<অক্ষর> Distances(
      Columns * (Unicode_Name_To_Codepoint_Largest_Name_আকার + 1), 0);

  auto Get = [&Distances, Columns](size_t Column, std::size_t Row) -> অক্ষর & {
    প্রতিজ্ঞা(Column < Columns);
    প্রতিজ্ঞা(Row < Rows);
    ফেরত Distances[Row * Columns + Column];
  };

  জন্য (std::size_t I = 0; I < Columns; I++)
    Get(I, 0) = I;

  // Visit the childrens,
  // Filling (and overriding) the matrix for the name fragment of each node
  // iteratively. CompleteName is used to collect the actual name of potential
  // match, respecting case and spacing.
  auto Visit_গ্রন্থি = [&](ধ্রুবক গ্রন্থি &N, std::size_t Row,
                       auto &Visit_গ্রন্থি) -> শূন্য {
    std::size_t J = 0;
    জন্য (; J < N.Name.আকার_3(); J++) {
      যদি (!isAlnum(N.Name[J]))
        চালু;

      Get(0, Row) = Row;

      জন্য (std::size_t I = 1; I < Columns; I++) {
        ধ্রুবক পূর্ণসংখ্যা Delete = Get(I - 1, Row) + 1;
        ধ্রুবক পূর্ণসংখ্যা ঢোকাও_2 = Get(I, Row - 1) + 1;

        ধ্রুবক পূর্ণসংখ্যা Replace =
            Get(I - 1, Row - 1) + (NormalizedName[I - 1] != N.Name[J] ? 1 : 0);

        Get(I, Row) = std::min({ঢোকাও_2, Delete, Replace});
      }

      Row++;
    }

    unsigned Cost = Get(Columns - 1, Row - 1);
    যদি (N.মান != 0xFFFFFFFF) {
      ঢোকাও_2(N, Cost, N.মান);
    }

    যদি (N.hasChildren()) {
      auto ChildOffset = N.ChildrenOffset;
      জন্য (;;) {
        গ্রন্থি C = read_গ্রন্থি(ChildOffset, &N);
        ChildOffset += C.আকার_2;
        যদি (!C.isValid())
          বিরতি;
        Visit_গ্রন্থি(C, Row, Visit_গ্রন্থি);
        যদি (!C.HasSibling)
          বিরতি;
      }
    }
  };

  গ্রন্থি মূল_2 = create_মূল();
  Visit_গ্রন্থি(মূল_2, 1, Visit_গ্রন্থি);
  ফেরত Matches;
}

} // namespace unicode

} // namespace sys
} // namespace llvm
