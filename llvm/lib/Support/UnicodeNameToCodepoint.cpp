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

#समावेश "llvm/ADT/STLExtras.h"
#समावेश "llvm/ADT/StringExtras.h"
#समावेश "llvm/ADT/StringRef.h"
#समावेश "llvm/Support/Unicode.h"

namespace llvm {
namespace sys {
namespace unicode {

बाह्य स्थिर अक्षर *स्थिर UnicodeNameToCodepointDict;
बाह्य स्थिर uint8_t *स्थिर Unicode_Name_To_Codepoint_सूचकांक;
बाह्य स्थिर std::माप_प्रकार Unicode_Name_To_Codepoint_सूचकांक_आकार;
बाह्य स्थिर std::माप_प्रकार Unicode_Name_To_Codepoint_Largest_Name_आकार;

using बफर_Type = SmallString<64>;

काष्ठा गाँठ {
  bool Is_मूल = false;
  char32_t मूल्य = 0xFFFFFFFF;
  uint32_t ChildrenOffset = 0;
  bool HasSibling = false;
  uint32_t आकार = 0;
  StringRef Name;
  स्थिर गाँठ *Parent = nullptr;

  constexpr bool isValid() स्थिर {
    वापस !Name.empty() || मूल्य == 0xFFFFFFFF;
  }
  constexpr bool hasChildren() स्थिर { वापस ChildrenOffset != 0 || Is_मूल; }

  std::string fullName() स्थिर {
    std::string S;
    // Reserve enough space for most unicode code points.
    // The chosen value represent the 99th percentile of name size as of
    // Unicode 15.0.
    S.reserve(46);
    स्थिर गाँठ *N = यह;
    जबतक (N) {
      std::reverse_copy(N->Name.begin(), N->Name.end(), std::back_inserter(S));
      N = N->Parent;
    }
    std::reverse(S.begin(), S.end());
    वापस S;
  }
};

अटल गाँठ create_मूल() {
  गाँठ N;
  N.Is_मूल = true;
  N.ChildrenOffset = 1;
  N.आकार = 1;
  वापस N;
}

अटल गाँठ read_गाँठ(uint32_t Offset, स्थिर गाँठ *Parent = nullptr) {
  अगर (Offset == 0)
    वापस create_मूल();

  uint32_t Origin = Offset;
  गाँठ N;
  N.Parent = Parent;
  uint8_t NameInfo = Unicode_Name_To_Codepoint_सूचकांक[Offset++];
  अगर (Offset + 6 >= Unicode_Name_To_Codepoint_सूचकांक_आकार)
    वापस N;

  bool LongName = NameInfo & 0x40;
  bool Has_मूल्य = NameInfo & 0x80;
  std::माप_प्रकार आकार = NameInfo & ~0xC0;
  अगर (LongName) {
    uint32_t NameOffset = (Unicode_Name_To_Codepoint_सूचकांक[Offset++] << 8);
    NameOffset |= Unicode_Name_To_Codepoint_सूचकांक[Offset++];
    N.Name = StringRef(UnicodeNameToCodepointDict + NameOffset, आकार);
  } अन्यथा {
    N.Name = StringRef(UnicodeNameToCodepointDict + आकार, 1);
  }
  अगर (Has_मूल्य) {
    uint8_t H = Unicode_Name_To_Codepoint_सूचकांक[Offset++];
    uint8_t M = Unicode_Name_To_Codepoint_सूचकांक[Offset++];
    uint8_t L = Unicode_Name_To_Codepoint_सूचकांक[Offset++];
    N.मूल्य = ((H << 16) | (M << 8) | L) >> 3;

    bool HasChildren = L & 0x02;
    N.HasSibling = L & 0x01;

    अगर (HasChildren) {
      N.ChildrenOffset = Unicode_Name_To_Codepoint_सूचकांक[Offset++] << 16;
      N.ChildrenOffset |= Unicode_Name_To_Codepoint_सूचकांक[Offset++] << 8;
      N.ChildrenOffset |= Unicode_Name_To_Codepoint_सूचकांक[Offset++];
    }
  } अन्यथा {
    uint8_t H = Unicode_Name_To_Codepoint_सूचकांक[Offset++];
    N.HasSibling = H & 0x80;
    bool HasChildren = H & 0x40;
    H &= uint8_t(~0xC0);
    अगर (HasChildren) {
      N.ChildrenOffset = (H << 16);
      N.ChildrenOffset |=
          (uint32_t(Unicode_Name_To_Codepoint_सूचकांक[Offset++]) << 8);
      N.ChildrenOffset |= Unicode_Name_To_Codepoint_सूचकांक[Offset++];
    }
  }
  N.आकार = Offset - Origin;
  वापस N;
}

अटल bool startsWith(StringRef Name, StringRef Needle, bool Strict,
                       std::माप_प्रकार &Consummed, अक्षर &PreviousCharInName,
                       bool IsPrefix = false) {

  Consummed = 0;
  अगर (Strict) {
    अगर (!Name.starts_with(Needle))
      वापस false;
    Consummed = Needle.आकार_2();
    वापस true;
  }
  अगर (Needle.empty())
    वापस true;

  स्वतः NamePos = Name.begin();
  स्वतः NeedlePos = Needle.begin();

  अक्षर PreviousCharInNameOrigin = PreviousCharInName;
  अक्षर PreviousCharInNeedle = *Needle.begin();
  स्वतः IgnoreSpaces = [](स्वतः It, स्वतः End, अक्षर &PreviousChar,
                         bool IsPrefix = false) {
    जबतक (It != End) {
      स्थिर स्वतः अग्र_2 = std::अग्र(It);
      // Ignore spaces, underscore, medial hyphens
      // The generator ensures a needle never ends (or starts) by a medial
      // hyphen https://unicode.org/reports/tr44/#UAX44-LM2.
      bool Ignore =
          *It == ' ' || *It == '_' ||
          (*It == '-' && isAlnum(PreviousChar) &&
           ((अग्र_2 != End && isAlnum(*अग्र_2)) || (अग्र_2 == End && IsPrefix)));
      PreviousChar = *It;
      अगर (!Ignore)
        अवरोध;
      ++It;
    }
    वापस It;
  };

  जबतक (true) {
    NamePos = IgnoreSpaces(NamePos, Name.end(), PreviousCharInName);
    NeedlePos =
        IgnoreSpaces(NeedlePos, Needle.end(), PreviousCharInNeedle, IsPrefix);
    अगर (NeedlePos == Needle.end())
      अवरोध;
    अगर (NamePos == Name.end())
      अवरोध;
    अगर (toUpper(*NeedlePos) != toUpper(*NamePos))
      अवरोध;
    NeedlePos++;
    NamePos++;
  }
  Consummed = std::distance(Name.begin(), NamePos);
  अगर (NeedlePos != Needle.end()) {
    PreviousCharInName = PreviousCharInNameOrigin;
  }
  वापस NeedlePos == Needle.end();
}

अटल std::tuple<गाँठ, bool, uint32_t>
compare_गाँठ(uint32_t Offset, StringRef Name, bool Strict,
            अक्षर PreviousCharInName, बफर_Type &बफर,
            स्थिर गाँठ *Parent = nullptr) {
  गाँठ N = read_गाँठ(Offset, Parent);
  std::माप_प्रकार Consummed = 0;
  bool DoesStartWith = N.Is_मूल || startsWith(Name, N.Name, Strict, Consummed,
                                              PreviousCharInName);
  अगर (!DoesStartWith)
    वापस std::make_tuple(N, false, 0);

  अगर (Name.आकार_2() - Consummed == 0 && N.मूल्य != 0xFFFFFFFF)
    वापस std::make_tuple(N, true, N.मूल्य);

  अगर (N.hasChildren()) {
    uint32_t ChildOffset = N.ChildrenOffset;
    क्रम (;;) {
      गाँठ C;
      bool Matches;
      uint32_t मूल्य;
      std::tie(C, Matches, मूल्य) =
          compare_गाँठ(ChildOffset, Name.substr(Consummed), Strict,
                      PreviousCharInName, बफर, &N);
      अगर (Matches) {
        std::reverse_copy(C.Name.begin(), C.Name.end(),
                          std::back_inserter(बफर));
        वापस std::make_tuple(N, true, मूल्य);
      }
      ChildOffset += C.आकार;
      अगर (!C.HasSibling)
        अवरोध;
    }
  }
  वापस std::make_tuple(N, false, 0);
}

अटल std::tuple<गाँठ, bool, uint32_t>
compare_गाँठ(uint32_t Offset, StringRef Name, bool Strict, बफर_Type &बफर) {
  वापस compare_गाँठ(Offset, Name, Strict, 0, बफर);
}

// clang-format off
constexpr स्थिर अक्षर *स्थिर HangulSyllables[][3] = {
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

अटल std::माप_प्रकार findSyllable(StringRef Name, bool Strict,
                                अक्षर &PreviousInName, पूर्णांक &Pos, पूर्णांक Column) {
  निश्चित(Column == 0 || Column == 1 || Column == 2);
  अटल std::माप_प्रकार गिनती_Per_Column[] = {LCount, VCount, TCount};
  पूर्णांक Len = -1;
  पूर्णांक पूर्व_2 = PreviousInName;
  क्रम (std::माप_प्रकार I = 0; I < गिनती_Per_Column[Column]; I++) {
    StringRef Syllable(HangulSyllables[I][Column]);
    अगर (पूर्णांक(Syllable.आकार_2()) <= Len)
      जारी;
    std::माप_प्रकार Consummed = 0;
    अक्षर PreviousInNameCopy = PreviousInName;
    bool DoesStartWith =
        startsWith(Name, Syllable, Strict, Consummed, PreviousInNameCopy);
    अगर (!DoesStartWith)
      जारी;
    Len = Consummed;
    Pos = I;
    पूर्व_2 = PreviousInNameCopy;
  }
  अगर (Len == -1)
    वापस 0;
  PreviousInName = पूर्व_2;
  वापस माप_प्रकार(Len);
}

अटल std::optional<char32_t>
nameToHangulCodePoint(StringRef Name, bool Strict, बफर_Type &बफर) {
  बफर.सफा();
  // Hangul Syllable Decomposition
  std::माप_प्रकार Consummed = 0;
  अक्षर NameStart = 0;
  bool DoesStartWith =
      startsWith(Name, "HANGUL SYLLABLE ", Strict, Consummed, NameStart);
  अगर (!DoesStartWith)
    वापस std::nullopt;
  Name = Name.substr(Consummed);
  पूर्णांक L = -1, V = -1, T = -1;
  Name = Name.substr(findSyllable(Name, Strict, NameStart, L, 0));
  Name = Name.substr(findSyllable(Name, Strict, NameStart, V, 1));
  Name = Name.substr(findSyllable(Name, Strict, NameStart, T, 2));
  अगर (L != -1 && V != -1 && T != -1 && Name.empty()) {
    अगर (!Strict) {
      बफर.append("HANGUL SYLLABLE ");
      अगर (L != -1)
        बफर.append(HangulSyllables[L][0]);
      अगर (V != -1)
        बफर.append(HangulSyllables[V][1]);
      अगर (T != -1)
        बफर.append(HangulSyllables[T][2]);
    }
    वापस SBase + (std::uint32_t(L) * VCount + std::uint32_t(V)) * TCount +
           std::uint32_t(T);
  }
  // Otherwise, it's an illegal syllable name.
  वापस std::nullopt;
}

काष्ठा Generated_Names_तथ्य {
  StringRef Prefix;
  uint32_t Start;
  uint32_t End;
};

// Unicode 18.0 Name Derivation Rule Prefix Strings (extracted/DerivedName.txt)
अटल स्थिर Generated_Names_तथ्य Generated_Names_तथ्य_Table[] = {
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

अटल std::optional<char32_t>
nameToGeneratedCodePoint(StringRef Name, bool Strict, बफर_Type &बफर) {
  क्रम (स्वतः &&Item : Generated_Names_तथ्य_Table) {
    बफर.सफा();
    std::माप_प्रकार Consummed = 0;
    अक्षर NameStart = 0;
    bool DoesStartWith = startsWith(Name, Item.Prefix, Strict, Consummed,
                                    NameStart, /*IsPrefix=*/true);
    अगर (!DoesStartWith)
      जारी;
    स्वतः संख्या = Name.substr(Consummed);
    अचिन्हित दीर्घ दीर्घ V = 0;
    // Be consistent about mandating upper casing.
    अगर (Strict &&
        llvm::any_of(संख्या, [](अक्षर C) { वापस C >= 'a' && C <= 'f'; }))
      वापस {};
    अगर (getAsUnsignedInteger(संख्या, 16, V) || V < Item.Start || V > Item.End)
      जारी;
    अगर (!Strict) {
      बफर.append(Item.Prefix);
      बफर.append(utohexstr(V, true));
    }
    वापस V;
  }
  वापस std::nullopt;
}

अटल std::optional<char32_t> nameToCodepoint(StringRef Name, bool Strict,
                                               बफर_Type &बफर) {
  अगर (Name.empty())
    वापस std::nullopt;

  std::optional<char32_t> Res = nameToHangulCodePoint(Name, Strict, बफर);
  अगर (!Res)
    Res = nameToGeneratedCodePoint(Name, Strict, बफर);
  अगर (Res)
    वापस *Res;

  बफर.सफा();
  गाँठ गाँठ;
  bool Matches;
  uint32_t मूल्य;
  std::tie(गाँठ, Matches, मूल्य) = compare_गाँठ(0, Name, Strict, बफर);
  अगर (Matches) {
    std::reverse(बफर.begin(), बफर.end());
    // UAX44-LM2. Ignore case, whitespace, underscore ('_'), and all medial
    // hyphens except the hyphen in U+1180 HANGUL JUNGSEONG O-E.
    अगर (!Strict && मूल्य == 0x116c && Name.contains_insensitive("O-E")) {
      बफर = "HANGUL JUNGSEONG O-E";
      मूल्य = 0x1180;
    }
    वापस मूल्य;
  }
  वापस std::nullopt;
}

std::optional<char32_t> nameToCodepointStrict(StringRef Name) {

  बफर_Type बफर;
  स्वतः Opt = nameToCodepoint(Name, true, बफर);
  वापस Opt;
}

std::optional<Loose_Matching_परिणाम>
nameToCodepointLooseMatching(StringRef Name) {
  बफर_Type बफर;
  स्वतः Opt = nameToCodepoint(Name, false, बफर);
  अगर (!Opt)
    वापस std::nullopt;
  वापस Loose_Matching_परिणाम{*Opt, बफर};
}

// Find the unicode character whose editing distance to Pattern
// is shortest, using the Wagner–Fischer algorithm.
llvm::SmallVector<MatchForCodepointName>
nearestMatchesForCodepointName(StringRef Pattern, std::माप_प्रकार Max_Matches_गिनती) {
  // We maintain a fixed size vector of matches,
  // sorted by distance
  // The worst match (with the biggest distance) are discarded when new elements
  // are added.
  std::माप_प्रकार LargestEditDistance = 0;
  llvm::SmallVector<MatchForCodepointName> Matches;
  Matches.reserve(Max_Matches_गिनती + 1);

  स्वतः जोड़ो_2 = [&](स्थिर गाँठ &गाँठ, uint32_t Distance,
                    char32_t मूल्य) -> bool {
    अगर (Distance > LargestEditDistance) {
      अगर (Matches.आकार_2() == Max_Matches_गिनती)
        वापस false;
      LargestEditDistance = Distance;
    }
    // To avoid allocations, the creation of the name is delayed
    // as much as possible.
    std::string Name;
    स्वतः GetName = [&] {
      अगर (Name.empty())
        Name = गाँठ.fullName();
      वापस Name;
    };

    स्वतः It = llvm::lower_bound(
        Matches, Distance,
        [&](स्थिर MatchForCodepointName &a, std::माप_प्रकार Distance) {
          अगर (Distance == a.Distance)
            वापस a.Name < GetName();
          वापस a.Distance < Distance;
        });
    अगर (It == Matches.end() && Matches.आकार_2() == Max_Matches_गिनती)
      वापस false;

    MatchForCodepointName M{GetName(), Distance, मूल्य};
    Matches.जोड़ो(It, std::move(M));
    अगर (Matches.आकार_2() > Max_Matches_गिनती)
      Matches.निकालो_back();
    वापस true;
  };

  // We ignore case, space, hyphens, etc,
  // in both the search pattern and the prospective names.
  स्वतः Normalize = [](StringRef Name) {
    std::string Out;
    Out.reserve(Name.आकार_2());
    क्रम (अक्षर C : Name) {
      अगर (isAlnum(C))
        Out.धकेलो_back(toUpper(C));
    }
    वापस Out;
  };
  std::string NormalizedName = Normalize(Pattern);

  // Allocate a matrix big enough for longest names.
  स्थिर std::माप_प्रकार Columns =
      std::min(NormalizedName.आकार_2(), Unicode_Name_To_Codepoint_Largest_Name_आकार) +
      1;

  [[maybe_unused]] अटल std::माप_प्रकार Rows =
      Unicode_Name_To_Codepoint_Largest_Name_आकार + 1;

  std::vector<अक्षर> Distances(
      Columns * (Unicode_Name_To_Codepoint_Largest_Name_आकार + 1), 0);

  स्वतः Get = [&Distances, Columns](माप_प्रकार Column, std::माप_प्रकार Row) -> अक्षर & {
    निश्चित(Column < Columns);
    निश्चित(Row < Rows);
    वापस Distances[Row * Columns + Column];
  };

  क्रम (std::माप_प्रकार I = 0; I < Columns; I++)
    Get(I, 0) = I;

  // Visit the childrens,
  // Filling (and overriding) the matrix for the name fragment of each node
  // iteratively. CompleteName is used to collect the actual name of potential
  // match, respecting case and spacing.
  स्वतः Visit_गाँठ = [&](स्थिर गाँठ &N, std::माप_प्रकार Row,
                       स्वतः &Visit_गाँठ) -> व्योम {
    std::माप_प्रकार J = 0;
    क्रम (; J < N.Name.आकार_2(); J++) {
      अगर (!isAlnum(N.Name[J]))
        जारी;

      Get(0, Row) = Row;

      क्रम (std::माप_प्रकार I = 1; I < Columns; I++) {
        स्थिर पूर्णांक Delete = Get(I - 1, Row) + 1;
        स्थिर पूर्णांक जोड़ो_2 = Get(I, Row - 1) + 1;

        स्थिर पूर्णांक Replace =
            Get(I - 1, Row - 1) + (NormalizedName[I - 1] != N.Name[J] ? 1 : 0);

        Get(I, Row) = std::min({जोड़ो_2, Delete, Replace});
      }

      Row++;
    }

    अचिन्हित Cost = Get(Columns - 1, Row - 1);
    अगर (N.मूल्य != 0xFFFFFFFF) {
      जोड़ो_2(N, Cost, N.मूल्य);
    }

    अगर (N.hasChildren()) {
      स्वतः ChildOffset = N.ChildrenOffset;
      क्रम (;;) {
        गाँठ C = read_गाँठ(ChildOffset, &N);
        ChildOffset += C.आकार;
        अगर (!C.isValid())
          अवरोध;
        Visit_गाँठ(C, Row, Visit_गाँठ);
        अगर (!C.HasSibling)
          अवरोध;
      }
    }
  };

  गाँठ मूल_2 = create_मूल();
  Visit_गाँठ(मूल_2, 1, Visit_गाँठ);
  वापस Matches;
}

} // namespace unicode

} // namespace sys
} // namespace llvm
