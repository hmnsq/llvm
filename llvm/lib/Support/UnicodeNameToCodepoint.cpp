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

संरचना ग्रन्थिः {
  तार्किक Is_मूलम् = मिच्छा;
  char32_t मानम् = 0xFFFFFFFF;
  uint32_t ChildrenOffset = 0;
  तार्किक HasSibling = मिच्छा;
  uint32_t Size = 0;
  StringRef Name;
  ध्रुव ग्रन्थिः *Parent = nullptr;

  constexpr तार्किक isValid() ध्रुव {
    पटिनिवत्त !Name.empty() || मानम् == 0xFFFFFFFF;
  }
  constexpr तार्किक hasChildren() ध्रुव { पटिनिवत्त ChildrenOffset != 0 || Is_मूलम्; }

  std::string fullName() ध्रुव {
    std::string S;
    // Reserve enough space for most unicode code points.
    // The chosen value represent the 99th percentile of name size as of
    // Unicode 15.0.
    S.reserve(46);
    ध्रुव ग्रन्थिः *N = एतत्;
    याव (N) {
      std::reverse_copy(N->Name.begin(), N->Name.end(), std::back_inserter(S));
      N = N->Parent;
    }
    std::reverse(S.begin(), S.end());
    पटिनिवत्त S;
  }
};

स्थिर ग्रन्थिः create_मूलम्() {
  ग्रन्थिः N;
  N.Is_मूलम् = सच्च;
  N.ChildrenOffset = 1;
  N.Size = 1;
  पटिनिवत्त N;
}

स्थिर ग्रन्थिः read_ग्रन्थिः(uint32_t Offset, ध्रुव ग्रन्थिः *Parent = nullptr) {
  सचे (Offset == 0)
    पटिनिवत्त create_मूलम्();

  uint32_t Origin = Offset;
  ग्रन्थिः N;
  N.Parent = Parent;
  uint8_t NameInfo = UnicodeNameToCodepointIndex[Offset++];
  सचे (Offset + 6 >= UnicodeNameToCodepointIndexSize)
    पटिनिवत्त N;

  तार्किक LongName = NameInfo & 0x40;
  तार्किक Has_मानम् = NameInfo & 0x80;
  std::size_t Size = NameInfo & ~0xC0;
  सचे (LongName) {
    uint32_t NameOffset = (UnicodeNameToCodepointIndex[Offset++] << 8);
    NameOffset |= UnicodeNameToCodepointIndex[Offset++];
    N.Name = StringRef(UnicodeNameToCodepointDict + NameOffset, Size);
  } अञ्ञथा {
    N.Name = StringRef(UnicodeNameToCodepointDict + Size, 1);
  }
  सचे (Has_मानम्) {
    uint8_t H = UnicodeNameToCodepointIndex[Offset++];
    uint8_t M = UnicodeNameToCodepointIndex[Offset++];
    uint8_t L = UnicodeNameToCodepointIndex[Offset++];
    N.मानम् = ((H << 16) | (M << 8) | L) >> 3;

    तार्किक HasChildren = L & 0x02;
    N.HasSibling = L & 0x01;

    सचे (HasChildren) {
      N.ChildrenOffset = UnicodeNameToCodepointIndex[Offset++] << 16;
      N.ChildrenOffset |= UnicodeNameToCodepointIndex[Offset++] << 8;
      N.ChildrenOffset |= UnicodeNameToCodepointIndex[Offset++];
    }
  } अञ्ञथा {
    uint8_t H = UnicodeNameToCodepointIndex[Offset++];
    N.HasSibling = H & 0x80;
    तार्किक HasChildren = H & 0x40;
    H &= uint8_t(~0xC0);
    सचे (HasChildren) {
      N.ChildrenOffset = (H << 16);
      N.ChildrenOffset |=
          (uint32_t(UnicodeNameToCodepointIndex[Offset++]) << 8);
      N.ChildrenOffset |= UnicodeNameToCodepointIndex[Offset++];
    }
  }
  N.Size = Offset - Origin;
  पटिनिवत्त N;
}

