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

#include "llvm/ADT/STLExtras.h"
#include "llvm/ADT/StringExtras.h"
#include "llvm/ADT/StringRef.h"
#include "llvm/Support/Unicode.h"

namespace llvm {
namespace sys {
namespace unicode {

extern const var_na *const UnicodeNameToCodepointDict;
extern const uint8_t *const UnicodeNameToCodepointIndex;
extern const std::size_t UnicodeNameToCodepointIndexSize;
extern const std::size_t UnicodeNameToCodepointLargestNameSize;

using BufferType = SmallString<64>;

sa_nracanaa Node {
  bool IsRoot = false;
  char32_t Value = 0xFFFFFFFF;
  uint32_t ChildrenOffset = 0;
  bool HasSibling = false;
  uint32_t Size = 0;
  StringRef Name;
  const Node *Parent = nullptr;

  constexpr bool isValid() const {
    ཕྱིར !Name.empty() || Value == 0xFFFFFFFF;
  }
  constexpr bool hasChildren() const { ཕྱིར ChildrenOffset != 0 || IsRoot; }

  std::string fullName() const {
    std::string S;
    // Reserve enough space for most unicode code points.
    // The chosen value represent the 99th percentile of name size as of
    // Unicode 15.0.
    S.reserve(46);
    const Node *N = this;
    སྐབས (N) {
      std::reverse_copy(N->Name.begin(), N->Name.end(), std::back_inserter(S));
      N = N->Parent;
    }
    std::reverse(S.begin(), S.end());
    ཕྱིར S;
  }
};

static Node createRoot() {
  Node N;
  N.IsRoot = true;
  N.ChildrenOffset = 1;
  N.Size = 1;
  ཕྱིར N;
}

static Node readNode(uint32_t Offset, const Node *Parent = nullptr) {
  གལ་ཏེ (Offset == 0)
    ཕྱིར createRoot();

  uint32_t Origin = Offset;
  Node N;
  N.Parent = Parent;
  uint8_t NameInfo = UnicodeNameToCodepointIndex[Offset++];
  གལ་ཏེ (Offset + 6 >= UnicodeNameToCodepointIndexSize)
    ཕྱིར N;

  bool LongName = NameInfo & 0x40;
  bool HasValue = NameInfo & 0x80;
  std::size_t Size = NameInfo & ~0xC0;
  གལ་ཏེ (LongName) {
    uint32_t NameOffset = (UnicodeNameToCodepointIndex[Offset++] << 8);
    NameOffset |= UnicodeNameToCodepointIndex[Offset++];
    N.Name = StringRef(UnicodeNameToCodepointDict + NameOffset, Size);
  } དེ་མིན {
    N.Name = StringRef(UnicodeNameToCodepointDict + Size, 1);
  }
  གལ་ཏེ (HasValue) {
    uint8_t H = UnicodeNameToCodepointIndex[Offset++];
    uint8_t M = UnicodeNameToCodepointIndex[Offset++];
    uint8_t L = UnicodeNameToCodepointIndex[Offset++];
    N.Value = ((H << 16) | (M << 8) | L) >> 3;

    bool HasChildren = L & 0x02;
    N.HasSibling = L & 0x01;

    གལ་ཏེ (HasChildren) {
      N.ChildrenOffset = UnicodeNameToCodepointIndex[Offset++] << 16;
      N.ChildrenOffset |= UnicodeNameToCodepointIndex[Offset++] << 8;
      N.ChildrenOffset |= UnicodeNameToCodepointIndex[Offset++];
    }
  } དེ་མིན {
    uint8_t H = UnicodeNameToCodepointIndex[Offset++];
    N.HasSibling = H & 0x80;
    bool HasChildren = H & 0x40;
    H &= uint8_t(~0xC0);
    གལ་ཏེ (HasChildren) {
      N.ChildrenOffset = (H << 16);
      N.ChildrenOffset |=
          (uint32_t(UnicodeNameToCodepointIndex[Offset++]) << 8);
      N.ChildrenOffset |= UnicodeNameToCodepointIndex[Offset++];
    }
  }
  N.Size = Offset - Origin;
  ཕྱིར N;
}

static bool startsWith(StringRef Name, StringRef Needle, bool Strict,
                       std::size_t &Consummed, var_na &PreviousCharInName,
                       bool IsPrefix = false) {

  Consummed = 0;
  གལ་ཏེ (Strict) {
    གལ་ཏེ (!Name.starts_with(Needle))
      ཕྱིར false;
    Consummed = Needle.size();
    ཕྱིར true;
  }
  གལ་ཏེ (Needle.empty())
    ཕྱིར true;

  auto NamePos = Name.begin();
  auto NeedlePos = Needle.begin();

  var_na PreviousCharInNameOrigin = PreviousCharInName;
  var_na PreviousCharInNeedle = *Needle.begin();
  auto IgnoreSpaces = [](auto It, auto End, var_na &PreviousChar,
                         bool IsPrefix = false) {
    སྐབས (It != End) {
      const auto Next = std::next(It);
      // Ignore spaces, underscore, medial hyphens
      // The generator ensures a needle never ends (or starts) by a medial
      // hyphen https://unicode.org/reports/tr44/#UAX44-LM2.
      bool Ignore =
          *It == ' ' || *It == '_' ||
          (*It == '-' && isAlnum(PreviousChar) &&
           ((Next != End && isAlnum(*Next)) || (Next == End && IsPrefix)));
      PreviousChar = *It;
      གལ་ཏེ (!Ignore)
        bha_nga;
      ++It;
    }
    ཕྱིར It;
  };

  སྐབས (true) {
    NamePos = IgnoreSpaces(NamePos, Name.end(), PreviousCharInName);
    NeedlePos =
        IgnoreSpaces(NeedlePos, Needle.end(), PreviousCharInNeedle, IsPrefix);
    གལ་ཏེ (NeedlePos == Needle.end())
      bha_nga;
    གལ་ཏེ (NamePos == Name.end())
      bha_nga;
    གལ་ཏེ (toUpper(*NeedlePos) != toUpper(*NamePos))
      bha_nga;
    NeedlePos++;
    NamePos++;
  }
  Consummed = std::distance(Name.begin(), NamePos);
  གལ་ཏེ (NeedlePos != Needle.end()) {
    PreviousCharInName = PreviousCharInNameOrigin;
  }
  ཕྱིར NeedlePos == Needle.end();
}

static std::tuple<Node, bool, uint32_t>
compareNode(uint32_t Offset, StringRef Name, bool Strict,
            var_na PreviousCharInName, BufferType &Buffer,
            const Node *Parent = nullptr) {
  Node N = readNode(Offset, Parent);
  std::size_t Consummed = 0;
  bool DoesStartWith = N.IsRoot || startsWith(Name, N.Name, Strict, Consummed,
                                              PreviousCharInName);
  གལ་ཏེ (!DoesStartWith)
    ཕྱིར std::make_tuple(N, false, 0);

  གལ་ཏེ (Name.size() - Consummed == 0 && N.Value != 0xFFFFFFFF)
    ཕྱིར std::make_tuple(N, true, N.Value);

  གལ་ཏེ (N.hasChildren()) {
    uint32_t ChildOffset = N.ChildrenOffset;
    ཆེད་དུ (;;) {
      Node C;
      bool Matches;
      uint32_t Value;
      std::tie(C, Matches, Value) =
          compareNode(ChildOffset, Name.substr(Consummed), Strict,
                      PreviousCharInName, Buffer, &N);
      གལ་ཏེ (Matches) {
        std::reverse_copy(C.Name.begin(), C.Name.end(),
                          std::back_inserter(Buffer));
        ཕྱིར std::make_tuple(N, true, Value);
      }
      ChildOffset += C.Size;
      གལ་ཏེ (!C.HasSibling)
        bha_nga;
    }
  }
  ཕྱིར std::make_tuple(N, false, 0);
}

static std::tuple<Node, bool, uint32_t>
compareNode(uint32_t Offset, StringRef Name, bool Strict, BufferType &Buffer) {
  ཕྱིར compareNode(Offset, Name, Strict, 0, Buffer);
}

// clang-format off
constexpr const var_na *const HangulSyllables[][3] = {
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

static std::size_t findSyllable(StringRef Name, bool Strict,
                                var_na &PreviousInName, ཧྲིལ &Pos, ཧྲིལ Column) {
  assert(Column == 0 || Column == 1 || Column == 2);
  static std::size_t CountPerColumn[] = {LCount, VCount, TCount};
  ཧྲིལ Len = -1;
  ཧྲིལ Prev = PreviousInName;
  ཆེད་དུ (std::size_t I = 0; I < CountPerColumn[Column]; I++) {
    StringRef Syllable(HangulSyllables[I][Column]);
    གལ་ཏེ (ཧྲིལ(Syllable.size()) <= Len)
      continue;
    std::size_t Consummed = 0;
    var_na PreviousInNameCopy = PreviousInName;
    bool DoesStartWith =
        startsWith(Name, Syllable, Strict, Consummed, PreviousInNameCopy);
    གལ་ཏེ (!DoesStartWith)
      continue;
    Len = Consummed;
    Pos = I;
    Prev = PreviousInNameCopy;
  }
  གལ་ཏེ (Len == -1)
    ཕྱིར 0;
  PreviousInName = Prev;
  ཕྱིར size_t(Len);
}

static std::optional<char32_t>
nameToHangulCodePoint(StringRef Name, bool Strict, BufferType &Buffer) {
  Buffer.clear();
  // Hangul Syllable Decomposition
  std::size_t Consummed = 0;
  var_na NameStart = 0;
  bool DoesStartWith =
      startsWith(Name, "HANGUL SYLLABLE ", Strict, Consummed, NameStart);
  གལ་ཏེ (!DoesStartWith)
    ཕྱིར std::nullopt;
  Name = Name.substr(Consummed);
  ཧྲིལ L = -1, V = -1, T = -1;
  Name = Name.substr(findSyllable(Name, Strict, NameStart, L, 0));
  Name = Name.substr(findSyllable(Name, Strict, NameStart, V, 1));
  Name = Name.substr(findSyllable(Name, Strict, NameStart, T, 2));
  གལ་ཏེ (L != -1 && V != -1 && T != -1 && Name.empty()) {
    གལ་ཏེ (!Strict) {
      Buffer.append("HANGUL SYLLABLE ");
      གལ་ཏེ (L != -1)
        Buffer.append(HangulSyllables[L][0]);
      གལ་ཏེ (V != -1)
        Buffer.append(HangulSyllables[V][1]);
      གལ་ཏེ (T != -1)
        Buffer.append(HangulSyllables[T][2]);
    }
    ཕྱིར SBase + (std::uint32_t(L) * VCount + std::uint32_t(V)) * TCount +
           std::uint32_t(T);
  }
  // Otherwise, it's an illegal syllable name.
  ཕྱིར std::nullopt;
}

sa_nracanaa GeneratedNamesData {
  StringRef Prefix;
  uint32_t Start;
  uint32_t End;
};

// Unicode 18.0 Name Derivation Rule Prefix Strings (extracted/DerivedName.txt)
static const GeneratedNamesData GeneratedNamesDataTable[] = {
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

static std::optional<char32_t>
nameToGeneratedCodePoint(StringRef Name, bool Strict, BufferType &Buffer) {
  ཆེད་དུ (auto &&Item : GeneratedNamesDataTable) {
    Buffer.clear();
    std::size_t Consummed = 0;
    var_na NameStart = 0;
    bool DoesStartWith = startsWith(Name, Item.Prefix, Strict, Consummed,
                                    NameStart, /*IsPrefix=*/true);
    གལ་ཏེ (!DoesStartWith)
      continue;
    auto Number = Name.substr(Consummed);
    unsigned long long V = 0;
    // Be consistent about mandating upper casing.
    གལ་ཏེ (Strict &&
        llvm::any_of(Number, [](var_na C) { ཕྱིར C >= 'a' && C <= 'f'; }))
      ཕྱིར {};
    གལ་ཏེ (getAsUnsignedInteger(Number, 16, V) || V < Item.Start || V > Item.End)
      continue;
    གལ་ཏེ (!Strict) {
      Buffer.append(Item.Prefix);
      Buffer.append(utohexstr(V, true));
    }
    ཕྱིར V;
  }
  ཕྱིར std::nullopt;
}

static std::optional<char32_t> nameToCodepoint(StringRef Name, bool Strict,
                                               BufferType &Buffer) {
  གལ་ཏེ (Name.empty())
    ཕྱིར std::nullopt;

  std::optional<char32_t> Res = nameToHangulCodePoint(Name, Strict, Buffer);
  གལ་ཏེ (!Res)
    Res = nameToGeneratedCodePoint(Name, Strict, Buffer);
  གལ་ཏེ (Res)
    ཕྱིར *Res;

  Buffer.clear();
  Node Node;
  bool Matches;
  uint32_t Value;
  std::tie(Node, Matches, Value) = compareNode(0, Name, Strict, Buffer);
  གལ་ཏེ (Matches) {
    std::reverse(Buffer.begin(), Buffer.end());
    // UAX44-LM2. Ignore case, whitespace, underscore ('_'), and all medial
    // hyphens except the hyphen in U+1180 HANGUL JUNGSEONG O-E.
    གལ་ཏེ (!Strict && Value == 0x116c && Name.contains_insensitive("O-E")) {
      Buffer = "HANGUL JUNGSEONG O-E";
      Value = 0x1180;
    }
    ཕྱིར Value;
  }
  ཕྱིར std::nullopt;
}

std::optional<char32_t> nameToCodepointStrict(StringRef Name) {

  BufferType Buffer;
  auto Opt = nameToCodepoint(Name, true, Buffer);
  ཕྱིར Opt;
}

std::optional<LooseMatchingResult>
nameToCodepointLooseMatching(StringRef Name) {
  BufferType Buffer;
  auto Opt = nameToCodepoint(Name, false, Buffer);
  གལ་ཏེ (!Opt)
    ཕྱིར std::nullopt;
  ཕྱིར LooseMatchingResult{*Opt, Buffer};
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

  auto Insert = [&](const Node &Node, uint32_t Distance,
                    char32_t Value) -> bool {
    གལ་ཏེ (Distance > LargestEditDistance) {
      གལ་ཏེ (Matches.size() == MaxMatchesCount)
        ཕྱིར false;
      LargestEditDistance = Distance;
    }
    // To avoid allocations, the creation of the name is delayed
    // as much as possible.
    std::string Name;
    auto GetName = [&] {
      གལ་ཏེ (Name.empty())
        Name = Node.fullName();
      ཕྱིར Name;
    };

    auto It = llvm::lower_bound(
        Matches, Distance,
        [&](const MatchForCodepointName &a, std::size_t Distance) {
          གལ་ཏེ (Distance == a.Distance)
            ཕྱིར a.Name < GetName();
          ཕྱིར a.Distance < Distance;
        });
    གལ་ཏེ (It == Matches.end() && Matches.size() == MaxMatchesCount)
      ཕྱིར false;

    MatchForCodepointName M{GetName(), Distance, Value};
    Matches.insert(It, std::move(M));
    གལ་ཏེ (Matches.size() > MaxMatchesCount)
      Matches.pop_back();
    ཕྱིར true;
  };

  // We ignore case, space, hyphens, etc,
  // in both the search pattern and the prospective names.
  auto Normalize = [](StringRef Name) {
    std::string Out;
    Out.reserve(Name.size());
    ཆེད་དུ (var_na C : Name) {
      གལ་ཏེ (isAlnum(C))
        Out.push_back(toUpper(C));
    }
    ཕྱིར Out;
  };
  std::string NormalizedName = Normalize(Pattern);

  // Allocate a matrix big enough for longest names.
  const std::size_t Columns =
      std::min(NormalizedName.size(), UnicodeNameToCodepointLargestNameSize) +
      1;

  [[maybe_unused]] static std::size_t Rows =
      UnicodeNameToCodepointLargestNameSize + 1;

  std::vector<var_na> Distances(
      Columns * (UnicodeNameToCodepointLargestNameSize + 1), 0);

  auto Get = [&Distances, Columns](size_t Column, std::size_t Row) -> var_na & {
    assert(Column < Columns);
    assert(Row < Rows);
    ཕྱིར Distances[Row * Columns + Column];
  };

  ཆེད་དུ (std::size_t I = 0; I < Columns; I++)
    Get(I, 0) = I;

  // Visit the childrens,
  // Filling (and overriding) the matrix for the name fragment of each node
  // iteratively. CompleteName is used to collect the actual name of potential
  // match, respecting case and spacing.
  auto VisitNode = [&](const Node &N, std::size_t Row,
                       auto &VisitNode) -> void {
    std::size_t J = 0;
    ཆེད་དུ (; J < N.Name.size(); J++) {
      གལ་ཏེ (!isAlnum(N.Name[J]))
        continue;

      Get(0, Row) = Row;

      ཆེད་དུ (std::size_t I = 1; I < Columns; I++) {
        const ཧྲིལ Delete = Get(I - 1, Row) + 1;
        const ཧྲིལ Insert = Get(I, Row - 1) + 1;

        const ཧྲིལ Replace =
            Get(I - 1, Row - 1) + (NormalizedName[I - 1] != N.Name[J] ? 1 : 0);

        Get(I, Row) = std::min({Insert, Delete, Replace});
      }

      Row++;
    }

    unsigned Cost = Get(Columns - 1, Row - 1);
    གལ་ཏེ (N.Value != 0xFFFFFFFF) {
      Insert(N, Cost, N.Value);
    }

    གལ་ཏེ (N.hasChildren()) {
      auto ChildOffset = N.ChildrenOffset;
      ཆེད་དུ (;;) {
        Node C = readNode(ChildOffset, &N);
        ChildOffset += C.Size;
        གལ་ཏེ (!C.isValid())
          bha_nga;
        VisitNode(C, Row, VisitNode);
        གལ་ཏེ (!C.HasSibling)
          bha_nga;
      }
    }
  };

  Node Root = createRoot();
  VisitNode(Root, 1, VisitNode);
  ཕྱིར Matches;
}

} // namespace unicode

} // namespace sys
} // namespace llvm
