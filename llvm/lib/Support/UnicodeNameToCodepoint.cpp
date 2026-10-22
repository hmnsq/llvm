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

बाह्य ध्रुव वर्ण *ध्रुव UnicodeNameToCodepointDict;
बाह्य ध्रुव uint8_t *ध्रुव UnicodeNameToCodepointIndex;
बाह्य ध्रुव std::size_t UnicodeNameToCodepointIndexSize;
बाह्य ध्रुव std::size_t UnicodeNameToCodepointLargestNameSize;

using BufferType = SmallString<64>;

संरचना गाँठ {
  तार्किक Is_मूल = असत्य;
  char32_t मूल्य = 0xFFFFFFFF;
  uint32_t ChildrenOffset = 0;
  तार्किक HasSibling = असत्य;
  uint32_t Size = 0;
  StringRef Name;
  ध्रुव गाँठ *Parent = nullptr;

  constexpr तार्किक isValid() ध्रुव {
    प्रत्यागम !Name.empty() || मूल्य == 0xFFFFFFFF;
  }
  constexpr तार्किक hasChildren() ध्रुव { प्रत्यागम ChildrenOffset != 0 || Is_मूल; }

  std::string fullName() ध्रुव {
    std::string S;
    // Reserve enough space for most unicode code points.
    // The chosen value represent the 99th percentile of name size as of
    // Unicode 15.0.
    S.reserve(46);
    ध्रुव गाँठ *N = एतत्;
    यावत् (N) {
      std::reverse_copy(N->Name.begin(), N->Name.end(), std::back_inserter(S));
      N = N->Parent;
    }
    std::reverse(S.begin(), S.end());
    प्रत्यागम S;
  }
};

स्थिर गाँठ create_मूल() {
  गाँठ N;
  N.Is_मूल = सत्य;
  N.ChildrenOffset = 1;
  N.Size = 1;
  प्रत्यागम N;
}

स्थिर गाँठ read_गाँठ(uint32_t Offset, ध्रुव गाँठ *Parent = nullptr) {
  यदि (Offset == 0)
    प्रत्यागम create_मूल();

  uint32_t Origin = Offset;
  गाँठ N;
  N.Parent = Parent;
  uint8_t NameInfo = UnicodeNameToCodepointIndex[Offset++];
  यदि (Offset + 6 >= UnicodeNameToCodepointIndexSize)
    प्रत्यागम N;

  तार्किक LongName = NameInfo & 0x40;
  तार्किक Has_मूल्य = NameInfo & 0x80;
  std::size_t Size = NameInfo & ~0xC0;
  यदि (LongName) {
    uint32_t NameOffset = (UnicodeNameToCodepointIndex[Offset++] << 8);
    NameOffset |= UnicodeNameToCodepointIndex[Offset++];
    N.Name = StringRef(UnicodeNameToCodepointDict + NameOffset, Size);
  } अन्यथा {
    N.Name = StringRef(UnicodeNameToCodepointDict + Size, 1);
  }
  यदि (Has_मूल्य) {
    uint8_t H = UnicodeNameToCodepointIndex[Offset++];
    uint8_t M = UnicodeNameToCodepointIndex[Offset++];
    uint8_t L = UnicodeNameToCodepointIndex[Offset++];
    N.मूल्य = ((H << 16) | (M << 8) | L) >> 3;

    तार्किक HasChildren = L & 0x02;
    N.HasSibling = L & 0x01;

    यदि (HasChildren) {
      N.ChildrenOffset = UnicodeNameToCodepointIndex[Offset++] << 16;
      N.ChildrenOffset |= UnicodeNameToCodepointIndex[Offset++] << 8;
      N.ChildrenOffset |= UnicodeNameToCodepointIndex[Offset++];
    }
  } अन्यथा {
    uint8_t H = UnicodeNameToCodepointIndex[Offset++];
    N.HasSibling = H & 0x80;
    तार्किक HasChildren = H & 0x40;
    H &= uint8_t(~0xC0);
    यदि (HasChildren) {
      N.ChildrenOffset = (H << 16);
      N.ChildrenOffset |=
          (uint32_t(UnicodeNameToCodepointIndex[Offset++]) << 8);
      N.ChildrenOffset |= UnicodeNameToCodepointIndex[Offset++];
    }
  }
  N.Size = Offset - Origin;
  प्रत्यागम N;
}