स्थिर तार्किक startsWith(StringRef Name, StringRef Needle, तार्किक Strict,
                       std::size_t &Consummed, वर्ण &PreviousCharInName,
                       तार्किक IsPrefix = मिच्छा) {

  Consummed = 0;
  सचे (Strict) {
    सचे (!Name.starts_with(Needle))
      पटिनिवत्त मिच्छा;
    Consummed = Needle.size();
    पटिनिवत्त सच्च;
  }
  सचे (Needle.empty())
    पटिनिवत्त सच्च;

  auto NamePos = Name.begin();
  auto NeedlePos = Needle.begin();

  वर्ण PreviousCharInNameOrigin = PreviousCharInName;
  वर्ण PreviousCharInNeedle = *Needle.begin();
  auto IgnoreSpaces = [](auto It, auto End, वर्ण &PreviousChar,
                         तार्किक IsPrefix = मिच्छा) {
    याव (It != End) {
      ध्रुव auto Next = std::next(It);
      // Ignore spaces, underscore, medial hyphens
      // The generator ensures a needle never ends (or starts) by a medial
      // hyphen https://unicode.org/reports/tr44/#UAX44-LM2.
      तार्किक Ignore =
          *It == ' ' || *It == '_' ||
          (*It == '-' && isAlnum(PreviousChar) &&
           ((Next != End && isAlnum(*Next)) || (Next == End && IsPrefix)));
      PreviousChar = *It;
      सचे (!Ignore)
        भङ्ग;
      ++It;
    }
    पटिनिवत्त It;
  };

  याव (सच्च) {
    NamePos = IgnoreSpaces(NamePos, Name.end(), PreviousCharInName);
    NeedlePos =
        IgnoreSpaces(NeedlePos, Needle.end(), PreviousCharInNeedle, IsPrefix);
    सचे (NeedlePos == Needle.end())
      भङ्ग;
    सचे (NamePos == Name.end())
      भङ्ग;
    सचे (toUpper(*NeedlePos) != toUpper(*NamePos))
      भङ्ग;
    NeedlePos++;
    NamePos++;
  }
  Consummed = std::distance(Name.begin(), NamePos);
  सचे (NeedlePos != Needle.end()) {
    PreviousCharInName = PreviousCharInNameOrigin;
  }
  पटिनिवत्त NeedlePos == Needle.end();
}

स्थिर std::tuple<ग्रन्थिः, तार्किक, uint32_t>
compare_ग्रन्थिः(uint32_t Offset, StringRef Name, तार्किक Strict,
            वर्ण PreviousCharInName, BufferType &Buffer,
            ध्रुव ग्रन्थिः *Parent = nullptr) {
  ग्रन्थिः N = read_ग्रन्थिः(Offset, Parent);
  std::size_t Consummed = 0;
  तार्किक DoesStartWith = N.Is_मूलम् || startsWith(Name, N.Name, Strict, Consummed,
                                              PreviousCharInName);
  सचे (!DoesStartWith)
    पटिनिवत्त std::make_tuple(N, मिच्छा, 0);

  सचे (Name.size() - Consummed == 0 && N.मानम् != 0xFFFFFFFF)
    पटिनिवत्त std::make_tuple(N, सच्च, N.मानम्);

  सचे (N.hasChildren()) {
    uint32_t ChildOffset = N.ChildrenOffset;
    अत्थाय (;;) {
      ग्रन्थिः C;
      तार्किक Matches;
      uint32_t मानम्;
      std::tie(C, Matches, मानम्) =
          compare_ग्रन्थिः(ChildOffset, Name.substr(Consummed), Strict,
                      PreviousCharInName, Buffer, &N);
      सचे (Matches) {
        std::reverse_copy(C.Name.begin(), C.Name.end(),
                          std::back_inserter(Buffer));
        पटिनिवत्त std::make_tuple(N, सच्च, मानम्);
      }
      ChildOffset += C.Size;
      सचे (!C.HasSibling)
        भङ्ग;
    }
  }
  पटिनिवत्त std::make_tuple(N, मिच्छा, 0);
}

स्थिर std::tuple<ग्रन्थिः, तार्किक, uint32_t>
compare_ग्रन्थिः(uint32_t Offset, StringRef Name, तार्किक Strict, BufferType &Buffer) {
  पटिनिवत्त compare_ग्रन्थिः(Offset, Name, Strict, 0, Buffer);
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
  स्थिर std::size_t गणनम्_Per_Column[] = {LCount, VCount, TCount};
  पूर्णाङ्क Len = -1;
  पूर्णाङ्क Prev = PreviousInName;
  अत्थाय (std::size_t I = 0; I < गणनम्_Per_Column[Column]; I++) {
    StringRef Syllable(HangulSyllables[I][Column]);
    सचे (पूर्णाङ्क(Syllable.size()) <= Len)
      अनुवर्त;
    std::size_t Consummed = 0;
    वर्ण PreviousInNameCopy = PreviousInName;
    तार्किक DoesStartWith =
        startsWith(Name, Syllable, Strict, Consummed, PreviousInNameCopy);
    सचे (!DoesStartWith)
      अनुवर्त;
    Len = Consummed;
    Pos = I;
    Prev = PreviousInNameCopy;
  }
  सचे (Len == -1)
    पटिनिवत्त 0;
  PreviousInName = Prev;
  पटिनिवत्त size_t(Len);
}

