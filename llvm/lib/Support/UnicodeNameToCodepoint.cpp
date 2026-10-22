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

#अन्तर्भाव "llvm/ADT/STLExtras.h"
#अन्तर्भाव "llvm/ADT/StringExtras.h"
#अन्तर्भाव "llvm/ADT/StringRef.h"
#अन्तर्भाव "llvm/Support/Unicode.h"

नामाकाश llvm {
नामाकाश sys {
नामाकाश unicode {

बाह्य ध्रुव वर्ण *ध्रुव Unicode_Name_गन्तव्यम्_Codepoint_Dict;
बाह्य ध्रुव uint8_t *ध्रुव Unicode_Name_गन्तव्यम्_Codepoint_सूचकांक;
बाह्य ध्रुव std::size_t Unicode_Name_गन्तव्यम्_Codepoint_सूचकांक_आकार;
बाह्य ध्रुव std::size_t Unicode_Name_गन्तव्यम्_Codepoint_Largest_Name_आकार;

उपयुज्य बफर_Type = SmallString<64>;

संरचना ग्रन्थिः {
  तार्किक Is_मूलम् = असत्य;
  char32_t मानम् = 0xFFFFFFFF;
  uint32_t Children_विस्थापनम् = 0;
  तार्किक HasSibling = असत्य;
  uint32_t आकार = 0;
  StringRef Name;
  ध्रुव ग्रन्थिः *Parent = शून्यसूचकः;

  constexpr तार्किक isValid() ध्रुव {
    प्रत्यागम !Name.रिक्तानि() || मानम् == 0xFFFFFFFF;
  }
  constexpr तार्किक hasChildren() ध्रुव { प्रत्यागम Children_विस्थापनम् != 0 || Is_मूलम्; }

  std::string पूर्णम्_Name() ध्रुव {
    std::string S;
    // Reserve enough space for most unicode code points.
    // The chosen value represent the 99th percentile of name size as of
    // Unicode 15.0.
    S.reserve(46);
    ध्रुव ग्रन्थिः *N = एतत्;
    यावत् (N) {
      std::reverse_copy(N->Name.begin(), N->Name.अन्तः(), std::पृष्ठभागः_inserter(S));
      N = N->Parent;
    }
    std::reverse(S.begin(), S.अन्तः());
    प्रत्यागम S;
  }
};

स्थिर ग्रन्थिः create_मूलम्() {
  ग्रन्थिः N;
  N.Is_मूलम् = सत्य;
  N.Children_विस्थापनम् = 1;
  N.आकार = 1;
  प्रत्यागम N;
}

स्थिर ग्रन्थिः read_ग्रन्थिः(uint32_t विस्थापनम्, ध्रुव ग्रन्थिः *Parent = शून्यसूचकः) {
  यदि (विस्थापनम् == 0)
    प्रत्यागम create_मूलम्();

  uint32_t Origin = विस्थापनम्;
  ग्रन्थिः N;
  N.Parent = Parent;
  uint8_t NameInfo = Unicode_Name_गन्तव्यम्_Codepoint_सूचकांक[विस्थापनम्++];
  यदि (विस्थापनम् + 6 >= Unicode_Name_गन्तव्यम्_Codepoint_सूचकांक_आकार)
    प्रत्यागम N;

  तार्किक LongName = NameInfo & 0x40;
  तार्किक Has_मानम् = NameInfo & 0x80;
  std::size_t आकार = NameInfo & ~0xC0;
  यदि (LongName) {
    uint32_t Name_विस्थापनम् = (Unicode_Name_गन्तव्यम्_Codepoint_सूचकांक[विस्थापनम्++] << 8);
    Name_विस्थापनम् |= Unicode_Name_गन्तव्यम्_Codepoint_सूचकांक[विस्थापनम्++];
    N.Name = StringRef(Unicode_Name_गन्तव्यम्_Codepoint_Dict + Name_विस्थापनम्, आकार);
  } अन्यथा {
    N.Name = StringRef(Unicode_Name_गन्तव्यम्_Codepoint_Dict + आकार, 1);
  }
  यदि (Has_मानम्) {
    uint8_t H = Unicode_Name_गन्तव्यम्_Codepoint_सूचकांक[विस्थापनम्++];
    uint8_t M = Unicode_Name_गन्तव्यम्_Codepoint_सूचकांक[विस्थापनम्++];
    uint8_t L = Unicode_Name_गन्तव्यम्_Codepoint_सूचकांक[विस्थापनम्++];
    N.मानम् = ((H << 16) | (M << 8) | L) >> 3;

    तार्किक HasChildren = L & 0x02;
    N.HasSibling = L & 0x01;

    यदि (HasChildren) {
      N.Children_विस्थापनम् = Unicode_Name_गन्तव्यम्_Codepoint_सूचकांक[विस्थापनम्++] << 16;
      N.Children_विस्थापनम् |= Unicode_Name_गन्तव्यम्_Codepoint_सूचकांक[विस्थापनम्++] << 8;
      N.Children_विस्थापनम् |= Unicode_Name_गन्तव्यम्_Codepoint_सूचकांक[विस्थापनम्++];
    }
  } अन्यथा {
    uint8_t H = Unicode_Name_गन्तव्यम्_Codepoint_सूचकांक[विस्थापनम्++];
    N.HasSibling = H & 0x80;
    तार्किक HasChildren = H & 0x40;
    H &= uint8_t(~0xC0);
    यदि (HasChildren) {
      N.Children_विस्थापनम् = (H << 16);
      N.Children_विस्थापनम् |=
          (uint32_t(Unicode_Name_गन्तव्यम्_Codepoint_सूचकांक[विस्थापनम्++]) << 8);
      N.Children_विस्थापनम् |= Unicode_Name_गन्तव्यम्_Codepoint_सूचकांक[विस्थापनम्++];
    }
  }
  N.आकार = विस्थापनम् - Origin;
  प्रत्यागम N;
}

स्थिर तार्किक startsWith(StringRef Name, StringRef Needle, तार्किक Strict,
                       std::size_t &Consummed, वर्ण &Previous_Char_अन्तर्_Name,
                       तार्किक Is_उपसर्गः = असत्य) {

  Consummed = 0;
  यदि (Strict) {
    यदि (!Name.starts_with(Needle))
      प्रत्यागम असत्य;
    Consummed = Needle.आकार_2();
    प्रत्यागम सत्य;
  }
  यदि (Needle.रिक्तानि())
    प्रत्यागम सत्य;

  स्वचालित NamePos = Name.begin();
  स्वचालित NeedlePos = Needle.begin();

  वर्ण Previous_Char_अन्तर्_Name_Origin = Previous_Char_अन्तर्_Name;
  वर्ण Previous_Char_अन्तर्_Needle = *Needle.begin();
  स्वचालित IgnoreSpaces = [](स्वचालित It, स्वचालित अन्तः_2, वर्ण &PreviousChar,
                         तार्किक Is_उपसर्गः = असत्य) {
    यावत् (It != अन्तः_2) {
      ध्रुव स्वचालित अग्र_2 = std::अग्र(It);
      // Ignore spaces, underscore, medial hyphens
      // The generator ensures a needle never ends (or starts) by a medial
      // hyphen https://unicode.org/reports/tr44/#UAX44-LM2.
      तार्किक Ignore =
          *It == ' ' || *It == '_' ||
          (*It == '-' && isAlnum(PreviousChar) &&
           ((अग्र_2 != अन्तः_2 && isAlnum(*अग्र_2)) || (अग्र_2 == अन्तः_2 && Is_उपसर्गः)));
      PreviousChar = *It;
      यदि (!Ignore)
        भङ्ग;
      ++It;
    }
    प्रत्यागम It;
  };

  यावत् (सत्य) {
    NamePos = IgnoreSpaces(NamePos, Name.अन्तः(), Previous_Char_अन्तर्_Name);
    NeedlePos =
        IgnoreSpaces(NeedlePos, Needle.अन्तः(), Previous_Char_अन्तर्_Needle, Is_उपसर्गः);
    यदि (NeedlePos == Needle.अन्तः())
      भङ्ग;
    यदि (NamePos == Name.अन्तः())
      भङ्ग;
    यदि (गन्तव्यम्_ऊर्ध्वस्थम्(*NeedlePos) != गन्तव्यम्_ऊर्ध्वस्थम्(*NamePos))
      भङ्ग;
    NeedlePos++;
    NamePos++;
  }
  Consummed = std::distance(Name.begin(), NamePos);
  यदि (NeedlePos != Needle.अन्तः()) {
    Previous_Char_अन्तर्_Name = Previous_Char_अन्तर्_Name_Origin;
  }
  प्रत्यागम NeedlePos == Needle.अन्तः();
}

स्थिर std::tuple<ग्रन्थिः, तार्किक, uint32_t>
compare_ग्रन्थिः(uint32_t विस्थापनम्, StringRef Name, तार्किक Strict,
            वर्ण Previous_Char_अन्तर्_Name, बफर_Type &बफर,
            ध्रुव ग्रन्थिः *Parent = शून्यसूचकः) {
  ग्रन्थिः N = read_ग्रन्थिः(विस्थापनम्, Parent);
  std::size_t Consummed = 0;
  तार्किक DoesStartWith = N.Is_मूलम् || startsWith(Name, N.Name, Strict, Consummed,
                                              Previous_Char_अन्तर्_Name);
  यदि (!DoesStartWith)
    प्रत्यागम std::make_tuple(N, असत्य, 0);

  यदि (Name.आकार_2() - Consummed == 0 && N.मानम् != 0xFFFFFFFF)
    प्रत्यागम std::make_tuple(N, सत्य, N.मानम्);

  यदि (N.hasChildren()) {
    uint32_t Child_विस्थापनम् = N.Children_विस्थापनम्;
    प्रति (;;) {
      ग्रन्थिः C;
      तार्किक Matches;
      uint32_t मानम्;
      std::tie(C, Matches, मानम्) =
          compare_ग्रन्थिः(Child_विस्थापनम्, Name.substr(Consummed), Strict,
                      Previous_Char_अन्तर्_Name, बफर, &N);
      यदि (Matches) {
        std::reverse_copy(C.Name.begin(), C.Name.अन्तः(),
                          std::पृष्ठभागः_inserter(बफर));
        प्रत्यागम std::make_tuple(N, सत्य, मानम्);
      }
      Child_विस्थापनम् += C.आकार;
      यदि (!C.HasSibling)
        भङ्ग;
    }
  }
  प्रत्यागम std::make_tuple(N, असत्य, 0);
}

स्थिर std::tuple<ग्रन्थिः, तार्किक, uint32_t>
compare_ग्रन्थिः(uint32_t विस्थापनम्, StringRef Name, तार्किक Strict, बफर_Type &बफर) {
  प्रत्यागम compare_ग्रन्थिः(विस्थापनम्, Name, Strict, 0, बफर);
}

// clang-format off
constexpr ध्रुव वर्ण *ध्रुव HangulSyllables[][3] = {
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

स्थिर std::size_t findSyllable(StringRef Name, तार्किक Strict,
                                वर्ण &Previous_अन्तर्_Name, पूर्णाङ्क &Pos, पूर्णाङ्क Column) {
  प्रतिज्ञा(Column == 0 || Column == 1 || Column == 2);
  स्थिर std::size_t गणनम्_प्रत्येकम्_Column[] = {LCount, VCount, TCount};
  पूर्णाङ्क Len = -1;
  पूर्णाङ्क पूर्व_2 = Previous_अन्तर्_Name;
  प्रति (std::size_t I = 0; I < गणनम्_प्रत्येकम्_Column[Column]; I++) {
    StringRef Syllable(HangulSyllables[I][Column]);
    यदि (पूर्णाङ्क(Syllable.आकार_2()) <= Len)
      अनुवर्त;
    std::size_t Consummed = 0;
    वर्ण Previous_अन्तर्_Name_Copy = Previous_अन्तर्_Name;
    तार्किक DoesStartWith =
        startsWith(Name, Syllable, Strict, Consummed, Previous_अन्तर्_Name_Copy);
    यदि (!DoesStartWith)
      अनुवर्त;
    Len = Consummed;
    Pos = I;
    पूर्व_2 = Previous_अन्तर्_Name_Copy;
  }
  यदि (Len == -1)
    प्रत्यागम 0;
  Previous_अन्तर्_Name = पूर्व_2;
  प्रत्यागम size_t(Len);
}

स्थिर std::optional<char32_t>
name_गन्तव्यम्_Hangul_संहिता_Point(StringRef Name, तार्किक Strict, बफर_Type &बफर) {
  बफर.clear();
  // Hangul Syllable Decomposition
  std::size_t Consummed = 0;
  वर्ण NameStart = 0;
  तार्किक DoesStartWith =
      startsWith(Name, "HANGUL SYLLABLE ", Strict, Consummed, NameStart);
  यदि (!DoesStartWith)
    प्रत्यागम std::nullopt;
  Name = Name.substr(Consummed);
  पूर्णाङ्क L = -1, V = -1, T = -1;
  Name = Name.substr(findSyllable(Name, Strict, NameStart, L, 0));
  Name = Name.substr(findSyllable(Name, Strict, NameStart, V, 1));
  Name = Name.substr(findSyllable(Name, Strict, NameStart, T, 2));
  यदि (L != -1 && V != -1 && T != -1 && Name.रिक्तानि()) {
    यदि (!Strict) {
      बफर.append("HANGUL SYLLABLE ");
      यदि (L != -1)
        बफर.append(HangulSyllables[L][0]);
      यदि (V != -1)
        बफर.append(HangulSyllables[V][1]);
      यदि (T != -1)
        बफर.append(HangulSyllables[T][2]);
    }
    प्रत्यागम SBase + (std::uint32_t(L) * VCount + std::uint32_t(V)) * TCount +
           std::uint32_t(T);
  }
  // Otherwise, it's an illegal syllable name.
  प्रत्यागम std::nullopt;
}

संरचना Generated_नामानि_दत्त {
  StringRef उपसर्गः;
  uint32_t Start;
  uint32_t अन्तः_2;
};

// Unicode 18.0 Name Derivation Rule Prefix Strings (extracted/DerivedName.txt)
स्थिर ध्रुव Generated_नामानि_दत्त Generated_नामानि_दत्त_सारणी[] = {
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

स्थिर std::optional<char32_t>
name_गन्तव्यम्_Generated_संहिता_Point(StringRef Name, तार्किक Strict, बफर_Type &बफर) {
  प्रति (स्वचालित &&Item : Generated_नामानि_दत्त_सारणी) {
    बफर.clear();
    std::size_t Consummed = 0;
    वर्ण NameStart = 0;
    तार्किक DoesStartWith = startsWith(Name, Item.उपसर्गः, Strict, Consummed,
                                    NameStart, /*IsPrefix=*/सत्य);
    यदि (!DoesStartWith)
      अनुवर्त;
    स्वचालित सङ्ख्या = Name.substr(Consummed);
    अचिह्नित दीर्घ दीर्घ V = 0;
    // Be consistent about mandating upper casing.
    यदि (Strict &&
        llvm::any_सम्बन्धि(सङ्ख्या, [](वर्ण C) { प्रत्यागम C >= 'a' && C <= 'f'; }))
      प्रत्यागम {};
    यदि (getAsUnsignedInteger(सङ्ख्या, 16, V) || V < Item.Start || V > Item.अन्तः_2)
      अनुवर्त;
    यदि (!Strict) {
      बफर.append(Item.उपसर्गः);
      बफर.append(utohexstr(V, सत्य));
    }
    प्रत्यागम V;
  }
  प्रत्यागम std::nullopt;
}

स्थिर std::optional<char32_t> name_गन्तव्यम्_Codepoint(StringRef Name, तार्किक Strict,
                                               बफर_Type &बफर) {
  यदि (Name.रिक्तानि())
    प्रत्यागम std::nullopt;

  std::optional<char32_t> Res = name_गन्तव्यम्_Hangul_संहिता_Point(Name, Strict, बफर);
  यदि (!Res)
    Res = name_गन्तव्यम्_Generated_संहिता_Point(Name, Strict, बफर);
  यदि (Res)
    प्रत्यागम *Res;

  बफर.clear();
  ग्रन्थिः ग्रन्थिः;
  तार्किक Matches;
  uint32_t मानम्;
  std::tie(ग्रन्थिः, Matches, मानम्) = compare_ग्रन्थिः(0, Name, Strict, बफर);
  यदि (Matches) {
    std::reverse(बफर.begin(), बफर.अन्तः());
    // UAX44-LM2. Ignore case, whitespace, underscore ('_'), and all medial
    // hyphens except the hyphen in U+1180 HANGUL JUNGSEONG O-E.
    यदि (!Strict && मानम् == 0x116c && Name.अन्तर्भवति_insensitive("O-E")) {
      बफर = "HANGUL JUNGSEONG O-E";
      मानम् = 0x1180;
    }
    प्रत्यागम मानम्;
  }
  प्रत्यागम std::nullopt;
}

std::optional<char32_t> name_गन्तव्यम्_Codepoint_Strict(StringRef Name) {

  बफर_Type बफर;
  स्वचालित Opt = name_गन्तव्यम्_Codepoint(Name, सत्य, बफर);
  प्रत्यागम Opt;
}

std::optional<Loose_Matching_परिणाम>
name_गन्तव्यम्_Codepoint_Loose_Matching(StringRef Name) {
  बफर_Type बफर;
  स्वचालित Opt = name_गन्तव्यम्_Codepoint(Name, असत्य, बफर);
  यदि (!Opt)
    प्रत्यागम std::nullopt;
  प्रत्यागम Loose_Matching_परिणाम{*Opt, बफर};
}

// Find the unicode character whose editing distance to Pattern
// is shortest, using the Wagner–Fischer algorithm.
llvm::Small_सदिशम्<मेलनम्_For_Codepoint_Name>
निकटतमः_Matches_For_Codepoint_Name(StringRef आदर्शः, std::size_t Max_Matches_गणनम्) {
  // We maintain a fixed size vector of matches,
  // sorted by distance
  // The worst match (with the biggest distance) are discarded when new elements
  // are added.
  std::size_t LargestEditDistance = 0;
  llvm::Small_सदिशम्<मेलनम्_For_Codepoint_Name> Matches;
  Matches.reserve(Max_Matches_गणनम् + 1);

  स्वचालित निवेशय_2 = [&](ध्रुव ग्रन्थिः &ग्रन्थिः, uint32_t Distance,
                    char32_t मानम्) -> तार्किक {
    यदि (Distance > LargestEditDistance) {
      यदि (Matches.आकार_2() == Max_Matches_गणनम्)
        प्रत्यागम असत्य;
      LargestEditDistance = Distance;
    }
    // To avoid allocations, the creation of the name is delayed
    // as much as possible.
    std::string Name;
    स्वचालित GetName = [&] {
      यदि (Name.रिक्तानि())
        Name = ग्रन्थिः.पूर्णम्_Name();
      प्रत्यागम Name;
    };

    स्वचालित It = llvm::अधस्तनम्_bound(
        Matches, Distance,
        [&](ध्रुव मेलनम्_For_Codepoint_Name &a, std::size_t Distance) {
          यदि (Distance == a.Distance)
            प्रत्यागम a.Name < GetName();
          प्रत्यागम a.Distance < Distance;
        });
    यदि (It == Matches.अन्तः() && Matches.आकार_2() == Max_Matches_गणनम्)
      प्रत्यागम असत्य;

    मेलनम्_For_Codepoint_Name M{GetName(), Distance, मानम्};
    Matches.निवेशय(It, std::move(M));
    यदि (Matches.आकार_2() > Max_Matches_गणनम्)
      Matches.निष्कासय_पृष्ठभागः();
    प्रत्यागम सत्य;
  };

  // We ignore case, space, hyphens, etc,
  // in both the search pattern and the prospective names.
  स्वचालित Normalize = [](StringRef Name) {
    std::string बहिर्गमनम्;
    बहिर्गमनम्.reserve(Name.आकार_2());
    प्रति (वर्ण C : Name) {
      यदि (isAlnum(C))
        बहिर्गमनम्.प्रक्षिप_पृष्ठभागः(गन्तव्यम्_ऊर्ध्वस्थम्(C));
    }
    प्रत्यागम बहिर्गमनम्;
  };
  std::string NormalizedName = Normalize(आदर्शः);

  // Allocate a matrix big enough for longest names.
  ध्रुव std::size_t Columns =
      std::min(NormalizedName.आकार_2(), Unicode_Name_गन्तव्यम्_Codepoint_Largest_Name_आकार) +
      1;

  [[maybe_अप्रयुक्तम्]] स्थिर std::size_t पङ्क्तिकाः_2 =
      Unicode_Name_गन्तव्यम्_Codepoint_Largest_Name_आकार + 1;

  std::सदिशम्<वर्ण> Distances(
      Columns * (Unicode_Name_गन्तव्यम्_Codepoint_Largest_Name_आकार + 1), 0);

  स्वचालित Get = [&Distances, Columns](size_t Column, std::size_t Row) -> वर्ण & {
    प्रतिज्ञा(Column < Columns);
    प्रतिज्ञा(Row < पङ्क्तिकाः_2);
    प्रत्यागम Distances[Row * Columns + Column];
  };

  प्रति (std::size_t I = 0; I < Columns; I++)
    Get(I, 0) = I;

  // Visit the childrens,
  // Filling (and overriding) the matrix for the name fragment of each node
  // iteratively. CompleteName is used to collect the actual name of potential
  // match, respecting case and spacing.
  स्वचालित दर्शय_ग्रन्थिः = [&](ध्रुव ग्रन्थिः &N, std::size_t Row,
                       स्वचालित &दर्शय_ग्रन्थिः) -> शून्य {
    std::size_t J = 0;
    प्रति (; J < N.Name.आकार_2(); J++) {
      यदि (!isAlnum(N.Name[J]))
        अनुवर्त;

      Get(0, Row) = Row;

      प्रति (std::size_t I = 1; I < Columns; I++) {
        ध्रुव पूर्णाङ्क Delete = Get(I - 1, Row) + 1;
        ध्रुव पूर्णाङ्क निवेशय_2 = Get(I, Row - 1) + 1;

        ध्रुव पूर्णाङ्क Replace =
            Get(I - 1, Row - 1) + (NormalizedName[I - 1] != N.Name[J] ? 1 : 0);

        Get(I, Row) = std::min({निवेशय_2, Delete, Replace});
      }

      Row++;
    }

    अचिह्नित Cost = Get(Columns - 1, Row - 1);
    यदि (N.मानम् != 0xFFFFFFFF) {
      निवेशय_2(N, Cost, N.मानम्);
    }

    यदि (N.hasChildren()) {
      स्वचालित Child_विस्थापनम् = N.Children_विस्थापनम्;
      प्रति (;;) {
        ग्रन्थिः C = read_ग्रन्थिः(Child_विस्थापनम्, &N);
        Child_विस्थापनम् += C.आकार;
        यदि (!C.isValid())
          भङ्ग;
        दर्शय_ग्रन्थिः(C, Row, दर्शय_ग्रन्थिः);
        यदि (!C.HasSibling)
          भङ्ग;
      }
    }
  };

  ग्रन्थिः मूलम्_2 = create_मूलम्();
  दर्शय_ग्रन्थिः(मूलम्_2, 1, दर्शय_ग्रन्थिः);
  प्रत्यागम Matches;
}

} // namespace unicode

} // namespace sys
} // namespace llvm
