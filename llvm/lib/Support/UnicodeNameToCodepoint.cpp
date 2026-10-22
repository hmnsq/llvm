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

بنية گرہ {
  منطقي Is_جڑ = خطأ;
  char32_t قدر = 0xFFFFFFFF;
  uint32_t ChildrenOffset = 0;
  منطقي HasSibling = خطأ;
  uint32_t Size = 0;
  StringRef Name;
  ثابت گرہ *Parent = nullptr;

  constexpr منطقي isValid() ثابت {
    أرجع !Name.empty() || قدر == 0xFFFFFFFF;
  }
  constexpr منطقي hasChildren() ثابت { أرجع ChildrenOffset != 0 || Is_جڑ; }

  std::string fullName() ثابت {
    std::string S;
    // Reserve enough space for most unicode code points.
    // The chosen value represent the 99th percentile of name size as of
    // Unicode 15.0.
    S.reserve(46);
    ثابت گرہ *N = هذا;
    طالما (N) {
      std::reverse_copy(N->Name.begin(), N->Name.end(), std::back_inserter(S));
      N = N->Parent;
    }
    std::reverse(S.begin(), S.end());
    أرجع S;
  }
};

ساكن گرہ create_جڑ() {
  گرہ N;
  N.Is_جڑ = true;
  N.ChildrenOffset = 1;
  N.Size = 1;
  أرجع N;
}

ساكن گرہ read_گرہ(uint32_t Offset, ثابت گرہ *Parent = nullptr) {
  إذا (Offset == 0)
    أرجع create_جڑ();

  uint32_t Origin = Offset;
  گرہ N;
  N.Parent = Parent;
  uint8_t NameInfo = UnicodeNameToCodepointIndex[Offset++];
  إذا (Offset + 6 >= UnicodeNameToCodepointIndexSize)
    أرجع N;

  منطقي LongName = NameInfo & 0x40;
  منطقي Has_قدر = NameInfo & 0x80;
  std::size_t Size = NameInfo & ~0xC0;
  إذا (LongName) {
    uint32_t NameOffset = (UnicodeNameToCodepointIndex[Offset++] << 8);
    NameOffset |= UnicodeNameToCodepointIndex[Offset++];
    N.Name = StringRef(UnicodeNameToCodepointDict + NameOffset, Size);
  } وإلا {
    N.Name = StringRef(UnicodeNameToCodepointDict + Size, 1);
  }
  إذا (Has_قدر) {
    uint8_t H = UnicodeNameToCodepointIndex[Offset++];
    uint8_t M = UnicodeNameToCodepointIndex[Offset++];
    uint8_t L = UnicodeNameToCodepointIndex[Offset++];
    N.قدر = ((H << 16) | (M << 8) | L) >> 3;

    منطقي HasChildren = L & 0x02;
    N.HasSibling = L & 0x01;

    إذا (HasChildren) {
      N.ChildrenOffset = UnicodeNameToCodepointIndex[Offset++] << 16;
      N.ChildrenOffset |= UnicodeNameToCodepointIndex[Offset++] << 8;
      N.ChildrenOffset |= UnicodeNameToCodepointIndex[Offset++];
    }
  } وإلا {
    uint8_t H = UnicodeNameToCodepointIndex[Offset++];
    N.HasSibling = H & 0x80;
    منطقي HasChildren = H & 0x40;
    H &= uint8_t(~0xC0);
    إذا (HasChildren) {
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
  إذا (Strict) {
    إذا (!Name.starts_with(Needle))
      أرجع خطأ;
    Consummed = Needle.size();
    أرجع true;
  }
  إذا (Needle.empty())
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
      إذا (!Ignore)
        اكسر;
      ++It;
    }
    أرجع It;
  };

  طالما (true) {
    NamePos = IgnoreSpaces(NamePos, Name.end(), PreviousCharInName);
    NeedlePos =
        IgnoreSpaces(NeedlePos, Needle.end(), PreviousCharInNeedle, IsPrefix);
    إذا (NeedlePos == Needle.end())
      اكسر;
    إذا (NamePos == Name.end())
      اكسر;
    إذا (toUpper(*NeedlePos) != toUpper(*NamePos))
      اكسر;
    NeedlePos++;
    NamePos++;
  }
  Consummed = std::distance(Name.begin(), NamePos);
  إذا (NeedlePos != Needle.end()) {
    PreviousCharInName = PreviousCharInNameOrigin;
  }
  أرجع NeedlePos == Needle.end();
}

