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

बाह्य अचर वर्ण *अचर UnicodeNameToCodepointDict;
बाह्य अचर uint8_t *अचर UnicodeNameToCodepointIndex;
बाह्य अचर std::माप_प्रकार UnicodeNameToCodepointIndexSize;
बाह्य अचर std::माप_प्रकार UnicodeNameToCodepointLargestNameSize;

using BufferType = SmallString<64>;

संरचना गाँठो {
  तार्किक Is_जरा = असत्य;
  char32_t मान = 0xFFFFFFFF;
  uint32_t ChildrenOffset = 0;
  तार्किक HasSibling = असत्य;
  uint32_t Size = 0;
  StringRef Name;
  अचर गाँठो *Parent = nullptr;

  constexpr तार्किक isValid() अचर {
    फर्काऊ !Name.empty() || मान == 0xFFFFFFFF;
  }
  constexpr तार्किक hasChildren() अचर { फर्काऊ ChildrenOffset != 0 || Is_जरा; }

  std::string fullName() अचर {
    std::string S;
    // Reserve enough space for most unicode code points.
    // The chosen value represent the 99th percentile of name size as of
    // Unicode 15.0.
    S.reserve(46);
    अचर गाँठो *N = एतत्;
    जबसम्म (N) {
      std::reverse_copy(N->Name.begin(), N->Name.end(), std::back_inserter(S));
      N = N->Parent;
    }
    std::reverse(S.begin(), S.end());
    फर्काऊ S;
  }
};

स्थिर गाँठो create_जरा() {
  गाँठो N;
  N.Is_जरा = सत्य;
  N.ChildrenOffset = 1;
  N.Size = 1;
  फर्काऊ N;
}

