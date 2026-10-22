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

बाह्य स्थिरांक अक्षर *स्थिरांक UnicodeNameToCodepointDict;
बाह्य स्थिरांक uint8_t *स्थिरांक UnicodeNameToCodepointIndex;
बाह्य स्थिरांक std::माप_प्रकार UnicodeNameToCodepointIndexSize;
बाह्य स्थिरांक std::माप_प्रकार UnicodeNameToCodepointLargestNameSize;

using BufferType = SmallString<64>;

संरचना गाठ {
  तार्किक Is_मूळ = असत्य;
  char32_t मूल्य = 0xFFFFFFFF;
  uint32_t ChildrenOffset = 0;
  तार्किक HasSibling = असत्य;
  uint32_t Size = 0;
  StringRef Name;
  स्थिरांक गाठ *Parent = nullptr;

  constexpr तार्किक isValid() स्थिरांक {
    परत !Name.empty() || मूल्य == 0xFFFFFFFF;
  }
  constexpr तार्किक hasChildren() स्थिरांक { परत ChildrenOffset != 0 || Is_मूळ; }

  std::string fullName() स्थिरांक {
    std::string S;
    // Reserve enough space for most unicode code points.
    // The chosen value represent the 99th percentile of name size as of
    // Unicode 15.0.
    S.reserve(46);
    स्थिरांक गाठ *N = हे;
    पर्यंत (N) {
      std::reverse_copy(N->Name.begin(), N->Name.end(), std::back_inserter(S));
      N = N->Parent;
    }
    std::reverse(S.begin(), S.end());
    परत S;
  }
};

स्थिर गाठ create_मूळ() {
  गाठ N;
  N.Is_मूळ = सत्य;
  N.ChildrenOffset = 1;
  N.Size = 1;
  परत N;
}

स्थिर गाठ read_गाठ(uint32_t Offset, स्थिरांक गाठ *Parent = nullptr) {
  जर (Offset == 0)
    परत create_मूळ();

  uint32_t Origin = Offset;
  गाठ N;
  N.Parent = Parent;
  uint8_t NameInfo = UnicodeNameToCodepointIndex[Offset++];
  जर (Offset + 6 >= UnicodeNameToCodepointIndexSize)
    परत N;

  तार्किक LongName = NameInfo & 0x40;
  तार्किक Has_मूल्य = NameInfo & 0x80;
  std::माप_प्रकार Size = NameInfo & ~0xC0;
  जर (LongName) {
    uint32_t NameOffset = (UnicodeNameToCodepointIndex[Offset++] << 8);
    NameOffset |= UnicodeNameToCodepointIndex[Offset++];
    N.Name = StringRef(UnicodeNameToCodepointDict + NameOffset, Size);
  } अन्यथा {
    N.Name = StringRef(UnicodeNameToCodepointDict + Size, 1);
  }
  जर (Has_मूल्य) {
    uint8_t H = UnicodeNameToCodepointIndex[Offset++];
    uint8_t M = UnicodeNameToCodepointIndex[Offset++];
    uint8_t L = UnicodeNameToCodepointIndex[Offset++];
    N.मूल्य = ((H << 16) | (M << 8) | L) >> 3;

    तार्किक HasChildren = L & 0x02;
    N.HasSibling = L & 0x01;

    जर (HasChildren) {
      N.ChildrenOffset = UnicodeNameToCodepointIndex[Offset++] << 16;
      N.ChildrenOffset |= UnicodeNameToCodepointIndex[Offset++] << 8;
      N.ChildrenOffset |= UnicodeNameToCodepointIndex[Offset++];
    }
  } अन्यथा {
    uint8_t H = UnicodeNameToCodepointIndex[Offset++];
    N.HasSibling = H & 0x80;
    तार्किक HasChildren = H & 0x40;
    H &= uint8_t(~0xC0);
    जर (HasChildren) {
      N.ChildrenOffset = (H << 16);
      N.ChildrenOffset |=
          (uint32_t(UnicodeNameToCodepointIndex[Offset++]) << 8);
      N.ChildrenOffset |= UnicodeNameToCodepointIndex[Offset++];
    }
  }
  N.Size = Offset - Origin;
  परत N;
}

