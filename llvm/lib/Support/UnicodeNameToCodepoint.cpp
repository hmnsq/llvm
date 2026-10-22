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

#ଅନ୍ତର୍ଭାଵ "llvm/ADT/STLExtras.h"
#ଅନ୍ତର୍ଭାଵ "llvm/ADT/StringExtras.h"
#ଅନ୍ତର୍ଭାଵ "llvm/ADT/StringRef.h"
#ଅନ୍ତର୍ଭାଵ "llvm/Support/Unicode.h"

ନାମାକାଶ llvm {
ନାମାକାଶ sys {
ନାମାକାଶ unicode {

ବାହ୍ୟ ଅଚଳ ଅକ୍ଷର *ଅଚଳ UnicodeNameToCodepointDict;
ବାହ୍ୟ ଅଚଳ uint8_t *ଅଚଳ UnicodeNameToCodepointIndex;
ବାହ୍ୟ ଅଚଳ std::size_t UnicodeNameToCodepointIndexSize;
ବାହ୍ୟ ଅଚଳ std::size_t UnicodeNameToCodepointLargestNameSize;

using BufferType = SmallString<64>;

ସଂରଚନା ଗଣ୍ଠି {
  ତାର୍କିକ Is_ମୂଳ = ମିଥ୍ୟା;
  char32_t ମୂଲ୍ୟ = 0xFFFFFFFF;
  uint32_t ChildrenOffset = 0;
  ତାର୍କିକ HasSibling = ମିଥ୍ୟା;
  uint32_t Size = 0;
  StringRef Name;
  ଅଚଳ ଗଣ୍ଠି *Parent = nullptr;

  constexpr ତାର୍କିକ isValid() ଅଚଳ {
    ଫେରନ୍ତୁ !Name.empty() || ମୂଲ୍ୟ == 0xFFFFFFFF;
  }
  constexpr ତାର୍କିକ hasChildren() ଅଚଳ { ଫେରନ୍ତୁ ChildrenOffset != 0 || Is_ମୂଳ; }

  std::string fullName() ଅଚଳ {
    std::string S;
    // Reserve enough space for most unicode code points.
    // The chosen value represent the 99th percentile of name size as of
    // Unicode 15.0.
    S.reserve(46);
    ଅଚଳ ଗଣ୍ଠି *N = ଏହି;
    ଯେତେବେଳେ (N) {
      std::reverse_copy(N->Name.begin(), N->Name.end(), std::back_inserter(S));
      N = N->Parent;
    }
    std::reverse(S.begin(), S.end());
    ଫେରନ୍ତୁ S;
  }
};

ସ୍ଥିର ଗଣ୍ଠି create_ମୂଳ() {
  ଗଣ୍ଠି N;
  N.Is_ମୂଳ = ସତ୍ୟ;
  N.ChildrenOffset = 1;
  N.Size = 1;
  ଫେରନ୍ତୁ N;
}

ସ୍ଥିର ଗଣ୍ଠି read_ଗଣ୍ଠି(uint32_t Offset, ଅଚଳ ଗଣ୍ଠି *Parent = nullptr) {
  ଯଦି (Offset == 0)
    ଫେରନ୍ତୁ create_ମୂଳ();

  uint32_t Origin = Offset;
  ଗଣ୍ଠି N;
  N.Parent = Parent;
  uint8_t NameInfo = UnicodeNameToCodepointIndex[Offset++];
  ଯଦି (Offset + 6 >= UnicodeNameToCodepointIndexSize)
    ଫେରନ୍ତୁ N;

  ତାର୍କିକ LongName = NameInfo & 0x40;
  ତାର୍କିକ Has_ମୂଲ୍ୟ = NameInfo & 0x80;
  std::size_t Size = NameInfo & ~0xC0;
  ଯଦି (LongName) {
    uint32_t NameOffset = (UnicodeNameToCodepointIndex[Offset++] << 8);
    NameOffset |= UnicodeNameToCodepointIndex[Offset++];
    N.Name = StringRef(UnicodeNameToCodepointDict + NameOffset, Size);
  } ଅନ୍ୟଥା {
    N.Name = StringRef(UnicodeNameToCodepointDict + Size, 1);
  }
  ଯଦି (Has_ମୂଲ୍ୟ) {
    uint8_t H = UnicodeNameToCodepointIndex[Offset++];
    uint8_t M = UnicodeNameToCodepointIndex[Offset++];
    uint8_t L = UnicodeNameToCodepointIndex[Offset++];
    N.ମୂଲ୍ୟ = ((H << 16) | (M << 8) | L) >> 3;

    ତାର୍କିକ HasChildren = L & 0x02;
    N.HasSibling = L & 0x01;

    ଯଦି (HasChildren) {
      N.ChildrenOffset = UnicodeNameToCodepointIndex[Offset++] << 16;
      N.ChildrenOffset |= UnicodeNameToCodepointIndex[Offset++] << 8;
      N.ChildrenOffset |= UnicodeNameToCodepointIndex[Offset++];
    }
  } ଅନ୍ୟଥା {
    uint8_t H = UnicodeNameToCodepointIndex[Offset++];
    N.HasSibling = H & 0x80;
    ତାର୍କିକ HasChildren = H & 0x40;
    H &= uint8_t(~0xC0);
    ଯଦି (HasChildren) {
      N.ChildrenOffset = (H << 16);
      N.ChildrenOffset |=
          (uint32_t(UnicodeNameToCodepointIndex[Offset++]) << 8);
      N.ChildrenOffset |= UnicodeNameToCodepointIndex[Offset++];
    }
  }
  N.Size = Offset - Origin;
  ଫେରନ୍ତୁ N;
}

ସ୍ଥିର ତାର୍କିକ startsWith(StringRef Name, StringRef Needle, ତାର୍କିକ Strict,
                       std::size_t &Consummed, ଅକ୍ଷର &PreviousCharInName,
                       ତାର୍କିକ IsPrefix = ମିଥ୍ୟା) {

  Consummed = 0;
  ଯଦି (Strict) {
    ଯଦି (!Name.starts_with(Needle))
      ଫେରନ୍ତୁ ମିଥ୍ୟା;
    Consummed = Needle.size();
    ଫେରନ୍ତୁ ସତ୍ୟ;
  }
  ଯଦି (Needle.empty())
    ଫେରନ୍ତୁ ସତ୍ୟ;

  auto NamePos = Name.begin();
  auto NeedlePos = Needle.begin();

  ଅକ୍ଷର PreviousCharInNameOrigin = PreviousCharInName;
  ଅକ୍ଷର PreviousCharInNeedle = *Needle.begin();
  auto IgnoreSpaces = [](auto It, auto End, ଅକ୍ଷର &PreviousChar,
                         ତାର୍କିକ IsPrefix = ମିଥ୍ୟା) {
    ଯେତେବେଳେ (It != End) {
      ଅଚଳ auto Next = std::next(It);
      // Ignore spaces, underscore, medial hyphens
      // The generator ensures a needle never ends (or starts) by a medial
      // hyphen https://unicode.org/reports/tr44/#UAX44-LM2.
      ତାର୍କିକ Ignore =
          *It == ' ' || *It == '_' ||
          (*It == '-' && isAlnum(PreviousChar) &&
           ((Next != End && isAlnum(*Next)) || (Next == End && IsPrefix)));
      PreviousChar = *It;
      ଯଦି (!Ignore)
        ଭାଙ୍ଗ;
      ++It;
    }
    ଫେରନ୍ତୁ It;
  };

  ଯେତେବେଳେ (ସତ୍ୟ) {
    NamePos = IgnoreSpaces(NamePos, Name.end(), PreviousCharInName);
    NeedlePos =
        IgnoreSpaces(NeedlePos, Needle.end(), PreviousCharInNeedle, IsPrefix);
    ଯଦି (NeedlePos == Needle.end())
      ଭାଙ୍ଗ;
    ଯଦି (NamePos == Name.end())
      ଭାଙ୍ଗ;
    ଯଦି (toUpper(*NeedlePos) != toUpper(*NamePos))
      ଭାଙ୍ଗ;
    NeedlePos++;
    NamePos++;
  }
  Consummed = std::distance(Name.begin(), NamePos);
  ଯଦି (NeedlePos != Needle.end()) {
    PreviousCharInName = PreviousCharInNameOrigin;
  }
  ଫେରନ୍ତୁ NeedlePos == Needle.end();
}

ସ୍ଥିର std::tuple<ଗଣ୍ଠି, ତାର୍କିକ, uint32_t>
compare_ଗଣ୍ଠି(uint32_t Offset, StringRef Name, ତାର୍କିକ Strict,
            ଅକ୍ଷର PreviousCharInName, BufferType &Buffer,
            ଅଚଳ ଗଣ୍ଠି *Parent = nullptr) {
  ଗଣ୍ଠି N = read_ଗଣ୍ଠି(Offset, Parent);
  std::size_t Consummed = 0;
  ତାର୍କିକ DoesStartWith = N.Is_ମୂଳ || startsWith(Name, N.Name, Strict, Consummed,
                                              PreviousCharInName);
  ଯଦି (!DoesStartWith)
    ଫେରନ୍ତୁ std::make_tuple(N, ମିଥ୍ୟା, 0);

  ଯଦି (Name.size() - Consummed == 0 && N.ମୂଲ୍ୟ != 0xFFFFFFFF)
    ଫେରନ୍ତୁ std::make_tuple(N, ସତ୍ୟ, N.ମୂଲ୍ୟ);

  ଯଦି (N.hasChildren()) {
    uint32_t ChildOffset = N.ChildrenOffset;
    ପାଇଁ (;;) {
      ଗଣ୍ଠି C;
      ତାର୍କିକ Matches;
      uint32_t ମୂଲ୍ୟ;
      std::tie(C, Matches, ମୂଲ୍ୟ) =
          compare_ଗଣ୍ଠି(ChildOffset, Name.substr(Consummed), Strict,
                      PreviousCharInName, Buffer, &N);
      ଯଦି (Matches) {
        std::reverse_copy(C.Name.begin(), C.Name.end(),
                          std::back_inserter(Buffer));
        ଫେରନ୍ତୁ std::make_tuple(N, ସତ୍ୟ, ମୂଲ୍ୟ);
      }
      ChildOffset += C.Size;
      ଯଦି (!C.HasSibling)
        ଭାଙ୍ଗ;
    }
  }
  ଫେରନ୍ତୁ std::make_tuple(N, ମିଥ୍ୟା, 0);
}

ସ୍ଥିର std::tuple<ଗଣ୍ଠି, ତାର୍କିକ, uint32_t>
compare_ଗଣ୍ଠି(uint32_t Offset, StringRef Name, ତାର୍କିକ Strict, BufferType &Buffer) {
  ଫେରନ୍ତୁ compare_ଗଣ୍ଠି(Offset, Name, Strict, 0, Buffer);
}

// clang-format off
constexpr ଅଚଳ ଅକ୍ଷର *ଅଚଳ HangulSyllables[][3] = {
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

ସ୍ଥିର std::size_t findSyllable(StringRef Name, ତାର୍କିକ Strict,
                                ଅକ୍ଷର &PreviousInName, ପୂର୍ଣ୍ଣସଂଖ୍ୟା &Pos, ପୂର୍ଣ୍ଣସଂଖ୍ୟା Column) {
  ପ୍ରତିଜ୍ଞା(Column == 0 || Column == 1 || Column == 2);
  ସ୍ଥିର std::size_t ଗଣନା_Per_Column[] = {LCount, VCount, TCount};
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା Len = -1;
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା Prev = PreviousInName;
  ପାଇଁ (std::size_t I = 0; I < ଗଣନା_Per_Column[Column]; I++) {
    StringRef Syllable(HangulSyllables[I][Column]);
    ଯଦି (ପୂର୍ଣ୍ଣସଂଖ୍ୟା(Syllable.size()) <= Len)
      ଜାରି;
    std::size_t Consummed = 0;
    ଅକ୍ଷର PreviousInNameCopy = PreviousInName;
    ତାର୍କିକ DoesStartWith =
        startsWith(Name, Syllable, Strict, Consummed, PreviousInNameCopy);
    ଯଦି (!DoesStartWith)
      ଜାରି;
    Len = Consummed;
    Pos = I;
    Prev = PreviousInNameCopy;
  }
  ଯଦି (Len == -1)
    ଫେରନ୍ତୁ 0;
  PreviousInName = Prev;
  ଫେରନ୍ତୁ size_t(Len);
}

ସ୍ଥିର std::optional<char32_t>
nameToHangulCodePoint(StringRef Name, ତାର୍କିକ Strict, BufferType &Buffer) {
  Buffer.clear();
  // Hangul Syllable Decomposition
  std::size_t Consummed = 0;
  ଅକ୍ଷର NameStart = 0;
  ତାର୍କିକ DoesStartWith =
      startsWith(Name, "HANGUL SYLLABLE ", Strict, Consummed, NameStart);
  ଯଦି (!DoesStartWith)
    ଫେରନ୍ତୁ std::nullopt;
  Name = Name.substr(Consummed);
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା L = -1, V = -1, T = -1;
  Name = Name.substr(findSyllable(Name, Strict, NameStart, L, 0));
  Name = Name.substr(findSyllable(Name, Strict, NameStart, V, 1));
  Name = Name.substr(findSyllable(Name, Strict, NameStart, T, 2));
  ଯଦି (L != -1 && V != -1 && T != -1 && Name.empty()) {
    ଯଦି (!Strict) {
      Buffer.append("HANGUL SYLLABLE ");
      ଯଦି (L != -1)
        Buffer.append(HangulSyllables[L][0]);
      ଯଦି (V != -1)
        Buffer.append(HangulSyllables[V][1]);
      ଯଦି (T != -1)
        Buffer.append(HangulSyllables[T][2]);
    }
    ଫେରନ୍ତୁ SBase + (std::uint32_t(L) * VCount + std::uint32_t(V)) * TCount +
           std::uint32_t(T);
  }
  // Otherwise, it's an illegal syllable name.
  ଫେରନ୍ତୁ std::nullopt;
}

ସଂରଚନା GeneratedNamesData {
  StringRef Prefix;
  uint32_t Start;
  uint32_t End;
};

// Unicode 18.0 Name Derivation Rule Prefix Strings (extracted/DerivedName.txt)
ସ୍ଥିର ଅଚଳ GeneratedNamesData GeneratedNamesDataTable[] = {
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

ସ୍ଥିର std::optional<char32_t>
nameToGeneratedCodePoint(StringRef Name, ତାର୍କିକ Strict, BufferType &Buffer) {
  ପାଇଁ (auto &&Item : GeneratedNamesDataTable) {
    Buffer.clear();
    std::size_t Consummed = 0;
    ଅକ୍ଷର NameStart = 0;
    ତାର୍କିକ DoesStartWith = startsWith(Name, Item.Prefix, Strict, Consummed,
                                    NameStart, /*IsPrefix=*/ସତ୍ୟ);
    ଯଦି (!DoesStartWith)
      ଜାରି;
    auto ସଂଖ୍ୟା = Name.substr(Consummed);
    unsigned long long V = 0;
    // Be consistent about mandating upper casing.
    ଯଦି (Strict &&
        llvm::any_of(ସଂଖ୍ୟା, [](ଅକ୍ଷର C) { ଫେରନ୍ତୁ C >= 'a' && C <= 'f'; }))
      ଫେରନ୍ତୁ {};
    ଯଦି (getAsUnsignedInteger(ସଂଖ୍ୟା, 16, V) || V < Item.Start || V > Item.End)
      ଜାରି;
    ଯଦି (!Strict) {
      Buffer.append(Item.Prefix);
      Buffer.append(utohexstr(V, ସତ୍ୟ));
    }
    ଫେରନ୍ତୁ V;
  }
  ଫେରନ୍ତୁ std::nullopt;
}

ସ୍ଥିର std::optional<char32_t> nameToCodepoint(StringRef Name, ତାର୍କିକ Strict,
                                               BufferType &Buffer) {
  ଯଦି (Name.empty())
    ଫେରନ୍ତୁ std::nullopt;

  std::optional<char32_t> Res = nameToHangulCodePoint(Name, Strict, Buffer);
  ଯଦି (!Res)
    Res = nameToGeneratedCodePoint(Name, Strict, Buffer);
  ଯଦି (Res)
    ଫେରନ୍ତୁ *Res;

  Buffer.clear();
  ଗଣ୍ଠି ଗଣ୍ଠି;
  ତାର୍କିକ Matches;
  uint32_t ମୂଲ୍ୟ;
  std::tie(ଗଣ୍ଠି, Matches, ମୂଲ୍ୟ) = compare_ଗଣ୍ଠି(0, Name, Strict, Buffer);
  ଯଦି (Matches) {
    std::reverse(Buffer.begin(), Buffer.end());
    // UAX44-LM2. Ignore case, whitespace, underscore ('_'), and all medial
    // hyphens except the hyphen in U+1180 HANGUL JUNGSEONG O-E.
    ଯଦି (!Strict && ମୂଲ୍ୟ == 0x116c && Name.contains_insensitive("O-E")) {
      Buffer = "HANGUL JUNGSEONG O-E";
      ମୂଲ୍ୟ = 0x1180;
    }
    ଫେରନ୍ତୁ ମୂଲ୍ୟ;
  }
  ଫେରନ୍ତୁ std::nullopt;
}

std::optional<char32_t> nameToCodepointStrict(StringRef Name) {

  BufferType Buffer;
  auto Opt = nameToCodepoint(Name, ସତ୍ୟ, Buffer);
  ଫେରନ୍ତୁ Opt;
}

std::optional<LooseMatchingResult>
nameToCodepointLooseMatching(StringRef Name) {
  BufferType Buffer;
  auto Opt = nameToCodepoint(Name, ମିଥ୍ୟା, Buffer);
  ଯଦି (!Opt)
    ଫେରନ୍ତୁ std::nullopt;
  ଫେରନ୍ତୁ LooseMatchingResult{*Opt, Buffer};
}

// Find the unicode character whose editing distance to Pattern
// is shortest, using the Wagner–Fischer algorithm.
llvm::SmallVector<MatchForCodepointName>
nearestMatchesForCodepointName(StringRef Pattern, std::size_t Max_Matches_ଗଣନା) {
  // We maintain a fixed size vector of matches,
  // sorted by distance
  // The worst match (with the biggest distance) are discarded when new elements
  // are added.
  std::size_t LargestEditDistance = 0;
  llvm::SmallVector<MatchForCodepointName> Matches;
  Matches.reserve(Max_Matches_ଗଣନା + 1);

  auto ଭର୍ତ୍ତି_2 = [&](ଅଚଳ ଗଣ୍ଠି &ଗଣ୍ଠି, uint32_t Distance,
                    char32_t ମୂଲ୍ୟ) -> ତାର୍କିକ {
    ଯଦି (Distance > LargestEditDistance) {
      ଯଦି (Matches.size() == Max_Matches_ଗଣନା)
        ଫେରନ୍ତୁ ମିଥ୍ୟା;
      LargestEditDistance = Distance;
    }
    // To avoid allocations, the creation of the name is delayed
    // as much as possible.
    std::string Name;
    auto GetName = [&] {
      ଯଦି (Name.empty())
        Name = ଗଣ୍ଠି.fullName();
      ଫେରନ୍ତୁ Name;
    };

    auto It = llvm::lower_bound(
        Matches, Distance,
        [&](ଅଚଳ MatchForCodepointName &a, std::size_t Distance) {
          ଯଦି (Distance == a.Distance)
            ଫେରନ୍ତୁ a.Name < GetName();
          ଫେରନ୍ତୁ a.Distance < Distance;
        });
    ଯଦି (It == Matches.end() && Matches.size() == Max_Matches_ଗଣନା)
      ଫେରନ୍ତୁ ମିଥ୍ୟା;

    MatchForCodepointName M{GetName(), Distance, ମୂଲ୍ୟ};
    Matches.ଭର୍ତ୍ତି(It, std::move(M));
    ଯଦି (Matches.size() > Max_Matches_ଗଣନା)
      Matches.କାଢ଼_back();
    ଫେରନ୍ତୁ ସତ୍ୟ;
  };

  // We ignore case, space, hyphens, etc,
  // in both the search pattern and the prospective names.
  auto Normalize = [](StringRef Name) {
    std::string Out;
    Out.reserve(Name.size());
    ପାଇଁ (ଅକ୍ଷର C : Name) {
      ଯଦି (isAlnum(C))
        Out.ଠେଲ_back(toUpper(C));
    }
    ଫେରନ୍ତୁ Out;
  };
  std::string NormalizedName = Normalize(Pattern);

  // Allocate a matrix big enough for longest names.
  ଅଚଳ std::size_t Columns =
      std::min(NormalizedName.size(), UnicodeNameToCodepointLargestNameSize) +
      1;

  [[maybe_unused]] ସ୍ଥିର std::size_t Rows =
      UnicodeNameToCodepointLargestNameSize + 1;

  std::vector<ଅକ୍ଷର> Distances(
      Columns * (UnicodeNameToCodepointLargestNameSize + 1), 0);

  auto Get = [&Distances, Columns](size_t Column, std::size_t Row) -> ଅକ୍ଷର & {
    ପ୍ରତିଜ୍ଞା(Column < Columns);
    ପ୍ରତିଜ୍ଞା(Row < Rows);
    ଫେରନ୍ତୁ Distances[Row * Columns + Column];
  };

  ପାଇଁ (std::size_t I = 0; I < Columns; I++)
    Get(I, 0) = I;

  // Visit the childrens,
  // Filling (and overriding) the matrix for the name fragment of each node
  // iteratively. CompleteName is used to collect the actual name of potential
  // match, respecting case and spacing.
  auto Visit_ଗଣ୍ଠି = [&](ଅଚଳ ଗଣ୍ଠି &N, std::size_t Row,
                       auto &Visit_ଗଣ୍ଠି) -> ଶୂନ୍ୟ {
    std::size_t J = 0;
    ପାଇଁ (; J < N.Name.size(); J++) {
      ଯଦି (!isAlnum(N.Name[J]))
        ଜାରି;

      Get(0, Row) = Row;

      ପାଇଁ (std::size_t I = 1; I < Columns; I++) {
        ଅଚଳ ପୂର୍ଣ୍ଣସଂଖ୍ୟା Delete = Get(I - 1, Row) + 1;
        ଅଚଳ ପୂର୍ଣ୍ଣସଂଖ୍ୟା ଭର୍ତ୍ତି_2 = Get(I, Row - 1) + 1;

        ଅଚଳ ପୂର୍ଣ୍ଣସଂଖ୍ୟା Replace =
            Get(I - 1, Row - 1) + (NormalizedName[I - 1] != N.Name[J] ? 1 : 0);

        Get(I, Row) = std::min({ଭର୍ତ୍ତି_2, Delete, Replace});
      }

      Row++;
    }

    unsigned Cost = Get(Columns - 1, Row - 1);
    ଯଦି (N.ମୂଲ୍ୟ != 0xFFFFFFFF) {
      ଭର୍ତ୍ତି_2(N, Cost, N.ମୂଲ୍ୟ);
    }

    ଯଦି (N.hasChildren()) {
      auto ChildOffset = N.ChildrenOffset;
      ପାଇଁ (;;) {
        ଗଣ୍ଠି C = read_ଗଣ୍ଠି(ChildOffset, &N);
        ChildOffset += C.Size;
        ଯଦି (!C.isValid())
          ଭାଙ୍ଗ;
        Visit_ଗଣ୍ଠି(C, Row, Visit_ଗଣ୍ଠି);
        ଯଦି (!C.HasSibling)
          ଭାଙ୍ଗ;
      }
    }
  };

  ଗଣ୍ଠି ମୂଳ_2 = create_ମୂଳ();
  Visit_ଗଣ୍ଠି(ମୂଳ_2, 1, Visit_ଗଣ୍ଠି);
  ଫେରନ୍ତୁ Matches;
}

} // namespace unicode

} // namespace sys
} // namespace llvm