स्थिर तार्किक startsWith(StringRef Name, StringRef Needle, तार्किक Strict,
                       std::size_t &Consummed, वर्ण &PreviousCharInName,
                       तार्किक IsPrefix = असत्य) {

  Consummed = 0;
  यदि (Strict) {
    यदि (!Name.starts_with(Needle))
      प्रत्यागम असत्य;
    Consummed = Needle.size();
    प्रत्यागम सत्य;
  }
  यदि (Needle.empty())
    प्रत्यागम सत्य;

  auto NamePos = Name.begin();
  auto NeedlePos = Needle.begin();

  वर्ण PreviousCharInNameOrigin = PreviousCharInName;
  वर्ण PreviousCharInNeedle = *Needle.begin();
  auto IgnoreSpaces = [](auto It, auto End, वर्ण &PreviousChar,
                         तार्किक IsPrefix = असत्य) {
    यावत् (It != End) {
      ध्रुव auto Next = std::next(It);
      // Ignore spaces, underscore, medial hyphens
      // The generator ensures a needle never ends (or starts) by a medial
      // hyphen https://unicode.org/reports/tr44/#UAX44-LM2.
      तार्किक Ignore =
          *It == ' ' || *It == '_' ||
          (*It == '-' && isAlnum(PreviousChar) &&
           ((Next != End && isAlnum(*Next)) || (Next == End && IsPrefix)));
      PreviousChar = *It;
      यदि (!Ignore)
        भङ्ग;
      ++It;
    }
    प्रत्यागम It;
  };

  यावत् (सत्य) {
    NamePos = IgnoreSpaces(NamePos, Name.end(), PreviousCharInName);
    NeedlePos =
        IgnoreSpaces(NeedlePos, Needle.end(), PreviousCharInNeedle, IsPrefix);
    यदि (NeedlePos == Needle.end())
      भङ्ग;
    यदि (NamePos == Name.end())
      भङ्ग;
    यदि (toUpper(*NeedlePos) != toUpper(*NamePos))
      भङ्ग;
    NeedlePos++;
    NamePos++;
  }
  Consummed = std::distance(Name.begin(), NamePos);
  यदि (NeedlePos != Needle.end()) {
    PreviousCharInName = PreviousCharInNameOrigin;
  }
  प्रत्यागम NeedlePos == Needle.end();
}

स्थिर std::tuple<गाँठ, तार्किक, uint32_t>
compare_गाँठ(uint32_t Offset, StringRef Name, तार्किक Strict,
            वर्ण PreviousCharInName, BufferType &Buffer,
            ध्रुव गाँठ *Parent = nullptr) {
  गाँठ N = read_गाँठ(Offset, Parent);
  std::size_t Consummed = 0;
  तार्किक DoesStartWith = N.Is_मूल || startsWith(Name, N.Name, Strict, Consummed,
                                              PreviousCharInName);
  यदि (!DoesStartWith)
    प्रत्यागम std::make_tuple(N, असत्य, 0);

  यदि (Name.size() - Consummed == 0 && N.मूल्य != 0xFFFFFFFF)
    प्रत्यागम std::make_tuple(N, सत्य, N.मूल्य);

  यदि (N.hasChildren()) {
    uint32_t ChildOffset = N.ChildrenOffset;
    प्रति (;;) {
      गाँठ C;
      तार्किक Matches;
      uint32_t मूल्य;
      std::tie(C, Matches, मूल्य) =
          compare_गाँठ(ChildOffset, Name.substr(Consummed), Strict,
                      PreviousCharInName, Buffer, &N);
      यदि (Matches) {
        std::reverse_copy(C.Name.begin(), C.Name.end(),
                          std::back_inserter(Buffer));
        प्रत्यागम std::make_tuple(N, सत्य, मूल्य);
      }
      ChildOffset += C.Size;
      यदि (!C.HasSibling)
        भङ्ग;
    }
  }
  प्रत्यागम std::make_tuple(N, असत्य, 0);
}

स्थिर std::tuple<गाँठ, तार्किक, uint32_t>
compare_गाँठ(uint32_t Offset, StringRef Name, तार्किक Strict, BufferType &Buffer) {
  प्रत्यागम compare_गाँठ(Offset, Name, Strict, 0, Buffer);
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
                                वर्ण &PreviousInName, पूर्णाङ्क &Pos, पूर्णाङ्क Column) {
  प्रतिज्ञा(Column == 0 || Column == 1 || Column == 2);
  स्थिर std::size_t गिनती_Per_Column[] = {LCount, VCount, TCount};
  पूर्णाङ्क Len = -1;
  पूर्णाङ्क Prev = PreviousInName;
  प्रति (std::size_t I = 0; I < गिनती_Per_Column[Column]; I++) {
    StringRef Syllable(HangulSyllables[I][Column]);
    यदि (पूर्णाङ्क(Syllable.size()) <= Len)
      अनुवर्त;
    std::size_t Consummed = 0;
    वर्ण PreviousInNameCopy = PreviousInName;
    तार्किक DoesStartWith =
        startsWith(Name, Syllable, Strict, Consummed, PreviousInNameCopy);
    यदि (!DoesStartWith)
      अनुवर्त;
    Len = Consummed;
    Pos = I;
    Prev = PreviousInNameCopy;
  }
  यदि (Len == -1)
    प्रत्यागम 0;
  PreviousInName = Prev;
  प्रत्यागम size_t(Len);
}