स्थिर तार्किक startsWith(StringRef Name, StringRef Needle, तार्किक Strict,
                       std::माप_प्रकार &Consummed, अक्षर &PreviousCharInName,
                       तार्किक IsPrefix = असत्य) {

  Consummed = 0;
  जर (Strict) {
    जर (!Name.starts_with(Needle))
      परत असत्य;
    Consummed = Needle.size();
    परत सत्य;
  }
  जर (Needle.empty())
    परत सत्य;

  स्वतः NamePos = Name.begin();
  स्वतः NeedlePos = Needle.begin();

  अक्षर PreviousCharInNameOrigin = PreviousCharInName;
  अक्षर PreviousCharInNeedle = *Needle.begin();
  स्वतः IgnoreSpaces = [](स्वतः It, स्वतः End, अक्षर &PreviousChar,
                         तार्किक IsPrefix = असत्य) {
    पर्यंत (It != End) {
      स्थिरांक स्वतः Next = std::next(It);
      // Ignore spaces, underscore, medial hyphens
      // The generator ensures a needle never ends (or starts) by a medial
      // hyphen https://unicode.org/reports/tr44/#UAX44-LM2.
      तार्किक Ignore =
          *It == ' ' || *It == '_' ||
          (*It == '-' && isAlnum(PreviousChar) &&
           ((Next != End && isAlnum(*Next)) || (Next == End && IsPrefix)));
      PreviousChar = *It;
      जर (!Ignore)
        थांबा;
      ++It;
    }
    परत It;
  };

  पर्यंत (सत्य) {
    NamePos = IgnoreSpaces(NamePos, Name.end(), PreviousCharInName);
    NeedlePos =
        IgnoreSpaces(NeedlePos, Needle.end(), PreviousCharInNeedle, IsPrefix);
    जर (NeedlePos == Needle.end())
      थांबा;
    जर (NamePos == Name.end())
      थांबा;
    जर (toUpper(*NeedlePos) != toUpper(*NamePos))
      थांबा;
    NeedlePos++;
    NamePos++;
  }
  Consummed = std::distance(Name.begin(), NamePos);
  जर (NeedlePos != Needle.end()) {
    PreviousCharInName = PreviousCharInNameOrigin;
  }
  परत NeedlePos == Needle.end();
}

स्थिर std::tuple<गाठ, तार्किक, uint32_t>
compare_गाठ(uint32_t Offset, StringRef Name, तार्किक Strict,
            अक्षर PreviousCharInName, BufferType &Buffer,
            स्थिरांक गाठ *Parent = nullptr) {
  गाठ N = read_गाठ(Offset, Parent);
  std::माप_प्रकार Consummed = 0;
  तार्किक DoesStartWith = N.Is_मूळ || startsWith(Name, N.Name, Strict, Consummed,
                                              PreviousCharInName);
  जर (!DoesStartWith)
    परत std::make_tuple(N, असत्य, 0);

  जर (Name.size() - Consummed == 0 && N.मूल्य != 0xFFFFFFFF)
    परत std::make_tuple(N, सत्य, N.मूल्य);

  जर (N.hasChildren()) {
    uint32_t ChildOffset = N.ChildrenOffset;
    साठी (;;) {
      गाठ C;
      तार्किक Matches;
      uint32_t मूल्य;
      std::tie(C, Matches, मूल्य) =
          compare_गाठ(ChildOffset, Name.substr(Consummed), Strict,
                      PreviousCharInName, Buffer, &N);
      जर (Matches) {
        std::reverse_copy(C.Name.begin(), C.Name.end(),
                          std::back_inserter(Buffer));
        परत std::make_tuple(N, सत्य, मूल्य);
      }
      ChildOffset += C.Size;
      जर (!C.HasSibling)
        थांबा;
    }
  }
  परत std::make_tuple(N, असत्य, 0);
}

स्थिर std::tuple<गाठ, तार्किक, uint32_t>
compare_गाठ(uint32_t Offset, StringRef Name, तार्किक Strict, BufferType &Buffer) {
  परत compare_गाठ(Offset, Name, Strict, 0, Buffer);
}

// clang-format off
constexpr स्थिरांक अक्षर *स्थिरांक HangulSyllables[][3] = {
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
                                अक्षर &PreviousInName, पूर्णांक &Pos, पूर्णांक Column) {
  प्रतिज्ञा(Column == 0 || Column == 1 || Column == 2);
  स्थिर std::माप_प्रकार मोजणी_Per_Column[] = {LCount, VCount, TCount};
  पूर्णांक Len = -1;
  पूर्णांक Prev = PreviousInName;
  साठी (std::माप_प्रकार I = 0; I < मोजणी_Per_Column[Column]; I++) {
    StringRef Syllable(HangulSyllables[I][Column]);
    जर (पूर्णांक(Syllable.size()) <= Len)
      सुरूठेवा;
    std::माप_प्रकार Consummed = 0;
    अक्षर PreviousInNameCopy = PreviousInName;
    तार्किक DoesStartWith =
        startsWith(Name, Syllable, Strict, Consummed, PreviousInNameCopy);
    जर (!DoesStartWith)
      सुरूठेवा;
    Len = Consummed;
    Pos = I;
    Prev = PreviousInNameCopy;
  }
  जर (Len == -1)
    परत 0;
  PreviousInName = Prev;
  परत माप_प्रकार(Len);
}

