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

#അന്തര്ഭാവ "llvm/ADT/STLExtras.h"
#അന്തര്ഭാവ "llvm/ADT/StringExtras.h"
#അന്തര്ഭാവ "llvm/ADT/StringRef.h"
#അന്തര്ഭാവ "llvm/Support/Unicode.h"

നാമാകാശ llvm {
നാമാകാശ sys {
നാമാകാശ unicode {

ബാഹ്യ സ്ഥിരം അക്ഷരം *സ്ഥിരം UnicodeNameToCodepointDict;
ബാഹ്യ സ്ഥിരം uint8_t *സ്ഥിരം UnicodeNameToCodepointIndex;
ബാഹ്യ സ്ഥിരം std::size_t UnicodeNameToCodepointIndexSize;
ബാഹ്യ സ്ഥിരം std::size_t UnicodeNameToCodepointLargestNameSize;

using BufferType = SmallString<64>;

ഘടന കണ്ണി {
  തർക്കം Is_വേര് = അസത്യം;
  char32_t മൂല്യം = 0xFFFFFFFF;
  uint32_t ChildrenOffset = 0;
  തർക്കം HasSibling = അസത്യം;
  uint32_t Size = 0;
  StringRef Name;
  സ്ഥിരം കണ്ണി *Parent = nullptr;

  constexpr തർക്കം isValid() സ്ഥിരം {
    തിരികെ !Name.empty() || മൂല്യം == 0xFFFFFFFF;
  }
  constexpr തർക്കം hasChildren() സ്ഥിരം { തിരികെ ChildrenOffset != 0 || Is_വേര്; }

  std::string fullName() സ്ഥിരം {
    std::string S;
    // Reserve enough space for most unicode code points.
    // The chosen value represent the 99th percentile of name size as of
    // Unicode 15.0.
    S.reserve(46);
    സ്ഥിരം കണ്ണി *N = ഇത്;
    ആകുമ്പോൾ (N) {
      std::reverse_copy(N->Name.begin(), N->Name.end(), std::back_inserter(S));
      N = N->Parent;
    }
    std::reverse(S.begin(), S.end());
    തിരികെ S;
  }
};

സ്ഥിതം കണ്ണി create_വേര്() {
  കണ്ണി N;
  N.Is_വേര് = സത്യം;
  N.ChildrenOffset = 1;
  N.Size = 1;
  തിരികെ N;
}

സ്ഥിതം കണ്ണി read_കണ്ണി(uint32_t Offset, സ്ഥിരം കണ്ണി *Parent = nullptr) {
  എങ്കിൽ (Offset == 0)
    തിരികെ create_വേര്();

  uint32_t Origin = Offset;
  കണ്ണി N;
  N.Parent = Parent;
  uint8_t NameInfo = UnicodeNameToCodepointIndex[Offset++];
  എങ്കിൽ (Offset + 6 >= UnicodeNameToCodepointIndexSize)
    തിരികെ N;

  തർക്കം LongName = NameInfo & 0x40;
  തർക്കം Has_മൂല്യം = NameInfo & 0x80;
  std::size_t Size = NameInfo & ~0xC0;
  എങ്കിൽ (LongName) {
    uint32_t NameOffset = (UnicodeNameToCodepointIndex[Offset++] << 8);
    NameOffset |= UnicodeNameToCodepointIndex[Offset++];
    N.Name = StringRef(UnicodeNameToCodepointDict + NameOffset, Size);
  } അല്ലെങ്കിൽ {
    N.Name = StringRef(UnicodeNameToCodepointDict + Size, 1);
  }
  എങ്കിൽ (Has_മൂല്യം) {
    uint8_t H = UnicodeNameToCodepointIndex[Offset++];
    uint8_t M = UnicodeNameToCodepointIndex[Offset++];
    uint8_t L = UnicodeNameToCodepointIndex[Offset++];
    N.മൂല്യം = ((H << 16) | (M << 8) | L) >> 3;

    തർക്കം HasChildren = L & 0x02;
    N.HasSibling = L & 0x01;

    എങ്കിൽ (HasChildren) {
      N.ChildrenOffset = UnicodeNameToCodepointIndex[Offset++] << 16;
      N.ChildrenOffset |= UnicodeNameToCodepointIndex[Offset++] << 8;
      N.ChildrenOffset |= UnicodeNameToCodepointIndex[Offset++];
    }
  } അല്ലെങ്കിൽ {
    uint8_t H = UnicodeNameToCodepointIndex[Offset++];
    N.HasSibling = H & 0x80;
    തർക്കം HasChildren = H & 0x40;
    H &= uint8_t(~0xC0);
    എങ്കിൽ (HasChildren) {
      N.ChildrenOffset = (H << 16);
      N.ChildrenOffset |=
          (uint32_t(UnicodeNameToCodepointIndex[Offset++]) << 8);
      N.ChildrenOffset |= UnicodeNameToCodepointIndex[Offset++];
    }
  }
  N.Size = Offset - Origin;
  തിരികെ N;
}

സ്ഥിതം തർക്കം startsWith(StringRef Name, StringRef Needle, തർക്കം Strict,
                       std::size_t &Consummed, അക്ഷരം &PreviousCharInName,
                       തർക്കം IsPrefix = അസത്യം) {

  Consummed = 0;
  എങ്കിൽ (Strict) {
    എങ്കിൽ (!Name.starts_with(Needle))
      തിരികെ അസത്യം;
    Consummed = Needle.size();
    തിരികെ സത്യം;
  }
  എങ്കിൽ (Needle.empty())
    തിരികെ സത്യം;

  auto NamePos = Name.begin();
  auto NeedlePos = Needle.begin();

  അക്ഷരം PreviousCharInNameOrigin = PreviousCharInName;
  അക്ഷരം PreviousCharInNeedle = *Needle.begin();
  auto IgnoreSpaces = [](auto It, auto End, അക്ഷരം &PreviousChar,
                         തർക്കം IsPrefix = അസത്യം) {
    ആകുമ്പോൾ (It != End) {
      സ്ഥിരം auto Next = std::next(It);
      // Ignore spaces, underscore, medial hyphens
      // The generator ensures a needle never ends (or starts) by a medial
      // hyphen https://unicode.org/reports/tr44/#UAX44-LM2.
      തർക്കം Ignore =
          *It == ' ' || *It == '_' ||
          (*It == '-' && isAlnum(PreviousChar) &&
           ((Next != End && isAlnum(*Next)) || (Next == End && IsPrefix)));
      PreviousChar = *It;
      എങ്കിൽ (!Ignore)
        നിർത്തുക;
      ++It;
    }
    തിരികെ It;
  };

  ആകുമ്പോൾ (സത്യം) {
    NamePos = IgnoreSpaces(NamePos, Name.end(), PreviousCharInName);
    NeedlePos =
        IgnoreSpaces(NeedlePos, Needle.end(), PreviousCharInNeedle, IsPrefix);
    എങ്കിൽ (NeedlePos == Needle.end())
      നിർത്തുക;
    എങ്കിൽ (NamePos == Name.end())
      നിർത്തുക;
    എങ്കിൽ (toUpper(*NeedlePos) != toUpper(*NamePos))
      നിർത്തുക;
    NeedlePos++;
    NamePos++;
  }
  Consummed = std::distance(Name.begin(), NamePos);
  എങ്കിൽ (NeedlePos != Needle.end()) {
    PreviousCharInName = PreviousCharInNameOrigin;
  }
  തിരികെ NeedlePos == Needle.end();
}

സ്ഥിതം std::tuple<കണ്ണി, തർക്കം, uint32_t>
compare_കണ്ണി(uint32_t Offset, StringRef Name, തർക്കം Strict,
            അക്ഷരം PreviousCharInName, BufferType &Buffer,
            സ്ഥിരം കണ്ണി *Parent = nullptr) {
  കണ്ണി N = read_കണ്ണി(Offset, Parent);
  std::size_t Consummed = 0;
  തർക്കം DoesStartWith = N.Is_വേര് || startsWith(Name, N.Name, Strict, Consummed,
                                              PreviousCharInName);
  എങ്കിൽ (!DoesStartWith)
    തിരികെ std::make_tuple(N, അസത്യം, 0);

  എങ്കിൽ (Name.size() - Consummed == 0 && N.മൂല്യം != 0xFFFFFFFF)
    തിരികെ std::make_tuple(N, സത്യം, N.മൂല്യം);

  എങ്കിൽ (N.hasChildren()) {
    uint32_t ChildOffset = N.ChildrenOffset;
    വേണ്ടി (;;) {
      കണ്ണി C;
      തർക്കം Matches;
      uint32_t മൂല്യം;
      std::tie(C, Matches, മൂല്യം) =
          compare_കണ്ണി(ChildOffset, Name.substr(Consummed), Strict,
                      PreviousCharInName, Buffer, &N);
      എങ്കിൽ (Matches) {
        std::reverse_copy(C.Name.begin(), C.Name.end(),
                          std::back_inserter(Buffer));
        തിരികെ std::make_tuple(N, സത്യം, മൂല്യം);
      }
      ChildOffset += C.Size;
      എങ്കിൽ (!C.HasSibling)
        നിർത്തുക;
    }
  }
  തിരികെ std::make_tuple(N, അസത്യം, 0);
}

സ്ഥിതം std::tuple<കണ്ണി, തർക്കം, uint32_t>
compare_കണ്ണി(uint32_t Offset, StringRef Name, തർക്കം Strict, BufferType &Buffer) {
  തിരികെ compare_കണ്ണി(Offset, Name, Strict, 0, Buffer);
}

// clang-format off
constexpr സ്ഥിരം അക്ഷരം *സ്ഥിരം HangulSyllables[][3] = {
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

സ്ഥിതം std::size_t findSyllable(StringRef Name, തർക്കം Strict,
                                അക്ഷരം &PreviousInName, പൂർണ്ണസംഖ്യ &Pos, പൂർണ്ണസംഖ്യ Column) {
  പ്രതിജ്ഞാ(Column == 0 || Column == 1 || Column == 2);
  സ്ഥിതം std::size_t എണ്ണം_Per_Column[] = {LCount, VCount, TCount};
  പൂർണ്ണസംഖ്യ Len = -1;
  പൂർണ്ണസംഖ്യ Prev = PreviousInName;
  വേണ്ടി (std::size_t I = 0; I < എണ്ണം_Per_Column[Column]; I++) {
    StringRef Syllable(HangulSyllables[I][Column]);
    എങ്കിൽ (പൂർണ്ണസംഖ്യ(Syllable.size()) <= Len)
      തുടരുക;
    std::size_t Consummed = 0;
    അക്ഷരം PreviousInNameCopy = PreviousInName;
    തർക്കം DoesStartWith =
        startsWith(Name, Syllable, Strict, Consummed, PreviousInNameCopy);
    എങ്കിൽ (!DoesStartWith)
      തുടരുക;
    Len = Consummed;
    Pos = I;
    Prev = PreviousInNameCopy;
  }
  എങ്കിൽ (Len == -1)
    തിരികെ 0;
  PreviousInName = Prev;
  തിരികെ size_t(Len);
}

സ്ഥിതം std::optional<char32_t>
nameToHangulCodePoint(StringRef Name, തർക്കം Strict, BufferType &Buffer) {
  Buffer.clear();
  // Hangul Syllable Decomposition
  std::size_t Consummed = 0;
  അക്ഷരം NameStart = 0;
  തർക്കം DoesStartWith =
      startsWith(Name, "HANGUL SYLLABLE ", Strict, Consummed, NameStart);
  എങ്കിൽ (!DoesStartWith)
    തിരികെ std::nullopt;
  Name = Name.substr(Consummed);
  പൂർണ്ണസംഖ്യ L = -1, V = -1, T = -1;
  Name = Name.substr(findSyllable(Name, Strict, NameStart, L, 0));
  Name = Name.substr(findSyllable(Name, Strict, NameStart, V, 1));
  Name = Name.substr(findSyllable(Name, Strict, NameStart, T, 2));
  എങ്കിൽ (L != -1 && V != -1 && T != -1 && Name.empty()) {
    എങ്കിൽ (!Strict) {
      Buffer.append("HANGUL SYLLABLE ");
      എങ്കിൽ (L != -1)
        Buffer.append(HangulSyllables[L][0]);
      എങ്കിൽ (V != -1)
        Buffer.append(HangulSyllables[V][1]);
      എങ്കിൽ (T != -1)
        Buffer.append(HangulSyllables[T][2]);
    }
    തിരികെ SBase + (std::uint32_t(L) * VCount + std::uint32_t(V)) * TCount +
           std::uint32_t(T);
  }
  // Otherwise, it's an illegal syllable name.
  തിരികെ std::nullopt;
}

ഘടന GeneratedNamesData {
  StringRef Prefix;
  uint32_t Start;
  uint32_t End;
};

// Unicode 18.0 Name Derivation Rule Prefix Strings (extracted/DerivedName.txt)
സ്ഥിതം സ്ഥിരം GeneratedNamesData GeneratedNamesDataTable[] = {
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

സ്ഥിതം std::optional<char32_t>
nameToGeneratedCodePoint(StringRef Name, തർക്കം Strict, BufferType &Buffer) {
  വേണ്ടി (auto &&Item : GeneratedNamesDataTable) {
    Buffer.clear();
    std::size_t Consummed = 0;
    അക്ഷരം NameStart = 0;
    തർക്കം DoesStartWith = startsWith(Name, Item.Prefix, Strict, Consummed,
                                    NameStart, /*IsPrefix=*/സത്യം);
    എങ്കിൽ (!DoesStartWith)
      തുടരുക;
    auto സംഖ്യ = Name.substr(Consummed);
    unsigned long long V = 0;
    // Be consistent about mandating upper casing.
    എങ്കിൽ (Strict &&
        llvm::any_of(സംഖ്യ, [](അക്ഷരം C) { തിരികെ C >= 'a' && C <= 'f'; }))
      തിരികെ {};
    എങ്കിൽ (getAsUnsignedInteger(സംഖ്യ, 16, V) || V < Item.Start || V > Item.End)
      തുടരുക;
    എങ്കിൽ (!Strict) {
      Buffer.append(Item.Prefix);
      Buffer.append(utohexstr(V, സത്യം));
    }
    തിരികെ V;
  }
  തിരികെ std::nullopt;
}

സ്ഥിതം std::optional<char32_t> nameToCodepoint(StringRef Name, തർക്കം Strict,
                                               BufferType &Buffer) {
  എങ്കിൽ (Name.empty())
    തിരികെ std::nullopt;

  std::optional<char32_t> Res = nameToHangulCodePoint(Name, Strict, Buffer);
  എങ്കിൽ (!Res)
    Res = nameToGeneratedCodePoint(Name, Strict, Buffer);
  എങ്കിൽ (Res)
    തിരികെ *Res;

  Buffer.clear();
  കണ്ണി കണ്ണി;
  തർക്കം Matches;
  uint32_t മൂല്യം;
  std::tie(കണ്ണി, Matches, മൂല്യം) = compare_കണ്ണി(0, Name, Strict, Buffer);
  എങ്കിൽ (Matches) {
    std::reverse(Buffer.begin(), Buffer.end());
    // UAX44-LM2. Ignore case, whitespace, underscore ('_'), and all medial
    // hyphens except the hyphen in U+1180 HANGUL JUNGSEONG O-E.
    എങ്കിൽ (!Strict && മൂല്യം == 0x116c && Name.contains_insensitive("O-E")) {
      Buffer = "HANGUL JUNGSEONG O-E";
      മൂല്യം = 0x1180;
    }
    തിരികെ മൂല്യം;
  }
  തിരികെ std::nullopt;
}

std::optional<char32_t> nameToCodepointStrict(StringRef Name) {

  BufferType Buffer;
  auto Opt = nameToCodepoint(Name, സത്യം, Buffer);
  തിരികെ Opt;
}

std::optional<LooseMatchingResult>
nameToCodepointLooseMatching(StringRef Name) {
  BufferType Buffer;
  auto Opt = nameToCodepoint(Name, അസത്യം, Buffer);
  എങ്കിൽ (!Opt)
    തിരികെ std::nullopt;
  തിരികെ LooseMatchingResult{*Opt, Buffer};
}

// Find the unicode character whose editing distance to Pattern
// is shortest, using the Wagner–Fischer algorithm.
llvm::SmallVector<MatchForCodepointName>
nearestMatchesForCodepointName(StringRef Pattern, std::size_t Max_Matches_എണ്ണം) {
  // We maintain a fixed size vector of matches,
  // sorted by distance
  // The worst match (with the biggest distance) are discarded when new elements
  // are added.
  std::size_t LargestEditDistance = 0;
  llvm::SmallVector<MatchForCodepointName> Matches;
  Matches.reserve(Max_Matches_എണ്ണം + 1);

  auto തിരുകുക_2 = [&](സ്ഥിരം കണ്ണി &കണ്ണി, uint32_t Distance,
                    char32_t മൂല്യം) -> തർക്കം {
    എങ്കിൽ (Distance > LargestEditDistance) {
      എങ്കിൽ (Matches.size() == Max_Matches_എണ്ണം)
        തിരികെ അസത്യം;
      LargestEditDistance = Distance;
    }
    // To avoid allocations, the creation of the name is delayed
    // as much as possible.
    std::string Name;
    auto GetName = [&] {
      എങ്കിൽ (Name.empty())
        Name = കണ്ണി.fullName();
      തിരികെ Name;
    };

    auto It = llvm::lower_bound(
        Matches, Distance,
        [&](സ്ഥിരം MatchForCodepointName &a, std::size_t Distance) {
          എങ്കിൽ (Distance == a.Distance)
            തിരികെ a.Name < GetName();
          തിരികെ a.Distance < Distance;
        });
    എങ്കിൽ (It == Matches.end() && Matches.size() == Max_Matches_എണ്ണം)
      തിരികെ അസത്യം;

    MatchForCodepointName M{GetName(), Distance, മൂല്യം};
    Matches.തിരുകുക(It, std::move(M));
    എങ്കിൽ (Matches.size() > Max_Matches_എണ്ണം)
      Matches.എടുക്കുക_back();
    തിരികെ സത്യം;
  };

  // We ignore case, space, hyphens, etc,
  // in both the search pattern and the prospective names.
  auto Normalize = [](StringRef Name) {
    std::string Out;
    Out.reserve(Name.size());
    വേണ്ടി (അക്ഷരം C : Name) {
      എങ്കിൽ (isAlnum(C))
        Out.തള്ളുക_back(toUpper(C));
    }
    തിരികെ Out;
  };
  std::string NormalizedName = Normalize(Pattern);

  // Allocate a matrix big enough for longest names.
  സ്ഥിരം std::size_t Columns =
      std::min(NormalizedName.size(), UnicodeNameToCodepointLargestNameSize) +
      1;

  [[maybe_unused]] സ്ഥിതം std::size_t Rows =
      UnicodeNameToCodepointLargestNameSize + 1;

  std::vector<അക്ഷരം> Distances(
      Columns * (UnicodeNameToCodepointLargestNameSize + 1), 0);

  auto Get = [&Distances, Columns](size_t Column, std::size_t Row) -> അക്ഷരം & {
    പ്രതിജ്ഞാ(Column < Columns);
    പ്രതിജ്ഞാ(Row < Rows);
    തിരികെ Distances[Row * Columns + Column];
  };

  വേണ്ടി (std::size_t I = 0; I < Columns; I++)
    Get(I, 0) = I;

  // Visit the childrens,
  // Filling (and overriding) the matrix for the name fragment of each node
  // iteratively. CompleteName is used to collect the actual name of potential
  // match, respecting case and spacing.
  auto Visit_കണ്ണി = [&](സ്ഥിരം കണ്ണി &N, std::size_t Row,
                       auto &Visit_കണ്ണി) -> ശൂന്യം {
    std::size_t J = 0;
    വേണ്ടി (; J < N.Name.size(); J++) {
      എങ്കിൽ (!isAlnum(N.Name[J]))
        തുടരുക;

      Get(0, Row) = Row;

      വേണ്ടി (std::size_t I = 1; I < Columns; I++) {
        സ്ഥിരം പൂർണ്ണസംഖ്യ Delete = Get(I - 1, Row) + 1;
        സ്ഥിരം പൂർണ്ണസംഖ്യ തിരുകുക_2 = Get(I, Row - 1) + 1;

        സ്ഥിരം പൂർണ്ണസംഖ്യ Replace =
            Get(I - 1, Row - 1) + (NormalizedName[I - 1] != N.Name[J] ? 1 : 0);

        Get(I, Row) = std::min({തിരുകുക_2, Delete, Replace});
      }

      Row++;
    }

    unsigned Cost = Get(Columns - 1, Row - 1);
    എങ്കിൽ (N.മൂല്യം != 0xFFFFFFFF) {
      തിരുകുക_2(N, Cost, N.മൂല്യം);
    }

    എങ്കിൽ (N.hasChildren()) {
      auto ChildOffset = N.ChildrenOffset;
      വേണ്ടി (;;) {
        കണ്ണി C = read_കണ്ണി(ChildOffset, &N);
        ChildOffset += C.Size;
        എങ്കിൽ (!C.isValid())
          നിർത്തുക;
        Visit_കണ്ണി(C, Row, Visit_കണ്ണി);
        എങ്കിൽ (!C.HasSibling)
          നിർത്തുക;
      }
    }
  };

  കണ്ണി വേര്_2 = create_വേര്();
  Visit_കണ്ണി(വേര്_2, 1, Visit_കണ്ണി);
  തിരികെ Matches;
}

} // namespace unicode

} // namespace sys
} // namespace llvm