स्थिर std::optional<char32_t>
nameToHangulCodePoint(StringRef Name, तार्किक Strict, BufferType &Buffer) {
  Buffer.clear();
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
  यदि (L != -1 && V != -1 && T != -1 && Name.empty()) {
    यदि (!Strict) {
      Buffer.append("HANGUL SYLLABLE ");
      यदि (L != -1)
        Buffer.append(HangulSyllables[L][0]);
      यदि (V != -1)
        Buffer.append(HangulSyllables[V][1]);
      यदि (T != -1)
        Buffer.append(HangulSyllables[T][2]);
    }
    प्रत्यागम SBase + (std::uint32_t(L) * VCount + std::uint32_t(V)) * TCount +
           std::uint32_t(T);
  }
  // Otherwise, it's an illegal syllable name.
  प्रत्यागम std::nullopt;
}

संरचना GeneratedNamesData {
  StringRef Prefix;
  uint32_t Start;
  uint32_t End;
};

// Unicode 18.0 Name Derivation Rule Prefix Strings (extracted/DerivedName.txt)
स्थिर ध्रुव GeneratedNamesData GeneratedNamesDataTable[] = {
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
nameToGeneratedCodePoint(StringRef Name, तार्किक Strict, BufferType &Buffer) {
  प्रति (auto &&Item : GeneratedNamesDataTable) {
    Buffer.clear();
    std::size_t Consummed = 0;
    वर्ण NameStart = 0;
    तार्किक DoesStartWith = startsWith(Name, Item.Prefix, Strict, Consummed,
                                    NameStart, /*IsPrefix=*/सत्य);
    यदि (!DoesStartWith)
      अनुवर्त;
    auto संख्या = Name.substr(Consummed);
    unsigned long long V = 0;
    // Be consistent about mandating upper casing.
    यदि (Strict &&
        llvm::any_of(संख्या, [](वर्ण C) { प्रत्यागम C >= 'a' && C <= 'f'; }))
      प्रत्यागम {};
    यदि (getAsUnsignedInteger(संख्या, 16, V) || V < Item.Start || V > Item.End)
      अनुवर्त;
    यदि (!Strict) {
      Buffer.append(Item.Prefix);
      Buffer.append(utohexstr(V, सत्य));
    }
    प्रत्यागम V;
  }
  प्रत्यागम std::nullopt;
}

स्थिर std::optional<char32_t> nameToCodepoint(StringRef Name, तार्किक Strict,
                                               BufferType &Buffer) {
  यदि (Name.empty())
    प्रत्यागम std::nullopt;

  std::optional<char32_t> Res = nameToHangulCodePoint(Name, Strict, Buffer);
  यदि (!Res)
    Res = nameToGeneratedCodePoint(Name, Strict, Buffer);
  यदि (Res)
    प्रत्यागम *Res;

  Buffer.clear();
  गाँठ गाँठ;
  तार्किक Matches;
  uint32_t मूल्य;
  std::tie(गाँठ, Matches, मूल्य) = compare_गाँठ(0, Name, Strict, Buffer);
  यदि (Matches) {
    std::reverse(Buffer.begin(), Buffer.end());
    // UAX44-LM2. Ignore case, whitespace, underscore ('_'), and all medial
    // hyphens except the hyphen in U+1180 HANGUL JUNGSEONG O-E.
    यदि (!Strict && मूल्य == 0x116c && Name.contains_insensitive("O-E")) {
      Buffer = "HANGUL JUNGSEONG O-E";
      मूल्य = 0x1180;
    }
    प्रत्यागम मूल्य;
  }
  प्रत्यागम std::nullopt;
}

std::optional<char32_t> nameToCodepointStrict(StringRef Name) {

  BufferType Buffer;
  auto Opt = nameToCodepoint(Name, सत्य, Buffer);
  प्रत्यागम Opt;
}

std::optional<LooseMatchingResult>
nameToCodepointLooseMatching(StringRef Name) {
  BufferType Buffer;
  auto Opt = nameToCodepoint(Name, असत्य, Buffer);
  यदि (!Opt)
    प्रत्यागम std::nullopt;
  प्रत्यागम LooseMatchingResult{*Opt, Buffer};
}

// Find the unicode character whose editing distance to Pattern
// is shortest, using the Wagner–Fischer algorithm.
llvm::SmallVector<MatchForCodepointName>
nearestMatchesForCodepointName(StringRef Pattern, std::size_t Max_Matches_गिनती) {
  // We maintain a fixed size vector of matches,
  // sorted by distance
  // The worst match (with the biggest distance) are discarded when new elements
  // are added.
  std::size_t LargestEditDistance = 0;
  llvm::SmallVector<MatchForCodepointName> Matches;
  Matches.reserve(Max_Matches_गिनती + 1);

  auto जोड़ो_2 = [&](ध्रुव गाँठ &गाँठ, uint32_t Distance,
                    char32_t मूल्य) -> तार्किक {
    यदि (Distance > LargestEditDistance) {
      यदि (Matches.size() == Max_Matches_गिनती)
        प्रत्यागम असत्य;
      LargestEditDistance = Distance;
    }
    // To avoid allocations, the creation of the name is delayed
    // as much as possible.
    std::string Name;
    auto GetName = [&] {
      यदि (Name.empty())
        Name = गाँठ.fullName();
      प्रत्यागम Name;
    };

    auto It = llvm::lower_bound(
        Matches, Distance,
        [&](ध्रुव MatchForCodepointName &a, std::size_t Distance) {
          यदि (Distance == a.Distance)
            प्रत्यागम a.Name < GetName();
          प्रत्यागम a.Distance < Distance;
        });
    यदि (It == Matches.end() && Matches.size() == Max_Matches_गिनती)
      प्रत्यागम असत्य;

    MatchForCodepointName M{GetName(), Distance, मूल्य};
    Matches.जोड़ो(It, std::move(M));
    यदि (Matches.size() > Max_Matches_गिनती)
      Matches.निकालो_back();
    प्रत्यागम सत्य;
  };

  // We ignore case, space, hyphens, etc,
  // in both the search pattern and the prospective names.
  auto Normalize = [](StringRef Name) {
    std::string Out;
    Out.reserve(Name.size());
    प्रति (वर्ण C : Name) {
      यदि (isAlnum(C))
        Out.धकेलो_back(toUpper(C));
    }
    प्रत्यागम Out;
  };
  std::string NormalizedName = Normalize(Pattern);

  // Allocate a matrix big enough for longest names.
  ध्रुव std::size_t Columns =
      std::min(NormalizedName.size(), UnicodeNameToCodepointLargestNameSize) +
      1;

  [[maybe_unused]] स्थिर std::size_t Rows =
      UnicodeNameToCodepointLargestNameSize + 1;

  std::vector<वर्ण> Distances(
      Columns * (UnicodeNameToCodepointLargestNameSize + 1), 0);

  auto Get = [&Distances, Columns](size_t Column, std::size_t Row) -> वर्ण & {
    प्रतिज्ञा(Column < Columns);
    प्रतिज्ञा(Row < Rows);
    प्रत्यागम Distances[Row * Columns + Column];
  };

  प्रति (std::size_t I = 0; I < Columns; I++)
    Get(I, 0) = I;

  // Visit the childrens,
  // Filling (and overriding) the matrix for the name fragment of each node
  // iteratively. CompleteName is used to collect the actual name of potential
  // match, respecting case and spacing.
  auto Visit_गाँठ = [&](ध्रुव गाँठ &N, std::size_t Row,
                       auto &Visit_गाँठ) -> शून्य {
    std::size_t J = 0;
    प्रति (; J < N.Name.size(); J++) {
      यदि (!isAlnum(N.Name[J]))
        अनुवर्त;

      Get(0, Row) = Row;

      प्रति (std::size_t I = 1; I < Columns; I++) {
        ध्रुव पूर्णाङ्क Delete = Get(I - 1, Row) + 1;
        ध्रुव पूर्णाङ्क जोड़ो_2 = Get(I, Row - 1) + 1;

        ध्रुव पूर्णाङ्क Replace =
            Get(I - 1, Row - 1) + (NormalizedName[I - 1] != N.Name[J] ? 1 : 0);

        Get(I, Row) = std::min({जोड़ो_2, Delete, Replace});
      }

      Row++;
    }

    unsigned Cost = Get(Columns - 1, Row - 1);
    यदि (N.मूल्य != 0xFFFFFFFF) {
      जोड़ो_2(N, Cost, N.मूल्य);
    }

    यदि (N.hasChildren()) {
      auto ChildOffset = N.ChildrenOffset;
      प्रति (;;) {
        गाँठ C = read_गाँठ(ChildOffset, &N);
        ChildOffset += C.Size;
        यदि (!C.isValid())
          भङ्ग;
        Visit_गाँठ(C, Row, Visit_गाँठ);
        यदि (!C.HasSibling)
          भङ्ग;
      }
    }
  };

  गाँठ मूल_2 = create_मूल();
  Visit_गाँठ(मूल_2, 1, Visit_गाँठ);
  प्रत्यागम Matches;
}

} // namespace unicode

} // namespace sys
} // namespace llvm
