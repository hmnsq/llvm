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

#ضمن "llvm/ADT/STLExtras.h"
#ضمن "llvm/ADT/StringExtras.h"
#ضمن "llvm/ADT/StringRef.h"
#ضمن "llvm/Support/Unicode.h"

نطاق llvm {
نطاق sys {
نطاق unicode {

خارجي ثابت حرف *ثابت UnicodeNameToCodepointDict;
خارجي ثابت uint8_t *ثابت UnicodeNameToCodepointIndex;
خارجي ثابت std::size_t UnicodeNameToCodepointIndexSize;
خارجي ثابت std::size_t UnicodeNameToCodepointLargestNameSize;

using BufferType = SmallString<64>;

بنية Node {
  منطقي IsRoot = خطأ;
  char32_t Value = 0xFFFFFFFF;
  uint32_t ChildrenOffset = 0;
  منطقي HasSibling = خطأ;
  uint32_t Size = 0;
  StringRef Name;
  ثابت Node *Parent = nullptr;

  constexpr منطقي isValid() ثابت {
    أرجع !Name.empty() || Value == 0xFFFFFFFF;
  }
  constexpr منطقي hasChildren() ثابت { أرجع ChildrenOffset != 0 || IsRoot; }

  std::string fullName() ثابت {
    std::string S;
    // Reserve enough space for most unicode code points.
    // The chosen value represent the 99th percentile of name size as of
    // Unicode 15.0.
    S.reserve(46);
    ثابت Node *N = هذا;
    طالما (N) {
      std::reverse_copy(N->Name.begin(), N->Name.end(), std::back_inserter(S));
      N = N->Parent;
    }
    std::reverse(S.begin(), S.end());
    أرجع S;
  }
};

ساكن Node createRoot() {
  Node N;
  N.IsRoot = true;
  N.ChildrenOffset = 1;
  N.Size = 1;
  أرجع N;
}

ساكن Node readNode(uint32_t Offset, ثابت Node *Parent = nullptr) {
  𐤀𐤌 (Offset == 0)
    أرجع createRoot();

  uint32_t Origin = Offset;
  Node N;
  N.Parent = Parent;
  uint8_t NameInfo = UnicodeNameToCodepointIndex[Offset++];
  𐤀𐤌 (Offset + 6 >= UnicodeNameToCodepointIndexSize)
    أرجع N;

  منطقي LongName = NameInfo & 0x40;
  منطقي HasValue = NameInfo & 0x80;
  std::size_t Size = NameInfo & ~0xC0;
  𐤀𐤌 (LongName) {
    uint32_t NameOffset = (UnicodeNameToCodepointIndex[Offset++] << 8);
    NameOffset |= UnicodeNameToCodepointIndex[Offset++];
    N.Name = StringRef(UnicodeNameToCodepointDict + NameOffset, Size);
  } وإلا {
    N.Name = StringRef(UnicodeNameToCodepointDict + Size, 1);
  }
  𐤀𐤌 (HasValue) {
    uint8_t H = UnicodeNameToCodepointIndex[Offset++];
    uint8_t M = UnicodeNameToCodepointIndex[Offset++];
    uint8_t L = UnicodeNameToCodepointIndex[Offset++];
    N.Value = ((H << 16) | (M << 8) | L) >> 3;

    منطقي HasChildren = L & 0x02;
    N.HasSibling = L & 0x01;

    𐤀𐤌 (HasChildren) {
      N.ChildrenOffset = UnicodeNameToCodepointIndex[Offset++] << 16;
      N.ChildrenOffset |= UnicodeNameToCodepointIndex[Offset++] << 8;
      N.ChildrenOffset |= UnicodeNameToCodepointIndex[Offset++];
    }
  } وإلا {
    uint8_t H = UnicodeNameToCodepointIndex[Offset++];
    N.HasSibling = H & 0x80;
    منطقي HasChildren = H & 0x40;
    H &= uint8_t(~0xC0);
    𐤀𐤌 (HasChildren) {
      N.ChildrenOffset = (H << 16);
      N.ChildrenOffset |=
          (uint32_t(UnicodeNameToCodepointIndex[Offset++]) << 8);
      N.ChildrenOffset |= UnicodeNameToCodepointIndex[Offset++];
    }
  }
  N.Size = Offset - Origin;
  أرجع N;
}

ساكن منطقي startsWith(StringRef Name, StringRef Needle, منطقي Strict,
                       std::size_t &Consummed, حرف &PreviousCharInName,
                       منطقي IsPrefix = خطأ) {

  Consummed = 0;
  𐤀𐤌 (Strict) {
    𐤀𐤌 (!Name.starts_with(Needle))
      أرجع خطأ;
    Consummed = Needle.size();
    أرجع true;
  }
  𐤀𐤌 (Needle.empty())
    أرجع true;

  auto NamePos = Name.begin();
  auto NeedlePos = Needle.begin();

  حرف PreviousCharInNameOrigin = PreviousCharInName;
  حرف PreviousCharInNeedle = *Needle.begin();
  auto IgnoreSpaces = [](auto It, auto End, حرف &PreviousChar,
                         منطقي IsPrefix = خطأ) {
    طالما (It != End) {
      ثابت auto Next = std::next(It);
      // Ignore spaces, underscore, medial hyphens
      // The generator ensures a needle never ends (or starts) by a medial
      // hyphen https://unicode.org/reports/tr44/#UAX44-LM2.
      منطقي Ignore =
          *It == ' ' || *It == '_' ||
          (*It == '-' && isAlnum(PreviousChar) &&
           ((Next != End && isAlnum(*Next)) || (Next == End && IsPrefix)));
      PreviousChar = *It;
      𐤀𐤌 (!Ignore)
        اكسر;
      ++It;
    }
    أرجع It;
  };

  طالما (true) {
    NamePos = IgnoreSpaces(NamePos, Name.end(), PreviousCharInName);
    NeedlePos =
        IgnoreSpaces(NeedlePos, Needle.end(), PreviousCharInNeedle, IsPrefix);
    𐤀𐤌 (NeedlePos == Needle.end())
      اكسر;
    𐤀𐤌 (NamePos == Name.end())
      اكسر;
    𐤀𐤌 (toUpper(*NeedlePos) != toUpper(*NamePos))
      اكسر;
    NeedlePos++;
    NamePos++;
  }
  Consummed = std::distance(Name.begin(), NamePos);
  𐤀𐤌 (NeedlePos != Needle.end()) {
    PreviousCharInName = PreviousCharInNameOrigin;
  }
  أرجع NeedlePos == Needle.end();
}

ساكن std::tuple<Node, منطقي, uint32_t>
compareNode(uint32_t Offset, StringRef Name, منطقي Strict,
            حرف PreviousCharInName, BufferType &Buffer,
            ثابت Node *Parent = nullptr) {
  Node N = readNode(Offset, Parent);
  std::size_t Consummed = 0;
  منطقي DoesStartWith = N.IsRoot || startsWith(Name, N.Name, Strict, Consummed,
                                              PreviousCharInName);
  𐤀𐤌 (!DoesStartWith)
    أرجع std::make_tuple(N, خطأ, 0);

  𐤀𐤌 (Name.size() - Consummed == 0 && N.Value != 0xFFFFFFFF)
    أرجع std::make_tuple(N, true, N.Value);

  𐤀𐤌 (N.hasChildren()) {
    uint32_t ChildOffset = N.ChildrenOffset;
    لكل (;;) {
      Node C;
      منطقي Matches;
      uint32_t Value;
      std::tie(C, Matches, Value) =
          compareNode(ChildOffset, Name.substr(Consummed), Strict,
                      PreviousCharInName, Buffer, &N);
      𐤀𐤌 (Matches) {
        std::reverse_copy(C.Name.begin(), C.Name.end(),
                          std::back_inserter(Buffer));
        أرجع std::make_tuple(N, true, Value);
      }
      ChildOffset += C.Size;
      𐤀𐤌 (!C.HasSibling)
        اكسر;
    }
  }
  أرجع std::make_tuple(N, خطأ, 0);
}

ساكن std::tuple<Node, منطقي, uint32_t>
compareNode(uint32_t Offset, StringRef Name, منطقي Strict, BufferType &Buffer) {
  أرجع compareNode(Offset, Name, Strict, 0, Buffer);
}

// clang-format off
constexpr ثابت حرف *ثابت HangulSyllables[][3] = {
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

ساكن std::size_t findSyllable(StringRef Name, منطقي Strict,
                                حرف &PreviousInName, صحيح &Pos, صحيح Column) {
  أكد(Column == 0 || Column == 1 || Column == 2);
  ساكن std::size_t CountPerColumn[] = {LCount, VCount, TCount};
  صحيح Len = -1;
  صحيح Prev = PreviousInName;
  لكل (std::size_t I = 0; I < CountPerColumn[Column]; I++) {
    StringRef Syllable(HangulSyllables[I][Column]);
    𐤀𐤌 (صحيح(Syllable.size()) <= Len)
      استمر;
    std::size_t Consummed = 0;
    حرف PreviousInNameCopy = PreviousInName;
    منطقي DoesStartWith =
        startsWith(Name, Syllable, Strict, Consummed, PreviousInNameCopy);
    𐤀𐤌 (!DoesStartWith)
      استمر;
    Len = Consummed;
    Pos = I;
    Prev = PreviousInNameCopy;
  }
  𐤀𐤌 (Len == -1)
    أرجع 0;
  PreviousInName = Prev;
  أرجع size_t(Len);
}

ساكن std::optional<char32_t>
nameToHangulCodePoint(StringRef Name, منطقي Strict, BufferType &Buffer) {
  Buffer.clear();
  // Hangul Syllable Decomposition
  std::size_t Consummed = 0;
  حرف NameStart = 0;
  منطقي DoesStartWith =
      startsWith(Name, "HANGUL SYLLABLE ", Strict, Consummed, NameStart);
  𐤀𐤌 (!DoesStartWith)
    أرجع std::nullopt;
  Name = Name.substr(Consummed);
  صحيح L = -1, V = -1, T = -1;
  Name = Name.substr(findSyllable(Name, Strict, NameStart, L, 0));
  Name = Name.substr(findSyllable(Name, Strict, NameStart, V, 1));
  Name = Name.substr(findSyllable(Name, Strict, NameStart, T, 2));
  𐤀𐤌 (L != -1 && V != -1 && T != -1 && Name.empty()) {
    𐤀𐤌 (!Strict) {
      Buffer.append("HANGUL SYLLABLE ");
      𐤀𐤌 (L != -1)
        Buffer.append(HangulSyllables[L][0]);
      𐤀𐤌 (V != -1)
        Buffer.append(HangulSyllables[V][1]);
      𐤀𐤌 (T != -1)
        Buffer.append(HangulSyllables[T][2]);
    }
    أرجع SBase + (std::uint32_t(L) * VCount + std::uint32_t(V)) * TCount +
           std::uint32_t(T);
  }
  // Otherwise, it's an illegal syllable name.
  أرجع std::nullopt;
}

بنية GeneratedNamesData {
  StringRef Prefix;
  uint32_t Start;
  uint32_t End;
};

// Unicode 18.0 Name Derivation Rule Prefix Strings (extracted/DerivedName.txt)
ساكن ثابت GeneratedNamesData GeneratedNamesDataTable[] = {
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

ساكن std::optional<char32_t>
nameToGeneratedCodePoint(StringRef Name, منطقي Strict, BufferType &Buffer) {
  لكل (auto &&Item : GeneratedNamesDataTable) {
    Buffer.clear();
    std::size_t Consummed = 0;
    حرف NameStart = 0;
    منطقي DoesStartWith = startsWith(Name, Item.Prefix, Strict, Consummed,
                                    NameStart, /*IsPrefix=*/true);
    𐤀𐤌 (!DoesStartWith)
      استمر;
    auto Number = Name.substr(Consummed);
    unsigned long long V = 0;
    // Be consistent about mandating upper casing.
    𐤀𐤌 (Strict &&
        llvm::any_of(Number, [](حرف C) { أرجع C >= 'a' && C <= 'f'; }))
      أرجع {};
    𐤀𐤌 (getAsUnsignedInteger(Number, 16, V) || V < Item.Start || V > Item.End)
      استمر;
    𐤀𐤌 (!Strict) {
      Buffer.append(Item.Prefix);
      Buffer.append(utohexstr(V, true));
    }
    أرجع V;
  }
  أرجع std::nullopt;
}

ساكن std::optional<char32_t> nameToCodepoint(StringRef Name, منطقي Strict,
                                               BufferType &Buffer) {
  𐤀𐤌 (Name.empty())
    أرجع std::nullopt;

  std::optional<char32_t> Res = nameToHangulCodePoint(Name, Strict, Buffer);
  𐤀𐤌 (!Res)
    Res = nameToGeneratedCodePoint(Name, Strict, Buffer);
  𐤀𐤌 (Res)
    أرجع *Res;

  Buffer.clear();
  Node Node;
  منطقي Matches;
  uint32_t Value;
  std::tie(Node, Matches, Value) = compareNode(0, Name, Strict, Buffer);
  𐤀𐤌 (Matches) {
    std::reverse(Buffer.begin(), Buffer.end());
    // UAX44-LM2. Ignore case, whitespace, underscore ('_'), and all medial
    // hyphens except the hyphen in U+1180 HANGUL JUNGSEONG O-E.
    𐤀𐤌 (!Strict && Value == 0x116c && Name.contains_insensitive("O-E")) {
      Buffer = "HANGUL JUNGSEONG O-E";
      Value = 0x1180;
    }
    أرجع Value;
  }
  أرجع std::nullopt;
}

std::optional<char32_t> nameToCodepointStrict(StringRef Name) {

  BufferType Buffer;
  auto Opt = nameToCodepoint(Name, true, Buffer);
  أرجع Opt;
}

std::optional<LooseMatchingResult>
nameToCodepointLooseMatching(StringRef Name) {
  BufferType Buffer;
  auto Opt = nameToCodepoint(Name, خطأ, Buffer);
  𐤀𐤌 (!Opt)
    أرجع std::nullopt;
  أرجع LooseMatchingResult{*Opt, Buffer};
}

// Find the unicode character whose editing distance to Pattern
// is shortest, using the Wagner–Fischer algorithm.
llvm::SmallVector<MatchForCodepointName>
nearestMatchesForCodepointName(StringRef Pattern, std::size_t MaxMatchesCount) {
  // We maintain a fixed size vector of matches,
  // sorted by distance
  // The worst match (with the biggest distance) are discarded when new elements
  // are added.
  std::size_t LargestEditDistance = 0;
  llvm::SmallVector<MatchForCodepointName> Matches;
  Matches.reserve(MaxMatchesCount + 1);

  auto Insert = [&](ثابت Node &Node, uint32_t Distance,
                    char32_t Value) -> منطقي {
    𐤀𐤌 (Distance > LargestEditDistance) {
      𐤀𐤌 (Matches.size() == MaxMatchesCount)
        أرجع خطأ;
      LargestEditDistance = Distance;
    }
    // To avoid allocations, the creation of the name is delayed
    // as much as possible.
    std::string Name;
    auto GetName = [&] {
      𐤀𐤌 (Name.empty())
        Name = Node.fullName();
      أرجع Name;
    };

    auto It = llvm::lower_bound(
        Matches, Distance,
        [&](ثابت MatchForCodepointName &a, std::size_t Distance) {
          𐤀𐤌 (Distance == a.Distance)
            أرجع a.Name < GetName();
          أرجع a.Distance < Distance;
        });
    𐤀𐤌 (It == Matches.end() && Matches.size() == MaxMatchesCount)
      أرجع خطأ;

    MatchForCodepointName M{GetName(), Distance, Value};
    Matches.insert(It, std::move(M));
    𐤀𐤌 (Matches.size() > MaxMatchesCount)
      Matches.pop_back();
    أرجع true;
  };

  // We ignore case, space, hyphens, etc,
  // in both the search pattern and the prospective names.
  auto Normalize = [](StringRef Name) {
    std::string Out;
    Out.reserve(Name.size());
    لكل (حرف C : Name) {
      𐤀𐤌 (isAlnum(C))
        Out.push_back(toUpper(C));
    }
    أرجع Out;
  };
  std::string NormalizedName = Normalize(Pattern);

  // Allocate a matrix big enough for longest names.
  ثابت std::size_t Columns =
      std::min(NormalizedName.size(), UnicodeNameToCodepointLargestNameSize) +
      1;

  [[maybe_unused]] ساكن std::size_t Rows =
      UnicodeNameToCodepointLargestNameSize + 1;

  std::vector<حرف> Distances(
      Columns * (UnicodeNameToCodepointLargestNameSize + 1), 0);

  auto Get = [&Distances, Columns](size_t Column, std::size_t Row) -> حرف & {
    أكد(Column < Columns);
    أكد(Row < Rows);
    أرجع Distances[Row * Columns + Column];
  };

  لكل (std::size_t I = 0; I < Columns; I++)
    Get(I, 0) = I;

  // Visit the childrens,
  // Filling (and overriding) the matrix for the name fragment of each node
  // iteratively. CompleteName is used to collect the actual name of potential
  // match, respecting case and spacing.
  auto VisitNode = [&](ثابت Node &N, std::size_t Row,
                       auto &VisitNode) -> فراغ {
    std::size_t J = 0;
    لكل (; J < N.Name.size(); J++) {
      𐤀𐤌 (!isAlnum(N.Name[J]))
        استمر;

      Get(0, Row) = Row;

      لكل (std::size_t I = 1; I < Columns; I++) {
        ثابت صحيح Delete = Get(I - 1, Row) + 1;
        ثابت صحيح Insert = Get(I, Row - 1) + 1;

        ثابت صحيح Replace =
            Get(I - 1, Row - 1) + (NormalizedName[I - 1] != N.Name[J] ? 1 : 0);

        Get(I, Row) = std::min({Insert, Delete, Replace});
      }

      Row++;
    }

    unsigned Cost = Get(Columns - 1, Row - 1);
    𐤀𐤌 (N.Value != 0xFFFFFFFF) {
      Insert(N, Cost, N.Value);
    }

    𐤀𐤌 (N.hasChildren()) {
      auto ChildOffset = N.ChildrenOffset;
      لكل (;;) {
        Node C = readNode(ChildOffset, &N);
        ChildOffset += C.Size;
        𐤀𐤌 (!C.isValid())
          اكسر;
        VisitNode(C, Row, VisitNode);
        𐤀𐤌 (!C.HasSibling)
          اكسر;
      }
    }
  };

  Node Root = createRoot();
  VisitNode(Root, 1, VisitNode);
  أرجع Matches;
}

} // namespace unicode

} // namespace sys
} // namespace llvm