स्थिर std::optional<char32_t>
nameToHangulCodePoint(StringRef Name, तार्किक Strict, BufferType &Buffer) {
  Buffer.clear();
  // Hangul Syllable Decomposition
  std::size_t Consummed = 0;
  वर्ण NameStart = 0;
  तार्किक DoesStartWith =
      startsWith(Name, "HANGUL SYLLABLE ", Strict, Consummed, NameStart);
  सचे (!DoesStartWith)
    पटिनिवत्त std::nullopt;
  Name = Name.substr(Consummed);
  पूर्णाङ्क L = -1, V = -1, T = -1;
  Name = Name.substr(findSyllable(Name, Strict, NameStart, L, 0));
  Name = Name.substr(findSyllable(Name, Strict, NameStart, V, 1));
  Name = Name.substr(findSyllable(Name, Strict, NameStart, T, 2));
  सचे (L != -1 && V != -1 && T != -1 && Name.empty()) {
    सचे (!Strict) {
      Buffer.append("HANGUL SYLLABLE ");
      सचे (L != -1)
        Buffer.append(HangulSyllables[L][0]);
      सचे (V != -1)
        Buffer.append(HangulSyllables[V][1]);
      सचे (T != -1)
        Buffer.append(HangulSyllables[T][2]);
    }
    पटिनिवत्त SBase + (std::uint32_t(L) * VCount + std::uint32_t(V)) * TCount +
           std::uint32_t(T);
  }
  // Otherwise, it's an illegal syllable name.
  पटिनिवत्त std::nullopt;
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
  अत्थाय (auto &&Item : GeneratedNamesDataTable) {
    Buffer.clear();
    std::size_t Consummed = 0;
    वर्ण NameStart = 0;
    तार्किक DoesStartWith = startsWith(Name, Item.Prefix, Strict, Consummed,
                                    NameStart, /*IsPrefix=*/सच्च);
    सचे (!DoesStartWith)
      अनुवर्त;
    auto सङ्ख्या = Name.substr(Consummed);
    unsigned long long V = 0;
    // Be consistent about mandating upper casing.
    सचे (Strict &&
        llvm::any_of(सङ्ख्या, [](वर्ण C) { पटिनिवत्त C >= 'a' && C <= 'f'; }))
      पटिनिवत्त {};
    सचे (getAsUnsignedInteger(सङ्ख्या, 16, V) || V < Item.Start || V > Item.End)
      अनुवर्त;
    सचे (!Strict) {
      Buffer.append(Item.Prefix);
      Buffer.append(utohexstr(V, सच्च));
    }
    पटिनिवत्त V;
  }
  पटिनिवत्त std::nullopt;
}

स्थिर std::optional<char32_t> nameToCodepoint(StringRef Name, तार्किक Strict,
                                               BufferType &Buffer) {
  सचे (Name.empty())
    पटिनिवत्त std::nullopt;

  std::optional<char32_t> Res = nameToHangulCodePoint(Name, Strict, Buffer);
  सचे (!Res)
    Res = nameToGeneratedCodePoint(Name, Strict, Buffer);
  सचे (Res)
    पटिनिवत्त *Res;

  Buffer.clear();
  ग्रन्थिः ग्रन्थिः;
  तार्किक Matches;
  uint32_t मानम्;
  std::tie(ग्रन्थिः, Matches, मानम्) = compare_ग्रन्थिः(0, Name, Strict, Buffer);
  सचे (Matches) {
    std::reverse(Buffer.begin(), Buffer.end());
    // UAX44-LM2. Ignore case, whitespace, underscore ('_'), and all medial
    // hyphens except the hyphen in U+1180 HANGUL JUNGSEONG O-E.
    सचे (!Strict && मानम् == 0x116c && Name.contains_insensitive("O-E")) {
      Buffer = "HANGUL JUNGSEONG O-E";
      मानम् = 0x1180;
    }
    पटिनिवत्त मानम्;
  }
  पटिनिवत्त std::nullopt;
}

std::optional<char32_t> nameToCodepointStrict(StringRef Name) {

  BufferType Buffer;
  auto Opt = nameToCodepoint(Name, सच्च, Buffer);
  पटिनिवत्त Opt;
}

std::optional<LooseMatchingResult>
nameToCodepointLooseMatching(StringRef Name) {
  BufferType Buffer;
  auto Opt = nameToCodepoint(Name, मिच्छा, Buffer);
  सचे (!Opt)
    पटिनिवत्त std::nullopt;
  पटिनिवत्त LooseMatchingResult{*Opt, Buffer};
}

