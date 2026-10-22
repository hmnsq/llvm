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

#અન્તર્ભાવ "llvm/ADT/STLExtras.h"
#અન્તર્ભાવ "llvm/ADT/StringExtras.h"
#અન્તર્ભાવ "llvm/ADT/StringRef.h"
#અન્તર્ભાવ "llvm/Support/Unicode.h"

નામાકાશ llvm {
નામાકાશ sys {
નામાકાશ unicode {

બાહ્ય અચલ અક્ષર *અચલ UnicodeNameToCodepointDict;
બાહ્ય અચલ uint8_t *અચલ UnicodeNameToCodepointIndex;
બાહ્ય અચલ std::size_t UnicodeNameToCodepointIndexSize;
બાહ્ય અચલ std::size_t UnicodeNameToCodepointLargestNameSize;

using BufferType = SmallString<64>;

સંરચના ગાંઠ {
  તાર્કિક Is_મૂળ = ખોટું;
  char32_t કિંમત = 0xFFFFFFFF;
  uint32_t ChildrenOffset = 0;
  તાર્કિક HasSibling = ખોટું;
  uint32_t Size = 0;
  StringRef Name;
  અચલ ગાંઠ *Parent = nullptr;

  constexpr તાર્કિક isValid() અચલ {
    પાછા !Name.empty() || કિંમત == 0xFFFFFFFF;
  }
  constexpr તાર્કિક hasChildren() અચલ { પાછા ChildrenOffset != 0 || Is_મૂળ; }

  std::string fullName() અચલ {
    std::string S;
    // Reserve enough space for most unicode code points.
    // The chosen value represent the 99th percentile of name size as of
    // Unicode 15.0.
    S.reserve(46);
    અચલ ગાંઠ *N = આ;
    જ્યારે (N) {
      std::reverse_copy(N->Name.begin(), N->Name.end(), std::back_inserter(S));
      N = N->Parent;
    }
    std::reverse(S.begin(), S.end());
    પાછા S;
  }
};

સ્થિર ગાંઠ create_મૂળ() {
  ગાંઠ N;
  N.Is_મૂળ = સાચું;
  N.ChildrenOffset = 1;
  N.Size = 1;
  પાછા N;
}

સ્થિર ગાંઠ read_ગાંઠ(uint32_t Offset, અચલ ગાંઠ *Parent = nullptr) {
  જો (Offset == 0)
    પાછા create_મૂળ();

  uint32_t Origin = Offset;
  ગાંઠ N;
  N.Parent = Parent;
  uint8_t NameInfo = UnicodeNameToCodepointIndex[Offset++];
  જો (Offset + 6 >= UnicodeNameToCodepointIndexSize)
    પાછા N;

  તાર્કિક LongName = NameInfo & 0x40;
  તાર્કિક Has_કિંમત = NameInfo & 0x80;
  std::size_t Size = NameInfo & ~0xC0;
  જો (LongName) {
    uint32_t NameOffset = (UnicodeNameToCodepointIndex[Offset++] << 8);
    NameOffset |= UnicodeNameToCodepointIndex[Offset++];
    N.Name = StringRef(UnicodeNameToCodepointDict + NameOffset, Size);
  } નહીંતર {
    N.Name = StringRef(UnicodeNameToCodepointDict + Size, 1);
  }
  જો (Has_કિંમત) {
    uint8_t H = UnicodeNameToCodepointIndex[Offset++];
    uint8_t M = UnicodeNameToCodepointIndex[Offset++];
    uint8_t L = UnicodeNameToCodepointIndex[Offset++];
    N.કિંમત = ((H << 16) | (M << 8) | L) >> 3;

    તાર્કિક HasChildren = L & 0x02;
    N.HasSibling = L & 0x01;

    જો (HasChildren) {
      N.ChildrenOffset = UnicodeNameToCodepointIndex[Offset++] << 16;
      N.ChildrenOffset |= UnicodeNameToCodepointIndex[Offset++] << 8;
      N.ChildrenOffset |= UnicodeNameToCodepointIndex[Offset++];
    }
  } નહીંતર {
    uint8_t H = UnicodeNameToCodepointIndex[Offset++];
    N.HasSibling = H & 0x80;
    તાર્કિક HasChildren = H & 0x40;
    H &= uint8_t(~0xC0);
    જો (HasChildren) {
      N.ChildrenOffset = (H << 16);
      N.ChildrenOffset |=
          (uint32_t(UnicodeNameToCodepointIndex[Offset++]) << 8);
      N.ChildrenOffset |= UnicodeNameToCodepointIndex[Offset++];
    }
  }
  N.Size = Offset - Origin;
  પાછા N;
}

સ્થિર તાર્કિક startsWith(StringRef Name, StringRef Needle, તાર્કિક Strict,
                       std::size_t &Consummed, અક્ષર &PreviousCharInName,
                       તાર્કિક IsPrefix = ખોટું) {

  Consummed = 0;
  જો (Strict) {
    જો (!Name.starts_with(Needle))
      પાછા ખોટું;
    Consummed = Needle.size();
    પાછા સાચું;
  }
  જો (Needle.empty())
    પાછા સાચું;

  auto NamePos = Name.begin();
  auto NeedlePos = Needle.begin();

  અક્ષર PreviousCharInNameOrigin = PreviousCharInName;
  અક્ષર PreviousCharInNeedle = *Needle.begin();
  auto IgnoreSpaces = [](auto It, auto End, અક્ષર &PreviousChar,
                         તાર્કિક IsPrefix = ખોટું) {
    જ્યારે (It != End) {
      અચલ auto Next = std::next(It);
      // Ignore spaces, underscore, medial hyphens
      // The generator ensures a needle never ends (or starts) by a medial
      // hyphen https://unicode.org/reports/tr44/#UAX44-LM2.
      તાર્કિક Ignore =
          *It == ' ' || *It == '_' ||
          (*It == '-' && isAlnum(PreviousChar) &&
           ((Next != End && isAlnum(*Next)) || (Next == End && IsPrefix)));
      PreviousChar = *It;
      જો (!Ignore)
        તોડો;
      ++It;
    }
    પાછા It;
  };

  જ્યારે (સાચું) {
    NamePos = IgnoreSpaces(NamePos, Name.end(), PreviousCharInName);
    NeedlePos =
        IgnoreSpaces(NeedlePos, Needle.end(), PreviousCharInNeedle, IsPrefix);
    જો (NeedlePos == Needle.end())
      તોડો;
    જો (NamePos == Name.end())
      તોડો;
    જો (toUpper(*NeedlePos) != toUpper(*NamePos))
      તોડો;
    NeedlePos++;
    NamePos++;
  }
  Consummed = std::distance(Name.begin(), NamePos);
  જો (NeedlePos != Needle.end()) {
    PreviousCharInName = PreviousCharInNameOrigin;
  }
  પાછા NeedlePos == Needle.end();
}

સ્થિર std::tuple<ગાંઠ, તાર્કિક, uint32_t>
compare_ગાંઠ(uint32_t Offset, StringRef Name, તાર્કિક Strict,
            અક્ષર PreviousCharInName, BufferType &Buffer,
            અચલ ગાંઠ *Parent = nullptr) {
  ગાંઠ N = read_ગાંઠ(Offset, Parent);
  std::size_t Consummed = 0;
  તાર્કિક DoesStartWith = N.Is_મૂળ || startsWith(Name, N.Name, Strict, Consummed,
                                              PreviousCharInName);
  જો (!DoesStartWith)
    પાછા std::make_tuple(N, ખોટું, 0);

  જો (Name.size() - Consummed == 0 && N.કિંમત != 0xFFFFFFFF)
    પાછા std::make_tuple(N, સાચું, N.કિંમત);

  જો (N.hasChildren()) {
    uint32_t ChildOffset = N.ChildrenOffset;
    માટે (;;) {
      ગાંઠ C;
      તાર્કિક Matches;
      uint32_t કિંમત;
      std::tie(C, Matches, કિંમત) =
          compare_ગાંઠ(ChildOffset, Name.substr(Consummed), Strict,
                      PreviousCharInName, Buffer, &N);
      જો (Matches) {
        std::reverse_copy(C.Name.begin(), C.Name.end(),
                          std::back_inserter(Buffer));
        પાછા std::make_tuple(N, સાચું, કિંમત);
      }
      ChildOffset += C.Size;
      જો (!C.HasSibling)
        તોડો;
    }
  }
  પાછા std::make_tuple(N, ખોટું, 0);
}

સ્થિર std::tuple<ગાંઠ, તાર્કિક, uint32_t>
compare_ગાંઠ(uint32_t Offset, StringRef Name, તાર્કિક Strict, BufferType &Buffer) {
  પાછા compare_ગાંઠ(Offset, Name, Strict, 0, Buffer);
}

// clang-format off
constexpr અચલ અક્ષર *અચલ HangulSyllables[][3] = {
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

સ્થિર std::size_t findSyllable(StringRef Name, તાર્કિક Strict,
                                અક્ષર &PreviousInName, પૂર્ણાંક &Pos, પૂર્ણાંક Column) {
  પ્રતિજ્ઞા(Column == 0 || Column == 1 || Column == 2);
  સ્થિર std::size_t ગણતરી_Per_Column[] = {LCount, VCount, TCount};
  પૂર્ણાંક Len = -1;
  પૂર્ણાંક Prev = PreviousInName;
  માટે (std::size_t I = 0; I < ગણતરી_Per_Column[Column]; I++) {
    StringRef Syllable(HangulSyllables[I][Column]);
    જો (પૂર્ણાંક(Syllable.size()) <= Len)
      ચાલુ;
    std::size_t Consummed = 0;
    અક્ષર PreviousInNameCopy = PreviousInName;
    તાર્કિક DoesStartWith =
        startsWith(Name, Syllable, Strict, Consummed, PreviousInNameCopy);
    જો (!DoesStartWith)
      ચાલુ;
    Len = Consummed;
    Pos = I;
    Prev = PreviousInNameCopy;
  }
  જો (Len == -1)
    પાછા 0;
  PreviousInName = Prev;
  પાછા size_t(Len);
}

સ્થિર std::optional<char32_t>
nameToHangulCodePoint(StringRef Name, તાર્કિક Strict, BufferType &Buffer) {
  Buffer.clear();
  // Hangul Syllable Decomposition
  std::size_t Consummed = 0;
  અક્ષર NameStart = 0;
  તાર્કિક DoesStartWith =
      startsWith(Name, "HANGUL SYLLABLE ", Strict, Consummed, NameStart);
  જો (!DoesStartWith)
    પાછા std::nullopt;
  Name = Name.substr(Consummed);
  પૂર્ણાંક L = -1, V = -1, T = -1;
  Name = Name.substr(findSyllable(Name, Strict, NameStart, L, 0));
  Name = Name.substr(findSyllable(Name, Strict, NameStart, V, 1));
  Name = Name.substr(findSyllable(Name, Strict, NameStart, T, 2));
  જો (L != -1 && V != -1 && T != -1 && Name.empty()) {
    જો (!Strict) {
      Buffer.append("HANGUL SYLLABLE ");
      જો (L != -1)
        Buffer.append(HangulSyllables[L][0]);
      જો (V != -1)
        Buffer.append(HangulSyllables[V][1]);
      જો (T != -1)
        Buffer.append(HangulSyllables[T][2]);
    }
    પાછા SBase + (std::uint32_t(L) * VCount + std::uint32_t(V)) * TCount +
           std::uint32_t(T);
  }
  // Otherwise, it's an illegal syllable name.
  પાછા std::nullopt;
}

સંરચના GeneratedNamesData {
  StringRef Prefix;
  uint32_t Start;
  uint32_t End;
};

// Unicode 18.0 Name Derivation Rule Prefix Strings (extracted/DerivedName.txt)
સ્થિર અચલ GeneratedNamesData GeneratedNamesDataTable[] = {
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

સ્થિર std::optional<char32_t>
nameToGeneratedCodePoint(StringRef Name, તાર્કિક Strict, BufferType &Buffer) {
  માટે (auto &&Item : GeneratedNamesDataTable) {
    Buffer.clear();
    std::size_t Consummed = 0;
    અક્ષર NameStart = 0;
    તાર્કિક DoesStartWith = startsWith(Name, Item.Prefix, Strict, Consummed,
                                    NameStart, /*IsPrefix=*/સાચું);
    જો (!DoesStartWith)
      ચાલુ;
    auto સંખ્યા = Name.substr(Consummed);
    unsigned long long V = 0;
    // Be consistent about mandating upper casing.
    જો (Strict &&
        llvm::any_of(સંખ્યા, [](અક્ષર C) { પાછા C >= 'a' && C <= 'f'; }))
      પાછા {};
    જો (getAsUnsignedInteger(સંખ્યા, 16, V) || V < Item.Start || V > Item.End)
      ચાલુ;
    જો (!Strict) {
      Buffer.append(Item.Prefix);
      Buffer.append(utohexstr(V, સાચું));
    }
    પાછા V;
  }
  પાછા std::nullopt;
}

સ્થિર std::optional<char32_t> nameToCodepoint(StringRef Name, તાર્કિક Strict,
                                               BufferType &Buffer) {
  જો (Name.empty())
    પાછા std::nullopt;

  std::optional<char32_t> Res = nameToHangulCodePoint(Name, Strict, Buffer);
  જો (!Res)
    Res = nameToGeneratedCodePoint(Name, Strict, Buffer);
  જો (Res)
    પાછા *Res;

  Buffer.clear();
  ગાંઠ ગાંઠ;
  તાર્કિક Matches;
  uint32_t કિંમત;
  std::tie(ગાંઠ, Matches, કિંમત) = compare_ગાંઠ(0, Name, Strict, Buffer);
  જો (Matches) {
    std::reverse(Buffer.begin(), Buffer.end());
    // UAX44-LM2. Ignore case, whitespace, underscore ('_'), and all medial
    // hyphens except the hyphen in U+1180 HANGUL JUNGSEONG O-E.
    જો (!Strict && કિંમત == 0x116c && Name.contains_insensitive("O-E")) {
      Buffer = "HANGUL JUNGSEONG O-E";
      કિંમત = 0x1180;
    }
    પાછા કિંમત;
  }
  પાછા std::nullopt;
}

std::optional<char32_t> nameToCodepointStrict(StringRef Name) {

  BufferType Buffer;
  auto Opt = nameToCodepoint(Name, સાચું, Buffer);
  પાછા Opt;
}

std::optional<LooseMatchingResult>
nameToCodepointLooseMatching(StringRef Name) {
  BufferType Buffer;
  auto Opt = nameToCodepoint(Name, ખોટું, Buffer);
  જો (!Opt)
    પાછા std::nullopt;
  પાછા LooseMatchingResult{*Opt, Buffer};
}

// Find the unicode character whose editing distance to Pattern
// is shortest, using the Wagner–Fischer algorithm.
llvm::SmallVector<MatchForCodepointName>
nearestMatchesForCodepointName(StringRef Pattern, std::size_t Max_Matches_ગણતરી) {
  // We maintain a fixed size vector of matches,
  // sorted by distance
  // The worst match (with the biggest distance) are discarded when new elements
  // are added.
  std::size_t LargestEditDistance = 0;
  llvm::SmallVector<MatchForCodepointName> Matches;
  Matches.reserve(Max_Matches_ગણતરી + 1);

  auto ઉમેરો_2 = [&](અચલ ગાંઠ &ગાંઠ, uint32_t Distance,
                    char32_t કિંમત) -> તાર્કિક {
    જો (Distance > LargestEditDistance) {
      જો (Matches.size() == Max_Matches_ગણતરી)
        પાછા ખોટું;
      LargestEditDistance = Distance;
    }
    // To avoid allocations, the creation of the name is delayed
    // as much as possible.
    std::string Name;
    auto GetName = [&] {
      જો (Name.empty())
        Name = ગાંઠ.fullName();
      પાછા Name;
    };

    auto It = llvm::lower_bound(
        Matches, Distance,
        [&](અચલ MatchForCodepointName &a, std::size_t Distance) {
          જો (Distance == a.Distance)
            પાછા a.Name < GetName();
          પાછા a.Distance < Distance;
        });
    જો (It == Matches.end() && Matches.size() == Max_Matches_ગણતરી)
      પાછા ખોટું;

    MatchForCodepointName M{GetName(), Distance, કિંમત};
    Matches.ઉમેરો(It, std::move(M));
    જો (Matches.size() > Max_Matches_ગણતરી)
      Matches.કાઢો_back();
    પાછા સાચું;
  };

  // We ignore case, space, hyphens, etc,
  // in both the search pattern and the prospective names.
  auto Normalize = [](StringRef Name) {
    std::string Out;
    Out.reserve(Name.size());
    માટે (અક્ષર C : Name) {
      જો (isAlnum(C))
        Out.ધકેલો_back(toUpper(C));
    }
    પાછા Out;
  };
  std::string NormalizedName = Normalize(Pattern);

  // Allocate a matrix big enough for longest names.
  અચલ std::size_t Columns =
      std::min(NormalizedName.size(), UnicodeNameToCodepointLargestNameSize) +
      1;

  [[maybe_unused]] સ્થિર std::size_t Rows =
      UnicodeNameToCodepointLargestNameSize + 1;

  std::vector<અક્ષર> Distances(
      Columns * (UnicodeNameToCodepointLargestNameSize + 1), 0);

  auto Get = [&Distances, Columns](size_t Column, std::size_t Row) -> અક્ષર & {
    પ્રતિજ્ઞા(Column < Columns);
    પ્રતિજ્ઞા(Row < Rows);
    પાછા Distances[Row * Columns + Column];
  };

  માટે (std::size_t I = 0; I < Columns; I++)
    Get(I, 0) = I;

  // Visit the childrens,
  // Filling (and overriding) the matrix for the name fragment of each node
  // iteratively. CompleteName is used to collect the actual name of potential
  // match, respecting case and spacing.
  auto Visit_ગાંઠ = [&](અચલ ગાંઠ &N, std::size_t Row,
                       auto &Visit_ગાંઠ) -> શૂન્ય {
    std::size_t J = 0;
    માટે (; J < N.Name.size(); J++) {
      જો (!isAlnum(N.Name[J]))
        ચાલુ;

      Get(0, Row) = Row;

      માટે (std::size_t I = 1; I < Columns; I++) {
        અચલ પૂર્ણાંક Delete = Get(I - 1, Row) + 1;
        અચલ પૂર્ણાંક ઉમેરો_2 = Get(I, Row - 1) + 1;

        અચલ પૂર્ણાંક Replace =
            Get(I - 1, Row - 1) + (NormalizedName[I - 1] != N.Name[J] ? 1 : 0);

        Get(I, Row) = std::min({ઉમેરો_2, Delete, Replace});
      }

      Row++;
    }

    unsigned Cost = Get(Columns - 1, Row - 1);
    જો (N.કિંમત != 0xFFFFFFFF) {
      ઉમેરો_2(N, Cost, N.કિંમત);
    }

    જો (N.hasChildren()) {
      auto ChildOffset = N.ChildrenOffset;
      માટે (;;) {
        ગાંઠ C = read_ગાંઠ(ChildOffset, &N);
        ChildOffset += C.Size;
        જો (!C.isValid())
          તોડો;
        Visit_ગાંઠ(C, Row, Visit_ગાંઠ);
        જો (!C.HasSibling)
          તોડો;
      }
    }
  };

  ગાંઠ મૂળ_2 = create_મૂળ();
  Visit_ગાંઠ(મૂળ_2, 1, Visit_ગાંઠ);
  પાછા Matches;
}

} // namespace unicode

} // namespace sys
} // namespace llvm