स्थिर std::optional<char32_t>
nameToHangulCodePoint(StringRef Name, तार्किक Strict, BufferType &Buffer) {
  Buffer.clear();
  // Hangul Syllable Decomposition
  std::माप_प्रकार Consummed = 0;
  अक्षर NameStart = 0;
  तार्किक DoesStartWith =
      startsWith(Name, "HANGUL SYLLABLE ", Strict, Consummed, NameStart);
  जर (!DoesStartWith)
    परत std::nullopt;
  Name = Name.substr(Consummed);
  पूर्णांक L = -1, V = -1, T = -1;
  Name = Name.substr(findSyllable(Name, Strict, NameStart, L, 0));
  Name = Name.substr(findSyllable(Name, Strict, NameStart, V, 1));
  Name = Name.substr(findSyllable(Name, Strict, NameStart, T, 2));
  जर (L != -1 && V != -1 && T != -1 && Name.empty()) {
    जर (!Strict) {
      Buffer.append("HANGUL SYLLABLE ");
      जर (L != -1)
        Buffer.append(HangulSyllables[L][0]);
      जर (V != -1)
        Buffer.append(HangulSyllables[V][1]);
      जर (T != -1)
        Buffer.append(HangulSyllables[T][2]);
    }
    परत SBase + (std::uint32_t(L) * VCount + std::uint32_t(V)) * TCount +
           std::uint32_t(T);
  }
  // Otherwise, it's an illegal syllable name.
  परत std::nullopt;
}

संरचना GeneratedNamesData {
  StringRef Prefix;
  uint32_t Start;
  uint32_t End;
};

// Unicode 18.0 Name Derivation Rule Prefix Strings (extracted/DerivedName.txt)
स्थिर स्थिरांक GeneratedNamesData GeneratedNamesDataTable[] = {
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
  साठी (स्वतः &&Item : GeneratedNamesDataTable) {
    Buffer.clear();
    std::माप_प्रकार Consummed = 0;
    अक्षर NameStart = 0;
    तार्किक DoesStartWith = startsWith(Name, Item.Prefix, Strict, Consummed,
                                    NameStart, /*IsPrefix=*/सत्य);
    जर (!DoesStartWith)
      सुरूठेवा;
    स्वतः संख्या = Name.substr(Consummed);
    अचिन्हित दीर्घ दीर्घ V = 0;
    // Be consistent about mandating upper casing.
    जर (Strict &&
        llvm::any_of(संख्या, [](अक्षर C) { परत C >= 'a' && C <= 'f'; }))
      परत {};
    जर (getAsUnsignedInteger(संख्या, 16, V) || V < Item.Start || V > Item.End)
      सुरूठेवा;
    जर (!Strict) {
      Buffer.append(Item.Prefix);
      Buffer.append(utohexstr(V, सत्य));
    }
    परत V;
  }
  परत std::nullopt;
}

स्थिर std::optional<char32_t> nameToCodepoint(StringRef Name, तार्किक Strict,
                                               BufferType &Buffer) {
  जर (Name.empty())
    परत std::nullopt;

  std::optional<char32_t> Res = nameToHangulCodePoint(Name, Strict, Buffer);
  जर (!Res)
    Res = nameToGeneratedCodePoint(Name, Strict, Buffer);
  जर (Res)
    परत *Res;

  Buffer.clear();
  गाठ गाठ;
  तार्किक Matches;
  uint32_t मूल्य;
  std::tie(गाठ, Matches, मूल्य) = compare_गाठ(0, Name, Strict, Buffer);
  जर (Matches) {
    std::reverse(Buffer.begin(), Buffer.end());
    // UAX44-LM2. Ignore case, whitespace, underscore ('_'), and all medial
    // hyphens except the hyphen in U+1180 HANGUL JUNGSEONG O-E.
    जर (!Strict && मूल्य == 0x116c && Name.contains_insensitive("O-E")) {
      Buffer = "HANGUL JUNGSEONG O-E";
      मूल्य = 0x1180;
    }
    परत मूल्य;
  }
  परत std::nullopt;
}

std::optional<char32_t> nameToCodepointStrict(StringRef Name) {

  BufferType Buffer;
  स्वतः Opt = nameToCodepoint(Name, सत्य, Buffer);
  परत Opt;
}

std::optional<LooseMatchingResult>
nameToCodepointLooseMatching(StringRef Name) {
  BufferType Buffer;
  स्वतः Opt = nameToCodepoint(Name, असत्य, Buffer);
  जर (!Opt)
    परत std::nullopt;
  परत LooseMatchingResult{*Opt, Buffer};
}