ساكن std::tuple<گرہ, منطقي, uint32_t>
compare_گرہ(uint32_t Offset, StringRef Name, منطقي Strict,
            حرف PreviousCharInName, BufferType &Buffer,
            ثابت گرہ *Parent = nullptr) {
  گرہ N = read_گرہ(Offset, Parent);
  std::size_t Consummed = 0;
  منطقي DoesStartWith = N.Is_جڑ || startsWith(Name, N.Name, Strict, Consummed,
                                              PreviousCharInName);
  إذا (!DoesStartWith)
    أرجع std::make_tuple(N, خطأ, 0);

  إذا (Name.size() - Consummed == 0 && N.قدر != 0xFFFFFFFF)
    أرجع std::make_tuple(N, true, N.قدر);

  إذا (N.hasChildren()) {
    uint32_t ChildOffset = N.ChildrenOffset;
    لكل (;;) {
      گرہ C;
      منطقي Matches;
      uint32_t قدر;
      std::tie(C, Matches, قدر) =
          compare_گرہ(ChildOffset, Name.substr(Consummed), Strict,
                      PreviousCharInName, Buffer, &N);
      إذا (Matches) {
        std::reverse_copy(C.Name.begin(), C.Name.end(),
                          std::back_inserter(Buffer));
        أرجع std::make_tuple(N, true, قدر);
      }
      ChildOffset += C.Size;
      إذا (!C.HasSibling)
        اكسر;
    }
  }
  أرجع std::make_tuple(N, خطأ, 0);
}

