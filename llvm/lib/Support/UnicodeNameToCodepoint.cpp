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

#అన్తర్భావ "llvm/ADT/STLExtras.h"
#అన్తర్భావ "llvm/ADT/StringExtras.h"
#అన్తర్భావ "llvm/ADT/StringRef.h"
#అన్తర్భావ "llvm/Support/Unicode.h"

నామాకాశ llvm {
నామాకాశ sys {
నామాకాశ unicode {

బాహ్య స్థిరరాశి అక్షరం *స్థిరరాశి Unicode_Name_గమ్యం_Codepoint_Dict;
బాహ్య స్థిరరాశి uint8_t *స్థిరరాశి Unicode_Name_గమ్యం_Codepoint_సూచీ;
బాహ్య స్థిరరాశి std::size_t Unicode_Name_గమ్యం_Codepoint_సూచీ_కొలత;
బాహ్య స్థిరరాశి std::size_t Unicode_Name_గమ్యం_Codepoint_అతిపెద్ద_Name_కొలత;

ఉపయోగించి బఫరు_Type = SmallString<64>;

నిర్మాణం కణుపు {
  తార్కిక Is_మూలం = అబద్ధం;
  char32_t విలువ = 0xFFFFFFFF;
  uint32_t Children_స్థానభ్రంశం = 0;
  తార్కిక HasSibling = అబద్ధం;
  uint32_t కొలత = 0;
  StringRef Name;
  స్థిరరాశి కణుపు *మాతృ_2 = శూన్యసూచిక;

  constexpr తార్కిక isValid() స్థిరరాశి {
    తిరిగి !Name.ఖాళీలు() || విలువ == 0xFFFFFFFF;
  }
  constexpr తార్కిక hasChildren() స్థిరరాశి { తిరిగి Children_స్థానభ్రంశం != 0 || Is_మూలం; }

  std::string నిండిన_Name() స్థిరరాశి {
    std::string S;
    // Reserve enough space for most unicode code points.
    // The chosen value represent the 99th percentile of name size as of
    // Unicode 15.0.
    S.reserve(46);
    స్థిరరాశి కణుపు *N = ఇది;
    అయితే (N) {
      std::తిరగేయి_copy(N->Name.begin(), N->Name.చివర(), std::వెనుక_inserter(S));
      N = N->మాతృ_2;
    }
    std::తిరగేయి(S.begin(), S.చివర());
    తిరిగి S;
  }
};

స్థిర కణుపు create_మూలం() {
  కణుపు N;
  N.Is_మూలం = నిజం;
  N.Children_స్థానభ్రంశం = 1;
  N.కొలత = 1;
  తిరిగి N;
}

స్థిర కణుపు read_కణుపు(uint32_t స్థానభ్రంశం, స్థిరరాశి కణుపు *మాతృ_2 = శూన్యసూచిక) {
  ఒకవేళ (స్థానభ్రంశం == 0)
    తిరిగి create_మూలం();

  uint32_t Origin = స్థానభ్రంశం;
  కణుపు N;
  N.మాతృ_2 = మాతృ_2;
  uint8_t NameInfo = Unicode_Name_గమ్యం_Codepoint_సూచీ[స్థానభ్రంశం++];
  ఒకవేళ (స్థానభ్రంశం + 6 >= Unicode_Name_గమ్యం_Codepoint_సూచీ_కొలత)
    తిరిగి N;

  తార్కిక LongName = NameInfo & 0x40;
  తార్కిక Has_విలువ = NameInfo & 0x80;
  std::size_t కొలత = NameInfo & ~0xC0;
  ఒకవేళ (LongName) {
    uint32_t Name_స్థానభ్రంశం = (Unicode_Name_గమ్యం_Codepoint_సూచీ[స్థానభ్రంశం++] << 8);
    Name_స్థానభ్రంశం |= Unicode_Name_గమ్యం_Codepoint_సూచీ[స్థానభ్రంశం++];
    N.Name = StringRef(Unicode_Name_గమ్యం_Codepoint_Dict + Name_స్థానభ్రంశం, కొలత);
  } లేకపోతే {
    N.Name = StringRef(Unicode_Name_గమ్యం_Codepoint_Dict + కొలత, 1);
  }
  ఒకవేళ (Has_విలువ) {
    uint8_t H = Unicode_Name_గమ్యం_Codepoint_సూచీ[స్థానభ్రంశం++];
    uint8_t M = Unicode_Name_గమ్యం_Codepoint_సూచీ[స్థానభ్రంశం++];
    uint8_t L = Unicode_Name_గమ్యం_Codepoint_సూచీ[స్థానభ్రంశం++];
    N.విలువ = ((H << 16) | (M << 8) | L) >> 3;

    తార్కిక HasChildren = L & 0x02;
    N.HasSibling = L & 0x01;

    ఒకవేళ (HasChildren) {
      N.Children_స్థానభ్రంశం = Unicode_Name_గమ్యం_Codepoint_సూచీ[స్థానభ్రంశం++] << 16;
      N.Children_స్థానభ్రంశం |= Unicode_Name_గమ్యం_Codepoint_సూచీ[స్థానభ్రంశం++] << 8;
      N.Children_స్థానభ్రంశం |= Unicode_Name_గమ్యం_Codepoint_సూచీ[స్థానభ్రంశం++];
    }
  } లేకపోతే {
    uint8_t H = Unicode_Name_గమ్యం_Codepoint_సూచీ[స్థానభ్రంశం++];
    N.HasSibling = H & 0x80;
    తార్కిక HasChildren = H & 0x40;
    H &= uint8_t(~0xC0);
    ఒకవేళ (HasChildren) {
      N.Children_స్థానభ్రంశం = (H << 16);
      N.Children_స్థానభ్రంశం |=
          (uint32_t(Unicode_Name_గమ్యం_Codepoint_సూచీ[స్థానభ్రంశం++]) << 8);
      N.Children_స్థానభ్రంశం |= Unicode_Name_గమ్యం_Codepoint_సూచీ[స్థానభ్రంశం++];
    }
  }
  N.కొలత = స్థానభ్రంశం - Origin;
  తిరిగి N;
}

స్థిర తార్కిక startsWith(StringRef Name, StringRef Needle, తార్కిక Strict,
                       std::size_t &Consummed, అక్షరం &మునుపటి_Char_లోపల_Name,
                       తార్కిక Is_ఉపసర్గ = అబద్ధం) {

  Consummed = 0;
  ఒకవేళ (Strict) {
    ఒకవేళ (!Name.starts_with(Needle))
      తిరిగి అబద్ధం;
    Consummed = Needle.కొలత_2();
    తిరిగి నిజం;
  }
  ఒకవేళ (Needle.ఖాళీలు())
    తిరిగి నిజం;

  స్వయంచాలక NamePos = Name.begin();
  స్వయంచాలక NeedlePos = Needle.begin();

  అక్షరం మునుపటి_Char_లోపల_Name_Origin = మునుపటి_Char_లోపల_Name;
  అక్షరం మునుపటి_Char_లోపల_Needle = *Needle.begin();
  స్వయంచాలక IgnoreSpaces = [](స్వయంచాలక It, స్వయంచాలక చివర_2, అక్షరం &మునుపటి_Char,
                         తార్కిక Is_ఉపసర్గ = అబద్ధం) {
    అయితే (It != చివర_2) {
      స్థిరరాశి స్వయంచాలక తదుపరి_2 = std::తదుపరి(It);
      // Ignore spaces, underscore, medial hyphens
      // The generator ensures a needle never ends (or starts) by a medial
      // hyphen https://unicode.org/reports/tr44/#UAX44-LM2.
      తార్కిక Ignore =
          *It == ' ' || *It == '_' ||
          (*It == '-' && isAlnum(మునుపటి_Char) &&
           ((తదుపరి_2 != చివర_2 && isAlnum(*తదుపరి_2)) || (తదుపరి_2 == చివర_2 && Is_ఉపసర్గ)));
      మునుపటి_Char = *It;
      ఒకవేళ (!Ignore)
        ఆపు;
      ++It;
    }
    తిరిగి It;
  };

  అయితే (నిజం) {
    NamePos = IgnoreSpaces(NamePos, Name.చివర(), మునుపటి_Char_లోపల_Name);
    NeedlePos =
        IgnoreSpaces(NeedlePos, Needle.చివర(), మునుపటి_Char_లోపల_Needle, Is_ఉపసర్గ);
    ఒకవేళ (NeedlePos == Needle.చివర())
      ఆపు;
    ఒకవేళ (NamePos == Name.చివర())
      ఆపు;
    ఒకవేళ (గమ్యం_పై(*NeedlePos) != గమ్యం_పై(*NamePos))
      ఆపు;
    NeedlePos++;
    NamePos++;
  }
  Consummed = std::దూరం(Name.begin(), NamePos);
  ఒకవేళ (NeedlePos != Needle.చివర()) {
    మునుపటి_Char_లోపల_Name = మునుపటి_Char_లోపల_Name_Origin;
  }
  తిరిగి NeedlePos == Needle.చివర();
}

స్థిర std::tuple<కణుపు, తార్కిక, uint32_t>
compare_కణుపు(uint32_t స్థానభ్రంశం, StringRef Name, తార్కిక Strict,
            అక్షరం మునుపటి_Char_లోపల_Name, బఫరు_Type &బఫరు,
            స్థిరరాశి కణుపు *మాతృ_2 = శూన్యసూచిక) {
  కణుపు N = read_కణుపు(స్థానభ్రంశం, మాతృ_2);
  std::size_t Consummed = 0;
  తార్కిక Does_ప్రారంభం_With = N.Is_మూలం || startsWith(Name, N.Name, Strict, Consummed,
                                              మునుపటి_Char_లోపల_Name);
  ఒకవేళ (!Does_ప్రారంభం_With)
    తిరిగి std::make_tuple(N, అబద్ధం, 0);

  ఒకవేళ (Name.కొలత_2() - Consummed == 0 && N.విలువ != 0xFFFFFFFF)
    తిరిగి std::make_tuple(N, నిజం, N.విలువ);

  ఒకవేళ (N.hasChildren()) {
    uint32_t బిడ్డ_స్థానభ్రంశం = N.Children_స్థానభ్రంశం;
    కొరకు (;;) {
      కణుపు C;
      తార్కిక Matches;
      uint32_t విలువ;
      std::tie(C, Matches, విలువ) =
          compare_కణుపు(బిడ్డ_స్థానభ్రంశం, Name.substr(Consummed), Strict,
                      మునుపటి_Char_లోపల_Name, బఫరు, &N);
      ఒకవేళ (Matches) {
        std::తిరగేయి_copy(C.Name.begin(), C.Name.చివర(),
                          std::వెనుక_inserter(బఫరు));
        తిరిగి std::make_tuple(N, నిజం, విలువ);
      }
      బిడ్డ_స్థానభ్రంశం += C.కొలత;
      ఒకవేళ (!C.HasSibling)
        ఆపు;
    }
  }
  తిరిగి std::make_tuple(N, అబద్ధం, 0);
}

స్థిర std::tuple<కణుపు, తార్కిక, uint32_t>
compare_కణుపు(uint32_t స్థానభ్రంశం, StringRef Name, తార్కిక Strict, బఫరు_Type &బఫరు) {
  తిరిగి compare_కణుపు(స్థానభ్రంశం, Name, Strict, 0, బఫరు);
}

// clang-format off
constexpr స్థిరరాశి అక్షరం *స్థిరరాశి HangulSyllables[][3] = {
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

స్థిర std::size_t కనుగొను_Syllable(StringRef Name, తార్కిక Strict,
                                అక్షరం &మునుపటి_లోపల_Name, పూర్ణాంకం &Pos, పూర్ణాంకం స్తంభం) {
  ప్రతిజ్ఞా(స్తంభం == 0 || స్తంభం == 1 || స్తంభం == 2);
  స్థిర std::size_t లెక్క_చొప్పున_స్తంభం[] = {LCount, VCount, TCount};
  పూర్ణాంకం Len = -1;
  పూర్ణాంకం Prev = మునుపటి_లోపల_Name;
  కొరకు (std::size_t I = 0; I < లెక్క_చొప్పున_స్తంభం[స్తంభం]; I++) {
    StringRef Syllable(HangulSyllables[I][స్తంభం]);
    ఒకవేళ (పూర్ణాంకం(Syllable.కొలత_2()) <= Len)
      కొనసాగు;
    std::size_t Consummed = 0;
    అక్షరం మునుపటి_లోపల_Name_Copy = మునుపటి_లోపల_Name;
    తార్కిక Does_ప్రారంభం_With =
        startsWith(Name, Syllable, Strict, Consummed, మునుపటి_లోపల_Name_Copy);
    ఒకవేళ (!Does_ప్రారంభం_With)
      కొనసాగు;
    Len = Consummed;
    Pos = I;
    Prev = మునుపటి_లోపల_Name_Copy;
  }
  ఒకవేళ (Len == -1)
    తిరిగి 0;
  మునుపటి_లోపల_Name = Prev;
  తిరిగి size_t(Len);
}

స్థిర std::optional<char32_t>
name_గమ్యం_Hangul_సంకేతం_బిందువు(StringRef Name, తార్కిక Strict, బఫరు_Type &బఫరు) {
  బఫరు.clear();
  // Hangul Syllable Decomposition
  std::size_t Consummed = 0;
  అక్షరం Name_ప్రారంభం = 0;
  తార్కిక Does_ప్రారంభం_With =
      startsWith(Name, "HANGUL SYLLABLE ", Strict, Consummed, Name_ప్రారంభం);
  ఒకవేళ (!Does_ప్రారంభం_With)
    తిరిగి std::nullopt;
  Name = Name.substr(Consummed);
  పూర్ణాంకం L = -1, V = -1, T = -1;
  Name = Name.substr(కనుగొను_Syllable(Name, Strict, Name_ప్రారంభం, L, 0));
  Name = Name.substr(కనుగొను_Syllable(Name, Strict, Name_ప్రారంభం, V, 1));
  Name = Name.substr(కనుగొను_Syllable(Name, Strict, Name_ప్రారంభం, T, 2));
  ఒకవేళ (L != -1 && V != -1 && T != -1 && Name.ఖాళీలు()) {
    ఒకవేళ (!Strict) {
      బఫరు.append("HANGUL SYLLABLE ");
      ఒకవేళ (L != -1)
        బఫరు.append(HangulSyllables[L][0]);
      ఒకవేళ (V != -1)
        బఫరు.append(HangulSyllables[V][1]);
      ఒకవేళ (T != -1)
        బఫరు.append(HangulSyllables[T][2]);
    }
    తిరిగి SBase + (std::uint32_t(L) * VCount + std::uint32_t(V)) * TCount +
           std::uint32_t(T);
  }
  // Otherwise, it's an illegal syllable name.
  తిరిగి std::nullopt;
}

నిర్మాణం Generated_పేర్లు_దత్తాంశం {
  StringRef ఉపసర్గ;
  uint32_t ప్రారంభం;
  uint32_t చివర_2;
};

// Unicode 18.0 Name Derivation Rule Prefix Strings (extracted/DerivedName.txt)
స్థిర స్థిరరాశి Generated_పేర్లు_దత్తాంశం Generated_పేర్లు_దత్తాంశం_పట్టిక[] = {
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

స్థిర std::optional<char32_t>
name_గమ్యం_Generated_సంకేతం_బిందువు(StringRef Name, తార్కిక Strict, బఫరు_Type &బఫరు) {
  కొరకు (స్వయంచాలక &&అంశం : Generated_పేర్లు_దత్తాంశం_పట్టిక) {
    బఫరు.clear();
    std::size_t Consummed = 0;
    అక్షరం Name_ప్రారంభం = 0;
    తార్కిక Does_ప్రారంభం_With = startsWith(Name, అంశం.ఉపసర్గ, Strict, Consummed,
                                    Name_ప్రారంభం, /*IsPrefix=*/నిజం);
    ఒకవేళ (!Does_ప్రారంభం_With)
      కొనసాగు;
    స్వయంచాలక సంఖ్య = Name.substr(Consummed);
    గుర్తులేని దీర్ఘ దీర్ఘ V = 0;
    // Be consistent about mandating upper casing.
    ఒకవేళ (Strict &&
        llvm::any_యొక్క(సంఖ్య, [](అక్షరం C) { తిరిగి C >= 'a' && C <= 'f'; }))
      తిరిగి {};
    ఒకవేళ (getAsUnsignedInteger(సంఖ్య, 16, V) || V < అంశం.ప్రారంభం || V > అంశం.చివర_2)
      కొనసాగు;
    ఒకవేళ (!Strict) {
      బఫరు.append(అంశం.ఉపసర్గ);
      బఫరు.append(utohexstr(V, నిజం));
    }
    తిరిగి V;
  }
  తిరిగి std::nullopt;
}

స్థిర std::optional<char32_t> name_గమ్యం_Codepoint(StringRef Name, తార్కిక Strict,
                                               బఫరు_Type &బఫరు) {
  ఒకవేళ (Name.ఖాళీలు())
    తిరిగి std::nullopt;

  std::optional<char32_t> Res = name_గమ్యం_Hangul_సంకేతం_బిందువు(Name, Strict, బఫరు);
  ఒకవేళ (!Res)
    Res = name_గమ్యం_Generated_సంకేతం_బిందువు(Name, Strict, బఫరు);
  ఒకవేళ (Res)
    తిరిగి *Res;

  బఫరు.clear();
  కణుపు కణుపు;
  తార్కిక Matches;
  uint32_t విలువ;
  std::tie(కణుపు, Matches, విలువ) = compare_కణుపు(0, Name, Strict, బఫరు);
  ఒకవేళ (Matches) {
    std::తిరగేయి(బఫరు.begin(), బఫరు.చివర());
    // UAX44-LM2. Ignore case, whitespace, underscore ('_'), and all medial
    // hyphens except the hyphen in U+1180 HANGUL JUNGSEONG O-E.
    ఒకవేళ (!Strict && విలువ == 0x116c && Name.కలిగియుంది_insensitive("O-E")) {
      బఫరు = "HANGUL JUNGSEONG O-E";
      విలువ = 0x1180;
    }
    తిరిగి విలువ;
  }
  తిరిగి std::nullopt;
}

std::optional<char32_t> name_గమ్యం_Codepoint_Strict(StringRef Name) {

  బఫరు_Type బఫరు;
  స్వయంచాలక Opt = name_గమ్యం_Codepoint(Name, నిజం, బఫరు);
  తిరిగి Opt;
}

std::optional<Loose_Matching_ఫలితం>
name_గమ్యం_Codepoint_Loose_Matching(StringRef Name) {
  బఫరు_Type బఫరు;
  స్వయంచాలక Opt = name_గమ్యం_Codepoint(Name, అబద్ధం, బఫరు);
  ఒకవేళ (!Opt)
    తిరిగి std::nullopt;
  తిరిగి Loose_Matching_ఫలితం{*Opt, బఫరు};
}

// Find the unicode character whose editing distance to Pattern
// is shortest, using the Wagner–Fischer algorithm.
llvm::Small_సదిశ<సరిపోలిక_For_Codepoint_Name>
సమీప_Matches_For_Codepoint_Name(StringRef నమూనా, std::size_t Max_Matches_లెక్క) {
  // We maintain a fixed size vector of matches,
  // sorted by distance
  // The worst match (with the biggest distance) are discarded when new elements
  // are added.
  std::size_t అతిపెద్ద_Edit_దూరం = 0;
  llvm::Small_సదిశ<సరిపోలిక_For_Codepoint_Name> Matches;
  Matches.reserve(Max_Matches_లెక్క + 1);

  స్వయంచాలక చొప్పించు_2 = [&](స్థిరరాశి కణుపు &కణుపు, uint32_t దూరం_2,
                    char32_t విలువ) -> తార్కిక {
    ఒకవేళ (దూరం_2 > అతిపెద్ద_Edit_దూరం) {
      ఒకవేళ (Matches.కొలత_2() == Max_Matches_లెక్క)
        తిరిగి అబద్ధం;
      అతిపెద్ద_Edit_దూరం = దూరం_2;
    }
    // To avoid allocations, the creation of the name is delayed
    // as much as possible.
    std::string Name;
    స్వయంచాలక GetName = [&] {
      ఒకవేళ (Name.ఖాళీలు())
        Name = కణుపు.నిండిన_Name();
      తిరిగి Name;
    };

    స్వయంచాలక It = llvm::క్రింది_bound(
        Matches, దూరం_2,
        [&](స్థిరరాశి సరిపోలిక_For_Codepoint_Name &a, std::size_t దూరం_2) {
          ఒకవేళ (దూరం_2 == a.దూరం_2)
            తిరిగి a.Name < GetName();
          తిరిగి a.దూరం_2 < దూరం_2;
        });
    ఒకవేళ (It == Matches.చివర() && Matches.కొలత_2() == Max_Matches_లెక్క)
      తిరిగి అబద్ధం;

    సరిపోలిక_For_Codepoint_Name M{GetName(), దూరం_2, విలువ};
    Matches.చొప్పించు(It, std::move(M));
    ఒకవేళ (Matches.కొలత_2() > Max_Matches_లెక్క)
      Matches.తీయి_వెనుక();
    తిరిగి నిజం;
  };

  // We ignore case, space, hyphens, etc,
  // in both the search pattern and the prospective names.
  స్వయంచాలక Normalize = [](StringRef Name) {
    std::string బహిర్గతం;
    బహిర్గతం.reserve(Name.కొలత_2());
    కొరకు (అక్షరం C : Name) {
      ఒకవేళ (isAlnum(C))
        బహిర్గతం.నెట్టు_వెనుక(గమ్యం_పై(C));
    }
    తిరిగి బహిర్గతం;
  };
  std::string NormalizedName = Normalize(నమూనా);

  // Allocate a matrix big enough for longest names.
  స్థిరరాశి std::size_t Columns =
      std::min(NormalizedName.కొలత_2(), Unicode_Name_గమ్యం_Codepoint_అతిపెద్ద_Name_కొలత) +
      1;

  [[maybe_వాడని]] స్థిర std::size_t అడ్డువరుసలు_2 =
      Unicode_Name_గమ్యం_Codepoint_అతిపెద్ద_Name_కొలత + 1;

  std::సదిశ<అక్షరం> Distances(
      Columns * (Unicode_Name_గమ్యం_Codepoint_అతిపెద్ద_Name_కొలత + 1), 0);

  స్వయంచాలక Get = [&Distances, Columns](size_t స్తంభం, std::size_t అడ్డువరుస) -> అక్షరం & {
    ప్రతిజ్ఞా(స్తంభం < Columns);
    ప్రతిజ్ఞా(అడ్డువరుస < అడ్డువరుసలు_2);
    తిరిగి Distances[అడ్డువరుస * Columns + స్తంభం];
  };

  కొరకు (std::size_t I = 0; I < Columns; I++)
    Get(I, 0) = I;

  // Visit the childrens,
  // Filling (and overriding) the matrix for the name fragment of each node
  // iteratively. CompleteName is used to collect the actual name of potential
  // match, respecting case and spacing.
  స్వయంచాలక సందర్శించు_కణుపు = [&](స్థిరరాశి కణుపు &N, std::size_t అడ్డువరుస,
                       స్వయంచాలక &సందర్శించు_కణుపు) -> శూన్య {
    std::size_t J = 0;
    కొరకు (; J < N.Name.కొలత_2(); J++) {
      ఒకవేళ (!isAlnum(N.Name[J]))
        కొనసాగు;

      Get(0, అడ్డువరుస) = అడ్డువరుస;

      కొరకు (std::size_t I = 1; I < Columns; I++) {
        స్థిరరాశి పూర్ణాంకం Delete = Get(I - 1, అడ్డువరుస) + 1;
        స్థిరరాశి పూర్ణాంకం చొప్పించు_2 = Get(I, అడ్డువరుస - 1) + 1;

        స్థిరరాశి పూర్ణాంకం Replace =
            Get(I - 1, అడ్డువరుస - 1) + (NormalizedName[I - 1] != N.Name[J] ? 1 : 0);

        Get(I, అడ్డువరుస) = std::min({చొప్పించు_2, Delete, Replace});
      }

      అడ్డువరుస++;
    }

    గుర్తులేని ఖర్చు = Get(Columns - 1, అడ్డువరుస - 1);
    ఒకవేళ (N.విలువ != 0xFFFFFFFF) {
      చొప్పించు_2(N, ఖర్చు, N.విలువ);
    }

    ఒకవేళ (N.hasChildren()) {
      స్వయంచాలక బిడ్డ_స్థానభ్రంశం = N.Children_స్థానభ్రంశం;
      కొరకు (;;) {
        కణుపు C = read_కణుపు(బిడ్డ_స్థానభ్రంశం, &N);
        బిడ్డ_స్థానభ్రంశం += C.కొలత;
        ఒకవేళ (!C.isValid())
          ఆపు;
        సందర్శించు_కణుపు(C, అడ్డువరుస, సందర్శించు_కణుపు);
        ఒకవేళ (!C.HasSibling)
          ఆపు;
      }
    }
  };

  కణుపు మూలం_2 = create_మూలం();
  సందర్శించు_కణుపు(మూలం_2, 1, సందర్శించు_కణుపు);
  తిరిగి Matches;
}

} // namespace unicode

} // namespace sys
} // namespace llvm