// Find the unicode character whose editing distance to Pattern
// is shortest, using the Wagner–Fischer algorithm.
llvm::SmallVector<MatchForCodepointName>
nearestMatchesForCodepointName(StringRef Pattern, std::माप_प्रकार Max_Matches_मोजणी) {
  // We maintain a fixed size vector of matches,
  // sorted by distance
  // The worst match (with the biggest distance) are discarded when new elements
  // are added.
  std::माप_प्रकार LargestEditDistance = 0;
  llvm::SmallVector<MatchForCodepointName> Matches;
  Matches.reserve(Max_Matches_मोजणी + 1);

  स्वतः घाला_2 = [&](स्थिरांक गाठ &गाठ, uint32_t Distance,
                    char32_t मूल्य) -> तार्किक {
    जर (Distance > LargestEditDistance) {
      जर (Matches.size() == Max_Matches_मोजणी)
        परत असत्य;
      LargestEditDistance = Distance;
    }
    // To avoid allocations, the creation of the name is delayed
    // as much as possible.
    std::string Name;
    स्वतः GetName = [&] {
      जर (Name.empty())
        Name = गाठ.fullName();
      परत Name;
    };

    स्वतः It = llvm::lower_bound(
        Matches, Distance,
        [&](स्थिरांक MatchForCodepointName &a, std::माप_प्रकार Distance) {
          जर (Distance == a.Distance)
            परत a.Name < GetName();
          परत a.Distance < Distance;
        });
    जर (It == Matches.end() && Matches.size() == Max_Matches_मोजणी)
      परत असत्य;

    MatchForCodepointName M{GetName(), Distance, मूल्य};
    Matches.घाला(It, std::move(M));
    जर (Matches.size() > Max_Matches_मोजणी)
      Matches.काढा_back();
    परत सत्य;
  };

  // We ignore case, space, hyphens, etc,
  // in both the search pattern and the prospective names.
  स्वतः Normalize = [](StringRef Name) {
    std::string Out;
    Out.reserve(Name.size());
    साठी (अक्षर C : Name) {
      जर (isAlnum(C))
        Out.ढकला_back(toUpper(C));
    }
    परत Out;
  };
  std::string NormalizedName = Normalize(Pattern);

  // Allocate a matrix big enough for longest names.
  स्थिरांक std::माप_प्रकार Columns =
      std::min(NormalizedName.size(), UnicodeNameToCodepointLargestNameSize) +
      1;

  [[maybe_unused]] स्थिर std::माप_प्रकार Rows =
      UnicodeNameToCodepointLargestNameSize + 1;

  std::vector<अक्षर> Distances(
      Columns * (UnicodeNameToCodepointLargestNameSize + 1), 0);

  स्वतः Get = [&Distances, Columns](माप_प्रकार Column, std::माप_प्रकार Row) -> अक्षर & {
    प्रतिज्ञा(Column < Columns);
    प्रतिज्ञा(Row < Rows);
    परत Distances[Row * Columns + Column];
  };

  साठी (std::माप_प्रकार I = 0; I < Columns; I++)
    Get(I, 0) = I;

  // Visit the childrens,
  // Filling (and overriding) the matrix for the name fragment of each node
  // iteratively. CompleteName is used to collect the actual name of potential
  // match, respecting case and spacing.
  स्वतः Visit_गाठ = [&](स्थिरांक गाठ &N, std::माप_प्रकार Row,
                       स्वतः &Visit_गाठ) -> शून्य {
    std::माप_प्रकार J = 0;
    साठी (; J < N.Name.size(); J++) {
      जर (!isAlnum(N.Name[J]))
        सुरूठेवा;

      Get(0, Row) = Row;

      साठी (std::माप_प्रकार I = 1; I < Columns; I++) {
        स्थिरांक पूर्णांक Delete = Get(I - 1, Row) + 1;
        स्थिरांक पूर्णांक घाला_2 = Get(I, Row - 1) + 1;

        स्थिरांक पूर्णांक Replace =
            Get(I - 1, Row - 1) + (NormalizedName[I - 1] != N.Name[J] ? 1 : 0);

        Get(I, Row) = std::min({घाला_2, Delete, Replace});
      }

      Row++;
    }

    अचिन्हित Cost = Get(Columns - 1, Row - 1);
    जर (N.मूल्य != 0xFFFFFFFF) {
      घाला_2(N, Cost, N.मूल्य);
    }

    जर (N.hasChildren()) {
      स्वतः ChildOffset = N.ChildrenOffset;
      साठी (;;) {
        गाठ C = read_गाठ(ChildOffset, &N);
        ChildOffset += C.Size;
        जर (!C.isValid())
          थांबा;
        Visit_गाठ(C, Row, Visit_गाठ);
        जर (!C.HasSibling)
          थांबा;
      }
    }
  };

  गाठ मूळ_2 = create_मूळ();
  Visit_गाठ(मूळ_2, 1, Visit_गाठ);
  परत Matches;
}

} // namespace unicode

} // namespace sys
} // namespace llvm
