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

#ಅನ್ತರ್ಭಾವ "llvm/ADT/STLExtras.h"
#ಅನ್ತರ್ಭಾವ "llvm/ADT/StringExtras.h"
#ಅನ್ತರ್ಭಾವ "llvm/ADT/StringRef.h"
#ಅನ್ತರ್ಭಾವ "llvm/Support/Unicode.h"

ನಾಮಾಕಾಶ llvm {
ನಾಮಾಕಾಶ sys {
ನಾಮಾಕಾಶ unicode {

ಬಾಹ್ಯ ಧ್ರುವ ವರ್ಣ *ಧ್ರುವ UnicodeNameToCodepointDict;
ಬಾಹ್ಯ ಧ್ರುವ uint8_t *ಧ್ರುವ UnicodeNameToCodepointIndex;
ಬಾಹ್ಯ ಧ್ರುವ std::size_t UnicodeNameToCodepointIndexSize;
ಬಾಹ್ಯ ಧ್ರುವ std::size_t UnicodeNameToCodepointLargestNameSize;

using BufferType = SmallString<64>;

ಸಂರಚನಾ ಗಂಟು {
  ತಾರ್ಕಿಕ Is_ಬೇರು = ಅಸತ್ಯ;
  char32_t ಮೌಲ್ಯ = 0xFFFFFFFF;
  uint32_t ChildrenOffset = 0;
  ತಾರ್ಕಿಕ HasSibling = ಅಸತ್ಯ;
  uint32_t Size = 0;
  StringRef Name;
  ಧ್ರುವ ಗಂಟು *Parent = nullptr;

  constexpr ತಾರ್ಕಿಕ isValid() ಧ್ರುವ {
    ಪ್ರತ್ಯಾಗಮ !Name.empty() || ಮೌಲ್ಯ == 0xFFFFFFFF;
  }
  constexpr ತಾರ್ಕಿಕ hasChildren() ಧ್ರುವ { ಪ್ರತ್ಯಾಗಮ ChildrenOffset != 0 || Is_ಬೇರು; }

  std::string fullName() ಧ್ರುವ {
    std::string S;
    // Reserve enough space for most unicode code points.
    // The chosen value represent the 99th percentile of name size as of
    // Unicode 15.0.
    S.reserve(46);
    ಧ್ರುವ ಗಂಟು *N = ಏತತ್;
    ಯಾವತ್ (N) {
      std::reverse_copy(N->Name.begin(), N->Name.end(), std::back_inserter(S));
      N = N->Parent;
    }
    std::reverse(S.begin(), S.end());
    ಪ್ರತ್ಯಾಗಮ S;
  }
};

ಸ್ಥಿರ ಗಂಟು create_ಬೇರು() {
  ಗಂಟು N;
  N.Is_ಬೇರು = ಸತ್ಯ;
  N.ChildrenOffset = 1;
  N.Size = 1;
  ಪ್ರತ್ಯಾಗಮ N;
}

ಸ್ಥಿರ ಗಂಟು read_ಗಂಟು(uint32_t Offset, ಧ್ರುವ ಗಂಟು *Parent = nullptr) {
  ಯದಿ (Offset == 0)
    ಪ್ರತ್ಯಾಗಮ create_ಬೇರು();

  uint32_t Origin = Offset;
  ಗಂಟು N;
  N.Parent = Parent;
  uint8_t NameInfo = UnicodeNameToCodepointIndex[Offset++];
  ಯದಿ (Offset + 6 >= UnicodeNameToCodepointIndexSize)
    ಪ್ರತ್ಯಾಗಮ N;

  ತಾರ್ಕಿಕ LongName = NameInfo & 0x40;
  ತಾರ್ಕಿಕ Has_ಮೌಲ್ಯ = NameInfo & 0x80;
  std::size_t Size = NameInfo & ~0xC0;
  ಯದಿ (LongName) {
    uint32_t NameOffset = (UnicodeNameToCodepointIndex[Offset++] << 8);
    NameOffset |= UnicodeNameToCodepointIndex[Offset++];
    N.Name = StringRef(UnicodeNameToCodepointDict + NameOffset, Size);
  } ಅನ್ಯಥಾ {
    N.Name = StringRef(UnicodeNameToCodepointDict + Size, 1);
  }
  ಯದಿ (Has_ಮೌಲ್ಯ) {
    uint8_t H = UnicodeNameToCodepointIndex[Offset++];
    uint8_t M = UnicodeNameToCodepointIndex[Offset++];
    uint8_t L = UnicodeNameToCodepointIndex[Offset++];
    N.ಮೌಲ್ಯ = ((H << 16) | (M << 8) | L) >> 3;

    ತಾರ್ಕಿಕ HasChildren = L & 0x02;
    N.HasSibling = L & 0x01;

    ಯದಿ (HasChildren) {
      N.ChildrenOffset = UnicodeNameToCodepointIndex[Offset++] << 16;
      N.ChildrenOffset |= UnicodeNameToCodepointIndex[Offset++] << 8;
      N.ChildrenOffset |= UnicodeNameToCodepointIndex[Offset++];
    }
  } ಅನ್ಯಥಾ {
    uint8_t H = UnicodeNameToCodepointIndex[Offset++];
    N.HasSibling = H & 0x80;
    ತಾರ್ಕಿಕ HasChildren = H & 0x40;
    H &= uint8_t(~0xC0);
    ಯದಿ (HasChildren) {
      N.ChildrenOffset = (H << 16);
      N.ChildrenOffset |=
          (uint32_t(UnicodeNameToCodepointIndex[Offset++]) << 8);
      N.ChildrenOffset |= UnicodeNameToCodepointIndex[Offset++];
    }
  }
  N.Size = Offset - Origin;
  ಪ್ರತ್ಯಾಗಮ N;
}

ಸ್ಥಿರ ತಾರ್ಕಿಕ startsWith(StringRef Name, StringRef Needle, ತಾರ್ಕಿಕ Strict,
                       std::size_t &Consummed, ವರ್ಣ &PreviousCharInName,
                       ತಾರ್ಕಿಕ IsPrefix = ಅಸತ್ಯ) {

  Consummed = 0;
  ಯದಿ (Strict) {
    ಯದಿ (!Name.starts_with(Needle))
      ಪ್ರತ್ಯಾಗಮ ಅಸತ್ಯ;
    Consummed = Needle.size();
    ಪ್ರತ್ಯಾಗಮ ಸತ್ಯ;
  }
  ಯದಿ (Needle.empty())
    ಪ್ರತ್ಯಾಗಮ ಸತ್ಯ;

  auto NamePos = Name.begin();
  auto NeedlePos = Needle.begin();

  ವರ್ಣ PreviousCharInNameOrigin = PreviousCharInName;
  ವರ್ಣ PreviousCharInNeedle = *Needle.begin();
  auto IgnoreSpaces = [](auto It, auto End, ವರ್ಣ &PreviousChar,
                         ತಾರ್ಕಿಕ IsPrefix = ಅಸತ್ಯ) {
    ಯಾವತ್ (It != End) {
      ಧ್ರುವ auto Next = std::next(It);
      // Ignore spaces, underscore, medial hyphens
      // The generator ensures a needle never ends (or starts) by a medial
      // hyphen https://unicode.org/reports/tr44/#UAX44-LM2.
      ತಾರ್ಕಿಕ Ignore =
          *It == ' ' || *It == '_' ||
          (*It == '-' && isAlnum(PreviousChar) &&
           ((Next != End && isAlnum(*Next)) || (Next == End && IsPrefix)));
      PreviousChar = *It;
      ಯದಿ (!Ignore)
        ಭಙ್ಗ;
      ++It;
    }
    ಪ್ರತ್ಯಾಗಮ It;
  };

  ಯಾವತ್ (ಸತ್ಯ) {
    NamePos = IgnoreSpaces(NamePos, Name.end(), PreviousCharInName);
    NeedlePos =
        IgnoreSpaces(NeedlePos, Needle.end(), PreviousCharInNeedle, IsPrefix);
    ಯದಿ (NeedlePos == Needle.end())
      ಭಙ್ಗ;
    ಯದಿ (NamePos == Name.end())
      ಭಙ್ಗ;
    ಯದಿ (toUpper(*NeedlePos) != toUpper(*NamePos))
      ಭಙ್ಗ;
    NeedlePos++;
    NamePos++;
  }
  Consummed = std::distance(Name.begin(), NamePos);
  ಯದಿ (NeedlePos != Needle.end()) {
    PreviousCharInName = PreviousCharInNameOrigin;
  }
  ಪ್ರತ್ಯಾಗಮ NeedlePos == Needle.end();
}

ಸ್ಥಿರ std::tuple<ಗಂಟು, ತಾರ್ಕಿಕ, uint32_t>
compare_ಗಂಟು(uint32_t Offset, StringRef Name, ತಾರ್ಕಿಕ Strict,
            ವರ್ಣ PreviousCharInName, BufferType &Buffer,
            ಧ್ರುವ ಗಂಟು *Parent = nullptr) {
  ಗಂಟು N = read_ಗಂಟು(Offset, Parent);
  std::size_t Consummed = 0;
  ತಾರ್ಕಿಕ DoesStartWith = N.Is_ಬೇರು || startsWith(Name, N.Name, Strict, Consummed,
                                              PreviousCharInName);
  ಯದಿ (!DoesStartWith)
    ಪ್ರತ್ಯಾಗಮ std::make_tuple(N, ಅಸತ್ಯ, 0);

  ಯದಿ (Name.size() - Consummed == 0 && N.ಮೌಲ್ಯ != 0xFFFFFFFF)
    ಪ್ರತ್ಯಾಗಮ std::make_tuple(N, ಸತ್ಯ, N.ಮೌಲ್ಯ);

  ಯದಿ (N.hasChildren()) {
    uint32_t ChildOffset = N.ChildrenOffset;
    ಪ್ರತಿ (;;) {
      ಗಂಟು C;
      ತಾರ್ಕಿಕ Matches;
      uint32_t ಮೌಲ್ಯ;
      std::tie(C, Matches, ಮೌಲ್ಯ) =
          compare_ಗಂಟು(ChildOffset, Name.substr(Consummed), Strict,
                      PreviousCharInName, Buffer, &N);
      ಯದಿ (Matches) {
        std::reverse_copy(C.Name.begin(), C.Name.end(),
                          std::back_inserter(Buffer));
        ಪ್ರತ್ಯಾಗಮ std::make_tuple(N, ಸತ್ಯ, ಮೌಲ್ಯ);
      }
      ChildOffset += C.Size;
      ಯದಿ (!C.HasSibling)
        ಭಙ್ಗ;
    }
  }
  ಪ್ರತ್ಯಾಗಮ std::make_tuple(N, ಅಸತ್ಯ, 0);
}

ಸ್ಥಿರ std::tuple<ಗಂಟು, ತಾರ್ಕಿಕ, uint32_t>
compare_ಗಂಟು(uint32_t Offset, StringRef Name, ತಾರ್ಕಿಕ Strict, BufferType &Buffer) {
  ಪ್ರತ್ಯಾಗಮ compare_ಗಂಟು(Offset, Name, Strict, 0, Buffer);
}

// clang-format off
constexpr ಧ್ರುವ ವರ್ಣ *ಧ್ರುವ HangulSyllables[][3] = {
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

ಸ್ಥಿರ std::size_t findSyllable(StringRef Name, ತಾರ್ಕಿಕ Strict,
                                ವರ್ಣ &PreviousInName, ಪೂರ್ಣಾಙ್ಕ &Pos, ಪೂರ್ಣಾಙ್ಕ Column) {
  ಪ್ರತಿಜ್ಞಾ(Column == 0 || Column == 1 || Column == 2);
  ಸ್ಥಿರ std::size_t ಎಣಿಕೆ_Per_Column[] = {LCount, VCount, TCount};
  ಪೂರ್ಣಾಙ್ಕ Len = -1;
  ಪೂರ್ಣಾಙ್ಕ Prev = PreviousInName;
  ಪ್ರತಿ (std::size_t I = 0; I < ಎಣಿಕೆ_Per_Column[Column]; I++) {
    StringRef Syllable(HangulSyllables[I][Column]);
    ಯದಿ (ಪೂರ್ಣಾಙ್ಕ(Syllable.size()) <= Len)
      ಅನುವರ್ತ;
    std::size_t Consummed = 0;
    ವರ್ಣ PreviousInNameCopy = PreviousInName;
    ತಾರ್ಕಿಕ DoesStartWith =
        startsWith(Name, Syllable, Strict, Consummed, PreviousInNameCopy);
    ಯದಿ (!DoesStartWith)
      ಅನುವರ್ತ;
    Len = Consummed;
    Pos = I;
    Prev = PreviousInNameCopy;
  }
  ಯದಿ (Len == -1)
    ಪ್ರತ್ಯಾಗಮ 0;
  PreviousInName = Prev;
  ಪ್ರತ್ಯಾಗಮ size_t(Len);
}

ಸ್ಥಿರ std::optional<char32_t>
nameToHangulCodePoint(StringRef Name, ತಾರ್ಕಿಕ Strict, BufferType &Buffer) {
  Buffer.clear();
  // Hangul Syllable Decomposition
  std::size_t Consummed = 0;
  ವರ್ಣ NameStart = 0;
  ತಾರ್ಕಿಕ DoesStartWith =
      startsWith(Name, "HANGUL SYLLABLE ", Strict, Consummed, NameStart);
  ಯದಿ (!DoesStartWith)
    ಪ್ರತ್ಯಾಗಮ std::nullopt;
  Name = Name.substr(Consummed);
  ಪೂರ್ಣಾಙ್ಕ L = -1, V = -1, T = -1;
  Name = Name.substr(findSyllable(Name, Strict, NameStart, L, 0));
  Name = Name.substr(findSyllable(Name, Strict, NameStart, V, 1));
  Name = Name.substr(findSyllable(Name, Strict, NameStart, T, 2));
  ಯದಿ (L != -1 && V != -1 && T != -1 && Name.empty()) {
    ಯದಿ (!Strict) {
      Buffer.append("HANGUL SYLLABLE ");
      ಯದಿ (L != -1)
        Buffer.append(HangulSyllables[L][0]);
      ಯದಿ (V != -1)
        Buffer.append(HangulSyllables[V][1]);
      ಯದಿ (T != -1)
        Buffer.append(HangulSyllables[T][2]);
    }
    ಪ್ರತ್ಯಾಗಮ SBase + (std::uint32_t(L) * VCount + std::uint32_t(V)) * TCount +
           std::uint32_t(T);
  }
  // Otherwise, it's an illegal syllable name.
  ಪ್ರತ್ಯಾಗಮ std::nullopt;
}

ಸಂರಚನಾ GeneratedNamesData {
  StringRef Prefix;
  uint32_t Start;
  uint32_t End;
};

// Unicode 18.0 Name Derivation Rule Prefix Strings (extracted/DerivedName.txt)
ಸ್ಥಿರ ಧ್ರುವ GeneratedNamesData GeneratedNamesDataTable[] = {
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

ಸ್ಥಿರ std::optional<char32_t>
nameToGeneratedCodePoint(StringRef Name, ತಾರ್ಕಿಕ Strict, BufferType &Buffer) {
  ಪ್ರತಿ (auto &&Item : GeneratedNamesDataTable) {
    Buffer.clear();
    std::size_t Consummed = 0;
    ವರ್ಣ NameStart = 0;
    ತಾರ್ಕಿಕ DoesStartWith = startsWith(Name, Item.Prefix, Strict, Consummed,
                                    NameStart, /*IsPrefix=*/ಸತ್ಯ);
    ಯದಿ (!DoesStartWith)
      ಅನುವರ್ತ;
    auto ಸಂಖ್ಯೆ = Name.substr(Consummed);
    unsigned long long V = 0;
    // Be consistent about mandating upper casing.
    ಯದಿ (Strict &&
        llvm::any_of(ಸಂಖ್ಯೆ, [](ವರ್ಣ C) { ಪ್ರತ್ಯಾಗಮ C >= 'a' && C <= 'f'; }))
      ಪ್ರತ್ಯಾಗಮ {};
    ಯದಿ (getAsUnsignedInteger(ಸಂಖ್ಯೆ, 16, V) || V < Item.Start || V > Item.End)
      ಅನುವರ್ತ;
    ಯದಿ (!Strict) {
      Buffer.append(Item.Prefix);
      Buffer.append(utohexstr(V, ಸತ್ಯ));
    }
    ಪ್ರತ್ಯಾಗಮ V;
  }
  ಪ್ರತ್ಯಾಗಮ std::nullopt;
}

ಸ್ಥಿರ std::optional<char32_t> nameToCodepoint(StringRef Name, ತಾರ್ಕಿಕ Strict,
                                               BufferType &Buffer) {
  ಯದಿ (Name.empty())
    ಪ್ರತ್ಯಾಗಮ std::nullopt;

  std::optional<char32_t> Res = nameToHangulCodePoint(Name, Strict, Buffer);
  ಯದಿ (!Res)
    Res = nameToGeneratedCodePoint(Name, Strict, Buffer);
  ಯದಿ (Res)
    ಪ್ರತ್ಯಾಗಮ *Res;

  Buffer.clear();
  ಗಂಟು ಗಂಟು;
  ತಾರ್ಕಿಕ Matches;
  uint32_t ಮೌಲ್ಯ;
  std::tie(ಗಂಟು, Matches, ಮೌಲ್ಯ) = compare_ಗಂಟು(0, Name, Strict, Buffer);
  ಯದಿ (Matches) {
    std::reverse(Buffer.begin(), Buffer.end());
    // UAX44-LM2. Ignore case, whitespace, underscore ('_'), and all medial
    // hyphens except the hyphen in U+1180 HANGUL JUNGSEONG O-E.
    ಯದಿ (!Strict && ಮೌಲ್ಯ == 0x116c && Name.contains_insensitive("O-E")) {
      Buffer = "HANGUL JUNGSEONG O-E";
      ಮೌಲ್ಯ = 0x1180;
    }
    ಪ್ರತ್ಯಾಗಮ ಮೌಲ್ಯ;
  }
  ಪ್ರತ್ಯಾಗಮ std::nullopt;
}

std::optional<char32_t> nameToCodepointStrict(StringRef Name) {

  BufferType Buffer;
  auto Opt = nameToCodepoint(Name, ಸತ್ಯ, Buffer);
  ಪ್ರತ್ಯಾಗಮ Opt;
}

std::optional<LooseMatchingResult>
nameToCodepointLooseMatching(StringRef Name) {
  BufferType Buffer;
  auto Opt = nameToCodepoint(Name, ಅಸತ್ಯ, Buffer);
  ಯದಿ (!Opt)
    ಪ್ರತ್ಯಾಗಮ std::nullopt;
  ಪ್ರತ್ಯಾಗಮ LooseMatchingResult{*Opt, Buffer};
}

// Find the unicode character whose editing distance to Pattern
// is shortest, using the Wagner–Fischer algorithm.
llvm::SmallVector<MatchForCodepointName>
nearestMatchesForCodepointName(StringRef Pattern, std::size_t Max_Matches_ಎಣಿಕೆ) {
  // We maintain a fixed size vector of matches,
  // sorted by distance
  // The worst match (with the biggest distance) are discarded when new elements
  // are added.
  std::size_t LargestEditDistance = 0;
  llvm::SmallVector<MatchForCodepointName> Matches;
  Matches.reserve(Max_Matches_ಎಣಿಕೆ + 1);

  auto ಸೇರಿಸು_2 = [&](ಧ್ರುವ ಗಂಟು &ಗಂಟು, uint32_t Distance,
                    char32_t ಮೌಲ್ಯ) -> ತಾರ್ಕಿಕ {
    ಯದಿ (Distance > LargestEditDistance) {
      ಯದಿ (Matches.size() == Max_Matches_ಎಣಿಕೆ)
        ಪ್ರತ್ಯಾಗಮ ಅಸತ್ಯ;
      LargestEditDistance = Distance;
    }
    // To avoid allocations, the creation of the name is delayed
    // as much as possible.
    std::string Name;
    auto GetName = [&] {
      ಯದಿ (Name.empty())
        Name = ಗಂಟು.fullName();
      ಪ್ರತ್ಯಾಗಮ Name;
    };

    auto It = llvm::lower_bound(
        Matches, Distance,
        [&](ಧ್ರುವ MatchForCodepointName &a, std::size_t Distance) {
          ಯದಿ (Distance == a.Distance)
            ಪ್ರತ್ಯಾಗಮ a.Name < GetName();
          ಪ್ರತ್ಯಾಗಮ a.Distance < Distance;
        });
    ಯದಿ (It == Matches.end() && Matches.size() == Max_Matches_ಎಣಿಕೆ)
      ಪ್ರತ್ಯಾಗಮ ಅಸತ್ಯ;

    MatchForCodepointName M{GetName(), Distance, ಮೌಲ್ಯ};
    Matches.ಸೇರಿಸು(It, std::move(M));
    ಯದಿ (Matches.size() > Max_Matches_ಎಣಿಕೆ)
      Matches.ತೆಗೆ_back();
    ಪ್ರತ್ಯಾಗಮ ಸತ್ಯ;
  };

  // We ignore case, space, hyphens, etc,
  // in both the search pattern and the prospective names.
  auto Normalize = [](StringRef Name) {
    std::string Out;
    Out.reserve(Name.size());
    ಪ್ರತಿ (ವರ್ಣ C : Name) {
      ಯದಿ (isAlnum(C))
        Out.ತಳ್ಳು_back(toUpper(C));
    }
    ಪ್ರತ್ಯಾಗಮ Out;
  };
  std::string NormalizedName = Normalize(Pattern);

  // Allocate a matrix big enough for longest names.
  ಧ್ರುವ std::size_t Columns =
      std::min(NormalizedName.size(), UnicodeNameToCodepointLargestNameSize) +
      1;

  [[maybe_unused]] ಸ್ಥಿರ std::size_t Rows =
      UnicodeNameToCodepointLargestNameSize + 1;

  std::vector<ವರ್ಣ> Distances(
      Columns * (UnicodeNameToCodepointLargestNameSize + 1), 0);

  auto Get = [&Distances, Columns](size_t Column, std::size_t Row) -> ವರ್ಣ & {
    ಪ್ರತಿಜ್ಞಾ(Column < Columns);
    ಪ್ರತಿಜ್ಞಾ(Row < Rows);
    ಪ್ರತ್ಯಾಗಮ Distances[Row * Columns + Column];
  };

  ಪ್ರತಿ (std::size_t I = 0; I < Columns; I++)
    Get(I, 0) = I;

  // Visit the childrens,
  // Filling (and overriding) the matrix for the name fragment of each node
  // iteratively. CompleteName is used to collect the actual name of potential
  // match, respecting case and spacing.
  auto Visit_ಗಂಟು = [&](ಧ್ರುವ ಗಂಟು &N, std::size_t Row,
                       auto &Visit_ಗಂಟು) -> ಶೂನ್ಯ {
    std::size_t J = 0;
    ಪ್ರತಿ (; J < N.Name.size(); J++) {
      ಯದಿ (!isAlnum(N.Name[J]))
        ಅನುವರ್ತ;

      Get(0, Row) = Row;

      ಪ್ರತಿ (std::size_t I = 1; I < Columns; I++) {
        ಧ್ರುವ ಪೂರ್ಣಾಙ್ಕ Delete = Get(I - 1, Row) + 1;
        ಧ್ರುವ ಪೂರ್ಣಾಙ್ಕ ಸೇರಿಸು_2 = Get(I, Row - 1) + 1;

        ಧ್ರುವ ಪೂರ್ಣಾಙ್ಕ Replace =
            Get(I - 1, Row - 1) + (NormalizedName[I - 1] != N.Name[J] ? 1 : 0);

        Get(I, Row) = std::min({ಸೇರಿಸು_2, Delete, Replace});
      }

      Row++;
    }

    unsigned Cost = Get(Columns - 1, Row - 1);
    ಯದಿ (N.ಮೌಲ್ಯ != 0xFFFFFFFF) {
      ಸೇರಿಸು_2(N, Cost, N.ಮೌಲ್ಯ);
    }

    ಯದಿ (N.hasChildren()) {
      auto ChildOffset = N.ChildrenOffset;
      ಪ್ರತಿ (;;) {
        ಗಂಟು C = read_ಗಂಟು(ChildOffset, &N);
        ChildOffset += C.Size;
        ಯದಿ (!C.isValid())
          ಭಙ್ಗ;
        Visit_ಗಂಟು(C, Row, Visit_ಗಂಟು);
        ಯದಿ (!C.HasSibling)
          ಭಙ್ಗ;
      }
    }
  };

  ಗಂಟು ಬೇರು_2 = create_ಬೇರು();
  Visit_ಗಂಟು(ಬೇರು_2, 1, Visit_ಗಂಟು);
  ಪ್ರತ್ಯಾಗಮ Matches;
}

} // namespace unicode

} // namespace sys
} // namespace llvm