ساكن std::tuple<گرہ, منطقي, uint32_t>
compare_گرہ(uint32_t Offset, StringRef Name, منطقي Strict, BufferType &Buffer) {
  أرجع compare_گرہ(Offset, Name, Strict, 0, Buffer);
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
  ساكن std::size_t گنتی_Per_Column[] = {LCount, VCount, TCount};
  صحيح Len = -1;
  صحيح Prev = PreviousInName;
  لكل (std::size_t I = 0; I < گنتی_Per_Column[Column]; I++) {
    StringRef Syllable(HangulSyllables[I][Column]);
    إذا (صحيح(Syllable.size()) <= Len)
      استمر;
    std::size_t Consummed = 0;
    حرف PreviousInNameCopy = PreviousInName;
    منطقي DoesStartWith =
        startsWith(Name, Syllable, Strict, Consummed, PreviousInNameCopy);
    إذا (!DoesStartWith)
      استمر;
    Len = Consummed;
    Pos = I;
    Prev = PreviousInNameCopy;
  }
  إذا (Len == -1)
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
  إذا (!DoesStartWith)
    أرجع std::nullopt;
  Name = Name.substr(Consummed);
  صحيح L = -1, V = -1, T = -1;
  Name = Name.substr(findSyllable(Name, Strict, NameStart, L, 0));
  Name = Name.substr(findSyllable(Name, Strict, NameStart, V, 1));
  Name = Name.substr(findSyllable(Name, Strict, NameStart, T, 2));
  إذا (L != -1 && V != -1 && T != -1 && Name.empty()) {
    إذا (!Strict) {
      Buffer.append("HANGUL SYLLABLE ");
      إذا (L != -1)
        Buffer.append(HangulSyllables[L][0]);
      إذا (V != -1)
        Buffer.append(HangulSyllables[V][1]);
      إذا (T != -1)
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
    إذا (!DoesStartWith)
      استمر;
    auto نمبر = Name.substr(Consummed);
    unsigned long long V = 0;
    // Be consistent about mandating upper casing.
    إذا (Strict &&
        llvm::any_of(نمبر, [](حرف C) { أرجع C >= 'a' && C <= 'f'; }))
      أرجع {};
    إذا (getAsUnsignedInteger(نمبر, 16, V) || V < Item.Start || V > Item.End)
      استمر;
    إذا (!Strict) {
      Buffer.append(Item.Prefix);
      Buffer.append(utohexstr(V, true));
    }
    أرجع V;
  }
  أرجع std::nullopt;
}

ساكن std::optional<char32_t> nameToCodepoint(StringRef Name, منطقي Strict,
                                               BufferType &Buffer) {
  إذا (Name.empty())
    أرجع std::nullopt;

  std::optional<char32_t> Res = nameToHangulCodePoint(Name, Strict, Buffer);
  إذا (!Res)
    Res = nameToGeneratedCodePoint(Name, Strict, Buffer);
  إذا (Res)
    أرجع *Res;

  Buffer.clear();
  گرہ گرہ;
  منطقي Matches;
  uint32_t قدر;
  std::tie(گرہ, Matches, قدر) = compare_گرہ(0, Name, Strict, Buffer);
  إذا (Matches) {
    std::reverse(Buffer.begin(), Buffer.end());
    // UAX44-LM2. Ignore case, whitespace, underscore ('_'), and all medial
    // hyphens except the hyphen in U+1180 HANGUL JUNGSEONG O-E.
    إذا (!Strict && قدر == 0x116c && Name.contains_insensitive("O-E")) {
      Buffer = "HANGUL JUNGSEONG O-E";
      قدر = 0x1180;
    }
    أرجع قدر;
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
  إذا (!Opt)
    أرجع std::nullopt;
  أرجع LooseMatchingResult{*Opt, Buffer};
}

// Find the unicode character whose editing distance to Pattern
// is shortest, using the Wagner–Fischer algorithm.
llvm::SmallVector<MatchForCodepointName>
nearestMatchesForCodepointName(StringRef Pattern, std::size_t Max_Matches_گنتی) {
  // We maintain a fixed size vector of matches,
  // sorted by distance
  // The worst match (with the biggest distance) are discarded when new elements
  // are added.
  std::size_t LargestEditDistance = 0;
  llvm::SmallVector<MatchForCodepointName> Matches;
  Matches.reserve(Max_Matches_گنتی + 1);

  auto داخل_کرو_2 = [&](ثابت گرہ &گرہ, uint32_t Distance,
                    char32_t قدر) -> منطقي {
    إذا (Distance > LargestEditDistance) {
      إذا (Matches.size() == Max_Matches_گنتی)
        أرجع خطأ;
      LargestEditDistance = Distance;
    }
    // To avoid allocations, the creation of the name is delayed
    // as much as possible.
    std::string Name;
    auto GetName = [&] {
      إذا (Name.empty())
        Name = گرہ.fullName();
      أرجع Name;
    };

    auto It = llvm::lower_bound(
        Matches, Distance,
        [&](ثابت MatchForCodepointName &a, std::size_t Distance) {
          إذا (Distance == a.Distance)
            أرجع a.Name < GetName();
          أرجع a.Distance < Distance;
        });
    إذا (It == Matches.end() && Matches.size() == Max_Matches_گنتی)
      أرجع خطأ;

    MatchForCodepointName M{GetName(), Distance, قدر};
    Matches.داخل_کرو(It, std::move(M));
    إذا (Matches.size() > Max_Matches_گنتی)
      Matches.نکالو_back();
    أرجع true;
  };

  // We ignore case, space, hyphens, etc,
  // in both the search pattern and the prospective names.
  auto Normalize = [](StringRef Name) {
    std::string Out;
    Out.reserve(Name.size());
    لكل (حرف C : Name) {
      إذا (isAlnum(C))
        Out.ڈالو_back(toUpper(C));
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
  auto Visit_گرہ = [&](ثابت گرہ &N, std::size_t Row,
                       auto &Visit_گرہ) -> فراغ {
    std::size_t J = 0;
    لكل (; J < N.Name.size(); J++) {
      إذا (!isAlnum(N.Name[J]))
        استمر;

      Get(0, Row) = Row;

      لكل (std::size_t I = 1; I < Columns; I++) {
        ثابت صحيح Delete = Get(I - 1, Row) + 1;
        ثابت صحيح داخل_کرو_2 = Get(I, Row - 1) + 1;

        ثابت صحيح Replace =
            Get(I - 1, Row - 1) + (NormalizedName[I - 1] != N.Name[J] ? 1 : 0);

        Get(I, Row) = std::min({داخل_کرو_2, Delete, Replace});
      }

      Row++;
    }

    unsigned Cost = Get(Columns - 1, Row - 1);
    إذا (N.قدر != 0xFFFFFFFF) {
      داخل_کرو_2(N, Cost, N.قدر);
    }

    إذا (N.hasChildren()) {
      auto ChildOffset = N.ChildrenOffset;
      لكل (;;) {
        گرہ C = read_گرہ(ChildOffset, &N);
        ChildOffset += C.Size;
        إذا (!C.isValid())
          اكسر;
        Visit_گرہ(C, Row, Visit_گرہ);
        إذا (!C.HasSibling)
          اكسر;
      }
    }
  };

  گرہ جڑ_2 = create_جڑ();
  Visit_گرہ(جڑ_2, 1, Visit_گرہ);
  أرجع Matches;
}

} // namespace unicode

} // namespace sys
} // namespace llvm
