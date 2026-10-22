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

#அந்தர்பாவ "llvm/ADT/STLExtras.h"
#அந்தர்பாவ "llvm/ADT/StringExtras.h"
#அந்தர்பாவ "llvm/ADT/StringRef.h"
#அந்தர்பாவ "llvm/Support/Unicode.h"

நாமாகாஶ llvm {
நாமாகாஶ sys {
நாமாகாஶ unicode {

வெளி மாறிலி எழுத்து *மாறிலி UnicodeNameToCodepointDict;
வெளி மாறிலி uint8_t *மாறிலி UnicodeNameToCodepointIndex;
வெளி மாறிலி std::size_t UnicodeNameToCodepointIndexSize;
வெளி மாறிலி std::size_t UnicodeNameToCodepointLargestNameSize;

using BufferType = SmallString<64>;

கட்டமைப்பு முனை {
  தருக்க Is_வேர் = பொய்;
  char32_t மதிப்பு = 0xFFFFFFFF;
  uint32_t ChildrenOffset = 0;
  தருக்க HasSibling = பொய்;
  uint32_t Size = 0;
  StringRef Name;
  மாறிலி முனை *Parent = nullptr;

  constexpr தருக்க isValid() மாறிலி {
    திரும்பு !Name.empty() || மதிப்பு == 0xFFFFFFFF;
  }
  constexpr தருக்க hasChildren() மாறிலி { திரும்பு ChildrenOffset != 0 || Is_வேர்; }

  std::string fullName() மாறிலி {
    std::string S;
    // Reserve enough space for most unicode code points.
    // The chosen value represent the 99th percentile of name size as of
    // Unicode 15.0.
    S.reserve(46);
    மாறிலி முனை *N = இது;
    போது (N) {
      std::reverse_copy(N->Name.begin(), N->Name.end(), std::back_inserter(S));
      N = N->Parent;
    }
    std::reverse(S.begin(), S.end());
    திரும்பு S;
  }
};

நிலையான முனை create_வேர்() {
  முனை N;
  N.Is_வேர் = உண்மை;
  N.ChildrenOffset = 1;
  N.Size = 1;
  திரும்பு N;
}

நிலையான முனை read_முனை(uint32_t Offset, மாறிலி முனை *Parent = nullptr) {
  எனில் (Offset == 0)
    திரும்பு create_வேர்();

  uint32_t Origin = Offset;
  முனை N;
  N.Parent = Parent;
  uint8_t NameInfo = UnicodeNameToCodepointIndex[Offset++];
  எனில் (Offset + 6 >= UnicodeNameToCodepointIndexSize)
    திரும்பு N;

  தருக்க LongName = NameInfo & 0x40;
  தருக்க Has_மதிப்பு = NameInfo & 0x80;
  std::size_t Size = NameInfo & ~0xC0;
  எனில் (LongName) {
    uint32_t NameOffset = (UnicodeNameToCodepointIndex[Offset++] << 8);
    NameOffset |= UnicodeNameToCodepointIndex[Offset++];
    N.Name = StringRef(UnicodeNameToCodepointDict + NameOffset, Size);
  } இல்லையெனில் {
    N.Name = StringRef(UnicodeNameToCodepointDict + Size, 1);
  }
  எனில் (Has_மதிப்பு) {
    uint8_t H = UnicodeNameToCodepointIndex[Offset++];
    uint8_t M = UnicodeNameToCodepointIndex[Offset++];
    uint8_t L = UnicodeNameToCodepointIndex[Offset++];
    N.மதிப்பு = ((H << 16) | (M << 8) | L) >> 3;

    தருக்க HasChildren = L & 0x02;
    N.HasSibling = L & 0x01;

    எனில் (HasChildren) {
      N.ChildrenOffset = UnicodeNameToCodepointIndex[Offset++] << 16;
      N.ChildrenOffset |= UnicodeNameToCodepointIndex[Offset++] << 8;
      N.ChildrenOffset |= UnicodeNameToCodepointIndex[Offset++];
    }
  } இல்லையெனில் {
    uint8_t H = UnicodeNameToCodepointIndex[Offset++];
    N.HasSibling = H & 0x80;
    தருக்க HasChildren = H & 0x40;
    H &= uint8_t(~0xC0);
    எனில் (HasChildren) {
      N.ChildrenOffset = (H << 16);
      N.ChildrenOffset |=
          (uint32_t(UnicodeNameToCodepointIndex[Offset++]) << 8);
      N.ChildrenOffset |= UnicodeNameToCodepointIndex[Offset++];
    }
  }
  N.Size = Offset - Origin;
  திரும்பு N;
}

நிலையான தருக்க startsWith(StringRef Name, StringRef Needle, தருக்க Strict,
                       std::size_t &Consummed, எழுத்து &PreviousCharInName,
                       தருக்க IsPrefix = பொய்) {

  Consummed = 0;
  எனில் (Strict) {
    எனில் (!Name.starts_with(Needle))
      திரும்பு பொய்;
    Consummed = Needle.size();
    திரும்பு உண்மை;
  }
  எனில் (Needle.empty())
    திரும்பு உண்மை;

  auto NamePos = Name.begin();
  auto NeedlePos = Needle.begin();

  எழுத்து PreviousCharInNameOrigin = PreviousCharInName;
  எழுத்து PreviousCharInNeedle = *Needle.begin();
  auto IgnoreSpaces = [](auto It, auto End, எழுத்து &PreviousChar,
                         தருக்க IsPrefix = பொய்) {
    போது (It != End) {
      மாறிலி auto Next = std::next(It);
      // Ignore spaces, underscore, medial hyphens
      // The generator ensures a needle never ends (or starts) by a medial
      // hyphen https://unicode.org/reports/tr44/#UAX44-LM2.
      தருக்க Ignore =
          *It == ' ' || *It == '_' ||
          (*It == '-' && isAlnum(PreviousChar) &&
           ((Next != End && isAlnum(*Next)) || (Next == End && IsPrefix)));
      PreviousChar = *It;
      எனில் (!Ignore)
        நிறுத்து;
      ++It;
    }
    திரும்பு It;
  };

  போது (உண்மை) {
    NamePos = IgnoreSpaces(NamePos, Name.end(), PreviousCharInName);
    NeedlePos =
        IgnoreSpaces(NeedlePos, Needle.end(), PreviousCharInNeedle, IsPrefix);
    எனில் (NeedlePos == Needle.end())
      நிறுத்து;
    எனில் (NamePos == Name.end())
      நிறுத்து;
    எனில் (toUpper(*NeedlePos) != toUpper(*NamePos))
      நிறுத்து;
    NeedlePos++;
    NamePos++;
  }
  Consummed = std::distance(Name.begin(), NamePos);
  எனில் (NeedlePos != Needle.end()) {
    PreviousCharInName = PreviousCharInNameOrigin;
  }
  திரும்பு NeedlePos == Needle.end();
}

நிலையான std::tuple<முனை, தருக்க, uint32_t>
compare_முனை(uint32_t Offset, StringRef Name, தருக்க Strict,
            எழுத்து PreviousCharInName, BufferType &Buffer,
            மாறிலி முனை *Parent = nullptr) {
  முனை N = read_முனை(Offset, Parent);
  std::size_t Consummed = 0;
  தருக்க DoesStartWith = N.Is_வேர் || startsWith(Name, N.Name, Strict, Consummed,
                                              PreviousCharInName);
  எனில் (!DoesStartWith)
    திரும்பு std::make_tuple(N, பொய், 0);

  எனில் (Name.size() - Consummed == 0 && N.மதிப்பு != 0xFFFFFFFF)
    திரும்பு std::make_tuple(N, உண்மை, N.மதிப்பு);

  எனில் (N.hasChildren()) {
    uint32_t ChildOffset = N.ChildrenOffset;
    க்கு (;;) {
      முனை C;
      தருக்க Matches;
      uint32_t மதிப்பு;
      std::tie(C, Matches, மதிப்பு) =
          compare_முனை(ChildOffset, Name.substr(Consummed), Strict,
                      PreviousCharInName, Buffer, &N);
      எனில் (Matches) {
        std::reverse_copy(C.Name.begin(), C.Name.end(),
                          std::back_inserter(Buffer));
        திரும்பு std::make_tuple(N, உண்மை, மதிப்பு);
      }
      ChildOffset += C.Size;
      எனில் (!C.HasSibling)
        நிறுத்து;
    }
  }
  திரும்பு std::make_tuple(N, பொய், 0);
}

நிலையான std::tuple<முனை, தருக்க, uint32_t>
compare_முனை(uint32_t Offset, StringRef Name, தருக்க Strict, BufferType &Buffer) {
  திரும்பு compare_முனை(Offset, Name, Strict, 0, Buffer);
}

// clang-format off
constexpr மாறிலி எழுத்து *மாறிலி HangulSyllables[][3] = {
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

நிலையான std::size_t findSyllable(StringRef Name, தருக்க Strict,
                                எழுத்து &PreviousInName, முழு &Pos, முழு Column) {
  ப்ரதிஜ்ஞா(Column == 0 || Column == 1 || Column == 2);
  நிலையான std::size_t மொத்தம்_Per_Column[] = {LCount, VCount, TCount};
  முழு Len = -1;
  முழு Prev = PreviousInName;
  க்கு (std::size_t I = 0; I < மொத்தம்_Per_Column[Column]; I++) {
    StringRef Syllable(HangulSyllables[I][Column]);
    எனில் (முழு(Syllable.size()) <= Len)
      தொடர்;
    std::size_t Consummed = 0;
    எழுத்து PreviousInNameCopy = PreviousInName;
    தருக்க DoesStartWith =
        startsWith(Name, Syllable, Strict, Consummed, PreviousInNameCopy);
    எனில் (!DoesStartWith)
      தொடர்;
    Len = Consummed;
    Pos = I;
    Prev = PreviousInNameCopy;
  }
  எனில் (Len == -1)
    திரும்பு 0;
  PreviousInName = Prev;
  திரும்பு size_t(Len);
}

நிலையான std::optional<char32_t>
nameToHangulCodePoint(StringRef Name, தருக்க Strict, BufferType &Buffer) {
  Buffer.clear();
  // Hangul Syllable Decomposition
  std::size_t Consummed = 0;
  எழுத்து NameStart = 0;
  தருக்க DoesStartWith =
      startsWith(Name, "HANGUL SYLLABLE ", Strict, Consummed, NameStart);
  எனில் (!DoesStartWith)
    திரும்பு std::nullopt;
  Name = Name.substr(Consummed);
  முழு L = -1, V = -1, T = -1;
  Name = Name.substr(findSyllable(Name, Strict, NameStart, L, 0));
  Name = Name.substr(findSyllable(Name, Strict, NameStart, V, 1));
  Name = Name.substr(findSyllable(Name, Strict, NameStart, T, 2));
  எனில் (L != -1 && V != -1 && T != -1 && Name.empty()) {
    எனில் (!Strict) {
      Buffer.append("HANGUL SYLLABLE ");
      எனில் (L != -1)
        Buffer.append(HangulSyllables[L][0]);
      எனில் (V != -1)
        Buffer.append(HangulSyllables[V][1]);
      எனில் (T != -1)
        Buffer.append(HangulSyllables[T][2]);
    }
    திரும்பு SBase + (std::uint32_t(L) * VCount + std::uint32_t(V)) * TCount +
           std::uint32_t(T);
  }
  // Otherwise, it's an illegal syllable name.
  திரும்பு std::nullopt;
}

கட்டமைப்பு GeneratedNamesData {
  StringRef Prefix;
  uint32_t Start;
  uint32_t End;
};

// Unicode 18.0 Name Derivation Rule Prefix Strings (extracted/DerivedName.txt)
நிலையான மாறிலி GeneratedNamesData GeneratedNamesDataTable[] = {
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

நிலையான std::optional<char32_t>
nameToGeneratedCodePoint(StringRef Name, தருக்க Strict, BufferType &Buffer) {
  க்கு (auto &&Item : GeneratedNamesDataTable) {
    Buffer.clear();
    std::size_t Consummed = 0;
    எழுத்து NameStart = 0;
    தருக்க DoesStartWith = startsWith(Name, Item.Prefix, Strict, Consummed,
                                    NameStart, /*IsPrefix=*/உண்மை);
    எனில் (!DoesStartWith)
      தொடர்;
    auto எண் = Name.substr(Consummed);
    unsigned long long V = 0;
    // Be consistent about mandating upper casing.
    எனில் (Strict &&
        llvm::any_of(எண், [](எழுத்து C) { திரும்பு C >= 'a' && C <= 'f'; }))
      திரும்பு {};
    எனில் (getAsUnsignedInteger(எண், 16, V) || V < Item.Start || V > Item.End)
      தொடர்;
    எனில் (!Strict) {
      Buffer.append(Item.Prefix);
      Buffer.append(utohexstr(V, உண்மை));
    }
    திரும்பு V;
  }
  திரும்பு std::nullopt;
}

நிலையான std::optional<char32_t> nameToCodepoint(StringRef Name, தருக்க Strict,
                                               BufferType &Buffer) {
  எனில் (Name.empty())
    திரும்பு std::nullopt;

  std::optional<char32_t> Res = nameToHangulCodePoint(Name, Strict, Buffer);
  எனில் (!Res)
    Res = nameToGeneratedCodePoint(Name, Strict, Buffer);
  எனில் (Res)
    திரும்பு *Res;

  Buffer.clear();
  முனை முனை;
  தருக்க Matches;
  uint32_t மதிப்பு;
  std::tie(முனை, Matches, மதிப்பு) = compare_முனை(0, Name, Strict, Buffer);
  எனில் (Matches) {
    std::reverse(Buffer.begin(), Buffer.end());
    // UAX44-LM2. Ignore case, whitespace, underscore ('_'), and all medial
    // hyphens except the hyphen in U+1180 HANGUL JUNGSEONG O-E.
    எனில் (!Strict && மதிப்பு == 0x116c && Name.contains_insensitive("O-E")) {
      Buffer = "HANGUL JUNGSEONG O-E";
      மதிப்பு = 0x1180;
    }
    திரும்பு மதிப்பு;
  }
  திரும்பு std::nullopt;
}

std::optional<char32_t> nameToCodepointStrict(StringRef Name) {

  BufferType Buffer;
  auto Opt = nameToCodepoint(Name, உண்மை, Buffer);
  திரும்பு Opt;
}

std::optional<LooseMatchingResult>
nameToCodepointLooseMatching(StringRef Name) {
  BufferType Buffer;
  auto Opt = nameToCodepoint(Name, பொய், Buffer);
  எனில் (!Opt)
    திரும்பு std::nullopt;
  திரும்பு LooseMatchingResult{*Opt, Buffer};
}

// Find the unicode character whose editing distance to Pattern
// is shortest, using the Wagner–Fischer algorithm.
llvm::SmallVector<MatchForCodepointName>
nearestMatchesForCodepointName(StringRef Pattern, std::size_t Max_Matches_மொத்தம்) {
  // We maintain a fixed size vector of matches,
  // sorted by distance
  // The worst match (with the biggest distance) are discarded when new elements
  // are added.
  std::size_t LargestEditDistance = 0;
  llvm::SmallVector<MatchForCodepointName> Matches;
  Matches.reserve(Max_Matches_மொத்தம் + 1);

  auto செருகு_2 = [&](மாறிலி முனை &முனை, uint32_t Distance,
                    char32_t மதிப்பு) -> தருக்க {
    எனில் (Distance > LargestEditDistance) {
      எனில் (Matches.size() == Max_Matches_மொத்தம்)
        திரும்பு பொய்;
      LargestEditDistance = Distance;
    }
    // To avoid allocations, the creation of the name is delayed
    // as much as possible.
    std::string Name;
    auto GetName = [&] {
      எனில் (Name.empty())
        Name = முனை.fullName();
      திரும்பு Name;
    };

    auto It = llvm::lower_bound(
        Matches, Distance,
        [&](மாறிலி MatchForCodepointName &a, std::size_t Distance) {
          எனில் (Distance == a.Distance)
            திரும்பு a.Name < GetName();
          திரும்பு a.Distance < Distance;
        });
    எனில் (It == Matches.end() && Matches.size() == Max_Matches_மொத்தம்)
      திரும்பு பொய்;

    MatchForCodepointName M{GetName(), Distance, மதிப்பு};
    Matches.செருகு(It, std::move(M));
    எனில் (Matches.size() > Max_Matches_மொத்தம்)
      Matches.எடு_back();
    திரும்பு உண்மை;
  };

  // We ignore case, space, hyphens, etc,
  // in both the search pattern and the prospective names.
  auto Normalize = [](StringRef Name) {
    std::string Out;
    Out.reserve(Name.size());
    க்கு (எழுத்து C : Name) {
      எனில் (isAlnum(C))
        Out.தள்ளு_back(toUpper(C));
    }
    திரும்பு Out;
  };
  std::string NormalizedName = Normalize(Pattern);

  // Allocate a matrix big enough for longest names.
  மாறிலி std::size_t Columns =
      std::min(NormalizedName.size(), UnicodeNameToCodepointLargestNameSize) +
      1;

  [[maybe_unused]] நிலையான std::size_t Rows =
      UnicodeNameToCodepointLargestNameSize + 1;

  std::vector<எழுத்து> Distances(
      Columns * (UnicodeNameToCodepointLargestNameSize + 1), 0);

  auto Get = [&Distances, Columns](size_t Column, std::size_t Row) -> எழுத்து & {
    ப்ரதிஜ்ஞா(Column < Columns);
    ப்ரதிஜ்ஞா(Row < Rows);
    திரும்பு Distances[Row * Columns + Column];
  };

  க்கு (std::size_t I = 0; I < Columns; I++)
    Get(I, 0) = I;

  // Visit the childrens,
  // Filling (and overriding) the matrix for the name fragment of each node
  // iteratively. CompleteName is used to collect the actual name of potential
  // match, respecting case and spacing.
  auto Visit_முனை = [&](மாறிலி முனை &N, std::size_t Row,
                       auto &Visit_முனை) -> வெறுமை {
    std::size_t J = 0;
    க்கு (; J < N.Name.size(); J++) {
      எனில் (!isAlnum(N.Name[J]))
        தொடர்;

      Get(0, Row) = Row;

      க்கு (std::size_t I = 1; I < Columns; I++) {
        மாறிலி முழு Delete = Get(I - 1, Row) + 1;
        மாறிலி முழு செருகு_2 = Get(I, Row - 1) + 1;

        மாறிலி முழு Replace =
            Get(I - 1, Row - 1) + (NormalizedName[I - 1] != N.Name[J] ? 1 : 0);

        Get(I, Row) = std::min({செருகு_2, Delete, Replace});
      }

      Row++;
    }

    unsigned Cost = Get(Columns - 1, Row - 1);
    எனில் (N.மதிப்பு != 0xFFFFFFFF) {
      செருகு_2(N, Cost, N.மதிப்பு);
    }

    எனில் (N.hasChildren()) {
      auto ChildOffset = N.ChildrenOffset;
      க்கு (;;) {
        முனை C = read_முனை(ChildOffset, &N);
        ChildOffset += C.Size;
        எனில் (!C.isValid())
          நிறுத்து;
        Visit_முனை(C, Row, Visit_முனை);
        எனில் (!C.HasSibling)
          நிறுத்து;
      }
    }
  };

  முனை வேர்_2 = create_வேர்();
  Visit_முனை(வேர்_2, 1, Visit_முனை);
  திரும்பு Matches;
}

} // namespace unicode

} // namespace sys
} // namespace llvm