// Find the unicode character whose editing distance to Pattern
// is shortest, using the Wagner–Fischer algorithm.
llvm::SmallVector<MatchForCodepointName>
nearestMatchesForCodepointName(StringRef Pattern, std::size_t Max_Matches_गणनम्) {
  // We maintain a fixed size vector of matches,
  // sorted by distance
  // The worst match (with the biggest distance) are discarded when new elements
  // are added.
  std::size_t LargestEditDistance = 0;
  llvm::SmallVector<MatchForCodepointName> Matches;
  Matches.reserve(Max_Matches_गणनम् + 1);

  auto निवेशय_2 = [&](ध्रुव ग्रन्थिः &ग्रन्थिः, uint32_t Distance,
                    char32_t मानम्) -> तार्किक {
    सचे (Distance > LargestEditDistance) {
      सचे (Matches.size() == Max_Matches_गणनम्)
        पटिनिवत्त मिच्छा;
      LargestEditDistance = Distance;
    }
    // To avoid allocations, the creation of the name is delayed
    // as much as possible.
    std::string Name;
    auto GetName = [&] {
      सचे (Name.empty())
        Name = ग्रन्थिः.fullName();
      पटिनिवत्त Name;
    };

    auto It = llvm::lower_bound(
        Matches, Distance,
        [&](ध्रुव MatchForCodepointName &a, std::size_t Distance) {
          सचे (Distance == a.Distance)
            पटिनिवत्त a.Name < GetName();
          पटिनिवत्त a.Distance < Distance;
        });
    सचे (It == Matches.end() && Matches.size() == Max_Matches_गणनम्)
      पटिनिवत्त मिच्छा;

    MatchForCodepointName M{GetName(), Distance, मानम्};
    Matches.निवेशय(It, std::move(M));
    सचे (Matches.size() > Max_Matches_गणनम्)
      Matches.निष्कासय_back();
    पटिनिवत्त सच्च;
  };

  // We ignore case, space, hyphens, etc,
  // in both the search pattern and the prospective names.
  auto Normalize = [](StringRef Name) {
    std::string Out;
    Out.reserve(Name.size());
    अत्थाय (वर्ण C : Name) {
      सचे (isAlnum(C))
        Out.प्रक्षिप_back(toUpper(C));
    }
    पटिनिवत्त Out;
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
    पटिनिवत्त Distances[Row * Columns + Column];
  };

  अत्थाय (std::size_t I = 0; I < Columns; I++)
    Get(I, 0) = I;

  // Visit the childrens,
  // Filling (and overriding) the matrix for the name fragment of each node
  // iteratively. CompleteName is used to collect the actual name of potential
  // match, respecting case and spacing.
  auto Visit_ग्रन्थिः = [&](ध्रुव ग्रन्थिः &N, std::size_t Row,
                       auto &Visit_ग्रन्थिः) -> शून्य {
    std::size_t J = 0;
    अत्थाय (; J < N.Name.size(); J++) {
      सचे (!isAlnum(N.Name[J]))
        अनुवर्त;

      Get(0, Row) = Row;

      अत्थाय (std::size_t I = 1; I < Columns; I++) {
        ध्रुव पूर्णाङ्क Delete = Get(I - 1, Row) + 1;
        ध्रुव पूर्णाङ्क निवेशय_2 = Get(I, Row - 1) + 1;

        ध्रुव पूर्णाङ्क Replace =
            Get(I - 1, Row - 1) + (NormalizedName[I - 1] != N.Name[J] ? 1 : 0);

        Get(I, Row) = std::min({निवेशय_2, Delete, Replace});
      }

      Row++;
    }

    unsigned Cost = Get(Columns - 1, Row - 1);
    सचे (N.मानम् != 0xFFFFFFFF) {
      निवेशय_2(N, Cost, N.मानम्);
    }

    सचे (N.hasChildren()) {
      auto ChildOffset = N.ChildrenOffset;
      अत्थाय (;;) {
        ग्रन्थिः C = read_ग्रन्थिः(ChildOffset, &N);
        ChildOffset += C.Size;
        सचे (!C.isValid())
          भङ्ग;
        Visit_ग्रन्थिः(C, Row, Visit_ग्रन्थिः);
        सचे (!C.HasSibling)
          भङ्ग;
      }
    }
  };

  ग्रन्थिः मूलम्_2 = create_मूलम्();
  Visit_ग्रन्थिः(मूलम्_2, 1, Visit_ग्रन्थिः);
  पटिनिवत्त Matches;
}

} // namespace unicode

} // namespace sys
} // namespace llvm
