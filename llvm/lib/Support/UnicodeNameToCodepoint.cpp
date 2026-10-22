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

#شامل "llvm/ADT/STLExtras.h"
#شامل "llvm/ADT/StringExtras.h"
#شامل "llvm/ADT/StringRef.h"
#شامل "llvm/Support/Unicode.h"

فضاینام llvm {
فضاینام sys {
فضاینام unicode {

بیرونی ثابت نویسه *ثابت UnicodeNameToCodepointDict;
بیرونی ثابت uint8_t *ثابت UnicodeNameToCodepointIndex;
بیرونی ثابت std::size_t UnicodeNameToCodepointIndexSize;
بیرونی ثابت std::size_t UnicodeNameToCodepointLargestNameSize;

using BufferType = SmallString<64>;

ساختار گره {
  منطقی Is_ریشه = نادرست;
  char32_t مقدار = 0xFFFFFFFF;
  uint32_t ChildrenOffset = 0;
  منطقی HasSibling = نادرست;
  uint32_t Size = 0;
  StringRef Name;
  ثابت گره *Parent = nullptr;

  constexpr منطقی isValid() ثابت {
    بازگشت !Name.empty() || مقدار == 0xFFFFFFFF;
  }
  constexpr منطقی hasChildren() ثابت { بازگشت ChildrenOffset != 0 || Is_ریشه; }

  std::string fullName() ثابت {
    std::string S;
    // Reserve enough space for most unicode code points.
    // The chosen value represent the 99th percentile of name size as of
    // Unicode 15.0.
    S.reserve(46);
    ثابت گره *N = این;
    وقتی (N) {
      std::reverse_copy(N->Name.begin(), N->Name.end(), std::back_inserter(S));
      N = N->Parent;
    }
    std::reverse(S.begin(), S.end());
    بازگشت S;
  }
};

ایستا گره create_ریشه() {
  گره N;
  N.Is_ریشه = درست;
  N.ChildrenOffset = 1;
  N.Size = 1;
  بازگشت N;
}

ایستا گره read_گره(uint32_t Offset, ثابت گره *Parent = nullptr) {
  اگر (Offset == 0)
    بازگشت create_ریشه();

  uint32_t Origin = Offset;
  گره N;
  N.Parent = Parent;
  uint8_t NameInfo = UnicodeNameToCodepointIndex[Offset++];
  اگر (Offset + 6 >= UnicodeNameToCodepointIndexSize)
    بازگشت N;

  منطقی LongName = NameInfo & 0x40;
  منطقی Has_مقدار = NameInfo & 0x80;
  std::size_t Size = NameInfo & ~0xC0;
  اگر (LongName) {
    uint32_t NameOffset = (UnicodeNameToCodepointIndex[Offset++] << 8);
    NameOffset |= UnicodeNameToCodepointIndex[Offset++];
    N.Name = StringRef(UnicodeNameToCodepointDict + NameOffset, Size);
  } وگرنه {
    N.Name = StringRef(UnicodeNameToCodepointDict + Size, 1);
  }
  اگر (Has_مقدار) {
    uint8_t H = UnicodeNameToCodepointIndex[Offset++];
    uint8_t M = UnicodeNameToCodepointIndex[Offset++];
    uint8_t L = UnicodeNameToCodepointIndex[Offset++];
    N.مقدار = ((H << 16) | (M << 8) | L) >> 3;

    منطقی HasChildren = L & 0x02;
    N.HasSibling = L & 0x01;

    اگر (HasChildren) {
      N.ChildrenOffset = UnicodeNameToCodepointIndex[Offset++] << 16;
      N.ChildrenOffset |= UnicodeNameToCodepointIndex[Offset++] << 8;
      N.ChildrenOffset |= UnicodeNameToCodepointIndex[Offset++];
    }
  } وگرنه {
    uint8_t H = UnicodeNameToCodepointIndex[Offset++];
    N.HasSibling = H & 0x80;
    منطقی HasChildren = H & 0x40;
    H &= uint8_t(~0xC0);
    اگر (HasChildren) {
      N.ChildrenOffset = (H << 16);
      N.ChildrenOffset |=
          (uint32_t(UnicodeNameToCodepointIndex[Offset++]) << 8);
      N.ChildrenOffset |= UnicodeNameToCodepointIndex[Offset++];
    }
  }
  N.Size = Offset - Origin;
  بازگشت N;
}

ایستا منطقی startsWith(StringRef Name, StringRef Needle, منطقی Strict,
                       std::size_t &Consummed, نویسه &PreviousCharInName,
                       منطقی IsPrefix = نادرست) {

  Consummed = 0;
  اگر (Strict) {
    اگر (!Name.starts_with(Needle))
      بازگشت نادرست;
    Consummed = Needle.size();
    بازگشت درست;
  }
  اگر (Needle.empty())
    بازگشت درست;

  auto NamePos = Name.begin();
  auto NeedlePos = Needle.begin();

  نویسه PreviousCharInNameOrigin = PreviousCharInName;
  نویسه PreviousCharInNeedle = *Needle.begin();
  auto IgnoreSpaces = [](auto It, auto End, نویسه &PreviousChar,
                         منطقی IsPrefix = نادرست) {
    وقتی (It != End) {
      ثابت auto Next = std::next(It);
      // Ignore spaces, underscore, medial hyphens
      // The generator ensures a needle never ends (or starts) by a medial
      // hyphen https://unicode.org/reports/tr44/#UAX44-LM2.
      منطقی Ignore =
          *It == ' ' || *It == '_' ||
          (*It == '-' && isAlnum(PreviousChar) &&
           ((Next != End && isAlnum(*Next)) || (Next == End && IsPrefix)));
      PreviousChar = *It;
      اگر (!Ignore)
        بشکن;
      ++It;
    }
    بازگشت It;
  };

  وقتی (درست) {
    NamePos = IgnoreSpaces(NamePos, Name.end(), PreviousCharInName);
    NeedlePos =
        IgnoreSpaces(NeedlePos, Needle.end(), PreviousCharInNeedle, IsPrefix);
    اگر (NeedlePos == Needle.end())
      بشکن;
    اگر (NamePos == Name.end())
      بشکن;
    اگر (toUpper(*NeedlePos) != toUpper(*NamePos))
      بشکن;
    NeedlePos++;
    NamePos++;
  }
  Consummed = std::distance(Name.begin(), NamePos);
  اگر (NeedlePos != Needle.end()) {
    PreviousCharInName = PreviousCharInNameOrigin;
  }
  بازگشت NeedlePos == Needle.end();
}

ایستا std::tuple<گره, منطقی, uint32_t>
compare_گره(uint32_t Offset, StringRef Name, منطقی Strict,
            نویسه PreviousCharInName, BufferType &Buffer,
            ثابت گره *Parent = nullptr) {
  گره N = read_گره(Offset, Parent);
  std::size_t Consummed = 0;
  منطقی DoesStartWith = N.Is_ریشه || startsWith(Name, N.Name, Strict, Consummed,
                                              PreviousCharInName);
  اگر (!DoesStartWith)
    بازگشت std::make_tuple(N, نادرست, 0);

  اگر (Name.size() - Consummed == 0 && N.مقدار != 0xFFFFFFFF)
    بازگشت std::make_tuple(N, درست, N.مقدار);

  اگر (N.hasChildren()) {
    uint32_t ChildOffset = N.ChildrenOffset;
    برای (;;) {
      گره C;
      منطقی Matches;
      uint32_t مقدار;
      std::tie(C, Matches, مقدار) =
          compare_گره(ChildOffset, Name.substr(Consummed), Strict,
                      PreviousCharInName, Buffer, &N);
      اگر (Matches) {
        std::reverse_copy(C.Name.begin(), C.Name.end(),
                          std::back_inserter(Buffer));
        بازگشت std::make_tuple(N, درست, مقدار);
      }
      ChildOffset += C.Size;
      اگر (!C.HasSibling)
        بشکن;
    }
  }
  بازگشت std::make_tuple(N, نادرست, 0);
}

ایستا std::tuple<گره, منطقی, uint32_t>
compare_گره(uint32_t Offset, StringRef Name, منطقی Strict, BufferType &Buffer) {
  بازگشت compare_گره(Offset, Name, Strict, 0, Buffer);
}

// clang-format off
constexpr ثابت نویسه *ثابت HangulSyllables[][3] = {
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

ایستا std::size_t findSyllable(StringRef Name, منطقی Strict,
                                نویسه &PreviousInName, صحیح &Pos, صحیح Column) {
  أكد(Column == 0 || Column == 1 || Column == 2);
  ایستا std::size_t شمار_Per_Column[] = {LCount, VCount, TCount};
  صحیح Len = -1;
  صحیح Prev = PreviousInName;
  برای (std::size_t I = 0; I < شمار_Per_Column[Column]; I++) {
    StringRef Syllable(HangulSyllables[I][Column]);
    اگر (صحیح(Syllable.size()) <= Len)
      ادامه;
    std::size_t Consummed = 0;
    نویسه PreviousInNameCopy = PreviousInName;
    منطقی DoesStartWith =
        startsWith(Name, Syllable, Strict, Consummed, PreviousInNameCopy);
    اگر (!DoesStartWith)
      ادامه;
    Len = Consummed;
    Pos = I;
    Prev = PreviousInNameCopy;
  }
  اگر (Len == -1)
    بازگشت 0;
  PreviousInName = Prev;
  بازگشت size_t(Len);
}

ایستا std::optional<char32_t>
nameToHangulCodePoint(StringRef Name, منطقی Strict, BufferType &Buffer) {
  Buffer.clear();
  // Hangul Syllable Decomposition
  std::size_t Consummed = 0;
  نویسه NameStart = 0;
  منطقی DoesStartWith =
      startsWith(Name, "HANGUL SYLLABLE ", Strict, Consummed, NameStart);
  اگر (!DoesStartWith)
    بازگشت std::nullopt;
  Name = Name.substr(Consummed);
  صحیح L = -1, V = -1, T = -1;
  Name = Name.substr(findSyllable(Name, Strict, NameStart, L, 0));
  Name = Name.substr(findSyllable(Name, Strict, NameStart, V, 1));
  Name = Name.substr(findSyllable(Name, Strict, NameStart, T, 2));
  اگر (L != -1 && V != -1 && T != -1 && Name.empty()) {
    اگر (!Strict) {
      Buffer.append("HANGUL SYLLABLE ");
      اگر (L != -1)
        Buffer.append(HangulSyllables[L][0]);
      اگر (V != -1)
        Buffer.append(HangulSyllables[V][1]);
      اگر (T != -1)
        Buffer.append(HangulSyllables[T][2]);
    }
    بازگشت SBase + (std::uint32_t(L) * VCount + std::uint32_t(V)) * TCount +
           std::uint32_t(T);
  }
  // Otherwise, it's an illegal syllable name.
  بازگشت std::nullopt;
}

ساختار GeneratedNamesData {
  StringRef Prefix;
  uint32_t Start;
  uint32_t End;
};

// Unicode 18.0 Name Derivation Rule Prefix Strings (extracted/DerivedName.txt)
ایستا ثابت GeneratedNamesData GeneratedNamesDataTable[] = {
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

ایستا std::optional<char32_t>
nameToGeneratedCodePoint(StringRef Name, منطقی Strict, BufferType &Buffer) {
  برای (auto &&Item : GeneratedNamesDataTable) {
    Buffer.clear();
    std::size_t Consummed = 0;
    نویسه NameStart = 0;
    منطقی DoesStartWith = startsWith(Name, Item.Prefix, Strict, Consummed,
                                    NameStart, /*IsPrefix=*/درست);
    اگر (!DoesStartWith)
      ادامه;
    auto عدد = Name.substr(Consummed);
    unsigned long long V = 0;
    // Be consistent about mandating upper casing.
    اگر (Strict &&
        llvm::any_of(عدد, [](نویسه C) { بازگشت C >= 'a' && C <= 'f'; }))
      بازگشت {};
    اگر (getAsUnsignedInteger(عدد, 16, V) || V < Item.Start || V > Item.End)
      ادامه;
    اگر (!Strict) {
      Buffer.append(Item.Prefix);
      Buffer.append(utohexstr(V, درست));
    }
    بازگشت V;
  }
  بازگشت std::nullopt;
}

ایستا std::optional<char32_t> nameToCodepoint(StringRef Name, منطقی Strict,
                                               BufferType &Buffer) {
  اگر (Name.empty())
    بازگشت std::nullopt;

  std::optional<char32_t> Res = nameToHangulCodePoint(Name, Strict, Buffer);
  اگر (!Res)
    Res = nameToGeneratedCodePoint(Name, Strict, Buffer);
  اگر (Res)
    بازگشت *Res;

  Buffer.clear();
  گره گره;
  منطقی Matches;
  uint32_t مقدار;
  std::tie(گره, Matches, مقدار) = compare_گره(0, Name, Strict, Buffer);
  اگر (Matches) {
    std::reverse(Buffer.begin(), Buffer.end());
    // UAX44-LM2. Ignore case, whitespace, underscore ('_'), and all medial
    // hyphens except the hyphen in U+1180 HANGUL JUNGSEONG O-E.
    اگر (!Strict && مقدار == 0x116c && Name.contains_insensitive("O-E")) {
      Buffer = "HANGUL JUNGSEONG O-E";
      مقدار = 0x1180;
    }
    بازگشت مقدار;
  }
  بازگشت std::nullopt;
}

std::optional<char32_t> nameToCodepointStrict(StringRef Name) {

  BufferType Buffer;
  auto Opt = nameToCodepoint(Name, درست, Buffer);
  بازگشت Opt;
}

std::optional<LooseMatchingResult>
nameToCodepointLooseMatching(StringRef Name) {
  BufferType Buffer;
  auto Opt = nameToCodepoint(Name, نادرست, Buffer);
  اگر (!Opt)
    بازگشت std::nullopt;
  بازگشت LooseMatchingResult{*Opt, Buffer};
}

// Find the unicode character whose editing distance to Pattern
// is shortest, using the Wagner–Fischer algorithm.
llvm::SmallVector<MatchForCodepointName>
nearestMatchesForCodepointName(StringRef Pattern, std::size_t Max_Matches_شمار) {
  // We maintain a fixed size vector of matches,
  // sorted by distance
  // The worst match (with the biggest distance) are discarded when new elements
  // are added.
  std::size_t LargestEditDistance = 0;
  llvm::SmallVector<MatchForCodepointName> Matches;
  Matches.reserve(Max_Matches_شمار + 1);

  auto درج_2 = [&](ثابت گره &گره, uint32_t Distance,
                    char32_t مقدار) -> منطقی {
    اگر (Distance > LargestEditDistance) {
      اگر (Matches.size() == Max_Matches_شمار)
        بازگشت نادرست;
      LargestEditDistance = Distance;
    }
    // To avoid allocations, the creation of the name is delayed
    // as much as possible.
    std::string Name;
    auto GetName = [&] {
      اگر (Name.empty())
        Name = گره.fullName();
      بازگشت Name;
    };

    auto It = llvm::lower_bound(
        Matches, Distance,
        [&](ثابت MatchForCodepointName &a, std::size_t Distance) {
          اگر (Distance == a.Distance)
            بازگشت a.Name < GetName();
          بازگشت a.Distance < Distance;
        });
    اگر (It == Matches.end() && Matches.size() == Max_Matches_شمار)
      بازگشت نادرست;

    MatchForCodepointName M{GetName(), Distance, مقدار};
    Matches.درج(It, std::move(M));
    اگر (Matches.size() > Max_Matches_شمار)
      Matches.برداشت_back();
    بازگشت درست;
  };

  // We ignore case, space, hyphens, etc,
  // in both the search pattern and the prospective names.
  auto Normalize = [](StringRef Name) {
    std::string Out;
    Out.reserve(Name.size());
    برای (نویسه C : Name) {
      اگر (isAlnum(C))
        Out.افزودن_back(toUpper(C));
    }
    بازگشت Out;
  };
  std::string NormalizedName = Normalize(Pattern);

  // Allocate a matrix big enough for longest names.
  ثابت std::size_t Columns =
      std::min(NormalizedName.size(), UnicodeNameToCodepointLargestNameSize) +
      1;

  [[maybe_unused]] ایستا std::size_t Rows =
      UnicodeNameToCodepointLargestNameSize + 1;

  std::vector<نویسه> Distances(
      Columns * (UnicodeNameToCodepointLargestNameSize + 1), 0);

  auto Get = [&Distances, Columns](size_t Column, std::size_t Row) -> نویسه & {
    أكد(Column < Columns);
    أكد(Row < Rows);
    بازگشت Distances[Row * Columns + Column];
  };

  برای (std::size_t I = 0; I < Columns; I++)
    Get(I, 0) = I;

  // Visit the childrens,
  // Filling (and overriding) the matrix for the name fragment of each node
  // iteratively. CompleteName is used to collect the actual name of potential
  // match, respecting case and spacing.
  auto Visit_گره = [&](ثابت گره &N, std::size_t Row,
                       auto &Visit_گره) -> تهی {
    std::size_t J = 0;
    برای (; J < N.Name.size(); J++) {
      اگر (!isAlnum(N.Name[J]))
        ادامه;

      Get(0, Row) = Row;

      برای (std::size_t I = 1; I < Columns; I++) {
        ثابت صحیح Delete = Get(I - 1, Row) + 1;
        ثابت صحیح درج_2 = Get(I, Row - 1) + 1;

        ثابت صحیح Replace =
            Get(I - 1, Row - 1) + (NormalizedName[I - 1] != N.Name[J] ? 1 : 0);

        Get(I, Row) = std::min({درج_2, Delete, Replace});
      }

      Row++;
    }

    unsigned Cost = Get(Columns - 1, Row - 1);
    اگر (N.مقدار != 0xFFFFFFFF) {
      درج_2(N, Cost, N.مقدار);
    }

    اگر (N.hasChildren()) {
      auto ChildOffset = N.ChildrenOffset;
      برای (;;) {
        گره C = read_گره(ChildOffset, &N);
        ChildOffset += C.Size;
        اگر (!C.isValid())
          بشکن;
        Visit_گره(C, Row, Visit_گره);
        اگر (!C.HasSibling)
          بشکن;
      }
    }
  };

  گره ریشه_2 = create_ریشه();
  Visit_گره(ریشه_2, 1, Visit_گره);
  بازگشت Matches;
}

} // namespace unicode

} // namespace sys
} // namespace llvm
