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

#ਅਨ੍ਤਰ੍ਭਾਵ "llvm/ADT/STLExtras.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "llvm/ADT/StringExtras.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "llvm/ADT/StringRef.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "llvm/Support/Unicode.h"

ਨਾਮਾਕਾਸ਼ llvm {
ਨਾਮਾਕਾਸ਼ sys {
ਨਾਮਾਕਾਸ਼ unicode {

ਬਾਹਰੀ ਅਚਲ ਅੱਖਰ *ਅਚਲ Unicode_Name_ਮੰਜ਼ਿਲ_Codepoint_Dict;
ਬਾਹਰੀ ਅਚਲ uint8_t *ਅਚਲ Unicode_Name_ਮੰਜ਼ਿਲ_Codepoint_ਸੂਚਕਾਂਕ;
ਬਾਹਰੀ ਅਚਲ std::size_t Unicode_Name_ਮੰਜ਼ਿਲ_Codepoint_ਸੂਚਕਾਂਕ_ਮਾਪ;
ਬਾਹਰੀ ਅਚਲ std::size_t Unicode_Name_ਮੰਜ਼ਿਲ_Codepoint_ਸਭ_ਤੋਂ_ਵੱਡਾ_Name_ਮਾਪ;

ਵਰਤੋਂ ਬਫਰ_Type = SmallString<64>;

ਬਣਤਰ ਗੰਢ {
  ਤਾਰਕਿਕ Is_ਜੜ੍ਹ = ਝੂਠ;
  char32_t ਮੁੱਲ = 0xFFFFFFFF;
  uint32_t Children_ਵਿੱਥ = 0;
  ਤਾਰਕਿਕ HasSibling = ਝੂਠ;
  uint32_t ਮਾਪ = 0;
  StringRef Name;
  ਅਚਲ ਗੰਢ *ਮਾਪਾ_2 = ਖਾਲੀ_ਸੂਚਕ;

  constexpr ਤਾਰਕਿਕ isValid() ਅਚਲ {
    ਵਾਪਸ !Name.ਖਾਲੀ_ਥਾਵਾਂ() || ਮੁੱਲ == 0xFFFFFFFF;
  }
  constexpr ਤਾਰਕਿਕ hasChildren() ਅਚਲ { ਵਾਪਸ Children_ਵਿੱਥ != 0 || Is_ਜੜ੍ਹ; }

  std::string ਭਰਿਆ_Name() ਅਚਲ {
    std::string S;
    // Reserve enough space for most unicode code points.
    // The chosen value represent the 99th percentile of name size as of
    // Unicode 15.0.
    S.reserve(46);
    ਅਚਲ ਗੰਢ *N = ਇਹ;
    ਜਦੋਂ (N) {
      std::ਉਲਟ_copy(N->Name.begin(), N->Name.ਸਿਰਾ(), std::ਪਿੱਠ_inserter(S));
      N = N->ਮਾਪਾ_2;
    }
    std::ਉਲਟ(S.begin(), S.ਸਿਰਾ());
    ਵਾਪਸ S;
  }
};

ਸਥਿਰ ਗੰਢ create_ਜੜ੍ਹ() {
  ਗੰਢ N;
  N.Is_ਜੜ੍ਹ = ਸੱਚ;
  N.Children_ਵਿੱਥ = 1;
  N.ਮਾਪ = 1;
  ਵਾਪਸ N;
}

ਸਥਿਰ ਗੰਢ read_ਗੰਢ(uint32_t ਵਿੱਥ, ਅਚਲ ਗੰਢ *ਮਾਪਾ_2 = ਖਾਲੀ_ਸੂਚਕ) {
  ਜੇ (ਵਿੱਥ == 0)
    ਵਾਪਸ create_ਜੜ੍ਹ();

  uint32_t Origin = ਵਿੱਥ;
  ਗੰਢ N;
  N.ਮਾਪਾ_2 = ਮਾਪਾ_2;
  uint8_t NameInfo = Unicode_Name_ਮੰਜ਼ਿਲ_Codepoint_ਸੂਚਕਾਂਕ[ਵਿੱਥ++];
  ਜੇ (ਵਿੱਥ + 6 >= Unicode_Name_ਮੰਜ਼ਿਲ_Codepoint_ਸੂਚਕਾਂਕ_ਮਾਪ)
    ਵਾਪਸ N;

  ਤਾਰਕਿਕ LongName = NameInfo & 0x40;
  ਤਾਰਕਿਕ Has_ਮੁੱਲ = NameInfo & 0x80;
  std::size_t ਮਾਪ = NameInfo & ~0xC0;
  ਜੇ (LongName) {
    uint32_t Name_ਵਿੱਥ = (Unicode_Name_ਮੰਜ਼ਿਲ_Codepoint_ਸੂਚਕਾਂਕ[ਵਿੱਥ++] << 8);
    Name_ਵਿੱਥ |= Unicode_Name_ਮੰਜ਼ਿਲ_Codepoint_ਸੂਚਕਾਂਕ[ਵਿੱਥ++];
    N.Name = StringRef(Unicode_Name_ਮੰਜ਼ਿਲ_Codepoint_Dict + Name_ਵਿੱਥ, ਮਾਪ);
  } ਨਹੀਂਤਾਂ {
    N.Name = StringRef(Unicode_Name_ਮੰਜ਼ਿਲ_Codepoint_Dict + ਮਾਪ, 1);
  }
  ਜੇ (Has_ਮੁੱਲ) {
    uint8_t H = Unicode_Name_ਮੰਜ਼ਿਲ_Codepoint_ਸੂਚਕਾਂਕ[ਵਿੱਥ++];
    uint8_t M = Unicode_Name_ਮੰਜ਼ਿਲ_Codepoint_ਸੂਚਕਾਂਕ[ਵਿੱਥ++];
    uint8_t L = Unicode_Name_ਮੰਜ਼ਿਲ_Codepoint_ਸੂਚਕਾਂਕ[ਵਿੱਥ++];
    N.ਮੁੱਲ = ((H << 16) | (M << 8) | L) >> 3;

    ਤਾਰਕਿਕ HasChildren = L & 0x02;
    N.HasSibling = L & 0x01;

    ਜੇ (HasChildren) {
      N.Children_ਵਿੱਥ = Unicode_Name_ਮੰਜ਼ਿਲ_Codepoint_ਸੂਚਕਾਂਕ[ਵਿੱਥ++] << 16;
      N.Children_ਵਿੱਥ |= Unicode_Name_ਮੰਜ਼ਿਲ_Codepoint_ਸੂਚਕਾਂਕ[ਵਿੱਥ++] << 8;
      N.Children_ਵਿੱਥ |= Unicode_Name_ਮੰਜ਼ਿਲ_Codepoint_ਸੂਚਕਾਂਕ[ਵਿੱਥ++];
    }
  } ਨਹੀਂਤਾਂ {
    uint8_t H = Unicode_Name_ਮੰਜ਼ਿਲ_Codepoint_ਸੂਚਕਾਂਕ[ਵਿੱਥ++];
    N.HasSibling = H & 0x80;
    ਤਾਰਕਿਕ HasChildren = H & 0x40;
    H &= uint8_t(~0xC0);
    ਜੇ (HasChildren) {
      N.Children_ਵਿੱਥ = (H << 16);
      N.Children_ਵਿੱਥ |=
          (uint32_t(Unicode_Name_ਮੰਜ਼ਿਲ_Codepoint_ਸੂਚਕਾਂਕ[ਵਿੱਥ++]) << 8);
      N.Children_ਵਿੱਥ |= Unicode_Name_ਮੰਜ਼ਿਲ_Codepoint_ਸੂਚਕਾਂਕ[ਵਿੱਥ++];
    }
  }
  N.ਮਾਪ = ਵਿੱਥ - Origin;
  ਵਾਪਸ N;
}

ਸਥਿਰ ਤਾਰਕਿਕ startsWith(StringRef Name, StringRef Needle, ਤਾਰਕਿਕ Strict,
                       std::size_t &Consummed, ਅੱਖਰ &ਪਿਛਲਾ_Char_ਵਿੱਚ_Name,
                       ਤਾਰਕਿਕ Is_ਅਗੇਤਰ = ਝੂਠ) {

  Consummed = 0;
  ਜੇ (Strict) {
    ਜੇ (!Name.starts_with(Needle))
      ਵਾਪਸ ਝੂਠ;
    Consummed = Needle.ਮਾਪ_2();
    ਵਾਪਸ ਸੱਚ;
  }
  ਜੇ (Needle.ਖਾਲੀ_ਥਾਵਾਂ())
    ਵਾਪਸ ਸੱਚ;

  ਸਵੈਚਾਲਿਤ NamePos = Name.begin();
  ਸਵੈਚਾਲਿਤ NeedlePos = Needle.begin();

  ਅੱਖਰ ਪਿਛਲਾ_Char_ਵਿੱਚ_Name_Origin = ਪਿਛਲਾ_Char_ਵਿੱਚ_Name;
  ਅੱਖਰ ਪਿਛਲਾ_Char_ਵਿੱਚ_Needle = *Needle.begin();
  ਸਵੈਚਾਲਿਤ IgnoreSpaces = [](ਸਵੈਚਾਲਿਤ It, ਸਵੈਚਾਲਿਤ ਸਿਰਾ_2, ਅੱਖਰ &ਪਿਛਲਾ_Char,
                         ਤਾਰਕਿਕ Is_ਅਗੇਤਰ = ਝੂਠ) {
    ਜਦੋਂ (It != ਸਿਰਾ_2) {
      ਅਚਲ ਸਵੈਚਾਲਿਤ ਅਗਲਾ_2 = std::ਅਗਲਾ(It);
      // Ignore spaces, underscore, medial hyphens
      // The generator ensures a needle never ends (or starts) by a medial
      // hyphen https://unicode.org/reports/tr44/#UAX44-LM2.
      ਤਾਰਕਿਕ Ignore =
          *It == ' ' || *It == '_' ||
          (*It == '-' && isAlnum(ਪਿਛਲਾ_Char) &&
           ((ਅਗਲਾ_2 != ਸਿਰਾ_2 && isAlnum(*ਅਗਲਾ_2)) || (ਅਗਲਾ_2 == ਸਿਰਾ_2 && Is_ਅਗੇਤਰ)));
      ਪਿਛਲਾ_Char = *It;
      ਜੇ (!Ignore)
        ਰੋਕੋ;
      ++It;
    }
    ਵਾਪਸ It;
  };

  ਜਦੋਂ (ਸੱਚ) {
    NamePos = IgnoreSpaces(NamePos, Name.ਸਿਰਾ(), ਪਿਛਲਾ_Char_ਵਿੱਚ_Name);
    NeedlePos =
        IgnoreSpaces(NeedlePos, Needle.ਸਿਰਾ(), ਪਿਛਲਾ_Char_ਵਿੱਚ_Needle, Is_ਅਗੇਤਰ);
    ਜੇ (NeedlePos == Needle.ਸਿਰਾ())
      ਰੋਕੋ;
    ਜੇ (NamePos == Name.ਸਿਰਾ())
      ਰੋਕੋ;
    ਜੇ (ਮੰਜ਼ਿਲ_ਉੱਤਲਾ(*NeedlePos) != ਮੰਜ਼ਿਲ_ਉੱਤਲਾ(*NamePos))
      ਰੋਕੋ;
    NeedlePos++;
    NamePos++;
  }
  Consummed = std::ਦੂਰੀ(Name.begin(), NamePos);
  ਜੇ (NeedlePos != Needle.ਸਿਰਾ()) {
    ਪਿਛਲਾ_Char_ਵਿੱਚ_Name = ਪਿਛਲਾ_Char_ਵਿੱਚ_Name_Origin;
  }
  ਵਾਪਸ NeedlePos == Needle.ਸਿਰਾ();
}

ਸਥਿਰ std::tuple<ਗੰਢ, ਤਾਰਕਿਕ, uint32_t>
compare_ਗੰਢ(uint32_t ਵਿੱਥ, StringRef Name, ਤਾਰਕਿਕ Strict,
            ਅੱਖਰ ਪਿਛਲਾ_Char_ਵਿੱਚ_Name, ਬਫਰ_Type &ਬਫਰ,
            ਅਚਲ ਗੰਢ *ਮਾਪਾ_2 = ਖਾਲੀ_ਸੂਚਕ) {
  ਗੰਢ N = read_ਗੰਢ(ਵਿੱਥ, ਮਾਪਾ_2);
  std::size_t Consummed = 0;
  ਤਾਰਕਿਕ Does_ਸ਼ੁਰੂ_With = N.Is_ਜੜ੍ਹ || startsWith(Name, N.Name, Strict, Consummed,
                                              ਪਿਛਲਾ_Char_ਵਿੱਚ_Name);
  ਜੇ (!Does_ਸ਼ੁਰੂ_With)
    ਵਾਪਸ std::make_tuple(N, ਝੂਠ, 0);

  ਜੇ (Name.ਮਾਪ_2() - Consummed == 0 && N.ਮੁੱਲ != 0xFFFFFFFF)
    ਵਾਪਸ std::make_tuple(N, ਸੱਚ, N.ਮੁੱਲ);

  ਜੇ (N.hasChildren()) {
    uint32_t ਬੱਚਾ_ਵਿੱਥ = N.Children_ਵਿੱਥ;
    ਲਈ (;;) {
      ਗੰਢ C;
      ਤਾਰਕਿਕ Matches;
      uint32_t ਮੁੱਲ;
      std::tie(C, Matches, ਮੁੱਲ) =
          compare_ਗੰਢ(ਬੱਚਾ_ਵਿੱਥ, Name.substr(Consummed), Strict,
                      ਪਿਛਲਾ_Char_ਵਿੱਚ_Name, ਬਫਰ, &N);
      ਜੇ (Matches) {
        std::ਉਲਟ_copy(C.Name.begin(), C.Name.ਸਿਰਾ(),
                          std::ਪਿੱਠ_inserter(ਬਫਰ));
        ਵਾਪਸ std::make_tuple(N, ਸੱਚ, ਮੁੱਲ);
      }
      ਬੱਚਾ_ਵਿੱਥ += C.ਮਾਪ;
      ਜੇ (!C.HasSibling)
        ਰੋਕੋ;
    }
  }
  ਵਾਪਸ std::make_tuple(N, ਝੂਠ, 0);
}

ਸਥਿਰ std::tuple<ਗੰਢ, ਤਾਰਕਿਕ, uint32_t>
compare_ਗੰਢ(uint32_t ਵਿੱਥ, StringRef Name, ਤਾਰਕਿਕ Strict, ਬਫਰ_Type &ਬਫਰ) {
  ਵਾਪਸ compare_ਗੰਢ(ਵਿੱਥ, Name, Strict, 0, ਬਫਰ);
}

// clang-format off
constexpr ਅਚਲ ਅੱਖਰ *ਅਚਲ HangulSyllables[][3] = {
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

ਸਥਿਰ std::size_t ਲੱਭੋ_Syllable(StringRef Name, ਤਾਰਕਿਕ Strict,
                                ਅੱਖਰ &ਪਿਛਲਾ_ਵਿੱਚ_Name, ਪੂਰਨ &Pos, ਪੂਰਨ ਕਾਲਮ) {
  ਪ੍ਰਤਿਜ੍ਞਾ(ਕਾਲਮ == 0 || ਕਾਲਮ == 1 || ਕਾਲਮ == 2);
  ਸਥਿਰ std::size_t ਗਿਣਤੀ_ਪ੍ਰਤੀ_ਕਾਲਮ[] = {LCount, VCount, TCount};
  ਪੂਰਨ Len = -1;
  ਪੂਰਨ Prev = ਪਿਛਲਾ_ਵਿੱਚ_Name;
  ਲਈ (std::size_t I = 0; I < ਗਿਣਤੀ_ਪ੍ਰਤੀ_ਕਾਲਮ[ਕਾਲਮ]; I++) {
    StringRef Syllable(HangulSyllables[I][ਕਾਲਮ]);
    ਜੇ (ਪੂਰਨ(Syllable.ਮਾਪ_2()) <= Len)
      ਜਾਰੀ;
    std::size_t Consummed = 0;
    ਅੱਖਰ ਪਿਛਲਾ_ਵਿੱਚ_Name_Copy = ਪਿਛਲਾ_ਵਿੱਚ_Name;
    ਤਾਰਕਿਕ Does_ਸ਼ੁਰੂ_With =
        startsWith(Name, Syllable, Strict, Consummed, ਪਿਛਲਾ_ਵਿੱਚ_Name_Copy);
    ਜੇ (!Does_ਸ਼ੁਰੂ_With)
      ਜਾਰੀ;
    Len = Consummed;
    Pos = I;
    Prev = ਪਿਛਲਾ_ਵਿੱਚ_Name_Copy;
  }
  ਜੇ (Len == -1)
    ਵਾਪਸ 0;
  ਪਿਛਲਾ_ਵਿੱਚ_Name = Prev;
  ਵਾਪਸ size_t(Len);
}

ਸਥਿਰ std::optional<char32_t>
name_ਮੰਜ਼ਿਲ_Hangul_ਕੋਡ_ਬਿੰਦੂ(StringRef Name, ਤਾਰਕਿਕ Strict, ਬਫਰ_Type &ਬਫਰ) {
  ਬਫਰ.clear();
  // Hangul Syllable Decomposition
  std::size_t Consummed = 0;
  ਅੱਖਰ Name_ਸ਼ੁਰੂ = 0;
  ਤਾਰਕਿਕ Does_ਸ਼ੁਰੂ_With =
      startsWith(Name, "HANGUL SYLLABLE ", Strict, Consummed, Name_ਸ਼ੁਰੂ);
  ਜੇ (!Does_ਸ਼ੁਰੂ_With)
    ਵਾਪਸ std::nullopt;
  Name = Name.substr(Consummed);
  ਪੂਰਨ L = -1, V = -1, T = -1;
  Name = Name.substr(ਲੱਭੋ_Syllable(Name, Strict, Name_ਸ਼ੁਰੂ, L, 0));
  Name = Name.substr(ਲੱਭੋ_Syllable(Name, Strict, Name_ਸ਼ੁਰੂ, V, 1));
  Name = Name.substr(ਲੱਭੋ_Syllable(Name, Strict, Name_ਸ਼ੁਰੂ, T, 2));
  ਜੇ (L != -1 && V != -1 && T != -1 && Name.ਖਾਲੀ_ਥਾਵਾਂ()) {
    ਜੇ (!Strict) {
      ਬਫਰ.append("HANGUL SYLLABLE ");
      ਜੇ (L != -1)
        ਬਫਰ.append(HangulSyllables[L][0]);
      ਜੇ (V != -1)
        ਬਫਰ.append(HangulSyllables[V][1]);
      ਜੇ (T != -1)
        ਬਫਰ.append(HangulSyllables[T][2]);
    }
    ਵਾਪਸ SBase + (std::uint32_t(L) * VCount + std::uint32_t(V)) * TCount +
           std::uint32_t(T);
  }
  // Otherwise, it's an illegal syllable name.
  ਵਾਪਸ std::nullopt;
}

ਬਣਤਰ Generated_ਨਾਂ_ਅੰਕੜੇ {
  StringRef ਅਗੇਤਰ;
  uint32_t ਸ਼ੁਰੂ;
  uint32_t ਸਿਰਾ_2;
};

// Unicode 18.0 Name Derivation Rule Prefix Strings (extracted/DerivedName.txt)
ਸਥਿਰ ਅਚਲ Generated_ਨਾਂ_ਅੰਕੜੇ Generated_ਨਾਂ_ਅੰਕੜੇ_ਸਾਰਣੀ[] = {
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

ਸਥਿਰ std::optional<char32_t>
name_ਮੰਜ਼ਿਲ_Generated_ਕੋਡ_ਬਿੰਦੂ(StringRef Name, ਤਾਰਕਿਕ Strict, ਬਫਰ_Type &ਬਫਰ) {
  ਲਈ (ਸਵੈਚਾਲਿਤ &&ਵਸਤੂ : Generated_ਨਾਂ_ਅੰਕੜੇ_ਸਾਰਣੀ) {
    ਬਫਰ.clear();
    std::size_t Consummed = 0;
    ਅੱਖਰ Name_ਸ਼ੁਰੂ = 0;
    ਤਾਰਕਿਕ Does_ਸ਼ੁਰੂ_With = startsWith(Name, ਵਸਤੂ.ਅਗੇਤਰ, Strict, Consummed,
                                    Name_ਸ਼ੁਰੂ, /*IsPrefix=*/ਸੱਚ);
    ਜੇ (!Does_ਸ਼ੁਰੂ_With)
      ਜਾਰੀ;
    ਸਵੈਚਾਲਿਤ ਸੰਖਿਆ = Name.substr(Consummed);
    ਬਿਨਾਨਿਸ਼ਾਨੀ ਲੰਮਾ ਲੰਮਾ V = 0;
    // Be consistent about mandating upper casing.
    ਜੇ (Strict &&
        llvm::any_ਦਾ(ਸੰਖਿਆ, [](ਅੱਖਰ C) { ਵਾਪਸ C >= 'a' && C <= 'f'; }))
      ਵਾਪਸ {};
    ਜੇ (getAsUnsignedInteger(ਸੰਖਿਆ, 16, V) || V < ਵਸਤੂ.ਸ਼ੁਰੂ || V > ਵਸਤੂ.ਸਿਰਾ_2)
      ਜਾਰੀ;
    ਜੇ (!Strict) {
      ਬਫਰ.append(ਵਸਤੂ.ਅਗੇਤਰ);
      ਬਫਰ.append(utohexstr(V, ਸੱਚ));
    }
    ਵਾਪਸ V;
  }
  ਵਾਪਸ std::nullopt;
}

ਸਥਿਰ std::optional<char32_t> name_ਮੰਜ਼ਿਲ_Codepoint(StringRef Name, ਤਾਰਕਿਕ Strict,
                                               ਬਫਰ_Type &ਬਫਰ) {
  ਜੇ (Name.ਖਾਲੀ_ਥਾਵਾਂ())
    ਵਾਪਸ std::nullopt;

  std::optional<char32_t> Res = name_ਮੰਜ਼ਿਲ_Hangul_ਕੋਡ_ਬਿੰਦੂ(Name, Strict, ਬਫਰ);
  ਜੇ (!Res)
    Res = name_ਮੰਜ਼ਿਲ_Generated_ਕੋਡ_ਬਿੰਦੂ(Name, Strict, ਬਫਰ);
  ਜੇ (Res)
    ਵਾਪਸ *Res;

  ਬਫਰ.clear();
  ਗੰਢ ਗੰਢ;
  ਤਾਰਕਿਕ Matches;
  uint32_t ਮੁੱਲ;
  std::tie(ਗੰਢ, Matches, ਮੁੱਲ) = compare_ਗੰਢ(0, Name, Strict, ਬਫਰ);
  ਜੇ (Matches) {
    std::ਉਲਟ(ਬਫਰ.begin(), ਬਫਰ.ਸਿਰਾ());
    // UAX44-LM2. Ignore case, whitespace, underscore ('_'), and all medial
    // hyphens except the hyphen in U+1180 HANGUL JUNGSEONG O-E.
    ਜੇ (!Strict && ਮੁੱਲ == 0x116c && Name.ਸ਼ਾਮਲ_ਹੈ_insensitive("O-E")) {
      ਬਫਰ = "HANGUL JUNGSEONG O-E";
      ਮੁੱਲ = 0x1180;
    }
    ਵਾਪਸ ਮੁੱਲ;
  }
  ਵਾਪਸ std::nullopt;
}

std::optional<char32_t> name_ਮੰਜ਼ਿਲ_Codepoint_Strict(StringRef Name) {

  ਬਫਰ_Type ਬਫਰ;
  ਸਵੈਚਾਲਿਤ Opt = name_ਮੰਜ਼ਿਲ_Codepoint(Name, ਸੱਚ, ਬਫਰ);
  ਵਾਪਸ Opt;
}

std::optional<Loose_Matching_ਨਤੀਜਾ>
name_ਮੰਜ਼ਿਲ_Codepoint_Loose_Matching(StringRef Name) {
  ਬਫਰ_Type ਬਫਰ;
  ਸਵੈਚਾਲਿਤ Opt = name_ਮੰਜ਼ਿਲ_Codepoint(Name, ਝੂਠ, ਬਫਰ);
  ਜੇ (!Opt)
    ਵਾਪਸ std::nullopt;
  ਵਾਪਸ Loose_Matching_ਨਤੀਜਾ{*Opt, ਬਫਰ};
}

// Find the unicode character whose editing distance to Pattern
// is shortest, using the Wagner–Fischer algorithm.
llvm::Small_ਸਦਿਸ਼<ਮੇਲ_For_Codepoint_Name>
ਸਭ_ਤੋਂ_ਨੇੜੇ_Matches_For_Codepoint_Name(StringRef ਨਮੂਨਾ, std::size_t Max_Matches_ਗਿਣਤੀ) {
  // We maintain a fixed size vector of matches,
  // sorted by distance
  // The worst match (with the biggest distance) are discarded when new elements
  // are added.
  std::size_t ਸਭ_ਤੋਂ_ਵੱਡਾ_Edit_ਦੂਰੀ = 0;
  llvm::Small_ਸਦਿਸ਼<ਮੇਲ_For_Codepoint_Name> Matches;
  Matches.reserve(Max_Matches_ਗਿਣਤੀ + 1);

  ਸਵੈਚਾਲਿਤ ਪਾਓ_2 = [&](ਅਚਲ ਗੰਢ &ਗੰਢ, uint32_t ਦੂਰੀ_2,
                    char32_t ਮੁੱਲ) -> ਤਾਰਕਿਕ {
    ਜੇ (ਦੂਰੀ_2 > ਸਭ_ਤੋਂ_ਵੱਡਾ_Edit_ਦੂਰੀ) {
      ਜੇ (Matches.ਮਾਪ_2() == Max_Matches_ਗਿਣਤੀ)
        ਵਾਪਸ ਝੂਠ;
      ਸਭ_ਤੋਂ_ਵੱਡਾ_Edit_ਦੂਰੀ = ਦੂਰੀ_2;
    }
    // To avoid allocations, the creation of the name is delayed
    // as much as possible.
    std::string Name;
    ਸਵੈਚਾਲਿਤ GetName = [&] {
      ਜੇ (Name.ਖਾਲੀ_ਥਾਵਾਂ())
        Name = ਗੰਢ.ਭਰਿਆ_Name();
      ਵਾਪਸ Name;
    };

    ਸਵੈਚਾਲਿਤ It = llvm::ਹੇਠਲਾ_bound(
        Matches, ਦੂਰੀ_2,
        [&](ਅਚਲ ਮੇਲ_For_Codepoint_Name &a, std::size_t ਦੂਰੀ_2) {
          ਜੇ (ਦੂਰੀ_2 == a.ਦੂਰੀ_2)
            ਵਾਪਸ a.Name < GetName();
          ਵਾਪਸ a.ਦੂਰੀ_2 < ਦੂਰੀ_2;
        });
    ਜੇ (It == Matches.ਸਿਰਾ() && Matches.ਮਾਪ_2() == Max_Matches_ਗਿਣਤੀ)
      ਵਾਪਸ ਝੂਠ;

    ਮੇਲ_For_Codepoint_Name M{GetName(), ਦੂਰੀ_2, ਮੁੱਲ};
    Matches.ਪਾਓ(It, std::move(M));
    ਜੇ (Matches.ਮਾਪ_2() > Max_Matches_ਗਿਣਤੀ)
      Matches.ਕੱਢੋ_ਪਿੱਠ();
    ਵਾਪਸ ਸੱਚ;
  };

  // We ignore case, space, hyphens, etc,
  // in both the search pattern and the prospective names.
  ਸਵੈਚਾਲਿਤ Normalize = [](StringRef Name) {
    std::string ਬਾਹਰਲਾ;
    ਬਾਹਰਲਾ.reserve(Name.ਮਾਪ_2());
    ਲਈ (ਅੱਖਰ C : Name) {
      ਜੇ (isAlnum(C))
        ਬਾਹਰਲਾ.ਧੱਕੋ_ਪਿੱਠ(ਮੰਜ਼ਿਲ_ਉੱਤਲਾ(C));
    }
    ਵਾਪਸ ਬਾਹਰਲਾ;
  };
  std::string NormalizedName = Normalize(ਨਮੂਨਾ);

  // Allocate a matrix big enough for longest names.
  ਅਚਲ std::size_t Columns =
      std::min(NormalizedName.ਮਾਪ_2(), Unicode_Name_ਮੰਜ਼ਿਲ_Codepoint_ਸਭ_ਤੋਂ_ਵੱਡਾ_Name_ਮਾਪ) +
      1;

  [[maybe_ਅਣਵਰਤਿਆ]] ਸਥਿਰ std::size_t ਪੰਕਤੀਆਂ_2 =
      Unicode_Name_ਮੰਜ਼ਿਲ_Codepoint_ਸਭ_ਤੋਂ_ਵੱਡਾ_Name_ਮਾਪ + 1;

  std::ਸਦਿਸ਼<ਅੱਖਰ> Distances(
      Columns * (Unicode_Name_ਮੰਜ਼ਿਲ_Codepoint_ਸਭ_ਤੋਂ_ਵੱਡਾ_Name_ਮਾਪ + 1), 0);

  ਸਵੈਚਾਲਿਤ Get = [&Distances, Columns](size_t ਕਾਲਮ, std::size_t ਪੰਕਤੀ) -> ਅੱਖਰ & {
    ਪ੍ਰਤਿਜ੍ਞਾ(ਕਾਲਮ < Columns);
    ਪ੍ਰਤਿਜ੍ਞਾ(ਪੰਕਤੀ < ਪੰਕਤੀਆਂ_2);
    ਵਾਪਸ Distances[ਪੰਕਤੀ * Columns + ਕਾਲਮ];
  };

  ਲਈ (std::size_t I = 0; I < Columns; I++)
    Get(I, 0) = I;

  // Visit the childrens,
  // Filling (and overriding) the matrix for the name fragment of each node
  // iteratively. CompleteName is used to collect the actual name of potential
  // match, respecting case and spacing.
  ਸਵੈਚਾਲਿਤ ਦੇਖੋ_ਗੰਢ = [&](ਅਚਲ ਗੰਢ &N, std::size_t ਪੰਕਤੀ,
                       ਸਵੈਚਾਲਿਤ &ਦੇਖੋ_ਗੰਢ) -> ਖਾਲੀ {
    std::size_t J = 0;
    ਲਈ (; J < N.Name.ਮਾਪ_2(); J++) {
      ਜੇ (!isAlnum(N.Name[J]))
        ਜਾਰੀ;

      Get(0, ਪੰਕਤੀ) = ਪੰਕਤੀ;

      ਲਈ (std::size_t I = 1; I < Columns; I++) {
        ਅਚਲ ਪੂਰਨ Delete = Get(I - 1, ਪੰਕਤੀ) + 1;
        ਅਚਲ ਪੂਰਨ ਪਾਓ_2 = Get(I, ਪੰਕਤੀ - 1) + 1;

        ਅਚਲ ਪੂਰਨ Replace =
            Get(I - 1, ਪੰਕਤੀ - 1) + (NormalizedName[I - 1] != N.Name[J] ? 1 : 0);

        Get(I, ਪੰਕਤੀ) = std::min({ਪਾਓ_2, Delete, Replace});
      }

      ਪੰਕਤੀ++;
    }

    ਬਿਨਾਨਿਸ਼ਾਨੀ ਲਾਗਤ = Get(Columns - 1, ਪੰਕਤੀ - 1);
    ਜੇ (N.ਮੁੱਲ != 0xFFFFFFFF) {
      ਪਾਓ_2(N, ਲਾਗਤ, N.ਮੁੱਲ);
    }

    ਜੇ (N.hasChildren()) {
      ਸਵੈਚਾਲਿਤ ਬੱਚਾ_ਵਿੱਥ = N.Children_ਵਿੱਥ;
      ਲਈ (;;) {
        ਗੰਢ C = read_ਗੰਢ(ਬੱਚਾ_ਵਿੱਥ, &N);
        ਬੱਚਾ_ਵਿੱਥ += C.ਮਾਪ;
        ਜੇ (!C.isValid())
          ਰੋਕੋ;
        ਦੇਖੋ_ਗੰਢ(C, ਪੰਕਤੀ, ਦੇਖੋ_ਗੰਢ);
        ਜੇ (!C.HasSibling)
          ਰੋਕੋ;
      }
    }
  };

  ਗੰਢ ਜੜ੍ਹ_2 = create_ਜੜ੍ਹ();
  ਦੇਖੋ_ਗੰਢ(ਜੜ੍ਹ_2, 1, ਦੇਖੋ_ਗੰਢ);
  ਵਾਪਸ Matches;
}

} // namespace unicode

} // namespace sys
} // namespace llvm
