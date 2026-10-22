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

#অন্তর্ভাৱ "llvm/ADT/STLExtras.h"
#অন্তর্ভাৱ "llvm/ADT/StringExtras.h"
#অন্তর্ভাৱ "llvm/ADT/StringRef.h"
#অন্তর্ভাৱ "llvm/Support/Unicode.h"

নামাকাশ llvm {
নামাকাশ sys {
নামাকাশ unicode {

বাহ্য ধ্রুৱ ৱর্ণ *ধ্রুৱ UnicodeNameToCodepointDict;
বাহ্য ধ্রুৱ uint8_t *ধ্রুৱ UnicodeNameToCodepointIndex;
বাহ্য ধ্রুৱ std::size_t UnicodeNameToCodepointIndexSize;
বাহ্য ধ্রুৱ std::size_t UnicodeNameToCodepointLargestNameSize;

using BufferType = SmallString<64>;

সংরচনা গ্ৰন্থি {
  তার্কিক Is_মূল = মিছা;
  char32_t মান = 0xFFFFFFFF;
  uint32_t ChildrenOffset = 0;
  তার্কিক HasSibling = মিছা;
  uint32_t Size = 0;
  StringRef Name;
  ধ্রুৱ গ্ৰন্থি *Parent = nullptr;

  constexpr তার্কিক isValid() ধ্রুৱ {
    ঘূৰাই !Name.empty() || মান == 0xFFFFFFFF;
  }
  constexpr তার্কিক hasChildren() ধ্রুৱ { ঘূৰাই ChildrenOffset != 0 || Is_মূল; }

  std::string fullName() ধ্রুৱ {
    std::string S;
    // Reserve enough space for most unicode code points.
    // The chosen value represent the 99th percentile of name size as of
    // Unicode 15.0.
    S.reserve(46);
    ধ্রুৱ গ্ৰন্থি *N = এতত্;
    যেতিয়ালৈ (N) {
      std::reverse_copy(N->Name.begin(), N->Name.end(), std::back_inserter(S));
      N = N->Parent;
    }
    std::reverse(S.begin(), S.end());
    ঘূৰাই S;
  }
};

স্থির গ্ৰন্থি create_মূল() {
  গ্ৰন্থি N;
  N.Is_মূল = সঁচা;
  N.ChildrenOffset = 1;
  N.Size = 1;
  ঘূৰাই N;
}

স্থির গ্ৰন্থি read_গ্ৰন্থি(uint32_t Offset, ধ্রুৱ গ্ৰন্থি *Parent = nullptr) {
  যদি (Offset == 0)
    ঘূৰাই create_মূল();

  uint32_t Origin = Offset;
  গ্ৰন্থি N;
  N.Parent = Parent;
  uint8_t NameInfo = UnicodeNameToCodepointIndex[Offset++];
  যদি (Offset + 6 >= UnicodeNameToCodepointIndexSize)
    ঘূৰাই N;

  তার্কিক LongName = NameInfo & 0x40;
  তার্কিক Has_মান = NameInfo & 0x80;
  std::size_t Size = NameInfo & ~0xC0;
  যদি (LongName) {
    uint32_t NameOffset = (UnicodeNameToCodepointIndex[Offset++] << 8);
    NameOffset |= UnicodeNameToCodepointIndex[Offset++];
    N.Name = StringRef(UnicodeNameToCodepointDict + NameOffset, Size);
  } অন্যথা {
    N.Name = StringRef(UnicodeNameToCodepointDict + Size, 1);
  }
  যদি (Has_মান) {
    uint8_t H = UnicodeNameToCodepointIndex[Offset++];
    uint8_t M = UnicodeNameToCodepointIndex[Offset++];
    uint8_t L = UnicodeNameToCodepointIndex[Offset++];
    N.মান = ((H << 16) | (M << 8) | L) >> 3;

    তার্কিক HasChildren = L & 0x02;
    N.HasSibling = L & 0x01;

    যদি (HasChildren) {
      N.ChildrenOffset = UnicodeNameToCodepointIndex[Offset++] << 16;
      N.ChildrenOffset |= UnicodeNameToCodepointIndex[Offset++] << 8;
      N.ChildrenOffset |= UnicodeNameToCodepointIndex[Offset++];
    }
  } অন্যথা {
    uint8_t H = UnicodeNameToCodepointIndex[Offset++];
    N.HasSibling = H & 0x80;
    তার্কিক HasChildren = H & 0x40;
    H &= uint8_t(~0xC0);
    যদি (HasChildren) {
      N.ChildrenOffset = (H << 16);
      N.ChildrenOffset |=
          (uint32_t(UnicodeNameToCodepointIndex[Offset++]) << 8);
      N.ChildrenOffset |= UnicodeNameToCodepointIndex[Offset++];
    }
  }
  N.Size = Offset - Origin;
  ঘূৰাই N;
}

স্থির তার্কিক startsWith(StringRef Name, StringRef Needle, তার্কিক Strict,
                       std::size_t &Consummed, ৱর্ণ &PreviousCharInName,
                       তার্কিক IsPrefix = মিছা) {

  Consummed = 0;
  যদি (Strict) {
    যদি (!Name.starts_with(Needle))
      ঘূৰাই মিছা;
    Consummed = Needle.size();
    ঘূৰাই সঁচা;
  }
  যদি (Needle.empty())
    ঘূৰাই সঁচা;

  auto NamePos = Name.begin();
  auto NeedlePos = Needle.begin();

  ৱর্ণ PreviousCharInNameOrigin = PreviousCharInName;
  ৱর্ণ PreviousCharInNeedle = *Needle.begin();
  auto IgnoreSpaces = [](auto It, auto End, ৱর্ণ &PreviousChar,
                         তার্কিক IsPrefix = মিছা) {
    যেতিয়ালৈ (It != End) {
      ধ্রুৱ auto Next = std::next(It);
      // Ignore spaces, underscore, medial hyphens
      // The generator ensures a needle never ends (or starts) by a medial
      // hyphen https://unicode.org/reports/tr44/#UAX44-LM2.
      তার্কিক Ignore =
          *It == ' ' || *It == '_' ||
          (*It == '-' && isAlnum(PreviousChar) &&
           ((Next != End && isAlnum(*Next)) || (Next == End && IsPrefix)));
      PreviousChar = *It;
      যদি (!Ignore)
        ভঙ্গ;
      ++It;
    }
    ঘূৰাই It;
  };

  যেতিয়ালৈ (সঁচা) {
    NamePos = IgnoreSpaces(NamePos, Name.end(), PreviousCharInName);
    NeedlePos =
        IgnoreSpaces(NeedlePos, Needle.end(), PreviousCharInNeedle, IsPrefix);
    যদি (NeedlePos == Needle.end())
      ভঙ্গ;
    যদি (NamePos == Name.end())
      ভঙ্গ;
    যদি (toUpper(*NeedlePos) != toUpper(*NamePos))
      ভঙ্গ;
    NeedlePos++;
    NamePos++;
  }
  Consummed = std::distance(Name.begin(), NamePos);
  যদি (NeedlePos != Needle.end()) {
    PreviousCharInName = PreviousCharInNameOrigin;
  }
  ঘূৰাই NeedlePos == Needle.end();
}

স্থির std::tuple<গ্ৰন্থি, তার্কিক, uint32_t>
compare_গ্ৰন্থি(uint32_t Offset, StringRef Name, তার্কিক Strict,
            ৱর্ণ PreviousCharInName, BufferType &Buffer,
            ধ্রুৱ গ্ৰন্থি *Parent = nullptr) {
  গ্ৰন্থি N = read_গ্ৰন্থি(Offset, Parent);
  std::size_t Consummed = 0;
  তার্কিক DoesStartWith = N.Is_মূল || startsWith(Name, N.Name, Strict, Consummed,
                                              PreviousCharInName);
  যদি (!DoesStartWith)
    ঘূৰাই std::make_tuple(N, মিছা, 0);

  যদি (Name.size() - Consummed == 0 && N.মান != 0xFFFFFFFF)
    ঘূৰাই std::make_tuple(N, সঁচা, N.মান);

  যদি (N.hasChildren()) {
    uint32_t ChildOffset = N.ChildrenOffset;
    বাবে (;;) {
      গ্ৰন্থি C;
      তার্কিক Matches;
      uint32_t মান;
      std::tie(C, Matches, মান) =
          compare_গ্ৰন্থি(ChildOffset, Name.substr(Consummed), Strict,
                      PreviousCharInName, Buffer, &N);
      যদি (Matches) {
        std::reverse_copy(C.Name.begin(), C.Name.end(),
                          std::back_inserter(Buffer));
        ঘূৰাই std::make_tuple(N, সঁচা, মান);
      }
      ChildOffset += C.Size;
      যদি (!C.HasSibling)
        ভঙ্গ;
    }
  }
  ঘূৰাই std::make_tuple(N, মিছা, 0);
}

স্থির std::tuple<গ্ৰন্থি, তার্কিক, uint32_t>
compare_গ্ৰন্থি(uint32_t Offset, StringRef Name, তার্কিক Strict, BufferType &Buffer) {
  ঘূৰাই compare_গ্ৰন্থি(Offset, Name, Strict, 0, Buffer);
}

// clang-format off
constexpr ধ্রুৱ ৱর্ণ *ধ্রুৱ HangulSyllables[][3] = {
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

স্থির std::size_t findSyllable(StringRef Name, তার্কিক Strict,
                                ৱর্ণ &PreviousInName, পূর্ণাঙ্ক &Pos, পূর্ণাঙ্ক Column) {
  প্রতিজ্ঞা(Column == 0 || Column == 1 || Column == 2);
  স্থির std::size_t গুনতি_Per_Column[] = {LCount, VCount, TCount};
  পূর্ণাঙ্ক Len = -1;
  পূর্ণাঙ্ক Prev = PreviousInName;
  বাবে (std::size_t I = 0; I < গুনতি_Per_Column[Column]; I++) {
    StringRef Syllable(HangulSyllables[I][Column]);
    যদি (পূর্ণাঙ্ক(Syllable.size()) <= Len)
      অনুৱর্ত;
    std::size_t Consummed = 0;
    ৱর্ণ PreviousInNameCopy = PreviousInName;
    তার্কিক DoesStartWith =
        startsWith(Name, Syllable, Strict, Consummed, PreviousInNameCopy);
    যদি (!DoesStartWith)
      অনুৱর্ত;
    Len = Consummed;
    Pos = I;
    Prev = PreviousInNameCopy;
  }
  যদি (Len == -1)
    ঘূৰাই 0;
  PreviousInName = Prev;
  ঘূৰাই size_t(Len);
}

স্থির std::optional<char32_t>
nameToHangulCodePoint(StringRef Name, তার্কিক Strict, BufferType &Buffer) {
  Buffer.clear();
  // Hangul Syllable Decomposition
  std::size_t Consummed = 0;
  ৱর্ণ NameStart = 0;
  তার্কিক DoesStartWith =
      startsWith(Name, "HANGUL SYLLABLE ", Strict, Consummed, NameStart);
  যদি (!DoesStartWith)
    ঘূৰাই std::nullopt;
  Name = Name.substr(Consummed);
  পূর্ণাঙ্ক L = -1, V = -1, T = -1;
  Name = Name.substr(findSyllable(Name, Strict, NameStart, L, 0));
  Name = Name.substr(findSyllable(Name, Strict, NameStart, V, 1));
  Name = Name.substr(findSyllable(Name, Strict, NameStart, T, 2));
  যদি (L != -1 && V != -1 && T != -1 && Name.empty()) {
    যদি (!Strict) {
      Buffer.append("HANGUL SYLLABLE ");
      যদি (L != -1)
        Buffer.append(HangulSyllables[L][0]);
      যদি (V != -1)
        Buffer.append(HangulSyllables[V][1]);
      যদি (T != -1)
        Buffer.append(HangulSyllables[T][2]);
    }
    ঘূৰাই SBase + (std::uint32_t(L) * VCount + std::uint32_t(V)) * TCount +
           std::uint32_t(T);
  }
  // Otherwise, it's an illegal syllable name.
  ঘূৰাই std::nullopt;
}

সংরচনা GeneratedNamesData {
  StringRef Prefix;
  uint32_t Start;
  uint32_t End;
};

// Unicode 18.0 Name Derivation Rule Prefix Strings (extracted/DerivedName.txt)
স্থির ধ্রুৱ GeneratedNamesData GeneratedNamesDataTable[] = {
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

স্থির std::optional<char32_t>
nameToGeneratedCodePoint(StringRef Name, তার্কিক Strict, BufferType &Buffer) {
  বাবে (auto &&Item : GeneratedNamesDataTable) {
    Buffer.clear();
    std::size_t Consummed = 0;
    ৱর্ণ NameStart = 0;
    তার্কিক DoesStartWith = startsWith(Name, Item.Prefix, Strict, Consummed,
                                    NameStart, /*IsPrefix=*/সঁচা);
    যদি (!DoesStartWith)
      অনুৱর্ত;
    auto সংখ্যা = Name.substr(Consummed);
    unsigned long long V = 0;
    // Be consistent about mandating upper casing.
    যদি (Strict &&
        llvm::any_of(সংখ্যা, [](ৱর্ণ C) { ঘূৰাই C >= 'a' && C <= 'f'; }))
      ঘূৰাই {};
    যদি (getAsUnsignedInteger(সংখ্যা, 16, V) || V < Item.Start || V > Item.End)
      অনুৱর্ত;
    যদি (!Strict) {
      Buffer.append(Item.Prefix);
      Buffer.append(utohexstr(V, সঁচা));
    }
    ঘূৰাই V;
  }
  ঘূৰাই std::nullopt;
}

স্থির std::optional<char32_t> nameToCodepoint(StringRef Name, তার্কিক Strict,
                                               BufferType &Buffer) {
  যদি (Name.empty())
    ঘূৰাই std::nullopt;

  std::optional<char32_t> Res = nameToHangulCodePoint(Name, Strict, Buffer);
  যদি (!Res)
    Res = nameToGeneratedCodePoint(Name, Strict, Buffer);
  যদি (Res)
    ঘূৰাই *Res;

  Buffer.clear();
  গ্ৰন্থি গ্ৰন্থি;
  তার্কিক Matches;
  uint32_t মান;
  std::tie(গ্ৰন্থি, Matches, মান) = compare_গ্ৰন্থি(0, Name, Strict, Buffer);
  যদি (Matches) {
    std::reverse(Buffer.begin(), Buffer.end());
    // UAX44-LM2. Ignore case, whitespace, underscore ('_'), and all medial
    // hyphens except the hyphen in U+1180 HANGUL JUNGSEONG O-E.
    যদি (!Strict && মান == 0x116c && Name.contains_insensitive("O-E")) {
      Buffer = "HANGUL JUNGSEONG O-E";
      মান = 0x1180;
    }
    ঘূৰাই মান;
  }
  ঘূৰাই std::nullopt;
}

std::optional<char32_t> nameToCodepointStrict(StringRef Name) {

  BufferType Buffer;
  auto Opt = nameToCodepoint(Name, সঁচা, Buffer);
  ঘূৰাই Opt;
}

std::optional<LooseMatchingResult>
nameToCodepointLooseMatching(StringRef Name) {
  BufferType Buffer;
  auto Opt = nameToCodepoint(Name, মিছা, Buffer);
  যদি (!Opt)
    ঘূৰাই std::nullopt;
  ঘূৰাই LooseMatchingResult{*Opt, Buffer};
}

// Find the unicode character whose editing distance to Pattern
// is shortest, using the Wagner–Fischer algorithm.
llvm::SmallVector<MatchForCodepointName>
nearestMatchesForCodepointName(StringRef Pattern, std::size_t Max_Matches_গুনতি) {
  // We maintain a fixed size vector of matches,
  // sorted by distance
  // The worst match (with the biggest distance) are discarded when new elements
  // are added.
  std::size_t LargestEditDistance = 0;
  llvm::SmallVector<MatchForCodepointName> Matches;
  Matches.reserve(Max_Matches_গুনতি + 1);

  auto ঢোকাও_2 = [&](ধ্রুৱ গ্ৰন্থি &গ্ৰন্থি, uint32_t Distance,
                    char32_t মান) -> তার্কিক {
    যদি (Distance > LargestEditDistance) {
      যদি (Matches.size() == Max_Matches_গুনতি)
        ঘূৰাই মিছা;
      LargestEditDistance = Distance;
    }
    // To avoid allocations, the creation of the name is delayed
    // as much as possible.
    std::string Name;
    auto GetName = [&] {
      যদি (Name.empty())
        Name = গ্ৰন্থি.fullName();
      ঘূৰাই Name;
    };

    auto It = llvm::lower_bound(
        Matches, Distance,
        [&](ধ্রুৱ MatchForCodepointName &a, std::size_t Distance) {
          যদি (Distance == a.Distance)
            ঘূৰাই a.Name < GetName();
          ঘূৰাই a.Distance < Distance;
        });
    যদি (It == Matches.end() && Matches.size() == Max_Matches_গুনতি)
      ঘূৰাই মিছা;

    MatchForCodepointName M{GetName(), Distance, মান};
    Matches.ঢোকাও(It, std::move(M));
    যদি (Matches.size() > Max_Matches_গুনতি)
      Matches.বেৰ_কৰো_back();
    ঘূৰাই সঁচা;
  };

  // We ignore case, space, hyphens, etc,
  // in both the search pattern and the prospective names.
  auto Normalize = [](StringRef Name) {
    std::string Out;
    Out.reserve(Name.size());
    বাবে (ৱর্ণ C : Name) {
      যদি (isAlnum(C))
        Out.ঠেলো_back(toUpper(C));
    }
    ঘূৰাই Out;
  };
  std::string NormalizedName = Normalize(Pattern);

  // Allocate a matrix big enough for longest names.
  ধ্রুৱ std::size_t Columns =
      std::min(NormalizedName.size(), UnicodeNameToCodepointLargestNameSize) +
      1;

  [[maybe_unused]] স্থির std::size_t Rows =
      UnicodeNameToCodepointLargestNameSize + 1;

  std::vector<ৱর্ণ> Distances(
      Columns * (UnicodeNameToCodepointLargestNameSize + 1), 0);

  auto Get = [&Distances, Columns](size_t Column, std::size_t Row) -> ৱর্ণ & {
    প্রতিজ্ঞা(Column < Columns);
    প্রতিজ্ঞা(Row < Rows);
    ঘূৰাই Distances[Row * Columns + Column];
  };

  বাবে (std::size_t I = 0; I < Columns; I++)
    Get(I, 0) = I;

  // Visit the childrens,
  // Filling (and overriding) the matrix for the name fragment of each node
  // iteratively. CompleteName is used to collect the actual name of potential
  // match, respecting case and spacing.
  auto Visit_গ্ৰন্থি = [&](ধ্রুৱ গ্ৰন্থি &N, std::size_t Row,
                       auto &Visit_গ্ৰন্থি) -> শূন্য {
    std::size_t J = 0;
    বাবে (; J < N.Name.size(); J++) {
      যদি (!isAlnum(N.Name[J]))
        অনুৱর্ত;

      Get(0, Row) = Row;

      বাবে (std::size_t I = 1; I < Columns; I++) {
        ধ্রুৱ পূর্ণাঙ্ক Delete = Get(I - 1, Row) + 1;
        ধ্রুৱ পূর্ণাঙ্ক ঢোকাও_2 = Get(I, Row - 1) + 1;

        ধ্রুৱ পূর্ণাঙ্ক Replace =
            Get(I - 1, Row - 1) + (NormalizedName[I - 1] != N.Name[J] ? 1 : 0);

        Get(I, Row) = std::min({ঢোকাও_2, Delete, Replace});
      }

      Row++;
    }

    unsigned Cost = Get(Columns - 1, Row - 1);
    যদি (N.মান != 0xFFFFFFFF) {
      ঢোকাও_2(N, Cost, N.মান);
    }

    যদি (N.hasChildren()) {
      auto ChildOffset = N.ChildrenOffset;
      বাবে (;;) {
        গ্ৰন্থি C = read_গ্ৰন্থি(ChildOffset, &N);
        ChildOffset += C.Size;
        যদি (!C.isValid())
          ভঙ্গ;
        Visit_গ্ৰন্থি(C, Row, Visit_গ্ৰন্থি);
        যদি (!C.HasSibling)
          ভঙ্গ;
      }
    }
  };

  গ্ৰন্থি মূল_2 = create_মূল();
  Visit_গ্ৰন্থি(মূল_2, 1, Visit_গ্ৰন্থি);
  ঘূৰাই Matches;
}

} // namespace unicode

} // namespace sys
} // namespace llvm