स्थिर गाँठो read_गाँठो(uint32_t Offset, अचर गाँठो *Parent = nullptr) {
  यदि (Offset == 0)
    फर्काऊ create_जरा();

  uint32_t Origin = Offset;
  गाँठो N;
  N.Parent = Parent;
  uint8_t NameInfo = UnicodeNameToCodepointIndex[Offset++];
  यदि (Offset + 6 >= UnicodeNameToCodepointIndexSize)
    फर्काऊ N;

  तार्किक LongName = NameInfo & 0x40;
  तार्किक Has_मान = NameInfo & 0x80;
  std::माप_प्रकार Size = NameInfo & ~0xC0;
  यदि (LongName) {
    uint32_t NameOffset = (UnicodeNameToCodepointIndex[Offset++] << 8);
    NameOffset |= UnicodeNameToCodepointIndex[Offset++];
    N.Name = StringRef(UnicodeNameToCodepointDict + NameOffset, Size);
  } अन्यथा {
    N.Name = StringRef(UnicodeNameToCodepointDict + Size, 1);
  }
  यदि (Has_मान) {
    uint8_t H = UnicodeNameToCodepointIndex[Offset++];
    uint8_t M = UnicodeNameToCodepointIndex[Offset++];
    uint8_t L = UnicodeNameToCodepointIndex[Offset++];
    N.मान = ((H << 16) | (M << 8) | L) >> 3;

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
  फर्काऊ N;
}

स्थिर तार्किक startsWith(StringRef Name, StringRef Needle, तार्किक Strict,
                       std::माप_प्रकार &Consummed, वर्ण &PreviousCharInName,
                       तार्किक IsPrefix = असत्य) {

  Consummed = 0;
  यदि (Strict) {
    यदि (!Name.starts_with(Needle))
      फर्काऊ असत्य;
    Consummed = Needle.size();
    फर्काऊ सत्य;
  }
  यदि (Needle.empty())
    फर्काऊ सत्य;

  स्वतः NamePos = Name.begin();
  स्वतः NeedlePos = Needle.begin();

  वर्ण PreviousCharInNameOrigin = PreviousCharInName;
  वर्ण PreviousCharInNeedle = *Needle.begin();
  स्वतः IgnoreSpaces = [](स्वतः It, स्वतः End, वर्ण &PreviousChar,
                         तार्किक IsPrefix = असत्य) {
    जबसम्म (It != End) {
      अचर स्वतः Next = std::next(It);
      // Ignore spaces, underscore, medial hyphens
      // The generator ensures a needle never ends (or starts) by a medial
      // hyphen https://unicode.org/reports/tr44/#UAX44-LM2.
      तार्किक Ignore =
          *It == ' ' || *It == '_' ||
          (*It == '-' && isAlnum(PreviousChar) &&
           ((Next != End && isAlnum(*Next)) || (Next == End && IsPrefix)));
      PreviousChar = *It;
      यदि (!Ignore)
        रोक;
      ++It;
    }
    फर्काऊ It;
  };

  जबसम्म (सत्य) {
    NamePos = IgnoreSpaces(NamePos, Name.end(), PreviousCharInName);
    NeedlePos =
        IgnoreSpaces(NeedlePos, Needle.end(), PreviousCharInNeedle, IsPrefix);
    यदि (NeedlePos == Needle.end())
      रोक;
    यदि (NamePos == Name.end())
      रोक;
    यदि (toUpper(*NeedlePos) != toUpper(*NamePos))
      रोक;
    NeedlePos++;
    NamePos++;
  }
  Consummed = std::distance(Name.begin(), NamePos);
  यदि (NeedlePos != Needle.end()) {
    PreviousCharInName = PreviousCharInNameOrigin;
  }
  फर्काऊ NeedlePos == Needle.end();
}

स्थिर std::tuple<गाँठो, तार्किक, uint32_t>
compare_गाँठो(uint32_t Offset, StringRef Name, तार्किक Strict,
            वर्ण PreviousCharInName, BufferType &Buffer,
            अचर गाँठो *Parent = nullptr) {
  गाँठो N = read_गाँठो(Offset, Parent);
  std::माप_प्रकार Consummed = 0;
  तार्किक DoesStartWith = N.Is_जरा || startsWith(Name, N.Name, Strict, Consummed,
                                              PreviousCharInName);
  यदि (!DoesStartWith)
    फर्काऊ std::make_tuple(N, असत्य, 0);

  यदि (Name.size() - Consummed == 0 && N.मान != 0xFFFFFFFF)
    फर्काऊ std::make_tuple(N, सत्य, N.मान);

  यदि (N.hasChildren()) {
    uint32_t ChildOffset = N.ChildrenOffset;
    लागि (;;) {
      गाँठो C;
      तार्किक Matches;
      uint32_t मान;
      std::tie(C, Matches, मान) =
          compare_गाँठो(ChildOffset, Name.substr(Consummed), Strict,
                      PreviousCharInName, Buffer, &N);
      यदि (Matches) {
        std::reverse_copy(C.Name.begin(), C.Name.end(),
                          std::back_inserter(Buffer));
        फर्काऊ std::make_tuple(N, सत्य, मान);
      }
      ChildOffset += C.Size;
      यदि (!C.HasSibling)
        रोक;
    }
  }
  फर्काऊ std::make_tuple(N, असत्य, 0);
}

स्थिर std::tuple<गाँठो, तार्किक, uint32_t>
compare_गाँठो(uint32_t Offset, StringRef Name, तार्किक Strict, BufferType &Buffer) {
  फर्काऊ compare_गाँठो(Offset, Name, Strict, 0, Buffer);
}

// clang-format off
constexpr अचर वर्ण *अचर HangulSyllables[][3] = {
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

स्थिर std::माप_प्रकार findSyllable(StringRef Name, तार्किक Strict,
                                वर्ण &PreviousInName, पूर्णाङ्क &Pos, पूर्णाङ्क Column) {
  प्रतिज्ञा(Column == 0 || Column == 1 || Column == 2);
  स्थिर std::माप_प्रकार गन्ती_Per_Column[] = {LCount, VCount, TCount};
  पूर्णाङ्क Len = -1;
  पूर्णाङ्क Prev = PreviousInName;
  लागि (std::माप_प्रकार I = 0; I < गन्ती_Per_Column[Column]; I++) {
    StringRef Syllable(HangulSyllables[I][Column]);
    यदि (पूर्णाङ्क(Syllable.size()) <= Len)
      जारी;
    std::माप_प्रकार Consummed = 0;
    वर्ण PreviousInNameCopy = PreviousInName;
    तार्किक DoesStartWith =
        startsWith(Name, Syllable, Strict, Consummed, PreviousInNameCopy);
    यदि (!DoesStartWith)
      जारी;
    Len = Consummed;
    Pos = I;
    Prev = PreviousInNameCopy;
  }
  यदि (Len == -1)
    फर्काऊ 0;
  PreviousInName = Prev;
  फर्काऊ माप_प्रकार(Len);
}

स्थिर std::optional<char32_t>
nameToHangulCodePoint(StringRef Name, तार्किक Strict, BufferType &Buffer) {
  Buffer.clear();
  // Hangul Syllable Decomposition
  std::माप_प्रकार Consummed = 0;
  वर्ण NameStart = 0;
  तार्किक DoesStartWith =
      startsWith(Name, "HANGUL SYLLABLE ", Strict, Consummed, NameStart);
  यदि (!DoesStartWith)
    फर्काऊ std::nullopt;
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
    फर्काऊ SBase + (std::uint32_t(L) * VCount + std::uint32_t(V)) * TCount +
           std::uint32_t(T);
  }
  // Otherwise, it's an illegal syllable name.
  फर्काऊ std::nullopt;
}

संरचना GeneratedNamesData {
  StringRef Prefix;
  uint32_t Start;
  uint32_t End;
};

// Unicode 18.0 Name Derivation Rule Prefix Strings (extracted/DerivedName.txt)
स्थिर अचर GeneratedNamesData GeneratedNamesDataTable[] = {
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
  लागि (स्वतः &&Item : GeneratedNamesDataTable) {
    Buffer.clear();
    std::माप_प्रकार Consummed = 0;
    वर्ण NameStart = 0;
    तार्किक DoesStartWith = startsWith(Name, Item.Prefix, Strict, Consummed,
                                    NameStart, /*IsPrefix=*/सत्य);
    यदि (!DoesStartWith)
      जारी;
    स्वतः सङ्ख्या = Name.substr(Consummed);
    अचिन्हित दीर्घ दीर्घ V = 0;
    // Be consistent about mandating upper casing.
    यदि (Strict &&
        llvm::any_of(सङ्ख्या, [](वर्ण C) { फर्काऊ C >= 'a' && C <= 'f'; }))
      फर्काऊ {};
    यदि (getAsUnsignedInteger(सङ्ख्या, 16, V) || V < Item.Start || V > Item.End)
      जारी;
    यदि (!Strict) {
      Buffer.append(Item.Prefix);
      Buffer.append(utohexstr(V, सत्य));
    }
    फर्काऊ V;
  }
  फर्काऊ std::nullopt;
}

स्थिर std::optional<char32_t> nameToCodepoint(StringRef Name, तार्किक Strict,
                                               BufferType &Buffer) {
  यदि (Name.empty())
    फर्काऊ std::nullopt;

  std::optional<char32_t> Res = nameToHangulCodePoint(Name, Strict, Buffer);
  यदि (!Res)
    Res = nameToGeneratedCodePoint(Name, Strict, Buffer);
  यदि (Res)
    फर्काऊ *Res;

  Buffer.clear();
  गाँठो गाँठो;
  तार्किक Matches;
  uint32_t मान;
  std::tie(गाँठो, Matches, मान) = compare_गाँठो(0, Name, Strict, Buffer);
  यदि (Matches) {
    std::reverse(Buffer.begin(), Buffer.end());
    // UAX44-LM2. Ignore case, whitespace, underscore ('_'), and all medial
    // hyphens except the hyphen in U+1180 HANGUL JUNGSEONG O-E.
    यदि (!Strict && मान == 0x116c && Name.contains_insensitive("O-E")) {
      Buffer = "HANGUL JUNGSEONG O-E";
      मान = 0x1180;
    }
    फर्काऊ मान;
  }
  फर्काऊ std::nullopt;
}

std::optional<char32_t> nameToCodepointStrict(StringRef Name) {

  BufferType Buffer;
  स्वतः Opt = nameToCodepoint(Name, सत्य, Buffer);
  फर्काऊ Opt;
}

std::optional<LooseMatchingResult>
nameToCodepointLooseMatching(StringRef Name) {
  BufferType Buffer;
  स्वतः Opt = nameToCodepoint(Name, असत्य, Buffer);
  यदि (!Opt)
    फर्काऊ std::nullopt;
  फर्काऊ LooseMatchingResult{*Opt, Buffer};
}

// Find the unicode character whose editing distance to Pattern
// is shortest, using the Wagner–Fischer algorithm.
llvm::SmallVector<MatchForCodepointName>
nearestMatchesForCodepointName(StringRef Pattern, std::माप_प्रकार Max_Matches_गन्ती) {
  // We maintain a fixed size vector of matches,
  // sorted by distance
  // The worst match (with the biggest distance) are discarded when new elements
  // are added.
  std::माप_प्रकार LargestEditDistance = 0;
  llvm::SmallVector<MatchForCodepointName> Matches;
  Matches.reserve(Max_Matches_गन्ती + 1);

  स्वतः घुसाउ_2 = [&](अचर गाँठो &गाँठो, uint32_t Distance,
                    char32_t मान) -> तार्किक {
    यदि (Distance > LargestEditDistance) {
      यदि (Matches.size() == Max_Matches_गन्ती)
        फर्काऊ असत्य;
      LargestEditDistance = Distance;
    }
    // To avoid allocations, the creation of the name is delayed
    // as much as possible.
    std::string Name;
    स्वतः GetName = [&] {
      यदि (Name.empty())
        Name = गाँठो.fullName();
      फर्काऊ Name;
    };

    स्वतः It = llvm::lower_bound(
        Matches, Distance,
        [&](अचर MatchForCodepointName &a, std::माप_प्रकार Distance) {
          यदि (Distance == a.Distance)
            फर्काऊ a.Name < GetName();
          फर्काऊ a.Distance < Distance;
        });
    यदि (It == Matches.end() && Matches.size() == Max_Matches_गन्ती)
      फर्काऊ असत्य;

    MatchForCodepointName M{GetName(), Distance, मान};
    Matches.घुसाउ(It, std::move(M));
    यदि (Matches.size() > Max_Matches_गन्ती)
      Matches.झिक_back();
    फर्काऊ सत्य;
  };

  // We ignore case, space, hyphens, etc,
  // in both the search pattern and the prospective names.
  स्वतः Normalize = [](StringRef Name) {
    std::string Out;
    Out.reserve(Name.size());
    लागि (वर्ण C : Name) {
      यदि (isAlnum(C))
        Out.थप_back(toUpper(C));
    }
    फर्काऊ Out;
  };
  std::string NormalizedName = Normalize(Pattern);

  // Allocate a matrix big enough for longest names.
  अचर std::माप_प्रकार Columns =
      std::min(NormalizedName.size(), UnicodeNameToCodepointLargestNameSize) +
      1;

  [[maybe_unused]] स्थिर std::माप_प्रकार Rows =
      UnicodeNameToCodepointLargestNameSize + 1;

  std::vector<वर्ण> Distances(
      Columns * (UnicodeNameToCodepointLargestNameSize + 1), 0);

  स्वतः Get = [&Distances, Columns](माप_प्रकार Column, std::माप_प्रकार Row) -> वर्ण & {
    प्रतिज्ञा(Column < Columns);
    प्रतिज्ञा(Row < Rows);
    फर्काऊ Distances[Row * Columns + Column];
  };

  लागि (std::माप_प्रकार I = 0; I < Columns; I++)
    Get(I, 0) = I;

  // Visit the childrens,
  // Filling (and overriding) the matrix for the name fragment of each node
  // iteratively. CompleteName is used to collect the actual name of potential
  // match, respecting case and spacing.
  स्वतः Visit_गाँठो = [&](अचर गाँठो &N, std::माप_प्रकार Row,
                       स्वतः &Visit_गाँठो) -> शून्य {
    std::माप_प्रकार J = 0;
    लागि (; J < N.Name.size(); J++) {
      यदि (!isAlnum(N.Name[J]))
        जारी;

      Get(0, Row) = Row;

      लागि (std::माप_प्रकार I = 1; I < Columns; I++) {
        अचर पूर्णाङ्क Delete = Get(I - 1, Row) + 1;
        अचर पूर्णाङ्क घुसाउ_2 = Get(I, Row - 1) + 1;

        अचर पूर्णाङ्क Replace =
            Get(I - 1, Row - 1) + (NormalizedName[I - 1] != N.Name[J] ? 1 : 0);

        Get(I, Row) = std::min({घुसाउ_2, Delete, Replace});
      }

      Row++;
    }

    अचिन्हित Cost = Get(Columns - 1, Row - 1);
    यदि (N.मान != 0xFFFFFFFF) {
      घुसाउ_2(N, Cost, N.मान);
    }

    यदि (N.hasChildren()) {
      स्वतः ChildOffset = N.ChildrenOffset;
      लागि (;;) {
        गाँठो C = read_गाँठो(ChildOffset, &N);
        ChildOffset += C.Size;
        यदि (!C.isValid())
          रोक;
        Visit_गाँठो(C, Row, Visit_गाँठो);
        यदि (!C.HasSibling)
          रोक;
      }
    }
  };

  गाँठो जरा_2 = create_जरा();
  Visit_गाँठो(जरा_2, 1, Visit_गाँठो);
  फर्काऊ Matches;
}

} // namespace unicode

} // namespace sys
} // namespace llvm
